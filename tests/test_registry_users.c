//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void TraiterGroupDataFromSAM_C(char *buffer, unsigned int rid, char *group, unsigned int group_size_max);
DWORD TestUserDataFromSAM_F(USERS_INFOS *User_infos, char*buffer);
BOOL TestUserDataFromSAM_V(USERS_INFOS *User_infos, char *buffer, char *computer);
DWORD HTDF(char *src, unsigned int nb);
void DecodeSAMHashXP(char *sk,char *datas_hs, int rid, char *user, BYTE *b_f);
void addPasswordtoDB(char *source, char*login, char*password, char*raw_password,unsigned int description_id,unsigned int session_id, sqlite3 *db);
BOOL registry_syskey_file(HK_F_OPEN *hks, char*sk, unsigned int sk_size);
//------------------------------------------------------------------------------
void addRegistryUsertoDB(char *source, char *name, char *RID, char *SID, char *group,
                         char *description, char *last_logon, char *last_password_change,
                         DWORD nb_connexion, char *type, DWORD state_id, DWORD session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_user (source,name,RID,SID,grp,description,last_logon,last_password_change,nb_connexion,type,state_id,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%lu,\"%s\",%lu,%lu);",
           source,name,RID,SID,group,description,last_logon,last_password_change,nb_connexion,type,state_id,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"RegistryUser\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%lu\";\"%s\";\"%lu\";\"%lu\";\r\n",
         source,name,RID,SID,group,description,last_logon,last_password_change,nb_connexion,type,state_id,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void GetSIDFromUser(char *user, char* rid, char *sid, unsigned int max_size)
{
  BYTE BSid[MAX_PATH]="";
  PSID psid       = (PSID)BSid;
  DWORD sid_size  = MAX_PATH;
  SID_NAME_USE TypeSid;

  char domain[MAX_PATH]="";
  DWORD domain_size = MAX_PATH;

  if(LookupAccountName(NULL,(LPCTSTR)user,psid,(LPDWORD)&sid_size,(LPTSTR)domain,(LPDWORD)&domain_size,(PSID_NAME_USE)&TypeSid))
  {
    if (IsValidSid(psid))SidtoUser(psid, user, rid, sid, max_size);
  }
}
//------------------------------------------------------------------------------
//load all users informations
/*typedef struct _USER_INFO_0 {
  LPWSTR usri0_name;
}*LPUSER_INFO_0;

typedef struct _USER_INFO_2 {
  LPWSTR usri2_name;
  LPWSTR usri2_password;
  DWORD  usri2_password_age;
  DWORD  usri2_priv;
  LPWSTR usri2_home_dir;
  LPWSTR usri2_comment;
  DWORD  usri2_flags;
  LPWSTR usri2_script_path;
  DWORD  usri2_auth_flags;
  LPWSTR usri2_full_name;
  LPWSTR usri2_usr_comment;
  LPWSTR usri2_parms;
  LPWSTR usri2_workstations;
  DWORD  usri2_last_logon;
  DWORD  usri2_last_logoff;
  DWORD  usri2_acct_expires;
  DWORD  usri2_max_storage;
  DWORD  usri2_units_per_week;
  PBYTE  usri2_logon_hours;
  DWORD  usri2_bad_pw_count;
  DWORD  usri2_num_logons;
  LPWSTR usri2_logon_server;
  DWORD  usri2_country_code;
  DWORD  usri2_code_page;
} USER_INFO_2, *PUSER_INFO_2, *LPUSER_INFO_2;

typedef struct _GROUP_USERS_INFO_0 {
  LPWSTR grui0_name;
} GROUP_USERS_INFO_0, *PGROUP_USERS_INFO_0, *LPGROUP_USERS_INFO_0;*/
//------------------------------------------------------------------------------
void Scan_registry_user_local(sqlite3 *db, unsigned int session_id)
{
  if (registry_users_extract(db,session_id))return;

  if (MyNetApiBufferFree && MyNetUserEnum && MyNetUserGetInfo && MyNetUserGetLocalGroups)
  {
    //enumerate all accounts
    DWORD nStatus, i, nb, total;
    LPUSER_INFO_0 pBuf, Buffer;
    USER_INFO_2 * pBuf_info;

    char name[MAX_PATH],RID[MAX_PATH],SID[MAX_PATH],group[MAX_PATH],type[MAX_PATH],
    description[MAX_PATH],last_logon[DATE_SIZE_MAX],last_password_change[DATE_SIZE_MAX];
    DWORD nb_connexion,state_id;

    LPGROUP_USERS_INFO_0 LBuffer,pTmpBuf;
    DWORD dwEntriesRead, dwTotalEntries, j, s;

    do
    {
      nStatus = MyNetUserEnum(0,0,2,(LPBYTE*)&pBuf,-1,&nb,&total,0);
      if (((nStatus == 0) || (nStatus == 234)) && (Buffer = pBuf) != 0)
      {
        for (i = 0; i < nb; i++)
        {
          if (Buffer->usri0_name!=0)
          {
            //init
            name[0]                 = 0;
            RID[0]                  = 0;
            SID[0]                  = 0;
            group[0]                = 0;
            description[0]          = 0;
            last_logon[0]           = 0;
            last_password_change[0] = 0;
            nb_connexion            = 0;
            state_id                = 0;
            type[0]                 = 0;

            if (MyNetUserGetInfo(0,Buffer->usri0_name,2,(LPBYTE *)&pBuf_info) == 0)
            {
              //name
              snprintf(name,MAX_PATH,"%S",Buffer->usri0_name);

              //get account infos ^^ SID + RID
              GetSIDFromUser(name, RID, SID, MAX_PATH);

              //group
              if (MyNetUserGetLocalGroups(0,Buffer->usri0_name,0,1,(LPBYTE*)&LBuffer,-1,&dwEntriesRead,&dwTotalEntries) == 0)
              {
                if ((pTmpBuf = LBuffer) != 0)
                {
                  for (j = 0; j < dwEntriesRead; j++)
                  {
                    if(pTmpBuf != 0)
                    {
                      s = strlen(group);
                      if (s>=MAX_PATH)break;

                      snprintf(group+s,MAX_PATH-s,"%S,",pTmpBuf->grui0_name);
                    }
                    pTmpBuf++;
                  }
                }
              }

              //description
              snprintf(description,MAX_PATH,"(%S) %S %S",pBuf_info->usri2_full_name,pBuf_info->usri2_usr_comment,pBuf_info->usri2_comment);

              //state
              if (pBuf_info->usri2_flags&0x2)state_id = 300;//Disable
              else state_id = 301;//Enable
              if (pBuf_info->usri2_flags&0x10)state_id+=2;//Lock

              //last_logon
              if (pBuf_info->usri2_last_logon != 0)
              {
                timeToString(pBuf_info->usri2_last_logon, last_logon, DATE_SIZE_MAX);
              }

              //last_password_change
              if (pBuf_info->usri2_password_age != 0)
              {
                snprintf(last_password_change,DATE_SIZE_MAX,"%lu %02lu:%02lu:%02lu",
                         pBuf_info->usri2_password_age/86400,
                         pBuf_info->usri2_password_age%86400/3600,
                         pBuf_info->usri2_password_age%86400%3600/60,
                         pBuf_info->usri2_password_age%86400%3600%60);
              }

              //nb_connexion
              nb_connexion = pBuf_info->usri2_num_logons;

              //type
              switch(pBuf_info->usri2_priv)
              {
                case 0:snprintf(type,MAX_PATH,"%lu : %s",pBuf_info->usri2_priv,cps[TXT_MSG_GUEST].c);break;
                case 1:snprintf(type,MAX_PATH,"%lu : %s",pBuf_info->usri2_priv,cps[TXT_MSG_USER].c);break;
                case 2:snprintf(type,MAX_PATH,"%lu : %s",pBuf_info->usri2_priv,cps[TXT_MSG_ADMIN].c);break;
                default:snprintf(type,MAX_PATH,"0x%02X : %s",(unsigned int)(pBuf_info->usri2_priv & 0xff),cps[TXT_MSG_UNK].c);break;
              }

              addRegistryUsertoDB("NETAPI32",name, RID, SID, group, description,
                                  last_logon, last_password_change,
                                  nb_connexion, type, state_id, session_id, db);
            }
            Buffer++;
          }else break;
        }
      }
    }while (nStatus == ERROR_MORE_DATA && start_scan);

    MyNetApiBufferFree(&pBuf);
  }
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
int GetRegistryOs(HK_F_OPEN *hks)
{
  char currentOS[MAX_PATH]="";
  if (Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "microsoft\\windows nt\\currentversion", NULL,"ProductName", currentOS, MAX_PATH))
  {
    if (Contient(currentOS,GUIDE_REG_OS_2000) ||
        Contient(currentOS,GUIDE_REG_OS_XP_32b) ||
        Contient(currentOS,GUIDE_REG_OS_2003_32b) ||
        Contient(currentOS,GUIDE_REG_OS_VISTA_32b) ||
        Contient(currentOS,GUIDE_REG_OS_7_32b) ||
        Contient(currentOS,GUIDE_REG_OS_2008_32b) ||
        Contient(currentOS,GUIDE_REG_OS_8_32b))return TRUE;
    else return FALSE;
  }
  return -1;
}
//------------------------------------------------------------------------------
void GetUserGroupFromRegFile(DWORD rid, char *group, DWORD group_size_max, HK_F_OPEN *hks, char *reg_path)
{
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, reg_path);
  if (nk_h == NULL)return;

  HBIN_CELL_NK_HEADER *nk_h_tmp;
  char cbuffer[MAX_LINE_SIZE], buffer[MAX_LINE_SIZE];
  DWORD valueSize,i,nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
  for (i=0;i<nbSubKey;i++)
  {
    //get nk of key :)
    nk_h_tmp = GetSubNKtonk(hks->buffer, hks->taille_fic, nk_h, hks->position, i);
    if (nk_h_tmp == NULL)continue;

    //C
    buffer[0]  = 0;
    cbuffer[0] = 0;
    valueSize  = MAX_LINE_SIZE;
    if(ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"C", buffer, &valueSize))
    {
      DataToHexaChar(buffer, valueSize, cbuffer, MAX_LINE_SIZE);
      TraiterGroupDataFromSAM_C(cbuffer, rid,  group,  group_size_max);
    }
  }
}
//------------------------------------------------------------------------------
void GetUserGroupFRF(DWORD userRID, char *group, DWORD size_max_group)
{
  char file[MAX_PATH];
  HK_F_OPEN hks;

  group[0] = 0;

  //get all file on by on on test if ok or not
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
  if (hitem!=NULL || !LOCAL_SCAN) //files
  {
    while(hitem!=NULL)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      //if (file[0] == 0 /*|| !(Contient(file,"SECURITY") || Contient(file,"security"))*/) continue;

      //open file + verify
      if(OpenRegFiletoMem(&hks, file))
      {
        //get group
        GetUserGroupFromRegFile(userRID, group, size_max_group, &hks, "SAM\\Domains\\Builtin\\Aliases");
        GetUserGroupFromRegFile(userRID, group, size_max_group, &hks, "SAM\\Domains\\Account\\Aliases");
        CloseRegFiletoMem(&hks);
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }
}
//------------------------------------------------------------------------------
void Scan_registry_user_file(HK_F_OPEN *hks, sqlite3 *db, unsigned int session_id, char *computer_name)
{
  DWORD userRID = 0;
  USERS_INFOS User_infos;
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"RegistryUser\";\"source\";\"name\";\"RID\";\"SID\";\"grp\";\"description\";\"last_logon\";\"last_password_change\";"
         "\"nb_connexion\";\"type\";\"state_id\";\"session_id\";\r\n");
  #endif
  //get ref key for hashs
  BYTE b_f[MAX_LINE_SIZE];
  Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "SAM\\Domains\\Account", NULL,"F", b_f, MAX_LINE_SIZE);

  //enum all users
  //exist or not in the file ?
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "SAM\\Domains\\Account\\Users");
  if (nk_h == NULL)return;

  HBIN_CELL_NK_HEADER *nk_h_tmp;
  DWORD valueSize;
  BOOL ok_test;
  char SubKeyName[MAX_PATH];
  char cbuffer[MAX_LINE_SIZE], buffer[MAX_LINE_SIZE];
  DWORD i,nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
  for (i=0;i<nbSubKey;i++)
  {
    ok_test = FALSE;
    //for each subkey
    if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, i, SubKeyName, MAX_PATH))
    {
      //get nk of key :)
      nk_h_tmp = GetSubNKtonk(hks->buffer, hks->taille_fic, nk_h, hks->position, i);
      if (nk_h_tmp == NULL)continue;

      //F
      buffer[0]  = 0;
      cbuffer[0] = 0;
      valueSize  = MAX_LINE_SIZE;
      if(ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"F", buffer, &valueSize))
      {
        DataToHexaChar(buffer, valueSize, cbuffer, MAX_LINE_SIZE);
        userRID = TestUserDataFromSAM_F(&User_infos,cbuffer);
        ok_test = TRUE;
      }

      //V
      buffer[0]  = 0;
      cbuffer[0] = 0;
      valueSize  = MAX_LINE_SIZE;
      if(ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"V", buffer, &valueSize))
      {
        DataToHexaChar(buffer, valueSize, cbuffer, MAX_LINE_SIZE);
        if(TestUserDataFromSAM_V(&User_infos,cbuffer,computer_name))
        {
          //test if rid and sid ok
          userRID = HTDF(SubKeyName,8);
          if(User_infos.RID[0] == 0)snprintf(User_infos.RID,MAX_PATH,"%05lu",userRID);
          if(User_infos.SID[0] == 0)snprintf(User_infos.SID,MAX_PATH,"S-1-5-?-?-?-?-%lu",userRID);
        }else
        {
          if(User_infos.RID[0] == 0 && userRID)snprintf(User_infos.RID,MAX_PATH,"%05lu",userRID);
          if(User_infos.SID[0] == 0 && userRID)snprintf(User_infos.SID,MAX_PATH,"S-1-5-?-?-?-?-%lu",userRID);
        }
        ok_test = TRUE;
      }else
      {
        if(User_infos.RID[0] == 0 && userRID)snprintf(User_infos.RID,MAX_PATH,"%05lu",userRID);
        if(User_infos.SID[0] == 0 && userRID)snprintf(User_infos.SID,MAX_PATH,"S-1-5-?-?-?-?-%lu",userRID);
      }

      if (!ok_test)continue;

      //get groups
      if (userRID) GetUserGroupFRF(userRID, User_infos.group, MAX_PATH);

      //get hashs
      if(b_f[0] != 0 && _SYSKEY[0] != 0)
      {
        DecodeSAMHashXP(_SYSKEY,User_infos.pwdump_pwd_raw_format,userRID,User_infos.name,b_f);
      }

      //add user
      convertStringToSQL(User_infos.description, MAX_PATH);
      addRegistryUsertoDB(hks->file,User_infos.name, User_infos.RID, User_infos.SID, User_infos.group,
                          User_infos.description, User_infos.last_logon, User_infos.last_password_change,
                          User_infos.nb_connexion, User_infos.type, User_infos.state_id,session_id, db);

      //add password
      if (TEST_REG_PASSWORD_ENABLE)
        addPasswordtoDB(hks->file, User_infos.name, User_infos.pwdump_pwd_format, User_infos.pwdump_pwd_raw_format, REG_PASSWORD_STRING_LOCAL_USER, session_id, db);
    }
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_user(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  char file[MAX_PATH], file_SAM[MAX_PATH]="";
  HK_F_OPEN hks;

  char sk[MAX_PATH]="";

  char computer[DEFAULT_TMP_SIZE]="";
  BOOL ok_computer = FALSE;

  //files or local
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
  if (hitem!=NULL || !LOCAL_SCAN) //files
  {
    while(hitem!=NULL)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      if (file[0] != 0)
      {
        charToLowChar(file);
        //check for SAM files
        if ((Contient(file,"sam")) && file_SAM[0] == 0)
        {
          strcpy(file_SAM,file);
          hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
          continue;
        }

        //open file + verify
        if(OpenRegFiletoMem(&hks, file))
        {
          //get syskey
          registry_syskey_file(&hks, sk, MAX_PATH);

          if (!ok_computer)
          {
            char tmp[DEFAULT_TMP_SIZE]="";
            Readnk_Value(hks.buffer, hks.taille_fic, (hks.pos_fhbin)+HBIN_HEADER_SIZE, hks.position, "ControlSet001\\Control\\ComputerName\\ComputerName", NULL,"ComputerName", tmp, DEFAULT_TMP_SIZE);

            if (tmp[0]!=0)
            {
              strcpy(computer,tmp);
              ok_computer = TRUE;
            }
          }

          Scan_registry_user_file(&hks, db, session_id,computer);

          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }

    //SAM file in last
    if (file_SAM[0] != 0)
    {
      //open file + verify
      if(OpenRegFiletoMem(&hks, file_SAM))
      {
        Scan_registry_user_file(&hks, db, session_id,computer);
        CloseRegFiletoMem(&hks);
      }
    }


  }else Scan_registry_user_local(db, session_id);

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
