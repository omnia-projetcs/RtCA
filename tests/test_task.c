//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addTasktoDB(char *id_ev, char *type, char *data, char*next_run, char *create_date, char*update_date, char *access_date,char*user,char*details, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_tache (id_ev,type,data,next_run,create_date,update_date,access_date,user,details,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           id_ev,type,data,next_run,create_date,update_date,access_date,user,details,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Task\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         id_ev,type,data,next_run,create_date,update_date,access_date,user,details,session_id);
  #endif
}
//------------------------------------------------------------------------------
void JobCheck(unsigned int session_id, sqlite3 *db, char *file)
{
  //default path : C:\WINDOWS\Tasks
  // source http://msdn.microsoft.com/en-us/library/cc248286%28PROT.13%29.aspx
  // http://lprng.sourceforge.net/LPRng-Reference-Multipart/jobfileformatconversion.htm
  // http://msdn.microsoft.com/en-us/library/cc248286%28v=prot.13%29.aspx
  typedef struct
  {
    unsigned short Product_Version;
    unsigned short File_Version;
    unsigned char Job_id[16];
    unsigned short App_Name_Len_Offset; //offset of short seize + unicode char
    unsigned short id_Offset;
  }HDR_JOB;
  HDR_JOB *hb;

  typedef struct
  {
    unsigned short enable;
    unsigned short flags;
    unsigned short unknow;
    unsigned short year;
    unsigned short month;
    unsigned short day;
    unsigned char unsed[6];
    unsigned short hour;
    unsigned short minute;
    unsigned short second;
  }S_TIME_JOB;

  //open file and read first datas
  HANDLE hfile = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (hfile != INVALID_HANDLE_VALUE)
  {
    DWORD sz_prefetch_job = GetFileSize(hfile,NULL);
    if ( sz_prefetch_job!= INVALID_FILE_SIZE)
    {
      char *buffer = malloc(sizeof(char)*sz_prefetch_job+1);
      if (buffer != NULL)
      {
        DWORD copiee;
        if(ReadFile(hfile, buffer, sz_prefetch_job,&copiee,0))
        {
          hb = (HDR_JOB*)buffer;
          if (hb->App_Name_Len_Offset+2 < sz_prefetch_job)
          {
            //time
            char time[MAX_PATH]="";
            char type[MAX_PATH]="";
            if (hb->id_Offset+sizeof(S_TIME_JOB) < sz_prefetch_job)
            {
              S_TIME_JOB *stj = (S_TIME_JOB *)(buffer + (hb->id_Offset));
              snprintf(time,DATE_SIZE_MAX,"%d/%02d/%02d %02d:%02d:%02d",
                       stj->year,stj->month,stj->day,
                       stj->hour,stj->minute,stj->second);

              //state
              if (stj->enable)strcpy(type,"ENABLE\0");
              else strcpy(type,"DISABLE\0");

              //0xffff
              //TASK_FLAG_RUN_ONLY_IF_LOGGED_ON = 0x2000
              if (stj->flags & 0x2000)strncat(type,",TASK_FLAG_RUN_ONLY_IF_LOGGED_ON\0",MAX_PATH-strlen(type));
              //TASK_FLAG_SYSTEM_REQUIRED       = 0x1000
              if (stj->flags & 0x1000)strncat(type,",TASK_FLAG_SYSTEM_REQUIRED\0",MAX_PATH-strlen(type));
              //TASK_FLAG_HIDDEN                = 0x0200
              if (stj->flags & 0x0200)strncat(type,",TASK_FLAG_HIDDEN\0",MAX_PATH-strlen(type));
              //TASK_FLAG_START_ONLY_IF_IDLE    = 0x0010
              if (stj->flags & 0x0010)strncat(type,",TASK_FLAG_START_ONLY_IF_IDLE\0",MAX_PATH-strlen(type));
              //TASK_FLAG_DISABLED              = 0x0004
              if (stj->flags & 0x0004)strncat(type,",TASK_FLAG_DISABLED\0",MAX_PATH-strlen(type));
              //TASK_FLAG_DELETE_WHEN_DONE      = 0x0002
              if (stj->flags & 0x0002)strncat(type,",TASK_FLAG_DELETE_WHEN_DONE\0",MAX_PATH-strlen(type));
              //TASK_FLAG_INTERACTIVE           = 0x0001
              if (stj->flags & 0x0001)strncat(type,",TASK_FLAG_INTERACTIVE\0",MAX_PATH-strlen(type));
            }

            //description
            char cmd[MAX_PATH]                = "",
                 desc[MAX_LINE_SIZE]          = "",
                 CreationTime[DATE_SIZE_MAX]  = "",
                 LastWriteTime[DATE_SIZE_MAX] = "",
                 LastAccessTime[DATE_SIZE_MAX]= "";

            //file times
            BY_HANDLE_FILE_INFORMATION hfi;
            if (GetFileInformationByHandle(hfile,&hfi))
            {
              filetimeToString_GMT(hfi.ftCreationTime, CreationTime, DATE_SIZE_MAX);
              filetimeToString_GMT(hfi.ftLastWriteTime, LastWriteTime, DATE_SIZE_MAX);
              filetimeToString_GMT(hfi.ftLastAccessTime, LastAccessTime, DATE_SIZE_MAX);
            }

            //size + datas
            unsigned short *s   = (char*)(buffer+(hb->App_Name_Len_Offset));
            unsigned short size = (*s)*2;
            unsigned char *c    = (char*)(buffer+(hb->App_Name_Len_Offset+2));
            snprintf(cmd,MAX_PATH,"%S",c);

            //init
            int i =0;
            FORMAT_TESTS_STRING items[10];
            for (i=0;i<10;i++) memset(items[i].s,0,DEFAULT_TMP_SIZE);

            for (i=0;i<10 && (c + size) < (buffer+sz_prefetch_job) ;i++)
            {
              //size of datas
              s     = (unsigned short *)(c + size);
              c     = (c + size + 2);
              size  = (*s)*2;
              if ((c + size) > (buffer+sz_prefetch_job))break;
              if (size > 0)
              {
                snprintf(items[i].s,DEFAULT_TMP_SIZE,"%S",c);
              }
            }

            for (i=0;i<10;i++)
            {
              if (items[i].s[2] != 0 && i!=2) //min 3 caracteres
              {
                if (desc[0] != 0)strncat(desc,", ",MAX_PATH);
                strncat(desc,items[i].s,MAX_PATH);
              }
            }
            strncat(desc,"\0",MAX_PATH);

            convertStringToSQL(cmd, MAX_PATH);
            convertStringToSQL(items[2].s, DEFAULT_TMP_SIZE);
            convertStringToSQL(desc, MAX_LINE_SIZE);
            addTasktoDB(file, type, cmd, time, CreationTime, LastWriteTime, LastAccessTime, items[2].s,desc, session_id, db);
          }
        }
        free(buffer);
      }
    }
    CloseHandle(hfile);
  }
}

//------------------------------------------------------------------------------
DWORD WINAPI Scan_task(LPVOID lParam)
{
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db,"BEGIN TRANSACTION;", NULL, NULL, NULL);

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Task\";\"id_ev\";\"type\";\"data\";\"next_run\";\"create_date\";\"update_date\";\"access_date\";\"user\";\"details\";\"session_id\";\r\n");
  #endif
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
  if (hitem!=NULL || !LOCAL_SCAN || WINE_OS)
  {

    char tmp_file_job[MAX_PATH],ext[MAX_PATH];
    while(hitem!=NULL)
    {
      tmp_file_job[0] = 0;
      ext[0]           = 0;
      GetTextFromTrv(hitem, tmp_file_job, MAX_PATH);
      if (!strcmp("job",extractExtFromFile(charToLowChar(tmp_file_job), ext, MAX_PATH)))
      {
        JobCheck(session_id, db, tmp_file_job);
      }

      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else //local
  {
    //enum locale task files
    //get system path
    char path[MAX_PATH] ="%WINDIR%\\Tasks\\*.job";
    ReplaceEnv("WINDIR",path,MAX_PATH);
    unsigned int nb_ok = 0;

    char tmp_file_job[MAX_PATH];

    WIN32_FIND_DATA data;
    HANDLE hfic = FindFirstFile(path, &data);
    if (hfic != INVALID_HANDLE_VALUE)
    {
      do
      {
        if((data.cFileName[0] == '.' && ((data.cFileName[1] == 0) || (data.cFileName[1] == '.'))) || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){}
        else
        {
          snprintf(tmp_file_job,MAX_PATH,"%s",path);
          tmp_file_job[strlen(tmp_file_job)-5]=0;
          strncat(tmp_file_job,data.cFileName,MAX_PATH-strlen(tmp_file_job));
          strncat(tmp_file_job,"\0",MAX_PATH-strlen(tmp_file_job));

          JobCheck(session_id, db, tmp_file_job);
          nb_ok++;
        }
      }while(FindNextFile (hfic,&data) !=0 && start_scan);
      FindClose(hfic);
    }

    //if protected
    if (nb_ok == 0)
    {
      if (MyNetScheduleJobEnum && MyNetApiBufferFree)
      {
        //get datas
        AT_ENUM *bufAtEnum,*b;
        DWORD EntriesRead=0,TotalEntries=0,ResumeHandle=0;
        NET_API_STATUS res = MyNetScheduleJobEnum(0,(LPBYTE*)&bufAtEnum,MAX_PREFERRED_LENGTH,&EntriesRead,&TotalEntries,&ResumeHandle);

        if(res == 0 && TotalEntries && EntriesRead && bufAtEnum)
        {
          char id_ev[DEFAULT_TMP_SIZE],type[DEFAULT_TMP_SIZE],data[MAX_PATH],next_run[DEFAULT_TMP_SIZE];

          for(b = bufAtEnum;EntriesRead>0;EntriesRead--)
          {
            if (b->Command > 0)
            {
              id_ev[0]    = 0;
              type[0]     = 0;
              data[0]     = 0;
              next_run[0] = 0;

              snprintf(id_ev,DEFAULT_TMP_SIZE,"%08lu",b->JobId);
              snprintf(data,MAX_PATH,"%S",b->Command);
              convertStringToSQL(data, MAX_PATH);
              //timeToString(b->JobTime, next_run, DEFAULT_TMP_SIZE);
              snprintf(next_run,DEFAULT_TMP_SIZE,"%lu:%02d:%02d",b->JobTime/3600000,(unsigned int)(b->JobTime%3600000)/60000,(unsigned int)((b->JobTime%3600000)%60000)/1000);

              switch(b->Flags)
              {
                case 1: addTasktoDB(id_ev,"JOB_RUN_PERIODICALLY",data,next_run,"","","","","",session_id,db);break;
                case 2: addTasktoDB(id_ev,"JOB_EXEC_ERROR",data,next_run,"","","","","",session_id,db);break;
                case 4: addTasktoDB(id_ev,"JOB_RUNS_TODAY",data,next_run,"","","","","",session_id,db);break;
                case 16:addTasktoDB(id_ev,"JOB_NONINTERACTIVE",data,next_run,"","","","","",session_id,db);break;
                default:
                  snprintf(type,DEFAULT_TMP_SIZE,"UNKNOW (%d)",b->Flags);
                  addTasktoDB(id_ev,type,data,next_run,"","","","","",session_id,db);
                break;
              }
            }
            b++;
          }
        }
        MyNetApiBufferFree(bufAtEnum);
      }
    }
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db,"END TRANSACTION;", NULL, NULL, NULL);

  h_thread_test[(unsigned int)lParam] = 0;
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  return 0;
}
