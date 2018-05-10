//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
#define PART_TYPE_NTFS              0x07

//------------------------------------------------------------------------------
#define DEFAULT_SECTOR_SZ           512
#define MAX_CTYPE_SZ                256
typedef struct PartitionInfos_struct
{
  BOOL bootable;
  unsigned char type;
  char ctype[MAX_CTYPE_SZ];

  unsigned int sector_size;
  ULONGLONG PartitionStartOffset;
  ULONGLONG PartitionSize;

  ULONGLONG MFT;
  ULONGLONG MFTMirror;
}PARTITIONINFOS_STRUCT;

#define NB_MAX_PRIMARY_PARTITION    4
typedef struct MBRInfos_struct
{
  unsigned int nb_partition;
  PARTITIONINFOS_STRUCT partitions[NB_MAX_PRIMARY_PARTITION];
}MBRINFOS_STRUCT;
//------------------------------------------------------------------------------
typedef struct CHSAddress
{
  BYTE head;
  BYTE sector:6;
  unsigned short cylinder:10;
}CHSADDRESS;
//------------------------------------------------------------------------------
#define DATA_PART_STRUCT_SZ         0x10    //16
typedef struct data_part_struct
{
  BYTE  BootablePart;       //0x80 = Boot
  BYTE  StartCHSAddress[3]; //8b head + 6b sector + 10b cylinder
  BYTE  PartType;           //only information
  BYTE  EndCHSAddress[3];   //8b head + 6b sector + 10b cylinder
  DWORD StartLBAAddress;
  DWORD Size;
}DATA_PART_STRUCT;
//------------------------------------------------------------------------------
#define MBR_HDR_SZ                  0x200    //512
#define BOOTCODE_SZ                 0x1BE    //446
/*typedef struct mbr_hdr
{
  BYTE BootCode[BOOTCODE_SZ];
  DATA_PART_STRUCT PartTableEntry[NB_MAX_PRIMARY_PARTITION];
  BYTE Signature[2];                //0xAA55 = 0x55 0xAA
}MBR_HDR, PMBR_HDR;*/
//------------------------------------------------------------------------------
#define NTFS_BPB_SZ                 0x200    //512
typedef struct ntfs_bpb
{
  //BYTE start_instruction[3];
  unsigned char HeadSignature[8];   //NTFS
  WORD      ByteBySector;
  BYTE      SectorsByCluster;
  BYTE      _reserved[7];
  BYTE      Descriptor;
  BYTE      _reserved0[2];
  WORD      SectorsByTrack;
  WORD      HeadsNumber;
  DWORD     HiddenSectors;
  BYTE      _reserved1[8];
  BYTE      TotalSectors[8];
  BYTE      MFT[8];
  BYTE      MFTMirror[8];
	DWORD		  ClustersByFileRecord;
	DWORD		  ClustersByIndexBlock;
	BYTE		  VolumeSN[8];

	BYTE		  BootCode[430];
  char      BootSignature[2];            //0xAA55
}NTFS_BPB;
