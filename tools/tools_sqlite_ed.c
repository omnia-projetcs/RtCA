//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
#define TYPE_SQLITE_LOAD_TABLES         0x0001
#define TYPE_SQLITE_LOAD_TABLE_SELECTED 0x0002
#define TYPE_SQLITE_REQUEST             0x0003
#define TYPE_SQLITE_LOAD_TABLE_HEADER   0x0004
#define TYPE_SQLITE_LOAD_TABLE_STRUCT   0x0005
#define NB_VIEW_COMPONENT_SQLITE_ED     12
//------------------------------------------------------------------------------
sqlite3 *db_sqlite_test;
DWORD nb_current_col_sqlite, nb_column_view_sqlite;

typedef struct
{
  HANDLE h_sqlite_ed;             //sqlite HANDLE

  HANDLE h_cname[MAX_LINE_SIZE];
  HANDLE h_ctext[MAX_LINE_SIZE];
  HANDLE h_exctext[MAX_LINE_SIZE];

  char table_name[MAX_LINE_SIZE];
  DWORD pos;
  DWORD nb_column;
}HSQLITE;
HSQLITE ssqlite;

typedef struct
{
  HANDLE h_sqlite_ed_table;             //sqlite HANDLE

  HANDLE h_cname[MAX_LINE_SIZE];
  HANDLE h_tcname[MAX_LINE_SIZE];
  HANDLE h_type[MAX_LINE_SIZE];
  HANDLE h_pkey[MAX_LINE_SIZE];
  HANDLE h_bremove[MAX_LINE_SIZE];

  char table_name[MAX_LINE_SIZE];
  DWORD pos;
  DWORD nb_items;

  BOOL disable[MAX_LINE_SIZE];
}HSQLITEE;
HSQLITEE ssqlitee;
#define BREMOVEINDEX            20000
#define BREMOVEMAXINDEX         40000

//------------------------------------------------------------------------------
int callback_sqlite_sqlite_ed(void *datas, int argc, char **argv, char **azColName)
{
  FORMAT_CALBAK_TYPE *type = datas;
  switch(type->type)
  {
    //----------------------------------------
    case TYPE_SQLITE_LOAD_TABLES:SendDlgItemMessage(h_sqlite_ed,DLG_SQL_ED_LB_TABLE,LB_ADDSTRING,0,(LPARAM)argv[0]);break;
    //----------------------------------------
    case TYPE_SQLITE_LOAD_TABLE_SELECTED:
    case TYPE_SQLITE_REQUEST:
    {
      unsigned short i;
      LVITEM lvi;
      lvi.mask = LVIF_TEXT|LVIF_PARAM;
      lvi.iSubItem = 0;
      lvi.lParam = LVM_SORTITEMS;
      lvi.pszText="";
      lvi.iItem = SendDlgItemMessage(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE,LVM_GETITEMCOUNT,0,0);

      if (lvi.iItem == 0) //first item, add headers
      {
        //size
        nb_current_col_sqlite = argc;
        RECT Rect;
        GetWindowRect(GetDlgItem(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE), &Rect);
        DWORD col_sz = ((Rect.right-Rect.left)/argc) -20;

        //create column
        LVCOLUMN lvc;
        lvc.mask                = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
        lvc.fmt                 = LVCFMT_LEFT;
        lvc.cx                  = col_sz;

        //add text header
        for (i=0;i<argc;i++)
        {
          lvc.pszText = azColName[i];
          SendDlgItemMessage(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE,LVM_INSERTCOLUMN,(WPARAM)i, (LPARAM)&lvc);
        }
      }

      //add item
      DWORD ref_item = ListView_InsertItem(GetDlgItem(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE), &lvi);

      for (i=0;i<argc;i++)
      {
        ListView_SetItemText(GetDlgItem(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE),ref_item,i,argv[i]);
      }
    }
    break;
    case TYPE_SQLITE_LOAD_TABLE_HEADER:
    {
      //create column
      LVCOLUMN lvc;
      lvc.mask                = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT;
      lvc.fmt                 = LVCFMT_LEFT;
      lvc.cx                  = 100;
      lvc.pszText             = argv[1];
      SendDlgItemMessage(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE,LVM_INSERTCOLUMN,(WPARAM)nb_column_view_sqlite++, (LPARAM)&lvc);
    }
    break;
    case TYPE_SQLITE_LOAD_TABLE_STRUCT: //get for edit table format !
    {
      //add first items !
      //create items + set text
      ssqlitee.disable[ssqlitee.nb_items]  = 0;
      ssqlitee.h_cname[ssqlitee.nb_items] = CreateWindow("Edit", argv[1],0x50810080|WS_TABSTOP,10,ssqlitee.pos,200,22,ssqlitee.h_sqlite_ed_table,(HMENU)NULL, hinst, NULL);
      ssqlitee.h_tcname[ssqlitee.nb_items] = CreateWindow("Edit", argv[1],0x50810880|WS_TABSTOP,10,ssqlitee.pos,100,22,ssqlitee.h_sqlite_ed_table,(HMENU)NULL, hinst, NULL);

      ssqlitee.h_type[ssqlitee.nb_items]  = CreateWindow("Combobox", argv[2],0x50210003,220,ssqlitee.pos,90,200,ssqlitee.h_sqlite_ed_table,(HMENU)NULL, hinst, NULL);
      ssqlitee.h_pkey[ssqlitee.nb_items]  = CreateWindow("Button","Primary Key", 0x50010003,320,ssqlitee.pos,100,22,ssqlitee.h_sqlite_ed_table,(HMENU)NULL, hinst, NULL);
      ssqlitee.h_bremove[ssqlitee.nb_items]  = CreateWindow("Button","Remove", 0x50010000,430,ssqlitee.pos,70,22,ssqlitee.h_sqlite_ed_table,(HMENU)(BREMOVEINDEX+ssqlitee.nb_items), hinst, NULL);

      if (argv[5][0] != '0') SendMessage(ssqlitee.h_pkey[ssqlitee.nb_items], BM_SETCHECK, BST_CHECKED,1);

      SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_INSERTSTRING,(WPARAM)-1, (LPARAM)"TEXT");
      SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_INSERTSTRING,(WPARAM)-1, (LPARAM)"NUMERIC");
      SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_INSERTSTRING,(WPARAM)-1, (LPARAM)"INTEGER");
      SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_INSERTSTRING,(WPARAM)-1, (LPARAM)"REAL");
      SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_INSERTSTRING,(WPARAM)-1, (LPARAM)"BLOB");
      SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_INSERTSTRING,(WPARAM)-1, (LPARAM)"NONE");

      switch (argv[2][0])
      {
        case 'T':SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_SETCURSEL,0,0);break;
        case 'N':
          if (argv[2][1] == 'U')SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_SETCURSEL,1,0);
          else SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_SETCURSEL,5,0);
        break;
        case 'I':SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_SETCURSEL,2,0);break;
        case 'R':SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_SETCURSEL,3,0);break;
        case 'B':SendMessage(ssqlitee.h_type[ssqlitee.nb_items], CB_SETCURSEL,4,0);break;
      }
      ssqlitee.pos+=30;

      //next
      ssqlitee.nb_items++;
    }
    break;
  }
  return 0;
}
//------------------------------------------------------------------------------
/*BOOL stringIsANumber(char *s)
{
  char *c = s;
  while (*c && *c > 57 && *c < 48)c++;
  if (*c != 0) return FALSE;

  return TRUE;
}*/
//------------------------------------------------------------------------------
void MoveAllComponentLineS(unsigned int pos)
{
  if (pos < 0) pos = 0;
  else if (pos > ssqlite.nb_column-1) pos = ssqlite.nb_column-1;

  DWORD i,j=0;
  unsigned int pos_max = NB_VIEW_COMPONENT_SQLITE_ED;
  for (i =0; i<pos; i++) //disable
  {
    ShowWindow(ssqlite.h_cname[i],SW_HIDE);
    ShowWindow(ssqlite.h_ctext[i],SW_HIDE);
    ShowWindow(ssqlite.h_exctext[i],SW_HIDE);
  }

  for (i = pos; i<pos+pos_max; i++,j++)
  {
    //move
    SetWindowPos(ssqlite.h_cname[i]  ,HWND_TOP,10  ,60+j*30,0,0,SWP_NOSIZE);
    SetWindowPos(ssqlite.h_ctext[i]  ,HWND_TOP,200 ,60+j*30,0,0,SWP_NOSIZE);
    SetWindowPos(ssqlite.h_exctext[i],HWND_TOP,410 ,60+j*30,0,0,SWP_NOSIZE);

    //view
    ShowWindow(ssqlite.h_cname[i],SW_SHOW);
    ShowWindow(ssqlite.h_ctext[i],SW_SHOW);
    ShowWindow(ssqlite.h_exctext[i],SW_SHOW);

    //update
    UpdateWindow(ssqlite.h_cname[i]);
    UpdateWindow(ssqlite.h_ctext[i]);
    UpdateWindow(ssqlite.h_exctext[i]);
  }

  for (i =pos+pos_max; i<ssqlite.nb_column; i++) //disable
  {
    ShowWindow(ssqlite.h_cname[i],SW_HIDE);
    ShowWindow(ssqlite.h_ctext[i],SW_HIDE);
    ShowWindow(ssqlite.h_exctext[i],SW_HIDE);
  }
}

//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_sqlite_ed_req(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            case DLG_SQL_ED_ED_REQUEST: //update
              {
                char request[MAX_LINE_SIZE]="";
                char table[MAX_LINE_SIZE]="";
                char params[MAX_LINE_SIZE]="";
                char conditions[MAX_LINE_SIZE]="";

                char tparam[MAX_LINE_SIZE]="";
                char tvalue[MAX_LINE_SIZE]="";
                char tcondition[MAX_LINE_SIZE]="";

                //get all datas
                int i;
                for (i =0; i < ssqlite.nb_column;i++)
                {
                  tparam[0] = 0;
                  GetWindowText(ssqlite.h_cname[i],tparam,MAX_LINE_SIZE);

                  tvalue[0] = 0;
                  GetWindowText(ssqlite.h_ctext[i],tvalue,MAX_LINE_SIZE);

                  tcondition[0] = 0;
                  GetWindowText(ssqlite.h_exctext[i],tcondition,MAX_LINE_SIZE);

                  //string standard
                  strncat(params,tparam,MAX_LINE_SIZE-strlen(params));
                  strncat(params," = \"\0",MAX_LINE_SIZE-strlen(params));
                  strncat(params,tvalue,MAX_LINE_SIZE-strlen(params));
                  strncat(params,"\",\0",MAX_LINE_SIZE-strlen(params));

                  strncat(conditions,tparam,MAX_LINE_SIZE-strlen(conditions));
                  strncat(conditions,"= \"\0",MAX_LINE_SIZE-strlen(conditions));
                  strncat(conditions,tcondition,MAX_LINE_SIZE-strlen(conditions));
                  strncat(conditions,"\" AND \0",MAX_LINE_SIZE-strlen(conditions));
                }

                //remove last , and AND ...
                params[strlen(params)-1] = 0;
                conditions[strlen(conditions)-5] = 0;

                //create request !
                GetWindowText(hwnd,table,MAX_LINE_SIZE);
                snprintf(request,MAX_LINE_SIZE, "UPDATE %s SET %s WHERE %s;",table, params, conditions);

                //MessageBox(NULL,request,request,MB_OK|MB_TOPMOST);
                //run request
                char *error_msg = 0;
                if(sqlite3_exec(db_sqlite_test,request, NULL, NULL, &error_msg)!= SQLITE_OK)
                {
                  snprintf(request,MAX_PATH,"Error: %s",error_msg);
                  SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)request);
                  sqlite3_free(error_msg);
                }else SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"OK");
              }
            break;
            case DLG_SQL_ED_BT_SEND: //add
            case DLG_SQL_ED_BT_INFO: //info
             {
                char request[MAX_LINE_SIZE]="";
                char table[MAX_LINE_SIZE]="";
                char params[MAX_LINE_SIZE]="";

                char tvalue[MAX_LINE_SIZE]="";

                //get all datas
                int i;
                for (i =0; i < ssqlite.nb_column;i++)
                {
                  tvalue[0] = 0;
                  GetWindowText(ssqlite.h_ctext[i],tvalue,MAX_LINE_SIZE);

                  //string standard
                  strncat(params,"\"\0",MAX_LINE_SIZE-strlen(params));
                  strncat(params,tvalue,MAX_LINE_SIZE-strlen(params));
                  strncat(params,"\",\0",MAX_LINE_SIZE-strlen(params));
                }

                //remove last , and AND ...
                params[strlen(params)-1] = 0;

                //create request !
                GetWindowText(hwnd,table,MAX_LINE_SIZE);
                snprintf(request,MAX_LINE_SIZE, "INSERT INTO %s VALUES (%s);",table, params);

                if (LOWORD(wParam) == DLG_SQL_ED_BT_INFO)
                {
                  MessageBox(NULL,request,"REQUEST INFORMATION",MB_OK|MB_TOPMOST);
                  //copy to clipboard
                  CopyStringToClipboard(request);
                  break;
                }

                //run request
                char *error_msg = 0;
                if(sqlite3_exec(db_sqlite_test,request, NULL, NULL, &error_msg)!= SQLITE_OK)
                {
                  snprintf(request,MAX_PATH,"Error: %s",error_msg);
                  SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)request);
                  sqlite3_free(error_msg);
                }else SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"OK");
              }
            break;
          }
        break;
      }
    break;
    case WM_VSCROLL:
    {
      unsigned int pos = GetScrollPos(hwnd, SB_VERT);
      switch (LOWORD(wParam))
      {
        case SB_TOP:            pos = 0;break;
        case SB_BOTTOM:         pos = ssqlite.nb_column-1;break;
        case SB_LINEUP:         pos--;break;
        case SB_LINEDOWN:       pos++;break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:  pos = HIWORD(wParam);break;
        case SB_PAGEDOWN:       pos+=NB_VIEW_COMPONENT_SQLITE_ED;break;
        case SB_PAGEUP:         pos-=NB_VIEW_COMPONENT_SQLITE_ED;break;
        case SB_ENDSCROLL:break;
      }
      if (pos < 0) pos = 0;
      if (pos > ssqlite.nb_column-1) pos = ssqlite.nb_column-1;

      SetScrollPos(hwnd, SB_VERT, pos, TRUE);
      MoveAllComponentLineS(pos);
    }
    break;
    case WM_CLOSE :
      //init struct
      while (ssqlite.nb_column--)
      {
        ssqlite.h_cname[ssqlite.nb_column] = 0;
        ssqlite.h_ctext[ssqlite.nb_column] = 0;
        ssqlite.h_exctext[ssqlite.nb_column] = 0;
      }

      ssqlite.nb_column           = 0;
      ssqlite.h_sqlite_ed         = 0;
      ssqlite.table_name[0]       = 0;

      //close
      DestroyWindow(hwnd);
      break;
  }
  return 0;
}
//------------------------------------------------------------------------------
void MoveAllComponentLine(unsigned int pos)
{
  if (pos < 0) pos = 0;
  else if (pos > ssqlitee.nb_items-1) pos = ssqlitee.nb_items-1;

  DWORD i,j=0;
  unsigned int pos_max = NB_VIEW_COMPONENT_SQLITE_ED;
  for (i =0; i<pos; i++) //disable
  {
    ShowWindow(ssqlitee.h_cname[i],SW_HIDE);
    ShowWindow(ssqlitee.h_tcname[i],SW_HIDE);
    ShowWindow(ssqlitee.h_type[i],SW_HIDE);
    ShowWindow(ssqlitee.h_pkey[i],SW_HIDE);
    ShowWindow(ssqlitee.h_bremove[i],SW_HIDE);
  }

  for (i = pos; i<pos+pos_max; i++,j++)
  {
    //move
    SetWindowPos(ssqlitee.h_cname[i]  ,HWND_TOP,10  ,60+j*30,0,0,SWP_NOSIZE);
    SetWindowPos(ssqlitee.h_tcname[i] ,HWND_TOP,220 ,60+j*30,0,0,SWP_NOSIZE);
    SetWindowPos(ssqlitee.h_type[i]   ,HWND_TOP,330 ,60+j*30,0,0,SWP_NOSIZE);
    SetWindowPos(ssqlitee.h_pkey[i]   ,HWND_TOP,440 ,60+j*30,0,0,SWP_NOSIZE);
    SetWindowPos(ssqlitee.h_bremove[i],HWND_TOP,550 ,60+j*30,0,0,SWP_NOSIZE);

    //view
    if (ssqlitee.disable[i] == FALSE)
    {
      ShowWindow(ssqlitee.h_cname[i],SW_SHOW);
      ShowWindow(ssqlitee.h_tcname[i],SW_SHOW);
      ShowWindow(ssqlitee.h_type[i],SW_SHOW);
      ShowWindow(ssqlitee.h_pkey[i],SW_SHOW);
      ShowWindow(ssqlitee.h_bremove[i],SW_SHOW);
    }else
    {
      ShowWindow(ssqlitee.h_cname[i],SW_HIDE);
      ShowWindow(ssqlitee.h_tcname[i],SW_HIDE);
      ShowWindow(ssqlitee.h_type[i],SW_HIDE);
      ShowWindow(ssqlitee.h_pkey[i],SW_HIDE);
      ShowWindow(ssqlitee.h_bremove[i],SW_HIDE);
    }

    //update
    UpdateWindow(ssqlitee.h_cname[i]);
    UpdateWindow(ssqlitee.h_tcname[i]);
    UpdateWindow(ssqlitee.h_type[i]);
    UpdateWindow(ssqlitee.h_pkey[i]);
    UpdateWindow(ssqlitee.h_bremove[i]);
  }

  for (i =pos+pos_max; i<ssqlitee.nb_items; i++) //disable
  {
    ShowWindow(ssqlitee.h_cname[i],SW_HIDE);
    ShowWindow(ssqlitee.h_tcname[i],SW_HIDE);
    ShowWindow(ssqlitee.h_type[i],SW_HIDE);
    ShowWindow(ssqlitee.h_pkey[i],SW_HIDE);
    ShowWindow(ssqlitee.h_bremove[i],SW_HIDE);
  }
}
//------------------------------------------------------------------------------
char* GetCurrentTableChamps(int id_exclude, char *champs_only, char *champs_full, DWORD sz_max)
{
  DWORD i;
  char tmp_name[MAX_LINE_SIZE], tmp_type[MAX_PATH];
  BOOL PK = FALSE;

  champs_only[0] = 0;
  champs_full[0] = 0;

  for (i=0;i<ssqlitee.nb_items;i++)
  {
    if (i != id_exclude && !ssqlitee.disable[i])//if already deleted
    {
      //get
      GetWindowText(ssqlitee.h_tcname[i],tmp_name,MAX_LINE_SIZE);                     //name
      GetWindowText(ssqlitee.h_type[i],tmp_type,MAX_PATH);                            //type
      if (Button_GetCheck(ssqlitee.h_pkey) == BST_CHECKED)PK = TRUE; else PK = FALSE; //PRIMARY KEY + NOT NULL

      //header only
      strncat(champs_only,"\"\0",sz_max-strlen(champs_only));
      strncat(champs_only,tmp_name,sz_max-strlen(champs_only));
      if (i == ssqlitee.nb_items-1)strncat(champs_only,"\"\0",sz_max-strlen(champs_only));
      else strncat(champs_only,"\",\0",sz_max-strlen(champs_only));

      //ALL
      GetWindowText(ssqlitee.h_cname[i],tmp_name,MAX_LINE_SIZE);                     //name
      strncat(champs_full,"\"\0",sz_max-strlen(champs_full));
      strncat(champs_full,tmp_name,sz_max-strlen(champs_full));
      strncat(champs_full,"\" \0",sz_max-strlen(champs_full));
      strncat(champs_full,tmp_type,sz_max-strlen(champs_full));
      if(PK)
      {
        strncat(champs_full," NOT NULL PRIMARY KEY\0",sz_max);
      }
      if (i == ssqlitee.nb_items-1)strncat(champs_full,"\0",sz_max);
      else strncat(champs_full,",\0",sz_max);
    }
  }

  if (champs_full[strlen(champs_full)-1] == ',')champs_full[strlen(champs_full)-1] = 0;
  if (champs_only[strlen(champs_only)-1] == ',')champs_only[strlen(champs_only)-1] = 0;

  return champs_full;
}
//------------------------------------------------------------------------------
void ReloadTable(HWND hwnd)
{
  //delete columns
  while (ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),1));
  ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),0);

  //init LV
  TRI_SQLITE_ED = FALSE;
  SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"");
  SendDlgItemMessage(hwnd,DLG_SQL_ED_LV_RESPONSE,LVM_DELETEALLITEMS,0, (LPARAM)"");

  //lB
  SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_RESETCONTENT,0, (LPARAM)"");

  //reload
  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = TYPE_SQLITE_LOAD_TABLES;
  sqlite3_exec(db_sqlite_test,"SELECT DISTINCT tbl_name FROM sqlite_master ORDER BY tbl_name;", callback_sqlite_sqlite_ed, &fcri, NULL);
}
//------------------------------------------------------------------------------
BOOL ModifyTableStruct(int index, BOOL exclude_index)
{
  char tmp_request[REQUEST_MAX_SIZE];

  //rename current database !
  snprintf(tmp_request, REQUEST_MAX_SIZE,"ALTER TABLE \"main\".\"%s\" RENAME TO \"oXHFcGcd04oXHFcGcd04_%s\";",ssqlitee.table_name,ssqlitee.table_name);
  if(!sqlite3_exec(db_sqlite_test,tmp_request,NULL,NULL,NULL))
  {
    //create new database
    char champs_only[MAX_LINE_SIZE],champs_full[MAX_LINE_SIZE];
    GetCurrentTableChamps(exclude_index?index:-1, champs_only, champs_full, MAX_LINE_SIZE);
    snprintf(tmp_request, REQUEST_MAX_SIZE,"CREATE TABLE \"main\".\"%s\" (%s);",ssqlitee.table_name,champs_full);
    if(!sqlite3_exec(db_sqlite_test,tmp_request,NULL,NULL,NULL))
    {
      //insert DATAS
      snprintf(tmp_request, REQUEST_MAX_SIZE,"INSERT INTO \"main\".\"%s\" SELECT %s FROM \"main\".\"oXHFcGcd04oXHFcGcd04_%s\";",ssqlitee.table_name,champs_only,ssqlitee.table_name);
      if(!sqlite3_exec(db_sqlite_test,tmp_request,NULL,NULL,NULL))
      {
        //DROP temporary databse
        snprintf(tmp_request, REQUEST_MAX_SIZE,"DROP TABLE \"main\".\"oXHFcGcd04oXHFcGcd04_%s\";",ssqlitee.table_name);
        sqlite3_exec(db_sqlite_test,tmp_request,NULL,NULL,NULL);

        //disable component
        ssqlitee.disable[index] = TRUE;
        ShowWindow(ssqlitee.h_cname[index],SW_HIDE);
        ShowWindow(ssqlitee.h_tcname[index],SW_HIDE);
        ShowWindow(ssqlitee.h_pkey[index],SW_HIDE);
        ShowWindow(ssqlitee.h_type[index],SW_HIDE);
        ShowWindow(ssqlitee.h_bremove[index],SW_HIDE);

        //reload table list !
        ReloadTable(h_sqlite_ed);
        return TRUE;
      }
    }
  }
  return FALSE;
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_sqlite_ed_table_req(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)
  {
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          if (LOWORD(wParam)>=BREMOVEINDEX && LOWORD(wParam) < BREMOVEMAXINDEX)
          {
            if (!ModifyTableStruct(LOWORD(wParam)-BREMOVEINDEX, TRUE))
            {
              //error msg
              SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"SQLITE MSG : ERROR REMOVE VAR FORMAT ERROR");
            }else SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"SQLITE MSG : OK");
          }else
          {
            switch(LOWORD(wParam))
            {
              case DLG_SQL_ED_ED_REQUEST: //update
                {
                  if (!ModifyTableStruct(LOWORD(wParam)-BREMOVEINDEX, FALSE))
                  {
                    //error msg
                    SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"SQLITE MSG : ERROR ALTER TABLE FORMAT ERROR");
                  }else SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"SQLITE MSG : OK");
                }
              break;
            }
          }
        break;
      }
    break;
    case WM_VSCROLL:
    {
      unsigned int pos = GetScrollPos(hwnd, SB_VERT);
      switch (LOWORD(wParam))
      {
        case SB_TOP:            pos = 0;break;
        case SB_BOTTOM:         pos = ssqlitee.nb_items-1;break;
        case SB_LINEUP:         pos--;break;
        case SB_LINEDOWN:       pos++;break;
        case SB_THUMBTRACK:
        case SB_THUMBPOSITION:  pos = HIWORD(wParam);break;
        case SB_PAGEDOWN:       pos+=NB_VIEW_COMPONENT_SQLITE_ED;break;
        case SB_PAGEUP:         pos-=NB_VIEW_COMPONENT_SQLITE_ED;break;
        case SB_ENDSCROLL:break;
      }
      if (pos < 0) pos = 0;
      else if (pos > ssqlitee.nb_items-1) pos = ssqlitee.nb_items-1;

      SetScrollPos(hwnd, SB_VERT, pos, TRUE);
      MoveAllComponentLine(pos);
    }
    break;
    case WM_CLOSE :
      //init struct
      while (ssqlitee.nb_items--)
      {
        ssqlitee.h_cname[ssqlite.nb_column]    = 0;
        ssqlitee.h_tcname[ssqlite.nb_column]   = 0;
        ssqlitee.h_type[ssqlite.nb_column]     = 0;
        ssqlitee.h_pkey[ssqlite.nb_column]     = 0;
        ssqlitee.h_bremove[ssqlite.nb_column]  = 0;
        ssqlitee.disable[ssqlite.nb_column]    = 0;
      }

      ssqlitee.nb_items           = 0;
      ssqlitee.h_sqlite_ed_table  = 0;
      ssqlitee.table_name[0]      = 0;

      //init struct
      ssqlitee.h_sqlite_ed_table   = 0;
      ssqlitee.pos = 60;

      //close
      DestroyWindow(hwnd);
    break;
  }
  return 0;
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_sqlite_ed(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            case DLG_SQL_ED_BT_SEARCH:
            {
              char tmp[MAX_PATH];
              HANDLE hlstv = GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE);

              //select lstv
              SetFocus(hlstv);
              SendDlgItemMessage(hwnd, DLG_SQL_ED_ED_SEARCH,WM_GETTEXT ,(WPARAM)MAX_PATH, (LPARAM)tmp);

              if (GetMenuState(GetMenu(h_main),BT_SEARCH_MATCH_CASE,MF_BYCOMMAND) == MF_CHECKED)pos_search_sqlite = LVSearch(hlstv, nb_current_col_sqlite, tmp, pos_search_sqlite);
              else pos_search_sqlite = LVSearchNoCass(hlstv, nb_current_col_sqlite, tmp, pos_search_sqlite);
            }
            break;
            case POPUP_A_SEARCH:
            {
              char tmp[MAX_PATH];
              SendDlgItemMessage(hwnd,DLG_SQL_ED_ED_SEARCH,WM_GETTEXT ,(WPARAM)MAX_PATH, (LPARAM)tmp);
              LVAllSearch(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE), nb_current_col_sqlite, tmp);
            }
            break;
            case DLG_SQL_ED_BT_LOAD:
            {
              OPENFILENAME ofn;
              char file[MAX_PATH]="";
              ZeroMemory(&ofn, sizeof(OPENFILENAME));
              ofn.lStructSize = sizeof(OPENFILENAME);
              ofn.hwndOwner = h_sqlite_ed;
              ofn.lpstrFile = file;
              ofn.nMaxFile = MAX_PATH;
              ofn.lpstrFilter ="SQLITE (*.sqlite)\0*.sqlite\0SQLITE (*.db)\0*.db\0ALL (*.*)\0*.*\0";
              ofn.nFilterIndex = 3;
              ofn.Flags =OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST| OFN_ALLOWMULTISELECT | OFN_EXPLORER;
              ofn.lpstrDefExt ="sqlite\0";

              if (GetOpenFileName(&ofn)==TRUE)
              {
                //open file
                sqlite3_open(file, &db_sqlite_test);
                //clean
                SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_RESETCONTENT,0,0);
                //load list of tables
                FORMAT_CALBAK_READ_INFO fcri;
                fcri.type = TYPE_SQLITE_LOAD_TABLES;
                sqlite3_exec(db_sqlite_test,"SELECT DISTINCT tbl_name FROM sqlite_master ORDER BY tbl_name;", callback_sqlite_sqlite_ed, &fcri, NULL);
                SetWindowText(hwnd,file);
              }
            }
            break;
            case DLG_SQL_ED_BT_CLOSE:
              sqlite3_close(db_sqlite_test);
              db_sqlite_test = NULL;

              //delete columns
              while (ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),1));
              ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),0);

              //init LV
              TRI_SQLITE_ED = FALSE;
              SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"");
              SendDlgItemMessage(hwnd,DLG_SQL_ED_LV_RESPONSE,LVM_DELETEALLITEMS,0, (LPARAM)"");

              //lB
              SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_RESETCONTENT,0, (LPARAM)"");
              SetWindowText(hwnd,NOM_FULL_APPLI);
            break;
            case DLG_SQL_ED_BT_SEND:
            {
              //delete columns
              while (ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),1));
              ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),0);

              //init LV
              TRI_SQLITE_ED = FALSE;
              SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"");
              SendDlgItemMessage(hwnd,DLG_SQL_ED_LV_RESPONSE,LVM_DELETEALLITEMS,0, (LPARAM)"");

              char request[MAX_PATH]="";
              GetWindowText(GetDlgItem(hwnd,DLG_SQL_ED_ED_REQUEST),request,MAX_PATH);

              //request
              FORMAT_CALBAK_READ_INFO fcri;
              fcri.type = TYPE_SQLITE_REQUEST;
              char *error_msg = 0;
              if (sqlite3_exec(db_sqlite_test,request, callback_sqlite_sqlite_ed, &fcri, &error_msg)!= SQLITE_OK)
              {
                snprintf(request,MAX_PATH,"Error: %s",error_msg);
                SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)request);
                sqlite3_free(error_msg);
              }else
              {
                //set number of items
                snprintf(request,MAX_PATH,"Item(s) : %lu",SendDlgItemMessage(hwnd,DLG_SQL_ED_LV_RESPONSE,LVM_GETITEMCOUNT,0,0));
                SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)request);
              }
            }
            break;

            case POPUP_SQLITE_REQUEST_SELECT:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"SELECT * FROM table;");break;
            case POPUP_SQLITE_REQUEST_INSERT:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"INSERT INTO table (param1,param2) VALUES(\"data1\",\"data2\");");break;
            case POPUP_SQLITE_REQUEST_DELETE:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"DELETE FROM table WHERE value=\"1\";");break;

            case POPUP_SQLITE_REQUEST_ALTER_TABLE: SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"ALTER TABLE \"main\".\"DbMeta\" RENAME TO \"DbMeta2\";");break;
            case POPUP_SQLITE_REQUEST_CREATE_TABLE:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"CREATE TABLE \"main\".\"DbMeta\" (\"value\" TEXT);");break;
            case POPUP_SQLITE_REQUEST_INSERT_INTO: SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"INSERT INTO \"main\".\"DbMeta\" SELECT \"value\" FROM \"main\".\"DbMeta2\";");break;
            case POPUP_SQLITE_REQUEST_DROP_TABLE:  SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"CDROP TABLE \"main\".\"DbMeta\";");break;

            case POPUP_SQLITE_REQUEST_VACCUM:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"VACUUM;");break;
            case POPUP_SQLITE_REQUEST_JOURNAL_ON:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"PRAGMA journal_mode = ON;");break;
            case POPUP_SQLITE_REQUEST_JOURNAL_OFF:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"PRAGMA journal_mode = OFF;");break;
            case POPUP_SQLITE_REQUEST_BEGIN_TRANSACTION:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"BEGIN TRANSACTION;");break;
            case POPUP_SQLITE_REQUEST_END_TRANSACTION:SetWindowText(GetDlgItem((HWND)hwnd,DLG_SQL_ED_ED_REQUEST),"END TRANSACTION;");break;

            case POPUP_SQLITE_INFOS:
              {
                HANDLE hlstv = GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE);
                long index = SendMessage(hlstv,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
                int i;
                if (index > -1)
                {
                  char tmp[MAX_LINE_SIZE+1], tmp2[MAX_LINE_SIZE+1];
                  RichEditInit(GetDlgItem(h_info,DLG_INFO_TXT));
                  LVCOLUMN lvc;
                  lvc.mask        = LVCF_TEXT;
                  lvc.cchTextMax  = MAX_LINE_SIZE;
                  lvc.pszText     = tmp;

                  for (i=0;i<nb_current_col_sqlite;i++)
                  {
                    tmp[0] = 0;
                    tmp2[0] = 0;
                    if (SendMessage(hlstv,LVM_GETCOLUMN,(WPARAM)i,(LPARAM)&lvc) != 0)
                    {
                      if (*tmp !='\0')
                      {
                        ListView_GetItemText(hlstv,index,i,tmp2,MAX_LINE_SIZE);

                        if (*tmp2 !='\0')
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
            case POPUP_SQLITE_REMOVE:
              {
                HANDLE hlstv = GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE);
                long index = SendMessage(hlstv,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
                if (index > -1)
                {
                  char request[MAX_LINE_SIZE]="";
                  char table[MAX_LINE_SIZE]="";
                  char conditions[MAX_LINE_SIZE]="";

                  char tparam[MAX_LINE_SIZE]="";
                  char tcondition[MAX_LINE_SIZE]="";

                  int i;
                  LVCOLUMN lvc;
                  lvc.mask        = LVCF_TEXT;
                  lvc.cchTextMax  = MAX_LINE_SIZE;
                  lvc.pszText     = tparam;

                  //create request !
                  DWORD id_item = SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETCURSEL,0,0);
                  if (SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXTLEN,id_item,0) <= MAX_LINE_SIZE)
                  {
                    SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXT,id_item,(LPARAM)table);
                  }

                  long int nb_items = ListView_GetItemCount(hlstv)-1;
                  for (index = nb_items; index >= 0; index--)
                  {
                    //init
                    if (SendMessage(hlstv,LVM_GETITEMSTATE,(WPARAM)index,(LPARAM)LVIS_SELECTED) != LVIS_SELECTED)continue;

                    request[0] = 0;
                    conditions[0] = 0;

                    //get all datas
                    for (i =0; i < nb_current_col_sqlite;i++)
                    {
                      tparam[0] = 0;
                      tcondition[0] = 0;
                      if (SendMessage(hlstv,LVM_GETCOLUMN,(WPARAM)i,(LPARAM)&lvc) != 0)
                      {
                        if (*tparam !='\0')
                        {
                          ListView_GetItemText(hlstv,index,i,tcondition,MAX_LINE_SIZE);
                          if (*tcondition !='\0')
                          {
                            strncat(conditions,tparam,MAX_LINE_SIZE-strlen(conditions));
                            strncat(conditions,"= \"\0",MAX_LINE_SIZE-strlen(conditions));
                            strncat(conditions,tcondition,MAX_LINE_SIZE-strlen(conditions));
                            strncat(conditions,"\" AND \0",MAX_LINE_SIZE-strlen(conditions));
                          }
                        }
                      }
                    }

                    //remove last AND ...
                    conditions[strlen(conditions)-5] = 0;

                    snprintf(request,MAX_LINE_SIZE, "DELETE FROM %s WHERE %s;",table, conditions);

                    //run request
                    char *error_msg = 0;
                    if(sqlite3_exec(db_sqlite_test,request, NULL, NULL, &error_msg)!= SQLITE_OK)
                    {
                      snprintf(request,MAX_PATH,"Error: %s",error_msg);
                      SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)request);
                      sqlite3_free(error_msg);
                    }else
                    {
                      SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"OK");
                      //remove item !
                      ListView_DeleteItem(hlstv,index);
                    }
                  }
                }
              }
            break;
            case POPUP_SQLITE_ED_TABLE:
            {
              ssqlitee.pos = 60;
              ssqlitee.h_sqlite_ed_table  = CreateDialog(0, MAKEINTRESOURCE(DLG_SQLITE_ED_TABLE) ,NULL,DialogProc_sqlite_ed_table_req);
              SendMessage(ssqlitee.h_sqlite_ed_table, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hinst, MAKEINTRESOURCE(ICON_APP)));
              ShowWindow(ssqlitee.h_sqlite_ed_table,SW_SHOW);

              DWORD id_item = SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETCURSEL,0,0);
              if (SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXTLEN,id_item,0) <= MAX_LINE_SIZE)
              {
                SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXT,id_item,(LPARAM)ssqlitee.table_name);
                SetWindowText(ssqlitee.h_sqlite_ed_table,ssqlitee.table_name);
              }

              //sqlite get table struct !
              char request[MAX_PATH];
              snprintf(request,MAX_PATH,"PRAGMA table_info (%s);",ssqlitee.table_name);
              FORMAT_CALBAK_READ_INFO fcri2;
              fcri2.type = TYPE_SQLITE_LOAD_TABLE_STRUCT;
              sqlite3_exec(db_sqlite_test,request, callback_sqlite_sqlite_ed, &fcri2, NULL);

              //init scroll bar !
              SetScrollRange(ssqlitee.h_sqlite_ed_table, SB_VERT, 0, ssqlitee.nb_items-1,TRUE);
              SetScrollPos(ssqlitee.h_sqlite_ed_table, SB_VERT, 0, TRUE);
              MoveAllComponentLine(0);
            }
            break;
            case POPUP_SQLITE_HDR_RESIZE:
              {
                //set auto header size
                int i = 0;
                while(ListView_SetColumnWidth(GetDlgItem(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE),i++,LVSCW_AUTOSIZE_USEHEADER));
              }
            break;

            case DLG_SQL_ED_BT_MODELS:
            {
              HMENU hmenu;
              if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_SQLITE_REQUEST)))!= NULL)
              {
                RECT Rect;
                GetWindowRect(GetDlgItem((HWND)hwnd,DLG_SQL_ED_BT_MODELS), &Rect);
                TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, Rect.left, Rect.bottom,hwnd, NULL);
                DestroyMenu(hmenu);
              }
            }
            break;
            //-----------------------------------------------------
            case POPUP_S_VIEW:
            {
              char file[MAX_PATH]="sqlite_export";
              OPENFILENAME ofn;
              ZeroMemory(&ofn, sizeof(OPENFILENAME));
              ofn.lStructSize = sizeof(OPENFILENAME);
              ofn.hwndOwner = h_sqlite_ed;
              ofn.lpstrFile = file;
              ofn.nMaxFile = MAX_PATH;
              ofn.lpstrFilter ="*.csv \0*.csv\0*.xml \0*.xml\0*.html \0*.html\0";
              ofn.nFilterIndex = 1;
              ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
              ofn.lpstrDefExt ="csv\0";
              if (GetSaveFileName(&ofn)==TRUE)
              {
                SaveLSTV(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE), file, ofn.nFilterIndex, nb_current_col_sqlite);
                SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Export done !!!");
              }
            }
            break;
            //-----------------------------------------------------
            case POPUP_S_SELECTION:
            {
              char file[MAX_PATH]="sqlite_export";
              OPENFILENAME ofn;
              ZeroMemory(&ofn, sizeof(OPENFILENAME));
              ofn.lStructSize = sizeof(OPENFILENAME);
              ofn.hwndOwner = h_sqlite_ed;
              ofn.lpstrFile = file;
              ofn.nMaxFile = MAX_PATH;
              ofn.lpstrFilter ="*.csv \0*.csv\0*.xml \0*.xml\0*.html \0*.html\0";
              ofn.nFilterIndex = 1;
              ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
              ofn.lpstrDefExt ="csv\0";
              if (GetSaveFileName(&ofn)==TRUE)
              {
                SaveLSTVSelectedItems(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE), file, ofn.nFilterIndex, nb_current_col_sqlite);
                SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Export done !!!");
              }
            }
            break;
          }
        break;
        case CBN_SELCHANGE:
          switch(LOWORD(wParam))
          {
            case DLG_SQL_ED_LB_TABLE:
            {
              //init LV
              pos_search_sqlite = 0;
              TRI_SQLITE_ED = FALSE;
              SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"");
              SendDlgItemMessage(hwnd,DLG_SQL_ED_LV_RESPONSE,LVM_DELETEALLITEMS,0, (LPARAM)"");

              //delete columns
              while (ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),1));
              ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),0);

              //get actual table
              char tmp_table[MAX_PATH]="";
              DWORD id_item = SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETCURSEL,0,0);
              if (SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXTLEN,id_item,0) <= MAX_PATH)
              {
                SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXT,id_item,(LPARAM)tmp_table);

                //request it
                char request[MAX_PATH];
                snprintf(request,MAX_PATH,"SELECT * FROM %s;",tmp_table);
                FORMAT_CALBAK_READ_INFO fcri;
                fcri.type = TYPE_SQLITE_LOAD_TABLE_SELECTED;
                sqlite3_exec(db_sqlite_test,request, callback_sqlite_sqlite_ed, &fcri, NULL);

                //if no item get headers !!!
                DWORD nb_items = SendDlgItemMessage(hwnd,DLG_SQL_ED_LV_RESPONSE,LVM_GETITEMCOUNT,0,0);
                if (nb_items < 1)
                {
                  snprintf(request,MAX_PATH,"PRAGMA table_info (%s);",tmp_table);
                  FORMAT_CALBAK_READ_INFO fcri2;
                  fcri2.type = TYPE_SQLITE_LOAD_TABLE_HEADER;
                  nb_column_view_sqlite = 0;
                  sqlite3_exec(db_sqlite_test,request, callback_sqlite_sqlite_ed, &fcri2, NULL);

                  //set auto header size
                  int i = 0;
                  while(ListView_SetColumnWidth(GetDlgItem(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE),i++,LVSCW_AUTOSIZE_USEHEADER));
                }

                //set auto header size
                int i = 0;
                while(ListView_SetColumnWidth(GetDlgItem(h_sqlite_ed,DLG_SQL_ED_LV_RESPONSE),i++,LVSCW_AUTOSIZE_USEHEADER));

                //set number of items
                snprintf(request,MAX_PATH,"Item(s) : %lu",nb_items);
                SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)request);
              }
            }
            break;
          }
        break;
      }
    break;
    case WM_DROPFILES://gestion du drag and drop de fichier ^^
    {
      char file[MAX_PATH]="";
      HDROP H_DropInfo=(HDROP)wParam;
      DWORD i=0,nb_path = DragQueryFile(H_DropInfo, 0xFFFFFFFF, file, MAX_PATH);
      //only fisrt item or fisrt good item
      for (i=0;i<nb_path;i++)
      {
        file[0] = 0;
        DragQueryFile(H_DropInfo, i, file, MAX_PATH);
        if (file[0] != 0)break;
      }

      //close if already open
      sqlite3_close(db_sqlite_test);
      db_sqlite_test = NULL;

      //delete columns
      while (ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),1));
      ListView_DeleteColumn(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE),0);


      //init LV
      TRI_SQLITE_ED = FALSE;
      SendDlgItemMessage(hwnd,DLG_SQL_ED_STATE_SB,SB_SETTEXT,0, (LPARAM)"");
      SendDlgItemMessage(hwnd,DLG_SQL_ED_LV_RESPONSE,LVM_DELETEALLITEMS,0, (LPARAM)"");

      //open file
      sqlite3_open(file, &db_sqlite_test);
      //clean
      SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_RESETCONTENT,0,0);
      //load list of tables
      FORMAT_CALBAK_READ_INFO fcri;
      fcri.type = TYPE_SQLITE_LOAD_TABLES;
      sqlite3_exec(db_sqlite_test,"SELECT DISTINCT tbl_name FROM sqlite_master ORDER BY tbl_name;", callback_sqlite_sqlite_ed, &fcri, NULL);

      DragFinish(H_DropInfo);
      SetWindowText(hwnd,file);
    }
    break;
    case WM_CLOSE :
      ShowWindow(hwnd, SW_HIDE);
      pos_search_sqlite = 0;
    break;
    case WM_SIZE:
    {
      unsigned int mWidth  = LOWORD(lParam);
      unsigned int mHeight = HIWORD(lParam);

      //controle de la taille minimum
      if (mWidth<610 ||mHeight<300)
      {
        RECT Rect;
        GetWindowRect(hwnd, &Rect);
        MoveWindow(hwnd,Rect.left,Rect.top,600+20,300+64,TRUE);
      }else
      {
        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_LB_TABLE)         ,2,0,192,mHeight-68,TRUE);
        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_BT_LOAD)          ,2,mHeight-55,95,30,TRUE);
        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_BT_CLOSE)         ,97,mHeight-55,95,30,TRUE);

        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_ED_REQUEST)       ,200,0,mWidth-245,180,TRUE);

        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_ED_SEARCH)        ,202,182,200,19,TRUE);
        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_BT_SEARCH)        ,404,182,60,19,TRUE);

        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_BT_SEND)          ,mWidth-43,0,41,160,TRUE);
        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_BT_MODELS)          ,mWidth-43,162,41,38,TRUE);

        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE)      ,200,204,mWidth-205,mHeight-230,TRUE);
        MoveWindow(GetDlgItem(hwnd,DLG_SQL_ED_STATE_SB)         ,0,mHeight-22,mWidth,22,TRUE);
      }
      InvalidateRect(hwnd, NULL, TRUE);
    }
    break;
    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code)
      {
        case LVN_COLUMNCLICK:
          switch(((LPNMHDR)lParam)->idFrom)
          {
            case DLG_SQL_ED_LV_RESPONSE:
              TRI_SQLITE_ED = !TRI_SQLITE_ED;
              c_Tri(GetDlgItem(hwnd,((LPNMHDR)lParam)->idFrom),((LPNMLISTVIEW)lParam)->iSubItem,TRI_SQLITE_ED);
            break;
          }
        break;
        case NM_DBLCLK:
          if (LOWORD(wParam) == DLG_SQL_ED_LV_RESPONSE)
          {
            //current line
            HANDLE hlstv = GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE);
            long index = SendMessage(hlstv,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
            int i;
            if (index > -1)
            {
              ssqlite.nb_column = nb_current_col_sqlite;
              if (ssqlite.nb_column > MAX_LINE_SIZE) ssqlite.nb_column = MAX_LINE_SIZE;

              ssqlite.h_sqlite_ed     = CreateDialog(0, MAKEINTRESOURCE(DLG_SQLITE_ED) ,NULL,DialogProc_sqlite_ed_req);
              SendMessage(ssqlite.h_sqlite_ed, WM_SETICON, ICON_BIG, (LPARAM)LoadIcon(hinst, MAKEINTRESOURCE(ICON_APP)));
              ShowWindow(ssqlite.h_sqlite_ed,SW_SHOW);

              DWORD id_item = SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETCURSEL,0,0);
              if (SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXTLEN,id_item,0) <= MAX_LINE_SIZE)
              {
                SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETTEXT,id_item,(LPARAM)ssqlite.table_name);
                SetWindowText(ssqlite.h_sqlite_ed,ssqlite.table_name);
              }

              //add one text + edit (memo format) by column
              char tmp[MAX_LINE_SIZE+1], tmp2[MAX_LINE_SIZE+1];
              LVCOLUMN lvc;
              lvc.mask        = LVCF_TEXT;
              lvc.cchTextMax  = MAX_LINE_SIZE;
              lvc.pszText     = tmp;

              ssqlite.pos = 60;

              for (i = 0; i<ssqlite.nb_column; i++)
              {
                //get columns
                tmp[0] = 0;
                tmp2[0] = 0;
                if (SendMessage(hlstv,LVM_GETCOLUMN,(WPARAM)i,(LPARAM)&lvc) != 0)
                {
                  if (*tmp !='\0')
                  {
                    ListView_GetItemText(hlstv,index,i,tmp2,MAX_LINE_SIZE);

                    //create items + set text
                    ssqlite.h_cname[i] = CreateWindow("Static", tmp,0x50000000,10,ssqlite.pos,150,22,ssqlite.h_sqlite_ed,(HMENU)NULL, hinst, NULL);
                    ssqlite.h_ctext[i] = CreateWindow("Edit", tmp2,0x50810080,200,ssqlite.pos,200,22,ssqlite.h_sqlite_ed,(HMENU)NULL, hinst, NULL);
                    ssqlite.h_exctext[i] = CreateWindow("Edit", tmp2,0x50810880,420,ssqlite.pos,150,22,ssqlite.h_sqlite_ed,(HMENU)NULL, hinst, NULL);

                    ssqlite.pos+=30;
                  }
                }
              }

              //init scroll bar !
              SetScrollRange(ssqlite.h_sqlite_ed, SB_VERT, 0, ssqlite.nb_column-1,TRUE);
              SetScrollPos(ssqlite.h_sqlite_ed, SB_VERT, 0, TRUE);
              MoveAllComponentLineS(0);
            }
          }
        break;
      }
    break;
    case WM_CONTEXTMENU:
      if ((HWND)wParam == GetDlgItem(hwnd,DLG_SQL_ED_LV_RESPONSE) && SendDlgItemMessage(hwnd,DLG_SQL_ED_LB_TABLE,LB_GETCURSEL,0,0) != LB_ERR)
      {
        HMENU hmenu;
        if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_LSTV_SQLITE)))!= NULL)
        {
          ModifyMenu(hmenu,POPUP_S_VIEW           ,MF_BYCOMMAND|MF_STRING ,POPUP_S_VIEW           ,cps[TXT_POPUP_S_VIEW].c);
          ModifyMenu(hmenu,POPUP_S_SELECTION      ,MF_BYCOMMAND|MF_STRING ,POPUP_S_SELECTION      ,cps[TXT_POPUP_S_SELECTION].c);

          POINT pos;
          if (GetCursorPos(&pos)!=0)
          {
            TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, pos.x, pos.y,hwnd, NULL);
          }else TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam),hwnd, NULL);
          DestroyMenu(hmenu);
        }
      }
    break;
  }
  return 0;
}
