//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../../RtCA.h"
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
ULONGLONG HexaToll(char *src, unsigned int nb) //pour conversion en hexa
{
 ULONGLONG k=0;//resultat
 DWORD j=1,i;

    for (i=nb;i>0;i--)
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
BOOL CopyFileFromMFT(HANDLE hfile, char *destination)
{
  BOOL ret = FALSE;

  //read the 512 first bytes
  DWORD read = 0;
  #define MIN_BUFFER_READ 4096
  char mbr_buffer[MIN_BUFFER_READ/*MBR_HDR_SZ*/];//4096 sz to read min (512 or 4096)
  if (ReadFile(hfile, mbr_buffer, MIN_BUFFER_READ,&read,0))
  {
    //check if partition or disk
    MBRINFOS_STRUCT infos;
    memset(&infos,0,sizeof(MBRINFOS_STRUCT));
    if (MBRReadInfos(mbr_buffer,read,&infos))
    {
      //get first partition in NTFS info
      unsigned int i, sector_sz = DEFAULT_SECTOR_SZ;
      for (i=0;i<NB_MAX_PRIMARY_PARTITION && i< infos.nb_partition;i++)
      {
        if (infos.partitions[i].type == 0x07) //NTFS
        {
          //pass datas :
          SetFilePointer(hfile,((infos.partitions[i].PartitionStartOffset)*sector_sz),0,FILE_BEGIN);

          memset(mbr_buffer,0,NTFS_BPB_SZ);
          if (ReadFile(hfile, mbr_buffer, NTFS_BPB_SZ,&read,0))
          {
            if (ReadPartInfos(mbr_buffer, NTFS_BPB_SZ, &infos, i))
            {
              sector_sz = infos.partitions[i].sector_size;

              //search file on the partition !!!


              char tmp[MAX_PATH];
              snprintf(tmp,MAX_PATH,"[Part:%d]\nnb_partition:%d\n%s\n%s(0x%02X)\nPartitionStartOffset:%llu\nPartitionSize:%llu\nsector_size:%llu\nMFT:%llu\nMFTex:%llu",i,infos.nb_partition,
                                                                      infos.partitions[i].bootable?"Part0:bootable":"",
                                                                      infos.partitions[i].ctype,(infos.partitions[i].type) & 0xFF,
                                                                      infos.partitions[i].PartitionStartOffset,
                                                                      infos.partitions[i].PartitionSize,
                                                                      infos.partitions[i].sector_size,
                                                                      infos.partitions[i].MFT,
                                                                      infos.partitions[i].MFTMirror);
              MessageBox(NULL,tmp,"test",MB_OK|MB_TOPMOST);
            }
          }
        }else
        {
          //next header
        }
      }

      /*char tmp[MAX_PATH];
      snprintf(tmp,MAX_PATH,"nb_partition:%d\n%s\n%s(0x%02X)",infos.nb_partition,
                                                              infos.partitions[0].bootable?"Part0:bootable":"",
                                                              infos.partitions[0].ctype,(infos.partitions[0].type) & 0xFF);
      MessageBox(NULL,tmp,"test",MB_OK|MB_TOPMOST);*/
    }else
    {
      //restart on the start of the file
      //SetFilePointer(hfile,0,0,FILE_BEGIN);

      //get the informations of the partition



    }
  }


/*
  char testm[MAX_LINE_DBSIZE]="";
  chartohexstring(raw_datas, raw_datas_sz, testm, MAX_LINE_DBSIZE);
  MessageBox(NULL,testm,"0+",MB_OK|MB_TOPMOST);*/

        //search the file on the MFT

        //get file size + allocate memory

        //copy the file
  return ret;
}

