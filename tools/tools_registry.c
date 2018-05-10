//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void ReadArboRawRegFile(HK_F_OPEN *hks, HBIN_CELL_NK_HEADER *nk_h, char *reg_file, HTREEITEM hparent, char *parent, char *root, HANDLE hlv, HANDLE htv)
{
  //get first root, if valide ?
  if (nk_h == NULL)return;

  //read all nk
  char tmp_key[MAX_PATH], tmp_root[MAX_PATH], tmp_parent[MAX_PATH];
  DWORD i,nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
  for (i=0;i<nbSubKey;i++)
  {
    if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, i, tmp_key, MAX_PATH))
    {
      snprintf(tmp_parent,MAX_PATH,"%s%s\\",parent,tmp_key);
      snprintf(tmp_root,MAX_PATH,"%s\\%s",root,tmp_key);

      ReadArboRawRegFile(hks,
                         GetSubNKtonk(hks->buffer, hks->taille_fic, nk_h, hks->position, i),
                         reg_file,
                         AddItemTreeViewImg(htv,tmp_key, hparent,ICON_DIRECTORY_REG),
                         tmp_parent,
                         tmp_root,
                         hlv, htv);
    }
  }

  //init
  LINE_ITEM lv_line[DLG_REG_LV_NB_COLUMN];
  char parent_key_update[DATE_SIZE_MAX];
  char Owner_SID[MAX_PATH];
  char tmp_value_trv[MAX_PATH];
  DWORD nbSubValue, type;
  strncpy(lv_line[0].c,reg_file,MAX_LINE_SIZE);
  strncpy(lv_line[1].c,parent,MAX_LINE_SIZE);

  lv_line[7].c[0] = 0;  //deleted = no view in this state
  lv_line[8].c[0] = 0;

  //read nk infos :)
  Readnk_Infos(hks->buffer,hks->taille_fic, (hks->pos_fhbin), hks->position,
               NULL, nk_h, parent_key_update, DATE_SIZE_MAX, NULL, 0,Owner_SID, MAX_PATH);

  Readnk_Class(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,
               NULL, nk_h, lv_line[8].c, MAX_PATH);

  //read all vk
  nbSubValue = GetValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
  for (i=0;i<nbSubValue;i++)
  {
    type = GetValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, i,lv_line[2].c,MAX_LINE_SIZE,lv_line[3].c,MAX_LINE_SIZE);
    switch(type)
    {
      case 0x00000001:
        strcpy(lv_line[4].c,"REG_SZ\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_TXT_REG);
      break;
      case 0x00000002:
        strcpy(lv_line[4].c,"REG_EXPAND_SZ\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_TXT_REG);
      break;
      case 0x00000003:
        strcpy(lv_line[4].c,"REG_BINARY\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_BIN_REG);
      break;
      case 0x00000004:
      case 0x00000005:
        strcpy(lv_line[4].c,"REG_DWORD\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_DWORD_REG);
      break;
      case 0x00000006:
        strcpy(lv_line[4].c,"REG_LINK\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_BIN_REG);
      break;
      case 0x00000007:
        strcpy(lv_line[4].c,"REG_MULTI_SZ\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_TXT_REG);
      break;
      case 0x0000000A:
        strcpy(lv_line[4].c,"REG_RESOURCE_REQUIREMENTS_LIST\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_BIN_REG);
      break;
      case 0x0000000b:
        strcpy(lv_line[4].c,"REG_QWORD\0");
        snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_DWORD_REG);
      break;
      default:
        if (type == 0x00000000)
        {
          strcpy(lv_line[4].c,"REG_NONE\0");
          snprintf(tmp_value_trv,MAX_PATH,"%s=%s",lv_line[2].c,lv_line[3].c);
        }else
        {
          strcpy(lv_line[4].c,"UNKNOW\0");
          snprintf(tmp_value_trv,MAX_PATH,"%s=(type:0x%08X)%s",lv_line[2].c,type,lv_line[3].c);
        }
        AddItemTreeViewImg(htv,tmp_value_trv, hparent,ICON_FILE_UNKNOW_REG);
      break;
    }

    //add to lstv
    strcpy(lv_line[5].c,parent_key_update);
    strcpy(lv_line[6].c,Owner_SID);
    AddToLVRegBin(hlv, lv_line, DLG_REG_LV_NB_COLUMN);
  }

  //no value : only directory
  if (nbSubValue < 1 && nk_h->nb_subkeys <1)
  {
    lv_line[2].c[0] = 0;
    lv_line[3].c[0] = 0;
    lv_line[4].c[0] = 0;
    strcpy(lv_line[5].c,parent_key_update);
    strcpy(lv_line[6].c,Owner_SID);
    AddToLVRegBin(hlv, lv_line, DLG_REG_LV_NB_COLUMN);
  }

  DWORD nb = ListView_GetItemCount(hlv);
  if (nb % 1000 == 0)
  {
    char tmp[MAX_PATH];
    snprintf(tmp,MAX_PATH,"Loading... %lu keys",nb);
    SendMessage(GetDlgItem(h_reg,STB),SB_SETTEXT,0, (LPARAM)tmp);
  }
}
//------------------------------------------------------------------------------
void GetRegFile(char *reg_file, HTREEITEM hparent, char *parent, BOOL recovery_mode, HANDLE hlv, HANDLE htv)
{
  if (reg_file[0] == 0) return;

  //load a binary file to TreeView and ListView
  if (recovery_mode)
  {
    GetRecoveryRegFile(reg_file, hparent, parent, hlv, htv);
    TreeView_SortChildren(htv,hparent,TRUE);
  }else
  {
    //simple mode
    HK_F_OPEN hks_tmp;
    if(OpenRegFiletoMem(&hks_tmp, reg_file))
    {
      ReadArboRawRegFile(&hks_tmp, (HBIN_CELL_NK_HEADER *)(hks_tmp.buffer+hks_tmp.position), reg_file, hparent, parent,"\\", hlv, htv);
      CloseRegFiletoMem(&hks_tmp);
    }
    TreeView_SortChildren(htv,hparent,TRUE);
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI LoadRegFiles(LPVOID lParam)
{
  //init
  HANDLE htv          = GetDlgItem(h_reg,TV_VIEW);
  HANDLE hlv          = GetDlgItem(h_reg,LV_VIEW);
  BOOL recovery_mode  = SendDlgItemMessage(h_reg_file,BT_REG_RECOVERY_MODE_CHK, BM_GETCHECK,(WPARAM) 0, (LPARAM)0);
  EnableWindow(hlv,FALSE);
  SendMessage(GetDlgItem(h_reg,STB),SB_SETTEXT,0, (LPARAM)"");

  char path[MAX_PATH]="";
  //HKEY_CURRENT_USER
  if(GetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_NTUSER),path,MAX_PATH))
    GetRegFile(path, AddItemTreeViewImg(htv,"HKEY_CURRENT_USER", TVI_ROOT,ICON_DIRECTORY_REG), "HKEY_CURRENT_USER\\", recovery_mode, hlv, htv);

  //HKEY_LOCAL_MACHINE
  HTREEITEM h_hklm = AddItemTreeViewImg(htv,"HKEY_LOCAL_MACHINE", TVI_ROOT,ICON_DIRECTORY_REG);
  path[0] = 0;
  if(GetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SAM),path,MAX_PATH))
    GetRegFile(path, AddItemTreeViewImg(htv,"SAM", h_hklm,ICON_DIRECTORY_REG), "HKEY_LOCAL_MACHINE\\SAM\\", recovery_mode, hlv, htv);

  path[0] = 0;
  if(GetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SECURITY),path,MAX_PATH))
    GetRegFile(path, AddItemTreeViewImg(htv,"SECURITY", h_hklm,ICON_DIRECTORY_REG), "HKEY_LOCAL_MACHINE\\SECURITY\\", recovery_mode, hlv, htv);

  path[0] = 0;
  if(GetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SOFTWARE),path,MAX_PATH))
    GetRegFile(path, AddItemTreeViewImg(htv,"SOFTWARE", h_hklm,ICON_DIRECTORY_REG), "HKEY_LOCAL_MACHINE\\SOFTWARE\\", recovery_mode, hlv, htv);

  path[0] = 0;
  if(GetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SYSTEM),path,MAX_PATH))
    GetRegFile(path, AddItemTreeViewImg(htv,"SYSTEM", h_hklm,ICON_DIRECTORY_REG), "HKEY_LOCAL_MACHINE\\SYSTEM\\", recovery_mode, hlv, htv);

  //other
  path[0] = 0;
  if(GetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_OTHER),path,MAX_PATH))
    GetRegFile(path, AddItemTreeViewImg(htv,"HKEY_???", TVI_ROOT,ICON_DIRECTORY_REG), "HKEY_???\\", recovery_mode, hlv, htv);

  //check nb items
  if (SendDlgItemMessage(h_reg,TV_VIEW,TVM_GETCOUNT,0,0) < 5)
    SendDlgItemMessage(h_reg,TV_VIEW,TVM_DELETEITEM,0,0);

  //state
  char tmp[MAX_PATH];
  snprintf(tmp,MAX_PATH,"Load : %lu keys",(DWORD)ListView_GetItemCount(hlv));
  SendMessage(GetDlgItem(h_reg,STB),SB_SETTEXT,0, (LPARAM)tmp);

  EnableWindow(hlv,TRUE);
  SetWindowText(GetDlgItem(h_reg_file,BT_REG_START),cps[TXT_BT_START].c);
  EnableWindow(GetDlgItem(h_reg_file,BT_REG_START),TRUE);
  reg_file_start_process = FALSE;
  return 0;
}
//------------------------------------------------------------------------------
int LireTreeTxt(HANDLE hparent, DWORD treeview, HTREEITEM hitem, char *txt, DWORD max_size)
{
  txt[0]=0;
  TV_ITEM tvi;
  tvi.mask = TVIF_TEXT;
  tvi.pszText = txt;
  tvi.iImage =0;
  tvi.cchTextMax = max_size;
  tvi.hItem = hitem;
  SendDlgItemMessage(hparent, treeview, TVM_GETITEM, (WPARAM)NULL, (LPARAM)(long)&tvi);
  return strlen(txt);
}
//------------------------------------------------------------------------------
void GetItemPath(HANDLE hparent, DWORD treeview, HTREEITEM hitem, char *path, DWORD max_size)
{
  char tmp_path[MAX_PATH],tmp_path2[MAX_PATH];
  path[0]=0;
  HTREEITEM hitem_parent = hitem;

  do{
    LireTreeTxt(hparent, treeview, hitem_parent,tmp_path, max_size);
    if (path[0]==0)
    {
      strncpy(path,tmp_path,max_size);
    }else
    {
      if (tmp_path[strlen(tmp_path)-1]=='\\')snprintf(tmp_path2,MAX_PATH,"%s%s",tmp_path,path);
      else snprintf(tmp_path2,MAX_PATH,"%s\\%s",tmp_path,path);
      strncpy(path,tmp_path2,max_size);
    }
  }while ((hitem_parent = (HTREEITEM)SendDlgItemMessage(hparent, treeview, TVM_GETNEXTITEM, (WPARAM)TVGN_PARENT, (LPARAM)hitem_parent)));
}
//------------------------------------------------------------------------------
void CopyTVData(HANDLE hparent, DWORD treeview, HTREEITEM hitem, BOOL item_only)
{
  char tmp[MAX_PATH];
  if (item_only) LireTreeTxt(hparent, treeview, hitem, tmp, MAX_PATH);//item only;
  else GetItemPath(hparent, treeview, hitem, tmp, MAX_PATH); //all path

  //copy dans le presse papier
  if(OpenClipboard(NULL))
  {
    EmptyClipboard();
    HANDLE hGlobal = GlobalAlloc(GHND | GMEM_SHARE, strlen(tmp)+1);
    if (hGlobal!=NULL)
    {
      char *p = (char *)GlobalLock(hGlobal);
      if (p != NULL)
      {
        strcpy(p, tmp);
      }
      GlobalUnlock(hGlobal);
      SetClipboardData(CF_TEXT, hGlobal);
    }
    CloseClipboard();
  }
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_reg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            //------------------------------------------
            case BT_SEARCH:
            {
              char tmp[MAX_PATH];
              //select lstv
              SetFocus(GetDlgItem(hwnd,LV_VIEW));
              SendDlgItemMessage(hwnd,ED_SEARCH,WM_GETTEXT ,(WPARAM)MAX_PATH, (LPARAM)tmp);
              if (!SendDlgItemMessage(hwnd,BT_SEARCH_MATCH_CASE, BM_GETCHECK,(WPARAM) 0, (LPARAM)0)) pos_search_reg = LVSearchNoCass(GetDlgItem(hwnd,LV_VIEW), DLG_REG_LV_NB_COLUMN, tmp, pos_search_reg);
              else pos_search_reg = LVSearch(GetDlgItem(hwnd,LV_VIEW), DLG_REG_LV_NB_COLUMN, tmp, pos_search_reg);
            }
            break;
            //------------------------------------------
            case BT_TREE_VIEW:
              if (IsWindowVisible(GetDlgItem(hwnd,LV_VIEW)))
              {
                ShowWindow(GetDlgItem(hwnd,ED_SEARCH), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd,BT_SEARCH), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd,LV_VIEW), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd,TV_VIEW), SW_SHOW);
              }else
              {
                ShowWindow(GetDlgItem(hwnd,TV_VIEW), SW_HIDE);
                ShowWindow(GetDlgItem(hwnd,ED_SEARCH), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd,BT_SEARCH), SW_SHOW);
                ShowWindow(GetDlgItem(hwnd,LV_VIEW), SW_SHOW);
              }
            break;
            //------------------------------------------
            case POPUP_TV_CP_COMPLET_PATH:CopyTVData(hwnd,TV_VIEW, (HTREEITEM)SendDlgItemMessage(hwnd, TV_VIEW, TVM_GETNEXTITEM, TVGN_CARET, 0), FALSE);break;
            case POPUP_TV_CP_VALUE_AND_DATA:CopyTVData(hwnd,TV_VIEW, (HTREEITEM)SendDlgItemMessage(hwnd, TV_VIEW, TVM_GETNEXTITEM, TVGN_CARET, 0), TRUE);break;
            //------------------------------------------
              case POPUP_S_VIEW:
              {
                char file[MAX_PATH]="raw_registry_export";
                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter ="*.csv \0*.csv\0*.xml \0*.xml\0*.html \0*.html\0*.reg \0*.reg\0";
                ofn.nFilterIndex = 1;
                ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                ofn.lpstrDefExt ="csv\0";
                if (GetSaveFileName(&ofn)==TRUE)
                {
                  if (ofn.nFilterIndex == SAVE_TYPE_REG5)SaveLSTVItemstoREG(GetDlgItem(hwnd,LV_VIEW), file, FALSE);
                  else SaveLSTV(GetDlgItem(hwnd,LV_VIEW), file, ofn.nFilterIndex, DLG_REG_LV_NB_COLUMN);
                  SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Export done !!!");
                }
              }
              break;
              //-----------------------------------------------------
              case POPUP_S_SELECTION:
              {
                char file[MAX_PATH]="raw_registry_selected_keys_export";
                OPENFILENAME ofn;
                ZeroMemory(&ofn, sizeof(OPENFILENAME));
                ofn.lStructSize = sizeof(OPENFILENAME);
                ofn.hwndOwner = hwnd;
                ofn.lpstrFile = file;
                ofn.nMaxFile = MAX_PATH;
                ofn.lpstrFilter ="*.csv \0*.csv\0*.xml \0*.xml\0*.html \0*.html\0*.reg \0*.reg\0";
                ofn.nFilterIndex = 1;
                ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
                ofn.lpstrDefExt ="csv\0";
                if (GetSaveFileName(&ofn)==TRUE)
                {
                  if (ofn.nFilterIndex == SAVE_TYPE_REG5)SaveLSTVItemstoREG(GetDlgItem(hwnd,LV_VIEW), file, TRUE);
                  else SaveLSTVSelectedItems(GetDlgItem(hwnd,LV_VIEW), file, ofn.nFilterIndex, DLG_REG_LV_NB_COLUMN);
                  SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Export done !!!");
                }
              }
              break;
              //-----------------------------------------------------
              case POPUP_A_SEARCH:
              {
                char tmp[MAX_PATH];
                SendDlgItemMessage(hwnd,ED_SEARCH,WM_GETTEXT ,(WPARAM)MAX_PATH, (LPARAM)tmp);
                LVAllSearch(GetDlgItem(hwnd,LV_VIEW), DLG_REG_LV_NB_COLUMN, tmp);
              }
              break;
              //-----------------------------------------------------
              case POPUP_I_00:CopyDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 0);break;
              case POPUP_I_01:CopyDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 1);break;
              case POPUP_I_02:CopyDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 2);break;
              case POPUP_I_03:CopyDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 3);break;
              case POPUP_I_04:CopyDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 4);break;
              case POPUP_I_05:CopyDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 5);break;
              case POPUP_I_06:CopyDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 6);break;
              //-----------------------------------------------------
              case POPUP_CP_LINE:CopyAllDataToClipboard(GetDlgItem(hwnd,LV_VIEW), SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), DLG_REG_LV_NB_COLUMN);break;
              //-----------------------------------------------------
              case POPUP_O_PATH:
              {
                char path[MAX_PATH]="";
                ListView_GetItemText(GetDlgItem(hwnd,LV_VIEW),SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED),0,path,MAX_PATH);
                if (path[0]!=0)
                {
                  char *c = path;
                  while (*c++);
                  while (*c != '\\' && *c != '/')c--;
                  c++;
                  *c=0;
                  ShellExecute(h_main, "explore", path, NULL,NULL,SW_SHOW);
                }
              }
              break;
          }
        break;
      }
    break;
    case WM_CLOSE : ShowWindow(hwnd, SW_HIDE);break;
    case WM_CONTEXTMENU:
      if ((HWND)wParam == GetDlgItem(hwnd,TV_VIEW))
      {
        //select trv item
        TV_HITTESTINFO tvh_info;
        tvh_info.pt.x = LOWORD(lParam);
        tvh_info.pt.y = HIWORD(lParam);
        ScreenToClient(GetDlgItem(hwnd,TV_VIEW), &(tvh_info.pt));
        HTREEITEM hItemSelect = TreeView_HitTest(GetDlgItem(hwnd,TV_VIEW), &tvh_info);
        if (hItemSelect != 0)TreeView_SelectItem(GetDlgItem(hwnd,TV_VIEW),hItemSelect);

        HMENU hmenu;
        if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_TV_REGISTRY)))!= NULL)
        {
          POINT pos;
          if (GetCursorPos(&pos)!=0)
          {
            TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, pos.x, pos.y,hwnd, NULL);
          }else TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),hwnd, NULL);
          DestroyMenu(hmenu);
        }
      }else if ((HWND)wParam == GetDlgItem(hwnd,LV_VIEW))
      {
        HMENU hmenu;
        if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_LSTV_REGISTRY)))!= NULL)
        {
          POINT pos;
          if (GetCursorPos(&pos)!=0)
          {
            TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, pos.x, pos.y,hwnd, NULL);
          }else TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),hwnd, NULL);
          DestroyMenu(hmenu);
        }
      }
    break;
    case WM_SIZE:
    {
      unsigned int mWidth  = LOWORD(lParam);
      unsigned int mHeight = HIWORD(lParam);

      //controle de la taille minimum
      if (mWidth<800 ||mHeight<600)
      {
        RECT Rect;
        GetWindowRect(hwnd, &Rect);
        MoveWindow(hwnd,Rect.left,Rect.top,800+20,600+64,TRUE);
      }else
      {
        MoveWindow(GetDlgItem(hwnd,ED_SEARCH),5,0,mWidth-122,22,TRUE);
        MoveWindow(GetDlgItem(hwnd,BT_SEARCH),mWidth-113,0,55,22,TRUE);
        MoveWindow(GetDlgItem(hwnd,BT_TREE_VIEW),mWidth-57,0,55,22,TRUE);
        MoveWindow(GetDlgItem(hwnd,BT_SEARCH_MATCH_CASE),mWidth-113,24,120,18,TRUE);

        MoveWindow(GetDlgItem(hwnd,LV_VIEW),5,47,mWidth-10,mHeight-73,TRUE);
        MoveWindow(GetDlgItem(hwnd,TV_VIEW),5,47,mWidth-10,mHeight-73,TRUE);
        MoveWindow(GetDlgItem(hwnd,STB),0,mHeight-22,mWidth,22,TRUE);

        //column resise
        unsigned int i;
        DWORD column_sz = (mWidth-30)/DLG_REG_LV_NB_COLUMN;
        for (i=0;i<DLG_REG_LV_NB_COLUMN;i++)
        {
          redimColumnH(GetDlgItem(hwnd,LV_VIEW),i,column_sz);
        }
      }
      InvalidateRect(hwnd, NULL, TRUE);
    }
    break;
    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code)
      {
        case LVN_COLUMNCLICK:
          TRI_REG_VIEW = !TRI_REG_VIEW;
          c_Tri(GetDlgItem(h_reg,LV_VIEW),((LPNMLISTVIEW)lParam)->iSubItem,TRI_REG_VIEW);
        break;
        case NM_DBLCLK:
          if (LOWORD(wParam) == LV_VIEW)
          {
            long index = SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
            int i;
            if (index > -1)
            {
              char tmp[MAX_LINE_SIZE+1], tmp2[MAX_LINE_SIZE+1];
              RichEditInit(GetDlgItem(h_info,DLG_INFO_TXT));

              //for each column
              LVCOLUMN lvc;
              lvc.mask        = LVCF_TEXT;
              lvc.cchTextMax  = MAX_LINE_SIZE;
              lvc.pszText     = tmp;

              for (i=0;i<DLG_REG_LV_NB_COLUMN;i++)
              {
                tmp[0] = 0;
                tmp2[0] = 0;
                if (SendDlgItemMessage(hwnd,LV_VIEW,LVM_GETCOLUMN,(WPARAM)i,(LPARAM)&lvc) != 0)
                {
                  if (*tmp != '\0')
                  {
                    ListView_GetItemText(GetDlgItem(h_reg,LV_VIEW),index,i,tmp2,MAX_LINE_SIZE);
                    if (*tmp2 != '\0')
                    {
                      RichEditCouleur(GetDlgItem(h_info,DLG_INFO_TXT),NOIR,"\r\n[");
                      RichEditCouleurGras(GetDlgItem(h_info,DLG_INFO_TXT),NOIR,tmp);
                      RichEditCouleur(GetDlgItem(h_info,DLG_INFO_TXT),NOIR,"]\r\n");
                      RichEditCouleur(GetDlgItem(h_info,DLG_INFO_TXT),NOIR,tmp2);
                      RichEditCouleur(GetDlgItem(h_info,DLG_INFO_TXT),NOIR,"\r\n");
                    }
                  }
                }
              }
              RichSetTopPos(GetDlgItem(h_info,DLG_INFO_TXT));
              if(RichEditTextSize(GetDlgItem(h_info,DLG_INFO_TXT)))
              {
                ShowWindow (h_info, SW_SHOW);
                UpdateWindow(h_info);
              }
            }
            RichSetTopPos(GetDlgItem(h_info,DLG_INFO_TXT));
          }
        break;
      }
    break;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
void InitDlgRegfile()
{
  SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_NTUSER),"");
  SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SAM),"");
  SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SECURITY),"");
  SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SOFTWARE),"");
  SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SYSTEM),"");
  SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_OTHER),"");

  TreeView_DeleteAllItems(GetDlgItem(h_reg,TV_VIEW));
  ListView_DeleteAllItems(GetDlgItem(h_reg,LV_VIEW));
}
//------------------------------------------------------------------------------
void AddDlgRegfilePath(DWORD id_item)
{
  char path[MAX_PATH]="";
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_reg_file;
  ofn.lpstrFile = path;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFilter = "*.* \0*.*\0";;
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_FILEMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_EXPLORER|OFN_SHOWHELP;
  ofn.lpstrDefExt ="*.*";
  if (GetOpenFileName(&ofn)==TRUE)
  {
    switch(id_item)
    {
      case DLG_REG_BT_NTUSER : SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_NTUSER),path);break;
      case DLG_REG_BT_SAM : SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SAM),path);break;
      case DLG_REG_BT_SECURITY : SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SECURITY),path);break;
      case DLG_REG_BT_SOFTWARE : SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SOFTWARE),path);break;
      case DLG_REG_BT_SYSTEM : SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_SYSTEM),path);break;
      case DLG_REG_BT_OTHER : SetWindowText(GetDlgItem(h_reg_file,DLG_REG_ED_OTHER),path);break;
    }
  }
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_reg_file(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            //------------------------------------------
            case BT_REG_START:
              if (!reg_file_start_process)
              {
                ShowWindow(h_reg_file, SW_HIDE);
                ShowWindow(h_reg, SW_SHOW);
                UpdateWindow(h_reg);
                //textbutton
                SetWindowText(GetDlgItem(hwnd,BT_REG_START),cps[TXT_BT_STOP].c);
                reg_file_start_process = TRUE;
                pos_search_reg         = 0;
                //process
                CreateThread(NULL,0,LoadRegFiles,NULL,0,0);
              }else
              {
                //stop thread
                reg_file_start_process = FALSE;
                //textbutton
                SetWindowText(GetDlgItem(hwnd,BT_REG_START),cps[TXT_BT_START].c);
                EnableWindow(GetDlgItem(hwnd,BT_REG_START),FALSE);
                ShowWindow(h_reg_file, SW_HIDE);
                ShowWindow(h_reg, SW_SHOW);
                UpdateWindow(h_reg);
              }
            break;
            //------------------------------------------
            case DLG_REG_BT_NTUSER:
            case DLG_REG_BT_SAM:
            case DLG_REG_BT_SECURITY:
            case DLG_REG_BT_SOFTWARE:
            case DLG_REG_BT_SYSTEM:
            case DLG_REG_BT_OTHER:AddDlgRegfilePath(LOWORD(wParam));break;
          }
        break;
      }
    break;
    case WM_CLOSE : ShowWindow(hwnd, SW_HIDE);break;
  }
  return FALSE;
}
