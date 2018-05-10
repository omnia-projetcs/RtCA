//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addClipboardtoDB(char *format, unsigned int code, char*description, char *data, char *user,
                  unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_clipboard (format,code,description,user,session_id,data) "
           "VALUES(\"%s\",\"%05d\",\"%s\",\"%s\",%d,\"%s\");",
           format,code,description,user,session_id,data);

  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4-strlen(request));

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Clipboard\";\"%s\";\"%05d\";\"%s\";\"%s\";\"%d\";\"%s\";\r\n",
         format,code,description,user,session_id,data);
  #endif
}
//------------------------------------------------------------------------------
void DatatoHexa(BYTE *data, DWORD data_size, char *str, DWORD str_size)
{
  str[0] = 0;
  char *m = (char *)malloc(data_size*3);
  if (m == NULL) return;
  DWORD i,j, size = 0;
  long int pos = 0;
  char *p, lineA[33], lineB[33];

  for (i=0;i<data_size && size<str_size;)
  {
    lineA[0] = 0;
    lineB[0] = 0;
    p = lineA;

    for (j=0;j<16 && i<data_size;j++,i++,p+=2)
    {
       snprintf(p,3,"%02X",data[i]&0xFF);
       if (data[i]>31 && data[i]<127)lineB[j]=data[i];else lineB[j]='.';
    }
    lineB[j] = 0;

    size = strlen(str);
    pos = str_size-size;
    if (pos> 0 && size<str_size)snprintf(&str[size],pos,"%s  %s\r\n",lineA,lineB);
  }
  free(m);
}
//------------------------------------------------------------------------------
void SaveBitmapToHexaStr(HBITMAP hBmp , char *str, DWORD str_size)
{
  BITMAP bmpInfo;
  GetObject(hBmp, sizeof(bmpInfo), &bmpInfo);

  SIZE sizeBmp;
  sizeBmp.cx = bmpInfo.bmWidth;
  sizeBmp.cy = bmpInfo.bmHeight;

  // allow memory
  DWORD dwSize = 4*sizeBmp.cx*sizeBmp.cy;
  LPVOID lpMem = VirtualAlloc(NULL, dwSize, MEM_COMMIT, PAGE_READWRITE);
  if(lpMem == NULL)return;

  BITMAPINFO bi;
  ZeroMemory(&bi, sizeof(BITMAPINFO));
  bi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi.bmiHeader.biPlanes = 1;
  bi.bmiHeader.biBitCount = 32;
  bi.bmiHeader.biWidth = sizeBmp.cx;
  bi.bmiHeader.biHeight = sizeBmp.cy;
  bi.bmiHeader.biCompression = BI_RGB;

  //read datas
  HDC hdc = GetDC(NULL);
  int nResult = GetDIBits(hdc, hBmp, 0, sizeBmp.cy, lpMem, &bi, DIB_RGB_COLORS);
  ReleaseDC(NULL, hdc);
  if (nResult > 0)DatatoHexa(lpMem, dwSize, str, str_size);

  VirtualFree(lpMem,dwSize,MEM_DECOMMIT);
}
//------------------------------------------------------------------------------
//http://msdn.microsoft.com/en-us/library/windows/desktop/ms649016%28v=vs.85%29.aspx
DWORD WINAPI Scan_clipboard(LPVOID lParam)
{
  //check if local or not :)
  if (!LOCAL_SCAN)
  {
    h_thread_test[(unsigned int)lParam] = 0;
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
    return 0;
  }

  //db
  sqlite3 *db = (sqlite3 *)db_scan;
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);

  //lecture du contenu du presse papier et extraction
  if (OpenClipboard(0))
  {
    char description[MAX_LINE_SIZE], format[DEFAULT_TMP_SIZE],
    data[MAX_LINE_SIZE],user[NB_USERNAME_SIZE+1]="";
    unsigned int session_id = current_session_id;
    HGLOBAL hMem;

    //user
    DWORD s=NB_USERNAME_SIZE;
    GetUserName(user,&s);

    int nb_items = CountClipboardFormats();
    if (nb_items > 0)
    {
      unsigned int uFormat = EnumClipboardFormats(0);
      #ifdef CMD_LINE_ONLY_NO_DB
      printf("\"Clipboard\";\"format\";\"code\";\"description\";\"user\";\"session_id\";\"data\";\r\n");
      #endif // CMD_LINE_ONLY_NO_DB
      while (uFormat && start_scan && GetLastError() == ERROR_SUCCESS && --nb_items>0)
      {
        //check if ok
        if (IsClipboardFormatAvailable(uFormat) == FALSE)
        {
          uFormat = EnumClipboardFormats(uFormat);
          continue;
        }

        description[0] = 0;
        data[0]= 0;
        if (GetClipboardFormatName(uFormat, description, MAX_LINE_SIZE) != 0)
        {
          hMem = GetClipboardData(uFormat);
          if (hMem != NULL)
          {
            switch(uFormat)
            {
              case CF_TEXT:
                //format
                strncpy(format,"CF_TEXT",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Text",DEFAULT_TMP_SIZE);
                //datas
                strncpy(data,GlobalLock(hMem),MAX_LINE_SIZE);
                convertStringToSQL(data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_BITMAP:
                //format
                strncpy(format,"CF_BITMAP",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Bitmap Picture",DEFAULT_TMP_SIZE);
                //do in bitmap to hexa
                SaveBitmapToHexaStr((HBITMAP)hMem , data, MAX_LINE_SIZE);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_METAFILEPICT:
                //format
                strncpy(format,"CF_METAFILEPICT",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Meta-File Picture",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
                GlobalUnlock(hMem);
              break;
              case CF_SYLK:
                //format
                strncpy(format,"CF_SYLK",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Microsoft Symbolic Link (SYLK) data",DEFAULT_TMP_SIZE);
                //datas
                snprintf(data,MAX_LINE_SIZE,"%s",(char*)GlobalLock(hMem));
                convertStringToSQL(data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_OEMTEXT:
                //format
                strncpy(format,"CF_OEMTEXT",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Text (OEM)",DEFAULT_TMP_SIZE);
                //datas
                strncpy(data,GlobalLock(hMem),MAX_LINE_SIZE);
                convertStringToSQL(data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_DIB:
                //format
                strncpy(format,"CF_DIB",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"DIB Bitmap Picture",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_DIF:
                //format
                strncpy(format,"CF_DIF",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Software Arts' Data Interchange information",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_TIFF:
                //format
                strncpy(format,"CF_TIFF",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Tagged Image File Format (TIFF) Picture",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_PALETTE:
                //format
                strncpy(format,"CF_PALETTE",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Colour Palette",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_PENDATA:
                //format
                strncpy(format,"CF_PENDATA",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Pen Data",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_UNICODETEXT:
                //format
                strncpy(format,"CF_UNICODETEXT",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Text Unicode",DEFAULT_TMP_SIZE);
                //datas
                snprintf(data,MAX_LINE_SIZE,"%S",GlobalLock(hMem));
                convertStringToSQL(data, MAX_LINE_SIZE);h_thread_test[(unsigned int)lParam] = 0;
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_RIFF:
                //format
                strncpy(format,"CF_RIFF",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"RIFF Audio data",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_WAVE:
                //format
                strncpy(format,"CF_WAVE",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Wave File",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_ENHMETAFILE:
                //format
                strncpy(format,"CF_ENHMETAFILE",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Enhanced Meta-File Picture",DEFAULT_TMP_SIZE);
                //datas
                DWORD dwSize = GetEnhMetaFileBits((HENHMETAFILE)hMem, 0, NULL);
                if (dwSize > 0)
                {
                  LPBYTE buffer = (LPBYTE)malloc(dwSize);
                  if (buffer != NULL)
                  {
                    if (GetEnhMetaFileBits((HENHMETAFILE)hMem, dwSize, buffer)!=0)
                    {
                      DatatoHexa(buffer, dwSize, data, MAX_LINE_SIZE);
                    }
                    free(buffer);
                  }
                }
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case CF_HDROP:
              {
                //format
                strncpy(format,"CF_HDROP",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"File List",DEFAULT_TMP_SIZE);

                HDROP H_DropInfo = (HDROP)hMem;
                char tmp[MAX_PATH];
                DWORD i,nb_path = DragQueryFile(H_DropInfo, 0xFFFFFFFF, tmp, MAX_PATH);
                long int s2 =MAX_LINE_SIZE;
                for (i=0;i<nb_path;i++)
                {
                  //traitement des données ^^
                  DragQueryFile(H_DropInfo, i, tmp, MAX_PATH);

                  //add
                  if (s2>0)
                  {
                    snprintf(data+strlen(data),s,"%s\r\n",tmp);
                    //strncpy(data+strlen(data),tmp,s);
                    s2-=strlen(data);
                  }
                }
                convertStringToSQL(data, MAX_LINE_SIZE);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              }
              break;
              case CF_LOCALE:
                //format
                strncpy(format,"CF_LOCALE",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Text Locale Identifier",DEFAULT_TMP_SIZE);
                //datas
                snprintf(data,MAX_LINE_SIZE,"0x%X",(unsigned int)GlobalLock(hMem));
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case 17: //CF_DIBV5
                //format
                strncpy(format,"CF_DIBV5",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), sizeof(BITMAPV5HEADER), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case 49155:
                //format
                strncpy(format,"UNKNOW",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"OwnerLink",DEFAULT_TMP_SIZE);
                //datas
                strncpy(data,GlobalLock(hMem),MAX_LINE_SIZE);
                convertStringToSQL(data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case 49156:
                //format
                strncpy(format,"UNKNOW",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Native Bitmap Picture",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case 49158:
                //format
                strncpy(format,"UNKNOW",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"FileName",DEFAULT_TMP_SIZE);
                //datas
                strncpy(data,GlobalLock(hMem),MAX_LINE_SIZE);
                convertStringToSQL(data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case 49159:
                //format
                strncpy(format,"UNKNOW",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"FileNameW",DEFAULT_TMP_SIZE);
                //datas
                snprintf(data,MAX_LINE_SIZE,"%S",GlobalLock(hMem));
                convertStringToSQL(data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              case 49298:
                //format
                strncpy(format,"UNKNOW",DEFAULT_TMP_SIZE);
                if (description[0]==0)strncpy(description,"Rich Text Format",DEFAULT_TMP_SIZE);
                //datas
                snprintf(data,MAX_LINE_SIZE,"%s",(char*)GlobalLock(hMem));
                convertStringToSQL(data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
              default:
                //format
                strncpy(format,"UNKNOW",DEFAULT_TMP_SIZE);
                //datas
                DatatoHexa(GlobalLock(hMem), GlobalSize(hMem), data, MAX_LINE_SIZE);
                GlobalUnlock(hMem);
                addClipboardtoDB(format, uFormat, description, data, user, session_id, db);
              break;
            }
          }
        }
        uFormat = EnumClipboardFormats(uFormat);
      }
    }
    CloseClipboard();
  }
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
