//------------------------------------------------------------------------------
// Projet SCP-DB        : Scan et tests de réseau
// Auteur               : Hanteville Nicolas
// Site                 : http:\\omni.a.free.fr
// Version              : 0.1
// Date de modification : 06/01/2010
// Description          : capture de trames
// Environnement        : compatiblité DEVCPP / VISUAL C++ / BORLAND C++ 5.x
//------------------------------------------------------------------------------
#include "_ressources.h"

const    //liste des protocoles connues (137 / 255)
   char ProtocolesTypesNames[138][20]={"0:IPv6 HOPOPT","1:ICMP","2:IGAP/IGMP/RGMP","3:GGP","4:IP in IP","5:ST","6:TCP","7:UCL/CBT","8:EGP","9:IGRP","10:BBN RCC",
   "11:NVP","12:PUP","13:ICMP","14:EMCON","15:XNET","16:Chaos","17:UDP","18:TMux","19:DCN","20:HMP","21:PRM","22:XEROX NS IDP","23:Trunk-1","24:Trunk-2","25:Leaf-1","26:Leaf-2",
   "27:RDP","28:IRTP","29: ISO TPC4","30:	NETBLT","31:MFE NSP","32:MERIT IP","33:DCCP","34:TPCP","35:IDPR IDPRP","36:XTP","37:DDP","38:IDPR CMTP","39:TP++ ","40:IL TP",
   "41:IPv6 over IPv4","42:SDRP","43:IPv6 RH","44:IPv6 FH","45:IDRP","46:RSVP","47:GRE","48:DSR","49:BNA","50:ESP","51:AH","52:I-NLSP","53:SWIPE","54:NARP","55:MEP","56:TLSP",
   "57:SKIP","58:ICMPv6/MLD","59:IPv6 NNH","60:IPv6 DO","61:AHIP","62:CFTP","63:ALN","64:SATNET/EXPAK","65:Kryptolan","66:MIT RVDP","67:IPPC","68:ADFS","69:SATNET",
   "70:VISA","71:IPCU","72:CPNE","73:CPHB","74:WSN","75:PVP","76:B SATNET M","77:SUN ND PT","78:WIDEBAND M","79:WIDEBAND EXPAK","80:ISO-IP","81:VMTP","82:SECURE-VMTP",
   "83:VINES","84:TTP","85:NSFNET-IGP","86:DGP","87:TCF","88:EIGRP","89:OSPF/MOSPF","90:SRPCP","91:LARP","92:MTP","93:AX.25","94:IP-IP EP","95:MICP","96:SCSP","97:EtherIP",
   "98:EH","99:APES","100:GMTP","101:IFMP","102:PNNI-IP","103:IPM","104:ARIS","105:SCPS","106:QNX","107:AN","108:IPPCP","109:SNP","110:CPP","111:IPX-IP","112:VRRP","113:PGM",
   "114:ANY 0-HOP P","115:L2TP","116:DDX DIIDE","117:IATP","118:ST","119:SRP","120:UTI","121:SMP","122:SM","123:PTP","124:ISIS-IPv4","125:FIRE","126:CRTP","127:CRUDP","128:SSCOPMCE",
   "129:IPLT","130:SPS","131:PIPE-IP","132:SCTP","133:FC","134:RSVP-E2E-IGNORE","135:Mobility Header","136:UDP-Lite","137:MPLS-IP"};

const char Hexa[256][3]={"00","01","02","03","04","05","06","07","08","09","0A","0B","0C","0D","0E","0F",
                         "10","11","12","13","14","15","16","17","18","19","1A","1B","1C","1D","1E","1F",
                         "20","21","22","23","24","25","26","27","28","29","2A","2B","2C","2D","2E","2F",
                         "30","31","32","33","34","35","36","37","38","39","3A","3B","3C","3D","3E","3F",
                         "40","41","42","43","44","45","46","47","48","49","4A","4B","4C","4D","4E","4F",
                         "50","51","52","53","54","55","56","57","58","59","5A","5B","5C","5D","5E","5F",
                         "60","61","62","63","64","65","66","67","68","69","6A","6B","6C","6D","6E","6F",
                         "70","71","72","73","74","75","76","77","78","79","7A","7B","7C","7D","7E","7F",
                         "80","81","82","83","84","85","86","87","88","89","8A","8B","8C","8D","8E","8F",
                         "90","91","92","93","94","95","96","97","98","99","9A","9B","9C","9D","9E","9F",
                         "A0","A1","A2","A3","A4","A5","A6","A7","A8","A9","AA","AB","AC","AD","AE","AF",
                         "B0","B1","B2","B3","B4","B5","B6","B7","B8","B9","BA","BB","BC","BD","BE","BF",
                         "C0","C1","C2","C3","C4","C5","C6","C7","C8","C9","CA","CB","CC","CD","CE","CF",
                         "D0","D1","D2","D3","D4","D5","D6","D7","D8","D9","DA","DB","DC","DD","DE","DF",
                         "E0","E1","E2","E3","E4","E5","E6","E7","E8","E9","EA","EB","EC","ED","EE","EF",
                         "F0","F1","F2","F3","F4","F5","F6","F7","F8","F9","FA","FB","FC","FD","FE","FF"};
//------------------------------------------------------------------------------
void ConvertionHexa(unsigned char* src,unsigned char *dest, unsigned long taillersc)
{
  unsigned long i=0;
  dest[0]=0;

  while (i<taillersc)strcat(dest,Hexa[src[i++]]);

  strcat(dest,"\0");
}
//------------------------------------------------------------------------------
BOOL TestPorts(unsigned int port,unsigned long int ID)
{
    BOOL ret =0;
    WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
    switch (port)
    {
      case 21:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix      =CODE_FTP;
           ret = 1;
           if (!(Button_Filtre_CODE & CODE_FTP))
           {
             Button_Filtre_CODE+=CODE_FTP;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"FTP");
           }
      break;
      case 23:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix      =CODE_TELNET;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_TELNET))
           {
             Button_Filtre_CODE+=CODE_TELNET;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"TELNET");
           }
      break;
      case 25:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix      =CODE_SMTP;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_SMTP))
           {
             Button_Filtre_CODE+=CODE_SMTP;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"SMTP");
           }
      break;
      case 53:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix      =CODE_DNS;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_DNS))
           {
             Button_Filtre_CODE+=CODE_DNS;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"DNS");
           }
      break;
      case 68:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix      =CODE_DHCP;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_DHCP))
           {
             Button_Filtre_CODE+=CODE_DHCP;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"DHCP");
           }
      break;
      case 80:
      case 8080:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix      =CODE_HTTP;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_HTTP))
           {
             Button_Filtre_CODE+=CODE_HTTP;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"HTTP");
           }
      break;
      case 110:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix      =CODE_POP3;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_POP3))
           {
             Button_Filtre_CODE+=CODE_POP3;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"POP3");
           }
      break;
      case 137:
      case 138:
      case 139:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix  =CODE_NETBIOS;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_NETBIOS))
           {
             Button_Filtre_CODE+=CODE_NETBIOS;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"NETBIOS");
           }
      break;
      case 143:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix     =CODE_IMAP;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_IMAP))
           {
             Button_Filtre_CODE+=CODE_IMAP;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"IMAP");
           }
      break;
      case 161:
      case 162:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix =CODE_SNMP;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_SNMP))
           {
             Button_Filtre_CODE+=CODE_SNMP;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"SNMP");
           }
      break;
      case 389:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix     =CODE_LDAP;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_LDAP))
           {
             Button_Filtre_CODE+=CODE_LDAP;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"LDAP");
           }
      break;
      case 443:
           #ifdef DEBUG_FILTRE
             printf("port:%d\n",port);
           #endif

           Trame_buffer[ID].protocoleChoix     =CODE_HTTPS;
           ret = 1;
           if (!(Button_Filtre_CODE&CODE_HTTPS))
           {
             Button_Filtre_CODE+=CODE_HTTPS;
             SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"HTTPS");
           }
      break;
      //default : Trame_buffer[ID].protocoleChoix     =CODE_INIT;break;
    }
    return ret;
    ReleaseMutex(hMutex_TRAME_BUFFER);
}
//------------------------------------------------------------------------------
//test si la machine n'est pas dans la liste on renvoir OK
BOOL TestMachineNExistePas(char *ip)
{
  //return 0;//test

  //on évite les machines en broadcast ^^
  if (!strcmp(ip,"0.0.0.0") || !strcmp(ip,"255.255.255.255") || !strcmp(ip,"127.0.0.1")) return 0;

  //récupération du nombre d'items
  HANDLE hListViewMachine = GetDlgItem(Tabl[TAB_SNIFF],LSTV);
  int i, count = ListView_GetItemCount(hListViewMachine);
  char tmpip[256];
  for (i=0;i<count;i++)
  {
    ListView_GetItemText(hListViewMachine,i,COL_IP_SNIFF,tmpip,255);
    if (!strcmp(ip,tmpip)) return 0;
  }

  return 1;
}
//------------------------------------------------------------------------------
BOOL TestMacMachineNExistePas(char *ip,char *MacDst)
{
  //return 0;//test

  //on évite les machines en broadcast ^^
  if (!strcmp(ip,"0.0.0.0") || !strcmp(ip,"255.255.255.255")) return 0;

  //récupération du nombre d'items
  HANDLE hListViewMachine = GetDlgItem(Tabl[TAB_SNIFF],LSTV);
  int i, count = ListView_GetItemCount(hListViewMachine);
  char tmpip[256];
  for (i=0;i<count;i++)
  {
    ListView_GetItemText(hListViewMachine,i,COL_IP_SNIFF,tmpip,255);
    if (!strcmp(ip,tmpip))
    {
      //on test si une adresse MAC est présente si non on ajoute l'adresse donné en paramètre
      char tmpMac[18]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0";
      ListView_GetItemText(hListViewMachine,i,COL_MAC_SNIFF,tmpMac,18);
      if (strlen(tmpMac)<17)
        ListView_SetItemText(hListViewMachine,i,COL_MAC_SNIFF,MacDst);

      return 0;
    }
  }
  return 1;
}
//------------------------------------------------------------------------------
//void TraitementTramearp(ARP_HDR *trame, char *MacSrc, char *MacDst,unsigned long int ID,unsigned int VLAN_ID, int proto_type)
void TraitementTramearp(unsigned int *buffer,unsigned long int ID)
{
  char tmp[MAX_PATH];
  long int itemPosM,itemPosM2,itemPosP;
  LVITEM lvi;
  HANDLE hListViewMachine = GetDlgItem(Tabl[TAB_SNIFF],LSTV);             //liste des machines sur le réseau (sans doublon)
  HANDLE hListViewPaquet = HListView_ref;//liste des paquets (avec gestion des filtres)

 //suivant le type de trame
 WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
 switch (Trame_buffer[ID].ProtoType)
 {
   case 0x8100: //802.1Q
        {
          ARP_HDR_802_1Q *trame;
          trame = (ARP_HDR_802_1Q*)buffer;

          //ips src + dst
          _snprintf(Trame_buffer[ID].ip_src,TAILLE_IP,"%d.%d.%d.%d",trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]);
          _snprintf(Trame_buffer[ID].ip_dst,TAILLE_IP,"%d.%d.%d.%d",trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]);

          //adresses mac src + dst
          _snprintf(Trame_buffer[ID].mac_src,18,"%02X:%02X:%02X:%02X:%02X:%02X",trame->h_source[0],trame->h_source[1],trame->h_source[2],trame->h_source[3],trame->h_source[4],trame->h_source[5]);
          _snprintf(Trame_buffer[ID].mac_dst,18,"%02X:%02X:%02X:%02X:%02X:%02X",trame->h_dest[0],trame->h_dest[1],trame->h_dest[2],trame->h_dest[3],trame->h_dest[4],trame->h_dest[5]);

          //Autres paramètres
          Trame_buffer[ID].protocoleChoix = CODE_ARP;

          //gestion des listes + affichage

          //test si la machine existe ou non
          BOOL MachineNExistePas = FALSE;
          BOOL MachineNExistePas2 = FALSE;
          if(strcmp(Trame_buffer[ID].mac_src,"FF:FF:FF:FF:FF:FF")!=0 && strcmp(Trame_buffer[ID].mac_src,"00:00:00:00:00:00")!=0)
          {
             //pour supprimer l'ip source ^^
             if (strcmp(Trame_buffer[ID].ip_src,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
               MachineNExistePas = TestMachineNExistePas(Trame_buffer[ID].ip_src);
          }

          if(strcmp(Trame_buffer[ID].mac_dst,"FF:FF:FF:FF:FF:FF")!=0 && strcmp(Trame_buffer[ID].mac_dst,"00:00:00:00:00:00")!=0)
          {
             //pour supprimer l'ip source ^^
             if (strcmp(Trame_buffer[ID].ip_dst,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
               MachineNExistePas = TestMachineNExistePas(Trame_buffer[ID].ip_dst);
          }

          //ajout d'une ligne pour la liste des machines
          lvi.mask = LVIF_TEXT|LVIF_PARAM;
          lvi.iSubItem = 0;
          lvi.lParam = LVM_SORTITEMS;
          lvi.pszText="";

          if(MachineNExistePas)
          {
            lvi.iItem = ListView_GetItemCount(hListViewMachine);
            itemPosM = ListView_InsertItem(hListViewMachine, &lvi);
            ListView_SetItemText(hListViewMachine,itemPosM,COL_MAC_SNIFF,Trame_buffer[ID].mac_src);
            ListView_SetItemText(hListViewMachine,itemPosM,COL_SRC,Trame_buffer[ID].ip_src);

            if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)
              ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,"ARP REQUEST")
            if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)
              ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,"ARP REPLY")
            else ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,"ARP");
          }
          if(MachineNExistePas2)
          {
            lvi.iItem = ListView_GetItemCount(hListViewMachine);
            itemPosM2 = ListView_InsertItem(hListViewMachine, &lvi);
            ListView_SetItemText(hListViewMachine,itemPosM,COL_MAC_SNIFF,Trame_buffer[ID].mac_dst);
            ListView_SetItemText(hListViewMachine,itemPosM2,COL_SRC,Trame_buffer[ID].ip_dst);

            if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)
              ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,"ARP REQUEST")
            else if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)
              ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,"ARP REPLY")
            else ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,"ARP");
          }else //gestion si la machine existe mais que l'adresse MAC n'est pas renseigné
          {
            TestMacMachineNExistePas(Trame_buffer[ID].ip_dst,Trame_buffer[ID].mac_dst);
          }

          //ajout d'une ligne pour la liste des paquets (avec gestion des filtres)
          lvi.iItem = ListView_GetItemCount(hListViewPaquet);
          itemPosP = ListView_InsertItem(hListViewPaquet, &lvi);
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_ARP);

          //[SRC] & [DST] (suite)
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_SRC,Trame_buffer[ID].ip_src);
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_DST,Trame_buffer[ID].ip_dst);

          //ID
          _snprintf(tmp,MAX_PATH,"%d",ID);
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_ID,tmp);

          //[PORT+PROTOCOL]
          if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)
          {
            ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,"ARP REQUEST");
          }else if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)
          {
            ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,"ARP REPLY");
          }else
          {
            ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,"ARP");
          }

          //gestion ajout dans les filtres
          if (!(Button_Filtre_CODE&CODE_ARP))
          {
            Button_Filtre_CODE+=CODE_ARP;
            SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"ARP");
          }
          RefreshListeView(hListViewPaquet,itemPosP);
        }
   break;
   case 0x0806://ARP
        {
          ARP_HDR *trame;
          trame = (ARP_HDR*)buffer;

          //ips src + dst
          _snprintf(Trame_buffer[ID].ip_src,TAILLE_IP,"%d.%d.%d.%d",trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]);
          _snprintf(Trame_buffer[ID].ip_dst,TAILLE_IP,"%d.%d.%d.%d",trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]);

          //adresses mac src + dst
          _snprintf(Trame_buffer[ID].mac_src,18,"%02X:%02X:%02X:%02X:%02X:%02X",trame->h_source[0],trame->h_source[1],trame->h_source[2],trame->h_source[3],trame->h_source[4],trame->h_source[5]);
          _snprintf(Trame_buffer[ID].mac_dst,18,"%02X:%02X:%02X:%02X:%02X:%02X",trame->h_dest[0],trame->h_dest[1],trame->h_dest[2],trame->h_dest[3],trame->h_dest[4],trame->h_dest[5]);

          //Autres paramètres
          Trame_buffer[ID].protocoleChoix = CODE_ARP;

          //gestion des listes + affichage

          //test si la machine existe ou non
          BOOL MachineNExistePas = FALSE;
          BOOL MachineNExistePas2 = FALSE;
          if(strcmp(Trame_buffer[ID].mac_src,"FF:FF:FF:FF:FF:FF")!=0 && strcmp(Trame_buffer[ID].mac_src,"00:00:00:00:00:00")!=0)
          {
             //pour supprimer l'ip source ^^
             if (strcmp(Trame_buffer[ID].ip_src,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
               MachineNExistePas = TestMachineNExistePas(Trame_buffer[ID].ip_src);
          }

          if(strcmp(Trame_buffer[ID].mac_dst,"FF:FF:FF:FF:FF:FF")!=0 && strcmp(Trame_buffer[ID].mac_dst,"00:00:00:00:00:00")!=0)
          {
             //pour supprimer l'ip source ^^
             if (strcmp(Trame_buffer[ID].ip_dst,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
               MachineNExistePas2 = TestMachineNExistePas(Trame_buffer[ID].ip_dst);
          }

          //ajout d'une ligne pour la liste des machines
          lvi.mask = LVIF_TEXT|LVIF_PARAM;
          lvi.iSubItem = 0;
          lvi.lParam = LVM_SORTITEMS;
          lvi.pszText="";

          if(MachineNExistePas)
          {
            lvi.iItem = ListView_GetItemCount(hListViewMachine);
            itemPosM = ListView_InsertItem(hListViewMachine, &lvi);
            ListView_SetItemText(hListViewMachine,itemPosM,COL_MAC_SNIFF,Trame_buffer[ID].mac_src);
            ListView_SetItemText(hListViewMachine,itemPosM,COL_SRC,Trame_buffer[ID].ip_src);

            if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)
              ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,"ARP REQUEST")
            if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)
              ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,"ARP REPLY")
            else ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,"ARP");
          }
          if(MachineNExistePas2)
          {
            lvi.iItem = ListView_GetItemCount(hListViewMachine);
            itemPosM2 = ListView_InsertItem(hListViewMachine, &lvi);
            ListView_SetItemText(hListViewMachine,itemPosM,COL_MAC_SNIFF,Trame_buffer[ID].mac_dst);
            ListView_SetItemText(hListViewMachine,itemPosM2,COL_SRC,Trame_buffer[ID].ip_dst);

            if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)
              ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,"ARP REQUEST")
            else if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)
              ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,"ARP REPLY")
            else ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,"ARP");
          }else //gestion si la machine existe mais que l'adresse MAC n'est pas renseigné
          {
            TestMacMachineNExistePas(Trame_buffer[ID].ip_dst,Trame_buffer[ID].mac_dst);
          }

          //ajout d'une ligne pour la liste des paquets (avec gestion des filtres)
          lvi.iItem = ListView_GetItemCount(hListViewPaquet);
          itemPosP = ListView_InsertItem(hListViewPaquet, &lvi);
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_ARP);

          //[SRC] & [DST] (suite)
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_SRC,Trame_buffer[ID].ip_src);
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_DST,Trame_buffer[ID].ip_dst);

          //ID
          _snprintf(tmp,MAX_PATH,"%d",ID);
          ListView_SetItemText(hListViewPaquet,itemPosP,COL_ID,tmp);

          //[PORT+PROTOCOL]
          if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)
          {
            ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,"ARP REQUEST");
          }else if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)
          {
            ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,"ARP REPLY");
          }else
          {
            ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,"ARP");
          }

          //gestion ajout dans les filtres
          if (!(Button_Filtre_CODE&CODE_ARP))
          {
            Button_Filtre_CODE+=CODE_ARP;
            SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"ARP");
          }
          RefreshListeView(hListViewPaquet,itemPosP);
        }
   break;
 }
 ReleaseMutex(hMutex_TRAME_BUFFER);
}
//------------------------------------------------------------------------------
void TraitementTrameOther(void *trame, char *MacSrc, char *MacDst,unsigned long int ID,char *chaine_type, long code)
{
  char tmp[MAX_PATH],IpSrc[17+20],IpDst[17+20];
  long int itemPosM,itemPosP;
  LVITEM lvi;
  //HANDLE hListViewMachine = GetDlgItem(Tabl[TAB_SNIFF],LSTV);             //liste des machines sur le réseau (sans doublon)
  HANDLE hListViewPaquet = HListView_ref;//liste des paquets (avec gestion des filtres)

  //[SRC] [DST](début pour vérifier si la machine est déja dans la base)

  //ajout d'une ligne pour la liste des machine
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";

  //ajout d'une ligne pour la liste des paquets (avec gestion des filtres)
  lvi.iItem = ListView_GetItemCount(hListViewPaquet);
  itemPosP = ListView_InsertItem(hListViewPaquet, &lvi);

  //gestion ajout dans les filtres
  Trame_buffer[ID].protocoleChoix=code;
  if (!(Button_Filtre_CODE&code))
  {
   Button_Filtre_CODE+=code;

   if (code == CODE_OTHER)
     SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"Autre");
   else
     SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)chaine_type);
  }

  //gestion des protocoles spécifiques ^^
  switch(code)
  {
   case CODE_IPV6:
      ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_IPV6);

      //on gère aussi le type de protocole en IPV6 ^^
      IPV6_HDRM *trame_bis;
      trame_bis = (IPV6_HDRM *)(Trame_buffer[ID].buffer+3);

      switch(trame_bis->ip_protocol)
      {
        case 0x00 : strncat(chaine_type," - hop-by-hop option\0",MAX_PATH);break;
        case IPPROTO_ICMP : strncat(chaine_type," - ICMP\0",MAX_PATH);break;
        case 0x3A : strncat(chaine_type," - ICMPv6\0",MAX_PATH);break;
        case 0x58 : strncat(chaine_type," - CISCO EIGRP\0",MAX_PATH);break;
        case IPPROTO_IGMP : strncat(chaine_type," - IGMP\0",MAX_PATH);break;
        case IPPROTO_GGP : strncat(chaine_type," - GGP\0",MAX_PATH);break;
        case IPPROTO_TCP : strncat(chaine_type," - TCP\0",MAX_PATH);break;
        case IPPROTO_PUP : strncat(chaine_type," - PUP\0",MAX_PATH);break;
        case IPPROTO_UDP : strncat(chaine_type," - UDP\0",MAX_PATH);break;
        case IPPROTO_IDP : strncat(chaine_type," - IDP\0",MAX_PATH);break;
        case IPPROTO_ND : strncat(chaine_type," - ND\0",MAX_PATH);break;
        case IPPROTO_RAW : strncat(chaine_type," - RAW\0",MAX_PATH);break;
        case IPPROTO_VRRP: strncat(chaine_type," - VRRP\0",MAX_PATH);break;
       // default : strncat(chaine_type," - Inconnu\0",MAX_PATH);break;
        default :
          if (trame_bis->ip_protocol>0 && trame_bis->ip_protocol<138)
          {
            strncat(chaine_type," - ",MAX_PATH);
            strncat(chaine_type,ProtocolesTypesNames[trame_bis->ip_protocol],MAX_PATH);
            strncat(chaine_type,"\0",MAX_PATH);break;
          }else
            strncat(chaine_type," - Inconnu\0",MAX_PATH);break;
      }break;
   default :
     ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_DEFAULT);
  }

  //[SRC] & [DST]
  ListView_SetItemText(hListViewPaquet,itemPosP,COL_SRC,MacSrc);
  ListView_SetItemText(hListViewPaquet,itemPosP,COL_DST,MacDst);

  WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  strcpy(Trame_buffer[ID].mac_src,MacSrc);
  strcpy(Trame_buffer[ID].mac_dst,MacDst);

  //type
  ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,chaine_type);

  //ID
  _snprintf(tmp,MAX_PATH,"%d",ID);
  ListView_SetItemText(hListViewPaquet,itemPosP,COL_ID,tmp);
  ReleaseMutex(hMutex_TRAME_BUFFER);


  //HANDLE hListView2 = GetDlgItem(Tabl[TAB_SNIFF],LSTV_PAQUETS);

  //gestion des données
  //ListView_SetItemText(hListViewPaquet,itemPosP,COL_DONNES,(unsigned char *)trame);
  RefreshListeView(hListViewPaquet,itemPosP);
}

//------------------------------------------------------------------------------
/*plantage ici pour le raw socket*/
void TraitementTrameIPV4(IPV4_HDR *trame, unsigned short taille_trame, char *MacSrc, char *MacDst,unsigned long int ID,unsigned int VLAN_ID)
{
  char tmp[MAX_PATH];
  char IpSrc[17+20],IpDst[17+20];
  //ajout d'un item à chacune des liste

  long int itemPosM,itemPosM2,itemPosP;
  LVITEM lvi;
  HANDLE hListViewMachine = GetDlgItem(Tabl[TAB_SNIFF],LSTV);       //liste des machines sur le réseau (sans doublon)
  HANDLE hListViewPaquet = HListView_ref;//liste des paquets (avec gestion des filtres)

  //[SRC] & [DST] (début pour vérifier si les machines sont déja dans la base)
  struct sockaddr_in source,dest;
  memset(&source, 0, sizeof(source));
  source.sin_addr.s_addr = trame->ip_srcaddr;

  memset(&dest, 0, sizeof(dest));
  dest.sin_addr.s_addr = trame->ip_destaddr;

  strcpy(IpSrc,inet_ntoa(source.sin_addr));
  strcpy(IpDst,inet_ntoa(dest.sin_addr));

  //test si la machine n'existe pas déjà
  BOOL MachineNExistePas = FALSE;
  BOOL MachineNExistePas2 = FALSE;

  if (MacSrc!=0)
  {
    if(strcmp(MacSrc,"FF:FF:FF:FF:FF:FF")!=0 && strcmp(MacSrc,"00:00:00:00:00:00")!=0)
    {
       //pour supprimer l'ip source ^^
       if (strcmp(IpSrc,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
         MachineNExistePas = TestMachineNExistePas(IpSrc);
    }
  }else if (strcmp(IpSrc,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
    MachineNExistePas = TestMachineNExistePas(IpSrc);

  if (MacDst!=0)
  {
    if(strcmp(MacDst,"FF:FF:FF:FF:FF:FF")!=0 && strcmp(MacDst,"00:00:00:00:00:00")!=0)
    {
       //pour supprimer l'ip source ^^
       if (strcmp(IpDst,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
         MachineNExistePas2 = TestMachineNExistePas(IpDst);
    }
  }else if (strcmp(IpDst,If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip)!=0)
    MachineNExistePas2 = TestMachineNExistePas(IpDst);

  if (VLAN_ID>0)
  {
    snprintf(tmp,20,"[VLAN ID :%d]",VLAN_ID);
    strcat(IpDst,tmp);
    strcat(IpSrc,tmp);

    strcat(IpDst,"\0");
    strcat(IpSrc,"\0");
  }

  //ajout d'une ligne pour la liste des machine
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";

  if(MachineNExistePas)
  {
    lvi.iItem = ListView_GetItemCount(hListViewMachine);
    itemPosM = ListView_InsertItem(hListViewMachine, &lvi);
    ListView_SetItemText(hListViewMachine,itemPosM,COL_MAC_SNIFF,MacSrc);
  }

  if(MachineNExistePas2)
  {
    lvi.iItem = ListView_GetItemCount(hListViewMachine);
    itemPosM2 = ListView_InsertItem(hListViewMachine, &lvi);
    ListView_SetItemText(hListViewMachine,itemPosM2,COL_MAC_SNIFF,MacDst);
  }

  //ajout d'une ligne pour la liste des paquets (avec gestion des filtres)
  lvi.iItem = ListView_GetItemCount(hListViewPaquet);
  itemPosP = ListView_InsertItem(hListViewPaquet, &lvi);

  //[SRC] & [DST] (suite)

  if(MachineNExistePas) ListView_SetItemText(hListViewMachine,itemPosM,COL_SRC,IpSrc);

  ListView_SetItemText(hListViewPaquet,itemPosP,COL_SRC,IpSrc);

  if(MachineNExistePas2) ListView_SetItemText(hListViewMachine,itemPosM2,COL_SRC,IpDst);

  ListView_SetItemText(hListViewPaquet,itemPosP,COL_DST,IpDst);

  //copie dans la struct trame
  //ip

  WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  strncpy(Trame_buffer[ID].ip_src,IpSrc,TAILLE_IP);
  strncpy(Trame_buffer[ID].ip_dst,IpDst,TAILLE_IP);
  //mac
  if (MacSrc != 0) strncpy(Trame_buffer[ID].mac_src,MacSrc,TAILLE_MAC);
  else Trame_buffer[ID].mac_src[0]=0;

  if (MacDst != 0) strncpy(Trame_buffer[ID].mac_dst,MacDst,TAILLE_MAC);
  else Trame_buffer[ID].mac_dst[0]=0;

  //ID
  _snprintf(tmp,MAX_PATH,"%d",ID);
  ListView_SetItemText(hListViewPaquet,itemPosP,COL_ID,tmp);

  //[PORT+PROTOCOL]

  //le port n'est valable que en UDP/TCP
  if (trame->ip_protocol == IPPROTO_TCP)
  {
    ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_TCP);

    TCP_HDR *tcpheader;
    tcpheader=(TCP_HDR*)(((unsigned char *)trame)+trame->ip_header_len*4);

    if(MachineNExistePas)
    {
      _snprintf(tmp,256,"Port SRC:%d / %s",ntohs(tcpheader->source_port),(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
      ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,tmp);
    }

    if(MachineNExistePas2)
    {
      _snprintf(tmp,256,"Port DST:%d / %s",ntohs(tcpheader->dest_port),(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
      ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,tmp);
    }

    _snprintf(tmp,256,"Port SRC:%d >> DST:%d / %s",ntohs(tcpheader->source_port),ntohs(tcpheader->dest_port),(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));

    //copie dans la struct trame
    if(!TestPorts(ntohs(tcpheader->source_port),ID))
      if(!TestPorts(ntohs(tcpheader->dest_port),ID))
       Trame_buffer[ID].protocoleChoix = CODE_OTHER;

    //Trame_buffer[ID].num_sequence = tcpheader->sequence;
    Trame_buffer[ID].src_port=ntohs(tcpheader->source_port);
    Trame_buffer[ID].dst_port=ntohs(tcpheader->dest_port);

    ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,tmp);
  }else if (trame->ip_protocol == IPPROTO_UDP)
  {
    ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_UDP);
    UDP_HDR *udpheader;
    udpheader=(UDP_HDR*)(((unsigned char *)trame)+trame->ip_header_len*4);

    if(MachineNExistePas)
    {
      _snprintf(tmp,256,"Port SRC:%d / %s",ntohs(udpheader->source_port),(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
      ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,tmp);
    }

    if(MachineNExistePas2)
    {
      _snprintf(tmp,256,"Port DST:%d / %s",ntohs(udpheader->dest_port),(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
      ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,tmp);
    }
    //copie dans la struct trame
    if(!TestPorts(ntohs(udpheader->source_port),ID))
    {
      if(!TestPorts(ntohs(udpheader->dest_port),ID))
      {
       Trame_buffer[ID].protocoleChoix = CODE_OTHER;
      _snprintf(tmp,256,"Port SRC:%d >> DST:%d / %s",ntohs(udpheader->source_port),ntohs(udpheader->dest_port),(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
      }else
      {
        //on affiche le protocole
        switch(Trame_buffer[ID].protocoleChoix)
        {
          case CODE_DNS :
               _snprintf(tmp,256,"(DNS)Port SRC:%d >> DST:%d / %s"
               ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
               ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
          break;
          case CODE_SNMP :
               _snprintf(tmp,256,"(SNMP)Port SRC:%d >> DST:%d / %s"
               ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
               ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
          break;
          case CODE_LDAP :
               _snprintf(tmp,256,"(LDAP)Port SRC:%d >> DST:%d / %s"
               ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
               ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
          break;
          case CODE_NETBIOS :
               _snprintf(tmp,256,"(NETBIOS)Port SRC:%d >> DST:%d / %s"
               ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
               ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
          break;
          case CODE_DHCP :
               _snprintf(tmp,256,"(DHCP)Port SRC:%d >> DST:%d / %s"
               ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
               ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
          break;
          default:
               _snprintf(tmp,256,"Port SRC:%d >> DST:%d / %s"
               ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
               ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
          break;
        }
      }
    }else
    {
      //on affiche le protocole
      switch(Trame_buffer[ID].protocoleChoix)
      {
        case CODE_DNS :
             _snprintf(tmp,256,"(DNS)Port SRC:%d >> DST:%d / %s"
             ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
             ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
        break;
        case CODE_SNMP :
             _snprintf(tmp,256,"(SNMP)Port SRC:%d >> DST:%d / %s"
             ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
             ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
        break;
        case CODE_LDAP :
             _snprintf(tmp,256,"(LDAP)Port SRC:%d >> DST:%d / %s"
             ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
             ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
        break;
        case CODE_NETBIOS :
             _snprintf(tmp,256,"(NETBIOS)Port SRC:%d >> DST:%d / %s"
             ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
             ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
        break;
        case CODE_DHCP :
             _snprintf(tmp,256,"(DHCP)Port SRC:%d >> DST:%d / %s"
             ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
             ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
        break;
        default:
             _snprintf(tmp,256,"Port SRC:%d >> DST:%d / %s"
             ,ntohs(udpheader->source_port),ntohs(udpheader->dest_port)
             ,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
        break;
      }
    }



    //Trame_buffer[ID].num_sequence = 0;
    Trame_buffer[ID].src_port=ntohs(udpheader->source_port);
    Trame_buffer[ID].dst_port=ntohs(udpheader->dest_port);
    ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,tmp);

  }else // si aucun des deux
  {
    if(MachineNExistePas)ListView_SetItemText(hListViewMachine,itemPosM,COL_PROTOCOL,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));
    if(MachineNExistePas2)ListView_SetItemText(hListViewMachine,itemPosM2,COL_PROTOCOL,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));

    ListView_SetItemText(hListViewPaquet,itemPosP,COL_PORT,(char*)(trame->ip_protocol<138?ProtocolesTypesNames[trame->ip_protocol]:"<inconnu>"));

    //on vérifie si ICMP ou IGMP
    if (trame->ip_protocol == IPPROTO_ICMP)
    {
      ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_ICMP);
      Trame_buffer[ID].protocoleChoix = CODE_ICMP;
      //gestion des filtres
      if (!(Button_Filtre_CODE&CODE_ICMP))
      {
       Button_Filtre_CODE+=CODE_ICMP;
       SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"ICMP");
      }
    }else if (trame->ip_protocol == IPPROTO_IGMP)
    {
      ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_ICMP);
      Trame_buffer[ID].protocoleChoix = CODE_IGMP;
      //gestion des filtres
      if (!(Button_Filtre_CODE&CODE_IGMP))
      {
       Button_Filtre_CODE+=CODE_IGMP;
       SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"IGMP");
      }
    //VRRP
    }else if (trame->ip_protocol == IPPROTO_VRRP)
    {
      Trame_buffer[ID].protocoleChoix = CODE_VRRP;
      ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_RES);
      //gestion des filtres
      if (!(Button_Filtre_CODE&CODE_VRRP))
      {
       Button_Filtre_CODE+=CODE_VRRP;
       SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"VRRP");
      }
    }else
    {
      Trame_buffer[ID].protocoleChoix = CODE_OTHER;
      //Trame_buffer[ID].num_sequence = 0;
      ListView_SetItemText(hListViewPaquet,itemPosP,COL_REF_COLOR,T_CODE_DEFAULT);
      //gestion des filtres
      if (!(Button_Filtre_CODE&CODE_OTHER))
      {
       Button_Filtre_CODE+=CODE_OTHER;
       SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"Autre");
      }
    }
  }
  ReleaseMutex(hMutex_TRAME_BUFFER);
  RefreshListeView(hListViewPaquet,itemPosP);
  //gestion des données
  //ListView_SetItemText(hListViewPaquet,itemPosP,COL_DONNES,(((unsigned char *)trame)+(trame->ip_header_len*4)+sizeof(TCP_HDR)));
}
//------------------------------------------------------------------------------
void TraitementTramePCAP(unsigned char *paquet, unsigned short taille_trame,unsigned long int ID)
{
  //pour la récupération des adresse MAC ^^
  struct ethhdr *     eth;
  eth = (struct ethhdr *) paquet;

  WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  Trame_buffer[ID].ProtoType = htons(eth->h_proto);
  ReleaseMutex(hMutex_TRAME_BUFFER);

  //si un protocol connu on le traite
  switch (Trame_buffer[ID].ProtoType)
  {
    case 0x0800://si en IPV4
    {
      //on récupère les adresse mac de source et destination
      char src[256],dst[256];
      _snprintf(src,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
      _snprintf(dst,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);

      //traitement IPV4 (on revoi seulement la taille de la trame restante ^^)
      TraitementTrameIPV4((IPV4_HDR *)(paquet+sizeof(struct ethhdr)),taille_trame,src,dst,ID,0);
    }break;
    case 0x8100: //802.1Q
    {
      //on peut déterminer le numéro du VLAN dispo ici
      struct _802_1Q_hdr * eth_802;
      eth_802 = (struct _802_1Q_hdr *) paquet;

      //suivant le type
      switch (htons(eth_802->h_type))
      {
        case 0x0800://si en IPV4
        {
          //on récupère les adresse mac de source et destination
          char src[256],dst[256];
          _snprintf(src,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth_802->h_source[0],eth_802->h_source[1],eth_802->h_source[2],eth_802->h_source[3],eth_802->h_source[4],eth_802->h_source[5]);
          _snprintf(dst,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth_802->h_dest[0],eth_802->h_dest[1],eth_802->h_dest[2],eth_802->h_dest[3],eth_802->h_dest[4],eth_802->h_dest[5]);

          //traitement IPV4 (on revoi seulement la taille de la trame restante ^^)
          TraitementTrameIPV4((IPV4_HDR *)(paquet+sizeof(struct _802_1Q_hdr)),taille_trame,src,dst,ID,eth_802->h_vlan_ID);
        }break;
        case 0x0806://ARP
        {
          //traitement IPV4 (on revoi seulement la taille de la trame restante ^^)
          TraitementTramearp((int *)(paquet),ID);
        }break;
        default ://les autres
        {
          char src[256],dst[256];
          _snprintf(src,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth_802->h_source[0],eth_802->h_source[1],eth_802->h_source[2],eth_802->h_source[3],eth_802->h_source[4],eth_802->h_source[5]);
          _snprintf(dst,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth_802->h_dest[0],eth_802->h_dest[1],eth_802->h_dest[2],eth_802->h_dest[3],eth_802->h_dest[4],eth_802->h_dest[5]);

          char tmp_other[20];
          _snprintf(tmp_other,20,"802.1:%d",Trame_buffer[ID].ProtoType);
          TraitementTrameOther((void*)(paquet+sizeof(struct _802_1Q_hdr)),src,dst,ID,tmp_other,CODE_OTHER);
        }break;
      }
    }
    case 0x0806://ARP
    {
      //traitement de la requête ARP
      TraitementTramearp((int *)(paquet),ID);
    }break;
    case 0x88CC://LLDP
    {
      char src[256],dst[256];
      _snprintf(src,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
      _snprintf(dst,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);

      TraitementTrameOther((void*)(paquet+sizeof(struct ethhdr)),src,dst,ID,"802.1:LLDP",CODE_LLDP);
    }break;
    default:
            WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
            //Ethernet 802.3
            if (Trame_buffer[ID].ProtoType>=0 && Trame_buffer[ID].ProtoType<=0x05FF)
            {
              char src[256],dst[256];
              _snprintf(src,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
              _snprintf(dst,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);

              if (Trame_buffer[ID].ProtoType> 17 && paquet[17] == 00 && paquet[18] == 00 ) //STP
              {
                Trame_buffer[ID].protocoleChoix = CODE_STP;

                 //ajout de la trame à la liste
                 TraitementTrameOther((void*)(paquet+sizeof(struct ethhdr)),src,dst,ID,"802.3:STP",CODE_STP);
              }else
              {
                //Trame_buffer[ID].protocoleChoix = CODE_OTHER;
                char tmp_other[20];
                _snprintf(tmp_other,20,"802.3:%d",Trame_buffer[ID].ProtoType);
                TraitementTrameOther((void*)(paquet+sizeof(struct ethhdr)),src,dst,ID,tmp_other,CODE_OTHER);
              }
            }else
            {
              char src[256],dst[256];
              _snprintf(src,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
              _snprintf(dst,256,"%02X:%02X:%02X:%02X:%02X:%02X",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);

              long code = CODE_OTHER;

              //traitement
              char tmp_other[256];

              switch(Trame_buffer[ID].ProtoType)
              {
                case 0x1011: strcpy(tmp_other,"(Apple IP-over-IEEE 1394)");break;
                case 0x2452: strcpy(tmp_other,"(IEEE 802.11 : Centrino promiscuous)");break;
                case 0x6002: strcpy(tmp_other,"DEC DNA Remote Console");break;
                case 0x8035: strcpy(tmp_other,"RARP\0");break;
                case 0x809B: strcpy(tmp_other,"AppleTalk : Ethertalk\0");break;
                case 0x80F3: strcpy(tmp_other,"AppleTalk : AARP\0");break;
                case 0x8100: strcpy(tmp_other,"Ethernet 802.1Q : VLAN-tagged frame\0");break;
                case 0x8135: strcpy(tmp_other,"Novell IPX : alt\0");break;
                case 0x8138: strcpy(tmp_other,"Novell\0");break;
                case 0x86DD: strcpy(tmp_other,"IPV6\0");code = CODE_IPV6;break;
                case 0x872D: strcpy(tmp_other,"(Cisco Wireless Lan Context Control Protocol)");break;
                case 0x8808: strcpy(tmp_other,"MAC Control)\0");break;
                case 0x8819: strcpy(tmp_other,"CobraNet\0");break;
                case 0x8847: strcpy(tmp_other,"Multiprotocol Label Switching unicast\0");break;
                case 0x8848: strcpy(tmp_other,"Multiprotocol Label Switching multicast\0");break;
                case 0x8863: strcpy(tmp_other,"PPoE Discovery Stage\0");break;
                case 0x8864: strcpy(tmp_other,"PPoE Session Stage\0");break;
                case 0x886F: strcpy(tmp_other,"Microsoft Network Load Balancing\0");break;
                case 0x888E: strcpy(tmp_other,"IEEE 802.1X : EAP over LAN\0");break;
                case 0x889A: strcpy(tmp_other,"SCSI over Ethernet : HyperSCSI\0");break;
                case 0x88A2: strcpy(tmp_other,"ATA over Ethernet\0");break;
                case 0x88A4: strcpy(tmp_other,"EtherCAT Protocol\0");break;
                case 0x88A8: strcpy(tmp_other,"IEEE 802.1ad : Provider Bridging\0");break;
                case 0x88AB: strcpy(tmp_other,"Ethernet Powerlink\0");break;
                case 0x88B5: strcpy(tmp_other,"AVB Transport Protocol\0");break;
                case 0x88CC: strcpy(tmp_other,"Ethernet 802.1 : LLDP\0");break;
                case 0x88CD: strcpy(tmp_other,"SERCOS III\0");break;
                case 0x88D8: strcpy(tmp_other,"MEF-8 : Circuit Emulation Services over Ethernet\0");break;
                case 0x88E1: strcpy(tmp_other,"HomePlug\0");break;
                case 0x88E5: strcpy(tmp_other,"IEEE 802.1AE : MAC security\0");break;
                case 0x88F7: strcpy(tmp_other,"IEEE 1588 : Precision Time Protocol\0");break;
                case 0x8902: strcpy(tmp_other,"IEEE 802.1ag : Connectivity Fault Management\0");break;
                case 0x8906: strcpy(tmp_other,"Fibre Channel over Ethernet\0");break;
                case 0x8914: strcpy(tmp_other,"FCoE Initialization Protocol\0");break;
                case 0x9000: strcpy(tmp_other,"LOOP");break;
                case 0x9100: strcpy(tmp_other,"Q-in-Q\0");break;
                case 0xCAFE: strcpy(tmp_other,"LLT : Veritas Low Latency Transport\0");break;
                default :
                    if (Trame_buffer[ID].ProtoType>=0 && Trame_buffer[ID].ProtoType<=0x05FF)
                       strcpy(tmp_other,"(Ethernet 802.3)\0");
                    else _snprintf(tmp_other,254,"Inconnu:0x%04X (%d)\0",Trame_buffer[ID].ProtoType,Trame_buffer[ID].ProtoType);
              }
              TraitementTrameOther((void*)(paquet+sizeof(struct ethhdr)),src,dst,ID,tmp_other,code);
            }
            ReleaseMutex(hMutex_TRAME_BUFFER);
    break;
  }
}
//------------------------------------------------------------------------------
DWORD  WINAPI TraiterTrame(LPVOID lParam)
//BOOL TraiterTrame(unsigned long int numligne, HANDLE Listeview)
{
   //si il n'y as pas de sniff en cours et pas de chargement de fichier c bon ^^
   if (test_SNIFF || chargement_pcap_file)
   {
     TraiterTrame_En_cours = FALSE;
     return 0;
   }

   HANDLE Listeview = ((TT_test*)lParam)->Listeview;
   unsigned long int numligne = ((TT_test*)lParam)->id_item;

    //SendMessage(Listeview,LVM_GETNEXTITEM,-1,LVNI_FOCUSED);

   if (numligne>=0 && numligne<ListView_GetItemCount(Listeview))//si bien un item de sélectionné ^^
   {
     //gestion pour éviter les plantages
     BOOL wait_for_work = (numligne == ListView_GetItemCount(Listeview)-1 && test_SNIFF)?1:0;

     char tmp[MAX_PATH],tmp2[TAILLE_MAX_BUFFER_TRAME*2]="\0";
     int id =0;
     TV_INSERTSTRUCT TvItem;
     TvItem.hInsertAfter = TVI_LAST;
     ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
     TvItem.item.mask  = TVIF_TEXT;

     HANDLE HtreeView = GetDlgItem(HTrame,TREE_DONNEES);
     HTREEITEM Htree;

     //récupération de l'id de l'item (numéro de trame dans le buffer)
     tmp[0]=0;
     ListView_GetItemText(Listeview,numligne,COL_ID,tmp,MAX_PATH-1);
     id = atoi(tmp);

     //changement du titre de la fenêtre
     _snprintf(tmp2,40,"Infos Trame (ID:%s)",tmp);
     SetWindowText(HTrame,tmp2);

     //à partir d'ici on traite la trame
      TreeView_DeleteAllItems(HtreeView);

     //traitement du header
     if (wait_for_work)WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);

     //on ajoute une info de taille de trame ^^
     _snprintf(tmp2,MAX_PATH,"Taille total de la trame : %d octets",Trame_buffer[id].taille_buffer);
     TvItem.hParent = TVI_ROOT;
     TvItem.item.pszText = tmp2;
     TreeView_InsertItem(HtreeView, &TvItem);

     if (Trame_buffer[id].type == BOOL_TYPE_PCAP)
     {
       TvItem.hParent = TVI_ROOT;
       TvItem.item.pszText = "Header";
       Htree = TreeView_InsertItem(HtreeView, &TvItem);

       TvItem.hParent = Htree;

       //MAC dst + translation adresse
       strcpy(tmp,Trame_buffer[id].mac_dst);

       ReadMacAdressConstructeur(tmp);
       _snprintf(tmp2,MAX_PATH,"Adresse MAC destination : %s",tmp);
       TvItem.item.pszText = tmp2;
       TreeView_InsertItem(HtreeView, &TvItem);

       //MAC src + translation adresse
       strcpy(tmp,Trame_buffer[id].mac_src);
       ReadMacAdressConstructeur(tmp);

       _snprintf(tmp2,MAX_PATH,"Adresse MAC source : %s",tmp);
       TvItem.item.pszText = tmp2;
       TreeView_InsertItem(HtreeView, &TvItem);

       //protocole
       //http://standards.ieee.org/regauth/ethertype/eth.txt
       _snprintf(tmp2,MAX_PATH,"Type : 0x%04X ",Trame_buffer[id].ProtoType);
       switch (Trame_buffer[id].ProtoType)
       {
         case 0x0800: strcat(tmp2,"(IPV4)");break;
         case 0x0806: strcat(tmp2,"(ARP)");break;
         case 0x1011: strcat(tmp2,"(Apple IP-over-IEEE 1394)");break;
         case 0x2452: strcat(tmp2,"(IEEE 802.11 : Centrino promiscuous)");break;
         case 0x6002: strcat(tmp2,"(DEC DNA Remote Console)");break;
         case 0x8035: strcat(tmp2,"(RARP)");break;
         case 0x809B: strcat(tmp2,"(AppleTalk : Ethertalk)");break;
         case 0x80F3: strcat(tmp2,"(AppleTalk : AARP)");break;
         case 0x8100: strcat(tmp2,"(Ethernet 802.1Q : VLAN-tagged frame)");break;
         case 0x8135: strcat(tmp2,"(Novell IPX : alt)");break;
         case 0x8138: strcat(tmp2,"(Novell)");break;
         case 0x86DD: strcat(tmp2,"(IPV6)");break;
         case 0x872D: strcat(tmp2,"(Cisco Wireless Lan Context Control Protocol)");break;
         case 0x8808: strcat(tmp2,"(MAC Control)");break;
         case 0x8819: strcat(tmp2,"(CobraNet)");break;
         case 0x8847: strcat(tmp2,"(Multiprotocol Label Switching unicast)");break;
         case 0x8848: strcat(tmp2,"(Multiprotocol Label Switching multicast)");break;
         case 0x8863: strcat(tmp2,"(PPoE Discovery Stage)");break;
         case 0x8864: strcat(tmp2,"(PPoE Session Stage)");break;
         case 0x886F: strcat(tmp2,"(Microsoft Network Load Balancing)");break;
         case 0x888E: strcat(tmp2,"(IEEE 802.1X : EAP over LAN)");break;
         case 0x889A: strcat(tmp2,"(SCSI over Ethernet : HyperSCSI)");break;
         case 0x88A2: strcat(tmp2,"(ATA over Ethernet)");break;
         case 0x88A4: strcat(tmp2,"(EtherCAT Protocol)");break;
         case 0x88A8: strcat(tmp2,"(IEEE 802.1ad : Provider Bridging)");break;
         case 0x88AB: strcat(tmp2,"(Ethernet Powerlink)");break;
         case 0x88B5: strcat(tmp2,"(AVB Transport Protocol)");break;
         case 0x88CC: strcat(tmp2,"(Ethernet 802.1 : LLDP)");break;
         case 0x88CD: strcat(tmp2,"(SERCOS III)");break;
         case 0x88D8: strcat(tmp2,"(MEF-8 : Circuit Emulation Services over Ethernet)");break;
         case 0x88E1: strcat(tmp2,"(HomePlug)");break;
         case 0x88E5: strcat(tmp2,"(IEEE 802.1AE : MAC security)");break;
         case 0x88F7: strcat(tmp2,"(IEEE 1588 : Precision Time Protocol)");break;
         case 0x8902: strcat(tmp2,"(IEEE 802.1ag : Connectivity Fault Management)");break;
         case 0x8906: strcat(tmp2,"(Fibre Channel over Ethernet)");break;
         case 0x8914: strcat(tmp2,"(FCoE Initialization Protocol)");break;
         case 0x9000: strcat(tmp2,"(LOOP)");break;
         case 0x9100: strcat(tmp2,"(Q-in-Q)");break;
         case 0xCAFE: strcat(tmp2,"(LLT : Veritas Low Latency Transport)");break;
         default :
              if (Trame_buffer[id].ProtoType>=0 && Trame_buffer[id].ProtoType<=0x05FF)
                 strcat(tmp2,"(Ethernet 802.3)");
              else strcat(tmp2,"(Inconnu)");
       }

       TvItem.item.pszText = tmp2;
       TreeView_InsertItem(HtreeView, &TvItem);

       TreeView_Expand(HtreeView,Htree,TVE_EXPAND	);
     }

     int type = Trame_buffer[id].ProtoType;

     switch (type)
     {
       //IPV4 + PCAP
       case 0x0800:TraitementAffichageIPV4("Body : IPV4 (0x0800)",id,HtreeView,TVI_ROOT);break;
       //ARP + PCAP
       case 0x0806:TraitementAffichageARP("Body : ARP (0x0806)", Trame_buffer[id].buffer,Trame_buffer[id].taille_buffer,id,HtreeView,TVI_ROOT);break;
       //802.3:STP
       case 0x0027:TraitementAffichageSTP(id,HtreeView,TVI_ROOT);break;
       //802.1 : LLDP + PCAP
       case 0x88cc:TraitementAffichageLLDP(id,HtreeView,TVI_ROOT);break;
       //IPV6
       case 0x86DD:TraitementAffichageIPV6("Body : IPV6 (0x86DD)",id,HtreeView,TVI_ROOT);break;
       //traitement affichage par défaut
       default :
               if(Trame_buffer[id].type == BOOL_TYPE_RAW && Trame_buffer[id].ProtoType==0)
                  TraitementAffichageRAW("Body : RAW IPV4 (0x0800)",id,HtreeView,TVI_ROOT);
               else
                  TraitementAffichageOther(id,HtreeView,TVI_ROOT);
       break;
     }

     //ajout du contenu total de la trame en hexa
     TraitementTrameAllHexa(id,HtreeView,TVI_ROOT);

     //affichage de trame
     ShowWindow(GetDlgItem(HTrame,RE), SW_HIDE);
     ShowWindow(GetDlgItem(HTrame,TREE_DONNEES), SW_SHOW);
     ShowWindow(HTrame, SW_SHOW);
     if (wait_for_work)ReleaseMutex(hMutex_TRAME_BUFFER);
     TraiterTrame_En_cours = FALSE;

     //sélection du premier item ^^
     HTREEITEM hSitem = (HTREEITEM)SendMessage(GetDlgItem(HTrame,TREE_DONNEES), TVM_GETNEXTITEM, TVGN_CHILD ,(LPARAM)(HTREEITEM)TVI_ROOT);
     TreeView_Select(GetDlgItem(HTrame,TREE_DONNEES),hSitem,TVGN_FIRSTVISIBLE);
     return 1;
   }
   TraiterTrame_En_cours = FALSE;
   return 0;
}
//------------------------------------------------------------------------------
DWORD  WINAPI Sniff(LPVOID lParam)
{
  SOCKET sock;  // le socket
  struct sockaddr_in/* source,*/dest;  //structure des interfaces pour le réseau

  unsigned long taille=1;
  char tmp[TAILLE_TMP];
  unsigned char Buffer[TAILLE_MAX_BUFFER_TRAME+1];
  TRAME_BUFFER *trame_buffer_tmp;

  MiseEnGras(GetDlgItem(HPrincipal,BT_SNIFF),TRUE);

  //init pour le réseau
  test_SNIFF = 1;
  InitReseau(0);

  NB_trame_buffer = 0;
  //Trame_buffer = (TRAME_BUFFER *) realloc(Trame_buffer,(NB_trame_buffer+1)*(sizeof(TRAME_BUFFER)+1));

  //init de la gestion des filtres
  Button_Filtre_CODE = 0;
  FiltreApplique = 0;

  //suppression des items de la listebox
  SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
  SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"<<aucun filtre>>");

  //si les filtres sont activés on désactive et on affiche la bonne listeview
  if (ShowWindow(HListView_ref,SW_SHOW) == 0)
  {
    ShowWindow(HListView_ref,SW_SHOW);
    ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),SW_HIDE);
  }

  //on désactive les boutons
  //renommage du bouton start
  SetWindowText(GetDlgItem(Tabl[TAB_SNIFF],BT_START),Language->arreter);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_RAW),FALSE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_PCAP),FALSE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_PROMISCUOUS),FALSE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),FALSE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_SAVE),FALSE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_LOAD),FALSE);


  //on affiche la liste des machine - chek de disco
  CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV_FILTRE),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),SW_SHOW) ;
  ShowWindow(HListView_ref,SW_SHOW);

  /*CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_CHECKED);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),SW_HIDE) ;
  ShowWindow(HListView_ref,SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_SHOW);*/

  //on vide les listes
  ListView_DeleteAllItems(GetDlgItem(Tabl[TAB_SNIFF],LSTV));
  ListView_DeleteAllItems(HListView_ref);

  //on sélectionne tous les items de la listebox
  SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE, LB_SELITEMRANGEEX,(WPARAM) 0,
  (LPARAM)SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE,LB_GETCOUNT,(WPARAM)0,(LPARAM)0)-1);

  //gestion de l'utilisation des rawsockets ou de la LIB PCAP
  if (SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_CHK_RAW, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
  {
    Journalisation_EcrireFichier(NULL, NULL,"DECOUVERTE - DEBUT des tests (RAW)!");
    //ouverture du socket en mode RAW
    if ((sock = socket(AF_INET, SOCK_RAW, IPPROTO_IP)) == INVALID_SOCKET)
        MessageBox(0,"Accès au socket réseau impossible!!!","Erreur socket",MB_OK|MB_TOPMOST|MB_ICONWARNING);
    else
    {
      //init de la structure
    	memset(&dest, 0, sizeof(dest));

      //test si n'importe quel interface on récupère tout
      dest.sin_addr.s_addr = inet_addr(If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].ip);
      dest.sin_family      = AF_INET;
      dest.sin_port        = 0;

      //bind pour écoute
      if (bind(sock,(struct sockaddr *)&dest,sizeof(dest)) == SOCKET_ERROR)
          MessageBox(0,"Bind réseau impossible!!!","Erreur bind",MB_OK|MB_TOPMOST|MB_ICONWARNING);
      else
      {
          #ifdef DEBUG_SNIFF_RAW
            printf("BIND OK\n");
          #endif

          //test pour activer le mode promiscuous (traitement des adresses qui ne sont pas les miennes également
        	if (SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_CHK_PROMISCUOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
        	{
             //pas de contrôle ici car en cas d'utilisation de certaines cartes réseaux WIFI bug
             //le mode promiscuous est activé par défaut pour le RAW (car quelques bugs sinon)
             WSAIoctl(sock, SIO_RCVALL, &taille, sizeof(taille), 0, 0,(LPDWORD)&tmp,0, 0);
          }

    	    //on lit la trame tant qu'elle existe
    	    do
    	    {
            //récupération des trames
            memset(&Buffer, 0, sizeof(Buffer));
            taille = recvfrom(sock,Buffer,TAILLE_MAX_BUFFER_TRAME,0,0,0);

            #ifdef DEBUG_SNIFF_RAW
              printf("recvfrom : taille : %d\n",taille);
            #endif

            //traitement des trames
            if (taille > 0)
            {
              //pour chaque nouvelle trame (on ajoute une trame en mémoire pour traitement)
              WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
              trame_buffer_tmp = (TRAME_BUFFER *) realloc(Trame_buffer,(NB_trame_buffer+1)*(sizeof(TRAME_BUFFER)+1));

              //erreur de reallocation : on quit
              if (trame_buffer_tmp == NULL)
              {
                MessageBox(0,"Erreur : mémoire insuffisante pour continuer la capture!!!","Erreur capture de trame",MB_OK|MB_TOPMOST|MB_ICONWARNING);
                break;
              }else
              {
                Trame_buffer = trame_buffer_tmp;
              }

              Trame_buffer[NB_trame_buffer].type = BOOL_TYPE_RAW;
              Trame_buffer[NB_trame_buffer].src_port=0;
              Trame_buffer[NB_trame_buffer].dst_port=0;

              if (taille<TAILLE_MAX_BUFFER_TRAME)
              {
                //memset(Trame_buffer[NB_trame_buffer].buffer_header, 0, sizeof(struct pcap_pkthdr));

                Trame_buffer[NB_trame_buffer].taille_buffer = taille;
                memcpy(Trame_buffer[NB_trame_buffer].buffer,Buffer,taille);
              }else
              {
                //memset(Trame_buffer[NB_trame_buffer].buffer_header, 0, sizeof(struct pcap_pkthdr));

                Trame_buffer[NB_trame_buffer].taille_buffer = TAILLE_MAX_BUFFER_TRAME;
                memcpy(Trame_buffer[NB_trame_buffer].buffer,Buffer,TAILLE_MAX_BUFFER_TRAME);
              }
              Trame_buffer[NB_trame_buffer].ProtoType = 0;
              TraitementTrameIPV4((IPV4_HDR *)Buffer, taille,0,0,NB_trame_buffer++,0);
              ReleaseMutex(hMutex_TRAME_BUFFER);
            }
          }while (taille>0);
      }
    	closesocket(sock);
    }
  }else //LIBPCAP
  {
    if (PCAP_OK)
    {
      Journalisation_EcrireFichier(NULL, NULL,"DECOUVERTE - DEBUT des tests (PCAP) !");
      //sélection de la carte réseau sur laquelle écouté (ici pour le test la 2)
      char errbuf[PCAP_ERRBUF_SIZE];

      //test pour activer le mode promiscuous
      //TAILLE_MAX_BUFFER_HEADER+TAILLE_MAX_BUFFER_TRAME
      if ((captureHandle = MYpcap_open(If_Dev[SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0)].description,65536,SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_CHK_PROMISCUOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0), 1000, NULL, errbuf))!=0)
      {
        struct pcap_pkthdr* packetHeader;
        const  u_char *     packetData;
        int res;

        while((res=MYpcap_next_ex(captureHandle, &packetHeader, &packetData))>= 0)
        {
          if(res == 0) continue;

          //pour chaque nouvelle trame (on ajoute une trame en mémoire pour traitement)
          WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
          Trame_buffer = (TRAME_BUFFER *) realloc(Trame_buffer,(NB_trame_buffer+1)*(sizeof(TRAME_BUFFER)+1));
          Trame_buffer[NB_trame_buffer].type = BOOL_TYPE_PCAP;
          Trame_buffer[NB_trame_buffer].src_port=0;
          Trame_buffer[NB_trame_buffer].dst_port=0;

          if (packetHeader->caplen<TAILLE_MAX_BUFFER_TRAME)
          {
            memcpy(Trame_buffer[NB_trame_buffer].buffer_header,packetHeader,sizeof(struct pcap_pkthdr));

            Trame_buffer[NB_trame_buffer].taille_buffer = packetHeader->caplen;
            memcpy(Trame_buffer[NB_trame_buffer].buffer,packetData,Trame_buffer[NB_trame_buffer].taille_buffer);
          }else //sinon on tronque le paquet
          {
            memcpy(Trame_buffer[NB_trame_buffer].buffer_header,packetHeader,sizeof(struct pcap_pkthdr));

            Trame_buffer[NB_trame_buffer].taille_buffer = TAILLE_MAX_BUFFER_TRAME;
            memcpy(Trame_buffer[NB_trame_buffer].buffer,packetData,TAILLE_MAX_BUFFER_TRAME);
          }
          ReleaseMutex(hMutex_TRAME_BUFFER);
          TraitementTramePCAP((unsigned char *)packetData,packetHeader->caplen,NB_trame_buffer++);
        }
        MYpcap_close(captureHandle);
      }
    }
  }

  stop:

  ReInitReseau(0);
  Tri(GetDlgItem(Tabl[TAB_SNIFF],LSTV),0,4); // seulement celui avec les IPS
  test_SNIFF = 0;
  //on réactive les boutons
  //renommage du bouton start
  SetWindowText(GetDlgItem(Tabl[TAB_SNIFF],BT_START),Language->demarrer);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_RAW),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_PCAP),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_PROMISCUOUS),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_SAVE),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_LOAD),TRUE);

  //on affiche la liste des machine
  CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV_FILTRE),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),SW_SHOW) ;
  ShowWindow(HListView_ref,SW_SHOW);
  /*CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_CHECKED);
  ShowWindow(HListView_ref,SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_SHOW);*/

  Journalisation_EcrireFichier(NULL, NULL,"DECOUVERTE - FIN des tests !");
  MiseEnGras(GetDlgItem(HPrincipal,BT_SNIFF),FALSE);
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI KillThreadSniff(LPVOID lParam)
{
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_START),FALSE);

  DWORD IDThread,a;
  GetExitCodeThread(Hsniff,&IDThread);
  TerminateThread(Hsniff,IDThread);


  //gestion de l'utilisation des rawsockets ou de la LIB PCAP
  if (SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_CHK_RAW, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
  {
    closesocket(sock);
  }else //LIBPCAP
  {
    MYpcap_close(captureHandle);
  }

  ReInitReseau(0);
  test_SNIFF = 0;
  //on réactive les boutons
  //renommage du bouton start
  SetWindowText(GetDlgItem(Tabl[TAB_SNIFF],BT_START),Language->demarrer);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_RAW),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_PCAP),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_CHK_PROMISCUOUS),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_START),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_SAVE),TRUE);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],BT_LOAD),TRUE);

  //on affiche la liste des machines
  /*CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_CHECKED);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),SW_HIDE) ;
  ShowWindow(HListView_ref,SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_SHOW);*/

  Tri(GetDlgItem(Tabl[TAB_SNIFF],LSTV),0,4); // seulement celui avec les IPS
  Journalisation_EcrireFichier(NULL, NULL,"DECOUVERTE - FIN des tests!");
  ReleaseMutex(hMutex_TRAME_BUFFER);
  MiseEnGras(GetDlgItem(HPrincipal,BT_SNIFF),FALSE);
  return 0;
}
//------------------------------------------------------------------------------
DWORD  WINAPI Filtre_Sniff(LPVOID lParam)
{
  //récupération du text de l'item sélectionné
  int i = SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE, LB_GETCURSEL,(WPARAM) 0, (LPARAM)0);

  if (i!=LB_ERR && NB_trame_buffer>0)
  {
    //en fonction du texte on a un id
    char buffer[256]="\0";
    if (SendDlgItemMessage(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE, LB_GETTEXT,(WPARAM) i, (LPARAM)buffer)!=LB_ERR)
    {
      LVITEM lvi;
      lvi.mask = LVIF_TEXT|LVIF_PARAM;
      lvi.iSubItem = 0;
      lvi.lParam = LVM_SORTITEMS;
      lvi.pszText="";

      int i,j,count=0;

      HANDLE hListView = GetDlgItem(Tabl[TAB_SNIFF],LSTV_FILTRE);
      HANDLE hListView2 = HListView_ref;

      //on vide la listeview
      ListView_DeleteAllItems(hListView);

      affichage_filtre = 1;
      ShowWindow(hListView2,SW_HIDE);
      ShowWindow(hListView,SW_SHOW);

      char tmp1[MAX_PATH];

      //pour cas critique
      if (strlen(buffer)<1)return 0;

      //test si aucun filtre
      if (buffer[0] == '<')
      {
        //on masque la listeview de filtre
        affichage_filtre = 0;
        ShowWindow(hListView,SW_HIDE);
        ShowWindow(hListView2,SW_SHOW);

      //test si correspond au filtres standards
      }else if (buffer[0] == '#') //c'est un  filtre spécifique, construit
      {
        if (buffer[1]=='I')//filtrage par ip
        {
          //test si un filrage par adresse MAC
          if (buffer[6]==':')
          {
            char mac[TAILLE_MAC];
            sprintf(mac,"%s",&buffer[4]);

            #ifdef DEBUG_FILTRE
            printf("DEBUG - FILTRE IP/MAC : %s\n",mac);
            #endif

            for (i=0;i<NB_trame_buffer;i++)
            {
              if (!strcmp(Trame_buffer[i].mac_src,mac) || !strcmp(Trame_buffer[i].mac_dst,mac))
              {
                //ajout de l'item
                lvi.iItem = count;
                ListView_InsertItem(hListView, &lvi);

                //copie des colonnes de la listeview d'origine dans celle du filtre
                for(j=0;j<COL_ID+1;j++)
                {
                  ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
                  ListView_SetItemText(hListView,count,j,tmp1);
                }
                count++;
              }
            }
          }else
          {
            char ip[TAILLE_IP];
            sprintf(ip,"%s",&buffer[4]);

            #ifdef DEBUG_FILTRE
            printf("DEBUG - FILTRE IP : %s\n",ip);
            #endif

            for (i=0;i<NB_trame_buffer;i++)
            {
              if (!strcmp(Trame_buffer[i].ip_src,ip) || !strcmp(Trame_buffer[i].ip_dst,ip))
              {
                //ajout de l'item
                lvi.iItem = count;
                ListView_InsertItem(hListView, &lvi);

                //copie des colonnes de la listeview d'origine dans celle du filtre
                for(j=0;j<COL_ID+1;j++)
                {
                  ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
                  ListView_SetItemText(hListView,count,j,tmp1);
                }
                count++;
              }
            }
          }
        }else if (buffer[1]=='P')//filtrage par port
        {
          unsigned int port = atoi(&buffer[3]);

          #ifdef DEBUG_FILTRE
          printf("DEBUG - FILTRE PORT : %d(%s)\n",port,buffer);
          #endif
          for (i=0;i<NB_trame_buffer;i++)
          {
            if (Trame_buffer[i].src_port==port || Trame_buffer[i].dst_port==port)
            {
              //ajout de l'item
              lvi.iItem = count;
              ListView_InsertItem(hListView, &lvi);

              //copie des colonnes de la listeview d'origine dans celle du filtre
              for(j=0;j<COL_ID+1;j++)
              {
                ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
                ListView_SetItemText(hListView,count,j,tmp1);
              }
              count++;
            }
          }
        }
      }else if (!strcmp(buffer,"ARP")/*!strcmp(buffer,"ARP REQUEST") || !strcmp(buffer,"ARP REPLY")*/)
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_ARP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"DHCP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_DHCP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"IPV6"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_IPV6)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"DNS"))
      {

        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
         #ifdef DEBUG_FILTRE
           printf("DEBUG : Filtre - DNS (i:%d)(code:%d)\n",i,Trame_buffer[i].protocoleChoix);
         #endif

          if (Trame_buffer[i].protocoleChoix == CODE_DNS)
          {
           #ifdef DEBUG_FILTRE
             printf("DEBUG : Filtre - DNS OK\n");
           #endif


            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"SMTP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_SMTP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"POP3"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_POP3)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"IMAP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_IMAP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"SNMP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_SNMP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"Autre"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_OTHER)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"FTP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_FTP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"HTTP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_HTTP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"HTTPS"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_HTTPS)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"LDAP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_LDAP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"NETBIOS"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_NETBIOS)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"CDP"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_CDP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }

      }else if (!strcmp(buffer,"802.3:STP"))
      {
        //on ajoute seulement les items qui ont la mention STP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_STP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"802.1:LLDP"))
      {
        //on ajoute seulement les items qui ont la mention STP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_LLDP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"ICMP"))
      {
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_ICMP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"IGMP"))
      {
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_IGMP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"VRRP"))
      {
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_VRRP)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }else if (!strcmp(buffer,"TELNET"))
      {
        //on ajoute seulement les items qui ont la mention ARP
        for (i=0;i<NB_trame_buffer;i++)
        {
          if (Trame_buffer[i].protocoleChoix == CODE_TELNET)
          {
            //ajout de l'item
            lvi.iItem = count;
            ListView_InsertItem(hListView, &lvi);

            //copie des colonnes de la listeview d'origine dans celle du filtre
            for(j=0;j<COL_ID+1;j++)
            {
              ListView_GetItemText(hListView2,i,j,tmp1,MAX_PATH);
              ListView_SetItemText(hListView,count,j,tmp1);
            }
            count++;
          }
        }
      }
    }else
    {
      //on masque la listeview de filtre
      affichage_filtre = 0;
      ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV_FILTRE),SW_HIDE);
      ShowWindow(HListView_ref,SW_SHOW);
    }
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI Open_PCap_file(LPVOID lParam)
{
  //si un chargement en cours on l'"arrête et on relance
  /*if(chargement_pcap_file)
  {
    DWORD IDThread;
    chargement_pcap_file = -1;
    while(chargement_pcap_file!=0) _sleep(100);
  }  */

  //on masque le lstv
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),FALSE);
  CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV_FILTRE),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),SW_SHOW) ;
  ShowWindow(HListView_ref,SW_SHOW);

  //on affiche le lstv d'importation et on masque les filtres
  //on affiche la liste des machine + chek de disco
  /*CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_CHECKED);

  ShowWindow(HListView_ref,SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_SHOW);*/

  affichage_filtre = 0;

  chargement_pcap_file=1;

  char errbuf[PCAP_ERRBUF_SIZE];
  if((fp_file_open = Mypcap_open_offline(tmp_fic_pcap,errbuf))!=NULL)
  {
    //init
    NB_trame_buffer = 0;

    //init de la gestion des filtres
    Button_Filtre_CODE = 0;
    FiltreApplique = 0;

    //suppression des items de la listebox
    SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
    SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_INSERTSTRING,(WPARAM)-1,(LPARAM)"<<aucun filtre>>");

    //on vide les listes
    ListView_DeleteAllItems(GetDlgItem(Tabl[TAB_SNIFF],LSTV));
    ListView_DeleteAllItems(HListView_ref);

    struct pcap_pkthdr* packetHeader;
    const  u_char *     packetData;
    int res;

    while((res=MYpcap_next_ex(fp_file_open, &packetHeader, &packetData))>= 0 && chargement_pcap_file!=-1)
    {
      if(res == 0) continue;

      WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
      //pour chaque nouvelle trame (on ajoute une trame en mémoire pour traitement)
      Trame_buffer = (TRAME_BUFFER *) realloc(Trame_buffer,(NB_trame_buffer+1)*(sizeof(TRAME_BUFFER)+1));
      Trame_buffer[NB_trame_buffer].type = BOOL_TYPE_PCAP;
      Trame_buffer[NB_trame_buffer].src_port=0;
      Trame_buffer[NB_trame_buffer].dst_port=0;

      if (packetHeader->caplen<TAILLE_MAX_BUFFER_TRAME)
      {
        memcpy(Trame_buffer[NB_trame_buffer].buffer_header,packetHeader,sizeof(struct pcap_pkthdr));

        Trame_buffer[NB_trame_buffer].taille_buffer = packetHeader->caplen;
        memcpy(Trame_buffer[NB_trame_buffer].buffer,packetData,Trame_buffer[NB_trame_buffer].taille_buffer);
      }else //sinon on tronque le paquet
      {
        memcpy(Trame_buffer[NB_trame_buffer].buffer_header,packetHeader,sizeof(struct pcap_pkthdr));

        Trame_buffer[NB_trame_buffer].taille_buffer = TAILLE_MAX_BUFFER_TRAME;
        memcpy(Trame_buffer[NB_trame_buffer].buffer,packetData,TAILLE_MAX_BUFFER_TRAME);
      }
      ReleaseMutex(hMutex_TRAME_BUFFER);

      TraitementTramePCAP((unsigned char *)packetData,packetHeader->caplen,NB_trame_buffer++);
    }
    MYpcap_close(fp_file_open);
  }
  chargement_pcap_file=0;

  Tri(GetDlgItem(Tabl[TAB_SNIFF],LSTV),0,4);

  //on masque le lstv
  SendMessage(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),LB_SELITEMRANGE ,(WPARAM)TRUE,(LPARAM)0);
  CheckDlgButton(Tabl[TAB_SNIFF],SNIFF_CHK_DISCO,BST_UNCHECKED);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV_FILTRE),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),SW_HIDE);
  ShowWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),SW_SHOW) ;
  ShowWindow(HListView_ref,SW_SHOW);
  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],SNIFF_LB_FILTRE),TRUE);
  SetWindowText(GetDlgItem(Tabl[TAB_SNIFF],BT_LOAD),Language->Charger);
  return 0;
}
