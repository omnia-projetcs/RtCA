//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addLDAPtoDB(char *dit, char *value, char *dn, char *data, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_ldap (dit,value,dn,data,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           dit,value,dn,data,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"LDAP\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         dit,value,dn,data,session_id);
  #endif
}
//------------------------------------------------------------------------------
//http://msdn.microsoft.com/en-us/library/aa367016%28v=vs.85%29.aspx
//------------------------------------------------------------------------------
//init LDAP
LDAP* init_ldap_connection()
{
  LDAP* ldap_c = NULL;

  //init HANDLE
  if ((ldap_c = ldap_init(NULL,LDAP_PORT)) == NULL)return NULL;

  //LDAP V3
  ULONG version = LDAP_VERSION3;
  if (ldap_set_option(ldap_c,LDAP_OPT_PROTOCOL_VERSION,(void*)&version) == LDAP_SUCCESS)
  {
    if (ldap_connect(ldap_c,NULL) == LDAP_SUCCESS)
    {
      //use local user credentials
      if (ldap_bind_s(ldap_c,NULL,NULL,LDAP_AUTH_NEGOTIATE) != LDAP_SUCCESS)
      {
        ldap_unbind(ldap_c);
        return NULL;
      }
    }else
    {
      ldap_unbind(ldap_c);
      return NULL;
    }
  }else
  {
    ldap_unbind(ldap_c);
    return NULL;
  }

  return ldap_c;
}
//------------------------------------------------------------------------------
//view if hexa !!!
char *ValidDataValue(char *attrbut, char *value, struct berval*value_len,char *tmp, unsigned int tmp_size)
{
  if (strlen(value) != value_len->bv_len)
  {
    strcpy(tmp,"0x\0");
    //binary data
    if (!strcmp(attrbut,"objectGUID") || !strcmp(attrbut,"fRSReplicaSetGUID")  || !strcmp(attrbut,"fRSVersionGUID"))  //GUID
    {
      //size : 4-2-2-2-6
      if (value_len->bv_len >= 16)
      {
        snprintf(tmp,DEFAULT_TMP_SIZE,"{%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
                 value_len->bv_val[3] & 0xFF,value_len->bv_val[2] & 0xFF,value_len->bv_val[1] & 0xFF,value_len->bv_val[0] & 0xFF,
                 value_len->bv_val[5] & 0xFF,value_len->bv_val[4] & 0xFF,
                 value_len->bv_val[7] & 0xFF,value_len->bv_val[6] & 0xFF,
                 value_len->bv_val[8] & 0xFF,value_len->bv_val[9] & 0xFF,
                 value_len->bv_val[10] & 0xFF,value_len->bv_val[11] & 0xFF,value_len->bv_val[12] & 0xFF,value_len->bv_val[13] & 0xFF,value_len->bv_val[14] & 0xFF,value_len->bv_val[15] & 0xFF);
        return tmp;

      }else return DataToHexaChar(value_len->bv_val, value_len->bv_len, tmp, tmp_size);

    }else if (!strcmp(attrbut,"objectSid")) //SID
    {
      SK_SID *sid = (SK_SID *)value_len->bv_val;
      if (sid != NULL)
      {
        unsigned int i, nb = sid->nb_ID;
        if (nb > 6)nb = 6;

        snprintf(tmp,DEFAULT_TMP_SIZE,"S-1-%u",sid->ID0);
        for (i=0;i<nb;i++)snprintf(tmp+strlen(tmp),DEFAULT_TMP_SIZE-strlen(tmp),"-%lu",sid->ID[i]);

        return tmp;
      }else return DataToHexaChar(value_len->bv_val, value_len->bv_len, tmp, tmp_size);
    }else if (!strcmp(attrbut,"auditingPolicy") ||
              !strcmp(attrbut,"ipsecData") ||
              !strcmp(attrbut,"dnsRecord") ||
              !strcmp(attrbut,"dNSProperty")) return DataToHexaChar(value_len->bv_val, value_len->bv_len, tmp, tmp_size);

  //date formats
  }else if ((!strcmp(attrbut,"whenCreated") || !strcmp(attrbut,"whenChanged")) && value_len->bv_len >= 14)
  {
    snprintf(tmp,DEFAULT_TMP_SIZE,"%c%c%c%c/%c%c/%c%c %c%c:%c%c:%c%c",
             value[0],value[1],value[2],value[3],value[4],value[5],value[6],value[7],
             value[8],value[9],value[10],value[11],value[12],value[13]);
    return tmp;

  }else if (!strcmp(attrbut,"creationTime") || !strcmp(attrbut,"lastLogon") || !strcmp(attrbut,"pwdLastSet") || !strcmp(attrbut,"priorSetTime"))
  {
    return DataToHexaChar(value_len->bv_val, value_len->bv_len, tmp, tmp_size);

    /*typedef struct
    {
      DWORD d2_utc;
      DWORD d1_utc;

    }*PDATE_M;
    PDATE_M Date_m = value_len->bv_val;

    if (Date_m->d1_utc == 0 && Date_m->d2_utc == 0)
    {
      strcpy(tmp,"0 (Never)\0");
      return tmp;
    }
    else
    {
      //DWORD d1_utc = (val >> 32);
      //DWORD d2_utc = (val & 0xFFFFFFFF);

      FILETIME FileTime;
      FileTime.dwLowDateTime  = Date_m->d2_utc;
      FileTime.dwHighDateTime = Date_m->d1_utc;

      return filetimeToString(FileTime, tmp, DEFAULT_TMP_SIZE);
    }*/
  }

  return value;
}
//------------------------------------------------------------------------------
BOOL GetLdapValues(char *dc, char *filter, unsigned int session_id, sqlite3 *db)
{
  LDAP* ldap_c = init_ldap_connection();
  if (dc != NULL && ldap_c != NULL)
  {
    #ifdef DEV_DEBUG_MODE
    printf("\n[LDAP] Get init \"%s\" (GetLdapValues) OK!\n",dc);
    #endif

    //search for all values with attributs !!!
    LDAPMessage *pMsg_dc = NULL;
    if (ldap_search_s(ldap_c,dc,LDAP_SCOPE_SUBTREE,filter,NULL,FALSE,&pMsg_dc) == LDAP_SUCCESS)
    {
      if (pMsg_dc != NULL)
      {
        #ifdef DEV_DEBUG_MODE
        printf("[LDAP] Start enum values for \"%s\" (%s) OK!\n",dc,filter);
        #endif

        //get count resultas
        DWORD nb_resultst =ldap_count_entries(ldap_c,pMsg_dc);
        #ifdef DEV_DEBUG_MODE
        printf("[LDAP] Get nb_resultst %d (ldap_count_entries) OK!\n",nb_resultst);
        #endif

        if (nb_resultst>0)
        {
          LDAPMessage* pEntry = NULL;
          char *pAttribute = NULL;
          BerElement* pBer = NULL;
          char **ppValue = NULL;
          struct berval**ppValueInfos = NULL;
          char *dn;
          DWORD i =0, j, jValue;
          char tmp[DEFAULT_TMP_SIZE];
          char suivi_tmp[MAX_LINE_SIZE];

          pEntry = ldap_first_entry(ldap_c, pMsg_dc);
          do
          {
            //error
            if(pEntry == NULL)
            {
              #ifdef DEV_DEBUG_MODE
              printf("[LDAP] failed with 0x%0lx\n", LdapGetLastError());
              #endif
              break;
            }

            //get only name for path !
            dn = ldap_get_dn(ldap_c, pEntry);

            #ifdef DEV_DEBUG_MODE
            if (dn != NULL)printf("[LDAP] entry %d - %s OK!\n",i,dn);
            else printf("[LDAP] entry %d - <NULL> OK!\n",i);
            #endif

            //get atribute
            pAttribute = ldap_first_attribute(ldap_c,pEntry,&pBer);
            while(pAttribute != NULL && start_scan)
            {
              //get value
              #ifdef DEV_DEBUG_MODE
              printf("[LDAP]  pAttribute %s :\n",pAttribute);
              #endif

              //Get the string values
              ppValue = ldap_get_values(ldap_c,pEntry,pAttribute);
              ppValueInfos = ldap_get_values_len(ldap_c,pEntry,pAttribute);

              if (ppValue != NULL)
              {

                jValue = ldap_count_values(ppValue);
                for (j=0;j<jValue;j++)
                {
                  #ifdef DEV_DEBUG_MODE
                  printf("[LDAP]   ppValue %s\n",ppValue[j]);
                  #endif

                  if (dn != NULL)
                  {
                    addLDAPtoDB(dc, pAttribute, dn, ValidDataValue(pAttribute,ppValue[j],ppValueInfos[j],tmp,DEFAULT_TMP_SIZE), session_id, db);

                    snprintf(suivi_tmp, MAX_LINE_SIZE, "%s - %s", dc, dn);
                    SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)suivi_tmp);
                  }else
                  {
                    addLDAPtoDB(dc, pAttribute, "", ValidDataValue(pAttribute,ppValue[j],ppValueInfos[j],tmp,DEFAULT_TMP_SIZE), session_id, db);
                    SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)dc);
                  }
                }
                ldap_value_free(ppValue);
                ppValue = NULL;
              }

              //next
              ldap_memfree(pAttribute);
              pAttribute = ldap_next_attribute(ldap_c,pEntry,pBer);
            }
            pEntry = ldap_next_entry(ldap_c, pEntry);
            if (dn!=NULL)ldap_memfree(dn);

            i++;
          }while(pEntry != NULL && start_scan);
        }
        ldap_msgfree(pMsg_dc);
      }
    }
    ldap_unbind(ldap_c);
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD Scan_ldap_g(LPVOID lParam, unsigned int session_id, sqlite3 *db)
{
  LDAP* ldap_c = init_ldap_connection();
  if (ldap_c != NULL)
  {
    #ifdef DEV_DEBUG_MODE
    printf("[LDAP] init connection OK!\n");
    #endif

    //get init DN
    LDAPMessage *pMsg_dc = NULL;
    if (ldap_search_s(ldap_c,NULL,LDAP_SCOPE_BASE,NULL,NULL,FALSE,&pMsg_dc) == LDAP_SUCCESS)
    {
      if (pMsg_dc != NULL)
      {
        #ifdef DEV_DEBUG_MODE
        printf("[LDAP] Get init DN (ldap_search_s) OK!\n");
        #endif

        //get datas + new search with new datas !!!
        char **ppszConfigDN = NULL;
        ppszConfigDN = ldap_get_values(ldap_c, pMsg_dc, "configurationNamingContext");
        if (ppszConfigDN!=NULL)
        {
          DWORD i, iValue = ldap_count_values(ppszConfigDN);

          #ifdef DEV_DEBUG_MODE
          printf("[LDAP] Get iValue %d (ldap_count_values) OK!\n",iValue);
          #endif

          //get DN root values one by one
          //LDAPMessage *pMsg = NULL;
          unsigned long int j=0;
          for (i=0;i<iValue && start_scan;i++)
          {
            #ifdef DEV_DEBUG_MODE
            printf("[LDAP] Get iValue[%d] \"%s\" (ldap_get_values) OK!\n",i,ppszConfigDN[i]);
            #endif

            //get values
            j = Contient(ppszConfigDN[i],"DC=");
            if (j>0)
            {
              //extract DC and load datas
              char *c,tmp[DEFAULT_TMP_SIZE],request[DEFAULT_TMP_SIZE];
              snprintf(tmp,DEFAULT_TMP_SIZE,"%s",ppszConfigDN[i]);
              c = tmp+(j-3);

              #ifdef DEV_DEBUG_MODE
              printf("[LDAP] Get view DC : (%d) \"%s\" OK!\n",j,c);
              #endif

              //get standards informations :
              GetLdapValues(c,"(ObjectClass=*)", session_id, db);
              snprintf(request,DEFAULT_TMP_SIZE,"CN=Configuration,%s",c);
              GetLdapValues(request,"(ObjectClass=*)", session_id, db);
              snprintf(request,DEFAULT_TMP_SIZE,"CN=Schema,CN=Configuration,%s",c);
              GetLdapValues(request,"(ObjectClass=*)", session_id, db);
              snprintf(request,DEFAULT_TMP_SIZE,"DC=DomainDnsZones,%s",c);
              GetLdapValues(request,"(ObjectClass=*)", session_id, db);
              snprintf(request,DEFAULT_TMP_SIZE,"DC=ForestDnsZones,%s",c);
              GetLdapValues(request,"(ObjectClass=*)", session_id, db);
            }else GetLdapValues(ppszConfigDN[i],"(ObjectClass=*)", session_id, db);
          }
        }
      }
    }

    //clean
    ldap_unbind(ldap_c);
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_ldap(LPVOID lParam)
{
  //check if local or not :)
  if (!LOCAL_SCAN)
  {
    h_thread_test[(unsigned int)lParam] = 0;
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);
    return 0;
  }
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"LDAP\";\"dit\";\"value\";\"dn\";\"data\";\"session_id\";\r\n");
  #endif
  //init
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  Scan_ldap_g(lParam,session_id,db);

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
