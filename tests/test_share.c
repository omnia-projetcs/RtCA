//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addSharetoDB(char *file, char *share, char *path, char *description, char *type, char *connexion, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_share (file,share,path,description,type,connexion,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           file,share,path,description,type,connexion,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Share\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,share,path,description,type,connexion,session_id);
  #endif
}
//------------------------------------------------------------------------------
void EnumShare(HK_F_OPEN *hks, unsigned int session_id, sqlite3 *db, char*reg_path)
{
  HBIN_CELL_NK_HEADER *nk_h = GetRegistryNK(hks->buffer, hks->taille_fic, (hks->pos_fhbin)+HBIN_HEADER_SIZE, hks->position, reg_path);
  if (nk_h == NULL) return;

  DWORD i, nbSubValue = GetValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, 0, NULL, 0, NULL, 0);
  char share[MAX_PATH], description[MAX_LINE_SIZE];
  for (i=0;i<nbSubValue;i++)
  {
    if (GetValueData(hks->buffer,hks->taille_fic, nk_h, (hks->pos_fhbin)+HBIN_HEADER_SIZE, i,share,MAX_PATH,description,MAX_LINE_SIZE))
    {
      convertStringToSQL(description, MAX_LINE_SIZE);
      addSharetoDB(hks->file,share, "", description, "", "", session_id, db);
    }
  }
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_share(LPVOID lParam)
{
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"BEGIN TRANSACTION;", NULL, NULL, NULL);
  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Share\";\"file\";\"share\";\"path\";\"description\";\"type\";\"connexion\";\"session_id\";\r\n");
  #endif
  if (!LOCAL_SCAN)
  {
    //get in registry files
    char file[MAX_PATH];
    HK_F_OPEN hks;
    HTREEITEM hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)TRV_HTREEITEM_CONF[FILES_TITLE_REGISTRY]);
    while(hitem!=NULL)
    {
      file[0] = 0;
      GetTextFromTrv(hitem, file, MAX_PATH);
      if (file[0] != 0)
      {
        //open file + verify
        if(OpenRegFiletoMem(&hks, file))
        {
          EnumShare(&hks, session_id, db, "ControlSet001\\Services\\LanmanServer\\Shares");
          EnumShare(&hks, session_id, db, "ControlSet002\\Services\\LanmanServer\\Shares");
          EnumShare(&hks, session_id, db, "ControlSet003\\Services\\LanmanServer\\Shares");
          EnumShare(&hks, session_id, db, "ControlSet004\\Services\\LanmanServer\\Shares");
          CloseRegFiletoMem(&hks);
        }
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }else
  {
    if (MyNetApiBufferFree != NULL && MyNetShareEnum != NULL )
    {
      NET_API_STATUS res;
      PSHARE_INFO_502 buffer,p;
      DWORD nb=0,tr=0,i;
      char share[DEFAULT_TMP_SIZE], path[MAX_PATH], description[MAX_PATH], type[DEFAULT_TMP_SIZE], connexion[DEFAULT_TMP_SIZE];

      do
      {
        res = MyNetShareEnum (0, 502, (LPBYTE *) &buffer,MAX_PREFERRED_LENGTH, &nb, &tr,0);
        if(res != ERROR_SUCCESS && res != ERROR_MORE_DATA)break;

        for(i=1,p=buffer;i<=nb;i++,p++)
        {
          snprintf(share,DEFAULT_TMP_SIZE,"%S",p->shi502_netname);
          snprintf(path,MAX_PATH,"%S",p->shi502_path);
          snprintf(description,MAX_PATH,"%S",p->shi502_remark);

          switch(p->shi502_type)
          {
            case STYPE_DISKTREE:  strncpy(type,"DISKTREE",DEFAULT_TMP_SIZE);break;
            case STYPE_PRINTQ:    strncpy(type,"PRINT",DEFAULT_TMP_SIZE);break;
            case STYPE_DEVICE:    strncpy(type,"DEVICE",DEFAULT_TMP_SIZE);break;
            case STYPE_IPC:       strncpy(type,"IPC",DEFAULT_TMP_SIZE);break;
            case STYPE_SPECIAL:   strncpy(type,"SPECIAL",DEFAULT_TMP_SIZE);break;
            case 0x40000000:      strncpy(type,"TEMPORARY",DEFAULT_TMP_SIZE);break;
            case -2147483645:     strncpy(type,"RPC",DEFAULT_TMP_SIZE);break;
            default :             snprintf(type,DEFAULT_TMP_SIZE,"UNKNOW (%lu)",p->shi502_type);break;
          }

          if (p->shi502_max_uses==-1)
            snprintf(connexion,DEFAULT_TMP_SIZE,"%lu/-",p->shi502_current_uses);
          else snprintf(connexion,DEFAULT_TMP_SIZE,"%lu/%lu",p->shi502_current_uses,p->shi502_max_uses);

          convertStringToSQL(path, MAX_PATH);
          convertStringToSQL(description, MAX_PATH);
          addSharetoDB("",share, path, description, type, connexion, session_id, db);
        }
      }while(res==ERROR_MORE_DATA);
      MyNetApiBufferFree(buffer);
    }
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
