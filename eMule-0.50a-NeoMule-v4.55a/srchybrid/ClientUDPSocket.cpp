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
#include "emule.h"
#include "ClientUDPSocket.h"
#include "Packets.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "PartFile.h"
#include "SharedFileList.h"
#include "UploadQueue.h"
#include "UpDownClient.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "ClientList.h"
#include "Listensocket.h"
#include <zlib/zlib.h>
#include "kademlia/kademlia/Kademlia.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "kademlia/net/KademliaUDPListener.h"
#include "kademlia/io/IOException.h"
#include "IPFilter.h"
#include "Log.h"
#include "EncryptedDatagramSocket.h"
#include "./kademlia/kademlia/prefs.h"
#include "./kademlia/utils/KadUDPKey.h"
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "Neo/NeoOpcodes.h" // NEO: NMP - [NeoModProt] <-- Xanatos --
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
#include "Neo/NatT/NatManager.h"
#include "Neo/NatT/NatTunnel.h" // NEO: NATT - [NatTraversal]
#include "sockets.h"
#include "server.h"
#include "serversocket.h"
#include "ServerList.h"
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
#include "Neo/LanCast/Lancast.h"
#endif //LANCAST // NEO: NLC END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// CClientUDPSocket

CClientUDPSocket::CClientUDPSocket()
{
	m_bWouldBlock = false;
	m_port=0;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	m_pendingPackets = 0;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	lastPriorityPacket = NULL;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
}

CClientUDPSocket::~CClientUDPSocket()
{
    theApp.uploadBandwidthThrottler->RemoveFromAllQueues(this); // ZZ:UploadBandWithThrottler (UDP)
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	theApp.downloadBandwidthThrottler->RemoveFromAllQueues(this);
	theApp.downloadqueue->RemoveFromProcessQueue(this);
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

    POSITION pos = controlpacket_queue.GetHeadPosition();
	while (pos){
		UDPPack* p = controlpacket_queue.GetNext(pos);
		delete p->packet;
		delete p;
	}

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
    pos = downloaddata_queue.GetHeadPosition();
	while (pos){
		CRawPacket* data = downloaddata_queue.GetNext(pos);
		char *rawdata = data->DetachPacket();
		delete [] rawdata;
		delete data;
	}
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
}

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
int	CClientUDPSocket::Receive(uint32 size, UINT hyperreceiving){
	ASSERT(m_pendingPackets || hyperreceiving);

	int recivedBytes = 0;

	static BYTE buffer[5000];
	SOCKADDR_IN sockAddr = {0};
	int iSockAddrLen = sizeof sockAddr;

	receiveLocker.Lock();
	for(;(m_pendingPackets > 0 || hyperreceiving) && recivedBytes < (int)size;m_pendingPackets--) {
	
		int length = ReceiveFrom(buffer, sizeof buffer, (SOCKADDR*)&sockAddr, &iSockAddrLen);
		CReceiveData* data = 0;
		if (length > 0) {
 #ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP]
			if(buffer[1] != OP_NAT_DATA) // Note: we don't count incomming nat traffic yet, it will be counted when the CEMSocket recive's is
 #endif //NATTUNNELING // NEO: UTCP END
			{
 #ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
				theApp.bandwidthControl->AddDown(length, true);
 #endif // NEO_BC // NEO: NBC END
				recivedBytes += length;
			}
			data = new CReceiveData((char*)buffer, length);
			data->sockAddr = sockAddr;
			downloaddata_queue.AddTail(data);
		}
		else if (length == SOCKET_ERROR) {
			DWORD dwError = WSAGetLastError();
			if (dwError == WSAEWOULDBLOCK) {
				m_pendingPackets = 0;
			}
			else {
				data = new CReceiveData((char*)buffer, 4);
				data->sockAddr = sockAddr;
				data->dwError = dwError;
				downloaddata_queue.AddTail(data);
			}
			break;
		}
	}
	receiveLocker.Unlock();

	return recivedBytes;
}

bool CClientUDPSocket::ProcessData(bool /*ignore*/){
	receiveLocker.Lock();
	while (downloaddata_queue.GetCount()){
		CReceiveData* data = downloaddata_queue.RemoveHead();
		int length  = data->size;
		SOCKADDR_IN sockAddr = data->sockAddr;
		BYTE* buffer = (BYTE*)data->DetachPacket(); // floating index initialized with begin of buffer
		DWORD dwError = data->dwError;
		receiveLocker.Unlock();

		ProcessReceivedData(buffer,length,sockAddr,sizeof sockAddr,dwError);

		delete [] buffer;
		delete data;
		receiveLocker.Lock();
	}
	bool ret = onDownControlQueue && NeoPrefs.UseDownloadBandwidthThrottler();
	receiveLocker.Unlock();
	return ret;
}
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --

void CClientUDPSocket::OnReceive(int nErrorCode)
{
	if (nErrorCode)
	{
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Error: Client UDP socket, error on receive event: %s"), GetErrorMessage(nErrorCode, 1));
	}
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	else if(NeoPrefs.UseDownloadBandwidthThrottler()){
		receiveLocker.Lock();
		m_pendingPackets ++;
		receiveLocker.Unlock();
		theApp.downloadqueue->AddToProcessQueue(this);
		theApp.downloadBandwidthThrottler->QueueForReceivingPacket(this);
	} 
	else
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
	{
		BYTE buffer[5000];
		SOCKADDR_IN sockAddr = {0};
		int iSockAddrLen = sizeof sockAddr;
		int nRealLen = ReceiveFrom(buffer, sizeof buffer, (SOCKADDR*)&sockAddr, &iSockAddrLen);

		// NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
		DWORD dwError = NULL;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
		if (nRealLen > 0) 
		{
 #ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP]
			if(buffer[1] != OP_NAT_DATA) // Note: we don't count incomming nat traffic yet, it will be counted when the CEMSocket recive's is
 #endif //NATTUNNELING // NEO: UTCP END
				theApp.bandwidthControl->AddDown(nRealLen, true);
		}
		else
#endif // NEO_BC // NEO: NBC END
		if(nRealLen == SOCKET_ERROR)
			dwError = WSAGetLastError();

		ProcessReceivedData(buffer,nRealLen,sockAddr,iSockAddrLen,dwError);
		// NEO: NDBT END <-- Xanatos --
	}
}

void CClientUDPSocket::ProcessReceivedData(BYTE* buffer, int nRealLen, SOCKADDR_IN &sockAddr, int iSockAddrLen, DWORD dwError) // NEO: NDBT - [NeoDownloadBandwidthThrottler] <-- Xanatos --
{

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	// Check if this packet comes from our server and is a nat callback request
	// Note: there is a good reason why the server don't send the packet obfuscated the emule way,
	//		it uses the server obfuscation to allow the client to verify (by the server key used for decrybtion) 
	//		do the packet comes realy from him,	because anyone could sent a udp packet with his IP and obfuscated in the way cleints do.
	if(CServer* cur_server = theApp.serverconnect->GetCurrentServer())
	{
		if(CServer* pServer = theApp.serverlist->GetServerByIP(cur_server->GetIP()))
		{
			if(sockAddr.sin_addr.S_un.S_addr == pServer->GetIP() && theApp.serverconnect->udpsocket != NULL)
			{
				uint32 dwKey = 0;
				if (pServer->GetCryptPingReplyPending() && pServer->GetChallenge() != 0 /* && pServer->GetPort() == ntohs(sockAddr.sin_port) - 12 */)
					dwKey = pServer->GetChallenge();
				else
					dwKey = pServer->GetServerKeyUDP();

				ASSERT( dwKey != 0 );
				BYTE* pBuffer;
				int nPayLoadLen = DecryptReceivedServer(buffer, nRealLen, &pBuffer, dwKey,sockAddr.sin_addr.S_un.S_addr);

				//if (nPayLoadLen == nRealLen){
				//	DebugLogWarning(_T("Expected encrypted packet, but received unencrytped from server %s, UDPKey %u, Challenge: %u"), pServer->GetListName(), pServer->GetServerKeyUDP(), pServer->GetChallenge());
				//	return; // we don't accept unencrypted callback requests for security reasons !!!
				//}else if (thePrefs.GetDebugServerUDPLevel() > 0)
				//	DEBUG_ONLY(DebugLog(_T("Received encrypted packet from server %s, UDPKey %u, Challenge: %u"), pServer->GetListName(), pServer->GetServerKeyUDP(), pServer->GetChallenge()));

				if (pBuffer[0] == OP_EDONKEYPROT && pBuffer[1] == OP_NAT_CALLBACKREQUESTED_UDP)
					theApp.serverconnect->connectedsocket->ProcessPacket(pBuffer+2, nPayLoadLen-2, OP_NAT_CALLBACKREQUESTED); // use the same prozessing function
				else if (thePrefs.GetDebugServerUDPLevel() > 0)
					Debug(_T("***NOTE: ServerUDPMessage from %s:%u - Unknown protocol 0x%02x\n, Encrypted: %s"), ipstr(sockAddr.sin_addr), ntohs(sockAddr.sin_port)-4, pBuffer[0], (nPayLoadLen == nRealLen) ? _T("Yes") : _T("No"));
				return;
			}
		}
	}
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --

	//if (!(theApp.ipfilter->IsFiltered(sockAddr.sin_addr.S_un.S_addr) || theApp.clientlist->IsBannedClient(sockAddr.sin_addr.S_un.S_addr)))
	if ((
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	  !theApp.lancast->IsLanIP(sockAddr.sin_addr.S_un.S_addr) &&
#endif //LANCAST // NEO: NLC END <-- Xanatos --
	  !(theApp.ipfilter->IsFiltered(sockAddr.sin_addr.S_un.S_addr)) ||
	    theApp.clientlist->IsBannedClient(sockAddr.sin_addr.S_un.S_addr)))
	{
		BYTE* pBuffer;
		uint32 nReceiverVerifyKey;
		uint32 nSenderVerifyKey;
		int nPacketLen = DecryptReceivedClient(buffer, nRealLen, &pBuffer, sockAddr.sin_addr.S_un.S_addr, &nReceiverVerifyKey, &nSenderVerifyKey);
		if (nPacketLen >= 1)
		{
			CString strError;
			try
			{
				switch (pBuffer[0])
				{
					case OP_EMULEPROT:
					{
						if (nPacketLen >= 2)
							ProcessPacket(pBuffer+2, nPacketLen-2, pBuffer[1], sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
						else
							throw CString(_T("eMule packet too short"));
						break;
					}
					case OP_KADEMLIAPACKEDPROT:
					{
						theStats.AddDownDataOverheadKad(nPacketLen);
						if (nPacketLen >= 2)
						{
							uint32 nNewSize = nPacketLen*10+300;
							BYTE* unpack = NULL;
							uLongf unpackedsize = 0;
							int iZLibResult = 0;
							do {
								delete[] unpack;
								unpack = new BYTE[nNewSize];
								unpackedsize = nNewSize-2;
								iZLibResult = uncompress(unpack+2, &unpackedsize, pBuffer+2, nPacketLen-2);
								nNewSize *= 2; // size for the next try if needed
							} while (iZLibResult == Z_BUF_ERROR && nNewSize < 250000);

							if (iZLibResult == Z_OK)
							{
								unpack[0] = OP_KADEMLIAHEADER;
								unpack[1] = pBuffer[1];
								try
								{
									Kademlia::CKademlia::ProcessPacket(unpack, unpackedsize+2, ntohl(sockAddr.sin_addr.S_un.S_addr), ntohs(sockAddr.sin_port)
										, (Kademlia::CPrefs::GetUDPVerifyKey(sockAddr.sin_addr.S_un.S_addr) == nReceiverVerifyKey)
										, Kademlia::CKadUDPKey(nSenderVerifyKey, theApp.GetPublicIP(false)) );
								}
								catch(...)
								{
									delete[] unpack;
									throw;
								}
							}
							else
							{
								delete[] unpack;
								CString strError;
								strError.Format(_T("Failed to uncompress Kad packet: zip error: %d (%hs)"), iZLibResult, zError(iZLibResult));
								throw strError;
							}
							delete[] unpack;
						}
						else
							throw CString(_T("Kad packet (compressed) too short"));
						break;
					}
					case OP_KADEMLIAHEADER:
					{
						theStats.AddDownDataOverheadKad(nPacketLen);
						if (nPacketLen >= 2)
							Kademlia::CKademlia::ProcessPacket(pBuffer, nPacketLen, ntohl(sockAddr.sin_addr.S_un.S_addr), ntohs(sockAddr.sin_port)
							, (Kademlia::CPrefs::GetUDPVerifyKey(sockAddr.sin_addr.S_un.S_addr) == nReceiverVerifyKey)
							, Kademlia::CKadUDPKey(nSenderVerifyKey, theApp.GetPublicIP(false)) );
						else
							throw CString(_T("Kad packet too short"));
						break;
					}
					// NEO: NMP - [NeoModProt] -- Xanatos -->
					case OP_MODPROT:
					{
						if (nPacketLen >= 2)
							ProcessModPacket(pBuffer+2, nPacketLen-2, pBuffer[1], sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
						else
							throw CString(_T("Mod packet too short"));
						break;
					}
					case OP_MODPACKEDPROT:
					{
						if (nPacketLen >= 2)
						{
							uint32 nNewSize = nPacketLen*10+300;
							byte* unpack = new byte[nNewSize];
							uLongf unpackedsize = nNewSize-2;
							int iZLibResult = uncompress(unpack+2, &unpackedsize, pBuffer+2, nPacketLen-2);
							if (iZLibResult == Z_OK)
							{
								unpack[0] = OP_MODPROT;
								unpack[1] = pBuffer[1];
								ProcessModPacket(unpack+2, unpackedsize, unpack[1], sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
							}
							else
							{
								delete[] unpack;
								CString strError;
								strError.Format(_T("Failed to uncompress Mod packet: zip error: %d (%hs)"), iZLibResult, zError(iZLibResult));
								throw strError;
							}
							delete[] unpack;
						}
						else
							throw CString(_T("Mod protocol packet (compressed) too short"));
						break;
					}
					// NEO: NMP END <-- Xanatos --
					default:
					{
						CString strError;
						strError.Format(_T("Unknown protocol 0x%02x"), pBuffer[0]);
						throw strError;
					}
				}
			}
			catch(CFileException* error)
			{
				error->Delete();
				strError = _T("Invalid packet received");
			}
			catch(CMemoryException* error)
			{
				error->Delete();
				strError = _T("Memory exception");
			}
			catch(CString error)
			{
				strError = error;
			}
			catch(Kademlia::CIOException* error)
			{
				error->Delete();
				strError = _T("Invalid packet received");
			}
			catch(CException* error)
			{
				error->Delete();
				strError = _T("General packet error");
			}
	#ifndef _DEBUG
			catch(...)
			{
				strError = _T("Unknown exception");
				ASSERT(0);
			}
	#endif
			if (thePrefs.GetVerbose() && !strError.IsEmpty())
			{
				CString strClientInfo;
				CUpDownClient* client;
				if (pBuffer[0] == OP_EMULEPROT)
					client = theApp.clientlist->FindClientByIP_UDP(sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
				else
					client = theApp.clientlist->FindClientByIP_KadPort(sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
				if (client)
					strClientInfo = client->DbgGetClientInfo();
				else
					strClientInfo.Format(_T("%s:%u"), ipstr(sockAddr.sin_addr), ntohs(sockAddr.sin_port));

				DebugLogWarning(_T("Client UDP socket: prot=0x%02x  opcode=0x%02x  sizeaftercrypt=%u realsize=%u  %s: %s"), pBuffer[0], pBuffer[1], nPacketLen, nRealLen, strError, strClientInfo);
			}
		}
		else if (nPacketLen == SOCKET_ERROR)
		{
			//DWORD dwError = WSAGetLastError(); // NEO: NDBT - [NeoDownloadBandwidthThrottler] <-- Xanatos --
			if (dwError == WSAECONNRESET)
			{
				// Depending on local and remote OS and depending on used local (remote?) router we may receive
				// WSAECONNRESET errors. According some KB articles, this is a special way of winsock to report 
				// that a sent UDP packet was not received by the remote host because it was not listening on 
				// the specified port -> no eMule running there.
				//
				// TODO: So, actually we should do something with this information and drop the related Kad node 
				// or eMule client...
				;
			}
			if (thePrefs.GetVerbose() && dwError != WSAECONNRESET)
			{
				CString strClientInfo;
				if (iSockAddrLen > 0 && sockAddr.sin_addr.S_un.S_addr != 0 && sockAddr.sin_addr.S_un.S_addr != INADDR_NONE)
					strClientInfo.Format(_T(" from %s:%u"), ipstr(sockAddr.sin_addr), ntohs(sockAddr.sin_port));
				DebugLogError(_T("Error: Client UDP socket, failed to receive data%s: %s"), strClientInfo, GetErrorMessage(dwError, 1));
			}
		}
	}
}

bool CClientUDPSocket::ProcessPacket(const BYTE* packet, UINT size, uint8 opcode, uint32 ip, uint16 port)
{
	switch(opcode)
	{
		case OP_REASKCALLBACKUDP:
		{
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_ReaskCallbackUDP", NULL, NULL, ip);
			theStats.AddDownDataOverheadOther(size);
			CUpDownClient* buddy = theApp.clientlist->GetBuddy();
			if( buddy )
			{
				if( size < 17 || buddy->socket == NULL )
					break;
				if (!md4cmp(packet, buddy->GetBuddyID()))
				{
					PokeUInt32(const_cast<BYTE*>(packet)+10, ip);
					PokeUInt16(const_cast<BYTE*>(packet)+14, port);
					Packet* response = new Packet(OP_EMULEPROT);
					response->opcode = OP_REASKCALLBACKTCP;
					response->pBuffer = new char[size];
					memcpy(response->pBuffer, packet+10, size-10);
					response->size = size-10;
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__ReaskCallbackTCP", buddy);
					theStats.AddUpDataOverheadFileRequest(response->size);
					buddy->socket->SendPacket(response);
				}
			}
			break;
		}
		case OP_REASKFILEPING:
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CSafeMemFile data_in(packet, size);
			uchar reqfilehash[16];
			data_in.ReadHash16(reqfilehash);
			CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(reqfilehash);
			
			bool bSenderMultipleIpUnknown = false;
			CUpDownClient* sender = theApp.uploadqueue->GetWaitingClientByIP_UDP(ip, port, true, &bSenderMultipleIpUnknown);
			if (!reqfile)
			{
				if (thePrefs.GetDebugClientUDPLevel() > 0) {
					DebugRecv("OP_ReaskFilePing", NULL, reqfilehash, ip);
					DebugSend("OP__FileNotFound", NULL);
				}

				Packet* response = new Packet(OP_FILENOTFOUND,0,OP_EMULEPROT);
				theStats.AddUpDataOverheadFileRequest(response->size);
				if (sender != NULL)
					SendPacket(response, ip, port, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
				else
					SendPacket(response, ip, port, false, NULL, false, 0);
				break;
			}
			if (sender)
			{
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
				if(NeoPrefs.IsAgressionDetection() == TRUE){
					sender->AddRequestCount(reqfilehash);
					if(sender->IsBanned()){
						Packet* response = new Packet(OP_QUEUEFULL,0,OP_EMULEPROT);
						theStats.AddUpDataOverheadFileRequest(response->size);
 						SendPacket(response, ip, port, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
						break;
					}
				}
#endif //ARGOS // NEO: NA END <-- Xanatos --

				// NEO: MOD - [NewUploadState] -- Xanatos -->
				//Xman: we don't answer this client, to force a tcp connection, then we can add him to upload
				if(/*sender->m_bAddNextConnect*/ sender->GetUploadState() == US_PENDING && theApp.uploadqueue->AcceptNewClient(true)) 
					break;
				// NEO: MOD END <-- Xanatos --

				if (thePrefs.GetDebugClientUDPLevel() > 0)
					DebugRecv("OP_ReaskFilePing", sender, reqfilehash);

				//Make sure we are still thinking about the same file
				if (md4cmp(reqfilehash, sender->GetUploadFileID()) == 0)
				{
					sender->AddAskedCount();
					sender->SetLastUpRequest();
					//I messed up when I first added extended info to UDP
					//I should have originally used the entire ProcessExtenedInfo the first time.
					//So now I am forced to check UDPVersion to see if we are sending all the extended info.
					//For now on, we should not have to change anything here if we change
					//anything to the extended info data as this will be taken care of in ProcessExtendedInfo()
					//Update extended info. 
					if (sender->GetUDPVersion() > 3)
					{
						sender->ProcessExtendedInfo(&data_in, reqfile, true); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
					}
					//Update our complete source counts.
					else if (sender->GetUDPVersion() > 2)
					{
						CClientFileStatus* status = sender->GetFileStatus(reqfile, true);  // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
						//uint16 nCompleteCountLast= sender->GetUpCompleteSourcesCount();
						uint16 nCompleteCountLast = status->GetCompleteSourcesCount(); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
						uint16 nCompleteCountNew = data_in.ReadUInt16();
						//sender->SetUpCompleteSourcesCount(nCompleteCountNew);
						status->SetCompleteSourcesCount(nCompleteCountNew); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
						if (nCompleteCountLast != nCompleteCountNew)
						{
							reqfile->UpdatePartsInfo();
						}
					}
					CSafeMemFile data_out(128);
					if(sender->GetUDPVersion() > 3)
					{
						if (reqfile->IsPartFile())
						//	((CPartFile*)reqfile)->WritePartStatus(&data_out);
						//else
						// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
							((CPartFile*)reqfile)->WritePartStatus(&data_out, sender);
						else if(!reqfile->WritePartSelection(&data_out, sender))
						// NEO: IPS END <-- Xanatos --
							data_out.WriteUInt16(0);
					}
					data_out.WriteUInt16((uint16)(theApp.uploadqueue->GetWaitingPosition(sender)));
					if (thePrefs.GetDebugClientUDPLevel() > 0)
						DebugSend("OP__ReaskAck", sender);
					Packet* response = new Packet(&data_out, OP_EMULEPROT);
					response->opcode = OP_REASKACK;
					theStats.AddUpDataOverheadFileRequest(response->size);
					SendPacket(response, ip, port, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
				}
				else
				{
					DebugLogError(_T("Client UDP socket; ReaskFilePing; reqfile does not match"));
					TRACE(_T("reqfile:         %s\n"), DbgGetFileInfo(reqfile->GetFileHash()));
					TRACE(_T("sender->GetRequestFile(): %s\n"), sender->GetRequestFile() ? DbgGetFileInfo(sender->GetRequestFile()->GetFileHash()) : _T("(null)"));
				}
			}
			else
			{
				if (thePrefs.GetDebugClientUDPLevel() > 0)
					DebugRecv("OP_ReaskFilePing", NULL, reqfilehash, ip);
				// Don't answer him. We probably have him on our queue already, but can't locate him. Force him to establish a TCP connection
				if (!bSenderMultipleIpUnknown){
					if(!NeoPrefs.UseInfiniteQueue()) // NEO: TQ - [TweakUploadQueue] <-- Xanatos --
					if (((uint32)theApp.uploadqueue->GetWaitingUserCount() + 50) > thePrefs.GetQueueSize())
					{
						if (thePrefs.GetDebugClientUDPLevel() > 0)
							DebugSend("OP__QueueFull", NULL);
						Packet* response = new Packet(OP_QUEUEFULL,0,OP_EMULEPROT);
						theStats.AddUpDataOverheadFileRequest(response->size);
						SendPacket(response, ip, port, false, NULL, false, 0); // we cannot answer this one encrypted since we dont know this client
					}
				}
				else{
					DebugLogWarning(_T("UDP Packet received - multiple clients with the same IP but different UDP port found. Possible UDP Portmapping problem, enforcing TCP connection. IP: %s, Port: %u"), ipstr(ip), port); 
				}
			}
			break;
		}
		case OP_QUEUEFULL:
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_QueueFull", sender, NULL, ip);
			if (sender && sender->UDPPacketPending()){
				//sender->SetRemoteQueueFull(true);
				// NEO: FIX - [SourceCount] -- Xanatos -->
				if(sender->GetDownloadState() == DS_ONQUEUE)
					sender->SetDownloadState(DS_REMOTEQUEUEFULL);
				sender->SetRemoteQueueRank(0);
				// NEO: FIX END <-- Xanatos --
				sender->UDPReaskACK(0);
			}
			else if (sender != NULL)
				DebugLogError(_T("Received UDP Packet (OP_QUEUEFULL) which was not requested (pendingflag == false); Ignored packet - %s"), sender->DbgGetClientInfo());
			break;
		}
		case OP_REASKACK:
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_ReaskAck", sender, NULL, ip);
			if (sender && sender->UDPPacketPending()){
				CSafeMemFile data_in(packet, size);
				if ( sender->GetUDPVersion() > 3 )
				{
					sender->ProcessFileStatus(true, &data_in, sender->GetRequestFile());
				}
				uint16 nRank = data_in.ReadUInt16();
				//sender->SetRemoteQueueFull(false);
				// NEO: FIX - [SourceCount] -- Xanatos -->
				if(sender->GetDownloadState() == DS_REMOTEQUEUEFULL)
					sender->SetDownloadState(DS_ONQUEUE);
				// NEO: FIX END <-- Xanatos --
				sender->UDPReaskACK(nRank);
				sender->AddAskedCountDown();

				// NEO: NMPm - [NeoModProtMultiPacket] -- Xanatos -->
				if(sender->SupportsExtUDPStatus() && sender->GetRequestFile())
				{
					CSafeMemFile data_out(128);
					data_out.WriteHash16(sender->GetRequestFile()->GetFileHash());
				
					sender->WriteModMultiPacket(&data_out, sender->GetRequestFile(), true, true); 

					if (thePrefs.GetDebugClientUDPLevel() > 0)
						DebugSend("OP__ModMultiPacket (UDP)", sender, sender->GetRequestFile()->GetFileHash());
					Packet* response = new Packet(&data_out, OP_MODPROT);
					response->opcode = OP_MODMULTIPACKET;
					theStats.AddUpDataOverheadFileRequest(response->size);
					SendPacket(response, ip, port, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
				}
				// NEO: NMPm END <-- Xanatos --
			}
			else if (sender != NULL)
				DebugLogError(_T("Received UDP Packet (OP_REASKACK) which was not requested (pendingflag == false); Ignored packet - %s"), sender->DbgGetClientInfo());
			
			break;
		}
		case OP_FILENOTFOUND:
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_FileNotFound", sender, NULL, ip);
			if (sender && sender->UDPPacketPending()){
				sender->UDPReaskFNF(); // may delete 'sender'!
				sender = NULL;
			}
			else if (sender != NULL)
				DebugLogError(_T("Received UDP Packet (OP_FILENOTFOUND) which was not requested (pendingflag == false); Ignored packet - %s"), sender->DbgGetClientInfo());

			break;
		}
		case OP_PORTTEST:
		{
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_PortTest", NULL, NULL, ip);
			theStats.AddDownDataOverheadOther(size);
			if (size == 1){
				if (packet[0] == 0x12){
					bool ret = theApp.listensocket->SendPortTestReply('1', true);
					AddDebugLogLine(true, _T("UDP Portcheck packet arrived - ACK sent back (status=%i)"), ret);
				}
			}
			break;
		}
		case OP_DIRECTCALLBACKREQ:
		{
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_DIRECTCALLBACKREQ", NULL, NULL, ip);
			if (!theApp.clientlist->AllowCalbackRequest(ip)){
				DebugLogWarning(_T("Ignored DirectCallback Request because this IP (%s) has sent too many request within a short time"), ipstr(ip));
				break;
			}
			// do we accept callbackrequests at all?
			if (Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsFirewalled())
			{
				theApp.clientlist->AddTrackCallbackRequests(ip);
				CSafeMemFile data(packet, size);
				uint16 nRemoteTCPPort = data.ReadUInt16();
				uchar uchUserHash[16];
				data.ReadHash16(uchUserHash);
				uint8 byConnectOptions = data.ReadUInt8();
				CUpDownClient* pRequester = theApp.clientlist->FindClientByUserHash(uchUserHash, ip, nRemoteTCPPort);
				if (pRequester == NULL) {
					pRequester = new CUpDownClient(NULL, nRemoteTCPPort, ip, 0, 0, true);
					pRequester->SetUserHash(uchUserHash);
					theApp.clientlist->AddClient(pRequester);
				}
				pRequester->SetConnectOptions(byConnectOptions, true, false);
				pRequester->SetDirectUDPCallbackSupport(false);
				pRequester->SetIP(ip);
				pRequester->SetUserPort(nRemoteTCPPort);
				DEBUG_ONLY( DebugLog(_T("Accepting incoming DirectCallbackRequest from %s"), pRequester->DbgGetClientInfo()) );
				pRequester->TryToConnect();
			}
			else
				DebugLogWarning(_T("Ignored DirectCallback Request because we do not accept DirectCall backs at all (%s)"), ipstr(ip));

			break;
		}
		default:
			theStats.AddDownDataOverheadOther(size);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
			{
				CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
				Debug(_T("Unknown client UDP packet: host=%s:%u (%s) opcode=0x%02x  size=%u\n"), ipstr(ip), port, sender ? sender->DbgGetClientInfo() : _T(""), opcode, size);
			}
			return false;
	}
	return true;
}

void CClientUDPSocket::OnSend(int nErrorCode){
	if (nErrorCode){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Error: Client UDP socket, error on send event: %s"), GetErrorMessage(nErrorCode, 1));
		return;
	}

// ZZ:UploadBandWithThrottler (UDP) -->
    sendLocker.Lock();
    m_bWouldBlock = false;
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	if(!NeoPrefs.UseBlockedQueue() && !controlpacket_queue.IsEmpty()) {
		sendLocker.Unlock();
		theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this);
		return;
	}
#else
	if(!controlpacket_queue.IsEmpty()) {
		theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this);
	}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
    sendLocker.Unlock();
// <-- ZZ:UploadBandWithThrottler (UDP)
}

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
bool CClientUDPSocket::IsPrioritySend() {
	sendLocker.Lock();
	bool empty = (controlpacket_queue.GetCount() == 0);
	if (empty){
		sendLocker.Unlock();
		return false;
	}
	bool retVal = controlpacket_queue.GetHead()->priority;
	sendLocker.Unlock();
	return retVal;
}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
SocketSentBytes CClientUDPSocket::Send(uint32 maxNumberOfBytesToSend, uint32 /*minFragSize*/, bool /*onlyAllowedToSendControlPacket*/, uint16 maxNumberOfPacketsToSend)
#else
SocketSentBytes CClientUDPSocket::SendControlData(uint32 maxNumberOfBytesToSend, uint32 /*minFragSize*/) // ZZ:UploadBandWithThrottler (UDP)
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
{
// ZZ:UploadBandWithThrottler (UDP) -->
	// NOTE: *** This function is invoked from a *different* thread!
    sendLocker.Lock();

    uint32 sentBytes = 0;
// <-- ZZ:UploadBandWithThrottler (UDP)
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	uint16 sentPackets = 0;
	for (;!controlpacket_queue.IsEmpty() && (sentPackets < maxNumberOfPacketsToSend || controlpacket_queue.GetHead()->priority)
										 && sentBytes < maxNumberOfBytesToSend;sentPackets++)
#else
	while (!controlpacket_queue.IsEmpty() && !IsBusy() && sentBytes < maxNumberOfBytesToSend) // ZZ:UploadBandWithThrottler (UDP)
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
	{
		UDPPack* cur_packet = controlpacket_queue.GetHead();
		if( GetTickCount() - cur_packet->dwTime < UDPMAXQUEUETIME )
		{
			uint32 nLen = cur_packet->packet->size+2;
			uchar* sendbuffer = new uchar[nLen];
			memcpy(sendbuffer,cur_packet->packet->GetUDPHeader(),2);
			memcpy(sendbuffer+2,cur_packet->packet->pBuffer,cur_packet->packet->size);
			
			if (cur_packet->bEncrypt && (theApp.GetPublicIP() > 0 || cur_packet->bKad)){
				nLen = EncryptSendClient(&sendbuffer, nLen, cur_packet->pachTargetClientHashORKadID, cur_packet->bKad,  cur_packet->nReceiverVerifyKey, (cur_packet->bKad ? Kademlia::CPrefs::GetUDPVerifyKey(cur_packet->dwIP) : (uint16)0));
				//DEBUG_ONLY(  AddDebugLogLine(DLP_VERYLOW, false, _T("Sent obfuscated UDP packet to clientIP: %s, Kad: %s, ReceiverKey: %u"), ipstr(cur_packet->dwIP), cur_packet->bKad ? _T("Yes") : _T("No"), cur_packet->nReceiverVerifyKey) );
			}

            if (!SendTo((char*)sendbuffer, nLen, cur_packet->dwIP, cur_packet->nPort)){
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP]
				if(cur_packet->packet->opcode != OP_NAT_DATA) // Note: we don't count outgoing nat traffic yet, it was already counted when the CEMSocket sent it
				// Note: we won't get a buttleneck here as NAT packets are declared as priority packets and always send
#endif //NATTUNNELING // NEO: UTCP END
					sentBytes += nLen; // ZZ:UploadBandWithThrottler (UDP)
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
				m_bWouldBlock = false;
				if (controlpacket_queue.GetHeadPosition() == lastPriorityPacket)
					lastPriorityPacket = NULL;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
				controlpacket_queue.RemoveHead();
				delete cur_packet->packet;
				delete cur_packet;
            }
			delete[] sendbuffer;
		}
		else
		{
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
			if (controlpacket_queue.GetHeadPosition() == lastPriorityPacket)
				lastPriorityPacket = NULL;
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
			controlpacket_queue.RemoveHead();
			delete cur_packet->packet;
			delete cur_packet;
		}
	}

// ZZ:UploadBandWithThrottler (UDP) -->
#if !defined(NEO_UBT) // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
    if(!IsBusy() && !controlpacket_queue.IsEmpty()) {
        theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this);
    }
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
    sendLocker.Unlock();

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
    SocketSentBytes returnVal = { true, 0, sentBytes, 0, sentPackets };
#else
    SocketSentBytes returnVal = { true, 0, sentBytes };
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
    return returnVal;
// <-- ZZ:UploadBandWithThrottler (UDP)
}

int CClientUDPSocket::SendTo(char* lpBuf,int nBufLen,uint32 dwIP, uint16 nPort){
	// NOTE: *** This function is invoked from a *different* thread!
	uint32 result = CAsyncSocket::SendTo(lpBuf,nBufLen,nPort,ipstr(dwIP));
	if (result == (uint32)SOCKET_ERROR){
		uint32 error = GetLastError();
		if (error == WSAEWOULDBLOCK){
			m_bWouldBlock = true;
			return -1;
		}
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Error: Client UDP socket, failed to send data to %s:%u: %s"), ipstr(dwIP), nPort, GetErrorMessage(error, 1));
	}
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	else
 #ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP]
		if(lpBuf[1] != OP_NAT_DATA) // Note: we don't count incomming nat traffic yet, it will be counted when the CEMSocket recive's is
 #endif //NATTUNNELING // NEO: UTCP END
			theApp.bandwidthControl->AddUp(result,true);
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	return 0;
}

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
bool PriorityPacket(uint8 prot, uint8 opcode)
{
	switch(prot){
		case OP_KADEMLIAHEADER:
		case OP_KADEMLIAPACKEDPROT:
			if (opcode == KADEMLIA_FIND_NODE || opcode == KADEMLIA_REQ || opcode == KADEMLIA_RES ||
			opcode == KADEMLIA2_REQ || opcode == KADEMLIA2_RES || // Kad 2.0
			opcode == KADEMLIA_HELLO_REQ || opcode == KADEMLIA_HELLO_RES ||
			opcode == KADEMLIA2_HELLO_REQ || opcode == KADEMLIA2_HELLO_RES || // Kad 2.0
			opcode == KADEMLIA_BOOTSTRAP_REQ || opcode == KADEMLIA_BOOTSTRAP_RES ||
			opcode == KADEMLIA2_BOOTSTRAP_REQ || opcode == KADEMLIA2_BOOTSTRAP_RES ||// Kad 2.0
			opcode == KADEMLIA_FIREWALLED_REQ || opcode == KADEMLIA_FIREWALLED_RES || opcode == KADEMLIA_FIREWALLED_ACK_RES)
				return Kademlia::CKademlia::GetPrefs()->GetRecheckIP();
			if (opcode == KADEMLIA_FINDBUDDY_REQ || opcode == KADEMLIA_FINDBUDDY_RES)
				return Kademlia::CKademlia::GetPrefs()->GetFindBuddy();
		default:
			return false;
	}
}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

bool CClientUDPSocket::SendPacket(Packet* packet, uint32 dwIP, uint16 nPort, bool bEncrypt, const uchar* pachTargetClientHashORKadID, bool bKad, uint32 nReceiverVerifyKey){
	UDPPack* newpending = new UDPPack;
	newpending->dwIP = dwIP;
	newpending->nPort = nPort;
	newpending->packet = packet;
	newpending->dwTime = GetTickCount();
	newpending->bEncrypt = bEncrypt && (pachTargetClientHashORKadID != NULL || (bKad && nReceiverVerifyKey != 0));
	newpending->bKad = bKad;
	newpending->nReceiverVerifyKey = nReceiverVerifyKey;

	if (newpending->bEncrypt && pachTargetClientHashORKadID != NULL)
		md4cpy(newpending->pachTargetClientHashORKadID, pachTargetClientHashORKadID);
	else
		md4clr(newpending->pachTargetClientHashORKadID);

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	newpending->priority = PriorityPacket(packet->prot,packet->opcode);
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

// ZZ:UploadBandWithThrottler (UDP) -->
    sendLocker.Lock();
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	if (newpending->priority) {
		if (!lastPriorityPacket){
			controlpacket_queue.AddHead(newpending);
			lastPriorityPacket = controlpacket_queue.GetHeadPosition();
		}else {
			controlpacket_queue.InsertAfter(lastPriorityPacket, newpending);
			controlpacket_queue.GetNext(lastPriorityPacket);
		}
	}
	else
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
		controlpacket_queue.AddTail(newpending);
    sendLocker.Unlock();

    theApp.uploadBandwidthThrottler->QueueForSendingControlPacket(this);
	return true;
// <-- ZZ:UploadBandWithThrottler (UDP)
}

bool CClientUDPSocket::Create()
{
	bool ret = true;

	if (thePrefs.GetUDPPort())
	{
		//ret = CAsyncSocket::Create(thePrefs.GetUDPPort(), SOCK_DGRAM, FD_READ | FD_WRITE, thePrefs.GetBindAddrW()) != FALSE;
		ret = CAsyncSocket::Create(thePrefs.GetUDPPort(), SOCK_DGRAM, FD_READ | FD_WRITE, theApp.GetBindAddress()) != FALSE; // NEO: MOD - [BindToAdapter]
		if (ret)
			m_port = thePrefs.GetUDPPort();
	}

	if (ret)
		m_port = thePrefs.GetUDPPort();

	return ret;
}

bool CClientUDPSocket::Rebind()
{
	if (thePrefs.GetUDPPort() == m_port)
		return false;
	Close();
	return Create();
}

// NEO: NMP - [NeoModProt] -- Xanatos -->
bool CClientUDPSocket::ProcessModPacket(const BYTE* packet, UINT size, uint8 opcode, uint32 ip, uint16 port)
{
	switch(opcode)
	{
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		case OP_XS_MULTICALLBACKUDP:
		{
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_XS_MultiCallbackUDP", NULL, NULL, ip);
			theStats.AddDownDataOverheadOther(size);

			CUpDownClient* buddy = theApp.clientlist->FindClientByUserHash(packet);
			if( buddy && buddy->GetXsBuddyStatus() == XB_LOW_BUDDY ) // is this cleint realy our buddy
			{
				if( size < 17 || buddy->socket == NULL )
					break;

				PokeUInt32(const_cast<BYTE*>(packet)+10, ip);
				PokeUInt16(const_cast<BYTE*>(packet)+14, port);
				Packet* response = new Packet(OP_MODPROT);
				response->opcode = OP_XS_MULTICALLBACKTCP;
				response->pBuffer = new char[size];
				memcpy(response->pBuffer, packet+10, size-10);
				response->size = size-10;
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__XS_ReaskCallbackTCP", buddy);
				theStats.AddUpDataOverheadFileRequest(response->size);
				buddy->socket->SendPacket(response);
			}
			break;
		}
#endif //NATTUNNELING // NEO: NATT END
		// NEO: NMPm - [NeoModProtMultiPacket]
		case OP_MODMULTIPACKET:
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CSafeMemFile data_in(packet, size);
			uchar reqfilehash[16];
			data_in.ReadHash16(reqfilehash);

			bool bSenderMultipleIpUnknown = false;
			CUpDownClient* sender = theApp.uploadqueue->GetWaitingClientByIP_UDP(ip, port, true, &bSenderMultipleIpUnknown);
			if (!sender)
				break;

			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_ModMultiPacket (UDP)", sender, reqfilehash);

			CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(reqfilehash);
			if (!reqfile)
				break;

			//Make sure we are still thinking about the same file
			if (md4cmp(reqfilehash, sender->GetUploadFileID()))
			{
				DebugLogError(_T("Client UDP socket; ModMultiPacket; reqfile does not match"));
				TRACE(_T("reqfile:         %s\n"), DbgGetFileInfo(reqfile->GetFileHash()));
				TRACE(_T("sender->GetRequestFile(): %s\n"), sender->GetRequestFile() ? DbgGetFileInfo(sender->GetRequestFile()->GetFileHash()) : _T("(null)"));
				break;
			}
			
			sender->ReadModMultiPacket(&data_in,reqfile);

			///////////////////// answer

			CSafeMemFile data_out(128);
			
			data_out.WriteHash16(reqfile->GetFileHash());
			sender->WriteModMultiPacket(&data_out, reqfile, false, true);
					
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugSend("OP_ModMultiPacketAns (UDP)", sender);

			Packet* response = new Packet(&data_out, OP_MODPROT);
			response->opcode = OP_MODMULTIPACKETANSWER;
			theStats.AddUpDataOverheadFileRequest(response->size);
			SendPacket(response, ip, port, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, NULL);

			break;
		}
		case OP_MODMULTIPACKETANSWER: 
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CSafeMemFile data_in(packet, size);
			uchar reqfilehash[16];
			data_in.ReadHash16(reqfilehash);

			CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
			if (!sender)
				break;

			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_ModMultiPacketAns (UDP)", sender, NULL, ip);

			CPartFile* reqfile = theApp.downloadqueue->GetFileByID(reqfilehash);
			if(reqfile)
				break;

			sender->ReadModMultiPacket(&data_in,reqfile);
			sender->ProcessModFileStatus(true, reqfile);

			break;
		}
		// NEO: NMPm END

		default:

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP]
			if(opcode >= OP_NAT_SYN && opcode <= OP_NAT_RST)
			{
				theApp.natmanager->DispatchUmTCPPacket(packet,size,opcode,ip,port);
				return true;
			}
			else if(opcode == OP_NAT_PING)
			{
				theApp.natmanager->ProcessNatPingPacket(packet,size,ip,port);
				return true;
			}
			else if(opcode >= 0xD0 && opcode <= 0xDF) // this range is reserved for NAT-T
			{
				if (thePrefs.GetDebugClientUDPLevel() > 0){
					CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
					if(!sender)
						theApp.uploadqueue->GetWaitingClientByIP_UDP(ip, port, true);
					Debug(_T("Unknown client UDP packet in NAT-T range !!!: host=%s:%u (%s) opcode=0x%02x  size=%u\n"), ipstr(ip), port, sender ? sender->DbgGetClientInfo() : _T(""), opcode, size);
				}
				return false;
			}
#endif //NATTUNNELING // NEO: UTCP END

			theStats.AddDownDataOverheadOther(size);
			if (thePrefs.GetDebugClientUDPLevel() > 0){
				CUpDownClient* sender = theApp.downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
				if(!sender)
					theApp.uploadqueue->GetWaitingClientByIP_UDP(ip, port, true);
				Debug(_T("Unknown client UDP MOD packet: host=%s:%u (%s) opcode=0x%02x  size=%u\n"), ipstr(ip), port, sender ? sender->DbgGetClientInfo() : _T(""), opcode, size);
			}
			return false;
	}
	return true;
}
// NEO: NMP END <-- Xanatos --
