//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addHosttoDB(char*file, char*ip, char*name, char*last_file_update, unsigned int session_id, sqlite3 *db)
{
  //chek name if malware or not
  MalwareCheck(name);
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[MAX_LINE_SIZE+DEFAULT_TMP_SIZE];
  snprintf(request,MAX_LINE_SIZE+DEFAULT_TMP_SIZE,
           "INSERT INTO extract_host (file,ip,name,last_file_update,malware_check,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           file,ip,name,last_file_update,malware_check,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"DNS\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,ip,name,last_file_update,malware_check,session_id);
  #endif
}
//------------------------------------------------------------------------------
int callback_sqlite_malware(void *datas, int argc, char **argv, char **azColName)
{
  if(malware_check[0] == 0)
  {
    if (argc>2) snprintf(malware_check,MAX_PATH,"%s (%s:%s)",argv[0],argv[1],argv[2]);
    else snprintf(malware_check,MAX_PATH,"%s (%s)",argv[0],argv[1]);
  }
  return 0;
}
//------------------------------------------------------------------------------
void MalwareCheck(char*name)
{
  malware_check[0] = 0;
  char *c = name;

  //if > 2 caracts
  if (*c ==0)return;
  if (*(c+1) ==0)return;
  if (*(c+2) ==0)return;
  if (*(c+3) ==0)return;

  char request[MAX_LINE_SIZE];
  //get only last 2 parts xxx.xxx
  while (*c++);
  while (*c != name[0] && *c != '.')c--; //fisrt
  if (*c != name[0])c--;
  while (*c != name[0] && *c != '.')c--; //second
  if (*c == '.')
  {
    c++;
    snprintf(request, MAX_LINE_SIZE,"SELECT description,domain,update_time FROM malware_dns_list WHERE domain LIKE \"%%%s\";",c);
  }else snprintf(request, MAX_LINE_SIZE,"SELECT description,domain,update_time FROM malware_dns_list WHERE domain LIKE \"%%%s\";",name);

  FORMAT_CALBAK_READ_INFO fcri;
  sqlite3_exec(db_scan, request, callback_sqlite_malware, &fcri, NULL);
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_dns(LPVOID lParam)
{
  unsigned int local_id = (unsigned int)lParam;

  //check if local or not :)
  if (!LOCAL_SCAN || WINE_OS)
  {
    h_thread_test[local_id] = 0;
    check_treeview(htrv_test, H_tests[local_id], TRV_STATE_UNCHECK);//db_scan
    return 0;
  }

  //init
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  //make file directory
  char file[MAX_PATH]="";
  char ip[IPV6_SIZE_MAX],name[MAX_PATH];
  snprintf(file,MAX_PATH,"%s\\WINDOWS\\system32\\drivers\\etc\\hosts",getenv("SYSTEMDRIVE"));

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"DNS\";\"file\";\"ip\";\"name\";\"last_file_update\";\"malware_check\";\"session_id\";\r\n");
  #endif // CMD_LINE_ONLY_NO_DB

  //open host file and read all hosts
  HANDLE Hfic = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (Hfic != INVALID_HANDLE_VALUE)
  {
    DWORD taille_fic = GetFileSize(Hfic,NULL);
    if (taille_fic != INVALID_FILE_SIZE)
    {
      char *buffer = (char *) HeapAlloc(GetProcessHeap(), 0, taille_fic+1);
      if (buffer != NULL)
      {
        //get last update
        char last_file_update[DATE_SIZE_MAX]="";
        FILETIME LastWriteTime;
        if(GetFileTime(Hfic,NULL,NULL,&LastWriteTime))filetimeToString_GMT(LastWriteTime, last_file_update, DATE_SIZE_MAX);

        //read data line by line
        DWORD copiee;
        char lines[MAX_PATH];
        if (ReadFile(Hfic, buffer, taille_fic,&copiee,0))
        {
          char *r = buffer;
          char *s,*c;
          while (*r)
          {
            //read line
            memset(lines,0,MAX_PATH);
            snprintf(lines,MAX_PATH,"%s",r);
            s = lines;
            while (*s && *s != '\r')s++;
            *s = 0;
            r = r+strlen(lines)+2;

            //comment or not :)
            if (lines[0]!='#' && strlen(lines) > 8)
            {
              //get IP
              snprintf(ip,IPV6_SIZE_MAX,"%s",lines);
              c = ip;

              while (*c && *c != ' ' && *c!= '\t' && (*c == '.' || *c == ':' || (*c<='9' && *c>='0')))c++;
              if (*c)
              {
                *c = 0;
                //get name
                c = lines+strlen(ip);
                while (*c && (*c == ' ' || *c == '\t'))c++;

                memset(name,0,MAX_PATH);
                snprintf(name,MAX_PATH,"%s",c);
                addHosttoDB(file, ip, name, last_file_update,session_id,db);
              }
            }
          }
        }
        HeapFree(GetProcessHeap(), 0, buffer);
      }
    }
    CloseHandle(Hfic);
  }
  //get cache (Only ok bifore lats W7 update)
  if (DnsGetCacheDataTable != NULL)
  {
    PDNS_RECORD pcache = NULL;
    DNS_RECORD* dnsRecords = NULL, *dnsr;
    IN_ADDR ipAddress;
    char last_file_update[DATE_SIZE_MAX]="";

    if (DnsGetCacheDataTable(&pcache) == TRUE)
    {
      PDNS_RECORD cache = pcache;
      while (cache)
      {
        memset(name,0,MAX_PATH);
        snprintf(name,MAX_PATH,"%S",cache->pName);
        if (name[0] != 0)
        {
          //get IP + TTL
          if(DnsQuery(name,DNS_TYPE_A,0,NULL,&dnsRecords,NULL) == ERROR_SUCCESS)
          {
            dnsr = dnsRecords;
            while (dnsr != NULL)
            {
              ipAddress.S_un.S_addr = dnsr->Data.A.IpAddress;
              if (inet_ntoa(ipAddress) != NULL)
              {
                snprintf(ip,IP_SIZE_MAX,"%s",inet_ntoa(ipAddress));
                snprintf(last_file_update,DATE_SIZE_MAX,"%lu (s)",dnsr->dwTtl);
                addHosttoDB("", ip, name, last_file_update,session_id,db);
              }
              dnsr = dnsr->pNext;
            }
            //free
            DnsRecordListFree(dnsRecords,DnsFreeRecordList);
          }
        }
        cache = cache->pNext;
      }
    }
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[local_id], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[local_id] = 0;
  return 0;
}
