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
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "server.h"
#include "packets.h"
#include "otherfunctions.h"

#ifdef NEW_SOCKETS_ENGINE
	#include "Engine/Files/TaskProcessorFiles.h"
	#include "FileHashControl.h"
#endif //NEW_SOCKETS_ENGINE

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef NEW_SOCKETS_ENGINE
CSharedFileList::CSharedFileList(CPreferences *pPrefs, CServerConnect *pServerConnect, CKnownFileList *pKnownFileList)
{
	m_pPrefs = pPrefs;
	m_pServerConnect = pServerConnect;
	m_pKnownFileList = pKnownFileList;
	m_pOutput = NULL;
	m_dwLastPublishED2KTime = 0;
	m_bLastPublishED2KFlag = true;
	m_pCurrentlyHashing = NULL;

	CSingleLock		sLock(&m_mutexList,true);

	m_mapSharedFiles.InitHashTable(1021);

	sLock.Unlock();

	FindSharedFiles();
}
#else
CSharedFileList::CSharedFileList(CPreferences *pPrefs, CKnownFileList *pKnownFileList)
{
	m_pPrefs = pPrefs;
	m_pKnownFileList = pKnownFileList;
	m_dwLastPublishED2KTime = 0;
	m_bLastPublishED2KFlag = true;
	m_pCurrentlyHashing = NULL;

	CSingleLock		sLock(&m_mutexList,true);

	m_mapSharedFiles.InitHashTable(1021);

	sLock.Unlock();

	FindSharedFiles();
}
#endif //NEW_SOCKETS_ENGINE
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSharedFileList::~CSharedFileList()
{
	m_pCurrentlyHashing = NULL;
	while (!m_waitingForHashList.IsEmpty())
		delete m_waitingForHashList.RemoveTail();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	NextLANBroadcast() broadcasts the next hash for LANCast
void CSharedFileList::NextLANBroadcast()
{
#ifndef NEW_SOCKETS_ENGINE
	if (g_App.m_app_state != g_App.APP_STATE_RUNNING)
		return;

	CSingleLock		sLock(&m_mutexList,true);

	if (!m_mapSharedFiles.IsEmpty())
	{
		CSharedFilesMap::CPair	*pCurVal = NULL;

		if (m_LancastKey.m_key != NULL)
			pCurVal = m_mapSharedFiles.PGetNextAssoc(m_mapSharedFiles.PLookup(m_LancastKey));

		if (pCurVal == NULL)
			pCurVal = m_mapSharedFiles.PGetFirstAssoc();

		if (pCurVal->value != NULL)
			m_LanCast.BroadcastHash(pCurVal->value);

		m_LancastKey = pCurVal->key;
	}
//	m_mutexList is unlocked here
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FindSharedFiles() adds all the files in the shared directories list to the known file list and the
//		shared files map
void CSharedFileList::FindSharedFiles()
{
	CSingleLock		sLock1(&m_mutexList,true); // list thread safe

	if (!m_mapSharedFiles.IsEmpty())
	{
		CCKey			bufKey;
		CKnownFile	   *pKnownFile = NULL;

		POSITION		pos = m_mapSharedFiles.GetStartPosition();

		while (pos!= NULL)
		{
			m_mapSharedFiles.GetNextAssoc(pos, bufKey, pKnownFile);
			pKnownFile->SetSharedFile(false);
		}
		m_mapSharedFiles.RemoveAll();

		sLock1.Unlock();

#ifndef NEW_SOCKETS_ENGINE
		g_App.m_pDownloadQueue->AddPartFilesToShare();
#endif //NEW_SOCKETS_ENGINE
	}
	else
		sLock1.Unlock();

//	If the Incoming directory isn't shared yet...
	if (m_pPrefs->SharedDirListCheckAndAdd(m_pPrefs->GetIncomingDir(), false))
	{
		AddFilesFromDirectory(m_pPrefs->GetIncomingDir());
	}

	CStringList	tmpSharedDirList;	//	list elements will be deleted in list destructor

//	Make local copy to prevent long locking of list resource
	m_pPrefs->SharedDirListCopy(&tmpSharedDirList);
	for (POSITION pos = tmpSharedDirList.GetHeadPosition(); pos != NULL;)
	{
		AddFilesFromDirectory(tmpSharedDirList.GetNext(pos));
	}

	int		iSharedCnt = m_mapSharedFiles.GetCount();
	int		iWaitCnt = m_waitingForHashList.GetCount();

#ifndef NEW_SOCKETS_ENGINE
	if (iWaitCnt == 0)
		AddLogLine(0, IDS_SHAREDFOUND, iSharedCnt);
	else
		AddLogLine(0, IDS_SHAREDFOUNDHASHING, iSharedCnt, iWaitCnt);
#endif //NEW_SOCKETS_ENGINE

	BuildSharedVDirForList(&tmpSharedDirList);
	HashNextFile();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsHashing() checks to see if the spec'd file is already in the waiting for hash list
bool CSharedFileList::IsHashing(const CString &strDirectory, const CString &strFileName)
{
	bool		bFound = false;

	for (POSITION pos = m_waitingForHashList.GetHeadPosition(); pos != NULL;)
	{
		UnknownFile_Struct		*pUnknownFile = reinterpret_cast<UnknownFile_Struct*>(m_waitingForHashList.GetNext(pos));

		if ( (pUnknownFile->m_strFileName.CompareNoCase(strFileName) == 0) &&
			(pUnknownFile->m_strDirectory.CompareNoCase(strDirectory) == 0) )
		{
			bFound = true;
			break;
		}
	}

	return bFound;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::RemoveFromHashing(CKnownFile *pHashed)
{
	if (m_pCurrentlyHashing != NULL)
	{
		POSITION	pos = m_waitingForHashList.Find(m_pCurrentlyHashing);

		if (pos != NULL)
		{
			if ( (m_pCurrentlyHashing->m_strFileName.CompareNoCase(pHashed->GetFileName()) == 0) &&
				(m_pCurrentlyHashing->m_strDirectory.CompareNoCase(pHashed->GetPath()) == 0) )
			{
				m_waitingForHashList.RemoveAt(pos);
				delete m_pCurrentlyHashing;
				m_pCurrentlyHashing = NULL;
				HashNextFile();		// start next hash if possible, but only if a previous hash finished
			}
		}
		else
			m_pCurrentlyHashing = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::FileHashingFailed(UnknownFile_Struct *pHashed)
{
	AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_FAILEDTOHASH, pHashed->m_strDirectory, pHashed->m_strFileName);
	if (m_pCurrentlyHashing != NULL)
	{
		POSITION	pos = m_waitingForHashList.Find(m_pCurrentlyHashing);

		if (pos != NULL)
		{
			if ( (m_pCurrentlyHashing->m_strFileName.CompareNoCase(pHashed->m_strFileName) == 0) &&
				(m_pCurrentlyHashing->m_strDirectory.CompareNoCase(pHashed->m_strDirectory) == 0) )
			{
				m_waitingForHashList.RemoveAt(pos);
				delete m_pCurrentlyHashing;
				m_pCurrentlyHashing = NULL;
				HashNextFile();		// start next hash if possible, but only if a previous hash finished
			}
		}
		else
			m_pCurrentlyHashing = NULL;
	}
	delete pHashed;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AddFilesFromDirectory() adds all the files in the spec'd directory to the known files list and the
//		shared files map.
void CSharedFileList::AddFilesFromDirectory(const CString &strDirectory)
{
	static const TCHAR s_apcForbiddenExt[][9] =
	{
		_T("part"), _T("met"), _T("settings"), _T("bak"), _T("txtsrc"),
		_T("stats"), _T("dir"), _T("dll"), _T("sys"), _T("drv"), _T("tmp"),
		_T("log"), _T("getright"), _T("jc!"), _T("ds_store"), _T("$$$"),
		_T("ini"), _T("lnk"), _T("pif"), _T("vbs"), _T("vbe")
	};
	ULONGLONG	qwFileSz;
	CFileFind	ff;
	CString		strName;
	bool		bEnd;
	int			iIdxExt;

	strName.Format(_T("%s\\*"), strDirectory);
	bEnd = !ff.FindFile(strName, 0);
	if (bEnd)
		return;

	CString		strExtension;

	while (!bEnd)
	{
		bEnd = !ff.FindNextFile();
		if ( ff.IsDirectory() || ff.IsDots() || ff.IsSystem() || ff.IsTemporary() ||
			((qwFileSz = ff.GetLength()) == 0) || (qwFileSz > MAX_EMULE_FILE_SIZE) )
			continue;

		CTime		lastWriteTime;

		ff.GetLastWriteTime(lastWriteTime);

		uint32	dwFileDate = static_cast<uint32>(lastWriteTime.GetTime());

		AdjustNTFSDaylightFileTime(&dwFileDate, ff.GetFilePath());
		strName = ff.GetFileName();

		if ((iIdxExt = strName.ReverseFind(_T('.'))) >= 0)
		{
			unsigned	ui;

			strExtension = strName.Mid(iIdxExt + 1);
			strExtension.MakeLower();

			for (ui = 0; ui < ARRSIZE(s_apcForbiddenExt); ui++)
			{
				if (strExtension.Compare(reinterpret_cast<const TCHAR*>(&s_apcForbiddenExt[ui])) == 0)
					break;
			}
		//	A file is one of the type we don't want to share...
			if (ui != ARRSIZE(s_apcForbiddenExt))
				continue;
		}

		CKnownFile	   *pKnownFile = m_pKnownFileList->FindKnownFile(strName, dwFileDate, qwFileSz);

	//	If the file is already known...
		if (pKnownFile != NULL)
		{
			strName = ff.GetFilePath();
			if (AddFile(pKnownFile, strName))
			{
				pKnownFile->SetPath(strDirectory);
				pKnownFile->SetFilePath(strName);
				m_bLastPublishED2KFlag = true;
			}
		}
	//	If the file is not in the known list...
		else
		{
		//	Start a thread to hash the new file
		//	If the spec'd file isn't already in the list (avoid duplications due to overlapping share paths)
			if (!IsHashing(strDirectory, strName))
			{
				UnknownFile_Struct	*pUnknownFile = new UnknownFile_Struct;

				pUnknownFile->m_strDirectory = strDirectory;
				pUnknownFile->m_strFileName = strName;
				m_waitingForHashList.AddTail(pUnknownFile);
			}
		}
	}
	ff.Close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SafeAddKnownFile() shares a new file.
//		Params:
//			pNewKnownFile - file object to add;
//			bOnlyAdd      - add a file to shared files map but don't offer to the server;
//			bDelay        - delay publishing a little trying to reduce a number of packets.
void CSharedFileList::SafeAddKnownFile(CKnownFile *pNewKnownFile, bool bOnlyAdd/*=false*/, bool bDelay/*=false*/)
{
	EMULE_TRY

	if (pNewKnownFile == NULL)
		return;

	RemoveFromHashing(pNewKnownFile);	// hashed ok, remove from list if it was in the list

	bool	bAdded = AddFile(pNewKnownFile, pNewKnownFile->GetFilePath());

//	If we're just to add the file and not advertise it, return
	if (bOnlyAdd)
		return;
//	Update the SharedFiles window if it's open
#ifndef NEW_SOCKETS_ENGINE
	if (bAdded && (m_pOutput != NULL))
		m_pOutput->AddFile(pNewKnownFile);
#endif //NEW_SOCKETS_ENGINE
	if (bDelay)
	{
	//	Delay publishing a little to avoid double publishing of small files as well
	//	as to increase a chance of buffering of several publish requests in one packet
		m_dwLastPublishED2KTime = ::GetTickCount();
	}
	m_bLastPublishED2KFlag = true;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RepublishFile() republishes a file on the server to update server statistics
void CSharedFileList::RepublishFile(CKnownFile *pFile, int iMode)
{
	CServer	*pCurServer = m_pServerConnect->GetCurrentServer();

//	Republish only if a server supports a feature (e.g. complete sources, file rating, etc.)
	if ((pCurServer != NULL) && (pCurServer->GetTCPFlags() & iMode))
	{
		m_bLastPublishED2KFlag = true;
		pFile->SetPublishedED2K(false);
		UpdateItem(pFile, false);	// update item in the list
	//	This information is not critical, so delay publishing a little
	//	to increase a chance of buffering of several publish requests in one packet
		m_dwLastPublishED2KTime = ::GetTickCount();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSharedFileList::AddFile(CKnownFile *pKnownFile, const TCHAR *pcFullName)
{
	CCKey			bufKey(pKnownFile->GetFileHash());
	CKnownFile		*pResultFile;
	CSingleLock		sLock(&m_mutexList, true);

//	Check if the file is already known
	if (m_mapSharedFiles.Lookup(bufKey, pResultFile))
	{
		sLock.Unlock();
		AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Attempt to add duplicate shared files '%s' and '%s'"), pResultFile->GetFilePath(), pcFullName);
		return false;
	}

//	A file is being shared now
	pKnownFile->SetSharedFile(true);

//	Add a file to the shared files map
	m_mapSharedFiles.SetAt(CCKey(pKnownFile->GetFileHash()), pKnownFile);
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::FileHashingFinished(CKnownFile *pKnownFile)
{
//	File hashing has finished for a shared file (non partfile)
//	- reading shared directories at startup and hashing files which were not found in known.met
//	- reading shared directories during runtime (user hit Reload button, added a shared directory, ...)
	CKnownFile	*pFoundFile = GetFileByID(pKnownFile->GetFileHash());

	if (pFoundFile == NULL)
	{
		m_pKnownFileList->SafeAddKnownFile(pKnownFile);
		SafeAddKnownFile(pKnownFile);
		AddLogLine(LOG_RGB_DIMMED, IDS_HASBEENHASHED, pKnownFile->GetFileName(), GetWaitingForHashCount());
	}
	else
	{
		AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Duplicate shared files '%s' and '%s'"), pFoundFile->GetFilePath(), pKnownFile->GetFilePath());

		RemoveFromHashing(pKnownFile);
		delete pKnownFile;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	RemoveFile() removes the first occurrence of 'pKnownFile' in the shared file list
void CSharedFileList::RemoveFile(CKnownFile *pKnownFile)
{
	pKnownFile->SetSharedFile(false);
#ifndef NEW_SOCKETS_ENGINE
	m_pOutput->RemoveFile(pKnownFile);
#endif //NEW_SOCKETS_ENGINE

	CSingleLock		sLock(&m_mutexList,true);

	m_mapSharedFiles.RemoveKey(CCKey(pKnownFile->GetFileHash()));
	m_pKnownFileList->RemoveFile(pKnownFile);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::Reload()
{
	FindSharedFiles();
#ifndef NEW_SOCKETS_ENGINE
	if (m_pOutput != NULL)
		m_pOutput->ShowFileList(this);
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::SetOutputCtrl(CSharedFilesCtrl *pSharedFilesCtrl)
{
#ifndef NEW_SOCKETS_ENGINE
	m_pOutput = pSharedFilesCtrl;
	m_pOutput->ShowFileList(this);
#endif //NEW_SOCKETS_ENGINE
	HashNextFile();		// if hashing not yet started, start it now
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
__inline uint32 GetRealPrio(byte byteIn)
{
	return (byteIn < 4) ? (byteIn + 1) : 0;
}

#define PUBLISH_MAX_FILES		200u
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::SendListToServer()
{
	EMULE_TRY

	m_dwLastPublishED2KTime = ::GetTickCount();
#ifdef OLD_SOCKETS_ENABLED
	CServer			*pCurServer;

	if (!m_pServerConnect->IsConnected() || ((pCurServer = m_pServerConnect->GetCurrentServer()) == NULL))
		return;

	CSingleLock		sLock(&m_mutexList, true);

	if (m_mapSharedFiles.IsEmpty())
		return;

	CCKey			bufKey;
	CKnownFile		*pKnownFile, *pListKFile;
	uint32			dwNumFiles, dwListNum, dwPrioPos2, dwPrioFile;
	POSITION		pos, pos2, pos3;
	CTypedPtrList<CPtrList, CKnownFile*>	sortedList(100);
	uint32			adwPrioDistrib[2 * 5];	//	double amount of all file priorities PR_VERYLOW..PR_RELEASE
	uint32			dwCnt, ui;

	memset(adwPrioDistrib, 0, sizeof(adwPrioDistrib));
//	For each shared file...
	for (dwListNum = 0, pos = m_mapSharedFiles.GetStartPosition(); pos != NULL;)
	{
		m_mapSharedFiles.GetNextAssoc(pos, bufKey, pKnownFile);
	//	Take only files which should be (and can be) published
		if ( !pKnownFile->GetPublishedED2K() &&
			 (!pKnownFile->IsLargeFile() || pCurServer->SupportsLargeFilesTCP()) )
		{
			dwPrioFile = GetRealPrio(pKnownFile->GetULPriority()) + ((pKnownFile->IsPartFile()) ? (ARRSIZE(adwPrioDistrib) / 2) : 0);
		//	List sorting can take quite some time when it's long. We need only PUBLISH_MAX_FILES
		//	top files, so we keep priority distribution and drop file this time
		//	if it can't be in the first PUBLISH_MAX_FILES for sure
			if (++dwListNum > PUBLISH_MAX_FILES)
			{
				for (dwCnt = 0, ui = dwPrioFile; ui < ARRSIZE(adwPrioDistrib); ui++)
					dwCnt += adwPrioDistrib[ui];
				if (dwCnt >= PUBLISH_MAX_FILES)
					continue;
			}
			adwPrioDistrib[dwPrioFile]++;
		//	Perform an insertion sort into 'sortedList'
			for (pos2 = sortedList.GetHeadPosition(); ((pos3 = pos2) != NULL);)
			{
				pListKFile = sortedList.GetNext(pos2);
			//	Publish part files first to speed up downloading
				dwPrioPos2 = GetRealPrio(pListKFile->GetULPriority()) + ((pListKFile->IsPartFile()) ? (ARRSIZE(adwPrioDistrib) / 2) : 0);

			//	If the file we're inserting has a higher priority than the one from the sorted list
				if (dwPrioPos2 <= dwPrioFile)
				{
				//	Insert it before the current one in the sorted list
					sortedList.InsertBefore(pos3, pKnownFile);
					break;
				}
			}
		//	If we didn't find a file to add this one before, add it to the end of the list
			if (pos3 == NULL)
				sortedList.AddTail(pKnownFile);
		}
	}
	sLock.Unlock();

	dwNumFiles = pCurServer->GetSoftMaxFiles();

//	If a lot of file were added after the connection to the server, a warning won't be displayed
//	It be displayed on the next connection to the server, what is also fine
	if (((dwNumFiles != 0) && (dwNumFiles < dwListNum)) || ((dwNumFiles == 0) && (dwListNum > 1000)))
		AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_SHAREDWARNING, dwListNum);

	if ((dwNumFiles - 1u) > (PUBLISH_MAX_FILES - 1u))
		dwNumFiles = PUBLISH_MAX_FILES;

	if (dwListNum < dwNumFiles)
	{
		if ((dwNumFiles = dwListNum) == 0)
		{
			m_bLastPublishED2KFlag = false;		//	everything was published
			return;
		}
	}
//	Construct the byte stream for the OFFERFILES packet
	CMemFile		packetStream(4096);

	packetStream.Write(&dwNumFiles, sizeof(dwNumFiles));	// <count:DWORD> published file count

	AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("Published %u file(s) on the server, %u remaining"), dwNumFiles, dwListNum - dwNumFiles);
	pos = sortedList.GetHeadPosition();
	do {
		pKnownFile = sortedList.GetNext(pos);
		WriteToOfferedFilePacket(pKnownFile, packetStream, pCurServer);	// (offered file info)[count]
		pKnownFile->SetPublishedED2K(true);
	} while(--dwNumFiles != 0);
//	Although only some items were change, redraw the whole list as it's faster this way
	UpdateItem(NULL, false);

	Packet		*pPacket = new Packet(&packetStream);

	pPacket->m_eOpcode = OP_OFFERFILES;
//	Always try to compress the packet
//   - this kind of data is highly compressable (N * (1 MD4 and plus 2 string meta data tags and 1 integer meta data tag))
//   - the minimum amount of data needed for one published file is ~60 bytes
//   - this function is called once when connecting to a server and when a file becomes shareable - so, it's called rarely
//   - if the compressed size is still >= the original size, we send the uncompressed packet
	if (pCurServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION)
		pPacket->PackPacket();

	g_App.m_pUploadQueue->AddUpDataOverheadServer(pPacket->m_dwSize);
	m_pServerConnect->SendPacket(pPacket, true);
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ClearED2KPublishInfo() resets file publishing information to allow overall republish
void CSharedFileList::ClearED2KPublishInfo()
{
	CKnownFile	*pKnownFile;
	CCKey		bufKey;
	CSingleLock	sLock(&m_mutexList, true);

	m_bLastPublishED2KFlag = true;
	for (POSITION pos = m_mapSharedFiles.GetStartPosition(); pos != 0;)
	{
		m_mapSharedFiles.GetNextAssoc(pos, bufKey, pKnownFile);
		pKnownFile->SetPublishedED2K(false);
	}
	UpdateItem(NULL, false);	// redraw the whole list
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::WriteToOfferedFilePacket(CKnownFile *pKFile, CMemFile &packetStream,
											   CServer *pServer, CUpDownClient* pClient/*= NULL*/)
{
#ifdef OLD_SOCKETS_ENABLED
	packetStream.Write(pKFile->GetFileHash(), 16);		// <filehash 16>

	uint32		dwClientID = 0;
	uint16		uClientPort = 0;

	if ((pServer != NULL) && ((pServer->GetTCPFlags() & SRV_TCPFLG_COMPRESSION) != 0))
	{
		if (pKFile->IsPartFile())
		{
		//	Publishing an incomplete files
			dwClientID = 0xFCFCFCFC;
			uClientPort = 0xFCFC;
		}
		else
		{
		//	Publishing a complete files
			dwClientID = 0xFBFBFBFB;
			uClientPort = 0xFBFB;
		}
 	}
	else if (m_pServerConnect->IsConnected() && !m_pServerConnect->IsLowID())
	{	//	If connected and highID, set client ID and port
			dwClientID = m_pServerConnect->GetClientID();
			uClientPort = g_App.m_pPrefs->GetPort();
	}

	packetStream.Write(&dwClientID, 4);						// <clientid 4>
	packetStream.Write(&uClientPort, 2);					// <clientport 2>

	CWrTag	tagWr;
	uint32	dwNumType;
	CString	strFileType(pKFile->GetSearchFileType(&dwNumType));
	uint32	dwRating, dwTagFilePos, dwTagCnt = 0;
	ECodingFormat eCF = cfLocalCodePage;
	bool	bNewTags;

	dwTagFilePos = static_cast<uint32>(packetStream.GetPosition());
	packetStream.Write(&dwTagCnt, 4);						// <tagcount 4>

	bNewTags = ( ((pServer != NULL) && (pServer->GetTCPFlags() & SRV_TCPFLG_NEWTAGS)) ||
		( (pClient != NULL) && pClient->IsEmuleClient() &&
		( ((pClient->GetClientSoft() == SO_EMULE) && (pClient->GetVersion() >= FORM_CLIENT_VER(0, 42, 7))) ||
		((pClient->GetClientSoft() == SO_PLUS) && (pClient->GetVersion() > FORM_CLIENT_VER(1, 1, 0))) ) ) );

#ifdef _UNICODE
	if (((pServer != NULL) && (pServer->GetTCPFlags() & SRV_TCPFLG_UNICODE))
		|| ((pClient != NULL) && pClient->GetStrCodingFormat() == cfUTF8))
	{
		eCF = cfUTF8;
	}
#endif

//	There's no need to send FT_FILEFORMAT (file extension without ".") here as a server takes it from the file name

	if (bNewTags)
	{
		tagWr.WriteNewEd2kTag(FT_FILENAME, pKFile->GetFileName(), packetStream, eCF);
		dwTagCnt++;

	//	2*32bit tags are sent to servers, but a real 64bit tag to clients
		if ((pServer != NULL) || !pKFile->IsLargeFile())
		{
			tagWr.WriteNewEd2kTag(FT_FILESIZE, static_cast<uint32>(pKFile->GetFileSize()), packetStream);
			if (pKFile->IsLargeFile())
			{
				tagWr.WriteNewEd2kTag(FT_FILESIZE_HI, static_cast<uint32>(pKFile->GetFileSize() >> 32ui64), packetStream);
				dwTagCnt++;
			}
		}
		else
			tagWr.WriteNewEd2kTag(FT_FILESIZE, pKFile->GetFileSize(), packetStream);
		dwTagCnt++;

		if (!strFileType.IsEmpty())
		{
			if ((pServer != NULL) && (pServer->GetTCPFlags() & SRV_TCPFLG_TYPETAGINTEGER))
				tagWr.WriteNewEd2kTag(FT_FILETYPE, dwNumType, packetStream);
			else
				tagWr.WriteNewEd2kTag(FT_FILETYPE, strFileType, packetStream);
			dwTagCnt++;
		}
		dwRating = static_cast<uint32>(pKFile->GetFileRating());
	//	Send rating to client or server if server supports it (17.6+); also check for valid value
		if ( ( ((pServer == NULL) || ((pServer->GetTCPFlags() & SRV_TCPFLG_TYPETAGINTEGER) == 0)) &&
			(pClient == NULL) ) || (dwRating > PF_RATING_EXCELLENT) )
		{
			dwRating = 0;
		}
		if (dwRating != 0)	//	report file rating
		{
		//	Convert internal rating value into server rating value:
		//	0 - not rated; 1 - fake; 2 - poor; 3 - fair; 4 - good; 5 - excellent
			if ((dwRating == PF_RATING_GOOD) || (dwRating == PF_RATING_FAIR))
				dwRating = (~dwRating & 7);
			if (pClient != NULL)	//	Convert rating into format sent by servers
				dwRating *= (255 / 5/*RatingExcellent*/);
			tagWr.WriteNewEd2kTag(FT_FILERATING, dwRating, packetStream);
			dwTagCnt++;
		}
	}
	else
	{
		tagWr.WriteToFile(FT_FILENAME, pKFile->GetFileName(), packetStream, eCF);				// {FILENAME : string}
		dwTagCnt++;

	//	2*32bit tags are sent to servers, but a real 64bit tag to clients
		if ((pServer != NULL) || !pKFile->IsLargeFile())
		{
			tagWr.WriteToFile(FT_FILESIZE, static_cast<uint32>(pKFile->GetFileSize()), packetStream);
			if (pKFile->IsLargeFile())
			{
				tagWr.WriteToFile(FT_FILESIZE_HI, static_cast<uint32>(pKFile->GetFileSize() >> 32ui64), packetStream);
				dwTagCnt++;
			}
		}
		else if (pClient->SupportsLargeFiles())
			tagWr.WriteToFile(FT_FILESIZE, pKFile->GetFileSize(), packetStream, true);
		else
			tagWr.WriteToFile(FT_FILESIZE, 0, packetStream);
		dwTagCnt++;

		if (!strFileType.IsEmpty())
		{
			tagWr.WriteToFile(FT_FILETYPE, strFileType, packetStream);	// {FILETYPE : string}
			dwTagCnt++;
		}
	}
//	Save valid tag count
	packetStream.Seek(dwTagFilePos, CFile::begin);
	packetStream.Write(&dwTagCnt, 4);
	packetStream.SeekToEnd();
#endif //OLD_SOCKETS_ENABLED
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetDatasize() returns the size of the largest shared file in 'qwLargest' and returns
//		the total size of all shared files as its result.
uint64 CSharedFileList::GetDatasize(uint64 *pqwLargest)
{
	*pqwLargest = 0;

	uint64			qwFilesTotalBytes = 0;
	CCKey			bufKey;
	CKnownFile	   *pKnownFile;

	CSingleLock		sLock(&m_mutexList, true);

	for (POSITION pos = m_mapSharedFiles.GetStartPosition(); pos != NULL;)
	{
		m_mapSharedFiles.GetNextAssoc(pos,bufKey,pKnownFile);
		qwFilesTotalBytes += pKnownFile->GetFileSize();
	//	If this file is bigger than all the others...
		if (pKnownFile->GetFileSize() > *pqwLargest)
			*pqwLargest = pKnownFile->GetFileSize();
	}

	return qwFilesTotalBytes;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetFileByID() returns the Known File with hash 'pbyteFileHash' or NULL if there is none.
CKnownFile *CSharedFileList::GetFileByID(const uchar *pbyteFileHash)
{
	CKnownFile		*pResultFile;
	CCKey			tKey(pbyteFileHash);
	CSingleLock		sLock(&m_mutexList, true);

	if ((pbyteFileHash != NULL) && m_mapSharedFiles.Lookup(tKey, pResultFile))
		return pResultFile;
	else
		return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::HashNextFile()
{
//	Wait for the dialog to be opened
	if ((g_App.m_pMDlg == NULL) || !::IsWindow(g_App.m_pMDlg->m_hWnd))
		return;
	if (g_App.m_pMDlg->IsRunning() && (m_pOutput != NULL))
		m_pOutput->ShowFilesCount();
	if (m_pCurrentlyHashing != NULL)	// one hash at a time
		return;
	if (m_waitingForHashList.IsEmpty())	//	If there're no files to hash, just return
		return;

	m_pCurrentlyHashing = m_waitingForHashList.GetHead();

#ifndef NEW_SOCKETS_ENGINE
	g_App.m_fileHashControl.AddToHash(m_pCurrentlyHashing->m_strDirectory, m_pCurrentlyHashing->m_strFileName);
#else
	g_stEngine.Files.FileHasher.AddToHash(this, m_pCurrentlyHashing->m_strDirectory, m_pCurrentlyHashing->m_strFileName);
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::UpdateItem(CKnownFile *pKnownFile, bool bResort/*=true*/)
{
#ifndef NEW_SOCKETS_ENGINE
	if (m_pOutput != NULL)
	{
		m_pOutput->PostMessage( WM_SFL_UPDATEITEM,
			static_cast<WPARAM>(bResort), reinterpret_cast<LPARAM>(pKnownFile) );
	}
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define INCOMPLETE_DIR		_T("!Incomplete Files")
#define SHARED_DIR			_T("!Shared Files")

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSharedFileList::AddSharedVDirForList(bool bIncomplete, const CString &strNewDir)
{
	EMULE_TRY
	// clean dir
	CString strDir(strNewDir);
	PathRemoveBackslash(strDir.GetBuffer());
	if (strDir.IsEmpty())
		return false;

	// check already added
	POSITION	pos = m_SharedVDirForList.GetStartPosition();

	while (pos != NULL)
	{
		CString strCurKey;
		CString strCurDir;
		m_SharedVDirForList.GetNextAssoc(pos, strCurKey, strCurDir);
		if (strCurDir.CompareNoCase(strDir) == 0)
			return false;
	}

	// set key
	CString strKey, strWantedKey;
	if (bIncomplete)
	{
		strWantedKey = INCOMPLETE_DIR;	// default name used by Hybrid for temp dir
	}
	else
	{
		strWantedKey = PathFindFileName((LPCTSTR)strDir);	// short name without complete path
		if ((strWantedKey.IsEmpty()) || ((strWantedKey.GetLength() > 1) && (strWantedKey[1] == ':')))
			strWantedKey = SHARED_DIR;
	}

	CString strDir0, strDir1;
	if ((m_SharedVDirForList.Lookup(strWantedKey, strDir0)) || (m_SharedVDirForList.Lookup(strWantedKey + _T(" (1)"), strDir1)))
	{
		// rename key 0
		if (!strDir0.IsEmpty())
		{
			m_SharedVDirForList[strWantedKey + _T(" (1)")] = strDir0;
			m_SharedVDirForList.RemoveKey(strWantedKey);
		}

		// build unique key
		unsigned ui = 2;
		do
		{
			strKey.Format(_T("%s (%u)"), strWantedKey, ui++);
		} while (m_SharedVDirForList.Lookup(strKey, strDir0));
	}
	else
		strKey = strWantedKey;
	m_SharedVDirForList[strKey] = strDir;

	return true;

	EMULE_CATCH
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::BuildSharedVDirForList(CStringList *sharedDirList)
{
	if (!m_pPrefs)
		return;

	m_SharedVDirForList.RemoveAll();

// Temp dirs
	AddSharedVDirForList(true, m_pPrefs->GetTempDir());

	CStringList	tmpTempDirList;	//	list elements will be deleted in list destructor

//	Make local copy to prevent long locking of list resource
	m_pPrefs->TempDirListCopy(&tmpTempDirList);

	POSITION	pos = tmpTempDirList.GetHeadPosition();

	while (pos != NULL)
		AddSharedVDirForList(true, tmpTempDirList.GetNext(pos));

	// incoming dir
	AddSharedVDirForList(false, m_pPrefs->GetIncomingDir());

	// shared dirs
	pos = sharedDirList->GetHeadPosition();
	while (pos != NULL)
		AddSharedVDirForList(false, sharedDirList->GetNext(pos));

	// category dirs
	for (int i = 0; i < CCat::GetNumCats(); i++)
		AddSharedVDirForList(false, CCat::GetCatPathByIndex(i));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFileList::Process()
{
	if (!m_bLastPublishED2KFlag || ((::GetTickCount() - m_dwLastPublishED2KTime) < ED2KREPUBLISHTIME))
		return;
	SendListToServer();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(CHashFileThread, CWinThread)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CHashFileThread::CHashFileThread()
{
	m_pPartFile = NULL;
	m_bChangeState = false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SetValues() sets the thread parameters
void CHashFileThread::SetValues(bool bChgState, LPCTSTR pcDir, LPCTSTR pcFilename, CPartFile *pPartFile)
{
	 m_bChangeState = bChgState;
	 m_strDirectory = pcDir;
	 m_strFileName = pcFilename;
	 m_pPartFile = pPartFile;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CHashFileThread::InitInstance()
{
	g_App.m_pPrefs->InitThreadLocale();
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CHashFileThread::Run()
{
#ifdef EP_SPIDERWEB
//	Setup Structured Exception handler for the this thread
	_set_se_translator(StructuredExceptionHandler);
#endif

//	If there is no PartFile owner or if there's no filename...
	if ((m_pPartFile != NULL) && !m_strFileName.IsEmpty())
	{
	//	Only one extensive hard drive access at a time to reduce I/O load
#ifndef NEW_SOCKETS_ENGINE
		CSingleLock sLock(&g_App.m_csPreventExtensiveHDAccess, TRUE);
#endif //NEW_SOCKETS_ENGINE

		CKnownFile	*pNewKnownFile = new CKnownFile();
		if (pNewKnownFile != NULL)
		{
		//	Notify regarding operation start to switch file state
			if (m_bChangeState && g_App.m_pMDlg)
				PostMessage(g_App.m_pMDlg->m_hWnd, TM_HASHINGSTARTED, (WPARAM)m_pPartFile, NULL);
		//	Try to create a hashed KnownFile for the spec'd file. If successful...
			if (pNewKnownFile->CreateFromFile(m_strDirectory, m_strFileName, true))
			{
			//	Notify the PartFile (via a message to the eMule window) that the file it was
			//		completing has finished hashing.
#ifndef NEW_SOCKETS_ENGINE
				if (g_App.m_pMDlg)
				{
					PostMessage( g_App.m_pMDlg->m_hWnd, TM_FINISHEDHASHING,
						(WPARAM)m_pPartFile, (LPARAM)pNewKnownFile );
				}
#endif //NEW_SOCKETS_ENGINE
			}
			else
			{
				delete pNewKnownFile;
			}
		}

	//	m_csPreventExtensiveHDAccess is unlocked here
	}
	AfxEndThread(0, true);
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
