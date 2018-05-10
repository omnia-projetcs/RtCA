//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void loadFile_test(char *file, unsigned int index)
{
  //load file
  HANDLE hfile = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (hfile != INVALID_HANDLE_VALUE)
  {
    DWORD size      = GetFileSize(hfile,NULL);
    char *buffer    = (char *) malloc(size+1);
    if (buffer != NULL)
    {
      DWORD copiee =0;
      ReadFile(hfile, buffer, size,&copiee,0);

      char line[MAX_LINE_SIZE+1], *l;
      char *s = buffer;
      while (*s)
      {
        line[0] = 0;
        l = line;

        while(*s && (*s != '\r') && (*s != '\n') && (l-line < MAX_LINE_SIZE))*l++ = *s++;
        while(*s && ((*s == '\n') || (*s == '\r')))s++;
        *l = 0;

        if (line[0] == '"')index = 2;
        switch(index)
        {
          //txt file one line by line
          case 0:
          case 1:FileToTreeView(line);break;

          //csv format
          case 2:
            //rescue only column 2
            l = line;
            if (*l)
            {
              l++;
              while(*l && ((*l != '"') && (*(l+1) != ';')))l++;

              if (*l == '"')
              {
                l+=3;
                if (*l)
                {
                  // ";
                  if (line[strlen(line)-2] == '"')
                  {
                    line[strlen(line)-2] = 0;
                    FileToTreeView(l);
                  }
                }
              }
            }
          break;
        }
      }
      free(buffer);
    }
  }
  CloseHandle(hfile);
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_conf(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_COMMAND:
      if (HIWORD(wParam) == BN_CLICKED)
      {
        switch(LOWORD(wParam))
        {
          case BT_START:
            if (start_scan) CreateThread(NULL,0,StopGUIScan,0,0,0);
            else
            {
              //init
              start_scan = TRUE;
              //local or not ?
              if(TreeView_GetCount(htrv_files) > NB_MX_TYPE_FILES_TITLE)LOCAL_SCAN = FALSE;
              else LOCAL_SCAN = TRUE;
              //read state !
              if (IsDlgButtonChecked(h_conf,BT_ACL_FILE_CHK)==BST_CHECKED)FILE_ACL=TRUE;
              else FILE_ACL=FALSE;
              if (IsDlgButtonChecked(h_conf,BT_ADS_FILE_CHK)==BST_CHECKED)FILE_ADS=TRUE;
              else FILE_ADS=FALSE;
              if (IsDlgButtonChecked(h_conf,BT_SHA_FILE_CHK)==BST_CHECKED)FILE_SHA=TRUE;
              else FILE_SHA=FALSE;
              if (IsDlgButtonChecked(h_conf,BT_UTC_CHK)==BST_CHECKED)UTC_TIME=TRUE;
              else UTC_TIME=FALSE;
              if (IsDlgButtonChecked(h_conf,BT_MAGIC_CHK)==BST_CHECKED)enable_magic=TRUE;
              else enable_magic=FALSE;
              if (IsDlgButtonChecked(h_conf,BT_RA_CHK)==BST_CHECKED)enable_remote=TRUE;
              else enable_remote=FALSE;
              if (Ischeck_treeview(htrv_test, H_tests[INDEX_FILE_NK]))enable_LNK= TRUE;
              else enable_LNK= FALSE;

              EnableWindow(htrv_files,FALSE);
              EnableWindow(GetDlgItem((HWND)h_conf,BT_ACL_FILE_CHK),FALSE);
              EnableWindow(GetDlgItem((HWND)h_conf,BT_ADS_FILE_CHK),FALSE);
              EnableWindow(GetDlgItem((HWND)h_conf,BT_SHA_FILE_CHK),FALSE);
              EnableWindow(GetDlgItem((HWND)h_conf,BT_UTC_CHK),FALSE);
              EnableWindow(GetDlgItem((HWND)h_conf,BT_MAGIC_CHK),FALSE);
              EnableWindow(GetDlgItem((HWND)h_conf,BT_RA_CHK),FALSE);

              if(TreeView_GetCount(htrv_files) > NB_MX_TYPE_FILES_TITLE)LOCAL_SCAN = FALSE;
              else LOCAL_SCAN = TRUE;

              //get session name if used
              session_name_ch[0] = 0;
              GetWindowText(GetDlgItem((HWND)h_conf,EDT_NAME_SESSION),session_name_ch,MAX_PATH);

              //create new session and select it !
              SendMessage(hCombo_session, CB_RESETCONTENT,0,0);
              FORMAT_CALBAK_READ_INFO fcri;
              fcri.type = TYPE_SQL_ADD_SESSION;
              SQLITE_WriteData(&fcri, SQLITE_LOCAL_BDD);

              //start
              h_thread_scan = CreateThread(NULL,0,GUIScan,0,0,0);
              SetWindowText(GetDlgItem((HWND)h_conf,BT_START),cps[TXT_BT_STOP].c);
            }
          break;
          //----------------------------------------
          case POPUP_TRV_FILES_ADD_FILE:
          {
            char files[MAX_LINE_DBSIZE]="";
            memset(files,0,MAX_LINE_DBSIZE);
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = h_conf;
            ofn.lpstrFile = files;
            ofn.nMaxFile = MAX_LINE_DBSIZE;
            ofn.lpstrFilter = "*.* \0*.*\0"
                              "*.log\0*.log\0"
                              "*.evt\0*.evt\0"
                              "*.evtx\0*.evtx\0"
                              "*.db\0*.db\0"
                              "*.sqlite\0*.sqlite\0"
                              "*.dat\0*.dat\0"
                              "*.pf\0*.pf\0"
                              "*.job\0*.job\0"
                              "ntds.dit\0ntds.dit\0"
                              "sam\0sam\0"
                              "system\0system\0"
                              "software\0software\0"
                              "security\0security\0"
                              "default\0default\0"
                              "hardware\0hardware\0";
            ofn.nFilterIndex = 1;
            ofn.Flags =/*OFN_FILEMUSTEXIST |*/ OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT|OFN_EXPLORER|OFN_SHOWHELP;
            ofn.lpstrDefExt ="*.*";
            if (GetOpenFileName(&ofn)==TRUE)
            {
              //firt is path
              char path[MAX_PATH],totalpath[MAX_PATH];
              snprintf(path,MAX_PATH,"%s",files);

              //after file name
              char *p = files+strlen(files)+1;
              if (*p == 0)FileToTreeView(path);
              while (*p)
              {
                snprintf(totalpath,MAX_PATH,"%s\\%s",path,p);
                FileToTreeView(totalpath);
                p = p+strlen(p)+1;
              }

              //tri and clean
              CleanTreeViewFiles(htrv_files);

              //expend des branches
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
            }
          }
          break;
          case POPUP_TRV_FILES_ADD_PATH:
          {
            BROWSEINFO browser;
            LPITEMIDLIST lip;
            char path[MAX_PATH]     = "";
            browser.hwndOwner       = h_conf;
            browser.pidlRoot        = 0;
            browser.lpfn            = 0;
            browser.iImage          = 0;
            browser.lParam          = 0;
            browser.ulFlags         = BIF_NEWDIALOGSTYLE;
            browser.pszDisplayName  = path;
            browser.lpszTitle       = "";
            lip = SHBrowseForFolder(&browser);
            if (lip != NULL)
            {
              if (SHGetPathFromIDList(lip,path))
              {
                FileToTreeView(path);

                //tri and clean
                CleanTreeViewFiles(htrv_files);

                //expend all
                SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
                SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
                SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
                SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
              }
            }
          }
          break;
          case POPUP_TRV_FILES_UP:
            {
              //get selection + parent
              HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files,TVM_GETNEXTITEM,(WPARAM)TVGN_CARET, (LPARAM)0);
              HTREEITEM hparent = (HTREEITEM)SendMessage(htrv_files,TVM_GETNEXTITEM,(WPARAM)TVGN_PARENT, (LPARAM)hitem);
              //get txt
              char tmp[MAX_PATH];
              GetItemTreeView(hitem,htrv_files,tmp, MAX_PATH);
              //add item
              if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_LOGS])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
              else if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_FILES])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
              else if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
              else if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_APPLI])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);

              //remove item
              SendMessage(htrv_files,TVM_DELETEITEM,(WPARAM)0, (LPARAM)hitem);
              //expend all
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
            }
          break;
          case POPUP_TRV_FILES_DOWN:
            {
              //get selection + parent
              HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files,TVM_GETNEXTITEM,(WPARAM)TVGN_CARET, (LPARAM)0);
              HTREEITEM hparent = (HTREEITEM)SendMessage(htrv_files,TVM_GETNEXTITEM,(WPARAM)TVGN_PARENT, (LPARAM)hitem);
              //get txt
              char tmp[MAX_PATH];
              GetItemTreeView(hitem,htrv_files,tmp, MAX_PATH);
              //add item
              if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_LOGS])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
              else if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_FILES])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
              else if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
              else if (hparent == TRV_HTREEITEM_CONF[FILES_TITLE_APPLI])AddItemTreeView(htrv_files,tmp, TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);

              //remove item
              SendMessage(htrv_files,TVM_DELETEITEM,(WPARAM)0, (LPARAM)hitem);
              //expend all
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
              SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);

            }
          break;
          case POPUP_TRV_FILES_REMOVE_ITEMS:TreeView_DeleteItem(htrv_files, (HTREEITEM)SendMessage(htrv_files,TVM_GETNEXTITEM,(WPARAM)TVGN_CARET, (LPARAM)0));break;
          case POPUP_TRV_FILES_CLEAN_ALL:
            SendMessage(htrv_files,TVM_DELETEITEM,(WPARAM)0, (LPARAM)TVI_ROOT);
            check_childs_treeview(htrv_test, FALSE);

            TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]      = AddItemTreeView(htrv_files,cps[TXT_FILE_AUDIT].c, TVI_ROOT);
            TRV_HTREEITEM_CONF[FILES_TITLE_FILES]     = AddItemTreeView(htrv_files,cps[TXT_FILE_REP].c, TVI_ROOT);
            TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]  = AddItemTreeView(htrv_files,cps[TXT_FILE_REGISTRY].c, TVI_ROOT);
            TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]     = AddItemTreeView(htrv_files,cps[TXT_FILE_APPLI].c, TVI_ROOT);
          break;
          case POPUP_TRV_FILES_OPEN_PATH:
          {
            //get item txt
            char path[MAX_PATH];
            TVITEM tvitem;
            tvitem.mask = TVIF_HANDLE|TVIF_TEXT;
            tvitem.hItem = (HTREEITEM)SendMessage(htrv_files,TVM_GETNEXTITEM,(WPARAM)TVGN_CARET, (LPARAM)0);
            tvitem.cchTextMax = MAX_PATH;
            tvitem.pszText = path;
            if (SendMessage(htrv_files,TVM_GETITEM,(WPARAM)0, (LPARAM)&tvitem))
            {
              //directory or file
              if(isDirectory(path))ShellExecute(h_main, "explore", path, NULL,NULL,SW_SHOW);
              else
              {
                //extract
                char *c = path;
                while (*c++);
                while (*c != '\\')c--;
                c++;
                *c=0;
                ShellExecute(h_main, "explore", path, NULL,NULL,SW_SHOW);
              }
            }
          }
          break;
          case POPUP_TRV_FILES_AUTO_SEARCH:
            if (B_AUTOSEARCH)
            {
              B_AUTOSEARCH = FALSE;
              DWORD IDThread;
              GetExitCodeThread(h_AUTOSEARCH,&IDThread);
              TerminateThread(h_AUTOSEARCH,IDThread);

              //clean results
              unsigned int i;
              for (i=0;i<NB_MX_TYPE_FILES_TITLE;i++)
              {
                //tri
                SendMessage(htrv_files,TVM_SORTCHILDREN, TRUE,(LPARAM)TRV_HTREEITEM_CONF[i]);
                SupDoublon(htrv_files,TRV_HTREEITEM_CONF[i]);
                SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[i]);
              }
            }else
            {
              B_AUTOSEARCH = TRUE;
              h_AUTOSEARCH = CreateThread(NULL,0,AutoSearchFiles,0,0,0);
            }
          break;
          case POPUP_TRV_FILES_AUTO_SEARCH_PATH:
            if (B_AUTOSEARCH)
            {
              B_AUTOSEARCH = FALSE;
              DWORD IDThread;
              GetExitCodeThread(h_AUTOSEARCH,&IDThread);
              TerminateThread(h_AUTOSEARCH,IDThread);

              //clean results
              unsigned int i;
              for (i=0;i<NB_MX_TYPE_FILES_TITLE;i++)
              {
                //tri
                SendMessage(htrv_files,TVM_SORTCHILDREN, TRUE,(LPARAM)TRV_HTREEITEM_CONF[i]);
                SupDoublon(htrv_files,TRV_HTREEITEM_CONF[i]);
                SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[i]);
              }
            }else
            {
              BROWSEINFO browser;
              LPITEMIDLIST lip;
              char path[MAX_PATH]     = "";
              browser.hwndOwner       = h_conf;
              browser.pidlRoot        = 0;
              browser.lpfn            = 0;
              browser.iImage          = 0;
              browser.lParam          = 0;
              browser.ulFlags         = BIF_NEWDIALOGSTYLE;
              browser.pszDisplayName  = path;
              browser.lpszTitle       = "";
              lip = SHBrowseForFolder(&browser);
              if (lip != NULL)
              {
                if (SHGetPathFromIDList(lip,path))
                {
                  //strncat(path,"\\\0",MAX_PATH);
                  B_AUTOSEARCH = TRUE;
                  h_AUTOSEARCH = CreateThread(NULL,0,AutoSearchFiles,path,0,0);
                }
              }
            }
          break;
          case POPUP_TRV_FILES_SAVE_LIST:
          {
            char file[MAX_PATH]="";
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = h_conf;
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter ="*.txt \0*.txt\0*.csv\0*.csv\0";
            ofn.nFilterIndex = 1;
            ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
            ofn.lpstrDefExt ="txt\0";
            if (GetSaveFileName(&ofn)==TRUE)
            {
              if (ofn.nFilterIndex == 2) SaveTRV(htrv_files, file, SAVE_TYPE_CSV);
              else SaveTRV(htrv_files, file, SAVE_TYPE_TXT);
            }
          }
          break;
          case POPUP_TRV_FILES_LOAD_LIST:
          {
            char file[MAX_LINE_SIZE]="";
            memset(file,0,MAX_LINE_SIZE);
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = h_conf;
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_LINE_SIZE;
            ofn.lpstrFilter = "*.* \0*.*\0"
                              "*.txt\0*.txt\0"
                              "*.csv\0*.csv\0";
            ofn.nFilterIndex = 1;
            ofn.Flags =/*OFN_FILEMUSTEXIST |*/ OFN_OVERWRITEPROMPT | OFN_ALLOWMULTISELECT|OFN_EXPLORER|OFN_SHOWHELP;
            ofn.lpstrDefExt ="*.*";
            if (GetOpenFileName(&ofn)==TRUE)
            {
              loadFile_test(file, ofn.nFilterIndex);
            }
          }
          break;

          case POPUP_TRV_CHECK_ALL:check_childs_treeview(htrv_test, TRUE);break;
          case POPUP_TRV_UNCHECK_ALL:check_childs_treeview(htrv_test, FALSE);break;
          case POPUP_TRV_STOP_TEST:
          {
            //get item index
            int index = GetTrvItemIndex((HTREEITEM)SendMessage(htrv_test,TVM_GETNEXTITEM,(WPARAM)TVGN_CARET, (LPARAM)0), htrv_test);
            if (index < NB_TESTS && index > -1)
            {
              //kill the thread
              DWORD IDThread;
              GetExitCodeThread(h_thread_test[index],&IDThread);
              TerminateThread(h_thread_test[index],IDThread);
              h_thread_test[index] = 0;
              check_treeview(htrv_test, H_tests[index], TRV_STATE_UNCHECK);
            }
          }
          break;
          case POPUP_TRV_FILES_BACKUP:
          if (BACKUP_FILE_LIST_started) BACKUP_FILE_LIST_started = FALSE;
          else
          {
            BACKUP_FILE_LIST_started = TRUE;
            char file[MAX_PATH]="";
            GenerateNameToSave(file, MAX_PATH, "filelist.zip");

            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(OPENFILENAME));
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.hwndOwner = h_conf;
            ofn.lpstrFile = file;
            ofn.nMaxFile = MAX_PATH;
            ofn.lpstrFilter ="*.zip\0";
            ofn.nFilterIndex = 1;
            ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
            ofn.lpstrDefExt ="zip\0";
            if (GetSaveFileName(&ofn)==TRUE)
            {
              SaveAllTRVFilesToZip(file);
            }
          }
          break;
          case POPUP_TRV_FILES_BACKUP_PATH:
          if (BACKUP_PATH_started) BACKUP_PATH_started = FALSE;
          else
          {
            BROWSEINFO browser;
            LPITEMIDLIST lip;
            char path[MAX_PATH]     = "";
            browser.hwndOwner       = h_conf;
            browser.pidlRoot        = 0;
            browser.lpfn            = 0;
            browser.iImage          = 0;
            browser.lParam          = 0;
            browser.ulFlags         = BIF_NEWDIALOGSTYLE;
            browser.pszDisplayName  = path;
            browser.lpszTitle       = "Path from";

            lip = SHBrowseForFolder(&browser);
            if (lip != NULL)
            {
              if (SHGetPathFromIDList(lip,path))
              {
                char file[MAX_PATH]="";
                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = h_conf;
                ofn.lpstrFile = file;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter ="*.zip\0";
                ofn.nFilterIndex = 1;
                ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                ofn.lpstrDefExt ="zip\0";
                if (GetSaveFileName(&ofn)==TRUE)
                {
                  SaveALLCustom(file, "Custom_Path", path);
                }
              }
            }
          }
          break;
        }
      }
    break;
    case WM_NOTIFY:
      /*if (((LPNMHDR)lParam)->code == LVN_COLUMNCLICK)
      {
        TRI_PROCESS_VIEW = !TRI_PROCESS_VIEW;
        c_Tri(((LPNMHDR)lParam)->hwndFrom,((LPNMLISTVIEW)lParam)->iSubItem,TRI_PROCESS_VIEW);
      }else */if (((LPNMHDR)lParam)->code == NM_CLICK && ((LPNMHDR)lParam)->hwndFrom == htrv_test)
      {
        //selected item and state
        TV_HITTESTINFO Struct;
        POINT pt;
        GetCursorPos(&pt);
        ScreenToClient(htrv_test, &pt);
        Struct.pt = pt;
        HTREEITEM hItemSelect = TreeView_HitTest(htrv_test, &Struct);
        if (hItemSelect != NULL)
        {
          if (Struct.flags == TVHT_ONITEMSTATEICON)TreeView_SelectItem(htrv_test,hItemSelect);
          else//check case
          {
            if (Ischeck_treeview(htrv_test, hItemSelect))check_treeview(htrv_test, hItemSelect, TRV_STATE_CHECK);
            else check_treeview(htrv_test, hItemSelect, TRV_STATE_UNCHECK);
          }
        }
      }
    break;
    case WM_CONTEXTMENU://popup menu
      if ((HWND)wParam == htrv_test)
      {
        //select trv item
        TV_HITTESTINFO tvh_info;
        tvh_info.pt.x = LOWORD(lParam);
        tvh_info.pt.y = HIWORD(lParam);
        ScreenToClient(htrv_test, &(tvh_info.pt));
        HTREEITEM hItemSelect = TreeView_HitTest(htrv_test, &tvh_info);
        if (hItemSelect != 0)TreeView_SelectItem(htrv_test,hItemSelect);

        //popup
        HMENU hmenu;
        if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_TRV_TEST)))!= NULL)
        {
          //set text !!!
          ModifyMenu(hmenu,POPUP_TRV_CHECK_ALL          ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_CHECK_ALL          ,cps[TXT_POPUP_CHECK_ALL].c);
          ModifyMenu(hmenu,POPUP_TRV_UNCHECK_ALL        ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_UNCHECK_ALL        ,cps[TXT_POPUP_UNCHECK_ALL].c);

          HTREEITEM item = (HTREEITEM)SendMessage(htrv_test,TVM_GETNEXTITEM,(WPARAM)TVGN_CARET, (LPARAM)0);
          BOOL check = Ischeck_treeview(htrv_test, item);

          if (start_scan && (item!=NULL) && check && (h_thread_test[GetTrvItemIndex(item, htrv_test)]!=NULL))
          {
            ModifyMenu(hmenu,POPUP_TRV_STOP_TEST        ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_STOP_TEST        ,cps[TXT_POPUP_STOP_TEST].c);
          }else
          {
            RemoveMenu(hmenu,POPUP_TRV_STOP_TEST,MF_BYCOMMAND);
            RemoveMenu(GetSubMenu(hmenu, 0),2,MF_BYPOSITION);
          }

          //affichage du popup menu
          POINT pos;
          if (GetCursorPos(&pos)!=0)
          {
            TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, pos.x, pos.y,hwnd, NULL);
          }else TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),hwnd, NULL);

          DestroyMenu(hmenu);
        }
      }else if ((HWND)wParam == htrv_files)
      {
        //select trv item
        TV_HITTESTINFO tvh_info;
        tvh_info.pt.x = LOWORD(lParam);
        tvh_info.pt.y = HIWORD(lParam);
        ScreenToClient(htrv_files, &(tvh_info.pt));
        HTREEITEM hItemSelect = TreeView_HitTest(htrv_files, &tvh_info);
        if (hItemSelect != NULL)TreeView_SelectItem(htrv_files,hItemSelect);

        //popup
        HMENU hmenu;
        if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_TRV_FILES)))!= NULL)
        {
          //set text !!!
          ModifyMenu(hmenu,POPUP_TRV_FILES_ADD_FILE     ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_ADD_FILE     ,cps[TXT_POPUP_ADD_FILE].c);
          ModifyMenu(hmenu,POPUP_TRV_FILES_ADD_PATH     ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_ADD_PATH     ,cps[TXT_POPUP_ADD_PATH].c);

          if (BACKUP_PATH_started) ModifyMenu(hmenu,POPUP_TRV_FILES_BACKUP_PATH  ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_BACKUP_PATH  ,cps[TXT_POPUP_BACKUP_PATH_STOP].c);
          else ModifyMenu(hmenu,POPUP_TRV_FILES_BACKUP_PATH  ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_BACKUP_PATH  ,cps[TXT_POPUP_BACKUP_PATH].c);

          if (B_AUTOSEARCH)
          {
            RemoveMenu(hmenu,POPUP_TRV_FILES_AUTO_SEARCH_PATH ,MF_BYCOMMAND);
            ModifyMenu(hmenu,POPUP_TRV_FILES_AUTO_SEARCH  ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_AUTO_SEARCH  ,cps[TXT_POPUP_AUTO_SEARCH_STOP].c);
          }else
          {
            ModifyMenu(hmenu,POPUP_TRV_FILES_AUTO_SEARCH  ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_AUTO_SEARCH  ,cps[TXT_POPUP_AUTO_SEARCH].c);
            ModifyMenu(hmenu,POPUP_TRV_FILES_AUTO_SEARCH_PATH  ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_AUTO_SEARCH_PATH  ,cps[TXT_POPUP_AUTO_SEARCH_PATH].c);
          }

          if (SendMessage(htrv_files,TVM_GETCOUNT,(WPARAM)0,(LPARAM)0) > 4)
          {
            ModifyMenu(hmenu,POPUP_TRV_FILES_UP           ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_UP           ,cps[TXT_POPUP_UP].c);
            ModifyMenu(hmenu,POPUP_TRV_FILES_DOWN         ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_DOWN         ,cps[TXT_POPUP_DOWN].c);
            ModifyMenu(hmenu,POPUP_TRV_FILES_REMOVE_ITEMS ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_REMOVE_ITEMS ,cps[TXT_POPUP_REMOVE_ITEMS].c);
            ModifyMenu(hmenu,POPUP_TRV_FILES_CLEAN_ALL    ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_CLEAN_ALL    ,cps[TXT_POPUP_CLEAN_ALL].c);
            ModifyMenu(hmenu,POPUP_TRV_FILES_OPEN_PATH    ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_OPEN_PATH    ,cps[TXT_POPUP_OPEN_PATH].c);
            ModifyMenu(hmenu,POPUP_TRV_FILES_SAVE_LIST    ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_SAVE_LIST    ,cps[TXT_POPUP_SAVE_LIST].c);

            if (BACKUP_FILE_LIST_started) ModifyMenu(hmenu,POPUP_TRV_FILES_BACKUP       ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_BACKUP       ,cps[TXT_POPUP_BACKUP_FILE_LIST_STOP].c);
            else ModifyMenu(hmenu,POPUP_TRV_FILES_BACKUP       ,MF_BYCOMMAND|MF_STRING ,POPUP_TRV_FILES_BACKUP       ,cps[TXT_POPUP_BACKUP_FILE_LIST].c);
          }else
          {
            RemoveMenu(GetSubMenu(hmenu,0),2              ,MF_BYPOSITION);

            RemoveMenu(hmenu,POPUP_TRV_FILES_UP           ,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_TRV_FILES_DOWN         ,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_TRV_FILES_REMOVE_ITEMS ,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_TRV_FILES_CLEAN_ALL    ,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_TRV_FILES_OPEN_PATH    ,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_TRV_FILES_SAVE_LIST    ,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_TRV_FILES_BACKUP       ,MF_BYCOMMAND);
          }

          //affichage du popup menu
          POINT pos;
          if (GetCursorPos(&pos)!=0)
          {
            TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, pos.x, pos.y,hwnd, NULL);
          }else TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),hwnd, NULL);
          DestroyMenu(hmenu);
        }
      }
    break;
    case WM_DROPFILES://gestion du drag and drop de fichier ^^
    {
      if (B_AUTOSEARCH)break;
      HDROP H_DropInfo=(HDROP)wParam;
      char tmp[MAX_LINE_SIZE];
      DWORD i,nb_path = DragQueryFile(H_DropInfo, 0xFFFFFFFF, tmp, MAX_LINE_SIZE);
      for (i=0;i<nb_path;i++)
      {
        //get data
        DragQueryFile(H_DropInfo, i, tmp, MAX_LINE_SIZE);
        //add
        FileToTreeView(tmp);
      }
      DragFinish(H_DropInfo);

      //tri and clean
      CleanTreeViewFiles(htrv_files);

      //expend des branches
      SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
      SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
      SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
      SendMessage(htrv_files,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
    }
    break;
    case WM_INITDIALOG:
      //add language correction
      SetWindowText(GetDlgItem(hwnd,BT_ACL_FILE_CHK),cps[TXT_CHECK_ACL].c);
      SetWindowText(GetDlgItem(hwnd,BT_SHA_FILE_CHK),cps[TXT_CHECK_SHA].c);
      SetWindowText(GetDlgItem(hwnd,BT_ADS_FILE_CHK),cps[TXT_CHECK_ADS].c);
      SetWindowText(GetDlgItem(hwnd,BT_START),cps[TXT_BT_START].c);
      SetWindowText(GetDlgItem(hwnd,GRP_CONF),cps[TXT_GRP_CONF].c);

      //check all tests
      CheckDlgButton(hwnd,BT_ACL_FILE_CHK,BST_CHECKED);
      CheckDlgButton(hwnd,BT_ADS_FILE_CHK,BST_CHECKED);
      CheckDlgButton(hwnd,BT_UTC_CHK,BST_CHECKED);

      //add icon
      SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(GetModuleHandle(0), MAKEINTRESOURCE(ICON_APP)));

      //add files owners
      htrv_files = GetDlgItem(hwnd,TRV_FILES);
      SendMessage(htrv_files,CBEM_SETIMAGELIST,0,(LPARAM)H_ImagList_icon);
      TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]      = AddItemTreeView(htrv_files,cps[TXT_FILE_AUDIT].c, TVI_ROOT);
      TRV_HTREEITEM_CONF[FILES_TITLE_FILES]     = AddItemTreeView(htrv_files,cps[TXT_FILE_REP].c, TVI_ROOT);
      TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]  = AddItemTreeView(htrv_files,cps[TXT_FILE_REGISTRY].c, TVI_ROOT);
      TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]     = AddItemTreeView(htrv_files,cps[TXT_FILE_APPLI].c, TVI_ROOT);

      //add list of test
      //get all tests in list of rubriques
      htrv_test = GetDlgItem(hwnd,TRV_TEST);
      unsigned int i;
      NB_TESTS = SendMessage(hlstbox, LB_GETCOUNT,0,0);
      char tmp[DEFAULT_TMP_SIZE];
      for (i=0;i<NB_TESTS;i++)
      {
        if (SendMessage(hlstbox, LB_GETTEXTLEN,i,0) < DEFAULT_TMP_SIZE)
        {
          tmp[0] = 0;
          if(SendMessage(hlstbox, LB_GETTEXT,i,(LPARAM)tmp) > 0)
          {
            //add item
            H_tests[i] = AddItemTreeView(htrv_test,tmp, TVI_ROOT);
          }
        }
      }
    break;
    case WM_SIZE:
    {
      unsigned int mWidth  = LOWORD(lParam);
      unsigned int mHeight = HIWORD(lParam);

      //controle de la taille minimum
      if ((mWidth<800) || (mHeight<600))
      {
        RECT Rect;
        GetWindowRect(hwnd, &Rect);
        MoveWindow(hwnd,Rect.left,Rect.top,800+20,600+64,TRUE);
      }else
      {
        MoveWindow(GetDlgItem(hwnd,TRV_FILES),0,0,mWidth/2,mHeight-27,TRUE);
        MoveWindow(GetDlgItem(hwnd,TRV_TEST),mWidth/2+2,0,(mWidth/2)-2,mHeight-167,TRUE);

        MoveWindow(GetDlgItem(hwnd,GRP_CONF),mWidth/2+2,mHeight-165,(mWidth/2)-2,98,TRUE);

        MoveWindow(GetDlgItem(hwnd,BT_ACL_FILE_CHK),mWidth/2+20,mHeight-148,(mWidth/4)-40,17,TRUE);
        MoveWindow(GetDlgItem(hwnd,BT_ADS_FILE_CHK),mWidth/2+20,mHeight-128,(mWidth/4)-40,17,TRUE);
        MoveWindow(GetDlgItem(hwnd,BT_SHA_FILE_CHK),mWidth/2+20,mHeight-108,(mWidth/4)-40,17,TRUE);
        MoveWindow(GetDlgItem(hwnd,BT_UTC_CHK),mWidth/2+20,mHeight-88,(mWidth/4)-40,17,TRUE);

        MoveWindow(GetDlgItem(hwnd,ST_NAME_SESSION),mWidth*3/4+20,mHeight-148,(mWidth/4)-40,17,TRUE);
        MoveWindow(GetDlgItem(hwnd,EDT_NAME_SESSION),mWidth*3/4+20,mHeight-128,(mWidth/4)-40,17,TRUE);

        MoveWindow(GetDlgItem(hwnd,BT_RA_CHK),mWidth*3/4+20,mHeight-108,(mWidth/4)-40,17,TRUE);
        MoveWindow(GetDlgItem(hwnd,BT_MAGIC_CHK),mWidth*3/4+20,mHeight-88,(mWidth/4)-40,17,TRUE);

        MoveWindow(GetDlgItem(hwnd,BT_START),mWidth/2+2,mHeight-64,(mWidth/2)-2,38,TRUE);
        MoveWindow(GetDlgItem(hwnd,DLG_CONF_SB),0,mHeight-25,mWidth,25,TRUE);
      }
      InvalidateRect(hwnd, NULL, TRUE);
    }
    break;
    case WM_CLOSE:
    {
      //if in stop case
      if ((start_scan == FALSE) && (stop_scan == TRUE))
      {
        sqlite3_close(db_scan);
        CloseWindow(hwnd);
        PostQuitMessage(0);
      }

      //kill all threads
      unsigned int i;
      DWORD IDThread;
      for (i=0;i<NB_TESTS;i++)
      {
        GetExitCodeThread(h_thread_test[i],&IDThread);
        TerminateThread(h_thread_test[i],IDThread);
      }

      GetExitCodeThread(h_thread_scan,&IDThread);
      TerminateThread(h_thread_scan,IDThread);

      ShowWindow (h_main, SW_SHOW);
      UpdateWindow(h_main);
      EndDialog(hwnd, 0);
    }
    break;
  }
  return 0;
}
