//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
char tmp_file_android[MAX_PATH];
//------------------------------------------------------------------------------
void addAndroidtoDB(char *file, char *parameter, char *data, char *date, DWORD id_language_description, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_android (file,parameter,date,id_language_description,session_id,data) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%lu\",%d,\"%s\");",
           file,parameter,date,id_language_description,session_id,data);

  //if datas too long
  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4-strlen(request));

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf ("\"ANDROID\";\"%s\";\"%s\";\"%s\";\"%lu\":\"%d\";\"%s\";\r\n",
           file,parameter,date,id_language_description,session_id,data);

  #endif
}
//------------------------------------------------------------------------------
int callback_sqlite_android(void *datas, int argc, char **argv, char **azColName)
{
  FORMAT_CALBAK_READ_INFO *type = (FORMAT_CALBAK_READ_INFO *)datas;
  char tmp[MAX_PATH]="";
  char date[DATE_SIZE_MAX]="";
  unsigned int i,size=0;
  if (type->type > 0 && type->type < nb_sql_ANDROID)
  {
    //copy datas
    for (i=0;i<argc && MAX_PATH-size > 0;i++)
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
    //get datas and write it
    convertStringToSQL(tmp, MAX_PATH);
    addAndroidtoDB(tmp_file_android,sql_ANDROID[type->type].params,tmp,date,sql_ANDROID[type->type].test_string_id,current_session_id,db_scan);
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_android_history(LPVOID lParam)
{
  FORMAT_CALBAK_READ_INFO data;

  //get child
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  if (!CONSOL_ONLY)
  {
    HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
    sqlite3 *db_tmp;

    while(hitem!=NULL)
    {
      //get item txt
      GetTextFromTrv(hitem, tmp_file_android, MAX_PATH);
      //test to open file
      if (sqlite3_open(tmp_file_android, &db_tmp) == SQLITE_OK)
      {
        for (data.type =0;data.type <nb_sql_ANDROID && start_scan;data.type = data.type+1)
        {
          sqlite3_exec(db_tmp, sql_ANDROID[data.type].sql, callback_sqlite_android, &data, NULL);
        }
        sqlite3_close(db_tmp);
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }

    if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  }

  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
