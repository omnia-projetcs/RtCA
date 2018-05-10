//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRegistryPathtoDB(char *file, char *hk, char *key, char*value, char *data, char *user, char* RID, char *sid, char *parent_key_update, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_path (file,hk,key,value,data,user,rid,sid,parent_key_update,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           file,hk,key,value,data,user,RID,sid,parent_key_update,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Registry_Path\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,hk,key,value,data,user,RID,sid,parent_key_update,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void reg_read_enum_PathValues(HKEY hk,char *chkey,char *key,unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,key,&CleTmp)!=ERROR_SUCCESS)return;

  DWORD nbValue,i,j;
  FILETIME last_update;
  if (RegQueryInfoKey (CleTmp,0,0,0,0,0,0,&nbValue,0,0,0,&last_update)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }

  //get date
  char parent_key_update[DATE_SIZE_MAX] = "";
  filetimeToString_GMT(last_update, parent_key_update, DATE_SIZE_MAX);

  //read USER + RID + SID
  char user[MAX_PATH], RID[MAX_PATH], sid[MAX_PATH];
  GetRegistryKeyOwner(CleTmp, user, RID, sid, MAX_PATH);

  //enum values
  char value[MAX_PATH], data[MAX_PATH];
  DWORD valueSize,dataSize,type;
  for (i=0;i<nbValue && start_scan;i++)
  {
    valueSize = MAX_PATH;
    dataSize  = MAX_PATH;
    value[0]  = 0;
    type      = 0;
    if (RegEnumValue (CleTmp,i,(LPTSTR)value,(LPDWORD)&valueSize,0,(LPDWORD)&type,(LPBYTE)data,(LPDWORD)&dataSize)==ERROR_SUCCESS)
    {
      switch(type)
      {
        case REG_EXPAND_SZ:
        case REG_SZ:
          convertStringToSQL(value, MAX_PATH);
          convertStringToSQL(data, MAX_PATH);
          addRegistryPathtoDB("",chkey,key,value,data,user,RID,sid,parent_key_update,session_id,db);break;
        /*case REG_BINARY:
        case REG_LINK:
          {
            tmp[0] = 0;
            snprintf(tmp,MAX_PATH,"%S",data);
            convertStringToSQL(value, MAX_PATH);
            convertStringToSQL(tmp, MAX_PATH);
            addRegistryPathtoDB("",chkey,key,value,tmp,user,RID,sid,parent_key_update,session_id,db);
          }
        break;*/
        case REG_MULTI_SZ:
          for (j=0;j<dataSize;j++)
          {
            if (data[j] == 0)data[j]=';';
          }
          convertStringToSQL(value, MAX_PATH);
          convertStringToSQL(data, MAX_PATH);
          addRegistryPathtoDB("",chkey,key,value,data,user,RID,sid,parent_key_update,session_id,db);
        break;
      }
    }
  }
  RegCloseKey(CleTmp);
}
//------------------------------------------------------------------------------
//numPath_local(HKEY_USERS,"HKEY_USERS","","Environment",session_id,db);
void EnumPath_local(HKEY hk,char *chk, char*key_before,char *key_after,unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,key_before,&CleTmp)!=ERROR_SUCCESS)return;

  DWORD nbSubKey=0,i;
  if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }

  char key[MAX_PATH], tmp_key[MAX_PATH];
  DWORD key_size;
  for (i=0;i<nbSubKey && start_scan;i++)
  {
    key[0]    = 0;
    key_size  = MAX_PATH;
    if (RegEnumKeyEx (CleTmp,i,key,(LPDWORD)&key_size,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
    {
      if (key_before==NULL)     snprintf(tmp_key,MAX_PATH,"%s\\%s\\"               ,key,key_after);
      else if (key_after==NULL) snprintf(tmp_key,MAX_PATH,"%s\\%s\\"    ,key_before,key);
      else                      snprintf(tmp_key,MAX_PATH,"%s\\%s\\%s\\",key_before,key,key_after);

      reg_read_enum_PathValues(hk,chk,tmp_key,session_id,db);
    }
  }
  RegCloseKey(CleTmp);
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
void EnumPath_file(HK_F_OPEN *hks, char*key_before,char *key_after,unsigned int session_id, sqlite3 *db, BOOL direct)
{
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer,hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,key_before);
  if (nk_h == NULL)return;

  char parent_key_update[DATE_SIZE_MAX] = "";
  char RID[MAX_PATH], sid[MAX_PATH];
  char value[MAX_PATH], data[MAX_PATH];

  char tmp_key[MAX_PATH],key_path[MAX_PATH];
  HBIN_CELL_NK_HEADER *nk_h_tmp;
  DWORD i,k, nbSubValue, nbSubKey;

  if(direct)
  {
    //get nk of key :)
    nk_h_tmp = GetRegistryNK(hks->buffer,hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,key_before);
    if (nk_h_tmp == NULL)return;

    //key update
    Readnk_Infos(hks->buffer,hks->taille_fic, (hks->pos_fhbin), hks->position,
                 NULL, nk_h_tmp, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);
    //get values
    nbSubValue = GetValueData(hks->buffer,hks->taille_fic, nk_h_tmp, (hks->pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
    for (k=0;k<nbSubValue && start_scan;k++)
    {
      if (GetValueData(hks->buffer,hks->taille_fic, nk_h_tmp, (hks->pos_fhbin)+HBIN_HEADER_SIZE, k,value,MAX_PATH,data,MAX_PATH))
      {
        //save
        convertStringToSQL(value, MAX_PATH);
        convertStringToSQL(data, MAX_PATH);
        addRegistryPathtoDB(hks->file,"",key_before,value,data,"",RID,sid,parent_key_update,session_id,db);
      }
    }
  }else
  {
    nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
    for (i=0;i<nbSubKey && start_scan;i++)
    {
      //for each subkey
      if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, i, tmp_key, MAX_PATH))
      {
        if (key_after!=NULL) snprintf(key_path,MAX_PATH,"%s\\%s\\%s",key_before,tmp_key,key_after);
        else snprintf(key_path,MAX_PATH,"%s\\%s",key_before,tmp_key);

        //get nk of key :)
        nk_h_tmp = GetRegistryNK(hks->buffer,hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,key_path);
        if (nk_h_tmp == NULL)continue;

        //key update
        Readnk_Infos(hks->buffer,hks->taille_fic, (hks->pos_fhbin), hks->position,
                     NULL, nk_h_tmp, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);
        //get values
        nbSubValue = GetValueData(hks->buffer,hks->taille_fic, nk_h_tmp, (hks->pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
        for (k=0;k<nbSubValue && start_scan;k++)
        {
          if (GetValueData(hks->buffer,hks->taille_fic, nk_h_tmp, (hks->pos_fhbin)+HBIN_HEADER_SIZE, k,value,MAX_PATH,data,MAX_PATH))
          {
            //save
            convertStringToSQL(value, MAX_PATH);
            convertStringToSQL(data, MAX_PATH);
            addRegistryPathtoDB(hks->file,"",key_path,value,data,"",RID,sid,parent_key_update,session_id,db);
          }
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_path(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  char file[MAX_PATH];
  HK_F_OPEN hks;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Registry_Path\";\"file\";\"hk\";\"key\";\"value\";\"data\";\"user\";\"rid\";\"sid\";\"parent_key_update\";\"session_id\";\r\n");
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
        //open file + verify
        if(OpenRegFiletoMem(&hks, file))
        {
          //enum all class open/edit/print values
          EnumPath_file(&hks,"Classes","shell\\open\\command",session_id,db, FALSE);
          //Enum envs
          EnumPath_file(&hks,"Environment","",session_id,db, TRUE);
          //Users folders
          EnumPath_file(&hks,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders","",session_id,db, TRUE);
          //all applications
          EnumPath_file(&hks,"Microsoft\\Windows\\CurrentVersion\\App Paths","",session_id,db, FALSE);
          EnumPath_file(&hks,"Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths","",session_id,db, FALSE);
          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    //enum all class open/edit/print values
    EnumPath_local(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Classes","shell\\open\\command",session_id,db);
    //Enum envs
    EnumPath_local(HKEY_USERS,"HKEY_USERS",NULL,"Environment",session_id,db);
    //Users folders
    EnumPath_local(HKEY_USERS,"HKEY_USERS",NULL,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders",session_id,db);
    //all applications
    EnumPath_local(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths",NULL,session_id,db);
    EnumPath_local(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\App Paths",NULL,session_id,db);
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
