//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRegistryServicetoDB(char *file, char *hk, char *key, char*name,
                            DWORD state_id, char*path, char*description, DWORD type_id,
                             char *last_update,unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_service_driver (file,hk,key,name,state_id,path,type_id,last_update,session_id,description) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",%lu,\"%s\",%lu,\"%s\",%d,\"%s\");",
           file,hk,key,CheckNameAndDescription(name,MAX_PATH),state_id,path,type_id,last_update,session_id,CheckNameAndDescription(description,MAX_PATH));

  //if description too long
  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4-strlen(request));

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Registry_Service\";\"%s\";\"%s\";\"%s\";\"%s\";\"%lu\";\"%s\";\"%lu\";\"%s\";\"%d\";\"%s\";\r\n",
         file,hk,key,name,state_id,path,type_id,last_update,session_id,description);
  #endif
}
//------------------------------------------------------------------------------
char *CheckNameAndDescription(char *name, unsigned int name_size_max)
{
 /* if (name != NULL)
  {
    if (name[0] == '@')
    {
      //get dll path
      char tmp[MAX_LINE_DBSIZE]="", dll_path[MAX_PATH]="";
      char *t = tmp;
      char *s = name+1; //pass '@'
      while (*s && *s != ',')*t++ = *s++;
      *t = 0;
      if (strlen(tmp) > 0)
      {
        ExpandEnvironmentStrings(tmp,dll_path,MAX_PATH);

        //get id :
        if (*s == ',')s++;
        if (*s == '-')s++;

        //load message
        HANDLE mhandle = LoadLibraryEx(dll_path, 0, LOAD_LIBRARY_AS_DATAFILE);
        if (mhandle != NULL)
        {
          tmp[0] = 0;
          if (FormatMessage(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_FROM_SYSTEM,
                        mhandle,atoi(s),0,tmp,MAX_LINE_DBSIZE,NULL))
          {
            if (tmp[0] != 0)snprintf(name,name_size_max,"%s",tmp);
          }else printf("FormatMessage : %s [%d] (%s)\n",dll_path,atoi(s),name);
          FreeLibrary(mhandle);
        }else printf("mhandle : %s [%d] (%s)\n",dll_path,atoi(s),name);
      }
    }
  }*/
  return name;
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void Scan_registry_service_local(char *ckey, sqlite3 *db, unsigned int session_id)
{
  HKEY CleTmp;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,ckey,&CleTmp)==ERROR_SUCCESS)
  {
    DWORD i,nbSubKey = 0;
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      FILETIME LastWriteTime;
      char key[MAX_PATH],key_path[MAX_PATH];
      DWORD key_size;
      DWORD state_id,type_id;
      char lastupdate[DATE_SIZE_MAX],
      name[MAX_PATH],path[MAX_PATH],description[MAX_PATH];

      for (i=0;i<nbSubKey && start_scan;i++)
      {
        key_size  = MAX_PATH;
        key[0]    = 0;
        if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,&LastWriteTime)==ERROR_SUCCESS)
        {
          //path
          snprintf(key_path,MAX_PATH,"%s%s",ckey,key);

          //read values
          name[0]       = 0;
          path[0]       = 0;
          description[0]= 0;
          lastupdate[0] = 0;

          //name
          if (ReadValue(HKEY_LOCAL_MACHINE,key_path,"DisplayName",name, MAX_PATH) == 0)
          {
            if (ReadValue(HKEY_LOCAL_MACHINE,key_path,"Group",name, MAX_PATH) == 0)continue;

            snprintf(name,MAX_PATH,"%s",key);
          }

          //state id
          state_id = ReadDwordValue(HKEY_LOCAL_MACHINE,key_path,"Start");
          switch(state_id)
          {
            case 0: state_id=210;break;//Kernel module   : 210
            case 1: state_id=211;break;//Start by system : 211
            case 2: state_id=212;break;//Automatic start : 212
            case 3: state_id=213;break;//Manual start    : 213
            case 4: state_id=214;break;//Disable         : 214
            default:state_id=215;break;//Unknow          : 215
          }

          //path : ImagePath
          ReadValue(HKEY_LOCAL_MACHINE,key_path,"ImagePath",path, MAX_PATH);

          //description : Description
          if(ReadValue(HKEY_LOCAL_MACHINE,key_path,"Description",description, MAX_PATH) == 0)
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"Group",description, MAX_PATH);

          //type_id
          type_id = ReadDwordValue(HKEY_LOCAL_MACHINE,key_path,"Type");
          if (type_id == 1)type_id = 200; //SERVICE : 200
          else type_id = 201;             //DRIVER  : 201

          //last update
          filetimeToString_GMT(LastWriteTime, lastupdate, DATE_SIZE_MAX);

          convertStringToSQL(path, MAX_PATH);
          convertStringToSQL(description, MAX_PATH);

          addRegistryServicetoDB("", "HKEY_LOCAL_MACHINE", key_path, name,
                                  state_id, path, description, type_id,
                                  lastupdate, session_id, db);
        }
      }
    }
    RegCloseKey(CleTmp);
  }
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
void Scan_registry_service_file(HK_F_OPEN *hks, char *ckey, unsigned int session_id, sqlite3 *db)
{
  //exist or not in the file ?
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, ckey);
  if (nk_h == NULL)return;

  char tmp_key[MAX_PATH],key_path[MAX_PATH],state[MAX_PATH];
  DWORD state_id,type_id;
  char lastupdate[DATE_SIZE_MAX],
  name[MAX_PATH],path[MAX_PATH],description[MAX_PATH];

  HBIN_CELL_NK_HEADER *nk_h_tmp;
  DWORD i,nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
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

      if (Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"DisplayName", name, MAX_PATH)==FALSE)
      {
        if (Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Group", name, MAX_PATH)==FALSE)continue;

        snprintf(name,MAX_PATH,"%s",tmp_key);
      }

      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Start", state, MAX_PATH))
      {
             if (strcmp(state,"0x00000000") == 0)state_id=210;//Kernel module   : 210
        else if (strcmp(state,"0x00000001") == 0)state_id=211;//Start by system : 211
        else if (strcmp(state,"0x00000002") == 0)state_id=212;//Automatic start : 212
        else if (strcmp(state,"0x00000003") == 0)state_id=213;//Manual start    : 213
        else if (strcmp(state,"0x00000004") == 0)state_id=214;//Disable         : 214
        else state_id=215;                                    //Unknow          : 215
      }else state_id = 0;

      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"ImagePath", path, MAX_PATH);

      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Description", description, MAX_PATH)==FALSE)
        Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Group", description, MAX_PATH);


      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"Type", state, MAX_PATH);

      if (strcmp(state,"0x00000001") == 0)     type_id = 200;//Kernel driver
      else if (strcmp(state,"0x00000002") == 0)type_id = 201;//File system driver
      else if (strcmp(state,"0x00000010") == 0)type_id = 202;//Own process
      else if (strcmp(state,"0x00000020") == 0)type_id = 203;//Share process
      else if (strcmp(state,"0x00000100") == 0)type_id = 204;//Interactive
      else type_id = 215;

      Readnk_Infos(hks->buffer, hks->taille_fic, (hks->pos_fhbin), hks->position, NULL, nk_h_tmp,
                   lastupdate, DATE_SIZE_MAX, NULL, 0, NULL, 0);

      convertStringToSQL(path, MAX_PATH);
      convertStringToSQL(description, MAX_PATH);
      addRegistryServicetoDB(hks->file, "", key_path, name,
                             state_id, path, description, type_id,
                             lastupdate, session_id, db);
    }
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_service(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  char file[MAX_PATH];
  HK_F_OPEN hks;
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Registry_Service\";\"file\";\"hk\";\"key\";\"name\";\"state_id\";\"path\";\"type_id\";\"last_update\";\"session_id\";\"description\";\r\n");
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
          Scan_registry_service_file(&hks,"ControlSet001\\Services", session_id, db);
          Scan_registry_service_file(&hks,"ControlSet002\\Services", session_id, db);
          Scan_registry_service_file(&hks,"ControlSet003\\Services", session_id, db);
          Scan_registry_service_file(&hks,"ControlSet004\\Services", session_id, db);

          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else Scan_registry_service_local("SYSTEM\\CurrentControlSet\\Services\\",db, session_id);

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
