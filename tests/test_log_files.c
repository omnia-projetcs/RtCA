//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
void TraiterEventlogFileLog(char * eventfile, sqlite3 *db, unsigned int session_id)
{
  //ouverture du fichier
  HANDLE Hlog = CreateFile(eventfile,GENERIC_READ,FILE_SHARE_READ,0,OPEN_EXISTING,FILE_FLAG_SEQUENTIAL_SCAN,0);
  if (Hlog != INVALID_HANDLE_VALUE)
  {
    //lecture du contenu
    long /*i,j=0,k,*/taille_fic = GetFileSize(Hlog,NULL);
    if (taille_fic > 17 && taille_fic!=INVALID_FILE_SIZE) // 17 = taile d'une ligne minimum
    {
      unsigned char *b, *buffer = (LPBYTE)HeapAlloc(GetProcessHeap(), 0, sizeof(unsigned char*)*taille_fic+1);
      b = buffer;
      if (buffer != NULL)
      {
        //read file datas
        DWORD copiee, position = 0, increm = 0;
        if (taille_fic > DIXM)increm = DIXM;
        else increm = taille_fic;

        while (position<taille_fic && increm!=0)
        {
          copiee = 0;
          ReadFile(Hlog, buffer+position, increm,&copiee,0);
          position +=copiee;
          if (taille_fic-position < increm)increm = taille_fic-position ;
        }
        #define DATE_BAD_FORMAT             0x0000
        #define DATE_FORMAT_SIMPLE          0x0010    //2012/12/31?10:10:10?Description
        #define DATE_FORMAT_SIMPLE_INVERT   0x0020    //31/12/2012?10:10:10?Description
        #define DATE_FORMAT_FULL            0x0030    //2011-08-08?19:37:52+0200?*?*?Description (if \t : source\tdescription)
        #define DATE_FORMAT_FULL_GUID       0x0031    //{*}?2011-08-08?19:37:052+0200?*{*}?*?*?source?state?Description
        #define DATE_FORMAT_UNIX            0x0040    //Dec?31?10:10:10?PC_NAME?source:?Description
        #define DATE_FORMAT_ENCR            0x0050    //[2012-12-31?10:10:10]?Description
        unsigned int DATE_FORMAT = DATE_BAD_FORMAT;

        //unicode file format ?
        BOOL unicode_file = FALSE;
        if (buffer[0] == 0xFF && buffer[1] == 0xFE)unicode_file = TRUE;

        //working line by line
        char line[MAX_LINE_SIZE], line_unicode[MAX_LINE_DBSIZE], *l, *c;
        char indx[DEFAULT_TMP_SIZE], send_date[DATE_SIZE_MAX], source[MAX_PATH],
             user[MAX_PATH], state[MAX_PATH], description[MAX_LINE_SIZE];
        DWORD j=0;
        b = buffer;

        //read first line
        if (unicode_file)
        {
          memset(line_unicode,0,MAX_LINE_DBSIZE);
          l = line_unicode;
          while ((*b!=0 || *(b+1)!=0) && ((l-line_unicode) < MAX_LINE_DBSIZE) && *b != '\r' && *b != '\n')*l++ = *b++;
          if (*b == '\r')b+=4;
          else if (*b == '\n')b+=2;
          *l++=0;
          *l=0;
          snprintf(line,MAX_LINE_SIZE,"%S",line_unicode+2);
        }else
        {
          memset(line,0,MAX_LINE_SIZE);
          l = line;
          while (*b && ((l-line) < MAX_LINE_SIZE) && *b != '\r' && *b != '\n')*l++ = *b++;

          if (*b == '\r')b+=2;
          else if (*b == '\n')b++;
          *l=0;
        }

        //determine file format !
        if (strlen(line) > 17)
        {
          if (line[0] == '[')
          {
            if (line[5] == '-' && line[8] == '-' && line[14] == ':' && line[17] == ':' && line[20] == ']')
              DATE_FORMAT = DATE_FORMAT_ENCR;
          }else if (line[0] == '{')
          {
            l = line;
            while (*l && *l != '}')l++;
            if (*l == '}')
            {
              if (*(l+6) == '-' && *(l+9) == '-' && *(l+15) == ':' && *(l+18) == ':' && *(l+21) == ':' && *(l+25) == '+')
                DATE_FORMAT = DATE_FORMAT_FULL_GUID;
            }
          }else
          {
            if (line[4] == '/' && line[7] == '/' && line[13] == ':' && line[16] == ':')
              DATE_FORMAT = DATE_FORMAT_SIMPLE;
            else if (line[2] == '/' && line[5] == '/' && line[13] == ':' && line[16] == ':')
              DATE_FORMAT = DATE_FORMAT_SIMPLE_INVERT;
            else if (line[4] == '-' && line[7] == '-' && line[13] == ':' && line[16] == ':' && line[19] == '+')
              DATE_FORMAT = DATE_FORMAT_FULL;
            else if (line[9] == ':' && line[12] == ':')
              DATE_FORMAT = DATE_FORMAT_UNIX;
          }
        }

        //add first line
        //init
        indx[0]         = 0;
        send_date[0]    = 0;
        source[0]       = 0;
        user[0]         = 0;
        state[0]        = 0;
        description[0]  = 0;

        //working
        switch(DATE_FORMAT)
        {
          case DATE_FORMAT_SIMPLE:
            //date
            strncpy(send_date,line,DATE_SIZE_MAX);
            send_date[DATE_SIZE_MAX-1]  = 0;
            send_date[10]               = '-';

            //description (datas)
            strncpy(description,(line+DATE_SIZE_MAX+1),MAX_LINE_SIZE);
          break;
          case DATE_FORMAT_SIMPLE_INVERT:
            //date
            send_date[0] =line[6]; //Année
            send_date[1] =line[7];
            send_date[2] =line[8];
            send_date[3] =line[9];
            send_date[4] ='/';
            send_date[5] =line[3]; //Mois
            send_date[6] =line[4];
            send_date[7] ='/';
            send_date[8] =line[0]; //Jour
            send_date[9] =line[1];
            send_date[10]='-';
            send_date[11]=line[11]; //Heures
            send_date[12]=line[12];
            send_date[13]=':';
            send_date[14]=line[14]; //Minutes
            send_date[15]=line[15];
            send_date[16]=':';
            send_date[17]=line[17]; //Secondes
            send_date[18]=line[18];
            send_date[19]=0;

            //description (datas)
            strncpy(description,(line+DATE_SIZE_MAX),MAX_LINE_SIZE);
          break;
          case DATE_FORMAT_FULL:
            //date
            strncpy(send_date,line,DATE_SIZE_MAX);
            send_date[DATE_SIZE_MAX-1]  = 0;
            send_date[4]                = '/';
            send_date[7]                = '/';
            send_date[10]               = '-';

            l = line+DATE_SIZE_MAX+5;
            while (*l && *l!='\t')l++;l++;
            while (*l && *l!='\t')l++;l++;

            //description (datas+source)
            snprintf(source,MAX_PATH,"%s",l);
            c = source;
            while(c-source <MAX_PATH && *c && *c!='\t')c++;
            if (*c == '\t')
            {
              strncpy(description,c+1,MAX_PATH);
              *c =0;
            }else
            {
              strncpy(description,source,MAX_PATH);
              source[0] = 0;
            }
          break;
          case DATE_FORMAT_FULL_GUID:
            //date
            l = line;
            while (*l && *l!=' ' && *l!='\t')l++;
            l++;
            strncpy(send_date,l,DATE_SIZE_MAX);
            send_date[DATE_SIZE_MAX-1]  = 0;
            send_date[4]                = '/';
            send_date[7]                = '/';
            send_date[10]               = '-';

            //pass 7 \t
            while (*l && *l!='\t')l++;l++;
            while (*l && *l!='\t')l++;l++;
            while (*l && *l!='\t')l++;l++;
            while (*l && *l!='\t')l++;l++;
            while (*l && *l!='\t')l++;l++;
            while (*l && *l!='\t')l++;l++;
            while (*l && *l!='\t')l++;l++;

            //source
            snprintf(source,MAX_PATH,"%s",l);
            c = source;
            while (*c && *c!=' ' && *c!='\t')c++;
            *c=0;

            //state
            while (*l && *l!=' ' && *l!='\t')l++;l++;
            snprintf(state,MAX_PATH,"%s",l);
            c = state;
            while (*c && *c!=' ' && *c!='\t')c++;
            *c=0;

            //description
            while (*l && *l!=' ' && *l!='\t')l++;l++;
            //description (datas)
            strncpy(description,l,MAX_LINE_SIZE);
          break;
          case DATE_FORMAT_UNIX:
            //date
            send_date[0] ='?'; //Année
            send_date[1] ='?';
            send_date[2] ='?';
            send_date[3] ='?';
            send_date[4] ='/';
            switch(line[0])      //mois
            {
              case 'J':
                if (line[1] == 'a'){send_date[5] = '0';send_date[6] = '1';}        //January
                else if (line[1] == 'u')
                {
                  if (line[2] == 'n'){send_date[5] = '0';send_date[6] = '6';}      //June
                  else if (line[2] == 'l'){send_date[5] = '0';send_date[6] = '7';} //July
                  else {send_date[5] = '_';send_date[6] = '_';}
                }else {send_date[5] = '_';send_date[6] = '_';}
              break;
              case 'F': send_date[5] = '0';send_date[6] = '2';break;                //February
              case 'M':
                if (line[2] == 'r'){send_date[5] = '0';send_date[6] = '3';}        //March
                else if (line[2] == 'y'){send_date[5] = '0';send_date[6] = '5';}   //May
                else {send_date[5] = '_';send_date[6] = '_';}
              break;
              case 'A':
                if (line[1] == 'p'){send_date[5] = '0';send_date[6] = '4';}        //April
                else if (line[1] == 'u'){send_date[5] = '0';send_date[6] = '8';}   //August
                else {send_date[5] = '_';send_date[6] = '_';}
              break;
              case 'S': send_date[5] = '0';send_date[6] = '9';break;                //September
              case 'O': send_date[5] = '1';send_date[6] = '0';break;                //October
              case 'N': send_date[5] = '1';send_date[6] = '1';break;                //November
              case 'D': send_date[5] = '1';send_date[6] = '2';break;                //December
              default : send_date[5] = '_';send_date[6] = '_';break;
            }
            send_date[7] ='/';
            if (line[4] == ' ')send_date[8] ='0';
            else send_date[8] =line[4]; //Jour
            send_date[9] =line[5];
            send_date[10]='-';
            send_date[11]=line[7]; //Heures
            send_date[12]=line[8];
            send_date[13]=line[9];
            send_date[14]=line[10]; //Minutes
            send_date[15]=line[11];
            send_date[16]=line[12];
            send_date[17]=line[13]; //Secondes
            send_date[18]=line[14];
            send_date[19]=0;

            //user/computer
            l = line+16;
            c = user;
            while((c-user < MAX_PATH) && *l && *l!=' ' && *l!='\t')*c++=*l++;
            *c = 0;

            //source
            l++;
            c = source;
            while((c-source < MAX_PATH) && *l && *l!=':')*c++=*l++;
            *c = 0;

            //description (datas)
            description[0] = 0;
            strncpy(description,(l+2),MAX_LINE_SIZE);
          break;
          case DATE_FORMAT_ENCR:
            //date
            strncpy(send_date,line+1,DATE_SIZE_MAX);
            send_date[DATE_SIZE_MAX-1]  = 0;
            send_date[4]                = '/';
            send_date[7]                = '/';
            send_date[10]               = '-';

            //description (datas)
            strncpy(description,(line+DATE_SIZE_MAX+2),MAX_LINE_SIZE);
          break;
          //--------------------------------------
          default:
            //bad file format
            HeapFree(GetProcessHeap(), 0, buffer);
            CloseHandle(Hlog);
            return;
          break;
        }

        //add item
        snprintf(indx,DEFAULT_TMP_SIZE,"%08lu",j++);
        addLogtoDB(eventfile, indx, "", send_date, send_date, source, description, user, "", "", state, "", session_id, db);

        //next lines
        do
        {
          if (unicode_file)
          {
            memset(line_unicode,0,MAX_LINE_DBSIZE);
            l = line_unicode;
            while ((*b!=0 || *(b+1)!=0) && ((l-line_unicode) < MAX_LINE_DBSIZE) && *b != '\r' && *b != '\n')*l++ = *b++;
            if (*b == '\r')b+=4;
            else if (*b == '\n')b+=2;
            *l++=0;
            *l=0;
            snprintf(line,MAX_LINE_SIZE,"%S",line_unicode);
          }else
          {
            memset(line,0,MAX_LINE_SIZE);
            l = line;
            while (*b && (l-line < MAX_LINE_SIZE) && *b != '\r' && *b != '\n')*l++ = *b++;

            if (*b == '\r')b+=2;
            else if (*b == '\n')b++;
            *l=0;
          }

          //init
          indx[0]         = 0;
          send_date[0]    = 0;
          source[0]       = 0;
          user[0]         = 0;
          state[0]        = 0;
          description[0]  = 0;

          //working
          switch(DATE_FORMAT)
          {
            case DATE_FORMAT_SIMPLE:
              //date
              strncpy(send_date,line,DATE_SIZE_MAX);
              send_date[DATE_SIZE_MAX-1]  = 0;
              send_date[10]               = '-';

              //description (datas)
              strncpy(description,(line+DATE_SIZE_MAX+1),MAX_LINE_SIZE);
            break;
            case DATE_FORMAT_SIMPLE_INVERT:
              //date
              send_date[0] =line[6]; //Année
              send_date[1] =line[7];
              send_date[2] =line[8];
              send_date[3] =line[9];
              send_date[4] ='/';
              send_date[5] =line[3]; //Mois
              send_date[6] =line[4];
              send_date[7] ='/';
              send_date[8] =line[0]; //Jour
              send_date[9] =line[1];
              send_date[10]='-';
              send_date[11]=line[11]; //Heures
              send_date[12]=line[12];
              send_date[13]=':';
              send_date[14]=line[14]; //Minutes
              send_date[15]=line[15];
              send_date[16]=':';
              send_date[17]=line[17]; //Secondes
              send_date[18]=line[18];
              send_date[19]=0;

              //description (datas)
              strncpy(description,(line+DATE_SIZE_MAX),MAX_LINE_SIZE);
            break;
            case DATE_FORMAT_FULL:
              //date
              strncpy(send_date,line,DATE_SIZE_MAX);
              send_date[DATE_SIZE_MAX-1]  = 0;
              send_date[4]                = '/';
              send_date[7]                = '/';
              send_date[10]               = '-';

              l = line+DATE_SIZE_MAX+5;
              while (*l && *l!='\t')l++;l++;
              while (*l && *l!='\t')l++;l++;

              //description (datas+source)
              snprintf(source,MAX_PATH,"%s",l);
              c = source;
              while(c-source <MAX_PATH && *c && *c!='\t')c++;
              if (*c == '\t')
              {
                strncpy(description,c+1,MAX_PATH);
                *c =0;
              }else
              {
                strncpy(description,source,MAX_PATH);
                source[0] = 0;
              }
            break;
            case DATE_FORMAT_FULL_GUID:
              //date
              l = line;
              while (*l && *l!=' ' && *l!='\t')l++;
              l++;
              snprintf(send_date,MAX_PATH,"%s",l);
              send_date[DATE_SIZE_MAX-1]  = 0;
              send_date[4]                = '/';
              send_date[7]                = '/';
              send_date[10]               = '-';

              //pass 7 \t
              while (*l && *l!='\t')l++;l++;
              while (*l && *l!='\t')l++;l++;
              while (*l && *l!='\t')l++;l++;
              while (*l && *l!='\t')l++;l++;
              while (*l && *l!='\t')l++;l++;
              while (*l && *l!='\t')l++;l++;
              while (*l && *l!='\t')l++;l++;

              //source
              snprintf(source,MAX_PATH,"%s",l);
              c = source;
              while (*c && *c!=' ' && *c!='\t')c++;
              *c=0;

              //state
              while (*l && *l!=' ' && *l!='\t')l++;l++;
              snprintf(state,MAX_PATH,"%s",l);
              c = state;
              while (*c && *c!=' ' && *c!='\t')c++;
              *c=0;

              //description
              while (*l && *l!=' ' && *l!='\t')l++;l++;
              //description (datas)
              snprintf(description,MAX_LINE_SIZE,"%s",l);
            break;
            case DATE_FORMAT_UNIX:
              //date
              send_date[0] ='?'; //Année
              send_date[1] ='?';
              send_date[2] ='?';
              send_date[3] ='?';
              send_date[4] ='/';
              switch(line[0])      //mois
              {
                case 'J':
                  if (line[1] == 'a'){send_date[5] = '0';send_date[6] = '1';}        //January
                  else if (line[1] == 'u')
                  {
                    if (line[2] == 'n'){send_date[5] = '0';send_date[6] = '6';}      //June
                    else if (line[2] == 'l'){send_date[5] = '0';send_date[6] = '7';} //July
                    else {send_date[5] = '_';send_date[6] = '_';}
                  }else {send_date[5] = '_';send_date[6] = '_';}
                break;
                case 'F': send_date[5] = '0';send_date[6] = '2';break;                //February
                case 'M':
                  if (line[2] == 'r'){send_date[5] = '0';send_date[6] = '3';}        //March
                  else if (line[2] == 'y'){send_date[5] = '0';send_date[6] = '5';}   //May
                  else {send_date[5] = '_';send_date[6] = '_';}
                break;
                case 'A':
                  if (line[1] == 'p'){send_date[5] = '0';send_date[6] = '4';}        //April
                  else if (line[1] == 'u'){send_date[5] = '0';send_date[6] = '8';}   //August
                  else {send_date[5] = '_';send_date[6] = '_';}
                break;
                case 'S': send_date[5] = '0';send_date[6] = '9';break;                //September
                case 'O': send_date[5] = '1';send_date[6] = '0';break;                //October
                case 'N': send_date[5] = '1';send_date[6] = '1';break;                //November
                case 'D': send_date[5] = '1';send_date[6] = '2';break;                //December
                default : send_date[5] = '_';send_date[6] = '_';break;
              }
              send_date[7] ='/';
              if (line[4] == ' ')send_date[8] ='0';
              else send_date[8] =line[4]; //Jour
              send_date[9] =line[5];
              send_date[10]='-';
              send_date[11]=line[7]; //Heures
              send_date[12]=line[8];
              send_date[13]=line[9];
              send_date[14]=line[10]; //Minutes
              send_date[15]=line[11];
              send_date[16]=line[12];
              send_date[17]=line[13]; //Secondes
              send_date[18]=line[14];
              send_date[19]=0;

              //user/computer
              l = line+16;
              c = user;
              while((c-user < MAX_PATH) && *l && *l!=' ' && *l!='\t')*c++=*l++;
              *c = 0;

              //source
              l++;
              c = source;
              while((c-source < MAX_PATH) && *l && *l!=':')*c++=*l++;
              *c = 0;

              //description (datas)
              description[0] = 0;
              strncpy(description,(l+2),MAX_LINE_SIZE);
            break;
            case DATE_FORMAT_ENCR:
              //date
              strncpy(send_date,line+1,DATE_SIZE_MAX);
              send_date[DATE_SIZE_MAX-1]  = 0;
              send_date[4]                = '/';
              send_date[7]                = '/';
              send_date[10]               = '-';

              //description (datas)
              strncpy(description,(line+DATE_SIZE_MAX+2),MAX_LINE_SIZE);
            break;
            //--------------------------------------
            default:
              //bad file format
              HeapFree(GetProcessHeap(), 0, buffer);
              CloseHandle(Hlog);
              return;
            break;
          }

          //add item
          snprintf(indx,DEFAULT_TMP_SIZE,"%08lu",j++);
          addLogtoDB(eventfile, indx, "", send_date, send_date, source, description, user, "", "", state, "", session_id, db);
        }while(*b);
        HeapFree(GetProcessHeap(), 0, buffer);
      }
    }
  }
  CloseHandle(Hlog);
}
