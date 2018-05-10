//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void addDeletedKey(char *source, char*key, char*value, char*data, char *type, char *sid, char *last_update,unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_registry_deleted_keys (source,key,value,data,type,sid,last_update,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           source,key,value,data,type,sid,last_update,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"Registry_Deleted_Key\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         source,key,value,data,type,sid,last_update,session_id);
  #endif
}
//------------------------------------------------------------------------------
DWORD Traiter_RegBin_nk_deleted(char *fic, DWORD position, DWORD taille_fic, char *buffer, unsigned int session_id, sqlite3 *db, BOOL deleted)
{
  if (position + HBIN_CELL_NK_SIZE > taille_fic)return 1;
    HBIN_CELL_NK_HEADER *nk_h = (HBIN_CELL_NK_HEADER *)(buffer+position);
  //valide ?
  if (nk_h->key_name_size >0 && nk_h->key_name_size<taille_fic && nk_h->size>0 && (position-HBIN_CELL_NK_SIZE)&& nk_h->type == 0x6B6E)
  {
    if (nk_h->nb_values > 0 && nk_h->nb_values < 0xFFFFFFFF && nk_h->val_ls_offset < 0xFFFFFFFF && nk_h->val_ls_offset > 0)
    {
      if (!deleted) return HBIN_CELL_NK_SIZE;

      char path[MAX_LINE_SIZE]="";
      char value[MAX_LINE_SIZE];
      char data[MAX_LINE_SIZE];
      char type[MAX_LINE_SIZE];
      char parent_key_update[DATE_SIZE_MAX];
      char Owner_SID[MAX_PATH]="";

      //lecture du path complet + SID ^^
      ReadPath(buffer, taille_fic, position, path,MAX_LINE_SIZE,NULL,Owner_SID,MAX_PATH);

      //lecture des valeures ^^
      DWORD i, k;
      S_ITEM_LS *item_ls;
      HBIN_CELL_VK_HEADER *vk_h;
      char tmp[MAX_LINE_SIZE];
      char *c;

      //last update
      parent_key_update[0]=0;
      if (nk_h->last_write.dwHighDateTime != 0 || nk_h->last_write.dwLowDateTime != 0)
      {
        filetimeToString_GMT(nk_h->last_write, parent_key_update, DATE_SIZE_MAX);
        if (parent_key_update[0] != '2' && deleted) //bad date
          parent_key_update[0]=0;
      }

      BOOL val_ok;
      for (i=0;i< (nk_h->nb_values);i++)
      {
        value[0] = 0;
        type[0] = 0;
        if (nk_h->val_ls_offset+0x1000+i*HBIN_CELL_ITEM_LS+4 >= taille_fic)break;
        item_ls = (S_ITEM_LS *)&buffer[nk_h->val_ls_offset+0x1000+i*HBIN_CELL_ITEM_LS+4];
        //lecture de l'offset de la valeure :
        if (item_ls->val_of>0 && (item_ls->val_of+0x1000+HBIN_CELL_VK_SIZE)<taille_fic)
        {
          //traitement des valeures !!!
          val_ok = TRUE;
          vk_h = (HBIN_CELL_VK_HEADER *)&buffer[item_ls->val_of+0x1000];

          //récupération du nom de la valeur :
          strncpy(value,vk_h->value,MAX_LINE_SIZE);
          if (vk_h->name_size>=MAX_LINE_SIZE)value[MAX_LINE_SIZE-1]=0;
          else
          {
            value[vk_h->name_size]=0;
            if (strlen(value) != vk_h->name_size)val_ok = FALSE;
          }

          data[0] = 0;
          switch(vk_h->data_type)
          {
            //Chaines
            case 0x00000001 : //REG_SZ, chaine ASCII et Unicodes
              if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
              {
                if (vk_h->data_size < 5)
                {
                  snprintf(data,MAX_LINE_SIZE,"%S",vk_h->cdata_offset);
                  data[vk_h->data_size/2]=0;
                }else snprintf(data,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE]);
              }
              strcpy(type,"REG_SZ\0");
            break;
            case 0x00000002 : //REG_EXPAND_SZ, chaine ASCII et Unicodes, contient des path type %path%
              data[0] = 0;
              if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
              {
                if (vk_h->data_size < 5)
                {
                  strcpy(data,vk_h->cdata_offset);
                  data[4]=0;
                }else snprintf(data,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE]);
              }
              strcpy(type,"REG_EXPAND_SZ\0");
            break;
            case 0x00000006 : //REG_LINK, chaine ASCII et Unicodes, lien lien
              if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
              {
                if (vk_h->data_size < 5)
                {
                  strcpy(data,vk_h->cdata_offset);
                  data[4]=0;
                }else snprintf(data,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE]);
              }
              strcpy(type,"REG_LINK\0");
            break;
            case 0x00000007 : //REG_MULTI_SZ, multiples chaine ASCII et Unicodes, lien
                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  if (vk_h->data_size < 5)
                  {
                    strcpy(data,vk_h->cdata_offset);
                    data[4]=0;
                  }else
                  {
                    //traitement des chaines multiples :
                    do
                    {
                      snprintf(tmp,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+2*strlen(data)]);
                      strncat (data,tmp,MAX_LINE_SIZE-strlen(data));
                      strncat (data," \0",MAX_LINE_SIZE-strlen(data));
                    }while (strlen(data)*2 < (vk_h->data_size));
                  }
                }
                strcpy(type,"REG_MULTI_SZ\0");
              break;
              case 0x00000003 : //REG_BINARY, données binaires
                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  if (vk_h->data_size < 5)
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",vk_h->cdata_offset[k]&0xff);
                      strncat(data,tmp,MAX_LINE_SIZE-strlen(data));
                    }
                    strncat(data,"\0",MAX_LINE_SIZE-strlen(data));
                  }else
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+k]&0xff);
                      strncat(data,tmp,MAX_LINE_SIZE-strlen(data));
                    }
                    strncat(data,"\0",MAX_LINE_SIZE-strlen(data));
                  }
                }
                strcpy(type,"REG_BINARY\0");
              break;
              case 0x0000000A : //REG_RESSOURCE_REQUIREMENT_LIST, données binaires
                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  if (vk_h->data_size < 5)
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",vk_h->cdata_offset[k]&0xff);
                      strncat(data,tmp,MAX_LINE_SIZE-strlen(data));
                    }
                    strncat(data,"\0",MAX_LINE_SIZE-strlen(data));
                  }else
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+k]&0xff);
                      strncat(data,tmp,MAX_LINE_SIZE-strlen(data));
                    }
                    strncat(data,"\0",MAX_LINE_SIZE-strlen(data));
                  }
                }
                strcpy(type,"REG_RESSOURCE_REQUIREMENT_LIST\0");
              break;
              case 0x00000004 : //REG_DWORD, données numériques 32bitschar
              case 0x00000005 : //REG_DWORD, données numériques 32bits signées
                if (vk_h->data_size > 0)
                {
                  snprintf(data,MAX_LINE_SIZE,"%08X",(unsigned int)(vk_h->data_offset&0xFFFFFFFF)); // en dword offset = valeur
                }
                strcpy(type,"REG_DWORD\0");
              break;
              case 0x0000000b : //REG_QWORD, données numériques 64bits signées
                if (vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  c = &buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE];
                  snprintf(data,MAX_LINE_SIZE,"0x%02X%02X%02X%02X%02X%02X%02X%02X",c[7]&0xff,c[6]&0xff,c[5]&0xff,c[4]&0xff,c[3]&0xff,c[2]&0xff,c[1]&0xff,c[0]&0xff);
                }
                strcpy(type,"REG_QWORD\0");
              break;
              default :
                if (deleted)val_ok = FALSE;
                //on ajoute pas au liste view les éléments inconnus non valides
                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  /*for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                  {
                    snprintf(tmp,10,"%02X",buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+k]&0xff);
                    strncat(data,tmp,MAX_LINE_SIZE);
                  }
                  strncat(data,"\0",MAX_LINE_SIZE);*/

                  if (vk_h->data_type == 0x00000000)strcpy(type,"REG_NONE\0");
                  else val_ok = FALSE;
                }else
                {
                  if (vk_h->data_type == 0x00000000)strcpy(type,"REG_NONE\0");
                  else val_ok = FALSE;
                }
              break;
          }

          if (val_ok && strlen(value) || strlen(data))
          {
            //ajout de l'item
            addDeletedKey(fic, path, value, data, type,Owner_SID, parent_key_update,session_id, db);
          }
        }else break;
      }

      //no value : only directory
      if ((nk_h->nb_values < 1 || nk_h->nb_values == 0xFFFFFFFF) && (nk_h->nb_subkeys < 1 || nk_h->nb_subkeys == 0xFFFFFFFF))
      {
        addDeletedKey(fic, path, "", "", "", Owner_SID, parent_key_update,session_id, db);
      }
    }
    return HBIN_CELL_NK_SIZE;
  }
  return 1;
}
//------------------------------------------------------------------------------
void Scan_registry_deletedKey_file(char *reg_file,unsigned int session_id,sqlite3 *db)
{
  //open file
  HANDLE Hfic = CreateFile(reg_file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (Hfic == INVALID_HANDLE_VALUE)return;

  DWORD taille_fic = GetFileSize(Hfic,NULL);
  if (taille_fic<1 || taille_fic == INVALID_FILE_SIZE)
  {
    CloseHandle(Hfic);
    return;
  }

  //alloc memory
  char *buffer = (char *) HeapAlloc(GetProcessHeap(), 0, taille_fic+1);
  if (!buffer)
  {
    CloseHandle(Hfic);
    return;
  }

  //load file
  DWORD copiee, position = 0, increm = 0;
  if (taille_fic > DIXM)increm = DIXM;
  else increm = taille_fic;

  while (position<taille_fic && increm!=0)
  {
    copiee = 0;
    ReadFile(Hfic, buffer+position, increm,&copiee,0);
    position +=copiee;
    if (taille_fic-position < increm)increm = taille_fic-position ;
  }
  CloseHandle(Hfic);

  //working
  if (position>0)
  {
    if (((REGF_HEADER*)buffer)->id == 0x66676572) //Fichier REG standard
    {
      taille_fic = position;
      position = 0x1000; //first hbin struct

      HBIN_HEADER *hb_h;
      DWORD pos_fhbin = 0;

      //recherche du 1er hbin !! (en cas de bug)
      while(position<taille_fic-4)
      {
        hb_h = (HBIN_HEADER *)&buffer[position];
        if (hb_h->id == 0x6E696268 )  //hbin
        {
          if (pos_fhbin == 0)pos_fhbin = position;
          position=position+HBIN_HEADER_SIZE;//entête hbin
          break;
        }else position++;
      }

      HBIN_CELL_PRE_HEADER *hb_ph;
      while(position<taille_fic-(HBIN_CELL_PRE_HEADER_SIZE+1))
      {
        //on ne traite que les clés nk (name key = directory)
        hb_ph = (HBIN_CELL_PRE_HEADER *)&buffer[position];
        if (((hb_ph->size[1]&0xFF) == 0xFF) && ((hb_ph->size[2]&0xFF) == 0xFF) && ((hb_ph->size[3]&0xFF) == 0xFF))
        {
          switch(hb_ph->type)
          {
            case 0x6B6E:  position = position + Traiter_RegBin_nk_deleted(reg_file, position, taille_fic, buffer,session_id,db,FALSE);break; //nk
            case 0x6B73 : if (position + HBIN_CELL_SK_SIZE < taille_fic)position = position + HBIN_CELL_SK_SIZE;else position++;break;//sk
            case 0x6B76 : if (position + HBIN_CELL_VK_SIZE < taille_fic)position = position + HBIN_CELL_VK_SIZE;else position++;break;//vk
            case 0x666C : if (position + HBIN_CELL_LF_SIZE < taille_fic)position = position + HBIN_CELL_LF_SIZE;else position++;break;//lf
            case 0x686C : if (position + HBIN_CELL_LH_SIZE < taille_fic)position = position + HBIN_CELL_LH_SIZE;else position++;break;//lh
            case 0x696C : if (position + HBIN_CELL_LI_SIZE < taille_fic)position = position + HBIN_CELL_LI_SIZE;else position++;break;//li
            case 0x6972 : if (position + HBIN_CELL_RI_SIZE < taille_fic)position = position + HBIN_CELL_RI_SIZE;else position++;break;//ri
            case 0x6264 : if (position + HBIN_CELL_DB_SIZE < taille_fic)position = position + HBIN_CELL_DB_SIZE;else position++;break;//db
            default : position++; break;
          }
        }else if (((hb_ph->size[0]&0xFF) != 0x00) &&((hb_ph->size[1]&0xFF) == 0x00) && ((hb_ph->size[2]&0xFF) == 0x00) && ((hb_ph->size[3]&0xFF) == 0x00) && hb_ph->type == 0x6B6E)
        {
            position = position + Traiter_RegBin_nk_deleted(reg_file, position, taille_fic, buffer,session_id,db,TRUE);
        }else position++;
      }
    }
  }
  //on libère la mémoire
  HeapFree(GetProcessHeap(), 0, buffer);
}
//------------------------------------------------------------------------------
DWORD WINAPI Scan_registry_deletedKey(LPVOID lParam)
{
  //init
  sqlite3 *db = (sqlite3 *)db_scan;
  unsigned int session_id = current_session_id;
  char file[MAX_PATH];

  #ifdef CMD_LINE_ONLY_NO_DB
  printf("\"Registry_Deleted_Key\";\"source\";\"key\";\"value\";\"data\";\"type\";\"sid\";\"last_update\";\"session_id\";\r\n");
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
        Scan_registry_deletedKey_file(file, session_id, db);
      }
      hitem = (HTREEITEM)SendMessage(htrv_files, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem);
    }
  }

  if(!SQLITE_FULL_SPEED)sqlite3_exec(db_scan,"END TRANSACTION;", NULL, NULL, NULL);
  check_treeview(htrv_test, H_tests[(unsigned int)lParam], TRV_STATE_UNCHECK);//db_scan
  h_thread_test[(unsigned int)lParam] = 0;
  return 0;
}
