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

//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-

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
}

CClientUDPSocket::~CClientUDPSocket()
{
    theAppPtr->uploadBandwidthThrottler->RemoveFromAllQueues(this); // ZZ:UploadBandWithThrottler (UDP)

    POSITION pos = controlpacket_queue.GetHeadPosition();
	while (pos){
		UDPPack* p = controlpacket_queue.GetNext(pos);
		delete p->packet;
		delete p;
	}
}

void CClientUDPSocket::OnReceive(int nErrorCode)
{
	if (nErrorCode)
	{
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Error: Client UDP socket, error on receive event: %s"), GetErrorMessage(nErrorCode, 1));
	}

	//Xman -Reask sources after IP change- v4 
	theAppPtr->last_traffic_reception=::GetTickCount(); //Threading Info: synchronized with the main thread
	//Xman end

	BYTE buffer[5000];
	SOCKADDR_IN sockAddr = {0};
	int iSockAddrLen = sizeof sockAddr;
	int nRealLen = ReceiveFrom(buffer, sizeof buffer, (SOCKADDR*)&sockAddr, &iSockAddrLen);
	//Xman
	// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	if (nRealLen > 0)		
		theAppPtr->pBandWidthControl->AddeMuleInUDPOverall(nRealLen);
	//Xman end
	if (!(theAppPtr->ipfilter->IsFiltered(sockAddr.sin_addr.S_un.S_addr) || theAppPtr->clientlist->IsBannedClient(sockAddr.sin_addr.S_un.S_addr)))
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
										, Kademlia::CKadUDPKey(nSenderVerifyKey, theAppPtr->GetPublicIP(false)) );
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
						//zz_fly :: Anti-Leecher
						//note: Clients sending a KAD tag from 0.49a+ but pretending to be 0.48a
						byte byOpcode = pBuffer[1];
						if(byOpcode == KADEMLIA_FIREWALLED2_REQ)
						{
							CUpDownClient* client = theAppPtr->clientlist->FindClientByIP(sockAddr.sin_addr.S_un.S_addr);
							if(client != NULL && client->GetClientSoft() == SO_EMULE && client->GetVersion() != 0 && client->GetVersion() < MAKE_CLIENT_VERSION(0, 49, 0))
							{
								client->BanLeecher(_T("Detected Vagaa"),5); //Bad Leecher, Hard Ban
								break;
							}
						}
						//zz_fly :: Anti-Leecher end
						if (nPacketLen >= 2)
							Kademlia::CKademlia::ProcessPacket(pBuffer, nPacketLen, ntohl(sockAddr.sin_addr.S_un.S_addr), ntohs(sockAddr.sin_port)
							, (Kademlia::CPrefs::GetUDPVerifyKey(sockAddr.sin_addr.S_un.S_addr) == nReceiverVerifyKey)
							, Kademlia::CKadUDPKey(nSenderVerifyKey, theAppPtr->GetPublicIP(false)) );
						else
							throw CString(_T("Kad packet too short"));
						break;
					}
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
					client = theAppPtr->clientlist->FindClientByIP_UDP(sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
				else
					client = theAppPtr->clientlist->FindClientByIP_KadPort(sockAddr.sin_addr.S_un.S_addr, ntohs(sockAddr.sin_port));
				if (client)
					strClientInfo = client->DbgGetClientInfo();
				else
					strClientInfo.Format(_T("%s:%u"), ipstr(sockAddr.sin_addr), ntohs(sockAddr.sin_port));

				DebugLogWarning(_T("Client UDP socket: prot=0x%02x  opcode=0x%02x  sizeaftercrypt=%u realsize=%u  %s: %s"), pBuffer[0], pBuffer[1], nPacketLen, nRealLen, strError, strClientInfo);
			}
		}
		else if (nPacketLen == SOCKET_ERROR)
		{
			DWORD dwError = WSAGetLastError();
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_ReaskCallbackUDP", NULL, NULL, ip);
#endif //zz_fly :: DummyCut
			theStats.AddDownDataOverheadOther(size);
			CUpDownClient* buddy = theAppPtr->clientlist->GetBuddy();
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__ReaskCallbackTCP", buddy);
#endif //zz_fly :: DummyCut
					theStats.AddUpDataOverheadFileRequest(response->size);
					buddy->SendPacket(response, true);
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
			CKnownFile* reqfile = theAppPtr->sharedfiles->GetFileByID(reqfilehash);
			
			bool bSenderMultipleIpUnknown = false;
			CUpDownClient* sender = theAppPtr->uploadqueue->GetWaitingClientByIP_UDP(ip, port, true, &bSenderMultipleIpUnknown);
			if (!reqfile)
			{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
				if (thePrefs.GetDebugClientUDPLevel() > 0) {
					DebugRecv("OP_ReaskFilePing", NULL, reqfilehash, ip);
					DebugSend("OP__FileNotFound", NULL);
				}
#endif //zz_fly :: DummyCut
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
				if (thePrefs.GetDebugClientUDPLevel() > 0)
					DebugRecv("OP_ReaskFilePing", sender, reqfilehash);
#endif //zz_fly :: DummyCut
				//Xman uploading problem client
				//we don't answer this client, to force a tcp connection, then we can add him to upload
				//Xman 4.8.2 update: we don't answer every client with the flag... because there are buggy clients (shareaza) out 
				//with send UDP forever although they are LowIDs
				if(sender->m_bAddNextConnect && theAppPtr->uploadqueue->AcceptNewClient(true))
					break;
				//Xman end

				//Xman Xtreme Mod
				//don't answer wrong filereaskpings, test if last action was OP_STARTUPLOADREQ.. must be on normal behavior
				if(sender->GetLastAction()!=OP_STARTUPLOADREQ)
				{
					//AddDebugLogLine(false,_T("-->Filereaskping without OP_STARTUPLOADREQ, client: %s"), sender->DbgGetClientInfo());
					break;
				}

				//check completed sources which want to download their "complete" file
				if(sender->GetRequestFile()==reqfile && sender->HasFileComplete())
				{
					AddDebugLogLine(false, _T("->client want to download a file it has already complete: %s, %s"), reqfile->GetFileName(), sender->DbgGetClientInfo());
					break; //no answer... a ban would also be not to bad
				}
				//Xman end

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
						//Xman better passive source finding
						/*
						sender->ProcessExtendedInfo(&data_in, reqfile);
						*/
						sender->ProcessExtendedInfo(&data_in, reqfile,true);
						//Xman end
					}
					//Update our complete source counts.
					else if (sender->GetUDPVersion() > 2)
					{
						uint16 nCompleteCountLast= sender->GetUpCompleteSourcesCount();
						uint16 nCompleteCountNew = data_in.ReadUInt16();
						sender->SetUpCompleteSourcesCount(nCompleteCountNew);
						if (nCompleteCountLast != nCompleteCountNew)
						{
							reqfile->UpdatePartsInfo();
						}
					}
					CSafeMemFile data_out(128);
					if(sender->GetUDPVersion() > 3)
					{
						if (reqfile->IsPartFile())
							((CPartFile*)reqfile)->WritePartStatus(&data_out);
						else
						{
							//Xman PowerRelease
							if (!reqfile->HideOvershares(&data_out, sender))
								data_out.WriteUInt16(0);
							//Xman end
						}
					}
					data_out.WriteUInt16((uint16)(theAppPtr->uploadqueue->GetWaitingPosition(sender)));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
					if (thePrefs.GetDebugClientUDPLevel() > 0)
						DebugSend("OP__ReaskAck", sender);
#endif //zz_fly :: DummyCut
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
				if (thePrefs.GetDebugClientUDPLevel() > 0)
					DebugRecv("OP_ReaskFilePing", NULL, reqfilehash, ip);
#endif //zz_fly :: DummyCut
				// Don't answer him. We probably have him on our queue already, but can't locate him. Force him to establish a TCP connection
				if (!bSenderMultipleIpUnknown){
					if (((uint32)theAppPtr->uploadqueue->GetWaitingUserCount() + 50) > thePrefs.GetQueueSize())
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
						if (thePrefs.GetDebugClientUDPLevel() > 0)
							DebugSend("OP__QueueFull", NULL);
#endif //zz_fly :: DummyCut
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
			CUpDownClient* sender = theAppPtr->downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_QueueFull", sender, NULL, ip);
#endif //zz_fly :: DummyCut
			if (sender && sender->UDPPacketPending()){
				sender->SetRemoteQueueFull(true);
				sender->UDPReaskACK(0);
			}
			else if (sender != NULL)
				DebugLogError(_T("Received UDP Packet (OP_QUEUEFULL) which was not requested (pendingflag == false); Ignored packet - %s"), sender->DbgGetClientInfo());
			break;
		}
		case OP_REASKACK:
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theAppPtr->downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_ReaskAck", sender, NULL, ip);
#endif //zz_fly :: DummyCut
			if (sender && sender->UDPPacketPending()){
				CSafeMemFile data_in(packet, size);
				if ( sender->GetUDPVersion() > 3 )
				{
					sender->ProcessFileStatus(true, &data_in, sender->GetRequestFile());
				}
				uint16 nRank = data_in.ReadUInt16();
				sender->SetRemoteQueueFull(false);
				sender->UDPReaskACK(nRank);
				sender->AddAskedCountDown();
			}
			else if (sender != NULL)
				DebugLogError(_T("Received UDP Packet (OP_REASKACK) which was not requested (pendingflag == false); Ignored packet - %s"), sender->DbgGetClientInfo());
			
			break;
		}
		case OP_FILENOTFOUND:
		{
			theStats.AddDownDataOverheadFileRequest(size);
			CUpDownClient* sender = theAppPtr->downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_FileNotFound", sender, NULL, ip);
#endif //zz_fly :: DummyCut
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_PortTest", NULL, NULL, ip);
#endif //zz_fly :: DummyCut
			theStats.AddDownDataOverheadOther(size);
			if (size == 1){
				if (packet[0] == 0x12){
					bool ret = theAppPtr->listensocket->SendPortTestReply('1', true);
					AddDebugLogLine(true, _T("UDP Portcheck packet arrived - ACK sent back (status=%i)"), ret);
				}
			}
			break;
		}
		case OP_DIRECTCALLBACKREQ:
		{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugRecv("OP_DIRECTCALLBACKREQ", NULL, NULL, ip);
#endif //zz_fly :: DummyCut
			if (!theAppPtr->clientlist->AllowCalbackRequest(ip)){
				DebugLogWarning(_T("Ignored DirectCallback Request because this IP (%s) has sent too many request within a short time"), ipstr(ip));
				break;
			}
			// do we accept callbackrequests at all?
			if (Kademlia::CKademlia::IsRunning() && Kademlia::CKademlia::IsFirewalled())
			{
				theAppPtr->clientlist->AddTrackCallbackRequests(ip);
				CSafeMemFile data(packet, size);
				uint16 nRemoteTCPPort = data.ReadUInt16();
				uchar uchUserHash[16];
				data.ReadHash16(uchUserHash);
				uint8 byConnectOptions = data.ReadUInt8();
				CUpDownClient* pRequester = theAppPtr->clientlist->FindClientByUserHash(uchUserHash, ip, nRemoteTCPPort);
				if (pRequester == NULL) {
					pRequester = new CUpDownClient(NULL, nRemoteTCPPort, ip, 0, 0, true);
					pRequester->SetUserHash(uchUserHash);
					theAppPtr->clientlist->AddClient(pRequester);
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
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE) //zz_fly :: DummyCut :: 090213
			if (thePrefs.GetDebugClientUDPLevel() > 0)
			{
				CUpDownClient* sender = theAppPtr->downloadqueue->GetDownloadClientByIP_UDP(ip, port, true);
				Debug(_T("Unknown client UDP packet: host=%s:%u (%s) opcode=0x%02x  size=%u\n"), ipstr(ip), port, sender ? sender->DbgGetClientInfo() : _T(""), opcode, size);
			}
#endif //zz_fly :: DummyCut
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

    if(!controlpacket_queue.IsEmpty()) {
        theAppPtr->uploadBandwidthThrottler->QueueForSendingControlPacket(this);
    }
    sendLocker.Unlock();
// <-- ZZ:UploadBandWithThrottler (UDP)
}

SocketSentBytes CClientUDPSocket::SendControlData(uint32 maxNumberOfBytesToSend, uint32 /*minFragSize*/){ // ZZ:UploadBandWithThrottler (UDP)
// ZZ:UploadBandWithThrottler (UDP) -->
	// NOTE: *** This function is invoked from a *different* thread!
    sendLocker.Lock();

    uint32 sentBytes = 0;
// <-- ZZ:UploadBandWithThrottler (UDP)

	while (!controlpacket_queue.IsEmpty() && !IsBusy() && sentBytes < maxNumberOfBytesToSend){ // ZZ:UploadBandWithThrottler (UDP)
		UDPPack* cur_packet = controlpacket_queue.GetHead();
		if( GetTickCount() - cur_packet->dwTime < UDPMAXQUEUETIME )
		{
			uint32 nLen = cur_packet->packet->size+2;
			uchar* sendbuffer = new uchar[nLen];
			memcpy(sendbuffer,cur_packet->packet->GetUDPHeader(),2);
			memcpy(sendbuffer+2,cur_packet->packet->pBuffer,cur_packet->packet->size);
			
			if (cur_packet->bEncrypt && (theAppPtr->GetPublicIP() > 0 || cur_packet->bKad)){
				nLen = EncryptSendClient(&sendbuffer, nLen, cur_packet->pachTargetClientHashORKadID, cur_packet->bKad,  cur_packet->nReceiverVerifyKey, (cur_packet->bKad ? Kademlia::CPrefs::GetUDPVerifyKey(cur_packet->dwIP) : (uint16)0));
				//DEBUG_ONLY(  AddDebugLogLine(DLP_VERYLOW, false, _T("Sent obfuscated UDP packet to clientIP: %s, Kad: %s, ReceiverKey: %u"), ipstr(cur_packet->dwIP), cur_packet->bKad ? _T("Yes") : _T("No"), cur_packet->nReceiverVerifyKey) );
			}

            if (!SendTo((char*)sendbuffer, nLen, cur_packet->dwIP, cur_packet->nPort)){
                sentBytes += nLen; // ZZ:UploadBandWithThrottler (UDP)
				//Xman
				// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
				sentBytes +=(20+8); //Header
				//Xman end
				controlpacket_queue.RemoveHead();
				delete cur_packet->packet;
				delete cur_packet;
            }
			delete[] sendbuffer;
		}
		else
		{
			controlpacket_queue.RemoveHead();
			delete cur_packet->packet;
			delete cur_packet;
		}
	}

// ZZ:UploadBandWithThrottler (UDP) -->
    if(!IsBusy() && !controlpacket_queue.IsEmpty()) {
        theAppPtr->uploadBandwidthThrottler->QueueForSendingControlPacket(this);
    }
    sendLocker.Unlock();

    SocketSentBytes returnVal = { true, 0, sentBytes };
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
	return 0;
}

bool CClientUDPSocket::SendPacket(Packet* packet, uint32 dwIP, uint16 nPort, bool bEncrypt, const uchar* pachTargetClientHashORKadID, bool bKad, uint32 nReceiverVerifyKey){
	UDPPack* newpending = new UDPPack;
	newpending->dwIP = dwIP;
	newpending->nPort = nPort;
	newpending->packet = packet;
	newpending->dwTime = GetTickCount();
	newpending->bEncrypt = bEncrypt && (pachTargetClientHashORKadID != NULL || (bKad && nReceiverVerifyKey != 0));
	newpending->bKad = bKad;
	newpending->nReceiverVerifyKey = nReceiverVerifyKey;

#ifdef _DEBUG
	if (newpending->packet->size > UDP_KAD_MAXFRAGMENT)
		DebugLogWarning(_T("Sending UDP packet > UDP_KAD_MAXFRAGMENT, opcode: %X, size: %u"), packet->opcode, packet->size);
#endif

	if (newpending->bEncrypt && pachTargetClientHashORKadID != NULL)
		md4cpy(newpending->pachTargetClientHashORKadID, pachTargetClientHashORKadID);
	else
		md4clr(newpending->pachTargetClientHashORKadID);
// ZZ:UploadBandWithThrottler (UDP) -->
    sendLocker.Lock();
	controlpacket_queue.AddTail(newpending);
    sendLocker.Unlock();

    theAppPtr->uploadBandwidthThrottler->QueueForSendingControlPacket(this);
	return true;
// <-- ZZ:UploadBandWithThrottler (UDP)
}

bool CClientUDPSocket::Create()
{
	bool ret = true;

	if (thePrefs.GetUDPPort())
	{
		ret = CAsyncSocket::Create(thePrefs.GetUDPPort(), SOCK_DGRAM, FD_READ | FD_WRITE, thePrefs.GetBindAddrW()) != FALSE;
		if (ret)
		{
			m_port = thePrefs.GetUDPPort();
			// the default socket size seems to be not enough for this UDP socket
			// because we tend to drop packets if several flow in at the same time
			int val = 64 * 1024;
			if (!SetSockOpt(SO_RCVBUF, &val, sizeof(val)))
				DebugLogError(_T("Failed to increase socket size on UDP socket"));
		}
	}

	//Xman
	//ACAT UPnP
	/*
	if (ret && thePrefs.GetUDPPort()){
		if (thePrefs.GetUPnPNat()){
			MyUPnP::UPNPNAT_MAPPING mapping;

			mapping.internalPort = mapping.externalPort = thePrefs.GetUDPPort();
			mapping.protocol = MyUPnP::UNAT_UDP;
			mapping.description = "UDP Port";
			if (theAppPtr->AddUPnPNatPort(&mapping, thePrefs.GetUPnPNatTryRandom()))
				thePrefs.SetUPnPUDPExternal(mapping.externalPort);
		}
		else{
			thePrefs.SetUPnPUDPExternal(thePrefs.GetUDPPort());
		}
	}
	*/
	//Xman End

	if (ret)
		m_port = thePrefs.GetUDPPort();

	return ret;
}

bool CClientUDPSocket::Rebind()
{
	//Official UPNP
	if (thePrefs.GetUDPPort() == m_port)
	//ACAT UPnP
	//if (thePrefs.udpport == m_port)
		return false;
	Close();

	//Xman
	//ACAT UPnP
	/*
	if(thePrefs.GetUPnPNat())
	{
		if(theAppPtr->m_UPnPNat.RemoveSpecifiedPort(thePrefs.m_iUPnPUDPExternal, MyUPnP::UNAT_UDP))
			AddLogLine(false, _T("UPNP: removed UDP-port %u"), thePrefs.m_iUPnPUDPExternal);
		else
			AddLogLine(false, _T("UPNP: failed to remove UDP-port %u"), thePrefs.m_iUPnPUDPExternal);
	}
	thePrefs.m_iUPnPUDPExternal=0;
	*/
	//Xman End
	return Create();
}

//zz_fly :: Rebind UPnP on IP-change :: start
//ACAT UPnP
/*
bool CClientUDPSocket::RebindUPnP(){ 
	if(theAppPtr->m_UPnPNat.RemoveSpecifiedPort(thePrefs.m_iUPnPUDPExternal, MyUPnP::UNAT_UDP))
	{
		AddLogLine(false, _T("UPNP: removed UDP-port %u"), thePrefs.m_iUPnPUDPExternal);
		thePrefs.m_iUPnPUDPExternal=0;
		MyUPnP::UPNPNAT_MAPPING mapping;
		mapping.internalPort = mapping.externalPort = thePrefs.GetUDPPort();
		mapping.protocol = MyUPnP::UNAT_UDP;
		mapping.description = "UDP Port";
		if (theAppPtr->AddUPnPNatPort(&mapping, thePrefs.GetUPnPNatTryRandom()))
		{
			thePrefs.SetUPnPUDPExternal(mapping.externalPort);
			return true;
		}
		else
			thePrefs.SetUPnPUDPExternal(thePrefs.GetUDPPort());
	}
	else
		AddLogLine(false, _T("UPNP: failed to remove UDP-port %u"), thePrefs.m_iUPnPUDPExternal);
	return false;
} 
*/
//zz_fly :: Rebind UPnP on IP-change :: end