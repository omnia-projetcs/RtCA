//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
char tmp_file_firefox[MAX_LINE_SIZE];
//------------------------------------------------------------------------------
void addFirefoxtoDB(char *file, char *parameter, char *data, char *date, DWORD id_language_description, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_firefox (file,parameter,date,id_language_description,session_id,data) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%lu\",%d,\"%s\");",
           file,parameter,date,id_language_description,session_id,data);

  //if datas too long
  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4-strlen(request));

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Firefox\";\"%s\";\"%s\";\"%s\";\"%lu\";\"%d\",\"%s\";\r\n",
         file,parameter,date,id_language_description,session_id,data);
  #endif
}
//------------------------------------------------------------------------------
int callback_sqlite_firefox(void *datas, int argc, char **argv, char **azColName)
{
  if (datas == NULL || argc < 1) return 0;
  FORMAT_CALBAK_READ_INFO *type = datas;
  char date[DATE_SIZE_MAX]="";
  unsigned int i;

  if (type->type <= nb_sql_FIREFOX && argc>0 && argv != NULL)
  {
    char *tmp = malloc(argc*MAX_PATH);
    if (tmp == NULL)return 0;

#ifdef _WIN64_VERSION_
    char tmp2[MAX_PATH]="";

    //copy datas
    for (i=0;i<argc && i<8 && start_scan;i++)
    {
      if (argv[i] == NULL)continue;

      //date or not ?
      if (strlen(argv[i]) == DATE_SIZE_MAX-1)
      {
        if (argv[i][4] == '/' && argv[i][13] == ':')
        {
          if (strcmp("1970/01/01 01:00:00",argv[i])!=0)strncpy(date,argv[i],DATE_SIZE_MAX);
          continue;
        }
      }
      convertUTF8toUTF16toChar(argv[i], strlen(argv[i]), tmp2, MAX_PATH);
      strncat(tmp,tmp2,MAX_PATH);
      strncat(tmp,", \0",MAX_PATH);
    }

    if(strlen(tmp)-2 > 0)tmp[strlen(tmp)-2] = 0;
#else
    unsigned int size=0;
    //copy datas
    for (i=0;i<argc && MAX_PATH-size > 0 && start_scan;i++)
    {
      if (argv[i] == NULL)continue;

      //date or not ?
      if (strlen(argv[i]) == DATE_SIZE_MAX-1)
      {
        if (argv[i][4] == '/' && argv[i][13] == ':')
        {
          if (strcmp("1970/01/01 01:00:00",argv[i])!=0)strncpy(date,argv[i],DATE_SIZE_MAX);
          continue;
        }
      }
      if (i>0)snprintf(tmp+size,MAX_PATH-size,", %s",convertUTF8toUTF16(argv[i], strlen(argv[i])+1));
      else snprintf(tmp+size,MAX_PATH-size,"%s",convertUTF8toUTF16(argv[i], strlen(argv[i])+1));
      size = strlen(tmp);
    }
#endif

    //get datas and write it
    convertStringToSQL(tmp, MAX_PATH);
    addFirefoxtoDB(tmp_file_firefox,sql_FIREFOX[type->type].params,tmp,date,sql_FIREFOX[type->type].test_string_id,current_session_id,db_scan);

    free(tmp);
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_firefox_history(LPVOID lParam)
{
  FORMAT_CALBAK_READ_INFO data;

  //get child
  HTREEITEM hitem = NULL;
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Firefox\";\"file\";\"parameter\";\"date\";\"id_language_description\";\"session_id\",\"data\";\r\n");
  #endif // CMD_LINE_ONLY_NO_DB
  if (!CONSOL_ONLY)hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
  if ((hitem == NULL && LOCAL_SCAN) || CONSOL_ONLY) //local
  {
    //get path of all profils users
    //HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList
    HKEY CleTmp   = 0;
    if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\",&CleTmp)==ERROR_SUCCESS)
    {
      DWORD i, nbSubKey=0, key_size;
      sqlite3 *db_tmp;
      char tmp_key[MAX_LINE_SIZE], tmp_key_path[MAX_LINE_SIZE];
      if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
      {
        //get subkey
        for(i=0;i<nbSubKey;i++)
        {
          key_size    = MAX_LINE_SIZE;
          tmp_key[0]  = 0;
          if (RegEnumKeyEx (CleTmp,i,tmp_key,&key_size,0,0,0,0)==ERROR_SUCCESS)
          {
            //generate the key path
            snprintf(tmp_key_path,MAX_LINE_SIZE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s\\",tmp_key);

            //get profil path
            if (ReadValue(HKEY_LOCAL_MACHINE,tmp_key_path,"ProfileImagePath",tmp_key, MAX_LINE_SIZE))
            {
              //verify the path if %systemdrive%
              ReplaceEnv("SYSTEMDRIVE",tmp_key,MAX_LINE_SIZE);

              //search profil directory
              snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\*.default",tmp_key);
              WIN32_FIND_DATA wfd0;
              HANDLE hfic = FindFirstFile(tmp_key_path, &wfd0);
              if (hfic != INVALID_HANDLE_VALUE)
              {
                do
                {
                  if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                  {
                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\content-prefs.sqlite",tmp_key,wfd0.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }

                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\cookies.sqlite",tmp_key,wfd0.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }

                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\places.sqlite",tmp_key,wfd0.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }

                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\search.sqlite",tmp_key,wfd0.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }
                  }
                }while(FindNextFile (hfic,&wfd0));

                FindClose(hfic);
              }
              //search profil directory
              /*snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\*.default",tmp_key);
              WIN32_FIND_DATA wfd1;
              hfic = FindFirstFile(tmp_key_path, &wfd1);
              if (hfic != INVALID_HANDLE_VALUE)
              {
                do
                {
                  if (wfd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                  {
                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\content-prefs.sqlite",tmp_key,wfd1.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }

                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\cookies.sqlite",tmp_key,wfd1.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }

                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\places.sqlite",tmp_key,wfd1.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }

                    snprintf(tmp_file_firefox,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\search.sqlite",tmp_key,wfd1.cFileName);
                    //test to open file
                    if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
                    {
                      for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
                      {
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                        sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
                        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                      }
                      sqlite3_close(db_tmp);
                    }
                  }
                }while(FindNextFile (hfic,&wfd0));
                FindClose(hfic);
              }*/
            }
          }
        }
      }
      RegCloseKey(CleTmp);
    }
  }else
  {
    sqlite3 *db_tmp;

    while(hitem!=NULL)
    {
      //get item txt
      GetTextFromTrv(hitem, tmp_file_firefox, MAX_LINE_SIZE);
      //test to open file
      if (sqlite3_open(tmp_file_firefox, &db_tmp) == SQLITE_OK)
      {
        for (data.type =0;data.type <nb_sql_FIREFOX && start_scan;data.type = data.type+1)
        {
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
          sqlite3_exec(db_tmp, sql_FIREFOX[data.type].sql, callback_sqlite_firefox, &data, NULL);
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
        }
        sqlite3_close(db_tmp);
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }

  if (!CONSOL_ONLY)check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
