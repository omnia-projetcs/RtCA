//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#ifndef _REGISTRY_RTCA_DEFINE_
#define _REGISTRY_RTCA_DEFINE_
//------------------------------------------------------------------------------
#define HBIN_FIRST_DEFAULT    0x1000
//header du fichier
typedef struct regf_header{
  long int id;            // "regf" = 0x66676572
  long int sec_number1;
  long int sec_number2;
  FILETIME timestamp;     //Date de dernière modification
  long int maj_version;
  long int min_version;
  long int file_type;     //0x0000 = normal, 0x0001 = transaction log
  long int format;
  long int sync;
  long int bin_data_size;
  long int control;       //0x0001
  char name[460];          //Name Hive en Unicode début a 0x30 et fin avec 0x0000
  long int sum;
} REGF_HEADER;
//------------------------------------------------------------------------------
//header hbin (taille : 32octets)
#define HBIN_HEADER_SIZE  0x20
typedef struct hbin_header{
  long int id;          // "hbin" = 0x6E696268
  DWORD hive_bin_last_header; //à partir de l'offset 0x1000+header size
  DWORD hive_bin_size;
  long int reserved1;
  long int reserved2;
  FILETIME timestamp;
  long int sum;
}HBIN_HEADER;
//------------------------------------------------------------------------------
//pre header de data
#define HBIN_CELL_PRE_HEADER_SIZE 6
typedef struct hbin_cell_pre_header{
  char size[4];   //normalement 0x??FFFFFF
  short type; /*
              Sub keys list : lf = 0x666C , lh = 0x686C , li = 0x696C , ri = 0x6972
              Named key     : nk = 0x6B6E
              Security key  : sk = 0x6B73
              Value key     : vk = 0x6B76
              Data block key: db = 0x6264
              */
}HBIN_CELL_PRE_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_LF_DATA_SIZE  8 // 4 + 4 de taille
typedef struct hbin_cell_lf_data_header{
  DWORD nk_of;                //emplacement de la clé nk à partir de l'octet de début de la structure hbin
  char name[4];               //les 4premières lettres de la chaine de NK liée
}HBIN_CELL_LF_DATA_HEADER;
#define HBIN_CELL_LF_SIZE  8 // 4 + 4 de taille
typedef struct hbin_cell_lf_header{
  DWORD size;

  short type;                 //0x666C : debut standard de la zone de DATA
  short nb_keys;              //nombre de clé
  HBIN_CELL_LF_DATA_HEADER hb_c[1];
}HBIN_CELL_LF_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_LH_SIZE  8 // 8 + 4 de taille
typedef struct hbin_cell_lh_header{
  DWORD size;

  short type;                 //0x686C : debut standard de la zone de DATA
  short nb_keys;              //nombre de clé
}HBIN_CELL_LH_HEADER;
#define HBIN_CELL_LH_DATA_SIZE  8 // 4 + 4 de taille
typedef struct hbin_cell_lh_data_header{
  DWORD nk_of;                //emplacement de la clé nk à partir de l'octet de début de la structure hbin
  char name[4];               //les 4premières lettres de la chaine de NK liée
}HBIN_CELL_LH_DATA_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_LI_SIZE  12 // 8 + 4 de taille
typedef struct hbin_cell_li_header{
  DWORD size;

  short type;                 //0x696C : debut standard de la zone de DATA
  short nb_keys;              //nombre de clé
  DWORD nk_of;                //emplacement de la clé nk
}HBIN_CELL_LI_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_DB_SIZE  16 // 12 + 4 de taille
typedef struct hbin_cell_db_header{
  DWORD size;

  short type;                 //0x6264 : debut standard de la zone de DATA
  short nb_segments;
  DWORD db_segment_list_offset;
  long padding;

}HBIN_CELL_DB_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_RI_SIZE  12 // 8 + 4 de taille
typedef struct hbin_cell_ri_header{
  DWORD size;

  short type;                 //0x6972 : debut standard de la zone de DATA
  short nb_keys;              //nombre de clé
  DWORD li_of;                //emplacement de la clé li
}HBIN_CELL_RI_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_NK_SIZE  80 // 76 + 4 de taille
typedef struct hbin_cell_nk_header{
  DWORD size;                 // 0xFFFFFFFF - size + le padding (size%8) = la taille

  short type;                 //0x6B6E : debut standard de la zone de DATA
  short format;               //root key = 0x2C, sinon 0x20
  FILETIME last_write;        //dernière modification
  long int unknow1;
  DWORD parent_key;           //emplacement de la clé parent
  DWORD nb_subkeys;           //nombre de sous clé
  DWORD nb_vl_subkeys;        //nombre de sous clé volatile
  DWORD lf_offset;            //-1 ou 0xffffffff = vide

  DWORD lsk_vol_offset;       //-1 ou 0xffffffff = vide
                              //emplacement de la liste de sk volatile à partir du début du hbin_data
  DWORD nb_values;
  DWORD val_ls_offset;
  DWORD sk_offset;            //-1 ou 0xffffffff = vide //emplacement de la liste de sk à partir du début du hbin
  DWORD class_name_offset;    //classe name offset -1 ou 0xffffffff = vide
  DWORD sbk_name_max_size;
  DWORD sbk_name_cl_max_size;
  DWORD value_name_max_size;
  DWORD value_data_max_size;
  DWORD unknow2;
  short key_name_size;
  short class_name_size;
}HBIN_CELL_NK_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_SK_SIZE  40 // 4 + 20 + 4 + 16 de taille
#define HBIN_CELL_SK_HSIZE 24 // 4 + 20
typedef struct hbin_cell_sk_header{
  DWORD size;

  short type;                 //0x6B73 : debut standard de la zone de DATA
  short unknow;
  DWORD last_sk;
  DWORD next_sk;
  DWORD ref_count;
  DWORD sk_size;
                              //... NT security descriptor
  char  revision;
  char  unknow_1;
  short control;

  DWORD owner_offset;         //not sur ... ???
  DWORD group_offset;
  DWORD SACL_offset;
  DWORD DACL_offset;
}HBIN_CELL_SK_HEADER;

#define SK_OWNER_SIZE_MAX   52
#define SK_HEADER_DATA_SIZE 24
typedef struct sk_header_data
{
  DWORD unknow_0;
  DWORD unknow_1;
  DWORD unknow_2;
  DWORD unknow_3;
  DWORD unknow_4;
  DWORD unknow_5;
  DWORD unknow_6;
}SK_HEADER_DATA;
typedef struct sk_sid
{
  char header0;           //0x01
  unsigned char nb_ID;    //0x05 ou 0x02
  char unknow[5];         // 0x0000000000
  unsigned char ID0;      // 0x05
  DWORD ID[5];            // SID
}SK_SID;

//------------------------------------------------------------------------------
#define HBIN_CELL_VK_SIZE  24 // 20 + 4 de taille
#define HBIN_CELL_VK_DATA_PADDING_SIZE  4
typedef struct hbin_cell_vk_header{
  DWORD size;

  short type;                 //0x6B76 : debut standard de la zone de DATA
  unsigned short name_size;
  unsigned short data_size;
  //unsigned short unknow;
  union
  {
    DWORD data_offset;
    char cdata_offset[4];
  };

  DWORD data_type;
  short flag;
  short padding;
  char value[1];
  //... Value Name
}HBIN_CELL_VK_HEADER;
//------------------------------------------------------------------------------
#define HBIN_CELL_ITEM_LS 4
typedef struct s_item_ls
{
  DWORD val_of;
}S_ITEM_LS;
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
typedef struct
{
  char file[MAX_PATH];
  char *buffer;       //all data of file
  DWORD taille_fic;   //size of file

  DWORD position;     //init position in the file
  DWORD pos_fhbin;    //first hbin struct position
}HK_F_OPEN;
//------------------------------------------------------------------------------
#endif
