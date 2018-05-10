//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addDisktoDB(char *drive, char *type, char *name, char *filesystem,
                 char *freespace, char *globalspace, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_disk (drive,type,name,filesystem,freespace,globalspace,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           drive,type,name,filesystem,freespace,globalspace,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Disk\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         drive,type,name,filesystem,freespace,globalspace,session_id);
  #endif
}
//------------------------------------------------------------------------------
void DisktoDB(char *drive, char *type, unsigned int session_id, sqlite3 *db)
{
  char volumeName[DEFAULT_TMP_SIZE]   = ""
      ,filesystem[DEFAULT_TMP_SIZE]   = ""
      ,freespace[DEFAULT_TMP_SIZE]    = ""
      ,globalspace[DEFAULT_TMP_SIZE]  = "";
  DWORD cluster=0, secteur=0, Cluster_libre=0, Total_Cluster=0,FileFlags=0,dw_component_length = DEFAULT_TMP_SIZE;

  //informations
  if (GetVolumeInformation(drive,volumeName,DEFAULT_TMP_SIZE,0,&dw_component_length,&FileFlags,filesystem,DEFAULT_TMP_SIZE) == 0)
  {
    volumeName[0]   = 0;
    filesystem[0]   = 0;
  }

  //size
  if (GetDiskFreeSpace(drive,&cluster,&secteur,&Cluster_libre,&Total_Cluster)!=0)
  {
    snprintf(freespace,DEFAULT_TMP_SIZE,"%1.2f Go",((Cluster_libre * cluster)/1024.0*secteur)/(1024.0*1024.0));
    snprintf(globalspace,DEFAULT_TMP_SIZE,"%1.2f Go",((Total_Cluster * cluster)/1024.0*secteur)/(1024.0*1024.0));
  }

  convertStringToSQL(volumeName, DEFAULT_TMP_SIZE);
  convertStringToSQL(filesystem, DEFAULT_TMP_SIZE);
  addDisktoDB(drive, type, volumeName, filesystem, freespace, globalspace, session_id, db);
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_disk(LPVOID lParam)
{
  //check if local or not :)
  if (!LOCAL_SCAN)
  {
    h_thread_test[(unsigned int)lParam] = 0;
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
    return 0;
  }

  //init
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  char tmp[MAX_PATH];
  int i,nblecteurs = GetLogicalDriveStrings(MAX_PATH,tmp);
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Disk\";\"drive\";\"type\";\"name\";\"filesystem\";\"freespace\";\"globalspace\";\"session_id\";\r\n");
  #endif // CMD_LINE_ONLY_NO_DB
  for (i=0;i<nblecteurs;i+=4)
  {
    switch(GetDriveType(&tmp[i]))
    {
      case DRIVE_UNKNOWN:     addDisktoDB(&tmp[i], "DRIVE_UNKNOWN"    , "", "", "", "", session_id, db);break;
      case DRIVE_NO_ROOT_DIR: addDisktoDB(&tmp[i], "DRIVE_NO_ROOT_DIR", "", "", "", "", session_id, db);break;
      case DRIVE_CDROM:       addDisktoDB(&tmp[i], "DRIVE_CDROM"      , "", "", "", "", session_id, db);break;

      case DRIVE_FIXED:       DisktoDB(&tmp[i], "DRIVE_FIXED"     , session_id, db);break;
      case DRIVE_REMOTE:      DisktoDB(&tmp[i], "DRIVE_REMOTE"    , session_id, db);break;
      case DRIVE_RAMDISK:     DisktoDB(&tmp[i], "DRIVE_RAMDISK"   , session_id, db);break;
      case DRIVE_REMOVABLE:   DisktoDB(&tmp[i], "DRIVE_REMOVABLE" , session_id, db);break;
      break;
    }
  }
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
