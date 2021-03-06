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
#include "Clientlist.h"
#include "OtherFunctions.h"
#include "PartFile.h"
#include "ListenSocket.h"
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
#include "Exceptions.h"
#include "ClientUDPSocket.h"
#include "shahashset.h"
#include "Log.h"
#include "BandWidthControl.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//IMPLEMENT_DYNAMIC(CClientException, CException) //Xman unused
//IMPLEMENT_DYNAMIC(CUpDownClient, CObject)
//Xman
// Maella -Upload Stop Reason-
// Maella -Download Stop Reason-
// Remark: static element are automaticaly initialized with zero
uint32 CUpDownClient::m_upStopReason[2][CUpDownClient::USR_EXCEPTION+1];
uint32 CUpDownClient::m_downStopReason[2][CUpDownClient::DSR_EXCEPTION+1];
// Maella end

DEF_THREAD_LOCAL(t_rng, SFMT, ())

CUpDownClient::CUpDownClient(CClientReqSocket* sender)//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
: m_upHistory_list(3),
m_downHistory_list(3)
// Maella end
{
	socket = sender;
	reqfile = NULL;
	Init();
}

CUpDownClient::CUpDownClient(CPartFile* in_reqfile, uint16 in_port, uint32 in_userid,uint32 in_serverip, uint16 in_serverport, bool ed2kID)//Xman // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
: m_upHistory_list(3),
m_downHistory_list(3)
// Maella end
{
	//Converting to the HybridID system.. The ED2K system didn't take into account of IP address ending in 0..
	//All IP addresses ending in 0 were assumed to be a lowID because of the calculations.
	socket = NULL;
	reqfile = in_reqfile;
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
}

void CUpDownClient::Init()
{
	m_abyUpPartStatusHidden = NULL; // morph4u :: SOTN :: Start

	//Xman Full Chunk
	upendsoon=false;
	
	//Xman filter clients with failed downloads
	m_faileddownloads=0;

	//Xman Xtreme Mod
	m_szFullUserIP=_T('?');
	m_cFailed = 0; //Xman Downloadmanager / Xtreme Mod // holds the failed connection attempts

	//Xman fix for startupload
	lastaction=0;

	//Xman fix for startupload (downloading side)
	protocolstepflag1=false;

	//Xman uploading problem client
	isupprob=false;

	//Xman askfordownload priority
	m_downloadpriority=1;

	m_nChatstate = MS_NONE;
	m_nKadState = KS_NONE;
	m_nUploadState = US_NONE;
	m_nDownloadState = DS_NONE;
	m_SecureIdentState = IS_UNAVAILABLE;
	m_nConnectingState = CCS_NONE;
	credits = NULL;
	m_bAddNextConnect = false;
	m_nUDPPort = 0;
	m_nKadPort = 0;
	m_nTransferredUp = 0;
	m_cAsked = 0;
	m_cDownAsked = 0;
	m_pszUsername = 0;
	m_nUserIDHybrid = 0;
	m_dwServerIP = 0;
	m_nServerPort = 0;
    m_iFileListRequested = 0;
	m_dwLastUpRequest = 0;
	m_bEmuleProtocol = false;
	m_bCompleteSource = false;
	m_bReaskPending = false;
	m_bUDPPending = false;
	m_byEmuleVersion = 0;
	m_nUserPort = 0;
	m_nPartCount = 0;
	m_nUpPartCount = 0;
	m_abyPartStatus = 0;
	m_abyUpPartStatus = 0;
	m_dwUploadTime = 0;
	m_nTransferredDown = 0;
	m_dwLastBlockReceived = 0;
	m_byDataCompVer = 0;
	m_byUDPVer = 0;
	m_bySourceExchange1Ver = 0;
	m_byAcceptCommentVer = 0;
	m_byExtendedRequestsVer = 0;
	m_nRemoteQueueRank = 0;
	m_dwLastSourceRequest = 0;
	m_dwLastSourceAnswer = 0;
	m_dwLastAskedForSources = 0;
	m_byCompatibleClient = 0;
	m_nSourceFrom = SF_SERVER;
	m_bIsHybrid = false;
	m_bIsML=false;
	m_fIsSpammer = 0;
	m_nCurSessionUp = 0;
	m_nCurSessionDown = 0;
	m_nCurSessionPayloadDown = 0;
	m_clientSoft=SO_UNKNOWN;
	m_bRemoteQueueFull = false;
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
	//Xman
	//remark: in most cases we use the defaultstrcut (ip=0)
	//we could use the right IP from begining by moving this method to the constructor
	//disadvantage: we create and immediately delete many clients, always searching the
	//country costs too much time
	//EastShare Start - added by AndCycle, IP to Country
	m_structUserLocation = theApp.ip2country->GetLocationFromIP(m_dwUserIP); // X: [IP2L] - [IP2Location]
	//EastShare End - added by AndCycle, IP to Country

        m_fHashsetRequestingAICH = 0;
	m_fHashsetRequestingMD4 = 0;
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
	m_nUpCompleteSourcesCount= 0;
	m_bTransferredDownMini = false;
    m_addedPayloadQueueSession = 0;
    m_nCurQueueSessionPayloadUp = 0; // PENDING: Is this necessary? ResetSessionUp()...
    m_lastRefreshedULDisplay = ::GetTickCount();
	m_bGPLEvildoer = false;
	m_bHelloAnswerPending = false;
	m_bACATHelloAnswerPending = false;	//zz_fly :: Client is always highid if we are connecting to them - by Enig123, idea from ACAT
	m_fNoViewSharedFiles = 0;
	m_bMultiPacket = 0;
	md4clr(requpfileid);
	m_nTotalUDPPackets = 0;
	m_nFailedUDPPackets = 0;
	m_fPeerCache = 0;
	m_fNeedOurPublicIP = 0;
    m_random_update_wait = t_rng->getUInt32()/(RAND32_MAX/1000);
	m_fQueueRankPending = 0;
	m_fUnaskQueueRankRecv = 0;
	m_fFailedFileIdReqs = 0;
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
	m_fDirectUDPCallback = 0;
	m_fSupportsFileIdent = 0;

	//Xman -----------------
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_displayUpDatarateCounter = 0;
	m_displayDownDatarateCounter = 0;

	m_nUpDatarate = 0;
	m_nUpDatarate10 = 0;
	m_nUpDatarateMeasure = 0;

	m_nDownDatarate = 0;
	m_nDownDatarate10 = 0;
	m_nDownDatarateMeasure = 0;
	// Maella end

	// Maella -Unnecessary Protocol Overload-
	CalculateJitteredFileReaskTime(false); //Xman 5.1 
	m_dwLastAskedTime = 0;
	m_dwLastUDPReaskTime = 0;
	m_dwNextTCPAskedTime = 0;
	// Maella end

	//Xman Xtreme Downloadmanager
	droptime=0;
	isduringswap=false;
	enterqueuetime=0;
	oldQR=0; //Xman diffQR

	//Xman end
	// Maella -Extended clean-up II-
	m_lastCleanUpCheck = GetTickCount(); //Note: this feature is important for Xtreme Downloadmanager
	// Maella end

	// X: [DSRB] - [Dynamic Send and Receive Buffer]
	sendbuffersize = 0;
	recvbuffersize = 0;

	filedata = NULL; // SiRoB: ReadBlockFromFileThread

	//Xman client percentage
	hiscompletedparts_percent_up=-1;
	hiscompletedparts_percent_down=-1;
	//Xman end

	//Xman end --------------
}

CUpDownClient::~CUpDownClient(){
// morph4u :: SOTN :: Start
	delete[] m_abyUpPartStatusHidden;
	m_abyUpPartStatusHidden = NULL;
// morph4u :: SOTN :: End
	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHRecoveryHashSet::ClientAICHRequestFailed(this);
	}

	ASSERT( m_nConnectingState == CCS_NONE || !CemuleDlg::IsRunning() );
	theApp.clientlist->RemoveClient(this, _T("Destructing client object"));

	if (socket){
		socket->client = 0;
		socket->Safe_Delete();
	}

	free(m_pszUsername);

	if (m_abyPartStatus){ //from MorphXT
		delete[] m_abyPartStatus;
		m_abyPartStatus = NULL;
	}
	if (m_abyUpPartStatus){ //from MorphXT
		delete[] m_abyUpPartStatus;
		m_abyUpPartStatus = NULL;
	}
	ClearUploadBlockRequests();

	for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;)
		delete m_DownloadBlocks_list.GetNext(pos);
	
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


	delete m_pReqFileAICHHash;

	//Xman --------------------
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_upHistory_list.RemoveAll();
	m_downHistory_list.RemoveAll();
	
	// Maella -Unnecessary Protocol Overload-
	m_partStatusMap.clear();
	//Xman end

	//Xman Extened credit- table-arragement
	if(Credits())
	{
		Credits()->SetLastSeen(); //ensure we keep the credits at least 6 hours in memory, without this line our LastSeen can be outdated if we did only UDP
		if(Credits()->GetDownloadedTotal()==0 && Credits()->GetUploadedTotal()==0) //just to save some CPU-cycles, a second test is done at credits
			Credits()->MarkToDelete();
	}
	//Xman end
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
	m_nClientVersion = 0;
	m_fSharedDirectories = 0;
	m_bMultiPacket = 0;
	m_fPeerCache = 0;
	m_byKadVersion = 0;
	m_fSupportsLargeFiles = 0;
	m_fExtMultiPacket = 0;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
	m_fSupportsSourceEx2 = 0;
	m_fDirectUDPCallback = 0;
	m_fSupportsFileIdent = 0;
	m_bGPLEvildoer = false; //>>> WiZaRd::FiX?
}

bool CUpDownClient::ProcessHelloPacket(const uchar* pachPacket, uint32 nSize)
{
	CSafeMemFile data(pachPacket, nSize);
	data.ReadUInt8(); // read size of userhash
	// reset all client properties; a client may not send a particular emule tag any longer
	//ClearHelloProperties(); // X: move to ProcessHelloTypePacket
	return ProcessHelloTypePacket(&data);
}

bool CUpDownClient::ProcessHelloAnswer(const uchar* pachPacket, uint32 nSize)
{
	CSafeMemFile data(pachPacket, nSize);
	bool bIsMule = ProcessHelloTypePacket(&data);
	m_bHelloAnswerPending = false;
	m_bACATHelloAnswerPending = false;	//zz_fly :: Client is always highid if we are connecting to them - by Enig123, idea from ACAT
	return bIsMule;
}

bool CUpDownClient::ProcessHelloTypePacket(CSafeMemFile* data)
{
	ClearHelloProperties();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	bool bDbgInfo = thePrefs.GetUseDebugDevice();
	m_strHelloInfo.Empty();
#endif

	// clear hello properties which can be changed _only_ on receiving OP_Hello/OP_HelloAnswer
	m_bIsHybrid = false;
	m_bIsML = false;
	m_fNoViewSharedFiles = 0;
	m_bUnicodeSupport = false;

	data->ReadHash16(m_achUserHash);
	m_nUserIDHybrid = data->ReadUInt32();
	uint16 nUserPort = data->ReadUInt16(); // hmm clientport is sent twice - why?
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (bDbgInfo){
		m_strHelloInfo.AppendFormat(_T("Hash=%s (%s)"), md4str(m_achUserHash), DbgGetHashTypeString(m_achUserHash));
		m_strHelloInfo.AppendFormat(_T("  UserID=%u (%s)"), m_nUserIDHybrid, ipstr(m_nUserIDHybrid));
		m_strHelloInfo.AppendFormat(_T("  Port=%u"), nUserPort);
	}
#endif
	
	uint_ptr dwEmuleTags = 0;
	bool bPrTag = false;
	size_t tagcount = data->ReadUInt32();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("  Tags=%u"), tagcount);
#endif
	for (size_t i = 0; i < tagcount; i++)
	{
		CTag temptag(data, true);
		switch (temptag.GetNameID())
		{
			case CT_NAME:
				if (temptag.IsStr()) {
					free(m_pszUsername);
					m_pszUsername = _tcsdup(temptag.GetStr());
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
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
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case CT_VERSION:
				if (temptag.IsInt()) {
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  Version=%u"), temptag.GetInt());
#endif
					m_nClientVersion = temptag.GetInt();
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case CT_PORT:
				if (temptag.IsInt()) {
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  Port=%u"), temptag.GetInt());
#endif
					nUserPort = (uint16)temptag.GetInt();
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case CT_MOD_VERSION:
				if (temptag.IsStr())
					m_strModVersion = temptag.GetStr();
				else if (temptag.IsInt())
					m_strModVersion.Format(_T("ModID=%u"), temptag.GetInt());
				else
					m_strModVersion = _T("ModID=<Unknown>");
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
#endif
				CheckForGPLEvilDoer();
				break;

			case CT_EMULE_UDPPORTS:
				// 16 KAD Port
				// 16 UDP Port
				if (temptag.IsInt()) {
					m_nKadPort = (uint16)(temptag.GetInt() >> 16);
					m_nUDPPort = (uint16)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  KadPort=%u  UDPPort=%u"), m_nKadPort, m_nUDPPort);
#endif
					dwEmuleTags |= 1;
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case CT_EMULE_BUDDYUDP:
				// 16 --Reserved for future use--
				// 16 BUDDY Port
				if (temptag.IsInt()) {
					m_nBuddyPort = (uint16)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyPort=%u"), m_nBuddyPort);
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case CT_EMULE_BUDDYIP:
				// 32 BUDDY IP
				if (temptag.IsInt()) {
					m_nBuddyIP = temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  BuddyIP=%s"), ipstr(m_nBuddyIP));
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
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
				//	1 MultiPacket - deprecated with FileIdentifiers/MultipacketExt2
				//  1 Preview
				if (temptag.IsInt()) {
					uint_ptr uOption = temptag.GetInt();
					m_fSupportsAICH			= (uOption >> 29) & 0x07;
					m_bUnicodeSupport		= (uOption >> 28) & 0x01;
					m_byUDPVer				= (uint8)((uOption >> 24) & 0x0f);
					m_byDataCompVer			= (uint8)((uOption >> 20) & 0x0f);
					m_bySupportSecIdent		= (uint8)((uOption >> 16) & 0x0f);
					m_bySourceExchange1Ver	= (uint8)((uOption >> 12) & 0x0f);
					m_byExtendedRequestsVer	= (uint8)((uOption >>  8) & 0x0f);
					m_byAcceptCommentVer	= (uint8)((uOption >>  4) & 0x0f);
					m_fPeerCache			= (uOption >>  3) & 0x01;
					m_fNoViewSharedFiles	= (uOption >>  2) & 0x01;
					m_bMultiPacket			= (uOption >>  1) & 0x01;
					dwEmuleTags |= 2;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo) {
						m_strHelloInfo.AppendFormat(_T("\n  PeerCache=%u  UDPVer=%u  DataComp=%u  SecIdent=%u  SrcExchg=%u")
							_T("  ExtReq=%u  Commnt=%u  NoViewFiles=%u  Unicode=%u"), 
							m_fPeerCache, m_byUDPVer, m_byDataCompVer, m_bySupportSecIdent, m_bySourceExchange1Ver, 
							m_byExtendedRequestsVer, m_byAcceptCommentVer, m_fNoViewSharedFiles, m_bUnicodeSupport);
//Xman
#ifdef LOGTAG
						m_strHelloInfo.AppendFormat(_T("\n m_fSupportsAICH=%u"), m_fSupportsAICH);
#endif
//Xman end	
					}
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case CT_EMULE_MISCOPTIONS2:
				//	18 Reserved
				//   1 Supports new FileIdentifiers/MultipacketExt2
				//   1 Direct UDP Callback supported and available
				//	 1 Supports ChatCaptchas
				//	 1 Supports SourceExachnge2 Packets, ignores SX1 Packet Version
				//	 1 Requires CryptLayer
				//	 1 Requests CryptLayer
				//	 1 Supports CryptLayer
				//	 1 Reserved (ModBit)
				//   1 Ext Multipacket (Hash+Size instead of Hash) - deprecated with FileIdentifiers/MultipacketExt2
				//   1 Large Files (includes support for 64bit tags)
				//   4 Kad Version - will go up to version 15 only (may need to add another field at some point in the future)
				if (temptag.IsInt()) {
					uint_ptr uOption = temptag.GetInt();
					m_fSupportsFileIdent	= (uOption >>  13) & 0x01;
					m_fDirectUDPCallback	= (uOption >>  12) & 0x01;
					m_fSupportsSourceEx2	= (uOption >>  10) & 0x01;
					m_fRequiresCryptLayer	= (uOption >>  9) & 0x01;
					m_fRequestsCryptLayer	= (uOption >>  8) & 0x01;
					m_fSupportsCryptLayer	= (uOption >>  7) & 0x01;
					// reserved 1
					m_fExtMultiPacket		= (uOption >>  5) & 0x01;
					m_fSupportsLargeFiles   = (uOption >>  4) & 0x01;
					m_byKadVersion			= (uint8)((uOption >>  0) & 0x0f);
					dwEmuleTags |= 8;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  KadVersion=%u, LargeFiles=%u ExtMultiPacket=%u CryptLayerSupport=%u CryptLayerRequest=%u CryptLayerRequires=%u m_fSupportsSourceEx2=%u DirectUDPCallback=%u"), m_byKadVersion, m_fSupportsLargeFiles, m_fExtMultiPacket, m_fSupportsCryptLayer, m_fRequestsCryptLayer, m_fRequiresCryptLayer, m_fSupportsSourceEx2, m_fDirectUDPCallback);
#endif
					m_fRequestsCryptLayer &= m_fSupportsCryptLayer;
					m_fRequiresCryptLayer &= m_fRequestsCryptLayer;

				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  ClientVer=%u.%u.%u.%u  Comptbl=%u"), (m_nClientVersion >> 17) & 0x7f, (m_nClientVersion >> 10) & 0x7f, (m_nClientVersion >> 7) & 0x07, m_nClientVersion & 0x7f, m_byCompatibleClient);
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			default:
				// Since eDonkeyHybrid 1.3 is no longer sending the additional Int32 at the end of the Hello packet,
				// we use the "pr=1" tag to determine them.
				if (temptag.GetName() && temptag.GetName()[0]=='p' && temptag.GetName()[1]=='r') {
					bPrTag = true;
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkTag=%s"), temptag.GetFullInfo());
#endif
				break;
		}
	}
	m_nUserPort = nUserPort;
	m_dwServerIP = data->ReadUInt32();
	m_nServerPort = data->ReadUInt16();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (bDbgInfo)
		m_strHelloInfo.AppendFormat(_T("\n  Server=%s:%u"), ipstr(m_dwServerIP), m_nServerPort);
#endif

	// Check for additional data in Hello packet to determine client's software version.
	//
	// *) eDonkeyHybrid 0.40 - 1.2 sends an additional Int32. (Since 1.3 they don't send it any longer.)
	// *) MLdonkey sends an additional Int32
	//
	if (data->GetLength() - data->GetPosition() == sizeof(uint32)){
		uint32 test = data->ReadUInt32();
		if (test == 'KDLM'){
			m_bIsML = true;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (bDbgInfo)
				m_strHelloInfo += _T("\n  ***AddData: \"MLDK\"");
#endif
		}
		else{
			m_bIsHybrid = true;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (bDbgInfo)
				m_strHelloInfo.AppendFormat(_T("\n  ***AddData: uint32=%u (0x%08x)"), test, test);
#endif
		}
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	else if (bDbgInfo && data->GetPosition() < data->GetLength()){
		uint_ptr uAddHelloDataSize = (uint_ptr)(data->GetLength() - data->GetPosition());
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
#endif

	//Xman IP to Country
	uint32 oldIP=m_dwUserIP;
	//Xman end

	SOCKADDR_IN sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	socket->GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	SetIP(sockAddr.sin_addr.S_un.S_addr);

	//Xman IP to Country
	//only search if changed, it's cheaper
	//EastShare Start - added by AndCycle, IP to Country
	if(oldIP!=m_dwUserIP)
		m_structUserLocation = theApp.ip2country->GetLocationFromIP(m_dwUserIP);// X: [IP2L] - [IP2Location]
	//EastShare End - added by AndCycle, IP to Country

	//(a)If this is a highID user, store the ID in the Hybrid format.
	//(b)Some older clients will not send a ID, these client are HighID users that are not connected to a server.
	//(c)Kad users with a *.*.*.0 IPs will look like a lowID user they are actually a highID user.. They can be detected easily
	//because they will send a ID that is the same as their IP..
	//zz_fly :: Client is always highid if we are connecting to them - by Enig123, idea from ACAT
	//note: if we can receive his hello answer, whatever the situation is, he must be a HighID. 
	//		if we and he send hello packet concurrent, only one instance will survive. this fix will not cause trouble in this case.
	/*
	if(!HasLowID() || m_nUserIDHybrid == 0 || m_nUserIDHybrid == m_dwUserIP ) 
	*/
	if(!HasLowID() || m_nUserIDHybrid == 0 || m_nUserIDHybrid == m_dwUserIP || m_bACATHelloAnswerPending)
	//zz_fly :: end
		m_nUserIDHybrid = ntohl(m_dwUserIP);

	if (credits == NULL){
		//Xman Extened credit- table-arragement
		//if (!theApp.clientlist->ComparePriorUserhash(m_dwUserIP, m_nUserPort, pFoundCredits)){
		if (!theApp.clientlist->ComparePriorUserhash(m_dwUserIP, m_nUserPort, m_achUserHash)){
			//if (thePrefs.GetLogBannedClients())
			//Ban();
			Ban(_T("Userhash changed (Found in TrackedClientsList)"));
		}
		else if (GetUploadState() != US_BANNED) //Enig123
			credits = theApp.clientcredits->GetCredit(m_achUserHash);
	}
	//else if (credits != pFoundCredits){
	else if (md4cmp(credits->GetKey(), m_achUserHash)){ //Enig123
		//Xman Extened credit- table-arragement
		credits->SetLastSeen(); //ensure to keep it at least 5 hours
		if(credits->GetUploadedTotal()==0 && credits->GetDownloadedTotal()==0)
			credits->MarkToDelete(); //check also if the old hash is used by an other client
		//Xman end
		// userhash change ok, however two hours "waittime" before it can be used
		credits = NULL;
		//if (thePrefs.GetLogBannedClients())
		//Ban();
		Ban(_T("Userhash changed"));
	}
	else {
		credits->SetLastSeen(); //Enig123::refresh is neccessary here to avoid wrong judgement
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

	if (m_bIsHybrid)
		m_fSharedDirectories = 1;

	if (thePrefs.GetVerbose() && GetServerIP() == INADDR_NONE)
		AddDebugLogLine(false, _T("Received invalid server IP %s from %s"), ipstr(GetServerIP()), DbgGetClientInfo());

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
	Packet* packet = new Packet(&data, OP_EDONKEYPROT, OP_HELLO);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__Hello", this);
#endif
	theStats.AddUpDataOverheadOther(packet->size);
	SendPacket(packet, true);

	m_bHelloAnswerPending = true;
	m_bACATHelloAnswerPending = true;	//zz_fly :: Client is always highid if we are connecting to them - by Enig123, idea from ACAT
}

void CUpDownClient::SendMuleInfoPacket(bool bAnswer){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(128);
	data.WriteUInt8((uint8)theApp.m_uCurVersionShort);
	data.WriteUInt8(EMULE_PROTOCOL);
	data.WriteUInt32(6); // nr. of tags
	CTag tag(ET_COMPRESSION,1);
	tag.WriteTagToFile(&data);
	CTag tag2(ET_UDPVER,4);
	tag2.WriteTagToFile(&data);
	CTag tag3(ET_UDPPORT,thePrefs.GetUDPPort());
	tag3.WriteTagToFile(&data);
	CTag tag4(ET_SOURCEEXCHANGE,3);
	tag4.WriteTagToFile(&data);
	CTag tag5(ET_EXTENDEDREQUEST,2);
	tag5.WriteTagToFile(&data);

	uint32 dwTagValue = (theApp.clientcredits->CryptoAvailable() ? 3 : 0);
	if (thePrefs.CanSeeShares() != vsfaNobody) // set 'Preview supported' only if 'View Shared Files' allowed
		dwTagValue |= 128;
	CTag tag6(ET_FEATURES, dwTagValue);
	tag6.WriteTagToFile(&data);


	Packet* packet = new Packet(&data, OP_EMULEPROT, !bAnswer?OP_EMULEINFO:OP_EMULEINFOANSWER);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend(!bAnswer ? "OP__EmuleInfo" : "OP__EmuleInfoAnswer", this);
#endif
	theStats.AddUpDataOverheadOther(packet->size);
	SendPacket(packet, true);
}

void CUpDownClient::ProcessMuleInfoPacket(const uchar* pachPacket, uint32 nSize)
{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	bool bDbgInfo = thePrefs.GetUseDebugDevice();
	m_strMuleInfo.Empty();
#endif
	CSafeMemFile data(pachPacket, nSize);
	m_byCompatibleClient = 0;
	m_byEmuleVersion = data.ReadUInt8();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("EmuleVer=0x%x"), (UINT)m_byEmuleVersion);
#endif
	if (m_byEmuleVersion == 0x2B)
		m_byEmuleVersion = 0x22;
	uint8 protversion = data.ReadUInt8();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("  ProtVer=%u"), (UINT)protversion);
#endif

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

	size_t tagcount = data.ReadUInt32();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (bDbgInfo)
		m_strMuleInfo.AppendFormat(_T("  Tags=%u"), tagcount);
#endif
	for (size_t i = 0; i < tagcount; i++)
	{
		CTag temptag(&data, false);
		switch (temptag.GetNameID())
		{
			case ET_COMPRESSION:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: data compression version
				if (temptag.IsInt()) {
					m_byDataCompVer = (uint8)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Compr=%u"), (UINT)temptag.GetInt());
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_UDPPORT:
				// Bits 31-16: 0 - reserved
				// Bits 15- 0: UDP port
				if (temptag.IsInt()) {
					m_nUDPPort = (uint16)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  UDPPort=%u"), (UINT)temptag.GetInt());
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_UDPVER:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: UDP protocol version
				if (temptag.IsInt()) {
					m_byUDPVer = (uint8)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  UDPVer=%u"), (UINT)temptag.GetInt());
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_SOURCEEXCHANGE:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: source exchange protocol version
				if (temptag.IsInt()) {
					m_bySourceExchange1Ver = (uint8)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  SrcExch=%u"), (UINT)temptag.GetInt());
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_COMMENTS:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: comments version
				if (temptag.IsInt()) {
					m_byAcceptCommentVer = (uint8)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Commnts=%u"), (UINT)temptag.GetInt());
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_EXTENDEDREQUEST:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: extended requests version
				if (temptag.IsInt()) {
					m_byExtendedRequestsVer = (uint8)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  ExtReq=%u"), (UINT)temptag.GetInt());
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_COMPATIBLECLIENT:
				// Bits 31- 8: 0 - reserved
				// Bits  7- 0: compatible client ID
				if (temptag.IsInt()) {
					m_byCompatibleClient = (uint8)temptag.GetInt();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  Comptbl=%u"), (UINT)temptag.GetInt());
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_FEATURES:
				// Bits 31- 8: 0 - reserved
				// Bit	    7: Preview
				// Bit   6- 0: secure identification
				if (temptag.IsInt()) {
					m_bySupportSecIdent = (uint8)((temptag.GetInt()) & 3);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (bDbgInfo)
						m_strMuleInfo.AppendFormat(_T("\n  SecIdent=%u"), m_bySupportSecIdent);
#endif
				}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				else if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
#endif
				break;

			case ET_MOD_VERSION:
				if (temptag.IsStr())
					m_strModVersion = temptag.GetStr();
				else if (temptag.IsInt())
					m_strModVersion.Format(_T("ModID=%u"), temptag.GetInt());
				else
					m_strModVersion = _T("ModID=<Unknown>");
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
#endif
				CheckForGPLEvilDoer();
				break;
			
			default:
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ***UnkTag=%s"), temptag.GetFullInfo());
#endif
				break;
		}
	}
	if (m_byDataCompVer == 0) {
		m_bySourceExchange1Ver = 0;
		m_byExtendedRequestsVer = 0;
		m_byAcceptCommentVer = 0;
		m_nUDPPort = 0;
	}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (bDbgInfo && data.GetPosition() < data.GetLength()) {
		m_strMuleInfo.AppendFormat(_T("\n  ***AddData: %u bytes"), data.GetLength() - data.GetPosition());
	}
#endif

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
	Packet* packet = new Packet(&data, OP_EDONKEYPROT, OP_HELLOANSWER);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__HelloAnswer", this);
#endif
	theStats.AddUpDataOverheadOther(packet->size);

	// Servers send a FIN right in the data packet on check connection, so we need to force the response immediate
	bool bForceSend = theApp.serverconnect->AwaitingTestFromIP(GetConnectIP());
	socket->SendPacket(packet, true, true, 0, bForceSend);

	m_bHelloAnswerPending = false;
}

void CUpDownClient::SendHelloTypePacket(CSafeMemFile* data)
{
	// morph4u :: EmulateOthers :: Start
	/*
	data->WriteHash16(thePrefs.GetUserHash());
	*/
uchar hash[16]; 
memcpy(hash,thePrefs.GetUserHash(), 16); 
	if (GetClientSoft() == SO_MLDONKEY)
{ 
		if(GetHashType() == SO_MLDONKEY)
{ 
			hash[5] = 'M'; //WiZaRd::Proper Hash Fake :P
			hash[14] = 'L'; //WiZaRd::Proper Hash Fake :P
		}
} 
	else if ((GetClientSoft() == SO_EDONKEY) ||
			 (GetClientSoft() == SO_EDONKEYHYBRID))
{ 
		uint8 random = (uint8)(rand()%_UI8_MAX); //Spike2, avoid C4244
		hash[5] = random == 14 ? random+1 : random; //WiZaRd::Avoid eMule Hash
		random = (uint8)(rand()%_UI8_MAX); //Spike2, avoid C4244
		hash[14] = random == 111 ? random+1 : random; //WiZaRd::Avoid eMule Hash
}
data->WriteHash16(hash);
// morph4u :: EmulateOthers :: End

	data->WriteUInt32(theApp.GetID());
	data->WriteUInt16(thePrefs.GetPort());

	uint32 tagcount = 6;

	if( theApp.clientlist->GetBuddy() && theApp.IsFirewalled() )
		tagcount += 2;

	data->WriteUInt32(tagcount);

	// eD2K Name

	// TODO implement multi language website which informs users of the effects of bad mods
	CTag tagName(CT_NAME, (!m_bGPLEvildoer) ? DEFAULT_NICK/*thePrefs.GetUserNick()*/ : _T("Please use a GPL-conform version of eMule") );
	tagName.WriteTagToFile(data, utf8strRaw);

	// morph4u :: EmulateOthers :: Start
	/*
	CTag tagVersion(CT_VERSION,EDONKEYVERSION);
	tagVersion.WriteTagToFile(data);
	*/
	if (GetClientSoft() == SO_SHAREAZA)
	{
		CTag tagVersion(CT_VERSION,SHAREAZAEMUVERSION);
		tagVersion.WriteTagToFile(data);
	}
	else 
	{
	// eD2K Version
	CTag tagVersion(CT_VERSION,EDONKEYVERSION);
	tagVersion.WriteTagToFile(data);
	}
	// morph4u :: EmulateOthers :: End

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
//>>> WiZaRd::XS-Workaround
	// ***
	// deprecated - will be set back to 3 with the next release (to allow the new version to spread first),
	// due to a bug in earlier eMule version. Use SupportsSourceEx2 and new opcodes instead
	//const UINT uSourceExchange1Ver	= 4;
	// ***
	const UINT uSourceExchange1Ver	= (m_bySourceExchange1Ver != 0 && m_bySourceExchange1Ver < 4) ? m_bySourceExchange1Ver : 4;
//<<< WiZaRd::XS-Workaround
	const UINT uExtendedRequestsVer	= 2;
	const UINT uNoViewSharedFiles	= (thePrefs.CanSeeShares() == vsfaNobody) ? 1 : 0; // for backward compatibility this has to be a 'negative' flag
	const UINT uMultiPacket			= 1;

	const UINT uPeerCache			= 1;// X: pretend to support peercache and accept PublicIPRequest
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
				(uPeerCache				<<  3) |
				(uNoViewSharedFiles		<<  2) |
				(uMultiPacket			<<  1)
				);
	tagMisOptions1.WriteTagToFile(data);

	// eMule Misc. Options #2
	const UINT uKadVersion			= KADEMLIA_VERSION;
	const UINT uSupportLargeFiles	= 1;
	const UINT uExtMultiPacket		= 1;
	const UINT uSupportsCryptLayer	= thePrefs.IsClientCryptLayerSupported() ? 1 : 0;
	const UINT uRequestsCryptLayer	= thePrefs.IsClientCryptLayerRequested() ? 1 : 0;
	const UINT uRequiresCryptLayer	= thePrefs.IsClientCryptLayerRequired() ? 1 : 0;
	const UINT uSupportsSourceEx2	= 1;
	// direct callback is only possible if connected to kad, tcp firewalled and verified UDP open (for example on a full cone NAT)
	const UINT uDirectUDPCallback	= (Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsFirewalled()
		&& !Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) && Kademlia::CUDPFirewallTester::IsVerified()) ? 1 : 0;
	const UINT uFileIdentifiers		= 1;

	CTag tagMisOptions2(CT_EMULE_MISCOPTIONS2, 
//				(RESERVED				     )
				(uFileIdentifiers		<< 13) |
				(uDirectUDPCallback		<< 12) |
				(uSupportsSourceEx2		<< 10) |
				(uRequiresCryptLayer	<<  9) |
				(uRequestsCryptLayer	<<  8) |
				(uSupportsCryptLayer	<<  7) |
				(uExtMultiPacket		<<  5) |
				(uSupportLargeFiles		<<  4) |
				(uKadVersion			<<  0) 
				);
	tagMisOptions2.WriteTagToFile(data);

	// eMule Version
	// morph4u :: EmulateOthers :: Start
	/*
	CTag tagMuleVersion(CT_EMULE_VERSION, 
				//(uCompatibleClientID		<< 24) |
				(CemuleApp::m_nVersionMjr	<< 17) |
				(CemuleApp::m_nVersionMin	<< 10) |
				(CemuleApp::m_nVersionUpd	<<  7) 
//				(RESERVED			     ) 
				);
	tagMuleVersion.WriteTagToFile(data);
	*/
	if (GetClientSoft() == SO_SHAREAZA)
	{
		CTag tagMuleVersion(CT_EMULE_VERSION,
				(SO_SHAREAZA				<< 24) |
				(2							<< 17) |
				(2							<< 10) |
				(5 							<<  7) |
				(0								 )
				);
		tagMuleVersion.WriteTagToFile(data);
	}
	else if (GetClientSoft() == SO_LPHANT)
	{
		CTag tagMuleVersion(CT_EMULE_VERSION,
				(SO_LPHANT					<< 24) |
				(2							<< 17) |
				(9							<< 10) |
				(0							<<  7)
				);
		tagMuleVersion.WriteTagToFile(data);
		}
	else if (GetClientSoft() == SO_MLDONKEY)
	{
		CTag tagMuleVersion(CT_EMULE_VERSION,
				(SO_MLDONKEY				<< 24) |
				(2							<< 17) |
				(7							<< 10) |
				(6							<<  7)
				);
		tagMuleVersion.WriteTagToFile(data);
		}
	else if (GetClientSoft() == SO_EDONKEY)
	{
		CTag tagMuleVersion(CT_EMULE_VERSION,
				(SO_EDONKEY					<< 24) |
				(10400						<< 17)
				);
		tagMuleVersion.WriteTagToFile(data);
		}
	else if (GetClientSoft() == SO_EDONKEYHYBRID)
	{
		CTag tagMuleVersion(CT_EMULE_VERSION,
				(SO_EDONKEYHYBRID			<< 24) |
				(10405						<< 17)
				);
		tagMuleVersion.WriteTagToFile(data);
	}
	else 
	{
	CTag tagMuleVersion(CT_EMULE_VERSION, 
				//(uCompatibleClientID		<< 24) |
				(CemuleApp::m_nVersionMjr	<< 17) |
				(CemuleApp::m_nVersionMin	<< 10) |
				(CemuleApp::m_nVersionUpd	<<  7) 
//				(RESERVED			     ) 
				);
	tagMuleVersion.WriteTagToFile(data);
	}
	// morph4u :: EmulateOthers :: End

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

bool CUpDownClient::Disconnected(LPCTSTR pszReason, bool bFromSocket, UpStopReason reason) // Maella -Upload Stop Reason-
{
	ASSERT( theApp.clientlist->IsValidClient(this) );
	// TODO LOGREMOVE
	if (m_nConnectingState == CCS_DIRECTCALLBACK)
		DebugLog(_T("Direct Callback failed - %s"), DbgGetClientInfo());
	
	if (Kademlia::CKademlia::IsRunning()){ // X: [BF] - [Bug Fix]
	if (GetKadState() == KS_QUEUED_FWCHECK_UDP || GetKadState() == KS_CONNECTING_FWCHECK_UDP)
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, true, ntohl(GetConnectIP()), 0); // inform the tester that this test was cancelled
	else if (GetKadState() == KS_FWCHECK_UDP)
		Kademlia::CUDPFirewallTester::SetUDPFWCheckResult(false, false, ntohl(GetConnectIP()), 0); // inform the tester that this test has failed
	else if (GetKadState() == KS_CONNECTED_BUDDY)
		DebugLogWarning(_T("Buddy client disconnected - %s, %s"), pszReason, DbgGetClientInfo());
	}
	//If this is a KAD client object, just delete it!
	SetKadState(KS_NONE);

	/*if (GetUploadState() == US_UPLOADING)
	{
		// sets US_NONE
		theApp.uploadqueue->RemoveFromUploadQueue(this, CString(_T("CUpDownClient::Disconnected: ")) + pszReason , reason); // Maella -Upload Stop Reason-
	}*/

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
		//Xman remark: the only reason can be a socket-error/timeout
		SetDownloadState(DS_ONQUEUE, CString(_T("Disconnected: ")) + pszReason, CUpDownClient::DSR_SOCKET);	// Maella -Download Stop Reason-
	}
	else{
		// ensure that all possible block requests are removed from the partfile
		ClearDownloadBlockRequests();
		/* //Xman Code Imrpovement moved down
		if (GetDownloadState() == DS_CONNECTED){ // successfully connected, but probably didn't responsed to our filerequest
			theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
			theApp.downloadqueue->RemoveSource(this);
	    }*/
	}

	// we had still an AICH request pending, handle it
	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHRecoveryHashSet::ClientAICHRequestFailed(this);
	}

	// The remote client does not have to answer with OP_HASHSETANSWER *immediatly* 
	// after we've sent OP_HASHSETREQUEST. It may occure that a (buggy) remote client 
	// is sending use another OP_FILESTATUS which would let us change to DL-state to DS_ONQUEUE.
	if (m_fHashsetRequestingMD4 && (reqfile != NULL))
        reqfile->m_bMD4HashsetNeeded = true;
	if (m_fHashsetRequestingAICH && (reqfile != NULL))
        reqfile->SetAICHHashSetNeeded(true);

	//offical move up
    if (m_iFileListRequested){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_SHAREDFILES_FAILED), GetUserName());
        m_iFileListRequested = 0;
	}

	//offical move up end
	ASSERT( theApp.clientlist->IsValidClient(this) );

	//check if this client is needed in any way, if not delete it
	bool bDelete = true; //Xman
	bool checkdownstate = m_nUploadState!=US_BANNED; //Code-Improvement
	switch(m_nUploadState){
		case US_UPLOADING:
			theApp.uploadqueue->RemoveFromUploadQueue(this, CString(_T("CUpDownClient::Disconnected: ")) + pszReason , reason); // Maella -Upload Stop Reason-
			break;
		case US_CONNECTING:
		{
			//Xman uploading problem client
			theApp.uploadqueue->RemoveFromUploadQueue(this,pszReason ,CUpDownClient::USR_SOCKET );
			isupprob=true;
			//back to queue
			theApp.uploadqueue->AddClientDirectToQueue(this);
			m_bAddNextConnect=true;
			bDelete = false;
			break;
			//Xman end
		}
		case US_ONUPLOADQUEUE:
			bDelete = false;
			break;
	}
	//Xman end

	//Xman remark:
	//this official code is only used at downloadstate, because upload is handled different with Xtreme
	//
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
	if(checkdownstate){
		bool bAddDeadSource = true;
		switch(m_nDownloadState){
			case DS_ONQUEUE:
			case DS_TOOMANYCONNS:
			case DS_NONEEDEDPARTS:
			case DS_LOWTOLOWIP:
				bDelete = false;
				break;
			case DS_CONNECTING:
				{
					m_cFailed++;
					//Xman Anti-Leecher
					if(IsBanned())
					{
						bDelete=true; //force delete and no retry and no deadsourcelist
						break;
					}
					//Xman end

					//Xman  spread reasks after timeout
					//Xman Xtreme Mod
					//why we should immediately reask the source ? 
					//either the source are busy -->let`s wait
					//or the source are gone, in this case, we shouldn't waste the socket, 
					//but ask other sources (too many connections)
					//TryToConnect(); is called indirect later

					//Xman 
					//optional retry connection attempts
					if(thePrefs.retryconnectionattempts==false)
						m_cFailed=5; //force deadsourcelist

					//Xman  udppending only 1 retry, and lets wait 70 sec
					if (m_cFailed < 2)  //we don't know this clients, only 2 attemps
					{
						SetNextTCPAskedTime(::GetTickCount()+70000); //wait 70 sec bevore the next try
						SetDownloadState(DS_NONE);
						bDelete = false; //Delete this socket but not this client
						break;
					}
					else if (m_cFailed < 3 && GetUserName()!=NULL && !m_bUDPPending ) //we now the client, give 3 attemps
					{
						//theApp.AddLogLine(false,_T("Client with with IP=%s, Version=%s, Name=%s failed to connect %u times"), GetFullIP(), GetClientVerString(), GetUserName(),m_cFailed);
						SetNextTCPAskedTime(::GetTickCount()+50000); //wait 50 sec bevore the next try
						SetDownloadState(DS_NONE);
						bDelete = false; //Delete this socket but not this client
						break;
					}
					//Xman end
					if (socket && socket->GetProxyConnectFailed())
						bAddDeadSource = false;
				}
			case DS_CONNECTED: //Xman Code Improvement delete non answering clients
			case DS_WAITCALLBACK:
				if (bAddDeadSource)
					theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
				theApp.downloadqueue->AddFailedTCPFileReask(); //Xman Xtreme Mod: count the failed TCP-connections
				if(m_bUDPPending) theApp.downloadqueue->AddFailedUDPFileReasks(); //Xman Xtreme Mod: for correct statistics, if it wasn't counted on connection established //Xman x4 test
			case DS_ERROR: //Xman Xtreme Mod: this clients get IP-Filtered!
				bDelete = true;
				break;
		}
	}

	// We keep chat partners in any case
	if (GetChatState() != MS_NONE){
		//Xman Code Improvement
		if(bDelete==true)
			theApp.downloadqueue->RemoveSource(this);
		//Xman end
		bDelete = false;
	}

	// Delete Socket
	if (!bFromSocket && socket){
		ASSERT( theApp.listensocket->IsValidSocket(socket) );
		socket->Safe_Delete();
	}
	socket = NULL;

	// finally, remove the client from the timeouttimer and reset the connecting state
	m_nConnectingState = CCS_NONE;
	theApp.clientlist->RemoveConnectingClient(this);

	if (bDelete)
	{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			Debug(_T("--- Deleted client            %s; Reason=%s\n"), DbgGetClientInfo(true), pszReason);
#endif
		return true;
	}
	else
	{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			Debug(_T("--- Disconnected client       %s; Reason=%s\n"), DbgGetClientInfo(true), pszReason);
#endif
		m_fHashsetRequestingMD4 = 0;
		m_fHashsetRequestingAICH = 0;
		SetSentCancelTransfer(0);
		m_bHelloAnswerPending = false;
		m_bACATHelloAnswerPending = false;	//zz_fly :: Client is always highid if we are connecting to them - by Enig123, idea from ACAT
		m_fQueueRankPending = 0;
		m_fFailedFileIdReqs = 0;
		m_fUnaskQueueRankRecv = 0;
		m_fSentOutOfPartReqs = 0;
		return false;
	}
}

//Returned bool is not if the TryToConnect is successful or not..
//false means the client was deleted!
//true means the client was not deleted!
bool CUpDownClient::TryToConnect(bool bIgnoreMaxCon, bool bNoCallbacks/*, CRuntimeClass* pClassSocket*/)
{
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
	if (socket != NULL){
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
	if ( (RequiresCryptLayer() && !thePrefs.IsClientCryptLayerSupported()) || (thePrefs.IsClientCryptLayerRequired() && !SupportsCryptLayer()) ){
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
		if (theApp.ipfilter->IsFiltered(uClientIP))
		{
			theStats.filteredclients++;
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(true, GetResString(IDS_IPFILTERED), ipstr(uClientIP), _T("IP filter"));
			m_cFailed=5; //force deletion //Xman 
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
			m_cFailed=5; //force deletion //Xman 
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
		//ASSERT( pClassSocket == NULL );
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
		if (!( (SupportsDirectUDPCallback() && thePrefs.GetUDPPort() != 0 && GetConnectIP() != 0) // Direct Callback
			|| (HasValidBuddyID() && Kademlia::CKademlia::IsConnected()) // Kad Callback
			|| theApp.serverconnect->IsLocalServer(GetServerIP(), GetServerPort()) )) // Server Callback
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
		/*if (pClassSocket == NULL)
			pClassSocket = RUNTIME_CLASS(CClientReqSocket);
		socket = static_cast<CClientReqSocket*>(pClassSocket->CreateObject());
		socket->SetClient(this);
		*/
		socket = new CClientReqSocket(this);
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
		CSafeMemFile data(2+16+1);
		data.WriteUInt16(thePrefs.GetPort()); // needs to know our port
		data.WriteHash16(thePrefs.GetUserHash()); // and userhash
		// our connection settings
		data.WriteUInt8(GetMyConnectOptions(true, false));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientUDPLevel() > 0)
			DebugSend("OP_DIRECTCALLBACKREQ", this);
#endif
		Packet* packet = new Packet(&data, OP_EMULEPROT, OP_DIRECTCALLBACKREQ);
		theStats.AddUpDataOverheadOther(packet->size);
		theApp.clientudp->SendPacket(packet, GetConnectIP(), GetKadPort(), ShouldReceiveCryptUDPPackets(), GetUserHash(), false, 0);
		return true;
	}
	////////////////////////////////////////////////////////////
	// 6) Server Callback + 7) Kad Callback
	if (GetDownloadState() == DS_CONNECTING)
		SetDownloadState(DS_WAITCALLBACK);
	
	if (GetUploadState() == US_CONNECTING){
		ASSERT( false ); // we should never try to connect in this case, but wait for the LowID to connect to us
		DebugLogError( _T("LowID and US_CONNECTING (%s)"), DbgGetClientInfo());
	}

	if (theApp.serverconnect->IsLocalServer(m_dwServerIP, m_nServerPort))
	{
		m_nConnectingState = CCS_SERVERCALLBACK;
		Packet* packet = new Packet(OP_CALLBACKREQUEST,4);
		PokeUInt32(packet->pBuffer, m_nUserIDHybrid);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugServerTCPLevel() > 0 || thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__CallbackRequest", this);
#endif
		theStats.AddUpDataOverheadServer(packet->size);
		theApp.serverconnect->SendPacket(packet);
		return true;
	}
	else if (HasValidBuddyID() && Kademlia::CKademlia::IsConnected())
	{
		m_nConnectingState = CCS_KADCALLBACK;
		if( GetBuddyIP() && GetBuddyPort())
		{
			CSafeMemFile bio(16+16+2);
			bio.WriteUInt128(&Kademlia::CUInt128(GetBuddyID()));
			bio.WriteUInt128(&Kademlia::CUInt128(reqfile->GetFileHash()));
			bio.WriteUInt16(thePrefs.GetPort());
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (thePrefs.GetDebugClientKadUDPLevel() > 0 || thePrefs.GetDebugClientUDPLevel() > 0)
				DebugSend("KadCallbackReq", this);
#endif
			Packet* packet = new Packet(&bio, OP_KADEMLIAHEADER, KADEMLIA_CALLBACK_REQ);
			theStats.AddUpDataOverheadKad(packet->size);
			// FIXME: We dont know which kadversion the buddy has, so we need to send unencrypted
			theApp.clientudp->SendPacket(packet, GetBuddyIP(), GetBuddyPort(), false, NULL, true, 0);
			SetDownloadState(DS_WAITCALLBACKKAD);
		}
		else
		{
			// I don't think we should ever have a buddy without its IP (anymore), but nevertheless let the functionality in
			//Create search to find buddy.
			Kademlia::CSearch *findSource = new Kademlia::CSearch;
			findSource->SetSearchTypes(Kademlia::CSearch::FINDSOURCE);
			findSource->SetTargetID(Kademlia::CUInt128(GetBuddyID()));
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
				//Started lookup..
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
	if (HasValidHash() && SupportsCryptLayer() && thePrefs.IsClientCryptLayerSupported() && (RequestsCryptLayer() || thePrefs.IsClientCryptLayerRequested())){
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
	m_cFailed = 0; //Xman Downloadmanager / Xtreme Mod // holds the failed connection attempts

	//Xman Xtreme Mod
	if (m_bUDPPending && !IsRemoteQueueFull()) //Xman  maybe the client has now place at its queue .. then it's right to not answer an UDPFilereaskping
	{
		m_nFailedUDPPackets++;
		theApp.downloadqueue->AddFailedUDPFileReasks();
	}
	m_bUDPPending = false;

	//Xman -Reask sources after IP change- v4 
	//Xman at this point we know, we have a internet-connection -> enable upload
	static uint32 lastaskedforip;
	if(theApp.internetmaybedown)
	{
		if(Kademlia::CKademlia::IsConnected() 
			&& (theApp.uploadqueue->waitinglist.GetCount()>10 //check if we have clients queued, otherwise inetmaybedown gives a wrong value
			|| theApp.last_ip_change==0) //we just started the client //Xman new adapter selection 
			&& theApp.last_ip_change  < ::GetTickCount() - MIN2MS(2) //only once in 2 minutes
			) 
			theApp.m_bneedpublicIP=true;
		if(theApp.IsConnected()) //only free the upload if we are connected. important! otherwise we would have problems with nafc-adapter on a hotstart
		{
			theApp.internetmaybedown = 0;
			theApp.last_traffic_reception = ::GetTickCount();
			theApp.pBandWidthControl->AddeMuleOut(1); //this reopens the upload (internetmaybedown checks for upload==0
		}
	}
	if(theApp.m_bneedpublicIP==true
		&& m_fPeerCache 
		&& lastaskedforip + SEC2MS(3) < ::GetTickCount() //give the client some time, we shouln't ask more than 2-3 clients after reconnect
		) 
	{
		SendPublicIPRequest();
		lastaskedforip=::GetTickCount();
		AddDebugLogLine(false, _T("Internet-connection was possibly down. ask client for ip: %s"), DbgGetClientInfo()); 
	}
	//Xman end

	// ok we have a connection, lets see if we want anything from this client

	// was this a direct callback?
	if (m_nConnectingState == CCS_DIRECTCALLBACK) // TODO LOGREMOVE
		DebugLog(_T("Direct Callback succeeded, connection established - %s"), DbgGetClientInfo()); 

	// remove the connecting timer and state
	//if (m_nConnectingState == CCS_NONE) // TODO LOGREMOVE
	//	DEBUG_ONLY( DebugLog(_T("ConnectionEstablished with CCS_NONE (incoming, thats fine)")) );
	m_nConnectingState = CCS_NONE;
	theApp.clientlist->RemoveConnectingClient(this);

	// check if we should use this client to retrieve our public IP
	//Xman
	/*
	if (theApp.GetPublicIP() == 0 && theApp.IsConnected() && m_fPeerCache)
	*/
	if (theApp.GetPublicIP() == 0 && theApp.serverconnect->IsConnected() && m_fPeerCache)
	//Xman end
		SendPublicIPRequest();

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

	switch(GetDownloadState())
	{
		case DS_CONNECTING:
		case DS_WAITCALLBACK:
		case DS_WAITCALLBACKKAD:
			m_bReaskPending = false;
			SetDownloadState(DS_CONNECTED);
			SendFileRequest();
			break;
	}

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
			if (theApp.uploadqueue->IsDownloading(this))
			{
				SetUploadState(US_UPLOADING);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__AcceptUploadReq", this);
#endif
				Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
				theStats.AddUpDataOverheadFileRequest(packet->size);
				SendPacket(packet,true);
				//Xman find best sources
				//Xman: in every case, we add this client to our downloadqueue
				CKnownFile* partfile = theApp.downloadqueue->GetFileByID(this->GetUploadFileID());
				if (partfile && partfile->IsPartFile())
					theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)partfile,this,true);
				//Xman end
			}
			break;
	}

	if (m_iFileListRequested == 1)
	{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend(m_fSharedDirectories ? "OP__AskSharedDirs" : "OP__AskSharedFiles", this);
#endif
        Packet* packet = new Packet(m_fSharedDirectories ? OP_ASKSHAREDDIRS : OP_ASKSHAREDFILES,0);
		theStats.AddUpDataOverheadOther(packet->size);
		SendPacket(packet,true);
	}

	while (!m_WaitingPackets_list.IsEmpty())
	{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("Buffered Packet", this);
#endif
		SendPacket(m_WaitingPackets_list.RemoveHead(), true);
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
			// ==> Enhanced Client Recognition [Spike] - Stulle
			/*
			case 40:
			*/
			case SO_SHAREAZA2:
			case SO_SHAREAZA3:
			case SO_SHAREAZA4:
			// <== Enhanced Client Recognition [Spike] - Stulle
				m_clientSoft = SO_SHAREAZA;
				pszSoftware = _T("Shareaza");
				break;
			case SO_LPHANT:
				m_clientSoft = SO_LPHANT;
				pszSoftware = _T("lphant");
				break;
			// ==> Enhanced Client Recognition [Spike] - Stulle
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
			// <== Enhanced Client Recognition [Spike] - Stulle
			case SO_EASYMULE2:
				m_clientSoft = SO_EASYMULE2;
				pszSoftware = _T("easyMule");
				break;
			default:
				// ==> Enhanced Client Recognition [Spike] - Stulle
				/*
				if (m_bIsML || m_byCompatibleClient == SO_MLDONKEY){
				*/
				if (m_bIsML || m_byCompatibleClient == SO_MLDONKEY || m_byCompatibleClient == SO_MLDONKEY2 || m_byCompatibleClient == SO_MLDONKEY3){
				// <== Enhanced Client Recognition [Spike] - Stulle
					m_clientSoft = SO_MLDONKEY;
					pszSoftware = _T("MLdonkey");
				}
				// ==> Enhanced Client Recognition [Spike] - Stulle
				/*
				else if (m_bIsHybrid){
				*/
				else if (m_bIsHybrid || m_byCompatibleClient == SO_EDONKEYHYBRID){
				// <== Enhanced Client Recognition [Spike] - Stulle
					m_clientSoft = SO_EDONKEYHYBRID;
					pszSoftware = _T("eDonkeyHybrid");
				}
				else if (m_byCompatibleClient != 0){
					// ==> Enhanced Client Recognition [Spike] - Stulle
					// Recognize all eMulePlus - just to be sure !
					// Note that the compare is done case-sensitive to avoid false positives.
					if (_tcsstr(m_strModVersion, _T("Plus 1")))
					{
						m_clientSoft = SO_EMULEPLUS;
						pszSoftware = _T("eMule Plus");
					}
					else
					{
						// <== Enhanced Client Recognition [Spike] - Stulle
						m_clientSoft = SO_XMULE; // means: 'eMule Compatible'
						pszSoftware = _T("eMule Compat");
					} // Enhanced Client Recognition [Spike] - Stulle
				}
				else{
					m_clientSoft = SO_EMULE;
					pszSoftware = _T("eMule");
				}
				break;
		}

		int iLen;
		TCHAR szSoftware[128];
		if (m_byEmuleVersion == 0){
			m_nClientVersion = MAKE_CLIENT_VERSION(0, 0, 0);
			iLen = _countof(szSoftware) - 1;
			_tcsncpy(szSoftware, pszSoftware, iLen);
		}
		else if (m_byEmuleVersion != 0x99){
			UINT nClientMinVersion = (m_byEmuleVersion >> 4)*10 + (m_byEmuleVersion & 0x0f);
			m_nClientVersion = MAKE_CLIENT_VERSION(0, nClientMinVersion, 0);
			iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v0.%u"), pszSoftware, nClientMinVersion);
		}
		else{
			UINT nClientMajVersion = (m_nClientVersion >> 17) & 0x7f;
			UINT nClientMinVersion = (m_nClientVersion >> 10) & 0x7f;
			UINT nClientUpVersion  = (m_nClientVersion >>  7) & 0x07;
			m_nClientVersion = MAKE_CLIENT_VERSION(nClientMajVersion, nClientMinVersion, nClientUpVersion);
			if (m_clientSoft == SO_EMULE)
				iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v%u.%u%c"), pszSoftware, nClientMajVersion, nClientMinVersion, _T('a') + nClientUpVersion);
			// ==> Enhanced Client Recognition [Spike] - Stulle
			else if (m_clientSoft == SO_EMULEPLUS)
			{
				if(nClientMinVersion == 0)
				{
					if(nClientUpVersion == 0)
						iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v%u"), pszSoftware, nClientMajVersion);
					else
						iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v%u%c"), pszSoftware, nClientMajVersion, _T('a') + nClientUpVersion - 1);
				}
				else
				{
					if(nClientUpVersion == 0)
						iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion);
					else
						iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v%u.%u%c"), pszSoftware, nClientMajVersion, nClientMinVersion, _T('a') + nClientUpVersion - 1);
				}
			}
			// <== Enhanced Client Recognition [Spike] - Stulle
			else if (m_clientSoft == SO_AMULE || m_clientSoft == SO_EASYMULE2 || nClientUpVersion != 0)
				iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v%u.%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion, nClientUpVersion);
			else if (m_clientSoft == SO_LPHANT)
				iLen = _sntprintf(szSoftware, _countof(szSoftware), (nClientMinVersion < 10) ? _T("%s v%u.0%u") : _T("%s v%u.%u"), pszSoftware, (nClientMajVersion-1), nClientMinVersion);
			else
				iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("%s v%u.%u"), pszSoftware, nClientMajVersion, nClientMinVersion);
		}
		if (iLen > 0){
			memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
			m_strClientSoftware.ReleaseBuffer(iLen);
		}
		return;
	}

	// ==> Enhanced Client Recognition [Spike] - Stulle
	/*
	if (m_bIsHybrid){
	*/
	if (m_bIsHybrid || m_byCompatibleClient == SO_EDONKEYHYBRID){
	// <== Enhanced Client Recognition [Spike] - Stulle
		m_clientSoft = SO_EDONKEYHYBRID;
		// seen:
		// 105010	0.50.10
		// 10501	0.50.1
		// 10405	1.4.5 // netfinity:
		// 10300	1.3.0
		// 10212	1.2.2 // Spike2 - Enhanced Client Recognition v2
		// 10211	1.2.1 // Spike2 - Enhanced Client Recognition v2
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
		else if (m_nClientVersion >= 10100 && m_nClientVersion <= 10409){ // netfinity:
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
			iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("eDonkeyHybrid v%u.%u.%u"), nClientMajVersion, nClientMinVersion, nClientUpVersion);
		else
			iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("eDonkeyHybrid v%u.%u"), nClientMajVersion, nClientMinVersion);
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
		int iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("MLdonkey v0.%u"), nClientMinVersion);
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
		int iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("Old eMule v0.%u"), nClientMinVersion);
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
	int iLen = _sntprintf(szSoftware, _countof(szSoftware), _T("eDonkey v0.%u"), nClientMinVersion);
	if (iLen > 0){
		memcpy(m_strClientSoftware.GetBuffer(iLen), szSoftware, iLen*sizeof(TCHAR));
		m_strClientSoftware.ReleaseBuffer(iLen);
	}
}

int CUpDownClient::GetHashType() const
{
	if (m_achUserHash[5] == 14 && m_achUserHash[14] == 111)
		return SO_EMULE;
	if (m_achUserHash[5] == 13 && m_achUserHash[14] == 110)
		return SO_OLDEMULE;
	if (m_achUserHash[5] == 'M' && m_achUserHash[14] == 'L')
		return SO_MLDONKEY;
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

void CUpDownClient::RequestSharedFileList()
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
}

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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__PublicKey", this);
#endif
	SendPacket(packet, true);
	m_SecureIdentState = IS_SIGNATURENEEDED;
}

void CUpDownClient::SendSignaturePacket(){
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
	bool bUseV2 = (m_bySupportSecIdent&1) == 0;

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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__Signature", this);
#endif
	SendPacket(packet, true);
	m_SecureIdentState = IS_ALLREQUESTSSEND;
}

void CUpDownClient::ProcessPublicKeyPacket(const uchar* pachPacket, uint32 nSize)
{
	theApp.clientlist->AddTrackClient(this);

	if (socket == NULL || credits == NULL || pachPacket[0] != nSize-1
		|| nSize < 10 || nSize > 250){
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

	if (socket == NULL || credits == NULL || nSize > 250 || nSize < 10){
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

		// BEGIN netfinity: Credits Reset Exploit Prevention
		if (credits->GetUploadedTotal() < (uint64) GetTransferredUp() || credits->GetDownloadedTotal() < (uint64) GetTransferredDown()){
			DebugLogWarning(_T("Possible credit reset exploit: Up=%u, Down=%u  %s"), GetTransferredUp(), GetTransferredDown(), DbgGetClientInfo());
			if (credits->GetUploadedTotal() < (uint64) GetTransferredUp())
				credits->AddUploaded((uint32) (GetTransferredUp() - credits->GetUploadedTotal()), GetIP());
			if (credits->GetDownloadedTotal() < (uint64) GetTransferredDown())
				credits->AddDownloaded((uint32) (GetTransferredDown() - credits->GetDownloadedTotal()), GetIP());
		}
		// END netfinity: Credits Reset Exploit Prevention 
	}
	else
	{
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
		SFMT&rng = *t_rng;
		uint32 dwRandom;
		do{
			dwRandom = rng.getUInt32();
		}while(dwRandom == 0);
		credits->m_dwCryptRndChallengeFor = dwRandom;
		Packet* packet = new Packet(OP_SECIDENTSTATE, 1+4, OP_EMULEPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		packet->pBuffer[0] = nValue;
		PokeUInt32(packet->pBuffer+1, dwRandom);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__SecIdentState", this);
#endif
		SendPacket(packet, true);
	}
	//else
		//ASSERT ( false );
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
	delete[] m_abyPartStatus;
	m_abyPartStatus = NULL;

	//Xman Xtreme Downloadmanager
	// Maella -Unnecessary Protocol Overload-
	//m_dwLastAskedTime = 0; we don't reset this value, to decide if a droped client may reenter the queue, see CheckAndAdd
	m_dwLastUDPReaskTime = 0;
	m_dwNextTCPAskedTime = 0;
	// Maella end
	//Xman  Xtreme Mod
	//at this point there can't be UDPpending... only if we remove the source.. and then we have to reset this value
	m_bUDPPending=false;

	SetRemoteQueueRank(0,false); 

	oldQR=0; //Xman DiffQR
	//Xman end

	m_nPartCount = 0;
	m_strClientFilename.Empty();
	m_bCompleteSource = false;
	delete m_pReqFileAICHHash;
	m_pReqFileAICHHash = NULL;
}

bool CUpDownClient::IsBanned() const
{
	return theApp.clientlist->IsBannedClient(GetConnectIP()); //Xman official bugfix, was: //GetIP()); 
}

// sends a packet, if needed it will establish a connection before
// options used: ignore max connections, control packet, delete packet
// !if the functions returns false that client object was deleted because the connection try failed and the object wasn't needed anymore.
bool CUpDownClient::SafeConnectAndSendPacket(Packet* packet)
{
	if (socket != NULL && socket->IsConnected())
	{
		socket->SendPacket(packet, true, true);
		return true;
	}
	else
	{
		m_WaitingPackets_list.AddTail(packet);
		return TryToConnect(true);
	}
}

bool CUpDownClient::SendPacket(Packet* packet, bool bDeletePacket, bool bVerifyConnection)
{
	if (socket != NULL && (!bVerifyConnection || socket->IsConnected()))
	{
		socket->SendPacket(packet, bDeletePacket, true);
		return true;
	}
	else
	{
		DebugLogError(_T("Outgoing packet (0x%X) discarded because expected socket or connection does not exists %s"), packet->opcode, DbgGetClientInfo());
		if (bDeletePacket)
			delete packet;
		return false;
	}
}

#ifdef _DEBUG
void CUpDownClient::AssertValid() const
{
	CObject::AssertValid();

	CHECK_OBJ(socket);
	CHECK_PTR(credits);
	CHECK_OBJ(reqfile);
	(void)m_abyUpPartStatus;
	m_OtherRequests_list.AssertValid();
	m_OtherNoNeeded_list.AssertValid();
	(void)m_lastPartAsked;
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
	(void)m_nUpPartCount;
	(void)m_nUpCompleteSourcesCount;
	(void)requpfileid;
    (void)m_lastRefreshedULDisplay;
	m_BlockRequests_queue.AssertValid();
	m_DoneBlocks_list.AssertValid();
	m_RequestedFiles_list.AssertValid();
	ASSERT( m_nDownloadState >= DS_DOWNLOADING && m_nDownloadState <= DS_NONE );
	(void)m_cDownAsked;
	(void)m_abyPartStatus;
	(void)m_strClientFilename;
	(void)m_nTransferredDown;
	(void)m_nCurSessionPayloadDown;
	(void)m_dwDownStartTime;
	(void)m_nLastBlockOffset;
	(void)m_nDownDatarate;
	(void)m_nRemoteQueueRank;
	(void)m_dwLastBlockReceived;
	(void)m_nPartCount;
	ASSERT( m_nSourceFrom >= SF_SERVER && m_nSourceFrom <= SF_SLS );
	CHECK_BOOL(m_bRemoteQueueFull);
	CHECK_BOOL(m_bCompleteSource);
	CHECK_BOOL(m_bReaskPending);
	CHECK_BOOL(m_bUDPPending);
	CHECK_BOOL(m_bTransferredDownMini);
	CHECK_BOOL(m_bUnicodeSupport);
	ASSERT( m_nKadState >= KS_NONE && m_nKadState <= KS_CONNECTING_FWCHECK_UDP);
	m_PendingBlocks_list.AssertValid();
	m_DownloadBlocks_list.AssertValid();
	ASSERT( m_nChatstate >= MS_NONE && m_nChatstate <= MS_UNABLETOCONNECT );
	CHECK_BOOL(m_bCollectionUploadSlot);
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
		_T("Connecting"),
		_T("Banned"),
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
	// X-Ray :: FiXeS :: Bugfix :: Start :: WiZaRd
	// one should never "append" via + operator! really bad style AND dangerous!
	/*
	return GetClientSoftVer() + _T(" [") + GetClientModVer() + _T(']');
	*/
	CString ret;
	ret.Format(L"%s [%s]", GetClientSoftVer(), GetClientModVer());
	return ret;
	// X-Ray :: FiXeS :: Bugfix :: End :: WiZaRd
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
			str.Format(_T("%p - Invalid client instance"), this);
		}
	}
	return str;
}

bool CUpDownClient::CheckHandshakeFinished() const
{
	/*if (m_bHelloAnswerPending)
	{
		// 24-Nov-2004 [bc]: The reason for this is that 2 clients are connecting to each other at the same..
		//if (thePrefs.GetVerbose())
		//	AddDebugLogLine(DLP_VERYLOW, false, _T("Handshake not finished - while processing packet: %s; %s"), DbgGetClientTCPOpcode(protocol, opcode), DbgGetClientInfo());
		return false;
	}
	return true;*/
	return !m_bHelloAnswerPending; //Enig123
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

CString CUpDownClient::GetDownloadStateDisplayString() const
{
	CString strState;
	switch (GetDownloadState())
	{
		case DS_CONNECTING:
			strState = GetResString(IDS_CONNECTING);
			break;
		case DS_CONNECTED:
			strState = GetResString(IDS_ASKING);
			break;
		case DS_WAITCALLBACK:
			strState = GetResString(IDS_CONNVIASERVER);
			break;
		case DS_ONQUEUE:
			if (IsRemoteQueueFull())
				strState = GetResString(IDS_QUEUEFULL);
			else
// ==> {diffQR} [Max] 
			//strState = GetResString(IDS_ONQUEUE);
			{
				if ( GetRemoteQueueRank()){
					if(GetDiffQR()==0)
						strState.Format(_T("QR: %u"), GetRemoteQueueRank());
					else
						strState.Format(_T("QR: %u (%+i)"), GetRemoteQueueRank(), GetDiffQR());// +QR [Max] 
				}
				else
					strState = GetResString(IDS_ONQUEUE);
			}
// <== {diffQR} [Max] 
			break;
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
		case DS_TOOMANYCONNSKAD:
			strState = GetResString(IDS_KAD_TOOMANDYKADLKPS);
			break;
	}

	//Xman 4.2
	if(GetUploadState()==US_UPLOADING)
		strState = _T(">>") + strState;
	//Xman end

	return strState;
}

CString CUpDownClient::GetUploadStateDisplayString() const
{
	CString strState;
	switch (GetUploadState()){
		case US_ONUPLOADQUEUE:
			strState = GetResString(IDS_ONQUEUE);
			break;
		case US_BANNED:
			strState = GetResString(IDS_BANNED);
			break;
		case US_CONNECTING:
			strState = GetResString(IDS_CONNECTING);
			break;
		case US_UPLOADING:
			//Xman Xtreme Upload
			if(GetPayloadInBuffer() == 0 && GetNumberOfRequestedBlocksInQueue() == 0 && thePrefs.IsExtControlsEnabled()) 
				strState = GetResString(IDS_US_STALLEDW4BR);
			else  if(GetPayloadInBuffer() == 0 && thePrefs.IsExtControlsEnabled()) 
				strState = GetResString(IDS_US_STALLEDREADINGFDISK);
			else
				strState = GetResString(IDS_TRANSFERRING);
			//Xman end
			break;
	}

	return strState;
}

void CUpDownClient::SendPublicIPRequest(){
	if (socket && socket->IsConnected()){
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__PublicIPReq", this);
#endif
		Packet* packet = new Packet(OP_PUBLICIP_REQ,0,OP_EMULEPROT);
		theStats.AddUpDataOverheadOther(packet->size);
		SendPacket(packet, true);
		m_fNeedOurPublicIP = 1;
	}
}

void CUpDownClient::ProcessPublicIPAnswer(const BYTE* pbyData, UINT uSize){
	if (uSize != 4)
		throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
	uint32 dwIP = PeekUInt32(pbyData);
	if (m_fNeedOurPublicIP == 1){ // did we?
		m_fNeedOurPublicIP = 0;

		//Xman: we have a problem if our IP ends with 0. The Check for IsLowID() would be true and
		//the code can't work (also official part)
		//Fix: if we receive a LowID we do a recheck
		if(dwIP==0) return; //thats wrong in every case
		if(IsLowID(dwIP) && dwIP!=theApp.recheck_ip)
		{
			//check if still needed:
			if(theApp.m_bneedpublicIP
				|| theApp.serverconnect->IsConnected() && theApp.GetPublicIP() == 0)
				theApp.recheck_ip=dwIP;
			return; //will force a new request
		}

		//Xman -Reask sources after IP change- v4 
		if(theApp.m_bneedpublicIP /*&& !::IsLowID(dwIP)*/) //this is the case we have kad-only but no upload->inet down ?
		{

			//Xman new adapter selection 
			{
				AddLogLine(false, _T("received an IP from a client: %u.%u.%u.%u, NAFC-Adapter will be checked"), (uint8)dwIP, (uint8)(dwIP>>8), (uint8)(dwIP>>16), (uint8)(dwIP>>24));
				theApp.pBandWidthControl->checkAdapterIndex(dwIP);
			}
			//Xman end

			if(theApp.last_valid_ip!=0 && theApp.last_valid_ip != dwIP)
			{

				//if we had a lowID we asume we get it again. Then let the server trigger
				if((theApp.serverconnect->IsConnecting() && IsLowID(theApp.last_valid_serverid))
					|| (theApp.serverconnect->IsConnected() && theApp.serverconnect->IsLowID() && theApp.GetPublicIP(true)!=dwIP))
				{
					//don't trigger
				}
				else
				{
					if(GetTickCount() - theApp.last_ip_change > FILEREASKTIME + 60000)
					{
						theApp.clientlist->TrigReaskForDownload(true);
						AddLogLine(false, _T("Kad Connection detected IP-change, changed IP from %s to %s, all sources will be reasked immediately"), ipstr(theApp.last_valid_ip), ipstr(dwIP));
					}
					else
					{
						theApp.clientlist->TrigReaskForDownload(false);
						AddLogLine(false, _T("Kad Connection detected IP-change, changed IP from %s to %s, all sources will be reasked within the next 10 minutes"), ipstr(theApp.last_valid_ip), ipstr(dwIP));
					}
					SetNextTCPAskedTime(::GetTickCount() + FILEREASKTIME); //not for this source
				}
				//zz_fly :: Rebind UPnP on IP-change :: start
				if(thePrefs.IsUPnPEnabled() && thePrefs.GetUPnPNatRebind())
				{
					// we don't want to trigger it twice, do we?!
					theApp.emuledlg->RefreshUPnP(false); // refresh the UPnP mappings once
				}
				//zz_fly :: Rebind UPnP on IP-change :: end
				// Xman reconnect Kad on IP-change
				if (Kademlia::CKademlia::IsConnected() && Kademlia::CKademlia::GetPrefs()->GetIPAddress())
					if(ntohl(Kademlia::CKademlia::GetIPAddress()) != dwIP)
					{
						if(Kademlia::CKademlia::GetIPAddress()!=0)
						{
							AddDebugLogLine(DLP_DEFAULT, false,  _T("Public IP Address reported from Kademlia (%s) differs from new found (%s), restart Kad"),ipstr(ntohl(Kademlia::CKademlia::GetIPAddress())),ipstr(dwIP));
							Kademlia::CKademlia::Stop();
							Kademlia::CKademlia::Start();
						}
						//Kad loaded the old IP, we must reset
						if(Kademlia::CKademlia::IsRunning()) //one more check
						{
							Kademlia::CKademlia::GetPrefs()->SetIPAddress(0);
							Kademlia::CKademlia::GetPrefs()->SetIPAddress(htonl(dwIP));
						}
					}
				//Xman end

				//on some routers it needs endless time until a server-reconnect is initiated after ip-change
				if(theApp.serverconnect->IsConnected() && theApp.GetPublicIP(true)!=dwIP)
				{
					theApp.serverconnect->Disconnect();
					theApp.serverconnect->ConnectToAnyServer();
				}
			}
			theApp.last_valid_ip=dwIP;

			theApp.m_bneedpublicIP=false;
			theApp.last_ip_change=::GetTickCount(); //remark: this is set when ever inet was down, even we receive the old ip

		}
		if(theApp.serverconnect->IsConnected())//remark: this is the case we have a lowid-server-connect
			if (theApp.GetPublicIP() == 0 /*&& !::IsLowID(dwIP)*/ )
			{
				theApp.SetPublicIP(dwIP); 
				theApp.last_valid_ip=dwIP;
			}
		//Xman end

		//Xman fix
		theApp.recheck_ip=0;
	}	
}

void CUpDownClient::CheckFailedFileIdReqs(const uchar* aucFileHash)
{
	if ( aucFileHash != NULL && (theApp.sharedfiles->IsUnsharedFile(aucFileHash) || theApp.downloadqueue->GetFileByID(aucFileHash)) )
		return;
	//if (GetDownloadState() != DS_DOWNLOADING) // filereq floods are never allowed!
	{
		if (m_fFailedFileIdReqs < 6)// NOTE: Do not increase this nr. without increasing the bits for 'm_fFailedFileIdReqs'
			m_fFailedFileIdReqs++;
		if (m_fFailedFileIdReqs >= 6)
		{
			//Xman we filter not ban!
			/*
			if (theApp.clientlist->GetBadRequests(this) < 2)
				theApp.clientlist->TrackBadRequest(this, 1);
			if (theApp.clientlist->GetBadRequests(this) == 2){
				theApp.clientlist->TrackBadRequest(this, -2); // reset so the client will not be rebanned right after the ban is lifted
				Ban(_T("FileReq flood"));
			}
			*/
			throw CString(_T("FileReq flood"));
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

bool  CUpDownClient::IsObfuscatedConnectionEstablished() const {
	if (socket != NULL && socket->IsConnected())
		return socket->IsObfusicating();
	else
		return false;
}

bool CUpDownClient::ShouldReceiveCryptUDPPackets() const {
	return (thePrefs.IsClientCryptLayerSupported() && SupportsCryptLayer() && theApp.GetPublicIP() != 0
		&& HasValidHash() && (thePrefs.IsClientCryptLayerRequested() || RequestsCryptLayer()) );
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
	CSafeMemFile data(2+2+4);
	data.WriteUInt16(Kademlia::CKademlia::GetPrefs()->GetInternKadPort());
	data.WriteUInt16(Kademlia::CKademlia::GetPrefs()->GetExternalKadPort());
	data.WriteUInt32(Kademlia::CKademlia::GetPrefs()->GetUDPVerifyKey(GetConnectIP()));
	Packet* packet = new Packet(&data, OP_EMULEPROT, OP_FWCHECKUDPREQ);
	theStats.AddUpDataOverheadKad(packet->size);
	SafeConnectAndSendPacket(packet);
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
	
	CSafeMemFile fileTestPacket1(1+2);
	fileTestPacket1.WriteUInt8(bErrorAlreadyKnown ? 1 : 0);
	fileTestPacket1.WriteUInt16(nRemoteInternPort);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA2_FIREWALLUDP", ntohl(GetConnectIP()), nRemoteInternPort);
#endif
	Kademlia::CKademlia::GetUDPListener()->SendPacket(&fileTestPacket1, KADEMLIA2_FIREWALLUDP, ntohl(GetConnectIP())
		, nRemoteInternPort, Kademlia::CKadUDPKey(dwSenderKey, theApp.GetPublicIP(false)), NULL);
	
	// if the client has a router with PAT (and therefore a different extern port than intern), test this port too
	if (nRemoteExternPort != 0 && nRemoteExternPort != nRemoteInternPort){
		CSafeMemFile fileTestPacket2(1+2);
		fileTestPacket2.WriteUInt8(bErrorAlreadyKnown ? 1 : 0);
		fileTestPacket2.WriteUInt16(nRemoteExternPort);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA2_FIREWALLUDP", ntohl(GetConnectIP()), nRemoteExternPort);
#endif
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

void CUpDownClient::SendSharedDirectories()
{
	//TODO: Don't send shared directories which do not contain any files
	// add shared directories
	CString strDir;
	CAtlArray<CString> arFolders;
	POSITION pos = thePrefs.shareddir_list.GetHeadPosition();
	while (pos)
	{
		strDir = theApp.sharedfiles->GetPseudoDirName(thePrefs.shareddir_list.GetNext(pos));
		if (!strDir.IsEmpty())
			arFolders.Add(strDir);
	}

	// add incoming folders
	for (size_t iCat = 0; iCat < thePrefs.GetCatCount(); iCat++)
	{
		strDir = theApp.sharedfiles->GetPseudoDirName(thePrefs.GetCategory(iCat)->strIncomingPath);
		if (!strDir.IsEmpty())
			arFolders.Add(strDir);
	}

	// add temporary folder if there are any temp files
	if (theApp.downloadqueue->GetFileCount() > 0)
		arFolders.Add(CString(OP_INCOMPLETE_SHARED_FILES));
	// add "Other" folder (for single shared files) if there are any single shared files
	if (theApp.sharedfiles->ProbablyHaveSingleSharedFiles())
		arFolders.Add(CString(OP_OTHER_SHARED_FILES));

	// build packet
	CSafeMemFile tempfile(80);
	tempfile.WriteUInt32((uint32)arFolders.GetCount());
	for (size_t i = 0; i < arFolders.GetCount(); i++)
		tempfile.WriteString(arFolders.GetAt(i), GetUnicodeSupport());

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__AskSharedDirsAnswer", this);
#endif
	Packet* replypacket = new Packet(&tempfile, OP_EDONKEYPROT, OP_ASKSHAREDDIRSANS);
	theStats.AddUpDataOverheadOther(replypacket->size);
	VERIFY( SendPacket(replypacket, true, true) );
}

//EastShare Start - added by AndCycle, IP to Country
// Superlexx - client's location
CString&	CUpDownClient::GetCountryName(/*bool longName*/) const {
	// X: [IP2L] - [IP2Location]
	if(!m_structUserLocation->locationName.IsEmpty()
		|| m_structUserLocation == &theApp.ip2country->defaultLocation)// X: show empty for unknown client
		return m_structUserLocation->locationName;
	return theApp.ip2country->GetCountryNameFromRef(m_structUserLocation->country/*,longName*/);
}

//Xman 5.1
// Maella -Unnecessary Protocol Overload-
void CUpDownClient::CalculateJitteredFileReaskTime(bool longer)
{
	if(longer==false)
	{
		if(GetClientSoft() == SO_MLDONKEY) // X-Ray :: FastReaskforMLDonkey
			m_jitteredFileReaskTime = MIN_REQUESTTIME;
		// X-Ray :: MOD :: Start
		// lower reasktime for emuleplus clients
		else if(GetClientSoft() == SO_EMULEPLUS)
			m_jitteredFileReaskTime = MIN2MS(28);
		// also a lower reasktime for shareaza clients
		else if (GetClientSoft() == SO_SHAREAZA)
			m_jitteredFileReaskTime = MIN_REQUESTTIME*2;
		else{
		// Maella -Unnecessary Protocol Overload-
		// Remark: a client will be remove from an upload queue after 3*FILEREASKTIME (~1 hour)
		//         a two small value increases the traffic + causes a banishment if lower than 10 minutes
			//uint32 jitter = t_rng->getUInt32() * (35*6) / RAND32_MAX; // 0..3.5 minutes, keep in mind integer overflow
			uint32 jitter = t_rng->getUInt32() / (RAND32_MAX/(35*6));
		m_jitteredFileReaskTime = FILEREASKTIME - (60*1000) + 1000*jitter - (2*60*1000); // -2..+2 minutes, keep the same average overload
		//Xman: result between 26 and 29.5 this is useful to use TCP-Connection from older clients
	}
	}
	else
		m_jitteredFileReaskTime = FILEREASKTIME + (3*60*1000); //32 min
		//this gives the client the chance to connect first
		//this connection can be used by Xtreme, see partfile->process
}
//Xman end

/*
int CUpDownClient::GetImageIndex() const{
    //morph4u +
	switch (GetDownloadState()) {
							case DS_CONNECTING:
							case DS_CONNECTED:
							case DS_WAITCALLBACKKAD:
							case DS_WAITCALLBACK:
								return 2; //red
							case DS_ONQUEUE:
								if(IsRemoteQueueFull())
									return 3; //gray
								else
									return 1; //orange
							case DS_DOWNLOADING:
							case DS_REQHASHSET:
								return 0; //green
							case DS_NONEEDEDPARTS:
							case DS_ERROR:
								return 3; //gray
							case DS_TOOMANYCONNS:
							case DS_TOOMANYCONNSKAD:
								return 2; //red
							default:
								return 3; //gray
						}
	//morph4u -
	return 3; //gray
}
*/
