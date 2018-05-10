//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "RtCA.h"
//------------------------------------------------------------------------------
BOOL isValideANSIChar(char *s, DWORD size)
{
  if (*s == 0) return FALSE;
  DWORD i;
  BOOL last = FALSE;
  for (i=0;i<size;i++)
  {
    if (s[i] > 127 || s[i] < 32)
    {
      if (last) return FALSE;
      else last = TRUE;
    }else last = FALSE;
  }
  return TRUE;
}
//------------------------------------------------------------------------------
DWORD WINAPI ImportCVSorSHA256deep(LPVOID lParam)
{
  //récupération du path
  OPENFILENAME ofn;
  char file[MAX_PATH]="";
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_main;
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_PATH;
  ofn.lpstrFilter ="CSV (*.csv)\0*.csv\0 sha256deep (*)\0*\0";
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_FILEMUSTEXIST| OFN_ALLOWMULTISELECT | OFN_EXPLORER;
  ofn.lpstrDefExt ="csv\0";

  if (GetOpenFileName(&ofn)==TRUE)
  {
    if (ofn.nFileOffset > 0)
    {
      //clean lstv
      ListView_DeleteAllItems(hlstv);

      //load file
      HANDLE hfile = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
      if (hfile == INVALID_HANDLE_VALUE)return 0;

      DWORD pos, taille_fic = GetFileSize(hfile,NULL);
      if (taille_fic <1 || taille_fic == INVALID_FILE_SIZE)
      {
        CloseHandle(hfile);
        return 0;
      }

      //memalloc
      char *buffer = (char *) HeapAlloc(GetProcessHeap(), 0, taille_fic+1);
      if (buffer == NULL)
      {
        CloseHandle(hfile);
        return FALSE;
      }

      //load file by 10M datas
      DWORD copiee, increm = 0, position =0;
      if (taille_fic > DIXM)increm = DIXM;
      else increm = taille_fic;

      while (position<taille_fic && increm!=0)
      {
        copiee = 0;
        ReadFile(hfile, buffer+position, increm,&copiee,0);
        position=position+copiee;
        if (taille_fic-position < increm)increm = taille_fic-position ;
      }
      CloseHandle(hfile);

      char line[MAX_LINE_SIZE],line_t[MAX_LINE_SIZE], *l, *p, *b = buffer;
      char *e = line+(MAX_LINE_SIZE-3);
      char file[MAX_PATH], ext[15], *c;

      LVITEM lvi;
      lvi.mask = LVIF_TEXT|LVIF_PARAM;
      lvi.iSubItem = 0;
      lvi.lParam = LVM_SORTITEMS;
      lvi.pszText="";
      lvi.iItem = ListView_GetItemCount(hlstv);

      if (ofn.nFilterIndex == 1) //CSV
      {
        BOOL separator = FALSE;
        do
        {
          //lines format : unix/windows
          /*
          "text";"text";
          "text";"text"
          test;test;
          test;test
          */

          //get line
          memset(line,0,MAX_LINE_SIZE);
          l = line;
          while (*b && *b != '\r' && *b!='\n' && e>l && (l-line < MAX_LINE_SIZE)) *l++=*b++;
          if (*b == '\r')b++;
          *l = 0;
          *l++ = 0;

          //remove ; in end of line
          if (*(--l) == ';')*l = 0;

          //transform if unicode, utf16 or utf8 !!!
          if (!isValideANSIChar(line,strlen(line)))
          {
            snprintf(line_t,MAX_LINE_SIZE,"%s",line);
            convertUTF8toUTF16toChar(line_t, strlen(line_t), line, MAX_LINE_SIZE);
          }

          //add item
          lvi.iItem = lvi.iItem+1;
          pos = ListView_InsertItem(hlstv, &lvi);

          //get data from each column !
          l = line;
          if (*l == '"') separator = TRUE;
          else separator = FALSE;
          for (increm = 0; increm<20 && *l;increm++)
          {
            if (separator)l++;
            if (*l)
            {
              //search end of column
              p = l;
              while (*l && *l !=';' && (*l !='"' || !separator))l++;
              *l = 0;
              if (*p != '\0')
              {
                ListView_SetItemText(hlstv,pos,increm,p);
              }

              if (separator)l+=2;
              else l++;
            }
          }
        }while (*b++);
      }else //sha256deep
      {
        current_session_id = 0;

        //add in bdd or not ?
        if (MessageBox(h_main,cps[TXT_MSG_RIGHT_ADMIN_ATTENTION].c,cps[REF_MSG].c,MB_YESNO|MB_TOPMOST)==IDYES)
        {
          //create new session and select it !
          SendMessage(hCombo_session, CB_RESETCONTENT,0,0);
          FORMAT_CALBAK_READ_INFO fcri;
          fcri.type = TYPE_SQL_ADD_SESSION;
          SQLITE_WriteData(&fcri, SQLITE_LOCAL_BDD);
          if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
        }

        char sha256_s[65];
        do
        {
          //get line
          memset(line,0,MAX_LINE_SIZE);
          l = line;
          while (*b && *b != '\r' && *b!='\n' && e>l && (l-line < MAX_LINE_SIZE)) *l++=*b++;
          if (*b == '\r')b++;
          *l = 0;

          //transform if unicode, utf16 or utf8 !!!
          if (!isValideANSIChar(line,strlen(line)))
          {
            snprintf(line_t,MAX_LINE_SIZE,"%s",line);
            convertUTF8toUTF16toChar(line_t, strlen(line_t), line, MAX_LINE_SIZE);
          }

          //working
          //line format : 64 = sha256 + 2 space + path+file
          if (strlen(line) > 68)
          {
            //verify format
            if (line[64] != ' ' || line[65] != ' ') continue;

            //add item
            lvi.iItem = lvi.iItem+1;
            pos = ListView_InsertItem(hlstv, &lvi);

            c = &line[66];
            //file
            memset(file,0,MAX_PATH);
            ListView_SetItemText(hlstv,pos,COLUMN_FILE,extractFileFromPath(c, file, MAX_PATH));

            //ext
            memset(ext,0,15);
            ListView_SetItemText(hlstv,pos,COLUMN_EXT,extractExtFromFile(file, ext, 15));

            //path
            c = &line[66];
            while (*c++);
            while (*c != '\\' && *c != '/')c--;
            c++;
            *c=0;
            ListView_SetItemText(hlstv,pos,COLUMN_PATH,&line[66]);

            //SHA256
            strncpy(sha256_s,line,64);
            sha256_s[64] = 0;
            ListView_SetItemText(hlstv,pos,COLUMN_SHA256,sha256_s);

            if (current_session_id != 0)
              addFiletoDB(&line[66],file,ext,"","","","","","","","","","","","","","",sha256_s,"","", current_session_id, db_scan);
          }
        }while (*b++);
        if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

        if (current_session_id != 0)EndSession(0, db_scan);
      }
      //free memory
      HeapFree(GetProcessHeap(), 0, buffer);
    }
  }

  char msg[MAX_PATH];
  snprintf(msg,MAX_PATH,"Load file done !!! (%u items)",ListView_GetItemCount(hlstv));
  SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)msg);
  return 0;
}
