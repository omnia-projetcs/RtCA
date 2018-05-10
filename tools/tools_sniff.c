//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : http://code.google.com/p/omnia-projetcs/
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
#include "tools_network/_entetes.h"
//------------------------------------------------------------------------------
BOOL FilterExist(char *filter)
{
  if (SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_LB_FILTRE, LB_FINDSTRINGEXACT, (WPARAM)-1, (LPARAM)filter)== LB_ERR) return FALSE;
  else return TRUE;
}
//------------------------------------------------------------------------------
BOOL ValideChDesc(char *data, unsigned size_test)
{
  unsigned int i;
  for (i=0;i<size_test;i++)
  {
    if ((data[i]<31 || data[i]>126) && data[i] != '\r' && data[i] != '\n' && data[i] != 0)return FALSE;
  }
  return TRUE;
}
//------------------------------------------------------------------------------
void GetPortDesc(unsigned int port, BOOL udp, char *desc, unsigned int desc_size_max)
{
  //get service port info
  if (udp) //udp
  {
    snprintf(desc,desc_size_max,"%05d/UDP",port);

    switch(port)
    {
      case 7  :strncat(desc," echo\0",desc_size_max);break;
      case 13 :strncat(desc," daytime\0",desc_size_max);break;
      case 20 :strncat(desc," ftp-datas\0",desc_size_max);break;
      case 21 :strncat(desc," ftp\0",desc_size_max);break;
      case 22 :strncat(desc," ssh\0",desc_size_max);break;
      case 23 :strncat(desc," telnet\0",desc_size_max);break;
      case 25 :strncat(desc," smtp\0",desc_size_max);break;
      case 26 :strncat(desc," rsftp\0",desc_size_max);break;
      case 31 :strncat(desc," msg-auth\0",desc_size_max);break;
      case 38 :strncat(desc," rap\0",desc_size_max);break;
      case 43 :strncat(desc," whois\0",desc_size_max);break;
      case 49 :strncat(desc," tacas\0",desc_size_max);break;
      case 53 :strncat(desc," dns\0",desc_size_max);break;
      case 65 :strncat(desc," tacas-ds\0",desc_size_max);break;
      case 67 :strncat(desc," dhcp\0",desc_size_max);break;
      case 68 :strncat(desc," dhcpc\0",desc_size_max);break;
      case 69 :strncat(desc," tftp\0",desc_size_max);break;
      case 70 :strncat(desc," gopher\0",desc_size_max);break;
      case 79 :strncat(desc," finger\0",desc_size_max);break;
      case 80 :strncat(desc," http\0",desc_size_max);break;
      case 88 :strncat(desc," kerberos\0",desc_size_max);break;
      case 92 :strncat(desc," npp\0",desc_size_max);break;
      case 93 :strncat(desc," dcp\0",desc_size_max);break;
      case 101:strncat(desc," hostame\0",desc_size_max);break;
      case 110:strncat(desc," pop3\0",desc_size_max);break;
      case 111:strncat(desc," rpcbind\0",desc_size_max);break;
      case 113:strncat(desc," auth\0",desc_size_max);break;
      case 115:strncat(desc," sftp\0",desc_size_max);break;
      case 118:strncat(desc," sqlsrv\0",desc_size_max);break;
      case 119:strncat(desc," nntp\0",desc_size_max);break;
      case 123:strncat(desc," ntp\0",desc_size_max);break;
      case 134:strncat(desc," ingres\0",desc_size_max);break;
      case 135:strncat(desc," msrpc\0",desc_size_max);break;
      case 137:strncat(desc," netbios-ns\0",desc_size_max);break;
      case 138:strncat(desc," netbios-dgm\0",desc_size_max);break;
      case 139:strncat(desc," netbios-ssn\0",desc_size_max);break;
      case 143:strncat(desc," imap\0",desc_size_max);break;
      case 156:strncat(desc," sqlsrv\0",desc_size_max);break;
      case 161:strncat(desc," snmp\0",desc_size_max);break;
      case 162:strncat(desc," snmp-trap\0",desc_size_max);break;
      case 256:strncat(desc," rap\0",desc_size_max);break;
      case 386:strncat(desc," asa\0",desc_size_max);break;
      case 389:strncat(desc," ldap\0",desc_size_max);break;
      case 443:strncat(desc," https\0",desc_size_max);break;
      case 445:strncat(desc," microsoft-ds\0",desc_size_max);break;
      case 465:strncat(desc," smtps\0",desc_size_max);break;
      case 529:strncat(desc," irc\0",desc_size_max);break;
      case 533:strncat(desc," netwall\0",desc_size_max);break;
      case 543:strncat(desc," klogin\0",desc_size_max);break;
      case 544:strncat(desc," kshell\0",desc_size_max);break;
      case 556:strncat(desc," remote-fs\0",desc_size_max);break;
      case 636:strncat(desc," ldapssl\0",desc_size_max);break;
      case 749:strncat(desc," kerberos-adm\0",desc_size_max);break;
      case 750:strncat(desc," kerberos\0",desc_size_max);break;
      case 751:strncat(desc," kerberos-master\0",desc_size_max);break;
      case 990:strncat(desc," ftps\0",desc_size_max);break;
      case 992:strncat(desc," telnets\0",desc_size_max);break;
      case 993:strncat(desc," imaps\0",desc_size_max);break;
      case 994:strncat(desc," ircs\0",desc_size_max);break;
      case 995:strncat(desc," pop3s\0",desc_size_max);break;
      case 1433:strncat(desc," ms-sql\0",desc_size_max);break;
      case 1494:strncat(desc," citrix-ica\0",desc_size_max);break;
      case 1512:strncat(desc," wins\0",desc_size_max);break;
      case 1521:strncat(desc," oracle\0",desc_size_max);break;
      case 1525:strncat(desc," oracle\0",desc_size_max);break;
      case 2049:strncat(desc," nfs\0",desc_size_max);break;
      case 2967:strncat(desc," symantec-av\0",desc_size_max);break;
      case 3389:strncat(desc," ms-term-server\0",desc_size_max);break;
      case 4000:strncat(desc," icq\0",desc_size_max);break;
      case 4666:strncat(desc," edonkey\0",desc_size_max);break;
      case 5000:strncat(desc," upnp\0",desc_size_max);break;
      case 5060:strncat(desc," sip\0",desc_size_max);break;
      case 6000:strncat(desc," x11\0",desc_size_max);break;
    }

  }else
  {
    snprintf(desc,desc_size_max,"%05d/TCP",port);

    switch(port)
    {
      case 7  :strncat(desc," echo\0",desc_size_max);break;
      case 13 :strncat(desc," daytime\0",desc_size_max);break;
      case 20 :strncat(desc," ftp-datas\0",desc_size_max);break;
      case 21 :strncat(desc," ftp\0",desc_size_max);break;
      case 22 :strncat(desc," ssh\0",desc_size_max);break;
      case 23 :strncat(desc," telnet\0",desc_size_max);break;
      case 25 :strncat(desc," smtp\0",desc_size_max);break;
      case 26 :strncat(desc," rsftp\0",desc_size_max);break;
      case 31 :strncat(desc," msg-auth\0",desc_size_max);break;
      case 38 :strncat(desc," rap\0",desc_size_max);break;
      case 43 :strncat(desc," whois\0",desc_size_max);break;
      case 49 :strncat(desc," tacas\0",desc_size_max);break;
      case 53 :strncat(desc," dns\0",desc_size_max);break;
      case 65 :strncat(desc," tacas-ds\0",desc_size_max);break;
      case 67 :strncat(desc," dhcp\0",desc_size_max);break;
      case 68 :strncat(desc," dhcpc\0",desc_size_max);break;
      case 69 :strncat(desc," tftp\0",desc_size_max);break;
      case 70 :strncat(desc," gopher\0",desc_size_max);break;
      case 79 :strncat(desc," finger\0",desc_size_max);break;
      case 80 :strncat(desc," http\0",desc_size_max);break;
      case 88 :strncat(desc," kerberos\0",desc_size_max);break;
      case 92 :strncat(desc," npp\0",desc_size_max);break;
      case 93 :strncat(desc," dcp\0",desc_size_max);break;
      case 101:strncat(desc," hostame\0",desc_size_max);break;
      case 110:strncat(desc," pop3\0",desc_size_max);break;
      case 111:strncat(desc," rpcbind\0",desc_size_max);break;
      case 113:strncat(desc," auth\0",desc_size_max);break;
      case 115:strncat(desc," sftp\0",desc_size_max);break;
      case 118:strncat(desc," sqlsrv\0",desc_size_max);break;
      case 119:strncat(desc," nntp\0",desc_size_max);break;
      case 123:strncat(desc," ntp\0",desc_size_max);break;
      case 134:strncat(desc," ingres\0",desc_size_max);break;
      case 135:strncat(desc," msrpc\0",desc_size_max);break;
      case 137:strncat(desc," netbios-ns\0",desc_size_max);break;
      case 138:strncat(desc," netbios-dgm\0",desc_size_max);break;
      case 139:strncat(desc," netbios-ssn\0",desc_size_max);break;
      case 143:strncat(desc," imap\0",desc_size_max);break;
      case 161:strncat(desc," snmp\0",desc_size_max);break;
      case 162:strncat(desc," snmp-trap\0",desc_size_max);break;
      case 386:strncat(desc," asa\0",desc_size_max);break;
      case 389:strncat(desc," ldap\0",desc_size_max);break;
      case 443:strncat(desc," https\0",desc_size_max);break;
      case 445:strncat(desc," microsoft-ds\0",desc_size_max);break;
      case 465:strncat(desc," smtps\0",desc_size_max);break;
      case 512:strncat(desc," exec\0",desc_size_max);break;
      case 513:strncat(desc," login\0",desc_size_max);break;
      case 533:strncat(desc," netwall\0",desc_size_max);break;
      case 543:strncat(desc," klogin\0",desc_size_max);break;
      case 544:strncat(desc," kshell\0",desc_size_max);break;
      case 556:strncat(desc," remote-fs\0",desc_size_max);break;
      case 636:strncat(desc," ldapssl\0",desc_size_max);break;
      case 749:strncat(desc," kerberos-adm\0",desc_size_max);break;
      case 750:strncat(desc," kerberos\0",desc_size_max);break;
      case 751:strncat(desc," kerberos-master\0",desc_size_max);break;
      case 783:strncat(desc," spamassassin\0",desc_size_max);break;
      case 873:strncat(desc," rsync\0",desc_size_max);break;
      case 893:strncat(desc," sun-manageconsole\0",desc_size_max);break;
      case 990:strncat(desc," ftps\0",desc_size_max);break;
      case 992:strncat(desc," telnets\0",desc_size_max);break;
      case 993:strncat(desc," imaps\0",desc_size_max);break;
      case 994:strncat(desc," ircs\0",desc_size_max);break;
      case 995:strncat(desc," pop3s\0",desc_size_max);break;
      case 1025:strncat(desc," nfs\0",desc_size_max);break;
      case 1112:strncat(desc," msql\0",desc_size_max);break;
      case 1241:strncat(desc," nessus\0",desc_size_max);break;
      case 1433:strncat(desc," ms-sql\0",desc_size_max);break;
      case 1494:strncat(desc," citrix-ica\0",desc_size_max);break;
      case 1521:strncat(desc," oracle\0",desc_size_max);break;
      case 1723:strncat(desc," pptp\0",desc_size_max);break;
      case 1755:strncat(desc," wms\0",desc_size_max);break;
      case 1761:
      case 1762:
      case 1763:strncat(desc," landesk-rc\0",desc_size_max);break;
      case 2049:strncat(desc," nfs\0",desc_size_max);break;
      case 2967:strncat(desc," symantec-av\0",desc_size_max);break;
      case 3000:strncat(desc," ppp\0",desc_size_max);break;
      case 3001:strncat(desc," nessus\0",desc_size_max);break;
      case 3128:strncat(desc," squid-http\0",desc_size_max);break;
      case 3306:strncat(desc," mysql\0",desc_size_max);break;
      case 3389:strncat(desc," ms-term-server\0",desc_size_max);break;
      case 3690:strncat(desc," svn\0",desc_size_max);break;
      case 4662:strncat(desc," edonkey\0",desc_size_max);break;
      case 5000:strncat(desc," upnp\0",desc_size_max);break;
      case 5060:strncat(desc," sip\0",desc_size_max);break;
      case 5679:strncat(desc," activesync\0",desc_size_max);break;
      case 5800:strncat(desc," vnc-http\0",desc_size_max);break;
      case 5900:strncat(desc," vnc\0",desc_size_max);break;
      case 6000:strncat(desc," x11\0",desc_size_max);break;
      case 8080:strncat(desc," http-proxy\0",desc_size_max);break;
    }
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI AddIp(LPVOID lParam)
{
  long int id = (unsigned long int)lParam;
  if (id == -1 || id > NB_trame_buffer) return 0;

  //check if ip is ok
  HANDLE hlstv = GetDlgItem(h_sniff,DLG_NS_LSTV);
  unsigned int i,nb_ip = ListView_GetItemCount(hlstv);

  char buffer[DEFAULT_TMP_SIZE];
  BOOL ip_src_exist = FALSE;
  BOOL ip_dst_exist = FALSE;

  for (i=0;i<nb_ip;i++)
  {
    buffer[0] = 0;
    ListView_GetItemText(hlstv,i,0,buffer,DEFAULT_TMP_SIZE);
    if (!strcmp(buffer,Trame_buffer[id].ip_src))
    {
      //exist, check if protocols are complets
      ip_src_exist = TRUE;

      //chek if ttl/os are ok
      buffer[0] = 0;
      ListView_GetItemText(hlstv,i,2,buffer,DEFAULT_TMP_SIZE);
      if (buffer[0] == 0 && Trame_buffer[id].TTL != 0)
      {
        //ttl
        buffer[0] = 0;
        snprintf(buffer,DEFAULT_TMP_SIZE,"%03d",Trame_buffer[id].TTL);
        ListView_SetItemText(hlstv,i,2,buffer);

        //OS
        if (Trame_buffer[id].TTL > 128)ListView_SetItemText(hlstv,i,3,"Router")
        else if (Trame_buffer[id].TTL > 64)ListView_SetItemText(hlstv,i,3,"Windows")
        else ListView_SetItemText(hlstv,i,3,"Linux");
      }

      //check protocol
      buffer[0] = 0;
      ListView_GetItemText(hlstv,i,4,buffer,DEFAULT_TMP_SIZE);
      if (buffer[0] == 0)
      {
        //protocol
        switch(Trame_buffer[id].ProtoType)
        {
          case IPPROTO_TCP:
            if (!Contient(buffer,TXT_TCP_IPV4))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_TCP_IPV4,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
          case IPPROTO_UDP:
            if (!Contient(buffer,TXT_UDP_IPV4))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_UDP_IPV4,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
          case IPPROTO_ICMP:
            if (!Contient(buffer,TXT_ICMPV4))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_ICMPV4,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
          case IPPROTO_IGMP:
            if (!Contient(buffer,TXT_IGMP))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_IGMP,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
        }
      }
    }
    if (!strcmp(buffer,Trame_buffer[id].ip_dst))
    {
      //exist, check if protocols are complets
      ip_dst_exist = TRUE;

      //check protocol
      buffer[0] = 0;
      ListView_GetItemText(hlstv,i,4,buffer,DEFAULT_TMP_SIZE);
      if (buffer[0] == 0)
      {
        //protocol
        switch(Trame_buffer[id].ProtoType)
        {
          case IPPROTO_TCP:
            if (!Contient(buffer,TXT_TCP_IPV4))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_TCP_IPV4,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
          case IPPROTO_UDP:
            if (!Contient(buffer,TXT_UDP_IPV4))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_UDP_IPV4,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
          case IPPROTO_ICMP:
            if (!Contient(buffer,TXT_ICMPV4))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_ICMPV4,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
          case IPPROTO_IGMP:
            if (!Contient(buffer,TXT_IGMP))
            {
              strncat(buffer,",",DEFAULT_TMP_SIZE);
              strncat(buffer,TXT_IGMP,DEFAULT_TMP_SIZE);
              strncat(buffer,"\0",DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv,i,4,buffer);
            }
          break;
        }
      }
    }
  }

  LVITEM lvi;
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";
  lvi.iItem = nb_ip;

  if (!ip_src_exist) //add item
  {
    long int intemPosTrame = ListView_InsertItem(hlstv, &lvi);
    if (intemPosTrame == -1)return 0;

    //ip
    ListView_SetItemText(hlstv,intemPosTrame,0,Trame_buffer[id].ip_src);

    //ttl
    buffer[0] = 0;
    snprintf(buffer,DEFAULT_TMP_SIZE,"%03d",Trame_buffer[id].TTL);
    if (Trame_buffer[id].TTL != 0)
    {
      ListView_SetItemText(hlstv,intemPosTrame,2,buffer);

      //OS
      if (Trame_buffer[id].TTL > 128)ListView_SetItemText(hlstv,intemPosTrame,3,"Router")
      else if (Trame_buffer[id].TTL > 64)ListView_SetItemText(hlstv,intemPosTrame,3,"Windows")
      else ListView_SetItemText(hlstv,intemPosTrame,3,"Linux");
    }

    //protocol
    switch(Trame_buffer[id].ProtoType)
    {
      case IPPROTO_TCP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_TCP_IPV4);break;
      case IPPROTO_UDP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_UDP_IPV4);break;
      case IPPROTO_ICMP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_ICMPV4);break;
      case IPPROTO_IGMP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_IGMP);break;
    }
  }
  if (!ip_dst_exist) //add item
  {
    long int intemPosTrame = ListView_InsertItem(hlstv, &lvi);
    if (intemPosTrame == -1)return 0;

    //ip
    ListView_SetItemText(hlstv,intemPosTrame,0,Trame_buffer[id].ip_dst);

    //protocol
    switch(Trame_buffer[id].ProtoType)
    {
      case IPPROTO_TCP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_TCP_IPV4);break;
      case IPPROTO_UDP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_UDP_IPV4);break;
      case IPPROTO_ICMP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_ICMPV4);break;
      case IPPROTO_IGMP:ListView_SetItemText(hlstv,intemPosTrame,4,TXT_IGMP);break;
    }
  }
}
//------------------------------------------------------------------------------
BOOL TraitementTrame(unsigned char *buffer, unsigned int taille,unsigned long int id)
{
  HANDLE hlstv_paquets  = GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS);

  //write database
  IPV4_HDR *ipv4_hdr = (IPV4_HDR *)buffer;
  struct sockaddr_in sock_in;

  switch(ipv4_hdr->ip_protocol)
  {
    case IPPROTO_TCP:
    case IPPROTO_UDP:
    case IPPROTO_ICMP:
    case IPPROTO_IGMP:break;
    default: return 0;
  }

  //ipv6 : disable
  //if (ipv4_hdr->ip_header_len*4 > taille || ipv4_hdr->ip_header_len < IPV4_HDR_SIZE || ipv4_hdr->ip_version == 6)return;

  char tmp[DEFAULT_TMP_SIZE];
  LVITEM lvi;
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";
  lvi.iItem = id;

  //src ip
  memset(&sock_in, 0, sizeof(sock_in));
  sock_in.sin_addr.s_addr = ipv4_hdr->ip_srcaddr;
  snprintf(Trame_buffer[id].ip_src,IP_SIZE_MAX,"%s",inet_ntoa(sock_in.sin_addr));

  //dst ip
  memset(&sock_in, 0, sizeof(sock_in));
  sock_in.sin_addr.s_addr = ipv4_hdr->ip_destaddr;
  snprintf(Trame_buffer[id].ip_dst,IP_SIZE_MAX,"%s",inet_ntoa(sock_in.sin_addr));

  //add only if tcp/udp/icmp/igmp and not 0.0.0.0
  if (!strcmp("0.0.0.0",Trame_buffer[id].ip_dst) || !strcmp("0.0.0.0",Trame_buffer[id].ip_src))return 0;

  long int intemPosTrame = ListView_InsertItem(hlstv_paquets, &lvi);
  if (intemPosTrame == -1)return 0;
    //ID
  snprintf(tmp,6,"%05d",intemPosTrame);
  ListView_SetItemText(hlstv_paquets,intemPosTrame,6,tmp);
  ListView_SetItemText(hlstv_paquets,intemPosTrame,2,Trame_buffer[id].ip_dst);
  ListView_SetItemText(hlstv_paquets,intemPosTrame,0,Trame_buffer[id].ip_src);

  //ttl
  Trame_buffer[id].TTL = ipv4_hdr->ip_ttl;

  //proto
  Trame_buffer[id].ProtoType = ipv4_hdr->ip_protocol;

  //verify if ip existe or add it with thread !
  AddIp(id);

  switch(ipv4_hdr->ip_protocol)
  {
    case IPPROTO_TCP:
    {
      TCP_HDR *tcpheader = (TCP_HDR*)(unsigned char *)(buffer+(ipv4_hdr->ip_header_len*4));
      Trame_buffer[id].src_port = ntohs(tcpheader->source_port);
      Trame_buffer[id].dst_port = ntohs(tcpheader->dest_port);

      GetPortDesc(Trame_buffer[id].src_port,FALSE, tmp, DEFAULT_TMP_SIZE);
      ListView_SetItemText(hlstv_paquets,intemPosTrame,1,tmp);

      if (!FilterExist(tmp) && strlen(tmp) > 9)
        SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_LB_FILTRE, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)tmp);

      GetPortDesc(Trame_buffer[id].dst_port,FALSE, tmp, DEFAULT_TMP_SIZE);
      ListView_SetItemText(hlstv_paquets,intemPosTrame,3,tmp);

      if (!FilterExist(tmp) && strlen(tmp) > 9)
        SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_LB_FILTRE, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)tmp);

      ListView_SetItemText(hlstv_paquets,intemPosTrame,4,TXT_TCP_IPV4);

      //datas :
      if (taille > IPV4_HDR_SIZE+TCP_HDR_SIZE)
      {
        snprintf(tmp,DEFAULT_TMP_SIZE,"%s",(buffer+IPV4_HDR_SIZE+TCP_HDR_SIZE));
        if (ValideChDesc(tmp,strlen(tmp)))
          ListView_SetItemText(hlstv_paquets,intemPosTrame,5,tmp);
      }
    }
    break;
    case IPPROTO_UDP:
    {
      UDP_HDR *udpheader = (UDP_HDR*)(unsigned char *)(buffer+(ipv4_hdr->ip_header_len*4));
      Trame_buffer[id].src_port = ntohs(udpheader->source_port);
      Trame_buffer[id].dst_port = ntohs(udpheader->dest_port);

      GetPortDesc(Trame_buffer[id].src_port, TRUE, tmp, DEFAULT_TMP_SIZE);
      ListView_SetItemText(hlstv_paquets,intemPosTrame,1,tmp);

      if (!FilterExist(tmp) && strlen(tmp) > 9)
        SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_LB_FILTRE, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)tmp);

      GetPortDesc(Trame_buffer[id].dst_port, TRUE, tmp, DEFAULT_TMP_SIZE);
      ListView_SetItemText(hlstv_paquets,intemPosTrame,3,tmp);

      if (!FilterExist(tmp) && strlen(tmp) > 9)
        SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_LB_FILTRE, LB_INSERTSTRING, (WPARAM)-1, (LPARAM)tmp);

      ListView_SetItemText(hlstv_paquets,intemPosTrame,4,TXT_UDP_IPV4);

      //datas :
      //datas :
      if (taille > IPV4_HDR_SIZE+UDP_HDR_SIZE)
      {
        snprintf(tmp,DEFAULT_TMP_SIZE,"%s",(buffer+IPV4_HDR_SIZE+UDP_HDR_SIZE));
        if (ValideChDesc(tmp,strlen(tmp)))
          ListView_SetItemText(hlstv_paquets,intemPosTrame,5,tmp);
      }
    }
    break;
    case IPPROTO_ICMP:
      ListView_SetItemText(hlstv_paquets,intemPosTrame,4,TXT_ICMPV4);

      //datas :
      if (taille > IPV4_HDR_SIZE+ICMP_HDR_SIZE)
      {
        snprintf(tmp,DEFAULT_TMP_SIZE,"%s",(buffer+IPV4_HDR_SIZE+ICMP_HDR_SIZE));
        if (ValideChDesc(tmp,strlen(tmp)))
          ListView_SetItemText(hlstv_paquets,intemPosTrame,5,tmp);
      }
    break;
    case IPPROTO_IGMP:
      ListView_SetItemText(hlstv_paquets,intemPosTrame,4,TXT_IGMP);
    break;
  }

  //refresh
  ListView_RedrawItems(hlstv_paquets,intemPosTrame,intemPosTrame);
  return 1;
}
//------------------------------------------------------------------------------
DWORD WINAPI Sniff(LPVOID lParam)
{
  //init
  ListView_DeleteAllItems(GetDlgItem(h_sniff,DLG_NS_LSTV));
  ListView_DeleteAllItems(GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE));
  ListView_DeleteAllItems(GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS));
  NB_trame_buffer = 0;

  WSADATA WSAData;
  WSAStartup(0x02, &WSAData);
  TRAME_BUFFER *trame_buffer_tmp;

  SOCKET sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP);
  if (sock != INVALID_SOCKET)
  {
    //init sock for read packets
    struct sockaddr_in sock_addr;
    memset(&sock_addr, 0, sizeof(sock_addr));

    char buffer[TAILLE_MAX_BUFFER_TRAME]="";
    SendDlgItemMessage(h_sniff,DLG_CONF_INTERFACE, CB_GETLBTEXT,SendDlgItemMessage(h_sniff,DLG_CONF_INTERFACE, CB_GETCURSEL,0,0),buffer);

    sock_addr.sin_addr.s_addr = inet_addr(buffer);
    sock_addr.sin_family      = AF_INET;
    sock_addr.sin_port        = 0;

    unsigned int ret = 0;

    //promiscious
    if (SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_CHK_PROMISCUOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
    {
      int rcvtimeo = 5000;
      setsockopt( sock , SOL_SOCKET , SO_RCVTIMEO , (const char *)&rcvtimeo , sizeof(rcvtimeo));
    }

    //bind interface !
    if (bind(sock,(struct sockaddr *)&sock_addr,sizeof(sock_addr)) != SOCKET_ERROR)
    {
      //text button
      SetWindowText(GetDlgItem((HWND)h_sniff,DLG_NS_BT_START),cps[TXT_BT_STOP].c);

      //promiscious mode
      unsigned long taille=1;
      buffer[0] = 0;
      WSAIoctl(sock, SIO_RCVALL, &taille, sizeof(taille), 0, 0,(LPDWORD)&buffer,0, 0);

      while (taille && start_sniff)
      {
        //get datas
        memset(&buffer, 0, sizeof(buffer));
        taille = recvfrom(sock,buffer,TAILLE_MAX_BUFFER_TRAME,0,0,0);
        if (taille > 0)
        {
          WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
          if (NB_trame_buffer%100 == 0) //allocate trames 100 by 100
          {
            trame_buffer_tmp = (TRAME_BUFFER *) realloc(Trame_buffer,(NB_trame_buffer+100)*(sizeof(TRAME_BUFFER)+1));
          }

          if (trame_buffer_tmp != NULL)
          {
            Trame_buffer = trame_buffer_tmp;

            //init new trame
            Trame_buffer[NB_trame_buffer].ProtoType = 0;
            Trame_buffer[NB_trame_buffer].ip_src[0] = 0;
            Trame_buffer[NB_trame_buffer].src_port  = 0;
            Trame_buffer[NB_trame_buffer].ip_dst[0] = 0;
            Trame_buffer[NB_trame_buffer].dst_port  = 0;
            Trame_buffer[NB_trame_buffer].TTL       = 0;

            memset(Trame_buffer[NB_trame_buffer].buffer_header, 0, TAILLE_MAX_BUFFER_HEADER);
            if (taille<TAILLE_MAX_BUFFER_TRAME)
            {
              Trame_buffer[NB_trame_buffer].taille_buffer = taille;
              memcpy(Trame_buffer[NB_trame_buffer].buffer,buffer,taille);
            }else
            {
              Trame_buffer[NB_trame_buffer].taille_buffer = TAILLE_MAX_BUFFER_TRAME;
              memcpy(Trame_buffer[NB_trame_buffer].buffer,buffer,TAILLE_MAX_BUFFER_TRAME);
            }

            //working
            ret = TraitementTrame(buffer, taille,NB_trame_buffer);
            if (ret)NB_trame_buffer++;
          }else
          {
            ReleaseMutex(hMutex_TRAME_BUFFER);
            break;
          }
          ReleaseMutex(hMutex_TRAME_BUFFER);
        }
      }
    }
  }

  WSACleanup();
  SetWindowText(GetDlgItem((HWND)h_sniff,DLG_NS_BT_START),cps[TXT_BT_START].c);
  EnableWindow(GetDlgItem(h_sniff,DLG_NS_BT_START),TRUE);
  return 0;
}
//------------------------------------------------------------------------------
HANDLE GetCurrentLstv()
{
  if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS)))return GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS);
  else if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE)))return GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE);
  else return GetDlgItem(h_sniff,DLG_NS_LSTV);
}
//------------------------------------------------------------------------------
DWORD GetCurrentLstvNbColumn()
{
  if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS)))return DLG_SNIFF_STATE_PAQUETS_NB_COLUMN;
  else if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE)))return DLG_SNIFF_STATE_FILTER_NB_COLUMN;
  else return DLG_SNIFF_STATE_IP_NB_COLUMN;
}
//------------------------------------------------------------------------------
DWORD  WINAPI Filtre_Sniff(LPVOID lParam)
{
  //index
  int index = SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_LB_FILTRE, LB_GETCURSEL,(WPARAM) 0, (LPARAM)0);

  HANDLE hlstv    = GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS);
  HANDLE hlstv_f  = GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE);

  //clean
  ListView_DeleteAllItems(hlstv_f);

  if (index == 0) //no filter
  {
    //if ip is visible
    HANDLE hlstv_ip = GetDlgItem(h_sniff,DLG_NS_LSTV);
    if(IsWindowVisible(hlstv_ip))ShowWindow(hlstv_ip,SW_HIDE);

    ShowWindow(hlstv_f,SW_HIDE);
    ShowWindow(hlstv,SW_SHOW);
  }else
  {
    //get filter text
    char buffer[DEFAULT_TMP_SIZE]="";
    if (SendDlgItemMessage(h_sniff,DLG_NS_SNIFF_LB_FILTRE, LB_GETTEXT,(WPARAM) index, (LPARAM)buffer)!=LB_ERR)
    {
      unsigned int i, j, count = 0;
      char tmp[DEFAULT_TMP_SIZE];
      LVITEM lvi;
      lvi.mask = LVIF_TEXT|LVIF_PARAM;
      lvi.iSubItem = 0;
      lvi.lParam = LVM_SORTITEMS;
      lvi.pszText="";

      for (i=0;i<NB_trame_buffer-1;i++)
      {
        tmp[0] = 0;
        ListView_GetItemText(hlstv,i,1,tmp,DEFAULT_TMP_SIZE);
        if (tmp[0] != 0 && !strcmp(tmp,buffer))
        {
          //ajout de l'item
          lvi.iItem = count;
          ListView_InsertItem(hlstv_f, &lvi);

          //copie des colonnes de la listeview d'origine dans celle du filtre
          for(j=0;j<DLG_SNIFF_STATE_PAQUETS_NB_COLUMN;j++)
          {
            tmp[0] = 0;
            ListView_GetItemText(hlstv,i,j,tmp,DEFAULT_TMP_SIZE);
            ListView_SetItemText(hlstv_f,count,j,tmp);
          }
          count++;
        }else
        {
          tmp[0] = 0;
          ListView_GetItemText(hlstv,i,3,tmp,DEFAULT_TMP_SIZE);
          if (tmp[0] != 0 && !strcmp(tmp,buffer))
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hlstv_f, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<DLG_SNIFF_STATE_PAQUETS_NB_COLUMN;j++)
            {
              tmp[0] = 0;
              ListView_GetItemText(hlstv,i,j,tmp,DEFAULT_TMP_SIZE);
              ListView_SetItemText(hlstv_f,count,j,tmp);
            }
            count++;
          }
        }
      }

      //if ip is visible
      HANDLE hlstv_ip = GetDlgItem(h_sniff,DLG_NS_LSTV);
      if(IsWindowVisible(hlstv_ip))ShowWindow(hlstv_ip,SW_HIDE);

      ShowWindow(hlstv,SW_HIDE);
      ShowWindow(hlstv_f,SW_SHOW);
    }
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI Filtre_Sniff_custom(LPVOID lParam)
{
  HANDLE hlstv    = GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS);
  HANDLE hlstv_f  = GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE);
  unsigned int type_filter = (unsigned int)lParam;

  HANDLE hlstv_tmp = GetCurrentLstv(); //get current listeview
  int index = SendMessage(hlstv_tmp, LVM_GETNEXTITEM,-1,LVNI_FOCUSED);

  //clean
  ListView_DeleteAllItems(hlstv_f);
  if (index == -1)return;

  unsigned int col1, col2; //search column
  char search_string[DEFAULT_TMP_SIZE]="";
  switch(type_filter)
  {
    case FILTER_IP_SRC:
      ListView_GetItemText(hlstv_tmp,index,0,search_string,DEFAULT_TMP_SIZE);
      col1 = 0;
      col2 = 2;
    break;
    case FILTER_IP_DST:
      ListView_GetItemText(hlstv_tmp,index,2,search_string,DEFAULT_TMP_SIZE);
      col1 = 0;
      col2 = 2;
    break;
    case FILTER_PORT_SRC:
      ListView_GetItemText(hlstv_tmp,index,1,search_string,DEFAULT_TMP_SIZE);
      col1 = 1;
      col2 = 3;
    break;
    case FILTER_PORT_DST:
      ListView_GetItemText(hlstv_tmp,index,3,search_string,DEFAULT_TMP_SIZE);
      col1 = 1;
      col2 = 3;
    break;
  }

  //search
  unsigned int i, j, count = 0;
  char tmp[DEFAULT_TMP_SIZE];
  LVITEM lvi;
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";

  for (i=0;i<NB_trame_buffer-1;i++)
  {
    tmp[0] = 0;
    ListView_GetItemText(hlstv,i,col1,tmp,DEFAULT_TMP_SIZE);
    if (tmp[0] != 0 && !strcmp(tmp,search_string))
    {
      //ajout de l'item
      lvi.iItem = count;
      ListView_InsertItem(hlstv_f, &lvi);

      //copie des colonnes de la listeview d'origine dans celle du filtre
      for(j=0;j<DLG_SNIFF_STATE_PAQUETS_NB_COLUMN;j++)
      {
        tmp[0] = 0;
        ListView_GetItemText(hlstv,i,j,tmp,DEFAULT_TMP_SIZE);
        ListView_SetItemText(hlstv_f,count,j,tmp);
      }
      count++;
    }else
    {
      tmp[0] = 0;
      ListView_GetItemText(hlstv,i,col2,tmp,DEFAULT_TMP_SIZE);
      if (tmp[0] != 0 && !strcmp(tmp,search_string))
      {
        //ajout de l'item
        lvi.iItem = count;
        ListView_InsertItem(hlstv_f, &lvi);

        //copie des colonnes de la listeview d'origine dans celle du filtre
        for(j=0;j<DLG_SNIFF_STATE_PAQUETS_NB_COLUMN;j++)
        {
          tmp[0] = 0;
          ListView_GetItemText(hlstv,i,j,tmp,DEFAULT_TMP_SIZE);
          ListView_SetItemText(hlstv_f,count,j,tmp);
        }
        count++;
      }
    }
  }

  //if ip is visible
  HANDLE hlstv_ip = GetDlgItem(h_sniff,DLG_NS_LSTV);
  if(IsWindowVisible(hlstv_ip))ShowWindow(hlstv_ip,SW_HIDE);

  ShowWindow(hlstv,SW_HIDE);
  ShowWindow(hlstv_f,SW_SHOW);
}

//------------------------------------------------------------------------------
void chartohexstring(char *src, unsigned int src_size, char *result, unsigned int result_size_max)
{
  result[0] = 0;
  if (!src_size || !result_size_max)return;

  char line[DEFAULT_TMP_SIZE],string_line[DEFAULT_TMP_SIZE],tmp[5];
  unsigned int i, j, k, r;
  for (i=0;i<src_size;)
  {
    //get string
    line[0]        = 0;
    string_line[0] = 0;
    for (j=0;i<src_size && j<16;j++,i++)
    {
      if (src[i]>31 && src[i]<127)string_line[j]=src[i];
      else string_line[j]='.';
    }
    string_line[j]=0;

    //write in line
    if (j == 16)
    {
      j = i-j;
      snprintf(line,DEFAULT_TMP_SIZE,"%02X %02X %02X %02X %02X %02X %02X %02X  %02X %02X %02X %02X %02X %02X %02X %02X  |  %s\r\n"
      ,src[j]&255,src[j+1]&255,src[j+2]&255,src[j+3]&255,src[j+4]&255,src[j+5]&255,src[j+6]&255,src[j+7]&255
      ,src[j+8]&255,src[j+9]&255,src[j+10]&255,src[j+11]&255,src[j+12]&255,src[j+13]&255,src[j+14]&255,src[j+15]&255
      ,string_line);
    }else //last line
    {
      r = i-j;
      for (k=0;k<j;k++)
      {
        if (k != 7)snprintf(tmp,4,"%02X ",src[r+k]&255);
        else snprintf(tmp,5,"%02X  ",src[r+k]&255);
        strncat(line,tmp,DEFAULT_TMP_SIZE);
      }
      for(;j<16;j++)
      {
        if (j != 7)strncat(line,"   \0",DEFAULT_TMP_SIZE);
        else strncat(line,"    \0",DEFAULT_TMP_SIZE);
      }
      strncat(line," |  \0",DEFAULT_TMP_SIZE);
      strncat(line,string_line,DEFAULT_TMP_SIZE);
      strncat(line,"\r\n\0",DEFAULT_TMP_SIZE);
    }
    strncat(result,line,result_size_max);
  }
  strncat(result,"\0",result_size_max);
}
//------------------------------------------------------------------------------
DWORD WINAPI LoadTrame_sniff(LPVOID lParam)
{
  HANDLE hlstv_tmp = GetCurrentLstv(); //get current listeview
  int index = SendMessage(hlstv_tmp, LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
  if (index == -1 || index > NB_trame_buffer) return 0;

  read_trame_sniff = TRUE;
  SetWindowText(hdbclk_sniff, "");
  if (start_sniff)WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  char buffer_ipv4[MAX_LINE_SIZE]="", buffer_hdr[MAX_LINE_SIZE]="", buffer_datas[MAX_LINE_SIZE]="", buffer_datas_hexa[MAX_LINE_SIZE]="", buffer_total[MAX_LINE_DBSIZE]="";

  //ipv4
  IPV4_HDR *ipv4_hdr = (IPV4_HDR *)Trame_buffer[index].buffer;
  snprintf(buffer_ipv4,REQUEST_MAX_SIZE,
               "[%s:%d->%s:%d]%05d\r\n"
               "[IPV4 HEADER]\r\n"
               "ip_header_len:\t%d\r\n"
               "ip_version:\t%d\r\n"
               "ip_tos:\t%d\r\n"
               "ip_total_length:\t%d\r\n"
               "ip_id:\t%d\r\n"
               "ip_reserved_zero:\t%d\r\n"
               "ip_dont_fragment:\t%d\r\n"
               "ip_more_fragment:\t%d\r\n"
               "ip_frag_offset:\t%d\r\n"
               "ip_ttl:\t%d\r\n"
               "ip_protocol:\t%d\r\n"
               "ip_srcaddr:\t%s\r\n"
               "ip_destaddr:\t%s\r\n\r\n",
                Trame_buffer[index].ip_src,Trame_buffer[index].src_port,
                Trame_buffer[index].ip_dst,Trame_buffer[index].dst_port,
                index,
                ipv4_hdr->ip_header_len,
                ipv4_hdr->ip_version,
                ipv4_hdr->ip_tos,
                ipv4_hdr->ip_total_length,
                ipv4_hdr->ip_id,
                ipv4_hdr->ip_reserved_zero,
                ipv4_hdr->ip_dont_fragment,
                ipv4_hdr->ip_more_fragment,
                ipv4_hdr->ip_frag_offset,
                ipv4_hdr->ip_ttl,
                ipv4_hdr->ip_protocol,
                Trame_buffer[index].ip_src,
                Trame_buffer[index].ip_dst);

  char *b = Trame_buffer[index].buffer;
  switch(Trame_buffer[index].ProtoType)
  {
    case IPPROTO_TCP:
    {
      TCP_HDR *tcpheader = (TCP_HDR*)(unsigned char *)(b+(ipv4_hdr->ip_header_len*4));
      snprintf(buffer_hdr,REQUEST_MAX_SIZE,
               "[TCP]\r\n"
               "source_port:\t%d\r\n"
               "dest_port:\t%d\r\n"
               "sequence:\t%ul\r\n"
               "acknowledge:\t%d\r\n"
               "ns:\t%d\r\n"
               "reserved:\t%d\r\n"
               "data_offset:\t%d\r\n"
               "flags:\t",
               tcpheader->source_port,
               tcpheader->dest_port,
               tcpheader->sequence,
               tcpheader->acknowledge,
               tcpheader->ns,
               tcpheader->reserved_part1,
               tcpheader->data_offset);
      if (tcpheader->fin)strncat(buffer_hdr,"FIN,\0",REQUEST_MAX_SIZE);
      if (tcpheader->syn)strncat(buffer_hdr,"SYN,\0",REQUEST_MAX_SIZE);
      if (tcpheader->rst)strncat(buffer_hdr,"RST,\0",REQUEST_MAX_SIZE);
      if (tcpheader->psh)strncat(buffer_hdr,"PSH,\0",REQUEST_MAX_SIZE);
      if (tcpheader->ack)strncat(buffer_hdr,"ACK,\0",REQUEST_MAX_SIZE);
      if (tcpheader->urg)strncat(buffer_hdr,"URG,\0",REQUEST_MAX_SIZE);
      if (tcpheader->ecn)strncat(buffer_hdr,"ECN,\0",REQUEST_MAX_SIZE);
      if (tcpheader->cwr)strncat(buffer_hdr,"CWR\0",REQUEST_MAX_SIZE);

      char tmp[10]="";
      strncat(buffer_hdr,"\r\nwindow:\t\0",REQUEST_MAX_SIZE);strncat(buffer_hdr,itoa(tcpheader->window,tmp,10),REQUEST_MAX_SIZE);
      tmp[0]=0;
      strncat(buffer_hdr,"\r\nchecksum:\t\0",REQUEST_MAX_SIZE);strncat(buffer_hdr,itoa(tcpheader->checksum,tmp,10),REQUEST_MAX_SIZE);
      tmp[0]=0;
      strncat(buffer_hdr,"\r\nurgent_pointer:\t\0",REQUEST_MAX_SIZE);strncat(buffer_hdr,itoa(tcpheader->urgent_pointer,tmp,10),REQUEST_MAX_SIZE);
      strncat(buffer_hdr,"\r\n\r\n\0",REQUEST_MAX_SIZE);

      //datas in hexa
      snprintf(buffer_datas,MAX_LINE_SIZE,"[DATAS-STRING]\r\n%s\r\n\r\n[DATAS-HEXA]\r\n",b+IPV4_HDR_SIZE+TCP_HDR_SIZE);
      if (!ValideChDesc(buffer_datas,strlen(buffer_datas)))strcpy(buffer_datas,"[DATAS-HEXA]\r\n\0");
      chartohexstring(b+IPV4_HDR_SIZE+TCP_HDR_SIZE, (Trame_buffer[index].taille_buffer)-IPV4_HDR_SIZE-TCP_HDR_SIZE,buffer_datas_hexa,MAX_LINE_SIZE);
    }
    break;
    case IPPROTO_UDP:
    {
      UDP_HDR *udpheader = (UDP_HDR*)(unsigned char *)(b+(ipv4_hdr->ip_header_len*4));
      snprintf(buffer_hdr,REQUEST_MAX_SIZE,
               "[UDP]\r\n"
               "source_port:\t%d\r\n"
               "dest_port:\t%d\r\n"
               "udp_length:\t%d\r\n"
               "udp_checksum:\t%d\r\n\r\n",
               udpheader->source_port,
               udpheader->dest_port,
               udpheader->udp_length,
               udpheader->udp_checksum);

      //datas in hexa
      snprintf(buffer_datas,MAX_LINE_SIZE,"[DATAS-STRING]\r\n%s\r\n\r\n[DATAS-HEXA]\r\n",b+IPV4_HDR_SIZE+UDP_HDR_SIZE);
      if (!ValideChDesc(buffer_datas,strlen(buffer_datas)))strcpy(buffer_datas,"[DATAS-HEXA]\r\n\0");
      chartohexstring(b+IPV4_HDR_SIZE+UDP_HDR_SIZE, (Trame_buffer[index].taille_buffer)-IPV4_HDR_SIZE-UDP_HDR_SIZE,buffer_datas_hexa,MAX_LINE_SIZE);
    }
    break;
    case IPPROTO_ICMP:
    {
      ICMP_HDR *icmp_hdr = (ICMP_HDR*)(unsigned char *)(b+(ipv4_hdr->ip_header_len*4));
      snprintf(buffer_hdr,REQUEST_MAX_SIZE,
               "[ICMP]\r\n"
               "type:\t%d\r\n"
               "code:\t%d\r\n"
               "checksum:\t%d\r\n"
               "id:\t%d\r\n"
               "seq:\t%d\r\n\r\n[DATAS-HEXA]\r\n",
               icmp_hdr->type,
               icmp_hdr->code,
               icmp_hdr->checksum,
               icmp_hdr->id,
               icmp_hdr->seq);

      //datas in hexa
      chartohexstring(b+IPV4_HDR_SIZE+ICMP_HDR_SIZE, (Trame_buffer[index].taille_buffer)-IPV4_HDR_SIZE-ICMP_HDR_SIZE,buffer_datas_hexa,MAX_LINE_SIZE);
    }
    break;
    case IPPROTO_IGMP:
    {
      IGMP_HDR *igmp_hdr = (IGMP_HDR*)(unsigned char *)(b+(ipv4_hdr->ip_header_len*4));
      snprintf(buffer_hdr,REQUEST_MAX_SIZE,
               "[IGMP]\r\n"
               "type:\t%d\r\n"
               "ttl:\t%d\r\n"
               "checksum:\t%d\r\n"
               "ip_groupe:\t%d\r\n\r\n[DATAS-HEXA]\r\n",
               igmp_hdr->type,
               igmp_hdr->Temp_reponse,
               igmp_hdr->checksum,
               igmp_hdr->ip_groupe);

      //datas in hexa
      chartohexstring(b+IPV4_HDR_SIZE+IGMP_HDR_SIZE, (Trame_buffer[index].taille_buffer)-IPV4_HDR_SIZE-IGMP_HDR_SIZE,buffer_datas_hexa,MAX_LINE_SIZE);
    }
    break;
  }

  //set text
  snprintf(buffer_total,MAX_LINE_DBSIZE,"%s%s%s%s",buffer_ipv4,buffer_hdr,buffer_datas,buffer_datas_hexa);
  SetWindowText(hdbclk_sniff, buffer_total);
  ShowWindow (hdbclk_sniff, SW_SHOW);
  read_trame_sniff = FALSE;
}
//------------------------------------------------------------------------------
DWORD WINAPI follow_stream(LPVOID lParam)
{
  HANDLE hlstv_tmp = GetCurrentLstv(); //get current listeview
  int index = SendMessage(hlstv_tmp, LVM_GETNEXTITEM,-1,LVNI_FOCUSED);
  if (index == -1 || index > NB_trame_buffer) return 0;
  //only for tcp and udp
  if (Trame_buffer[index].ProtoType != IPPROTO_TCP && Trame_buffer[index].ProtoType != IPPROTO_UDP)return 0;

  follow_sniff = TRUE;
  SetWindowText(hdbclk_sniff, "");
  if (start_sniff)WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  char *buffer = (char *)malloc(MAX_LINE_SIZE), *b;
  buffer[0] = 0;

  DWORD i;
  char frame_buffer[MAX_LINE_SIZE], data_buffer[MAX_LINE_SIZE];
  for (i=0;i<NB_trame_buffer;i++)
  {
    if (Trame_buffer[index].ProtoType == Trame_buffer[i].ProtoType &&  //protocle
        (!strcmp(Trame_buffer[index].ip_src,Trame_buffer[i].ip_src) || !strcmp(Trame_buffer[index].ip_src,Trame_buffer[i].ip_dst)) && //ip
        (!strcmp(Trame_buffer[index].ip_dst,Trame_buffer[i].ip_src) || !strcmp(Trame_buffer[index].ip_dst,Trame_buffer[i].ip_dst)) &&
        (Trame_buffer[index].src_port == Trame_buffer[i].src_port || Trame_buffer[index].src_port == Trame_buffer[i].dst_port) &&     //port
        (Trame_buffer[index].dst_port == Trame_buffer[i].src_port || Trame_buffer[index].dst_port == Trame_buffer[i].dst_port))
    {
      frame_buffer[0] = 0;
      if (Trame_buffer[index].ProtoType == IPPROTO_TCP)
      {
        data_buffer[0]=0;
        snprintf(data_buffer,MAX_LINE_SIZE,"%s",(Trame_buffer[i].buffer+IPV4_HDR_SIZE+TCP_HDR_SIZE));
        if (!ValideChDesc(data_buffer,strlen(data_buffer)))data_buffer[0]=0;
        snprintf(frame_buffer,MAX_LINE_SIZE,"[%s:%d->%s:%d]%05d\r\n%s\r\n\r\n",
                 Trame_buffer[i].ip_src,Trame_buffer[i].src_port,
                 Trame_buffer[i].ip_dst,Trame_buffer[i].dst_port,i,data_buffer);
      }else if (Trame_buffer[index].ProtoType == IPPROTO_UDP)
      {
        data_buffer[0]=0;
        snprintf(data_buffer,MAX_LINE_SIZE,"%s",(Trame_buffer[i].buffer+IPV4_HDR_SIZE+UDP_HDR_SIZE));
        if (!ValideChDesc(data_buffer,strlen(data_buffer)))data_buffer[0]=0;
        snprintf(frame_buffer,MAX_LINE_SIZE,"[%s:%d->%s:%d]%05d\r\n%s\r\n\r\n",
                 Trame_buffer[i].ip_src,Trame_buffer[i].src_port,
                 Trame_buffer[i].ip_dst,Trame_buffer[i].dst_port,i,data_buffer);
      }

      //add
      b = realloc(buffer,(i+1)*MAX_LINE_SIZE);
      if (b == NULL) break;
      buffer = b;

      strcat(buffer,frame_buffer);
      strcat(buffer,"\0");
    }
  }

  if (start_sniff)ReleaseMutex(hMutex_TRAME_BUFFER);

  //set text
  SetWindowText(hdbclk_sniff, buffer);
  free(buffer);
  ShowWindow (hdbclk_sniff, SW_SHOW);
  follow_sniff = FALSE;
}

//------------------------------------------------------------------------------
//subclass of hdbclk_sniff
//------------------------------------------------------------------------------
LRESULT APIENTRY subclass_hdbclk_sniff(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_CLOSE)ShowWindow (hdbclk_sniff, SW_HIDE);
  else return CallWindowProc(wndproc_hdbclk_sniff, hwnd, uMsg, wParam, lParam);
  return 0;
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_sniff(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_ERASEBKGND:break;
    case WM_DRAWITEM:
    {
      //only if listview
      LPDRAWITEMSTRUCT lpDrawItem = (LPDRAWITEMSTRUCT)lParam;
      if (lpDrawItem->CtlType != ODT_LISTVIEW)break;

      //if selected
      HBRUSH color;
      char tmp[MAX_PATH];
      if(ListView_GetItemState(lpDrawItem->hwndItem, lpDrawItem->itemID, LVIS_SELECTED) & LVIS_SELECTED)
      {
        SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        color = GetSysColorBrush(COLOR_HIGHLIGHT);
      }else
      {
        FillRect(lpDrawItem->hDC, &lpDrawItem->rcItem, GetSysColorBrush(COLOR_WINDOW));
        SetTextColor(lpDrawItem->hDC, GetSysColor(COLOR_MENUTEXT));

        //line color choice
        tmp[0]     = 0;
        ListView_GetItemText(lpDrawItem->hwndItem,lpDrawItem->itemID,4/*protocol column*/,tmp,MAX_PATH);
        if (!strcmp(tmp,TXT_TCP_IPV4)       || !strcmp(tmp,TXT_TCP_IPV6))color = Hb_green;
        else if (!strcmp(tmp,TXT_UDP_IPV4)  || !strcmp(tmp,TXT_UDP_IPV6))color = Hb_blue;
        else if (!strcmp(tmp,TXT_ICMPV4)    || !strcmp(tmp,TXT_ICMPV6))color = Hb_pink;
        else if (!strcmp(tmp,TXT_IGMP))color = Hb_violet;
        else color = (HBRUSH)GetStockObject(WHITE_BRUSH);
      }

      //apply
      int x, y, i, nb_column = GetCurrentLstvNbColumn();
      RECT rect;
      ListView_GetItemRect(lpDrawItem->hwndItem, lpDrawItem->itemID, &rect, LVIR_BOUNDS);
      x           = rect.left;
      rect.bottom = lpDrawItem->rcItem.bottom;

      //column one by one
      for (i=0;i<nb_column;i++)
      {
        y = ListView_GetColumnWidth(lpDrawItem->hwndItem, i);
        rect.left  = x;
        rect.right = x + y;

        //item text
        tmp[0]     = 0;
        ListView_GetItemText(lpDrawItem->hwndItem,lpDrawItem->itemID,i,tmp,MAX_PATH);
        FillRect(lpDrawItem->hDC, &rect, (HBRUSH)color);
        rect.left  = x + 4;
        rect.right = x + y - 4;
        DrawText(lpDrawItem->hDC,tmp, strlen(tmp), &rect, DT_VCENTER);

        //next
        x += y;
      }
    }
    break;
    case WM_SIZE:
    {
      unsigned int mWidth  = LOWORD(lParam);
      unsigned int mHeight = HIWORD(lParam);

      unsigned int lb_size = 200;
      unsigned int lv_size = mWidth-(lb_size+5);

      MoveWindow(GetDlgItem(hwnd,DLG_NS_SNIFF_LB_FILTRE),5,0,lb_size,mHeight-105,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_NS_LSTV),lb_size+5,0,lv_size,mHeight-5,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_NS_LSTV_FILTRE),lb_size+5,0,lv_size,mHeight-5,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_NS_LSTV_PAQUETS),lb_size+5,0,lv_size,mHeight-5,TRUE);

      MoveWindow(GetDlgItem(hwnd,DLG_CONF_INTERFACE),5,mHeight-100,lb_size-5,100,TRUE);

      MoveWindow(GetDlgItem(hwnd,DLG_NS_SNIFF_CHK_PROMISCUOUS),5,mHeight-75,lb_size-5,18,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_NS_SNIFF_CHK_DISCO),5,mHeight-55,lb_size-5,18,TRUE);

      MoveWindow(GetDlgItem(hwnd,DLG_NS_BT_SAVE),5,mHeight-35,(lb_size-10)/2,25,TRUE);
      MoveWindow(GetDlgItem(hwnd,DLG_NS_BT_START),((lb_size-10)/2)+5,mHeight-35,(lb_size-10)/2,25,TRUE);

      if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS)))
      {
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV),SW_HIDE);
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV_FILTRE),SW_HIDE);
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV_PAQUETS),SW_SHOW);
      }else if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE)))
      {
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV),SW_HIDE);
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV_PAQUETS),SW_HIDE);
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV_FILTRE),SW_SHOW);
      }else
      {
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV_FILTRE),SW_HIDE);
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV_PAQUETS),SW_HIDE);
        ShowWindow(GetDlgItem(hwnd,DLG_NS_LSTV),SW_SHOW);
      }

      //resize columns
      unsigned int i;
      DWORD column_sz = (lv_size-40)/DLG_SNIFF_STATE_PAQUETS_NB_COLUMN;
      for (i=0;i<DLG_SNIFF_STATE_PAQUETS_NB_COLUMN;i++)
      {
        redimColumnH(GetDlgItem(hwnd,DLG_NS_LSTV_PAQUETS),i,column_sz);
      }

      column_sz = (lv_size-40)/DLG_SNIFF_STATE_FILTER_NB_COLUMN;
      for (i=0;i<DLG_SNIFF_STATE_FILTER_NB_COLUMN;i++)
      {
        redimColumnH(GetDlgItem(hwnd,DLG_NS_LSTV_FILTRE),i,column_sz);
      }

      column_sz = (lv_size-40)/DLG_SNIFF_STATE_IP_NB_COLUMN;
      for (i=0;i<DLG_SNIFF_STATE_IP_NB_COLUMN;i++)
      {
        redimColumnH(GetDlgItem(hwnd,DLG_NS_LSTV),i,column_sz);
      }
    }
    break;
    case WM_NOTIFY:
      switch(((LPNMHDR)lParam)->code)
      {
        // tri of columns
        case LVN_COLUMNCLICK:
          if (!start_sniff)
          {
            TRI_SNIFF_VIEW = !TRI_SNIFF_VIEW;
            c_Tri(GetCurrentLstv(),((LPNMLISTVIEW)lParam)->iSubItem,TRI_SNIFF_VIEW);
          }
        break;
        //get item info
        case NM_DBLCLK:if ((LOWORD(wParam) == DLG_NS_LSTV_PAQUETS || LOWORD(wParam) == DLG_NS_LSTV_FILTRE) /*&& !read_trame_sniff*/)CreateThread(NULL,0,LoadTrame_sniff,NULL,0,0); break;
      }
    break;
    case WM_CONTEXTMENU:
    {
      HWND hlstv = GetCurrentLstv();
      if (ListView_GetItemCount(hlstv) > 0 && (HWND)wParam == hlstv)
      {
        HMENU hmenu;
        if ((hmenu = LoadMenu(hinst, MAKEINTRESOURCE(POPUP_LSTV_SNIFF)))!= NULL)
        {
          //set text !!!
          ModifyMenu(hmenu,POPUP_SNIFF_FILTRE_IP_SRC   ,MF_BYCOMMAND|MF_STRING ,POPUP_SNIFF_FILTRE_IP_SRC   ,cps[TXT_POPUP_SNIFF_FILTRE_IP_SRC].c);
          ModifyMenu(hmenu,POPUP_SNIFF_FILTRE_IP_DST   ,MF_BYCOMMAND|MF_STRING ,POPUP_SNIFF_FILTRE_IP_DST   ,cps[TXT_POPUP_SNIFF_FILTRE_IP_DST].c);
          ModifyMenu(hmenu,POPUP_SNIFF_FILTRE_PORT_SRC ,MF_BYCOMMAND|MF_STRING ,POPUP_SNIFF_FILTRE_PORT_SRC ,cps[TXT_POPUP_SNIFF_FILTRE_PORT_SRC].c);
          ModifyMenu(hmenu,POPUP_SNIFF_FILTRE_PORT_DST ,MF_BYCOMMAND|MF_STRING ,POPUP_SNIFF_FILTRE_PORT_DST ,cps[TXT_POPUP_SNIFF_FILTRE_PORT_DST].c);
          ModifyMenu(hmenu,POPUP_LINK                  ,MF_BYCOMMAND|MF_STRING ,POPUP_LINK                  ,cps[TXT_POPUP_LINK].c);

          ModifyMenu(hmenu,POPUP_S_VIEW                ,MF_BYCOMMAND|MF_STRING ,POPUP_S_VIEW                ,cps[TXT_POPUP_S_VIEW].c);
          ModifyMenu(hmenu,POPUP_S_SELECTION           ,MF_BYCOMMAND|MF_STRING ,POPUP_S_SELECTION           ,cps[TXT_POPUP_S_SELECTION].c);

          if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV)))
          {
            RemoveMenu(hmenu,5,MF_BYPOSITION);
            RemoveMenu(hmenu,2,MF_BYPOSITION);
            RemoveMenu(hmenu,POPUP_SNIFF_FILTRE_IP_DST,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_SNIFF_FILTRE_PORT_SRC,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_SNIFF_FILTRE_PORT_DST,MF_BYCOMMAND);
            RemoveMenu(hmenu,POPUP_LINK,MF_BYCOMMAND);
          }

          //affichage du popup menu
          TrackPopupMenuEx(GetSubMenu(hmenu, 0), 0, LOWORD(lParam), HIWORD(lParam), hwnd, NULL);
          DestroyMenu(hmenu);
        }
      }
    }
    break;
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case LBN_SELCHANGE:
          switch(LOWORD(wParam))
          {
            //filter view
            case DLG_NS_SNIFF_LB_FILTRE:CreateThread(NULL,0,Filtre_Sniff,0,0,0);break;
          }
        break;
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            case DLG_NS_SNIFF_CHK_DISCO:
              if(IsWindowVisible(GetDlgItem(h_sniff,DLG_NS_LSTV)))
              {
                ShowWindow(GetDlgItem(h_sniff,DLG_NS_LSTV),SW_HIDE);
                ShowWindow(GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE),SW_HIDE);
                ShowWindow(GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS),SW_SHOW);
              }else
              {
                ShowWindow(GetDlgItem(h_sniff,DLG_NS_LSTV_FILTRE),SW_HIDE);
                ShowWindow(GetDlgItem(h_sniff,DLG_NS_LSTV_PAQUETS),SW_HIDE);
                ShowWindow(GetDlgItem(h_sniff,DLG_NS_LSTV),SW_SHOW);
              }
            break;
            case DLG_NS_BT_START:
              if (start_sniff)
              {
                start_sniff = FALSE;
                EnableWindow(GetDlgItem(h_sniff,DLG_NS_BT_START),FALSE);
              }else
              {
                start_sniff = TRUE;
                Hsniff = CreateThread(NULL,0,Sniff,(PVOID)0,0,0);
              }
            break;
            //-----------------------------------------------------
            case POPUP_SNIFF_FILTRE_IP_SRC:CreateThread(NULL,0,Filtre_Sniff_custom,(PVOID)FILTER_IP_SRC,0,0);break;
            case POPUP_SNIFF_FILTRE_IP_DST:CreateThread(NULL,0,Filtre_Sniff_custom,(PVOID)FILTER_IP_DST,0,0);break;
            case POPUP_SNIFF_FILTRE_PORT_SRC:CreateThread(NULL,0,Filtre_Sniff_custom,(PVOID)FILTER_PORT_SRC,0,0);break;
            case POPUP_SNIFF_FILTRE_PORT_DST:CreateThread(NULL,0,Filtre_Sniff_custom,(PVOID)FILTER_PORT_DST,0,0);break;
            //-----------------------------------------------------
            case POPUP_LINK:if(!follow_sniff)CreateThread(NULL,0,follow_stream,NULL,0,0);break;
            //-----------------------------------------------------
            //view
            case DLG_NS_BT_SAVE:
            case POPUP_S_VIEW:
            {
              char file[MAX_PATH]="RAW_socket_save";
              OPENFILENAME ofn;
              ZeroMemory(&ofn, sizeof(OPENFILENAME));
              ofn.lStructSize  = sizeof(OPENFILENAME);
              ofn.hwndOwner    = h_sniff;
              ofn.lpstrFile    = file;
              ofn.nMaxFile     = MAX_PATH;
              ofn.lpstrFilter  ="*.csv \0*.csv\0*.xml \0*.xml\0*.html \0*.html\0";//*.pcap \0*.pcap\0";
              ofn.nFilterIndex = 1;
              ofn.Flags        = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
              ofn.lpstrDefExt  =".csv\0";
              if (GetSaveFileName(&ofn)==TRUE)
              {
                if (ofn.nFilterIndex == SAVE_TYPE_PCAP)
                {
                  //a coder
                }else SaveLSTV(GetCurrentLstv(), file, ofn.nFilterIndex, GetCurrentLstvNbColumn());
              }
            }
            break;
            //selected
            case POPUP_S_SELECTION:
            {
              char file[MAX_PATH]="RAW_socket_save";
              OPENFILENAME ofn;
              ZeroMemory(&ofn, sizeof(OPENFILENAME));
              ofn.lStructSize  = sizeof(OPENFILENAME);
              ofn.hwndOwner    = h_sniff;
              ofn.lpstrFile    = file;
              ofn.nMaxFile     = MAX_PATH;
              ofn.lpstrFilter  ="*.csv \0*.csv\0*.xml \0*.xml\0*.html \0*.html\0";//*.pcap \0*.pcap*/\0";
              ofn.nFilterIndex = 1;
              ofn.Flags        = OFN_PATHMUSTEXIST | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT;
              ofn.lpstrDefExt  =".csv\0";
              if (GetSaveFileName(&ofn)==TRUE)
              {
                if (ofn.nFilterIndex == SAVE_TYPE_PCAP)
                {
                  //a coder
                }else SaveLSTVSelectedItems(GetCurrentLstv(), file, ofn.nFilterIndex, GetCurrentLstvNbColumn());
              }
            }
            break;
          }
        break;
      }
    break;
    case WM_CLOSE : ShowWindow(hwnd, SW_HIDE);break;
  }
  return FALSE;
}
