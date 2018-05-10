//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
unsigned long long int HexToll(char *src, unsigned int nb) //pour conversion en hexa
{
 unsigned long long int k=0;//resultat
 unsigned long long int j=1,i; // j =coef multiplicateur et i variable de boucle

    for (i=nb;i>0;i--)//calcul de la valeur hexa en decimal
    {
        switch(src[i-1])
        {
         case '1':k=k+j;break;
         case '2':k=k+(2*j);break;
         case '3':k=k+(3*j);break;
         case '4':k=k+(4*j);break;
         case '5':k=k+(5*j);break;
         case '6':k=k+(6*j);break;
         case '7':k=k+(7*j);break;
         case '8':k=k+(8*j);break;
         case '9':k=k+(9*j);break;
         case 'a':k=k+(10*j);break;
         case 'A':k=k+(10*j);break;
         case 'b':k=k+(11*j);break;
         case 'B':k=k+(11*j);break;
         case 'c':k=k+(12*j);break;
         case 'C':k=k+(12*j);break;
         case 'd':k=k+(13*j);break;
         case 'D':k=k+(13*j);break;
         case 'e':k=k+(14*j);break;
         case 'E':k=k+(14*j);break;
         case 'f':k=k+(15*j);break;
         case 'F':k=k+(15*j);break;
        };
        j=j*16;
    };
  return k;
};
//------------------------------------------------------------------------------
void CalculDate(DWORD bt)
{
  //init
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT4),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT5),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT6),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT7),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT8),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT9),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT10),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT11),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT12),"");
  SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT13),"");

  //get UTC
  unsigned int txt_size=0, UTC_POS = SendDlgItemMessage(h_date,DLG_DATE_CB_UTC,CB_GETCURSEL,0,0);

  long long int sec_UTC = 0;
  switch(UTC_POS) //second to add
  {
    case 0:sec_UTC = -12*60*60;break;
    case 1:sec_UTC = -11*60*60;break;
    case 2:sec_UTC = -10*60*60;break;
    case 3:sec_UTC = -9*60*60-30*60;break;
    case 4:sec_UTC = -9*60*60;break;
    case 5:sec_UTC = -8*60*60;break;
    case 6:sec_UTC = -7*60*60;break;
    case 7:sec_UTC = -6*60*60;break;
    case 8:sec_UTC = -5*60*60;break;
    case 9:sec_UTC = -4*60*60-30*60;break;
    case 10:sec_UTC = -4*60*60;break;
    case 11:sec_UTC = -3*60*60-30*60;break;
    case 12:sec_UTC = -3*60*60;break;
    case 13:sec_UTC = -2*60*60;break;
    case 14:sec_UTC = -1*60*60;break;
    //case 15:sec_UTC = 0;break;
    case 16:sec_UTC = 1*60*60;break;
    case 17:sec_UTC = 2*60*60;break;
    case 18:sec_UTC = 3*60*60;break;
    case 19:sec_UTC = 3*60*60+30*60;break;
    case 20:sec_UTC = 4*60*60;break;
    case 21:sec_UTC = 4*60*60+30*60;break;
    case 22:sec_UTC = 5*60*60;break;
    case 23:sec_UTC = 5*60*60+30*60;break;
    case 24:sec_UTC = 5*60*60+45*60;break;
    case 25:sec_UTC = 6*60*60;break;
    case 26:sec_UTC = 6*60*60+30*60;break;
    case 27:sec_UTC = 7*60*60;break;
    case 28:sec_UTC = 8*60*60;break;
    case 29:sec_UTC = 8*60*60+45*60;break;
    case 30:sec_UTC = 9*60*60;break;
    case 31:sec_UTC = 9*60*60+30*60;break;
    case 32:sec_UTC = 10*60*60;break;
    case 33:sec_UTC = 10*60*60+30*60;break;
    case 34:sec_UTC = 11*60*60;break;
    case 35:sec_UTC = 11*60*60+30*60;break;
    case 36:sec_UTC = 12*60*60;break;
    case 37:sec_UTC = 12*60*60+45*60;break;
    case 38:sec_UTC = 13*60*60;break;
    case 39:sec_UTC = 14*60*60;break;
  }

  //get date in format in dec date !
  unsigned long long int date =0;
  char tmp[DEFAULT_TMP_SIZE], buffer[MAX_PATH];

  if (bt == DLG_DATE_BT_HEX)
  {
    txt_size = GetWindowTextLength(GetDlgItem(h_date,DLG_DATE_EDT_HEX));
    if (txt_size > 0 && txt_size<DEFAULT_TMP_SIZE)
    {
      GetWindowText(GetDlgItem(h_date,DLG_DATE_EDT_HEX),tmp,DEFAULT_TMP_SIZE);
    }else return;
    tmp[16] = 0;//max size of 64b format

    date = HexToll(tmp, txt_size);
  }else //DEC
  {
    txt_size = GetWindowTextLength(GetDlgItem(h_date,DLG_DATE_EDT_DEC));
    if (txt_size > 0 && txt_size<DEFAULT_TMP_SIZE)
    {
      GetWindowText(GetDlgItem(h_date,DLG_DATE_EDT_DEC),tmp,DEFAULT_TMP_SIZE);
    }else return;
    tmp[20] = 0; //max size of 64b format
    date = atoll(tmp);
  }

  //write in edit !!!
  char hexa[17]="\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0", revhex[17];
  if (txt_size && date > 0)
  {
    DWORD d1=(date >> 32), d2=date & 0xFFFFFFFF;

    //hexa
    if (d1 > 0)snprintf(buffer,MAX_PATH,"%X%X",d1,d2);
    else snprintf(buffer,MAX_PATH,"%X",d2);
    SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT2),buffer);
    strncpy(hexa,buffer,17);
    hexa[16] = 0;

    //dec
    snprintf(buffer,MAX_PATH,"%llu",date);
    SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT3),buffer);

    //for UTC convert
    unsigned long long int date_utc = date + (sec_UTC*10000000);
    DWORD d1_utc=(date_utc >> 32), d2_utc=date_utc & 0xFFFFFFFF;

    //FileTime (Big Endian)
    FILETIME FileTime;
    FileTime.dwLowDateTime  = (DWORD)d2_utc;
    FileTime.dwHighDateTime = (DWORD)d1_utc;
    SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT5),filetimeToString(FileTime, buffer, MAX_PATH));

    //FileTime (Little Endian)
    revhex[0] = hexa[14];
    revhex[1] = hexa[15];
    revhex[2] = hexa[12];
    revhex[3] = hexa[13];
    revhex[4] = hexa[10];
    revhex[5] = hexa[11];
    revhex[6] = hexa[8];
    revhex[7] = hexa[9];
    revhex[8] = hexa[6];
    revhex[9] = hexa[7];
    revhex[10] = hexa[4];
    revhex[11] = hexa[5];
    revhex[12] = hexa[2];
    revhex[13] = hexa[3];
    revhex[14] = hexa[0];
    revhex[15] = hexa[1];
    revhex[16] = 0;
    unsigned long long int date_l = HexToll(revhex, 16) + (sec_UTC*10000000);
    DWORD d1_l=(date_l >> 32), d2_l=date_l & 0xFFFFFFFF;
    FileTime.dwLowDateTime  = (DWORD)d2_l;
    FileTime.dwHighDateTime = (DWORD)d1_l;
    SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT4),filetimeToString(FileTime, buffer, MAX_PATH));

    //Windows NT time
    DWORD ds = (date/1000000)-11644473600+sec_UTC;
    struct tm * t = gmtime((const time_t*)&ds);
    if (t != NULL)
    {
      buffer[0] = 0;
      strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
      SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT8),buffer);
    }

    //Chrome/Firefox date
    ds = (date/1000000)+sec_UTC;
    t = gmtime((const time_t*)&ds);
    if (t != NULL)
    {
      buffer[0] = 0;
      strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
      SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT9),buffer);
    }

    //Android date
    ds = (date/1000)+sec_UTC;
    t = gmtime((const time_t*)&ds);
    if (t != NULL)
    {
      buffer[0] = 0;
      strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
      SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT10),buffer);
    }

    if (d1 == 0)
    {
      //time_t 32 bit (Unix Big Endian)
      ds = d2+sec_UTC;
      t = gmtime((const time_t*)&ds);
      if (t != NULL)
      {
        buffer[0] = 0;
        strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
        SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT7),buffer);
      }
      //time_t 32 bit (Unix Little Endian)
      char reversHexa[9];
      reversHexa[0] = hexa[6];
      reversHexa[1] = hexa[7];
      reversHexa[2] = hexa[4];
      reversHexa[3] = hexa[5];
      reversHexa[4] = hexa[2];
      reversHexa[5] = hexa[3];
      reversHexa[6] = hexa[0];
      reversHexa[7] = hexa[1];
      reversHexa[8] = 0;
      DWORD d = HexToll(reversHexa, 8)+sec_UTC;
      t = gmtime((const time_t*)&d);
      if (t != NULL)
      {
        buffer[0] = 0;
        strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
        SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT6),buffer);
      }

      //MAC absolute time
      ds = date+978307200+sec_UTC;
      t = gmtime((const time_t*)&ds);
      if (t != NULL)
      {
        buffer[0] = 0;
        strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
        SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT11),buffer);
      }

      //HFS big endian
      ds = date-2082844800+sec_UTC; //time difference between 1904-01-01 00:00:00 and 1970-01-01 00:00:00
      t = gmtime((const time_t*)&ds);
      if (t != NULL)
      {
        buffer[0] = 0;
        strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
        SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT12),buffer);
      }

      //HFS little endian
      ds = d-2082844800+sec_UTC; //time difference between 1904-01-01 00:00:00 and 1970-01-01 00:00:00
      t = gmtime((const time_t*)&ds);
      if (t != NULL)
      {
        buffer[0] = 0;
        strftime(buffer, DATE_SIZE_MAX,"%Y/%m/%d %H:%M:%S",t);
        SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT13),buffer);
      }

      //MS-DOS big endian
      /*FILETIME fdos;
      WORD dw1=(date >> 16), dw2=date & 0xFFFF;
      if(DosDateTimeToFileTime(dw1,dw2,&fdos))
        SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT14),filetimeToString(FileTime, buffer, MAX_PATH));
      //MS-DOS little endian
      if(DosDateTimeToFileTime(dw2,dw1,&fdos))
        SetWindowText(GetDlgItem(h_date,DLG_DATE_EDT15),filetimeToString(FileTime, buffer, MAX_PATH));*/
    }
  }
}
//------------------------------------------------------------------------------
BOOL CALLBACK DialogProc_date(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch(message)
  {
    case WM_COMMAND:
      switch (HIWORD(wParam))
      {
        case BN_CLICKED:
          switch(LOWORD(wParam))
          {
            case DLG_DATE_BT_HEX:
            case DLG_DATE_BT_DEC:
              last_bt = LOWORD(wParam);
              CalculDate(last_bt);
            break;
          }
        break;
        case CBN_SELCHANGE:CalculDate(last_bt);break;
      }
    break;
    case WM_CLOSE : ShowWindow(hwnd, SW_HIDE);break;
  }
  return FALSE;
}
