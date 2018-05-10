//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addProcesstoDB(char *process, char *pid, char *path, char *cmd,
                    char *owner, char *rid, char *sid, char *start_date,
                    char *protocol, char *ip_src, char *port_src,
                    char *ip_dst, char *port_dst, char *state,
                    char *hidden,char *parent_process, char *parent_pid, char *sha256, char * sign, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_process (process,pid,path,cmd,owner,"
           "rid,sid,start_date,protocol,ip_src,port_src,ip_dst,port_dst,state,hidden,parent_process,parent_pid,sha256,sign,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           process,pid,path,cmd,owner,rid,sid,start_date,protocol,ip_src,port_src,ip_dst,port_dst,state,hidden,parent_process,parent_pid,sha256,sign,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Process\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         process,pid,path,cmd,owner,rid,sid,start_date,protocol,ip_src,port_src,ip_dst,port_dst,state,hidden,parent_process,parent_pid,sha256,sign,session_id);
  #endif
}
//------------------------------------------------------------------------------
BOOL GetProcessArg(HANDLE hProcess, char* arg, unsigned int size)
{
  arg[0]=0;
  BOOL ret = FALSE;

  PROCESS_BASIC_INFORMATION pbi;
  __PEB PEB;
  __INFOBLOCK Block;

  wchar_t *cmd = NULL;

  if (MyNtQueryInformationProcess != NULL)
  {
    DWORD dwSize=0;
    if(MyNtQueryInformationProcess(hProcess, ProcessBasicInformation, &pbi, sizeof(pbi), &dwSize) == 0)
    {
      //lecture de la mémoire
      if (ReadProcessMemory(hProcess, pbi.PebBaseAddress, &PEB, sizeof(PEB), &dwSize) != 0)
      {
        if (ReadProcessMemory(hProcess, (LPVOID)PEB.dwInfoBlockAddress, &Block, sizeof(Block), &dwSize) != 0)
        {
          cmd = NULL;
          cmd = (wchar_t*)malloc(Block.wMaxLength+1 * sizeof(wchar_t));
          if (cmd!=NULL)
          {
            if (ReadProcessMemory(hProcess, (LPVOID)Block.dwCmdLineAddress, cmd, Block.wMaxLength, &dwSize) != 0)
            {
              snprintf(arg,size,"%S",cmd);
              ret = TRUE;
            }
            free(cmd);
          }
        }
      }
    }
  }

  return ret;
}
//------------------------------------------------------------------------------
void GetProcessOwner(DWORD pid, char *owner, char *rid, char *sid, DWORD size_max)
{
  //init
  HANDLE hProcess = OpenProcess(READ_CONTROL,0,pid);
  owner[0] = 0;
  rid[0] = 0;
  sid[0] = 0;

  //user
  if (hProcess!=NULL)
  {
    PSECURITY_DESCRIPTOR psDesc = (PSECURITY_DESCRIPTOR)GlobalAlloc(GMEM_FIXED,MAX_PATH);
    DWORD dwSize = 0;
    PSID psid=0;
    BOOL proprio = FALSE;
    if (psDesc != NULL)
    {
      //info de l'objet
      if (GetKernelObjectSecurity(hProcess,OWNER_SECURITY_INFORMATION,psDesc,1000,&dwSize))
      {
        //security descriptor
        if (GetSecurityDescriptorOwner(psDesc,&psid, &proprio))
        {
          SidtoUser(psid, owner, rid, sid, size_max);
        }
      }
      GlobalFree(psDesc);
    }
    CloseHandle(hProcess);
  }
}
//------------------------------------------------------------------------------
BOOL GetNameFromIP(char *ip, char *ip_name, unsigned int name_sz_max)
{
  ip_name[0] = 0;

  if (ip[0] == '*') return FALSE;

  struct hostent* remoteHost;
  struct in_addr in;
  in.s_addr = inet_addr(ip);
  if ((remoteHost=gethostbyaddr((char *)&in, 4, AF_INET))!=0)
  {
   snprintf(ip_name,name_sz_max,"%s",remoteHost->h_name);
   if (ip_name[0] != '\\' && ip_name[1] != '\\')return TRUE;
  }

  return FALSE;
}
//------------------------------------------------------------------------------
DWORD GetPortsFromPID(DWORD pid, LINE_PROC_ITEM *port_line, unsigned int nb_item_max,unsigned int taille_max_line)
{
  DWORD nb_item = 0;

  if (MyGetExtendedTcpTable ==NULL || MyGetExtendedUdpTable ==NULL)
  {
    return 0;
  }

  //TCP
  PVOID pTCPTable = NULL;
  DWORD i, size = 0;
  if (MyGetExtendedTcpTable(pTCPTable, &size, TRUE, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0) == ERROR_INSUFFICIENT_BUFFER)
  {
    pTCPTable = malloc(size+1);
    if (pTCPTable != NULL)
    {
      if (MyGetExtendedTcpTable(pTCPTable, &size, TRUE, AF_INET, TCP_TABLE_OWNER_MODULE_ALL, 0) == NO_ERROR)
      {
        for(i=0; i<((PMIB_TCPTABLE_OWNER_MODULE)pTCPTable)->dwNumEntries && nb_item<nb_item_max;i++)
        {
          MIB_TCPROW_OWNER_MODULE module = ((PMIB_TCPTABLE_OWNER_MODULE)pTCPTable)->table[i];
          if (pid == module.dwOwningPid)
          {
            strncpy(port_line[nb_item].protocol,"TCP",taille_max_line);

            snprintf(port_line[nb_item].IP_src,taille_max_line,"%s",inet_ntoa(*(struct in_addr *)&module.dwLocalAddr));
            snprintf(port_line[nb_item].IP_dst,taille_max_line,"%s",inet_ntoa(*(struct in_addr *)&module.dwRemoteAddr));

            if (nb_item>0) //optimize perf !!!
            {
              if (!strcmp(port_line[nb_item].IP_src,port_line[nb_item-1].IP_src))
              {
               snprintf(port_line[nb_item].name_src,SIZE_ITEMS_PORT_MAX,"%s",port_line[nb_item-1].name_src);
              }else GetNameFromIP(port_line[nb_item].IP_src, port_line[nb_item].name_src, SIZE_ITEMS_PORT_MAX);

              if (!strcmp(port_line[nb_item].IP_dst,port_line[nb_item-1].IP_dst))
              {
               snprintf(port_line[nb_item].name_dst,SIZE_ITEMS_PORT_MAX,"%s",port_line[nb_item-1].name_dst);
              }else GetNameFromIP(port_line[nb_item].IP_dst, port_line[nb_item].name_dst, SIZE_ITEMS_PORT_MAX);
            }else
            {
              GetNameFromIP(port_line[nb_item].IP_src, port_line[nb_item].name_src, SIZE_ITEMS_PORT_MAX);
              GetNameFromIP(port_line[nb_item].IP_dst, port_line[nb_item].name_dst, SIZE_ITEMS_PORT_MAX);
            }

            snprintf(port_line[nb_item].Port_src,taille_max_line,"%d",htons((u_short) module.dwLocalPort));
            snprintf(port_line[nb_item].Port_dst,taille_max_line,"%d",htons((u_short) module.dwRemotePort));

            switch(module.dwState)
            {
              case 1:strncpy(   port_line[nb_item].state,"CLOSED"       ,taille_max_line);break;
              case 2:strncpy(   port_line[nb_item].state,"LISTENING"    ,taille_max_line);break;
              case 3:strncpy(   port_line[nb_item].state,"SYN_SENT"     ,taille_max_line);break;
              case 4:strncpy(   port_line[nb_item].state,"SYN_RECIVED"  ,taille_max_line);break;
              case 5:strncpy(   port_line[nb_item].state,"ESTABLISHED"  ,taille_max_line);break;
              case 6:
              case 7:strncpy(   port_line[nb_item].state,"FIN_WAIT"     ,taille_max_line);break;
              case 8:strncpy(   port_line[nb_item].state,"CLOSE_WAIT"   ,taille_max_line);break;
              case 9:strncpy(   port_line[nb_item].state,"CLOSING"      ,taille_max_line);break;
              case 10:strncpy(  port_line[nb_item].state,"LAST_ACK"     ,taille_max_line);break;
              case 11:strncpy(  port_line[nb_item].state,"TIME_WAIT"    ,taille_max_line);break;
              case 12:strncpy(  port_line[nb_item].state,"DELETE_TCB"   ,taille_max_line);break;
              default:snprintf( port_line[nb_item].state,taille_max_line,"UNKNOW (%lu)",module.dwState);break;
            }
            nb_item++;
          }
        }
      }
      free(pTCPTable);
    }
  }

  //UDP
  PVOID pUDPTable = NULL;
  size = 0;
  if (MyGetExtendedUdpTable(pUDPTable, &size, TRUE, AF_INET, UDP_TABLE_OWNER_PID, 0) == ERROR_INSUFFICIENT_BUFFER)
  {
    pUDPTable = malloc(size+1);
    if(pUDPTable != NULL)
    {
      if (MyGetExtendedUdpTable(pUDPTable, &size, TRUE, AF_INET, UDP_TABLE_OWNER_PID, 0) == NO_ERROR)
      {
        PMIB_UDPTABLE_EX UDP_table = (PMIB_UDPTABLE_EX)pUDPTable;
        for (i=0; i<UDP_table->dwNumEntries && nb_item<nb_item_max;i++)
        {
          if (pid == UDP_table->table[i].dwProcessId)
          {
            strncpy(port_line[nb_item].protocol,"UDP",taille_max_line);
            snprintf(port_line[nb_item].IP_src,taille_max_line,"%s",inet_ntoa(*(struct in_addr *)&UDP_table->table[i].dwLocalAddr));
            strncpy(port_line[nb_item].IP_dst,"*.*",taille_max_line);
            snprintf(port_line[nb_item].Port_src,taille_max_line,"%d",htons((u_short) UDP_table->table[i].dwLocalPort));
            port_line[nb_item].Port_dst[0]=0;
            strncpy(port_line[nb_item].state,"LISTENING",taille_max_line);
            nb_item++;
          }
        }
      }
      free(pUDPTable);
    }
  }
  return nb_item;
}
//------------------------------------------------------------------------------
void EnumProcessAndThread(DWORD nb_process, PROCESS_INFOS_ARGS *process_info,unsigned int session_id,sqlite3 *db)
{
  HANDLE hProcess;
  DWORD d_pid, i, j, k, cbNeeded;
  BOOL ok;
  LINE_PROC_ITEM *port_line;
  HMODULE hMod[MAX_LINE_SIZE];
  FILETIME lpCreationTime, lpExitTime, lpKernelTime, lpUserTime;
  char process[DEFAULT_TMP_SIZE],
       pid[DEFAULT_TMP_SIZE],
       path[MAX_LINE_SIZE],
       cmd[MAX_LINE_SIZE],
       owner[DEFAULT_TMP_SIZE],
       rid[DEFAULT_TMP_SIZE],
       sid[DEFAULT_TMP_SIZE],
       start_date[DATE_SIZE_MAX],
       verified[MAX_LINE_SIZE],
       h_sha256[MAX_LINE_SIZE];

  char src_name[MAX_LINE_SIZE];
  char dst_name[MAX_LINE_SIZE];

  port_line = (LINE_PROC_ITEM *) malloc(sizeof(LINE_PROC_ITEM)*MAX_LINE_SIZE);
  if (port_line == NULL)return;

  //force enumerate all process by id !
  for (d_pid=FIRST_PROCESS_ID;d_pid<LAST_PROCESS_ID && start_scan;d_pid++)
  {
    //open process info
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,0,d_pid);
    if (hProcess == NULL)continue;

    //cmd
    cmd[0]=0;
    GetProcessArg(hProcess, cmd, MAX_LINE_SIZE);

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
        else if (strcmp(cmd,process_info[i].args) == 0){ok = FALSE;break;}
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
      if (EnumProcessModules(hProcess,hMod, MAX_LINE_SIZE,&cbNeeded))
      {
        if (GetModuleFileNameEx(hProcess,hMod[0],path,MAX_LINE_SIZE) == 0)path[0] = 0;
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

      convertStringToSQL(path, MAX_LINE_SIZE);
      convertStringToSQL(cmd, MAX_LINE_SIZE);

      //sha256 + signed
      GetSHAandVerifyFromPathFile(path, h_sha256, verified, MAX_LINE_SIZE);

      //add items !
      if (j == 0)addProcesstoDB(process, pid, path, cmd, owner, rid, sid, start_date,"", "", "","", "", "", "X", "", "",h_sha256, verified,session_id,db);
      else
      {
        for (k=0;k<j;k++)
        {
          if (port_line[k].name_src[0] != 0)snprintf(src_name,MAX_LINE_SIZE,"%s:%s",port_line[k].IP_src,port_line[k].name_src);
          else snprintf(src_name,MAX_LINE_SIZE,"%s",port_line[k].IP_src);

          if (port_line[k].name_dst[0] != 0)snprintf(dst_name,MAX_LINE_SIZE,"%s:%s",port_line[k].IP_dst,port_line[k].name_dst);
          else snprintf(dst_name,MAX_LINE_SIZE,"%s",port_line[k].IP_dst);

          addProcesstoDB(process, pid, path, cmd, owner, rid, sid, start_date,
                                port_line[k].protocol, src_name, port_line[k].Port_src,
                                dst_name, port_line[k].Port_dst, port_line[k].state, "X", "", "",h_sha256, verified,session_id,db);
        }
      }
    }
    CloseHandle(hProcess);
  }
  free(port_line);
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_process(LPVOID lParam)
{
  //check if local or not :)
  if (!LOCAL_SCAN || WINE_OS)
  {
    h_thread_test[(unsigned int)lParam] = 0;
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
    return 0;
  }

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Process\";\"PID\";\"Path\";\"Command\";\"Owner\";\"RID\";\"SID\";\"Start_date\";\"Protocol\";\"IP_src\";\"Port_src\";\"IP_dst\";\"Port_dst\";\"State\";\"Hidden\";\"Parent_process\";\"Parent_PID\";\"session_id\";\r\n");
  #endif
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  PROCESSENTRY32 pe = {sizeof(PROCESSENTRY32)};
  HANDLE hCT = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS|TH32CS_SNAPTHREAD, 0);
  if (hCT==INVALID_HANDLE_VALUE)return 0;

  DWORD cbNeeded, k, j, nb_process=0;
  HANDLE hProcess, parent_hProcess;
  HMODULE hMod[MAX_LINE_SIZE];
  FILETIME lpCreationTime, lpExitTime, lpKernelTime, lpUserTime;
  LINE_PROC_ITEM *port_line;
  char process[DEFAULT_TMP_SIZE],
       pid[DEFAULT_TMP_SIZE],
       path[MAX_LINE_SIZE],
       cmd[MAX_LINE_SIZE],
       owner[DEFAULT_TMP_SIZE],
       rid[DEFAULT_TMP_SIZE],
       sid[DEFAULT_TMP_SIZE],
       start_date[DATE_SIZE_MAX],
       parent_pid[DEFAULT_TMP_SIZE],
       parent_path[MAX_LINE_SIZE],
       verified[MAX_LINE_SIZE],
       h_sha256[MAX_LINE_SIZE];

  char src_name[MAX_LINE_SIZE];
  char dst_name[MAX_LINE_SIZE];

  PROCESS_INFOS_ARGS *process_infos;
  process_infos = (PROCESS_INFOS_ARGS *)malloc(sizeof(PROCESS_INFOS_ARGS)*MAX_PATH);

  port_line = (LINE_PROC_ITEM *) malloc(sizeof(LINE_PROC_ITEM)*MAX_LINE_SIZE);
  if (port_line == NULL)return 0;

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  while(Process32Next(hCT, &pe) && start_scan)
  {
    //open process info
    hProcess = OpenProcess(PROCESS_QUERY_INFORMATION|PROCESS_VM_READ,0,pe.th32ProcessID);
    if (hProcess == NULL)continue;

    //process
    snprintf(process,DEFAULT_TMP_SIZE,"%s",pe.szExeFile);

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
    if (nb_process<MAX_PATH && process_infos != NULL)
    {
      process_infos[nb_process].pid = pe.th32ProcessID;
      snprintf(process_infos[nb_process].args,MAX_LINE_SIZE,"%s",cmd);
      nb_process++;
    }
    convertStringToSQL(path, MAX_LINE_SIZE);
    convertStringToSQL(cmd, MAX_LINE_SIZE);

    //sha256 + signed
    GetSHAandVerifyFromPathFile(path, h_sha256, verified, MAX_LINE_SIZE);

    //add items !
    if (j == 0)addProcesstoDB(process, pid, path, cmd, owner, rid, sid, start_date,"", "", "","", "", "",""  , parent_path, parent_pid,h_sha256, verified,session_id,db);
    else
    {
      for (k=0;k<j;k++)
      {
        if (port_line[k].name_src[0] != 0)snprintf(src_name,MAX_LINE_SIZE,"%s:%s",port_line[k].IP_src,port_line[k].name_src);
        else snprintf(src_name,MAX_LINE_SIZE,"%s",port_line[k].IP_src);

        if (port_line[k].name_dst[0] != 0)snprintf(dst_name,MAX_LINE_SIZE,"%s:%s",port_line[k].IP_dst,port_line[k].name_dst);
        else snprintf(dst_name,MAX_LINE_SIZE,"%s",port_line[k].IP_dst);

        addProcesstoDB(process, pid, path, cmd, owner, rid, sid, start_date,
                              port_line[k].protocol, src_name, port_line[k].Port_src,
                              dst_name, port_line[k].Port_dst, port_line[k].state, "", parent_path, parent_pid,h_sha256, verified,session_id,db);
      }
    }
    CloseHandle(hProcess);
  }
  free(process_infos);
  free(port_line);

  //verify shadow process !!!
  if(process_infos != NULL) EnumProcessAndThread(nb_process, process_infos,session_id,db);

  CloseHandle(hCT);
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
