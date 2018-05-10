//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void AddToLVRegBin(HANDLE hlv, LINE_ITEM *item, unsigned short nb_colonne)
{
  //ajout de la ligne
  LVITEM lvi;
  lvi.mask = LVIF_TEXT|LVIF_PARAM;
  lvi.iSubItem = 0;
  lvi.lParam = LVM_SORTITEMS;
  lvi.pszText="";
  lvi.iItem = ListView_GetItemCount(hlv);
  long int itemPos = ListView_InsertItem(hlv, &lvi);

  //ajout des items
  unsigned short i=0;
  for (;i<nb_colonne;i++){if (item[i].c[0]!=0)ListView_SetItemText(hlv,itemPos,i,item[i].c);}
}
//------------------------------------------------------------------------------
void AddNextValueToTreeView(HANDLE htv, HTREEITEM hparent, char *value, DWORD value_type)
{
  switch(value_type)
  {
    case 0x00000001:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_TXT_REG);break;
    case 0x00000002:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_TXT_REG);break;
    case 0x00000003:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_BIN_REG);break;
    case 0x00000004:
    case 0x00000005:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_DWORD_REG);break;
    case 0x00000006:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_BIN_REG);break;
    case 0x00000007:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_TXT_REG);break;
    case 0x0000000b:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_DWORD_REG);break;
    default:AddItemTreeViewImg(htv,value, hparent,ICON_FILE_UNKNOW_REG);break;
  }
}
//------------------------------------------------------------------------------
HTREEITEM AddFirstValueToTreeView(HANDLE htv, HTREEITEM hparent, char *path, char *value, DWORD value_type)
{
  unsigned int i, nb = ExtractTextFromPathNb(path);
  HTREEITEM hitem = hparent, last_hitem;
  TVITEM tvitem;
  char tmp_item[MAX_PATH],tmp_read_item[MAX_PATH];
  for(i=1;i<nb;i++)
  {
    last_hitem = hitem;
    hitem = (HTREEITEM)SendMessage(htv, TVM_GETNEXTITEM,(WPARAM)TVGN_CHILD, (LPARAM)hitem);
    if (hitem == 0) //not exist
    {
      //add directory
      for(;i<nb;i++)
      {
        tmp_item[0] = 0;
        ExtractTextFromPath(path, tmp_item, MAX_PATH, i);
        last_hitem = AddItemTreeViewImg(htv, tmp_item, last_hitem, ICON_DIRECTORY_REG);
      }

      //add items
      AddNextValueToTreeView(htv, last_hitem, value, value_type);
      return last_hitem;
    }else //have child
    {
      //search if exist
      last_hitem = hitem;

      //get string
      tmp_item[0] = 0;
      ExtractTextFromPath(path, tmp_item, MAX_PATH, i);
      charToLowChar(tmp_item);
      do
      {
        //if exist, replace hitem
        tmp_read_item[0] = 0;
        tvitem.hItem = hitem;
        tvitem.mask = TVIF_TEXT;//|TVIF_IMAGE;
        tvitem.pszText = tmp_read_item;
        tvitem.cchTextMax = MAX_PATH;
        if (SendMessage(htv, TVM_GETITEM,(WPARAM)0, (LPARAM)&tvitem))
        {
          //check if a directory
          if(tvitem.iImage != ICON_DIRECTORY_REG) continue;
          else if (strcmp(tmp_item,charToLowChar(tmp_read_item)) == 0)break;
        }else return 0;
      }while((hitem = (HTREEITEM)SendMessage(htv, TVM_GETNEXTITEM,(WPARAM)TVGN_NEXT, (LPARAM)hitem)));

      if (hitem == 0) //not exist
      {
        //add directory
        for(;i<nb;i++)
        {
          tmp_item[0] = 0;
          ExtractTextFromPath(path, tmp_item, MAX_PATH, i);
          last_hitem = AddItemTreeViewImg(htv, tmp_item, last_hitem, ICON_DIRECTORY_REG);
        }

        //add items
        AddNextValueToTreeView(htv, last_hitem, value, value_type);
        return last_hitem;
      }
    }
  }

  if (i == nb && hitem != 0)
  {
    AddNextValueToTreeView(htv, hitem, value, value_type);
    return hitem;
  }
  return 0;
}
//------------------------------------------------------------------------------
void ReadPath(char *buffer, DWORD taille_fic, DWORD position, char *path, unsigned int path_size_max, char *parent, char *sid, unsigned int sid_size_max)
{
  path[0]=0;
  char tmp[MAX_LINE_SIZE];
  DWORD pos =position;
  HBIN_CELL_NK_HEADER *nk_h = (HBIN_CELL_NK_HEADER *)(buffer+pos);

  //Get SID
  sid[0] = 0;
  if (nk_h != NULL)
  {
    if (nk_h->key_name_size >0 && nk_h->key_name_size<taille_fic && nk_h->size>0 && nk_h->type == 0x6B6E)
    {
      if (nk_h->sk_offset != 0xFFFFFFFF && nk_h->sk_offset >0 && (HBIN_FIRST_DEFAULT+nk_h->sk_offset)<taille_fic)
      {
        //read owner and this rid + sid
        HBIN_CELL_SK_HEADER *sk = (HBIN_CELL_SK_HEADER *)&buffer[HBIN_FIRST_DEFAULT+nk_h->sk_offset];
        if (HBIN_FIRST_DEFAULT+nk_h->sk_offset+sk->owner_offset+SK_OWNER_SIZE_MAX<taille_fic)
        {
          SK_SID *sk_owner = (SK_SID *)&buffer[HBIN_FIRST_DEFAULT+nk_h->sk_offset+sk->owner_offset+SK_HEADER_DATA_SIZE];
          if (sk_owner->nb_ID > 0 && sk_owner->nb_ID < 0xff)
          {
            unsigned int i, nb = sk_owner->nb_ID;
            if (nb > 5)nb = 5;

            //SID
            if (sid != NULL && sk_owner->ID0 > 0)
            {
              BOOL ok = TRUE;
              for (i=0;i<nb;i++)
              {
                if (sk_owner->ID[i] < 1)ok=FALSE;
              }
              if (ok)
              {
                snprintf(sid,sid_size_max,"S-1-%u",sk_owner->ID0);
                for (i=0;i<nb;i++)snprintf(sid+strlen(sid),sid_size_max-strlen(sid),"-%lu",sk_owner->ID[i]);
              }
            }
          }
        }
      }
    }
  }

  //Get path
  while (0x1000+nk_h->parent_key < taille_fic && nk_h->parent_key > 0)
  {
    //lecture du nom
    if (nk_h->key_name_size >0 && nk_h->key_name_size<taille_fic && nk_h->size>0 && (pos-HBIN_CELL_NK_SIZE)&& nk_h->type == 0x6B6E)
    {
      tmp[0]=0;
      strncpy(tmp,(char *)(buffer+pos+HBIN_CELL_NK_SIZE),MAX_LINE_SIZE);
      if (nk_h->key_name_size>MAX_LINE_SIZE)tmp[MAX_LINE_SIZE-1]=0;
      else tmp[nk_h->key_name_size]=0;

      strncat(tmp,"\\\0",MAX_LINE_SIZE-strlen(tmp));
      strncat(tmp,path,MAX_LINE_SIZE-strlen(tmp));
      strncat(tmp,"\0",MAX_LINE_SIZE-strlen(tmp));
      snprintf(path,path_size_max,"%s",tmp);
    }

    pos = 0x1000+nk_h->parent_key;
    nk_h = (HBIN_CELL_NK_HEADER *)(buffer+pos);
  }

  //suppression de l'éventuel \\ de fin ^^
  pos = strlen(path);
  if (pos>0 && pos-1 < path_size_max)
  {
    if (path[pos-1]=='\\')
    {
      path[pos-1]=0;
    }
  }

  //add parent in first case
  if (parent != NULL)
  {
    snprintf(tmp,MAX_LINE_SIZE,"%s",path);
    char *c = tmp;
    while (*c && *c != '\\')c++;
    if (*c == '\\')
    {
      c++;
      snprintf(path,path_size_max,"%s%s",parent,c);
    }
  }
}
//------------------------------------------------------------------------------
//nk keys ( == directory)
DWORD Traiter_RegBin_nk(char *fic, HTREEITEM hparent, char *parent, DWORD position, DWORD taille_fic, char *buffer, HANDLE hlv,HANDLE htv, BOOL deleted)
{
  if (position + HBIN_CELL_NK_SIZE > taille_fic)return 1;
    HBIN_CELL_NK_HEADER *nk_h = (HBIN_CELL_NK_HEADER *)(buffer+position);
  //valide ?
  if (nk_h->key_name_size >0 && nk_h->key_name_size<taille_fic && nk_h->size>0 && (position-HBIN_CELL_NK_SIZE)&& nk_h->type == 0x6B6E)
  {
    if (nk_h->nb_values > 0 && nk_h->nb_values < 0xFFFFFFFF && nk_h->val_ls_offset < 0xFFFFFFFF && nk_h->val_ls_offset > 0)
    {
      LINE_ITEM lv_line[DLG_REG_LV_NB_COLUMN];
      char parent_key_update[DATE_SIZE_MAX];
      char Owner_SID[MAX_PATH];

      strcpy(lv_line[0].c,fic);

      //lecture du path complet + SID ^^
      ReadPath(buffer, taille_fic, position, lv_line[1].c,MAX_LINE_SIZE,parent,Owner_SID,MAX_PATH);
      if (deleted)strcpy(lv_line[7].c,"X\0");
      else lv_line[7].c[0]=0;

      lv_line[8].c[0]=0;
      if (nk_h->class_name_offset < 0xFFFFFFFF && nk_h->class_name_size > 0 && (HBIN_FIRST_DEFAULT+nk_h->class_name_offset)<taille_fic)
      {
        if (nk_h->class_name_size > 4) // bypass header 0xe8FFFFFF
          snprintf(lv_line[8].c,MAX_PATH,"%S",&buffer[HBIN_FIRST_DEFAULT+nk_h->class_name_offset+4]);
        else snprintf(lv_line[8].c,MAX_PATH,"%S",&buffer[HBIN_FIRST_DEFAULT+nk_h->class_name_offset]);
      }

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

      HTREEITEM htparent = 0;
      BOOL val_ok;
      for (i=0;i< (nk_h->nb_values);i++)
      {
        if (nk_h->val_ls_offset+0x1000+i*HBIN_CELL_ITEM_LS+4 >= taille_fic)break;
        item_ls = (S_ITEM_LS *)&buffer[nk_h->val_ls_offset+0x1000+i*HBIN_CELL_ITEM_LS+4];
        //lecture de l'offset de la valeure :
        if (item_ls->val_of>0 && (item_ls->val_of+0x1000+HBIN_CELL_VK_SIZE)<taille_fic)
        {
          //traitement des valeures !!!
          val_ok = TRUE;
          vk_h = (HBIN_CELL_VK_HEADER *)&buffer[item_ls->val_of+0x1000];

          //récupération du nom de la valeur :
          strncpy(lv_line[2].c,vk_h->value,MAX_LINE_SIZE);
          if (vk_h->name_size>=MAX_LINE_SIZE)lv_line[2].c[MAX_LINE_SIZE-1]=0;
          else
          {
            lv_line[2].c[vk_h->name_size]=0;
            if (strlen(lv_line[2].c) != vk_h->name_size)val_ok = FALSE;
          }

          switch(vk_h->data_type)
          {
            //Chaines
            case 0x00000001 : //REG_SZ, chaine ASCII et Unicodes
              lv_line[3].c[0] = 0;
              if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
              {
                if (vk_h->data_size < 5)
                {
                  snprintf(lv_line[3].c,MAX_LINE_SIZE,"%S",vk_h->cdata_offset);
                  lv_line[3].c[vk_h->data_size/2]=0;
                }else snprintf(lv_line[3].c,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE]);

                snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
              }else
              {
                if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                {
                  snprintf(tmp,MAX_LINE_SIZE,"%s=",lv_line[2].c);
                }else
                {
                  snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                }
              }
              strcpy(lv_line[4].c,"REG_SZ\0");
            break;
            case 0x00000002 : //REG_EXPAND_SZ, chaine ASCII et Unicodes, contient des path type %path%
              lv_line[3].c[0] = 0;
              if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
              {
                if (vk_h->data_size < 5)
                {
                  strcpy(lv_line[3].c,vk_h->cdata_offset);
                  lv_line[3].c[4]=0;
                }else snprintf(lv_line[3].c,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE]);

                snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
              }else
              {
                if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                {
                  snprintf(tmp,MAX_LINE_SIZE,"%s=",lv_line[2].c);
                }else
                {
                  snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                }
              }
              strcpy(lv_line[4].c,"REG_EXPAND_SZ\0");
            break;
            case 0x00000006 : //REG_LINK, chaine ASCII et Unicodes, lien lien
              lv_line[3].c[0] = 0;
              if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
              {
                if (vk_h->data_size < 5)
                {
                  strcpy(lv_line[3].c,vk_h->cdata_offset);
                  lv_line[3].c[4]=0;
                }else snprintf(lv_line[3].c,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE]);

                snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
              }else
              {
                if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                {
                  snprintf(tmp,MAX_LINE_SIZE,"%s=",lv_line[2].c);
                }else
                {
                  snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                }
              }
              strcpy(lv_line[4].c,"REG_LINK\0");
            break;
            case 0x00000007 : //REG_MULTI_SZ, multiples chaine ASCII et Unicodes, lien
                lv_line[3].c[0] = 0;

                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  if (vk_h->data_size < 5)
                  {
                    strcpy(lv_line[3].c,vk_h->cdata_offset);
                    lv_line[3].c[4]=0;
                  }else
                  {
                    //traitement des chaines multiples :
                    do
                    {
                      snprintf(tmp,MAX_LINE_SIZE,"%S",(wchar_t *)&buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+2*strlen(lv_line[3].c)]);
                      strncat (lv_line[3].c,tmp,MAX_LINE_SIZE-strlen(lv_line[3].c));
                      strncat (lv_line[3].c," \0",MAX_LINE_SIZE-strlen(lv_line[3].c));
                    }while (strlen(lv_line[3].c)*2 < (vk_h->data_size));
                  }

                  snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
                }else
                {
                  if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=",lv_line[2].c);
                  }else
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                  }
                }
                strcpy(lv_line[4].c,"REG_MULTI_SZ\0");
              break;
              case 0x00000003 : //REG_BINARY, données binaires
                lv_line[3].c[0] = 0;
                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  if (vk_h->data_size < 5)
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",vk_h->cdata_offset[k]&0xff);
                      strncat(lv_line[3].c,tmp,MAX_LINE_SIZE-strlen(lv_line[3].c));
                    }
                    strncat(lv_line[3].c,"\0",MAX_LINE_SIZE-strlen(lv_line[3].c));
                  }else
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+k]&0xff);
                      strncat(lv_line[3].c,tmp,MAX_LINE_SIZE-strlen(lv_line[3].c));
                    }
                    strncat(lv_line[3].c,"\0",MAX_LINE_SIZE-strlen(lv_line[3].c));
                  }
                  snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
                }else
                {
                  if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=",lv_line[2].c);
                  }else
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                  }
                }
                strcpy(lv_line[4].c,"REG_BINARY\0");
              break;
              case 0x0000000A : //REG_RESSOURCE_REQUIREMENT_LIST, données binaires
                lv_line[3].c[0] = 0;
                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  if (vk_h->data_size < 5)
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",vk_h->cdata_offset[k]&0xff);
                      strncat(lv_line[3].c,tmp,MAX_LINE_SIZE-strlen(lv_line[3].c));
                    }
                    strncat(lv_line[3].c,"\0",MAX_LINE_SIZE-strlen(lv_line[3].c));
                  }else
                  {
                    for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                    {
                      snprintf(tmp,10,"%02X",buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+k]&0xff);
                      strncat(lv_line[3].c,tmp,MAX_LINE_SIZE-strlen(lv_line[3].c));
                    }
                    strncat(lv_line[3].c,"\0",MAX_LINE_SIZE-strlen(lv_line[3].c));
                  }
                  snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
                }else
                {
                  if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=",lv_line[2].c);
                  }else
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                  }
                }
                strcpy(lv_line[4].c,"REG_RESSOURCE_REQUIREMENT_LIST\0");
              break;
              case 0x00000004 : //REG_DWORD, données numériques 32bitschar
              case 0x00000005 : //REG_DWORD, données numériques 32bits signées
                lv_line[3].c[0] = 0;
                if (vk_h->data_size > 0)
                {
                  snprintf(lv_line[3].c,MAX_LINE_SIZE,"%08X",(unsigned int)(vk_h->data_offset&0xFFFFFFFF)); // en dword offset = valeur
                  snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
                }else
                {
                  snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(data:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                }
                strcpy(lv_line[4].c,"REG_DWORD\0");
              break;
              case 0x0000000b : //REG_QWORD, données numériques 64bits signées
                lv_line[3].c[0] = 0;
                if (vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  c = &buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE];
                  snprintf(lv_line[3].c,MAX_LINE_SIZE,"0x%02X%02X%02X%02X%02X%02X%02X%02X",c[7]&0xff,c[6]&0xff,c[5]&0xff,c[4]&0xff,c[3]&0xff,c[2]&0xff,c[1]&0xff,c[0]&0xff);

                  snprintf(tmp,MAX_LINE_SIZE,"%s=%s",lv_line[2].c,lv_line[3].c);
                }else
                {
                  if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=",lv_line[2].c);
                  }else
                  {
                    snprintf(tmp,MAX_LINE_SIZE,"%s=(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,vk_h->data_size,vk_h->data_offset);
                  }
                }
                strcpy(lv_line[4].c,"REG_QWORD\0");
              break;
              default :
                if (deleted)val_ok = FALSE;
                //on ajoute pas au liste view les éléments inconnus non valides
                lv_line[3].c[0] = 0;
                if (vk_h->data_size < 0xFFFFFFFF && vk_h->data_offset > 0 && vk_h->data_offset < taille_fic)
                {
                  for (k=0;k<vk_h->data_size && k/2<MAX_LINE_SIZE;k++)
                  {
                    snprintf(tmp,10,"%02X",buffer[0x1000+vk_h->data_offset+HBIN_CELL_VK_DATA_PADDING_SIZE+k]&0xff);
                    strncat(lv_line[3].c,tmp,MAX_LINE_SIZE-strlen(lv_line[3].c));
                  }
                  strncat(lv_line[3].c,"\0",MAX_LINE_SIZE-strlen(lv_line[3].c));

                  if (vk_h->data_type == 0x00000000)strcpy(lv_line[4].c,"REG_NONE\0");
                  else
                  {
                    val_ok = FALSE;
                    //if (!deleted)snprintf(lv_line[4].c,MAX_LINE_SIZE,"0x%08X",(unsigned int)(vk_h->data_type & 0xFFFFFFFF));
                  }

                  snprintf(tmp,MAX_LINE_SIZE,"%s=(type:0x%08X)%s",lv_line[2].c,(unsigned int)(vk_h->data_type & 0xFFFFFFFF),lv_line[3].c);
                }else
                {
                  if (vk_h->data_type == 0x00000000)strcpy(lv_line[4].c,"REG_NONE\0");
                  else
                  {
                    val_ok = FALSE;
                    //if (!deleted)snprintf(lv_line[4].c,MAX_LINE_SIZE,"0x%08X",(unsigned int)(vk_h->data_type & 0xFFFFFFFF));
                  }
                  /*if (!deleted)
                  {
                    if(vk_h->data_offset == 0 && vk_h->data_size ==0)
                    {
                      snprintf(tmp,MAX_LINE_SIZE,"%s=(type:0x%08X)",lv_line[2].c,(unsigned int)(vk_h->data_type & 0xFFFFFFFF));
                    }else
                    {
                      snprintf(tmp,MAX_LINE_SIZE,"%s=(type:0x%08X)(len:%d)(off:%lu)<DATA ERROR>",lv_line[2].c,(unsigned int)(vk_h->data_type & 0xFFFFFFFF),vk_h->data_size,vk_h->data_offset);
                    }
                  }*/
                }
              break;
          }

          if (val_ok && (strlen(lv_line[2].c) || strlen(lv_line[3].c)))
          {
            //ajout de l'item
            //if (i == 0 || htparent == 0) htparent = AddFirstValueToTreeView(htv, hparent, lv_line[1].c, tmp, vk_h->data_type);
            //else AddNextValueToTreeView(htv, htparent, tmp, vk_h->data_type);

            strcpy(lv_line[5].c,parent_key_update);
            strcpy(lv_line[6].c,Owner_SID);
            AddToLVRegBin(hlv, lv_line, DLG_REG_LV_NB_COLUMN);
          }
        }else break;
      }

      //no value : only directory
      if ((nk_h->nb_values < 1 || nk_h->nb_values == 0xFFFFFFFF) && (nk_h->nb_subkeys < 1 || nk_h->nb_subkeys == 0xFFFFFFFF))
      {
        lv_line[2].c[0] = 0;
        lv_line[3].c[0] = 0;
        lv_line[4].c[0] = 0;
        strcpy(lv_line[5].c,parent_key_update);
        strcpy(lv_line[6].c,Owner_SID);
        AddToLVRegBin(hlv, lv_line, DLG_REG_LV_NB_COLUMN);
      }

      DWORD nb = ListView_GetItemCount(hlv);
      if (nb % 1000 == 0)
      {
        char tmp[MAX_PATH];
        snprintf(tmp,MAX_PATH,"Loading... %lu keys",nb);
        SendMessage(GetDlgItem(h_reg,STB),SB_SETTEXT,0, (LPARAM)tmp);
      }
    }
    return HBIN_CELL_NK_SIZE;
  }
  return 1;
}
//------------------------------------------------------------------------------
void GetRecoveryRegFile(char *reg_file, HTREEITEM hparent, char *parent, HANDLE hlv, HANDLE htv)
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
            case 0x6B6E:  position = position + Traiter_RegBin_nk(reg_file,hparent, parent, position, taille_fic, buffer,hlv,htv,FALSE);break; //nk
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
            position = position + Traiter_RegBin_nk(reg_file,hparent, parent, position, taille_fic, buffer,hlv,htv,TRUE);
        }else position++;
      }
    }
  }
  //on libère la mémoire
  HeapFree(GetProcessHeap(), 0, buffer);
}
