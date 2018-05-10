//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void addFileLNKtoDB(char *file, char *create_time, char *last_access_time, char *last_modification_time,
                   char *local_path, char *to, unsigned int session_id, sqlite3 *db)
{
  #ifndef CMD_LINE_ONLY_NO_DB
  char request[REQUEST_MAX_SIZE];
  snprintf(request,REQUEST_MAX_SIZE,
           "INSERT INTO extract_file_nk "
           "(file,create_time,last_access_time,last_modification_time,local_path,to_,session_id) "
           "VALUES(\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",\"%s\",%d);",
           file,create_time,last_access_time,last_modification_time,local_path,to,session_id);
  sqlite3_exec(db,request, NULL, NULL, NULL);
  #else
  printf("\"FileLNK\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%s\";\"%d\";\r\n",
         file,create_time,last_access_time,last_modification_time,local_path,to,session_id);
  #endif
}
//------------------------------------------------------------------------------
void read_datas_lnk(unsigned char *buffer, DWORD taille_fic,
                    char *create_time, char *last_access_time, char *last_modification_time,
                    unsigned char *local_path, unsigned char *to)
{
  typedef struct
  {
    unsigned int header_size;         // default : 0x0000004C (76)
    unsigned char lnk_class_id[16];   // default : 00021401-0000-0000-00c0-000000000046
    unsigned int data_flag;
    unsigned int file_attribute_flag;
    FILETIME create_time;
    FILETIME last_access_time;
    FILETIME last_modification_time;
    unsigned int file_size;
    unsigned int icon_index;
    unsigned int show_window_value;
    unsigned short hot_key;
    unsigned char reserved[10];
    unsigned short item_list_size;
  }LNK_STRUCT, *PLNK_STRUCT;
  unsigned char tmp[MAX_PATH]="", *b;

  //header
  PLNK_STRUCT p = (PLNK_STRUCT)buffer;

  //get times
  filetimeToString_GMT(p->create_time, create_time, DATE_SIZE_MAX);
  filetimeToString_GMT(p->last_access_time, last_access_time, DATE_SIZE_MAX);
  filetimeToString_GMT(p->last_modification_time, last_modification_time, DATE_SIZE_MAX);

  //unicode or not
  BOOL unicode = FALSE;
  if ((p->data_flag & 0x80) == 0x80) unicode = TRUE;

  //if items list we pass
  if ((p->data_flag & 0x01) == 0x01)//hashlinktargetid
  {
    b = buffer + 2 + p->header_size + p->item_list_size;
  }else b = buffer + p->header_size;

  if (b-buffer < taille_fic)
  {
    typedef struct
    {
      unsigned short struct_size;
    }LNK_WORD_STRUCT, *PLNK_WORD_STRUCT;

    //link info
    if ((p->data_flag & 0x02) == 0x02)
    {
      typedef struct
      {
        DWORD struct_size;
        DWORD header_size;
        DWORD flags;
        DWORD vol_ID_offset;
        DWORD local_base_path_offset;
        DWORD Network_relative_path_offset;
        DWORD commun_path_offset;

        //infos
        DWORD CommonNetworkRelativeLinkSize;
        DWORD CommonNetworkRelativeLinkFlags;
        DWORD NetNameOffset;
        DWORD DeviceNameOffset;
        DWORD NetWorkProviderType;
      }LNK_INFO_STRUCT, *PLNK_INFO_STRUCT;
      PLNK_INFO_STRUCT pi = (PLNK_INFO_STRUCT)b;

      if ((pi->flags & 0x01) == 0x01 && (pi->flags & 0x02)  == 0x02)
      {
        if (pi->local_base_path_offset && pi->local_base_path_offset < pi->struct_size)snprintf(to,MAX_PATH,"%s",(char*)(b+pi->local_base_path_offset));

        if (pi->NetNameOffset && pi->NetNameOffset+pi->header_size < pi->struct_size)snprintf(local_path,MAX_PATH,"%s",(char*)(b+pi->NetNameOffset+pi->header_size));

        if (pi->DeviceNameOffset && pi->DeviceNameOffset+pi->header_size < pi->struct_size)snprintf(tmp,MAX_PATH,"%s\\",(char*)(b+pi->DeviceNameOffset+pi->header_size));

        if (pi->commun_path_offset && pi->commun_path_offset < pi->struct_size)snprintf(to,MAX_PATH,"%s%s",tmp,(char*)(b+pi->commun_path_offset));
      }else if ((pi->flags & 0x01) == 0x01) //local
      {
        if (pi->local_base_path_offset && pi->local_base_path_offset < pi->struct_size)snprintf(to,MAX_PATH,"%s",(char*)(b+pi->local_base_path_offset));
      }else if ((pi->flags & 0x02)  == 0x02) //network
      {
        //local path
        if (pi->NetNameOffset && pi->NetNameOffset+pi->header_size < pi->struct_size)snprintf(local_path,MAX_PATH,"%s",(char*)(b+pi->NetNameOffset+pi->header_size));

        if (pi->DeviceNameOffset && pi->DeviceNameOffset+pi->header_size < pi->struct_size)snprintf(tmp,MAX_PATH,"%s\\",(char*)(b+pi->DeviceNameOffset+pi->header_size));

        if (pi->commun_path_offset && pi->commun_path_offset < pi->struct_size)snprintf(to,MAX_PATH,"%s%s",tmp,(char*)(b+pi->commun_path_offset));
      }
      b = b + pi->struct_size;

    }else if ((p->data_flag & 0x04) == 0x04 || (p->data_flag & 0x08) == 0x08/* || (p->data_flag & 0x10) == 0x10*/)
    {
      //other
      if (*local_path == '\0' && *to == '\0' && b-buffer < taille_fic)
      {
        DWORD tmp_size;

      //pass datas no used

        //0x04 = have name/description first ?
        PLNK_WORD_STRUCT t = (PLNK_WORD_STRUCT)b;
        if ((p->data_flag & 0x04) == 0x04)
        {
          //0x08 = relative path
          if ((p->data_flag & 0x08) == 0x08)
          {
            if (unicode) b = b + t->struct_size*2 + 2;
            else b = b + t->struct_size + 2;
            t = b;

            if(b-buffer < taille_fic)
            {
              if (unicode) snprintf(to,MAX_PATH,"%S",b+2);
              else snprintf(to,MAX_PATH,"%s",(char*)(b+2));

              tmp_size = t->struct_size +b-buffer ;
              if (tmp_size < taille_fic && tmp_size < MAX_PATH) to[t->struct_size] = 0;

              if (unicode) b = b + t->struct_size*2 + 2;
              else b = b + t->struct_size + 2;
              t = b;

              //0x10 = workingdir
              if ((p->data_flag & 0x10) == 0x10 && b-buffer < taille_fic)
              {
                if (unicode) b = b + t->struct_size*2 + 2;
                else b = b + t->struct_size + 2;
                t = b;
              }

              //0x20 = arguments
              if ((p->data_flag & 0x20) == 0x20 && b-buffer < taille_fic)
              {
                if (unicode) snprintf(tmp,MAX_PATH," %S",b+2);
                else snprintf(tmp,MAX_PATH," %s",(char*)(b+2));

                tmp_size = t->struct_size +b-buffer;
                if (tmp_size < taille_fic && tmp_size+1 < MAX_PATH) tmp[t->struct_size+1] = 0;

                strncat(to,tmp,MAX_PATH);
                strncat(to,"\0",MAX_PATH);
              }
            }
          }else
          {
            if (unicode) snprintf(to,MAX_PATH,"%S",b+2);
            else snprintf(to,MAX_PATH,"%s",(char*)(b+2));

            tmp_size = t->struct_size +b-buffer ;
            if (tmp_size < taille_fic && tmp_size < MAX_PATH) to[t->struct_size] = 0;
          }
        }else
        {
          //0x08 = relative path
          if ((p->data_flag & 0x08) == 0x08 && b-buffer < taille_fic)
          {
            //if only we use it !!!
            if (unicode) snprintf(to,MAX_PATH,"%S",b+2);
            else snprintf(to,MAX_PATH,"%s",(char*)(b+2));

            tmp_size = t->struct_size +b-buffer ;
            if (tmp_size < taille_fic && tmp_size < MAX_PATH) to[t->struct_size] = 0;

            if (unicode) b = b + t->struct_size*2 + 2;
            else b = b + t->struct_size + 2;
            t = b;

            //0x10 = workingdir
            if ((p->data_flag & 0x10) == 0x10 && b-buffer < taille_fic)
            {
              if (unicode) b = b + t->struct_size*2 + 2;
              else b = b + t->struct_size + 2;
              t = b;
            }

            //0x20 = arguments
            if ((p->data_flag & 0x20) == 0x20 && b-buffer < taille_fic)
            {
              if (unicode) snprintf(tmp,MAX_PATH," %S",b+2);
              else snprintf(tmp,MAX_PATH," %s",(char*)(b+2));

              tmp_size = t->struct_size +b-buffer;
              if (tmp_size < taille_fic && tmp_size+1 < MAX_PATH) tmp[t->struct_size+1] = 0;

              strncat(to,tmp,MAX_PATH);
              strncat(to,"\0",MAX_PATH);
            }
          }else
          {
            //0x10 = workingdir
            if ((p->data_flag & 0x10) == 0x10 && b-buffer < taille_fic)
            {
              if (unicode) snprintf(to,MAX_PATH,"%S",b+2);
              else snprintf(to,MAX_PATH,"%s",(char*)(b+2));

              tmp_size = t->struct_size +b-buffer;
              if (tmp_size < taille_fic && tmp_size < MAX_PATH) to[t->struct_size] = 0;

              if (unicode) b = b + t->struct_size*2 + 2;
              else b = b + t->struct_size + 2;
              t = b;
            }

            //0x20 = arguments
            if ((p->data_flag & 0x20) == 0x20 && b-buffer < taille_fic)
            {
              if (unicode) snprintf(tmp,MAX_PATH," %S",b+2);
              else snprintf(tmp,MAX_PATH," %s",b+2);

              tmp_size = t->struct_size +b-buffer;
              if (tmp_size < taille_fic && tmp_size+1 < MAX_PATH) tmp[t->struct_size+1] = 0;

              strncat(to,tmp,MAX_PATH);
              strncat(to,"\0",MAX_PATH);
            }
          }
        }
      }
    }else if ((p->data_flag & 0x200) == 0x200 || p->data_flag == 0x80) // windows 8 format : ext_string
    {
      unsigned char *pos = b;
      PLNK_WORD_STRUCT t = (PLNK_WORD_STRUCT)b;

      if ((p->data_flag & 0x10) == 0x10)  //workingdir
      {
        if (unicode) pos = pos + t->struct_size*2 + 2;
        else pos = pos + t->struct_size + 2;
      }

      if (pos-buffer < taille_fic)
      {
        t = pos;
        if ((p->data_flag & 0x20) == 0x20)  //arguments
        {
          if (unicode) pos = pos + t->struct_size*2 + 2;
          else pos = pos + t->struct_size + 2;
        }

        if (pos-buffer < taille_fic)
        {
          t = pos;
          if ((p->data_flag & 0x40) == 0x40)  //iconlocation
          {
            if (unicode) pos = pos + t->struct_size*2 + 2;
            else pos = pos + t->struct_size + 2;
          }

          //pass first headers !!
          if (pos-buffer < taille_fic)
          {
            t = pos;
            if (pos-buffer+t->struct_size+8 < taille_fic)
            {
              snprintf(to,MAX_PATH,"%s",(char*)(pos+t->struct_size+8));
            }
          }
        }
      }
    }

    if (*local_path == '\0' && *to == '\0' && b-buffer < taille_fic)
    {
      if ((p->data_flag & 0x01) == 0x01)
      {
        //first struct
        unsigned char *pos = buffer + p->header_size + 2;
        if (pos-buffer < taille_fic)
        {
          PLNK_WORD_STRUCT t = pos;
          pos = pos + t->struct_size;

          //second struct
          if (pos-buffer+8 < taille_fic)
          {
            if (unicode)snprintf(to,MAX_PATH,"%S",pos+8);
            else snprintf(to,MAX_PATH,"%s",(char*)(pos+8));
          }
        }
      }
    }
  }
}
//------------------------------------------------------------------------------
//http://liblnk.googlecode.com/files/Windows%20Shortcut%20File%20%28LNK%29%20format.pdf
void ReadLNKInfos(char *file, unsigned int session_id, sqlite3 *db)
{
  char create_time[DATE_SIZE_MAX]="",last_access_time[DATE_SIZE_MAX]="",last_modification_time[DATE_SIZE_MAX]="";
  unsigned char local_path[MAX_PATH]="",to[MAX_PATH]="";

  HANDLE Hfic = CreateFile(file,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,0,0);
  if (Hfic != INVALID_HANDLE_VALUE)
  {
    DWORD taille_fic = GetFileSize(Hfic,NULL);
    if (taille_fic>0 && taille_fic!=INVALID_FILE_SIZE)
    {
      unsigned char *buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(unsigned char*)*taille_fic+1);
      if (buffer != NULL)
      {
        DWORD copiee = 0;
        ReadFile(Hfic, buffer, taille_fic,&copiee,0);
        if (copiee>0)
        {
          if (copiee != taille_fic) taille_fic = copiee;
          read_datas_lnk(buffer, taille_fic,create_time,last_access_time,last_modification_time,
                         local_path,to);
        }
        HeapFree(GetProcessHeap(), 0,buffer);
      }
    }
  }else
  {
    //for file format extern LNK bug !!!
    FILE *ffile = fopen(file, "rb" );
    if (ffile != NULL)
    {
      fseek(ffile, 0, SEEK_END);
      DWORD taille_fic = ftell(ffile);
      if (taille_fic>0)
      {
        unsigned char *buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(unsigned char*)*taille_fic+1);
        if (buffer != NULL)
        {
          fseek(ffile, 0, SEEK_SET);
          DWORD copiee = fread (buffer,1,taille_fic,ffile);
          if (copiee>0)
          {
            if (copiee != taille_fic) taille_fic = copiee;
            read_datas_lnk(buffer, taille_fic,create_time,last_access_time,last_modification_time,
                           local_path,to);
          }
          HeapFree(GetProcessHeap(), 0,buffer);
        }
      }
      fclose(ffile);
    }
  }
  addFileLNKtoDB(file,create_time,last_access_time,last_modification_time,local_path,to,session_id,db);
  CloseHandle(Hfic);
}
