//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//-----------------------------------------------------------------------------
/*
NOTIFYICONDATA TrayIcon;
HHOOK HHook; // Handle du hook global
*/
//-----------------------------------------------------------------------------
//Infos: http://msdn.microsoft.com/en-us/library/dd183402%28v=vs.85%29.aspx
DWORD WINAPI ImpEcran(LPVOID lParam)
{
  if (B_SCREENSHOT_START)return 0;
  B_SCREENSHOT_START = TRUE;
  //unsigned int type_screenshot = (unsigned int)lParam;

//set file to save
  //actual date
  time_t date;
  time(&date);
  //actual path
  char local_path[MAX_PATH] = "";
  char fic[MAX_PATH]        = "";

  snprintf(fic,MAX_PATH,"%s\\%s",GetLocalPath(local_path, MAX_PATH),(char *)ctime(&date));
  fic[strlen(fic)-1] = 0;
  strncat(fic,".bmp\0",MAX_PATH-strlen(fic));
  fic[strlen(fic)-15]= 'h';
  fic[strlen(fic)-12]= 'm';

  //only window : get window name
  HANDLE HFen=0;
  /*if (type_screenshot == 1)
  {
    HFen=GetForegroundWindow();
    if (HFen == 0)return 0;

    char tmp[MAX_PATH];
    GetWindowText(HFen,tmp,MAX_PATH);

    //deletes bad char to filename
    char *c = tmp;
    while(*c)
    {
      if (*c == '|' || *c == '<' || *c == '>' || *c == ':' || *c == '*' || *c == '?' || *c == '"' || *c == '/' || *c == '\\')*c = ' ';
      c++;
    }

    //add to file name :)
    unsigned int s = strlen(fic);
    if (s > 4)
    {
      fic[s-4]='_';
      fic[s-3]=0;
      strncat(fic,tmp,MAX_PATH);
      strncat(fic,".bmp\0",MAX_PATH);
    }
  }*/

  unsigned int Height=0,Width=0;

  //get datas :)
  HDC memDC,hDC;
  //RECT rec;
  if ((memDC=GetDC(HFen))!=0)
  {
    if ((hDC=CreateCompatibleDC(memDC))!=0)
    {
      /*if (type_screenshot == 1 && HFen!=NULL)
      {
          GetClientRect(HFen,&rec);

          Height =rec.bottom-rec.top;
          Width = rec.right-rec.left;
      }else
      {*/
        Height=GetDeviceCaps(memDC,VERTRES);
        Width=GetDeviceCaps(memDC,HORZRES);
      //}

      if (Height && Width)
      {
        BITMAPINFO BmpInfo;
        BmpInfo.bmiHeader.biSize          =sizeof(BITMAPINFOHEADER);
        BmpInfo.bmiHeader.biWidth         =Width;
        BmpInfo.bmiHeader.biHeight        =Height;
        BmpInfo.bmiHeader.biPlanes        =1;
        BmpInfo.bmiHeader.biBitCount      =24;
        BmpInfo.bmiHeader.biCompression   =BI_RGB;
        BmpInfo.bmiHeader.biSizeImage     =0;
        BmpInfo.bmiHeader.biXPelsPerMeter =0;
        BmpInfo.bmiHeader.biYPelsPerMeter =0;
        BmpInfo.bmiHeader.biClrUsed       =0;
        BmpInfo.bmiHeader.biClrImportant  =0;

        //get data, convert and copy to memory !!!
        void *pBits;
        HBITMAP Hbmp;
        if ((Hbmp = CreateDIBSection(0,&BmpInfo,DIB_RGB_COLORS,&pBits,0,0))!=0)
        {
          if (pBits != NULL)
          {
            if (SelectObject(hDC,Hbmp)!=0)
            {
              if (BitBlt(hDC,0,0,Width,Height,memDC,0,0,SRCCOPY))
              {
                unsigned int dwBmBitsSize = ((Width * 24+31)/32)* 4 *Height;
                HANDLE mem_bmp = GlobalAlloc(GHND, dwBmBitsSize + sizeof(BITMAPINFOHEADER));
                if (mem_bmp != NULL)
                {
                  LPBITMAPINFOHEADER lpbi = (LPBITMAPINFOHEADER)GlobalLock(mem_bmp);
                  *lpbi = BmpInfo.bmiHeader;

                  //get datas
                  if (GetDIBits(memDC, Hbmp, 0, (UINT) Height, (LPSTR)lpbi + sizeof(BITMAPINFOHEADER),(LPBITMAPINFO)lpbi, DIB_RGB_COLORS)!=0)
                  {
                    //create file
                    HANDLE hfile = CreateFile(fic, GENERIC_WRITE,0,NULL,CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL|FILE_FLAG_SEQUENTIAL_SCAN, NULL);
                    if (hfile != INVALID_HANDLE_VALUE)
                    {
                      BITMAPFILEHEADER   bmfHdr;
                      bmfHdr.bfType = 0x4D42; // "BM"
                      bmfHdr.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + dwBmBitsSize;
                      bmfHdr.bfReserved1 = 0;
                      bmfHdr.bfReserved2 = 0;
                      bmfHdr.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER)+(DWORD)sizeof(BITMAPINFOHEADER);

                      DWORD writed;
                      WriteFile(hfile, (LPSTR)&bmfHdr, sizeof (BITMAPFILEHEADER), &writed, NULL);
                      WriteFile(hfile, (LPSTR)lpbi, bmfHdr.bfSize, &writed, NULL);
                      CloseHandle(hfile);
                    }
                  }
                  GlobalFree(mem_bmp);
                }
              }
            }
          }
          DeleteObject(Hbmp);
        }
      }
      DeleteDC(hDC);
    }
    DeleteDC(memDC);
  }
  B_SCREENSHOT_START = FALSE;
  return 0;
}
//-----------------------------------------------------------------------------
__declspec(dllexport) LRESULT CALLBACK HookProc ( int Format, WPARAM WParam, LPARAM LParam)
{
   //si imprime écran standard
    if ((Format == HC_ACTION) && (WParam == WM_KEYDOWN || WM_SYSCHAR))
    {
        // Structure d'info sur la touche appuyée
        KBDLLHOOKSTRUCT hookstruct = *((KBDLLHOOKSTRUCT*)LParam);
        if(hookstruct.vkCode==44)CreateThread(0,0,ImpEcran,0,0,0);              //touche: Impr écran
        else if(hookstruct.vkCode==19)CreateThread(0,0,ImpEcran,(LPVOID)1,0,0); //touche Pause
        else if(hookstruct.vkCode==35)CreateThread(0,0,ImpEcran,(LPVOID)2,0,0); //touche Fin
    }
   // Renvoi des messages au sytème pour permettre d'autres hooks
   return CallNextHookEx(HHook, Format, WParam, LParam);
}
//-----------------------------------------------------------------------------
void SCREENSHOT_fct()
{
  B_SCREENSHOT = !B_SCREENSHOT;
  if (B_SCREENSHOT)
  {
    //add tray icone
    ZeroMemory(&TrayIcon, sizeof(NOTIFYICONDATA));
    TrayIcon.cbSize = sizeof(NOTIFYICONDATA);
    TrayIcon.hWnd = h_main;
    TrayIcon.uID = 0;
    TrayIcon.hIcon = (HICON)LoadIcon(hinst, MAKEINTRESOURCE(ICON_APP));
    TrayIcon.uCallbackMessage = MY_WM_NOTIFYICON;
    TrayIcon.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    strcpy(TrayIcon.szTip, NOM_FULL_APPLI);
    Shell_NotifyIcon(NIM_ADD,&TrayIcon);

    //start hooking
    HHook = SetWindowsHookEx( WH_KEYBOARD_LL, (HOOKPROC) HookProc, hinst, 0);

    //check
    CheckMenuItem(GetMenu(h_main),BT_SREEENSHOT,MF_BYCOMMAND|MF_CHECKED);
  }else
  {
    //remove tray icone
    DestroyIcon(TrayIcon.hIcon);
    Shell_NotifyIcon(NIM_DELETE,&TrayIcon);

    //stop hooking
    UnhookWindowsHookEx(HHook);

    //check
    CheckMenuItem(GetMenu(h_main),BT_SREEENSHOT,MF_BYCOMMAND|MF_UNCHECKED);
  }
}
