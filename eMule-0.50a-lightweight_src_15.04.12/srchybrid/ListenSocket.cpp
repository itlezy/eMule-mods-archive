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
#include "DebugHelpers.h"
#include "emule.h"
#include "ListenSocket.h"
#include "opcodes.h"
#include "UpDownClient.h"
#include "ClientList.h"
#include "OtherFunctions.h"
#include "DownloadQueue.h"
#include "Statistics.h"
#include "IPFilter.h"
#include "SharedFileList.h"
#include "PartFile.h"
#include "SafeFile.h"
#include "Packets.h"
#include "UploadQueue.h"
#include "ServerList.h"
#include "Server.h"
#include "Sockets.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "Exceptions.h"
#include "Kademlia/Utils/uint128.h"
#include "Kademlia/Kademlia/kademlia.h"
#include "Kademlia/Kademlia/prefs.h"
#include "ClientUDPSocket.h"
#include "SHAHashSet.h"
#include "Log.h"
//Xman
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
#include "Defaults.h" // X: [POFC] - [PauseOnFileComplete]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CClientReqSocket

//IMPLEMENT_DYNCREATE(CClientReqSocket, CEMSocket)

CClientReqSocket::CClientReqSocket(CUpDownClient* in_client):
	deletethis(false),
	deltimer(0),
	m_bPortTestCon(false),
	m_nOnConnect(SS_Other)
{
	SetClient(in_client);
	/*
	theApp.listensocket->AddSocket(this);
	*/	//Enig123
	ResetTimeOutTimer();
	theApp.listensocket->AddSocket(this);	//Enig123
}

void CClientReqSocket::SetConState( SocketState val )
{
	//If no change, do nothing..
	if( (UINT)val == m_nOnConnect )
		return;
	//Decrease count of old state..
	switch( m_nOnConnect )
	{
		case SS_Half:
			theApp.listensocket->m_nHalfOpen--;
			break;
		case SS_Complete:
			theApp.listensocket->m_nComp--;
	}
	//Set state to new state..
	m_nOnConnect = val;
	//Increase count of new state..
	switch( m_nOnConnect )
	{
		case SS_Half:
			theApp.listensocket->m_nHalfOpen++;
			break;
		case SS_Complete:
			theApp.listensocket->m_nComp++;
	}
}

void CClientReqSocket::WaitForOnConnect()
{
	SetConState(SS_Half);
}
	
CClientReqSocket::~CClientReqSocket()
{
	//This will update our statistics.
	SetConState(SS_Other);
	//Xman
	// Maella -Code Fix-
	if(client != NULL && client->socket == this){
		client->socket = NULL;
	}
	client = NULL;
	// Maella End
	//theApp.listensocket->RemoveSocket(this);//Enig123::code improvements
	DEBUG_ONLY (theApp.clientlist->Debug_SocketDeleted(this));
}

void CClientReqSocket::SetClient(CUpDownClient* pClient)
{
	client = pClient;
	if (client)
		client->socket = this;
}

void CClientReqSocket::ResetTimeOutTimer(){
	timeout_timer = ::GetTickCount();
}

UINT CClientReqSocket::GetTimeOut()
{
	//Xman / Maella:
	// Normally a TCP session is closed after 40s of inactivity 
	// To limit the number of simultaneous TCP session we use an adaptive timeout
	if((client != NULL) &&
		(client->GetUploadState() == US_ONUPLOADQUEUE || client->GetUploadState() == US_NONE) &&
		(client->GetDownloadState() == DS_ONQUEUE     || client->GetDownloadState() == DS_NONE)){
			// Smaller timeout (30s)
			// => help to reduce up to 25% the number of simultaneous connections
			return SEC2MS(30);
	}
	//Xman Xtreme Upload
	else if(client != NULL && //&& client->m_pPCDownSocket==NULL && client->m_pPCUpSocket==NULL && //Xman: don't touch peercache
		client->GetUploadState()== US_CONNECTING)
		return SEC2MS(20); 
	//Xman end
	else
		return CEMSocket::GetTimeOut();
}

bool CClientReqSocket::CheckTimeOut()
{
	if(m_nOnConnect == SS_Half)
	{
		//This socket is still in a half connection state.. Because of SP2, we don't know
		//if this socket is actually failing, or if this socket is just queued in SP2's new
		//protection queue. Therefore we give the socket a chance to either finally report
		//the connection error, or finally make it through SP2's new queued socket system..
		if (::GetTickCount() - timeout_timer > SEC2MS(90)){//CEMSocket::GetTimeOut()*4){ //Xman : 90 seconds should be enough
			timeout_timer = ::GetTickCount();
			CString str;
			str.Format(_T("Timeout: State:%u = SS_Half"), m_nOnConnect);
			//Xman : at a test I found out a second retry brings nothing at this state
			if(client) client->m_cFailed=5;
			//Xman end
			Disconnect(str,CUpDownClient::USR_SOCKET); // Maella -Upload Stop Reason-
			return true;
		}
		return false;
	}
	UINT uTimeout = GetTimeOut();
	if(client)
	{
		if (client->GetKadState() == KS_CONNECTED_BUDDY)
			uTimeout += MIN2MS(15);
/*		if (client->m_nChatstate!=MS_NONE)
			//We extend the timeout time here to avoid people chatting from disconnecting to fast.
			uTimeout += CONNECTION_TIMEOUT;*/
	}
	if (::GetTickCount() - timeout_timer > uTimeout){
		timeout_timer = ::GetTickCount();
		CString str;
		str.Format(_T("Timeout: State:%u (0 = SS_Other, 1 = SS_Half, 2 = SS_Complete"), m_nOnConnect);
		Disconnect(str, CUpDownClient::USR_SOCKET); // Maella -Upload Stop Reason-
		return true;
	}
	
	//there are some clients which are uploading to us.. and also get an uploadslot from us..
	//but they don't send a blockrequest. the socket won't timeout and we have a 0-uploadsocket
	if(client && client->GetUploadState()==US_UPLOADING && isready==false && client->GetUpStartTimeDelay()>=MIN2MS(1))
	{
		//timeout_timer = ::GetTickCount();
		theApp.uploadqueue->RemoveFromUploadQueue(client, _T("0-Uploadsocket"),CUpDownClient::USR_SOCKET); // Maella -Upload Stop Reason-
	}
	//Xman end
	return false;
}

void CClientReqSocket::OnClose(int nErrorCode){
	ASSERT (theApp.listensocket->IsValidSocket(this));

	CEMSocket::OnClose(nErrorCode);

	LPCTSTR pszReason;
	CString* pstrReason = NULL;
	if (nErrorCode == 0)
		pszReason = _T("Close");
	else if (thePrefs.GetVerbose()){
		pstrReason = new CString;
		*pstrReason = GetErrorMessage(nErrorCode, 1);
		pszReason = *pstrReason;
	}
	else
		pszReason = NULL;
	Disconnect(pszReason, CUpDownClient::USR_SOCKET);  // Maella -Upload Stop Reason-
	delete pstrReason;

}

//Xman improved socket closing
void CClientReqSocket::CloseSocket()
{
	CEMSocket::OnClose(0);
	Close();
	deltimer = ::GetTickCount();
}
//Xman end

void CClientReqSocket::Disconnect(LPCTSTR pszReason, CUpDownClient::UpStopReason reason){ // Maella -Upload Stop Reason-

	AsyncSelect(0);

	//Xman 4.8.2
	//Threadsafe Statechange
	SetConnectedState(ES_DISCONNECTED);
	//byConnected = ES_DISCONNECTED;
	//Xman end

	if (client){
		if(client->Disconnected(CString(_T("CClientReqSocket::Disconnect(): ")) + pszReason, true, reason)){ // Maella -Upload Stop Reason-
			CUpDownClient* temp = client;
			client->socket = NULL;
			client = NULL;
			delete temp;
		}
		else
			client = NULL;
		}
	Safe_Delete();
};

/*
void CClientReqSocket::Delete_Timed(){
*/	//Enig123::code improvements
bool CClientReqSocket::Delete_Timed() const {
// it seems that MFC Sockets call socketfunctions after they are deleted, even if the socket is closed
// and select(0) is set. So we need to wait some time to make sure this doesn't happens
	/*
	if (::GetTickCount() - deltimer > 14000) //Xman changed from 10 to 14 seconds = ~ 15 sec
		delete this;
	*/

	//Xman improved socket closing
	//other than official this isn't anymore the time between shutting down and deletion
	//but the time from closing and removing from uploadbandwidththrottler to deletion
	//because we had already some waiting-time we can reduce this one (maybe we don't need it anymore)
	/*
	if (::GetTickCount() - deltimer > 10000) //for the moment 10 seconds for safety
		delete this;
	*/
	//zz_fly :: it is confirm that this fix is necessary to very big uploaders(upload>3000). normal user do not need this fix.
	//			a simple solution is increase the time here. but for unknown reason, Xman had dropped it.
	//			althought Xman's fix use more memory(about 10~20MB), but i restore it.
	return (::GetTickCount() - deltimer > 10000);
	//Xman end
}

//Xman
// - Maella -Code Fix-
void CClientReqSocket::Safe_Delete()
{
	ASSERT (theApp.listensocket->IsValidSocket(this));

	AsyncSelect(0);

	deltimer = ::GetTickCount();
	if (m_SocketData.hSocket != INVALID_SOCKET) // deadlake PROXYSUPPORT - changed to AsyncSocketEx
		ShutDown(SD_BOTH);
	if(client != NULL && client->socket == this){
		// They might have an error in the cross link somewhere
		client->socket = NULL;
	}
	client = NULL;
	//Xman 4.8.2
	//Threadsafe Statechange
	SetConnectedState(ES_DISCONNECTED);
	//byConnected = ES_DISCONNECTED;
	//Xman end
	deletethis = true;
}
// Maella end

bool CClientReqSocket::ProcessPacket(const BYTE* packet, uint32 size, UINT opcode)
{
	try
	{
			if (!client && opcode != OP_HELLO)
			{
				theStats.AddDownDataOverheadOther(size);
				throw GetResString(IDS_ERR_NOHELLO);
			}
			else if (client && opcode != OP_HELLO && opcode != OP_HELLOANSWER)
				client->CheckHandshakeFinished();
			switch(opcode)
			{
				case OP_HELLOANSWER:
				{
					theStats.AddDownDataOverheadOther(size);
					client->ProcessHelloAnswer(packet,size);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_HelloAnswer", client);
						Debug(_T("  %s\n"), client->DbgGetHelloInfo());
					}
#endif

					// start secure identification, if
					//  - we have received OP_EMULEINFO and OP_HELLOANSWER (old eMule)
					//	- we have received eMule-OP_HELLOANSWER (new eMule)
					if (client->GetInfoPacketsReceived() == IP_BOTH)
						client->InfoPacketsReceived();

					//Xman don't continue sending after banning
					if(client && client->GetUploadState()==US_BANNED)
						break;
					//Xman end

					if (client)
					{
						client->ConnectionEstablished();
					}
					break;
				}
				case OP_HELLO:
				{
					theStats.AddDownDataOverheadOther(size);
					//Xman
					// Maella -Code Fix-
					if(client != NULL)
					{
						CString oldHash;
						CString newHash;
						if(size > 17){
							oldHash = (client->GetUserHash() == NULL) ? _T("null") : EncodeBase16(client->GetUserHash(), 16);
							newHash = EncodeBase16((const uchar*)&packet[1], 16);
						}
						if(oldHash != newHash){
							AddDebugLogLine(true,  _T("User %s (client=%s) try to send multiple OP_HELLO, old hash=%s, new has=%s"), client->GetUserName(), client->GetClientSoftVer(), oldHash, newHash);
						}
						else {
							AddDebugLogLine(true, _T("User %s (client=%s) try to send multiple OP_HELLO"), client->GetUserName(), client->GetClientSoftVer());
						}
						throw CString(_T("Invalid request received"));
					}
					// Maella end
					const bool bNewClient = (client == NULL);
					if (bNewClient)
					{
						// create new client to save standart informations
						client = new CUpDownClient(this);
					}

					bool bIsMuleHello = false;
					try
					{
						bIsMuleHello = client->ProcessHelloPacket(packet,size);
					}
					catch(...)
					{
						if (bNewClient)
						{
							// Don't let CUpDownClient::Disconnected be processed for a client which is not in the list of clients.
							delete client;
							client = NULL;
						}
						throw;
					}

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_Hello", client);
						Debug(_T("  %s\n"), client->DbgGetHelloInfo());
					}
#endif

					// now we check if we know this client already. if yes this socket will
					// be attached to the known client, the new client will be deleted
					// and the var. "client" will point to the known client.
					// if not we keep our new-constructed client ;)
					if (theApp.clientlist->AttachToAlreadyKnown(&client,this))
					{
						// update the old client informations
						bIsMuleHello = client->ProcessHelloPacket(packet,size);
					}
					else 
					{
						theApp.clientlist->AddClient(client, bNewClient ); //Xman Code Improvement don't search new generated clients in lists
					}

					//Xman don't continue sending after banning
					if(client && client->GetUploadState()==US_BANNED)
						break;
					//Xman end

					// send a response packet with standart informations
					if (client->GetHashType() == SO_EMULE && !bIsMuleHello)
						client->SendMuleInfoPacket(false);
					client->SendHelloAnswer();
					if (client)
						client->ConnectionEstablished();

					ASSERT( client );
					if(client)
					{
						// start secure identification, if
						//	- we have received eMule-OP_HELLO (new eMule)
						if (client->GetInfoPacketsReceived() == IP_BOTH)
							client->InfoPacketsReceived();

						if( client->GetKadPort() && client->GetKadVersion() > 1)
							Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort());
					}
					break;
				}
				case OP_REQUESTFILENAME:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileRequest", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					// Spike2 from eMule+ - START
					//	IP banned, no answer for this request
					if(client && client->GetUploadState()==US_BANNED)
						break;
					// Spike2 from eMule+ - END

					if (size >= 16)
					{
						if (!client->GetWaitStartTime())
							client->SetWaitStartTime();

						CSafeMemFile data_in(packet, size);
						uchar reqfilehash[16];
						data_in.ReadHash16(reqfilehash);

						CKnownFile* reqfile;
						if ( (reqfile = theApp.sharedfiles->GetFileByID(reqfilehash)) == NULL ){
							if ( !((reqfile = theApp.downloadqueue->GetFileByID(reqfilehash)) != NULL
								&& reqfile->GetFileSize() > (uint64)PARTSIZE) )
							{
//>>> WiZaRd::Missing code?
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugSend("OP__FileReqAnsNoFil", client, packet);
#endif
								Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
								md4cpy(replypacket->pBuffer, reqfilehash);
								theStats.AddUpDataOverheadFileRequest(replypacket->size);
								SendPacket(replypacket, true);
//<<< WiZaRd::Missing code?
								client->CheckFailedFileIdReqs(reqfilehash);
								break;
							}
						}

						if (reqfile->IsLargeFile() && !client->SupportsLargeFiles()){
//>>> WiZaRd::Missing code?
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__FileReqAnsNoFil", client, packet);
#endif
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->pBuffer, reqfile->GetFileHash());
							theStats.AddUpDataOverheadFileRequest(replypacket->size);
							SendPacket(replypacket, true);
//<<< WiZaRd::Missing code?
							DebugLogWarning(_T("Client without 64bit file support requested large file; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
							break;
						}

						client->SetUploadFileID(reqfile);
						client->SetLastAction(OP_REQUESTFILENAME);	//Xman fix for startupload

						if (!client->ProcessExtendedInfo(&data_in, reqfile)){
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__FileReqAnsNoFil", client, packet);
#endif
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->pBuffer, reqfile->GetFileHash());
							theStats.AddUpDataOverheadFileRequest(replypacket->size);
							SendPacket(replypacket, true);
							if(thePrefs.GetLogPartmismatch()) //Xman Log part/size-mismatch
								DebugLogWarning(_T("Partcount mismatch on requested file, sending FNF; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
							break;
						}

						// if we are downloading this file, this could be a new source
						// no passive adding of files with only one part
						if (reqfile->IsPartFile() && reqfile->GetFileSize() > (uint64)PARTSIZE)
						{
							CPartFile* partfile=(CPartFile*)reqfile;
							if (partfile->GetMaxSources() > partfile->GetSourceCount()) 
								theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client, true);
						}


						// send filename etc
						CSafeMemFile data_out(128);
						data_out.WriteHash16(reqfile->GetFileHash());
						data_out.WriteString(reqfile->GetFileName(), client->GetUnicodeSupport());
						Packet* packet = new Packet(&data_out, OP_EDONKEYPROT, OP_REQFILENAMEANSWER);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileReqAnswer", client, reqfile->GetFileHash());
#endif
						theStats.AddUpDataOverheadFileRequest(packet->size);
						SendPacket(packet, true);
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
				case OP_SETREQFILEID:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_SetReqFileID", client,  (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);

					if (size == 16)
					{
						if (!client->GetWaitStartTime())
							client->SetWaitStartTime();
						CKnownFile* reqfile;
						if ( (reqfile = theApp.sharedfiles->GetFileByID(packet)) == NULL ){
							if ( !((reqfile = theApp.downloadqueue->GetFileByID(packet)) != NULL
								&& reqfile->GetFileSize() > (uint64)PARTSIZE) )
							{
								// send file request no such file packet (0x48)
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugSend("OP__FileReqAnsNoFil", client, packet);
#endif
								Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
								md4cpy(replypacket->pBuffer, packet);
								theStats.AddUpDataOverheadFileRequest(replypacket->size);
								SendPacket(replypacket, true);
								client->CheckFailedFileIdReqs(packet);
								break;
							}
						}
						if (reqfile->IsLargeFile() && !client->SupportsLargeFiles()){
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__FileReqAnsNoFil", client, packet);
#endif
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->pBuffer, packet);
							theStats.AddUpDataOverheadFileRequest(replypacket->size);
							SendPacket(replypacket, true);
							DebugLogWarning(_T("Client without 64bit file support requested large file; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
							break;
						}

						client->SetUploadFileID(reqfile);
						client->SetLastAction(OP_SETREQFILEID);	//Xman fix for startupload
						// send filestatus
						CSafeMemFile data(16+16);
						data.WriteHash16(reqfile->GetFileHash());
		// morph4u :: SOTN :: Start
						reqfile->WriteSafePartStatus(&data, client);
/*
				if (reqfile->IsPartFile())
							((CPartFile*)reqfile)->WritePartStatus(&data);
*/
// morph4u :: SOTN :: End
											Packet* packet = new Packet(&data, OP_EDONKEYPROT, OP_FILESTATUS);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileStatus", client, reqfile->GetFileHash());
#endif
						theStats.AddUpDataOverheadFileRequest(packet->size);
						SendPacket(packet, true);
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
				case OP_FILEREQANSNOFIL:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileReqAnsNoFil", client,  (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					if (size == 16)
					{

						CPartFile* reqfile = theApp.downloadqueue->GetFileByID(packet);
						if (!reqfile){
							client->CheckFailedFileIdReqs(packet);
							break;
						}
						else
							reqfile->m_DeadSourceList.AddDeadSource(client);

						//Xman Filefaker Detection		
						if(client->GetUploadState()!=US_NONE && reqfile->GetFileSize()>PARTSIZE)
						{
							CKnownFile* upfile = theApp.sharedfiles->GetFileByID(client->GetUploadFileID());
							if(upfile && upfile == reqfile) //we speak about the same file
							{
								AddDebugLogLine(false,_T("Dropped src: (%s) does not seem to have own reqfile!(TCP)"), DbgGetClientInfo()); 
								if(!theApp.uploadqueue->RemoveFromUploadQueue(client, _T("Src says he does not have the file he's dl'ing")))
									theApp.uploadqueue->RemoveFromWaitingQueue(client);
								else
									ASSERT(!theApp.uploadqueue->RemoveFromWaitingQueue(client));
							}
						}
						//Xman end

						// if that client does not have my file maybe has another different
						// we try to swap to another file ignoring no needed parts files
						if (client->GetRequestFile()==reqfile) //Xman just to be sure
						switch (client->GetDownloadState())
						{
							case DS_CONNECTED:
							case DS_ONQUEUE:
							case DS_NONEEDEDPARTS:
                                //Xman Xtreme Downloadmanager
								if (!client->SwapToAnotherFile(true, true, true, NULL)) 
								{ 
    								theApp.downloadqueue->RemoveSource(client);
                                }
								//Xman end
							break;
						}
						break;
					}
					throw GetResString(IDS_ERR_WRONGPACKAGESIZE);
					break;
				}
				case OP_REQFILENAMEANSWER:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileReqAnswer", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet,size);
					uchar cfilehash[16];
					data.ReadHash16(cfilehash);

					CPartFile* file = theApp.downloadqueue->GetFileByID(cfilehash);
					if (file == NULL)
						client->CheckFailedFileIdReqs(cfilehash);
					client->ProcessFileInfo(&data, file);
					break;
				}
				case OP_FILESTATUS:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FileStatus", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					CSafeMemFile data(packet,size);
					uchar cfilehash[16];
					data.ReadHash16(cfilehash);
					CPartFile* file = theApp.downloadqueue->GetFileByID(cfilehash);
					if (file == NULL)
						//Xman Code Fix
						//following situation: we are downloading a cue file (100 bytes) from many sources
						//this file will be finished earlier than our sources are answering
						//throwing an exception will filter good sources
						//added by zz_fly. this situation can also happen here. thanks Enig123
					{
						CKnownFile* reqfiletocheck = theApp.sharedfiles->GetFileByID(cfilehash);
						if(reqfiletocheck!=NULL){ 
							AddDebugLogLine(false, _T("client send NULL FileStatus: %s"), client->DbgGetClientInfo());
							break;
						}
						else
						//Xman end
						client->CheckFailedFileIdReqs(cfilehash);
					} //Xman
					//zz_fly :: test code
					//note: the easyMule client may send unexpect OP_FILESTATUS packet. from its src, it will send OP_FILESTATUS when CPartFile::FlushBuffer().
					//		ignore those unexpect packets. thanks Enig123
					if(client->GetDownloadState() == DS_DOWNLOADING && wcsistr(client->GetClientModVer(), L"easyMule"))
						AddDebugLogLine(false, _T("easyMule client send unexpect OP_FILESTATUS packet: %s"), client->DbgGetClientInfo());
					else
					//zz_fly :: test code
					client->ProcessFileStatus(false, &data, file);
					break;
				}
				case OP_STARTUPLOADREQ:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_StartUpLoadReq", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					if (!client->CheckHandshakeFinished())
						break;
					if (size == 16)
					{
	                   CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(packet);
						if (reqfile)
						{
							client->SetUploadFileID(reqfile);
							client->SetLastAction(OP_STARTUPLOADREQ);	//Xman fix for startupload
//>>> WiZaRd - in any case, we add this client to our downloadqueue
//this has the advantage that *every* who wants something from us will be a src for us
							if (reqfile->IsPartFile() && reqfile->GetFileSize() > (uint64)PARTSIZE)
							{
								if (((CPartFile*)reqfile)->GetMaxSourcePerFileSoft() > ((CPartFile*)reqfile)->GetSourceCount()) // Spike2 - hold the HardLimit !!
									theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client); // Spike2 - removed option to disable DeadSourceList
							}
//<<< WiZaRd - in any case, we add this client to our downloadqueue
							theApp.uploadqueue->AddClientToQueue(client);
						}
						else
							client->CheckFailedFileIdReqs(packet);
						break;
					}
					break;
				}
				case OP_QUEUERANK:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_QueueRank", client);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessEdonkeyQueueRank(packet, size);
					break;
				}
				case OP_ACCEPTUPLOADREQ:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_AcceptUploadReq", client, (size >= 16) ? packet : NULL);
						if (size > 0)
							Debug(_T("  ***NOTE: Packet contains %u additional bytes\n"), size);
						Debug(_T("  QR=%d\n"), client->IsRemoteQueueFull() ? (UINT)-1 : (UINT)client->GetRemoteQueueRank());
					}
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessAcceptUpload();
					break;
				}
				case OP_REQUESTPARTS:
				{
					// see also OP_REQUESTPARTS_I64
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestParts", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet,size);
					uchar reqfilehash[16];
					data.ReadHash16(reqfilehash);

					uint32 auStartOffsets[3];
					auStartOffsets[0] = data.ReadUInt32();
					auStartOffsets[1] = data.ReadUInt32();
					auStartOffsets[2] = data.ReadUInt32();

					uint32 auEndOffsets[3];
					auEndOffsets[0] = data.ReadUInt32();
					auEndOffsets[1] = data.ReadUInt32();
					auEndOffsets[2] = data.ReadUInt32();

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						Debug(_T("  Start1=%u  End1=%u  Size=%u\n"), auStartOffsets[0], auEndOffsets[0], auEndOffsets[0] - auStartOffsets[0]);
						Debug(_T("  Start2=%u  End2=%u  Size=%u\n"), auStartOffsets[1], auEndOffsets[1], auEndOffsets[1] - auStartOffsets[1]);
						Debug(_T("  Start3=%u  End3=%u  Size=%u\n"), auStartOffsets[2], auEndOffsets[2], auEndOffsets[2] - auStartOffsets[2]);
					}
#endif

					if(client->CloseBackdoor(reqfilehash))//Xman Close Backdoor v2
						break;

					for (size_t i = 0; i < ARRSIZE(auStartOffsets); i++)
					{
						if (auEndOffsets[i] > auStartOffsets[i])
						{
							Requested_Block_Struct* reqblock = new Requested_Block_Struct;
							reqblock->StartOffset = auStartOffsets[i];
							reqblock->EndOffset = auEndOffsets[i];
							md4cpy(reqblock->FileID, reqfilehash);
							reqblock->transferred = 0;
							client->AddReqBlock(reqblock);
						}
						else
						{
							if (thePrefs.GetVerbose())
							{
								if (auEndOffsets[i] != 0 || auStartOffsets[i] != 0)
									DebugLogWarning(_T("Client requests invalid %u. file block %u-%u (%d bytes): %s"), i, auStartOffsets[i], auEndOffsets[i], auEndOffsets[i] - auStartOffsets[i], client->DbgGetClientInfo());
							}
						}
					}
					//Xman for SiRoB: ReadBlockFromFileThread
					if(client->GetUploadState() == US_UPLOADING)
						client->CreateNextBlockPackage();
					//Xman end
					break;
				}
				case OP_CANCELTRANSFER:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_CancelTransfer", client);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					//Xman Code Improvement
					if (theApp.uploadqueue->RemoveFromUploadQueue(client, _T("Remote client cancelled transfer."), CUpDownClient::USR_CANCELLED)){ // Maella -Upload Stop Reason-
						client->SetUploadFileID(NULL);
					}
					break;
				}
				case OP_END_OF_DOWNLOAD:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_EndOfDownload", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					if (size>=16 && !md4cmp(client->GetUploadFileID(),packet))
					{
						//Xman Code Improvement
						if (theApp.uploadqueue->RemoveFromUploadQueue(client, _T("Remote client ended transfer."), CUpDownClient::USR_CANCELLED)){ // Maella -Upload Stop Reason-
							client->SetUploadFileID(NULL);
						}
					}
					else
						client->CheckFailedFileIdReqs(packet);
					break;
				}
				case OP_HASHSETREQUEST:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetReq", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);

					if (size != 16)
						throw GetResString(IDS_ERR_WRONGHPACKAGESIZE);
					client->SendHashsetPacket(packet, 16, false);
					break;
				}
				case OP_HASHSETANSWER:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetAnswer", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessHashSet(packet, size, false);
					break;
				}
				case OP_SENDINGPART:
				{
					// see also OP_SENDINGPART_I64
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 1)
						DebugRecv("OP_SendingPart", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(24);
					if (client->GetRequestFile() && !client->GetRequestFile()->IsStopped() && (client->GetRequestFile()->GetStatus()==PS_READY || client->GetRequestFile()->GetStatus()==PS_EMPTY))
					{
						client->ProcessBlockPacket(packet, size, false, false);
						if (client->GetRequestFile())
						{
						if (client->GetRequestFile()->IsStopped() || client->GetRequestFile()->GetStatus()==PS_PAUSED || client->GetRequestFile()->GetStatus()==PS_ERROR)
						{
							client->SendCancelTransfer();
							client->SetDownloadState(client->GetRequestFile()->IsStopped() ? DS_NONE : DS_ONQUEUE,_T("paused file"), CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
						}
					}
					else
							ASSERT( false );
					}
					else
					{
						client->SendCancelTransfer();
						client->SetDownloadState((client->GetRequestFile()==NULL || client->GetRequestFile()->IsStopped()) ? DS_NONE : DS_ONQUEUE,_T("paused file"), CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
					}
					break;
				}
				case OP_OUTOFPARTREQS:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_OutOfPartReqs", client);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					if (client->GetDownloadState() == DS_DOWNLOADING)
					{
						// X-Ray :: FiXeS :: Statfix :: Start :: WiZaRd
						// this prevents that we count the session as failed it 
						// would be wrong to do that - because the other guy is the bad one :P						
						client->SetTransferredDownMini();
						// X-Ray :: FiXeS :: Statfix :: End :: WiZaRd
						client->SetDownloadState(DS_ONQUEUE, _T("The remote client decided to stop/complete the transfer (got OP_OutOfPartReqs)."), CUpDownClient::DSR_OUTOFPART); // Maella -Download Stop Reason-
					}
					break;
				}
				case OP_CHANGE_CLIENT_ID:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_ChangedClientID", client);
#endif
					theStats.AddDownDataOverheadOther(size);
					CSafeMemFile data(packet, size);
					uint32 nNewUserID = data.ReadUInt32();
					uint32 nNewServerIP = data.ReadUInt32();
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						Debug(_T("  NewUserID=%u (%08x, %s)  NewServerIP=%u (%08x, %s)\n"), nNewUserID, nNewUserID, ipstr(nNewUserID), nNewServerIP, nNewServerIP, ipstr(nNewServerIP));
#endif
					if (IsLowID(nNewUserID))
					{	// client changed server and has a LowID
						CServer* pNewServer = theApp.serverlist->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
							client->SetUserIDHybrid(nNewUserID); // update UserID only if we know the server
							client->SetServerIP(nNewServerIP);
							client->SetServerPort(pNewServer->GetPort());
						}
					}
					else if (nNewUserID == client->GetIP())
					{	// client changed server and has a HighID(IP)
						client->SetUserIDHybrid(ntohl(nNewUserID));
						CServer* pNewServer = theApp.serverlist->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
							client->SetServerIP(nNewServerIP);
							client->SetServerPort(pNewServer->GetPort());
						}
					}
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					else{
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							Debug(_T("***NOTE: OP_ChangedClientID unknown contents\n"));
					}
					UINT uAddData = (UINT)(data.GetLength() - data.GetPosition());
					if (uAddData > 0){
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							Debug(_T("***NOTE: OP_ChangedClientID contains add. data %s\n"), DbgGetHexDump(packet + data.GetPosition(), uAddData));
					}
#endif
					break;
				}
				case OP_MESSAGE:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Message", client);
#endif
					theStats.AddDownDataOverheadOther(size);
					
					if (size < 2)
						throw CString(_T("invalid message packet"));
					CSafeMemFile data(packet, size);
					UINT length = data.ReadUInt16();
					if (length+2 != size)
						throw CString(_T("invalid message packet"));
					{
						CSafeMemFile data;
						data.WriteString(_T(":-[ This client doesn't support message."), client->GetUnicodeSupport());
						Packet* packet = new Packet(&data, OP_EDONKEYPROT, OP_MESSAGE);
						theStats.AddUpDataOverheadOther(packet->size);
						client->SafeConnectAndSendPacket(packet);
					}
					break;
				}
				case OP_ASKSHAREDFILES:
				{	
					// client wants to know what we have in share, let's see if we allow him to know that
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFiles", client);
#endif
					theStats.AddDownDataOverheadOther(size);


					CAtlList<CKnownFile*> list;
					if (thePrefs.CanSeeShares()==vsfaEverybody)
					{
#ifdef REPLACE_ATLMAP
						for (CKnownFilesMap::const_iterator it = theApp.sharedfiles->m_Files_map.begin(); it != theApp.sharedfiles->m_Files_map.end(); ++it)
						{
							CKnownFile* cur_file = it->second;
#else
						CCKey bufKey;
						CKnownFile* cur_file;
						for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition();pos != 0;)
						{
							theApp.sharedfiles->m_Files_map.GetNextAssoc(pos,bufKey,cur_file);
#endif
							if (!cur_file->IsLargeFile() || client->SupportsLargeFiles())
								list.AddTail(cur_file);
						}
						AddLogLine(true, GetResString(IDS_REQ_SHAREDFILES), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_ACCEPTED));
					}
					else
					{
						//Xman show his IP 
						CString buffer(CString(client->GetUserName()) + _T(" [") + client->GetUserIPString() + _T(']'));
						DebugLog(GetResString(IDS_REQ_SHAREDFILES), buffer, client->GetUserIDHybrid(), GetResString(IDS_DENIED));
					}
                    
					// now create the memfile for the packet
					UINT iTotalCount = (UINT)list.GetCount();
					CSafeMemFile tempfile(80);
					tempfile.WriteUInt32(iTotalCount);
					while (list.GetCount())
					{
						theApp.sharedfiles->CreateOfferedFilePacket((CKnownFile*)list.RemoveHead(), &tempfile, NULL, client);// X: [CI] - [Code Improvement]
					}

					// create a packet and send it
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__AskSharedFilesAnswer", client);
#endif
					Packet* replypacket = new Packet(&tempfile, OP_EDONKEYPROT, OP_ASKSHAREDFILESANSWER);
					theStats.AddUpDataOverheadOther(replypacket->size);
					SendPacket(replypacket, true, true);
					break;
				}
				case OP_ASKSHAREDFILESANSWER:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesAnswer", client);
#endif
					theStats.AddDownDataOverheadOther(size);
					client->ProcessSharedFileList(packet,size);
					break;
				}
                case OP_ASKSHAREDDIRS:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDirectories", client);
#endif
					theStats.AddDownDataOverheadOther(size);

                    if (thePrefs.CanSeeShares()==vsfaEverybody)
					{
						AddLogLine(true, GetResString(IDS_SHAREDREQ1), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_ACCEPTED));
						client->SendSharedDirectories();
					}
					else
					{
						DebugLog(GetResString(IDS_SHAREDREQ1), client->GetUserName(), client->GetUserIDHybrid(), GetResString(IDS_DENIED));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDeniedAnswer", client);
#endif
						Packet* replypacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);
						theStats.AddUpDataOverheadOther(replypacket->size);
                        SendPacket(replypacket, true, true);
                    }
                    break;
                }
                case OP_ASKSHAREDFILESDIR:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesInDirectory", client);
#endif
					theStats.AddDownDataOverheadOther(size);

                    CSafeMemFile data(packet, size);
                    CString strReqDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
					CString strOrgReqDir = strReqDir;
                    if (thePrefs.CanSeeShares()==vsfaEverybody)
					{
						AddLogLine(true, GetResString(IDS_SHAREDREQ2), client->GetUserName(), client->GetUserIDHybrid(), strReqDir, GetResString(IDS_ACCEPTED));
                        ASSERT( data.GetPosition() == data.GetLength() );
                        CAtlList<CKnownFile*> list;
						if (strReqDir == OP_INCOMPLETE_SHARED_FILES)
						{
							// get all shared files from download queue
							for (POSITION pos = theApp.downloadqueue->filelist.GetHeadPosition();pos != 0;){// X: [CI] - [Code Improvement]
								CPartFile* pFile = theApp.downloadqueue->filelist.GetNext(pos);
								if (pFile->GetStatus(true) != PS_READY || (pFile->IsLargeFile() && !client->SupportsLargeFiles()))
									continue;
								list.AddTail(pFile);
							}
						}
						else
						{
							bool bSingleSharedFiles = strReqDir == OP_OTHER_SHARED_FILES;
							if (!bSingleSharedFiles)
								strReqDir = theApp.sharedfiles->GetDirNameByPseudo(strReqDir);
							if (!strReqDir.IsEmpty())
							{
								// get all shared files from requested directory
#ifdef REPLACE_ATLMAP
								for (CKnownFilesMap::const_iterator it = theApp.sharedfiles->m_Files_map.begin(); it != theApp.sharedfiles->m_Files_map.end(); ++it)
								{
									CKnownFile* cur_file = it->second;
#else
								for (POSITION pos = theApp.sharedfiles->m_Files_map.GetStartPosition();pos != 0;)
								{
									CCKey bufKey;
									CKnownFile* cur_file;
									theApp.sharedfiles->m_Files_map.GetNextAssoc(pos, bufKey, cur_file);
#endif

								// all files which are not within a shared directory have to be single shared files
									if (((!bSingleSharedFiles && CompareDirectories(strReqDir, cur_file->GetSharedDirectory()) == 0) || (bSingleSharedFiles && !theApp.sharedfiles->ShouldBeShared(cur_file->GetSharedDirectory(), _T(""), false)))
										&& (!cur_file->IsLargeFile() || client->SupportsLargeFiles()))
									{
										list.AddTail(cur_file);
									}

								}
							}
							else
								DebugLogError(_T("View shared files: Pseudonym for requested Directory (%s) was not found - sending empty result"), strOrgReqDir);	
						}

						// Currently we are sending each shared directory, even if it does not contain any files.
						// Because of this we also have to send an empty shared files list..
						CSafeMemFile tempfile(80);
						tempfile.WriteString(strOrgReqDir, client->GetUnicodeSupport());
						tempfile.WriteUInt32((uint32)list.GetCount());
						while (list.GetCount())
						{
							theApp.sharedfiles->CreateOfferedFilePacket(list.RemoveHead(), &tempfile, NULL, client);// X: [CI] - [Code Improvement]
						}

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedFilesInDirectoryAnswer", client);
#endif
						Packet* replypacket = new Packet(&tempfile, OP_EDONKEYPROT, OP_ASKSHAREDFILESDIRANS);
						theStats.AddUpDataOverheadOther(replypacket->size);
						SendPacket(replypacket, true, true);
					}
                    else
					{
						DebugLog(GetResString(IDS_SHAREDREQ2), client->GetUserName(), client->GetUserIDHybrid(), strReqDir, GetResString(IDS_DENIED));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AskSharedDeniedAnswer", client);
#endif
						Packet* replypacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);
                        theStats.AddUpDataOverheadOther(replypacket->size);
                        SendPacket(replypacket, true, true);
                    }
                    break;
                }
				case OP_ASKSHAREDDIRSANS:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDirectoriesAnswer", client);
#endif
					theStats.AddDownDataOverheadOther(size);
                    if (client->GetFileListRequested() == 1)
					{
                        CSafeMemFile data(packet, size);
						UINT uDirs = data.ReadUInt32();
                        for (UINT i = 0; i < uDirs; i++)
						{
                            CString strDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
							// Better send the received and untouched directory string back to that client
							//PathRemoveBackslash(strDir.GetBuffer());
							//strDir.ReleaseBuffer();
							AddLogLine(true, GetResString(IDS_SHAREDANSW), client->GetUserName(), client->GetUserIDHybrid(), strDir);

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugSend("OP__AskSharedFilesInDirectory", client);
#endif
							CSafeMemFile tempfile(80);
							tempfile.WriteString(strDir, client->GetUnicodeSupport());
							Packet* replypacket = new Packet(&tempfile, OP_EDONKEYPROT, OP_ASKSHAREDFILESDIR);
                            theStats.AddUpDataOverheadOther(replypacket->size);
                            SendPacket(replypacket, true, true);
                        }
                        ASSERT( data.GetPosition() == data.GetLength() );
                        client->SetFileListRequested(uDirs);
                    }
					else
						AddLogLine(true, GetResString(IDS_SHAREDANSW2), client->GetUserName(), client->GetUserIDHybrid());
                    break;
                }
                case OP_ASKSHAREDFILESDIRANS:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedFilesInDirectoryAnswer", client);
#endif
					theStats.AddDownDataOverheadOther(size);

                    CSafeMemFile data(packet, size);
                    CString strDir = data.ReadString(client->GetUnicodeSupport()!=utf8strNone);
					PathRemoveBackslash(strDir.GetBuffer());
					strDir.ReleaseBuffer();
                    if (client->GetFileListRequested() > 0)
					{
						AddLogLine(true, GetResString(IDS_SHAREDINFO1), client->GetUserName(), client->GetUserIDHybrid(), strDir);
						client->ProcessSharedFileList(packet + (UINT)data.GetPosition(), (UINT)(size - data.GetPosition()), strDir);
						if (client->GetFileListRequested() == 0)
							AddLogLine(true, GetResString(IDS_SHAREDINFO2), client->GetUserName(), client->GetUserIDHybrid());
                    }
					else
						AddLogLine(true, GetResString(IDS_SHAREDANSW3), client->GetUserName(), client->GetUserIDHybrid(), strDir);
                    break;
                }
                case OP_ASKSHAREDDENIEDANS:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AskSharedDeniedAnswer", client);
#endif
					theStats.AddDownDataOverheadOther(size);

					AddLogLine(true, GetResString(IDS_SHAREDREQDENIED), client->GetUserName(), client->GetUserIDHybrid());
					client->SetFileListRequested(0);
                    break;
                }
				default:
					theStats.AddDownDataOverheadOther(size);
					//Xman final version: don't log too much
#ifdef LOGTAG
					AddDebugLogLine(_T("Received unknown edonkey-Packet"));
					PacketToDebugLogLine(false, packet, size, opcode);
#endif
					break;
			}
	}
	/*
	catch(CClientException* ex) // nearly same as the 'CString' exception but with optional deleting of the client
	{
		if (thePrefs.GetVerbose() && !ex->m_strMsg.IsEmpty())
			DebugLogWarning(_T("Error: %s - while processing eDonkey packet: opcode=%s  size=%u; %s"), ex->m_strMsg, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client && ex->m_bDelete)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet (CClientException): ") + ex->m_strMsg);
		Disconnect(ex->m_strMsg);
		ex->Delete();
		return false;
	}
	*/
	//Xman Xtreme Mod Exception Handling 
	catch(CString error)
	{
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		GetPeerName((SOCKADDR*)&sockAddr,&nSockAddrLen);
		theApp.clientlist->AddBannedClient(ntohl(sockAddr.sin_addr.S_un.S_addr));
		if (client != NULL){
			if (thePrefs.GetVerbose() && !error.IsEmpty()){
					DebugLogWarning(_T("Error: %s - while processing eDonkey packet: opcode=%s  size=%u; %s"), error, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
					AddDebugLogLine(false, _T("Client %s with IP: %s has been added temporary to the banned list"), client->GetUserName(), client->GetFullIP());			
			}									
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet (CString exception): ") + error, CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
		}
		else {
			if (thePrefs.GetVerbose() && !error.IsEmpty()){
				AddDebugLogLine(false, _T("Client with IP=%s caused an error or did something bad: %s. Disconnecting client!"), ipstr(sockAddr.sin_addr), error.GetBuffer());
				AddDebugLogLine(false, _T("IP: %s has been added temporary to the banned list"), ipstr(sockAddr.sin_addr));			
			}
		}
		//PacketToDebugLogLine(true, packet, size, opcode, DLP_DEFAULT);	//Xman too many infos
		Disconnect(_T("processing eDonkey packet:  CString Exception ") + error, CUpDownClient::USR_EXCEPTION);  // Maella -Upload Stop Reason-
		return false;
	}
	catch(CFileException* pFileException){
		pFileException->Delete();
		if (thePrefs.GetVerbose())
		{
			AddDebugLogLine(true, _T("A client has caused an exception. Disconnecting client!"));		
			PacketToDebugLogLine(true, packet, size, opcode);
		}
		// Xman -Temporary Ban IP of clients causing an error-
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		GetPeerName((SOCKADDR*)&sockAddr,&nSockAddrLen);
		theApp.clientlist->AddBannedClient(ntohl(sockAddr.sin_addr.S_un.S_addr));

		if (client != NULL){ 			
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Client %s with IP: %s has been added temporary to the banned list"), client->GetUserName(), client->GetFullIP());			
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet. CFile Exception "), CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
		}
		else {
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("IP: %s has been added temporary to the banned list"), ipstr(sockAddr.sin_addr));			
		}		
		Disconnect(_T("processing eDonkey packet: client caused a FileException"),CUpDownClient::USR_EXCEPTION);  // Maella -Upload Stop Reason-
		return false;
	}
	catch(...){
		if (thePrefs.GetVerbose())
		{
			AddDebugLogLine(true, _T("A client has caused an undefined exception. Disconnecting client!"));		
			PacketToDebugLogLine(true, packet, size, opcode);
		}
		// Xman -Temporary Ban IP of clients causing an error-
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		GetPeerName((SOCKADDR*)&sockAddr,&nSockAddrLen);
		theApp.clientlist->AddBannedClient(ntohl(sockAddr.sin_addr.S_un.S_addr));

		if (client != NULL){ 			
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Client %s with IP: %s has been added temporary to the banned list"), client->GetUserName(), client->GetFullIP());			
			client->SetDownloadState(DS_ERROR, _T("Error while processing eDonkey packet. undefined Exception "), CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
		}
		else {
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("IP: %s has been added temporary to the banned list"), ipstr(sockAddr.sin_addr));			
		}		
		Disconnect(_T("processing eDonkey packet: client caused an undefined Exception"),CUpDownClient::USR_EXCEPTION);  // Maella -Upload Stop Reason-
		return false;
	}
	//Xman end
	return true;
}

bool CClientReqSocket::ProcessExtPacket(const BYTE* packet, uint32 size, UINT opcode, UINT uRawSize)
{
	try
	{
			if (!client && opcode!=OP_PORTTEST)
			{
				theStats.AddDownDataOverheadOther(uRawSize);
				throw GetResString(IDS_ERR_UNKNOWNCLIENTACTION);
			}
#ifdef _DEBUG// X: [RDL] - [Remove Debug Log]
			if (thePrefs.m_iDbgHeap >= 2 && opcode!=OP_PORTTEST)
				ASSERT_VALID(client);
#endif
			switch(opcode)
			{
                case OP_MULTIPACKET: // deprecated
				case OP_MULTIPACKET_EXT: // deprecated
				case OP_MULTIPACKET_EXT2:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						if (opcode == OP_MULTIPACKET_EXT)
							DebugRecv("OP_MultiPacket_Ext", client, (size >= 24) ? packet : NULL);
						else
							DebugRecv("OP_MultiPacket", client, (size >= 16) ? packet : NULL);
					}
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished();

					if( client->GetKadPort() && client->GetKadVersion() > 1)
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort());

					CSafeMemFile data_in(packet,size);
					uint64 nSize = 0;
					CKnownFile* reqfile;
					bool bNotFound = false;
					uchar reqfilehash[16];
					if (opcode == OP_MULTIPACKET_EXT2) // fileidentifier support
					{
						CFileIdentifierSA fileIdent;
						if (!fileIdent.ReadIdentifier(&data_in))
						{
							DebugLogWarning(_T("Error while reading file identifier from MultiPacket_Ext2 - %s"), client->DbgGetClientInfo());
							break;
						}
						md4cpy(reqfilehash, fileIdent.GetMD4Hash()); // need this in case we want to sent a FNF
						if ( (reqfile = theApp.sharedfiles->GetFileByID(fileIdent.GetMD4Hash())) == NULL ){
							if ( !((reqfile = theApp.downloadqueue->GetFileByID(fileIdent.GetMD4Hash())) != NULL
									&& reqfile->GetFileSize() > (uint64)PARTSIZE) )
							{
								bNotFound = true;
								client->CheckFailedFileIdReqs(fileIdent.GetMD4Hash());
							}
						}
						if (!bNotFound && !reqfile->GetFileIdentifier().CompareRelaxed(fileIdent)){
							bNotFound = true;
							//Xman Log part/size-mismatch
							if(thePrefs.GetLogPartmismatch())
								DebugLogWarning(_T("FileIdentifier mismatch on requested file, sending FNF; %s, File=\"%s\", Local Ident: %s, Received Ident: %s"), client->DbgGetClientInfo()
									, reqfile->GetFileName() , reqfile->GetFileIdentifier().DbgInfo(), fileIdent.DbgInfo());
						}
					}
					else // no fileidentifier
					{
					data_in.ReadHash16(reqfilehash);
					if (opcode == OP_MULTIPACKET_EXT){
						nSize = data_in.ReadUInt64();
					}
					if ( (reqfile = theApp.sharedfiles->GetFileByID(reqfilehash)) == NULL ){
						if ( !((reqfile = theApp.downloadqueue->GetFileByID(reqfilehash)) != NULL
								&& reqfile->GetFileSize() > (uint64)PARTSIZE) )
						{
								bNotFound = true;
							client->CheckFailedFileIdReqs(reqfilehash);
						}
					}
						if (!bNotFound && nSize != 0 && nSize != reqfile->GetFileSize()){
							bNotFound = true;
							//Xman Log part/size-mismatch
							if(thePrefs.GetLogPartmismatch())
								DebugLogWarning(_T("Size mismatch on requested file, sending FNF; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
						}
					}

					if (!bNotFound && reqfile->IsLargeFile() && !client->SupportsLargeFiles()){
						bNotFound = true;
						DebugLogWarning(_T("Client without 64bit file support requested large file; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
					}
					if (bNotFound)
					{
						// send file request no such file packet (0x48)
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__FileReqAnsNoFil", client, packet);
#endif
						Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
						md4cpy(replypacket->pBuffer, reqfilehash);
						theStats.AddUpDataOverheadFileRequest(replypacket->size);
						SendPacket(replypacket, true);
						break;
					}

					if (!client->GetWaitStartTime())
						client->SetWaitStartTime();
					
					//Xman Code Improvement: Add passive source after we evaluated OP_REQUESTFILENAME
					// if we are downloading this file, this could be a new source
					// no passive adding of files with only one part
					/*
					if (reqfile->IsPartFile() && reqfile->GetFileSize() > (uint64)PARTSIZE)
					{
						if (((CPartFile*)reqfile)->GetMaxSources() > ((CPartFile*)reqfile)->GetSourceCount()) 
							theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client, true);
					}
					*/
					// check to see if this is a new file they are asking for
					client->SetUploadFileID(reqfile);
					client->SetLastAction(opcode);	//Xman fix for startupload
					uint8 opcode_in;
					CSafeMemFile data_out(128);
					if (opcode == OP_MULTIPACKET_EXT2) // fileidentifier support
						reqfile->GetFileIdentifierC().WriteIdentifier(&data_out);
					else
					data_out.WriteHash16(reqfile->GetFileHash());
					bool bAnswerFNF = false;
					while(data_in.GetLength()-data_in.GetPosition() && !bAnswerFNF)
					{
						opcode_in = data_in.ReadUInt8();
						switch(opcode_in)
						{
							case OP_REQUESTFILENAME:
							{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPReqFileName", client, packet);
#endif

								if (!client->ProcessExtendedInfo(&data_in, reqfile)){
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
									if (thePrefs.GetDebugClientTCPLevel() > 0)
										DebugSend("OP__FileReqAnsNoFil", client, packet);
#endif
									Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
									md4cpy(replypacket->pBuffer, reqfile->GetFileHash());
									theStats.AddUpDataOverheadFileRequest(replypacket->size);
									SendPacket(replypacket, true);
									//Xman Log part/size-mismatch
									if(thePrefs.GetLogPartmismatch())
										DebugLogWarning(_T("Partcount mismatch on requested file, sending FNF; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());
									bAnswerFNF = true;
									break;
								}
								
								//Xman Code Improvement: Add passive source after we evaluated OP_REQUESTFILENAME
								// if we are downloading this file, this could be a new source
								// no passive adding of files with only one part
								if (reqfile->IsPartFile() && reqfile->GetFileSize() > (uint64)PARTSIZE)
								{
									CPartFile* partfile=(CPartFile*)reqfile;
									if (partfile->GetMaxSources() > partfile->GetSourceCount()) 
										theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)reqfile, client, true);
								}
								//Xman end

								data_out.WriteUInt8(OP_REQFILENAMEANSWER);
								data_out.WriteString(reqfile->GetFileName(), client->GetUnicodeSupport());
								break;
							}
							case OP_AICHFILEHASHREQ:
							{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPAichFileHashReq", client, packet);
#endif
								if (client->SupportsFileIdentifiers() || opcode == OP_MULTIPACKET_EXT2)
								{// not allowed anymore with fileidents supported
									DebugLogWarning(_T("Client requested AICH Hash packet, but supports FileIdentifiers, ignored - %s"), client->DbgGetClientInfo());
									break;
								}
								if (client->IsSupportingAICH() && reqfile->GetFileIdentifier().HasAICHHash())
								{
									data_out.WriteUInt8(OP_AICHFILEHASHANS);
									reqfile->GetFileIdentifier().GetAICHHash().Write(&data_out);
								}
								break;
							}
							case OP_SETREQFILEID:
							{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPSetReqFileID", client, packet);
#endif
								data_out.WriteUInt8(OP_FILESTATUS);
// morph4u :: SOTN :: Start
								reqfile->WriteSafePartStatus(&data_out, client);
/*
								if (reqfile->IsPartFile())
									((CPartFile*)reqfile)->WritePartStatus(&data_out);
*/
// morph4u :: SOTN :: End
								break;
							}
							//We still send the source packet separately.. 
							case OP_REQUESTSOURCES2:
							case OP_REQUESTSOURCES:
							{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv(opcode_in == OP_REQUESTSOURCES2 ? "OP_MPReqSources2" : "OP_MPReqSources", client, packet);
#endif

								if (thePrefs.GetDebugSourceExchange())
									AddDebugLogLine(false, _T("SXRecv: Client source request; %s, File=\"%s\""), client->DbgGetClientInfo(), reqfile->GetFileName());

								uint8 byRequestedVersion = 0;
								uint16 byRequestedOptions = 0;
								if (opcode_in == OP_REQUESTSOURCES2){ // SX2 requests contains additional data
									byRequestedVersion = data_in.ReadUInt8();
									byRequestedOptions = data_in.ReadUInt16();
								}
								//Although this shouldn't happen, it's a just in case to any Mods that mess with version numbers.
								if (byRequestedVersion > 0 || client->GetSourceExchange1Version() > 1)
								{
									DWORD dwTimePassed = ::GetTickCount() - client->GetLastSrcReqTime() + CONNECTION_LATENCY;
									bool bNeverAskedBefore = client->GetLastSrcReqTime() == 0;
									if (
										//if not complete and file is rare
										(    reqfile->IsPartFile()
										&& (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS)
										&& ((CPartFile*)reqfile)->GetSourceCount() <= RARE_FILE
										) ||
										//OR if file is not rare or if file is complete
										(bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS * MINCOMMONPENALTY)
										)
									{
										client->SetLastSrcReqTime();
										Packet* tosend = reqfile->CreateSrcInfoPacket(client, byRequestedVersion, byRequestedOptions);
										if (tosend)
										{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
											if (thePrefs.GetDebugClientTCPLevel() > 0)
												DebugSend("OP__AnswerSources", client, reqfile->GetFileHash());
#endif
											theStats.AddUpDataOverheadSourceExchange(tosend->size);
											SendPacket(tosend, true);
										}
									}
									/*else
									{
									if (thePrefs.GetVerbose())
									AddDebugLogLine(false, _T("RCV: Source Request to fast. (This is testing the new timers to see how much older client will not receive this)"));
									}*/
								}
								break;
							}
							default:
								{
									CString strError;
									strError.Format(_T("Invalid sub opcode 0x%02x received"), opcode_in);
									throw strError;
								}
						}
					}
					if( data_out.GetLength() > 16  && !bAnswerFNF)
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__MultiPacketAns", client, reqfile->GetFileHash());
#endif
						Packet* reply = new Packet(&data_out, OP_EMULEPROT, (opcode == OP_MULTIPACKET_EXT2) ?  OP_MULTIPACKETANSWER_EXT2 : OP_MULTIPACKETANSWER);
						theStats.AddUpDataOverheadFileRequest(reply->size);
						SendPacket(reply, true);
					}
					break;
				}
				case OP_MULTIPACKETANSWER:
				case OP_MULTIPACKETANSWER_EXT2:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_MultiPacketAns", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					client->CheckHandshakeFinished();

					if( client->GetKadPort() && client->GetKadVersion() > 1)
						Kademlia::CKademlia::Bootstrap(ntohl(client->GetIP()), client->GetKadPort());

					CSafeMemFile data_in(packet,size);
					
					CPartFile* reqfile = NULL;
					if (opcode == OP_MULTIPACKETANSWER_EXT2)
					{
						CFileIdentifierSA fileIdent;
						if (!fileIdent.ReadIdentifier(&data_in))
							throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER_EXT2; ReadIdentifier() failed)");
						reqfile = theApp.downloadqueue->GetFileByID(fileIdent.GetMD4Hash());
						if (reqfile==NULL){
							client->CheckFailedFileIdReqs(fileIdent.GetMD4Hash());
							throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER_EXT2; reqfile==NULL)");
						}
						if (!reqfile->GetFileIdentifier().CompareRelaxed(fileIdent))
							throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER_EXT2; FileIdentifier mistmatch)");
						if (fileIdent.HasAICHHash())
							client->ProcessAICHFileHash(NULL, reqfile, &fileIdent.GetAICHHash());
					}
					else{
					uchar reqfilehash[16];
					data_in.ReadHash16(reqfilehash);
						reqfile = theApp.downloadqueue->GetFileByID(reqfilehash);
					//Make sure we are downloading this file.
					if (reqfile==NULL){
						//Xman Code Fix
						//following situation: we are downloading a cue file (100 bytes) from many sources
						//this file will be finished earlier than our sources are answering
						//throwing an exception will filter good sources
						//swapping can't be done at this point, so I let it just timeout.
						CKnownFile* reqfiletocheck = theApp.sharedfiles->GetFileByID(reqfilehash);
						if(reqfiletocheck!=NULL && time(NULL) - (sint64)reqfiletocheck->GetUtcFileDate() < 90){ // X: Check whether file was finished 89sec ago
							//AddDebugLogLine(false, _T("client send NULL reqfile: %s"), client->DbgGetClientInfo());
							break;
						}
						client->CheckFailedFileIdReqs(reqfilehash);
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; reqfile==NULL)");
						//Xman end
					}
					// X: can happen with a late answer after stopped
					/*if (client->GetRequestFile()==NULL){
						if(reqfile->xState == POFC_WAITING){// X: [POFC] - [PauseOnFileComplete] avoid when file is completed
							client->CheckFailedFileIdReqs(reqfilehash);
							break;
						}
						throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; client->GetRequestFile()==NULL)");
					}*/
					if (reqfile != client->GetRequestFile())
					{
						//Xman Code Fix
						client->CheckFailedFileIdReqs(reqfilehash);
						//can happen with a late answer after swapping -->break!
						break;
						/*
						CKnownFile* reqfiletocheck = theApp.sharedfiles->GetFileByID(reqfilehash);
						if(reqfiletocheck!=NULL)
						{
							AddDebugLogLine(false, _T("reqfile!=client->GetRequestFile(): %s"), client->DbgGetClientInfo());
							break;
						}
						else
							throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (OP_MULTIPACKETANSWER; reqfile!=client->GetRequestFile())");
						*/
						//Xman end
					}
					}
					uint8 opcode_in;
					while(data_in.GetLength()-data_in.GetPosition())
					{
						opcode_in = data_in.ReadUInt8();
						switch(opcode_in)
						{
							case OP_REQFILENAMEANSWER:
							{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPReqFileNameAns", client, packet);
#endif

								client->ProcessFileInfo(&data_in, reqfile);
								break;
							}
							case OP_FILESTATUS:
							{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPFileStatus", client, packet);
#endif

								client->ProcessFileStatus(false, &data_in, reqfile);
								break;
							}
							case OP_AICHFILEHASHANS:
							{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientTCPLevel() > 0)
									DebugRecv("OP_MPAichFileHashAns", client);
#endif

								client->ProcessAICHFileHash(&data_in, reqfile, NULL);
								break;
							}
							default:
								{
									CString strError;
									strError.Format(_T("Invalid sub opcode 0x%02x received"), opcode_in);
									throw strError;
								}
						}
					}
					break;
				}
				case OP_EMULEINFO:
				{
					theStats.AddDownDataOverheadOther(uRawSize);
					client->ProcessMuleInfoPacket(packet,size);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_EmuleInfo", client);
						Debug(_T("  %s\n"), client->DbgGetMuleInfo());
					}
#endif

					// start secure identification, if
					//  - we have received eD2K and eMule info (old eMule)
					if (client->GetInfoPacketsReceived() == IP_BOTH)
						client->InfoPacketsReceived();

					client->SendMuleInfoPacket(true);
					break;
				}
				case OP_EMULEINFOANSWER:
				{
					theStats.AddDownDataOverheadOther(uRawSize);
					client->ProcessMuleInfoPacket(packet,size);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugRecv("OP_EmuleInfoAnswer", client);
						Debug(_T("  %s\n"), client->DbgGetMuleInfo());
					}
#endif

					// start secure identification, if
					//  - we have received eD2K and eMule info (old eMule)
					if (client->GetInfoPacketsReceived() == IP_BOTH)
						client->InfoPacketsReceived();
					break;
				}
				case OP_SECIDENTSTATE:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_SecIdentState", client);
#endif
					theStats.AddDownDataOverheadOther(uRawSize);
					client->ProcessSecIdentStatePacket(packet,size);
					if (client->GetSecureIdentState() == IS_SIGNATURENEEDED)
						client->SendSignaturePacket();
					else if (client->GetSecureIdentState() == IS_KEYANDSIGNEEDED)
					{
						client->SendPublicKeyPacket();
						client->SendSignaturePacket();
					}
					break;
				}
				case OP_PUBLICKEY:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicKey", client);
#endif
					theStats.AddDownDataOverheadOther(uRawSize);

					client->ProcessPublicKeyPacket(packet,size);
					break;
				}
  				case OP_SIGNATURE:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Signature", client);
#endif
					theStats.AddDownDataOverheadOther(uRawSize);
					client->ProcessSignaturePacket(packet,size);
					break;
				}
				case OP_QUEUERANKING:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_QueueRanking", client);
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					client->CheckHandshakeFinished();
					client->ProcessEmuleQueueRank(packet, size);
					break;
				}
				case OP_REQUESTSOURCES:
				case OP_REQUESTSOURCES2:
	                        {
						CSafeMemFile data(packet, size);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv(opcode == OP_REQUESTSOURCES2 ? "OP_MPReqSources2" : "OP_MPReqSources", client, (size >= 16) ? packet : NULL);
#endif

						theStats.AddDownDataOverheadSourceExchange(uRawSize);
						client->CheckHandshakeFinished();

						uint8 byRequestedVersion = 0;
						uint16 byRequestedOptions = 0;
						if (opcode == OP_REQUESTSOURCES2){ // SX2 requests contains additional data
							byRequestedVersion = data.ReadUInt8();
							byRequestedOptions = data.ReadUInt16();
						}
						//Although this shouldn't happen, it's a just in case to any Mods that mess with version numbers.
						if (byRequestedVersion > 0 || client->GetSourceExchange1Version() > 1)
						{
							if (size < 16)
								throw GetResString(IDS_ERR_BADSIZE);

							if (thePrefs.GetDebugSourceExchange())
								AddDebugLogLine(false, _T("SXRecv: Client source request; %s, %s"), client->DbgGetClientInfo(), DbgGetFileInfo(packet));

							//first check shared file list, then download list
							uchar ucHash[16];
							data.ReadHash16(ucHash);
							CKnownFile* reqfile;
							if ((reqfile = theApp.sharedfiles->GetFileByID(ucHash)) != NULL ||
								(reqfile = theApp.downloadqueue->GetFileByID(ucHash)) != NULL)
							{
								// There are some clients which do not follow the correct protocol procedure of sending
								// the sequence OP_REQUESTFILENAME, OP_SETREQFILEID, OP_REQUESTSOURCES. If those clients
								// are doing this, they will not get the optimal set of sources which we could offer if
								// the would follow the above noted protocol sequence. They better to it the right way
								// or they will get just a random set of sources because we do not know their download
								// part status which may get cleared with the call of 'SetUploadFileID'.
								client->SetUploadFileID(reqfile);

								DWORD dwTimePassed = ::GetTickCount() - client->GetLastSrcReqTime() + CONNECTION_LATENCY;
								bool bNeverAskedBefore = client->GetLastSrcReqTime() == 0;
								if (
									//if not complete and file is rare
									(    reqfile->IsPartFile()
									&& (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS)
									&& ((CPartFile*)reqfile)->GetSourceCount() <= RARE_FILE
									) ||
									//OR if file is not rare or if file is complete
									(bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASKS * MINCOMMONPENALTY)
									)
								{
									client->SetLastSrcReqTime();
									Packet* tosend = reqfile->CreateSrcInfoPacket(client, byRequestedVersion, byRequestedOptions);
									if (tosend)
									{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
										if (thePrefs.GetDebugClientTCPLevel() > 0)
											DebugSend("OP__AnswerSources", client, reqfile->GetFileHash());
#endif
										theStats.AddUpDataOverheadSourceExchange(tosend->size);
										SendPacket(tosend, true, true);
									}
								}
							}
							else
								client->CheckFailedFileIdReqs(ucHash);
						}
						break;
					}
				case OP_ANSWERSOURCES:
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_AnswerSources", client, (size >= 16) ? packet : NULL);
#endif
						theStats.AddDownDataOverheadSourceExchange(uRawSize);
						client->CheckHandshakeFinished();
					
						CSafeMemFile data(packet, size);
						uchar hash[16];
						data.ReadHash16(hash);
						CKnownFile* file = theApp.downloadqueue->GetFileByID(hash);
						if (file){
							if (file->IsPartFile()){
								//set the client's answer time
								client->SetLastSrcAnswerTime();
								//and set the file's last answer time
								((CPartFile*)file)->SetLastAnsweredTime();
								((CPartFile*)file)->AddClientSources(&data, client->GetSourceExchange1Version(), false, client);
							}
						}
						else
							client->CheckFailedFileIdReqs(hash);
						break;
					}
				case OP_ANSWERSOURCES2:
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_AnswerSources2", client, (size >= 17) ? packet : NULL);
#endif
						theStats.AddDownDataOverheadSourceExchange(uRawSize);
						client->CheckHandshakeFinished();

						CSafeMemFile data(packet, size);
						uint8 byVersion = data.ReadUInt8();
						uchar hash[16];
						data.ReadHash16(hash);
						CKnownFile* file = theApp.downloadqueue->GetFileByID(hash);
						if (file){
							if (file->IsPartFile()){
								//set the client's answer time
								client->SetLastSrcAnswerTime();
								//and set the file's last answer time
								((CPartFile*)file)->SetLastAnsweredTime();
								((CPartFile*)file)->AddClientSources(&data, byVersion, true, client);
							}
						}
						else
							client->CheckFailedFileIdReqs(hash);
						break;
					}
				case OP_PEERCACHE_QUERY:
				{
					theStats.AddDownDataOverheadFileRequest(uRawSize);
					/*CSafeMemFile dataSend(128);
					dataSend.WriteUInt8(1/*PCPCK_VERSION*);
					dataSend.WriteUInt8(0/*PCOP_NONE*);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						DebugSend("OP__PeerCacheAnswer", client);
						Debug(_T("  %s\n"), _T("Not supported"));
					}
#endif
					Packet* pEd2kPacket = new Packet(&dataSend, OP_EMULEPROT, OP_PEERCACHE_ANSWER);
					theStats.AddUpDataOverheadFileRequest(pEd2kPacket->size);
					SendPacket(pEd2kPacket);
					*/
					break;
				}
				case OP_PUBLICIP_ANSWER:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicIPAns", client);
#endif
					theStats.AddDownDataOverheadOther(uRawSize);
					client->ProcessPublicIPAnswer(packet,size);
					break;
				}
				case OP_PUBLICIP_REQ:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_PublicIPReq", client);
#endif
					theStats.AddDownDataOverheadOther(uRawSize);

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("OP__PublicIPAns", client);
#endif
					Packet* pPacket = new Packet(OP_PUBLICIP_ANSWER, 4, OP_EMULEPROT);
					PokeUInt32(pPacket->pBuffer, client->GetIP());
					theStats.AddUpDataOverheadOther(pPacket->size);
					SendPacket(pPacket);
					break;
				}
				case OP_PORTTEST:
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_PortTest", client);
#endif
						theStats.AddDownDataOverheadOther(uRawSize);

						m_bPortTestCon=true;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__PortTest", client);
#endif
						Packet* replypacket = new Packet(OP_PORTTEST, 1);
						replypacket->pBuffer[0]=0x12;
						theStats.AddUpDataOverheadOther(replypacket->size);
						SendPacket(replypacket);
						break;
					}
				case OP_CALLBACK:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_Callback", client);
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					if(!Kademlia::CKademlia::IsRunning())
						break;
					CSafeMemFile data(packet, size);
					Kademlia::CUInt128 check;
					data.ReadUInt128(&check);
					check.Xor(Kademlia::CUInt128(true));
					if(check == Kademlia::CKademlia::GetPrefs()->GetKadID())
					{
						Kademlia::CUInt128 fileid;
						data.ReadUInt128(&fileid);
						uchar fileid2[16];
						fileid.ToByteArray(fileid2);
						CKnownFile* reqfile;
						if ( (reqfile = theApp.sharedfiles->GetFileByID(fileid2)) == NULL )
						{
							if ( (reqfile = theApp.downloadqueue->GetFileByID(fileid2)) == NULL)
							{
								client->CheckFailedFileIdReqs(fileid2);
								break;
							}
						}

						uint32 ip = data.ReadUInt32();
						uint16 tcp = data.ReadUInt16();
						//Enig123 :: don't reconnect to banned clients
						uint32 nClientIP = ntohl(ip);
						if (theApp.clientlist->IsBannedClient(nClientIP)) {
							if (thePrefs.GetLogBannedClients())
								AddDebugLogLine(false, _T("Banned client IP=%s asked me to callback, rejected!"), ipstr(nClientIP));
							break;
						}
						//End
						CUpDownClient* callback = theApp.clientlist->FindClientByIP(nClientIP, tcp);
						if( callback == NULL )
						{
							callback = new CUpDownClient(NULL,tcp,ip,0,0);
							/*
							theApp.clientlist->AddClient(callback);
							*/	//Enig123::Optimizations - SkipDupCheck
							theApp.clientlist->AddClient(callback, true);
						}
						callback->TryToConnect(true); //zz_fly :: TODO: check whether the IsBannedClient() block in TryToConnect() is needed
					}
					break;
				}
				case OP_BUDDYPING:
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_BuddyPing", client);
#endif
						theStats.AddDownDataOverheadOther(uRawSize);

						CUpDownClient* buddy = theApp.clientlist->GetBuddy();
						if( buddy != client || client->GetKadVersion() == 0 || !client->AllowIncomeingBuddyPingPong() )
							//This ping was not from our buddy or wrong version or packet sent to fast. Ignore
							break;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__BuddyPong", client);
#endif
						Packet* replypacket = new Packet(OP_BUDDYPONG, 0, OP_EMULEPROT);
						theStats.AddDownDataOverheadOther(replypacket->size);
						SendPacket(replypacket);
						client->SetLastBuddyPingPongTime();
						break;
					}
				case OP_BUDDYPONG:
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_BuddyPong", client);
#endif
						theStats.AddDownDataOverheadOther(uRawSize);

						CUpDownClient* buddy = theApp.clientlist->GetBuddy();
						if( buddy != client || client->GetKadVersion() == 0 )
							//This pong was not from our buddy or wrong version. Ignore
							break;
						client->SetLastBuddyPingPongTime();
						//All this is for is to reset our socket timeout.
						break;
					}
				case OP_REASKCALLBACKTCP:
					{
						theStats.AddDownDataOverheadFileRequest(uRawSize);
						CUpDownClient* buddy = theApp.clientlist->GetBuddy();
						if (buddy != client) {
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
							if (thePrefs.GetDebugClientTCPLevel() > 0)
								DebugRecv("OP_ReaskCallbackTCP", client, NULL);
#endif
							//This callback was not from our buddy.. Ignore.
							break;
						}
						CSafeMemFile data_in(packet, size);
						uint32 destip = data_in.ReadUInt32();
						uint16 destport = data_in.ReadUInt16();
						uchar reqfilehash[16];
						data_in.ReadHash16(reqfilehash);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugRecv("OP_ReaskCallbackTCP", client, reqfilehash);
#endif
						CKnownFile* reqfile = theApp.sharedfiles->GetFileByID(reqfilehash);
						
						bool bSenderMultipleIpUnknown = false;
						CUpDownClient* sender = theApp.uploadqueue->GetWaitingClientByIP_UDP(destip, destport, true, &bSenderMultipleIpUnknown);					
						if (!reqfile)
						{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
							if (thePrefs.GetDebugClientUDPLevel() > 0)
								DebugSend("OP__FileNotFound", NULL);
#endif
							Packet* response = new Packet(OP_FILENOTFOUND,0,OP_EMULEPROT);
							theStats.AddUpDataOverheadFileRequest(response->size);
							if (sender != NULL)
								theApp.clientudp->SendPacket(response, destip, destport, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
							else
								theApp.clientudp->SendPacket(response, destip, destport, false, NULL, false, 0);
							break;
						}

						if (sender)
						{
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
									sender->ProcessExtendedInfo(&data_in, reqfile);
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
// morph4u :: SOTN :: Start
								reqfile->WriteSafePartStatus(&data_out, sender, true);
/*
									if (reqfile->IsPartFile())
										((CPartFile*)reqfile)->WritePartStatus(&data_out);
*/
// morph4u :: SOTN :: End								
								}
								data_out.WriteUInt16((uint16)theApp.uploadqueue->GetWaitingPosition(sender));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
								if (thePrefs.GetDebugClientUDPLevel() > 0)
									DebugSend("OP__ReaskAck", sender);
#endif
								Packet* response = new Packet(&data_out, OP_EMULEPROT, OP_REASKACK);
								theStats.AddUpDataOverheadFileRequest(response->size);
								theApp.clientudp->SendPacket(response, destip, destport, sender->ShouldReceiveCryptUDPPackets(), sender->GetUserHash(), false, 0);
							}
							else
							{
								DebugLogWarning(_T("Client UDP socket; OP_REASKCALLBACKTCP; reqfile does not match"));
								TRACE(_T("reqfile:         %s\n"), DbgGetFileInfo(reqfile->GetFileHash()));
								TRACE(_T("sender->GetRequestFile(): %s\n"), sender->GetRequestFile() ? DbgGetFileInfo(sender->GetRequestFile()->GetFileHash()) : _T("(null)"));
							}
						}
						else
						{
							if (!bSenderMultipleIpUnknown){
								if ((theApp.uploadqueue->GetWaitingUserCount() + 50) > thePrefs.GetQueueSize())
								{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
									if (thePrefs.GetDebugClientUDPLevel() > 0)
										DebugSend("OP__QueueFull", NULL);
#endif
									Packet* response = new Packet(OP_QUEUEFULL,0,OP_EMULEPROT);
									theStats.AddUpDataOverheadFileRequest(response->size);
									theApp.clientudp->SendPacket(response, destip, destport, false, NULL, false, 0);
								}
							}
							else{
								DebugLogWarning(_T("OP_REASKCALLBACKTCP Packet received - multiple clients with the same IP but different UDP port found. Possible UDP Portmapping problem, enforcing TCP connection. IP: %s, Port: %u"), ipstr(destip), destport); 
							}
						}
						break;
					}
				case OP_AICHANSWER:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichAnswer", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					client->ProcessAICHAnswer(packet,size);
					break;
				}
				case OP_AICHREQUEST:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichRequest", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					client->ProcessAICHRequest(packet,size);
					break;
				}
				case OP_AICHFILEHASHANS:
				{
					// those should not be received normally, since we should only get those in MULTIPACKET
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichFileHashAns", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					CSafeMemFile data(packet, size);
					client->ProcessAICHFileHash(&data, NULL, NULL);
					break;
				}
				case OP_AICHFILEHASHREQ:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_AichFileHashReq", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(uRawSize);

					// those should not be received normally, since we should only get those in MULTIPACKET
					CSafeMemFile data(packet, size);
					uchar abyHash[16];
					data.ReadHash16(abyHash);
					CKnownFile* pPartFile = theApp.sharedfiles->GetFileByID(abyHash);
					if (pPartFile == NULL){
						client->CheckFailedFileIdReqs(abyHash);
						break;
					}
					if (client->IsSupportingAICH() && pPartFile->GetFileIdentifier().HasAICHHash())
					{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
						if (thePrefs.GetDebugClientTCPLevel() > 0)
							DebugSend("OP__AichFileHashAns", client, abyHash);
#endif
						CSafeMemFile data_out(16+HASHSIZE);
						data_out.WriteHash16(abyHash);
						pPartFile->GetFileIdentifier().GetAICHHash().Write(&data_out);
						Packet* response = new Packet(&data_out, OP_EMULEPROT, OP_AICHFILEHASHANS);
						theStats.AddUpDataOverheadFileRequest(response->size);
						SendPacket(response);
					}
					break;
				}
				case OP_REQUESTPARTS_I64:
				{
					// see also OP_REQUESTPARTS
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_RequestParts_I64", client, (size >= 16) ? packet : NULL);
#endif
					theStats.AddDownDataOverheadFileRequest(size);

					CSafeMemFile data(packet, size);
					uchar reqfilehash[16];
					data.ReadHash16(reqfilehash);

					uint64 auStartOffsets[3];
					auStartOffsets[0] = data.ReadUInt64();
					auStartOffsets[1] = data.ReadUInt64();
					auStartOffsets[2] = data.ReadUInt64();

					uint64 auEndOffsets[3];
					auEndOffsets[0] = data.ReadUInt64();
					auEndOffsets[1] = data.ReadUInt64();
					auEndOffsets[2] = data.ReadUInt64();

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						Debug(_T("  Start1=%I64u  End1=%I64u  Size=%I64u\n"), auStartOffsets[0], auEndOffsets[0], auEndOffsets[0] - auStartOffsets[0]);
						Debug(_T("  Start2=%I64u  End2=%I64u  Size=%I64u\n"), auStartOffsets[1], auEndOffsets[1], auEndOffsets[1] - auStartOffsets[1]);
						Debug(_T("  Start3=%I64u  End3=%I64u  Size=%I64u\n"), auStartOffsets[2], auEndOffsets[2], auEndOffsets[2] - auStartOffsets[2]);
					}
#endif

					if(client->CloseBackdoor(reqfilehash))//Xman Close Backdoor v2
						break;

					for (size_t i = 0; i < ARRSIZE(auStartOffsets); i++)
					{
						if (auEndOffsets[i] > auStartOffsets[i])
						{
							Requested_Block_Struct* reqblock = new Requested_Block_Struct;
							reqblock->StartOffset = auStartOffsets[i];
							reqblock->EndOffset = auEndOffsets[i];
							md4cpy(reqblock->FileID, reqfilehash);
							reqblock->transferred = 0;
							client->AddReqBlock(reqblock);
						}
						else
						{
							if (thePrefs.GetVerbose())
							{
								if (auEndOffsets[i] != 0 || auStartOffsets[i] != 0)
									DebugLogWarning(_T("Client requests invalid %u. file block %I64u-%I64u (%I64d bytes): %s"), i, auStartOffsets[i], auEndOffsets[i], auEndOffsets[i] - auStartOffsets[i], client->DbgGetClientInfo());
							}
						}
					}
					//Xman for SiRoB: ReadBlockFromFileThread
					if(client->GetUploadState() == US_UPLOADING)
						client->CreateNextBlockPackage();
					break;
					//Xman end
				}
				case OP_COMPRESSEDPART:
				case OP_SENDINGPART_I64:
				case OP_COMPRESSEDPART_I64:
				{
					// see also OP_SENDINGPART
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 1){
						if (opcode == OP_COMPRESSEDPART)
							DebugRecv("OP_CompressedPart", client, (size >= 16) ? packet : NULL);
						else if (opcode == OP_SENDINGPART_I64)
							DebugRecv("OP_SendingPart_I64", client, (size >= 16) ? packet : NULL);
						else
							DebugRecv("OP_CompressedPart_I64", client, (size >= 16) ? packet : NULL);
					}
#endif

					theStats.AddDownDataOverheadFileRequest(16 + 2*(opcode == OP_COMPRESSEDPART ? 4 : 8));
					client->CheckHandshakeFinished();

					if (client->GetRequestFile() && !client->GetRequestFile()->IsStopped() && (client->GetRequestFile()->GetStatus()==PS_READY || client->GetRequestFile()->GetStatus()==PS_EMPTY))
					{
						client->ProcessBlockPacket(packet, size, (opcode == OP_COMPRESSEDPART || opcode == OP_COMPRESSEDPART_I64), (opcode == OP_SENDINGPART_I64 || opcode == OP_COMPRESSEDPART_I64) );
						if (client->GetRequestFile())
						{
						if (client->GetRequestFile()->IsStopped() || client->GetRequestFile()->GetStatus()==PS_PAUSED || client->GetRequestFile()->GetStatus()==PS_ERROR)
						{
							client->SendCancelTransfer();
							client->SetDownloadState(client->GetRequestFile()->IsStopped() ? DS_NONE : DS_ONQUEUE,_T("paused file"), CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
						}
					}
					else
							ASSERT( false );
					}
					else
					{
						client->SendCancelTransfer();
						client->SetDownloadState((client->GetRequestFile()==NULL || client->GetRequestFile()->IsStopped()) ? DS_NONE : DS_ONQUEUE,_T("paused file") ,CUpDownClient::DSR_PAUSED); // Maella -Download Stop Reason-
					}
					break;
				}
				case OP_FWCHECKUDPREQ: //*Support required for Kadversion >= 6
				{
					// Kad related packet
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_FWCHECKUDPREQ", client);
#endif
					theStats.AddDownDataOverheadOther(uRawSize);
					CSafeMemFile data(packet, size);
					client->ProcessFirewallCheckUDPRequest(&data);
					break;
				}
				case OP_KAD_FWTCPCHECK_ACK: //*Support required for Kadversion >= 7
				{
					// Kad related packet, replaces KADEMLIA_FIREWALLED_ACK_RES
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_KAD_FWTCPCHECK_ACK", client);
#endif
					if (theApp.clientlist->IsKadFirewallCheckIP(client->GetIP())){
						if (Kademlia::CKademlia::IsRunning())
							Kademlia::CKademlia::GetPrefs()->IncFirewalled();
					}
					else
						DebugLogWarning(_T("Unrequested OP_KAD_FWTCPCHECK_ACK packet from client %s"), client->DbgGetClientInfo());
					break;
				}
				// ==> Recognize MlDonkey XS Answer [Spike2/ideas by Wiz] - Stulle
				case OP_XSMLDONKEY:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_XSmlDonkey", client);
#endif
					theStats.AddDownDataOverheadOther(size);

					client->CheckHandshakeFinished();

					break;
				}
				case OP_HASHSETANSWER2:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetAnswer2", client);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					client->ProcessHashSet(packet, size, true);
					break;
				}
				case OP_HASHSETREQUEST2:
				{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugRecv("OP_HashSetReq2", client);
#endif
					theStats.AddDownDataOverheadFileRequest(size);
					client->SendHashsetPacket(packet, size, true);
					break;
				}
				// <== Recognize MlDonkey XS Answer [Spike2/ideas by Wiz] - Stulle
				default:
					theStats.AddDownDataOverheadOther(uRawSize);
					//Xman final version: don't log too much
#ifdef LOGTAG
					AddDebugLogLine(false, _T("Received unknown emule-Packet"));
					PacketToDebugLogLine(false, packet, size, opcode);
#endif
					break;
			}
	}
	/*
	catch(CClientException* ex) // nearly same as the 'CString' exception but with optional deleting of the client
	{
		if (thePrefs.GetVerbose() && !ex->m_strMsg.IsEmpty())
			DebugLogWarning(_T("%s - while processing eMule packet: opcode=%s  size=%u; %s"), ex->m_strMsg, DbgGetMuleClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if (client && ex->m_bDelete)
			client->SetDownloadState(DS_ERROR, _T("Error while processing eMule packet: ") + ex->m_strMsg);
		Disconnect(ex->m_strMsg);
		ex->Delete();
		return false;
	}
	*/
	//Xman Xtreme Mod Exception Handling 
	catch(CString error)
	{
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		GetPeerName((SOCKADDR*)&sockAddr,&nSockAddrLen);
		theApp.clientlist->AddBannedClient(ntohl(sockAddr.sin_addr.S_un.S_addr));
		if (client != NULL){
			if (thePrefs.GetVerbose() && !error.IsEmpty()){
				DebugLogWarning(_T("Error: %s - while processing eMule packet: opcode=%s  size=%u; %s"), error, DbgGetDonkeyClientTCPOpcode(opcode), size, DbgGetClientInfo());
				AddDebugLogLine(false, _T("Client %s with IP: %s has been added temporary to the banned list"), client->GetUserName(), client->GetFullIP());			
			}									
			client->SetDownloadState(DS_ERROR, _T("ProcessExtPacket error. CString Exception ") + error, CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
		}
		else {

			if (thePrefs.GetVerbose() && !error.IsEmpty()){
				AddDebugLogLine(false, _T("Client with IP=%s caused an error or did something bad: %s. Disconnecting client!"), ipstr(sockAddr.sin_addr), error.GetBuffer());
				AddDebugLogLine(false, _T("IP: %s has been added temporary to the banned list"), ipstr(sockAddr.sin_addr));			
			}
		}
		//PacketToDebugLogLine(true, packet, size, opcode, DLP_DEFAULT);	//Xman too many infos
		Disconnect(_T("ProcessExtPacket error. CString Exception") + error, CUpDownClient::USR_EXCEPTION);  // Maella -Upload Stop Reason-
		return false;
	}
	catch(CFileException* pFileException){
		pFileException->Delete();
		if (thePrefs.GetVerbose())
		{
			AddDebugLogLine(true, _T("A client has caused an exception. Disconnecting client!"));		
			PacketToDebugLogLine(true, packet, size, opcode);
		}
		// Xman -Temporary Ban IP of clients causing an error-
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		GetPeerName((SOCKADDR*)&sockAddr,&nSockAddrLen);
		theApp.clientlist->AddBannedClient(ntohl(sockAddr.sin_addr.S_un.S_addr));

		if (client != NULL){ 			
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Client %s with IP: %s has been added temporary to the banned list"), client->GetUserName(), client->GetFullIP());			
			client->SetDownloadState(DS_ERROR,_T("ProcessExtPacket error. CFile Exception "), CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
		}
		else {
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("IP: %s has been added temporary to the banned list"), ipstr(sockAddr.sin_addr));			
		}		
		Disconnect(_T("ProcessExtPacket: client caused a FileException") ,CUpDownClient::USR_EXCEPTION);  // Maella -Upload Stop Reason-
		return false;
	}
	catch(...){
		if (thePrefs.GetVerbose())
		{
			AddDebugLogLine(true, _T("A client has caused an undefined exception. Disconnecting client!"));		
			PacketToDebugLogLine(true, packet, size, opcode);
		}
		// Xman -Temporary Ban IP of clients causing an error-
		SOCKADDR_IN sockAddr;
		memset(&sockAddr, 0, sizeof(sockAddr));
		int nSockAddrLen = sizeof(sockAddr);
		GetPeerName((SOCKADDR*)&sockAddr,&nSockAddrLen);
		theApp.clientlist->AddBannedClient(ntohl(sockAddr.sin_addr.S_un.S_addr));

		if (client != NULL){ 			
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("Client %s with IP: %s has been added temporary to the banned list"), client->GetUserName(), client->GetFullIP());			
			client->SetDownloadState(DS_ERROR,_T("ProcessExtPacket error. undefined Exception "), CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
		}
		else {
			if (thePrefs.GetVerbose())
				AddDebugLogLine(false, _T("IP: %s has been added temporary to the banned list"), ipstr(sockAddr.sin_addr));			
		}		
		Disconnect(_T("ProcessExtPacket: client caused an undefined Exception"), CUpDownClient::USR_EXCEPTION);  // Maella -Upload Stop Reason-
		return false;
	}
	//Xman end
	return true;
}
//Xman
// - Maella -Dump information of unknown packet in debug tab-
void CClientReqSocket::PacketToDebugLogLine(bool isOpcodeKnown, const uchar* packet, uint32 size, UINT opcode)
{
	if (thePrefs.GetVerbose())
	{
		// Dump Packet information	
		CString buffer; 
		if(isOpcodeKnown == false)
			buffer.Format(_T("Packet with unknown opcode: 0x%x, size=%d"), opcode, size);
		else
			buffer.Format(_T("Invalid packet with opcode: 0x%x, size=%d"), opcode, size);

		// Client information
		if(client != NULL){
			CString uploadString = _T("unknown");
			switch (client->GetUploadState()){
			case US_ONUPLOADQUEUE:
				uploadString = GetResString(IDS_ONQUEUE);
				break;
			case US_BANNED:
				uploadString = GetResString(IDS_BANNED);
				break;
			case US_CONNECTING:
				uploadString = GetResString(IDS_CONNECTING);
				break;
			case US_UPLOADING:
				uploadString = GetResString(IDS_TRANSFERRING);
				break;
			case US_NONE:
				uploadString = _T("none");
				break;
			}

			CString downloadString = _T("unknown");
			switch (client->GetDownloadState()) {
			case DS_CONNECTING:
				downloadString = GetResString(IDS_CONNECTING);
				break;
			case DS_CONNECTED:
				downloadString = GetResString(IDS_ASKING);
				break;
			case DS_WAITCALLBACK:
				downloadString = GetResString(IDS_CONNVIASERVER);
				break;
			case DS_ONQUEUE:
				if( client->IsRemoteQueueFull() )
					downloadString = GetResString(IDS_QUEUEFULL);
				else
					downloadString = GetResString(IDS_ONQUEUE);
				break;
			case DS_DOWNLOADING:
				downloadString = GetResString(IDS_TRANSFERRING);
				break;
			case DS_REQHASHSET:
				downloadString = GetResString(IDS_RECHASHSET);
				break;
			case DS_NONEEDEDPARTS:
				downloadString = GetResString(IDS_NONEEDEDPARTS);
				break;
			case DS_LOWTOLOWIP:
				downloadString = GetResString(IDS_NOCONNECTLOW2LOW);
				break;
			case DS_TOOMANYCONNS:
				downloadString = GetResString(IDS_TOOMANYCONNS);
				break;
			case DS_NONE:
				downloadString = _T("none");
				break;
			}

			buffer += _T(", client=") + client->GetClientSoftVer();
			buffer += _T(", up. state=") + uploadString;
			buffer += _T(", down. state=") + downloadString;
		}
		// Hex packet dump
		buffer += _T(", data=[");	
		UINT i;
#ifdef LOGTAG
		for (i = 0; i < size ; i++){
#else
		for (i = 0; i < size && i < 50; i++){
#endif
			if (i > 0)
				buffer += _T(' ');
			TCHAR temp[33];
			buffer += _itot((unsigned char)packet[i], temp, 16);		
		}
		buffer += (i == size) ? _T("]") : _T("..]");
		DbgAppendClientInfo(buffer);
		AddDebugLogLine(false,_T("%s"), buffer);
	}
}
// Maella end

CString CClientReqSocket::DbgGetClientInfo()
{
	CString str;
	SOCKADDR_IN sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	if (sockAddr.sin_addr.S_un.S_addr != 0 && (client == NULL || sockAddr.sin_addr.S_un.S_addr != client->GetIP()))
		str.AppendFormat(_T("IP=%s"), ipstr(sockAddr.sin_addr));
	if (client){
		if (!str.IsEmpty())
			str += _T("; ");
		str += _T("Client=") + client->DbgGetClientInfo();
	}
	return str;
}

void CClientReqSocket::DbgAppendClientInfo(CString& str)
{
	CString strClientInfo(DbgGetClientInfo());
	if (!strClientInfo.IsEmpty()){
		if (!str.IsEmpty())
			str += _T("; ");
		str += strClientInfo;
	}
}

void CClientReqSocket::OnConnect(int nErrorCode)
{
	SetConState(SS_Complete);
	CEMSocket::OnConnect(nErrorCode);
	if (nErrorCode)
	{
	    CString strTCPError;
		//Xman Bugfix, thanks Howe
		/*
		if (thePrefs.GetVerbose())
		{
			strTCPError = GetFullErrorMessage(nErrorCode);
		    if ((nErrorCode != WSAECONNREFUSED && nErrorCode != WSAETIMEDOUT) || !GetLastProxyError().IsEmpty())
			    DebugLogError(_T("Client TCP socket (OnConnect): %s; %s"), strTCPError, DbgGetClientInfo());
		}
		*/
		strTCPError = GetFullErrorMessage(nErrorCode);
		if ((nErrorCode != WSAECONNREFUSED && nErrorCode != WSAETIMEDOUT) || !GetLastProxyError().IsEmpty())
		{
		    //Xman final version:
			//don't show this "error" because it's too normal and I only saw:
			//WSAENETUNREACH and WSAEHOSTUNREACH
			//DebugLogError(_T("Client TCP socket (OnConnect): %s; %s"), strTCPError, DbgGetClientInfo());
			//Xman Xtreme Mod: in such a case don't give the clients more connection-retrys
			if(client)
				client->m_cFailed=5;
		}
		Disconnect(strTCPError, CUpDownClient::USR_SOCKET); // Maella -Upload Stop Reason-
	}
	else
	{
		//This socket may have been delayed by SP2 protection, lets make sure it doesn't time out instantly.
		ResetTimeOutTimer();

		// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
		SetMSSFromSocket(m_SocketData.hSocket);
	}
}

void CClientReqSocket::OnSend(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnSend(nErrorCode);

	//Xman NAFC
	//after any socket is connected we have the only change to find out which IP emule was bound
	if(theApp.listensocket->boundcheck)
	{
		theApp.listensocket->boundcheck = false;
		sockaddr_in socketcheck;
		int length= sizeof(socketcheck);
		if(getsockname(m_SocketData.hSocket, (SOCKADDR*)&socketcheck, &length)==0)
		{
			theApp.pBandWidthControl->SetBoundIP(socketcheck.sin_addr.S_un.S_addr);
		}

	}
}

void CClientReqSocket::OnError(int nErrorCode)
{
	CString strTCPError;
	if (thePrefs.GetVerbose())
	{
		if (nErrorCode == ERR_WRONGHEADER)
			strTCPError = _T("Error: Wrong header");
		else if (nErrorCode == ERR_TOOBIG)
			strTCPError = _T("Error: Too much data sent");
		else if (nErrorCode == ERR_ENCRYPTION)
			strTCPError = _T("Error: Encryption layer error");
		else if (nErrorCode == ERR_ENCRYPTION_NOTALLOWED)
			strTCPError = _T("Error: Unencrypted Connection when Encryption was required");
		else
			strTCPError = GetErrorMessage(nErrorCode);
		DebugLogWarning(_T("Client TCP socket: %s; %s"), strTCPError, DbgGetClientInfo());
	}
	//Xman Xtreme Mod: in such a case don't give the clients more connection-retrys
	if(client)
		client->m_cFailed=5;

	Disconnect(strTCPError, CUpDownClient::USR_SOCKET); // Maella -Upload Stop Reason-
}

bool CClientReqSocket::PacketReceivedCppEH(Packet* packet)
{
	bool bResult;
	UINT uRawSize = packet->size;
#ifdef LOGTAG
		PacketToDebugLogLine(false, (const BYTE*)packet->pBuffer,packet->size,packet->opcode); //Xman for test
#endif
	switch (packet->prot){
		case OP_EDONKEYPROT:
			bResult = ProcessPacket((const BYTE*)packet->pBuffer,packet->size,packet->opcode);
			break;
		case OP_PACKEDPROT:
			if (!packet->UnPackPacket()){
				if (thePrefs.GetVerbose())
					DebugLogError(_T("Failed to decompress client TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());
				bResult = false;
				break;
			}
		case OP_EMULEPROT:
			bResult = ProcessExtPacket((const BYTE*)packet->pBuffer, packet->size, packet->opcode, uRawSize);
			break;
		default:{
			theStats.AddDownDataOverheadOther(uRawSize);
			if (thePrefs.GetVerbose())
				DebugLogWarning(_T("Received unknown client TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());

			if (client){
				client->SetDownloadState(DS_ERROR, _T("Unknown protocol"), CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
				theApp.clientlist->m_globDeadSourceList.AddDeadSource(client); //Xman Xtreme Mod  I don't AddDeadsource in disconnected
			}
			Disconnect(_T("Unknown protocol"), CUpDownClient::USR_EXCEPTION); // Maella -Upload Stop Reason-
			bResult = false;
		}
		break;
	}
	return bResult;
}

#if !NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
int FilterSE(DWORD dwExCode, LPEXCEPTION_POINTERS pExPtrs, CClientReqSocket* reqsock, Packet* packet)
{
	if (thePrefs.GetVerbose())
	{
		CString strExError;
		if (pExPtrs){
			const EXCEPTION_RECORD* er = pExPtrs->ExceptionRecord;
			strExError.Format(_T("Error: Unknown exception %08x in CClientReqSocket::PacketReceived at %p"), er->ExceptionCode, er->ExceptionAddress);
		}
		else
			strExError.Format(_T("Error: Unknown exception %08x in CClientReqSocket::PacketReceived"), dwExCode);

		// we already had an unknown exception, better be prepared for dealing with invalid data -> use another exception handler
		try{
			CString strError = strExError;
			strError.AppendFormat(_T("; %s"), DbgGetClientTCPPacket(packet?packet->prot:0, packet?packet->opcode:0, packet?packet->size:0));
			reqsock->DbgAppendClientInfo(strError);
			DebugLogError(_T("%s"), strError);
		}
		catch(...){
			ASSERT(0);
			DebugLogError(_T("%s"), strExError);
		}
	}
	
	// this searches the next exception handler -> catch(...) in 'CAsyncSocketExHelperWindow::WindowProc'
	// as long as I do not know where and why we are crashing, I prefere to have it handled that way which
	// worked fine in 28a/b.
	//
	// 03-Jan-2004 [bc]: Returning the execution to the catch-all handler in 'CAsyncSocketExHelperWindow::WindowProc'
	// can make things even worse, because in some situations, the socket will continue fireing received events. And
	// because the processed packet (which has thrown the exception) was not removed from the EMSocket buffers, it would
	// be processed again and again.
	//return EXCEPTION_CONTINUE_SEARCH;

	// this would continue the program "as usual" -> return execution to the '__except' handler
	return EXCEPTION_EXECUTE_HANDLER;
}
#endif//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER

#if !NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
int CClientReqSocket::PacketReceivedSEH(Packet* packet)
{
	int iResult;
	// this function is only here to get a chance of determining the crash address via SEH
	__try{
		iResult = PacketReceivedCppEH(packet);
	}
	__except(FilterSE(GetExceptionCode(), GetExceptionInformation(), this, packet)){
		iResult = -1;
	}
	return iResult;
}
#endif//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER

bool CClientReqSocket::PacketReceived(Packet* packet)
{
	bool bResult;
#if !NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
	int iResult = PacketReceivedSEH(packet);
	if (iResult < 0)
	{
		if (client)
		{
			client->SetDownloadState(DS_ERROR, _T("Unknown Exception"), CUpDownClient::DSR_EXCEPTION); // Maella -Download Stop Reason-
			theApp.clientlist->m_globDeadSourceList.AddDeadSource(client); //Xman Xtreme Mod  I don't AddDeadsource in disconnected
		}
		Disconnect(_T("Unknown Exception"), CUpDownClient::USR_EXCEPTION); // Maella -Upload Stop Reason-
		bResult = false;
	}
	else
		bResult = iResult!=0;
#else//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
	bResult = PacketReceivedCppEH(packet);
#endif//!NO_USE_CLIENT_TCP_CATCH_ALL_HANDLER
	return bResult;
}

void CClientReqSocket::OnReceive(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnReceive(nErrorCode);
}

bool CClientReqSocket::Create()
{
	theApp.listensocket->AddConnection();
	return (CAsyncSocketEx::Create(0, SOCK_STREAM, FD_WRITE | FD_READ | FD_CLOSE | FD_CONNECT, thePrefs.GetBindAddrW()) != FALSE);
}

SocketSentBytes CClientReqSocket::SendControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend)
{
	SocketSentBytes returnStatus = CEMSocket::SendControlData(maxNumberOfBytesToSend, overchargeMaxBytesToSend);
	if (returnStatus.success && (returnStatus.sentBytesControlPackets > 0 || returnStatus.sentBytesStandardPackets > 0))
		ResetTimeOutTimer();
	return returnStatus;
}

SocketSentBytes CClientReqSocket::SendFileAndControlData(uint32 maxNumberOfBytesToSend, uint32 overchargeMaxBytesToSend)
{
	SocketSentBytes returnStatus = CEMSocket::SendFileAndControlData(maxNumberOfBytesToSend, overchargeMaxBytesToSend);
	if (returnStatus.success && (returnStatus.sentBytesControlPackets > 0 || returnStatus.sentBytesStandardPackets > 0))
		ResetTimeOutTimer();
	return returnStatus;
}

void CClientReqSocket::SendPacket(Packet* packet, bool delpacket, bool controlpacket, uint32 actualPayloadSize, bool bForceImmediateSend)
{
	ResetTimeOutTimer();
	CEMSocket::SendPacket(packet,delpacket,controlpacket, actualPayloadSize, bForceImmediateSend);
}

bool CListenSocket::SendPortTestReply(char result, bool disconnect)
{
	for(POSITION pos = socket_list.GetHeadPosition(); pos != NULL; ){// X: [CI] - [Code Improvement]
		CClientReqSocket* cur_sock = socket_list.GetNext(pos);
		if (cur_sock->m_bPortTestCon)
		{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__PortTest", cur_sock->client);
#endif
			Packet* replypacket = new Packet(OP_PORTTEST, 1);
			replypacket->pBuffer[0]=result;
			theStats.AddUpDataOverheadOther(replypacket->size);
			cur_sock->SendPacket(replypacket);
			if (disconnect)
				cur_sock->m_bPortTestCon = false;
			return true;
		}
	}
	return false;
}

CListenSocket::CListenSocket()
	: bListening(false)
	, maxconnectionreached(0)
	, m_OpenSocketsInterval(0)
	, m_nPendingConnections(0)
	, peakconnections(0)
	, totalconnectionchecks(0)
	, averageconnections(0.0f)
	, activeconnections(0)
	, m_port(0)
	, m_nHalfOpen(0)
	, m_nComp(0)
	, boundcheck(true)//Xman NAFC
{
	memset(m_ConnectionStates, 0, sizeof m_ConnectionStates);
}

CListenSocket::~CListenSocket()
{
	Close();
	KillAllSockets();
}

bool CListenSocket::Rebind()
{
	if (thePrefs.GetPort() == m_port)
		return false;

	Close();
	KillAllSockets();
	//Xman NAFC
	boundcheck = true;

	return StartListening();
}

bool CListenSocket::StartListening(){
	//Xman Info about binding
	if(thePrefs.GetBindAddrW()!=NULL)
	{
		AddLogLine(false,_T("You specified an ip-address to bind. Try to bind to: %s"), thePrefs.GetBindAddrW());
	}
	//Xman Info about binding

	bListening = true;

	// Creating the socket with SO_REUSEADDR may solve LowID issues if emule was restarted
	// quickly or started after a crash, but(!) it will also create another problem. If the
	// socket is already used by some other application (e.g. a 2nd emule), we though bind
	// to that socket leading to the situation that 2 applications are listening at the same
	// port!
	if (!Create(thePrefs.GetPort(), SOCK_STREAM, FD_ACCEPT, thePrefs.GetBindAddrW())

	// Rejecting a connection with conditional WSAAccept and not using SO_CONDITIONAL_ACCEPT
	// -------------------------------------------------------------------------------------
	// recv: SYN
	// send: SYN ACK (!)
	// recv: ACK
	// send: ACK RST
	// recv: PSH ACK + OP_HELLO packet
	// send: RST
	// --- 455 total bytes (depending on OP_HELLO packet)
	// In case SO_CONDITIONAL_ACCEPT is not used, the TCP/IP stack establishes the connection
	// before WSAAccept has a chance to reject it. That's why the remote peer starts to send
	// it's first data packet.
	// ---
	// Not using SO_CONDITIONAL_ACCEPT gives us 6 TCP packets and the OP_HELLO data. We
	// have to lookup the IP only 1 time. This is still way less traffic than rejecting the
	// connection by closing it after the 'Accept'.

	// Rejecting a connection with conditional WSAAccept and using SO_CONDITIONAL_ACCEPT
	// ---------------------------------------------------------------------------------
	// recv: SYN
	// send: ACK RST
	// recv: SYN
	// send: ACK RST
	// recv: SYN
	// send: ACK RST
	// --- 348 total bytes
	// The TCP/IP stack tries to establish the connection 3 times until it gives up. 
	// Furthermore the remote peer experiences a total timeout of ~ 1 minute which is
	// supposed to be the default TCP/IP connection timeout (as noted in MSDN).
	// ---
	// Although we get a total of 6 TCP packets in case of using SO_CONDITIONAL_ACCEPT,
	// it's still less than not using SO_CONDITIONAL_ACCEPT. But, we have to lookup
	// the IP 3 times instead of 1 time.

	//if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxySettings().UseProxy) {
	//	int iOptVal = 1;
	//	VERIFY( SetSockOpt(SO_CONDITIONAL_ACCEPT, &iOptVal, sizeof iOptVal) );
	//}

	|| !Listen())
		return false;

	//Xman Info about binding
	if(thePrefs.GetBindAddrW()!=NULL)
	{
		AddLogLine(false,_T("binding successful"));
	}
	//Xman Info about binding

	m_port = thePrefs.GetPort();
	return true;
}

void CListenSocket::ReStartListening()
{
	bListening = true;

	ASSERT( m_nPendingConnections >= 0 );
	if (m_nPendingConnections > 0)
	{
		m_nPendingConnections--;
		OnAccept(0);
	}
}

void CListenSocket::StopListening()
{
	bListening = false;
	maxconnectionreached++;
}

static int s_iAcceptConnectionCondRejected;

int CALLBACK AcceptConnectionCond(LPWSABUF lpCallerId, LPWSABUF /*lpCallerData*/, LPQOS /*lpSQOS*/, LPQOS /*lpGQOS*/,
								  LPWSABUF /*lpCalleeId*/, LPWSABUF /*lpCalleeData*/, GROUP FAR* /*g*/, DWORD_PTR /*dwCallbackData*/)
{
	if (lpCallerId && lpCallerId->buf && lpCallerId->len >= sizeof SOCKADDR_IN)
	{
		LPSOCKADDR_IN pSockAddr = (LPSOCKADDR_IN)lpCallerId->buf;
		ASSERT( pSockAddr->sin_addr.S_un.S_addr != 0 && pSockAddr->sin_addr.S_un.S_addr != INADDR_NONE );

		if (theApp.ipfilter->IsFiltered(pSockAddr->sin_addr.S_un.S_addr)){
			if (thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - IP filter"), ipstr(pSockAddr->sin_addr.S_un.S_addr));
			s_iAcceptConnectionCondRejected = 1;
			return CF_REJECT;
		}

		if (theApp.clientlist->IsBannedClient(pSockAddr->sin_addr.S_un.S_addr)){
			if (thePrefs.GetLogBannedClients()){
				CUpDownClient* pClient = theApp.clientlist->FindClientByIP(pSockAddr->sin_addr.S_un.S_addr);
				AddDebugLogLine(false, _T("Rejecting connection attempt of banned client %s %s"), ipstr(pSockAddr->sin_addr.S_un.S_addr), pClient ? pClient->DbgGetClientInfo() : _T("unknown")); //Xman code fix
			}
			s_iAcceptConnectionCondRejected = 2;
			return CF_REJECT;
		}
	}
	else {
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Client TCP socket: AcceptConnectionCond unexpected lpCallerId"));
	}

	return CF_ACCEPT;
}

void CListenSocket::OnAccept(int nErrorCode)
{
	if (!nErrorCode)
	{
		//Xman
		// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		theApp.pBandWidthControl->AddeMuleInTCPOverall(0); // SYN
		// Maella end
		m_nPendingConnections++;
		if (m_nPendingConnections < 1){
			ASSERT(0);
			m_nPendingConnections = 1;
		}

		if (TooManySockets(true) && !theApp.serverconnect->IsConnecting()){
			StopListening();
			return;
		}
		else if (!bListening)
			ReStartListening(); //If the client is still at maxconnections, this will allow it to go above it.. But if you don't, you will get a lowID on all servers.

		uint32 nFataErrors = 0;
		while (m_nPendingConnections > 0)
		{
			m_nPendingConnections--;

			CClientReqSocket* newclient;
			SOCKADDR_IN SockAddr = {0};
			int iSockAddrLen = sizeof SockAddr;
			if (thePrefs.GetConditionalTCPAccept() && !thePrefs.GetProxySettings().UseProxy)
			{
				s_iAcceptConnectionCondRejected = 0;
				SOCKET sNew = WSAAccept(m_SocketData.hSocket, (SOCKADDR*)&SockAddr, &iSockAddrLen, AcceptConnectionCond, 0);
				if (sNew == INVALID_SOCKET){
					DWORD nError = GetLastError();
					if (nError == WSAEWOULDBLOCK){
						DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
						m_nPendingConnections = 0;
						break;
					}
					else{
						if (nError != WSAECONNREFUSED || s_iAcceptConnectionCondRejected == 0){
							DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
							nFataErrors++;
						}
						else if (s_iAcceptConnectionCondRejected == 1)
							theStats.filteredclients++;
					}
					if (nFataErrors > 10){
						// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
						// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
						// this should basically never happen anyway
						// however if we are in such a position, try to reinitalize the socket.
						DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
						Close();
						StartListening();
						m_nPendingConnections = 0;
						break;
					}
					continue;
				}
				newclient = new CClientReqSocket;
				//Xman
				// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
				theApp.pBandWidthControl->AddeMuleOutTCPOverall(0);
				// Maella end

				VERIFY( newclient->InitAsyncSocketExInstance() );
				newclient->m_SocketData.hSocket = sNew;
				newclient->AttachHandle(sNew);

				AddConnection();

				// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
				newclient->SetMSSFromSocket(newclient->GetSocketHandle());
			}
			else
			{
				newclient = new CClientReqSocket;
				//Xman
				// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
				theApp.pBandWidthControl->AddeMuleOutTCPOverall(0);
				// Maella end
				if (!Accept(*newclient, (SOCKADDR*)&SockAddr, &iSockAddrLen)){
					newclient->Safe_Delete();
					DWORD nError = GetLastError();
					if (nError == WSAEWOULDBLOCK){
						DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says WSAEWOULDBLOCK - setting counter to zero!"), __FUNCTION__, m_nPendingConnections);
						m_nPendingConnections = 0;
						break;
					}
					else{
						DebugLogError(LOG_STATUSBAR, _T("%hs: Backlogcounter says %u connections waiting, Accept() says %s - setting counter to zero!"), __FUNCTION__, m_nPendingConnections, GetErrorMessage(nError, 1));
						nFataErrors++;
					}
					if (nFataErrors > 10){
						// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
						// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
						// this should basically never happen anyway
						// however if we are in such a position, try to reinitalize the socket.
						DebugLogError(LOG_STATUSBAR, _T("%hs: Accept() Error Loop, recreating socket"), __FUNCTION__);
						Close();
						StartListening();
						m_nPendingConnections = 0;
						break;
					}
					continue;
				}

				AddConnection();

				// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
				newclient->SetMSSFromSocket(newclient->GetSocketHandle());

				if (SockAddr.sin_addr.S_un.S_addr == 0) // for safety..
				{
					iSockAddrLen = sizeof SockAddr;
					newclient->GetPeerName((SOCKADDR*)&SockAddr, &iSockAddrLen);
					DebugLogWarning(_T("SockAddr.sin_addr.S_un.S_addr == 0;  GetPeerName returned %s"), ipstr(SockAddr.sin_addr.S_un.S_addr));
				}

				ASSERT( SockAddr.sin_addr.S_un.S_addr != 0 && SockAddr.sin_addr.S_un.S_addr != INADDR_NONE );

				if (theApp.ipfilter->IsFiltered(SockAddr.sin_addr.S_un.S_addr)){
					if (thePrefs.GetLogFilteredIPs())
						AddDebugLogLine(false, _T("Rejecting connection attempt (IP=%s) - IP filter"), ipstr(SockAddr.sin_addr.S_un.S_addr));
					newclient->Safe_Delete();
					theStats.filteredclients++;
					continue;
				}

				if (theApp.clientlist->IsBannedClient(SockAddr.sin_addr.S_un.S_addr)){
					if (thePrefs.GetLogBannedClients()){
						CUpDownClient* pClient = theApp.clientlist->FindClientByIP(SockAddr.sin_addr.S_un.S_addr);
						AddDebugLogLine(false, _T("Rejecting connection attempt of banned client %s %s"), ipstr(SockAddr.sin_addr.S_un.S_addr), pClient ? pClient->DbgGetClientInfo() : _T("unknown")); //Xman Code Fix
					}
					newclient->Safe_Delete();
					continue;
				}
			}
			newclient->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);
		}

		ASSERT( m_nPendingConnections >= 0 );
	}
}

//Xman
// Maella -Code Improvement-
void CListenSocket::Process()
{

	// Update counter
	m_OpenSocketsInterval = 0;
	// Check state	
	for (POSITION pos1 = socket_list.GetHeadPosition(), pos2; (pos2 = pos1) != NULL; ){
		CClientReqSocket* cur_sock = socket_list.GetNext(pos1);
		if(cur_sock->deletethis == true){
			//Xman improved socket closing
			if(cur_sock->m_SocketData.hSocket != NULL && cur_sock->m_SocketData.hSocket != INVALID_SOCKET){
				//cur_sock->Close();	// calls 'closesocket'
				cur_sock->CloseSocket();
			}
			//Xman end
			else {
				// Remove instance from the list (Recursive)
				/*
				cur_sock->Delete_Timed();	// may delete 'cur_sock'
				*/	//Enig123::code improvements
				if (cur_sock->Delete_Timed()) {
					socket_list.RemoveAt(pos2);
					delete cur_sock;
				}
			}
		}
		else {
			cur_sock->CheckTimeOut();	// may call 'shutdown'
		}
	}

	if((bListening == false) && 
		(GetOpenSockets()+5 < thePrefs.GetMaxConnections() || theApp.serverconnect->IsConnecting() == true)){
			ReStartListening();
		}
}
// Maella end

void CListenSocket::RecalculateStats()
{
	memset(m_ConnectionStates, 0, sizeof m_ConnectionStates);
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL; )
	{
		switch (socket_list.GetNext(pos)->GetConState())
		{
		case ES_DISCONNECTED:
			m_ConnectionStates[0]++;
			break;
		case ES_NOTCONNECTED:
			m_ConnectionStates[1]++;
			break;
		case ES_CONNECTED:
			m_ConnectionStates[2]++;
			break;
		}
	}
}

void CListenSocket::AddSocket(CClientReqSocket* toadd)
{
	socket_list.AddTail(toadd);
}

//Xman
// - Maella -Code Improvement-
/*//Enig123::code improvements
void CListenSocket::RemoveSocket(CClientReqSocket* todel)
{
	POSITION pos = socket_list.Find(todel);
	if (pos)
		socket_list.RemoveAt(pos);
}
*/
void CListenSocket::KillAllSockets()
{
	while(socket_list.GetHeadPosition() != NULL)
	{
		CClientReqSocket* cur_socket = socket_list.RemoveHead();//Enig123::code improvements
		if (cur_socket->client)
			delete cur_socket->client;
		//else
		//{
		//Xman  Codefix
		//Threadsafe Statechange
		cur_socket->SetConnectedState(ES_DISCONNECTED);
		//Xman end
		//Xman at some system there is a crash in ASyncsocketEx when closing this socket
		//this error occur after the socket is removed from socket-list. a try catch should avoid this crash
		try{
			delete cur_socket;
		}
		catch (...) {
			//nothing todo
		}
		//Xman end
		//}
	}
}
//Xman end

void CListenSocket::AddConnection()
{
	m_OpenSocketsInterval++;
}

bool CListenSocket::TooManySockets(bool bIgnoreInterval)
{
	if (   GetOpenSockets() > thePrefs.GetMaxConnections()
		|| (m_OpenSocketsInterval > (thePrefs.GetMaxConperFive() * GetMaxConperFiveModifier()) && !bIgnoreInterval)
		|| (m_nHalfOpen >= thePrefs.GetMaxHalfConnections() && !bIgnoreInterval))
		return true;
	return false;
}

bool CListenSocket::IsValidSocket(CClientReqSocket* totest)
{
	return socket_list.Find(totest) != NULL;
}

#ifdef _DEBUG
void CListenSocket::Debug_ClientDeleted(CUpDownClient* deleted)
{
	for (POSITION pos = socket_list.GetHeadPosition(); pos != NULL;)
	{
		CClientReqSocket* cur_sock = socket_list.GetNext(pos);
		if (!AfxIsValidAddress(cur_sock, sizeof(CClientReqSocket)))
			AfxDebugBreak();
		if (thePrefs.m_iDbgHeap >= 2)
			ASSERT_VALID(cur_sock);
		if (cur_sock->client == deleted)
			AfxDebugBreak();
	}
}
#endif

void CListenSocket::UpdateConnectionsStatus()
{
	activeconnections = GetOpenSockets();
	// Update statistics for 'peak connections'
	if (peakconnections < activeconnections)
		peakconnections = activeconnections;
	if (peakconnections > thePrefs.GetConnPeakConnections())
		thePrefs.SetConnPeakConnections(peakconnections);

	if (theApp.IsConnected())
	{
		totalconnectionchecks++;
		//Xman : how should this become 0 ?
		/*
		if (totalconnectionchecks == 0) {
			// wrap around occured, avoid division by zero
			totalconnectionchecks = 100;
		}
		*/

		// Get a weight for the 'avg. connections' value. The longer we run the higher 
		// gets the weight (the percent of 'avg. connections' we use).
		float fPercent = (float)(totalconnectionchecks - 1) / (float)totalconnectionchecks;
		if (fPercent > 0.99F)
			fPercent = 0.99F;

		// The longer we run the more we use the 'avg. connections' value and the less we
		// use the 'active connections' value. However, if we are running quite some time
		// without any connections (except the server connection) we will eventually create 
		// a floating point underflow exception.
		averageconnections = averageconnections * fPercent + activeconnections * (1.0F - fPercent);
		if (averageconnections < 0.001F)
			averageconnections = 0.001F;	// avoid floating point underflow
	}
}

//Xman Xtreme Mod
float CListenSocket::GetMaxConperFiveModifier(){

	//Xman if we have reached 96% of max connections --> 33 % of connections per 5
	if ((float)GetOpenSockets() > (0.96f * thePrefs.GetMaxConnections()))
		return 0.33f;
	//Xman if we have reached 88% of max connections --> 60 % of connections per 5
	if ((float)GetOpenSockets() > (0.88f * thePrefs.GetMaxConnections()))
		return 0.6f;
	//Xman if we have reached 80% of max connections --> 80 % of connections per 5
	if ((float)GetOpenSockets() > (0.8f * thePrefs.GetMaxConnections()))
		return 0.8f;

	const float SpikeSize = (float)GetOpenSockets() - (int)averageconnections ;
	if ( SpikeSize < 1.0f )
		return 1.0f;

	// Remark: default factor (XMan 3)2.5, try to increase the number of connections per 5 second
	const float SpikeTolerance = 3.0f * (float)thePrefs.GetMaxConperFive();
	if ( SpikeSize > SpikeTolerance )
		return 0.0f;

	return 1.0f - (SpikeSize/SpikeTolerance);
}
//Xman end