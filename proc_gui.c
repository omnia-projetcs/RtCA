//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "RtCA.h"
//------------------------------------------------------------------------------
unsigned long int Contient(char*data,char*chaine)
{
  unsigned long int i=0;
  char *d = data;
  char *c = chaine;

  if (!*c || !*d) return 0;

  while (*d)
  {
    c = chaine;
    while (*d == *c && *c && *d){d++;c++;i++;}

    if (*c == 0) return i;
    d++;i++;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
unsigned long int ContientNoCass(char*data,char*chaine)
{
  unsigned long int i=0;
  char *d = charToLowChar(data);
  char *c = charToLowChar(chaine);
  if (!*c || !*d) return 0;

  while (*d)
  {
    c = chaine;
    while (*d == *c && *c && *d){d++;c++;i++;}

    if (*c == 0) return i;
    d++;i++;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
void LVDelete(HANDLE hlv)
{
  //test si des enregistrements
  DWORD NBLigne=SendMessage(hlv,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0);
  if (NBLigne>0)
  {
    int j;
    for (j=NBLigne-1;j>-1;j--)//ligne par ligne
    {
      //on vérifie que la ligne est bien sélectionnée
      if (SendMessage(hlv,LVM_GETITEMSTATE,(WPARAM)j,(LPARAM)LVIS_SELECTED) == LVIS_SELECTED)
        SendMessage(hlv,LVM_DELETEITEM,(WPARAM)j,(LPARAM)NULL);
    }
  }
}
//------------------------------------------------------------------------------
DWORD LVSearch(HANDLE hlv, unsigned short nb_col, char *search, DWORD start_id)
{
  DWORD i, j, nb_item = ListView_GetItemCount(hlv);
  char tmp[MAX_LINE_SIZE];
  for (i=start_id;i<nb_item;i++)
  {
    for (j=0;j<nb_col;j++)
    {
      //récupération du text
      tmp[0]=0;
      ListView_GetItemText(hlv,i,j,tmp,MAX_LINE_SIZE);

      //test si la recherche est présente dedans
      if (Contient(tmp,search))
      {
        //sélection
        SendMessage(hlv, LVM_ENSUREVISIBLE, i, 0);
        ListView_SetItemState(hlv, -1, 0, LVIS_SELECTED); // deselect all
        ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);

        //on quitte
        return i+1;
      }
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD LVSearchNoCass(HANDLE hlv, unsigned short nb_col, char *search, DWORD start_id)
{
  DWORD i, j, nb_item = ListView_GetItemCount(hlv);
  char tmp[MAX_LINE_SIZE];
  for (i=start_id;i<nb_item;i++)
  {
    for (j=0;j<nb_col;j++)
    {
      //récupération du text
      tmp[0]=0;
      ListView_GetItemText(hlv,i,j,tmp,MAX_LINE_SIZE);

      //test si la recherche est présente dedans
      if (ContientNoCass(tmp,search))
      {
        //sélection
        SendMessage(hlv, LVM_ENSUREVISIBLE, i, 0);
        ListView_SetItemState(hlv, -1, 0, LVIS_SELECTED); // deselect all
        ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);

        //on quitte
        return i+1;
      }
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
void LVAllSearch(HANDLE hlv, unsigned short nb_col, char *search)
{
  DWORD i, j, nb_item = ListView_GetItemCount(hlv);
  char tmp[MAX_LINE_SIZE];

  ListView_SetItemState(hlv, -1, 0, LVIS_SELECTED); // deselect all

  if (GetMenuState(GetMenu(h_main),BT_SEARCH_MATCH_CASE,MF_BYCOMMAND) == MF_CHECKED)
  {
    for (i=0;i<nb_item;i++)
    {
      for (j=0;j<nb_col;j++)
      {
        //récupération du text
        tmp[0]=0;
        ListView_GetItemText(hlv,i,j,tmp,MAX_LINE_SIZE);

        //test si la recherche est présente dedans
        if (Contient(tmp,search))
        {
          //sélection
          ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);
        }
      }
    }
  }else
  {
    for (i=0;i<nb_item;i++)
    {
      for (j=0;j<nb_col;j++)
      {
        //récupération du text
        tmp[0]=0;
        ListView_GetItemText(hlv,i,j,tmp,MAX_LINE_SIZE);

        //test si la recherche est présente dedans
        if (ContientNoCass(tmp,search))
        {
          //sélection
          ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
void Modify_Style(HANDLE hcomp, long style, BOOL add)
{
  if(add) SetWindowLong(hcomp, GWL_STYLE, GetWindowLong(hcomp, GWL_STYLE) | style);
  else SetWindowLong(hcomp, GWL_STYLE, GetWindowLong(hcomp, GWL_STYLE) & ~style);
}
//------------------------------------------------------------------------------
void FileToTreeView(char *c_path)
{
  if(isDirectory(c_path))
  {
    char path[MAX_PATH];
    if (c_path[strlen(c_path)-1]=='\\')snprintf(path,MAX_PATH,"%s",c_path);
    else snprintf(path,MAX_PATH,"%s\\",c_path);
    AddItemTreeView(htrv_files,path, TRV_HTREEITEM_CONF[FILES_TITLE_FILES]);
    check_treeview(htrv_test, H_tests[INDEX_FILE], TRV_STATE_CHECK);
  }else
  {
    char file[MAX_PATH];
    extractFileFromPath(c_path, file, MAX_PATH);
    AddItemFiletoTreeView(htrv_files, charToLowChar(file), NULL, c_path);
  }
}
//------------------------------------------------------------------------------
void CleanTreeViewFiles(HANDLE htrv)
{
  unsigned int i;
  //tri and clean
  for (i=0;i<NB_MX_TYPE_FILES_TITLE;i++)
  {
    SendMessage(htrv,TVM_SORTCHILDREN, TRUE,(LPARAM)TRV_HTREEITEM_CONF[i]);
    SupDoublon(htrv,TRV_HTREEITEM_CONF[i]);
    SendMessage(htrv,TVM_EXPAND, TVE_EXPAND,(LPARAM)TRV_HTREEITEM_CONF[i]);
  }
}
//------------------------------------------------------------------------------
int CALLBACK CompareStringTri(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3)
{
  sort_st *st = (sort_st *)lParam3;

  static char buf1[MAX_LINE_SIZE], buf2[MAX_LINE_SIZE];

  ListView_GetItemText(st->hlv, lParam1, st->col, buf1, MAX_LINE_SIZE);
  ListView_GetItemText(st->hlv, lParam2, st->col, buf2, MAX_LINE_SIZE);

  if (st->sort) return (strcmp(charToLowChar(buf1), charToLowChar(buf2)));
  else return (strcmp(charToLowChar(buf1), charToLowChar(buf2))*-1);
}
//------------------------------------------------------------------------------
void c_Tri(HANDLE hlv, unsigned short colonne_ref, BOOL sort)
{
  static sort_st st;
  st.hlv  = hlv;
  st.sort = sort;
  st.col  = colonne_ref;

  ListView_SortItemsEx(st.hlv,CompareStringTri, (LPARAM)&st);
}
//------------------------------------------------------------------------------
void redimColumnH(HANDLE hlv,int column,unsigned int col_size)
{
  LVCOLUMN lvc;
  lvc.mask = LVCF_WIDTH;
  lvc.cx = col_size;

  SendMessage(hlv,LVM_SETCOLUMN,(WPARAM)column, (LPARAM)&lvc);
}
//------------------------------------------------------------------------------
void AddtoLV(HANDLE hlv, unsigned int nb_column, LINE_ITEM *item, BOOL sel)
{
  LVITEM lvi;
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";
  lvi.iItem = ListView_GetItemCount(hlv);
  DWORD i=0, ref_item = ListView_InsertItem(hlv, &lvi);

  do{
    ListView_SetItemText(hlv,ref_item,i,item[i].c);
  }while (i++<nb_column);

  if (sel)
  {
    //sélection of the last !!!
    SendMessage(hlv, LVM_ENSUREVISIBLE, lvi.iItem, 0);
  }

}
//------------------------------------------------------------------------------
void GetItemTreeView(HTREEITEM hitem,HANDLE htrv,char *txt, unsigned int size)
{
  txt[0]          = 0;
  TV_ITEM tvi;
  tvi.mask        = TVIF_TEXT;
  tvi.pszText     = txt;
  tvi.cchTextMax  = size;
  tvi.hItem       = hitem;
  SendMessage(htrv,TVM_GETITEM, TVGN_CARET, (long)&tvi);
}
//------------------------------------------------------------------------------
HTREEITEM AddItemTreeView(HANDLE Htreeview,char *txt, HTREEITEM hparent)
{
  TV_INSERTSTRUCT tvinsertitem;
  tvinsertitem.hParent = hparent;
  tvinsertitem.hInsertAfter = TVI_ROOT;
  tvinsertitem.item.mask = TVIF_TEXT;
  tvinsertitem.item.pszText = txt;
  return (HTREEITEM)SendMessage(Htreeview,TVM_INSERTITEM ,(WPARAM) 0, (LPARAM)&tvinsertitem);
}

//------------------------------------------------------------------------------
HTREEITEM AddItemTreeViewImg(HANDLE Htreeview, char *txt, HTREEITEM hparent, unsigned int index_img)
{
  TV_INSERTSTRUCT tvitem;
  tvitem.hParent = hparent;
  tvitem.hInsertAfter = TVI_ROOT;
  tvitem.item.mask = TVIF_TEXT|TVIF_IMAGE|TVIF_SELECTEDIMAGE;
  tvitem.item.pszText = txt;
  tvitem.item.iImage=tvitem.item.iSelectedImage=index_img;
  return (HTREEITEM)SendMessage(Htreeview, TVM_INSERTITEM,(WPARAM)0, (LPARAM)&tvitem);
}
//------------------------------------------------------------------------------
//check treeview items state
BOOL Ischeck_treeview(HANDLE Htree, HTREEITEM hitem)
{
  TVITEM tvitem;
  tvitem.mask       = TVIF_HANDLE | TVIF_STATE;
  tvitem.hItem      = hitem;
  tvitem.stateMask  = TVIS_STATEIMAGEMASK;
  TreeView_GetItem(Htree, &tvitem);

  return ((BOOL)(tvitem.state >> 12) - 1);
}
//------------------------------------------------------------------------------
//check treeview item
void check_treeview(HANDLE htrv, HTREEITEM hitem, int state)
{
   TV_ITEM tvitem;
   tvitem.mask      = TVIF_HANDLE | TVIF_STATE;
   tvitem.hItem     = hitem;
   tvitem.stateMask = TVIS_STATEIMAGEMASK;
   tvitem.state     = INDEXTOSTATEIMAGEMASK(state);
   TreeView_SetItem(htrv, &tvitem);
}
//------------------------------------------------------------------------------
void check_childs_treeview(HANDLE htrv, BOOL check)
{
  unsigned int i=0, state = check?TRV_STATE_CHECK:TRV_STATE_UNCHECK;
  while (i<NB_TESTS)
  {
    check_treeview(htrv, H_tests[i++], state);
  }
}
//------------------------------------------------------------------------------
/*void uncheckdisable(HANDLE htrv, HTREEITEM hitem)
{
  check_treeview(htrv, hitem, TRV_STATE_UNCHECK_DISABLE);
}
//------------------------------------------------------------------------------
void disableAllcheck(HANDLE htrv, BOOL disable)
{
  int i;
  for (i=0;i<NB_TESTS;i++)
  {
    if (Ischeck_treeview(htrv, H_tests[i]))check_treeview(htrv, H_tests[i], disable?TRV_STATE_CHECK_DISABLE:TRV_STATE_CHECK);
    else check_treeview(htrv, H_tests[i], disable?TRV_STATE_UNCHECK_DISABLE:TRV_STATE_UNCHECK);
  }
}*/

//------------------------------------------------------------------------------
void SupDoublon(HANDLE htrv,HTREEITEM htreeParent)
{
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)htreeParent);
  HTREEITEM hprecitem = hitem;
  //on énumère tous les items fils du treeview
  char tmp[MAX_PATH],prec[MAX_PATH]="";
  do
  {
    //récupération du texte de l'item
    TVITEM tvitem;
    tmp[0]=0;
    tvitem.hItem = hitem;
    tvitem.mask = TVIF_TEXT;
    tvitem.pszText = tmp;
    tvitem.cchTextMax = MAX_PATH;
    if (SendMessage(htrv, TVM_GETITEM,(WPARAM)0, (LPARAM)&tvitem))
    {
      if (strcmp(tmp,prec)==0)
      {
        //on supprime l'item
        SendMessage(htrv, TVM_DELETEITEM,(WPARAM)NULL, (LPARAM)hitem);
        hitem = hprecitem;
      }else
      {
        strcpy(prec,tmp);
        hprecitem = hitem;
      }
    }
  }while((hitem = (HTREEITEM)SendMessage(htrv, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem)));
}
//------------------------------------------------------------------------------
void AddComboBoxItem(HANDLE hcombo, char *txt, DWORD img)
{
  //add item in combobox
  COMBOBOXEXITEM item;
  item.mask         = CBEIF_TEXT | CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
  item.iItem        = SendMessage(hcombo,CB_GETCOUNT,0,0);
  item.iImage       = item.iSelectedImage = img;
  item.pszText      = txt;
  item.cchTextMax   = strlen(txt);
  item.iIndent      = 0;
  SendMessage(hcombo,CBEM_INSERTITEM,0,(LPARAM)&item);
}
//-----------------------------------------------------------------------------
void IDM_STAY_ON_TOP_fct()
{
  STAY_ON_TOP = !STAY_ON_TOP;
  if (STAY_ON_TOP)
  {
    SetWindowPos(h_main     ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_process  ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_sniff    ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_reg_file ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_reg      ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_date     ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_state    ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_sqlite_ed,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_hexa     ,HWND_TOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    CheckMenuItem(GetMenu(h_main),IDM_STAY_ON_TOP,MF_BYCOMMAND|MF_CHECKED);
  }else
  {
    SetWindowPos(h_main     ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_process  ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_sniff    ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_reg_file ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_reg      ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_date     ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_state    ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_sqlite_ed,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    SetWindowPos(h_hexa     ,HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOSIZE);
    CheckMenuItem(GetMenu(h_main),IDM_STAY_ON_TOP,MF_BYCOMMAND|MF_UNCHECKED);
  }
}
//-----------------------------------------------------------------------------
void DisableGrid(HANDLE hlv, BOOL disable, int menu)
{
  if (disable)
  {
    CheckMenuItem(GetMenu(h_main),menu,MF_BYCOMMAND|MF_CHECKED);
    SendMessage(hlv,LVM_SETEXTENDEDLISTVIEWSTYLE,LVS_EX_GRIDLINES,0);
  }else
  {
    CheckMenuItem(GetMenu(h_main),menu,MF_BYCOMMAND|MF_UNCHECKED);
    SendMessage(hlv,LVM_SETEXTENDEDLISTVIEWSTYLE,0,LVS_EX_GRIDLINES);
  }
}
//-----------------------------------------------------------------------------
