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
#include "ServerList.h"
#include "emule.h"
#include "server.h"
#include "HttpDownloadDlg.h"
#include "SafeFile.h"
#include "otherfunctions.h"
#include "IPFilter.h"
#include <share.h>
#include <io.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerList::CServerList() : m_serverList(20)
{
	m_dwServerPos = 0;
	searchserverpos = 0;
	statserverpos = 0;
	m_dwDelSrvCnt = 0;
	m_bListLoaded = false;
	m_nLastSaved = ::GetTickCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::AutoUpdate()
{
	g_App.m_pPrefs->LoadServerlistAddresses();
	if (g_App.m_pPrefs->m_addressesList.IsEmpty())
	{
		MessageBox(g_App.m_pMDlg->m_wndServer.m_hWnd, GetResString(IDS_ERR_EMPTYADRESSESDAT), GetResString(IDS_ERR_EMPTYADRESSESDAT_TITLE), MB_ICONASTERISK);
		return;
	}
	bool bDownloaded = false;
	CString servermet;
	CString oldservermet;
	CString strURLToDownload;
	CSingleLock AccessLock(&m_csServerMetFile, TRUE);
	servermet.Format(_T("%sserver.met"), g_App.m_pPrefs->GetConfigDir());
	oldservermet.Format(_T("%sserver_met.old"), g_App.m_pPrefs->GetConfigDir());
	_tremove(oldservermet);
	_trename(servermet, oldservermet);

	POSITION Pos = g_App.m_pPrefs->m_addressesList.GetHeadPosition();
	while ((!bDownloaded) && (Pos != NULL))
	{
		CHttpDownloadDlg dlgDownload;
		strURLToDownload = g_App.m_pPrefs->m_addressesList.GetNext(Pos);
		dlgDownload.m_strInitializingTitle = GetResString(IDS_HTTP_CAPTION);
		dlgDownload.m_nIDPercentage = IDS_HTTPDOWNLOAD_PERCENTAGE;
		dlgDownload.m_sURLToDownload = strURLToDownload;
		dlgDownload.m_sFileToDownloadInto = servermet;
		if (dlgDownload.DoModal() == IDOK)
		{
			_tremove(oldservermet);
			bDownloaded = true;
		}
		else
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_FAILEDDOWNLOADMET, strURLToDownload);
	}
	if (!bDownloaded)
		_trename(oldservermet, servermet);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerList::Init()
{
//	Update IPFilter
	bool	bLoaded = false;

	if (g_App.m_pPrefs->IsIPFilterUpdateOnStart())
	{
		if (g_App.m_pPrefs->GetIPFilterUpdateFrequency() >= 0)
		{
			CTime	LastUpdate(static_cast<time_t>(g_App.m_pPrefs->GetLastIPFilterUpdate()));
			time_t	tLastUpdate = mktime(LastUpdate.GetLocalTm(NULL));
			time_t	tNow = mktime(CTime::GetCurrentTime().GetLocalTm(NULL));

			if ((difftime(tNow, tLastUpdate) >= ((g_App.m_pPrefs->GetIPFilterUpdateFrequency() + 1) * 60 * 60 * 24)))
			{
				bLoaded = g_App.m_pIPFilter->DownloadIPFilter();
				if (bLoaded)
					g_App.m_pPrefs->SetLastIPFilterUpdate(static_cast<uint32>(tNow));
			}
		}
	}

//	Only loads IPFilter.dat if auto-update is disabled or has failed (it avoids loading it twice).
	if (!bLoaded)
		g_App.m_pIPFilter->LoadFromDefaultFile(false);

//	Auto update the list by using an url
	if (g_App.m_pPrefs->AutoServerlist())
		AutoUpdate();

//	Load Metfile
	CString	strPath(g_App.m_pPrefs->GetConfigDir());
		
	strPath += _T("server.met");

	CSingleLock	AccessLock(&m_csServerMetFile, TRUE);

//	Disable redraw during the time, when server will be added & sorted
	g_App.m_pMDlg->m_wndServer.m_ctlServerList.SetRedraw(FALSE);

//	Don't delete existing servers, because we might have added one by command line
	bool	bRes = AddServerMetToList(strPath, false);

//	Insert static servers from textfile
	strPath.Format(_T("%s") CFGFILE_STATICSERVERS, g_App.m_pPrefs->GetConfigDir());
	LoadServersFromTextFile(strPath);

	m_bListLoaded = true;	//	Protection from list erase on fast closure 

//	Make first list sort after all initial information is added
	g_App.m_pMDlg->m_wndServer.m_ctlServerList.SortFirstInit();

//	Enable redraw since servers were already added & sorted
	g_App.m_pMDlg->m_wndServer.m_ctlServerList.SetRedraw(TRUE);

//	Autoconnect to server
	if (g_App.m_pPrefs->DoAutoConnect())
		g_App.m_pMDlg->OnBnClickedButton2();

	return bRes;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerList::AddServerMetToList(const CString& strFile, bool bMergeWithPrevList /*= true*/)
{
	CSafeBufferedFile	servermet;
	CServer		*pNewServer = NULL, *pExistingServer = NULL;
	byte		byteServerListVersion;

	try
	{
		if (!servermet.Open(strFile, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
		{
			if (!bMergeWithPrevList)
			{
				AddLogLine(LOG_RGB_ERROR, IDS_ERR_LOADSERVERMET);
				g_App.m_pMDlg->DisableAutoBackup();
			}
			return false;
		}
		servermet.Read(&byteServerListVersion, 1);
		if ((byteServerListVersion != 0xE0) && (byteServerListVersion != MET_HEADER))
		{
			servermet.Close();
			AddLogLine(LOG_RGB_ERROR, IDS_ERR_BADSERVERMETVERSION, byteServerListVersion);
			g_App.m_pMDlg->DisableAutoBackup();
			return false;
		}
		
		uint32 dwServerCount;
		servermet.Read(&dwServerCount, 4);

		ServerMet_Struct sCurSavingServer;
		uint32 dwAddedServerCount = 0;
		CString strListName;
		
		for (uint32 j = 0; j != dwServerCount; j++)
		{
		//	get server
			servermet.Read(&sCurSavingServer, sizeof(ServerMet_Struct));
			try
			{
				pNewServer = new CServer(&sCurSavingServer);
			}
			catch (CMemoryException* error)
			{
				error->Delete();
				continue;
			}
			
		//add tags
			for (uint32 i = 0; i < sCurSavingServer.m_dwTagCount; i++)
				pNewServer->AddTagFromFile(servermet);
		//	set listname for server
			if (pNewServer->GetListName().IsEmpty())
			{
				strListName.Format(_T("Server %s"), pNewServer->GetAddress());
				pNewServer->SetListName(strListName);
			}
			
			if (!g_App.m_pMDlg->m_wndServer.m_ctlServerList.AddServer(pNewServer, true, true))
			{
				pExistingServer = g_App.m_pServerList->GetServerByAddress(pNewServer->GetAddress(), pNewServer->GetPort());
				if (pExistingServer)
				{
					pExistingServer->SetListName(pNewServer->GetListName());
					if (pNewServer->GetDescription())
						pExistingServer->SetDescription(pNewServer->GetDescription());
					g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pExistingServer);
				}
				safe_delete(pNewServer);
			}
			else
				dwAddedServerCount++;

			pNewServer = NULL;
		}

		if (!bMergeWithPrevList)
			AddLogLine(LOG_FL_SBAR, IDS_SERVERSFOUND, dwServerCount);
		else
			AddLogLine(LOG_FL_SBAR, IDS_SERVERSADDED, dwAddedServerCount, dwServerCount - dwAddedServerCount);
		servermet.Close();
	}
	catch (CFileException *pError)
	{
		safe_delete(pNewServer);
		
		OUTPUT_DEBUG_TRACE();
		if (pError->m_cause == CFileException::endOfFile)
			AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Error: the file server.met is corrupted, unable to load serverlist!"));
		else
			AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Unexpected file error while reading server.met: %s, unable to load serverlist!"), GetErrorMessage(pError));
		pError->Delete();
		g_App.m_pMDlg->DisableAutoBackup();
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerList::AddServer(CServer *pNewServer, bool bChangeServerInfo/*=false*/)
{
	if (pNewServer == NULL)
		return false;

	if(pNewServer->GetDynIP() == _T("255.255.255.255"))
		return false;

	if (g_App.m_pPrefs->IsFilterBadIPs())
		if (!IsGoodServerIP(pNewServer))
			return false;

	CServer	*pExistingServer = FindServerByAddress(*pNewServer, false);

	if (pExistingServer != NULL)
	{
		if (bChangeServerInfo)
		{
			pExistingServer->SetListName(pNewServer->GetListName());
			pExistingServer->SetPort(pNewServer->GetPort());
			pExistingServer->SetAuxPort(pNewServer->GetAuxPort());
			pExistingServer->ResetFailedCount();
			g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pExistingServer);

			return true;
		}

		pExistingServer->ResetFailedCount();
		g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pExistingServer);
		return false;
	}

	m_serverList.AddTail(pNewServer);

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::ServerStats()
{
#ifdef OLD_SOCKETS_ENABLED
	if (g_App.m_pServerConnect->IsConnected() && m_serverList.GetCount() > 0)
	{
		uint32	dwChallenge, dwCurTime, dwRnd;
		CServer	*pTestServer, *pPingServer = GetNextStatServer();

		if ((pTestServer = pPingServer) == NULL)
			return;

		dwRnd = rand();
		dwCurTime = static_cast<uint32>(time(NULL));
		while ( (pPingServer->GetLastPingedTime() != 0) &&
			((dwCurTime - pPingServer->GetLastPingedTime()) < UDPSERVSTATREASKTIME) )
		{
			pPingServer = GetNextStatServer();
			if (pPingServer == pTestServer)
				return;
		}
		if ( pPingServer->GetFailedCount() >= g_App.m_pPrefs->GetDeadserverRetries()
		  && g_App.m_pPrefs->DeadServer()
		  && !pPingServer->IsStaticMember() )
		{
			g_App.m_pMDlg->m_wndServer.m_ctlServerList.RemoveServer(pPingServer);
			return;
		}

		pPingServer->SetRealLastPingedTime(dwCurTime);
		if ( !pPingServer->GetCryptPingReplyPending() && (g_App.GetPublicIP() != 0) &&
			g_App.m_pPrefs->IsServerCryptLayerUDPEnabled() )
		{
		//	We try a obfsucation ping first and wait 20 seconds for an answer
		//	if it doesn't get replied, it isn't counted as error with following normal ping
			pPingServer->SetCryptPingReplyPending(true);

			uint32	dwPktLen = 4 + (dwRnd & 15); // max padding 16 bytes
			byte	abyteRawPacket[4 + 15];

			dwChallenge = (rand() << 17) | (rand() << 2) | ((dwRnd >> 8) & 3);
			if (dwChallenge == 0)
				dwChallenge++;
			POKE_DWORD(abyteRawPacket, dwChallenge);
			for (uint32 i = 4; i < dwPktLen; i++) // filling up the remaining bytes with random data
				abyteRawPacket[i] = static_cast<byte>(rand());

			pPingServer->SetChallenge(dwChallenge);
			pPingServer->SetLastPinged(GetTickCount());
			pPingServer->SetLastPingedTime(dwCurTime - UDPSERVSTATREASKTIME + 20); // give it 20 seconds to respond
			
			g_App.m_pUploadQueue->AddUpDataOverheadServer(dwPktLen);
#ifdef _CRYPT_READY
			g_App.m_pServerConnect->SendUDPPacket(NULL, pPingServer, false, pPingServer->GetPort() + 12, abyteRawPacket, dwPktLen);
#endif
		}
		else
		{
		//	Obfsucation ping request wasn't answered (or not sent at all), so probably
		//	the server doesn't supports obfuscation -- continue with a normal request
			Packet	*pPacket = new Packet(OP_GLOBSERVSTATREQ, 4);

			pPingServer->SetCryptPingReplyPending(false);
			dwChallenge = 0x55AA0000 + (dwRnd & 0xFFFF);
			pPingServer->SetChallenge(dwChallenge);
			POKE_DWORD(pPacket->m_pcBuffer, dwChallenge);
			pPingServer->SetLastPinged(::GetTickCount());
			pPingServer->SetLastPingedTime(dwCurTime - (dwRnd % UDPSRVSTATREASKRNDTIME));
			pPingServer->AddFailedCount();
			g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pPingServer);
			g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
			g_App.m_pServerConnect->SendUDPPacket(pPacket, pPingServer, true);
		}
	}
#endif //OLD_SOCKETS_ENABLED
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*filter Servers with invalid IP's / Port
0.*
10.*
172.16.0.0 - 172.31.255.255
192.168.0.0 - 192.168.255.255
127.*
*/
bool CServerList::IsGoodServerIP(CServer* in_server)
{
	if (!in_server->HasPublicAddress() && !in_server->HasDynIP())
		return false;

//	Using filters from m_pIPFilter.dat for server filtering
//	Now filtering servers listed in m_pIPFilter.dat and DynIP servers
	if (g_App.m_pPrefs->IsFilterServersByIP())
	{
		if (in_server->HasDynIP() || g_App.m_pIPFilter->IsFiltered(in_server->GetIP()))
		{
			if (!g_App.m_pPrefs->IsCMNotLog())
				AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("Filtered Server: %s %hs"), in_server->GetFullIP(), g_App.m_pIPFilter->GetLastHit());
			return false;
		}
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::RemoveServer(CServer* out_server)
{
	POSITION remove_pos = m_serverList.Find(out_server);

	if (remove_pos != NULL)
	{
	//	Reset UDP server
		if (g_App.m_pDownloadQueue->pCurUDPServer == out_server)
			g_App.m_pDownloadQueue->pCurUDPServer = GetSuccServer(out_server);
	//	Finally delete a server
		delete m_serverList.GetAt(remove_pos);
		m_serverList.RemoveAt(remove_pos);
		m_dwDelSrvCnt++;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::GetServersStatus(uint32 &total, uint32 &failed, uint32 &user, uint32 &file,
																	 uint32 &dwLowIdUsers, uint32 &tuser, uint32 &tfile, double &occ)
{
	total = m_serverList.GetCount();
	failed = 0;
	user = 0;
	file = 0;
	tuser = 0;
	tfile = 0;
	occ = 0;
	dwLowIdUsers = 0;

	uint32 dwMaxUsers = 0;
	uint32 tuserk = 0;

	CServer	*pServer;

	for (POSITION pos = m_serverList.GetHeadPosition(); pos != NULL; )
	{
		pServer = m_serverList.GetNext(pos);

		if (pServer->GetFailedCount())
			failed++;
		else
		{
			user += pServer->GetNumUsers();
			file += pServer->GetFiles();
			dwLowIdUsers += pServer->GetLowIDUsers();
		}
		tuser += pServer->GetNumUsers();
		tfile += pServer->GetFiles();

		if (pServer->GetMaxUsers())
		{
			tuserk += pServer->GetNumUsers();
			dwMaxUsers += pServer->GetMaxUsers();
		}
	}
	if (dwMaxUsers > 0)
		occ = static_cast<double>(tuserk * 100.0) / dwMaxUsers;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::GetUserFileStatus(uint32 &user, uint32 &file)
{
	user = 0;
	file = 0;

	CServer	   *pServer;

	for (POSITION pos = m_serverList.GetHeadPosition(); pos != NULL; )
	{
		pServer = m_serverList.GetNext(pos);
		if (!pServer->GetFailedCount())
		{
			user += pServer->GetNumUsers();
			file += pServer->GetFiles();
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServerList::~CServerList()
{
	if (m_bListLoaded)	//	Save only if the list was already loaded to avoid list reset
		SaveServerMetToFile();

	for (POSITION pos1 = m_serverList.GetHeadPosition(); pos1 != NULL;)
		delete m_serverList.GetNext(pos1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::MoveServerDown(CServer *pSrv)
{
	POSITION	pos1, pos2;
	int			i = 0;
	CServer		*pServer;

	for (pos1 = m_serverList.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		pServer = m_serverList.GetNext(pos1);
		if (pServer == pSrv)
		{
			m_serverList.AddTail(pServer);
			m_serverList.RemoveAt(pos2);
			return;
		}
		i++;
		if (i == m_serverList.GetCount())
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Sort() reorders the server list by priority.
void CServerList::Sort()
{
	POSITION	pos1, pos2;
	int			i = 0;
	CServer		*pServer;

//	For each server in the list...
	for (pos1 = m_serverList.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		pServer = m_serverList.GetNext(pos1);

	//	If the server is high priority, stick it at the head of the list.
		if (pServer->GetPreferences() == PR_HIGH)
		{
			m_serverList.AddHead(pServer);
			m_serverList.RemoveAt(pos2);
		}
	//	If the server is low priority, stick it at the tail of the list.
		else if (pServer->GetPreferences() == PR_LOW)
		{
			m_serverList.AddTail(pServer);
			m_serverList.RemoveAt(pos2);
		}
		i++;
	//	If we're at the end of the list...
		if (i == m_serverList.GetCount())
			break;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetNextServer() returns actually pointed server & move pointer to the next one in the list or NULL
//	if the list pointer is past the end of the list. (Right now there is no usability checking).
CServer* CServerList::GetNextServer(bool bOnlyObfuscated)
{
	CServer		*pNextServer = NULL;
	POSITION	posIndex;

	if (m_serverList.IsEmpty())
		return NULL;
	
	const uint32	dwSrvCount = static_cast<uint32>(m_serverList.GetCount());

//	If we're at the end or past the end of the list
//	move pointer to the begin of the list & return NULL as server
	if (m_dwServerPos >= dwSrvCount)
	{
		m_dwServerPos = 0;
		return NULL;
	}

//	Find next server
	while ((pNextServer == NULL) && (m_dwServerPos < dwSrvCount))
	{
		posIndex = m_serverList.FindIndex(m_dwServerPos);
		// Check if search position is still valid (could be corrupted by server delete operation)
		if (posIndex == NULL)
		{
			posIndex = m_serverList.GetHeadPosition();
			m_dwServerPos = 0;
		}

		m_dwServerPos++;	//	Move the pointer to the next one
		if (!bOnlyObfuscated || m_serverList.GetAt(posIndex)->SupportsObfuscationTCP())
			pNextServer = m_serverList.GetAt(posIndex);	//	Get actual server
	}

	return pNextServer;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer* CServerList::GetNextSearchServer()
{
	CServer		*nextserver = NULL;
	unsigned	ui = 0;

	while (!nextserver && (ui < static_cast<unsigned>(m_serverList.GetCount())))
	{
		POSITION posIndex = m_serverList.FindIndex(searchserverpos);
	//	Check if search position is still valid (could be corrupted by server delete operation)
		if (posIndex == NULL)
		{
			posIndex = m_serverList.GetHeadPosition();
			searchserverpos = 0;
		}
		nextserver = m_serverList.GetAt(posIndex);
		searchserverpos++;
		ui++;
		if (searchserverpos >= static_cast<unsigned>(m_serverList.GetCount()))
			searchserverpos = 0;
	}
	return nextserver;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer* CServerList::GetNextStatServer()
{
	CServer		*nextserver = NULL;
	unsigned	ui = 0;

	while (!nextserver && (ui < static_cast<unsigned>(m_serverList.GetCount())))
	{
		POSITION posIndex = m_serverList.FindIndex(statserverpos);
	//	Check if search position is still valid (could be corrupted by server delete operation)
		if (posIndex == NULL)
		{
			posIndex = m_serverList.GetHeadPosition();
			statserverpos = 0;
		}
		nextserver = m_serverList.GetAt(posIndex);
		statserverpos++;
		ui++;
		if (statserverpos >= static_cast<unsigned>(m_serverList.GetCount()))
			statserverpos = 0;
	}
	return nextserver;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer* CServerList::GetSuccServer(const CServer *pLastSrv) const
{
	if (m_serverList.IsEmpty())
		return NULL;
	if (pLastSrv == NULL)
		return m_serverList.GetHead();

	POSITION pos = m_serverList.Find(const_cast<CServer*>(pLastSrv));

	if (pos == NULL)
		return m_serverList.GetHead();

	m_serverList.GetNext(pos);
	if (pos == NULL)
		return NULL;
	return m_serverList.GetAt(pos);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer* CServerList::FindServerByAddress(const CServer& in_server, bool bCheckPort/* = true*/)
{
	CServer	   *pServer;

	for (POSITION pos = m_serverList.GetHeadPosition(); pos != NULL; )
	{
		pServer = m_serverList.GetNext(pos);
		if (pServer->HasSameAddress(in_server, bCheckPort))
			return pServer;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetServerByAddress() searches the server list for a server with stringified IP address 'strAddress'
//	and port 'nPort'. If none is found, NULL is returned.
CServer *CServerList::GetServerByAddress(const CString& strAddress, uint16 nPort)
{
//	Perform a linear search through the server list for a Server with the specified IP and port
	for (POSITION pos = m_serverList.GetHeadPosition(); pos != NULL;)
	{
		CServer	*pServer = m_serverList.GetNext(pos);

		if ((nPort == pServer->GetPort() || nPort == pServer->GetAuxPort() || nPort == 0) && pServer->GetAddress() == strAddress)
		{
			return pServer;
		}
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CServer* CServerList::GetServerByIP(uint32 nIP, uint16 port)
{
	for (POSITION pos = m_serverList.GetHeadPosition(); pos != NULL;)
	{
		CServer* s = m_serverList.GetNext(pos);
		if (s->GetIP() == nIP && (port == 0 || s->GetPort() == port))
			return s;
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerList::SaveServerMetToFile()
{
	CSingleLock AccessLock(&m_csServerMetFile, TRUE);

//	To save data safely CMemFile is used first to prepare the whole output (to reduce
//	number of filesystem I/O), then data is flushed to CFile in one shot.
//	Previous solution based on CStdioFile is not suitable for this case, as
//	CStdioFile can't generate proper exceptions due to the fact that it buffers
//	data before writing. As a result exceptions were not generated for many error cases
//	causing different file corruptions...
	CFile		file;
	CMemFile	MFile(8 * 1024);
	byte		byteServerListVersion = 0xE0;
	uint32		dwTmp, dwTagFilePos, dwServerCount = m_serverList.GetCount();
	ServerMet_Struct sCurSavingServer;
	void		*pBufBeg, *pBufEnd;
	CServer		*pNextServer;
	CString		strTmp;
	CWrTag		tagWr;

	MFile.Write(&byteServerListVersion, 1);
	MFile.Write(&dwServerCount, 4);

	for (uint32 j = 0; j < dwServerCount; j++)
	{
		dwTagFilePos = static_cast<uint32>(MFile.GetPosition()) + sizeof(ServerMet_StructShort);
		pNextServer = GetServerAt(j);
		sCurSavingServer.m_dwIP = pNextServer->GetIP();
		sCurSavingServer.m_uPort = pNextServer->GetPort();

		uint32	dwTagCnt = 0;
			
		sCurSavingServer.m_dwTagCount = dwTagCnt;
		MFile.Write(&sCurSavingServer, sizeof(ServerMet_Struct));

		strTmp = pNextServer->GetListName();
		if (!strTmp.IsEmpty())
		{
			if (IsUTF8Required(strTmp))
			{
				tagWr.WriteToFile(ST_SERVERNAME, strTmp, MFile, cfUTF8withBOM);
				dwTagCnt++;
			}
			tagWr.WriteToFile(ST_SERVERNAME, strTmp, MFile);
			dwTagCnt++;
		}
		strTmp = pNextServer->GetDynIP();
		if (!strTmp.IsEmpty())
		{
			if (IsUTF8Required(strTmp))
			{
				tagWr.WriteToFile(ST_DYNIP, strTmp, MFile, cfUTF8withBOM);
				dwTagCnt++;
			}
			tagWr.WriteToFile(ST_DYNIP, strTmp, MFile);
			dwTagCnt++;
		}
		strTmp = pNextServer->GetDescription();
		if (!strTmp.IsEmpty())
		{
			if (IsUTF8Required(strTmp))
			{
				tagWr.WriteToFile(ST_DESCRIPTION, strTmp, MFile, cfUTF8withBOM);
				dwTagCnt++;
			}
			tagWr.WriteToFile(ST_DESCRIPTION, strTmp, MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetFailedCount() != 0)
		{
			tagWr.WriteToFile(ST_FAIL, pNextServer->GetFailedCount(), MFile);
			dwTagCnt++;
		}
	//	Save server preferences using eDonkey correct values
		if ((dwTmp = pNextServer->eMule2ed2k(pNextServer->GetPreferences())) != 0)
		{
			tagWr.WriteToFile(ST_PREFERENCE, dwTmp, MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetNumUsers() != 0)
		{
			tagWr.WriteToFile("users", pNextServer->GetNumUsers(), MFile, false);
			dwTagCnt++;
		}
		if (pNextServer->GetFiles() != 0)
		{
			tagWr.WriteToFile("files", pNextServer->GetFiles(), MFile, false);
			dwTagCnt++;
		}
		if (pNextServer->GetPing() != 0)
		{
			tagWr.WriteToFile(ST_PING, pNextServer->GetPing(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetRealLastPingedTime() != 0)
		{
			tagWr.WriteToFile(ST_LASTPING, pNextServer->GetRealLastPingedTime(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetMaxUsers() != 0)
		{
			tagWr.WriteToFile(ST_MAXUSERS, pNextServer->GetMaxUsers(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetSoftMaxFiles() != 0)
		{
			tagWr.WriteToFile(ST_SOFTFILES, pNextServer->GetSoftMaxFiles(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetHardMaxFiles() != 0)
		{
			tagWr.WriteToFile(ST_HARDFILES, pNextServer->GetHardMaxFiles(), MFile);
			dwTagCnt++;
		}
		strTmp = pNextServer->GetVersion();
		if (!strTmp.IsEmpty())
		{
			tagWr.WriteToFile(ST_VERSION, strTmp, MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetUDPFlags() != 0)
		{
			tagWr.WriteToFile(ST_UDPFLAGS, pNextServer->GetUDPFlags(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetLowIDUsers() != 0)
		{
			tagWr.WriteToFile(ST_LOWIDUSERS, pNextServer->GetLowIDUsers(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetServerKeyUDPForce())
		{
			tagWr.WriteToFile(ST_UDPKEY, pNextServer->GetServerKeyUDPForce(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetServerKeyUDPIP())
		{
			tagWr.WriteToFile(ST_UDPKEYIP, pNextServer->GetServerKeyUDPIP(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetObfuscationPortTCP())
		{
			tagWr.WriteToFile(ST_TCPPORTOBFUSCATION, pNextServer->GetObfuscationPortTCP(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetObfuscationPortUDP())
		{
			tagWr.WriteToFile(ST_UDPPORTOBFUSCATION, pNextServer->GetObfuscationPortUDP(), MFile);
			dwTagCnt++;
		}
		if (pNextServer->GetAuxPort() != 0 && pNextServer->GetAuxPort() != pNextServer->GetPort())
		{
			tagWr.WriteToFile("AuxPort", pNextServer->GetAuxPort(), MFile, false);
			dwTagCnt++;
		}
	//	Save valid tag count
		MFile.Seek(dwTagFilePos, CFile::begin);
		MFile.Write(&dwTagCnt, 4);
		MFile.SeekToEnd();
	}
	MFile.SeekToBegin();

	strTmp.Format(_T("%sserver.met.new"), g_App.m_pPrefs->GetConfigDir());
	try
	{
		if (!file.Open(strTmp, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite))
		{
			AddLogLine(LOG_RGB_ERROR, IDS_ERROR_SAVEFILE, _T("server.met"));
			return false;
		}
	//	Steal stream buffer pointer and length without detaching it for easier destruction
		dwTmp = MFile.GetBufferPtr(CMemFile::bufferRead, ~0u, &pBufBeg, &pBufEnd);
		file.Write(pBufBeg, dwTmp);

		file.Close();	// Close can generate an exception as well - keep in try/catch
	}
	catch (CFileException *error)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_RGB_ERROR, IDS_ERROR_SAVEFILE2, _T("server.met"), GetErrorMessage(error));
		error->Delete();
	//	About is used instead of Close as it doesn't generate exception which we don't need here
		file.Abort();
 	//	Remove the partially written or otherwise damaged temporary file
 		::DeleteFile(strTmp);
		return false;
	}
	m_nLastSaved = ::GetTickCount();

	CString strCurServerMet, strOldServerMet;

	strCurServerMet.Format(_T("%sserver.met"), g_App.m_pPrefs->GetConfigDir());
	strOldServerMet.Format(_T("%sserver_met.old"), g_App.m_pPrefs->GetConfigDir());
	_tremove(strOldServerMet);
	_trename(strCurServerMet, strOldServerMet);
	_trename(strTmp, strCurServerMet);
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::LoadServersFromTextFile(const CString &strFilename)
{
	FILE	*pSL = _tfsopen(strFilename, _T("rb"), _SH_DENYWR);

	if (pSL == NULL)
		return;

	CString		strLine, strHost, strPriority, strPort;
	uint16		uPort = 0;

	fread(&uPort, sizeof(uint16), 1, pSL);
	if (uPort != 0xFEFFu)
	{
		fclose(pSL);
		if ((pSL = _tfsopen(strFilename, _T("r"), _SH_DENYWR)) == NULL)
			return;
	}

	while (!feof(pSL))
	{
		LPTSTR		pcLine = strLine.GetBuffer(1024);

		if (_fgetts(pcLine, 1024, pSL) == NULL)
			break;

		strLine.ReleaseBuffer();

		if (*pcLine == _T('#') || *pcLine == _T('/'))
			continue;

	//	Format is host:port,priority,Name
		if (strLine.GetLength() < 5)
			continue;

	//	Fetch host
		int pos = strLine.Find(_T(':'));
		if (pos < 0)
		{
			if ((pos = strLine.Find(_T(','))) < 0)
				continue;
		}
		strHost = strLine.Left(pos);
		strLine = strLine.Mid(pos + 1);

	//	Fetch port
		if ((pos = strLine.Find(_T(','))) < 0)
			continue;
		strPort = strLine.Left(pos);
		strLine = strLine.Mid(pos + 1);

	//	Fetch priority
		pos = strLine.Find(_T(','));

		uint32	dwPriority = PR_HIGH;

		if (pos == 1)
		{
			strPriority = strLine.Left(pos);
			dwPriority = _tstoi(strPriority);
			if (dwPriority > PR_HIGH)
				dwPriority = PR_HIGH;
			strLine = strLine.Mid(pos + 1);
		}

	//	Here only name is left in string
		strLine.Remove(_T('\r'));
		strLine.Remove(_T('\n'));

		uPort = static_cast<uint16>(_tstoi(strPort));

	//	Check existence of server, create server object add it to the list 
		CServer	*pExistingServer = GetServerByAddress(strHost, uPort);
		if (pExistingServer == NULL)
		{
			CServer	*pNewServer = new CServer(uPort, strHost);
			if (pNewServer != NULL)
			{
			//	Set properties
				pNewServer->SetListName(strLine);
				pNewServer->SetIsStaticMember(true);
				pNewServer->SetPreference(static_cast<byte>(dwPriority));
				if (!g_App.m_pMDlg->m_wndServer.m_ctlServerList.AddServer(pNewServer, true))
					delete pNewServer;
			}
		}
		else
		{
		//	Change properties
			pExistingServer->SetListName(strLine);
			pExistingServer->SetIsStaticMember(true);
			pExistingServer->SetPreference(static_cast<byte>(dwPriority));
			if (g_App.m_pMDlg->m_wndServer)
				g_App.m_pMDlg->m_wndServer.m_ctlServerList.RefreshServer(*pExistingServer);
		}
	}

	fclose(pSL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CServerList::SaveServersToTextFile()
{
	static const TCHAR s_acHeader[] = {
		_T("############################################\n")
		_T("# Static Servers File\n")
		_T("# enter one server per line\n")
		_T("#\n")
		_T("# Format:\n")
		_T("# ServerIP/Hostname:Port,Priority,ServerName\n")
		_T("#\n")
		_T("# Priority:\n")
		_T("# 2 = High, 1 = Normal, 0 = Low\n")
		_T("#\n")
		_T("# Examples:\n")
		_T("# 120.120.120.120:4661,0,TestServer1\n")
		_T("# myserver.yi.org:4662,1,TestServer2\n\n")
	};
	bool	bResult = false;
	const CServer *pSrv;
	CString strBuf(s_acHeader);

	for (POSITION pos = m_serverList.GetHeadPosition(); pos != NULL;)
	{
		pSrv = m_serverList.GetNext(pos);
		if (pSrv->IsStaticMember())
		{
			strBuf.AppendFormat(_T("%s:%u,%u,%s\n"),
								pSrv->GetAddress(), pSrv->GetPort(),
								pSrv->GetPreferences(), pSrv->GetListName());
		}
	}

#ifdef _UNICODE
	bool	bIsUnicode = IsUTF8Required(strBuf);
#else
	bool	bIsUnicode = false;
#endif
	FILE	*pStaticFile;
	CString	strStaticFilePath = g_App.m_pPrefs->GetConfigDir();
	int iWriteResult = 0;

	strStaticFilePath += CFGFILE_STATICSERVERS;
	if ((pStaticFile = _tfsopen(strStaticFilePath, (bIsUnicode) ? _T("wb") : _T("w"), _SH_DENYWR)) == NULL)
		return bResult;

	if (bIsUnicode)
	{
		iWriteResult = fputwc(0xFEFF, pStaticFile);
		strBuf.Replace(_T("\n"), _T("\r\n"));
	}

	if (iWriteResult != _TEOF)
	{
		if (_fputts(strBuf.GetString(), pStaticFile) != _TEOF)
			bResult = true;
	}

	fclose(pStaticFile);
	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::Process()
{
	if (::GetTickCount() - m_nLastSaved > MIN2MS(17))
		SaveServerMetToFile();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::ResetIP2Country()
{
	CServer *cur_server;

	for(POSITION pos = m_serverList.GetHeadPosition(); pos != NULL; m_serverList.GetNext(pos))
	{
		cur_server = m_serverList.GetAt(pos);
		cur_server->ResetIP2Country();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CServerList::CheckForExpiredUDPKeys()
{
	if (!g_App.m_pPrefs->IsServerCryptLayerUDPEnabled())
		return;

	uint32	dwKeysTotal = 0, dwKeysExpired = 0, dwPingDelayed = 0;
	uint32	dwDiff, dwNow = static_cast<uint32>(time(NULL));
	uint32	dwIP = g_App.GetPublicIP();
	CServer	*pSrv;

	for (POSITION pos = m_serverList.GetHeadPosition(); pos != NULL;)
	{
		pSrv = m_serverList.GetNext(pos);
		if (pSrv->SupportsObfuscationUDP())
		{
			if ((pSrv->GetServerKeyUDPForce() != 0) && (pSrv->GetServerKeyUDPIP() != dwIP))
			{
				dwKeysTotal++;
				dwKeysExpired++;
				if ((dwDiff = (dwNow - pSrv->GetRealLastPingedTime())) < UDPSERVSTATMINREASKTIME)
				{
					dwPingDelayed++;
				//	Next ping: Now + (MinimumDelay - already elapsed time)
					pSrv->SetLastPingedTime(dwNow - UDPSERVSTATREASKTIME + UDPSERVSTATMINREASKTIME - dwDiff);
				}
				else
					pSrv->SetLastPingedTime(0);
			}
			else if (pSrv->GetServerKeyUDP() != 0)
				dwKeysTotal++;
		}
	}

	AddLogLine(LOG_FL_DBG, _T("Possible IP Change - check for expired server UDP-keys: total keys %u, expired %u, pings forced %u, delayed pings %u"),
		dwKeysTotal, dwKeysExpired, dwKeysExpired - dwPingDelayed, dwPingDelayed); 
}
