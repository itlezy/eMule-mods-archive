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
#include "updownclient.h"
#include "packets.h"
#include "ServerList.h"
#include "ServerSocket.h"
#include "emuleDlg.h"
#include "opcodes.h"
#include "SearchList.h"
#include "otherfunctions.h"
#include "server.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef OLD_SOCKETS_ENABLED

#pragma pack(1)
struct LoginAnswer_Struct
{
	uint32		dwClientID;
};
#pragma pack()


CServerSocket::CServerSocket(CServerConnect* in_serverconnect)
{
	m_pServerConnect = in_serverconnect;
	m_iConnectionState = CS_NOTCONNECTED;
	m_pServer = NULL;
	m_dwOldID = 0;
	m_dwLastTransmission = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerSocket::~CServerSocket()
{
	delete m_pServer;
	m_pServer = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	OnConnect() handles the "connect" socket notification (dispatched from AsyncSocketExHelperWindow).
void CServerSocket::OnConnect(int nErrorCode)
{
	EMULE_TRY

	CAsyncSocketEx::OnConnect(nErrorCode);

	switch (nErrorCode)
	{
		case 0:	// No error
		{
			if (m_pServer->HasDynIP())
			{
				SOCKADDR_IN		sockAddr;

				memzero(&sockAddr, sizeof(sockAddr));

				int		iSockAddrLen = sizeof(sockAddr);

				GetPeerName(reinterpret_cast<SOCKADDR*>(&sockAddr),&iSockAddrLen);
				m_pServer->SetIP(sockAddr.sin_addr.S_un.S_addr);

			//	Update the Server with the actual IP that we connected to
				CServer		*pServer = g_App.m_pServerList->GetServerByAddress(m_pServer->GetAddress(), m_pServer->GetPort());

				if (pServer != NULL)
					pServer->SetIP(sockAddr.sin_addr.S_un.S_addr);
			}
		//	We're waiting for the server to log us in.
			SetConnectionState(CS_WAITFORLOGIN);
			break;
		}
		case WSAEADDRNOTAVAIL:	// Various errors which basically mean "we couldn't connect to the server".
		case WSAECONNREFUSED:
		case WSAENETUNREACH:
		case WSAETIMEDOUT:
		case WSAEADDRINUSE:
			SetConnectionState(CS_SERVERDEAD);
			return;
		case WSAECONNABORTED:
			if (m_bProxyConnectFailed)
			{
				m_bProxyConnectFailed = false;
				SetConnectionState(CS_SERVERDEAD);
				return;
			}
		default:
		//	Some error occurred which we don't understand
			SetConnectionState(CS_FATALERROR);
			return;
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	OnReceive() handles the "receive" socket notification (dispatched from AsyncSocketExHelperWindow).
void CServerSocket::OnReceive(int nErrorCode)
{
//	If we're not connected or connecting, ignore this.
	if (m_iConnectionState != CS_CONNECTED && !this->m_pServerConnect->IsConnecting())
	{
		return;
	}
	CEMSocket::OnReceive(nErrorCode);
	m_dwLastTransmission = GetTickCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerSocket::ProcessPacket(char* pcPacketBuf, uint32 dwPacketSize, EnumOpcodes eOpcode)
{
	try
	{
		switch (eOpcode)
		{
			case OP_SERVERMESSAGE:
			{
				CString	strMessages, strMessage;
				CServer	*pServer = (m_pServer != NULL) ? g_App.m_pServerList->GetServerByAddress(m_pServer->GetAddress(), m_pServer->GetPort()) : NULL;

				if (dwPacketSize >= 2)
				{
					UINT			uiLen = PEEK_WORD(pcPacketBuf);
					ECodingFormat	eCF = cfLocalCodePage;

					if (uiLen > dwPacketSize - 2)
						uiLen = dwPacketSize - 2;

					if ((pServer != NULL) && (pServer->GetTCPFlags() & SRV_TCPFLG_UNICODE))
						eCF = cfUTF8;

					MB2Str(eCF, &strMessages, pcPacketBuf + sizeof(uint16), uiLen);
				}

			//	16.40 servers do not send separate OP_SERVERMESSAGE packets for each line;
			//	instead of this they are sending all text lines with one OP_SERVERMESSAGE packet.
				int			iPos = 0;

				for (;;)
				{
					strMessage = strMessages.Tokenize(_T("\r\n"), iPos);
					if (strMessage.IsEmpty())
						break;

					bool bOutputMessage = true;

					if (_tcsnicmp(strMessage, _T("server version"), 14) == 0)
					{
						CString	strVer = strMessage.Mid(14);

						strVer.Trim();
						strVer = strVer.Left(64); // truncate string to avoid misuse by servers in showing ads

						if (pServer != NULL)
						{
							unsigned	uiVerMaj, uiVerMin;

							if (_stscanf(strVer, _T("%u.%u"), &uiVerMaj, &uiVerMin) == 2)
								strVer.Format(_T("%u.%02u"), uiVerMaj, uiVerMin);
							pServer->SetVersion(strVer);
							g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
						}
					}
					else if (_tcsncmp(strMessage, _T("ERROR"), 5) == 0)
					{
						AddLogLine( LOG_FL_SBAR | LOG_RGB_ERROR, _T("ERROR %s (%s:%u) - %s"),
							(pServer != NULL) ? pServer->GetListName() : GetResString(IDS_PW_SERVER),
							(m_pServer != NULL) ? (const TCHAR*)m_pServer->GetAddress() : _T(""),
							(m_pServer != NULL) ? m_pServer->GetPort() : 0, strMessage.Mid(5).Trim(_T(" :")) );
						bOutputMessage = false;
					}
					else if (_tcsncmp(strMessage, _T("WARNING"), 7) == 0)
					{
						AddLogLine( LOG_FL_SBAR | LOG_RGB_WARNING, _T("WARNING %s (%s:%u) - %s"),
							(pServer != NULL) ? pServer->GetListName() : GetResString(IDS_PW_SERVER),
							(m_pServer != NULL) ? (const TCHAR*)m_pServer->GetAddress() : _T(""),
							(m_pServer != NULL) ? m_pServer->GetPort() : 0, strMessage.Mid(7).Trim(_T(" :")));
						bOutputMessage = false;
					}

					int	iIdx1 = strMessage.Find(_T("[emDynIP: ")), iIdx2 = strMessage.Find(_T(']'));

					if ((iIdx1 >= 0) && (iIdx1 < iIdx2))
					{
						CString	strDynIP = strMessage.Mid(iIdx1 + 10, iIdx2 - (iIdx1 + 10));

						strDynIP.Trim();
						if (strDynIP.GetLength() && strDynIP.GetLength() < 51)
						{
							if (pServer != NULL)
							{
								pServer->SetDynIP(strDynIP);
								if (m_pServer != NULL)
									m_pServer->SetDynIP(strDynIP);
								g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
							}
						}
					}

					if (bOutputMessage)
						g_App.m_pMDlg->AddServerMessageLine(strMessage);
				}
				break;
			}
			case OP_IDCHANGE:
			{
				if (dwPacketSize < sizeof(LoginAnswer_Struct))
					throw CString(_T("corrupt or invalid login answer from server received"));

				uint32	dwClientID = ((LoginAnswer_Struct*)pcPacketBuf)->dwClientID;
				byte	state;
				CServer	*pServer = NULL;

				// get & save TCP flags for current server
				ASSERT( m_pServer );
				if (m_pServer != NULL)
				{
					if (dwPacketSize >= sizeof(LoginAnswer_Struct) + 4)
						m_pServer->SetTCPFlags(*((uint32*)(pcPacketBuf + sizeof(LoginAnswer_Struct))));
					else
						m_pServer->SetTCPFlags(0);

					// copy TCP flags into the server in the server list
					pServer = g_App.m_pServerList->GetServerByAddress(m_pServer->GetAddress(), m_pServer->GetPort());
					if (pServer != NULL)
						pServer->SetTCPFlags(m_pServer->GetTCPFlags());

					if (dwPacketSize >= sizeof(LoginAnswer_Struct) + 8)
					{
					//	Standard server port to be advertized to other clients
						uint16	uPort = static_cast<uint16>(*((uint32*)(pcPacketBuf + sizeof(LoginAnswer_Struct) + 4)));

						if (m_pServer->GetPort() != uPort)
							m_pServer->SetAuxPort(m_pServer->GetPort());
						else if (m_pServer->GetAuxPort() == uPort)
							m_pServer->SetAuxPort(0);
						m_pServer->SetPort(uPort);

					//	Refresh the servers infos in the servers list
						pServer = g_App.m_pServerList->GetServerByAddress(m_pServer->GetAddress(), m_pServer->GetPort());
						if (pServer != NULL)
						{
							pServer->SetAuxPort(m_pServer->GetAuxPort());
							pServer->SetPort(m_pServer->GetPort());
						}
					}
				}

				uint32 dwSrvReportedIP = 0, dwObfuscationTCPPort = 0;

				if (dwPacketSize >= 20)
				{
					dwSrvReportedIP = PEEK_DWORD(pcPacketBuf + 12);
#if 0	//	what does IP have to do with LowID? -- doesn't look right...
					if (::IsLowID(dwSrvReportedIP))
						dwSrvReportedIP = 0;
#endif
					dwObfuscationTCPPort = PEEK_DWORD(pcPacketBuf + 16);
					if (dwObfuscationTCPPort != 0)
					{
						if (m_pServer != NULL)
							m_pServer->SetObfuscationPortTCP(static_cast<uint16>(dwObfuscationTCPPort));
						if (pServer != NULL)
							pServer->SetObfuscationPortTCP(static_cast<uint16>(dwObfuscationTCPPort));
					}
				}

				if (dwClientID == 0)
				{
					if ((state = g_App.m_pPrefs->GetSmartIdState()) != 0)
						g_App.m_pPrefs->SetSmartIdState(static_cast<byte>((state + 1) & 3));
					break;
				}

				if (g_App.m_pPrefs->GetSmartIdCheck())
				{
					if (!IsLowID(dwClientID))
						g_App.m_pPrefs->SetSmartIdState(1);
					else
					{
						state = g_App.m_pPrefs->GetSmartIdState();
						if (state != 0)
						{
							if (++state > 3)
							{
								g_App.m_pPrefs->SetSmartIdState(0);
							//	Sending message when connected with lowid
								CString strMessageText = GetResString(IDS_GOTLOWID);

								g_App.m_pMDlg->SendMail(strMessageText, g_App.m_pPrefs->GetNotifierPopOnServerError(), g_App.m_pPrefs->IsSMTPWarningEnabled());
								g_App.m_pMDlg->ShowNotifier(strMessageText, TBN_SERVER, false, g_App.m_pPrefs->GetNotifierPopOnServerError());
							}
							else
							{
								SetConnectionState(CS_ERROR);		//Cax2 - smartId bugfix
								g_App.m_pMDlg->AddLogLine(0, IDS_GOTLOWID);
								g_App.m_pPrefs->SetSmartIdState(state);
							}
							break;
						}
					}
				}

				if (m_iConnectionState != CS_CONNECTED)
				{
					SetConnectionState(CS_CONNECTED);
					g_App.OnlineSig();
				}
				m_pServerConnect->SetClientID(dwClientID);
				if (::IsLowID(dwClientID) && (dwSrvReportedIP != 0))
					g_App.SetPublicIP(dwSrvReportedIP);

				if (dwClientID != m_dwOldID)	//	Sending message if ID changed
				{
					m_dwOldID = dwClientID;

					CString strMessageText;
					strMessageText.Format(GetResString(IDS_NEWCLIENTID), dwClientID);
					g_App.m_pMDlg->SendMail(strMessageText, g_App.m_pPrefs->GetNotifierPopOnServerError(), g_App.m_pPrefs->IsSMTPWarningEnabled());
				}
				AddLogLine(0, IDS_NEWCLIENTID, dwClientID);

				g_App.m_pDownloadQueue->ResumeFiles();
				break;
			}
			case OP_SEARCHRESULT:
			{
				bool bIsMoreResultsAvailable;
				uint16 uResultsCount = g_App.m_pSearchList->ProcessSearchAnswer(pcPacketBuf, dwPacketSize, cfUTF8, &bIsMoreResultsAvailable);
				g_App.m_pMDlg->m_dlgSearch.LocalSearchEnd(uResultsCount, bIsMoreResultsAvailable);
				break;
			}
			case OP_FOUNDSOURCES_OBFU:
			case OP_FOUNDSOURCES:
			{
				CMemFile		packetStream(reinterpret_cast<BYTE*>(pcPacketBuf), dwPacketSize);
				uchar			fileHash[16];
				CPartFile		*pPartFile;

				packetStream.Read(fileHash, 16);

				if ((pPartFile = g_App.m_pDownloadQueue->GetFileByID(fileHash)) != NULL)
					pPartFile->AddServerSources( packetStream, m_pServer->GetIP(),
						m_pServer->GetPort(), (eOpcode == OP_FOUNDSOURCES_OBFU) );
				break;
			}
			case OP_SERVERSTATUS:
			{
				// FIXME some status packets have a different size -> why? structure?
				if (dwPacketSize < 8)
					break;//throw "Invalid status packet";
				uint32	dwCurUsersNum = PEEK_DWORD(pcPacketBuf);
				uint32	dwCurFilesNum = PEEK_DWORD(pcPacketBuf + 4);
				CServer	*pServer = g_App.m_pServerList->GetServerByAddress(m_pServer->GetAddress(), m_pServer->GetPort());
				if (pServer != NULL)
				{
					pServer->SetUserCount(dwCurUsersNum);
					pServer->SetFileCount(dwCurFilesNum);
					g_App.m_pMDlg->ShowUserCount(dwCurUsersNum);
					g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
				}
				break;
			}
			case OP_SERVERIDENT:
			{
				// OP_SERVERIDENT - this is sent by the server only if we send a OP_GETSERVERLIST
				if (dwPacketSize < (16 + 4 + 2 + 4))
				{
					AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Unknown server info received!"));
				//	Throw "Invalid server info received";
					break;
				}

				CServer	*pServer = (m_pServer != NULL) ? g_App.m_pServerList->GetServerByAddress(m_pServer->GetAddress(), m_pServer->GetPort()) : NULL;

				if (pServer == NULL)
					break;

				byte			aucHash[16];
				uint32			dwServerIP, dwTags;
				uint16			nServerPort;
				CSafeMemFile	data((BYTE*)pcPacketBuf, dwPacketSize);

				data.Read(aucHash, 16);			// <hash 16>
				data.Read(&dwServerIP, 4);		// <serverip 4>
				data.Read(&nServerPort, 2);		// <serverport 2>
				data.Read(&dwTags, 4);			// <tagcount 4>

				CString			strName, strDescr;
				ECodingFormat	eCF = (pServer->GetTCPFlags() & SRV_TCPFLG_UNICODE) ? cfUTF8 : cfLocalCodePage;

				for (UINT i = 0; i < dwTags; i++)
				{
					CTag tag;

					tag.FillFromStream(data, eCF);
					if (tag.GetTagID() == ST_SERVERNAME)
					{
						if (tag.IsStr())
						{
							tag.GetStringValue(&strName);
							strName.Remove(_T('\b'));
							strName.Remove(_T('\r'));
							strName.Remove(_T('\t'));
							strName.Trim();
						}
					}
					else if (tag.GetTagID() == ST_DESCRIPTION)
					{
						if (tag.IsStr())
						{
							tag.GetStringValue(&strDescr);
							strDescr.Remove(_T('\b'));
							strDescr.Remove(_T('\r'));
							strDescr.Remove(_T('\t'));
							strDescr.Trim();
						}
					}
				}

				if(!pServer->IsStaticMember())
					pServer->SetListName(strName);
				pServer->SetDescription(strDescr);
				if (((uint32*)aucHash)[0] == 0x2A2A2A2A)
				{
					const CString& rstrVersion = pServer->GetVersion();

					if (!rstrVersion.IsEmpty())
						pServer->SetVersion(_T("eFarm ") + rstrVersion);
					else
						pServer->SetVersion(_T("eFarm"));
				}
				g_App.m_pMDlg->ShowConnectionState(true, pServer->GetListName());
				g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);
				break;
			}
			case OP_SERVERLIST:	//	Add server's Server List to own Server List
			{
				if (!g_App.m_pPrefs->GetAddServersFromServer())
					break;
				try
				{
					CSafeMemFile	packetStream((BYTE*)pcPacketBuf, dwPacketSize);
					byte			byteNumServers;

					packetStream.Read(&byteNumServers, 1);

				//	Verify packet size: <server count 1>(<ip 4><port 2>)*count
					if ((1u + byteNumServers * (4u + 2u)) > dwPacketSize)
						byteNumServers = 0;

					int		iAddedServers = 0;
					uint32	dwIP;
					uint16	uPort;
					CString	strIP;

					while (byteNumServers != 0)
					{
						packetStream.Read(&dwIP, 4);
						packetStream.Read(&uPort, 2);
						ipstr(&strIP, dwIP);

						CServer		*pNewServer = new CServer(uPort, strIP);

						pNewServer->SetListName(pNewServer->GetFullIP());
						if (!g_App.m_pMDlg->m_wndServer.m_ctlServerList.AddServer(pNewServer, true))
							delete pNewServer;
						else
							iAddedServers++;
						byteNumServers--;
					}
					if (iAddedServers)
						AddLogLine(0, IDS_NEWSERVERS, iAddedServers);
				}
			//	CFileException change to CException cause "new" also can be a reason for exception
				catch (CException *error)
				{
					OUTPUT_DEBUG_TRACE();
					AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Invalid serverlist packet received"));
					error->Delete();
				}
				break;
			}
			case OP_CALLBACKREQUESTED:
			{
			//	If the packet is the right size...
				if (dwPacketSize == 6)
				{
					CUpDownClient	*pClient;
					uint32			dwIP;
					uint16			uPort;

					dwIP = PEEK_DWORD(pcPacketBuf);
					uPort = PEEK_WORD(pcPacketBuf + 4);
					pClient = g_App.m_pClientList->FindClientByIP(dwIP, uPort);

				//	If we already know of the client, make sure we're connected to him
					if (pClient != NULL)
					{
						pClient->TryToConnect();
					}
					else	//	If this client is new to us...
					{
						try
						{
							pClient = new CUpDownClient(uPort, dwIP, 0, 0, NULL, UID_ED2K);
						//	Add the new client to our client list. If successful...
							if (g_App.m_pClientList->AddClient(pClient))
							{
								pClient->SetUserName(GetResString(IDS_SERVER_SOURCE));
								pClient->TryToConnect();
							}
							else
							{
								safe_delete(pClient);
							}
						}
						catch(CMemoryException* error)
						{
							OUTPUT_DEBUG_TRACE();
							error->Delete();
						}
					}
				}
				break;
			}
			case OP_REJECT:
			{
			//  This could happen if we send a command with the wrong protocol
			//	(e.g. sending a compressed packet to a server which does not support that protocol)
				AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Server rejected last command"));
				break;
			}
			default:
				;
		}
		return true;
	}
	catch(CFileException *pError)
	{
		OUTPUT_DEBUG_TRACE();
		pError->m_strFileName = "server packet";
		AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Unhandled error while processing packet from server (%s)"), GetErrorMessage(pError));
		pError->Delete();
		if (eOpcode == OP_SEARCHRESULT || eOpcode == OP_FOUNDSOURCES)
			return true;
	}
	catch(CMemoryException *pError)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Unhandled error while processing packet from server (CMemoryException)"));
		pError->Delete();
		if (eOpcode == OP_SEARCHRESULT || eOpcode == OP_FOUNDSOURCES)
			return true;
	}
	catch(CString strError)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Unhandled error while processing packet from server (%s)"), strError);
	}
	catch(...)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Unhandled error while processing packet from server (Unknown exception)"));
	}

	SetConnectionState(CS_DISCONNECTED);
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerSocket::ConnectTo(CServer *pSrv, bool bNoCrypt)
{
	EMULE_TRY

	if (pSrv == NULL)
		return;

	safe_delete(m_pServer);	//	if m_pServer already exists, then replace it
	m_pServer = new CServer(pSrv);

	AddLogLine(0, IDS_CONNECTINGTO, m_pServer->GetListName(), m_pServer->GetFullIP(), m_pServer->GetPort());

	SetConnectionState(CS_CONNECTING);

//	Ensure that we're listening for the server's listening port connection attempt.
	g_App.m_pListenSocket->RestartListening();

//	Try to connect to 'server'. If there's an error...
	uint16	uConnPort;

	if (g_App.m_pPrefs->IsServerAuxPortUsed() && pSrv->GetAuxPort() != 0)
		uConnPort = pSrv->GetAuxPort();
	else
		uConnPort = pSrv->GetPort();

	if (!Connect(CStringA(pSrv->GetAddress()), uConnPort))
	{
		int	iError = GetLastError();

		if (iError != WSAEWOULDBLOCK)
		{
			AddLogLine(0, IDS_ERR_CONNECTIONERROR, m_pServer->GetListName(), m_pServer->GetFullIP(), uConnPort, iError);
		//	Set the connection state and notify our ServerConnect that the connection has failed.
			SetConnectionState(CS_FATALERROR);
			return;
		}
	}
//	If we connected successfully...
	else
	{
	//	Set the connection state and notify our ServerConnect that the connection has been established.
		SetConnectionState(CS_CONNECTED);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerSocket::OnError(int nErrorCode)
{
	EMULE_TRY

	SetConnectionState(CS_DISCONNECTED);
	AddLogLine( LOG_FL_DBG | LOG_RGB_ERROR, _T("Error in serversocket: %s (%s:%i): %u"),
					 m_pServer->GetListName(), m_pServer->GetFullIP(), m_pServer->GetPort(), nErrorCode );

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerSocket::PacketReceived(Packet* pPacket)
{
	EMULE_TRY

//	Calculate overhead at one place before decompression
	g_App.m_pDownloadQueue->AddDownDataOverheadServer(pPacket->m_dwSize);

	if (pPacket->m_byteProtocol == OP_PACKEDPROT)
	{
		if (!pPacket->UnpackPacket(250000))
		{
			AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Failed to decompress server TCP packet: protocol=0x%02x opcode=0x%02x size=%u"),
				pPacket ? pPacket->m_byteProtocol : 0,
				pPacket ? pPacket->m_eOpcode : 0,
				pPacket ? pPacket->m_dwSize : 0);
			return;
		}
		pPacket->m_byteProtocol = OP_EDONKEYPROT;
	}

	if (pPacket->m_byteProtocol == OP_EDONKEYPROT)
		ProcessPacket(pPacket->m_pcBuffer,pPacket->m_dwSize,pPacket->m_eOpcode);
	else
		AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Received server TCP packet with unknown protocol: protocol=0x%02x opcode=0x%02x size=%u"),
			pPacket ? pPacket->m_byteProtocol : 0,
			pPacket ? pPacket->m_eOpcode : 0,
			pPacket ? pPacket->m_dwSize : 0);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerSocket::OnClose(int nErrorCode)
{
	NOPRM(nErrorCode);

	CEMSocket::OnClose(0);
	if (m_iConnectionState == CS_WAITFORLOGIN)
		SetConnectionState(CS_SERVERFULL);
	else if (m_iConnectionState == CS_CONNECTED)
		SetConnectionState(CS_DISCONNECTED);
	else
		SetConnectionState(CS_NOTCONNECTED);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetConnectionState() sets the connection state of this server socket and notifies
//	its associated ServerConnect of the change in state.
void CServerSocket::SetConnectionState(int iNewState)
{
	EMULE_TRY

	m_iConnectionState = iNewState;

	if (iNewState <= 0)
		m_pServerConnect->ConnectionFailed(this);
	else if ((iNewState == CS_CONNECTED) || (iNewState == CS_WAITFORLOGIN))
	{
		if (m_pServerConnect != NULL)
			m_pServerConnect->ConnectionEstablished(this);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //OLD_SOCKETS_ENABLED
