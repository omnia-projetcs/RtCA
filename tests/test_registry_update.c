//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRegistryUpdatetoDB(char *file, char *hk, char *key, char*component,char *name,
                             char *publisher, char*description_package_name,
                             char *install_date_update,unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_update (file,hk,key,component,name,publisher,description_package_name,install_date_update,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           file,hk,key,component,name,publisher,description_package_name,install_date_update,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Update\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,hk,key,component,name,publisher,description_package_name,install_date_update,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void EnumUpdate(HKEY hk, char *chk,char *path, unsigned int session_id, sqlite3 *db, BOOL holder)
{
  HKEY CleTmp,CleTmp2,CleTmp3;
  char tmp_key[MAX_PATH]="",ttmp_key[MAX_PATH],tttmp_key[MAX_PATH],key[MAX_PATH],key2[MAX_PATH],key3[MAX_PATH];
  DWORD i,j,k,nbSubKey=0,nbSubKey2,nbSubKey3,key_size,key_size2,key_size3;
  BOOL ok;
  if (RegOpenKey(hk,path,&CleTmp)==ERROR_SUCCESS)
  {
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      char InstalledBy[MAX_PATH],
      description_package_name[MAX_PATH],
      name[MAX_PATH],
      lastupdate[DATE_SIZE_MAX];
      FILETIME LastWriteTime;
      for (i=0;i<nbSubKey && start_scan;i++)
      {
        if (holder)
        {
          key_size  = MAX_PATH;
          key[0]    = 0;
          if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,&LastWriteTime)==ERROR_SUCCESS)
          {
            snprintf(ttmp_key,MAX_PATH,"%s%s\\",path,key);

            ok = FALSE;
            description_package_name[0] = 0;
            name[0]                     = 0;
            if (ReadValue(hk,ttmp_key,"Description",description_package_name, MAX_PATH))ok = TRUE;
            if (ReadValue(hk,ttmp_key,"PackageName",name, MAX_PATH))ok = TRUE;
            else if (ReadValue(hk,ttmp_key,"InstallName",name, MAX_PATH))ok = TRUE;

            if (ok)
            {
              InstalledBy[0]        = 0;

              //InstalledBy
              if(!ReadValue(hk,ttmp_key,"InstalledBy",InstalledBy, MAX_PATH))
                ReadValue(hk,ttmp_key,"InstallUser",InstalledBy, MAX_PATH);

              //last update
              filetimeToString_GMT(LastWriteTime, lastupdate, DATE_SIZE_MAX);

              convertStringToSQL(description_package_name, MAX_PATH);
              addRegistryUpdatetoDB("", chk, tmp_key, key, name,
                                    InstalledBy, description_package_name, lastupdate, session_id, db);
            }
            //other package
            if (RegOpenKey(hk,ttmp_key,&CleTmp2)==ERROR_SUCCESS)
            {
              nbSubKey2 = 0;
              if (RegQueryInfoKey (CleTmp2,0,0,0,&nbSubKey2,0,0,0,0,0,0,0)==ERROR_SUCCESS)
              {
                for (j=0;j<nbSubKey2;j++)
                {
                  key_size2 = MAX_PATH;
                  key2[0]   = 0;
                  if (RegEnumKeyEx (CleTmp2,j,key2,&key_size2,0,0,0,&LastWriteTime)==ERROR_SUCCESS)
                  {
                    snprintf(tttmp_key,MAX_PATH,"%s%s\\",ttmp_key,key2);
                    if (RegOpenKey(hk,tttmp_key,&CleTmp3)==ERROR_SUCCESS)
                    {
                      nbSubKey3 = 0;
                      if (RegQueryInfoKey (CleTmp3,0,0,0,&nbSubKey3,0,0,0,0,0,0,0)==ERROR_SUCCESS)
                      {
                        for (k=0;k<nbSubKey3;k++)
                        {
                          key_size3 = MAX_PATH;
                          key3[0]   = 0;
                          if (RegEnumKeyEx (CleTmp3,k,key3,&key_size3,0,0,0,&LastWriteTime)==ERROR_SUCCESS)
                          {
                            snprintf(tmp_key,MAX_PATH,"%s%s\\",tttmp_key,key3);

                            name[0]                     = 0;
                            InstalledBy[0]                = 0;
                            description_package_name[0] = 0;

                            //InstalledBy
                            ReadValue(hk,tmp_key,"InstalledBy",InstalledBy, MAX_PATH);

                            //description_package_name
                            ReadValue(hk,tmp_key,"Description",description_package_name, MAX_PATH);

                            //last update
                            filetimeToString_GMT(LastWriteTime, lastupdate, DATE_SIZE_MAX);

                            convertStringToSQL(description_package_name, MAX_PATH);
                            addRegistryUpdatetoDB("", chk, tmp_key, key, key3,
                                                  InstalledBy, description_package_name, lastupdate, session_id, db);
                          }
                        }
                      }
                      RegCloseKey(CleTmp3);
                    }
                  }
                }
              }
              RegCloseKey(CleTmp2);
            }
          }
        }else
        {
          key_size = MAX_PATH;
          if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,&LastWriteTime)==ERROR_SUCCESS)
          {
            snprintf(tmp_key,MAX_PATH,"%s%s\\",path,key);

            name[0]                     = 0;
            InstalledBy[0]              = 0;
            description_package_name[0] = 0;

            //InstalledBy
            ReadValue(hk,tmp_key,"InstallUser",InstalledBy, MAX_PATH);

            //description_package_name
            ReadValue(hk,tmp_key,"InstallLocation",description_package_name, MAX_PATH);
            ReadValue(hk,tmp_key,"InstallName",name, MAX_PATH);

            //last update
            filetimeToString_GMT(LastWriteTime, lastupdate, DATE_SIZE_MAX);

            convertStringToSQL(description_package_name, MAX_PATH);
            addRegistryUpdatetoDB("", chk, tmp_key, key, name,
                                  InstalledBy, description_package_name, lastupdate, session_id, db);
          }
        }
      }
    }
    RegCloseKey(CleTmp);
  }
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
void EnumUpdate_file(HK_F_OPEN *hks, char *ckey, unsigned int session_id, sqlite3 *db, BOOL holder)
{
  //exist or not in the file ?
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, ckey);
  if (nk_h == NULL)return;

  char tmp_key[MAX_PATH], tmp_key2[MAX_PATH], tmp_key3[MAX_PATH], key_path[MAX_PATH];
  char InstalledBy[MAX_PATH],
  description_package_name[MAX_PATH],
  name[MAX_PATH],
  lastupdate[DATE_SIZE_MAX];

  HBIN_CELL_NK_HEADER *nk_h_tmp, *nk_h_tmp2, *nk_h_tmp3;
  DWORD i,j,k, nbSubKey2,nbSubKey3,nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
  if(holder)
  {
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

        if (Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Description", description_package_name, MAX_PATH) ||
            Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"PackageName", name, MAX_PATH))
        {
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstalledBy", InstalledBy, MAX_PATH);

          Readnk_Infos(hks->buffer, hks->taille_fic, (hks->pos_fhbin), hks->position, NULL, nk_h_tmp,
                       lastupdate, DATE_SIZE_MAX, NULL, 0, NULL, 0);

          convertStringToSQL(description_package_name, MAX_PATH);
          addRegistryUpdatetoDB(hks->file, "", key_path, tmp_key, name,
                                InstalledBy, description_package_name, lastupdate, session_id, db);
        }

        //other package
        nbSubKey2 = GetSubNK(hks->buffer, hks->taille_fic, nk_h_tmp, hks->position, 0, NULL, 0);
        for (j=0;j<nbSubKey2;j++)
        {
          //for each subkey
          if(GetSubNK(hks->buffer, hks->taille_fic, nk_h_tmp, hks->position, j, tmp_key2, MAX_PATH))
          {
            //get nk of key :)
            nk_h_tmp2 = GetSubNKtonk(hks->buffer, hks->taille_fic, nk_h_tmp, hks->position, j);
            if (nk_h_tmp2 == NULL)continue;

            nbSubKey3 = GetSubNK(hks->buffer, hks->taille_fic, nk_h_tmp2, hks->position, 0, NULL, 0);
            for (k=0;k<nbSubKey3;k++)
            {
              //for each subkey
              if(GetSubNK(hks->buffer, hks->taille_fic, nk_h_tmp2, hks->position, k, tmp_key3, MAX_PATH))
              {
                //get nk of key :)
                nk_h_tmp3 = GetSubNKtonk(hks->buffer, hks->taille_fic, nk_h_tmp2, hks->position, k);
                if (nk_h_tmp3 == NULL)continue;

                //read datas ^^
                snprintf(key_path,MAX_PATH,"%s\\%s\\%s\\%s",ckey,tmp_key,tmp_key2,tmp_key3);

                Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp3,"Description", description_package_name, MAX_PATH);
                Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp3,"InstalledBy", InstalledBy, MAX_PATH);

                Readnk_Infos(hks->buffer, hks->taille_fic, (hks->pos_fhbin), hks->position, NULL, nk_h_tmp3,
                             lastupdate, DATE_SIZE_MAX, NULL, 0, NULL, 0);

                convertStringToSQL(description_package_name, MAX_PATH);
                addRegistryUpdatetoDB(hks->file, "", key_path, tmp_key, tmp_key3,
                                      InstalledBy, description_package_name, lastupdate, session_id, db);
              }
            }
          }
        }
      }
    }
  }else
  {
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

        Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstalledBy", InstalledBy, MAX_PATH);
        Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstallLocation", description_package_name, MAX_PATH);
        Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstallName", name, MAX_PATH);


        Readnk_Infos(hks->buffer, hks->taille_fic, (hks->pos_fhbin), hks->position, NULL, nk_h_tmp,
                     lastupdate, DATE_SIZE_MAX, NULL, 0, NULL, 0);

        convertStringToSQL(description_package_name, MAX_PATH);
        addRegistryUpdatetoDB(hks->file, "", key_path, tmp_key, name,
                              InstalledBy, description_package_name, lastupdate, session_id, db);
      }
    }
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_update(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  char file[MAX_PATH];
  HK_F_OPEN hks;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Update\";\"file\";\"hk\";\"key\";\"component\";\"name\";\"publisher\";\"description_package_name\";\"install_date_update\";\"session_id\";\r\n");
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
          EnumUpdate_file(&hks,"Microsoft\\Updates", session_id, db, TRUE);
          EnumUpdate_file(&hks,"Wow6432Node\\Microsoft\\Updates", session_id, db, TRUE);
          EnumUpdate_file(&hks,"Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\Packages", session_id, db, FALSE);
          EnumUpdate_file(&hks,"Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\Packages", session_id, db, FALSE);

          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    EnumUpdate(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Microsoft\\Updates\\", session_id, db, TRUE);
    EnumUpdate(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Wow6432Node\\Microsoft\\Updates\\", session_id, db, TRUE);
    EnumUpdate(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\Packages\\", session_id, db, FALSE);
    EnumUpdate(HKEY_LOCAL_MACHINE,"HKEY_LOCAL_MACHINE","SOFTWARE\\Wow6432Node\\Microsoft\\Windows\\CurrentVersion\\Component Based Servicing\\Packages\\", session_id, db, FALSE);
  }



  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
