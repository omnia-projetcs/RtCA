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
  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4-strlen(request));

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
/*BOOL GetMessageString(DWORD MessageId, HANDLE ddl_handle, char *msg, DWORD msg_max_size)
{
  DWORD_PTR* pArgs  = NULL;
  msg[0]            = 0;
  if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE, ddl_handle,MessageId,0,msg,msg_max_size,(va_list*)pArgs)return FALSE;
  else return TRUE;
}*/
//------------------------------------------------------------------------------
char *GetDataHexaAndString(char *s, unsigned int sz, char *tmp, DWORD tmp_sz)
{
  //get hexa
  DataToHexaChar(s,sz,tmp,tmp_sz);
  strncat(tmp," : ",tmp_sz);

  DWORD i;
  char tmp_2[MAX_PATH]="";
  for (i = 0; i< sz; i++)
  {
    if (s[i] < 127 && s[i] > 31)tmp_2[i] = s[i];
    else tmp_2[i] = '.';
  }
  tmp_2[i] = 0;

  strncat(tmp,tmp_2,tmp_sz);
  strncat(tmp,"\0",tmp_sz);

  return tmp;
}
//------------------------------------------------------------------------------
BOOL GetMultiMessageString(DWORD MessageId, HANDLE ddl_handle, char *msg, DWORD msg_max_size, DWORD argc, char* argv, DWORD argv_sz)
{
  DWORD_PTR* pArgs  = NULL;
  msg[0]            = 0;

  if (argc > 0 && argv_sz > 0)
  {
    pArgs = (DWORD_PTR*)malloc(sizeof(DWORD_PTR) * argc);
    if (pArgs)
    {
      DWORD i;
      char* pString = argv;
      BOOL ret = FALSE;

     // char tmp[MAX_LINE_SIZE]="";
     // GetDataHexaAndString(pString, argv_sz, tmp, MAX_LINE_SIZE);

      //MessageBox(NULL,*GetDataHexaAndString(pString, argv_sz, tmp, MAX_LINE_SIZE)/*DataToHexaChar(argv,argv_sz,tmp,MAX_LINE_SIZE)*/,"--",MB_OK|MB_TOPMOST);

      for (i = 0; i < argc; i++)
      {
          pArgs[i] = (DWORD_PTR)pString;
          //MessageBox(NULL,pString,"0-",MB_OK|MB_TOPMOST);
          pString += strlen(pString) + 1;
      }
/*
      if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_IGNORE_INSERTS | FORMAT_MESSAGE_ARGUMENT_ARRAY, ddl_handle,MessageId,0,msg,msg_max_size,(va_list*)pArgs) != 0)
      {
        msg[0] = 0;*/
        if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE | FORMAT_MESSAGE_ARGUMENT_ARRAY, ddl_handle,MessageId,0,msg,msg_max_size,(va_list*)pArgs) != 0)
          ret  = TRUE;
/*
        strncat(msg,tmp,msg_max_size);
        strncat(msg,"\0",msg_max_size);*/
   //   }

        //FORMAT_MESSAGE_IGNORE_INSERTS

      free(pArgs);
      return ret;
    }
  }else
  {
    if (!FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_FROM_HMODULE, ddl_handle,MessageId,0,msg,msg_max_size,(va_list*)pArgs))return FALSE;
    else return TRUE;
  }
  return FALSE;
}
//------------------------------------------------------------------------------
BOOL GetEventDatas(EVENTLOGRECORD *pevlr, char *eventname, char *source, char *description, unsigned int description_max_sz, DWORD dwRead)
{
  if (description == NULL || description_max_sz <2 )return FALSE;
  description[0] = 0;
  BOOL ret = FALSE;
  //unsigned int lvl = 0;

  //get
  //get DLL resource
  char key[MAX_PATH], ddlpath[MAX_PATH]="", ddlpath_ok[MAX_PATH]="", tmp_EventCategory[MAX_LINE_SIZE], msg[MAX_LINE_SIZE];
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
        //get all messages from a recorder !
        //event category: %s
        GetMultiMessageString(pevlr->EventCategory, mhandle, tmp_EventCategory, MAX_LINE_SIZE, 0, 0, 0);
        GetMultiMessageString(pevlr->EventID, mhandle, msg, MAX_LINE_SIZE, pevlr->NumStrings, pevlr + (pevlr->StringOffset),pevlr->DataLength);

        snprintf(description, description_max_sz,"%s%s",tmp_EventCategory,msg);
        FreeLibrary(mhandle);
        ret = TRUE;
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
  BYTE *bTmp, *bBuffer = (PBYTE)malloc(cbBuffer+1);
  if (bBuffer == NULL) return;

  pevlr = (EVENTLOGRECORD *) &bBuffer;
  DWORD status = ERROR_SUCCESS;
  unsigned long int i=1;//, z;
  DWORD dwRead, dwNeeded;

  memset(bBuffer,0,cbBuffer);
  while (status == ERROR_SUCCESS && i < cRecords && start_scan)
  {
    if(!ReadEventLog(Heventlog,EVENTLOG_BACKWARDS_READ |EVENTLOG_SEQUENTIAL_READ,0,bBuffer,cbBuffer,&dwRead,&dwNeeded))
    {
      //if error buffer, resize it !
      status = GetLastError();
      if(status == ERROR_INSUFFICIENT_BUFFER)
      {
        cbBuffer = dwNeeded;
        bTmp = (PBYTE)realloc(bBuffer, cbBuffer);

        if (bTmp == NULL)break;
        else
        {
          bBuffer = bTmp;
          status = ERROR_SUCCESS; //continue news !
          continue;
        }
      }
    }else
    {
      //get datas !
      pevlr = (EVENTLOGRECORD *) bBuffer;
      if (pevlr!=NULL && pevlr->Length>0)
      {
        PBYTE pRecord = bBuffer;
        PBYTE pEndOfRecords = bBuffer + dwRead;
        while (pRecord < pEndOfRecords && i < cRecords && start_scan )
        {
          pevlr = (EVENTLOGRECORD *)pRecord;

          //record number
          snprintf(indx,DEFAULT_TMP_SIZE,"%08lu",pevlr->RecordNumber & 0xFFFF);

          //Type
          switch(pevlr->EventType)
          {
            case EVENTLOG_ERROR_TYPE       : strcpy(state,"ERROR\0"); break;        //0x01
            case EVENTLOG_WARNING_TYPE     : strcpy(state,"WARNING\0"); break;      //0x02
            case EVENTLOG_INFORMATION_TYPE : strcpy(state,"INFORMATION\0"); break;  //0x04
            case EVENTLOG_AUDIT_SUCCESS    : strcpy(state,"AUDIT_SUCCESS\0"); break;//0x08
            case EVENTLOG_AUDIT_FAILURE    : strcpy(state,"AUDIT_FAILURE\0"); break;//0x10
            default :snprintf(state,DEFAULT_TMP_SIZE,"UNKNOW:%d",pevlr->EventType);break;
          }

          if (state[0]!='U')
          {
            //date : send_date
            timeToString(pevlr->TimeGenerated, send_date, DATE_SIZE_MAX);

            //date : write_date
            timeToString(pevlr->TimeWritten, write_date, DATE_SIZE_MAX);

            //source
            source[0]=0;
            if (sizeof(EVENTLOGRECORD) < pevlr->Length && sizeof(EVENTLOGRECORD)+1 < (pEndOfRecords-pRecord))
              strncpy(source,(char *)pevlr+sizeof(EVENTLOGRECORD),DEFAULT_TMP_SIZE);

            //ID
            snprintf(log_id,DEFAULT_TMP_SIZE,"%08lu",pevlr->EventID & 0xFFFF);

            //user+rid+sid
            user[0] = 0;
            rid[0]  = 0;
            sid[0]  = 0;
            if (pevlr->UserSidOffset < (pEndOfRecords-pRecord) && pevlr->UserSidLength > 0)
              SidtoUser((PSID)((LPBYTE) pevlr + pevlr->UserSidOffset), user, rid, sid, DEFAULT_TMP_SIZE);

            //descriptions infos !!!
            if ((pevlr->StringOffset+ pevlr->DataLength) < dwRead)
            {
              //sources from https://msdn.microsoft.com/en-us/library/windows/desktop/bb427356%28v=vs.85%29.aspx
              //GetEventDatas(pevlr, eventname, source, description, MAX_LINE_SIZE,dwRead);
            }

            //end !!!
            if (strcmp(send_date,write_date) != 0)strncpy(critical,"X",DEFAULT_TMP_SIZE);
            else critical[0]=0;

            //add
            convertStringToSQL(source, MAX_PATH);
            convertStringToSQL(description, MAX_LINE_SIZE);

            addLogtoDB(eventname, indx, log_id,
                       send_date, write_date, source, description,
                       user, rid, sid, state, critical, session_id, db);
          }

          pRecord += ((PEVENTLOGRECORD)pRecord)->Length;
          i++;
        }
        memset(bBuffer,0,cbBuffer);
      }
    }
  }
  free(bBuffer);
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
      if (strcmp("evt",ext) == 0){SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)tmp);TraiterEventlogFileEvt(tmp, db, session_id);}
      else if (strcmp("evtx",ext) == 0){SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)tmp);TraiterEventlogFileEvtx(tmp, db, session_id);}
      else if (strcmp("log",ext) == 0){SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)tmp);TraiterEventlogFileLog(tmp, db, session_id);}
      if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    //direct load events from files names  ???

    //review her



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
              SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)eventname);
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

  SendMessage(GetDlgItem((HWND)h_conf,DLG_CONF_SB),SB_SETTEXT,0, (LPARAM)"");

  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
