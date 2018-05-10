//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addPasswordtoDB(char *source, char*login, char*password, char*raw_password,unsigned int description_id,unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_account_password (source,login,password,raw_password,description_id,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",%d,%d);",
           source,login,password,raw_password,description_id,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Password\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\"%d\";\r\n",
         source,login,password,raw_password,description_id,session_id);
  #endif
}
//------------------------------------------------------------------------------
void vncpwd(unsigned char *pwd, int bytelen)
{     // if bytelen is 0 it's a hex string
  int len, tmp;
  unsigned char  fixedkey[8] = { 23,82,107,6,35,78,88,7 }, *p, *o;

  if(bytelen)
  {
    o = pwd + bytelen;
  }else
  {
    for(p = o = pwd; *p; p += 2, o++)
    {
      sscanf(p, "%02x", &tmp);
      *o = tmp;
    }
  }

  len = o - pwd;
  tmp = len % 8;
  len /= 8;
  if(tmp) len++;

  deskey(fixedkey, 1);
  for(p = pwd; len--; p += 8) des(p, p);

  *o = 0;
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void Scan_registry_password_local(sqlite3 *db,unsigned int session_id)
{
  char login[MAX_PATH], password[MAX_PATH], raw_password[MAX_PATH],tmp[MAX_PATH], source[MAX_PATH];

  //VNC3
  login[0] = 0;
  password[0] = 0;
  raw_password[0] = 0;

  if (ReadValue(HKEY_CURRENT_USER,"SOFTWARE\\ORL\\WinVNC3","Password",password, MAX_PATH) == FALSE)
  {
    if (ReadValue(HKEY_CURRENT_USER,"SOFTWARE\\ORL\\WinVNC3\\Default","Password",password, MAX_PATH) == FALSE)
    {
      if (ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\ORL\\WinVNC3","Password",password, MAX_PATH) == FALSE)
      {
        ReadValue(HKEY_LOCAL_MACHINE,"SOFTWARE\\ORL\\WinVNC3\\Default","Password",password, MAX_PATH);
        strcpy(source,"HKEY_LOCAL_MACHINE\\SOFTWARE\\ORL\\WinVNC3\\Default\0");
      }else strcpy(source,"HKEY_LOCAL_MACHINE\\SOFTWARE\\ORL\\WinVNC3\0");
    }else strcpy(source,"HKEY_CURRENT_USER\\SOFTWARE\\ORL\\WinVNC3\\Default\0");
  }else strcpy(source,"HKEY_CURRENT_USER\\SOFTWARE\\ORL\\WinVNC3\0");



  if (strlen(password)>3)
  {
    snprintf(raw_password,MAX_PATH,"0x%02x%02x%02x%02x",password[0]&255,password[1]&255,password[2]&255,password[3]&255);
    vncpwd((u_char *)password,8);
    addPasswordtoDB(source,login,password,raw_password,REG_PASSWORD_STRING_VNC,session_id,db);
  }

  //Screen saver
  login[0] = 0;
  password[0] = 0;
  raw_password[0] = 0;
  tmp[0] = 0;

  if (ReadValue(HKEY_CURRENT_USER,".DEFAULT\\CONTROL PANEL\\DESKTOP","ScreenSave_Data",tmp, MAX_PATH) == FALSE)
  {
    ReadValue(HKEY_USERS,".DEFAULT\\CONTROL PANEL\\DESKTOP","ScreenSave_Data",tmp, MAX_PATH);
    strcpy(source,"HKEY_USERS\\.DEFAULT\\CONTROL PANEL\\DESKTOP\0");
  }else strcpy(source,"HKEY_CURRENT_USER\\.DEFAULT\\CONTROL PANEL\\DESKTOP\0");


  if (tmp[0] != 0)
  {
    int i,j=0,k=0,size = strlen(tmp);
    if (size>0)
    {
      if (size>0xFF)size=0xFF;

      //permet déchiffrement (codage XOR avec la clé de windows)
      const BYTE key[128] = {
      0x48, 0xEE, 0x76, 0x1D, 0x67, 0x69, 0xA1, 0x1B, 0x7A, 0x8C, 0x47, 0xF8, 0x54, 0x95, 0x97, 0x5F,
      0x78, 0xD9, 0xDA, 0x6C, 0x59, 0xD7, 0x6B, 0x35, 0xC5, 0x77, 0x85, 0x18, 0x2A, 0x0E, 0x52, 0xFF,
      0x00, 0xE3, 0x1B, 0x71, 0x8D, 0x34, 0x63, 0xEB, 0x91, 0xC3, 0x24, 0x0F, 0xB7, 0xC2, 0xF8, 0xE3,
      0xB6, 0x54, 0x4C, 0x35, 0x54, 0xE7, 0xC9, 0x49, 0x28, 0xA3, 0x85, 0x11, 0x0B, 0x2C, 0x68, 0xFB,
      0xEE, 0x7D, 0xF6, 0x6C, 0xE3, 0x9C, 0x2D, 0xE4, 0x72, 0xC3, 0xBB, 0x85, 0x1A, 0x12, 0x3C, 0x32,
      0xE3, 0x6B, 0x4F, 0x4D, 0xF4, 0xA9, 0x24, 0xC8, 0xFA, 0x78, 0xAD, 0x23, 0xA1, 0xE4, 0x6D, 0x9A,
      0x04, 0xCE, 0x2B, 0xC5, 0xB6, 0xC5, 0xEF, 0x93, 0x5C, 0xA8, 0x85, 0x2B, 0x41, 0x37, 0x72, 0xFA,
      0x57, 0x45, 0x41, 0xA1, 0x20, 0x4F, 0x80, 0xB3, 0xD5, 0x23, 0x02, 0x64, 0x3F, 0x6C, 0xF1, 0x0F};

      strcpy(raw_password,tmp);
      for (i=0; i<size/2; i++)
      {
        k = 0;
        // transformer en entier
        j -= (j=tmp[i*2  ])>0x39 ? 0x37:0x30;
        k += j*0x10;
        j -= (j=tmp[i*2+1])>0x39 ? 0x37:0x30;
        k += j;
        // effectuer le décryptage
        k ^= key[i];
        tmp[i] = k;
      }
      tmp[size/2] = 0;
      strcpy(password,tmp);

      addPasswordtoDB(source,login,password,raw_password,REG_PASSWORD_STRING_SCREENSAVER,session_id,db);
    }
  }

  //terminal server
  login[0] = 0;
  password[0] = 0;
  raw_password[0] = 0;

  if (ReadValue(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\DefaultUserConfiguration","password",raw_password, MAX_PATH) == FALSE)
  {
    if (ReadValue(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\Console","password",raw_password, MAX_PATH) == FALSE)
    {
      ReadValue(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp","password",raw_password, MAX_PATH);
      strcpy(source,"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\RDP-Tcp\0");
    }else strcpy(source,"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\WinStations\\Console\0");
  }else strcpy(source,"HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Terminal Server\\DefaultUserConfiguration\0");

  if (raw_password[0] != 0)
  {
    addPasswordtoDB(source,login,raw_password,raw_password,REG_PASSWORD_STRING_TERMINAL_SERVER,session_id,db);
  }

  //Session auto login (win 98,2000, NT)
  tmp[0] = 0;
  login[0] = 0;
  password[0] = 0;
  raw_password[0] = 0;

  ReadValue(HKEY_LOCAL_MACHINE,"Software\\Windows NT\\CurrentVersion\\Winlogon","DefaultUserName",password, MAX_PATH);
  ReadValue(HKEY_LOCAL_MACHINE,"Software\\Windows NT\\CurrentVersion\\Winlogon","DefaultPassword",raw_password, MAX_PATH);
  ReadValue(HKEY_LOCAL_MACHINE,"Software\\Windows NT\\CurrentVersion\\Winlogon","DefaultDomainName",tmp, MAX_PATH);

  if (tmp[0] != 0 || password[0] != 0 || raw_password[0] != 0)
  {
    snprintf(login,MAX_PATH,"%s\\%s",tmp,password); //domaine\\user
    addPasswordtoDB("HKEY_LOCAL_MACHINE\\Software\\Windows NT\\CurrentVersion\\Winlogon",login,raw_password,raw_password,REG_PASSWORD_STRING_AUTO_LOGON,session_id,db);
  }

  //--------------------------------------------------
  //read login+password of AD
  //--------------------------------------------------

 //--------------------------------------------------
  //read LSa secrets
  //--------------------------------------------------

  //--------------------------------------------------
  //read login+password of mscache
  //--------------------------------------------------
/*
  //get LSAKEY
  char LSA_key[MAX_LINE_SIZE]="", NLKM_key[MAX_LINE_SIZE]="";

  BOOL ok_LSA_key = FALSE;
  //Windows Vista,7,8,2008 OS
  set_sam_tree_access(HKEY_LOCAL_MACHINE, "SECURITY\\Policy");
  if (ReadValue(HKEY_LOCAL_MACHINE,"SECURITY\\Policy\\PolEKList","",LSA_key, MAX_LINE_SIZE))
  {
    ok_LSA_key = TRUE;
  }else
  {
    //Windows XP,2000,2003
    if (ReadValue(HKEY_LOCAL_MACHINE,"SECURITY\\Policy\\PolSecretEncryptionKey","",LSA_key, MAX_LINE_SIZE))
    {
      ok_LSA_key = TRUE;
    }
  }
  restore_sam_tree_access(HKEY_LOCAL_MACHINE, "SECURITY\\Policy");

  //get NLKM
  BOOL ok_NLKM_key = FALSE;
  set_sam_tree_access(HKEY_LOCAL_MACHINE, "SECURITY\\Policy\\Secrets");
  if (ReadValue(HKEY_LOCAL_MACHINE,"SECURITY\\Policy\\Secrets\\NL$KM\\CurrVal","",LSA_key, MAX_LINE_SIZE))ok_NLKM_key = TRUE;
  restore_sam_tree_access(HKEY_LOCAL_MACHINE, "SECURITY\\Policy\\Secrets");
*/
/*in coding*/

}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
void Scan_registry_password_file(HK_F_OPEN *hks,sqlite3 *db,unsigned int session_id)
{
  char login[MAX_PATH], password[MAX_PATH], raw_password[MAX_PATH],tmp[MAX_PATH], source[MAX_PATH];
  DWORD size;

  //VNC3
  login[0]        = 0;
  password[0]     = 0;
  raw_password[0] = 0;
  source[0]       = 0;

  size = MAX_PATH;
  if(ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "software\\orl\\winvnc3", NULL, "password", password, &size) == FALSE)
  {
    size = MAX_PATH;
    if(ReadBinarynk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "software\\orl\\winvnc3\\default", NULL, "password", password, &size))
      snprintf(source,MAX_PATH,"%s software\\orl\\winvnc3\\default",hks->file);
  }else snprintf(source,MAX_PATH,"%s software\\orl\\winvnc3",hks->file);

  if (size != MAX_PATH && size > 0 && source[0] != 0)
  {
    snprintf(raw_password,MAX_PATH,"0x%02x%02x%02x%02x",password[0]&255,password[1]&255,password[2]&255,password[3]&255);
    vncpwd((u_char *)password,8);
    addPasswordtoDB(source,login,password,raw_password,REG_PASSWORD_STRING_VNC,session_id,db);
  }

  //Screen saver
  login[0]        = 0;
  password[0]     = 0;
  raw_password[0] = 0;
  source[0]       = 0;
  tmp[0]          = 0;
  if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, ".default\\control panel\\desktop", NULL, "screensave_data", tmp, MAX_PATH))
  {
    if (tmp[0] != 0)
    {
      int i,j=0,k=0,size = strlen(tmp);
      if (size>0)
      {
        if (size>0xFF)size=0xFF;

        //permet déchiffrement (codage XOR avec la clé de windows)
        const BYTE key[128] = {
        0x48, 0xEE, 0x76, 0x1D, 0x67, 0x69, 0xA1, 0x1B, 0x7A, 0x8C, 0x47, 0xF8, 0x54, 0x95, 0x97, 0x5F,
        0x78, 0xD9, 0xDA, 0x6C, 0x59, 0xD7, 0x6B, 0x35, 0xC5, 0x77, 0x85, 0x18, 0x2A, 0x0E, 0x52, 0xFF,
        0x00, 0xE3, 0x1B, 0x71, 0x8D, 0x34, 0x63, 0xEB, 0x91, 0xC3, 0x24, 0x0F, 0xB7, 0xC2, 0xF8, 0xE3,
        0xB6, 0x54, 0x4C, 0x35, 0x54, 0xE7, 0xC9, 0x49, 0x28, 0xA3, 0x85, 0x11, 0x0B, 0x2C, 0x68, 0xFB,
        0xEE, 0x7D, 0xF6, 0x6C, 0xE3, 0x9C, 0x2D, 0xE4, 0x72, 0xC3, 0xBB, 0x85, 0x1A, 0x12, 0x3C, 0x32,
        0xE3, 0x6B, 0x4F, 0x4D, 0xF4, 0xA9, 0x24, 0xC8, 0xFA, 0x78, 0xAD, 0x23, 0xA1, 0xE4, 0x6D, 0x9A,
        0x04, 0xCE, 0x2B, 0xC5, 0xB6, 0xC5, 0xEF, 0x93, 0x5C, 0xA8, 0x85, 0x2B, 0x41, 0x37, 0x72, 0xFA,
        0x57, 0x45, 0x41, 0xA1, 0x20, 0x4F, 0x80, 0xB3, 0xD5, 0x23, 0x02, 0x64, 0x3F, 0x6C, 0xF1, 0x0F};

        strcpy(raw_password,tmp);
        for (i=0; i<size/2; i++)
        {
          k = 0;
          // transformer en entier
          j -= (j=tmp[i*2  ])>0x39 ? 0x37:0x30;
          k += j*0x10;
          j -= (j=tmp[i*2+1])>0x39 ? 0x37:0x30;
          k += j;
          // effectuer le décryptage
          k ^= key[i];
          tmp[i] = k;
        }
        tmp[size/2] = 0;
        strcpy(password,tmp);

        snprintf(source,MAX_PATH,"%s .default\\control panel\\desktop",hks->file);
        addPasswordtoDB(source,login,password,raw_password,REG_PASSWORD_STRING_SCREENSAVER,session_id,db);
      }
    }
  }

  //terminal server
  login[0] = 0;
  password[0] = 0;
  raw_password[0] = 0;
  if(!Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "controlset001\\control\\terminal server\\defaultuserconfiguration", NULL, "password", raw_password, MAX_PATH))
  {
    if(!Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "controlset001\\control\\terminal server\\winstations\\console", NULL, "password", raw_password, MAX_PATH))
    {
      if(Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "controlset001\\control\\terminal server\\winstations\\rdp-tcp", NULL, "password", raw_password, MAX_PATH))
      {
        snprintf(source,MAX_PATH,"%s controlset001\\control\\terminal server\\winstations\\rdp-tcp",hks->file);
        addPasswordtoDB(source,login,raw_password,raw_password,REG_PASSWORD_STRING_TERMINAL_SERVER,session_id,db);
      }
    }else
    {
      snprintf(source,MAX_PATH,"%s controlset001\\control\\terminal server\\winstations\\console",hks->file);
      addPasswordtoDB(source,login,raw_password,raw_password,REG_PASSWORD_STRING_TERMINAL_SERVER,session_id,db);
    }
  }else
  {
    snprintf(source,MAX_PATH,"%s controlset001\\control\\terminal server\\defaultuserconfiguration",hks->file);
    addPasswordtoDB(source,login,raw_password,raw_password,REG_PASSWORD_STRING_TERMINAL_SERVER,session_id,db);
  }

  //Session auto login
  login[0]        = 0;
  password[0]     = 0;
  raw_password[0] = 0;
  source[0]       = 0;
  tmp[0]          = 0;

  Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "software\\windows nt\\currentversion\\winlogon", NULL, "defaultusername", password, MAX_PATH);
  Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "software\\windows nt\\currentversion\\winlogon", NULL, "defaultpassword", raw_password, MAX_PATH);
  Readnk_Value(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, "software\\windows nt\\currentversion\\winlogon", NULL, "defaultdomainname", tmp, MAX_PATH);

  if (tmp[0] != 0 || password[0] != 0 || raw_password[0] != 0)
  {
    snprintf(login,MAX_PATH,"%s\\%s",tmp,password); //domaine\\user
    snprintf(source,MAX_PATH,"%s software\\windows nt\\currentversion\\winlogon",hks->file);
    addPasswordtoDB(source,login,raw_password,raw_password,REG_PASSWORD_STRING_AUTO_LOGON,session_id,db);
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_password(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  char file[MAX_PATH];
  HK_F_OPEN hks;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Password\";\"source\";\"login\";\"password\";\"raw_password\";\"description_id\";\"session_id\";\r\n");
  #endif
  //files or local
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
  if (hitem!=NULL || !LOCAL_SCAN) //files
  {
    while(hitem!=NULL)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      if (file[0] != 0)
      {
        //open file + verify
        if(OpenRegFiletoMem(&hks, file))
        {
          Scan_registry_password_file(&hks, db, session_id);
          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else Scan_registry_password_local(db,session_id);

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
