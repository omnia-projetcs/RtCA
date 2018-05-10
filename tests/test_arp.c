//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addARPtoDB(char *ip, char *mac, char *type, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_arp (ip,mac,type,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",%d);",
           ip,mac,type,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"ARP\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         ip,mac,type,session_id);
  #endif
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_arp(LPVOID lParam)
{
  //check if local or not :)
  if (LOCAL_SCAN == FALSE)
  {
    h_thread_test[(unsigned int)lParam] = 0;
    check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
    return 0;
  }

  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  //load route table);
  DWORD dwSize = 0,i;
  //allocate momery
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  if (GetIpNetTable(NULL, &dwSize, 0) == ERROR_INSUFFICIENT_BUFFER)
  {
    if (dwSize > 0)
    {
      PMIB_IPNETTABLE pIpNetTable = (MIB_IPNETTABLE *) malloc (dwSize);
      if (pIpNetTable != NULL)
      {
        //Get datas
        if(GetIpNetTable(pIpNetTable, &dwSize, 0) == NO_ERROR)
        {
          char ip[IP_SIZE_MAX], mac[MAC_SIZE], type[DEFAULT_TMP_SIZE];


          #ifdef CMD_LINE_ONLY_NO_DB
          printf("\"ARP\";\"ip\";\"mac\";\"type\";\"session_id\";\r\n");
          #endif
          for (i=0; i<pIpNetTable->dwNumEntries; i++)
          {
            snprintf(ip,IP_SIZE_MAX,"%s",inet_ntoa(*(struct in_addr*)&pIpNetTable->table[i].dwAddr));
            snprintf(mac,MAC_SIZE,"%02X:%02X:%02X:%02X:%02X:%02X",
                     pIpNetTable->table[i].bPhysAddr[0] & 0xFF,
                     pIpNetTable->table[i].bPhysAddr[1] & 0xFF,
                     pIpNetTable->table[i].bPhysAddr[2] & 0xFF,
                     pIpNetTable->table[i].bPhysAddr[3] & 0xFF,
                     pIpNetTable->table[i].bPhysAddr[4] & 0xFF,
                     pIpNetTable->table[i].bPhysAddr[5] & 0xFF);

            switch(pIpNetTable->table[i].dwType)
            {
              case 1:addARPtoDB(ip, mac, "MIB_IPNET_TYPE_OTHER", session_id, db);break;
              case 2:addARPtoDB(ip, mac, "MIB_IPNET_TYPE_INVALID", session_id, db);break;
              case 3:addARPtoDB(ip, mac, "MIB_IPNET_TYPE_DYNAMIC", session_id, db);break;
              case 4:addARPtoDB(ip, mac, "MIB_IPNET_TYPE_STATIC", session_id, db);break;
              default:
                snprintf(type,DEFAULT_TMP_SIZE,"UNKNOW (code:%ld)",pIpNetTable->table[i].dwType);
                addARPtoDB(ip, mac, type, session_id, db);break;
            }
          }
        }
        free(pIpNetTable);
      }
    }
  }
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);

  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
