//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
char tmp_file_chrome[MAX_PATH];
void addPasswordtoDB(char *source, char*login, char*password, char*raw_password,unsigned int description_id,unsigned int session_id, sqlite3 *db);
//------------------------------------------------------------------------------
void addChrometoDB(char *file, char *parameter, char *data, char *date, DWORD id_language_description, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_chrome (file,parameter,date,id_language_description,session_id,data) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%lu\",%d,\"%s\");",
           file,parameter,date,id_language_description,session_id,data);

  //if datas too long
  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4-strlen(request));

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Chrome\";\"%s\";\"%s\";\"%s\";\"%lu\";\"%d\";\"%s\";\r\n",
         file,parameter,date,id_language_description,session_id,data);
  #endif
}
//------------------------------------------------------------------------------
int callback_sqlite_chrome(void *datas, int argc, char **argv, char **azColName)
{
  FORMAT_CALBAK_READ_INFO *type = datas;
  char tmp[MAX_PATH]="";
  char date[DATE_SIZE_MAX]="";
  unsigned int i,size=0;

  if (type->type <= nb_sql_CHROME)
  {
    //special case of password !!!
    if(sql_CHROME[type->type].test_string_id == SPECIAL_CASE_CHROME_PASSWORD)
    {
      char password[MAX_PATH]="", raw_password[MAX_PATH]="";

      //decrypt datas password !!!
      CRYPT_INTEGER_BLOB data_in, data_out;
      data_in.cbData = strlen(argv[3]);
      data_in.pbData = (BYTE*)argv[3];

      snprintf(tmp,MAX_PATH,"%s, %s",argv[1],argv[2]);
      snprintf(raw_password,MAX_PATH,"%s",argv[3]);

      if (CryptUnprotectData(&data_out,NULL,NULL,NULL,NULL,0,&data_in))
      {
        snprintf(password,MAX_PATH,"%s",data_out.pbData);
        LocalFree(data_out.pbData);
      }
      convertStringToSQL(tmp, MAX_PATH);
      convertStringToSQL(password, MAX_PATH);
      convertStringToSQL(raw_password, MAX_PATH);
      addPasswordtoDB(tmp_file_chrome,tmp,password,raw_password,SPECIAL_CASE_CHROME_PASSWORD,current_session_id,db_scan);
    }else
    {
      //copy datas
      for (i=0;i<argc && MAX_PATH-size > 0 && start_scan;i++)
      {
        if (argv[i] == NULL)continue;

        //date or not ?
        if (strlen(argv[i]) == DATE_SIZE_MAX-1)
        {
          if (argv[i][4] == '/' && argv[i][13] == ':')
          {
            if (strcmp("1970/01/01 01:00:00",argv[i])!=0 && strcmp("1601/01/01 01:00:00",argv[i])!=0)strncpy(date,argv[i],DATE_SIZE_MAX);
            continue;
          }
        }
        if (i>0)snprintf(tmp+size,MAX_PATH-size,", %s",convertUTF8toUTF16(argv[i], strlen(argv[i])+1));
        else snprintf(tmp+size,MAX_PATH-size,"%s",convertUTF8toUTF16(argv[i], strlen(argv[i])+1));
        size = strlen(tmp);
      }
      //get datas and write it
      convertStringToSQL(tmp, MAX_PATH);
      addChrometoDB(tmp_file_chrome,sql_CHROME[type->type].params,tmp,date,sql_CHROME[type->type].test_string_id,current_session_id,db_scan);
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_chrome_history(LPVOID lParam)
{
  FORMAT_CALBAK_READ_INFO data;

  //get child
  HTREEITEM hitem = NULL;
  if (!CONSOL_ONLY)hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
  if ((hitem == NULL && LOCAL_SCAN) || CONSOL_ONLY)
  {
    //get path of all profils users
    //HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList
    HKEY CleTmp   = 0;
    if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\",&CleTmp)==ERROR_SUCCESS)
    {
      DWORD i, nbSubKey=0, key_size;
      sqlite3 *db_tmp;

      char tmp_key[MAX_PATH], tmp_key_path[MAX_PATH];
      if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
      {
        #ifdef CMD_LINE_ONLY_NO_DB
        printf("\"file\";\"parameter\";\"date\";\"id_language_description\";\"session_id\";\"data\";\r\n");
        #endif

        //get subkey
        for(i=0;i<nbSubKey;i++)
        {
          key_size    = MAX_PATH;
          tmp_key[0]  = 0;
          if (RegEnumKeyEx (CleTmp,i,tmp_key,&key_size,0,0,0,0)==ERROR_SUCCESS)
          {
            //generate the key path
            snprintf(tmp_key_path,MAX_PATH,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s\\",tmp_key);
            //get profil path
            if (ReadValue(HKEY_LOCAL_MACHINE,tmp_key_path,"ProfileImagePath",tmp_key, MAX_PATH))
            {
              //verify the path if %systemdrive%
              ReplaceEnv("SYSTEMDRIVE",tmp_key,MAX_PATH);


              //search file in this path
              snprintf(tmp_key_path,MAX_PATH,"%s\\Local Settings\\Application Data\\Google\\Chrome\\User Data\\Default\\*.*",tmp_key);
              WIN32_FIND_DATA wfd;
              HANDLE hfic = FindFirstFile(tmp_key_path, &wfd);
              if (hfic != INVALID_HANDLE_VALUE)
              {
                do
                {
                  if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}else
                  {
                    if(wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || wfd.cFileName[1] == '.')){}
                    else
                    {
                      //test all files
                      snprintf(tmp_file_chrome,MAX_PATH,"%s\\Local Settings\\Application Data\\Google\\Chrome\\User Data\\Default\\%s",tmp_key,wfd.cFileName);

                      //test to open file
                      if (sqlite3_open(tmp_file_chrome, &db_tmp) == SQLITE_OK)
                      {
                        for (data.type =0;data.type <nb_sql_CHROME && start_scan;data.type = data.type+1)
                        {
                          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
                          sqlite3_exec(db_tmp, sql_CHROME[data.type].sql, callback_sqlite_chrome, &data, NULL);
                          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
                        }
                        sqlite3_close(db_tmp);
                      }
                    }
                  }
                }while(FindNextFile (hfic,&wfd) !=0);
                FindClose(hfic);
              }
            }
          }
        }
      }
      RegCloseKey(CleTmp);
    }
  }else
  {
    sqlite3 *db_tmp;
    #ifdef CMD_LINE_ONLY_NO_DB
    printf("\"Chrome\";\"file\";\"parameter\";\"date\";\"id_language_description\";\"session_id\";\"data\";\r\n");
    #endif
    while(hitem!=NULL)
    {
      //get item txt
      GetTextFromTrv(hitem, tmp_file_chrome, MAX_PATH);
      //test to open file
      if (sqlite3_open(tmp_file_chrome, &db_tmp) == SQLITE_OK)
      {
        for (data.type =0;data.type <nb_sql_CHROME && start_scan;data.type = data.type+1)
        {
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
          sqlite3_exec(db_tmp, sql_CHROME[data.type].sql, callback_sqlite_chrome, &data, NULL);
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
