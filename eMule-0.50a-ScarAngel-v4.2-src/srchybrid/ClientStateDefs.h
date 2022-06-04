//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

enum EUploadState{
	US_UPLOADING,
	US_ONUPLOADQUEUE,
	US_CONNECTING,
	US_BANNED,
	US_NONE
};

enum EDownloadState{
	DS_DOWNLOADING,
	DS_ONQUEUE,
	DS_CONNECTED,
	DS_CONNECTING,
	DS_WAITCALLBACK,
	DS_WAITCALLBACKKAD,
	DS_REQHASHSET,
	DS_NONEEDEDPARTS,
	DS_TOOMANYCONNS,
	DS_TOOMANYCONNSKAD,
	DS_LOWTOLOWIP,
	DS_BANNED,
	DS_ERROR,
	DS_NONE,
	DS_REMOTEQUEUEFULL  // not used yet, except in statistics
};

enum EPeerCacheDownState{
	PCDS_NONE = 0,
	PCDS_WAIT_CLIENT_REPLY,
	PCDS_WAIT_CACHE_REPLY,
	PCDS_DOWNLOADING
};

enum EPeerCacheUpState{
	PCUS_NONE = 0,
	PCUS_WAIT_CACHE_REPLY,
	PCUS_UPLOADING
};

enum EChatState{
	MS_NONE,
	MS_CHATTING,
	MS_CONNECTING,
	MS_UNABLETOCONNECT
};

enum EKadState{
	KS_NONE,
	KS_QUEUED_FWCHECK,
	KS_CONNECTING_FWCHECK,
	KS_CONNECTED_FWCHECK,
	KS_QUEUED_BUDDY,
	KS_INCOMING_BUDDY,
	KS_CONNECTING_BUDDY,
	KS_CONNECTED_BUDDY,
	KS_QUEUED_FWCHECK_UDP,
	KS_FWCHECK_UDP,
	KS_CONNECTING_FWCHECK_UDP
};

enum EClientSoftware{
	SO_EMULE			= 0,	// default
	SO_CDONKEY			= 1,	// ET_COMPATIBLECLIENT
	SO_XMULE			= 2,	// ET_COMPATIBLECLIENT
	SO_AMULE			= 3,	// ET_COMPATIBLECLIENT
	SO_SHAREAZA			= 4,	// ET_COMPATIBLECLIENT
	// ==> Enhanced Client Recognition [Spike] - Stulle
	SO_EMULEPLUS			= 5,
	SO_HYDRANODE			= 6,
	// <== Enhanced Client Recognition [Spike] - Stulle
	SO_MLDONKEY			= 10,	// ET_COMPATIBLECLIENT
	SO_LPHANT			= 20,	// ET_COMPATIBLECLIENT
	// ==> Enhanced Client Recognition [Spike] - Stulle
	SO_SHAREAZA2			= 28,
	SO_TRUSTYFILES			= 30,
	SO_SHAREAZA3			= 40,
	// <== Enhanced Client Recognition [Spike] - Stulle
	// other client types which are not identified with ET_COMPATIBLECLIENT
	SO_EDONKEYHYBRID	= 50,
	// ==> Enhanced Client Recognition [Spike] - Stulle
	/*
	SO_EDONKEY,
	SO_OLDEMULE,
	*/
	SO_EDONKEY			= 51,
	SO_MLDONKEY2		= 52,
	SO_OLDEMULE			= 53,
	SO_SHAREAZA4		= 68,
	SO_MLDONKEY3		= 152,
	// <== Enhanced Client Recognition [Spike] - Stulle
	SO_URL,
	SO_UNKNOWN
};

enum ESecureIdentState{
	IS_UNAVAILABLE		= 0,
	IS_ALLREQUESTSSEND  = 0,
	IS_SIGNATURENEEDED	= 1,
	IS_KEYANDSIGNEEDED	= 2,
};

enum EInfoPacketState{
	IP_NONE				= 0,
	IP_EDONKEYPROTPACK  = 1,
	IP_EMULEPROTPACK	= 2,
	IP_BOTH				= 3,
};

enum ESourceFrom{
	SF_SERVER			= 0,
	SF_KADEMLIA			= 1,
	SF_SOURCE_EXCHANGE	= 2,
	SF_PASSIVE			= 3,
	SF_LINK				= 4,
	SF_SLS				= 5			//Xman SLS
};

enum EChatCaptchaState{
	CA_NONE				= 0,
	CA_CHALLENGESENT,
	CA_CAPTCHASOLVED,
	CA_ACCEPTING,
	CA_CAPTCHARECV,
	CA_SOLUTIONSENT
};

enum EConnectingState{
	CCS_NONE				= 0,
	CCS_DIRECTTCP,
	CCS_DIRECTCALLBACK,
	CCS_KADCALLBACK,
	CCS_SERVERCALLBACK,
	CCS_PRECONDITIONS
};

// ==> See chunk that we hide [SiRoB] - Stulle
enum EChunkStatus{
	SC_AVAILABLE		= 1,
	SC_HIDDENBYSOTN		= 2,
	SC_HIDDENBYHIDEOS	= 4/*,
	SC_PARTIAL			= 8 //MORPH - Added By SiRoB, ICS merged into partstatus*/
};
// <== See chunk that we hide [SiRoB] - Stulle

#ifdef _DEBUG
	// use the 'Enums' only for debug builds, each enum costs 4 bytes (3 unused)
#define _EClientSoftware	EClientSoftware
#define _EChatState			EChatState
#define _EKadState			EKadState
#define _ESecureIdentState	ESecureIdentState
#define _EUploadState		EUploadState
#define _EDownloadState		EDownloadState
#define _ESourceFrom		ESourceFrom
#define _EChatCaptchaState  EChatCaptchaState
#define _EConnectingState	EConnectingState
#define _EModClient			EModClient // Mod Icons - Stulle
#else
#define _EClientSoftware	uint8
#define _EChatState			uint8
#define _EKadState			uint8
#define _ESecureIdentState	uint8
#define _EUploadState		uint8
#define _EDownloadState		uint8
#define _ESourceFrom		uint8
#define _EChatCaptchaState	uint8
#define _EConnectingState	uint8
#define _EModClient			uint8 // Mod Icons - Stulle
#endif
// ==> Mod Icons - Stulle
enum EModClient{
	MOD_NONE,
	MOD_SCAR,
	MOD_STULLE,
	MOD_XTREME,
	MOD_MORPH,
	MOD_EASTSHARE,
	MOD_EMF,
	MOD_NEO,
	MOD_MEPHISTO,
	MOD_XRAY,
	MOD_MAGIC
};
// <== Mod Icons - Stulle

// ==> FunnyNick [SiRoB/Stulle] - Stulle
enum FnTagSelection {
	CS_NONE = 0,
	CS_SHORT,
	CS_FULL,
	CS_CUST
};
// <== FunnyNick [SiRoB/Stulle] - Stulle

// ==> Anti Uploader Ban [Stulle] - Stulle
enum AntiUploaderBanCaseSelection {
	CS_1 = 0,
	CS_2,
	CS_3
};
// <== Anti Uploader Ban [Stulle] - Stulle
