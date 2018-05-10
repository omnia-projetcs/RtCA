//------------------------------------------------------------------------------
// Projet RtCA          : Read to Catch All
// Auteur               : Nicolas Hanteville
// Site                 : https://github.com/omnia-projetcs/omnia-projetcs
// Licence              : GPL V3
//------------------------------------------------------------------------------
//Userd
//------------------------------------------------------------------------------
//entête IP
typedef struct ip_hdr
{
  unsigned char  ip_header_len_version;
  /*
  unsigned char  ip_header_len:4;  // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
  unsigned char  ip_version   :4;  // 4-bit IPv4 version*/

  unsigned char  ip_tos;           // IP type of service
  unsigned short ip_total_length;  // Total length
  unsigned short ip_id;            // Unique identifier
  unsigned short flags;
  /*
  unsigned char  ip_reserved_zero :1;
  unsigned char  ip_dont_fragment :1;
  unsigned char  ip_more_fragment :1;
  unsigned int   ip_frag_offset :13;    //fragment offset*/

  unsigned char  ip_ttl;           // Time to live
  unsigned char  ip_protocol;      // Protocol(TCP,UDP etc)
  unsigned short ip_checksum;      // IP checksum
  unsigned int   ip_srcaddr;       // Source address
  unsigned int   ip_destaddr;      // Source address
}IPV4_HDR;
#define IPV4_HDR_SIZE  20
//------------------------------------------------------------------------------
// entête TCP
typedef struct tcp_header
{
	unsigned short source_port;
	unsigned short dest_port;
	unsigned int   sequence;
	unsigned int   acknowledge;

	unsigned char data_offset;
	/*
	unsigned char  ns   :1;
	unsigned char  reserved_part1:3;
	unsigned char  data_offset:4;*/

	unsigned char flags;
	/*
	unsigned char  fin  :1;      //Finish Flag
	unsigned char  syn  :1;      //Synchronise Flag
	unsigned char  rst  :1;      //Reset Flag
	unsigned char  psh  :1;      //Push Flag
	unsigned char  ack  :1;      //Acknowledgement Flag
	unsigned char  urg  :1;      //Urgent Flag
	unsigned char  ecn  :1;      //ECN-Echo Flag
  unsigned char  cwr  :1;      //Congestion Window Reduced Flag*/

	////////////////////////////////
	unsigned short window;          // window
	unsigned short checksum;        // checksum
	unsigned short urgent_pointer;  // urgent pointer
} TCP_HDR;
#define TCP_HDR_SIZE  20
//------------------------------------------------------------------------------
// entête UDP
typedef struct udp_hdr
{
  unsigned short source_port;     // Source port no.
  unsigned short dest_port;       // Dest. port no.
  unsigned short udp_length;      // Udp packet length
  unsigned short udp_checksum;    // Udp checksum (optional)
}UDP_HDR;
#define UDP_HDR_SIZE 8
//------------------------------------------------------------------------------
// entête ICMP
//http://www.frameip.com/entete-icmp/
typedef struct icmp_hdr
{
  unsigned char type;
  unsigned char code;
  unsigned short checksum;
  unsigned short id;
  unsigned short seq;
}ICMP_HDR;
#define ICMP_HDR_SIZE 10
//------------------------------------------------------------------------------
//tailles spécifiques
#define TAILLE_PAQUET_ARP_REPLY                   60
#define TAILLE_PAQUET_ARP_REQUEST                 42
#define TAILLE_PAQUET_TCP                         58
#define TAILLE_PAQUET_UDP                         42//14+20+8
#define TAILLE_PAQUET_ICMP                        74

//------------------------------------------------------------------------------
//FLAG
#define FLAG_FIN                                  1
#define FLAG_SYN                                  2
#define FLAG_RST                                  4

#define FLAG_PSH                                  8
#define FLAG_ACK                                  16
#define FLAG_URG                                  32
#define FLAG_ECN                                  64
#define FLAG_CWR                                 128
//------------------------------------------------------------------------------
// entête IGMP
typedef struct igmp_hdr
{
    char type;
    char Temp_reponse;
    unsigned short checksum;
    unsigned int ip_groupe;
}   IGMP_HDR;
#define IGMP_HDR_SIZE 8
//------------------------------------------------------------------------------
//not used
//------------------------------------------------------------------------------
typedef struct ip_hdr6
{
  unsigned char  ip_header_len:4;     // 4-bit header length (in 32-bit words) normally=5 (Means 20 Bytes may be 24 also)
  unsigned char  ip_Traffic_Class;    // 8-bits Internet traffic priority delivery value
  unsigned int   ip_Flow_Label:20;    // 20-bits Used for specifying special router handling from source to destination(s) for a sequence of packets.
  unsigned int   ip_Payload_Length:16;// 16-bits Specifies the length of the data in the packet. When cleared to zero, the option is a hop-by-hop Jumbo payload.
  unsigned char  ip_Next_Header;      // 8-bits specifies the next encapsulated protocol. The values are compatible with those specified for the IPv4 protocol field.
  unsigned char  ip_Hop_Limit;        // 8-bits For each router that forwards the packet, the hop limit is decremented by 1. When the hop limit field reaches zero, the packet is discarded.
                                      //        This replaces the TTL field in the IPv4 header that was originally intended to be used as a time based hop limit.
  unsigned int   ip_srcaddr;          // 32-bits adresse de source 0000:0000:0000:0000
  unsigned int   ip_destaddr;         // 32-bits adresse de destination 0000:0000:0000:0000
}IPV6_HDR;

//------------------------------------------------------------------------------
//ARP
typedef struct arp_hdr
{
  unsigned char  h_dest[6];     // Destination host address  //adresse MAC
  unsigned char  h_source[6];   // Source host address
  unsigned short net_type;      //type de protocole (ICMP, ARP, ...)
  unsigned char  hard_type[2];
  unsigned char  protocol_type[2]; // 0x06 = IP
  unsigned char  hard_length;
  unsigned char  logic_length;
  unsigned char  operation[2];

  unsigned char  _h_source[6];   // Source host address
  unsigned char  ip_srcaddr[4]; // Source address
  unsigned char  _h_dest[6];     // Destination host address  //adresse MAC
  unsigned char  ip_destaddr[4];// Destination address
}  ARP_HDR;
//------------------------------------------------------------------------------
//IPX
//http://www.networksorcery.com/enp/protocol/ipx.htm
typedef struct ipx_hdr
{
  unsigned short checksum;          //toujours 0xFFFF
  unsigned short Packet_Length;
  unsigned char Transport_control;  //equivalent au TTL
  unsigned char type;               //0x00	Unknown.
                                    //0x01 	RIP, Routing Information Packet.
                                    //0x02 	Echo Packet.
                                    //0x03 	Error Packet.
                                    //0x04 	PEP, Packet Exchange Protocol.
                                    //0x05 	SPX, Sequenced Packet Protocol.
                                    //0x10-0x1F 	Experimental Protocols.
                                    //0x11 	NCP, NetWare Core Protocol.
                                    //0x12-0xFF  ???
  unsigned int Destination_network; //sous réseau de destination
  unsigned long long int Destination_node:48; //adresse mac du destinataire
  unsigned short Destination_socket;

  unsigned int Source_network;
  unsigned long long int Source_node:48;
  unsigned short Source_socket;     //0x01	RIP, Routing Information Packet.
                                    //0x02	Echo Protocol Packet.
                                    //0x03 	Error Handling Packet.
                                    //0x20-0x3F 	Experimental.
                                    //0x40-0xBB8 	Xerox registered.
                                    //0xBB9 	Dynamically Assigned.
                                    //0x451 	NCP, NetWare Core Protocol.
                                    //0x452 	SAP, Service Advertising Protocol.
                                    //0x453 	RIP, Routing Information Protocol.
                                    //0x455 	NetBios.
                                    //0x456 	Diagnostic.
                                    //0x457 	Serialization.
                                    //0x8060 	IPX.
                                    //0x9091 	TCP over IPXF.
                                    //0x9092 	UDP over IPXF.
                                    //0x9093 	IPXF, IPX Fragmentation Protocol.
  //data
}IPX_HDR;
//------------------------------------------------------------------------------

//DHCP IPV6
typedef struct dhcp_ipv6_hdr
{
  unsigned char message;
  //data ...
  //http://www.networksorcery.com/enp/protocol/dhcpv6.htm

}DHCP_IPV6_HDR;
//------------------------------------------------------------------------------
//DHCP
typedef struct dhcp_hdr
{
  unsigned char Opcode;                  // 1 = requête;  2 = réponse
  unsigned char Hardware_type;           // http://www.networksorcery.com/enp/protocol/dhcp.htm
  unsigned char Hardware_address_length;
  unsigned char Hop_count;               // équivalent au TTL
  unsigned int Transaction_ID;           // numéro aléatoire d'identification
  unsigned short Number_of_seconds;      // temp en seconde depuis la demande

  //flags sur 16bits
  unsigned char Broadcast:1;
  unsigned short flags:15;

  //adresses
  unsigned int client_ip;
  unsigned int your_ip;
  unsigned int serveur_ip;
  unsigned int gateway_ip;
  unsigned short mac_adresse_client;
  unsigned long long int Server_host_name;
  unsigned char Boot_filename[12];
  //options ...
  //http://www.networksorcery.com/enp/protocol/bootp/options.htm

}DHCP_HDR;
//------------------------------------------------------------------------------
//entête DNS (après ethernet + IP + TCP/UDP)
typedef struct dns_hdr
{
  unsigned short transaction_ID;           //Identification
  //flags
  unsigned char  Query_Response   :1;      //QR (1bit) [Query/Response] : 0 = demande; 1 = réponse
  unsigned char  Opcode           :4;      //Opcode (4 bits) : 0 = recherche standard; 1 = recherche inverse ; 2 = état du serveur ; 3 = réservé;
                                           // 4 = notification, 5 = mise à jour, 6-15 = réservés
  unsigned char  Authorite_res    :1;      //AA (1bit) [Authoritative response] : le serveur de noms est celui qui a
                                           //autorité sur le domaine.
  unsigned char  Truncated        :1;      //TC (1bit) [Truncated] : Le réponse totale est plus longue (>512 octets)
                                           //que celle contenue dans le message
  unsigned char  Recursion_Desired:1;      //RD (1bit) [Recursion desired] : le demandeur demande au serveur de noms d'effectuer,
                                           //si nécessaire, une recherche récursive. Ce serveur va s'occuper des recherches auprès
                                           //d'autres serveurs. Sinon la recherche est itérative : le demandeur devra lui-même
                                           //interroger les serveurs dont le nom figurera dans la réponse.
  unsigned char  Recursion_Available:1;    //indique si les requêtes récursives sont suportés par le serveur (peut donner des idées pour le transfert de zone)
  unsigned char  reserved:1;
  unsigned char  authentification_data:1;  //0: non, 1:oui
  unsigned char  Checking_Disabled:1;       //0: vérification activé / 1:non activé
  unsigned char  Return_code:4;            //

  unsigned short question_count;
  unsigned short answer_count;
  unsigned short authority_count;
  unsigned short Additionnel_Count;

  //requête
  //int *queries;  // les données sont une chaine qui représente la requête
                 // les 4 derniers octets représentent: = type standard query
                 //   - (2premiers) : le type de pointeur (PTR = domaine name pointeur)
                 //   - (2derniers) : le type de class : IN = 0x0001
}DNS_HDR;

//------------------------------------------------------------------------------
//pour ping ICMP
typedef struct tagIPINFO
{
	unsigned char Ttl;
	unsigned char Tos;
	unsigned char Flags;
  	unsigned char OptionsSize;
  	unsigned char OptionsData;
}IPINFO;
typedef IPINFO* PIPINFO;

typedef struct tagICMPECHO
{
     DWORD Address;
     unsigned long  Status,RoundTripTime;
     unsigned char a[8];
     IPINFO  Options;
}ICMPECHO;
//------------------------------------------------------------------------------

