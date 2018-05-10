//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRegistrySoftwaretoDB(char *file, char *hk, char *key, char*name,
                             char *publisher, char*uninstallstring, char*installlocation, char *install_date_update,
                             char *install_user, char*url,char*source,char*valid,unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_software (file,hk,key,name,publisher,uninstallstring,installlocation,install_date_update,install_user,url,source,valid,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           file,hk,key,name,publisher,uninstallstring,installlocation,install_date_update,install_user,url,source,valid,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Software\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,hk,key,name,publisher,uninstallstring,installlocation,install_date_update,install_user,url,source,valid,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void Scan_registry_softwareKey(HKEY hk, char *chk, char *ckey, sqlite3 *db, unsigned int session_id)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,ckey,&CleTmp)==ERROR_SUCCESS)
  {
    DWORD nbSubKey = 0;
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      FILETIME LastWriteTime;
      char key[MAX_PATH], lastupdate[DATE_SIZE_MAX],key_path[MAX_PATH];
      char name[MAX_PATH],version[MAX_PATH],publisher[MAX_PATH],uninstallstring[MAX_PATH],
      installlocation[MAX_PATH],install_user[MAX_PATH],url[MAX_PATH],source[MAX_PATH];
      DWORD i, key_size, tmp_size;
      for (i=0;i<nbSubKey && start_scan;i++)
      {
        key_size  = MAX_PATH;
        key[0]    = 0;
        if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,&LastWriteTime)==ERROR_SUCCESS)
        {
          //init
          name[0]             = 0;
          version[0]          = 0;
          publisher[0]        = 0;
          uninstallstring[0]  = 0;
          installlocation[0]  = 0;
          install_user[0]     = 0;
          url[0]              = 0;
          source[0]           = 0;

          //path
          snprintf(key_path,MAX_PATH,"%s%s",ckey,key);

          //name
          if(ReadValue(hk,key_path,"DisplayName",name, MAX_PATH)==0)continue;
          //version
          if (ReadValue(hk,key_path,"DisplayVersion",version, MAX_PATH))
          {
            tmp_size = strlen(name);
            if (tmp_size < MAX_PATH)
            {
              snprintf(name+tmp_size,MAX_PATH-tmp_size," (%s)",version);
            }
          }

          //publisher
          ReadValue(hk,key_path,"Publisher",publisher, MAX_PATH);

          //uninstallstring
          if (ReadValue(hk,key_path,"UninstallString",uninstallstring, MAX_PATH) == 0)
            ReadValue(hk,key_path,"QuietUninstallString",uninstallstring, MAX_PATH);

          //installlocation
          if (ReadValue(hk,key_path,"InstallLocation",installlocation, MAX_PATH) == 0)
            if (ReadValue(hk,key_path,"Inno Setup: App Path",installlocation, MAX_PATH) == 0)
              ReadValue(hk,key_path,"RegistryLocation",installlocation, MAX_PATH);

          //last update
          filetimeToString_GMT(LastWriteTime, lastupdate, DATE_SIZE_MAX);

          //install_user
          ReadValue(hk,key_path,"Inno Setup: User",install_user, MAX_PATH);

          //url
          if (ReadValue(hk,key_path,"URLInfoAbout",url, MAX_PATH) == 0)
            if (ReadValue(hk,key_path,"URLUpdateInfo",url, MAX_PATH) == 0)
              if (ReadValue(hk,key_path,"HelpLink",url, MAX_PATH) == 0)
                ReadValue(hk,key_path,"Readme",url, MAX_PATH);

          //source
          ReadValue(hk,key_path,"InstallSource",source, MAX_PATH);

          //for bad datas with "
          convertStringToSQL(name, MAX_PATH);
          convertStringToSQL(publisher, MAX_PATH);
          convertStringToSQL(uninstallstring, MAX_PATH);
          convertStringToSQL(installlocation, MAX_PATH);
          convertStringToSQL(install_user, MAX_PATH);
          convertStringToSQL(url, MAX_PATH);
          convertStringToSQL(source, MAX_PATH);

          addRegistrySoftwaretoDB("", chk, key_path, name, publisher, uninstallstring,
                                  installlocation, lastupdate, install_user, url, source, "", session_id, db);
        }
      }
    }
    RegCloseKey(CleTmp);
  }
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
void Scan_registry_softwareKey_file(HK_F_OPEN *hks, char *ckey, sqlite3 *db, unsigned int session_id)
{
  //exist or not in the file ?
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, ckey);
  if (nk_h == NULL)return;

  //get subkey
  HBIN_CELL_NK_HEADER *nk_h_tmp;
  char lastupdate[DATE_SIZE_MAX],key_path[MAX_PATH], tmp_key[MAX_PATH];
  char name[MAX_PATH],version[MAX_PATH],publisher[MAX_PATH],uninstallstring[MAX_PATH],
       installlocation[MAX_PATH],install_user[MAX_PATH],url[MAX_PATH],source[MAX_PATH];

  DWORD tmp_size, i, nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
  for (i=0;i<nbSubKey;i++)
  {
    //for each subkey
    if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, i, tmp_key, MAX_PATH))
    {
      //get nk of key :)
      nk_h_tmp = GetSubNKtonk(hks->buffer, hks->taille_fic, nk_h, hks->position, i);
      if (nk_h_tmp == NULL)continue;

      //read datas ^^
      snprintf(key_path,MAX_PATH,"%s\\%s",ckey,tmp_key);

      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"DisplayName", name, MAX_PATH)==FALSE)continue;
      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"DisplayVersion", version, MAX_PATH)==TRUE)
      {
        tmp_size = strlen(name);
        if (tmp_size < MAX_PATH)
        {
          snprintf(name+tmp_size,MAX_PATH-tmp_size," (%s)",version);
        }
      }

      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Publisher", publisher, MAX_PATH);

      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"UninstallString", uninstallstring, MAX_PATH)==FALSE)
        Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"QuietUninstallString", uninstallstring, MAX_PATH);

      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstallLocation", installlocation, MAX_PATH)==FALSE)
        if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Inno Setup: App Path", installlocation, MAX_PATH)==FALSE)
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"RegistryLocation", installlocation, MAX_PATH);

      Readnk_Infos(hks->buffer, hks->taille_fic, (hks->pos_fhbin), hks->position, NULL, nk_h_tmp,
                   lastupdate, DATE_SIZE_MAX, NULL, 0, NULL, 0);

      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Inno Setup: User", install_user, MAX_PATH);

      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"URLInfoAbout", url, MAX_PATH)==FALSE)
        if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"URLUpdateInfo", url, MAX_PATH)==FALSE)
          if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"HelpLink", url, MAX_PATH)==FALSE)
            Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Readme", url, MAX_PATH);

      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstallSource", source, MAX_PATH);

      //for bad datas with "
      convertStringToSQL(name, MAX_PATH);
      convertStringToSQL(publisher, MAX_PATH);
      convertStringToSQL(uninstallstring, MAX_PATH);
      convertStringToSQL(installlocation, MAX_PATH);
      convertStringToSQL(install_user, MAX_PATH);
      convertStringToSQL(url, MAX_PATH);
      convertStringToSQL(source, MAX_PATH);

      addRegistrySoftwaretoDB(hks->file, "", key_path, name, publisher, uninstallstring,
                              installlocation, lastupdate, install_user, url, source, "", session_id, db);
    }
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_software(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  char file[MAX_PATH];
  HK_F_OPEN hks;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Software\";\"file\";\"hk\";\"key\";\"name\";\"publisher\";\"uninstallstring\";\"installlocation\";\"install_date_update\";\"install_user\";\"url\";\"source\";\"valid\";\"session_id\";\r\n");
  #endif
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

        //open file + verify
        if(OpenRegFiletoMem(&hks, file))
        {
          Scan_registry_softwareKey_file(&hks,"Microsoft\\Windows\\CurrentVersion\\Uninstall",db,session_id);
          Scan_registry_softwareKey_file(&hks,"Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall",db,session_id);

          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    Scan_registry_softwareKey(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\",db,session_id);
    Scan_registry_softwareKey(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\",db,session_id);
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
