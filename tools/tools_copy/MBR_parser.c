//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../../RtCA.h"
//------------------------------------------------------------------------------
BOOL ReadPartInfos(char *raw_datas, unsigned int raw_datas_sz, MBRINFOS_STRUCT*infos, unsigned int index)
{
  //add size control : raw_datas_sz


  NTFS_BPB *bpb       = (NTFS_BPB *)(raw_datas+3);
  char *BootSignature = (char *)(raw_datas+510);

  //get sector size
  if (bpb->ByteBySector > 0)
  {
    infos->partitions[index].sector_size = bpb->ByteBySector;
  }

  //MFT table
  infos->partitions[index].MFT        = HexaToll(bpb->MFT,8);
  infos->partitions[index].MFTMirror  = HexaToll(bpb->MFTMirror,8);

  //test a supprimer
  //infos->partitions[index].PartitionSize = HexaToll(bpb->TotalSectors,8)/infos->partitions[index].sector_size;

  char tmp[MAX_PATH];
  snprintf(tmp,MAX_PATH,"MFT:%llu (%X)\nMFTMirror:%llu (%X)", infos->partitions[index].MFT,
                                                              HexaToll(bpb->MFT,8),
                                                              infos->partitions[index].MFTMirror,
                                                              HexaToll(bpb->MFTMirror,8));
  MessageBox(NULL,tmp,"test-",MB_OK|MB_TOPMOST);

  //check if OK
  if ((BootSignature[0] & 0xFF) == 0x55 && (BootSignature[1] & 0xFF) == 0xAA)
  {
    //check if a direct partition RAW
    if (infos->partitions[index].ctype == 0 && infos->partitions[index].PartitionSize == 0)
    {
      infos->partitions[index].PartitionStartOffset = 0;

      //part type
      if (bpb->HeadSignature[0] == 'N' && bpb->HeadSignature[1] == 'T' && bpb->HeadSignature[2] == 'F' && bpb->HeadSignature[3] == 'S')
      {
        infos->partitions[index].type = PART_TYPE_NTFS;
        Partition_Type(infos->partitions[index].type, infos->partitions[index].ctype, MAX_CTYPE_SZ);
      }

      //size
      //infos->partitions[index].PartitionSize = HexaToll(bpb->TotalSectors,8)/infos->partitions[index].sector_size;
    }
    return TRUE;
  }

  return FALSE;
}
//------------------------------------------------------------------------------
BOOL MBRReadInfos(char *raw_datas, unsigned int raw_datas_sz, MBRINFOS_STRUCT*infos)
{
  if (raw_datas_sz < MBR_HDR_SZ) return FALSE;

  char *signature = (raw_datas+(BOOTCODE_SZ+DATA_PART_STRUCT_SZ*4));
  if ((signature[0] & 0xFF) != 0x55 || (signature[1] & 0xFF) != 0xAA) return FALSE;

  //get MBR infos ! (not reliable)
  DATA_PART_STRUCT *dps = (DATA_PART_STRUCT *)(raw_datas+BOOTCODE_SZ);
  BOOL ret = FALSE;
  unsigned int i;

  memset(infos,0,sizeof(MBRINFOS_STRUCT));
  for (i=0;i<NB_MAX_PRIMARY_PARTITION;i++)
  {
    //bootable or not ?
    if (dps[i].BootablePart == 0x80)
      infos->partitions[i].bootable = TRUE;

    //type
    if (dps[i].PartType != 0)
    {
      infos->partitions[i].type = dps[i].PartType;
      infos->nb_partition = infos->nb_partition+1;
      Partition_Type(infos->partitions[i].type, infos->partitions[i].ctype, MAX_CTYPE_SZ);
      ret = TRUE;
    }

    //first part by sector + size
    infos->partitions[i].PartitionStartOffset = dps[i].StartLBAAddress;
    infos->partitions[i].PartitionSize = dps[i].Size;
  }
  return ret;
}
//------------------------------------------------------------------------------
char *Partition_Type(unsigned int code, char *ctype, unsigned int ctype_sz_max)
{
  //extra partition types taken from Gary Kessler's MBRParser.pl:
  //http://www.garykessler.net/software/index.html
  // and Wikipedia
  //http://en.wikipedia.org/wiki/Partition_type
  switch(code)
  {
    case 0x00:snprintf(ctype,ctype_sz_max,"Empty");break;
    case 0x01:snprintf(ctype,ctype_sz_max,"FAT12,CHS");break;
    case 0x02:
    case 0x03:snprintf(ctype,ctype_sz_max,"XENIX root,CHS");break;
    case 0x04:snprintf(ctype,ctype_sz_max,"FAT16 16-32MB,CHS");break;
    case 0x05:snprintf(ctype,ctype_sz_max,"Microsoft Extended");break;
    case 0x06:snprintf(ctype,ctype_sz_max,"FAT16 32MB,CHS");break;
    case 0x07:snprintf(ctype,ctype_sz_max,"NTFS");break;
    case 0x08:snprintf(ctype,ctype_sz_max,"AIX BOOT/QNX");break;
    case 0x09:snprintf(ctype,ctype_sz_max,"OS9/QNX");break;
    case 0x0a:snprintf(ctype,ctype_sz_max,"OS2/OPUS");break;
    case 0x0b:snprintf(ctype,ctype_sz_max,"FAT32,CHS");break;
    case 0x0c:snprintf(ctype,ctype_sz_max,"FAT32,LBA");break;

    case 0x0e:snprintf(ctype,ctype_sz_max,"FAT16, 32MB-2GB,LBA");break;
    case 0x0f:snprintf(ctype,ctype_sz_max,"Microsoft Extended, LBA");break;
    case 0x10:snprintf(ctype,ctype_sz_max,"OPUS");break;
    case 0x11:snprintf(ctype,ctype_sz_max,"Hidden FAT12,CHS");break;

    case 0x14:snprintf(ctype,ctype_sz_max,"Hidden FAT16,16-32MB,CHS");break;
    case 0x15:snprintf(ctype,ctype_sz_max,"OS/2 Boot Manager/SWAP");break;
    case 0x16:snprintf(ctype,ctype_sz_max,"Hidden FAT16,32MB-2GB,CHS");break;
    case 0x17:snprintf(ctype,ctype_sz_max,"OS/2 Boot Manager IFS/HPFS/NTFS/exFAT");break;
    case 0x18:snprintf(ctype,ctype_sz_max,"AST SmartSleep Partition");break;
    case 0x19:snprintf(ctype,ctype_sz_max,"Willowtech Photon coS");break;

    case 0x1b:snprintf(ctype,ctype_sz_max,"Hidden FAT32,CHS");break;
    case 0x1c:snprintf(ctype,ctype_sz_max,"Hidden FAT32,LBA");break;

    case 0x1e:snprintf(ctype,ctype_sz_max,"Hidden FAT16,32MB-2GB,LBA");break;
    case 0x1f:snprintf(ctype,ctype_sz_max,"OS/2 Boot Manager");break;
    case 0x20:snprintf(ctype,ctype_sz_max,"Willowtech Photon coS/Windows Mobile update XIP");break;
    case 0x21:snprintf(ctype,ctype_sz_max,"HP Volume Expansion/FSo2");break;
    case 0x22:snprintf(ctype,ctype_sz_max,"Oxygen Extended Partition Table");break;
    case 0x23:snprintf(ctype,ctype_sz_max,"Windows Mobile boot XIP");break;
    case 0x24:snprintf(ctype,ctype_sz_max,"Logical sectored FAT12/FAT16");break;
    case 0x25:snprintf(ctype,ctype_sz_max,"Windows Mobile IMGFS");break;
    case 0x26:snprintf(ctype,ctype_sz_max,"Reserved");break;
    case 0x27:snprintf(ctype,ctype_sz_max,"PQservice");break;

    case 0x2a:snprintf(ctype,ctype_sz_max,"AthFS/FS");break;
    case 0x2b:snprintf(ctype,ctype_sz_max,"SyllableSecure");break;

    case 0x31:snprintf(ctype,ctype_sz_max,"Reserved");break;
    case 0x32:snprintf(ctype,ctype_sz_max,"NOS");break;
    case 0x33:
    case 0x34:snprintf(ctype,ctype_sz_max,"Reserved");break;
    case 0x35:snprintf(ctype,ctype_sz_max,"JFS");break;
    case 0x36:snprintf(ctype,ctype_sz_max,"Reserved");break;

    case 0x38:snprintf(ctype,ctype_sz_max,"THEOS version 3.2, 2GB partition");break;
    case 0x39:snprintf(ctype,ctype_sz_max,"Plan 9 partition");break;
    case 0x3a:snprintf(ctype,ctype_sz_max,"THEOS version 4, 4 GB partition");break;
    case 0x3b:snprintf(ctype,ctype_sz_max,"THEOS version 4 extended partition");break;
    case 0x3c:snprintf(ctype,ctype_sz_max,"PartitionMagic recovery partition");break;
    case 0x3d:snprintf(ctype,ctype_sz_max,"Hidden NetWare");break;

    case 0x3f:snprintf(ctype,ctype_sz_max,"OS/32");break;
    case 0x40:snprintf(ctype,ctype_sz_max,"PICK R83/Venix 80286");break;
    case 0x41:snprintf(ctype,ctype_sz_max,"Personal RISC Boot/Old Linux/Minix/PPC PReP");break;
    case 0x42:snprintf(ctype,ctype_sz_max,"Microsoft MBR,Dynamic Disk");break;
    case 0x43:snprintf(ctype,ctype_sz_max,"Old Linux native (disk shared with DR DOS 6.0) ");break;
    case 0x44:snprintf(ctype,ctype_sz_max,"GoBack partition");break;
    case 0x45:snprintf(ctype,ctype_sz_max,"Priam/Boot-US boot manager,CHS/EUMEL/ELAN");break;
    case 0x46:
    case 0x47:
    case 0x48:snprintf(ctype,ctype_sz_max,"EUMEL/ELAN");break;

    case 0x4a:snprintf(ctype,ctype_sz_max,"Aquila/ALFS/THIN");break;

    case 0x4c:snprintf(ctype,ctype_sz_max,"Aos");break;
    case 0x4d:snprintf(ctype,ctype_sz_max,"Primary QNX POSIX volume on disk");break;
    case 0x4e:snprintf(ctype,ctype_sz_max,"Secondary QNX POSIX volume on disk");break;
    case 0x4f:snprintf(ctype,ctype_sz_max,"Tertiary QNX POSIX volume on disk/ETH Oberon");break;
    case 0x50:snprintf(ctype,ctype_sz_max,"ETH Oberon/Disk Manager 4/Lynx RTOS");break;
    case 0x51:snprintf(ctype,ctype_sz_max,"Novell/Disk Manager 4-6");break;
    case 0x52:snprintf(ctype,ctype_sz_max,"CP/M/System V/AT, V/386");break;
    case 0x53:
    case 0x54:snprintf(ctype,ctype_sz_max,"Disk Manager 6");break;
    case 0x55:snprintf(ctype,ctype_sz_max,"EZ-Drive");break;
    case 0x56:snprintf(ctype,ctype_sz_max,"Logical sectored FAT12/FAT16/EZ-Drive/VFeature");break;
    case 0x57:snprintf(ctype,ctype_sz_max,"DrivePro/Novell VNDI partition");break;

    case 0x5c:snprintf(ctype,ctype_sz_max,"Priam EDisk Partitioned Volume");break;
    case 0x5d:snprintf(ctype,ctype_sz_max,"APTI alternative partition");break;
    case 0x5e:
    case 0x5f:snprintf(ctype,ctype_sz_max,"APTI alternative extended partition");break;

    case 0x61:snprintf(ctype,ctype_sz_max,"SpeedStor");break;
    case 0x63:snprintf(ctype,ctype_sz_max,"Unix System V");break;
    case 0x64:snprintf(ctype,ctype_sz_max,"NetWare File System 286/2/PC-ARMOUR protected partition");break;
    case 0x65:
    case 0x66:snprintf(ctype,ctype_sz_max,"NetWare File System 386");break;
    case 0x67:snprintf(ctype,ctype_sz_max,"NetWare Wolf Mountain");break;
    case 0x68:snprintf(ctype,ctype_sz_max,"NetWare");break;
    case 0x69:snprintf(ctype,ctype_sz_max,"NetWare 5/Novell Storage Services (NSS)");break;

    case 0x70:snprintf(ctype,ctype_sz_max,"DiskSecure multiboot");break;
    case 0x71:snprintf(ctype,ctype_sz_max,"Reserved");break;
    case 0x72:snprintf(ctype,ctype_sz_max,"APTI alternative FAT12,CHS/Unix V7/x86");break;
    case 0x73:snprintf(ctype,ctype_sz_max,"Reserved");break;
    case 0x74:snprintf(ctype,ctype_sz_max,"Scramdisk");break;
    case 0x75:snprintf(ctype,ctype_sz_max,"PC/IX");break;
    case 0x76:snprintf(ctype,ctype_sz_max,"Reserved");break;
    case 0x77:snprintf(ctype,ctype_sz_max,"Novell VNDI, M2FS, M2CS");break;
    case 0x78:snprintf(ctype,ctype_sz_max,"XOSL bootloader filesystem");break;
    case 0x79:snprintf(ctype,ctype_sz_max,"APTI alternative FAT16, CHS");break;
    case 0x7a:snprintf(ctype,ctype_sz_max,"APTI alternative FAT16X,LBA,SFN");break;
    case 0x7b:snprintf(ctype,ctype_sz_max,"APTI alternative FAT16B,CHS,SFN");break;
    case 0x7c:snprintf(ctype,ctype_sz_max,"APTI alternative FAT32X,LBA,SFN");break;
    case 0x7d:snprintf(ctype,ctype_sz_max,"APTI alternative FAT32,CHS,SFN");break;
    case 0x7f:snprintf(ctype,ctype_sz_max,"Alternative OS Development Partition Standard");break;
    case 0x80:snprintf(ctype,ctype_sz_max,"Minix file system");break;
    case 0x81:snprintf(ctype,ctype_sz_max,"Minix file system/Mitac Advanced Disk Manager");break;
    case 0x82:snprintf(ctype,ctype_sz_max,"Solaris x86 or Linux Swap");break;
    case 0x83:snprintf(ctype,ctype_sz_max,"Linux");break;
    case 0x84:snprintf(ctype,ctype_sz_max,"Hibernation");break;
    case 0x85:snprintf(ctype,ctype_sz_max,"Linux Extended");break;
    case 0x86:snprintf(ctype,ctype_sz_max,"NTFS Volume Set");break;
    case 0x87:snprintf(ctype,ctype_sz_max,"NTFS Volume Set");break;
    case 0x88:snprintf(ctype,ctype_sz_max,"Linux plaintext partition table");break;

    case 0x8a:snprintf(ctype,ctype_sz_max,"Linux kernel image");break;
    case 0x8b:snprintf(ctype,ctype_sz_max,"Win NT4 Legacy fault-tolerant FAT32");break;
    case 0x8c:snprintf(ctype,ctype_sz_max,"Win NT4 Legacy fault-tolerant FAT32X");break;
    case 0x8d:snprintf(ctype,ctype_sz_max,"Hidden FAT12");break;
    case 0x8e:snprintf(ctype,ctype_sz_max,"Linux LVM");break;

    case 0x90:snprintf(ctype,ctype_sz_max,"Hidden FAT16");break;
    case 0x91:snprintf(ctype,ctype_sz_max,"Hidden extended partition,CHS");break;
    case 0x92:snprintf(ctype,ctype_sz_max,"Hidden FAT16B");break;
    case 0x93:snprintf(ctype,ctype_sz_max,"Amoeba native filesystem/Hidden Linux");break;
    case 0x94:snprintf(ctype,ctype_sz_max,"Amoeba bad block table");break;
    case 0x95:snprintf(ctype,ctype_sz_max,"EXOPC native");break;
    case 0x96:snprintf(ctype,ctype_sz_max,"ISO-9660 filesystem");break;
    case 0x97:snprintf(ctype,ctype_sz_max,"Hidden FAT32");break;
    case 0x98:snprintf(ctype,ctype_sz_max,"Hidden FAT32X/service partition");break;
    case 0x99:snprintf(ctype,ctype_sz_max,"EISA SCSI/Unix");break;
    case 0x9a:snprintf(ctype,ctype_sz_max,"Hidden FAT16X");break;
    case 0x9b:snprintf(ctype,ctype_sz_max,"Hidden extended,LBA");break;

    case 0x9e:snprintf(ctype,ctype_sz_max,"VSTA/ForthOS");break;
    case 0x9f:snprintf(ctype,ctype_sz_max,"BSD/OS");break;
    case 0xa0:snprintf(ctype,ctype_sz_max,"Hibernation/Diagnostic partition for HP laptops");break;
    case 0xa1:snprintf(ctype,ctype_sz_max,"Hibernation/HP Volume Expansion");break;

    case 0xa3:
    case 0xa4:snprintf(ctype,ctype_sz_max,"HP Volume Expansion");break;
    case 0xa5:snprintf(ctype,ctype_sz_max,"FreeBSD");break;
    case 0xa6:snprintf(ctype,ctype_sz_max,"OpenBSD/HP Volume Expansion");break;
    case 0xa7:snprintf(ctype,ctype_sz_max,"HP Volume Expansion");break;
    case 0xa8:snprintf(ctype,ctype_sz_max,"Mac OSX");break;
    case 0xa9:snprintf(ctype,ctype_sz_max,"NetBSD");break;
    case 0xaa:snprintf(ctype,ctype_sz_max,"Olivetti MS-DOS FAT12");break;
    case 0xab:snprintf(ctype,ctype_sz_max,"Mac OSX Boot/GO!");break;

    case 0xad:snprintf(ctype,ctype_sz_max,"ADFS/FileCore format");break;
    case 0xae:snprintf(ctype,ctype_sz_max,"ShagOS file system");break;
    case 0xaf:snprintf(ctype,ctype_sz_max,"MacOS X HFS/ShagOS swap");break;
    case 0xb0:snprintf(ctype,ctype_sz_max,"Boot-Star dummy partition");break;
    case 0xb1:snprintf(ctype,ctype_sz_max,"HP Volume Expansion/QNX Neutrino power-safe file system");break;
    case 0xb2:snprintf(ctype,ctype_sz_max,"QNX Neutrino power-safe file system");break;
    case 0xb3:snprintf(ctype,ctype_sz_max,"HP Volume Expansion/QNX Neutrino power-safe file system");break;
    case 0xb4:snprintf(ctype,ctype_sz_max,"HP Volume Expansion");break;

    case 0xb6:snprintf(ctype,ctype_sz_max,"HP Volume Expansion/Corrupted fault-tolerant FAT16B mirrored master volume");break;
    case 0xb7:snprintf(ctype,ctype_sz_max,"BSDI/Corrupted fault-tolerant HPFS/NTFS mirrored master volume");break;
    case 0xb8:snprintf(ctype,ctype_sz_max,"BSDI Swap");break;

    case 0xbb:snprintf(ctype,ctype_sz_max,"Boot Wizard hidden/Corrupted fault-tolerant FAT32 mirrored master volume");break;
    case 0xbc:snprintf(ctype,ctype_sz_max,"Backup Capsule/Acronis Secure Zone/Corrupted fault-tolerant FAT32X mirrored master volume");break;
    case 0xbd:snprintf(ctype,ctype_sz_max,"BonnyDOS/286");break;
    case 0xbe:snprintf(ctype,ctype_sz_max,"Solaris 8 boot");break;
    case 0xbf:snprintf(ctype,ctype_sz_max,"Solaris x86");break;
    case 0xc0:snprintf(ctype,ctype_sz_max,"Secured FAT partition/NTFT");break;
    case 0xc1:snprintf(ctype,ctype_sz_max,"Secured FAT12");break;
    case 0xc2:snprintf(ctype,ctype_sz_max,"Hidden Linux native filesystem");break;
    case 0xc3:snprintf(ctype,ctype_sz_max,"Hidden Linux swap");break;
    case 0xc4:snprintf(ctype,ctype_sz_max,"Secured FAT16");break;
    case 0xc5:snprintf(ctype,ctype_sz_max,"Secured extended partition,CHS");break;
    case 0xc6:snprintf(ctype,ctype_sz_max,"Secured FAT16B/Corrupted fault-tolerant FAT16B mirrored slave volume");break;
    case 0xc7:snprintf(ctype,ctype_sz_max,"Syrinx boot/Corrupted fault-tolerant HPFS/NTFS mirrored slave volume");break;
    case 0xc8:
    case 0xc9:
    case 0xca:snprintf(ctype,ctype_sz_max,"DR-DOS");break;
    case 0xcb:snprintf(ctype,ctype_sz_max,"Secured FAT32/Corrupted fault-tolerant FAT32 mirrored slave volume");break;
    case 0xcc:snprintf(ctype,ctype_sz_max,"Secured FAT32X/Corrupted fault-tolerant FAT32X mirrored slave volume");break;
    case 0xcd:snprintf(ctype,ctype_sz_max,"Memory dump");break;
    case 0xce:snprintf(ctype,ctype_sz_max,"Secured FAT16X");break;
    case 0xcf:snprintf(ctype,ctype_sz_max,"Secured extended partition,LBA");break;
    case 0xd0:snprintf(ctype,ctype_sz_max,"Secured FAT");break;
    case 0xd1:snprintf(ctype,ctype_sz_max,"Secured FAT12");break;

    case 0xd4:snprintf(ctype,ctype_sz_max,"Secured FAT16");break;
    case 0xd5:snprintf(ctype,ctype_sz_max,"Secured extended partition,CHS");break;
    case 0xd6:snprintf(ctype,ctype_sz_max,"Secured FAT16B");break;

    case 0xd8:snprintf(ctype,ctype_sz_max,"CP/M-86");break;
    case 0xda:snprintf(ctype,ctype_sz_max,"Shielded disk");break;
    case 0xdb:snprintf(ctype,ctype_sz_max,"FAT32 system restore partition/CP/M-86/CTOS/boot image for x86 supervisor CPU");break;

    case 0xdd:snprintf(ctype,ctype_sz_max,"Hidden memory dump");break;
    case 0xde:snprintf(ctype,ctype_sz_max,"Dell PowerEdge Server utilities (FAT fs)");break;
    case 0xdf:snprintf(ctype,ctype_sz_max,"DG/UX virtual disk manager partition/EMBRM");break;
    case 0xe0:snprintf(ctype,ctype_sz_max,"ST AVFS");break;
    case 0xe1:snprintf(ctype,ctype_sz_max,"Extended FAT12");break;
    case 0xe2:snprintf(ctype,ctype_sz_max,"DOS read-only XFDISK");break;
    case 0xe3:snprintf(ctype,ctype_sz_max,"DOS read-only");break;
    case 0xe4:snprintf(ctype,ctype_sz_max,"Extended FAT16");break;
    case 0xe5:snprintf(ctype,ctype_sz_max,"Logical sectored FAT12/FAT16");break;
    case 0xe6:snprintf(ctype,ctype_sz_max,"SpeedStor");break;

    case 0xe8:snprintf(ctype,ctype_sz_max,"Linux Unified Key Setup");break;

    case 0xeb:snprintf(ctype,ctype_sz_max,"BeOS BFS");break;
    case 0xec:snprintf(ctype,ctype_sz_max,"SkyFS");break;
    case 0xed:snprintf(ctype,ctype_sz_max,"EDC loader/GPT hybrid MBR");break;
    case 0xee:snprintf(ctype,ctype_sz_max,"EFI GPT Disk");break;
    case 0xef:snprintf(ctype,ctype_sz_max,"EFI System Parition");break;
    case 0xf0:snprintf(ctype,ctype_sz_max,"PA-RISC Linux boot loader/OS2 floppy");break;
    case 0xf1:snprintf(ctype,ctype_sz_max,"SpeedStor");break;
    case 0xf2:snprintf(ctype,ctype_sz_max,"Logical sectored FAT12 or FAT16");break;
    case 0xf3:snprintf(ctype,ctype_sz_max,"SpeedStor");break;
    case 0xf4:snprintf(ctype,ctype_sz_max,"SpeedStor DOS partition/single partition for NGF or TwinFS");break;
    case 0xf5:snprintf(ctype,ctype_sz_max,"MD0-MD9 for NGF or TwinFS");break;
    case 0xf6:snprintf(ctype,ctype_sz_max,"SpeedStor");break;
    case 0xf7:snprintf(ctype,ctype_sz_max,"EFAT/Solid State file system");break;

    case 0xf9:snprintf(ctype,ctype_sz_max,"pCache ext2/ext3 persistent cache");break;
    case 0xfa:snprintf(ctype,ctype_sz_max,"x86 emulator");break;
    case 0xfb:snprintf(ctype,ctype_sz_max,"VMware VMFS filesystem partition");break;
    case 0xfc:snprintf(ctype,ctype_sz_max,"VMWare Swap");break;
    case 0xfd:snprintf(ctype,ctype_sz_max,"Linux RAID superblock/FreeDOS");break;
    case 0xfe:snprintf(ctype,ctype_sz_max,"SpeedStor/LANstep/PS/2/Linux LVM/Disk Administration hidden partition");break;
    case 0xff:snprintf(ctype,ctype_sz_max,"XENIX bad block table");break;
    default:snprintf(ctype,ctype_sz_max,"Unknow!(%02X)",code & 0xFF);break;
  }

  return ctype;
}
