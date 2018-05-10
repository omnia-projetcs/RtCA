//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
#include "../RtCA.h"
//------------------------------------------------------------------------------
BOOL BackupSQLITESession(char *src, char *dst)
{
  sqlite3 *dst_file, *src_file;
  if (sqlite3_open(dst, &dst_file) != SQLITE_OK)
  {
    return FALSE;
  }

  if (sqlite3_open(src, &src_file) != SQLITE_OK)
  {
    sqlite3_close(dst_file);
    return FALSE;
  }

  //backup
  BOOL ret = FALSE;
  sqlite3_backup *pBackup = sqlite3_backup_init(dst_file, "main", src_file, "main");
  if (pBackup)
  {
    sqlite3_backup_step(pBackup, -1);
    sqlite3_backup_finish(pBackup);
    ret = TRUE;
  }

  //close
  sqlite3_close(dst_file);
  sqlite3_close(src_file);

  return ret;
}
//------------------------------------------------------------------------------
BOOL SQLITE_LoadSession(char *file)
{
  //backup file first
  SQLITE_SaveSession("RtCA.sqlite.back");

  //BOOL ret = BackupSQLITESession(file, SQLITE_LOCAL_BDD);
  char request[MAX_LINE_SIZE];
  snprintf(request,MAX_LINE_SIZE,"ATTACH '%s' AS toMerge;",file);
  sqlite3_exec(db_scan,request, NULL, NULL, NULL);

  sqlite3_exec(db_scan,"INSERT INTO session SELECT * FROM toMerge.session;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_IE (file,parameter,data,id_language_description,session_id,date)"
                        " SELECT file,parameter,data,id_language_description,session_id,date FROM toMerge.extract_IE;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_android (file,parameter,data,id_language_description,session_id,date)"
                        " SELECT file,parameter,data,id_language_description,session_id,date FROM toMerge.extract_android;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_antivirus (path,name,editor,engine,bdd,url_update,enable,last_update,session_id)"
                        " SELECT path,name,editor,engine,bdd,url_update,enable,last_update,session_id FROM toMerge.extract_antivirus;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_arp (ip,mac,type,session_id)"
                        " SELECT ip,mac,type,session_id FROM toMerge.extract_arp;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_chrome (file,parameter,data,id_language_description,session_id,date)"
                        " SELECT file,parameter,data,id_language_description,session_id,date FROM toMerge.extract_chrome;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_firefox (file,parameter,data,id_language_description,session_id,date)"
                        " SELECT file,parameter,data,id_language_description,session_id,date FROM toMerge.extract_firefox;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_clipboard (format,data,session_id,user,code,description)"
                        " SELECT format,data,session_id,user,code,description FROM toMerge.extract_clipboard;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_disk (drive,type,filesystem,freespace,globalspace,session_id,name)"
                        " SELECT drive,type,filesystem,freespace,globalspace,session_id,name FROM toMerge.extract_disk;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_env (string,session_id,user,source)"
                        " SELECT string,session_id,user,source FROM toMerge.extract_env;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_file (path,file,extension,Create_time,Modify_time,Access_Time,Owner,ACL,Hidden,System,Archive,Encrypted,Tempory,SAH256,Size,ADS,VirusTotal,Description,session_id,SID,RID)"
                        " SELECT path,file,extension,Create_time,Modify_time,Access_Time,Owner,ACL,Hidden,System,Archive,Encrypted,Tempory,SAH256,Size,ADS,VirusTotal,Description,session_id,SID,RID FROM toMerge.extract_file;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_file_nk (file,create_time,last_access_time,last_modification_time,local_path,to_,session_id)"
                        " SELECT file,create_time,last_access_time,last_modification_time,local_path,to_,session_id FROM toMerge.extract_file_nk;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_firewall (file,session_id,rule,hk,key,application)"
                        " SELECT file,session_id,rule,hk,key,application FROM toMerge.extract_firewall;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_guide (file,key,value,data_read,title_id,description_id,ok_id,session_id,hk,data)"
                        " SELECT file,key,value,data_read,title_id,description_id,ok_id,session_id,hk,data FROM toMerge.extract_guide;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_host (file,ip,name,last_file_update,session_id,malware_check)"
                        " SELECT file,ip,name,last_file_update,session_id,malware_check FROM toMerge.extract_host;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_ldap (dit,value,dn,data,session_id)"
                        " SELECT dit,value,dn,data,session_id FROM toMerge.extract_ldap;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_log (event,indx,log_id,send_date,write_date,source,description,user,sid,critical,session_id,rid,state)"
                        " SELECT event,indx,log_id,send_date,write_date,source,description,user,sid,critical,session_id,rid,state FROM toMerge.extract_log;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_network (card,description,ip,netmask,gateway,dns,domain,dhcp_mode,dhcp_server,wifi,last_update,session_id,guid,source)"
                        " SELECT card,description,ip,netmask,gateway,dns,domain,dhcp_mode,dhcp_server,wifi,last_update,session_id,guid,source FROM toMerge.extract_network;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_pipe (pipe,owner,rid,sid,session_id)"
                        " SELECT pipe,owner,rid,sid,session_id FROM toMerge.extract_pipe;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_policy (source,type_id,description_id,data,session_id)"
                        " SELECT source,type_id,description_id,data,session_id FROM toMerge.extract_policy;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_prefetch (file,create_time,last_update,session_id,last_access,count,exec,path,depend)"
                        " SELECT file,create_time,last_update,session_id,last_access,count,exec,path,depend FROM toMerge.extract_prefetch;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_process (process,pid,path,cmd,owner,rid,sid,start_date,protocol,ip_src,port_src,ip_dst,port_dst,state,session_id,hidden,parent_process,parent_pid)"
                        " SELECT process,pid,path,cmd,owner,rid,sid,start_date,protocol,ip_src,port_src,ip_dst,port_dst,state,session_id,hidden,parent_process,parent_pid FROM toMerge.extract_process;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_route (destination,netmask,gateway,metric,session_id)"
                        " SELECT destination,netmask,gateway,metric,session_id FROM toMerge.extract_route;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_share (share,path,description,connexion,session_id,type,file)"
                        " SELECT share,path,description,connexion,session_id,type,file FROM toMerge.extract_share;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_tache (id_ev,type,data,session_id,next_run,create_date,update_date,details,access_date,user)"
                        " SELECT id_ev,type,data,session_id,next_run,create_date,update_date,details,access_date,user FROM toMerge.extract_tache;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_account_password (source,login,password,description_id,session_id,raw_password)"
                        " SELECT source,login,password,description_id,session_id,raw_password FROM toMerge.extract_registry_account_password;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_deleted_keys (source,key,value,data,type,sid,last_update,session_id)"
                        " SELECT source,key,value,data,type,sid,last_update,session_id FROM toMerge.extract_registry_deleted_keys;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_mru (file,key,value,data,description_id,parent_key_update,user,rid,sid,session_id,hk)"
                        " SELECT file,key,value,data,description_id,parent_key_update,user,rid,sid,session_id,hk FROM toMerge.extract_registry_mru;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_shell_bags (source,key,value,data,sid,last_update,session_id)"
                        " SELECT * FROM toMerge.extract_registry_shell_bags;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_path (file,key,value,data,user,rid,sid,session_id,hk,parent_key_update)"
                        " SELECT file,key,value,data,user,rid,sid,session_id,hk,parent_key_update FROM toMerge.extract_registry_path;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_service_driver (file,key,name,state_id,path,description,type_id,last_update,session_id,hk)"
                        " SELECT file,key,name,state_id,path,description,type_id,last_update,session_id,hk FROM toMerge.extract_registry_service_driver;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_software (file,key,name,publisher,uninstallstring,installlocation,install_date_update,install_user,url,source,valid,session_id,hk)"
                        " SELECT file,key,name,publisher,uninstallstring,installlocation,install_date_update,install_user,url,source,valid,session_id,hk FROM toMerge.extract_registry_software;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_settings (file,key,value,data,type_id,description_id,parent_key_update,session_id,hk)"
                        " SELECT file,key,value,data,type_id,description_id,parent_key_update,session_id,hk FROM toMerge.extract_registry_settings;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_start (file,key,value,data,last_parent_key_update,session_id,hk)"
                        " SELECT file,key,value,data,last_parent_key_update,session_id,hk FROM toMerge.extract_registry_start;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_update (file,key,component,name,publisher,description_package_name,install_date_update,session_id,hk)"
                        " SELECT file,key,component,name,publisher,description_package_name,install_date_update,session_id,hk FROM toMerge.extract_registry_update;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_usb (file,key,name,vendor_id,product_id,description,last_use,session_id,hk,lecteur,pusb)"
                        " SELECT file,key,name,vendor_id,product_id,description,last_use,session_id,hk,lecteur,pusb FROM toMerge.extract_registry_usb;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_user (name,RID,SID,description,grp,last_logon,last_password_change,nb_connexion,state_id,type,session_id,source)"
               " SELECT name,RID,SID,description,grp,last_logon,last_password_change,nb_connexion,state_id,type,session_id,source FROM toMerge.extract_registry_user;", NULL, NULL, NULL);
  sqlite3_exec(db_scan, "INSERT INTO extract_registry_userassist (file,key,raw_type,type_id,path,use_count,session_number,last_use,user,RID,SID,session_id,hk,time)"
               " SELECT file,key,raw_type,type_id,path,use_count,session_number,last_use,user,RID,SID,session_id,hk,time FROM toMerge.extract_registry_userassist;", NULL, NULL, NULL);

  sqlite3_exec(db_scan, "INSERT INTO malware_file_list (filename,sha256,description,source,update_time,params)"
               " SELECT filename,sha256,description,source,update_time,params FROM toMerge.malware_file_list;", NULL, NULL, NULL);

  sqlite3_exec(db_scan,"DETACH DATABASE toMerge;", NULL, NULL, NULL);

  //recharger la liste des sessions
  FORMAT_CALBAK_READ_INFO fcri;
  fcri.type = TYPE_SQLITE_FLAG_SESSIONS_INIT;
  SQLITE_LireData(&fcri, SQLITE_LOCAL_BDD);

  SendMessage(hCombo_session, CB_SETCURSEL,SendMessage(hCombo_session, CB_GETCOUNT,0,0)-1,0);
  return FALSE;
}
//------------------------------------------------------------------------------
BOOL SQLITE_SaveSession(char *file)
{
  BOOL ret = BackupSQLITESession(SQLITE_LOCAL_BDD, file);
  return ret;
}
