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
//Xman
/*
#include "BarShader.h"
*/
//Xman end
#include "ClientStateDefs.h"
//Xman
#include "otherfunctions.h"
#include <map>
#include "IP2Country.h" //EastShare - added by AndCycle, IP to Country
#include "Preferences.h" //Xman Funny-Nick (Stulle/Morph)

class CTag; //Xman Anti-Leecher
class CClientReqSocket;
class CPeerCacheDownSocket;
class CPeerCacheUpSocket;
class CFriend;
class CPartFile;
class CClientCredits;
class CAbstractFile;
class CKnownFile;
class Packet;
class CxImage;
struct Requested_Block_Struct;
class CSafeMemFile;
class CEMSocket;
class CAICHHash;
enum EUtf8Str;

struct Pending_Block_Struct{
	Pending_Block_Struct()
	{
		block = NULL;
		zStream = NULL;
		totalUnzipped = 0;
		fZStreamError = 0;
		fRecovered = 0;
		fQueued = 0;
	}
	Requested_Block_Struct*	block;
	struct z_stream_s*      zStream;       // Barry - Used to unzip packets
	UINT					totalUnzipped; // Barry - This holds the total unzipped bytes for all packets so far
	UINT					fZStreamError : 1,
							fRecovered    : 1,
							fQueued		  : 3;
};

//Xman
// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#pragma pack(4 /*Bytes*/)
struct TransferredData {
	UINT	dataLength;
	DWORD	timeStamp; // => based on enkeyDEV(Ottavio84)
};
//Xman end

//Xman
// BEGIN SiRoB: ReadBlockFromFileThread
class CUpDownClient;
class CReadBlockFromFileThread : public CWinThread
{
	DECLARE_DYNCREATE(CReadBlockFromFileThread)
protected:
	CReadBlockFromFileThread()	{}
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void			SetReadBlockFromFile(LPCTSTR filepath, uint64 startOffset, uint32 togo, CUpDownClient* client, CSyncObject* lockhandle);
	void			StopReadBlock();
private:
	uint64			StartOffset;
	uint32			togo;
	CUpDownClient*	m_client;
	CString			m_clientname; //Fafner: avoid possible crash - 080421
	CString			fullname;
	CSyncObject*	m_lockhandle;
	CEvent			pauseEvent;
	volatile bool	doRun;
};
// END SiRoB: ReadBlockFromFileThread

#pragma pack(1)
struct Requested_File_Struct{
	uchar	  fileid[16];
	uint32	  lastasked;
	uint8	  badrequests;
};
#pragma pack()

struct PartFileStamp{
	CPartFile*	file;
	DWORD		timestamp;
};

#define	MAKE_CLIENT_VERSION(mjr, min, upd) \
	((UINT)(mjr)*100U*10U*100U + (UINT)(min)*100U*10U + (UINT)(upd)*100U)

//#pragma pack(2)
class CUpDownClient : public CObject
{
	DECLARE_DYNAMIC(CUpDownClient)

	friend class CUploadQueue;
public:
    void PrintUploadStatus();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Base
	CUpDownClient(CClientReqSocket* sender = 0);
	CUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid, uint32 in_serverup, uint16 in_serverport, bool ed2kID = false);
	virtual ~CUpDownClient();

	// Maella -Upload Stop Reason-
	enum UpStopReason {USR_NONE, USR_SOCKET, USR_COMPLETEDRANSFER, USR_CANCELLED, USR_DIFFERENT_FILE, USR_BLOCKING, USR_EXCEPTION };
	// Maella -Download Stop Reason-
	enum DownStopReason {DSR_NONE, DSR_PAUSED, DSR_NONEEDEDPARTS, DSR_CORRUPTEDBLOCK, DSR_TIMEOUT, DSR_SOCKET, DSR_OUTOFPART, DSR_EXCEPTION};

	void			StartDownload();
	virtual void	CheckDownloadTimeout();
	virtual void	SendCancelTransfer(Packet* packet = NULL);
	virtual bool	IsEd2kClient() const							{ return true; }
	// Maella -Upload Stop Reason-
	/*
	virtual bool	Disconnected(LPCTSTR pszReason, bool bFromSocket = false);
	*/
	virtual bool	Disconnected(LPCTSTR pszReason, bool bFromSocket = false, UpStopReason reason = USR_NONE);
	//Xman end
	virtual bool	TryToConnect(bool bIgnoreMaxCon = false, bool bNoCallbacks = false, CRuntimeClass* pClassSocket = NULL);
	virtual void	Connect();
	virtual void	ConnectionEstablished();
	virtual void	OnSocketConnected(int nErrorCode);
	bool			CheckHandshakeFinished() const;
	void			CheckFailedFileIdReqs(const uchar* aucFileHash);
	uint32			GetUserIDHybrid() const							{ return m_nUserIDHybrid; }
	void			SetUserIDHybrid(uint32 val)						{ m_nUserIDHybrid = val; }
	//Xman Funny-Nick (Stulle/Morph)
	/*
	LPCTSTR			GetUserName() const								{ return m_pszUsername; }
	*/
	// ==> No FunnyNick for bad guys [Stulle] - Stulle
	/*
	LPCTSTR			GetUserName() const								{ return (thePrefs.DisplayFunnyNick() && m_pszFunnyNick)?m_pszFunnyNick:m_pszUsername; }
	*/
	LPCTSTR			GetUserName() const								{ return (thePrefs.DisplayFunnyNick() && m_bLeecher==0 && m_pszFunnyNick)?m_pszFunnyNick:m_pszUsername; }
	// <== No FunnyNick for bad guys [Stulle] - Stulle
	//Xman end

	void			SetUserName(LPCTSTR pszNewName);
	uint32			GetIP() const									{ return m_dwUserIP; }
	void			SetIP( uint32 val ) //Only use this when you know the real IP or when your clearing it.
						{
							m_dwUserIP = val;
							m_nConnectIP = val;
							m_szFullUserIP= ipstr(m_nConnectIP); //Xman
						}
	__inline bool	HasLowID() const								{ return (m_nUserIDHybrid < 16777216); }
	uint32			GetConnectIP() const							{ return m_nConnectIP; }
	uint16			GetUserPort() const								{ return m_nUserPort; }
	void			SetUserPort(uint16 val)							{ m_nUserPort = val; }
	UINT			GetTransferredUp() const						{ return m_nTransferredUp; }
	UINT			GetTransferredDown() const						{ return m_nTransferredDown; }
	uint32			GetServerIP() const								{ return m_dwServerIP; }
	void			SetServerIP(uint32 nIP)							{ m_dwServerIP = nIP; }
	uint16			GetServerPort() const							{ return m_nServerPort; }
	void			SetServerPort(uint16 nPort)						{ m_nServerPort = nPort; }
	const uchar*	GetUserHash() const								{ return (uchar*)m_achUserHash; }
	void			SetUserHash(const uchar* pUserHash);
	bool			HasValidHash() const
						{
							return ((const uint32*)m_achUserHash)[0] != 0 || ((const uint32*)m_achUserHash)[1] != 0 || ((const uint32*)m_achUserHash)[2] != 0 || ((const uint32*)m_achUserHash)[3] != 0;
						}
	int				GetHashType() const;
	const uchar*	GetBuddyID() const								{ return (uchar*)m_achBuddyID; }
	void			SetBuddyID(const uchar* m_achTempBuddyID);
	bool			HasValidBuddyID() const							{ return m_bBuddyIDValid; }
	void			SetBuddyIP( uint32 val )						{ m_nBuddyIP = val; }
	uint32			GetBuddyIP() const								{ return m_nBuddyIP; }
	void			SetBuddyPort( uint16 val )						{ m_nBuddyPort = val; }
	uint16			GetBuddyPort() const							{ return m_nBuddyPort; }
	EClientSoftware	GetClientSoft() const							{ return (EClientSoftware)m_clientSoft; }
	const CString&	GetClientSoftVer() const						{ return m_strClientSoftware; }
	const CString&	GetClientModVer() const							{ return m_strModVersion; }
	void			InitClientSoftwareVersion();
	UINT			GetVersion() const								{ return m_nClientVersion; }
	uint8			GetMuleVersion() const							{ return m_byEmuleVersion; }
	bool			ExtProtocolAvailable() const					{ return m_bEmuleProtocol; }
	bool			SupportMultiPacket() const						{ return m_bMultiPacket; }
	bool			SupportExtMultiPacket() const					{ return m_fExtMultiPacket; }
	bool			SupportPeerCache() const						{ return m_fPeerCache; }
	bool			SupportsLargeFiles() const						{ return m_fSupportsLargeFiles; }
	bool			SupportsFileIdentifiers() const					{ return m_fSupportsFileIdent; }
	bool			IsEmuleClient() const							{ return m_byEmuleVersion!=0; }
	uint8			GetSourceExchange1Version() const				{ return m_bySourceExchange1Ver; }
	bool			SupportsSourceExchange2() const					{ return m_fSupportsSourceEx2; }
	CClientCredits* Credits() const									{ return credits; }
	bool			IsBanned() const;
	const CString&	GetClientFilename() const						{ return m_strClientFilename; }
	void			SetClientFilename(const CString& fileName)		{ m_strClientFilename = fileName; }
	uint16			GetUDPPort() const								{ return m_nUDPPort; }
	void			SetUDPPort(uint16 nPort)						{ m_nUDPPort = nPort; }
	uint8			GetUDPVersion() const							{ return m_byUDPVer; }
	bool			SupportsUDP() const								{ return GetUDPVersion() != 0 && m_nUDPPort != 0; }
	uint16			GetKadPort() const								{ return m_nKadPort; }
	void			SetKadPort(uint16 nPort)						{ m_nKadPort = nPort; }
	uint8			GetExtendedRequestsVersion() const				{ return m_byExtendedRequestsVer; }
	void			RequestSharedFileList();
	void			ProcessSharedFileList(const uchar* pachPacket, UINT nSize, LPCTSTR pszDirectory = NULL);
	EConnectingState GetConnectingState() const						{ return (EConnectingState)m_nConnectingState; }

	void			ClearHelloProperties();
	bool			ProcessHelloAnswer(const uchar* pachPacket, UINT nSize);
	bool			ProcessHelloPacket(const uchar* pachPacket, UINT nSize);
	void			SendHelloAnswer();
	virtual void	SendHelloPacket();
	void			SendMuleInfoPacket(bool bAnswer);
	void			ProcessMuleInfoPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessMuleCommentPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessEmuleQueueRank(const uchar* packet, UINT size);
	void			ProcessEdonkeyQueueRank(const uchar* packet, UINT size);
	void			CheckQueueRankFlood();
	bool			Compare(const CUpDownClient* tocomp, bool bIgnoreUserhash = false) const;
	void			ResetFileStatusInfo();
	uint32			GetLastSrcReqTime() const						{ return m_dwLastSourceRequest; }
	void			SetLastSrcReqTime()								{ m_dwLastSourceRequest = ::GetTickCount(); }
	uint32			GetLastSrcAnswerTime() const					{ return m_dwLastSourceAnswer; }
	void			SetLastSrcAnswerTime()							{ m_dwLastSourceAnswer = ::GetTickCount(); }
	uint32			GetLastAskedForSources() const					{ return m_dwLastAskedForSources; }
	void			SetLastAskedForSources()						{ m_dwLastAskedForSources = ::GetTickCount(); }
	bool			GetFriendSlot() const;
	// ==> Mephisto Upload - Mephisto
	/*
	void			SetFriendSlot(bool bNV)							{ m_bFriendSlot = bNV; }
	*/
	void			SetFriendSlot(bool bNV);
	// <== Mephisto Upload - Mephisto
	bool			IsFriend() const								{ return m_Friend != NULL; }
	CFriend*		GetFriend() const;
	void			SetCommentDirty(bool bDirty = true)				{ m_bCommentDirty = bDirty; }
	bool			GetSentCancelTransfer() const					{ return m_fSentCancelTransfer; }
	void			SetSentCancelTransfer(bool bVal)				{ m_fSentCancelTransfer = bVal; }
	void			ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize);
	void			SendPublicIPRequest();
	uint8			GetKadVersion()	const							{ return m_byKadVersion; }
	bool			SendBuddyPingPong()								{ return m_dwLastBuddyPingPongTime < ::GetTickCount(); }
	bool			AllowIncomeingBuddyPingPong()					{ return m_dwLastBuddyPingPongTime < (::GetTickCount()-(3*60*1000)); }
	void			SetLastBuddyPingPongTime()						{ m_dwLastBuddyPingPongTime = (::GetTickCount()+(10*60*1000)); }
	void			ProcessFirewallCheckUDPRequest(CSafeMemFile* data);
	void			SendSharedDirectories();

	// secure ident
	void			SendPublicKeyPacket();
	void			SendSignaturePacket();
	void			ProcessPublicKeyPacket(const uchar* pachPacket, UINT nSize);
	void			ProcessSignaturePacket(const uchar* pachPacket, UINT nSize);
	uint8			GetSecureIdentState() const						{ return (uint8)m_SecureIdentState; }
	void			SendSecIdentStatePacket();
	void			ProcessSecIdentStatePacket(const uchar* pachPacket, UINT nSize);
	uint8			GetInfoPacketsReceived() const					{ return m_byInfopacketsReceived; }
	void			InfoPacketsReceived();
	bool			HasPassedSecureIdent(bool bPassIfUnavailable) const;
	// preview
	void			SendPreviewRequest(const CAbstractFile* pForFile);
	void			SendPreviewAnswer(const CKnownFile* pForFile, CxImage** imgFrames, uint8 nCount);
	void			ProcessPreviewReq(const uchar* pachPacket, UINT nSize);
	void			ProcessPreviewAnswer(const uchar* pachPacket, UINT nSize);
	bool			GetPreviewSupport() const						{ return m_fSupportsPreview && GetViewSharedFilesSupport(); }
	bool			GetViewSharedFilesSupport() const				{ return m_fNoViewSharedFiles==0; }
	bool			SafeConnectAndSendPacket(Packet* packet);
	bool			SendPacket(Packet* packet, bool bDeletePacket, bool bVerifyConnection = false);
	void			CheckForGPLEvilDoer();
	// Encryption / Obfuscation / Connectoptions
	bool			SupportsCryptLayer() const						{ return m_fSupportsCryptLayer; }
	bool			RequestsCryptLayer() const						{ return SupportsCryptLayer() && m_fRequestsCryptLayer; }
	bool			RequiresCryptLayer() const						{ return RequestsCryptLayer() && m_fRequiresCryptLayer; }
	bool			SupportsDirectUDPCallback() const				{ return m_fDirectUDPCallback != 0 && HasValidHash() && GetKadPort() != 0; }
	void			SetCryptLayerSupport(bool bVal)					{ m_fSupportsCryptLayer = bVal ? 1 : 0; }
	void			SetCryptLayerRequest(bool bVal)					{ m_fRequestsCryptLayer = bVal ? 1 : 0; }
	void			SetCryptLayerRequires(bool bVal)				{ m_fRequiresCryptLayer = bVal ? 1 : 0; }
	void			SetDirectUDPCallbackSupport(bool bVal)			{ m_fDirectUDPCallback = bVal ? 1 : 0; }
	void			SetConnectOptions(uint8 byOptions, bool bEncryption = true, bool bCallback = true); // shortcut, sets crypt, callback etc based from the tagvalue we recieve
	bool			IsObfuscatedConnectionEstablished() const;
	bool			ShouldReceiveCryptUDPPackets() const;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Upload
	EUploadState	GetUploadState() const							{ return (EUploadState)m_nUploadState; }
	void			SetUploadState(EUploadState news);
	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	/*
	uint32			GetWaitStartTime() const;
	*/
	sint64			GetWaitStartTime() const;
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	void 			SetWaitStartTime();
	void 			ClearWaitStartTime();
	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	/*
	uint32			GetWaitTime() const								{ return m_dwUploadTime - GetWaitStartTime(); }
	*/
	uint32			GetWaitTime() const								{ return (uint32)(m_dwUploadTime - GetWaitStartTime()); }
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	bool			IsDownloading() const							{ return (m_nUploadState == US_UPLOADING); }
	bool			HasBlocks() const								{ return !m_BlockRequests_queue.IsEmpty(); }
	UINT            GetNumberOfRequestedBlocksInQueue() const       { return m_BlockRequests_queue.GetCount(); }
	//Xman
	/*
	UINT			GetDatarate() const								{ return m_nUpDatarate; }	
	*/
	//Xman end
	UINT			GetScore(bool sysvalue, bool isdownloading = false, bool onlybasevalue = false) const;
	void			AddReqBlock(Requested_Block_Struct* reqblock);
	//Xman for SiRoB: ReadBlockFromFileThread
	/*
	void			CreateNextBlockPackage(bool bBigBuffer = false);
	*/
	void			CreateNextBlockPackage();
	//Xman end
	uint32			GetUpStartTimeDelay() const						{ return ::GetTickCount() - m_dwUploadTime; }
	void 			SetUpStartTime()								{ m_dwUploadTime = ::GetTickCount(); }
	void			SendHashsetPacket(const uchar* pData, uint32 nSize, bool bFileIdentifiers);
	const uchar*	GetUploadFileID() const							{ return requpfileid; }
	void			SetUploadFileID(CKnownFile* newreqfile);
	UINT			SendBlockData();
	//Xman - Fix Filtered Block Request
	/*
	void			ClearUploadBlockRequests();
	*/
	void			ClearUploadBlockRequests(bool truncatequeues=true);
	//Xman end
	void			SendRankingInfo();
	void			SendCommentInfo(/*const*/ CKnownFile *file);
	void			AddRequestCount(const uchar* fileid);
	void			UnBan();
	void			Ban(LPCTSTR pszReason = NULL);
	UINT			GetAskedCount() const							{ return m_cAsked; }
	void			AddAskedCount()									{ m_cAsked++; }
	void			SetAskedCount(UINT m_cInAsked)				{ m_cAsked = m_cInAsked; }
	void			FlushSendBlocks(); // call this when you stop upload, or the socket might be not able to send
	uint32			GetLastUpRequest() const						{ return m_dwLastUpRequest; }
	void			SetLastUpRequest()								{ m_dwLastUpRequest = ::GetTickCount(); }
	void			SetCollectionUploadSlot(bool bValue);
	bool			HasCollectionUploadSlot() const					{ return m_bCollectionUploadSlot; }

	UINT			GetSessionUp() const							{ return m_nTransferredUp - m_nCurSessionUp; }
	void			ResetSessionUp() {
						m_nCurSessionUp = m_nTransferredUp;
						m_addedPayloadQueueSession = 0;
						m_nCurQueueSessionPayloadUp = 0;
					}

	UINT			GetSessionDown() const							{ return m_nTransferredDown - m_nCurSessionDown; }
	UINT            GetSessionPayloadDown() const                   { return m_nCurSessionPayloadDown; }
	void			ResetSessionDown() {
						m_nCurSessionDown = m_nTransferredDown;
						m_nCurSessionPayloadDown = 0;
					}
	UINT			GetQueueSessionPayloadUp() const				{ return m_nCurQueueSessionPayloadUp; }
    UINT			GetPayloadInBuffer() const						{ return m_addedPayloadQueueSession - GetQueueSessionPayloadUp(); }

	//Xman better passive source finding
	/*
	bool			ProcessExtendedInfo(CSafeMemFile* packet, CKnownFile* tempreqfile);
	*/
	bool			ProcessExtendedInfo(CSafeMemFile* packet, CKnownFile* tempreqfile, bool isUDP=false);
	//Xman end
	uint16			GetUpPartCount() const							{ return m_nUpPartCount; }
	void			DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const;
	bool			IsUpPartAvailable(UINT iPart) const {
						// ==> See chunk that we hide [SiRoB] - Stulle
						/*
						return (iPart >= m_nUpPartCount || !m_abyUpPartStatus) ? false : m_abyUpPartStatus[iPart] != 0;
						*/
						return (iPart>=m_nUpPartCount || !m_abyUpPartStatus) ? false : m_abyUpPartStatus[iPart]&SC_AVAILABLE;
						// <== See chunk that we hide [SiRoB] - Stulle
					}
	uint8*			GetUpPartStatus() const							{ return m_abyUpPartStatus; }
    float           GetCombinedFilePrioAndCredit();

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Download
	UINT			GetAskedCountDown() const						{ return m_cDownAsked; }
	void			AddAskedCountDown()								{ m_cDownAsked++; }
	void			SetAskedCountDown(UINT cInDownAsked)			{ m_cDownAsked = cInDownAsked; }
	EDownloadState	GetDownloadState() const						{ return (EDownloadState)m_nDownloadState; }
	//Xman // Maella -Download Stop Reason-
	/*
	void			SetDownloadState(EDownloadState nNewState, LPCTSTR pszReason = _T("Unspecified"));
	uint32			GetLastAskedTime(const CPartFile* partFile = NULL) const;
    void            SetLastAskedTime()								{ m_fileReaskTimes.SetAt(reqfile, ::GetTickCount()); }
	*/
	void			SetDownloadState(EDownloadState nNewState, LPCTSTR pszReason = _T("Unspecified"),DownStopReason reason = DSR_NONE);
	//Xman end
	bool			IsPartAvailable(UINT iPart) const {
						return (iPart >= m_nPartCount || !m_abyPartStatus) ? false : m_abyPartStatus[iPart] != 0;
					}
	uint8*			GetPartStatus() const							{ return m_abyPartStatus; }
	uint16			GetPartCount() const							{ return m_nPartCount; }
	//Xman
	/*
	UINT			GetDownloadDatarate() const						{ return m_nDownDatarate; }
	UINT			GetRemoteQueueRank() const						{ return m_nRemoteQueueRank; }
	void			SetRemoteQueueRank(UINT nr, bool bUpdateDisplay = false);
	*/
	UINT			GetRemoteQueueRank() const						{ return m_nRemoteQueueRank; }
	//Xman end
	bool			IsRemoteQueueFull() const						{ return m_bRemoteQueueFull; }
	void			SetRemoteQueueFull(bool flag)					{ m_bRemoteQueueFull = flag; }
	void			DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const;
	bool			AskForDownload();
	virtual void	SendFileRequest();
	void			SendStartupLoadReq();
	void			ProcessFileInfo(CSafeMemFile* data, CPartFile* file);
	void			ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file);
	void			ProcessHashSet(const uchar* data, UINT size, bool bFileIdentifiers);
	void			ProcessAcceptUpload();
	bool			AddRequestForAnotherFile(CPartFile* file);
	void			CreateBlockRequests(int iMaxBlocks);
	virtual void	SendBlockRequests();
	virtual bool	SendHttpBlockRequests();
	virtual void	ProcessBlockPacket(const uchar* packet, UINT size, bool packed, bool bI64Offsets);
	virtual void	ProcessHttpBlockPacket(const BYTE* pucData, UINT uSize);
	void			ClearDownloadBlockRequests();
	//Xtreme Full Chunk
	/*
	void			SendOutOfPartReqsAndAddToWaitingQueue();
	UINT			CalculateDownloadRate();
	*/
	void			SendOutOfPartReqsAndAddToWaitingQueue(bool givebonus=false);
	//Xman end
	uint16			GetAvailablePartCount() const;
	//Xman
	/*
	bool			SwapToAnotherFile(LPCTSTR pszReason, bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile = NULL, bool allowSame = true, bool isAboutToAsk = false, bool debug = false); // ZZ:DownloadManager
	void			DontSwapTo(CPartFile* file);
	bool			IsSwapSuspended(const CPartFile* file, const bool allowShortReaskTime = false, const bool fileIsNNP = false) /*const*//*; // ZZ:DownloadManager
    uint32          GetTimeUntilReask() const;
    uint32          GetTimeUntilReask(const CPartFile* file) const;
    uint32			GetTimeUntilReask(const CPartFile* file, const bool allowShortReaskTime, const bool useGivenNNP = false, const bool givenNNP = false) const;
	*/
	//Xman end
	void			UDPReaskACK(uint16 nNewQR);
	void			UDPReaskFNF();
	void			UDPReaskForDownload();
	bool			UDPPacketPending() const						{ return m_bUDPPending; }
	bool			IsSourceRequestAllowed() const;
    bool            IsSourceRequestAllowed(CPartFile* partfile, bool sourceExchangeCheck = false) const; // ZZ:DownloadManager

	bool			IsValidSource() const;
	ESourceFrom		GetSourceFrom() const							{ return (ESourceFrom)m_nSourceFrom; }
	void			SetSourceFrom(ESourceFrom val)					{ m_nSourceFrom = (_ESourceFrom)val; }

	void			SetDownStartTime()								{ m_dwDownStartTime = ::GetTickCount(); }
	uint32			GetDownTimeDifference(boolean clear = true)	{
						uint32 myTime = m_dwDownStartTime;
						if(clear) m_dwDownStartTime = 0;
						return ::GetTickCount() - myTime;
					}
	bool			GetTransferredDownMini() const					{ return m_bTransferredDownMini; }
	void			SetTransferredDownMini()						{ m_bTransferredDownMini = true; }
	void			InitTransferredDownMini()						{ m_bTransferredDownMini = false; }
	UINT			GetA4AFCount() const							{ return m_OtherRequests_list.GetCount(); }

	uint16			GetUpCompleteSourcesCount() const				{ return m_nUpCompleteSourcesCount; }
	void			SetUpCompleteSourcesCount(uint16 n)				{ m_nUpCompleteSourcesCount = n; }

	///////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Chat
	EChatState		GetChatState() const							{ return (EChatState)m_nChatstate; }
	void			SetChatState(EChatState nNewS)					{ m_nChatstate = (_EChatState)nNewS; }
	EChatCaptchaState GetChatCaptchaState() const					{ return (EChatCaptchaState)m_nChatCaptchaState; }
	void			SetChatCaptchaState(EChatCaptchaState nNewS)	{ m_nChatCaptchaState = (_EChatCaptchaState)nNewS; }
	void			ProcessChatMessage(CSafeMemFile* data, uint32 nLength);
	void			SendChatMessage(CString strMessage);
	void			ProcessCaptchaRequest(CSafeMemFile* data);
	void			ProcessCaptchaReqRes(uint8 nStatus);
	// message filtering
	uint8			GetMessagesReceived() const						{ return m_cMessagesReceived; }
	void			SetMessagesReceived(uint8 nCount)				{ m_cMessagesReceived = nCount; }
	void			IncMessagesReceived()							{ m_cMessagesReceived < 255 ? ++m_cMessagesReceived : 255; }
	uint8			GetMessagesSent() const							{ return m_cMessagesSent; }
	void			SetMessagesSent(uint8 nCount)					{ m_cMessagesSent = nCount; }
	void			IncMessagesSent()								{ m_cMessagesSent < 255 ? ++m_cMessagesSent : 255; }
	bool			IsSpammer() const								{ return m_fIsSpammer; }
	void			SetSpammer(bool bVal);
	bool			GetMessageFiltered() const						{ return m_fMessageFiltered; }
	void			SetMessageFiltered(bool bVal);


	//KadIPCheck
	EKadState		GetKadState() const								{ return (EKadState)m_nKadState; }
	void			SetKadState(EKadState nNewS)					{ m_nKadState = (_EKadState)nNewS; }

	//File Comment
	bool			HasFileComment() const							{ return !m_strFileComment.IsEmpty(); }
    const CString&	GetFileComment() const							{ return m_strFileComment; } 
    void			SetFileComment(LPCTSTR pszComment)				{ m_strFileComment = pszComment; }

	bool			HasFileRating() const							{ return m_uFileRating > 0; }
    uint8			GetFileRating() const							{ return m_uFileRating; }
    void			SetFileRating(uint8 uRating)					{ m_uFileRating = uRating; }

	// Barry - Process zip file as it arrives, don't need to wait until end of block
	int				unzip(Pending_Block_Struct *block, const BYTE *zipped, UINT lenZipped, BYTE **unzipped, UINT *lenUnzipped, int iRecursion = 0);
	void			UpdateDisplayedInfo(bool force = false);
	int             GetFileListRequested() const					{ return m_iFileListRequested; }
    void            SetFileListRequested(int iFileListRequested)	{ m_iFileListRequested = iFileListRequested; }

	virtual void	SetRequestFile(CPartFile* pReqFile);
	CPartFile*		GetRequestFile() const							{ return reqfile; }

	// AICH Stuff
	void			SetReqFileAICHHash(CAICHHash* val);
	CAICHHash*		GetReqFileAICHHash() const						{ return m_pReqFileAICHHash; }
	bool			IsSupportingAICH() const						{ return m_fSupportsAICH & 0x01; }
	void			SendAICHRequest(CPartFile* pForFile, uint16 nPart);
	bool			IsAICHReqPending() const						{ return m_fAICHRequested; }
	void			ProcessAICHAnswer(const uchar* packet, UINT size);
	void			ProcessAICHRequest(const uchar* packet, UINT size);
	void			ProcessAICHFileHash(CSafeMemFile* data, CPartFile* file, const CAICHHash* pAICHHash);

	EUtf8Str		GetUnicodeSupport() const;

	CString			GetDownloadStateDisplayString() const;
	CString			GetUploadStateDisplayString() const;

	LPCTSTR			DbgGetDownloadState() const;
	LPCTSTR			DbgGetUploadState() const;
	LPCTSTR			DbgGetKadState() const;
	CString			DbgGetClientInfo(bool bFormatIP = false) const;
	CString			DbgGetFullClientSoftVer() const;
	const CString&	DbgGetHelloInfo() const							{ return m_strHelloInfo; }
	const CString&	DbgGetMuleInfo() const							{ return m_strMuleInfo; }

// ZZ:DownloadManager -->
	const bool      IsInNoNeededList(const CPartFile* fileToCheck) const;
	//Xman
	/*
    const bool      SwapToRightFile(CPartFile* SwapTo, CPartFile* cur_file, bool ignoreSuspensions, bool SwapToIsNNPFile, bool isNNPFile, bool& wasSkippedDueToSourceExchange, bool doAgressiveSwapping = false, bool debug = false);
    const DWORD     getLastTriedToConnectTime() { return m_dwLastTriedToConnect; }
	*/
	//Xman end
// <-- ZZ:DownloadManager

#ifdef _DEBUG
	// Diagnostic Support
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

	CClientReqSocket* socket;
	CClientCredits*	credits;
	CFriend*		m_Friend;
	uint8*			m_abyUpPartStatus;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherRequests_list;
	CTypedPtrList<CPtrList, CPartFile*> m_OtherNoNeeded_list;
	uint16			m_lastPartAsked;
	bool			m_bAddNextConnect;

	//Xman not used
	/*
    void			SetSlotNumber(UINT newValue)					{ m_slotNumber = newValue; }
    UINT			GetSlotNumber() const							{ return m_slotNumber; }
	*/
	//Xman end
	//Xman Xtreme Upload: Peercache-part
	/*
    CEMSocket*		GetFileUploadSocket(bool log = false);
	*/
    CClientReqSocket*		GetFileUploadSocket(bool log = false) const ;
	//Xman end

	///////////////////////////////////////////////////////////////////////////
	// PeerCache client
	//
	bool IsDownloadingFromPeerCache() const;
	bool IsUploadingToPeerCache() const;
	//Xman Xtreme Upload: Peercache-part
	bool HasPeerCacheState() {return m_ePeerCacheUpState!=PCUS_NONE; /* m_ePeerCacheUpState == PCUS_WAIT_CACHE_REPLY || m_ePeerCacheUpState == PCUS_UPLOADING;*/}
	//Xman end
	void SetPeerCacheDownState(EPeerCacheDownState eState);
	void SetPeerCacheUpState(EPeerCacheUpState eState);

	int  GetHttpSendState() const									{ return m_iHttpSendState; }
	void SetHttpSendState(int iState)								{ m_iHttpSendState = iState; }

	bool SendPeerCacheFileRequest();
	bool ProcessPeerCacheQuery(const uchar* packet, UINT size);
	bool ProcessPeerCacheAnswer(const uchar* packet, UINT size);
	bool ProcessPeerCacheAcknowledge(const uchar* packet, UINT size);
	void OnPeerCacheDownSocketClosed(int nErrorCode);
	bool OnPeerCacheDownSocketTimeout();
	
	bool ProcessPeerCacheDownHttpResponse(const CStringAArray& astrHeaders);
	bool ProcessPeerCacheDownHttpResponseBody(const BYTE* pucData, UINT uSize);
	void ProcessPeerCacheUpHttpResponse(const CStringAArray& astrHeaders);
	UINT ProcessPeerCacheUpHttpRequest(const CStringAArray& astrHeaders);

	virtual bool ProcessHttpDownResponse(const CStringAArray& astrHeaders);
	virtual bool ProcessHttpDownResponseBody(const BYTE* pucData, UINT uSize);

	CPeerCacheDownSocket* m_pPCDownSocket;
	CPeerCacheUpSocket* m_pPCUpSocket;

//-----------------------------------------------------------------
//Xman:
public:
	
	//Xman Full Chunk
	bool upendsoon;
	// ==> Mephisto Upload - Mephisto
	/*
	bool 			IsDifferentPartBlock();
	*/
	bool 			IsDifferentPartBlock(bool bCheckForMulti = false);
	// <== Mephisto Upload - Mephisto
	//Xman end
	
	//Xman Dynamic block request (netfinity/Xman)
	uint16			GetRemainingBlocksToDownload() const { return (uint16)m_PendingBlocks_list.GetCount() /*+ m_DownloadBlocks_list.GetCount()*/;}

	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
    //UINT			GetUploadDatarate(UINT samples) const; //unused
    UINT			GetUploadDatarate() const {return m_nUpDatarate;}
	UINT			GetDownloadDatarate() const {return m_nDownDatarate;}
	UINT			GetDownloadDatarate10() const {return m_nDownDatarate10;}

	void			AddUploadRate(const UINT size);

	void			AddDownloadRate(const UINT size) { m_nDownDatarateMeasure += size; } 

	void			CompUploadRate(); // Actualize datarate

	void			CompDownloadRate(); // Actualize datarate
	
	// Maella end

	const CString			GetUserIPString() const { return m_szFullUserIP; }

	//Xman filter clients with failed downloads
	uint16 m_faileddownloads;

	//Xman askfordownload priority
	sint8 m_downloadpriority;

	// ==> Mephisto Upload - Mephisto
	/*
	//Xman Xtreme Upload
	//CompUploadRate and looks if a datarate is too high
	//this forces the uploadbandwidththrottler to set a new Full Slot
	bool			CheckDatarate(uint8 dataratestocheck);
	*/
	// <== Mephisto Upload - Mephisto

	//Xman uploading problem client
	bool isupprob;

	CString			GetFullIP()					{return m_szFullUserIP;}

	//Xman see OnUploadqueue
	void			SetOldUploadFileID();
	uchar*			GetOldUploadFileID()		{return oldfileid;} 
	//Xman fix for startupload
	void			SetLastAction(UINT action)	{lastaction=action;}
	UINT			GetLastAction()				{return lastaction;}
	//Xman end
	
	//Xman fix for startupload (downloading side)
	bool protocolstepflag1;

	//Xman Xtreme Downloadmanager
	void			TrigNextSafeAskForDownload(CPartFile* pFile); // Maella -Unnecessary Protocol Overload-
	bool			SwapToAnotherFile(bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile = NULL, bool lookatmintime=false, bool allow_go_over_hardlimit=false);
	void			DontSwapTo(CPartFile* file);
	bool			IsSwapSuspended(CPartFile* file);
	bool			DoSwap(CPartFile* SwapTo, bool bRemoveCompletely=false);
	void			SetRemoteQueueRank(UINT nr, bool updatedisplay=true);	//remark: updatedisplay is used on different way than official

	//Xman Anti-Leecher
	void			TestLeecher();
	void			BanLeecher(LPCTSTR pszReason , uint8 leechercategory); 
	uint8			IsLeecher()	const		{return m_bLeecher;}
	bool			ProcessUnknownHelloTag(CTag *tag, CString &pszReason);
	void			ProcessUnknownInfoTag(CTag *tag, CString &pszReason);
	void			ProcessBanMessage();
	CString			GetBanMessageString() const {return strBanReason_permament;}


	//>>> Anti-XS-Exploit (Xman)
	void IncXSAnswer()  {m_uiXSAnswer++;}
	void IncXSReqs()  {m_uiXSReqs++;}
	UINT GetXSReqs() const { return m_uiXSReqs;}
	UINT GetXSAnswers() const {return m_uiXSAnswer;}
	bool IsXSExploiter() const { return m_uiXSReqs>2 && ((float)(m_uiXSAnswer+1))/m_uiXSReqs<0.5f;}
	//<<< Anti-XS-Exploit

	//Xman end

	void			ShowRequestedFiles(); // - show requested files (sivka)

	// Maella -Unnecessary Protocol Overload-
	uint32 GetLastAskedTime() const {return m_dwLastAskedTime;}
	uint32 GetNextTCPAskedTime() const {return m_dwNextTCPAskedTime;}
	void   SetNextTCPAskedTime(uint32 time) {m_dwNextTCPAskedTime = time;}
	uint32 GetLastFileAskedTime(CPartFile* pFile) {return m_partStatusMap[pFile].dwStartUploadReqTime;}
	uint32 GetJitteredFileReaskTime() const {return m_jitteredFileReaskTime;} // range 25.5..29.5 min 
	void   CalculateJitteredFileReaskTime(bool longer); //Xman 5.1 
	//Xman own method
	bool   HasTooManyFailedUDP() const {return m_nTotalUDPPackets > 3 && ((float)(m_nFailedUDPPackets/m_nTotalUDPPackets) > .3);} 
	bool	GetLowIDReaskPening() const {return m_bReaskPending;}
	//Maella end

	//Xman Xtreme Downloadmanager
	uint32 droptime;
	uint32 enterqueuetime;	//indicates, when a source was first asked
	sint32 GetDiffQR() const {return oldQR==0 ? 0 : m_nRemoteQueueRank - oldQR;} //Xman diffQR

	
	//check completed sources which want to download their "complete" file
	bool HasFileComplete() const {return m_bCompleteSource;}

	// Maella -Extended clean-up II-
	void CleanUp(CPartFile* pDeletedFile);
	DWORD m_lastCleanUpCheck;
	// Maella end

	//Xman Funny-Nick (Stulle/Morph)
	void	UpdateFunnyNick();
	//Xman end

	//Xman client percentage
	sint8	hiscompletedparts_percent_up;
	sint8	hiscompletedparts_percent_down;
	sint8	GetHisCompletedPartsPercent_UP() const		{return hiscompletedparts_percent_up;}
	sint8	GetHisCompletedPartsPercent_Down() const	{return m_abyPartStatus==NULL ? -1 : hiscompletedparts_percent_down;}
	//Xman end

#ifdef PRINT_STATISTIC
	uint32 GetPartStatusMapCount()	{return m_partStatusMap.size();}
	uint32 GetupHistoryCount()		{return m_upHistory_list.GetSize();}
	uint32 GetdownHistoryCount()	{return m_downHistory_list.GetSize();}
	uint32 GetDontSwapListCount()	{return m_DontSwap_list.GetSize();}
	uint32 GetBlockRequestedCount() {return m_BlockRequests_queue.GetSize();}
	uint32 GetDoneBlocksCount()		{return m_DoneBlocks_list.GetSize();}
	uint32 GetRequestedFilesCount() {return m_RequestedFiles_list.GetSize();}
	uint32 GetPendingBlockCount()	{return m_PendingBlocks_list.GetSize();}
	uint32 GetDownloadBlockCount()	{return m_DownloadBlocks_list.GetSize();}
	uint32 GetNoNeededListCount()	{return m_OtherNoNeeded_list.GetSize();}
	uint32 GetOtherRequestListCount() {return m_OtherRequests_list.GetSize();}
#endif

	//zz_fly :: Drop stalled downloads :: netfinity :: start
	uint32	GetLastBlockReceived()	const	{return m_dwLastBlockReceived;} 
	bool	IsPendingBlocksListEmpty() const	{return m_PendingBlocks_list.IsEmpty() ? true : false;} //avoid a warning
	uint64	GetBytesRemaining() const;
	//zz_fly :: Drop stalled downloads :: netfinity :: end

	// BEGIN SiRoB: ReadBlockFromFileThread
	void	SetReadBlockFromFileBuffer(byte* pdata) {m_abyfiledata = pdata;};
	// END SiRoB: ReadBlockFromFileThread

// Maella -Unnecessary Protocol Overload-
protected:
	struct PartStatus{
		PartStatus() : dwStartUploadReqTime(0) {}
		uint32 dwStartUploadReqTime; // Used to avoid Ban()
	};
	typedef std::map<CPartFile*, PartStatus> PartStatusMap;
	PartStatusMap m_partStatusMap;



private:
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint8	m_displayUpDatarateCounter; // refresh display timer
	uint8	m_displayDownDatarateCounter; // refresh display timer

	UINT	m_nUpDatarate; // current datarate (updated every seconds)
	UINT	m_nUpDatarate10; //current datarate over 10 seconds
	UINT	m_nUpDatarateMeasure; // transfered byte since the last measure
	CList<TransferredData> m_upHistory_list;

	UINT	m_nDownDatarate; // current datarate (updated every seconds)
	UINT	m_nDownDatarate10; //Xman 0.46b: hold the avg over 10 secondes, needed for the faster endgame
	UINT	m_nDownDatarateMeasure; // transfered byte since the last measure
	//uint32	m_sumDownHistory; // max 4GB
	CList<TransferredData> m_downHistory_list;
	// Maella end

	CString m_szFullUserIP; //Xman hold the IP-String

	// Maella -Unnecessary Protocol Overload-
	uint32 m_dwLastAskedTime;     // Last attempt to refresh the download session with TCP or UDP
	uint32 m_dwLastUDPReaskTime;  // Last attempt to refresh the download session with UDP
	uint32 m_dwNextTCPAskedTime;  // Time of the next refresh for the download session with TCP
	uint32 m_jitteredFileReaskTime;

	//Xman Xtreme Downloadmanager
	bool isduringswap;	//indicates, that we are during a swap operation
	UINT oldQR; //Xman diffQR

	// BEGIN SiRoB: ReadBlockFromFileThread
	byte* m_abyfiledata;
	uint32 m_utogo;
	CReadBlockFromFileThread* m_readblockthread;
	// END SiRoB: ReadBlockFromFileThread


	// Maella -Upload Stop Reason-
public:
	static void   AddUpStopCount(bool failed, UpStopReason reason) {++m_upStopReason[failed?0:1][reason];}
	static uint32 GetUpStopCount(bool failed, UpStopReason reason) {return m_upStopReason[failed?0:1][reason];}
	//Xman uploading problem client
	static void		SubUploadSocketStopCount(){--m_upStopReason[0][USR_SOCKET];}

	uint8	m_cFailed; //Xman Downloadmanager / Xtreme Mod // holds the failed connection attempts

	// Maella -Download Stop Reason-

	static uint32 GetDownStopCount(bool failed, DownStopReason reason) {return m_downStopReason[failed?0:1][reason];}


private:
	static void   AddDownStopCount(bool failed, DownStopReason reason) {++m_downStopReason[failed?0:1][reason];}

	static uint32 m_upStopReason[2][USR_EXCEPTION+1];
	static uint32 m_downStopReason[2][DSR_EXCEPTION+1];
	// Maella end
	
	//Xman Anti-Leecher
	uint8	m_bLeecher; 
	CString	old_m_strClientSoftwareFULL;
	CString	old_m_pszUsername;
	CString m_strBanMessage; //hold the message temporary
	CString strBanReason_permament; //keeps the message in short version
	uint8 uhashsize;
	uint8 m_uNickchanges; //Xman Anti-Nick-Changer
	uint32 m_ulastNickChage; //Xman Anti-Nick-Changer

	//>>> Anti-XS-Exploit (Xman)
	uint32 m_uiXSAnswer;
	uint32 m_uiXSReqs;
	//<<< Anti-XS-Exploit

	//Xman end

	//Xman Anti-Leecher: simple Anti-Thief
	float	GetXtremeVersion(CString modversion) const;
	static const CString str_ANTAddOn;
	static const CString GetANTAddOn()
	{
		CString nick=_T("[");
		srand( (unsigned)time( NULL ) );
		for(uint8 i = 0; i < 4; ++i)
		{
				nick.AppendFormat(_T("%c"), 32+rand()%95);
		}
		nick += _T(']');

		return nick;
	}
	//Xman end

	//Xman Funny-Nick (Stulle/Morph)
	TCHAR*	m_pszFunnyNick;
	//Xman end

//Xman end
//--------------------------------------------------------------------------------------

protected:
	int		m_iHttpSendState;
	uint32	m_uPeerCacheDownloadPushId;
	uint32	m_uPeerCacheUploadPushId;
	uint32	m_uPeerCacheRemoteIP;
	bool	m_bPeerCacheDownHit;
	bool	m_bPeerCacheUpHit;
	EPeerCacheDownState m_ePeerCacheDownState;
	EPeerCacheUpState m_ePeerCacheUpState;

protected:
	// base
	void	Init();
	//Xman Anti-Leecher: decide if Hello or Hello-Answer ->now independent from any other variables
	/*
	bool	ProcessHelloTypePacket(CSafeMemFile* data);
	*/
	bool	ProcessHelloTypePacket(CSafeMemFile* data, bool isHelloPacket);
	//Xman end
	void	SendHelloTypePacket(CSafeMemFile* data);
	void	CreateStandartPackets(byte* data, UINT togo, Requested_Block_Struct* currentblock, bool bFromPF = true);
	void	CreatePackedPackets(byte* data, UINT togo, Requested_Block_Struct* currentblock, bool bFromPF = true);
	void	SendFirewallCheckUDPRequest();
	void	SendHashSetRequest();

	uint32	m_nConnectIP;		// holds the supposed IP or (after we had a connection) the real IP
	uint32	m_dwUserIP;			// holds 0 (real IP not yet available) or the real IP (after we had a connection)
	uint32	m_dwServerIP;
	uint32	m_nUserIDHybrid;
	uint16	m_nUserPort;
	uint16	m_nServerPort;
	UINT	m_nClientVersion;
	//--group to aligned int32
	uint8	m_byEmuleVersion;
	uint8	m_byDataCompVer;
	bool	m_bEmuleProtocol;
	bool	m_bIsHybrid;
	//--group to aligned int32
	TCHAR*	m_pszUsername;
	uchar	m_achUserHash[16];
	uint16	m_nUDPPort;
	uint16	m_nKadPort;
	//--group to aligned int32
	uint8	m_byUDPVer;
	uint8	m_bySourceExchange1Ver;
	uint8	m_byAcceptCommentVer;
	uint8	m_byExtendedRequestsVer;
	//--group to aligned int32
	uint8	m_byCompatibleClient;
	bool	m_bFriendSlot;
	bool	m_bCommentDirty;
	bool	m_bIsML;
	//--group to aligned int32
	bool	m_bGPLEvildoer;
	bool m_bHelloAnswerPending;
	uint8	m_byInfopacketsReceived;	// have we received the edonkeyprot and emuleprot packet already (see InfoPacketsReceived() )
	uint8	m_bySupportSecIdent;
	//--group to aligned int32
	uint32	m_dwLastSignatureIP;
	CString m_strClientSoftware;
	CString m_strModVersion;
	uint32	m_dwLastSourceRequest;
	uint32	m_dwLastSourceAnswer;
	uint32	m_dwLastAskedForSources;
    int     m_iFileListRequested;
	CString	m_strFileComment;
	//--group to aligned int32
	uint8	m_uFileRating;
	uint8	m_cMessagesReceived;		// count of chatmessages he sent to me
	uint8	m_cMessagesSent;			// count of chatmessages I sent to him
	bool	m_bMultiPacket;
	//--group to aligned int32
	bool	m_bUnicodeSupport;
	bool	m_bBuddyIDValid;
	uint16	m_nBuddyPort;
	//--group to aligned int32
	uint8	m_byKadVersion;
	uint8	m_cCaptchasSent;

	uint32	m_nBuddyIP;
	uint32	m_dwLastBuddyPingPongTime;
	uchar	m_achBuddyID[16];
	CString m_strHelloInfo;
	CString m_strMuleInfo;
	CString m_strCaptchaChallenge;
	CString m_strCaptchaPendingMsg;
	

	// States
	_EClientSoftware	m_clientSoft;
	_EChatState			m_nChatstate;
	_EKadState			m_nKadState;
	_ESecureIdentState	m_SecureIdentState;
	_EUploadState		m_nUploadState;
	_EDownloadState		m_nDownloadState;
	_ESourceFrom		m_nSourceFrom;
	_EChatCaptchaState	m_nChatCaptchaState;
	_EConnectingState	m_nConnectingState;
	_EModClient         m_uModClient; // Mod Icons - Stulle

	CTypedPtrList<CPtrList, Packet*> m_WaitingPackets_list;
	CList<PartFileStamp> m_DontSwap_list;

	////////////////////////////////////////////////////////////////////////
	// Upload
	//
	int GetFilePrioAsNumber() const;

	UINT		m_nTransferredUp;
	uint32		m_dwUploadTime;
	UINT		m_cAsked;
	uint32		m_dwLastUpRequest;
	UINT		m_nCurSessionUp;
	UINT		m_nCurSessionDown;
    UINT	    m_nCurQueueSessionPayloadUp;
    UINT	    m_addedPayloadQueueSession;
	uint16		m_nUpPartCount;
	uint16		m_nUpCompleteSourcesCount;
	uchar		requpfileid[16];
	//Xman not used
	/*
    UINT		m_slotNumber;
	*/
	//Xman end
	bool		m_bCollectionUploadSlot;
	uchar		oldfileid[16]; //Xman see OnUploadqueue
	UINT		lastaction; //Xman fix for startupload

	//Xman not used
	/*
	typedef struct TransferredData {
		UINT	datalen;
		DWORD	timestamp;
	};
	*/
	//Xman end
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_BlockRequests_queue;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DoneBlocks_list;
	CTypedPtrList<CPtrList, Requested_File_Struct*>	 m_RequestedFiles_list;

	//////////////////////////////////////////////////////////
	// Download
	//
public: // File Settings [sivka/Stulle] - Stulle
	CPartFile*	reqfile;
protected: // File Settings [sivka/Stulle] - Stulle
	CAICHHash*  m_pReqFileAICHHash; 
	UINT		m_cDownAsked;
	uint8*		m_abyPartStatus;
	CString		m_strClientFilename;
	UINT		m_nTransferredDown;
	UINT        m_nCurSessionPayloadDown;
	uint32		m_dwDownStartTime;
	uint64      m_nLastBlockOffset;
	uint32		m_dwLastBlockReceived;
	UINT		m_nTotalUDPPackets;
	UINT		m_nFailedUDPPackets;
	UINT		m_nRemoteQueueRank;
	//--group to aligned int32
	bool		m_bRemoteQueueFull;
	bool		m_bCompleteSource;
	uint16		m_nPartCount;
	//--group to aligned int32
	//Xman not used
	/*
	uint16		m_cShowDR;
	*/
	//Xman end
	bool		m_bReaskPending;
	bool		m_bUDPPending;
	bool		m_bTransferredDownMini;
	bool		m_bHasMatchingAICHHash;

	// Download from URL
	CStringA	m_strUrlPath;
	uint64		m_uReqStart;
	uint64		m_uReqEnd;
	uint64		m_nUrlStartPos;

	//Xman not used
	/*
	//////////////////////////////////////////////////////////
	// Upload data rate computation
	//
	UINT		m_nUpDatarate;
	UINT		m_nSumForAvgUpDataRate;
	CList<TransferredData> m_AvarageUDR_list;

	//////////////////////////////////////////////////////////
	// Download data rate computation
	//
	UINT		m_nDownDatarate;
	UINT		m_nDownDataRateMS;
	UINT		m_nSumForAvgDownDataRate;
	CList<TransferredData> m_AvarageDDR_list;
	*/
	//Xman end

	//////////////////////////////////////////////////////////
	// GUI helpers
	//
	//Xman not used
	/*
	static CBarShader s_StatusBar;
	static CBarShader s_UpStatusBar;
	*/
	//Xman end
	DWORD		m_lastRefreshedDLDisplay;
    DWORD		m_lastRefreshedULDisplay;
    uint32      m_random_update_wait;

	// using bitfield for less important flags, to save some bytes
	UINT m_fHashsetRequestingMD4 : 1, // we have sent a hashset request to this client in the current connection
		 m_fSharedDirectories : 1, // client supports OP_ASKSHAREDIRS opcodes
		 m_fSentCancelTransfer: 1, // we have sent an OP_CANCELTRANSFER in the current connection
		 m_fNoViewSharedFiles : 1, // client has disabled the 'View Shared Files' feature, if this flag is not set, we just know that we don't know for sure if it is enabled
		 m_fSupportsPreview   : 1,
		 m_fPreviewReqPending : 1,
		 m_fPreviewAnsPending : 1,
		 m_fIsSpammer		  : 1,
		 m_fMessageFiltered   : 1,
		 m_fPeerCache		  : 1,
		 m_fQueueRankPending  : 1,
		 m_fUnaskQueueRankRecv: 2,
		 m_fFailedFileIdReqs  : 4, // nr. of failed file-id related requests per connection
		 m_fNeedOurPublicIP	  : 1, // we requested our IP from this client
		 m_fSupportsAICH	  : 3,
		 m_fAICHRequested     : 1,
		 m_fSentOutOfPartReqs : 1,
		 m_fSupportsLargeFiles: 1,
		 m_fExtMultiPacket	  : 1,
		 m_fRequestsCryptLayer: 1,
	     m_fSupportsCryptLayer: 1,
		 m_fRequiresCryptLayer: 1,
		 m_fSupportsSourceEx2 : 1,
		 m_fSupportsCaptcha	  : 1,
		 m_fDirectUDPCallback : 1,	
		 m_fSupportsFileIdent : 1; // 0 bits left
	UINT m_fHashsetRequestingAICH : 1; // 31 bits left
	CTypedPtrList<CPtrList, Pending_Block_Struct*>	 m_PendingBlocks_list;
	CTypedPtrList<CPtrList, Requested_Block_Struct*> m_DownloadBlocks_list;

	//Xman
	/*
    bool    m_bSourceExchangeSwapped; // ZZ:DownloadManager
    DWORD   lastSwapForSourceExchangeTick; // ZZ:DownloadManaager
    bool    DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason); // ZZ:DownloadManager
    CMap<CPartFile*, CPartFile*, DWORD, DWORD> m_fileReaskTimes; // ZZ:DownloadManager (one resk timestamp for each file)
    DWORD   m_dwLastTriedToConnect; // ZZ:DownloadManager (one resk timestamp for each file)
    bool    RecentlySwappedForSourceExchange() { return ::GetTickCount()-lastSwapForSourceExchangeTick < 30*1000; } // ZZ:DownloadManager
    void    SetSwapForSourceExchangeTick() { lastSwapForSourceExchangeTick = ::GetTickCount(); } // ZZ:DownloadManager
	*/
	//Xman end

//EastShare Start - added by AndCycle, IP to Country
public:
	CString			GetCountryName(bool longName = true) const; //Xman changed 
	int				GetCountryFlagIndex() const;
	void			ResetIP2Country(uint32 dwIP = 0);

//private:
	/*struct*/	Country_Struct* m_structUserCountry; //EastShare - added by AndCycle, IP to Country
//EastShare End - added by AndCycle, IP to Country

public:
	bool	GetSmallFilePush() const; // push small files [sivka] - Stulle

	float	GetRareFilePushRatio() const ; // push rare file - Stulle

	// ==> Mod Icons - Stulle
	EModClient	GetModClient() const	{ return (EModClient)m_uModClient; }
	// <== Mod Icons - Stulle

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	uint32			m_nSelectedChunk;
	void	SetPartCount(uint16 parts) { m_nUpPartCount = parts;}
	void GetUploadingAndUploadedPart(uint8* abyUpPartUploadingAndUploaded, uint32 partcount) const;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	bool IsSuperiorClient() const; // Superior Client Handling [Stulle] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	bool			GetPowerShared() const;
	bool			GetPowerShared(const CKnownFile* file) const;
	// <== PowerShare [ZZ/MorphXT] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	int		GetClientStyle(bool bDl, bool bUl, bool bShare, bool bOwnCredits) const;
	bool	GetPowerReleased() const;
	// <== Design Settings [eWombat/Stulle] - Stulle

	CString GetZeroScoreString() const; // Display reason for zero score [Stulle] - Stulle

	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	bool	IsPBFClient() const;
	bool	IsSecure() const;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	void	SetGiveWaittimeBack(bool in)		{m_bGiveWaittimeBack = in;}
	bool	GetGiveWaittimeBack() const			{return m_bGiveWaittimeBack;}
protected:
	bool	m_bGiveWaittimeBack;
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle

     // ==> Anti Uploader Ban [Stulle] - Stulle
private:
	bool	m_bAntiUploaderCaseThree;
public:
	bool	AntiUploaderBanActive();
	bool	GetAntiUploaderCaseThree()	{return m_bAntiUploaderCaseThree;}
     // <== Anti Uploader Ban [Stulle] - Stulle

	// ==> Spread Credits Slot [Stulle] - Stulle
	uint8 m_uSpreadClient;
public:
	uint8 GetSpreadClient() const	{return m_uSpreadClient;}
	void SetSpreadClient(uint8 in){m_uSpreadClient = in;};
	// <== Spread Credits Slot [Stulle] - Stulle

	// ==> Mephisto Upload - Mephisto
	void SetSlotNumber(UINT newValue) { m_slotNumber = newValue; }
	UINT GetSlotNumber() const { return m_slotNumber; }
	void SetStartProblem(uint8 newValue) { m_uStartProblem = newValue; }
	uint8 GetStartProblem() const { return m_uStartProblem; }
	void SetEndMovedDown(bool in) { m_bEndMovedDown = in; }
	bool GetEndMovedDown() const { return m_bEndMovedDown; }
private:
	UINT m_slotNumber;
	uint8 m_uStartProblem;
	bool m_bEndMovedDown;
	// <== Mephisto Upload - Mephisto

	// ==> Multiple Part Transfer [Stulle] - Mephisto
	UINT m_uFinishedChunks;
public:
	void ResetFinishedChunks()	{m_uFinishedChunks = 0;}
	void IncrFinishedChunks()	{m_uFinishedChunks++;}
	UINT GetFinishedChunks() const {return m_uFinishedChunks;}
	// <== Multiple Part Transfer [Stulle] - Mephisto

	// ==> Display remaining upload time [Stulle] - Mephisto
	CString GetRemainingUploadTime() const;
	// <== Display remaining upload time [Stulle] - Mephisto
};
//#pragma pack()
