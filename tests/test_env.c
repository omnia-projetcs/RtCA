//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addEnvtoDB(char *source, char *env, char *user, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_env (source,string,user,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",%d);",
           source,env,user,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"ARP\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         source,env,user,session_id);
  #endif
}
//------------------------------------------------------------------------------
void EnumEnv(HK_F_OPEN *hks, unsigned int session_id, sqlite3 *db, char *path)
{
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, path);
  if (nk_h == NULL) return;

  DWORD i, nbSubValue = GetValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
  char tmp_key[MAX_PATH], tmp_value[MAX_LINE_SIZE], tmp[MAX_LINE_SIZE];
  for (i=0;i<nbSubValue;i++)
  {
    if (GetValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, i,tmp_key,MAX_PATH,tmp_value,MAX_LINE_SIZE))
    {
      snprintf(tmp,MAX_LINE_SIZE,"%s=%s",tmp_key,tmp_value);
      convertStringToSQL(tmp, MAX_LINE_SIZE);
      addEnvtoDB(path,tmp, hks->file, session_id, db);
    }
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_env(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);

  //check if local or not :)
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"ENV\";\"source\";\"string\";\"user\";\"session_id\";\r\n");
  #endif // CMD_LINE_ONLY_NO_DB
  if (!LOCAL_SCAN)
  {
    //get in registry files
    char file[MAX_PATH];
    HK_F_OPEN hks;
    HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
    while(hitem!=NULL)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      if (file[0] != 0)
      {
        //open file + verify
        if(OpenRegFiletoMem(&hks, file))
        {
          EnumEnv(&hks, session_id, db,"ControlSet001\\Control\\Session Manager\\Environment");
          EnumEnv(&hks, session_id, db,"ControlSet002\\Control\\Session Manager\\Environment");
          EnumEnv(&hks, session_id, db,"ControlSet003\\Control\\Session Manager\\Environment");
          EnumEnv(&hks, session_id, db,"ControlSet004\\Control\\Session Manager\\Environment");
          EnumEnv(&hks, session_id, db,"Environment");
          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    DWORD s = NB_USERNAME_SIZE;
    char user[NB_USERNAME_SIZE+1]="",str[MAX_LINE_SIZE];
    GetUserName(user,&s);

    char *c = GetEnvironmentStrings();
    while (*c)
    {
      str[0]=0;
      snprintf(str,MAX_LINE_SIZE,"%s",c);

      if (str[0]!=0)
      {
        convertStringToSQL(str, MAX_LINE_SIZE);
        addEnvtoDB("",str, user, session_id, db);
      }
      c += strlen(c)+1;
    }
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
