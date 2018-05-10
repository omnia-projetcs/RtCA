//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../../RtCA.h"
/*#include "Vss.h"
#include "VsWriter.h"
#include "VsBackup.h"*/
//------------------------------------------------------------------------------
//https://github.com/nihon-tc/LFMtest/tree/master/header/Microsoft%20SDKs/Windows/v7.0A/Include
//------------------------------------------------------------------------------
/*BOOL VSSFileCopyFilefromPath(char *path_src, char *path_dst)
{
  BOOL Ok       = FALSE;
  HMODULE hDLL  = LoadLibrary( "VssApi.dll");

  if (hDLL!=NULL)
  {
    //load functions
    typedef HRESULT (WINAPI *ISVOLUMESNAPSHOTTED)(char* pwszVolumeName, BOOL *pbSnapshotsPresent, LONG *plSnapshotCapability);
    ISVOLUMESNAPSHOTTED MyIsVolumeSnapshotted = (ISVOLUMESNAPSHOTTED) GetProcAddress(hDLL,"IsVolumeSnapshotted");

    typedef HRESULT (WINAPI *CREATEVSSBACKUPCOMPONENT)(VssBackupComponents **ppBackup);
    CREATEVSSBACKUPCOMPONENT MyCreateVssBackupComponents = (CREATEVSSBACKUPCOMPONENT) GetProcAddress(hDLL,"CreateVssBackupComponents");

    if (MyIsVolumeSnapshotted && MyCreateVssBackupComponents)
    {
      //init
      if (CoInitializeEx(NULL, COINIT_MULTITHREADED); == S_OK)
      {
        //check if snapshot exist
        char path[MAX_PATH] = "c:\\";
        path[0] = path_src[0];

        BOOL SnapshotsPresent   = FALSE;
        LONG SnapshotCapability = 0;
        HRESULT ret = MyIsVolumeSnapshotted(path, &SnapshotsPresent, &SnapshotCapability);

        if (ret == S_OK && SnapshotsPresent)
        {
          IVssBackupComponents **ppBackup;
          ret = MyCreateVssBackupComponents(ppBackup);
          if (ret == S_OK && ppBackup != NULL)
          {

            //get list of snapshot and get the last


            //backup the file


          }
        }
      }
    }

    FreeLibrary(hDLL);
  }

  return Ok;






  /*BOOL Ok=FALSE;
  //load dll to verify if funtion exist
  HMODULE hDLL;
  if ((hDLL = LoadLibrary( "VssApi.dll"))!=NULL)
  {
    char cmd_vssadmin[MAX_PATH]="", path_src_VSS[MAX_PATH]="";
    //create a snapshot of lecteur 2003r2/2008/2012 only
    snprintf(cmd_vssadmin,MAX_PATH,"vssadmin create shadow /for=%c:",path_src[0]);
    system(cmd_vssadmin);

    //copy file
    unsigned int test = 400; //not good code but compatible with much systems
    do
    {
      snprintf(path_src_VSS,MAX_PATH,"\\\\?\\GLOBALROOT\\Device\\HarddiskVolumeShadowCopy%d%s",test,&path_src[2]);

    }while (CopyFile(path_src_VSS,path_dst,FALSE) == 0 && test-->0);
    if (test > 0 )Ok = TRUE;

    //no delete snapshot
    //delete snapshot
    //snprintf(cmd_vssadmin,MAX_PATH,"vssadmin delete shadows /for=%c: /shadow=%s",path_src[0],snapshoot_id);
    //system(cmd_vssadmin);
    FreeLibrary(hDLL);
  }
  return Ok;
}*/
