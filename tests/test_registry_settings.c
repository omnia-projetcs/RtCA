//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addRegistrySettingstoDB(char *file, char *hk, char *key, char*value, char *data, char*type_id, char*description_id, char *parent_key_update, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_settings (file,hk,key,value,data,type_id,description_id,parent_key_update,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%s,%s,\"%s\",%d);",
           file,hk,key,value,data,type_id,description_id,parent_key_update,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Registry_Settings\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,hk,key,value,data,type_id,description_id,parent_key_update,session_id);
  #endif
}
//------------------------------------------------------------------------------
//local function part !!!
//------------------------------------------------------------------------------
BOOL GetWindowsCDKey_local(HKEY hk, char *key, char *value, char *result, unsigned int size_max)
{
  BOOL ret = FALSE;
  HKEY CleTmp = 0;
  if(RegOpenKey(hk,key,&CleTmp) == ERROR_SUCCESS)
  {
    BYTE data[MAX_PATH],enc[MAX_PATH];
    DWORD dwDataLen	= MAX_PATH, serial_size;
    if(RegQueryValueEx(CleTmp,value,0,0,data,&dwDataLen) == ERROR_SUCCESS)
    {
      if (dwDataLen<66)return FALSE;
      char sk[25] = "BCDFGHJKMPQRTVWXY2346789";
      char lpszSerial[MAX_PATH];
      int i,c=0,nCur=0;

      for(i=52;i<=66;i++)enc[i-52] = data[i];
			for(i=24;i>=0;i--)
			{
				nCur = 0;
				for(c=14;c>-1;c--)
				{
					nCur = nCur * 256;
					nCur ^= enc[c];
					enc[c] = nCur / 24;
					nCur %= 24;
				}
				lpszSerial[i] = sk[nCur];
			}

			serial_size = 0;
			for(i=0;lpszSerial[i] && (i+i/5) < 30 && size_max>serial_size;i++)
      {
        if(i % 5 == 0 && i>0)snprintf(result+serial_size,size_max-serial_size,"-%c",lpszSerial[i]);
				else snprintf(result+serial_size,size_max-serial_size,"%c",lpszSerial[i]);
				serial_size = strlen(result);
			}
      ret = TRUE;
    }
    RegCloseKey(CleTmp);
  }
  return ret;
}
//------------------------------------------------------------------------------
int callback_sqlite_registry_local(void *datas, int argc, char **argv, char **azColName)
{
  if (argv[0] == 0)return 0;

  FORMAT_CALBAK_TYPE *type = datas;
  unsigned int session_id = current_session_id;
  switch(type->type)
  {
    case SQLITE_REGISTRY_TYPE_SETTINGS:
    {
      HKEY hk = hkStringtohkey(argv[0]);
      switch(atoi(argv[3]))//value_type
      {
        case TYPE_VALUE_STRING:
        {
          char tmp[MAX_PATH]="";
          if (ReadValue(hk,argv[1],argv[2],tmp, MAX_PATH))
          {
            char parent_key_update[DATE_SIZE_MAX];
            ReadKeyUpdate(hk,argv[1], parent_key_update, DATE_SIZE_MAX);
            convertStringToSQL(tmp, MAX_PATH);
            addRegistrySettingstoDB("", argv[0], argv[1], argv[2], tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
          }
        }
        break;
        case TYPE_VALUE_DWORD:
        {
          char tmp[MAX_PATH]="";
          long int value= ReadDwordValue(hk,argv[1],argv[2]);
          if (value != -1)
          {
            snprintf(tmp,MAX_PATH,"0x%08X",(unsigned int)value);
            char parent_key_update[DATE_SIZE_MAX];
            ReadKeyUpdate(hk,argv[1], parent_key_update, DATE_SIZE_MAX);
            addRegistrySettingstoDB("", argv[0], argv[1], argv[2], tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
          }
        }
        break;
        case TYPE_VALUE_MULTI_STRING:
        {
          char tmp[MAX_PATH]="";
          long int i, tmp_size = ReadValue(hk,argv[1],argv[2],tmp, MAX_PATH);
          if (tmp_size>0)
          {
            char parent_key_update[DATE_SIZE_MAX];
            ReadKeyUpdate(hk,argv[1], parent_key_update, DATE_SIZE_MAX);
            for (i=0;i<tmp_size;i++)
            {
              if (tmp[i] == 0)tmp[i]=';';
            }
            convertStringToSQL(tmp, MAX_PATH);
            addRegistrySettingstoDB("", argv[0], argv[1], argv[2], tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
          }
        }
        break;
        case TYPE_VALUE_MULTI_WSTRING:
          {
            char tmp[REQUEST_MAX_SIZE]="",data_read[REQUEST_MAX_SIZE];
            DWORD pos=0, data_size_read = ReadValue(hk,argv[1],argv[2],tmp, REQUEST_MAX_SIZE);

            if (data_size_read)
            {
              char parent_key_update[DATE_SIZE_MAX];
              ReadKeyUpdate(hk,argv[1], parent_key_update, DATE_SIZE_MAX);

              while ((pos-1)*2<data_size_read)
              {
                snprintf(data_read+pos,REQUEST_MAX_SIZE,"%S;",tmp+(pos*2-1));
                pos = strlen(data_read);
              }

              convertStringToSQL(data_read, MAX_PATH);
              addRegistrySettingstoDB("", argv[0], argv[1], argv[2], data_read, argv[4], argv[5], parent_key_update, session_id, db_scan);
            }
          }
        break;
        case TYPE_VALUE_FILETIME:
        {
          char tmp[MAX_PATH]="";
          FILETIME ft;
          if (hk == 0 || argv[1][0]==0)break;
          ReadFILETIMEValue(hk,argv[1],argv[2],&ft);
          if (ft.dwHighDateTime+ft.dwLowDateTime > 0)
          {
            filetimeToString_GMT(ft, tmp, MAX_PATH);

            char parent_key_update[DATE_SIZE_MAX];
            ReadKeyUpdate(hk,argv[1], parent_key_update, DATE_SIZE_MAX);
            addRegistrySettingstoDB("", argv[0], argv[1], argv[2], tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
          }
        }
        break;
        case TYPE_VALUE_DWORD_TIME:
        {
          char tmp[MAX_PATH]="";
          long int value= ReadDwordValue(hk,argv[1],argv[2]);
          if (value != -1)
          {
            char parent_key_update[DATE_SIZE_MAX];
            ReadKeyUpdate(hk,argv[1], parent_key_update, DATE_SIZE_MAX);

            addRegistrySettingstoDB("", argv[0], argv[1], argv[2], timeToString(value, tmp, MAX_PATH), argv[4], argv[5], parent_key_update, session_id, db_scan);
          }
        }
        break;
        case TYPE_VALUE_WIN_SERIAL:
        {
          char tmp[MAX_PATH]="";
          if (GetWindowsCDKey_local(hk, argv[1], argv[2], tmp, MAX_PATH))
          {
            char parent_key_update[DATE_SIZE_MAX];
            ReadKeyUpdate(hk,argv[1], parent_key_update, DATE_SIZE_MAX);
            addRegistrySettingstoDB("", argv[0], argv[1], argv[2], tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
          }
        }
        break;
        case TYPE_ENUM_STRING_VALUE:
        {
          HKEY CleTmp=0;
          if (RegOpenKey(hk,argv[1],&CleTmp)!=ERROR_SUCCESS)return 0;

          FILETIME lastupdate;
          char parent_key_update[DATE_SIZE_MAX];

          DWORD NameSize, DataSize;
          char Name[MAX_PATH], Data[MAX_PATH], tmp[MAX_PATH];
          DWORD nbValue = 0, i,j, type2;
          if (RegQueryInfoKey (CleTmp,0,0,0,0,0,0,&nbValue,0,0,0,&lastupdate)==ERROR_SUCCESS)
          {
            filetimeToString_GMT(lastupdate, parent_key_update, DATE_SIZE_MAX);
            for (i=0;i<nbValue;i++)
            {
              NameSize = MAX_PATH;
              DataSize = MAX_PATH;
              Name[0]  = 0;
              Data[0]  = 0;
              type2     = 0;
              if (RegEnumValue (CleTmp,i,(LPTSTR)Name,(LPDWORD)&NameSize,0,(LPDWORD)&type2,(LPBYTE)Data,(LPDWORD)&DataSize)==ERROR_SUCCESS)
              {
                switch(type2)
                {
                  case REG_EXPAND_SZ:
                  case REG_SZ:addRegistrySettingstoDB("", argv[0], argv[1], Name, Data, argv[4], argv[5], parent_key_update, session_id, db_scan);break;
                  case REG_LINK:
                    snprintf(tmp,MAX_PATH,"%S",Data);
                    convertStringToSQL(tmp, MAX_PATH);
                    addRegistrySettingstoDB("", argv[0], argv[1], Name, tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
                  break;
                  case REG_MULTI_SZ:
                    for (j=0;j<DataSize;j++)
                    {
                      if (Data[j] == 0)Data[j]=';';
                    }
                    convertStringToSQL(Data, MAX_PATH);
                    addRegistrySettingstoDB("", argv[0], argv[1], Name, Data, argv[4], argv[5], parent_key_update, session_id, db_scan);
                  break;
                  case REG_DWORD:
                    snprintf(tmp,MAX_PATH,"0x%08X",&Data[0]);
                    addRegistrySettingstoDB("", argv[0], argv[1], Name, tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
                  break;
                  default : //binary/dword
                    strncpy(tmp,"0x",MAX_PATH);
                    NameSize = 0;
                    for (j=0;j<DataSize && NameSize<MAX_PATH;j++)
                    {
                      NameSize = strlen(tmp);
                      snprintf(tmp+NameSize,MAX_PATH-NameSize,"%02X",Data[j]&0xFF);
                    }
                    addRegistrySettingstoDB("", argv[0], argv[1], Name, tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
                  break;
                }
              }
            }
          }
        }
        break;
      }
    }
    break;
  }
  return 0;
}
//------------------------------------------------------------------------------
BOOL SyskeyExtract(char *cJD, char *cSkew1, char *cGBG, char *cData, char *sk, unsigned int sk_size)
{
  char tmp[SZ_PART_SYSKEY];

  if (strlen(cGBG)<0x08)return FALSE;
  if (strlen(cSkew1)<0x08)return FALSE;
  if (strlen(cJD)<0x08)return FALSE;
  if (strlen(cData)<0x08)return FALSE;

  //traitement et commutation des données pour générer la syskey
  tmp[0x00] = cGBG[0x00];
  tmp[0x01] = cGBG[0x01];
  tmp[0x02] = cSkew1[0x02];
  tmp[0x03] = cSkew1[0x03];
  tmp[0x04] = cSkew1[0x00];
  tmp[0x05] = cSkew1[0x01];
  tmp[0x06] = cJD[0x04];
  tmp[0x07] = cJD[0x05];
  tmp[0x08] = cGBG[0x06];
  tmp[0x09] = cGBG[0x07];
  tmp[0x0A] = cGBG[0x02];
  tmp[0x0B] = cGBG[0x03];
  tmp[0x0C] = cData[0x02];
  tmp[0x0D] = cData[0x03];
  tmp[0x0E] = cJD[0x06];
  tmp[0x0F] = cJD[0x07];
  tmp[0x10] = cJD[0x00];
  tmp[0x11] = cJD[0x01];
  tmp[0x12] = cSkew1[0x04];
  tmp[0x13] = cSkew1[0x05];
  tmp[0x14] = cJD[0x02];
  tmp[0x15] = cJD[0x03];
  tmp[0x16] = cData[0x00];
  tmp[0x17] = cData[0x01];
  tmp[0x18] = cData[0x04];
  tmp[0x19] = cData[0x05];
  tmp[0x1A] = cGBG[0x04];
  tmp[0x1B] = cGBG[0x05];
  tmp[0x1C] = cData[0x06];
  tmp[0x1D] = cData[0x07];
  tmp[0x1E] = cSkew1[0x06];
  tmp[0x1F] = cSkew1[0x07];
  tmp[0x20] = 0;

  if (sk!=NULL)
  {
    strncpy(sk,tmp,sk_size);
    strncpy(_SYSKEY,tmp,MAX_PATH);
    return TRUE;
  }else return FALSE;
}
//------------------------------------------------------------------------------
BOOL registry_syskey_local(char*sk, unsigned int sk_size)
{
  HKEY CleTmp=0;
  char cJD[SZ_PART_SYSKEY]="", cSkew1[SZ_PART_SYSKEY]="", cGBG[SZ_PART_SYSKEY]="", cData[SZ_PART_SYSKEY]="";
  DWORD size;

  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Lsa\\JD\\",&CleTmp)==ERROR_SUCCESS)
  {
    size = SZ_PART_SYSKEY;
    if (RegQueryInfoKey(CleTmp, cJD, &size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return FALSE;
    RegCloseKey(CleTmp);
  }else return FALSE;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Lsa\\Skew1\\",&CleTmp)==ERROR_SUCCESS)
  {
    size = SZ_PART_SYSKEY;
    if (RegQueryInfoKey(CleTmp, cSkew1, &size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return FALSE;
    RegCloseKey(CleTmp);
  }else return FALSE;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Lsa\\GBG\\",&CleTmp)==ERROR_SUCCESS)
  {
    size = SZ_PART_SYSKEY;
    if (RegQueryInfoKey(CleTmp, cGBG, &size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return FALSE;
    RegCloseKey(CleTmp);
  }else return FALSE;
  if (RegOpenKey(HKEY_LOCAL_MACHINE,"SYSTEM\\CurrentControlSet\\Control\\Lsa\\Data\\",&CleTmp)==ERROR_SUCCESS)
  {
    size = SZ_PART_SYSKEY;
    if (RegQueryInfoKey(CleTmp, cData, &size, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL) != ERROR_SUCCESS)return FALSE;
    RegCloseKey(CleTmp);
  }else return FALSE;

  //traitement
  return SyskeyExtract(cJD, cSkew1, cGBG, cData, sk, sk_size);
}
//------------------------------------------------------------------------------
void Scan_registry_setting_local(sqlite3 *db)
{
  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = SQLITE_REGISTRY_TYPE_SETTINGS;
  sqlite3_exec(db, "SELECT hkey,key,value,value_type,type_id,description_id FROM extract_registry_settings_request;", callback_sqlite_registry_local, &fcri, NULL);

  //syskey
  char sk[MAX_PATH]="";
  if(registry_syskey_local(sk, MAX_PATH))
  {
    addRegistrySettingstoDB("", "HKEY_LOCAL_MACHINE", "SYSTEM\\CurrentControlSet\\Control\\Lsa\\JD,Skew1,GBG,Data","", sk, "100", SYSKEY_STRING_DEF, "", current_session_id, db_scan);
  }
}
//------------------------------------------------------------------------------
//file registry part
//------------------------------------------------------------------------------
HK_F_OPEN local_hks;
//------------------------------------------------------------------------------
int callback_sqlite_registry_file(void *datas, int argc, char **argv, char **azColName)
{
  FORMAT_CALBAK_TYPE *type = datas;
  unsigned int session_id = current_session_id;
  char tmp[MAX_LINE_SIZE];
  switch(type->type)
  {
    case SQLITE_REGISTRY_TYPE_SETTINGS:
    {
      switch(atoi(argv[3]))//value_type
      {
        case TYPE_VALUE_STRING:
        case TYPE_VALUE_DWORD:
        case TYPE_VALUE_MULTI_STRING:
          if (Readnk_Value(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_hks.position,
                           argv[1], NULL, argv[2], tmp, MAX_LINE_SIZE))
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX];
            Readnk_Infos(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin), local_hks.position,
                         argv[1], NULL, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);

            //save
            convertStringToSQL(tmp, MAX_LINE_SIZE);
            addRegistrySettingstoDB(local_hks.file, "", argv[1], argv[2], tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
          }
        break;
        case TYPE_VALUE_MULTI_WSTRING:
        {
          char data_read[MAX_LINE_SIZE];
          DWORD pos=0, data_size_read = MAX_LINE_SIZE;
          if (ReadBinarynk_Value(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_hks.position,
                                 argv[1], NULL, argv[2], tmp, &data_size_read))
          {
            if (data_size_read)
            {
              //data_read
              while ((pos-1)*2<data_size_read)
              {
                snprintf(data_read+pos,MAX_LINE_SIZE,"%S;",tmp+(pos*2-1));
                pos = strlen(data_read);
              }

              //key update
              char parent_key_update[DATE_SIZE_MAX];
              Readnk_Infos(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin), local_hks.position,
                           argv[1], NULL, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);

              //save
              convertStringToSQL(data_read, MAX_LINE_SIZE);
              addRegistrySettingstoDB(local_hks.file, "", argv[1], argv[2], data_read, argv[4], argv[5], parent_key_update, session_id, db_scan);
            }
          }
        }
        break;
        case TYPE_VALUE_FILETIME:
        {
          DWORD data_size = sizeof(FILETIME)+1;
          FILETIME f_date;
          if (ReadBinarynk_Value(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_hks.position,
                           argv[1], NULL, argv[2], (void*)&f_date, &data_size))
          {
            if(data_size && f_date.dwHighDateTime+f_date.dwLowDateTime > 0)
            {
              //key update
              char parent_key_update[DATE_SIZE_MAX];
              Readnk_Infos(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin), local_hks.position,
                           argv[1], NULL, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);

              //convert date
              tmp[0] = 0;
              filetimeToString_GMT(f_date, tmp, DATE_SIZE_MAX);

              //save
              convertStringToSQL(tmp, MAX_LINE_SIZE);
              addRegistrySettingstoDB(local_hks.file, "", argv[1], argv[2], tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
            }
          }
        }
        break;
        case TYPE_VALUE_DWORD_TIME:
          if (Readnk_Value(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_hks.position,
                           argv[1], NULL, argv[2], tmp, MAX_LINE_SIZE))
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX];
            Readnk_Infos(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin), local_hks.position,
                         argv[1], NULL, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);

            //convert in date : Windows NT time
            addRegistrySettingstoDB(local_hks.file, "", argv[1], argv[2], timeToString(HexToll(tmp, strlen(tmp)), tmp, MAX_LINE_SIZE), argv[4], argv[5], parent_key_update, session_id, db_scan);
         }
        break;
        case TYPE_VALUE_WIN_SERIAL:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_hks.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX];
            Readnk_Infos(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin), local_hks.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);



            //get value
            DWORD test_size = MAX_LINE_SIZE;
            DWORD serial_size;
            ReadBinarynk_Value(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_hks.position,
                                                         NULL, nk_h, argv[2], (void*)tmp, &test_size);
            if (test_size>65)
            {
              char result[MAX_PATH]="";

              char key[25] = "BCDFGHJKMPQRTVWXY2346789";
              BYTE enc[MAX_PATH];
              char lpszSerial[MAX_PATH];
              int i,c=0,nCur=0;

              for(i=52;i<=66;i++)enc[i-52] = tmp[i];
              for(i=24;i>=0;i--)
              {
                nCur = 0;
                for(c=14;c>-1;c--)
                {
                  nCur = nCur * 256;
                  nCur ^= enc[c];
                  enc[c] = nCur / 24;
                  nCur %= 24;
                }
                lpszSerial[i] = key[nCur];
              }

              serial_size = 0;
              for(i=0;lpszSerial[i] && (i+i/5) < 30 && MAX_PATH>serial_size;i++)
              {
                if(i % 5 == 0 && i>0)snprintf(result+serial_size,MAX_PATH-serial_size,"-%c",lpszSerial[i]);
                else snprintf(result+serial_size,MAX_PATH-serial_size,"%c",lpszSerial[i]);
                serial_size = strlen(result);
              }

              //save
              convertStringToSQL(result, MAX_LINE_SIZE);
              addRegistrySettingstoDB(local_hks.file, "", argv[1], argv[2], result, argv[4], argv[5], parent_key_update, session_id, db_scan);
            }
          }
        }
        break;
        case TYPE_ENUM_STRING_VALUE:
        {
          HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, local_hks.position,argv[1]);
          if (nk_h!=NULL)
          {
            //key update
            char parent_key_update[DATE_SIZE_MAX];
            Readnk_Infos(local_hks.buffer,local_hks.taille_fic, (local_hks.pos_fhbin), local_hks.position,
                         NULL, nk_h, parent_key_update, DATE_SIZE_MAX, NULL, 0,NULL, 0);

            //get values
            char value[MAX_PATH];
            DWORD i, nbSubValue = GetValueData(local_hks.buffer,local_hks.taille_fic, nk_h, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
            for (i=0;i<nbSubValue;i++)
            {
              if (GetValueData(local_hks.buffer,local_hks.taille_fic, nk_h, (local_hks.pos_fhbin)+HBIN_HEADER_SIZE, i,value,MAX_PATH,tmp,MAX_LINE_SIZE))
              {
                //save
                convertStringToSQL(value, MAX_PATH);
                convertStringToSQL(tmp, MAX_LINE_SIZE);
                addRegistrySettingstoDB(local_hks.file, "", argv[1], value, tmp, argv[4], argv[5], parent_key_update, session_id, db_scan);
              }
            }
          }
        }
        break;
      }
    }
  }
  return 0;
}
//------------------------------------------------------------------------------
BOOL registry_syskey_file(HK_F_OPEN *hks, char*sk, unsigned int sk_size)
{
  char cJD[SZ_PART_SYSKEY]="",
       cSkew1[SZ_PART_SYSKEY]="",
       cGBG[SZ_PART_SYSKEY]="",
       cData[SZ_PART_SYSKEY]="";

  if(Readnk_Class(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,
                  "ControlSet001\\Control\\Lsa\\JD", NULL, cJD, SZ_PART_SYSKEY)==FALSE) return FALSE;
  if(Readnk_Class(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,
                  "ControlSet001\\Control\\Lsa\\Skew1", NULL, cSkew1, SZ_PART_SYSKEY)==FALSE) return FALSE;
  if(Readnk_Class(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,
                  "ControlSet001\\Control\\Lsa\\GBG", NULL, cGBG, SZ_PART_SYSKEY)==FALSE) return FALSE;
  if(Readnk_Class(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position,
                  "ControlSet001\\Control\\Lsa\\Data", NULL, cData, SZ_PART_SYSKEY)==FALSE) return FALSE;
  //traitement
  return SyskeyExtract(cJD, cSkew1, cGBG, cData, sk, sk_size);
}
//------------------------------------------------------------------------------
void Scan_registry_setting_file(sqlite3 *db, char *file)
{
  //Open file and init datas !
  if(OpenRegFiletoMem(&local_hks, file))
  {
    FORMAT_CALBAK_READ_INFO fcri;
    fcri.type = SQLITE_REGISTRY_TYPE_SETTINGS;
    sqlite3_exec(db, "SELECT hkey,search_key,value,value_type,type_id,description_id FROM extract_registry_settings_request;", callback_sqlite_registry_file, &fcri, NULL);

    //syskey
    char sk[MAX_PATH]="";
    if(registry_syskey_file(&local_hks, sk, MAX_PATH))
    {
      addRegistrySettingstoDB(local_hks.file, "", "ControlSet001\\Control\\Lsa\\JD,Skew1,GBG,Data","", sk, "100", SYSKEY_STRING_DEF, "", current_session_id, db_scan);
    }

    CloseRegFiletoMem(&local_hks);
  }
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_setting(LPVOID lParam)
{
  //init
  char file[MAX_PATH];

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Registry_Settings\";\"file\";\"hk\";\"key\";\"value\";\"data\";\"type_id\";\"description_id\";\"parent_key_update\";\"session_id\";\r\n");
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
        //verify
        Scan_registry_setting_file(db_scan,file);
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else Scan_registry_setting_local(db_scan); //local

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
