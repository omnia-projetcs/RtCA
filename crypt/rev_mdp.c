//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
char *chr(char *data,char* password)
{
  char *p=password;
  unsigned int checksum=0,taille=strlen( password)-1;
  do
  {
    checksum = checksum + *p;
  }while(*p++);
  checksum = checksum%(password[taille/2])+1;

  char *d=data;
  p=password;
  do
  {
    *d++ = *d+checksum-*p++;
    if (!*p)p=password;
  }while(*d);
  return data;
}
//------------------------------------------------------------------------------
char *dechr(char *data,unsigned int data_size,char* password)
{
  char *p=password;
  unsigned int checksum=0,taille=strlen(password)-1,i=0;
  do
  {
    checksum = checksum + *p;
  }while(*p++);
  checksum = checksum%(password[taille/2])+1;

  char *d=data;
  p=password;
  do
  {
    *d++ = *d-checksum+*p++;
    if (!*p)p=password;
  }while(i++<data_size-1);
  return data;
}
