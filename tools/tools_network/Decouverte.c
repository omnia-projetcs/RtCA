//------------------------------------------------------------------------------
// Projet SCP-DB        : Scan et tests de réseau
// Auteur               : Hanteville Nicolas
// Site                 : http:\\omni.a.free.fr
// Version              : 0.1
// Date de modification : 06/01/2010
// Description          : fonction de scanne ARP + DNS + PING
// Environnement        : compatiblité DEVCPP / VISUAL C++ / BORLAND C++ 5.x
//------------------------------------------------------------------------------
#include "_ressources.h"
//------------------------------------------------------------------------------
// disco spécifique pour une ligne donnée pour la liste view de sniff
DWORD  WINAPI Sniff_infos(LPVOID lParam)
{
  unsigned long int ligne = (unsigned long int)lParam;

  //lecture de l'ip de la ligne (si pas déja remplis les infos)
  char ip[TMP_DEFAULT+1]="",mac[MAX_PATH+1]="", TtL[TMP_DEFAULT+1]="";
  char tmpIp[MAX_PATH+1]="";
  HANDLE hListView = GetDlgItem(Tabl[TAB_SNIFF],LSTV);

  char ctimeout[MAX_PATH+1];
  GetWindowText(GetDlgItem(Tabl[TAB_CONF],TOM_ICMP),ctimeout,MAX_PATH);
  time_out = atoi(ctimeout);

  ListView_GetItemText(hListView,ligne,COL_IP_SNIFF,tmpIp,TMP_DEFAULT);
  if (strlen(tmpIp)<=17)//ok
  {
    strcpy(ip,tmpIp);

    //résolution DNS
    //Reset_Netbios();
    struct hostent* remoteHost;
    struct in_addr in;
    in.s_addr = inet_addr(ip);
    if ((remoteHost=gethostbyaddr((char *)&in, 4, AF_INET))!=0)
    {
      strncat(ip," (",TMP_DEFAULT);
      strncat(ip,remoteHost->h_name,TMP_DEFAULT);
      strncat(ip,")\0",TMP_DEFAULT);
    }

    //résolution ARP ou requête ARP si n'existe pas
    ListView_GetItemText(hListView,ligne,COL_MAC_SNIFF,mac,TMP_DEFAULT);
    if (strlen(mac) == 17) //ok
    {
      WaitForSingleObject(hMutex,INFINITE);
      ReadMacAdressConstructeur(mac);
      ReleaseMutex(hMutex);
    }else if (strlen(mac) < 17)
    {
      mac[0]=0;
      RecupMacAdressEtConstructeur(ip,mac,NULL,1);
    }

    //TTL
    HANDLE myhdnl = pIcmpCreateFile();
    if (myhdnl != INVALID_HANDLE_VALUE)
    {
      LPHOSTENT pHost = gethostbyname(tmpIp);
      DWORD *dwAddress;
      ICMPECHO icmpEcho;
      dwAddress = (DWORD *)(*pHost->h_addr_list);
      pIcmpSendEcho(myhdnl,*dwAddress,0,0,0,&icmpEcho,sizeof(icmpEcho),time_out);

      //PING
      if ((icmpEcho.Status==0)&&(icmpEcho.Options.Ttl>0))
      {
        //ttl
        _snprintf(TtL,TMP_DEFAULT,"TTL:%d",icmpEcho.Options.Ttl);

        //OS
        if (icmpEcho.Options.Ttl<=MACH_LINUX)strncat(TtL," Linux\0",TMP_DEFAULT);
        else if (icmpEcho.Options.Ttl<=MACH_WINDOWS)strncat(TtL," Windows\0",TMP_DEFAULT);
        else strncat(TtL," Routeur\0",TMP_DEFAULT);

      }else strcpy(TtL,"FIREWALL\0");
    }
    pIcmpCloseHandle(myhdnl);

    //on applique le résultat
    ListView_SetItemText(hListView,ligne,COL_IP_SNIFF,ip);
    ListView_SetItemText(hListView,ligne,COL_MAC_SNIFF,mac);
    ListView_SetItemText(hListView,ligne,COL_TTLOS,TtL);
  }

  ReleaseSemaphore(hSemaphoreSniff,1,NULL);
  return 0;
}
//------------------------------------------------------------------------------
DWORD  WINAPI Sniff_infos_ALL(LPVOID lParam)
{
  test_SNIFF = 1;
  InitReseau(0);
  //Reset_Netbios();

  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),FALSE);

  hMutex = CreateMutex(0,FALSE,"mutex");

  //chargement des fonction de la DLL ICMP pour le TTL ^^
  if ((hndlIcmp = LoadLibrary("ICMP.DLL"))!=0)
  {
    //chargement des fonctions
    pIcmpCreateFile  = (HANDLE (WINAPI *)(void))GetProcAddress((HMODULE)hndlIcmp,"IcmpCreateFile");
    pIcmpCloseHandle = (BOOL (WINAPI *)(HANDLE))GetProcAddress((HMODULE)hndlIcmp,"IcmpCloseHandle");
    pIcmpSendEcho = (DWORD (WINAPI *)(HANDLE,DWORD,LPVOID,WORD,PIPINFO,LPVOID,DWORD,DWORD))	GetProcAddress((HMODULE)hndlIcmp,"IcmpSendEcho");
    pIcmpSendEcho2 = (DWORD (WINAPI *)(HANDLE,HANDLE,PIO_APC_ROUTINE,PVOID,IPAddr,LPVOID,WORD,PIP_OPTION_INFORMATION,LPVOID,DWORD,DWORD))	GetProcAddress((HMODULE)hndlIcmp,"IcmpSendEcho2");

    if (pIcmpCreateFile!=0 && pIcmpCloseHandle!=0 && pIcmpSendEcho!=0 && pIcmpSendEcho2)
    {
      if (Ficbuffer!=0)
      {
        if (lParam == (LPVOID)65535) //toutes les machines de la liste view
        {
          char _mytmp_num_thread[MAX_PATH+1];
          GetWindowText(GetDlgItem(Tabl[TAB_CONF],EDIT_MAX_THREAD),_mytmp_num_thread,MAX_PATH);
          nb_max_thread = atoi(_mytmp_num_thread);

          if (nb_max_thread<NB_MIN_THREAD)
            nb_max_thread = NB_MIN_THREAD;
          else if (nb_max_thread>NB_MAX_THREAD)
            nb_max_thread = NB_MAX_THREAD;

          hSemaphoreSniff=CreateSemaphore(NULL,nb_max_thread,nb_max_thread,NULL);
          unsigned long int i,nb = ListView_GetItemCount(GetDlgItem(Tabl[TAB_SNIFF],LSTV));
          for (i=0;i<nb;i++)
          {
            WaitForSingleObject(hSemaphoreSniff,INFINITE);
            CreateThread(NULL,0,Sniff_infos,(PVOID)i,0,0);
          }

          for(i=0;i<nb_max_thread;i++)WaitForSingleObject(hSemaphoreSniff,INFINITE);
        }else //une machine spécifique
        {
          hSemaphoreSniff=CreateSemaphore(NULL,1,1,NULL);
          WaitForSingleObject(hSemaphoreSniff,INFINITE);

          CreateThread(NULL,0,Sniff_infos,(PVOID)lParam,0,0);
          WaitForSingleObject(hSemaphoreSniff,INFINITE);
        }
      }
    }
    FreeLibrary((HMODULE)hndlIcmp);
  }
  CloseHandle(hSemaphoreSniff);

  WaitForSingleObject(hMutex,INFINITE);
  ReleaseMutex(hMutex);
  CloseHandle(hMutex);

  EnableWindow(GetDlgItem(Tabl[TAB_SNIFF],LSTV),TRUE);
  //LocalFree(Ficbuffer);
  //Ficbuffer = 0;
  ReInitReseau(0);
  test_SNIFF = 0;
}

//------------------------------------------------------------------------------
DWORD  WINAPI Disco(LPVOID lParam)
{
  //récupération de l'ip
  char Ip[17]="\0";

  char txtMac[MAX_PATH]="\0";
  char txtPromiscous[MAX_PATH]="\0";
  char txtTTL[20]="\0";
  char Mac_seul[TAILLE_MAC];
  char txtOS[TMP_DEFAULT+1]="\0";
  char txtDNS[TMP_DEFAULT+1]="\0";

  BOOL Firewall=1;
  BOOL re_ARP = 0;
  BOOL ARP_existe = FALSE;
  strcpy(Ip,(char*)lParam);
  ReleaseSemaphore(hSemaphoreIp,1,NULL);

    //PING
    //unsigned int My_TTL = Ping_simple(Ip);
    unsigned int My_TTL = DLLPing_simple(Ip);
    //if (My_TTL==0)My_TTL = Ping_simple(Ip);

    if (My_TTL)
    {
      Firewall = 0;
      sprintf(txtTTL,"TTL:%d",My_TTL);

      //OS
      if (My_TTL<=MACH_LINUX)strcpy(txtOS,"Linux\0");
      else if (My_TTL<=MACH_WINDOWS)strcpy(txtOS,"Windows\0");
      else strcpy(txtOS,"Routeur\0");
    }

    //ARP
    if (RecupMacAdressEtConstructeur(Ip,txtMac,Mac_seul,SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP, BM_GETCHECK,(WPARAM) 0, (LPARAM)0)) && test_DISCO)
    {
      if (Firewall)
      {
        strcpy(txtOS,"FIREWALL\0");
        Firewall = 0;
      }
      re_ARP = 1;
      ARP_existe = TRUE;

      //tests promiscous
      if (Promiscuous_param)
      {
         //on commence par définir l'ip du test
         //on vérifi si l'ip termine en .1 si c'est le cas on prend .2 comme référence sinon on prend .1
         unsigned short taille_Ip = strlen(Ip)-1;

         //test pour voir si l'on ne traite pas notre propre machine en adresse virtuelle
         if (!strcmp(Mac_seul,Mac_Promiscuous) && Ip[taille_Ip-1] == '.' && (Ip[taille_Ip] == '1' || Ip[taille_Ip] == '2'))
         {
          // on ne doit pas tester la machine
          ReleaseSemaphore(hSemaphore,1,NULL);
          return 0;
         }
      }
    }

    //DNS
    if (SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_DNS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0) && test_DISCO)
    {
      INIT_connnection_NETBIOS(Ip);

      struct hostent* remoteHost;
      struct in_addr in;
      in.s_addr = inet_addr(Ip);
      if ((remoteHost=gethostbyaddr((char *)&in, 4, AF_INET))!=0)
      {
          strncpy(txtDNS,remoteHost->h_name,256);
          if (Firewall)
          {
            strcpy(txtOS,"FIREWALL\0");
            Firewall = 0;
          }

          //NETBIOS (test pour récupération de l'OS)
          ResolutionOS_Netbios(Ip, txtOS, TMP_DEFAULT);

          /*wchar_t serveur[MAX_PATH];
          char tmp[MAX_PATH];
          _snprintf(tmp,MAX_PATH,"\\\\%s",Ip);
          //init de la chaine (pour connexion à la machine)
          mbstowcs( serveur,tmp,MAX_PATH);

          //lecture des informations NETBIOS
          WKSTA_INFO_100 *mybuff;
          NET_API_STATUS res = NetWkstaGetInfo(serveur, 100,(PVOID)&mybuff);

          if((res == ERROR_SUCCESS || res == ERROR_MORE_DATA) && mybuff)
          {
            //on test le type d'os et on met a jour
            switch(mybuff->wki100_ver_major)
            {
              case 4:
                switch (mybuff->wki100_ver_minor)
                {
                    case 0:strcpy(txtOS,"Windows 95/NT4");break;
                    case 10:strcpy(txtOS,"Windows 98");break;//
                    case 90:strcpy(txtOS,"Windows ME");break;
                }
              break;
              case 5:
                switch (mybuff->wki100_ver_minor)
                {
                    case 0:strcpy(txtOS,"Windows 2K");break;
                    case 1:strcpy(txtOS,"Windows XP");break;//
                    case 2:strcpy(txtOS,"Windows 2003");break;
                }
              break;
              case 6:
                switch (mybuff->wki100_ver_minor)
                {
                    case 0:strcpy(txtOS,"Windows Vista\n");break;
                    case 1:strcpy(txtOS,"Windows 2008\n");break;
                }
              break;
              case 7:
                switch (mybuff->wki100_ver_minor)
                {
                    case 0:strcpy(txtOS,"Windows 7\n");break;
                }
              break;
              default:
                    _snprintf(txtOS,TMP_DEFAULT,"Windows [major:%d;minor:%d]",mybuff->wki100_ver_major,mybuff->wki100_ver_minor);
              break;
            }
          }
          NetApiBufferFree(mybuff);*/

          //test si null session possible
          if (NullSession_Netbios(Ip))
            strncat(txtOS," [NULL SESSION]\0",TAILLE_TMP);
          /*
          _snprintf(tmp,TAILLE_TMP,"\\\\%s\\ipc$",Ip);

          NETRESOURCE NetRes;
          NetRes.dwType	= RESOURCETYPE_ANY;
          NetRes.lpLocalName ="";
          NetRes.lpRemoteName	= tmp;
          NetRes.lpProvider ="";
          if (WNetAddConnection2(&NetRes,"","",0)==NO_ERROR)//null session possible
            strncat(txtOS," [NULL SESSION]\0",TAILLE_TMP);
            */



          //récupération de l'adresse MAC par netbios si récupération même si non réussi
          //permet de vérifier si l'adresse n'est pas l'adresse d'un cache ARP
          NCB my_ncb,ncb;
          ADAPTER_STATUS astat;
          UCHAR i,len,j;
          char tmpMac[MAX_PATH];
          char tmp[MAX_PATH];

          for (j=0;j<10 && !re_ARP;j++)// a revoir suivant la sélection de la carte réseau
          {
            my_ncb.ncb_command=NCBASTAT; // ADAPTER STATUS
            my_ncb.ncb_buffer=(void far *) &astat; // point to return buffer
            my_ncb.ncb_length=sizeof(astat); // return buffer length
            my_ncb.ncb_rto=50; // receive time out (500 ms unit)
            my_ncb.ncb_sto=50; // send time out (500 ms unit)
            my_ncb.ncb_lana_num=0; // default LANA number
            len=strlen(txtDNS);
            strcpy(my_ncb.ncb_callname,txtDNS); // get NETBIOS name

            for (i=len;i<sizeof(my_ncb.ncb_callname);i++)
            my_ncb.ncb_callname[i]=' '; // pad NETBIOS name with spaces
            my_ncb.ncb_lana_num=j; // set default LANA number (j : 0-4)

            ncb.ncb_command = NCBRESET;
            ncb.ncb_lsn=0;
            ncb.ncb_lana_num=my_ncb.ncb_lana_num;
            ncb.ncb_callname[0]=20;
            ncb.ncb_callname[2]=20;
            Netbios( &ncb );
            Netbios(&my_ncb);

            if ((my_ncb.ncb_retcode==NRC_GOODRET)||(my_ncb.ncb_retcode==NRC_INCOMP))
            {
              #ifdef DEBUG_DISCO
                printf("DEBUG - DISCO : carte id:%d\n",j);
              #endif

              _snprintf (tmpMac,25,"%02X:%02X:%02X:%02X:%02X:%02X\0"
              ,astat.adapter_address[0]
              ,astat.adapter_address[1]
              ,astat.adapter_address[2]
              ,astat.adapter_address[3]
              ,astat.adapter_address[4]
              ,astat.adapter_address[5]);

              //pour éviter une comparaison de la zone de texte en + de l'adress MAC s'il y a
              for (i=0;i<18;i++)tmp[i] = txtMac[i];
              tmp[18] =0;

              //si les adresses ne sont pas identiques le NETBIOS est plus fiable!
              if (strcmp(tmpMac,tmp)!=0 && strcmp(tmpMac,"00:00:00:00:00:00")!=0 && strcmp(tmpMac,"FF:FF:FF:FF:FF:FF")!=0)
              {
                strcpy(txtMac,tmpMac);
                if (SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                {
                  WaitForSingleObject(hMutex,INFINITE);
                  ReadMacAdressConstructeur(txtMac);
                  ReleaseMutex(hMutex);
                }
              }
              re_ARP = 1;
            }else if (my_ncb.ncb_retcode==NRC_CMDTMO) re_ARP = 1; // command time out
            else if (my_ncb.ncb_retcode==NRC_BRIDGE){}
            else re_ARP = 1;
          }
      }
    }
  ReleaseSemaphore(hSemaphore,1,NULL);

  //on écrit le résultat dans le listeview
  if (!Firewall) //si une machine est détectée
  {
    WaitForSingleObject(hMutexLSTV,INFINITE);

    long int itemPos;
    LVITEM lvi;
    HANDLE hListView = GetDlgItem(Tabl[TAB_DISCO],LSTV);

    //ajout d'une ligne
    lvi.mask = LVIF_TEXT|LVIF_PARAM;
    lvi.iSubItem = 0;
    lvi.lParam = LVM_SORTITEMS;
    lvi.pszText="";
    lvi.iItem = ListView_GetItemCount(hListView);
    itemPos = ListView_InsertItem(hListView, &lvi);

    ListView_SetItemText(hListView,itemPos,COL_IP,Ip);
    ListView_SetItemText(hListView,itemPos,COL_MAC,txtMac);
    ListView_SetItemText(hListView,itemPos,COL_TTL,txtTTL);
    ListView_SetItemText(hListView,itemPos,COL_NOM,txtDNS);
    ListView_SetItemText(hListView,itemPos,COL_OS,txtOS);
    //ListView_SetItemText(hListView,itemPos,COL_PROMISCUOUS,txtPromiscous);
    ReleaseMutex(hMutexLSTV);

    //tests promiscous
    if (ARP_existe && Promiscuous_param && test_DISCO)
    {
       //on commence par définir l'ip du test
       //on vérifi si l'ip termine en .1 si c'est le cas on prend .2 comme référence sinon on prend .1
       unsigned short taille_Ip = strlen(Ip)-1;
       char Ip_test[16];

       if (Ip[taille_Ip] == '1' && Ip[taille_Ip-1] == '.')
       {
         strcpy(Ip_test,Ip);
         Ip_test[taille_Ip] = '2';
       }else
       {
         strcpy(Ip_test,Ip_ref_Promiscous);
       }

        //ouverture de l'interface réseau
        pcap_t *fp;
        char errbuf[PCAP_ERRBUF_SIZE];

        WaitForSingleObject(hMutexPromiscuousTest,INFINITE);
        short id_tmp = SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0);

        int promiscous = 0;
        if(SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_LOCAL_MAC_PROMISCOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0) == FALSE)
          promiscous = 1;

        if ((fp = MYpcap_open_live(If_Dev[id_tmp].description,65536,promiscous,1000,errbuf)) != NULL)
        {
          if (!promiscous)
          {
            if (!strcmp(Ip,If_Dev[id_tmp].ip))
            {
              id_tmp = -1;
            }else strcpy(Ip_test,If_Dev[id_tmp].ip);
          }else
          {
            CreerPaquetArp_test_str(Ip_test,Ip_test, Mac_Promiscuous, "FF:FF:FF:FF:FF:FF", 0,fp,promiscous);
            CreerPaquetArp_test_str(Ip_test,Ip_test, Mac_Promiscuous, "FF:FF:FF:FF:FF:FF", 0,fp,promiscous);
            CreerPaquetArp_test_str(Ip_test,Ip_test, Mac_Promiscuous, "FF:FF:FF:FF:FF:FF", 0,fp,promiscous);
          }

          if (id_tmp!=-1)
          {
              //les différents tests
              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B47, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "FF:FF:FF:FF:FF:FE", fp,promiscous))
                   strcat(txtPromiscous,"B47, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B24, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "FF:FF:FF:00:00:00", fp,promiscous))
                   strcat(txtPromiscous,"B24, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B16, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "FF:FF:00:00:00:00", fp,promiscous))
                   strcat(txtPromiscous,"B16, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B8, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "FF:00:00:00:00:00", fp,promiscous))
                   strcat(txtPromiscous,"B8, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B1, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "01:00:00:00:00:00", fp,promiscous))
                   strcat(txtPromiscous,"B1, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M0, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "01:00:5E:00:00:00", fp,promiscous))
                   strcat(txtPromiscous,"M0, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M1, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "01:00:5E:00:00:01", fp,promiscous))
                   strcat(txtPromiscous,"M1, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M2, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "01:00:5E:00:00:02", fp,promiscous))
                   strcat(txtPromiscous,"M2, ");

              if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M3, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
                if (TestRepondArp(Ip_test, Mac_Promiscuous, Ip, "01:00:5E:00:00:03", fp,promiscous))
                   strcat(txtPromiscous,"M3");
          }
          strcat(txtPromiscous,"\0");
        }
        MYpcap_close(fp);
        ReleaseMutex(hMutexPromiscuousTest);

        WaitForSingleObject(hMutexLSTV,INFINITE);
        ListView_SetItemText(hListView,itemPos,COL_PROMISCUOUS,txtPromiscous);
        ReleaseMutex(hMutexLSTV);
    }
  }
  Journalisation_EcrireFichier(Ip, NULL,"DISCO - FIN des tests!");
  return 0;
}

//------------------------------------------------------------------------------
DWORD  WINAPI DiscoScan(LPVOID lParam)
{
  #ifdef DEBUG_TIME_TEST
  int time_calc_debut = GetTickCount();
  #endif

  DWORD LIp1,LIp2;
  BYTE L11,L12,L13,L14,L21,L22,L23,L24;
  char Ip[17];
  int a,b,c,d, initb,initc,initd,finb,finc,find;

  MiseEnGras(GetDlgItem(HPrincipal,BT_DISCO),TRUE);

  //maj
  //routage_route = LirePasserelle(routage_route_mac,routage_route_ip);

  test_DISCO = 1;

  //gestion du promiscous
  if (PCAP_OK) Promiscuous_param = SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_PROMISCUOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0);
  else Promiscuous_param = 0;

  char _mytmp_num_thread[MAX_PATH+1];
  GetWindowText(GetDlgItem(Tabl[TAB_CONF],EDIT_MAX_THREAD),_mytmp_num_thread,MAX_PATH);
  nb_max_thread = atoi(_mytmp_num_thread);

  if (nb_max_thread<NB_MIN_THREAD)
    nb_max_thread = NB_MIN_THREAD;
  else if (nb_max_thread>NB_MAX_THREAD)
    nb_max_thread = NB_MAX_THREAD;

  //si l'ICMP est bien accessible ^^
  if (IcmpOk)
  {
    char ctimeout[MAX_PATH+1];
    GetWindowText(GetDlgItem(Tabl[TAB_CONF],TOM_ICMP),ctimeout,MAX_PATH);
    time_out = atoi(ctimeout);

    //récupération des 2 IPS
    SendDlgItemMessage(Tabl[TAB_DISCO],IP, IPM_GETADDRESS,(WPARAM) 0, (LPARAM)&LIp1);
    SendDlgItemMessage(Tabl[TAB_DISCO],IP2, IPM_GETADDRESS,(WPARAM) 0, (LPARAM)&LIp2);

    //init
    long int NB_Machines_total = 0;
    long int point = 0;

    //on récupère chacun des octet
    L11 = LIp1 >> 24;
    L12 = (LIp1 >> 16) & 0xFF;
    L13 = (LIp1 >> 8) & 0xFF;
    L14 = LIp1 & 0xFF;

    L21 = LIp2 >> 24;
    L22 = (LIp2 >> 16) & 0xFF;
    L23 = (LIp2 >> 8) & 0xFF;
    L24 = LIp2 & 0xFF;

    //init du réseau
    InitReseau(0);
    //Reset_Netbios();

    //désactiver les boutons
    SetWindowText(GetDlgItem(Tabl[TAB_DISCO],BT_START),Language->arreter);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],LSTV),FALSE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP),FALSE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_DNS),FALSE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_PROMISCUOUS),FALSE);

    //gestion du promiscous création de l'ip racine pour déterminer l'adresse usurpée
    BOOL promiscous_test_ip_src = FALSE;
    if (Promiscuous_param)
    {
      if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_LOCAL_MAC_PROMISCOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
      {
        unsigned short id_tmp = SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0);
        strcpy(Ip_ref_Promiscous,If_Dev[id_tmp].ip);
        strcpy(Mac_Promiscuous,If_Dev[id_tmp].mac);
      }else
      {
          _snprintf(Ip_ref_Promiscous,16,"%d.%d.%d.1",L11,L12,L13);
          //récupération de l'adresse MAC qui va être utilisé pour faire le test promiscuous
          unsigned short k=0;
          GetWindowText(GetDlgItem(Tabl[TAB_CONF],EDIT_MAC_PROM),Mac_Promiscuous,TAILLE_MAC);

          //et vérification de son format
          for (k=0;k<18;k++)
          {
            if (Mac_Promiscuous[k]>58 && Mac_Promiscuous[k]<65 || Mac_Promiscuous[k]>70 && Mac_Promiscuous[k]<97 || Mac_Promiscuous[k]>102 || Mac_Promiscuous[k]<48)
            {
              strcpy(Mac_Promiscuous,"00:11:22:33:44:55");
              SetWindowText(GetDlgItem(Tabl[TAB_CONF],EDIT_MAC_PROM),Mac_Promiscuous);
              break;
            }
          }
      }
    }

    //test de la validité des IPS
    if (LIp1!=0 && L14 >0 && L14 < 255 && L13 <255 && L12 <255 && L11<255 || LIp2!=0 && L24 >0 && L24 < 255 && L23 <255 && L22 <255 && L21<255)
    {
       hSemaphore=CreateSemaphore(NULL,nb_max_thread,nb_max_thread,NULL); //liste des scan simulatnéesS
       hSemaphoreIp=CreateSemaphore(NULL,1,1,NULL); //récupération de l'adresse IP

       //chargement du fichier des adresse MAC
       if (SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
       {
         if (Ficbuffer!=0)
         {
           hMutex = CreateMutex(0,FALSE,"mutex"); // access au fichier des adresses MAC (si arp résolution activé)
         }else
         {
           //on décoche la case car problème
           SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP, BM_SETCHECK,(WPARAM)BST_UNCHECKED, (LPARAM)0);
         }

       }
       hMutexLSTV = CreateMutex(0,FALSE,"mutexLSTV"); // ajout d'un item à la liste view

       hMutexPromiscuousTest = CreateMutex(0,FALSE,"mutexPROMISCOUS"); // test promiscuous

       HANDLE hListView = GetDlgItem(Tabl[TAB_DISCO],LSTV);
       ListView_DeleteAllItems(hListView);

       HANDLE ProgressBAR = GetDlgItem(Tabl[TAB_DISCO],ARP_PB);
       SendMessage(ProgressBAR, PBM_SETPOS, (WPARAM)0, 0);

       if ((LIp1 == 0 && L21>0) || LIp1 == LIp2)
       {
         NB_Machines_total = 1;

         WaitForSingleObject(hSemaphore,INFINITE);
         WaitForSingleObject(hSemaphoreIp,INFINITE);

         _snprintf(Ip,16,"%d.%d.%d.%d",L21,L22,L23,L24);
         Journalisation_EcrireFichier(Ip, NULL,"DISCO - DEBUT des tests!");
         CreateThread(NULL,0,Disco,Ip,0,0);
       }else if (LIp2 == 0 && L14 >0 && L14 < 255 && L13 <255 && L12 <255 && L11<255)
       {
         NB_Machines_total = 1;

         WaitForSingleObject(hSemaphore,INFINITE);
         WaitForSingleObject(hSemaphoreIp,INFINITE);

         _snprintf(Ip,16,"%d.%d.%d.%d",L11,L12,L13,L14);
         Journalisation_EcrireFichier(Ip, NULL,"DISCO - DEBUT des tests!");
         CreateThread(NULL,0,Disco,Ip,0,0);

       }else if (L21 >= L11 && (L22 >= L12 || L21 > L11) && (L23 >= L13 || (L22 > L12 || L21 > L11)) && (L24 >= L14 || (L23 > L13 || (L22 > L12 || L21 > L11))))
       {
         //calcul du nombre de machine
         NB_Machines_total=((L21-L11)*(255*255*255))+1;//octet 1

         if (L12<=L22)//octet 2
            NB_Machines_total+=(L22-L12)*(255*255);
         else
             NB_Machines_total+=((255-L12)+L22)*(255*255);

          if (L13<=L23)//octet 3
             NB_Machines_total+=(L23-L13)*(255);
          else
             NB_Machines_total+=((255-L13)+L23)*(255);

          if (L14<=L24)//octet 4
             NB_Machines_total+=(L24-L14);
          else
             NB_Machines_total+=((255-L14)+L24);

         //boucle
         for (a = L11; a<=L21 && test_DISCO;a++)
         {
           if (a == L21)
           {
             initb = L12;
             finb = L22;
           }else
           {
             initb = 1;
             finb = 255;
           }
           for (b = initb; b<=finb && test_DISCO;b++ )
           {
             if (b == L22)
             {
               initc = L13;
               finc = L23;
             }else
             {
               initc = 1;
               finc = 255;
             }
             for (c = initc; c<=finc && test_DISCO;c++)
             {
               if (c == L23)
               {
                 initd = L14;
                 find = L24;
               }else
               {
                 initd = 1;
                 find = 255;
               }
               for (d = initd; d<=find && test_DISCO;d++)
               {
                 WaitForSingleObject(hSemaphore,INFINITE);
                 WaitForSingleObject(hSemaphoreIp,INFINITE);
                 _snprintf(Ip,16,"%d.%d.%d.%d",a,b,c,d);
                 Journalisation_EcrireFichier(Ip, NULL,"DISCO - DEBUT des tests!");
                 CreateThread(NULL,0,Disco,Ip,0,0);

                 //progression + info ip
                 SetWindowText(GetDlgItem(Tabl[TAB_DISCO],INFO_IP),Ip);
                 SendMessage(ProgressBAR, PBM_SETPOS, (WPARAM)(++point)*100/NB_Machines_total, 0);
               }
             }
           }
         }

       }else MessageBox(HPrincipal,Language->error_msg_ip,Language->error,MB_OK);

       //On grise le bouton pour éviter un conflit avec la fonction pour kill le scan
       //EnableWindow(GetDlgItem(Tabl[TAB_DISCO],BT_START),FALSE);

       //on attend la fin NB_MAX_THREAD threads
       for (a=0;a<nb_max_thread;a++)WaitForSingleObject(hSemaphore,INFINITE);
       for (a=0;a<nb_max_thread;a++)ReleaseSemaphore(hSemaphore,1,NULL);
       CloseHandle(hSemaphore);

       WaitForSingleObject(hSemaphoreIp,INFINITE);
       ReleaseSemaphore(hSemaphoreIp,1,NULL);
       CloseHandle(hSemaphoreIp);

       //MUTEX
       WaitForSingleObject(hMutexLSTV,INFINITE);
       ReleaseMutex(hMutexLSTV);
       CloseHandle(hMutexLSTV);

       WaitForSingleObject(hMutexPromiscuousTest,INFINITE);
       ReleaseMutex(hMutexPromiscuousTest);
       CloseHandle(hMutexPromiscuousTest);

       if (SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
       {
         //libération de la mémoire
         //LocalFree(Ficbuffer);
         //Ficbuffer = 0;
         WaitForSingleObject(hMutex,INFINITE);
         ReleaseMutex(hMutex);
         CloseHandle(hMutex);
       }

       Tri(hListView,0,NB_COL);

       SendMessage(ProgressBAR, PBM_SETPOS, (WPARAM)0, 0);
       SetWindowText(GetDlgItem(Tabl[TAB_DISCO],INFO_IP),"");

    }else MessageBox(HPrincipal,Language->error_msg_ip,Language->error,MB_OK);

    //réactiver les boutons
    SetWindowText(GetDlgItem(Tabl[TAB_DISCO],BT_START),Language->demarrer);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],LSTV),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_DNS),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_PROMISCUOUS),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],BT_START),TRUE);

    ReInitReseau(0);
  }
  test_DISCO = 0;

  #ifdef DEBUG_TIME_TEST
  printf("DISCO - Temps exécution : %d\n",GetTickCount()-time_calc_debut);
  #endif

  Journalisation_EcrireFichier(NULL, NULL,"DISCO - FIN des tests!");
  MiseEnGras(GetDlgItem(HPrincipal,BT_DISCO),FALSE);

  return 0;
}
//------------------------------------------------------------------------------
BOOL DISCOresolution_ARP;
BOOL DISCOresolution_DNS_NETBIOS;
//------------------------------------------------------------------------------
//gestion de l'écoute de paquets ARP, ICMP, TCP, UDP
DWORD  WINAPI EcouteDecouverteGlobaleDisco(LPVOID lParam)
{
  WaitForSingleObject(hMutex_Ecoute_DISCO,INFINITE);

  //gestion pour l'ajout instantanée des items :p
  long int itemPos;
  LVITEM lvi;
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";

  char tmp[50];

  TEST_DISCO_ECOUTE *conf_ecoute = (TEST_DISCO_ECOUTE *)lParam;
  pcap_t *fp;
  char errbuf[PCAP_ERRBUF_SIZE];
  int res;

  if ((fp = MYpcap_open_live(If_Dev[conf_ecoute->id_carte_reseau].description,65536,0,1000,errbuf)) != NULL)
  {
    //on écoute
    struct pcap_pkthdr *header;
    const u_char *pkt_data;
    unsigned long int i;

    while((res = MYpcap_next_ex(fp, &header, &pkt_data)) >= 0 && conf_ecoute->ScanActif)
    {
       if (res ==0 || header->caplen < TAILLE_PAQUET_UDP)continue;

       //suivant le type de trame on traite ^^
       if (pkt_data[12] == 0x08)
       {
          //ICMP REPLY // TCP // UDP
          if (pkt_data[13] == 0x00 && (pkt_data[23] == 0x01 || pkt_data[23] == 0x06 || pkt_data[23] == 0x11))
          {
            //vérification de l'ip source
            for (i=0;i<conf_ecoute->nb_machines && conf_ecoute->ScanActif;i++)
            {
              if (conf_ecoute->MonScan[i].ttl == 0 || conf_ecoute->MonScan[i].existe==FALSE)
              {
                if (conf_ecoute->MonScan[i].ip[0] == pkt_data[26]
                &&  conf_ecoute->MonScan[i].ip[1] == pkt_data[27]
                &&  conf_ecoute->MonScan[i].ip[2] == pkt_data[28]
                &&  conf_ecoute->MonScan[i].ip[3] == pkt_data[29])
                {
                  //lecture de son adresse MAC
                  conf_ecoute->MonScan[i].mac[0] = pkt_data[6];
                  conf_ecoute->MonScan[i].mac[1] = pkt_data[7];
                  conf_ecoute->MonScan[i].mac[2] = pkt_data[8];
                  conf_ecoute->MonScan[i].mac[3] = pkt_data[9];
                  conf_ecoute->MonScan[i].mac[4] = pkt_data[10];
                  conf_ecoute->MonScan[i].mac[5] = pkt_data[11];

                  //test s'il existe déja ^^
                  if (conf_ecoute->MonScan[i].ttl == 0)
                  {
                    if (conf_ecoute->MonScan[i].id == -1)
                    {
                      lvi.iItem = ListView_GetItemCount(conf_ecoute->hListView);
                      itemPos = ListView_InsertItem(conf_ecoute->hListView, &lvi);
                      conf_ecoute->MonScan[i].id = itemPos;

                      //IP
                      sprintf(tmp,"%d.%d.%d.%d",conf_ecoute->MonScan[i].ip[0],conf_ecoute->MonScan[i].ip[1]
                                               ,conf_ecoute->MonScan[i].ip[2],conf_ecoute->MonScan[i].ip[3]);
                      ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_IP,tmp);

                      //MAC
                      sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",conf_ecoute->MonScan[i].mac[0],conf_ecoute->MonScan[i].mac[1],conf_ecoute->MonScan[i].mac[2],conf_ecoute->MonScan[i].mac[3],conf_ecoute->MonScan[i].mac[4],conf_ecoute->MonScan[i].mac[5]);
                      ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_MAC,tmp);
                      if (conf_ecoute->ScanGlobal)
                        ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_TTL,"0");

                    }else
                      itemPos = conf_ecoute->MonScan[i].id;

                    if (!conf_ecoute->ScanGlobal)
                    {
                      //TTL
                      if (pkt_data[22]>0)
                      {
                        if (pkt_data[22]<=MACH_LINUX)strcpy(tmp,"Linux\0");
                        else if (pkt_data[22]<=MACH_WINDOWS)strcpy(tmp,"Windows\0");
                        else strcpy(tmp,"Routeur\0");
                      }else strcpy(tmp,"0\0");

                      ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_OS,tmp);

                       _snprintf(tmp,8,"TTL:%d",pkt_data[22]);
                      ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_TTL,tmp);
                    }else
                    {
                      if (pkt_data[22]<=MACH_LINUX)
                        _snprintf(tmp,20,"TTL:%d (Linux)",pkt_data[22]);
                      else if (pkt_data[22]<=MACH_WINDOWS)
                        _snprintf(tmp,20,"TTL:%d (Windows)",pkt_data[22]);
                      else
                        _snprintf(tmp,20,"TTL:%d (Routeur)",pkt_data[22]);
                      ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_NOM,tmp);
                    }
                  }

                  //lecture du TTL
                  conf_ecoute->MonScan[i].ttl = pkt_data[22];

                  //etat ok
                  conf_ecoute->MonScan[i].existe=TRUE;
                  conf_ecoute->nb_machines_vue++;
                  break;
                }
              }
            }
          //ARP REPLY
          }else if (pkt_data[13] == 0x06 && pkt_data[21] == 0x02)
          {
            //vérification de l'ip source
            for (i=0;i<conf_ecoute->nb_machines && conf_ecoute->ScanActif;i++)
            {
              if (conf_ecoute->MonScan[i].ttl == 0 || conf_ecoute->MonScan[i].existe==0)
              {
                if (conf_ecoute->MonScan[i].ip[0] == pkt_data[28]
                &&  conf_ecoute->MonScan[i].ip[1] == pkt_data[29]
                &&  conf_ecoute->MonScan[i].ip[2] == pkt_data[30]
                &&  conf_ecoute->MonScan[i].ip[3] == pkt_data[31])
                {
                  //lecture de son adresse MAC
                  conf_ecoute->MonScan[i].mac[0] = pkt_data[6];
                  conf_ecoute->MonScan[i].mac[1] = pkt_data[7];
                  conf_ecoute->MonScan[i].mac[2] = pkt_data[8];
                  conf_ecoute->MonScan[i].mac[3] = pkt_data[9];
                  conf_ecoute->MonScan[i].mac[4] = pkt_data[10];
                  conf_ecoute->MonScan[i].mac[5] = pkt_data[11];

                  //test s'il existe déja ^^
                  if (conf_ecoute->MonScan[i].existe==0)
                  {
                    lvi.iItem = ListView_GetItemCount(conf_ecoute->hListView);
                    itemPos = ListView_InsertItem(conf_ecoute->hListView, &lvi);
                    conf_ecoute->MonScan[i].id = itemPos;

                    //IP
                    sprintf(tmp,"%d.%d.%d.%d",conf_ecoute->MonScan[i].ip[0],conf_ecoute->MonScan[i].ip[1]
                                             ,conf_ecoute->MonScan[i].ip[2],conf_ecoute->MonScan[i].ip[3]);
                    ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_IP,tmp);

                    //MAC
                    sprintf(tmp,"%02X:%02X:%02X:%02X:%02X:%02X",conf_ecoute->MonScan[i].mac[0],conf_ecoute->MonScan[i].mac[1],conf_ecoute->MonScan[i].mac[2],conf_ecoute->MonScan[i].mac[3],conf_ecoute->MonScan[i].mac[4],conf_ecoute->MonScan[i].mac[5]);
                    ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_MAC,tmp);
                    if (conf_ecoute->ScanGlobal)
                      ListView_SetItemText(conf_ecoute->hListView,itemPos,COL_TTL,"0");
                  }

                  //etat ok
                  conf_ecoute->MonScan[i].existe=TRUE;
                  conf_ecoute->nb_machines_vue++;
                  break;
                }
              }
            }
          }
       }
    }
    MYpcap_close(fp);
  }
  return 0;
}
//------------------------------------------------------------------------------

//modifier ici pour éviter une duplication  !! utiliser id
DWORD  WINAPI TestDiscoFinal(LPVOID lParam)
{
  //récupération des informations de la machine ^^
  TEST_DISCO_SCAN_2 *machine = (TEST_DISCO_SCAN_2 *)lParam;

  char ip[TAILLE_IP];
  char txtMAC[MAX_PATH+TMP_DEFAULT]="\0";
  char txtOS[TMP_DEFAULT]="\0";
  char txtMOM[TMP_DEFAULT]="\0";
  char txtPROMISCIOUS[TMP_DEFAULT]="\0";

  _snprintf(ip,TAILLE_IP,"%d.%d.%d.%d",machine->ip[0],machine->ip[1],machine->ip[2],machine->ip[3]);
  if (machine->mac[0] != 0 && machine->mac[0] != 0xff ||
      machine->mac[1] != 0 && machine->mac[1] != 0xff ||
      machine->mac[2] != 0 && machine->mac[2] != 0xff ||
      machine->mac[3] != 0 && machine->mac[3] != 0xff ||
      machine->mac[4] != 0 && machine->mac[4] != 0xff ||
      machine->mac[5] != 0 && machine->mac[5] != 0xff)
    _snprintf(txtMAC,TMP_DEFAULT,"%02X:%02X:%02X:%02X:%02X:%02X",machine->mac[0],machine->mac[1],machine->mac[2],machine->mac[3],machine->mac[4],machine->mac[5]);

  //libération du sémaphore :p
  ReleaseSemaphore(hSemaphoreIp,1,NULL);

  HANDLE hListView = GetDlgItem(Tabl[TAB_DISCO],LSTV);
  INIT_connnection_NETBIOS(ip);

  if (machine->ttl <1)
  {
    machine->ttl  = DLLPing_simple(ip);
    if (machine->ttl)
    {
      _snprintf(txtOS,TMP_DEFAULT,"TTL:%d",machine->ttl);
      ListView_SetItemText(hListView,machine->id,COL_TTL,txtOS);

      if (machine->ttl<=MACH_LINUX)strcpy(txtOS,"Linux\0");
      else if (machine->ttl<=MACH_WINDOWS)strcpy(txtOS,"Windows\0");
      else strcpy(txtOS,"Routeur\0");
    }else
    {
      strcpy(txtOS,"FIREWALL");
    }
    ListView_SetItemText(hListView,machine->id,COL_OS,txtOS);
  }

  //résolution ARP
  if (DISCOresolution_ARP && test_DISCO)
  {
    WaitForSingleObject(hMutexARP,INFINITE);
    ReadMacAdressConstructeur(txtMAC);
    ListView_SetItemText(hListView,machine->id,COL_MAC,txtMAC);
    ReleaseMutex(hMutexARP);
  }

  //test netbios + DNS
  if(DISCOresolution_DNS_NETBIOS && test_DISCO)
  {
    ResDNS(ip, txtMOM, TMP_DEFAULT);
    if (ResolutionOS_Netbios(ip, txtOS,TMP_DEFAULT-8) == FALSE)
    {
      ListView_GetItemText(hListView,machine->id,COL_OS,txtOS,TMP_DEFAULT-8);
    }

    if (NullSession_Netbios(ip))
      strcat(txtOS," [NULL SESSION]\0");

    WaitForSingleObject(hMutexLSTV,INFINITE);
    ListView_SetItemText(hListView,machine->id,COL_NOM,txtMOM);
    ListView_SetItemText(hListView,machine->id,COL_OS,txtOS);
    ReleaseMutex(hMutexLSTV);
  }

  //tests promiscious :p
  if (Promiscuous_param && test_DISCO)
  {
    pcap_t *fp;
    char errbuf[PCAP_ERRBUF_SIZE];
    char Mac_Promiscuous[TAILLE_MAC];
    char Ip_test[TAILLE_IP];
    WaitForSingleObject(hMutexPromiscuousTest,INFINITE);
    short id_tmp = SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0);

    int promiscous = 0;
    if(SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_LOCAL_MAC_PROMISCOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0) == FALSE)
      promiscous = 1;

    if ((fp = MYpcap_open_live(If_Dev[id_tmp].description,65536,promiscous,1000,errbuf)) != NULL)
    {
      //lecture de mon adresse IP :p
      strcpy(Ip_test,If_Dev[id_tmp].ip);

      if (promiscous)
      {
        GetWindowText(GetDlgItem(Tabl[TAB_CONF],EDIT_MAC_PROM),Mac_Promiscuous,TAILLE_MAC);
        CreerPaquetArp_test_str(Ip_test,Ip_test, Mac_Promiscuous, "FF:FF:FF:FF:FF:FF", 0,fp,promiscous);
        CreerPaquetArp_test_str(Ip_test,Ip_test, Mac_Promiscuous, "FF:FF:FF:FF:FF:FF", 0,fp,promiscous);
        CreerPaquetArp_test_str(Ip_test,Ip_test, Mac_Promiscuous, "FF:FF:FF:FF:FF:FF", 0,fp,promiscous);
      }else
      {
        if (!strcmp(ip,Ip_test))
        {
          id_tmp = -1;
        }else
        {
          //lecture de l'adresse MAC de ma machine :p
          strcpy(Mac_Promiscuous,If_Dev[id_tmp].mac);
        }
      }

      if (id_tmp>-1)
      {
        //les différents tests
        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B47, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "FF:FF:FF:FF:FF:FE", fp,promiscous))
             strcat(txtPROMISCIOUS,"B47, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B24, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "FF:FF:FF:00:00:00", fp,promiscous))
             strcat(txtPROMISCIOUS,"B24, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B16, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "FF:FF:00:00:00:00", fp,promiscous))
             strcat(txtPROMISCIOUS,"B16, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B8, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "FF:00:00:00:00:00", fp,promiscous))
             strcat(txtPROMISCIOUS,"B8, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_B1, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "01:00:00:00:00:00", fp,promiscous))
             strcat(txtPROMISCIOUS,"B1, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M0, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "01:00:5E:00:00:00", fp,promiscous))
             strcat(txtPROMISCIOUS,"M0, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M1, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "01:00:5E:00:00:01", fp,promiscous))
             strcat(txtPROMISCIOUS,"M1, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M2, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "01:00:5E:00:00:02", fp,promiscous))
             strcat(txtPROMISCIOUS,"M2, ");

        if (SendDlgItemMessage(Tabl[TAB_CONF],CONF_CHK_PROM_M3, BM_GETCHECK,(WPARAM) 0, (LPARAM)0))
          if (TestRepondArp(Ip_test, Mac_Promiscuous, ip, "01:00:5E:00:00:03", fp,promiscous))
             strcat(txtPROMISCIOUS,"M3");
      }
      strcat(txtPROMISCIOUS,"\0");
    }
    MYpcap_close(fp);
    ReleaseMutex(hMutexPromiscuousTest);
  }

  //maj de la liste des items
  WaitForSingleObject(hMutexLSTV,INFINITE);
  ListView_SetItemText(hListView,machine->id,COL_PROMISCUOUS,txtPROMISCIOUS);
  ReleaseMutex(hMutexLSTV);

  //libération du scan
  ReleaseSemaphore(hSemaphore,1,NULL);
}
//------------------------------------------------------------------------------
DWORD  WINAPI DiscoScan2(LPVOID lParam)
{
  //récupération de la liste des IPS
  DWORD LIp1,LIp2;
  BYTE L11,L12,L13,L14,L21,L22,L23,L24;
  int a,b,c,d, initb,initc,initd,finb,finc,find;

  MiseEnGras(GetDlgItem(HPrincipal,BT_DISCO),TRUE);

  //nombre de thread
  char _mytmp_num_thread[MAX_PATH+1];
  GetWindowText(GetDlgItem(Tabl[TAB_CONF],EDIT_MAX_THREAD),_mytmp_num_thread,MAX_PATH);
  nb_max_thread = atoi(_mytmp_num_thread);

  if (nb_max_thread<NB_MIN_THREAD)
    nb_max_thread = NB_MIN_THREAD;
  else if (nb_max_thread>NB_MAX_THREAD)
    nb_max_thread = NB_MAX_THREAD;

  //time out ^^
  char ctimeout[MAX_PATH+1];
  GetWindowText(GetDlgItem(Tabl[TAB_CONF],TOM_ICMP),ctimeout,MAX_PATH);
  time_out = atoi(ctimeout);

  //suppression des machines de la liste
  ListView_DeleteAllItems(GetDlgItem(Tabl[TAB_DISCO],LSTV));

  //récupération des 2 IPS
  SendDlgItemMessage(Tabl[TAB_DISCO],IP, IPM_GETADDRESS,(WPARAM) 0, (LPARAM)&LIp1);
  SendDlgItemMessage(Tabl[TAB_DISCO],IP2, IPM_GETADDRESS,(WPARAM) 0, (LPARAM)&LIp2);

  //test de la validité des ips
  //on récupère chacun des octet
  L11 = LIp1 >> 24;
  L12 = (LIp1 >> 16) & 0xFF;
  L13 = (LIp1 >> 8) & 0xFF;
  L14 = LIp1 & 0xFF;

  L21 = LIp2 >> 24;
  L22 = (LIp2 >> 16) & 0xFF;
  L23 = (LIp2 >> 8) & 0xFF;
  L24 = LIp2 & 0xFF;

  //test des adresses IP
  if (LIp1!=0 && L14 >0 && L14 < 255 && L13 <255 && L12 <255 && L11<255 || LIp2!=0 && L24 >0 && L24 < 255 && L23 <255 && L22 <255 && L21<255)
  {
    //on désactive les options
    SetWindowText(GetDlgItem(Tabl[TAB_DISCO],BT_START),Language->arreter);
    //EnableWindow(GetDlgItem(Tabl[TAB_DISCO],LSTV),FALSE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP),FALSE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_DNS),FALSE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_PROMISCUOUS),FALSE);

    //init du réseau
    //Reset_Netbios();
    InitReseau(0);
    test_DISCO = TRUE;

    //traitement de la liste des IPS à traiter ^^
    unsigned long NB_Machines_total =0,test_nb=0;
    TEST_DISCO_SCAN_2 *ListeDesMachines;

    //1 seule IP
    if ((LIp1 == 0 && L21>0) || LIp1 == LIp2)
    {
      NB_Machines_total = test_nb = 1;

      //init la liste des machines
      ListeDesMachines = (TEST_DISCO_SCAN_2*)LocalAlloc(LMEM_FIXED,sizeof(TEST_DISCO_SCAN_2)*1);
      ListeDesMachines[0].ip[0] = L21;
      ListeDesMachines[0].ip[1] = L22;
      ListeDesMachines[0].ip[2] = L23;
      ListeDesMachines[0].ip[3] = L24;

      ListeDesMachines[0].mac[0]=0xff;
      ListeDesMachines[0].mac[1]=0xff;
      ListeDesMachines[0].mac[2]=0xff;
      ListeDesMachines[0].mac[3]=0xff;
      ListeDesMachines[0].mac[4]=0xff;
      ListeDesMachines[0].mac[5]=0xff;

      ListeDesMachines[0].existe = FALSE;
      ListeDesMachines[0].ttl = 0;
      ListeDesMachines[0].id = -1;

    //1 seule IP
    }else if (LIp2 == 0 && L14 >0 && L14 < 255 && L13 <255 && L12 <255 && L11<255)
    {
      NB_Machines_total = test_nb = 1;

      //init la liste des machines
      ListeDesMachines = (TEST_DISCO_SCAN_2*)LocalAlloc(LMEM_FIXED,sizeof(TEST_DISCO_SCAN_2)*1);
      ListeDesMachines[0].ip[0] = L11;
      ListeDesMachines[0].ip[1] = L12;
      ListeDesMachines[0].ip[2] = L13;
      ListeDesMachines[0].ip[3] = L14;

      ListeDesMachines[0].mac[0]=0xff;
      ListeDesMachines[0].mac[1]=0xff;
      ListeDesMachines[0].mac[2]=0xff;
      ListeDesMachines[0].mac[3]=0xff;
      ListeDesMachines[0].mac[4]=0xff;
      ListeDesMachines[0].mac[5]=0xff;
      ListeDesMachines[0].existe = FALSE;
      ListeDesMachines[0].ttl = 0;
      ListeDesMachines[0].id = -1;

    //multiples IP
    }else if (L21 >= L11 && (L22 >= L12 || L21 > L11) && (L23 >= L13 || (L22 > L12 || L21 > L11)) && (L24 >= L14 || (L23 > L13 || (L22 > L12 || L21 > L11))))
    {
     //calcul du nombre de machine
     NB_Machines_total = ((L21-L11)*(256*256*256))+1;
     if (L12<=L22)//octet 2
        NB_Machines_total+=(L22-L12)*(256*256);
     else
         NB_Machines_total+=((256-L12)+L22)*(256*256);

      if (L13<=L23)//octet 3
         NB_Machines_total+=(L23-L13)*(256);
      else
         NB_Machines_total+=((256-L13)+L23)*(256);

      if (L14<=L24)//octet 4
         NB_Machines_total+=(L24-L14);
      else
         NB_Machines_total+=((256-L14)+L24);

      //init la liste des machines
      ListeDesMachines = (TEST_DISCO_SCAN_2*)LocalAlloc(LMEM_FIXED,sizeof(TEST_DISCO_SCAN_2)*NB_Machines_total);
      for (a = L11; a<=L21 && test_DISCO;a++)
      {
        if (a == L21)
        {
          initb = L12;
          finb = L22;
        }else
        {
          initb = 1;
          finb = 255;
        }
        for (b = initb; b<=finb && test_DISCO;b++)
        {
          if (b == L22)
          {
            initc = L13;
            finc = L23;
          }else
          {
            initc = 1;
            finc = 255;
          }
          for (c = initc; c<=finc && test_DISCO;c++)
          {
            if (c == L23)
            {
              initd = L14;
              find = L24;
            }else
            {
              initd = 1;
              find = 255;
            }
            for (d = initd; d<=find && test_nb<NB_Machines_total && test_DISCO;d++)
            {
              ListeDesMachines[test_nb].ip[0] = a;
              ListeDesMachines[test_nb].ip[1] = b;
              ListeDesMachines[test_nb].ip[2] = c;
              ListeDesMachines[test_nb].ip[3] = d;

              ListeDesMachines[test_nb].mac[0]=0xff;
              ListeDesMachines[test_nb].mac[1]=0xff;
              ListeDesMachines[test_nb].mac[2]=0xff;
              ListeDesMachines[test_nb].mac[3]=0xff;
              ListeDesMachines[test_nb].mac[4]=0xff;
              ListeDesMachines[test_nb].mac[5]=0xff;
              ListeDesMachines[test_nb].existe = FALSE;
              ListeDesMachines[test_nb].ttl = 0;
              ListeDesMachines[test_nb].id = -1;
              test_nb++;
            }
          }
        }
      }
    }

    //test ^^
    TEST_DISCO_ECOUTE MontTest;
    // MontTest.fin_apres_ScanActif = FALSE;

    //init
    MontTest.MonScan = ListeDesMachines;
    MontTest.nb_machines = test_nb;
    MontTest.nb_machines_vue = 0;
    MontTest.ScanActif = TRUE;
    MontTest.ScanGlobal = FALSE;
    MontTest.id_carte_reseau = SendDlgItemMessage(Tabl[TAB_CONF],CONF_INTERFACE, CB_GETCURSEL,(WPARAM)0, (LPARAM)0);

    //état des tests
    Promiscuous_param = SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_PROMISCUOUS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0);
    DISCOresolution_ARP = SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP, BM_GETCHECK,(WPARAM) 0, (LPARAM)0);
    DISCOresolution_DNS_NETBIOS = SendDlgItemMessage(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_DNS, BM_GETCHECK,(WPARAM) 0, (LPARAM)0);

    //maj de la passerelle
    routage_route = LirePasserelle(routage_route_mac,routage_route_ip);

    unsigned char ip_src[4];
    unsigned char mac_src[6];
    LireIpOctal(If_Dev[MontTest.id_carte_reseau].ip,ip_src);
    LireMacOctal(If_Dev[MontTest.id_carte_reseau].mac,mac_src);
    MontTest.hListView = GetDlgItem(Tabl[TAB_DISCO],LSTV);

    //lancement de l'écoute ^^
    Journalisation_EcrireFichier(NULL, NULL,"DISCO - DEBUT de découverte des machines!");
    hMutex_Ecoute_DISCO = CreateMutex(0,FALSE,"hMutex_Ecoute_DISCO");
    HANDLE hecoute = CreateThread(NULL,0,EcouteDecouverteGlobaleDisco,(PVOID)&MontTest,0,0);

    //envoie des trames :p
    pcap_t *fp;
    char errbuf[PCAP_ERRBUF_SIZE];
    if ((fp = MYpcap_open_live(If_Dev[MontTest.id_carte_reseau].description,65536,0,1000,errbuf)) != NULL)
    {
      for (test_nb=0;test_nb<MontTest.nb_machines && test_DISCO;test_nb++)
      {
        //arp
        CreerPaquetArp_test(ip_src, ListeDesMachines[test_nb].ip,mac_src,ListeDesMachines[test_nb].mac,0,fp);

        //icmp
        CreerPaquetICMP_test(ip_src, mac_src, ListeDesMachines[test_nb].ip,ListeDesMachines[test_nb].mac,0,fp);

        if (routage_route)
          CreerPaquetICMP_test(ip_src, mac_src, ListeDesMachines[test_nb].ip,routage_route_mac,0,fp);

        if (test_nb%10 == 0) //toutes les 10 machines on fait un Sleep pour pas saturer les connexions ^^
          Sleep(10);
      }
    }
    MYpcap_close(fp);

    //on attend la réponse :p
    if (test_DISCO) Sleep(time_out);
    MontTest.ScanActif = FALSE;

    WaitForSingleObject(hMutex_Ecoute_DISCO,INFINITE);
    CloseHandle(hMutex_Ecoute_DISCO);


    //on attend la fin ^^
   // while (MontTest.fin_apres_ScanActif != TRUE)Sleep(100);

    //on tue le thread pour plus de prudence :p
    DWORD IDThread;
    GetExitCodeThread(hecoute,&IDThread);
    TerminateThread(hecoute,IDThread);
    Journalisation_EcrireFichier(NULL, NULL,"DISCO - FIN de découverte des machines!");

    //on passe à la suite pour les autres machines on lance des tests :p
    hSemaphore=CreateSemaphore(NULL,nb_max_thread,nb_max_thread,NULL); //liste des scan simulatnéesS
    hSemaphoreIp=CreateSemaphore(NULL,1,1,NULL); //récupération de l'adresse IP
    hMutexLSTV = CreateMutex(0,FALSE,"mutexLSTV"); // ajout d'un item à la liste view
    hMutexARP = CreateMutex(0,FALSE,"hMutexARP"); // ajout d'un item à la liste view
    hMutexPromiscuousTest = CreateMutex(0,FALSE,"mutexPROMISCOUS"); // test promiscuous

    //on exécute le thread de scan ^^
    HANDLE ProgressBAR = GetDlgItem(Tabl[TAB_DISCO],ARP_PB);
    char tmp_ip[TAILLE_IP];
    unsigned long test_nb_ok=0;
    for (test_nb=0;test_nb<MontTest.nb_machines && test_DISCO;test_nb++)
    {
      if (ListeDesMachines[test_nb].existe)
      {
        WaitForSingleObject(hSemaphore,INFINITE);
        WaitForSingleObject(hSemaphoreIp,INFINITE);

        CreateThread(NULL,0,TestDiscoFinal,(void*)&MontTest.MonScan[test_nb],0,0);
        sprintf(tmp_ip,"%d.%d.%d.%d",ListeDesMachines[test_nb].ip[0],ListeDesMachines[test_nb].ip[1],ListeDesMachines[test_nb].ip[2],ListeDesMachines[test_nb].ip[3]);

        SetWindowText(GetDlgItem(Tabl[TAB_DISCO],INFO_IP),tmp_ip);
        Journalisation_EcrireFichier(tmp_ip, NULL,"DISCO - debut resolution ARP/DNS/NETBIOS/PROMISCIOUS!");
        SendMessage(ProgressBAR, PBM_SETPOS, (WPARAM)++test_nb_ok*100/MontTest.nb_machines_vue, 0);
      }

    }

    //vidage des sémaphores et mutex
    //on attend la fin NB_MAX_THREAD threads
    for (test_nb=0;test_nb<nb_max_thread;test_nb++)WaitForSingleObject(hSemaphore,INFINITE);
    for (test_nb=0;test_nb<nb_max_thread;test_nb++)ReleaseSemaphore(hSemaphore,1,NULL);
    CloseHandle(hSemaphore);

    WaitForSingleObject(hSemaphoreIp,INFINITE);
    ReleaseSemaphore(hSemaphoreIp,1,NULL);
    CloseHandle(hSemaphoreIp);

    test_DISCO = FALSE;

    //MUTEX
    WaitForSingleObject(hMutexLSTV,INFINITE);
    ReleaseMutex(hMutexLSTV);
    CloseHandle(hMutexLSTV);

    WaitForSingleObject(hMutexARP,INFINITE);
    ReleaseMutex(hMutexARP);
    CloseHandle(hMutexARP);

    WaitForSingleObject(hMutexPromiscuousTest,INFINITE);
    ReleaseMutex(hMutexPromiscuousTest);
    CloseHandle(hMutexPromiscuousTest);

    Journalisation_EcrireFichier(NULL, NULL,"DISCO - FIN de la résolution des infos des machines!");

    Tri(GetDlgItem(Tabl[TAB_DISCO],LSTV),0,NB_COL);

    SendMessage(ProgressBAR, PBM_SETPOS, (WPARAM)0, 0);
    SetWindowText(GetDlgItem(Tabl[TAB_DISCO],INFO_IP),"");

    //vidage de la mémoire :p
    LocalFree(ListeDesMachines);

    //init du réseau
    ReInitReseau(0);

    //on réactive les options
    SetWindowText(GetDlgItem(Tabl[TAB_DISCO],BT_START),Language->demarrer);
    //EnableWindow(GetDlgItem(Tabl[TAB_DISCO],LSTV),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_ARP),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_RESOLUTION_DNS),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],ARP_CHK_PROMISCUOUS),TRUE);
    EnableWindow(GetDlgItem(Tabl[TAB_DISCO],BT_START),TRUE);

  }

  MiseEnGras(GetDlgItem(HPrincipal,BT_DISCO),FALSE);

  return 0;
}
//------------------------------------------------------------------------------
DWORD  WINAPI KillThreadDisco(LPVOID lParam)
{
  test_DISCO = 0;

  return 0;
}
