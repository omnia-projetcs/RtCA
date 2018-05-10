************************************************************************
** README RTCA                                                        **
************************************************************************

#Introduction
  RtCA (to Read, to Catch All) est un programme multi-langues prévu pour 
  fonctionner sous les environnements Windows à partir d'XP et avec 
  l'émulateur Windows sous Linux nommé Wine. 
  Il est utilisable en mode console ou GUI (par défaut).
  Le concept de l'outil est de permettre l'extraction de configuration locale 
  d'une machine Windows ainsi que l'exploitation de ces données ainsi que 
  les bases de données sqlite des téléphones Android et des journaux d'audit 
  Unix/Linux.
  RtCA a été développé en langage C par Nicolas Hanteville, il est compilé 
  sous codeblocks. Sa licence est GPLv3. Certains composants de l'application 
  ont été extraits d'autres codes, les dites sources sont explicités et 
  commentés dans le code.


#Compatibilité
  L'exploitation et l'extraction de configuration est prévu pour les systèmes 
  Windows XP, 7, 8, 2003 et 2008 en x86 et x64 et même Wine.
  Les formats de journaux d'audit exploitables sont : .log Windows/Unix/Linux
  Les données des fichiers de base de données des téléphones Android peuvent 
  être extraits.
  Les fichiers Windows de prefetch, job et registre brutes sont exploitables.


#Fonctionnalités
  Le mode console s'avère être plus limité que le mode GUI. En effet, de 
  nombreux outils sont intégrés dans l'application GUI ainsi que des 
  fonctionnalités fines d'exploitation mais non présentes pour un soucis 
  de simplification sous le mode console.


#Utilisation

  ##Contexte d'exploitation 
    L'application a pour objectif de travailler de manière "online" et "offline" 
    sur des fichiers ou la configuration système.
    L'extraction de configuration d'une machine Windows peut se faire "online" 
    et "offline", mais l'analyse des fichiers pour systèmes Android nécessite 
    au préalable une extraction de ceux-ci de l'appareil.

  ##Consultation des résultats
    La consultation peut être effectuée sur la page principale de l'application 
    en sélectionnant la session et le module.
    Un moteur de recherche par mot clé est activé et l'extraction des résultats 
    est possible.

  ##Export des résultats
    Les résultats des tests sont enregistrés en base de données SQLITE, elles 
    peuvent être exportés sous les formes :
      * CSV (les données encadrées par des guillemets, le séparateur étant le point virgule)
      * HTML (sous forme de tableau)
      * XML (sous forme structurée par type)
      * PWDUMP (pour les empreintes de mots de passe, format exploitable 
      directement par les outils de cassage)

  ##Tests 
    Un grand nombre d'éléments sont pris en charge par l'application afin d'en 
    extraire un maximum d'informations.
    Pour lancer des tests, il faut créer une nouvelle session, menu 
    Fichier->Nouvelle session puis, dans la nouvelle fenêtre créé, il est 
    nécessaire de sélectionner les tests à activer, ajouter le cas échant les 
    fichiers a tester à la liste (une fonction de recherche des fichiers pris en compte 
    est présente).

    ###Fichiers et répertoires
      Permet d'extraire la liste des fichiers présents sur la machine ainsi que 
      leurs méta datas telles que les ACL, si des ADS dépendent du fichier.
      L'option de génération d'empreinte sha256 et de vérification dans la base 
      du site https://www.virustotal.com est aussi possible.
      La vérification des empreintes sha256 sur le site de https://www.virustotal.com 
      ne peut être fait qu'une fois l'extraction de la liste des fichiers et leurs 
      empreintes obtenues lors du visionnage des résultats.

    ###Journaux d'audit
      Les fichiers de journaux d'audits pris en compte sont :
        * les fichiers *.evt des systèmes avant Windows Vista
        * les fichiers *.evtx des systèmes à partir de Windows Vista
        * les fichiers *.log système de Windows 
        * les journaux d'audit au format Linux/Unix

    ###Disques
      Liste des lecteurs, taille et système de fichiers sur le système.

    ###Presse papier
      Contenu textuel et binaire du presse-papier.

    ###Variables locales
      Liste des variables locales actuelles et contenues dans la base de 
      registre.

    ###Tâches planifiées
      Contenu des fichiers Windows *.job.

    ###Processus
      Permet de lister les processus actuels et les ports réseaux associés.
      Une routine de test vérifie si les threads et entrées des processus 
      correspondent bien. Le cas échéant, le processus est considéré comme caché.

    ###Pipes
      Liste des pipes actuellement ouverts sur le système.

    ###Réseau
      Configuration des cartes réseaux.

    ###Table de routage
      Liste des routes implémentées sur la machine.

    ###Résolution DNS
      Contenu du cache DNS avec la vérification si des noms DNS appartiennent 
      à des noms de domaine de malware connus.

    ###Cache ARP
      Contenu du cache ARP, permet la détection en cas d'attaque de type 
      "Man in the Middle" par empoisonnement de cache ARP.

    ###Partages
      Liste des partages réseaux.
      Ces éléments peuvent aussi être extraits à partir des fichiers de registre 
      (SAM, SYSTEM, etc.).

    ###Registre
      Liste des éléments de configuration présents dans la base de registre, 
      dont les historiques des utilisateurs, mots de passe, liste des mises à 
      jour, etc. Ces éléments peuvent aussi être extraits à partir des fichiers 
      de registre (SAM, SYSTEM, etc.).

    ###Antivirus
      Configuration des Antivirus présents sur la machine. Ces éléments peuvent 
      aussi être extraits à partir des fichiers de registre (SAM, SYSTEM, etc.).

    ###Pare-feu
      Liste des règles du par feu Windows d'exclusion et d'inclusion.
      Ces éléments peuvent aussi être extraits à partir des fichiers de registre 
      (SAM, SYSTEM, etc.).

    ###Firefox, Chrome, IE
      Historique de navigation, formulaires, mots de passe, etc.
      Peuvent être directement extraits des fichiers *.db, *.sqlite, *.DAT.

    ###Android
      Historique de navigation, formulaires, mots de passe, etc.
      Ne peut être extrait qu'à partir des fichiers sqlite *.db d'Android.

    ###Prefetch
      Liste des applications exécutés sur la machine, peut être traité à partir 
      des fichiers *.pf.


  ##Outils complémentaires
    Ils sont contenus dans le menu "Outils".

    ###Copie de fichiers
      Il est possible d'utiliser les fonctionnalités de copies avancées du 
      système d'exploitation Windows (accès de fichiers restreints, shadow copy...) 
      afin d'effectuer des copies de fichiers protégés. Une fonctionnalité de 
      sauvegarde des fichiers de configuration importants compressés dans un 
      fichier ZIP est aussi possible.

    ###Processus
      Permet de lister les processus actuels et les ports réseaux associés.
      Une routine de test vérifie si les threads et entrées des processus 
      correspondent bien. Le cas échéant, le processus est considéré comme caché.
      Le module intègre le dump de la mémoire RAW d'un processus, l'injection 
      de DLL et les DLL liées. La liste est exportable et sa mise à jour manuelle est 
      possible.

    ###Explorateur de registre
      Cet outil est un explorateur de fichier de base de registre brute tel que 
      SAM, SYSTEM, SECURITY...
      Il extrait le tout sous forme d'arbre et de tableau avec la liste des clés, 
      valeurs, propriétaire de la clé et date de modification. Si la valeur a 
      été supprimée elle est indiquée.
      Un moteur de recherche est inclus avec la possibilité d'export.

    ###Outil de capture réseau
      Permet d'effectuer une capture de flux réseau sans installer de driver ou 
      de module particulier. Ne nécessite que d'exécuter RtCA avoir des droits 
      d'administration. La capture de flux est effectuée en RAW socket (les 
      informations sur la couche Ethernet ne sont donc pas accessibles).
      L'extraction de la liste des IP et des informations sommaire des paquets 
      est possible.
      Un affichage détaillé est possible en accès direct à chaque paquet.
      Il intègre aussi des filtres automatiques et le suivi de connexion.

    ###Décodeur de date
      Permet à partir d'une valeur numérique ou hexadécimale de générer les 
      dates les plus courantes tels que Filetime de Microsoft, time_t de Unix, 
      etc.

    ###Éditeur SQLITE
      Un simple visionneur de contenu de fichier SQLITE avec un champ pour 
      faire des requêtes. Un export des résultats est possible.

    ###Analyseur global
      Permet de corréler l'ensemble des résultats obtenus sur l'ensemble des 
      sessions. Avec la possibilité de filtrer par session et par test.

    ###Imprime écran
      Ce module peut être activé et désactivé dans le menu "Configuration", 
      il permet d'effectuer des imprimes écran sauvegardés au format BMP dans 
      le répertoire de l'application.

#Sauvegarde et export des résultats
  Par défaut l'export des résultats est effectué en base de données SQLITE 
  par session, hormis les outils spécifiques.
  L'extraction des résultats est possible en CSV, XML, HTML et .PWDUMP
  La sauvegarde globale des fichiers locaux lors d'une extraction de configuration 
  est effectué en fichier ZIP.
