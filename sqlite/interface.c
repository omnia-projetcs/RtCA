//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
int callback_sqlite(void *datas, int argc, char **argv, char **azColName)
{
  //FORMAT_CALBAK_TYPE *type = datas;
  FORMAT_CALBAK_READ_INFO *type = datas;
  switch(type->type)
  {
    //----------------------------------------
    case TYPE_SQLITE_FLAG_GET_ITEMS_INFO:
    {
      //add item in the lstv !!!
      FORMAT_CALBAK_READ_INFO fcri;
      fcri.type = TYPE_SQLITE_TEST;
      char request[MAX_LINE_SIZE];

      //request
      switch(atoi(argv[0]))
      {
        case MODE_SQL_NONE:break;
        case MODE_SQL_FULL:
          snprintf(request, MAX_LINE_SIZE,"%s%d AND session_id=%lu;",argv[1],(int)current_lang_id,current_session_id);
          sqlite3_exec(db_scan, request, callback_sqlite, &fcri, NULL);
        break;
        case MODE_SQL_LITE:
          snprintf(request, MAX_LINE_SIZE,"%s session_id=%lu;",argv[1],current_session_id);
          sqlite3_exec(db_scan, request, callback_sqlite, &fcri, NULL);
        break;
        case MODE_SQL_LOGS: //special stat for audit log
          snprintf(request, MAX_LINE_SIZE,"%s%d WHERE session_id=%lu;",argv[1],
                   (int)current_lang_id,current_session_id);
          sqlite3_exec(db_scan, request, callback_sqlite, &fcri, NULL);
        break;
      }
    }
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_GET_COLUM_COUNT:
      nb_current_columns = atoi(argv[0]);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_VIEW_CHANGE:
    case TYPE_SQLITE_FLAG_LANGUAGE_COL_CHANGE:
    {
      //column + string
      LVCOLUMN lvc;
      lvc.mask      = LVCF_TEXT;
      lvc.pszText   = convertUTF8toUTF16(argv[1], strlen(argv[1])+1);
      SendMessage(hlstv,LVM_SETCOLUMN,(WPARAM)atoi(argv[0]), (LPARAM)&lvc);
    }
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LANGUAGE_CHANGE:
    {
      unsigned int id_component = atoi(argv[1]);
      switch(atoi(argv[0]))
      {
        case TYPE_MENU:ModifyMenu(GetMenu(h_main),id_component,MF_BYCOMMAND|MF_STRING,id_component,convertUTF8toUTF16(argv[2], strlen(argv[2])+1));break;
        case TYPE_MENU_TITLE:
          ModifyMenu(GetMenu(h_main),id_component,MF_BYPOSITION|MF_STRING,id_component,convertUTF8toUTF16(argv[2], strlen(argv[2])+1));
        break;
        case TYPE_MENU_SUB_TITLE:
        {
          char menu_id[2]="\0\0";
          menu_id[0]      = argv[1][0];
          char *c = argv[1];
          id_component    = atoi(++c);
          ModifyMenu(GetSubMenu(GetMenu(h_main),atoi(menu_id)),id_component,MF_BYPOSITION|MF_STRING,id_component,convertUTF8toUTF16(argv[2], strlen(argv[2])+1));
        }
        break;
      }
    }
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LOAD_ROOTKIT_DB:
    {
      //add line
      LVITEM lvi;
      lvi.mask = LVIF_TEXT|LVIF_PARAM;
      lvi.iSubItem = 0;
      lvi.lParam = LVM_SORTITEMS;
      lvi.pszText="";
      lvi.iItem = ListView_GetItemCount(type->form);
      unsigned int i=0, ref_item = ListView_InsertItem(type->form, &lvi);

      for (i=0;i<argc;i++)
        ListView_SetItemText(type->form,ref_item,i,argv[i]);
    }
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_SESSIONS_INIT:
      if (nb_session<NB_MAX_SESSION)
      {
        session[nb_session++] = atol(argv[0]);
        SendMessage(hCombo_session, CB_INSERTSTRING,(WPARAM)-1, (LPARAM)convertUTF8toUTF16(argv[1], strlen(argv[1])+1));
      }
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_TESTS_INIT:
        SendMessage(hlstbox, LB_ADDSTRING,0,(LPARAM)convertUTF8toUTF16(argv[0], strlen(argv[0])+1));
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LANG_INIT_STRINGS:
    {
      int id_item = atoi(argv[1]);
      if (id_item< NB_COMPONENT_STRING)
      {
        strncpy(cps[id_item].c,convertUTF8toUTF16(argv[0], strlen(argv[0])+1),COMPONENT_STRING_MAX_SIZE);
      }
    }
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LANG_INIT:AddComboBoxItem(hCombo_lang, convertUTF8toUTF16(argv[0], strlen(argv[0])+1), atoi(argv[2]));break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_COLUMN_COUNT_INIT:
    {
      st_columns.nb_columns  = atoi(argv[0]);
      strncpy(st_columns.request,convertUTF8toUTF16(argv[1], strlen(argv[1])+1),REQUEST_MAX_SIZE);
      st_columns.mode_simple = atoi(argv[2]);
    }
    break;
    //----------------------------------------
    case TYPE_SQLITE_TEST:
    {
      //add line
      LVITEM lvi;
      lvi.mask = LVIF_TEXT|LVIF_PARAM;
      lvi.iSubItem = 0;
      lvi.lParam = LVM_SORTITEMS;
      lvi.pszText="";
      lvi.iItem = ListView_GetItemCount(hlstv);
      unsigned int i=0, ref_item = ListView_InsertItem(hlstv, &lvi);

      for (i=0;i<argc;i++)
        ListView_SetItemText(hlstv,ref_item,i,argv[i]);
    }
    break;
    //---------------------------------------
    case TYPE_SQLITE_FLAG_ANDROID_INIT_STRINGS:
      if (nb_sql_ANDROID>=NB_MAX_SQL_REQ)break;
      strncpy(sql_ANDROID[nb_sql_ANDROID].sql,argv[0],MAX_PATH);
      strncpy(sql_ANDROID[nb_sql_ANDROID].params,argv[1],MAX_PATH);
      strncpy(sql_ANDROID[nb_sql_ANDROID].sql_localtime,argv[3],MAX_PATH);
      sql_ANDROID[nb_sql_ANDROID].test_string_id = atoi(argv[2]);
      nb_sql_ANDROID++;
    break;
    case TYPE_SQLITE_FLAG_CHROME_INIT_STRINGS:
      if (nb_sql_CHROME>=NB_MAX_SQL_REQ)break;
      strncpy(sql_CHROME[nb_sql_CHROME].sql,argv[0],MAX_PATH);
      strncpy(sql_CHROME[nb_sql_CHROME].params,argv[1],MAX_PATH);
      strncpy(sql_CHROME[nb_sql_CHROME].sql_localtime,argv[3],MAX_PATH);
      sql_CHROME[nb_sql_CHROME].test_string_id = atoi(argv[2]);
      nb_sql_CHROME++;
    break;
    case TYPE_SQLITE_FLAG_FIREFOX_INIT_STRINGS:
      if (nb_sql_FIREFOX>=NB_MAX_SQL_REQ)break;
      strncpy(sql_FIREFOX[nb_sql_FIREFOX].sql,argv[0],MAX_PATH);
      strncpy(sql_FIREFOX[nb_sql_FIREFOX].params,argv[1],MAX_PATH);
      strncpy(sql_FIREFOX[nb_sql_FIREFOX].sql_localtime,argv[3],MAX_PATH);
      sql_FIREFOX[nb_sql_FIREFOX].test_string_id = atoi(argv[2]);
      nb_sql_FIREFOX++;
    break;
  }
  return 0;
}
//------------------------------------------------------------------------------
BOOL SQLITE_Data(FORMAT_CALBAK_READ_INFO *datas, char *sqlite_file/*not used*/, DWORD flag/*not used*/)
{
  sqlite3 *db = db_scan;
  BOOL ret = TRUE;
  char request[MAX_LINE_SIZE];

  switch(datas->type)
  {
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LANGUAGE_COL_CHANGE:
      if (current_item_selected > -1)
      {
        //column + string
        snprintf(request, MAX_LINE_SIZE,"SELECT column, string "
                                        "FROM language_strings_columns_string,language_strings_columns_settings "
                                        "WHERE language_strings_columns_string.id_language=%d AND language_strings_columns_settings.id_item=language_strings_columns_string.id_item AND ord=%d;",(int)current_lang_id,current_item_selected);
        sqlite3_exec(db, request, callback_sqlite, datas, NULL);
      }
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_GET_COLUM_COUNT:
      snprintf(request, MAX_LINE_SIZE,"SELECT count FROM language_strings_columns_settings WHERE ord = %d;",current_item_selected);
      sqlite3_exec(db, request, callback_sqlite, datas, NULL);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_GET_ITEMS_INFO:
      snprintf(request, MAX_LINE_SIZE,"SELECT mode,request FROM language_strings_columns_settings WHERE ord = %d;",current_item_selected);
      sqlite3_exec(db, request, callback_sqlite, datas, NULL);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_VIEW_CHANGE:    //select results from data base
      if (current_item_selected < 0)break;

      //clean la liste view + treeview
      ListView_DeleteAllItems(hlstv);

      //delete all header in the listeview
      while (ListView_DeleteColumn(hlstv,1));

      //define size for all columns and create it
      RECT Rect;
      GetWindowRect(hlstv, &Rect);

      //set column
      LVCOLUMN lvc;
      lvc.mask = LVCF_TEXT|LVCF_WIDTH|LVCF_FMT|LVCF_SUBITEM;
      lvc.fmt = LVCFMT_LEFT;
      lvc.cx = (Rect.right-Rect.left-40)/nb_current_columns;
      lvc.pszText = "";

      unsigned int i;
      for (i=0;i<nb_current_columns;i++)
      {
        SendMessage(hlstv,LVM_INSERTCOLUMN,(WPARAM)i, (LPARAM)&lvc);
      }
      ListView_DeleteColumn(hlstv,nb_current_columns); // for clean the last column in modify!

      //set column title
      snprintf(request, MAX_LINE_SIZE,"SELECT column, string "
                                      "FROM language_strings_columns_string,language_strings_columns_settings "
                                      "WHERE language_strings_columns_string.id_language=%d AND language_strings_columns_settings.id_item=language_strings_columns_string.id_item AND ord=%d;",(int)current_lang_id,current_item_selected);
      sqlite3_exec(db, request, callback_sqlite, datas, NULL);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_COLUMN_COUNT_INIT:
        snprintf(request, MAX_LINE_SIZE,"SELECT id_item,count,request,mode,type "
                                        "FROM language_strings_columns_settings;");
        sqlite3_exec(db, request, callback_sqlite, datas, NULL);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LANGUAGE_CHANGE:
        snprintf(request, MAX_LINE_SIZE,"SELECT language_components.type,language_components.id_component,language_components.string,language_components.id_language,language.name,language.id "
                                        "FROM language, language_components "
                                        "WHERE language.id =%d AND language.id = language_components.id_language;",(int)current_lang_id);
        sqlite3_exec(db, request, callback_sqlite, datas, NULL);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_SESSIONS_INIT:
      sqlite3_exec(db, "SELECT id,name FROM session;", callback_sqlite, datas, NULL);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_TESTS_INIT:
        snprintf(request, MAX_LINE_SIZE,"SELECT language_strings.string,language_strings.id_language,language.name,language.id,language_strings.id_item "
                                        "FROM language, language_strings, language_strings_columns_settings "
                                        "WHERE language.id =%d AND language.id = language_strings.id_language AND language_strings_columns_settings.id_item = language_strings.id_item ORDER BY language_strings_columns_settings.ord;",(int)current_lang_id);
        sqlite3_exec(db, request, callback_sqlite, datas, NULL);
    break;
    case TYPE_SQLITE_FLAG_LANG_INIT_STRINGS:
        snprintf(request, MAX_LINE_SIZE,"SELECT language_strings_load.string,language_strings_load.id_item "
                                        "FROM language, language_strings_load "
                                        "WHERE language.id =%d AND language.id = language_strings_load.id_language ORDER BY id_item;",(int)current_lang_id);
        sqlite3_exec(db, request, callback_sqlite, datas, NULL);
    break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LANG_INIT:sqlite3_exec(db, "SELECT name, id, index_img FROM language;", callback_sqlite, datas, NULL);break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_ANDROID_INIT_STRINGS:sqlite3_exec(db, "SELECT sql_utc, params, id_tests_string, sql FROM extract_android_request;", callback_sqlite, datas, NULL);break;
    case TYPE_SQLITE_FLAG_CHROME_INIT_STRINGS:sqlite3_exec(db, "SELECT sql_utc, params, id_tests_string, sql FROM extract_chrome_request;", callback_sqlite, datas, NULL);break;
    case TYPE_SQLITE_FLAG_FIREFOX_INIT_STRINGS:sqlite3_exec(db, "SELECT sql_utc, params, id_tests_string, sql FROM extract_firefox_request;", callback_sqlite, datas, NULL);break;
    //----------------------------------------
    case TYPE_SQLITE_FLAG_LOAD_ROOTKIT_DB:sqlite3_exec(db, "SELECT filename,sha256,description,source,update_time,params FROM malware_file_list;", callback_sqlite, datas, NULL);break;
    //----------------------------------------

    //****************************************
    //delete
    case TYPE_SQL_REMOVE_SESSION:
    {
      snprintf(request, MAX_LINE_SIZE,"DELETE from session WHERE id=%lu;"
                                      "DELETE from extract_IE WHERE session_id=%lu;"
                                      "DELETE from extract_android WHERE session_id=%lu;"
                                      "DELETE from extract_antivirus WHERE session_id=%lu;"
                                      "DELETE from extract_arp WHERE session_id=%lu;"
                                      "DELETE from extract_chrome WHERE session_id=%lu;"
                                      "DELETE from extract_clipboard WHERE session_id=%lu;"
                                      "DELETE from extract_disk WHERE session_id=%lu;"
                                      "DELETE from extract_env WHERE session_id=%lu;"
                                      "DELETE from extract_file WHERE session_id=%lu;"
                                      "DELETE from extract_firefox WHERE session_id=%lu;"
                                      "DELETE from extract_firewall WHERE session_id=%lu;"
                                      "DELETE from extract_guide WHERE session_id=%lu;"
                                      "DELETE from extract_host WHERE session_id=%lu;"
                                      "DELETE from extract_log WHERE session_id=%lu;"
                                      "DELETE from extract_network WHERE session_id=%lu;"
                                      "DELETE from extract_pipe WHERE session_id=%lu;"
                                      "DELETE from extract_policy WHERE session_id=%lu;"
                                      "DELETE from extract_process WHERE session_id=%lu;"
                                      "DELETE from extract_route WHERE session_id=%lu;"
                                      "DELETE from extract_share WHERE session_id=%lu;"
                                      "DELETE from extract_tache WHERE session_id=%lu;"
                                      "DELETE from extract_registry_account_password WHERE session_id=%lu;"
                                      "DELETE from extract_registry_mru WHERE session_id=%lu;"
                                      "DELETE from extract_registry_shell_bags WHERE session_id=%lu;"
                                      "DELETE from extract_registry_path WHERE session_id=%lu;"
                                      "DELETE from extract_registry_service_driver WHERE session_id=%lu;"
                                      "DELETE from extract_registry_settings WHERE session_id=%lu;"
                                      "DELETE from extract_registry_software WHERE session_id=%lu;"
                                      "DELETE from extract_registry_start WHERE session_id=%lu;"
                                      "DELETE from extract_registry_update WHERE session_id=%lu;"
                                      "DELETE from extract_registry_usb WHERE session_id=%lu;"
                                      "DELETE from extract_registry_user WHERE session_id=%lu;"
                                      "DELETE from extract_registry_userassist WHERE session_id=%lu;"
                                      "DELETE from extract_prefetch WHERE session_id=%lu;"
                                      "DELETE from extract_registry_deleted_keys WHERE session_id=%lu;"
                                      "DELETE from extract_file_nk WHERE session_id=%lu;"
                                      "DELETE from extract_ldap WHERE session_id=%lu;"
                                      ,current_session_id,current_session_id,current_session_id,current_session_id,current_session_id
                                      ,current_session_id,current_session_id,current_session_id,current_session_id,current_session_id
                                      ,current_session_id,current_session_id,current_session_id,current_session_id,current_session_id
                                      ,current_session_id,current_session_id,current_session_id,current_session_id,current_session_id
                                      ,current_session_id,current_session_id,current_session_id,current_session_id,current_session_id
                                      ,current_session_id,current_session_id,current_session_id,current_session_id,current_session_id
                                      ,current_session_id,current_session_id,current_session_id,current_session_id,current_session_id
                                      ,current_session_id,current_session_id,current_session_id);
      sqlite3_exec(db,request, callback_sqlite, datas, NULL);
    }
    break;
    //----------------------------------------
    case TYPE_SQL_REMOVE_ALL_SESSION:
      sqlite3_exec(db,"DELETE from session;"
                      "DELETE from extract_IE;"
                      "DELETE from extract_android;"
                      "DELETE from extract_antivirus;"
                      "DELETE from extract_arp;"
                      "DELETE from extract_chrome;"
                      "DELETE from extract_clipboard;"
                      "DELETE from extract_disk;"
                      "DELETE from extract_env;"
                      "DELETE from extract_file;"
                      "DELETE from extract_firefox;"
                      "DELETE from extract_firewall;"
                      "DELETE from extract_guide;"
                      "DELETE from extract_host;"
                      "DELETE from extract_log;"
                      "DELETE from extract_network;"
                      "DELETE from extract_pipe;"
                      "DELETE from extract_policy;"
                      "DELETE from extract_process;"
                      "DELETE from extract_route;"
                      "DELETE from extract_share;"
                      "DELETE from extract_tache;"
                      "DELETE from extract_registry_account_password;"
                      "DELETE from extract_registry_mru;"
                      "DELETE from extract_registry_shell_bags;"
                      "DELETE from extract_registry_path;"
                      "DELETE from extract_registry_service_driver;"
                      "DELETE from extract_registry_settings;"
                      "DELETE from extract_registry_software;"
                      "DELETE from extract_registry_start;"
                      "DELETE from extract_registry_update;"
                      "DELETE from extract_registry_usb;"
                      "DELETE from extract_registry_user;"
                      "DELETE from extract_registry_userassist;"
                      "DELETE from extract_prefetch;"
                      "DELETE from extract_registry_deleted_keys;"
                      "DELETE from extract_file_nk;"
                      "DELETE from extract_ldap;"
                      , callback_sqlite, datas, NULL);
    break;
    //----------------------------------------

    //****************************************
    //add
    case TYPE_SQL_ADD_SESSION:
    {
      AddNewSession(LOCAL_SCAN, session_name_ch, db);

      //read all sessions
      nb_session = 0;
      FORMAT_CALBAK_READ_INFO fcri;
      fcri.type  = TYPE_SQLITE_FLAG_SESSIONS_INIT;
      SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

      //select last item
      SendMessage(hCombo_session, CB_SETCURSEL,SendMessage(hCombo_session, CB_GETCOUNT,0,0)-1,0);
    }
    break;
    //----------------------------------------
    default:ret = FALSE;break;
  }
  return ret;
}
//------------------------------------------------------------------------------
BOOL SQLITE_LireData(FORMAT_CALBAK_READ_INFO *datas, char *sqlite_file)
{
  return SQLITE_Data(datas, sqlite_file, SQLITE_OPEN_READONLY);
}
//------------------------------------------------------------------------------
BOOL SQLITE_WriteData(FORMAT_CALBAK_READ_INFO *datas, char *sqlite_file)
{
  return SQLITE_Data(datas, sqlite_file, SQLITE_OPEN_READWRITE);
}
//------------------------------------------------------------------------------
void GetColumnInfo(unsigned int id)
{
  //init struct for get infos
  st_columns.mode_simple = 0;
  st_columns.nb_columns  = 0;
  st_columns.request[0]  = 0;

  //
  FORMAT_CALBAK_READ_INFO fcri;
  char request[MAX_LINE_SIZE];

  fcri.type = TYPE_SQLITE_FLAG_COLUMN_COUNT_INIT;
  snprintf(request, MAX_LINE_SIZE,"SELECT count,request,mode "
                                  "FROM language_strings_columns_settings "
                                  "WHERE ord = %d;",id);
  sqlite3_exec(db_scan, request, callback_sqlite, &fcri, NULL);
}
