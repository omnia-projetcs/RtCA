//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//----------------------------------------------------------------------------
void ExtractSQLITE_DB()
{
  //save ressource in a local file
  HRSRC hr   = FindResource(NULL, MAKEINTRESOURCE(SQLITE_F), RT_RCDATA);
  HGLOBAL hb = LoadResource(NULL, hr);
  if (hb != NULL)
  {
    char local_path[MAX_PATH]="",file[MAX_PATH];
    GetLocalPath(local_path, MAX_PATH);
    snprintf(file,MAX_PATH,"%s\\%s",local_path,DEFAULT_SQLITE_FILE);

    //save the file in local
    HANDLE hf = CreateFile(file, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
    if (hf != INVALID_HANDLE_VALUE)
    {
      DWORD nb = 0;
      WriteFile(hf, LockResource(hb), SizeofResource(NULL, hr), &nb, NULL);
      CloseHandle(hf);
    }
  }
}
//------------------------------------------------------------------------------
//init strings + tests name + menu
//------------------------------------------------------------------------------
void InitGlobalLangueString(unsigned int langue_id)
{
  current_lang_id = langue_id;

  //get global GUI and message strings
  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = TYPE_SQLITE_FLAG_LANG_INIT_STRINGS;
  SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

  if (CONSOL_ONLY == FALSE)
  {
    //update menu
    FORMAT_CALBAK_READ_INFO fcri;
    fcri.type = TYPE_SQLITE_FLAG_LANGUAGE_CHANGE;
    SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

    if (current_item_selected > -1)
    {
      fcri.type = TYPE_SQLITE_FLAG_LANGUAGE_COL_CHANGE;
      SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);
    }

    //update list of tests
    SendMessage(hlstbox, LB_RESETCONTENT,0,0);
    fcri.type = TYPE_SQLITE_FLAG_TESTS_INIT;
    SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

    //update Windows
    ShowWindow(h_main, SW_HIDE);
    ShowWindow(h_main, SW_SHOW);
    UpdateWindow(h_main);

    //select item ! + force use of gui message
    if (current_item_selected > -1)
    {
      SendMessage(hlstbox, LB_SETCURSEL, current_item_selected, 0);
      TRI_RESULT_VIEW       = FALSE;
      TRI_PROCESS_VIEW      = FALSE;
      TRI_REG_VIEW          = FALSE;
      column_tri            = -1;
      pos_search            = 0;
      pos_search_reg        = 0;

      FORMAT_CALBAK_READ_INFO fcri;

      //get column count
      fcri.type = TYPE_SQLITE_FLAG_GET_COLUM_COUNT;
      SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

      //get column + text
      fcri.type = TYPE_SQLITE_FLAG_VIEW_CHANGE;
      SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

      //get items infos + items
      ListView_DeleteAllItems(hlstv);
      fcri.type = TYPE_SQLITE_FLAG_GET_ITEMS_INFO;
      SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

      char tmp_infos[DEFAULT_TMP_SIZE];
      snprintf(tmp_infos,DEFAULT_TMP_SIZE,"Item(s) : %d",ListView_GetItemCount(hlstv));
      SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)tmp_infos);
    }
  }
}
//------------------------------------------------------------------------------
//init string for Internet
//------------------------------------------------------------------------------
void InitSQLStrings()
{
  //db for ANDROID, FIREFOX,CHROME
  nb_sql_ANDROID  = 0;
  nb_sql_CHROME   = 0;
  nb_sql_FIREFOX  = 0;

  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = TYPE_SQLITE_FLAG_ANDROID_INIT_STRINGS;
  SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

  fcri.type = TYPE_SQLITE_FLAG_CHROME_INIT_STRINGS;
  SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

  fcri.type = TYPE_SQLITE_FLAG_FIREFOX_INIT_STRINGS;
  SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);
}
//------------------------------------------------------------------------------
//hide DOS form
//------------------------------------------------------------------------------
void hideDOSForm()
{
char pszOldWindowTitle[MAX_PATH];
GetConsoleTitle(pszOldWindowTitle, MAX_PATH);
ShowWindow(FindWindow(NULL, pszOldWindowTitle), SW_HIDE);
}
//------------------------------------------------------------------------------
BOOL HaveAdminRight()
{
  //get current token
  HANDLE  hThread;
  if(!OpenThreadToken(GetCurrentThread(), TOKEN_QUERY, FALSE, &hThread))
  {
    if(GetLastError() == ERROR_NO_TOKEN)
    {
       if(!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hThread))return TRUE;
    }else return TRUE;
  }

  typedef enum _MY_TOKEN_INFORMATION_CLASS {
    TokenUser=1,
    TokenGroups,
    TokenPrivileges,
    TokenOwner,
    TokenPrimaryGroup,
    TokenDefaultDacl,
    TokenSource,
    TokenType,
    TokenImpersonationLevel,
    TokenStatistics,
    TokenRestrictedSids,
    TokenSessionId,
    TokenGroupsAndPrivileges,
    TokenSessionReference,
    TokenSandBoxInert,
    TokenAuditPolicy,
    TokenOrigin,
    TokenElevationType,
    TokenLinkedToken,
    TokenElevation,
    TokenHasRestrictions,
    TokenAccessInformation,
    TokenVirtualizationAllowed,
    TokenVirtualizationEnabled,
    TokenIntegrityLevel,
    TokenUIAccess,
    TokenMandatoryPolicy,
    TokenLogonSid,
    MaxTokenInfoClass
  }TOKEN_MY_INFORMATION_CLASS;

  typedef enum  {
    TokenElevationTypeDefault  = 1,
    TokenElevationTypeFull,
    TokenElevationTypeLimited
  } TOKEN_ELEVATION_TYPE , *PTOKEN_ELEVATION_TYPE;

  BOOL ret = TRUE;
  DWORD  size = 0;
  if(!GetTokenInformation(hThread, TokenElevationType, NULL, 0, &size))
  {
    if (GetLastError()== ERROR_INSUFFICIENT_BUFFER)
    {
      TOKEN_ELEVATION_TYPE *ptet = (TOKEN_ELEVATION_TYPE *)malloc(size);
      if (ptet != NULL)
      {
        if (GetTokenInformation(hThread, TokenElevationType, ptet, size, &size))
        {
          if ((*ptet == TokenElevationTypeDefault) || (*ptet == TokenElevationTypeLimited))
          {
            ret = FALSE;
          }
        }
        free(ptet);
      }
    }
  }

  CloseHandle(hThread);
  return ret;
}
//------------------------------------------------------------------------------
//init start configuration
//------------------------------------------------------------------------------
void InitGlobalConfig(BOOL acl, BOOL ads, BOOL sha, BOOL local_scan, BOOL utc)
{
  //in wine or not ?
  WINE_OS = isWine();

  //default language
  if (CONSOL_ONLY)current_lang_id       = 1;
  else//get langue_id from RtCA.ini
  {
    //get current path
    char path[MAX_PATH]="";
    GetLocalPath(path, MAX_PATH);
    strcat(path,DEFAULT_INI_FILE);

    //get value
    char default_lang_id[DEFAULT_TMP_SIZE];
    GetPrivateProfileString("CONF","DEFAULT_LANG_ID","1",default_lang_id,DEFAULT_TMP_SIZE,path);
    current_lang_id = atoi(default_lang_id);

    if (IsDlgButtonChecked(h_proxy,PROXY_CHK_SAVE)==BST_CHECKED)
    {
      //if login and mdp
      char tmp[DEFAULT_TMP_SIZE]="";
      GetPrivateProfileString("PROXY","PROXY_URL","",tmp,DEFAULT_TMP_SIZE,path);
      if (tmp[0] != 0)SetWindowText(GetDlgItem((HWND)h_proxy,PROXY_ED_PROXY),tmp);

      tmp[0] = 0;
      GetPrivateProfileString("PROXY","PROXY_LOGIN","",tmp,DEFAULT_TMP_SIZE,path);
      if (tmp[0] != 0)SetWindowText(GetDlgItem((HWND)h_proxy,PROXY_ED_USER),tmp);

      tmp[0] = 0;
      GetPrivateProfileString("PROXY","PROXY_PASSWORD","",tmp,DEFAULT_TMP_SIZE,path);
      if (tmp[0] != 0)
      {
        //simple XOR function
        SetWindowText(GetDlgItem((HWND)h_proxy,PROXY_ED_PASSWORD),dechr(tmp,strlen(tmp),MDP_TEST));
      }
    }
    SendMessage(hCombo_lang, CB_SETCURSEL,current_lang_id-1,0);
  }

  //init globals var
  current_item_selected = -1;

  STAY_ON_TOP           = FALSE;
  enable_LNK            = FALSE;

  FILE_ACL              = acl;
  FILE_ADS              = ads;
  FILE_SHA              = sha;
  UTC_TIME              = utc;
  LOCAL_SCAN            = local_scan;

  start_scan            = FALSE;
  stop_scan             = FALSE;
  disable_m_context     = FALSE;
  disable_p_context     = FALSE;
  update_thread_start   = FALSE;
  export_type           = 0;
  _SYSKEY[0]            = 0;  //global syskey for decrypt hash of users

  //load data for db
  InitSQLStrings();
  InitGlobalLangueString(current_lang_id);
  current_session_id = session[0];

  if (!CONSOL_ONLY)LoadRootKitDB(hlstv_db);

  WSADATA WSAData;
  WSAStartup(0x02, &WSAData );

  SetDebugPrivilege(TRUE);

  //check if admin right ok
  if(!HaveAdminRight())
  {
    if (!CONSOL_ONLY)
    {
      if (current_session_id != 0)MessageBox(h_main,cps[TXT_MSG_RIGHT_ADMIN].c,cps[TXT_MSG_RIGHT_ADMIN_ATTENTION].c,MB_OK|MB_TOPMOST|MB_ICONWARNING);
    }else printf("[%s] %s\n",cps[TXT_MSG_RIGHT_ADMIN_ATTENTION].c,cps[TXT_MSG_RIGHT_ADMIN].c);
  }

  //init if 64b
  #ifndef _WIN64_VERSION_
  OldValue_W64b = FALSE;
  ReviewWOW64Redirect(OldValue_W64b);
  #endif
}
//------------------------------------------------------------------------------
//load tools items
//------------------------------------------------------------------------------
void LoadTools()
{
  nb_tools = 0;

  //open file
  char local_path[MAX_PATH]="",file[MAX_PATH];
  GetLocalPath(local_path, MAX_PATH);
  snprintf(file,MAX_PATH,"%s\\%s",local_path,DEFAULT_TOOL_MENU_FILE);

  HANDLE hFile = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (hFile != INVALID_HANDLE_VALUE)
  {
    DWORD file_size = GetFileSize(hFile,NULL);
    if (file_size>0)
    {
      //load file
      char *buffer = (char*)LocalAlloc(LMEM_FIXED, sizeof(char)*file_size+1);
      if (buffer != NULL)
      {
         DWORD copiee = 0;
         ReadFile(hFile, buffer, file_size,&copiee,0);
         if (copiee<file_size)file_size = copiee;
         if (file_size>0)
         {
           //work
           char line[MAX_LINE_SIZE] ="";
           char *t,tmp[DEFAULT_TMP_SIZE];
           char *l = line;
           char *b = buffer;

           while (*b)
           {
             //get line by line
             l = line;
             while (*b && (*b != '\r') && (*b!='\n') && (l-line < MAX_LINE_SIZE)) *l++ = *b++;
             *l = 0;

             if (line[0] != '#')
             {
               //work format : TYPE;TITLE;CMD;PARAMS
               //    exemple : 01;Open file;;

               //TYPE
               tmp[0] = line[0];
               tmp[1] = line[1];
               tmp[2] = 0;
               tools_load[nb_tools].type = atoi(tmp);

               //TITLE
               tools_load[nb_tools].title[0] = 0;
               t = tools_load[nb_tools].title;
               l = line+3; // pass "01;"

               while(*l && (*l!=';') && (t-tmp < DEFAULT_TMP_SIZE)) *t++ = *l++;
               *t = 0;
               l++;

               //CMD
               tools_load[nb_tools].cmd[0] = 0;
               t = tools_load[nb_tools].cmd;

               while(*l && (*l!=';') && (t-tmp < DEFAULT_TMP_SIZE)) *t++ = *l++;
               *t = 0;
               l++;

               //PARAMS
               tools_load[nb_tools].params[0] = 0;
               t = tools_load[nb_tools].params;

               while(*l && (*l!=';') && (t-tmp < DEFAULT_TMP_SIZE)) *t++ = *l++;
               *t = 0;
               l++;

               //next
               //printf("[%d] (%s) %s - %s\n",tools_load[nb_tools].type,tools_load[nb_tools].title,tools_load[nb_tools].cmd,tools_load[nb_tools].params);
               nb_tools++;
             }
             while (*b && ((*b=='\r') || (*b == '\n')))b++; //pass \r\n
           }
         }
        LocalFree(buffer);
      }
    }
  }
  CloseHandle(hFile);
}
//------------------------------------------------------------------------------
//init GUI configuration
//------------------------------------------------------------------------------
DWORD WINAPI InitGUIConfig(LPVOID lParam)
{
  if (GetFileAttributes(SQLITE_LOCAL_BDD) == INVALID_FILE_ATTRIBUTES)ExtractSQLITE_DB();

  BOOL reinit = (BOOL)lParam;
  if (reinit)
  {
    //clean
    sqlite3_close(db_scan);
  }

  //hidden DOS form
  #ifndef DEV_DEBUG_MODE
    hideDOSForm();
  #endif

  //global init
  B_AUTOSEARCH          = FALSE;
  h_AUTOSEARCH          = NULL;
  search_rootkit        = FALSE;
  search_rootkit_process_tool= FALSE;
  BACKUP_FILE_LIST_started = FALSE;
  BACKUP_PATH_started   = FALSE;
  ExportStart           = FALSE;
  TRI_RESULT_VIEW       = FALSE;
  TRI_PROCESS_VIEW      = FALSE;
  TRI_REG_VIEW          = FALSE;
  column_tri            = -1;
  NB_TESTS              = 0;
  pos_search            = 0;
  pos_search_reg        = 0;
  current_OS[0]         = 0;
  current_OS_BE_64b     = FALSE;
  nb_current_columns    = 0;
  current_lang_id       = 1;
  follow_sniff          = FALSE;
  reg_file_start_process = FALSE;
  AVIRUSTTAL            = FALSE;
  VIRUSTTAL             = FALSE;
  SQLITE_FULL_SPEED     = TRUE;
  B_SCREENSHOT          = FALSE;
  B_SCREENSHOT_START    = FALSE;
  enable_magic          = FALSE;
  enable_remote         = FALSE;
  DISABLE_GRID_LV_ALL   = FALSE;
  h_Hexa                = 0;
  last_search_mode      = DEFAULT_MODE_SEARCH_HX;
  last_pos_hex_search   = 0;
  use_proxy_advanced_settings = 0;
  use_other_proxy       = 0;

  if (!reinit)
  {
    Trame_buffer = malloc(100*sizeof(TRAME_BUFFER));
    hMutex_TRAME_BUFFER = CreateMutex(0,FALSE,0);
  }

  //open sqlite file
  if (sqlite3_open(SQLITE_LOCAL_BDD, &db_scan) != SQLITE_OK)
  {
    char local_path[MAX_PATH]="",file[MAX_PATH];
    GetLocalPath(local_path, MAX_PATH);
    snprintf(file,MAX_PATH,"%s\\%s",local_path,DEFAULT_TM_SQLITE_FILE);

    //if tmp sqlite file exist free !!
    //for bug case
    if (GetFileAttributes(file) != INVALID_FILE_ATTRIBUTES)
    {
      DeleteFile(file);
    }

    sqlite3_open(SQLITE_LOCAL_BDD, &db_scan);
  }

  if (!reinit)
  {
    //Init language cb
    HANDLE H_ImagList = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 , /*nb icones*/2, 0);
    ImageList_AddIcon(H_ImagList,(HICON)LoadIcon((HINSTANCE) hinst, MAKEINTRESOURCE(ICON_LANGUAGE_EN)));
    ImageList_AddIcon(H_ImagList,(HICON)LoadIcon((HINSTANCE) hinst, MAKEINTRESOURCE(ICON_LANGUAGE_FR)));
    ImageList_SetBkColor(H_ImagList, GetSysColor(COLOR_WINDOW));
    SendMessage(hCombo_lang,CBEM_SETIMAGELIST,0,(LPARAM)H_ImagList);
  }

  //init la combo box des langues
  SendMessage(hCombo_lang, CB_RESETCONTENT,0,0);
  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = TYPE_SQLITE_FLAG_LANG_INIT;
  SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);
  SendMessage(hCombo_lang, CB_SETCURSEL,0,0);//default select English

  //list of sessions !!!
  SendMessage(hCombo_session, CB_RESETCONTENT,0,0);
  nb_session = 0;
  fcri.type  = TYPE_SQLITE_FLAG_SESSIONS_INIT;
  SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);
  SendMessage(hCombo_session, CB_SETCURSEL,0,0);

  //icons for tests
  if (!reinit)
  {
    H_ImagList_icon = ImageList_Create(GetSystemMetrics(SM_CXSMICON),GetSystemMetrics(SM_CYSMICON), ILC_COLOR32 , /*nb icon*/2, 0);
    ImageList_AddIcon(H_ImagList_icon,(HICON)LoadIcon((HINSTANCE) GetModuleHandle(0), MAKEINTRESOURCE(ICON_FOLDER)));
    ImageList_AddIcon(H_ImagList_icon,(HICON)LoadIcon((HINSTANCE) GetModuleHandle(0), MAKEINTRESOURCE(ICON_FILE)));
    ImageList_SetBkColor(H_ImagList_icon, GetSysColor(COLOR_WINDOW));
  }

  //all others datas
  InitGlobalConfig(FALSE, FALSE, FALSE, TRUE, FALSE);

  //init help messages
  AddtoToolTip(htoolbar, htooltip, hinst, 2, NULL, cps[TXT_TOOLTIP_NEW_SESSION].c);
  AddtoToolTip(htoolbar, htooltip, hinst, 5, NULL, cps[TXT_TOOLTIP_SEARCH].c);

  //init load of externes tools or actions :
  LoadTools();

  if (WINE_OS)
  {
    EnableMenuItem(GetMenu(h_main),IDM_TOOLS_PROCESS,MF_BYCOMMAND|MF_GRAYED);
  }

  CheckMenuItem(GetMenu(h_main),BT_SQLITE_FULL_SPEED,MF_BYCOMMAND|MF_CHECKED);
  sqlite3_exec(db_scan,"PRAGMA journal_mode = OFF;", NULL, NULL, NULL);

  //init dd menu to copy only driver in disk format
  char letter;
  char tmp[] = "C:\\";
  for (letter='Z';letter>='A';letter--)
  {
    tmp[0] = letter;
    switch(GetDriveType(tmp))
    {
      case DRIVE_FIXED:
      case DRIVE_REMOVABLE:break;
      default:
        RemoveMenu(GetSubMenu(GetSubMenu(GetMenu(h_main),2),5),letter-'A',MF_BYPOSITION); //-E au lieu de -A car 5 menus en +
      break;
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
// add help message to toolips
//------------------------------------------------------------------------------
void AddtoToolTip(HWND hcompo, HWND hTTip, HINSTANCE hinst, unsigned int id, char *title, char *txt)
{
  RECT rect;
  SendMessage(hcompo, TB_GETITEMRECT, id, (LPARAM) &rect);

  TOOLINFO ti;
  ti.cbSize       = sizeof(TOOLINFO);
  ti.uFlags       = TTF_SUBCLASS;
  ti.hwnd         = hcompo;
  ti.hinst        = hinst;
  ti.uId          = id;
  ti.lpszText     = txt;
  ti.rect.left    = rect.left;
  ti.rect.top     = rect.top;
  ti.rect.right   = rect.right;
  ti.rect.bottom  = rect.bottom;

  SendMessage(hTTip, TTM_ADDTOOL   , 0               , (LPARAM) &ti);
  if (title != NULL) SendMessage(hTTip, TTM_SETTITLE  , (WPARAM)TTI_INFO, (LPARAM)title);
  SendMessage(hTTip, TB_SETTOOLTIPS, (WPARAM)hcompo  , 0);
}
//------------------------------------------------------------------------------
// add help message to toolips
//------------------------------------------------------------------------------
void ModifyToolTip(HWND hcompo, HWND hTTip, HINSTANCE hinst, unsigned int id, char *title, char *txt)
{
  RECT rect;
  SendMessage(hcompo, TB_GETITEMRECT, id, (LPARAM) &rect);

  TOOLINFO ti;
  ti.cbSize       = sizeof(TOOLINFO);
  ti.uFlags       = TTF_SUBCLASS;
  ti.hwnd         = hcompo;
  ti.hinst        = hinst;
  ti.uId          = id;
  ti.lpszText     = txt;
  ti.rect.left    = rect.left;
  ti.rect.top     = rect.top;
  ti.rect.right   = rect.right;
  ti.rect.bottom  = rect.bottom;

  SendMessage(hTTip, TTM_SETTOOLINFO, (WPARAM)hcompo, (LPARAM) &ti);//TTM_UPDATETIPTEXT
  if (title != NULL) SendMessage(hTTip, TTM_SETTITLE , (WPARAM)TTI_INFO, (LPARAM)title);
}

//------------------------------------------------------------------------------
//End of GUI
//------------------------------------------------------------------------------
void EndGUIConfig(HANDLE hwnd)
{
  //quit tray icone
  if (B_SCREENSHOT)
  {
    DestroyIcon(TrayIcon.hIcon);
    Shell_NotifyIcon(NIM_DELETE,&TrayIcon);
    UnhookWindowsHookEx(HHook);
  }

  sqlite3_close(db_scan);
  #ifndef _WIN64_VERSION_
  ReviewWOW64Redirect(OldValue_W64b);
  #endif

  //clean
  DeleteObject(Hb_green);
  DeleteObject(Hb_blue);
  DeleteObject(Hb_pink);
  DeleteObject(Hb_violet);

  free(Trame_buffer);
  CloseHandle(hMutex_TRAME_BUFFER);

  //save current language if not 1
  //get current path
  char path[MAX_PATH]="";
  GetLocalPath(path, MAX_PATH);
  strncat(path,DEFAULT_INI_FILE,MAX_PATH-strlen(path));

  //set value
  char default_lang_id[DEFAULT_TMP_SIZE];
  WritePrivateProfileString("CONF","DEFAULT_LANG_ID",itoa(current_lang_id,default_lang_id,10),path);

  //login+mdp and proxy
  char tmp[DEFAULT_TMP_SIZE]="";
  if (GetWindowTextLength(GetDlgItem(h_proxy,PROXY_ED_PROXY)) > 0)
  {
    GetWindowText(GetDlgItem(h_proxy,PROXY_ED_PROXY),tmp,DEFAULT_TMP_SIZE);
    WritePrivateProfileString("PROXY","PROXY_URL",tmp,path);
  }

  tmp[0] = 0;
  if (GetWindowTextLength(GetDlgItem(h_proxy,PROXY_ED_USER)) > 0)
  {
    GetWindowText(GetDlgItem(h_proxy,PROXY_ED_USER),tmp,DEFAULT_TMP_SIZE);
    WritePrivateProfileString("PROXY","PROXY_LOGIN",tmp,path);
  }

  tmp[0] = 0;
  if (GetWindowTextLength(GetDlgItem(h_proxy,PROXY_ED_PASSWORD)) > 0)
  {
    GetWindowText(GetDlgItem(h_proxy,PROXY_ED_PASSWORD),tmp,DEFAULT_TMP_SIZE);
    WritePrivateProfileString("PROXY","PROXY_PASSWORD",chr(tmp,MDP_TEST),path);
  }

  CloseWindow(hwnd);

  //clean richedit
  FreeLibrary(richDll);

  WSACleanup();
  PostQuitMessage(0);
}
