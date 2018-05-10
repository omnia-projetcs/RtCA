//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
#define dd_buffer_size  4096
//------------------------------------------------------------------------------
BOOL dd(char *disk, char *file, LONGLONG file_sz_max, BOOL progress)
{
  BOOL ret = FALSE;
  //different disk formats :
  //"\\\\?\\C:"
  //"\\\\.\\PhysicalDrive0"

  //create file to save
  HANDLE hf_save = CreateFile(file,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,NULL);
  if (hf_save != INVALID_HANDLE_VALUE)
  {
    //open disk path
    HANDLE hdisk = CreateFile(disk,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
    if (hdisk != INVALID_HANDLE_VALUE)
    {
      //next for type :
      DWORD dw=0;
      char bf[dd_buffer_size]="", cprogress[MAX_LINE_SIZE];

      //all disk
      if (file_sz_max == 0)
      {
        LONGLONG bread = 0;
        do
        {
          dw = 0;
          if(ReadFile(hdisk, bf, dd_buffer_size,&dw,0))
          {
            dw = 0;
            if (WriteFile(hf_save, bf, dd_buffer_size,&dw,0))
            {
              if (dw != 0)
              {
                ret = TRUE;
                bread = bread+dd_buffer_size;

                if (progress)
                {
                  snprintf(cprogress,MAX_LINE_SIZE,"DD %llu mo %s -> %s",bread/1048576,disk,file);
                  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)cprogress);
                  printf("%s\r",cprogress);
                }
              }else break;
            }else break;
          }else break;
        }while(dw != 0);

      //MBR or similar size
      }else if (file_sz_max < dd_buffer_size)
      {
        if(ReadFile(hdisk, bf, dd_buffer_size,&dw,0))
        {
          dw = 0;
          //if the size < 512 impossible, if the size < 4096 error
          //no need to test the error but the data read
          if(WriteFile(hf_save, bf, file_sz_max,&dw,0))
          {
            ret = TRUE;
            if (progress)
            {
              snprintf(cprogress,MAX_LINE_SIZE,"DD %llu b %s -> %s",file_sz_max,disk,file);
              SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)cprogress);
              printf("%s\n",cprogress);
            }
          }
        }
      }else
      {
        //other size limit
        LONGLONG bread = 0;
        unsigned int bu_sz = dd_buffer_size;
        do
        {
          dw = 0;
          if(ReadFile(hdisk, bf, bu_sz,&dw,0))
          {
            dw = 0;
            if (WriteFile(hf_save, bf, bu_sz,&dw,0))
            {
              if (dw != 0)
              {
                bread = bread + bu_sz;
                if (file_sz_max-bread < bu_sz)bu_sz = file_sz_max-bread;

                if (progress)
                {
                  snprintf(cprogress,MAX_LINE_SIZE,"DD %llu/%llu mo %s -> %s",bread/1048576,file_sz_max/1048576,disk,file);
                  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)cprogress);
                  printf("%s\r",cprogress);
                }
              }
              else break;
            }else break;
          }else break;
        }while (bread < file_sz_max);

        if (bread == file_sz_max)ret = TRUE;
      }
      CloseHandle(hdisk);
    }
    CloseHandle(hf_save);
  }

  return ret;
}
//------------------------------------------------------------------------------
void dd_mbr()
{
  char file[MAX_LINE_SIZE]="";
  GenerateNameToSave(file, MAX_LINE_SIZE, "MBR.raw");
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_main;
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_LINE_SIZE;
  ofn.lpstrFilter ="*.raw \0*.raw\0*.* \0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
  ofn.lpstrDefExt ="raw\0";

  if (GetSaveFileName(&ofn)==TRUE)
  {
    if(!dd("\\\\.\\PhysicalDrive0", file, 512, TRUE))
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"MBR copy : error!");
  }
}
//------------------------------------------------------------------------------
//ext = test.zip
char *GenerateNameToSave(char *name, DWORD name_max_size, char *ext)
{
  //make filename
  DWORD taille = 256;
  char computername[256]="";
  GetComputerName(computername,&taille);

  //get current time
  time_t date;
  time(&date);
  struct tm *today = localtime(&date);
  char date_today[DATE_SIZE_MAX];
  strftime(date_today, DATE_SIZE_MAX,"%Y-%m-%d_%H.%M.%S",today);
  snprintf(name,name_max_size,"[%s]_%s_%s",computername,date_today,ext);
  return name;
}
//------------------------------------------------------------------------------
DWORD WINAPI BackupDrive(LPVOID lParam)
{
  char letter = lParam;
  char file[MAX_LINE_SIZE]="", tmp[MAX_LINE_SIZE]="";
  GenerateNameToSave(file, MAX_LINE_SIZE, "BackupDrive.raw");

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_main;
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_LINE_SIZE;
  ofn.lpstrFilter ="*.raw \0*.raw\0*.* \0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
  ofn.lpstrDefExt ="raw\0";

  if (GetSaveFileName(&ofn)==TRUE)
  {
    snprintf(tmp,MAX_LINE_SIZE,"\\\\?\\%c:",letter);
    dd(tmp, file, 0, TRUE);
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI BackupDisk(LPVOID lParam)
{
  char letter = lParam;
  char file[MAX_LINE_SIZE]="", tmp[MAX_LINE_SIZE]="";
  GenerateNameToSave(file, MAX_LINE_SIZE, "BackupDisk.raw");

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_main;
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_LINE_SIZE;
  ofn.lpstrFilter ="*.raw \0*.raw\0*.* \0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
  ofn.lpstrDefExt ="raw\0";

  if (GetSaveFileName(&ofn)==TRUE)
  {
    char pathdisk[MAX_LINE_SIZE];
    snprintf(pathdisk,MAX_LINE_SIZE,"\\\\.\\PhysicalDrive%c",letter);

    if(dd(pathdisk, file, 0, TRUE))
    {
      snprintf(tmp,MAX_LINE_SIZE,"Physical Drive imagine %s saved to: %s",pathdisk,file);
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp);
    }else
    {
      snprintf(tmp,MAX_LINE_SIZE,"Error in imagine Physical Drive %s to: %s",pathdisk,file);
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp);
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
BOOL CreateFileCopyFilefromPath(char *path_src, char *path_dst)
{
  HANDLE Hfic_src = CreateFile(path_src,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
  if (Hfic_src == INVALID_HANDLE_VALUE)return FALSE;

  HANDLE Hfic_dst = CreateFile(path_dst,GENERIC_WRITE,FILE_SHARE_WRITE, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, 0);
  if (Hfic_dst == INVALID_HANDLE_VALUE)
  {
    CloseHandle(Hfic_dst);
    return FALSE;
  }

  DWORD position = 0, increm = 0, copiee, taille_fic = GetFileSize(Hfic_src,NULL);
  if (taille_fic > 0)
  {
    char *buffer = (char *) HeapAlloc(GetProcessHeap(), 0, taille_fic+1);
    if (!buffer)
    {
      CloseHandle(Hfic_src);
      CloseHandle(Hfic_dst);
      return FALSE;
    }

    if (taille_fic > DIXM)increm = DIXM;
    else increm = taille_fic;

    while (position<taille_fic && increm!=0)
    {
      copiee = 0;
      ReadFile(Hfic_src, buffer+position, increm, &copiee,0);
      position += copiee;
      if (taille_fic-position < increm)increm = taille_fic-position ;
    }

    WriteFile(Hfic_dst,buffer,taille_fic,&copiee,0);
    HeapFree(GetProcessHeap(), 0, buffer);

    CloseHandle(Hfic_src);
    CloseHandle(Hfic_dst);
    return TRUE;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
BOOL VSSFileCopyFilefromPath(char *path_src, char *path_dst)
{
  BOOL Ok=FALSE;
  //load dll to verify if funtion exist
  if (hDLL_VSSAPI!=NULL)
  {
    char cmd_vssadmin[MAX_LINE_SIZE]="", path_src_VSS[MAX_LINE_SIZE]="";
    //create a snapshot of lecteur 2003r2/2008/2012 only
    snprintf(cmd_vssadmin,MAX_LINE_SIZE,"vssadmin create shadow /for=%c:",path_src[0]);
    system(cmd_vssadmin);

    //copy file
    unsigned int test = 400; //not good code but compatible with much systems
    do
    {
      snprintf(path_src_VSS,MAX_LINE_SIZE,"\\\\?\\GLOBALROOT\\Device\\HarddiskVolumeShadowCopy%d%s",test,&path_src[2]);

    }while (CopyFile(path_src_VSS,path_dst,FALSE) == 0 && test-->0);
    if (test > 0 )Ok = TRUE;

    //no delete snapshot
    //delete snapshot
    //snprintf(cmd_vssadmin,MAX_LINE_SIZE,"vssadmin delete shadows /for=%c: /shadow=%s",path_src[0],snapshoot_id);
    //system(cmd_vssadmin);
  }
  return Ok;
}
//------------------------------------------------------------------------------
//copy file directly from disk
BOOL DirectFileCopy(char *path_src, char *path_dst)
{
  if (path_src[0] == '\\' || path_src[0] == '/')return FALSE;
  BOOL ret = FALSE;

  //extract drive letter ex: "c:\" and verify if NTFS
  char path[4]="C:\\\0", filesystem[DEFAULT_TMP_SIZE];
  path[0] = path_src[0];
  DWORD FileFlags=0;
  char volume[MAX_LINE_SIZE]="",finalvolume[MAX_LINE_SIZE]="";

  if (GetVolumeInformation(path,NULL,0,NULL,NULL,&FileFlags,filesystem,DEFAULT_TMP_SIZE) == 0)return FALSE;

  //copy only on NTFS system
  if (!strcmp(filesystem,"NTFS") || !strcmp(filesystem,"ntfs"))
  {
    //get volume path (remove final \ of c:\)
    path[2] = 0;
    unsigned int vol_sz_name = QueryDosDevice(path,volume,MAX_LINE_SIZE);
    //only \Device\HarddiskVolume1 format (start on 1 but PhysicalDrive start on 0)
    if(vol_sz_name > 22)
    {
      snprintf(finalvolume,MAX_LINE_SIZE,"\\\\.\\PhysicalDrive%c",volume[22]-1);
      //open the device
      HANDLE hdrive = CreateFile(finalvolume,GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);
      if (hdrive != INVALID_HANDLE_VALUE)
      {
        ret = CopyFileFromMFT(hdrive, path_dst);
        CloseHandle(hdrive);
      }
    }
  }
  return ret;
}
//------------------------------------------------------------------------------
BOOL CopyFilefromPath(char *path_src, char *path_dst, BOOL direct_shadow_copy)
{
  //if (direct_shadow_copy)VSSFileCopyFilefromPath(path_src, path_dst);//by (VSS) Shadow copy
  //else
  {
    if (!CreateFileCopyFilefromPath(path_src, path_dst))//by share read
    {
      if(CopyFile(path_src,path_dst,FALSE)) return TRUE;  //by sys copy
      else
      {
        //or by (VSS) Shadow copy
        //VSSFileCopyFilefromPath(path_src, path_dst);
        return FALSE;
      }
    }else return TRUE;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
//convert \Device\HarddiskVolume1\ to reel path
BOOL ConvertDosPathtoReel(char *path, unsigned int max_sz_path)
{
  char tmp_path[MAX_LINE_SIZE],tmp_pathfinal[MAX_LINE_SIZE];
  if (path[0] == '\\' && path[1] != '\\')
  {
    charToLowChar(path);

    //for each system file
    char tmp[MAX_LINE_SIZE], let[3]="C:";
    int i,nblecteurs = GetLogicalDriveStrings(MAX_LINE_SIZE,tmp);
    //search
    for (i=0;i<nblecteurs;i+=4)
    {
      let[0]      = tmp[i]; //only 'C:' not 'C:\'
      tmp_path[0] = 0;
      if (QueryDosDevice(let,tmp_path,MAX_LINE_SIZE) > 0)
      {
        if (Contient(path,charToLowChar(tmp_path)))//ok ^^
        {
          snprintf(tmp_pathfinal,MAX_LINE_SIZE,"%s",path);
          snprintf(path,max_sz_path,"%c:\%s",let[0],tmp_pathfinal+strlen(tmp_path));
          return TRUE;
        }
      }
    }
  }else return TRUE; //also OK
  return FALSE;
}
//------------------------------------------------------------------------------
char LastZIPAdd[MAX_LINE_SIZE];

BOOL customAddSrc(register TZIP *tzip, const void *destname, const void *src, DWORD len, DWORD flags)
{
  if (!strcmp(LastZIPAdd,destname)) return FALSE;
  else snprintf(LastZIPAdd,MAX_LINE_SIZE,"%s",destname);

  //check if add is possible or copy file in local, add it and remove the copy !
  if (addSrc(tzip,  destname, src,len, flags) == 0) return TRUE;
  else
  {
    //extract filename
    char file[MAX_PATH]="", currentpath[MAX_LINE_SIZE]="", path[MAX_LINE_SIZE]="";
    //snprintf(path,MAX_LINE_SIZE,"%s\\TEMP_FILE_T",GetLocalPath(currentpath, MAX_LINE_SIZE));
    snprintf(path,MAX_LINE_SIZE,"%s\\%s",GetLocalPath(currentpath, MAX_LINE_SIZE),extractFileFromPath(src, file, MAX_PATH));

    //copy file in the local path
    CopyFilefromPath(src, path, FALSE);

    //add file on ZIP
    BOOL ret = FALSE;
    if (addSrc(tzip,  destname, path,len, flags) == 0) ret = TRUE;
    else ret = FALSE;

    //remove local file copied !
    DeleteFile(path);

    return ret;
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI BackupRegFile(LPVOID lParam)
{
  //choix du répertoire de destination ^^
  BROWSEINFO browser;
  LPITEMIDLIST lip;
  char tmp[MAX_LINE_SIZE]      = "";
  char cmd[MAX_LINE_SIZE]   	 = "";
  browser.hwndOwner       = h_main;
  browser.pidlRoot        = 0;
  browser.lpfn            = 0;
  browser.iImage          = 0;
  browser.ulFlags         = BIF_NEWDIALOGSTYLE; //you can create new directories
  browser.lParam          = 0;
  browser.pszDisplayName  = tmp;
  browser.lpszTitle       = cps[TXT_SAVE_TO].c;
  lip                     = SHBrowseForFolder(&browser);
  if (lip != NULL)
  {
    SHGetPathFromIDList(lip,tmp);
    if (*tmp != '\0')
    {
      SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Copy registry files in progress...");
      //save all registry
      //http://msdn.microsoft.com/en-us/library/windows/desktop/ms724877%28v=vs.85%29.aspx
      //Config hardware volatile keys
      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKLM\\HARDWARE \"%s\\HARDWARE_volatile_key\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
      //SAM
      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKLM\\SAM \"%s\\SAM\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
      //Security
      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKLM\\SECURITY \"%s\\SECURITY\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
      //Software
      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKLM\\SOFTWARE \"%s\\SOFTWARE\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
      //System
      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKLM\\SYSTEM \"%s\\SYSTEM\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
      //Classes.dat
      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKCR \"%s\\CLASSES.dat\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
      //Default
      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKU\\.DEFAULT \"%s\\DEFAULT\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);

      snprintf(cmd,MAX_LINE_SIZE,"SAVE HKCU \"%s\\HKCU_NTUSER.DAT\"",tmp);
      ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);

      //Read current users loaded
      HKEY CleTmp;
      if (RegOpenKey(HKEY_USERS,"",&CleTmp)==ERROR_SUCCESS)
      {
        DWORD nbSubKey = 0;
        if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
        {
          DWORD i, key_size;
          char key[MAX_LINE_SIZE];
          for (i=0;i<nbSubKey;i++)
          {
            key_size  = MAX_LINE_SIZE;
            key[0]    = 0;
            if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,0)==ERROR_SUCCESS)
            {
              if (key_size > 20 && Contient(key,"_Classes") == 0)
              {
                snprintf(cmd,MAX_LINE_SIZE,"SAVE HKU\\%s \"%s\\HKU_%s_NTUSER.DAT\"",key,tmp,key);
                ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
              }
            }
          }
        }
        RegCloseKey(CleTmp);
      }

      //current users & users

      CopyRegistryUSERmToPath(tmp, "C:\\Documents and Settings\\*.*", "C:\\Documents and Settings");
      CopyRegistryUSERmToPath(tmp, "D:\\Documents and Settings\\*.*", "D:\\Documents and Settings");
      CopyRegistryUSERmToPath(tmp, "E:\\Documents and Settings\\*.*", "E:\\Documents and Settings");

      CopyRegistryUSERmToPath(tmp, "C:\\Users\\*.*", "C:\\Users");
      CopyRegistryUSERmToPath(tmp, "D:\\Users\\*.*", "D:\\Users");
      CopyRegistryUSERmToPath(tmp, "E:\\Users\\*.*", "E:\\Users");

      SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Copy registry file's done !!!");
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"");
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
void CopyFileFromTo(char *from, char *to, unsigned int ext_size)
{
  WIN32_FIND_DATA data;
  char src[MAX_LINE_SIZE], dst[MAX_LINE_SIZE];

  if (ext_size == 0)
  {
    //get filename !
    char *c = from;
    while (*c++);
    while (*c != '\\' && *c != '/')c--;
    c++;

    snprintf(dst,MAX_LINE_SIZE,"%s%s",to,c);
    CopyFilefromPath(from, dst, FALSE);
    return;
  }

  //énumération des fichier
  HANDLE hfic = FindFirstFile(from, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
      else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}
      else
      {
        strcpy(src,from);
        src[strlen(src)-ext_size] =0;
        strncat(src,data.cFileName,MAX_LINE_SIZE);

        snprintf(dst,MAX_LINE_SIZE,"%s%s",to,data.cFileName);

        //copy
        CopyFilefromPath(src, dst, FALSE);
      }
    }while(FindNextFile (hfic,&data) != 0);
    FindClose(hfic);
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI BackupEvtFile(LPVOID lParam)
{
  //choix du répertoire de destination ^^
  BROWSEINFO browser;
  LPITEMIDLIST lip;
  char tmp[MAX_LINE_SIZE+1]    = "";
  char path[MAX_LINE_SIZE]     = "",
       path2[MAX_LINE_SIZE]    = "";
  browser.hwndOwner       = h_main;
  browser.pidlRoot        = 0;
  browser.lpfn            = 0;
  browser.iImage          = 0;
  browser.ulFlags         = BIF_NEWDIALOGSTYLE; //permet la création d'un dossier
  browser.lParam          = 0;
  browser.pszDisplayName  = tmp;  //résultat ici
  browser.lpszTitle       = cps[TXT_SAVE_TO].c;
  lip                     = SHBrowseForFolder(&browser);
  if (lip != NULL)
  {
    SHGetPathFromIDList(lip,tmp);
    if (*tmp != '\0')
    {
      strncat(tmp,"\\\0",MAX_LINE_SIZE);

      //récupération du chemin du système
      if(ReadValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "SystemRoot", path, MAX_LINE_SIZE))
      {
        if (path[0]!=0)
        {
          //on copie les fichier evt
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\config\\*.evt",path);
          CopyFileFromTo(path2, tmp, 5);

          //le fichiers evtx
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\*.evtx",path);
          CopyFileFromTo(path2, tmp, 6);
          //if bug !
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\Application.evtx",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\HardwareEvents.evtx",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\Internet Explorer.evtx",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\Windows PowerShell.evtx",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\System.evtx",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\Setup.evtx",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\Security.evtx",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\OAlerts.evtx",path);CopyFileFromTo(path2, tmp, 0);

          //other sys logs
          snprintf(path2,MAX_LINE_SIZE,"%s\\WindowsUpdate.log",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\comsetup.log",path);CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\wmsetup.log",path);CopyFileFromTo(path2, tmp, 0);

          //usb history
          snprintf(path2,MAX_LINE_SIZE,"%s\\setupapi.log",path);
          CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\inf\\setupapi.app.log",path);
          CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\inf\\setupapi.dev.log",path);
          CopyFileFromTo(path2, tmp, 0);

          //firewall history
          snprintf(path2,MAX_LINE_SIZE,"%s\\pfirewall.log",path);
          CopyFileFromTo(path2, tmp, 0);
          snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\logfiles\\firewall\\pfirewall.log",path);
          CopyFileFromTo(path2, tmp, 0);
        }else //default path
        {
          //system
          CopyFileFromTo("c:\\windows\\system32\\config\\*.evt", tmp, 5);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\*.evtx", tmp, 6);
          //bug cases
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Application.evtx", tmp, 0);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\HardwareEvents.evtx", tmp, 0);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Internet Explorer.evtx", tmp, 0);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Windows PowerShell.evtx", tmp, 0);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\System.evtx", tmp, 0);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Setup.evtx", tmp, 0);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Security.evtx", tmp, 0);
          CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\OAlerts.evtx", tmp, 0);


          CopyFileFromTo("c:\\windows\\WindowsUpdate.log", tmp, 0);
          CopyFileFromTo("c:\\windows\\comsetup.log", tmp, 0);
          CopyFileFromTo("c:\\windows\\wmsetup.log", tmp, 0);

          //usb
          CopyFileFromTo("c:\\windows\\setupapi.log", tmp, 0); //2000,xp,2003
          CopyFileFromTo("c:\\windows\\inf\\setupapi.app.log", tmp, 0);//after
          CopyFileFromTo("c:\\windows\\inf\\setupapi.dev.log", tmp, 0);

          //firewall history
          CopyFileFromTo("c:\\windows\\pfirewall.log", tmp, 0);//after
          CopyFileFromTo("c:\\windows\\system32\\logfiles\\firewall\\pfirewall.log", tmp, 0);
        }
      }else //default pathtmp,
      {
        //system
        CopyFileFromTo("c:\\windows\\system32\\config\\*.evt", tmp, 5);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\*.evtx", tmp, 6);
        //bug cases
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Application.evtx", tmp, 0);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\HardwareEvents.evtx", tmp, 0);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Internet Explorer.evtx", tmp, 0);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Windows PowerShell.evtx", tmp, 0);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\System.evtx", tmp, 0);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Setup.evtx", tmp, 0);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\Security.evtx", tmp, 0);
        CopyFileFromTo("c:\\windows\\system32\\winevt\\Logs\\OAlerts.evtx", tmp, 0);

        CopyFileFromTo("c:\\windows\\WindowsUpdate.log", tmp, 0);
        CopyFileFromTo("c:\\windows\\comsetup.log", tmp, 0);
        CopyFileFromTo("c:\\windows\\wmsetup.log", tmp, 0);

        //usb
        CopyFileFromTo("c:\\windows\\setupapi.log", tmp, 0); //2000,xp,2003
        CopyFileFromTo("c:\\windows\\inf\\setupapi.app.log", tmp, 0);//after
        CopyFileFromTo("c:\\windows\\inf\\setupapi.dev.log", tmp, 0);

        //firewall history
        CopyFileFromTo("c:\\windows\\pfirewall.log", tmp, 0);//after
        CopyFileFromTo("c:\\windows\\system32\\logfiles\\firewall\\pfirewall.log", tmp, 0);
      }

      //get users logs
      CopyAllLogFileFromMultChoicePathTo(NULL, "\\AppData\\Local\\Temp\\*.log", 5, tmp);
      CopyAllLogFileFromMultChoicePathTo(NULL, "\\AppData\\Local\\Cisco\\JabberVideo\\Logs\\*.log", 5, tmp);
      CopyAllLogFileFromMultChoicePathTo(NULL, "\\AppData\\Local\\mRemoteNG\\*.log", 5, tmp);

      SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Copy event file's done !!!");
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI BackupNTDIS(LPVOID lParam)
{
  char file[MAX_LINE_SIZE]="";
  GenerateNameToSave(file, MAX_LINE_SIZE, "ntds.dit");
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_main;
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_LINE_SIZE;
  ofn.lpstrFilter ="*.dit \0*.dit\0*.* \0*.*\0";
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
  ofn.lpstrDefExt ="dit\0";

  if (GetSaveFileName(&ofn)==TRUE)
  {
    //get ntdis.dit system path
    char path[MAX_LINE_SIZE]     = "";
      if(ReadValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\NTDS\\Parameters", "DSA Database File", path, MAX_LINE_SIZE))
      {
        if (path[0]!=0)
        {
          ReplaceEnv("WINDIR",path,MAX_LINE_SIZE);
          CopyFilefromPath(path, file, FALSE);
        }else CopyFilefromPath("c:\\windows\\ntds\\ntds.dit",file, FALSE);
      }else CopyFilefromPath("c:\\windows\\ntds\\ntds.dit",file, FALSE);
      SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Copy file NTDIS.DIT done !!!");
  }
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI BackupFile(LPVOID lParam)
{
  char file[MAX_LINE_SIZE]     = "",
      file_p[MAX_LINE_SIZE]    = "",
       path[MAX_LINE_SIZE]     = "",
       path2[MAX_LINE_SIZE]    = "";

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_main;
  ofn.lpstrFile = file_p;
  ofn.nMaxFile = MAX_LINE_SIZE;
  ofn.lpstrFilter = "*.* \0*.*\0";;
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_FILEMUSTEXIST|OFN_OVERWRITEPROMPT|OFN_EXPLORER|OFN_SHOWHELP;
  ofn.lpstrDefExt ="*.*";
  if (GetOpenFileName(&ofn)==TRUE)
  {
    //get path to save
    BROWSEINFO browser;
    LPITEMIDLIST lip;
    browser.hwndOwner       = h_main;
    browser.pidlRoot        = 0;
    browser.lpfn            = 0;
    browser.iImage          = 0;
    browser.ulFlags         = BIF_NEWDIALOGSTYLE; //permet la création d'un dossier
    browser.lParam          = 0;
    browser.pszDisplayName  = path;  //résultat ici
    browser.lpszTitle       = cps[TXT_SAVE_TO].c;
    lip                     = SHBrowseForFolder(&browser);
    if (lip != NULL)
    {
      if (SHGetPathFromIDList(lip,path))
      {
        snprintf(path2,MAX_LINE_SIZE,"%s\\%s",path,extractFileFromPath(file_p, file, MAX_LINE_SIZE));

        if(DirectFileCopy(file_p, path)) SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Copy file done !!!");
        else CopyFilefromPath(file_p,path, FALSE);
      }
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
char* GetLocalPath(char *path, unsigned int sizeMax)
{
  char *c = path+GetModuleFileName(0, path,sizeMax);
  while(*c != '\\') c--;
  *c = 0;
  return path;
}
//------------------------------------------------------------------------------
void CopyEvtToZIP(void *hz,char *path,char*current_path, char*computername,HANDLE MyhFile)
{
  char tmp_path[MAX_LINE_SIZE],path2[MAX_LINE_SIZE],path3[MAX_LINE_SIZE],file[MAX_LINE_SIZE];
  if (path == NULL)snprintf(tmp_path,MAX_LINE_SIZE,"%s\\system32\\config\\*.evt",path);
  else snprintf(tmp_path,MAX_LINE_SIZE,"c:\\Windows\\system32\\config\\*.evt");

  WIN32_FIND_DATA data;
  DWORD copiee;
  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (path == NULL)snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\config\\%s",path,data.cFileName);
      else snprintf(path2,MAX_LINE_SIZE,"c:\\Windows\\system32\\config\\%s",data.cFileName);

      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        WriteFile(MyhFile,path2,strlen(path2),&copiee,0);
        WriteFile(MyhFile,"\r\n",2,&copiee,0);
      }

      snprintf(path3,MAX_LINE_SIZE,"%s\\%s",current_path,data.cFileName);
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)path3);
      if(CopyFilefromPath(path2, path3, FALSE))
      {
        snprintf(file,MAX_LINE_SIZE,"%s\\eventlog\\%s",computername,data.cFileName);
        customAddSrc((TZIP *)hz,(void *)file, (void *)path3,0, 2);
        DeleteFile(path3);
      }
    }while(FindNextFile (hfic,&data) !=0);
    FindClose(hfic);
  }
}
//------------------------------------------------------------------------------
void CopyEvtxToZIP(void *hz,char *path,char*current_path, char*computername,HANDLE MyhFile)
{
  char tmp_path[MAX_LINE_SIZE],path2[MAX_LINE_SIZE],path3[MAX_LINE_SIZE],file[MAX_LINE_SIZE];
  if (path == NULL)snprintf(tmp_path,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\*.evtx",path);
  else snprintf(tmp_path,MAX_LINE_SIZE,"c:\\Windows\\system32\\winevt\\Logs\\*.evtx");

  WIN32_FIND_DATA data;
  DWORD copiee;
  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (path == NULL)snprintf(path2,MAX_LINE_SIZE,"%s\\system32\\winevt\\Logs\\%s",path,data.cFileName);
      else snprintf(path2,MAX_LINE_SIZE,"c:\\Windows\\system32\\winevt\\Logs\\%s",data.cFileName);

      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        WriteFile(MyhFile,path2,strlen(path2),&copiee,0);
        WriteFile(MyhFile,"\r\n",2,&copiee,0);
      }

      snprintf(path3,MAX_LINE_SIZE,"%s\\%s",current_path,data.cFileName);
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)path3);
      if(CopyFilefromPath(path2, path3, FALSE))
      {
        snprintf(file,MAX_LINE_SIZE,"%s\\eventlog\\%s",computername,data.cFileName);
        customAddSrc((TZIP *)hz,(void *)file, (void *)path3,0, 2);
        DeleteFile(path3);
      }
    }while(FindNextFile (hfic,&data) !=0);
    FindClose(hfic);
  }
}
//------------------------------------------------------------------------------
void CopyPrefetchToZIP(void *hz,char *path, char*current_path, char*computername,HANDLE MyhFile)
{
  char tmp_path[MAX_LINE_SIZE],path2[MAX_LINE_SIZE],path3[MAX_LINE_SIZE],file[MAX_LINE_SIZE];
  if (path == NULL)snprintf(tmp_path,MAX_LINE_SIZE,"%s\\Prefetch\\*.pf",path);
  else snprintf(tmp_path,MAX_LINE_SIZE,"c:\\Windows\\Prefetch\\*.pf");

  WIN32_FIND_DATA data;
  DWORD copiee;
  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (path == NULL)snprintf(path2,MAX_LINE_SIZE,"%s\\Prefetch\\%s",path,data.cFileName);
      else snprintf(path2,MAX_LINE_SIZE,"c:\\Windows\\Prefetch\\%s",data.cFileName);

      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        WriteFile(MyhFile,path2,strlen(path2),&copiee,0);
        WriteFile(MyhFile,"\r\n",2,&copiee,0);
      }

      snprintf(path3,MAX_LINE_SIZE,"%s\\%s",current_path,data.cFileName);
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)path3);
      if(CopyFilefromPath(path2, path3, FALSE))
      {
        snprintf(file,MAX_LINE_SIZE,"%s\\Prefetch\\%s",computername,data.cFileName);
        customAddSrc((TZIP *)hz,(void *)file, (void *)path3,0, 2);
        DeleteFile(path3);
      }
    }while(FindNextFile (hfic,&data) !=0);
    FindClose(hfic);
  }
}
//------------------------------------------------------------------------------
void CopyJobToZIP(void *hz,char *path, char*current_path, char*computername,HANDLE MyhFile)
{
  char tmp_path[MAX_LINE_SIZE],path2[MAX_LINE_SIZE],path3[MAX_LINE_SIZE],file[MAX_LINE_SIZE];
  if (path == NULL)snprintf(tmp_path,MAX_LINE_SIZE,"%s\\Tasks\\*.job",path);
  else snprintf(tmp_path,MAX_LINE_SIZE,"c:\\Windows\\Tasks\\*.job");

  WIN32_FIND_DATA data;
  DWORD copiee;
  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (path == NULL)snprintf(path2,MAX_LINE_SIZE,"%s\\Tasks\\%s",path,data.cFileName);
      else snprintf(path2,MAX_LINE_SIZE,"c:\\Windows\\Tasks\\%s",data.cFileName);

      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        WriteFile(MyhFile,path2,strlen(path2),&copiee,0);
        WriteFile(MyhFile,"\r\n",2,&copiee,0);
      }

      snprintf(path3,MAX_LINE_SIZE,"%s\\%s",current_path,data.cFileName);
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)path3);
      if(CopyFilefromPath(path2, path3, FALSE))
      {
        snprintf(file,MAX_LINE_SIZE,"%s\\Tasks\\%s",computername,data.cFileName);
        customAddSrc((TZIP *)hz,(void *)file, (void *)path3,0, 2);
        DeleteFile(path3);
      }
    }while(FindNextFile (hfic,&data) !=0);
    FindClose(hfic);
  }
}
//------------------------------------------------------------------------------
void CopyFileToZIP(void *hz,char *fileinzip, char *sysfile, char*computername,HANDLE MyhFile)
{
  if(FileExist(sysfile))
  {
    char file[MAX_LINE_SIZE];
    DWORD copiee;
    snprintf(file,MAX_LINE_SIZE,"%s\\%s",computername,fileinzip);
    customAddSrc((TZIP *)hz,  (void *)file, (void *)sysfile,0, 2);

    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)sysfile);

    snprintf(file,MAX_LINE_SIZE,"%s\r\n",sysfile);
    WriteFile(MyhFile,file,strlen(file),&copiee,0);
  }
}
//------------------------------------------------------------------------------
void CopyAllLogFileFromMultChoicePath(void *hz, char*computername, HANDLE MyhFile, char *start_path_file, char *end_path_file, unsigned int ext_sz)
{
  WIN32_FIND_DATA data, wfd;
  char tmp[MAX_LINE_SIZE]="",file_tmp[MAX_LINE_SIZE]="",zip_file_tmp[MAX_LINE_SIZE]="";
  HANDLE hfic;

  if (start_path_file != NULL)
  {
    char tmp_search[MAX_LINE_SIZE]="";
    snprintf(tmp_search, MAX_LINE_SIZE, "%s*.*",start_path_file);
    HANDLE hdir = FindFirstFile(tmp_search, &wfd);
    if (hdir != INVALID_HANDLE_VALUE)
    {
      do
      {
        if(wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || wfd.cFileName[1] == '.')){}
        else if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          snprintf(tmp,MAX_LINE_SIZE,"%s%s%s",start_path_file,wfd.cFileName,end_path_file);
          hfic = FindFirstFile(tmp, &data);
          if (hfic != INVALID_HANDLE_VALUE)
          {
            do
            {
              if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
              else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}
              else
              {
                snprintf(file_tmp,MAX_LINE_SIZE,"%s",tmp);
                file_tmp[strlen(file_tmp)-ext_sz] = 0;
                strncat(file_tmp,data.cFileName,MAX_LINE_SIZE-strlen(file_tmp));
                strncat(file_tmp,"\0",MAX_LINE_SIZE-strlen(file_tmp));

                snprintf(zip_file_tmp,MAX_LINE_SIZE,"logs\\%s_%s",wfd.cFileName,data.cFileName);
                CopyFileToZIP(hz, zip_file_tmp, file_tmp, computername, MyhFile);
              }
            }while(FindNextFile (hfic,&data) !=0 && start_scan);
            FindClose(hfic);
          }
        }
      }while(FindNextFile(hdir,&wfd) !=0 && start_scan);
      FindClose(hdir);
    }
  }else
  {
    //get user local path from registry !
    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList\*\ ProfileImagePath
    HKEY CleTmp;
    if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\",&CleTmp)==ERROR_SUCCESS)
    {
      DWORD i,nbSubKey = 0;
      if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
      {
        char key_path[MAX_PATH], key[MAX_PATH], key2[MAX_PATH];
        DWORD key_size;

        for (i=0;i<nbSubKey;i++)
        {
          key[0] = 0;
          key_size = MAX_PATH;

          if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,0)==ERROR_SUCCESS)
          {
            snprintf(key_path,MAX_PATH,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s",key);
            key2[0] = 0;
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"ProfileImagePath",key2, DEFAULT_TMP_SIZE);

            //get files
            snprintf(tmp,MAX_LINE_SIZE,"%s%s",key2,end_path_file);
            hfic = FindFirstFile(tmp, &data);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
                else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}
                else
                {
                  snprintf(file_tmp,MAX_LINE_SIZE,"%s",tmp);
                  file_tmp[strlen(file_tmp)-ext_sz] = 0;
                  strncat(file_tmp,data.cFileName,MAX_LINE_SIZE-strlen(file_tmp));
                  strncat(file_tmp,"\0",MAX_LINE_SIZE-strlen(file_tmp));

                  snprintf(zip_file_tmp,MAX_LINE_SIZE,"logs\\%s_%s",key,data.cFileName);
                  CopyFileToZIP(hz, zip_file_tmp, file_tmp, computername, MyhFile);
                }
              }while(FindNextFile (hfic,&data) !=0 && start_scan);
              FindClose(hfic);
            }
          }
        }
      }
      RegCloseKey(CleTmp);
    }
  }
}
//------------------------------------------------------------------------------
void CopyAllLogFileFromMultChoicePathTo(char *start_path_file, char *end_path_file, unsigned int ext_sz, char *topath)
{
  WIN32_FIND_DATA data, wfd;
  char tmp[MAX_LINE_SIZE]="",file_tmp[MAX_LINE_SIZE]="";
  HANDLE hfic;

  if (start_path_file != NULL)
  {
    char tmp_search[MAX_LINE_SIZE]="";
    snprintf(tmp_search, MAX_LINE_SIZE, "%s*.*",start_path_file);
    HANDLE hdir = FindFirstFile(tmp_search, &wfd);
    if (hdir != INVALID_HANDLE_VALUE)
    {
      do
      {
        if(wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || wfd.cFileName[1] == '.')){}
        else if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
          snprintf(tmp,MAX_LINE_SIZE,"%s%s%s",start_path_file,wfd.cFileName,end_path_file);
          hfic = FindFirstFile(tmp, &data);
          if (hfic != INVALID_HANDLE_VALUE)
          {
            do
            {
              if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
              else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}
              else
              {
                snprintf(file_tmp,MAX_LINE_SIZE,"%s",tmp);
                file_tmp[strlen(file_tmp)-ext_sz] = 0;
                strncat(file_tmp,data.cFileName,MAX_LINE_SIZE-strlen(file_tmp));
                strncat(file_tmp,"\0",MAX_LINE_SIZE-strlen(file_tmp));

                CopyFileFromTo(file_tmp, topath, 0);
              }
            }while(FindNextFile (hfic,&data) !=0 && start_scan);
            FindClose(hfic);
          }
        }
      }while(FindNextFile(hdir,&wfd) !=0 && start_scan);
      FindClose(hdir);
    }
  }else
  {
    //get user local path from registry !
    // HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\CurrentVersion\ProfileList\*\ ProfileImagePath
    HKEY CleTmp;
    if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\",&CleTmp)==ERROR_SUCCESS)
    {
      DWORD i,nbSubKey = 0;
      if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
      {
        char key_path[MAX_PATH], key[MAX_PATH], key2[MAX_PATH];
        DWORD key_size;

        for (i=0;i<nbSubKey;i++)
        {
          key[0] = 0;
          key_size = MAX_PATH;

          if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,0)==ERROR_SUCCESS)
          {
            snprintf(key_path,MAX_PATH,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s",key);
            key2[0] = 0;
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"ProfileImagePath",key2, DEFAULT_TMP_SIZE);

            //get files
            snprintf(tmp,MAX_LINE_SIZE,"%s%s",key2,end_path_file);
            hfic = FindFirstFile(tmp, &data);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
                else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}
                else
                {
                  snprintf(file_tmp,MAX_LINE_SIZE,"%s",tmp);
                  file_tmp[strlen(file_tmp)-ext_sz] = 0;
                  strncat(file_tmp,data.cFileName,MAX_LINE_SIZE-strlen(file_tmp));
                  strncat(file_tmp,"\0",MAX_LINE_SIZE-strlen(file_tmp));

                  CopyFileFromTo(file_tmp, topath, 0);
                }
              }while(FindNextFile (hfic,&data) !=0 && start_scan);
              FindClose(hfic);
            }
          }
        }
      }
      RegCloseKey(CleTmp);
    }
  }
}
//------------------------------------------------------------------------------
void CopyAllWinLogToZIP(void *hz,char *path,char*computername,HANDLE MyhFile)
{
  if (path == NULL)
  {
    CopyFileToZIP(hz,"logs\\setupapi.log", "c:\\windows\\setupapi.log", computername, MyhFile);
    CopyFileToZIP(hz,"logs\\setupapi.app.log", "c:\\windows\\inf\\setupapi.app.log", computername, MyhFile);
    CopyFileToZIP(hz,"logs\\setupapi.dev.log", "c:\\windows\\inf\\setupapi.dev.log", computername, MyhFile);

    CopyFileToZIP(hz,"logs\\Windowsupdate.log", "c:\\windows\\WindowsUpdate.log", computername, MyhFile);
    CopyFileToZIP(hz,"logs\\comsetup.log", "c:\\windows\\comsetup.log", computername, MyhFile);
    CopyFileToZIP(hz,"logs\\wmsetup.log", "c:\\windows\\wmsetup.log", computername, MyhFile);

    CopyFileToZIP(hz,"logs\\pfirewall.log", "c:\\windows\\pfirewall.log", computername, MyhFile);
    CopyFileToZIP(hz,"logs\\pfirewall.log", "c:\\windows\\system32\\logfiles\\firewall\\pfirewall.log", computername, MyhFile);

    //all log from temporary directories
    CopyAllLogFileFromMultChoicePath(hz, computername, MyhFile, NULL, "\\AppData\\Local\\Temp\\*.log", 5);
    CopyAllLogFileFromMultChoicePath(hz, computername, MyhFile, NULL, "\\AppData\\Local\\Cisco\\JabberVideo\\Logs\\*.log", 5);
    CopyAllLogFileFromMultChoicePath(hz, computername, MyhFile, NULL, "\\AppData\\Local\\mRemoteNG\\*.log", 5);

  }else
  {
    char tmp_path[MAX_LINE_SIZE];
    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\setupapi.log",path);
    CopyFileToZIP(hz,"logs\\setupapi.log", tmp_path, computername, MyhFile);
    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\inf\\setupapi.app.log",path);
    CopyFileToZIP(hz,"logs\\setupapi.log", tmp_path, computername, MyhFile);
    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\inf\\setupapi.dev.log",path);
    CopyFileToZIP(hz,"logs\\setupapi.dev.log", tmp_path, computername, MyhFile);

    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\Windowsupdate.log",path);
    CopyFileToZIP(hz,"logs\\Windowsupdate.log", tmp_path, computername, MyhFile);
    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\comsetup.log",path);
    CopyFileToZIP(hz,"logs\\comsetup.log", tmp_path, computername, MyhFile);
    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\wmsetup.log",path);
    CopyFileToZIP(hz,"logs\\wmsetup.log", tmp_path, computername, MyhFile);

    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\pfirewall.log",path);
    CopyFileToZIP(hz,"logs\\pfirewall.log", tmp_path, computername, MyhFile);
    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\system32\\logfiles\\firewall\\pfirewall.log",path);
    CopyFileToZIP(hz,"logs\\pfirewall.log", tmp_path, computername, MyhFile);

    if ((path[1] == ':') &&(path[4] == 'I' || path[4] == 'i') && (path[3] == 'W' || path[3] == 'w'))snprintf(tmp_path,MAX_LINE_SIZE,"%c:\\Users\\",path[0]);
    else snprintf(tmp_path,MAX_LINE_SIZE,"%s\\Users\\",path);

    CopyAllLogFileFromMultChoicePath(hz, computername, MyhFile, tmp_path, "\\AppData\\Local\\Temp\\*.log", 5);
    CopyAllLogFileFromMultChoicePath(hz, computername, MyhFile, tmp_path, "\\AppData\\Local\\Cisco\\JabberVideo\\Logs\\*.log", 5);
    CopyAllLogFileFromMultChoicePath(hz, computername, MyhFile, tmp_path, "\\AppData\\Local\\mRemoteNG\\*.log", 5);
  }
}
//------------------------------------------------------------------------------
void CopyNTDISToZIP(void *hz,char *path, char*current_path,char*computername,HANDLE MyhFile)
{
  char local_path[MAX_LINE_SIZE];
  snprintf(local_path,MAX_LINE_SIZE,"%s\\NTDIS.DIT",current_path);
  char file[MAX_LINE_SIZE];
  snprintf(file,MAX_LINE_SIZE,"%s\\AD\\NTDIS.DIT",computername);
  DWORD copiee;

  if (path == NULL)
  {
    if(CopyFilefromPath("c:\\windows\\ntds\\ntds.dit", local_path, FALSE))
    {
      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        WriteFile(MyhFile,"c:\\windows\\ntds\\ntds.dit\r\n",strlen("c:\\windows\\ntds\\ntds.dit\r\n"),&copiee,0);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"c:\\windows\\ntds\\ntds.dit");
      }
      customAddSrc((TZIP *)hz, (void *)file,(void *)local_path, 0, 2);
      DeleteFile(local_path);
    }
  }else
  {
    if(CopyFilefromPath(path, local_path, FALSE))
    {
      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        WriteFile(MyhFile,path,strlen(path),&copiee,0);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)path);
        WriteFile(MyhFile,"\r\n",2,&copiee,0);
      }
      customAddSrc((TZIP *)hz, (void *)file,(void *)local_path, 0, 2);
      DeleteFile(local_path);
    }
  }
}
//------------------------------------------------------------------------------
void CopyRegistryToZIP(void *hz, char*local_path,char*computername,HANDLE MyhFile)
{
  char tmp_path[MAX_LINE_SIZE];
  char file[MAX_LINE_SIZE];
  DWORD copiee;

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\HARDWARE_volatile_key",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKLM\\HARDWARE \"%s\\HARDWARE_volatile_key\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\HARDWARE_volatile_key",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\SAM",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKLM\\SAM \"%s\\SAM\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\SAM",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\SECURITY",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKLM\\SECURITY \"%s\\SECURITY\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\SECURITY",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\SOFTWARE",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKLM\\SOFTWARE \"%s\\SOFTWARE\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\SOFTWARE",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\SYSTEM",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKLM\\SYSTEM \"%s\\SYSTEM\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\SYSTEM",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\CLASSES.dat",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKCR \"%s\\CLASSES.dat\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\CLASSES.dat",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\DEFAULT",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKU\\.DEFAULT \"%s\\DEFAULT\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\DEFAULT",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);

  snprintf(file,MAX_LINE_SIZE,"%s\\Registry\\NTUSER.DAT",computername);
  snprintf(tmp_path,MAX_LINE_SIZE,"REG SAVE HKCU \"%s\\NTUSER.DAT\"",local_path);
  if (MyhFile != INVALID_HANDLE_VALUE)
  {
    WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
    WriteFile(MyhFile,"\r\n",2,&copiee,0);
  }
  system(tmp_path);
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\NTUSER.DAT",local_path);
  customAddSrc((TZIP *)hz,  (void *)file, (void *)tmp_path,0, 2);
  DeleteFile(tmp_path);
}
//------------------------------------------------------------------------------
void CopyRegistryUSERmToZIP(void *hz, char*local_path,char*computername,HANDLE MyhFile, char *tmp_path, char *path)
{
  char tmp_path2[MAX_LINE_SIZE],tmp_path_dst[MAX_LINE_SIZE];
  WIN32_FIND_DATA data;
  DWORD copiee;
  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
      else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        //1
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\NTUSER.DAT",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Registry\\%s_NTUSER.DAT",computername,data.cFileName);
        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
        if (MyhFile != INVALID_HANDLE_VALUE)
        {
          WriteFile(MyhFile,tmp_path2,strlen(tmp_path2),&copiee,0);
          SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);
          WriteFile(MyhFile,"\r\n",2,&copiee,0);
        }

        //2
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Application Data\\Microsoft\\Windows\\UsrClass.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Registry\\%s_UsrClass.dat",computername,data.cFileName);
        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
        if (MyhFile != INVALID_HANDLE_VALUE)
        {
          WriteFile(MyhFile,tmp_path2,strlen(tmp_path2),&copiee,0);
          SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);
          WriteFile(MyhFile,"\r\n",2,&copiee,0);
        }

        //3
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\AppData\\Local\\Microsoft\\Windows\\UsrClass.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Registry\\%s_UsrClass.dat",computername,data.cFileName);
        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
        if (MyhFile != INVALID_HANDLE_VALUE)
        {
          WriteFile(MyhFile,tmp_path2,strlen(tmp_path2),&copiee,0);
          SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);
          WriteFile(MyhFile,"\r\n",2,&copiee,0);
        }
      }
    }while(FindNextFile (hfic,&data) !=0);
    FindClose(hfic);
  }
}//------------------------------------------------------------------------------
void CopyRegistryUSERmToPath(char*pathToCopy, char *tmp_path, char *path)
{
  char tmp_path2[MAX_LINE_SIZE],tmp_path_dst[MAX_LINE_SIZE];
  WIN32_FIND_DATA data;
  DWORD copiee;
  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
      else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        //1
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\NTUSER.DAT",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\%s_NTUSER.DAT",pathToCopy,data.cFileName);
        CopyFile(tmp_path2,tmp_path_dst,FALSE);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);

        //2
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Application Data\\Microsoft\\Windows\\UsrClass.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\%s_UsrClass.dat",pathToCopy,data.cFileName);
        CopyFile(tmp_path2,tmp_path_dst,FALSE);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);

        //3
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\AppData\\Local\\Microsoft\\Windows\\UsrClass.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\%s_UsrClass.dat",pathToCopy,data.cFileName);
        CopyFile(tmp_path2,tmp_path_dst,FALSE);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);
      }
    }while(FindNextFile (hfic,&data) !=0);
    FindClose(hfic);
  }
}
//------------------------------------------------------------------------------
void CopyRegistryUSERToZIP(void *hz, char*local_path,char*computername,HANDLE MyhFile)
{
  CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, "C:\\Documents and Settings\\*.*", "C:\\Documents and Settings");
  CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, "D:\\Documents and Settings\\*.*", "D:\\Documents and Settings");
  CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, "E:\\Documents and Settings\\*.*", "E:\\Documents and Settings");

  CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, "C:\\Users\\*.*", "C:\\Users");
  CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, "D:\\Users\\*.*", "D:\\Users");
  CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, "E:\\Users\\*.*", "E:\\Users");

  //hives loaded
  //Read current users loaded
  HKEY CleTmp;
  if (RegOpenKey(HKEY_USERS,"",&CleTmp)==ERROR_SUCCESS)
  {
    DWORD nbSubKey = 0;
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      DWORD i, key_size, copiee;
      char key[MAX_LINE_SIZE];
      char tmp_path_dst[MAX_LINE_SIZE], cmd[MAX_LINE_SIZE];
      for (i=0;i<nbSubKey;i++)
      {
        key_size  = MAX_LINE_SIZE;
        key[0]    = 0;
        if (RegEnumKeyEx (CleTmp,i,key,&key_size,0,0,0,0)==ERROR_SUCCESS)
        {
          if (key_size > 20 && Contient(key,"_Classes") == 0)
          {
            snprintf(cmd,MAX_LINE_SIZE,"SAVE HKU\\%s \"%s\\HKU_%s_NTUSER.DAT\"",key,local_path,key);
            ShellExecute(h_main, "open","REG",cmd,NULL,SW_HIDE);
            Sleep(2000);

            //add to ZIP
            snprintf(cmd,MAX_LINE_SIZE,"%s\\HKU_%s_NTUSER.DAT",local_path,key);
            snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Registry\\HKU_%s_NTUSER.DAT",computername,key);
            customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)cmd,0, 2);
            if (MyhFile != INVALID_HANDLE_VALUE)
            {
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"REG SAVE HKU\\%s \"%s\\HKU_%s_NTUSER.DAT\"",key,local_path,key);
              WriteFile(MyhFile,tmp_path_dst,strlen(tmp_path_dst),&copiee,0);
              SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_dst);
              WriteFile(MyhFile,"\r\n",2,&copiee,0);
            }

            //REMOVE
            DeleteFile(cmd);
          }
        }
      }
    }
    RegCloseKey(CleTmp);
  }

}
//------------------------------------------------------------------------------
void CopyMBRToZip(void *hz, char*local_path,char*computername,HANDLE MyhFile)
{
  char tmp_file_to_save[MAX_LINE_SIZE]="",zip_path_to_save[MAX_LINE_SIZE]="";

  snprintf(tmp_file_to_save,MAX_LINE_SIZE,"%s\\mbr.raw",local_path);
  if(dd("\\\\.\\PhysicalDrive0", tmp_file_to_save, 512, TRUE))
  {
    //if(FileExist(tmp_file_to_save))
    {
      snprintf(zip_path_to_save,MAX_LINE_SIZE,"%s\\Disk\\mbr.raw",computername);
      addSrc((TZIP *)hz,  (void *)zip_path_to_save, (void *)tmp_file_to_save,0, 2);
      DeleteFile(tmp_file_to_save);

      if (MyhFile != INVALID_HANDLE_VALUE)
      {
        DWORD copiee;
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"MBR");
        WriteFile(MyhFile,"MBR OK\r\n",8,&copiee,0);
      }
    }
  }
}
//------------------------------------------------------------------------------
void CopyNavigatorHistoryToZIP(void *hz, char*local_path,char*computername,HANDLE MyhFile)
{
  HKEY CleTmp   = 0;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\",&CleTmp)==ERROR_SUCCESS)
  {
    DWORD i, nbSubKey=0, key_size;
    DWORD copiee;
    char tmp_key[MAX_LINE_SIZE], tmp_key_path[MAX_LINE_SIZE], tmp_path[MAX_LINE_SIZE], tmp_path2[MAX_LINE_SIZE], tmp_path_dst[MAX_LINE_SIZE],user[MAX_LINE_SIZE];
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      //get subkey
      for(i=0;i<nbSubKey;i++)
      {
        key_size    = MAX_LINE_SIZE;
        tmp_key[0]  = 0;
        user[0]     = 0;
        if (RegEnumKeyEx (CleTmp,i,user,&key_size,0,0,0,0)==ERROR_SUCCESS)
        {
          //generate the key path
          snprintf(tmp_key_path,MAX_LINE_SIZE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s\\",user);

          //get profil path
          if (ReadValue(HKEY_LOCAL_MACHINE,tmp_key_path,"ProfileImagePath",tmp_key, MAX_LINE_SIZE))
          {
            //verify the path if %systemdrive%
            ReplaceEnv("SYSTEMDRIVE",tmp_key,MAX_LINE_SIZE);

          //---------------
          //firefox
            //search profil directory
            snprintf(tmp_key_path,MAX_PATH,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\*.default",tmp_key);
            WIN32_FIND_DATA wfd0;
            HANDLE hfic = FindFirstFile(tmp_key_path, &wfd0);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                  snprintf(tmp_path,MAX_PATH,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\content-prefs.sqlite",tmp_key,wfd0.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_content-prefs.sqlite",computername,user,wfd0.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  snprintf(tmp_path,MAX_PATH,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\cookies.sqlite",tmp_key,wfd0.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_cookies.sqlite",computername,user,wfd0.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  snprintf(tmp_path,MAX_PATH,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\places.sqlite",tmp_key,wfd0.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_places.sqlite",computername,user,wfd0.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  snprintf(tmp_path,MAX_PATH,"%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\search.sqlite",tmp_key,wfd0.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_search.sqlite",computername,user,wfd0.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }
                }
              }while(FindNextFile (hfic,&wfd0));

              FindClose(hfic);
            }

            /*snprintf(tmp_key_path,MAX_PATH,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\*.default",tmp_key);
            WIN32_FIND_DATA wfd1;
            hfic = FindFirstFile(tmp_key_path, &wfd1);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (wfd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                  //above
                  snprintf(tmp_path,MAX_PATH,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\content-prefs.sqlite",tmp_key,wfd1.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_content-prefs.sqlite",computername,user,wfd1.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  snprintf(tmp_path,MAX_PATH,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\cookies.sqlite",tmp_key,wfd1.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_cookies.sqlite",computername,user,wfd1.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  snprintf(tmp_path,MAX_PATH,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\places.sqlite",tmp_key,wfd1.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_places.sqlite",computername,user,wfd1.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  snprintf(tmp_path,MAX_PATH,"%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\search.sqlite",tmp_key,wfd1.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_PATH,"%s\\Navigator\\Firefox\\%s_%s_search.sqlite",computername,user,wfd1.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }
                }
              }while(FindNextFile (hfic,&wfd0));
              FindClose(hfic);
            }*/

            //---------------
            //chrome
            //search file in this path
            WIN32_FIND_DATA wfd;
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Local Settings\\Application Data\\Google\\Chrome\\User Data\\Default\\*.*",tmp_key);
            hfic = FindFirstFile(tmp_key_path, &wfd);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){}else
                {
                  if(wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || wfd.cFileName[1] == '.')){}
                  else
                  {
                    //test all files
                    snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\Local Settings\\Application Data\\Google\\Chrome\\User Data\\Default\\%s",tmp_key,wfd.cFileName);
                    snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\Chrome\\%s_%s",computername,user,wfd.cFileName);

                    if (FileExist(tmp_path2))
                    {
                      //add file
                      customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
                      if (MyhFile != INVALID_HANDLE_VALUE)
                      {
                        WriteFile(MyhFile,tmp_path2,strlen(tmp_path2),&copiee,0);
                        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);
                        WriteFile(MyhFile,"\r\n",2,&copiee,0);
                      }
                    }
                  }
                }
              }while(FindNextFile (hfic,&wfd) !=0);
            }
            //---------------
            //IE
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Cookies\\index.dat",tmp_key);
            snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_Cookies_index.dat",computername,user);
            if (FileExist(tmp_key_path))
            {
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }

            //other
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Microsoft\\Windows\\Cookies\\Low\\index.dat",tmp_key);
            snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_Cookies_Low_index.dat",computername,user);
            if (FileExist(tmp_key_path))
            {
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }

            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Microsoft\\Windows\\Cookies\\PrivacIE\\index.dat",tmp_key);
            snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_Cookies_PrivacIE_index.dat",computername,user);

            if (FileExist(tmp_key_path))
            {
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }

            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Internet Explorer\\DOMStore\\index.dat",tmp_key);
            snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_DOMStore_index.dat",computername,user);

            if (FileExist(tmp_key_path))
            {
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }

            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Feeds Cache\\index.dat",tmp_key);
            snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_Feeds_Cache_index.dat",computername,user);

            if (FileExist(tmp_key_path))
            {
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }

            //search other files cache
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Local Settings\\Historique\\*.*",tmp_key);
            HANDLE hfic2 = FindFirstFile(tmp_key_path, &wfd0);
            if (hfic2 != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                  if(wfd0.cFileName[0] == '.' && (wfd0.cFileName[1] == 0 || wfd0.cFileName[1] == '.'))continue;

                  snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Local Settings\\Historique\\%s\\index.dat",tmp_key,wfd0.cFileName);
                  snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_%s_Feeds_Cache_index.dat",computername,user,wfd0.cFileName);
                  if (FileExist(tmp_key_path))
                  {
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  //get file and tests it
                  WIN32_FIND_DATA wfd1;
                  snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Local Settings\\Historique\\%s\\*.*",tmp_key,wfd0.cFileName);
                  HANDLE hfic3 = FindFirstFile(tmp_key_path, &wfd1);
                  if (hfic3 == INVALID_HANDLE_VALUE)continue;
                  do
                  {
                    if (wfd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                    {
                      if(wfd1.cFileName[0] == '.' && (wfd1.cFileName[1] == 0 || wfd1.cFileName[1] == '.'))continue;
                      snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Local Settings\\Historique\\%s\\%s\\index.dat",tmp_key,wfd0.cFileName,wfd1.cFileName);
                      snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_%s_%s_index.dat",computername,user,wfd0.cFileName,wfd1.cFileName);
                      if (FileExist(tmp_key_path))
                      {
                        //add file
                        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
                        if (MyhFile != INVALID_HANDLE_VALUE)
                        {
                          WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                          SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                          WriteFile(MyhFile,"\r\n",2,&copiee,0);
                        }
                      }
                    }
                  }while(FindNextFile (hfic3,&wfd1) !=0 && start_scan);
                  FindClose(hfic3);
                }
              }while(FindNextFile (hfic2,&wfd0) !=0);
              FindClose(hfic2);
            }
          }
        }
      }
    }
    RegCloseKey(CleTmp);
  }
}
//------------------------------------------------------------------------------
void CopyChatHistoryToZIP(void *hz ,char*computername,HANDLE MyhFile)
{
  HKEY CleTmp   = 0;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\",&CleTmp)==ERROR_SUCCESS)
  {
    DWORD i, nbSubKey=0, key_size;
    DWORD copiee;
    char tmp_key[MAX_LINE_SIZE], tmp_key_path[MAX_LINE_SIZE], tmp_path[MAX_LINE_SIZE], tmp_path2[MAX_LINE_SIZE], tmp_path_dst[MAX_LINE_SIZE],user[MAX_LINE_SIZE];
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      //get subkey
      for(i=0;i<nbSubKey;i++)
      {
        key_size    = MAX_LINE_SIZE;
        tmp_key[0]  = 0;
        user[0]     = 0;
        if (RegEnumKeyEx (CleTmp,i,user,&key_size,0,0,0,0)==ERROR_SUCCESS)
        {
          //generate the key path
          snprintf(tmp_key_path,MAX_LINE_SIZE,"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\ProfileList\\%s\\",user);

          //get profil path
          if (ReadValue(HKEY_LOCAL_MACHINE,tmp_key_path,"ProfileImagePath",tmp_key, MAX_LINE_SIZE))
          {
            //verify the path if %systemdrive%
            ReplaceEnv("SYSTEMDRIVE",tmp_key,MAX_LINE_SIZE);

            //communicator
            WIN32_FIND_DATA wfd0;
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Communicator\\*.*",tmp_key);
            //snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\Local Settings\\Application Data\\Microsoft\\Communicator\\*.*",tmp_key);
            HANDLE hfic = FindFirstFile(tmp_key_path, &wfd0);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                  if(wfd0.cFileName[0] == '.' && (wfd0.cFileName[1] == 0 || wfd0.cFileName[1] == '.'))continue;
                  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Communicator\\%s\\GalContacts.db",tmp_key,wfd0.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Communicator\\%s_%s_GalContacts.db",computername,user,wfd0.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }
                }
              }while(FindNextFile (hfic,&wfd0) !=0 && start_scan);
              FindClose(hfic);
            }


            //Lync
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\Office\\*.*",tmp_key);
            hfic = FindFirstFile(tmp_key_path, &wfd0);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                  if(wfd0.cFileName[0] == '.' && (wfd0.cFileName[1] == 0 || wfd0.cFileName[1] == '.'))continue;

                  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Office\\%s\\*.*",tmp_key,wfd0.cFileName);
                  WIN32_FIND_DATA wfd1;
                  HANDLE hfic1 = FindFirstFile(tmp_path, &wfd1);
                  if (hfic1 != INVALID_HANDLE_VALUE)
                  {
                    do
                    {
                      if (wfd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                      {
                        if(wfd1.cFileName[0] == '.' && (wfd1.cFileName[1] == 0 || wfd1.cFileName[1] == '.'))continue;

                        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\Office\\%s\\%s\\GalContacts.db",tmp_key,wfd0.cFileName,wfd1.cFileName);
                        if (FileExist(tmp_path2))
                        {
                          snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Lync\\%s_%s_GalContacts.db",computername,user,wfd1.cFileName);
                          //add file
                          customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
                          if (MyhFile != INVALID_HANDLE_VALUE)
                          {
                            WriteFile(MyhFile,tmp_path2,strlen(tmp_path2),&copiee,0);
                            SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);
                            WriteFile(MyhFile,"\r\n",2,&copiee,0);
                          }
                        }
                      }
                    }while(FindNextFile (hfic1,&wfd1) !=0 && start_scan);
                    FindClose(hfic1);
                  }
                }
              }while(FindNextFile (hfic,&wfd0) !=0 && start_scan);
              FindClose(hfic);
            }

            //Avaya
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\history.xml",tmp_key);
            if (FileExist(tmp_key_path))
            {
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_history.xml",computername,user);
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\userprofile.xml",tmp_key);
            if (FileExist(tmp_key_path))
            {
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_userprofile.xml",computername,user);
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\Log Files\\SIPMessages.txt",tmp_key);
            if (FileExist(tmp_key_path))
            {
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_SIPMessages.txt",computername,user);
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\Log Files\\onexcui.txt",tmp_key);
            if (FileExist(tmp_key_path))
            {
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_onexcui.txt",computername,user);
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\Log Files\\EndpointLog.txt",tmp_key);
            if (FileExist(tmp_key_path))
            {
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_EndpointLog.txt",computername,user);
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }

            //Cisco JAbber
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Cisco\\JabberVideo\\Logs\\Audit.log",tmp_key);
            if (FileExist(tmp_key_path))
            {
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Cisco\\%s_Audit.log",computername,user);
              //add file
              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
              if (MyhFile != INVALID_HANDLE_VALUE)
              {
                WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
                SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
                WriteFile(MyhFile,"\r\n",2,&copiee,0);
              }
            }
            snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Cisco\\JabberVideo\\*.*",tmp_key);
            hfic = FindFirstFile(tmp_key_path, &wfd0);
            if (hfic != INVALID_HANDLE_VALUE)
            {
              do
              {
                if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                  if(wfd0.cFileName[0] == '.' && (wfd0.cFileName[1] == 0 || wfd0.cFileName[1] == '.'))continue;

                  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Cisco\\JabberVideo\\%s\\mycontacts.xml",tmp_key,wfd0.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Cisco\\%s_%s_mycontacts.xml",computername,user,wfd0.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }

                  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\AppData\\Roaming\\Cisco\\JabberVideo\\%s\\recentcalls.xml",tmp_key,wfd0.cFileName);
                  if (FileExist(tmp_path))
                  {
                    snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Cisco\\%s_%s_recentcalls.xml",computername,user,wfd0.cFileName);
                    //add file
                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                    if (MyhFile != INVALID_HANDLE_VALUE)
                    {
                      WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                      WriteFile(MyhFile,"\r\n",2,&copiee,0);
                    }
                  }
                }
              }while(FindNextFile (hfic,&wfd0) !=0 && start_scan);
              FindClose(hfic);
            }
          }
        }
      }
    }
    RegCloseKey(CleTmp);
  }
}
//------------------------------------------------------------------------------
void CopyChatCustomPathToZIP(void *hz, char*local_path,char*computername,HANDLE MyhFile)
{
  DWORD copiee;
  char tmp_key[MAX_LINE_SIZE], tmp_key_path[MAX_LINE_SIZE], tmp_path[MAX_LINE_SIZE], tmp_path2[MAX_LINE_SIZE], tmp_path_dst[MAX_LINE_SIZE];

  WIN32_FIND_DATA wfd;
  snprintf(tmp_key,MAX_LINE_SIZE,"%s\\*.*",local_path);
  HANDLE hfi = FindFirstFile(tmp_key, &wfd);
  if (hfi != INVALID_HANDLE_VALUE)
  {
    do
    {
      if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        if(wfd.cFileName[0] == '.' && (wfd.cFileName[1] == 0 || wfd.cFileName[1] == '.'))continue;

        //communicator
        WIN32_FIND_DATA wfd0;
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Local\\Microsoft\\Communicator\\*.*",local_path,wfd.cFileName);
        HANDLE hfic = FindFirstFile(tmp_key_path, &wfd0);
        if (hfic != INVALID_HANDLE_VALUE)
        {
          do
          {
            if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              if(wfd0.cFileName[0] == '.' && (wfd0.cFileName[1] == 0 || wfd0.cFileName[1] == '.'))continue;

              snprintf(tmp_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Communicator\\%s\\GalContacts.db",tmp_key,wfd0.cFileName);
              if (FileExist(tmp_path))
              {
                snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Communicator\\%s_%s_GalContacts.db",computername,wfd.cFileName,wfd0.cFileName);
                //add file
                customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                if (MyhFile != INVALID_HANDLE_VALUE)
                {
                  WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                  WriteFile(MyhFile,"\r\n",2,&copiee,0);
                }
              }
            }
          }while(FindNextFile (hfic,&wfd0) !=0 && start_scan);
          FindClose(hfic);
        }

        //Lync
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Local\\Microsoft\Office\\*.*",local_path,wfd.cFileName);
        hfic = FindFirstFile(tmp_key_path, &wfd0);
        if (hfic != INVALID_HANDLE_VALUE)
        {
          do
          {
            if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              if(wfd0.cFileName[0] == '.' && (wfd0.cFileName[1] == 0 || wfd0.cFileName[1] == '.'))continue;

              snprintf(tmp_path,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Office\\%s\\*.*",tmp_key,wfd0.cFileName);
              WIN32_FIND_DATA wfd1;
              HANDLE hfic1 = FindFirstFile(tmp_path, &wfd1);
              if (hfic1 != INVALID_HANDLE_VALUE)
              {
                do
                {
                  if (wfd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                  {
                    if(wfd1.cFileName[0] == '.' && (wfd1.cFileName[1] == 0 || wfd1.cFileName[1] == '.'))continue;

                    snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\AppData\\Local\\Microsoft\\Office\\%s\\%s\\GalContacts.db",tmp_key,wfd0.cFileName,wfd1.cFileName);
                    if (FileExist(tmp_path2))
                    {
                      snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Lync\\%s_%s_GalContacts.db",computername,wfd.cFileName,wfd1.cFileName);
                      //add file
                      customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
                      if (MyhFile != INVALID_HANDLE_VALUE)
                      {
                        WriteFile(MyhFile,tmp_path2,strlen(tmp_path2),&copiee,0);
                        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path2);
                        WriteFile(MyhFile,"\r\n",2,&copiee,0);
                      }
                    }
                  }
                }while(FindNextFile (hfic1,&wfd1) !=0 && start_scan);
                FindClose(hfic1);
              }
            }
          }while(FindNextFile (hfic,&wfd0) !=0 && start_scan);
          FindClose(hfic);
        }

        //Avaya
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\history.xml",local_path,wfd.cFileName);
        if (FileExist(tmp_key_path))
        {
          snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_history.xml",computername,wfd.cFileName);
          //add file
          customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
          if (MyhFile != INVALID_HANDLE_VALUE)
          {
            WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
            SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
            WriteFile(MyhFile,"\r\n",2,&copiee,0);
          }
        }
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\userprofile.xml",local_path,wfd.cFileName);
        if (FileExist(tmp_key_path))
        {
          snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_userprofile.xml",computername,wfd.cFileName);
          //add file
          customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
          if (MyhFile != INVALID_HANDLE_VALUE)
          {
            WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
            SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
            WriteFile(MyhFile,"\r\n",2,&copiee,0);
          }
        }
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\Log Files\\SIPMessages.txt",local_path,wfd.cFileName);
        if (FileExist(tmp_key_path))
        {
          snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_SIPMessages.txt",computername,wfd.cFileName);
          //add file
          customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
          if (MyhFile != INVALID_HANDLE_VALUE)
          {
            WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
            SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
            WriteFile(MyhFile,"\r\n",2,&copiee,0);
          }
        }
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\Log Files\\onexcui.txt",local_path,wfd.cFileName);
        if (FileExist(tmp_key_path))
        {
          snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_onexcui.txt",computername,wfd.cFileName);
          //add file
          customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
          if (MyhFile != INVALID_HANDLE_VALUE)
          {
            WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
            SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
            WriteFile(MyhFile,"\r\n",2,&copiee,0);
          }
        }
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Avaya\\Avaya one-X Communicator\\Log Files\\EndpointLog.txt",local_path,wfd.cFileName);
        if (FileExist(tmp_key_path))
        {
          snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Avaya\\%s_EndpointLog.txt",computername,wfd.cFileName);
          //add file
          customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
          if (MyhFile != INVALID_HANDLE_VALUE)
          {
            WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
            SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
            WriteFile(MyhFile,"\r\n",2,&copiee,0);
          }
        }

        //Cisco JAbber
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Local\\Cisco\\JabberVideo\\Logs\\Audit.log",local_path,wfd.cFileName);
        if (FileExist(tmp_key_path))
        {
          snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Cisco\\%s_Audit.log",computername,wfd.cFileName);
          //add file
          customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_key_path,0, 2);
          if (MyhFile != INVALID_HANDLE_VALUE)
          {
            WriteFile(MyhFile,tmp_key_path,strlen(tmp_key_path),&copiee,0);
            SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_key_path);
            WriteFile(MyhFile,"\r\n",2,&copiee,0);
          }
        }
        snprintf(tmp_key_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Cisco\\JabberVideo\\*.*",local_path,wfd.cFileName);
        hfic = FindFirstFile(tmp_key_path, &wfd0);
        if (hfic != INVALID_HANDLE_VALUE)
        {
          do
          {
            if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              if(wfd0.cFileName[0] == '.' && (wfd0.cFileName[1] == 0 || wfd0.cFileName[1] == '.'))continue;

              snprintf(tmp_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Cisco\\JabberVideo\\%s\\mycontacts.xml",local_path,wfd.cFileName,wfd0.cFileName);
              if (FileExist(tmp_path))
              {
                snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Cisco\\%s_%s_mycontacts.xml",computername,wfd.cFileName,wfd0.cFileName);
                //add file
                customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                if (MyhFile != INVALID_HANDLE_VALUE)
                {
                  WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                  WriteFile(MyhFile,"\r\n",2,&copiee,0);
                }
              }

              snprintf(tmp_path,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Cisco\\JabberVideo\\%s\\recentcalls.xml",local_path,wfd.cFileName,wfd0.cFileName);
              if (FileExist(tmp_path))
              {
                snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Chat\\Cisco\\%s_%s_recentcalls.xml",computername,wfd.cFileName,wfd0.cFileName);
                //add file
                customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path,0, 2);
                if (MyhFile != INVALID_HANDLE_VALUE)
                {
                  WriteFile(MyhFile,tmp_path,strlen(tmp_path),&copiee,0);
                  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path);
                  WriteFile(MyhFile,"\r\n",2,&copiee,0);
                }
              }
            }
          }while(FindNextFile (hfic,&wfd0) !=0 && start_scan);
          FindClose(hfic);
        }

      }
    }while(FindNextFile (hfi,&wfd) !=0 && start_scan);
    FindClose(hfi);
  }
}
//------------------------------------------------------------------------------
void CopyNavigatorHistoryCustomPathToZIP(void *hz, char*local_path,char*computername,HANDLE MyhFile, char *tmp_path, char *path)
{
  //search
  char tmp_path2[MAX_LINE_SIZE], tmp_path3[MAX_LINE_SIZE],tmp_path_dst[MAX_LINE_SIZE],tmp_path_src[MAX_LINE_SIZE];
  WIN32_FIND_DATA data, wfd0, wfd1;
  HANDLE hfic = FindFirstFile(tmp_path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if(data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')){}
      else if (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
      {
        //check for all users the default navigators paths !
        //Firefox
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\Application Data\\Mozilla\\Firefox\\Profiles\\*.*",path,data.cFileName);
        HANDLE hfic0 = FindFirstFile(tmp_path2, &wfd0);
        if (hfic0 != INVALID_HANDLE_VALUE)
        {
          //for each profil
          do
          {
            if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //profil by profil
            {
              //search each sqlite files
              snprintf(tmp_path3,MAX_LINE_SIZE,"%s\\%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\*.sqlite",path,data.cFileName,wfd0.cFileName);
              HANDLE hfic1 = FindFirstFile(tmp_path3, &wfd1);
              if (hfic1 != INVALID_HANDLE_VALUE)
              {
                do
                {
                  snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\Application Data\\Mozilla\\Firefox\\Profiles\\%s\\%s",path,data.cFileName,wfd0.cFileName,wfd1.cFileName);
                  snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\Firefox\\%s_%s_%s",computername,data.cFileName,wfd0.cFileName,wfd1.cFileName);

                  customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
                  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);
                }while(FindNextFile (hfic1,&wfd1) !=0);
                FindClose(hfic1);
              }
            }
          }while(FindNextFile (hfic0,&wfd0) !=0);
          FindClose(hfic0);
        }
        //Firefox W7-
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\*.*",path,data.cFileName);
        hfic0 = FindFirstFile(tmp_path2, &wfd0);
        if (hfic0 != INVALID_HANDLE_VALUE)
        {
          //for each profil
          do
          {
            if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) //profil by profil
            {
              //search each sqlite files
              snprintf(tmp_path3,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\*.sqlite",path,data.cFileName,wfd0.cFileName);
              HANDLE hfic1 = FindFirstFile(tmp_path3, &wfd1);
              if (hfic1 != INVALID_HANDLE_VALUE)
              {
                do
                {
                  snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Mozilla\\Firefox\\Profiles\\%s\\%s",path,data.cFileName,wfd0.cFileName,wfd1.cFileName);
                  snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\Firefox\\%s_%s_%s",computername,data.cFileName,wfd0.cFileName,wfd1.cFileName);

                  customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
                  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);
                }while(FindNextFile (hfic1,&wfd1) !=0);
                FindClose(hfic1);
              }
            }
          }while(FindNextFile (hfic0,&wfd0) !=0);
          FindClose(hfic0);
        }

        //chrome
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Application Data\\Google\\Chrome\\User Data\\default\\*.*",path,data.cFileName);
        hfic0 = FindFirstFile(tmp_path2, &wfd0);
        if (hfic0 != INVALID_HANDLE_VALUE)
        {
          //for each file !
          do
          {
            if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY){} //only the files
            else
            {
              snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Application Data\\Google\\Chrome\\User Data\\default\\%s",path,data.cFileName,wfd0.cFileName);
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\Google\\%s_%s",computername,data.cFileName,wfd0.cFileName);

              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
              SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);
            }
          }while(FindNextFile (hfic0,&wfd0) !=0);
          FindClose(hfic0);
        }

        //IE cookies
        snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\Cookies\\index.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_Cookies_index.dat",computername,data.cFileName);
        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);
        snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\AppData\\Roaming\\Microsoft\\Windows\\Cookies\\PrivacIE\\index.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_Cookies_PrivacIE_index.dat",computername,data.cFileName);
        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);
        snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\AppData\\Local\\Microsoft\\Internet Explorer\\DOMStore\\index.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_DOMStore_index.dat",computername,data.cFileName);
        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);
        snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\AppData\\Local\\Microsoft\\Feeds Cache\\index.dat",path,data.cFileName);
        snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_Feeds_Cache_index.dat",computername,data.cFileName);
        customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
        SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);

        //search other files for IE
        snprintf(tmp_path2,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Historique\\*.*",path,data.cFileName);
        hfic0 = FindFirstFile(tmp_path2, &wfd0);
        if (hfic0 != INVALID_HANDLE_VALUE)
        {
          //for each file !
          do
          {
            if (wfd0.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
              snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Historique\\%s\\index.dat",path,data.cFileName,wfd0.cFileName);
              snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_%s",computername,data.cFileName,wfd0.cFileName);

              customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
              SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);

              //next path if exist
              snprintf(tmp_path3,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Historique\\%s\\*.*",path,data.cFileName,wfd0.cFileName);
              HANDLE hfic1 = FindFirstFile(tmp_path3, &wfd1);
              if (hfic1 != INVALID_HANDLE_VALUE)
              {
                do
                {
                  if (wfd1.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                  {
                    snprintf(tmp_path_src,MAX_LINE_SIZE,"%s\\%s\\Local Settings\\Historique\\%s\\%s\\index.dat",path,data.cFileName,wfd0.cFileName,wfd1.cFileName);
                    snprintf(tmp_path_dst,MAX_LINE_SIZE,"%s\\Navigator\\IE\\%s_%s_%s",computername,data.cFileName,wfd0.cFileName,wfd1.cFileName);

                    customAddSrc((TZIP *)hz,  (void *)tmp_path_dst, (void *)tmp_path2,0, 2);
                    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)tmp_path_src);
                  }
                }while(FindNextFile (hfic1,&wfd1) !=0);
                FindClose(hfic1);
              }
            }
          }while(FindNextFile (hfic0,&wfd0) !=0);
          FindClose(hfic0);
        }

      }
    }while(FindNextFile (hfic,&data) !=0);
    FindClose(hfic);
  }
}
//------------------------------------------------------------------------------
void addProcesstoCSV(char *process, char *pid, char *path, char *cmd,
                    char *owner, char *rid, char *sid, char *start_date,
                    char *protocol, char *ip_src, char *port_src,
                    char *ip_dst, char *port_dst, char *state,
                    char *hidden,char *parent_process, char *parent_pid, HANDLE filetosave, void *hz, char*computername, HANDLE MyhFile)
{
  char h_sha256[MAX_LINE_SIZE], verified[MAX_LINE_SIZE];
  DWORD dw;

  if (path[0] != 'P') // if title or not !
  {
    if (MyhFile != INVALID_HANDLE_VALUE)
    {
      char tmp_buff_w[MAX_LINE_SIZE], tmp_path[MAX_LINE_SIZE];

      snprintf(tmp_path,MAX_LINE_SIZE,"%s\\Process\\%s_%s",computername,pid,process);
      snprintf(tmp_buff_w,MAX_LINE_SIZE,"%s",path);
      ConvertPathFromPath(tmp_buff_w);

      if (GetSHAandVerifyFromPathFile(path, h_sha256, verified, MAX_LINE_SIZE))
      {
        customAddSrc((TZIP *)hz,  (void *)tmp_path, (void *)tmp_buff_w,0, 2);

        snprintf(tmp_buff_w,MAX_LINE_SIZE,"%s, SHA256:%s, Verified:%s\r\n",path,h_sha256,verified);
        WriteFile(MyhFile,tmp_buff_w,strlen(tmp_buff_w),&dw,0);
      }
    }
  }else
  {
    strncpy(h_sha256,"SHA256",MAX_LINE_SIZE);
    strncpy(verified,"Verified",MAX_LINE_SIZE);
  }

  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\r\n",
           process,pid,path,cmd,owner,rid,sid,start_date,protocol,ip_src,port_src,ip_dst,port_dst,state,hidden,parent_process,parent_pid,h_sha256,verified);
  WriteFile(filetosave,request,strlen(request),&dw,0);
}
//------------------------------------------------------------------------------
void EnumProcessAndThreadToFile(DWORD nb_process, PROCESS_INFOS_ARGS *process_info, HANDLE filetosave, void *hz, char*computername, HANDLE MyhFile)
{
  HANDLE hProcess;
  DWORD d_pid, i, j, k, cbNeeded;
  BOOL ok;
  LINE_PROC_ITEM *port_line;
  HMODULE hMod[MAX_PATH];
  FILETIME lpCreationTime, lpExitTime, lpKernelTime, lpUserTime;
  char process[DEFAULT_TMP_SIZE],
       pid[DEFAULT_TMP_SIZE],
       path[MAX_PATH],
       cmd[MAX_PATH],
       owner[DEFAULT_TMP_SIZE],
       rid[DEFAULT_TMP_SIZE],
       sid[DEFAULT_TMP_SIZE],
       start_date[DATE_SIZE_MAX]/*,
       parent_pid[DEFAULT_TMP_SIZE],
       parent_path[MAX_PATH]*/;

  char src_name[MAX_PATH];
  char dst_name[MAX_PATH];

  port_line = (LINE_PROC_ITEM *) malloc(sizeof(LINE_PROC_ITEM)*MAX_LINE_SIZE);
  if (port_line == NULL)return;

  //force enumerate all process by id !
  for (d_pid=FIRST_PROCESS_ID;d_pid<LAST_PROCESS_ID;d_pid++)
  {
    //open process info
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,0,d_pid);
    if (hProcess == NULL)continue;

    //cmd
    cmd[0]=0;
    GetProcessArg(hProcess, cmd, MAX_PATH);

    if (cmd[0]==0)
    {
      ok = FALSE;
    }else
    {
      ok = TRUE;
      //verify if exist or not
      for (i=0;i<nb_process;i++)
      {
        if (process_info[i].pid == d_pid){ok = FALSE;break;}
        else
        {
          if (strcmp(cmd,process_info[i].args) == 0){ok = FALSE;break;}
        }
      }
    }

    if (ok)
    {
      //process
      process[0] = 0;
      GetModuleBaseName(hProcess,NULL,process,DEFAULT_TMP_SIZE);

      //pid
      snprintf(pid,DEFAULT_TMP_SIZE,"%05lu",d_pid);

      //path
      path[0]=0;
      if (EnumProcessModules(hProcess,hMod, MAX_PATH,&cbNeeded))
      {
        if (GetModuleFileNameEx(hProcess,hMod[0],path,MAX_PATH) == 0)path[0] = 0;
      }

      //owner
      GetProcessOwner(d_pid, owner, rid, sid, DEFAULT_TMP_SIZE);

      //start date process
      start_date[0] = 0;
      if (GetProcessTimes(hProcess, &lpCreationTime,&lpExitTime, &lpKernelTime, &lpUserTime))
      {
        //traitement de la date
        if (lpCreationTime.dwHighDateTime != 0 && lpCreationTime.dwLowDateTime != 0)
        {
          filetimeToString_GMT(lpCreationTime, start_date, DATE_SIZE_MAX);
        }
      }

      //ports !
      j=GetPortsFromPID(d_pid, port_line, MAX_LINE_SIZE, SIZE_ITEMS_PORT_MAX);

      convertStringToSQL(path, MAX_PATH);
      convertStringToSQL(cmd, MAX_PATH);

      //add items !
      if (j == 0)addProcesstoCSV(process, pid, path, cmd, owner, rid, sid, start_date,"", "", "","", "", "", "X", "", "",filetosave, hz, computername, MyhFile);
      else
      {
        for (k=0;k<j;k++)
        {
          if (port_line[k].name_src[0] != 0)snprintf(src_name,MAX_PATH,"%s:%s",port_line[k].IP_src,port_line[k].name_src);
          else snprintf(src_name,MAX_PATH,"%s",port_line[k].IP_src);

          if (port_line[k].name_dst[0] != 0)snprintf(dst_name,MAX_PATH,"%s:%s",port_line[k].IP_dst,port_line[k].name_dst);
          else snprintf(dst_name,MAX_PATH,"%s",port_line[k].IP_dst);

          addProcesstoCSV(process, pid, path, cmd, owner, rid, sid, start_date,
                                port_line[k].protocol, src_name, port_line[k].Port_src,
                                dst_name, port_line[k].Port_dst, port_line[k].state, "X", "", "",filetosave, hz, computername, MyhFile);
        }
      }
    }
    CloseHandle(hProcess);
  }
  free(port_line);
}
//------------------------------------------------------------------------------
void Scan_process_to_file(HANDLE filetosave, void *hz, char*computername, HANDLE MyhFile)
{
  addProcesstoCSV("Process", "PID", "Path", "Command",
                    "Owner", "RID", "SID", "Start_date",
                    "Protocol", "IP_src", "Port_src",
                    "IP_dst", "Port_dst", "State",
                    "Hidden", "Parent_process", "Parent_pid", filetosave, hz, computername, MyhFile);
  //init
  PROCESSENTRY32 pe = {sizeof(PROCESSENTRY32)};
  HANDLE hCT = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS|TH32CS_SNAPTHREAD, 0);
  if (hCT==INVALID_HANDLE_VALUE)return;

  DWORD cbNeeded, k, j, nb_process=0;
  HANDLE hProcess, parent_hProcess;
  HMODULE hMod[MAX_LINE_SIZE];
  FILETIME lpCreationTime, lpExitTime, lpKernelTime, lpUserTime;
  LINE_PROC_ITEM *port_line;
  PROCESS_INFOS_ARGS *process_infos;
  char process[DEFAULT_TMP_SIZE],
       pid[DEFAULT_TMP_SIZE],
       path[MAX_LINE_SIZE],
       cmd[MAX_LINE_SIZE],
       owner[DEFAULT_TMP_SIZE],
       rid[DEFAULT_TMP_SIZE],
       sid[DEFAULT_TMP_SIZE],
       start_date[DATE_SIZE_MAX],
       parent_pid[DEFAULT_TMP_SIZE],
       parent_path[MAX_LINE_SIZE];

  char src_name[DEFAULT_TMP_SIZE];
  char dst_name[DEFAULT_TMP_SIZE];

  process_infos = (PROCESS_INFOS_ARGS *) malloc(sizeof(PROCESS_INFOS_ARGS)*MAX_PATH);

  port_line = (LINE_PROC_ITEM *) malloc(sizeof(LINE_PROC_ITEM)*MAX_LINE_SIZE);
  if (port_line == NULL)return;

  while(Process32Next(hCT, &pe))
  {
    //open process info
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,0,pe.th32ProcessID);
    if (hProcess == NULL)continue;

    //process
    process[0] = 0;
    strncpy(process,pe.szExeFile,DEFAULT_TMP_SIZE);

    //pid
    snprintf(pid,DEFAULT_TMP_SIZE,"%05lu",pe.th32ProcessID);

    //path
    path[0]=0;
    if (EnumProcessModules(hProcess,hMod, MAX_LINE_SIZE,&cbNeeded))
    {
      if (GetModuleFileNameEx(hProcess,hMod[0],path,MAX_LINE_SIZE) == 0)path[0] = 0;
    }

    //cmd
    cmd[0]=0;
    GetProcessArg(hProcess, cmd, MAX_LINE_SIZE);

    //owner
    GetProcessOwner(pe.th32ProcessID, owner, rid, sid, DEFAULT_TMP_SIZE);

    //parent processID
    snprintf(parent_pid,DEFAULT_TMP_SIZE,"%05lu",pe.th32ParentProcessID);

    //parent name
    parent_path[0]=0;
    parent_hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,0,pe.th32ParentProcessID);
    if (parent_hProcess != NULL)
    {
      if (EnumProcessModules(parent_hProcess,hMod, MAX_LINE_SIZE,&cbNeeded))
      {
        if (GetModuleFileNameEx(parent_hProcess,hMod[0],parent_path,MAX_LINE_SIZE) == 0)parent_path[0] = 0;
      }
      CloseHandle(parent_hProcess);
    }

    //start date process
    start_date[0] = 0;
    if (GetProcessTimes(hProcess, &lpCreationTime,&lpExitTime, &lpKernelTime, &lpUserTime))
    {
      //traitement de la date
      if (lpCreationTime.dwHighDateTime != 0 && lpCreationTime.dwLowDateTime != 0)
      {
       filetimeToString_GMT(lpCreationTime, start_date, DATE_SIZE_MAX);
      }
    }

    //ports !
    j=GetPortsFromPID(pe.th32ProcessID, port_line, MAX_LINE_SIZE, SIZE_ITEMS_PORT_MAX);

    //update list of process
    if (nb_process<MAX_PATH && process_infos!=NULL)
    {
      process_infos[nb_process].pid = pe.th32ProcessID;
      snprintf(process_infos[nb_process].args,MAX_LINE_SIZE,"%s",cmd);
      nb_process++;
    }

    convertStringToSQL(path, MAX_LINE_SIZE);
    convertStringToSQL(cmd, MAX_LINE_SIZE);

    //add items !
    if (j == 0)addProcesstoCSV(process, pid, path, cmd, owner, rid, sid, start_date,"", "", "","", "", "",""  , parent_path, parent_pid,filetosave, hz, computername, MyhFile);
    else
    {
      for (k=0;k<j;k++)
      {
        if (port_line[k].name_src[0] != 0)snprintf(src_name,DEFAULT_TMP_SIZE,"%s:%s",port_line[k].IP_src,port_line[k].name_src);
        else snprintf(src_name,DEFAULT_TMP_SIZE,"%s",port_line[k].IP_src);

        if (port_line[k].name_dst[0] != 0)snprintf(dst_name,DEFAULT_TMP_SIZE,"%s:%s",port_line[k].IP_dst,port_line[k].name_dst);
        else snprintf(dst_name,DEFAULT_TMP_SIZE,"%s",port_line[k].IP_dst);

        addProcesstoCSV(process, pid, path, cmd, owner, rid, sid, start_date,
                              port_line[k].protocol, src_name, port_line[k].Port_src,
                              dst_name, port_line[k].Port_dst, port_line[k].state, "", parent_path, parent_pid,filetosave, hz, computername, MyhFile);
      }
    }
    CloseHandle(hProcess);
  }

  //verify shadow process !!!
  EnumProcessAndThreadToFile(nb_process, process_infos, filetosave, hz, computername, MyhFile);

  free(process_infos);
  free(port_line);
  CloseHandle(hCT);
}
//------------------------------------------------------------------------------
void CopyProcessToZip(void *hz, char*local_path, char*computername, HANDLE MyhFile)
{
  //extract list of process and backup on csv file
  char tmp_path[MAX_LINE_SIZE],tmp_file[MAX_LINE_SIZE];

  snprintf(tmp_file,MAX_LINE_SIZE,"%s\\process_list.csv",local_path);
  HANDLE filetosave = CreateFile(tmp_file, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, 0);
  if (filetosave != INVALID_HANDLE_VALUE)
  {
    Scan_process_to_file(filetosave,hz,computername, MyhFile);
    CloseHandle(filetosave);
  }
  //backup file on zip file
  snprintf(tmp_path,MAX_LINE_SIZE,"%s\\Process\\process_list.csv",computername);
  customAddSrc((TZIP *)hz,  (void *)tmp_path, (void *)tmp_file,0, 2);
  DeleteFile(tmp_file);
}
//------------------------------------------------------------------------------
void SaveALL(char*filetosave, char *computername, BOOL local)
{
  void *hz;
  if (createZip(&hz, (void *)filetosave,0,2,"")==0)
  {
    LastZIPAdd[0] = 0;
    SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Backup files :");

    //get local directory
    DWORD copiee;
    char local_path[MAX_LINE_SIZE]="",file[MAX_LINE_SIZE];
    GetLocalPath(local_path, MAX_LINE_SIZE);

    snprintf(file,MAX_LINE_SIZE,"%s\\log.txt",local_path);
    HANDLE MyhFile = CreateFile(file, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, 0);

    char tmp_path[MAX_LINE_SIZE]="";
    char *s = tmp_path;

    if(!ReadValue(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", "SystemRoot", tmp_path, MAX_LINE_SIZE))
      s = NULL;

    if (tmp_path[0] == 0 || local)s = NULL;

    //evt + evtx
    WriteFile(MyhFile,"[LOG]\r\n",strlen("[LOG]\r\n"),&copiee,0);
    CopyEvtToZIP(hz,s,local_path,computername,MyhFile);
    CopyEvtxToZIP(hz,s,local_path,computername,MyhFile);

    //system logs
    CopyAllWinLogToZIP(hz,s,computername,MyhFile);

    //prefetch
    WriteFile(MyhFile,"\r\n[PREFETCH]\r\n",strlen("\r\n[PREFETCH]\r\n"),&copiee,0);
    CopyPrefetchToZIP(hz,s,local_path,computername,MyhFile);

    //job
    WriteFile(MyhFile,"\r\n[JOB]\r\n",strlen("\r\n[JOB]\r\n"),&copiee,0);
    CopyJobToZIP(hz,s,local_path,computername,MyhFile);

    //NTDS.DIT
    WriteFile(MyhFile,"\r\n[AD]\r\n",strlen("\r\n[AD]\r\n"),&copiee,0);
    s = tmp_path;
    tmp_path[0] = 0;
    if(!ReadValue(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Services\\NTDS\\Parameters", "DSA Database File", tmp_path, MAX_LINE_SIZE))
      s = NULL;
    else ReplaceEnv("WINDIR",tmp_path,MAX_LINE_SIZE);

    if (tmp_path[0] == 0)s = NULL;

    CopyNTDISToZIP(hz,s,local_path,computername,MyhFile);

    //registry
    WriteFile(MyhFile,"\r\n[REGISRY]\r\n",strlen("\r\n[REGISTRY]\r\n"),&copiee,0);
    CopyRegistryToZIP(hz,local_path,computername,MyhFile);
    CopyRegistryUSERToZIP(hz,local_path,computername,MyhFile);

    //navigator
    WriteFile(MyhFile,"\r\n[NAVIGATOR]\r\n",strlen("\r\n[NAVIGATOR]\r\n"),&copiee,0);
    CopyNavigatorHistoryToZIP(hz,local_path,computername,MyhFile);

    //Chat LYNC/SKYPE
    WriteFile(MyhFile,"\r\n[Chat]\r\n",strlen("\r\n[Chat]\r\n"),&copiee,0);
    CopyChatHistoryToZIP(hz,computername,MyhFile);

    //MBR
    WriteFile(MyhFile,"\r\n[MBR]\r\n",strlen("\r\n[MBR]\r\n"),&copiee,0);
    CopyMBRToZip(hz,local_path,computername,MyhFile);

    //Process
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"PROCESS");
    WriteFile(MyhFile,"\r\n[PROCESS]\r\n",strlen("\r\n[PROCESS]\r\n"),&copiee,0);
    CopyProcessToZip(hz,local_path,computername,MyhFile);

    //list of files
    CloseHandle(MyhFile);

    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\log.txt",computername);
    customAddSrc((TZIP *)hz,  (void *)tmp_path, (void *)file,0, 2);
    DeleteFile(file);

    ZipClose(hz);

    SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Copy of all files done !");
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"");
    LastZIPAdd[0] = 0;
  }
}
//------------------------------------------------------------------------------
void SaveAllTRVFilesToZip(char*filetosave)
{
  BACKUP_FILE_LIST_started = TRUE;
  void *hz;
  if (createZip(&hz, (void *)filetosave,0,2,"")==0)
  {
    //list all trv items !
    char tmp_files_src[MAX_LINE_SIZE];
    HTREEITEM hitem;
    unsigned int i;

    //get local directory
    DWORD copiee;
    char local_path[MAX_LINE_SIZE]="",file[MAX_LINE_SIZE];
    snprintf(file,MAX_LINE_SIZE,"%s\\log.txt",GetLocalPath(local_path, MAX_LINE_SIZE));
    HANDLE MyhFile = CreateFile(file, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, 0);

    SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)"Backup files...");

    for (i=0; i<4; i++)
    {
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[i]);
      while(hitem!=NULL && BACKUP_FILE_LIST_started)
      {
        tmp_files_src[0] = 0;
        if (GetTextFromTrv(hitem, tmp_files_src, MAX_LINE_SIZE) != NULL)
        {
          //if (GetFileAttributes(tmp_files_src)&FILE_ATTRIBUTE_DIRECTORY == 0)
          {
            if (MyhFile != INVALID_HANDLE_VALUE)
            {
              WriteFile(MyhFile,tmp_files_src,strlen(tmp_files_src),&copiee,0);
              WriteFile(MyhFile,"\r\n",2,&copiee,0);
            }

            SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)tmp_files_src);
            customAddSrc((TZIP *)hz,(void *)(tmp_files_src+3),(void *)tmp_files_src,0, 2); //pass "c:\"
          }
        }
        hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
      }
    }
    //clean
    CloseHandle(MyhFile);
    customAddSrc((TZIP *)hz,  (void *)"log.txt", (void *)file,0, 2);
    DeleteFile(file);
    ZipClose(hz);

    SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)"Copy of all files done !");
  }
  BACKUP_FILE_LIST_started = FALSE;
}
//------------------------------------------------------------------------------
void SaveALLCustom(char*filetosave, char *computername, char *path)
{
  BACKUP_PATH_started = TRUE;
  void *hz;
  if (createZip(&hz, (void *)filetosave,0,2,"")==0)
  {
    LastZIPAdd[0] = 0;
    SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)"Backup files...");

    //get local directory
    char local_path[MAX_LINE_SIZE]="",file[MAX_LINE_SIZE];
    snprintf(file,MAX_LINE_SIZE,"%s\\log.txt",GetLocalPath(local_path, MAX_LINE_SIZE));
    HANDLE MyhFile = CreateFile(file, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL, 0);

    //copy
    char tmp_path[MAX_LINE_SIZE]="";
    char *s = tmp_path;
    snprintf(tmp_path,MAX_LINE_SIZE, "%swindows",path);

    //evt + evtx
    CopyEvtToZIP(hz,s,local_path,computername,MyhFile);
    CopyEvtxToZIP(hz,s,local_path,computername,MyhFile);
    //system logs
    CopyAllWinLogToZIP(hz,s,computername,MyhFile);
    //prefetch
    CopyPrefetchToZIP(hz,s,local_path,computername,MyhFile);
    //job
    CopyJobToZIP(hz,s,local_path,computername,MyhFile);
    //AD NTDS.DIT
    snprintf(tmp_path,MAX_LINE_SIZE, "%swindows\\System32\\Ntds.dit",path);
    CopyNTDISToZIP(hz,s,local_path,computername,MyhFile);

    //registry hives
    char tmp_src[MAX_LINE_SIZE], tmp_dst[MAX_LINE_SIZE], tmp_file[MAX_LINE_SIZE];
    snprintf(tmp_src,MAX_LINE_SIZE,"%s\\SYSTEM",tmp_path);
    snprintf(tmp_file,MAX_LINE_SIZE,"%s\\Registry\\SYSTEM",computername);
    customAddSrc((TZIP *)hz,(void *)tmp_src, (void *)tmp_file,0, 2);
    snprintf(tmp_src,MAX_LINE_SIZE,"%s\\SOFTWARE",tmp_path);
    snprintf(tmp_file,MAX_LINE_SIZE,"%s\\Registry\\SOFTWARE",computername);
    customAddSrc((TZIP *)hz,(void *)tmp_src, (void *)tmp_file,0, 2);
    snprintf(tmp_src,MAX_LINE_SIZE,"%s\\SECURITY",tmp_path);
    snprintf(tmp_file,MAX_LINE_SIZE,"%s\\Registry\\SECURITY",computername);
    customAddSrc((TZIP *)hz,(void *)tmp_src, (void *)tmp_file,0, 2);
    snprintf(tmp_src,MAX_LINE_SIZE,"%s\\SAM",tmp_path);
    snprintf(tmp_file,MAX_LINE_SIZE,"%s\\Registry\\SAM",computername);
    customAddSrc((TZIP *)hz,(void *)tmp_src, (void *)tmp_file,0, 2);
    snprintf(tmp_src,MAX_LINE_SIZE,"%s\\DEFAULT",tmp_path);
    snprintf(tmp_file,MAX_LINE_SIZE,"%s\\Registry\\DEFAULT",computername);
    customAddSrc((TZIP *)hz,(void *)tmp_src, (void *)tmp_file,0, 2);

    //registry users
    snprintf(tmp_src,MAX_LINE_SIZE,"%s\\Documents and Settings\\*.*",path);
    snprintf(tmp_dst,MAX_LINE_SIZE,"%s\\Documents and Settings",path);
    CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, tmp_src, tmp_dst);
    CopyNavigatorHistoryCustomPathToZIP(hz,local_path,computername,MyhFile, tmp_src, tmp_dst); //navigator
    snprintf(tmp_src,MAX_LINE_SIZE,"%s\\Users\\*.*",path);
    snprintf(tmp_dst,MAX_LINE_SIZE,"%s\\Users",path);
    CopyRegistryUSERmToZIP(hz, local_path,computername,MyhFile, tmp_src, tmp_dst);

    //Navigator
    CopyNavigatorHistoryCustomPathToZIP(hz,local_path,computername,MyhFile, tmp_src, tmp_dst); //navigator

    //Chat
    CopyChatCustomPathToZIP(hz,local_path,computername,MyhFile);

    //clean
    CloseHandle(MyhFile);
    snprintf(tmp_path,MAX_LINE_SIZE,"%s\\log.txt",computername);
    customAddSrc((TZIP *)hz,  (void *)tmp_path, (void *)file,0, 2);
    DeleteFile(file);
    ZipClose(hz);

    SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)"Copy of all files done !");
    SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"");
    LastZIPAdd[0] = 0;
  }
  BACKUP_PATH_started = FALSE;
}
//------------------------------------------------------------------------------
DWORD WINAPI BackupAllFiles(LPVOID lParam)
{
  char file[MAX_LINE_SIZE]="";
  GenerateNameToSave(file, MAX_LINE_SIZE, "ALL.zip");

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(OPENFILENAME));
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = h_main;
  ofn.lpstrFile = file;
  ofn.nMaxFile = MAX_LINE_SIZE;
  ofn.lpstrFilter ="*.zip\0*.zip\0";
  ofn.nFilterIndex = 1;
  ofn.Flags =OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
  ofn.lpstrDefExt ="zip\0";
  if (GetSaveFileName(&ofn)==TRUE)
  {
    //get 7
    DWORD taille = 256;
    char computername[256]="";
    GetComputerName(computername,&taille);
    SaveALL(file,computername, TRUE);
    SendMessage(hstatus_bar,SB_SETTEXT,0, (LPARAM)"Backup all file done !!!");
  }
  return 0;
}
