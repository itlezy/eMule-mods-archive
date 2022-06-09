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
#include "PartFile.h"
#include "SafeFile.h"
#ifndef NEW_SOCKETS_ENGINE
#include "BarShader.h"
	#include "emule.h"
	#include "updownclient.h"
#else
	#include "Engine/Data/Prefs.h"
#endif //NEW_SOCKETS_ENGINE
#include "server.h"
#include "ED2KLink.h"
#include "Preview.h"
#include "ini2.h"
#include "SharedFileList.h"
#include "ArchiveRecovery.h"
#include <sys/stat.h>
#include <share.h>
#include <io.h>
#include "MMServer.h"
#include "otherstructs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define PROGRESS_HEIGHT 4

CPartFile::CPartFile()
{
	Init();
}

CPartFile::CPartFile(CSearchFile* searchresult, EnumCategories eCatID)
{
	EMULE_TRY
#ifndef NEW_SOCKETS_ENGINE
	Init();

	md4cpy(m_fileHash, searchresult->GetFileHash());
	for (int i = 0; i < searchresult->m_tagArray.GetCount(); i++)
	{
		const CTag *pTag = searchresult->m_tagArray[i];

		switch (pTag->GetTagID())
		{
			case FT_FILENAME:
				if (pTag->IsStr())
				{
#ifdef _UNICODE
					if (IsFileNameEmpty())
#endif
						SetFileName(pTag->GetStringValue());
				}
				break;

			case FT_FILESIZE:
				SetFileSize(searchresult->GetFileSize());
				break;

			default:
			{
			//	Keep only known and valid tags to avoid possible side effects
				if ( (pTag->GetTagID() != 0) && (pTag->GetTagName() == NULL) &&
					(pTag->IsStr() || pTag->IsInt()) )
				{
					static const struct
					{
						byte	byteID;
						byte	byteType;
					} s_aTags[] =
					{
						{ FT_MEDIA_ARTIST,	TAGTYPE_STRING },
						{ FT_MEDIA_ALBUM,	TAGTYPE_STRING },
						{ FT_MEDIA_TITLE,	TAGTYPE_STRING },
						{ FT_MEDIA_LENGTH,	TAGTYPE_UINT32 },
						{ FT_MEDIA_BITRATE,	TAGTYPE_UINT32 },
						{ FT_MEDIA_CODEC,	TAGTYPE_STRING },
						{ FT_FILETYPE,		TAGTYPE_STRING },
						{ FT_FILEFORMAT,	TAGTYPE_STRING }
					};

					for (unsigned ui = 0; ui < ARRSIZE(s_aTags); ui++)
					{
						if (pTag->GetTagID() == s_aTags[ui].byteID && pTag->GetType() == s_aTags[ui].byteType)
						{
						//	Skip string tags with empty string values
							if (pTag->IsStr() && pTag->IsStringValueEmpty())
								break;
						//	Skip integer tags with '0' values
							if (pTag->IsInt() && (pTag->GetIntValue() == 0))
								break;

							CTag	*pNewTag = new CTag(*pTag);
							m_tagArray.Add(pNewTag);
							break;
						}
					}
				}
			}
		}
	}
//	Don't ask for a hashset, if there is none
	if (GetFileSize() < PARTSIZE)
		m_bHashSetNeeded = false;

//	Set Category, don't save data yet
	if (eCatID == CAT_NONE)
		g_App.m_pDownloadQueue->SetAutoCat(this);
	else
		SetCatID(eCatID);

	CreatePartFile();

#endif //NEW_SOCKETS_ENGINE
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPartFile::CPartFile(CString edonkeylink, EnumCategories eCatID)
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	CED2KLink* pLink = NULL;
	try
	{
		pLink = CED2KLink::CreateLinkFromUrl(edonkeylink);
		_ASSERT(pLink != NULL);
		CED2KFileLink* pFileLink = pLink->GetFileLink();
		if (pFileLink == NULL)
			throw CString(_T("not an ed2k server or file link"));
		InitializeFromLink(pFileLink, eCatID);
	}
	catch (CString error)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_INVALIDLINK, error);
		SetStatus(PS_ERROR);
	}
	delete pLink;
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPartFile::CPartFile(CED2KFileLink* fileLink, EnumCategories eCatID)
{
	InitializeFromLink(fileLink, eCatID);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::InitializeFromLink(CED2KFileLink* fileLink, EnumCategories eCatID)
{
	EMULE_TRY

	Init();

	try
	{
		SetFileName(fileLink->GetName());
		SetFileSize(fileLink->GetSize());
	//	Prevent to donload an empty file
		if (!GetFileSize())
		{
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_SKIPZEROLENGTHFILE, m_strFileName);
#endif
			SetStatus(PS_ERROR);
			return;
		}
	//	Don't ask for a hashset, if there is none
		if (GetFileSize() < PARTSIZE)
			m_bHashSetNeeded = false;
		md4cpy(m_fileHash, fileLink->GetHashKey());
#ifndef NEW_SOCKETS_ENGINE
		if (!g_App.m_pDownloadQueue->FileExists(m_fileHash))
		{
		//	Set Category, don't save data yet
			if (eCatID == CAT_NONE)
				g_App.m_pDownloadQueue->SetAutoCat(this);
			else
				SetCatID(eCatID);

			CreatePartFile();
		}
		else
#endif //NEW_SOCKETS_ENGINE
			SetStatus(PS_ERROR);
	}
	catch (CString error)
	{
		OUTPUT_DEBUG_TRACE();
#ifndef NEW_SOCKETS_ENGINE
		AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_INVALIDLINK, error);
#endif
		SetStatus(PS_ERROR);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::Init()
{
	EMULE_TRY

	m_strTempDir = _T("");
	m_dwLastFileSourcesRequestTime = 0;
	m_nLastBufferFlushTime = m_dwLastPurgeTime = ::GetTickCount();
	m_bPaused = false;
	m_bStopped = false;
	SetStatus(PS_EMPTY);
	m_qwBytesTransferred = 0;
	priority = PR_NORMAL;
	m_uNumTransferringSrcs = 0;
	m_uSrcNNP = 0;
	m_uSrcOnQueue = 0;
	m_uSrcHighQR = 0;
	m_uSrcConnecting = 0;
	m_uSrcWaitForFileReq = 0;
	m_uSrcConnected = 0;
	m_uSrcConnViaServer = 0;
	m_uSrcLowToLow = 0;
	m_uSrcLowIDOnOtherServer = 0;
	m_uSrcQueueFull = 0;
	m_uSrcA4AF = 0;
	m_dwDataRate = 0;
	m_bHashSetNeeded = true;
	m_uProcessCounter = 0;
	m_dblPercentCompleted = 0.0;
	m_strPartMetFileName = _T("");
	m_qwCompletedSize = 0;
	m_bPreviewing = false;
	lastseencomplete = NULL;
	m_dwAvailablePartsCount = 0;
	m_ClientSrcAnswered = 0;
	m_LastNoNeededCheck = 0;
	m_eRating = PF_RATING_NONE;
	m_nTotalBufferData = 0;
	m_bRecoveringArchive = false;
	m_qwGainDueToCompression = 0;
	m_qwLostDueToCorruption = 0;
	m_iTotalPacketsSavedDueToICH = 0;
	m_bHasRating = false;
	m_bHasComment = false;
	m_uLastCompleteSrcCount = 0;
	m_SessionStartTime = m_timeLastDownTransfer = CTime::GetCurrentTime();
	m_qwSessionStartSize = 0;
	m_AvgDataRate = 0;
	m_StartTimeReset = false;
	m_eCategoryID = CAT_NONE;
	m_bIsBeingDeleted = false;
	m_bIsPreallocated = false;
//	Async data flush (i.e. only in process)
	m_bDataFlushReq = false;
//	Set to true for fakes.rar download
	m_bIsFakesDotRar = false;
//	Set intial gaps sum to 0
	m_qwGapsSum = 0;
//	Set total size of completed parts to 0
	m_qwCompletedPartsSize = 0;

#ifndef NEW_SOCKETS_ENGINE
	if (g_App.m_pPrefs->IsDAPEnabled())
		m_bAutoPriority = true;
	else
#endif //NEW_SOCKETS_ENGINE
		m_bAutoPriority = false;

	m_bIsPreviewableArchive = false;
	m_cIsMovie = 0;
	m_bIsMpgMovie  = false;
	m_bIsMpgAudio = false;

	::InitializeCriticalSection(&m_csSourceLists);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPartFile::~CPartFile()
{
	EMULE_TRY

	if (!m_bIsBeingDeleted && (m_hPartFileWrite.m_hFile != INVALID_HANDLE_VALUE))
	{
	//	Ensure all buffered data is written
		FlushBuffer();

		ClosePartFile();
	//	Update met file
		SavePartFile();
	}

	m_srcPartFrequencies.RemoveAll();
	m_PartsStatusVector.clear();

//	Will be unlocked on exit
	CSingleLock lockGap(&m_csGapListAndPartStatus, TRUE);

	while (!gaplist.IsEmpty())
		delete gaplist.RemoveHead();

	::DeleteCriticalSection(&m_csSourceLists);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::CreatePartFile()
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	m_strTempDir = g_App.m_pPrefs->GetTempDir();
#else
	m_strTempDir = g_stEngine.Prefs.GetTempDir();
#endif //NEW_SOCKETS_ENGINE

	if (GetCatID() != CAT_NONE)
	{
		CString strCatTempDir =	CCat::GetCatByID(GetCatID())->GetTempPath();
		if (!strCatTempDir.IsEmpty())
			m_strTempDir = strCatTempDir;
	}

	if (m_strTempDir.Right(1) == _T('\\'))
	{
		m_strTempDir.Truncate(m_strTempDir.GetLength() - 1);
	}

//	Check that file size is supported by the protocol and the current partition
	if (GetFileSize() > 0xFFFFFFFFui64)
	{
		UINT	dwResStrId;

		for (;;)
		{
			if (GetFileSize() > MAX_EMULE_FILE_SIZE)
				dwResStrId = IDS_ERR_TOOLARGEFILE;
			else if (IsFileOnFATVolume(m_strTempDir))
				dwResStrId = IDS_ERR_TOOLARGEFILE4FS;
			else
				break;
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_FILEOPEN, GetFileName(), GetResString(dwResStrId));
			SetStatus(PS_ERROR);
			return;
		}
	}

	SetPath(m_strTempDir);

	int iPartNumber = 0;
	CString strFullPartFileName;

	do
	{
		strFullPartFileName.Format(_T("%s\\%03u.part"), m_strTempDir, ++iPartNumber);
	}
	while (PathFileExists(strFullPartFileName));

	m_strPartMetFileName.Format(_T("%03u.part.met"), iPartNumber);
	m_strFullName.Format(_T("%s.met"), strFullPartFileName);
	SetFilePath(strFullPartFileName);

	CString	buffer = m_strPartMetFileName.Left(m_strPartMetFileName.GetLength() - 4);
	CTag	*pPartNameTag = new CTag(FT_PARTFILENAME, buffer);

	m_tagArray.Add(pPartNameTag);

	if (!m_hPartFileWrite.Open(strFullPartFileName, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::osSequentialScan))
	{
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_CREATEPARTFILE);
		SetStatus(PS_ERROR);
	}
	if (!m_hPartFileRead.Open(strFullPartFileName, CFile::modeRead | CFile::shareDenyNone | CFile::osSequentialScan))
	{
		m_hPartFileWrite.Close();
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_CREATEPARTFILE);
		SetStatus(PS_ERROR);
	}

	const uint32 dwPartCnt = GetPartCount();

	m_srcPartFrequencies.SetSize(dwPartCnt);
	m_PartsStatusVector.resize(dwPartCnt);

	for (uint32 i = 0; i < dwPartCnt; i++)
	{
		m_srcPartFrequencies[i] = 0;
		m_PartsStatusVector[i] = 0;
	}

//	Note: the gap should be initialized after parts status vector was filled
	AddGap(0, GetFileSize() - 1ui64);

	m_bPaused = false;

#ifndef NEW_SOCKETS_ENGINE
//	Before we save file we need to establish UAP
	if (g_App.m_pPrefs->IsUAPEnabled())
	{
		SetAutoULPriority(true);
		SetULPriority(PR_RELEASE);
		UpdateSharedFileDisplay();
	}
#endif //NEW_SOCKETS_ENGINE

//	Don't request hashset for file which size < PARTSIZE
//	NB! file with size = PARTSIZE has hashset consisted of two hashes
	if (GetFileSize() < PARTSIZE)
		m_bHashSetNeeded = false;

#ifndef NEW_SOCKETS_ENGINE
//	Filename cleanup
	if (g_App.m_pPrefs->GetAutoFilenameCleanup())
		SetFileName(CleanupFilename(GetFileName()));
#endif //NEW_SOCKETS_ENGINE

	SavePartFile();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::LoadPartFile(LPCTSTR in_directory, LPCTSTR in_filename)
{
	EMULE_TRY

	CMap<uint16, uint16, Gap_Struct*, Gap_Struct*> gap_map;
	CSafeBufferedFile file;
	CString	strCorruptedParts;

	m_qwBytesTransferred = 0;
	m_strPartMetFileName = in_filename;
	m_strTempDir = in_directory;
	if (m_strTempDir.Right(1) == _T('\\'))
		m_strTempDir.Truncate(m_strTempDir.GetLength() - 1);
	SetPath(m_strTempDir);

	m_strFullName.Format(_T("%s\\%s"), m_strTempDir, m_strPartMetFileName);
	SetFilePath(m_strFullName.Left(m_strFullName.GetLength() - 4));

	try
	{
	//	Read file data form part.met file
		if (!file.Open(m_strFullName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
		{
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_RGB_ERROR, IDS_ERR_OPENMET, m_strPartMetFileName, m_strFileName);
			g_App.m_pMDlg->DisableAutoBackup();
#endif
			return false;
		}
		byte version;
		file.Read(&version, 1);
		if ((version != PARTFILE_VERSION) && (version != PARTFILE_VERSION_LARGEFILE))
		{
			file.Close();
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_RGB_ERROR, IDS_ERR_BADMETVERSION, m_strPartMetFileName, m_strFileName);
			g_App.m_pMDlg->DisableAutoBackup();
#endif
			return false;
		}
		LoadDateFromFile(file);
		if (!LoadHashsetFromFile(file, false))
		{
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_HASHSETCORR, in_filename);
#endif
		}
		LoadSettingsFile();

		uint32	dwTagCnt;

		file.Read(&dwTagCnt, 4);
		for (uint32 j = 0; j < dwTagCnt; j++)
		{
			CTag* newtag = new CTag();

			newtag->FillFromStream(file);
			switch (newtag->GetTagID())
			{
				case FT_FILENAME:
					if (!newtag->IsStr())
					{
#ifndef NEW_SOCKETS_ENGINE
						AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_METCORRUPT, m_strPartMetFileName, m_strFileName);
						delete newtag;
						g_App.m_pMDlg->DisableAutoBackup();
#endif //NEW_SOCKETS_ENGINE

						return false;
					}
#ifdef _UNICODE
					if (IsFileNameEmpty())
#endif
						SetFileName(newtag->GetStringValue());
					break;

				case FT_LASTSEENCOMPLETE:
					if (newtag->IsInt())
						lastseencomplete = newtag->GetIntValue();
					break;

				case FT_FILESIZE:
					if (newtag->IsAnyInt())
						SetFileSize(newtag->GetInt64Value());
					break;

				case FT_TRANSFERRED:
					if (newtag->IsAnyInt())
						newtag->GetInt64Value(&m_qwBytesTransferred);
					break;

				case FT_COMPRESSION:
					if (newtag->IsAnyInt())
						newtag->GetInt64Value(&m_qwGainDueToCompression);
					break;

				case FT_CORRUPTED:
					if (newtag->IsAnyInt())
						newtag->GetInt64Value(&m_qwLostDueToCorruption);
					break;

				case FT_DLPRIORITY:
					if (newtag->IsInt())
					{
						if ((priority = static_cast<byte>(newtag->GetIntValue())) == PR_AUTO)
						{
							SetAutoPriority(true);
							priority = PR_HIGH;
						}
						else
						{
							SetAutoPriority(false);
							if ((priority != PR_LOW) && (priority != PR_NORMAL) && (priority != PR_HIGH))
								priority = PR_NORMAL;
						}
					}
					break;

				case FT_STATUS:
					if (newtag->IsInt())
						m_bPaused = (newtag->GetIntValue() != 0);
					break;

				case FT_ULPRIORITY:
					if (newtag->IsInt())
					{
						byte	byteULPrio = static_cast<byte>(newtag->GetIntValue());

						if (byteULPrio == PR_AUTO)
						{
							SetAutoULPriority(true);
							SetULPriority(PR_RELEASE);
						}
						else
						{
							SetAutoULPriority(false);
							if ( (byteULPrio != PR_VERYLOW) && (byteULPrio != PR_LOW) &&
								(byteULPrio != PR_NORMAL) && (byteULPrio != PR_HIGH) && (byteULPrio != PR_RELEASE) )
							{
								byteULPrio = PR_NORMAL;
							}
							SetULPriority(byteULPrio);
						}
					}
					break;

				case FT_CATEGORY:
					if (newtag->IsInt())
					{
						m_eCategoryID = static_cast<_EnumCategories>(newtag->GetIntValue());
						if (CCat::GetCatIndexByID(m_eCategoryID) < 0)
							m_eCategoryID = CAT_NONE;
					}
					break;

				case FT_PERMISSIONS:
					if (newtag->IsInt() && (newtag->GetIntValue() <= PERM_NOONE))
						SetPermissions((byte)newtag->GetIntValue());
					break;

				case FT_ATTRANSFERRED:						// (FT_ATTRANSFERRED:int) all time transferred (low long)
					if (newtag->IsInt())
						statistic.m_qwAllTimeTransferred = (statistic.m_qwAllTimeTransferred & (0xFFFFFFFFi64 << 32)) | newtag->GetIntValue();
					break;

				case FT_ATTRANSFERREDHI:					// (FT_ATTRANSFERREDHI:int) all time transferred (high long)
					if (newtag->IsInt())
						statistic.m_qwAllTimeTransferred = (statistic.m_qwAllTimeTransferred & 0xFFFFFFFF) |
							(static_cast<uint64>(newtag->GetIntValue()) << 32);
					break;

				case FT_ATREQUESTED:						// (FT_ATREQUESTED:int) all time requested
					if (newtag->IsInt())
						statistic.m_dwAllTimeRequested = newtag->GetIntValue();
					break;

				case FT_ATACCEPTED:							// (FT_ATACCEPTED:int) all time accepted
					if (newtag->IsInt())
						statistic.m_dwAllTimeAccepted = newtag->GetIntValue();
					break;

				case FT_CORRUPTEDPARTS:
					if (newtag->IsStr())
						newtag->GetStringValue(&strCorruptedParts);
					break;

				default:
				{
					const char	*pcTagName = newtag->GetTagName();

					if ((newtag->GetTagID() == 0) && (pcTagName == NULL))
						break;
					if ((pcTagName != NULL) && (pcTagName[0] == FT_GAPSTART || pcTagName[0] == FT_GAPEND))
					{
						if (newtag->IsAnyInt())
						{
							Gap_Struct *gap;
							uint16 gapkey = static_cast<uint16>(atoi(&pcTagName[1]));

							if (!gap_map.Lookup(gapkey, gap))
							{
								gap = new Gap_Struct;
								gap_map.SetAt(gapkey, gap);
								gap->qwStartOffset = ~0ui64;
								gap->qwEndOffset = ~0ui64;
							}
							if (pcTagName[0] == FT_GAPSTART)
								newtag->GetInt64Value(&gap->qwStartOffset);
							if (pcTagName[0] == FT_GAPEND)
							{
								newtag->GetInt64Value(&gap->qwEndOffset);
								gap->qwEndOffset--;
							}
						}
					}
					else
					{
						m_tagArray.Add(newtag);
						continue;
					}
				}
			}
			delete newtag;
		}
		file.Close();
#if 1	//code left for smooth migration, delete in v1.2f

		if (m_strAlternativePath.IsEmpty())
		{
		CStdioFile	f;
		CString		strPath(m_strFilePath);

		strPath += _T(".dir");
		if (f.Open(strPath, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
		{
			f.ReadString(m_strAlternativePath);
			f.Close();
		}
		if (!m_strAlternativePath.IsEmpty())	//	resave old setting to the new location
			SaveSettingsFile();
		}
#endif
	}
	catch (CFileException * error)
	{
		OUTPUT_DEBUG_TRACE();
#ifndef NEW_SOCKETS_ENGINE
		if (error->m_cause == CFileException::endOfFile)
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_METCORRUPT, m_strPartMetFileName, m_strFileName);
		else
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_FILEERROR, m_strPartMetFileName, m_strFileName, GetErrorMessage(error));
#endif //NEW_SOCKETS_ENGINE
		error->Delete();
		for (POSITION pos = gap_map.GetStartPosition(); pos != NULL;)
		{
			Gap_Struct* gap;
			uint16 gapkey;
			gap_map.GetNextAssoc(pos, gapkey, gap);
			delete gap;
		}
#ifndef NEW_SOCKETS_ENGINE
		g_App.m_pMDlg->DisableAutoBackup();
#endif //NEW_SOCKETS_ENGINE

		return false;
	}

//	At this point, the file size was defined. initialize related variables
	const uint32 dwPartCnt = GetPartCount();

	m_srcPartFrequencies.SetSize(dwPartCnt);
	m_PartsStatusVector.resize(dwPartCnt);

	for (uint32 i = 0; i < dwPartCnt; i++)
	{
		m_srcPartFrequencies[i] = 0;
		m_PartsStatusVector[i] = PART_VERIFIED;	// AddGap will reset this where required
	}

//	Now to flush the map into the list
	for (POSITION pos = gap_map.GetStartPosition(); pos != NULL;)
	{
		Gap_Struct* gap;
		uint16 gapkey;
		gap_map.GetNextAssoc(pos, gapkey, gap);

		if ( (gap->qwStartOffset != ~0ui64) && (gap->qwEndOffset != ~0ui64) &&
			(gap->qwStartOffset <= gap->qwEndOffset) && (gap->qwStartOffset < GetFileSize()) )
		{
			if (gap->qwEndOffset >= GetFileSize())
				gap->qwEndOffset = GetFileSize() - 1ui64;	// Clipping
			AddGap(gap->qwStartOffset, gap->qwEndOffset);	// All tags accounted for, use safe adding
		}
		delete gap;
	}

//	Check that file size is supported by the protocol and the current partition
	if (GetFileSize() > 0xFFFFFFFFui64)
	{
		UINT	dwResStrId;

		for (;;)
		{
			if (GetFileSize() > MAX_EMULE_FILE_SIZE)
				dwResStrId = IDS_ERR_TOOLARGEFILE;
			else if (IsFileOnFATVolume(m_strTempDir))
				dwResStrId = IDS_ERR_TOOLARGEFILE4FS;
			else
				break;
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_FILEOPEN, m_strFilePath, GetResString(dwResStrId));
			return false;
		}
	}

	if (!strCorruptedParts.IsEmpty())
	{
		CString	strPart;
		uint32	dwPart;

		for (int iPos = 0;;)
		{
			strPart = strCorruptedParts.Tokenize(_T(","), iPos);
			if (strPart.IsEmpty())
				break;

			if (_stscanf(strPart, _T("%u"), &dwPart) == 1)
			{
				if (dwPart < dwPartCnt)
				{
					m_csGapListAndPartStatus.Lock();
					m_PartsStatusVector[dwPart] |= PART_CORRUPTED;
					m_csGapListAndPartStatus.Unlock();
				}
			}
		}
	}

//	Calculate the size of completed parts
	for (uint32 i = 0; i < GetPartCount(); i++)
	{
		uint64 qwStart, qwEnd;

		if (IsPartComplete(i, &qwStart, &qwEnd))
			m_qwCompletedPartsSize += qwEnd - qwStart + 1ui64;
	}

	EMULE_TRY

	LoadPartFileStats();

	EMULE_CATCH

//	Open permanent handles
	if (!m_hPartFileWrite.Open(m_strFilePath, CFile::modeWrite | CFile::shareDenyWrite | CFile::osSequentialScan))
	{
#ifndef NEW_SOCKETS_ENGINE
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_FILEOPEN, m_strFilePath, m_strFileName);
#endif
		return false;
	}
	if (!m_hPartFileRead.Open(m_strFilePath, CFile::modeRead | CFile::shareDenyNone | CFile::osSequentialScan))
	{
		m_hPartFileWrite.Close();
#ifndef NEW_SOCKETS_ENGINE
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_FILEOPEN, m_strFilePath, m_strFileName);
#endif
		return false;
	}

	uint64	qwFileSz = m_hPartFileWrite.GetLength();

	if (qwFileSz < GetFileSize())
		AddGap(qwFileSz, GetFileSize() - 1ui64);

//	Goes both ways - Partfile should never be too large
	if (qwFileSz > GetFileSize())
	{
		TRACE("Partfile \"%s\" is too large! Truncating %I64u bytes.\n", GetFileName(), qwFileSz - GetFileSize());
		m_hPartFileWrite.SetLength(GetFileSize());
	}

	SetStatus(PS_EMPTY);

	UpdateCompletedInfos();
//	Smoothed "remaining time" calculation
	m_qwSessionStartSize = m_qwCompletedSize;

	FILETIME	ftModify;

	if (::GetFileTime(m_hPartFileWrite, NULL, NULL, &ftModify))
		m_timeLastDownTransfer = CTime(ftModify);

//	Check hashcount, filestatus etc.
	CSingleLock Lock(&m_csHashList, TRUE);
	uint32 dwSize = m_partHashArray.GetCount();
	Lock.Unlock();
	if (dwSize != GetED2KPartHashCount())
	{
		m_bHashSetNeeded = true;
		return true;
	}
	else
	{
		m_bHashSetNeeded = false;
		for (int i = 0; i < GetPartCount(); i++)
		{
			if (IsPartComplete(i))
			{
				SetStatus(PS_READY);
				break;
			}
		}
	}
	m_csGapListAndPartStatus.Lock();

	bool	bIsGapListEmpty = B2b(gaplist.IsEmpty());

	m_csGapListAndPartStatus.Unlock();

// Is this file complete already?
	if (bIsGapListEmpty)
	{
		CompleteFile(false);
		return true;
	}

//	Check file integrity only when at least one full chunk is available
//	to make it faster and to avoid adding of a non-shareable file to the shared files list
	if (m_eStatus == PS_READY)
	{
		uint32	dwFileDate = static_cast<uint32>(m_timeLastDownTransfer.GetTime());

	//	Check date of .part file - if it's wrong, rehash file
		AdjustNTFSDaylightFileTime(&dwFileDate, m_strFilePath);
		if (m_timetLastWriteDate != dwFileDate)
		{
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_RGB_WARNING, IDS_ERR_REHASH, m_strFileName);
#endif
		//	Rehash
			SetStatus(PS_WAITINGFORHASH);
#ifndef NEW_SOCKETS_ENGINE
			CHashFileThread* addfilethread = (CHashFileThread*) AfxBeginThread(RUNTIME_CLASS(CHashFileThread), THREAD_PRIORITY_NORMAL + g_App.m_pPrefs->GetMainProcessPriority(), 0, CREATE_SUSPENDED);
			addfilethread->SetValues(true, GetPath(), m_hPartFileWrite.GetFileName().GetBuffer(), this);
			addfilethread->ResumeThread();
#endif
		}
	}

	if (m_bPaused)
		SetStartTimeReset(true);

//	load own file comments and ratings
	UpdateFileRatingCommentAvail();

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::TryToRecoverPartFile(LPCTSTR in_directory, LPCTSTR in_filename)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	CString strTempName;

	m_strPartMetFileName = in_filename;
	m_strTempDir = in_directory;
	if (m_strTempDir.Right(1) == _T('\\'))
		m_strTempDir.Truncate(m_strTempDir.GetLength() - 1);
	SetPath(m_strTempDir);

	m_strFullName.Format(_T("%s\\%s"), m_strTempDir, m_strPartMetFileName);
	SetFilePath(m_strFullName.Left(m_strFullName.GetLength() - 4));

//	Rename ".met" to ".met.bad"
	strTempName.Format(_T("%s.bad"), m_strFullName);
	if (_trename(m_strFullName, strTempName) != 0)
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to rename part.met file '%s' to '%s' - %s"), m_strFullName, strTempName, _tcserror(errno));

//	Copy ".met.bak" to ".met"
	strTempName.Format(_T("%s.bak"), m_strFullName);
	if (CopyFile(strTempName, m_strFullName, false) == 0)
	{
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to copy .bak file '%s' to '%s'"), strTempName, m_strFullName);
		AddLogLine(LOG_RGB_ERROR, IDS_TRYTORECOVER_FAIL, m_strFullName);
		return false;
	}

	AddLogLine(0, IDS_TRYTORECOVER, m_strFullName);

	bool bLoadPartFileStatus = LoadPartFile(in_directory, in_filename);

	if (bLoadPartFileStatus)
		AddLogLine(LOG_RGB_SUCCESS, IDS_RECOVERED_PARTMET, GetFileName());
	else
		AddLogLine(LOG_RGB_ERROR, IDS_TRYTORECOVER_FAIL, m_strFullName);

	return bLoadPartFileStatus;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::SavePartFile()
{
	EMULE_TRY

	switch (m_eStatus)
	{
		case PS_WAITINGFORHASH:
		case PS_HASHING:
		case PS_COMPLETING:
		case PS_COMPLETE:
			return false;
	}

//	Protect file corruption during calling from different threads
//	will be unlocked on exit
	CSingleLock sLock(&m_csSavePartFile, TRUE);

//	Get filedate
	CFileFind ff;
	bool bEnd = !ff.FindFile(m_strFilePath, 0);
	if (!bEnd)
		ff.FindNextFile();
	if (bEnd || ff.IsDirectory())
	{
#ifndef NEW_SOCKETS_ENGINE
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_SAVEMET, GetResString(IDS_ERR_PART_FNF), m_strPartMetFileName, m_strFileName);
#endif
		return false;
	}

//	Create a backup of ".met"
	::CopyFile(m_strFullName, m_strFullName + _T(".bak"), FALSE);

	CTime lwtime;
	ff.GetLastWriteTime(lwtime);
	m_timetLastWriteDate = static_cast<uint32>(lwtime.GetTime());
	AdjustNTFSDaylightFileTime(&m_timetLastWriteDate, ff.GetFilePath());
	ff.Close();

//	Create a temporary file
	CString strTempName(m_strFullName);

	strTempName.Append(_T(".tmp"));

//	To save data safely CMemFile is used first to prepare the whole output (to reduce
//	number of filesystem I/O), then data is flushed to CFile in one shot.
//	Previous solution based on CStdioFile is not suitable for this case, as
//	CStdioFile can't generate proper exceptions due to the fact that it buffers
//	data before writing. As a result exceptions were not generated for many error cases
//	causing different file corruptions...
	CFile		file;
	CMemFile	MFile(8 * 1024);

	bool	bLargeFile = IsLargeFile();
	byte	byteVer = (bLargeFile) ? PARTFILE_VERSION_LARGEFILE : PARTFILE_VERSION;

	MFile.Write(&byteVer, 1);	//	Version
	MFile.Write(&m_timetLastWriteDate, 4);	//	Date
	MFile.Write(&m_fileHash, 16);	//	Hash

	CSingleLock Lock(&m_csHashList, TRUE);
	uint16 uParts = static_cast<uint16>(m_partHashArray.GetCount());
	MFile.Write(&uParts, 2);
	for (int x = 0; x != uParts; x++)
		MFile.Write(m_partHashArray[x], 16);
	Lock.Unlock();

//	Tags
	CArray<Gap_Struct, Gap_Struct> aGaps;
	CWrTag	tagWr;
	uint32	dwTmp, dwTagFilePos, dwTagCount = 0;

	dwTagFilePos = static_cast<uint32>(MFile.GetPosition());
	MFile.Write(&dwTagCount, 4);

	if (IsUTF8Required(GetFileName()))
	{
		tagWr.WriteToFile(FT_FILENAME, GetFileName(), MFile, cfUTF8withBOM);
		dwTagCount++;
	}
	tagWr.WriteToFile(FT_FILENAME, GetFileName(), MFile);
	dwTagCount++;
	tagWr.WriteToFile(FT_FILESIZE, GetFileSize(), MFile, bLargeFile);
	dwTagCount++;
	if (m_qwBytesTransferred != 0)
	{
		tagWr.WriteToFile(FT_TRANSFERRED, m_qwBytesTransferred, MFile, bLargeFile);
		dwTagCount++;
	}
//	Compression gain can be negative in case of disconnection in the middle of
//	compressed stream, but we're not going to save it for compatibility purposes
	if (static_cast<sint64>(m_qwGainDueToCompression) > 0)
	{
		for (;;)
		{
			if (m_qwGainDueToCompression <= 0xFFFFFFFFui64)
				dwTmp = static_cast<uint32>(m_qwGainDueToCompression);
			else if (!bLargeFile)
				dwTmp = 0xFFFFFFFF;
			else
			{
				tagWr.WriteToFile(FT_COMPRESSION, m_qwGainDueToCompression, MFile, true);
				break;
			}
			tagWr.WriteToFile(FT_COMPRESSION, dwTmp, MFile);
			break;
		}
		dwTagCount++;
	}
	if (m_qwLostDueToCorruption != 0)
	{
		for (;;)
		{
			if (m_qwLostDueToCorruption <= 0xFFFFFFFFui64)
				dwTmp = static_cast<uint32>(m_qwLostDueToCorruption);
			else if (!bLargeFile)
				dwTmp = 0xFFFFFFFF;
			else
			{
				tagWr.WriteToFile(FT_CORRUPTED, m_qwLostDueToCorruption, MFile, true);
				break;
			}
			tagWr.WriteToFile(FT_CORRUPTED, dwTmp, MFile);
			break;
		}
		dwTagCount++;
	}
	if (m_bPaused)
	{
		tagWr.WriteToFile(FT_STATUS, 1, MFile);
		dwTagCount++;
	}
	tagWr.WriteToFile(FT_DLPRIORITY, (IsAutoPrioritized()) ? PR_AUTO : priority, MFile);
	dwTagCount++;
	tagWr.WriteToFile(FT_ULPRIORITY, (IsULAutoPrioritized()) ? PR_AUTO : GetULPriority(), MFile);
	dwTagCount++;
	if (static_cast<uint32>(lastseencomplete.GetTime()) != 0)
	{
		tagWr.WriteToFile(FT_LASTSEENCOMPLETE, static_cast<uint32>(lastseencomplete.GetTime()), MFile);
		dwTagCount++;
	}
	if (m_eCategoryID != CAT_NONE)
	{
		tagWr.WriteToFile(FT_CATEGORY, static_cast<uint32>(m_eCategoryID), MFile);
		dwTagCount++;
	}
	if (statistic.GetAllTimeTransferred() != 0)
	{
		tagWr.WriteToFile(FT_ATTRANSFERRED, static_cast<uint32>(statistic.GetAllTimeTransferred()), MFile);
		if ((dwTmp = static_cast<uint32>(statistic.GetAllTimeTransferred() >> 32)) != 0)
		{
			tagWr.WriteToFile(FT_ATTRANSFERREDHI, dwTmp, MFile);
			dwTagCount++;
		}
		dwTagCount++;
	}
	if (statistic.GetAllTimeRequests() != 0)
	{
		tagWr.WriteToFile(FT_ATREQUESTED, statistic.GetAllTimeRequests(), MFile);
		dwTagCount++;
	}
	if (statistic.GetAllTimeAccepts() != 0)
	{
		tagWr.WriteToFile(FT_ATACCEPTED, statistic.GetAllTimeAccepts(), MFile);
		dwTagCount++;
	}
//	Store corrupted part numbers
	CString	strCorruptedParts;
	uint32	j, dwTagNum;

	for (j = 0; j < GetPartCount(); j++)
	{
		if (IsCorruptedPart(j))
			strCorruptedParts.AppendFormat((strCorruptedParts.IsEmpty()) ? _T("%u") : _T(",%u"), j);
	}
	if (!strCorruptedParts.IsEmpty())
	{
		tagWr.WriteToFile(FT_CORRUPTEDPARTS, strCorruptedParts, MFile);
		dwTagCount++;
	}
	tagWr.WriteToFile(FT_PERMISSIONS, GetPermissions(), MFile);
	dwTagCount++;

	dwTagNum = m_tagArray.GetCount();
	dwTagCount += dwTagNum;
	for (j = 0; j < dwTagNum; j++)
		m_tagArray[j]->WriteToFile(MFile);

//	Make a gap list copy to avoid synchronization object allocation for a long time
	GetGapListCopy(&aGaps);

	char	acNameBuffer[16];
	void	*pBufBeg, *pBufEnd;
	uint32	dwPos = 0;

	Gap_Struct	*pGap = aGaps.GetData();
	Gap_Struct	*pGapMax = pGap + aGaps.GetCount();

	dwTagCount += 2 * aGaps.GetCount();
	for (; pGap < pGapMax; pGap++)
	{
		itoa(dwPos++, &acNameBuffer[1], 10);
		acNameBuffer[0] = FT_GAPSTART;
		tagWr.WriteToFile(acNameBuffer, pGap->qwStartOffset, MFile, bLargeFile);
	//	gap start = first missing byte but gap ends = first non-missing byte in edonkey
	//	but I think its easier to user the real limits
		acNameBuffer[0] = FT_GAPEND;
		tagWr.WriteToFile(acNameBuffer, pGap->qwEndOffset + 1ui64, MFile, bLargeFile);
	}

//	Save valid tag count
	MFile.Seek(dwTagFilePos, CFile::begin);
	MFile.Write(&dwTagCount, 4);
	MFile.SeekToBegin();

	try
	{
		if (!file.Open(strTempName, CFile::modeCreate | CFile::modeWrite))
		{
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERROR_SAVEFILE, strTempName);
#endif
			return false;
		}
	//	Steal stream buffer pointer and length without detaching it for easier destruction
		dwTmp = MFile.GetBufferPtr(CMemFile::bufferRead, ~0u, &pBufBeg, &pBufEnd);
		file.Write(pBufBeg, dwTmp);
		file.Flush();

		file.Close();	// Close can generate an exception as well - keep in try/catch
	}
	catch (CFileException *error)
	{
		OUTPUT_DEBUG_TRACE();
#ifndef NEW_SOCKETS_ENGINE
		AddLogLine(LOG_RGB_ERROR, IDS_ERR_SAVEMET, GetErrorMessage(error), m_strPartMetFileName, m_strFileName);
#endif
		error->Delete();
	//	About is used instead of Close as it doesn't generate exception which we don't need here
		file.Abort();
 	//	Remove the partially written or otherwise damaged temporary file
 		::DeleteFile(strTempName);
		return false;
	}

//	Delete original ".met", after successfully writing the temporary part.met file...
	if (_tremove(m_strFullName) != 0 && errno != ENOENT)
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to remove '%s' - %s"), m_strFullName, _tcserror(errno));

//	Rename ".met.tmp" to ".met"
	if (_trename(strTempName, m_strFullName) != 0)
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to move temporary part.met file '%s' to '%s' - %s"), strTempName, m_strFullName, _tcserror(errno));

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::FileRehashingStarted()
{
	if (m_eStatus == PS_WAITINGFORHASH)
		SetStatus(PS_HASHING);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::PartFileHashFinished(CKnownFile *pKnownFile)
{
	EMULE_TRY

	bool	bErrorFound = false;

	CSingleLock Lock(&m_csHashList, TRUE);

	uint32 dwCount = static_cast<uint32>(m_partHashArray.GetSize());

//	Number of parts to check taking into consideration files with size % PARTSIZE = 0 and size < PARTSIZE
	dwCount = (dwCount > GetPartCount()) ? GetPartCount() : dwCount;

	for (uint32 i = 0; i < dwCount; i++)
	{
		uint64 qwPartStart, qwPartEnd;

		if (IsPartComplete(i, &qwPartStart, &qwPartEnd))
		{
			if ((pKnownFile->GetPartHash(i) == NULL) || (md4cmp(pKnownFile->GetPartHash(i), GetPartHash(i)) != 0))
			{
#ifndef NEW_SOCKETS_ENGINE
				AddLogLine(LOG_RGB_WARNING, IDS_ERR_FOUNDCORRUPTION, i, GetFileName());
#endif
				AddGap(qwPartStart, qwPartEnd);
				m_csGapListAndPartStatus.Lock();
				m_PartsStatusVector[i] |= PART_CORRUPTED;
				m_csGapListAndPartStatus.Unlock();
			//	Increment corruption statistics
				AddRxCorruptedAmount(static_cast<uint32>(qwPartEnd - qwPartStart + 1));
				bErrorFound = true;
			}
		}
	}
	Lock.Unlock();
	delete pKnownFile;

	if (!bErrorFound)
	{
		if (m_eStatus == PS_COMPLETING)
		{
			CompleteFile(true);
			return;
		}
#ifndef NEW_SOCKETS_ENGINE
		else
			AddLogLine(0, IDS_HASHINGDONE, GetFileName());
#endif
	}
	else
	{
		SetStatus(PS_READY);
		SavePartFile();
	//	If a file is "ready" it can't be paused
		m_bPaused = false;
		return;
	}
	SetStatus(PS_READY);
	SavePartFile();
#ifndef NEW_SOCKETS_ENGINE
	g_App.m_pSharedFilesList->SafeAddKnownFile(this);
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::AddGap(uint64 qwStart, uint64 qwEnd)
{
	EMULE_TRY

	CSingleLock lockGap(&m_csGapListAndPartStatus, TRUE);

	POSITION pos1, pos2;

	for (pos1 = gaplist.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		Gap_Struct	*pGap = gaplist.GetNext(pos1);

		if ((pGap->qwStartOffset >= qwStart) && (pGap->qwEndOffset <= qwEnd))
		{ // this gap is inside the new gap - delete
			gaplist.RemoveAt(pos2);
			m_qwGapsSum -= pGap->qwEndOffset - pGap->qwStartOffset + 1ui64;
			FillGapInParts(pGap->qwStartOffset, pGap->qwEndOffset);
			delete pGap;
		}
		else if ((pGap->qwStartOffset >= qwStart) && (pGap->qwStartOffset <= qwEnd))
		{ // a part of this gap is in the new gap - extend limit and delete
			qwEnd = pGap->qwEndOffset;
			gaplist.RemoveAt(pos2);
			m_qwGapsSum -= qwEnd - pGap->qwStartOffset + 1ui64;
			FillGapInParts(pGap->qwStartOffset, qwEnd);
			delete pGap;
		}
		else if ((pGap->qwEndOffset <= qwEnd) && (pGap->qwEndOffset >= qwStart))
		{ // a part of this gap is in the new gap - extend limit and delete
			qwStart = pGap->qwStartOffset;
			gaplist.RemoveAt(pos2);
			m_qwGapsSum -= pGap->qwEndOffset - qwStart + 1ui64;
			FillGapInParts(qwStart, pGap->qwEndOffset);
			delete pGap;
		}
		else if ((qwStart >= pGap->qwStartOffset) && (qwEnd <= pGap->qwEndOffset))
		{ // new gap is already inside this gap - return
			return;
		}
	}
	Gap_Struct* new_gap = new Gap_Struct;
	new_gap->qwStartOffset = qwStart;
	new_gap->qwEndOffset = qwEnd;
	m_qwGapsSum += qwEnd - qwStart + 1ui64;
	AddGapToParts(qwStart, qwEnd);
	gaplist.AddTail(new_gap);

	lockGap.Unlock();

	UpdateDisplayedInfo();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsComplete() returns false if there are gaps in the range 'qwStart' to 'qwEnd' (inclusive).
bool CPartFile::IsComplete(uint64 qwStart, uint64 qwEnd)
{
	EMULE_TRY

	if (qwEnd >= GetFileSize())
		qwEnd = GetFileSize() - 1ui64;

	bool	bRc = true;

// 	Will be unlocked on exit
	CSingleLock lockGap(&m_csGapListAndPartStatus, TRUE);

	for (POSITION pos = gaplist.GetHeadPosition(); pos != NULL;)
	{
		Gap_Struct	*pGap = gaplist.GetNext(pos);

		if ( (pGap->qwStartOffset >= qwStart && pGap->qwEndOffset <= qwEnd)			// gap is inside range (this check is unnecessary)
		     || (pGap->qwStartOffset >= qwStart && pGap->qwStartOffset <= qwEnd)	// gap starts inside range
		     || (pGap->qwEndOffset <= qwEnd && pGap->qwEndOffset >= qwStart)		// gap ends inside range
		     || (qwStart >= pGap->qwStartOffset && qwEnd <= pGap->qwEndOffset) )	// range is inside gap
		{
			bRc = false;
			break;
		}
	}
	return bRc;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsPartComplete() returns true if a part is complete and verified.
bool CPartFile::IsPartComplete(uint32 dwPart) const
{
	return (m_PartsStatusVector[dwPart] == PART_VERIFIED);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsPartComplete() returns true if a part is complete and verified.
//		Params:
//			dwPart   - part number;
//			pqwStart - pointer to get part start offset;
//			pqwEnd   - pointer to get part end offset (inclusive).
bool CPartFile::IsPartComplete(uint32 dwPart, uint64 *pqwStart, uint64 *pqwEnd) const
{
	uint64 qwStart = static_cast<uint64>(dwPart) * PARTSIZE;
	uint64 qwEnd = ((qwStart + PARTSIZE - 1ui64) >= GetFileSize()) ? (GetFileSize() - 1ui64) : (qwStart + PARTSIZE - 1ui64);

	*pqwStart = qwStart;
	*pqwEnd = qwEnd;

	return (m_PartsStatusVector[dwPart] == PART_VERIFIED);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsPartFull() returns true if all data of the was received.
bool CPartFile::IsPartFull(uint32 dwPart) const
{
	return ((m_PartsStatusVector[dwPart] & 0xFFFFFF) == 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::IsPartDownloading(uint32 dwPart) const
{
	EMULE_TRY

	for (POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL;)
	{
		Requested_Block_Struct *pReqBlock = requestedblocks_list.GetNext(pos);

		if (dwPart == static_cast<uint32>(pReqBlock->qwStartOffset / PARTSIZE))
			return true;
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::IsAlreadyRequested(uint64 qwStart, uint64 qwEnd)
{
	EMULE_TRY

	for (POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL;)
	{
		Requested_Block_Struct	*cur_block = requestedblocks_list.GetNext(pos);

		if ((qwStart <= cur_block->qwEndOffset) && (qwEnd >= cur_block->qwStartOffset))
			return true;
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetGapsInPart() return request blocks for the corresponding part
//		Params:
//			dwPartNum   - a part to examine;
//			ppNewBlocks - output array for allocated request blocks (can be NULL to check for block allocation);
//			dwCount     - number of blocks to allocate.
//		Return:
//			number of allocated blocks or number of possible blocks to allocate if newblocks = NULL.
uint32 CPartFile::GetGapsInPart(uint32 dwPartNum, Requested_Block_Struct **ppNewBlocks, uint32 dwCount)
{
	uint32	dwNewBlockCount = 0;

	EMULE_TRY

//	Calculate offsets of the Part
	const uint64 qwPartStart = static_cast<uint64>(dwPartNum) * PARTSIZE;
	const uint64 qwPartEnd = ((qwPartStart + PARTSIZE - 1ui64) >= GetFileSize()) ? (GetFileSize() - 1ui64) : (qwPartStart + PARTSIZE - 1ui64);

	CSingleLock lockGap(&m_csGapListAndPartStatus, TRUE);	// will be unlocked on exit

//	Retrieve Gap(s) matching the part
	for (POSITION pos = gaplist.GetHeadPosition(); pos != NULL;)
	{
		Gap_Struct *pGap = gaplist.GetNext(pos);

	//	Check if Gap is inside the limit
		if ((pGap->qwStartOffset > qwPartEnd) || (pGap->qwEndOffset < qwPartStart))
			continue;

	//	Calculate offsets of the Gap within the part
		uint64 qwGapStart = (pGap->qwStartOffset < qwPartStart) ? qwPartStart : pGap->qwStartOffset;
		const uint64 qwGapEnd = (pGap->qwEndOffset > qwPartEnd) ? qwPartEnd : pGap->qwEndOffset;

	//	Create n request block(s)
		while ((qwGapEnd >= qwGapStart) && (dwNewBlockCount < dwCount))
		{
		//	Calculate offsets of the block
			const uint64 qwStartOffset = qwGapStart;
			const uint64 qwEndOffset = ((qwGapStart + EMBLOCKSIZE - 1ui64) >= qwGapEnd) ? qwGapEnd : ((qwGapStart + EMBLOCKSIZE - 1ui64) - (qwGapStart % EMBLOCKSIZE));

		//	Prepare offset of next block
			qwGapStart = qwEndOffset + 1ui64;

		//	Check if block has already been requested
			if (!IsAlreadyRequested(qwStartOffset, qwEndOffset))
			{
				if (ppNewBlocks != NULL)
				{
				//	Create 1 request block
					Requested_Block_Struct *block = new Requested_Block_Struct;

					block->qwStartOffset = qwStartOffset;
					block->qwEndOffset = qwEndOffset;
					md4cpy(block->m_fileHash, GetFileHash());

				//	Flag the block as 'requested'
					requestedblocks_list.AddTail(block);

				//	Return the block to the source
					ppNewBlocks[dwNewBlockCount] = block;
				}
				dwNewBlockCount++;
			}
		}
	}

	EMULE_CATCH

	return dwNewBlockCount;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::FillGap(uint64 qwStart, uint64 qwEnd)
{
	EMULE_TRY

	POSITION pos1, pos2;

	CSingleLock lockGap(&m_csGapListAndPartStatus, TRUE);

	for (pos1 = gaplist.GetHeadPosition(); (pos2 = pos1) != NULL; )
	{
		Gap_Struct	*pGap = gaplist.GetNext(pos1);

		if ((pGap->qwStartOffset >= qwStart) && (pGap->qwEndOffset <= qwEnd))
		{ // our part fills this gap completely
			gaplist.RemoveAt(pos2);
			m_qwGapsSum -= pGap->qwEndOffset - pGap->qwStartOffset + 1ui64;
			FillGapInParts(pGap->qwStartOffset, pGap->qwEndOffset);
			delete pGap;
		}
		else if ((pGap->qwStartOffset >= qwStart) && (pGap->qwStartOffset <= qwEnd))
		{ // a part of this gap is in the part - set limit
			m_qwGapsSum -= qwEnd - pGap->qwStartOffset + 1ui64;
			FillGapInParts(pGap->qwStartOffset, qwEnd);
			pGap->qwStartOffset = qwEnd + 1ui64;
		}
		else if ((pGap->qwEndOffset <= qwEnd) && (pGap->qwEndOffset >= qwStart))
		{ // a part of this gap is in the part - set limit
			m_qwGapsSum -= pGap->qwEndOffset - qwStart + 1ui64;
			FillGapInParts(qwStart, pGap->qwEndOffset);
			pGap->qwEndOffset = qwStart - 1ui64;
		}
		else if ((qwStart >= pGap->qwStartOffset) && (qwEnd <= pGap->qwEndOffset))
		{
			uint64 qwTmp = pGap->qwEndOffset;

			pGap->qwEndOffset = qwStart - 1;
			pGap = new Gap_Struct;
			pGap->qwStartOffset = qwEnd + 1;
			pGap->qwEndOffset = qwTmp;
			m_qwGapsSum -= qwEnd - qwStart + 1ui64;
			FillGapInParts(qwStart, qwEnd);
			gaplist.InsertAfter(pos1, pGap);
			break;
		}
	}
	lockGap.Unlock();

	UpdateCompletedInfos();
	UpdateDisplayedInfo();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function updates the part status vector based on gap information
// Must be called only under m_csGapListAndPartStatus protection
void CPartFile::FillGapInParts(uint64 qwGapStart, uint64 qwGapEnd)
{
	uint32	dwBegPart = static_cast<uint32>(qwGapStart / PARTSIZE);
	uint32	dwEndPart = static_cast<uint32>(qwGapEnd / PARTSIZE);
	uint64	qwPartStart = static_cast<uint64>(dwBegPart) * PARTSIZE;

	for (uint32 dwPart = dwBegPart; dwPart <= dwEndPart; dwPart++, qwPartStart += PARTSIZE)
	{
		uint64 qwPartEnd = ((qwPartStart + PARTSIZE - 1ui64) >= GetFileSize()) ? (GetFileSize() - 1ui64) : (qwPartStart + PARTSIZE - 1ui64);
		uint32 dwPartFilledSize = GetPartLeftToDLSize(dwPart);

		if (qwGapStart <= qwPartStart)
		{
			if (qwGapEnd >= qwPartEnd)	//	[Gs (Ps Pe) Ge]
				dwPartFilledSize = 0;
			else						//	[Gs (Ps Ge] Pe)
				dwPartFilledSize -= static_cast<uint32>(qwGapEnd - qwPartStart + 1);
		}
		else
		{
			if (qwGapEnd < qwPartEnd)	//	(Ps [Gs Ge] Pe) else (Ps [Gs Pe) Ge]
				qwPartEnd = qwGapEnd;
			dwPartFilledSize -= static_cast<uint32>(qwPartEnd - qwGapStart + 1);
		}

		m_PartsStatusVector[dwPart] = (m_PartsStatusVector[dwPart] & 0xFF000000) | dwPartFilledSize;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This function updates the part status vector based on gap information
// Must be called only under m_csGapListAndPartStatus protection
void CPartFile::AddGapToParts(uint64 qwGapStart, uint64 qwGapEnd)
{
	uint32	dwBegPart = static_cast<uint32>(qwGapStart / PARTSIZE);
	uint32	dwEndPart = static_cast<uint32>(qwGapEnd / PARTSIZE);
	uint64	qwPartStart = static_cast<uint64>(dwBegPart) * PARTSIZE;

	for (uint32 dwPart = dwBegPart; dwPart <= dwEndPart; dwPart++, qwPartStart += PARTSIZE)
	{
		uint64 qwPartEnd = ((qwPartStart + PARTSIZE - 1ui64) >= GetFileSize()) ? (GetFileSize() - 1ui64) : (qwPartStart + PARTSIZE - 1ui64);
		uint32 dwPartFilledSize = GetPartLeftToDLSize(dwPart);

		if (qwGapStart <= qwPartStart)
		{
			if (qwGapEnd >= qwPartEnd)	//	[Gs (Ps Pe) Ge]
				dwPartFilledSize = static_cast<uint32>(qwPartEnd - qwPartStart + 1);
			else						//	[Gs (Ps Ge] Pe)
				dwPartFilledSize += static_cast<uint32>(qwGapEnd - qwPartStart + 1);
		}
		else
		{
			if (qwGapEnd < qwPartEnd)	//	(Ps [Gs Ge] Pe) else (Ps [Gs Pe) Ge]
				qwPartEnd = qwGapEnd;
			dwPartFilledSize += static_cast<uint32>(qwPartEnd - qwGapStart + 1);
		}
	//	Reset PART_VERIFIED status as a gap was added to this part
		m_PartsStatusVector[dwPart] =
			(m_PartsStatusVector[dwPart] & (0xFF000000 & ~PART_VERIFIED)) | dwPartFilledSize;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::UpdateCompletedInfos()
{
	EMULE_TRY

	if ((m_qwGapsSum != 0) || (requestedblocks_list.GetCount() != 0))
	{
		m_dblPercentCompleted = (1.0 - static_cast<double>(m_qwGapsSum) / static_cast<double>(GetFileSize())) * 100;
		m_qwCompletedSize = GetFileSize() - m_qwGapsSum;
	}
	else
	{
		m_dblPercentCompleted = 100.0;
		m_qwCompletedSize = GetFileSize();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::DrawStatusBar(CDC* dc, RECT* rect, bool bFlat)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	const COLORREF crProgress = (bFlat) ? RGB(0, 150, 0) : RGB(0, 192, 0);
	COLORREF crHave, crPending, crMissing;

//	grayPause - Colors by status
	EnumPartFileStatuses	eFileStatus = GetStatus();
	bool			bNotPaused = (eFileStatus == PS_EMPTY) || (eFileStatus == PS_READY);

	if (bNotPaused)
	{
		crHave = (bFlat) ? RGB(0, 0, 0) : RGB(95, 95, 95);
		crPending = RGB(255, 208, 0);
		crMissing = RGB(255, 0, 0);
	}
	else
	{
		crHave = (bFlat) ? RGB(105, 105, 105) : RGB(142, 142, 142);
		crPending = RGB(255, 240, 142);
		crMissing = RGB(255, 142, 142);
	}

	CBarShader s_ChunkBar(rect->bottom - rect->top, rect->right - rect->left, crHave, GetFileSize());

	if (m_eStatus == PS_COMPLETE || m_eStatus == PS_COMPLETING)
	{
		s_ChunkBar.Fill(crProgress);
		s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat);
		m_dblPercentCompleted = 100.0;
		m_qwCompletedSize = GetFileSize();
		return;
	}

//	Red gaps
	CArray<Gap_Struct, Gap_Struct> aGaps;

//	Make a gap list copy to avoid synchronization object allocation for a long time
	GetGapListCopy(&aGaps);

	Gap_Struct	*pGap = aGaps.GetData();
	Gap_Struct	*pGapMax = pGap + aGaps.GetCount();

	for (; pGap < pGapMax; pGap++)
	{
		uint64	qwGapBeg = pGap->qwStartOffset;
		uint64	qwGapEnd = pGap->qwEndOffset + 1ui64;

		for (uint32 i = static_cast<uint32>(qwGapBeg / PARTSIZE); i < GetPartCount(); i++)
		{
			const uint64 qwPartStart = static_cast<uint64>(i) * PARTSIZE;
			const uint64 qwPartEnd = ((qwPartStart + PARTSIZE) > GetFileSize()) ? GetFileSize() : (qwPartStart + PARTSIZE);

			if (qwGapEnd > qwPartEnd)
				qwGapEnd = qwPartEnd;	// The rest is in the next part

			COLORREF	color;
			int			iFreq;

			if (m_srcPartFrequencies.GetCount() >= (INT_PTR)i && ((iFreq = m_srcPartFrequencies[i] - 1) >= 0))
			{
				if (bNotPaused)
				{
					if (IsPartDownloading(i))
						color = crPending;	// Downloading part in yellow
					else
						color = RGB(0, ((210 - 8 * iFreq) < 0) ? 0 : (210 - 8 * iFreq), 255);
				}
				else
					color = RGB(100, ((210 - 8 * iFreq) < 0) ? 0 : (255 - 8 * iFreq), 255);
			}
			else
				color = crMissing;

			s_ChunkBar.FillRange(qwGapBeg, qwGapEnd, color);

			qwGapBeg = qwGapEnd;
			qwGapEnd = pGap->qwEndOffset + 1ui64;
			if (qwGapBeg == qwGapEnd)	// Finished?
				break;
		}
	}

	s_ChunkBar.Draw(dc, rect->left, rect->top, bFlat);

//	Green progress
	RECT gaprect;
	gaprect.top = rect->top;
	gaprect.bottom = gaprect.top + PROGRESS_HEIGHT;
	gaprect.left = rect->left;

	uint32	w = rect->right - rect->left + 1;
	uint32	wp = (uint32)(m_dblPercentCompleted / 100.0 * w + 0.5);

	if (!bFlat)
	{
		CBarShader	s_LoadBar(PROGRESS_HEIGHT, wp, crPending, m_qwCompletedSize);

		s_LoadBar.FillRange(0, m_qwCompletedPartsSize, crProgress);
		s_LoadBar.Draw(dc, gaprect.left, gaprect.top, false);
	}
	else
	{
		uint32		wc = (uint32)(static_cast<double>(m_qwCompletedPartsSize) / static_cast<double>(GetFileSize()) * w + 0.5);
		int			left = rect->left;

	//	Green
		gaprect.right = left + wc;
		CBrush pr_Brush(crProgress);
		dc->FillRect(&gaprect, &pr_Brush);

	//	Yellow
		gaprect.left = left + wc;
		gaprect.right = left + wp;
		CBrush pe_Brush(crPending);
		dc->FillRect(&gaprect, &pe_Brush);

	//	Draw gray progress only if flat
		gaprect.left = left + wp;
		gaprect.right = left + w;
		CBrush rgb_Brush(RGB(224, 224, 224));
		dc->FillRect(&gaprect, &rgb_Brush);
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::WritePartStatus(CFile* file)
{
	EMULE_TRY

	uint32	dwDone = 0, dwParts = GetED2KPartCount();

	file->Write(&dwParts, 2);
	while (dwDone != dwParts)
	{
		byte towrite = 0;
		for (uint32 i = 0; i < 8; i++)
		{
		//	Report that we have "void" part for files with size % PARTSIZE = 0
			if ( ((dwDone == (dwParts - 1)) && (dwParts != GetPartCount())) ||
				(IsPartShared(dwDone) && IsPartComplete(dwDone)) )
				towrite |= (1 << i);

			if (++dwDone == dwParts)
				break;
		}
		file->Write(&towrite, 1);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::WriteCompleteSourcesCount(CFile* file)
{
	uint16 completecount = GetCompleteSourcesCount();
	file->Write(&completecount, 2);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CPartFile::GetValidSourcesCount()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	uint32	dwListsMask = SLM_VALID_SOURCES;
	uint32	dwCounter = 0;

	for (EnumDLQState eDS = DS_DOWNLOADING; eDS < DS_LAST_QUEUED_STATE; ++eDS, dwListsMask >>= 1)
	{
		if ((dwListsMask & 1) != 0)
			dwCounter += m_SourceLists[eDS].size();
	}

	return dwCounter;

	EMULE_CATCH

#endif //NEW_SOCKETS_ENGINE
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CPartFile::GetNotCurrentSourcesCount()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	uint32	dwCounter = 0;

	for (int iDS = 0; iDS < DS_LAST_QUEUED_STATE; iDS++)
	{
		if ((iDS != DS_ONQUEUE) && (iDS != DS_DOWNLOADING) && (iDS != DS_LOWID_ON_OTHER_SERVER))
			dwCounter += m_SourceLists[iDS].size();
	}

	return dwCounter;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EnumPartFileStatuses CPartFile::GetStatus() const
{
	if (!m_bPaused || m_eStatus == PS_ERROR || m_eStatus == PS_COMPLETING || m_eStatus == PS_COMPLETE)
		return m_eStatus;
	else if (m_bStopped)
		return PS_STOPPED;
	else
		return PS_PAUSED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Remove no needed sources from download queue
//	[In] dwNumberSources2Remove: Number of sources to be removed from list
void CPartFile::RemoveNoNeededSources(uint32 dwNumberSources2Remove)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	ClientList	sourceListCopy;
	uint32		dwIdx = 0, dwRemoved;

//	First remove clients with "NNS" status
	GetCopySourceList(DS_NONEEDEDPARTS, &sourceListCopy);
	dwRemoved = (sourceListCopy.size() > dwNumberSources2Remove) ? dwNumberSources2Remove : sourceListCopy.size();

	for (ClientList::const_iterator cIt = sourceListCopy.begin(); dwIdx < dwRemoved; dwIdx++)
		g_App.m_pDownloadQueue->RemoveSource(*cIt++);

	if (dwRemoved < dwNumberSources2Remove)
	{
	//	Remove clients with "Queue Full" status
		GetCopySourceList(DS_ONQUEUE, &sourceListCopy);
		for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
		{
			CUpDownClient	*pSource = *cIt;

		//	The sources will be dropped in 2 cases:
		//	1) the remote client does not have required parts
		//	2) the remote queue is still full & we waited at least 3 reasks to enter into the queue
			if (pSource->IsRemoteQueueFull() && (pSource->GetAskedCountDown() > 3))
			{
				g_App.m_pDownloadQueue->RemoveSource(pSource);
				if (++dwRemoved >= dwNumberSources2Remove)
					break;
			}
		}
	}

	EMULE_CATCH

	m_dwLastPurgeTime = ::GetTickCount();
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CPartFile::Process(uint32 dwReduceDownload /*in percent*/, uint32 dwIteration)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	ClientList		sourceListCopy;
	uint16			nOldTransSourceCount = GetTransferringSrcCount();
	uint16			iOldSrcOnQueueCount = GetOnQueueSrcCount();
	DWORD			dwCurTick = ::GetTickCount();
	CUpDownClient  *pSource = NULL;
	uint32			dwCurrentDataRate = 0;
	uint32			dwLastNoNeededCheckTime = 0;
	uint32			dwDataRate = 0;

//	flush the data to disk if:
//	1. it was requested
//	2. buffer size exceeds limit
//	3. data was not written within time limit
	if ( m_bDataFlushReq
		|| (m_nTotalBufferData > g_App.m_pPrefs->GetFileBufferSize())
		|| (dwCurTick > (m_nLastBufferFlushTime + BUFFER_TIME_LIMIT)) )
	{
	//	Avoid flushing while copying preview file
		if (!m_bPreviewing)
			FlushBuffer();
	}

//	Always process the transferring sources
//	Note: in case of disconnection sources will be put back to queue after download timeout
	if (!m_SourceLists[DS_DOWNLOADING].empty())
	{
		GetCopySourceList(DS_DOWNLOADING, &sourceListCopy);
		for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
		{
			pSource = *cIt;

		// Check if new client has non DL state, then it was removed after creation of the list copy, so ignore it
			if (pSource->GetDownloadState() != DS_DOWNLOADING)
				continue;

			dwCurrentDataRate = pSource->DownloadProcess(dwReduceDownload);

		//	Since client can be dropped due timeout we need to check the state again
			if (pSource->GetDownloadState() != DS_DOWNLOADING)
				continue;
			dwDataRate += dwCurrentDataRate;
		}
	}
	m_dwDataRate = dwDataRate;
	m_uNumTransferringSrcs = static_cast<uint16>(m_SourceLists[DS_DOWNLOADING].size());

//	count & process every 10th time the "connecting" sources
	if (dwIteration == 8)
	{
	//	Count connecting sources
		m_uSrcConnecting = static_cast<uint16>(m_SourceLists[DS_CONNECTING].size());
	//	Process passive sources that are waiting on connection
		if (!m_SourceLists[DS_WAITCALLBACK].empty())
		{
			GetCopySourceList(DS_WAITCALLBACK, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;

#ifdef OLD_SOCKETS_ENABLED
			//	If the request socket still doesn't exist & we waited more than
			//	double connection time (as connection is done through the server)
				if ( (pSource->m_pRequestSocket == NULL) &&
					(dwCurTick - pSource->GetLastAskedTime() > (2 * CONNECTION_TIMEOUT)) )
				{
				//	Remote sources may not answer to callback if we're a NNS for them;
				//	A new source (which was never contacted) has GetClientNeededParts() = 0
					if ( ( (pSource->GetUploadState() == US_ONUPLOADQUEUE) ||
						(pSource->GetClientNeededParts() != 0) ) &&
						((dwCurTick - pSource->GetEnteredConnectedState()) < MAX_PURGEQUEUETIME) )
					{
						pSource->UpdateDownloadStateAfterFileReask();
					}
					else
					{
					//	Remove the source (from download and upload queues) in the following cases:
					//	1) we have nothing to get from it and it is not in our upload queue
					//	in the most cases it happens when we try to initiate a connection to a
					//	new source; if a source is really good it will find us or we'll find it again
					//	2) source became inactive as it stopped contacting us
						delete pSource;
					}
				}
#endif //OLD_SOCKETS_ENABLED
			}
		}	
		m_uSrcConnViaServer = static_cast<uint16>(m_SourceLists[DS_WAITCALLBACK].size());
	}

	if ((g_App.m_pServerConnect == NULL) || !g_App.m_pServerConnect->IsConnected())
		return m_dwDataRate;

//	Process every 10th time the rest of the sources
	if (dwIteration == 1)
	{
		if (!m_SourceLists[DS_CONNECTED].empty())
		{
			GetCopySourceList(DS_CONNECTED, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;

#ifdef OLD_SOCKETS_ENABLED
			//	If the request socket doesn't exist or isn't connected
				if ((pSource->m_pRequestSocket == NULL) || !pSource->m_pRequestSocket->IsConnected())
				{
					pSource->SetDownloadState(DS_ERROR);
					pSource->Disconnected();
				}
				else if (pSource->GetDownloadState() == DS_CONNECTED
					&& (dwCurTick - pSource->GetEnteredConnectedState() > CONNECTION_TIMEOUT + 20000))
				{
					pSource->UpdateDownloadStateAfterFileReask();
				}
#endif //OLD_SOCKETS_ENABLED
			}
		}
		m_uSrcConnected = static_cast<uint16>(m_SourceLists[DS_CONNECTED].size());
	}
	else if (dwIteration == 2)	// process NNS sources
	{
		if (!m_SourceLists[DS_NONEEDEDPARTS].empty())
		{
			CServer	*pCurSrv = g_App.m_pServerConnect->GetCurrentServer();
			uint32	dwLocalSrvIP = (pCurSrv != NULL) ? pCurSrv->GetIP() : 0xFFFFFFFF;

			GetCopySourceList(DS_NONEEDEDPARTS, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;

			//	Process remote LowID sources
				if (pSource->HasLowID() && (pSource->GetServerIP() != dwLocalSrvIP))
				{
					if (pSource->m_pRequestSocket == NULL)
					{
					//	Since we can't connect to a remote LowID client, it can be kept only if it keeps requesting our files
						if ((dwCurTick - pSource->GetEnteredConnectedState()) > MAX_PURGEQUEUETIME)
						{
						//	Remove the source (from download and upload queues) in the following cases:
						//	1) some ed2k clients send UDP reasks when they're LowID, as we can't contact
						//	them in this case, we keep it for a while and remove after some time to avoid leeching
						//	2) source became inactive as it stopped contacting us (we drop from
						//	upload queue here to free room in it faster)
							delete pSource;
						}
					}
				//	If remote LowID is downloading from us, what can be happening for a pretty long time,
				//	it doesn't contact us at that time, so we need to send reask in the same session from
				//	time to time, otherwise we can be purged from the remote upload queue due to inactivity
					else if ((pSource->GetUploadState() == US_UPLOADING) && pSource->m_pRequestSocket->IsConnected())
					{
						if (dwCurTick > pSource->GetNextFileReaskTime())
							pSource->AskForDownload();
					}
					continue;
				}

				if (dwCurTick > pSource->GetNextFileReaskTime())
					pSource->AskForDownload();
			}
		}
		m_uSrcNNP = static_cast<uint16>(m_SourceLists[DS_NONEEDEDPARTS].size());
	}
	else if (dwIteration == 3)	// process the sources on the queue
	{
		if (!m_SourceLists[DS_ONQUEUE].empty())
		{
			uint16	uSrcQueueFull = 0;
			uint16	uSrcHighQR = 0;

			GetCopySourceList(DS_ONQUEUE, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;

				if (pSource->IsRemoteQueueFull())
					uSrcQueueFull++;
				else if (pSource->GetRemoteQueueRank() > QUEUERANK_HIGH)
					uSrcHighQR++;

				if (dwCurTick > pSource->GetNextFileReaskTime())
					pSource->SendReask();
			}
			m_uSrcQueueFull = uSrcQueueFull;
			m_uSrcHighQR = uSrcHighQR;
			m_uSrcOnQueue = static_cast<uint16>(m_SourceLists[DS_ONQUEUE].size());
		}
		else
		{
			m_uSrcOnQueue = m_uSrcQueueFull = m_uSrcHighQR = 0;
		}
	}
	else if (dwIteration == 4)	// process LowID clients on other servers
	{
		if (!m_SourceLists[DS_LOWID_ON_OTHER_SERVER].empty())
		{
			GetCopySourceList(DS_LOWID_ON_OTHER_SERVER, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;

				if (pSource->GetDownloadState() != DS_LOWID_ON_OTHER_SERVER)
					continue;
				if (pSource->m_pRequestSocket == NULL)
				{
				//	Since we can't connect to a remote LowID client, it can be kept only if it keeps requesting our files
					if ((dwCurTick - pSource->GetEnteredConnectedState()) > MAX_PURGEQUEUETIME)
					{
					//	Remove the source (from download and upload queues) in the following cases:
					//	1) some ed2k clients send UDP reasks when they're LowID, as we can't contact
					//	them in this case, we keep it for a while and remove after some time to avoid leeching
					//	2) source became inactive as it stopped contacting us (we drop from
					//	upload queue here to free room in it faster)
						delete pSource;
					}
				}
			//	If remote LowID is downloading from us, what can be happening for a pretty long time,
			//	it doesn't contact us at that time, so we need to send reask in the same session from
			//	time to time, otherwise we can be purged from the remote upload queue due to inactivity
				else if ((pSource->GetUploadState() == US_UPLOADING) && pSource->m_pRequestSocket->IsConnected())
				{
					if (dwCurTick > pSource->GetNextFileReaskTime())
						pSource->AskForDownload();
				}
			}
		}
		m_uSrcLowIDOnOtherServer = static_cast<uint16>(m_SourceLists[DS_LOWID_ON_OTHER_SERVER].size());
	}
	else if (dwIteration == 5)	// process sources which haven't requested file yet
	{
		if (!m_SourceLists[DS_WAIT_FOR_FILE_REQUEST].empty())
		{
			GetCopySourceList(DS_WAIT_FOR_FILE_REQUEST, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
			//	The first request is always TCP request
				(*cIt)->AskForDownload();
			}
		}
		m_uSrcWaitForFileReq = static_cast<uint16>(m_SourceLists[DS_WAIT_FOR_FILE_REQUEST].size());
	}
	else if (dwIteration == 6)	// process LowID to LowID clients
	{
		if (!m_SourceLists[DS_LOWTOLOWID].empty())
		{
			GetCopySourceList(DS_LOWTOLOWID, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;

				if (pSource->GetDownloadState() != DS_LOWTOLOWID)
					continue;
				if (pSource->m_pRequestSocket == NULL)
				{
				//	Since we can't connect to a remote LowID client, it can be kept only if it keeps requesting our files
					if ((dwCurTick - pSource->GetEnteredConnectedState()) > MAX_PURGEQUEUETIME)
					{
					//	Remove the source (from download and upload queues) in the following cases:
					//	1) some ed2k clients send UDP reasks when they're LowID, as we can't contact
					//	them in this case, we keep it for a while and remove after some time to avoid leeching
					//	2) source became inactive as it stopped contacting us (we drop from
					//	upload queue here to free room in it faster)
						delete pSource;
					}
				}
			//	If remote LowID is downloading from us, what can be happening for a pretty long time,
			//	it doesn't contact us at that time, so we need to send reask in the same session from
			//	time to time, otherwise we can be purged from the remote upload queue due to inactivity
				else if ((pSource->GetUploadState() == US_UPLOADING) && pSource->m_pRequestSocket->IsConnected())
				{
					if (dwCurTick > pSource->GetNextFileReaskTime())
						pSource->AskForDownload();
				}
			}
		}
		m_uSrcLowToLow = static_cast<uint16>(m_SourceLists[DS_LOWTOLOWID].size());
	}
	else if (dwIteration == 7)	// manage other sources (A4AF, request from the server)
	{
		uint16	uSourceNumber = GetSourceCount();

	//	A4AF management
	//	Swap sources with NNS status if possible
		BOOL bIsNNS;

		if (((!m_LastNoNeededCheck) || (dwCurTick - m_LastNoNeededCheck) > 30000))
		{
			m_LastNoNeededCheck = dwCurTick;
			GetCopySourceList(DS_NONEEDEDPARTS, &sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;
			//	Check if A4AF Auto file is enable
				if (g_App.m_pDownloadQueue->GetA4AFAutoFile() == NULL)
				//	A4AF auto is disable => switch to any file
					pSource->SwapToAnotherFile(NULL);
				else if (g_App.m_pDownloadQueue->GetA4AFAutoFile() != this)
				{
				//	A4AF auto is enable & isn't current file => switch to A4AF auto file
					if (pSource->m_otherNoNeededMap.IsEmpty())
					{
						pSource->SwapToAnotherFile(g_App.m_pDownloadQueue->GetA4AFAutoFile());
					}
					else
					{
						bIsNNS = pSource->m_otherNoNeededMap.Lookup(g_App.m_pDownloadQueue->GetA4AFAutoFile(), dwLastNoNeededCheckTime);
						if (!bIsNNS || (dwCurTick - dwLastNoNeededCheckTime) > FILEREASKTIME)
						{
							pSource->SwapToAnotherFile(g_App.m_pDownloadQueue->GetA4AFAutoFile());
						}
					}
				}
			}
		}

	//	If this "File" is A4AF auto & A4AF list isn't empty,
	//	then check A4AF list and swap source to "this", if source isn't NNS
		if (g_App.m_pDownloadQueue->GetA4AFAutoFile() == this && !m_A4AFSourceLists.empty())
		{
			GetCopyA4AFSourceList(&sourceListCopy);
			for (ClientList::const_iterator cIt = sourceListCopy.begin(); cIt != sourceListCopy.end(); cIt++)
			{
				pSource = *cIt;
			// allow A4AF switch only for clients with static states 
			// with one exception "DS_WAIT_FOR_FILE_REQUEST" for paused files
				if ( (pSource != NULL) 
					&& (pSource->GetDownloadState() == DS_ONQUEUE 
						|| pSource->GetDownloadState() == DS_NONEEDEDPARTS
						|| pSource->GetDownloadState() == DS_LOWID_ON_OTHER_SERVER
						|| (pSource->GetDownloadState() == DS_WAIT_FOR_FILE_REQUEST
							&& GetStatus() == PS_PAUSED)) )
				{
					if (pSource->m_otherNoNeededMap.IsEmpty())
						pSource->SwapToAnotherFile(this);
					else
					{
						bIsNNS = pSource->m_otherNoNeededMap.Lookup(this, dwLastNoNeededCheckTime);
						if (!bIsNNS || (dwCurTick - dwLastNoNeededCheckTime) > FILEREASKTIME)
						{
							pSource->SwapToAnotherFile(this);
						}
					}
				}
			}
		}
		m_uSrcA4AF = static_cast<uint16>(m_A4AFSourceLists.size());

	//	Check if we want new sources from server
	//	1. connection state will be checked in DL-queue manager, so we don't need to do it here
	//	2. we can call SetRequiredSourcesRefresh() so many time as we want, cause this function
	//	   checks if the file exists.
		if ( ( (!m_dwLastFileSourcesRequestTime) || (dwCurTick - m_dwLastFileSourcesRequestTime) > SERVERREASKTIME)
			&& g_App.m_pPrefs->GetMaxSourcePerFileSoft() > uSourceNumber )
		{
		//	Local server
			m_dwLastFileSourcesRequestTime = dwCurTick;
			g_App.m_pDownloadQueue->SetRequiredSourcesRefresh(this);
		}

	//	Save sources in case there isn't DL (no need to do it so precise, so we can easily skip some ticks)
#ifdef OLD_SOCKETS_ENABLED
		if (g_App.m_pPrefs->SLSEnable() && m_SourceLists[DS_DOWNLOADING].empty() && g_App.m_pServerConnect->IsConnected())
		{
			m_sourcesaver.Process(this, g_App.m_pPrefs->SLSMaxSourcesPerFile());
		}
#endif //OLD_SOCKETS_ENABLED

	//	Calculate data rate, set limit etc. (every 5 sec)
		m_uProcessCounter++;
		if (m_uProcessCounter == 5)
		{
			m_uProcessCounter = 0;
			UpdateCompletedInfos();
			UpdateDisplayedInfo();
		}
	}

//	For status category filters check the status change
//	1. is transfering <-> is not transfering
//	2. is on the queue <-> is not on the queue
	if ( (nOldTransSourceCount == 0 && GetTransferringSrcCount() != 0)
		|| (nOldTransSourceCount != 0 && GetTransferringSrcCount() == 0)
	     || (iOldSrcOnQueueCount == 0 && GetOnQueueSrcCount() != 0)
	     || (iOldSrcOnQueueCount != 0 && GetOnQueueSrcCount() == 0) )
	{
		UpdateGUIAfterStateChange();
	}

	return m_dwDataRate;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::AddServerSources(CMemFile &pSources, uint32 dwSrvIP, uint16 uSrvPort, bool bWithObfuscationAndHash)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

//	If this part file is paused or complete/completing or erroneous, no sources needed any more
	EnumPartFileStatuses	eFileStatus = GetStatus();
	bool			bSkip = ( eFileStatus == PS_STOPPED || eFileStatus == PS_COMPLETE ||
		eFileStatus == PS_COMPLETING || eFileStatus == PS_ERROR );
	CUpDownClient	*pNewSource;
	uint32			dwUserID, dwUserIDHyb;
	uint16			uUserPort;
	uchar			abyteUserHash[16], byteCount, byteCryptOpt;

	pSources.Read(&byteCount, 1);

//	If there is a need for more sources just remove NNS
	uint32	dwSrcCountPlusNew = GetSourceCount() + byteCount;

	if ( (dwSrcCountPlusNew > g_App.m_pPrefs->GetMaxSourcePerFile())
		&& (m_dwLastPurgeTime + PURGE_TIME) < ::GetTickCount() )
	{
		RemoveNoNeededSources(dwSrcCountPlusNew - g_App.m_pPrefs->GetMaxSourcePerFile());
	}

	for (int i = 0; i < static_cast<int>(byteCount); i++)
	{
		pSources.Read(&dwUserID, 4);
		pSources.Read(&uUserPort, 2);

		byteCryptOpt = 0;
		if (bWithObfuscationAndHash)
		{
			pSources.Read(&byteCryptOpt, 1);
			if ((byteCryptOpt & 0x80) != 0)
				pSources.Read(abyteUserHash, 16);
		}

	//	Since we may received multiple search source UDP results we have to "consume" all data of that packet
		if (bSkip)
			continue;

#ifdef OLD_SOCKETS_ENABLED
	//	Don't add the sources if we are not connected to a server
		if (!g_App.m_pServerConnect->IsConnected())
			break;
	//	Check first if we are this source
		if (g_App.m_pServerConnect->IsLowID())
		{
			if ( g_App.m_pServerConnect->GetClientID() == dwUserID &&
				 (g_App.m_pPrefs->GetPort() == uUserPort) &&
				 inet_addr(g_App.m_pServerConnect->GetCurrentServer()->GetFullIP()) == dwSrvIP )
			{
				continue;
			}
		}
		else if ( g_App.m_pServerConnect->GetClientID() == dwUserID &&
				(g_App.m_pPrefs->GetPort() == uUserPort) )
		{
			continue;
		}
		else if ( IsLowID(dwUserID) &&
				  !g_App.m_pServerConnect->IsLocalServer(dwSrvIP, uSrvPort) )
		{
			continue;
		}
#endif //OLD_SOCKETS_ENABLED

		if (g_App.m_pPrefs->GetMaxSourcePerFile() > GetSourceCount())
		{
			dwUserIDHyb = IsLowID(dwUserID) ? dwUserID : ntohl(dwUserID);
			pNewSource = g_App.m_pDownloadQueue->CheckAndAddSource(this, dwUserIDHyb, uUserPort, dwSrvIP, uSrvPort, NULL);
			if (pNewSource != NULL)
				pNewSource->SetUserName(GetResString(IDS_SERVER_SOURCE));
		}
		else
		{	//	Since we may received multiple search source UDP results we have to "consume" all data of that packet
			bSkip = true;
			continue;
		}
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	NewSrcPartsInfo & UpdateAvailablePartsCount & CompleteSourcesCount merged (CPU load)
void CPartFile::NewSrcPartsInfo()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	EnumPartFileStatuses eStatus = GetStatus();

//	Don't update the source related info when sources will be removed
	if ((eStatus == PS_STOPPED) || (eStatus == PS_COMPLETING) || (eStatus == PS_COMPLETE))
		return;

//	Cache part count
	uint32		dwPartCount = GetPartCount();

	CUpDownClient			*pSource;
	uint16					uNewCompleteSrcCount = 0;
	CArray<uint16,uint16>	PartFrequencyArray;
	ClientList				clientListCopy;

	PartFrequencyArray.SetSize(dwPartCount);	//	array is zero initialized inside

	GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		pSource = *cIt;

		uint32		dwSourcePartsCounter = 0;
	//	Count available parts from source
		for (uint32 i = 0; i < dwPartCount; i++)
		{
			if (pSource->IsPartAvailable(i))
			{
				PartFrequencyArray[i]++;
				dwSourcePartsCounter++;
			}
		}
	//	Count full sources
		if (dwSourcePartsCounter == dwPartCount)
			uNewCompleteSrcCount++;
	}

	m_uLastCompleteSrcCount = uNewCompleteSrcCount;

//	Increase size if necessary
	if (static_cast<uint32>(m_srcPartFrequencies.GetSize()) < dwPartCount)
		m_srcPartFrequencies.SetSize(dwPartCount);

	uint32	dwTmp, dwNewMaxSrcPartFrequency = 0;
	uint32	dwNewAvailablePartsCount = 0;
//	Count available parts over all sources
	for (uint32 i = 0; i < dwPartCount; i++)
	{
		dwTmp = static_cast<uint32>(PartFrequencyArray[i]);
		m_srcPartFrequencies[i] = static_cast<uint16>(dwTmp);

		if (dwTmp != 0)
		{
			dwNewAvailablePartsCount++;
			if (dwTmp > dwNewMaxSrcPartFrequency)
				dwNewMaxSrcPartFrequency = dwTmp;
		}
	}

	m_uMaxSrcPartFrequency = static_cast<uint16>(dwNewMaxSrcPartFrequency);
	m_dwAvailablePartsCount = dwNewAvailablePartsCount;

	if (dwNewAvailablePartsCount == dwPartCount)
	{
		lastseencomplete = CTime::GetCurrentTime();
	}

	UpdateDisplayedInfo();

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Intelligent Chunk Request(ICR)
uint16 CPartFile::GetNextRequiredPart(CUpDownClient *pSource)
{
#ifdef OLD_SOCKETS_ENABLED
	EMULE_TRY

	if (pSource == NULL)
		return 0xFFFF;

	uint32	dwGoodPart = pSource->GetLastDownPartAsked();

	if ((dwGoodPart != 0xFFFF) && (GetGapsInPart(dwGoodPart, NULL, 1) != 0))
	{
//		AddLogLine(LOG_FL_DBG, _T("Priority 0 (always choose currently downloaded part until it will be finished). File: %s, Part: %u"), this->GetFileName(), dwGoodPart);
		return static_cast<uint16>(dwGoodPart);
	}

	const uint32		dwPartCnt = GetPartCount();
	std::vector<uint32> PartsFilledSizeVector(dwPartCnt, NOT_AVAILABLE);	// Mark parts as not available to save CPU load
	uint32				dwNumAvailableNeededParts = 0;
	uint32				i, dwNumPartiallyBlockedParts = 0;

//	Scan for requested chunks in order to find out how much they're filled
	for (i = 0; i < dwPartCnt; i++)
	{
		if (pSource->IsPartAvailable(i))
		{
			if ((PartsFilledSizeVector[i] = GetPartLeftToDLSize(i)) != 0)
				dwNumAvailableNeededParts++;
		}
	}
	if (dwNumAvailableNeededParts == 0)
		return 0xFFFF;

//	Scan DownloadList for Blocked parts
	std::vector<byte>	abyteBlockedParts(dwPartCnt, FREE_TO_DL);
	uint32				dwTmp;

	for (POSITION pos = requestedblocks_list.GetHeadPosition(); pos != NULL;)
	{
		Requested_Block_Struct *pReqBlock = requestedblocks_list.GetNext(pos);

		dwTmp = static_cast<uint32>(pReqBlock->qwStartOffset / PARTSIZE);
		if ((PartsFilledSizeVector[dwTmp] == NOT_AVAILABLE)
			|| (abyteBlockedParts[dwTmp] == PARTIALLY_BLOCKED) || (abyteBlockedParts[dwTmp] == FULLY_BLOCKED))
			continue;
	//	Let's check if all gaps in part were already requested.
		if (GetGapsInPart(dwTmp, NULL, 1) == 0)
		{
		//	If we can't get this part from this client, reduce the count
			if (PartsFilledSizeVector[dwTmp] != 0)
				dwNumAvailableNeededParts--;
		//	No gaps found => fully blocked
			abyteBlockedParts[dwTmp] = FULLY_BLOCKED;
		}
		else
		{
		//	Count the blocked parts only in case if they are available & was not counted before
			dwNumPartiallyBlockedParts++;
			abyteBlockedParts[dwTmp] = PARTIALLY_BLOCKED;
		}
	}

//	All arrays/lists self-destruct in their destructors on return
	std::vector<uint32>	adwRareParts(dwPartCnt);	// Priority 2
	uint32				adwMovieParts[3];			// Priority 1 => only first+last parts
	std::vector<uint32>	adwRandomParts(dwPartCnt);	// Priority 4
	const uint32		dwPartCnt1 = dwPartCnt - 1, dwLastPartSize = GetLastPartSize();
	const uint32		dwPreviewMode = GetMovieMode();
//	Mode 0=NoPreview, 1=AVI(1F,1L), 4=MPG(1F), 5=MPG(2F), 7=AVI(1F,2L) F=First,L=Last

//	Define condition of rareness
//	Reduce this threshold for low number of sources to avoid downloading of the same part from several sources
	uint32		dwRarestPart = (m_uMaxSrcPartFrequency > 11) ? 4 : (m_uMaxSrcPartFrequency / 4 + 1);

//	Define a shortest-to-complete condition, cause it will be inverted if all needed parts are downloaded
	const bool	bShortestToComplete = (dwNumAvailableNeededParts > dwNumPartiallyBlockedParts);
	byte		byteMostCompletedRareState = PARTIALLY_BLOCKED;

	uint32		dwMostCompletedRarePartIdx = 0, dwMostCompletedPartIdx = 0;
	uint32		dwMostCompletedRarePartSz = PARTSZ32, dwMoviePartsCnt = 0;
	uint32		dwMostCompletedPartSz = (bShortestToComplete) ? PARTSZ32 : 0;
	uint32		dwRareRank = ~0u, dwRandomRank = ~0u, dwRareVirginCnt = 0;
	uint32		dwRareFilledRank = 0, dwRarePartCnt = 0, dwRandomPartCnt = 0;

//	For each part in the part file...
	for (i = 0; i < dwPartCnt; i++)
	{
	//	If the part is available and it hasn't been completed...
		if (PartsFilledSizeVector[i] != NOT_AVAILABLE && abyteBlockedParts[i] != FULLY_BLOCKED && PartsFilledSizeVector[i] != 0)
		{
		//	------------------------ Rare Parts -------------------------
			if (dwRarestPart >= (dwTmp = m_srcPartFrequencies[i]))
			{
				if (PartsFilledSizeVector[i] == PARTSZ32 || ((i == dwPartCnt1) && PartsFilledSizeVector[i] == dwLastPartSize))
				{
					if (dwTmp <= dwRareRank)	// Select the rarest parts
					{
						if (dwTmp < dwRareRank)
						{
							dwRareRank = dwTmp;
							dwRarePartCnt = 0;
							dwRareVirginCnt = 0;
						}
						adwRareParts[dwRarePartCnt++] = i;
					//	Calculate a number of untouched rare parts (which aren't currently requested)
						if (abyteBlockedParts[i] != PARTIALLY_BLOCKED)
							dwRareVirginCnt++;
					}
				}
			//	There can be situations when several rare sources are available for downloading,
			//	they can have the same rare parts, better to start downloading of different rare parts
			//	than download one from several sources. Besides selecting the shortest part to complete,
			//	we also take into account is this part already requested from another rare source or not
				else if ( ((byteMostCompletedRareState == abyteBlockedParts[i]) && (PartsFilledSizeVector[i] < dwMostCompletedRarePartSz)) ||
					((byteMostCompletedRareState != abyteBlockedParts[i]) && (byteMostCompletedRareState == PARTIALLY_BLOCKED)) )
				{
					byteMostCompletedRareState = abyteBlockedParts[i];
					dwMostCompletedRarePartSz = PartsFilledSizeVector[i];
					dwMostCompletedRarePartIdx = i;
					dwRareFilledRank = dwTmp;
				}
			}
		//	--------------------------- Movie or Archive ---------------------------
			switch (dwPreviewMode)
			{
				case 7:	// AVI(1F,2L) downloading sequence: chunk 0, last, next to last
					if ((i == 0) || ((dwPartCnt > 2) && (i == dwPartCnt - 2)))
						adwMovieParts[dwMoviePartsCnt++] = i;
					if (dwPartCnt > 2 && (i == dwPartCnt - 1))
					{
						if ((dwMoviePartsCnt != 0) && (adwMovieParts[dwMoviePartsCnt - 1] != 0))
						{
							adwMovieParts[dwMoviePartsCnt] = adwMovieParts[dwMoviePartsCnt - 1];
							adwMovieParts[dwMoviePartsCnt - 1] = i;
							dwMoviePartsCnt++;
						}
						else
							adwMovieParts[dwMoviePartsCnt++] = i;
					}
					break;

				case 5:	// MPG(2F) downloading sequence: chunk 0, chunk 1
					if ((i == 0) || ((i == 1) && (dwPartCnt > 1)))
						adwMovieParts[dwMoviePartsCnt++] = i;
					break;

				case 1:	// AVI(1F,1L) downloading sequence: chunk 0, last chunk
					if (dwPartCnt > 1 && (i == dwPartCnt1))
						adwMovieParts[dwMoviePartsCnt++] = i;
				case 4:	// MPG(1F)
					if (i == 0)
						adwMovieParts[dwMoviePartsCnt++] = 0;
			}
		//	--------------------- widespreaded parts ---------------------
		//	Check blocked parts - don't download already downloading parts unless there's something else
			if ((abyteBlockedParts[i] == PARTIALLY_BLOCKED) && bShortestToComplete)
				continue;

			if (PartsFilledSizeVector[i] == PARTSZ32 || ((i == dwPartCnt1) && PartsFilledSizeVector[i] == dwLastPartSize))
			{
			//	Fresh part
				if ((dwTmp = m_srcPartFrequencies[i]) <= dwRandomRank)
				{
					if (dwTmp < dwRandomRank)	// Select the rarest parts among them
					{
						dwRandomRank = dwTmp;
						dwRandomPartCnt = 0;
					}
					adwRandomParts[dwRandomPartCnt++] = i;
				}
			}
			else
			{
			//	Invert condition shortest-to-complete condition if all parts are partially blocked:
			//	1) select the most completed part if there're chunks not in downloading state
			//	2) select the most incomplete part if all available parts from this source're in downloading state
				if (!((PartsFilledSizeVector[i] < dwMostCompletedPartSz) ^ bShortestToComplete))
				{
					dwMostCompletedPartIdx = i;
					dwMostCompletedPartSz = PartsFilledSizeVector[i];
				}
			}
		}
	}

//	Select final result
//	---------------------------------- Movies Part  -----------------------------------------------------
//	The preview parts have highest priority in order to provide the user information about the file as soon as possible
//	This will lead to the faster decision about futher download
	if (dwMoviePartsCnt != 0)
	{
	//	Select the most appropriate part (for the case when all preview parts are already in downloading)
		dwGoodPart = adwMovieParts[0];
	//	Avoid several downloads for the same part if there's a preview part which isn't in downloading state yet
		i = 0;
		do
		{
			dwTmp = adwMovieParts[i];
			if (abyteBlockedParts[dwTmp] != PARTIALLY_BLOCKED)
			{
				dwGoodPart = dwTmp;
				break;
			}
		} while (++i < dwMoviePartsCnt);
//		AddLogLine(LOG_FL_DBG, _T("Priority 1 (Preview part). File: %s, Part: %u"), this->GetFileName(), dwGoodPart);
	}
//	---------------------------------- Rare Parts ------------------------------------------------------
	else if ((dwRarePartCnt != 0) || (dwRareFilledRank != 0))
	{
	//	Download an incomplete part first, but if there're more than one source for an incomplete part
	//	and there's a fresh part with only one source, start downloading of the unique fresh part;
	//	Also if an incomplete part is currently being downloading and there is a chance
	//	to start download of fresh rare part, choose the fresh one
		if ((dwRareFilledRank != 0) && ( (dwRareFilledRank == 1) || (dwRarePartCnt == 0) ||
			((dwRareRank != 1) && (abyteBlockedParts[dwMostCompletedRarePartIdx] != PARTIALLY_BLOCKED)) ) )
		{
			dwGoodPart = dwMostCompletedRarePartIdx;
//			AddLogLine(LOG_FL_DBG, _T("Priority 2 (Rarest unfinished part). File: %s, Part: %u"), this->GetFileName(), dwGoodPart);
		}
		else
		{
		//	If there is only one rare part, then take it, otherwise take random one
			if (dwRarePartCnt == 1)
				dwTmp = 0;
			else
			{
			//	There can be situations when several rare sources are available for downloading,
			//	they can have the same rare parts, better to start downloading of different rare parts
			//	than download one from several sources. Also due to reply delay and data buffering,
			//	a part can be seen as untouched (PartsFilledSizeVector[i] == PARTSIZE), but as
			//	the download request was already sent, this part also should be treated as occupied
				if ((dwRarePartCnt == dwRareVirginCnt) || (dwRareVirginCnt == 0))
					dwTmp = static_cast<uint32>(rand() * (dwRarePartCnt - 1) + RAND_MAX / 2) / RAND_MAX;
				else
				{
					dwTmp = static_cast<uint32>(rand() * (dwRareVirginCnt - 1) + RAND_MAX / 2) / RAND_MAX;
					for (i = 0;; i++)
					{
						if ((abyteBlockedParts[adwRareParts[i]] != PARTIALLY_BLOCKED) && (static_cast<int>(--dwTmp) < 0))
						{
							dwTmp = i;
							break;
						}
					}
				}
			}
			dwGoodPart = adwRareParts[dwTmp];
//			AddLogLine(LOG_FL_DBG, _T("Priority 3 (Rarest unrequested part). File: %s, Part: %u"), this->GetFileName(), dwGoodPart);
		}
	}
//	--------------------------------------- Requested ----------------------------------------------------
//	Try to complete an UnfinishedPart and MostDownloaded as soon as possible to become a new source
//	it doesn't matter which one to download, if there are same filled parts.
	else if ((dwMostCompletedPartSz != PARTSZ32) && (dwMostCompletedPartSz != 0))
	{
		dwGoodPart = dwMostCompletedPartIdx;
//		AddLogLine(LOG_FL_DBG, _T("Priority 4 (Widespread unfinished part). File: %s, Part: %u"), this->GetFileName(), dwGoodPart);
	}
//	-------------------------------- Unrequested & Random -------------------------------------------------
	else if (dwRandomPartCnt != 0)
	{
		if (dwRandomPartCnt == 1)
			dwTmp = 0;
		else
			dwTmp = static_cast<uint32>(rand() * (dwRandomPartCnt - 1) + RAND_MAX / 2) / RAND_MAX;
		dwGoodPart = adwRandomParts[dwTmp];
//		AddLogLine(LOG_FL_DBG, _T("Priority 5 (Widespread unrequested part). File: %s, Part: %u"), this->GetFileName(), dwGoodPart);
	}
	else
	{
		dwGoodPart = 0xFFFF;
	}

/*
	CString strDebug;

	strDebug.Format(_T("ICR-Test: File: %s, A-parts : %u, BP-Parts %u. Choosen Part: %u"),
						GetFileName(), dwNumAvailableNeededParts, dwNumPartiallyBlockedParts, dwGoodPart);
	if (dwGoodPart < dwPartCnt)
	{
		strDebug.AppendFormat(_T(", P-Freq: %u, blocked %s"), m_srcPartFrequencies[dwGoodPart], ((abyteBlockedParts[dwGoodPart] == PARTIALLY_BLOCKED)? _T("yes"):_T("no")));
	}
	AddLogLine(LOG_FL_DBG, strDebug);
*/

	return static_cast<uint16>(dwGoodPart);

	EMULE_CATCH
#endif //OLD_SOCKETS_ENABLED
	return 0xFFFF;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::RemoveBlockFromList(const uint64 &qwStart, const uint64 &qwEnd)
{
	EMULE_TRY

	POSITION pos1, pos2;
	for (pos1 = requestedblocks_list.GetHeadPosition(); (pos2 = pos1) != NULL;)
	{
		Requested_Block_Struct	*pReqBlock = requestedblocks_list.GetNext(pos1);

		if ((pReqBlock->qwStartOffset <= qwStart) && (pReqBlock->qwEndOffset >= qwEnd))
			requestedblocks_list.RemoveAt(pos2);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::RemoveAllRequestedBlocks(void)
{
	EMULE_TRY

	requestedblocks_list.RemoveAll();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::CompleteFile(bool bIsHashingDone)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	StopFile(false);

//	Show completing instead of paused
	SetStatus(PS_COMPLETING);

	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

	UpdateDisplayedInfo();

//	If the part file isn't already hashed...
	if (!bIsHashingDone)
	{
	//	Create a thread to hash the part file in the background
		m_dwDataRate = 0;

		CString	partFileNameBase = m_strPartMetFileName.Left(m_strPartMetFileName.GetLength() - 4);
		CHashFileThread	*addFileThread;

		addFileThread = (CHashFileThread*)AfxBeginThread( RUNTIME_CLASS(CHashFileThread),
		                THREAD_PRIORITY_BELOW_NORMAL + g_App.m_pPrefs->GetMainProcessPriority(),
		                0,
		                CREATE_SUSPENDED );
		addFileThread->SetValues(false, m_strTempDir, partFileNameBase, this);
		addFileThread->ResumeThread();
		return;
	}
	else
	{
		CWinThread *pThread = AfxBeginThread((AFX_THREADPROC)CompleteThreadProc, this, THREAD_PRIORITY_BELOW_NORMAL + g_App.m_pPrefs->GetMainProcessPriority(), 0, 0);

		if (pThread == NULL)
			throw CString(_T("error creating file complete thread"));
	}
	GetAvgDataRate(true);

	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

	UpdateDisplayedInfo();
	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void GenerateNewFileName(const CString& outputDir, CString& newname)
{
	ASSERT(!newname.IsEmpty());

	int namecount = 0;

	TCHAR p_fname[_MAX_FNAME];
	TCHAR p_fext[_MAX_EXT];
	_tsplitpath(newname, NULL, NULL, p_fname, p_fext);

	CString fname(p_fname);
	CString fext(p_fext);

//	Search for matching ()s and check if it contains a number inside
	if (fname.Right(1) == _T(')'))
	{
		int ll = fname.GetLength();
		int ob = fname.ReverseFind(_T('('));
	//	Check for ) existence or () situation
		if (ob != -1 && ob + 2 != ll)
		{
			bool found = true;
			for (int i = ob + 1; i < ll - 1; i++)
			{
			//	Check for digits inside brackets
				if (!_istdigit(fname[i]))
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				namecount = _ttoi(fname.Mid(ob + 1, ll - ob - 2));
				fname.Truncate(ob);
			}
		}
	}

	do
	{
		namecount++;
		newname.Format(_T("%s\\%s(%d)%s"), outputDir, fname, namecount, fext);
	}
	while (PathFileExists(newname));
}

//	Use threads for file completion
BOOL CPartFile::PerformFileComplete(void)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

//	Removes any old fakes.rar that may exist in shared directories before completing the newest fakes.rar
	if (IsFakesDotRar())
	{
		CStringList	strSharedDirsList;
		CString		strFakesFile;

		strFakesFile.Format(_T("%s\\fakes.rar"), g_App.m_pPrefs->GetIncomingDir());
		if (PathFileExists(strFakesFile))
			if (!::DeleteFile(strFakesFile))
				AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Couldn't delete fakes.rar (%s)"), strFakesFile);


		g_App.m_pPrefs->SharedDirListCopy(&strSharedDirsList);

		for (POSITION pos = strSharedDirsList.GetHeadPosition(); pos != NULL; )
		{
			strFakesFile.Format(_T("%s\\fakes.rar"), strSharedDirsList.GetNext(pos));
			if (PathFileExists(strFakesFile))
				if (!::DeleteFile(strFakesFile))
					AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Couldn't delete fakes.rar (%s)"), strFakesFile);
		}
	}

//	Get current category
	EnumCategories eCatID = GetCatID();

	CString strFullPartFileName(m_strFilePath);	// save .part file name for deletion

	CString newname, strOutputDir = GetOutputDir();
	newname.Format(_T("%s\\%s"), strOutputDir, g_App.StripInvalidFilenameChars(GetFileName()));

//	Create all required directories
	CreateAllDirectories(&strOutputDir);

	bool bRenamed = false;
	if (PathFileExists(newname))
	{
		bRenamed = true;
		GenerateNewFileName(strOutputDir, newname);
	}

//	Save full .met file name for temp. file deletions, because a name is altered before it
	CString	strTempFullName = m_strFullName;

//	Make copy if files are on different partitions and a file isn't small (> 1 Mb)
//	Small files will be copied by system routine which blocks uploading start of the same file
	if ( (_totupper(m_strFilePath[0]) != _totupper(newname[0])) &&
		(m_strFilePath[1] == _T(':') && newname[1] == _T(':')) && (GetFileSize() > 1024ui64 * 1024ui64) )
	{
		bool	bException = false;
		CFile	fInputFile, fOutputFile;
		CString	strTempNew = newname;

		strTempNew += _T(".$$$");
		try
		{
			if ((GetFileSize() > 0xFFFFFFFFui64) && IsFileOnFATVolume(newname))
			{
			//	FAT partition doesn't support file > 4 GB, we check it on completion as
			//	destination directory can be changed several times in several places
				CString	strErr;

				strErr.Format(GetResString(IDS_ERR_COMPLETIONFAILED), newname, GetResString(IDS_ERR_TOOLARGEFILE4FS));
				throw strErr;
			}
		//	We going to write everything into the temporary file at the same place the newname located
		//	and after we finished - rename, this will take care of synchronization problem with SharedFiles
			if ( !fInputFile.Open( m_strFilePath, CFile::modeRead | CFile::shareDenyNone | CFile::typeBinary
			                       | CFile::osSequentialScan ) )
			{
			//	Let's catch to care about closing everything
				throw CString(_T("can't open file for reading (") + m_strFilePath + _T(')'));
			}
			if ( !fOutputFile.Open( strTempNew, CFile::modeWrite | CFile::shareExclusive | CFile::osSequentialScan
								   | CFile::modeCreate ) )
			{
			//	Let's catch to care about closing everything
				throw CString(_T("can't open file for writing (") + strTempNew + _T(')'));
			}

			int		iBlockSize = g_App.m_pPrefs->SlowCompleteBlockSize() * 1024;
			char	*pcBuffer = new char[iBlockSize];
			DWORD	dwRead;

		//	Only one extensive hard drive access at a time to reduce I/O load
			CSingleLock Lock(&g_App.m_csPreventExtensiveHDAccess, TRUE);

		//	Most of the work done here
		//	just copy the file data from one to another
			do
			{
				dwRead = fInputFile.Read(pcBuffer, iBlockSize);
				fOutputFile.Write(pcBuffer, dwRead);
			//	Release the CPU for other processes
				Sleep(0);
			}
			while (dwRead > 0);

			Lock.Unlock();
			delete []pcBuffer;

			fInputFile.Close();
			fOutputFile.Close();

		//	Rename temporary file back to normal name
			if (_trename(strTempNew, newname))
			{
			//	Lets catch to care about closing everything
				throw CString(_T("renaming of completed file failed"));
			}

		//	Synchronization with reading upload thread
			CSingleLock lockComplete(&m_csFileCompletion, TRUE);

			SetPath(strOutputDir);
			SetFilePath(newname);
			m_bPaused = false;
			m_eStatus = PS_COMPLETE;
			m_uSrcA4AF = 0;

			lockComplete.Unlock();

		//	Update GUI after synchronization is released
			UpdateGUIAfterStateChange();

		//	Close permanent handles
			try
			{
				ClosePartFile();
			}
			catch (CFileException *error)
			{
				OUTPUT_DEBUG_TRACE();
				AddLogLine(LOG_FL_DBG, _T("Failed to close permanent handle %s - %s"), GetFileName(), GetErrorMessage(error));
				error->Delete();
			}
			::DeleteFile(strFullPartFileName);
		}
	//	Check for exception that will be thrown by "new"
		catch (CMemoryException * error)
		{
			error->Delete();
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_OUTMEM);
			bException = true;
		}
		catch (CString str)
		{
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, str);
			bException = true;
		}
		catch (CException *error)
		{
		//	Sending message for erroneous file
			CString MessageText;
			MessageText.Format(GetResString(IDS_ERR_COMPLETIONFAILED), GetFileName(), GetErrorMessage(error));
			g_App.m_pMDlg->SendMail(MessageText, g_App.m_pPrefs->GetNotifierPopOnImportantError(), g_App.m_pPrefs->IsSMTPWarningEnabled());
			g_App.m_pMDlg->ShowNotifier(MessageText, TBN_IMPORTANTEVENT, false, g_App.m_pPrefs->GetNotifierPopOnImportantError());
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_COMPLETIONFAILED, GetFileName(), GetErrorMessage(error));
			error->Delete();
			bException = true;
		}
		if (bException)
		{
		//	File should be closed to make Initialization possible
			try
			{
				ClosePartFile();
			}
			catch (CFileException *error)
			{
				OUTPUT_DEBUG_TRACE();
				AddLogLine(LOG_FL_DBG, _T("Failed to close permanent handle %s - %s"), GetFileName(), GetErrorMessage(error));
				error->Delete();
			}

			if (fOutputFile.m_hFile != INVALID_HANDLE_VALUE)
				fOutputFile.Close();
		//	Delete temporary file if it still exists
			::DeleteFile(strTempNew);

			m_bPaused = true;
			SetStatus(PS_ERROR);
			AddLogLine(LOG_RGB_ERROR, IDS_ERR_COMPLETIONFAILED2);
		//	If fInputFile is still opened, it will be closed here in its destructor
			return FALSE;
		}
	}
	else
	{
	//	Synchronization with reading upload thread
		CSingleLock lockComplete(&m_csFileCompletion, TRUE);

	//	Close permanent handles
		try
		{
			ClosePartFile();
		}
		catch (CFileException *error)
		{
			OUTPUT_DEBUG_TRACE();
			AddLogLine(LOG_FL_DBG, _T("Failed to close permanent handle %s - %s"), GetFileName(), GetErrorMessage(error));
			error->Delete();
		}

		if (_trename(m_strFilePath, newname))
		{
			lockComplete.Unlock();

		//	Sending message for erroneous file
			CString MessageText;
			MessageText.Format(GetResString(IDS_ERR_COMPLETIONFAILED), GetFileName(), _tcserror(errno));
			g_App.m_pMDlg->SendMail(MessageText, g_App.m_pPrefs->GetNotifierPopOnImportantError(), g_App.m_pPrefs->IsSMTPWarningEnabled());
			g_App.m_pMDlg->ShowNotifier(MessageText, TBN_IMPORTANTEVENT, false, g_App.m_pPrefs->GetNotifierPopOnImportantError());
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_COMPLETIONFAILED, GetFileName(), _tcserror(errno));
			AddLogLine(LOG_RGB_ERROR, IDS_ERR_COMPLETIONFAILED2);
			m_bPaused = true;
			SetStatus(PS_ERROR);
			return FALSE;
		}

		SetPath(strOutputDir);
		SetFilePath(newname);
		m_bPaused = false;
		m_eStatus = PS_COMPLETE;
		m_uSrcA4AF = 0;

		lockComplete.Unlock();

	//	Update GUI after synchronization is released
		UpdateGUIAfterStateChange();
	}

//	To have the accurate date stored in known.met we have to update the 'date' of a just completed file.
//	If we don't update the file date here (after committing the file and before adding the record to known.met),
//	that file will be rehashed at next startup and there would also be a duplicate entry (hash+size) in known.met because of different file date!
	GetAdjustedFileTime(newname, &m_timetLastWriteDate);

	if (_tremove(strTempFullName))
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strTempFullName);

	CString	strFileToRemove;
#if 1	//code left for smooth migration, delete in v1.2f

//	Remove .dir file
	strFileToRemove = strFullPartFileName + _T(".dir");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);
#endif

//	Remove .bak file
	strFileToRemove = strTempFullName + _T(".bak");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove .bad file
	strFileToRemove = strTempFullName + _T(".bad");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove .settings file
	strFileToRemove = strFullPartFileName + _T(".settings");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove .txtsrc file
	strFileToRemove = strFullPartFileName + _T(".txtsrc");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove the stats file
	strFileToRemove = strFullPartFileName + _T(".stats");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Change AutoPriority to NORMAL when UAP is disabled
	if (IsULAutoPrioritized() && !g_App.m_pPrefs->IsUAPEnabled())
	{
		SetAutoULPriority(false);
		SetULPriority(PR_NORMAL);
	}

	UpdateDisplayedInfo();

	CCat		*pCat = CCat::GetCatByID(GetCatID());
	CString		strCategory = (pCat != NULL) ? pCat->GetTitle() : GetResString(IDS_CAT_UNCATEGORIZED);

	AddLogLine(LOG_FL_SBAR | LOG_RGB_SUCCESS, IDS_DOWNLOADDONE, GetFileName(), strCategory);

//	Sending message for completed file
	CString MessageText;
	MessageText.Format(GetResString(IDS_TBN_DOWNLOADDONE), GetFileName(), strCategory);
	g_App.m_pMDlg->SendMail(MessageText, g_App.m_pPrefs->GetUseDownloadNotifier(), g_App.m_pPrefs->IsSMTPInfoEnabled());
	g_App.m_pMDlg->ShowNotifier(MessageText, TBN_DLOAD, false, g_App.m_pPrefs->GetUseDownloadNotifier());
	if (bRenamed)
		AddLogLine(LOG_FL_SBAR, IDS_DOWNLOADRENAMED, _tcsrchr(newname, _T('\\')) + 1);
	g_App.m_pKnownFilesList->SafeAddKnownFile(this);
	g_App.m_pDownloadQueue->RemoveFile(this);
	UpdateDisplayedInfo();

#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pMMServer->AddFinishedFile(this);
#endif //OLD_SOCKETS_ENABLED

//	Starts next paused file
	if (g_App.m_pPrefs->DownloadPausedOnComplete() && !IsFakesDotRar())
	{
		g_App.m_pDownloadQueue->StartNextFile(eCatID);
	}

	g_App.m_pPrefs->Add2DownCompletedFiles();			// Increments cumDownCompletedFiles in prefs struct
	g_App.m_pPrefs->Add2DownSessionCompletedFiles(); 	// Increments sesDownCompletedFiles in prefs struct
	g_App.m_pPrefs->SaveCompletedDownloadsStat();		// Saves cumDownCompletedFiles to INI

//	Republish a file on the server to update server complete sources counter
	g_App.m_pSharedFilesList->RepublishFile(this, SRV_TCPFLG_COMPRESSION/*check feature support*/);

	if (GetPath() != g_App.m_pPrefs->GetIncomingDir())
	{
		if (g_App.m_pPrefs->SharedDirListCheckAndAdd(GetPath(), true))
		{
		//	New path was added to the list, scan for files
			g_App.m_pSharedFilesList->Reload();
		}
	}

	if (g_App.m_pPrefs->IsAutoClearCompleted())
	{
		g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.SetRedraw(false);
		g_App.m_pDownloadList->ClearCompleted(CAT_ALL);
		g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.SetRedraw(true);
	}

	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

//	Scan completed files for viruses
	if (g_App.m_pPrefs->IsAVEnabled() &&
		g_App.m_pPrefs->IsAVScanCompleted() && !g_App.m_pPrefs->GetAVPath().IsEmpty())
	{
		strTempFullName.Format(_T("%s \"%s\""), g_App.m_pPrefs->GetAVParams(), m_strFilePath);
		ShellExecute(NULL, _T("open"), g_App.m_pPrefs->GetAVPath(), strTempFullName, NULL, SW_SHOW);
	}

//	FakeCheck
	if (IsFakesDotRar())
	{
		CFakeCheck	*pFakeCheck = g_App.m_pFakeCheck;
		CString		strFakesDotDatPath = g_App.m_pPrefs->GetConfigDir() + _T("fakes.dat");
		CString		strBackupFilePath(strFakesDotDatPath);

		strBackupFilePath += _T(".bak");

	//	Backups fakes.dat
		::MoveFile(strFakesDotDatPath, strBackupFilePath);

		if (pFakeCheck->ExtractRARArchive(m_strFilePath, g_App.m_pPrefs->GetConfigDir()))
		{
			g_App.AddLogLine(LOG_FL_SBAR | LOG_RGB_SUCCESS, IDS_FAKE_SUCCESS_UPDATE, g_App.m_pPrefs->GetFakesDatVersion(), g_App.m_pPrefs->GetDLingFakeListVersion());
			g_App.m_pPrefs->SetFakesDatVersion(g_App.m_pPrefs->GetDLingFakeListVersion());
			g_App.m_pPrefs->SetDLingFakeListVersion(0);
			g_App.m_pPrefs->SetDLingFakeListLink(_T(""));
			pFakeCheck->LoadFromDatFile();
			::DeleteFile(strBackupFilePath);
		}
		else
		{
		//	The extraction was unsuccessful, so we restore fakes.dat
			::DeleteFile(strFakesDotDatPath);
			::MoveFile(strBackupFilePath, strFakesDotDatPath);
		}

		CKnownFile	*pThisFile = g_App.m_pSharedFilesList->GetFileByID(GetFileHash());

		if (pThisFile != NULL)
		{
			pThisFile->SetAutoULPriority(false);
			pThisFile->SetULPriority(PR_RELEASE);
			g_App.m_pSharedFilesList->Reload();
		}
	}

	return TRUE;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::RemoveAllSources()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	CUpDownClient	*pRemoveSrc;
	ClientList		clientListCopy;

	GetCopySourceLists(SLM_ALL, &clientListCopy, true);

	m_uMaxSrcPartFrequency = 0;
	m_dwAvailablePartsCount = 0;
	for (uint32 i = 0; i < GetPartCount(); i++)
		m_srcPartFrequencies[i] = 0;

	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		pRemoveSrc = *cIt;

	//	Transfer sources to other downloading files if possible
		if (!pRemoveSrc->SwapToAnotherFile(NULL, A4AF_REMOVE))
		{
		//	If the switching wasn't successful we remove the source from the download queue
			if ((pRemoveSrc->GetUploadState() == US_NONE) && (pRemoveSrc->GetChatState() == MS_NONE))
				delete pRemoveSrc;
			else
				g_App.m_pDownloadQueue->RemoveSource(pRemoveSrc);
		}
		else
		{
		//	If the switching was successful we need to remove the entry from graphical list
			g_App.m_pDownloadList->RemoveSource(pRemoveSrc, this);
		}
	}

//	Remove A4AF entries from graphical list & anotherRequestList
	if (!m_A4AFSourceLists.empty())
	{
		GetCopyA4AFSourceList(&clientListCopy, true);
		for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
		{
			pRemoveSrc = *cIt;
			g_App.m_pDownloadList->RemoveSource(pRemoveSrc, this);
			pRemoveSrc->RemoveRequestForAnotherFile(this);
		}
	}

	m_uNumTransferringSrcs = 0;
	m_uSrcNNP = 0;
	m_uSrcOnQueue = 0;
	m_uSrcHighQR = 0;
	m_uSrcConnecting = 0;
	m_uSrcWaitForFileReq = 0;
	m_uSrcConnected = 0;
	m_uSrcConnViaServer = 0;
	m_uSrcLowToLow = 0;
	m_uSrcLowIDOnOtherServer = 0;
	m_uSrcQueueFull = 0;
	m_uSrcA4AF = 0;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::DeleteFile()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	ASSERT (!m_bPreviewing);

	CString	strFileToRemove;

	GetResString(&strFileToRemove, IDS_CANCELLED);
	strFileToRemove += _T(": %s");
	AddLogLine(0, strFileToRemove, GetFileName());

	m_bIsBeingDeleted = true;

	StopFile();

	ClosePartFile();

	if (_tremove(m_strFullName))
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, m_strFullName);

	if (_tremove(m_strFilePath))
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, m_strFilePath);

	CString	strFilePath = RemoveFileExtension(m_strFullName);
#if 1	//code left for smooth migration, delete in v1.2f

//	Remove .dir file
	strFileToRemove = strFilePath + _T(".dir");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);
#endif

//	Remove .bak file
	strFileToRemove = m_strFullName + _T(".bak");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove .bad file
	strFileToRemove = m_strFullName + _T(".bad");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove .settings file
	strFileToRemove = strFilePath + _T(".settings");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove .txtsrc file
	strFileToRemove = strFilePath + _T(".txtsrc");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	Remove the stats file
	strFileToRemove = strFilePath + _T(".stats");
	if (_tremove(strFileToRemove) && errno != ENOENT)
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_DELETEFAILED, strFileToRemove);

//	File properties get deleted inside one of these calls now...
	g_App.m_pSharedFilesList->RemoveFile(this);
	g_App.m_pDownloadQueue->RemoveFile(this);
	g_App.m_pDownloadList->RemoveFile(this);

	delete this;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::HashSinglePart(uint32 dwPartNum)
{
#ifndef NEW_SOCKETS_ENGINE
	CFile	*pFile = NULL;

	EMULE_TRY

	UINT dwResErrStrID;
	uint32 dwPartCnt = static_cast<uint32>(GetPartCount());

	if ((GetHashCount() <= dwPartNum) && (dwPartCnt > 1))
		dwResErrStrID = IDS_ERR_HASHERRORWARNING;
	else if ((dwPartCnt != 1) && (GetPartHash(dwPartNum) == NULL))
		dwResErrStrID = IDS_ERR_INCOMPLETEHASH;
	else
	{
		uchar	hashresult[16];

	//	Duplicate part file read handle to eliminate read collisions
		pFile = m_hPartFileRead.Duplicate();
		pFile->Seek(static_cast<uint64>(dwPartNum) * PARTSIZE, CFile::begin);

		CreateHashFromFile(pFile, GetPartSize(dwPartNum), hashresult);

	//	File handle is closed in destructor
		delete pFile;
		pFile = NULL;

		return (md4cmp(hashresult, (GetFileSize() >= PARTSIZE) ? GetPartHash(dwPartNum) : m_fileHash) == 0) ? true : false;
	}

	AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, dwResErrStrID, GetFileName());
	this->m_bHashSetNeeded = true;
	return true;

	EMULE_CATCH

	delete pFile;
#endif //NEW_SOCKETS_ENGINE
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::IsArchive() const
{
	return (ED2KFT_ARCHIVE == GetFileType());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::SetPriority(byte byteNewPriority, bool bSaveSettings /*= true*/)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	priority = byteNewPriority;
	g_App.m_pDownloadQueue->SortByPriority();
	UpdateDisplayedInfo();
	if (bSaveSettings)
		SavePartFile();

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::StopFile(bool bUpdateDisplay /* TRUE */)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (m_eStatus == PS_COMPLETE || m_eStatus == PS_COMPLETING)
		return;

	ClientList	DownloadingListCopy;

//	Change status as soon as possible in order to prevent
//	unpredictable behaviour	during the source removal
	m_bPaused = true;
	m_bStopped = true;

//	Need to tell any connected clients to stop sending the file
//	and send downloading clients a cancel (NOT if completing)
	GetCopySourceList(DS_DOWNLOADING, &DownloadingListCopy, true);
	for (ClientList::const_iterator cIt = DownloadingListCopy.begin(); cIt != DownloadingListCopy.end(); cIt++)
	{
		CUpDownClient	*pSource = *cIt;

#ifdef OLD_SOCKETS_ENABLED
		pSource->SendCancelTransfer();
		pSource->SetDownloadState(DS_ONQUEUE);
#endif //OLD_SOCKETS_ENABLED
	}

//	Reset A4AF auto file
	if (g_App.m_pDownloadQueue->GetA4AFAutoFile() == this)
	{
		g_App.m_pDownloadQueue->SetA4AFAutoFile(NULL);
	}

	RemoveAllSources();

	m_dwDataRate = 0;

//	Save info to disk
	if (!m_bIsBeingDeleted)
	{
		FlushBuffer();
		SavePartFile();
		SaveSettingsFile();
	}

//	Update GUI information (if the file was stopped for completing GUI will be updated in CompleteFile)
	if (bUpdateDisplay)
	{
		UpdateDisplayedInfo();
		g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::PauseFile()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if ((m_eStatus == PS_COMPLETE) || (m_eStatus == PS_COMPLETING))
		return;

	CUpDownClient	*pSource;
	ClientList		clientListCopy;

//	Reset A4AF auto file
	if (g_App.m_pDownloadQueue->GetA4AFAutoFile() == this)
	{
		g_App.m_pDownloadQueue->SetA4AFAutoFile(NULL);
	}

//	Send CANCEL to downloading clients
	GetCopySourceList(DS_DOWNLOADING, &clientListCopy, true);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		pSource = *cIt;
		pSource->SendCancelTransfer();
		pSource->SetDownloadState(DS_ONQUEUE);
	}
//	At this point nobody is downloading so we can try to swap sources
	GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		pSource = *cIt;
		pSource->SwapToAnotherFile(NULL, A4AF_IGNORE_TIME_LIMIT);
	}

	m_bPaused = true;
	m_bStopped = false;

	UpdateGUIAfterStateChange();

	m_dwDataRate = 0;
	m_uNumTransferringSrcs = 0;
	UpdateDisplayedInfo();

	if (!m_bIsBeingDeleted)
	{
		SavePartFile();
		SaveSettingsFile();
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::ResumeFile()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (m_eStatus == PS_COMPLETE || m_eStatus == PS_COMPLETING)
		return;

//	Check if chunks are availible & set status
	EnumPartFileStatuses	eNewStatus = PS_EMPTY;

	for (uint32 ui = 0; ui < GetPartCount(); ui++)
	{
		if (IsPartComplete(ui))
		{
			eNewStatus = PS_READY;
			break;
		}
	}
	SetStatus(eNewStatus);

	if (GetStartTimeReset())
	{
	//	Needed for files which are paused after adding or loading
		m_SessionStartTime = CTime::GetCurrentTime();
		SetStartTimeReset(false);
	}

	m_bPaused = false;
	m_bStopped = false;
	m_dwLastFileSourcesRequestTime = 0;
	SavePartFile();
	SaveSettingsFile();
	UpdateDisplayedInfo();

	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Used to load initial sources from Server right after connect
void CPartFile::GetSourcesAfterServerConnect()
{
	EMULE_TRY

	if (m_dwLastFileSourcesRequestTime > 0)
	{
		m_dwLastFileSourcesRequestTime = 0;
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CPartFile::GetPartFileStatus()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	UINT		dwResStrId;

	if (GetTransferringSrcCount() > 0)
		dwResStrId = IDS_DOWNLOADING;
	else
		dwResStrId = (!IsStalled()) ? IDS_WAITING : IDS_STALLED;

	switch (GetStatus())
	{
		case PS_HASHING:
		case PS_WAITINGFORHASH:
			dwResStrId = IDS_HASHING;
			break;
		case PS_COMPLETING:
			dwResStrId = IDS_COMPLETING;
			break;
		case PS_COMPLETE:
			dwResStrId = IDS_COMPLETE;
			break;
		case PS_PAUSED:
			dwResStrId = IDS_PAUSED;
			break;
		case PS_ERROR:
			dwResStrId = IDS_ERRORLIKE;
			break;
		case PS_STOPPED:
			dwResStrId = IDS_STOPPED;
			break;
	}
	return GetResString(dwResStrId);

	EMULE_CATCH

#endif //NEW_SOCKETS_ENGINE
	return _T("");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPartFile::GetPartFileStatusID() const
{
	EMULE_TRY

	int	iFileStatus, iStatus = 0;

	if (GetTransferringSrcCount() > 0)
		iStatus = PS_DOWNLOADING;
	else
		iStatus = (!IsStalled()) ? PS_WAITINGFORSOURCE : PS_STALLED;

	iFileStatus = GetStatus();
	switch (iFileStatus)
	{
		case PS_HASHING:
		case PS_WAITINGFORHASH:
		case PS_COMPLETING:
		case PS_COMPLETE:
		case PS_PAUSED:
		case PS_ERROR:
		case PS_STOPPED:
			iStatus = iFileStatus;
			break;
	}
	return iStatus;

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CPartFile::GetPartFileStatusRang()
{
	EMULE_TRY

//	Sort order is Complete - Completing - Downloading - Waiting - Stalled - Paused - Hashing - Stopped - Errors
	int status = 2;
	if (GetTransferringSrcCount() == 0)
		status = (!IsStalled()) ? 3 : 4;

	switch (GetStatus())
	{
		case PS_HASHING:
		case PS_WAITINGFORHASH:
			status = 6;
			break;
		case PS_COMPLETING:
			status = 1;
			break;
		case PS_COMPLETE:
			status = 0;
			break;
		case PS_PAUSED:
			status = 5;
			break;
		case PS_STOPPED:
			status = 7;
			break;
		case PS_ERROR:
			status = 8;
			break;
	}
	return status;

	EMULE_CATCH

	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
sint32 CPartFile::GetTimeRemaining(bool bAverage /*=false*/)
{
	uint32	dwDataRate = (bAverage) ? GetAvgDataRate() : GetDataRate();
	sint32	iTimeRemaining;

	iTimeRemaining = (dwDataRate != 0) ? static_cast<sint32>((GetFileSize() - GetCompletedSize()) / static_cast<uint64>(dwDataRate)) : -1;
	return ((iTimeRemaining > 0 && iTimeRemaining <= 8640000 /*100 days*/) ? iTimeRemaining : -1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetAvgDataRate() recalculates the average bytes/second for the session. It stores the
//	average in 'm_AvgDataRate' as well as returning it.
uint32 CPartFile::GetAvgDataRate(bool bUpdated /*=false*/)
{
	EMULE_TRY

	if (bUpdated)
	{
		CTimeSpan	timeSpan;

	//	Calc on last FlushBuffer() to keep sessionstats for completed files
		if ((m_eStatus == PS_COMPLETE) || (m_eStatus == PS_COMPLETING))
			timeSpan = GetFlushTimeSpan();
		else
		{
		//	TODO: Modify to use time since first data
			timeSpan = GetSessionTimeSpan();
		}

	//	Delay 1st calculation a bit & dont use datarate for balancing (peak-stopper?)
		uint32	dwSecs = static_cast<uint32>(timeSpan.GetTotalSeconds());

		if (dwSecs > 10)
			m_AvgDataRate = static_cast<uint32>(GetSessionTransferred() / static_cast<uint64>(dwSecs));
		else
			m_AvgDataRate = 0;
	}

	EMULE_CATCH
	return m_AvgDataRate;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::PreviewFile()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (IsArchivePreviewable())
	{
		if ((!m_bRecoveringArchive) && (!m_bPreviewing))
			CArchiveRecovery::recover(this);
		return;
	}

	if (!PreviewAvailable())
		return;

	if (g_App.m_pPrefs->BackupPreview())
	{
		m_bPreviewing = true;
		CPreviewThread* pThread = (CPreviewThread*) AfxBeginThread(RUNTIME_CLASS(CPreviewThread), THREAD_PRIORITY_NORMAL + g_App.m_pPrefs->GetMainProcessPriority(), 0, CREATE_SUSPENDED);
		pThread->SetValues(this, g_App.m_pPrefs->GetVideoPlayer(), g_App.m_pPrefs->GetVideoPlayerArgs());
		pThread->ResumeThread();
	}
	else
	{
		CString strLine(m_strFilePath);
		CString strPlayer(g_App.m_pPrefs->GetVideoPlayer());

	//	If path has spaces use quotes
		if (strLine.Find(_T(' ')) >= 0)
			strLine = _T('\"') + strLine + _T('\"');

		if (strPlayer.IsEmpty())
			ShellExecute(NULL, NULL, strLine, NULL, NULL, SW_SHOWNORMAL);
		else
		{
			CString strPlayerPath, strArgs(g_App.m_pPrefs->GetVideoPlayerArgs());
			int i = strPlayer.ReverseFind('\\');

			if (i >= 0)
				strPlayerPath = strPlayer.Left(i + 1);
			if (!strArgs.IsEmpty())
				strArgs += _T(' ');
			strArgs += strLine;
			ShellExecute(NULL, _T("open"), strPlayer, strArgs, strPlayerPath, SW_SHOWNORMAL);
		}
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::PreviewAvailable()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	EnumPartFileStatuses	eFileStatus = GetStatus();

// 	No preview while completing or hashing or when file is already complete
	if (eFileStatus == PS_COMPLETING || eFileStatus == PS_COMPLETE || eFileStatus == PS_HASHING)
		return false;

//	Allow preview of archives of any length > 1k
	if (IsArchivePreviewable())
	{
	//	Preview creates two files: a file copy of complete data (file size is up to
	//	the most distant received data block) and reconstructed archive. So the required
	//	free space is the current file size (equal to the most distant data block except
	//	for preallocated files) plus total complete data size (the size of reconstructed
	//	archive will be close or less than that)
	//	plus some extra just in case (also not to fill the whole partition)
		return ( (GetFileSize() > 1024ui64) && (GetCompletedSize() > 1024ui64) && !m_bRecoveringArchive
			&& (GetFreeDiskSpaceX(m_strTempDir) > (m_hPartFileRead.GetLength() + GetCompletedSize() + 20ui64 * 1024ui64 * 1024ui64)) );
	}

	byte	byteMovieMode = GetMovieMode();

	if (byteMovieMode == 0)
	{
	//	Check only for AVI style movie, as below first chunk check is enough for all MPG-style ones
		if (IsAviKindMovie())
			byteMovieMode = (GetLastPartSize() < static_cast<uint32>(GetFileSize() / 100ui64)) ? 7 : 1;
	}

	if (g_App.m_pPrefs->BackupPreview())
	{
		return ( ((eFileStatus == PS_READY) || IsPaused())
				&& !m_bPreviewing && (GetPartCount() > 2) && IsMovie() && IsPartComplete(0)
				&& (((byteMovieMode != 1) && (byteMovieMode != 7)) || IsPartComplete(GetPartCount() - 1))
				&& ((byteMovieMode != 7) || IsPartComplete(GetPartCount() - 2))
				&& (GetFreeDiskSpaceX(m_strTempDir) >= (GetFileSize() + 50ui64 * 1024ui64 * 1024ui64)) );
	}
	else
	{
		TCHAR szVideoPlayerFileName[_MAX_FNAME];
		_tsplitpath(g_App.m_pPrefs->GetVideoPlayer(), NULL, NULL, szVideoPlayerFileName, NULL);

	//	Enable the preview command if the according option is specified 'PreviewSmallBlocks'
	//	or if VideoLAN client is specified
		if (g_App.m_pPrefs->GetPreviewSmallBlocks() || !_tcsicmp(szVideoPlayerFileName, _T("vlc")))
		{
			if (m_bPreviewing)
				return false;

			if (!(eFileStatus == PS_READY || eFileStatus == PS_EMPTY || IsPaused()))
				return false;

			EED2KFileType	eFileType = GetFileType();

			if (eFileType != ED2KFT_VIDEO && eFileType != ED2KFT_AUDIO && eFileType != ED2KFT_CDIMAGE)
				return false;

		//	If it's an MPEG file, VLC is even capable of showing parts of the file if the beginning of the file is missing!
			if (IsMpgMovie() || IsMpgAudio())
			{
			//	TODO: search a block which is at least 16K (Audio) or 256K (Video)
				if (GetCompletedSize() < 16ui64 * 1024ui64)
					return false;
			}
			else
			{
			//	For AVI files it depends on the used codec..
				if (!IsComplete(0, 256 * 1024))
					return false;
			}

			return true;
		}
		else
		{
			return ( ((eFileStatus == PS_READY) || IsPaused())
					&& !m_bPreviewing && (GetPartCount() > 2) && IsMovie() && IsPartComplete(0)
					&& (((byteMovieMode != 1) && (byteMovieMode != 7)) || IsPartComplete(GetPartCount() - 1))
					&& ((byteMovieMode != 7) || IsPartComplete(GetPartCount() - 2)) );
		}
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::AllowGet1stLast()
{
	if (!(IsMovie() || IsArchivePreviewable()) || (GetPartCount() < 3))
		return false;

	byte	byteMovieMode = GetMovieMode();

	if (byteMovieMode == 0)
	{
	//	Check only for AVI style movie, as below first chunk check is enough for all MPG-style ones
		if (IsAviKindMovie())
			byteMovieMode = (GetLastPartSize() < static_cast<uint32>(GetFileSize() / 100ui64)) ? 7 : 1;
	}
	return !( IsPartComplete(0)
				&& (((byteMovieMode != 1) && (byteMovieMode != 7)) || IsPartComplete(GetPartCount() - 1))
				&& ((byteMovieMode != 7) || IsPartComplete(GetPartCount() - 2)) );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet* CPartFile::CreateSrcInfoPacket(const CUpDownClient *pForClient, byte byteRequestedVer, uint16 uRequestedOpt)
{
	NOPRM(uRequestedOpt);	// We don't support any special SX2 options yet, reserved for later use

#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (pForClient->m_pReqPartFile != this)
		return NULL;

	if (GetSourceCount() == 0)
		return NULL;

	CMemFile	packetStream(1024);
	uint32		dwCount = 0;
	byte		byteUsedVer;
	bool		bNeededPart, bIsSX2Packet;

	if (pForClient->SupportsSourceExchange2() && (byteRequestedVer != 0))
	{
	//	Highest vesion supported by both clients
		byteUsedVer = min(byteRequestedVer, SOURCEEXCHANGE2_VERSION);
		packetStream.Write(&byteUsedVer, 1);
		bIsSX2Packet = true;
	}
	else
	{
		byteUsedVer = pForClient->GetSourceExchange1Version();
		bIsSX2Packet = false;
	}

	packetStream.Write(m_fileHash, 16);
	packetStream.Write(&dwCount, 2);

	CUpDownClient  *pPotentialSource;
	const byte		*pbyteForClientPartStatus, *pbytePotentialClientPartStatus;
	uint32			dwID, dwSrvIP, dwRes;
	uint16			uPort, uSrvPort;
	ClientList		clientListCopy;
	const uint16	uPartCount = GetPartCount();

	GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		pPotentialSource = *cIt;

	//	Don't send a client to itself as a source (part 1)
		if (pForClient == pPotentialSource)
			continue;
	//	Don't send a client to itself as a source (part 2) or another client with same hash
		if ((dwRes = pForClient->Compare(pPotentialSource)) != 0)
			continue;
	//	Don't send a client using a hash that was identified for another client because he could be a hash stealer
		if (pPotentialSource->m_pCredits != NULL
			&& pPotentialSource->m_pCredits->GetCurrentIdentState(pPotentialSource->GetIP()) == IS_IDBADGUY)
			continue;
	//	Check LowID status
		if (pPotentialSource->HasLowID())
		{
		//	Don't send a LowID client to LowID as they will not able to connect
			if (pForClient->HasLowID())
				continue;
		//	Don't send a LowID client if server IP & server port are unknown
			if (pPotentialSource->GetServerIP() == 0 || pPotentialSource->GetServerPort() == 0)
				continue;
		//	Don't send a LowID client to HighID if client on different server
			if ( pPotentialSource->GetServerIP() != pForClient->GetServerIP()
				|| pPotentialSource->GetServerPort() != pForClient->GetServerPort() )
				continue;
		}
	//	Don't send clients that have no parts available
		if (pPotentialSource->GetAvailablePartCount() == 0)
			continue;
	//	Don't share Lan clients sources as they are private ips/userids
		if (pPotentialSource->IsOnLAN())
			continue;
	//	don't send erroneous or fake sources
		if (uPartCount != pPotentialSource->GetPartCount())
			continue;

		bNeededPart = false;

	//	Only send source which have needed parts for this client if possible
		pbyteForClientPartStatus = pForClient->GetPartStatus();
		pbytePotentialClientPartStatus = pPotentialSource->GetPartStatus();

	//	Both clients support part statuses so we try to find needed clients
		if (pbyteForClientPartStatus != NULL && pbytePotentialClientPartStatus != NULL)
		{
			for (uint32 i = 0; i < uPartCount; i++)
			{
				if (pbytePotentialClientPartStatus[i] && !pbyteForClientPartStatus[i])
				{
					bNeededPart = true;
					break;
				}
			}
		}
	//	If not we give a sources with at least one part (for files that have more than one part)
		else
			bNeededPart = true;

		if (bNeededPart)
		{
			if (byteUsedVer >= 3)
				dwID = pPotentialSource->GetUserIDHybrid();
			else	//	use the old ed2k user ID convention for LowID sources on the same server
			{
				if (pPotentialSource->HasLowID())
					dwID = pPotentialSource->GetUserIDHybrid();
				else
					dwID = pPotentialSource->GetIP();
			}
			uPort = pPotentialSource->GetUserPort();
			dwSrvIP = pPotentialSource->GetServerIP();
			uSrvPort = pPotentialSource->GetServerPort();

			packetStream.Write(&dwID, 4);
			packetStream.Write(&uPort, 2);
			packetStream.Write(&dwSrvIP, 4);
			packetStream.Write(&uSrvPort, 2);
			if (byteUsedVer > 1)
			{
				packetStream.Write(pPotentialSource->GetUserHash(), 16);
				if (byteUsedVer >= 4)	// CryptSettings - SourceExchange V4
				{
					// 5 Reserved
					// 1 CryptLayer Required
					// 1 CryptLayer Requested
					// 1 CryptLayer Supported
					const byte byteCryptOptions = pPotentialSource->GetCryptLayer();

					packetStream.Write(&byteCryptOptions, 1);
				}
			}
			if (++dwCount > 500)
				break;
		}
	}

	if (dwCount == 0)
		return NULL;

	packetStream.Seek((bIsSX2Packet) ? 17 : 16, 0);
	packetStream.Write(&dwCount, 2);

	Packet	*pPacket = new Packet(&packetStream, OP_EMULEPROT);

	pPacket->m_eOpcode = (bIsSX2Packet) ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES;
// (1+)16+2+501*(4+2+4+2+16+1) = 14547 (14548) bytes max.
	if (pPacket->m_dwSize > 354)
		pPacket->PackPacket();

	return pPacket;

	EMULE_CATCH

#endif //NEW_SOCKETS_ENGINE
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::AddClientSources(CMemFile *packetStream, byte byteSXVer, bool bSX2, const CUpDownClient *pClient/*=NULL*/)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

//	If this part file is paused or complete/completing or erroneous no sources needed any more.
	EnumPartFileStatuses	eFileStatus = GetStatus();

	if ( (eFileStatus == PS_PAUSED) || (eFileStatus == PS_STOPPED)
		|| (eFileStatus == PS_COMPLETE) || (eFileStatus == PS_COMPLETING)
		|| (eFileStatus == PS_ERROR) )
		return;

	uint32		dwNumSources = 0, dwPktSXVer = 0, dwDataSize, dwSz;

	if (!bSX2)	//	For SX1 (deprecated)
	{
	//	Check if the data size matches the number of sources and eventually correct the source
	//	exchange version while reading the packet data. Otherwise we could experience a higher
	//	chance in dealing with wrong source data, user hashes and finally duplicate sources
		packetStream->Read(&dwNumSources, 2);
		dwDataSize = static_cast<uint32>(packetStream->GetLength() - packetStream->GetPosition());

	//	If a newer version inserts additional data (like v2), the below code will correctly filter those packets.
	//	If it appends additional data after <count>(<Sources>)[count], we are in trouble with the
	//	below code. Thus a client which doesn't understand newer version should never receive such a packet.
		if (dwNumSources * (4u + 2u + 4u + 2u + 16u + 1u) == dwDataSize)	//	Check for v4 packet
		{
			if (byteSXVer >= 4)
				dwPktSXVer = 4;
		}
		else if (dwNumSources * (4u + 2u + 4u + 2u + 16u) == dwDataSize)	//	Check for v2/v3 packet
		{
			if (byteSXVer >= 2)
				dwPktSXVer = (byteSXVer == 2) ? 2 : 3;
		}
		else if (dwNumSources * (4u + 2u + 4u + 2u) == dwDataSize)	//	Check for v1 packet
		{
			if (byteSXVer >= 1)
				dwPktSXVer = 1;
		}
	}
	else	//	For SX2
	{
		//	Check if a version is known by us and do quick sanity check on known version
		//	Unlike SX1, packet will be ignored in case of any error
		if ((static_cast<unsigned>(byteSXVer) - 1u) > static_cast<unsigned>(SOURCEEXCHANGE2_VERSION - 1))
		{
			g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T("Received invalid SX2 packet (v%u) from %s for %s"),
				byteSXVer, (pClient != NULL) ? pClient->GetClientNameWithSoftware() : _T("???"), GetFileName());
			return;
		}
	//	All known versions have first 2 bytes as count; unknown version were already filtered out above
		packetStream->Read(&dwNumSources, 2);
		dwDataSize = static_cast<uint32>(packetStream->GetLength() - packetStream->GetPosition());

		dwSz = ~dwDataSize;
		switch (byteSXVer)
		{
			case 1:
				dwSz = dwNumSources * (4u + 2u + 4u + 2u);
				break;
			case 2:
			case 3:
				dwSz = dwNumSources * (4u + 2u + 4u + 2u + 16u);
				break;
			case 4:
				dwSz = dwNumSources * (4u + 2u + 4u + 2u + 16u + 1u);
				break;
		}
		if (dwSz == dwDataSize)
			dwPktSXVer = byteSXVer;
	}

	if (dwPktSXVer == 0)
	{
		g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T("Received invalid SX%c packet (v%u) of data size %u (%u) from %s for %s"),
			(bSX2) ? _T('2') : _T('1'), byteSXVer, dwDataSize, dwNumSources,
			(pClient != NULL) ? pClient->GetClientNameWithSoftware() : _T("???"), GetFileName());
		return;
	}

	CClientSource	source;

	for (uint32 i = 0; i < dwNumSources; i++)
	{
		packetStream->Read(&source.dwSrcIDHybrid, 4);
	//	Convert the ed2k ID into Hybrid ID for SX v1 & v2
		if ((dwPktSXVer < 3) && !IsLowID(source.dwSrcIDHybrid))
			source.dwSrcIDHybrid = ntohl(source.dwSrcIDHybrid);
		packetStream->Read(&source.sourcePort, 2);
		packetStream->Read(&source.serverIP, 4);
		packetStream->Read(&source.serverPort, 2);
		if (dwPktSXVer > 1)
		{
			packetStream->Read(&source.achUserHash, 16);
			if (dwPktSXVer >= 4)
			{
				byte byteCryptOptions;
				
				packetStream->Read(&byteCryptOptions, 1);
			}
		}
		else
			md4clr(source.achUserHash);
		AddClientSource(&source, i, true, static_cast<byte>(dwPktSXVer));
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Updates priority of file if autopriority is activated
void CPartFile::UpdateDownloadAutoPriority(void)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (IsAutoPrioritized())
	{
		int iValidSources = GetSourceCount() - GetNotCurrentSourcesCount();

		if (iValidSources < g_App.m_pPrefs->PriorityHigh() && priority != PR_HIGH)
			SetPriority(PR_HIGH);
		else if (iValidSources >= g_App.m_pPrefs->PriorityHigh() && iValidSources < g_App.m_pPrefs->PriorityLow() && priority != PR_NORMAL)
			SetPriority(PR_NORMAL);
		else if (iValidSources >= g_App.m_pPrefs->PriorityLow() && priority != PR_LOW)
			SetPriority(PR_LOW);
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CPartFile::GetStatsFullPath() const
{
	return m_strFilePath + _T(".stats");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--- xrmb:keepPartFileStats ---
bool CPartFile::LoadPartFileStats()
{
	EMULE_TRY

	byte	abyteTmp[4 + 4 + 8];
	FILE	*fh = _tfsopen(GetStatsFullPath(), _T("rb"), _SH_DENYWR);

//	If we cant open this file you lose the stats
	if (fh == NULL)
		return false;

//	Get version
	byte	version = 0;
	if (fread(&version, sizeof(version), 1, fh) != 1)
	{
		fclose(fh);
		return false;
	}

//	Skip the former storage place for part file requests/accepted/tranfer statistics
//	Starting from v1.1c this data is kept in .part.met
//	'fread' is used instead of 'fseek' to increase performance of file stream buffering
	if (fread(abyteTmp, sizeof(abyteTmp), 1, fh) != 1)
	{
		fclose(fh);
		return false;
	}

//	This is for the parttraffic
//	I thought about explicit adding the traffic like for the other stats above
//	but it doesnt make sense, because you'll no transfer if you reach this
	if (!LoadFromFileTraffic(fh, version))
	{
#ifndef NEW_SOCKETS_ENGINE
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_TRAFFICDAT_CORRUPT, GetStatsFullPath());
		g_App.m_pMDlg->DisableAutoBackup();
#endif
		fclose(fh);
		return false;
	}

	fclose(fh);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::SavePartFileStats()
{
	static const uint32 s_adwDummy[4] = { 0, 0, 0, 0 };
	EMULE_TRY

//	Don't save for completing file as .part.stats is deleted anyway after completion;
//	Don't save for complete to avoid creation of wrong .stats in the destination
//	directory, as filenames are already changed
	if ((m_eStatus == PS_COMPLETING) || (m_eStatus == PS_COMPLETE))
		return false;

	FILE *fh = _tfsopen(GetStatsFullPath(), _T("wb"), _SH_DENYWR);

//	If we can't open this file you lose the stats
	if (fh == NULL)
		return false;

//	Set version
	byte	version = 2;
	if (fwrite(&version, sizeof(version), 1, fh) != 1)
	{
		fclose(fh);
		return false;
	}

//	Fill with zeroes the former storage place for part file requests/accepted/tranfer statistics
//	Starting from v1.1c this data is kept in .part.met
	if (fwrite(s_adwDummy, sizeof(s_adwDummy), 1, fh) != 1)
	{
		fclose(fh);
		return false;
	}

//	This is for the parttraffic
	if (SaveToFileTraffic(fh) == false)
	{
		fclose(fh);
		return false;
	}

	fclose(fh);

	return true;

	EMULE_CATCH

	return false;
}
//--- :xrmb ---
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Used as separate thread to complete file
UINT CPartFile::CompleteThreadProc(CPartFile* pFile)
{
#ifndef NEW_SOCKETS_ENGINE
#ifdef EP_SPIDERWEB
//	Setup Structured Exception handler for the this thread
	_set_se_translator(StructuredExceptionHandler);
#endif

	EMULE_TRY

	g_App.m_pPrefs->InitThreadLocale();

	if (pFile == NULL)
		return ~0u;

	pFile->PerformFileComplete();

	return 0;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return ~0u;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CPartFile::DtLocale(CTime dt)
{
#ifndef NEW_SOCKETS_ENGINE
	static const UINT		s_adwDayResId[] =
	{
		IDS_SCH_EXCEPT_SUN, IDS_SCH_EXCEPT_MON, IDS_SCH_EXCEPT_TUE, IDS_SCH_EXCEPT_WED,
		IDS_SCH_EXCEPT_THU, IDS_SCH_EXCEPT_FRI, IDS_SCH_EXCEPT_SAT
	};
	SYSTEMTIME st;
	dt.GetAsSystemTime(st);

	int		nDateSize = GetDateFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0);
	int		nTimeSize = GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, NULL, 0);
	CString	strDate, strTime, strBuffer;

	GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, strDate.GetBuffer(nDateSize), nDateSize);
	strDate.ReleaseBuffer();

	GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, strTime.GetBuffer(nTimeSize), nTimeSize);
	strTime.ReleaseBuffer();

	strBuffer.Format(_T("%s, %s, %s"),
		GetResString(s_adwDayResId[dt.GetDayOfWeek() - 1]), strDate, strTime);
	return strBuffer;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadSettingsFile() loads settings for partfiles (Movie Preview Mode, Stopped status, Preallocation)
void CPartFile::LoadSettingsFile()
{
	EMULE_TRY

	CString	strIniFile(m_strFilePath);
	uint32	dwMode;

	strIniFile += _T(".settings");

	CIni filesettings(strIniFile, INI_MODE_READONLY | INI_MODE_ANSIONLY);

	filesettings.SetDefaultCategory(_T("Settings"));
	dwMode = filesettings.GetUInt32(_T("MoviePreviewMode"), 0);
	if ((dwMode > 7) || ((dwMode & 6) == 2) || (dwMode == 6))	// Check for valid values 0, 1, 4, 5, 7
		dwMode = 0;
	CKnownFile::SetMovieMode(static_cast<byte>(dwMode));
	m_bStopped = filesettings.GetBool(_T("StoppedStatus"), m_bStopped);
	m_bIsPreallocated = filesettings.GetBool(_T("Preallocated"), m_bIsPreallocated);
	filesettings.GetString(&m_strAlternativePath, _T("AltDestDir"), _T(""));

	filesettings.CloseWithoutSave();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SaveSettingsFile() save settings for partfiles (Movie Preview Mode, Stopped status, Preallocation)
void CPartFile::SaveSettingsFile()
{
	EMULE_TRY

	CString strIniFile(m_strFilePath);

	strIniFile += _T(".settings");

	CIni filesettings(strIniFile, INI_MODE_ANSIONLY);

	filesettings.SetDefaultCategory(_T("Settings"));
	filesettings.SetInt(_T("MoviePreviewMode"), ((CKnownFile*)this)->GetMovieMode());
	filesettings.SetBool(_T("StoppedStatus"), m_bStopped);
	filesettings.SetBool(_T("Preallocated"), m_bIsPreallocated);
	filesettings.SetString(_T("AltDestDir"), m_strAlternativePath);

	filesettings.SaveAndClose();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	WriteToBuffer() buffer rx data which will be flushed later by timer or size threshold.
//		Params:
//			iComprGain - number of bytes saved by compression (it can be negative
//			             after connection loss in the middle of compressed block);
//			data       - pointer to data buffer;
//			qwStart    - data start offset;
//			qwEnd      - data end offset (inclusive);
//			iFlags     - additional flags.
//				PF_WR2BUF_FL_FREEBUFFER - data buffer can be used for further operations (caller doesn't free it)
//				PF_WR2BUF_FL_ENDOFBLOCK - current data is the last portion of the requested block
//		Return: number of buffered bytes
uint32 CPartFile::WriteToBuffer(sint32 iComprGain, BYTE *data, uint64 qwStart, uint64 qwEnd, int iFlags)
{
	EMULE_TRY

	uint32	dwDataLen = static_cast<uint32>(qwEnd - qwStart + 1);

	if (iComprGain != 0)
	{
	//	Compression gain can be negative in case of disconnection in the middle of compressed stream
		m_qwGainDueToCompression += static_cast<sint64>(iComprGain);
		g_App.m_pPrefs->Add2SavedFromCompression(iComprGain);
	}

//	Occasionally packets are duplicated, no point writing it twice
	if (IsComplete(qwStart, qwEnd))
	{
		AddLogLine(LOG_FL_DBG, _T("File '%s' has already been written from %I64u to %I64u size %u"), GetFileName(), qwStart, qwEnd, dwDataLen);
	}
	else
	{
	//	Create a new buffered queue entry
		PartFileBufferedData *item = new PartFileBufferedData;

		item->qwStartOffset = qwStart;
		item->qwEndOffset = qwEnd;

		if ((iFlags & PF_WR2BUF_FL_FREEBUFFER) == 0)
		{
		//	Create copy of data as new buffer
			BYTE *buffer = new BYTE[dwDataLen];

			memcpy2(buffer, data, dwDataLen);
			data = buffer;
		}
		item->pbyteBuffer = data;

	//	Add to the queue in the correct position (most likely the end)
		PartFileBufferedData *queueItem;
		bool added = false;
		POSITION pos = m_BufferedData_list.GetTailPosition();
		if (pos != NULL)
		{
			do
			{
				POSITION posNext = pos;

				queueItem = m_BufferedData_list.GetPrev(pos);
				if (item->qwEndOffset > queueItem->qwEndOffset)
				{
					added = true;
					m_BufferedData_list.InsertAfter(posNext, item);
					break;
				}
			} while (pos != NULL);
		}
	//	Reset timer for empty cache to buffer more effectively, otherwise timer
	//	running all the time can flush just a little data to the disk on the first hit
		else
			m_nLastBufferFlushTime = ::GetTickCount();
		if (!added)
			m_BufferedData_list.AddHead(item);

	//	Increment buffer size marker
		m_nTotalBufferData += dwDataLen;

	//	Mark this small section of the file as filled
		FillGap(item->qwStartOffset, item->qwEndOffset);

		uint32 dwPartNum = static_cast<uint32>(qwStart / PARTSIZE);

	//	Check that all data of the part is received
		if (IsPartFull(dwPartNum))
		{
		//	Prevent data flush during processesing of the data from the sockets
			m_bDataFlushReq = true;
		}
		else if (((iFlags & PF_WR2BUF_FL_ENDOFBLOCK) != 0) && IsCorruptedPart(dwPartNum))
		{
		//	Flush and check data to minimize amount of additional unrequired redownloads
		//	Do it only by end of the block basis to minimize system load as some clients send by small portions
			m_bDataFlushReq = true;
		}

		m_timeLastDownTransfer = CTime::GetCurrentTime();

	//	Return the length of data written to the buffer
		return dwDataLen;
	}

	EMULE_CATCH

	if ((iFlags & PF_WR2BUF_FL_FREEBUFFER) != 0)
		delete[] data;

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FlushBuffer() flushes buffered data from the source to the part file
void CPartFile::FlushBuffer(void)
{
	EMULE_TRY

	m_bDataFlushReq = false;

	m_nLastBufferFlushTime = ::GetTickCount();

//	If there's no buffered data to flush, just return
	if (m_BufferedData_list.IsEmpty())
		return;

//	Reset the Last Down Transfer timer
	m_timeLastDownTransfer = CTime::GetCurrentTime();
	GetAvgDataRate(true);

//	Calculate the number of parts in the part file
	uint32	dwBegPart, dwEndPart, dwPartCount = GetPartCount();
	bool *changedPart = new bool[dwPartCount];

	try
	{
	//	Remember which parts need to be checked at the end of the flush
		for (uint32 dwPartNum = 0; dwPartNum < dwPartCount; dwPartNum++)
			changedPart[dwPartNum] = false;

	//	Ensure file is big enough to write data to (the last item will be the furthest from the start)
		PartFileBufferedData *item = m_BufferedData_list.GetTail();

	//	If the end of the buffered data is past the current end of the part file, extend it
		if (m_hPartFileWrite.GetLength() <= item->qwEndOffset)
			m_hPartFileWrite.SetLength(item->qwEndOffset + 1);

	//	For each block of buffered data...
		for (int i = m_BufferedData_list.GetCount(); i > 0; i--)
		{
		//	Remove item from queue
			item = m_BufferedData_list.RemoveHead();

		//	For each part spanned by the buffered data, set the part's changed flag
			dwBegPart = static_cast<uint32>(item->qwStartOffset / PARTSIZE);
			dwEndPart = static_cast<uint32>(item->qwEndOffset / PARTSIZE);
			for (uint32 dwPart = dwBegPart; dwPart <= dwEndPart; dwPart++)
				changedPart[dwPart] = true;

		//	Go to the correct position in file and write block of data
			m_hPartFileWrite.Seek(item->qwStartOffset, CFile::begin);

			uint32 lenData = static_cast<uint32>(item->qwEndOffset - item->qwStartOffset + 1);

		//	Decrease buffer size
			m_nTotalBufferData -= lenData;
			m_hPartFileWrite.Write(item->pbyteBuffer, lenData);

		//	Release memory used by this item
			delete[] item->pbyteBuffer;
			delete item;
		}

	//	If the size of the part file is greater than the size of the target file,
	//	set it to the size of the target file.
		if (m_hPartFileWrite.GetLength() > GetFileSize())
			m_hPartFileWrite.SetLength(GetFileSize());

	//	Flush to disk
		m_hPartFileWrite.Flush();

	//	Check each part of the file
		uint32	partRange = GetLastPartSize() - 1;
		uint64	qwPartBeg, qwPartEnd;

	//	For each part in the part file (in reverse)...
		for (int iPartNum = (int)dwPartCount - 1; iPartNum >= 0; iPartNum--)
		{
		//	If we didn't just write it, skip it
			if (changedPart[iPartNum] == false)
			{
			//	Any parts other than last must be full size
				partRange = PARTSZ32 - 1;
				continue;
			}

			qwPartBeg = static_cast<uint64>(static_cast<uint32>(iPartNum)) * PARTSIZE;
			qwPartEnd = qwPartBeg + static_cast<uint64>(partRange);
		//	Is this 9MB part complete
			if (IsComplete(qwPartBeg, qwPartEnd))
			{
			//	Is part corrupt
				if (!HashSinglePart(iPartNum))
				{
#ifndef NEW_SOCKETS_ENGINE
					AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_PARTCORRUPT, iPartNum, GetFileName());
#endif
					AddGap(qwPartBeg, qwPartEnd);
					m_csGapListAndPartStatus.Lock();
					m_PartsStatusVector[iPartNum] |= PART_CORRUPTED;
					m_csGapListAndPartStatus.Unlock();
				//	Increment corruption statistics
					AddRxCorruptedAmount(partRange + 1);
				}
				else
				{
					m_csGapListAndPartStatus.Lock();
					m_PartsStatusVector[iPartNum] = (m_PartsStatusVector[iPartNum] & 0xFFFFFF) | PART_VERIFIED;
					m_csGapListAndPartStatus.Unlock();

				//	Successfully completed part, make it available for sharing
					if (m_eStatus == PS_EMPTY)
					{
						SetStatus(PS_READY);
#ifndef NEW_SOCKETS_ENGINE
					//	Delay publishing a little to avoid double publishing of small files
						g_App.m_pSharedFilesList->SafeAddKnownFile(this, false, true);
#endif //NEW_SOCKETS_ENGINE
					}

					m_qwCompletedPartsSize += static_cast<uint64>(partRange + 1);
				}
			}
			else if (IsCorruptedPart(iPartNum))
			{
			//	Try to recover with minimal loss
				if (HashSinglePart(iPartNum))
				{
					m_csGapListAndPartStatus.Lock();
					m_PartsStatusVector[iPartNum] = (m_PartsStatusVector[iPartNum] & 0xFFFFFF) | PART_VERIFIED;
					m_csGapListAndPartStatus.Unlock();

					m_iTotalPacketsSavedDueToICH++;
#ifndef NEW_SOCKETS_ENGINE
					g_App.m_pPrefs->IncSessionPartsSavedByICH();
#endif //NEW_SOCKETS_ENGINE

					uint32 dwSaved = GetPartLeftToDLSize(iPartNum);

					FillGap(qwPartBeg, qwPartEnd);
					RemoveBlockFromList(qwPartBeg, qwPartEnd);
#ifndef NEW_SOCKETS_ENGINE
					AddLogLine(LOG_FL_SBAR | LOG_RGB_SUCCESS, IDS_ICHWORKED, iPartNum, m_strFileName, CastItoXBytes(dwSaved));
#endif

				//	Decrement corruption statistics by the number of saved bytes
					m_qwLostDueToCorruption -= dwSaved;
					g_App.m_pPrefs->SubLostFromCorruption(dwSaved);

				//	Successfully completed part, make it available for sharing
					if (m_eStatus == PS_EMPTY)
					{
						SetStatus(PS_READY);
#ifndef NEW_SOCKETS_ENGINE
					//	Delay publishing a little to avoid double publishing of small files
						g_App.m_pSharedFilesList->SafeAddKnownFile(this, false, true);
#endif //NEW_SOCKETS_ENGINE
					}
					m_qwCompletedPartsSize += static_cast<uint64>(partRange + 1);
				}
			}
		//	Any parts other than last must be full size
			partRange = PARTSZ32 - 1;
		}

	//	Update met file
		SavePartFile();

	//	Is this file finished?
		m_csGapListAndPartStatus.Lock();

		bool	bIsFileFinished = gaplist.IsEmpty() && !m_bHashSetNeeded;	// can't complete without a hashset

		m_csGapListAndPartStatus.Unlock();

		if (bIsFileFinished)
			CompleteFile(false);
	}
	catch (CFileException * error)
	{
		OUTPUT_DEBUG_TRACE();

#ifndef NEW_SOCKETS_ENGINE
		if (g_App.m_pPrefs->IsErrorBeepEnabled())
			Beep(800, 200);
#endif //NEW_SOCKETS_ENGINE

		if (error->m_cause == CFileException::diskFull)
		{
			CString strSpace = CastItoXBytes(GetFreeDiskSpaceX(m_strTempDir));
			CString strTemp;

			strTemp.Format(_T("%s (%s)"), strSpace, m_strTempDir);
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_OUTOFSPACE, GetFileName());
			AddLogLine(LOG_RGB_ERROR, IDS_DWTOT_FS, strTemp);
		//	Sending message for insufficient diskspace
			CString MessageText;
			MessageText.Format(GetResString(IDS_ERR_OUTOFSPACE), GetFileName());
			g_App.m_pMDlg->SendMail(MessageText, g_App.m_pPrefs->GetNotifierPopOnImportantError(), g_App.m_pPrefs->IsSMTPWarningEnabled());
			g_App.m_pMDlg->ShowNotifier(MessageText, TBN_IMPORTANTEVENT, false, g_App.m_pPrefs->GetNotifierPopOnImportantError());
#endif //NEW_SOCKETS_ENGINE
		}
		else
		{
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_WRITEERROR, GetFileName(), GetErrorMessage(error));
#endif
			SetStatus(PS_ERROR);
		}
	//	Don't pause stopped file to keep the state
		if (!m_bStopped)
			PauseFile();
		error->Delete();
	}
	catch (...)
	{
#ifndef NEW_SOCKETS_ENGINE
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_WRITEERROR, GetFileName(), GetResString(IDS_UNKNOWN));
#endif
		SetStatus(PS_ERROR);
		PauseFile();

	//	When ERRONEOUS close PartFile to allow Initialize
		ClosePartFile();
	}
	delete[] changedPart;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This will invert the gap list, up to caller to delete gaps when done
// 'Gaps' returned are really the filled areas, and guaranteed to be in order
// Returned format is like for 'gaplist' -- the end is inclusive
void CPartFile::GetFilledList(CTypedPtrList<CPtrList, Gap_Struct*> *filled)
{
	EMULE_TRY

	Gap_Struct	*gap, *best = NULL;
	POSITION	pos;
	uint64		qwBestEnd, qwStart = 0;

//	Will be unlocked on exit
	CSingleLock lockGap(&m_csGapListAndPartStatus, TRUE);

//	Loop until done
	bool finished = B2b(gaplist.IsEmpty());	//	no need to crash on the empty list...

	while (!finished)
	{
		finished = true;
	//	Find first gap after current start pos
		qwBestEnd = GetFileSize();
		pos = gaplist.GetHeadPosition();
		while (pos != NULL)
		{
			gap = gaplist.GetNext(pos);
			if ((gap->qwStartOffset > qwStart) && (gap->qwEndOffset < qwBestEnd))
			{
				best = gap;
				qwBestEnd = best->qwEndOffset;
				finished = false;
			}
		}

		if (!finished)
		{
		//	Invert this gap
			gap = new Gap_Struct;
			gap->qwStartOffset = qwStart;
			gap->qwEndOffset = best->qwStartOffset - 1ui64;
			qwStart = best->qwEndOffset + 1ui64;
			filled->AddTail(gap);
		}
		else
		{
		//	This is possible if there's only one gap in the beginning of the file
			if (best == NULL)
				best = gaplist.GetHead();
			qwBestEnd = GetFileSize() - 1ui64;
			if (best->qwEndOffset < qwBestEnd)	//	Don't add anything beyond file size
			{
				gap = new Gap_Struct;
				gap->qwStartOffset = best->qwEndOffset + 1ui64;
				gap->qwEndOffset = qwBestEnd;
				filled->AddTail(gap);
			}
		}
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::UpdateFileRatingCommentAvail()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (!this)
		return;

	bool prev = (m_bHasComment || m_bHasRating);

	m_bHasComment = false;
	m_bHasRating = false;

	ClientList	clientListCopy;

	GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		CUpDownClient	*pSource = *cIt;

		if (!pSource->IsFileCommentEmpty())
			m_bHasComment = true;
		if (pSource->GetFileRating() != PF_RATING_NONE)
			m_bHasRating = true;
		if (m_bHasComment && m_bHasRating)
			break;
	}

	if ((!m_bHasComment) || (!m_bHasRating))
	{
		for (POSITION pos = m_pastCommentList.GetHeadPosition(); pos != NULL;)
		{
			CPastComment &pc = m_pastCommentList.GetNext(pos);
			if (pc.GetComment().GetLength() > 0)
				m_bHasComment = true;
			if (pc.GetRating() > 0)
				m_bHasRating = true;
			if (m_bHasComment && m_bHasRating)
				break;
		}
	}

	if ((!m_bHasComment) || (!m_bHasRating))
	{
		EnumPartFileRating	eRated = GetFileRating();

		if (!GetFileComment().IsEmpty())
			m_bHasComment = true;
		if (eRated != PF_RATING_NONE)
			m_bHasRating = true;
	}

	if (prev != (m_bHasComment || m_bHasRating))
		UpdateDisplayedInfo();

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint16 CPartFile::GetSourceCount()
{
	uint32	dwCount = 0;

	for (EnumDLQState eDS = DS_DOWNLOADING; eDS < DS_LAST_QUEUED_STATE; ++eDS)
		dwCount += m_SourceLists[eDS].size();
	return static_cast<uint16>(dwCount);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EnumPartFileRating CPartFile::GetRating()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (!m_bHasRating)
		return PF_RATING_NONE;

	uint32	dwNum = 0, dwTot = 0, dwRate;
	ClientList	clientListCopy;

	GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		CUpDownClient	*pSource = *cIt;

		dwRate = pSource->GetFileRating();
		if (dwRate != 0)
		{
			dwNum++;
		//	Bugfix: for some £$%#ing reason fair=4 & good=3, breaking the progression from fake(1) to excellent(5)
			if (dwRate == PF_RATING_GOOD || dwRate == PF_RATING_FAIR)
				dwRate = (dwRate == PF_RATING_GOOD) ? 4 : 3;

			dwTot += dwRate;
		}
	}

	for (POSITION pos = m_pastCommentList.GetHeadPosition(); pos != NULL;)
	{
		dwRate = m_pastCommentList.GetNext(pos).GetRating();

		if (dwRate != 0)
		{
			dwNum++;
		//	Bugfix: for some £$%#ing reason  fair=4 & good=3, breaking the progression from fake(1) to excellent(5)
			if (dwRate == PF_RATING_GOOD || dwRate == PF_RATING_FAIR)
				dwRate = (dwRate == PF_RATING_GOOD) ? 4 : 3;

			dwTot += dwRate;
		}
	}

	dwRate = GetFileRating();
	if (dwRate != 0)
	{
		dwNum++;
	//	Bugfix: for some £$%#ing reason  fair=4 & good=3, breaking the progression from fake(1) to excellent(5)
		if (dwRate == PF_RATING_GOOD || dwRate == PF_RATING_FAIR)
			dwRate = (dwRate == PF_RATING_GOOD) ? 4 : 3;

		dwTot += dwRate;
	}

	if (dwNum != 0)
	{
		dwNum = (dwTot + (dwNum >> 1u)) / dwNum;	//Cax2 - get the average of all the ratings
	//	Bugfix: for some £$%#ing reason good=3 & fair=4, breaking the progression from fake(1) to excellent(5)
		if (dwNum == 3 || dwNum == 4)
			dwNum = (dwNum == 3) ? PF_RATING_FAIR : PF_RATING_GOOD;
	}
	return static_cast<_EnumPartFileRating>(dwNum);	//0 returned if no ratings found

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return PF_RATING_NONE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	UpdateDisplayedInfo() causes the DownloadList ctrl to redisplay the item for this file if either
void CPartFile::UpdateDisplayedInfo()
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	g_App.m_pDownloadList->UpdateFile(this);
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Hostname sources in ED2K superLink supporting Source Exchange v2
void CPartFile::AddClientSource(CClientSource* pSource, int iSource, bool bExchanged, byte byteSourceExchangeVersion)
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
//	Check first if we are this source
	if (g_App.m_pServerConnect->IsLowID() && g_App.m_pServerConnect->IsConnected())
	{
		if ( g_App.m_pServerConnect->GetClientID() == pSource->dwSrcIDHybrid &&
			 g_App.m_pPrefs->GetPort() == pSource->sourcePort )
		{
			return;
		}

	//	Although we are currently having a LowID, we could have had a HighID before, which
	//	that client is sending us now! seems unlikely ... it happend!
		if ( (g_App.m_pPrefs->GetPort() == pSource->sourcePort) &&
			(ntohl(g_App.m_pServerConnect->GetLocalIP()) == pSource->dwSrcIDHybrid) )
		{
			return;
		}
	}
	else if ( (g_App.m_pPrefs->GetPort() == pSource->sourcePort) &&
		(ntohl(g_App.m_pServerConnect->GetClientID()) == pSource->dwSrcIDHybrid) )
	{
		return;
	}
//	Use LowID source only if it's on the same server with us
	else if ( IsLowID(pSource->dwSrcIDHybrid) &&
			  !g_App.m_pServerConnect->IsLocalServer(pSource->serverIP, pSource->serverPort) )
	{
		return;
	}
#endif //OLD_SOCKETS_ENABLED

#ifndef NEW_SOCKETS_ENGINE
	if ( g_App.m_pPrefs->GetMaxSourcePerFile() > GetSourceCount()
	     && iSource < static_cast<int>(g_App.m_pPrefs->GetMaxSourcePerFileSoft()) )
	{
		CUpDownClient* pNewSource = g_App.m_pDownloadQueue->CheckAndAddSource(this, pSource->dwSrcIDHybrid, pSource->sourcePort, pSource->serverIP, pSource->serverPort, pSource->achUserHash);
		if (bExchanged && pNewSource)
		{
		//	Name the exchanged source
			pNewSource->SetUserName(GetResString(IDS_EXCHANGEDSOURCE));
		//	Check exchange version
			if (byteSourceExchangeVersion > 1)
				pNewSource->SetUserHash(pSource->achUserHash);
		}
	}
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::AddClientSources(CTypedPtrList<CPtrList, CClientSource*>* sources)
{
	EMULE_TRY

	POSITION pos;
	pos = sources->GetHeadPosition();
	while (pos != NULL)
	{
		CClientSource	*pSrc = (CClientSource*)sources->GetAt(pos);

		if (pSrc->sourceType != ED2KLINK_SOURCE_IP)
		{
			sources->GetNext(pos);
			continue;
		}

		POSITION removePos = pos;
		sources->GetNext(pos);
		sources->RemoveAt(removePos);

		AddClientSource(pSrc);
		delete pSrc;
	}
#ifndef NEW_SOCKETS_ENGINE
	g_App.m_pDownloadQueue->AddClientHostnameToResolve(sources);
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Create source ed2k from downloads
CString CPartFile::CreateED2KSourceLink(uint32 dwExpireIn, int iSourceCnt)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	CTypedPtrList<CPtrList, CUpDownClient*>	newSourceList;
	ClientList		clientListCopy;
	CUpDownClient	*pSource;

	GetCopySourceLists(SLM_ALL, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		pSource = *cIt;
		if (pSource->HasLowID())
			continue;
		if (newSourceList.IsEmpty())
		{
			newSourceList.AddHead(pSource);
			continue;
		}
		if ((iSourceCnt == 0) || (newSourceList.GetCount() < iSourceCnt) || (pSource->GetAvailablePartCount() > newSourceList.GetTail()->GetAvailablePartCount()))
		{
			if ((iSourceCnt != 0) && (newSourceList.GetCount() == iSourceCnt))
				newSourceList.RemoveTail();

			ASSERT(newSourceList.GetCount() < iSourceCnt);

			POSITION	pos, pos2;
			bool		bInserted = false;

			for (pos = newSourceList.GetTailPosition(); (pos2 = pos) != NULL;)
			{
				if (newSourceList.GetPrev(pos)->GetAvailablePartCount() > pSource->GetAvailablePartCount())
				{
					newSourceList.InsertAfter(pos2, pSource);
					bInserted = true;
					break;
				}
			}
			if (!bInserted)
				newSourceList.AddHead(pSource);
		}
	}

	if (newSourceList.IsEmpty())
	{
#ifdef OLD_SOCKETS_ENABLED
		if (!g_App.m_pServerConnect->IsConnected() || g_App.m_pServerConnect->IsLowID())
		{
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_SOURCELINKFAILED);
			return _T("");
		}
#else
		return _T("");
#endif //OLD_SOCKETS_ENABLED
	}

#ifdef OLD_SOCKETS_ENABLED
	uint32	dwID = g_App.m_pServerConnect->GetClientID();
#else
	uint32	dwID = 0;
#endif //OLD_SOCKETS_ENABLED

	CString	strLink(CreateED2kLink());

	strLink += _T("|sources");
	if (dwExpireIn != 0)
	{
		CTime	timeExpirationDate = CTime::GetCurrentTime() + CTimeSpan(dwExpireIn, 0, 0, 0);

		strLink += timeExpirationDate.Format(_T("@%y%m%d"));
	}

	if (!IsLowID(dwID))
		strLink.AppendFormat(_T(",%u.%u.%u.%u:%u"), (byte)dwID, (byte)(dwID >> 8), (byte)(dwID >> 16), (byte)(dwID >> 24), g_App.m_pPrefs->GetPort());

	AddLogLine(LOG_FL_DBG, _T("Generated ed2k link with %u sources for file '%s' (part count %u)"), newSourceList.GetCount(), GetFileName(), GetPartCount());

	while (!newSourceList.IsEmpty())
	{
		pSource = newSourceList.RemoveHead();
		AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("| User: %s - Available part count: %u"), pSource->GetUserName(), pSource->GetAvailablePartCount());
		strLink.AppendFormat(_T(",%s:%u"), pSource->GetFullIP(), pSource->GetUserPort());
	}

	strLink += _T("|/");
	return strLink;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return _T("");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::SetAlternativeOutputDir(CString *path)
{
	path->TrimRight(_T('\\'));
	m_strAlternativePath = *path;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetOutputDir() returns the appropriate destination directory path for this file.
CString CPartFile::GetOutputDir()
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	CString	strRetPath = g_App.m_pPrefs->GetIncomingDir();
	CString	strCatPath;

	if (GetCatID() != CAT_NONE)
		strCatPath = CCat::GetCatByID(GetCatID())->GetPath();

//	If no alternative path defined
	if (m_strAlternativePath.IsEmpty())
	{
	//	... and a category dir is defined...
		if (strCatPath != _T(""))
			return strCatPath; // Return the category dir
		else
			return strRetPath;	// Return the incoming dir
	}

//	MOREVIT - Why all the heavy duty shell call nonsense? You could replace all of this
//	with a single call to PathFileExists(). Somebody just thought they were more
//	clever than they were methinks.

//	We're going to use desktop folder "static" functions , we need an object
//	of IShellFolder type and DeskTop always exists
	IShellFolder	*pDesktop = NULL;
	ITEMIDLIST	*pList = NULL;
	CString	strDir;

//	Try to create DeskTop folder object
	if (SHGetDesktopFolder(&pDesktop) == NOERROR)
	{
		ULONG	uAttrib	= SFGAO_FOLDER; //we're going to look for the "folder" attribute

		strDir = m_strAlternativePath;
	//	"ParseDisplayName" uses UNICODE so BSTR will go
	//	We have to remember to release it
		BSTR	pData	= strDir.AllocSysString();

		if (pData)
		{
		//	'm_strAlternativePath' is always created without "\" but this was the way
		//	the original code was, maybe it was for a reason :) , so I am going to leave
		//	first check without adding "\" as well ...

		//	So here we convert "display name" (path string) into ID list used by Shell
			pDesktop->ParseDisplayName(NULL, NULL, pData, NULL, &pList, &uAttrib);
		//	Release the BSTR
			::SysFreeString(pData);
		//	If list we got is null then no such object exists
			if (!pList)
			{
			//	Add "\" to get folder name
				strDir += _T('\\');
			//	Allocate new search BSTR
				pData = strDir.AllocSysString();
				if (pData)
				{
				//	We're going to look for "folder" attribute
					uAttrib	= SFGAO_FOLDER ;
				//	We convert "display name" (path string) into ID list used by Shell
					pDesktop->ParseDisplayName(NULL, NULL, pData, NULL, &pList, &uAttrib);
				//	Release the BSTR
					::SysFreeString(pData);
				}
			}
		//	If we found the object and it's "folder" type
			if (pList && (uAttrib & SFGAO_FOLDER))
			{
				LPMALLOC	ppMalloc;

				if (SHGetMalloc(&ppMalloc) == NOERROR)
				{
					ppMalloc->Free(pList);
					ppMalloc->Release();
				}

			//	Share the destination directory
				if (g_App.m_pPrefs->SharedDirListCheckAndAdd(strDir, true))
				{
				//	New path was added to the list, scan for files
					g_App.m_pSharedFilesList->Reload();
				}
				return m_strAlternativePath;
			}
		//	Just in case the object is not folder we have to release memory
			else if (pList)
			{
				LPMALLOC	ppMalloc;

				if (SHGetMalloc(&ppMalloc) == NOERROR)
				{
					ppMalloc->Free(pList);
					ppMalloc->Release();
				}
			}
		}
		pDesktop->Release();
	}

//	Default return
	strRetPath.TrimRight(_T('\\'));
	return strRetPath;

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE

	return _T("");
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CPartFile::GetDownloadFileInfo()
{
#ifndef NEW_SOCKETS_ENGINE
	CString strInfo, strLastSeenComplete, strLastProgress, strTimeLeft, strFileSz = CastItoXBytes(GetFileSize());
	double dblAvailability = 0.0;

	if (lastseencomplete == NULL)
		GetResString(&strLastSeenComplete, IDS_NEVER);
	else
		strLastSeenComplete = LocalizeLastSeenComplete();

	if (GetTransferred() == 0)
		GetResString(&strLastProgress, IDS_NEVER);
	else
		strLastProgress = LocalizeLastDownTransfer();

	if (GetPartCount() != 0)
		dblAvailability = GetAvailablePartCount() * 100 / GetPartCount();

	EnumPartFileStatuses	eFileStatus = GetStatus();

	if ((eFileStatus != PS_COMPLETING) && (eFileStatus != PS_COMPLETE))
		strTimeLeft = CastSecondsToHM(GetTimeRemaining());
	else
		strTimeLeft = _T("-");

	strInfo.Format( _T("%s (%s)\n\n%s: %s\n\n%s: %s\n") + GetResString(IDS_PARTINFOS),
		GetFileName(), strFileSz,
		GetResString(IDS_STATUS), GetPartFileStatus(),
		GetResString(IDS_FILEHASH), HashToString(GetFileHash()),
		GetResString(IDS_DL_FILENAME), m_strFilePath,
		GetPartCount(), GetResString(IDS_AVAIL), GetAvailablePartCount(), dblAvailability,
		static_cast<int>(GetPercentCompleted()),
		CastItoXBytes(GetCompletedSize()), strFileSz, GetTransferringSrcCount(),
		GetResString(IDS_LASTSEENCOMPLETE), strLastSeenComplete,
		GetResString(IDS_LASTRECEPTION), strLastProgress,
		GetResString(IDS_DLCOL_REMAININGTIME), strTimeLeft,
		GetOnQueueSrcCount(), GetQueueFullSrcCount(), GetNoNeededPartsSrcCount(),
		GetSrcA4AFCount(), GetConnectingSrcCount() );

	return strInfo;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Improved Tooltips
CString CPartFile::GetDownloadFileInfo4Tooltips()
{
#ifndef NEW_SOCKETS_ENGINE
	CString strInfo, strLastSeenComplete, strLastProgress, strTransferSrcCount, strFileSz = CastItoXBytes(GetFileSize());
	double dblAvailability = 0.0;

	if (lastseencomplete == NULL)
		GetResString(&strLastSeenComplete, IDS_NEVER);
	else
		strLastSeenComplete = LocalizeLastSeenComplete();

	if (GetTransferred() == 0)
		GetResString(&strLastProgress, IDS_NEVER);
	else
		strLastProgress = LocalizeLastDownTransfer();

	if (GetPartCount() != 0)
		dblAvailability = GetAvailablePartCount() * 100 / GetPartCount();

	if (GetTransferringSrcCount() > 0)
		strTransferSrcCount.Format(GetResString(IDS_TT_PARTINFOS3), GetTransferringSrcCount());

	strInfo.Format(_T("<t=2><b>%s</b><br><t=2>%s (%s %s)<br><t=2>%d%s (%s / %s) %s<br><hr=100%%><br><b>%s:<t></b>%s<br><b>%s:<t></b>%s<br><b>%s:<t></b>%s<br><b>%s<t></b>%s<br><b>%s:<t></b>%d, %s: %d (%.1f%%)<br><b>%s:<t></b>%s<br><b>%s:<t></b>%s<br><b>%s:<t></b>")
		+ GetResString(IDS_TT_STATUS2) + _T("<br><b>%s:<t></b>%s"),
		GetFileName(), strFileSz, CastItoThousands(GetFileSize()), GetResString(IDS_BYTES),
		static_cast<int>(GetPercentCompleted()), GetResString(IDS_PROCDONE), CastItoXBytes(GetCompletedSize()), strFileSz, strTransferSrcCount,
		GetResString(IDS_STATUS), GetPartFileStatus(),
		GetResString(IDS_FILEHASH), HashToString(GetFileHash()),
		GetResString(IDS_DL_FILENAME), m_strFilePath,
		GetResString(IDS_FD_OUTPUT), GetOutputDir(),
		GetResString(IDS_UP_PARTS), GetPartCount(), GetResString(IDS_AVAIL), GetAvailablePartCount(), dblAvailability,
		GetResString(IDS_LASTSEENCOMPLETE), strLastSeenComplete,
		GetResString(IDS_LASTRECEPTION), strLastProgress,
		GetResString(IDS_DL_SOURCES), GetOnQueueSrcCount(),
		GetQueueFullSrcCount(),
		GetNoNeededPartsSrcCount(),
		GetSrcA4AFCount(),
		GetConnectingSrcCount(),
		GetResString(IDS_SIZE_ON_DISK), CastItoXBytes(GetRealFileSize()) );

	return strInfo;
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Generate file progress bar for WebServer
void CPartFile::GetProgressString(CString *pstrChunkBar, uint32 dwSize)
{
	EMULE_TRY

	const TCHAR crProgress = _T('9');	// Green
	EnumPartFileStatuses	eFileStatus = GetStatus();

	if ((eFileStatus == PS_COMPLETE) || (eFileStatus == PS_COMPLETING))
	{
		for (uint32 i = 0; i < dwSize; i++)
			pstrChunkBar->AppendChar(crProgress);
	}
	else
	{
		CArray<Gap_Struct, Gap_Struct> aGaps;
		const COLORREF	crHave = RGB(0, 0, 0);			// Black
		const COLORREF	crPending = RGB(0, 255, 0);		// Yellow
		const COLORREF	crMissing = RGB(255, 0, 0);		// Red
		CBarShader		ChunkBar(0, dwSize, crHave, GetFileSize());

	//	Make a gap list copy to avoid synchronization object allocation for a long time
		GetGapListCopy(&aGaps);

		Gap_Struct	*pGap = aGaps.GetData();
		Gap_Struct	*pGapMax = pGap + aGaps.GetCount();

		for (; pGap < pGapMax; pGap++)
		{
			uint64	qwGapBeg = pGap->qwStartOffset;
			uint64	qwGapEnd = pGap->qwEndOffset + 1ui64;

			for (uint32 i = static_cast<uint32>(qwGapBeg / PARTSIZE); i < GetPartCount(); i++)
			{
				const uint64 qwPartStart = static_cast<uint64>(i) * PARTSIZE;
				const uint64 qwPartEnd = ((qwPartStart + PARTSIZE) > GetFileSize()) ? GetFileSize() : (qwPartStart + PARTSIZE);
				unsigned	uiFreq;
				COLORREF	color;

				if (qwGapEnd > qwPartEnd)
					qwGapEnd = qwPartEnd;	// The rest is in the next part

				if (m_srcPartFrequencies.GetCount() >= (INT_PTR)i && ((uiFreq = m_srcPartFrequencies[i]) != 0))
				{
					if (IsPartDownloading(i))
						color = crPending;	// Downloading part in yellow
					else
						color = RGB(0, 0, (uiFreq < 11) ? (6 - ((uiFreq - 1u) >> 1u)) : 1);	//1(dark blue)..6(light blue)
				}
				else
					color = crMissing;

				ChunkBar.FillRange(qwGapBeg, qwGapEnd, color);

				qwGapBeg = qwGapEnd;
				qwGapEnd = pGap->qwEndOffset + 1ui64;
				if (qwGapBeg == qwGapEnd) // finished?
					break;
			}
		}
		ChunkBar.GenerateWSBar(pstrChunkBar);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A4AF management
void CPartFile::DownloadAllA4AF(bool bSameCat /* FALSE */)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (!m_A4AFSourceLists.empty())
	{
		ClientList			clientListCopy;
		CUpDownClient*		pClient;
	
		GetCopyA4AFSourceList(&clientListCopy);
		for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
		{
			pClient = *cIt;

			if(bSameCat)
			{
				if (pClient->m_pReqPartFile->GetCatID() == GetCatID())
					pClient->SwapToAnotherFile(this);
			}
			else
			{
				pClient->SwapToAnotherFile(this);
			}
		}
	}

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
byte* CPartFile::MMCreatePartStatus()
{
	EMULE_TRY

//	Create partstatus + info in mobilemule protocol specs
//	result needs to be deleted[] | slow, but not timecritical
	byte		*result = new byte[GetPartCount() + 1];
	uint64		qwPartBeg;
	unsigned	uiFreq, uiOctet;

	for (unsigned ui = 0; ui < GetPartCount(); ui++)
	{
		uiOctet = 0;
		if (IsPartComplete(ui))
			uiOctet = 1;
		else
		{
			qwPartBeg = static_cast<uint64>(ui) * PARTSIZE;
			if (IsComplete(qwPartBeg + static_cast<uint64>(0 * (PARTSIZE / 3)), qwPartBeg + static_cast<uint64>(1 * (PARTSIZE / 3) - 1)))
				uiOctet += 2;
			if (IsComplete(qwPartBeg + static_cast<uint64>(1 * (PARTSIZE / 3)), qwPartBeg + static_cast<uint64>(2 * (PARTSIZE / 3) - 1)))
				uiOctet += 4;
			if (IsComplete(qwPartBeg + static_cast<uint64>(2 * (PARTSIZE / 3)), qwPartBeg + static_cast<uint64>(3 * (PARTSIZE / 3) - 1)))
				uiOctet += 8;

			if ((uiFreq = static_cast<unsigned>(m_srcPartFrequencies[ui])) > 42)
				uiOctet += 0xF0;
			else
				uiOctet += (((uiFreq + 2u) / 3u) << 4u);
		}
		result[ui] = static_cast<byte>(uiOctet);
	}
	return result;

	EMULE_CATCH

	return NULL;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::CheckAndAddPastComment(CUpDownClient *pClient)
{
#ifndef NEW_SOCKETS_ENGINE
	if (pClient->IsFileCommentEmpty() && (pClient->GetFileRating() == PF_RATING_NONE))
		return;

	if (pClient->m_pReqPartFile != this)
		return;

	m_pastCommentList.Add(pClient->GetUserHash(), pClient->GetUserName(), GetFileName(), pClient->GetFileComment(), pClient->GetFileRating());
	UpdateFileRatingCommentAvail();
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::RemovePastComment(CUpDownClient *pClient, bool bRestore)
{
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

	if (!pClient)
		return;

	if (bRestore)
	{
		CString	strPastComment;
		EnumPartFileRating	ePastRating;

		if ( (m_pastCommentList.GetCommentRating(pClient->GetUserHash(), &strPastComment, &ePastRating))
		     && (pClient->IsFileCommentEmpty())
		     && (pClient->GetFileRating() == PF_RATING_NONE) )
		{
		//	Restore past comment
			pClient->SetFileComment(strPastComment);
			pClient->SetFileRating(ePastRating);
		}
	}

	m_pastCommentList.Remove(pClient->GetUserHash());

	EMULE_CATCH
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EnumCategories CPartFile::GetCatID() const
{
//	If catID is valid... (valid range is 1..CAT_PREDEFINED-1 but a
//	real check would have to scan the category map. This isn't the
//	best place to do that.
#ifndef VS2002 // In VS2002 the const cast operator causes an ambiguity
	if (static_cast<const _EnumCategories>(m_eCategoryID) >= CAT_PREDEFINED)
#else
	if (const_cast<CPartFile*>(this)->m_eCategoryID >= CAT_PREDEFINED)
#endif
		return CAT_NONE;
	else
		return m_eCategoryID;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::SetCatID(EnumCategories eCatID)
{
	m_eCategoryID = eCatID;

//	Set new prio
	if (IsPartFile())
	{
		int	iPriority = 0;

		if (eCatID != CAT_NONE)
			iPriority = CCat::GetCatByID(GetCatID())->GetPriority();

		switch (iPriority)
		{
			case 1:
				SetAutoPriority(false);
				SetPriority(PR_LOW, false);
				break;

			case 2:
				SetAutoPriority(false);
				SetPriority(PR_NORMAL, false);
				break;

			case 3:
				SetAutoPriority(false);
				SetPriority(PR_HIGH, false);
				break;

			case 4:
				SetAutoPriority(true);
				SetPriority(PR_HIGH, false);
			case 0:
				break;
		}

		if (IsPartFile() && !m_strTempDir.IsEmpty())
			SavePartFile();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::SetStatus(EnumPartFileStatuses in)
{
	m_eStatus = in;

	UpdateGUIAfterStateChange();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::UpdateGUIAfterStateChange()
{
#ifndef NEW_SOCKETS_ENGINE
//	Function can be called from CPartFile destructor on shutdown at that time GUI has been already destroyed
	if (g_App.m_pMDlg == NULL)
		return;

	EnumCategories	eCurCat = g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.GetCurTabCat();

//	Changes in file status can affect whether the file belongs to a predefined category or not
	if (eCurCat >= CAT_PREDEFINED)
		g_App.m_pMDlg->m_wndTransfer.m_ctlDownloadList.ChangeCategoryByID(eCurCat);
	else
		UpdateDisplayedInfo();

//	Update file counts in the tabs to reflect status changes
	g_App.m_pMDlg->m_wndTransfer.UpdateCatTabTitles();
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetRealFileSize() returns real file size on disk with respect to NTFS compression and/or NTFS sparse files
uint64 CPartFile::GetRealFileSize()
{
	return ::GetDiskFileSize(GetFilePath());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsStalled() performs stall check
bool CPartFile::IsStalled() const
{
	bool	bIsStalled = false;

	CTimeSpan	ts = CTime::GetCurrentTime() - GetLastDownTransfer();

//	If WARN_PERIOD_OF_NO_PROGRESS days have gone by without a single download...
//	(??? or maybe sooner if there are no needed parts available?)
	if (ts.GetTotalHours() >= 24 * WARN_PERIOD_OF_NO_PROGRESS)
		bIsStalled = true;

	return bIsStalled;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AllocateNeededSpace() is the preallocate function used to allocate the space needed by a file.
//	It uses a thread to let eMule still work and not drop sources while generating the file.
void CPartFile::AllocateNeededSpace()
{
	m_bIsPreallocated = true;

	AfxBeginThread(AllocateNeededSpaceProc, this, THREAD_PRIORITY_BELOW_NORMAL + g_App.m_pPrefs->GetMainProcessPriority(), 0, 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	AllocateNeededSpaceProc() is the thread created by AllocateNeededSpace().
//	It receives a pointer to 'm_hPartFileWrite' and the filesize-1 (where it has to write the bogus data to allocate space.
UINT CPartFile::AllocateNeededSpaceProc(LPVOID lpParameter)
{
	CPartFile	*pPartFile = reinterpret_cast<CPartFile*>(lpParameter);

	g_App.m_pPrefs->InitThreadLocale();

	try
	{
		pPartFile->m_hPartFileWrite.Seek(pPartFile->GetFileSize() - 1ui64, CFile::begin);
		pPartFile->m_hPartFileWrite.Write("1", 1);
		pPartFile->m_hPartFileWrite.Flush();

		pPartFile->SaveSettingsFile();
	}
	catch (CFileException *error)
	{
		AddLogLine( LOG_FL_SBAR | LOG_RGB_ERROR, IDS_FAILED_PREALLOCATION,
						 pPartFile->GetFileName(), GetErrorMessage(error) );

		pPartFile->m_bIsPreallocated = false;
		error->Delete();
	}

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsLastBlockComplete() returns status of last block of file (if downloaded the file was already allocated).
bool CPartFile::IsLastBlockComplete()
{
	return IsComplete(GetFileSize() - static_cast<uint64>(GetLastBlockSize()), GetFileSize() - 1ui64);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ClosePartFile() closes all .part file handles
void CPartFile::ClosePartFile()
{
	if (m_hPartFileWrite.m_hFile != INVALID_HANDLE_VALUE)
	{
		m_hPartFileRead.Close();
		m_hPartFileWrite.Close();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ReadFileForUpload() opens known file and read information.
//		Params:
//			qwOffset      - file offset to start reading from;
//			dwBytesToRead - number of bytes to read;
//			pbyteBuffer   - buffer to receive data.
//		Return:
//			< 0 in case of error, > 0 - information was read from a part file, else 0.
int CPartFile::ReadFileForUpload(uint64 qwOffset, uint32 dwBytesToRead, byte *pbyteBuffer)
{
	int	iRc = -2;

	EMULE_TRY

//	Synchronization with completion thread
//	will be unlocked on exit
	CSingleLock lockComplete(&m_csFileCompletion, TRUE);

	if (m_eStatus != PS_COMPLETE)
	{
	//	This file isn't complete yet
		m_hPartFileRead.Seek(qwOffset, CFile::begin);
		if (m_hPartFileRead.Read(pbyteBuffer, dwBytesToRead) == dwBytesToRead)
			iRc = 1;
	}
	else
	{
	//	This file is complete and is located in destination directory
		iRc = CKnownFile::ReadFileForUpload(qwOffset, dwBytesToRead, pbyteBuffer);
	}

	EMULE_CATCH

	return iRc;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetFirstLastChunk4Preview() sets Movie Preview Mode by checking media type and needed index size (last 1% of file)
void CPartFile::GetFirstLastChunk4Preview()
{
	EMULE_TRY

	if (GetMovieMode())
		SetMovieMode(0);
	else
	{
		if (IsAviKindMovie())
			SetMovieMode((GetLastPartSize() < static_cast<uint32>(GetFileSize() / 100ui64)) ? 7 : 1);
		else if (IsMpgMovie())
		{
			if (GetFileSize() < 209715200ui64)
				SetMovieMode(4);
			else
				SetMovieMode(5);
		}
		else if (IsArchivePreviewable())	// allow 1st chunk for better recovery
			SetMovieMode(4);
		else
			SetMovieMode(0);
	}
	SaveSettingsFile();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetGapListCopy() returns a protected copy of the gap list
void CPartFile::GetGapListCopy(CArray<Gap_Struct, Gap_Struct> *pGapArray)
{
	Gap_Struct	*pGap;

	CSingleLock lockGap(&m_csGapListAndPartStatus, TRUE);	// will be unlocked on the exit

	pGapArray->SetSize(gaplist.GetCount(), 0);
	pGap = pGapArray->GetData();

	for (POSITION pos = gaplist.GetHeadPosition(); pos != NULL;)
		*pGap++ = *gaplist.GetNext(pos);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::SetFileName(const CString& NewName, bool bClearName)
{
//	Set the name & extension
	CKnownFile::SetFileName(NewName, bClearName);

	CString strExt = GetFileExtension();
	static const TCHAR s_apcPreviewableExt[][4] =
	{
		_T("zip"), _T("rar"), _T("cbz"), _T("cbr")
	};
	static const TCHAR s_apcAviMovieExt[][5] =
	{
		_T("avi"), _T("divx"), _T("xvid")
	};
	static const TCHAR s_apcAviKindMovieExt[][4] =
	{
		_T("ogm"), _T("mkv"), _T("wmv"), _T("mp4")
	};
	static const TCHAR s_apcMpgMovieExt[][5] =
	{
		_T("mpg"), _T("mpeg"), _T("vob"), _T("asf"), _T("rm"), _T("bin"), _T("mpe"), _T("dat")
	};
	static const TCHAR s_apcMpgAudioExt[][4] =
	{
		_T("mp3"), _T("mp2"), _T("mpa")
	};

//	Check previewable status
	m_bIsPreviewableArchive = false;
	for (unsigned ui = 0; ui < ARRSIZE(s_apcPreviewableExt); ui++)
	{
		if (strExt.Compare(reinterpret_cast<const TCHAR*>(&s_apcPreviewableExt[ui])) == 0)
		{
			m_bIsPreviewableArchive = true;
			break;
		}
	}

//	Check movie status
	m_cIsMovie = (ED2KFT_VIDEO == GetFileType() || strExt == _T("bin")) ? -1 : 0;

//	Check AVI movie status
	for (unsigned ui = 0; ui < ARRSIZE(s_apcAviMovieExt); ui++)
	{
		if (strExt.Compare(reinterpret_cast<const TCHAR*>(&s_apcAviMovieExt[ui])) == 0)
		{
			m_cIsMovie = 1;
			break;
		}
	}

//	Check AVI kind movie status
	for (unsigned ui = 0; ui < ARRSIZE(s_apcAviKindMovieExt); ui++)
	{
		if (strExt.Compare(reinterpret_cast<const TCHAR*>(&s_apcAviKindMovieExt[ui])) == 0)
		{
			m_cIsMovie = 2;
			break;
		}
	}

//	Check MPG movie status
	m_bIsMpgMovie = false;
	for (unsigned ui = 0; ui < ARRSIZE(s_apcMpgMovieExt); ui++)
	{
		if (strExt.Compare(reinterpret_cast<const TCHAR*>(&s_apcMpgMovieExt[ui])) == 0)
		{
			m_bIsMpgMovie = true;
			break;
		}
	}

//	Check MPG movie status
	m_bIsMpgAudio = false;
	for (unsigned ui = 0; ui < ARRSIZE(s_apcMpgAudioExt); ui++)
	{
		if (strExt.Compare(reinterpret_cast<const TCHAR*>(&s_apcMpgAudioExt[ui])) == 0)
		{
			m_bIsMpgAudio = true;
			break;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::AddRxCorruptedAmount(uint32 dwBadBytes)
{
	m_qwLostDueToCorruption += dwBadBytes;
	g_App.m_pPrefs->Add2LostFromCorruption(dwBadBytes);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::GetCopySourceList(EnumDLQState eClientDS, ClientList *pCopy, bool bClearSourceList /*= false*/)
{
	if (!pCopy->empty())
		pCopy->clear();

	if ((eClientDS < DS_LAST_QUEUED_STATE) && !m_SourceLists[eClientDS].empty())
	{
		EnterCriticalSection(&m_csSourceLists);

		if (bClearSourceList)
			pCopy->splice(pCopy->end(), m_SourceLists[eClientDS]);
		else
			pCopy->insert(pCopy->begin(), m_SourceLists[eClientDS].begin(), m_SourceLists[eClientDS].end());

		LeaveCriticalSection(&m_csSourceLists);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::GetCopySourceLists(uint32 dwListsMask, ClientList *pCopy, bool bClearSourceLists /*= false*/)
{
	if (!pCopy->empty())
		pCopy->clear();

	EnterCriticalSection(&m_csSourceLists);
	for (EnumDLQState eDS = DS_DOWNLOADING; eDS < DS_LAST_QUEUED_STATE; ++eDS, dwListsMask >>= 1)
	{
		if (((dwListsMask & 1) != 0) && !m_SourceLists[eDS].empty())
		{
			if (bClearSourceLists)
				pCopy->splice(pCopy->end(), m_SourceLists[eDS]);
			else
				pCopy->insert(pCopy->end(), m_SourceLists[eDS].begin(), m_SourceLists[eDS].end());
		}
	}
	LeaveCriticalSection(&m_csSourceLists);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::RemoveClientFromDLSourceList(CUpDownClient *pClient)
{
	EnumDLQState	eDS = pClient->GetDownloadState();

	if (eDS < DS_LAST_QUEUED_STATE)
	{
		uint32	dwListSizeBefore, dwListSizeAfter;

		EnterCriticalSection(&m_csSourceLists);

		dwListSizeBefore = m_SourceLists[eDS].size();
		m_SourceLists[eDS].remove(pClient);
		dwListSizeAfter = m_SourceLists[eDS].size();

		LeaveCriticalSection(&m_csSourceLists);

	//	Update availability information if a client was removed
		if (dwListSizeAfter != dwListSizeBefore)
		{
			NewSrcPartsInfo();
			return true;
		}
	}
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::AddClientToSourceList(CUpDownClient *pClient, EnumDLQState eClientDS)
{
	if (pClient == NULL)
		return;

	if (eClientDS < DS_LAST_QUEUED_STATE)
	{
		EnterCriticalSection(&m_csSourceLists);

		if (find(m_SourceLists[eClientDS].begin(), m_SourceLists[eClientDS].end(), pClient) == m_SourceLists[eClientDS].end())
			m_SourceLists[eClientDS].push_back(pClient);

		LeaveCriticalSection(&m_csSourceLists);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CPartFile::IsClientInSourceList(CUpDownClient *pClient)
{
	bool			bResult = false;
	EnumDLQState	eDS = pClient->GetDownloadState();

	if ((eDS < DS_LAST_QUEUED_STATE) && !m_SourceLists[eDS].empty())
	{
		EnterCriticalSection(&m_csSourceLists);
		ClientList::iterator itResult = find(m_SourceLists[eDS].begin(), m_SourceLists[eDS].end(), pClient);
		bResult = (itResult != m_SourceLists[eDS].end());
		LeaveCriticalSection(&m_csSourceLists);
	}
	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::ClearSourceLists()
{
	EnterCriticalSection(&m_csSourceLists);
	for (EnumDLQState eDS = DS_DOWNLOADING; eDS < DS_LAST_QUEUED_STATE; ++eDS)
		m_SourceLists[eDS].clear();
	LeaveCriticalSection(&m_csSourceLists);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::SwapClientBetweenSourceLists(CUpDownClient *pClient, EnumDLQState eSourceDS, EnumDLQState eTargetDS)
{
//	Speed up swapping (i.e. don't call the related functions if client is outside the lists)

//	Check if we need to remove the client
	if (eSourceDS < DS_LAST_QUEUED_STATE)
	{
		EnterCriticalSection(&m_csSourceLists);
		m_SourceLists[eSourceDS].remove(pClient);
		pClient->ChangeDownloadState(eTargetDS);
	//	Check if we need to add client to another queue
		if (eTargetDS < DS_LAST_QUEUED_STATE)
		{
			m_SourceLists[eTargetDS].push_back(pClient);
			LeaveCriticalSection(&m_csSourceLists);
		}
		else
		{
			LeaveCriticalSection(&m_csSourceLists);

		//	Update file status information if client was removed from the queue
			NewSrcPartsInfo();
		}
	}
	else
	{
	//	Check if we need to add client to another queue
		if (eTargetDS < DS_LAST_QUEUED_STATE)
		{
			EnumPartFileStatuses eStatus = GetStatus();

		//	Don't attach the sources if a file is complete or stopped
			if ((eStatus != PS_COMPLETE) && (eStatus != PS_COMPLETING) && (eStatus != PS_STOPPED))
			{
				EnterCriticalSection(&m_csSourceLists);
				pClient->ChangeDownloadState(eTargetDS);
				m_SourceLists[eTargetDS].push_back(pClient);
				LeaveCriticalSection(&m_csSourceLists);
			}
		}
		else
			pClient->ChangeDownloadState(eTargetDS);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::AddClientToA4AFSourceList(CUpDownClient *pClient)
{
	if (pClient == NULL)
		return;

	EnterCriticalSection(&m_csSourceLists);

	if (find(m_A4AFSourceLists.begin(), m_A4AFSourceLists.end(), pClient) == m_A4AFSourceLists.end())
		m_A4AFSourceLists.push_back(pClient);

	LeaveCriticalSection(&m_csSourceLists);

//	As Process() isn't called for paused files, update A4AF count to display it properly
	m_uSrcA4AF = static_cast<uint16>(m_A4AFSourceLists.size());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::RemoveClientFromA4AFSourceList(CUpDownClient *pClient)
{
	if (!m_A4AFSourceLists.empty())
	{
		EnterCriticalSection(&m_csSourceLists);

		m_A4AFSourceLists.remove(pClient);

		LeaveCriticalSection(&m_csSourceLists);

	//	As Process() isn't called for paused files, update A4AF count to display it properly
		m_uSrcA4AF = static_cast<uint16>(m_A4AFSourceLists.size());
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::GetCopyA4AFSourceList(ClientList *pCopy, bool bClearSourceList /*=false*/)
{
	if (!pCopy->empty())
		pCopy->clear();

	if (!m_A4AFSourceLists.empty())
	{
		EnterCriticalSection(&m_csSourceLists);

		if (bClearSourceList)
			pCopy->splice(pCopy->begin(), m_A4AFSourceLists);
		else
			pCopy->insert(pCopy->begin(), m_A4AFSourceLists.begin(), m_A4AFSourceLists.end());

		LeaveCriticalSection(&m_csSourceLists);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPartFile::ClearA4AFSourceList()
{
	EnterCriticalSection(&m_csSourceLists);
	m_A4AFSourceLists.clear();
	LeaveCriticalSection(&m_csSourceLists);
}
