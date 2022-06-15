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
#include "stdafx.h"
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include "emule.h"
#include "UpDownClient.h"
#include "FriendList.h"
#include "Clientlist.h"
#include "OtherFunctions.h"
#include "PartFile.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "Friend.h"
#include <zlib/zlib.h>
#include "Packets.h"
#include "Opcodes.h"
#include "SafeFile.h"
#include "Preferences.h"
#include "Server.h"
#include "ClientCredits.h"
#include "IPFilter.h"
#include "Statistics.h"
#include "Sockets.h"
#include "DownloadQueue.h"
#include "UploadQueue.h"
#include "SearchFile.h"
#include "SearchList.h"
#include "SharedFileList.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Search.h"
#include "Kademlia/Kademlia/SearchManager.h"
#include "Kademlia/Kademlia/UDPFirewallTester.h"
#include "Kademlia/routing/RoutingZone.h"
#include "Kademlia/Utils/UInt128.h"
#include "Kademlia/Net/KademliaUDPListener.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "emuledlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "CxImage/xImage.h"
#include "PreviewDlg.h"
#include "Exceptions.h"
#include "Peercachefinder.h"
#include "ClientUDPSocket.h"
#include "shahashset.h"
#include "Log.h"
#include "CaptchaGenerator.h"
#include "Neo/NeoVersion.h" // NEO: NV - [NeoVersion] <-- Xanatos --
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "Neo/NeoOpcodes.h" // NEO: NMP - [NeoModProt] <-- Xanatos --
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
#include "Neo/Sources/SourceList.h"
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
#include "ClientDetailDialog.h" // NEO: VSF - [VirtualSharedFiles] <-- Xanatos --
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "Neo/NatT/NatManager.h"
#include "Neo/NatT/NatSocket.h"
#include "Neo/NatT/NatTunnel.h" // NEO: NATT - [NatTraversal]
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
#include "Neo/LanCast/Lancast.h"
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#include "KnownFileList.h" // NEO: SCFS - [SmartClientFileStatus] // NEO: NLC END <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
#include "Neo/Argos.h"
#endif // ARGOS // NEO: NA END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define URLINDICATOR	_T("http:|www.|.de |.net |.com |.org |.to |.tk |.cc |.fr |ftp:|ed2k:|https:|ftp.|.info|.biz|.uk|.eu|.es|.tv|.cn|.tw|.ws|.nu|.jp")

IMPLEMENT_DYNAMIC(CClientException, CException)
IMPLEMENT_DYNAMIC(CUpDownClient, CObject)

CUpDownClient::CUpDownClient(CClientReqSocket* sender)
{
	socket = sender;
	reqfile = NULL;
	Init();
}

CUpDownClient::CUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid,uint32 in_serverip, uint16 in_serverport, bool ed2kID)
{
	//Converting to the HybridID system.. The ED2K system didn't take into account of IP address ending in 0..
	//All IP addresses ending in 0 were assumed to be a lowID because of the calculations.
	socket = NULL;
	reqfile = in_reqfile;
	// NEO: FIX - [SourceCount] -- Xanatos -->
	if(reqfile)
		reqfile->IncrSrcStatisticsValue(DS_NONE); 
	// NEO: FIX END <-- Xanatos --
	Init();
	m_nUserPort = in_port;
	//If this is a ED2K source, check if it's a lowID.. If not, convert it to a HyrbidID.
	//Else, it's already in hybrid form.
	if(ed2kID && !IsLowID(in_userid))
		m_nUserIDHybrid = ntohl(in_userid);
	else
		m_nUserIDHybrid = in_userid;

	//If highID and ED2K source, incoming ID and IP are equal..
	//If highID and Kad source, incoming IP needs ntohl for the IP
	if (!HasLowID() && ed2kID)
		m_nConnectIP = in_userid;
	else if(!HasLowID())
		m_nConnectIP = ntohl(in_userid);
	m_dwServerIP = in_serverip;
	m_nServerPort = in_serverport;

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	CheckOnLAN(m_nConnectIP);
#endif //LANCAST // NEO: NLC END <-- Xanatos --
}

void CUpDownClient::Init()
{
	m_nChatstate = MS_NONE;
	m_nKadState = KS_NONE;
	m_nChatCaptchaState = CA_NONE;
	m_nUploadState = US_NONE;
	m_nDownloadState = DS_NONE;
	m_SecureIdentState = IS_UNAVAILABLE;
	m_nConnectingState = CCS_NONE;
	m_ePeerCacheDownState = PCDS_NONE;
	m_ePeerCacheUpState = PCUS_NONE;

	credits = NULL;
	m_nSumForAvgUpDataRate = 0;
	//m_bAddNextConnect = false; // NEO: MOD - [NewUploadState] <-- Xanatos --
	//m_cShowDR = 0; // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	m_nUDPPort = 0;
	m_nKadPort = 0;
	m_nTransferredUp = 0;
	m_cAsked = 0;
	m_cDownAsked = 0;
	m_nUpDatarate = 0;
	m_pszUsername = 0;
	m_nUserIDHybrid = 0;
	m_dwServerIP = 0;
	m_nServerPort = 0;
    m_iFileListRequested = 0;
	// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
	m_bRequestingFileList = false;
	m_bFileListRequested = false;
	m_bDeniesShare = false;
	// NEO: XSF END <-- Xanatos --
	m_dwLastUpRequest = 0;
	m_bEmuleProtocol = false;
	//m_bCompleteSource = false; // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
	m_bFriendSlot = false;
	m_bCommentDirty = false;
	m_bReaskPending = false;
	m_bUDPPending = false;
	m_byEmuleVersion = 0;
	m_nUserPort = 0;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	//m_nPartCount = 0;
	//m_nUpPartCount = 0;
	//m_abyPartStatus = 0;
	//m_abyUpPartStatus = 0;
	m_dwUploadTime = 0;
	m_nTransferredDown = 0;
	m_nDownDatarate = 0;
	//m_nDownDataRateMS = 0; // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	m_dwLastBlockReceived = 0;
	m_byDataCompVer = 0;
	m_byUDPVer = 0;
	m_bySourceExchange1Ver = 0;
	m_byAcceptCommentVer = 0;
	m_byExtendedRequestsVer = 0;
	m_nRemoteQueueRank = 0;
	m_nRemoteQueueRankOld = 0; // NEO: CQR - [CollorQueueRank] <-- Xanatos --
	m_dwLastSourceRequest = 0;
	m_dwLastSourceAnswer = 0;
	m_dwLastAskedForSources = 0;
	m_byCompatibleClient = 0;
	m_nSourceFrom = SF_SERVER;
	m_bIsHybrid = false;
	m_bIsML=false;
	m_Friend = NULL;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	//m_uFileRating=0;
	//(void)m_strFileComment;
	m_fMessageFiltered = 0;
	m_fIsSpammer = 0;
	m_cMessagesReceived = 0;
	m_cMessagesSent = 0;
	m_nCurSessionUp = 0;
	m_nCurSessionDown = 0;
    m_nCurSessionPayloadDown = 0;
	m_nSumForAvgDownDataRate = 0;
	m_clientSoft=SO_UNKNOWN;
	//m_bRemoteQueueFull = false; // NEO: FIX - [SourceCount] <-- Xanatos --
	md4clr(m_achUserHash);
	SetBuddyID(NULL);
	m_nBuddyIP = 0;
	m_nBuddyPort = 0;
	if (socket){
		SOCKADDR_IN sockAddr = {0};
		int nSockAddrLen = sizeof(sockAddr);
		socket->GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
		SetIP(sockAddr.sin_addr.S_un.S_addr);
	}
	else{
		SetIP(0);
	}
	m_fHashsetRequesting = 0;
	m_fSharedDirectories = 0;
	m_fSentCancelTransfer = 0;
	m_nClientVersion = 0;
	m_lastRefreshedDLDisplay = 0;
	m_dwDownStartTime = 0;
	m_nLastBlockOffset = (uint64)-1;
	m_bUnicodeSupport = false;
	m_dwLastSignatureIP = 0;
	m_bySupportSecIdent = 0;
	m_byInfopacketsReceived = IP_NONE;
	m_lastPartAsked = (uint16)-1;
	//m_nUpCompleteSourcesCount= 0; // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
	m_fSupportsPreview = 0;
	m_fPreviewReqPending = 0;
	m_fPreviewAnsPending = 0;
	m_bTransferredDownMini = false;
    m_addedPayloadQueueSession = 0;
    m_nCurQueueSessionPayloadUp = 0; // PENDING: Is this necessary? ResetSessionUp()...
    m_lastRefreshedULDisplay = ::GetTickCount();
	m_bGPLEvildoer = false;
	//m_bHelloAnswerPending = false;
	m_byHelloPacketState = HP_NONE; // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
	m_fNoViewSharedFiles = 0;
	m_bMultiPacket = 0;
	md4clr(requpfileid);
	m_nTotalUDPPackets = 0;
	m_nFailedUDPPackets = 0;
	m_nUrlStartPos = (uint64)-1;
	m_iHttpSendState = 0;
	m_fPeerCache = 0;
	m_uPeerCacheDownloadPushId = 0;
	m_uPeerCacheUploadPushId = 0;
	m_pPCDownSocket = NULL;
	m_pPCUpSocket = NULL;
	m_uPeerCacheRemoteIP = 0;
	m_bPeerCacheDownHit = false;
	m_bPeerCacheUpHit = false;
	m_fNeedOurPublicIP = 0;
    m_random_update_wait = (uint32)(rand()/(RAND_MAX/1000));
    m_bSourceExchangeSwapped = false; // ZZ:DownloadManager
    //m_dwLastTriedToConnect = ::GetTickCount()-20*60*1000; // ZZ:DownloadManager
	m_dwLastTriedToConnect = 0; // NEO: DRT - [DownloadReaskTweaks] <-- Xanatos --
	m_fQueueRankPending = 0;
	m_fUnaskQueueRankRecv = 0;
	m_fFailedFileIdReqs = 0;
#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    m_slotNumber = 0;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	m_bUpEndSoon = false; // NEO: NUSM - [NeoUploadSlotManagement] <-- Xanatos --
    lastSwapForSourceExchangeTick = 0;
	m_pReqFileAICHHash = NULL;
	m_fSupportsAICH = 0;
	m_fAICHRequested = 0;
	m_byKadVersion = 0;
	SetLastBuddyPingPongTime();
	m_fSentOutOfPartReqs = 0;
	m_bCollectionUploadSlot = false;
	m_fSupportsLargeFiles = 0;
	m_fExtMultiPacket = 0;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
	m_fSupportsSourceEx2 = 0;
	m_fSupportsCaptcha = 0;
	m_fDirectUDPCallback = 0;
	m_cCaptchasSent = 0;

	m_dwLastUsableDownloadState = 0; // NEO: SDT - [SourcesDropTweaks] <-- Xanatos --
	// NEO: SR - [SpreadReask] -- Xanatos -->
	if(NeoPrefs.PartPrefs.UseSpreadReaskEnable())
		SetSpreadReaskModyfier();
	// NEO: SR END <-- Xanatos --
	m_uFaildCount = 0; // NEO: TCR - [TCPConnectionRetry] <-- Xanatos --
	m_uLastSeen = 0; // NEO: MOD - [LastSeen] <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	source = NULL;
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

	m_fileStatusMap.InitHashTable(29); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
	m_bDisableSwaping = false; // NEO: MCM - [ManualClientManagement] <-- Xanatos --

	// NEO: NMP - [NeoModProt] -- Xanatos -->
	m_fSupportsModProt = 0;
	m_fSupportNeoXS = 0;
	// NEO: L2HAC - [LowID2HighIDAutoCallback]
	m_fSupportL2HAC = 0;
	m_dwNextL2HACTime = ::GetTickCount();
	// NEO: L2HAC END
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_fSupportsNatTraversal = 0;
#endif //NATTUNNELING // NEO: NATT END
	m_fLowIDUDPPingSupport = 0;
	m_fUnsolicitedPartStatus = 0;
	m_fExtendedComments = 0; // NEO: XC - [ExtendedComments]
	// NEO: ICS - [InteligentChunkSelection]
	m_fIncompletePartVer1 = 0;
	m_fIncompletePartVer2 = 0;
	// NEO: ICS END
	m_fSubChunksSupport = 0; // NEO: SCT - [SubChunkTransfer]
	m_fExtendedUDPStatus = 0;

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_bNeoKadNatT = false;
	m_bSvrNatT = false;
	m_NeedOurPublicPort = 0; // NEO: RTP - [ReuseTCPPort]
	// NEO: XSB - [XSBuddy]
	SetXsBuddyStatus(XB_NONE);
	m_nXsBuddyIP = 0;
	m_nXsBuddyPort = 0;
	// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END
	// NEO: NMP END <-- Xanatos --

	m_byFileRequestState = FR_NONE; // NEO: USPS - [UnSolicitedPartStatus] <-- Xanatos --

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	m_bLanClient = false;
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
	m_bNotifyIdChage = false; 
	m_bWainingNotifyIdChage = false; 
	// NEO: RIC END <-- Xanatos --

	m_fUpIsProblematic = 0; // NEO: UPC - [UploadingProblemClient] <-- Xanatos --

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	m_faileddownloads = 0;

	m_uFastXSCounter = 0;
	m_uXSReqs = 0;
	m_uXSAnswer = 0;
#endif // ARGOS // NEO: NA END <-- Xanatos --

	m_nModClient = MOD_NONE; // NEO: MID - [ModID] <-- Xanatos -- 

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	m_structUserCountry = theApp.ip2country->GetCountryFromIP(m_dwUserIP);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
}

CUpDownClient::~CUpDownClient(){
	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHHashSet::ClientAICHRequestFailed(this);
	}

	if (GetFriend() != NULL)
	{
		if (GetFriend()->IsTryingToConnect())
			GetFriend()->UpdateFriendConnectionState(FCR_DELETED);
        m_Friend->SetLinkedClient(NULL);
	}
	ASSERT( m_nConnectingState == CCS_NONE || !theApp.emuledlg->IsRunning() );
	theApp.clientlist->RemoveClient(this, _T("Destructing client object"));

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	LinkSource(NULL); // detach source
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

	if (socket){
		socket->client = 0;
		socket->Safe_Delete();
	}
	if (m_pPCDownSocket){
		m_pPCDownSocket->client = NULL;
		m_pPCDownSocket->Safe_Delete();
	}
	if (m_pPCUpSocket){
		m_pPCUpSocket->client = NULL;
		m_pPCUpSocket->Safe_Delete();
	}
	
	free(m_pszUsername);
	
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* Status;
	CCKey key;
	POSITION pos = m_fileStatusMap.GetStartPosition();
	while (pos){
		m_fileStatusMap.GetNextAssoc(pos, key, Status);
#ifdef NEO_CD // NEO: SFL - [SourceFileList]
		Status->SetUsed(false);
		if(!Status->IsArcived())
#endif // NEO_CD // NEO: SFL END
			delete Status;
	}
	m_fileStatusMap.RemoveAll();
	// NEO: SCFS END <-- Xanatos --

	/*delete[] m_abyPartStatus;
	m_abyPartStatus = NULL;
	
	delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;*/
	
	ClearUploadBlockRequests();

	// NEO: DBR - [DynamicBlockRequest] -- Xanatos --
	//for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;)
	//	delete m_DownloadBlocks_list.GetNext(pos);
	
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition();pos != 0;)
		delete m_RequestedFiles_list.GetNext(pos);
	
	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition();pos != 0;){
		Pending_Block_Struct *pending = m_PendingBlocks_list.GetNext(pos);
		delete pending->block;
		// Not always allocated
		if (pending->zStream){
			inflateEnd(pending->zStream);
			delete pending->zStream;
		}
		delete pending;
	}

	for (POSITION pos = m_WaitingPackets_list.GetHeadPosition();pos != 0;)
		delete m_WaitingPackets_list.GetNext(pos);
	
	DEBUG_ONLY (theApp.listensocket->Debug_ClientDeleted(this));
	SetUploadFileID(NULL);

	SetRequestFile(NULL); // NEO: FIX - [SourceCount] <-- Xanatos --

    m_fileReaskTimes.RemoveAll(); // ZZ:DownloadManager (one resk timestamp for each file)

	delete m_pReqFileAICHHash;

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
	ClearTags();
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

	// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
	while (!m_listFiles.IsEmpty())
		delete m_listFiles.RemoveHead();
	// NEO: XSF END <-- Xanatos --

	// NEO: XCTA - [XmanExtenedCreditTableArragement] -- Xanatos -->
	if(Credits()){
		Credits()->SetLastSeen(); //ensure we keep the credits at least 6 hours in memory, without this line our LastSeen can be outdated if we did only UDP

		ASSERT(Credits()->IsUsed());
		Credits()->UnUse();
	}
	// NEO: XCTA END  <-- Xanatos --
}

void CUpDownClient::ClearHelloProperties()
{
	m_nUDPPort = 0;
	m_byUDPVer = 0;
	m_byDataCompVer = 0;
	m_byEmuleVersion = 0;
	m_bySourceExchange1Ver = 0;
	m_byAcceptCommentVer = 0;
	m_byExtendedRequestsVer = 0;
	m_byCompatibleClient = 0;
	m_nKadPort = 0;
	m_bySupportSecIdent = 0;
	m_fSupportsPreview = 0;
	m_nClientVersion = 0;
	m_fSharedDirectories = 0;
	m_bMultiPacket = 0;
	m_fPeerCache = 0;
	m_uPeerCacheDownloadPushId = 0;
	m_uPeerCacheUploadPushId = 0;
	m_byKadVersion = 0;
	m_fSupportsLargeFiles = 0;
	m_fExtMultiPacket = 0;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
	m_fSupportsSourceEx2 = 0;
	m_fSupportsCaptcha = 0;
	m_fDirectUDPCallback = 0;
}

bool CUpDownClient::ProcessHelloPacket(const uchar* pachPacket, uint32 nSize)
{
	CSafeMemFile data(pachPacket, nSize);
	data.ReadUInt8(); // read size of userhash
	// reset all client properties; a client may not send a particular emule tag any longer
	ClearHelloProperties();
	m_byHelloPacketState = HP_HELLO; // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
	return ProcessHelloTypePacket(&data);
}

bool CUpDownClient::ProcessHelloAnswer(const uchar* pachPacket, uint32 nSize)
{
	CSafeMemFile data(pachPacket, nSize);
	bool bIsMule = ProcessHelloTypePacket(&data);
	//m_bHelloAnswerPending = false;
	m_byHelloPacketState |= HP_HELLOANSWER; // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
	return bIsMule;
}

bool CUpDownClient::ProcessHelloTypePacket(CSafeMemFile* data)
{
	bool bDbgInfo = thePrefs.GetUseDebugDevice();
	m_strHelloInfo.Empty();
	// clear hello properties which can be changed _only_ on receiving OP_Hello/OP_HelloAnswer
	m_bIsHybrid = false;
	m_bIsML = false;
	m_fNoViewSharedFiles = 0;
	m_bUnicodeSupport = false;

	ClearModInfoProperties(); // NEO: NMP - [NeoModProt] <-- Xanatos --

	data->ReadHash16(m_achUserHash);
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("Hash=%s (%s)"), md4str(m_achUserHash), DbgGetHashTypeString(m_achUserHash));
	m_nUserIDHybrid = data->ReadUInt32();
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("  UserID=%u (%s)"), m_nUserIDHybrid, ipstr(m_nUserIDHybrid));
	uint16 nUserPort = data->ReadUInt16(); // hmm clientport is sent twice - why?
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("  Port=%u"), nUserPort);
	
	DWORD dwEmuleTags = 0;
	bool bPrTag = false;
	uint32 tagcount = data->ReadUInt32();
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("  Tags=%u"), tagcount);
	for (uint32 i = 0; i < tagcount; i++)
	{
		CTag temptag(data, true);
		switch (temptag.GetNameID())
		{
			case CT_NAME:
				if (temptag.IsStr()) {
					m_Dirs2Update.AddHead(_T("")); // NEO: XSF - [ExtendedSharedFiles] <-- Xanatos --
					free(m_pszUsername);
					m_pszUsername = _tcsdup(temptag.GetStr());
					if (bDbgInfo) {
						if (m_pszUsername) {//filter username for bad chars
							TCHAR* psz = m_pszUsername;
							while (*psz != _T('\0')) {
								if (*psz == _T('\n') || *psz == _T('\r'))
									*psz = _T(' ');
								psz++;
							}
						}
						m_strHelloInfo.AppendFormat(_T("\n  Name='%s'"), m_pszUsername);
					}
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_VERSION:
				if (temptag.IsInt()) {
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  Version=%u"), temptag.GetInt());
					m_nClientVersion = temptag.GetInt();
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_PORT:
				if (temptag.IsInt()) {
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  Port=%u"), temptag.GetInt());
					nUserPort = (uint16)temptag.GetInt();
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_MOD_VERSION:
				if (temptag.IsStr())
					m_strModVersion = temptag.GetStr();
				else if (temptag.IsInt())
					m_strModVersion.Format(_T("ModID=%u"), temptag.GetInt());
				else
					m_strModVersion = _T("ModID=<Unknown>");
				DetectMod(); // NEO: MID - [ModID] <-- Xanatos -- 
				if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
				CheckForGPLEvilDoer();
				break;

			case CT_EMULE_UDPPORTS:
				// 16 KAD Port
				// 16 UDP Port
				if (temptag.IsInt()) {
					m_nKadPort = (uint16)(temptag.GetInt() >> 16);
					m_nUDPPort = (uint16)temptag.GetInt();
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  KadPort=%u  UDPPort=%u"), m_nKadPort, m_nUDPPort);
					dwEmuleTags |= 1;
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_BUDDYUDP:
				// 16 --Reserved for future use--
				// 16 BUDDY Port
				if (temptag.IsInt()) {
					m_nBuddyPort = (uint16)temptag.GetInt();
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyPort=%u"), m_nBuddyPort);
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_BUDDYIP:
				// 32 BUDDY IP
				if (temptag.IsInt()) {
					m_nBuddyIP = temptag.GetInt();
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyIP=%s"), ipstr(m_nBuddyIP));
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_MISCOPTIONS1:
				//  3 AICH Version (0 = not supported)
				//  1 Unicode
				//  4 UDP version
				//  4 Data compression version
				//  4 Secure Ident
				//  4 Source Exchange - deprecated
				//  4 Ext. Requests
				//  4 Comments
				//	1 PeerChache supported
				//	1 No 'View Shared Files' supported
				//	1 MultiPacket
				//  1 Preview
				if (temptag.IsInt()) {
					m_fSupportsAICH			= (temptag.GetInt() >> 29) & 0x07;
					m_bUnicodeSupport		= (temptag.GetInt() >> 28) & 0x01;
					m_byUDPVer				= (uint8)((temptag.GetInt() >> 24) & 0x0f);
					m_byDataCompVer			= (uint8)((temptag.GetInt() >> 20) & 0x0f);
					m_bySupportSecIdent		= (uint8)((temptag.GetInt() >> 16) & 0x0f);
					m_bySourceExchange1Ver	= (uint8)((temptag.GetInt() >> 12) & 0x0f);
					m_byExtendedRequestsVer	= (uint8)((temptag.GetInt() >>  8) & 0x0f);
					m_byAcceptCommentVer	= (uint8)((temptag.GetInt() >>  4) & 0x0f);
					m_fPeerCache			= (temptag.GetInt() >>  3) & 0x01;
					m_fNoViewSharedFiles	= (temptag.GetInt() >>  2) & 0x01;
					m_bMultiPacket			= (temptag.GetInt() >>  1) & 0x01;
					m_fSupportsPreview		= (temptag.GetInt() >>  0) & 0x01;
					dwEmuleTags |= 2;
					if (bDbgInfo) {
						m_strHelloInfo.AppendFormat(_T("\n  PeerCache=%u  UDPVer=%u  DataComp=%u  SecIdent=%u  SrcExchg=%u")
													_T("  ExtReq=%u  Commnt=%u  Preview=%u  NoViewFiles=%u  Unicode=%u"), 
													m_fPeerCache, m_byUDPVer, m_byDataCompVer, m_bySupportSecIdent, m_bySourceExchange1Ver, 
													m_byExtendedRequestsVer, m_byAcceptCommentVer, m_fSupportsPreview, m_fNoViewSharedFiles, m_bUnicodeSupport);
					}
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_MISCOPTIONS2:
				//	19 Reserved
				//   1 Direct UDP Callback supported and available
				//	 1 Supports ChatCaptchas
				//	 1 Supports SourceExachnge2 Packets, ignores SX1 Packet Version
				//	 1 Requires CryptLayer
				//	 1 Requests CryptLayer
				//	 1 Supports CryptLayer
				//	 1 Reserved (ModBit)
				//   1 Ext Multipacket (Hash+Size instead of Hash)
				//   1 Large Files (includes support for 64bit tags)
				//   4 Kad Version - will go up to version 15 only (may need to add another field at some point in the future)
				if (temptag.IsInt()) {
					m_fDirectUDPCallback	= (temptag.GetInt() >>  12) & 0x01;
					m_fSupportsCaptcha	    = (temptag.GetInt() >>  11) & 0x01;
					m_fSupportsSourceEx2	= (temptag.GetInt() >>  10) & 0x01;
					m_fRequiresCryptLayer	= (temptag.GetInt() >>  9) & 0x01;
					m_fRequestsCryptLayer	= (temptag.GetInt() >>  8) & 0x01;
					m_fSupportsCryptLayer	= (temptag.GetInt() >>  7) & 0x01;
					m_fSupportsModProt		= (temptag.GetInt() >>  6) & 0x01; // NEO: NMP - [NeoModProt] <-- Xanatos --
					m_fExtMultiPacket		= (temptag.GetInt() >>  5) & 0x01;
					m_fSupportsLargeFiles   = (temptag.GetInt() >>  4) & 0x01;
					m_byKadVersion			= (uint8)((temptag.GetInt() >>  0) & 0x0f);
					dwEmuleTags |= 8;
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  KadVersion=%u, LargeFiles=%u ExtMultiPacket=%u CryptLayerSupport=%u CryptLayerRequest=%u CryptLayerRequires=%u SupportsSourceEx2=%u SupportsCaptcha=%u DirectUDPCallback=%u"), m_byKadVersion, m_fSupportsLargeFiles, m_fExtMultiPacket, m_fSupportsCryptLayer, m_fRequestsCryptLayer, m_fRequiresCryptLayer, m_fSupportsSourceEx2, m_fSupportsCaptcha, m_fDirectUDPCallback);
					m_fRequestsCryptLayer &= m_fSupportsCryptLayer;
					m_fRequiresCryptLayer &= m_fRequestsCryptLayer;

				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_VERSION:
				//  8 Compatible Client ID
				//  7 Mjr Version (Doesn't really matter..)
				//  7 Min Version (Only need 0-99)
				//  3 Upd Version (Only need 0-5)
				//  7 Bld Version (Only need 0-99) -- currently not used
				if (temptag.IsInt()) {
					m_byCompatibleClient = (uint8)((temptag.GetInt() >> 24));
					m_nClientVersion = temptag.GetInt() & 0x00ffffff;
					m_byEmuleVersion = 0x99;
					m_fSharedDirectories = 1;
					dwEmuleTags |= 4;
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  ClientVer=%u.%u.%u.%u  Comptbl=%u"), (m_nClientVersion >> 17) & 0x7f, (m_nClientVersion >> 10) & 0x7f, (m_nClientVersion >> 7) & 0x07, m_nClientVersion & 0x7f, m_byCompatibleClient);
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			// Note: This is for ICSv1 BAckwards compatybility
			// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
			case ET_INCOMPLETEPARTS:
				if (temptag.IsInt())
					m_fIncompletePartVer1 = temptag.GetInt() != 0; // we don't support v2 over the hello packet anymore
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			// NEO: ICS END <-- Xanatos --

			default:
				// Since eDonkeyHybrid 1.3 is no longer sending the additional Int32 at the end of the Hello packet,
				// we use the "pr=1" tag to determine them.
				if (temptag.GetName() && temptag.GetName()[0]=='p' && temptag.GetName()[1]=='r') {
					bPrTag = true;
				}
				if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkTag=%s"), temptag.GetFullInfo());
		}
	}
	m_nUserPort = nUserPort;
	m_dwServerIP = data->ReadUInt32();
	m_nServerPort = data->ReadUInt16();
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("\n  Server=%s:%u"), ipstr(m_dwServerIP), m_nServerPort);

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	if(m_pszUsername == NULL){
		SetUserName(_T("NULL"));
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s, Banreason: UserName is NULL !!!"), ipstr(GetConnectIP()));
		Ban(_T("UserName is NULL"));
	}
#endif // ARGOS // NEO: NA END <-- Xanatos --

	// Check for additional data in Hello packet to determine client's software version.
	//
	// *) eDonkeyHybrid 0.40 - 1.2 sends an additional Int32. (Since 1.3 they don't send it any longer.)
	// *) MLdonkey sends an additional Int32
	//
	if (data->GetLength() - data->GetPosition() == sizeof(uint32)){
		uint32 test = data->ReadUInt32();
		if (test == 'KDLM'){
			m_bIsML = true;
			if (bDbgInfo)
				m_strHelloInfo += _T("\n  ***AddData: \"MLDK\"");
		}
		else{
			m_bIsHybrid = true;
			if (bDbgInfo)
				m_strHelloInfo.AppendFormat(_T("\n  ***AddData: uint32=%u (0x%08x)"), test, test);
		}
	}
	else if (bDbgInfo && data->GetPosition() < data->GetLength()){
		UINT uAddHelloDataSize = (UINT)(data->GetLength() - data->GetPosition());
		if (uAddHelloDataSize == sizeof(uint32)){
			DWORD dwAddHelloInt32 = data->ReadUInt32();
			m_strHelloInfo.AppendFormat(_T("\n  ***AddData: uint32=%u (0x%08x)"), dwAddHelloInt32, dwAddHelloInt32);
		}
		else if (uAddHelloDataSize == sizeof(uint32)+sizeof(uint16)){
			DWORD dwAddHelloInt32 = data->ReadUInt32();
			WORD w = data->ReadUInt16();
			m_strHelloInfo.AppendFormat(_T("\n  ***AddData: uint32=%u (0x%08x),  uint16=%u (0x%04x)"), dwAddHelloInt32, dwAddHelloInt32, w, w);
		}
		else
			m_strHelloInfo.AppendFormat(_T("\n  ***AddData: %u bytes"), uAddHelloDataSize);
	}

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	uint32 oldIP = m_dwUserIP;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	SOCKADDR_IN sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	socket->GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	SetIP(sockAddr.sin_addr.S_un.S_addr);

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	if(oldIP != m_dwUserIP)
		m_structUserCountry = theApp.ip2country->GetCountryFromIP(m_dwUserIP);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	if (thePrefs.GetAddServersFromClients() && m_dwServerIP && m_nServerPort){
		CServer* addsrv = new CServer(m_nServerPort, ipstr(m_dwServerIP));
		addsrv->SetListName(addsrv->GetAddress());
		addsrv->SetPreference(SRV_PR_LOW);
		if (!theApp.emuledlg->serverwnd->serverlistctrl.AddServer(addsrv, true))
			delete addsrv;
	}

	//(a)If this is a highID user, store the ID in the Hybrid format.
	//(b)Some older clients will not send a ID, these client are HighID users that are not connected to a server.
	//(c)Kad users with a *.*.*.0 IPs will look like a lowID user they are actually a highID user.. They can be detected easily
	//because they will send a ID that is the same as their IP..
	if(!HasLowID() || m_nUserIDHybrid == 0 || m_nUserIDHybrid == m_dwUserIP ) 
		m_nUserIDHybrid = ntohl(m_dwUserIP);

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	if(socket->HaveNatLayer())
		m_nUDPPort = ntohs(sockAddr.sin_port);
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	if (NeoPrefs.IsHashThiefDetection()){
		if(md4cmp(m_achUserHash, thePrefs.GetUserHash())==0 && theApp.GetID()!= m_nUserIDHybrid){ //if client is using our Hash ban him!
			if (thePrefs.GetLogBannedClients())
				AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash match our own"), GetUserName(), ipstr(GetConnectIP()));
			Ban(_T("Credit hack detected"));
		}
	}

  if(NeoPrefs.IsHashChangeDetection() == 1){
#endif // ARGOS // NEO: NA END <-- Xanatos --

	CClientCredits* pFoundCredits = theApp.clientcredits->GetCredit(m_achUserHash);
	if (credits == NULL){
		credits = pFoundCredits;
		//if (!theApp.clientlist->ComparePriorUserhash(m_dwUserIP, m_nUserPort, pFoundCredits)){
		if (!theApp.clientlist->ComparePriorUserhash(m_dwUserIP, m_nUserPort, m_achUserHash)){ // NEO: XCTA - [XmanExtenedCreditTableArragement] <-- Xanatos --
			if (thePrefs.GetLogBannedClients())
				AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash changed (Found in TrackedClientsList)"), GetUserName(), ipstr(GetConnectIP()));
			Ban(_T("Userhash changed 2")); // NEO: NA - [NeoArgos] <-- Xanatos --
			//Ban();
		}	
	}
	else if (credits != pFoundCredits){
		// NEO: XCTA - [XmanExtenedCreditTableArragement] -- Xanatos -->
		credits->SetLastSeen(); //ensure to keep it at least 5 hours
		ASSERT(Credits()->IsUsed());
		Credits()->UnUse();
		// NEO: XCTA END  <-- Xanatos --
		// userhash change ok, however two hours "waittime" before it can be used
		credits = pFoundCredits;
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Userhash changed"), GetUserName(), ipstr(GetConnectIP()));
		Ban(_T("Userhash changed")); // NEO: NA - [NeoArgos] <-- Xanatos --
		//Ban();
	}

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
  }else
	   credits = theApp.clientcredits->GetCredit(m_achUserHash);
#endif // ARGOS // NEO: NA END <-- Xanatos --
	if (GetFriend() != NULL && GetFriend()->HasUserhash() && md4cmp(GetFriend()->m_abyUserhash, m_achUserHash) != 0)
	{
		// this isnt our friend anymore and it will be removed/replaced, tell our friendobject about it
		if (GetFriend()->IsTryingToConnect())
			GetFriend()->UpdateFriendConnectionState(FCR_USERHASHFAILED); // this will remove our linked friend
		else
			GetFriend()->SetLinkedClient(NULL);
	}
	// do not replace friendobjects which have no userhash, but the fitting ip with another friend object with the 
	// fitting userhash (both objects would fit to this instance), as this could lead to unwanted results
	if (GetFriend() == NULL || GetFriend()->HasUserhash() || GetFriend()->m_dwLastUsedIP != GetConnectIP()
		|| GetFriend()->m_nLastUsedPort != GetUserPort())
	{
		if ((m_Friend = theApp.friendlist->SearchFriend(m_achUserHash, m_dwUserIP, m_nUserPort)) != NULL){
			// Link the friend to that client
			m_Friend->SetLinkedClient(this);
		}
		else{
			// avoid that an unwanted client instance keeps a friend slot
			SetFriendSlot(false);
		}
	}
	else{
		// however, copy over our userhash in this case
		md4cpy(GetFriend()->m_abyUserhash, m_achUserHash);
	}


	// check for known major gpl breaker
	CString strBuffer = m_pszUsername;
	strBuffer.MakeUpper();
	strBuffer.Remove(_T(' '));
	if (strBuffer.Find(_T("EMULE-CLIENT")) != -1 || strBuffer.Find(_T("POWERMULE")) != -1 ){
		m_bGPLEvildoer = true;  
	}

	m_byInfopacketsReceived |= IP_EDONKEYPROTPACK;
	// check if at least CT_EMULEVERSION was received, all other tags are optional
	bool bIsMule = (dwEmuleTags & 0x04) == 0x04;
	if (bIsMule){
		m_bEmuleProtocol = true;
		m_byInfopacketsReceived |= IP_EMULEPROTPACK;
	}
	else if (bPrTag){
		m_bIsHybrid = true;
	}

	InitClientSoftwareVersion();

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	if(NeoPrefs.EnableSourceList())
	{
		// NEO: SHM - [SourceHashMonitor]
		if(NeoPrefs.UseSourceHashMonitor() && !theApp.sourcelist->MonitorSource(this)) // check if the cleint changed its hash to often
			LinkSource(NULL);
		else
		// NEO: SHM END
		{
			LinkSource(theApp.sourcelist->GetSource(m_achUserHash)); // give a source object to this clinet
			source->Attach(this); // and update it
		}
	}
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

	if (m_bIsHybrid)
		m_fSharedDirectories = 1;

	if (thePrefs.GetVerbose() && GetServerIP() == INADDR_NONE)
		AddDebugLogLine(false, _T("Received invalid server IP %s from %s"), ipstr(GetServerIP()), DbgGetClientInfo());

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	if(NeoPrefs.IsNickThiefDetection())
		theApp.argos->CheckForNickThief(this);
	if(NeoPrefs.IsModThiefDetection())
		theApp.argos->CheckForModThief(this);

	if(NeoPrefs.UseDLPScanner())
		theApp.argos->CheckClient(this);
#endif // ARGOS // NEO: NA END <-- Xanatos --

	return bIsMule;
}

void CUpDownClient::SendHelloPacket(){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(128);
	data.WriteUInt8(16); // size of userhash
	SendHelloTypePacket(&data);
	Packet* packet = new Packet(&data);
	packet->opcode = OP_HELLO;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__Hello", this);
	theStats.AddUpDataOverheadOther(packet->size);
	socket->SendPacket(packet,true);

	//m_bHelloAnswerPending = true;
	m_byHelloPacketState = HP_HELLO; // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
	return;
}

void CUpDownClient::SendMuleInfoPacket(bool bAnswer){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(128);
	data.WriteUInt8((uint8)theApp.m_uCurVersionShort);
	data.WriteUInt8(EMULE_PROTOCOL);
	data.WriteUInt32(7); // nr. of tags
	CTag tag(ET_COMPRESSION,1);
	tag.WriteTagToFile(&data);
	CTag tag2(ET_UDPVER,4);
	tag2.WriteTagToFile(&data);
	CTag tag3(ET_UDPPORT,thePrefs.GetUDPPort());
	tag3.WriteTagToFile(&data);
	CTag tag4(ET_SOURCEEXCHANGE,3);
	tag4.WriteTagToFile(&data);
	CTag tag5(ET_COMMENTS,1);
	tag5.WriteTagToFile(&data);
	CTag tag6(ET_EXTENDEDREQUEST,2);
	tag6.WriteTagToFile(&data);

	uint32 dwTagValue = (theApp.clientcredits->CryptoAvailable() ? 3 : 0);
	//if (thePrefs.CanSeeShares() != vsfaNobody) // set 'Preview supported' only if 'View Shared Files' allowed
	if (NeoPrefs.UseShowSharePermissions() || thePrefs.CanSeeShares() != vsfaNobody) // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
		dwTagValue |= 128;
	CTag tag7(ET_FEATURES, dwTagValue);
	tag7.WriteTagToFile(&data);

	Packet* packet = new Packet(&data,OP_EMULEPROT);
	if (!bAnswer)
		packet->opcode = OP_EMULEINFO;
	else
		packet->opcode = OP_EMULEINFOANSWER;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend(!bAnswer ? "OP__EmuleInfo" : "OP__EmuleInfoAnswer", this);
	theStats.AddUpDataOverheadOther(packet->size);
	socket->SendPacket(packet,true,true);
}

void CUpDownClient::ProcessMuleInfoPacket(const uchar* pachPacket, uint32 nSize)
{
	bool bDbgInfo = thePrefs.GetUseDebugDevice();
	m_strMuleInfo.Empty();
	CSafeMemFile data(pachPacket, nSize);
	m_byCompatibleClient = 0;
	m_byEmuleVersion = data.ReadUInt8();
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("EmuleVer=0x%x"), (UINT)m_byEmuleVersion);
	if (m_byEmuleVersion == 0x2B)
		m_byEmuleVersion = 0x22;
	uint8 protversion = data.ReadUInt8();
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("  ProtVer=%u"), (UINT)protversion);

	//implicitly supported options by older clients
	if (protversion == EMULE_PROTOCOL) {
		//in the future do not use version to guess about new features

		if (m_byEmuleVersion < 0x25 && m_byEmuleVersion > 0x22)
			m_byUDPVer = 1;

		if (m_byEmuleVersion < 0x25 && m_byEmuleVersion > 0x21)
			m_bySourceExchange1Ver = 1;

		if (m_byEmuleVersion == 0x24)
			m_byAcceptCommentVer = 1;

		// Shared directories are requested from eMule 0.28+ because eMule 0.27 has a bug in 
		// the OP_ASKSHAREDFILESDIR handler, which does not return the shared files for a 
		// directory which has a trailing backslash.
		if (m_byEmuleVersion >= 0x28 && !m_bIsML) // MLdonkey currently does not support shared directories
			m_fSharedDirectories = 1;

	} else {
		return;
	}

	uint32 tagcount = data.ReadUInt32();
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("  Tags=%u"), (UINT)tagcount);
	for (uint32 i = 0; i < tagcount; i++)
	{
		CTag temptag(&data, false);
		switch (temptag.GetNameID())
		{
			case ET_COMPRESSION:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: data compression version
				if (temptag.IsInt()) {
					m_byDataCompVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Compr=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_UDPPORT:
				// Bits 31-16: 0 - reserved
				// Bits 15- 0: UDP port
				if (temptag.IsInt()) {
					m_nUDPPort = (uint16)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  UDPPort=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_UDPVER:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: UDP protocol version
				if (temptag.IsInt()) {
					m_byUDPVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  UDPVer=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_SOURCEEXCHANGE:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: source exchange protocol version
				if (temptag.IsInt()) {
					m_bySourceExchange1Ver = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  SrcExch=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_COMMENTS:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: comments version
				if (temptag.IsInt()) {
					m_byAcceptCommentVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Commnts=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_EXTENDEDREQUEST:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: extended requests version
				if (temptag.IsInt()) {
					m_byExtendedRequestsVer = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  ExtReq=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_COMPATIBLECLIENT:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: compatible client ID
				if (temptag.IsInt()) {
					m_byCompatibleClient = (uint8)temptag.GetInt();
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Comptbl=%u"), (UINT)temptag.GetInt());
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_FEATURES:
				// Bits 31- 8: 0 - reserved
				// Bit	    7: Preview
				// Bit   6- 0: secure identification
				if (temptag.IsInt()) {
					m_bySupportSecIdent = (uint8)((temptag.GetInt()) & 3);
					m_fSupportsPreview  = (temptag.GetInt() >> 7) & 1;
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  SecIdent=%u  Preview=%u"), m_bySupportSecIdent, m_fSupportsPreview);
				}
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			
			case ET_MOD_VERSION:
				if (temptag.IsStr())
					m_strModVersion = temptag.GetStr();
				else if (temptag.IsInt())
					m_strModVersion.Format(_T("ModID=%u"), temptag.GetInt());
				else
					m_strModVersion = _T("ModID=<Unknwon>");
				DetectMod(); // NEO: MID - [ModID] <-- Xanatos -- 
				if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
				CheckForGPLEvilDoer();
				break;
			
			default:
				if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkTag=%s"), temptag.GetFullInfo());
		}
	}
	if (m_byDataCompVer == 0) {
		m_bySourceExchange1Ver = 0;
		m_byExtendedRequestsVer = 0;
		m_byAcceptCommentVer = 0;
		m_nUDPPort = 0;
	}
	if (bDbgInfo && data.GetPosition() < data.GetLength()) {
		m_strMuleInfo.AppendFormat(_T("\n  ***AddData: %u bytes"), data.GetLength() - data.GetPosition());
	}

	m_bEmuleProtocol = true;
	m_byInfopacketsReceived |= IP_EMULEPROTPACK;
	InitClientSoftwareVersion();

	if (thePrefs.GetVerbose() && GetServerIP() == INADDR_NONE)
		AddDebugLogLine(false, _T("Received invalid server IP %s from %s"), ipstr(GetServerIP()), DbgGetClientInfo());
}

void CUpDownClient::SendHelloAnswer(){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(128);
	SendHelloTypePacket(&data);
	Packet* packet = new Packet(&data);
	packet->opcode = OP_HELLOANSWER;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__HelloAnswer", this);
	theStats.AddUpDataOverheadOther(packet->size);

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	socket->SendPacket(packet, true, true);
#else
	// Servers send a FIN right in the data packet on check connection, so we need to force the response immediate
	bool bForceSend = theApp.serverconnect->AwaitingTestFromIP(GetConnectIP());
	socket->SendPacket(packet, true, true, 0, bForceSend);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

	//m_bHelloAnswerPending = false;
	m_byHelloPacketState |= HP_HELLOANSWER; // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
}

void CUpDownClient::SendHelloTypePacket(CSafeMemFile* data)
{
	data->WriteHash16(thePrefs.GetUserHash());
	uint32 clientid;
	clientid = theApp.GetID();
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	if( clientid == 0 && socket->HaveNatLayer() )
		clientid = 1; // we must send a ID != 0 otherwise the remote cleint would think we ahe High ID
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --

	data->WriteUInt32(clientid);
	data->WriteUInt16(thePrefs.GetPort());

	uint32 tagcount = 6;
	tagcount += 1; // NEO: MID - [ModID] <-- Xanatos -- 

	// Note: This is for ICSv1 BAckwards compatybility
	if ( NeoPrefs.UseIncompletePartStatus() && (m_pszUsername == NULL || !m_strModVersion.IsEmpty())) tagcount += 1; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --

	if( theApp.clientlist->GetBuddy() && theApp.IsFirewalled() )
		tagcount += 2;

	data->WriteUInt32(tagcount);

	// eD2K Name

	// TODO implement multi language website which informs users of the effects of bad mods
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	CTag tagName(CT_NAME, NeoPrefs.IsNickThiefDetection() ? theApp.argos->GetAntiNickThiefNick() : thePrefs.GetUserNick() );
#else // NEO: NA END
	CTag tagName(CT_NAME, (!m_bGPLEvildoer) ? thePrefs.GetUserNick() : _T("Please use a GPL-conform version of eMule") );
#endif // ARGOS <-- Xanatos --
	tagName.WriteTagToFile(data, utf8strRaw);

	// eD2K Version
	CTag tagVersion(CT_VERSION,EDONKEYVERSION);
	tagVersion.WriteTagToFile(data);

	// eMule UDP Ports
	uint32 kadUDPPort = 0;
	if(Kademlia::CKademlia::IsConnected())
	{
		if (Kademlia::CKademlia::GetPrefs()->GetExternalKadPort() != 0 
			&& Kademlia::CKademlia::GetPrefs()->GetUseExternKadPort()
			&& Kademlia::CUDPFirewallTester::IsVerified())
		{
			kadUDPPort = Kademlia::CKademlia::GetPrefs()->GetExternalKadPort();
		}
		else
			kadUDPPort = Kademlia::CKademlia::GetPrefs()->GetInternKadPort();
	}
	CTag tagUdpPorts(CT_EMULE_UDPPORTS, 
				((uint32)kadUDPPort			   << 16) |
				((uint32)thePrefs.GetUDPPort() <<  0)
				); 
	tagUdpPorts.WriteTagToFile(data);
	
	if( theApp.clientlist->GetBuddy() && theApp.IsFirewalled() )
	{
		CTag tagBuddyIP(CT_EMULE_BUDDYIP, theApp.clientlist->GetBuddy()->GetIP() ); 
		tagBuddyIP.WriteTagToFile(data);
	
		CTag tagBuddyPort(CT_EMULE_BUDDYUDP, 
//					( RESERVED												)
					((uint32)theApp.clientlist->GetBuddy()->GetUDPPort()  ) 
					);
		tagBuddyPort.WriteTagToFile(data);
	}

	// eMule Misc. Options #1
	const UINT uUdpVer				= 4;
	const UINT uDataCompVer			= 1;
	const UINT uSupportSecIdent		= theApp.clientcredits->CryptoAvailable() ? 3 : 0;
	// ***
	// deprecated - will be set back to 3 with the next release (to allow the new version to spread first),
	// due to a bug in earlier eMule version. Use SupportsSourceEx2 and new opcodes instead
	const UINT uSourceExchange1Ver	= 4;
	// ***
	const UINT uExtendedRequestsVer	= 2;
	const UINT uAcceptCommentVer	= 1;
	//const UINT uNoViewSharedFiles	= (thePrefs.CanSeeShares() == vsfaNobody) ? 1 : 0; // for backward compatibility this has to be a 'negative' flag
	const UINT uNoViewSharedFiles	= (!NeoPrefs.UseShowSharePermissions() && thePrefs.CanSeeShares() == vsfaNobody) ? 1 : 0; // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	const UINT uMultiPacket			= 1;
	//const UINT uSupportPreview		= (thePrefs.CanSeeShares() != vsfaNobody) ? 1 : 0; // set 'Preview supported' only if 'View Shared Files' allowed
	const UINT uSupportPreview		= (NeoPrefs.UseShowSharePermissions() || thePrefs.CanSeeShares() != vsfaNobody) ? 1 : 0; // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	const UINT uPeerCache			= 1;
	const UINT uUnicodeSupport		= 1;
	const UINT nAICHVer				= 1;
	CTag tagMisOptions1(CT_EMULE_MISCOPTIONS1, 
				(nAICHVer				<< 29) |
				(uUnicodeSupport		<< 28) |
				(uUdpVer				<< 24) |
				(uDataCompVer			<< 20) |
				(uSupportSecIdent		<< 16) |
				(uSourceExchange1Ver	<< 12) |
				(uExtendedRequestsVer	<<  8) |
				(uAcceptCommentVer		<<  4) |
				(uPeerCache				<<  3) |
				(uNoViewSharedFiles		<<  2) |
				(uMultiPacket			<<  1) |
				(uSupportPreview		<<  0)
				);
	tagMisOptions1.WriteTagToFile(data);

	// eMule Misc. Options #2
	const UINT uKadVersion			= KADEMLIA_VERSION;
	const UINT uSupportLargeFiles	= 1;
	const UINT uExtMultiPacket		= 1;
	const UINT uReserved			= 1; // mod bit // NEO: NMP - [NeoModProt] <-- Xanatos --
	const UINT uSupportsCryptLayer	= thePrefs.IsClientCryptLayerSupported() ? 1 : 0;
	const UINT uRequestsCryptLayer	= thePrefs.IsClientCryptLayerRequested() ? 1 : 0;
	const UINT uRequiresCryptLayer	= thePrefs.IsClientCryptLayerRequired() ? 1 : 0;
	const UINT uSupportsSourceEx2	= 1;
	const UINT uSupportsCaptcha		= 1;
	// direct callback is only possible if connected to kad, tcp firewalled and verified UDP open (for example on a full cone NAT)
	const UINT uDirectUDPCallback	= (Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsFirewalled()
		&& !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified()) ? 1 : 0;

	CTag tagMisOptions2(CT_EMULE_MISCOPTIONS2, 
//				(RESERVED				     )
				(uDirectUDPCallback		<< 12) |
				(uSupportsCaptcha		<< 11) |
				(uSupportsSourceEx2		<< 10) |
				(uRequiresCryptLayer	<<  9) |
				(uRequestsCryptLayer	<<  8) |
				(uSupportsCryptLayer	<<  7) |
				(uReserved				<<  6) |
				(uExtMultiPacket		<<  5) |
				(uSupportLargeFiles		<<  4) |
				(uKadVersion			<<  0) 
				);
	tagMisOptions2.WriteTagToFile(data);

	// eMule Version
	CTag tagMuleVersion(CT_EMULE_VERSION, 
				//(uCompatibleClientID		<< 24) |
				(CemuleApp::m_nVersionMjr	<< 17) |
				(CemuleApp::m_nVersionMin	<< 10) |
				(CemuleApp::m_nVersionUpd	<<  7) 
//				(RESERVED			     ) 
				);
	tagMuleVersion.WriteTagToFile(data);

	// NEO: MID - [ModID] -- Xanatos -->
	CTag tagMODVersion(CT_MOD_VERSION, CString(MOD_VERSION));
	tagMODVersion.WriteTagToFile(data);
	// NEO: MID END <-- Xanatos --

	// Note: This is for ICSv1 BAckwards compatybility
	// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
	if(NeoPrefs.UseIncompletePartStatus() && (m_pszUsername == NULL || !m_strModVersion.IsEmpty())){
		CTag tagIncompleteParts(ET_INCOMPLETEPARTS,1);
		tagIncompleteParts.WriteTagToFile(data);
	}
	// NEO: ICS END <-- Xanatos --

	uint32 dwIP;
	uint16 nPort;
	if (theApp.serverconnect->IsConnected()){
		dwIP = theApp.serverconnect->GetCurrentServer()->GetIP();
		nPort = theApp.serverconnect->GetCurrentServer()->GetPort();
#ifdef _DEBUG
		if (dwIP == theApp.serverconnect->GetLocalIP()){
			dwIP = 0;
			nPort = 0;
		}
#endif
	}
	else{
		nPort = 0;
		dwIP = 0;
	}
	data->WriteUInt32(dwIP);
	data->WriteUInt16(nPort);
//	data->WriteUInt32(dwIP); //The Hybrid added some bits here, what ARE THEY FOR?
}

// NEO: XC - [ExtendedComments] -- Xanatos -->
void CUpDownClient::ProcessMuleCommentPacket(const uchar* pachPacket, uint32 nSize){
	CSafeMemFile data(pachPacket,nSize);
	CKnownFile *file;
	KnownComment cs;

	if(IsExtendedComments()) {
		uchar filehash[16];
		if (nSize<16)
			return;
		data.Read(&filehash,16);
		file = theApp.sharedfiles->GetFileByID(filehash);
		if (!file)
			file = theApp.downloadqueue->GetFileByID(filehash);
		if (!file)
			return;
	}
	else {
		file = reqfile;
		if (!file || !file->IsPartFile())
			return;
	}
	CClientFileStatus* status = GetFileStatus(file,true); // NEO: SCFS - [SmartClientFileStatus]

	cs.m_strFileName = status->GetFileName(); // NEO: SCFS - [SmartClientFileStatus]
	if (data.GetLength() - data.GetPosition()<(sizeof(cs.m_uRating)+sizeof(UINT)))
		return;
	cs.m_uRating = data.ReadUInt8();
	ULONGLONG uLength = data.ReadUInt32();
	if (thePrefs.GetLogRatingDescReceived() && cs.m_uRating > 0)
		AddDebugLogLine(false, GetResString(IDS_RATINGRECV), cs.m_strFileName, cs.m_uRating);
	if (uLength > data.GetLength() - data.GetPosition())
		uLength = data.GetLength() - data.GetPosition();
	if (uLength > MAXFILECOMMENTLEN*3)
		uLength = MAXFILECOMMENTLEN*3;
	if (uLength > 0){
		cs.m_strComment = data.ReadString(GetUnicodeSupport()!=utf8strNone, (UINT)uLength);
		if (thePrefs.GetLogRatingDescReceived() && !cs.m_strComment.IsEmpty())
			AddDebugLogLine(false, GetResString(IDS_DESCRIPTIONRECV), cs.m_strFileName, cs.m_strComment);

		// test if comment is filtered
		if (!thePrefs.GetCommentFilter().IsEmpty())
		{
			CString strCommentLower(cs.m_strComment);
			strCommentLower.MakeLower();

			int iPos = 0;
			CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
			while (!strFilter.IsEmpty())
			{
				// comment filters are already in lowercase, compare with temp. lowercased received comment
				if (strCommentLower.Find(strFilter) >= 0)
				{
					cs.m_strComment.Empty();
					cs.m_uRating = 0;
					SetSpammer(true);
#ifdef ARGOS // NEO: NA - [NeoArgos]
					if(NeoPrefs.IsSpamerDetection() == TRUE){
						if (thePrefs.GetLogBannedClients())
							AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Comment spammer"), GetUserName(), ipstr(GetConnectIP()));
						Ban(_T("Comment spammer"));	
					}
#endif // ARGOS // NEO: NA END
					break;
				}
				strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
			}
		}
	}
	if (cs.m_uRating <= 0 && cs.m_strComment.IsEmpty())
		return;

	// NEO: SCFS - [SmartClientFileStatus]
	status->SetFileComment(cs.m_strComment);
	status->SetFileRating(cs.m_uRating);
	// NEO: SCFS END

	if (HasValidHash())
		md4cpy(cs.m_achUserHash, GetUserHash());
	cs.m_strUserName = GetUserName();
	file->AddComment(cs);
}
// NEO: XC END <-- Xanatos --

/*void CUpDownClient::ProcessMuleCommentPacket(const uchar* pachPacket, uint32 nSize)
{
	if (reqfile && reqfile->IsPartFile())
	{
		CClientFileStatus* status = GetFileStatus(reqfile,true); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --

		CSafeMemFile data(pachPacket, nSize);
		uint8 uRating = data.ReadUInt8();
		if (thePrefs.GetLogRatingDescReceived() && uRating > 0)
			//AddDebugLogLine(false, GetResString(IDS_RATINGRECV), m_strClientFilename, uRating);
			AddDebugLogLine(false, GetResString(IDS_RATINGRECV), status->GetFileName(), uRating); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
		CString strComment;
		UINT uLength = data.ReadUInt32();
		if (uLength > 0)
		{
			// we have to increase the raw max. allowed file comment len because of possible UTF8 encoding.
			if (uLength > MAXFILECOMMENTLEN*3)
				uLength = MAXFILECOMMENTLEN*3;
			strComment = data.ReadString(GetUnicodeSupport()!=utf8strNone, uLength);
			if (thePrefs.GetLogRatingDescReceived() && !strComment.IsEmpty())
				//AddDebugLogLine(false, GetResString(IDS_DESCRIPTIONRECV), m_strClientFilename, strComment);
				AddDebugLogLine(false, GetResString(IDS_DESCRIPTIONRECV), status->GetFileName(), strComment); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
				

			// test if comment is filtered
			if (!thePrefs.GetCommentFilter().IsEmpty())
			{
				CString strCommentLower(strComment);
				strCommentLower.MakeLower();

				int iPos = 0;
				CString strFilter(thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos));
				while (!strFilter.IsEmpty())
				{
					// comment filters are already in lowercase, compare with temp. lowercased received comment
					if (strCommentLower.Find(strFilter) >= 0)
					{
						strComment.Empty();
						uRating = 0;
						SetSpammer(true);
						break;
					}
					strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
				}
			}
		}
		if (!strComment.IsEmpty() || uRating > 0)
		{
			// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
			status->SetFileComment(strComment);
			status->SetFileRating(uRating);
			// NEO: SCFS END <-- Xanatos --
			//m_strFileComment = strComment;
			//m_uFileRating = uRating;
			reqfile->UpdateFileRatingCommentAvail();
		}
	}
}*/

bool CUpDownClient::Disconnected(LPCTSTR pszReason, bool bFromSocket)
{
	ASSERT( theApp.clientlist->IsValidClient(this) );
	
	// TODO LOGREMOVE
	if (m_nConnectingState == CCS_DIRECTCALLBACK)
		DebugLog(_T("Direct Callback failed - %s"), DbgGetClientInfo());
	
	if (GetKadState() == KS_QUEUED_FWCHECK_UDP || GetKadState() == KS_CONNECTING_FWCHECK_UDP)
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, true, ntohl(GetConnectIP()), 0); // inform the tester that this test was cancelled
	else if (GetKadState() == KS_FWCHECK_UDP)
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, false, ntohl(GetConnectIP()), 0); // inform the tester that this test has failed
	else if (GetKadState() == KS_CONNECTED_BUDDY)
		DebugLogWarning(_T("Buddy client disconnected - %s, %s"), pszReason, DbgGetClientInfo());
	//If this is a KAD client object, just delete it!
	SetKadState(KS_NONE);

#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
    if (GetUploadState() == US_UPLOADING || GetUploadState() == US_CONNECTING 
	|| GetUploadState() == US_WAITCALLBACK || GetUploadState() == US_WAITCALLBACKKAD
	|| GetUploadState() == US_WAITCALLBACKXS // NEO: XSB - [XSBuddy]
	)
#else
    if (GetUploadState() == US_UPLOADING || GetUploadState() == US_CONNECTING)
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
	{
		// sets US_NONE
		theApp.uploadqueue->RemoveFromUploadQueue(this, CString(_T("CUpDownClient::Disconnected: ")) + pszReason);
	}

	// 28-Jun-2004 [bc]: re-applied this patch which was in 0.30b-0.30e. it does not seem to solve the bug but
	// it does not hurt either...
	if (m_BlockRequests_queue.GetCount() > 0 || m_DoneBlocks_list.GetCount()){
		// Although this should not happen, it seems(?) to happens sometimes. The problem we may run into here is as follows:
		//
		// 1.) If we do not clear the block send requests for that client, we will send those blocks next time the client
		// gets an upload slot. But because we are starting to send any available block send requests right _before_ the
		// remote client had a chance to prepare to deal with them, the first sent blocks will get dropped by the client.
		// Worst thing here is, because the blocks are zipped and can therefore only be uncompressed when the first block
		// was received, all of those sent blocks will create a lot of uncompress errors at the remote client.
		//
		// 2.) The remote client may have already received those blocks from some other client when it gets the next
		// upload slot.
        DebugLogWarning(_T("Disconnected client with non empty block send queue; %s reqs: %s doneblocks: %s"), DbgGetClientInfo(), m_BlockRequests_queue.GetCount() > 0 ? _T("true") : _T("false"), m_DoneBlocks_list.GetCount() ? _T("true") : _T("false"));
		ClearUploadBlockRequests();
	}

	if (GetDownloadState() == DS_DOWNLOADING){
		ASSERT( m_nConnectingState == CCS_NONE );
		if (m_ePeerCacheDownState == PCDS_WAIT_CACHE_REPLY || m_ePeerCacheDownState == PCDS_DOWNLOADING)
			theApp.m_pPeerCache->DownloadAttemptFailed();
		SetDownloadState(DS_ONQUEUE, CString(_T("Disconnected: ")) + pszReason);
	}
	else{
		// ensure that all possible block requests are removed from the partfile
		ClearDownloadBlockRequests();

		if (GetDownloadState() == DS_CONNECTED){ // successfully connected, but probably didn't responsed to our filerequest
			theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
			theApp.downloadqueue->RemoveSource(this);
	    }
	}

	// we had still an AICH request pending, handle it
	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHHashSet::ClientAICHRequestFailed(this);
	}

	// The remote client does not have to answer with OP_HASHSETANSWER *immediatly* 
	// after we've sent OP_HASHSETREQUEST. It may occure that a (buggy) remote client 
	// is sending use another OP_FILESTATUS which would let us change to DL-state to DS_ONQUEUE.
	if (((GetDownloadState() == DS_REQHASHSET) || m_fHashsetRequesting) && (reqfile != NULL))
        reqfile->hashsetneeded = true;

    //if (m_iFileListRequested){
	// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
	if (m_iFileListRequested || m_bRequestingFileList){
		m_bRequestingFileList = false; 
	// NEO: XSF END <-- Xanatos --
		LogWarning(LOG_STATUSBAR, GetResString(IDS_SHAREDFILES_FAILED), GetUserName());
        m_iFileListRequested = 0;
	}

	if (m_Friend)
		theApp.friendlist->RefreshFriend(m_Friend);

	ASSERT( theApp.clientlist->IsValidClient(this) );

	//check if this client is needed in any way, if not delete it
	bool bDelete = true;
	// NEO: CI#8 - [CodeImprovement] -- Xanatos --
	/*switch(m_nUploadState){
		case US_PENDING: // NEO: MOD - [NewUploadState] <-- Xanatos --
		case US_NONEEDEDPARTS: // NEO: SCT - [SubChunkTransfer] <-- Xanatos --
		case US_ONUPLOADQUEUE:
			bDelete = false;
			break;
	}
	switch(m_nDownloadState){
		case DS_ONQUEUE:
		case DS_TOOMANYCONNS:
		case DS_NONEEDEDPARTS:
		case DS_LOWTOLOWIP:
			bDelete = false;
	}*/

	// NEO: TCR - [TCPConnectionRetry] -- Xanatos -->
	bool bFaild = false; 

	bool bRetry = false;
	bool bForceDelete = false;
	if(m_uFaildCount == (uint16)-1){
		bForceDelete = true;
		m_uFaildCount = 0; // reset
	}

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	// We don't use Connection retry on sources form long term storrage!!!
	if(IsLongTermStorred())
		bForceDelete = true;
#endif // NEO_SS // NEO: NSS END

	CPartPreferences* PartPrefs = reqfile ? reqfile->PartPrefs : &NeoPrefs.PartPrefs;

	// Note: the user may be still there just with an other server or buddy so reset the yet used relay data, and we try to get connected by the next in row

	switch(m_nUploadState){
		case US_WAITCALLBACK:
			SetServerPort(0);
			SetServerIP(0);
			break;
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		case US_WAITCALLBACKKAD: // NEO: LUC - [LowIDUploadCallBack]
			SetBuddyPort(0);
			SetBuddyIP(0);
			break;
		// NEO: XSB - [XSBuddy]
		case US_WAITCALLBACKXS:
			SetXsBuddyIP(0);
			SetXsBuddyPort(0);
			break;
		// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END
	}

	switch(m_nDownloadState){
		case DS_WAITCALLBACK:
			SetServerPort(0);
			SetServerIP(0);
			break;
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		case DS_WAITCALLBACKKAD: // NEO: LUC - [LowIDUploadCallBack]
			SetBuddyPort(0);
			SetBuddyIP(0);
			break;
		// NEO: XSB - [XSBuddy]
		case DS_WAITCALLBACKXS:
			SetXsBuddyIP(0);
			SetXsBuddyPort(0);
			break;
		// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END
	}
	// NEO: TCR END <-- Xanatos --

	// Dead Soure Handling
	//
	// If we failed to connect to that client, it is supposed to be 'dead'. Add the IP
	// to the 'dead sources' lists so we don't waste resources and bandwidth to connect
	// to that client again within the next hour.
	//
	// But, if we were just connecting to a proxy and failed to do so, that client IP
	// is supposed to be valid until the proxy itself tells us that the IP can not be
	// connected to (e.g. 504 Bad Gateway)
	//
	/*
	// X-ToDo: check this
	if ( (m_nConnectingState != CCS_NONE && !(socket && socket->GetProxyConnectFailed()))
		|| m_nDownloadState == DS_ERROR)
	{
		if (m_nDownloadState != DS_NONE) // Unable to connect = Remove any downloadstate
			theApp.downloadqueue->RemoveSource(this);
		theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
		bDelete = true;
	}
	*/
	bool bAddDeadSource = true;
	switch(m_nUploadState){
		// NEO: CI#8 - [CodeImprovement] -- Xanatos -->
		case US_PENDING: // NEO: MOD - [NewUploadState]
		case US_NONEEDEDPARTS: // NEO: SCT - [SubChunkTransfer]
		case US_ONUPLOADQUEUE:
			bDelete = false;
			break;
		// NEO: CI#8 END <-- Xanatos --
		case US_CONNECTING:
			if (socket && socket->GetProxyConnectFailed())
				bAddDeadSource = false;
			//if (thePrefs.GetLogUlDlEvents())
            //    AddDebugLogLine(DLP_VERYLOW, true,_T("Removing connecting client from upload list: %s Client: %s"), pszReason, DbgGetClientInfo());
		case US_WAITCALLBACK:
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		case US_WAITCALLBACKKAD: // NEO: LUC - [LowIDUploadCallBack]
		case US_WAITCALLBACKXS: // NEO: XSB - [XSBuddy]
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			bFaild = true; // NEO: TCR - [TCPConnectionRetry] <-- Xanatos --

			// NEO: UPC - [UploadingProblemClient] -- Xanatos -->
			theApp.uploadqueue->RemoveFromUploadQueue(this,pszReason);
			m_fUpIsProblematic = 1;

			//back to queue
			theApp.uploadqueue->AddClientDirectToQueue(this);

			bDelete = false;
			bRetry = true;
			break;
			// NEO: UPC END <-- Xanatos --
		case US_ERROR:
			// NEO: CI#8 - [CodeImprovement] -- Xanatos --
			bDelete = true;
	}

	//bAddDeadSource = true; // NEO: CI#8 - [CodeImprovement] <-- Xanatos --
	switch(m_nDownloadState){
		// NEO: CI#8 - [CodeImprovement] -- Xanatos -->
		case DS_ONQUEUE:
		case DS_REMOTEQUEUEFULL: // NEO: FIX - [SourceCount]
		case DS_TOOMANYCONNS:
		case DS_NONEEDEDPARTS:
		case DS_LOWTOLOWIP:
			bDelete = false;
			break;
		// NEO: CI#8 END <-- Xanatos --
		case DS_CONNECTING:
			if (socket && socket->GetProxyConnectFailed())
				bAddDeadSource = false;
		case DS_WAITCALLBACK:
		// NEO: TCR - [TCPConnectionRetry] -- Xanatos -->
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		case DS_WAITCALLBACKKAD: // NEO: LUC - [LowIDUploadCallBack]
		case DS_WAITCALLBACKXS: // NEO: XSB - [XSBuddy]
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			bFaild = true;
		
			if(!PartPrefs->GetTCPConnectionRetry() || bForceDelete || IsBanned())
			{
				bRetry = false;
			}
			else if (IsSourceConfirmed() && GetFaildCount() < PartPrefs->GetTCPConnectionRetry())
			{
				SetNextAskedTime(SEC2MS(50));
				bRetry = true;
			}
			else if (GetFaildCount() < PartPrefs->GetTCPNewConnectionRetry())
			{
				SetNextAskedTime(SEC2MS(70));
				bRetry = true;
			}

			if(bRetry){
				SetDownloadState(DS_CONNECTIONRETRY);
				bDelete = false;
			}
		// NEO: TCR END <-- Xanatos --
		case DS_ERROR:
			// NEO: CI#8 - [CodeImprovement] -- Xanatos --
			bDelete = true;
	}

	// NEO: CI#8 - [CodeImprovement] -- Xanatos -->
	if(bFaild){
		theApp.CountConnectionFailed(); // NEO: NCC - [NeoConnectionChecker]

		if(!bRetry && bAddDeadSource){
			if (m_nDownloadState != DS_NONE) // Unable to connect = Remove any downloadstate
				theApp.downloadqueue->RemoveSource(this);
			theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
		}
		IncrementFaildCount(); // NEO: TCR - [TCPConnectionRetry]
	}
	// NEO: CI#8 END <-- Xanatos --

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	if(bFaild && source)
		source->ConnectionFaild();
#endif // NEO_CD // NEO: NCD END

	// We keep chat partners in any case
	if (GetChatState() != MS_NONE){
		bDelete = false;
		if (GetFriend() != NULL && GetFriend()->IsTryingToConnect())
			GetFriend()->UpdateFriendConnectionState(FCR_DISCONNECTED); // for friends any connectionupdate is handled in the friend class
		else
			theApp.emuledlg->chatwnd->chatselector.ConnectingResult(this,false); // other clients update directly
	}
	
	// Delete Socket
	if (!bFromSocket && socket){
		ASSERT( theApp.listensocket->IsValidSocket(socket) );
		socket->Safe_Delete();
	}
	socket = NULL;

	// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
	m_bNotifyIdChage = false;
	m_bWainingNotifyIdChage = false;
	// NEO: RIC END <-- Xanatos --

	theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);

	// finally, remove the client from the timeouttimer and reset the connecting state
	m_nConnectingState = CCS_NONE;
	theApp.clientlist->RemoveConnectingClient(this);

	if (bDelete)
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			Debug(_T("--- Deleted client            %s; Reason=%s\n"), DbgGetClientInfo(true), pszReason);
		return true;
	}
	else
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			Debug(_T("--- Disconnected client       %s; Reason=%s\n"), DbgGetClientInfo(true), pszReason);
		m_fHashsetRequesting = 0;
		SetSentCancelTransfer(0);
		//m_bHelloAnswerPending = false;
		m_byHelloPacketState = HP_NONE; // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
		m_byFileRequestState = FR_NONE; // NEO: USPS - [UnSolicitedPartStatus] <-- Xanatos --
		m_fQueueRankPending = 0;
		m_fFailedFileIdReqs = 0;
		m_fUnaskQueueRankRecv = 0;
		SetXsBuddyStatus(XB_NONE); // NEO: XSB - [XSBuddy]
		m_uPeerCacheDownloadPushId = 0;
		m_uPeerCacheUploadPushId = 0;
		m_uPeerCacheRemoteIP = 0;
		SetPeerCacheDownState(PCDS_NONE);
		SetPeerCacheUpState(PCUS_NONE);
		if (m_pPCDownSocket){
			m_pPCDownSocket->client = NULL;
			m_pPCDownSocket->Safe_Delete();
		}
		if (m_pPCUpSocket){
			m_pPCUpSocket->client = NULL;
			m_pPCUpSocket->Safe_Delete();
		}
		m_fSentOutOfPartReqs = 0;
		m_Dirs2Update.RemoveAll(); // NEO: XSF - [ExtendedSharedFiles] <-- Xanatos --
		return false;
	}
}

//Returned bool is not if the TryToConnect is successful or not..
//false means the client was deleted!
//true means the client was not deleted!
bool CUpDownClient::TryToConnect(bool bIgnoreMaxCon, bool bNoCallbacks, CRuntimeClass* pClassSocket)
{
	// NEO: FCC - [FixConnectionCollision] -- Xanatos -->
	if(socket != NULL && socket->GetConState() != ES_DISCONNECTED){
		if (CheckHandshakeFinished())
			ConnectionEstablished();
		else if (m_byHelloPacketState == HP_NONE)
			DebugLog(LOG_SUCCESS, _T("[FIX CONNECTION COLLISION] Already initiated socket, Waiting for an OP_HELLO from client: %s"), DbgGetClientInfo());
		else if (m_byHelloPacketState == HP_HELLO)
			DebugLog(LOG_SUCCESS, _T("[FIX CONNECTION COLLISION] Already initiated socket, OP_HELLO already sent and waiting an OP_HELLOANSWER from client: %s"), DbgGetClientInfo()); 
		else if (m_byHelloPacketState == HP_HELLOANSWER)
			DebugLog(LOG_ERROR, _T("[FIX CONNECTION COLLISION] Already initiated socket, OP_HELLOANSWER without OP_HELLO from client: %s"), DbgGetClientInfo()); 
		return true;
	}
	// NEO: FCC END <-- Xanatos --

	// There are 7 possible ways how we are going to connect in this function, sorted by priority:
	// 1) Already Connected/Connecting
	//		We are already connected or try to connect right now. Abort, no additional Disconnect() call will be done
	// 2) Immediate Fail
	//		Some precheck or precondition failed, or no other way is available, so we do not try to connect at all
	//		but fail right away, possibly deleting the client as it becomes useless
	// 3) Normal Outgoing TCP Connection
	//		Applies to all HighIDs/Open clients: We do a straight forward connection try to the TCP port of the client
	// 4) Direct Callback Connections
	//		Applies to TCP firewalled - UDP open clients: We sent a UDP packet to the client, requesting him to connect
	//		to us. This is pretty easy too and ressourcewise nearly on the same level as 3)
	// (* 5) Waiting/Abort
	//		This check is done outside this function.
	//		We want to connect for some download related thing (for example reasking), but the client has a LowID and
	//		is on our uploadqueue. So we are smart and safing ressources by just waiting untill he reasks us, so we don't
	//		have to do the ressource intensive options 6 or 7. *)
	// 6) Server Callback
	//		This client is firewalled, but connected to our server. We sent the server a callback request to forward to
	//		the client and hope for the best
	// 7) Kad Callback
	//		This client is firewalled, but has a Kad buddy. We sent the buddy a callback request to forward to the client
	//		and hope for the best

	if( GetKadState() == KS_QUEUED_FWCHECK )
		SetKadState(KS_CONNECTING_FWCHECK);
	else if (GetKadState() == KS_QUEUED_FWCHECK_UDP)
		SetKadState(KS_CONNECTING_FWCHECK_UDP);

	////////////////////////////////////////////////////////////
	// Check for 1) Already Connected/Connecting
	if (m_nConnectingState != CCS_NONE) {
		DebugLog(_T("TryToConnect: Already Connecting (%s)"), DbgGetClientInfo());// TODO LogRemove
		return true;
	}
	else if (socket != NULL){
		if (socket->IsConnected())
		{
			if (CheckHandshakeFinished()){
				DEBUG_ONLY( DebugLog(_T("TryToConnect: Already Connected (%s)"), DbgGetClientInfo()) );// TODO LogRemove
				ConnectionEstablished();
			}
			else
				DebugLogWarning( _T("TryToConnect found connected socket, but without Handshake finished - %s"), DbgGetClientInfo());
			return true;
		}
		else
			socket->Safe_Delete();
	}
	m_nConnectingState = CCS_PRECONDITIONS; // We now officially try to connect :)

	////////////////////////////////////////////////////////////
	// Check for 2) Immediate Fail

	if (theApp.listensocket->TooManySockets() && !bIgnoreMaxCon)
	{
		// This is a sanitize check and counts as a "hard failure", so this check should be also done before calling
		// TryToConnect if a special handling, like waiting till there are enough connection avaiable should be fone
		DebugLogWarning(_T("TryToConnect: Too many connections sanitize check (%s)"), DbgGetClientInfo());
		if(Disconnected(_T("Too many connections")))
		{
			delete this;
			return false;
		}
		return true;
	}
	// do not try to connect to source which are incompatible with our encryption setting (one requires it, and the other one doesn't supports it)
	if ( (RequiresCryptLayer() && !thePrefs.IsClientCryptLayerSupported()) || (thePrefs.IsClientCryptLayerRequired() && !SupportsCryptLayer()) )
	{
		DEBUG_ONLY( AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected outgoing connection because CryptLayer-Setting (Obfuscation) was incompatible %s"), DbgGetClientInfo()) );
		if(Disconnected(_T("CryptLayer-Settings (Obfuscation) incompatible"))){
			delete this;
			return false;
		}
		else
			return true;
	}

	uint32 uClientIP = (GetIP() != 0) ? GetIP() : GetConnectIP();
	if (uClientIP == 0 && !HasLowID())
		uClientIP = ntohl(m_nUserIDHybrid);
	if (uClientIP)
	{
		// although we filter all received IPs (server sources, source exchange) and all incomming connection attempts,
		// we do have to filter outgoing connection attempts here too, because we may have updated the ip filter list
		//if (theApp.ipfilter->IsFiltered(uClientIP))
		if (
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		 !theApp.lancast->IsLanIP(uClientIP) &&
#endif //LANCAST // NEO: NLC END <-- Xanatos --
		  theApp.ipfilter->IsFiltered(uClientIP))
		{
			theStats.filteredclients++;
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(true, GetResString(IDS_IPFILTERED), ipstr(uClientIP), theApp.ipfilter->GetLastHit());
			SetForDelete(); // NEO: TCR - [TCPConnectionRetry] <-- Xanatos --
			if (Disconnected(_T("IPFilter")))
			{
				delete this;
				return false;
			}
			return true;
		}

		// for safety: check again whether that IP is banned
		if (theApp.clientlist->IsBannedClient(uClientIP))
		{
			if (thePrefs.GetLogBannedClients())
				AddDebugLogLine(false, _T("Refused to connect to banned client %s"), DbgGetClientInfo());
			SetForDelete(); // NEO: TCR - [TCPConnectionRetry] <-- Xanatos --
			if (Disconnected(_T("Banned IP")))
			{
				delete this;
				return false;
			}
			return true;
		}
	}

	if ( HasLowID() )
	{
		ASSERT( pClassSocket == NULL );
		if(!theApp.CanDoCallback(this)) // lowid2lowid check used for the whole function, don't remove
		{
			// We cannot reach this client, so we hard fail to connect, if this client should be kept,
			// for example because we might want to wait a bit and hope we get a highid, this check has
			// to be done before calling this function
			if(Disconnected(_T("LowID->LowID")))
			{
				delete this;
				return false;
			}
			return true;
		}

		// are callbacks disallowed?
		if (bNoCallbacks){
			DebugLogError(_T("TryToConnect: Would like to do callback on a no-callback client, %s"), DbgGetClientInfo());
			if(Disconnected(_T("LowID: No Callback Option allowed")))
			{
				delete this;
				return false;
			}
			return true;
		}

		// Is any callback available?
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		if (!( (SupportsDirectUDPCallback() && thePrefs.GetUDPPort() != 0 && GetConnectIP() != 0) // Direct Callback
			|| (HasValidBuddyID() && Kademlia::CKademlia::IsConnected()) // Kad Callback
			|| theApp.serverconnect->CanCallback(GetServerIP(), GetServerPort()) )) // Server Callback
#else
		if (!( (SupportsDirectUDPCallback() && thePrefs.GetUDPPort() != 0 && GetConnectIP() != 0) // Direct Callback
			|| (HasValidBuddyID() && Kademlia::CKademlia::IsConnected()) // Kad Callback
			|| theApp.serverconnect->IsLocalServer(GetServerIP(), GetServerPort()) )) // Server Callback
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

		{
			// Nope
			if(Disconnected(_T("LowID: No Callback Option available")))
			{
				delete this;
				return false;
			}
			return true;
		}
	}

	// Prechecks finished, now for the real connecting
	////////////////////////////////////////////////////

	theApp.clientlist->AddConnectingClient(this); // Starts and checks for the timeout, ensures following Disconnect() or ConnectionEstablished() call

	////////////////////////////////////////////////////////////
	// 3) Normal Outgoing TCP Connection
	if (!HasLowID())
	{
		m_nConnectingState = CCS_DIRECTTCP;
		if (pClassSocket == NULL)
			pClassSocket = RUNTIME_CLASS(CClientReqSocket);
		socket = static_cast<CClientReqSocket*>(pClassSocket->CreateObject());
		socket->SetClient(this);
		if (!socket->Create())
		{
			socket->Safe_Delete();
			// we let the timeout handle the cleanup in this case
			DebugLogError(_T("TryToConnect: Failed to create socket for outgoing connection, %s"), DbgGetClientInfo());
		}
		else
			Connect();
		return true;
	}
	////////////////////////////////////////////////////////////
	// 4) Direct Callback Connections
	else if (SupportsDirectUDPCallback() && thePrefs.GetUDPPort() != 0 && GetConnectIP() != 0)
	{
		m_nConnectingState = CCS_DIRECTCALLBACK;
		// TODO LOGREMOVE
		DebugLog(_T("Direct Callback on port %u to client %s (%s) "), GetKadPort(), DbgGetClientInfo(), md4str(GetUserHash()));
		CSafeMemFile data;
		data.WriteUInt16(thePrefs.GetPort()); // needs to know our port
		data.WriteHash16(thePrefs.GetUserHash()); // and userhash
		// our connection settings
		data.WriteUInt8(GetMyConnectOptions(true, false));
		if (thePrefs.GetDebugClientUDPLevel() > 0)
			DebugSend("OP_DIRECTCALLBACKREQ", this);
		Packet* packet = new Packet(&data, OP_EMULEPROT);
		packet->opcode = OP_DIRECTCALLBACKREQ;
		theStats.AddUpDataOverheadOther(packet->size);
		theApp.clientudp->SendPacket(packet, GetConnectIP(), GetKadPort(), ShouldReceiveCryptUDPPackets(), GetUserHash(), false, 0);
		return true;
	}
	////////////////////////////////////////////////////////////
	// 6) Server Callback + 7) Kad Callback
	if (GetDownloadState() == DS_CONNECTING)
		SetDownloadState(DS_WAITCALLBACK);
	
	if (GetUploadState() == US_CONNECTING){
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
		SetUploadState(US_WAITCALLBACK);
#else
		ASSERT( false ); // we should never try to connect in this case, but wait for the LowID to connect to us
		DebugLogError( _T("LowID and US_CONNECTING (%s)"), DbgGetClientInfo());
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
	}

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	// If we are here and booth cleints are firewalled, we already know booth support Nat Traversal
	if( HasLowID() && (theApp.IsFirewalled() || GetUserPort() == 0)) // GetUserPort() == 0 for manualy added debug cleints
	{
		if (pClassSocket == NULL)
			pClassSocket = RUNTIME_CLASS(CClientReqSocket);
		socket = static_cast<CClientReqSocket*>(pClassSocket->CreateObject());
		socket->SetClient(this);
		socket->InitNatSupport();
		if (!socket->Create())
		{
			socket->Safe_Delete();
			// we let the timeout handle the cleanup in this case
			DebugLogError(_T("TryToConnect: Failed to create socket for outgoing connection, %s"), DbgGetClientInfo());
		}
		else if (theApp.natmanager->PrepareConnect(this)) // if we have a tunel we can connect imminetly
			Connect();
		return true;
	}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --


#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	if (theApp.serverconnect->CanCallback(m_dwServerIP,m_nServerPort))
#else
	if (theApp.serverconnect->IsLocalServer(m_dwServerIP, m_nServerPort))
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
	{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		//if(theApp.serverconnect->IsLowID())
		if(socket && socket->GetNatLayer())
		{
			if(CNatTunnel* tunnel = theApp.natmanager->PrepareTunnel(this))
			{
				tunnel->PrepareCallback((uchar*)&m_nUserIDHybrid, ed2k_id);
				if(GetUDPPort() != 0)
					theApp.natmanager->SendNatPing(GetConnectIP(),GetUDPPort(),true,0,0,true);
				else
					tunnel->SendCallback();
			}
			return true;
		}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		m_nConnectingState = CCS_SERVERCALLBACK;
		Packet* packet = new Packet(OP_CALLBACKREQUEST,4);
		PokeUInt32(packet->pBuffer, m_nUserIDHybrid);
		if (thePrefs.GetDebugServerTCPLevel() > 0 || thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__CallbackRequest", this);
		theStats.AddUpDataOverheadServer(packet->size);
		theApp.serverconnect->SendPacket(packet);
		return true;
	}
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
	// Note: now we have an own "buddy" substitute over XS so if we can we pick it, for nat anyway,
	//			for normal callbacks to, this way we dont drop every secund reask and at the same we lower the kad load
	else if(GetXsBuddyIP() && HasValidHash())
	{
		// NEO: NATT - [NatTraversal]
		if(socket && socket->GetNatLayer())
		{
			if(CNatTunnel* tunnel = theApp.natmanager->PrepareTunnel(this))
			{
				tunnel->PrepareCallback(GetUserHash(), xs_id, GetXsBuddyIP(), GetXsBuddyPort(), true);
				if(GetUDPPort() != 0)
					theApp.natmanager->SendNatPing(GetConnectIP(),GetUDPPort(),true,0,0,true);
				else
					tunnel->SendCallback();
			}
		}
		else
		// NEO: NATT END
		{
			CSafeMemFile data(128);
			data.WriteHash16(GetUserHash());

			// write our hash
			data.WriteHash16(thePrefs.GetUserHash());

			// write multi callback opcode
			data.WriteUInt8(OP_CALLBACKREQUEST_XS);

			// write out TCP port
			data.WriteUInt16(thePrefs.GetPort());

			//Obfuscation support
			if (HasValidHash() && SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested()))
			{
				const UINT uSupportsCryptLayer	= thePrefs.IsClientCryptLayerSupported() ? 1 : 0;
				const UINT uRequestsCryptLayer	= thePrefs.IsClientCryptLayerRequested() ? 1 : 0;
				const UINT uRequiresCryptLayer	= thePrefs.IsClientCryptLayerRequired() ? 1 : 0;
				const UINT uObfuscation =
				(uRequiresCryptLayer	<<  2) |
				(uRequestsCryptLayer	<<  1) |
				(uSupportsCryptLayer	<<  0);
				data.WriteUInt8((uint8)uObfuscation);
				data.WriteHash16(thePrefs.GetUserHash()); 
			}

			// send packet over buddy
			Packet* packet = new Packet(&data, OP_MODPROT);
			packet->opcode = OP_XS_MULTICALLBACKUDP;
			theStats.AddUpDataOverheadFileRequest(packet->size);
			theApp.clientudp->SendPacket(packet, GetXsBuddyIP(), GetXsBuddyPort(), false, NULL, true, 0);
		}

		// NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
		if( GetUploadState() == US_WAITCALLBACK )
			SetUploadState(US_WAITCALLBACKXS);

		if( GetDownloadState() == DS_WAITCALLBACK )
		// NEO: LUC END
			SetDownloadState(DS_WAITCALLBACKXS);
		return true;
	}
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
	else if (HasValidBuddyID() && Kademlia::CKademlia::IsConnected())
	{
		m_nConnectingState = CCS_KADCALLBACK;
		if( GetBuddyIP() && GetBuddyPort())
		{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			//if(Kademlia::CKademlia::IsFirewalled())
			if(socket && socket->GetNatLayer())
			{
				if(CNatTunnel* tunnel = theApp.natmanager->PrepareTunnel(this))
				{
					tunnel->PrepareCallback(GetBuddyID(), kad_id, GetBuddyIP(), GetBuddyPort(), true);
					if(GetUDPPort() != 0)
						theApp.natmanager->SendNatPing(GetConnectIP(),GetUDPPort(),true,0,0,true);
					else
						tunnel->SendCallback();
				}
				return true;
			}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

			CSafeMemFile bio(34);
			bio.WriteUInt128(&Kademlia::CUInt128(GetBuddyID()));
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
			if( GetDownloadState() != DS_WAITCALLBACK )
				bio.WriteUInt128(&Kademlia::CUInt128(requpfileid));
			else
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
			bio.WriteUInt128(&Kademlia::CUInt128(reqfile->GetFileHash()));
			bio.WriteUInt16(thePrefs.GetPort());
			if (thePrefs.GetDebugClientKadUDPLevel() > 0 || thePrefs.GetDebugClientUDPLevel() > 0)
				DebugSend("KadCallbackReq", this);
			Packet* packet = new Packet(&bio, OP_KADEMLIAHEADER);
			packet->opcode = KADEMLIA_CALLBACK_REQ;
			theStats.AddUpDataOverheadKad(packet->size);
			// FIXME: We dont know which kadversion the buddy has, so we need to send unencrypted
			theApp.clientudp->SendPacket(packet, GetBuddyIP(), GetBuddyPort(), false, NULL, true, 0);
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
			if( GetUploadState() == US_WAITCALLBACK )
				SetUploadState(US_WAITCALLBACKKAD);

			if( GetDownloadState() == DS_WAITCALLBACK )
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
			SetDownloadState(DS_WAITCALLBACKKAD);
		}
		else
		{
			// I don't think we should ever have a buddy without its IP (anymore), but nevertheless let the functionality in
			//Create search to find buddy.
			Kademlia::CSearch *findSource = new Kademlia::CSearch;
			findSource->SetSearchTypes(Kademlia::CSearch::FINDSOURCE);
			findSource->SetTargetID(Kademlia::CUInt128(GetBuddyID()));
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
			if( GetDownloadState() != DS_WAITCALLBACK )
				findSource->AddFileID(Kademlia::CUInt128(requpfileid));
			else
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
			findSource->AddFileID(Kademlia::CUInt128(reqfile->GetFileHash()));
			if( Kademlia::CKademlia::GetPrefs()->GetTotalSource() > 0 || Kademlia::CSearchManager::AlreadySearchingFor(Kademlia::CUInt128(GetBuddyID())))
			{
				//There are too many source lookups already or we are already searching this key.
				// bad luck, as lookups aren't supposed to hapen anyway, we just let it fail, if we want
				// to actually really use lookups (so buddies without known IPs), this should be reworked
				// for example by adding a queuesystem for queries
				DebugLogWarning(_T("TryToConnect: Buddy without knonw IP, Lookup crrently impossible"));
				return true;
			}
			if(Kademlia::CSearchManager::StartSearch(findSource))
			{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
				//if(Kademlia::CKademlia::IsFirewalled())
				if(socket && socket->GetNatLayer())
				{
					if(CNatTunnel* tunnel = theApp.natmanager->PrepareTunnel(this))
					{
						tunnel->PrepareCallback(GetBuddyID(), kad_id, 0, 0, true, false);
					}
				}
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
				//Started lookup..
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
				if( GetUploadState() == US_WAITCALLBACK )
					SetUploadState(US_WAITCALLBACKKAD);

				if( GetDownloadState() == DS_WAITCALLBACK )
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
				SetDownloadState(DS_WAITCALLBACKKAD);
			}
			else
			{
				//This should never happen..
				ASSERT(0);
			}
		}
		return true;
	}
	else {
		ASSERT( false );
		DebugLogError(_T("TryToConnect: Bug: No Callback available despite prechecks"));
		return true;
	}
}

void CUpDownClient::Connect()
{
	// enable or disable crypting based on our and the remote clients preference
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	if (socket->HaveNatLayer() == false && // with NAT-T the NatManager will take care of obfuscation
		HasValidHash() && SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested())){
#else
	if (HasValidHash() && SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested())){
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
		//DebugLog(_T("Enabling CryptLayer on outgoing connection to client %s"), DbgGetClientInfo()); // to be removed later
		socket->SetConnectionEncryption(true, GetUserHash(), false);
	}
	else
		socket->SetConnectionEncryption(false, NULL, false);

	//Try to always tell the socket to WaitForOnConnect before you call Connect.
	socket->WaitForOnConnect();
	SOCKADDR_IN sockAddr = {0};
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_port = htons(GetUserPort());
	sockAddr.sin_addr.S_un.S_addr = GetConnectIP();
	socket->Connect((SOCKADDR*)&sockAddr, sizeof sockAddr);
	SendHelloPacket();
}

void CUpDownClient::ConnectionEstablished()
{
	// ok we have a connection, lets see if we want anything from this client
	
	// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	if(!IsLanClient())
#endif //LANCAST // NEO: NLC END
	{
		if(theApp.IsConnectionFailed() && !theApp.serverconnect->IsConnected()){ // connection failed and no server, get new public IP/Port
			theApp.SetPublicIP(0);
#ifdef NATTUNNELING // NEO: RTP - [ReuseTCPPort]
			theApp.SetPublicPort(0);
#endif //NATTUNNELING // NEO: RTP END
		}
		theApp.SetConnectionEstablished(); // reset
	}
	// NEO: NCC END <-- Xanatos --

	// was this a direct callback?
	if (m_nConnectingState == CCS_DIRECTCALLBACK) // TODO LOGREMOVE
		DebugLog(_T("Direct Callback succeeded, connection established - %s"), DbgGetClientInfo()); 

	// remove the connecting timer and state
	//if (m_nConnectingState == CCS_NONE) // TODO LOGREMOVE
	//	DEBUG_ONLY( DebugLog(_T("ConnectionEstablished with CCS_NONE (incoming, thats fine)")) );
	m_nConnectingState = CCS_NONE;
	theApp.clientlist->RemoveConnectingClient(this);

	ResetFaildCount(); // NEO: TCR - [TCPConnectionRetry] <-- Xanatos --
	SetLastSeen(); // NEO: MOD - [LastSeen] <-- Xanatos --

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	if(source)
		source->ConnectionSuccess();
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

	// check if we should use this client to retrieve our public IP
	//if (theApp.GetPublicIP() == 0 && theApp.IsConnected() && m_fPeerCache)
	if (theApp.GetPublicIP() == 0 && m_fPeerCache) // NEO: NCC - [NeoConnectionChecker] <-- Xanatos --
		SendPublicIPRequest();

#ifdef NATTUNNELING // NEO: RTP - [ReuseTCPPort] -- Xanatos -->
	if (NeoPrefs.ReuseTCPPort() && theApp.GetPublicPort() == 0 && theApp.IsFirewalled() && m_fSupportsNatTraversal)
		SendPublicPortRequest();
#endif //NATTUNNELING // NEO: RTP END <-- Xanatos --

	switch(GetKadState())
	{
		case KS_CONNECTING_FWCHECK:
            SetKadState(KS_CONNECTED_FWCHECK);
			break;
		case KS_CONNECTING_BUDDY:
		case KS_INCOMING_BUDDY:
			DEBUG_ONLY( DebugLog(_T("Set KS_CONNECTED_BUDDY for client %s"), DbgGetClientInfo()) );
			SetKadState(KS_CONNECTED_BUDDY);
			break;
		case KS_CONNECTING_FWCHECK_UDP:
			SetKadState(KS_FWCHECK_UDP);
			DEBUG_ONLY( DebugLog(_T("Set KS_FWCHECK_UDP for client %s"), DbgGetClientInfo()) );
			SendFirewallCheckUDPRequest();
			break;
	}

#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
	if(GetXsBuddyStatus() == XB_CONNECTING)
		SendXsBuddyRequest();
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --

	if (GetChatState() == MS_CONNECTING || GetChatState() == MS_CHATTING)
	{
		if (GetFriend() != NULL && GetFriend()->IsTryingToConnect()){
			GetFriend()->UpdateFriendConnectionState(FCR_ESTABLISHED); // for friends any connectionupdate is handled in the friend class
			if (credits != NULL && credits->GetCurrentIdentState(GetConnectIP()) == IS_IDFAILED)
				GetFriend()->UpdateFriendConnectionState(FCR_SECUREIDENTFAILED);
		}
		else
			theApp.emuledlg->chatwnd->chatselector.ConnectingResult(this, true); // other clients update directly
	}

	switch(GetDownloadState())
	{
		case DS_CONNECTING:
		case DS_WAITCALLBACK:
		case DS_WAITCALLBACKKAD:
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
		case DS_WAITCALLBACKXS:
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
			m_bReaskPending = false;
			SetDownloadState(DS_CONNECTED);
			SendFileRequest();
			break;
	}

	// NEO: L2HAC - [LowID2HighIDAutoCallback] -- Xanatos -->
	if(reqfile && SupportsL2HAC() && HasLowID() && CanDoL2HAC()){
		SetNextL2HAC(MIN_REQUESTTIME);
		m_bReaskPending = false;
		m_dwLastTriedToConnect = ::GetTickCount();
		SetDownloadState(DS_CONNECTED);
		SendFileRequest();
	}
	// NEO: L2HAC END <-- Xanatos --

	if (m_bReaskPending)
	{
		m_bReaskPending = false;
		if (GetDownloadState() != DS_NONE && GetDownloadState() != DS_DOWNLOADING)
		{
			SetDownloadState(DS_CONNECTED);
			SendFileRequest();
		}
	}

	switch(GetUploadState())
	{
		case US_CONNECTING:
		case US_WAITCALLBACK:
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
		case US_WAITCALLBACKKAD:
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
		case US_WAITCALLBACKXS:
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
			if (theApp.uploadqueue->IsDownloading(this))
			{
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
				socket->SetPriorityReceive(true);
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
				socket->SetPrioritySend(true);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
				SetUploadState(US_UPLOADING);
				SendAcceptUpload(); // NEO: MOD - [SendAcceptUpload] <-- Xanatos --
				/*if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__AcceptUploadReq", this);
				Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
				theStats.AddUpDataOverheadFileRequest(packet->size);
				socket->SendPacket(packet,true);*/
			}
	}

	//if (m_iFileListRequested == 1)
	//{
	// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
	if (m_bRequestingFileList)
	{
		m_bRequestingFileList = false;
		m_iFileListRequested++;
	// NEO: XSF END <-- Xanatos --
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend(m_fSharedDirectories ? "OP__AskSharedDirs" : "OP__AskSharedFiles", this);
        Packet* packet = new Packet(m_fSharedDirectories ? OP_ASKSHAREDDIRS : OP_ASKSHAREDFILES,0);
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true,true);
	}

	// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
	/* Xanatos:
	* This solution is much beter that the normal "Maella -Inform sources of an ID change-",
	* by using this opcode instad of an compleer reask we can save some overhead
	* and avoid to be banned...
	*/
	if(m_bNotifyIdChage){
		m_bNotifyIdChage = false;
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__Change_Client_Id", this);
		Packet* packet = new Packet(OP_CHANGE_CLIENT_ID, 8);
		PokeUInt32(packet->pBuffer, theApp.GetPublicIP()); // New ID
		PokeUInt32(packet->pBuffer + 4, theApp.serverconnect->IsConnected() ? theApp.serverconnect->GetCurrentServer()->GetIP() : 0x00000000); // New Server IP
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true);
	}
	// NEO: RIC END <-- Xanatos --

	while (!m_WaitingPackets_list.IsEmpty())
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("Buffered Packet", this);
		socket->SendPacket(m_WaitingPackets_list.RemoveHead());
	}

}

void CUpDownClient::InitClientSoftwareVersion()
{
	if (m_pszUsername == NULL){
		m_clientSoft = SO_UNKNOWN;
		return;
	}

	int iHashType = GetHashType();
	if (m_bEmuleProtocol || iHashType == SO_EMULE){
		LPCTSTR pszSoftware;
		switch(m_byCompatibleClient){
			case SO_CDONKEY:
				m_clientSoft = SO_CDONKEY;
				pszSoftware = _T("cDonkey");
				break;
			case SO_XMULE:
				m_clientSoft = SO_XMULE;
				pszSoftware = _T("xMule");
				break;
			case SO_AMULE:
				m_clientSoft = SO_AMULE;
				pszSoftware = _T("aMule");
				break;
			case SO_SHAREAZA:
			// NEO: ECR - [EnhancedClientRecognization] -- Xanatos -->
			case SO_SHAREAZA2:
			case SO_SHAREAZA3:
			case SO_SHAREAZA4:
			// NEO: ECR END <-- Xanatos --
			//case 40:
				m_clientSoft = SO_SHAREAZA;
				pszSoftware = _T("Shareaza");
				break;
			case SO_LPHANT:
				m_clientSoft = SO_LPHANT;
				pszSoftware = _T("lphant");
				break;
			// NEO: ECR - [EnhancedClientRecognization] -- Xanatos -->
			case SO_EMULEPLUS:
				m_clientSoft = SO_EMULEPLUS;
				pszSoftware = _T("eMule Plus");
				break;
			case SO_HYDRANODE:
				m_clientSoft = SO_HYDRANODE;
				pszSoftware = _T("Hydranode");
				break;
			case SO_TRUSTYFILES:
				m_clientSoft = SO_TRUSTYFILES;
				pszSoftware = _T("TrustyFiles");
				break;
			// NEO: ECR END <-- Xanatos --
			default:
				if (m_bIsML || m_byCompatibleClient == SO_MLDONKEY || m_byCompatibleClient == SO_MLDONKEY2 || m_byCompatibleClient == SO_MLDONKEY3){ // NEO: ECR - [EnhancedClientRecognization] <-- Xanatos --
				//if (m_bIsML || m_byCompatibleClient == SO_MLDONKEY){
					m_clientSoft = SO_MLDONKEY;
					pszSoftware = _T("MLdonkey");
				}
				else if (m_bIsHybrid || m_byCompatibleClient == SO_EDONKEYHYBRID){ // NEO: ECR - [EnhancedClientRecognization] <-- Xanatos --
				//else if (m_bIsHybrid){
					m_clientSoft = SO_EDONKEYHYBRID;
					pszSoftware = _T("eDonkeyHybrid");
				}
				else if (m_byCompatibleClient != 0){
					// NEO: ECR - [EnhancedClientRecognization] -- Xanatos -->
					if (GetUserName() && StrStrI(GetUserName(),_T("shareaza")))
					{
						m_clientSoft = SO_SHAREAZA;
						pszSoftware = _T("Shareaza");
					}
					else if (StrStr(m_strModVersion,_T("Plus 1")))
					{
						m_clientSoft = SO_EMULEPLUS;
						pszSoftware = _T("eMule Plus");
					}
					else
					{
						m_clientSoft = SO_XMULE; // means: 'eMule Compatible'
						pszSoftware = _T("eMule Compat");
					}
					// NEO: ECR END <-- Xanatos --
					//m_clientSoft = SO_XMULE; // means: 'eMule Compatible'
					//pszSoftware = _T("eMule Compat");
				}
				else{
					m_clientSoft = SO_EMULE;
					pszSoftware = _T("eMule");
				}
		}

		int iLen;
		TCHAR szSoftware[128];
		if (m_byEmuleVersion == 0){
			m_nClientVersion = MAKE_CLIENT_VERSION(0, 0, 0);
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s"), pszSoftware);
		}
		else if (m_byEmuleVersion != 0x99){
			UINT nClientMinVersion = (m_byEmuleVersion >> 4)*10 + (m_byEmuleVersion & 0x0f);
			m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v0.%u"), pszSoftware, nClientMinVersion);
		}
		else{
			UINT nClientMajVersion = (m_nClientVersion >> 17) & 0x7f;
			UINT nClientMinVersion = (m_nClientVersion >> 10) & 0x7f;
			UINT nClientUpVersion  = (m_nClientVersion >>  7) & 0x07;
			m_nClientVersion = MAKE_CLIENT_VERSION(nClientMajVersion, nClientMinVersion, nClientUpVersion);
			if (m_clientSoft == SO_EMULE)
				iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u%c"), pszSoftware, nClientMajVersion, nClientMinVersion, _T('a') + nClientUpVersion);
			// NEO: ECR - [EnhancedClientRecognization] -- Xanatos -->
			else if (m_clientSoft == SO_EMULEPLUS)
			{
				if(nClientMinVersion == 0)
				{
					if(nClientUpVersion == 0)
						iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u"), pszSoftware, nClientMajVersion);
					else
						iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u%c"), pszSoftware, nClientMajVersion, _T('a') + nClientUpVersion - 1);
				}
				else
				{
					if(nClientUpVersion == 0)
						iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion);
					else
						iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u%c"), pszSoftware, nClientMajVersion, nClientMinVersion, _T('a') + nClientUpVersion - 1);
				}
			}
			// NEO: ECR END <-- Xanatos --
			else if (m_clientSoft == SO_AMULE || nClientUpVersion != 0)
				iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion, nClientUpVersion);
			else if (m_clientSoft == SO_LPHANT)
			{
				if (nClientMinVersion < 10)
				    iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.0%u"), pszSoftware, (nClientMajVersion-1), nClientMinVersion);
				else
					iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u"), pszSoftware, (nClientMajVersion-1), nClientMinVersion);
			}
			else
				iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion);
		}
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	if (m_bIsHybrid){
		m_clientSoft = SO_EDONKEYHYBRID;
		// seen:
		// 105010	0.50.10
		// 10501	0.50.1
		// 10300	1.3.0
		// 10201	1.2.1
		// 10103	1.1.3
		// 10102	1.1.2
		// 10100	1.1
		// 1051		0.51.0
		// 1002		1.0.2
		// 1000		1.0
		// 501		0.50.1

		UINT nClientMajVersion;
		UINT nClientMinVersion;
		UINT nClientUpVersion;
		if (m_nClientVersion > 100000){
			UINT uMaj = m_nClientVersion/100000;
			nClientMajVersion = uMaj - 1;
			nClientMinVersion = (m_nClientVersion - uMaj*100000) / 100;
			nClientUpVersion = m_nClientVersion % 100;
		}
		else if (m_nClientVersion >= 10100 && m_nClientVersion <= 10309){
			UINT uMaj = m_nClientVersion/10000;
			nClientMajVersion = uMaj;
			nClientMinVersion = (m_nClientVersion - uMaj*10000) / 100;
			nClientUpVersion = m_nClientVersion % 10;
		}
		else if (m_nClientVersion > 10000){
			UINT uMaj = m_nClientVersion/10000;
			nClientMajVersion = uMaj - 1;
			nClientMinVersion = (m_nClientVersion - uMaj*10000) / 10;
			nClientUpVersion = m_nClientVersion % 10;
		}
		else if (m_nClientVersion >= 1000 && m_nClientVersion < 1020){
			UINT uMaj = m_nClientVersion/1000;
			nClientMajVersion = uMaj;
			nClientMinVersion = (m_nClientVersion - uMaj*1000) / 10;
			nClientUpVersion = m_nClientVersion % 10;
		}
		else if (m_nClientVersion > 1000){
			UINT uMaj = m_nClientVersion/1000;
			nClientMajVersion = uMaj - 1;
			nClientMinVersion = m_nClientVersion - uMaj*1000;
			nClientUpVersion = 0;
		}
		else if (m_nClientVersion > 100){
			UINT uMin = m_nClientVersion/10;
			nClientMajVersion = 0;
			nClientMinVersion = uMin;
			nClientUpVersion = m_nClientVersion - uMin*10;
		}
		else{
			nClientMajVersion = 0;
			nClientMinVersion = m_nClientVersion;
			nClientUpVersion = 0;
		}
		m_nClientVersion = MAKE_CLIENT_VERSION(nClientMajVersion, nClientMinVersion, nClientUpVersion);

		int iLen;
		TCHAR szSoftware[128];
		if (nClientUpVersion)
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("eDonkeyHybrid v%u.%u.%u"), nClientMajVersion, nClientMinVersion, nClientUpVersion);
		else
			iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("eDonkeyHybrid v%u.%u"), nClientMajVersion, nClientMinVersion);
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	if (m_bIsML || iHashType == SO_MLDONKEY){
		m_clientSoft = SO_MLDONKEY;
		UINT nClientMinVersion = m_nClientVersion;
		m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
		TCHAR szSoftware[128];
		int iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("MLdonkey v0.%u"), nClientMinVersion);
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	if (iHashType == SO_OLDEMULE){
		m_clientSoft = SO_OLDEMULE;
		UINT nClientMinVersion = m_nClientVersion;
		m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
		TCHAR szSoftware[128];
		int iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("Old eMule v0.%u"), nClientMinVersion);
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	m_clientSoft = SO_EDONKEY;
	UINT nClientMinVersion = m_nClientVersion;
	m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
	TCHAR szSoftware[128];
	int iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("eDonkey v0.%u"), nClientMinVersion);
	if (iLen > 0){
		memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
		m_strClientSoftware.ReleaseBuffer(iLen);
	}
}

int CUpDownClient::GetHashType() const
{
	if (m_achUserHash[5] == 13 && m_achUserHash[14] == 110)
		return SO_OLDEMULE;
	else if (m_achUserHash[5] == 14 && m_achUserHash[14] == 111)
		return SO_EMULE;
 	else if (m_achUserHash[5] == 'M' && m_achUserHash[14] == 'L')
		return SO_MLDONKEY;
	else
		return SO_UNKNOWN;
}

void CUpDownClient::SetUserName(LPCTSTR pszNewName)
{
	free(m_pszUsername);
	if (pszNewName)
		m_pszUsername = _tcsdup(pszNewName);
	else
		m_pszUsername = NULL;
}

// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
bool CUpDownClient::RequestSharedFileList(bool bForce){
	if (!GetViewSharedFilesSupport())
		return false;
	if (!bForce && m_bFileListRequested)
		return false;
	if (m_bRequestingFileList) {
		LogWarning(LOG_STATUSBAR, GetResString(IDS_X_REQUESTING_SHARED_FILES), GetUserName(), GetUserIDHybrid());
		return false;
	}
	m_bRequestingFileList = true;
	m_bFileListRequested = true;
	AddLogLine(true, GetResString(IDS_SHAREDFILES_REQUEST), GetUserName());
	if (!TryToConnect(true))
		return false;
	m_Dirs2Update.RemoveAll();
	return true;
}

bool CUpDownClient::SendDirRequest(CString path, bool bForce)
{
	bool requested;
	if (!m_listDirs.Lookup(path, requested))
		return false;
	if (!bForce && requested)
		return false;

	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__AskSharedFilesInDirectory", this);
	CSafeMemFile tempfile(80);
	tempfile.WriteString(path, GetUnicodeSupport());
	Packet* replypacket = new Packet(&tempfile);
	replypacket->opcode = OP_ASKSHAREDFILESDIR;
	theStats.AddUpDataOverheadOther(replypacket->size);
	if (!socket || !socket->IsConnected())
		if (!TryToConnect(true)) {
			return false;
		}
	SetFileListRequested(GetFileListRequested()+1);
	socket->SendPacket(replypacket, true, true);
	m_listDirs.SetAt(path, true);
	return true;
}

void CUpDownClient::ProcessSharedFileList(const uchar* pachPacket, uint32 nSize, LPCTSTR pszDirectory)
{
	if (m_iFileListRequested > 0)
	{
		m_iFileListRequested--;
		CString dir;
		if (pszDirectory)
			dir = pszDirectory;
		else
			m_listDirs.SetAt(_T(""), true);

		for (POSITION pos = m_listFiles.GetHeadPosition(); pos != NULL; ) {
			POSITION pos1 = pos;
			CSearchFile *curfile = m_listFiles.GetNext(pos);
			if (curfile->GetDirectory() == dir) {
				delete curfile;
				m_listFiles.RemoveAt(pos1);
			}
		}

		uint32 nSearchID = (uint32)this;
		CSafeMemFile data(pachPacket, nSize);
		UINT results = data.ReadUInt32();

		for (UINT i = 0; i < results; i++){
			CSearchFile* toadd = new CSearchFile(&data, GetUnicodeSupport()!=utf8strNone, nSearchID, 0, 0, dir);
			toadd->SetClientID(GetIP());
			toadd->SetClientPort(GetUserPort());
			toadd->SetClientServerIP(GetServerIP());
			toadd->SetClientServerPort(GetServerPort());
			if (GetServerIP() && GetServerPort()){
				CSearchFile::SServer server(GetServerIP(), GetServerPort(), false);
				server.m_uAvail = 1;
				toadd->AddServer(server);
			}
			toadd->SetPreviewPossible( GetPreviewSupport() && ED2KFT_VIDEO == GetED2KFileTypeID(toadd->GetFileName()) );
			m_listFiles.AddTail(toadd);
		}
		data.Close();
		m_Dirs2Update.AddHead(dir);
	}
}

void CUpDownClient::ProcessSharedDirsList(const uchar* pachPacket, uint32 nSize)
{
	bool requested;
	if (m_iFileListRequested > 0)
	{
		m_iFileListRequested--;
		CSafeMemFile data(pachPacket, nSize);
		UINT uDirs = data.ReadUInt32();
		if (uDirs > 0)
			m_listDirs.RemoveAll();
		for (UINT i = 0; i < uDirs; i++)
		{
			CString strDir = data.ReadString(GetUnicodeSupport()!=utf8strNone);
			AddDebugLogLine(true, GetResString(IDS_SHAREDANSW), GetUserName(), GetUserIDHybrid(), strDir);
			if (!m_listDirs.Lookup(strDir, requested))
				m_listDirs.SetAt(strDir, false);
		}
		ASSERT( data.GetPosition() == data.GetLength() );
		for (POSITION pos = m_listFiles.GetHeadPosition(); pos != NULL; ) {
			POSITION pos1 = pos;
			CSearchFile *curfile = m_listFiles.GetNext(pos);
			if (!m_listDirs.Lookup(curfile->GetDirectory(), requested)) {
				delete curfile;
				m_listFiles.RemoveAt(pos1);
			}
		}
		m_Dirs2Update.AddHead(_T(""));
	}
	else
		AddLogLine(true, GetResString(IDS_SHAREDANSW2), GetUserName(), GetUserIDHybrid());
}

void CUpDownClient::SetDeniesShare(bool in, bool updateTree)
{
	m_bDeniesShare = in;
	if (in) {
		m_listDirs.RemoveAll();
		while (!m_listFiles.IsEmpty())
			delete m_listFiles.RemoveHead();
	}
	if (updateTree) 
		m_Dirs2Update.AddHead(_T(""));
}
// NEO: XSF END <-- Xanatos --

/*void CUpDownClient::RequestSharedFileList()
{
	if (m_iFileListRequested == 0){
		AddLogLine(true, GetResString(IDS_SHAREDFILES_REQUEST), GetUserName());
    	m_iFileListRequested = 1;
		TryToConnect(true);
	}
	else{
		LogWarning(LOG_STATUSBAR, _T("Requesting shared files from user %s (%u) is already in progress"), GetUserName(), GetUserIDHybrid());
	}
}

void CUpDownClient::ProcessSharedFileList(const uchar* pachPacket, uint32 nSize, LPCTSTR pszDirectory)
{
	if (m_iFileListRequested > 0)
	{
        m_iFileListRequested--;
		theApp.searchlist->ProcessSearchAnswer(pachPacket,nSize,this,NULL,pszDirectory);
	}
}*/

void CUpDownClient::SetUserHash(const uchar* pucUserHash)
{
	if( pucUserHash == NULL ){
		md4clr(m_achUserHash);
		return;
	}
	md4cpy(m_achUserHash, pucUserHash);
}

void CUpDownClient::SetBuddyID(const uchar* pucBuddyID)
{
	if( pucBuddyID == NULL ){
		md4clr(m_achBuddyID);
		m_bBuddyIDValid = false;
		return;
	}
	m_bBuddyIDValid = true;
	md4cpy(m_achBuddyID, pucBuddyID);
}

void CUpDownClient::SendPublicKeyPacket()
{
	// send our public key to the client who requested it
	if (socket == NULL || credits == NULL || m_SecureIdentState != IS_KEYANDSIGNEEDED){
		ASSERT ( false );
		return;
	}
	if (!theApp.clientcredits->CryptoAvailable())
		return;

    Packet* packet = new Packet(OP_PUBLICKEY,theApp.clientcredits->GetPubKeyLen() + 1,OP_EMULEPROT);
	theStats.AddUpDataOverheadOther(packet->size);
	memcpy(packet->pBuffer+1,theApp.clientcredits->GetPublicKey(), theApp.clientcredits->GetPubKeyLen());
	packet->pBuffer[0] = theApp.clientcredits->GetPubKeyLen();
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__PublicKey", this);
	socket->SendPacket(packet,true,true);
	m_SecureIdentState = IS_SIGNATURENEEDED;
}

void CUpDownClient::SendSignaturePacket()
{
	// signate the public key of this client and send it
	if (socket == NULL || credits == NULL || m_SecureIdentState == 0){
		ASSERT ( false );
		return;
	}

	if (!theApp.clientcredits->CryptoAvailable())
		return;
	if (credits->GetSecIDKeyLen() == 0)
		return; // We don't have his public key yet, will be back here later
	// do we have a challenge value received (actually we should if we are in this function)
	if (credits->m_dwCryptRndChallengeFrom == 0){
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Want to send signature but challenge value is invalid ('%s')"), GetUserName());
		return;
	}
	// v2
	// we will use v1 as default, except if only v2 is supported
	bool bUseV2;
	if ( (m_bySupportSecIdent&1) == 1 )
		bUseV2 = false;
	else
		bUseV2 = true;

	uint8 byChaIPKind = 0;
	uint32 ChallengeIP = 0;
	if (bUseV2){
		if (theApp.serverconnect->GetClientID() == 0 || theApp.serverconnect->IsLowID()){
			// we cannot do not know for sure our public ip, so use the remote clients one
			ChallengeIP = GetIP();
			byChaIPKind = CRYPT_CIP_REMOTECLIENT;
		}
		else{
			ChallengeIP = theApp.serverconnect->GetClientID();
			byChaIPKind  = CRYPT_CIP_LOCALCLIENT;
		}
	}
	//end v2
	uchar achBuffer[250];
	uint8 siglen = theApp.clientcredits->CreateSignature(credits, achBuffer,  250, ChallengeIP, byChaIPKind );
	if (siglen == 0){
		ASSERT ( false );
		return;
	}
	Packet* packet = new Packet(OP_SIGNATURE,siglen + 1+ ( (bUseV2)? 1:0 ),OP_EMULEPROT);
	theStats.AddUpDataOverheadOther(packet->size);
	memcpy(packet->pBuffer+1,achBuffer, siglen);
	packet->pBuffer[0] = siglen;
	if (bUseV2)
		packet->pBuffer[1+siglen] = byChaIPKind;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__Signature", this);
	socket->SendPacket(packet,true,true);
	m_SecureIdentState = IS_ALLREQUESTSSEND;
}

void CUpDownClient::ProcessPublicKeyPacket(const uchar* pachPacket, uint32 nSize)
{
	theApp.clientlist->AddTrackClient(this);

	if (socket == NULL || credits == NULL || pachPacket[0] != nSize-1
		|| nSize == 0 || nSize > 250){
		ASSERT ( false );
		return;
	}
	if (!theApp.clientcredits->CryptoAvailable())
		return;
	// the function will handle everything (mulitple key etc)
	if (credits->SetSecureIdent(pachPacket+1, pachPacket[0])){
		// if this client wants a signature, now we can send him one
		if (m_SecureIdentState == IS_SIGNATURENEEDED){
			SendSignaturePacket();
		}
		else if(m_SecureIdentState == IS_KEYANDSIGNEEDED)
		{
			// something is wrong
			if (thePrefs.GetLogSecureIdent())
				AddDebugLogLine(false, _T("Invalid State error: IS_KEYANDSIGNEEDED in ProcessPublicKeyPacket"));
		}
	}
	else
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Failed to use new received public key"));
	}
}

void CUpDownClient::ProcessSignaturePacket(const uchar* pachPacket, uint32 nSize)
{
	// here we spread the good guys from the bad ones ;)

	if (socket == NULL || credits == NULL || nSize == 0 || nSize > 250){
		ASSERT ( false );
		return;
	}

	uint8 byChaIPKind;
	if (pachPacket[0] == nSize-1)
		byChaIPKind = 0;
	else if (pachPacket[0] == nSize-2 && (m_bySupportSecIdent & 2) > 0) //v2
		byChaIPKind = pachPacket[nSize-1];
	else{
		ASSERT ( false );
		return;
	}

	if (!theApp.clientcredits->CryptoAvailable())
		return;
	
	// we accept only one signature per IP, to avoid floods which need a lot cpu time for cryptfunctions
	if (m_dwLastSignatureIP == GetIP())
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("received multiple signatures from one client"));
		return;
	}
	
	// also make sure this client has a public key
	if (credits->GetSecIDKeyLen() == 0)
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("received signature for client without public key"));
		return;
	}
	
	// and one more check: did we ask for a signature and sent a challange packet?
	if (credits->m_dwCryptRndChallengeFor == 0)
	{
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("received signature for client with invalid challenge value ('%s')"), GetUserName());
		return;
	}

	if (theApp.clientcredits->VerifyIdent(credits, pachPacket+1, pachPacket[0], GetIP(), byChaIPKind ) ){
		// result is saved in function abouve
		//if (thePrefs.GetLogSecureIdent())
		//	AddDebugLogLine(false, _T("'%s' has passed the secure identification, V2 State: %i"), GetUserName(), byChaIPKind);

		// inform our friendobject if needed
		if (GetFriend() != NULL && GetFriend()->IsTryingToConnect())
			GetFriend()->UpdateFriendConnectionState(FCR_USERHASHVERIFIED);

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
		if(source)
			source->SUIPassed(this);
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
	}
	else
	{
		if (GetFriend() != NULL && GetFriend()->IsTryingToConnect())
			GetFriend()->UpdateFriendConnectionState(FCR_SECUREIDENTFAILED);
		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("'%s' has failed the secure identification, V2 State: %i"), GetUserName(), byChaIPKind);
	}
	m_dwLastSignatureIP = GetIP(); 
}

void CUpDownClient::SendSecIdentStatePacket()
{
	// check if we need public key and signature
	uint8 nValue = 0;
	if (credits){
		if (theApp.clientcredits->CryptoAvailable()){
			if (credits->GetSecIDKeyLen() == 0)
				nValue = IS_KEYANDSIGNEEDED;
			else if (m_dwLastSignatureIP != GetIP())
				nValue = IS_SIGNATURENEEDED;
		}
		if (nValue == 0){
			//if (thePrefs.GetLogSecureIdent())
			//	AddDebugLogLine(false, _T("Not sending SecIdentState Packet, because State is Zero"));
			return;
		}
		// crypt: send random data to sign
		uint32 dwRandom = rand()+1;
		credits->m_dwCryptRndChallengeFor = dwRandom;
		Packet* packet = new Packet(OP_SECIDENTSTATE,5,OP_EMULEPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		packet->pBuffer[0] = nValue;
		PokeUInt32(packet->pBuffer+1, dwRandom);
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__SecIdentState", this);
		socket->SendPacket(packet,true,true);
	}
	else
		ASSERT ( false );
}

void CUpDownClient::ProcessSecIdentStatePacket(const uchar* pachPacket, uint32 nSize)
{
	if (nSize != 5)
		return;
	if (!credits){
		ASSERT ( false );
		return;
	}
	switch(pachPacket[0]){
		case 0:
			m_SecureIdentState = IS_UNAVAILABLE;
			break;
		case 1:
			m_SecureIdentState = IS_SIGNATURENEEDED;
			break;
		case 2:
			m_SecureIdentState = IS_KEYANDSIGNEEDED;
			break;
	}
	credits->m_dwCryptRndChallengeFrom = PeekUInt32(pachPacket+1);
}

void CUpDownClient::InfoPacketsReceived()
{
	// indicates that both Information Packets has been received
	// needed for actions, which process data from both packets
	ASSERT ( m_byInfopacketsReceived == IP_BOTH );
	m_byInfopacketsReceived = IP_NONE;
	
	if (m_bySupportSecIdent){
		SendSecIdentStatePacket();
	}
}

void CUpDownClient::ResetFileStatusInfo()
{
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	//delete[] m_abyPartStatus;
	//m_abyPartStatus = NULL;
	m_nRemoteQueueRank = 0;
	m_nRemoteQueueRankOld = 0; // NEO: CQR - [CollorQueueRank] <-- Xanatos --
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	//m_nPartCount = 0;
	//m_strClientFilename.Empty();
	//m_bCompleteSource = false;
	//m_uFileRating = 0;
	//m_strFileComment.Empty();
	delete m_pReqFileAICHHash;
	m_pReqFileAICHHash = NULL;
}

bool CUpDownClient::IsBanned() const
{
	//return theApp.clientlist->IsBannedClient(GetIP());
	return theApp.clientlist->IsBannedClient(GetConnectIP()); // NEO: FIX - [IsBanned] <-- Xanatos --
}

void CUpDownClient::SendPreviewRequest(const CAbstractFile* pForFile)
{
	if (m_fPreviewReqPending == 0){
		m_fPreviewReqPending = 1;
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__RequestPreview", this, pForFile->GetFileHash());
		Packet* packet = new Packet(OP_REQUESTPREVIEW,16,OP_EMULEPROT);
		md4cpy(packet->pBuffer,pForFile->GetFileHash());
		theStats.AddUpDataOverheadOther(packet->size);
		SafeSendPacket(packet);
	}
	else{
		LogWarning(LOG_STATUSBAR, GetResString(IDS_ERR_PREVIEWALREADY));
	}
}

void CUpDownClient::SendPreviewAnswer(const CKnownFile* pForFile, CxImage** imgFrames, uint8 nCount)
{
	m_fPreviewAnsPending = 0;
	CSafeMemFile data(1024);
	if (pForFile){
		data.WriteHash16(pForFile->GetFileHash());
	}
	else{
		static const uchar _aucZeroHash[16] = {0};
		data.WriteHash16(_aucZeroHash);
	}
	data.WriteUInt8(nCount);
	for (int i = 0; i != nCount; i++){
		if (imgFrames == NULL){
			ASSERT ( false );
			return;
		}
		CxImage* cur_frame = imgFrames[i];
		if (cur_frame == NULL){
			ASSERT ( false );
			return;
		}
		BYTE* abyResultBuffer = NULL;
		long nResultSize = 0;
		if (!cur_frame->Encode(abyResultBuffer, nResultSize, CXIMAGE_FORMAT_PNG)){
			ASSERT ( false );			
			return;
		}
		data.WriteUInt32(nResultSize);
		data.Write(abyResultBuffer, nResultSize);
		free(abyResultBuffer);
	}
	Packet* packet = new Packet(&data, OP_EMULEPROT);
	packet->opcode = OP_PREVIEWANSWER;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__PreviewAnswer", this, (uchar*)packet->pBuffer);
	theStats.AddUpDataOverheadOther(packet->size);
	SafeSendPacket(packet);
}

void CUpDownClient::ProcessPreviewReq(const uchar* pachPacket, uint32 nSize)
{
	if (nSize < 16)
		throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
	
	if(!NeoPrefs.UseShowSharePermissions()) // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	if (m_fPreviewAnsPending || thePrefs.CanSeeShares()==vsfaNobody || (thePrefs.CanSeeShares()==vsfaFriends && !IsFriend()))
		return;
	
	m_fPreviewAnsPending = 1;
	CKnownFile* previewFile = theApp.sharedfiles->GetFileByID(pachPacket);
	if (previewFile == NULL){
		SendPreviewAnswer(NULL, NULL, 0);
	}
	else{
		// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
		int Perm = ((previewFile->GetPermissions() == PERM_DEFAULT) ? thePrefs.CanSeeShares() : previewFile->GetPermissions());
		if (NeoPrefs.UseShowSharePermissions() && (Perm == PERM_NONE || (Perm == PERM_FRIENDS && !IsFriend())))
			return;
		// NEO: SSP END <-- Xanatos --
		previewFile->GrabImage(4,0,true,450,this);
	}
}

void CUpDownClient::ProcessPreviewAnswer(const uchar* pachPacket, uint32 nSize)
{
	if (m_fPreviewReqPending == 0)
		return;
	m_fPreviewReqPending = 0;
	CSafeMemFile data(pachPacket, nSize);
	uchar Hash[16];
	data.ReadHash16(Hash);
	uint8 nCount = data.ReadUInt8();
	if (nCount == 0){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_PREVIEWFAILED), GetUserName());
		return;
	}
	CSearchFile* sfile = theApp.searchlist->GetSearchFileByHash(Hash);
	// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->
	if (sfile == NULL && !m_listFiles.IsEmpty())
		for (POSITION pos = m_listFiles.GetHeadPosition(); pos != NULL; ) {
			CSearchFile *item = m_listFiles.GetNext(pos);
			if (!md4cmp(Hash, item->GetFileHash())) {
				sfile = item;
				break;
			}
		}
	// NEO: XSF END <-- Xanatos --
	if (sfile == NULL){
		//already deleted
		return;
	}

	BYTE* pBuffer = NULL;
	try{
		for (int i = 0; i != nCount; i++){
			uint32 nImgSize = data.ReadUInt32();
			if (nImgSize > nSize)
				throw CString(_T("CUpDownClient::ProcessPreviewAnswer - Provided image size exceeds limit"));
			pBuffer = new BYTE[nImgSize];
			data.Read(pBuffer, nImgSize);
			CxImage* image = new CxImage(pBuffer, nImgSize, CXIMAGE_FORMAT_PNG);
			delete[] pBuffer;
			pBuffer = NULL;
			if (image->IsValid()){
				sfile->AddPreviewImg(image);
			}
		}
	}
	catch(...){
		delete[] pBuffer;
		throw;
	}
	(new PreviewDlg())->SetFile(sfile);
}

// sends a packet, if needed it will establish a connection before
// options used: ignore max connections, control packet, delete packet
// !if the functions returns false that client object was deleted because the connection try failed and the object wasn't needed anymore.
bool CUpDownClient::SafeSendPacket(Packet* packet){
	if (socket && socket->IsConnected()){
		socket->SendPacket(packet, true);
		return true;
	}
	else{
		m_WaitingPackets_list.AddTail(packet);
		return TryToConnect(true);
	}
}

#ifdef _DEBUG
void CUpDownClient::AssertValid() const
{
	CObject::AssertValid();

	CHECK_OBJ(socket);
	CHECK_PTR(credits);
	CHECK_PTR(m_Friend);
	CHECK_OBJ(reqfile);
	//(void)m_abyUpPartStatus; // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
	m_OtherRequests_list.AssertValid();
	m_OtherNoNeeded_list.AssertValid();
	(void)m_lastPartAsked;
	(void)m_cMessagesReceived;
	(void)m_cMessagesSent;
	(void)m_dwUserIP;
	(void)m_dwServerIP;
	(void)m_nUserIDHybrid;
	(void)m_nUserPort;
	(void)m_nServerPort;
	(void)m_nClientVersion;
	(void)m_nUpDatarate;
	(void)m_byEmuleVersion;
	(void)m_byDataCompVer;
	CHECK_BOOL(m_bEmuleProtocol);
	CHECK_BOOL(m_bIsHybrid);
	(void)m_pszUsername;
	(void)m_achUserHash;
	(void)m_achBuddyID;
	(void)m_nBuddyIP;
	(void)m_nBuddyPort;
	(void)m_nUDPPort;
	(void)m_nKadPort;
	(void)m_byUDPVer;
	(void)m_bySourceExchange1Ver;
	(void)m_byAcceptCommentVer;
	(void)m_byExtendedRequestsVer;
	CHECK_BOOL(m_bFriendSlot);
	CHECK_BOOL(m_bCommentDirty);
	CHECK_BOOL(m_bIsML);
	//ASSERT( m_clientSoft >= SO_EMULE && m_clientSoft <= SO_SHAREAZA || m_clientSoft == SO_MLDONKEY || m_clientSoft >= SO_EDONKEYHYBRID && m_clientSoft <= SO_UNKNOWN );
	(void)m_strClientSoftware;
	(void)m_dwLastSourceRequest;
	(void)m_dwLastSourceAnswer;
	(void)m_dwLastAskedForSources;
    (void)m_iFileListRequested;
	(void)m_byCompatibleClient;
	m_WaitingPackets_list.AssertValid();
	m_DontSwap_list.AssertValid();
	(void)m_lastRefreshedDLDisplay;
	ASSERT( m_SecureIdentState >= IS_UNAVAILABLE && m_SecureIdentState <= IS_KEYANDSIGNEEDED );
	(void)m_dwLastSignatureIP;
	ASSERT( (m_byInfopacketsReceived & ~IP_BOTH) == 0 );
	(void)m_bySupportSecIdent;
	(void)m_nTransferredUp;
	ASSERT( m_nUploadState >= US_UPLOADING && m_nUploadState <= US_NONE );
	(void)m_dwUploadTime;
	(void)m_cAsked;
	(void)m_dwLastUpRequest;
	(void)m_nCurSessionUp;
    (void)m_nCurQueueSessionPayloadUp;
    (void)m_addedPayloadQueueSession;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	//(void)m_nUpPartCount;
	//(void)m_nUpCompleteSourcesCount;
	(void)s_UpStatusBar;
	(void)requpfileid;
    (void)m_lastRefreshedULDisplay;
	m_AvarageUDR_list.AssertValid();
	m_BlockRequests_queue.AssertValid();
	m_DoneBlocks_list.AssertValid();
	m_RequestedFiles_list.AssertValid();
	ASSERT( m_nDownloadState >= DS_DOWNLOADING && m_nDownloadState <= DS_NONE );
	(void)m_cDownAsked;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	//(void)m_abyPartStatus;
	//(void)m_strClientFilename;
	(void)m_nTransferredDown;
    (void)m_nCurSessionPayloadDown;
	(void)m_dwDownStartTime;
	(void)m_nLastBlockOffset;
	(void)m_nDownDatarate;
	//(void)m_nDownDataRateMS; // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	(void)m_nSumForAvgDownDataRate;
	//(void)m_cShowDR; // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
	(void)m_nRemoteQueueRank;
	(void)m_nRemoteQueueRankOld; // NEO: CQR - [CollorQueueRank] <-- Xanatos --
	(void)m_dwLastBlockReceived;
	//(void)m_nPartCount; // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
	ASSERT( m_nSourceFrom >= SF_SERVER && m_nSourceFrom <= SF_LINK );
	//CHECK_BOOL(m_bRemoteQueueFull); // NEO: FIX - [SourceCount] <-- Xanatos --
	//CHECK_BOOL(m_bCompleteSource); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
	CHECK_BOOL(m_bReaskPending);
	CHECK_BOOL(m_bUDPPending);
	CHECK_BOOL(m_bTransferredDownMini);
	CHECK_BOOL(m_bUnicodeSupport);
	ASSERT( m_nKadState >= KS_NONE && m_nKadState <= KS_CONNECTING_FWCHECK_UDP);
	m_AvarageDDR_list.AssertValid();
	(void)m_nSumForAvgUpDataRate;
	m_PendingBlocks_list.AssertValid();
	//m_DownloadBlocks_list.AssertValid(); // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
	(void)s_StatusBar;
	ASSERT( m_nChatstate >= MS_NONE && m_nChatstate <= MS_UNABLETOCONNECT );
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	//(void)m_strFileComment;
	//(void)m_uFileRating;
	CHECK_BOOL(m_bCollectionUploadSlot);
	// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
	(void)m_bNotifyIdChage;
	(void)m_bWainingNotifyIdChage;
	// NEO: RIC END <-- Xanatos --
#undef CHECK_PTR
#undef CHECK_BOOL
}
#endif

#ifdef _DEBUG
void CUpDownClient::Dump(CDumpContext& dc) const
{
	CObject::Dump(dc);
}
#endif

LPCTSTR CUpDownClient::DbgGetDownloadState() const
{
	const static LPCTSTR apszState[] =
	{
		_T("Downloading"),
		_T("OnQueue"),
		_T("Connected"),
		_T("Connecting"),
		_T("WaitCallback"),
		_T("WaitCallbackKad"),
		_T("ReqHashSet"),
		_T("NoNeededParts"),
		_T("TooManyConns"),
		_T("TooManyConnsKad"),
		_T("LowToLowIp"),
		_T("Banned"),
		_T("Error"),
		_T("None"),
		_T("RemoteQueueFull")
	};
	if (GetDownloadState() >= ARRSIZE(apszState))
		return _T("*Unknown*");
	return apszState[GetDownloadState()];
}

LPCTSTR CUpDownClient::DbgGetUploadState() const
{
	const static LPCTSTR apszState[] =
	{
		_T("Uploading"),
		_T("OnUploadQueue"),
		_T("WaitCallback"),
		_T("Connecting"),
		_T("Pending"),
		_T("LowToLowIp"),
		_T("Banned"),
		_T("Error"),
		_T("None")
	};
	if (GetUploadState() >= ARRSIZE(apszState))
		return _T("*Unknown*");
	return apszState[GetUploadState()];
}

LPCTSTR CUpDownClient::DbgGetKadState() const
{
	const static LPCTSTR apszState[] =
	{
		_T("None"),
		_T("FwCheckQueued"),
		_T("FwCheckConnecting"),
		_T("FwCheckConnected"),
		_T("BuddyQueued"),
		_T("BuddyIncoming"),
		_T("BuddyConnecting"),
		_T("BuddyConnected"),
		_T("QueuedFWCheckUDP"),
		_T("FWCheckUDP"),
		_T("FwCheckConnectingUDP")
	};
	if (GetKadState() >= ARRSIZE(apszState))
		return _T("*Unknown*");
	return apszState[GetKadState()];
}

CString CUpDownClient::DbgGetFullClientSoftVer() const
{
	if (GetClientModVer().IsEmpty())
		return GetClientSoftVer();
	return GetClientSoftVer() + _T(" [") + GetClientModVer() + _T(']');
}

CString CUpDownClient::DbgGetClientInfo(bool bFormatIP) const
{
	CString str;
	if (this != NULL)
	{
		try{
			if (HasLowID())
			{
				if (GetConnectIP())
				{
					str.Format(_T("%u@%s (%s) '%s' (%s,%s/%s/%s)"),
						GetUserIDHybrid(), ipstr(GetServerIP()),
						ipstr(GetConnectIP()),
						GetUserName(),
						DbgGetFullClientSoftVer(),
						DbgGetDownloadState(), DbgGetUploadState(), DbgGetKadState());
				}
				else
				{
					str.Format(_T("%u@%s '%s' (%s,%s/%s/%s)"),
						GetUserIDHybrid(), ipstr(GetServerIP()),
						GetUserName(),
						DbgGetFullClientSoftVer(),
						DbgGetDownloadState(), DbgGetUploadState(), DbgGetKadState());
				}
			}
			else
			{
				str.Format(bFormatIP ? _T("%-15s '%s' (%s,%s/%s/%s)") : _T("%s '%s' (%s,%s/%s/%s)"),
					ipstr(GetConnectIP()),
					GetUserName(),
					DbgGetFullClientSoftVer(),
					DbgGetDownloadState(), DbgGetUploadState(), DbgGetKadState());
			}
		}
		catch(...){
			str.Format(_T("%08x - Invalid client instance"), this);
		}
	}
	return str;
}

bool CUpDownClient::CheckHandshakeFinished() const
{
	//if (m_bHelloAnswerPending)
	if (m_byHelloPacketState != HP_BOTH) // NEO: FCC - [FixConnectionCollision] <-- Xanatos --
	{
		// 24-Nov-2004 [bc]: The reason for this is that 2 clients are connecting to each other at the same..
		//if (thePrefs.GetVerbose())
		//	AddDebugLogLine(DLP_VERYLOW, false, _T("Handshake not finished - while processing packet: %s; %s"), DbgGetClientTCPOpcode(protocol, opcode), DbgGetClientInfo());
		return false;
	}

	return true;
}

void CUpDownClient::CheckForGPLEvilDoer()
{
	if (!m_strModVersion.IsEmpty()){
		LPCTSTR pszModVersion = (LPCTSTR)m_strModVersion;

		// skip leading spaces
		while (*pszModVersion == _T(' '))
			pszModVersion++;

		// check for known major gpl breaker
		if (_tcsnicmp(pszModVersion, _T("LH"), 2)==0 || _tcsnicmp(pszModVersion, _T("LIO"), 3)==0 || _tcsnicmp(pszModVersion, _T("PLUS PLUS"), 9)==0)
			m_bGPLEvildoer = true;
	}
}

void CUpDownClient::OnSocketConnected(int /*nErrorCode*/)
{
}

CString CUpDownClient::GetDownloadStateDisplayString() const
{
	CString strState;
	switch (GetDownloadState())
	{
		case DS_CONNECTING:
			strState = GetResString(IDS_CONNECTING);
			break;
		// NEO: SD - [StandByDL] -- Xanatos -->
		case DS_HALTED:
			strState = GetResString(IDS_X_STANDBY);
			break;
		// NEO: SD END <-- Xanatos --
		case DS_CONNECTED:
			strState = GetResString(IDS_ASKING);
			break;
		// NEO: TCR - [TCPConnectionRetry] -- Xanatos -->
		case DS_CONNECTIONRETRY:
			strState = GetResString(IDS_X_CONNECTION_RETRY);
			break;
		// NEO: TCR END <-- Xanatos --
		// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
		case DS_CACHED:
			strState = GetResString(IDS_X_CACHED);
			break;
		// NEO: XSC END <-- Xanatos --
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
		case DS_LOADED:
			strState = GetResString(IDS_X_LOADED);
			break;
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
		case DS_WAITCALLBACK:
			strState = GetResString(IDS_CONNVIASERVER);
			break;
		//case DS_ONQUEUE:
		//	if (IsRemoteQueueFull())
		//		strState = GetResString(IDS_QUEUEFULL);
		//	else
		//		strState = GetResString(IDS_ONQUEUE);
		//	break;
		// NEO: FIX - [SourceCount] -- Xanatos -->
		case DS_ONQUEUE:
			strState = GetResString(IDS_ONQUEUE);
			break;
		case DS_REMOTEQUEUEFULL:
			strState = GetResString(IDS_QUEUEFULL);
			break;
		// NEO: FIX END <-- Xanatos --
		case DS_DOWNLOADING:
			strState = GetResString(IDS_TRANSFERRING);
			break;
		case DS_REQHASHSET:
			strState = GetResString(IDS_RECHASHSET);
			break;
		case DS_NONEEDEDPARTS:
			strState = GetResString(IDS_NONEEDEDPARTS);
			break;
		case DS_LOWTOLOWIP:
			strState = GetResString(IDS_NOCONNECTLOW2LOW);
			break;
		case DS_TOOMANYCONNS:
			strState = GetResString(IDS_TOOMANYCONNS);
			break;
		case DS_ERROR:
			strState = GetResString(IDS_ERROR);
			break;
		case DS_WAITCALLBACKKAD:
			strState = GetResString(IDS_KAD_WAITCBK);
			break;
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
		case DS_WAITCALLBACKXS:
			strState = GetResString(IDS_X_XS_WAITCBK);
			break;
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
		case DS_TOOMANYCONNSKAD:
			strState = GetResString(IDS_KAD_TOOMANDYKADLKPS);
			break;
	}

	if (thePrefs.GetPeerCacheShow())
	{
		switch (m_ePeerCacheDownState)
		{
		case PCDS_WAIT_CLIENT_REPLY:
			strState += _T(" ")+GetResString(IDS_PCDS_CLIENTWAIT);
			break;
		case PCDS_WAIT_CACHE_REPLY:
			strState += _T(" ")+GetResString(IDS_PCDS_CACHEWAIT);
			break;
		case PCDS_DOWNLOADING:
			strState += _T(" ")+GetResString(IDS_CACHE);
			break;
		}
		if (m_ePeerCacheDownState != PCDS_NONE && m_bPeerCacheDownHit)
			strState += _T(" Hit");
	}

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	if(IsLanClient())
		strState += GetResString(IDS_X_LANCAST_ADDON);
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	return strState;
}

CString CUpDownClient::GetUploadStateDisplayString() const
{
	CString strState;
	switch (GetUploadState()){
		case US_ONUPLOADQUEUE:
			strState = GetResString(IDS_ONQUEUE);
			break;
		// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
		case US_NONEEDEDPARTS:
			strState = GetResString(IDS_NONEEDEDPARTS);
			break;
		// NEO: SCT END <-- Xanatos --
		case US_PENDING:
			strState = GetResString(IDS_CL_PENDING);
			break;
		case US_LOWTOLOWIP:
			strState = GetResString(IDS_CL_LOW2LOW);
			break;
		case US_BANNED:
			strState = GetResString(IDS_BANNED);
			break;
		case US_ERROR:
			strState = GetResString(IDS_ERROR);
			break;
		case US_CONNECTING:
			strState = GetResString(IDS_CONNECTING);
			break;
		case US_WAITCALLBACK:
			strState = GetResString(IDS_CONNVIASERVER);
			break;
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
		case US_WAITCALLBACKKAD:
			strState = GetResString(IDS_KAD_WAITCBK);
			break;
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
		case US_WAITCALLBACKXS:
			strState = GetResString(IDS_X_XS_WAITCBK);
			break;
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
		case US_UPLOADING:
            if(GetPayloadInBuffer() == 0 && GetNumberOfRequestedBlocksInQueue() == 0 && thePrefs.IsExtControlsEnabled()) {
				strState = GetResString(IDS_US_STALLEDW4BR);
            } else if(GetPayloadInBuffer() == 0 && thePrefs.IsExtControlsEnabled()) {
				strState = GetResString(IDS_US_STALLEDREADINGFDISK);
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
            } else if (!HaveTrickleSlot()){
#else
            } else if(GetSlotNumber() <= theApp.uploadqueue->GetActiveUploadsCount()) {
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
				strState = GetResString(IDS_TRANSFERRING);
            } else {
                strState = GetResString(IDS_TRICKLING);
            }
 #ifdef BW_MOD// NEO: BM - [BandwidthModeration]
			if(socket && socket->m_eSlotType == ST_RELEASE)
				strState += GetResString(IDS_X_RELEASE_ADDON);
			else if(socket && socket->m_eSlotType == ST_FRIEND)
				strState += GetResString(IDS_X_FRIEND_ADDON);
 #endif // BW_MOD // NEO: BM END
			break;
	}

	if (thePrefs.GetPeerCacheShow())
	{
		switch (m_ePeerCacheUpState)
		{
		case PCUS_WAIT_CACHE_REPLY:
			strState += _T(" CacheWait");
			break;
		case PCUS_UPLOADING:
			strState += _T(" Cache");
			break;
		}
		if (m_ePeerCacheUpState != PCUS_NONE && m_bPeerCacheUpHit)
			strState += _T(" Hit");
	}

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	if(IsLanClient())
		strState += GetResString(IDS_X_LANCAST_ADDON);
#endif //LANCAST // NEO: NLC END <-- Xanatos --

	return strState;
}

void CUpDownClient::SendPublicIPRequest(){
	if (socket && socket->IsConnected()){
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__PublicIPReq", this);
		Packet* packet = new Packet(OP_PUBLICIP_REQ,0,OP_EMULEPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true);
		m_fNeedOurPublicIP = 1;
	}
}

void CUpDownClient::ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize){
	if (uSize != 4)
		throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
	uint32 dwIP = PeekUInt32(pbyData);
	if (m_fNeedOurPublicIP == 1){ // did we?
		m_fNeedOurPublicIP = 0;
		if (theApp.GetPublicIP() == 0 && !::IsLowID(dwIP) ){
			theApp.SetPublicIP(dwIP);
			// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
			if(NeoPrefs.IsCheckIPChange())
				theApp.CheckIDChange(dwIP);
			// NEO: RIC END <-- Xanatos --
		}
	}	
}

void CUpDownClient::CheckFailedFileIdReqs(const uchar* aucFileHash)
{
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	if(NeoPrefs.IsFileScannerDetection() == false)
		return;
#endif // ARGOS // NEO: NA END <-- Xanatos --
	if ( aucFileHash != NULL && (theApp.sharedfiles->IsUnsharedFile(aucFileHash) || theApp.downloadqueue->GetFileByID(aucFileHash)) )
		return;
	//if (GetDownloadState() != DS_DOWNLOADING) // filereq floods are never allowed!
	{
		if (m_fFailedFileIdReqs < 6)// NOTE: Do not increase this nr. without increasing the bits for 'm_fFailedFileIdReqs'
			m_fFailedFileIdReqs++;
		if (m_fFailedFileIdReqs == 6)
		{
			if (theApp.clientlist->GetBadRequests(this) < 2)
				theApp.clientlist->TrackBadRequest(this, 1);
			if (theApp.clientlist->GetBadRequests(this) == 2){
				theApp.clientlist->TrackBadRequest(this, -2); // reset so the client will not be rebanned right after the ban is lifted
				Ban(_T("FileReq flood"));
			}
			throw CString(thePrefs.GetLogBannedClients() ? _T("FileReq flood") : _T(""));
		}
	}
}

EUtf8Str CUpDownClient::GetUnicodeSupport() const
{
	if (m_bUnicodeSupport)
		return utf8strRaw;
	return utf8strNone;
}

void CUpDownClient::SetSpammer(bool bVal){ 
	if (bVal)
		Ban(_T("Identified as Spammer"));
	else if (IsBanned() && m_fIsSpammer)
		UnBan();
	m_fIsSpammer = bVal ? 1 : 0;
}

void  CUpDownClient::SetMessageFiltered(bool bVal)	{
	m_fMessageFiltered = bVal ? 1 : 0;
}

bool  CUpDownClient::IsObfuscatedConnectionEstablished() const {
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	if(socket && socket->GetNatLayer() && socket->GetNatLayer()->IsObfuscatedConnectionEstablished())
		return true;
	else
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
	if (socket != NULL && socket->IsConnected())
		return socket->IsObfusicating();
	else
		return false;
}

bool CUpDownClient::ShouldReceiveCryptUDPPackets() const {
	return (thePrefs.IsClientCryptLayerSupported() && SupportsCryptLayer() && theApp.GetPublicIP() != 0
		&& HasValidHash() && (thePrefs.IsClientCryptLayerRequested() || RequestsCryptLayer()) );
}

void CUpDownClient::ProcessChatMessage(CSafeMemFile* data, uint32 nLength)
{
	//filter me?
	if ( (thePrefs.MsgOnlyFriends() && !IsFriend()) || (thePrefs.MsgOnlySecure() && GetUserName()==NULL) )
	{
		if (!GetMessageFiltered()){
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false,_T("Filtered Message from '%s' (IP:%s)"), GetUserName(), ipstr(GetConnectIP()));
		}
		SetMessageFiltered(true);
		return;
	}

	CString strMessage(data->ReadString(GetUnicodeSupport()!=utf8strNone, nLength));
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  %s\n"), strMessage);
	
	// default filtering
	CString strMessageCheck(strMessage);
	strMessageCheck.MakeLower();
	CString resToken;
	int curPos = 0;
	resToken = thePrefs.GetMessageFilter().Tokenize(_T("|"), curPos);
	while (!resToken.IsEmpty())
	{
		resToken.Trim();
		if (strMessageCheck.Find(resToken.MakeLower()) > -1){
			if ( thePrefs.IsAdvSpamfilterEnabled() && !IsFriend() && GetMessagesSent() == 0 ){
				SetSpammer(true);
				theApp.emuledlg->chatwnd->chatselector.EndSession(this);
			}
			return;
		}
		resToken = thePrefs.GetMessageFilter().Tokenize(_T("|"), curPos);
	}

	// advanced spamfilter check
	if (thePrefs.IsChatCaptchaEnabled() && !IsFriend()) {
		// captcha checks outrank any further checks - if the captcha has been solved, we assume its no spam
		// first check if we need to sent a captcha request to this client
		if (GetMessagesSent() == 0 && GetMessagesReceived() == 0 && GetChatCaptchaState() != CA_CAPTCHASOLVED)
		{
			// we have never sent a message to this client, and no message from him has ever passed our filters
			if (GetChatCaptchaState() != CA_CHALLENGESENT)
			{
				// we also aren't currently expecting a cpatcha response
				if (m_fSupportsCaptcha != NULL)
				{
					// and he supports captcha, so send him on and store the message (without showing for now)
					if (m_cCaptchasSent < 3) // no more than 3 tries
					{
						m_strCaptchaPendingMsg = strMessage;
						CSafeMemFile fileAnswer(1024);
						fileAnswer.WriteUInt8(0); // no tags, for future use
						CCaptchaGenerator captcha(4);
						if (captcha.WriteCaptchaImage(fileAnswer)){
							m_strCaptchaChallenge = captcha.GetCaptchaText();
							m_nChatCaptchaState = CA_CHALLENGESENT;
							m_cCaptchasSent++;
							Packet* packet = new Packet(&fileAnswer, OP_EMULEPROT, OP_CHATCAPTCHAREQ);
							theStats.AddUpDataOverheadOther(packet->size);
							SafeSendPacket(packet);
						}
						else{
							ASSERT( false );
							DebugLogError(_T("Failed to create Captcha for client %s"), DbgGetClientInfo());
						}
					}
				}
				else
				{
					// client doesn't supports captchas, but we require them, tell him that its not going to work out
					// with an answer message (will not be shown and doesn't counts as sent message)
					if (m_cCaptchasSent < 1) // dont sent this notifier more than once
					{
						m_cCaptchasSent++;
						// always sent in english
						CString rstrMessage = _T("In order to avoid spam messages, this user requires you to solve a captcha before you can send a message to him. However your client does not supports captchas, so you will not be able to chat with this user.");
						SendChatMessage(rstrMessage);
						DebugLog(_T("Received message from client not supporting captchs, filtered and sent notifier (%s)"), DbgGetClientInfo());
					}
					else
						DebugLog(_T("Received message from client not supporting captchs, filtered, didn't sent notifier (%s)"), DbgGetClientInfo());
				}
				return;
			}
			else //(GetChatCaptchaState() == CA_CHALLENGESENT)
			{
				// this message must be the answer to the captcha request we send him, lets verify
				ASSERT( !m_strCaptchaChallenge.IsEmpty() );
				if (m_strCaptchaChallenge.CompareNoCase(strMessage.Trim().Right(min(strMessage.GetLength(), m_strCaptchaChallenge.GetLength()))) == 0){
					// allright
					DebugLog(_T("Captcha solved, showing withheld message (%s)"), DbgGetClientInfo());
					m_nChatCaptchaState = CA_CAPTCHASOLVED; // this state isn't persitent, but the messagecounter will be used to determine later if the captcha has been solved
					// replace captchaanswer with withheld message and show it
					strMessage = m_strCaptchaPendingMsg;
					m_cCaptchasSent = 0;
					m_strCaptchaChallenge = _T("");
					Packet* packet = new Packet(OP_CHATCAPTCHARES, 1, OP_EMULEPROT, false);
					packet->pBuffer[0] = 0; // status response
					theStats.AddUpDataOverheadOther(packet->size);
					SafeSendPacket(packet);
				}
				else{ // wrong, cleanup and ignore
					DebugLogWarning(_T("Captcha answer failed (%s)"), DbgGetClientInfo());
					m_nChatCaptchaState = CA_NONE;
					m_strCaptchaChallenge = _T("");
					m_strCaptchaPendingMsg = _T("");
					Packet* packet = new Packet(OP_CHATCAPTCHARES, 1, OP_EMULEPROT, false);
					packet->pBuffer[0] = (m_cCaptchasSent < 3)? 1 : 2; // status response
					theStats.AddUpDataOverheadOther(packet->size);
					SafeSendPacket(packet);
					return; // nothing more todo
				}
			}	
		}
		else
			DEBUG_ONLY( DebugLog(_T("Message passed CaptchaFilter - already solved or not needed (%s)"), DbgGetClientInfo()) );

	}
	if (thePrefs.IsAdvSpamfilterEnabled() && !IsFriend()) // friends are never spammer... (but what if two spammers are friends :P )
	{	
		bool bIsSpam = false;
		if (IsSpammer())
			bIsSpam = true;
		else
		{

			// first fixed criteria: If a client  sends me an URL in his first message before I response to him
			// there is a 99,9% chance that it is some poor guy advising his leech mod, or selling you .. well you know :P
			if (GetMessagesSent() == 0){
				int curPos=0;
				CString resToken = CString(URLINDICATOR).Tokenize(_T("|"), curPos);
				while (resToken != _T("")){
					if (strMessage.Find(resToken) > (-1) )
						bIsSpam = true;
					resToken= CString(URLINDICATOR).Tokenize(_T("|"),curPos);
				}
				// second fixed criteria: he sent me 4  or more messages and I didn't answered him once
				if (GetMessagesReceived() > 3)
					bIsSpam = true;
			}
		}
		if (bIsSpam)
		{
			if (IsSpammer()){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("'%s' has been marked as spammer"), GetUserName());
			}
			SetSpammer(true);
			theApp.emuledlg->chatwnd->chatselector.EndSession(this);
			return;

		}
	}

	theApp.emuledlg->chatwnd->chatselector.ProcessMessage(this, strMessage);
}

void CUpDownClient::ProcessCaptchaRequest(CSafeMemFile* data){
	// received a captcha request, check if we actually accept it (only after sending a message ourself to this client)
	if (GetChatCaptchaState() == CA_ACCEPTING && GetChatState() != MS_NONE
		&& theApp.emuledlg->chatwnd->chatselector.GetItemByClient(this) != NULL)
	{
		// read tags (for future use)
		uint8 nTagCount = data->ReadUInt8();
		for (uint32 i = 0; i < nTagCount; i++)
			CTag tag(data, true);
		// sanitize checks - we want a small captcha not a wallpaper
		uint32 nSize = (uint32)(data->GetLength() - data->GetPosition());
		if ( nSize > 128 && nSize < 4096)
		{
			ULONGLONG pos = data->GetPosition();
			BYTE* byBuffer = data->Detach();
			CxImage imgCaptcha(&byBuffer[pos], nSize, CXIMAGE_FORMAT_BMP);
			//free(byBuffer);
			if (imgCaptcha.IsValid() && imgCaptcha.GetHeight() > 10 && imgCaptcha.GetHeight() < 50
				&& imgCaptcha.GetWidth() > 10 && imgCaptcha.GetWidth() < 150 )
			{
				HBITMAP hbmp = imgCaptcha.MakeBitmap();
				if (hbmp != NULL){
					m_nChatCaptchaState = CA_CAPTCHARECV;
					theApp.emuledlg->chatwnd->chatselector.ShowCaptchaRequest(this, hbmp);
					DeleteObject(hbmp);
				}
				else
					DebugLogWarning(_T("Received captcha request from client, Creating bitmap failed (%s)"), DbgGetClientInfo());
			}
			else
				DebugLogWarning(_T("Received captcha request from client, processing image failed or invalid pixel size (%s)"), DbgGetClientInfo());
		}
		else
			DebugLogWarning(_T("Received captcha request from client, size sanitize check failed (%u) (%s)"), nSize, DbgGetClientInfo());
	}
	else
		DebugLogWarning(_T("Received captcha request from client, but don't accepting it at this time (%s)"), DbgGetClientInfo());
}

void CUpDownClient::ProcessCaptchaReqRes(uint8 nStatus)
{
	if (GetChatCaptchaState() == CA_SOLUTIONSENT && GetChatState() != MS_NONE
		&& theApp.emuledlg->chatwnd->chatselector.GetItemByClient(this) != NULL)
	{
		ASSERT( nStatus < 3 );
		m_nChatCaptchaState = CA_NONE;
		theApp.emuledlg->chatwnd->chatselector.ShowCaptchaResult(this, GetResString((nStatus == 0) ? IDS_CAPTCHASOLVED : IDS_CAPTCHAFAILED));
	}
	else {
		m_nChatCaptchaState = CA_NONE;
		DebugLogWarning(_T("Received captcha result from client, but don't accepting it at this time (%s)"), DbgGetClientInfo());
	}
}

CFriend* CUpDownClient::GetFriend() const
{
	if (m_Friend != NULL && theApp.friendlist->IsValid(m_Friend))
		return m_Friend;
	else if (m_Friend != NULL)
		ASSERT( FALSE );
	return NULL;
}

void CUpDownClient::SendChatMessage(CString strMessage)
{
	CSafeMemFile data;
	data.WriteString(strMessage, GetUnicodeSupport());
	Packet* packet = new Packet(&data, OP_EDONKEYPROT, OP_MESSAGE);
	theStats.AddUpDataOverheadOther(packet->size);
	SafeSendPacket(packet);
}

bool CUpDownClient::HasPassedSecureIdent(bool bPassIfUnavailable) const
{
	if (credits != NULL)
	{
		if (credits->GetCurrentIdentState(GetConnectIP()) == IS_IDENTIFIED 
			|| (credits->GetCurrentIdentState(GetConnectIP()) == IS_NOTAVAILABLE && bPassIfUnavailable))
		{
			return true;
		}
	}
	return false;
}

void CUpDownClient::SendFirewallCheckUDPRequest()
{
	ASSERT( GetKadState() == KS_FWCHECK_UDP );
	if (!Kademlia::CKademlia::IsRunning()){
		SetKadState(KS_NONE);
		return;
	}
	else if (GetUploadState() != US_NONE || GetDownloadState() != DS_NONE || GetChatState() != MS_NONE
		|| GetKadVersion() <= KADEMLIA_VERSION5_48a || GetKadPort() == 0)
	{
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, true, ntohl(GetIP()), 0); // inform the tester that this test was cancelled
		SetKadState(KS_NONE);
		return;
	}
	ASSERT( Kademlia::CKademlia::GetPrefs()->GetExternalKadPort() != 0 );
	CSafeMemFile data;
	data.WriteUInt16(Kademlia::CKademlia::GetPrefs()->GetInternKadPort());
	data.WriteUInt16(Kademlia::CKademlia::GetPrefs()->GetExternalKadPort());
	data.WriteUInt32(Kademlia::CKademlia::GetPrefs()->GetUDPVerifyKey(GetConnectIP()));
	Packet* packet = new Packet(&data, OP_EMULEPROT, OP_FWCHECKUDPREQ);
	theStats.AddUpDataOverheadKad(packet->size);
	SafeSendPacket(packet);
}

void CUpDownClient::ProcessFirewallCheckUDPRequest(CSafeMemFile* data){
	if (!Kademlia::CKademlia::IsRunning() || Kademlia::CKademlia::GetUDPListener() == NULL){
		DebugLogWarning(_T("Ignored Kad Firewallrequest UDP because Kad is not running (%s)"), DbgGetClientInfo());
		return;
	}
	// first search if we know this IP already, if so the result might be biased and we need tell the requester 
	bool bErrorAlreadyKnown = false;
	if (GetUploadState() != US_NONE || GetDownloadState() != DS_NONE || GetChatState() != MS_NONE)
		bErrorAlreadyKnown = true;
	else if (Kademlia::CKademlia::GetRoutingZone()->GetContact(ntohl(GetConnectIP()), 0, false) != NULL)
		bErrorAlreadyKnown = true;

	uint16 nRemoteInternPort = data->ReadUInt16();
	uint16 nRemoteExternPort = data->ReadUInt16();
	uint32 dwSenderKey = data->ReadUInt32();
	if (nRemoteInternPort == 0){
		DebugLogError(_T("UDP Firewallcheck requested with Intern Port == 0 (%s)"), DbgGetClientInfo());
		return;
	}
	if (dwSenderKey == 0)
		DebugLogWarning(_T("UDP Firewallcheck requested with SenderKey == 0 (%s)"), DbgGetClientInfo());
	
	CSafeMemFile fileTestPacket1;
	fileTestPacket1.WriteUInt8(bErrorAlreadyKnown ? 1 : 0);
	fileTestPacket1.WriteUInt16(nRemoteInternPort);
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA2_FIREWALLUDP", ntohl(GetConnectIP()), nRemoteInternPort);
	Kademlia::CKademlia::GetUDPListener()->SendPacket(&fileTestPacket1, KADEMLIA2_FIREWALLUDP, ntohl(GetConnectIP())
		, nRemoteInternPort, Kademlia::CKadUDPKey(dwSenderKey, theApp.GetPublicIP(false)), NULL);
	
	// if the client has a router with PAT (and therefore a different extern port than intern), test this port too
	if (nRemoteExternPort != 0 && nRemoteExternPort != nRemoteInternPort){
		CSafeMemFile fileTestPacket2;
		fileTestPacket2.WriteUInt8(bErrorAlreadyKnown ? 1 : 0);
		fileTestPacket2.WriteUInt16(nRemoteExternPort);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA2_FIREWALLUDP", ntohl(GetConnectIP()), nRemoteExternPort);
		Kademlia::CKademlia::GetUDPListener()->SendPacket(&fileTestPacket2, KADEMLIA2_FIREWALLUDP, ntohl(GetConnectIP())
			, nRemoteExternPort, Kademlia::CKadUDPKey(dwSenderKey, theApp.GetPublicIP(false)), NULL);
	}
	DebugLog(_T("Answered UDP Firewallcheck request (%s)"), DbgGetClientInfo());
}

void CUpDownClient::SetConnectOptions(uint8 byOptions, bool bEncryption, bool bCallback)
{
	SetCryptLayerSupport((byOptions & 0x01) != 0 && bEncryption);
	SetCryptLayerRequest((byOptions & 0x02) != 0 && bEncryption);
	SetCryptLayerRequires((byOptions & 0x04) != 0 && bEncryption);
	SetDirectUDPCallbackSupport((byOptions & 0x08) != 0 && bCallback);
}

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
void CUpDownClient::CheckOnLAN( uint32 val )
{
	if (!NeoPrefs.IsLancastEnabled() || !val)
		return;
	m_bLanClient = theApp.lancast->IsLanIP(val);

	ASSERT(!m_bLanClient || socket == NULL || socket->IsLanSocket()); // LanSocket state is set on create and on connection accept so it should be always done at this point
	//if(m_bLanClient){
		//m_nUserIDHybrid = ntohl(m_dwUserIP);
		//if(socket)
		//	socket->SetOnLan();
	//}
}
#endif //LANCAST // NEO: NLC END <-- Xanatos --

// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
bool CUpDownClient::NotifyIdChage()
{
	if (theApp.listensocket->TooManySockets() && !(socket && socket->IsConnected()) )
	{
		m_bWainingNotifyIdChage = true;
		return true;
	}
	m_bWainingNotifyIdChage = false;

	m_bNotifyIdChage = true;
	return TryToConnect();
}
// NEO: RIC END <-- Xanatos --


#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
bool CUpDownClient::IsLongTermStorred()
{
	if(GetSourceFrom() != SF_STORAGE)
		return false;
	// if the source was seen recently we handle it as if it would be one from server/xs/kad
	return time(NULL) - m_uLastSeen > (UINT)NeoPrefs.PartPrefs.GetSourceReaskTime()*5; // 29 min * 5 ~2,5Hr thats a very reasonable time
}
#endif // NEO_SS // NEO: NSS END <-- Xanatos --

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
void CUpDownClient::LinkSource(CKnownSource* Source)
{
	ASSERT(!source || source->IsUsed());
	if(source){
		// NEO: SFL - [SourceFileList]
		if(NeoPrefs.SaveSourceFileList()){
			// NEO: SCFS - [SmartClientFileStatus]
			CClientFileStatus* Status;
			CCKey key;
			POSITION pos = m_fileStatusMap.GetStartPosition();
			while (pos){
				m_fileStatusMap.GetNextAssoc(pos, key, Status);
				if(!Status->IsArcived()){ // should only happen when the feature was enabled later on
					SeenFileStruct* SeenFile = source->GetSeenFile(Status->GetFileHash());
					if(!SeenFile){ // file yet unknown
						CKnownFile* File = theApp.downloadqueue->GetFileByID(Status->GetFileHash());
						if(!File)
							File = theApp.knownfiles->FindKnownFileByID(Status->GetFileHash());
						if(File)
							SeenFile = source->AddSeenFile(File->GetFileHash(),File->GetFileSize());
					}
					if(SeenFile && SeenFile->prtFileStatus == NULL){ // just in case
						SeenFile->prtFileStatus = Status;
						Status->SetArcived();
					}
				}
				if(Status->IsArcived()){
					m_fileStatusMap.RemoveKey(key);
					Status->SetUsed(false);
				}
			}
			// NEO: SCFS END
		}
		// NEO: SFL END
		source->UnUse();
		source->Detach(this);
	}
	source = Source;
	if(Source){
		Source->Use();
		// NEO: SFL - [SourceFileList]
		if(NeoPrefs.SaveSourceFileList()){
			// NEO: SCFS - [SmartClientFileStatus]
			SeenFileStruct* SeenFile;
			CCKey tmpkey;
			POSITION pos = Source->m_SeenFiles.GetStartPosition();
			while (pos){
				Source->m_SeenFiles.GetNextAssoc(pos, tmpkey, SeenFile);
				if(SeenFile->prtFileStatus && SeenFile->prtFileStatus->IsUsed() == false){ // don't share states !!!
					CClientFileStatus* Status;
					if(m_fileStatusMap.Lookup(CCKey(SeenFile->abyFileHash), Status)){
						//SeenFile->prtFileStatus->Update(Status); // this happens only when the user changes his hash or more likly and other user gets his IP
						m_fileStatusMap.RemoveKey(CCKey(SeenFile->abyFileHash));
						Status->SetUsed(false);
						if(!Status->IsArcived())
							delete Status;
					}
					m_fileStatusMap.SetAt(CCKey(SeenFile->prtFileStatus->GetFileHash()), SeenFile->prtFileStatus);
					SeenFile->prtFileStatus->SetUsed();
				}
			}
			// NEO: SCFS END
		}
		// NEO: SFL END
	}
}

void CUpDownClient::AttachSource(CKnownSource* Source)
{
	LinkSource(Source);

	SetIP(source->GetIP()); // m_nConnectIP = m_dwUserIP
	m_nUserPort = source->GetUserPort();

	m_nUserIDHybrid = source->GetUserIDHybrid();
	m_nUDPPort = source->GetUDPPort();
	m_nKadPort = source->GetKadPort();

	SetUserName(source->GetUserName());
	m_strClientSoftware = source->GetClientSoftVer();
	m_strModVersion = source->GetClientModVer();
	m_clientSoft = source->GetClientSoft();
}
#endif // NEO_CD // NEO: NCD END <-- Xanatos --

// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
CClientFileStatus* CUpDownClient::GetFileStatus(const CKnownFile* File, bool bAdd) const
{
	if(File == NULL){
		ASSERT(bAdd == false);
		return NULL;
	}
	CClientFileStatus* Status;
	if(m_fileStatusMap.Lookup(CCKey(File->GetFileHash()),Status))
		return Status;
	else if(bAdd){
		Status = new CClientFileStatus(File);
		(const_cast<CUpDownClient*>(this))->m_fileStatusMap.SetAt(CCKey(Status->GetFileHash()),Status);
#ifdef NEO_CD // NEO: SFL - [SourceFileList]
		//Status->SetUsed(); // is done in the construct
		if(NeoPrefs.SaveSourceFileList() && source){
			SeenFileStruct* SeenFile = source->GetSeenFile(Status->GetFileHash());
			if(SeenFile && SeenFile->prtFileStatus == NULL){ // seen file should be there, ptr = 0 only when 2 cleints use the same soruce object
				SeenFile->prtFileStatus = Status;
				Status->SetArcived();
			}
		}
#endif // NEO_CD // NEO: SFL END
		return Status;
	}else
		return NULL;
}

void CUpDownClient::ClearFileStatus(const CKnownFile* File)
{
	CClientFileStatus* Status = GetFileStatus(File);
	if(Status){
		m_fileStatusMap.RemoveKey(CCKey(File->GetFileHash()));
#ifdef NEO_CD // NEO: SFL - [SourceFileList]
		Status->SetUsed(false);
		if(!Status->IsArcived())
#endif // NEO_CD // NEO: SFL END
			delete Status;
	}
}
// NEO: SCFS END <-- Xanatos --

// NEO: NMP - [NeoModProt] -- Xanatos -->
void CUpDownClient::SendModInfoPacket(){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(128);

	uint32 tagcount=1;

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	if(theApp.IsFirewalled() && Kademlia::CKademlia::IsConnected())
		tagcount += 1; 

	// NEO: XSB - [XSBuddy]
	if(theApp.clientlist->GetXsBuddyStatus() == XB_HIGH_BUDDY && theApp.IsFirewalled())
		tagcount += 2; 
	// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END

	data.WriteUInt32(tagcount); // nr. of tags


	const UINT L2HAC = NeoPrefs.UseLowID2HighIDAutoCallback()	? 1 : 0; // NEO: L2HAC - [LowID2HighIDAutoCallback]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	const UINT NatT = NeoPrefs.IsNatTraversalEnabled()	? 1 : 0;
#endif //NATTUNNELING // NEO: NATT END
	const UINT SCT = NeoPrefs.UseSubChunkTransfer()		? 1 : 0; // NEO: SCT - [SubChunkTransfer]
	const UINT ICS = NeoPrefs.UseIncompletePartStatus()	? 1 : 0; // NEO: ICS - [InteligentChunkSelection] 
	const UINT UDPs = (SCT != 0 || ICS != 0)			? 1 : 0;
	uint32 uProtMod =	
						((L2HAC	& 0x01)	<< 8 ) | // NEO: L2HAC - [LowID2HighIDAutoCallback]
						((SCT	& 0x01)	<< 7 ) | // NEO: SCT - [SubChunkTransfer]
						((1		& 0x01)	<< 6 ) | // NEO: XC - [ExtendedComments]
						((1		& 0x01)	<< 5 ) | // NEO: NXS - [NeoXS]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
						((NatT	& 0x01)	<< 4 ) |
#endif //NATTUNNELING // NEO: NATT END
						((UDPs  & 0x01)	<< 3 ) | // UDP Mod File Status
						((ICS	& 0x01)	<< 2 ) | // ICSv2 Regular Version
						((1		& 0x01)	<< 1 ) | // LowID UDP Ping Support (notifyes a fix form xman1 that allow the remote low ID to use udp reasks) // NEO: MOD - [NewUploadState]
						((1		& 0x01)	<< 0 );  // Unsolicited Part Status (idea from netfinity, allows our client ro recieve filestatus at any time) // NEO: USPS - [UnSolicitedPartStatus]
	CTag tagProtExt(MT_MOD_PROTOCOL_EXTENSIONS,uProtMod);
	tagProtExt.WriteNewEd2kTag(&data);
	

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	if(theApp.IsFirewalled() && Kademlia::CKademlia::IsConnected())
	{
		// Note: to connect a cleint over his buddy or to request a calback we need not only his buddy IP/Port but also his Buddy ID
		// So to get this realy to work we need to transmitt the buddy ID to
		Kademlia::CUInt128 uBuddyID(true);
		uBuddyID.Xor(Kademlia::CKademlia::GetPrefs()->GetKadID());
		byte cID[16];
		uBuddyID.ToByteArray(cID);
		CTag tagBuddyID(MT_EMULE_BUDDYID, cID);
		tagBuddyID.WriteNewEd2kTag(&data);
	}

	// NEO: XSB - [XSBuddy]
	if(theApp.clientlist->GetXsBuddyStatus() == XB_HIGH_BUDDY && theApp.IsFirewalled())
	{
		CTag tagBuddyIP(MT_XS_EMULE_BUDDYIP, theApp.clientlist->GetXsBuddy()->GetIP() ); 
		tagBuddyIP.WriteTagToFile(&data);
	
		CTag tagBuddyPort(MT_XS_EMULE_BUDDYUDP, 
//					( RESERVED												)
					((uint32)theApp.clientlist->GetXsBuddy()->GetUDPPort()  ) 
					);
		tagBuddyPort.WriteTagToFile(&data);
	} 
	// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END

	Packet* packet = new Packet(&data,OP_MODPROT);
	packet->opcode = OP_MODINFOPACKET;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__ModInfoPacket", this);
	theStats.AddUpDataOverheadOther(packet->size);
	socket->SendPacket(packet,true,true);
}

void CUpDownClient::ClearModInfoProperties()
{
	m_nModClient = MOD_NONE; // NEO: MID - [ModID] <-- Xanatos -- 

	m_fSupportsModProt = 0;
	m_fSupportNeoXS = 0;
	m_fSupportL2HAC = 0; // NEO: L2HAC - [LowID2HighIDAutoCallback]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_fSupportsNatTraversal = 0;
#endif //NATTUNNELING // NEO: NATT END
	m_fLowIDUDPPingSupport = 0;
	m_fUnsolicitedPartStatus = 0;
	m_fExtendedComments = 0; // NEO: XC - [ExtendedComments]
	// NEO: ICS - [InteligentChunkSelection]
	m_fIncompletePartVer1 = 0;
	m_fIncompletePartVer2 = 0;
	// NEO: ICS END
	m_fSubChunksSupport = 0; // NEO: SCT - [SubChunkTransfer]
	m_fExtendedUDPStatus = 0;

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_bNeoKadNatT = false;
#endif //NATTUNNELING // NEO: NATT END
}

void CUpDownClient::ProcessModInfoPacket(const uchar* pachPacket, uint32 nSize)
{
	bool bDbgInfo = thePrefs.GetUseDebugDevice();
	CString m_strModInfo;
	CSafeMemFile data(pachPacket, nSize);

	uint32 tagcount = data.ReadUInt32();
	if (bDbgInfo)
		m_strModInfo.AppendFormat(_T("  Tags=%u"), (UINT)tagcount);
	for (uint32 i = 0; i < tagcount; i++)
	{
		CTag temptag(&data, false);
		switch (temptag.GetNameID())
		{
			case MT_MOD_PROTOCOL_EXTENSIONS:
				if (temptag.IsInt()){
					m_fSupportL2HAC				= ((temptag.GetInt() >> 8 ) & 0x01) != 0; // NEO: L2HAC - [LowID2HighIDAutoCallback]
					m_fSubChunksSupport			= ((temptag.GetInt() >> 7 ) & 0x01) != 0; // NEO: SCT - [SubChunkTransfer]
					m_fExtendedComments			= ((temptag.GetInt() >> 6 ) & 0x01) != 0; // NEO: XC - [ExtendedComments]
					m_fSupportNeoXS				= ((temptag.GetInt() >> 5 ) & 0x01) != 0; // NEO: NXS - [NeoXS]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
					m_fSupportsNatTraversal		= ((temptag.GetInt() >> 4 ) & 0x01) != 0; // Support Nat Traversal
#endif //NATTUNNELING // NEO: NATT END
					m_fExtendedUDPStatus		= ((temptag.GetInt() >> 3 ) & 0x01) != 0; // UDP Mod File Status
					m_fIncompletePartVer2		= ((temptag.GetInt() >> 2 ) & 0x01) != 0; // ICSv2 new official Version // NEO: ICS - [InteligentChunkSelection] 
					m_fLowIDUDPPingSupport		= ((temptag.GetInt() >> 1 ) & 0x01) != 0; // LowID UDP Ping Support (notifyes a fix form xman1 that allow the remote low ID to use udp reasks) // NEO: MOD - [NewUploadState]
					m_fUnsolicitedPartStatus	= ((temptag.GetInt() >> 0 ) & 0x01) != 0; // Unsolicited Part Status (idea from netfinity, allows our client ro recieve filestatus at any time) // NEO: USPS - [UnSolicitedPartStatus]
				}else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
			case MT_EMULE_BUDDYID:
				if(temptag.IsHash())
				{
					SetBuddyID(temptag.GetHash());
					m_bNeoKadNatT = true; // this tellus us that this is an cleint that supports the unofficial Kad NatT callback
				}
				else{
					if (bDbgInfo)
						m_strModInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				}
				break;

			// NEO: XSB - [XSBuddy]
			case MT_XS_EMULE_BUDDYUDP:
				// 16 --Reserved for future use--
				// 16 BUDDY Port
				if (temptag.IsInt()) {
					m_nXsBuddyPort = (uint16)temptag.GetInt();
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyPort=%u"), m_nBuddyPort);
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case MT_XS_EMULE_BUDDYIP:
				// 32 BUDDY IP
				if (temptag.IsInt()) {
					m_nXsBuddyIP = temptag.GetInt();
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyIP=%s"), ipstr(m_nBuddyIP));
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;
			// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END

			case CT_MOD_VERSION:
				if (temptag.IsStr())
					m_strModVersion = temptag.GetStr();
				else if (temptag.IsInt())
					m_strModVersion.Format(_T("ModID=%u"), temptag.GetInt());
				else
					m_strModVersion = _T("ModID=<Unknown>");
				DetectMod(); // NEO: MID - [ModID] <-- Xanatos -- 
				if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
				break;
			default:
				if (bDbgInfo)
					m_strModInfo.AppendFormat(_T("\n  ***UnkTag=%s"), temptag.GetFullInfo());
		}
	}

	if (bDbgInfo && data.GetPosition() < data.GetLength()){
		m_strModInfo.AppendFormat(_T("\n  ***AddData: %u bytes"), data.GetLength() - data.GetPosition());
	}
}
// NEO: NMP END <-- Xanatos --

// Note: This is for ICSv1 BAckwards compatybility
// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
void CUpDownClient::SendLegacyICSStatus(CKnownFile* file) 
{
	if (NeoPrefs.UseIncompletePartStatus() && GetIncompletePartVersion() && file->IsPartFile())
	{
		CSafeMemFile data(16+16);
		data.WriteHash16(file->GetFileHash());
		((CPartFile*)file)->WriteIncPartStatus(&data);
		Packet* packet = new Packet(&data, OP_EMULEPROT);
		packet->opcode = OP_FILEINCSTATUS;
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__FileIncompleteStatus", this, file->GetFileHash());
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet,true);
	}
}
// NEO: ICS END <-- Xanatos --

// NEO: NMPm - [NeoModProtMultiPacket] -- Xanatos -->
// Every entry is identyfyed by an uint8 opcode and have a uint16 size field
// This Multi Packet is designed to be used with file reask retated stuff as it contains a file hash on the very begin
//
void CUpDownClient::WriteModMultiPacket(CSafeMemFile* data_out, CKnownFile* file, bool bReq, bool bUDP)
{
	bool bWriteSize = SupportsModProt();

	CClientFileStatus* status = GetFileStatus(file, true); // NEO: SCFS - [SmartClientFileStatus]

	// NEO: SCT - [SubChunkTransfer]
	if(bReq == false // send the map only whe it's an answer
	 && NeoPrefs.UseSubChunkTransfer() && SupportsSubChunks() 
	 && file->IsPartFile() 
	 && (bUDP ? status->m_uSCTpos // send in udp packet only when we are in progress sending out map, when the map is out the pos will be reseted to 0
			  : GetUploadState() == US_NONEEDEDPARTS) ) // the cleint does not need any full part but he may need some blocks of incomplete parts
	{
		data_out->WriteUInt8(OP_SUBCHUNKMAPS);
		ULONGLONG pos = data_out->GetPosition();
		if(bWriteSize)
			data_out->WriteUInt16(0);
		((CPartFile*)file)->WriteSubChunkMaps(data_out, status);
		if(bWriteSize){
			data_out->Seek(pos, CFile::begin);
			data_out->WriteUInt16((uint16)(data_out->GetLength() - (data_out->GetPosition()+2)));
			data_out->Seek(0, CFile::end);
		}
	}
	// NEO: SCT END

	uint16 PartStatusSize = 0;
	if(bWriteSize){
		PartStatusSize = 2 + (file->GetED2KPartCount()/8);
		if(file->GetED2KPartCount()%8)
			PartStatusSize += 1;
	}

	// NEO: ICS - [InteligentChunkSelection]
	if (NeoPrefs.UseIncompletePartStatus() && GetIncompletePartVersion() >= (bReq ? 2 : 1) && file->IsPartFile())
	{
		data_out->WriteUInt8(OP_FILEINCSTATUS);
		if(bWriteSize)
			data_out->WriteUInt16(PartStatusSize);
		((CPartFile*)file)->WriteIncPartStatus(data_out);
	}
	// NEO: ICS END
}

void CUpDownClient::ReadModMultiPacket(CSafeMemFile* data_in, CKnownFile* file, uint8 opcode)
{
	CClientFileStatus* status = GetFileStatus(file, true); // NEO: SCFS - [SmartClientFileStatus]

	bool bReadSize = SupportsModProt();

	while (data_in->GetLength() - data_in->GetPosition())
	{
		// Note: opcode it != 0 only when we cone from the official multi packet // for backwards compatybility
		uint8 opcode_in = opcode ? opcode : data_in->ReadUInt8(); // read the Entry ID
		uint32 size_in = bReadSize ? data_in->ReadUInt16() : 0; // every mod multi packet entry have a size information, so it can be skipped 
		if(size_in == 0xFFFF) // just in case
			size_in = data_in->ReadUInt32();
		uint32 pos = (uint32)data_in->GetPosition();
		try{
			switch (opcode_in)
			{
				case NULL: // kill opcode to exit the loop if you want place datas below this entry list you must send the kill opcode first
					return;
				// NEO: SCT - [SubChunkTransfer]
				case OP_SUBCHUNKMAPS:
					status->ReadSubChunkMaps(data_in); // NEO: SCFS - [SmartClientFileStatus]
					ASSERT( !bReadSize || pos + size_in == data_in->GetPosition());
					break;
				// NEO: SCT END
				// NEO: ICS - [InteligentChunkSelection]
				case OP_FILEINCSTATUS:
					status->ReadFileStatus(data_in, CFS_Incomplete);
					file->UpdatePartsInfoEx(CFS_Incomplete);
					ASSERT( !bReadSize || pos + size_in == data_in->GetPosition());
					break;
				// NEO: ICS END
				default:
				{
					// Note: when we arrived at this point there is a bug in the mod capability detection/notification!
					//			The remote client shall send only segments that we know.
					if(bReadSize == 0 || data_in->GetPosition() + size_in > data_in->GetLength())
						AfxThrowFileException(CFileException::endOfFile, 0, _T("MemoryFile"));
					DebugLog(_T("Unknown MOD sub opcode 0x%02x received"), opcode_in);
					data_in->Seek(size_in, CFile::current);
				}
			} // switch
		}
		catch(CFileException* error) // we handle them here to know what sub opcode caused that error
		{
			error->Delete();
			CString strError;
			strError.Format(_T("Invalid Mod sub opcode 0x%02x received, size %i, pos %i"), opcode_in, size_in, pos);
			throw strError;
		}
		if(opcode) // for backwards compatybility
			return; // in this mode we read only one entry at the time
	} // while
}
// NEO: NMPm END <-- Xanatos --

// NEO: NXS - [NeoXS] -- Xanatos -->
void CUpDownClient::WriteNeoXSTags(CSafeMemFile* data) const
{
	byte byValue[255];
	ZeroMemory(&byValue[0],sizeof(byValue));
	byte* bValue = &byValue[0];
	uint8 uLength = 0;

	if(HasLowID() && GetServerIP() && GetServerPort())
	{
		WriteNanoTagIDLen(bValue,NT_ServerIPPort,4+2); uLength+=2; // len = 2
		*((uint32*)bValue) = GetServerIP();
		bValue+=4; uLength+=4; // len = 6
		*((uint16*)bValue) = GetServerPort();
		bValue+=2; uLength+=2; // len = 8
	}

	if(HasLowID() && HasValidBuddyID())
	{
		WriteNanoTagIDLen(bValue,NT_BuddyID,16); uLength+=2; // len = 10
		memcpy(bValue,GetBuddyID(),16);
		bValue+=16; uLength+=16; // len = 26

		if(GetBuddyIP() && GetBuddyPort())
		{
			WriteNanoTagIDLen(bValue,NT_BuddyIPPort,4+2); uLength+=2; // len = 28
			*((uint32*)bValue) = GetBuddyIP();
			bValue+=4; uLength+=4; // len = 32
			*((uint16*)bValue) = GetBuddyPort();
			bValue+=2; uLength+=2; // len = 34
		}
	}

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	const uint8 uSupportsNatTraversal = SupportsNatTraversal();
	if(uSupportsNatTraversal)
	{
		WriteNanoTagIDLen(bValue,NT_NATT); uLength++; // len = 40
		(byte&)*bValue = uSupportsNatTraversal; 
		if(m_bNeoKadNatT)
			(byte&)*bValue |= 0x80; // this flag tells us that the cleint supports an unifficial kad Callback
		bValue++; uLength++; // len = 41
	}

	// NEO: XSB - [XSBuddy]
	if(GetXsBuddyIP())
	{
		WriteNanoTagIDLen(bValue,NT_XsBuddyIPPort,4+2); uLength+=2; // len = 43
		*((uint32*)bValue) = GetXsBuddyIP();
		bValue+=4; uLength+=4; // len = 47
		*((uint16*)bValue) = GetXsBuddyPort();
		bValue+=2; uLength+=2; // len = 49
	}
	// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END

	// 5 Reserved (!)
	// 1 CryptLayer Required
	// 1 CryptLayer Requested
	// 1 CryptLayer Supported
	const uint8 uSupportsCryptLayer	= SupportsCryptLayer() ? 1 : 0;
	const uint8 uRequestsCryptLayer	= RequestsCryptLayer() ? 1 : 0;
	const uint8 uRequiresCryptLayer	= RequiresCryptLayer() ? 1 : 0;
	const uint8 byCryptOptions = (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
	if(byCryptOptions)
	{
		WriteNanoTagIDLen(bValue,NT_OBFU); uLength++; // len = 50
		(byte&)*bValue = byCryptOptions; 
		bValue++; uLength++; // len = 51
	}

	ASSERT(uLength <= 51);
	ASSERT(uLength == (bValue-&byValue[0]));

	data->WriteUInt8((uint8)(bValue-&byValue[0]));
	if(uLength)
		data->Write(&byValue[0],uLength);
}

void CUpDownClient::tNeoXSTags::Clear()
{
	bEmpty = true;

	dwServerIP = 0;
	nServerPort = 0;

	bBuddyID = false;
	//memset(&abyBuddyID[0],0,16);
	dwBuddyIP = 0;
	nBuddyPort = 0;

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	uSupportsNatTraversal = 0;

	// NEO: XSB - [XSBuddy]
	dwXsBuddyIP = 0;
	nXsBuddyPort = 0;
	// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END

	byCryptOptions = 0;
}

void CUpDownClient::tNeoXSTags::Attach(CUpDownClient* newcleint)
{
	if(bEmpty)
		return;

	newcleint->SetServerIP(dwServerIP);
	newcleint->SetServerPort(nServerPort);

	if(bBuddyID){
		newcleint->SetBuddyID(&abyBuddyID[0]);
		newcleint->SetBuddyIP(dwBuddyIP);
		newcleint->SetBuddyPort(nBuddyPort);
	}

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	newcleint->SetNatTraversalSupport(uSupportsNatTraversal != 0, (uSupportsNatTraversal & 0x80) != 0);

	// NEO: XSB - [XSBuddy]
	newcleint->SetXsBuddyIP(dwXsBuddyIP);
	newcleint->SetXsBuddyPort(nXsBuddyPort);
	// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END

	newcleint->SetCryptLayerSupport((byCryptOptions & 0x01) != 0);
	newcleint->SetCryptLayerRequest((byCryptOptions & 0x02) != 0);
	newcleint->SetCryptLayerRequires((byCryptOptions & 0x04) != 0);
}

void CUpDownClient::ReadNeoXSTags(CSafeMemFile* sources, tNeoXSTags* NeoXSTags)
{
	NeoXSTags->Clear(); // clear the old data and set empty

	UINT len = sources->ReadUInt8();
	if(len == 0xFF) // just in cas for future versions
		len = sources->ReadUInt16();

	if(len)
	{
		if(sources->GetPosition() + len > sources->GetLength())
			throw CString(_T("Wrong Neo XS Tag List size !!!"));

		byte byValue[256];
		sources->Read(&byValue[0],min(len,sizeof(byValue)));
		byte* pBuffer = &byValue[0];
		byte* pBuffEnd = &byValue[0] + len; // last valid adress

		if(len > sizeof(byValue)) // just in cas for future versions
			sources->Seek(len-sizeof(byValue),CFile::current); // skip what we havn't read abive

		while(pBuffEnd >= pBuffer + 1) // we need to have one byte in peto, the GetNanoTagLen may want read 2 of tham, and besides entries without content are not valid anyway
		{
			uint8 uID = GetNanoTagID(pBuffer);
			uint8 uLength = GetNanoTagLen(pBuffer);
			if(pBuffer + uLength > pBuffEnd || uLength == 0)
				throw StrLine(_T("Wrong Neo XS Tag Entry %i size %i!!!"),(int)uID,(int)uLength);

			byte* bValue = pBuffer;
			pBuffer += uLength;

			switch(uID)
			{
			case NT_ServerIPPort:
				ASSERT(uLength == 6);
				NeoXSTags->dwServerIP = *((uint32*)bValue);
				bValue += 4;
				NeoXSTags->nServerPort = *((uint16*)bValue);
				break;
			case NT_BuddyID:
				ASSERT(uLength == 16);
				NeoXSTags->bBuddyID = true;
				memcpy(&NeoXSTags->abyBuddyID[0],bValue,16);
				break;
			case NT_BuddyIPPort:
				ASSERT(uLength == 6);
				NeoXSTags->dwBuddyIP = *((uint32*)bValue);
				bValue += 4;
				NeoXSTags->nBuddyPort = *((uint16*)bValue);
				break;
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
			case NT_NATT:
				NeoXSTags->uSupportsNatTraversal = ((byte)*bValue & 0x81);
				break;
			// NEO: XSB - [XSBuddy]
			case NT_XsBuddyIPPort:
				ASSERT(uLength == 6);
				NeoXSTags->dwXsBuddyIP = *((uint32*)bValue);
				bValue += 4;
				NeoXSTags->nXsBuddyPort = *((uint16*)bValue);
				break;
			// NEO: XSB END
#endif //NATTUNNELING // NEO: NATT END
			case NT_OBFU:
				NeoXSTags->byCryptOptions = (byte)*bValue ;
				break;
			default:
				DebugLog(_T("Unknown Neo XS Tag 0x%02x received, size %i"), uID, uLength);
			} // switch
		} // while

		NeoXSTags->bEmpty = false;
	}
}
// NEO: NXS END <-- Xanatos --

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
bool CUpDownClient::IsCallback(bool bReask)
{
	return (m_bRequestingFileList // NEO: XSF - [ExtendedSharedFiles]
		||  m_bNotifyIdChage // NEO: RIC - [ReaskOnIDChange]
		||  GetUploadState() == US_WAITCALLBACK // NEO: LUC - [LowIDUploadCallBack]
		|| (GetDownloadState() == DS_WAITCALLBACK && (!bReask || !GetRemoteQueueRank() || m_bReaskPending))); // Reask over KAD is only done every 2nd time
}

// NEO: XSB - [XSBuddy]
void CUpDownClient::SendXsBuddyRequest()
{
	if (socket && socket->IsConnected()){
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__XS_BuddyReq", this);
		Packet* packet = new Packet(OP_XS_BUDDY_REQ,0,OP_MODPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true);
	}
}
// NEO: XSB END

// NEO: RTP - [ReuseTCPPort]
void CUpDownClient::SendPublicPortRequest(){
	if (socket && socket->IsConnected()){
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__PublicPortReq", this);
		Packet* packet = new Packet(OP_PUBLICPORT_REQ,0,OP_MODPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true);
		m_NeedOurPublicPort = 1;
	}
}

void CUpDownClient::ProcessPublicPortAnswer(const BYTE* pbyData, UINT uSize){
	if (uSize != 2)
		throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
	uint16 uPort = PeekUInt16(pbyData);
	if (m_NeedOurPublicPort == 1){ // did we?
		m_NeedOurPublicPort = 0;
		theApp.SetPublicPort(uPort);
	}	
}
// NEO: RTP END
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
bool CUpDownClient::CheckXSAsk()
{
	// Check for XS exploid, asking without answering
	// WARNING: official clients do this an accepted but not answered request is already being counted !!!
	// for proper operation "unsigned int nTimePassedClient = dwTickCount - GetLastAskedForSources(); // GetLastSrcAnswerTime();" is nessesery !!!
	if(m_uXSReqs > 2 
	 && (!m_uXSAnswer || (m_uXSReqs / m_uXSAnswer > 2))){
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: XsExploit (no answer)"), GetUserName(), ipstr(GetConnectIP()));
		Ban(_T("XsExploit (no answer)"));
		return true;
	}

	// check for fast XS, to fast asking
	// WARNING: official clients do this due to a inconsistent implementation of the reask roules !!!
	// Bevoure we can count the ask we must go sure the client had got our last answer and set SetLastSrcReqTime() only when we realy answered !!!
	 uint32 timeSinceLastAnswer = ::GetTickCount() - GetLastSrcReqTime();
	if(timeSinceLastAnswer > CONNECTION_LATENCY){ // ~22 sec
		if(timeSinceLastAnswer < (SOURCECLIENTREASKS/2)){ // 20 min
			m_uFastXSCounter++;
			if(m_uFastXSCounter >= BADCLIENTBAN){
				if (thePrefs.GetLogBannedClients())
					AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: XsExploit (to fast)"), GetUserName(), ipstr(GetConnectIP()));
				Ban(_T("XsExploit (to fast)"));
				return true;
			}
		}
		else if(m_uFastXSCounter)
			m_uFastXSCounter--;
	}

	return false;
}
#endif // ARGOS // NEO: NA END <-- Xanatos --

// NEO: MID - [ModID] -- Xanatos -->
void CUpDownClient::DetectMod() {
	if(StrStrI(m_strModVersion,_T("Neo Mule"))!=0)
		m_nModClient = MOD_NEO;
	else if(StrStrI(m_strModVersion,_T("MorphXT"))!=0)
		m_nModClient = MOD_MORPH;
	else if(StrStrI(m_strModVersion,_T("ScarAngel"))!=0)
		m_nModClient = MOD_SCAR;
	else if(StrStrI(m_strModVersion,_T("StulleMule"))!=0)
		m_nModClient = MOD_STULLE;
	else if(StrStrI(m_strModVersion,_T("MAXmod"))!=0)
		m_nModClient = MOD_MAXMOD;
	else if(StrStrI(m_strModVersion,_T("Xtreme"))!=0)
		m_nModClient = MOD_XTREME;
	else if(StrStrI(m_strModVersion,_T("EastShare"))!=0)
		m_nModClient = MOD_EASTSHARE;
	else if(StrStrI(m_strModVersion,_T("iONiX"))!=0)
		m_nModClient = MOD_IONIX;
	else if(StrStrI(m_strModVersion,_T("Cyrex2001"))!=0)
		m_nModClient = MOD_CYREX;
	else if(StrStrI(m_strModVersion,_T("NextEMF"))!=0)
		m_nModClient = MOD_NEXTEMF;
	else
		m_nModClient = MOD_UNKNOWN;
}
// NEO: MID END <-- Xanatos --

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
CString	CUpDownClient::GetCountryName(bool longName) const {
	return theApp.ip2country->GetCountryNameFromRef(m_structUserCountry,longName);
}

int CUpDownClient::GetCountryFlagIndex() const {
	return m_structUserCountry->FlagIndex;
}

void CUpDownClient::ResetIP2Country(uint32 m_dwIP){
	m_structUserCountry = theApp.ip2country->GetCountryFromIP((m_dwIP)?m_dwIP:m_dwUserIP);
}
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
void CUpDownClient::GetTooltipBaseInfo(CString &info)
{
	if(GetUserName())
		info.Format(GetResString(IDS_X_USERINFO), GetUserName(), m_nUserIDHybrid, GetResString(HasLowID() ? IDS_IDLOW : IDS_IDHIGH), DbgGetFullClientSoftVer());
	else
		info.Format(GetResString(IDS_X_USERINFO2));
	if (m_dwServerIP)
	{
		in_addr server;
		server.S_un.S_addr = m_dwServerIP;
		info.AppendFormat(GetResString(IDS_X_SERVER), ipstr(server), m_nServerPort);
	}
}

void CUpDownClient::GetTooltipDownloadInfo(CString &info, bool a4af, CPartFile* file, bool base) 
{
	if (IsEd2kClient())
	{
		if(base)
			GetTooltipBaseInfo(info);

		if (file == NULL)
			file = reqfile;

		info.AppendFormat(GetResString(IDS_X_LASTASK), CastSecondsToHM((::GetTickCount() - (DWORD)GetLastAskedTime())/1000));
		info.AppendFormat(GetResString(IDS_X_LASTASK2), CastSecondsToHM((::GetTickCount() - (DWORD)getLastTriedToConnectTime())/1000));
		info.AppendFormat(GetResString(IDS_X_NEXTASK),	CastSecondsToHM(GetTimeUntilReask(reqfile)/1000),CastSecondsToHM(GetTimeUntilReask(file)/1000)); // ZZ:DownloadManager

		info.AppendFormat(GetResString(IDS_X_SOURCEINFO), GetAskedCountDown());
		info.AppendFormat(GetResString(IDS_X_SOURCEINFO2), GetAvailablePartCount());
		if (reqfile)
		{
			if (a4af)
				info.AppendFormat(GetResString(IDS_X_ASKEDFAF), reqfile->GetFileName());
			
			// NEO: SCFS - [SmartClientFileStatus]
			if (CClientFileStatus* status = GetFileStatus(file,false))
				info.AppendFormat(GetResString(IDS_X_CLIENTSOURCENAME), status->GetFileName());
			// NEO: SCFS END
			
			// NEO: XC - [ExtendedComments]
			CPartFile* file = (CPartFile*)reqfile;
			for (POSITION pos = file->GetCommentList().GetHeadPosition(); pos != NULL;)
			{					
				KnownComment* cur_cs = file->GetCommentList().GetNext(pos);
				if (!md4cmp(cur_cs->m_achUserHash,GetUserHash()))
				{
					if (cur_cs->m_strComment.IsEmpty())
						info.AppendFormat(GetResString(IDS_X_CMT_NONE)); 	//No comment entered
					else
						info.AppendFormat(GetResString(IDS_X_CMT_READ), cur_cs->m_strComment);

					info.AppendFormat(GetResString(IDS_X_CMT_RATING), GetRateString(cur_cs->m_uRating));
					break;
				} 					
			}
			// NEO: XC END	
		}

		// ZZ:DownloadManager -->
		if (thePrefs.IsExtControlsEnabled() && !m_OtherRequests_list.IsEmpty()){
			info.AppendFormat(GetResString(IDS_X_A4AF));
			for (POSITION pos3 = m_OtherRequests_list.GetHeadPosition(); pos3!=NULL; m_OtherRequests_list.GetNext(pos3))
				info.AppendFormat(_T("<br>%s"),m_OtherRequests_list.GetAt(pos3)->GetFileName());
		}
		// ZZ:DownloadManager <--
	}
	else if (GetUserName())
		info.AppendFormat(GetResString(IDS_X_AVAIL_PARTS), GetUserName(), GetAvailablePartCount());
}

void CUpDownClient::GetTooltipUploadInfo(CString &info, bool base)
{
	if (base) GetTooltipBaseInfo(info);

	if(CKnownFile* file = theApp.sharedfiles->GetFileByID(requpfileid))
	{
		info.AppendFormat(GetResString(IDS_X_REQUESTED), file->GetFileName());
		info.AppendFormat(GetResString(IDS_X_FILESTATS_SESSION), file->statistic.GetAccepts(), file->statistic.GetRequests(), CastItoXBytes(file->statistic.GetTransferred(), false, false));
		info.AppendFormat(GetResString(IDS_X_FILESTATS_TOTAL), file->statistic.GetAllTimeAccepts(), file->statistic.GetAllTimeRequests(), CastItoXBytes(file->statistic.GetAllTimeTransferred(), false, false));
	}
	else
		info.AppendFormat(GetResString(IDS_REQ_UNKNOWNFILE));
}

void CUpDownClient::GetTooltipQueueInfo(CString &info, bool base)
{
	if (base) GetTooltipBaseInfo(info);
	if(CKnownFile* file = theApp.sharedfiles->GetFileByID(requpfileid))
		info.AppendFormat(GetResString(IDS_X_REQUESTED), file->GetFileName());

	info.AppendFormat(GetResString(IDS_X_QL_RATING), GetScore(false,false,true));
	info.AppendFormat(GetResString(IDS_X_QL_SCORE), GetScore(false));
	info.AppendFormat(GetResString(IDS_X_SOURCEINFO), GetAskedCount());
	info.AppendFormat(GetResString(IDS_X_QL_LASTSEEN), CastSecondsToHM((GetTickCount() - GetLastUpRequest())/1000));
	info.AppendFormat(GetResString(IDS_X_ENTEREDQUEUE), CastSecondsToHM((GetTickCount() - GetWaitStartTime())/1000));
}

void CUpDownClient::GetTooltipClientInfo(CString &info)
{
	GetTooltipBaseInfo(info);
	if(reqfile)
	{
		info.AppendFormat(GetResString(IDS_X_DOWNLOAD_INFO));
		info.AppendFormat(GetResString(IDS_X_REQUESTED), reqfile->GetFileName());
		GetTooltipDownloadInfo(info, false, false);
	}

	switch(GetUploadState())
	{
		case US_UPLOADING:
			info.AppendFormat(GetResString(IDS_X_UPLOAD_INFO));
			GetTooltipQueueInfo(info, false);
			break;
		case US_ONUPLOADQUEUE:
			info.AppendFormat(GetResString(IDS_X_QUEUE_INFO));
			GetTooltipUploadInfo(info, false);
			break;
	}

}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --