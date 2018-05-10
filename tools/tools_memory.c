//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//memory dump of process (need debug mode)
DWORD WINAPI DumpProcessMemory(LPVOID lParam)
{
  //open process
  DWORD pid = (DWORD)lParam;
  HANDLE hProc = OpenProcess(PROCESS_VM_READ|PROCESS_QUERY_INFORMATION/*|PROCESS_ALL_ACCESS*/, FALSE, pid);
  if (hProc!=NULL)
  {
    //hardward info : last memory process
    SYSTEM_INFO si;
    #ifdef _WIN64_VERSION_  //64bit
      GetSystemInfo(&si);
    #else
      GetNativeSystemInfo(&si);
    #endif

    OPENFILENAME ofn;
    ZeroMemory(&ofn, sizeof(OPENFILENAME));
    char path[MAX_PATH] ="";
    ofn.lStructSize     = sizeof(OPENFILENAME);
    ofn.hwndOwner       = h_main;
    ofn.lpstrFile       = path;
    ofn.nMaxFile        = MAX_PATH;
    ofn.lpstrFilter     = "RAW\0*.raw\0";
    ofn.nFilterIndex    = 1;
    ofn.Flags           = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
    ofn.lpstrDefExt     ="raw\0";

    if (GetSaveFileName(&ofn)==TRUE)
    {
      HANDLE MyhFile = CreateFile(path, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, 0);
      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        DWORD copiee;
        MEMORY_BASIC_INFORMATION mbi;
        LPVOID mem = 0;
        LPVOID buffer;
        while (mem < si.lpMaximumApplicationAddress)
        {
          //read memory info
          if (!VirtualQueryEx(hProc,mem, &mbi, sizeof(MEMORY_BASIC_INFORMATION)))break;
          if (mbi.RegionSize > 0)
          {
            buffer = (char*)malloc(mbi.RegionSize);
            if (buffer!=NULL)
            {
              if (ReadProcessMemory(hProc, mbi.BaseAddress, buffer, mbi.RegionSize, NULL))
                WriteFile(MyhFile,buffer,mbi.RegionSize,&copiee,0);

              free(buffer);
            }
          }else break;
          //next
          mem = (LPVOID)((DWORD)mbi.BaseAddress + (DWORD)mbi.RegionSize);
        }
      }
      CloseHandle(MyhFile);
    }
    CloseHandle(hProc);
  }
  return 0;
}
//------------------------------------------------------------------------------
