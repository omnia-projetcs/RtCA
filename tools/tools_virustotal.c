//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
#define MAX_VST_ERROR 5
DWORD vt_error;
//------------------------------------------------------------------------------
//use for update
void UpdateDataBaseWithVirusTotal(char *sha256, char* virustotal_datas)
{
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "UPDATE extract_file SET VirusTotal='%s' WHERE SAH256='%s';",
           virustotal_datas,sha256);
  sqlite3_exec(db_scan,request, NULL, NULL, NULL);
}
//------------------------------------------------------------------------------
typedef struct
{
  char token[MAX_PATH];
  char sha256[65];
  int exist;                  //existe ou pas dans la base
  char last_analysis_date[20];
  char detection_ratio[20];
}VIRUSTOTAL_STR;
//------------------------------------------------------------------------------
void GetCSRFToken(VIRUSTOTAL_STR *vts)
{
  HINTERNET M_connexion = 0;
  if (!use_other_proxy)M_connexion = InternetOpen("",/*INTERNET_OPEN_TYPE_DIRECT*/INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE);
  else M_connexion = InternetOpen("",/*INTERNET_OPEN_TYPE_DIRECT*/INTERNET_OPEN_TYPE_PROXY, proxy_ch_auth, NULL, 0);

  if (M_connexion==NULL)M_connexion = InternetOpen("",INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE);
  if (M_connexion==NULL)return;

  //init connexion
  HINTERNET M_session = InternetConnect(M_connexion, "www.virustotal.com",443,"","",INTERNET_SERVICE_HTTP,0,0);
  if (M_session==NULL)
  {
    InternetCloseHandle(M_connexion);
    return;
  }

  //connexion
  HINTERNET M_requete = HttpOpenRequest(M_session,"GET","www.virustotal.com",NULL,"",NULL,
                                        INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_SECURE
                                        |INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID,0);

  if (use_proxy_advanced_settings)
  {
    InternetSetOption(M_requete,INTERNET_OPTION_PROXY_USERNAME,proxy_ch_user,sizeof(proxy_ch_user));
    InternetSetOption(M_requete,INTERNET_OPTION_PROXY_PASSWORD,proxy_ch_password,sizeof(proxy_ch_password));
  }

  if (M_requete==NULL)
  {
    InternetCloseHandle(M_session);
    InternetCloseHandle(M_connexion);
    return;
  }else if (HttpSendRequest(M_requete, NULL, 0, NULL, 0))
  {
    //traitement !!!
    char buffer[MAX_PATH];
    DWORD dwNumberOfBytesRead = MAX_PATH;

    if(HttpQueryInfo(M_requete,HTTP_QUERY_SET_COOKIE, buffer, &dwNumberOfBytesRead, 0))
    {
      if (dwNumberOfBytesRead>42)buffer[42]=0;

      //on passe : csrftoken=
      strcpy(vts->token,buffer+10);
    }
    InternetCloseHandle(M_requete);
  }
  //close
  InternetCloseHandle(M_session);
  InternetCloseHandle(M_connexion);
}
//------------------------------------------------------------------------------
void GetSHA256Info(VIRUSTOTAL_STR *vts)
{
    //init connexion
  HINTERNET M_connexion = 0;
  if (!use_other_proxy)M_connexion = InternetOpen("",/*INTERNET_OPEN_TYPE_DIRECT*/INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE);
  else M_connexion = InternetOpen("",/*INTERNET_OPEN_TYPE_DIRECT*/INTERNET_OPEN_TYPE_PROXY, proxy_ch_auth, NULL, 0);

  if (M_connexion==NULL)M_connexion = InternetOpen("",INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, INTERNET_FLAG_NO_CACHE_WRITE);
  if (M_connexion==NULL)return;

  HINTERNET M_session = InternetConnect(M_connexion, "www.virustotal.com",443,"","",INTERNET_SERVICE_HTTP,0,0);
  if (M_session==NULL)
  {
    InternetCloseHandle(M_connexion);
    return;
  }
  char request[MAX_PATH]="";
  snprintf(request,MAX_PATH,"/file/upload/?sha256=%s",vts->sha256);

  //connexion
  HINTERNET M_requete = HttpOpenRequest(M_session,"GET",request,NULL,"https://www.virustotal.com/",NULL,
                                        INTERNET_FLAG_NO_CACHE_WRITE|INTERNET_FLAG_SECURE
                                        |INTERNET_FLAG_IGNORE_CERT_CN_INVALID|INTERNET_FLAG_IGNORE_CERT_DATE_INVALID,0);

  if (use_proxy_advanced_settings)
  {
    InternetSetOption(M_requete,INTERNET_OPTION_PROXY_USERNAME,proxy_ch_user,sizeof(proxy_ch_user));
    InternetSetOption(M_requete,INTERNET_OPTION_PROXY_PASSWORD,proxy_ch_password,sizeof(proxy_ch_password));
  }

  //Création du paramètre
  char cookie[MAX_PATH]="";
  snprintf(cookie,MAX_PATH,"Cookie: csrftoken=%s",vts->token);

  if (M_requete==NULL)
  {
    InternetCloseHandle(M_session);
    InternetCloseHandle(M_connexion);
    return;
  }else if (HttpSendRequest(M_requete, cookie, strlen(cookie), NULL, 0))
  {
    INTERNET_BUFFERS ib;
    ib.dwStructSize       = sizeof(INTERNET_BUFFERS);
    ib.lpcszHeader        = NULL;
    ib.dwHeadersLength    = 0;
    ib.dwHeadersTotal     = 0;
    ib.dwOffsetLow        = 0;
    ib.dwOffsetHigh       = 0;

    char resultat[16000];
    ib.lpvBuffer          = resultat;
    ib.dwBufferLength     = 16000-1;
    ib.dwBufferTotal      = 16000-1;

    if(InternetReadFileEx(M_requete,&ib,IRF_NO_WAIT,0))
    {
      if (strlen(resultat)>20)
      {
        //"file_exists": true
        char *c = resultat;
        while (*c && (*c != ',' || *(c+1)!=' '|| *(c+2)!='"' || *(c+3)!='f'|| *(c+4)!='i'))c++;
        if (*c == ',' && *(c+1)== ' '&& *(c+2)== '"' && *(c+3)== 'f'&& *(c+4)== 'i')
        {
          c+=strlen(", \"file_exists\": "); //17

          if (*c == 't')
          {
            vts->exist = TRUE;
          }else vts->exist = FALSE;
        }else vts->exist = FALSE;

        //lecture + convertion : , "last_analysis_date": "
        c = resultat;
        while (*c && (*c != ',' || *(c+1)!=' '|| *(c+2)!='"' || *(c+3)!='l'|| *(c+4)!='a'|| *(c+17)!='d'))c++;

        if (*c == ',' && *(c+1)==' ' && *(c+2)=='"' && *(c+3)=='l' && *(c+4)=='a' && *(c+17)=='d')
        {
          c+=strlen(", \"last_analysis_date\": \""); //25

          //test si une date ou non !!!
          if (*c == 'u')strncpy(vts->last_analysis_date,"NULL",5);
          else
          {
            strncpy(vts->last_analysis_date,c,19);

            vts->last_analysis_date[4]='/';
            vts->last_analysis_date[7]='/';
            vts->last_analysis_date[10]='-';
            vts->last_analysis_date[19]=0;
          }
        }

        //lecture : detection_ratio
        c = resultat;
        while (*c && (*c != '['))c++;
        if (*c == '[')
        {
          c++;

          if (*(c+1) == ',')
          {
            snprintf(vts->detection_ratio,19,"0%s",c);
          }else strncpy(vts->detection_ratio,c,19);

          //recherche de la fin
          c = vts->detection_ratio;
          while (*c && *c!=']')c++;
          *c=0;
        }
      }else vts->exist = -2;
    }
    InternetCloseHandle(M_requete);
  }

  //close
  InternetCloseHandle(M_session);
  InternetCloseHandle(M_connexion);
}
//------------------------------------------------------------------------------
BOOL CheckItemToVirusTotal(HANDLE hlv, DWORD item, unsigned int column_sha256, unsigned int colum_sav, char *token, BOOL check, BOOL check_all_lv_items)
{
  BOOL ret = FALSE;

  //init
  VIRUSTOTAL_STR vts;
  vts.token[0]              = 0;
  vts.exist                 = -1;
  vts.sha256[0]             = 0;
  vts.last_analysis_date[0] = 0;
  vts.detection_ratio[0]    = 0;

  char tmp[51] = "";
  ListView_GetItemText(hlv,item,column_sha256,vts.sha256,65);
  ListView_GetItemText(hlv,item,colum_sav,tmp,50);

  //if (vts.sha256[0] == 0 || (check && (tmp[0] != 0)/* || tmp[0] == 'R' || (tmp[0] == 'U' && tmp[7] == 'R')))*/)return;
  if (vts.sha256[0] == 0 || (check && (tmp[0] != 0 && tmp[0] != 'R' && tmp[0] != 'U')))return FALSE;

  if (token != NULL)
  {
    if (token[0] != 0)strcpy(vts.token,token);
    else
    {
      GetCSRFToken(&vts);
      if (vts.token[0] != 0)
        strcpy(token,vts.token);
    }
  }else GetCSRFToken(&vts);

  if (vts.token[0] != 0)
  {
    //test du hash
    GetSHA256Info(&vts);
  }

  //résultats
  char resultats[MAX_LINE_SIZE];
  switch(vts.exist)
  {
    case -1:strcpy(resultats,"Connection error\0");vt_error++;break;
    case -2:strcpy(resultats,"Unkown datas\0");break;
    case 0:
      ret = TRUE;
      if (vts.last_analysis_date[0] != 0 && vts.detection_ratio[0] != 0)
      {
        snprintf(resultats,MAX_LINE_SIZE,"Ratio : %s (Last analysis : %s) Url : https://www.virustotal.com/file/%s/analysis/",vts.detection_ratio,vts.last_analysis_date,vts.sha256);
        UpdateDataBaseWithVirusTotal(vts.sha256, resultats);
      }else if (vts.last_analysis_date[0] != 0 || vts.detection_ratio[0] != 0)
      {
        snprintf(resultats,MAX_LINE_SIZE,"Unknow Ratio : %s (Last analysis : %s) Url : https://www.virustotal.com/file/%s/analysis/",vts.detection_ratio,vts.last_analysis_date,vts.sha256);
      }else snprintf(resultats,MAX_LINE_SIZE,"Unknow");
    break;
    case 1:
      ret = TRUE;
      snprintf(resultats,MAX_LINE_SIZE,"Ratio : %s (Last analysis : %s) Url : https://www.virustotal.com/file/%s/analysis/",vts.detection_ratio,vts.last_analysis_date,vts.sha256);
      UpdateDataBaseWithVirusTotal(vts.sha256, resultats);
    break;
  }
  ListView_SetItemText(hlv,item,colum_sav,resultats);//owner

  //check all others files identical
  // too slow
  /*if (check_all_lv_items)
  {
    DWORD i, nb_items = SendMessage(hlv,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0);
    if (nb_items > item)
    {
      char tmp_sha256[MAX_PATH];
      for (i=item+1;i++;i<nb_items)
      {
        tmp_sha256[0] = 0;
        ListView_GetItemText(hlv,i,column_sha256,tmp_sha256,MAX_PATH);
        if (!strcmp(tmp_sha256,vts.sha256))
        {
          ListView_SetItemText(hlv,i,colum_sav,resultats);
        }
      }
    }
  }*/
  return ret;
}
//------------------------------------------------------------------------------
typedef struct
{
  DWORD id;
  HANDLE hlv;
  char token[MAX_PATH];
  DWORD error;
}ST_VIRUSTOTAL;
HANDLE hSemaphore, hSemaphoreItem;

//------------------------------------------------------------------------------
DWORD WINAPI TCheckFileToVirusTotal(LPVOID lParam)
{
  ST_VIRUSTOTAL *s = (ST_VIRUSTOTAL *)lParam;
  DWORD item = s->id;
  HANDLE hlv = s->hlv;
  ReleaseSemaphore(hSemaphoreItem,1,NULL);
  if (s->error >= MAX_VST_ERROR)
  {
    s->token[0] = 0;
    s->error = 0;
  }

  s->error = s->error + CheckItemToVirusTotal(hlv, item, COLUMN_SHA256, COLUMN_VIRUSTOTAL,(char*)(s->token),TRUE, TRUE);
  ReleaseSemaphore(hSemaphore,1,NULL);
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI TCheckFileToVirusTotalProcess(LPVOID lParam)
{
  ST_VIRUSTOTAL *s = (ST_VIRUSTOTAL *)lParam;
  DWORD item = s->id;
  HANDLE hlv = s->hlv;
  //char token[MAX_PATH];
  //strncpy(token,s->token,MAX_PATH);
  ReleaseSemaphore(hSemaphoreItem,1,NULL);
  if (s->error >= MAX_VST_ERROR)
  {
    s->token[0] = 0;
    s->error = 0;
  }

  s->error = s->error + CheckItemToVirusTotal(hlv, item, 18, 18,(char*)(s->token),FALSE, FALSE);
  ReleaseSemaphore(hSemaphore,1,NULL);
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI CheckAllFileToVirusTotal(LPVOID lParam)
{
  //init semaphore
  #define MAX_THREADS_CK_VT 10
  ST_VIRUSTOTAL sv[MAX_THREADS_CK_VT];

  vt_error = 0;
  //Ratio : 0, 43 (Last analysis : UKtF/RH/IW-VLofR_62) Url : https://www.virustotal.com/file/2842973d15a14323e08598be1dfb87e54bf88a76be8c7bc94c56b079446edf38/analysis/
  hSemaphore=CreateSemaphore(NULL,NB_VIRUTOTAL_THREADS*MAX_THREADS_CK_VT,NB_VIRUTOTAL_THREADS*MAX_THREADS_CK_VT,NULL);

  hSemaphoreItem=CreateSemaphore(NULL,1,1,NULL);

  char msg[MAX_PATH];
  SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)"Loading key for VirusTotal check...");

  //la gestion du nombre en lecture continue permet d'effectuer un scan pendant l'énumération
  //et de gérer la suppression d'items
  DWORD i, j;
  for (j=0;j<MAX_THREADS_CK_VT;j++)
  {
    sv[j].hlv      = hlstv;
    sv[j].token[0] = 0;
    sv[j].error    = 0;
  }

  char s_sha[MAX_PATH]="";
  for (i=0,j=0;i<SendMessage(hlstv,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0);i++)
  {
    WaitForSingleObject(hSemaphore,INFINITE);
    WaitForSingleObject(hSemaphoreItem,INFINITE);

    if ((i%NB_VIRUTOTAL_THREADS) == 0)
    {
      j++;
      if (j>=MAX_THREADS_CK_VT)j=0;
    }

    sv[j].id = i;

    s_sha[0] = 0;
    ListView_GetItemText(hlstv,i,18,s_sha,MAX_PATH);
    if (s_sha[0] != 'U' && s_sha[0] != 'R')
    {
      snprintf(msg,MAX_PATH,"Load Item %ld/%ld (%ld/%d) for VirusTotal check...",i,SendMessage(hlstv,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0),j,MAX_THREADS_CK_VT);
      SendMessage(hstatus_bar,SB_SETTEXT,1, (LPARAM)msg);

      CreateThread(NULL,0,TCheckFileToVirusTotal,&(sv[j]),0,0);
    }else
    {
      ReleaseSemaphore(hSemaphoreItem,1,NULL);
      ReleaseSemaphore(hSemaphore,1,NULL);
    }
  }

  unsigned int a;
  for (a=0;a<NB_VIRUTOTAL_THREADS*MAX_THREADS_CK_VT;a++)WaitForSingleObject(hSemaphore,INFINITE);
  CloseHandle(hSemaphore);
  CloseHandle(hSemaphoreItem);

  AVIRUSTTAL = FALSE;
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI CheckAllFileToVirusTotalProcess(LPVOID lParam)
{
  //init semaphore
  ST_VIRUSTOTAL sv;
  sv.hlv = hlstv_process;

  //Ratio : 0, 43 (Last analysis : UKtF/RH/IW-VLofR_62) Url : https://www.virustotal.com/file/2842973d15a14323e08598be1dfb87e54bf88a76be8c7bc94c56b079446edf38/analysis/
  hSemaphore=CreateSemaphore(NULL,NB_VIRUTOTAL_THREADS,NB_VIRUTOTAL_THREADS,NULL);
  hSemaphoreItem=CreateSemaphore(NULL,1,1,NULL);

  //la gestion du nombre en lecture continue permet d'effectuer un scan pendant l'énumération
  long int i;
  vt_error=0;
	sv.token[0] = 0;
  for (i=0;i<SendMessage(hlstv_process,LVM_GETITEMCOUNT,(WPARAM)0,(LPARAM)0);i++)
  {
    WaitForSingleObject(hSemaphore,INFINITE);
    WaitForSingleObject(hSemaphoreItem,INFINITE);
    sv.id = i;

    char s_sha[MAX_PATH]="";
    ListView_GetItemText(hlstv_process,i,18,s_sha,MAX_PATH);
    if (s_sha[0] != 0 && s_sha[0] != 'U' && s_sha[0] != 'R')
    {
      //ListView_SetItemText(hlstv_process,i,18,s_sha);
      //get VirusTotal Datas
      //connexion error or bad token!!
      if (vt_error >= NB_VIRUTOTAL_ERROR_MAX)
      {
        sv.token[0] = 0;
        vt_error=0;
      }
      CreateThread(NULL,0,TCheckFileToVirusTotalProcess,&sv,0,0);
    }else
    {
      ReleaseSemaphore(hSemaphoreItem,1,NULL);
      ReleaseSemaphore(hSemaphore,1,NULL);
    }
  }

  unsigned int a;
  for (a=0;a<NB_VIRUTOTAL_THREADS;a++)WaitForSingleObject(hSemaphore,INFINITE);
  CloseHandle(hSemaphore);
  CloseHandle(hSemaphoreItem);

  AVIRUSTTAL = FALSE;
  return 0;
}
//------------------------------------------------------------------------------
DWORD WINAPI CheckSelectedItemToVirusTotal(LPVOID lParam)
{
  CheckItemToVirusTotal(hlstv, SendMessage(hlstv,LVM_GETNEXTITEM,-1,LVNI_FOCUSED), COLUMN_SHA256, COLUMN_VIRUSTOTAL, NULL, TRUE, TRUE);
  VIRUSTTAL = FALSE;
  return 0;
}
