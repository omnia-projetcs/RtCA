//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
/*
params format :
  - NULL/0 : Format standard (match)
  - 1      : AU début du nom
  - 2      : En fin d nom

  LSTV : hlstv_db
*/
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void addNewRootkitToDB(HANDLE hlstv, char *filename, char*sha256, char* description, char *source, char*update_time, int params, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB

  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO malware_file_list "
           "(filename,sha256,description,source,update_time,params) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
            filename, sha256, description, source, update_time, params);

  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE-strlen(request));

  sqlite3_exec(db,request, NULL, NULL, NULL);

  //add too in the listv
    //add line
  LVITEM lvi;
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";
  lvi.iItem = ListView_GetItemCount(hlstv);
  unsigned int ref_item = ListView_InsertItem(hlstv, &lvi);

  ListView_SetItemText(hlstv,ref_item,0,filename);
  ListView_SetItemText(hlstv,ref_item,1,sha256);
  ListView_SetItemText(hlstv,ref_item,2,description);
  ListView_SetItemText(hlstv,ref_item,3,source);
  ListView_SetItemText(hlstv,ref_item,4,update_time);
  snprintf(request,REQUEST_MAX_SIZE,"%d",params);
  ListView_SetItemText(hlstv,ref_item,5,request);

  #else
  printf(\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",filename, sha256, description, source, update_time, params);
  #endif
}
//------------------------------------------------------------------------------
//load database
void LoadRootKitDB(HANDLE lstvdb)
{
  FORMAT_CALBAK_READ_INFO fcri2;
  fcri2.type = TYPE_SQLITE_FLAG_LOAD_ROOTKIT_DB;
  fcri2.form = lstvdb;

  //Update LSTV
  ListView_DeleteAllItems(lstvdb);
  SQLITE_Data(&fcri2, SQLITE_LOCAL_BDD, SQLITE_OPEN_READONLY);
}
//------------------------------------------------------------------------------
BOOL CheckIfRootKitDbExist(HANDLE lstvdb, char *filename, char *sha256, char *description, char *source, char* update_time, DWORD buffers_sz)
{
  DWORD i, nb_item = ListView_GetItemCount(lstvdb);
  char tmp_file[MAX_LINE_SIZE], tmp_sha256[MAX_LINE_SIZE], tmp_params[MAX_LINE_SIZE];
  BOOL check_ok;

  //lower
  if (filename[0] != 0) charToLowChar(filename);
  if (sha256[0] != 0)   charToLowChar(sha256);

  for (i=0;i<nb_item;i++)
  {
    if (filename[0] != 0)
    {
      tmp_file[0] = 0;
      ListView_GetItemText(lstvdb,i,0,tmp_file,MAX_LINE_SIZE);
      if (tmp_file[0] != 0)
      {
        check_ok      = FALSE;
        tmp_params[0] = 0;
        charToLowChar(tmp_file);
        ListView_GetItemText(lstvdb,i,5,tmp_params,MAX_LINE_SIZE);
        if (tmp_params[0] ==0 || tmp_params[0] =='0')
        {
          if (!strcmp(filename,tmp_file))         check_ok = TRUE;
        }else if (tmp_params[0] =='1')
        {
          if (startWith(filename, tmp_file)) check_ok = TRUE;
        }else if (tmp_params[0] =='2')
        {
          if (endWith(filename, tmp_file)) check_ok = TRUE;
        }

        if (check_ok)
        {
          if(sha256!=NULL)      ListView_GetItemText(lstvdb,i,1,sha256,buffers_sz);
          if(description!=NULL) ListView_GetItemText(lstvdb,i,2,description,buffers_sz);
          if(source!=NULL)      ListView_GetItemText(lstvdb,i,3,source,buffers_sz);
          if(update_time!=NULL) ListView_GetItemText(lstvdb,i,4,update_time,buffers_sz);
          return TRUE;
        }
      }
    }

    if (sha256[0] != 0)
    {
      tmp_sha256[0] = 0;
      ListView_GetItemText(lstvdb,i,0,tmp_sha256,MAX_LINE_SIZE);
      if (tmp_sha256[0] != 0)
      {
        tmp_params[0] = 0;
        charToLowChar(tmp_sha256);
        if (!strcmp(filename,tmp_sha256)) //no other check for hash
        {
          if(filename!=NULL)    ListView_GetItemText(lstvdb,i,0,filename,buffers_sz);
          if(description!=NULL) ListView_GetItemText(lstvdb,i,2,description,buffers_sz);
          if(source!=NULL)      ListView_GetItemText(lstvdb,i,3,source,buffers_sz);
          if(update_time!=NULL) ListView_GetItemText(lstvdb,i,4,update_time,buffers_sz);
          return TRUE;
        }
      }
    }else return FALSE;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
void checkLstvItemId(HANDLE hlstv, HANDLE hlstv_rootkit, DWORD item_id, DWORD col_file, DWORD col_sha256, DWORD col_ref, BOOL msg)
{
  char file[MAX_PATH]="";
  char sha256[MAX_PATH]="";

  ListView_GetItemText(hlstv,item_id,col_file,file,MAX_PATH);
  ListView_GetItemText(hlstv,item_id,col_sha256,sha256,MAX_PATH);

  if (file[0]!=0 || sha256[0]!=0)
  {
    //check
    char description[MAX_PATH], source[MAX_PATH], update_time[MAX_PATH];
    if(CheckIfRootKitDbExist(hlstv_rootkit, file, sha256, description, source, update_time, MAX_PATH))
    {
      char tmp_hidden[MAX_PATH]="", hidden[MAX_LINE_SIZE];
      ListView_GetItemText(hlstv,item_id,col_ref,tmp_hidden,MAX_PATH);

      if (*tmp_hidden != '\0')snprintf(hidden,MAX_LINE_SIZE,"%s, %s (%s) %s", tmp_hidden,description,source,update_time);
      else snprintf(hidden,MAX_LINE_SIZE,"%s (%s) %s", description,source,update_time);

      //already added !
      if (tmp_hidden[0] != 'A')ListView_SetItemText(hlstv,item_id,col_ref,hidden);
    }else if (msg)
    {
        MessageBox(NULL,"This file/process not in the Data Base :)","Check",MB_OK|MB_TOPMOST);
    }
  }

}
//------------------------------------------------------------------------------
DWORD WINAPI checkAllLstvItem(LPVOID lParam)
{
  DWORD col_ref = (DWORD)lParam;
  DWORD col_file = 1; //default : file

  if (col_ref == 14)col_file = 0;
  DWORD i, nb_item = ListView_GetItemCount(hlstv);
  for (i = 0; i<nb_item && search_rootkit; i++)
  {
    checkLstvItemId(hlstv, hlstv_db, i, col_file,17, col_ref, FALSE);
  }
  search_rootkit = FALSE;
  return 0;
}//------------------------------------------------------------------------------
DWORD WINAPI checkAllLstvItemProcessTools(LPVOID lParam)
{
  DWORD i, nb_item = ListView_GetItemCount(hlstv_process);
  for (i = 0; i<nb_item && search_rootkit_process_tool; i++)
  {
    checkLstvItemId(hlstv_process, hlstv_db, i, 0,18, 14, FALSE);
  }
  search_rootkit_process_tool = FALSE;
  return 0;
}
