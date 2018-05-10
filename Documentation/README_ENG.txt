************************************************************************
** RTCA README                                                        **
************************************************************************

# Introduction
  RTCA (to Read to Catch All) is a multi-language intended for operate 
  on Windows and Linux Windows emulator (Wine).
  It can be used in console mode or GUI (default).
  The concept of the tool allow the extraction of local configuration
  of Windows, Unix/Linux log and Android files on SQLITE databases.
  RTCA has been developed in C by Nicolas Hanteville, compiled with
  codeblocks. His license is GPLv3. If some of the application components
  were extracted from other codes, it is commented in the code.


# Compatibility
  The operation and configuration of the extraction is provided for systems
  Windows XP, 7, 8, 2003 and 2008 x86/x64 and Wine.
  Formats usable audit logs are : Windows / Unix / Linux
  Data files database Android phones can be extracted.
  Windows prefetch files, and registry RAW files, job are too exploitable.


# Features
  The console appears to be more limited than the GUI mode. Indeed,
  Many tools are integrated in the GUI application and not used in console 
  mode.


# Using

  # # Operating Environment
    The application is designed to work as "online" and "offline" on files 
    or system configuration. Extraction of configuration from a Windows 
    machine can be done "online" and "offline", but the analysis file from 
    Android requires to extraction thereof from the device.

  ## Browsing results
    The consultation can be done on the main page of the application
    selecting the session and module. A search keyword is activated and 
    the extraction results is possible.

  ## Export results
    Test results are saved in a database SQLITE, they can be exported to : 
      * CSV (data framed by quotation marks, the separator is a semicolon)
      * HTML (tabular)
      * XML (as structured by type)
      * Pwdump (for fingerprints passwords, usable format directly by cracking tools)

  ## Tests
    Many elements are supported by the application in order to extract max 
    of information. To run, you must create a new session menu File-> New session 
    then in the new window is created, it is necessary to select the tests 
    to activate and add appropriate file to check to the list 
    (a function to auto search for files to include is present).

    ### Files and directories
      Allows you to retrieve a list of files on the machine and their 
      meta-datas such as ACL and ADS. The option generation and verification 
      sha256 footprint in the database site https://www.virustotal.com.

    ### Audit Logs
      The audit log files are taken into account:
        * Files *. Evt systems before Windows Vista
        * Files *. Evtx systems from Windows Vista
        * Files *. System log in Windows
        * Audit log format Linux / Unix

    ### Disks
      List of drives, size and file system on the system.

    ### Clipboard
      Binary and textual content of the clipboard.

    ### Local Variables
      List of local variables and current contained in the database
      register.

    ### Scheduled Tasks
      Windows file content *. Job.

    ### Process
      To list the current processes and associated network ports.
      A test routine checks if the threads and processes inputs
      correspond, hidden process can be found.

    ### Pipes
      Pipes on the system.

    ### Network
      Network cards configuration.

    ### Routing Table
      Routes implemented on the machine

    ### DNS Resolution
      DNS cache with content checking if DNS names know as malware.

    ### ARP Cache
      Contents of the ARP cache, allows the detection in case of attack type
      "Man in the Middle" by ARP cache poisoning.

    ### Shares
      List of network shares. These items can also be extracted from the 
      registry files (SYSTEM, etc.)..

    ### Register
      List of configuration items present in the registry, whose historical 
      users, passwords, list of updates, etc. These items can also be 
      extracted from files register (SAM, SYSTEM, etc.).

    ### Antivirus
      Configuring Antivirus on the machine. These elements can
      also be extracted from the registry files (SOFTWARE, SYSTEM, etc.)..

    ### Firewall
      List of exclusion and inclusion Windows Firewall rules.
      These items can also be extracted from the registry files
      (SOFTWARE, SYSTEM, etc.)..

    ### Firefox, Chrome, IE
      Browsing history, forms, passwords, etc..
      Can be directly extracted files. * Db *. Sqlite *. DAT.

    ### Android
      Browsing history, forms, passwords, etc..
      Can be extracted only from sqlite files. * Db Android.

    ### Prefetch
      List of applications running on the machine can be processed from
      files. * pf.


  ## Additional Tools
    They are contained in the "Tools" menu.

    ### Copy files
      It is possible to utilize the functionality of advanced copies
      Windows operating system (access restricted files, shadow copy ...)
      to make copies of protected files. Functionality backup configuration 
      files compressed to ZIP file is also possible.

    ### Process
      To list the current processes and associated network ports.
      A test routine checks if the threads and processes inputs
      correspond. If necessary, the process is considered hidden.
      The module integrates the memory dump RAW process, injection
      DLL and DLL linked. The list is exportable and manual update is
      possible.

    ### Browser register
      This tool is a file explorer of RAW registry files (SAM, SYSTEM, 
      SECURITY, etc.) It extracts all under a tree and a table with a list 
      of key, values, the key owner and modification date. If the value has
      deleted its indicated. A search engine is included with the ability 
      to export.

    ### Network Capture Tool
      Allows capture network flow without installing driver or particular 
      module. Requires only run RTCA have rights administration. Capture 
      flow is performed RAW socket (the informations on the Ethernet layer 
      are not accessible). Extraction of the IP list and summary information 
      packets are possible. A custom view is possible with direct access 
      to each packet. It also includes automatic filters and connection tracking.

    ### Date decoder
      From allows a numeric value or hexadecimal and generate the most 
      common dates such as Microsoft FILETIME, time_t Unix, etc.

    ### Editor SQLITE
      A simple viewer file content with a field to do SQL  requests. 
      An export of results is possible.

    ### Comprehensive Analyzer
      To correlate the overall results of all sessions. With the ability 
      to filter by session and test.

    ### Print Screen
      This module can be activated and deactivated in the "Setup" menu,
      it allows you to make screen printed saved in BMP format to the application 
      directory.

# Save and export the results
  By default, the export of results is done in database SQLITE by session, 
  except for special tools. The extraction results can be CSV, XML, HTML 
  and. Pwdump. The overall backup local files when extracting configuration
  ZIP file is performed.
