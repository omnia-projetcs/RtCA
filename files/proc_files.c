//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//sha256deep format
void ConsoleDirectory_sha256deep(char *tmp_path)
{
  char path[MAX_PATH];
  if (tmp_path[0] == '"')
  {
    //extract separators : "
    snprintf(path,strlen(tmp_path)-2,"%s",tmp_path+1);
    path[strlen(path)] = 0;
  }else snprintf(path,MAX_PATH,"%s",tmp_path);

  printf("[%s]\n",path);

  char search[MAX_PATH]="",path_ex[MAX_PATH],s_sha[MAX_PATH];
  snprintf(search,MAX_PATH,"%s*.*",path);

  WIN32_FIND_DATA data;
  HANDLE hfic = FindFirstFile(search, &data);
  if (hfic == INVALID_HANDLE_VALUE)return;
  do
  {
    if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
    else
    {
      if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        snprintf(path_ex,MAX_PATH,"%s%s\\",path,data.cFileName);
        ConsoleDirectory_sha256deep(path_ex);
      }else
      {
        s_sha[0] = 0;
        snprintf(path_ex,MAX_PATH,"%s%s",path,data.cFileName);
        if ((data.nFileSizeLow + data.nFileSizeHigh) < MAX_FILE_SIZE_HASH)
        {
          FileToSHA256_noTM(path_ex, s_sha);
        }
        if (s_sha[0] != 0)printf("%s  %s\n",s_sha,path_ex);
      }
    }
  }while(FindNextFile (hfic,&data) !=0);
  FindClose(hfic);
}
//------------------------------------------------------------------------------
void AddItemFiletoTreeView(HANDLE htv, char *lowcase_file, char *path, char *global_path)
{
  //get extension
  char ext[MAX_PATH], tmp_path[MAX_PATH];
  if (extractExtFromFile(lowcase_file, ext, MAX_PATH))
  {
    if (strcmp(ext,"lnk")==0)
    {
      if (global_path != NULL)
        AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
      }
      check_treeview(htrv_test, H_tests[INDEX_FILE_NK], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_FILE], TRV_STATE_CHECK);
    }else if (strcmp(ext,"log")==0 ||
              strcmp(ext,"evt")==0 ||
              strcmp(ext,"evtx")==0) //logs
    {
      if (global_path != NULL)
        AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
      }
      check_treeview(htrv_test, H_tests[INDEX_LOG], TRV_STATE_CHECK);
    }else if (strcmp(lowcase_file,"security.dat")==0 ||
              strcmp(lowcase_file,"ntuser.dat")==0   || (Contient(lowcase_file,"ntuser")>0 && strcmp(ext,"dat")==0)   ||
              strcmp(lowcase_file,"settings.dat")==0 || (startWith(lowcase_file,"settings_") && strcmp(ext,"dat")==0)   || //win8
              strcmp(lowcase_file,"usrclass.dat")==0 || (Contient(lowcase_file,"usrclass")>0 && strcmp(ext,"dat")==0) ||
              strcmp(lowcase_file,"classes.dat")==0  || (Contient(lowcase_file,"classes")>0 && strcmp(ext,"dat")==0)) //registry
    {
      if (global_path != NULL)AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
      }

      check_treeview(htrv_test, H_tests[INDEX_LAN], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_ENV], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_SHARE], TRV_STATE_CHECK);

      unsigned int i;
      for (i = INDEX_REG_CONF;i<=INDEX_REG_FIREWALL;i++)
      {
        check_treeview(htrv_test, H_tests[i], TRV_STATE_CHECK);
      }

    }else if (strcmp(ext,"db")==0     ||          //android
              strcmp(ext,"sqlite")==0 ||          //firefox
              strcmp(ext,"dat")==0    ||          //ie
              strcmp(lowcase_file,"index.dat")==0 || (startWith(lowcase_file,"index_") && strcmp(ext,"dat")==0) ||
              strcmp(lowcase_file,"ntds.dit")==0  || (startWith(lowcase_file,"ntds_") && strcmp(ext,"dit")==0)) //applications
    {
      if (global_path != NULL)AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      }
      check_treeview(htrv_test, H_tests[INDEX_ANDROID], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_NAV_CHROME], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_NAV_FIREFOX], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_NAV_IE], TRV_STATE_CHECK);
    }else if (!strcmp(ext,"pf")) //prefetch
    {
      if (global_path != NULL)AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      }
      check_treeview(htrv_test, H_tests[INDEX_PREFETCH], TRV_STATE_CHECK);
    }else if (!strcmp(ext,"job")) //prefetch
    {
      if (global_path != NULL)AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      }
      check_treeview(htrv_test, H_tests[INDEX_TASK], TRV_STATE_CHECK);
    }
  }else
  {
    if (strcmp(lowcase_file,"sam")==0         ||
        strcmp(lowcase_file,"software")==0    ||
        strcmp(lowcase_file,"system")==0      ||
        strcmp(lowcase_file,"default")==0     ||
        strcmp(lowcase_file,"hardware")==0    ||
        strcmp(lowcase_file,"security")==0    ||
        strcmp(lowcase_file,"bcd-template")==0|| //win8
        strcmp(lowcase_file,"components")==0  || //win8
        strcmp(lowcase_file,"drivers")==0     || //win8
        strcmp(lowcase_file,"bbi")==0         || //win8
        strcmp(lowcase_file,"elam")==0        || //win8
        strcmp(lowcase_file,"fp")==0          || //win8

        startWith(lowcase_file,"sam_")         ||
        startWith(lowcase_file,"software_")    ||
        startWith(lowcase_file,"system_")      ||
        startWith(lowcase_file,"default_")     ||
        startWith(lowcase_file,"hardware_")    ||
        startWith(lowcase_file,"security_")    ||
        startWith(lowcase_file,"bcd-template_")|| //win8
        startWith(lowcase_file,"components_")  || //win8
        startWith(lowcase_file,"drivers_")     || //win8
        startWith(lowcase_file,"bbi_")         || //win8
        startWith(lowcase_file,"elam_")        || //win8
        startWith(lowcase_file,"fp_")) //win8
    {
      if (global_path != NULL)AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
      }

      check_treeview(htrv_test, H_tests[INDEX_LAN], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_ENV], TRV_STATE_CHECK);
      check_treeview(htrv_test, H_tests[INDEX_SHARE], TRV_STATE_CHECK);

      unsigned int i;
      for (i = INDEX_REG_CONF;i<=INDEX_REG_FIREWALL;i++)
      {
        check_treeview(htrv_test, H_tests[i], TRV_STATE_CHECK);
      }
    }else if (strcmp(lowcase_file,"archived history")==0 ||  //chrome
              strcmp(lowcase_file,"history")==0 ||
              strcmp(lowcase_file,"cookies")==0 ||
              strcmp(lowcase_file,"default")==0 ||
              strcmp(lowcase_file,"login data")==0 ||
              strcmp(lowcase_file,"top sites")==0 ||
              strcmp(lowcase_file,"web data")==0)
    {
      if (global_path != NULL)AddItemTreeView(htv,global_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      else
      {
        snprintf(tmp_path,MAX_PATH,"%s%s",path,lowcase_file);
        AddItemTreeView(htv,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
      }
      check_treeview(htrv_test, H_tests[INDEX_NAV_CHROME], TRV_STATE_CHECK);
    }
  }
}
//------------------------------------------------------------------------------
void scan_file(char *path, HANDLE htv)
{
  WIN32_FIND_DATA data;
  char tmp_path[MAX_PATH], file[MAX_PATH];

  snprintf(tmp_path,MAX_PATH,"%s*.*",path);

  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic == INVALID_HANDLE_VALUE)return;
  do
  {
    // return
    if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
    else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
    {
      snprintf(tmp_path,MAX_PATH,"%s%s\\",path,data.cFileName);
      SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)tmp_path);
      scan_file(tmp_path, htv);
    }else //file
    {
      strncpy(file,data.cFileName,MAX_PATH);
      AddItemFiletoTreeView(htv, charToLowChar(file), path, NULL);
    }
  }while(FindNextFile (hfic,&data) !=0);
  FindClose(hfic);
}
//------------------------------------------------------------------------------
DWORD  WINAPI AutoSearchFiles(LPVOID lParam)
{
  if (lParam == NULL)
  {
    //list all
    char tmp[MAX_PATH], tmp_l[4]="C:\\";
    int i,nblecteurs = GetLogicalDriveStrings(MAX_PATH,tmp);

    //search
    for (i=0;i<nblecteurs;i+=4)
    {
      switch(GetDriveType(&tmp[i]))
      {
        case DRIVE_REMOVABLE: if(disable_USB_SHARE)break;
        case DRIVE_REMOTE:    if(disable_USB_SHARE)break;
        case DRIVE_FIXED:
        case DRIVE_RAMDISK:
          tmp_l[0] = tmp[i];
          AddItemTreeView(htrv_files,tmp_l, TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
          scan_file(tmp_l, htrv_files);
        break;
      }
    }
  }else
  {
    char tmp_path[MAX_PATH];
    snprintf(tmp_path,MAX_PATH,"%s\\",(char*)lParam);

    AddItemTreeView(htrv_files,tmp_path, TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
    scan_file(tmp_path, htrv_files);
  }

  //tri and clean
  CleanTreeViewFiles(htrv_files);
  SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)"");
  B_AUTOSEARCH = FALSE;
  return 0;
}
