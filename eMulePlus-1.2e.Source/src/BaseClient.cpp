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

#include "stdafx.h"
#include "updownclient.h"
#include "emule.h"
#include "server.h"
#include "Friend.h"
#include "ServerList.h"
#include "UploadQueue.h"
#include "Clientlist.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "IP2Country.h"
#include "IPFilter.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

//	members of CUpDownClient
//	which are used by down and uploading functions

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OLD_SOCKETS_ENABLED
CUpDownClient::CUpDownClient(CClientReqSocket *pRequestSocket) :
	m_pendingBlocksList(3), m_downloadBlocksList(3)
{
	m_pRequestSocket = pRequestSocket;
	SetDLRequiredFile(NULL);
	Init();
	m_fUserInfoWasReceived = 1;
}
#endif //OLD_SOCKETS_ENABLED
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient::CUpDownClient(uint16 uPort, uint32 dwUserID, uint32 dwSrvIP, uint16 uSrvPort, CPartFile *pReqPartFile, EnumUserIDType eIDType) :
	m_pendingBlocksList(3), m_downloadBlocksList(3)
{
#ifdef OLD_SOCKETS_ENABLED
	m_pRequestSocket = NULL;
#endif //OLD_SOCKETS_ENABLED

	Init();

	if ((eIDType == UID_ED2K) && !IsLowID(dwUserID))
		SetUserIDHybrid(ntohl(dwUserID));
	else
		SetUserIDHybrid(dwUserID);
	m_uUserPort = uPort;

	if (!HasLowID())
	{
		m_dwConnectIP = ntohl(m_dwUserIDHybrid);
		m_strFullUserIP.Format( _T("%u.%u.%u.%u"),
								static_cast<byte>(m_dwConnectIP),
								static_cast<byte>(m_dwConnectIP >> 8),
								static_cast<byte>(m_dwConnectIP >> 16),
								static_cast<byte>(m_dwConnectIP >> 24) );
		m_uUserCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwConnectIP);
	}
	m_dwServerIP = dwSrvIP;
	m_uServerPort = uSrvPort;
	SetDLRequiredFile(pReqPartFile);
	m_fUserInfoWasReceived = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::Init()
{
	EMULE_TRY

	m_strFullUserIP.Empty();
	m_pCredits = NULL;
	m_dwEnteredConnectedState = 0;
	m_uLastPartAsked = 0xFFFF;
	m_dwClientSumDLDataRateOverLastNMeasurements = 0;
	m_dwClientSumDLTickOverLastNMeasurements = 0;
	m_dwTransferredInLastPeriod = 0;
	m_bLimitlessDL = true;
	m_uUDPPort = 0;
	m_byteFailedCount = 0;
	m_dwBanTime = 0;
	m_dwTransferredUp = 0;
	m_dwAskedCount = 0;
	m_byteIncorrectBlockRequests = 0;
	m_dwDownAskedCount = 0;
	m_dwUpDataRate = 0;
	m_strUserName.Empty();
	m_uUserCountryIdx = NO_COUNTRY_INFO;
	m_dwUserIP = 0;
	m_dwConnectIP = 0;
	SetUserIDHybrid(0);
	m_uServerPort = 0;
	m_bIsOnLan = false;
	m_eBanState = BAN_CLIENT_NONE;
	m_iFileListRequested = m_iFileListRequestedSave = 0;
	m_dwLastUpRequest = 0;
	m_bEmuleProtocol = false;
	m_eStrCodingFormat = cfLocalCodePage;
	m_uNeededParts = 0;
	m_bCommentDirty = false;
	m_bReaskPending = false;
	m_byteNumUDPPendingReqs = 0;
	m_bUDPPending = false;
	m_byteEmuleVersion = 0;
	m_dwClientVersion = 0;
	m_strModString.Empty();
	m_strClientSoft = _T("???");
	m_uUserPort = 0;
	m_uPartCount = 0;
	m_uUpPartCount = 0;
	m_pbytePartStatuses = NULL;
	m_pbyteUpPartStatuses = NULL;
	m_uAvailUpPartCount = 0;
	m_uAvailPartCount = 0;
	ResetLastAskedTime();
	m_eDownloadState = DS_NONE;
	m_dwUploadTime = 0;
	m_dwTransferredDown = 0;
	m_dwSessionDownloadedData = 0;
	m_dwDownDataRate = 0;
	m_byteFailedFileRequestsCount = 0;
	m_eUploadState = US_NONE;
	m_dwLastBlockReceived = 0;
	m_byteDataCompVer = 0;
	m_dwPlusVers = 0;
	m_byteUDPVer = 0;
	m_byteSourceExchange1Ver = 0;
	m_byteAcceptCommentVer = 0;
	m_byteExtendedRequestsVer = 0;
	m_uRemoteQueueRank = 0;
	m_dwLastSourceRequest = 0;
	m_dwLastSourceAnswer = 0;
	m_byteCompatibleClient = 0;
	m_bIsHybrid = false;
	m_eChatState = MS_NONE;
	m_dwAwayMessageResendCount = 0;
	m_bIsML = false;
	m_pFriend = NULL;
	m_eRating = PF_RATING_NONE;
	m_strComment.Empty();
	m_dwLastGotULDataTime = 0;
	m_eClientSoft = SO_UNKNOWN;
	md4clr(m_userHash);
	m_fRequestingHashSet = 0;
	m_fNoViewSharedFiles = 0;
	m_fSupportsAskSharedDirs = 0;
	m_fNoDataForRemoteClient = 1;
	m_fAddNextConnect = 0;
	m_fSupportsLargeFiles = 0;
	m_fServerWasChanged = 0;
	m_fRxWrongFileRequest = 0;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
	m_fSentCancelTransfer = 0;
	m_fSupportsSourceEx2 = 0;
	m_fSupportsMultiPacket = 0;
	m_fSupportsExtMultiPacket = 0;
	m_fPeerCache = 0;
	m_fIdenThief = 0;

#ifdef OLD_SOCKETS_ENABLED
	if (m_pRequestSocket != NULL)
	{
		SOCKADDR_IN	sockAddr = {0};
		int			iSockAddrLen = sizeof(sockAddr);

		m_pRequestSocket->GetPeerName(reinterpret_cast<SOCKADDR*>(&sockAddr), &iSockAddrLen);
		m_dwUserIP = sockAddr.sin_addr.s_addr;
		ipstr(&m_strFullUserIP, m_dwConnectIP = m_dwUserIP);
		m_uUserCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwConnectIP);
	}
#endif //OLD_SOCKETS_ENABLED
	m_uUpCompleteSourcesCount = 0;
	m_dwUpCompleteSourcesTime = 0;
	md4clr(m_reqFileHash);

	m_dwLastL2HACExec = 0;
	m_dwL2HACTime = 0;
	m_bL2HACEnabled = false;

	m_eSecureIdentState = IS_UNAVAILABLE;
	m_dwLastSignatureIP = 0;
	m_byteSupportSecIdent = 0;
	m_eInfoPacketsReceived = IP_NONE;

	m_byteActionsOnNameChange = 0;
	m_bHasUserNameForbiddenStrings = false;
	m_bIsMODNameChanged = false;
	m_bHasMODNameForbiddenStrings = false;
	m_bIsCommunity = false;

	m_bIsHandshakeFinished = false;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient::~CUpDownClient()
{
	EMULE_TRY

	g_App.m_pDownloadList->RemoveSource(this);
	g_App.m_pClientList->RemoveClient(this);

	if (m_pFriend != NULL)
		m_pFriend->SetLinkedClient(NULL);

#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pListenSocket->DeleteSocket(m_pRequestSocket);
#endif //OLD_SOCKETS_ENABLED

	delete[] m_pbytePartStatuses;
	m_pbytePartStatuses	= NULL;
	delete[] m_pbyteUpPartStatuses;
	m_pbyteUpPartStatuses = NULL;

	ClearUploadBlockRequests();

	while (!m_requestedFilesList.IsEmpty())
		delete m_requestedFilesList.RemoveHead();

	ClearDownloadBlocksList();
	ClearPendingBlocksList();

	if (m_eRating != PF_RATING_NONE || m_strComment.GetLength() > 0)
	{
		m_eRating = PF_RATING_NONE;
		m_strComment.Empty();
		m_pReqPartFile->UpdateFileRatingCommentAvail();
	}

#ifdef OLD_SOCKETS_ENABLED
	DEBUG_ONLY (g_App.m_pListenSocket->Debug_ClientDeleted(this));
#endif //OLD_SOCKETS_ENABLED
	SetUploadFileID(NULL);

	if (m_eChatState != MS_NONE)
		g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.EndSession(this, false);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ClearHelloProperties()
{
	m_uUDPPort = 0;
	m_byteUDPVer = 0;
	m_byteDataCompVer = 0;
	m_byteEmuleVersion = 0;
	m_byteSourceExchange1Ver = 0;
	m_byteAcceptCommentVer = 0;
	m_byteExtendedRequestsVer = 0;
	m_byteCompatibleClient = 0;
	m_byteSupportSecIdent = 0;
	m_dwClientVersion = 0;
	m_fSupportsAskSharedDirs = 0;
	m_fSupportsLargeFiles = 0;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
	m_fSupportsSourceEx2 = 0;
	m_fSupportsMultiPacket = 0;
	m_fSupportsExtMultiPacket = 0;
	m_fPeerCache = 0;
	m_fIdenThief = 0;
	m_dwPlusVers = 0;
	m_dwL2HACTime = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::ProcessHelloPacket(BYTE *pbytePacket, uint32 dwSize)
{
	CSafeMemFile	packetStream(pbytePacket, dwSize);
	byte			byteHashSize;

	packetStream.Read(&byteHashSize, 1);
//	Reset all client properties; a client may not send a particular emule tag any longer
	ClearHelloProperties();
	return ProcessHelloTypePacket(packetStream);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ProcessHelloAnswer(byte *pbytePacket, uint32 dwSize)
{
	EMULE_TRY

//	Ban the client if he changes a userhash and was already connected one time
	if (m_fUserInfoWasReceived != 0
		&& HasValidHash() && (md4cmp(GetUserHash(), pbytePacket) != 0))
	{
		Ban(BAN_CLIENT_HASH_STEALER);
		if (!g_App.m_pPrefs->IsCMNotLog())
		{
			AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Ban client %s (%s:%u) changed userhash from %s to %s on HelloAns"),
				GetClientNameWithSoftware(), GetFullIP(), GetUserPort(),
				HashToString(GetUserHash()), HashToString(pbytePacket));
		}
	}

	CSafeMemFile packetStream(pbytePacket, dwSize);
	ProcessHelloTypePacket(packetStream);
	SetHandshakeStatus(true);
	m_fUserInfoWasReceived = 1;
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::ProcessHelloTypePacket(CSafeMemFile &packetStream)
{
	m_bIsHybrid = false;
	m_bIsML = false;
	m_fNoViewSharedFiles = 0;
	m_eStrCodingFormat = cfLocalCodePage;

	packetStream.Read(&m_userHash, 16);

	uint32	dwTagCount, dwEmuleTags = 0;

	packetStream.Read(&m_dwUserIDHybrid, 4);
	m_bIsLowID = IsLowID(m_dwUserIDHybrid);

	packetStream.Read(&m_uUserPort, 2);
	packetStream.Read(&dwTagCount, 4);

	bool	bCrashed = false, bPrTag = false;

	for (uint32 i = 0; i < dwTagCount; i++)
	{
		CTag	TempTag;

		TempTag.FillFromStream(packetStream, cfUTF8);
		switch (TempTag.GetTagID())
		{
			case CT_NAME:
				if (TempTag.IsStr())
				{
					if (!TempTag.IsStringValueEmpty())
					{
						if (!TempTag.IsStringValueEqual(m_strUserName))
						{
							TempTag.GetStringValue(&m_strUserName);
							m_byteActionsOnNameChange |= AONC_FORBIDDEN_NAME_CHECK | AONC_COMMUNITY_CHECK;
						}
						break;
					}
				}
				bCrashed = true;
				m_strUserName = _T("[Invalid user name]");
				break;

			case CT_VERSION:
				if (TempTag.IsInt())
					m_dwClientVersion = TempTag.GetIntValue();
				break;

			case CT_PORT:
				if (TempTag.IsInt())
					m_uUserPort = static_cast<uint16>(TempTag.GetIntValue());
				break;

			case CT_MOD_VERSION:
				if (TempTag.IsStr())
				{
					if (!TempTag.IsStringValueEqual(m_strModString))
					{
						TempTag.GetStringValue(&m_strModString);
						m_bIsMODNameChanged = true;
					}
				}
				else if (TempTag.IsInt())
					m_strModString.Format(_T("ModID=%u"), TempTag.GetIntValue());
				else
					m_strModString = _T("ModID=<Unknwon>");
				break;

			case CT_EMULE_UDPPORTS:
				// 16 KAD Port
				// 16 UDP Port
				if (TempTag.IsInt())
				{
					m_uUDPPort = static_cast<uint16>(TempTag.GetIntValue());
					dwEmuleTags |= 1;
				}
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
				//	1 PeerCache supported
				//	1 No 'View Shared Files' supported
				//	1 MultiPacket
				//  1 Preview
				if (TempTag.IsInt())
				{
					uint32	dwOpt = TempTag.GetIntValue();

					dwOpt >>= 1;	//skip Preview
					m_fSupportsMultiPacket = dwOpt & 0x01;
					dwOpt >>= 1;
					m_fNoViewSharedFiles = dwOpt & 0x01;
					dwOpt >>= 1;
					m_fPeerCache = dwOpt & 0x01;
					dwOpt >>= 1;
					m_byteAcceptCommentVer = static_cast<byte>(dwOpt & 0x0F);
					dwOpt >>= 4;
					m_byteExtendedRequestsVer = static_cast<byte>(dwOpt & 0x0F);
					dwOpt >>= 4;
					m_byteSourceExchange1Ver = static_cast<byte>(dwOpt & 0x0F);
					dwOpt >>= 4;
					m_byteSupportSecIdent = static_cast<byte>(dwOpt & 0x0F);
					dwOpt >>= 4;
					m_byteDataCompVer = static_cast<byte>(dwOpt & 0x0F);
					dwOpt >>= 4;
					m_byteUDPVer = static_cast<byte>(dwOpt & 0x0F);
					dwOpt >>= 4;
#ifdef _UNICODE
					m_eStrCodingFormat = (dwOpt & 0x01) ? cfUTF8 : cfLocalCodePage;
					dwOpt >>= 1;
#endif

					dwEmuleTags |= 2;
				}
				break;

			case CT_EMULE_MISCOPTIONS2:
				//  21 Reserved
				//   1 Supports SourceExachange2 Packets, ignores SX1 Packet Version
				//   1 Requires CryptLayer
				//   1 Requests CryptLayer
				//   1 Supports CryptLayer
				//   1 Reserved (ModBit)
				//   1 Ext Multipacket (Hash+Size instead of Hash)
				//   1 Large Files (includes support for 64bit tags)
				//   4 Kad Version
				if (TempTag.IsInt())
				{
					uint32	dwOpt = TempTag.GetIntValue();

					dwOpt >>= 4;	//skip Kad Version
					m_fSupportsLargeFiles = dwOpt & 0x01;
					dwOpt >>= 1;
					m_fSupportsExtMultiPacket = dwOpt & 0x01;
					dwOpt >>= 1;
					dwOpt >>= 1;	//Reserved
					SetCryptLayer(dwOpt);
					dwOpt >>= 3;
					m_fSupportsSourceEx2 = dwOpt & 0x01;
					dwOpt >>= 1;
//					dwEmuleTags |= 8;
				}
				break;

			case CT_EMULE_VERSION:
				//  8 Compatible Client ID
				//  7 Maj Version
				//  7 Min Version
				//  3 Upd Version
				//  7 Bld Version
				if (TempTag.IsInt())
				{
					m_dwClientVersion = TempTag.GetIntValue();
					m_byteCompatibleClient = static_cast<byte>(m_dwClientVersion >> 24);
					m_dwClientVersion &= 0x00FFFFFF;
					m_byteEmuleVersion = 0x99;
					m_fSupportsAskSharedDirs = 1;
					dwEmuleTags |= 4;
				}
				break;

			default:
			//	Since eDonkeyHybrid 1.3 is no longer sending the additional Int32
			//	at the end of the Hello packet, we use the "pr=1" tag to determine them
				if ((TempTag.GetTagName() != NULL) && (TempTag.GetTagName()[0] == 'p') && (TempTag.GetTagName()[1] == 'r'))
					bPrTag = true;
		}
	}

//	Check the other client's server parameters and add it to our server list
	uint32 		dwServerIP = 0;
	uint16 		uServerPort = 0;

	packetStream.Read(&dwServerIP, 4);
	packetStream.Read(&uServerPort, 2);
	m_fServerWasChanged = (m_dwServerIP != dwServerIP) || (m_uServerPort != uServerPort);
	if (m_fServerWasChanged != 0)
	{
		m_dwServerIP = dwServerIP;
		m_uServerPort = uServerPort;
	//	If the client didn't try to crash us with a blank name and the "add servers to server list on connect" preference is on...
		if (!bCrashed && g_App.m_pPrefs->GetAddServersFromClients())
		{
		//	... and server IP and port are valid (fast check) and the server isn't already in our server list
			if ((m_dwServerIP != 0) && (m_uServerPort != 0) && !g_App.m_pServerList->GetServerByIP(m_dwServerIP, m_uServerPort))
			{
				CServer		*pServer = new CServer(m_uServerPort, ipstr(m_dwServerIP));

				pServer->SetListName(pServer->GetAddress());
				pServer->SetPreference(PR_LOW);	// not very reliable way to add, so make it low priority
				if (!g_App.m_pMDlg->m_wndServer.m_ctlServerList.AddServer(pServer, true))
					delete pServer;
			}
		}
	}

// Check for additional data in Hello packet to determine client's software
// - eDonkeyHybrid 0.40 - 1.2 sends an additional Int32 (since 1.3 they don't send it anymore)
// - MLdonkey sends an additional Int32
	if (packetStream.GetLength() - packetStream.GetPosition() == 4)
	{
		uint32		dwTest;

		packetStream.Read(&dwTest, 4);
		if (dwTest == 'KDLM')
		{
			m_bIsML = true;
		}
		else
			m_bIsHybrid = true;
	}

	SOCKADDR_IN	sockAddr = {0};
	int			iSockAddrLen = sizeof(sockAddr);

#ifdef OLD_SOCKETS_ENABLED
	m_pRequestSocket->GetPeerName((SOCKADDR*)&sockAddr, &iSockAddrLen);
#endif
	m_dwUserIP = sockAddr.sin_addr.s_addr;
	ipstr(&m_strFullUserIP, m_dwUserIP);
	if (m_dwConnectIP != m_dwUserIP)
	{
		m_dwConnectIP = m_dwUserIP;
		m_uUserCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwConnectIP);
	}

//	If Lancast is enabled check to see if this client is on the LAN
	m_bIsOnLan = ((g_App.m_pPrefs->GetLancastEnabled()) && ((g_App.m_pPrefs->GetLancastIP() & g_App.m_pPrefs->GetLancastSubnet()) == (m_dwUserIP & g_App.m_pPrefs->GetLancastSubnet())));

//	(a) If this is a HighID user, store the ID in the Hybrid format.
//	(b) Some older clients will not send an ID, these client are HighID users that are not connected to a server.
//	(c) The ed2k users (that are not connected to Kad) with a *.*.*.0 IPs will look like a LowID user they are actually
//		a HighID user. They can be detected easily because they will send an ID that is the same as their IP.
	if (!HasLowID() || m_dwUserIDHybrid == 0 || m_dwUserIDHybrid == m_dwUserIP || m_bIsOnLan)
		SetUserIDHybrid(ntohl(m_dwUserIP));

//	Get client credits
	uchar key[16];
	md4cpy(key, m_userHash);
	m_pCredits = g_App.m_pClientCreditList->GetCredit(key);

	if ((m_pFriend = g_App.m_pFriendList->SearchFriend(key, m_dwUserIP, m_uUserPort)) != NULL)
	{
	//	Link the friend to that client
		m_pFriend->SetLinkedClient(this);

		if (m_eChatState == MS_CONNECTING)
		{
			TCITEM		tcNewItem;

			tcNewItem.mask = TCIF_TEXT;
			tcNewItem.pszText = m_strUserName.GetBuffer(256);
			g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.SetItem(g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.GetTabByClient(this), &tcNewItem);
		}
	}
//	Check if at least CT_EMULEVERSION was received, all other tags are optional
	bool bIsMule = ((dwEmuleTags & 0x04) != 0);

	if (!bIsMule && bPrTag)
		m_bIsHybrid = true;
	if (m_bIsHybrid)
		m_fSupportsAskSharedDirs = 1;

	ReGetClientSoft();

//--- detectmystolenhash ---
#ifdef OLD_SOCKETS_ENABLED
//	Check if remote client has our HASH
	if (md4cmp(m_userHash, g_App.m_pPrefs->GetUserHash()) == 0
		&& g_App.m_pPrefs->IsCounterMeasures())
	{
		Ban(BAN_CLIENT_USE_OUR_HASH);
	}
#endif //OLD_SOCKETS_ENABLED

//	Check famous stolen hashes/names
	if (HasUserNameForbiddenStrings())
		Ban(BAN_CLIENT_KWOWN_LEECHER);

	m_eInfoPacketsReceived |= IP_EDONKEYPROTPACK;
	if (bIsMule)
	{
		m_bEmuleProtocol = true;
		m_eInfoPacketsReceived |= IP_EMULEPROTPACK;
	}

	return bIsMule;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendHelloPacket()
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
//	If IP is filtered, don't greet him but disconnect...
	if (m_pRequestSocket != NULL)
	{
		CMemFile	packetStream(128);
		byte		byteHashSize = 16;

		packetStream.Write(&byteHashSize, 1);
		SendHelloTypePacket(packetStream);

		Packet		*pPacket = new Packet(&packetStream);

		pPacket->m_eOpcode = OP_HELLO;
		g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
		m_pRequestSocket->SendPacket(pPacket, true);
	}
#endif //OLD_SOCKETS_ENABLED
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendMuleInfoPacket(bool bAnswer)
{
	EMULE_TRY
#ifdef OLD_SOCKETS_ENABLED
	if (m_pRequestSocket)
	{
		CMemFile packetStream(128);
	//	Header of the info packet
		byte uClientVersion = CURRENT_VERSION_SHORT;
		packetStream.Write(&uClientVersion, 1);
		byte uProtVersion = EMULE_PROTOCOL;
		packetStream.Write(&uProtVersion, 1);

		CWrTag	tagWr;
		uint32	dwTagCount = 10;

		packetStream.Write(&dwTagCount, 4);
	//	Body of the info packet
		tagWr.WriteToFile(ET_COMPRESSION, 1, packetStream);
		tagWr.WriteToFile(ET_UDPVER, 4, packetStream);
		tagWr.WriteToFile(ET_UDPPORT, g_App.m_pPrefs->GetUDPPort(), packetStream);
		tagWr.WriteToFile(ET_SOURCEEXCHANGE, SOURCEEXCHANGE1_VERSION, packetStream);
		tagWr.WriteToFile(ET_COMMENTS, 1, packetStream);
		tagWr.WriteToFile(ET_MOD_PLUS, CURRENT_PLUS_VERSION, packetStream);
		tagWr.WriteToFile(ET_EXTENDEDREQUEST, 2, packetStream);
		tagWr.WriteToFile(ET_MOD_VERSION, _T(PLUS_VERSION_STR), packetStream);
		tagWr.WriteToFile(ET_L2HAC, FILEREASKTIME, packetStream);
		tagWr.WriteToFile(ET_FEATURES, (uint32)(g_App.m_pClientCreditList->CryptoAvailable() ? 3 : 0), packetStream);

		Packet		*pPacket = new Packet(&packetStream, OP_EMULEPROT);
		if (!bAnswer)
			pPacket->m_eOpcode = OP_EMULEINFO;
		else
			pPacket->m_eOpcode = OP_EMULEINFOANSWER;

		if (m_pRequestSocket != NULL)
		{
			g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
			m_pRequestSocket->SendPacket(pPacket, true, true);
		}
		else
		{
			delete pPacket;
		}
	}
#endif //OLD_SOCKETS_ENABLED
	EMULE_CATCH
}

void CUpDownClient::ProcessMuleInfoPacket(BYTE* pbytePacket, uint32 dwSize)
{
	EMULE_TRY

	CSafeMemFile packetStream(pbytePacket, dwSize);
	m_byteCompatibleClient = 0;

	byte	byteEmuleVer, byteEmuleProt;

	packetStream.Read(&byteEmuleVer, 1);
	if (byteEmuleVer == 0x2B)
		byteEmuleVer = 0x22;

	packetStream.Read(&byteEmuleProt, 1);

//	Implicitly supported options by older clients
	if (byteEmuleProt != EMULE_PROTOCOL)
		return;
	if (m_byteEmuleVersion != 0x99)	//CT_EMULE_VERSION wasn't received
		m_byteEmuleVersion = byteEmuleVer;
	else
	{
	//	Make version again in the CT_EMULE_VERSION style
		m_dwClientVersion = (GET_CLIENT_MAJVER(m_dwClientVersion) << 17) |
			(GET_CLIENT_MINVER(m_dwClientVersion) << 10) | (GET_CLIENT_UDPVER(m_dwClientVersion) << 7);
	}

//	In the future do not use version to guess about new features
	if (m_byteEmuleVersion < 0x25 && m_byteEmuleVersion > 0x22)
		m_byteUDPVer = 1;

	if (m_byteEmuleVersion < 0x25 && m_byteEmuleVersion > 0x21)
		m_byteSourceExchange1Ver = 1;

	if (m_byteEmuleVersion == 0x24)
		m_byteAcceptCommentVer = 1;

//	Shared directories are requested since eMule 0.28+ because eMule 0.27 has a bug in
//	the OP_ASKSHAREDFILESDIR handler, which does not return the shared files for a directory
//	which has a trailing backslash. MLdonkey currently does not support shared directories
//	eMule+ supports shared directories since 1h (0x0108) -- eMule+ is checked later after detection
	if ((m_byteEmuleVersion >= 0x28) && !m_bIsML)
		m_fSupportsAskSharedDirs = 1;

	m_bEmuleProtocol = true;
	m_dwL2HACTime = 0;

	uint32 i, dwTagCount;
	packetStream.Read(&dwTagCount, 4);

	try
	{
		for (i = 0; i < dwTagCount; i++)
		{
			CTag	TempTag;

			TempTag.FillFromStream(packetStream, cfLocalCodePage);
			switch (TempTag.GetTagID())
			{
				case ET_COMPRESSION:
					if (TempTag.IsInt())
						m_byteDataCompVer = static_cast<byte>(TempTag.GetIntValue());
					break;

				case ET_UDPPORT:
					if (TempTag.IsInt())
						m_uUDPPort = static_cast<uint16>(TempTag.GetIntValue());
					break;

				case ET_UDPVER:
					if (TempTag.IsInt())
						m_byteUDPVer = static_cast<byte>(TempTag.GetIntValue());
					break;

				case ET_SOURCEEXCHANGE:
					if (TempTag.IsInt())
						m_byteSourceExchange1Ver = static_cast<byte>(TempTag.GetIntValue());
					break;

				case ET_COMMENTS:
					if (TempTag.IsInt())
						m_byteAcceptCommentVer = static_cast<byte>(TempTag.GetIntValue());
					break;

				case ET_EXTENDEDREQUEST:
					if (TempTag.IsInt())
						m_byteExtendedRequestsVer = static_cast<byte>(TempTag.GetIntValue());
					break;

				case ET_COMPATIBLECLIENT:
				//	Bits 31- 8: 0 - reserved
				//	Bits  7- 0: compatible client ID
					if (TempTag.IsInt())
						m_byteCompatibleClient = static_cast<byte>(TempTag.GetIntValue());
					break;

				case ET_MOD_VERSION:
					if (TempTag.IsStr())
					{
						if (!TempTag.IsStringValueEqual(m_strModString))
						{
							TempTag.GetStringValue(&m_strModString);
							m_bIsMODNameChanged = true;
						}
					}
					else if (TempTag.IsInt())
						m_strModString.Format(_T("ModID=%u"), TempTag.GetIntValue());
					else
						m_strModString = _T("ModID=<Unknwon>");
					break;

				case ET_MOD_PLUS:
					if (TempTag.IsInt())
						m_dwPlusVers = TempTag.GetIntValue();
					break;

				case ET_L2HAC:
					if (TempTag.IsInt())
						m_dwL2HACTime = TempTag.GetIntValue();
					break;

				case ET_FEATURES:
				//	Bits 31- 8: 0 - reserved
				//	Bit      7: Preview
				//	Bit   6- 0: secure identification
					if (TempTag.IsInt())
						m_byteSupportSecIdent = static_cast<byte>(TempTag.GetIntValue() & 3);
					break;
			}
		}
	}
	catch (CException *error)
	{
		error->Delete();
#ifdef _DEBUG
		AddLogLine( LOG_FL_DBG, _T("Error while processing emuletag %u/%u from client %s %s:%u"), i + 1,
						 dwTagCount, GetClientNameWithSoftware(), GetFullIP(), GetUserPort() );
#endif
	}

	if (!m_dwL2HACTime)
		m_dwL2HACTime = L2HAC_DEFAULT_EMULE;
	if (m_dwL2HACTime < L2HAC_MIN_TIME || m_dwL2HACTime > L2HAC_MAX_TIME)
		m_dwL2HACTime = 0;

	if (m_byteDataCompVer == 0)
	{
		m_byteSourceExchange1Ver = 0;
		m_byteExtendedRequestsVer = 0;
		m_byteAcceptCommentVer = 0;
		m_uUDPPort = 0;
		m_dwL2HACTime = 0;
	}

//	Validate eMule Plus version

//	Check minimal existing version and the last version sending ET_MOD_PLUS
	if ((m_dwPlusVers > 0x111) || (m_dwPlusVers < 0x100))
		m_dwPlusVers = 0;
	if (m_dwPlusVers != 0)
	{
		if ( ((m_dwPlusVers != CURRENT_PLUS_VERSION) && (m_strModString.Compare(_T(PLUS_VERSION_STR)) == 0)) ||
			((m_dwPlusVers <= 0x103) && (m_byteEmuleVersion > 0x26)) ||
			((m_dwPlusVers > 0x103) && m_strModString.IsEmpty()) ||	//several old versions didn't have ModString
			(m_strModString.GetLength() > 10) ||
			( (m_strModString.GetLength() >= 5) &&
			(memcmp(m_strModString.GetString(), _T("Plus "), sizeof(_T("Plus ")) - sizeof(TCHAR)) != 0) &&
			(memcmp(m_strModString.GetString(), _T("koizo"), sizeof(_T("koizo")) - sizeof(TCHAR)) != 0) ) )
		{
			m_dwPlusVers = 0;
		}
	}

//	eMule Plus supports shared directories since 1h (0x0108)
	if ((m_dwPlusVers != 0) && (m_dwPlusVers < 0x0108))
		m_fSupportsAskSharedDirs = 0;

	ReGetClientSoft();

//	Reset version if client type is incorrect (here SO_UNKNOWN means that handshaking hasn't finished yet)
	if ((m_dwPlusVers != 0) && (m_eClientSoft != SO_PLUS) && (m_eClientSoft != SO_UNKNOWN))
		m_dwPlusVers = 0;

	m_eInfoPacketsReceived |= IP_EMULEPROTPACK;

//	eMule Plus supports 'complete sources' since version 1f (force ET_EXTENDEDREQUEST v2 for backward compatibility)
	if ((GetPlusVersion() < 0x0108) && (GetPlusVersion() >= 0x0106) && (GetExtendedRequestsVersion() == 1))
		m_byteExtendedRequestsVer = 2;

//	Check for leeching clients
	if (HasMODNameForbiddenStrings())
		Ban(BAN_CLIENT_KWOWN_LEECHER);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendHelloAnswer()
{
	EMULE_TRY
#ifdef OLD_SOCKETS_ENABLED
	if (m_pRequestSocket != NULL)
	{
		CMemFile packetStream(128);
		SendHelloTypePacket(packetStream);
		Packet		*pPacket = new Packet(&packetStream);
		pPacket->m_eOpcode = OP_HELLOANSWER;
		g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
		m_pRequestSocket->SendPacket(pPacket, true);
		SetHandshakeStatus(true);
	}
#endif //OLD_SOCKETS_ENABLED
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendHelloTypePacket(CMemFile &packetStream)
{
	EMULE_TRY

	packetStream.Write(g_App.m_pPrefs->GetUserHash(), 16);					// <userhash 16>

#ifdef OLD_SOCKETS_ENABLED
	uint32		dwClientID = g_App.m_pServerConnect->GetClientID();

	packetStream.Write(&dwClientID, 4);				// <ourclientid 4>
#endif //OLD_SOCKETS_ENABLED

	uint16		uPort = g_App.m_pPrefs->GetListenPort();

	packetStream.Write(&uPort, 2);					// <ourport 2>

	CWrTag		tagWr;
	uint32		dwTagCount = 6;

	packetStream.Write(&dwTagCount, 4);				// <tagcount 4>

	g_App.m_pPrefs->WritePreparedNameTag(packetStream);		// { NAME : int }
	tagWr.WriteToFile(CT_VERSION, EDONKEYVERSION, packetStream);		// { VERSION : int }
	tagWr.WriteToFile(CT_EMULE_UDPPORTS, g_App.m_pPrefs->GetUDPPort(), packetStream);

//	eMule Misc. Options #1
	const uint32	dwSupportSecIdent = (g_App.m_pClientCreditList->CryptoAvailable() ? 3 : 0);
	const uint32	dwNoViewSharedFiles = ((g_App.m_pPrefs->CanSeeShares() == SEE_SHARE_NOONE) ? 1 : 0); // for backward compatibility this has to be a 'negative' flag
	tagWr.WriteToFile( CT_EMULE_MISCOPTIONS1,
#ifdef _UNICODE
		(1						<< 4*7) |	//Unicode support
#endif
		(4						<< 4*6) |	//UDP version
		(1						<< 4*5) |	//Data compression version
		(dwSupportSecIdent		<< 4*4) |	//Secure Ident
		(SOURCEEXCHANGE1_VERSION << 4*3) |	//Source Exchange
		(2						<< 4*2) |	//Ext. Requests
		(1						<< 4*1) |	//Comments
		(dwNoViewSharedFiles	<< 1*2), packetStream );	//No 'View Shared Files' supported

//	eMule Misc. Options #2
	const int	iCryptSupports = (g_App.m_pPrefs->IsClientCryptLayerSupported()) ? 1 : 0;
	const int	iCryptRequests = (g_App.m_pPrefs->IsClientCryptLayerRequested()) ? 1 : 0;
	const int	iCryptRequires = (g_App.m_pPrefs->IsClientCryptLayerRequired()) ? 1 : 0;
	tagWr.WriteToFile( CT_EMULE_MISCOPTIONS2,
		(1						<< 10) |	//Source Exchange 2
#ifdef _CRYPT_READY
		(iCryptRequires			<<  9) |
		(iCryptRequests			<<  8) |
		(iCryptSupports			<<  7) |
#endif
		(1						<<  4), packetStream );	//Large File support

#if ((CURRENT_PLUS_VERSION & 0xF) > 7)
#error Incompatible eMule Plus version build number is used
#endif
	tagWr.WriteToFile( CT_EMULE_VERSION, (PLUS_COMPATIBLECLIENTID << 24) | ((CURRENT_PLUS_VERSION & 0x7F00) << 9) |
		((CURRENT_PLUS_VERSION & 0xF0) << 6) | ((CURRENT_PLUS_VERSION & 0x7) << 7), packetStream );

	uint32		dwIP = 0;

	uPort = 0;
#ifdef OLD_SOCKETS_ENABLED

	if (g_App.m_pServerConnect->IsConnected())
	{
		CServer	*pCurServer = g_App.m_pServerConnect->GetCurrentServer();

		dwIP = pCurServer->GetIP();
		uPort = pCurServer->GetPort();
	}
#endif //OLD_SOCKETS_ENABLED
	packetStream.Write(&dwIP, 4);					// <serverip 4>
	packetStream.Write(&uPort, 2);					// <serverport 2>
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ProcessMuleCommentPacket(byte *pbytePacket, uint32 dwSize)
{
	EMULE_TRY

	if ((m_pReqPartFile != NULL) && ((dwSize > (sizeof(m_eRating) + sizeof(int) - 1))))
	{
		CSafeMemFile packetStream(pbytePacket, dwSize);
		uint32	dwContainer, dwLen;
		bool	bPrevCommentEmpty = m_strComment.IsEmpty();
		EnumPartFileRating	ePrevRating = m_eRating;

		packetStream.Read(&m_eRating, sizeof(m_eRating));
		packetStream.Read(&dwLen, sizeof(dwLen));

		dwContainer = static_cast<uint32>(packetStream.GetLength() - packetStream.GetPosition());
		if (dwLen > dwContainer)
			dwLen = dwContainer;
	//	Increase the raw max. allowed file comment length because of possible UTF8 encoding
	//	which according to the standard can use up to 4 bytes to encode one character
		if (dwLen > 4 * MAXFILECOMMENTLEN)
			dwLen = 4 * MAXFILECOMMENTLEN;

		if (dwLen != 0)
		{
			ReadMB2Str(m_eStrCodingFormat, &m_strComment, packetStream, dwLen);

			if (m_strComment.GetLength() > MAXFILECOMMENTLEN) // enforce the max len on the comment
				m_strComment = m_strComment.Left(MAXFILECOMMENTLEN);

		//	Comment filter
			CString strList(g_App.m_pPrefs->GetCommentFilter().MakeLower());
			if (!strList.IsEmpty())
			{
			//	Make a copy of casesensitive comment
				CString	strResToken, lowComment(m_strComment);
				int		iCurPos = 0;

				lowComment.MakeLower();	//	Lowercase copy to search in
				for (;;)
				{
					strResToken = strList.Tokenize(_T("|"), iCurPos);
					if (strResToken.IsEmpty())
						break;
					if (lowComment.Find(strResToken) >= 0)
					{
						AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Description by client '%s' for file '%s' filtered: %s"),
										 m_strUserName, m_pReqPartFile->GetFileName(), m_strComment );
						m_strComment.Empty();
						m_eRating = PF_RATING_NONE;
					//	Clean previous rating, so spam doesn't influence overall score
					//	This is required as a user can update comment filter
						if ((ePrevRating != PF_RATING_NONE) || !bPrevCommentEmpty)
							m_pReqPartFile->UpdateFileRatingCommentAvail();
						break;
					}
				}
			}
			if (!m_strComment.IsEmpty())
			{
				m_pReqPartFile->SetHasComment(true);
				AddLogLine( LOG_FL_DBG, _T("Description by client '%s' for file '%s' received: %s"),
								 m_strUserName, m_pReqPartFile->GetFileName(), m_strComment );
			}
		}
		if (m_eRating != PF_RATING_NONE)
		{
			m_pReqPartFile->SetHasRating(true);
			AddLogLine( LOG_FL_DBG, _T("Rating by client '%s' for file '%s' received: %s"),
							 m_strUserName, m_pReqPartFile->GetFileName(), ::GetRatingString(m_eRating) );
		}
		if ((!m_strComment.IsEmpty()) || (m_eRating != PF_RATING_NONE))
			m_pReqPartFile->RemovePastComment(this, false);
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::Disconnected(bool bRetryConnection /*=true*/)
{
	bool	bDeleted = false;

	EMULE_TRY

	if (g_App.m_pClientList->IsValidClient(this))
	{
	//	If the client is uploading, remove it from the upload queue
		if (GetUploadState() == US_UPLOADING)
			g_App.m_pUploadQueue->RemoveFromUploadQueue(this, ETS_DISCONNECT);

	//	Clear upload block lists, otherwise unrequested stuck blocks can be uploaded during next upload to the same client.
	//	That could happen when next time a remote client requests different blocks. That could be if a remote client:
	//	1) received some data from us and connection was lost.
	//	2) lost connection with us, but received these blocks from another client.
		ClearUploadBlockRequests();

	//	If the client is downloading, leave it on the queue but stop it from downloading
		if (GetDownloadState() == DS_DOWNLOADING)
			UpdateOnqueueDownloadState();
	// The remote client does not have to answer with OP_HASHSETANSWER *immediatly*
	// after we've sent OP_HASHSETREQUEST. It may occur that a (buggy) remote client
	// is sending us another OP_FILESTATUS which would let us change to DL-state DS_ONQUEUE.
		if (((GetDownloadState() == DS_REQHASHSET) || m_fRequestingHashSet) && m_pReqPartFile != NULL)
			m_pReqPartFile->m_bHashSetNeeded = true;
		ASSERT(g_App.m_pClientList->IsValidClient(this));

	//	Check if this client is needed in any way, if not delete it
		bool bDelete = true;

		switch (m_eUploadState)
		{
			case US_ONUPLOADQUEUE:
				bDelete = false;
		}
		switch (m_eDownloadState)
		{
			case DS_ONQUEUE:
			case DS_NONEEDEDPARTS:
			case DS_LOWTOLOWID:
			case DS_LOWID_ON_OTHER_SERVER:
				bDelete = false;
		}

		switch (m_eUploadState)
		{
			case US_CONNECTING:
				bDelete = true;
		}
		switch (m_eDownloadState)
		{
			case DS_CONNECTING:
				m_byteFailedCount++;
				if (bRetryConnection && m_byteFailedCount <= 2)
				{
					TryToConnect();
					return bDeleted;
				}

			case DS_WAITCALLBACK:
			case DS_ERROR:
				bDelete = true;
				g_App.m_pDownloadQueue->RemoveSource(this);
		}

		if (GetChatState() != MS_NONE)
		{
			bDelete = false;
			g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.ConnectingResult(this, false);
		}
#ifdef OLD_SOCKETS_ENABLED
		g_App.m_pListenSocket->DeleteSocket(m_pRequestSocket);
#endif //OLD_SOCKETS_ENABLED

		if (m_iFileListRequested)
		{
			AddLogLine(LOG_FL_SBAR, IDS_SHAREDFILES_FAILED, m_strUserName);
			m_iFileListRequested = m_iFileListRequestedSave = 0;
		}
		if (m_pFriend)
		{
			g_App.m_pFriendList->RefreshFriend(m_pFriend);
		}

		if (bDelete)
		{
			delete this;
			bDeleted = true;
		}
		else
		{
			m_fRequestingHashSet = 0;
			m_fSentCancelTransfer = 0;
			g_App.m_pClientList->UpdateClient(this);
		}
	}

	EMULE_CATCH

	return bDeleted;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::TryToConnect(bool bIgnoreMaxCon)
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
//	If we already have a connected request socket, we're done
//	note: we don't need to check if IP is filtered because in case of active connection it was already done
	if ((m_pRequestSocket != NULL) 
		&& (m_pRequestSocket->IsConnected() || m_pRequestSocket->IsConnecting()))
	{
		if (IsHandshakeFinished())
			ConnectionEstablished();
		return true;
	}

//	Check if it's allowed to create new socket
	if ((g_App.m_pListenSocket->TooManySockets() && !bIgnoreMaxCon))
	{
		Disconnected();
		return false;
	}
#endif //OLD_SOCKETS_ENABLED


//	Although we filter all received IPs (server sources, source exchange) and all incoming connection attempts,
//	we do have to filter outgoing connection attempts here too, because we may have updated the ip filter list
//	or client was added(removed) to(from) dynamic IP filtering list
	if ((m_dwConnectIP > 0) && g_App.m_pIPFilter->IsFiltered(m_dwConnectIP))
	{
		if (!g_App.m_pPrefs->IsCMNotLog())
		{
			AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Filtered client (don't connect): %s %hs"),
							 GetFullIP(), g_App.m_pIPFilter->GetLastHit() );
		}
	//	Disconnect and don't retry.
		Disconnected(false);
		InterlockedIncrement(&g_App.m_lOutgoingFiltered);
		InterlockedIncrement(&g_App.m_lTotalFiltered);
		return false;
	}

//	Check the cases when it's not possible to establish a connection to remote LowID client
//	1. our client hasn't connected to the server
//	2. both clients have LowIDs
//	3. the remote client is on another server
//	4. LowID client in US_CONNECTING state what according to Merkur's design should never happen
	if (HasLowID())
	{
#ifdef OLD_SOCKETS_ENABLED
		if (!g_App.m_pServerConnect->IsConnected()
			|| g_App.m_pServerConnect->IsLowID())
		{
			if (GetDownloadState() == DS_CONNECTING)
				SetDownloadState(DS_LOWTOLOWID);
			if (GetUploadState() == US_CONNECTING)
				SetUploadState(US_NONE);

			Disconnected(false);
			return false;
		}
		else if (g_App.m_pServerConnect->GetCurrentServer()->GetIP() != GetServerIP())
		{
			if (GetDownloadState() == DS_CONNECTING)
			{
			//	We come here after A4AF swap, as there's no information about
			//	remote file status, set general state
				SetDownloadState(DS_LOWID_ON_OTHER_SERVER);
			}
			if (GetUploadState() == US_CONNECTING)
				SetUploadState(US_NONE);

			Disconnected(false);
			return false;
		}
		else
		{
		//	MOD Note: Do not change this part - Merkur
			if (GetDownloadState() == DS_CONNECTING)
				SetDownloadState(DS_WAITCALLBACK);

			if (GetUploadState() == US_CONNECTING)
			{
				SetUploadState(US_NONE);
				Disconnected(false);
				return false;
			}
		//	MOD Note - end

			Packet		*pPacket = new Packet(OP_CALLBACKREQUEST, 4);

			POKE_DWORD(pPacket->m_pcBuffer, m_dwUserIDHybrid);
			g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
			g_App.m_pServerConnect->SendPacket(pPacket);

			m_bReaskPending = true;
		}
#endif //OLD_SOCKETS_ENABLED
	}
	else
	{
#ifdef OLD_SOCKETS_ENABLED
		if (m_pRequestSocket == NULL)
		{
			m_pRequestSocket = new CClientReqSocket(this);
			if (!m_pRequestSocket->Create())
			{
				m_pRequestSocket->Safe_Delete();
				return true;
			}
		}
		else if (!m_pRequestSocket->IsConnected())
		{
			m_pRequestSocket->Safe_Delete();
			m_pRequestSocket = new CClientReqSocket(this);
			if (!m_pRequestSocket->Create())
			{
				m_pRequestSocket->Safe_Delete();
				return true;
			}
		}

		SOCKADDR_IN	sockAddr = {0};

		sockAddr.sin_family = AF_INET;
		sockAddr.sin_port = fast_htons(GetUserPort());
		sockAddr.sin_addr.s_addr = m_dwConnectIP;
		m_pRequestSocket->Connect(reinterpret_cast<SOCKADDR*>(&sockAddr), sizeof(sockAddr));
#endif //OLD_SOCKETS_ENABLED

		SendHelloPacket();
	}
	return true;

	EMULE_CATCH
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ConnectionEstablished()
{
	EMULE_TRY

	m_byteFailedCount = 0;

//	Ok we have a connection, let's see if we want anything from this client.

//	If we're connecting for chat...
	if (GetChatState() == MS_CONNECTING)
	{
		g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.ConnectingResult(this, true);
	}
//	If we're connecting for download...
	switch (GetDownloadState())
	{
		case DS_CONNECTING:
		case DS_WAITCALLBACK:
			m_bReaskPending = false;
			SetDownloadState(DS_CONNECTED);
			SendFileRequest();
			break;

		case DS_ONQUEUE:
		case DS_NONEEDEDPARTS:
			if (!HasLowID())
				break;
		//	Check LowID properties of local server connection if it exists, otherwise consider 
		//	current client state as DS_LOWTOLOW
			if (g_App.m_pServerConnect->IsConnected())
			{
			//	If remote client is still on the same server then check if it's allowed to reask,
			//	otherwise remote client falls through & reasks or sets proper download state
			//	note: Don't reask faster than old client version when LowID client is on the same server
				if ((g_App.m_pServerConnect->GetCurrentServer()->GetIP() == GetServerIP())
					&& ((::GetTickCount() - m_dwLastAskedTime) < OLD_FILEREASKTIME))
				{
					break;
				}
			}
	//	If remote client has LowID & connected to another server, then send it a file request
	//	every time, when it establishes a new connection & minimal request time is gone
		case DS_LOWTOLOWID:
		case DS_LOWID_ON_OTHER_SERVER:
			if ((m_pReqPartFile != NULL) && (m_pReqPartFile->GetStatus() != PS_PAUSED)
				&& ((::GetTickCount() - m_dwLastAskedTime) > MIN_REQUESTTIME))
			{
				m_bReaskPending = false;
			//	Update the time to prevent an additional request from PartFile::Process()
				SetLastAskedTime();
				SetDownloadState(DS_CONNECTED);
				SendFileRequest();
			}
		//	Check if our client is connected to server & we can put client into proper LowID state 
			else if (g_App.m_pServerConnect->IsConnected())
			{
			//	Put client in proper state if server was changed
				if (m_fServerWasChanged == 1)
				{
					if (g_App.m_pServerConnect->IsLowID())
						SetDownloadState(DS_LOWTOLOWID);
					else if (m_uNeededParts == 0)
						SetDownloadState(DS_NONEEDEDPARTS);
					else if (g_App.m_pServerConnect->GetCurrentServer()->GetIP() != GetServerIP())
						SetDownloadState(DS_LOWID_ON_OTHER_SERVER);
					else
						SetDownloadState(DS_ONQUEUE);
				}
			}
		//	Temporarily put the client into active state 
		//	note: the proper state will be set in CUpDownClient::TryToConnect() 
			else
			{
				if (m_uNeededParts != 0)
					SetDownloadState(DS_ONQUEUE);
				else
					SetDownloadState(DS_NONEEDEDPARTS);
			}
			break;
	}
//	If we're connecting for a "reask"...
	if (m_bReaskPending)
	{
		m_bReaskPending = false;
		if (GetDownloadState() != DS_NONE && GetDownloadState() != DS_DOWNLOADING)
		{
		//	Update the time to prevent an additional request from PartFile::Process()
			SetLastAskedTime();
			SetDownloadState(DS_CONNECTED);
			SendFileRequest();
		}
	}
//	If we're connecting for upload...
	switch (GetUploadState())
	{
		case US_CONNECTING:
		{
			if (g_App.m_pUploadQueue->IsDownloading(this))
			{
				Packet		*pPacket = new Packet(OP_ACCEPTUPLOADREQ, 0);

				g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);

#ifdef OLD_SOCKETS_ENABLED
				m_pRequestSocket->SendPacket(pPacket, true);
#endif OLD_SOCKETS_ENABLED

				SetUploadState(US_UPLOADING);
			//	Set Upload Timer
				SetLastGotULData();
			}
		}
	}
//	If we're connecting to retrieve the client's file list...
	if (m_iFileListRequested == 1)
	{
		Packet		*pPacket = new Packet((m_fSupportsAskSharedDirs != 0) ? OP_ASKSHAREDDIRS : OP_ASKSHAREDFILES, 0);

		g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);

#ifdef OLD_SOCKETS_ENABLED
		m_pRequestSocket->SendPacket(pPacket, true, true);
#endif OLD_SOCKETS_ENABLED
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CUpDownClient::GetHashType()
{
	if (m_userHash[5] == 14 && m_userHash[14] == 111)
		return SO_EMULE;
	else if (m_userHash[5] == 13 && m_userHash[14] == 110)
		return SO_OLDEMULE;
	else if (m_userHash[5] == 'M' && m_userHash[14] == 'L')
		return SO_MLDONKEY;
	else
		return SO_UNKNOWN;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ReGetClientSoft()
{
	EMULE_TRY

//	If this client hasn't been contacted, it is SO_UNKNOWN
	if (m_strUserName.IsEmpty())
	{
		m_eClientSoft = SO_UNKNOWN;
		return;
	}
//	In some cases at the time of OP_EMULEINO (which is received before OP_HELLOANSWER)
//	user hash can be zero. The processing will be done on OP_HELLOANSWER reception
	if (!HasValidHash())
		return;

	uint32	dwMajVer, dwMinVer, dwUpdVer;
	int		iHashType = GetHashType();

	switch (iHashType)
	{
	//	If a client is an old emule...
		case SO_OLDEMULE:
		{
			if ((m_byteEmuleVersion != 0x99) && (m_byteCompatibleClient == 0) && !m_bIsML)
			{
				m_eClientSoft = SO_OLDEMULE;
				m_strClientSoft.Format(_T("%s v0.%u"), GetResString(IDS_OLDEMULE), m_dwClientVersion);
				m_dwClientVersion = FORM_CLIENT_VER(0, m_dwClientVersion, 0);
				break;
			}
		}
	//	If a client is eMule compatible client...
		case SO_EMULE:
		{
			m_eClientSoft = GetClientTypeFromCompatibilityTag(m_byteCompatibleClient);
			if ((m_eClientSoft == SO_EMULE) || (m_eClientSoft == SO_UNKNOWN))
			{
				if (m_bIsML)
				{
					m_eClientSoft = SO_MLDONKEY;
					if (m_byteEmuleVersion == 0)	//neither CT_EMULE_VERSION nor OP_EMULEINFO was received
					{
						m_strClientSoft.Format(_T("MLdonkey v0.%u"), m_dwClientVersion);
						m_dwClientVersion = FORM_CLIENT_VER(0, m_dwClientVersion, 0);
						break;
					}
				}
				else if (m_bIsHybrid)
				{
					m_eClientSoft = SO_EDONKEYHYBRID;
				}
				else if (m_eClientSoft == SO_UNKNOWN)
				{
					m_eClientSoft = SO_EMULE;
					m_strModString.Format(_T("CompatID %u"), m_byteCompatibleClient);
				}
			}
			if ((m_dwPlusVers != 0) && (m_eClientSoft != SO_EMULE))
				m_dwPlusVers = 0;

			CString	strClientVer = _T(" ?");

			if (m_dwPlusVers != 0)
			{
			//	Adjust old client identification to the new style
				dwMajVer = m_dwPlusVers >> 8;
				dwMinVer = (m_dwPlusVers >> 4) & 0xF;
				dwUpdVer = m_dwPlusVers & 0xF;

				if ( !m_strModString.IsEmpty() &&
					(memcmp(m_strModString.GetString(), _T("Plus"), sizeof(_T("Plus")) - sizeof(TCHAR)) == 0) )
				{
					m_strModString.Empty();
				}
				m_eClientSoft = SO_PLUS;
				goto AdjustOldIdentification;
			}
			if (m_byteEmuleVersion == 0)
			{
				m_dwClientVersion = FORM_CLIENT_VER(0, 0, 0);
			}
			else if (m_byteEmuleVersion != 0x99)
			{
				dwMinVer = (m_byteEmuleVersion >> 4) * 10 + (m_byteEmuleVersion & 0xF);
				m_dwClientVersion = FORM_CLIENT_VER(0, dwMinVer, 0);
				strClientVer.Format(_T("0.%u"), dwMinVer);
			}
			else
			{
				dwMajVer = (m_dwClientVersion >> 17) & 0x7F;
				dwMinVer = (m_dwClientVersion >> 10) & 0x7F;
				dwUpdVer = (m_dwClientVersion >>  7) & 0x07;

				if ((m_eClientSoft == SO_LPHANT) && (static_cast<int>(--dwMajVer) < 0))
					dwMajVer++;

AdjustOldIdentification:
				m_dwClientVersion = FORM_CLIENT_VER(dwMajVer, dwMinVer, dwUpdVer);

				if (m_eClientSoft == SO_PLUS)
				{
					if (m_fPeerCache == 1)
					{
						m_fIdenThief = 1;
						m_eClientSoft = SO_EMULE;
						m_bIsCommunity = false;
						m_dwClientVersion = FORM_CLIENT_VER(43, 0, 0);	//set something reasonable
						m_strModString.Empty();
						m_strUserName.Format(_T("[IdentityThief %08X]"), this);
						m_strClientSoft.Format(_T("%s Mod v ?"), GetClientNameString(m_eClientSoft));
						break;
					}
					else
					{
						if (m_dwPlusVers == 0)
							m_dwPlusVers = (dwMajVer << 8) | ((dwMinVer & 0xF) << 4) | dwUpdVer;
						strClientVer.Format(_T("%u"), dwMajVer);
						if (dwMinVer != 0)
							strClientVer.AppendFormat(_T(".%u"), dwMinVer);
						if (dwUpdVer != 0)
							strClientVer += static_cast<TCHAR>(_T('a') + dwUpdVer - 1);
					}
				}
				else if ((m_eClientSoft == SO_EMULE) && (dwUpdVer < 26))
					strClientVer.Format(_T("%u.%u%c"), dwMajVer, dwMinVer, _T('a') + dwUpdVer);
				else if ((dwUpdVer != 0) || (m_eClientSoft == SO_AMULE))
					strClientVer.Format(_T("%u.%u.%u"), dwMajVer, dwMinVer, dwUpdVer);
				else
					strClientVer.Format(_T("%u.%u"), dwMajVer, dwMinVer);
			}
			if (m_strModString.IsEmpty())
				m_strClientSoft.Format(_T("%s v%s"), GetClientNameString(m_eClientSoft), strClientVer);
			else
				m_strClientSoft.Format(_T("%s v%s [%s]"), GetClientNameString(m_eClientSoft), strClientVer, m_strModString);
			break;
		}
		default:
		//	If a client is eDonkey-Hybrid...
			if (m_bIsHybrid)
			{
				m_eClientSoft = SO_EDONKEYHYBRID;

			//	I've never seen such idiotic version format in my life. Exampes:
			//	105321 - eDonkey 0.53.21
			//	1053   - eDonkey 0.53
			//	10502  - eDonkey 0.50.2
			//	1044   - eDonkey 0.44
			//	1432   - eDonkey 0.43.2 (first hybrid version)
			//	53248  - Overnet 0.53.24.8
			//	531    - Overnet 0.53.1
			//	53     - Overnet 0.53
			//	1000   - combined eDonkey-Overnet 1.0
			//	1001   - combined eDonkey-Overnet 1.0.1
			//	10200  - combined eDonkey-Overnet 1.2.0

			//	Convert eDonkey format into Overnet
				if (((m_dwClientVersion >= 1044) && (m_dwClientVersion < 1054)) || (m_dwClientVersion == 1432))
					m_dwClientVersion -= 1000;
				else if ((m_dwClientVersion > 10440) && (m_dwClientVersion < 10540))
					m_dwClientVersion -= 10000;
				else if ((m_dwClientVersion > 104400) && (m_dwClientVersion < 105400))
					m_dwClientVersion -= 100000;

				if (m_dwClientVersion < 1000)
				{
					dwMajVer = 0;
					if (m_dwClientVersion < 100)
					{
						dwMinVer = m_dwClientVersion;
						dwUpdVer = 0;
					}
					else
					{
						dwMinVer = m_dwClientVersion / 10;
						dwUpdVer = m_dwClientVersion - dwMinVer * 10;
					}
				}
				else if ((m_dwClientVersion > 4400) && (m_dwClientVersion < 5400))
				{
					dwMajVer = 0;
					dwMinVer = m_dwClientVersion / 100;
					dwUpdVer = m_dwClientVersion - dwMinVer * 100;
				}
				else if ((m_dwClientVersion > 44000) && (m_dwClientVersion < 54000))
				{
					dwMajVer = 0;
					dwMinVer = m_dwClientVersion / 1000;
					dwUpdVer = m_dwClientVersion / 10 - dwMinVer * 100;	//skip minor build number
				}
				else
				{
					if (m_dwClientVersion >= 10000)	// that's just an assumption
					{
						dwMajVer = m_dwClientVersion / 10000;
						m_dwClientVersion -= dwMajVer * 10000;
					}
					else
					{
						dwMajVer = m_dwClientVersion / 1000;
						m_dwClientVersion -= dwMajVer * 1000;
					}
					dwMinVer = m_dwClientVersion / 100;
					dwUpdVer = m_dwClientVersion - dwMinVer * 100;
				}
				m_dwClientVersion = FORM_CLIENT_VER(dwMajVer, dwMinVer, dwUpdVer);

				if (dwUpdVer)
					m_strClientSoft.Format(_T("Hybrid v%u.%u.%u"), dwMajVer, dwMinVer, dwUpdVer);
				else
					m_strClientSoft.Format(_T("Hybrid v%u.%u"), dwMajVer, dwMinVer);
			}
		//	If a client is MLdonkey...
			else if (m_bIsML || (iHashType == SO_MLDONKEY))
			{
				m_eClientSoft = SO_MLDONKEY;
				m_strClientSoft.Format(_T("MLdonkey v0.%u"), m_dwClientVersion);
				m_dwClientVersion = FORM_CLIENT_VER(0, m_dwClientVersion, 0);
			}
			else
			{
				m_eClientSoft = SO_EDONKEY;
				m_strClientSoft.Format(_T("eDonkey v0.%u"), m_dwClientVersion);
				m_dwClientVersion = FORM_CLIENT_VER(0, m_dwClientVersion, 0);
			}
			break;
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::RequestSharedFileList()
{
	EMULE_TRY

	if (m_iFileListRequested == 0)
	{
		AddLogLine(LOG_FL_SBAR, IDS_SHAREDFILES_REQUEST, m_strUserName);
		m_iFileListRequested = m_iFileListRequestedSave = 1;
		TryToConnect(true);
	}
	else
	{
		AddLogLine(LOG_FL_SBAR, IDS_SHAREDFILES_INPROGRESS, m_strUserName, GetUserIDHybrid());
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ProcessSharedFileList(byte *pbytePacket, uint32 dwSize, LPCTSTR strDirectory)
{
	EMULE_TRY

	if (m_iFileListRequested > 0)
	{
		bool		bFirstDir = (m_iFileListRequested == m_iFileListRequestedSave);

		m_iFileListRequested--;
		g_App.m_pSearchList->ProcessSharedFileListAnswer(pbytePacket, dwSize, this, strDirectory, bFirstDir);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CUpDownClient::GetUploadFileInfo()
{
	if (this == NULL)
		return _T("");

	CString strInfo;
	CKnownFile *pReqFile = g_App.m_pSharedFilesList->GetFileByID(this->m_reqFileHash);

//	Build info text and display it
	strInfo.Format(GetResString(IDS_USERINFO), m_strUserName, GetUserIDHybrid());
	if (pReqFile)
	{
		strInfo.AppendFormat( GetResString(IDS_SF_REQUESTED) + _T(' ') + pReqFile->GetFileName() + _T('\n') + GetResString(IDS_FILESTATS_SESSION) + 
							  GetResString(IDS_FILESTATS_TOTAL), pReqFile->statistic.GetAccepts(), pReqFile->statistic.GetRequests(),
							  CastItoXBytes(pReqFile->statistic.GetTransferred()), pReqFile->statistic.GetAllTimeAccepts(),
							  pReqFile->statistic.GetAllTimeRequests(), CastItoXBytes(pReqFile->statistic.GetAllTimeTransferred()) );
	}
	else
		strInfo += GetResString(IDS_REQ_UNKNOWNFILE);

	return strInfo;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
HICON CUpDownClient::GetClientInfo4Tooltips(CString &strInfo, bool bForUpload /*false*/)
{
	EMULE_TRY

	if (this == NULL)
		return (HICON)NULL;

	CString		strCountry, strUserName(m_strUserName);

	strUserName.Trim();
	strUserName.Replace(_T("\n"), _T("<br>"));
	strUserName.Replace(_T("<"), _T("<<"));
	if (g_App.m_pIP2Country->IsIP2Country())
		 strCountry.Format(_T(" (<b>%s</b>)"), GetCountryName());

	strInfo.Format(_T("<t=1><b>%s</b><br><t=1>%s: %u%s<br><hr=100%%><br><b>%s:<t></b>%s:%u (<b>%s</b>)"),
		strUserName, GetResString(IDS_USERID), GetUserIDHybrid(), strCountry,
		GetResString(IDS_CLIENT), GetFullIP(), GetUserPort(), m_strClientSoft);

	CServer *pServer = g_App.m_pServerList->GetServerByIP(GetServerIP(), GetServerPort());

	if (pServer != NULL)
	{
		CString			strServerName = pServer->GetListName();

		strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s:%u"), GetResString(IDS_SERVER), ipstr(GetServerIP()), GetServerPort());

		strServerName.Replace(_T("<"), _T("<<"));
		strServerName.Replace(_T("\n"), _T("<br>"));
		if (!strServerName.IsEmpty())
		{
			strInfo += _T(" (<b>");
			strInfo += strServerName;
			strInfo += _T("</b>)");
		}
	}

	if (IsBanned())
		strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"), GetResString(IDS_BANNED), GetBanString());

	if (Credits() != NULL && Credits()->GetUploadedTotal() != GetTransferredUp())
		strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s (%s)"), GetResString(IDS_STATS_DDATA), CastItoXBytes(GetTransferredUp()), CastItoXBytes(Credits()->GetUploadedTotal()));
	else if (GetTransferredUp() > 0)
		strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"), GetResString(IDS_STATS_DDATA), CastItoXBytes(GetTransferredUp()));

	if (Credits() != NULL && Credits()->GetDownloadedTotal() != GetTransferredDown())
		strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s (%s)"), GetResString(IDS_STATS_UDATA), CastItoXBytes(GetTransferredDown()), CastItoXBytes(Credits()->GetDownloadedTotal()));
	else if (GetTransferredDown() > 0)
		strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"), GetResString(IDS_STATS_UDATA), CastItoXBytes(GetTransferredDown()));

	if (bForUpload)
	{
		CKnownFile *m_pReqSharedFile = g_App.m_pSharedFilesList->GetFileByID(this->m_reqFileHash);

		if (m_pReqSharedFile)
		{
			strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s<br><b>%s:</b><t>"), GetResString(IDS_TT_REQUESTED), m_pReqSharedFile->GetFileName(), GetResString(IDS_TT_FILESTATS_SESSION));
			strInfo.AppendFormat(GetResString(IDS_TT_FILESTATS), m_pReqSharedFile->statistic.GetAccepts(), m_pReqSharedFile->statistic.GetRequests(), CastItoXBytes(m_pReqSharedFile->statistic.GetTransferred()));
			strInfo.AppendFormat(_T("<br><b>%s:</b><t>"), GetResString(IDS_TT_FILESTATS_TOTAL));
			strInfo.AppendFormat(GetResString(IDS_TT_FILESTATS), m_pReqSharedFile->statistic.GetAllTimeAccepts(), m_pReqSharedFile->statistic.GetAllTimeRequests(), CastItoXBytes(m_pReqSharedFile->statistic.GetAllTimeTransferred()));
		}
		else
			strInfo.AppendFormat(_T("<br><b>%s:</b><t>%s"), GetResString(IDS_TT_REQUESTED), GetResString(IDS_REQ_UNKNOWNFILE));
	}

	return g_App.m_pMDlg->m_clientImgLists[CLIENT_IMGLST_PLAIN].ExtractIcon(GetClientIconIndex());

	EMULE_CATCH

	return (HICON)NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SendPublicKeyPacket() sends our public key to the client that requested it.
void CUpDownClient::SendPublicKeyPacket()
{
#ifdef DEBUG_SHOW_SECUREID
	DEBUG_ONLY(AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Sending public key to '%s'"), m_strUserName));
#endif

#ifdef OLD_SOCKETS_ENABLED
	if (m_pRequestSocket == NULL || m_pCredits == NULL || m_eSecureIdentState != IS_KEYANDSIGNEEDED)
	{
		ASSERT (false);
		return;
	}
#endif //OLD_SOCKETS_ENABLED
	if (!g_App.m_pClientCreditList->CryptoAvailable())
		return;

	Packet		*pPacket = new Packet(OP_PUBLICKEY, g_App.m_pClientCreditList->GetPubKeyLen() + 1, OP_EMULEPROT);

	g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
	memcpy2(pPacket->m_pcBuffer + 1, g_App.m_pClientCreditList->GetPublicKey(), g_App.m_pClientCreditList->GetPubKeyLen());
	pPacket->m_pcBuffer[0] = g_App.m_pClientCreditList->GetPubKeyLen();
#ifdef OLD_SOCKETS_ENABLED

	m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED

	m_eSecureIdentState = IS_SIGNATURENEEDED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendSignaturePacket()
{
//	Sign the public key of this client and send it
#ifdef OLD_SOCKETS_ENABLED
	if (m_pRequestSocket == NULL || m_pCredits == NULL || m_eSecureIdentState == 0)
	{
		ASSERT (false);
		return;
	}
#endif //OLD_SOCKETS_ENABLED

	if (!g_App.m_pClientCreditList->CryptoAvailable())
		return;

//	We don't have his public key yet, will be back here later
	if (m_pCredits->GetSecIDKeyLen() == 0)
		return;
#ifdef DEBUG_SHOW_SECUREID
	DEBUG_ONLY(AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Sending signature key to '%s'"), m_strUserName));
#endif
//	Do we have a challenge value received (actually we should if we are in this function)
	if (m_pCredits->m_dwCryptRndChallengeFrom == 0)
	{
		AddLogLine(LOG_FL_DBG, _T("Client '%s' want to send signature but challenge value is invalid"), m_strUserName);
		return;
	}
//	v2, we will use v1 as default, except if only v2 is supported
	bool		bUseV2;

	if ((m_byteSupportSecIdent & 1) == 1)
		bUseV2 = false;
	else
		bUseV2 = true;

	byte		byteChallengeIPKind = 0;
	uint32		ChallengeIP = 0;
#ifdef OLD_SOCKETS_ENABLED

	if (bUseV2)
	{
		if (g_App.m_pServerConnect->GetClientID() == 0 || g_App.m_pServerConnect->IsLowID())
		{
		//	We cannot know for sure our public ip, so use the remote clients one
			ChallengeIP = GetIP();
			byteChallengeIPKind = CRYPT_CIP_REMOTECLIENT;
		}
		else
		{
			ChallengeIP = g_App.m_pServerConnect->GetClientID();
			byteChallengeIPKind = CRYPT_CIP_LOCALCLIENT;
		}
	}
#endif //OLD_SOCKETS_ENABLED
	uchar		achBuffer[250];
	byte		siglen = g_App.m_pClientCreditList->CreateSignature(m_pCredits, achBuffer, 250, ChallengeIP, byteChallengeIPKind);

	if (siglen == 0)
	{
		ASSERT (false);
		return;
	}
	Packet		*pPacket = new Packet(OP_SIGNATURE, siglen + 1 + ((bUseV2) ? 1 : 0), OP_EMULEPROT);

	g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
	memcpy2(pPacket->m_pcBuffer + 1, achBuffer, siglen);
	pPacket->m_pcBuffer[0] = siglen;
	if (bUseV2)
		pPacket->m_pcBuffer[1 + siglen] = byteChallengeIPKind;
#ifdef OLD_SOCKETS_ENABLED

	m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED

	m_eSecureIdentState = IS_ALLREQUESTSSEND;
}

void CUpDownClient::ProcessPublicKeyPacket(uchar* pbytePacket, uint32 dwSize)
{
#ifdef DEBUG_SHOW_SECUREID
	DEBUG_ONLY(g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Receiving public key from '%s'"), m_strUserName));
#endif

#ifdef OLD_SOCKETS_ENABLED
	if ( m_pRequestSocket == NULL || m_pCredits == NULL || pbytePacket[0] != dwSize - 1
	     || dwSize == 0 || dwSize > 250 )
	{
		ASSERT (false);
		return;
	}
#endif //OLD_SOCKETS_ENABLED
	if (!g_App.m_pClientCreditList->CryptoAvailable())
		return;
//	the function will handle everything (mulitple key etc)
	if (m_pCredits->SetSecureIdent(pbytePacket + 1, pbytePacket[0]))
	{
	//	if this client wants a signature, now we can send him one
		if (m_eSecureIdentState == IS_SIGNATURENEEDED)
		{
			SendSignaturePacket();
		}
		else if (m_eSecureIdentState == IS_KEYANDSIGNEEDED)
		{
		//	Something is wrong
			AddLogLine( LOG_FL_DBG, _T("Client '%s': Invalid State error, IS_KEYANDSIGNEEDED in ProcessPublicKeyPacket"),
							 m_strUserName );
		}
	}
	else
	{
		AddLogLine(LOG_FL_DBG, _T("Client '%s' failed to use new received public key"), m_strUserName);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ProcessSignaturePacket(uchar *pbytePacket, uint32 dwSize)
{
#ifdef DEBUG_SHOW_SECUREID
	DEBUG_ONLY(AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Receiving signature from '%s'"), m_strUserName));
#endif

//	Here we separate the good guys from the bad ones ;)

#ifdef OLD_SOCKETS_ENABLED
	if (m_pRequestSocket == NULL || m_pCredits == NULL || dwSize == 0 || dwSize > 250)
	{
		ASSERT (false);
		return;
	}
#endif //OLD_SOCKETS_ENABLED

	byte	byteChallengeIPKind, byteSigLen = pbytePacket[0];

//	If the packet consists only of the signature length and signature (SecureIdent V1)...
	if (dwSize == sizeof(byteSigLen) + byteSigLen)
	{
		byteChallengeIPKind = 0;
	}
//	If the packet is long enough to contain the challenge IP kind (SecureIdent V2) and the client supports V2...
	else if ( (dwSize == byteSigLen + sizeof(byteSigLen) + sizeof(byteChallengeIPKind)) &&
		((m_byteSupportSecIdent & 2) > 0))
	{
		byteChallengeIPKind = pbytePacket[dwSize - 1];
	}
	else
	{
		ASSERT (false);
		return;
	}

	if (!g_App.m_pClientCreditList->CryptoAvailable())
		return;

//	If we already have a signature from this client IP...
	if (m_dwLastSignatureIP == GetIP())
	{
	//	We accept only one signature per IP, to avoid floods which need a lot cpu time for crypt functions
		AddLogLine(LOG_FL_DBG, _T("Received multiple signatures from client %s"), GetClientNameWithSoftware());
		return;
	}
//	If we don't have a public key for this client...
	if (m_pCredits->GetSecIDKeyLen() == 0)
	{
		AddLogLine(LOG_FL_DBG, _T("Received signature for client %s without public key"), GetClientNameWithSoftware());
		return;
	}
//	If we haven't generated a challenge value for this client yet...
	if (m_pCredits->m_dwCryptRndChallengeFor == 0)
	{
		AddLogLine(LOG_FL_DBG, _T("Received signature for client %s with invalid challenge value"), GetClientNameWithSoftware());
		return;
	}

	if (g_App.m_pClientCreditList->VerifyIdent(m_pCredits, pbytePacket + 1, byteSigLen, GetIP(), byteChallengeIPKind))
	{
	//	result is saved in function above
	}
#ifdef DEBUG_SHOW_SECUREID
	else
	{
		AddLogLine(LOG_FL_DBG, _T("Client %s has failed the secure identification, V2 State: %i"), GetClientNameWithSoftware(), byteChallengeIPKind);
	}
#endif

	m_dwLastSignatureIP = GetIP();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendSecIdentStatePacket()
{
//	Check if we need public key and signature
	BYTE		byteValue = 0;

	if (m_pCredits != NULL)
	{
		if (g_App.m_pClientCreditList->CryptoAvailable())
		{
			if (m_pCredits->GetSecIDKeyLen() == 0)
			{
				byteValue = IS_KEYANDSIGNEEDED;
			}
			else if (m_dwLastSignatureIP != GetIP())
			{
				byteValue = IS_SIGNATURENEEDED;
			}
		}
	//	If we already have the key and signature, we're done.
		if (byteValue == 0)
		{
#ifdef DEBUG_SHOW_SECUREID
			DEBUG_ONLY(AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Not sending SecIdentState Packet, because State is Zero")));
#endif
			return;
		}
	//	Crypt: send random data to sign
		uint32		dwRandom = rand() + 1;

		m_pCredits->m_dwCryptRndChallengeFor = dwRandom;

#ifdef DEBUG_SHOW_SECUREID
		DEBUG_ONLY(AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Sending SecIdentState packet, state: %u (to '%s')"), byteValue, m_strUserName));
#endif

		Packet		*pPacket = new Packet(OP_SECIDENTSTATE, 5, OP_EMULEPROT);

		g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
		pPacket->m_pcBuffer[0] = byteValue;
		memcpy(pPacket->m_pcBuffer + 1, &dwRandom, sizeof(dwRandom));

#ifdef OLD_SOCKETS_ENABLED
		m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ProcessSecIdentStatePacket(uchar *pbytePacket, uint32 dwSize)
{
	if (dwSize != 5)
		return;
	if (m_pCredits == NULL)
		return;

	switch (pbytePacket[0])
	{
		case 0:
			m_eSecureIdentState = IS_UNAVAILABLE;
			break;
		case 1:
			m_eSecureIdentState = IS_SIGNATURENEEDED;
			break;
		case 2:
			m_eSecureIdentState = IS_KEYANDSIGNEEDED;
			break;
	}
	uint32		dwRandom;

	memcpy2(&dwRandom, pbytePacket + 1, 4);
	m_pCredits->m_dwCryptRndChallengeFrom = dwRandom;

#ifdef DEBUG_SHOW_SECUREID
	DEBUG_ONLY(AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Received SecIdentState Packet, state: %u"), pbytePacket[0]));
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	InfoPacketsReceived() is called once the HELLO/HELLOANSWER and EMULEINFO/EMULEINFOANSWER information has
//	been received.
void CUpDownClient::InfoPacketsReceived()
{
	ASSERT (m_eInfoPacketsReceived == IP_BOTH);

	m_eInfoPacketsReceived = IP_NONE;

//	If this client supports secure credits...
	if (m_byteSupportSecIdent)
	{
		SendSecIdentStatePacket();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CUpDownClient::GetCountryName() const
{
	return g_App.m_pIP2Country->GetCountryNameByIndex(m_uUserCountryIdx);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ResetIP2Country()
{
	m_uUserCountryIdx = g_App.m_pIP2Country->GetCountryFromIP(m_dwUserIP);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::HasUserNameForbiddenStrings()
{
	if (m_byteActionsOnNameChange & AONC_FORBIDDEN_NAME_CHECK)
	{
		m_bHasUserNameForbiddenStrings = IsStolenName(m_strUserName);

		m_byteActionsOnNameChange &= ~AONC_FORBIDDEN_NAME_CHECK;
	}

	return m_bHasUserNameForbiddenStrings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::HasMODNameForbiddenStrings()
{
	if (m_bIsMODNameChanged)
	{
		m_bHasMODNameForbiddenStrings = IsLeecherType(m_strModString);

	//	Check if a client isn't eMulePlus, but uses our MOD string
		if (!m_bHasMODNameForbiddenStrings && (m_dwPlusVers == 0))
		{
			m_bHasMODNameForbiddenStrings = (m_strModString == _T(PLUS_VERSION_STR));
		}

		m_bIsMODNameChanged = false;
	}

	return m_bHasMODNameForbiddenStrings;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CUpDownClient::GetClientIconIndex() const
{
	uint32	dwImgIdx = GetClientSoft();

	if ((dwImgIdx == SO_EMULE) || (dwImgIdx == SO_PLUS))
	{
		if (m_pCredits->GetCurrentIdentState(GetIP()) != IS_IDENTIFIED)
			dwImgIdx = SO_OLDEMULE;
	}
	else if (dwImgIdx > SO_UNKNOWN)
		dwImgIdx = SO_UNKNOWN;

	return static_cast<int>(dwImgIdx);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const CString& CUpDownClient::GetFullSoftVersionString() const
{
	return m_strClientSoft;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CUpDownClient::GetClientNameWithSoftware() const
{
	CString strBuf;

	strBuf.Format(_T("'%s' (%s)"), m_strUserName, m_strClientSoft);
	return strBuf;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::IsObfuscatedConnectionEstablished() const
{
	if ((m_pRequestSocket != NULL) && m_pRequestSocket->IsConnected())
#ifdef _CRYPT_READY
		return m_pRequestSocket->IsObfusicating();
#else
		return false;
#endif
	else
		return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::ShouldReceiveCryptUDPPackets() const
{
#ifdef _CRYPT_READY
#else
	return false;
#endif
}
