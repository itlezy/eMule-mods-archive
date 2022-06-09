//this file is part of eMule
//Copyright (C)2003-2004 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "MMServer.h"
#include "MMsocket.h"
#include "PartFile.h"
#include "KnownFile.h"
#include "opcodes.h"
#include "MD5Sum.h"
#include "SearchDlg.h"
#include "packets.h"
#include "SearchList.h"
#include "WebServer.h"
#include "otherfunctions.h"
#include "server.h"

#ifdef OLD_SOCKETS_ENABLED

CMMServer::CMMServer(void)
{
	h_timer = NULL;
	m_cPWFailed = 0;
	m_dwBlocked = 0;
	m_pSocket = NULL;
	m_nSessionID = 0;
	m_byPendingCommand = 0;
	m_pPendingCommandSocket = NULL;
	m_bUseFakeContent = false;
	m_nMaxDownloads = 0;
	m_nMaxBufDownloads = 0;
	m_bGrabListLogin = false;
}

CMMServer::~CMMServer(void)
{
	DeleteSearchFiles();
	delete m_pSocket;
	if (h_timer != NULL)
	{
		KillTimer(0,h_timer);
	}
}

void CMMServer::Init()
{
	if (g_App.m_pPrefs->IsMMServerEnabled() && !m_pSocket)
	{
		m_pSocket = new CListenMMSocket(this);
		if (!m_pSocket->Create())
		{
			StopServer();
			g_App.m_pMDlg->AddLogLine(LOG_RGB_ERROR, IDS_MMFAILED);
		}
		else
		{
			g_App.m_pMDlg->AddLogLine(0, IDS_MMSTARTED, g_App.m_pPrefs->GetMMPort(), _T(MM_STRVERSION));
		}
	}
}

void CMMServer::StopServer()
{
	delete m_pSocket;
	m_pSocket = NULL;
}

void CMMServer::DeleteSearchFiles()
{
	for (int i = 0; i != m_SendSearchList.GetSize(); i++)
	{
		delete m_SendSearchList[i];
	}
	m_SendSearchList.SetSize(0);
}

bool CMMServer::PreProcessPacket(char* pPacket, uint32 nSize, CMMSocket* sender)
{
	if (nSize >= 3)
	{
		uint16 nSessionID;
		memcpy2(&nSessionID, pPacket + 1, sizeof(nSessionID));
		if ((m_nSessionID && nSessionID == m_nSessionID) || pPacket[0] == MMP_HELLO)
		{
			return true;
		}
		else
		{
			CMMPacket* packet = new CMMPacket(MMP_INVALIDID);
			sender->SendPacket(packet);
			m_nSessionID = 0;
			return false;
		}
	}

	CMMPacket* packet = new CMMPacket(MMP_GENERALERROR);
	sender->SendPacket(packet);
	return false;
}

void CMMServer::ProcessHelloPacket(CMMData* data, CMMSocket* sender)
{
	CMMPacket* packet = new CMMPacket(MMP_HELLOANS);
	if (data->ReadByte() != MM_VERSION)
	{
		packet->WriteByte(MMT_WRONGVERSION);
		sender->SendPacket(packet);
		return;
	}
	else
	{
		if(m_dwBlocked && m_dwBlocked > ::GetTickCount())
		{
			packet->WriteByte(MMT_WRONGPASSWORD);
			sender->SendPacket(packet);
			return;
		}
		byte	abyteDigest[16];
		CString	strPlainPW = data->ReadString();

		if ( (md4cmp(MD5Sum(strPlainPW, abyteDigest), g_App.m_pPrefs->GetMMPass()) != 0) ||
			strPlainPW.IsEmpty() )
		{
			m_dwBlocked = 0;
			packet->WriteByte(MMT_WRONGPASSWORD);
			sender->SendPacket(packet);
			m_cPWFailed++;
			if (m_cPWFailed == 3)
			{
				g_App.m_pMDlg->AddLogLine(LOG_RGB_WARNING, IDS_MM_LOGIN_WARNING);
				m_cPWFailed = 0;
				m_dwBlocked = ::GetTickCount() + MMS_BLOCKTIME;
			}
			return;
		}
		else
		{
			m_bUseFakeContent = (data->ReadByte() != 0);
			m_nMaxDownloads = data->ReadShort();
			m_nMaxBufDownloads = data->ReadShort();
			m_bGrabListLogin = (data->ReadByte() != 0);
		//	Everything is ok, new session id
			g_App.m_pMDlg->AddLogLine(LOG_RGB_NOTICE, IDS_MM_SUCCESS_LOGIN);
			packet->WriteByte(MMT_OK);
			m_nSessionID = static_cast<uint16>(rand());
			packet->WriteShort(m_nSessionID);
			packet->WriteString(g_App.m_pPrefs->GetUserNick());
			packet->WriteShort(g_App.m_pPrefs->GetMaxUpload() / 10);
			packet->WriteShort((g_App.m_pPrefs->GetMaxDownload() == UNLIMITED) ? 0 : (g_App.m_pPrefs->GetMaxDownload() / 10));
			ProcessStatusRequest(sender,packet);
		}
	}
}

void CMMServer::ProcessStatusRequest(CMMSocket* sender, CMMPacket* packet)
{
	if (packet == NULL)
		packet = new CMMPacket(MMP_STATUSANSWER);
	else
		packet->WriteByte(MMP_STATUSANSWER);

	packet->WriteShort((uint16)g_App.m_pUploadQueue->GetDataRate()/100);
	packet->WriteShort((uint16)((g_App.m_pPrefs->GetMaxGraphUploadRate()*1024)/1000));
	packet->WriteShort((uint16)g_App.m_pDownloadQueue->GetDataRate()/100);
	packet->WriteShort((uint16)((g_App.m_pPrefs->GetMaxGraphDownloadRate()*1024)/1000));
	packet->WriteByte((byte)g_App.m_pDownloadQueue->GetTransferringFiles());
	packet->WriteByte((byte)g_App.m_pDownloadQueue->GetPausedFileCount());
	packet->WriteInt(static_cast<uint32>(g_App.stat_sessionReceivedBytes / 1048576ui64));
	packet->WriteShort((uint16)((g_App.m_pMDlg->m_dlgStatistics.GetAvgDownloadRate(0)*1024)/100));
	if (g_App.m_pServerConnect->IsConnected())
	{
		if(g_App.m_pServerConnect->IsLowID())
			packet->WriteByte(1);
		else
			packet->WriteByte(2);

		CServer	*pCurServer;

		if ((pCurServer = g_App.m_pServerConnect->GetCurrentServer()) != NULL)
		{
			packet->WriteInt(pCurServer->GetNumUsers());
			packet->WriteString(pCurServer->GetListName());
		}
		else
		{
			packet->WriteInt(0);
			packet->WriteString("");
		}
	}
	else
	{
		packet->WriteByte(0);
		packet->WriteInt(0);
		packet->WriteString("");
	}

//	Kademlia isn't supported by eMule Plus - inform the MobileMule client
	packet->WriteByte(0);
	packet->WriteInt(0);

	sender->SendPacket(packet);
}

void CMMServer::ProcessFileListRequest(CMMSocket* sender, CMMPacket* packet)
{
	if (packet == NULL)
		packet = new CMMPacket(MMP_FILELISTANS);
	else
		packet->WriteByte(MMP_FILELISTANS);

	int	nCount = CCat::GetNumUserCats() + 1;

	packet->WriteByte(static_cast<byte>(nCount));
	packet->WriteString(CCat::GetPredefinedCatTitle(CAT_ALL));
	for (int i = 1; i < nCount; i++)
	{
		packet->WriteString(CCat::GetCatByUserIndex(i)->GetTitle());
	}

	nCount = g_App.m_pDownloadQueue->GetFileCount();
	if (nCount > m_nMaxDownloads)
		nCount = m_nMaxDownloads;
	m_SentFileList.SetSize(nCount);
	packet->WriteByte(static_cast<byte>(nCount));
	for (int i = 0; i < nCount; i++)
	{
	//	While this is not the fastest method to trace this list, it's not timecritical here
		CPartFile* cur_file = g_App.m_pDownloadQueue->GetFileByIndex(i);
		if (cur_file == NULL)
		{
			delete packet;
			packet = new CMMPacket(MMP_GENERALERROR);
			sender->SendPacket(packet);
			ASSERT(false);
			return;
		}
		m_SentFileList[i] = cur_file;
		if (cur_file->IsPaused())
			packet->WriteByte(MMT_PAUSED);
		else
		{
			if (cur_file->GetTransferringSrcCount() > 0)
				packet->WriteByte(MMT_DOWNLOADING);
			else
				packet->WriteByte(MMT_WAITING);
		}

		packet->WriteString(cur_file->GetFileName());
		packet->WriteByte(static_cast<byte>(CCat::GetUserCatIndexByID(cur_file->GetCatID())));

		if (i < m_nMaxBufDownloads)
		{
			packet->WriteByte(1);
			WriteFileInfo(cur_file, packet);
		}
		else
		{
			packet->WriteByte(0);
		}
	}
	sender->SendPacket(packet);
}

void CMMServer::ProcessFinishedListRequest(CMMSocket* sender)
{
	CMMPacket  *packet = new CMMPacket(MMP_FINISHEDANS);
	int			nCount = CCat::GetNumUserCats() + 1;

	packet->WriteByte(static_cast<byte>(nCount));
	packet->WriteString(CCat::GetPredefinedCatTitle(CAT_ALL));
	for (int i = 1; i < nCount; i++)
	{
		packet->WriteString(CCat::GetCatByUserIndex(i)->GetTitle());
	}

	nCount = (m_SentFinishedList.GetCount() > 30)? 30 : m_SentFinishedList.GetCount();
	packet->WriteByte(static_cast<byte>(nCount));
	for (int i = 0; i < nCount; i++)
	{
		CKnownFile	*cur_file = m_SentFinishedList[i];

		packet->WriteByte(0xFF);
		packet->WriteString(cur_file->GetFileName());

		CPartFile	*pPartFile = dynamic_cast<CPartFile*>(cur_file);

		if (pPartFile != NULL)
			packet->WriteByte(static_cast<byte>(CCat::GetUserCatIndexByID(pPartFile->GetCatID())));
		else
			packet->WriteByte(0);
	}
	sender->SendPacket(packet);
}

void CMMServer::ProcessFileCommand(CMMData* data, CMMSocket* sender)
{
	byte byCommand = data->ReadByte();
	byte byFileIndex = data->ReadByte();
	if (byFileIndex >= m_SentFileList.GetSize()
		|| !g_App.m_pDownloadQueue->IsInDLQueue(m_SentFileList[byFileIndex]))
	{
		CMMPacket* packet = new CMMPacket(MMP_GENERALERROR);
		sender->SendPacket(packet);
		ASSERT ( false );
		return;
	}
	CPartFile* selFile = m_SentFileList[byFileIndex];
	switch (byCommand)
	{
		case MMT_PAUSE:
			selFile->PauseFile();
			break;
		case MMT_RESUME:
			selFile->ResumeFile();
			break;
		case MMT_CANCEL:
		{
			switch(selFile->GetStatus())
			{
				case PS_WAITINGFORHASH:
				case PS_HASHING:
				case PS_COMPLETING:
				case PS_COMPLETE:
					break;
				default:
					if (g_App.m_pPrefs->StartDownloadPaused())
						g_App.m_pDownloadQueue->StartNextFile(CCat::GetAllCatType());
				case PS_PAUSED:
				case PS_STOPPED:
					selFile->DeleteFile();
					g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();
					break;
			}
			break;
		}
		default:
			CMMPacket* packet = new CMMPacket(MMP_GENERALERROR);
			sender->SendPacket(packet);
			return;
	}
	CMMPacket* packet = new CMMPacket(MMP_FILECOMMANDANS);
	ProcessFileListRequest(sender,packet);
}

void  CMMServer::ProcessDetailRequest(CMMData* data, CMMSocket* sender)
{
	byte byFileIndex = data->ReadByte();
	if (byFileIndex >= m_SentFileList.GetSize()
		|| !g_App.m_pDownloadQueue->IsInDLQueue(m_SentFileList[byFileIndex]))
	{
		CMMPacket* packet = new CMMPacket(MMP_GENERALERROR);
		sender->SendPacket(packet);
		ASSERT ( false );
		return;
	}
	CPartFile* selFile = m_SentFileList[byFileIndex];
	CMMPacket* packet = new CMMPacket(MMP_FILEDETAILANS);
	WriteFileInfo(selFile, packet);
	sender->SendPacket(packet);
}

void  CMMServer::ProcessCommandRequest(CMMData* data, CMMSocket* sender)
{
	byte byCommand = data->ReadByte();
	bool bSuccess = false;
	bool bQueueCommand = false;
	switch (byCommand)
	{
		case MMT_SDEMULE:
		case MMT_SDPC:
			h_timer = SetTimer(0,0,5000,CommandTimer);
			if (h_timer)
				bSuccess = true;
			bQueueCommand = true;
			break;
		case MMT_SERVERCONNECT:
			g_App.m_pServerConnect->ConnectToAnyServer();
			bSuccess = true;
			break;
	}
	if (bSuccess)
	{
		CMMPacket* packet = new CMMPacket(MMP_COMMANDANS);
		sender->SendPacket(packet);
		if (bQueueCommand)
			m_byPendingCommand = byCommand;
	}
	else
	{
		CMMPacket* packet = new CMMPacket(MMP_GENERALERROR);
		sender->SendPacket(packet);
		ASSERT ( false );
		return;
	}
}

void  CMMServer::ProcessSearchRequest(CMMData* data, CMMSocket* sender)
{
	DeleteSearchFiles();

	if (!g_App.m_pServerConnect->IsConnected())
	{
		CMMPacket	   *packet = new CMMPacket(MMP_SEARCHANS);

		packet->WriteByte(MMT_NOTCONNECTED);
		sender->SendPacket(packet);
		return;
	}

	CString	strSearch = data->ReadString();
	byte	byType = data->ReadByte();
	UINT	dwStringID = IDS_SEARCH_ANY;

	switch (byType)
	{
		case 1:
			dwStringID = IDS_SEARCH_ARC;
			break;
		case 2:
			dwStringID = IDS_SEARCH_AUDIO;
			break;
		case 3:
			dwStringID = IDS_SEARCH_CDIMG;
			break;
		case 4:
			dwStringID = IDS_SEARCH_PRG;
			break;
		case 5:
			dwStringID = IDS_SEARCH_VIDEO;
			break;
	}

	CString	strLocalSearchType = GetResString(dwStringID);

	h_timer = SetTimer(0,0,20000,CommandTimer);
	if (h_timer == NULL)
	{
		CMMPacket	   *packet = new CMMPacket(MMP_GENERALERROR);

		sender->SendPacket(packet);
		ASSERT ( false );
		return;
	}

	m_byPendingCommand = MMT_SEARCH;
	m_pPendingCommandSocket = sender;

	g_App.m_pSearchList->NewSearch(NULL, strLocalSearchType , MMS_SEARCHID, true);
	g_App.m_pMDlg->m_dlgSearch.DoNewEd2kSearch(strSearch, strLocalSearchType, 0, 0, 0, _T(""), false, _T(""), MMS_SEARCHID);

	char	buffer[500];
	int		iBuffLen = wsprintfA(buffer, "HTTP/1.1 200 OK\r\nConnection: Close\r\nContent-Type: %s\r\n", GetContentType());

	sender->Send(buffer, iBuffLen);
}


void  CMMServer::ProcessChangeLimitRequest(CMMData* data, CMMSocket* sender)
{
	uint16 uNewUpload = data->ReadShort();
	uint16 uNewDownload = data->ReadShort();

	g_App.m_pPrefs->SetMaxUploadWithCheck(uNewUpload * 10);
	if (uNewDownload != UNLIMITED)
		g_App.m_pPrefs->SetMaxDownloadWithCheck(uNewDownload * 10);
	else
		g_App.m_pPrefs->SetMaxDownload(uNewDownload);

	CMMPacket* packet = new CMMPacket(MMP_CHANGELIMITANS);
	packet->WriteShort(g_App.m_pPrefs->GetMaxUpload() / 10);
	packet->WriteShort((g_App.m_pPrefs->GetMaxDownload() == UNLIMITED) ? 0 : (g_App.m_pPrefs->GetMaxDownload() / 10));
	sender->SendPacket(packet);
}


void CMMServer::SearchFinished(bool bTimeOut)
{
#define MAXRESULTS	20
	if (h_timer != 0)
	{
		KillTimer(0,h_timer);
		h_timer = 0;
	}
	if (m_pPendingCommandSocket == NULL)
		return;
	if (bTimeOut)
	{
		CMMPacket* packet = new CMMPacket(MMP_SEARCHANS);
		packet->WriteByte(MMT_TIMEDOUT);
		packet->m_bSpecialHeader = true;
		m_pPendingCommandSocket->SendPacket(packet);
	}
	else if (g_App.m_pSearchList->GetFoundFiles(MMS_SEARCHID) == 0)
	{
		CMMPacket* packet = new CMMPacket(MMP_SEARCHANS);
		packet->WriteByte(MMT_NORESULTS);
		packet->m_bSpecialHeader = true;
		m_pPendingCommandSocket->SendPacket(packet);
	}
	else
	{
		unsigned uiResults = g_App.m_pSearchList->GetFoundFiles(MMS_SEARCHID);
		if (uiResults > MAXRESULTS)
			uiResults = MAXRESULTS;
		m_SendSearchList.SetSize(uiResults);
		CMMPacket* packet = new CMMPacket(MMP_SEARCHANS);
		packet->m_bSpecialHeader = true;
		packet->WriteByte(MMT_OK);
		packet->WriteByte(static_cast<byte>(uiResults));
		for (unsigned ui = 0; ui < uiResults; ui++)
		{
			CSearchFile* cur_file = g_App.m_pSearchList->DetachNextFile(MMS_SEARCHID);
			m_SendSearchList[ui] = cur_file;
			packet->WriteString(cur_file->GetFileName());
			packet->WriteShort(static_cast<uint16>(cur_file->GetSourceCount()));
			packet->WriteInt64(cur_file->GetFileSize());
		}
		m_pPendingCommandSocket->SendPacket(packet);
		g_App.m_pMDlg->m_dlgSearch.DeleteSearch(MMS_SEARCHID);
	}
	m_pPendingCommandSocket = NULL;
}

void  CMMServer::ProcessDownloadRequest(CMMData* data, CMMSocket* sender)
{
	byte byFileIndex = data->ReadByte();

	if (byFileIndex >= m_SendSearchList.GetSize() )
	{
		CMMPacket* packet = new CMMPacket(MMP_GENERALERROR);
		sender->SendPacket(packet);
		ASSERT ( false );
		return;
	}

	CSearchFile* todownload = m_SendSearchList[byFileIndex];

	g_App.m_pDownloadQueue->AddSearchToDownload(todownload, CAT_NONE, g_App.m_pPrefs->StartDownloadPaused());

	CMMPacket* packet = new CMMPacket(MMP_DOWNLOADANS);

	if (g_App.m_pDownloadQueue->GetFileByID(todownload->GetFileHash()) != NULL)
	{
		packet->WriteByte(MMT_OK);
	}
	else
	{
		packet->WriteByte(MMT_FAILED);
	}
	sender->SendPacket(packet);
}

//	Preview isn't supported in eMule Plus - return an error
void  CMMServer::ProcessPreviewRequest(CMMData *data, CMMSocket *sender)
{
	CMMPacket	*packet = new CMMPacket(MMP_PREVIEWANS);
	NOPRM(data);

	packet->WriteByte(MMT_FAILED);
	sender->SendPacket(packet);
	return;
}

void CMMServer::Process()
{
	if (m_pSocket)
		m_pSocket->Process();
}

const char* CMMServer::GetContentType()
{
	return (m_bUseFakeContent) ? "image/vnd.wap.wbmp" : "application/octet-stream";
}

VOID CALLBACK CMMServer::CommandTimer(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime)
{
	NOPRM(hwnd); NOPRM(uMsg); NOPRM(idEvent); NOPRM(dwTime);
	// NOTE: Always handle all type of MFC exceptions in TimerProcs - otherwise we'll get mem leaks
	EMULE_TRY

	KillTimer(0, g_App.m_pMMServer->h_timer);
	g_App.m_pMMServer->h_timer = 0;
	switch(g_App.m_pMMServer->m_byPendingCommand)
	{
		case MMT_SDPC:
		{
			HANDLE hToken;
			TOKEN_PRIVILEGES tkp;	// Get a token for this process.
			try
			{
				if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
					throw;
				LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
				tkp.PrivilegeCount = 1;  // one privilege to set
				tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;	// Get the shutdown privilege for this process.
				AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
			}
			catch(...)
			{
				g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_NOTICE, GetResString(IDS_WEB_SHUTDOWN) + _T(' ') + GetResString(IDS_FAILED));
			}
			if (!ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0))
				break;
		}
		case MMT_SDEMULE:
			g_App.m_app_state = g_App.APP_STATE_SHUTTINGDOWN;
			SendMessage(g_App.m_pMDlg->m_hWnd, WM_CLOSE, 0, 0);
			break;
		case MMT_SEARCH:
			g_App.m_pMMServer->SearchFinished(true);
			break;
	}

	EMULE_CATCH
}

void  CMMServer::ProcessStatisticsRequest(CMMData* data, CMMSocket* sender)
{
	uint16 nWidth = data->ReadShort();
	CArray<UpDown>* rawData = g_App.m_pWebServer->GetPointsForWeb();
	int nRawDataSize = rawData->GetSize();
	uint32 dwCompressEvery = (nRawDataSize > nWidth) ? static_cast<uint32>(nRawDataSize) / nWidth : 1;
	int nPos = (nRawDataSize > nWidth) ? (nRawDataSize % nWidth) : 0;
	uint32 dwAddUp, dwAddDown, dwAddCon, i, i2;
	ASSERT(nPos + dwCompressEvery * nWidth == nRawDataSize || (nPos == 0 && nRawDataSize < nWidth));

	CMMPacket* packet = new CMMPacket(MMP_STATISTICSANS);
	packet->WriteShort(static_cast<uint16>((nRawDataSize - nPos) * g_App.m_pPrefs->GetTrafficOMeterInterval()));
	packet->WriteShort(static_cast<uint16>(min(nWidth, nRawDataSize)));
	while (nPos < nRawDataSize)
	{
		dwAddUp = dwAddDown = dwAddCon = 0;
		for (i = 0; i < dwCompressEvery; i++)
		{
			if (nPos >= nRawDataSize)
			{
				ASSERT ( false );
				break;
			}
			else
			{
				dwAddUp += rawData->ElementAt(nPos).dwDownRate;
				dwAddDown += rawData->ElementAt(nPos).dwUpRate;
				dwAddCon += rawData->ElementAt(nPos).dwConnections;
			}
			nPos++;
		}
		i2 = i >> 1;
		packet->WriteInt((dwAddUp + i2) / i);
		packet->WriteInt((dwAddDown + i2) / i);
		packet->WriteShort(static_cast<uint16>((dwAddCon + i2) / i));
	}
	ASSERT( nPos == nRawDataSize );
	sender->SendPacket(packet);
}

void CMMServer::WriteFileInfo(CPartFile* selFile, CMMPacket* packet)
{
	packet->WriteInt64(selFile->GetFileSize());
	packet->WriteInt64(selFile->GetTransferred());
	packet->WriteInt64(selFile->GetCompletedSize());
	packet->WriteShort(static_cast<uint16>(selFile->GetDataRate() / 100));
	packet->WriteShort(selFile->GetSourceCount());
	packet->WriteShort(selFile->GetTransferringSrcCount());
	if (selFile->IsAutoPrioritized())
	{
		packet->WriteByte(4);
	}
	else
	{
		packet->WriteByte(selFile->GetPriority());
	}
	byte	*pbyteParts = selFile->MMCreatePartStatus();
	uint32	dwPartCnt = selFile->GetPartCount();

	packet->WriteShort(static_cast<uint16>(dwPartCnt));
	packet->WriteMem(pbyteParts, dwPartCnt);
	delete []pbyteParts;
}
#endif //OLD_SOCKETS_ENABLED
