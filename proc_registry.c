//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "RtCA.h"
//-----------------------------------------------------------------------------
void OpenRegeditKey(char* chk, char *key)
{
  //write last key to use
  char tmp[MAX_PATH];
  BOOL ok = FALSE;
  snprintf(tmp,MAX_PATH,"\\%s\\%s",chk,key);
  HKEY CleTmp=0;
  DWORD cbData=strlen(tmp)+1;

  // on ouvre la cle
  if (RegOpenKey(HKEY_CURRENT_USER,"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Applets\\Regedit",&CleTmp)!=ERROR_SUCCESS)
     return;

 if (RegSetValueEx(CleTmp,"LastKey",0,REG_SZ,(const BYTE *)tmp,cbData)==ERROR_SUCCESS)ok = TRUE;

  //On ferme la cle
  RegCloseKey(CleTmp);

  //Open regedit
  if(ok)ShellExecute(h_main, "open","regedit","",NULL,SW_SHOW);
}
//-----------------------------------------------------------------------------
HKEY hkStringtohkey(char *chkey)
{
  if (chkey == NULL)return FALSE;
  if (strlen(chkey)<9)return FALSE;

       if (chkey[5]=='L') return HKEY_LOCAL_MACHINE;
  else if (chkey[5]=='U') return HKEY_USERS;
  else if (chkey[5]=='P') return HKEY_PERFORMANCE_DATA;
  else if (chkey[5]=='D') return HKEY_DYN_DATA;
  else if (chkey[6]=='L') return HKEY_CLASSES_ROOT;
  else if (chkey[13]=='U')return HKEY_CURRENT_USER;
  else if (chkey[14]=='C')return HKEY_CURRENT_CONFIG;

  return FALSE;
}
//-----------------------------------------------------------------------------
void GetRegistryKeyOwner(HKEY hKey, char* owner,char *rid, char *sid, unsigned int size_max)
{
  owner[0]=0;
  rid[0]=0;
  sid[0]=0;
  PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)GlobalAlloc(GMEM_FIXED,MAX_PATH);
  if (psd != NULL)
  {
    //read datas
    DWORD dwSize = MAX_PATH;
    if (RegGetKeySecurity(hKey, OWNER_SECURITY_INFORMATION, psd,&dwSize) == ERROR_SUCCESS)
    {
      PSID psid = NULL;
      BOOL pFlag = FALSE;
      if(GetSecurityDescriptorOwner(psd, &psid, &pFlag))
      {
        SidtoUser(psid, owner, rid, sid, size_max);
      }
    }
    GlobalFree(psd);
  }
}
//------------------------------------------------------------------------------
void ReadKeyUpdate(HKEY ENTETE,char *chemin, char *date, DWORD size_date)
{
  date[0]=0;
  HKEY CleTmp=0;

  FILETIME lastupdate;// dernière mise a jour ou creation de la cle
  if (RegOpenKey(ENTETE,chemin,&CleTmp)==ERROR_SUCCESS)
  {
    if(RegQueryInfoKey(CleTmp,0,0,0,0,0,0,0,0,0,0,&lastupdate)==ERROR_SUCCESS)
    {
      filetimeToString_GMT(lastupdate, date, size_date);
    }
    RegCloseKey(CleTmp);
  }
}
//------------------------------------------------------------------------------
DWORD ReadValue(HKEY hk,char *path,char *value,char *data, DWORD data_size)
{
  DWORD data_size_read = 0;
  HKEY CleTmp=0;

  //open key
  if (RegOpenKey(hk,path,&CleTmp)!=ERROR_SUCCESS)
     return FALSE;

  //size of data
  if (RegQueryValueEx(CleTmp, value, 0, 0, 0, &data_size_read)!=ERROR_SUCCESS)
     return FALSE;

  //alloc
  char *c = (char *)malloc(data_size_read+1);
  if (c == NULL)return FALSE;

  //read value
  if (RegQueryValueEx(CleTmp, value, 0, 0, (LPBYTE)c, &data_size_read)!=ERROR_SUCCESS)
  {
    free(c);
    return FALSE;
  }

  if (data_size_read<data_size) memcpy(data,c,data_size_read);
  else memcpy(data,c,data_size);

  //free + close
  free(c);
  RegCloseKey(CleTmp);
  return data_size_read;
}
//------------------------------------------------------------------------------
long int ReadDwordValue(HKEY hk,char *path,char *value)
{
  DWORD data;
  if (ReadValue(hk,path,value,(char*)&data, sizeof(data)))
  {
    return data;
  }
  return -1;
}
//------------------------------------------------------------------------------
void ReadFILETIMEValue(HKEY hk,char *path,char *value, FILETIME *ft)
{
  unsigned long long int val=0;
  ReadValue(hk,path,value,(char*)&val, 8);

  ft->dwLowDateTime   = (DWORD) val;
  ft->dwHighDateTime  = (DWORD) (val >> 32);
}
