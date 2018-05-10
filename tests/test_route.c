//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRoutetoDB(char *destination, char *netmask, char *gateway, DWORD metric, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_route (destination,netmask,gateway,metric,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%lu\",%d);",
           destination,netmask,gateway,metric,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Route\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         destination,netmask,gateway,metric,session_id);
  #endif
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_route(LPVOID lParam)
{
  //check if local or not :)
  if (!LOCAL_SCAN)
  {
    h_thread_test[(unsigned int)lParam] = 0;
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
    return 0;
  }

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Route\";\"destination\";\"netmask\";\"gateway\";\"metric\";\"session_id\";\r\n");
  #endif
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  //load route table);
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  if (MyGetIpForwardTable!= NULL)
  {
    //load all table
    PMIB_IPFORWARDTABLE pIpForwardTable = (MIB_IPFORWARDTABLE*) HeapAlloc(GetProcessHeap(), 0, (sizeof(MIB_IPFORWARDTABLE)));
    if (pIpForwardTable != NULL)
    {
      //alloc memory
      DWORD i, dwSize = 0;
      if (MyGetIpForwardTable(pIpForwardTable, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER)
      {
        HeapFree(GetProcessHeap(), 0,pIpForwardTable);
        pIpForwardTable = (MIB_IPFORWARDTABLE*) HeapAlloc(GetProcessHeap(), 0,dwSize);

        if (pIpForwardTable == NULL)
        {
          return 0;
        }
      }

      //get datas
      if (MyGetIpForwardTable(pIpForwardTable, &dwSize, 0) == NO_ERROR)
      {
        struct in_addr IpAddr_dst;
        struct in_addr IpAddr_msk;
        struct in_addr IpAddr_hop;

        char destination[IP_SIZE_MAX];
        char netmask[IP_SIZE_MAX];
        char gateway[IP_SIZE_MAX];

        for (i = 0; i < (int) pIpForwardTable->dwNumEntries; i++)
        {
          IpAddr_dst.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardDest;
          snprintf(destination,IP_SIZE_MAX,"%s",inet_ntoa(IpAddr_dst));

          IpAddr_msk.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardMask;
          snprintf(netmask,IP_SIZE_MAX,"%s",inet_ntoa(IpAddr_msk));

          IpAddr_hop.S_un.S_addr = (u_long) pIpForwardTable->table[i].dwForwardNextHop;
          snprintf(gateway,IP_SIZE_MAX,"%s",inet_ntoa(IpAddr_hop));

          addRoutetoDB(destination,
                       netmask,
                       gateway,
                       pIpForwardTable->table[i].dwForwardMetric1,session_id,db);
        }
      }
      HeapFree(GetProcessHeap(), 0,pIpForwardTable);
    }
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
