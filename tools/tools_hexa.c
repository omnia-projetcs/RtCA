//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
#define HEXA_READER_ID_COLUMN     0
#define HEXA_READER_HEX_F_COLUMN  1
#define HEXA_READER_HEX_L_COLUMN  8
#define HEXA_READER_TXT_COLUMN    9
//------------------------------------------------------------------------------
DWORD WINAPI LoadHexaFile(LPVOID lParam)
{
  char file[MAX_PATH]="";
  BOOL inc = FALSE;
  if (lParam != 0)
  {
    snprintf(file,MAX_PATH,"%s",(char*)lParam);
    inc = TRUE;
  }else
  {
    //récupération du path
    OPENFILENAME ofn;

    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = h_hexa;
    ofn.lpstrFile = file;
    ofn.nMaxFile = MAX_PATH;
    ofn.lpstrFilter ="*.*\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.Flags =OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST | OFN_EXPLORER;
    ofn.lpstrDefExt ="*.*\0";
    if (GetOpenFileName(&ofn)==TRUE)inc = TRUE;
  }

  if (inc)
  {
    SetWindowText(GetDlgItem(h_hexa,DLG_HEXA_BT_LOAD),"Stop");

    //clean LSVs
    ListView_DeleteAllItems(GetDlgItem(h_hexa,DLG_HEXA_LV_INFOS));
    ListView_DeleteAllItems(GetDlgItem(h_hexa,DLG_HEXA_LV_HEXA));

    //get file info :)
    WIN32_FIND_DATA data;
    char tmp[MAX_PATH]="";
    HANDLE hfic = FindFirstFile(file, &data);
    if (hfic != INVALID_HANDLE_VALUE)
    {
      HANDLE hlv = GetDlgItem(h_hexa,DLG_HEXA_LV_INFOS);
      LVITEM lvi;
      lvi.mask = LVIF_TEXT|LVIF_PARAM;
      lvi.iSubItem = 0;
      lvi.lParam = LVM_SORTITEMS;
      lvi.pszText="";
      lvi.iItem = ListView_GetItemCount(hlv);
      DWORD ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"File:");
      ListView_SetItemText(hlv,ref_item,1,file);

      //size
      tmp[0] = 0;
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"Size:");
      if ((data.nFileSizeLow+data.nFileSizeHigh) > 1099511627776)snprintf(tmp,DEFAULT_TMP_SIZE,"%u%uo (%uTo)(0x%X)",(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)>>32),(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)&0xFFFFFFFF),(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)/1099511627776),(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh));
      else if (data.nFileSizeLow+data.nFileSizeHigh > 1073741824)snprintf(tmp,DEFAULT_TMP_SIZE,"%u%uo (%uGo)(0x%X)",(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)>>32),(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)&0xFFFFFFFF),(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)/1073741824),(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh));
      else if (data.nFileSizeLow+data.nFileSizeHigh > 1048576)snprintf(tmp,DEFAULT_TMP_SIZE,"%uo (%uMo)(0x%X)",(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh),(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)/1048576),(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh));
      else if (data.nFileSizeLow+data.nFileSizeHigh  > 1024)snprintf(tmp,DEFAULT_TMP_SIZE,"%uo (%uKo)(0x%X)",(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh),(unsigned int)((data.nFileSizeLow+data.nFileSizeHigh)/1024),(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh));
      else snprintf(tmp,DEFAULT_TMP_SIZE,"%uo (0x%X)",(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh),(unsigned int)(data.nFileSizeLow+data.nFileSizeHigh));
      ListView_SetItemText(hlv,ref_item,1,tmp);

      tmp[0] = 0;
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"SHA256:");
      if ((data.nFileSizeLow + data.nFileSizeHigh) < MAX_FILE_SIZE_HASH) FileToSHA256(file, tmp); else tmp[0] = 0;
      ListView_SetItemText(hlv,ref_item,1,tmp);
      tmp[0] = 0;
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"Type:");
      ReadMagicNumber(file, tmp, MAX_PATH);
      ListView_SetItemText(hlv,ref_item,1,tmp);

      tmp[0] = 0;
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"Create:");
      filetimeToString_GMT(data.ftCreationTime, tmp, DATE_SIZE_MAX);
      ListView_SetItemText(hlv,ref_item,1,tmp);
      tmp[0] = 0;
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"Last update:");
      filetimeToString_GMT(data.ftLastWriteTime, tmp, DATE_SIZE_MAX);
      ListView_SetItemText(hlv,ref_item,1,tmp);
      tmp[0] = 0;
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"Last access:");
      filetimeToString_GMT(data.ftLastAccessTime, tmp, DATE_SIZE_MAX);
      ListView_SetItemText(hlv,ref_item,1,tmp);

      //get file infos :
      char t0[MAX_PATH]="", t1[MAX_PATH]="", t2[MAX_PATH]="", t3[MAX_PATH]="";
      GetACLS(file, t0, t1, t2, t3, MAX_PATH);

      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"ACL Owner:");
      snprintf(t2,MAX_PATH,"%s (%s)",t1,t3);
      ListView_SetItemText(hlv,ref_item,1,t2);

      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"ACLs:");
      ListView_SetItemText(hlv,ref_item,1,t0);

      t0[0] = 0;
      t1[0] = 0;
      t2[0] = 0;
      t3[0] = 0;
      FileInfoRead(file, t0, t1, t2, t3, MAX_PATH);

      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"ProductName:");
      ListView_SetItemText(hlv,ref_item,1,t0);
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"FileVersion:");
      ListView_SetItemText(hlv,ref_item,1,t1);
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"CompanyName:");
      ListView_SetItemText(hlv,ref_item,1,t2);
      lvi.iItem = ListView_GetItemCount(hlv);
      ref_item = ListView_InsertItem(hlv, &lvi);
      ListView_SetItemText(hlv,ref_item,0,"FileDescription:");
      ListView_SetItemText(hlv,ref_item,1,t3);
      FindClose(hfic);
    }

    //load file and write line in lsv
    HANDLE Hfic = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0/*FILE_FLAG_SEQUENTIAL_SCAN*/,0);
    if (Hfic != INVALID_HANDLE_VALUE)
    {
      DWORD taille_fic = GetFileSize(Hfic,NULL);
      if (taille_fic != INVALID_FILE_SIZE)
      {
        char *buffer = (char *) HeapAlloc(GetProcessHeap(), 0, taille_fic+1);
        if (buffer != NULL)
        {
          DWORD i,column=HEXA_READER_HEX_F_COLUMN,ref_item=0,copiee = 0;
          ReadFile(Hfic, buffer, taille_fic,&copiee,0);

          HANDLE hlv = GetDlgItem(h_hexa,DLG_HEXA_LV_HEXA);
          LVITEM lvi;
          lvi.mask = LVIF_TEXT|LVIF_PARAM;
          lvi.iSubItem = 0;
          lvi.lParam = LVM_SORTITEMS;
          lvi.pszText="";

          //work with datas
          char buf[33],tmp[MAX_PATH];
          for (i=0;i<taille_fic && h_Hexa != NULL;i++)
          {
            //end of block
            if (i%4 == 0 && i!=0)
            {
              snprintf(tmp,9,"%02X%02X%02X%02X", buffer[i-3]&0xFF,buffer[i-2]&0xFF,buffer[i-1]&0xFF,buffer[i]&0xFF);
              ListView_SetItemText(hlv,ref_item,column++,tmp);
            }

            //end of line
            if (i%32 == 0)
            {
              column=HEXA_READER_HEX_F_COLUMN;

              //add data
              buf[32] = 0;
              ListView_SetItemText(hlv,ref_item,HEXA_READER_TXT_COLUMN,buf);
              memset(buf,0,32);

              lvi.iItem = ListView_GetItemCount(hlv);
              ref_item = ListView_InsertItem(hlv, &lvi);

              //line number
              snprintf(tmp,MAX_PATH,"0x%08X",ref_item*0x20);
              ListView_SetItemText(hlv,ref_item,HEXA_READER_ID_COLUMN,tmp);
            }

            if (buffer[i] < 127 && buffer[i] > 31 )buf[i%32] = buffer[i];
            else buf[i%32] = '.';
          }

          //end of last block
          if (i%4 != 0)
          {
            if (i%3 == 0)     snprintf(tmp,9,"%02X%02X%02X  ",buffer[i-2]&0xFF,buffer[i-1]&0xFF,buffer[i]&0xFF);
            else if (i%2 == 0)snprintf(tmp,9,"%02X%02X    ",buffer[i-1]&0xFF,buffer[i]&0xFF);
            else              snprintf(tmp,9,"%02X      ",buffer[i]&0xFF);

            ListView_SetItemText(hlv,ref_item,column,tmp);
          }

          //end of last line
          if (i%32 != 0)
          {
            //line number
            snprintf(tmp,MAX_PATH,"0x%08X",ref_item*0x20);
            ListView_SetItemText(hlv,ref_item,HEXA_READER_ID_COLUMN,tmp);

            //add data
            buf[(i%32)+1] = 0;
            ListView_SetItemText(hlv,ref_item,HEXA_READER_TXT_COLUMN,buf);
          }
          HeapFree(GetProcessHeap(), 0, buffer);
        }
      }
      CloseHandle(Hfic);
    }
    SetWindowText(h_hexa,file);
  }

  SetWindowText(GetDlgItem(h_hexa,DLG_HEXA_BT_LOAD),"Load");
  EnableWindow(GetDlgItem(h_hexa,DLG_HEXA_BT_LOAD),TRUE);

  h_Hexa = 0;
  return 0;
}
//------------------------------------------------------------------------------
DWORD Hex_search_unicode(HANDLE hlv, char *search_unicode, DWORD start_id)
{
  DWORD k,i, nb_item = ListView_GetItemCount(hlv);
  if (!nb_item) return -1;

  //prepareformat : s.t.r.i.n.g.
  char search[MAX_LINE_SIZE];
  for (i=0;i<strlen(search_unicode) && i*2+1 < MAX_LINE_SIZE;i++)
  {
    search[i*2]=search_unicode[i];
    search[i*2+1]='.';
  }
  search[i*2+1]=0;

  char tmp[MAX_LINE_SIZE], tmp2[MAX_LINE_SIZE];
  unsigned long int pos = 0;
  for (i=start_id+1;i<nb_item-1;i++)
  {
    //load last line + line + next line
    tmp[0] = 0;
    for (k=i-1;(k<nb_item-1) && (k<i+2);k++)
    {
      ListView_GetItemText(hlv,k,HEXA_READER_TXT_COLUMN,tmp2,MAX_LINE_SIZE);
      strncat(tmp,tmp2,MAX_LINE_SIZE-strlen(tmp));
      strncat(tmp,"\0",MAX_LINE_SIZE-strlen(tmp));
    }

    //test si la recherche est présente dedans
    pos = ContientNoCass(tmp,search);
    if (pos)
    {
      if (pos < 32) //first line = i-1
      {
        i = i-1;
      }if(pos < 64) //line = i
      {
        //i = i;
      }else //after line = i+1
      {
        i = i+1;
      }

      //sélection
      SendMessage(hlv, LVM_ENSUREVISIBLE, i, 0);
      ListView_SetItemState(hlv, -1, 0, LVIS_SELECTED); // deselect all
      ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);

      //on quitte
      return i;
    }
  }
  return -1;
}
//------------------------------------------------------------------------------
DWORD Hex_search_hexa(HANDLE hlv, char *search, DWORD start_id)
{
  DWORD k,i, j, nb_item = ListView_GetItemCount(hlv);
  if (!nb_item) return -1;

  printf("OK \n");

  char tmp[MAX_LINE_SIZE], tmp2[MAX_LINE_SIZE];
  unsigned long int pos = 0;
  for (i=start_id+1;i<nb_item-1;i++)
  {
    //load last line + line + next line
    tmp[0] = 0;
    for (k=i-1;(k<nb_item-1) && (k<i+2);k++)
    {
      for (j=HEXA_READER_HEX_F_COLUMN;j<=HEXA_READER_HEX_L_COLUMN;j++)
      {
        tmp2[0] = 0;
        ListView_GetItemText(hlv,k,j,tmp2,9);
        strncat(tmp,tmp2,MAX_LINE_SIZE-strlen(tmp));
        strncat(tmp,"\0",MAX_LINE_SIZE-strlen(tmp));
      }
    }

    //test si la recherche est présente dedans
    pos = ContientNoCass(tmp,search);
    if (pos)
    {
      if (pos < 64) //first line = i-1
      {
        i = i-1;
      }if(pos < 128) //line = i
      {
        //i = i;
      }else //after line = i+1
      {
        i = i+1;
      }

      //sélection
      SendMessage(hlv, LVM_ENSUREVISIBLE, i, 0);
      ListView_SetItemState(hlv, -1, 0, LVIS_SELECTED); // deselect all
      ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);

      //on quitte
      return i;
    }
  }
  return -1;
}
//------------------------------------------------------------------------------
DWORD Hex_search_ansi(HANDLE hlv, char *search, DWORD start_id)
{
  DWORD k,i, nb_item = ListView_GetItemCount(hlv);
  if (!nb_item) return -1;

  char tmp[MAX_LINE_SIZE], tmp2[MAX_LINE_SIZE];
  unsigned long int pos = 0;
  for (i=start_id+1;i<nb_item-1;i++)
  {
    //load last line + line + next line
    tmp[0] = 0;
    for (k=i-1;(k<nb_item-1) && (k<i+2);k++)
    {
      ListView_GetItemText(hlv,k,HEXA_READER_TXT_COLUMN,tmp2,MAX_LINE_SIZE);
      strncat(tmp,tmp2,MAX_LINE_SIZE-strlen(tmp));
      strncat(tmp,"\0",MAX_LINE_SIZE-strlen(tmp));
    }

    //test si la recherche est présente dedans
    pos = ContientNoCass(tmp,search);
    if (pos)
    {
      if (pos < 32) //first line = i-1
      {
        i = i-1;
      }if(pos < 64) //line = i
      {
        //i = i;
      }else //after line = i+1
      {
        i = i+1;
      }

      //sélection
      SendMessage(hlv, LVM_ENSUREVISIBLE, i, 0);
      ListView_SetItemState(hlv, -1, 0, LVIS_SELECTED); // deselect all
      ListView_SetItemState(hlv,i,LVIS_SELECTED,LVIS_SELECTED);

      //on quitte
      return i;
    }
  }
  return -1;
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_hexa(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_SIZE:
    {
      unsigned int mWidth  = LOWORD(lParam);
      unsigned int mHeight = HIWORD(lParam);

      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_BT_LOAD),4,2,92,19,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_BT_CLOSE),100,2,92,19,TRUE);

      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_LV_INFOS),4,23,(mWidth-12)/4,mHeight-9-4*21,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_EDT_SEARCH),4,mHeight-5-3*21,196,19,TRUE);

      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_CHK_UNICODE),4,mHeight-5-2*21,92,19,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_CHK_HEXA),4,mHeight-5-21,92,19,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_BT_SEARCH),100,mHeight-5-21,92,19,TRUE);

      MoveWindow(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA),(mWidth-12)/4+8,2,(mWidth-12)*3/4,mHeight-7,TRUE);
    }
    break;
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            //load file !!!
            case DLG_HEXA_BT_LOAD:
            {
              if (h_Hexa != 0)
              {
                h_Hexa = NULL;
                EnableWindow(GetDlgItem(h_hexa,DLG_HEXA_BT_LOAD),FALSE);
              }else h_Hexa = CreateThread(NULL,0,LoadHexaFile,0,0,0);
            }
            break;
            case DLG_HEXA_BT_CLOSE:
              if (h_Hexa != 0)h_Hexa = NULL;

              //clean LSVs
              ListView_DeleteAllItems(GetDlgItem(hwnd,DLG_HEXA_LV_INFOS));
              ListView_DeleteAllItems(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA));
              SetWindowText(h_hexa,NOM_FULL_APPLI);
            break;
            case DLG_HEXA_BT_SEARCH:
              if (IsDlgButtonChecked(hwnd,DLG_HEXA_CHK_UNICODE)==BST_CHECKED)
              {
                if (last_search_mode != UNICODE_MODE_SEARCH_HX)
                {
                  //reinit search
                  last_pos_hex_search   = 0;
                  last_search_mode = UNICODE_MODE_SEARCH_HX;
                }

                char tmp[DEFAULT_TMP_SIZE];
                SendDlgItemMessage(hwnd,DLG_HEXA_EDT_SEARCH,WM_GETTEXT ,(WPARAM)DEFAULT_TMP_SIZE, (LPARAM)tmp);

                if (last_pos_hex_search)last_pos_hex_search++;
                last_pos_hex_search = Hex_search_unicode(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA), tmp, last_pos_hex_search);
              }else if (IsDlgButtonChecked(hwnd,DLG_HEXA_CHK_HEXA)==BST_CHECKED)
              {
                if (last_search_mode != HEXA_MODE_SEARCH_HX)
                {
                  //reinit search
                  last_pos_hex_search   = 0;
                  last_search_mode = HEXA_MODE_SEARCH_HX;
                }

                char tmp[DEFAULT_TMP_SIZE];
                SendDlgItemMessage(hwnd,DLG_HEXA_EDT_SEARCH,WM_GETTEXT ,(WPARAM)DEFAULT_TMP_SIZE, (LPARAM)tmp);

                if (last_pos_hex_search)last_pos_hex_search++;
                last_pos_hex_search = Hex_search_hexa(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA), tmp, last_pos_hex_search);
              }else
              {
                if (last_search_mode != DEFAULT_MODE_SEARCH_HX)
                {
                  //reinit search
                  last_pos_hex_search   = 0;
                  last_search_mode = DEFAULT_MODE_SEARCH_HX;
                }

                char tmp[DEFAULT_TMP_SIZE];
                SendDlgItemMessage(hwnd,DLG_HEXA_EDT_SEARCH,WM_GETTEXT ,(WPARAM)DEFAULT_TMP_SIZE, (LPARAM)tmp);

                if (last_pos_hex_search)last_pos_hex_search++;
                last_pos_hex_search = Hex_search_ansi(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA), tmp, last_pos_hex_search);
              }

            break;
            case DLG_HEXA_CHK_UNICODE:CheckDlgButton(hwnd,DLG_HEXA_CHK_HEXA,BST_UNCHECKED);break;
            case DLG_HEXA_CHK_HEXA:CheckDlgButton(hwnd,DLG_HEXA_CHK_UNICODE,BST_UNCHECKED);break;
            case POPUP_CP_LINE:CopyAllDataToClipboard(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA), SendMessage(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA),LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 10);break;
            case POPUP_CP_LINE_HEXA:CopyColumnDataToClipboard(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA), SendMessage(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA),LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 8,1);break;
            case POPUP_CP_LINE_STRING:CopyColumnDataToClipboard(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA), SendMessage(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA),LVM_GETNEXTITEM,-1,LVNI_FOCUSED), 1,9);break;
          }
        break;
        //case CBN_SELCHANGE:CalculDate(last_bt);break;
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

      if (h_Hexa != 0)
      {
        h_Hexa = NULL;
        Sleep(1000);
        //clean LSVs
        ListView_DeleteAllItems(GetDlgItem(hwnd,DLG_HEXA_LV_INFOS));
        ListView_DeleteAllItems(GetDlgItem(hwnd,DLG_HEXA_LV_HEXA));
      }
      h_Hexa = CreateThread(NULL,0,LoadHexaFile,(LPVOID)(LPARAM)file,0,0);

      DragFinish(H_DropInfo);
    }break;
    case WM_CONTEXTMENU:
      if ((HWND)wParam == GetDlgItem(hwnd,DLG_HEXA_LV_HEXA))
      {
        HMENU hmenu;
        if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_LSTV_HEXA)))!= NULL)
        {
          POINT pos;
          if (GetCursorPos(&pos)!=0)
          {
            TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, pos.x, pos.y,hwnd, NULL);
          }
          DestroyMenu(hmenu);
        }
      }
    break;
    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code)
      {
        case NM_DBLCLK:
          if (LOWORD(wParam) == DLG_HEXA_LV_INFOS || LOWORD(wParam) == DLG_HEXA_LV_HEXA)
          {
            HANDLE hlstv = GetDlgItem(hwnd,LOWORD(wParam));
            long i, index = SendMessage(hlstv,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
            if (index > -1)
            {
              char tmp[MAX_LINE_SIZE+2];
              RichEditInit(GetDlgItem(h_info,DLG_INFO_TXT));

              for (i=0;i<10;i++)
              {
                tmp[0] = 0;
                ListView_GetItemText(hlstv,index,i,tmp,MAX_LINE_SIZE);
                if (*tmp != '\0')
                {
                  strncat(tmp," \0",MAX_LINE_SIZE);
                  RichEditCouleur(GetDlgItem(h_info,DLG_INFO_TXT),NOIR,tmp);
                }
              }

              RichSetTopPos(GetDlgItem(h_info,DLG_INFO_TXT));
              if(RichEditTextSize(GetDlgItem(h_info,DLG_INFO_TXT)))
              {
                ShowWindow (h_info, SW_SHOW);
                UpdateWindow(h_info);
              }
            }
          }
        break;
      }
    break;
    case WM_CLOSE : ShowWindow(hwnd, SW_HIDE);break;
  }
  return FALSE;
}
