//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addShellBags(char *source, char*key, char*value, char*data, char *sid, char *last_update,unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_shell_bags (source,key,value,data,sid,last_update,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           source,key,value,data,sid,last_update,session_id);

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Shell_Bags\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         source,key,value,data,sid,last_update,session_id);
  #endif
}
//------------------------------------------------------------------------------
  typedef struct
  {
    unsigned short struct_size;
    unsigned char type;
    unsigned char padding;
    DWORD file_size;
    DWORD create_time;
    unsigned short type2;
  }START_SHELLBAG;
//http://www.williballenthin.com/forensics/shellbags/index.html
BOOL Read_ShellBags_Datas(char *srchk, char *ckey, char *SID, char *last_update, char *value, char*data, char *pathcmd, char *file, DWORD file_sz, DWORD dataSize, unsigned int session_id, sqlite3 *db, BOOL local)
{
  //read date
  START_SHELLBAG *ss = (START_SHELLBAG *)data;
  file[0] = 0;

  BOOL checklongpath = FALSE;

  //check if datas are valid !
  if (ss->struct_size < 0x15 || ss->type == 0x71 || ss->type == 0x2e || ss->type == 0x00) return FALSE;
  /*exploit GUId her*/

  if (ss->type == 0x2f)
  {
    //letter
    snprintf(file,file_sz,"%s",&data[3]);
  }else if ((ss->type == 0x15 || ss->type == 0xC4 || ss->type == 0x38 || ss->type == 0x39 || ss->type == 0x43|| ss->type == 0x31 || ss->type == 0x32 || ss->type == 0x35 || ss->type == 0x3C) && data[dataSize-6] != 0x2F)//in ZIP file + file + directory
  {
    //read full file name
    char *c = &data[dataSize-8];

    while (*c != 0x00 || *(c-1) != 0x00) c--;
    c++;

    //by passe W10 formats
    if (*(c+1) != 0x00) c+=4;

    snprintf(file,file_sz,"%S",c);

    //checks for spec files formats
    if (ss->type == 0x31 && file[0] == '@')
    {
      c-=4;
      while (*c != 0x00 || *(c-1) != 0x00) c--;
      c+=1;
      snprintf(file,file_sz,"%S",c);
    }
  }else if (ss->type == 0x12 || ss->type == 0x15 || ss->type == 0xC0 || ss->type == 0xC5 || ss->type == 0x18 || data[dataSize-6] == 0x2F) //file in ZIP file
  {
    //read full file name
    char *c = &data[dataSize-8];

    //last is the directory name (already)
    while (*c != 0x00 || *(c-1) != 0x00) c--;
    c++;
    char *d = c;
    //file name
    c-=4;
    while (*c != 0x00 || *(c-1) != 0x00) c--;
    c++;

    snprintf(file,file_sz,"%S",c);
    if (strlen(file) < 1)snprintf(file,file_sz,"%S",d);
  }else
  {
    //direct share path
    snprintf(file,file_sz,"%s",&data[5]);
    checklongpath = TRUE;
  }

  //for specifics formats !
  if (file[0] > 126 || file[0] < 32)
  {
    snprintf(file,file_sz,"%s",&data[14]);
    checklongpath = TRUE;
  }

  if (pathcmd != NULL)
  {
    char globalpath[MAX_LINE_SIZE]="";

    if (ss->type != 0x2f) //start path letter
    {
      if (file[0] == '\\' || file[0] == '/' || file[1] == ':' || pathcmd[strlen(pathcmd)-1] == '/' || pathcmd[strlen(pathcmd)-1] == '\\')snprintf(globalpath,MAX_LINE_SIZE,"%s%s",pathcmd,file);
      else snprintf(globalpath,MAX_LINE_SIZE,"%s\\%s",pathcmd,file);

      if (checklongpath && local)
      {
        char tmp_f[MAX_LINE_SIZE]="";
        if (GetLongPathName(globalpath,tmp_f,MAX_LINE_SIZE) > 0)
        {
          addShellBags(srchk, ckey, value, tmp_f, SID, last_update,session_id, db);
          return TRUE;
        }
      }

      addShellBags(srchk, ckey, value, globalpath, SID, last_update,session_id, db);
    }else addShellBags(srchk, ckey, value, file, SID, last_update,session_id, db);
  }else addShellBags(srchk, ckey, value, file, SID, last_update,session_id, db);

  return TRUE;
}
//------------------------------------------------------------------------------
void Scan_registry_ShellBags_file(HK_F_OPEN *hks, char *ckey, char *pathcmd, unsigned int session_id, sqlite3 *db, BOOL first)
{
  //exist or not in the file ?
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, ckey);
  if (nk_h == NULL)return;

  //get key infos
  char SID[MAX_PATH], last_update[DATE_SIZE_MAX];
  Readnk_Infos(hks->buffer, hks->taille_fic, (hks->pos_fhbin), hks->position, NULL, nk_h,
               last_update, DATE_SIZE_MAX, NULL, 0, SID, MAX_PATH);

  //enumerate values
  DWORD i, dataSize, nbSubValue = GetValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
  char value[MAX_PATH], data[MAX_LINE_SIZE], tmpfile[MAX_LINE_SIZE], key_path[MAX_LINE_SIZE];
  char pathcmdtmp[MAX_LINE_SIZE] = "";
  for (i=0;i<nbSubValue && start_scan;i++)
  {
    dataSize  = MAX_LINE_SIZE;
    value[0]  = 0;
    data[0]   = 0;
    if(GetBinaryValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,data, &dataSize))
    {
      //read datas :)
      if (value[0] != 0 && data[0] != 0 && dataSize > 40)
      {
        if (strcmp(value,"MRUListEx")!=0 && strcmp(value,"NodeSlot")!=0 && strcmp(value,"NodeSlots")!=0)
        {
          if (Read_ShellBags_Datas(hks->file,
                                   ckey, SID, last_update, value, data, pathcmd, tmpfile, MAX_LINE_SIZE ,dataSize, session_id, db, FALSE))
          {
            if (pathcmd == NULL)snprintf(pathcmdtmp,MAX_LINE_SIZE,"%s",tmpfile);
            else
            {
              if (tmpfile[0] == '\\' || tmpfile[0] == '/' || tmpfile[1] == ':' || pathcmd[strlen(pathcmd)-1] == '/' || pathcmd[strlen(pathcmd)-1] == '\\')snprintf(pathcmdtmp,MAX_LINE_SIZE,"%s%s",pathcmd, tmpfile);
              else snprintf(pathcmdtmp,MAX_LINE_SIZE,"%s\\%s",pathcmd, tmpfile);
            }
          }
          snprintf(key_path,MAX_LINE_SIZE,"%s\\%s",ckey,value);
          Scan_registry_ShellBags_file(hks, key_path, pathcmdtmp, session_id, db, FALSE);
        }
      }
    }
  }

  //subkeys
  if (first)
  {
    DWORD j,nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
    char tmp_key[MAX_PATH], tmp_ckey[MAX_PATH];
    for (j=0;j<nbSubKey && start_scan;j++)
    {
      if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, j, tmp_key, MAX_PATH))
      {
        if (tmp_key[0] != 0)
        {
          snprintf(tmp_ckey,MAX_PATH,"%s\\%s",ckey,tmp_key);
          Scan_registry_ShellBags_file(hks, tmp_ckey, pathcmdtmp, session_id, db, FALSE);
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
void Enum_ShellBags(char *ckey, char *SID, char *pathcmd, unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey((HKEY)HKEY_USERS,ckey,&CleTmp)!=ERROR_SUCCESS)return;

  DWORD nbSubKey=0, nbSubValue=0, i;
  FILETIME lastupdate;
  char date[DATE_SIZE_MAX]="";
  char key_path[MAX_LINE_SIZE];

  if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,&nbSubValue,NULL,NULL,NULL,&lastupdate)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }
  //date
  filetimeToString_GMT(lastupdate, date, DATE_SIZE_MAX);

  //read values
  DWORD valueSize,dataSize;
  char value[MAX_LINE_SIZE], data[MAX_LINE_SIZE], tmp_path[MAX_LINE_SIZE];
  char pathcmdtmp[MAX_LINE_SIZE] = "", tmpfile[MAX_LINE_SIZE]="";
  for (i=0;i<nbSubValue && start_scan;i++)
  {
    valueSize = MAX_LINE_SIZE;
    dataSize  = MAX_LINE_SIZE;
    value[0]  = 0;
    data[0]  = 0;

    if (RegEnumValue (CleTmp,i,(LPTSTR)value,(LPDWORD)&valueSize,0,NULL,(LPBYTE)data,(LPDWORD)&dataSize)==ERROR_SUCCESS)
    {
      //read datas :)
      if (value[0] != 0 && data[0] != 0 && dataSize && valueSize)
      {
        if (strcmp(value,"MRUListEx")!=0 && strcmp(value,"NodeSlot")!=0 && strcmp(value,"NodeSlots")!=0)
        {
          if(Read_ShellBags_Datas("HKEY_USERS", ckey, SID, date, value, data, pathcmd, tmpfile, MAX_LINE_SIZE, dataSize, session_id, db, TRUE))
          {
            if (pathcmd == NULL)snprintf(pathcmdtmp,MAX_LINE_SIZE,"%s",tmpfile);
            else
            {
              if (tmpfile[0] == '\\' || tmpfile[0] == '/' || tmpfile[1] == ':' || pathcmd[strlen(pathcmd)-1] == '/' || pathcmd[strlen(pathcmd)-1] == '\\')snprintf(pathcmdtmp,MAX_LINE_SIZE,"%s%s",pathcmd, tmpfile);
              else snprintf(pathcmdtmp,MAX_LINE_SIZE,"%s\\%s",pathcmd, tmpfile);

              //check global path !
              tmp_path[0] = 0;
              if (GetLongPathName(pathcmdtmp, tmp_path, MAX_LINE_SIZE) > 0)
              {
                snprintf(pathcmdtmp,MAX_LINE_SIZE,"%s",tmp_path);
              }
            }
          }
          snprintf(key_path,MAX_LINE_SIZE,"%s\\%s",ckey,value);
          Enum_ShellBags(key_path, SID, pathcmdtmp, session_id, db);
        }
      }
      memset(data,0,MAX_LINE_SIZE);
      memset(value,0,MAX_LINE_SIZE);
    }
  }
  RegCloseKey(CleTmp);
}
//------------------------------------------------------------------------------
void Scan_registry_ShellBags_direct(char *ckey, unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey((HKEY)HKEY_USERS,"",&CleTmp)!=ERROR_SUCCESS)return;

  DWORD nbSubKey=0, i,key_size;
  if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }

  //datas
  char SID[MAX_PATH];

  //get all key
  char key_path[MAX_PATH];
  for (i=0;i<nbSubKey && start_scan;i++)
  {
    key_size  = MAX_PATH;
    SID[0]    = 0;
    if (RegEnumKeyEx (CleTmp,i,SID,(LPDWORD)&key_size,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
    {
      //only for reals users
      if (key_size < 10)continue;

      //enumerate datas
      snprintf(key_path,MAX_PATH,"%s\\%s",SID,ckey);
      Enum_ShellBags(key_path, SID, NULL,session_id, db);
    }
  }
  RegCloseKey(CleTmp);
}
//------------------------------------------------------------------------------
// source : http://support.microsoft.com/kb/813711
DWORD WINAPI Scan_registry_ShellBags(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  char file[MAX_PATH];
  HK_F_OPEN hks;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Shell_Bags\";\"source\";\"key\";\"value\";\"data\";\"sid\";\"last_update\";\"session_id\";\r\n");
  #endif
  //files or local
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
        if(OpenRegFiletoMem(&hks, file))
        {
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
          Scan_registry_ShellBags_file(&hks,"Software\\Microsoft\\Windows\\Shell\\BagMRU",file,session_id,db,TRUE);
          Scan_registry_ShellBags_file(&hks,"Software\\Microsoft\\Windows\\ShellNoRoam\\BagMRU",file,session_id,db,TRUE);
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
          Scan_registry_ShellBags_file(&hks,"Local Settings\\Software\\Microsoft\\Windows\\Shell\\BagMRU",file,session_id,db,TRUE);
          Scan_registry_ShellBags_file(&hks,"Local Settings\\Software\\Microsoft\\Windows\\ShellNoRoam\\BagMRU",file,session_id,db,TRUE);
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
          Scan_registry_ShellBags_file(&hks,"Wow6432Node\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\BagMRU",file,session_id,db,TRUE);
          Scan_registry_ShellBags_file(&hks,"Wow6432Node\\Local Settings\\Software\\Microsoft\\Windows\\ShellNoRoam\\BagMRU",file,session_id,db,TRUE);
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
    Scan_registry_ShellBags_direct("Software\\Microsoft\\Windows\\Shell\\BagMRU",session_id,db);
    Scan_registry_ShellBags_direct("Software\\Microsoft\\Windows\\ShellNoRoam\\BagMRU",session_id,db);
    if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

    if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
    Scan_registry_ShellBags_direct("Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\BagMRU",session_id,db);
    Scan_registry_ShellBags_direct("Software\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\ShellNoRoam\\BagMRU",session_id,db);
    if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

    if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
    Scan_registry_ShellBags_direct("Software\\Classes\\Wow6432Node\\Local Settings\\Software\\Microsoft\\Windows\\Shell\\BagMRU",session_id,db);
    Scan_registry_ShellBags_direct("Software\\Classes\\Wow6432Node\\Local Settings\\Software\\Microsoft\\Windows\\ShellNoRoam\\BagMRU",session_id,db);
    if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  }

  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan);
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
