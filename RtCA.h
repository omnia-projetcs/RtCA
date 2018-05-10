//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
//******************************************************************************
//#define _WIN64_VERSION_        1       //Enable for 64bit OS Compilation
//#define VISUAL_STUDIO          1       //Enable for visual studio compilation
//debug mode dev test
//#define DEV_DEBUG_MODE         1
//#define CMD_LINE_ONLY_NO_DB    1       // define RtCA on only command line tool (much function with no db)
//******************************************************************************
#define _WIN32_WINNT			     0x0501  //fonctionne au minimum sous Windows 2000
#define _WIN32_IE              0x0501  //fonctionne avec ie5 min pour utilisation de LVS_EX_FULLROWSELECT

#define MY_WM_NOTIFYICON        WM_USER+1 //click on tray icone
#define TTM_SETTITLE	         (WM_USER + 32)
#define TTI_INFO	             1

#define NOM_APPLI              "RtCA"
#define URL_APPLI              "https://github.com/omnia-projetcs/omnia-projetcs"

#define SQLITE_F               100
#define DEFAULT_SQLITE_FILE    "RtCA.sqlite"
#define DEFAULT_TM_SQLITE_FILE "RtCA.sqlite-journal"
#define DEFAULT_TOOL_MENU_FILE "tools.cfg"

#define MAX_FILE_SIZE_HASH      1024*1024*50 // 50mo

#define DEFAULT_INI_FILE       "\\RtCA.ini"

#define IDR_VERSION                 1

#define ICON_APP                  100   //icone de l'application
#define ICON_LANGUAGE_EN          101
#define ICON_LANGUAGE_FR          102

#define ICON_FOLDER               103   //icone treeview
#define ICON_FILE                 104
#define ICON_FILE_BIN             105
#define ICON_FILE_DWORD           106
#define ICON_FILE_TXT             107
#define ICON_FILE_UNKNOW          108

#define MY_ACCEL                  200
//******************************************************************************
#define ICON_DIRECTORY_REG        0x00
#define ICON_FILE_REG             0x01
#define ICON_FILE_BIN_REG         0x02
#define ICON_FILE_DWORD_REG       0x03
#define ICON_FILE_TXT_REG         0x04
#define ICON_FILE_UNKNOW_REG      0x05
//******************************************************************************
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <Winsock2.h>           //gestion socket (avant windows.h car existe la version 1 dans windows.h)
#include <windows.h>
#include <Windowsx.h>
#include "version.h"            //for version tracking
#include <commctrl.h>           //componants
#include <shlobj.h>             //browser
#include <iprtrmib.h>           //tracert PMIB_IPFORWARDTABLE
#include <iphlpapi.h>           //network
#include <lmshare.h>            //share
#include <richedit.h>           //Richedit
#include <tlhelp32.h>           //process
#include <psapi.h>              //process
#include "sqlite/sqlite3.h"     //sqlite
#include "sqlite/interface.h"

#include "Registry.h"           //registry

#include <Windns.h>             //DnsQuery

#include <Wincrypt.h>           //FOR DECODE CHROME/CHROMIUM password
//#include <wintrust.h>           //FOR VERIFY SIGN OF FILES
#include "crypt/d3des.h"        //Crypto
#include <lmaccess.h>           //group account list
#include <wininet.h>            //for VirusTotal + update

#include "files/LiteZip.h"      //zip file for save all locals datas

#include "tools/tools_copy/MFT_parser.h"

#include <Winldap.h>            //LDAP
//---------------------------------------------------------------------------
char SQLITE_LOCAL_BDD[MAX_PATH];
HACCEL hcl;
//---------------------------------------------------------------------------
PVOID OldValue_W64b;            //64bits OS

//for cross compilation bug in 64bit
#ifndef GWL_WNDPROC
  #define GWL_WNDPROC (-4)
#endif
//---------------------------------------------------------------------------
char _SYSKEY[MAX_PATH];
//---------------------------------------------------------------------------
#ifdef VISUAL_STUDIO
  #pragma comment (lib, "comdlg32.lib")
  #pragma comment (lib, "comctl32.lib")
  #pragma comment (lib, "gdi32.lib")
  #pragma comment (lib, "psapi.lib")
  #pragma comment (lib, "dnsapi.lib")
  #pragma comment (lib, "ws2_32.lib")
  #pragma comment (lib, "crypt32.lib")
  #pragma comment (lib, "iphlpapi.lib")
  #pragma comment (lib, "wininet.lib")
  #pragma comment (lib, "Wldap32.lib")
#endif
//---------------------------------------------------------------------------
//size constant
#undef MAX_PATH                       //new value for max_path
#define MAX_PATH                 1024

#define DEFAULT_TMP_SIZE          256
#define MAX_LINE_SIZE      8*MAX_PATH
#define MAX_LINE_DBSIZE    2*MAX_LINE_SIZE
#define REQUEST_MAX_SIZE      1024*64
#define DEFAULT_EVENT_SIZE    1024*64

#define HEIGHT_SEARCH              30
#define DATE_SIZE_MAX              20
#define NUM_S_MAX                  10
#define NB_USERNAME_SIZE           21
#define DRIVE_SIZE                  4
#define COMPUTER_NAME_SIZE_MAX     16
#define IP_SIZE_MAX                16   //255.255.255.255   = 16
#define IPV6_SIZE_MAX              56   //39 + 1 + 15 + 1 => 0000:0000:0000:0000:0000:0000:0000:0000:000.000.000.000
#define MAC_SIZE                   18   //AA:BB:CC:DD:EE:FF = 18
#define NB_MAX_PORTS            65536
#define SZ_PART_SYSKEY           0x21
#define NB_TESTS_GLOBALS           34

#define OMB              10*1024*1024    //10mo
#define DIXM             10*1024*1024    //10mo
#define DBM               2*1024*1024    //2mo

#define WINDOWS_X_MIN_SZ          780
#define WINDOWS_Y_MIN_SZ          540
//------------------------------------------------------------------------------
//debug message
#define NB_MSG_COLUMN               6

#define MSG_INFO               "INFO"
#define MSG_WARN               "WARN"
#define MSG_ERROR             "ERROR"
#define MSG_TEST            "TESTING"
//------------------------------------------------------------------------------
//TOOLS POPUP MENU OPEN
#define NB_MAX_TOOLS            256

#define TOOL_TYPE_OPEN          0   //open file
#define TOOL_TYPE_EDIT          1   //edit file
#define TOOL_TYPE_LCMD          2   //exec local cmd


unsigned int nb_tools;
typedef struct
{
  unsigned int type;
  char title[DEFAULT_TMP_SIZE];
  char cmd[DEFAULT_TMP_SIZE];
  char params[DEFAULT_TMP_SIZE];
}TOOLS_USE;
TOOLS_USE tools_load[NB_MAX_TOOLS];

#define POPUP_MENU_TOOLS_START  25000


//------------------------------------------------------------------------------
//TREVIEW STATE
#define TRV_STATE_CHECK             2
#define TRV_STATE_UNCHECK           1
/*
#define TRV_STATE_CHECK_DISABLE     4
#define TRV_STATE_UNCHECK_DISABLE   3*/
//------------------------------------------------------------------------------
#define DLG_MAIN                 1000
#define LV_INFO                  1001
#define TOOL_BAR                 1002

HWND hCombo_session,hCombo_lang,htoolbar,hstatus_bar,he_search, chk_search, hlstbox,hlstv, htooltip, hlstv_db;
HWND htrv_test, htrv_files, hlstv_process;
HINSTANCE hinst;
HANDLE H_ImagList_icon;

HANDLE h_process, h_sniff, h_reg_file, h_reg, h_date, h_state, h_sqlite_ed, h_hexa, h_proxy, h_info;
BOOL disable_m_context, disable_p_context, enable_magic, enable_remote;

BOOL enable_LNK;
BOOL disable_USB_SHARE;

#define DEFAULT_MODE_SEARCH_HX  0
#define UNICODE_MODE_SEARCH_HX  1
#define HEXA_MODE_SEARCH_HX     2
unsigned int last_search_mode;
unsigned long int last_pos_hex_search;
HANDLE h_Hexa;

#define DLG_CONF                 2000
#define CB_LANG                  2001
#define GRP_CONF                 2002
#define ST_LANG                  2003
#define TRV_TEST                 2004
#define BT_START                 2005
#define BT_EXPORT                2006
#define TRV_FILES                2007
#define CB_SESSION               2009
#define DLG_CONF_SB              2010

#define BT_ACL_FILE_CHK          2012
#define BT_SHA_FILE_CHK          2014
#define BT_ADS_FILE_CHK          2015
#define BT_REGISTRY_RECOV_MODE   2016
#define BT_UTC_CHK               2017
#define BT_MAGIC_CHK             2018
#define BT_RA_CHK                2019
#define EDT_NAME_SESSION         2020
#define ST_NAME_SESSION          2021

#define DLG_VIEW                 3000
#define LV_VIEW                  3001
#define LV_BOX                   3002
#define CB_VIEW                  3003
#define ED_SEARCH                3004
#define BT_SEARCH                3005
#define LV_VIEW_INFO             3006
#define BT_SEARCH_MATCH_CASE     3007
#define BT_SQLITE_FULL_SPEED     3008
#define BT_DISABLE_GRID          3009
#define BT_ENABLE_DATE_NO_UPDATE 3010
#define BT_DISABLE_USB_SHARE_LIST 3011
BOOL DISABLE_GRID_LV_ALL;
BOOL SQLITE_FULL_SPEED;
BOOL enable_DATE_NO_UPDATE;

#define DLG_PROCESS              4000
unsigned int nb_column_process_view;

#define DLG_NETWORK_SNIFF           5000
#define DLG_NS_SNIFF_LB_FILTRE      5001
#define DLG_NS_BT_START             5002
#define DLG_NS_LSTV                 5003
#define DLG_NS_LSTV_FILTRE          5004
#define DLG_NS_LSTV_PAQUETS         5005
#define DLG_NS_BT_SAVE              5006
#define DLG_NS_SNIFF_CHK_PROMISCUOUS 5007
#define DLG_NS_SNIFF_CHK_DISCO      5008

#define POPUP_SNIFF_FILTRE_IP_SRC   5009
#define POPUP_SNIFF_FILTRE_IP_DST   5010
#define POPUP_SNIFF_FILTRE_PORT_SRC 5011
#define POPUP_SNIFF_FILTRE_PORT_DST 5012
#define POPUP_LINK                  5013
#define POPUP_LSTV_SNIFF            5014
#define DLG_CONF_INTERFACE          5015
#define POPUP_TV_REGISTRY           5016
#define POPUP_LSTV_REGISTRY         5017

#define DLG_SNIFF_STATE_PAQUETS_NB_COLUMN  7
#define DLG_SNIFF_STATE_FILTER_NB_COLUMN   7
#define DLG_SNIFF_STATE_IP_NB_COLUMN       6

#define FILTER_IP_SRC               0
#define FILTER_IP_DST               1
#define FILTER_PORT_SRC             2
#define FILTER_PORT_DST             3

BOOL reg_file_start_process;
#define DLG_REGISTRY_EXPLORER_FILE  6000
#define DLG_REG_ED_NTUSER           6001
#define DLG_REG_ED_SAM              6002
#define DLG_REG_ED_SECURITY         6003
#define DLG_REG_ED_SOFTWARE         6004
#define DLG_REG_ED_SYSTEM           6005
#define DLG_REG_ED_OTHER            6006
#define DLG_REG_BT_NTUSER           6010
#define DLG_REG_BT_SAM              6011
#define DLG_REG_BT_SECURITY         6012
#define DLG_REG_BT_SOFTWARE         6013
#define DLG_REG_BT_SYSTEM           6014
#define DLG_REG_BT_OTHER            6015
#define BT_REG_RECOVERY_MODE_CHK    6020
#define BT_REG_START                6021

#define DLG_REGISTRY_EXPLORER_VIEW  6050
#define BT_TREE_VIEW                6051
#define TV_VIEW                     6052
#define STB                         6053

#define DLG_REG_LV_NB_COLUMN          9

//date
#define DLG_DATE                    7000
#define DLG_DATE_EDT_HEX            7001
#define DLG_DATE_EDT_DEC            7002
#define DLG_DATE_BT_HEX             7003
#define DLG_DATE_BT_DEC             7004
#define DLG_DATE_CB_UTC             7005
#define DLG_DATE_EDT2               7007
#define DLG_DATE_EDT3               7008
#define DLG_DATE_EDT4               7009
#define DLG_DATE_EDT5               7010
#define DLG_DATE_EDT6               7011
#define DLG_DATE_EDT7               7012
#define DLG_DATE_EDT8               7013
#define DLG_DATE_EDT9               7014
#define DLG_DATE_EDT10              7015
#define DLG_DATE_EDT11              7016
#define DLG_DATE_EDT12              7017
#define DLG_DATE_EDT13              7018
/*#define DLG_DATE_EDT14              7019
#define DLG_DATE_EDT15              7020*/
DWORD last_bt;

#define DLG_STATE                   8000
#define DLG_STATE_LB_SESSION        8001
#define DLG_STATE_LB_TEST           8002
#define DLG_STATE_BT_ALL            8003
#define DLG_STATE_BT_CRITICAL       8004
#define DLG_STATE_BT_LOG_STATE      8005
#define DLG_STATE_ED_TIME_1         8006
#define DLG_STATE_ED_TIME_2         8007
#define DLG_STATE_BT_FILTER         8008
#define DLG_STATE_LV_ALL            8009
#define DLG_STATE_LV_CRITICAL       8010
#define DLG_STATE_LV_LOG_STATE      8011
#define DLG_STATE_LV_FILTER         8012
#define DLG_STATE_SB                8013
#define DLG_STATE_BT_LOAD           8014

#define DLG_SQLITE_EDITOR           9000
#define DLG_SQL_ED_LB_TABLE         9001
#define DLG_SQL_ED_BT_LOAD          9002
#define DLG_SQL_ED_BT_CLOSE         9003
#define DLG_SQL_ED_ED_REQUEST       9004
#define DLG_SQL_ED_BT_SEND          9005
#define DLG_SQL_ED_LV_RESPONSE      9006
#define DLG_SQL_ED_STATE_SB         9007
#define POPUP_LSTV_SQLITE           9008
#define MY_POPUP_SCREENSHOT         9009
#define MSG_SCREENSHOT              9010
//#define MSG_SCREENSHOT_WINDOW       9011
#define DLG_SQL_ED_BT_MODELS        9012
#define DLG_SQL_ED_BT_INFO          9013
#define DLG_SQL_ED_ED_SEARCH        9014
#define DLG_SQL_ED_BT_SEARCH        9015

#define POPUP_SQLITE_REQUEST        9020

#define POPUP_SQLITE_REQUEST_SELECT             9021
#define POPUP_SQLITE_REQUEST_INSERT             9022
#define POPUP_SQLITE_REQUEST_DELETE             9023
#define POPUP_SQLITE_REQUEST_VACCUM             9024
#define POPUP_SQLITE_REQUEST_JOURNAL_ON         9025
#define POPUP_SQLITE_REQUEST_JOURNAL_OFF        9026
#define POPUP_SQLITE_REQUEST_BEGIN_TRANSACTION  9027
#define POPUP_SQLITE_REQUEST_END_TRANSACTION    9028

#define POPUP_SQLITE_REQUEST_ALTER_TABLE        9029
#define POPUP_SQLITE_REQUEST_CREATE_TABLE       9030
#define POPUP_SQLITE_REQUEST_INSERT_INTO        9031
#define POPUP_SQLITE_REQUEST_DROP_TABLE         9032

#define DLG_SQLITE_ED                           9040
#define DLG_SQLITE_ED_TABLE                     9041

#define POPUP_SQLITE_HDR_RESIZE     9050
#define POPUP_SQLITE_INFOS          9051
#define POPUP_SQLITE_REMOVE         9052
#define POPUP_SQLITE_ED_TABLE       9053

#define DLG_HEXA_READER             9100
#define DLG_HEXA_BT_LOAD            9101
#define DLG_HEXA_BT_CLOSE           9102
#define DLG_HEXA_BT_SEARCH          9103
#define DLG_HEXA_EDT_SEARCH         9104
#define DLG_HEXA_CHK_UNICODE        9105
#define DLG_HEXA_CHK_HEXA           9106
#define DLG_HEXA_LV_INFOS           9107
#define DLG_HEXA_LV_HEXA            9108
#define POPUP_LSTV_HEXA             9109


#define DLG_PROXY                   9200
BOOL use_proxy_advanced_settings;
BOOL use_other_proxy;
char proxy_ch_auth[DEFAULT_TMP_SIZE];
char proxy_ch_user[DEFAULT_TMP_SIZE];
char proxy_ch_password[DEFAULT_TMP_SIZE];

#define PROXY_ST_PROXY              9201
#define PROXY_CHK_USE               9202
#define PROXY_CHK_AUTOCONF          9203
#define PROXY_ST_USER               9204
#define PROXY_ST_PASSWORD           9205
#define PROXY_ED_USER               9206
#define PROXY_ED_PASSWORD           9207
#define PROXY_ED_PROXY              9208
#define PROXY_BT_OK                 9209
#define PROXY_CHK_SAVE              9210

#define DLG_INFO                    9300
#define DLG_INFO_TXT                9301
//------------------------------------------------------------------------------
#define MY_MENU                 10000
#define IDM_NEW_SESSION         10001
#define IDM_OPEN_FILE           10001
#define IDM_OPEN_DIRECTORY      10002
#define IDM_SAVE_ALL            10003
#define IDM_CHECK_ALL_TESTS     10006
#define IDM_DEL_SESSION         10007
#define IDM_DEL_ALL_SESSION     10008
#define IDM_STAY_ON_TOP         10009
#define IDM_DEBUG_MODE          10010
#define IDM_ABOUT               10015
#define IDM_RTCA_HOME           10016
#define IDM_DEBUG_LIST          10017
#define IDM_LOAD_SESSION_FILE   10019
#define IDM_SAVE_SESSION_FILE   10020
#define IDM_TOOLS_CP_HIBERNATE  10021
#define IDM_UNCHECK_ALL_TESTS   10022
#define IDM_QUIT                10023
#define IDM_REFRESH_SESSION     10024
#define IDM_RTCA_UPDATE         10025
#define BT_SREEENSHOT           10026
#define IDM_LOAD_OTHER_BDD      10027
BOOL B_SCREENSHOT;
BOOL B_SCREENSHOT_START;
NOTIFYICONDATA TrayIcon;
HHOOK HHook; // Handle du hook global
#define BT_PROXY                10028

#define IDM_TOOLS_CP_REGISTRY   10100
#define IDM_TOOLS_CP_AUDIT      10101
#define IDM_TOOLS_CP_AD         10102
#define IDM_TOOLS_CP_FILE       10103
#define IDM_TOOLS_PROCESS       10104
#define IDM_TOOLS_REG_EXPLORER  10105
#define IDM_TOOLS_SNIFF         10106
#define IDM_TOOLS_DATE          10107
#define IDM_TOOLS_ANALYSER      10109
#define IDM_TOOLS_SQLITE_ED     10110
#define IDM_TOOLS_GLOBAL_COPY   10111
#define IDM_TOOLS_HEXA_READER   10112

#define IDM_TOOLS_CP_DRIVE_A    10201
#define IDM_TOOLS_CP_DRIVE_B    10202
#define IDM_TOOLS_CP_DRIVE_C    10203
#define IDM_TOOLS_CP_DRIVE_D    10204
#define IDM_TOOLS_CP_DRIVE_E    10205
#define IDM_TOOLS_CP_DRIVE_F    10206
#define IDM_TOOLS_CP_DRIVE_G    10207
#define IDM_TOOLS_CP_DRIVE_H    10208
#define IDM_TOOLS_CP_DRIVE_I    10209
#define IDM_TOOLS_CP_DRIVE_J    10210
#define IDM_TOOLS_CP_DRIVE_K    10211
#define IDM_TOOLS_CP_DRIVE_L    10212
#define IDM_TOOLS_CP_DRIVE_M    10213
#define IDM_TOOLS_CP_DRIVE_N    10214
#define IDM_TOOLS_CP_DRIVE_O    10215
#define IDM_TOOLS_CP_DRIVE_P    10216
#define IDM_TOOLS_CP_DRIVE_Q    10217
#define IDM_TOOLS_CP_DRIVE_R    10218
#define IDM_TOOLS_CP_DRIVE_S    10219
#define IDM_TOOLS_CP_DRIVE_T    10220
#define IDM_TOOLS_CP_DRIVE_U    10221
#define IDM_TOOLS_CP_DRIVE_V    10222
#define IDM_TOOLS_CP_DRIVE_W    10223
#define IDM_TOOLS_CP_DRIVE_X    10224
#define IDM_TOOLS_CP_DRIVE_Y    10225
#define IDM_TOOLS_CP_DRIVE_Z    10226

#define IDM_TOOLS_CP_DISK_0     10227
#define IDM_TOOLS_CP_DISK_1     10228
#define IDM_TOOLS_CP_DISK_2     10229
#define IDM_TOOLS_CP_DISK_3     10230
#define IDM_TOOLS_CP_MBR        10231
//------------------------------------------------------------------------------
#define POPUP_TRV_FILES_REF_ITEMS_STRINGS         0
#define POPUP_TRV_FILES_REF_NB_ITEMS_STRINGS      7
#define POPUP_TRV_FILES_REF_NB_ITEMS_STRINGS_EXT  1
#define POPUP_TRV_FILES                       11000
#define POPUP_TRV_FILES_ADD_FILE              11001
#define POPUP_TRV_FILES_ADD_PATH              11002
#define POPUP_TRV_FILES_REMOVE_ITEMS          11003
#define POPUP_TRV_FILES_CLEAN_ALL             11004
#define POPUP_TRV_FILES_OPEN_PATH             11005
#define POPUP_TRV_FILES_AUTO_SEARCH           11006
#define POPUP_TRV_FILES_SAVE_LIST             11007
#define POPUP_TRV_FILES_UP                    11008
#define POPUP_TRV_FILES_DOWN                  11009
#define POPUP_TRV_FILES_AUTO_SEARCH_PATH      11010
#define POPUP_TRV_FILES_LOAD_LIST             11011
#define POPUP_TRV_FILES_BACKUP                11012
#define POPUP_TRV_FILES_BACKUP_PATH           11013

#define POPUP_TRV_TEST                        11100
#define POPUP_TRV_CHECK_ALL                   11101
#define POPUP_TRV_UNCHECK_ALL                 11102
#define POPUP_TRV_STOP_TEST                   11103

#define POPUP_EXPORT                          12000
#define POPUP_E_CSV                           12001
#define POPUP_E_HTML                          12002
#define POPUP_E_XML                           12003

#define POPUP_SELECT_ALL_SESSION              12100
#define POPUP_SELECT_ALL_TEST                 12101
#define SELECT_ALL_SESSION                    12102
#define SELECT_ALL_TEST                       12103
BOOL SELECT_SESSION, SELECT_TEST;

#define POPUP_LSTV_EMPTY_FILE                 12999
BOOL AVIRUSTTAL,VIRUSTTAL;
HANDLE h_AVIRUSTTAL, h_VIRUSTTAL;

#define NB_VIRUTOTAL_THREADS_REF                  5//8
#define NB_VIRUTOTAL_THREADS                      NB_VIRUTOTAL_THREADS_REF
#define NB_VIRUTOTAL_ERROR_MAX                    5
#define COLUMN_SHA256                            17
#define COLUMN_PATH                               0
#define COLUMN_FILE                               1
#define COLUMN_EXT                                2
#define COLUMN_VIRUSTOTAL           COLUMN_SHA256+1


#define POPUP_LSTV                            13000
#define POPUP_S_VIEW                          13001
#define POPUP_S_SELECTION                     13002
#define POPUP_O_PATH                          13003
#define POPUP_A_SEARCH                        13004
#define POPUP_COPY_TO_CLIPBORD                    8
#define POPUP_CP_LINE                         13005
#define POPUP_TV_CP_COMPLET_PATH              13006
#define POPUP_TV_CP_VALUE_AND_DATA            13007

#define POPUP_I_00                            13010
#define POPUP_I_01                            13011
#define POPUP_I_02                            13012
#define POPUP_I_03                            13013
#define POPUP_I_04                            13014
#define POPUP_I_05                            13015
#define POPUP_I_06                            13016
#define POPUP_I_07                            13017
#define POPUP_I_08                            13018
#define POPUP_I_09                            13019
#define POPUP_I_10                            13020
#define POPUP_I_11                            13021
#define POPUP_I_12                            13022
#define POPUP_I_13                            13023
#define POPUP_I_14                            13024
#define POPUP_I_15                            13025
#define POPUP_I_16                            13026
#define POPUP_I_17                            13027
#define POPUP_I_18                            13028
#define POPUP_I_19                            13029

#define POPUP_FILE_IMPORT_FILE                13036
#define POPUP_FILE_REMOVE_ITEM                13037
#define POPUP_FILE_VIRUSTOTAL                 13038
#define POPUP_FILE_VIRUSTOTAL_ALL             13039

#define POPUP_CP_LINE_HEXA                    13040
#define POPUP_CP_LINE_STRING                  13041

#define NB_POPUP_I                            20

#define POPUP_OPEN_PATH                       13100
#define POPUP_OPEN_REG_PATH                   13101
#define POPUP_OPEN_FILE_PATH                  13102

#define POPUP_LSTV_HDR                        13110
#define POPUP_H_00                            13110
#define POPUP_H_01                            13111
#define POPUP_H_02                            13112
#define POPUP_H_03                            13113
#define POPUP_H_04                            13114
#define POPUP_H_05                            13115
#define POPUP_H_06                            13116
#define POPUP_H_07                            13117
#define POPUP_H_08                            13118
#define POPUP_H_09                            13119
#define POPUP_H_10                            13120
#define POPUP_H_11                            13121
#define POPUP_H_12                            13122
#define POPUP_H_13                            13123
#define POPUP_H_14                            13124
#define POPUP_H_15                            13125
#define POPUP_H_16                            13126
#define POPUP_H_17                            13127
#define POPUP_H_18                            13128
#define POPUP_H_19                            13129

#define POPUP_LSTV_PROCESS                    13200
#define POPUP_KILL_PROCESS                    13201
#define POPUP_DUMP_MEMORY                     13202
#define POPUP_ADD_DLL_INJECT_REMOTE_THREAD    13203
#define POPUP_REM_DLL_INJECT_REMOTE_THREAD    13204
#define POPUP_PROCESS_REFRESH                 13205
#define POPUP_VIRUSTOTAL_CHECK                13206
#define POPUP_VIRUSTOTAL_CHECK_ALL            13207

#define POPUP_ADD_ROOTKIT_FILE                14000
#define POPUP_CHECK_ROOTKIT_FILE              14001
#define POPUP_CHECK_ALL_ROOTKIT_FILE          14002
#define POPUP_ADD_ROOTKIT_PROCESS             14003
#define POPUP_CHECK_ROOTKIT_PROCESS           14004
#define POPUP_CHECK_ALL_ROOTKIT_PROCESS       14005

#define POPUP_DLL_INJECT                         14
#define POPUP_PROCESS_COPY_TO_CLIPBORD           17

#define NB_PROCESS_COLUMN                        19

//------------------------------------------------------------------------------
char NOM_FULL_APPLI[DEFAULT_TMP_SIZE];
//------------------------------------------------------------------------------
//save type
#define SAVE_TYPE_CSV                             1
#define SAVE_TYPE_XML                             2
#define SAVE_TYPE_HTML                            3
#define SAVE_TYPE_PWDUMP                          4
#define SAVE_TYPE_PCAP                            4
#define SAVE_TYPE_REG5                            4

//sqlite only
#define SAVE_TYPE_TXT                             4
#define SAVE_TYPE_PWDUMP_                         5

unsigned int stat_export_column;
HANDLE MyhFile_export,h_Export;
unsigned int export_type;
unsigned int current_lang_id;
DWORD line;
BOOL ExportStart;
char current_test_export_path[MAX_PATH];
//------------------------------------------------------------------------------
//general config
HWND h_main, h_conf;

//tests
#define NB_MAX_TESTS                            256
#define NB_MAX_ITEMS_HEADERS_XML                256

unsigned int NB_TESTS, nb_current_test;
HTREEITEM H_tests[NB_MAX_TESTS];          //list of tests
HANDLE h_thread_test[NB_MAX_TESTS];       //threads for tests
typedef struct
{
  char s[DEFAULT_TMP_SIZE];
}FORMAT_TESTS_STRING;
FORMAT_TESTS_STRING S_tests_XML_header[NB_MAX_ITEMS_HEADERS_XML];

#define NB_MX_TYPE_FILES_TITLE      4
#define FILES_TITLE_LOGS            0
#define FILES_TITLE_FILES           1
#define FILES_TITLE_REGISTRY        2
#define FILES_TITLE_APPLI           3
HTREEITEM TRV_HTREEITEM_CONF[NB_MX_TYPE_FILES_TITLE]; //list of files

BOOL TEST_REG_PASSWORD_ENABLE;

#define INDEX_FILE                  0
#define INDEX_FILE_NK               1
#define INDEX_LOG                   2
#define INDEX_DISK                  3
#define INDEX_CLIPBOARD             4
#define INDEX_ENV                   5
#define INDEX_TASK                  6
#define INDEX_PROCESS               7
#define INDEX_PREFETCH              8
#define INDEX_PIPE                  9
#define INDEX_LAN                  10
#define INDEX_ROUTE                11
#define INDEX_DNS                  12
#define INDEX_ARP                  13
#define INDEX_SHARE                14
#define INDEX_REG_CONF             15
#define INDEX_REG_SERVICES         16
#define INDEX_REG_USB              17
#define INDEX_REG_SOFTWARE         18
#define INDEX_REG_UPDATE           19
#define INDEX_REG_START            20
#define INDEX_REG_USERS            21
#define INDEX_REG_USERASSIST       22
#define INDEX_REG_MRU              23
#define INDEX_REG_SHELLBAGS        24
#define INDEX_REG_PASSWORD         25
#define INDEX_REG_PATH             26
#define INDEX_REG_GUIDE            27
#define INDEX_REG_DELETED_KEY      28
#define INDEX_ANTIVIRUS            29
#define INDEX_REG_FIREWALL         30
#define INDEX_NAV_FIREFOX          31
#define INDEX_NAV_CHROME           32
#define INDEX_NAV_IE               33
#define INDEX_ANDROID              34
#define INDEX_LDAP                 35

//------------------------------------------------------------------------------
//parameters
BOOL WINE_OS;     //if run in wine !!!
BOOL CONSOL_ONLY;
BOOL STAY_ON_TOP;
BOOL FILE_ACL;
BOOL FILE_ADS;
BOOL FILE_SHA;

BOOL UTC_TIME;

BOOL LOCAL_SCAN;
BOOL update_thread_start;
//------------------------------------------------------------------------------
//scan stats
BOOL B_AUTOSEARCH;
HANDLE h_AUTOSEARCH;

BOOL search_rootkit,search_rootkit_process_tool;
HANDLE H_thread_search_rootkit,H_thread_search_rootkit_process_tools;

//scan
BOOL start_scan, stop_scan;
HANDLE h_thread_scan;
sqlite3 *db_scan;
//------------------------------------------------------------------------------
//process
#define SIZE_ITEMS_PORT_MAX        256
typedef struct line_proc_item
{
  char protocol[SIZE_ITEMS_PORT_MAX];
  char state[SIZE_ITEMS_PORT_MAX];

  char IP_src[SIZE_ITEMS_PORT_MAX];
  char IP_dst[SIZE_ITEMS_PORT_MAX];

  char name_src[SIZE_ITEMS_PORT_MAX];
  char name_dst[SIZE_ITEMS_PORT_MAX];

  char Port_src[SIZE_ITEMS_PORT_MAX];
  char Port_dst[SIZE_ITEMS_PORT_MAX];
}LINE_PROC_ITEM;

#define FIRST_PROCESS_ID  1
#define LAST_PROCESS_ID   32768
typedef struct
{
  DWORD pid;
  char args[MAX_LINE_SIZE];
}PROCESS_INFOS_ARGS;
//------------------------------------------------------------------------------
//struct
typedef struct
{
  char c[MAX_LINE_SIZE];
}LINE_ITEM;
//------------------------------------------------------------------------------
//for sort in lstv
BOOL TRI_RESULT_VIEW, TRI_PROCESS_VIEW, TRI_SNIFF_VIEW, TRI_REG_VIEW;
BOOL TRI_STATE_ALL, TRI_STATE_LOG, TRI_STATE_CRITICAL, TRI_STATE_FILTER, TRI_SQLITE_ED;
int column_tri;

typedef struct SORT_ST
{
  HANDLE hlv;
  BOOL sort;
  unsigned int col;
}sort_st;
//------------------------------------------------------------------------------
//for loading language in local component
#define NB_COMPONENT_STRING         104
#define COMPONENT_STRING_MAX_SIZE   DEFAULT_TMP_SIZE

#define TXT_OPEN_PATH               4
#define REF_MSG                     8
#define TXT_BT_START                10
#define TXT_BT_STOP                 11
#define TXT_SAVE_TO                 12

#define TXT_BT_EXPORT_START         13
#define TXT_BT_EXPORT_STOP          14

#define TXT_POPUP_S_VIEW            15
#define TXT_POPUP_S_SELECTION       16
#define TXT_POPUP_CLIPBORAD         17
#define TXT_POPUP_A_SEARCH          18

#define TXT_FILE_AUDIT              19
#define TXT_FILE_REP                20
#define TXT_FILE_REGISTRY           21
#define TXT_FILE_APPLI              22

#define TXT_CHECK_ACL               23
#define TXT_CHECK_SHA               24
#define TXT_CHECK_ADS               25

#define TXT_POPUP_CHECK_ALL         26
#define TXT_POPUP_UNCHECK_ALL       27

#define TXT_POPUP_ADD_FILE          28
#define TXT_POPUP_ADD_PATH          29
#define TXT_POPUP_UP                30
#define TXT_POPUP_DOWN              31
#define TXT_POPUP_REMOVE_ITEMS      32
#define TXT_POPUP_CLEAN_ALL         33
#define TXT_POPUP_OPEN_PATH         34
#define TXT_POPUP_AUTO_SEARCH       35
#define TXT_POPUP_SAVE_LIST         36
#define TXT_POPUP_AUTO_SEARCH_STOP  37

#define TXT_TOOLTIP_NEW_SESSION     38
#define TXT_TOOLTIP_SEARCH          39


#define TXT_POPUP_STOP_TEST         40

#define TXT_MSG_ADMIN               41
#define TXT_MSG_GUEST               42
#define TXT_MSG_USER                43
#define TXT_MSG_UNK                 44
#define TXT_MSG_NEVER               45
#define TXT_MSG_MDP_NEVER_EXP       46

#define TXT_MSG_BDR                 47

#define TXT_OPEN_REG_PATH           49

#define TXT_COLUMN_PROCESS_REF      52
#define NB_COLUMN_PROCESS_DEF       19

#define TXT_POPUP_CP_LINE           71

#define TXT_KILL_PROCESS            72
#define TXT_DUMP_PROC_MEM           73
#define TXT_POPUP_DLLINJECT         74
#define TXT_ADD_THREAD_INJECT_DLL   75
#define TXT_REM_THREAD_INJECT_DLL   76
#define TXT_POPUP_REFRESH           77

#define TXT_POPUP_SNIFF_FILTRE_IP_SRC   78
#define TXT_POPUP_SNIFF_FILTRE_IP_DST   79
#define TXT_POPUP_SNIFF_FILTRE_PORT_SRC 80
#define TXT_POPUP_SNIFF_FILTRE_PORT_DST 81
#define TXT_POPUP_LINK                  82

#define TXT_SNIFF_FILTRE                83

#define TXT_STOP_CHK_ALL_SHA256         84
#define TXT_LOAD_FILE                   85
#define TXT_REMOVE_FILE                 86
#define TXT_CHK_SHA256                  87
#define TXT_STOP_CHK_SHA256             88
#define TXT_CHK_ALL_SHA256              89
#define TXT_CHK_ALL_SHA256              89
#define TXT_POPUP_AUTO_SEARCH_PATH      90

#define TXT_UPDATE_START                91
#define TXT_UPDATE_END                  92
#define TXT_MSG_RIGHT_ADMIN             93
#define TXT_MSG_RIGHT_ADMIN_ATTENTION   94

#define TXT_GRP_CONF                    95
#define TXT_ADD_DB                      96

#define TXT_ADD_ROOTKIT_TO_DB           97
#define TXT_CHECK_ROOTKIT               98
#define TXT_CHECK_ALL_ROOTKIT           99

#define TXT_POPUP_BACKUP_FILE_LIST      100
#define TXT_POPUP_BACKUP_PATH           101

#define TXT_POPUP_BACKUP_FILE_LIST_STOP 102
#define TXT_POPUP_BACKUP_PATH_STOP      103

typedef struct
{
  char c[COMPONENT_STRING_MAX_SIZE];
}COMPONENT_STRING;
COMPONENT_STRING cps[NB_COMPONENT_STRING];

#define REG_PASSWORD_STRING_VNC                600
#define REG_PASSWORD_STRING_SCREENSAVER        601
#define REG_PASSWORD_STRING_TERMINAL_SERVER    602
#define REG_PASSWORD_STRING_AUTO_LOGON         603
#define SYSKEY_STRING_DEF                     "604"
#define REG_PASSWORD_STRING_LOCAL_USER         605
#define LSAKEY_STRING_DEF                     "606"
#define NLKM_STRING_DEF                       "607"
#define REG_MSCACHE_STRING                     608

BOOL BACKUP_PATH_started, BACKUP_FILE_LIST_started;
//------------------------------------------------------------------------------
//registry params
#define TYPE_VALUE_STRING           0
#define TYPE_VALUE_DWORD            1
#define TYPE_VALUE_MULTI_STRING     2
#define TYPE_VALUE_WSTRING          3
#define TYPE_VALUE_FILETIME         4
#define TYPE_VALUE_MULTI_WSTRING    5
#define TYPE_VALUE_DWORD_TIME       6

#define TYPE_VALUE_WIN_SERIAL       100
#define TYPE_ENUM_SUBNK_DATE        101
#define TYPE_DBL_ENUM_VALUE         102
#define TYPE_ENUM_STRING_VALUE      200   //all string
#define TYPE_ENUM_STRING_WVALUE     210   //all wildstring
#define TYPE_ENUM_STRING_NVALUE     211   //all string with MRU* name
#define TYPE_ENUM_STRING_RVALUE     201   //all string under one key
#define TYPE_ENUM_STRING_RRVALUE    202   //all string under thow key + key
#define TYPE_ENUM_STRING_R_VALUE    203   //all string under one key + key

#define SQLITE_REGISTRY_TYPE_SETTINGS 0x00000000
#define SQLITE_REGISTRY_TYPE_RUN      0x00000000
#define SQLITE_REGISTRY_TYPE_MRU      0x00000000
#define SQLITE_REGISTRY_TYPE_MRU2     0x00000002
#define SQLITE_GUIDE                  0x00000000

HK_F_OPEN hks_mru, hks_mru2;

//for test in guide functions
#define GUIDE_REG_TEST_IDENTIQUE  0x00000000
#define GUIDE_REG_TEST_CONTIENT   0x00000001
#define GUIDE_REG_TEST_EXIST      0x00000002
#define GUIDE_REG_TEST_NEXISTPAS  0x00000003

#define GUIDE_REG_OS_2000         "2000"
#define GUIDE_REG_OS_XP_32b       "XP"
#define GUIDE_REG_OS_XP_64b       "XP_64"
#define GUIDE_REG_OS_2003_32b     "2003"
#define GUIDE_REG_OS_2003_64b     "2003_64"
#define GUIDE_REG_OS_VISTA_32b    "Vista"
#define GUIDE_REG_OS_VISTA_64b    "Vista_64"
#define GUIDE_REG_OS_7_32b        "7"
#define GUIDE_REG_OS_7_64b        "7_64"
#define GUIDE_REG_OS_2008_32b     "2008"
#define GUIDE_REG_OS_2008_64b     "2008_64"
#define GUIDE_REG_OS_8_32b        "8"
#define GUIDE_REG_OS_8_64b        "8_64"
#define GUIDE_REG_OS_ALL          "ALL"
#define GUIDE_REG_OS_ALL_ONLY_32b "ALL_ONLY_32"
#define GUIDE_REG_OS_ALL_ONLY_64b "ALL_ONLY_64"

#define GUIDE_REG_TEST_OK         500
#define GUIDE_REG_TEST_NOK        501

char current_OS[DEFAULT_TMP_SIZE];
BOOL current_OS_BE_64b;
BOOL current_OS_unknow;

//------------------------------------------------------------------------------
#define SIO_RCVALL _WSAIOW(IOC_VENDOR,1)
#define TAILLE_MAX_BUFFER_HEADER    256
#define TAILLE_MAX_BUFFER_TRAME     2600
typedef struct zone_trame
{
  unsigned char  buffer_header[TAILLE_MAX_BUFFER_HEADER];
  unsigned char  buffer[TAILLE_MAX_BUFFER_TRAME];         // taille max d'un trame normalement  = 1500 + ...

  unsigned int   taille_buffer;
  unsigned int   ProtoType;                               // ethernet, .... (code exacte)
  char           ip_src[IP_SIZE_MAX];
  unsigned short src_port;
  char           ip_dst[IP_SIZE_MAX];
  unsigned short dst_port;
  unsigned short TTL;
}TRAME_BUFFER;

HANDLE hMutex_TRAME_BUFFER;
TRAME_BUFFER *Trame_buffer;                               //liste des trames lues
unsigned long int NB_trame_buffer;                        //nombre de trames lues
BOOL start_sniff;
HANDLE Hsniff;
BOOL follow_sniff;

//color for sniff
#define TXT_TCP_IPV4 "TCP/IPv4"
#define TXT_TCP_IPV6 "TCP/IPv6"
#define TXT_UDP_IPV4 "UDP/IPv4"
#define TXT_UDP_IPV6 "UDP/IPv6"

#define TXT_ICMPV4   "ICMPv4"
#define TXT_ICMPV6   "ICMPv6"
#define TXT_IGMP     "IGMP"
HBRUSH Hb_green, Hb_blue, Hb_pink, Hb_violet;
//------------------------------------------------------------------------------
//users
typedef struct
{
  char name[MAX_PATH];
  char RID[MAX_PATH];
  char SID[MAX_PATH];
  char group[MAX_PATH];
  char type[MAX_PATH];
  char description[MAX_PATH];
  char last_logon[DATE_SIZE_MAX];
  char last_password_change[MAX_PATH];
  char pwdump_pwd_raw_format[MAX_PATH];
  char pwdump_pwd_format[MAX_PATH];

  DWORD nb_connexion;
  DWORD state_id;
}USERS_INFOS;
//------------------------------------------------------------------------------
#define NB_MAX_SQL_REQ        MAX_PATH

#define SPECIAL_CASE_CHROME_PASSWORD 4

typedef struct
{
  char sql[MAX_PATH];           //utc
  char params[MAX_PATH];
  DWORD test_string_id;
  char sql_localtime[MAX_PATH];
}SQL_REQ;

typedef struct
{
  sqlite3 *db;
  unsigned int session_id;
  DWORD index;
  char file[MAX_PATH];
}CALBACK_SQLITE_TEST_FORMAT;

unsigned int nb_sql_ANDROID, nb_sql_CHROME, nb_sql_FIREFOX;

SQL_REQ sql_ANDROID[NB_MAX_SQL_REQ];
SQL_REQ sql_CHROME[NB_MAX_SQL_REQ];
SQL_REQ sql_FIREFOX[NB_MAX_SQL_REQ];

//------------------------------------------------------------------------------
//CMD
unsigned int item_n;
#define CMD_TYPE_LIST_SESSION   0x00000001
#define CMD_TYPE_LIST_LANGUAGE  0x00000002
#define CMD_TYPE_LIST_TESTS     0x00000003
#define CMD_READ_SESSION        0x00000004

#define NB_MAX_ITEM_CMD         256
unsigned int nb_file_cmd;
unsigned int nb_path_cmd;

typedef struct
{
  char c[MAX_PATH];
}FORMAT_FILE_CMD;

FORMAT_FILE_CMD file_cmd[NB_MAX_ITEM_CMD], path_cmd[NB_MAX_ITEM_CMD];
//------------------------------------------------------------------------------
//columns
#define TYPE_SQL_STANDARD           0
#define TYPE_SQL_EXPLORER           1
#define TYPE_SQL_PROCESS            2
#define TYPE_SQL_STATE              3

#define MODE_SQL_NONE              99
#define MODE_SQL_FULL               0
#define MODE_SQL_LITE               1
#define MODE_SQL_LOGS               2

#define NB_MAX_COLUMNS             32
#define NB_MAX_ST_COLUMS           32
typedef struct
{
  unsigned int mode_simple;
  unsigned int nb_columns;
  char request[REQUEST_MAX_SIZE];
}ST_COLUMS;

ST_COLUMS st_columns; // use for export datas
unsigned int nb_current_columns;
int current_item_selected;

//sessions
#define NB_MAX_SESSION             MAX_LINE_SIZE
unsigned long int current_session_id;
unsigned long int nb_session, session[NB_MAX_SESSION];

DWORD pos_search,pos_search_reg,pos_search_sqlite;
char session_name_ch[MAX_PATH];
char malware_check[MAX_PATH];
//------------------------------------------------------------------------------
//richedit
//couleur
#define ROUGE RGB(255, 0, 0)
#define NOIR RGB(0  ,  0, 0)
#define VERT RGB(51 ,153, 0)
#define BLEU RGB(0  ,  0,255)
#define GRIS RGB(153,153,153)

HINSTANCE richDll;
//HWND hdbclk_info;
WNDPROC wndproc_hdbclk_info;

//RicheEdit
void RichSetTopPos(HWND HRichEdit);
void RichEditInit(HWND HRichEdit);
void RichEditCouleur(HWND HRichEdit,COLORREF couleur,char* txt);
void RichEditCouleurGras(HWND HRichEdit,COLORREF couleur,char* txt);
DWORD RichEditTextSize(HWND HRichEdit);
//------------------------------------------------------------------------------
//MD5
#include "crypt/md5.h"
typedef unsigned char md5_byte_t; /* 8-bit byte */
typedef unsigned int md5_word_t; /* 32-bit word */

/* Define the state of the MD5 Algorithm. */
typedef struct md5_state_s {
    md5_word_t count[2];	/* message length in bits, lsw first */
    md5_word_t abcd[4];		/* digest buffer */
    md5_byte_t buf[64];		/* accumulate block */
}md5_state_t;

/* Initialize the algorithm. */
void md5_init(md5_state_t *pms);

/* Append a string to the message. */
void md5_append(md5_state_t *pms, const md5_byte_t *data, int nbytes);

/* Finish the message and return the digest. */
void md5_finish(md5_state_t *pms, md5_byte_t digest[16]);
//------------------------------------------------------------------------------
#include "crypt/rc4.h"
#include "crypt/opensslv.h"
//------------------------------------------------------------------------------
//DLL load
//------------------------------------------------------------------------------
#define WINTRUST_ACTION_GENERIC_VERIFY_V2   { 0xaac56b, 0xcd44, 0x11d0, { 0x8c,0xc2,0x00,0xc0,0x4f,0xc2,0x95,0xee }}
typedef struct WINTRUST_FILE_INFO_
{
    DWORD   cbStruct;
    LPCWSTR pcwszFilePath;
    HANDLE  hFile;
    GUID*   pgKnownSubject;
} WINTRUST_FILE_INFO, *PWINTRUST_FILE_INFO;

typedef struct _CERT_STRONG_SIGN_SERIALIZED_INFO {
  DWORD  dwFlags;
  LPWSTR pwszCNGSignHashAlgids;
  LPWSTR pwszCNGPubKeyMinBitLengths;
} CERT_STRONG_SIGN_SERIALIZED_INFO, *PCERT_STRONG_SIGN_SERIALIZED_INFO;

typedef struct _CERT_STRONG_SIGN_PARA {
  DWORD cbSize;
  DWORD dwInfoChoice;
  union {
    void                              *pvInfo;
    PCERT_STRONG_SIGN_SERIALIZED_INFO pSerializedInfo;
    LPSTR                             pszOID;
  } DUMMYUNIONNAME;
} CERT_STRONG_SIGN_PARA, *PCERT_STRONG_SIGN_PARA;

typedef struct WINTRUST_SIGNATURE_SETTINGS_ {
  DWORD                  cbStruct;
  DWORD                  dwIndex;
  DWORD                  dwFlags;
  DWORD                  cSecondarySigs;
  DWORD                  dwVerifiedSigIndex;
  PCERT_STRONG_SIGN_PARA pCryptoPolicy;
} WINTRUST_SIGNATURE_SETTINGS, *PWINTRUST_SIGNATURE_SETTINGS;

typedef struct _WINTRUST_DATA {
  DWORD                       cbStruct;
  LPVOID                      pPolicyCallbackData;
  LPVOID                      pSIPClientData;
  DWORD                       dwUIChoice;
  DWORD                       fdwRevocationChecks;
  DWORD                       dwUnionChoice;
  union
  {
    struct WINTRUST_FILE_INFO_  *pFile;
    struct WINTRUST_CATALOG_INFO_  *pCatalog;
    struct WINTRUST_BLOB_INFO_  *pBlob;
    struct WINTRUST_SGNR_INFO_  *pSgnr;
    struct WINTRUST_CERT_INFO_  *pCert;
  };
  DWORD                       dwStateAction;
  HANDLE                      hWVTStateData;
  WCHAR                       *pwszURLReference;
  DWORD                       dwProvFlags;
  DWORD                       dwUIContext;
  WINTRUST_SIGNATURE_SETTINGS *pSignatureSettings;
} WINTRUST_DATA, *PWINTRUST_DATA;

#define TRUST_E_NOSIGNATURE                                _HRESULT_TYPEDEF_(0x800B0100L)
#define TRUST_E_SUBJECT_FORM_UNKNOWN                       _HRESULT_TYPEDEF_(0x800B0003L)
#define TRUST_E_EXPLICIT_DISTRUST                          _HRESULT_TYPEDEF_(0X800B0111)
#define TRUST_E_SUBJECT_NOT_TRUSTED                        _HRESULT_TYPEDEF_(0x800B0004L)
#define CRYPT_E_SECURITY_SETTINGS                          _HRESULT_TYPEDEF_(0x80092026L)
#define TRUST_E_PROVIDER_UNKNOWN                           _HRESULT_TYPEDEF_(0x800B0001L)

typedef struct _AT_ENUM {
  DWORD JobId;
  DWORD JobTime;
  DWORD DaysOfMonth;
  UCHAR DaysOfWeek;
  UCHAR Flags;
  LPWSTR Command;
}AT_ENUM;

#define DNS_MALWARE_MIN_SIZE  4

/*typedef enum  {
  TCP_TABLE_BASIC_LISTENER,
  TCP_TABLE_BASIC_CONNECTIONS,
  TCP_TABLE_BASIC_ALL,
  TCP_TABLE_OWNER_PID_LISTENER,
  TCP_TABLE_OWNER_PID_CONNECTIONS,
  TCP_TABLE_OWNER_PID_ALL,
  TCP_TABLE_OWNER_MODULE_LISTENER,
  TCP_TABLE_OWNER_MODULE_CONNECTIONS,
  TCP_TABLE_OWNER_MODULE_ALL
}TCP_TABLE_CLASS, *PTCP_TABLE_CLASS;*/

typedef enum  {
  UDP_TABLE_BASIC,
  UDP_TABLE_OWNER_PID,
  UDP_TABLE_OWNER_MODULE
}UDP_TABLE_CLASS, *PUDP_TABLE_CLASS;

//struct
#define TCPIP_OWNING_MODULE_SIZE 16
typedef struct _MIB_TCPROW_OWNER_MODULE
{
  DWORD         dwState;
  DWORD         dwLocalAddr;
  DWORD         dwLocalPort;
  DWORD         dwRemoteAddr;
  DWORD         dwRemotePort;
  DWORD         dwOwningPid;
  LARGE_INTEGER liCreateTimestamp;
  ULONGLONG     OwningModuleInfo[TCPIP_OWNING_MODULE_SIZE];
}MIB_TCPROW_OWNER_MODULE, *PMIB_TCPROW_OWNER_MODULE;

typedef struct {
  DWORD                   dwNumEntries;
  MIB_TCPROW_OWNER_MODULE table[ANY_SIZE];
}MIB_TCPTABLE_OWNER_MODULE, *PMIB_TCPTABLE_OWNER_MODULE;

typedef struct _MIB_UDPROW_EX {
   DWORD dwLocalAddr;
   DWORD dwLocalPort;
   DWORD dwProcessId;
}MIB_UDPROW_EX, *PMIB_UDPROW_EX;

typedef struct _MIB_UDPTABLE_EX
{
   DWORD dwNumEntries;
   MIB_UDPROW_EX table[1];
}MIB_UDPTABLE_EX, *PMIB_UDPTABLE_EX;

HMODULE hDLL_NETAPI32;
typedef NET_API_STATUS (WINAPI *NETAPIBUFFERFREE)(LPVOID Buffer);
NETAPIBUFFERFREE MyNetApiBufferFree;
typedef NET_API_STATUS (WINAPI *NETGROUPENUM)(LPCWSTR servername, DWORD level, LPBYTE* bufptr, DWORD prefmaxlen, LPDWORD entriesread, LPDWORD totalentries, LPDWORD resume_handle);
NETGROUPENUM MyNetLocalGroupEnum;
typedef DWORD (WINAPI *NETUSERENUM)(LPCWSTR servername, DWORD level, DWORD filter, LPBYTE* bufptr, DWORD prefmaxlen, LPDWORD entriesread, LPDWORD totalentries, LPDWORD resume_handle);
NETUSERENUM MyNetUserEnum;
typedef DWORD (WINAPI *NETUSERGETINFO)( LPCWSTR servername, LPCWSTR username, DWORD level, LPBYTE* bufptr);
NETUSERGETINFO MyNetUserGetInfo;
typedef DWORD (WINAPI *NETUSERGETLOCALGROUPS)( LPCWSTR servername, LPCWSTR username, DWORD level, DWORD flags, LPBYTE* bufptr, DWORD prefmaxlen, LPDWORD entriesread, LPDWORD totalentries);
NETUSERGETLOCALGROUPS MyNetUserGetLocalGroups;
typedef NET_API_STATUS (WINAPI *NETSHAREENUM)(LPWSTR servername, DWORD level, LPBYTE* bufptr, DWORD prefmaxlen, LPDWORD entriesread, LPDWORD totalentries, LPDWORD resume_handle);
NETSHAREENUM MyNetShareEnum;
typedef NET_API_STATUS (WINAPI *NETSCHEDULEJOBENUM)(LPCWSTR servername,LPBYTE* PointerToBuffer,DWORD PreferredMaximumLength,LPDWORD EntriesRead,LPDWORD TotalEntries,LPDWORD ResumeHandle );
NETSCHEDULEJOBENUM MyNetScheduleJobEnum;

HMODULE hDLL_ADVAPI32;
typedef int (WINAPI *SF)(unsigned char*, int*, unsigned char*);
SF sf27;
SF sf25;

HMODULE hDLL_KERNEL32;
typedef BOOL (WINAPI *WOW64DISABLEREDIRECT)(PVOID *OldValue);
WOW64DISABLEREDIRECT Wow64DisableWow64FsRedirect;
WOW64DISABLEREDIRECT Wow64RevertWow64FsRedirect;

HMODULE hDLL_DNSAPI;
typedef int(WINAPI *DNS_GET_CACHE_DATA_TABLE)(PDNS_RECORD*);
DNS_GET_CACHE_DATA_TABLE DnsGetCacheDataTable;

HMODULE hDLL_WINTRUST;
typedef LONG (WINAPI *WINVERIFYTRUST)(HWND hWnd, GUID *pgActionID, LPVOID pWVTData);
WINVERIFYTRUST WinVerifyTrust;

HMODULE hDLL_IPHLPAPI;
typedef DWORD (WINAPI TypeGetExtendedTcpTable)(PVOID, PDWORD, BOOL, ULONG, TCP_TABLE_CLASS, ULONG);
typedef DWORD (WINAPI TypeGetExtendedUdpTable)(PVOID, PDWORD, BOOL, ULONG, UDP_TABLE_CLASS, ULONG);
typedef DWORD (WINAPI *GETIPFORWARDTABLE)(PMIB_IPFORWARDTABLE pIpForwardTable, PULONG pdwSize, BOOL bOrder);
TypeGetExtendedTcpTable *MyGetExtendedTcpTable;
TypeGetExtendedUdpTable *MyGetExtendedUdpTable;
GETIPFORWARDTABLE MyGetIpForwardTable;

HMODULE hDLL_VSSAPI;

HMODULE hDLL_VERSION;
typedef BOOL (WINAPI * GETFILEVERSIONINFO)(LPCTSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);
typedef BOOL (WINAPI * VERQUERYVALUE)(LPCVOID pBlock, LPCTSTR lpSubBlock, LPVOID *lplpBuffer, PUINT puLen);
GETFILEVERSIONINFO MyGetFileVersionInfo;
VERQUERYVALUE MyVerQueryValue;

typedef struct _PEB
{
  BYTE Reserved1[2];
  BYTE BeingDebugged;
  BYTE Reserved2[229];
  PVOID Reserved3[59];
  ULONG SessionId;
} *PPEB;

typedef struct _PROCESS_BASIC_INFORMATION
{
  PVOID Reserved1;
  PPEB PebBaseAddress;
  PVOID Reserved2[2];
  ULONG_PTR UniqueProcessId;
  PVOID Reserved3;
}PROCESS_BASIC_INFORMATION;

typedef enum _PROCESSINFOCLASS
{
  ProcessBasicInformation
}PROCESSINFOCLASS;

typedef struct ___PEB
{
  DWORD dwFiller[4];
  DWORD dwInfoBlockAddress;
}__PEB;

typedef struct ___INFOBLOCK
{
  DWORD dwFiller[16];
  WORD wLength;
  WORD wMaxLength;
  DWORD dwCmdLineAddress;
}__INFOBLOCK;

HMODULE hDLL_NTDLL;
typedef LONG (WINAPI * LPNTQUERYINFOPROCESS)(HANDLE h, PROCESSINFOCLASS p, PVOID pv, ULONG u, PULONG pu);
LPNTQUERYINFOPROCESS MyNtQueryInformationProcess;
//------------------------------------------------------------------------------
//SQLITE functions
void ExtractSQLITE_DB();
char *convertStringToSQL(char *data, unsigned int size_max);
BOOL SQLITE_LireData(FORMAT_CALBAK_READ_INFO *datas, char *sqlite_file);
BOOL SQLITE_WriteData(FORMAT_CALBAK_READ_INFO *datas, char *sqlite_file);
BOOL SQLITE_LoadSession(char *file);
BOOL SQLITE_SaveSession(char *file);
void addLogtoDB(char *event, char *indx, char *log_id,
                char *send_date, char *write_date,
                char *source, char *description, char *user, char *rid, char *sid,
                char *state, char *critical, unsigned int session_id, sqlite3 *db);
void addFiletoDB(char *path, char *file, char *extension,
                  char *Create_time, char *Modify_time, char *Access_Time,char *Size,
                  char *Owner, char *RID, char *sid, char *ACL,
                  char *Hidden, char *System, char *Archive, char *Encrypted, char *Tempory,
                  char *ADS, char *SAH256, char *VirusTotal, char *Description, unsigned int session_id, sqlite3 *db);
void GetColumnInfo(unsigned int id);

void EndSession(DWORD id, sqlite3 *db);

//save function
BOOL customAddSrc(register TZIP *tzip, const void *destname, const void *src, DWORD len, DWORD flags);
char *GenerateNameToSave(char *name, DWORD name_max_size, char *ext);
BOOL SaveLSTV(HANDLE hlv, char *file, unsigned int type, unsigned int nb_column);
BOOL SaveLSTVSelectedItems(HANDLE hlv, char *file, unsigned int type, unsigned int nb_column);
BOOL SaveTRV(HANDLE htv, char *file, unsigned int type);
BOOL SaveLSTVItemstoREG(HANDLE hlv, char *file, BOOL selected);
DWORD WINAPI SaveAll(LPVOID lParam);
DWORD WINAPI ChoiceSaveAll(LPVOID lParam);
void CopyDataToClipboard(HANDLE hlv, DWORD line, unsigned short column);
void CopyAllDataToClipboard(HANDLE hlv, DWORD line, unsigned short nbcolumn);
void CopyColumnDataToClipboard(HANDLE hlv, DWORD nline, unsigned short nbcolumn, unsigned short startcolumn);
void SaveALL(char*filetosave, char*computername, BOOL local);

//import function
DWORD WINAPI ImportCVSorSHA256deep(LPVOID lParam);

//usuals functions
unsigned int ExtractTextFromPathNb(char *path);
void CopyStringToClipboard(char *s);
char *ExtractTextFromPath(char *path, char *txt, unsigned int txt_size_max, unsigned int index);
void ReviewWOW64Redirect(PVOID OldValue_W64b);
unsigned long int Contient(char*data,char*chaine);
unsigned long int ContientNoCass(char*data,char*chaine);
char *ReplaceEnv(char *var, char *path, unsigned int size_max);
BOOL SetDebugPrivilege(BOOL enable);
BOOL startWith(char* txt, char *search);
BOOL endWith(char* txt, char *search);
char *filetimeToString(FILETIME FileTime, char *str, unsigned int string_size);
char *filetimeToString_GMT(FILETIME FileTime, char *str, unsigned int string_size);
char *timeToString(DWORD time, char *str, unsigned int string_size);
char *convertUTF8toUTF16(char *src, DWORD max_size);
char *convertUTF8toUTF16toChar(char *src, DWORD size_src, char *dst, DWORD size_dst);
void replace_one_char(char *buffer, unsigned long int taille, char chtoreplace, char chreplace);
char *charToLowChar(char *src);
char *DataToHexaChar(char *data, unsigned int data_size, char *hexa_char, unsigned int hexa_char_size);
char *extractDirectoryFromPath(char *path);
char *extractExtFromFile(char *file, char *ext, unsigned int ext_size_max);
char *extractFileFromPath(char *path, char *file, unsigned int file_size_max);
BOOL isDirectory(char *path);
void LoadAllDLLAndFunction();
void FreeAllDLLAndFunction();

//init functions
void UpdateRtCA();
void InitSQLStrings();
void InitString();
void InitGlobalLangueString(unsigned int langue_id);
void InitGlobalConfig(BOOL acl, BOOL ads, BOOL sha, BOOL local_scan, BOOL utc);
DWORD WINAPI InitGUIConfig(LPVOID lParam);
void EndGUIConfig(HANDLE hwnd);
BOOL isWine();

//cmd function
int callback_sqlite_CMD(void *datas, int argc, char **argv, char **azColName);
void AddNewSession(BOOL local_only, char *name, sqlite3 *db);

//mdp_save in ini file
#define MDP_TEST        "zoo218745963zooTO"
char *chr(char *data,char* password);
char *dechr(char *data,unsigned int data_size,char* password);

//gui interfaces
void redimColumn(HANDLE f,int lv,int column,unsigned int col_size);
void redimColumnH(HANDLE hlv,int column,unsigned int col_size);
void Modify_Style(HANDLE hcomp, long style, BOOL add);
void AddComboBoxItem(HANDLE hcombo, char *txt, DWORD img);
void FileToTreeView(char *c_path);
void DisableGrid(HANDLE hlv, BOOL disable, int menu);

int CALLBACK CompareStringTri(LPARAM lParam1, LPARAM lParam2, LPARAM lParam3);
void c_Tri(HANDLE hlv, unsigned short colonne_ref, BOOL sort);
void AddtoLV(HANDLE hlv, unsigned int nb_column, LINE_ITEM *item, BOOL select);
void LVDelete(HANDLE hlv);
DWORD LVSearch(HANDLE hlv, unsigned short nb_col, char *search, DWORD start);
DWORD LVSearchNoCass(HANDLE hlv, unsigned short nb_col, char *search, DWORD start);
void LVAllSearch(HANDLE hlv, unsigned short nb_col, char *search);

void SupDoublon(HANDLE htrv,HTREEITEM htreeParent);
void check_childs_treeview(HANDLE htrv, BOOL check);
void check_treeview(HANDLE htrv, HTREEITEM hitem, int state);
BOOL Ischeck_treeview(HANDLE htrv, HTREEITEM hitem);
HTREEITEM AddItemTreeView(HANDLE Htreeview, char *txt, HTREEITEM hparent);
HTREEITEM AddItemTreeViewImg(HANDLE Htreeview, char *txt, HTREEITEM hparent, unsigned int index_img);
void GetItemTreeView(HTREEITEM hitem, HANDLE htrv,char *txt, unsigned int size);
char *GetTextFromTrv(HTREEITEM hitem, char *txt, DWORD item_size_max);
int GetTrvItemIndex(HTREEITEM hitem, HANDLE htrv);

void AddtoToolTip(HWND hcompo, HWND hTTip, HINSTANCE hinst, unsigned int id, char *title, char *txt);
void ModifyToolTip(HWND hcompo, HWND hTTip, HINSTANCE hinst, unsigned int id, char *title, char *txt);
void IDM_STAY_ON_TOP_fct();
void SCREENSHOT_fct();

//file function
BOOL FileExist(char *file);
char *ConvertPathFromPath(char *path);
char* GetLocalPath(char *path, unsigned int sizeMax);
void GetACLS(char *file, char *acls, char* owner,char *rid, char *sid, unsigned int size_max);
void GetOwner(char *file, char* owner,char *rid, char *sid, unsigned int size_max);
void SidtoUser(PSID psid, char *user, char *rid, char *sid, unsigned int max_size);
void GetSIDFromUser(char *user, char* rid, char *sid, unsigned int max_size);
void CleanTreeViewFiles(HANDLE htrv);
void AddItemFiletoTreeView(HANDLE htv, char *lowcase_file, char *path, char *global_path);
DWORD  WINAPI AutoSearchFiles(LPVOID lParam);
void FileToSHA256(char *path, char *csha256);
void FileToSHA256_noTM(char *path, char *csha256); //with no date time modify on the file MFT
int VerifySignFile(char *file, char *msg, unsigned int msg_sz_max);
BOOL GetSHAandVerifyFromPathFile(char *path, char *sha256, char *verify, unsigned int buffer_max_sz);
void ConsoleDirectory_sha256deep(char *tmp_path);
BOOL dd(char *disk, char *file, LONGLONG file_sz_max, BOOL progress);
void loadFile_test(char *file, unsigned int index);
void SaveAllTRVFilesToZip(char*filetosave);
void SaveALLCustom(char*filetosave, char *computername, char *path);

BOOL VSSFileCopyFilefromPath(char *path_src, char *path_dst);

//MFT
ULONGLONG HexaToll(char *src, unsigned int nb);
BOOL CopyFileFromMFT(HANDLE hfile, char *destination);
char *Partition_Type(unsigned int code, char *ctype, unsigned int ctype_sz_max);
BOOL MBRReadInfos(char *raw_datas, unsigned int raw_datas_sz, MBRINFOS_STRUCT*infos);
BOOL ReadPartInfos(char *raw_datas, unsigned int raw_datas_sz, MBRINFOS_STRUCT*infos, unsigned int index);

//rootkit
void addNewRootkitToDB(HANDLE hlstv, char *filename, char*sha256, char* description, char *source, char*update_time, int params, sqlite3 *db);
void LoadRootKitDB(HANDLE lstvdb);
BOOL CheckIfRootKitDbExist(HANDLE lstvdb, char *filename, char *sha256, char *description, char *source, char* update_time, DWORD buffers_sz);
void checkLstvItemId(HANDLE hlstv, HANDLE hlstv_rootkit, DWORD item_id, DWORD col_file, DWORD col_sha256, DWORD col_ref, BOOL msg);
DWORD WINAPI checkAllLstvItem(LPVOID lParam);
DWORD WINAPI checkAllLstvItemProcessTools(LPVOID lParam);

//registry functions
void OpenRegeditKey(char* chk, char *key);
HKEY hkStringtohkey(char *chkey);
int set_sam_tree_access( HKEY start, char *pth);
void GetRegistryKeyOwner(HKEY hKey, char* owner,char *rid, char *sid, unsigned int size_max);
void ReadKeyUpdate(HKEY ENTETE,char *chemin, char *date, DWORD size_date);
DWORD ReadValue(HKEY hk,char *path,char *value,char *data, DWORD data_size);
long int ReadDwordValue(HKEY hk,char *path,char *value);
void ReadFILETIMEValue(HKEY hk,char *path,char *value, FILETIME *ft);
void Scan_HCU_files_ALL(DWORD session, sqlite3 *db, int CMDScanNum);

//registry function for raw files
int callback_sqlite_registry_mru_file(void *datas, int argc, char **argv, char **azColName);

void AddToLVRegBin(HANDLE hlv, LINE_ITEM *item, unsigned short nb_colonne);
BOOL registry_syskey_local(char*sk, unsigned int sk_size);
BOOL registry_users_extract(sqlite3 *db, unsigned int session_id);
BOOL OpenRegFiletoMem(HK_F_OPEN *hks, char *file);
void CloseRegFiletoMem(HK_F_OPEN *hks);
HBIN_CELL_NK_HEADER *GetRegistryNK(char *buffer, DWORD taille_fic, DWORD position, DWORD pos_fhbin, char *reg_path);
DWORD GetSubNK(char *buffer, DWORD taille_fic, HBIN_CELL_NK_HEADER *nk_h, DWORD pos_fhbin,
               unsigned int index, char *subkey, unsigned int subkey_size);
HBIN_CELL_NK_HEADER * GetSubNKtonk(char *buffer, DWORD taille_fic, HBIN_CELL_NK_HEADER *nk_h, DWORD pos_fhbin, unsigned int index);
DWORD GetValueData(char *buffer, DWORD taille_fic, HBIN_CELL_NK_HEADER *nk_h, DWORD pos_fhbin,
                   unsigned int index, char *value, unsigned int value_size, char *data, unsigned int data_size);
BOOL Readnk_Value(char *buffer, DWORD taille_fic, DWORD position, DWORD pos_fhbin, char *reg_path, HBIN_CELL_NK_HEADER *nk_h_t,
                  char *read_value, char *data, unsigned int data_size);
DWORD ReadBinarynk_Value(char *buffer, DWORD taille_fic, DWORD position, DWORD pos_fhbin, char *reg_path, HBIN_CELL_NK_HEADER *nk_h_t,
                         char *read_value, void *data, DWORD *data_size);
BOOL Readnk_Class(char *buffer, DWORD taille_fic, DWORD position, DWORD pos_fhbin, char *reg_path, HBIN_CELL_NK_HEADER *nk_h_t,
                  char *Class, unsigned int Class_size);
BOOL Readnk_Infos(char *buffer, DWORD taille_fic, DWORD position, DWORD pos_fhbin, char *reg_path, HBIN_CELL_NK_HEADER *nk_h_t,
                  char *last_update, unsigned int last_update_size, char *rid, unsigned int rid_size,char *sid, unsigned int sid_size);
DWORD GetRegistryData(HBIN_CELL_VK_HEADER *vk_h, DWORD taille_fic, char *buffer, DWORD pos_fhbin, char *data, DWORD data_size);
DWORD GetBinaryRegistryData(HBIN_CELL_VK_HEADER *vk_h, DWORD taille_fic, char *buffer, DWORD pos_fhbin, char *data, DWORD *data_size);
DWORD GetBinaryValueData(char *buffer, DWORD taille_fic, HBIN_CELL_NK_HEADER *nk_h, DWORD pos_fhbin,
                         unsigned int index, char *value, unsigned int value_size, char *data, DWORD *data_size);
void ReadLNKInfos(char *file, unsigned int session_id, sqlite3 *db);
void GetRecoveryRegFile(char *reg_file, HTREEITEM hparent, char *parent, HANDLE hlv, HANDLE htv);
void ReadPath(char *buffer, DWORD taille_fic, DWORD position, char *path, unsigned int path_size_max, char *parent, char *sid, unsigned int sid_size_max);
void registry_userassist_file(HK_F_OPEN *hks, char *ckey, unsigned int session_id, sqlite3 *db);
void Scan_registry_ShellBags_file(HK_F_OPEN *hks, char *ckey, char *pathcmd, unsigned int session_id, sqlite3 *db, BOOL first);
void Scan_registry_deletedKey_file(char *reg_file,unsigned int session_id,sqlite3 *db);
BOOL GetStorageInfos(char *infos, char *vendor, char*product, char *id, char* GUID, char *path_t, unsigned int sz_max);

//haxe
void ReadMagicNumber(char *file, char *magicnumber, unsigned short magicnumber_size_max);

//process
BOOL GetProcessArg(HANDLE hProcess, char* arg, unsigned int size);
void GetProcessOwner(DWORD pid, char *owner, char *rid, char *sid, DWORD size_max);
DWORD GetPortsFromPID(DWORD pid, LINE_PROC_ITEM *port_line, unsigned int nb_item_max,unsigned int taille_max_line);
void LoadPRocessList(HWND hlv);
void FileInfoRead(char *file, char *ProductName, char *FileVersion, char *CompanyName, char *FileDescription, DWORD size_max);

//log functions
BOOL readMessageDatas(EVENTLOGRECORD *pevlr, char *eventname, char *source, char *resultat, unsigned int resultat_max_size);
void TraiterEventlogFileEvt(char * eventfile, sqlite3 *db, unsigned int session_id);
void TraiterEventlogFileLog(char * eventfile, sqlite3 *db, unsigned int session_id);
void TraiterEventlogFileEvtx(char *eventfile, sqlite3 *db, unsigned int session_id);
char *CheckNameAndDescription(char *name, unsigned int name_size_max);

//reg file explorer
void InitDlgRegfile();

//network
BOOL GetMACAdresseFromGUID_CARD(char *GUID, char *mac, unsigned int mac_size);

//virustotal
DWORD WINAPI CheckAllFileToVirusTotal(LPVOID lParam);
DWORD WINAPI CheckAllFileToVirusTotalProcess(LPVOID lParam);
DWORD WINAPI CheckSelectedItemToVirusTotal(LPVOID lParam);
BOOL CheckItemToVirusTotal(HANDLE hlv, DWORD item, unsigned int column_sha256, unsigned int colum_sav, char *token, BOOL check, BOOL check_all_lv_items);

//DNS malware
void MalwareCheck(char*name);
BOOL SQLITE_Data(FORMAT_CALBAK_READ_INFO *datas, char *sqlite_file, DWORD flag);

//state
void InitGuiState();

//GUI functions
BOOL CALLBACK DialogProc_conf(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_info(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_sniff(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_reg_file(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_reg(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_date(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_state(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_sqlite_ed(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_hexa(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_proxy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK DialogProc_infos(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

DWORD WINAPI ImpEcran(LPVOID lParam);

//Scan function
DWORD WINAPI Scan_files(LPVOID lParam);
DWORD WINAPI Scan_log(LPVOID lParam);
DWORD WINAPI Scan_clipboard(LPVOID lParam);
DWORD WINAPI Scan_env(LPVOID lParam);
DWORD WINAPI Scan_disk(LPVOID lParam);
DWORD WINAPI Scan_network(LPVOID lParam);
DWORD WINAPI Scan_route(LPVOID lParam);
DWORD WINAPI Scan_share(LPVOID lParam);
DWORD WINAPI Scan_pipe(LPVOID lParam);
DWORD WINAPI Scan_process(LPVOID lParam);
DWORD WINAPI Scan_android_history(LPVOID lParam);
DWORD WINAPI Scan_chrome_history(LPVOID lParam);
DWORD WINAPI Scan_ie_history(LPVOID lParam);
DWORD WINAPI Scan_firefox_history(LPVOID lParam);
DWORD WINAPI Scan_registry_setting(LPVOID lParam);
DWORD WINAPI Scan_registry_software(LPVOID lParam);
DWORD WINAPI Scan_registry_update(LPVOID lParam);
DWORD WINAPI Scan_registry_service(LPVOID lParam);
DWORD WINAPI Scan_registry_start(LPVOID lParam);
DWORD WINAPI Scan_registry_usb(LPVOID lParam);
DWORD WINAPI Scan_registry_user(LPVOID lParam);
DWORD WINAPI Scan_registry_password(LPVOID lParam);
DWORD WINAPI Scan_registry_userassist(LPVOID lParam);
DWORD WINAPI Scan_registry_mru(LPVOID lParam);
DWORD WINAPI Scan_registry_ShellBags(LPVOID lParam);
DWORD WINAPI Scan_registry_path(LPVOID lParam);
DWORD WINAPI Scan_registry_deletedKey(LPVOID lParam);
DWORD WINAPI Scan_guide(LPVOID lParam);
DWORD WINAPI Scan_dns(LPVOID lParam);
DWORD WINAPI Scan_arp(LPVOID lParam);
DWORD WINAPI Scan_task(LPVOID lParam);
DWORD WINAPI Scan_antivirus(LPVOID lParam);
DWORD WINAPI Scan_firewall(LPVOID lParam);
DWORD WINAPI Scan_prefetch(LPVOID lParam);
DWORD WINAPI Scan_ldap(LPVOID lParam);

DWORD WINAPI CMDScanNum(LPVOID lParam);
DWORD WINAPI CMDScan(LPVOID lParam);
DWORD WINAPI GUIScan(LPVOID lParam);
DWORD WINAPI StopGUIScan(LPVOID lParam);

DWORD WINAPI BackupRegFile(LPVOID lParam);
DWORD WINAPI BackupEvtFile(LPVOID lParam);
DWORD WINAPI BackupNTDIS(LPVOID lParam);
DWORD WINAPI BackupDrive(LPVOID lParam);
void dd_mbr();
DWORD WINAPI BackupDisk(LPVOID lParam);
DWORD WINAPI BackupFile(LPVOID lParam);
DWORD WINAPI BackupAllFiles(LPVOID lParam);
DWORD WINAPI DumpProcessMemory(LPVOID lParam);

