//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRegistryStarttoDB(char *file, char *hk, char *key,
               char *value, char *data, char *last_parent_key_update, DWORD session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  if (*value == '\0' && *data == '\0')return;

  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_start (file,hk,key,value,data,last_parent_key_update,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%lu);",
           file,hk,key,value,data,last_parent_key_update,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Start\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,hk,key,value,data,last_parent_key_update,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
/*void EnumRegRunValue(HKEY hk, char *s_hk,char *key,unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,key,&CleTmp)!=ERROR_SUCCESS)return;

  FILETIME lastupdate;
  DWORD i, nbSubValue=0;
  if (RegQueryInfoKey (CleTmp,0,0,0,0,0,0,&nbSubValue,0,0,0,&lastupdate)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }

  //get date update
  char date[DATE_SIZE_MAX];
  filetimeToString_GMT(lastupdate, date, DATE_SIZE_MAX);

  for (i=0;i<nbSubValue;i++)
  {

  }

  RegCloseKey(CleTmp);
}*/


void EnumRegRunValue(HKEY hk, char *s_hk,char *key,unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,key,&CleTmp)==ERROR_SUCCESS)
  {
    DWORD k=0,i,j, nbSubValue = 0, value_size, data_size,type;
    char value[MAX_PATH], data[MAX_PATH],date[DATE_SIZE_MAX],tmp[MAX_PATH];
    FILETIME lastupdate;

    if (RegQueryInfoKey (CleTmp,0,0,0,0,0,0,&nbSubValue,0,0,0,&lastupdate)==ERROR_SUCCESS)
    {
      filetimeToString_GMT(lastupdate, date, DATE_SIZE_MAX);
      //for (k=0;k<nbSubValue && start_scan;k++)
      while(k<nbSubValue && start_scan)
      {
        value_size = MAX_PATH;
        data_size  = MAX_PATH;
        value[0]   = 0;
        data[0]    = 0;
        i = k;
        if (RegEnumValue (CleTmp,i,value,&value_size,NULL,(LPDWORD)&type,(LPBYTE)data,&data_size)==ERROR_SUCCESS)
        {
          switch(type)
          {
            case REG_EXPAND_SZ:
            case REG_SZ:
              convertStringToSQL(value, MAX_PATH);
              convertStringToSQL(data, MAX_PATH);
              addRegistryStarttoDB("", s_hk, key, value, data, date, session_id, db);break;
            case REG_LINK:
              convertStringToSQL(value, MAX_PATH);
              snprintf(tmp,MAX_PATH,"%S",data);
              convertStringToSQL(tmp, MAX_PATH);
              addRegistryStarttoDB("", s_hk, key, value, tmp, date, session_id, db);break;
            case REG_MULTI_SZ:
              for (j=0;j<data_size;j++)
              {
                if (data[j] == 0)data[j]=';';
              }
              convertStringToSQL(data, MAX_PATH);
              addRegistryStarttoDB("", s_hk, key, value, data, date, session_id, db);break;
          }
        }
        k++;
      }
    }
    RegCloseKey(CleTmp);
  }
}
//------------------------------------------------------------------------------
int callback_sqlite_registry_run_local(void *datas, int argc, char **argv, char **azColName)
{
  //FORMAT_CALBAK_TYPE *type = datas; // == SQLITE_REGISTRY_TYPE_RUN
  unsigned int session_id = current_session_id;

  //enumerate
  HKEY hk = hkStringtohkey(argv[0]);

  switch(atoi(argv[2]))
  {
    //enum value on key
    case 0:EnumRegRunValue(hk,argv[0],argv[1],session_id,db_scan);break;
    //read string value
    case 1:
    {
      char tmp[MAX_PATH]="";
      if (ReadValue(hk,argv[1],argv[3],tmp, MAX_PATH))
      {
        char date[DATE_SIZE_MAX]="";
        ReadKeyUpdate(hk,argv[1], date, DATE_SIZE_MAX);
        addRegistryStarttoDB("", argv[0], argv[1], argv[3], tmp, date, session_id, db_scan);
      }
    }
    break;
    //read dword value
    case 2:
    {
      char tmp[MAX_PATH]="";
      long int data = ReadDwordValue(hk,argv[1],argv[3]);
      if (data != -1)
      {
        snprintf(tmp,MAX_PATH,"%lu",data);
        char date[DATE_SIZE_MAX]="";
        ReadKeyUpdate(hk,argv[1], date, DATE_SIZE_MAX);
        addRegistryStarttoDB("", argv[0], argv[1], argv[3], tmp, date, session_id, db_scan);
      }
    }
    break;
  }
  return 0;
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
HK_F_OPEN local_start_hks;
//------------------------------------------------------------------------------
int callback_sqlite_registry_run_file(void *datas, int argc, char **argv, char **azColName)
{
  unsigned int session_id = current_session_id;
  char tmp[MAX_PATH];
  switch(atoi(argv[2])) //type
  {
    case 0: //enum
    {
      HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(local_start_hks.buffer,local_start_hks.taille_fic, (local_start_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_start_hks.position,argv[1]);
      if (nk_h!=NULL)
      {
        //key update
        char parent_key_update[DATE_SIZE_MAX];
        Readnk_Infos(local_start_hks.buffer,local_start_hks.taille_fic, (local_start_hks.pos_fhbin), local_start_hks.position,
                     NULL, nk_h, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);

        //get values
        char value[MAX_PATH];
        DWORD i, nbSubKey = GetValueData(local_start_hks.buffer,local_start_hks.taille_fic, nk_h, (local_start_hks.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);

        for (i=0;i<nbSubKey && start_scan;i++)
        {
          if (GetValueData(local_start_hks.buffer,local_start_hks.taille_fic, nk_h, (local_start_hks.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_PATH))
          {
            //save
            convertStringToSQL(value, MAX_PATH);
            convertStringToSQL(tmp, MAX_PATH);
            addRegistryStarttoDB(local_start_hks.file, "", argv[1], value, tmp, parent_key_update, session_id, db_scan);
          }
        }
      }
    }
    break;

    case 1: //string
    case 2: //dword
      if (Readnk_Value(local_start_hks.buffer,local_start_hks.taille_fic, (local_start_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_start_hks.position,
                       argv[1], NULL, argv[3], tmp, MAX_LINE_SIZE))
      {
        //key update
        char parent_key_update[DATE_SIZE_MAX];
        Readnk_Infos(local_start_hks.buffer,local_start_hks.taille_fic, (local_start_hks.pos_fhbin), local_start_hks.position,
                     argv[1], NULL, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);

        //save
        convertStringToSQL(tmp, MAX_LINE_SIZE);
        addRegistryStarttoDB(local_start_hks.file, "", argv[1], argv[3], tmp, parent_key_update, session_id, db_scan);
      }
    break;
  }
  return 0;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_start(LPVOID lParam)
{
  //init
  char file[MAX_PATH];

  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = SQLITE_REGISTRY_TYPE_RUN;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Start\";\"file\";\"hk\";\"key\";\"value\";\"data\";\"last_parent_key_update\";\"session_id\";\r\n");
  #endif
  //files or local
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
  if (hitem!=NULL || !LOCAL_SCAN) //files
  {
    while(hitem!=NULL && start_scan)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      if (file[0] != 0)
      {
        //verify
        if(OpenRegFiletoMem(&local_start_hks, file))
        {
          sqlite3_exec(db_scan, "SELECT hk,key_search,type,value FROM extract_registry_start_request;", callback_sqlite_registry_run_file, &fcri, NULL);
          CloseRegFiletoMem(&local_start_hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    //read all registry
    sqlite3_exec(db_scan, "SELECT hk,key,type,value FROM extract_registry_start_request;", callback_sqlite_registry_run_local, &fcri, NULL);

    //path of start menu of current user :
    //not be done because, he can be rescue in file search !
    //HKEY_CURRENT_USER\Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders : Startup
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan);
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
