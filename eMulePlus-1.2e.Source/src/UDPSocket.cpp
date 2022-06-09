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
#include "packets.h"
#include "UDPSocket.h"
#include "ServerList.h"
#include "otherfunctions.h"
#include "otherstructs.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#ifdef OLD_SOCKETS_ENABLED

#define	ERR_UDP_MISCONFIGURED_SERVER _T("Error while processing incoming UDP Packet (most likely a misconfigured server)")

CUDPSocketWnd::CUDPSocketWnd(){
}

BEGIN_MESSAGE_MAP(CUDPSocketWnd, CWnd)
	ON_MESSAGE(WM_DNSLOOKUPDONE, OnDNSLookupDone)
END_MESSAGE_MAP()

LRESULT CUDPSocketWnd::OnDNSLookupDone(WPARAM wParam,LPARAM lParam)
{
	EMULE_TRY

	if(m_pOwner)
		m_pOwner->DnsLookupDone(wParam,lParam);
	return true;

	EMULE_CATCH

	return false;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUDPSocket::CUDPSocket(CServerConnect* in_serverconnect)
{
	m_hWndResolveMessage = NULL;
	m_pcSendBuffer = NULL;
	m_pServer = NULL;
	m_pServerConnect = in_serverconnect;
	m_hDNSTask = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUDPSocket::~CUDPSocket()
{
	EMULE_TRY

	delete m_pServer;
	delete[] m_pcSendBuffer;

	m_udpwnd.DestroyWindow();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool  CUDPSocket::Create()
{
	EMULE_TRY

	VERIFY( m_udpwnd.CreateEx(0, AfxRegisterWndClass(0),_T("Emule Socket Wnd"),WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));
	m_hWndResolveMessage = m_udpwnd.m_hWnd;
	m_udpwnd.m_pOwner = this;

	return (CAsyncSocket::Create(0, SOCK_DGRAM, FD_READ)) ? true : false;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUDPSocket::OnReceive(int iErrorCode)
{
	NOPRM(iErrorCode);
	EMULE_TRY

	byte		abyteBuffer[5000];
	SOCKADDR_IN	sockAddr = {0};
	int			iSockAddrLen = sizeof(sockAddr);
	int			iLength = ReceiveFrom(abyteBuffer, sizeof(abyteBuffer), (SOCKADDR*)&sockAddr, &iSockAddrLen);

	if ((iLength >= 2) && (abyteBuffer[0] == OP_EDONKEYPROT))	//SOCKET_ERROR = -1
		ProcessPacket(abyteBuffer + 2, iLength - 2, abyteBuffer[1], sockAddr.sin_addr.s_addr, ntohs(sockAddr.sin_port));
	
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUDPSocket::ProcessPacket(byte *pbytePacket, uint32 dwSize, byte uOpcode, uint32 dwIP, uint16 uUDPPort)
{
	CServer	*pServer = NULL;
	bool	bUpdateServerDesc;

	try
	{
		g_App.m_pDownloadQueue->AddDownDataOverheadServer(dwSize);
		if ((pServer = g_App.m_pServerList->GetServerByAddress(ipstr(dwIP), uUDPPort - 4)) == NULL)
		{
		//	Don't process packet from unknown servers
			return false;
		}
		if ((bUpdateServerDesc = (pServer->GetFailedCount() != 0)) == true)
			pServer->ResetFailedCount();

		switch (uOpcode)
		{
			case OP_GLOBSEARCHRES:
			{
				CSafeMemFile	pckStream(pbytePacket, dwSize);
				uint32			dwLeft;
				uint16			uResults;
				byte			byteTmp;

				if (g_App.m_pSearchList->AllowUDPSearchAnswer())
				{
					do
					{
						uResults = g_App.m_pSearchList->ProcessUDPSearchAnswer(pckStream, cfUTF8);

					//	Check if there is another search results packet
						dwLeft = static_cast<uint32>(pckStream.GetLength() - pckStream.GetPosition());
						if (dwLeft >= 2)
						{
							pckStream.Read(&byteTmp, 1);
							dwLeft--;
							if (byteTmp != OP_EDONKEYPROT)
								break;

							pckStream.Read(&byteTmp, 1);
							dwLeft--;
							if (byteTmp != OP_GLOBSEARCHRES)
								break;
						}
					} while(dwLeft != 0);
					g_App.m_pMDlg->m_dlgSearch.AddGlobalEd2kSearchResults(uResults);
				}
				break;
			}
			case OP_GLOBFOUNDSOURCES:
			{
				CSafeMemFile	sources((BYTE*)pbytePacket, dwSize);
				uchar			abyteFileId[16];
				uint32			dwLeft;
				byte			byteTmp;

				do
				{
					sources.Read(abyteFileId, 16);

					CPartFile* 		pPartFile = g_App.m_pDownloadQueue->GetFileByID(abyteFileId);

					if (pPartFile)
						pPartFile->AddServerSources(sources, dwIP, uUDPPort - 4, false);
					else
					{
					//	Skip sources for that file
						sources.Read(&byteTmp, 1);
						sources.Seek(byteTmp * (4 + 2), SEEK_CUR);
					}

				//	Check if there is another source packet
					dwLeft = static_cast<uint32>(sources.GetLength() - sources.GetPosition());
					if (dwLeft >= 2)
					{
						sources.Read(&byteTmp, 1);
						dwLeft--;
						if (byteTmp != OP_EDONKEYPROT)
							break;

						sources.Read(&byteTmp, 1);
						dwLeft--;
						if (byteTmp != OP_GLOBFOUNDSOURCES)
							break;
					}
				} while(dwLeft != 0);
				break;
			}
 			case OP_GLOBSERVSTATRES:
			{
			//	Check minimal allowed size (contain basic information)
				if ((dwSize < 12) || (PEEK_DWORD(pbytePacket) != pServer->GetChallenge()))
					return false;
				pServer->SetChallenge(0);
				pServer->SetCryptPingReplyPending(false);

				uint32	dwCurTime = static_cast<uint32>(time(NULL));

			//	If we used Obfuscated ping, we still need to reset the time properly
				pServer->SetLastPingedTime(dwCurTime - (rand() % UDPSRVSTATREASKRNDTIME));

				pServer->SetUserCount(PEEK_DWORD(pbytePacket + 4));
				pServer->SetFileCount(PEEK_DWORD(pbytePacket + 8));

				pServer->SetMaxUsers((dwSize >= 16) ? PEEK_DWORD(pbytePacket + 12) : 0);
				pServer->SetSoftMaxFiles((dwSize >= 24) ? PEEK_DWORD(pbytePacket + 16) : 0);
				pServer->SetHardMaxFiles((dwSize >= 24) ? PEEK_DWORD(pbytePacket + 20) : 0);

				pServer->SetUDPFlags((dwSize >= 28) ? PEEK_DWORD(pbytePacket + 24) : 0);
				pServer->SetLowIDUsers((dwSize >= 32) ? PEEK_DWORD(pbytePacket + 28) : 0);

				pServer->SetObfuscationPortUDP((dwSize >= 40) ? PEEK_WORD(pbytePacket + 32) : 0);
				pServer->SetObfuscationPortTCP((dwSize >= 40) ? PEEK_WORD(pbytePacket + 34) : 0);
				pServer->SetServerKeyUDP((dwSize >= 40) ? PEEK_DWORD(pbytePacket + 36) : 0);

				pServer->SetPing(::GetTickCount() - pServer->GetLastPinged());

				pServer->SetLastDescPingedCount(false);
				if (pServer->GetLastDescPingedCount() < 2)
				{
				// eserver 16.45+ supports a new OP_SERVER_DESC_RES answer, if the OP_SERVER_DESC_REQ contains a uint32
				// challenge, the server returns additional info with OP_SERVER_DESC_RES. To properly distinguish the
				// old and new OP_SERVER_DESC_RES answer, the challenge has to be selected carefully. The first 2 bytes
				// of the challenge (in network byte order) MUST NOT be a valid string-len-int16!
					Packet	*pPacket = new Packet(OP_SERVER_DESC_REQ, 4);
					uint32	dwChallenge = (rand() << 16) | INV_SERV_DESC_LEN; // 0xF0FF = 'invalid' string length
					pServer->SetDescReqChallenge(dwChallenge);
					POKE_DWORD(pPacket->m_pcBuffer, dwChallenge);
					g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
					g_App.m_pServerConnect->SendUDPPacket(pPacket, pServer, true);
				}
				else
					pServer->SetLastDescPingedCount(true);

				bUpdateServerDesc = true;
				break;
			}

 			case OP_SERVER_DESC_RES:
			{
				// old packet: <name_len 2><name name_len><desc_len 2><desc desc_len>
				// new packet: <challenge 4><taglist>
				//
				// NOTE: To properly distinguish between the two packets which are both using the same opcode...
				// the first two bytes of <challenge> (in network byte order) have to be an invalid <name_len> at least.

				CSafeMemFile	srvinfo((BYTE*)pbytePacket, dwSize);
				uint32			ui, dwTmp;

				if ((dwSize >= 8) && (PEEK_WORD(pbytePacket) == INV_SERV_DESC_LEN))
				{
					srvinfo.Read(&dwTmp, 4);	 // read challenge
					if ((pServer->GetDescReqChallenge() != 0) && (dwTmp == pServer->GetDescReqChallenge()))
					{
						pServer->SetDescReqChallenge(0);
						srvinfo.Read(&dwTmp, 4);	 // read tag count
						for (ui = 0; ui < dwTmp; ui++)
						{
							CTag	TempTag;

							TempTag.FillFromStream(srvinfo, cfUTF8);
							if (TempTag.GetTagID() == ST_SERVERNAME)
							{
								if (TempTag.IsStr() && !pServer->IsStaticMember())
									pServer->SetListName(TempTag.GetStringValue());
							}
							else if (TempTag.GetTagID() == ST_DESCRIPTION)
							{
								if (TempTag.IsStr())
									pServer->SetDescription(TempTag.GetStringValue());
							}
							else if (TempTag.GetTagID() == ST_DYNIP)
							{
								if (TempTag.IsStr())
									pServer->SetDynIP(TempTag.GetStringValue());
							}
							else if (TempTag.GetTagID() == ST_VERSION)
							{
								if (TempTag.IsStr())
									pServer->SetVersion(TempTag.GetStringValue());
								else if (TempTag.IsInt())
								{
									CString strVersion;

									strVersion.Format(_T("%u.%02u"), TempTag.GetIntValue() >> 16, TempTag.GetIntValue() & 0xFFFF);
									pServer->SetVersion(strVersion);
								}
							}
							else if (TempTag.GetTagID() == ST_AUXPORTSLIST)
							{
								if (TempTag.IsStr())	// <string> = <port> [, <port>...]
								{
								}
							}
						}
					}
				}
				else
				{
					uint16	uStringLen;
					CString	strBuf;

					srvinfo.Read(&uStringLen, 2);
					ReadMB2Str(cfUTF8, &strBuf, srvinfo, uStringLen);

					if(!pServer->IsStaticMember())
						pServer->SetListName(strBuf);

					srvinfo.Read(&uStringLen, 2);
					ReadMB2Str(cfUTF8, &strBuf, srvinfo, uStringLen);

					pServer->SetDescription(strBuf);
				}
				bUpdateServerDesc = true;
				break;
			}
			default:
				return false;
		}

		if (bUpdateServerDesc)
			g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pServer);

		return true;
	}
	catch(CFileException* error)
	{
		CString	strBuff(ERR_UDP_MISCONFIGURED_SERVER);

		if (pServer != NULL)
			strBuff.AppendFormat(_T(" %s:%u"), pServer->GetAddress(), pServer->GetPort());

		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, strBuff);
		error->Delete();
		if (uOpcode == OP_GLOBSEARCHRES || uOpcode == OP_GLOBFOUNDSOURCES)
			return true;
	}
	catch(CMemoryException* error)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, ERR_UDP_MISCONFIGURED_SERVER _T(" - CMemoryException"));
		error->Delete();
		if (uOpcode == OP_GLOBSEARCHRES || uOpcode == OP_GLOBFOUNDSOURCES)
			return true;
	}
	catch(CString error)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, ERR_UDP_MISCONFIGURED_SERVER _T(" - ") + error);
	}
	catch(...)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, ERR_UDP_MISCONFIGURED_SERVER);
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUDPSocket::AsyncResolveDNS(LPCTSTR lpszHostAddress, UINT nHostPort)
{
	EMULE_TRY

	m_lpszHostAddress = lpszHostAddress;
	m_nHostPort = nHostPort;
	if (m_hDNSTask)
		WSACancelAsyncRequest(m_hDNSTask);
	m_hDNSTask = NULL;
// 	See if we have an IP already
	USES_CONVERSION;
	SOCKADDR_IN sockAddr;
	memzero(&sockAddr, sizeof(sockAddr));
	LPSTR lpszAscii = T2A((LPTSTR)m_lpszHostAddress);
	sockAddr.sin_family = AF_INET;
	sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);
	sockAddr.sin_port = fast_htons((u_short)m_nHostPort);

//	Backup for send socket
	m_SaveAddr = sockAddr;

	if (sockAddr.sin_addr.s_addr == INADDR_NONE)
	{
	//	Resolve hostname "hostname" asynchronously
		memzero(DnsHostBuffer, sizeof(DnsHostBuffer));

		USES_CONVERSION;
		m_hDNSTask = WSAAsyncGetHostByName( m_hWndResolveMessage,
											   WM_DNSLOOKUPDONE,
											   CT2A(lpszHostAddress),
											   DnsHostBuffer,
											   MAXGETHOSTSTRUCT );

		if (m_hDNSTask == 0)
		{
			delete[] m_pcSendBuffer;
			m_pcSendBuffer = NULL;
			delete m_pServer;
			m_pServer = NULL;
#ifdef _DEBUG
			AfxMessageBox(_T("LOOKUPERROR DNSTASKHANDLE = 0"));
#endif
		}
	}
	else
	{
		SendBuffer();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUDPSocket::DnsLookupDone(WPARAM wp, LPARAM lp)
{
	NOPRM(wp);
	EMULE_TRY

	m_hDNSTask = NULL;

//	An asynchronous database routine completed
	if (WSAGETASYNCERROR(lp) != 0)
	{
		delete[] m_pcSendBuffer;
		m_pcSendBuffer = NULL;
		delete m_pServer;
		m_pServer = NULL;
		return;
	}
	if (m_SaveAddr.sin_addr.s_addr == INADDR_NONE)
	{
	//	Get the structure length
		int iBufLen = WSAGETASYNCBUFLEN(lp);
		LPHOSTENT lphost = (LPHOSTENT)malloc(iBufLen);
		memcpy2(lphost, DnsHostBuffer, iBufLen);
		m_SaveAddr.sin_addr.s_addr = ((LPIN_ADDR)lphost->h_addr)->s_addr;
		free(lphost);
	//	Also reset the receive buffer
		memzero(DnsHostBuffer, sizeof(DnsHostBuffer));
	}
	if (m_pServer)
	{
		CServer* update = g_App.m_pServerList->GetServerByAddress(m_pServer->GetAddress(), m_pServer->GetPort());
		if (update)
			update->SetIP(m_SaveAddr.sin_addr.S_un.S_addr);
		SendBuffer();
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUDPSocket::SendBuffer()
{
	EMULE_TRY

	if(m_pServer && m_pcSendBuffer)
	{
		SendTo(m_pcSendBuffer, m_dwSendBufLength, (SOCKADDR*)&m_SaveAddr, sizeof(m_SaveAddr));
	}
	safe_delete(m_pServer);
	delete[] m_pcSendBuffer;
	m_pcSendBuffer = NULL;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUDPSocket::SendPacket(Packet *pPacket, CServer *pHostServer)
{
//	At this point m_pServer & m_pcSendBuffer must be already NULL but it seems that sometime we are not able
//	to get WM_DNSLOOKUPDONE before we send a new packet therefore the existency of the objects will be checked
	safe_delete(m_pServer);
	delete[] m_pcSendBuffer;
	m_pcSendBuffer = NULL;

//	Now we are trying to create a new objects
	try
	{
		m_pServer = new CServer(pHostServer);
		m_pcSendBuffer = new char[pPacket->m_dwSize + sizeof(UDP_Header_Struct)];
		memcpy(m_pcSendBuffer, pPacket->GetUDPHeader(), sizeof(UDP_Header_Struct));
		memcpy2(m_pcSendBuffer + sizeof(UDP_Header_Struct), pPacket->m_pcBuffer, pPacket->m_dwSize);
		m_dwSendBufLength = pPacket->m_dwSize + sizeof(UDP_Header_Struct);
		AsyncResolveDNS(m_pServer->GetAddress(), m_pServer->GetPort() + 4);
	}
	catch (CException * error)
	{
		OUTPUT_DEBUG_TRACE();
		error->Delete();
		safe_delete(m_pServer);
		delete[] m_pcSendBuffer;
		m_pcSendBuffer = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //OLD_SOCKETS_ENABLED
