//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
//header infos
typedef struct _EVENTLOGHEADER {
  ULONG HeaderSize;
  ULONG Signature;
  ULONG MajorVersion;
  ULONG MinorVersion;
  ULONG StartOffset;
  ULONG EndOffset;
  ULONG CurrentRecordNumber;
  ULONG OldestRecordNumber;
  ULONG MaxSize;
  ULONG Flags;
  ULONG Retention;
  ULONG EndHeaderSize;
} EVENTLOGHEADER, *PEVENTLOGHEADER;
//------------------------------------------------------------------------------
void EventRecordGo(BYTE *buffer, DWORD mem_read, char*eventname, char *eventfile, sqlite3 *db, unsigned int session_id)
{
  char indx[DEFAULT_TMP_SIZE], log_id[DEFAULT_TMP_SIZE],
  send_date[DATE_SIZE_MAX], write_date[DATE_SIZE_MAX],
  source[MAX_PATH], description[MAX_LINE_SIZE],
  user[DEFAULT_TMP_SIZE], rid[DEFAULT_TMP_SIZE], sid[DEFAULT_TMP_SIZE],
  state[DEFAULT_TMP_SIZE], critical[DEFAULT_TMP_SIZE];

  EVENTLOGRECORD *pevlr = (EVENTLOGRECORD *)buffer;

  //init
  snprintf(indx,DEFAULT_TMP_SIZE,"%08lu",pevlr->RecordNumber & 0xFFFF);

  //Type
  switch(pevlr->EventType)
  {
    case EVENTLOG_ERROR_TYPE       : strcpy(state,"ERROR\0"); break;        //0x01
    case EVENTLOG_WARNING_TYPE     : strcpy(state,"WARNING\0"); break;      //0x02
    case EVENTLOG_INFORMATION_TYPE : strcpy(state,"INFORMATION\0"); break;  //0x04
    case EVENTLOG_AUDIT_SUCCESS    : strcpy(state,"AUDIT_SUCCESS\0"); break;//0x08
    case EVENTLOG_AUDIT_FAILURE    : strcpy(state,"AUDIT_FAILURE\0"); break;//0x10
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
    if (sizeof(EVENTLOGRECORD) < pevlr->Length)
      snprintf(source,DEFAULT_TMP_SIZE,"%S",(char *)pevlr+sizeof(EVENTLOGRECORD));

    //ID
    snprintf(log_id,DEFAULT_TMP_SIZE,"%08lu",pevlr->EventID& 0xFFFF);

    //user+rid+sid
    user[0] = 0;
    rid[0]  = 0;
    sid[0]  = 0;
    SidtoUser((PSID)((LPBYTE) pevlr + pevlr->UserSidOffset), user, rid, sid, DEFAULT_TMP_SIZE);

    //descriptions strings !!!
    //init
    memset(description, 0, MAX_LINE_SIZE);

    if ((pevlr->StringOffset+ pevlr->DataLength) < mem_read)
    {
      if (readMessageDatas(pevlr, eventname, source, description, MAX_LINE_SIZE) == FALSE)
      {
        //get string in raw mode !
        description[0] = 0;
        int z;
        char*c = ((LPBYTE) pevlr + pevlr->StringOffset);
        for (z = 0; z < pevlr->NumStrings; z++)
        {
          snprintf(description+strlen(description),MAX_LINE_SIZE-strlen(description),"%S,",c);
          c += strlen((char *) c) + 2;
        }
      }
    }

    if (strcmp(send_date,write_date) != 0)strncpy(critical,"X",DEFAULT_TMP_SIZE);
    else critical[0]=0;

    convertStringToSQL(source, MAX_PATH);
    convertStringToSQL(description, MAX_LINE_SIZE);
    addLogtoDB(eventfile, indx, log_id,
           send_date, write_date, source, description,
           user, rid, sid, state, critical, session_id, db);
  }
}
//------------------------------------------------------------------------------
void TraiterEventlogFileEvt(char * eventfile, sqlite3 *db, unsigned int session_id)
{
  HANDLE Hlog = CreateFile(eventfile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (Hlog != INVALID_HANDLE_VALUE)
  {
    char eventname[MAX_PATH];
    extractFileFromPath(eventfile, eventname, MAX_PATH);
    if (strlen(eventname)-4 > 0)
    {
      //remove .evt
      eventname[strlen(eventname)-4] = 0;
    }else eventname[0] = 0;

    DWORD taille_fic = GetFileSize(Hlog,NULL);
    if (taille_fic>0 && taille_fic!=INVALID_FILE_SIZE)
    {
      unsigned char *b, *buffer;
      DWORD copiee =0, position = 0, increm = 0, tmp_increm, tmp_cpy, c=0;
      if (taille_fic > DIXM)
      {
        buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(unsigned char*)*DIXM+1);
        increm = DIXM;
      }else
      {
        buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(unsigned char*)*taille_fic+1);
        increm = taille_fic;
      }

      if (buffer == NULL)
      {
        CloseHandle(Hlog);
        return;
      }

      //read file header
      ReadFile(Hlog, buffer, increm,&copiee,0);
      position +=copiee;
      if (taille_fic-position < increm)increm = taille_fic-position ;
      b = buffer;

      //good file or not ?
      if (((PEVENTLOGHEADER)b)->HeaderSize == 0x30 && ((PEVENTLOGHEADER)b)->MajorVersion == 1&& ((PEVENTLOGHEADER)b)->MinorVersion == 1)
      {
        //first record
        EVENTLOGRECORD *pevlr = (EVENTLOGRECORD *) b;
        pevlr = (EVENTLOGRECORD *)((LPBYTE) pevlr+0x30);
        c+=0x30;  //record first position after header

        //good header or not ?
        //if not search next good header !
        if (pevlr->Reserved != 1699505740)
        {
          while (c<copiee && pevlr->Reserved != 1699505740)
          {
            pevlr = (EVENTLOGRECORD *)((LPBYTE) pevlr+1);
            c++;
          }
        }

        DWORD last = 0;
        BOOL BAD = FALSE;

        //get record !!
        while(position<=taille_fic && copiee>0 && start_scan)
        {
          while (c < copiee && (copiee-c) >= pevlr->Length && pevlr->Length > 0)
          {
            if (pevlr->Reserved != 1699505740) //error 0x654c664c
            {
              BAD = TRUE;
              break;
            }
            EventRecordGo((BYTE *)pevlr, copiee, eventname, eventfile, db, session_id);

            //next record
            c+=pevlr->Length;
            pevlr = (EVENTLOGRECORD *)((LPBYTE) pevlr + pevlr->Length);
          }

          if (BAD)break;
          if (increm == 0)break;

          //next file part to read
          if (c == copiee)
          {
            c = 0;
            if (taille_fic-position < increm)increm = taille_fic-position;
            if (increm <1 || increm>taille_fic)break;
            if(!ReadFile(Hlog, buffer, increm,&copiee,0))break;
            position +=copiee;
            b = buffer;
            pevlr = (EVENTLOGRECORD *) b;
          }else
          {
            //all buffer have not been read !!!
            //copy in next buffer datas to read
            tmp_cpy = increm-(((LPBYTE) pevlr)-buffer);
            if (tmp_cpy > 0 && tmp_cpy < increm)memcpy(buffer,((LPBYTE) pevlr),tmp_cpy);

            if (taille_fic-position < (increm-tmp_cpy))tmp_increm = taille_fic-position;
            else tmp_increm = increm-tmp_cpy;
            if (tmp_increm <1 || tmp_increm>increm )break;

            c = 0;
            if(!ReadFile(Hlog, buffer+tmp_cpy, tmp_increm,&copiee,0))break;
            position +=copiee;
            copiee +=tmp_cpy;
            b = buffer;
            pevlr = (EVENTLOGRECORD *) b;
          }

          //anti looping
          if (last != position)last = position;
          else break;
        }
      }
      //free memory
      HeapFree(GetProcessHeap(), 0, buffer);
    }
  }
  CloseHandle(Hlog);
}
