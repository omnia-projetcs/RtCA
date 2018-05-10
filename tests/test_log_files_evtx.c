//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
#define NB_ST_ITEM        100
typedef struct _STR_T
{
  DWORD pos;
  unsigned short id;
  char s[DEFAULT_TMP_SIZE];
}STR_T;

typedef struct _STRING_TABLE
{
  STR_T item[NB_ST_ITEM];
  short Qualifiers;  //ID
  short Computer;    //User-SID
  short UserID;      //User-SID
  short Provider;    //Source
  short Level;       //Type
}STRING_TABLE;

//début de données qui fait 8 octets
#define STH     8
typedef struct _STRING_TABLE_HEADER
{
  unsigned short unknow1;
  unsigned short unknow2;
  unsigned short unknow3;
  unsigned short size;
}STRING_TABLE_HEADER;

//------------------------------------------------------------------------------
unsigned int ReadStringTable(char *buffer, DWORD size, DWORD pos_real,STRING_TABLE *my_s_table, unsigned int nb_item_ex_table)
{
  //les données sont stockées en unsigned short à la suite ^^
  typedef struct s_i
  {
    unsigned int off;
  }S_I;
  S_I *o;
  char *b = buffer;
  unsigned int pos = 0;
  unsigned int nb = 0, id=0;

  my_s_table->Qualifiers  = -1;
  my_s_table->Computer    = -1;
  my_s_table->UserID      = -1;
  my_s_table->Provider    = -1;
  my_s_table->Level       = -1;


  do
  {
    o = (S_I *)b;
    if (o->off > 0 && o->off<size)
    {
      //on commence a off+0x80 et l'entête de data est de 8 octets
      snprintf(my_s_table->item[nb].s,DEFAULT_TMP_SIZE,"%S (%d)",buffer-0x80+STH+o->off,id);

      if (strlen(my_s_table->item[nb].s)>1)
      {
        if (!strcmp("Qualifiers"    ,my_s_table->item[nb].s))my_s_table->Qualifiers = nb;
        else if (!strcmp("Computer" ,my_s_table->item[nb].s))my_s_table->Computer   = nb;
        else if (!strcmp("UserID"   ,my_s_table->item[nb].s))my_s_table->UserID     = nb;
        else if (!strcmp("Provider" ,my_s_table->item[nb].s))my_s_table->Provider   = nb;
        else if (!strcmp("Level"    ,my_s_table->item[nb].s))my_s_table->Level      = nb;
        my_s_table->item[nb].pos = pos_real+STH+o->off;
        my_s_table->item[nb++].id = id;
      }
    }
    id++;
    b+=4;
    pos+=4;
  }while(pos < 0x100 && nb<nb_item_ex_table);

  return nb;
}
//------------------------------------------------------------------------------
void TraiterDescription (char *buffer,DWORD size_buffer,char *description,DWORD size_description)
{
  char *s = buffer;
  char *d = description;
  DWORD j =0, i =0,k;

  while (j < size_buffer-1 && i<size_description)
  {
    while (*s == 0x00 && j<size_buffer ){s++;j++;};

    //copie
    k=0;
    while (j < size_buffer-1 && i<size_description && *s != 0x00 && (d-description < size_description))
    {
      if (*s < 32 || *s > 126)s++;
      else
      {
        *d++=*s++;
        i++;
        s++;
        j+=2;
        k++;
      }
    }

    //traitement des chaines incorrectes
    if (k<4)
    {
      while (k>1){d--;i--;k--;}
    }else
    {
      *d++=' ';
      i++;
    }
  }
  *d = 0;
}
//------------------------------------------------------------------------------
unsigned int ReadRecord(char *buffer, DWORD size, STRING_TABLE *my_s_table, char *eventfile, sqlite3 *db, unsigned int session_id)
{
  #define EVENTLOG_H_EVTX_SIZE                    4+4+8+8+6
  typedef struct _EVENTLOG_H_EVTX {
    unsigned char MagicString[4];                 //4 ** + 0x00 + 0x00
    unsigned int RecordSize;                      //4
    unsigned long long int RecordNumber;          //8   //RecordNumber
    unsigned long long int TimeCreated;           //8
    unsigned char padding_star_xml[6];            //6 : 0f01 0100 0c01
  }EVENTLOG_H_EVTX;
  EVENTLOG_H_EVTX *h_dheader = (EVENTLOG_H_EVTX *)buffer;

  char indx[DEFAULT_TMP_SIZE]="", log_id[DEFAULT_TMP_SIZE]="",
  send_date[DATE_SIZE_MAX]="", write_date[DATE_SIZE_MAX]="",
  source[MAX_PATH]="", description[MAX_LINE_SIZE]="",
  user[DEFAULT_TMP_SIZE]="", rid[DEFAULT_TMP_SIZE]="", sid[DEFAULT_TMP_SIZE]="",
  state[DEFAULT_TMP_SIZE]="", critical[DEFAULT_TMP_SIZE]="";

  //DWORD id=0;

  //test de validité
  if (h_dheader->RecordSize <= size
   && h_dheader->MagicString[0] == '*' && h_dheader->MagicString[1] == '*' && h_dheader->MagicString[2] == 0 && h_dheader->MagicString[3] == 0)
  {
    //lecture des informations du header
    //id
    snprintf(indx,DEFAULT_TMP_SIZE,"%08lu",(long unsigned int)(h_dheader->RecordNumber));

    //date : send_date
    FILETIME FileTime;
    FileTime.dwLowDateTime = (DWORD) h_dheader->TimeCreated;
    FileTime.dwHighDateTime = (DWORD)(h_dheader->TimeCreated >> 32);
    filetimeToString_GMT(FileTime, send_date, DATE_SIZE_MAX);

    //date : write_date = same
    strcpy(write_date,send_date);

    //afin de passer les enregistrelent avec header on passe tant qu'on à pas accès au header attendu ^^
    //permet de traiter les cas de données corrompues
    //0x00 0000 0100 0400 0100 0400
    char *c = buffer+EVENTLOG_H_EVTX_SIZE;
    DWORD pos=EVENTLOG_H_EVTX_SIZE;
    BOOL ok =FALSE;
    do
    {
      if (*c == 0x00 && *(c+1) == 0x00 && *(c+2) == 0x00 &&
      *(c+3) == 0x01 && *(c+4) == 0x00 && *(c+5) == 0x04 && *(c+6) == 0x00 &&
      *(c+7) == 0x01 && *(c+8) == 0x00 && *(c+9) == 0x04 &&*(c+10) == 0x00)
      {
        ok = TRUE;
        break;
      }

      c++;
      pos++;
    }while(pos<h_dheader->RecordSize-12);

    //traitement de la suite ^^
    if (ok)
    {
      //recherche de l'id ^^ dans une zone de moins de 100octets ^^
      //0x2100 0400 0000 XXXX = ID
      typedef struct _ID
      {
        unsigned short param;   // 0x0021
        unsigned char end;
        unsigned char type;     // level
        unsigned short padding; // 0x0000
        unsigned short id;
      }M_ID;
      M_ID * mid;

      //unsigned short id;

      while(pos<h_dheader->RecordSize-10)
      {
        if (*c == 0x21 && *(c+1) == 0x00)
        {
          //id identifié ^^
          mid = (M_ID *)c;
          snprintf(log_id,DEFAULT_TMP_SIZE,"%08lu",(DWORD) mid->id);
          //id = mid->id;

          //Type
          switch(mid->type)
          {
            case 0x00 : strcpy(state,"INFORMATION"); break;
            case 0x01 : strcpy(state,"WARNING"); break;
            case 0x02 : strcpy(state,"ERROR"); break;
            case 0x04 : strcpy(state,"CRITICAL"); break;
            case 0x08 : strcpy(state,"AUDIT_SUCCESS"); break;
            case 0x10 : strcpy(state,"AUDIT_FAILURE"); break;
            default :snprintf(state,DEFAULT_TMP_SIZE,"UNKNOW (%d)",mid->type);break;
          }
        }
        c++;pos++;
      }
      c+=8; //on passe l'id + 6 octets de padding

      //Source
      char *desc = c;
      source[0] = 0;
      while(pos+4<h_dheader->RecordSize-10)
      {//0x05- 0x12 = S-1-5-18, revoir la phase de détection pour prendre en compte le SID !!!!
        if (*c != 0x00 && *(c+1) == 0x00) //unicode
          if (*(c+2) != 0x00 && *(c+3)==0x00)
          //if (*c == 0x05)
          {
            snprintf(source,MAX_PATH,"%S",c);
            break;
          }
        c++;
        pos++;
      }

      //si aucune source n'est définie on ajoute le nom du fichier sans evtx ^^
      if (source[0] == 0)
      {
        char *n = eventfile;
        while (*n)n++;
        while (*n != '\\')n--;
        n++;
        snprintf(source,MAX_PATH,"%s",n);
        source[strlen(source)-5]=0;

        //on vérifie que pas de % dans le nom sinon on met 0 à sa place ^^
        n = source;
        while (*n)
        {
          if (*n == '%'){*n=0; break;}
          n++;
        }
      }

      //traitement de la description !!
      TraiterDescription (desc,h_dheader->RecordSize/*-pos*/,description,MAX_LINE_SIZE);
    }

    //add
    convertStringToSQL(source, MAX_PATH);
    convertStringToSQL(description, MAX_LINE_SIZE);
    addLogtoDB(eventfile, indx, log_id,
           send_date, write_date, source, description,
           user, rid, sid, state, critical, session_id, db);
  }
  return h_dheader->RecordSize;
}
//------------------------------------------------------------------------------
//header
typedef struct _EVENTLOGHEADER_EVTX {
  char MagicString[8];                 //8 ElfFile + 0x00
  unsigned long long int FirstChunk;            //8
  unsigned long long int LastChunk;             //8
  unsigned long long int NextRecord;            //8
  unsigned int HeaderSize;                      //4 toujours 0x80 = 128
  unsigned short MinorVersion;                  //2 toujours 1
  unsigned short MajorVersion;                  //2 toujours 3
  unsigned long long int CurrentSize;           //8
  unsigned char Reserved[72];                   //72
  unsigned int flag;                            //4
  unsigned int checksum;                        //4 checksum
}EVENTLOGHEADER_EVTX;

//record
typedef struct _EVENTLOGCHUNKHEADER_EVTX {
  char MagicString[8];                 //8 ElfChnk + 0x00
  unsigned long long int FirstRecord;           //8
  unsigned long long int LastRecord;            //8
  unsigned long long int FirstLogicRecord;      //8
  unsigned long long int LastLogicRecord;       //8
  unsigned int HeaderSize;                      //4 toujours 0x80 = 128
  unsigned int LastRecord_Offset;               //4
  unsigned int NextRecord_Offset;               //4
  unsigned int checksum_object;                 //4
  unsigned char Reserved[64];                   //64
  unsigned int flag;                            //4
  unsigned int checksum;                        //4 checksum
}EVENTLOGCHUNKHEADER_EVTX;

//------------------------------------------------------------------------------
void TraiterEventlogFileEvtx(char *eventfile, sqlite3 *db, unsigned int session_id)
{
  HANDLE Hlog = CreateFile(eventfile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (Hlog != INVALID_HANDLE_VALUE)
  {
    DWORD taille_fic = GetFileSize(Hlog,NULL);
    if (taille_fic>0 && taille_fic>0x126C && taille_fic!=INVALID_FILE_SIZE)
    {
      unsigned char *b, *buffer;
      DWORD copiee =0, position = 0, increm = 0;
      if (taille_fic > DIXM)
      {
        buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(unsigned char*)*DIXM+1);
        increm = DIXM;
      }else
      {
        buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(unsigned char*)*taille_fic+1);
        increm = taille_fic;
      }

      if (buffer == NULL)
      {
        CloseHandle(Hlog);
        return;
      }
      //read file header
      //SetFilePointer(Hlog,0x10000,NULL,FILE_BEGIN);
      ReadFile(Hlog, buffer, increm,&copiee,0);
      position +=copiee;
      if (taille_fic-position < increm)increm = taille_fic-position ;
      b = buffer + 0x1000;

      //file ok ?
      EVENTLOGHEADER_EVTX* h_evtx = (EVENTLOGHEADER_EVTX*)buffer;
      if (strcmp("ElfFile",h_evtx->MagicString)==0 && h_evtx->HeaderSize == 0x80 && h_evtx->MinorVersion>0 && h_evtx->MajorVersion>0)
      {
        EVENTLOGCHUNKHEADER_EVTX *h_cheader;
        STRING_TABLE my_s_table;
        DWORD p,tp=0,pos = 0x1000;
        DWORD i,nb_enrg = 0;
        DWORD last = 0, tmp_increm, tmp_cpy;
        unsigned int nb;

        while(position<=taille_fic && copiee>0 && start_scan)
        {
          do
          {
            h_cheader = (EVENTLOGCHUNKHEADER_EVTX *)b;
            if (strcmp(h_cheader->MagicString,"ElfChnk")!=0)break;

            //lecture du nombre d'enregistrement de se chunk
            nb_enrg = h_cheader->LastRecord - h_cheader->FirstRecord + 1;

            nb = ReadStringTable(b+0x80, copiee-0x80-pos,pos, &my_s_table, NB_ST_ITEM);
            if (nb>0)
            {
              b+=0x200;
              i=0;
              p=0;
              do
              {
                b+=p;
                p = ReadRecord(b, copiee-tp,&my_s_table,eventfile,db,session_id);
                i++;
                tp+=p;
              }while ( p!= 0 && i<nb_enrg && tp < copiee && start_scan);
            }
            //next session
            if (pos + 0x10280 > copiee)break;

            pos+=0x10000;
            tp = pos;
            b = buffer + pos;
          }while (pos<(copiee) && start_scan  && (b+sizeof(EVENTLOGCHUNKHEADER_EVTX)) < (buffer+copiee));

          if (increm == 0)break;

          //next read
          if (pos == copiee)
          {
            tp = 0;
            pos = 0;
            if (taille_fic-position < increm)increm = taille_fic-position;
            if (increm <1 || increm > DIXM)break;
            if(!ReadFile(Hlog, buffer, increm,&copiee,0))break;
            position +=copiee;
            b = buffer;
          }else
          {
            //all buffer have not been read !!!
            //copy in next buffer datas to read
            tmp_cpy = increm-pos;
            if (tmp_cpy > 0 && tmp_cpy < increm) memcpy(buffer,buffer+pos,tmp_cpy);

            if (taille_fic-position < (increm-tmp_cpy))tmp_increm = taille_fic-position;
            else tmp_increm = increm-tmp_cpy;
            if (tmp_increm <1 || tmp_increm > increm)break;

            tp = 0;
            pos = 0;
            if(!ReadFile(Hlog, buffer+tmp_cpy, tmp_increm,&copiee,0))break;
            position +=copiee;
            copiee +=tmp_cpy;
            b = buffer;
          }

          //anti looping
          if (last != position)last = position;
          else break;
        }
      }
      //free memory
      HeapFree(GetProcessHeap(), 0, buffer);
    }
  }
  CloseHandle(Hlog);
}
