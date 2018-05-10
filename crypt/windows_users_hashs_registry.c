//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void addRegistryUsertoDB(char *source,char *name, char *RID, char *SID, char *group, char *description, char *last_logon, char *last_password_change,DWORD nb_connexion, char *type, DWORD state_id, DWORD session_id, sqlite3 *db);
void addPasswordtoDB(char *source, char*login, char*password, char*raw_password,unsigned int description_id,unsigned int session_id, sqlite3 *db);
//------------------------------------------------------------------------------
/*
Extrait en partie de Pwdump 1 de Jeremy Allison.
Pour modifier les droits et accéder aux données des utilisateurs dans le regitre.
*/
//------------------------------------------------------------------------------
//lecture du SID à partir d'un nom
BOOL get_sid(const char *name, SID **ppsid)
{
  SID_NAME_USE sid_use;
  DWORD sid_size = 0;
  DWORD dom_size = 0;
  char *domain;

  *ppsid = 0;
  if(LookupAccountName(0, name, 0, &sid_size, 0, &dom_size, &sid_use) == 0) {
    if(GetLastError() != ERROR_INSUFFICIENT_BUFFER)return FALSE;
  }

  *ppsid = (SID *)LocalAlloc( LMEM_FIXED, sid_size);
  domain = (char *)LocalAlloc( LMEM_FIXED, dom_size);
  if( (*ppsid == 0) || (domain == 0))
  {
    if(*ppsid)
      LocalFree((HLOCAL)*ppsid);
    if(domain)
      LocalFree((HLOCAL)domain);
    *ppsid = 0;
    return FALSE;
  }

  if(LookupAccountName(0, name, *ppsid, &sid_size, domain, &dom_size, &sid_use) == 0)
  {
    LocalFree((HLOCAL)*ppsid);
    LocalFree((HLOCAL)domain);
    *ppsid = 0;
    return FALSE;
  }

  LocalFree((HLOCAL)domain);
  return TRUE;
}
//------------------------------------------------------------------------------
//génération de profil DACL pour modifier les droits !!
BOOL create_sd_from_list( SECURITY_DESCRIPTOR *sdout, int num, ...)
{
  va_list ap;
  SID **sids = 0;
  char *name;
  DWORD amask;
  DWORD acl_size;
  PACL pacl = 0;
  int i;
  if((sids = (SID **)calloc(1,sizeof(SID *)*num)) == 0)return FALSE;

  acl_size = num * (sizeof(ACL) +
             sizeof(ACCESS_ALLOWED_ACE) +
             sizeof(DWORD));

  /* Collect all the SID's */
  va_start( ap, num);
  for( i = 0; i < num; i++) {
    name = va_arg( ap, char *);
    amask = va_arg(ap, DWORD);
    if(get_sid( name, &sids[i]) == FALSE)goto cleanup;

    acl_size += GetLengthSid(sids[i]);
  }
  va_end(ap);

  if((pacl = (PACL)LocalAlloc( LMEM_FIXED, acl_size)) == 0)goto cleanup;
  if(InitializeSecurityDescriptor( sdout, SECURITY_DESCRIPTOR_REVISION) == FALSE)goto cleanup;
  if(InitializeAcl( pacl, acl_size, ACL_REVISION) == FALSE)goto cleanup;

  va_start(ap, num);
  for( i = 0; i < num; i++) {
    ACE_HEADER *ace_p;
    name = va_arg( ap, char *);
    amask = va_arg( ap, DWORD);
    if(AddAccessAllowedAce( pacl, ACL_REVISION, amask, sids[i]) == FALSE)goto cleanup;

    /* Make sure the ACE is inheritable */
    if(GetAce( pacl, 0, (LPVOID *)&ace_p) == FALSE)goto cleanup;

    ace_p->AceFlags |= ( CONTAINER_INHERIT_ACE | OBJECT_INHERIT_ACE);
  }

  /* Add the ACL into the sd. */
  if(SetSecurityDescriptorDacl( sdout, TRUE, pacl, FALSE) == FALSE)goto cleanup;

  for( i = 0; i < num; i++)
    if(sids[i] != 0)
      LocalFree((HLOCAL)sids[i]);
  free(sids);

  return TRUE;

cleanup:

  if(sids != 0) {
    for( i = 0; i < num; i++)
      if(sids[i] != 0)
        LocalFree((HLOCAL)sids[i]);
    free(sids);
  }
  if(pacl != 0)
    LocalFree((HLOCAL)pacl);
  return FALSE;
}
//------------------------------------------------------------------------------
int set_userkeys_security( HKEY start, const char *path, SECURITY_DESCRIPTOR *psd,HKEY *return_key)
{
	HKEY key;
	DWORD err;
	char usersid[128];
	DWORD indx = 0;

	/* Open the path and enum all the user keys - setting
	   the same security on them. */
	if((err = RegOpenKeyEx( start, path, 0, KEY_ENUMERATE_SUB_KEYS, &key)) !=ERROR_SUCCESS)return -1;
	/* Now enumerate the subkeys, setting the security on them all. */
	do {
		DWORD size;
		FILETIME ft;

		size = sizeof(usersid);
		err = RegEnumKeyEx(	key, indx, usersid, &size, 0, 0, 0, &ft);
		if(err == ERROR_SUCCESS) {
			HKEY subkey;

			indx++;
			if((err = RegOpenKeyEx( key, usersid, 0, WRITE_DAC, &subkey)) !=ERROR_SUCCESS)
      {
				RegCloseKey(key);
				return -1;
			}
			if((err = RegSetKeySecurity( subkey, DACL_SECURITY_INFORMATION,psd)) != ERROR_SUCCESS)
      {
				RegCloseKey(subkey);
				RegCloseKey(key);
				return -1;
			}
			RegCloseKey(subkey);
		}
	} while(err == ERROR_SUCCESS);

	if(err != ERROR_NO_MORE_ITEMS)
  {
		RegCloseKey(key);
		return -1;
	}
	if(return_key == 0)
		RegCloseKey(key);
	else
		*return_key = key;
	return 0;
}
//------------------------------------------------------------------------------
BOOL AdministratorGroupName(char *group_name, unsigned short gn_max_size)
{
  BOOL ret = FALSE;
  if (MyNetApiBufferFree && MyNetLocalGroupEnum)
  {
    //Enumerate group to find Administrator group
    LPLOCALGROUP_INFO_0 pBuf = 0;
    DWORD nb = 0, total=0;

    NET_API_STATUS nStatus = MyNetLocalGroupEnum(0,0,(LPBYTE*)&pBuf,2048,&nb,&total,0);
    if (((nStatus == 0/*NERR_Success*/) || (nStatus == ERROR_MORE_DATA)) && ((pBuf) != 0) && (nb>0))
    {
        //le 1er compte est toujour l'administrateur, ils sont chargés dans l'ordre de rid!
        snprintf(group_name,gn_max_size,"%S",pBuf->lgrpi0_name);
        ret = TRUE;
    }
    MyNetApiBufferFree(pBuf);
  }
  return ret;
}
//------------------------------------------------------------------------------
int restore_sam_tree_access(HKEY start, char *pth)
{
	char path[MAX_PATH];
	char *p;
	HKEY key;
	DWORD err;
	SECURITY_DESCRIPTOR sd;
	DWORD admin_mask;

  snprintf(path,MAX_PATH,"%s",pth);
	admin_mask = WRITE_DAC | READ_CONTROL;

  char group_name[256];
  if (AdministratorGroupName(group_name, 255))
  {
      if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,group_name, admin_mask) == FALSE)       //get local admin group
        if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrators", admin_mask) == FALSE)       //english
          if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrateurs", admin_mask) == FALSE)    //french
            if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administradores", admin_mask) == FALSE)  //spanish
              if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administratoren", admin_mask) == FALSE)//dutch
            return -1;
  }else if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrators", admin_mask) == FALSE)       //english
          if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrateurs", admin_mask) == FALSE)    //french
            if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administradores", admin_mask) == FALSE)  //spanish
              if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administratoren", admin_mask) == FALSE)//dutch
            return -1;

	// Remove the security on the user keys first.
	if(set_userkeys_security( start, path, &sd, 0) != 0)return -1;

	// now go up the path, restoring security
	do {
		if((err = RegOpenKeyEx( start, path, 0, WRITE_DAC, &key)) !=ERROR_SUCCESS)return -1;
		if((err = RegSetKeySecurity( key, DACL_SECURITY_INFORMATION,&sd)) != ERROR_SUCCESS)
    {
			RegCloseKey(key);
			return  -1;
		}
		RegCloseKey(key);
		p = strrchr(path, '\\');
		if( p != 0)
			*p = 0;
	} while( p != 0 );

	return 0;
}
//------------------------------------------------------------------------------
int set_sam_tree_access( HKEY start, char *pth)
{
	char path[MAX_PATH];
	char *p;
	HKEY key;
	DWORD err;
	BOOL security_changed = FALSE;
	SECURITY_DESCRIPTOR sd;
	DWORD admin_mask;
	BOOL finished = FALSE;

	snprintf(path,MAX_PATH,"%s",pth);
	admin_mask = WRITE_DAC | READ_CONTROL | KEY_QUERY_VALUE | KEY_ENUMERATE_SUB_KEYS;

  //lecture du group d'administrateur
  char group_name[256];
  if (AdministratorGroupName(group_name, 255))
  {
      if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,group_name, admin_mask) == FALSE)       //local
        if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrators", admin_mask) == FALSE)       //english
          if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrateurs", admin_mask) == FALSE)    //french
            if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administradores", admin_mask) == FALSE)  //spanish
              if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administratoren", admin_mask) == FALSE)//dutch
                return -1;
  }else if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrators", admin_mask) == FALSE)       //english
          if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administrateurs", admin_mask) == FALSE)    //french
            if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administradores", admin_mask) == FALSE)  //spanish
              if(create_sd_from_list( &sd, 2, "SYSTEM", GENERIC_ALL,"Administratoren", admin_mask) == FALSE)//dutch
                return -1;

	p = strchr(path, '\\');

	do {
		if( p != 0)
			*p = 0;
		else
			finished = TRUE;

		if((err = RegOpenKeyEx( start, path, 0, WRITE_DAC, &key)) != ERROR_SUCCESS)return (security_changed ? -2: -1);
		if((err = RegSetKeySecurity( key, DACL_SECURITY_INFORMATION,&sd)) != ERROR_SUCCESS)
    {
			RegCloseKey(key);
			return (security_changed ? -2: -1);
		}
		security_changed = TRUE;
		RegCloseKey(key);
		if(p != 0) {
			*p++ = '\\';
			p = strchr(p, '\\');
		}
	} while( !finished );

	if(set_userkeys_security( start, path, &sd, &key) != 0)
		return -2;
	return 0;
}
//------------------------------------------------------------------------------
unsigned long int HTD(char *src) //pour conversion en hexa
{
 unsigned long int k=0;//resultat
 unsigned long int j=1,i; // j =coef multiplicateur et i variable de boucle

 //inversion pour test de traitement
 char hexa[8];
 hexa[0] = src[7];
 hexa[1] = src[6];
 hexa[2] = src[5];
 hexa[3] = src[4];
 hexa[4] = src[3];
 hexa[5] = src[2];
 hexa[6] = src[1];
 hexa[7] = src[0];

    for (i=0;i<8;i++)//calcul de la valeur hexa en decimal
    {
        switch(hexa[i])
        {
         case '1':k=k+j;break;
         case '2':k=k+(2*j);break;
         case '3':k=k+(3*j);break;
         case '4':k=k+(4*j);break;
         case '5':k=k+(5*j);break;
         case '6':k=k+(6*j);break;
         case '7':k=k+(7*j);break;
         case '8':k=k+(8*j);break;
         case '9':k=k+(9*j);break;
         case 'a':k=k+(10*j);break;
         case 'A':k=k+(10*j);break;
         case 'b':k=k+(11*j);break;
         case 'B':k=k+(11*j);break;
         case 'c':k=k+(12*j);break;
         case 'C':k=k+(12*j);break;
         case 'd':k=k+(13*j);break;
         case 'D':k=k+(13*j);break;
         case 'e':k=k+(14*j);break;
         case 'E':k=k+(14*j);break;
         case 'f':k=k+(15*j);break;
         case 'F':k=k+(15*j);break;
        };
        j=j*16;
    };
  return k;
};
//------------------------------------------------------------------------------
DWORD HTDF(char *src, unsigned int nb) //pour conversion en hexa
{
 DWORD k=0;//resultat
 DWORD j=1,i; // j =coef multiplicateur et i variable de boucle

    for (i=nb;i>0;i--)//calcul de la valeur hexa en decimal
    {
        switch(src[i-1])
        {
         case '1':k=k+j;break;
         case '2':k=k+(2*j);break;
         case '3':k=k+(3*j);break;
         case '4':k=k+(4*j);break;
         case '5':k=k+(5*j);break;
         case '6':k=k+(6*j);break;
         case '7':k=k+(7*j);break;
         case '8':k=k+(8*j);break;
         case '9':k=k+(9*j);break;
         case 'a':k=k+(10*j);break;
         case 'A':k=k+(10*j);break;
         case 'b':k=k+(11*j);break;
         case 'B':k=k+(11*j);break;
         case 'c':k=k+(12*j);break;
         case 'C':k=k+(12*j);break;
         case 'd':k=k+(13*j);break;
         case 'D':k=k+(13*j);break;
         case 'e':k=k+(14*j);break;
         case 'E':k=k+(14*j);break;
         case 'f':k=k+(15*j);break;
         case 'F':k=k+(15*j);break;
        };
        j=j*16;
    };
  return k;
};
//---------------------------------------------------------------------------------------------------------------
char HexaToDecS(char *hexa)
{
  char k=0;
  char j=16;
  switch(hexa[0])
  {
   case '1':k=k+j;break;
   case '2':k=k+(2*j);break;
   case '3':k=k+(3*j);break;
   case '4':k=k+(4*j);break;
   case '5':k=k+(5*j);break;
   case '6':k=k+(6*j);break;
   case '7':k=k+(7*j);break;
   case '8':k=k+(8*j);break;
   case '9':k=k+(9*j);break;
   case 'a':k=k+(10*j);break;
   case 'A':k=k+(10*j);break;
   case 'b':k=k+(11*j);break;
   case 'B':k=k+(11*j);break;
   case 'c':k=k+(12*j);break;
   case 'C':k=k+(12*j);break;
   case 'd':k=k+(13*j);break;
   case 'D':k=k+(13*j);break;
   case 'e':k=k+(14*j);break;
   case 'E':k=k+(14*j);break;
   case 'f':k=k+(15*j);break;
   case 'F':k=k+(15*j);break;
  };
  j=1;
  switch(hexa[1])
  {
   case '1':k=k+j;break;
   case '2':k=k+(2*j);break;
   case '3':k=k+(3*j);break;
   case '4':k=k+(4*j);break;
   case '5':k=k+(5*j);break;
   case '6':k=k+(6*j);break;
   case '7':k=k+(7*j);break;
   case '8':k=k+(8*j);break;
   case '9':k=k+(9*j);break;
   case 'a':k=k+(10*j);break;
   case 'A':k=k+(10*j);break;
   case 'b':k=k+(11*j);break;
   case 'B':k=k+(11*j);break;
   case 'c':k=k+(12*j);break;
   case 'C':k=k+(12*j);break;
   case 'd':k=k+(13*j);break;
   case 'D':k=k+(13*j);break;
   case 'e':k=k+(14*j);break;
   case 'E':k=k+(14*j);break;
   case 'f':k=k+(15*j);break;
   case 'F':k=k+(15*j);break;
  };
  return k;
};
//------------------------------------------------------------------------------
//conversion de chaine Wildstring stocké sous forme hexa en char
char *SHexaToString(char *src, char *dst, unsigned int dst_size_max)
{
  unsigned int i,j=0,size = strlen(src);
  char *d = dst;
  for (i=0;(i<size) && (j<dst_size_max);i+=4,j++)
  {
    if ((src[i]=='1') && (src[i+1]=='9') && (src[i+2]=='2') && (src[i+3]=='0'))*d++ = '\'';
    else *d++ = HexaToDecS(&src[i]);
  }
  *d =0;
  return dst;
}
//------------------------------------------------------------------------------
//V1 XP/2003
void DecodeSAMHashXP(char *sk,char *datas_hs, int rid, char *user, BYTE *b_f)
{
  if (b_f != NULL)
  {
    //chargement des fonctions pour l'exploitation DES
    if (sf25 == NULL || sf27 == NULL)return;

    //transcodage en byte de la sysley
    BYTE b_sk[16];
    unsigned int i, ref;
    for (i=0;i<16;i++)
    {
      b_sk[i] = (BYTE)HexaToDecS(&sk[i*2]);
    }

    //découpage codage en byte des hashs
    BYTE b_LM[16]="",b_NT[16]="";

    //LM
    if (datas_hs[0] == '<')ref =18;
    else
    {
      for (i=0;i<16;i++)
      {
        b_LM[i] = (BYTE)HexaToDecS(&datas_hs[i*2+1]); // +1 = on passe le : de début ^^
      }
      ref = 34;
    }

    //NT
    if (datas_hs[ref] != '<')
    {
      for (i=0;i<16;i++)
      {
        b_NT[i] = (BYTE)HexaToDecS(&datas_hs[i*2+ref]);
      }
    }

    //init pour le déchiffrement ^^
    unsigned char aqwerty[] = "!@#$%^&*()qwertyUIOPAzxcvbnmQQQQQQQQQQQQ)(*@&%";
    unsigned char anum[] = "0123456789012345678901234567890123456789";
    unsigned char almpassword[] = "LMPASSWORD";
    unsigned char antpassword[] = "NTPASSWORD";

    MD5_CTX md5c;
    unsigned char md5hash[0x10];
    RC4_KEY rc4k;
    unsigned char hbootkey[0x20];
    unsigned char obfkey[0x10];
    unsigned char fb[0x10];
    int j;
    char tmp[11],result[MAX_PATH];

    //init de la clé pour exploitation
    MD5_Init( &md5c );
    MD5_Update( &md5c, &b_f[0x70], 0x10 );
    MD5_Update( &md5c, aqwerty, 0x2f );
    MD5_Update( &md5c, b_sk, 0x10 );
    MD5_Update( &md5c, anum, 0x29 );
    MD5_Final( md5hash, &md5c );
    RC4_set_key( &rc4k, 0x10, md5hash );
    RC4( &rc4k, 0x20, &b_f[0x80], hbootkey );

    //traitement des données ^^
    sprintf(datas_hs,"%s:%d:",user,rid);
    if (b_LM[0]!= 0)
    {
      //décodage ^^


      MD5_Init( &md5c );
      MD5_Update( &md5c, hbootkey, 0x10 );
      MD5_Update( &md5c, &rid, 0x4 );
      MD5_Update( &md5c, almpassword, 0xb );
      MD5_Final( md5hash, &md5c );

      RC4_set_key( &rc4k, 0x10, md5hash );
      RC4( &rc4k, 0x10, b_LM, obfkey );

      sf25( obfkey, (int*)&rid, fb );

      //transformation des données en hexa ^^
      result[0]=0;
      for (j=0;j<0x10;j++)
      {
        snprintf(result+strlen(result),MAX_LINE_SIZE-strlen(result),"%.2X",fb[j]);
      }

      if (!strcmp(result,"AAD3B435B51404EEAAD3B435B51404EE"))
        strncat(datas_hs,"NO PASSWORD*********************:\0",MAX_LINE_SIZE);
      else
      {
        strncat(datas_hs,result,MAX_LINE_SIZE);
        strncat(datas_hs,":\0",MAX_LINE_SIZE);
      }
    }else strncat(datas_hs,"NO PASSWORD*********************:\0",MAX_LINE_SIZE);

    if (b_NT[0]!= 0)
    {
      MD5_Init( &md5c );
      MD5_Update( &md5c, hbootkey, 0x10 );
      MD5_Update( &md5c, &rid, 0x4 );
      MD5_Update( &md5c, antpassword, 0xb );
      MD5_Final( md5hash, &md5c );

      RC4_set_key( &rc4k, 0x10, md5hash );
      RC4( &rc4k, 0x10,b_NT, obfkey );

      sf27( obfkey, (int*)&rid, fb );

      result[0]=0;
      for (j=0;j<0x10;j++)
      {
        snprintf(tmp,10,"%.2X",fb[j]);
        strncat(result,tmp,MAX_PATH-strlen(result));
      }
      strncat(result,"\0",MAX_PATH-strlen(result));

      if (!strcmp(result,"31D6CFE0D16AE931B73C59D7E0C089C0"))
        strncat(datas_hs,"NO PASSWORD*********************\0",MAX_LINE_SIZE-strlen(datas_hs));
      else strncat(datas_hs,result,MAX_LINE_SIZE-strlen(datas_hs));

      strncat(datas_hs,":::\0",MAX_LINE_SIZE-strlen(datas_hs));
    }else strncat(datas_hs,"NO PASSWORD*********************:::\0",MAX_LINE_SIZE-strlen(datas_hs));
  }
}
//------------------------------------------------------------------------------
BOOL TestUserDataFromSAM_V(USERS_INFOS *User_infos, char *buffer, char *computer)
{
  //init
  User_infos->name[0]                  = 0;
  User_infos->RID[0]                   = 0;
  User_infos->SID[0]                   = 0;
  User_infos->group[0]                 = 0;
  User_infos->type[0]                  = 0;
  User_infos->description[0]           = 0;
  User_infos->pwdump_pwd_raw_format[0] = 0;
  User_infos->pwdump_pwd_format[0]     = 0;

  //get datas
  BOOL ret = FALSE;
  char tmp[MAX_PATH],tmp2[MAX_PATH],tmp3[MAX_PATH];
  unsigned long int size_total = strlen(buffer);
  if (size_total < 350)return FALSE;

  //possibilité aussi de chercher :
  //chercher dans la chaine la chaine suivante : 000001020000000520000000200200000102000000052000000020020000
  //+Nom(Wildstring) + 0000 + Description + 0100/0102/FFFF

//--name
  //emplacement du nom (taille de la strcuturitem[10].c[0]=0;e d'entête + emplacement)
  //0x0C
  tmp[0] = buffer[30];
  tmp[1] = buffer[31];
  tmp[2] = buffer[28];
  tmp[3] = buffer[29];
  tmp[4] = buffer[26];
  tmp[5] = buffer[27];
  tmp[6] = buffer[24];
  tmp[7] = buffer[25];
  unsigned int of_name = (204+ HTDF(tmp,8))*2;
  //lecture de la taille du nom sur 1 int  = 4octets
  //0x10 = taille nom user
  tmp[0] = buffer[38];
  tmp[1] = buffer[39];
  tmp[2] = buffer[36];
  tmp[3] = buffer[37];
  tmp[4] = buffer[34];
  tmp[5] = buffer[35];
  tmp[6] = buffer[32];
  tmp[7] = buffer[33];
  unsigned int taille_nom = HTDF(tmp,8)/2;

//-- nom complet
  //emplacement de la description (taille de la strcuture d'entête + emplacement)
  //0x18
  tmp[0] = buffer[54];
  tmp[1] = buffer[55];
  tmp[2] = buffer[52];
  tmp[3] = buffer[53];
  tmp[4] = buffer[50];
  tmp[5] = buffer[51];
  tmp[6] = buffer[48];
  tmp[7] = buffer[49];
  unsigned int of_full_name = (204+ HTDF(tmp,8))*2;
  //lecture de la taille du nom complet sur 1 int  = 4octets
  //0x1C = taille du nom complet
  tmp[0] = buffer[62];
  tmp[1] = buffer[63];
  tmp[2] = buffer[60];
  tmp[3] = buffer[61];
  tmp[4] = buffer[58];
  tmp[5] = buffer[59];
  tmp[6] = buffer[56];
  tmp[7] = buffer[57];
  unsigned int taille_full_name = HTDF(tmp,8)/2;

//--description
  //emplacement de la description (taille de la strcuture d'entête + emplacement)
  //0x24
  tmp[0] = buffer[78];
  tmp[1] = buffer[79];
  tmp[2] = buffer[76];
  tmp[3] = buffer[77];
  tmp[4] = buffer[74];
  tmp[5] = buffer[75];
  tmp[6] = buffer[72];
  tmp[7] = buffer[73];
  unsigned int of_description = (204+ HTDF(tmp,8))*2;
  //lecture de la taille de la description sur 1 int  = 4octets
  //0x28 = taille de description
  tmp[0] = buffer[86];
  tmp[1] = buffer[87];
  tmp[2] = buffer[84];
  tmp[3] = buffer[85];
  tmp[4] = buffer[82];
  tmp[5] = buffer[83];
  tmp[6] = buffer[80];
  tmp[7] = buffer[81];
  unsigned int taille_description = HTDF(tmp,8)/2;

//password hash
//-- LM PASSWORD
  //0x9C
  tmp[0] = buffer[318];
  tmp[1] = buffer[319];
  tmp[2] = buffer[316];
  tmp[3] = buffer[317];
  tmp[4] = buffer[314];
  tmp[5] = buffer[315];
  tmp[6] = buffer[312];
  tmp[7] = buffer[313];
  unsigned int of_lmpw = (204+ HTDF(tmp,8))*2;
  //0xA0
  tmp[0] = buffer[326];
  tmp[1] = buffer[327];
  tmp[2] = buffer[324];
  tmp[3] = buffer[325];
  tmp[4] = buffer[322];
  tmp[5] = buffer[323];
  tmp[6] = buffer[320];
  tmp[7] = buffer[321];
  unsigned int taille_lmpw = HTDF(tmp,8)*2;

//-- NT PASSWORD
  //0xA8
  tmp[0] = buffer[342];
  tmp[1] = buffer[343];
  tmp[2] = buffer[340];
  tmp[3] = buffer[341];
  tmp[4] = buffer[338];
  tmp[5] = buffer[339];
  tmp[6] = buffer[336];
  tmp[7] = buffer[337];
  unsigned int of_ntpw = (204+ HTDF(tmp,8))*2;
  //0xAC
  tmp[0] = buffer[350];
  tmp[1] = buffer[351];
  tmp[2] = buffer[348];
  tmp[3] = buffer[349];
  tmp[4] = buffer[346];
  tmp[5] = buffer[347];
  tmp[6] = buffer[344];
  tmp[7] = buffer[345];
  unsigned int taille_ntpw = HTDF(tmp,8)*2;

  //---results---
  //name
  if ((taille_nom>0) && (taille_nom<size_total) && (of_name>0) && (of_name<size_total))
  {
    tmp[0] = 0;
    tmp2[0] = 0;
    strncpy(tmp,(char*)(buffer+of_name),MAX_PATH);
    SHexaToString(tmp,tmp2,MAX_PATH);
    if (taille_nom<MAX_PATH)tmp2[taille_nom]=0;
    else tmp2[MAX_PATH-1]=0;

    if (computer[0] == 0)strncpy(User_infos->name,tmp2,MAX_PATH);
    else snprintf(User_infos->name,MAX_PATH,"%s\\%s",computer,tmp2);
    ret = TRUE;
  }
  //lecture de la description (fullname)
  if ((taille_full_name>0) && (taille_full_name<size_total) && (of_full_name>0) && (of_full_name<size_total))
  {
    tmp[0]  = 0;
    tmp2[0] = 0;
    strncpy(tmp,(char*)(buffer+of_full_name),MAX_PATH);
    SHexaToString(tmp,tmp2,MAX_PATH);
    if (taille_full_name<MAX_PATH)tmp2[taille_full_name]=0;
    else tmp2[MAX_PATH-1]=0;
    ret = TRUE;
  }else tmp2[0] = 0;

  //lecture de la description (comment)
  if ((taille_description>0) && (taille_description<size_total) && (of_description>0) && (of_description<size_total))
  {
    tmp[0]  = 0;
    tmp3[0] = 0;
    strncpy(tmp,(char*)(buffer+of_description),MAX_PATH);
    SHexaToString(tmp,tmp3,MAX_PATH);
    if (taille_description<MAX_PATH)tmp3[taille_description]=0;
    else tmp3[MAX_PATH-1]=0;

    if (tmp2[0] != 0)snprintf(User_infos->description,MAX_PATH,"(%s) %s",tmp2,tmp3);
    else snprintf(User_infos->description,MAX_PATH,"%s",tmp3);
    ret = TRUE;
  }else if ((taille_full_name>0) && (tmp2[0] != 0)) snprintf(User_infos->description,MAX_PATH,"(%s)",tmp2);

  //type
  if (((buffer[8]=='B') || (buffer[8]=='b')) && ((buffer[9]=='C') || (buffer[9]=='c')))snprintf(User_infos->type,MAX_PATH,"2 : %s",cps[TXT_MSG_ADMIN].c);
  else if (((buffer[8]=='B') || (buffer[8]=='b')) && (buffer[9]=='0'))snprintf(User_infos->type,MAX_PATH,"0 : %s",cps[TXT_MSG_GUEST].c);
  else if (((buffer[8]=='D') || (buffer[8]=='d')) && (buffer[9]=='4'))snprintf(User_infos->type,MAX_PATH,"1 : %s",cps[TXT_MSG_USER].c);
  else snprintf(User_infos->type,MAX_PATH,"0x%c%c : %s",buffer[8],buffer[9],cps[TXT_MSG_UNK].c);

  //SID+RID
  //SID : après 12 octets donc 24 caractères + dernière clée : 2o donc 4caractères : 2400 4400 0200 0105 0000 0000 0005 1500 0000
  //fin : 0000
  //SID = col3
  tmp3[0] = 0;
  unsigned int type_id = 0, type_id2=0, last_id=0;
  unsigned long int i = Contient(buffer,"2400440002000105000000000005"); // 1500 0000 = 21 le SID de début
  if ((i>0) && (i<(strlen(buffer)-40)))
  {
    //création du SID : 4o-4o-4o-4o-4o
    sprintf(tmp,"%c%c%c%c%c%c%c%c",buffer[i+6],buffer[i+7],buffer[i+4],buffer[i+5],buffer[i+2],buffer[i+3],buffer[i],buffer[i+1]);
    type_id = HTD(tmp);
    snprintf(tmp2,MAX_PATH,"S-1-5-%lu",type_id);
    snprintf(tmp3,MAX_PATH,"%s",tmp2);

    sprintf(tmp,"%c%c%c%c%c%c%c%c",buffer[i+14],buffer[i+15],buffer[i+12],buffer[i+13],buffer[i+10],buffer[i+11],buffer[i+8],buffer[i+9]);
    type_id2 = HTD(tmp);
    snprintf(tmp2,MAX_PATH,"-%lu",type_id2);
    strncat(tmp3,tmp2,MAX_PATH-strlen(tmp3));

    sprintf(tmp,"%c%c%c%c%c%c%c%c",buffer[i+22],buffer[i+23],buffer[i+20],buffer[i+21],buffer[i+18],buffer[i+19],buffer[i+16],buffer[i+17]);
    snprintf(tmp2,MAX_PATH,"-%lu",HTD(tmp));
    strncat(tmp3,tmp2,MAX_PATH-strlen(tmp3));

    sprintf(tmp,"%c%c%c%c%c%c%c%c",buffer[i+30],buffer[i+31],buffer[i+28],buffer[i+29],buffer[i+26],buffer[i+27],buffer[i+24],buffer[i+25]);
    snprintf(tmp2,MAX_PATH,"-%lu",HTD(tmp));
    strncat(tmp3,tmp2,MAX_PATH-strlen(tmp3));

    sprintf(tmp,"%c%c%c%c%c%c%c%c",buffer[i+38],buffer[i+39],buffer[i+36],buffer[i+37],buffer[i+34],buffer[i+35],buffer[i+32],buffer[i+33]);
    last_id = HTD(tmp);

    snprintf(User_infos->SID,MAX_PATH,"%s-%lu",tmp3,last_id);
    snprintf(User_infos->RID,MAX_PATH,"%05lu",last_id);

    //descriptions infos +
    switch (type_id)
    {
      case 1: strncat(User_infos->description," Rights : Dialup\0",MAX_PATH);
      case 2: strncat(User_infos->description," Rights : Network\0",MAX_PATH);
      case 3: strncat(User_infos->description," Rights : Batch\0",MAX_PATH);
      case 4: strncat(User_infos->description," Rights : Interative\0",MAX_PATH);
      case 5: strncat(User_infos->description," Rights : Driver\0",MAX_PATH);
      case 6: strncat(User_infos->description," Rights : Service\0",MAX_PATH);
      case 7: strncat(User_infos->description," Rights : Anonymous logon\0",MAX_PATH);
      case 8: strncat(User_infos->description," Rights : Proxy\0",MAX_PATH);
      case 9: strncat(User_infos->description," Rights : Entreprise domain controllers\0",MAX_PATH);
      case 10: strncat(User_infos->description," Rights : Self\0",MAX_PATH);
      case 11: strncat(User_infos->description," Rights : Authenticated Users\0",MAX_PATH);
      case 12: strncat(User_infos->description," Rights : Restricted\0",MAX_PATH);
      case 13: strncat(User_infos->description," Rights : Terminal server user\0",MAX_PATH);
      case 14: strncat(User_infos->description," Rights : Remote interactive logon\0",MAX_PATH);
      case 15: strncat(User_infos->description," Rights : This Organization\0",MAX_PATH);
      case 18: strncat(User_infos->description," Rights : System\0",MAX_PATH);
      case 19: strncat(User_infos->description," Rights : Local service\0",MAX_PATH);
      case 20: strncat(User_infos->description," Rights : Network service\0",MAX_PATH);
      case 21:
        switch(last_id)
        {
          case 500 : strncat(User_infos->description," Rights : Local Administrator\0",MAX_PATH);break;
          case 501 : strncat(User_infos->description," Rights : Local Guest\0",MAX_PATH);break;
          case 502 : strncat(User_infos->description," Rights : Krbtgt\0",MAX_PATH);break;
          case 512 : strncat(User_infos->description," Rights : Domain Admins\0",MAX_PATH);break;
          case 513 : strncat(User_infos->description," Rights : Domain Users\0",MAX_PATH);break;
          case 514 : strncat(User_infos->description," Rights : Domain Guests\0",MAX_PATH);break;
          case 515 : strncat(User_infos->description," Rights : Domain Computers\0",MAX_PATH);break;
          case 516 : strncat(User_infos->description," Rights : Domain Controllers\0",MAX_PATH);break;
          case 517 : strncat(User_infos->description," Rights : Cert Publishers\0",MAX_PATH);break;
          case 518 : strncat(User_infos->description," Rights : Schema Admins\0",MAX_PATH);break;
          case 519 : strncat(User_infos->description," Rights : Enterprise Admins\0",MAX_PATH);break;
          case 520 : strncat(User_infos->description," Rights : Group Policy Creator Owners\0",MAX_PATH);break;
          case 553 : strncat(User_infos->description," Rights : RAS and IAS Servers\0",MAX_PATH);break;
        }
      break;
      case 32:
        switch(last_id)
        {
          case 544 : strncat(User_infos->description," Rights : Administrators\0",MAX_PATH);break;
          case 545 : strncat(User_infos->description," Rights : Users\0",MAX_PATH);break;
          case 546 : strncat(User_infos->description," Rights : Guests\0",MAX_PATH);break;
          case 547 : strncat(User_infos->description," Rights : Power Users\0",MAX_PATH);break;
          case 548 : strncat(User_infos->description," Rights : Account Operators\0",MAX_PATH);break;
          case 549 : strncat(User_infos->description," Rights : Server Operators\0",MAX_PATH);break;
          case 550 : strncat(User_infos->description," Rights : Print Operators\0",MAX_PATH);break;
          case 551 : strncat(User_infos->description," Rights : Backup Operators\0",MAX_PATH);break;
          case 552 : strncat(User_infos->description," Rights : Replicator \0",MAX_PATH);break;
          case 554 : strncat(User_infos->description," Rights : Pre-Windows 2000 Compatible Access\0",MAX_PATH);break;
          case 555 : strncat(User_infos->description," Rights : Remote Desktop Users\0",MAX_PATH);break;
          case 556 : strncat(User_infos->description," Rights : Network Configuration Operators\0",MAX_PATH);break;
          case 557 : strncat(User_infos->description," Rights : Incoming Forest Trust Builders\0",MAX_PATH);break;
          case 558 : strncat(User_infos->description," Rights : Performance Monitor Users\0",MAX_PATH);break;
          case 559 : strncat(User_infos->description," Rights : Performance Log Users\0",MAX_PATH);break;
          case 560 : strncat(User_infos->description," Rights : Windows Authorization Access Group\0",MAX_PATH);break;
          case 561 : strncat(User_infos->description," Rights : Terminal Server License Servers\0",MAX_PATH);break;
        }
      break;
      case 64:
        switch(last_id)
        {
          case 10 : strncat(User_infos->description," Rights : NTLM Authentication\0",MAX_PATH);break;
          case 14 : strncat(User_infos->description," Rights : SChannel Authentication\0",MAX_PATH);break;
          case 21 : strncat(User_infos->description," Rights : Digest Authentication\0",MAX_PATH);break;
        }
      break;
    }
    ret = TRUE;
  }
  //hash NT::LM
  if (ret)
  {
    tmp2[0]=0;
    tmp3[0]=0;
    //8 => 4 size of separator
    if ((taille_lmpw > 8) && (of_lmpw>0) && ((of_lmpw + 8+ taille_lmpw)<=size_total))
    {
      strncpy(tmp2,buffer+of_lmpw+8,MAX_PATH);
      tmp2[32]=0;
    }else strcpy(tmp2,"NO PASSWORD*********************\0");//LM

    if ((taille_ntpw > 8) && (of_ntpw>0) && ((of_ntpw + taille_ntpw)<=size_total))
    {
      strncpy(tmp3,buffer+(of_ntpw+8),MAX_PATH);
      tmp3[32]=0;
    }else strcpy(tmp3,"NO PASSWORD*********************\0");//NT

    if ((tmp2[0]!=0) && (tmp3[0]!=0))
    {
      //pwdump format
      //<user>:<id>:<lanman pw>:<NT pw>:comment:homedir:
      //snprintf(item[10].c,MAX_PATH,"%s:%lu:%s:%s:::",item[2].c,last_id,tmp2,tmp3);
      snprintf(User_infos->pwdump_pwd_raw_format,MAX_PATH,":%s:%s",tmp2,tmp3);
    }
  }
  return ret;
}
//------------------------------------------------------------------------------
//extraction des données de la valeur F du profil
DWORD TestUserDataFromSAM_F(USERS_INFOS *User_infos, char*buffer)
{
  User_infos->last_logon[0]            = 0;
  User_infos->last_password_change[0]  = 0;
  User_infos->nb_connexion             = 0;
  User_infos->state_id                 = 0;

  DWORD userRID = 0;

  if (strlen(buffer)>0x8F)
  {
    char tmp[MAX_PATH];

    //Offset 0008 : date de dernière connexion
    FILETIME FileTime;

    tmp[0] = buffer[0x16];
    tmp[1] = buffer[0x17];
    tmp[2] = buffer[0x14];
    tmp[3] = buffer[0x15];
    tmp[4] = buffer[0x12];
    tmp[5] = buffer[0x13];
    tmp[6] = buffer[0x10];
    tmp[7] = buffer[0x11];
    FileTime.dwLowDateTime = HTDF(tmp,8);

    tmp[0] = buffer[0x1E];
    tmp[1] = buffer[0x1F];
    tmp[2] = buffer[0x1C];
    tmp[3] = buffer[0x1D];
    tmp[4] = buffer[0x1A];
    tmp[5] = buffer[0x1B];
    tmp[6] = buffer[0x18];
    tmp[7] = buffer[0x19];
    FileTime.dwHighDateTime = HTDF(tmp,8);
    if ((FileTime.dwHighDateTime == 0) && (FileTime.dwLowDateTime == 0))strncpy(User_infos->last_logon,cps[TXT_MSG_NEVER].c,DATE_SIZE_MAX);
    else
    {
      filetimeToString_GMT(FileTime, User_infos->last_logon, DATE_SIZE_MAX);
      if (User_infos->last_logon[0] == 0)strncpy(User_infos->last_logon,cps[TXT_MSG_NEVER].c,DATE_SIZE_MAX);
    }

    //Last password change
    tmp[0] = buffer[0x36];
    tmp[1] = buffer[0x37];
    tmp[2] = buffer[0x34];
    tmp[3] = buffer[0x35];
    tmp[4] = buffer[0x32];
    tmp[5] = buffer[0x33];
    tmp[6] = buffer[0x30];
    tmp[7] = buffer[0x31];
    FileTime.dwLowDateTime = HTDF(tmp,8);

    tmp[0] = buffer[0x3E];
    tmp[1] = buffer[0x3F];
    tmp[2] = buffer[0x3C];
    tmp[3] = buffer[0x3D];
    tmp[4] = buffer[0x3A];
    tmp[5] = buffer[0x3B];
    tmp[6] = buffer[0x38];
    tmp[7] = buffer[0x39];
    FileTime.dwHighDateTime = HTDF(tmp,8);
    if ((FileTime.dwHighDateTime == 0) && (FileTime.dwLowDateTime == 0))strncpy(User_infos->last_password_change,cps[TXT_MSG_NEVER].c,DATE_SIZE_MAX);
    else
    {
      filetimeToString_GMT(FileTime, User_infos->last_password_change, DATE_SIZE_MAX);
      if (User_infos->last_password_change[0] == 0)strncpy(User_infos->last_password_change,cps[TXT_MSG_NEVER].c,DATE_SIZE_MAX);
    }

    //Offste 0038, second digit = State
    if (buffer[0x71]=='0')User_infos->state_id=301;
    else User_infos->state_id=300;

    //Password Expire
    if ((buffer[0x73]=='2')||(buffer[0x73]=='3')||(buffer[0x73]=='6')||(buffer[0x73]=='7')||(buffer[0x73]=='A')||(buffer[0x73]=='B')||(buffer[0x73]=='E')||(buffer[0x73]=='F'))
    {
      strncat(User_infos->last_password_change," (",MAX_PATH);
      strncat(User_infos->last_password_change,cps[TXT_MSG_MDP_NEVER_EXP].c,MAX_PATH);
      strncat(User_infos->last_password_change,")\0",MAX_PATH);
    }

    //nombre de connexion 0x42
    tmp[0] = buffer[0x86];
    tmp[1] = buffer[0x87];
    tmp[2] = buffer[0x84];
    tmp[3] = buffer[0x85];
    User_infos->nb_connexion = HTDF(tmp,4);

    //Offset 0030 : user RID
    tmp[0] = buffer[0x66];
    tmp[1] = buffer[0x67];
    tmp[2] = buffer[0x64];
    tmp[3] = buffer[0x65];
    tmp[4] = buffer[0x62];
    tmp[5] = buffer[0x63];
    tmp[6] = buffer[0x60];
    tmp[7] = buffer[0x61];
    userRID = HTDF(tmp,8);
  }

  return userRID;
}
//------------------------------------------------------------------------------
//extraction des données de la valeur C de groupe
//------------------------------------------------------------------------------
//SAM\SAM\Domains\Builtin\Aliases\00000220 le C

//0x10 = emplacement nom de groupe en unicode (taille totale) + 52
//0x14 = taille nom de groupe

//0x1C = emplacement description + 52
//0x20 = taille description en unicode (taille totale)

//0x28 = début du 1er SID structure + 52
//0x30 = nombre d'instances (user + paddings)
void TraiterGroupDataFromSAM_C(char *buffer, unsigned int rid, char *group, unsigned int group_size_max)
{
  char tmp[MAX_LINE_SIZE];
  unsigned long int size_total = strlen(buffer);

  char group_name[MAX_PATH]="";

  //lecture de la taille du nom sur 1 int  = 4octets
  //0x14 = taille nom de groupe
  tmp[0] = buffer[46];
  tmp[1] = buffer[47];
  tmp[2] = buffer[44];
  tmp[3] = buffer[45];
  tmp[4] = buffer[42];
  tmp[5] = buffer[43];
  tmp[6] = buffer[40];
  tmp[7] = buffer[41];
  unsigned int taille_nom = HTDF(tmp,8)/2;

  //lecture de la taille de la description sur 1 int  = 4octets
  //0x20 = taille de description
  /*tmp[0] = buffer[70];
  tmp[1] = buffer[71];
  tmp[2] = buffer[68];
  tmp[3] = buffer[69];
  tmp[4] = buffer[66];
  tmp[5] = buffer[67];
  tmp[6] = buffer[64];
  tmp[7] = buffer[65];
  unsigned int taille_description = HTDF(tmp,8)/2;*/

  //0x10 = emplacement nom de groupe en unicode (taille totale) + 52
  tmp[0] = buffer[38];
  tmp[1] = buffer[39];
  tmp[2] = buffer[36];
  tmp[3] = buffer[37];
  tmp[4] = buffer[34];
  tmp[5] = buffer[35];
  tmp[6] = buffer[32];
  tmp[7] = buffer[33];
  unsigned int of_name = (52+ HTDF(tmp,8))*2;

  //0x1C = emplacement description + 52
  /*tmp[0] = buffer[62];
  tmp[1] = buffer[63];
  tmp[2] = buffer[60];
  tmp[3] = buffer[61];
  tmp[4] = buffer[58];
  tmp[5] = buffer[59];
  tmp[6] = buffer[56];
  tmp[7] = buffer[57];
  unsigned int of_description = (52+ HTDF(tmp,8))*2;*/

  //0x28 = début du 1er SID structure + 52
  tmp[0] = buffer[86];
  tmp[1] = buffer[87];
  tmp[2] = buffer[84];
  tmp[3] = buffer[85];
  tmp[4] = buffer[82];
  tmp[5] = buffer[83];
  tmp[6] = buffer[80];
  tmp[7] = buffer[81];
  unsigned int of_sid = (52+ HTDF(tmp,8))*2;

  //0x30 = nombre d'instances (user)
  tmp[0] = buffer[102];
  tmp[1] = buffer[103];
  tmp[2] = buffer[100];
  tmp[3] = buffer[101];
  tmp[4] = buffer[98];
  tmp[5] = buffer[99];
  tmp[6] = buffer[96];
  tmp[7] = buffer[97];
  DWORD nb_sid = HTDF(tmp,8);

  //2 = name
  if ((taille_nom>0) && (taille_nom<size_total) && (of_name>0) && (of_name<size_total))
  {
    strncpy(tmp,(char*)(buffer+of_name),MAX_LINE_SIZE);
    SHexaToString(tmp,group_name,MAX_PATH);
    if (taille_nom<MAX_PATH)group_name[taille_nom]=0;
    else group_name[MAX_PATH-1]=0;
  }

  //4 = description
  /*if (taille_description>0 && taille_description<size_total && of_description>0 && of_description<size_total)
  {
    tmp[0]=0;
    strncpy(tmp,(char*)(item[12].c+of_description),MAX_LINE_SIZE);
    SHexaToString(tmp,item[4].c);
    item[4].c[taille_description]=0;
  }*/

  //3 = sid
  #define DATA_USER_HEADER_SIZE  8
  /*typedef struct data_user
  {
    char valid;       //0x01
    char nb_sid;      //0x05 ou 0x01
    char padding[5];  //0x0000 0000 00
    char type;        //0x05
    char sid[1];      //par defaut découpé en section de 2octet *nb_part
  }DATA_USER;
  */

  //cas de groupes vides
  if (of_sid+(DATA_USER_HEADER_SIZE+nb_sid*4)*2>size_total)return;

  DWORD i, nb_c_sid;
  char *d,*c = &buffer[of_sid];
  for (i=0;i<nb_sid;i++)
  {
    //lecture du nombre de sid
    nb_c_sid = HexaToDecS(c+2);
    if (nb_c_sid != 1)
    {
      d = c+(DATA_USER_HEADER_SIZE*2)+(nb_c_sid-1)*8; // 48 = 6*8
      tmp[0] = d[6];
      tmp[1] = d[7];
      tmp[2] = d[4];
      tmp[3] = d[5];
      tmp[4] = d[2];
      tmp[5] = d[3];
      tmp[6] = d[0];
      tmp[7] = d[1];

      if(HTDF(tmp,8) == rid)
      {
        if (group[0] != 0)strncat(group,", ",group_size_max);
        strncat(group,group_name,group_size_max);
        strncat(group,"\0",group_size_max);
        return;
      }
    }

    //next
    c = c+((DATA_USER_HEADER_SIZE+nb_c_sid*4)*2);
  }
}
//------------------------------------------------------------------------------
void GetUserGroupInfos(unsigned int rid, char *group, unsigned int group_size_max, char *reg_path)
{
  int ret = set_sam_tree_access(HKEY_LOCAL_MACHINE,reg_path);
  if (ret == 0)
  {
    //read all groups
    HKEY CleTmp;
    if (RegOpenKey(HKEY_LOCAL_MACHINE,reg_path,&CleTmp)==ERROR_SUCCESS)
    {
      DWORD i,nbSubKey = 0, SubKeysize, valueSize;
      char NomSubKey[MAX_PATH], path[MAX_PATH],buffer[MAX_LINE_SIZE],cbuffer[MAX_LINE_SIZE];
      RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0);
      for(i=0;i<nbSubKey;i++)
      {
        NomSubKey[0]=0;
        SubKeysize = MAX_PATH;
        if (RegEnumKeyEx(CleTmp,i,NomSubKey,&SubKeysize,0,0,0,0)==ERROR_SUCCESS)
        {
          //génération du path group
          buffer[0] = 0;
          cbuffer[0] = 0;
          snprintf(path,MAX_PATH,"%s\\%s",reg_path,NomSubKey);
          if((valueSize = ReadValue(HKEY_LOCAL_MACHINE, path, "C", buffer, MAX_LINE_SIZE)))
          {
            DataToHexaChar(buffer, valueSize, cbuffer, MAX_LINE_SIZE);
            TraiterGroupDataFromSAM_C(cbuffer, rid,  group,  group_size_max);
          }
        }
      }
      RegCloseKey(CleTmp);
    }
  }
  restore_sam_tree_access(HKEY_LOCAL_MACHINE,reg_path);
}

//------------------------------------------------------------------------------
void GetUserGroup(unsigned int rid, char *group, unsigned int group_size_max)
{
  //first the services group
  group[0] = 0;
  //account group
  GetUserGroupInfos(rid, group, group_size_max,"SECURITY\\SAM\\Domains\\Builtin\\Aliases");
  //service group
  GetUserGroupInfos(rid, group, group_size_max,"SECURITY\\SAM\\Domains\\Account\\Aliases");
}
//------------------------------------------------------------------------------
BOOL registry_users_extract(sqlite3 *db, unsigned int session_id)
{
  BOOL ok = FALSE;
  DWORD userRID;
  USERS_INFOS User_infos;

  //import syskey
  char sk[MAX_PATH]="";
  BOOL syskeyok = registry_syskey_local(sk, MAX_PATH);

  //get current computer name
  char computer_name[COMPUTER_NAME_SIZE_MAX]="";
  DWORD taille = COMPUTER_NAME_SIZE_MAX;
  GetComputerName(computer_name,&taille);

  //Set ACL in registry
  int ret = set_sam_tree_access(HKEY_LOCAL_MACHINE,"SECURITY\\SAM\\Domains\\Account\\Users");
  if (ret == 0)
  {
    //get reference key for users hashs
    HKEY CleTmp;
    BOOL ok_test;
    BYTE b_f[MAX_LINE_SIZE];
    if(ReadValue(HKEY_LOCAL_MACHINE, "SAM\\SAM\\Domains\\Account", "F", (char*)b_f, MAX_LINE_SIZE)<= 0x80)b_f[0] = 0;

    //get users
    if (RegOpenKey(HKEY_LOCAL_MACHINE,"SAM\\SAM\\Domains\\Account\\Users\\",&CleTmp)==ERROR_SUCCESS)
    {
      DWORD i, valueSize, SizeSubKeyName, nbSubKey = 0;
      if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
      {
        char SubKeyName[MAX_PATH], path[MAX_PATH], buffer[MAX_LINE_SIZE], cbuffer[MAX_LINE_SIZE];

        for (i=0;i<nbSubKey;i++)
        {
          ok_test       = FALSE;
          SizeSubKeyName=MAX_PATH;// on reinitialise la taille a chaque fois sinon il ne lit pas la valeur suivant
          SubKeyName[0] = 0;
          if(RegEnumKeyEx (CleTmp,i,SubKeyName,&SizeSubKeyName,0,0,0,0)!=ERROR_SUCCESS)continue;

          //read value
          snprintf(path,MAX_PATH,"SAM\\SAM\\Domains\\Account\\Users\\%s",SubKeyName);

          //F value
          cbuffer[0]= 0;
          buffer[0] = 0;
          userRID   = 0;
          valueSize = ReadValue(HKEY_LOCAL_MACHINE, path, "F", buffer, MAX_LINE_SIZE);
          if (valueSize)
          {
            DataToHexaChar(buffer, valueSize, cbuffer, MAX_LINE_SIZE);
            userRID = TestUserDataFromSAM_F(&User_infos,cbuffer);
            ok_test = TRUE;
         }

          //V value
          cbuffer[0]= 0;
          buffer[0] = 0;
          valueSize = ReadValue(HKEY_LOCAL_MACHINE, path, "V", buffer, MAX_LINE_SIZE);
          if (valueSize)
          {
            //read datas
            DataToHexaChar(buffer, valueSize, cbuffer, MAX_LINE_SIZE);
            if(TestUserDataFromSAM_V(&User_infos,cbuffer,computer_name))
            {
              //test if rid and sid ok
              userRID = HTDF(SubKeyName,8);
              if(User_infos.RID[0] == 0)snprintf(User_infos.RID,MAX_PATH,"%05lu",userRID);
              if(User_infos.SID[0] == 0)snprintf(User_infos.SID,MAX_PATH,"S-1-5-?-?-?-?-%lu",userRID);
           }else
            {
              if((User_infos.RID[0] == 0) && userRID)snprintf(User_infos.RID,MAX_PATH,"%05lu",userRID);
              if((User_infos.SID[0] == 0) && userRID)snprintf(User_infos.SID,MAX_PATH,"S-1-5-?-?-?-?-%lu",userRID);
            }
            ok_test = TRUE;
          }else
          {
            if((User_infos.RID[0] == 0) && userRID)snprintf(User_infos.RID,MAX_PATH,"%05lu",userRID);
            if((User_infos.SID[0] == 0) && userRID)snprintf(User_infos.SID,MAX_PATH,"S-1-5-?-?-?-?-%lu",userRID);
         }

          if (!ok_test)continue;

          //get groups
          if (userRID) GetUserGroup(userRID, User_infos.group, MAX_PATH);

          //get hashs
          if((b_f[0] != 0) && syskeyok)
          {
            DecodeSAMHashXP(sk,User_infos.pwdump_pwd_raw_format,userRID,User_infos.name,b_f);
          }

          //add user
          convertStringToSQL(User_infos.description, MAX_PATH);
          addRegistryUsertoDB("HKEY_LOCAL_MACHINE\\SAM",User_infos.name, User_infos.RID, User_infos.SID, User_infos.group,
                              User_infos.description, User_infos.last_logon, User_infos.last_password_change,
                              User_infos.nb_connexion, User_infos.type, User_infos.state_id,session_id, db);

          //add password
          if (TEST_REG_PASSWORD_ENABLE)
            addPasswordtoDB("HKEY_LOCAL_MACHINE\\SAM", User_infos.name, User_infos.pwdump_pwd_format, User_infos.pwdump_pwd_raw_format, REG_PASSWORD_STRING_LOCAL_USER, session_id, db);
          ok = TRUE;
        }
      }
      RegCloseKey(CleTmp);
    }
  }

  //Restore ACL in registry
  restore_sam_tree_access(HKEY_LOCAL_MACHINE,"SECURITY\\SAM\\Domains\\Account\\Users");

  return ok;
}

//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
