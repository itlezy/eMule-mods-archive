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

#ifdef OLD_SOCKETS_ENABLED
#include "ListenSocket.h"
#endif //OLD_SOCKETS_ENABLED
#include "PartFile.h"
#include "ClientCredits.h"
#include "BarShader.h"
#include "Loggable.h"
#include "StringConversion.h"
#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <vector>
#include <list>
#pragma warning(pop)

enum _EnumChatStates
{
	MS_NONE				= 0,
	MS_CHATTING			= 1,
	MS_CONNECTING		= 2,
	MS_UNABLETOCONNECT	= 3
};
typedef EnumDomain<_EnumChatStates>	EnumChatStates;

enum EnumSecureIdentState
{
	IS_UNAVAILABLE		= 0,
	IS_ALLREQUESTSSEND	= 0,
	IS_SIGNATURENEEDED	= 1,
	IS_KEYANDSIGNEEDED	= 2
};

//	0) The ed2k ID was created to inform about LowID client that can't be accessed
//	The ed2k ID has following format:
//		0x00000000 - 0x00FFFFFF - LowID
//		0x10000000 - 0xFFFFFFFF - IP in reversed network order
//	Initial design sacrificed "x.x.x.0" addresses for the simplicity, thus they were treated as LowID
//	1) The Hybrid ID is a modification of ed2k ID that allows to resolve this disadvantage:
//	The Hybrid ID has following format:
//		0x00000000 - 0x00FFFFFF - LowID
//		0x10000000 - 0xFFFFFFFF - IP in network order
//	NOTE: "0.x.x.x" addresses're reserved and not routed, so they can be considered as illegal
enum EnumUserIDType
{
	UID_ED2K			= 0,
	UID_HYBRID			= 1
};

enum _EnumInfoPacketState
{
	IP_NONE				= 0,
	IP_EDONKEYPROTPACK	= 1,
	IP_EMULEPROTPACK	= 2,
	IP_BOTH				= 3
};

enum _EnumBlockSendResult
{
	BSR_BUSY							= 0,		// Socket is busy, the data sending is impossible
	BSR_OK								= 1,		// All data was sent
	BSR_OK_WANTS_MORE_BANDWIDTH			= 2,		// Data was sent, there is more data
	BSR_FAILED_NOT_ENOUGHT_BANDWIDTH	= 3,		// No data was sent, because allowed bandwidth is more narrow than next packet size
	BSR_FAILED_CLIENT_LEFT_UQ			= 4,		// No data was sent & client left UPLOAD queue
	BSR_FAILED_NO_REQUESTED_BLOCKS		= 5			// No data was sent, because we don't have any block request from remote client
};
typedef EnumDomain<_EnumBlockSendResult> EnumBlockSendResult;

enum _EnumBlockPacketCreationResult
{
	BPCR_OK							= 0,		// Packet was created
	BPCR_FAILED_NO_REQUESTED_BLOCKS	= 1,		// Failed, because there're no request blocks to process
	BPCR_FAILED_BLOCKED_PART		= 2,		// Failed, because part was blocked
	BPCR_FAILED_FILE_ERROR			= 3,		// Failed, because file is blocked or does not exist
	BPCR_FAILED_TIME_LIMIT			= 4			// Failed, because client reached the limit
};
typedef EnumDomain<_EnumBlockPacketCreationResult> EnumBlockPacketCreationResult;

enum _EnumBanState
{
	BAN_CLIENT_NONE,
	BAN_CLIENT_AGGRESSIVE,
	BAN_CLIENT_SPAMMING,
	BAN_CLIENT_USE_OUR_HASH,
	BAN_CLIENT_HASH_STEALER,
	BAN_CLIENT_KWOWN_LEECHER,

	BAN_CLIENT_LAST
};
typedef EnumDomain<_EnumBanState> EnumBanState;

class EnumInfoPacketState : public EnumDomain<_EnumInfoPacketState>
{
public:
							EnumInfoPacketState() {};
							EnumInfoPacketState(const _EnumInfoPacketState eValue)
								: EnumDomain<_EnumInfoPacketState>(eValue) {}
	EnumInfoPacketState		&operator |=(const EnumInfoPacketState &e2) { m_value |= e2.m_value; return *this; }
};

typedef std::list<CUpDownClient*> ClientList;
typedef std::vector<CUpDownClient*> ClientVector;

//	D0..D13 - Update, D14..D23 - Minor version, D24..D31 - Major version
#define	FORM_CLIENT_VER(Maj, Min, Upd)	(((Maj) << 24) | ((Min) << 14) | (Upd))
#define	GET_CLIENT_MAJVER(Ver)	(((Ver) >> 24) & 0xFF)
#define	GET_CLIENT_MINVER(Ver)	(((Ver) >> 14) & 0x3FF)
#define	GET_CLIENT_UDPVER(Ver)	((Ver) & 0x3FFF)

//	Defines for actions on name change
#define AONC_FORBIDDEN_NAME_CHECK	0x01
#define AONC_COMMUNITY_CHECK		0x02

#define CLIENT_COMPARE_SAME_IP		0x01
#define CLIENT_COMPARE_SAME_ID		0x02
#define CLIENT_COMPARE_SAME_HASH	0x04

#define A4AF_ONE_WAY				0x01
#define A4AF_TO_PAUSED				0x02
#define A4AF_IGNORE_TIME_LIMIT		0x04

// predifined situation
#define A4AF_REMOVE			(A4AF_ONE_WAY | A4AF_TO_PAUSED | A4AF_IGNORE_TIME_LIMIT)

#ifdef OLD_SOCKETS_ENABLED
class CClientReqSocket;
#endif //OLD_SOCKETS_ENABLED
class CFriend;
class CSafeMemFile;
struct Pending_Block_Struct;

class CUpDownClient : public CLoggable
{
public:
#ifdef OLD_SOCKETS_ENABLED
	CClientReqSocket   *m_pRequestSocket;
#endif //OLD_SOCKETS_ENABLED

//	Upload variables
	CFriend			   *m_pFriend;
	CClientCredits	   *m_pCredits;
	uchar				m_reqFileHash[16];

//	Download variables
	CPartFile		   *m_pReqPartFile;
	CTypedPtrList<CPtrList, CPartFile*>		m_otherRequestsList;
	CTypedPtrMap<CMapPtrToWord,CPartFile*, uint32>		m_otherNoNeededMap;

private:
	typedef deque<Packet*>					PacketDeque;
	typedef deque<Requested_Block_Struct*>	ReqBlockDeque;

//	Client identification
	BYTE				m_userHash[16];
	uint16				m_uUserCountryIdx;

//	Client name & related properties
	bool				m_bHasUserNameForbiddenStrings;
	bool				m_bIsCommunity;
	CString				m_strUserName;
	byte				m_byteActionsOnNameChange;

//	MOD string & related properties
	bool				m_bIsMODNameChanged;
	CString				m_strModString;
	bool				m_bHasMODNameForbiddenStrings;

//	Remote client software identification
	EnumClientTypes		m_eClientSoft;
	CString				m_strClientSoft;
	uint32				m_dwClientVersion;
	uint32				m_dwPlusVers;
	byte				m_byteEmuleVersion;
	byte				m_byteCompatibleClient;
	bool				m_bIsHybrid;
	bool				m_bIsML;

//	Supported protocols & features
	bool				m_bIsOnLan;	// LANCAST
	bool				m_bEmuleProtocol;	// eMule protocol
	bool				m_bIsHandshakeFinished;

	byte				m_byteAcceptCommentVer;
	byte				m_byteDataCompVer;
	byte				m_byteExtendedRequestsVer;
	byte				m_byteSourceExchange1Ver;
	byte				m_byteUDPVer;
	byte				m_byteSupportSecIdent;
	ECodingFormat		m_eStrCodingFormat;

//	User IP, ports & related properties
	uint32				m_dwUserIP;			// reflect last user real IP in reversed network order
	uint32				m_dwConnectIP;	// IP (in reversed network order) used to connect to HighID clients and for country indetification
	uint32				m_dwUserIDHybrid;	// HighID: IP in network order, LowID: ID assigned by the server
	uint16				m_uUserPort;
	uint16				m_uUDPPort;
	CString				m_strFullUserIP;
	bool				m_bIsLowID;

	uint32				m_dwServerIP;
	uint16				m_uServerPort;

//	Secure identification
	EnumSecureIdentState		m_eSecureIdentState;
	EnumInfoPacketState		m_eInfoPacketsReceived;	// Have we received the edonkeyprot and emuleprot packet already (see InfoPacketsReceived())
	uint32					m_dwLastSignatureIP;

//	Request of shared files & dirs
	int					m_iFileListRequested;
	int					m_iFileListRequestedSave;

	byte				m_byteFailedCount;	// Count failed connection tries

//	Download
	EnumDLQState		m_eDownloadState;
	bool				m_bLimitlessDL;
	uint32				m_dwDownAskedCount;
	uint32				m_dwTransferredDown;
	uint32				m_dwSessionDownloadedData;
	uint64				m_qwLastBlockOffset;
	CTypedPtrList<CPtrList, Pending_Block_Struct*>		m_pendingBlocksList;
	CTypedPtrList<CPtrList, Requested_Block_Struct*>	m_downloadBlocksList;
	bool				m_bTransferredDownMini;
	bool				m_bReaskPending;
	bool				m_bUDPPending;
	byte				m_byteNumUDPPendingReqs;

//	Download: Remote queue
	int					m_iDeltaQueueRank;
	uint16				m_uRemoteQueueRank;

//	Download: requested file properties
	CString				m_strClientFilename;
	byte				*m_pbytePartStatuses;
	uint16				m_uPartCount;
	uint16				m_uLastPartAsked;
	uint16				m_uAvailPartCount;
	uint16				m_uNeededParts;	// Number of parts which can be downloaded from this source

//	Download: Speed calculation
	uint32				m_dwDownDataRate;
	std::deque<int>		m_averageDLDataRateList;
	std::deque<uint32>	m_averageDLTickList;
	uint32				m_dwClientSumDLDataRateOverLastNMeasurements;
	uint32				m_dwClientSumDLTickOverLastNMeasurements;
	uint32				m_dwLastDataRateCalculationTime;
	uint32				m_dwTransferredInLastPeriod;

//	Download: time variables
	uint32				m_dwEnteredConnectedState;
	uint32				m_dwLastAskedTime;
	uint32				m_dwNextFileReaskTime;
	uint32				m_dwDownStartTime;
	uint32				m_dwDLQueueWaitTime;
	uint32				m_dwLastSourceRequest;
	uint32				m_dwLastSourceAnswer;
	uint32				m_dwLastBlockReceived;

//	Upload
	EnumULQState		m_eUploadState;
	EnumBanState		m_eBanState;
	uint32				m_dwTransferredUp;
	uint32				m_dwCurSessionUp;
	uint32				m_dwAskedCount;
	byte				m_byteFailedFileRequestsCount;
	byte 				m_byteIncorrectBlockRequests;
	uint32				m_dwCompressionGain;
	uint32				m_dwUncompressed;

	PacketDeque			m_blockSendQueue;
	ReqBlockDeque		m_blockRequestsQueue;
	ReqBlockDeque		m_doneBlocksList;

	CTypedPtrList<CPtrList, Requested_File_Struct*>		m_requestedFilesList;

//	Upload: Speed calculation
	uint32				m_dwUpDataRate;
	std::deque<uint32>	m_averageUDRList;
	std::deque<uint32>	m_averageULTickList;

//	Parts & sources information from remote client, which is sent by every file request
	byte				*m_pbyteUpPartStatuses;
	uint16				m_uUpPartCount;
	uint16				m_uAvailUpPartCount;
	uint16				m_uUpCompleteSourcesCount;
	uint32				m_dwUpCompleteSourcesTime;

//	Upload time variables
	uint32				m_dwUploadTime;
	uint32				m_dwBanTime;
	uint32				m_dwLastGotULDataTime;
	uint32				m_dwLastUpRequest;

//	Chat
	EnumChatStates		m_eChatState;
	uint32				m_dwAwayMessageResendCount;

//	Comment & rating
	CString				m_strComment;
	EnumPartFileRating	m_eRating;
	bool				m_bCommentDirty;

//	L2HAC Mod: Low ID to High ID automatic callback
	uint32				m_dwL2HACTime;
	uint32				m_dwLastL2HACExec;
	bool				m_bL2HACEnabled;

//	Using bitfield for less important flags, to save some bytes
	uint32				m_fRequestingHashSet : 1,		// We have sent a hashset request to this client in the current connection
						m_fNoViewSharedFiles : 1,		// Client has disabled the 'View Shared Files' feature, if this flag isn't set, we just know that we don't know for sure if it's enabled
						m_fSupportsAskSharedDirs : 1,	// Client supports OP_ASKSHAREDIRS opcodes
						m_fBlocksWereRequested : 1, 	// Download block request was sent to the client
						m_fNoDataForRemoteClient : 1,	// Remote client cannot download any part from us
						m_fAddNextConnect : 1,			// LowID client which missed an attempt to get an upload slot
						m_fSupportsLargeFiles : 1,
						m_fServerWasChanged : 1,
						m_fUserInfoWasReceived : 1,
						m_fRxWrongFileRequest : 1,		// Received file request with mismatched part count
						m_fRequestsCryptLayer : 1,
						m_fSupportsCryptLayer : 1,
						m_fRequiresCryptLayer : 1,
						m_fSentCancelTransfer : 1,		// OP_CANCELTRANSFER was sent in the current session to stop downloading
						m_fSupportsSourceEx2 : 1,
						m_fSupportsMultiPacket : 1,
						m_fSupportsExtMultiPacket : 1,
						m_fPeerCache : 1,
						m_fIdenThief : 1;

public:
#ifdef OLD_SOCKETS_ENABLED
	CUpDownClient(CClientReqSocket* sender = NULL);
#endif //OLD_SOCKETS_ENABLED
	CUpDownClient(uint16 uPort, uint32 dwUserID, uint32 dwSrvIP, uint16 uSrvPort, CPartFile *pReqFile, EnumUserIDType eIDType);
	~CUpDownClient();

	uint32		Compare(CUpDownClient *pClient) const;

	bool			Disconnected(bool bRetryConnection = true);
	bool			TryToConnect(bool bIgnoreMaxCon = false);
	void			ConnectionEstablished();
//	Accessors
	uint32			GetUserIDHybrid() const		{ return m_dwUserIDHybrid; }
	void			SetUserIDHybrid(uint32 dwUserIDHyb)
	{
		m_dwUserIDHybrid = dwUserIDHyb;
		m_bIsLowID = (m_dwUserIDHybrid < 0x1000000);
	}
	bool			HasLowID() const			{ return m_bIsLowID; }

	const CString&		GetUserName() const						{ return m_strUserName; }
	void			SetUserName(const CString &strNewName)	{ m_strUserName = strNewName; }
	bool			IsUserNameEmpty() const					{ return m_strUserName.IsEmpty(); }
	int				CmpUserNames(const TCHAR *pcName2) const	{ return _tcsicmp(m_strUserName, pcName2); }
	bool			HasUserNameForbiddenStrings();
	bool			IsCommunity();

	uint32			GetIP()	const				{ return m_dwUserIP; }
	const CString&		GetFullIP() const			{ return m_strFullUserIP; }
	uint16			GetUserPort() const			{ return m_uUserPort; }
	uint16			GetUDPPort()				{ return m_uUDPPort; }

	uint32			GetTransferredUp() const	{ return m_dwTransferredUp; }
	uint32			GetTransferredDown() const	{ return m_dwTransferredDown; }

	uint32			GetServerIP() const			{ return m_dwServerIP; }
	void			SetServerIP(uint32 dwIP)	{ m_dwServerIP = dwIP; }
	uint16			GetServerPort()	const		{ return m_uServerPort; }
	void			SetServerPort(uint16 uPort)	{ m_uServerPort = uPort; }

	const uchar*	GetUserHash() const			{ return m_userHash; }
	void			SetUserHash(const uchar *pbyteUserHash)		{ md4cpy(m_userHash, pbyteUserHash); }
	int				GetHashType();
	bool			HasValidHash() const		{ return (((uint32*)m_userHash)[0] != 0) || (((uint32*)m_userHash)[1] != 0)
														|| (((uint32*)m_userHash)[2] != 0) || (((uint32*)m_userHash)[3] != 0); }

	bool			IsFriend()	const			{ return m_pFriend != NULL; }

	bool			IsOnLAN()					{ return m_bIsOnLan; }

	ECodingFormat	GetStrCodingFormat() const	{ return m_eStrCodingFormat; }

	bool			IsEmuleClient()				{ return (m_byteEmuleVersion != 0); }
	EnumClientTypes	GetClientSoft() const		{ return m_eClientSoft; }
	uint32			GetVersion() const			{ return m_dwClientVersion; }
	byte			GetMuleVersion() const		{ return m_byteEmuleVersion; }
	uint32			GetPlusVersion()			{ return m_dwPlusVers; }
	const CString&		GetModString() const		{ return m_strModString; }
	bool			IsModStringEmpty() const	{ return m_strModString.IsEmpty(); }
	bool			HasMODNameForbiddenStrings();
	const CString&		GetFullSoftVersionString() const;
	CString			GetClientNameWithSoftware() const;
	void			ReGetClientSoft();

	bool			ExtProtocolAvailable()		{ return m_bEmuleProtocol; }
	byte			GetUDPVersion() const		{ return m_byteUDPVer; }
	bool			SupportsLargeFiles() const	{ return m_fSupportsLargeFiles; }
	byte			GetExtendedRequestsVersion(){ return m_byteExtendedRequestsVer; }
	byte			GetSourceExchange1Version() const			{ return m_byteSourceExchange1Ver; }
	bool			SupportsSourceExchange2() const				{ return m_fSupportsSourceEx2; }

	void			Ban(EnumBanState eReason);
	void			UnBan();
	bool			IsBanned(bool bIgnoreDS = false) const		{ return ((m_eBanState != BAN_CLIENT_NONE) && (bIgnoreDS || m_eDownloadState != DS_DOWNLOADING)); }
	void			SendBanMessage();
	CString			GetBanString();

	const CString&		GetClientFilename() const	{ return m_strClientFilename; }
	bool			IsClientFilenameEmpty() const		{ return m_strClientFilename.IsEmpty(); }

	bool			GetViewSharedFilesSupport() const	{ return (m_fNoViewSharedFiles == 0); }
	void			RequestSharedFileList();
	void			ProcessSharedFileList(byte *pbytePacket, uint32 dwPacketSize, LPCTSTR pszDirectory = NULL);

	void			SetLastSrcReqTime()							{ m_dwLastSourceRequest = ::GetTickCount(); }
	void			SetLastSrcAnswerTime()						{ m_dwLastSourceAnswer = ::GetTickCount(); }
	uint32			GetLastSrcReqTime()							{ return m_dwLastSourceRequest; }
	uint32			GetLastSrcAnswerTime()						{ return m_dwLastSourceAnswer; }
	uint32			GetEnteredConnectedState()					{ return m_dwEnteredConnectedState; }
	void			SetCommentDirty(bool bCommentDirty = true)	{ m_bCommentDirty = bCommentDirty; }

	CClientCredits* Credits()					{ return m_pCredits; }
	EnumSecureIdentState	GetSecureIdentState() const			{ return m_eSecureIdentState; }

// Encryption
	bool			SupportsCryptLayer() const						{ return m_fSupportsCryptLayer; }
	bool			RequestsCryptLayer() const						{ return m_fRequestsCryptLayer; }
	bool			RequiresCryptLayer() const						{ return m_fRequiresCryptLayer; }
	void			SetCryptLayer(int iCryptOpt)
	{
		int	iTmp = iCryptOpt & 1;

		m_fSupportsCryptLayer = iTmp;
		iTmp &= (iCryptOpt >> 1);
		m_fRequestsCryptLayer = iTmp;
		iTmp &= (iCryptOpt >> 2);
		m_fRequiresCryptLayer = iTmp;
	}
	byte			GetCryptLayer() const
	{
		return static_cast<byte>((m_fSupportsCryptLayer & 1) | ((m_fRequestsCryptLayer ? 1 : 0) << 1) | ((m_fRequiresCryptLayer ? 1 : 0) << 2));
	}
	bool			IsObfuscatedConnectionEstablished() const;
	bool			ShouldReceiveCryptUDPPackets() const;

//	Packets sending & processing
	void			SetHandshakeStatus(bool b)			{ m_bIsHandshakeFinished = b; }
	bool			IsHandshakeFinished() const			{ return m_bIsHandshakeFinished; }
	EnumInfoPacketState	GetInfoPacketsReceived() const	{ return m_eInfoPacketsReceived; }
	void			InfoPacketsReceived();

	void			SendHelloAnswer();
	void			ProcessHelloAnswer(byte *pbytePacket, uint32 dwPacketSize);
	void			SendHelloPacket();
	bool			ProcessHelloPacket(BYTE *pbytePacket, uint32 dwPacketSize);
	void			SendCancelTransfer();
	void			SendHashsetPacket(byte *pbytePacket);
	uint32			SendBlockData(uint32 dwMaxAmount, EnumBlockSendResult &eResult);
	void			SendRankingInfo();
	void			SendBlockRequests();
	int				ProcessBlockPacket(byte *pbytePacket, uint32 dwPacketSize, bool bPacked, bool b64bOffsets);
	void			SendFileRequest();
	void			SendStartUploadRequest();
	void			SendHashsetRequest();
	void	 		ProcessFileHash(CSafeMemFile &packetStream);
	void			ProcessFileInfo(CSafeMemFile &packetStream);
	void			ProcessFileStatus(CSafeMemFile &packetStream, bool bUDPPacket = false);
	void			ProcessHashSet(byte *pbytePacket, uint32 dwPacketSize);

	void			SendMuleInfoPacket(bool bAnswer);
	void			ProcessMuleInfoPacket(BYTE *pbytePacket, uint32 dwPacketSize);
	void			ProcessMuleCommentPacket(byte *pbytePacket, uint32 dwPacketSize);
	void			SendCommentInfo(CKnownFile *pKnownFile);

	void			SendPublicKeyPacket();
	void			ProcessPublicKeyPacket(uchar *pbytePacket, uint32 dwPacketSize);
	void			SendSignaturePacket();
	void			ProcessSignaturePacket(uchar *pbytePacket, uint32 dwPacketSize);
	void			SendSecIdentStatePacket();
	void			ProcessSecIdentStatePacket(uchar *pbytePacket, uint32 dwPacketSize);

//	Upload
	EnumULQState	GetUploadState()			{ return m_eUploadState; }
	void			SetUploadState(EnumULQState eNewState);
	bool			IsDownloading() const		{ return (m_eUploadState == US_UPLOADING); }
	bool			IsInWaitingQueue() const		{ return (m_eUploadState == US_ONUPLOADQUEUE); }
//	Upload: time functions
	void			SetUpStartTime()				{ m_dwUploadTime = ::GetTickCount(); }
	uint32			GetUpStartTimeDelay()		{ return ::GetTickCount() - m_dwUploadTime; }
	uint32			GetWaitStartTime() const;
	void			SetWaitStartTime();
	uint32			GetWaitTime()				{ return (m_dwUploadTime - GetWaitStartTime()); }
	uint32			GetLastAskedDelay();
	uint32			GetBanTime()				{ return m_dwBanTime; }
	void			SetLastUpRequest()			{ m_dwLastUpRequest = ::GetTickCount(); }
	uint32			GetLastUpRequest()			{ return m_dwLastUpRequest; }
	void			SetLastGotULData()			{ m_dwLastGotULDataTime = ::GetTickCount(); }
	uint32			GetLastGotULData()			{ return m_dwLastGotULDataTime; }
//	Upload: data statistics
	uint32			GetUpDataRate() const		{ return m_dwUpDataRate; }
	uint32			GetSessionUp()				{ return m_dwTransferredUp - m_dwCurSessionUp; }
	void			ResetSessionUp()			{ m_dwCurSessionUp = m_dwTransferredUp; }
	uint32			GetScore(bool bOnlyBaseValue = false);
	double			GetCompression()	{ return static_cast<double>(m_dwCompressionGain)/m_dwUncompressed*100.0; }	// Add rod show compression
	void			ResetCompressionGain()		{ m_dwCompressionGain = 0; m_dwUncompressed=1; }	// Add show compression
//	Upload: file
	void			SetUploadFileID(uchar *pbyteReqFileID);
	uchar*			GetUploadFileID()	{ return m_reqFileHash; }
	void			AddRequestCount(uchar *pbyteFileId);
//	Upload: parts
	void			AddReqBlock(Requested_Block_Struct *pReqBlock);
	void			ClearUploadBlockRequests();
	uint32			GetCurrentlyUploadingPart();
	void			FlushSendBlocks();	// Call this when you stop upload, or the m_pRequestSocket might be not able to send
//	Parts & sources information from remote client, which is sent by every file request
	bool			ProcessExtendedInfo(byte *pbytePacket, uint32 dwPacketSz, CKnownFile *pKnownFile);
	uint16			GetUpPartCount() const		{ return m_uUpPartCount; }
	uint16			GetAvailUpPartCount() const	{ return m_uAvailUpPartCount; }
	const byte*		GetUpPartStatus() const		{ return m_pbyteUpPartStatuses; }
	void			CheckForUpPartsInfo();
	bool			IsThereDataForRemoteClient() const	{ return (m_fNoDataForRemoteClient == 0); }
	void			SetWrongFileRequest()				{ m_fRxWrongFileRequest = 1; }
	void			ResetWrongFileRequest()				{ m_fRxWrongFileRequest = 0; }
	bool			IsWrongFileRequest() const			{ return (m_fRxWrongFileRequest == 1); }
//	Upload: file request counter
	uint32			GetAskedCount()						{ return m_dwAskedCount; }
	void			AddAskedCount()						{ m_dwAskedCount++; }
	void			SetAskedCount(uint32 dwAskedCount)	{ m_dwAskedCount = dwAskedCount; }
	byte			GetFailedFileRequests()				{ return m_byteFailedFileRequestsCount; }
	void			UpdateFailedFileRequests()			{ m_byteFailedFileRequestsCount++; }
	void			ResetFailedFileRequests()			{ m_byteFailedFileRequestsCount = 0; }
// Upload: block request counter
	void			AddIncorrectBlockRequest()			{ m_byteIncorrectBlockRequests++; }
	void			ResetIncorrectBlockRequestCounter()	{ m_byteIncorrectBlockRequests = 0; }
	byte			GetIncorrectBlockRequests()			{ return m_byteIncorrectBlockRequests; }
	bool			IsAddNextConnect() const			{ return (m_fAddNextConnect == 1); }
	void			SetAddNextConnect(bool bVal)		{ m_fAddNextConnect = (bVal) ? 1 : 0; }

// Upload: source exchange
	void			LeaveSourceLists();

//	Download
	void			SetDLRequiredFile(CPartFile *pPartFile)	{ m_pReqPartFile = pPartFile; }
	EnumDLQState	GetDownloadState()			{ return m_eDownloadState; }
	void			SetDownloadState(EnumDLQState eNewState);
	void			ChangeDownloadState(EnumDLQState eNewState)	{ m_eDownloadState = eNewState; }
	void			UpdateDownloadStateAfterFileReask();
	void			UpdateOnqueueDownloadState();
	CString			GetDownloadStateAsString(void);
	uint32			DownloadProcess(uint32 dwReduceDownload);
	bool			AskForDownload();
	uint32			GetDownloadDataRate()		{ return m_dwDownDataRate; }
	void			SendReask();

	uint32			GetAskedCountDown()							{ return m_dwDownAskedCount; }
	void			AddAskedCountDown()							{ m_dwDownAskedCount++; }
	void			SetAskedCountDown(uint32 dwDownAskedCount)	{ m_dwDownAskedCount = dwDownAskedCount; }

	bool			IsUDPRequestAllowed();
	bool			IsUDPFileReqPending() const		{ return m_bUDPPending; }

	bool			WasCancelTransferSent() const	{ return m_fSentCancelTransfer; }

//	Download: time functions
	uint32			GetNextFileReaskTime() const	{ return m_dwNextFileReaskTime; }
	void			SetNextFileReaskTime();
	uint32			GetLastAskedTime() const		{ return m_dwLastAskedTime; }
	void			SetLastAskedTime();
	void			ResetLastAskedTime()			{ m_dwLastAskedTime = 0; m_dwNextFileReaskTime = 0; }
	uint32			GetDLQueueWaitTime()			{ return ::GetTickCount() - m_dwDLQueueWaitTime; }
	void			StartDLQueueWaitTimer()			{ m_dwDLQueueWaitTime = ::GetTickCount(); }
	void			UpdateLastBlockReceivedTime()	{ m_dwLastBlockReceived = ::GetTickCount(); }
//	Download: parts info
	bool			IsPartAvailable(uint32 dwPart)			{ return ((dwPart >= m_uPartCount) || (m_pbytePartStatuses == NULL)) ? false : ((m_pbytePartStatuses[dwPart] != 0) ? true : false); }
	uint16			GetClientNeededParts() const			{ return m_uNeededParts; }
	const byte*		GetPartStatus() const					{ return m_pbytePartStatuses; }
	uint16			GetPartCount() const					{ return m_uPartCount; }
	uint16			GetAvailablePartCount() const			{ return m_uAvailPartCount; }
	bool			IsCompleteSource()						{ return ((m_uAvailPartCount == m_uPartCount) && (m_uAvailPartCount != 0)); }
	uint32			GetRemainingTimeForCurrentPart();
	uint32			GetRemainingSizeForCurrentPart(uint32 *pdwDownloadingPart);
	uint16			GetLastDownPartAsked()					{ return m_uLastPartAsked; }
	void			SetLastDownPartAsked(uint16 uLastPart)	{ m_uLastPartAsked = uLastPart; }
	void			ShowDownloadingParts(std::_Bvector& parts);
//	Download: blocks
	void			ClearPendingBlocksList(void);
	void			ClearDownloadBlocksList(void);
//	Download: remote queue rank
	uint16			GetRemoteQueueRank()						{ return m_uRemoteQueueRank; }
	int				GetDifference()								{ return m_iDeltaQueueRank; }
	bool 			IsRemoteQueueFull()								{ return ((m_uRemoteQueueRank == 0) && IsEmuleClient()); }
	void			SetRemoteQueueRank(uint16 uRemoteQueueRank, bool bUpdateGUI = true);
//	Download: A4AF
	bool			AddRequestForAnotherFile(CPartFile *pPartFile);
	void			RemoveRequestForAnotherFile(CPartFile *pPartFile);
	bool			SwapToAnotherFile(CPartFile *pPartFile, uint32 dwOptions = 0);

	void			UDPReaskACK(uint16 uQueueRank);
	void			UDPReaskFNF();
	bool			UDPReaskForDownload();
	bool			IsSourceRequestAllowed();
	uint32			GetRemoteRatio();
	double			GetRemoteBaseModifier();
	uint16			GetUpCompleteSourcesCount()			{ return m_uUpCompleteSourcesCount; }
	uint32			GetUpCompleteSourcesTime()			{ return m_dwUpCompleteSourcesTime; }
	void			SetUpCompleteSourcesCount(uint16 uUpCompleteSourcesCount)	{ m_uUpCompleteSourcesCount = uUpCompleteSourcesCount; m_dwUpCompleteSourcesTime = ::GetTickCount(); }

//	Chat
	EnumChatStates		GetChatState()							{ return m_eChatState; }
	void				SetChatState(EnumChatStates eChatState)	{ m_eChatState = eChatState; }
	uint32			GetAwayMessageResendCount()					{ return m_dwAwayMessageResendCount; }
	void			SetAwayMessageResendCount(uint32 dwAwayMessageResendCount)	{ m_dwAwayMessageResendCount = dwAwayMessageResendCount; }

//	File Comment
	const CString&		GetFileComment()					{ return m_strComment; }
	bool			IsFileCommentEmpty() const				{ return m_strComment.IsEmpty(); }
	void			SetFileComment(const CString &strComment)		{ m_strComment = strComment; }
	EnumPartFileRating	GetFileRating()								{ return m_eRating; }
	void				SetFileRating(EnumPartFileRating eRating)	{ m_eRating = eRating; }

//	GUI 
	void			UpdateDisplayedInfo();
	void			DrawStatusBar(CDC *pDC, RECT *pRect, bool bOnlyGreyRect, bool bFlat);
	void			DrawUpStatusBar(CDC *pDC, RECT *pRect, bool bFlat);
	int				GetClientIconIndex() const;
	CString			GetUploadFileInfo();
	HICON			GetClientInfo4Tooltips(CString &strInfo, bool bForUpload = false);

//	Request of shared files & dirs
	int				GetFileListRequested() { return m_iFileListRequested; }
	void			SetFileListRequested(int iFileListRequested) { m_iFileListRequested = m_iFileListRequestedSave = iFileListRequested; }

//	LowID 2 High ID callback request feature
	uint32			GetL2HACTime()								{ return m_dwL2HACTime ? (m_dwL2HACTime - L2HAC_CALLBACK_PRECEDE) : 0; }
	void			SetLastL2HACExecution(uint32 dwLastL2HACExec = 0)	{ m_dwLastL2HACExec = dwLastL2HACExec ? dwLastL2HACExec : ::GetTickCount(); }
	uint32			GetLastL2HACExecution()						{ return m_dwLastL2HACExec; }
	void			EnableL2HAC()								{ m_bL2HACEnabled = true; }
	void			DisableL2HAC()								{ m_bL2HACEnabled = false; }
	bool			IsL2HACEnabled()							{ return m_bL2HACEnabled; }

//	Country feature
	CString			GetCountryName() const;
	int				GetCountryIndex() const			{ return m_uUserCountryIdx; }
	void			ResetIP2Country();

private:
	void			Init();
//	Hello packet
	bool			ProcessHelloTypePacket(CSafeMemFile &packetStream);
	void			SendHelloTypePacket(CMemFile &packetStream);
	void			ClearHelloProperties();

	int				unzip(Pending_Block_Struct *pRequestedBlock, BYTE *pbyteZipped, uint32 dwLenZipped, uint32 *pdwLenUnzipped);

//	Upload packet creation
	EnumBlockPacketCreationResult	CreateNextBlockPackage();
	bool			CreateStandardPackets(byte *pbyteFileData, uint32 dwToGo, Requested_Block_Struct *pCurrentBlock, byte byteFilePriority, bool bFromPF = true);
	bool			CreatePackedPackets(byte *pbyteFileData, uint32 dwToGo, Requested_Block_Struct *pCurrentBlock, byte byteFilePriority, bool bFromPF = true);
	bool			SplitAndAddPacketsToSendQueue(Packet *pPacket, PacketDeque *pSendQueue, byte byteFilePriority, bool bFromPF);
	int				IsDifferentPartBlock(bool *pbJSBlocked);

//	Misc
	void			ClearPartStatuses();	// delete allocated array & related flags
};
