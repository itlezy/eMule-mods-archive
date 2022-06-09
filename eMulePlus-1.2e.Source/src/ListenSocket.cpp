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
#include "emule.h"
#include "ListenSocket.h"
#include "server.h"
#include "opcodes.h"
#include "KnownFile.h"
#include "ServerList.h"
#include "SharedFileList.h"
#include "UploadQueue.h"
#include "updownclient.h"
#include "ClientList.h"
#include "otherfunctions.h"
#include "IPFilter.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#ifdef OLD_SOCKETS_ENABLED

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CClientReqSocket
CClientReqSocket::CClientReqSocket(CUpDownClient *in_client)
{
	m_pClient = in_client;
	if (in_client)
		m_pClient->m_pRequestSocket = this;
	ResetTimeOutTimer();
	m_bDeleteThis = false;
	m_dwDeleteTimer = 0;
	g_App.m_pListenSocket->AddSocket(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClientReqSocket::~CClientReqSocket()
{
	if (m_pClient != NULL)
	{
		m_pClient->SetHandshakeStatus(false);
		m_pClient->m_pRequestSocket = NULL;
		m_pClient = NULL;
	}
	g_App.m_pListenSocket->RemoveSocket(this);

	DEBUG_ONLY (g_App.m_pClientList->Debug_SocketDeleted(this));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::ResetTimeOutTimer()
{
	m_dwTimeoutTimer = ::GetTickCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CheckTimeOut() checks the timeout timer, and, if it has expired, it disconnects the m_pRequestSocket
//	and resets the timeout timer. It returns true if the timer has expired, false if it has not.
bool CClientReqSocket::CheckTimeOut()
{
	uint32	dwCurTicks, dwCurTimeout = CONNECTION_TIMEOUT;

//	Extend timeout for connecting state
	if (m_eConnectionState == ES_CONNECTING)
	{
	//	This socket is still in a half connection state.. Because of SP2, we don't know
	//	if this socket is actually failing, or if this socket is just queued in SP2's new
	//	protection queue. Therefore we give the socket a chance to either finally report
	//	the connection error, or finally make it through SP2's new queued socket system..
		dwCurTimeout = 4 * CONNECTION_TIMEOUT;
	}
	else if (m_pClient != NULL)
	{
	//	Increase socket timeout for DL clients
		if (m_pClient->GetDownloadState() == DS_DOWNLOADING)
			return false;
	//	Extend the timeout to avoid too fast disconnecting of chatting people
		if (m_pClient->GetChatState() == MS_CHATTING)
			dwCurTimeout += 2 * CONNECTION_TIMEOUT;
	}
//	If the timeout period has been exceeded...
	if (((dwCurTicks = ::GetTickCount()) - m_dwTimeoutTimer) > dwCurTimeout)
	{
		m_dwTimeoutTimer = dwCurTicks;	//	Reset the timeout timer
		Disconnect();
		return true;
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::Close()
{
	if (m_pClient != NULL)
		m_pClient->SetHandshakeStatus(false);

	CAsyncSocketEx::Close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::OnClose(int nErrorCode)
{
	ASSERT (g_App.m_pListenSocket->IsValidSocket(this));
	CEMSocket::OnClose(nErrorCode);
	Disconnect();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::Disconnect()
{
//	Tell the m_pRequestSocket not to receive any more notifications
	AsyncSelect(0);
	m_eConnectionState = ES_DISCONNECTED;
	if (m_pClient != NULL && !g_App.m_pClientList->IsValidClient(m_pClient))
	{
		AddLogLine(LOG_FL_DBG, _T("Invalid client '%s'"), m_pClient->GetUserName());
		m_pClient->SetHandshakeStatus(false);
		m_pClient = NULL;
	}
	if (m_pClient == NULL)
		Safe_Delete();
	else
		m_pClient->Disconnected();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::TimedDelete()
{
//	It seems that MFC Sockets call socketfunctions after they are deleted, even if the m_pRequestSocket is closed
//	and select(0) is set. So we need to wait some time to make sure this doesn't happen.
	if (::GetTickCount() - m_dwDeleteTimer > 10000)
		delete this;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::Safe_Delete()
{
	ASSERT (g_App.m_pListenSocket->IsValidSocket(this));
	AsyncSelect(0);
	m_eConnectionState = ES_DISCONNECTED;
	m_dwDeleteTimer = ::GetTickCount();
	if (m_SocketData.hSocket != INVALID_SOCKET)
		ShutDown(2);
	if (m_pClient != NULL)
	{
		m_pClient->SetHandshakeStatus(false);
		m_pClient->m_pRequestSocket = NULL;
		m_pClient = NULL;
	}
	m_bDeleteThis = true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientReqSocket::ProcessPacket(byte *pbytePacket, uint32 dwPacketSize, EnumOpcodes eOpcode)
{
	EnumDLQState eClientDLState;

	try
	{
		try
		{
			if (m_pClient != NULL)
			{
				if (!m_pClient->IsHandshakeFinished() && eOpcode != OP_HELLOANSWER)
					throw CString(_T("a client asks for something without hello-handshake"));
			}
			else if (eOpcode != OP_HELLO)
			{
				throw CString(_T("a client asks for something without saying hello"));
			}

			switch (eOpcode)
			{
				case OP_HELLOANSWER:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
#if 1
					if ((m_pClient != NULL) && m_pClient->IsHandshakeFinished())
						AddLogLine( LOG_FL_DBG | LOG_RGB_ERROR, _T("Wrong HSA status %s"), m_pClient->GetClientNameWithSoftware());
#endif
					m_pClient->ProcessHelloAnswer(pbytePacket, dwPacketSize);
				//	Start secure identification, if
				//	- we have received OP_EMULEINFO and OP_HELLOANSWER (old eMule)
				//	- we have received eMule-OP_HELLOANSWER (new eMule)
					if ((m_pClient != NULL) && (m_pClient->GetInfoPacketsReceived() == IP_BOTH))
						m_pClient->InfoPacketsReceived();
					if (m_pClient != NULL)
					{
						m_pClient->ConnectionEstablished();
					//	Update of client in Clientlist will lead to GUI update in ClientListCtrl as well as DownloadListCtrl
						g_App.m_pClientList->UpdateClient(m_pClient);
					}
					break;
				}
				case OP_HELLO:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
					bool bIsMuleHello, bNewClient = false;
					if (!m_pClient)
					{
					//	Create new client to save standard informations
						m_pClient = new CUpDownClient(this);
						bNewClient = true;
					}

					try
					{
						bIsMuleHello = m_pClient->ProcessHelloPacket(pbytePacket, dwPacketSize);
					}
					catch(...)
					{
						if (bNewClient)
							safe_delete(m_pClient);
						throw CString(_T("unable to process HelloPacket"));
					}
				//	Now we check if we know this client already. If yes this m_pRequestSocket will be attached
				//	to the known client, the new client will be deleted and the var. "client" will point
				//	to the known client. If not we keep our new-constructed client.
					if (g_App.m_pClientList->AttachToAlreadyKnown(&m_pClient, this))
					{
					//	Update the old client informations
						bIsMuleHello = m_pClient->ProcessHelloPacket(pbytePacket, dwPacketSize);
						m_pClient->DisableL2HAC();
						bNewClient = false;
					}
					else
					{
					//	If new client was not created delete him
						if (!g_App.m_pClientList->AddClient(m_pClient))
						{
							safe_delete(m_pClient);
							throw CString(_T("unable to add client in ClientList"));
						}
						m_pClient->SetCommentDirty();
					}

				//	Send a response packet with standard information
					if ((m_pClient->GetHashType() == SO_EMULE) && !bIsMuleHello)
						m_pClient->SendMuleInfoPacket(false);
#if 1
					if ((m_pClient != NULL) && m_pClient->IsHandshakeFinished())
						AddLogLine( LOG_FL_DBG | LOG_RGB_ERROR, _T("Wrong HS status %s"), m_pClient->GetClientNameWithSoftware());
#endif

					m_pClient->SendHelloAnswer();
					if (m_pClient != NULL)
						m_pClient->ConnectionEstablished();
					if (m_pClient != NULL)
					{
					//	Start secure identification, if
					//	- we have received eMule-OP_HELLO (new eMule)
						if (m_pClient->GetInfoPacketsReceived() == IP_BOTH)
							m_pClient->InfoPacketsReceived();

					//	Update of client in Clientlist will lead to GUI update in ClientListCtrl as well as DownloadListCtrl
						if (!bNewClient)
							g_App.m_pClientList->UpdateClient(m_pClient);
					}
					break;
				}
				case OP_REQUESTFILENAME:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);

				//	IP banned, no answer for this request
					if (m_pClient->IsBanned())
						break;

				//	If we're filtering scanning (and misbehaving) clients and this client has asked for a
				//	non-existant file more than three times...
					if (g_App.m_pPrefs->IsScanFilterEnabled() && m_pClient->GetFailedFileRequests() >= 3)
					{
					//	Temporarily ban the client from the upload queue
						g_App.m_pIPFilter->AddTemporaryBannedIP(m_pClient->GetIP());
					//	If we're logging countermeasures...
						if (!g_App.m_pPrefs->IsCMNotLog())
							AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Client %s (IP: %s) added to filtered clients due to file scanning"),
											 m_pClient->GetClientNameWithSoftware(), m_pClient->GetFullIP() );
						Disconnect();
						break;
					}

				//	Since we don't process extended information it is better for compatibility with
				//	the rest of the network to always process a filehash & extended information
				//	only if the file exists and was activated by tag ET_EXTENDEDREQUEST
					if (dwPacketSize >= 16)
					{
						CKnownFile	*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pbytePacket);

					//	If we are not sharing the requested file...
						if (pKnownFile == NULL)
						{
						//	If we've just started a download we may want to use that client as a source.
						//	Moreover we need to check if the requested file exists in the DL queue
						//	in order to filter the clients correct way.

						//	Look for the requested file in the download queue
							CPartFile		*pPartFile = g_App.m_pDownloadQueue->GetFileByID(pbytePacket);

						//	If we found the file and it isn't completed...
							if (pPartFile != NULL && pPartFile->IsPartFile())
							{
							//	... and we don't have enough sources already
								if (g_App.m_pPrefs->GetMaxSourcePerFile() > pPartFile->GetSourceCount())
								{
									g_App.m_pDownloadQueue->CheckAndAddKnownSource(pPartFile, m_pClient);
								}
							}
						//	If we're not currently downloading the requested file...
							else
								m_pClient->UpdateFailedFileRequests();

						//	If the client is using our user hash (hash stealer)
							if (md4cmp(g_App.m_pPrefs->GetUserHash(), m_pClient->GetUserHash()) == 0)
								AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("Client %s (IP: %s) is using our userhash"), m_pClient->GetClientNameWithSoftware(), m_pClient->GetFullIP());
						}
						else	//	If we are sharing the requested file...
						{
							if (pKnownFile->IsLargeFile() && !m_pClient->SupportsLargeFiles())
							{
								AddLogLine( LOG_FL_DBG | LOG_RGB_WARNING, _T("Client '%s' (%s) without 64bit file support requested large file '%s'"),
									m_pClient->GetClientNameWithSoftware(), m_pClient->GetFullIP(), pKnownFile->GetFileName() );
								break;
							}

						//	If this client is requesting this file for the first time
							if (md4cmp(m_pClient->m_reqFileHash, pbytePacket) != 0)
								m_pClient->SetCommentDirty();
						//	Send filename etc
							m_pClient->ResetFailedFileRequests();
							m_pClient->SetUploadFileID(pbytePacket);
							if (!m_pClient->ProcessExtendedInfo(pbytePacket, dwPacketSize, pKnownFile))
							{
								m_pClient->SetWrongFileRequest();

							//	Reply that requested file does not exist
								Packet *replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
								md4cpy(replypacket->m_pcBuffer, pbytePacket);
								g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(replypacket->m_dwSize);
								SendPacket(replypacket, true);
#if 0
								AddLogLine( LOG_FL_DBG | LOG_RGB_WARNING, _T("Mismatched part count on file request '%s' by %s (%s)"),
												pKnownFile->GetFileName(), m_pClient->GetClientNameWithSoftware(), m_pClient->GetFullIP() );
#endif
								break;
							}
							m_pClient->ResetWrongFileRequest();

						//	... and we're downloading the file... (this could be a new source)
							if (pKnownFile->IsPartFile())
							{
							//	... and we don't have enough sources already
								if (g_App.m_pPrefs->GetMaxSourcePerFile() > static_cast<CPartFile*>(pKnownFile)->GetSourceCount())
								{
									g_App.m_pDownloadQueue->CheckAndAddKnownSource((CPartFile*)pKnownFile, m_pClient);
								}
							}

							CSafeMemFile		packetStream(128);
							CStringA			strEncoded;
							uint16				uNameLength;

							packetStream.Write(pKnownFile->GetFileHash(), 16);

							uNameLength = static_cast<uint16>(Str2MB(m_pClient->GetStrCodingFormat(), &strEncoded, pKnownFile->GetFileName()));
							packetStream.Write(&uNameLength, 2);
							packetStream.Write(strEncoded, uNameLength);

							Packet		*pReplyPacket = new Packet(&packetStream);

							pReplyPacket->m_eOpcode = OP_REQFILENAMEANSWER;
							g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pReplyPacket->m_dwSize);
							SendPacket(pReplyPacket, true);
							m_pClient->SendCommentInfo(pKnownFile);
						}
					}
				//	If the packet is too small (malformed)...
					else
					{
						m_pClient->UpdateFailedFileRequests();
						throw CString(_T("invalid packet size"));
					}
					break;
				}
				case OP_FILEREQANSNOFIL:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					if (dwPacketSize != 16)
						throw CString(_T("invalid packet size"));

				//	If that client doesn't have my file maybe he has another one
					CPartFile	*pPartFile = g_App.m_pDownloadQueue->GetFileByID(pbytePacket);
					if (pPartFile == NULL)
						break;

				//	Do nothing if no file was requested
					if ((m_pClient == NULL) || (m_pClient->m_pReqPartFile == NULL))
						break;

				//	We try to swap to another file ignoring no needed parts files
					switch (m_pClient->GetDownloadState())
					{
						case DS_CONNECTED:
						case DS_ONQUEUE:
						case DS_NONEEDEDPARTS:
						case DS_LOWID_ON_OTHER_SERVER:
						case DS_LOWTOLOWID:
						{
						//	try to switch the sources in case of A4AF
							if (!m_pClient->SwapToAnotherFile(NULL, A4AF_REMOVE))
								g_App.m_pDownloadQueue->RemoveSource(m_pClient, true);
							else
								g_App.m_pDownloadList->RemoveSource(m_pClient, pPartFile);
							break;
						}
					}
					break;
				}
				case OP_REQFILENAMEANSWER:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);

					CSafeMemFile	packetStream(pbytePacket, dwPacketSize);

					m_pClient->ProcessFileHash(packetStream);
					m_pClient->ProcessFileInfo(packetStream);
					break;
				}
				case OP_FILESTATUS:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);

					CSafeMemFile	packetStream(pbytePacket, dwPacketSize);

					m_pClient->ProcessFileHash(packetStream);
					m_pClient->ProcessFileStatus(packetStream);
					break;
				}
				case OP_STARTUPLOADREQ:
				// The old eMule clients (<0.26) as well as original eDonkey don't send a hash (dwPacketSize is 0)
				// so we will add client to UL-queue anyway, but inside AddClientToQueue() will be checked
				// if any file was requested before.
				{
					CKnownFile	*pKnownFile = NULL;

				//	count only real upload request
					m_pClient->SetLastUpRequest();
					m_pClient->AddAskedCount();

					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					if (dwPacketSize == 16)
					{
					//	Check if file exits
						pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pbytePacket);
						if (pKnownFile != NULL)
						{
						//	If we are downloading this file, this could be a new source
							if (pKnownFile->IsPartFile())
								if (g_App.m_pPrefs->GetMaxSourcePerFile() > ((CPartFile*)pKnownFile)->GetSourceCount()) //<<--
									g_App.m_pDownloadQueue->CheckAndAddKnownSource((CPartFile*)pKnownFile, m_pClient);

						//	Check to see if this is a new file they are asking for
							if (md4cmp(m_pClient->GetUploadFileID(), pbytePacket) != 0)
								m_pClient->SetCommentDirty();

							m_pClient->ResetFailedFileRequests();
							m_pClient->SetUploadFileID(pbytePacket);
							m_pClient->SendCommentInfo(pKnownFile);
						}
						else
						{
						//	If client still asking for not existing, he is definitely violating eDonkey protocol
							if (!g_App.m_pDownloadQueue->GetFileByID(pbytePacket))
								m_pClient->UpdateFailedFileRequests();

						//	Reply that requested file does not exist
							Packet *replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->m_pcBuffer, pbytePacket);
							g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(replypacket->m_dwSize);
							SendPacket(replypacket, true);
							break;
						}
					}
					else
					{
						pKnownFile = g_App.m_pSharedFilesList->GetFileByID((uchar*)m_pClient->m_reqFileHash);
					}
				//	count only real file request
					if (pKnownFile != NULL)
					{
						pKnownFile->statistic.AddRequest();
						if (!m_pClient->IsOnLAN())
						{
							m_pClient->AddRequestCount((uchar*)m_pClient->m_reqFileHash);
							pKnownFile->AddClientToSourceList(m_pClient);
						}

					//	add file to queue
						g_App.m_pUploadQueue->AddClientToWaitingQueue(m_pClient);

					//	Send a ban message if client tries to enter into the waiting queue
						if (m_pClient->IsBanned() && g_App.m_pPrefs->IsBanMessageEnabled())
							m_pClient->SendBanMessage();
					}
					break;
				}
				case OP_QUEUERANK:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					uint32 rank = PEEK_DWORD(pbytePacket);
					m_pClient->SetRemoteQueueRank(static_cast<uint16>(rank));
					break;
				}
				case OP_ACCEPTUPLOADREQ:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					if (m_pClient->m_pReqPartFile && g_App.m_pDownloadQueue->IsInDLQueue(m_pClient->m_pReqPartFile))
					{
						eClientDLState = m_pClient->GetDownloadState();
					//	If the client is in our download queue and the part file is downloading/waiting an not paused
					//	Ask for next block while downloading
						if (eClientDLState == DS_DOWNLOADING)
						{
						//	Sometimes remote client sends us OP_ACCEPTUPLOADREQ during DS_DOWNLOADING state
						//	To prevent desynconization of request we will clear the list of pending blocks
							m_pClient->ClearPendingBlocksList();
							m_pClient->SendBlockRequests();
						}
					//	Change the state & request the blocks in following cases:
					//	1) DS_CONNECTED - local client opened the connection & got the download invitation
					//	2) DS_ONQUEUE, DS_WAIT_FOR_FILE_REQUEST - remote client opened the connection & invited us to the download
					//	3) DS_LOWID_ON_OTHER_SERVER - remote LowID client opened the connection & invited us to the download
						else if ( ( eClientDLState == DS_CONNECTED
									|| eClientDLState == DS_ONQUEUE
									|| eClientDLState == DS_WAIT_FOR_FILE_REQUEST
									|| eClientDLState == DS_LOWID_ON_OTHER_SERVER
									|| eClientDLState == DS_LOWTOLOWID )
								&& !m_pClient->m_pReqPartFile->IsPaused()
								&& (m_pClient->m_pReqPartFile->GetStatus() == PS_READY || m_pClient->m_pReqPartFile->GetStatus() == PS_EMPTY))
						{
							m_pClient->SetDownloadState(DS_DOWNLOADING);
							m_pClient->SetLastDownPartAsked(0xFFFF);
							m_pClient->SendBlockRequests();
						}
						else
						{
							m_pClient->SendCancelTransfer();
						//	Don't change a state for client which doesn't have needed parts in order to prevent faster(additional) file status request
							if (eClientDLState != DS_NONEEDEDPARTS)
								m_pClient->SetDownloadState(DS_ONQUEUE);
						}
					}
					else
					{
					//	As a file was complete it isn't in the download list anymore, but our
					//	client is still in the upload queues of many other sources, thus they
					//	can continue inviting us for download. So if a client is in our upload
					//	queue we need to send cancel to close socket faster and avoid source deletion
					//	to keep its position in the queue. If a source isn't in our upload queue
					//	it was probably a full source, so we don't need it anymore.
						if (!m_pClient->m_pReqPartFile && m_pClient->GetAskedCountDown() == 0)
							throw CString(_T("upload was offered before file was requested"));
						else
						{
						//	Let's assume that nonzero ask count means a source which was in our upload
						//	and download queues (if it's only in download queue, it's deleted on completion)
							if (m_pClient->GetAskedCountDown())
								m_pClient->SendCancelTransfer();
							else
								throw CString(_T("wrong fileID sent (!m_pReqPartFile), file: ") + ((m_pClient->m_pReqPartFile != NULL) ? m_pClient->m_pReqPartFile->GetFileName() : _T("NULL")));
						}
					}
					break;
				}
				case OP_REQUESTPARTS:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);

					if (!CheckUploadStateForRequestParts())
						break;

					if (dwPacketSize >= 40)	// 16+(3*4)+(3*4)
					{
						Requested_Block_Struct	reqBlock;
						uint32	dwBeg, dwEnd;

						md4cpy(reqBlock.m_fileHash, pbytePacket);

						dwBeg = PEEK_DWORD(pbytePacket + 16);
						dwEnd = PEEK_DWORD(pbytePacket + 16 + 3*4);
						if (dwEnd > dwBeg)
						{
							reqBlock.qwStartOffset = static_cast<uint64>(dwBeg);
							reqBlock.qwEndOffset = static_cast<uint64>(dwEnd);
							m_pClient->AddReqBlock(&reqBlock);
						}

						dwBeg = PEEK_DWORD(pbytePacket + 16 + 4);
						dwEnd = PEEK_DWORD(pbytePacket + 16 + 3*4 + 4);
						if (dwEnd > dwBeg)
						{
							reqBlock.qwStartOffset = static_cast<uint64>(dwBeg);
							reqBlock.qwEndOffset = static_cast<uint64>(dwEnd);
							m_pClient->AddReqBlock(&reqBlock);
						}

						dwBeg = PEEK_DWORD(pbytePacket + 16 + 2*4);
						dwEnd = PEEK_DWORD(pbytePacket + 16 + 3*4 + 2*4);
						if (dwEnd > dwBeg)
						{
							reqBlock.qwStartOffset = static_cast<uint64>(dwBeg);
							reqBlock.qwEndOffset = static_cast<uint64>(dwEnd);
							m_pClient->AddReqBlock(&reqBlock);
						}
					}
					break;
				}
				case OP_CANCELTRANSFER:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					g_App.m_pUploadQueue->RemoveFromUploadQueue(m_pClient, ETS_CANCELED);
					if (g_App.m_pPrefs->IsClientTransferLogEnabled())
						g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T("Client %s: upload session ended due to a cancelled transfer"), m_pClient->GetClientNameWithSoftware());
					m_pClient->SetUploadFileID(NULL);
					break;
				}
				case OP_END_OF_DOWNLOAD:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					if (dwPacketSize >= 16 && !md4cmp(m_pClient->GetUploadFileID(), pbytePacket))
					{
						g_App.m_pUploadQueue->RemoveFromUploadQueue(m_pClient, ETS_END_OF_DOWNLOAD);
						if (g_App.m_pPrefs->IsClientTransferLogEnabled())
							AddLogLine(LOG_FL_DBG, _T("Client %s: upload session ended due to a completed transfer"), m_pClient->GetClientNameWithSoftware());
						m_pClient->SetUploadFileID(NULL);
					}
					break;
				}
				case OP_HASHSETREQUEST:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					if (dwPacketSize != 16)
						throw CString(_T("invalid packet size"));
					m_pClient->SendHashsetPacket(pbytePacket);
					break;
				}
				case OP_HASHSETANSWER:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					m_pClient->ProcessHashSet(pbytePacket, dwPacketSize);
					break;
				}
				case OP_SENDINGPART:
				{
					m_pClient->UpdateLastBlockReceivedTime();
				//	Before we process the packet we need to check the client's download state.
				//	If the source sends us a packet while we're in an unwanted state (we
				//	haven't made a request) we switch him to an error state
					eClientDLState = m_pClient->GetDownloadState();
				//	If we're uploading to this client
					if (eClientDLState == DS_DOWNLOADING)
					{
						EnumPartFileStatuses	eFileStatus;

					//	If we have made a part file request of this client and the requested file is Waiting...
						if ( (m_pClient->m_pReqPartFile != NULL) &&
							(((eFileStatus = m_pClient->m_pReqPartFile->GetStatus()) == PS_READY) || (eFileStatus == PS_EMPTY)) )
						{
							g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(24);
							if (m_pClient->ProcessBlockPacket(pbytePacket, dwPacketSize, false, false) == 0)
							{
							//	If the file is now in a stopped state (paused/stopped/error)
								if ( (m_pClient->m_pReqPartFile != NULL) &&
									( ((eFileStatus = m_pClient->m_pReqPartFile->GetStatus()) == PS_PAUSED) ||
									(eFileStatus == PS_STOPPED) || (eFileStatus == PS_ERROR) ) )
								{
									m_pClient->SendCancelTransfer();
									m_pClient->SetDownloadState((m_pClient->m_pReqPartFile->IsStopped()) ? DS_NONE : DS_ONQUEUE);
								}
							}
							else
							{
							//	Client deletion was requested
								delete m_pClient;
								m_pClient = NULL;
							}
						}
					//	If we didn't want this file (or don't now)
						else
						{
							g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
							m_pClient->SendCancelTransfer();
							m_pClient->SetDownloadState(((m_pClient->m_pReqPartFile == NULL) || m_pClient->m_pReqPartFile->IsStopped()) ? DS_NONE : DS_ONQUEUE);
						}
					}
					else
					{
						g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					//	After we stop downloading by some reason a client still can continue sending
					//	data for a while -- don't disconnect it with error not to lose the source
						if (!m_pClient->WasCancelTransferSent())
						{
							m_pClient->SetDownloadState(DS_ERROR);
							m_pClient->Disconnected();
						}
					}
					break;
				}
				case OP_OUTOFPARTREQS:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					if (m_pClient->GetDownloadState() == DS_DOWNLOADING)
					{
						m_pClient->UpdateOnqueueDownloadState();
					}
					break;
				}
				case OP_SETREQFILEID:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);

					if (m_pClient->GetFailedFileRequests() >= 3 && g_App.m_pPrefs->IsScanFilterEnabled())
					{
						g_App.m_pIPFilter->AddTemporaryBannedIP(m_pClient->GetIP());
						if (!g_App.m_pPrefs->IsCMNotLog())
							AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Client '%s' (%s) added to filtered clients due to file scanning"),
											 m_pClient->GetFullIP(), m_pClient->GetFullSoftVersionString() );
						Disconnect();
						break;
					}

					if (dwPacketSize == 16)
					{
						if (m_pClient->IsWrongFileRequest())
						{
					//	Don't reply to the source after mismatch file request was detected
							Disconnect();
							break;
						}

						CKnownFile	*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pbytePacket);

						if (pKnownFile == NULL)
						{
						//	If we just started a download we may want to use that client as a source.
						//  Moreover we need to check if the requested file exists in the DL queue
						//  in order to filter the clients the correct way.
							CPartFile* pPartFile = g_App.m_pDownloadQueue->GetFileByID(pbytePacket);
							if (pPartFile && pPartFile->IsPartFile())
							{
								if (g_App.m_pPrefs->GetMaxSourcePerFile() > pPartFile->GetSourceCount()) //<<--
									g_App.m_pDownloadQueue->CheckAndAddKnownSource(pPartFile, m_pClient);
							}
							else
								m_pClient->UpdateFailedFileRequests();

						//	Send file request no such file
							Packet* replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->m_pcBuffer, pbytePacket);
							g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(replypacket->m_dwSize);
							SendPacket(replypacket, true);
							break;
						}

						if (pKnownFile->IsLargeFile() && !m_pClient->SupportsLargeFiles())
						{
							AddLogLine( LOG_FL_DBG | LOG_RGB_WARNING, _T("Client '%s' (%s) without 64bit file support requested large file '%s'"),
								m_pClient->GetClientNameWithSoftware(), m_pClient->GetFullIP(), pKnownFile->GetFileName() );
							Packet *replypacket = new Packet(OP_FILEREQANSNOFIL, 16);
							md4cpy(replypacket->m_pcBuffer, pbytePacket);
							g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(replypacket->m_dwSize);
							SendPacket(replypacket, true);
							break;
						}

					//	Reset Failed Request Counter
						m_pClient->ResetFailedFileRequests();
					//	If we are downloading this file, this could be a new source
						if (pKnownFile->IsPartFile())
							if (g_App.m_pPrefs->GetMaxSourcePerFile() > ((CPartFile*)pKnownFile)->GetSourceCount())
								g_App.m_pDownloadQueue->CheckAndAddKnownSource((CPartFile*)pKnownFile, m_pClient);

					//	Check to see if this is a new file they are asking for
						if (md4cmp(m_pClient->GetUploadFileID(), pbytePacket) != 0)
							m_pClient->SetCommentDirty();

					//	Send filestatus
						m_pClient->SetUploadFileID(pbytePacket);

						CMemFile packetStream(16 + 2 + 58);	// enough for 4GB without reallocation

						packetStream.Write(pKnownFile->GetFileHash(), 16);
						if (pKnownFile->IsPartFile())
							((CPartFile*)pKnownFile)->WritePartStatus(&packetStream);
						else if (pKnownFile->GetJumpstartEnabled())
							pKnownFile->WriteJumpstartPartStatus(m_pClient, &packetStream);
						else if (pKnownFile->HasHiddenParts())
							pKnownFile->WritePartStatus(&packetStream);
						else
						{
							uint32 null = 0;
							packetStream.Write(&null, 2);
						}

						Packet	*replypacket = new Packet(&packetStream);
						replypacket->m_eOpcode = OP_FILESTATUS;
						g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(replypacket->m_dwSize);
						SendPacket(replypacket, true);
					}
					else
						throw CString(_T("invalid packet size"));
					break;
				}
				case OP_CHANGE_CLIENT_ID:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
					CSafeMemFile packetStream(pbytePacket, dwPacketSize);
					uint32 nNewUserID;
					uint32 nNewServerIP;

					packetStream.Read(&nNewUserID, 4);
					packetStream.Read(&nNewServerIP, 4);
				//	Client changed server and got a LowID
					if (IsLowID(nNewUserID))
					{
						CServer * pNewServer = g_App.m_pServerList->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
						// 	Update UserID only if we know the server
							m_pClient->SetUserIDHybrid(nNewUserID);
							m_pClient->SetServerIP(nNewServerIP);
							m_pClient->SetServerPort(pNewServer->GetPort());
						}
					}
				//	Client changed server and got a HighID(IP)
					else if (nNewUserID == m_pClient->GetIP())
					{
						m_pClient->SetUserIDHybrid(ntohl(nNewUserID));
						CServer* pNewServer = g_App.m_pServerList->GetServerByIP(nNewServerIP);
						if (pNewServer != NULL)
						{
							m_pClient->SetServerIP(nNewServerIP);
							m_pClient->SetServerPort(pNewServer->GetPort());
						}
					}
					break;
				}
				case OP_CHANGE_SLOT:	//	Sometimes sent by Hybrid
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwPacketSize);
					break;
				}
				case OP_MESSAGE:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);

					uint32	dwMsgLen;

					for(;;)
					{
					//	Verify the packet size
						if (dwPacketSize >= 2)
						{
							dwMsgLen = PEEK_WORD(pbytePacket);
							if ((dwMsgLen + 2) == dwPacketSize)
								break;
						}
						throw CString(_T("invalid message packet"));
					}
					if (dwMsgLen > MAX_CLIENT_MSG_LEN)
						dwMsgLen = MAX_CLIENT_MSG_LEN;

					CString		strMessage;

					MB2Str(m_pClient->GetStrCodingFormat(), &strMessage, reinterpret_cast<char*>(pbytePacket + 2), dwMsgLen);
					g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.ProcessMessage(m_pClient, strMessage);
					break;
				}
				case OP_ASKSHAREDFILES:
				{
				//	Client wants to know what we have in share, let's see if we allow him to know that
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);

				//	IP banned, no answer for this request
					if (m_pClient->IsBanned())
						break;

					CPtrList	list;

					if ( g_App.m_pPrefs->CanSeeShares() == SEE_SHARE_EVERYBODY
					  || (g_App.m_pPrefs->CanSeeShares() == SEE_SHARE_FRIENDS && m_pClient->IsFriend()) )
					{
						CCKey		bufKey;
						CKnownFile	*cur_file;

						for (POSITION pos = g_App.m_pSharedFilesList->m_mapSharedFiles.GetStartPosition(); pos != NULL;)
						{
							g_App.m_pSharedFilesList->m_mapSharedFiles.GetNextAssoc(pos, bufKey, cur_file);

							if ( ( (cur_file->GetPermissions() == PERM_ALL) ||
								((cur_file->GetPermissions() == PERM_FRIENDS) && m_pClient->IsFriend()) ) &&
								(!cur_file->IsLargeFile() || m_pClient->SupportsLargeFiles()) )
							{
								list.AddTail((void*&)cur_file);
							}
						}
						AddLogLine( LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_REQ_SHAREDFILES,
										 m_pClient->GetClientNameWithSoftware(), GetResString(IDS_ACCEPTED) );
					}
					else
					{
						AddLogLine( LOG_FL_SBAR | LOG_RGB_WARNING, IDS_REQ_SHAREDFILES,
										 m_pClient->GetClientNameWithSoftware(), GetResString(IDS_DENIED) );
					}

				//	Build the reply packet and send it.
					uint32			dwNumFiles = list.GetCount();
					CSafeMemFile	packetStream(256);

					packetStream.Write(&dwNumFiles, 4);
					while (list.GetCount())
					{
						g_App.m_pSharedFilesList->WriteToOfferedFilePacket(reinterpret_cast<CKnownFile*>(list.GetHead()), packetStream, NULL, m_pClient);
						list.RemoveHead();
					}

					Packet		*pReplyPacket = new Packet(&packetStream);

					pReplyPacket->m_eOpcode = OP_ASKSHAREDFILESANSWER;
					g_App.m_pUploadQueue->AddUpDataOverheadOther(pReplyPacket->m_dwSize);
					SendPacket(pReplyPacket, true, true);
					break;
				}
				case OP_ASKSHAREDFILESANSWER:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
					m_pClient->ProcessSharedFileList(pbytePacket, dwPacketSize);
					break;
				}
				case OP_ASKSHAREDDIRS:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
					ASSERT(dwPacketSize == 0);

				//	IP banned, no answer for this request
					if (m_pClient->IsBanned())
					{
						break;
					}

					if ( g_App.m_pPrefs->CanSeeShares() == SEE_SHARE_EVERYBODY
					  || (g_App.m_pPrefs->CanSeeShares() == SEE_SHARE_FRIENDS && m_pClient->IsFriend()) )
					{
						AddLogLine( LOG_FL_SBAR | LOG_RGB_NOTICE, IDS_SHAREDREQ1, m_pClient->GetClientNameWithSoftware(),
							GetResString(IDS_ACCEPTED) );

						CString		strDir, strTest;

					//	Virtual folder list
						CArray<CString, CString&>	astrFolders;
						CMapStringToString			&mapVDirs = g_App.m_pSharedFilesList->GetSharedVDirForList();
						POSITION					pos = mapVDirs.GetStartPosition();

						while (pos)
						{
							CString		strKey, strDir;

							mapVDirs.GetNextAssoc(pos, strKey, strDir);

							int			i = 0;

							while ((i < astrFolders.GetCount()) && (strKey.CompareNoCase(astrFolders.GetAt(i)) > 0))
								i++;
							astrFolders.InsertAt(i, strKey);
						}

					//	Build the reply packet
						CSafeMemFile		packetStream(80);
						uint32				dwNumDirs = astrFolders.GetCount();

						packetStream.Write(&dwNumDirs, 4);
					//	For each directory in the array...
						for (int ix = 0; ix < astrFolders.GetCount(); ix++)
						{
							const CString		&strDir = astrFolders.GetAt(ix);
							uint16				uCnt = static_cast<uint16>(strDir.GetLength());

							packetStream.Write(&uCnt, 2);
							WriteStr2MB(m_pClient->GetStrCodingFormat(), strDir, packetStream);
						}

						Packet		*pReplyPacket = new Packet(&packetStream);

						pReplyPacket->m_eOpcode = OP_ASKSHAREDDIRSANS;
						g_App.m_pUploadQueue->AddUpDataOverheadOther(pReplyPacket->m_dwSize);
						SendPacket(pReplyPacket, true, true);
					}
					else
					{
						AddLogLine( LOG_FL_SBAR | LOG_RGB_WARNING, IDS_SHAREDREQ1, m_pClient->GetClientNameWithSoftware(),
							GetResString(IDS_DENIED) );

						Packet		*pReplyPacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);

						g_App.m_pUploadQueue->AddUpDataOverheadOther(pReplyPacket->m_dwSize);
						SendPacket(pReplyPacket, true, true);
					}
					break;
				}
				case OP_ASKSHAREDFILESDIR:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);

				//	IP banned, no answer for this request
					if (m_pClient->IsBanned())
						break;

					CSafeMemFile		packetStream(pbytePacket, dwPacketSize);

					uint16		uDirNameLen;
					packetStream.Read(&uDirNameLen, 2);

					CString		strReqDir, strReqVDir;
					ReadMB2Str(m_pClient->GetStrCodingFormat(), &strReqVDir, packetStream, uDirNameLen);
					PathRemoveBackslash(strReqVDir.GetBuffer());
					strReqVDir.ReleaseBuffer();

				//	Retrieve real dir from virtual dir
					CMapStringToString		&mapVDirs = g_App.m_pSharedFilesList->GetSharedVDirForList();
					BOOL		bReqVDir = mapVDirs.Lookup(strReqVDir, strReqDir);

					if ( (bReqVDir)
					  && ( g_App.m_pPrefs->CanSeeShares() == SEE_SHARE_EVERYBODY
					    || (g_App.m_pPrefs->CanSeeShares() == SEE_SHARE_FRIENDS && m_pClient->IsFriend()) ) )
					{
						AddLogLine( LOG_FL_DBG, _T("Client %s requested your list of shared files for directory '%s' ['%s'] (accepted)"),
										 m_pClient->GetClientNameWithSoftware(), strReqVDir, strReqDir );
						ASSERT(packetStream.GetPosition() == packetStream.GetLength());

						CCKey		bufKey;
						CKnownFile	*pKnownFile;

						CTypedPtrList<CPtrList, CKnownFile*>	list;

						for (POSITION pos = g_App.m_pSharedFilesList->m_mapSharedFiles.GetStartPosition(); pos != NULL;)
						{
							g_App.m_pSharedFilesList->m_mapSharedFiles.GetNextAssoc(pos, bufKey, pKnownFile);

							CString		strSharedFileDir(pKnownFile->GetPath());

							PathRemoveBackslash(strSharedFileDir.GetBuffer());
							strSharedFileDir.ReleaseBuffer();
							if ( (strReqDir.CompareNoCase(strSharedFileDir) == 0) &&
								((pKnownFile->GetPermissions() == PERM_ALL) || ((pKnownFile->GetPermissions() == PERM_FRIENDS) && m_pClient->IsFriend())) &&
								(!pKnownFile->IsLargeFile() || m_pClient->SupportsLargeFiles()) )
							{
								list.AddTail(pKnownFile);
							}
						}

					//	Currently we are sending each shared directory, even if it does not contain any files.
					//	Because of this we also have to send an empty shared files list..
						CSafeMemFile		packetStream(256);

						uint16		uDirNameLen = static_cast<uint16>(strReqVDir.GetLength());
						packetStream.Write(&uDirNameLen, 2);

						WriteStr2MB(m_pClient->GetStrCodingFormat(), strReqVDir, packetStream);

						uint32		dwNumFiles = list.GetCount();
						packetStream.Write(&dwNumFiles, 4);

						while (list.GetCount())
						{
							g_App.m_pSharedFilesList->WriteToOfferedFilePacket(list.GetHead(), packetStream, NULL, m_pClient);
							list.RemoveHead();
						}

						Packet		*pReplyPacket = new Packet(&packetStream);

						pReplyPacket->m_eOpcode = OP_ASKSHAREDFILESDIRANS;
						g_App.m_pUploadQueue->AddUpDataOverheadOther(pReplyPacket->m_dwSize);
						SendPacket(pReplyPacket, true, true);
					}
				//	If the requester isn't allowed to see our shares...
					else
					{
						AddLogLine( LOG_FL_DBG, _T("Client %s requested your list of shared files for directory '%s' ['%s'] (denied)"),
										 m_pClient->GetClientNameWithSoftware(), strReqVDir, strReqDir );

						Packet		*pReplyPacket = new Packet(OP_ASKSHAREDDENIEDANS, 0);

						g_App.m_pUploadQueue->AddUpDataOverheadOther(pReplyPacket->m_dwSize);
						SendPacket(pReplyPacket, true, true);
					}
					break;
				}
				case OP_ASKSHAREDDIRSANS:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
					if (m_pClient->GetFileListRequested() == 1)
					{
						CSafeMemFile		packetStream(pbytePacket, dwPacketSize);

						uint32		dwNumDirs;
						packetStream.Read(&dwNumDirs, 4);

						for (UINT i = 0; i < dwNumDirs; i++)
						{
							uint16		uDirNameLen;
							packetStream.Read(&uDirNameLen, 2);

							CStringA		strEncodedDirName;
							packetStream.Read(strEncodedDirName.GetBuffer(uDirNameLen), uDirNameLen);
							strEncodedDirName.ReleaseBuffer(uDirNameLen);

							CMemFile		packetStream2(128);

							uDirNameLen = static_cast<uint16>(strEncodedDirName.GetLength());
							packetStream2.Write(&uDirNameLen, 2);
							packetStream2.Write(strEncodedDirName.GetString(), uDirNameLen);

							Packet		*pReplyPacket = new Packet(&packetStream2);

							pReplyPacket->m_eOpcode = OP_ASKSHAREDFILESDIR;
							g_App.m_pUploadQueue->AddUpDataOverheadOther(pReplyPacket->m_dwSize);
							SendPacket(pReplyPacket, true, true);
						}

						ASSERT(packetStream.GetPosition() == packetStream.GetLength());
						m_pClient->SetFileListRequested(dwNumDirs);
					}
					else
					{
						AddLogLine( LOG_FL_DBG, _T("Client %s sent not requested list of shared directories - ignored"),
										 m_pClient->GetClientNameWithSoftware() );
					}

					break;
				}
				case OP_ASKSHAREDFILESDIRANS:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);

					CSafeMemFile	packetStream(pbytePacket, dwPacketSize);
					uint32			dwPos;

				//	Get the directory name length
					uint16		uDirNameLen;
					packetStream.Read(&uDirNameLen, 2);

				//	Get the directory name
					CString		strDirName;
					ReadMB2Str(m_pClient->GetStrCodingFormat(), &strDirName, packetStream, uDirNameLen);
					PathRemoveBackslash(strDirName.GetBuffer());
					strDirName.ReleaseBuffer();

					if (m_pClient->GetFileListRequested() > 0)
					{
						AddLogLine( LOG_FL_DBG, _T("Client %s sent list of shared files for directory '%s'"),
										 m_pClient->GetClientNameWithSoftware(), strDirName );
						dwPos = static_cast<uint32>(packetStream.GetPosition());
						m_pClient->ProcessSharedFileList(pbytePacket + dwPos, dwPacketSize - dwPos, strDirName);
						if (m_pClient->GetFileListRequested() == 0)
						{
							AddLogLine( LOG_FL_DBG, _T("Client %s finished sending list of shared files"),
											 m_pClient->GetClientNameWithSoftware() );
						}
					}
					else
					{
						AddLogLine( LOG_FL_DBG, _T("Client %s sent not requested list of shared files for directory '%s' - ignored"),
										 m_pClient->GetClientNameWithSoftware(), strDirName);
					}
					break;
				}
				case OP_ASKSHAREDDENIEDANS:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
					ASSERT(dwPacketSize == 0);
					AddLogLine(LOG_FL_SBAR, IDS_SHAREDREQDENIED, m_pClient->GetClientNameWithSoftware());
					m_pClient->SetFileListRequested(0);
					break;
				}
				default:
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwPacketSize);
			}
		}
		catch (CFileException *error)
		{
			OUTPUT_DEBUG_TRACE();
			error->Delete();
			throw CString(_T("invalid or corrupted packet received"));
		}
		catch (CMemoryException *error)
		{
			OUTPUT_DEBUG_TRACE();
			error->Delete();
			throw CString(_T("Memory exception"));
		}
	}
	catch (CString error)
	{
		OUTPUT_DEBUG_TRACE();
		if (m_pClient)
		{
			m_pClient->SetDownloadState(DS_ERROR);
			AddLogLine( LOG_FL_DBG | LOG_RGB_INDIAN_RED, _T("Client %s caused eDonkey packet (%s) processing error: %s. Disconnecting client!"),
				m_pClient->GetClientNameWithSoftware(), DbgGetClientTCPOpcode(false, static_cast<byte>(eOpcode)), error );
		}

		Disconnect();
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientReqSocket::ProcessExtPacket(byte *pbytePacket, uint32 dwSize, EnumOpcodes opcode, uint32 dwRawSize)
{
	try
	{
		try
		{
			if (m_pClient == NULL)
			{
				g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwRawSize);
				throw CString(_T("unknown client sends extended protocol packet"));
			}

			EnumDLQState eClientDLState = m_pClient->GetDownloadState();

			switch (opcode)
			{
				case OP_MULTIPACKETANSWER:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwRawSize);

					CSafeMemFile	packetStream(pbytePacket, dwSize);
					byte			byteOpcode;

					m_pClient->ProcessFileHash(packetStream);

					while ((packetStream.GetLength() - packetStream.GetPosition()) > 0)
					{
						packetStream.Read(&byteOpcode, sizeof(byteOpcode));

						switch (byteOpcode)
						{
							case OP_REQFILENAMEANSWER:
								m_pClient->ProcessFileInfo(packetStream);
								break;

							case OP_FILESTATUS:
								m_pClient->ProcessFileStatus(packetStream);
								break;

							case OP_AICHFILEHASHANS:
#if 1//temporary until it's implemented
								throw CString(_T("received unrequested AICH"));
#endif
								break;

							default:
							{
								CString strError;
								strError.Format(_T("invalid sub opcode %#x received"), byteOpcode);
								throw strError;
							}
						}
					}
					break;
				}
				case OP_EMULEINFO:
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwRawSize);
					m_pClient->ProcessMuleInfoPacket(pbytePacket, dwSize);
				//	Start secure identification, if
				//  - we have received eD2K and eMule info (old eMule)
					if (m_pClient->GetInfoPacketsReceived() == IP_BOTH)
						m_pClient->InfoPacketsReceived();
					m_pClient->SendMuleInfoPacket(true);
					break;

				case OP_EMULEINFOANSWER:
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwRawSize);
					m_pClient->ProcessMuleInfoPacket(pbytePacket, dwSize);
				//	Update of client in the client list
					g_App.m_pClientList->UpdateClient(m_pClient);
				//	Start secure identification, if
				//  - we have received eD2K and eMule info (old eMule)
					if (m_pClient->GetInfoPacketsReceived() == IP_BOTH)
						m_pClient->InfoPacketsReceived();
					break;

				case OP_SECIDENTSTATE:
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwRawSize);
					m_pClient->ProcessSecIdentStatePacket(pbytePacket, dwSize);
					if (m_pClient->GetSecureIdentState() == IS_SIGNATURENEEDED)
					{
						m_pClient->SendSignaturePacket();
					}
					else if (m_pClient->GetSecureIdentState() == IS_KEYANDSIGNEEDED)
					{
						m_pClient->SendPublicKeyPacket();
						m_pClient->SendSignaturePacket();
					}
					break;

				case OP_PUBLICKEY:
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwRawSize);
					if (!m_pClient->IsBanned())
						m_pClient->ProcessPublicKeyPacket(pbytePacket, dwSize);
					break;

				case OP_SIGNATURE:
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwRawSize);
					if (!m_pClient->IsBanned())
						m_pClient->ProcessSignaturePacket(pbytePacket, dwSize);
					break;

				case OP_REQUESTPARTS_I64:
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwRawSize);

					if (!CheckUploadStateForRequestParts())
						break;

					if (dwSize >= 64)	// = 16+(3*8)+(3*8)
					{
						Requested_Block_Struct	reqBlock;

						md4cpy(reqBlock.m_fileHash, pbytePacket);

						reqBlock.qwStartOffset = PEEK_QWORD(pbytePacket + 16);
						reqBlock.qwEndOffset = PEEK_QWORD(pbytePacket + 16 + 3*8);
						if (reqBlock.qwEndOffset > reqBlock.qwStartOffset)
							m_pClient->AddReqBlock(&reqBlock);

						reqBlock.qwStartOffset = PEEK_QWORD(pbytePacket + 16 + 8);
						reqBlock.qwEndOffset = PEEK_QWORD(pbytePacket + 16 + 3*8 + 8);
						if (reqBlock.qwEndOffset > reqBlock.qwStartOffset)
							m_pClient->AddReqBlock(&reqBlock);

						reqBlock.qwStartOffset = PEEK_QWORD(pbytePacket + 16 + 2*8);
						reqBlock.qwEndOffset = PEEK_QWORD(pbytePacket + 16 + 3*8 + 2*8);
						if (reqBlock.qwEndOffset > reqBlock.qwStartOffset)
							m_pClient->AddReqBlock(&reqBlock);
					}
					break;

				case OP_COMPRESSEDPART:
				case OP_SENDINGPART_I64:
				case OP_COMPRESSEDPART_I64:
					m_pClient->UpdateLastBlockReceivedTime();
				//	Before we process a packet we need to check Download State
				//	If client send us a packet in an unwanted state (we don't request (need) this packet)
				//	we will switch him to error state
					if (eClientDLState == DS_DOWNLOADING)
					{
						EnumPartFileStatuses	eFileStatus;

						if ( (m_pClient->m_pReqPartFile != NULL) &&
							(((eFileStatus = m_pClient->m_pReqPartFile->GetStatus()) == PS_READY) || (eFileStatus == PS_EMPTY)) )
						{
							uint32 dwInc = 16 + 4 + 4;	//24

							if (opcode == OP_COMPRESSEDPART_I64)
								dwInc = 16 + 8 + 4;	//28
							else if (opcode == OP_SENDINGPART_I64)
								dwInc = 16 + 8 + 8;	//32
							g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwInc);
							m_pClient->ProcessBlockPacket( pbytePacket, dwSize,
								(opcode != OP_SENDINGPART_I64) /*compressed*/,
								(opcode != OP_COMPRESSEDPART) /*64bit*/ );
							if ( (m_pClient->m_pReqPartFile != NULL) &&
								( ((eFileStatus = m_pClient->m_pReqPartFile->GetStatus()) == PS_PAUSED) ||
								(eFileStatus == PS_STOPPED) || (eFileStatus == PS_ERROR) ) )
							{
								m_pClient->SendCancelTransfer();
								m_pClient->SetDownloadState((m_pClient->m_pReqPartFile->IsStopped()) ? DS_NONE : DS_ONQUEUE);
							}
						}
						else
						{
							g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwRawSize);
							m_pClient->SendCancelTransfer();
							m_pClient->SetDownloadState(((m_pClient->m_pReqPartFile == NULL) || m_pClient->m_pReqPartFile->IsStopped()) ? DS_NONE : DS_ONQUEUE);
						}
					}
					else
					{
						g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwRawSize);
					//	After we stop downloading by some reason a client still can continue sending
					//	data for a while -- don't disconnect it with error not to lose the source
						if (!m_pClient->WasCancelTransferSent())
						{
							m_pClient->SetDownloadState(DS_ERROR);
							m_pClient->Disconnected();
						}
					}
					break;

				case OP_QUEUERANKING:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwRawSize);
					if (dwSize < 2)
						throw CString(_T("invalid size"));

					uint16 newrank = PEEK_WORD(pbytePacket);

					m_pClient->SetRemoteQueueRank(newrank);
				//	Switch DL state in case newrank != 0
					if ((newrank != 0) && (eClientDLState == DS_DOWNLOADING))
						m_pClient->UpdateOnqueueDownloadState();
					break;
				}
				case OP_REQUESTSOURCES:
				case OP_REQUESTSOURCES2:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadSourceExchange(dwRawSize);

					CKnownFile	*pKFile;
					byte	*pbytePktPos = pbytePacket, byteRequestedVer = 0;
					uint16	uRequestedOptions = 0;

					if ((opcode == OP_REQUESTSOURCES2) && (dwSize >= 19))
					{	// SX2 request contains additional data
						byteRequestedVer = *pbytePktPos;
						uRequestedOptions = PEEK_WORD(pbytePktPos + 1);
						pbytePktPos += 3;
					}
				//	Original client don't answer if client's SourceExchange Version is less or equal than 1
				//	but i think we need to keep compatibility with old eMule Plus versions
					if ((byteRequestedVer != 0) || (m_pClient->GetSourceExchange1Version() >= 1))
					{
						if (dwSize < static_cast<uint32>(pbytePktPos - pbytePacket + 16))
							throw CString(_T("invalid size"));

					//	First check shared file list, then download list
						if ( ((pKFile = g_App.m_pSharedFilesList->GetFileByID(pbytePktPos)) != NULL) ||
							((pKFile = g_App.m_pDownloadQueue->GetFileByID(pbytePktPos)) != NULL) )
						{
							DWORD		dwTimePassed = ::GetTickCount() - m_pClient->GetLastSrcReqTime() + CONNECTION_LATENCY;
							bool		bNeverAskedBefore = m_pClient->GetLastSrcReqTime() == 0;

							if (
							//	If not complete and file is rare, allow once every 10 minutes
							    ( pKFile->IsPartFile()
							      && ((CPartFile*)pKFile)->GetSourceCount() <= RARE_FILE * 2
							      && (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASK) )
							//	OR if file is not rare or if file is complete, allow every 90 minutes
							    || (bNeverAskedBefore || dwTimePassed > SOURCECLIENTREASK * MINCOMMONPENALTY) )
							{
								m_pClient->SetLastSrcReqTime();

								Packet	*tosend = pKFile->CreateSrcInfoPacket(m_pClient, byteRequestedVer, uRequestedOptions);

								if (tosend)
								{
									g_App.m_pUploadQueue->AddUpDataOverheadSourceExchange(tosend->m_dwSize);
									SendPacket(tosend, true, true);
								}
							}
						}
					}
					break;
				}
				case OP_ANSWERSOURCES:
				case OP_ANSWERSOURCES2:
				{
					g_App.m_pDownloadQueue->AddDownDataOverheadSourceExchange(dwRawSize);

					CSafeMemFile	packetStream(pbytePacket, dwSize);
					uchar			hash[16];
					byte			byteSXVer;

					if (opcode == OP_ANSWERSOURCES2)
						packetStream.Read(&byteSXVer, 1);
					else
						byteSXVer = m_pClient->GetSourceExchange1Version();
					packetStream.Read(hash, 16);

					CKnownFile		*pKnownFile = g_App.m_pDownloadQueue->GetFileByID(hash);

					if ((pKnownFile != NULL) && pKnownFile->IsPartFile())
					{
					//	Set the client's answer time
						m_pClient->SetLastSrcAnswerTime();
					//	and set the pKnownFile's last answer time
						((CPartFile*)pKnownFile)->SetLastAnsweredTime();
						((CPartFile*)pKnownFile)->AddClientSources(&packetStream, byteSXVer, (opcode == OP_ANSWERSOURCES2), m_pClient);
					}
					break;
				}
				case OP_FILEDESC:
					g_App.m_pDownloadQueue->AddDownDataOverheadFileRequest(dwRawSize);
					m_pClient->ProcessMuleCommentPacket(pbytePacket, dwSize);
					break;

				default:
					g_App.m_pDownloadQueue->AddDownDataOverheadOther(dwRawSize);
			}
		}
		catch (CFileException *error)
		{
			OUTPUT_DEBUG_TRACE();
			error->Delete();
			throw CString(_T("invalid or corrupted packet received"));
		}
		catch(CMemoryException *error)
		{
			OUTPUT_DEBUG_TRACE();
			error->Delete();
			throw CString(_T("Memory exception"));
		}
	}
	catch (CString error)
	{
		OUTPUT_DEBUG_TRACE();
		if (m_pClient == NULL)
			AddLogLine(LOG_FL_DBG, _T("A client caused an error or did something bad: %s. Disconnecting client!"), error);
		else
		{
			AddLogLine( LOG_FL_DBG | LOG_RGB_INDIAN_RED, _T("Client %s caused eMule packet (%s) processing error: %s. Disconnecting client!"),
				m_pClient->GetClientNameWithSoftware(), DbgGetClientTCPOpcode(true, static_cast<byte>(opcode)), error );
			m_pClient->SetDownloadState(DS_ERROR);
		}
		Disconnect();
		return false;
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::OnSend(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnSend(nErrorCode);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::OnError(int nErrorCode)
{
	if (m_pClient != NULL)
		AddLogLine( LOG_FL_DBG, _T("Client %s (IP: %s) caused an error: %u. Disconnecting client!"),
						 m_pClient->GetClientNameWithSoftware(), m_pClient->GetFullIP(), nErrorCode );
	else
		AddLogLine(LOG_FL_DBG, _T("A client caused an error (%u) or did something bad. Disconnecting client!"), nErrorCode);
	Disconnect();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::PacketReceived(Packet* packet)
{
	try
	{
		uint32	dwRawSize = packet->m_dwSize;

		switch (packet->m_byteProtocol)
		{
			case OP_EDONKEYPROT:
				ProcessPacket(reinterpret_cast<byte*>(packet->m_pcBuffer), packet->m_dwSize, packet->m_eOpcode);
				break;
			case OP_PACKEDPROT:
				if (!packet->UnpackPacket())
				{
					ASSERT(false);
					break;
				}
			case OP_EMULEPROT:
				ProcessExtPacket(reinterpret_cast<byte*>(packet->m_pcBuffer), packet->m_dwSize, packet->m_eOpcode, dwRawSize);
				break;
			default:
			//	Lets free the socket from buggy and unknown clients
				if (m_pClient)
					m_pClient->SetDownloadState(DS_ERROR);
				Disconnect();
		}
	}
	catch(...)
	{
		OUTPUT_DEBUG_TRACE();
	//	If we get here, we probably had an access violation
		AddLogLine( LOG_FL_DBG | LOG_RGB_ERROR, _T(__FUNCTION__) _T(": Unknown exception. Protocol=%#02x opcode=%#02x size=%u"),
			(packet != NULL) ? packet->m_byteProtocol : 0, (packet != NULL) ? packet->m_eOpcode : 0, (packet != NULL) ? packet->m_dwSize : 0 );

	//	TODO: This exception handler should definitively be *here*. Though we may get some very
	//	strange socket deletion crashs if we disconnect a client's TCP socket on catching an exception
	//	here. See also comments in 'CAsyncSocketExHelperWindow::WindowProc'
	//	if (m_pClient)
	//		m_pClient->SetDownloadState(DS_ERROR);
	//	Disconnect();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::OnReceive(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnReceive(nErrorCode);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientReqSocket::Create()
{
	g_App.m_pListenSocket->AddConnection();
	return (CAsyncSocketEx::Create(0, SOCK_STREAM, FD_CONNECT | FD_WRITE | FD_READ | FD_CLOSE) != FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::SendPacket(Packet *pPacket, bool bDeletePacket/*=true*/, bool bControlPacket/*=true*/)
{
	ResetTimeOutTimer();
	CEMSocket::SendPacket(pPacket, bDeletePacket, bControlPacket);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientReqSocket::OnConnect(int nErrorCode)
{
	CEMSocket::OnConnect(nErrorCode);

	if (nErrorCode == 0)
	{
	//	Socket may have been delayed by SP2 protection, let's make sure it doesn't timeout instantly
		ResetTimeOutTimer();
	}
	else
		Disconnect();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientReqSocket::CheckUploadStateForRequestParts()
{
//	Ignore block request if remote client doesn't download from us
	if (m_pClient->GetUploadState() != US_UPLOADING)
	{
		m_pClient->AddIncorrectBlockRequest();
		if (m_pClient->GetIncorrectBlockRequests() > 3)
		{
			g_App.m_pIPFilter->AddTemporaryBannedIP(m_pClient->GetIP());
			if (!g_App.m_pPrefs->IsCMNotLog())
				AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Client '%s' (%s) added to filtered clients 3 data blocks request in wrong state"),
								m_pClient->GetClientNameWithSoftware(), m_pClient->GetFullIP());
			Disconnect();
		}
		return false;
	}

	m_pClient->ResetIncorrectBlockRequestCounter();
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//	CListenSocket member functions

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CListenSocket constructor
CListenSocket::CListenSocket()
{
	m_iMaxConnectionsReachedCount = 0;
	InterlockedExchange(&m_lNumPendingConnections, 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CListenSocket::~CListenSocket()
{
	Close();
	KillAllSockets();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CListenSocket::StartListening()
{
	m_bListening = true;

	return(this->Create(g_App.m_pPrefs->GetPort(), SOCK_STREAM, FD_ACCEPT) && this->Listen());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CListenSocket::RestartListening()
{
	m_bListening = true;
	if (InterlockedDecrement(&m_lNumPendingConnections) >= 0)
	{
		OnAccept(0);
	}
	else
	{
		InterlockedExchange(&m_lNumPendingConnections, 0);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CListenSocket::StopListening()
{
	m_bListening = false;
	m_iMaxConnectionsReachedCount++;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static int CALLBACK AcceptConnectionCond(LPWSABUF lpCallerId, LPWSABUF /*lpCallerData*/, LPQOS /*lpSQOS*/, LPQOS /*lpGQOS*/,
								  LPWSABUF /*lpCalleeId*/, LPWSABUF /*lpCalleeData*/, GROUP FAR* /*g*/, DWORD /*dwCallbackData*/)
{
	if (lpCallerId != NULL 
		&& lpCallerId->buf != NULL
		&& lpCallerId->len >= sizeof(SOCKADDR_IN))
	{
		LPSOCKADDR_IN pSockAddr = (LPSOCKADDR_IN)lpCallerId->buf;
		ASSERT( pSockAddr->sin_addr.S_un.S_addr != 0 && pSockAddr->sin_addr.S_un.S_addr != INADDR_NONE );

	//	Since the TCP/IP stack tries to establish the connection 3 times the simple caching of filtered IP will
	//	prevent multiple search in the IP filter table
		if (g_App.m_pIPFilter->IsCachedAndFiltered(pSockAddr->sin_addr.S_un.S_addr))
			return CF_REJECT;
	}

	return CF_ACCEPT;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	OnAccept() is called to notify the request socket that it can accept pending connection requests by calling
//	Accept().
//
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
void CListenSocket::OnAccept(int nErrorCode)
{
	if (nErrorCode == 0)
	{
		InterlockedIncrement(&m_lNumPendingConnections);
	//	If the number of pending connections was less than 0 (which obviously shouldn't happen)...
		InterlockedCompareExchange(&m_lNumPendingConnections, 1, 0);
		if (TooManySockets(true) && !g_App.m_pServerConnect->IsConnecting())
		{
			StopListening();
			return;
		}
		else if (m_bListening == false)
			RestartListening(); //If the client is still at maxconnections, this will allow it to go above it.. But if you don't, you will get a lowID on all servers.

		uint32	dwError, dwFataErrors = 0;

		while (InterlockedDecrement(&m_lNumPendingConnections) >= 0)
		{
			CClientReqSocket	*pNewClientSocket;
			SOCKADDR_IN			sockAddr = {0};
			int					iSockAddrLen = sizeof(sockAddr);
			
			if (g_App.m_pPrefs->GetProxySettings().m_bUseProxy)
			{
				pNewClientSocket = new CClientReqSocket();

				if (!Accept(*pNewClientSocket))
				{
					dwError = ::WSAGetLastError();
					pNewClientSocket->Safe_Delete();
				// check if pending connections were existed
					if (dwError == WSAEWOULDBLOCK)
					{
						AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("WSAGetLastError = WSAEWOULDBLOCK with %u"), m_lNumPendingConnections);
						InterlockedExchange(&m_lNumPendingConnections, 0);
						break;
					}
					else
					{
						AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("WSAGetLastError = %u"), dwError);
						if (++dwFataErrors > 10)
						{
						// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
						// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
						// this should basically never happen anyway
						// however if we are in such a position, try to reinitalize the socket.
							AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Accept() Error Loop"));
							Close();
							StartListening();
							InterlockedExchange(&m_lNumPendingConnections, 0);
							break;
						}
					}
					continue;
				}

				pNewClientSocket->GetPeerName((SOCKADDR*)&sockAddr, &iSockAddrLen);
				if (g_App.m_pIPFilter->IsFiltered(sockAddr.sin_addr.S_un.S_addr))
				{
					InterlockedIncrement(&g_App.m_lIncomingFiltered);
					InterlockedIncrement(&g_App.m_lTotalFiltered);
					pNewClientSocket->Safe_Delete();
					continue;
				}
			}
			else
			{
				SOCKET sNew = WSAAccept(m_SocketData.hSocket, (SOCKADDR*)&sockAddr, &iSockAddrLen, AcceptConnectionCond, 0);

				if (sNew == INVALID_SOCKET)
				{
					dwError = ::WSAGetLastError();
				// check if pending connections were existed
					if (dwError == WSAEWOULDBLOCK)
					{
						AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("WSAGetLastError = WSAEWOULDBLOCK with %u"), m_lNumPendingConnections);
						InterlockedExchange(&m_lNumPendingConnections, 0);
						break;
					}
					else if (dwError == WSAECONNREFUSED)
					{
						InterlockedIncrement(&g_App.m_lIncomingFiltered);
						InterlockedIncrement(&g_App.m_lTotalFiltered);
					}
					else
					{
						AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("WSAGetLastError = %u"), dwError);
						if (++dwFataErrors > 10)
						{
						// the question is what todo on a error. We cant just ignore it because then the backlog will fill up
						// and lock everything. We can also just endlos try to repeat it because this will lock up eMule
						// this should basically never happen anyway
						// however if we are in such a position, try to reinitalize the socket.
							AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Accept() Error Loop"));
							Close();
							StartListening();
							InterlockedExchange(&m_lNumPendingConnections, 0);
							break;
						}
					}
					continue;
				}

				pNewClientSocket = new CClientReqSocket();
				
				VERIFY(pNewClientSocket->InitAsyncSocketExInstance());
				pNewClientSocket->m_SocketData.hSocket=sNew;
				pNewClientSocket->AttachHandle(sNew);
			}

			pNewClientSocket->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);
			AddConnection();
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Process() takes care of cleaning up invalid, deleted, and timed out sockets from the open request socket list.
void CListenSocket::Process()
{
	POSITION	pos2;

//	For each open request socket...
	for (POSITION pos1 = m_openSocketList.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		m_openSocketList.GetNext(pos1);

		CClientReqSocket	*pSocket = m_openSocketList.GetAt(pos2);

	//	If the request socket is marked for deletion...
		if (pSocket->m_bDeleteThis)
		{
			if (pSocket->m_SocketData.hSocket != INVALID_SOCKET) // deadlake PROXYSUPPORT - changed to AsyncSocketEx
			{
				pSocket->Close();
			}
			else
			{
				pSocket->TimedDelete();
			}
		}
		else
		{
		//	If the request socket has timed out, disconnect it.
			m_openSocketList.GetAt(pos2)->CheckTimeOut();
		}
	}
//	If we're not currently listening for new connections and we're either just connecting to a server
//	or we're not within 5 connections of max then start listening again.
	if ((GetNumOpenSockets() + 5 < g_App.m_pPrefs->GetMaxConnections() 
		|| g_App.m_pServerConnect->IsConnecting()) && !m_bListening)
	{
		RestartListening();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CListenSocket::AddSocket(CClientReqSocket* toadd)
{
	m_openSocketList.AddTail(toadd);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CListenSocket::RemoveSocket(CClientReqSocket *pSocket)
{
	POSITION	pos;

	pos = m_openSocketList.Find(pSocket);
	if (pos != NULL)
		m_openSocketList.RemoveAt(pos);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	DeleteSocket() schedules the socket 'pSocket' for deletion if it's in the open socket list.
bool CListenSocket::DeleteSocket(CClientReqSocket *pSocket)
{
	bool		bDeleted = false;

	if (pSocket != NULL && IsValidSocket(pSocket))
	{
		pSocket->Safe_Delete();
		bDeleted = true;
	}

	return bDeleted;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CListenSocket::KillAllSockets()
{
	for (POSITION pos = m_openSocketList.GetHeadPosition(); pos != NULL; pos = m_openSocketList.GetHeadPosition())
	{
		CClientReqSocket * cur_socket = m_openSocketList.GetAt(pos);
		if (cur_socket->m_pClient)
			delete cur_socket->m_pClient;
		else
			delete cur_socket;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CListenSocket::AddConnection()
{
// check how many connection was open in last 5 sec
// note: use the while loop to handle the change of _MaxConPerFive_ in case of lower value
	while (m_dwSocketOpenTime.size() >= g_App.m_pPrefs->GetMaxConPerFive())
	{
		m_dwSocketOpenTime.pop_front();
	}
	m_dwSocketOpenTime.push_back(::GetTickCount());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	TooManySockets() returns true if the number of open sockets is greater than the number specified
//	in Preferences. If 'bIgnoreInterval' is false, it will also return true if the number of
//	new connections since the last Process() call is greater than the "connections per 5 secs"
//	setting in Preferences.
bool CListenSocket::TooManySockets(bool bIgnoreInterval)
{
	if (GetNumOpenSockets() > g_App.m_pPrefs->GetMaxConnections())
		return true;
	if (bIgnoreInterval)
		return false;
//	Check how much time was elapsed since first allowed connection within 5 sec was opened
	if (m_dwSocketOpenTime.size() >= g_App.m_pPrefs->GetMaxConPerFive() && (::GetTickCount() - m_dwSocketOpenTime.front()) <= 5000)
		return true;

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsValidSocket() returns true if 'pSocket' is in the open request socket list.
bool CListenSocket::IsValidSocket(CClientReqSocket *pSocket)
{
	return (m_openSocketList.Find(pSocket) != NULL) ? true : false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CListenSocket::Debug_ClientDeleted(CUpDownClient* deleted)
{
	POSITION pos1, pos2;
	for (pos1 = m_openSocketList.GetHeadPosition();(pos2 = pos1) != NULL;)
	{
		m_openSocketList.GetNext(pos1);
		CClientReqSocket* cur_sock = m_openSocketList.GetAt(pos2);
		if (!AfxIsValidAddress(cur_sock, sizeof(CClientReqSocket)))
		{
			AfxDebugBreak();
		}
		if (cur_sock->m_pClient == deleted)
		{
			AfxDebugBreak();
		}
	}
}
#endif //OLD_SOCKETS_ENABLED
