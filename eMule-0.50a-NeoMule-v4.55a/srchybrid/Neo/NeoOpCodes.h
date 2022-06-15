//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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


#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
#define LANCAST_PORT			5000
#define LANCAST_GROUP			_T("224.0.0.1")

#define OP_LANCASTPROT			0xC6

#define OP_HASH					0x01	// Broadcast hash
#define OP_HASHSEARCH			0x02	// Search the subnet for a hash
#define OP_HASHSEARCHRESPONSE	0x03	// Hash found on this client
#define OP_FILESEARCH			0x04	// Search the subnet for a file
#define OP_FILESEARCHRESPONSE	0x05	// File found on this client
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
#define OP_VOODOOREQUEST		0x06	// Search the subnet for voodoo clients
#endif // VOODOO // NEO: VOODOO END
#endif //LANCAST // NEO: NLC END <-- Xanatos --


// NEO: FCFG - [FileConfiguration] -- Xanatos -->
#define NEOFILE_VERSION					NEOFILE_VERSION_450
#define NEOFILE_VERSION_OLD				NEOFILE_VERSION_400
#define NEOFILE_VERSION_254				0xD0
#define NEOFILE_VERSION_255				0xD1
#define NEOFILE_VERSION_300				0xD2
#define NEOFILE_VERSION_400				0xD3
#define NEOFILE_VERSION_450				0xD4

#define PARTPREFSFILE_VERSION			PARTPREFSFILE_VERSION_450
//#define PARTPREFSFILE_VERSION_OLD		
#define PARTPREFSFILE_VERSION_450		0xE2

#define KNOWNPREFSFILE_VERSION			KNOWNPREFSFILE_VERSION_450
#define KNOWNPREFSFILE_VERSION_OLD		KNOWNPREFSFILE_VERSION_400
#define KNOWNPREFSFILE_VERSION_254		0xB0
#define KNOWNPREFSFILE_VERSION_300		0xB1
#define KNOWNPREFSFILE_VERSION_400		0xB2
#define KNOWNPREFSFILE_VERSION_450		0xB3
// NEO: FCFG END <-- Xanatos --

// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
#define TRAFFICFILE_VERSION				TRAFFICFILE_VERSION_400
#define TRAFFICFILE_VERSION_OLD			TRAFFICFILE_VERSION_300

#define TRAFFICFILE_VERSION_254			0xA0 // not more supported
#define TRAFFICFILE_VERSION_300			0xA1
#define TRAFFICFILE_VERSION_400			0xA2
// NEO: NPT END <-- Xanatos --

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
#define SOURCEFILE_VERSION				SOURCEFILE_VERSION_400
//#define SOURCEFILE_VERSION_OLD			SOURCEFILE_VERSION_

#define SOURCEFILE_VERSION_400			0xF0
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
#define SRCFILE_VERSION					SRCFILE_VERSION_450
//#define SRCFILE_VERSION_OLD				SRCFILE_VERSION_
#define SRCFILE_VERSION_450				0xE1
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

#define AICHBLOCKMAP_VERSION			0xA0 // NEO: SCV - [SubChunkVerification] <-- Xanatos --

// NEO: XCs - [SaveComments] -- Xanatos -->
#define COMMENTSFILE_VERSION			COMMENTSFILE_VERSION_300
#define COMMENTSFILE_VERSION_OLD		COMMENTSFILE_VERSION_255
#define COMMENTSFILE_VERSION_255		0xC0
#define COMMENTSFILE_VERSION_300		0xC1
// NEO: XCs END <-- Xanatos --

// NEO: NMP - [NeoModProt] -- Xanatos -->
#define OP_MODPROT				0x4D // 'M' 
#define	OP_MODPACKEDPROT 		0x6D // 'm'

#define OP_MODINFOPACKET		0x01

#define MT_MOD_PROTOCOL_EXTENSIONS  0x4D //'M'
#define MT_MOD_PROTOCOL_EXTENSIONS2 0x6D //'m'

#define MT_NEO_PROTOCOL_EXTENSIONS  0x4E //'N'
#define MT_NEO_PROTOCOL_EXTENSIONS2 0x6E //'n'

#define OP_MODMULTIPACKET		0x11	// NEO: NMPm - [NeoModProtMultiPacket]
#define OP_MODMULTIPACKETANSWER	0x12	// NEO: NMPm - [NeoModProtMultiPacket]
// NEO: NMP END <-- Xanatos --

#define ET_INCOMPLETEPARTS		0x3D	// NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
#define OP_FILEINCSTATUS		0x8E	// NEO: ICS - [InteligentChunkSelection] <-- Xanatos -- 

#define OP_SUBCHUNKMAPS			0x53	// NEO: SCT - [SubChunkTransfer] <-- Xanatos --


#define OP_NEO_ANSWERSOURCES	0x82	// NEO: NXS - [NeoXS] <-- Xanatos --

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
#define	OP_NAT_CARIER			OP_MODPROT

#define OP_NAT_PING				0xD7

#define OP_NAT_SYN				0xD0	// <Ver 1>(<MSS 4>)
#define OP_NAT_SYN_ACK			0xD1	// <Ver 1>(<MSS 4>)

#define OP_NAT_DATA				0xD2	// <SeqNr 4>{data}
#define OP_NAT_DATA_ACK			0xD3	// <SeqNr 4><RecvWndSize 4>

#define OP_NAT_FIN				0xD4	// (empty)
#define OP_NAT_FIN_ACK			0xD5	// (empty)

#define OP_NAT_RST				0xD6	// (<Err 4>)

//
#define MT_EMULE_BUDDYID		0x40 //'@'	// <hash 16>
#define MT_XS_EMULE_BUDDYIP		0x42 //'B'	// <ip 4>
#define MT_XS_EMULE_BUDDYUDP	0x62 //'b'	// <port 2>

#define OP_NAT_CALLBACKREQUEST_KAD	0xB0

//
#define OP_XS_BUDDY_REQ			0xB1	// empty
#define OP_XS_BUDDY_ANSWER		0xB2	// <result 1>
#define OP_XS_BUDDYPING			0xB3	// empty
#define OP_XS_MULTICALLBACKUDP	0xB4	//
#define OP_XS_MULTICALLBACKTCP	0xB5	//

#define OP_CALLBACKREQUEST_XS		0xBA //
#define OP_NAT_CALLBACKREQUEST_XS	0xBB //

//
#define OP_PUBLICPORT_REQ		0xA7	// empty
#define	OP_PUBLICPORT_ANSWER	0xA8	// <port 2>

#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

// NEO: FCFG - [FileConfiguration] -- Xanatos -->
// partprefs
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
#define OP_ENABLE_LAN_CAST						0xF1
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
#define OP_ENABLE_VOODOO						0xF2
#endif // VOODOO // NEO: VOODOO END

// NEO: IPS - [InteligentPartSharing]
#define OP_USE_INTELIGENT_PART_SHARING			0x01
#define OP_INTELIGENT_PART_SHARING_TIMER		0x02
#define OP_MAX_PROZENT_TO_HIDE					0x03

// OverAvalibly
#define OP_HIDE_OVER_AVALIBLY_PARTS				0x04
#define OP_HIDE_OVER_AVALIBLY_MODE				0x05
#define OP_HIDE_OVER_AVALIBLY_VALUE				0x06

#define OP_BLOCK_HIGH_OVERAVALIBLY_PARTS		0x07
#define OP_BLOCK_HIGH_OVER_AVALIBLY_FACTOR		0x08

// OverShared
#define OP_HIDE_OVER_SHARED_PARTS				0x09
#define OP_HIDE_OVER_SHARED_MODE				0x0A
#define OP_HIDE_OVER_SHARED_VALUE				0x0B
#define OP_HIDE_OVER_SHARED_CALC				0x0C

#define OP_BLOCK_HIGH_OVER_SHARED_PARTS			0x0D
#define OP_BLOCK_HIGH_OVER_SHARED_FACTOR		0x0E

// DontHideUnderAvalibly
#define OP_DONT_HIDE_UNDER_AVALIBLY_PARTS		0x0F
#define OP_DONT_HIDE_UNDER_AVALIBLY_MODE		0x10
#define OP_DONT_HIDEUNDER_AVALIBLY_VALUE		0x11

// Other
#define OP_SHOW_ALWAYS_SOME_PARTS				0x12
#define OP_SHOW_ALWAYS_SOME_PARTS_VALUE			0x13

#define OP_SHOW_ALWAYS_INCOMPLETE_PARTS			0x14
// NEO: IPS END

// NEO: SRS - [SmartReleaseSharing]
#define OP_RELEASE_MODE							0x21
#define OP_RELEASE_LEVEL						0x22
#define OP_RELEASE_TIMER						0x23

// release limit
#define OP_RELEASE_LIMIT						0x24
#define OP_RELEASE_LIMIT_MODE					0x25
#define OP_RELEASE_LIMIT_HIGH					0x26
#define OP_RELEASE_LIMIT_LOW					0x27

#define OP_RELEASE_LIMIT_LINK					0x28

#define OP_RELEASE_LIMIT_COMPLETE				0x29
#define OP_RELEASE_LIMIT_COMPLETE_MODE			0x2A
#define OP_RELEASE_LIMIT_COMPLETE_HIGH			0x2B
#define OP_RELEASE_LIMIT_COMPLETE_LOW			0x2C

// limit
#define OP_LIMIT_LINK							0x2D

// source limit
#define OP_SOURCE_LIMIT							0x2E
#define OP_SOURCE_LIMIT_MODE					0x2F
#define OP_SOURCE_LIMIT_HIGH					0x30
#define OP_SOURCE_LIMIT_LOW						0x31

#define OP_SOURCE_LIMIT_LINK					0x32

#define OP_SOURCE_LIMIT_COMPLETE				0x33
#define OP_SOURCE_LIMIT_COMPLETE_MODE			0x34
#define OP_SOURCE_LIMIT_COMPLETE_HIGH			0x35
#define OP_SOURCE_LIMIT_COMPLETE_LOW			0x36
// NEO: SRS END

// NEO: MPS - [ManualPartSharing]
#define OP_PR_PART_ON							0xA1
#define OP_PR_PART_HIDEN						0xA2
#define OP_PR_PART_OFF							0xA3
// NEO: MPS END

// knownprefs
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
#define OP_LC_INTERVALS							0xF3

#define OL_LAN_SOURCE_REASK_TIME				0xF4
#define OL_LAN_NNP_SOURCE_REASK_TIME			0xF5
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
#define OP_VOODOO_XS							0xF6
#endif // VOODOO // NEO: VOODOO END

// NEO: SRT - [SourceRequestTweaks]
// General
#define OP_MAX_SOURCE							0x01

// Management
#define OP_SWAP_LIMIT							0x02

//XS
#define OP_XS_ENABLE							0x03
#define OP_XS_LIMIT								0x04
#define OP_XS_INTERVALS							0x05
#define OP_XS_CLIENT_INTERVALS					0x06
#define OP_XS_CLEINT_DELAY						0x07
#define OP_XS_RARE_LIMIT						0x08

// SVR
#define OP_SVR_ENABLE							0x09
#define OP_SVR_LIMIT							0x0A
#define OP_SVR_INTERVALS						0x0B

//KAD
#define OP_KAD_ENABLE							0x0C
#define OP_KAD_LIMIT							0x0D
#define OP_KAD_INTERVALS						0x0E
#define OP_KAD_MAX_FILES						0x0F
#define OP_KAD_REPEAT_DELAY						0x10

//UDP
#define OP_UDP_ENABLE							0x11
#define OP_UDP_LIMIT							0x12
#define OP_UDP_INTERVALS						0x13
#define OP_UDP_GLOBAL_INTERVALS					0x14
#define OP_UDP_FILES_PER_SERVER					0x15
// NEO: SRT END

// NEO: XSC - [ExtremeSourceCache]
#define OP_USE_SOURCE_CACHE						0x16
#define OP_SOURCE_CACHE_LIMIT					0x17
#define OP_SOURCE_CACHE_TIME					0x18
// NEO: XSC END

// NEO: ASL - [AutoSoftLock]
#define OP_AUTO_SOFT_LOCK						0x19
#define OP_AUTO_SOFT_LOCK_LIMIT					0x1A
// NEO: ASL END

// NEO: AHL - [AutoHardLimit]
#define OP_AUTO_HARD_LIMIT						0x1B
#define OP_AUTO_HARD_LIMIT_TIME					0x1C
// NEO: AHL END

// NEO: CSL - [CategorySourceLimit]
#define OP_CATEGORY_SOURCE_LIMIT				0x1D
#define OP_CATEGORY_SOURCE_LIMIT_LIMIT			0x1E
#define OP_CATEGORY_SOURCE_LIMIT_TIME			0x1F
// NEO: CSL END

// NEO: GSL - [GlobalSourceLimit]
#define OP_GLOBAL_SOURCE_LIMIT					0x20
#define OP_GLOBAL_SOURCE_LIMIT_LIMIT			0x21
#define OP_GLOBAL_SOURCE_LIMIT_TIME				0x22
// NEO: GSL END

#define OP_MIN_SOURCE_PER_FILE					0x23

#define OP_TCP_CONNECTION_RETRY					0x30 // NEO: TCR - [TCPConnectionRetry]

// NEO: DRT - [DownloadReaskTweaks]
#define OP_SPREAD_REASK_ENABLE					0x31
#define OP_SPREAD_REASK_TIME					0x32
#define OP_SOURCE_REASK_TIME					0x33
#define OP_FULLQ_SOURCE_REASK_TIME				0x34
#define OP_NNP_SOURCE_REASK_TIME				0x35
// NEO: DRT END

// NEO: SDT - [SourcesDropTweaks]
#define OP_DROP_TIME							0x41

//Bad
#define OP_BAD_SOURCE_DROP						0x42
#define OP_BAD_SOURCE_LIMIT_MODE				0x43
#define OP_BAD_SOURCE_LIMIT						0x44
#define OP_BAD_SOURCE_DROP_MODE					0x45
#define OP_BAD_SOURCE_DROP_TIME					0x46

//NNP
#define OP_NNP_SOURCE_DROP						0x47
#define OP_NNP_SOURCE_LIMIT_MODE				0x48
#define OP_NNP_SOURCE_LIMIT						0x49
#define OP_NNP_SOURCE_DROP_MODE					0x4A
#define OP_NNP_SOURCE_DROP_TIME					0x4B

//FullQ
#define OP_FULLQ_SOURCE_DROP					0x4C
#define OP_FULLQ_SOURCE_LIMIT_MODE				0x4D
#define OP_FULLQ_SOURCE_LIMIT					0x4E
#define OP_FULLQ_SOURCE_DROP_MODE				0x4F
#define OP_FULLQ_SOURCE_DROP_TIME				0x50

//HighQ
#define OP_HIGHQ_SOURCE_DROP					0x51
#define OP_HIGHQ_SOURCE_LIMIT_MODE				0x52
#define OP_HIGHQ_SOURCE_LIMIT					0x53
#define OP_HIGHQ_SOURCE_DROP_MODE				0x54
#define OP_HIGHQ_SOURCE_DROP_TIME				0x55
#define OP_HIGHQ_SOURCE_RANK_MODE				0x56
#define OP_HIGHQ_SOURCE_MAX_RANK				0x57

#define OP_DEAD_TIME							0x58
#define OP_DEAD_TIME_FW_MILTU					0x59
#define OP_GLOBAL_DEAD_TIME						0x5A
#define OP_GLOBAL_DEAD_TIME_FW_MILTU			0x5B
// NEO: SDT END

#define OP_FORCE_A4AF							0x60 // NEO: NXC - [NewExtendedCategories] 

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
#define OP_ENABLE_SOURCE_STORAGE				0x71
#define OP_SOURCE_STORAGE_LIMIT					0x72

#define OP_STORE_ALSO_A4AF_SOURCES				0x73

#define OP_AUTO_SAVE_SOURCES					0x74
#define OP_AUTO_SAVE_SOURCES_INTERVALS			0x75

#define OP_AUTO_LOAD_SOURCES					0x76
#define OP_LOADED_SOURCE_CLEAN_UP_TIME			0x77

#define OP_TOTAL_SOURCE_RESTORE					0x78

#define OP_SOURCE_STORAGE_REASK_LIMIT			0x79

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
#define OP_REASK_PROPABILITY					0x7A
#define OP_UNPREDICTED_PROPABILITY				0x7B
#define OP_UNPREDICTED_REASK_PROPABILITY		0x7C
 #endif // NEO_SA // NEO: NSA END

#define OP_AUTO_REASK_STORED_SOURCES_DELAY		0x7D

#define OP_GROUP_STORED_SOURCE_REASK			0x7E
#define OP_STORED_SOURCE_GROUP_INTERVALS		0x7F
#define OP_STORED_SOURCE_GROUP_SIZE				0x80
#endif // NEO_SS // NEO: NSS END

#define OP_PR_PART_WANTED						0xA4 // NEO: MCS - [ManualChunkSelection]
// NEO: FCFG END <-- Xanatos --

// NEO: NSS - [NeoSourceStorage] -- Xanatos -->
#define SFT_SOURCE_REQFILE						0x50

#define SFT_IP									0x01
#define SFT_IP2									0x16
#define SFT_SERVER_IP							0x02
#define SFT_HYBRID_ID							0x03
#define SFT_PORT								0x04
#define SFT_SERVER_PORT							0x05
#define SFT_USER_NAME							0x06
#define SFT_USER_HASH							0x07
#define SFT_UDP_PORT							0x08
#define SFT_KAD_PORT							0x09
#define SFT_BUDDY_ID							0xB0
#define SFT_BUDDY_IP							0xB1
#define SFT_BUDDY_PORT							0xB2
#define SFT_CLIENT_SOFTWARE						0x11
#define SFT_SOFTWARE_VERSION					0x12
#define SFT_CLIENT_VERSION						0x13
#define SFT_CLIENT_MODIFICATION					0x14

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
#define SFT_IP_ZONE								0x30
#define SFT_IP_ZONE_LEVEL						0x31
#define SFT_LAST_SEEN							0x15
 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
#define SFT_STATIC_IP							0x32
#define SFT_MAX_ON_TIME							0x33
#define SFT_MID_ON_TIME							0x34
#define SFT_MIN_ON_TIME							0x35
#define SFT_MAX_OFF_TIME						0x36
#define SFT_MID_OFF_TIME						0x37
#define SFT_MIN_OFF_TIME						0x38
#define SFT_MAX_IP_TIME							0x45
#define SFT_MID_IP_TIME							0x46
#define SFT_MIN_IP_TIME							0x47
#define SFT_MAX_FAILD_COUNT						0x39
#define SFT_MID_FAILD_COUNT						0x40
#define SFT_MIN_FAILD_COUNT						0x41
#define SFT_PORT_INTEGRITY_TIME					0x51
#define SFT_LAST_SEEN_DURATION					0x48
#define SFT_TOTAL_SEEN_DURATION					0x49
#define SFT_LAST_LINK_TIME	 					0x50
#define SFT_LAST_ANALYSIS						0x42
#define SFT_ANALYSIS_QUALITY					0x43
#define SFT_ANALYSIS_NEEDED						0x44
 #endif // NEO_SA // NEO: NSA END

// ip Tables
#define IFT_IP									0x01
#define IFT_PORT								0x02
#define IFT_FIRSTSEEN							0x03
#define IFT_LASTSEEN							0x04
#define IFT_UNREACHABLE							0x05

 #ifdef NEO_CD // NEO: SFL - [SourceFileList]
#define FFT_HASH								0x01
#define FFT_SIZE								0x02
#define FFT_LASTSEEN							0x03
// NEO: SCFS - [SmartClientFileStatus]
#define FFT_FILE_NAME							0x10
#define FFT_PART_STATUS							0x11
#define FFT_INC_PART_STATUS						0x12
#define FFT_SEEN_PART_STATUS					0x13
#define FFT_SCT_STATUS							0x14
// NEO: SCFS END
 #endif // NEO_CD // NEO: SFL END
#endif // NEO_SS // NEO: NSS END
// NEO: NSS END <-- Xanatos --

