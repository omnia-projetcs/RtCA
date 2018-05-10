//------------------------------------------------------------------------------
// Projet SCP-DB        : Scan et tests de réseau
// Auteur               : Hanteville Nicolas
// Site                 : http:\\omni.a.free.fr
// Version              : 0.1
// Date de modification : 06/01/2010
// Description          : traitement des formats de trames
// Environnement        : compatiblité DEVCPP / VISUAL C++ / BORLAND C++ 5.x
//------------------------------------------------------------------------------
#include "_ressources.h"
const    //liste des protocoles connues (137 / 255)
   char ProtocolesTypesNames2[138][20]={"0:IPv6 HOPOPT","1:ICMP","2:IGAP/IGMP/RGMP","3:GGP","4:IP in IP","5:ST","6:TCP","7:UCL/CBT","8:EGP","9:IGRP","10:BBN RCC",
   "11:NVP","12:PUP","13:ICMP","14:EMCON","15:XNET","16:Chaos","17:UDP","18:TMux","19:DCN","20:HMP","21:PRM","22:XEROX NS IDP","23:Trunk-1","24:Trunk-2","25:Leaf-1","26:Leaf-2",
   "27:RDP","28:IRTP","29: ISO TPC4","30:	NETBLT","31:MFE NSP","32:MERIT IP","33:DCCP","34:TPCP","35:IDPR IDPRP","36:XTP","37:DDP","38:IDPR CMTP","39:TP++ ","40:IL TP",
   "41:IPv6 over IPv4","42:SDRP","43:IPv6 RH","44:IPv6 FH","45:IDRP","46:RSVP","47:GRE","48:DSR","49:BNA","50:ESP","51:AH","52:I-NLSP","53:SWIPE","54:NARP","55:MEP","56:TLSP",
   "57:SKIP","58:ICMPv6/MLD","59:IPv6 NNH","60:IPv6 DO","61:AHIP","62:CFTP","63:ALN","64:SATNET/EXPAK","65:Kryptolan","66:MIT RVDP","67:IPPC","68:ADFS","69:SATNET",
   "70:VISA","71:IPCU","72:CPNE","73:CPHB","74:WSN","75:PVP","76:B SATNET M","77:SUN ND PT","78:WIDEBAND M","79:WIDEBAND EXPAK","80:ISO-IP","81:VMTP","82:SECURE-VMTP",
   "83:VINES","84:TTP","85:NSFNET-IGP","86:DGP","87:TCF","88:EIGRP","89:OSPF/MOSPF","90:SRPCP","91:LARP","92:MTP","93:AX.25","94:IP-IP EP","95:MICP","96:SCSP","97:EtherIP",
   "98:EH","99:APES","100:GMTP","101:IFMP","102:PNNI-IP","103:IPM","104:ARIS","105:SCPS","106:QNX","107:AN","108:IPPCP","109:SNP","110:CPP","111:IPX-IP","112:VRRP","113:PGM",
   "114:ANY 0-HOP P","115:L2TP","116:DDX DIIDE","117:IATP","118:ST","119:SRP","120:UTI","121:SMP","122:SM","123:PTP","124:ISIS-IPv4","125:FIRE","126:CRTP","127:CRUDP","128:SSCOPMCE",
   "129:IPLT","130:SPS","131:PIPE-IP","132:SCTP","133:FC","134:RSVP-E2E-IGNORE","135:Mobility Header","136:UDP-Lite","137:MPLS-IP"};
//------------------------------------------------------------------------------
//fonctions  de traitement
void TraitementAffichageRAW(char *titre, unsigned long int id, HANDLE HtreeView, HTREEITEM Htree)
{
  //on récupère la trame, on enlève l'entête
  //WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  IPV4_HDRM2 *trame;
  trame = (IPV4_HDRM2 *)Trame_buffer[id].buffer;

  //ajout du titre item courrant
  char tmp[MAX_PATH+1] = "Internet Protocol\0";
  TV_INSERTSTRUCT TvItem;
  TvItem.hInsertAfter = TVI_LAST;
  ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
  TvItem.item.mask  = TVIF_TEXT;
  TvItem.hParent = Htree;
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  //on continue avec le reste des informations
  _snprintf(tmp,MAX_PATH,"Version : %d",trame->ip_version);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Taille de l'entête : %d",trame->ip_header_len*trame->ip_version);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"DSF : 0x%02X",trame->ip_dsf);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Taille total : %d (0x%02X%02X)",trame->ip_total_length[0]*256 +trame->ip_total_length[1],trame->ip_total_length[0],trame->ip_total_length[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //ip_total_length[2]

  _snprintf(tmp,MAX_PATH,"Identification : %d (0x%02X%02X)",trame->ip_id[0]*256 +trame->ip_id[1],trame->ip_id[0],trame->ip_id[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Flags : %d (réservé), %d (aucun fragment), %d  (fragments à suivre)",(trame->ip_frag_et_offset[0]&128)/128,(trame->ip_frag_et_offset[0]&64)/64,(trame->ip_frag_et_offset[0]&32)/32);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Frag Offset : %d",trame->ip_frag_et_offset[1] + (trame->ip_frag_et_offset[0]&31)*255);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Time To Live : %d (0x%02X)",trame->ip_ttl,trame->ip_ttl);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Protocole : %d (0x%02X)",trame->ip_protocol,trame->ip_protocol);

  //ajout du protocole
  switch(trame->ip_protocol)
  {
    case IPPROTO_IP : strncat(tmp," IP\0",MAX_PATH);break;
    case IPPROTO_ICMP : strncat(tmp," ICMP\0",MAX_PATH);break;
    case IPPROTO_IGMP : strncat(tmp," IGMP\0",MAX_PATH);break;
    case IPPROTO_GGP : strncat(tmp," GGP\0",MAX_PATH);break;
    case IPPROTO_TCP : strncat(tmp," TCP\0",MAX_PATH);break;
    case IPPROTO_PUP : strncat(tmp," PUP\0",MAX_PATH);break;
    case IPPROTO_UDP : strncat(tmp," UDP\0",MAX_PATH);break;
    case IPPROTO_IDP : strncat(tmp," IDP\0",MAX_PATH);break;
    case IPPROTO_ND : strncat(tmp," ND\0",MAX_PATH);break;
    case IPPROTO_RAW : strncat(tmp," RAW\0",MAX_PATH);break;
    case IPPROTO_VRRP: strncat(tmp," VRRP\0",MAX_PATH);break;
    default : strncat(tmp," Inconnu\0",MAX_PATH);break;
  }
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //checksum
  _snprintf(tmp,MAX_PATH,"Header CHECKSUM : 0x%02X%02X",trame->ip_checksum[0],trame->ip_checksum[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //ip
  _snprintf(tmp,MAX_PATH,"IP source : %d.%d.%d.%d (0x%02X%02X%02X%02X)"
  ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]
  ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"IP destination : %d.%d.%d.%d (0x%02X%02X%02X%02X)"
  ,trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]
  ,trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);

  //traitement en fonction du type de protocole
  switch(trame->ip_protocol)
  {
    case IPPROTO_ICMP :
         {
           ICMP_HDRM2 *trame_ICMP;
           trame_ICMP = (ICMP_HDRM2 *)(Trame_buffer[id].buffer+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Internet Control Message Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

          _snprintf(tmp,MAX_PATH,"type : 0x%02X",trame_ICMP->type);

          switch(trame_ICMP->type)
          {
            case 0:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse à une demande d'écho\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 3:
                 if(trame_ICMP->code == 0) strncat(tmp," Réseau inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Hôte inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Protocole inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 3) strncat(tmp," Port inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 4) strncat(tmp," Fragmentation nécessaire mais interdite\0",MAX_PATH);
                 else if(trame_ICMP->code == 5) strncat(tmp," Echec de routage par la source\0",MAX_PATH);
                 else if(trame_ICMP->code == 6) strncat(tmp," Réseau de destination inconnu\0",MAX_PATH);
                 else if(trame_ICMP->code == 7) strncat(tmp," Hôte de destination inconnue\0",MAX_PATH);
                 else if(trame_ICMP->code == 8) strncat(tmp," Machine source isolée\0",MAX_PATH);
                 else if(trame_ICMP->code == 9) strncat(tmp," Réseau de destination interdit administrativement\0",MAX_PATH);
                 else if(trame_ICMP->code == 10) strncat(tmp," Hôte de destination interdite administrativement\0",MAX_PATH);
                 else if(trame_ICMP->code == 11) strncat(tmp," Réseau inaccessible pour ce type de service\0",MAX_PATH);
                 else if(trame_ICMP->code == 12) strncat(tmp," Hôte inaccessible pour ce type de service\0",MAX_PATH);
                 else if(trame_ICMP->code == 13) strncat(tmp," Communication interdite par un filtre\0",MAX_PATH);
                 else if(trame_ICMP->code == 14) strncat(tmp," Arrêt de continuité de la communication\0",MAX_PATH);
                 else if(trame_ICMP->code == 15) strncat(tmp," Redirection pour un réseau et pour un service donné\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 4:
                 if(trame_ICMP->code == 0) strncat(tmp," Volume de donnée trop importante\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 5:
                 if(trame_ICMP->code == 0) strncat(tmp," Redirection pour un hôte\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Redirection pour un hôte et pour un service donné\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Redirection pour un réseau\0",MAX_PATH);
                 else if(trame_ICMP->code == 3) strncat(tmp," Redirection pour un réseau et pour un service donné\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 8:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande d'écho\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 9:
                 if(trame_ICMP->code == 0) strncat(tmp," Avertissement routeur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 10:
                 if(trame_ICMP->code == 0) strncat(tmp," Sollicitation routeur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 11:
                 if(trame_ICMP->code == 0) strncat(tmp," Durée de vie écoulée avant d'arrivée à destination\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Temps limite de réassemblage du fragment dépassé\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 12:
                 if(trame_ICMP->code == 0) strncat(tmp," En-tête IP invalide\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Manque d'une option obligatoire\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Mauvaise longueur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 13:
                 if(trame_ICMP->code == 0) strncat(tmp," Requête pour un marqueur temporel \0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 14:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse pour un marqueur temporel\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 15:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande d'adresse réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 16:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse d'adresse réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 17:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande de masque de sous réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 18:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse de masque de sous réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            default : strncat(tmp," Code message inconnu\0",MAX_PATH);break;
          }
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->identifiant[0],trame_ICMP->identifiant[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Numéro de séquence : %d (0x%02X%02X)",trame_ICMP->numero_seq[0]*0x100+trame_ICMP->numero_seq[1],trame_ICMP->numero_seq[0],trame_ICMP->numero_seq[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          //données
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+5+2);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-28;

             for (i=0;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
         }
    break;
    case IPPROTO_TCP :
         {
           TCP_HDRM2 *trame_TCP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_TCP = (TCP_HDRM2 *)(Trame_buffer[id].buffer+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           if (!ResolutionPort(Trame_buffer[id].src_port,tmp2))tmp2[0]=0;
           if (!ResolutionPort(Trame_buffer[id].dst_port,tmp3))tmp3[0]=0;

           _snprintf(tmp,MAX_PATH,"Transmission Control Protocol [Ports : %d (%s) -> %d (%s)]"
           ,Trame_buffer[id].src_port,tmp2[0]!=0?tmp2:"-",Trame_buffer[id].dst_port,tmp3[0]!=0?tmp3:"-");

           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           //on ajoute maintenant la suite :)
           _snprintf(tmp,MAX_PATH,"Port source : %d (0x%02X%02X) %s"
           ,trame_TCP->source_port[0]*256+trame_TCP->source_port[1],trame_TCP->source_port[0],trame_TCP->source_port[1],tmp2[0]!=0?tmp2:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Port de destination : %d (0x%02X%02X) %s"
           ,trame_TCP->dest_port[0]*256+trame_TCP->dest_port[1],trame_TCP->dest_port[0],trame_TCP->dest_port[1],tmp3[0]!=0?tmp3:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Séquence : %u (0x%02X%02X%02X%02X)"
           ,trame_TCP->sequence[0]*0x1000000+trame_TCP->sequence[1]*0x10000+trame_TCP->sequence[2]*0x100+trame_TCP->sequence[3]
           ,trame_TCP->sequence[0],trame_TCP->sequence[1],trame_TCP->sequence[2],trame_TCP->sequence[3]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Acquittement : %u (0x%02X%02X%02X%02X)"
           ,trame_TCP->acknowledge[0]*0x1000000+trame_TCP->acknowledge[1]*0x10000+trame_TCP->acknowledge[2]*0x100+trame_TCP->acknowledge[3]
           ,trame_TCP->acknowledge[0],trame_TCP->acknowledge[1],trame_TCP->acknowledge[2],trame_TCP->acknowledge[3]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Taille de l'entête : %d",(trame_TCP->header_length_sup/0x10)*trame->ip_version);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           strcpy(tmp,"Flags :\0");
           if ((trame_TCP->flags&128)/128)strcat(tmp," CWR\0");
           if ((trame_TCP->flags&64)/64)strcat(tmp," ECN\0");
           if ((trame_TCP->flags&32)/32)strcat(tmp," URG\0");
           if ((trame_TCP->flags&16)/16)strcat(tmp," ACK\0");
           if ((trame_TCP->flags&8)/8)strcat(tmp," PSH\0");
           if ((trame_TCP->flags&4)/4)strcat(tmp," RST\0");
           if ((trame_TCP->flags&2)/2)strcat(tmp," SYN\0");
           if ((trame_TCP->flags&1)/1)strcat(tmp," FIN\0");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Windows : %d (0x%02X%02X)",trame_TCP->window[0]*0x100+trame_TCP->window[1],trame_TCP->window[0],trame_TCP->window[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_TCP->checksum[0],trame_TCP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Urgent : 0x%02X%02X",trame_TCP->urgent_pointer[0],trame_TCP->urgent_pointer[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           //travail maintenant en fonction du protocole identifié ^^
           //DATA (traitement de l'affichage par ligne en cas de reconnaissance d'un '\r ou \n)
           //suivant les protocoles
           //if (Trame_buffer[id].protocoleChoix!=CODE_OTHER)//!non reconnu
           {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+10);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-40;

             for (i=0;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
         }
    break;
    case IPPROTO_UDP :
         {
           UDP_HDRM2 *trame_UDP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_UDP = (UDP_HDRM2 *)(Trame_buffer[id].buffer+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           if (!ResolutionPort(Trame_buffer[id].src_port,tmp2))tmp2[0]=0;
           if (!ResolutionPort(Trame_buffer[id].dst_port,tmp3))tmp3[0]=0;

           _snprintf(tmp,MAX_PATH,"User Datagram Protocol [Ports : %d (%s) -> %d (%s)]"
           ,Trame_buffer[id].src_port,tmp2[0]!=0?tmp2:"-",Trame_buffer[id].dst_port,tmp3[0]!=0?tmp3:"-");

           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           //on ajoute maintenant la suite :)
           _snprintf(tmp,MAX_PATH,"Port source : %d (0x%02X%02X) %s"
           ,trame_UDP->source_port[0]*256+trame_UDP->source_port[1],trame_UDP->source_port[0],trame_UDP->source_port[1],tmp2[0]!=0?tmp2:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Port de destination : %d (0x%02X%02X) %s"
           ,trame_UDP->dest_port[0]*256+trame_UDP->dest_port[1],trame_UDP->dest_port[0],trame_UDP->dest_port[1],tmp3[0]!=0?tmp3:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Taille : %d (0x%02X%02X)",trame_UDP->length[0]*0x100+trame_UDP->length[1],trame_UDP->length[0],trame_UDP->length[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_UDP->checksum[0],trame_UDP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);


           //traitement des protocoles connus
           switch(Trame_buffer[id].protocoleChoix)
           {
             case CODE_DHCP :
                  {
                     TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
                     TvItem.hParent = TVI_ROOT;
                     strcpy(tmp,"DHCP : Bootstrap Protocol");
                     TvItem.item.pszText = tmp;
                     Htree = TreeView_InsertItem(HtreeView, &TvItem);
                     TvItem.hParent = Htree;
                     HTREEITEM Htree_tmp = Htree;

                     DHCP_HDRM *trame_DHCP;
                     trame_DHCP = (DHCP_HDRM *)(Trame_buffer[id].buffer+5+2);

                     switch(trame_DHCP->Type)
                     {
                       case 1: strcpy(tmp,"Type de message : (1) Boot Request");break;
                       default : _snprintf(tmp,MAX_PATH,"Type de message : (%d) Inconnu",trame_DHCP->Type);
                     }
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     switch(trame_DHCP->Hard_Type)
                     {
                       case 1: strcpy(tmp,"Hardware type : (1) Ethernet");break;
                       default : _snprintf(tmp,MAX_PATH,"Hardware type : (%d) Inconnu",trame_DHCP->Hard_Type);
                     }
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Taille de l'adresse physique : %d octet(s)",trame_DHCP->Hard_Length);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Nombre de saut : %d",trame_DHCP->Hop);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Numéro de transaction : 0x%02X%02X%02X%02X"
                     ,trame_DHCP->ID[0],trame_DHCP->ID[1],trame_DHCP->ID[2],trame_DHCP->ID[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Temps écoulé : %d secondes (0x%02X%02X)",trame_DHCP->time[1]*256+trame_DHCP->time[0],trame_DHCP->time[0],trame_DHCP->time[1]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"BOOTP flags : 0x%02X%02X",trame_DHCP->flags[0],trame_DHCP->flags[1]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"IP du client : %d.%d.%d.%d"
                     ,trame_DHCP->IP_client[0],trame_DHCP->IP_client[1],trame_DHCP->IP_client[2],trame_DHCP->IP_client[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"Your IP : %d.%d.%d.%d"
                     ,trame_DHCP->IP_your[0],trame_DHCP->IP_your[1],trame_DHCP->IP_your[2],trame_DHCP->IP_your[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"IP du serveur : %d.%d.%d.%d"
                     ,trame_DHCP->IP_serveur[0],trame_DHCP->IP_serveur[1],trame_DHCP->IP_serveur[2],trame_DHCP->IP_serveur[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"IP due la passerelle : %d.%d.%d.%d"
                     ,trame_DHCP->IP_passerelle[0],trame_DHCP->IP_passerelle[1],trame_DHCP->IP_passerelle[2],trame_DHCP->IP_passerelle[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     if (trame_DHCP->Hard_Length ==6 ) //par défaut
                     {
                       _snprintf(tmp,MAX_PATH,"Adresse MAC du client : %02X:%02X:%02X:%02X:%02X:%02X"
                       ,trame_DHCP->MAC_client[0],trame_DHCP->MAC_client[1]
                       ,trame_DHCP->MAC_client[2],trame_DHCP->MAC_client[3]
                       ,trame_DHCP->MAC_client[4],trame_DHCP->MAC_client[5]);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       //padding
                       _snprintf(tmp,MAX_PATH,"Adresse MAC Padding : %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
                       ,trame_DHCP->MAC_padding[0],trame_DHCP->MAC_padding[1]
                       ,trame_DHCP->MAC_padding[2],trame_DHCP->MAC_padding[3]
                       ,trame_DHCP->MAC_padding[4],trame_DHCP->MAC_padding[5]
                       ,trame_DHCP->MAC_padding[6],trame_DHCP->MAC_padding[7]
                       ,trame_DHCP->MAC_padding[8],trame_DHCP->MAC_padding[9]
                       );
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                     }else
                     {
                       char tmp2[4];
                       unsigned int j;
                       if (trame_DHCP->Hard_Length>0)
                       {
                         if (trame_DHCP->Hard_Length<6)
                         {
                           strcpy(tmp,"Adresse MAC du client : ");
                           for (j=0;j<trame_DHCP->Hard_Length;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_client[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);

                           strcpy(tmp,"Adresse MAC Padding : ");
                           for (j=trame_DHCP->Hard_Length;j<6;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_client[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           for (j=0;j<10;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);
                         }else //si >6
                         {
                           _snprintf(tmp,MAX_PATH,"Adresse MAC du client : %02X:%02X:%02X:%02X:%02X:%02X:"
                           ,trame_DHCP->MAC_client[0],trame_DHCP->MAC_client[1]
                           ,trame_DHCP->MAC_client[2],trame_DHCP->MAC_client[3]
                           ,trame_DHCP->MAC_client[4],trame_DHCP->MAC_client[5]);

                           for (j=0;j<(trame_DHCP->Hard_Length-6);j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);

                           strcpy(tmp,"Adresse MAC Padding : ");
                           for (j=(trame_DHCP->Hard_Length-6);j<10;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);
                         }
                       }
                     }

                     _snprintf(tmp,MAX_PATH,"Nom du serveur hôte : %s",trame_DHCP->Serveur_Host);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Nom du fichier de configuration : %s",trame_DHCP->Boot_File_Name);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Magic cookie : 0x%02X%02X%02X%02X"
                     ,trame_DHCP->Magic_cookie[0],trame_DHCP->Magic_cookie[1],trame_DHCP->Magic_cookie[2],trame_DHCP->Magic_cookie[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     //gestion des options
                     strcpy(tmp,"Options : ");
                     TvItem.item.pszText = tmp;
                     Htree = TreeView_InsertItem(HtreeView, &TvItem);
                     TvItem.hParent = Htree;

                     unsigned short j=0;
                     while (trame_DHCP->Options[j] != 0xFF && j<64)
                     {
                       //lecture de l'option
                       switch(trame_DHCP->Options[j])
                       {
                         case 0x0C :strcpy(tmp,"Option : (0x0C) Host Name");break;
                         case 0x2B :strcpy(tmp,"Option : (0x2B) Vendor-Specific Information");break;
                         case 0x35 :strcpy(tmp,"Option : (0x35) DHCP Message Type");break;
                         case 0x37 :strcpy(tmp,"Option : (0x37) Parameter Request List");break;
                         case 0x3C :strcpy(tmp,"Option : (0x3C) Vendor class identifier");break;
                         case 0x3D :strcpy(tmp,"Option : (0x3D) Client identifier (type + Adresse MAC)");break;
                         case 0x74 :strcpy(tmp,"Option : (0x74) DHCP Auto-Configuration");break;
                         default :_snprintf(tmp,MAX_PATH,"Option : (0x%02X) Inconnu",trame_DHCP->Options[j]);break;
                       }
                       TvItem.item.pszText = tmp;
                       TvItem.hParent = Htree;
                       TvItem.hParent = TreeView_InsertItem(HtreeView, &TvItem);

                       _snprintf(tmp,MAX_PATH,"Taille des données : %d",trame_DHCP->Options[j+1]);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       strcpy(tmp,"Valeur : 0x");
                       unsigned int k=0;
                       char tmp2[3];
                       for (k=0; k<trame_DHCP->Options[j+1]; k++)
                       {
                         _snprintf(tmp2,3,"%02X",trame_DHCP->Options[j+2+k]);
                         strncat(tmp,tmp2,MAX_PATH);
                       }

                       char tmp3[256];
                       strncpy(tmp3,&(trame_DHCP->Options[j+2]),trame_DHCP->Options[j+1]);
                       tmp3[trame_DHCP->Options[j+1]]=0;

                       strncat(tmp," (",MAX_PATH);
                       strncat(tmp,tmp3,MAX_PATH);
                       strncat(tmp,")\0",MAX_PATH);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                       j+=2+trame_DHCP->Options[j+1];
                     }
                     //gestion des options
                     if (j<64)
                     {
                       _snprintf(tmp,MAX_PATH,"Fin des options : 0x%02X",trame_DHCP->Options[j]);
                       TvItem.hParent = Htree;
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       strcpy(tmp,"Padding : ....");
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                     }
                     TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
                     TreeView_Expand(HtreeView,Htree_tmp,TVE_EXPAND);
                  }
             break;
             default:
             //affichage des données
             {
               TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
               TvItem.hParent = TVI_ROOT;
               strcpy(tmp,"Données");
               TvItem.item.pszText = tmp;
               Htree = TreeView_InsertItem(HtreeView, &TvItem);
               TvItem.hParent = Htree;

               //traitement
               char *buffer_donnees;
               buffer_donnees = (char *)(Trame_buffer[id].buffer+5+2);

               //traitement par 16 + 16 carcatères
               char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
               unsigned short count=0,i;
               unsigned int taille = Trame_buffer[id].taille_buffer-28;

               for (i=0;i<taille;i++)
               {
                 if (count == 16)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                   _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                   ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                   ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                   ,tmp3);
                   TvItem.item.pszText = tmp;
                   TreeView_InsertItem(HtreeView, &TvItem);
                   count=0;
                 }

                 tmp2[count]=buffer_donnees[i];
                 if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
                 else tmp3[count]='.';

                 count++;
               }
               if (count>0)
               {
                 for (;count<32;count++)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                 }
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
               }
             }
           }
         }
    break;
    case IPPROTO_IGMP :
         {
           IGMP_HDRM2 *trame_IGMP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_IGMP = (IGMP_HDRM2 *)(Trame_buffer[id].buffer+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Internet Group Management Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           _snprintf(tmp,MAX_PATH,"Type : 0x%02X%",trame_IGMP->type);
           if (trame_IGMP->type == 11)strncat(tmp," Requête pour identifier les groupes ayant des membres actifs.",MAX_PATH);
           else if (trame_IGMP->type == 12)strncat(tmp," Rapport d'appartenance au groupe émis par un membre actif du groupe (IGMP version 1)",MAX_PATH);
           else if (trame_IGMP->type == 16)strncat(tmp," Rapport d'appartenance au groupe émis par un membre actif du groupe (IGMP version 2)",MAX_PATH);
           else if (trame_IGMP->type == 17)strncat(tmp," Un membre annonce son départ du groupe.",MAX_PATH);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Temps Maximum de réponse : %3.1f secondes (0x%02X%)",trame_IGMP->Temp_reponse,trame_IGMP->Temp_reponse/10);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_IGMP->checksum[0],trame_IGMP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Adresse de Multicast : %d.%d.%d.%d (0x%02X%02X%02X%02X)"
          ,trame_IGMP->ip_destaddr[0],trame_IGMP->ip_destaddr[1],trame_IGMP->ip_destaddr[2],trame_IGMP->ip_destaddr[3]
          ,trame_IGMP->ip_destaddr[0],trame_IGMP->ip_destaddr[1],trame_IGMP->ip_destaddr[2],trame_IGMP->ip_destaddr[3]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          //données
          {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+5+2);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-28;

             for (i=0;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
         }
    break;
    default :
            //affichage standard + toutes les données en hexa
           {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+5);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-20;

             for (i=0;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
    break;
  }
  //ReleaseMutex(hMutex_TRAME_BUFFER);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
}
//------------------------------------------------------------------------------
//fonctions  de traitement
void TraitementAffichageIPV4(char *titre,/* IPV4_HDR *trame, unsigned short taille_trame,*/ unsigned long int id, HANDLE HtreeView, HTREEITEM Htree)
{
  //on récupère la trame, on enlève l'entête
  IPV4_HDRM *trame;

  //ajout du titre item courrant
  char tmp[MAX_PATH+1] = "Internet Protocol Version 4\0";
  TV_INSERTSTRUCT TvItem;
  TvItem.hInsertAfter = TVI_LAST;
  ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
  TvItem.item.mask  = TVIF_TEXT;
  TvItem.hParent = Htree;
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  //WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  trame = (IPV4_HDRM *)(Trame_buffer[id].buffer+3);

  //on continue avec le reste des informations
  _snprintf(tmp,MAX_PATH,"Version : %d",trame->ip_version);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Taille de l'entête : %d",trame->ip_header_len*trame->ip_version);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"DSF : 0x%02X",trame->ip_dsf);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Taille total : %d (0x%02X%02X)",trame->ip_total_length[0]*256 +trame->ip_total_length[1],trame->ip_total_length[0],trame->ip_total_length[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //ip_total_length[2]

  _snprintf(tmp,MAX_PATH,"Identification : %d (0x%02X%02X)",trame->ip_id[0]*256 +trame->ip_id[1],trame->ip_id[0],trame->ip_id[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Flags : %d (réservé), %d (aucun fragment), %d  (fragments à suivre)",(trame->ip_frag_et_offset[0]&128)/128,(trame->ip_frag_et_offset[0]&64)/64,(trame->ip_frag_et_offset[0]&32)/32);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Frag Offset : %d",trame->ip_frag_et_offset[1] + (trame->ip_frag_et_offset[0]&31)*255);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Time To Live : %d (0x%02X)",trame->ip_ttl,trame->ip_ttl);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Protocole : %d (0x%02X)",trame->ip_protocol,trame->ip_protocol);

  //ajout du protocole
  switch(trame->ip_protocol)
  {
    case IPPROTO_IP : strncat(tmp," IP\0",MAX_PATH);break;
    case IPPROTO_ICMP : strncat(tmp," ICMP\0",MAX_PATH);break;
    case IPPROTO_IGMP : strncat(tmp," IGMP\0",MAX_PATH);break;
    case IPPROTO_GGP : strncat(tmp," GGP\0",MAX_PATH);break;
    case IPPROTO_TCP : strncat(tmp," TCP\0",MAX_PATH);break;
    case IPPROTO_PUP : strncat(tmp," PUP\0",MAX_PATH);break;
    case IPPROTO_UDP : strncat(tmp," UDP\0",MAX_PATH);break;
    case IPPROTO_IDP : strncat(tmp," IDP\0",MAX_PATH);break;
    case IPPROTO_ND : strncat(tmp," ND\0",MAX_PATH);break;
    case IPPROTO_RAW : strncat(tmp," RAW\0",MAX_PATH);break;
    case IPPROTO_VRRP: strncat(tmp," VRRP\0",MAX_PATH);break;
    case 0x59: strncat(tmp," OSPF IGP\0",MAX_PATH);break;
    default : strncat(tmp," Inconnu\0",MAX_PATH);break;
  }
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //checksum
  _snprintf(tmp,MAX_PATH,"Header CHECKSUM : 0x%02X%02X",trame->ip_checksum[0],trame->ip_checksum[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //ip
  _snprintf(tmp,MAX_PATH,"IP source : %d.%d.%d.%d (0x%02X%02X%02X%02X)"
  ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]
  ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"IP destination : %d.%d.%d.%d (0x%02X%02X%02X%02X)"
  ,trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]
  ,trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);

  //traitement en fonction du type de protocole
  switch(trame->ip_protocol)
  {
    case IPPROTO_ICMP :
         {
           ICMP_HDRM *trame_ICMP;
           trame_ICMP = (ICMP_HDRM *)(Trame_buffer[id].buffer+3+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Internet Control Message Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

          _snprintf(tmp,MAX_PATH,"type : 0x%02X",trame_ICMP->type);

          switch(trame_ICMP->type)
          {
            case 0:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse à une demande d'écho\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 3:
                 if(trame_ICMP->code == 0) strncat(tmp," Réseau inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Hôte inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Protocole inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 3) strncat(tmp," Port inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 4) strncat(tmp," Fragmentation nécessaire mais interdite\0",MAX_PATH);
                 else if(trame_ICMP->code == 5) strncat(tmp," Echec de routage par la source\0",MAX_PATH);
                 else if(trame_ICMP->code == 6) strncat(tmp," Réseau de destination inconnu\0",MAX_PATH);
                 else if(trame_ICMP->code == 7) strncat(tmp," Hôte de destination inconnue\0",MAX_PATH);
                 else if(trame_ICMP->code == 8) strncat(tmp," Machine source isolée\0",MAX_PATH);
                 else if(trame_ICMP->code == 9) strncat(tmp," Réseau de destination interdit administrativement\0",MAX_PATH);
                 else if(trame_ICMP->code == 10) strncat(tmp," Hôte de destination interdite administrativement\0",MAX_PATH);
                 else if(trame_ICMP->code == 11) strncat(tmp," Réseau inaccessible pour ce type de service\0",MAX_PATH);
                 else if(trame_ICMP->code == 12) strncat(tmp," Hôte inaccessible pour ce type de service\0",MAX_PATH);
                 else if(trame_ICMP->code == 13) strncat(tmp," Communication interdite par un filtre\0",MAX_PATH);
                 else if(trame_ICMP->code == 14) strncat(tmp," Arrêt de continuité de la communication\0",MAX_PATH);
                 else if(trame_ICMP->code == 15) strncat(tmp," Redirection pour un réseau et pour un service donné\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 4:
                 if(trame_ICMP->code == 0) strncat(tmp," Volume de donnée trop importante\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 5:
                 if(trame_ICMP->code == 0) strncat(tmp," Redirection pour un hôte\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Redirection pour un hôte et pour un service donné\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Redirection pour un réseau\0",MAX_PATH);
                 else if(trame_ICMP->code == 3) strncat(tmp," Redirection pour un réseau et pour un service donné\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 8:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande d'écho\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 9:
                 if(trame_ICMP->code == 0) strncat(tmp," Avertissement routeur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 10:
                 if(trame_ICMP->code == 0) strncat(tmp," Sollicitation routeur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 11:
                 if(trame_ICMP->code == 0) strncat(tmp," Durée de vie écoulée avant d'arrivée à destination\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Temps limite de réassemblage du fragment dépassé\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 12:
                 if(trame_ICMP->code == 0) strncat(tmp," En-tête IP invalide\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Manque d'une option obligatoire\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Mauvaise longueur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 13:
                 if(trame_ICMP->code == 0) strncat(tmp," Requête pour un marqueur temporel \0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 14:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse pour un marqueur temporel\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 15:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande d'adresse réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 16:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse d'adresse réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 17:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande de masque de sous réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 18:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse de masque de sous réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            default : strncat(tmp," Code message inconnu\0",MAX_PATH);break;
          }
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->identifiant[0],trame_ICMP->identifiant[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Numéro de séquence : %d (0x%02X%02X)",trame_ICMP->numero_seq[0]*0x100+trame_ICMP->numero_seq[1],trame_ICMP->numero_seq[0],trame_ICMP->numero_seq[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          //données
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+2);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-40;

             for (i=2;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }

         }
    break;
    case IPPROTO_TCP :
         {
           TCP_HDRM *trame_TCP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_TCP = (TCP_HDRM *)(Trame_buffer[id].buffer+3+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           if (!ResolutionPort(Trame_buffer[id].src_port,tmp2))tmp2[0]=0;
           if (!ResolutionPort(Trame_buffer[id].dst_port,tmp3))tmp3[0]=0;

           _snprintf(tmp,MAX_PATH,"Transmission Control Protocol [Ports : %d (%s) -> %d (%s)]"
           ,Trame_buffer[id].src_port,tmp2[0]!=0?tmp2:"-",Trame_buffer[id].dst_port,tmp3[0]!=0?tmp3:"-");

           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           //on ajoute maintenant la suite :)
           _snprintf(tmp,MAX_PATH,"Port source : %d (0x%02X%02X) %s"
           ,trame_TCP->source_port[0]*256+trame_TCP->source_port[1],trame_TCP->source_port[0],trame_TCP->source_port[1],tmp2[0]!=0?tmp2:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Port de destination : %d (0x%02X%02X) %s"
           ,trame_TCP->dest_port[0]*256+trame_TCP->dest_port[1],trame_TCP->dest_port[0],trame_TCP->dest_port[1],tmp3[0]!=0?tmp3:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Séquence : %u (0x%02X%02X%02X%02X)"
           ,trame_TCP->sequence[0]*0x1000000+trame_TCP->sequence[1]*0x10000+trame_TCP->sequence[2]*0x100+trame_TCP->sequence[3]
           ,trame_TCP->sequence[0],trame_TCP->sequence[1],trame_TCP->sequence[2],trame_TCP->sequence[3]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Acquittement : %u (0x%02X%02X%02X%02X)"
           ,trame_TCP->acknowledge[0]*0x1000000+trame_TCP->acknowledge[1]*0x10000+trame_TCP->acknowledge[2]*0x100+trame_TCP->acknowledge[3]
           ,trame_TCP->acknowledge[0],trame_TCP->acknowledge[1],trame_TCP->acknowledge[2],trame_TCP->acknowledge[3]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Taille de l'entête : %d",(trame_TCP->header_length_sup/0x10)*trame->ip_version);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           strcpy(tmp,"Flags :\0");
           if ((trame_TCP->flags&128)/128)strcat(tmp," CWR\0");
           if ((trame_TCP->flags&64)/64)strcat(tmp," ECN\0");
           if ((trame_TCP->flags&32)/32)strcat(tmp," URG\0");
           if ((trame_TCP->flags&16)/16)strcat(tmp," ACK\0");
           if ((trame_TCP->flags&8)/8)strcat(tmp," PSH\0");
           if ((trame_TCP->flags&4)/4)strcat(tmp," RST\0");
           if ((trame_TCP->flags&2)/2)strcat(tmp," SYN\0");
           if ((trame_TCP->flags&1)/1)strcat(tmp," FIN\0");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Windows : %d (0x%02X%02X)",trame_TCP->window[0]*0x100+trame_TCP->window[1],trame_TCP->window[0],trame_TCP->window[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_TCP->checksum[0],trame_TCP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Urgent : 0x%02X%02X",trame_TCP->urgent_pointer[0],trame_TCP->urgent_pointer[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           //travail maintenant en fonction du protocole identifié ^^
           //DATA (traitement de l'affichage par ligne en cas de reconnaissance d'un '\r ou \n)
           //suivant les protocoles
           //if (Trame_buffer[id].protocoleChoix!=CODE_OTHER)//!non reconnu
           {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+4);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-48;

             for (i=6;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
         }
    break;

    case IPPROTO_UDP :
         {
           UDP_HDRM *trame_UDP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_UDP = (UDP_HDRM *)(Trame_buffer[id].buffer+3+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           if (!ResolutionPort(Trame_buffer[id].src_port,tmp2))tmp2[0]=0;
           if (!ResolutionPort(Trame_buffer[id].dst_port,tmp3))tmp3[0]=0;

           _snprintf(tmp,MAX_PATH,"User Datagram Protocol [Ports : %d (%s) -> %d (%s)]"
           ,Trame_buffer[id].src_port,tmp2[0]!=0?tmp2:"-",Trame_buffer[id].dst_port,tmp3[0]!=0?tmp3:"-");

           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           //on ajoute maintenant la suite :)
           _snprintf(tmp,MAX_PATH,"Port source : %d (0x%02X%02X) %s"
           ,trame_UDP->source_port[0]*256+trame_UDP->source_port[1],trame_UDP->source_port[0],trame_UDP->source_port[1],tmp2[0]!=0?tmp2:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Port de destination : %d (0x%02X%02X) %s"
           ,trame_UDP->dest_port[0]*256+trame_UDP->dest_port[1],trame_UDP->dest_port[0],trame_UDP->dest_port[1],tmp3[0]!=0?tmp3:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Taille : %d (0x%02X%02X)",trame_UDP->length[0]*0x100+trame_UDP->length[1],trame_UDP->length[0],trame_UDP->length[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_UDP->checksum[0],trame_UDP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);


           //traitement des protocoles connus

           /*
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
           */

           switch(Trame_buffer[id].protocoleChoix)
           {
             case CODE_DHCP :
                  {
                     TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
                     TvItem.hParent = TVI_ROOT;
                     strcpy(tmp,"DHCP : Bootstrap Protocol");
                     TvItem.item.pszText = tmp;
                     Htree = TreeView_InsertItem(HtreeView, &TvItem);
                     TvItem.hParent = Htree;
                     HTREEITEM Htree_tmp = Htree;

                     DHCP_HDRM *trame_DHCP;
                     trame_DHCP = (DHCP_HDRM *)(Trame_buffer[id].buffer+3+5+2);

                     switch(trame_DHCP->Type)
                     {
                       case 1: strcpy(tmp,"Type de message : (1) Boot Request");break;
                       default : _snprintf(tmp,MAX_PATH,"Type de message : (%d) Inconnu",trame_DHCP->Type);
                     }
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     switch(trame_DHCP->Hard_Type)
                     {
                       case 1: strcpy(tmp,"Hardware type : (1) Ethernet");break;
                       default : _snprintf(tmp,MAX_PATH,"Hardware type : (%d) Inconnu",trame_DHCP->Hard_Type);
                     }
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Taille de l'adresse physique : %d octet(s)",trame_DHCP->Hard_Length);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Nombre de saut : %d",trame_DHCP->Hop);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Numéro de transaction : 0x%02X%02X%02X%02X"
                     ,trame_DHCP->ID[0],trame_DHCP->ID[1],trame_DHCP->ID[2],trame_DHCP->ID[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Temps écoulé : %d secondes (0x%02X%02X)",trame_DHCP->time[1]*256+trame_DHCP->time[0],trame_DHCP->time[0],trame_DHCP->time[1]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"BOOTP flags : 0x%02X%02X",trame_DHCP->flags[0],trame_DHCP->flags[1]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"IP du client : %d.%d.%d.%d"
                     ,trame_DHCP->IP_client[0],trame_DHCP->IP_client[1],trame_DHCP->IP_client[2],trame_DHCP->IP_client[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"Your IP : %d.%d.%d.%d"
                     ,trame_DHCP->IP_your[0],trame_DHCP->IP_your[1],trame_DHCP->IP_your[2],trame_DHCP->IP_your[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"IP du serveur : %d.%d.%d.%d"
                     ,trame_DHCP->IP_serveur[0],trame_DHCP->IP_serveur[1],trame_DHCP->IP_serveur[2],trame_DHCP->IP_serveur[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"IP due la passerelle : %d.%d.%d.%d"
                     ,trame_DHCP->IP_passerelle[0],trame_DHCP->IP_passerelle[1],trame_DHCP->IP_passerelle[2],trame_DHCP->IP_passerelle[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     if (trame_DHCP->Hard_Length ==6 ) //par défaut
                     {
                       _snprintf(tmp,MAX_PATH,"Adresse MAC du client : %02X:%02X:%02X:%02X:%02X:%02X"
                       ,trame_DHCP->MAC_client[0],trame_DHCP->MAC_client[1]
                       ,trame_DHCP->MAC_client[2],trame_DHCP->MAC_client[3]
                       ,trame_DHCP->MAC_client[4],trame_DHCP->MAC_client[5]);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       //padding
                       _snprintf(tmp,MAX_PATH,"Adresse MAC Padding : %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
                       ,trame_DHCP->MAC_padding[0],trame_DHCP->MAC_padding[1]
                       ,trame_DHCP->MAC_padding[2],trame_DHCP->MAC_padding[3]
                       ,trame_DHCP->MAC_padding[4],trame_DHCP->MAC_padding[5]
                       ,trame_DHCP->MAC_padding[6],trame_DHCP->MAC_padding[7]
                       ,trame_DHCP->MAC_padding[8],trame_DHCP->MAC_padding[9]
                       );
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                     }else
                     {
                       char tmp2[4];
                       unsigned int j;
                       if (trame_DHCP->Hard_Length>0)
                       {
                         if (trame_DHCP->Hard_Length<6)
                         {
                           strcpy(tmp,"Adresse MAC du client : ");
                           for (j=0;j<trame_DHCP->Hard_Length;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_client[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);

                           strcpy(tmp,"Adresse MAC Padding : ");
                           for (j=trame_DHCP->Hard_Length;j<6;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_client[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           for (j=0;j<10;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);
                         }else //si >6
                         {
                           _snprintf(tmp,MAX_PATH,"Adresse MAC du client : %02X:%02X:%02X:%02X:%02X:%02X:"
                           ,trame_DHCP->MAC_client[0],trame_DHCP->MAC_client[1]
                           ,trame_DHCP->MAC_client[2],trame_DHCP->MAC_client[3]
                           ,trame_DHCP->MAC_client[4],trame_DHCP->MAC_client[5]);

                           for (j=0;j<(trame_DHCP->Hard_Length-6);j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);

                           strcpy(tmp,"Adresse MAC Padding : ");
                           for (j=(trame_DHCP->Hard_Length-6);j<10;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);
                         }
                       }
                     }

                     _snprintf(tmp,MAX_PATH,"Nom du serveur hôte : %s",trame_DHCP->Serveur_Host);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Nom du fichier de configuration : %s",trame_DHCP->Boot_File_Name);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Magic cookie : 0x%02X%02X%02X%02X"
                     ,trame_DHCP->Magic_cookie[0],trame_DHCP->Magic_cookie[1],trame_DHCP->Magic_cookie[2],trame_DHCP->Magic_cookie[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     //gestion des options
                     strcpy(tmp,"Options : ");
                     TvItem.item.pszText = tmp;
                     Htree = TreeView_InsertItem(HtreeView, &TvItem);
                     TvItem.hParent = Htree;

                     unsigned short j=0;
                     while (trame_DHCP->Options[j] != 0xFF && j<64)
                     {
                       //lecture de l'option
                       switch(trame_DHCP->Options[j])
                       {
                         case 0x0C :strcpy(tmp,"Option : (0x0C) Host Name");break;
                         case 0x2B :strcpy(tmp,"Option : (0x2B) Vendor-Specific Information");break;
                         case 0x35 :strcpy(tmp,"Option : (0x35) DHCP Message Type");break;
                         case 0x37 :strcpy(tmp,"Option : (0x37) Parameter Request List");break;
                         case 0x3C :strcpy(tmp,"Option : (0x3C) Vendor class identifier");break;
                         case 0x3D :strcpy(tmp,"Option : (0x3D) Client identifier (type + Adresse MAC)");break;
                         case 0x74 :strcpy(tmp,"Option : (0x74) DHCP Auto-Configuration");break;
                         default :_snprintf(tmp,MAX_PATH,"Option : (0x%02X) Inconnu",trame_DHCP->Options[j]);break;
                       }
                       TvItem.item.pszText = tmp;
                       TvItem.hParent = Htree;
                       TvItem.hParent = TreeView_InsertItem(HtreeView, &TvItem);

                       _snprintf(tmp,MAX_PATH,"Taille des données : %d",trame_DHCP->Options[j+1]);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       strcpy(tmp,"Valeur : 0x");
                       unsigned int k=0;
                       char tmp2[3];
                       for (k=0; k<trame_DHCP->Options[j+1]; k++)
                       {
                         _snprintf(tmp2,3,"%02X",trame_DHCP->Options[j+2+k]);
                         strncat(tmp,tmp2,MAX_PATH);
                       }

                       char tmp3[256];
                       strncpy(tmp3,&(trame_DHCP->Options[j+2]),trame_DHCP->Options[j+1]);
                       tmp3[trame_DHCP->Options[j+1]]=0;

                       strncat(tmp," (",MAX_PATH);
                       strncat(tmp,tmp3,MAX_PATH);
                       strncat(tmp,")\0",MAX_PATH);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                       j+=2+trame_DHCP->Options[j+1];
                     }
                     //gestion des options
                     if (j<64)
                     {
                       _snprintf(tmp,MAX_PATH,"Fin des options : 0x%02X",trame_DHCP->Options[j]);
                       TvItem.hParent = Htree;
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       strcpy(tmp,"Padding : ....");
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                     }
                     TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
                     TreeView_Expand(HtreeView,Htree_tmp,TVE_EXPAND);
                  }
             break;
             default:
             //affichage des données
             {
               TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
               TvItem.hParent = TVI_ROOT;
               strcpy(tmp,"Données");
               TvItem.item.pszText = tmp;
               Htree = TreeView_InsertItem(HtreeView, &TvItem);
               TvItem.hParent = Htree;

               //traitement
               char *buffer_donnees;
               buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+1);

               //traitement par 16 + 16 carcatères
               char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
               unsigned short count=0,i;
               unsigned int taille = Trame_buffer[id].taille_buffer-36;

               for (i=6;i<taille;i++)
               {
                 if (count == 16)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                   _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                   ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                   ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                   ,tmp3);
                   TvItem.item.pszText = tmp;
                   TreeView_InsertItem(HtreeView, &TvItem);
                   count=0;
                 }

                 tmp2[count]=buffer_donnees[i];
                 if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
                 else tmp3[count]='.';

                 count++;
               }
               if (count>0)
               {
                 for (;count<32;count++)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                 }
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
               }
             }
           }
         }
    break;
    case IPPROTO_IGMP :
         {
           IGMP_HDRM *trame_IGMP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_IGMP = (IGMP_HDRM *)(Trame_buffer[id].buffer+3+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Internet Group Management Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           _snprintf(tmp,MAX_PATH,"Type : 0x%02X%",trame_IGMP->type);
           if (trame_IGMP->type == 11)strncat(tmp," Requête pour identifier les groupes ayant des membres actifs.",MAX_PATH);
           else if (trame_IGMP->type == 12)strncat(tmp," Rapport d'appartenance au groupe émis par un membre actif du groupe (IGMP version 1)",MAX_PATH);
           else if (trame_IGMP->type == 16)strncat(tmp," Rapport d'appartenance au groupe émis par un membre actif du groupe (IGMP version 2)",MAX_PATH);
           else if (trame_IGMP->type == 17)strncat(tmp," Un membre annonce son départ du groupe.",MAX_PATH);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Temps Maximum de réponse : %3.1f secondes (0x%02X%)",trame_IGMP->Temp_reponse,trame_IGMP->Temp_reponse/10);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_IGMP->checksum[0],trame_IGMP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Adresse de Multicast : %d.%d.%d.%d (0x%02X%0 Version 62X%02X%02X)"
          ,trame_IGMP->ip_destaddr[0],trame_IGMP->ip_destaddr[1],trame_IGMP->ip_destaddr[2],trame_IGMP->ip_destaddr[3]
          ,trame_IGMP->ip_destaddr[0],trame_IGMP->ip_destaddr[1],trame_IGMP->ip_destaddr[2],trame_IGMP->ip_destaddr[3]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          //données
          {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement Version 6
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+2);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-40;

             for (i=2;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
         }
    break;
    case IPPROTO_VRRP :
         {
           VRRP_HDRM *trame_VRRP;
           trame_VRRP = (VRRP_HDRM *)(Trame_buffer[id].buffer+3+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Virtual Router Redundancy Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           _snprintf(tmp,MAX_PATH,"Version : %d",(trame_VRRP->Vers_Type&240)/16);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Type de paquet : %d",trame_VRRP->Vers_Type&15);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Virtual Rtr ID : %d (0x%02X)",trame_VRRP->Virtual_RTT_ID,trame_VRRP->Virtual_RTT_ID);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Priorité : %d (0x%02X)",trame_VRRP->Priorite,trame_VRRP->Priorite);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Nombre d'IP : %d (0x%02X)",trame_VRRP->count_ip_addr,trame_VRRP->count_ip_addr);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Type d'authentification : %d (0x%02X)",trame_VRRP->Auth_Type,trame_VRRP->Auth_Type);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Temps avant avertissement : %d secondes",trame_VRRP->Adver_Int);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_VRRP->Checksum[0],trame_VRRP->Checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           //boucle de gestion des adresses IP
           switch(trame_VRRP->count_ip_addr)
           {
             case 1:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
             case 2:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(2) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip1[0],trame_VRRP->Ip1[1],trame_VRRP->Ip1[2],trame_VRRP->Ip1[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
             case 3:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(2) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip1[0],trame_VRRP->Ip1[1],trame_VRRP->Ip1[2],trame_VRRP->Ip1[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(3) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip2[0],trame_VRRP->Ip2[1],trame_VRRP->Ip2[2],trame_VRRP->Ip2[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
             case 4:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(2) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip1[0],trame_VRRP->Ip1[1],trame_VRRP->Ip1[2],trame_VRRP->Ip1[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(3) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip2[0],trame_VRRP->Ip2[1],trame_VRRP->Ip2[2],trame_VRRP->Ip2[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(4) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip3[0],trame_VRRP->Ip3[1],trame_VRRP->Ip3[2],trame_VRRP->Ip3[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
           }
      }
    break;



    /*case IPPROTO_GGP :

    break;
    case IPPROTO_PUP :

    break;
    case IPPROTO_IDP : //Xerox Network System
    //http://www.javvin.com/protocolIDP.html
    //http://www.protocols.com/pbook/xns.htm
        /* Le protocole IDP utilise une simple trame reprenant le réseau de destination (4 bytes)
         , le numéro du port de destination utilisé (2 Bytes)
         , le réseau de départ (4 bytes)
         , le nombre de noeud rencontré et le type de message


         struct IDP
         {
           unsigned char ip_dest[4];
           unsigned char port_dest[2];
           unsigned char ip_src[4];
           unsigned char port_src[2];

           unsigned char hop_count; // :4 ?
           unsigned char paquet_type;
         }
         *//*
    break;
    case IPPROTO_ND :

    break;
    case IPPROTO_RAW :

    break;*/
    default :
            //affichage standard + toutes les données en hexa
           {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-28;

             for (i=2;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
    break;
  }
  //ReleaseMutex(hMutex_TRAME_BUFFER);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
}
//------------------------------------------------------------------------------
void TraitementAffichageIPV6(char *titre,/* IPV4_HDR *trame, unsigned short taille_trame,*/ unsigned long int id, HANDLE HtreeView, HTREEITEM Htree)
{
  //on récupère la trame, on enlève l'entête
  IPV6_HDRM *trame;

  //ajout du titre item courrant
  char tmp[MAX_PATH+1] = "Internet Protocol Version 6\0";
  TV_INSERTSTRUCT TvItem;
  TvItem.hInsertAfter = TVI_LAST;
  ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
  TvItem.item.mask  = TVIF_TEXT;
  TvItem.hParent = Htree;
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  //WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  trame = (IPV6_HDRM *)(Trame_buffer[id].buffer+3);

  //on continue avec le reste des informations : version : sur 4 bits ^^
  _snprintf(tmp,MAX_PATH,"Version : %d",(trame->ip_version&0xF0) >> 4);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //Classe ^^ : 8 bits ^^
  _snprintf(tmp,MAX_PATH,"Classe : 0x%02X%02X",(trame->ip_version & 0x0F),(trame->version_ext[0]&0xF0) >> 4);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //Flow label : 20 bits
  _snprintf(tmp,MAX_PATH,"Flow label : 0x%02X%02X",trame->version_ext[1]&0xFF,trame->version_ext[2]&0xFF);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Tailles données après entête :  %d octets (0x%02X%02X)",trame->ip_Payload_Length[0]*256+trame->ip_Payload_Length[1],trame->ip_Payload_Length[0],trame->ip_Payload_Length[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Protocole : %d (0x%02X)",trame->ip_protocol,trame->ip_protocol);
  //ajout du protocole
  switch(trame->ip_protocol)
  {
    case 0x00 : strncat(tmp," hop-by-hop option\0",MAX_PATH);break;
    case IPPROTO_ICMP : strncat(tmp," ICMP\0",MAX_PATH);break;
    case 0x3A : strncat(tmp," ICMPv6\0",MAX_PATH);break;
    case IPPROTO_IGMP : strncat(tmp," IGMP\0",MAX_PATH);break;
    case IPPROTO_GGP : strncat(tmp," GGP\0",MAX_PATH);break;
    case IPPROTO_TCP : strncat(tmp," TCP\0",MAX_PATH);break;
    case IPPROTO_PUP : strncat(tmp," PUP\0",MAX_PATH);break;
    case IPPROTO_UDP : strncat(tmp," UDP\0",MAX_PATH);break;
    case IPPROTO_IDP : strncat(tmp," IDP\0",MAX_PATH);break;
    case IPPROTO_ND : strncat(tmp," ND\0",MAX_PATH);break;
    case IPPROTO_RAW : strncat(tmp," RAW\0",MAX_PATH);break;
    case IPPROTO_VRRP: strncat(tmp," VRRP\0",MAX_PATH);break;
    default :
      if (trame->ip_protocol>0 && trame->ip_protocol<138)
      {
        strncat(tmp," ",MAX_PATH);
        strncat(tmp,ProtocolesTypesNames2[trame->ip_protocol],MAX_PATH);
        strncat(tmp,"\0",MAX_PATH);break;
      }else
        strncat(tmp," Inconnu\0",MAX_PATH);break;
  }
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //état des hop
  _snprintf(tmp,MAX_PATH,"Hop Limit : %d (0x%02X)",trame->ip_Hop_Limit,trame->ip_Hop_Limit);
  //ajout du protocole
  switch(trame->ip_Hop_Limit)
  {
    case 00 : strncat(tmp," Option Sauts après sauts\0",MAX_PATH);break;
    case 43 : strncat(tmp," Option Routage\0",MAX_PATH);break;
    case 44 : strncat(tmp," Option Fragmentation\0",MAX_PATH);break;
    case 50 : strncat(tmp," Option ESP\0",MAX_PATH);break;
    case 51 : strncat(tmp," Option AH\0",MAX_PATH);break;
    case 60 : strncat(tmp," Option Destination\0",MAX_PATH);break;
    default : strncat(tmp," Inconnu\0",MAX_PATH);break;
  }
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  //ip
  _snprintf(tmp,MAX_PATH,"IP source : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
  ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]
  ,trame->ip_srcaddr[4],trame->ip_srcaddr[5],trame->ip_srcaddr[6],trame->ip_srcaddr[7]
  ,trame->ip_srcaddr[8],trame->ip_srcaddr[9],trame->ip_srcaddr[10],trame->ip_srcaddr[11]
  ,trame->ip_srcaddr[12],trame->ip_srcaddr[13],trame->ip_srcaddr[14],trame->ip_srcaddr[15]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"IP destination : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
  ,trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]
  ,trame->ip_destaddr[4],trame->ip_destaddr[5],trame->ip_destaddr[6],trame->ip_destaddr[7]
  ,trame->ip_destaddr[8],trame->ip_destaddr[9],trame->ip_destaddr[10],trame->ip_destaddr[11]
  ,trame->ip_destaddr[12],trame->ip_destaddr[13],trame->ip_destaddr[14],trame->ip_destaddr[15]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);

  //traitement en fonction du type de protocole
  char *buffer_donnees;
  unsigned char variable_ajustement = 0;
  unsigned int start_i = 2;
  unsigned int protocole = trame->ip_protocol;
  unsigned int var_ajuste = 0;

  rest:

  switch(protocole)
  {
    case 0x00 : //Next header: IPv6 hop-by-hop option (0x00)
    {
      HEADER_HBH *trame_hbh;
      trame_hbh = (HEADER_HBH *)(Trame_buffer[id].buffer+3+sizeof(char)*5+5);

      strcpy(tmp,"hop-by-hop option");
      TvItem.item.pszText = tmp;
      TvItem.hParent = TreeView_InsertItem(HtreeView, &TvItem);

      _snprintf(tmp,MAX_PATH,"Protocole : %d (0x%02X)",trame_hbh->ip_protocol,trame_hbh->ip_protocol);
      //ajout du protocole
      switch(trame_hbh->ip_protocol)
      {
        case 0x00 : strncat(tmp," hop-by-hop option\0",MAX_PATH);break;
        case IPPROTO_ICMP : strncat(tmp," ICMP\0",MAX_PATH);break;
        case 0x3A : strncat(tmp," ICMPv6\0",MAX_PATH);break;
        case IPPROTO_IGMP : strncat(tmp," IGMP\0",MAX_PATH);break;
        case IPPROTO_GGP : strncat(tmp," GGP\0",MAX_PATH);break;
        case IPPROTO_TCP : strncat(tmp," TCP\0",MAX_PATH);break;
        case IPPROTO_PUP : strncat(tmp," PUP\0",MAX_PATH);break;
        case IPPROTO_UDP : strncat(tmp," UDP\0",MAX_PATH);break;
        case IPPROTO_IDP : strncat(tmp," IDP\0",MAX_PATH);break;
        case IPPROTO_ND : strncat(tmp," ND\0",MAX_PATH);break;
        case IPPROTO_RAW : strncat(tmp," RAW\0",MAX_PATH);break;
        case IPPROTO_VRRP: strncat(tmp," VRRP\0",MAX_PATH);break;
        default :
          if (trame_hbh->ip_protocol>0 && trame_hbh->ip_protocol<138)
          {
            strncat(tmp," ",MAX_PATH);
            strncat(tmp,ProtocolesTypesNames2[trame_hbh->ip_protocol],MAX_PATH);
            strncat(tmp,"\0",MAX_PATH);break;
          }else
            strncat(tmp," Inconnu\0",MAX_PATH);break;
      }
      TvItem.item.pszText = tmp;
      TreeView_InsertItem(HtreeView, &TvItem);

      if (trame_hbh->Length == 0x00)
        _snprintf(tmp,MAX_PATH,"Taille : %d (8 octets par défaut)",trame_hbh->Length);
      else
        _snprintf(tmp,MAX_PATH,"Taille : %d (0x%02X)",trame_hbh->Length,trame_hbh->Length);

      TvItem.item.pszText = tmp;
      TreeView_InsertItem(HtreeView, &TvItem);

      _snprintf(tmp,MAX_PATH,"Routeur alerte : 0x%02X%02X%02X%02X",trame_hbh->Routeur_Alerte[0],trame_hbh->Routeur_Alerte[1],trame_hbh->Routeur_Alerte[2],trame_hbh->Routeur_Alerte[3]);
      TvItem.item.pszText = tmp;
      TreeView_InsertItem(HtreeView, &TvItem);

      _snprintf(tmp,MAX_PATH,"PADN : 0x%02X%02X",trame_hbh->Padn[0],trame_hbh->Padn[1]);
      TvItem.item.pszText = tmp;
      TreeView_InsertItem(HtreeView, &TvItem);

      //on relance le test de protocole ^^
      protocole = trame_hbh->ip_protocol;
      //start_i+=8;
      var_ajuste=1;
      goto rest;
    }
    break;
    case 0x3A : //ICMPv6
         {
           ICMP_HDRMV6 *trame_ICMP;
           trame_ICMP = (ICMP_HDRMV6 *)(Trame_buffer[id].buffer+3+sizeof(char)*5+5+var_ajuste);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Internet Control Message Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

          _snprintf(tmp,MAX_PATH,"type : 0x%02X (%d)",trame_ICMP->type,trame_ICMP->type);
          switch(trame_ICMP->type)
          {
            case 1:
                    strncat(tmp," Destination unreachable\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);

                    switch(trame_ICMP->code)
                    {
                      case 0: strncat(tmp," no route to destination\0",MAX_PATH);break;
                      case 1: strncat(tmp," communication with destination administratively prohibited\0",MAX_PATH);break;
                      case 2: strncat(tmp," (not assigned)\0",MAX_PATH);break;
                      case 3: strncat(tmp," address unreachable\0",MAX_PATH);break;
                      case 4: strncat(tmp," port unreachable\0",MAX_PATH);break;
                      case 5: strncat(tmp," Source address failed ingress/egress policy\0",MAX_PATH);break;
                      case 6: strncat(tmp," Reject route to destination\0",MAX_PATH);break;
                    }
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+6+var_ajuste+sizeof(char)*2);
                    variable_ajustement = 40+16;
                    start_i+=4;
            break;
            case 2:
                    strncat(tmp," Packet too big\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //on affiche le code
                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //checksum
                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //MTU
                    _snprintf(tmp,MAX_PATH,"MTU : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+6+var_ajuste+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 3:
                    strncat(tmp," Time exceeded\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    switch(trame_ICMP->code)
                    {
                      case 0: strncat(tmp," hop limit exceeded in transit\0",MAX_PATH);break;
                      case 1: strncat(tmp," fragment reassembly time exceeded\0",MAX_PATH);break;
                    }
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*2);
                    variable_ajustement = 40+16;
                    start_i+=4;
            break;
            case 4:
                    strncat(tmp," Parameter problem\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    switch(trame_ICMP->code)
                    {
                      case 0: strncat(tmp," erroneous header field encountered\0",MAX_PATH);break;
                      case 1: strncat(tmp," unrecognized Next Header type encountered\0",MAX_PATH);break;
                      case 2: strncat(tmp," unrecognized IPv6 option encountered\0",MAX_PATH);break;
                    }
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //Pointeur
                    _snprintf(tmp,MAX_PATH,"Pointer : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+var_ajuste+5+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 128:
                    strncat(tmp," Echo request\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //ID
                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Séquence number : 0x%02X%02X%02X%02X",trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //datas ^^
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+var_ajuste+5+6+sizeof(char)*2);
                    variable_ajustement = 40+24;
                    start_i+=4;
            break;
            case 129:
                    strncat(tmp," Echo reply\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //ID
                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Séquence number : 0x%02X%02X%02X%02X",trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //datas ^^
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+var_ajuste+5+6+sizeof(char)*2);
                    variable_ajustement = 40+24;
                    start_i+=4;
            break;
            case 130:
                    strncat(tmp," Group Membership Query\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Maximum Response Delay : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Target IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*5);
                    variable_ajustement = 40+36;
            break;
            case 131:
                    strncat(tmp," Group Membership Report\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Maximum Response Delay : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Target IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+var_ajuste+5+6+sizeof(char)*5);
                    variable_ajustement = 40+36;
            break;
            case 132:
                    strncat(tmp," Group Membership Reduction\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Maximum Response Delay : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Target IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*5);
                    variable_ajustement = 40+36;
            break;
            case 133: strncat(tmp," Router Solicitation\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Target IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*5);
                    variable_ajustement = 40+36;
            break;
            case 134: strncat(tmp," Router Advertisement\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //Hop Limit
                    _snprintf(tmp,MAX_PATH,"Hop limit : 0x%02X (%d)",trame_ICMP->option[0],trame_ICMP->option[0]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //gestion des infos autres ^^
                    /*_snprintf(tmp,MAX_PATH,"Managed address configuration : %s",trame_ICMP->option[1]&0x80 == TRUE?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Other stateful configuration : %s",trame_ICMP->option[1]&0x40 == TRUE?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Not home agent : %s",trame_ICMP->option[1]&0x20 == TRUE?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Routeur préférence : %s",trame_ICMP->option[1]&0x10 == TRUE?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);*/

                    //réservé
                    _snprintf(tmp,MAX_PATH,"FLAG : 0x%02X",trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Router Lifetime : 0x%02X%02X (%d)",trame_ICMP->option[2],trame_ICMP->option[3],trame_ICMP->option[2]*256 + trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Reachable Time : 0x%02X%02X%02X%02X",trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Retrans Timer : 0x%02X%02X%02X%02X",trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+28;
            break;
            case 135: strncat(tmp," Neighbor Solicitation\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Target IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*5);
                    variable_ajustement = 40+36;
            break;
            case 136: strncat(tmp," Neighbor Advertisement\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //gestion des infos autres ^^
                    _snprintf(tmp,MAX_PATH,"Router flag : %s",trame_ICMP->option[0]&0x80?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Solicited flag : %s",trame_ICMP->option[0]&0x40?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Override flag : %s",trame_ICMP->option[0]&0x20?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X%02X",trame_ICMP->option[0]/*&0x1F*/,trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Target IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*5);
                    variable_ajustement = 40+36;
            break;
            case 137: strncat(tmp," Redirect\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Target IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Destination IP : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr2[0],trame_ICMP->ip_destaddr2[1],trame_ICMP->ip_destaddr2[2],trame_ICMP->ip_destaddr2[3]
                    ,trame_ICMP->ip_destaddr2[4],trame_ICMP->ip_destaddr2[5],trame_ICMP->ip_destaddr2[6],trame_ICMP->ip_destaddr2[7]
                    ,trame_ICMP->ip_destaddr2[8],trame_ICMP->ip_destaddr2[9],trame_ICMP->ip_destaddr2[10],trame_ICMP->ip_destaddr2[11]
                    ,trame_ICMP->ip_destaddr2[12],trame_ICMP->ip_destaddr2[13],trame_ICMP->ip_destaddr2[14],trame_ICMP->ip_destaddr2[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*5);
                    variable_ajustement = 40+36;

            break;
            case 138: strncat(tmp," Router Renumbering\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Sequence number : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Segment number : 0x%02X (%d)",trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[0]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"FLAG T : %s",trame_ICMP->ip_destaddr[1]&0x80?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    _snprintf(tmp,MAX_PATH,"FLAG R : %s",trame_ICMP->ip_destaddr[1]&0x40?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    _snprintf(tmp,MAX_PATH,"FLAG A : %s",trame_ICMP->ip_destaddr[1]&0x20?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    _snprintf(tmp,MAX_PATH,"FLAG S : %s",trame_ICMP->ip_destaddr[1]&0x10?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    _snprintf(tmp,MAX_PATH,"FLAG P : %s",trame_ICMP->ip_destaddr[1]&0x08?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    _snprintf(tmp,MAX_PATH,"FLAG red : 0x%02X",trame_ICMP->ip_destaddr[1]&0x07);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Max delay : 0x%02X (%d)",trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[2]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*2);
                    variable_ajustement = 40+22;
                    start_i+=2;
            break;
            case 139: strncat(tmp," ICMP Node Information Query\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Qtype : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Flags : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 140: strncat(tmp," ICMP Node Information Response\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Qtype : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Flags : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 141: strncat(tmp," Inverse Neighbor Discovery Solicitation Message\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 142: strncat(tmp," Inverse Neighbor Discovery Advertisement Message\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 143: strncat(tmp," MLDv2 Multicast Listener Report\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Mode : 0x%02X",trame_ICMP->ip_destaddr[0]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Taille des données : 0x%02X",trame_ICMP->ip_destaddr[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    /*_snprintf(tmp,MAX_PATH,"Multicast Address Record [1] : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]
                    ,trame_ICMP->ip_destaddr2[0],trame_ICMP->ip_destaddr2[1],trame_ICMP->ip_destaddr2[2],trame_ICMP->ip_destaddr2[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);*/


                    /*int temp_M = trame_ICMP->option[2]*256+trame_ICMP->option[3];
                    _snprintf(tmp,MAX_PATH,"Nb Mcast Address Records : 0x%02X%02X (%d)",trame_ICMP->option[2],trame_ICMP->option[3],temp_M);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);*/

/*
                    if (temp_M >0)
                    {
                      _snprintf(tmp,MAX_PATH,"Multicast Address Record [1] : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                      ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                      ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                      ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                      ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                      TvItem.item.pszText = tmp;
                      TreeView_InsertItem(HtreeView, &TvItem);

                      if (temp_M >1)
                      {
                        _snprintf(tmp,MAX_PATH,"Multicast Address Record [2] : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                        ,trame_ICMP->ip_destaddr2[0],trame_ICMP->ip_destaddr2[1],trame_ICMP->ip_destaddr2[2],trame_ICMP->ip_destaddr2[3]
                        ,trame_ICMP->ip_destaddr2[4],trame_ICMP->ip_destaddr2[5],trame_ICMP->ip_destaddr2[6],trame_ICMP->ip_destaddr2[7]
                        ,trame_ICMP->ip_destaddr2[8],trame_ICMP->ip_destaddr2[9],trame_ICMP->ip_destaddr2[10],trame_ICMP->ip_destaddr2[11]
                        ,trame_ICMP->ip_destaddr2[12],trame_ICMP->ip_destaddr2[13],trame_ICMP->ip_destaddr2[14],trame_ICMP->ip_destaddr2[15]);
                        TvItem.item.pszText = tmp;
                        TreeView_InsertItem(HtreeView, &TvItem);
                      }
                    }*/

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*7);
                    variable_ajustement = 40+52;
            break;
            case 144: strncat(tmp," Home Agent Address Discovery Request Message\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 145: strncat(tmp," Home Agent Address Discovery Reply Message\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Home Agent Addresses : %02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X:%02X%02X"
                    ,trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1],trame_ICMP->ip_destaddr[2],trame_ICMP->ip_destaddr[3]
                    ,trame_ICMP->ip_destaddr[4],trame_ICMP->ip_destaddr[5],trame_ICMP->ip_destaddr[6],trame_ICMP->ip_destaddr[7]
                    ,trame_ICMP->ip_destaddr[8],trame_ICMP->ip_destaddr[9],trame_ICMP->ip_destaddr[10],trame_ICMP->ip_destaddr[11]
                    ,trame_ICMP->ip_destaddr[12],trame_ICMP->ip_destaddr[13],trame_ICMP->ip_destaddr[14],trame_ICMP->ip_destaddr[15]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*5);
                    variable_ajustement = 40+36;
            break;
            case 146: strncat(tmp," Mobile Prefix Solicitation\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 147: strncat(tmp," Mobile Prefix Advertisement\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //gestion des infos autres ^^
                    _snprintf(tmp,MAX_PATH,"Managed Address Configuration : %s",trame_ICMP->option[2]&0x80 == TRUE?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Other stateful configuration : %s",trame_ICMP->option[2]&0x40 == TRUE?"TRUE":"FALSE");
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //réservé
                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X",trame_ICMP->option[2]&0x3F,trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 148: strncat(tmp," Certification Path Solicitation\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Component  : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 149: strncat(tmp," Certification Path Advertisement\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"ALL components  : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Component  : 0x%02X%02X",trame_ICMP->ip_destaddr[0],trame_ICMP->ip_destaddr[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*2);
                    variable_ajustement = 40+22;
                    start_i+=2;
            break;
            case 150: strncat(tmp," Experimental mobility protocols\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Subtype : 0x%02X",trame_ICMP->option[0]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Option : 0x%02X%02X%02X",trame_ICMP->option[1],trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    //DATAS
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;
            break;
            case 151: strncat(tmp," MRD, Multicast Router Advertisement\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Query Interval : 0x%02X%02X",trame_ICMP->option[0],trame_ICMP->option[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Robustness Variable : 0x%02X%02X",trame_ICMP->option[2],trame_ICMP->option[3]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);
                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*3);
                    variable_ajustement = 40+20;

            break;
            case 152: strncat(tmp," MRD, Multicast Router Solicitation\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*2);
                    variable_ajustement = 40+16;
                    start_i+=4;
            break;
            case 153: strncat(tmp," MRD, Multicast Router Termination\0",MAX_PATH);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
                    TvItem.item.pszText = tmp;
                    TreeView_InsertItem(HtreeView, &TvItem);

                    //DATA
                    buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*2);
                    variable_ajustement = 40+16;
                    start_i+=4;
            break;
            case 154: strncat(tmp," FMIPv6 messages\0",MAX_PATH);break;
            default:
              TvItem.item.pszText = tmp;
              TreeView_InsertItem(HtreeView, &TvItem);

              _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
              TvItem.item.pszText = tmp;
              TreeView_InsertItem(HtreeView, &TvItem);

              _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
              TvItem.item.pszText = tmp;
              TreeView_InsertItem(HtreeView, &TvItem);

              buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+var_ajuste+6+sizeof(char)*2);
              variable_ajustement = 40+16;
              start_i+=4;
          }

          //données
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);

             //si encore des données après ^^
             if (Trame_buffer[id].taille_buffer-(variable_ajustement+start_i)>1)
             {
               TvItem.hParent = TVI_ROOT;
               strcpy(tmp,"Données");
               TvItem.item.pszText = tmp;
               Htree = TreeView_InsertItem(HtreeView, &TvItem);
               TvItem.hParent = Htree;

               //traitement par 16 + 16 carcatères
               char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
               unsigned short count=0,i;
               unsigned int taille = Trame_buffer[id].taille_buffer-(variable_ajustement);

               for (i=start_i;i<taille;i++)
               {
                 if (count == 16)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                   _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                   ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                   ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                   ,tmp3);
                   TvItem.item.pszText = tmp;
                   TreeView_InsertItem(HtreeView, &TvItem);
                   count=0;
                 }

                 tmp2[count]=buffer_donnees[i];
                 if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
                 else tmp3[count]='.';

                 count++;
               }
               if (count>0)
               {
                 for (;count<32;count++)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                 }
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
               }
            }
         }
    break;
    case IPPROTO_ICMP :
         {
           ICMP_HDRM *trame_ICMP;
           trame_ICMP = (ICMP_HDRM *)(Trame_buffer[id].buffer+3+sizeof(char)*5+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Internet Control Message Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

          _snprintf(tmp,MAX_PATH,"type : 0x%02X",trame_ICMP->type);

          switch(trame_ICMP->type)
          {
            case 0:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse à une demande d'écho\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 3:
                 if(trame_ICMP->code == 0) strncat(tmp," Réseau inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Hôte inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Protocole inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 3) strncat(tmp," Port inaccessible\0",MAX_PATH);
                 else if(trame_ICMP->code == 4) strncat(tmp," Fragmentation nécessaire mais interdite\0",MAX_PATH);
                 else if(trame_ICMP->code == 5) strncat(tmp," Echec de routage par la source\0",MAX_PATH);
                 else if(trame_ICMP->code == 6) strncat(tmp," Réseau de destination inconnu\0",MAX_PATH);
                 else if(trame_ICMP->code == 7) strncat(tmp," Hôte de destination inconnue\0",MAX_PATH);
                 else if(trame_ICMP->code == 8) strncat(tmp," Machine source isolée\0",MAX_PATH);
                 else if(trame_ICMP->code == 9) strncat(tmp," Réseau de destination interdit administrativement\0",MAX_PATH);
                 else if(trame_ICMP->code == 10) strncat(tmp," Hôte de destination interdite administrativement\0",MAX_PATH);
                 else if(trame_ICMP->code == 11) strncat(tmp," Réseau inaccessible pour ce type de service\0",MAX_PATH);
                 else if(trame_ICMP->code == 12) strncat(tmp," Hôte inaccessible pour ce type de service\0",MAX_PATH);
                 else if(trame_ICMP->code == 13) strncat(tmp," Communication interdite par un filtre\0",MAX_PATH);
                 else if(trame_ICMP->code == 14) strncat(tmp," Arrêt de continuité de la communication\0",MAX_PATH);
                 else if(trame_ICMP->code == 15) strncat(tmp," Redirection pour un réseau et pour un service donné\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 4:
                 if(trame_ICMP->code == 0) strncat(tmp," Volume de donnée trop importante\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 5:
                 if(trame_ICMP->code == 0) strncat(tmp," Redirection pour un hôte\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Redirection pour un hôte et pour un service donné\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Redirection pour un réseau\0",MAX_PATH);
                 else if(trame_ICMP->code == 3) strncat(tmp," Redirection pour un réseau et pour un service donné\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 8:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande d'écho\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 9:
                 if(trame_ICMP->code == 0) strncat(tmp," Avertissement routeur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 10:
                 if(trame_ICMP->code == 0) strncat(tmp," Sollicitation routeur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 11:
                 if(trame_ICMP->code == 0) strncat(tmp," Durée de vie écoulée avant d'arrivée à destination\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Temps limite de réassemblage du fragment dépassé\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 12:
                 if(trame_ICMP->code == 0) strncat(tmp," En-tête IP invalide\0",MAX_PATH);
                 else if(trame_ICMP->code == 1) strncat(tmp," Manque d'une option obligatoire\0",MAX_PATH);
                 else if(trame_ICMP->code == 2) strncat(tmp," Mauvaise longueur\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 13:
                 if(trame_ICMP->code == 0) strncat(tmp," Requête pour un marqueur temporel \0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 14:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse pour un marqueur temporel\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 15:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande d'adresse réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 16:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse d'adresse réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 17:
                 if(trame_ICMP->code == 0) strncat(tmp," Demande de masque de sous réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            case 18:
                 if(trame_ICMP->code == 0) strncat(tmp," Réponse de masque de sous réseau\0",MAX_PATH);
                 else strncat(tmp," Code message inconnu\0",MAX_PATH);
            break;
            default : strncat(tmp," Code message inconnu\0",MAX_PATH);break;
          }
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Code : 0x%02X",trame_ICMP->code);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_ICMP->checksum[0],trame_ICMP->checksum[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Identifiant : 0x%02X%02X",trame_ICMP->identifiant[0],trame_ICMP->identifiant[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Numéro de séquence : %d (0x%02X%02X)",trame_ICMP->numero_seq[0]*0x100+trame_ICMP->numero_seq[1],trame_ICMP->numero_seq[0],trame_ICMP->numero_seq[1]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          //données
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+2);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-40;

             for (i=2;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }

         }
    break;
    case IPPROTO_TCP :
         {
           TCP_HDRM *trame_TCP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_TCP = (TCP_HDRM *)(Trame_buffer[id].buffer+3+sizeof(char)*5+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;

           Trame_buffer[id].src_port = trame_TCP->source_port[0]*256+trame_TCP->source_port[1];
           Trame_buffer[id].dst_port = trame_TCP->dest_port[0]*256+trame_TCP->dest_port[1];

           if (!ResolutionPort(Trame_buffer[id].src_port,tmp2))tmp2[0]=0;
           if (!ResolutionPort(Trame_buffer[id].dst_port,tmp3))tmp3[0]=0;

           _snprintf(tmp,MAX_PATH,"Transmission Control Protocol [Ports : %d (%s) -> %d (%s)]"
           ,Trame_buffer[id].src_port,tmp2[0]!=0?tmp2:"-",Trame_buffer[id].dst_port,tmp3[0]!=0?tmp3:"-");

           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           //on ajoute maintenant la suite :)
           _snprintf(tmp,MAX_PATH,"Port source : %d (0x%02X%02X) %s"
           ,Trame_buffer[id].src_port,trame_TCP->source_port[0],trame_TCP->source_port[1],tmp2[0]!=0?tmp2:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Port de destination : %d (0x%02X%02X) %s"
           ,Trame_buffer[id].dst_port,trame_TCP->dest_port[0],trame_TCP->dest_port[1],tmp3[0]!=0?tmp3:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Séquence : %u (0x%02X%02X%02X%02X)"
           ,trame_TCP->sequence[0]*0x1000000+trame_TCP->sequence[1]*0x10000+trame_TCP->sequence[2]*0x100+trame_TCP->sequence[3]
           ,trame_TCP->sequence[0],trame_TCP->sequence[1],trame_TCP->sequence[2],trame_TCP->sequence[3]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Acquittement : %u (0x%02X%02X%02X%02X)"
           ,trame_TCP->acknowledge[0]*0x1000000+trame_TCP->acknowledge[1]*0x10000+trame_TCP->acknowledge[2]*0x100+trame_TCP->acknowledge[3]
           ,trame_TCP->acknowledge[0],trame_TCP->acknowledge[1],trame_TCP->acknowledge[2],trame_TCP->acknowledge[3]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Taille de l'entête : %d",(trame_TCP->header_length_sup/0x10)*trame->ip_version);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           strcpy(tmp,"Flags :\0");
           if ((trame_TCP->flags&128)/128)strcat(tmp," CWR\0");
           if ((trame_TCP->flags&64)/64)strcat(tmp," ECN\0");
           if ((trame_TCP->flags&32)/32)strcat(tmp," URG\0");
           if ((trame_TCP->flags&16)/16)strcat(tmp," ACK\0");
           if ((trame_TCP->flags&8)/8)strcat(tmp," PSH\0");
           if ((trame_TCP->flags&4)/4)strcat(tmp," RST\0");
           if ((trame_TCP->flags&2)/2)strcat(tmp," SYN\0");
           if ((trame_TCP->flags&1)/1)strcat(tmp," FIN\0");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Windows : %d (0x%02X%02X)",trame_TCP->window[0]*0x100+trame_TCP->window[1],trame_TCP->window[0],trame_TCP->window[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_TCP->checksum[0],trame_TCP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Urgent : 0x%02X%02X",trame_TCP->urgent_pointer[0],trame_TCP->urgent_pointer[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           //travail maintenant en fonction du protocole identifié ^^
           //DATA (traitement de l'affichage par ligne en cas de reconnaissance d'un '\r ou \n)
           //suivant les protocoles
           //if (Trame_buffer[id].protocoleChoix!=CODE_OTHER)//!non reconnu
           {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+4+sizeof(char)*5);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-(48+20);

             for (i=6;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
         }
    break;

    case IPPROTO_UDP :
         {
           UDP_HDRM *trame_UDP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_UDP = (UDP_HDRM *)(Trame_buffer[id].buffer+3+sizeof(char)*5+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           Trame_buffer[id].src_port = trame_UDP->source_port[0]*256+trame_UDP->source_port[1];
           Trame_buffer[id].dst_port = trame_UDP->dest_port[0]*256+trame_UDP->dest_port[1];

           if (!ResolutionPort(Trame_buffer[id].src_port,tmp2))tmp2[0]=0;
           if (!ResolutionPort(Trame_buffer[id].dst_port,tmp3))tmp3[0]=0;

           _snprintf(tmp,MAX_PATH,"User Datagram Protocol [Ports : %d (%s) -> %d (%s)]"
           ,Trame_buffer[id].src_port,tmp2[0]!=0?tmp2:"-",Trame_buffer[id].dst_port,tmp3[0]!=0?tmp3:"-");

           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           //on ajoute maintenant la suite :)
           _snprintf(tmp,MAX_PATH,"Port source : %d (0x%02X%02X) %s"
           ,Trame_buffer[id].src_port,trame_UDP->source_port[0],trame_UDP->source_port[1],tmp2[0]!=0?tmp2:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Port de destination : %d (0x%02X%02X) %s"
           ,Trame_buffer[id].dst_port,trame_UDP->dest_port[0],trame_UDP->dest_port[1],tmp3[0]!=0?tmp3:" ");
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Taille : %d (0x%02X%02X)",trame_UDP->length[0]*0x100+trame_UDP->length[1],trame_UDP->length[0],trame_UDP->length[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_UDP->checksum[0],trame_UDP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);


           //traitement des protocoles connus

           /*
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
           */

           switch(Trame_buffer[id].protocoleChoix)
           {
             case CODE_DHCP :
                  {
                     TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
                     TvItem.hParent = TVI_ROOT;
                     strcpy(tmp,"DHCP : Bootstrap Protocol");
                     TvItem.item.pszText = tmp;
                     Htree = TreeView_InsertItem(HtreeView, &TvItem);
                     TvItem.hParent = Htree;
                     HTREEITEM Htree_tmp = Htree;

                     DHCP_HDRM *trame_DHCP;
                     trame_DHCP = (DHCP_HDRM *)(Trame_buffer[id].buffer+3+5+2+sizeof(char)*5);

                     switch(trame_DHCP->Type)
                     {
                       case 1: strcpy(tmp,"Type de message : (1) Boot Request");break;
                       default : _snprintf(tmp,MAX_PATH,"Type de message : (%d) Inconnu",trame_DHCP->Type);
                     }
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     switch(trame_DHCP->Hard_Type)
                     {
                       case 1: strcpy(tmp,"Hardware type : (1) Ethernet");break;
                       default : _snprintf(tmp,MAX_PATH,"Hardware type : (%d) Inconnu",trame_DHCP->Hard_Type);
                     }
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Taille de l'adresse physique : %d octet(s)",trame_DHCP->Hard_Length);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Nombre de saut : %d",trame_DHCP->Hop);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Numéro de transaction : 0x%02X%02X%02X%02X"
                     ,trame_DHCP->ID[0],trame_DHCP->ID[1],trame_DHCP->ID[2],trame_DHCP->ID[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Temps écoulé : %d secondes (0x%02X%02X)",trame_DHCP->time[1]*256+trame_DHCP->time[0],trame_DHCP->time[0],trame_DHCP->time[1]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"BOOTP flags : 0x%02X%02X",trame_DHCP->flags[0],trame_DHCP->flags[1]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"IP du client : %d.%d.%d.%d"
                     ,trame_DHCP->IP_client[0],trame_DHCP->IP_client[1],trame_DHCP->IP_client[2],trame_DHCP->IP_client[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"Your IP : %d.%d.%d.%d"
                     ,trame_DHCP->IP_your[0],trame_DHCP->IP_your[1],trame_DHCP->IP_your[2],trame_DHCP->IP_your[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"IP du serveur : %d.%d.%d.%d"
                     ,trame_DHCP->IP_serveur[0],trame_DHCP->IP_serveur[1],trame_DHCP->IP_serveur[2],trame_DHCP->IP_serveur[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);
                     _snprintf(tmp,MAX_PATH,"IP due la passerelle : %d.%d.%d.%d"
                     ,trame_DHCP->IP_passerelle[0],trame_DHCP->IP_passerelle[1],trame_DHCP->IP_passerelle[2],trame_DHCP->IP_passerelle[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     if (trame_DHCP->Hard_Length ==6 ) //par défaut
                     {
                       _snprintf(tmp,MAX_PATH,"Adresse MAC du client : %02X:%02X:%02X:%02X:%02X:%02X"
                       ,trame_DHCP->MAC_client[0],trame_DHCP->MAC_client[1]
                       ,trame_DHCP->MAC_client[2],trame_DHCP->MAC_client[3]
                       ,trame_DHCP->MAC_client[4],trame_DHCP->MAC_client[5]);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       //padding
                       _snprintf(tmp,MAX_PATH,"Adresse MAC Padding : %02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X:%02X"
                       ,trame_DHCP->MAC_padding[0],trame_DHCP->MAC_padding[1]
                       ,trame_DHCP->MAC_padding[2],trame_DHCP->MAC_padding[3]
                       ,trame_DHCP->MAC_padding[4],trame_DHCP->MAC_padding[5]
                       ,trame_DHCP->MAC_padding[6],trame_DHCP->MAC_padding[7]
                       ,trame_DHCP->MAC_padding[8],trame_DHCP->MAC_padding[9]
                       );
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                     }else
                     {
                       char tmp2[4];
                       unsigned int j;
                       if (trame_DHCP->Hard_Length>0)
                       {
                         if (trame_DHCP->Hard_Length<6)
                         {
                           strcpy(tmp,"Adresse MAC du client : ");
                           for (j=0;j<trame_DHCP->Hard_Length;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_client[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);

                           strcpy(tmp,"Adresse MAC Padding : ");
                           for (j=trame_DHCP->Hard_Length;j<6;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_client[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           for (j=0;j<10;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);
                         }else //si >6
                         {
                           _snprintf(tmp,MAX_PATH,"Adresse MAC du client : %02X:%02X:%02X:%02X:%02X:%02X:"
                           ,trame_DHCP->MAC_client[0],trame_DHCP->MAC_client[1]
                           ,trame_DHCP->MAC_client[2],trame_DHCP->MAC_client[3]
                           ,trame_DHCP->MAC_client[4],trame_DHCP->MAC_client[5]);

                           for (j=0;j<(trame_DHCP->Hard_Length-6);j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);

                           strcpy(tmp,"Adresse MAC Padding : ");
                           for (j=(trame_DHCP->Hard_Length-6);j<10;j++)
                           {
                             _snprintf(tmp2,4,"%02X:",trame_DHCP->MAC_padding[j]);
                             strncat(tmp,tmp2,MAX_PATH);
                           }
                           strncat(tmp,"\0",MAX_PATH);
                           tmp[strlen(tmp)-1]=0; // suppression du dernier ':'
                           TvItem.item.pszText = tmp;
                           TreeView_InsertItem(HtreeView, &TvItem);
                         }
                       }
                     }

                     _snprintf(tmp,MAX_PATH,"Nom du serveur hôte : %s",trame_DHCP->Serveur_Host);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Nom du fichier de configuration : %s",trame_DHCP->Boot_File_Name);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     _snprintf(tmp,MAX_PATH,"Magic cookie : 0x%02X%02X%02X%02X"
                     ,trame_DHCP->Magic_cookie[0],trame_DHCP->Magic_cookie[1],trame_DHCP->Magic_cookie[2],trame_DHCP->Magic_cookie[3]);
                     TvItem.item.pszText = tmp;
                     TreeView_InsertItem(HtreeView, &TvItem);

                     //gestion des options
                     strcpy(tmp,"Options : ");
                     TvItem.item.pszText = tmp;
                     Htree = TreeView_InsertItem(HtreeView, &TvItem);
                     TvItem.hParent = Htree;

                     unsigned short j=0;
                     while (trame_DHCP->Options[j] != 0xFF && j<64)
                     {
                       //lecture de l'option
                       switch(trame_DHCP->Options[j])
                       {
                         case 0x0C :strcpy(tmp,"Option : (0x0C) Host Name");break;
                         case 0x2B :strcpy(tmp,"Option : (0x2B) Vendor-Specific Information");break;
                         case 0x35 :strcpy(tmp,"Option : (0x35) DHCP Message Type");break;
                         case 0x37 :strcpy(tmp,"Option : (0x37) Parameter Request List");break;
                         case 0x3C :strcpy(tmp,"Option : (0x3C) Vendor class identifier");break;
                         case 0x3D :strcpy(tmp,"Option : (0x3D) Client identifier (type + Adresse MAC)");break;
                         case 0x74 :strcpy(tmp,"Option : (0x74) DHCP Auto-Configuration");break;
                         default :_snprintf(tmp,MAX_PATH,"Option : (0x%02X) Inconnu",trame_DHCP->Options[j]);break;
                       }
                       TvItem.item.pszText = tmp;
                       TvItem.hParent = Htree;
                       TvItem.hParent = TreeView_InsertItem(HtreeView, &TvItem);

                       _snprintf(tmp,MAX_PATH,"Taille des données : %d",trame_DHCP->Options[j+1]);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       strcpy(tmp,"Valeur : 0x");
                       unsigned int k=0;
                       char tmp2[3];
                       for (k=0; k<trame_DHCP->Options[j+1]; k++)
                       {
                         _snprintf(tmp2,3,"%02X",trame_DHCP->Options[j+2+k]);
                         strncat(tmp,tmp2,MAX_PATH);
                       }

                       char tmp3[256];
                       strncpy(tmp3,&(trame_DHCP->Options[j+2]),trame_DHCP->Options[j+1]);
                       tmp3[trame_DHCP->Options[j+1]]=0;

                       strncat(tmp," (",MAX_PATH);
                       strncat(tmp,tmp3,MAX_PATH);
                       strncat(tmp,")\0",MAX_PATH);
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                       j+=2+trame_DHCP->Options[j+1];
                     }
                     //gestion des options
                     if (j<64)
                     {
                       _snprintf(tmp,MAX_PATH,"Fin des options : 0x%02X",trame_DHCP->Options[j]);
                       TvItem.hParent = Htree;
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);

                       strcpy(tmp,"Padding : ....");
                       TvItem.item.pszText = tmp;
                       TreeView_InsertItem(HtreeView, &TvItem);
                     }
                     TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
                     TreeView_Expand(HtreeView,Htree_tmp,TVE_EXPAND);
                  }
             break;
             default:
             //affichage des données
             {
               TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
               TvItem.hParent = TVI_ROOT;
               strcpy(tmp,"Données");
               TvItem.item.pszText = tmp;
               Htree = TreeView_InsertItem(HtreeView, &TvItem);
               TvItem.hParent = Htree;

               //traitement
               char *buffer_donnees;
               buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+1+sizeof(char)*5);

               //traitement par 16 + 16 carcatères
               char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
               unsigned short count=0,i;
               unsigned int taille = Trame_buffer[id].taille_buffer-(36+20);

               for (i=6;i<taille;i++)
               {
                 if (count == 16)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                   _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                   ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                   ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                   ,tmp3);
                   TvItem.item.pszText = tmp;
                   TreeView_InsertItem(HtreeView, &TvItem);
                   count=0;
                 }

                 tmp2[count]=buffer_donnees[i];
                 if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
                 else tmp3[count]='.';

                 count++;
               }
               if (count>0)
               {
                 for (;count<32;count++)
                 {
                   tmp2[count]=0;
                   tmp3[count]=0;
                 }
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
               }
             }
           }
         }
    break;
    case IPPROTO_IGMP :
         {
           IGMP_HDRM *trame_IGMP;
           char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
           trame_IGMP = (IGMP_HDRM *)(Trame_buffer[id].buffer+3+sizeof(char)*5+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Internet Group Management Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           _snprintf(tmp,MAX_PATH,"Type : 0x%02X%",trame_IGMP->type);
           if (trame_IGMP->type == 11)strncat(tmp," Requête pour identifier les groupes ayant des membres actifs.",MAX_PATH);
           else if (trame_IGMP->type == 12)strncat(tmp," Rapport d'appartenance au groupe émis par un membre actif du groupe (IGMP version 1)",MAX_PATH);
           else if (trame_IGMP->type == 16)strncat(tmp," Rapport d'appartenance au groupe émis par un membre actif du groupe (IGMP version 2)",MAX_PATH);
           else if (trame_IGMP->type == 17)strncat(tmp," Un membre annonce son départ du groupe.",MAX_PATH);
           TvItem.item.pszText = tmp;             //traitement par 16 + 16 carcatères
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Temps Maximum de réponse : %3.1f secondes (0x%02X%)",trame_IGMP->Temp_reponse,trame_IGMP->Temp_reponse/10);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_IGMP->checksum[0],trame_IGMP->checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH,"Adresse de Multicast : %d.%d.%d.%d (0x%02X%02X%02X%02X)"
          ,trame_IGMP->ip_destaddr[0],trame_IGMP->ip_destaddr[1],trame_IGMP->ip_destaddr[2],trame_IGMP->ip_destaddr[3]
          ,trame_IGMP->ip_destaddr[0],trame_IGMP->ip_destaddr[1],trame_IGMP->ip_destaddr[2],trame_IGMP->ip_destaddr[3]);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          //données
          {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+5+2+sizeof(char)*5);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-(40+20);

             for (i=2;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
         }
    break;
    case IPPROTO_VRRP :
         {
           VRRP_HDRM *trame_VRRP;
           trame_VRRP = (VRRP_HDRM *)(Trame_buffer[id].buffer+3+sizeof(char)*5+5);

           //init TVI_ROOT
           TvItem.hParent = TVI_ROOT;
           strcpy(tmp,"Virtual Router Redundancy Protocol");
           TvItem.item.pszText = tmp;
           Htree = TreeView_InsertItem(HtreeView, &TvItem);
           TvItem.hParent = Htree;

           _snprintf(tmp,MAX_PATH,"Version : %d",(trame_VRRP->Vers_Type&240)/16);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Type de paquet : %d",trame_VRRP->Vers_Type&15);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Virtual Rtr ID : %d (0x%02X)",trame_VRRP->Virtual_RTT_ID,trame_VRRP->Virtual_RTT_ID);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Priorité : %d (0x%02X)",trame_VRRP->Priorite,trame_VRRP->Priorite);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Nombre d'IP : %d (0x%02X)",trame_VRRP->count_ip_addr,trame_VRRP->count_ip_addr);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Type d'authentification : %d (0x%02X)",trame_VRRP->Auth_Type,trame_VRRP->Auth_Type);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Temps avant avertissement : %d secondes",trame_VRRP->Adver_Int);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           _snprintf(tmp,MAX_PATH,"Checksum : 0x%02X%02X",trame_VRRP->Checksum[0],trame_VRRP->Checksum[1]);
           TvItem.item.pszText = tmp;
           TreeView_InsertItem(HtreeView, &TvItem);

           //boucle de gestion des adresses IP
           switch(trame_VRRP->count_ip_addr)
           {
             case 1:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
             case 2:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(2) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip1[0],trame_VRRP->Ip1[1],trame_VRRP->Ip1[2],trame_VRRP->Ip1[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
             case 3:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(2) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip1[0],trame_VRRP->Ip1[1],trame_VRRP->Ip1[2],trame_VRRP->Ip1[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(3) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip2[0],trame_VRRP->Ip2[1],trame_VRRP->Ip2[2],trame_VRRP->Ip2[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
             case 4:
                  _snprintf(tmp,MAX_PATH,"IP(1) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip[0],trame_VRRP->Ip[1],trame_VRRP->Ip[2],trame_VRRP->Ip[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(2) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip1[0],trame_VRRP->Ip1[1],trame_VRRP->Ip1[2],trame_VRRP->Ip1[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(3) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip2[0],trame_VRRP->Ip2[1],trame_VRRP->Ip2[2],trame_VRRP->Ip2[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
                  _snprintf(tmp,MAX_PATH,"IP(4) : %d.%d.%d.%d"
                  ,trame_VRRP->Ip3[0],trame_VRRP->Ip3[1],trame_VRRP->Ip3[2],trame_VRRP->Ip3[3]);
                  TvItem.item.pszText = tmp;
                  TreeView_InsertItem(HtreeView, &TvItem);
             break;
           }
      }
    break;



    /*case IPPROTO_GGP :

    break;
    case IPPROTO_PUP :

    break;
    case IPPROTO_IDP : //Xerox Network System
    //http://www.javvin.com/protocolIDP.html
    //http://www.protocols.com/pbook/xns.htm
        /* Le protocole IDP utilise une simple trame reprenant le réseau de destination (4 bytes)
         , le numéro du port de destination utilisé (2 Bytes)
         , le réseau de départ (4 bytes)
         , le nombre de noeud rencontré et le type de message


         struct IDP
         {
           unsigned char ip_dest[4];
           unsigned char port_dest[2];
           unsigned char ip_src[4];
           unsigned char port_src[2];

           unsigned char hop_count; // :4 ?
           unsigned char paquet_type;
         }
         *//*
    break;
    case IPPROTO_ND :

    break;
    case IPPROTO_RAW :

    break;*/
    default :
            //affichage standard + toutes les données en hexa
           {
             TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
             TvItem.hParent = TVI_ROOT;
             strcpy(tmp,"Données");
             TvItem.item.pszText = tmp;
             Htree = TreeView_InsertItem(HtreeView, &TvItem);
             TvItem.hParent = Htree;

             //traitement
             char *buffer_donnees;
             buffer_donnees = (char *)(Trame_buffer[id].buffer+3+sizeof(char)*5+6);

             //traitement par 16 + 16 carcatères
             char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
             unsigned short count=0,i;
             unsigned int taille = Trame_buffer[id].taille_buffer-(36+20);

             for (i=6;i<taille;i++)
             {
               if (count == 16)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
                 _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
                 ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
                 ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
                 ,tmp3);
                 TvItem.item.pszText = tmp;
                 TreeView_InsertItem(HtreeView, &TvItem);
                 count=0;
               }

               tmp2[count]=buffer_donnees[i];
               if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
               else tmp3[count]='.';

               count++;
             }
             if (count>0)
             {
               for (;count<32;count++)
               {
                 tmp2[count]=0;
                 tmp3[count]=0;
               }
               _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
               ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
               ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
               ,tmp3);
               TvItem.item.pszText = tmp;
               TreeView_InsertItem(HtreeView, &TvItem);
             }
           }
    break;
  }
  //ReleaseMutex(hMutex_TRAME_BUFFER);
  //on ajoute un item trame total avec les données en hexa
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
  /*TvItem.hParent = TVI_ROOT;
  TvItem.item.pszText = "Trame entière";
  Htree =TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;
  //traitement par 16 + 16 carcatères
  char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
  unsigned short count=0,i;
  unsigned int taille = Trame_buffer[id].taille_buffer;

  for (i=0;i<taille;i++)
  {
     if (count == 16)
     {
       tmp2[count]=0;
       tmp3[count]=0;
       _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
       ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
       ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
       ,tmp3);
       TvItem.item.pszText = tmp;
       TreeView_InsertItem(HtreeView, &TvItem);
       count=0;
     }

     tmp2[count]=Trame_buffer[id].buffer[i];
     if (Trame_buffer[id].buffer[i]>31 && Trame_buffer[id].buffer[i]<127)tmp3[count]=Trame_buffer[id].buffer[i];
     else tmp3[count]='.';

     count++;
  }
  if (count>0)
  {
     for (;count<32;count++)
     {
       tmp2[count]=0;
       tmp3[count]=0;
     }
     _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
     ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
     ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
     ,tmp3);
     TvItem.item.pszText = tmp;
     TreeView_InsertItem(HtreeView, &TvItem);
  }
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);*/
}
//------------------------------------------------------------------------------
void TraitementAffichageARP(char *titre, unsigned int *buffer, unsigned short taille_trame, unsigned long int ID, HANDLE HtreeView, HTREEITEM Htree)
{
  char tmp[MAX_PATH] = "Adress Resolution Protocol\0";
  unsigned int i;
  //WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  switch (Trame_buffer[ID].ProtoType)
  {
    case 0x8100: //802.1Q
         {
              //a coder
         }
    break;
    case 0x0806:
         {
          ARP_HDR *trame;
          trame = (ARP_HDR*)buffer;

          //ajout du titre item courrant
          TV_INSERTSTRUCT TvItem;
          TvItem.hInsertAfter = TVI_LAST;
          ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
          TvItem.item.mask  = TVIF_TEXT;

          //titre
          TvItem.hParent = Htree;
          if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01) strcat(tmp," (requête)\0");
          else if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02) strcat(tmp," (réponse)\0");
          else strcat(tmp," (inconnu)\0");

          TvItem.item.pszText = tmp;
          Htree = TreeView_InsertItem(HtreeView, &TvItem);

          //on continue avec le reste des informations
          TvItem.hParent = Htree;
          _snprintf(tmp,MAX_PATH-1,"Hardware type : 0x%02X%02X%s",trame->hard_type[0],trame->hard_type[1],trame->hard_type[1]==0x01?" Ethernet":"");
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          switch(trame->protocol_type[0])
          {
            case 0x08:
                 switch(trame->protocol_type[1])
                 {
                  case 0x00: TvItem.item.pszText = "Protocole type : IPV4 (0x800)";break;
                  case 0x06: TvItem.item.pszText = "Protocole type : ARP (0x806)";break;
                  default :
                  _snprintf(tmp,MAX_PATH-1,"Protocole type : 0x%02X%02X",trame->protocol_type[0],trame->protocol_type[1]);
                  TvItem.item.pszText = tmp;
                 }
            break;
            case 0x81:
                 switch(trame->protocol_type[1])
                 {
                  case 0x00: strcat(tmp,"Protocole type : Ethernet 802.1Q (0x8100)");break;
                  default :
                  _snprintf(tmp,MAX_PATH-1,"Protocole type : 0x%02X%02X",trame->protocol_type[0],trame->protocol_type[1]);
                  TvItem.item.pszText = tmp;
                 }
            break;
            case 0x88:
                 switch(trame->protocol_type[1])
                 {
                  case 0xCC: strcat(tmp,"Protocole type : Ethernet 802.1 (0x88CC)");break;
                  default :
                  _snprintf(tmp,MAX_PATH-1,"Protocole type : 0x%02X%02X",trame->protocol_type[0],trame->protocol_type[1]);
                  TvItem.item.pszText = tmp;
                 }
            break;
            default :
              _snprintf(tmp,MAX_PATH-1,"Protocole type : 0x%02X%02X",trame->protocol_type[0],trame->protocol_type[1]);
              TvItem.item.pszText = tmp;
          }
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH-1,"Hardware size : %d",trame->hard_length);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          _snprintf(tmp,MAX_PATH-1,"Ip size : %d",trame->logic_length);
          TvItem.item.pszText = tmp;
          TreeView_InsertItem(HtreeView, &TvItem);

          if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)TvItem.item.pszText = "Opération : REQUEST (0x0001)";
          else if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)TvItem.item.pszText = "Opération : REPLY (0x0002)";
          else
          {
            _snprintf(tmp,MAX_PATH-1,"Operation : INCONNU (0x%02X%02X)",trame->operation[0],trame->operation[1]);
            TvItem.item.pszText = tmp;
          }

          TreeView_InsertItem(HtreeView, &TvItem);

          if (trame->logic_length == 4 && trame->hard_length == 6)//si IPV4
          {
            _snprintf(tmp,MAX_PATH-1,"Source : %d.%d.%d.%d (%02X:%02X:%02X:%02X:%02X:%02X)"
            ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]
            ,trame->_h_source[0],trame->_h_source[1],trame->_h_source[2],trame->_h_source[3],trame->_h_source[4],trame->_h_source[5]);
            TvItem.item.pszText = tmp;
            TreeView_InsertItem(HtreeView, &TvItem);

            _snprintf(tmp,MAX_PATH-1,"Destination : %d.%d.%d.%d (%02X:%02X:%02X:%02X:%02X:%02X)"
            ,trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]
            ,trame->_h_dest[0],trame->_h_dest[1],trame->_h_dest[2],trame->_h_dest[3],trame->_h_dest[4],trame->_h_dest[5]);
            TvItem.item.pszText = tmp;
            TreeView_InsertItem(HtreeView, &TvItem);
          }
          TreeView_Expand(HtreeView,Htree,TVE_EXPAND	);

          //ajout d'info de compréhension
          if (trame->operation[0] == 0x00 && trame->operation[1] == 0x01)
          {
            _snprintf(tmp,MAX_PATH-1,"Infos : Qu'elle est l'adresse MAC de \"%d.%d.%d.%d\" répondre à %d.%d.%d.%d"
            ,trame->ip_destaddr[0],trame->ip_destaddr[1],trame->ip_destaddr[2],trame->ip_destaddr[3]
            ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]);

            TvItem.hParent = TVI_ROOT;
            TvItem.item.pszText = tmp;
            TreeView_InsertItem(HtreeView, &TvItem);
          }else if (trame->operation[0] == 0x00 && trame->operation[1] == 0x02)
          {
            _snprintf(tmp,MAX_PATH-1,"Infos : Mon adresse MAC est \"%02X:%02X:%02X:%02X:%02X:%02X\" (%d.%d.%d.%d)"
            ,trame->_h_source[0],trame->_h_source[1],trame->_h_source[2],trame->_h_source[3],trame->_h_source[4],trame->_h_source[5]
            ,trame->ip_srcaddr[0],trame->ip_srcaddr[1],trame->ip_srcaddr[2],trame->ip_srcaddr[3]);

            TvItem.hParent = TVI_ROOT;
            TvItem.item.pszText = tmp;
            TreeView_InsertItem(HtreeView, &TvItem);
          }
         }
    break;
  }
  //ReleaseMutex(hMutex_TRAME_BUFFER);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND	);


 //gestion du contenu
 //AfficherHexa(Trame_buffer[ID].buffer, Trame_buffer[ID].taille_buffer, GetDlgItem(HTrame,EDIT_DONNEES_HEXA));
}
//------------------------------------------------------------------------------
void TraitementAffichageLLDP(unsigned long int id, HANDLE HtreeView, HTREEITEM Htree)
{
  //on récupère la trame, on enlève l'entête
  LLDP_HDRM *trame;
  //WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  trame = (LLDP_HDRM *)(Trame_buffer[id].buffer+3);

  //ajout du titre item courrant
  char tmp[MAX_PATH+1] = "Body : Ethernet 802.1 : LLDP (0x88CC)\0";
  TV_INSERTSTRUCT TvItem;
  TvItem.hInsertAfter = TVI_LAST;
  ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
  TvItem.item.mask  = TVIF_TEXT;
  TvItem.hParent = Htree;
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  //le reste des infos
  _snprintf(tmp,MAX_PATH,"TLV Type : %d (Chassis Id)",(trame->TLV[0] & 254)/2);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"TLV Taille : %d",(trame->TLV[0] & 1) *256 + trame->TLV[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"Chassis Id Subtype : %d (Adresse MAC)",trame->Id);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"Adresse MAC : %02X:%02X:%02X:%02X:%02X:%02X"
  ,trame->MAC_addr[0],trame->MAC_addr[1],trame->MAC_addr[2],trame->MAC_addr[3],trame->MAC_addr[4],trame->MAC_addr[5]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"TLV Type : %d",(trame->TLVL[0] & 254)/2);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"TLV Taille : %d",(trame->TLVL[0] & 1) *256 + trame->TLVL[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"Port Id Subtype : %d",trame->Port_Id_subtype);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"Port Id : %c%c%c%c",trame->Port_Id[0],trame->Port_Id[1],trame->Port_Id[2],trame->Port_Id[3]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"TLV Type : %d (Time to Live)",(trame->TTL[0] & 254)/2);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"TLV Taille : %d",(trame->TTL[0] & 1) *256 + trame->TTL[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"Secondes: %d",trame->TTL[2] *256 + trame->TTL[3]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"TLV Type : %d (End of LLDPDU)",(trame->End[0] & 254)/2);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"TLV Taille : %d",(trame->End[0] & 1) *256 + trame->End[1]);

  //ReleaseMutex(hMutex_TRAME_BUFFER);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  TreeView_Expand(HtreeView,Htree,TVE_EXPAND	);
}
//------------------------------------------------------------------------------
void TraitementAffichageSTP(unsigned long int id, HANDLE HtreeView, HTREEITEM Htree)
{
  //on récupère la trame, on enlève l'entête
  STP_HDRM *trame;
  //WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  trame = (STP_HDRM *)(Trame_buffer[id].buffer+3);

  //ajout du titre item courrant
  char tmp[MAX_PATH+1] = "LLC : Logical-Link Control\0";
  TV_INSERTSTRUCT TvItem;
  TvItem.hInsertAfter = TVI_LAST;
  ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
  TvItem.item.mask  = TVIF_TEXT;
  TvItem.hParent = Htree;
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  //le reste des infos
  _snprintf(tmp,MAX_PATH,"DSAP: Spanning Tree BPDU (Bridge Protocol Data Units) : 0x%02X",trame->DSAP_ST_BPDU);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"SSAP: Spanning Tree BPDU (Bridge Protocol Data Units) : 0x%02X",trame->SSAP_ST_BPDU);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  _snprintf(tmp,MAX_PATH,"Champ de contrôle : 0x%02X",trame->Control_field);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND	);

  //passage du protocole STP:
  TvItem.hParent = TVI_ROOT;
  strcpy(tmp,"Spanning Tree Protocol\0");
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  _snprintf(tmp,MAX_PATH,"Protocol Identifier : %d (0x%02X%02X)",trame->Proto_Id[0]*256+trame->Proto_Id[1],trame->Proto_Id[0],trame->Proto_Id[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Version du protocole : %d ",trame->Proto_Version);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  switch(trame->BPDU_Type)
  {
    case 0x02 : _snprintf(tmp,MAX_PATH,"BPDU Type : 0x%02X (Rapid/Multiple Spanning Tree)",trame->BPDU_Type);break;
    default : _snprintf(tmp,MAX_PATH,"Version du protocole : 0x%02X ",trame->BPDU_Type);
  }
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"BPDU flags : 0x%02X (",trame->BPDU_Flag);
  //traiter les flags
  if ((trame->BPDU_Flag & 128)/128)strncat(tmp,"Topology Change Acknowledgment : Oui,",MAX_PATH);
  else strncat(tmp,"Topology Change Acknowledgment : Non,",MAX_PATH);
  if ((trame->BPDU_Flag & 64)/64)strncat(tmp," Agreement : Oui,",MAX_PATH);
  else strncat(tmp," Agreement : Non,",MAX_PATH);
  if ((trame->BPDU_Flag & 32)/32)strncat(tmp," Forwarding : Oui,",MAX_PATH);
  else strncat(tmp," Forwarding : Non,",MAX_PATH);
  if ((trame->BPDU_Flag & 16)/16)strncat(tmp," Learning : Oui,",MAX_PATH);
  else strncat(tmp," Learning : Non,",MAX_PATH);

  char tmp2[MAX_PATH+1];
  _snprintf(tmp2,MAX_PATH," Port rôle : (binaire : %d%d),",(trame->BPDU_Flag&8)/8,(trame->BPDU_Flag&4)/4);
  strncat(tmp,tmp2,MAX_PATH);

  if ((trame->BPDU_Flag & 2)/2)strncat(tmp," Proposal : Oui,",MAX_PATH);
  else strncat(tmp," Proposal : Non,",MAX_PATH);
  if ((trame->BPDU_Flag & 1))strncat(tmp," Topology Change : Oui)\0",MAX_PATH);
  else strncat(tmp," Topology Change : Non)\0",MAX_PATH);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Root ID : 0x%02X%02X (Adresse MAC : %02X:%02X:%02X:%02X:%02X:%02X",trame->ROOT_ID[0],trame->ROOT_ID[1]
  ,trame->ROOT_ID[2],trame->ROOT_ID[3],trame->ROOT_ID[4],trame->ROOT_ID[5],trame->ROOT_ID[6],trame->ROOT_ID[7]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Root Cost : 0x%02X%02X%02X%02X",trame->ROOT_Cost[0],trame->ROOT_Cost[1],trame->ROOT_Cost[2],trame->ROOT_Cost[3]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Bridge ID : 0x%02X%02X (Adresse MAC : %02X:%02X:%02X:%02X:%02X:%02X",trame->Bridge_ID[0],trame->Bridge_ID[1]
  ,trame->Bridge_ID[2],trame->Bridge_ID[3],trame->Bridge_ID[4],trame->Bridge_ID[5],trame->Bridge_ID[6],trame->Bridge_ID[7]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Port : 0x%02X%02X",trame->Port[0],trame->Port[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Age : %d (0x%02X%02X)",trame->Age[1]*256+trame->Age[0],trame->Age[0],trame->Age[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Age Maximum : %d (0x%02X%02X)",trame->Age_Max[1]*256+trame->Age_Max[0],trame->Age_Max[0],trame->Age_Max[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Hello Time : %d (0x%02X%02X)",trame->Hello_Time[1]*256+trame->Hello_Time[0],trame->Hello_Time[0],trame->Hello_Time[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Forward Time : %d (0x%02X%02X)",trame->Forward_Time[1]*256+trame->Forward_Time[0],trame->Forward_Time[0],trame->Forward_Time[1]);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  _snprintf(tmp,MAX_PATH,"Taille de la version 1 : %d (0x%02X)",trame->Version,trame->Version);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  TreeView_Expand(HtreeView,Htree,TVE_EXPAND	);

  //les données
  TvItem.hParent = TVI_ROOT;
  strcpy(tmp,"Données");
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X   |  %c%c%c%c%c%c%c%c%c%c"
  ,trame->Trailer[0]&255,trame->Trailer[1]&255,trame->Trailer[2]&255,trame->Trailer[3]&255,trame->Trailer[4]&255,trame->Trailer[5]&255
  ,trame->Num_seq[0]&255,trame->Num_seq[1]&255,trame->Num_seq[2]&255,trame->Num_seq[3]&255
  ,trame->Trailer[0]&255 > 31 &&trame->Trailer[0]&255 < 127 ? trame->Trailer[0] : '.'
  ,trame->Trailer[1]&255 > 31 &&trame->Trailer[1]&255 < 127 ? trame->Trailer[1] : '.'
  ,trame->Trailer[2]&255 > 31 &&trame->Trailer[2]&255 < 127 ? trame->Trailer[2] : '.'
  ,trame->Trailer[3]&255 > 31 &&trame->Trailer[3]&255 < 127 ? trame->Trailer[3] : '.'
  ,trame->Trailer[4]&255 > 31 &&trame->Trailer[4]&255 < 127 ? trame->Trailer[4] : '.'
  ,trame->Trailer[5]&255 > 31 &&trame->Trailer[5]&255 < 127 ? trame->Trailer[5] : '.'

  ,trame->Num_seq[0]&255 > 31 &&trame->Num_seq[0]&255 < 127 ? trame->Num_seq[0] : '.'
  ,trame->Num_seq[1]&255 > 31 &&trame->Num_seq[1]&255 < 127 ? trame->Num_seq[1] : '.'
  ,trame->Num_seq[2]&255 > 31 &&trame->Num_seq[2]&255 < 127 ? trame->Num_seq[2] : '.'
  ,trame->Num_seq[3]&255 > 31 &&trame->Num_seq[3]&255 < 127 ? trame->Num_seq[3] : '.'
  );

  //ReleaseMutex(hMutex_TRAME_BUFFER);
  TvItem.item.pszText = tmp;
  TreeView_InsertItem(HtreeView, &TvItem);

  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
}
//------------------------------------------------------------------------------
void TraitementAffichageOther(unsigned long int id, HANDLE HtreeView, HTREEITEM Htree)
{
//on récupère la trame, on enlève l'entête

  //les données
  char tmp[MAX_PATH+1] = "Données\0";
  TV_INSERTSTRUCT TvItem;
  TvItem.hInsertAfter = TVI_LAST;
  ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
  TvItem.item.mask  = TVIF_TEXT;
  TvItem.hParent = Htree;
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  //traitement
  //WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  char *buffer_donnees;
  if (Trame_buffer[id].type == BOOL_TYPE_RAW)
    buffer_donnees = (char *)(Trame_buffer[id].buffer);
  else
    buffer_donnees = (char *)(Trame_buffer[id].buffer+2);

  //traitement par 16 + 16 carcatères
  char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
  unsigned short count=0,i;
  unsigned int taille = Trame_buffer[id].taille_buffer-8;

  for (i=6;i<taille;i++)
  {
    if (count == 16)
    {
      tmp2[count]=0;
      tmp3[count]=0;
      _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
      ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
      ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
      ,tmp3);
      TvItem.item.pszText = tmp;
      TreeView_InsertItem(HtreeView, &TvItem);
      count=0;
    }

    tmp2[count]=buffer_donnees[i];
    if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
    else tmp3[count]='.';

    count++;
  }
  if (count>0)
  {
    for (;count<32;count++)
    {
      tmp2[count]=0;
      tmp3[count]=0;
    }
    _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
    ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
    ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
    ,tmp3);
    TvItem.item.pszText = tmp;
    TreeView_InsertItem(HtreeView, &TvItem);
  }
  //ReleaseMutex(hMutex_TRAME_BUFFER);
  TreeView_Expand(HtreeView,Htree,TVE_EXPAND);
}
//------------------------------------------------------------------------------
//fonction de traitement du suivie de connexion TCP
void SuivieDeConnexionTCP(char *titre, long int code_filtre, unsigned long int id, BOOL local, BOOL other)
{
  //traitement des trames 1/1
  long int i=0;
  char buffer[MAX_PATH+1],buffer2[MAX_PATH+1];

  // utilisation de la fenêtre de trame
  HANDLE HrichEdit = GetDlgItem(HTrame,RE);
  ERREUR_M *buffer_donnees;

  //structure de transfert
  unsigned int taille,k;
  BOOL ok=FALSE;
  BOOL rec = FALSE;

  unsigned int port_dst = Trame_buffer[id].dst_port;
  unsigned int port_src = Trame_buffer[id].src_port;

  WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);
  if (local)
  {
    //traitement du dialogue
    for (i=0;i<NB_trame_buffer;i++)
    {
      //si bien du smtp et bien les bons ports
      if (Trame_buffer[i].protocoleChoix == code_filtre || (Trame_buffer[i].protocoleChoix == CODE_OTHER && other))
      {
        //traitement
        buffer_donnees = (ERREUR_M *)(&Trame_buffer[i].buffer[13]);

        //si les ips sont les mêmes :  non traité pour gains de performance et la faible probabilité d'erreur
        // chance : 1/(65535*65535)

        //gestion incrémentation des ports
        if (Trame_buffer[i].src_port == port_src+1 && Trame_buffer[i].dst_port == port_dst)port_src++;
        if (Trame_buffer[i].dst_port == port_dst+1 && Trame_buffer[i].src_port == port_src)port_dst++;

        if (Trame_buffer[i].src_port == port_dst+1 && Trame_buffer[i].dst_port == port_src)port_dst++;
        if (Trame_buffer[i].dst_port == port_src+1 && Trame_buffer[i].src_port == port_dst)port_src++;

        //différentiation de la question et de la réponse
        if (Trame_buffer[i].src_port  == port_src && Trame_buffer[i].dst_port  == port_dst)
        {
          //question
          _snprintf(buffer,MAX_PATH,"\r\n[%s:%d->%s:%d] %s",Trame_buffer[i].ip_src,Trame_buffer[i].src_port,Trame_buffer[i].ip_dst,Trame_buffer[i].dst_port,buffer_donnees->donnes);
          taille = strlen(buffer);
          if (taille>23)
          {
            taille=-1;
            buffer[taille]=0;
            RichEditCouleur(HrichEdit,VERT,buffer);
            if (rec)
            {
              rec=FALSE;
              RichEditCouleur(HrichEdit,VERT,"\r\n");
            }
          }
        }else if (Trame_buffer[i].src_port  == port_dst && Trame_buffer[i].dst_port  == port_src)
        {
          //réponse
          //_snprintf(buffer,MAX_PATH,"Envoyé : %s",buffer_donnees->donnes);
          _snprintf(buffer,MAX_PATH,"\r\n[%s:%d->%s:%d] %s",Trame_buffer[i].ip_src,Trame_buffer[i].src_port,Trame_buffer[i].ip_dst,Trame_buffer[i].dst_port,buffer_donnees->donnes);

          taille = strlen(buffer);
          if (taille>23)
          {
              buffer[taille-1]=0;
              RichEditCouleur(HrichEdit,ROUGE,buffer);
            if (!rec)
            {
              rec=TRUE;
              RichEditCouleur(HrichEdit,VERT,"\r\n");
            }
          }
        }
      }
    }
  }else
  {
    //traitement du dialogue
    for (i=0;i<NB_trame_buffer;i++)
    {
      //si bien du smtp et bien les bons ports
      if (Trame_buffer[i].protocoleChoix == code_filtre)
      {
        //traitement
        buffer_donnees = (ERREUR_M *)(&Trame_buffer[i].buffer[13]);

        _snprintf(buffer,MAX_PATH,"\r\n%s",buffer_donnees->donnes);
        taille = strlen(buffer);
        if (taille>0)
        {
            buffer[taille-2]=0;
            RichEditCouleur(HrichEdit,BLEU,buffer);
        }
      }
    }
  }
  ReleaseMutex(hMutex_TRAME_BUFFER);

 //changement du titre de la fenêtre
 SetWindowText(HTrame,titre);

 //une fois fini on affiche le résultat
 ShowWindow(GetDlgItem(HTrame,TREE_DONNEES), SW_HIDE);
 ShowWindow(GetDlgItem(HTrame,RE), SW_SHOW);
 ShowWindow(HTrame, SW_SHOW);

  //ajouter un popup possible pour gérer l'exportation
}
//------------------------------------------------------------------------------
//fonction de traitement du suivie de connexion intelligente
void SuivieDeConnexionIA(unsigned long int id)
{
  //traitement des trames 1/1
  long int i=0;
  char buffer[MAX_PATH+1],buffer2[MAX_PATH+1];

  // utilisation de la fenêtre de trame
  HANDLE HrichEdit = GetDlgItem(HTrame,RE);
  SetWindowText(HrichEdit,"");
  ERREUR_M *buffer_donnees;

  //structure de transfert
  unsigned int taille,k;
  BOOL ok=FALSE;
  BOOL rec = FALSE;

  WaitForSingleObject(hMutex_TRAME_BUFFER,INFINITE);


  //on test si IPV4 - TCP ou UDP
  if (Trame_buffer[id].ProtoType == 0x0800)
  {

    unsigned int port_dst = Trame_buffer[id].dst_port;
    unsigned int port_src = Trame_buffer[id].src_port;
    //traitement du dialogue
    for (i=0;i<NB_trame_buffer;i++)
    {
      //si bien du smtp et bien les bons ports
      if (Trame_buffer[i].protocoleChoix == Trame_buffer[id].protocoleChoix || (Trame_buffer[i].protocoleChoix == CODE_OTHER))
      {
        //traitement
        buffer_donnees = (ERREUR_M *)(&Trame_buffer[i].buffer[13]);

        //si les ips sont les mêmes :  non traité pour gains de performance et la faible probabilité d'erreur
        // chance : 1/(65535*65535)

        //gestion incrémentation des ports
        if (Trame_buffer[i].src_port == port_src+1 && Trame_buffer[i].dst_port == port_dst)port_src++;
        if (Trame_buffer[i].dst_port == port_dst+1 && Trame_buffer[i].src_port == port_src)port_dst++;

        if (Trame_buffer[i].src_port == port_dst+1 && Trame_buffer[i].dst_port == port_src)port_dst++;
        if (Trame_buffer[i].dst_port == port_src+1 && Trame_buffer[i].src_port == port_dst)port_src++;

        //différentiation de la question et de la réponse
        if (Trame_buffer[i].src_port  == port_src && Trame_buffer[i].dst_port  == port_dst)
        {
          //question
          _snprintf(buffer,MAX_PATH,"\r\n[%s:%d->%s:%d] %s",Trame_buffer[i].ip_src,Trame_buffer[i].src_port,Trame_buffer[i].ip_dst,Trame_buffer[i].dst_port,buffer_donnees->donnes);
          taille = strlen(buffer);
          if (taille>23)
          {
            taille=-1;
            buffer[taille]=0;
            RichEditCouleur(HrichEdit,VERT,buffer);
            if (rec)
            {
              rec=FALSE;
              RichEditCouleur(HrichEdit,VERT,"\r\n");
            }
          }
        }else if (Trame_buffer[i].src_port  == port_dst && Trame_buffer[i].dst_port  == port_src)
        {
          //réponse
          //_snprintf(buffer,MAX_PATH,"Envoyé : %s",buffer_donnees->donnes);
          _snprintf(buffer,MAX_PATH,"\r\n[%s:%d->%s:%d] %s",Trame_buffer[i].ip_src,Trame_buffer[i].src_port,Trame_buffer[i].ip_dst,Trame_buffer[i].dst_port,buffer_donnees->donnes);

          taille = strlen(buffer);
          if (taille>23)
          {
              buffer[taille-1]=0;
              RichEditCouleur(HrichEdit,ROUGE,buffer);
            if (!rec)
            {
              rec=TRUE;
              RichEditCouleur(HrichEdit,VERT,"\r\n");
            }
          }
        }
      }
    }
  }else if (Trame_buffer[id].ip_src[0]!=0)//par protoType + IP
  {
    //traitement du dialogue
    for (i=0;i<NB_trame_buffer;i++)
    {
      //si bien du smtp et bien les bons ports
      if (Trame_buffer[i].protocoleChoix == Trame_buffer[id].protocoleChoix)
      {
        //traitement
        buffer_donnees = (ERREUR_M *)(&Trame_buffer[i].buffer[13]);

        //différentiation de la question et de la réponse
        if (!strcmp(Trame_buffer[i].ip_src,Trame_buffer[id].ip_src) && !strcmp(Trame_buffer[i].ip_dst,Trame_buffer[id].ip_dst))
        {
          _snprintf(buffer,MAX_PATH,"\r\n[%s->%s] %s",Trame_buffer[i].ip_src,Trame_buffer[i].ip_dst,buffer_donnees->donnes);
          RichEditCouleur(HrichEdit,VERT,buffer);
          if (rec)
          {
            rec=FALSE;
            RichEditCouleur(HrichEdit,VERT,"\r\n");
          }
        }else if (!strcmp(Trame_buffer[i].ip_dst,Trame_buffer[id].ip_src) && !strcmp(Trame_buffer[i].ip_src,Trame_buffer[id].ip_dst))
        {
          _snprintf(buffer,MAX_PATH,"\r\n[%s->%s] %s",Trame_buffer[i].ip_src,Trame_buffer[i].ip_dst,buffer_donnees->donnes);
          RichEditCouleur(HrichEdit,ROUGE,buffer);
          if (!rec)
          {
            rec=TRUE;
            RichEditCouleur(HrichEdit,ROUGE,"\r\n");
          }
        }
      }
    }
  }else //par protoType seul
  {
    //traitement du dialogue
    for (i=0;i<NB_trame_buffer;i++)
    {
      //si bien du smtp et bien les bons ports
      if (Trame_buffer[i].protocoleChoix == Trame_buffer[id].protocoleChoix)
      {
        //traitement
        buffer_donnees = (ERREUR_M *)(&Trame_buffer[i].buffer[13]);
        _snprintf(buffer,MAX_PATH,"\r\n[%s->%s] %s",Trame_buffer[i].ip_src,Trame_buffer[i].ip_dst,buffer_donnees->donnes);
        RichEditCouleur(HrichEdit,BLEU,buffer);
      }
    }
  }

  ReleaseMutex(hMutex_TRAME_BUFFER);

  //changement du titre de la fenêtre
  SetWindowText(HTrame,"Reconstitution");

  //une fois fini on affiche le résultat
  ShowWindow(GetDlgItem(HTrame,TREE_DONNEES), SW_HIDE);
  ShowWindow(GetDlgItem(HTrame,RE), SW_SHOW);
  ShowWindow(HTrame, SW_SHOW);
  //ajouter un popup possible pour gérer l'exportation
}
//------------------------------------------------------------------------------
void TraitementTrameAllHexa(unsigned long int id, HANDLE HtreeView, HTREEITEM Htree)
{
  //on ajout un item maître
  char tmp[MAX_PATH+1] = "Trame entière (hexa)\0";
  TV_INSERTSTRUCT TvItem;
  TvItem.hInsertAfter = TVI_LAST;
  ZeroMemory(&(TvItem.item), sizeof(TV_ITEM));
  TvItem.item.mask  = TVIF_TEXT;
  TvItem.hParent = Htree;
  TvItem.item.pszText = tmp;
  Htree = TreeView_InsertItem(HtreeView, &TvItem);
  TvItem.hParent = Htree;

  //création de chacune des lignes hexa et ajout
  char *buffer_donnees;
  if (Trame_buffer[id].type == BOOL_TYPE_RAW)
    buffer_donnees = (char *)(Trame_buffer[id].buffer);
  else
    buffer_donnees = (char *)(Trame_buffer[id].buffer);

  //traitement par 16 + 16 carcatères
  char tmp2[MAX_PATH+1],tmp3[MAX_PATH+1];
  unsigned short count=0,i;
  unsigned int taille = Trame_buffer[id].taille_buffer;

  //toutes les lignes pleines en premières
  for (i=0;i<taille;i++)
  {
    if (count == 16)
    {
      tmp2[count]=0;
      tmp3[count]=0;
      _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
      ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
      ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
      ,tmp3);
      TvItem.item.pszText = tmp;
      TreeView_InsertItem(HtreeView, &TvItem);
      count=0;
    }

    tmp2[count]=buffer_donnees[i];
    if (buffer_donnees[i]>31 && buffer_donnees[i]<127)tmp3[count]=buffer_donnees[i];
    else tmp3[count]='.';

    count++;
  }

  //puis le reste
  if (count>0)
  {
    for (;count<32;count++)
    {
      tmp2[count]=0;
      tmp3[count]=0;
    }
    _snprintf(tmp,MAX_PATH,"%02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  %02X %02X %02X %02X  |  %s"
    ,tmp2[0]&255,tmp2[1]&255,tmp2[2]&255,tmp2[3]&255,tmp2[4]&255,tmp2[5]&255,tmp2[6]&255,tmp2[7]&255
    ,tmp2[8]&255,tmp2[9]&255,tmp2[10]&255,tmp2[11]&255,tmp2[12]&255,tmp2[13]&255,tmp2[14]&255,tmp2[15]&255
    ,tmp3);
    TvItem.item.pszText = tmp;
    TreeView_InsertItem(HtreeView, &TvItem);
  }
}
