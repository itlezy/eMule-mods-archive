//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

#ifndef NEW_SOCKETS_ENGINE

// Version strings
#define CURRENT_VERSION	"1.2e"

//	Version format: D0..D3 - build number, D4..D7 - minor version, D8..D14 - major version
//	NB! do not make build number greater than 7 (required for compatibility)
#define CURRENT_PLUS_VERSION	0x0125

#else //NEW_SOCKETS_ENGINE

// Version 2 strings
#define CURRENT_PLUS_VERSION	0x0200

#define CURRENT_VERSION			"2 alpha"

#endif //NEW_SOCKETS_ENGINE


#ifndef _DEBUG
	#define CURRENT_VERSION_LONG	_T(CURRENT_VERSION)
#else
	#define CURRENT_VERSION_LONG	_T(CURRENT_VERSION) _T(" DEBUG")
#endif


// Constants
#define PLUS_VERSION_STR		"Plus 1.2e"
#define WS_SERVER_TOKEN			"eMulePlus/1.2e"
#define CLIENT_NAME				_T("eMule Plus")
#define CLIENT_NAME_WITH_VER	CLIENT_NAME _T(" v") CURRENT_VERSION_LONG
#define PLUS_COMPATIBLECLIENTID	5
#define RSAKEYSIZE				384		//384 bits
#define HTTP_USERAGENT			_T("Mozilla/4.0 (compatible; MSIE 6.0; Windows NT 5.0)")

#define CURRENT_VERSION_SHORT	0x43
#define EMULE_PROTOCOL			0x01

#define EDONKEYVERSION			0x3c
#define PREFFILE_VERSION		0x14
#define PARTFILE_VERSION		0xe0
#define PARTFILE_VERSION_LARGEFILE	0xe2
#define SOURCEEXCHANGE1_VERSION	3	// P2P Source Exchange 1 protocol version (below 4 to stop spreading wrong data by buggy 0.47c)
#define SOURCEEXCHANGE2_VERSION	4	// Replaces the version sent in MISC_OPTIONS flag for SX1

#define CREDITFILE_VERSION		0x12
#define CREDITFILE_VERSION_29	0x11
#ifdef _DEBUG
#define EMULE_GUID				_T("EMULE-{4EADC6FC-516F-4b7c-9066-97D893649569}-DEBUG")
#else
#define EMULE_GUID				_T("EMULE-{4EADC6FC-516F-4b7c-9066-97D893649569}")
#endif

#define SEC2MS(sec)				((sec)*1000)
#define MIN2MS(min)				SEC2MS((min)*60)
#define MIN2S(min)				((min)*60)

#define	LANCASTFILEREASKTIME	15000	//	LANCAST File re-ask (UDP) time for LAN clients is 15 sec 
										//	if UDP request failed TCP the file will be reasked in 30 sec

// MOD Note: Do not change this part - Merkur
#define MAX_RESULTS				100 	// Max global search results
#define MAX_MORE_SEARCH_REQ		5		// This gives a max. total search results of (1+5)*201 = 1206 or (1+5)*300 = 1800
#define MAX_CLIENTCONNECTIONTRY	2
#define CONNECTION_TIMEOUT		40000	// Set this lower if you want less connections at once, 
										// set it higher if you have enough sockets 
										// (eDonkey has its own timout too, so a very high value won't affect this)
#define MIN_REQUESTTIME			MIN2MS(13)
#define FILEREASKTIME			MIN2MS(28)	//28 mins
#define OLD_FILEREASKTIME		MIN2MS(21)	//21 mins - reask time by old version of the client
#define HIGH2LOW_REASKDELAY		MIN2MS(4)
#define UDPFILEREASKTIMEOUT		MIN2MS(2)	// Interval to send TCP reask if there is no answer for UDP reask
#define SERVERREASKTIME			MIN2MS(14)	// Don't set this too low, it wont speed up anything, 
											// but it could kill emule or your internetconnection
#define SERVER_SRC_REQ_QUANT	300000	// Per 1 quant is allowed 15 source requests
#define UDPSERVERREASKTIME		MIN2MS(22)
#define SOURCECLIENTREASK		MIN2MS(10)
#define ED2KREPUBLISHTIME		MIN2MS(1)	//1 min
#define MINCOMMONPENALTY		9
#define UDPSERVERSTATTIME		SEC2MS(5)	// 5 secs
#define UDPSRVSTATREASKRNDTIME	1801u		// max randomization (prime number)
#define UDPSERVSTATREASKTIME	(4*3600+UDPSRVSTATREASKRNDTIME)	// 4.5 hours (random time of up to 1/2 hour is subtracted at runtime after each ping)
#define UDPSERVSTATMINREASKTIME	MIN2S(20)	// minimum time between two pings even when trying to force a premature ping for a new UDP key
#define UDPSERVERPORT			4665	// Default Server UDP port
#define MAX_SOURCES_FILE_SOFT	1000u
#define MAX_SOURCES_FILE_UDP	100u
// MOD Note: end

#define RARE_FILE				25		// Sources to consider a rare file
#define UPLOAD_LOW_CLIENT_DR	2400	// Uploadspeed per client in bytes
#define MIN_UP_CLIENTS_ALLOWED	2
#define MAX_UP_CLIENTS_ALLOWED	100
#define DOWNLOADTIMEOUT			MIN2MS(2)	// 2 min
#define CONSERVTIMEOUT			g_App.m_pPrefs->SrvConTimeout()
#define MAX_PURGEQUEUETIME		MIN2MS(100)	// Timeout to purge dead sources
#define CONNECTION_LATENCY		22050	// Latency for responses

// You shouldn't change anything here or eMule will probably not work!
#define MAXFRAGSIZE				1300
#define PARTSIZE				9728000ui64
#define PARTSZ32				9728000	// for 32bit operations
#define MAX_EMULE_FILE_SIZE		549755813887ui64 // (512GB - 1) max size which can be transferred
#define OLD_MAX_EMULE_FILE_SIZE	4290048000ui64	// (4294967295/PARTSIZE)*PARTSIZE = ~4GB
#define EMBLOCKSIZE				184320ui64
#define EMBLOCKSZ32				184320	// for 32bit operations
#define MAXFILECOMMENTLEN		128

#define PURGE_TIME				30000	// Time between dropping of unneeded sources
#define QUEUERANK_HIGH			1000
#define QUEUERANK_LOW			250
#define MAX_QUEUE				10000
#define MIN_QUEUE				1000
#define EXPIREIN				g_App.m_pPrefs->WhenSourcesOutdated()
#define RESAVETIME				7200000	// Save sources every 120 minutes (additionally to 'on exit')
#define MAX_NICK_LENGTH			49		// Max length of user nick without trailing zero
#define SESSIONMAXTIME			MIN2MS(120)	// Max upload time

//# Enumeration and EnumDomain class defs
template <typename DataType>
class Enumeration
{
	typedef DataType		BaseType;

protected:
	typedef const DataType	ConstBaseType;
	DataType	m_value;

public:
				Enumeration()
					: m_value(0) {}
protected:
				Enumeration(const DataType value)
					: m_value(value) {}
};

template <typename EnumType,class BaseClass = Enumeration<BYTE> >
class EnumDomain : public BaseClass
{
public:
	typedef EnumType	EType;

				EnumDomain()
					: BaseClass() {}
				EnumDomain(const EnumType eValue)
					: BaseClass(static_cast<ConstBaseType>(eValue)) {}

#ifndef VS2002	// Causes an ambiguity in VS 2002
	operator EnumType() { return static_cast<EnumType>(m_value); }
#endif
	operator const EnumType() const { return static_cast<EnumType>(m_value); }
	EnumType operator ++() { return static_cast<EnumType>(++m_value); }
};

// Packet header and tag codes
#define OP_EDONKEYHEADER	0xE3
#define OP_EDONKEYPROT		OP_EDONKEYHEADER
#define OP_PACKEDPROT		0xD4
#define OP_EMULEPROT		0xC5
#define OP_LANCASTPROT		0xC6
#define OP_UDPRESERVEDPROT1	0xA3	// reserved for later UDP headers (important for EncryptedDatagramSocket)
#define OP_UDPRESERVEDPROT2	0xB2	// reserved for later UDP headers (important for EncryptedDatagramSocket)
#define OP_MLDONKEYPROT		0x00

#define MET_HEADER				0x0E
#define MET_HEADER_I64TAGS		0x0F

#define UNLIMITED				0xFFFF

enum _EnumOpcodes
{
//	client <-> server
	OP_LOGINREQUEST				= 0x01,	// <HASH 16><ID 4><PORT 2><1 Tag_set{NICK,EMULEVER,PORT}>
	OP_REJECT					= 0x05,	// (null)
	OP_GETSERVERLIST			= 0x14,	// (null)client->server
	OP_OFFERFILES				= 0x15,	// <count 4>(<HASH 16><ID 4><PORT 2><1 Tag_set>)[count]
	OP_SEARCHREQUEST			= 0x16,	// <Query_Tree>
	OP_DISCONNECT				= 0x18,	// (not verified)
	OP_GETSOURCES				= 0x19,	// <HASH 16>
	OP_SEARCH_USER				= 0x1A,	// <Query_Tree>
	OP_CALLBACKREQUEST			= 0x1C,	// <ID 4>
	OP_QUERY_CHATS				= 0x1D,	// (deprecated not supported by server any longer)
	OP_CHAT_MESSAGE				= 0x1E,	// (deprecated not supported by server any longer)
	OP_JOIN_ROOM				= 0x1F,	// (deprecated not supported by server any longer)
	OP_QUERY_MORE_RESULT		= 0x21,	// (null)
	OP_GETSOURCES_OBFU			= 0x23,
	OP_SERVERLIST				= 0x32,	// <count 1>(<IP 4><PORT 2>)[count] server->client
	OP_SEARCHRESULT				= 0x33,	// <count 4>(<HASH 16><ID 4><PORT 2><1 Tag_set>)[count]
	OP_SERVERSTATUS				= 0x34,	// <USER 4><FILES 4>
	OP_CALLBACKREQUESTED		= 0x35,	// <IP 4><PORT 2>
	OP_CALLBACK_FAIL			= 0x36,	// (null notverified)
	OP_SERVERMESSAGE			= 0x38,	// <len 2><Message len>
	OP_CHAT_ROOM_REQUEST		= 0x39,	// (deprecated not supported by server any longer)
	OP_CHAT_BROADCAST			= 0x3A,	// (deprecated not supported by server any longer)
	OP_CHAT_USER_JOIN			= 0x3B,	// (deprecated not supported by server any longer)
	OP_CHAT_USER_LEAVE			= 0x3C,	// (deprecated not supported by server any longer)
	OP_CHAT_USER				= 0x3D,	// (deprecated not supported by server any longer)
	OP_IDCHANGE					= 0x40,	// <NEW_ID 4><server_flags 4><primary_tcp_port 4><client_IP_address 4>
	OP_SERVERIDENT				= 0x41,	// <HASH 16><IP 4><PORT 2>{1 TAG_SET}
	OP_FOUNDSOURCES				= 0x42,	// <HASH 16><count 1>(<ID 4><PORT 2>)[count]
	OP_USERS_LIST				= 0x43,	// <count 4>(<HASH 16><ID 4><PORT 2><1 Tag_set>)[count]
	OP_FOUNDSOURCES_OBFU		= 0x44,	// <HASH 16><count 1>(<ID 4><PORT 2><obf settings 1>(UserHash16 if obf&0x08))[count]

// client <-> client
	OP_HELLO					= 0x01,	// 0x10<HASH 16><ID 4><PORT 2><1 Tag_set> - alias for OP_LOGINREQUEST?
	OP_SENDINGPART				= 0x46,	// <HASH 16><von 4><bis 4><Daten len:(von-bis)>
	OP_REQUESTPARTS				= 0x47,	// <HASH 16><von[3] 4*3><bis[3] 4*3>
	OP_FILEREQANSNOFIL			= 0x48,	// <HASH 16>
	OP_END_OF_DOWNLOAD			= 0x49,	// <HASH 16>
	OP_ASKSHAREDFILES			= 0x4A,	// (null)
	OP_ASKSHAREDFILESANSWER 	= 0x4B,	// <count 4>(<HASH 16><ID 4><PORT 2><1 Tag_set>)[count]
	OP_HELLOANSWER				= 0x4C,	// <HASH 16><ID 4><PORT 2><1 Tag_set><SERVER_IP 4><SERVER_PORT 2>
	OP_CHANGE_CLIENT_ID 		= 0x4D,	// <ID_old 4><ID_new 4>
	OP_MESSAGE					= 0x4E,	// <len 2><Message len>
	OP_SETREQFILEID				= 0x4F,	// <HASH 16>
	OP_FILESTATUS				= 0x50,	// <HASH 16><count 2><status(bit array) len:((count+7)/8)>
	OP_HASHSETREQUEST			= 0x51,	// <HASH 16>
	OP_HASHSETANSWER			= 0x52,	// <count 2><HASH[count] 16*count>
	OP_STARTUPLOADREQ			= 0x54,	// <HASH 16>
	OP_ACCEPTUPLOADREQ			= 0x55,	// (null)
	OP_CANCELTRANSFER			= 0x56,	// (null)
	OP_OUTOFPARTREQS			= 0x57,	// (null)
	OP_REQUESTFILENAME			= 0x58,	// <HASH 16>	(more correctly file_name_request)
	OP_REQFILENAMEANSWER		= 0x59,	// <HASH 16><len 4><NAME len>
	OP_CHANGE_SLOT				= 0x5B,	// <HASH 16>
	OP_QUEUERANK				= 0x5C,	// <wert  4> (slot index of the request)
	OP_ASKSHAREDDIRS			= 0x5D,	// (null)
	OP_ASKSHAREDFILESDIR		= 0x5E,	// <len 2><Directory len>
	OP_ASKSHAREDDIRSANS			= 0x5F,	// <count 4>(<len 2><Directory len>)[count]
	OP_ASKSHAREDFILESDIRANS		= 0x60,	// <len 2><Directory len><count 4>(<HASH 16><ID 4><PORT 2><1 Tag_set>)[count]
	OP_ASKSHAREDDENIEDANS		= 0x61,	// (null)

// extended prot client <-> extended prot client
	OP_EMULEINFO				= 0x01,	//
	OP_EMULEINFOANSWER			= 0x02,	//
	OP_COMPRESSEDPART			= 0x40,	// <HASH 16><von 4><size 4><Daten len:size>
	OP_QUEUERANKING				= 0x60,	// <RANG 2>
	OP_FILEDESC					= 0x61,	// <len 2><NAME len>
	OP_REQUESTSOURCES			= 0x81,	// <HASH 16>
	OP_ANSWERSOURCES			= 0x82,	//
	OP_REQUESTSOURCES2			= 0x83,	// <HASH 16><Version 1><Options 2>
	OP_ANSWERSOURCES2			= 0x84,	// <Version 1>[content]
	OP_PUBLICKEY				= 0x85,	// <len 1><pubkey len>
	OP_SIGNATURE				= 0x86,	// v1: <len 1><signature len>  v2:<len 1><signature len><sigIPused 1>
	OP_SECIDENTSTATE			= 0x87,	// <state 1><rndchallenge 4>
	OP_REQUESTPREVIEW			= 0x90,	// <HASH 16>
	OP_PREVIEWANSWER			= 0x91,	// <HASH 16><frames 1>{frames * <len 4><frame len>}
	OP_MULTIPACKET				= 0x92,
	OP_MULTIPACKETANSWER		= 0x93,
	OP_PEERCACHE_QUERY			= 0x94,
	OP_PEERCACHE_ANSWER			= 0x95,
	OP_PEERCACHE_ACK			= 0x96,
	OP_PUBLICIP_REQ				= 0x97,
	OP_PUBLICIP_ANSWER			= 0x98,
	OP_CALLBACK					= 0x99,	// <HASH 16><HASH 16><uint 16>
	OP_REASKCALLBACKTCP			= 0x9A,
	OP_AICHREQUEST				= 0x9B,	// <HASH 16><uint16><HASH aichhashlen>
	OP_AICHANSWER				= 0x9C,	// <HASH 16><uint16><HASH aichhashlen> <data>
	OP_AICHFILEHASHANS			= 0x9D,
	OP_AICHFILEHASHREQ			= 0x9E,
	OP_BUDDYPING				= 0x9F,
	OP_BUDDYPONG				= 0xA0,
	OP_COMPRESSEDPART_I64		= 0xA1,	// <HASH 16><von 8><size 4><Daten len:size>
	OP_SENDINGPART_I64			= 0xA2,	// <HASH 16><von 8><bis 8><Daten len:(von-bis)>
	OP_REQUESTPARTS_I64			= 0xA3,	// <HASH 16><von[3] 8*3><bis[3] 8*3>
	OP_MULTIPACKET_EXT			= 0xA4,
	OP_CHATCAPTCHAREQ			= 0xA5,	// <tags 1>[tags]<Captcha BITMAP>
	OP_CHATCAPTCHARES			= 0xA6,	// <status 1>

//	extended prot client <-> extended prot client UDP
	OP_REASKFILEPING			= 0x90,	// <HASH 16>
	OP_REASKACK					= 0x91,	// <HASH 16>
	OP_FILENOTFOUND				= 0x92,
	OP_QUEUEFULL				= 0x93,
	OP_REASKCALLBACKUDP			= 0x94,
	OP_PORTTEST					= 0xFE,	// Connection Test

//	client <-> UDP server
	OP_GLOBSEARCHREQ3			= 0x90,	// <1 tag set><search_tree>
	OP_GLOBSEARCHREQ2			= 0x92,	// <search_tree>
	OP_GLOBGETSOURCES2			= 0x94,	// <HASH 16><SIZE 4>
	OP_GLOBSERVSTATREQ			= 0x96,	// (null)
	OP_GLOBSERVSTATRES			= 0x97,	// <USER 4><FILES 4>
	OP_GLOBSEARCHREQ			= 0x98,	// <search_tree>
	OP_GLOBSEARCHRES			= 0x99,	//
	OP_GLOBGETSOURCES			= 0x9A,	// <HASH 16>
	OP_GLOBFOUNDSOURCES			= 0x9B,	//
	OP_GLOBCALLBACKREQ			= 0x9C,	// <ID 4>
	OP_INVALID_LOWID			= 0x9E,	// <ID 4>
	OP_SERVER_LIST_REQ			= 0xA0,	// <IP 4><PORT 2>
	OP_SERVER_LIST_RES			= 0xA1,	// <count 1> (<ip 4><port 2>)[count]
	OP_SERVER_DESC_REQ			= 0xA2,	// (null)
	OP_SERVER_DESC_RES			= 0xA3,	// <name_len 2><name name_len><desc_len 2 desc_en>
	OP_SERVER_LIST_REQ2			= 0xA4,	// (null)

//	Lancast
	OP_HASH						= 0x01,	// Broadcast hash
	OP_HASHSEARCH				= 0x02,	// Search the subnet for a hash
	OP_HASHSEARCHRESPONSE		= 0x03,	// Hash found on this client
	OP_FILESEARCH				= 0x04,	// Search the subnet for a file
	OP_FILESEARCHRESPONSE		= 0x05	// File found on this client
};

typedef EnumDomain<_EnumOpcodes>	EnumOpcodes;

#define INV_SERV_DESC_LEN		0xF0FF	// used as 'invalid' string len for OP_SERVER_DESC_REQ/RES

// eDonkeyHybrid truncates every received client message to 200 bytes, although it allows to send messages of any(?) size
#define MAX_CLIENT_MSG_LEN		450		// using 200 is just too short

// Define a parent domain for all tags
// class EnumPacketTags : public Enumeration<BYTE> {};

enum _EnumServerTags
{
// server tags
	ST_SERVERNAME			= 0x01,	// <string>
	ST_DESCRIPTION			= 0x0B,	// <string>
	ST_PING					= 0x0C,	// <int>
	ST_FAIL					= 0x0D,	// <int>
	ST_PREFERENCE			= 0x0E,	// <int>
	ST_PORT					= 0x0F,	// <uint32>
	ST_IP					= 0x10,	// <uint32>
	ST_DYNIP				= 0x85,
//	ST_LASTPING				= 0x86,	// <int> No longer used.
	ST_MAXUSERS				= 0x87,
	ST_SOFTFILES			= 0x88,
	ST_HARDFILES			= 0x89,
	ST_LASTPING				= 0x90,	// <int>
	ST_VERSION				= 0x91,	// <string>
	ST_UDPFLAGS				= 0x92,	// <int>
	ST_AUXPORTSLIST			= 0x93,	// <string>
	ST_LOWIDUSERS			= 0x94,	// <uint32>
	ST_UDPKEY				= 0x95,	// <uint32>
	ST_UDPKEYIP				= 0x96,	// <uint32>
	ST_TCPPORTOBFUSCATION	= 0x97,	// <uint16>
	ST_UDPPORTOBFUSCATION	= 0x98	// <uint16>
};
typedef EnumDomain<_EnumServerTags>	EnumServerTags;

enum _EnumFileTags
{
	FT_FILENAME				= 0x01,	// <string>
	FT_FILESIZE				= 0x02,	// <uin32> (or <uint64> when supported)
	FT_FILETYPE				= 0x03,	// <string>
	FT_FILEFORMAT			= 0x04,	// <string>
	FT_LASTSEENCOMPLETE		= 0x05,	// <uint32>

	FT_TRANSFERRED			= 0x08,	// <uint32>
	FT_GAPSTART				= 0x09,	// <uint32>
	FT_GAPEND				= 0x0A,	// <uint32>
	FT_DESCRIPTION			= 0x0B,	// <string>
	FT_PARTFILENAME			= 0x12,	// <string>
	FT_STATUS				= 0x14,	// <uint32>
	FT_SOURCES				= 0x15,	// <uint32>
	FT_PERMISSIONS			= 0x16,	// <uint32>
	FT_DLPRIORITY			= 0x18,
	FT_ULPRIORITY			= 0x19,
	FT_COMPRESSION			= 0x1A,
	FT_CORRUPTED			= 0x1B,
	FT_KADLASTPUBLISHKEY	= 0x20,	// <uint32>
	FT_KADLASTPUBLISHSRC	= 0x21,	// <uint32>
	FT_FLAGS				= 0x22,	// <uint32>
	FT_DL_ACTIVE_TIME		= 0x23,	// <uint32>
	FT_CORRUPTEDPARTS		= 0x24,	// <string>
	FT_DL_PREVIEW			= 0x25,
	FT_KADLASTPUBLISHNOTES	= 0x26,	// <uint32>
	FT_AICH_HASH			= 0x27,
	FT_FILEHASH				= 0x28,
	FT_COMPLETE_SOURCES		= 0x30,	// nr. of sources which share a complete version of the associated file (supported by eserver 16.46+)
	FT_COLLECTIONAUTHOR		= 0x31,
	FT_COLLECTIONAUTHORKEY	= 0x32,
	FT_LASTSHARED			= 0x34,	// <uint32>
	FT_FILESIZE_HI			= 0x3A,	// <uint32>

//	Statistics
	FT_ATTRANSFERRED		= 0x50,
	FT_ATREQUESTED			= 0x51,
	FT_ATACCEPTED			= 0x52,
	FT_CATEGORY				= 0x53,	// <uint32>
	FT_ATTRANSFERREDHI		= 0x54,
	FT_MEDIA_ARTIST			= 0xD0,	// <string>
	FT_MEDIA_ALBUM			= 0xD1,	// <string>
	FT_MEDIA_TITLE			= 0xD2,	// <string>
	FT_MEDIA_LENGTH			= 0xD3,	// <uint32>
	FT_MEDIA_BITRATE		= 0xD4,	// <uint32>
	FT_MEDIA_CODEC			= 0xD5,	// <string>
	FT_FILERATING			= 0xF7	// <uint8>
};

//	Additional media meta data tags from eDonkeyHybrid (note also the uppercase/lowercase)
#define FT_ED2K_MEDIA_ARTIST	"Artist"	// <string>
#define FT_ED2K_MEDIA_ALBUM		"Album"		// <string>
#define FT_ED2K_MEDIA_TITLE		"Title"		// <string>
#define FT_ED2K_MEDIA_LENGTH	"length"	// <string>
#define FT_ED2K_MEDIA_BITRATE	"bitrate"	// <uint32>
#define FT_ED2K_MEDIA_CODEC		"codec"		// <string>

// ed2k search expression comparison operators
#define ED2K_SEARCH_OP_EQUAL         0 // eserver 16.45+
#define ED2K_SEARCH_OP_GREATER       1 // dserver
#define ED2K_SEARCH_OP_LESS          2 // dserver
#define ED2K_SEARCH_OP_GREATER_EQUAL 3 // eserver 16.45+
#define ED2K_SEARCH_OP_LESS_EQUAL    4 // eserver 16.45+
#define ED2K_SEARCH_OP_NOTEQUAL      5 // eserver 16.45+

enum _EnumClientTags
{
// client tags, ed2k-prot OP_HELLO
	CT_NAME					= 0x01,
	CT_VERSION				= 0x11,
	CT_PORT					= 0x0f,
	CT_SERVER_FLAGS			= 0x20, // currently used only to inform a server about supported features
	CT_MOD_VERSION			= 0x55,
	CT_FRIENDSHARING		= 0x66, // [Bloodymad@Vorlost Anti-Friendsharing 0.3]
	CT_EMULE_RESERVED1		= 0xf0,
	CT_EMULE_RESERVED2		= 0xf1,
	CT_EMULE_RESERVED3		= 0xf2,
	CT_EMULE_RESERVED4		= 0xf3,
	CT_EMULE_RESERVED5		= 0xf4,
	CT_EMULE_RESERVED6		= 0xf5,
	CT_EMULE_RESERVED7		= 0xf6,
	CT_EMULE_RESERVED8		= 0xf7,
	CT_EMULE_RESERVED9		= 0xf8,
	CT_EMULE_UDPPORTS		= 0xf9,
	CT_EMULE_MISCOPTIONS1	= 0xfa,
	CT_EMULE_VERSION		= 0xfb,
	CT_EMULE_RESERVED10		= 0xfc,
	CT_EMULE_RESERVED11		= 0xfd,
	CT_EMULE_MISCOPTIONS2	= 0xfe,
	CT_EMULE_RESERVED13		= 0xff,

	CT_SERVER_UDPSEARCH_FLAGS	= 0x0e
};

//	Client Capabilities on connection to a server (CT_SERVER_FLAGS)
#define SRVCAP_ZLIB				0x0001
#define SRVCAP_IP_IN_LOGIN		0x0002
#define SRVCAP_AUXPORT			0x0004
#define SRVCAP_NEWTAGS			0x0008
#define SRVCAP_UNICODE			0x0010
#define SRVCAP_LARGEFILES		0x0100
#define SRVCAP_SUPPORTCRYPT		0x0200
#define SRVCAP_REQUESTCRYPT		0x0400
#define SRVCAP_REQUIRECRYPT		0x0800

//	Client Capabilities for UDP requests (CT_SERVER_UDPSEARCH_FLAGS)
#define SRVCAP_UDP_NEWTAGS_LARGEFILES	0x01

enum _EnumEmuleTags
{
// eMule tags, emule-extend-prot OP_EMULEINFO
	ET_COMPRESSION				= 0x20,
	ET_UDPPORT					= 0x21,
	ET_UDPVER					= 0x22,
	ET_SOURCEEXCHANGE			= 0x23,
	ET_COMMENTS					= 0x24,
	ET_EXTENDEDREQUEST			= 0x25,
	ET_COMPATIBLECLIENT			= 0x26,
	ET_FEATURES					= 0x27,	// secure credits (official 0.29b)

	ET_INCOMPLETEPARTS			= 0x3D, // ICS
	ET_L2HAC					= 0x3E, // L2HAC

// mod tags
	ET_MOD_FEATURESET			= 0x54, // [Bloodymad Featureset]
	ET_MOD_VERSION				= 0x55, // Mod ver Generic String
	ET_MOD_PLUS					= 0x99, // To avoid conflicts with ET_TAROD_VERSION recognized by lugdunum srvers

// identified MOD's
	ET_MOD_BOWLFISH				= 0x5A,
	ET_MOD_TAROD				= 0x77,
	ET_MOD_TAROD_VERSION		= 0x78,
	ET_MOD_Morph				= 0x79,
	ET_MOD_Morph_VERSION		= 0x80,
	ET_MOD_LSD					= 0x87,
	ET_MOD_LSD_VERSION			= 0x88
};

enum _EnumFriendTags
{
	FF_NAME		= 0x01,
	FF_KADID	= 0x02
};

// emuleapp <-> emuleapp
#define OP_ED2KLINK				12000
#define OP_CLCOMMAND			12001

// Menu commands
#define MP_MESSAGE				10102
#define MP_DETAIL				10103
#define MP_ADDFRIEND			10104
#define MP_REMOVEFRIEND			10105
#define MP_SHOWLIST				10106
#define MP_CANCEL				10201
#define MP_STOP					10202
#define MP_PAUSE				10203
#define MP_RESUME				10204
#define MP_CLEARCOMPLETED		10205
#define MP_CLEARALLCOMPLETED	10206
#define MP_OPEN					10207
#define MP_PREVIEW				10208
#define MP_RESUMEPAUSED			10209
#define MP_DOWNNOW				10211
#define MP_SAVELOG				10212
#define MP_SAVERTF				10213
#define MP_OPENFOLDER 			10214
#define MP_CUSTOMIZETOOLBAR		10215
#define MP_SELECTTOOLBARBITMAPDIR 10216
#define MP_SELECTTOOLBARBITMAP	10217
#define MP_NOTEXTLABELS			10218
#define MP_TEXTLABELS			10219
#define MP_TEXTLABELSONRIGHT	10220
#define MP_TOOLBARBITMAP		10221
#define MP_CHANGEDIR			10222
#define MP_SHELLCONTEXT			10223
#define MP_DOCLEANUP			10224
#define MP_INITIALIZE			10225
#define MP_PREALLOCATE			10226
#define MP_SMARTFILTER			10229
#define MP_SRCFILTER_UPLOADING	10230
#define MP_SRCFILTER_ONQUEUE	10231
#define MP_SRCFILTER_FULLQUEUE	10232
#define MP_SRCFILTER_CONNECTED	10233
#define MP_SRCFILTER_CONNECTING	10234
#define MP_SRCFILTER_NNP		10235
#define MP_SRCFILTER_WAITFILEREQ	10236
#define MP_SRCFILTER_LOWTOLOWID	10237
#define MP_SRCFILTER_BANNED		10238
#define MP_SRCFILTER_ERROR		10239
#define MP_SRCFILTER_A4AF		10240
#define MP_SRCFILTER_UNKNOWN	10241
#define MP_SRCFILTER_HIDEALL	10242
#define MP_SRCFILTER_SHOWALL	10243
#define MP_JUMPSTART			10244
#define MP_SEARCHRELATED		10245
#define MP_SRCFILTER_OTHERSRVLOWID	10246

// IRC
#define Irc_Version				_T("(SMIRCv00.68)")
#define Irc_Op					10280
#define Irc_DeOp				10281
#define Irc_Voice				10282
#define Irc_DeVoice				10283
#define Irc_HalfOp				10284
#define Irc_DeHalfOp			10285
#define Irc_Kick				10286
#define Irc_Slap				10287
#define Irc_Join				10288
#define Irc_Close				10289
#define Irc_Priv				10290
#define Irc_Owner				10294
#define Irc_DeOwner				10295
#define Irc_Protect				10296
#define Irc_DeProtect			10297
#define Irc_Refresh				10298
#define Irc_WhoIs				10299

// Context Menus
#define MP_PRIOVERYLOW			10300
#define MP_PRIOLOW				10301
#define MP_PRIONORMAL			10302
#define MP_PRIOHIGH				10303
#define MP_PRIOAUTO				10304
#define MP_PRIORELEASE			10305
#define MP_GETED2KLINK			10306
#define MP_GETHTMLED2KLINK		10307
#define MP_GETSOURCEED2KLINK	10308
#define MP_METINFO				10309
#define MP_PERMALL				10310
#define MP_PERMFRIENDS			10311
#define MP_PERMNONE				10312
#define MP_CONNECTTO			10313
#define MP_REMOVE				10314
#define MP_REMOVEALL			10315
#define MP_REMOVESELECTED		10316
#define MP_UNBAN				10317
#define MP_ADDTOSTATIC			10318
#define MP_GETHASH				10319
#define MP_RENAME				10321
#define MP_METINFOSOURCES		10322
#define MP_EDIT					10323
#define MP_CAT_ADD				10324
#define MP_CAT_EDIT				10325
#define MP_CAT_REMOVE			10326
#define MP_RESUMENEXT			10327
#define MP_COPYSELECTED			10329
#define MP_SELECTALL			10330
#define MP_AUTOSCROLL			10331
#define MP_REMOVEFROMSTATIC		10332
#define MP_VIEWFILECOMMENTS		10333
#define MP_CMT					10334
#define MP_SWITCHCTRL			10335
#define MP_SFL_CLEARSTATS		10336
#define MP_SFL_CLEARALLSTATS	10337
#define MP_SFL_ALLYAKNOW		10338
#define MP_SFL_MERGEKNOWN		10339
#define MP_SFL_DELKNOWN			10340
#define MP_SFL_PARTON			10341
#define MP_SFL_PARTHIDDEN		10343
#define MP_ASK_FOR_FILES		10344
#define MP_ALL_A4AF_TO_HERE		10345
#define MP_ALL_A4AF_SAMECAT		10346
#define MP_ALL_A4AF_AUTO		10347
#define MP_MOVIE				10348
#define MP_ALL_A4AF_TO_OTHER	10349
#define MP_AV_SCAN				10350
#define MP_WORDWRAP				10351
#define MP_WEBURL				10400 // +64 BEWARE: taking up to 10463
#define MP_WEBURL_LAST			10463

// Filter sub-menu
#define MP_FILTER_NONE			10764
#define MP_FILTER_BANNED		10765
#define MP_FILTER_FRIEND		10766
#define MP_FILTER_CREDIT		10767

// Category sub-menu
#define MP_ASSIGNCAT			10800 // +100 BEWARE: taking up to 10899
#define MP_LASTASSIGNCAT		10899
#define MP_SHOWPREDEFINEDCAT_0	10900
#define MP_SHOWPREDEFINEDCAT_LAST 10999
#define MP_CAT_SET_0			11000 // +100 BEWARE: taking up to 11099
#define MP_CAT_SET_LAST			11099

// Thread messages
#define TM_FINISHEDHASHING			(WM_APP + 10)
#define TM_HASHFAILED				(WM_APP + 11)
#define TM_HASHINGSTARTED			(WM_APP + 12)
#define TM_SOURCEHOSTNAMERESOLVED	(WM_APP + 1450)

// Quick Speed Changer
#define MP_QS_PA				10533
#define MP_QS_UA				10534

#define UM_SPN_SIZED			10535

// Windows message IDs

// Asynchronous UI update messages
#define WM_SFL_UPDATEITEM			(WM_USER + 2001)
#define WM_DL_REFRESH				(WM_USER + 2002)
#define WM_CL_REFRESH				(WM_USER + 2003)
#define WM_UL_REFRESH				(WM_USER + 2004)
#define WM_QL_REFRESH				(WM_USER + 2005)
#define WM_LOG_REFRESH				(WM_USER + 2006)
#define WM_DLOG_REFRESH				(WM_USER + 2007)

// Web Server
#define WEB_CONNECT_TO_SERVER	WM_USER + 7001
#define WEB_REMOVE_SERVER		WM_USER + 7002
#define WEB_SHARED_FILES_RELOAD	WM_USER + 7003
#define WEB_ADD_TO_STATIC		WM_USER + 7004
#define WEB_REMOVE_FROM_STATIC	WM_USER + 7005
#define WEB_CLEAR_COMPLETED		WM_USER + 7006
#define WEB_FILE_RENAME			WM_USER + 7007

#define PR_VERYLOW			4 // I Had to change this because it didn't save negative number correctly.
#define SRV_PR_LOW			2
#define PR_LOW				0
#define PR_NORMAL			1 // Don't change this - needed for eDonkey clients and server!
#define SRV_PR_NORMAL		0
#define PR_HIGH				2
#define SRV_PR_HIGH			1
#define PR_RELEASE			3
#define PR_AUTO				5
#define PR_JUMPSTART		6

#define L2HAC_DEFAULT_EMULE		(FILEREASKTIME)
#define L2HAC_MIN_TIME			900000
#define L2HAC_MAX_TIME			3600000
#define L2HAC_CALLBACK_PRECEDE	(CONNECTION_TIMEOUT >> 1)

//	Statistics section
enum
{
	STATS_DLSRC_TOTAL = 0,
	STATS_DLSRC_TRANSFERRING,

	STATS_DLSRC_ONQUEUE,
	STATS_DLSRC_QUEUEFULL,
	STATS_DLSRC_NNS,
	STATS_DLSRC_CONNECTED,
	STATS_DLSRC_CONNECTING,
	STATS_DLSRC_CONNECTING_VIA_SRV,
	STATS_DLSRC_WAIT4FILEREQ,
	STATS_DLSRC_LOW2LOW,
	STATS_DLSRC_LOWID_ON_OTHER_SRV,
	STATS_DLSRC_BANNED,
	STATS_DLSRC_HIGH_QR,

	STATS_DLSRC_COUNT
};

enum
{
	STATS_DLDAT_ACTFILESIZE = 0,	// Total Size of Active Downloads
	STATS_DLDAT_SIZE2TRANSFER,		// Total Size Left to Transfer
	STATS_DLDAT_ACTFILEREQSPACE,	// Total Space Needed by Active Downloads
	STATS_DLDAT_FILESIZETOTAL,		// Total Size of All Downloads
	STATS_DLDAT_FILEREQSPACETOTAL,	// Total Space Needed by All Downloads
	STATS_DLDAT_FILEREALSIZE,		// Total Size on Disk of All Downloads

	STATS_DLDATA_COUNT
};

// Average calculation
#define AVG_SESSION					0
#define AVG_TIME					1
#define AVG_TOTAL					2

//	Download State
enum _EnumDLQState
{
	DS_DOWNLOADING	=	0,
	DS_CONNECTING,		// connecting state: we are connecting to the remote client
	DS_WAITCALLBACK,	// connecting state: we are waiting connection from the remote LowID client
	DS_CONNECTED,
	DS_ONQUEUE,
	DS_NONEEDEDPARTS,	// used for HighID, Local and Remote LowIDs
	DS_WAIT_FOR_FILE_REQUEST,
	DS_LOWID_ON_OTHER_SERVER,
	DS_REQHASHSET,
	DS_LOWTOLOWID,

	DS_LAST_QUEUED_STATE,

	DS_ERROR,
	DS_NONE,

	DS_LASTSTATE
};
typedef EnumDomain<_EnumDLQState>		EnumDLQState;

// State Lists Masks
#define SLM_ALL						0x000001FF
#define SLM_ALLOWED_TO_A4AF_SWAP	0x000001F6
#define SLM_ALLOWED_TO_SAVE			0x0000017B	// excluded DS_WAITCALLBACK & DS_LOWID_ON_OTHER_SERVER
#define SLM_VALID_SOURCES				0x0000009F	// note: consider connecting & asking sources as valid althrough their status is unknown
#define SLM_CHECK_SERVER_CHANGE		0x000002B0

//	Upload state
enum _EnumULQState
{
	US_UPLOADING		=	0,
	US_ONUPLOADQUEUE		 ,
	US_CONNECTING			 ,
	US_BANNED				 ,
	US_NONE					 ,
	US_LAST
};
typedef EnumDomain<_EnumULQState>		EnumULQState;

// Client image list definitions
enum
{
	CLIENT_IMGLST_PLAIN = 0,
	CLIENT_IMGLST_FRIEND,
	CLIENT_IMGLST_CREDITUP,
	CLIENT_IMGLST_CREDITDOWN,
	CLIENT_IMGLST_BANNED,

	CLIENT_IMGLST_COUNT
};

// Search type
enum _EnumSearchFileTypes
{
	SFT_SERVER = 0,
	SFT_CLIENT
};
typedef EnumDomain<_EnumSearchFileTypes>	EnumSearchFileTypes;
