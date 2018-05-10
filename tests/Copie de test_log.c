//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addLogtoDB(  char *event, char *indx, char *log_id,
                  char *send_date, char *write_date,
                  char *source, char *description, char *user, char *rid, char *sid,
                  char *state, char *critical, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_log (event,indx,log_id,send_date,write_date,source,user,rid,sid,state,critical,session_id,description) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d,\"%s\");",
           event,indx,log_id,send_date,write_date,source,user,rid,sid,state,critical,session_id,description);

  //if description too long
  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4);

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Log\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\"%s\";\r\n",
         event,indx,log_id,send_date,write_date,source,user,rid,sid,state,critical,session_id,description);
  #endif
}
//------------------------------------------------------------------------------
void GetPointToArray(EVENTLOGRECORD *pevlr, DWORD_PTR Array[], DWORD nb_array_max)
{
  LPBYTE c = ((LPBYTE) pevlr + pevlr->StringOffset);
  int i;
  for (i = 0; i < pevlr->NumStrings && i < nb_array_max; i++)
  {
      Array[i] = (int) c;
      c += strlen((char *) c) + 1;
  }
}
//------------------------------------------------------------------------------
//thanks to http://code.google.com/p/evtail/
//thanks to http://msdn.microsoft.com/en-us/library/windows/desktop/bb427356%28v=vs.85%29.aspx
BOOL readMessageDatas(EVENTLOGRECORD *pevlr, char *eventname, char *source, char *resultat, unsigned int resultat_max_size)
{
  resultat[0] = 0;
  DWORD ret = FALSE;

  //get library path
  char key[MAX_PATH], ddlpath[MAX_PATH]="", ddlpath_ok[MAX_PATH]="";
  snprintf(key,MAX_PATH,"SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s\\%s",eventname,source);
  if (ReadValue(HKEY_LOCAL_MACHINE,key,"EventMessageFile",ddlpath, MAX_PATH))
  {
    //replace system string type of %string%
    ExpandEnvironmentStrings(ddlpath, ddlpath_ok, MAX_PATH);
    //load DATAFILE
    if(FileExist(ddlpath_ok))
    {
      HANDLE mhandle = LoadLibraryEx(ddlpath_ok, 0, LOAD_LIBRARY_AS_DATAFILE);
      if (mhandle != NULL)
      {
        DWORD_PTR dwpRepStrings[0xFF];
        //GetPointToArray(pevlr, dwpRepStrings, 0xFF);

        ret = FormatMessage(FORMAT_MESSAGE_FROM_HMODULE|FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_ARGUMENT_ARRAY,
                            mhandle,pevlr->EventID,0,resultat,resultat_max_size-1,dwpRepStrings);

        FreeLibrary(mhandle);
      }
    }
  }
  return (BOOL)ret;
}

//------------------------------------------------------------------------------
BOOL GetEventDatas(EVENTLOGRECORD *pevlr, char *eventname, char *source, char *description, unsigned int description_max_sz)
{
  if (description == NULL || description_max_sz <2 )return FALSE;
  description[0] = 0;
  BOOL ret = FALSE;

  //get
  //get DLL resource
  char key[MAX_PATH], ddlpath[MAX_PATH]="", ddlpath_ok[MAX_PATH]="";
  snprintf(key,MAX_PATH,"SYSTEM\\CurrentControlSet\\Services\\EventLog\\%s\\%s",eventname,source);
  if (ReadValue(HKEY_LOCAL_MACHINE,key,"EventMessageFile",ddlpath, MAX_PATH))
  {
    //replace system string type of %string%
    ExpandEnvironmentStrings(ddlpath, ddlpath_ok, MAX_PATH);
    //load DATAFILE
    if(FileExist(ddlpath_ok))
    {
      HANDLE mhandle = LoadLibraryEx(ddlpath_ok, 0, LOAD_LIBRARY_AS_DATAFILE);
      if (mhandle != NULL)
      {
        //read and convert datas



        FreeLibrary(mhandle);
      }
    }
  }
  return ret;
}
//------------------------------------------------------------------------------
void LireEvent(HANDLE Heventlog, char *eventname, sqlite3 *db, unsigned int session_id, DWORD cRecords)
{
  char indx[DEFAULT_TMP_SIZE], log_id[DEFAULT_TMP_SIZE],
  send_date[DATE_SIZE_MAX], write_date[DATE_SIZE_MAX],
  source[MAX_PATH], description[MAX_LINE_SIZE],
  user[DEFAULT_TMP_SIZE], rid[DEFAULT_TMP_SIZE], sid[DEFAULT_TMP_SIZE],
  state[DEFAULT_TMP_SIZE], critical[DEFAULT_TMP_SIZE];

  EVENTLOGRECORD *pevlr;
  DWORD cbBuffer = 0x10000;  //64k
  BYTE bBuffer[0x10000+1];
  pevlr = (EVENTLOGRECORD *) &bBuffer;
  DWORD dwRead, dwNeeded;

  unsigned long int i=1, z;
  memset(bBuffer,0,cbBuffer);
  while (ReadEventLog(Heventlog,EVENTLOG_BACKWARDS_READ |EVENTLOG_SEQUENTIAL_READ,0,&bBuffer,cbBuffer,&dwRead,&dwNeeded) && i < cRecords && start_scan)
  {
    pevlr = (EVENTLOGRECORD *) &bBuffer;
    if (pevlr!=NULL && pevlr->Length>0)
    {
      while (dwRead > 0 && i < cRecords && start_scan)
      {
        //record number
        snprintf(indx,DEFAULT_TMP_SIZE,"%08lu",pevlr->RecordNumber & 0xFFFF);

        //Type
        switch(pevlr->EventType)
        {
          case EVENTLOG_ERROR_TYPE       : strcpy(state,"ERROR"); break;        //0x01
          case EVENTLOG_WARNING_TYPE     : strcpy(state,"WARNING"); break;      //0x02
          case EVENTLOG_INFORMATION_TYPE : strcpy(state,"INFORMATION"); break;  //0x04
          case EVENTLOG_AUDIT_SUCCESS    : strcpy(state,"AUDIT_SUCCESS"); break;//0x08
          case EVENTLOG_AUDIT_FAILURE    : strcpy(state,"AUDIT_FAILURE"); break;//0x10
          default :state[0]=0;break;
        }

        if (state[0]!=0)
        {
          //date : send_date
          timeToString(pevlr->TimeGenerated, send_date, DATE_SIZE_MAX);

          //date : write_date
          timeToString(pevlr->TimeWritten, write_date, DATE_SIZE_MAX);

          //source
          source[0]=0;
          if (sizeof(EVENTLOGRECORD) < pevlr->Length && sizeof(EVENTLOGRECORD)+1 < dwRead)
            strncpy(source,(char *)pevlr+sizeof(EVENTLOGRECORD),DEFAULT_TMP_SIZE);

          //ID
          snprintf(log_id,DEFAULT_TMP_SIZE,"%08lu",pevlr->EventID & 0xFFFF);

          //user+rid+sid
          user[0] = 0;
          rid[0]  = 0;
          sid[0]  = 0;
          if (pevlr->UserSidOffset < dwRead && pevlr->UserSidLength > 0)
            SidtoUser((PSID)((LPBYTE) pevlr + pevlr->UserSidOffset), user, rid, sid, DEFAULT_TMP_SIZE);

          if ((pevlr->StringOffset+ pevlr->DataLength) < dwRead)
          {
            GetEventDatas(pevlr, eventname, source, description, MAX_LINE_SIZE);



            //if (readMessageDatas(pevlr, eventname, source, description, MAX_LINE_SIZE) == FALSE)
            /*{
              //get string in raw mode !
              //plante aussi ici
              /*description[0] = 0;
              char*c = ((LPBYTE) pevlr + pevlr->StringOffset);
              for (z = 0; z < pevlr->NumStrings; z++)
              {
                snprintf(description+strlen(description),MAX_LINE_SIZE-strlen(description),"%s,",c);
                c += strlen((char *) c) + 1;
              }
            }*/
          }

          if (strcmp(send_date,write_date) != 0)strncpy(critical,"X",DEFAULT_TMP_SIZE);
          else critical[0]=0;

          //add
          convertStringToSQL(source, MAX_PATH);
          convertStringToSQL(description, MAX_LINE_SIZE);

          addLogtoDB(eventname, indx, log_id,
                     send_date, write_date, source, description,
                     user, rid, sid, state, critical, session_id, db);
        }
        pevlr = (EVENTLOGRECORD *)((LPBYTE) pevlr + pevlr->Length);
        dwRead = dwRead-pevlr->Length;
        i++;
      }
      memset(bBuffer,0,cbBuffer);
    }
    break;
  }
}
//------------------------------------------------------------------------------
void OpenDirectEventLog(char *eventname, sqlite3 *db, unsigned int session_id)
{
  //backup puis event log
  HANDLE Heventlog = OpenBackupEventLog(NULL,eventname);
  DWORD cRecords=0;
  if (Heventlog!=NULL)
  {
    if (GetNumberOfEventLogRecords(Heventlog, &cRecords))
    {
      if (cRecords>0)
      {
        LireEvent(Heventlog,eventname,db,session_id,cRecords);
        CloseEventLog(Heventlog);
      }
    }
  }

  Heventlog = OpenEventLog(NULL,eventname);
  cRecords=0;
  if (Heventlog!=NULL)
  {
    if (GetNumberOfEventLogRecords(Heventlog, &cRecords))
    {
      if (cRecords>0)
      {
        LireEvent(Heventlog,eventname,db,session_id,cRecords);
        CloseEventLog(Heventlog);
      }
    }
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_log(LPVOID lParam)
{
  unsigned int session_id = current_session_id;
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Log\";\"event\";\"indx\";\"log_id\";\"send_date\";\"write_date\";\"source\";\"user\";\"rid\";\"sid\";\"state\";\"critical\";\"session_id\";\"description\";\r\n");
  #endif
  //db
  sqlite3 *db = (sqlite3 *)db_scan;
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_LOGS]);
  if (hitem!=NULL || !LOCAL_SCAN)
  {
    char tmp[MAX_PATH];
    char ext[10];
    while(hitem!=NULL)
    {
      //get item txt
      GetTextFromTrv(hitem, tmp, MAX_PATH);

      //get extension on verify
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      ext[0] = 0;
      extractExtFromFile(tmp, ext, 10);
      if (strcmp("evt",ext) == 0)TraiterEventlogFileEvt(tmp, db, session_id);
      else if (strcmp("evtx",ext) == 0)TraiterEventlogFileEvtx(tmp, db, session_id);
      else if (strcmp("log",ext) == 0)TraiterEventlogFileLog(tmp, db, session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    //direct load events from files names !





    //read in the registry of all event
    char eventname[MAX_PATH];
    HKEY CleTmp=0;
    DWORD nbSubKey=0,i=0, TailleNomSubKey;

    BOOL ok = FALSE;
    if (RegOpenKey((HKEY)HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Services\\Eventlog",&CleTmp)==ERROR_SUCCESS)
    {
      if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
      {
        if (nbSubKey > 0)
        {
          for (i=0;i<nbSubKey;i++)
          {
            TailleNomSubKey = MAX_PATH;
            eventname[0] = 0;
            if (RegEnumKeyEx (CleTmp,i,eventname,(LPDWORD)&TailleNomSubKey,0,0,0,0)==ERROR_SUCCESS)
            {
              ok=TRUE;
              if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
              OpenDirectEventLog(eventname,db,session_id);
              if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
            }
          }
        }
      }
      RegCloseKey(CleTmp);
    }
    if (!ok)
    {
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("Application",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("HardwareEvents",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("Internet Explorer",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("Key Management Service",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("Setup",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("Security",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("System",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("Windows PowerShell",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
      OpenDirectEventLog("OSession",db,session_id);
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
    }
  }

  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
