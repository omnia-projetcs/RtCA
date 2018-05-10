//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRegistryMRUtoDB(char *file, char *hk, char *key, char*value, char *data, char*description_id, char *user, char* RID, char *sid, char *parent_key_update, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_mru (file,hk,key,value,data,description_id,user,rid,sid,parent_key_update,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%s,\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           file,hk,key,value,data,description_id,user,RID,sid,parent_key_update,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Registry_MRU\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,hk,key,value,data,description_id,user,RID,sid,parent_key_update,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
void reg_read_enum_MRUvalues(HKEY hk,char *chkey,char *key,char *exclu,char* description_id,unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,key,&CleTmp)!=ERROR_SUCCESS)return;

  DWORD nbValue,i,j;
  FILETIME last_update;
  if (RegQueryInfoKey (CleTmp,0,0,0,0,0,0,&nbValue,0,0,0,&last_update)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }

  //get date
  char parent_key_update[DATE_SIZE_MAX] = "";
  filetimeToString_GMT(last_update, parent_key_update, DATE_SIZE_MAX);

  //read USER + RID + SID
  char tmp[MAX_PATH];
  char user[MAX_PATH], RID[MAX_PATH], sid[MAX_PATH];
  GetRegistryKeyOwner(CleTmp, user, RID, sid, MAX_PATH);

  //enum values
  char value[MAX_PATH], data[MAX_PATH];
  DWORD valueSize,dataSize,type;
  for (i=0;i<nbValue && start_scan;i++)
  {
    valueSize = MAX_PATH;
    dataSize  = MAX_PATH;
    value[0]  = 0;
    data[0]   = 0;
    type      = 0;
    if (RegEnumValue (CleTmp,i,(LPTSTR)value,(LPDWORD)&valueSize,0,(LPDWORD)&type,(LPBYTE)data,(LPDWORD)&dataSize)==ERROR_SUCCESS)
    {
      //if (strcmp(charToLowChar(value),exclu) == 0)continue;
      switch(type)
      {
        case REG_EXPAND_SZ:
        case REG_SZ:
          convertStringToSQL(value, MAX_PATH);
          convertStringToSQL(data, MAX_PATH);
          addRegistryMRUtoDB("",chkey,key,value,data,description_id,user,RID,sid,parent_key_update,session_id,db);break;
        case REG_BINARY:
        case REG_LINK:
          snprintf(tmp,MAX_PATH,"%S",data);
          convertStringToSQL(value, MAX_PATH);
          convertStringToSQL(tmp, MAX_PATH);
          addRegistryMRUtoDB("",chkey,key,value,tmp,description_id,user,RID,sid,parent_key_update,session_id,db);break;
        case REG_MULTI_SZ:
          for (j=0;j<dataSize;j++)
          {
            if (data[j] == 0)data[j]=';';
          }
          convertStringToSQL(value, MAX_PATH);
          convertStringToSQL(data, MAX_PATH);
          addRegistryMRUtoDB("",chkey,key,value,data,description_id,user,RID,sid,parent_key_update,session_id,db);break;
      }
    }
  }
  RegCloseKey(CleTmp);
}
//------------------------------------------------------------------------------
void reg_read_enum_MRUNvalues(HKEY hk,char *chkey,char *key,char *exclu,char* description_id,unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,key,&CleTmp)!=ERROR_SUCCESS)return;

  DWORD nbValue,i,j;
  FILETIME last_update;
  if (RegQueryInfoKey (CleTmp,0,0,0,0,0,0,&nbValue,0,0,0,&last_update)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }

  //get date
  char parent_key_update[DATE_SIZE_MAX] = "";
  filetimeToString_GMT(last_update, parent_key_update, DATE_SIZE_MAX);

  //read USER + RID + SID
  char tmp[MAX_PATH];
  char user[MAX_PATH], RID[MAX_PATH], sid[MAX_PATH];
  GetRegistryKeyOwner(CleTmp, user, RID, sid, MAX_PATH);

  //enum values
  char value[MAX_PATH], data[MAX_PATH];
  DWORD valueSize,dataSize,type;
  for (i=0;i<nbValue && start_scan;i++)
  {
    valueSize = MAX_PATH;
    dataSize  = MAX_PATH;
    value[0]  = 0;
    data[0]   = 0;
    type      = 0;
    if (RegEnumValue (CleTmp,i,(LPTSTR)value,(LPDWORD)&valueSize,0,(LPDWORD)&type,(LPBYTE)data,(LPDWORD)&dataSize)==ERROR_SUCCESS)
    {
      if (Contient(charToLowChar(value),exclu))
      {
        switch(type)
        {
          case REG_EXPAND_SZ:
          case REG_SZ:
            convertStringToSQL(value, MAX_PATH);
            convertStringToSQL(data, MAX_PATH);
            addRegistryMRUtoDB("",chkey,key,value,data,description_id,user,RID,sid,parent_key_update,session_id,db);break;
          case REG_BINARY:
          case REG_LINK:
            snprintf(tmp,MAX_PATH,"%S",data);
            convertStringToSQL(value, MAX_PATH);
            convertStringToSQL(tmp, MAX_PATH);
            addRegistryMRUtoDB("",chkey,key,value,tmp,description_id,user,RID,sid,parent_key_update,session_id,db);break;
          case REG_MULTI_SZ:
            for (j=0;j<dataSize;j++)
            {
              if (data[j] == 0)data[j]=';';
            }
            convertStringToSQL(value, MAX_PATH);
            convertStringToSQL(data, MAX_PATH);
            addRegistryMRUtoDB("",chkey,key,value,data,description_id,user,RID,sid,parent_key_update,session_id,db);break;
        }
      }
    }
  }
  RegCloseKey(CleTmp);
}
//------------------------------------------------------------------------------
void reg_read_enum_MRUWvalues(HKEY hk,char *chkey,char *key,char *exclu,char* description_id,unsigned int session_id, sqlite3 *db)
{
  HKEY CleTmp;
  if (RegOpenKey(hk,key,&CleTmp)!=ERROR_SUCCESS)return;

  DWORD nbValue,i;
  FILETIME last_update;
  if (RegQueryInfoKey (CleTmp,0,0,0,0,0,0,&nbValue,0,0,0,&last_update)!=ERROR_SUCCESS)
  {
    RegCloseKey(CleTmp);
    return;
  }

  //get date
  char parent_key_update[DATE_SIZE_MAX] = "";
  filetimeToString_GMT(last_update, parent_key_update, DATE_SIZE_MAX);

  //read USER + RID + SID
  char user[MAX_PATH], RID[MAX_PATH], sid[MAX_PATH];
  GetRegistryKeyOwner(CleTmp, user, RID, sid, MAX_PATH);

  //enum values
  char value[MAX_PATH], data[MAX_PATH], data_s[MAX_LINE_SIZE];
  DWORD valueSize,dataSize,type;
  for (i=0;i<nbValue && start_scan;i++)
  {
    valueSize = MAX_PATH;
    dataSize  = MAX_PATH;
    value[0]  = 0;
    data[0]   = 0;
    type      = 0;
    if (RegEnumValue (CleTmp,i,(LPTSTR)value,(LPDWORD)&valueSize,0,(LPDWORD)&type,(LPBYTE)data,(LPDWORD)&dataSize)==ERROR_SUCCESS)
    {
      convertStringToSQL(value, MAX_PATH);
      snprintf(data_s,MAX_LINE_SIZE,"%S",data);
      convertStringToSQL(data_s, MAX_PATH);
      addRegistryMRUtoDB("",chkey,key,value,data_s,description_id,user,RID,sid,parent_key_update,session_id,db);
    }
  }
  RegCloseKey(CleTmp);
}
//------------------------------------------------------------------------------
void ReadDatas(unsigned int type, HKEY hk, char *chk, char *key_path, char *value, char*description_id, unsigned int session_id, sqlite3 *db)
{
  switch(type)//value_type
  {
    //list of all string in a directory and exclude "value"
    case TYPE_ENUM_STRING_VALUE:reg_read_enum_MRUvalues(hk, chk,key_path,value,description_id, session_id, db);break;
    case TYPE_ENUM_STRING_NVALUE:reg_read_enum_MRUNvalues(hk, chk,key_path,value,description_id, session_id, db);break;
    case TYPE_ENUM_STRING_WVALUE:reg_read_enum_MRUWvalues(hk, chk,key_path,value,description_id, session_id, db);break;

    case TYPE_ENUM_SUBNK_DATE:
    {
      //read all subkey
      HKEY CleTmp, CleTmp2;
      if (RegOpenKey(hk,key_path,&CleTmp)!=ERROR_SUCCESS)return;

      //enum keys
      DWORD nbSubKey=0,i;
      if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
      {
        RegCloseKey(CleTmp);
        return;
      }

      char key[MAX_PATH], tmp_key[MAX_PATH], lastupdate[DATE_SIZE_MAX] ="";
      DWORD key_size;
      FILETIME LastWriteTime;
      char user[MAX_PATH], RID[MAX_PATH], sid[MAX_PATH];

      for (i=0;i<nbSubKey && start_scan;i++)
      {
        key[0]    = 0;
        key_size  = MAX_PATH;
        if (RegEnumKeyEx (CleTmp,i,key,(LPDWORD)&key_size,NULL,NULL,NULL,&LastWriteTime)==ERROR_SUCCESS)
        {
          snprintf(tmp_key,MAX_PATH,"%s\\%s",key_path,key);
          if (RegOpenKey(hk,tmp_key,&CleTmp2)==ERROR_SUCCESS)
          {
            user[0] = 0;
            RID[0]  = 0;
            sid[0]  = 0;
            GetRegistryKeyOwner(CleTmp2, user, RID, sid, MAX_PATH);

            filetimeToString_GMT(LastWriteTime, lastupdate, DATE_SIZE_MAX);
            addRegistryMRUtoDB("",chk,tmp_key,"","",description_id,user,RID,sid,lastupdate,session_id,db);
            RegCloseKey(CleTmp2);
          }
        }
      }
      RegCloseKey(CleTmp);
    }
    break;

    case TYPE_DBL_ENUM_VALUE:
    {
      //read all subkey
      HKEY CleTmp, CleTmp2, CleTmp3;
      if (RegOpenKey(hk,key_path,&CleTmp)!=ERROR_SUCCESS)return;

      //enum keys
      DWORD nbSubKey=0, nbSubKey2,i,j;
      if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
      {
        RegCloseKey(CleTmp);
        return;
      }

      char key[MAX_PATH], key2[MAX_PATH], tmp_key[MAX_PATH], tmp_key2[MAX_PATH], lastupdate[DATE_SIZE_MAX] ="";
      DWORD key_size,key_size2;
      FILETIME LastWriteTime;
      char user[MAX_PATH], RID[MAX_PATH], sid[MAX_PATH], data[MAX_PATH];

      for (i=0;i<nbSubKey && start_scan;i++)
      {
        key[0]    = 0;
        key_size  = MAX_PATH;
        if (RegEnumKeyEx (CleTmp,i,key,(LPDWORD)&key_size,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
        {
          snprintf(tmp_key,MAX_PATH,"%s\\%s\\AVGeneral\\cRecentFiles",key_path,key);
          if (RegOpenKey(hk,tmp_key,&CleTmp2)==ERROR_SUCCESS)
          {
            nbSubKey2 = 0;
            if (RegQueryInfoKey (CleTmp2,NULL,NULL,NULL,&nbSubKey2,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
            {
              RegCloseKey(CleTmp2);
              continue;
            }

            for (j=0;j<nbSubKey2 && start_scan;j++)
            {
              key2[0]    = 0;
              key_size2  = MAX_PATH;
              if (RegEnumKeyEx (CleTmp2,j,key2,(LPDWORD)&key_size2,NULL,NULL,NULL,&LastWriteTime)==ERROR_SUCCESS)
              {
                snprintf(tmp_key2,MAX_PATH,"%s\\%s",tmp_key,key2);
                if (RegOpenKey(hk,tmp_key2,&CleTmp3)==ERROR_SUCCESS)
                {
                  user[0] = 0;
                  RID[0]  = 0;
                  sid[0]  = 0;
                  data[0] = 0;
                  GetRegistryKeyOwner(CleTmp3, user, RID, sid, MAX_PATH);
                  filetimeToString_GMT(LastWriteTime, lastupdate, DATE_SIZE_MAX);

                  if(ReadValue(hk,tmp_key2,value,data, MAX_PATH))
                  {
                    convertStringToSQL(data, MAX_PATH);
                    addRegistryMRUtoDB("",chk,tmp_key2,value,data,description_id,user,RID,sid,lastupdate,session_id,db);
                  }
                  RegCloseKey(CleTmp3);
                }
              }
            }
            RegCloseKey(CleTmp2);
          }
        }
      }
      RegCloseKey(CleTmp);
    }
    break;

    //all string under one key
    case TYPE_ENUM_STRING_RVALUE:
    {
      //read all subkey
      HKEY CleTmp;
      if (RegOpenKey(hk,key_path,&CleTmp)!=ERROR_SUCCESS)return;

      DWORD nbSubKey=0,i;
      if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
      {
        RegCloseKey(CleTmp);
        return;
      }

      char key[MAX_PATH], tmp_key[MAX_PATH];
      DWORD key_size;
      for (i=0;i<nbSubKey && start_scan;i++)
      {
        key[0]    = 0;
        key_size  = MAX_PATH;
        if (RegEnumKeyEx (CleTmp,i,key,(LPDWORD)&key_size,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
        {
          snprintf(tmp_key,MAX_PATH,"%s\\%s",key_path,key);
          reg_read_enum_MRUvalues(hk,chk,tmp_key,value,description_id, session_id, db_scan);
        }
      }
      RegCloseKey(CleTmp);

    }break;

    //all string under thow key + key
    case TYPE_ENUM_STRING_RRVALUE:
    {
      //read all subkey
      HKEY CleTmp,CleTmp2;
      if (RegOpenKey(hk,key_path,&CleTmp)!=ERROR_SUCCESS)return;

      DWORD nbSubKey=0,nbSubKey2,i,j;
      if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
      {
        RegCloseKey(CleTmp);
        return;
      }

      char key[MAX_PATH], tmp_key[MAX_PATH],key2[MAX_PATH], tmp_key2[MAX_PATH];
      DWORD key_size,key_size2;
      for (i=0;i<nbSubKey && start_scan;i++)
      {
        key[0]    = 0;
        key_size  = MAX_PATH;
        if (RegEnumKeyEx (CleTmp,i,key,(LPDWORD)&key_size,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
        {
          snprintf(tmp_key,MAX_PATH,"%s\\%s",key_path,key);

          //second key !!!
          if (RegOpenKey(hk,tmp_key,&CleTmp2)!=ERROR_SUCCESS)continue;

          nbSubKey2 = 0;
          if (RegQueryInfoKey (CleTmp2,NULL,NULL,NULL,&nbSubKey2,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
          {
            RegCloseKey(CleTmp2);
            continue;
          }

          for (j=0;j<nbSubKey2 && start_scan;j++)
          {
            key2[0]    = 0;
            key_size2  = MAX_PATH;
            if (RegEnumKeyEx (CleTmp2,j,key2,(LPDWORD)&key_size2,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
            {
              snprintf(tmp_key2,MAX_PATH,"%s\\%s\\%s",tmp_key,key2,value);
              reg_read_enum_MRUvalues(hk,chk,tmp_key2,"",description_id, session_id, db_scan);
            }
          }
          RegCloseKey(CleTmp2);
        }
      }
      RegCloseKey(CleTmp);

    }break;

    //all string under one key + key
    case TYPE_ENUM_STRING_R_VALUE:
    {
      //read all subkey
      HKEY CleTmp;
      if (RegOpenKey(hk,key_path,&CleTmp)!=ERROR_SUCCESS)return;

      DWORD nbSubKey=0,i;
      if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
      {
        RegCloseKey(CleTmp);
        return;
      }

      char key[MAX_PATH], tmp_key[MAX_PATH];
      DWORD key_size;
      for (i=0;i<nbSubKey && start_scan;i++)
      {
        key[0]    = 0;
        key_size  = MAX_PATH;
        if (RegEnumKeyEx (CleTmp,i,key,(LPDWORD)&key_size,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
        {
          snprintf(tmp_key,MAX_PATH,"%s\\%s\\%s",key_path,key,value);
          reg_read_enum_MRUvalues(hk,chk,tmp_key,"",description_id, session_id, db_scan);
        }
      }
      RegCloseKey(CleTmp);
    }break;

    //only one value
    case TYPE_VALUE_STRING:
    {
      char data[MAX_PATH];
      if (ReadValue(hk,key_path,value,data, MAX_PATH))
      {
        HKEY CleTmp;
        if (RegOpenKey(hk,key_path,&CleTmp)==ERROR_SUCCESS)
        {
          char user[MAX_PATH]="", RID[MAX_PATH]="", SID[MAX_PATH]="";
          GetRegistryKeyOwner(CleTmp, user, RID, SID, MAX_PATH);
          RegCloseKey(CleTmp);

          char parent_key_update[DATE_SIZE_MAX]="";
          ReadKeyUpdate(hk,key_path, parent_key_update, DATE_SIZE_MAX);

          convertStringToSQL(data, MAX_PATH);
          addRegistryMRUtoDB("",chk,key_path,value,data,description_id,user,RID,SID,parent_key_update,session_id,db_scan);
        }
      }
    }break;
    //only one value
    case TYPE_VALUE_WSTRING:
    {
      char data[MAX_PATH],tmp[MAX_PATH];
      if (ReadValue(hk,key_path,value,tmp, MAX_PATH))
      {
        HKEY CleTmp;
        if (RegOpenKey(hk,key_path,&CleTmp)==ERROR_SUCCESS)
        {
          snprintf(data,MAX_PATH,"%S",tmp);

          char user[MAX_PATH]="", RID[MAX_PATH]="", SID[MAX_PATH]="";
          GetRegistryKeyOwner(CleTmp, user, RID, SID, MAX_PATH);
          RegCloseKey(CleTmp);

          char parent_key_update[DATE_SIZE_MAX]="";
          ReadKeyUpdate(hk,key_path, parent_key_update, DATE_SIZE_MAX);

          convertStringToSQL(data, MAX_PATH);
          addRegistryMRUtoDB("",chk,key_path,value,data,description_id,user,RID,SID,parent_key_update,session_id,db_scan);
        }
      }
    }break;
  }
}
//------------------------------------------------------------------------------
int callback_sqlite_registry_mru_local(void *datas, int argc, char **argv, char **azColName)
{
  FORMAT_CALBAK_TYPE *type = datas;
  unsigned int session_id = current_session_id;
  switch(type->type)
  {
    case SQLITE_REGISTRY_TYPE_MRU:
    {
      HKEY hk = hkStringtohkey(argv[0]);

      //case
      if (hk == HKEY_USERS)
      {
        //read all subkey
        HKEY CleTmp;
        if (RegOpenKey((HKEY)HKEY_USERS,"",&CleTmp)!=ERROR_SUCCESS)return 0;

        DWORD nbSubKey=0,i;
        if (RegQueryInfoKey (CleTmp,NULL,NULL,NULL,&nbSubKey,NULL,NULL,NULL,NULL,NULL,NULL,NULL)!=ERROR_SUCCESS)
        {
          RegCloseKey(CleTmp);
          return 0;
        }

        char key[MAX_PATH], tmp_key[MAX_PATH];
        DWORD key_size;
        for (i=0;i<nbSubKey && start_scan;i++)
        {
          key[0]    = 0;
          key_size  = MAX_PATH;
          if (RegEnumKeyEx (CleTmp,i,key,(LPDWORD)&key_size,NULL,NULL,NULL,NULL)==ERROR_SUCCESS)
          {
            //if (key_size >9) //not for system
            {
              snprintf(tmp_key,MAX_PATH,"%s\\%s",key,argv[1]);
              ReadDatas(atoi(argv[3]), hk, argv[0], tmp_key, argv[2], argv[5], session_id, db_scan);
            }
          }
        }
        RegCloseKey(CleTmp);
      }else ReadDatas(atoi(argv[3]), hk, argv[0], argv[1], argv[2], argv[5], session_id, db_scan);
    }break;
  }
  return 0;
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
int callback_sqlite_registry_mru_file(void *datas, int argc, char **argv, char **azColName)
{
  FORMAT_CALBAK_TYPE *type = datas;
  unsigned int session_id = current_session_id;
  char tmp[MAX_LINE_SIZE];

  switch(type->type)
  {
    case SQLITE_REGISTRY_TYPE_MRU:
    {
      switch(atoi(argv[3]))//value_type
      {
        case TYPE_VALUE_STRING:
        case TYPE_VALUE_WSTRING:
          if (Readnk_Value(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,
                           argv[1], NULL, argv[2], tmp, MAX_LINE_SIZE))
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                         argv[1], NULL, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);
            //save
            convertStringToSQL(tmp, MAX_LINE_SIZE);
            addRegistryMRUtoDB(hks_mru.file,"",argv[1],argv[2],tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
          }
        break;
        case TYPE_ENUM_STRING_RVALUE://all string under one key
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH];
            DWORD i, nbSubValue = GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);

            for (i=0;i<nbSubValue && start_scan;i++)
            {
              if (GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_LINE_SIZE))
              {
                //if (strcmp(charToLowChar(value),argv[2]) != 0)
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru.file,"",argv[1],value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_VALUE://list of all string in a directory and exclude "value"
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH];
            DWORD i, nbSubValue = GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
            for (i=0;i<nbSubValue && start_scan;i++)
            {
              if (GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_LINE_SIZE))
              {
                //if (strcmp(charToLowChar(value),argv[2]) != 0)
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru.file,"",argv[1],value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_NVALUE://list of all string in a directory with "value"
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH];
            DWORD i, nbSubValue = GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
            for (i=0;i<nbSubValue && start_scan;i++)
            {
              if (GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_LINE_SIZE))
              {
                if (Contient(charToLowChar(value),argv[2]))
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru.file,"",argv[1],value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_WVALUE:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH],data[MAX_LINE_SIZE];
            DWORD i, nbSubValue = GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
            DWORD sz_value = MAX_LINE_SIZE;
            for (i=0;i<nbSubValue && start_scan;i++)
            {
              sz_value = MAX_LINE_SIZE;
              if (GetBinaryValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,&sz_value))
              {
                //save
                convertStringToSQL(value, MAX_PATH);
                snprintf(data,MAX_LINE_SIZE,"%S",tmp);
                convertStringToSQL(tmp, MAX_LINE_SIZE);
                addRegistryMRUtoDB(hks_mru.file,"",argv[1],value,data,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
              }
            }
          }
        }
        break;

        case TYPE_ENUM_SUBNK_DATE:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h!=NULL)
          {
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";

            //get values
            char value[MAX_PATH], tmp_key[MAX_PATH];
            DWORD i, nbSubnk = GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0);

            for (i=0;i<nbSubnk && start_scan;i++)
            {
              if (GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, i, value, MAX_PATH))
              {
                snprintf(tmp_key,MAX_PATH,"%s\\%s",argv[1],value);
                HBIN_CELL_NK_HEADER *nk_ht = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,tmp_key);

                if (nk_ht!=NULL)
                {
                  //key update
                  Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                               NULL, nk_ht, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);
                  //save
                  convertStringToSQL(tmp_key, MAX_PATH);
                  addRegistryMRUtoDB(hks_mru.file,"",tmp_key,"","",argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_DBL_ENUM_VALUE:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h==NULL)break;

          char parent_key_update[DATE_SIZE_MAX]="";
          char RID[MAX_PATH]="", sid[MAX_PATH]="", data[MAX_PATH];
          HBIN_CELL_NK_HEADER *nk_ht, *nk_ht2;

          //get values
          char value2[MAX_PATH],value[MAX_PATH], tmp_key2[MAX_PATH], tmp_key[MAX_PATH];
          DWORD i,j, nbSubnk2, nbSubnk = GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0);
          for (i=0;i<nbSubnk && start_scan;i++)
          {
            if (GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, i, value, MAX_PATH))
            {
              snprintf(tmp_key,MAX_PATH,"%s\\%s\\AVGeneral\\cRecentFiles",argv[1],value);
              nk_ht = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,tmp_key);

              nbSubnk2 = GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_ht, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0);
              for (j=0;j<nbSubnk2 && start_scan;j++)
              {
                if (GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_ht, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, j, value2, MAX_PATH))
                {
                  snprintf(tmp_key2,MAX_PATH,"%s\\%s",tmp_key,value2);
                  nk_ht2 = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,tmp_key2);

                  //datas
                  if(Readnk_Value(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position, NULL, nk_ht2, argv[2],
                                  data, MAX_PATH))
                  {
                    //key update
                    Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                                 NULL, nk_ht2, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

                    //save
                    convertStringToSQL(data, MAX_PATH);
                    addRegistryMRUtoDB(hks_mru.file,"",tmp_key2,argv[2],data,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                  }
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_RRVALUE://all string under thow key + key
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h == NULL)return 0;

          char parent_key_update[DATE_SIZE_MAX]="";
          char RID[MAX_PATH]="", sid[MAX_PATH]="";
          char value[MAX_PATH];

          char tmp_key[MAX_PATH], tmp_key2[MAX_PATH], key_path[MAX_PATH];
          HBIN_CELL_NK_HEADER *nk_h_tmp, *nk_h_tmp2;
          DWORD i,j,k, nbSubValue,nbSubKey2,nbSubKey = GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, hks_mru.position, 0, NULL, 0);
          for (i=0;i<nbSubKey && start_scan;i++)
          {
            if(GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, hks_mru.position, i, tmp_key, MAX_PATH))
            {
              //get nk of key :)
              nk_h_tmp = GetSubNKtonk(hks_mru.buffer, hks_mru.taille_fic, nk_h, hks_mru.position, i);
              if (nk_h_tmp == NULL)continue;

              nbSubKey2 = GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h_tmp, hks_mru.position, 0, NULL, 0);
              for (j=0;j<nbSubKey2 && start_scan;j++)
              {
                if(GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h_tmp, hks_mru.position, j, tmp_key2, MAX_PATH))
                {
                  //get nk of key :)
                  snprintf(key_path,MAX_PATH,"%s\\%s\\%s\\%s",argv[1],tmp_key,tmp_key2,argv[2]);
                  nk_h_tmp2 = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,key_path);
                  if (nk_h_tmp2 == NULL)continue;

                  //key update
                  Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                               NULL, nk_h_tmp2, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

                  //get values
                  nbSubValue = GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h_tmp2, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
                  for (k=0;k<nbSubValue;k++)
                  {
                    if (GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h_tmp2, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, k,value,MAX_PATH,tmp,MAX_LINE_SIZE))
                    {
                      //save
                      convertStringToSQL(value, MAX_PATH);
                      convertStringToSQL(tmp, MAX_LINE_SIZE);
                      addRegistryMRUtoDB(hks_mru.file,"",key_path,value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                    }
                  }
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_R_VALUE://all string under one key + key
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,argv[1]);
          if (nk_h == NULL)return 0;

          char parent_key_update[DATE_SIZE_MAX]="";
          char RID[MAX_PATH]="", sid[MAX_PATH]="";
          char value[MAX_PATH];

          char tmp_key[MAX_PATH], key_path[MAX_PATH];
          HBIN_CELL_NK_HEADER *nk_h_tmp, *nk_h_tmp2;
          DWORD i,k, nbSubValue,nbSubKey = GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, hks_mru.position, 0, NULL, 0);
          for (i=0;i<nbSubKey && start_scan;i++)
          {
            if(GetSubNK(hks_mru.buffer, hks_mru.taille_fic, nk_h, hks_mru.position, i, tmp_key, MAX_PATH))
            {
              //get nk of key :)
              nk_h_tmp = GetSubNKtonk(hks_mru.buffer, hks_mru.taille_fic, nk_h, hks_mru.position, i);
              if (nk_h_tmp == NULL)continue;

              snprintf(key_path,MAX_PATH,"%s\\%s\\%s",argv[1],tmp_key,argv[2]);
              nk_h_tmp2 = GetRegistryNK(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru.position,key_path);
              if (nk_h_tmp2 == NULL)continue;

              //key update
              Readnk_Infos(hks_mru.buffer,hks_mru.taille_fic, (hks_mru.pos_fhbin), hks_mru.position,
                           NULL, nk_h_tmp2, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

              //get values
              nbSubValue = GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h_tmp2, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
              for (k=0;k<nbSubValue;k++)
              {
                if (GetValueData(hks_mru.buffer,hks_mru.taille_fic, nk_h_tmp2, (hks_mru.pos_fhbin)+HBIN_HEADER_SIZE, k,value,MAX_PATH,tmp,MAX_LINE_SIZE))
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru.file,"",key_path,value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
      }
    }break;
    case SQLITE_REGISTRY_TYPE_MRU2:
    {
      switch(atoi(argv[3]))//value_type
      {
        case TYPE_VALUE_STRING:
        case TYPE_VALUE_WSTRING:
          if (Readnk_Value(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,
                           argv[1], NULL, argv[2], tmp, MAX_LINE_SIZE))
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                         argv[1], NULL, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);
            //save
            convertStringToSQL(tmp, MAX_LINE_SIZE);
            addRegistryMRUtoDB(hks_mru2.file,"",argv[1],argv[2],tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
          }
        break;
        case TYPE_ENUM_STRING_RVALUE://all string under one key
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH];
            DWORD i, nbSubValue = GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);

            for (i=0;i<nbSubValue && start_scan;i++)
            {
              if (GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_LINE_SIZE))
              {
                //if (strcmp(charToLowChar(value),argv[2]) != 0)
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru2.file,"",argv[1],value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_VALUE://list of all string in a directory and exclude "value"
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH];
            DWORD i, nbSubValue = GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
            for (i=0;i<nbSubValue && start_scan;i++)
            {
              if (GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_LINE_SIZE))
              {
                //if (strcmp(charToLowChar(value),argv[2]) != 0)
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru2.file,"",argv[1],value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_NVALUE://list of all string in a directory with "value"
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH];
            DWORD i, nbSubValue = GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
            for (i=0;i<nbSubValue && start_scan;i++)
            {
              if (GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_LINE_SIZE))
              {
                if (Contient(charToLowChar(value),argv[2]))
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru2.file,"",argv[1],value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_WVALUE:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";
            Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

            //get values
            char value[MAX_PATH],data[MAX_LINE_SIZE];
            DWORD i, nbSubValue = GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
            DWORD sz_value = MAX_LINE_SIZE;
            for (i=0;i<nbSubValue && start_scan;i++)
            {
              sz_value = MAX_LINE_SIZE;
              if (GetBinaryValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,&sz_value))
              {
                //save
                convertStringToSQL(value, MAX_PATH);
                snprintf(data,MAX_LINE_SIZE,"%S",tmp);
                convertStringToSQL(tmp, MAX_LINE_SIZE);
                addRegistryMRUtoDB(hks_mru2.file,"",argv[1],value,data,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
              }
            }
          }
        }
        break;

        case TYPE_ENUM_SUBNK_DATE:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h!=NULL)
          {
            char parent_key_update[DATE_SIZE_MAX]="";
            char RID[MAX_PATH]="", sid[MAX_PATH]="";

            //get values
            char value[MAX_PATH], tmp_key[MAX_PATH];
            DWORD i, nbSubnk = GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0);

            for (i=0;i<nbSubnk && start_scan;i++)
            {
              if (GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, i, value, MAX_PATH))
              {
                snprintf(tmp_key,MAX_PATH,"%s\\%s",argv[1],value);
                HBIN_CELL_NK_HEADER *nk_ht = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,tmp_key);

                if (nk_ht!=NULL)
                {
                  //key update
                  Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                               NULL, nk_ht, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);
                  //save
                  convertStringToSQL(tmp_key, MAX_PATH);
                  addRegistryMRUtoDB(hks_mru2.file,"",tmp_key,"","",argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
        case TYPE_DBL_ENUM_VALUE:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h==NULL)break;

          char parent_key_update[DATE_SIZE_MAX]="";
          char RID[MAX_PATH]="", sid[MAX_PATH]="", data[MAX_PATH];
          HBIN_CELL_NK_HEADER *nk_ht, *nk_ht2;

          //get values
          char value2[MAX_PATH],value[MAX_PATH], tmp_key2[MAX_PATH], tmp_key[MAX_PATH];
          DWORD i,j, nbSubnk2, nbSubnk = GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0);
          for (i=0;i<nbSubnk && start_scan;i++)
          {
            if (GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, i, value, MAX_PATH))
            {
              snprintf(tmp_key,MAX_PATH,"%s\\%s\\AVGeneral\\cRecentFiles",argv[1],value);
              nk_ht = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,tmp_key);

              nbSubnk2 = GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_ht, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0);
              for (j=0;j<nbSubnk2 && start_scan;j++)
              {
                if (GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_ht, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, j, value2, MAX_PATH))
                {
                  snprintf(tmp_key2,MAX_PATH,"%s\\%s",tmp_key,value2);
                  nk_ht2 = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,tmp_key2);

                  //datas
                  if(Readnk_Value(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position, NULL, nk_ht2, argv[2],
                                  data, MAX_PATH))
                  {
                    //key update
                    Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                                 NULL, nk_ht2, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

                    //save
                    convertStringToSQL(data, MAX_PATH);
                    addRegistryMRUtoDB(hks_mru2.file,"",tmp_key2,argv[2],data,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                  }
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_RRVALUE://all string under thow key + key
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h == NULL)return 0;

          char parent_key_update[DATE_SIZE_MAX]="";
          char RID[MAX_PATH]="", sid[MAX_PATH]="";
          char value[MAX_PATH];

          char tmp_key[MAX_PATH], tmp_key2[MAX_PATH], key_path[MAX_PATH];
          HBIN_CELL_NK_HEADER *nk_h_tmp, *nk_h_tmp2;
          DWORD i,j,k, nbSubValue,nbSubKey2,nbSubKey = GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, hks_mru2.position, 0, NULL, 0);
          for (i=0;i<nbSubKey && start_scan;i++)
          {
            if(GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, hks_mru2.position, i, tmp_key, MAX_PATH))
            {
              //get nk of key :)
              nk_h_tmp = GetSubNKtonk(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, hks_mru2.position, i);
              if (nk_h_tmp == NULL)continue;

              nbSubKey2 = GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h_tmp, hks_mru2.position, 0, NULL, 0);
              for (j=0;j<nbSubKey2 && start_scan;j++)
              {
                if(GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h_tmp, hks_mru2.position, j, tmp_key2, MAX_PATH))
                {
                  //get nk of key :)
                  snprintf(key_path,MAX_PATH,"%s\\%s\\%s\\%s",argv[1],tmp_key,tmp_key2,argv[2]);
                  nk_h_tmp2 = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,key_path);
                  if (nk_h_tmp2 == NULL)continue;

                  //key update
                  Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                               NULL, nk_h_tmp2, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

                  //get values
                  nbSubValue = GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h_tmp2, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
                  for (k=0;k<nbSubValue;k++)
                  {
                    if (GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h_tmp2, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, k,value,MAX_PATH,tmp,MAX_LINE_SIZE))
                    {
                      //save
                      convertStringToSQL(value, MAX_PATH);
                      convertStringToSQL(tmp, MAX_LINE_SIZE);
                      addRegistryMRUtoDB(hks_mru2.file,"",key_path,value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                    }
                  }
                }
              }
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_R_VALUE://all string under one key + key
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,argv[1]);
          if (nk_h == NULL)return 0;

          char parent_key_update[DATE_SIZE_MAX]="";
          char RID[MAX_PATH]="", sid[MAX_PATH]="";
          char value[MAX_PATH];

          char tmp_key[MAX_PATH], key_path[MAX_PATH];
          HBIN_CELL_NK_HEADER *nk_h_tmp, *nk_h_tmp2;
          DWORD i,k, nbSubValue,nbSubKey = GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, hks_mru2.position, 0, NULL, 0);
          for (i=0;i<nbSubKey && start_scan;i++)
          {
            if(GetSubNK(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, hks_mru2.position, i, tmp_key, MAX_PATH))
            {
              //get nk of key :)
              nk_h_tmp = GetSubNKtonk(hks_mru2.buffer, hks_mru2.taille_fic, nk_h, hks_mru2.position, i);
              if (nk_h_tmp == NULL)continue;

              snprintf(key_path,MAX_PATH,"%s\\%s\\%s",argv[1],tmp_key,argv[2]);
              nk_h_tmp2 = GetRegistryNK(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, hks_mru2.position,key_path);
              if (nk_h_tmp2 == NULL)continue;

              //key update
              Readnk_Infos(hks_mru2.buffer,hks_mru2.taille_fic, (hks_mru2.pos_fhbin), hks_mru2.position,
                           NULL, nk_h_tmp2, parent_key_update, DATE_SIZE_MAX, RID, MAX_PATH,sid, MAX_PATH);

              //get values
              nbSubValue = GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h_tmp2, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
              for (k=0;k<nbSubValue;k++)
              {
                if (GetValueData(hks_mru2.buffer,hks_mru2.taille_fic, nk_h_tmp2, (hks_mru2.pos_fhbin)+HBIN_HEADER_SIZE, k,value,MAX_PATH,tmp,MAX_LINE_SIZE))
                {
                  //save
                  convertStringToSQL(value, MAX_PATH);
                  convertStringToSQL(tmp, MAX_LINE_SIZE);
                  addRegistryMRUtoDB(hks_mru2.file,"",key_path,value,tmp,argv[5],"",RID,sid,parent_key_update,session_id,db_scan);
                }
              }
            }
          }
        }
        break;
      }
    }break;
  }
  return 0;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_mru(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  char file[MAX_PATH];
  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = SQLITE_REGISTRY_TYPE_MRU;

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Registry_MRU\";\"file\";\"hk\";\"key\";\"value\";\"data\";\"description_id\";\"user\";\"rid\";\"sid\";\"parent_key_update\";\"session_id\";\r\n");
  #endif
  //files or local
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
  if (hitem!=NULL || !LOCAL_SCAN) //files
  {
    while(hitem!=NULL && start_scan)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      if (file[0] != 0)
      {
        //open file + verify
        if(OpenRegFiletoMem(&hks_mru, file))
        {
          sqlite3_exec(db, "SELECT hkey,search_key,value,value_type,type_id,description_id FROM extract_registry_mru_request;", callback_sqlite_registry_mru_file, &fcri, NULL);

          CloseRegFiletoMem(&hks_mru);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    sqlite3_exec(db, "SELECT hkey,key,value,value_type,type_id,description_id FROM extract_registry_mru_request;", callback_sqlite_registry_mru_local, &fcri, NULL);
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
