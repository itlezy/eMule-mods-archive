//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "DLP.h" //Xman DLP
#include "BandWidthControl.h"
#include "Neo/NeoOpcodes.h" // NEO: NMP - [NeoModProt] <-- Xanatos --
#include "Neo/Functions.h" // NEO: NMP - [NeoModProt] <-- Xanatos --
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "Neo/NatManager.h"
#include "Neo/NatSocket.h"
#include "Neo/NatTunnel.h" // NEO: NATT - [NatTraversal]
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#include "./Addons/ModIconMapping.h" //>>> WiZaRd::ModIconMapper

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//>>> WiZaRd::ECR [Spike2]
#define SO_OLD_MLDONKEY	-1 //0, 10, 53, 154 are reserved
//<<< WiZaRd::ECR [Spike2]

//IMPLEMENT_DYNAMIC(CClientException, CException) //Xman unused
IMPLEMENT_DYNAMIC(CUpDownClient, CObject)
//Xman
// Maella -Upload Stop Reason-
// Maella -Download Stop Reason-
// Remark: static element are automaticaly initialized with zero
uint32 CUpDownClient::m_upStopReason[2][CUpDownClient::USR_EXCEPTION+1];
uint32 CUpDownClient::m_downStopReason[2][CUpDownClient::DSR_EXCEPTION+1];
// Maella end
//Xman Anti-Leecher: simple Anti-Thief
const CString CUpDownClient::str_ANTAddOn=CUpDownClient::GetANTAddOn();

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
	m_iModIconIndex = MODMAP_NONE; //>>> WiZaRd::ModIconMapper
	//Xman Full Chunk
	upendsoon=false;
	
	//Xman filter clients with failed downloads
	m_faileddownloads=0;

	//Xman Xtreme Mod
	m_szFullUserIP=_T("?");
	m_cFailed = 0; //Xman Downloadmanager / Xtreme Mod // holds the failed connection attempts

	//Xman fix for startupload
	lastaction=0;

	//Xman fix for startupload (downloading side)
	protocolstepflag1=false;

	//Xman uploading problem client
	isupprob=false;

	//Xman askfordownload priority
	m_downloadpriority=1;

	credits = 0;
	m_bAddNextConnect = false;
	m_nChatstate = MS_NONE;
	m_nKadState = KS_NONE;
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
	m_bFriendSlot = false;
	m_bCommentDirty = false;
	m_bReaskPending = false;
	m_bUDPPending = false;
	m_byEmuleVersion = 0;
	m_nUserPort = 0;
	m_nPartCount = 0;
	m_nUpPartCount = 0;
	m_abyPartStatus = 0;
	m_abyUpPartStatus = 0;
	m_nDownloadState = DS_NONE;
	m_dwUploadTime = 0;
	m_nTransferredDown = 0;
	m_nUploadState = US_NONE;
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
	m_Friend = NULL;
	m_uFileRating=0;
	(void)m_strFileComment;
	m_fMessageFiltered = 0;
	m_fIsSpammer = 0;
	m_cMessagesReceived = 0;
	m_cMessagesSent = 0;
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
	m_structUserCountry = theApp.ip2country->GetCountryFromIP(m_dwUserIP); 
	//EastShare End - added by AndCycle, IP to Country

	m_fHashsetRequesting = 0;
	m_fSharedDirectories = 0;
	m_fSentCancelTransfer = 0;
	m_nClientVersion = 0;
	m_lastRefreshedDLDisplay = 0;
	m_dwDownStartTime = 0;
	m_nLastBlockOffset = (uint64)-1;
	m_bUnicodeSupport = false;
	m_SecureIdentState = IS_UNAVAILABLE;
	m_dwLastSignatureIP = 0;
	m_bySupportSecIdent = 0;
	m_byInfopacketsReceived = IP_NONE;
	m_lastPartAsked = (uint16)-1;
	m_nUpCompleteSourcesCount= 0;
	m_fSupportsPreview = 0;
	m_fPreviewReqPending = 0;
	m_fPreviewAnsPending = 0;
	m_bTransferredDownMini = false;
    m_addedPayloadQueueSession = 0;
    m_nCurQueueSessionPayloadUp = 0; // PENDING: Is this necessary? ResetSessionUp()...
    m_lastRefreshedULDisplay = ::GetTickCount();
	m_bGPLEvildoer = false;
	m_byHelloPacketState = HP_NONE;  //m_bHelloAnswerPending = false; //Xman Fix Connection Collision (Sirob)
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
	m_ePeerCacheDownState = PCDS_NONE;
	m_ePeerCacheUpState = PCUS_NONE;
	m_bPeerCacheDownHit = false;
	m_bPeerCacheUpHit = false;
	m_fNeedOurPublicIP = 0;
    m_random_update_wait = (uint32)(rand()/(RAND_MAX/1000));
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

	//Xman -----------------
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	m_displayUpDatarateCounter = 0;
	m_displayDownDatarateCounter = 0;

	m_nUpDatarate = 0;
	m_nUpDatarate10 = 0;
	m_nUpDatarateMeasure = 0;

	m_nDownDatarate = 0;
	m_nDownDatarate10=0;
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
	
	//Xman Anti-Leecher
	m_bLeecher = 0;
	old_m_pszUsername.Empty();
	m_strBanMessage.Empty();
	strBanReason_permament.Empty(); 
	uhashsize=16;
	//Xman Anti-Nick-Changer
	m_uNickchanges=0;
	m_ulastNickChage=0; //no need to initalize
	//Xman end

	//>>> Anti-XS-Exploit (Xman)
	m_uiXSReqs = 0;
	m_uiXSAnswer = 0;
	//<<< Anti-XS-Exploit

	filedata = NULL; // SiRoB: ReadBlockFromFileThread

	//Xman Funny-Nick (Stulle/Morph)
	m_pszFunnyNick=NULL;
	//Xman end

	//Xman client percentage
	hiscompletedparts_percent_up=-1;
	hiscompletedparts_percent_down=-1;
	//Xman end

	//Xman end --------------

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

    //m_dwLastTriedToConnect = ::GetTickCount()-20*60*1000; // ZZ:DownloadManager
	m_dwLastTriedToConnect = 0; // NEO: L2HAC - [LowID2HighIDAutoCallback] <-- Xanatos --
}

CUpDownClient::~CUpDownClient(){

	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHHashSet::ClientAICHRequestFailed(this);
	}

	if (m_Friend)
        m_Friend->SetLinkedClient(NULL);
	
	theApp.clientlist->RemoveClient(this, _T("Destructing client object"));

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

	delete[] m_abyPartStatus;
	m_abyPartStatus = NULL;

	delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;

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

	//Xman Funny-Nick (Stulle/Morph)
	if (m_pszFunnyNick) {
		delete[] m_pszFunnyNick;
		m_pszFunnyNick = NULL;
	}
	//Xman end


	//Xman end -----------------
}
//Xman Anti-Leecher
//Xman DLP (no more extra tags inside this function)
void CUpDownClient::TestLeecher(){

	//Xman DLP
	if(theApp.dlp->IsDLPavailable()==false)
		return;
	//Xman end

	if (thePrefs.GetAntiLeecherMod())
	{
		if(old_m_strClientSoftwareFULL.IsEmpty() || old_m_strClientSoftwareFULL!= DbgGetFullClientSoftVer() )
		{
		
			old_m_strClientSoftwareFULL = DbgGetFullClientSoftVer();
			LPCTSTR reason=theApp.dlp->DLPCheckModstring_Hard(m_strModVersion,m_strClientSoftware);
			if(reason)
			{
				BanLeecher(reason,5); //hard ban
				return;
			}
			reason=theApp.dlp->DLPCheckModstring_Soft(m_strModVersion,m_strClientSoftware);
			if(reason)
			{
				BanLeecher(reason,4); //soft ban
				return;
			}
			else if(IsLeecher()==4)
			{
				m_bLeecher=0;	//unban, because it is now a good mod
				m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
				old_m_pszUsername.Empty(); //force recheck
			}

		}
	}
	else if(IsLeecher()==4)
	{
		m_bLeecher=0;	//unban, because user doesn't want to check it anymore
		m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
		old_m_pszUsername.Empty(); //force recheck
		old_m_strClientSoftwareFULL.Empty(); //force recheck if user re enable function
	}


	if(thePrefs.GeTAntiLeecheruserhash() && HasValidHash())
	{
		PBYTE uhash=(PBYTE)GetUserHash();
		LPCTSTR reason=theApp.dlp->DLPCheckUserhash(uhash);
		if(reason)
		{
			BanLeecher(_T("*AJ*"),18);
			return;
		}
	}

	if (thePrefs.GetAntiLeecherName())
	{

		//Xman Anti-Nick-Changer
		if(m_pszUsername!=NULL && old_m_pszUsername.IsEmpty()==false)
		{
			if(old_m_pszUsername!=m_pszUsername)
			{
				if(IsLeecher()==0 && m_strModVersion.IsEmpty() //check only if it isn't a known leecher and doesn't send modversion
					&& ::GetTickCount() - m_ulastNickChage < HR2MS(3)) //last nickchane was in less than 3 hours
				{
					m_uNickchanges++;
					if(m_uNickchanges >=3)
					{
						BanLeecher(_T("Nick-Changer"),5); //hard ban
						return;
					}
				}
			}
			else
			{
				//decrease the value if it's the same nick
				if(m_uNickchanges>0)
					m_uNickchanges--;
			}
		}
		//Xman end Anti-Nick-Changer
		
		if(m_bLeecher!=4 && m_pszUsername!=NULL && (old_m_pszUsername.IsEmpty() || old_m_pszUsername!=m_pszUsername)) //remark: because old_m_pszUsername is CString and there operator != is defined, it isn't a pointer comparison 
		{
			old_m_pszUsername = m_pszUsername;
			m_ulastNickChage=::GetTickCount(); //Xman Anti-Nick-Changer

			//find gamer snake 
			if (HasValidHash())
			{
				CString struserhash=md4str(GetUserHash());
				LPCTSTR reason=theApp.dlp->DLPCheckNameAndHashAndMod(m_pszUsername,struserhash,m_strModVersion);
				if(reason)
				{
					BanLeecher(reason,10); //soft ban
					return;
				}
			}

			LPCTSTR reason=theApp.dlp->DLPCheckUsername_Soft(m_pszUsername);
			if(reason)
			{
				BanLeecher(reason,10); //soft ban
				return;
			}

			reason=theApp.dlp->DLPCheckUsername_Hard(m_pszUsername);
			if(reason)
			{
				BanLeecher(reason,5); //hard ban
				return;
			}

			if(IsLeecher()==10 && reason==NULL)
			{
				m_bLeecher=0; //unban it is a good mod now
				m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			}
		}
	}
	else if(IsLeecher()==10)
	{
		m_bLeecher=0;	//unban, because user doesn't want to check it anymore
		m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
		old_m_pszUsername.Empty(); //force recheck if user re enable function
	}

	if (m_nClientVersion > MAKE_CLIENT_VERSION(0, 30, 0) && m_byEmuleVersion > 0 && m_byEmuleVersion != 0x99 && m_clientSoft == SO_EMULE)
	{
		BanLeecher(_T("Fake emuleVersion"),9);
		return;
	} else
	//Xman Anti-Leecher: simple Anti-Thief
	if(m_bLeecher==0 || m_bLeecher==6 || m_bLeecher==11 ) //only check if not banned by other criterion
	{
		static 	const float MOD_FLOAT_VERSION= (float)_tstof(CString(MOD_VERSION).Mid(7)) ;
		const float xtremeversion=GetXtremeVersion(m_strModVersion);
		if(thePrefs.GetAntiLeecherThief())
		{
			if(xtremeversion==MOD_FLOAT_VERSION && !StrStrI(m_strClientSoftware,MOD_MAJOR_VERSION))
			{
				BanLeecher(_T("Mod-ID Faker"),6);
				return;
			}
			if(xtremeversion>=4.4f && CString(m_pszUsername).Right(m_strModVersion.GetLength()+1)!=m_strModVersion + _T("»"))
			{
				BanLeecher(_T("MOD-ID Faker(advanced)"),6);
				return;
			}

			if(IsLeecher()==6)
			{
				m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
				m_bLeecher=0; //unban it isn't anymore a mod faker
			}
		}
		else if(IsLeecher()==6)
		{
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
		
		//Xman new Anti-Nick-Thief
		if(thePrefs.GetAntiLeecherThief() )
		{
			if(StrStrI(m_pszUsername, str_ANTAddOn)) 
			{
				BanLeecher(_T("Nick Thief"),11);
				return;
			}
			if(IsLeecher()==11)
			{
				m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't a nickthief anymore
			}
		}
		else if(IsLeecher()==11)
		{
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
	}
	//Xman end simple Anti-Thief
}
//Xman end

void CUpDownClient::ProcessBanMessage()
{
	if(m_strBanMessage.IsEmpty()==false)
	{
		AddLeecherLogLine(false,m_strBanMessage);
		theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	}
	m_strBanMessage.Empty();
}

//Xman end

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
	m_strModVersion.Empty(); //Maella -Support for tag ET_MOD_VERSION 0x55
}

bool CUpDownClient::ProcessHelloPacket(const uchar* pachPacket, uint32 nSize)
{
	CSafeMemFile data(pachPacket, nSize);
	//Xman Anti-Leecher
	//data.ReadUInt8(); // read size of userhash
	uhashsize=data.ReadUInt8();
	//Xman end
	// reset all client properties; a client may not send a particular emule tag any longer
	ClearHelloProperties();
	m_byHelloPacketState = HP_HELLO; //Xman Fix Connection Collision (Sirob)
	return ProcessHelloTypePacket(&data,true); //Xman Anti-Leecher
}

bool CUpDownClient::ProcessHelloAnswer(const uchar* pachPacket, uint32 nSize)
{
	//Xman Anti-Leecher
	uhashsize=16;
	//Xman end
	CSafeMemFile data(pachPacket, nSize);
	bool bIsMule = ProcessHelloTypePacket(&data, false); //Xman Anti-Leecher
	m_byHelloPacketState |= HP_HELLOANSWER; //m_bHelloAnswerPending = false; //Xman Fix Connection Collision (Sirob)
	return bIsMule;
}

bool CUpDownClient::ProcessHelloTypePacket(CSafeMemFile* data, bool isHelloPacket) //Xman Anti-Leecher
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
	
	CString strBanReason; //Xman Anti-Leecher
	bool nonofficialopcodes = false; //Xman Anti-Leecher
	CString unknownopcode; //Xman Anti-Leecher
	bool wronghello = false; //Xman Anti-Leecher
	uint32 hellotagorder = 1; //Xman Anti-Leecher
	bool foundmd4string = false; //Xman Anti-Leecher

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
				//Xman Anti-Leecher
				if(hellotagorder!=1)
					wronghello=true;
				hellotagorder++;
				//Xman end
				break;

			case CT_VERSION:
				if (temptag.IsInt()) {
					if (bDbgInfo)
						m_strHelloInfo.AppendFormat(_T("\n  Version=%u"), temptag.GetInt());
					m_nClientVersion = temptag.GetInt();
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				//Xman Anti-Leecher
				if(hellotagorder!=2)
					wronghello=true;
				hellotagorder++;
				//Xman end
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
//Xman
#ifdef LOGTAG
						m_strHelloInfo.AppendFormat(_T("\n m_fSupportsAICH=%u"), m_fSupportsAICH);
#endif
//Xman end						
					}
				}
				else if (bDbgInfo)
					m_strHelloInfo.AppendFormat(_T("\n  ***UnkType=%s"), temptag.GetFullInfo());
				break;

			case CT_EMULE_MISCOPTIONS2:
				//	21 Reserved
				//	 1 Supports SourceExachnge2 Packets, ignores SX1 Packet Version
				//	 1 Requires CryptLayer
				//	 1 Requests CryptLayer
				//	 1 Supports CryptLayer
				//	 1 Reserved (ModBit)
				//   1 Ext Multipacket (Hash+Size instead of Hash)
				//   1 Large Files (includes support for 64bit tags)
				//   4 Kad Version
				if (temptag.IsInt()) {
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
						m_strHelloInfo.AppendFormat(_T("\n  KadVersion=%u, LargeFiles=%u ExtMultiPacket=%u CryptLayerSupport=%u CryptLayerRequest=%u CryptLayerRequires=%u m_fSupportsSourceEx2=%u"), m_byKadVersion, m_fSupportsLargeFiles, m_fExtMultiPacket, m_fSupportsCryptLayer, m_fRequestsCryptLayer, m_fRequiresCryptLayer, m_fSupportsSourceEx2);
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

			//Xman Anti-Leecher
			case 0x69: //Webcache WC_TAG_VOODOO
			case 0x6A: //Webcache WC_TAG_FLAGS
			case 0x3D: //ICS
				nonofficialopcodes=true; //Xman Anti-Leecher
				break;
			//Xman end
			default:
				//Xman Anti-Leecher
				if(thePrefs.GetAntiLeecherSnafu())
					if(ProcessUnknownHelloTag(&temptag, strBanReason))
						foundmd4string=true;
				
				unknownopcode.AppendFormat(_T(",%s"),temptag.GetFullInfo());
				//Xman end
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
	// Check for additional data in Hello packet to determine client's software version.
	//
	// *) eDonkeyHybrid 0.40 - 1.2 sends an additional Int32. (Since 1.3 they don't send it any longer.)
	// *) MLdonkey sends an additional Int32
	//
	if (data->GetLength() - data->GetPosition() == sizeof(uint32)){
		uint32 test = data->ReadUInt32();
		if (test == 'KDLM') 
		{
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
 			m_structUserCountry = theApp.ip2country->GetCountryFromIP(m_dwUserIP);
 	//EastShare End - added by AndCycle, IP to Country

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

	CClientCredits* pFoundCredits = theApp.clientcredits->GetCredit(m_achUserHash);
	if (credits == NULL){
		credits = pFoundCredits;
		//Xman Extened credit- table-arragement
		//if (!theApp.clientlist->ComparePriorUserhash(m_dwUserIP, m_nUserPort, pFoundCredits)){
		if (!theApp.clientlist->ComparePriorUserhash(m_dwUserIP, m_nUserPort, m_achUserHash)){
			//if (thePrefs.GetLogBannedClients())
				AddLeecherLogLine(false, _T("Clients: %s (%s), Banreason: Userhash changed (Found in TrackedClientsList)"), GetUserName(), ipstr(GetConnectIP())); //Xman
			Ban();
		}	
	}
	else if (credits != pFoundCredits){
		//Xman Extened credit- table-arragement
		credits->SetLastSeen(); //ensure to keep it at least 5 hours
		if(credits->GetUploadedTotal()==0 && credits->GetDownloadedTotal()==0)
			credits->MarkToDelete(); //check also if the old hash is used by an other client
		//Xman end
		// userhash change ok, however two hours "waittime" before it can be used
		credits = pFoundCredits;
		//if (thePrefs.GetLogBannedClients())
			AddLeecherLogLine(false, _T("Clients: %s (%s), Banreason: Userhash changed"), GetUserName(), ipstr(GetConnectIP())); //Xman
		Ban();
	}

	//Xman - MORPH START  always call setLinked_client 
    CFriend*        new_friend ; 
    if ((new_friend = theApp.friendlist->SearchFriend(m_achUserHash, m_dwUserIP, m_nUserPort)) != NULL)
	{ 
		// Link the friend to that client 
		m_Friend=new_friend; 
		m_Friend->SetLinkedClient(this); 
	} 
	else
	{ 
		// avoid that an unwanted client instance keeps a friend slot 
		SetFriendSlot(false); 
		if (m_Friend) m_Friend->SetLinkedClient(NULL); // morph, does this help agianst chrashing due to friend slots? 
		m_Friend=NULL;//is newfriend 
	} 
	/*  original official code:always call setLinke_client
	if ((m_Friend = theApp.friendlist->SearchFriend(m_achUserHash, m_dwUserIP, m_nUserPort)) != NULL){
		// Link the friend to that client
        m_Friend->SetLinkedClient(this);
	}
	else{
		// avoid that an unwanted client instance keeps a friend slot
		SetFriendSlot(false);
	}
	*/
	//Xman - MORPH END  always call setLinke_client

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
	CheckModIconIndex(); //>>> WiZaRd::ModIconMapper	

	if (m_bIsHybrid)
		m_fSharedDirectories = 1;

	if (thePrefs.GetVerbose() && GetServerIP() == INADDR_NONE)
		AddDebugLogLine(false, _T("Received invalid server IP %s from %s"), ipstr(GetServerIP()), DbgGetClientInfo());

	//Xman Anti-Leecher
	if(thePrefs.GetAntiLeecher())
	{
		if (theApp.GetID()!=m_nUserIDHybrid && memcmp(m_achUserHash, thePrefs.GetUserHash(), 16)==0)
		{
			strBanReason = _T("Anti Credit Hack");
			BanLeecher(strBanReason,9);
			return bIsMule;
		}
		if(strBanReason.IsEmpty()==false && thePrefs.GetAntiLeecherSnafu())
		{
			BanLeecher(strBanReason,2); //snafu = old leecher = hard ban
			return bIsMule;
		}
		if(foundmd4string && thePrefs.GetAntiLeecherSnafu())
		{
			strBanReason = _T("md4-string in opcode");
			BanLeecher(strBanReason, 15);
			return bIsMule;
		}
		if(thePrefs.GetAntiLeecherBadHello() && (m_clientSoft==SO_EMULE || (m_clientSoft==SO_XMULE && m_byCompatibleClient!=SO_XMULE)))
		{
			if(wronghello)
			{
				strBanReason= _T("wrong hello order");
				BanLeecher(strBanReason,1); //these are Leechers of a big german Leechercommunity
				return bIsMule;
			}
			if(data->GetPosition() < data->GetLength())
			{
				strBanReason= _T("extra bytes");
				BanLeecher(strBanReason,13); // darkmule (or buggy)
				return bIsMule;
			}
			if(uhashsize!=16)
			{
				strBanReason= _T("wrong Hashsize");
				BanLeecher(strBanReason,14); //new united community
				return bIsMule;
			}
			if(m_fSupportsAICH > 1  && m_clientSoft == SO_EMULE && m_nClientVersion <= MAKE_CLIENT_VERSION(CemuleApp::m_nVersionMjr, CemuleApp::m_nVersionMin, CemuleApp::m_nVersionUpd))
			{
				strBanReason= _T("Applejuice");
				BanLeecher(strBanReason,17); //Applejuice 
				return bIsMule;
			}
		}

		//if it is now a good mod, remove the reducing of score but do a second test
		if(IsLeecher()==1 || IsLeecher()==15) //category 2 is snafu and always a hard ban, need only to check 1
		{
			m_bLeecher=0; //it's a good mod now
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			old_m_strClientSoftwareFULL.Empty();	//force recheck
			old_m_pszUsername.Empty();
		}

		if(IsLeecher()==14 && isHelloPacket) //check if it is a Hello-Packet
		{
			m_bLeecher=0; //it's a good mod now
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			old_m_strClientSoftwareFULL.Empty();	//force recheck
			old_m_pszUsername.Empty();
		}

		if(IsLeecher()==17)
		{
			m_bLeecher=0; //it's a good mod now
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			old_m_strClientSoftwareFULL.Empty();	//force recheck
			old_m_pszUsername.Empty();
		}

		TestLeecher(); //test for modstring, nick and thiefs

		if(thePrefs.GetAntiLeecheremcrypt())
		{
			//Xman remark: I only check for 0.44d. 
			if(m_nClientVersion == MAKE_CLIENT_VERSION(0,44,3) && m_strModVersion.IsEmpty() && m_byCompatibleClient==0 && m_bUnicodeSupport==false && bIsMule)
			{
				if(IsLeecher()==0)
				{
					strBanReason = _T("emcrypt");
					BanLeecher(strBanReason,12); // emcrypt = no unicode for unicode version
				}
			}
			else if(IsLeecher()==12)
			{
				m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't any longer a emcrypt
			}
		}
		else if(IsLeecher()==12) 
		{
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}

		if(thePrefs.GetAntiGhost() )
		{
			if(m_strModVersion.IsEmpty() &&
				((nonofficialopcodes==true	&&	GetClientSoft()!=SO_LPHANT)
				|| ((unknownopcode.IsEmpty()==false || m_byAcceptCommentVer > 1) && m_clientSoft == SO_EMULE && m_nClientVersion <= MAKE_CLIENT_VERSION(CemuleApp::m_nVersionMjr, CemuleApp::m_nVersionMin, CemuleApp::m_nVersionUpd)))
				)
			{
				if(IsLeecher()==0)
				{
					strBanReason = _T("GhostMod");
					if(unknownopcode.IsEmpty()==false)
						strBanReason += _T(" ") + unknownopcode;
					BanLeecher(strBanReason,3); // ghost mod = webcache tag without modstring
				}
			}
			else if(IsLeecher()==3) 
			{
				m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't any longer a ghost mod
			}

		}
		else if(IsLeecher()==3)
		{
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
	}
	else if(IsLeecher()>0)
	{
		m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
		m_bLeecher=0; //unban, user doesn't want to ban it anymore
	}
	//Xman end

	UpdateFunnyNick(); //Xman Funny-Nick (Stulle/Morph)

	return bIsMule;
}

// returns 'false', if client instance was deleted!
bool CUpDownClient::SendHelloPacket(){
	if (socket == NULL){
		ASSERT(0);
		return true;
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

	m_byHelloPacketState = HP_HELLO; //m_bHelloAnswerPending = true; //Xman Fix Connection Collision (Sirob)
	return true;
}

void CUpDownClient::SendMuleInfoPacket(bool bAnswer){
	if (socket == NULL){
		ASSERT(0);
		return;
	}

	CSafeMemFile data(128);
	data.WriteUInt8((uint8)theApp.m_uCurVersionShort);
	data.WriteUInt8(EMULE_PROTOCOL);
	
	//Xman
	// Maella -Support for tag ET_MOD_VERSION 0x55-
	uint32 tagcount = 7;
	//Added by SiRoB, Don't send MOD_VERSION to client that don't support it to reduce overhead
	bool bSendModVersion = m_strModVersion.GetLength() || m_pszUsername==NULL;
	if (bSendModVersion)
		tagcount +=1;
	data.WriteUInt32(tagcount); // nr. of tags
	// Maella end
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
	if (thePrefs.CanSeeShares() != vsfaNobody) // set 'Preview supported' only if 'View Shared Files' allowed
		dwTagValue |= 128;
	CTag tag7(ET_FEATURES, dwTagValue);
	tag7.WriteTagToFile(&data);

	//Xman
	// Maella -Support for tag ET_MOD_VERSION 0x55-
	if (bSendModVersion){
		CTag tag8(ET_MOD_VERSION, MOD_VERSION);
		tag8.WriteTagToFile(&data);
	}
	// Maella end


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

	CString strBanReason = NULL; //Xman Anti-Leecher
	bool nonofficialopcodes = false; //Xman Anti-Leecher

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
				if (bDbgInfo)
					m_strMuleInfo.AppendFormat(_T("\n  ModID=%s"), m_strModVersion);
				CheckForGPLEvilDoer();
				break;

			//Xman Anti-Leecher
			case 0x3D: //ICS
				nonofficialopcodes=true; //Xman Anti-Leecher
				break;
			//Xman end
			default:
				//Xman Anti-Leecher
				if(thePrefs.GetAntiLeecher())
					ProcessUnknownInfoTag(&temptag, strBanReason);
				//Xman end
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
	CheckModIconIndex(); //>>> WiZaRd::ModIconMapper

	if (thePrefs.GetVerbose() && GetServerIP() == INADDR_NONE)
		AddDebugLogLine(false, _T("Received invalid server IP %s from %s"), ipstr(GetServerIP()), DbgGetClientInfo());

	//Xman Anti-Leecher
	if(thePrefs.GetAntiLeecher())
	{
		if(strBanReason.IsEmpty()==false && thePrefs.GetAntiLeecherSnafu())
		{
			BanLeecher(strBanReason,2); //snafu = old leecher = hard ban
			return;
		}

		TestLeecher(); //test for modstring (older clients send it with the MuleInfoPacket

		if(thePrefs.GetAntiGhost() )
		{
			if(nonofficialopcodes==true && m_strModVersion.IsEmpty() &&  GetClientSoft()!=SO_LPHANT)
			{
				if(IsLeecher()==0)
				{
					strBanReason = _T("GhostMod");
					BanLeecher(strBanReason,3); // ghost mod = webcache tag without modstring
				}
			}
			else if(IsLeecher()==3)
			{
				m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
				m_bLeecher=0; //unban, it isn't any longer a ghost mod
			}

		}
		else if(IsLeecher()==3)
		{
			m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
			m_bLeecher=0; //unban, user doesn't want to ban it anymore
		}
	}
	else if(IsLeecher()>0)
	{
		m_strBanMessage.Format(_T("unban - Client %s"),DbgGetClientInfo());
		m_bLeecher=0; //unban, user doesn't want to ban it anymore
	}
	//Xman end
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
	
	// Servers send a FIN right in the data packet on check connection, so we need to force the response immediate
	bool bForceSend = theApp.serverconnect->AwaitingTestFromIP(GetConnectIP());
	socket->SendPacket(packet, true, true, 0, bForceSend);
	
	m_byHelloPacketState |= HP_HELLOANSWER; //Xman Fix Connection Collision (Sirob)
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

	if( theApp.clientlist->GetBuddy() && theApp.IsFirewalled() )
		tagcount += 2;

	//Xman ModID
	//Xman - Added by SiRoB, Don't send MOD_VERSION to client that don't support it to reduce overhead
	//Xman send modtring only to non Leechers or Modthiefs
	bool bSendModVersion = (m_strModVersion.GetLength() || m_pszUsername==NULL) && (IsLeecher()==0 || IsLeecher()==6);
	if (bSendModVersion) tagcount+=1;
	//Xman END   - Added by SiRoB, Don't send MOD_VERSION to client that don't support it to reduce overhead


	data->WriteUInt32(tagcount);


	// eD2K Name

	// TODO implement multi language website which informs users of the effects of bad mods
	//Xman Anti-Leecher
	if(IsLeecher()==12)
	{
		//emcrypt
		CTag tagName(CT_NAME, _T("You are using a spyware infected emule version") );
		tagName.WriteTagToFile(data, utf8strRaw);
	}
	//Xman send Nickaddon only to non Leechers or NickThiefs/ModThiefs
	else if(IsLeecher()==0 || IsLeecher()==11 || IsLeecher()==6) 
	{
		CTag tagName(CT_NAME, (!m_bGPLEvildoer) ? thePrefs.GetUserNick() + _T(' ') + str_ANTAddOn + MOD_NICK_ADD : _T("Please use a GPL-conform version of eMule") ); //Xman Anti-Leecher: simple Anti-Thief
		tagName.WriteTagToFile(data, utf8strRaw);
	}
	//else send the standard-nick
	else
	{
		CTag tagName(CT_NAME, (!m_bGPLEvildoer) ? thePrefs.GetUserNick()  : _T("Please use a GPL-conform version of eMule") ); //Xman Anti-Leecher: simple Anti-Thief
		tagName.WriteTagToFile(data, utf8strRaw);
	}
	//Xman end

	// eD2K Version
	CTag tagVersion(CT_VERSION,EDONKEYVERSION);
	tagVersion.WriteTagToFile(data);

	// eMule UDP Ports
	uint32 kadUDPPort = 0;
	if(Kademlia::CKademlia::IsConnected())
	{
		kadUDPPort = thePrefs.GetUDPPort();
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
	const UINT uNoViewSharedFiles	= (thePrefs.CanSeeShares() == vsfaNobody) ? 1 : 0; // for backward compatibility this has to be a 'negative' flag
	const UINT uMultiPacket			= 1;
	const UINT uSupportPreview		= (thePrefs.CanSeeShares() != vsfaNobody) ? 1 : 0; // set 'Preview supported' only if 'View Shared Files' allowed
	const UINT uPeerCache			= 1;
	const UINT uUnicodeSupport		= 1;
	const UINT nAICHVer				= 1;
	CTag tagMisOptions1(CT_EMULE_MISCOPTIONS1, 
				(nAICHVer				<< 29) |
				(uUnicodeSupport		<< 28) |
				(uUdpVer				<< 24) |
				(uDataCompVer			<< 20) |
				(uSupportSecIdent		<< 16) |
				(uSourceExchange1Ver		<< 12) |
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

	CTag tagMisOptions2(CT_EMULE_MISCOPTIONS2, 
//		(RESERVED				     )
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

	//Xman - modID
	if (bSendModVersion) {
		CTag tagMODVersion(ET_MOD_VERSION, MOD_VERSION);
		tagMODVersion.WriteTagToFile(data);
	}
	//Xman end - modID
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

void CUpDownClient::ProcessMuleCommentPacket(const uchar* pachPacket, uint32 nSize)
{
	if (reqfile && reqfile->IsPartFile())
	{
		CSafeMemFile data(pachPacket, nSize);
		uint8 uRating = data.ReadUInt8();
		if (thePrefs.GetLogRatingDescReceived() && uRating > 0)
			AddDebugLogLine(false, GetResString(IDS_RATINGRECV), m_strClientFilename, uRating);
		CString strComment;
		UINT uLength = data.ReadUInt32();
		if (uLength > 0)
		{
			// we have to increase the raw max. allowed file comment len because of possible UTF8 encoding.
			if (uLength > MAXFILECOMMENTLEN*3)
				uLength = MAXFILECOMMENTLEN*3;
			strComment = data.ReadString(GetUnicodeSupport()!=utf8strNone, uLength);
			if (thePrefs.GetLogRatingDescReceived() && !strComment.IsEmpty())
				AddDebugLogLine(false, GetResString(IDS_DESCRIPTIONRECV), m_strClientFilename, strComment);

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
						break;
					}
					strFilter = thePrefs.GetCommentFilter().Tokenize(_T("|"), iPos);
				}
			}
		}
		if (!strComment.IsEmpty() || uRating > 0)
		{
			m_strFileComment = strComment;
			m_uFileRating = uRating;
			reqfile->UpdateFileRatingCommentAvail();
		}
	}
}

bool CUpDownClient::Disconnected(LPCTSTR pszReason, bool bFromSocket, UpStopReason reason) // Maella -Upload Stop Reason-
{
	ASSERT( theApp.clientlist->IsValidClient(this) );

	//If this is a KAD client object, just delete it!
	SetKadState(KS_NONE);

#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
    if (GetUploadState() == US_UPLOADING || GetUploadState() == US_ERROR //Xman Xtreme Mod
	|| GetUploadState() == US_WAITCALLBACK || GetUploadState() == US_WAITCALLBACKKAD
	|| GetUploadState() == US_WAITCALLBACKXS // NEO: XSB - [XSBuddy]
	)
#else
    if (GetUploadState() == US_UPLOADING || GetUploadState()==US_ERROR) //Xman Xtreme Mod
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
	{
		//if (thePrefs.GetLogUlDlEvents() && GetUploadState()==US_UPLOADING && m_fSentOutOfPartReqs==0 && !theApp.uploadqueue->IsOnUploadQueue(this))
		//	DebugLog(_T("Disconnected client removed from upload queue and waiting list: %s"), DbgGetClientInfo());
		theApp.uploadqueue->RemoveFromUploadQueue(this, CString(_T("CUpDownClient::Disconnected: ")) + pszReason , reason); // Maella -Upload Stop Reason-
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
		if (m_ePeerCacheDownState == PCDS_WAIT_CACHE_REPLY || m_ePeerCacheDownState == PCDS_DOWNLOADING)
			theApp.m_pPeerCache->DownloadAttemptFailed();
		//Xman remark: the only reason can be a socket-error/timeout
		SetDownloadState(DS_ONQUEUE, CString(_T("Disconnected: ")) + pszReason, CUpDownClient::DSR_SOCKET);	// Maella -Download Stop Reason-
	}
	else{
		// ensure that all possible block requests are removed from the partfile
		ClearDownloadBlockRequests();
		/* //Xman Code Imrpovement moved down
		if(GetDownloadState() == DS_CONNECTED){
			theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
			theApp.downloadqueue->RemoveSource(this);
	    }
		*/
	}

	// we had still an AICH request pending, handle it
	if (IsAICHReqPending()){
		m_fAICHRequested = FALSE;
		CAICHHashSet::ClientAICHRequestFailed(this);
	}

	// The remote client does not have to answer with OP_HASHSETANSWER *immediatly* 
	// after we've sent OP_HASHSETREQUEST. It may occure that a (buggy) remote client 
	// is sending use another OP_FILESTATUS which would let us change to DL-state to DS_ONQUEUE.
	if (((GetDownloadState() == DS_REQHASHSET) || m_fHashsetRequesting) && (reqfile))
        reqfile->hashsetneeded = true;

	ASSERT( theApp.clientlist->IsValidClient(this) );

	// NEO: TCR - [TCPConnectionRetry] -- Xanatos -->
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

	//check if this client is needed in any way, if not delete it
	bool bDelete = true;
	switch(m_nUploadState){
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
	}

	switch(m_nUploadState){
		case US_CONNECTING:
		{
			//if (thePrefs.GetLogUlDlEvents())
			//    AddDebugLogLine(DLP_VERYLOW, true,_T("Removing connecting client from upload list: %s Client: %s"), pszReason, DbgGetClientInfo());
			//Xman uploading problem client
			{
				theApp.uploadqueue->RemoveFromUploadQueue(this,pszReason ,CUpDownClient::USR_SOCKET );
				isupprob=true;
				//back to queue
				theApp.uploadqueue->AddClientDirectToQueue(this);
				m_bAddNextConnect=true;
				bDelete = false;
				break;
			}
			//xman end
		}
		case US_WAITCALLBACK:
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		case US_WAITCALLBACKKAD: // NEO: LUC - [LowIDUploadCallBack]
		case US_WAITCALLBACKXS: // NEO: XSB - [XSBuddy]
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
		case US_ERROR: //Xman Xtreme Mod: this clients get IP-Filtered!
		//Xman 5.1 why we should keep it ?
		case US_BANNED:
			bDelete = true;
	}

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
	bool bAddDeadSource = true;

	
	if(m_nUploadState!=US_BANNED) //Xman DLP - Anti-Leecher / Code-Improvement
	switch(m_nDownloadState){
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
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		case DS_WAITCALLBACKKAD: // NEO: LUC - [LowIDUploadCallBack]
		case DS_WAITCALLBACKXS: // NEO: XSB - [XSBuddy]
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			if (bAddDeadSource)
				theApp.clientlist->m_globDeadSourceList.AddDeadSource(this);
			theApp.downloadqueue->AddFailedTCPFileReask(); //Xman Xtreme Mod: count the failed TCP-connections
			if(m_bUDPPending) theApp.downloadqueue->AddFailedUDPFileReasks(); //Xman Xtreme Mod: for correct statistics, if it wasn't counted on connection established //Xman x4 test
		case DS_ERROR: //Xman Xtreme Mod: this clients get IP-Filtered!
			bDelete = true;
	}

	if (GetChatState() != MS_NONE){
		//Xman Code Improvement
		if(bDelete==true)
			theApp.downloadqueue->RemoveSource(this);
		//Xman end
		bDelete = false;
		theApp.emuledlg->chatwnd->chatselector.ConnectingResult(this,false);
	}
	
	if (!bFromSocket && socket){
		ASSERT( theApp.listensocket->IsValidSocket(socket) );
		socket->Safe_Delete();
	}
	socket = 0;

    if (m_iFileListRequested){
		LogWarning(LOG_STATUSBAR, GetResString(IDS_SHAREDFILES_FAILED), GetUserName());
        m_iFileListRequested = 0;
	}

	if (m_Friend)
		theApp.friendlist->RefreshFriend(m_Friend);

	//Xman Code Improvement: don't refresh list-item on deletion
	if(bDelete==false)
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);

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
		m_byHelloPacketState = HP_NONE; //m_bHelloAnswerPending = false; //Xman Fix Connection Collision (Sirob)
		m_fQueueRankPending = 0;
		m_fFailedFileIdReqs = 0;
		m_fUnaskQueueRankRecv = 0;
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
		return false;
	}
}

//Returned bool is not if the TryToConnect is successful or not..
//false means the client was deleted!
//true means the client was not deleted!
bool CUpDownClient::TryToConnect(bool bIgnoreMaxCon, CRuntimeClass* pClassSocket)
{

	// do not try to connect to source which are incompatible with our encryption setting (one requires it, and the other one doesn't supports it)
	if ( (RequiresCryptLayer() && !thePrefs.IsClientCryptLayerSupported()) || (thePrefs.IsClientCryptLayerRequired() && !SupportsCryptLayer()) ){
#if defined(_DEBUG) || defined(_BETA)
		// TODO: Remove after testing
		AddDebugLogLine(DLP_DEFAULT, false, _T("Rejected outgoing connection because CryptLayer-Setting (Obfuscation) was incompatible %s"), DbgGetClientInfo() );
#endif
		if(Disconnected(_T("CryptLayer-Settings (Obfuscation) incompatible"))){
			delete this;
			return false;
		}
		else
			return true;
	}

	//Xman Fix Connection Collision (Sirob)
	bool socketnotinitiated = (socket == NULL || socket->GetConState() == ES_DISCONNECTED);
	if (socketnotinitiated) 
	{
	//Xman end Fix Connection Collision (Sirob)
		if (theApp.listensocket->TooManySockets() && !bIgnoreMaxCon /*&& !(socket && socket->IsConnected())*/ ) //Xman Fix Connection Collision (Sirob)
	{
		if(Disconnected(_T("Too many connections")))
		{
			delete this;
			return false;
		}
		return true;
	}

	uint32 uClientIP = GetIP();
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
				AddDebugLogLine(true, GetResString(IDS_IPFILTERED), ipstr(uClientIP), theApp.ipfilter->GetLastHit());
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

	if( GetKadState() == KS_QUEUED_FWCHECK )
		SetKadState(KS_CONNECTING_FWCHECK);

		if ( HasLowID() )
	{
		if(!theApp.DoCallback(this))
		{
			//We cannot do a callback!
			if (GetDownloadState() == DS_CONNECTING)
				SetDownloadState(DS_LOWTOLOWIP);
			else if (GetDownloadState() == DS_REQHASHSET)
			{
				SetDownloadState(DS_ONQUEUE);
				reqfile->hashsetneeded = true;
			}
			if (GetUploadState() == US_CONNECTING)
			{
				if(Disconnected(_T("LowID->LowID and US_CONNECTING")))
				{
					delete this;
					return false;
				}
			}
			return true;
		}

		//We already know we are not firewalled here as the above condition already detected LowID->LowID and returned.
		//If ANYTHING changes with the "if(!theApp.DoCallback(this))" above that will let you fall through 
		//with the condition that the source is firewalled and we are firewalled, we must
		//recheck it before the this check..
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		if( HasValidBuddyID() && !GetBuddyIP() && !GetBuddyPort() && !theApp.serverconnect->CanCallback(GetServerIP(), GetServerPort()))
#else
		if( HasValidBuddyID() && !GetBuddyIP() && !GetBuddyPort() && !theApp.serverconnect->IsLocalServer(GetServerIP(), GetServerPort()))
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		{
			//This is a Kad firewalled source that we want to do a special callback because it has no buddyIP or buddyPort.
			if( Kademlia::CKademlia::IsConnected() )
			{
				//We are connect to Kad
				if( Kademlia::CKademlia::GetPrefs()->GetTotalSource() > 0 || Kademlia::CSearchManager::AlreadySearchingFor(Kademlia::CUInt128(GetBuddyID())))
				{
					//There are too many source lookups already or we are already searching this key.
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
					if (GetDownloadState() == DS_CONNECTING)
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
					SetDownloadState(DS_TOOMANYCONNSKAD);
					return true;
				}
			}
		}
	}

		//Xman Fix Connection Collision (Sirob)
		//Useless with socketnotinitiated
		/*
	if (!socket || !socket->IsConnected())
	{
		*/
		//Xman end
		if (socket)
			socket->Safe_Delete();
		if (pClassSocket == NULL)
			pClassSocket = RUNTIME_CLASS(CClientReqSocket);
		socket = static_cast<CClientReqSocket*>(pClassSocket->CreateObject());
		socket->SetClient(this);
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		// If we are here and booth cleints are firewalled, we already know booth support Nat Traversal
		if( HasLowID() && (theApp.IsFirewalled() || GetUserPort() == 0)) // GetUserPort() == 0 for manualy added debug cleints
			socket->InitNatSupport();
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		if (!socket->Create())
		{
			socket->Safe_Delete();
			return true;
		}
		//Xman Fix Connection Collision (Sirob)
		//Useless with socketnotinitiated
		/*
	}
	else
	{
			ConnectionEstablished();
		return true;
	}
		*/
		//Xman end
	// MOD Note: Do not change this part - Merkur
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
	// has low ID and dont have a active tunel
	if (HasLowID() && !(socket->GetNatLayer() && theApp.natmanager->PrepareConnect(this)))
#else
	if (HasLowID())
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
	{
		if (GetDownloadState() == DS_CONNECTING)
			SetDownloadState(DS_WAITCALLBACK);
		if (GetUploadState() == US_CONNECTING)
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
			SetUploadState(US_WAITCALLBACK);
#else
		{
			if(Disconnected(_T("LowID and US_CONNECTING")))
			{
				delete this;
				return false;
			}
			return true;
		}
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
		if (theApp.serverconnect->CanCallback(m_dwServerIP,m_nServerPort))
#else
		if (theApp.serverconnect->IsLocalServer(m_dwServerIP,m_nServerPort))
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			//if(theApp.serverconnect->IsLowID())
			if(socket->GetNatLayer())
			{
				if(CNatTunnel* tunnel = theApp.natmanager->PrepareTunnel(this))
				{
					tunnel->PrepareCallback((uchar*)&m_nUserIDHybrid, ed2k_id);
					if(GetUDPPort() != 0)
						theApp.natmanager->SendNatPing(GetConnectIP(),GetUDPPort(),true,0,0,true);
					else
						tunnel->SendCallback();
				}
			}
			else
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
		{
			Packet* packet = new Packet(OP_CALLBACKREQUEST,4);
			PokeUInt32(packet->pBuffer, m_nUserIDHybrid);
			if (thePrefs.GetDebugServerTCPLevel() > 0 || thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__CallbackRequest", this);
			theStats.AddUpDataOverheadServer(packet->size);
			theApp.serverconnect->SendPacket(packet);
			}
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
			ASSERT(IsCallback());
#else
			SetDownloadState(DS_WAITCALLBACK);
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
				}
				else
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
		// Note: now we have an own "buddy" substitute over XS so if we can we pick it, for nat anyway,
		//			for normal callbacks to, this way we dont drop every secund reask and at the same we lower the kad load
		if(GetXsBuddyIP() && HasValidHash())
		{
			if( IsCallback() )
			{
				// NEO: NATT - [NatTraversal]
				//if(theApp.IsFirewalled())
				if(socket->GetNatLayer())
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
			}
		}
		else
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
			{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			//if ( !GetRemoteQueueRank() || m_bReaskPending )
			if ( IsCallback(true) )
#else
			if ( GetUploadState() == US_NONE && (!GetRemoteQueueRank() || m_bReaskPending) )
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			{
				if( !HasValidBuddyID() )
				{
					theApp.downloadqueue->RemoveSource(this);
					if(Disconnected(_T("LowID and US_NONE and QR=0")))
					{
						delete this;
						return false;
					}
					return true;
				}
				
				if( !Kademlia::CKademlia::IsConnected() )
				{
					//We are not connected to Kad and this is a Kad Firewalled source..
					theApp.downloadqueue->RemoveSource(this);
					{
						if(Disconnected(_T("Kad Firewalled source but not connected to Kad.")))
						{
							delete this;
							return false;
						}
						return true;
					}
				}
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
                if( IsCallback() )
#else
                if( GetDownloadState() == DS_WAITCALLBACK )
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
				{
					if( GetBuddyIP() && GetBuddyPort())
					{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
						//if(Kademlia::CKademlia::IsFirewalled())
						if(socket->GetNatLayer())
						{
							if(CNatTunnel* tunnel = theApp.natmanager->PrepareTunnel(this))
							{
								tunnel->PrepareCallback(GetBuddyID(), kad_id, GetBuddyIP(), GetBuddyPort(), true);
								if(GetUDPPort() != 0)
									theApp.natmanager->SendNatPing(GetConnectIP(),GetUDPPort(),true,0,0,true);
								else
									tunnel->SendCallback();
							}
						}
						else
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
						{
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
						}
#ifdef NATTUNNELING // NEO: LUC - [LowIDUploadCallBack] -- Xanatos -->
						if( GetUploadState() == US_WAITCALLBACK )
							SetUploadState(US_WAITCALLBACKKAD);

						if( GetDownloadState() == DS_WAITCALLBACK )
#endif //NATTUNNELING // NEO: LUC END <-- Xanatos --
						SetDownloadState(DS_WAITCALLBACKKAD);
					}
					else
					{
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
						if(Kademlia::CSearchManager::StartSearch(findSource))
						{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
							//if(Kademlia::CKademlia::IsFirewalled())
							if(socket->GetNatLayer())
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
				}
			}
			else
			{
				if (GetDownloadState() == DS_WAITCALLBACK)
				{
					m_bReaskPending = true;
					SetDownloadState(DS_ONQUEUE);
				}
			}
		}
	}
	// MOD Note - end
	else
	{
		if (!Connect())
			return false; // client was deleted!
	}
	//Xman Fix Connection Collision (Sirob)
	}
	else if(IsBanned()) //call this only with my fix inside!
	{
		//Xman first check for banned clients. Could occur during our communication
		//if (thePrefs.GetLogBannedClients()) 
			AddDebugLogLine(false, _T("Refused to connect to banned client %s"), DbgGetClientInfo());
		m_cFailed=5; //force deletion //Xman 
		if (Disconnected(_T("Banned IP")))
		{
			delete this;
			return false;
		}
	}
	else if (CheckHandshakeFinished()) {
		ConnectionEstablished();
		return true;
	}
	else if (m_byHelloPacketState == HP_NONE) {
		if (!SendHelloPacket())
			return false; // client was deleted!
		DebugLog(LOG_SUCCESS, _T("[FIX CONNECTION COLLISION] Already initiated socket, OP_HELLO have been sent to client: %s"), DbgGetClientInfo());
	} /*else
		DebugLog(LOG_SUCCESS, _T("[FIX CONNECTION COLLISION] Already initiated socket, OP_HELLO already sent and waiting an OP_HELLOANSWER from client: %s"), DbgGetClientInfo());
		*/
	//Xman end Fix Connection Collision (Sirob)

	return true;
}

bool CUpDownClient::Connect()
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
	if (!SendHelloPacket())
		return false; // client was deleted!
	return true;
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
			&& (theApp.uploadqueue->waitinglist.GetSize()>10 //check if we have clients queued, otherwise inetmaybedown gives a wrong value
			|| theApp.last_ip_change==0) //we just started the client //Xman new adapter selection 
			&& theApp.last_ip_change  < ::GetTickCount() - MIN2MS(2) //only once in 2 minutes
			) 
			theApp.m_bneedpublicIP=true;
		if(theApp.IsConnected()) //only free the upload if we are connected. important! otherwise we would have problems with nafc-adapter on a hotstart
		{
			theApp.internetmaybedown=false;
			theApp.last_traffic_reception=::GetTickCount();
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
		AddDebugLogLine(false, _T("internet-connection was possible down. ask client for ip: %s"), DbgGetClientInfo()); 
	}

	// ok we have a connection, lets see if we want anything from this client
	
	// check if we should use this client to retrieve our public IP
	if (theApp.GetPublicIP() == 0 && theApp.serverconnect->IsConnected() && m_fPeerCache)
		SendPublicIPRequest();

	//Xman end

#ifdef NATTUNNELING // NEO: RTP - [ReuseTCPPort] -- Xanatos -->
	if (thePrefs.ReuseTCPPort() && theApp.GetPublicPort() == 0 && theApp.serverconnect->IsConnected() && theApp.IsFirewalled() && m_fSupportsNatTraversal)
		SendPublicPortRequest();
#endif //NATTUNNELING // NEO: RTP END <-- Xanatos --

	switch(GetKadState())
	{
		case KS_CONNECTING_FWCHECK:
            SetKadState(KS_CONNECTED_FWCHECK);
			break;
		case KS_CONNECTING_BUDDY:
		case KS_INCOMING_BUDDY:
			SetKadState(KS_CONNECTED_BUDDY);
			break;
	}

#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
	if(GetXsBuddyStatus() == XB_CONNECTING)
		SendXsBuddyRequest();
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --

	if (GetChatState() == MS_CONNECTING || GetChatState() == MS_CHATTING)
		theApp.emuledlg->chatwnd->chatselector.ConnectingResult(this,true);

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
	if(SupportsL2HAC() && HasLowID() && CanDoL2HAC()){
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
				SetUploadState(US_UPLOADING);
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__AcceptUploadReq", this);
				Packet* packet = new Packet(OP_ACCEPTUPLOADREQ,0);
				theStats.AddUpDataOverheadFileRequest(packet->size);
				socket->SendPacket(packet,true);
				//Xman find best sources
				//Xman: in every case, we add this client to our downloadqueue
				CKnownFile* partfile = theApp.downloadqueue->GetFileByID(this->GetUploadFileID());
				if (partfile && partfile->IsPartFile())
					theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)partfile,this,true);
				//Xman end
			}
	}

	if (m_iFileListRequested == 1)
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend(m_fSharedDirectories ? "OP__AskSharedDirs" : "OP__AskSharedFiles", this);
        Packet* packet = new Packet(m_fSharedDirectories ? OP_ASKSHAREDDIRS : OP_ASKSHAREDFILES,0);
		theStats.AddUpDataOverheadOther(packet->size);
		socket->SendPacket(packet,true,true);
	}


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
//>>> WiZaRd::ECR [Spike2]
			case SO_SHAREAZA2:
			case SO_SHAREAZA3:
			case SO_SHAREAZA4:
			//case 40:
//<<< WiZaRd::ECR [Spike2]
				m_clientSoft = SO_SHAREAZA;
				pszSoftware = _T("Shareaza");
				break;
			case SO_LPHANT:
				m_clientSoft = SO_LPHANT;
				pszSoftware = _T("lphant");
				break;
//>>> WiZaRd::ECR [Spike2]
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
//<<< WiZaRd::ECR [Spike2]
			default:
//>>> WiZaRd::ECR [Spike2]
				if (m_bIsML || m_byCompatibleClient == SO_MLDONKEY || m_byCompatibleClient == SO_MLDONKEY2 || m_byCompatibleClient == SO_MLDONKEY3){
				//if (m_bIsML || m_byCompatibleClient == SO_MLDONKEY){
//<<< WiZaRd::ECR [Spike2]
					m_clientSoft = SO_MLDONKEY;
					pszSoftware = _T("MLdonkey");
				}
//>>> WiZaRd::ECR [Spike2]
				else if (m_bIsHybrid || m_byCompatibleClient == SO_EDONKEYHYBRID){
				//else if (m_bIsHybrid){
//<<< WiZaRd::ECR [Spike2]
					m_clientSoft = SO_EDONKEYHYBRID;
					pszSoftware = _T("eDonkeyHybrid");
				}
				else if (m_byCompatibleClient != 0){
//>>> WiZaRd::ECR [Spike2]
					if (StrStrI(m_pszUsername,_T("shareaza")))
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
//<<< WiZaRd::ECR [Spike2]
					{
					m_clientSoft = SO_XMULE; // means: 'eMule Compatible'
					pszSoftware = _T("eMule Compat");
				}
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
//>>> WiZaRd::ECR [Spike2]
			else if (m_clientSoft == SO_EMULEPLUS)
				{
				if(nClientUpVersion > 1)
						iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%u%c"), pszSoftware, nClientMajVersion, nClientMinVersion, _T('a') + nClientUpVersion - 1);
				else
					iLen = _sntprintf(szSoftware, ARRSIZE(szSoftware), _T("%s v%u.%ua"), pszSoftware, nClientMajVersion, nClientMinVersion);
				}
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

//>>> WiZaRd::ECR [Spike2]
	if (m_bIsHybrid || m_byCompatibleClient == SO_EDONKEYHYBRID){
	//if (m_bIsHybrid){
//<<< WiZaRd::ECR [Spike2]
		m_clientSoft = SO_EDONKEYHYBRID;
		// seen:
		// 105010	0.50.10
		// 10501	0.50.1
		// 10300	1.3.0
//>>> WiZaRd::ECR [Spike2]
		// 10212	1.2.2
		// 10211	1.2.1
//<<< WiZaRd::ECR [Spike2]
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

//>>> WiZaRd::ECR [Spike2]
	//if (m_bIsML || iHashType == SO_MLDONKEY){
	if (m_bIsML || iHashType == SO_MLDONKEY || iHashType == SO_OLD_MLDONKEY){
//<<< WiZaRd::ECR [Spike2]
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
//>>> WiZaRd::ECR [Spike2]
		//return SO_MLDONKEY;
		return SO_OLD_MLDONKEY;
	else if (m_achUserHash[5] == 0x0E && m_achUserHash[14] == 0x6F) 
		return SO_MLDONKEY;
//<<< WiZaRd::ECR [Spike2]
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

	UpdateFunnyNick(); //Xman Funny-Nick (Stulle/Morph)
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
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__PublicKey", this);
	socket->SendPacket(packet,true,true);
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
	m_uFileRating = 0;
	m_strFileComment.Empty();

	delete m_pReqFileAICHHash;
	m_pReqFileAICHHash = NULL;
}

bool CUpDownClient::IsBanned() const
{
	return theApp.clientlist->IsBannedClient(GetConnectIP()); //Xman official bugfix, was: //GetIP()); 
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
	
	if (m_fPreviewAnsPending || thePrefs.CanSeeShares()==vsfaNobody || (thePrefs.CanSeeShares()==vsfaFriends && !IsFriend()))
		return;
	
	m_fPreviewAnsPending = 1;
	CKnownFile* previewFile = theApp.sharedfiles->GetFileByID(pachPacket);
	if (previewFile == NULL){
		SendPreviewAnswer(NULL, NULL, 0);
	}
	else{
		previewFile->GrabImage(4,0,true,450,this);
	}
}

void CUpDownClient::ProcessPreviewAnswer(const uchar* pachPacket, uint32 nSize){
	if (m_fPreviewReqPending == 0)
		return;
	m_fPreviewReqPending = 0;
	CSafeMemFile data(pachPacket,nSize);
	uchar Hash[16];
	data.ReadHash16(Hash);
	uint8 nCount = data.ReadUInt8();
	if (nCount == 0){
		LogError(LOG_STATUSBAR, GetResString(IDS_ERR_PREVIEWFAILED), GetUserName());
		return;
	}
	CSearchFile* sfile = theApp.searchlist->GetSearchFileByHash(Hash);
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
		socket->SendPacket(packet);
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

	(void)m_iModIconIndex; //>>> WiZaRd::ModIconMappings
	CHECK_OBJ(socket);
	CHECK_PTR(credits);
	CHECK_PTR(m_Friend);
	CHECK_OBJ(reqfile);
	(void)m_abyUpPartStatus;
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
	ASSERT( m_nSourceFrom >= SF_SERVER && m_nSourceFrom <= SF_LINK );
	CHECK_BOOL(m_bRemoteQueueFull);
	CHECK_BOOL(m_bCompleteSource);
	CHECK_BOOL(m_bReaskPending);
	CHECK_BOOL(m_bUDPPending);
	CHECK_BOOL(m_bTransferredDownMini);
	CHECK_BOOL(m_bUnicodeSupport);
	ASSERT( m_nKadState >= KS_NONE && m_nKadState <= KS_CONNECTED_BUDDY );
	m_PendingBlocks_list.AssertValid();
	m_DownloadBlocks_list.AssertValid();
	ASSERT( m_nChatstate >= MS_NONE && m_nChatstate <= MS_UNABLETOCONNECT );
	(void)m_strFileComment;
	(void)m_uFileRating;
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
			_T("BuddyConnected")
	};
	if (GetKadState() >= ARRSIZE(apszState))
		return _T("*Unknown*");
	return apszState[GetKadState()];
}

CString CUpDownClient::DbgGetFullClientSoftVer() const
{
	// X-Ray :: FiXeS :: Bugfix :: Start :: WiZaRd
	// one should never "append" via + operator! really bad style AND dangerous!
	/*
	if (GetClientModVer().IsEmpty())
		return GetClientSoftVer();
	return GetClientSoftVer() + _T(" [") + GetClientModVer() + _T(']');
	*/
	CString ret = _T("");
	if (m_strModVersion.IsEmpty())
		ret = m_strClientSoftware;
	else
		ret.Format(_T("%s [%s]"), m_strClientSoftware, m_strModVersion);
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
			str.Format(_T("%08x - Invalid client instance"), this);
		}
	}
	return str;
}

bool CUpDownClient::CheckHandshakeFinished() const
{
	if (m_byHelloPacketState != HP_BOTH /*m_bHelloAnswerPending == true*/) //Xman Fix Connection Collision (Sirob)
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
				strState = GetResString(IDS_ONQUEUE);
			break;
		case DS_DOWNLOADING:
			strState = GetResString(IDS_TRANSFERRING);
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			if(socket && socket->HaveNatLayer())
				strState.Append(GetResString(IDS_X_LOW2LOW_ADDON));
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
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

	//if (thePrefs.GetPeerCacheShow())
	//{
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
	//}

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
            //Xman Xtreme Upload
			if(GetPayloadInBuffer() == 0 && GetNumberOfRequestedBlocksInQueue() == 0 && thePrefs.IsExtControlsEnabled()) 
				strState = GetResString(IDS_US_STALLEDW4BR);
			else  if(GetPayloadInBuffer() == 0 && thePrefs.IsExtControlsEnabled()) 
				strState = GetResString(IDS_US_STALLEDREADINGFDISK);
			else
				strState = GetResString(IDS_TRANSFERRING);
			//Xman end
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->
			if(socket && socket->HaveNatLayer())
				strState.Append(GetResString(IDS_X_LOW2LOW_ADDON));
#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --
			break;
	}

	//if (thePrefs.GetPeerCacheShow())
	//{
		switch (m_ePeerCacheUpState)
		{
		case PCUS_WAIT_CACHE_REPLY:
			strState += _T(" CacheWait");
			break;
		case PCUS_UPLOADING:
			strState += _T(" PeerCache");
			break;
		}
		if (m_ePeerCacheUpState != PCUS_NONE && m_bPeerCacheUpHit)
			strState += _T(" Hit");
	//}

	//Xman 4.2
	if(GetDownloadState()==DS_DOWNLOADING)
		strState = _T("<<") + strState;
	//Xman end

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
				uint32 test=dwIP;
				CString tmp;
				tmp.Format(_T("recebido um ip do cliente: %u.%u.%u.%u, NAFC-Adapter sera checado"), (uint8)test, (uint8)(test>>8), (uint8)(test>>16), (uint8)(test>>24));
				AddLogLine(false,tmp);
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
						AddLogLine(false, _T("Kad Connexao detectou uma mudanca de IP, mudado IP de %s para %s, todas as fontes serao perguntadas novamente"), ipstr(theApp.last_valid_ip), ipstr(dwIP));
				}
				else
				{
				theApp.clientlist->TrigReaskForDownload(false);
						AddLogLine(false, _T("A conexao Kad detectou uma mundanca de IP, mudado ip de  %s para %s, todas as fontes serao perguntadas em 10 minutos"), ipstr(theApp.last_valid_ip), ipstr(dwIP));
				}
				SetNextTCPAskedTime(::GetTickCount() + FILEREASKTIME); //not for this source
				}
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
		if (m_fFailedFileIdReqs == 6)
		{
			//Xman we filter not ban!
			/*
			if (theApp.clientlist->GetBadRequests(this) < 2)
				theApp.clientlist->TrackBadRequest(this, 1);
			if (theApp.clientlist->GetBadRequests(this) == 2){
				theApp.clientlist->TrackBadRequest(this, -2); // reset so the client will not be rebanned right after the ban is lifted
				//Ban(_T("FileReq flood")); //Xman we filter not ban!
			}
			*/
			throw CString(/*thePrefs.GetLogBannedClients() ?*/ _T("FileReq flood") /*: _T("")*/); //Xman
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


//EastShare Start - added by AndCycle, IP to Country
// Superlexx - client's location
CString	CUpDownClient::GetCountryName(bool longName) const {
	return theApp.ip2country->GetCountryNameFromRef(m_structUserCountry,longName);
}

int CUpDownClient::GetCountryFlagIndex() const {
	return m_structUserCountry->FlagIndex;
}
//MORPH START - Changed by SiRoB, ProxyClient
void CUpDownClient::ResetIP2Country(uint32 m_dwIP){
	m_structUserCountry = theApp.ip2country->GetCountryFromIP((m_dwIP)?m_dwIP:m_dwUserIP);
}
//MORPH END - Changed by SiRoB, ProxyClient
//EastShare End - added by AndCycle, IP to Country


//Xman Anti-Leecher
bool CUpDownClient::ProcessUnknownHelloTag(CTag *tag, CString &pszReason)
{
#ifndef LOGTAG
	//Xman DLP
	if(pszReason.IsEmpty()==false)
		return false;
	//Xman end
#endif

	//Xman DLP
	if(theApp.dlp->IsDLPavailable()==false)
		return false;

	bool foundmd4=false;

	LPCTSTR strSnafuTag=theApp.dlp->DLPCheckHelloTag(tag->GetNameID());
	if (strSnafuTag!=NULL)
	{
		pszReason.Format(_T("Suspect Hello-Tag: %s"),strSnafuTag);
	}
	//Xman end

	if (strSnafuTag==NULL && tag->IsStr() && tag->GetStr().GetLength() >= 32)
		foundmd4=true;

#ifdef LOGTAG
		if(m_byCompatibleClient==0 && GetHashType() == SO_EMULE )
		{
			AddDebugLogLine(false,_T("Unknown HelloTag: 0x%x, %s, client:%s"), tag->GetNameID(), tag->GetFullInfo(), DbgGetClientInfo());
		}
#endif

	return foundmd4;
}
void CUpDownClient::ProcessUnknownInfoTag(CTag *tag, CString &pszReason)
{
#ifndef LOGTAG
	//Xman DLP
	if(pszReason.IsEmpty()==false)
		return;
	//Xman end
#endif


	//Xman DLP
	if(theApp.dlp->IsDLPavailable()==false)
		return;
	LPCTSTR strSnafuTag=theApp.dlp->DLPCheckInfoTag(tag->GetNameID());
	if (strSnafuTag!=NULL)
	{
		pszReason.Format(_T("Suspect eMuleInfo-Tag: %s"),strSnafuTag);
	}
	//Xman end
#ifdef LOGTAG
		if(m_byCompatibleClient==0 && GetHashType() == SO_EMULE )
		{
			AddDebugLogLine(false,_T("Unknown InfoTag: 0x%x, %s, client:%s"), tag->GetNameID(), tag->GetFullInfo(), DbgGetClientInfo());
		}
#endif

}
//Xman end
//Xman
// - show requested files (sivka)
void CUpDownClient::ShowRequestedFiles()
{
	CString fileList;
	fileList += _T("[")+GetResString(IDS_CLIENT)+_T(": ")+(GetUserName())+_T("]");
	fileList += _T("\n\nList of Downloading Files:\n");
	fileList += _T("__________________________\n\n") ; 
	if (reqfile  && reqfile->IsPartFile())
	{
		fileList += reqfile->GetFileName(); 
		for(POSITION pos = m_OtherRequests_list.GetHeadPosition();pos!=0;m_OtherRequests_list.GetNext(pos))
		{
			fileList += _T("\n") ; 
			fileList += m_OtherRequests_list.GetAt(pos)->GetFileName(); 
		}
		for(POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos!=0;m_OtherNoNeeded_list.GetNext(pos))
		{
			fileList += _T("\n") ;
			fileList += m_OtherNoNeeded_list.GetAt(pos)->GetFileName();
		}
	}
	else
		fileList += _T("You have not requested a file from this user.");
	fileList += _T("\n\n\nList of Uploading Files:\n");
	fileList += _T("__________________________\n\n") ; 
	CKnownFile* uploadfile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
	if(uploadfile)
		fileList += uploadfile->GetFileName();
	else
		fileList += _T("This user has not requested a file from you.");
	AfxMessageBox(fileList,MB_OK);

}
//Xman end

//Xman Anti-Leecher: simple Anti-Thief
//gives back the Xtreme-Mod Version as float
//0 if it isn't a Xtreme-Mod
//remark: highest subversion allowed is 9!
float CUpDownClient::GetXtremeVersion(CString modversion) const
{
	if(modversion.GetLength()<10)
		return 0.0f;

	if(modversion.Left(7).CompareNoCase(_T("Xtreme "))!=0)
		return 0.0f;

	return (float)_tstof(modversion.Mid(7));

}
//Xman end


//Xman 5.1
// Maella -Unnecessary Protocol Overload-
void CUpDownClient::CalculateJitteredFileReaskTime(bool longer)
{
	if(longer==false)
	{
		// Maella -Unnecessary Protocol Overload-
		// Remark: a client will be remove from an upload queue after 3*FILEREASKTIME (~1 hour)
		//         a two small value increases the traffic + causes a banishment if lower than 10 minutes
		//         srand() is already called a few times..
		uint32 jitter = rand() * (35*6) / RAND_MAX; // 0..3.5 minutes, keep in mind integer overflow
		m_jitteredFileReaskTime = FILEREASKTIME - (60*1000) + 1000*jitter - (2*60*1000); // -2..+2 minutes, keep the same average overload
		//Xman: result between 26 and 29.5 this is useful to use TCP-Connection from older clients
	}
	else
		m_jitteredFileReaskTime = FILEREASKTIME + (3*60*1000); //32 min
		//this gives the client the chance to connect first
		//this connection can be used by Xtreme, see partfile->process
}
//Xman end

//Xman Funny-Nick (Stulle/Morph)
void CUpDownClient::UpdateFunnyNick()
{
	if(m_pszUsername == NULL || 
		!IsEd2kClient() || //MORPH - Changed by Stulle, no FunnyNick for http DL
		_tcsnicmp(m_pszUsername, _T("http://emule"),12) != 0 &&
		_tcsicmp(m_pszUsername, _T("")) != 0)
		return;

	if(m_pszFunnyNick!=NULL)
		return; //why generate a new one ? userhash can't change without getting banned

	// preffix table
	const static LPCTSTR apszPreFix[] =
	{
		_T("Sapo "),			//0
			_T("João "),	//1
			_T("Negão "),	//2
			_T("Maria "),	//3
			_T("Zezinho "),	//4
			_T("Bruno "),
			_T("Tevez "),
			_T("Carlos "),
			_T("Charles "),
			_T("Eneida "),
			_T("Miudo "),		//10
			_T("Renato "),
			_T("Marcelo "),
			_T("Boizaum "),
			_T("Maluquinho "),
			_T("Moreno "),
			_T("Jonnhy "),
			_T("Zé "),
			_T("Lindão "),
			_T("Deus "),
			_T("Super "),		//20
			_T("Maluco "),
			_T("Chimpas "),
			_T("Mauricio "),
			_T("Gaucho"),
			_T("Electro "),
			_T("Jaffet "),
			_T("Secão "),
			_T("Eduardo "),
			_T("Evandro "),
			_T("Augusto "),	//30
			_T("Pirão "),
			_T("Sapo "),
			_T("Alexandre "),
			_T("Caculé "),
			_T("Cilmo "),
			_T("Novato "),
			_T("Irmã "),
			_T("Maltez "),
			_T("Everton "),
			_T("Popo "),		//40
			_T("Jack "),
			_T("Dieginho "),
			_T("Cadu "),
			_T("I"),
			_T("Sou "),
			_T("Jaffet "),
			_T("Jaffet "),
			_T("Ronaldo "),
			_T("Ronaldinho "),
			_T("Rocky "),		//50
			_T("Fred "),
			_T("Robinho "),
			_T("Cafu "),
			_T("Dida "),
			_T("Jaffet "),
			_T("Tafarel "),
			_T("Japones "),
			_T("Americano "),
			_T("Roberto "),
			_T("Adriano "),		//60
			_T("Jaffet "),
			_T("Boizaum "),
			_T("Pirão "),
			_T("Novato "),
			_T("Tomba "),
			_T("X"),
			_T("Biel "),
			_T("Zanoni "),
			_T("Mauricio "),
			_T("Mc"),			//70
			_T("Carla "),
			_T("Sheila "),
			_T("Jonny "),
			_T("Fausto "),
			_T("Jana "),
			_T("Marcel "),
			_T("Marcão "),
			_T("Batatinha "),
			_T("Manda Chuva "),
			_T("Seu "),		//80
			_T("Almondega "),
			_T("Chimpas "),
			_T("Jaffet "),
			_T("Boizaum "),
			_T("Bruno "),
			_T("Matador "),		//86
	};
#define NB_PREFIX 87 
#define MAX_PREFIXSIZE 15

	// suffix table
	const static LPCTSTR apszSuffix[] =
	{
		_T("Do mal"),		//0
			_T("Assassino"),
			_T("da Night"),
			_T("Bonitão"),
			_T("Gostosão"),
			_T("Lindão"),
			_T("Surfista"),
			_T("Tequila"),
			_T("Gatinho"),
			_T("do Surf"),
			_T("Peludo"),		//10
			_T("Malvado"),
			_T("Moreno"),
			_T("Loirão"),
			_T("do Funk"),
			_T("da Selva"),
			_T("Pegador"),
			_T("Pop"),
			_T("Pop"),
			_T("Pop"),
			_T("Malvado"),		//20
			_T("do Amor"),
			_T("Inocente"),
			_T("Patricinha"),
			_T("Fortão"),
			_T("Pegador"),
			_T("Matador"),
			_T("Assasino"),
			_T("Tevez"),
			_T("Pop"),
			_T("Macho"),		//30
			_T("Moreninho"),
			_T("Facinho"),
			_T("Doido"),
			_T("Rambo"),
			_T("do Mato"),
			_T("Matador"),
			_T("Diferente"),
			_T("Smurf"),
			_T("Bonitão"),
			_T("Estrela"),	//40
			_T("Tigrão"),
			_T("do Funk"),
			_T("Wolf"),
			_T("Borboletinha"),
			_T("Cavalo"),
			_T("Godzilla"),
			_T("Branquelo"),
			_T("Brasileiro"),
			_T("Mendigo"), 
			_T("Exterminador"),	//50
			_T("Cabeçudo"),
			_T("Gaucho"),
			_T("Pegador"),
			_T("de Bermuda")	//54
	};
#define NB_SUFFIX 55 
#define MAX_SUFFIXSIZE 12

	//--- if we get an id, we can generate the same random name for this user over and over... so much about randomness :) ---
	if(m_achUserHash)
	{
		uint32	x=0x7d726d62; // < xrmb :)
		uint8	a=m_achUserHash[5]  ^ m_achUserHash[7]  ^ m_achUserHash[15] ^ m_achUserHash[4];
		uint8	b=m_achUserHash[11] ^ m_achUserHash[9]  ^ m_achUserHash[12] ^ m_achUserHash[1];
		uint8	c=m_achUserHash[3]  ^ m_achUserHash[14] ^ m_achUserHash[6]  ^ m_achUserHash[13];
		uint8	d=m_achUserHash[2]  ^ m_achUserHash[0]  ^ m_achUserHash[10] ^ m_achUserHash[8];
		uint32	e=(a<<24) + (b<<16) + (c<<8) + d;
		srand(e^x);
	}

	if (m_pszFunnyNick) {
		delete[] m_pszFunnyNick;
		m_pszFunnyNick = NULL;
	}

	CString tag = _T("[FN]");;
	uint8 uTagLength = 4+2; //one space + /0
	
	m_pszFunnyNick = new TCHAR[uTagLength+MAX_PREFIXSIZE+MAX_SUFFIXSIZE];
	// pick random suffix and prefix
	{
		_tcscpy(m_pszFunnyNick, tag);
		_tcscat(m_pszFunnyNick, _T(" "));
		_tcscat(m_pszFunnyNick, apszPreFix[rand()%NB_PREFIX]);
		_tcscat(m_pszFunnyNick, apszSuffix[rand()%NB_SUFFIX]);
	}

	//--- make the rand random again ---
	if(m_achUserHash)
		srand((unsigned)time(NULL));
}
//Xman end
//>>> WiZaRd::ModIconMapper
void	CUpDownClient::CheckModIconIndex()
{
	if(m_strModVersion.IsEmpty())
		m_iModIconIndex = MODMAP_NONE;
	else
		m_iModIconIndex = theApp.theModIconMap->GetIconIndexForModstring(m_strModVersion);
}

int	CUpDownClient::GetModIconIndex() const
{
	//if he's a bad one, just return that icon straight away
//	if(IsBadGuy()) //needs CA support
//		return MODMAP_BADGUY;
	return m_iModIconIndex;
}
//<<< WiZaRd::ModIconMapper

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


	uint32 uProtMod =	
						((1		& 0x01)	<< 8 ) | // NEO: L2HAC - [LowID2HighIDAutoCallback]
						((0		& 0x01)	<< 7 ) | // NEO: SCT - [SubChunkTransfer]
						((1		& 0x01)	<< 6 ) | // NEO: XC - [ExtendedComments]
						((1		& 0x01)	<< 5 ) | // NEO: NXS - [NeoXS]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
						((1		& 0x01)	<< 4 ) |
#endif //NATTUNNELING // NEO: NATT END
						((0		& 0x01)	<< 3 ) | // UDP Mod File Status
						((0		& 0x01)	<< 2 ) | // ICSv2 Regular Version
						((1		& 0x01)	<< 1 ) | // LowID UDP Ping Support (notifyes a fix form xman1 that allow the remote low ID to use udp reasks) // NEO: MOD - [NewUploadState]
						((0		& 0x01)	<< 0 );  // Unsolicited Part Status (idea from netfinity, allows our client ro recieve filestatus at any time) // NEO: USPS - [UnSolicitedPartStatus]
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
	m_fSupportsModProt = 0;
	m_fSupportNeoXS = 0;
	m_fSupportL2HAC = 0; // NEO: L2HAC - [LowID2HighIDAutoCallback]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
	m_fSupportsNatTraversal = 0;
#endif //NATTUNNELING // NEO: NATT END
	m_fLowIDUDPPingSupport = 0;

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
					//m_fSubChunksSupport			= ((temptag.GetInt() >> 7 ) & 0x01) != 0; // NEO: SCT - [SubChunkTransfer]
					//m_fExtendedComments			= ((temptag.GetInt() >> 6 ) & 0x01) != 0; // NEO: XC - [ExtendedComments]
					m_fSupportNeoXS				= ((temptag.GetInt() >> 5 ) & 0x01) != 0; // NEO: NXS - [NeoXS]
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
					m_fSupportsNatTraversal		= ((temptag.GetInt() >> 4 ) & 0x01) != 0; // Support Nat Traversal
#endif //NATTUNNELING // NEO: NATT END
					//m_fExtendedUDPStatus		= ((temptag.GetInt() >> 3 ) & 0x01) != 0; // UDP Mod File Status
					//m_fIncompletePartVer2		= ((temptag.GetInt() >> 2 ) & 0x01) != 0; // ICSv2 new official Version // NEO: ICS - [InteligentChunkSelection] 
					m_fLowIDUDPPingSupport		= ((temptag.GetInt() >> 1 ) & 0x01) != 0; // LowID UDP Ping Support (notifyes a fix form xman1 that allow the remote low ID to use udp reasks) // NEO: MOD - [NewUploadState]
					//m_fUnsolicitedPartStatus	= ((temptag.GetInt() >> 0 ) & 0x01) != 0; // Unsolicited Part Status (idea from netfinity, allows our client ro recieve filestatus at any time) // NEO: USPS - [UnSolicitedPartStatus]
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
	const uint8 uSupportsNatTraversal = this->SupportsNatTraversal();
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
			if(pBuffer + uLength > pBuffEnd || uLength == 0){
				CString err;
				err.Format(_T("Wrong Neo XS Tag Entry %i size %i!!!"),(int)uID,(int)uLength);
				throw err;
			}

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
	return (m_iFileListRequested // X?
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
