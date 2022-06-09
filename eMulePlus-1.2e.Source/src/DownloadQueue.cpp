//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "opcodes.h"
#include "DownloadQueue.h"
#include "server.h"
#include "ServerList.h"
#include "updownclient.h"
#include "PartFile.h"
#include "ED2KLink.h"
#include "otherfunctions.h"
#include "SharedFileList.h"
#include "IPFilter.h"
#include "emule.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

static const uint32 g_dwZeroConst = 0;

BEGIN_MESSAGE_MAP(CHostnameSourceWnd, CWnd)
	ON_MESSAGE(TM_SOURCEHOSTNAMERESOLVED, OnSourceHostnameResolved)
	ON_WM_TIMER()
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHostnameSourceWnd::CHostnameSourceWnd()
{
	m_pOwner = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CHostnameSourceWnd::OnSourceHostnameResolved(WPARAM wParam, LPARAM lParam)
{
	if (m_pOwner)
		m_pOwner->SourceHostnameResolved(wParam, lParam);

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadQueue::CDownloadQueue(CSharedFileList *in_pSharedFileList)
{
	m_pSharedFileList = in_pSharedFileList;
	m_dwDataRate = 0;
	pCurUDPServer = NULL;
	m_dwLastUDPSearchTime = 0;
	m_dwLastUDPStatTime = 0;
	m_pLastUDPSearchedFile = NULL;
	m_dwAverDataRate = 0;
	m_uiUDCounter = 0;
	m_dwSumDownDataOverheadInDeque = 0;
	m_nDownDataRateMSOverhead = 0;
	m_nDownDataRateOverhead = 0;
	m_nDownDataOverheadSourceExchange = 0;
	m_nDownDataOverheadFileRequest = 0;
	m_nDownDataOverheadOther = 0;
	m_nDownDataOverheadServer = 0;
	m_nDownDataOverheadSourceExchangePackets = 0;
	m_nDownDataOverheadFileRequestPackets = 0;
	m_nDownDataOverheadOtherPackets = 0;
	m_nDownDataOverheadServerPackets = 0;
	m_A4AF_auto_file = NULL;
	m_bIsInitialized = false;
	m_dwLastTCPSourcesRequestTime = 0;
	m_pLastTCPSrcReqServer = NULL;
	m_dwBannedCounter = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CDownloadQueue::~CDownloadQueue()
{
	EMULE_TRY

//--- xrmb:keepPartFileStats ---
	SaveAllPartFileStats();
//--- :xrmb ---

	while (!m_partFileList.IsEmpty())
	{
		CPartFile* pPartFile = m_partFileList.RemoveHead();

		if (pPartFile != NULL)
			delete pPartFile;
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::AddPartFilesToShare()
{
	EMULE_TRY

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		CPartFile	* pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile && pPartFile->GetRawStatus() == PS_READY)
			m_pSharedFileList->SafeAddKnownFile(pPartFile, true);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::CompDownDataRateOverhead()
{
	EMULE_TRY

//	update the list & sum
	m_DownDataOverheadDeque.push_back(m_nDownDataRateMSOverhead);
	m_dwSumDownDataOverheadInDeque += m_nDownDataRateMSOverhead;

//	reset the data
	m_nDownDataRateMSOverhead = 0;

//	chech deque length
	while (m_DownDataOverheadDeque.size() > 150)
	{
		m_dwSumDownDataOverheadInDeque -= m_DownDataOverheadDeque.front();
		m_DownDataOverheadDeque.pop_front();
	}

//	calculate average value
	if (m_DownDataOverheadDeque.size() > 10)
		m_nDownDataRateOverhead = 10 * m_dwSumDownDataOverheadInDeque/m_DownDataOverheadDeque.size();
	else
		m_nDownDataRateOverhead = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::Init()
{
	EMULE_TRY

//	Find all part files, read & hash them if needed and store them into a list
	uint16	iCount = 0;
	CString	strTempDir = g_App.m_pPrefs->GetTempDir();
	CStringList	tmpTempDirList;	//	list elements will be deleted in list destructor

//	Make local copy to prevent long locking of list resource
	g_App.m_pPrefs->TempDirListCopy(&tmpTempDirList);
//	Add the main temporary directory to the temporary directories list for easier processing
	strTempDir += _T('\\');
	tmpTempDirList.AddHead(strTempDir);

//	For each temp dir...
	for (POSITION pos = tmpTempDirList.GetHeadPosition(); pos != NULL; )
	{
		CFileFind		ff;
		CString			strTempDirPath(tmpTempDirList.GetNext(pos));
		CString			strSearchPath = strTempDirPath;

		strSearchPath += _T("*.part.met");

	//	Check all part.met files
		bool	bEnd = !ff.FindFile(strSearchPath, 0);

	//	Until we can't find any more .met files...
		while (!bEnd)
		{
			bEnd = !ff.FindNextFile();
		//	If, for some reason, there's a .met directory, skip it
			if (ff.IsDirectory())
				continue;

			CPartFile		*pPartFile = new CPartFile();

		//	Try to load the part file. If we were successful...
		
			bool bLoadPartFileStatus = pPartFile->LoadPartFile(strTempDirPath, ff.GetFileName());

		//	Try recovering part.met.bak file if load failed before
			if (!bLoadPartFileStatus)
				bLoadPartFileStatus = pPartFile->TryToRecoverPartFile(strTempDirPath, ff.GetFileName());

			if (bLoadPartFileStatus)
			{
				iCount++;
			//	Check if is an A4AF auto file
				if (md4cmp(pPartFile->GetFileHash(), g_App.m_pPrefs->GetA4AFHash()) == 0)
					g_App.m_pDownloadQueue->SetA4AFAutoFile(pPartFile);

			//	Add it to the download queue
				m_partFileList.AddTail(pPartFile);

			//	PartFiles are always shared files
				if (pPartFile->GetRawStatus() == PS_READY)
					m_pSharedFileList->SafeAddKnownFile(pPartFile);

				g_App.m_pDownloadList->AddFile(pPartFile);

			//	Load SLS sources on PartFile loading
				m_sourcesaver.LoadSources(pPartFile);
			}
			else
			{
				delete pPartFile;
			}
		}
		ff.Close();
	}

	if (iCount == 0)
		AddLogLine(0, IDS_NOPARTSFOUND);
	else
	{
		AddLogLine(0, IDS_FOUNDPARTS, iCount);
		SortByPriority();
	}

//--- xrmb:keepPartFileStats ---
	m_lastPartFileStatsSave = GetTickCount();
//--- :xrmb ---

	m_wndHSCallback = NULL;
	sourceHostnameResolveRetry = 0;
	m_bIsResolving = false;
	m_bIsInitialized = true;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::AddSearchToDownload(CSearchFile *pNewSearchFile, EnumCategories eCatID/*=CAT_NONE*/, bool bPaused/*=false*/)
{
	EMULE_TRY

	if (pNewSearchFile->GetFileSize() == 0)
	{
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_SKIPZEROLENGTHFILE, pNewSearchFile->GetFileName());
		return;
	}
	if (FileExists(pNewSearchFile->GetFileHash()))
		return;

	CPartFile	*pNewPartFile = new CPartFile(pNewSearchFile, eCatID);

	if (pNewPartFile->GetStatus() == PS_ERROR)
	{
		delete pNewPartFile;
		return;
	}
	AddDownload(pNewPartFile, bPaused);

	if (pNewSearchFile->GetClientHybridID() != 0 && pNewSearchFile->GetClientPort() != 0)
	{
	//	If the search result is from a client ('View Files') add that client as source
		if (pNewSearchFile->GetType() == SFT_CLIENT)
		{
			CClientSource	source;

			source.dwSrcIDHybrid = pNewSearchFile->GetClientHybridID();
			source.sourcePort = pNewSearchFile->GetClientPort();
			source.serverIP = pNewSearchFile->GetClientServerIP();
			source.serverPort = pNewSearchFile->GetClientServerPort();

			if (pNewPartFile != NULL)
				pNewPartFile->AddClientSource(&source, 0, false, 0);
			else
			{
				CPartFile	*pPartFile = GetFileByID(pNewSearchFile->GetFileHash());

				if (pPartFile != NULL)
					pPartFile->AddClientSource(&source, 0, false, 0);
			}
		}
		else
		{
		//	If the search result is from OP_GLOBSEARCHRES there may also be a source
			CMemFile	sourcesFile(16);
			uint32		dwClientID;
			uint16		uPort;
			byte		byteSources = 1;

			sourcesFile.Write(&byteSources, 1);
			dwClientID = pNewSearchFile->GetClientID();
			sourcesFile.Write(&dwClientID, 4);
			uPort = pNewSearchFile->GetClientPort();
			sourcesFile.Write(&uPort, 2);

			sourcesFile.SeekToBegin();
			pNewPartFile->AddServerSources(sourcesFile, pNewSearchFile->GetClientServerIP(), pNewSearchFile->GetClientServerPort(), false);
		}
	}

	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::AddFileLinkToDownload(CED2KFileLink *pLink, EnumCategories eCatID)
{
	EMULE_TRY

	CPartFile	*pNewPartFile = new CPartFile(pLink, eCatID);

	if (pNewPartFile->GetStatus() == PS_ERROR)
	{
		delete pNewPartFile;
		pNewPartFile = NULL;
	}
	else
	{
		AddDownload(pNewPartFile, g_App.m_pPrefs->StartDownloadPaused());
	}

	if (pLink->HasValidSources())
	{
		if (pNewPartFile)
		{
			if (pLink->ClientSourcesList)
			//	Add all IP sources and eliminate them from list
				pNewPartFile->AddClientSources(pLink->ClientSourcesList);
		}
		else
		{
			CPartFile	*pPartFile = GetFileByID(pLink->GetHashKey());

			if (pPartFile && pLink->ClientSourcesList)
			//	Add all IP sources and eliminate them from list
				pPartFile->AddClientSources(pLink->ClientSourcesList);
		}
	}
	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	StartNextFile() starts a paused download preferably in the same category (takes in account priority)
void CDownloadQueue::StartNextFile(EnumCategories eCatID)
{
	EMULE_TRY

	CPartFile	*pPartFile, *pTempPartFile;
	CPartFile	*pStartFile = NULL;
	POSITION	pos;

	bool bFoundInSameCat = false;

//	Try to find a paused file in the same category
	for (pos = m_partFileList.GetHeadPosition(); pos != NULL; )
	{
		pTempPartFile = m_partFileList.GetNext(pos);
		if (CCat::FileBelongsToGivenCat(pTempPartFile, eCatID) && pTempPartFile->GetStatus() == PS_PAUSED)
		{
			bFoundInSameCat = true;
			break;
		}
	}
	for (pos = m_partFileList.GetHeadPosition(); pos != NULL; )
	{
		pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile && pPartFile->GetStatus() == PS_PAUSED && ((!bFoundInSameCat && g_App.m_pPrefs->IsResumeOtherCat())
			|| (bFoundInSameCat &&  CCat::FileBelongsToGivenCat(pPartFile, eCatID))))
		{
		//	If no start file has been found or 'pPartFile' is higher priority...
		//  and now it's same priority but next in file order (ie. 1/2/3 in a series)
			if (pStartFile == NULL || pStartFile->GetPriority() < pPartFile->GetPriority()
				|| ( pStartFile->GetPriority() == pPartFile->GetPriority()
				    && (pStartFile->CmpFileNames(pPartFile->GetFileName()) > 0) ) )
				pStartFile = pPartFile;
		}
	}

	if (pStartFile != NULL)
		pStartFile->ResumeFile();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::AddDownload(CPartFile *pNewPartFile, bool bPaused)
{
	EMULE_TRY

	m_partFileList.AddTail(pNewPartFile);
	SortByPriority();

	if (bPaused)
	{
		pNewPartFile->PauseFile();
		pNewPartFile->SetStartTimeReset(true);
	}

	g_App.m_pDownloadList->AddFile(pNewPartFile);

	CCat		*pCat = CCat::GetCatByID(pNewPartFile->GetCatID());
	CString		strCategory = (pCat != NULL) ? pCat->GetTitle() : GetResString(IDS_CAT_UNCATEGORIZED);

	AddLogLine(LOG_FL_SBAR, IDS_NEWDOWNLOAD, pNewPartFile->GetFileName(), strCategory);

//	Sending message when a download is added
	CString	strMessageText;

	strMessageText.Format(GetResString(IDS_NEWDOWNLOAD), pNewPartFile->GetFileName(), strCategory);
	g_App.m_pMDlg->SendMail( strMessageText, g_App.m_pPrefs->GetUseDownloadAddNotifier(),
	                                        g_App.m_pPrefs->IsSMTPInfoEnabled() );
	g_App.m_pMDlg->ShowNotifier( strMessageText, TBN_DLOAD_ADD, false,
	                                      g_App.m_pPrefs->GetUseDownloadAddNotifier() );
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDownloadQueue::FileExists(const uchar *strFileHash)
{
	bool		bFileExists = false;
	CKnownFile	*pKnownFile;

	EMULE_TRY

	if ((pKnownFile = m_pSharedFileList->GetFileByID(strFileHash)) != NULL)
	{
		if (pKnownFile->IsPartFile())
			AddLogLine(LOG_FL_SBAR, IDS_ERR_ALREADY_DOWNLOADING, pKnownFile->GetFileName());
		else
			AddLogLine(LOG_FL_SBAR, IDS_ERR_ALREADY_DOWNLOADED, pKnownFile->GetFileName());
		bFileExists = true;
	}
	else if ((pKnownFile = GetFileByID(strFileHash)) != NULL)
	{
		AddLogLine(LOG_FL_SBAR, IDS_ERR_ALREADY_DOWNLOADING, pKnownFile->GetFileName());
		bFileExists = true;
	}

	EMULE_CATCH

	return bFileExists;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::UpdateSourceStatesAfterServerChange()
{
	EMULE_TRY

	ClientList sourceListCopy;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL; )
	{
		CPartFile	*pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile != NULL)
		{
			EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

			if ((eFileStatus == PS_READY) || (eFileStatus == PS_EMPTY) || (eFileStatus == PS_PAUSED))
			{
				pPartFile->GetCopySourceLists(SLM_CHECK_SERVER_CHANGE, &sourceListCopy);
				for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
				{
					(*cIt)->UpdateDownloadStateAfterFileReask();
				}
			}
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::Process()
{
	EMULE_TRY

	if (!m_bIsInitialized)
		return;

	uint32	dwDownSpeed = 0;
	uint32	dwMaxDLSpeed = g_App.m_pPrefs->GetMaxDownload();

	if ((dwMaxDLSpeed != UNLIMITED) && ((m_dwDataRate > 1500) || (dwMaxDLSpeed < 30)))
	{
		uint32	dwMaxDLSpeedBPS = dwMaxDLSpeed * 1024;	// Bytes per second

	//	Don't remove upper DL limit, otherwise one DL will block all others
	//	Calculate what percentage of the max data rate the current data rate is
		dwDownSpeed = (dwMaxDLSpeedBPS * 10) / (m_dwDataRate + 1);
	//	Pin it between 50% and 200% of max
		if (dwDownSpeed < 50)
			dwDownSpeed = 50;
		else if (dwDownSpeed > 200)
			dwDownSpeed = 200;
	}

//	Process all the part files in the queue in decreasing order of priority
//	m_partFileList is already sorted by prio every time a file is added or deleted
//	therefore we removed all the extra loops...
	uint32	dwDataRate = 0;

	m_uiUDCounter++;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		CPartFile	*pPartFile = m_partFileList.GetAt(pos);

		if (pPartFile != NULL)
		{
			EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

			if ((eFileStatus == PS_READY) || (eFileStatus == PS_EMPTY))
				dwDataRate += pPartFile->Process(dwDownSpeed, m_uiUDCounter /*iteration*/);
		}
	//	Due to a fact that list is sorted by priority, position can be changed
	//	while doing file processing, thus do GetNext at the end to avoid exceptions
	//	This though doesn't resolve the problem that not all files might be processed by the loop
		m_partFileList.GetNext(pos);
	}
	m_dwDataRate = dwDataRate;

	DWORD	dwCurTick = ::GetTickCount();

//	Save used bandwidth for speed calculations
	if (m_averageTickList.empty() || dwCurTick - m_averageTickList.front() >= 500)	// Update no faster than every .5 sec
	{
		m_averageDataRateList.push_front(g_App.stat_sessionReceivedBytes);
	//	Save time between each speed snapshot
		m_averageTickList.push_front(dwCurTick);

	//	Save 40 secs of data. It seems to give the most accurate result atm...
		while (dwCurTick - m_averageTickList.back() > 40000)
		{
			m_averageDataRateList.pop_back();
			m_averageTickList.pop_back();
		}
	}

//	Calculate average data rate
	if (m_averageDataRateList.size() > 1)
		m_dwAverDataRate = static_cast<uint32>((static_cast<double>(m_averageDataRateList.front() - m_averageDataRateList.back())) * 1000.0 / (m_averageTickList.front() - m_averageTickList.back()));
	else
		m_dwAverDataRate = 0;

//	The code of sources refresh on local server was moved here in order to prevent
//	the situation where the client is blacklisted. We create:
//	1) one 'packet' which contains 15 buffered OP_GETSOURCES ED2K packets to be sent in one TCP frame every 300 sec
//		or
//	2) one OP_GETSOURCES contained hash & size of a bunch of files (up to 15)
#ifdef OLD_SOCKETS_ENABLED
	if (m_uiUDCounter == 2 && g_App.m_pServerConnect->IsConnected() && !m_LocalServerSourcesReqQueue.IsEmpty())
	{
		if (!m_dwLastTCPSourcesRequestTime || (dwCurTick - m_dwLastTCPSourcesRequestTime) > SERVER_SRC_REQ_QUANT)
		{
			int			iRequestedNumber = 0;
			CMemFile	packetStream((16 + 4 + 8) * 15);
		//	Server accepts OP_GETSOURCES containing several files, plus <HASH 16><SIZE 4>
			uint64		qwTmp;
			uint32		dwSrvTCPFlags = g_App.m_pServerConnect->GetCurrentServer()->GetTCPFlags();
			bool		bSupportNewFormat = (dwSrvTCPFlags & SRV_TCPFLG_EXT_GETSOURCES) != 0;
			bool		bSrvSupportsLargeFiles = (dwSrvTCPFlags & SRV_TCPFLG_LARGEFILES) != 0;

			while (iRequestedNumber < 15 && !m_LocalServerSourcesReqQueue.IsEmpty())
			{
				CPartFile	   *pPartFile = m_LocalServerSourcesReqQueue.RemoveHead();
				EnumPartFileStatuses	eFileStatus;

				if ( (pPartFile != NULL) &&
					(((eFileStatus = pPartFile->GetStatus()) == PS_READY) || (eFileStatus == PS_EMPTY)) &&
					(bSrvSupportsLargeFiles || !pPartFile->IsLargeFile()) )
				{
					if (bSupportNewFormat)
					{
						packetStream.Write(pPartFile->GetFileHash(), 16);
						qwTmp = pPartFile->GetFileSize();
						if (pPartFile->IsLargeFile())
						{
							packetStream.Write(&g_dwZeroConst, 4);	// indicates that this is a large file and a uint64 follows
							packetStream.Write(&qwTmp, sizeof(qwTmp));
						}
						else
							packetStream.Write(&qwTmp, 4);
					}
					else
					{
						Packet	packet(OP_GETSOURCES, 16);

						md4cpy(packet.m_pcBuffer, pPartFile->GetFileHash());
						packetStream.Write(packet.GetPacket(), packet.GetRealPacketSize());
					}
					iRequestedNumber++;
				}
			}

			const int	iStreamSize = static_cast<int>(packetStream.GetLength());

			if (iStreamSize > 0)
			{
				if (bSupportNewFormat)
				{
					Packet	packet(&packetStream);

#ifdef _CRYPT_READY
					packet.m_eOpcode = ( g_App.m_pPrefs->IsClientCryptLayerSupported() &&
						(dwSrvTCPFlags & SRV_TCPFLG_TCPOBFUSCATION) != 0) ) ? OP_GETSOURCES_OBFU : OP_GETSOURCES;
#else
					packet.m_eOpcode = OP_GETSOURCES;
#endif
					g_App.m_pUploadQueue->AddUpDataOverheadServer(packet.m_dwSize);
					g_App.m_pServerConnect->SendPacket(&packet, false);
				}
				else
				{
				//	Create one 'packet' which contains all buffered OP_GETSOURCES ED2K packets to be sent in one TCP frame
					Packet	*packet = new Packet(new char[iStreamSize], iStreamSize, true, 0, false);

					packetStream.SeekToBegin();
					packetStream.Read(packet->GetPacket(), iStreamSize);
					g_App.m_pUploadQueue->AddUpDataOverheadServer(packet->m_dwSize);
					g_App.m_pServerConnect->SendPacket(packet, true);
				}
			//	Update time only in case we sent something
				m_dwLastTCPSourcesRequestTime = dwCurTick;
			}
		}
	}
#endif //OLD_SOCKETS_ENABLED

	if (m_uiUDCounter == 5)
	{
		if ((!m_dwLastUDPStatTime) || (dwCurTick - m_dwLastUDPStatTime) > UDPSERVERSTATTIME)
		{
			m_dwLastUDPStatTime = dwCurTick;
			g_App.m_pServerList->ServerStats();
		}
	}
	else if (m_uiUDCounter >= 10)
	{
		m_uiUDCounter = 0;
		if ((!m_dwLastUDPSearchTime) || (dwCurTick - m_dwLastUDPSearchTime) > UDPSERVERREASKTIME)
		{
			SendNextUDPPacket();
		}
//--- xrmb:keepPartFileStats ---
		if (m_lastPartFileStatsSave + 300000 < dwCurTick)
		{
			m_lastPartFileStatsSave = dwCurTick;
			SaveAllPartFileStats();
		}
//--- :xrmb ---
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SaveAllPartFileStats()
{
	EMULE_TRY

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		CPartFile	*pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile)
		{
			pPartFile->SavePartFileStats();
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetFileByIndex() returns the 'iIndex'th (0-based) file in the queue or NULL if 'iIndex' is out of range.
CPartFile *CDownloadQueue::GetFileByIndex(int iIndex)
{
	POSITION	pos = m_partFileList.FindIndex(iIndex);

	return (pos != NULL) ? m_partFileList.GetAt(pos) : NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetFileByID() returns the part file from the download queue with file hash 'strFileHash'
//	or NULL if there is none.
CPartFile *CDownloadQueue::GetFileByID(const uchar* strFileHash)
{
	CPartFile	   *pPartFile = NULL;

	EMULE_TRY

	CPartFile	   *pTempPartFile;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		pTempPartFile = m_partFileList.GetNext(pos);

		if (pTempPartFile && !md4cmp(strFileHash, pTempPartFile->GetFileHash()))
		{
			pPartFile = pTempPartFile;
			break;
		}
	}

	EMULE_CATCH

	return pPartFile;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDownloadQueue::IsInDLQueue(CKnownFile *pFileToTest)
{
	bool	bIsInDLQueue = false;

	EMULE_TRY

	POSITION	pos = m_partFileList.Find(pFileToTest);

	if (pos != NULL)
		bIsInDLQueue = true;

	EMULE_CATCH

	return bIsInDLQueue;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient* CDownloadQueue::CheckAndAddSource(CPartFile *pSenderFile, uint32 dwUserIDHyb, uint16 uUserPort, uint32 dwSrvIP, uint16 uSrvPort, uchar *pbyteUserHash)
{
	EMULE_TRY

	if (!IsGoodHybridID(dwUserIDHyb))
		return NULL;

	uint32	dwIP = ntohl(dwUserIDHyb);

//	Check if source is our own client (now checking also TCP port)
	if (dwIP == g_App.m_pPrefs->GetLancastIP() && uUserPort == g_App.m_pPrefs->GetPort())
		return NULL;

//	If this part file is stopped or complete/completing or erroneous, no sources needed anymore
	EnumPartFileStatuses	eFileStatus = pSenderFile->GetStatus();

	if (eFileStatus == PS_STOPPED || eFileStatus == PS_COMPLETE
		|| eFileStatus == PS_COMPLETING || eFileStatus == PS_ERROR)
	{
		return NULL;
	}

//	Check if client has to be filtered, to prevent adding it to DL-queue by source exchange
//	1) The LowID source cann't be filtered, so we need to be sure that client has the server IP & port
//	2) Filter HighID source over an IP that extracted from Hybrid User ID
	if (IsLowID(dwUserIDHyb))
	{
	//	TODO: add handling of firewalled Kad users who send an ID "0x1" & don't have a proper server information 
		if ((dwSrvIP == 0) || (uSrvPort == 0))
			return NULL;
	}
	else if (g_App.m_pIPFilter->IsFiltered(dwIP))
	{
		InterlockedIncrement(&g_App.m_lSXFiltered);
		InterlockedIncrement(&g_App.m_lTotalFiltered);
		if (!g_App.m_pPrefs->IsCMNotLog())
			AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Filtered (reject as new source): %u.%u.%u.%u %hs"),
							 (byte)dwIP, (byte)(dwIP >> 8), (byte)(dwIP >> 16),
							 (byte)(dwIP >> 24), g_App.m_pIPFilter->GetLastHit() );
		return NULL;
	}

// Now we need to check if client is already in the DL-queue with all available information
	CUpDownClient	*pTempSource = g_App.m_pClientList->FindClient(dwUserIDHyb, uUserPort, dwSrvIP, uSrvPort, pbyteUserHash);

//	Client is already known
	if (pTempSource != NULL)
	{
	//	Check UL only case (client exist only in UL-queue)
		if (pTempSource->m_pReqPartFile == NULL)
		{
		// 	Add if OtherRequestList is empty (source was removed)
			if (pTempSource->m_otherRequestsList.IsEmpty())
			{
				pTempSource->SetDLRequiredFile(pSenderFile);
				pTempSource->SetDownloadState(DS_WAIT_FOR_FILE_REQUEST);
				pTempSource->ResetLastAskedTime();
				pSenderFile->RemovePastComment(pTempSource);
				g_App.m_pDownloadList->AddSource(pSenderFile, pTempSource, false);
			}
		}
		else if (pTempSource->m_pReqPartFile != pSenderFile)
		{
			if (pTempSource->AddRequestForAnotherFile(pSenderFile))
				g_App.m_pDownloadList->AddSource(pSenderFile, pTempSource, true);
		}

		return NULL;
	}
//	Client doesn't exist
	else
	{
		CUpDownClient	*pNewSource = new CUpDownClient(uUserPort, dwUserIDHyb, dwSrvIP, uSrvPort, pSenderFile, UID_HYBRID);

		if (g_App.m_pClientList->AddClient(pNewSource, true))
		{
			pNewSource->SetDLRequiredFile(pSenderFile);
			pNewSource->SetDownloadState(DS_WAIT_FOR_FILE_REQUEST);
			pSenderFile->RemovePastComment(pNewSource);
			g_App.m_pDownloadList->AddSource(pSenderFile, pNewSource, false);
			return pNewSource;
		}
		else
			safe_delete(pNewSource);
	}

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CDownloadQueue::CheckAndAddKnownSource(CPartFile *pPartFile, CUpDownClient *pKnownSource)
{
	EMULE_TRY

	EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

//	Stop adding sources to stopped, completing, completed and erroneous files
	if (eFileStatus == PS_STOPPED || eFileStatus == PS_COMPLETE
		|| eFileStatus == PS_COMPLETING || eFileStatus == PS_ERROR)
	{
		return false;
	}

	if (!IsGoodHybridID(pKnownSource->GetUserIDHybrid()))
		return false;

//	If we receive ourselves (now checking IP and UDP port) as a source we drop it
	if ( md4cmp(g_App.m_pPrefs->GetUserHash(), pKnownSource->GetUserHash()) == 0 &&
		 pKnownSource->GetIP() == g_App.m_pPrefs->GetLancastIP() &&
		 pKnownSource->GetUDPPort() == g_App.m_pPrefs->GetUDPPort() )
		return false;

//	Check if file is in download list
	if (IsInDLQueue(pPartFile))
	{
	//	If requested file is already specified
		if (pKnownSource->m_pReqPartFile)
		{
		//	Check if it's the same file
			if (pPartFile == pKnownSource->m_pReqPartFile)
			{
			//	If file was found, to speed up checks we will use variable pKnownSourceslot
				if (pPartFile->IsClientInSourceList(pKnownSource))
					return true;
			}
			else
			{
				if (pKnownSource->AddRequestForAnotherFile(pPartFile))
					g_App.m_pDownloadList->AddSource(pPartFile,pKnownSource,false);
				return true;
			}
		}
	}

	pKnownSource->SetDLRequiredFile(pPartFile);
	pKnownSource->SetDownloadState(DS_WAIT_FOR_FILE_REQUEST);
	pPartFile->RemovePastComment(pKnownSource);
	//pKnownSource->StartDLQueueWaitTimer();
	g_App.m_pDownloadList->AddSource(pPartFile,pKnownSource,false);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function removes the source from all files
void CDownloadQueue::RemoveSource(CUpDownClient *toremove, bool updatewindow /*true*/)
{
	EMULE_TRY

	CPartFile	*pPartFile;

	toremove->SetDownloadState(DS_NONE);

//	Now a remove client in A4AF files. Let's use "OtherRequests_list" for this matter
	for (POSITION pos = toremove->m_otherRequestsList.GetHeadPosition(); pos != NULL; )
	{
		pPartFile = toremove->m_otherRequestsList.GetNext(pos);
		if (IsInDLQueue(pPartFile))
		{
			pPartFile->RemoveClientFromA4AFSourceList(toremove);
		}
	}
//	Clear client from all graphical lists
	if (updatewindow)
		g_App.m_pDownloadList->RemoveSource(toremove, NULL);

	if (toremove->m_pReqPartFile != NULL)
		toremove->m_pReqPartFile->CheckAndAddPastComment(toremove);

	toremove->SetDLRequiredFile(NULL);
	toremove->m_otherNoNeededMap.RemoveAll();
	toremove->m_otherRequestsList.RemoveAll();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::RemoveFile(CPartFile* toremove)
{
	EMULE_TRY

	POSITION remove_pos = m_partFileList.Find(toremove);
	if (remove_pos != NULL)
		m_partFileList.RemoveAt(remove_pos);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::ResumeFiles()
{
	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL; )
	{
		CPartFile* pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile)
		{
			EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

			if ((eFileStatus == PS_READY) || (eFileStatus == PS_EMPTY))
			{
				pPartFile->ResumeFile();
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::DeleteAll()
{
	EMULE_TRY

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL; )
	{
		CPartFile* pPartFile = m_partFileList.GetNext(pos);
		if (pPartFile != NULL)
		{
			pPartFile->ClearSourceLists();
			pPartFile->ClearA4AFSourceList();

		//	Barry - Should also remove all requested blocks
		//	Don't worry about deleting the blocks, that gets handled
		//	when CUpDownClient is deleted in CClientList::DeleteAll()
			pPartFile->RemoveAllRequestedBlocks();
		}
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Max. file IDs per UDP packet
// ----------------------------
// 576 - 30 bytes of header (28 for UDP, 2 for "E3 9A" edonkey proto) = 546 bytes
// 546 / 16 = 34
// 546 / 20 = 27
// Every large file adds extra 8 bytes
bool CDownloadQueue::SendNextUDPPacket()
{
#ifdef OLD_SOCKETS_ENABLED
	EMULE_TRY

	if ( !m_bIsInitialized || m_partFileList.IsEmpty() ||
		!g_App.m_pServerConnect->IsConnected() ||
		g_App.m_pPrefs->IsClientCryptLayerRequired() ) // sources received without userhash can't be used, so don't ask
	{
		return false;
	}

	CServer	*pCurServer = g_App.m_pServerConnect->GetCurrentServer();

//	Get server for request
	pCurUDPServer = g_App.m_pServerList->GetSuccServer(pCurUDPServer);
//	Check local server & try switch to another one
	if (pCurUDPServer == g_App.m_pServerList->GetServerByAddress(pCurServer->GetAddress(), pCurServer->GetPort()))
	{
		pCurUDPServer = g_App.m_pServerList->GetSuccServer(pCurUDPServer);
	}
//	If we got NULL pointer back, that means
//	1. list is empty
//	2. we are in the end of the list
//	in both cases we will stop any UDP request
	if (pCurUDPServer == NULL)
	{
		StopUDPRequests();
		return false;
	}

	uint32		dwSz, dwPos, dwSrvUDPFlags = pCurUDPServer->GetUDPFlags();
	bool		bGetSources2Packet = (dwSrvUDPFlags & SRV_UDPFLG_EXT_GETSOURCES2) != 0;
	bool		bSrvSupportsLargeFiles = (dwSrvUDPFlags & SRV_UDPFLG_LARGEFILES) != 0;
//	Check support of extended request
	uint32		dwMaxPacketSz = (dwSrvUDPFlags & SRV_UDPFLG_EXT_GETSOURCES) ? 546 : 16;
//	Allocate a little bit more to simplify the algorithm
	CMemFile	dataGlobGetSources(dwMaxPacketSz + 20/*GETSOURCES2*/ + 8/*LargeFile*/);
	POSITION	posLastSearchedFile = NULL;

//	Get position in the file list
	if (m_pLastUDPSearchedFile != NULL)
		posLastSearchedFile = m_partFileList.Find(m_pLastUDPSearchedFile);

//	Check if file was found. This can happens in 2 cases
//	1. initial case: m_pLastUDPSearchedFile is not yet definded
//	2. if "m_pLastUDPSearchedFile" was deleted & m_partFileList.Find() return a 0
	if (posLastSearchedFile == NULL)
		posLastSearchedFile = m_partFileList.GetHeadPosition();

//	At this point we have a start position to search in "posLastSearchedFile"
//	so we can create a packet
	uint64			qwTmp;
	CPartFile		*pNextPartFile;

	for (int i = 0; i < m_partFileList.GetCount(); i++)
	{
		if ((pNextPartFile = m_partFileList.GetNext(posLastSearchedFile)) != NULL)
		{
			EnumPartFileStatuses	eFileStatus = pNextPartFile->GetStatus();

			if ( ((eFileStatus == PS_READY) || (eFileStatus == PS_EMPTY)) &&
				(g_App.m_pPrefs->GetMaxSourcePerFileUDP() > pNextPartFile->GetSourceCount()) &&
				(bSrvSupportsLargeFiles || !pNextPartFile->IsLargeFile()) )
			{
				dwPos = static_cast<uint32>(dataGlobGetSources.GetLength());
				dataGlobGetSources.Write(pNextPartFile->GetFileHash(), 16);

				if (bGetSources2Packet)
				{
					qwTmp = pNextPartFile->GetFileSize();
					if (pNextPartFile->IsLargeFile())
					{
						dataGlobGetSources.Write(&g_dwZeroConst, 4);	// indicates that this is a large file and a uint64 follows
						dataGlobGetSources.Write(&qwTmp, sizeof(qwTmp));
					}
					else
						dataGlobGetSources.Write(&qwTmp, 4);
				}
			//	If packet is filled break the loop
				if ((dwSz = static_cast<uint32>(dataGlobGetSources.GetLength())) >= dwMaxPacketSz)
				{
					if (dwSz > dwMaxPacketSz)	//	Rollback the last file
						dataGlobGetSources.SetLength(dwPos);
					break;
				}
				m_pLastUDPSearchedFile = pNextPartFile;
			}
		//	Check for the end of the loop & start from the head if it's needed
			if (posLastSearchedFile == NULL)
				posLastSearchedFile = m_partFileList.GetHeadPosition();
		}
	}

	if (static_cast<uint32>(dataGlobGetSources.GetLength()) != 0)
	{
		Packet			packet(&dataGlobGetSources);

		packet.m_eOpcode = (bGetSources2Packet) ? OP_GLOBGETSOURCES2 : OP_GLOBGETSOURCES;
		g_App.m_pUploadQueue->AddUpDataOverheadServer(packet.m_dwSize);
		g_App.m_pServerConnect->SendUDPPacket(&packet, pCurUDPServer, false);
	}
	return true;

	EMULE_CATCH
#endif //OLD_SOCKETS_ENABLED
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::StopUDPRequests()
{
	EMULE_TRY

	pCurUDPServer = NULL;
	m_dwLastUDPSearchTime = ::GetTickCount();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SortByPriority()
{
	EMULE_TRY

	POSITION		pos1, pos2;
	CPartFile	   *pPartFile;
	int				i = 0;

	for (pos1 = m_partFileList.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		pPartFile = m_partFileList.GetNext(pos1);

		if (pPartFile)
		{
			if (pPartFile->GetPriority() == PR_HIGH)
			{
				m_partFileList.AddHead(pPartFile);
				m_partFileList.RemoveAt(pos2);
			}
			else if (pPartFile->GetPriority() == PR_LOW)
			{
				m_partFileList.AddTail(pPartFile);
				m_partFileList.RemoveAt(pos2);
			}

			if (++i == m_partFileList.GetCount())
				break;
		}
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient* CDownloadQueue::GetDownloadClientByIP_UDP(uint32 dwIP, uint16 uUDPPort)
{
	EMULE_TRY

	POSITION		pos1;
	CUpDownClient	*pSource;
	CPartFile		*pPartFile;
	ClientList		clientListCopy;

	for (pos1 = m_partFileList.GetHeadPosition(); pos1 != NULL;)
	{
		pPartFile = m_partFileList.GetNext(pos1);

		if (pPartFile != NULL)
		{
			pPartFile->GetCopySourceLists(SLM_ALL, &clientListCopy);
			for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
			{
				pSource = *cIt;

				if ((dwIP == pSource->GetIP()) && (uUDPPort == pSource->GetUDPPort()))
					return pSource;
			}
		}
	}
	EMULE_CATCH
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::GetDownloadStats(uint32 adwSrc[], uint64 aqwData[])
{
	EMULE_TRY

	memzero(adwSrc, sizeof(adwSrc[0]) * STATS_DLSRC_COUNT);
	memzero(aqwData, sizeof(aqwData[0]) * STATS_DLDATA_COUNT);

	CPartFile	   *pPartFile;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		if ((pPartFile = m_partFileList.GetNext(pos)) != NULL)
		{
			adwSrc[STATS_DLSRC_TOTAL] += pPartFile->GetSourceCount();
			adwSrc[STATS_DLSRC_TRANSFERRING] += pPartFile->GetTransferringSrcCount();
			adwSrc[STATS_DLSRC_ONQUEUE] += pPartFile->GetOnQueueSrcCount();
			adwSrc[STATS_DLSRC_QUEUEFULL] += pPartFile->GetQueueFullSrcCount();
			adwSrc[STATS_DLSRC_NNS] += pPartFile->GetNoNeededPartsSrcCount();
			adwSrc[STATS_DLSRC_CONNECTED] += pPartFile->GetConnectedSrcCount();
			adwSrc[STATS_DLSRC_CONNECTING] += pPartFile->GetConnectingSrcCount();
			adwSrc[STATS_DLSRC_CONNECTING_VIA_SRV] += pPartFile->GetConnectingViaServerSrcCount();
			adwSrc[STATS_DLSRC_WAIT4FILEREQ] += pPartFile->GetWaitForFileReqSrcCount();
			adwSrc[STATS_DLSRC_LOW2LOW] += pPartFile->GetLow2LowSrcCount();
			adwSrc[STATS_DLSRC_LOWID_ON_OTHER_SRV] += pPartFile->GetLowIDOnOtherServer();
			adwSrc[STATS_DLSRC_HIGH_QR] += pPartFile->GetHighQRSrcCount();

			uint64 qwNeededSpace = (pPartFile->GetFileSize() - pPartFile->GetRealFileSize());
			EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

			if ((eFileStatus == PS_READY) || (eFileStatus == PS_EMPTY))
			{
				aqwData[STATS_DLDAT_ACTFILESIZE] += pPartFile->GetFileSize();
				aqwData[STATS_DLDAT_SIZE2TRANSFER] += pPartFile->GetSizeToTransfer();
				aqwData[STATS_DLDAT_ACTFILEREQSPACE] += qwNeededSpace;
			}
			aqwData[STATS_DLDAT_FILESIZETOTAL] += pPartFile->GetFileSize();
			aqwData[STATS_DLDAT_FILEREQSPACETOTAL] += qwNeededSpace;
			aqwData[STATS_DLDAT_FILEREALSIZE] += pPartFile->GetRealFileSize();
		}
	}

	adwSrc[STATS_DLSRC_BANNED] = m_dwBannedCounter;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SetAutoSourcesPerFile()
{
	sint32 cur_tmc = 0;
	sint32 cur_stotal = 0;
	sint32 old_mspf = g_App.m_pPrefs->GetMaxSourcePerFile();
	sint32 new_mspf = old_mspf;
	sint32 min_spf = g_App.m_pPrefs->GetMinAutoSourcesPerFile();
	sint32 max_spf = g_App.m_pPrefs->GetMaxAutoSourcesPerFile();
	sint32 max_stotal = g_App.m_pPrefs->GetMaxAutoSourcesTotal();
	sint32 max_sesrc = g_App.m_pPrefs->GetMaxAutoExchangeSources();
//	Count too many connections for all files
	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != 0; )
	{
		CPartFile * cur_file = m_partFileList.GetNext(pos);
		cur_tmc += cur_file->GetWaitForFileReqSrcCount();
		cur_stotal += cur_file->GetSourceCount();
	}

//	Check lower and upper limit
	sint32 current_dls = g_App.m_pDownloadQueue->GetActiveFileCount();
	if (current_dls > 0)
	{
		sint32 useful_min = max_stotal / current_dls;
		if (min_spf > useful_min)
		{
			min_spf = useful_min;
			g_App.m_pPrefs->SetMinAutoSourcesPerFile(min_spf);
		}
	}
	if (max_spf > max_stotal)
	{
		max_spf = max_stotal;
		g_App.m_pPrefs->SetMaxAutoSourcesPerFile(max_spf);
	}

//	Adjust SPF according to situation
	if (cur_stotal > max_stotal)
	{
	//	Too many total sources, lower
		if (cur_stotal - max_stotal > 200)
		{
			new_mspf = old_mspf - 10;
		}
		else
		{
			new_mspf = old_mspf - 2;
		}
	}
	else
	{
	//	Too many connections, lower sources per file
		if ((cur_tmc > 10) && (old_mspf > min_spf))
		{
			new_mspf = old_mspf - (cur_tmc / 15);
		}

	//	Not too many connections, raise sources per file depending on situation
		else if ((cur_tmc < 11) && (old_mspf < 200))
		{
			new_mspf = old_mspf + 10;
		}
		else if ((cur_tmc < 21) && (old_mspf < 200))
		{
			new_mspf = old_mspf + 2;
		}
		else if ((cur_tmc < 6) && (old_mspf < 400))
		{
			new_mspf = old_mspf + 5;
		}
		else if ((cur_tmc < 11) && (old_mspf < 400))
		{
			new_mspf = old_mspf + 2;
		}
		else if ((cur_tmc < 6) && (old_mspf < 600))
		{
			new_mspf = old_mspf + 5;
		}
		else if ((cur_tmc < 2) && (old_mspf < 800))
		{
			new_mspf = old_mspf + 5;
		}
		else if ((cur_tmc == 0) && (old_mspf < max_spf))
		{
			new_mspf = old_mspf + 2;
		}
	}

//	Stay between min_spf and max_spf
	if (new_mspf < min_spf)
		new_mspf = min_spf;
	if (new_mspf > max_spf)
		new_mspf = max_spf;

//	Change values
	if (old_mspf != new_mspf)
	{
		g_App.m_pPrefs->SetMaxSourcePerFile(new_mspf);

		if (g_App.m_pPrefs->IsAutoSourcesLogEnabled())
		{
			AddLogLine( LOG_FL_DBG | LOG_RGB_NOTICE, _T("AutoSourcesPerFile: TMC=%u / Total=%u -> Changed from %u to %u (%+d)"),
							 cur_tmc, cur_stotal, old_mspf, new_mspf, new_mspf - old_mspf );
		}

	//	Change source excange limit according to sources per file
		sint32 new_sesrc = g_App.m_pPrefs->GetMaxSourcePerFileSoft();
		g_App.m_pPrefs->SetXSUpTo((new_sesrc < max_sesrc) ? new_sesrc : max_sesrc);
	}
	else if (g_App.m_pPrefs->IsAutoSourcesLogEnabled())
	{
		AddLogLine( LOG_FL_DBG | LOG_RGB_NOTICE, _T("AutoSourcesPerFile: TMC=%u / Total=%u -> Not changed from %u"),
						 cur_tmc, cur_stotal, old_mspf );
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDownloadQueue::GetTransferringFiles() const
{
	EMULE_TRY

	int				iCount = 0;
	CPartFile	   *pPartFile;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		pPartFile = m_partFileList.GetNext(pos);
		if ((pPartFile != NULL) && (pPartFile->GetTransferringSrcCount() > 0))
			iCount++;
	}

	return iCount;

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDownloadQueue::GetPausedFileCount() const
{
	EMULE_TRY

	int				iCount = 0;
	CPartFile	   *pPartFile;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile)
		{
			if (pPartFile->GetStatus() == PS_PAUSED)
				iCount++;
		}
	}

	return iCount;

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDownloadQueue::GetStoppedFileCount() const
{
	EMULE_TRY

	int				iCount = 0;
	CPartFile	   *pPartFile;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile != NULL && pPartFile->GetStatus() == PS_STOPPED)
		{
			iCount++;
		}
	}

	return iCount;

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CDownloadQueue::GetActiveFileCount() const
{
	EMULE_TRY

	int				iCount = 0;
	CPartFile	   *pPartFile;

	for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		pPartFile = m_partFileList.GetNext(pos);

		if (pPartFile)
		{
			EnumPartFileStatuses	eFileStatus = pPartFile->GetStatus();

			if ((eFileStatus == PS_READY) || (eFileStatus == PS_EMPTY))
			{
				iCount++;
			}
		}
	}

	return iCount;

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::AddClientHostnameToResolve(CTypedPtrList<CPtrList, CClientSource*>* pLink)
{
	if (pLink != NULL)
		hostnameResolveQueue.AddTail(pLink);

	if (hostnameResolveQueue.IsEmpty() || m_bIsResolving)
		return;

	ResolveNextSourceHostname();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::ResolveNextSourceHostname()
{
	m_bIsResolving = true;
	CClientSource* source;
	for (;;)
	{
	 //	No more hostnames to resolve
		if (hostnameResolveQueue.IsEmpty())
		{
		//	Free unneccessary window
			delete m_wndHSCallback;
			m_wndHSCallback = NULL;
			m_bIsResolving = false;
			return;
		}

		source = hostnameResolveQueue.GetHead();

		if (source->sourceType != ED2KLINK_SOURCE_HOSTNAME)
		{
			delete source;
			hostnameResolveQueue.RemoveHead();
		}
		else
			break;
	}

	memzero(hostentBuffer, sizeof(hostentBuffer));

	if (!m_wndHSCallback)
	{
		m_wndHSCallback = new CHostnameSourceWnd();
		VERIFY(m_wndHSCallback->CreateEx(0, AfxRegisterWndClass(0), _T("Emule DNS Source Socket Wnd"), WS_OVERLAPPED, 0, 0, 0, 0, NULL, NULL));
		m_wndHSCallback->m_pOwner = this;
	}

	AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Resolving source hostname: %s"), source->sourceHostname);
	HANDLE RetValue;
	USES_CONVERSION;
	RetValue = WSAAsyncGetHostByName( m_wndHSCallback->m_hWnd, TM_SOURCEHOSTNAMERESOLVED,
	                                  CT2CA(source->sourceHostname), hostentBuffer, MAXGETHOSTSTRUCT );

	if (RetValue == 0)
	{
		if (sourceHostnameResolveRetry > 3)
		{
			CClientSource * hostsource;
			hostsource = hostnameResolveQueue.RemoveHead();
			AddLogLine( LOG_FL_DBG | LOG_RGB_ERROR, _T(__FUNCTION__) _T(": Error calling WSAAsyncGetHostByName to resolve source '%s'"),
				hostsource->sourceHostname );
			delete hostsource;
			delete m_wndHSCallback;
			m_wndHSCallback = NULL;
			memzero(hostentBuffer, sizeof(hostentBuffer));
			m_bIsResolving = false;
		}
		else
		{
			sourceHostnameResolveRetry += 1;
			memzero(hostentBuffer, sizeof(hostentBuffer));
			AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Source resolve retry number %i"), sourceHostnameResolveRetry);
			ResolveNextSourceHostname();
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SourceHostnameResolved(WPARAM wp, LPARAM lp)
{
	NOPRM(wp);
	ASSERT (hostnameResolveQueue.GetCount() > 0);

	sourceHostnameResolveRetry = 0;

	CClientSource* hostsource;
	hostsource = hostnameResolveQueue.RemoveHead();

	if (WSAGETASYNCERROR(lp) != 0)
	{
		DEBUG_ONLY(AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T(__FUNCTION__) _T(": Error resolving source '%s'"), hostsource->sourceHostname));
		delete hostsource;
		delete m_wndHSCallback;
		m_wndHSCallback = NULL;
		memzero(hostentBuffer, sizeof(hostentBuffer));
		ResolveNextSourceHostname();
		return;
	}

	int iBufLen = WSAGETASYNCBUFLEN(lp);
	LPHOSTENT lphost = (LPHOSTENT)malloc(iBufLen);
	memcpy2(lphost, hostentBuffer, iBufLen);
	hostsource->dwSrcIDHybrid = ntohl(((LPIN_ADDR)lphost->h_addr_list[0])->s_addr);
	in_addr resolvedaddr = *(LPIN_ADDR)lphost->h_addr_list[0];
	free(lphost);

	AddLogLine( LOG_FL_DBG, _T(__FUNCTION__) _T(": Resolved source hostname: %s - %u.%u.%u.%u"),
					 hostsource->sourceHostname,
					 resolvedaddr.S_un.S_un_b.s_b1,
					 resolvedaddr.S_un.S_un_b.s_b2,
					 resolvedaddr.S_un.S_un_b.s_b3,
					 resolvedaddr.S_un.S_un_b.s_b4 );

	CPartFile* partfile = this->GetFileByID(hostsource->filehashkey);

	if (partfile != NULL)
		partfile->AddClientSource(hostsource);
	else
	{
	//	TODO: notify user the partfile exists no more?
		DEBUG_ONLY(AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T(__FUNCTION__) _T(": impossible to add source because the partfile doesn't exist anymore")));
	}
	delete hostsource;
	memzero(hostentBuffer, sizeof(hostentBuffer));
	ResolveNextSourceHostname();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::GetUDPSearchStatus(CString *pstrOut)
{
	if (!m_bIsInitialized
#ifdef OLD_SOCKETS_ENABLED
		|| m_partFileList.IsEmpty() || !g_App.m_pServerConnect->IsConnected()
#endif //OLD_SOCKETS_ENABLED
		)
		GetResString(pstrOut, IDS_FSTAT_WAITING);
	else if (pCurUDPServer)
		GetResString(pstrOut, IDS_UDPSEARCH_PROGRESS);
	else if (m_dwLastUDPSearchTime)
	{
		uint32 dwNextUDPSearchTimeInSec = (m_dwLastUDPSearchTime + UDPSERVERREASKTIME - ::GetTickCount())/1000;

	//	Avoid to show some crazy time when search moment came, but search wasn't initiated yet
		if (dwNextUDPSearchTimeInSec > UDPSERVERREASKTIME)
			dwNextUDPSearchTimeInSec = 0;
		pstrOut->Format(GetResString(IDS_UDPSEARCH_NEXT), CastSecondsToHM(dwNextUDPSearchTimeInSec));
	}
	else
		GetResString(pstrOut, IDS_UDPSEARCH_NOTSTART);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SetA4AFAutoFile(CPartFile *file)
{
	EMULE_TRY

	m_A4AF_auto_file = file;

	if (file)
		g_App.m_pPrefs->SetA4AFHash(file->GetFileHash());
	else
	{
	//	Set null hash
		uchar null_hash[16];

		md4clr(null_hash);
		g_App.m_pPrefs->SetA4AFHash(null_hash);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::ResetCatParts(int iCatIndex)
{
//	We need a copy of the list because of the original list update done in SetCatID
	CTypedPtrList<CPtrList, CPartFile*> locallist;

	POSITION		pos;
	CPartFile	   *pPartFile;

	for (pos = m_partFileList.GetHeadPosition(); pos != NULL;)
	{
		pPartFile = m_partFileList.GetNext(pos);
		if (pPartFile != NULL)
			locallist.AddTail(pPartFile);
	}

	for (pos = locallist.GetHeadPosition(); pos != NULL;)
	{
		pPartFile = locallist.GetNext(pos);

		if (CCat::GetCatIndexByID(pPartFile->GetCatID()) == iCatIndex)
			pPartFile->SetCatID(CAT_NONE);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SetCatPrio(int iCatIndex, byte newprio)
{
//	We need a copy of the list because of the original list update done in SetCatID
	CTypedPtrList<CPtrList, CPartFile*>	locallist;

	POSITION		pos;
	CPartFile	   *pPartFile;

	for (pos = m_partFileList.GetHeadPosition(); pos != NULL; )
	{
		pPartFile = m_partFileList.GetNext(pos);
		if (pPartFile)
		{
			locallist.AddTail(pPartFile);
		}
	}

	for (pos = locallist.GetHeadPosition(); pos != NULL; )
	{
		pPartFile = locallist.GetNext(pos);

		if (iCatIndex == 0 || CCat::GetCatIndexByID(pPartFile->GetCatID()) == iCatIndex)
		{
			if (newprio == PR_AUTO)
			{
				pPartFile->SetAutoPriority(true);
				pPartFile->UpdateDownloadAutoPriority();
			}
			else
			{
				pPartFile->SetAutoPriority(false);
				pPartFile->SetPriority(newprio);
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SetCatStatus(int iCatIndex, uint16 newstatus)
{
	bool reset = false;
	POSITION pos = m_partFileList.GetHeadPosition();

	while (pos != NULL)
	{
		CPartFile * cur_file = m_partFileList.GetNext(pos);

		if (!cur_file)
			continue;

		if ( iCatIndex == -1 || (iCatIndex == -2 && cur_file->GetCatID() == 0)
		     || (iCatIndex == 0 && CCat::FileBelongsToGivenCat(cur_file, CAT_ALL))
		     || (iCatIndex >= CCat::GetNumPredefinedCats() && CCat::GetCatIDByIndex(iCatIndex) == cur_file->GetCatID()) )
		{
			switch (newstatus)
			{
				case MP_CANCEL:
					cur_file->DeleteFile();
					reset = true;
					break;
				case MP_PAUSE:
					cur_file->PauseFile();
					break;
				case MP_STOP:
					cur_file->StopFile();
					break;
				case MP_RESUME:
					if(cur_file->GetStatus() == PS_PAUSED || cur_file->GetStatus() == PS_STOPPED)
						cur_file->ResumeFile();
					break;
			}
		}
		if (reset)
		{
			reset = false;
			pos = m_partFileList.GetHeadPosition();
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CDownloadQueue::SetAutoCat(CPartFile *pPartFile)
{
	if (CCat::GetNumCats() > 1)
	{
		for (int ix = CCat::GetNumPredefinedCats(); ix < CCat::GetNumCats(); ix++)
		{
			int		iCurPos = 0;
			CString	strCatExt = CCat::GetCatByIndex(ix)->GetAutoCatExt();

		//	No need to compare again an empty AutoCat array
			if (strCatExt.IsEmpty())
				continue;

			strCatExt.MakeLower();

			CString	strFullName = pPartFile->GetFileName();

			strFullName.MakeLower();

			CString	strCmpExt = strCatExt.Tokenize(_T("|"), iCurPos);

			while (!strCmpExt.IsEmpty())
			{
			//	Allow wildcards in autocat string
				if (strCmpExt.FindOneOf(_T("*?")) != -1)
				{
				//	Use wildcards
					if (PathMatchSpec(static_cast<LPCTSTR>(strFullName.GetString()), static_cast<LPCTSTR>(strCmpExt.GetString())))
					{
						if (pPartFile->GetCatID() == CAT_NONE)
							pPartFile->SetCatID(CCat::GetCatIDByIndex(ix));
						return;
					}
				}
				else
				{
					if (strFullName.Find(strCmpExt) != -1)
					{
						if (pPartFile->GetCatID() == CAT_NONE)
							pPartFile->SetCatID(CCat::GetCatIDByIndex(ix));
						return;
					}
				}
				strCmpExt = strCatExt.Tokenize(_T("|"), iCurPos);
			}
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// SaveAllSLSFiles() - Updates all .txtsrc files with current sources
void CDownloadQueue::SaveAllSLSFiles()
{
	EMULE_TRY

	if (g_App.m_pPrefs->SLSEnable())
	{
		for (POSITION pos = m_partFileList.GetHeadPosition(); pos != NULL;)
		{
			CPartFile	*pPartFile = m_partFileList.GetNext(pos);

		//	Don't save if nothing to save...
			if ((pPartFile != NULL) && (pPartFile->GetSourceCount() != 0))
				g_App.m_pDownloadQueue->m_sourcesaver.Process(pPartFile, g_App.m_pPrefs->SLSMaxSourcesPerFile(), true);
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
