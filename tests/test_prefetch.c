//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
  #define HDR_PREFETCH_SIZE             0xA4
  #define PREFETCH_APPLI_NAME_MAX_SIZE  60
  typedef struct
  {
    DWORD v_number;                                         //0x00
    DWORD m_number;                                         //0x04
    DWORD os_vtype;                                         //0x08
    DWORD pf_size;                                          //0x0C
    char filename[PREFETCH_APPLI_NAME_MAX_SIZE];            //0x10
    DWORD file_hash;                                        //0x4B
    DWORD d_unknow;                                         //0x50
    DWORD secA_off;                                         //0x54
    DWORD secA_nb_sec;                                      //0x58
    DWORD secB_off;                                         //0x5C
    DWORD secB_nb_sec;                                      //0x60
    DWORD secC_off;   //offset of application path          //0x64
    DWORD secC_size;  //section size                        //0x68
    DWORD secD_off;                                         //0x6C
    DWORD secD_nb_sec;                                      //0x70
    DWORD secD_size;                                        //0x74
    FILETIME last_file_exec;    //usefull                   //0x78
    FILETIME last_file_exec2;                               //0x80
    FILETIME f_unknow2;                                     //0x88
    DWORD file_exec_counter;    //usefull                   //0x90
    DWORD nb_sections;                                      //0x94
    DWORD file_exec_counter2;    //usefull                  //0x98
    DWORD nb_sections2;                                     //0x9C-0xA0
  }HDR_PREFETCH;

  typedef struct
  {
    char header[3];                                         //"MAM"


  }HDR_PREFETCH_W10;


//------------------------------------------------------------------------------
void addPrefetchtoDB(char *file, char *create_time, char *last_update, char*last_access, DWORD count, char *exec, char *depend, char *path_application, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE+4];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_prefetch (file,path,create_time,last_update,last_access,count,exec,session_id,depend) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%08lu\",\"%s\",%d,\"%s\");",
           file,path_application,create_time,last_update,last_access,count,exec,session_id,depend);

  //if data too long
  if (request[strlen(request)-1]!=';')strncat(request,"\");\0",REQUEST_MAX_SIZE+4-strlen(request));

  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Prefetch\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%08lu\";\"%s\";\"%d\";\"%s\";\r\n",
         file,path_application,create_time,last_update,last_access,count,exec,session_id,depend);
  #endif
}
//------------------------------------------------------------------------------
void PfCheck(unsigned int session_id, sqlite3 *db, char *file)
{
  //open file and read first datas
  HANDLE hfile = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (hfile != INVALID_HANDLE_VALUE)
  {
    DWORD sz_prefetch_file = GetFileSize(hfile,NULL);
    if ( sz_prefetch_file!= INVALID_FILE_SIZE && sz_prefetch_file > sizeof(HDR_PREFETCH))
    {
      char CreationTime[DATE_SIZE_MAX]="",LastWriteTime[DATE_SIZE_MAX]="",LastAccessTime[DATE_SIZE_MAX]="";
      DWORD count = 0, copiee;
      char exec_time[DATE_SIZE_MAX]="";
      char *depend;
      char *buffer;
      HDR_PREFETCH *pf;
      char *b, *e;

      char application_name[PREFETCH_APPLI_NAME_MAX_SIZE+1], path_application[MAX_PATH]="", path_depend[MAX_PATH]="";
      BOOL ok_path_application = FALSE;

      buffer = malloc(sizeof(char)*sz_prefetch_file+1);
      if (buffer!=NULL)
      {
        ReadFile(hfile, buffer, sz_prefetch_file,&copiee,0);

        //file times
        BY_HANDLE_FILE_INFORMATION hfi;
        if (GetFileInformationByHandle(hfile,&hfi))
        {
          filetimeToString_GMT(hfi.ftCreationTime, CreationTime, DATE_SIZE_MAX);
          filetimeToString_GMT(hfi.ftLastWriteTime, LastWriteTime, DATE_SIZE_MAX);
          filetimeToString_GMT(hfi.ftLastAccessTime, LastAccessTime, DATE_SIZE_MAX);
        }

        if(buffer[0] == 'M' && buffer[1] == 'A' && buffer[2] == 'M') // W10 format
        {
          HDR_PREFETCH_W10 *wpf = (HDR_PREFETCH_W10 *) buffer;

          /*to do*/

          addPrefetchtoDB(file, CreationTime, LastWriteTime, LastAccessTime, 0, "", "", "",session_id, db);
        }else
        {
          pf = (HDR_PREFETCH*)buffer;

          if (pf->v_number > 0x11) //0x17 after winxp/2003 (+8octets)
          {
            count = pf->file_exec_counter2;
            filetimeToString_GMT(pf->last_file_exec2, exec_time, DATE_SIZE_MAX);
          }else
          {
            count = pf->file_exec_counter;
            filetimeToString_GMT(pf->last_file_exec, exec_time, DATE_SIZE_MAX);
          }

          snprintf(application_name,PREFETCH_APPLI_NAME_MAX_SIZE,"%S",pf->filename);
          application_name[PREFETCH_APPLI_NAME_MAX_SIZE]=0;

          //search in path datas if name exist
          ok_path_application = FALSE;
          b = buffer+pf->secC_off;                  //start of datas
          e = buffer+pf->secC_off+pf->secC_size;    //end of datas
          DWORD size_depend = (e-b);
          if (size_depend <= sz_prefetch_file)
          {
            while (b<e)
            {
              snprintf(path_application,MAX_PATH,"%S",b);
              if (ContientNoCass(path_application,application_name))
              {
                ok_path_application = TRUE;
                break;
              }else
              {
                b = b + (strlen(path_application)+1)*2;
              }
            };

            if (!ok_path_application)path_application[0] = 0;

            //depend
            b = buffer+pf->secC_off;                  //start of datas
            e = buffer+pf->secC_off+pf->secC_size;    //end of datas
            depend = (char*)malloc(size_depend+2);
            if (depend != NULL)
            {
              depend[0] = 0;
              while (b<e)
              {
                snprintf(path_depend,MAX_PATH,"%S\r\n",b);
                snprintf(depend+strlen(depend),size_depend-strlen(depend),"%s",path_depend);
                //strncat(depend,path_depend,size_depend);
                b = b + (strlen(path_depend)-1)*2;
              };
              strncat(depend,"\0",size_depend);
              addPrefetchtoDB(file, CreationTime, LastWriteTime, LastAccessTime, count, exec_time, depend, path_application,session_id, db);
              free(depend);
            }else addPrefetchtoDB(file, CreationTime, LastWriteTime, LastAccessTime, count, exec_time, "", path_application,session_id, db);
          }
        }
        free(buffer);
      }
    }
    CloseHandle(hfile);
  }
}

//------------------------------------------------------------------------------
//format : http://www.forensicswiki.org/wiki/Windows_Prefetch_File_Format
DWORD WINAPI Scan_prefetch(LPVOID lParam)
{
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Prefetch\";\"file\";\"path\";\"create_time\";\"last_update\";\"last_access\";\"count\";\"exec\";\"session_id\";\"depend\";\r\n");
  #endif
  //check if local or not :)
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_APPLI]);
  if (hitem!=NULL || !LOCAL_SCAN || WINE_OS)
  {
    if(!SQLITE_FULL_SPEED)sqlite3_exec(db,"BEGIN TRANSACTION;", NULL, NULL, NULL);
    char tmp_file_pref[MAX_PATH],ext[MAX_PATH];
    while(hitem!=NULL)
    {
      tmp_file_pref[0] = 0;
      ext[0]           = 0;
      GetTextFromTrv(hitem, tmp_file_pref, MAX_PATH);
      if (!strcmp("pf",extractExtFromFile(charToLowChar(tmp_file_pref), ext, MAX_PATH)))
        PfCheck(session_id, db, tmp_file_pref);

      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }

    if(!SQLITE_FULL_SPEED)sqlite3_exec(db,"END TRANSACTION;", NULL, NULL, NULL);
    h_thread_test[(unsigned int)lParam] = 0;
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
    return 0;
  }

  //init
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);

  //get system path
  char path[MAX_PATH] ="%WINDIR%\\Prefetch\\*.pf";
  ReplaceEnv("WINDIR",path,MAX_PATH);

  char path_f[MAX_PATH];

  WIN32_FIND_DATA data;
  HANDLE hfic = FindFirstFile(path, &data);
  if (hfic != INVALID_HANDLE_VALUE)
  {
    do
    {
      if((data.cFileName[0] == '.' && (data.cFileName[1] == 0 || data.cFileName[1] == '.')) || (data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)){}
      else
      {
        snprintf(path_f,MAX_PATH,"%s",path);
        path_f[strlen(path_f)-4]=0;
        strncat(path_f,data.cFileName,MAX_PATH-strlen(path_f));
        strncat(path_f,"\0",MAX_PATH-strlen(path_f));

        PfCheck(session_id, db, path_f);
      }
    }while(FindNextFile (hfic,&data) !=0 && start_scan);
    FindClose(hfic);
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
