//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addCAntivirustoDB(char *path, char *name, char *editor, char *engine, char *url_update, char*bdd, char*last_update,char*enable, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_antivirus (path,name,editor,engine,bdd,url_update,last_update,enable,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           path,name,editor,engine,bdd,url_update,last_update,enable,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"ANTIVIRUS\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
           path,name,editor,engine,bdd,url_update,last_update,enable,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void Scan_antivirus_local(sqlite3 *db, unsigned int session_id)
{
  //test by Antivirus
  char path[MAX_PATH],
       name[DEFAULT_TMP_SIZE],
       editor[DEFAULT_TMP_SIZE],
       engine[DEFAULT_TMP_SIZE],
       url_update[MAX_PATH],
       bdd[DEFAULT_TMP_SIZE],
       last_update[DEFAULT_TMP_SIZE];

  //Windows Microsoft Essentiels
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Microsoft Antimalware","InstallLocation",path, MAX_PATH))
  {
    strncpy(name,"Microsoft Security Essentials",DEFAULT_TMP_SIZE);
    strncpy(editor,"Microsoft",DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Microsoft Antimalware\\Signature Updates","EngineVersion",engine, DEFAULT_TMP_SIZE);
    url_update[0] = 0;
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Microsoft Antimalware\\Signature Updates","AVSignatureVersion",bdd, DEFAULT_TMP_SIZE);

    FILETIME FileTime;
    ReadFILETIMEValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Microsoft Antimalware\\Signature Updates","SignaturesLastUpdated", &FileTime);
    if (FileTime.dwHighDateTime ==0 && FileTime.dwLowDateTime == 0)last_update[0] = 0;
    else filetimeToString_GMT(FileTime, last_update, DATE_SIZE_MAX);

    if (ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Microsoft\\Microsoft Antimalware\\Real-Time Protection","DisableRealtimeMonitoring"))
      addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //AVG
  HKEY CleTmp;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Avg\\",&CleTmp)==ERROR_SUCCESS)
  {
    DWORD i,nbSubKey = 0;
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      char key_path[MAX_PATH];
      DWORD key_size;
      for (i=0;i<nbSubKey;i++)
      {
        key_size = DEFAULT_TMP_SIZE;
        path        [0]=0;
        name        [0]=0;
        editor      [0]=0;
        engine      [0]=0;
        url_update  [0]=0;
        bdd         [0]=0;
        last_update [0]=0;
        if (RegEnumKeyEx (CleTmp,i,name,&key_size,0,0,0,0)==ERROR_SUCCESS)
        {
          snprintf(key_path,MAX_PATH,"SOFTWARE\\Avg\\%s",name);
          ReadValue(HKEY_LOCAL_MACHINE,key_path,"AvgDir",path, MAX_PATH);

          strncpy(editor,"Avg",DEFAULT_TMP_SIZE);
          snprintf(key_path,MAX_PATH,"SOFTWARE\\Avg\\%s\\LinkScanner\\Prevalence",name);
          ReadValue(HKEY_LOCAL_MACHINE,key_path,"CODEVER",engine, DEFAULT_TMP_SIZE);
          url_update[0] = 0;

          snprintf(key_path,MAX_PATH,"SOFTWARE\\Avg\\%s\\Features\\Core",name);
          snprintf(bdd,DEFAULT_TMP_SIZE,"%lu",ReadDwordValue(HKEY_LOCAL_MACHINE,key_path,"fea_CORE"));

          //read the last update ^^
          snprintf(key_path,MAX_PATH,"SOFTWARE\\Avg\\%s\\Features",name);
          snprintf(last_update,DATE_SIZE_MAX,"%lu",ReadDwordValue(HKEY_LOCAL_MACHINE,key_path,"fea_Base"));

          FILETIME FileTime;
          snprintf(key_path,MAX_PATH,"SOFTWARE\\Avg\\%s\\InstallTimes",name);
          ReadFILETIMEValue(HKEY_LOCAL_MACHINE,key_path,last_update, &FileTime);
          if (FileTime.dwHighDateTime ==0 && FileTime.dwLowDateTime == 0)last_update[0] = 0;
          else filetimeToString_GMT(FileTime, last_update, DATE_SIZE_MAX);

          addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
        }
      }
    }
    RegCloseKey(CleTmp);
  }

  //G DATA
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data","RootPath",path, MAX_PATH))
  {
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data\\AVKInternetSecurity","AppTitle",name, DEFAULT_TMP_SIZE);
    strncpy(editor,"G Data",DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data\\AntiVirenKit\\OnlineUpdates","ScanWL",engine, DEFAULT_TMP_SIZE);

    char tmp0[DEFAULT_TMP_SIZE]="", tmp1[DEFAULT_TMP_SIZE]="";
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data\\AntiVirenKit\\OnlineUpdates","Engine0",tmp0, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data\\AntiVirenKit\\OnlineUpdates","Engine1",tmp1, DEFAULT_TMP_SIZE);
    snprintf(bdd,DEFAULT_TMP_SIZE,"%s/%s",tmp0,tmp1);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data\\AntiVirenKit","UpgradeCenterLink",url_update, MAX_PATH);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data\\AVStatus","SigDate",last_update, DEFAULT_TMP_SIZE);

    if (ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\G Data\\IntelliScan","Enabled"))
      addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Avira
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Avira\\AntiVir Desktop","Path",path, MAX_PATH))
  {
    strncpy(name,"AntiVir",DEFAULT_TMP_SIZE);
    strncpy(editor,"Avira",DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Avira\\AntiVir Desktop","EngineVersion",engine, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Avira\\AntiVir Desktop","VdfVersion",bdd, DEFAULT_TMP_SIZE);
    timeToString(ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Avira\\AntiVir Desktop","InstallationDate"), last_update, DEFAULT_TMP_SIZE);

    if (ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Avira\\AntiVir Desktop","SecurityDetection"))
      addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Avast
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\AVAST Software\\Avast","ProgramFolder",path, MAX_PATH))
  {
    strncpy(name,"Avast",DEFAULT_TMP_SIZE);
    strncpy(editor,"AVAST Software",DEFAULT_TMP_SIZE);

    char tmp[MAX_PATH]="",tmp2[MAX_PATH];
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\AVAST Software\\Avast","Version",tmp, MAX_PATH);
    snprintf(engine,DEFAULT_TMP_SIZE,"%s.%lu",tmp,ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\AVAST Software\\Avast","SetupVersion"));

    //bdd
    snprintf(tmp,MAX_PATH,"%s\\Setup\\setup.ini",path);
    GetPrivateProfileString("PartsLatest","part.vps","",bdd,DEFAULT_TMP_SIZE,tmp);

    //last_update
    GetPrivateProfileString("Info","LastUpdateTime","",tmp2,DEFAULT_TMP_SIZE,tmp);
    if (atoi(tmp2))timeToString(atol(tmp2), last_update, DEFAULT_TMP_SIZE);

    //state :
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\AVAST Software\\Avast","DataFolder",tmp2, MAX_PATH);
    snprintf(tmp,MAX_PATH,"%s\\ScriptShield.ini",tmp2);
    tmp2[0]=0;
    GetPrivateProfileString("Common","ProviderEnabled","0",tmp2,MAX_PATH,tmp);
    if (atoi(tmp2))addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Kaspersky
  CleTmp = 0;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\KasperskyLab\\protected",&CleTmp)==ERROR_SUCCESS)
  {
    DWORD i,nbSubKey = 0;
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      char key_path[MAX_PATH];
      char tmp[DEFAULT_TMP_SIZE]="",tmp1[DEFAULT_TMP_SIZE]="";
      DWORD key_size;
      for (i=0;i<nbSubKey;i++)
      {
        key_size = DEFAULT_TMP_SIZE;
        tmp[0]=0;
        path        [0]=0;
        name        [0]=0;
        editor      [0]=0;
        engine      [0]=0;
        url_update  [0]=0;
        bdd         [0]=0;
        last_update [0]=0;
        if (RegEnumKeyEx (CleTmp,i,tmp,&key_size,0,0,0,0)==ERROR_SUCCESS)
        {
          snprintf(key_path,MAX_PATH,"SOFTWARE\\KasperskyLab\\protected\\%s\\environment",tmp);

          //name
          if (ReadValue(HKEY_LOCAL_MACHINE,key_path,"ProductName",name, DEFAULT_TMP_SIZE))
          {
            //editor
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"CompanyName",editor, DEFAULT_TMP_SIZE);
            //MAX_PATH
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"ProductRoot",path, MAX_PATH);

            //engine
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"ProductVersion",engine, DEFAULT_TMP_SIZE);
            //url_update
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"LicUpgradeLink",url_update, MAX_PATH);
            //bdd
            tmp1[0]=0;
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"InstallBasesID",tmp1, DEFAULT_TMP_SIZE);
            snprintf(bdd,DEFAULT_TMP_SIZE,"%lu",(DWORD)atol(tmp1));
            //last_update
            tmp1[0]=0;
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"InstallDate",tmp1, DEFAULT_TMP_SIZE);
            timeToString(atol(tmp1), last_update, DEFAULT_TMP_SIZE);

            //enable
            snprintf(key_path,MAX_PATH,"SOFTWARE\\KasperskyLab\\protected\\%s",tmp);
            if (ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Avira\\AntiVir Desktop","enable"))
              addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
            else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
          }
        }
      }
    }
    RegCloseKey(CleTmp);
  }

  //bitdefender
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Bitdefender\\Gonzales\\Install","InstallPath",path, MAX_PATH))
  {
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Bitdefender\\Gonzales","WscDisplayName",name, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Bitdefender\\Gonzales","WscManufacturerName",editor, DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Bitdefender\\Gonzales","ProductVersion",engine, DEFAULT_TMP_SIZE);

    timeToString(ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Bitdefender\\Gonzales\\Update","LastUpdateDate"), last_update, DEFAULT_TMP_SIZE);

    if (ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Bitdefender\\Gonzales","ContScanState"))
      addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Panda Antivirus
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Panda Software\\Setup","Path",path, MAX_PATH))
  {
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Panda Software\\Setup","ProductName",name, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Panda Software\\Setup","InitialCompanyName",editor, DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Panda Software\\Setup","Normal",engine, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Sophos antivirus
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Sophos\\SAVService\\Application","Path",path, MAX_PATH))
  {
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Sophos\\SAVService\\Application","EventSource",name, DEFAULT_TMP_SIZE);
    strncpy(editor,"Sophos",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Sophos\\AutoUpdate","ProductVersion",engine, DEFAULT_TMP_SIZE);

    timeToString(ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Sophos\\AutoUpdate\\UpdateStatus","LastUpdateTime"), last_update, DEFAULT_TMP_SIZE);

    if (ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Sophos\\SAVService\\Components\\BackgroundScanFactory","Level"))
      addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Coranti
  CleTmp = 0;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Coranti",&CleTmp)==ERROR_SUCCESS)
  {
    DWORD i,nbSubKey = 0;
    if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
    {
      char key_path[MAX_PATH];
      DWORD key_size;
      for (i=0;i<nbSubKey;i++)
      {
        key_size = DEFAULT_TMP_SIZE;
        path        [0]=0;
        name        [0]=0;
        editor      [0]=0;
        engine      [0]=0;
        url_update  [0]=0;
        bdd         [0]=0;
        last_update [0]=0;
        if (RegEnumKeyEx (CleTmp,i,name,&key_size,0,0,0,0)==ERROR_SUCCESS)
        {
          snprintf(key_path,MAX_PATH,"SOFTWARE\\Coranti\\%sSettings",name);

          if (ReadValue(HKEY_LOCAL_MACHINE,key_path,"ProductVersion",engine, DEFAULT_TMP_SIZE))
          {
            ReadValue(HKEY_LOCAL_MACHINE,key_path,"InstallLocation",path, MAX_PATH);
            strncpy(editor,"Coranti",DEFAULT_TMP_SIZE);

            addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
          }
        }
      }
    }
    RegCloseKey(CleTmp);
  }

  //Rising Antivirus
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\rising\\RAV","installpath",path, MAX_PATH))
  {
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\rising\\RAV","name",name, DEFAULT_TMP_SIZE);
    strncpy(editor,"Rising",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\rising\\RAV","Version",engine, DEFAULT_TMP_SIZE);
    snprintf(bdd,DEFAULT_TMP_SIZE,"%s",engine);

    char tmp_path[MAX_PATH];
    snprintf(tmp_path,MAX_PATH,"%s\\CompsVer.inf",path);
    GetPrivateProfileString("UPDATE","Time","",last_update,DEFAULT_TMP_SIZE,tmp_path);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //PC-cillinNTCorp
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;

  if (ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.","ProgramVer",engine, DEFAULT_TMP_SIZE) == FALSE)
    if (ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.","EngineZipVer",engine, DEFAULT_TMP_SIZE) == FALSE)engine[0] = 0;

  if(engine[0] != 0)
  {
    strncpy(name,"Trend Micro Office Scan",DEFAULT_TMP_SIZE);
    strncpy(editor,"Trend Micro",DEFAULT_TMP_SIZE);
    DWORD ver = ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.","PatternVer");
    if (ver == 0)
    {
      ver = ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.","PatternVer1");
      if (ver == 0)
        ver = ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.","InternalPatternVer");
    }

    if (ver != 0)snprintf(bdd,DEFAULT_TMP_SIZE,"%lu",ver);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\TrendMicro\\PC-cillinNTCorp\\CurrentVersion","last_update",engine, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.","TmFilter-Ver",path, MAX_PATH);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Symantec
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\InstalledApps","SAV Install Directory",path, MAX_PATH) == FALSE)
    if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\InstalledApps","Savrt",path, MAX_PATH) == FALSE)
      if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\InstalledApps","SAVCE",path, MAX_PATH) == FALSE)
        if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\InstalledApps","NAV",path, MAX_PATH) == FALSE)path[0] = 0;

  if(path[0] != 0)
  {
    strncpy(name,"Symantec AntiVirus",DEFAULT_TMP_SIZE);
    strncpy(editor,"Symantec",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\Common Client","Version",engine, DEFAULT_TMP_SIZE);

    if (engine[0]==0)
    {
      char tmp_path[MAX_PATH];
      snprintf(tmp_path,MAX_PATH,"%s\\VERSION.DAT",path);
      GetPrivateProfileString("Versions","PublicRevision","",engine,DEFAULT_TMP_SIZE,tmp_path);
    }

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\Norton AntiVirus","AdvChkPath",path, MAX_PATH))
  {
    strncpy(name,"Norton AntiVirus",DEFAULT_TMP_SIZE);
    strncpy(editor,"Symantec",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\Norton AntiVirus","CurrentVersion",engine, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\Norton AntiVirus\\DefAnnuity","ServID",bdd, DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Symantec\\Norton AntiVirus\\Virus Defs\\LastUpdate\\","SystemTime",last_update, DEFAULT_TMP_SIZE);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Comodo
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\ComodoGroup\\CDI\\1","InstallProductPath",path, MAX_PATH))
  {
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\ComodoGroup\\CDI\\1","Product Name",name, DEFAULT_TMP_SIZE);
    strncpy(editor,"ComodoGroup",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\ComodoGroup\\CDI\\1","Product Version",engine, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //McAfee
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\VirusScan Enterprise\\CurrentVersion","szEngineVer",engine, DEFAULT_TMP_SIZE) == FALSE)
  {
    if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\VirusScan","szCurrentVersionNumber",engine, DEFAULT_TMP_SIZE) == FALSE)
    {
      if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\NetShieldNT\\CurrentVersion","szEngineVer",engine, DEFAULT_TMP_SIZE) == FALSE)engine[0] = 0;
      else strncpy(name,"McAfee NetShieldNT",DEFAULT_TMP_SIZE);
    }else strncpy(name,"McAfee VirusScan Suite",DEFAULT_TMP_SIZE);
  }else strncpy(name,"McAfee VirusScan Entreprise",DEFAULT_TMP_SIZE);

  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\VirusScan Enterprise\\CurrentVersion","szVirDefVer",bdd, DEFAULT_TMP_SIZE) == FALSE)
    if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\NetShieldNT\\CurrentVersion","szVirDefVer",bdd, DEFAULT_TMP_SIZE) == FALSE)bdd[0]=0;

  if(engine[0] != 0)
  {
    //date
    if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\VirusScan Enterprise\\CurrentVersion","szVirDefDate",last_update, DEFAULT_TMP_SIZE) == FALSE)
      if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\NetShieldNT\\CurrentVersion","szVirDefDate",last_update, DEFAULT_TMP_SIZE) == FALSE)last_update[0]=0;
  }else
  {
    //search version
    CleTmp = 0;
    if (RegOpenKey(HKEY_LOCAL_MACHINE,"SOFTWARE\\Network Associates\\TVD\\Shared Components\\VirusScan Engine",&CleTmp)==ERROR_SUCCESS)
    {
      DWORD i,nbSubKey = 0;
      if (RegQueryInfoKey (CleTmp,0,0,0,&nbSubKey,0,0,0,0,0,0,0)==ERROR_SUCCESS)
      {
        char key_path[MAX_PATH];
        char tmp_key[MAX_PATH];
        DWORD key_size;
        for (i=0;i<nbSubKey;i++)
        {
          key_size    = DEFAULT_TMP_SIZE;
          tmp_key[0]  = 0;
          if (RegEnumKeyEx (CleTmp,i,tmp_key,&key_size,0,0,0,0)==ERROR_SUCCESS)
          {
            snprintf(key_path,MAX_PATH,"SOFTWARE\\Network Associates\\TVD\\Shared Components\\VirusScan Engine\\%s",tmp_key);

            if(ReadValue(HKEY_LOCAL_MACHINE,key_path,"szDatDate",bdd, DEFAULT_TMP_SIZE) == FALSE)
              if(ReadValue(HKEY_LOCAL_MACHINE,key_path,"szDatVersion",bdd, DEFAULT_TMP_SIZE) == FALSE)bdd[0]=0;

            if(ReadValue(HKEY_LOCAL_MACHINE,key_path,"szEngineVer",url_update, DEFAULT_TMP_SIZE))
            {
              snprintf(engine,DEFAULT_TMP_SIZE,"%s",url_update);
              url_update[0] = 0;
            }
          }
        }
      }
      RegCloseKey(CleTmp);
    }
  }
  if(engine[0] != 0)
  {
    strncpy(editor,"McAfee",DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //other
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Wow6432Node\\McAfee\\AVEngine","szInstallDir32",path, DEFAULT_TMP_SIZE))
  {
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Wow6432Node\\McAfee\\DesktopProtection","Product",name, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Wow6432Node\\McAfee\\DesktopProtection","szProductVer",engine, DEFAULT_TMP_SIZE);

    snprintf(bdd,DEFAULT_TMP_SIZE,"%d.%d",ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Wow6432Node\\McAfee\\AVEngine","AVDatVersion"),
                                          ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Wow6432Node\\McAfee\\AVEngine","AVDatVersionMinor"));
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Wow6432Node\\McAfee\\AVEngine","AVDatDate",last_update, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,"McAfee/Intel",engine,"",bdd,last_update,"?",session_id,db);
  }

  //F-SECURE
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Data Fellows\\F-Secure\\Anti-Virus","CommDir",path, MAX_PATH))
  {
    strncpy(name,"F-Secure Suite",DEFAULT_TMP_SIZE);
    strncpy(editor,"F-Secure",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Data Fellows\\F-Secure\\Anti-Virus","CurrentVersionEx",engine, DEFAULT_TMP_SIZE);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //NOD32
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Eset\\Nod\\CurrentVersion\\Info","InstallDir",path, MAX_PATH))
  {
    strncpy(name,"Nod32",DEFAULT_TMP_SIZE);
    strncpy(editor,"Eset",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Eset\\Nod\\CurrentVersion\\Info","Build",engine, DEFAULT_TMP_SIZE);
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Eset\\Nod\\CurrentVersion\\Info","CurrentVersion",bdd, DEFAULT_TMP_SIZE);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //VirusGuard
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Softed\\ViGUARD\\Settings","Directory",path, MAX_PATH))
  {
    strncpy(name,"VirusGuard",DEFAULT_TMP_SIZE);
    strncpy(editor,"Softed",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Softed\\ViGUARD\\Settings","Revision",engine, DEFAULT_TMP_SIZE);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Zone alarme Antivirus
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Zone Labs\\ZoneAlarm","InstallDirectory",path, MAX_PATH))
  {
    strncpy(name,"ZoneAlarm",DEFAULT_TMP_SIZE);
    strncpy(editor,"Zone Labs",DEFAULT_TMP_SIZE);

    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Zone Labs\\ZoneAlarm","AVEngineVer",engine, DEFAULT_TMP_SIZE);
    snprintf(bdd,DEFAULT_TMP_SIZE,"%lu",ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Zone Labs\\ZoneAlarm","AVDatVersion"));
    ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Zone Labs\\ZoneAlarm","CCSServerURL",url_update, MAX_PATH);

    //date
    timeToString(ReadDwordValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\Zone Labs\\ZoneAlarm","LastAVUpdate")+252221453, last_update, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
void Scan_antivirus_file(HK_F_OPEN *hks, sqlite3 *db, unsigned int session_id)
{
  //test by Antivirus
  char path[MAX_PATH],
       name[MAX_PATH],
       editor[DEFAULT_TMP_SIZE],
       engine[DEFAULT_TMP_SIZE],
       url_update[MAX_PATH],
       bdd[DEFAULT_TMP_SIZE],
       last_update[DEFAULT_TMP_SIZE],
       tmp[MAX_PATH];

  //Windows Microsoft Essentiels
  url_update  [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Microsoft\\Microsoft Antimalware", NULL,"InstallSource", path, MAX_PATH))
  {
    strncpy(name,"Microsoft Security Essentials",DEFAULT_TMP_SIZE);
    strncpy(editor,"Microsoft",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Microsoft\\Microsoft Antimalware\\Signature Updates", NULL,"EngineVersion", engine, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Microsoft\\Microsoft Antimalware\\Signature Updates", NULL,"AVSignatureVersion", bdd, DEFAULT_TMP_SIZE);

    FILETIME FileTime;
    DWORD size = sizeof(FileTime);
    ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Microsoft\\Microsoft Antimalware\\Signature Updates", NULL, "SignaturesLastUpdated", (void*)&FileTime, &size);
    if (FileTime.dwHighDateTime ==0 && FileTime.dwLowDateTime == 0)last_update[0] = 0;
    else filetimeToString_GMT(FileTime, last_update, DATE_SIZE_MAX);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Microsoft\\Microsoft Antimalware\\Real-Time Protection", NULL,"DisableRealtimeMonitoring", tmp, MAX_PATH);
    if(strcmp(tmp,"0x00000001")==1)addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //G DATA
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data", NULL,"RootPath", path, MAX_PATH))
  {
    strncpy(editor,"G Data",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data\\AVKInternetSecurity", NULL,"AppTitle", name, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data\\AntiVirenKit\\OnlineUpdates", NULL,"ScanWL", engine, DEFAULT_TMP_SIZE);

    char tmp0[DEFAULT_TMP_SIZE]="", tmp1[DEFAULT_TMP_SIZE]="";
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data\\AntiVirenKit\\OnlineUpdates", NULL,"Engine0", tmp0, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data\\AntiVirenKit\\OnlineUpdates", NULL,"Engine1", tmp1, DEFAULT_TMP_SIZE);
    snprintf(bdd,DEFAULT_TMP_SIZE,"%s/%s",tmp0,tmp1);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data\\AntiVirenKit", NULL,"UpgradeCenterLink", url_update, MAX_PATH);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data\\AVStatus", NULL,"SigDate", last_update, DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "G Data\\IntelliScan", NULL,"Enabled", tmp, MAX_PATH);
    if(strcmp(tmp,"0x00000001")==1)addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Avira
  url_update  [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Avira\\AntiVir Desktop", NULL,"Path", path, MAX_PATH))
  {
    strncpy(name,"AntiVir",DEFAULT_TMP_SIZE);
    strncpy(editor,"Avira",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Avira\\AntiVir Desktop", NULL,"EngineVersion", engine, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Avira\\AntiVir Desktop", NULL,"VdfVersion", bdd, DEFAULT_TMP_SIZE);

    DWORD dTime;
    DWORD size = sizeof(DWORD);
    ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Avira\\AntiVir Desktop", NULL, "InstallationDate", (void*)&dTime, &size);
    if(dTime>0)timeToString(dTime, last_update, DATE_SIZE_MAX);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Avira\\AntiVir Desktop", NULL,"SecurityDetection", tmp, MAX_PATH);
    if(strcmp(tmp,"0x00000001")==1)addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Avast
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "AVAST Software\\Avast", NULL,"ProgramFolder", path, MAX_PATH))
  {
    strncpy(name,"Avast",DEFAULT_TMP_SIZE);
    strncpy(editor,"AVAST Software",DEFAULT_TMP_SIZE);

    char t[MAX_PATH]="",tmp2[MAX_PATH];
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "AVAST Software\\Avast", NULL,"Version", t, MAX_PATH);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "AVAST Software\\Avast", NULL,"SetupVersion", tmp2, DEFAULT_TMP_SIZE);
    snprintf(engine,DEFAULT_TMP_SIZE,"%s.%s",t,tmp2);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //bitdefender
  url_update  [0]=0;
  bdd         [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Bitdefender\\Gonzales\\Install", NULL,"InstallPath", path, MAX_PATH))
  {
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Bitdefender\\Gonzales", NULL,"WscDisplayName", name, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Bitdefender\\Gonzales", NULL,"WscManufacturerName", editor, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Bitdefender\\Gonzales", NULL,"ProductVersion", engine, DEFAULT_TMP_SIZE);

    DWORD dTime;
    DWORD size = sizeof(DWORD);
    ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Bitdefender\\Gonzales\\Update", NULL, "LastUpdateDate", (void*)&dTime, &size);
    if(dTime>0)timeToString(dTime, last_update, DATE_SIZE_MAX);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Bitdefender\\Gonzales", NULL,"ContScanState", tmp, MAX_PATH);
    if(strcmp(tmp,"0x00000001")==1)addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Panda Antivirus
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Panda Software\\Setup", NULL,"Path", path, MAX_PATH))
  {
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Panda Software\\Setup", NULL,"ProductName", name, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Panda Software\\Setup", NULL,"InitialCompanyName", editor, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Panda Software\\Setup", NULL,"Normal", engine, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Sophos antivirus
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  engine      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Sophos\\SAVService\\Application", NULL,"Path", path, MAX_PATH))
  {
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Sophos\\SAVService\\Application", NULL,"EventSource", name, DEFAULT_TMP_SIZE);
    strncpy(editor,"Sophos",DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Sophos\\AutoUpdate", NULL,"ProductVersion", engine, DEFAULT_TMP_SIZE);

    DWORD dTime;
    DWORD size = sizeof(DWORD);
    ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Sophos\\AutoUpdate\\UpdateStatus", NULL, "LastUpdateTime", (void*)&dTime, &size);
    if(dTime>0)timeToString(dTime, last_update, DATE_SIZE_MAX);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Sophos\\SAVService\\Components\\BackgroundScanFactory", NULL,"Level", tmp, MAX_PATH);
    if(strcmp(tmp,"0x00000001")==1)addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
    else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
  }

  //Rising Antivirus
  url_update  [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "rising\\RAV", NULL,"installpath", path, MAX_PATH))
  {
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "rising\\RAV", NULL,"name", name, DEFAULT_TMP_SIZE);
    strncpy(editor,"Rising",DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "rising\\RAV", NULL,"Version", engine, DEFAULT_TMP_SIZE);
    snprintf(bdd,DEFAULT_TMP_SIZE,"%s",engine);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //PC-cillinNTCorp
  url_update  [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.", NULL,"ProgramVer", engine, DEFAULT_TMP_SIZE) == FALSE)
    if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.", NULL,"EngineZipVer", engine, DEFAULT_TMP_SIZE) == FALSE)engine[0] = 0;

  if(engine[0] != 0)
  {
    strncpy(name,"Trend Micro Office Scan",DEFAULT_TMP_SIZE);
    strncpy(editor,"Trend Micro",DEFAULT_TMP_SIZE);

    if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.", NULL,"PatternVer", bdd, DEFAULT_TMP_SIZE) == FALSE)
      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.", NULL,"PatternVer1", bdd, DEFAULT_TMP_SIZE) == FALSE)
        Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.", NULL,"InternalPatternVer", bdd, DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "TrendMicro\\PC-cillinNTCorp\\CurrentVersion", NULL,"last_update", engine, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "TrendMicro\\PC-cillinNTCorp\\CurrentVersion\\Misc.", NULL,"TmFilter-Ver", path, MAX_PATH);

    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Symantec
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\InstalledApps", NULL,"SAV Install Directory", path, MAX_PATH) == FALSE)
    if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\InstalledApps", NULL,"Savrt", path, MAX_PATH) == FALSE)
      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\InstalledApps", NULL,"SAVCE", path, MAX_PATH) == FALSE)
        if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\InstalledApps", NULL,"NAV", path, MAX_PATH) == FALSE)path[0] = 0;

  if(path[0] != 0)
  {
    strncpy(name,"Symantec AntiVirus",DEFAULT_TMP_SIZE);
    strncpy(editor,"Symantec",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\Common Client", NULL,"Version", engine, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\Norton AntiVirus", NULL,"AdvChkPath", path, MAX_PATH))
  {
    strncpy(name,"Norton AntiVirus",DEFAULT_TMP_SIZE);
    strncpy(editor,"Symantec",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\Norton AntiVirus", NULL,"CurrentVersion", engine, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\Norton AntiVirus\\DefAnnuity", NULL,"ServID", bdd, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Symantec\\Norton AntiVirus\\Virus Defs\\LastUpdate", NULL,"SystemTime", last_update, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Comodo
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "ComodoGroup\\CDI\\1", NULL,"AdvChkPath", path, MAX_PATH))
  {
    strncpy(editor,"ComodoGroup",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "ComodoGroup\\CDI\\1", NULL,"Product Name", name, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "SOFTWARE\\ComodoGroup\\CDI\\1", NULL,"Product Version", engine, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //F-SECURE
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Data Fellows\\F-Secure\\Anti-Virus", NULL,"CommDir", path, MAX_PATH))
  {
    strncpy(name,"F-Secure Suite",DEFAULT_TMP_SIZE);
    strncpy(editor,"F-Secure",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Data Fellows\\F-Secure\\Anti-Virus", NULL,"CurrentVersionEx", engine, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //NOD32
  url_update  [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Eset\\Nod\\CurrentVersion\\Info", NULL,"InstallDir", path, MAX_PATH))
  {
    strncpy(name,"Nod32",DEFAULT_TMP_SIZE);
    strncpy(editor,"Eset",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Eset\\Nod\\CurrentVersion\\Info", NULL,"Build", engine, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Eset\\Nod\\CurrentVersion\\Info", NULL,"CurrentVersion", bdd, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //VirusGuard
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Softed\\ViGUARD\\Settings", NULL,"Directory", path, MAX_PATH))
  {
    strncpy(name,"VirusGuard",DEFAULT_TMP_SIZE);
    strncpy(editor,"Softed",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Softed\\ViGUARD\\Settings", NULL,"Revision", engine, DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //Zone alarme Antivirus
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Zone Labs\\ZoneAlarm", NULL,"InstallDirectory", path, MAX_PATH))
  {
    strncpy(name,"ZoneAlarm",DEFAULT_TMP_SIZE);
    strncpy(editor,"Zone Labs",DEFAULT_TMP_SIZE);

    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Zone Labs\\ZoneAlarm", NULL,"AVEngineVer", engine, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Zone Labs\\ZoneAlarm", NULL,"AVDatVersion", bdd, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Zone Labs\\ZoneAlarm", NULL,"CCSServerURL", url_update, MAX_PATH);

    DWORD dTime=0;
    DWORD size = sizeof(DWORD);
    ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Zone Labs\\ZoneAlarm", NULL, "LastAVUpdate", (void*)&dTime, &size);
    if(dTime>0)timeToString(dTime+252221453, last_update, DATE_SIZE_MAX);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  //AVG
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Avg");
  if (nk_h != NULL)
  {
    char key_path[MAX_PATH];
    DWORD i, nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
    strncpy(editor,"Avg",DEFAULT_TMP_SIZE);
    for (i=0;i<nbSubKey;i++)
    {
      //for each subkey
      if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, i, name, MAX_PATH)==FALSE)continue;

      snprintf(key_path,MAX_PATH,"Avg\\%s",name);
      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path, NULL,"AvgDir", path, MAX_PATH);
      snprintf(key_path,MAX_PATH,"Avg\\%s\\LinkScanner\\Prevalence",name);
      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path, NULL,"CODEVER", engine, DEFAULT_TMP_SIZE);
      snprintf(key_path,MAX_PATH,"Avg\\%s\\Features\\Core",name);
      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path, NULL,"fea_CORE", bdd, DEFAULT_TMP_SIZE);

      //read the last update ^^
      snprintf(key_path,MAX_PATH,"SOFTWARE\\Avg\\%s\\Features",name);
      Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path, NULL,"fea_Base", url_update, DEFAULT_TMP_SIZE);
      snprintf(last_update,DATE_SIZE_MAX,"%lu",atol(url_update+2));

      FILETIME FileTime;
      DWORD size = sizeof(FILETIME);
      snprintf(key_path,MAX_PATH,"Avg\\%s\\InstallTimes",name);
      if(ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path, NULL, last_update, (void*)&FileTime, &size))
      {
        if (FileTime.dwHighDateTime ==0 && FileTime.dwLowDateTime == 0)last_update[0] = 0;
        else filetimeToString_GMT(FileTime, last_update, DATE_SIZE_MAX);
      }else last_update[0] = 0;

      url_update[0] = 0;
      addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
    }
  }

  //Kaspersky
  HBIN_CELL_NK_HEADER *nk_h_tmp;
  nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "KasperskyLab\\protected");
  if (nk_h != NULL)
  {
    char key_path[MAX_PATH], tmp_key[MAX_PATH];
    DWORD i, nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
    for (i=0;i<nbSubKey;i++)
    {
      //for each subkey
      if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, i, tmp_key, MAX_PATH))
      {
        snprintf(key_path,MAX_PATH,"KasperskyLab\\protected\\%s\\environment",tmp_key);
        nk_h_tmp = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path);
        if (nk_h_tmp == NULL)continue;

        if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"ProductName", name, DEFAULT_TMP_SIZE))
        {
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"CompanyName", editor, DEFAULT_TMP_SIZE);
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"ProductRoot", path, MAX_PATH);
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"ProductVersion", engine, DEFAULT_TMP_SIZE);
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"LicUpgradeLink", url_update, MAX_PATH);
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstallBasesID", bdd, DEFAULT_TMP_SIZE);

          DWORD dTime=0;
          DWORD size = sizeof(DWORD);
          ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp, "dTime", (void*)&dTime, &size);
          if(dTime>0)timeToString(dTime, last_update, DEFAULT_TMP_SIZE);

          snprintf(key_path,MAX_PATH,"KasperskyLab\\protected\\%s",tmp_key);
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path, NULL,"enable", tmp, MAX_PATH);
          if(strcmp(tmp,"0x00000001")==1)addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"X",session_id,db);
          else addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"",session_id,db);
        }
      }
    }
  }

  //Coranti
  nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "SOFTWARE\\Coranti");
  if (nk_h != NULL)
  {
    char key_path[MAX_PATH];
    DWORD i, nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
    strncpy(editor,"Coranti",DEFAULT_TMP_SIZE);
    for (i=0;i<nbSubKey;i++)
    {
      if(GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, i, name, MAX_PATH))
      {
        snprintf(key_path,MAX_PATH,"KasperskyLab\\protected\\%s\\environment",name);
        nk_h_tmp = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, key_path);
        if (nk_h_tmp == NULL)continue;

        if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"ProductVersion", engine, DEFAULT_TMP_SIZE))
        {
          Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"InstallLocation", path, MAX_PATH);
          addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
        }
      }
    }
  }

  //McAfee
  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Network Associates\\TVD\\VirusScan Enterprise\\CurrentVersion", NULL,"szEngineVer", engine, DEFAULT_TMP_SIZE))
    strncpy(name,"McAfee VirusScan Entreprise",DEFAULT_TMP_SIZE);
  else if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Network Associates\\TVD\\VirusScan", NULL,"szCurrentVersionNumber", engine, DEFAULT_TMP_SIZE))
    strncpy(name,"McAfee VirusScan Suite",DEFAULT_TMP_SIZE);
  else if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Network Associates\\TVD\\NetShieldNT\\CurrentVersion", NULL,"szEngineVer", engine, DEFAULT_TMP_SIZE))
    strncpy(name,"McAfee NetShieldNT",DEFAULT_TMP_SIZE);
  else engine[0]=0;

  if(engine[0] != 0)
  {
    if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Network Associates\\TVD\\VirusScan Enterprise\\CurrentVersion", NULL,"szVirDefDate", last_update, DEFAULT_TMP_SIZE)==FALSE)
       if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Network Associates\\TVD\\NetShieldNT\\CurrentVersion", NULL,"szVirDefDate", last_update, DEFAULT_TMP_SIZE)==FALSE)
         last_update [0]=0;
  }else
  {
    nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "Network Associates\\TVD\\Shared Components\\VirusScan Engine");
    if (nk_h != NULL)
    {
      DWORD i, nbSubKey = GetSubNK(hks->buffer, hks->taille_fic, nk_h, hks->position, 0, NULL, 0);
      for (i=0;i<nbSubKey;i++)
      {
        nk_h_tmp = GetSubNKtonk(hks->buffer, hks->taille_fic, nk_h, hks->position, i);
        if (nk_h_tmp == NULL)continue;

        if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"szDatDate", bdd, DEFAULT_TMP_SIZE) == FALSE)
          if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"szDatVersion", bdd, DEFAULT_TMP_SIZE) == FALSE)
            bdd[0]=0;

        if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, NULL, nk_h_tmp,"szEngineVer", url_update, DEFAULT_TMP_SIZE) == FALSE)
        {
            snprintf(engine,DEFAULT_TMP_SIZE,"%s",url_update);
            url_update[0] = 0;
        }
      }
    }
  }

  if(engine[0] != 0)
  {
    strncpy(editor,"McAfee",DEFAULT_TMP_SIZE);
    addCAntivirustoDB(path,name,editor,engine,url_update,bdd,last_update,"?",session_id,db);
  }

  path        [0]=0;
  name        [0]=0;
  editor      [0]=0;
  url_update  [0]=0;
  bdd         [0]=0;
  last_update [0]=0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\AVEngine", NULL,"szInstallDir32", path, DEFAULT_TMP_SIZE))
  {
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\DesktopProtection", NULL,"Product", name, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\DesktopProtection", NULL,"szProductVer", engine, DEFAULT_TMP_SIZE);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\AVEngine", NULL,"AVDatDate", last_update, DEFAULT_TMP_SIZE);

    /*DWORD a=0, b=0, sz_a=4, sz_b = 4;
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\AVEngine", NULL,"AVDatVersion", &a, 4);
    Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\AVEngine", NULL,"AVDatVersionMinor", &b, 4);
    //ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\AVEngine", NULL,"AVDatVersion", &a, &sz_a);
    //ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "McAfee\\AVEngine", NULL,"AVDatVersionMinor", &b, &sz_b);
    snprintf(bdd,DEFAULT_TMP_SIZE,"%d.%d",a,b);*/

    addCAntivirustoDB(path,name,"McAfee/Intel",engine,"",bdd,last_update,"?",session_id,db);
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_antivirus(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;

  char file[MAX_PATH];
  HK_F_OPEN hks;

  //files or local
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
  if (hitem!=NULL  || !LOCAL_SCAN) //files
  {
    #ifdef CMD_LINE_ONLY_NO_DB
    printf("\"ANTIVIRUS\";\"path\";\"name\";\"editor\";\"engine\";\"bdd\";\"url_update\";\"last_update\";\"enable\";\"session_id\"\r\n");
    #endif
    while(hitem!=NULL)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      if (file[0] != 0)
      {
        //open file + verify
        if(OpenRegFiletoMem(&hks, file))
        {
          Scan_antivirus_file(&hks,db, session_id);
          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    #ifdef CMD_LINE_ONLY_NO_DB
    printf("\"ANTIVIRUS\";\"path\";\"name\";\"editor\";\"engine\";\"bdd\";\"url_update\";\"last_update\";\"enable\";\"session_id\";\r\n");
    #endif

    Scan_antivirus_local(db, session_id);
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
