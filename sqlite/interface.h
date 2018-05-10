//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#define TYPE_SQLITE_FLAG_LANG_INIT            0x0001
#define TYPE_SQLITE_FLAG_TESTS_INIT           0x0002
#define TYPE_SQLITE_FLAG_SESSIONS_INIT        0x0004
#define TYPE_SQLITE_FLAG_LANG_INIT_STRINGS    0x0006
#define TYPE_SQLITE_FLAG_ANDROID_INIT_STRINGS 0x0007
#define TYPE_SQLITE_FLAG_CHROME_INIT_STRINGS  0x0008
#define TYPE_SQLITE_FLAG_FIREFOX_INIT_STRINGS 0x0009

#define TYPE_SQLITE_FLAG_VIEW_CHANGE          0x0010
#define TYPE_SQLITE_FLAG_LANGUAGE_CHANGE      0x0011
#define TYPE_SQLITE_FLAG_LANGUAGE_COL_CHANGE  0x0012

#define TYPE_SQLITE_FLAG_COLUMN_COUNT_INIT    0x0020
#define TYPE_SQLITE_FLAG_GET_COLUM_COUNT      0x0021
#define TYPE_SQLITE_FLAG_GET_ITEMS_INFO       0x0122
#define TYPE_SQLITE_FLAG_GET_LOG_STATE_COUNT  0x0123

#define TYPE_SQLITE_FLAG_LOAD_ROOTKIT_DB      0x0124

//test type
#define TYPE_SQLITE_TEST                      0x1000
#define TYPE_SQLITE_TEST_DO                   0x1001

//remove
#define TYPE_SQL_REMOVE_SESSION               0x2000
#define TYPE_SQL_REMOVE_ALL_SESSION           0x2001

//add
#define TYPE_SQL_ADD_SESSION                  0x3000

//export
#define TYPE_SQL_EXPORT_HEAD                  0x4000
#define TYPE_SQL_EXPORT_DATAS                 0x4001

//------------------------------------------------------------------------------
//components type for language
#define TYPE_ST                               0x0000  //00 : static+grp+button
#define TYPE_MENU                             0x0001  //01 : menu
#define TYPE_MENU_TITLE                       0x0002  //02 : menu title
#define TYPE_MENU_SUB_TITLE                   0x0004  //04 : subtitle of menu
//------------------------------------------------------------------------------
/*typedef struct
{
  DWORD type;
}FORMAT_CALBAK_TYPE;*/

typedef struct
{
  DWORD type;
  HANDLE form;
}FORMAT_CALBAK_READ_INFO, FORMAT_CALBAK_TYPE;
