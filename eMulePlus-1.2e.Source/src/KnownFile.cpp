// parts of this file are based on work from pan One (http://home-3.tiscali.nl/~meost/pms/)
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
#include "updownclient.h"
#include "KnownFile.h"
#include "server.h"
#include "opcodes.h"
#include <io.h>
#include <sys/stat.h>
#ifndef NEW_SOCKETS_ENGINE
	#include "emule.h"
#endif //NEW_SOCKETS_ENGINE
#include "ini2.h"
#include "QArray.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#ifndef NEW_SOCKETS_ENGINE
	#include "Jumpstarter.h"
#endif //NEW_SOCKETS_ENGINE
#include "otherstructs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const uint16 s_auSearchResID[ED2KFT_COUNT] =
{
	IDS_SEARCH_ANY,		// ED2KFT_ANY
	IDS_SEARCH_AUDIO,	// ED2KFT_AUDIO
	IDS_SEARCH_VIDEO,	// ED2KFT_VIDEO
	IDS_SEARCH_PICS,	// ED2KFT_IMAGE
	IDS_SEARCH_PRG,		// ED2KFT_PROGRAM
	IDS_SEARCH_DOC,		// ED2KFT_DOCUMENT
	IDS_SEARCH_ARC,		// ED2KFT_ARCHIVE
	IDS_SEARCH_CDIMG	// ED2KFT_CDIMAGE
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CAbstractFile::~CAbstractFile()
{
	for (int i = 0; i < m_tagArray.GetSize(); i++)
		safe_delete(m_tagArray[i]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::CreateED2kLink() const
{
	TCHAR	acHashStr[MAX_HASHSTR_SIZE];
	CString	strFName(g_App.StripInvalidFilenameChars(GetFileName(), false));	// spaces to dots
	CString	strLink;

	strLink.Format( _T("ed2k://|file|%s|%I64u|%s|/"),
#ifndef NEW_SOCKETS_ENGINE
		g_App.m_pPrefs->GetExportLocalizedLinks() ? strFName : Ed2kURIEncode(strFName),
#else
		GetFileName(),
#endif //NEW_SOCKETS_ENGINE
		GetFileSize(), md4str(GetFileHash(), acHashStr) );

	return strLink;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::CreateED2kSourceLink() const
{
	CString		strLink = CreateED2kLink();
#ifdef OLD_SOCKETS_ENABLED

	if (g_App.m_pServerConnect->IsConnected() && !g_App.m_pServerConnect->IsLowID())
	{
		uint32			dwID = g_App.m_pServerConnect->GetClientID();

		strLink.AppendFormat( _T("|sources,%u.%u.%u.%u:%u|/"),
								static_cast<BYTE>(dwID),
								static_cast<BYTE>(dwID>>8),
								static_cast<BYTE>(dwID>>16),
								static_cast<BYTE>(dwID>>24),
								g_App.m_pPrefs->GetListenPort() );
	}
#endif //OLD_SOCKETS_ENABLED

	return strLink;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::CreateHTMLED2kLink() const
{
	CString		strLink;

	strLink.Format(_T("<a href=\"%s\">%s</a>"), CreateED2kLink(),
#ifndef NEW_SOCKETS_ENGINE
		g_App.StripInvalidFilenameChars(GetFileName(), true) 
#else
		GetFileName()
#endif //NEW_SOCKETS_ENGINE
		);
	return strLink;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileStatistic::CFileStatistic()
{
	partTraffic = NULL;
	partTrafficSession = NULL;

	blockTraffic = NULL;
	blockTrafficSession = NULL;

	partAccepted = NULL;
	partAcceptedSession = NULL;

	completeReleases = 0.0;
	m_iNumRequested = m_iNumAccepted = 0;
	m_dwAllTimeRequested = m_dwAllTimeAccepted = 0;
	m_qwNumTransferred = m_qwAllTimeTransferred = 0;
	fileParent = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFileStatistic::~CFileStatistic()
{
	EMULE_TRY

	if (partTraffic)			delete[] partTraffic;			partTraffic=NULL;
	if (partTrafficSession)		delete[] partTrafficSession;	partTrafficSession=NULL;

	if (partAccepted)			delete[] partAccepted;			partAccepted=NULL;
	if (partAcceptedSession)	delete[] partAcceptedSession;	partAcceptedSession=NULL;

	if (blockTraffic)			delete[] blockTraffic;			blockTraffic=NULL;
	if (blockTrafficSession)	delete[] blockTrafficSession;	blockTrafficSession=NULL;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::AddRequest()
{
	EMULE_TRY

	m_iNumRequested++;
	m_dwAllTimeRequested++;
#ifndef NEW_SOCKETS_ENGINE
	g_App.m_pKnownFilesList->m_iNumRequested++;
	if (::IsWindow(g_App.m_pMDlg->m_wndSharedFiles.m_hWnd))	//fix crash at exit [TwoBottle Mod]
	{
		fileParent->UpdateSharedFileDisplay();
	}
#endif //NEW_SOCKETS_ENGINE
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::AddAccepted()
{
	EMULE_TRY

	m_iNumAccepted++;
	m_dwAllTimeAccepted++;
#ifndef NEW_SOCKETS_ENGINE
	g_App.m_pKnownFilesList->m_iNumAccepted++;
#endif //NEW_SOCKETS_ENGINE
	fileParent->UpdateSharedFileDisplay();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::AddTransferred(uint32 dwBytes)
{
	EMULE_TRY

	m_qwNumTransferred += dwBytes;
	m_qwAllTimeTransferred += dwBytes;
#ifndef NEW_SOCKETS_ENGINE
	g_App.m_pKnownFilesList->m_qwNumTransferred += dwBytes;
#endif //NEW_SOCKETS_ENGINE
	fileParent->UpdateSharedFileDisplay();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CKnownFile::CKnownFile() : m_strComment(_T(""))
{
	m_timetLastWriteDate = 0;
	m_bytePriority = PR_NORMAL;

#ifndef NEW_SOCKETS_ENGINE
	m_bAutoPriority = g_App.m_pPrefs->IsUAPEnabled();
#endif //NEW_SOCKETS_ENGINE

	m_bytePermissions = g_App.m_pPrefs->GetFilePermission();
	statistic.fileParent = this;
	m_bCommentLoaded = false;
	m_byteMoviePreviewMode = 0;
	m_eRating = PF_RATING_NONE;
	m_nCompleteSourcesCount = 0;
	m_nCompleteSourcesCountLo = 0;
	m_nCompleteSourcesCountHi = 0;
	m_nCompleteSourcesTime = 0;

	m_sharedFile = false;
	md4clr(m_fileHash);

	m_Jumpstarter = NULL;
	m_pPartsHashSet = NULL;
	m_dblSizeRatio = 1.0;
	m_bIsCompressedTransferAllowed = true;
	m_bPublishedED2K = false;

	::InitializeCriticalSection(&m_csSourceList);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CKnownFile::~CKnownFile()
{
	EMULE_TRY

	CSingleLock Lock(&m_csHashList, TRUE);
	delete[] m_pPartsHashSet;
	Lock.Unlock();

#ifndef NEW_SOCKETS_ENGINE
	delete m_Jumpstarter;
#endif //NEW_SOCKETS_ENGINE

	::DeleteCriticalSection(&m_csSourceList);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::CalculateCompleteSources()
{
	EMULE_TRY

	uint32	dwCurTick;

#ifndef NEW_SOCKETS_ENGINE
	if ( (!IsPartFile() || IsPaused()) &&
		(((dwCurTick = ::GetTickCount()) - m_nCompleteSourcesTime) >= 2*60*1000) )	// 2 min.
	{
		CQArray<uint16,uint16> count;
		CTypedPtrList<CPtrList, CUpDownClient*> clientuploadlist;
		m_nCompleteSourcesCount= m_nCompleteSourcesCountLo= m_nCompleteSourcesCountHi= 0;
		g_App.m_pClientList->GetClientListByFileID(&clientuploadlist, GetFileHash());
		count.SetSize(0, clientuploadlist.GetSize());
		for (POSITION pos= clientuploadlist.GetHeadPosition(); pos != NULL; )
		{
			CUpDownClient *cur_src= clientuploadlist.GetNext(pos);
			uint16 cur_count= cur_src->GetUpCompleteSourcesCount();
			if ((cur_count) && ((dwCurTick - cur_src->GetUpCompleteSourcesTime()) < 30*60*1000))	// 30 min.
				count.Add(cur_count);
		}

		uint32 n= count.GetSize();
		if (n > 0)
		{
			count.QuickSort();

		//	Calculate range
			uint32 i= n >> 1;		// (n / 2)
			uint32 j= (n * 3) >> 2;	// (n * 3) / 4
			uint32 k= (n * 7) >> 3;	// (n * 7) / 8
			if (n < 5)
			{
				m_nCompleteSourcesCount= count.GetAt(i);
				m_nCompleteSourcesCountLo= 0;
				m_nCompleteSourcesCountHi= m_nCompleteSourcesCount;
			}
			else if (n < 10)
			{
				m_nCompleteSourcesCount= count.GetAt(i);
				m_nCompleteSourcesCountLo= count.GetAt(i - 1);
				m_nCompleteSourcesCountHi= count.GetAt(i + 1);
			}
			else if (n < 20)
			{
				m_nCompleteSourcesCount= count.GetAt(i);
				m_nCompleteSourcesCountLo= count.GetAt(i);
				m_nCompleteSourcesCountHi= count.GetAt(j);
			}
			else
			{
				m_nCompleteSourcesCount= count.GetAt(j);
				m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
				m_nCompleteSourcesCountHi= count.GetAt(k);
			}
		}

		m_nCompleteSourcesTime = dwCurTick;
	}

	UpdateUploadAutoPriority();
	UpdateSharedFileDisplay();
#endif //NEW_SOCKETS_ENGINE
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::UpdateSharedFileDisplay()
{
#ifndef NEW_SOCKETS_ENGINE
	if (g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.m_hWnd != NULL)
	{
		g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.PostMessage( WM_SFL_UPDATEITEM,
																				static_cast<WPARAM>(g_App.m_pMDlg->m_wndSharedFiles.IsWindowVisible()),
																				reinterpret_cast<LPARAM>(this) );
	}
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::SetFileSize(uint64 qwFileSize)
{
	CAbstractFile::SetFileSize(qwFileSize);

	// Examples of parthashs, hashsets and filehashs for different filesizes
	// according the ed2k protocol
	//----------------------------------------------------------------------
	//
	//File size: 3 bytes
	//File hash: 2D55E87D0E21F49B9AD25F98531F3724
	//Nr.hashes: 0
	//
	//
	//File size: 1*PARTSIZE
	//File hash: A72CA8DF7F07154E217C236C89C17619
	//Nr.hashes: 2
	//Hash[  0]: 4891ED2E5C9C49F442145A3A5F608299
	//Hash[  1]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 1*PARTSIZE + 1 byte
	//File hash: 2F620AE9D462CBB6A59FE8401D2B3D23
	//Nr.hashes: 2
	//Hash[  0]: 121795F0BEDE02DDC7C5426D0995F53F
	//Hash[  1]: C329E527945B8FE75B3C5E8826755747
	//
	//
	//File size: 2*PARTSIZE
	//File hash: A54C5E562D5E03CA7D77961EB9A745A4
	//Nr.hashes: 3
	//Hash[  0]: B3F5CE2A06BF403BFB9BFFF68BDDC4D9
	//Hash[  1]: 509AA30C9EA8FC136B1159DF2F35B8A9
	//Hash[  2]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 3*PARTSIZE
	//File hash: 5E249B96F9A46A18FC2489B005BF2667
	//Nr.hashes: 4
	//Hash[  0]: 5319896A2ECAD43BF17E2E3575278E72
	//Hash[  1]: D86EF157D5E49C5ED502EDC15BB5F82B
	//Hash[  2]: 10F2D5B1FCB95C0840519C58D708480F
	//Hash[  3]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 3*PARTSIZE + 1 byte
	//File hash: 797ED552F34380CAFF8C958207E40355
	//Nr.hashes: 4
	//Hash[  0]: FC7FD02CCD6987DCF1421F4C0AF94FB8
	//Hash[  1]: 2FE466AF8A7C06DA3365317B75A5ACFE
	//Hash[  2]: 873D3BF52629F7C1527C6E8E473C1C30
	//Hash[  3]: BCE50BEE7877BB07BB6FDA56BFE142FB
	//

	//   File size       Data parts      ED2K parts      ED2K part hashes
	// ---------------------------------------------------------------
	// 1..PARTSIZE-1     1               1                       0(!)
	// PARTSIZE            1               2(!)                   2(!)
	// PARTSIZE+1        2               2                       2
	// PARTSIZE*2        2               3(!)                   3(!)
	// PARTSIZE*2+1    3               3                       3

	if (qwFileSize == 0)
	{
		m_uPartCount = 0;
		m_dwED2KPartCount = 0;
		m_dwED2KPartHashCount = 0;
		m_dwBlockCount = 0;
		m_dwLastPartSz = 0;
		m_dwLastBlkSz = 0;
		return;
	}

	m_dwLastPartSz = static_cast<uint32>(qwFileSize % PARTSIZE);
	m_dwED2KPartHashCount = static_cast<uint32>(qwFileSize / PARTSIZE);
	m_uPartCount = static_cast<uint16>(m_dwED2KPartHashCount + ((m_dwLastPartSz != 0) ? 1 : 0));
	m_dwLastPartSz = (m_dwLastPartSz != 0) ? m_dwLastPartSz : PARTSZ32;

	m_dwED2KPartCount = m_dwED2KPartHashCount + 1;

	if (m_dwED2KPartHashCount != 0)
		m_dwED2KPartHashCount++;

//	Block traffic
	m_dwLastBlkSz = static_cast<uint32>(qwFileSize % EMBLOCKSIZE);
	m_dwBlockCount = static_cast<uint32>(qwFileSize / EMBLOCKSIZE) + ((m_dwLastBlkSz != 0) ? 1 : 0);
	m_dwLastBlkSz = (m_dwLastBlkSz != 0) ? m_dwLastBlkSz : EMBLOCKSZ32;

//	Calculate size ration (=SmallFilePushRatio)
	if (m_uPartCount < 2u)
	{
		m_dblSizeRatio = PARTSZ32 / static_cast<double>(static_cast<uint32>(qwFileSize));
		if (m_dblSizeRatio > 100.0)
			m_dblSizeRatio = 100.0;
	}

//	Intialize array with required status: all parts shared
//	Size must be m_dwED2KPartCount as WritePartStatus sends also status of "void" chunk when filesize % PARTSIZE = 0
	m_PartsStatusVector.resize(m_dwED2KPartCount, 0x00);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::SetFileName(const CString& NewName, bool bClearName)
{
//	Set the name & extension
	CAbstractFile::SetFileName(NewName, bClearName);
//	Update full file name
	if (!IsPartFile())	// don't update for part file as the name should point at xxx.part
	{
		if (m_strKnownFileDirectory.IsEmpty())	// don't create full name for non-existent known file
			m_strFilePath.Empty();
		else
			m_strFilePath.Format(_T("%s\\%s"), m_strKnownFileDirectory, m_strFileName);	//SetFilePath()
	}

//	Don't try to compress already compressed files
	m_bIsCompressedTransferAllowed = ((GetFileType() != ED2KFT_ARCHIVE) || (m_strFileExtension == _T("tar")));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::CreateFromFile(const CString &strDir, const CString &strFileName, bool bPartFile)
{
	EMULE_TRY

//	First create the filehashset

//	Try to open the specified file
	CFile					file;
	CArray<uchar*,uchar*>	partHashArray;
	uchar					fileHash[16];
	CString					strNameBuffer;

	SetPath(strDir);
	strNameBuffer.Format(_T("%s\\%s"), strDir, strFileName);
	SetFilePath(strNameBuffer);

//	If we failed to open the specified file, return false
	if ( !file.Open(strNameBuffer, CFile::modeRead | CFile::shareDenyNone |
		CFile::typeBinary | CFile::osSequentialScan) )
		return false;

	ULONGLONG qwFileSz = file.GetLength();

	if (qwFileSz > MAX_EMULE_FILE_SIZE)
	{
		file.Close();
		return false;
	}

	AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("File %s is being hashed"), (bPartFile) ? strNameBuffer : strFileName);

	//set member variables
	SetFileSize(qwFileSz);
	SetFileName(strFileName, false);
//	after filesize(m_lFileSize) was defined we can get number of hashes
	const uint32 dwHashCount = GetED2KPartHashCount();

//	Create hashset
	if (dwHashCount > 1)
	{
		//	allocate memory for hash array
		partHashArray.SetSize(dwHashCount);
		//	allocate memory for hashset
		uchar*	pPartsHashSet = new uchar[16*dwHashCount];

		for (uint32 i = 0; i < dwHashCount; i++)
		{
		//	get hash over global pointer
			uchar* pNewPartHash = pPartsHashSet + 16*i;

		//	Create a hash for the next part
			if (i == (dwHashCount - 1))
				CreateHashFromFile(&file, static_cast<uint32>(GetFileSize() - static_cast<uint64>(i) * PARTSIZE), pNewPartHash);
			else
				CreateHashFromFile(&file, PARTSZ32, pNewPartHash);
#ifndef NEW_SOCKETS_ENGINE
			if (!g_App.m_pMDlg->IsRunning())	// in case of shutdown while still hashing
			{
				file.Close();
				delete[] pPartsHashSet;
				return false;
			}
#endif //NEW_SOCKETS_ENGINE
			partHashArray.SetAt(i, pNewPartHash);
		}
		CreateHashFromString(pPartsHashSet, dwHashCount*16, fileHash);

		//copy(add) data from local to global
		CSingleLock Lock(&m_csHashList, TRUE);
		m_pPartsHashSet = pPartsHashSet;
		m_partHashArray.SetSize(dwHashCount);
		for (uint32 i = 0; i < dwHashCount; i++)
			m_partHashArray.SetAt(i, partHashArray[i]);

		md4cpy(m_fileHash, fileHash);
		Lock.Unlock();
	}
	else if (dwHashCount == 0)
	{
	//	Hash creation was split into 2 steps, because fuction CreateHashFromFile()
	//	takes some time to finish hashing. Therefore to prevent possible interference
	//	we create a local instance of file hash & then copy it into global one
		CreateHashFromFile(&file, static_cast<uint32>(GetFileSize()), fileHash);
		md4cpy(m_fileHash, fileHash);
	}

	file.Close();

//	Set last write time
	GetAdjustedFileTime(strNameBuffer, &m_timetLastWriteDate);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadHashsetFromFile() loads the KnownFile with a hashset from the file 'file' in the format of "known.met".
//		If 'bCheckHash' is true, the file hash in 'file' must match a file hash calculated from the part hashes
//		'file'. false is returned on failure, true on success.
bool CKnownFile::LoadHashsetFromFile(CFile &file, bool bCheckHash)
{
//	Needed for memfiles. It's probably better to switch everything to CFile...
	EMULE_TRY

	uchar					FileHash[16];
	uchar* 					pPartsHashSet = NULL;
	CArray<uchar*,uchar*>	partHashArray;
	uint16					uNumParts;

//	Read the file hash if required, or just skip it if we need just check a hash
	if (!bCheckHash)
		file.Read(&m_fileHash,16);			// <HASH> file hash
	else
		file.Seek(16, CFile::current);

//	Read the number of parts
	file.Read(&uNumParts,2);		// <count:WORD> number of parts

	INT_PTR		iParts = static_cast<uint16>(uNumParts);

	if (iParts < 0)		//Cax2 - why? MOREVIT - Yes, really. Why?? Not used elsewhere.
	{
		if (!m_strFileName.IsEmpty() )
		{
		//	TODO: Add some way to tell if the file is really "known.met" or a memfile.
			AddLogLine(LOG_FL_DBG, _T("Loaded hashset size is corrupt in known.met, file: %s!"), m_strFileName);
		}
		else
			AddLogLine(LOG_FL_DBG, _T("Loaded hashset size is corrupt in known.met!"));
	}

	//load hash array in to local buffer
	if (uNumParts == 0)
	{
		if (!bCheckHash)
			return true;	// load was successfully
		else
			return false;	// any check for hash set is impossible
	}
	else		// need to load hashset
	{
	//	If the number of hashes doesn't match the number of hashes the file should have...
		if (bCheckHash && static_cast<uint32>(uNumParts) != GetED2KPartHashCount())
		{
		//	move a pointer to the begin of the file tags
			file.Seek(uNumParts*16, CFile::current);
			return false;
		}

	//	allocate memory for hash array
		partHashArray.SetSize(uNumParts);
	//	allocate memory for hashset
		pPartsHashSet = new uchar[16*uNumParts];

	//	Read the part hashes.
		for (int i = 0; i < static_cast<int>(uNumParts); i++)
		{
			//	get hash over global pointer
			uchar*	pPartHash = pPartsHashSet + 16*i;

			file.Read(pPartHash, 16);		// <HASH>[count] part hashes
			partHashArray.SetAt(i, pPartHash);
		}

	//	recreate a file hash from hash set
		CreateHashFromString(pPartsHashSet, uNumParts*16, FileHash);

	//	If the file hash from 'file' matches the calculated hash...
		if (md4cmp(m_fileHash,FileHash) == 0)
		{
			//lock access to global variables & change them
			CSingleLock Lock(&m_csHashList, TRUE);
			//check if hash set was allready loaded & delete previous one
			delete[] m_pPartsHashSet;
			m_pPartsHashSet = pPartsHashSet;
			m_partHashArray.SetSize(uNumParts);
			for (uint32 i = 0; i < static_cast<int>(uNumParts); i++)
			{
				m_partHashArray.SetAt(i, partHashArray[i]);
			}
			Lock.Unlock();
			return true;
		}
		else
		{
			delete[] pPartsHashSet;
			partHashArray.RemoveAll();
			return false;
		}
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadTagsFromFile() parses what tags we know and stores the rest in 'm_tagArray' so they can be rewritten.
bool CKnownFile::LoadTagsFromFile(CFile &file)
{
	EMULE_TRY

	uint32		dwNumTags;

	file.Read(&dwNumTags, 4);							// {<TAGCOUNT:DWORD>

	for (uint32 j = 0; j < dwNumTags; j++)
	{
		CTag		*newtag = new CTag();

		newtag->FillFromStream(file);
		switch (newtag->GetTagID())
		{
			case FT_FILENAME:							// (FT_FILENAME:string) file name
				if (newtag->IsStr())
				{
#ifdef _UNICODE
					if (IsFileNameEmpty())
#endif
						SetFileName(newtag->GetStringValue());
				}
				break;

			case FT_FILESIZE:							// (FT_FILESIZE:int) file size (bytes)
				if (newtag->IsAnyInt())
					SetFileSize(newtag->GetInt64Value());
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

			case FT_ULPRIORITY:							// (FT_ULPRIORITY:int) upload priority
				if (newtag->IsInt())
				{
					m_bytePriority = static_cast<byte>(newtag->GetIntValue());
					if (m_bytePriority == PR_AUTO)
					{
						SetAutoULPriority(true);
						SetULPriority(PR_RELEASE);
					}
					else
					{
						SetAutoULPriority(false);
						if ( (m_bytePriority != PR_VERYLOW) && (m_bytePriority != PR_LOW) &&
							(m_bytePriority != PR_NORMAL) && (m_bytePriority != PR_HIGH) && (m_bytePriority != PR_RELEASE) )
						{
							m_bytePriority = PR_NORMAL;
						}
					}
				}
				break;

			case FT_PERMISSIONS:						// (FT_PERMISSIONS:int) upload permissions
				if (newtag->IsInt() && (newtag->GetIntValue() <= PERM_NOONE))
					m_bytePermissions = static_cast<byte>(newtag->GetIntValue());
				break;

			case FT_PARTFILENAME:	//	This tag is used to keep part file name, delete it as file's already complete
				break;

			default:
				m_tagArray.Add(newtag);
				continue;
		}
		delete newtag;
	}
	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::LoadDateFromFile(CFile& file)
{
	EMULE_TRY

	file.Read(&m_timetLastWriteDate,4);
	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::LoadFromFile(CFile& file)
{
	EMULE_TRY

	bool		result, hashset_ok;

	result = LoadDateFromFile(file);				// <DATE:time_t> last write date/time
	hashset_ok = LoadHashsetFromFile(file,false);	// (HASHSET)
	result = result && LoadTagsFromFile(file);		// <Tag_set>
	result = result && hashset_ok && (GetED2KPartHashCount() == static_cast<uint32>(m_partHashArray.GetCount()));

//	Cax2 - if m_strFileName==NULL, don't report it, it should still be ok...
#ifndef NEW_SOCKETS_ENGINE
	if(!hashset_ok && !m_strFileName.IsEmpty())
		AddLogLine(LOG_RGB_WARNING, IDS_ERR_KNOWNHASHCORR, m_strFileName);
#endif //NEW_SOCKETS_ENGINE

#ifndef NEW_SOCKETS_ENGINE
	if (result && CJumpstarter::ShouldBeEnabledForFile(this))
	{
		m_Jumpstarter = new CJumpstarter(this);
		AddLogLine(0, IDS_JS_ENABLED, m_strFileName);
	}
#endif //NEW_SOCKETS_ENGINE

	return result;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::WriteToFile(CFile &file)
{
	byte		abyteBuf[22];
	CSingleLock	Lock(&m_csHashList, TRUE);
	CWrTag		tagWr;

	uint32	dwNumParts = m_partHashArray.GetCount();

	if ((dwNumParts != 0) && (m_pPartsHashSet == NULL))
		dwNumParts = 0;

	POKE_DWORD(&abyteBuf[0], m_timetLastWriteDate);				// <DATE:time_t> last write date/time
	md4cpy(&abyteBuf[4], m_fileHash);							// <HASH>
	POKE_WORD(&abyteBuf[20], static_cast<uint16>(dwNumParts));	// <count:WORD> number of chunk hashes

	file.Write(abyteBuf, sizeof(abyteBuf));
	if (m_pPartsHashSet != NULL)
		file.Write(m_pPartsHashSet, 16 * dwNumParts);			// <HASH>[count] part hashes

	Lock.Unlock();

//	Tags
	uint32	dwTagFilePos = static_cast<uint32>(file.GetPosition());
	uint32	dwTmp, dwTagCnt = 0;

	file.Write(&dwTagCnt, 4);									// <TAGCOUNT:DWORD>

	if (IsUTF8Required(GetFileName()))
	{
		tagWr.WriteToFile(FT_FILENAME, GetFileName(), file, cfUTF8withBOM);		// (FT_FILENAME:string) file name
		dwTagCnt++;
	}
	tagWr.WriteToFile(FT_FILENAME, GetFileName(), file);		// (FT_FILENAME:string) file name
	dwTagCnt++;
	tagWr.WriteToFile(FT_FILESIZE, GetFileSize(), file, IsLargeFile());
	dwTagCnt++;
	if (statistic.GetAllTimeTransferred() != 0)
	{
		tagWr.WriteToFile(FT_ATTRANSFERRED, static_cast<uint32>(statistic.GetAllTimeTransferred()), file);
		if ((dwTmp = static_cast<uint32>(statistic.GetAllTimeTransferred() >> 32)) != 0)
		{
			tagWr.WriteToFile(FT_ATTRANSFERREDHI, dwTmp, file);
			dwTagCnt++;
		}
		dwTagCnt++;
	}
	if (statistic.GetAllTimeRequests() != 0)
	{
		tagWr.WriteToFile(FT_ATREQUESTED, statistic.GetAllTimeRequests(), file);
		dwTagCnt++;
	}
	if (statistic.GetAllTimeAccepts() != 0)
	{
		tagWr.WriteToFile(FT_ATACCEPTED, statistic.GetAllTimeAccepts(), file);
		dwTagCnt++;
	}
	tagWr.WriteToFile(FT_ULPRIORITY, IsULAutoPrioritized() ? PR_AUTO : m_bytePriority, file);
	dwTagCnt++;
	tagWr.WriteToFile(FT_PERMISSIONS, m_bytePermissions, file);
	dwTagCnt++;

//	Unknown tags
	for (int j = 0; j < m_tagArray.GetCount(); j++)
	{
		if (m_tagArray[j]->IsStr() || m_tagArray[j]->IsInt())
		{
			m_tagArray[j]->WriteToFile(file);
			dwTagCnt++;
		}
	}
//	Save valid tag count
	file.Seek(dwTagFilePos, CFile::begin);
	file.Write(&dwTagCnt, 4);
	file.SeekToEnd();

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CreateHashFromInput() generates a hash from the next 'dwLength' bytes of 'file' or 'pbyteMem'
//		The hash is returned in '*pbyteHash'.
void CKnownFile::CreateHashFromInput(CFile *file, uint32 dwLength, uchar *pbyteHash, uchar *pbyteMem)
{
	uint32 *pdwHash = reinterpret_cast<uint32*>(pbyteHash);

	pdwHash[0] = 0x67452301;
	pdwHash[1] = 0xEFCDAB89;
	pdwHash[2] = 0x98BADCFE;
	pdwHash[3] = 0x10325476;

	uint32		dwRequired = dwLength;
	uchar		pbyteBuff[1024];	//	should be small enough to eliminate stack size check

	if (pbyteMem != NULL)
	{
	//	Process memory block
		while (dwRequired >= 64)
		{
			MD4Transform(pdwHash, reinterpret_cast<uint32*>(pbyteMem));
			pbyteMem += 64;
			dwRequired -= 64;
		}
	//	Copy the rest
		if (dwRequired != 0)
			memcpy2(pbyteBuff, pbyteMem, dwRequired);
	}
	else
	{
	//	Process a file
		uint32		dwBlockSz = 16 * 4096;
		void		*pAlloc = malloc(dwBlockSz);
		uchar		*pbytePtr = reinterpret_cast<uchar*>(pAlloc);

		if (pbytePtr == NULL)
		{
		//	Use small local buffer in case of low memory
			pbytePtr = pbyteBuff;
			dwBlockSz = sizeof(pbyteBuff);
		}

		while (dwRequired >= dwBlockSz)
		{
			file->Read(pbytePtr, dwBlockSz);
			uint32 i = 0;
			do
			{
				MD4Transform(pdwHash, (uint32*)(pbytePtr + i));
				i += 64;
			} while(i < dwBlockSz);
			dwRequired -= dwBlockSz;
		}
		if (dwRequired >= 64)
		{
			uint32 dwLen = dwRequired & ~63;

			file->Read(pbytePtr, dwLen);
			uint32 i = 0;
			do
			{
				MD4Transform(pdwHash, reinterpret_cast<uint32*>(pbytePtr + i));
				i += 64;
			} while(i < dwLen);
			dwRequired -= dwLen;
		}
	//	Read the rest
		if (dwRequired != 0)
			file->Read(pbyteBuff, dwRequired);
		if (pAlloc != NULL)
			free(pAlloc);
	}
	// in byte scale 512 = 64, 448 = 56
	pbyteBuff[dwRequired++] = 0x80;
	if (dwRequired > 56)
	{
		memzero(&pbyteBuff[dwRequired], 64 - dwRequired);
		MD4Transform(pdwHash, reinterpret_cast<uint32*>(pbyteBuff));
		dwRequired = 0;
	}
	memzero(&pbyteBuff[dwRequired], 56 - dwRequired);
// Add size (convert to bits)
	*reinterpret_cast<uint32*>(&pbyteBuff[56]) = (dwLength << 3);
	*reinterpret_cast<uint32*>(&pbyteBuff[60]) = (dwLength >> 29);

	MD4Transform(pdwHash, reinterpret_cast<uint32*>(pbyteBuff));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const uchar* CKnownFile::GetPartHash(uint32 dwPart) const
{
	EMULE_TRY
	if (dwPart >= static_cast<uint32>(m_partHashArray.GetCount()))
		return NULL;

	return m_partHashArray[dwPart];

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CKnownFile::GetPopularityRatio()
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	if (g_App.m_pKnownFilesList->GetTotalRequested()==0)
		return 1.0;
	if (statistic.GetRequests() == 0)
		return 2.0;

	double		dblRatio;

	dblRatio = (static_cast<double>(statistic.GetRequests())) / (static_cast<double>(g_App.m_pKnownFilesList->GetTotalRequested()));
	dblRatio = -4.0 * log10(static_cast<double>(g_App.m_pSharedFilesList->GetCount())) * dblRatio;
	dblRatio = 2.0 * exp(dblRatio);

	if (dblRatio < 1.0)
	{
		dblRatio = 1.0;
	}
	else if (dblRatio > 2.0)
	{
		dblRatio = 2.0;
	}

	return dblRatio;
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH

	return 1.0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static void __fastcall MD4Transform(uint32 Hash[4], uint32 x[16])
{
	uint32 a = Hash[0];
	uint32 b = Hash[1];
	uint32 c = Hash[2];
	uint32 d = Hash[3];

	/* Round 1 */
	MD4_FF(a, b, c, d, x[ 0], S11); // 01
	MD4_FF(d, a, b, c, x[ 1], S12); // 02
	MD4_FF(c, d, a, b, x[ 2], S13); // 03
	MD4_FF(b, c, d, a, x[ 3], S14); // 04
	MD4_FF(a, b, c, d, x[ 4], S11); // 05
	MD4_FF(d, a, b, c, x[ 5], S12); // 06
	MD4_FF(c, d, a, b, x[ 6], S13); // 07
	MD4_FF(b, c, d, a, x[ 7], S14); // 08
	MD4_FF(a, b, c, d, x[ 8], S11); // 09
	MD4_FF(d, a, b, c, x[ 9], S12); // 10
	MD4_FF(c, d, a, b, x[10], S13); // 11
	MD4_FF(b, c, d, a, x[11], S14); // 12
	MD4_FF(a, b, c, d, x[12], S11); // 13
	MD4_FF(d, a, b, c, x[13], S12); // 14
	MD4_FF(c, d, a, b, x[14], S13); // 15
	MD4_FF(b, c, d, a, x[15], S14); // 16

	/* Round 2 */
	MD4_GG(a, b, c, d, x[ 0], S21); // 17
	MD4_GG(d, a, b, c, x[ 4], S22); // 18
	MD4_GG(c, d, a, b, x[ 8], S23); // 19
	MD4_GG(b, c, d, a, x[12], S24); // 20
	MD4_GG(a, b, c, d, x[ 1], S21); // 21
	MD4_GG(d, a, b, c, x[ 5], S22); // 22
	MD4_GG(c, d, a, b, x[ 9], S23); // 23
	MD4_GG(b, c, d, a, x[13], S24); // 24
	MD4_GG(a, b, c, d, x[ 2], S21); // 25
	MD4_GG(d, a, b, c, x[ 6], S22); // 26
	MD4_GG(c, d, a, b, x[10], S23); // 27
	MD4_GG(b, c, d, a, x[14], S24); // 28
	MD4_GG(a, b, c, d, x[ 3], S21); // 29
	MD4_GG(d, a, b, c, x[ 7], S22); // 30
	MD4_GG(c, d, a, b, x[11], S23); // 31
	MD4_GG(b, c, d, a, x[15], S24); // 32

	/* Round 3 */
	MD4_HH(a, b, c, d, x[ 0], S31); // 33
	MD4_HH(d, a, b, c, x[ 8], S32); // 34
	MD4_HH(c, d, a, b, x[ 4], S33); // 35
	MD4_HH(b, c, d, a, x[12], S34); // 36
	MD4_HH(a, b, c, d, x[ 2], S31); // 37
	MD4_HH(d, a, b, c, x[10], S32); // 38
	MD4_HH(c, d, a, b, x[ 6], S33); // 39
	MD4_HH(b, c, d, a, x[14], S34); // 40
	MD4_HH(a, b, c, d, x[ 1], S31); // 41
	MD4_HH(d, a, b, c, x[ 9], S32); // 42
	MD4_HH(c, d, a, b, x[ 5], S33); // 43
	MD4_HH(b, c, d, a, x[13], S34); // 44
	MD4_HH(a, b, c, d, x[ 3], S31); // 45
	MD4_HH(d, a, b, c, x[11], S32); // 46
	MD4_HH(c, d, a, b, x[ 7], S33); // 47
	MD4_HH(b, c, d, a, x[15], S34); // 48

	Hash[0] += a;
	Hash[1] += b;
	Hash[2] += c;
	Hash[3] += d;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Updates priority of file if autopriority is activated
void CKnownFile::UpdateUploadAutoPriority(void)
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	if (IsULAutoPrioritized() && !IsCompleting())
	{
		if (IsPartFile() && g_App.m_pDownloadQueue && !IsPaused())
		{
			CPartFile		*pPartFile = dynamic_cast<CPartFile*>(this);

			if(pPartFile != NULL)
			{
				int iValidSources = pPartFile->GetSourceCount() - pPartFile->GetNotCurrentSourcesCount();

				if (iValidSources < RARE_FILE)
					SetULPriority(PR_RELEASE);
				else if (iValidSources < 200)
					SetULPriority(PR_HIGH);
				else if (iValidSources < 400)
					SetULPriority(PR_NORMAL);
				else
					SetULPriority(PR_LOW);
			}
		}
		else
		{
			int iCompleteSources = GetCompleteSourcesCount();	// used KnownFile's method

			if (iCompleteSources < 5)
				SetULPriority(PR_RELEASE);
			else if (iCompleteSources < RARE_FILE)
				SetULPriority(PR_HIGH);
			else if (iCompleteSources < 200)
				SetULPriority(PR_NORMAL);
			else
				SetULPriority(PR_LOW);
		}
	}
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAbstractFile::SetFileName(const CString& NewName, bool bClearName)
{
	m_strFileName = NewName;

	if (!m_strFileName.IsEmpty())
	{
		m_eFileType = GetED2KFileTypeID(m_strFileName);

		int		iIdxExtension = m_strFileName.ReverseFind(_T('.'));

		if (iIdxExtension != -1)
		{
			m_strFileExtension = m_strFileName.Mid(iIdxExtension + 1);
			m_strFileExtension.MakeLower();
		}
		else
			m_strFileExtension.Empty();
	}
	else
	{
		m_eFileType = ED2KFT_ANY;
		m_strFileExtension.Empty();
	}

	if (bClearName)
	{
		static const TCHAR s_acRepTbl[] =
		{
			_T('/'), _T('>'), _T('<'), _T('*'), _T(':'), _T('?'), _T('|'), _T('\"'), _T('\\')
		};

		for (unsigned ui = 0; ui < _countof(s_acRepTbl); ui++)
			m_strFileName.Replace(s_acRepTbl[ui], _T('-'));
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CAbstractFile::GetFileTypeString()
{
#ifndef NEW_SOCKETS_ENGINE
	unsigned	uiType = m_eFileType;

	if (uiType >= ED2KFT_COUNT)
		uiType = ED2KFT_ANY;

	return GetResString(s_auSearchResID[uiType]);
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CAbstractFile::CmpFileTypes(uint32 dwType2) const
{
#ifndef NEW_SOCKETS_ENGINE
	uint32	dwType1 = m_eFileType;

	if (dwType1 == dwType2)	//	the same type, no need to compare strings
		return 0;

	if (dwType1 >= ED2KFT_COUNT)
		dwType1 = ED2KFT_ANY;
	if (dwType2 >= ED2KFT_COUNT)
		dwType2 = ED2KFT_ANY;

//	As all types are predefined it's safe to use case sensitive compare to speed up
	return _tcscmp(GetResString(s_auSearchResID[dwType1]), GetResString(s_auSearchResID[dwType2]));
#else
	return 0;
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LPCSTR CAbstractFile::GetSearchFileType(uint32 *pdwType)
{
#ifndef NEW_SOCKETS_ENGINE
	static const char *const s_apcSearchTypes[] =
	{
		"",			// ED2KFT_ANY
		"Audio",	// ED2KFT_AUDIO
		"Video",	// ED2KFT_VIDEO
		"Image",	// ED2KFT_IMAGE
		"Pro",		// ED2KFT_PROGRAM, ED2KFT_ARCHIVE, ED2KFT_CDIMAGE
		"Doc"		// ED2KFT_DOCUMENT
	};
	unsigned	uiType = m_eFileType;

	if ((uiType == ED2KFT_ARCHIVE) || (uiType == ED2KFT_CDIMAGE))
		uiType = ED2KFT_PROGRAM;
	if (uiType >= ARRSIZE(s_apcSearchTypes))
		uiType = ED2KFT_ANY;
	*pdwType = uiType;

	return s_apcSearchTypes[uiType];
#else
	return "";
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet* CKnownFile::CreateSrcInfoPacket(const CUpDownClient *pForClient, byte byteRequestedVer, uint16 uRequestedOpt)
{
	NOPRM(uRequestedOpt);	// we don't support any special SX2 options yet, reserved for later use
#ifndef NEW_SOCKETS_ENGINE
	EMULE_TRY

// don't send the sources to the banned clients in order to save our bandwidth & don't help them to hurt the network
// note that at same time the banned clients will be sent to the normal clients, which can decide how to handle those banned sources 
	if (pForClient->IsBanned())
		return NULL;

	ClientList SourceList;

	GetCopySourceList(&SourceList);

	if (SourceList.empty())
		return NULL;

	CMemFile	packetStream(1024);
	uint32		dwCount = 0;
	byte		byteUsedVer;
	bool		bValidSource, bIsSX2Packet;

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

	packetStream.Write(pForClient->m_reqFileHash, 16);
	packetStream.Write(&dwCount, 2);

	CUpDownClient  *pPotentialSource;
	uint32			dwID, dwSrvIP, dwRes, dwCurTime = ::GetTickCount();
	const byte		*pbyteForClientPartStatus, *pbytePotentialClientPartStatus;
	uint16			uPort, uSrvPort;

	for (ClientList::const_iterator cIt = SourceList.begin(); cIt != SourceList.end(); cIt++)
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
	//	Don't send a client that did only one request to prevent spreading of unstable clients
		if (pPotentialSource->GetAskedCount() < 2)
			continue;
	//	Don't send a client that did not request any file during MAX_PURGEQUEUETIME, because client is offline
		if (dwCurTime > (pPotentialSource->GetLastUpRequest() + MAX_PURGEQUEUETIME))
			continue;
	//	Check LowID status
		if (pPotentialSource->HasLowID())
		{
		//	Don't send a LowID client to LowID as they won't be able to connect
			if (pForClient->HasLowID())
				continue;
		//	Don't send a LowID client if server IP & server port are unknown
			if (pPotentialSource->GetServerIP() == 0 || pPotentialSource->GetServerPort() == 0)
				continue;
		//	Don't send a LowID client to HighID if client is on different server
			if ( pPotentialSource->GetServerIP() != pForClient->GetServerIP()
				|| pPotentialSource->GetServerPort() != pForClient->GetServerPort() )
				continue;
		}
	//	Don't share Lan clients sources as they are private ips/userids
		if (pPotentialSource->IsOnLAN())
			continue;

		bValidSource = true;
		pbytePotentialClientPartStatus = pPotentialSource->GetUpPartStatus();
	// check if potential source reported parts info with same file hash
		if (!md4cmp(pPotentialSource->m_reqFileHash, GetFileHash()) && pbytePotentialClientPartStatus != NULL)
		{
		//	Don't send clients that have no parts available
			if (pPotentialSource->GetAvailUpPartCount() == 0 && pForClient->GetUpPartCount() > 1)
				continue;
			pbyteForClientPartStatus = pForClient->GetUpPartStatus();
		// check if target client also reported parts status
			if (pbyteForClientPartStatus != NULL)
			{
			//	Don't send the clients that support the upload parts statuses & have different number of the parts
				if (pForClient->GetUpPartCount() != pPotentialSource->GetUpPartCount())
					continue;

			// Both clients support part statuses so we gonna check & exclude NNS source for target client
				bValidSource = false;
				for (uint32 i = 0; i < pForClient->GetUpPartCount(); i++)
				{
					if(pbytePotentialClientPartStatus[i] && !pbyteForClientPartStatus[i])
					{
						bValidSource = true;
						break;
					}
				}
			}
		}

		if (bValidSource)
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

	Packet	*pPacket = new Packet(&packetStream,OP_EMULEPROT);

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
//	For File Comment
void CKnownFile::LoadComment()
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	CString		strFullPath;
	unsigned	uiRate;

	strFullPath.Format(_T("%sfileinfo.ini"), g_App.m_pPrefs->GetConfigDir());

	CIni ini(strFullPath, INI_MODE_READONLY);

	ini.SetDefaultCategory(HashToString(m_fileHash));
	m_strComment = ini.GetString(_T("Comment"), _T("")).Left(MAXFILECOMMENTLEN);
	uiRate = ini.GetInt(_T("Rate"), PF_RATING_NONE);
	if (uiRate > PF_RATING_EXCELLENT)
		uiRate = PF_RATING_NONE;
	m_eRating = static_cast<_EnumPartFileRating>(uiRate);
	m_bCommentLoaded = true;

	ini.CloseWithoutSave();
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::RemoveFileCommentAndRating()
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	CString strFullPath;

	strFullPath.Format(_T("%sfileinfo.ini"),g_App.m_pPrefs->GetConfigDir());

	CIni ini(strFullPath);

	ini.DeleteCategory(HashToString(m_fileHash));
	ini.SaveAndClose();

	m_strComment.Empty();
	m_eRating = PF_RATING_NONE;

	if (IsPartFile())
		((CPartFile*)this)->UpdateFileRatingCommentAvail();
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::SetFileComment(const CString &strNewComment)
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
	CString	strFullPath;

	strFullPath.Format(_T("%sfileinfo.ini"), g_App.m_pPrefs->GetConfigDir());

	CIni ini(strFullPath);

	ini.SetDefaultCategory(HashToString(m_fileHash));

	ini.SetString(_T("Comment"), strNewComment);
	m_strComment = strNewComment;

	ini.SaveAndClose();

	CTypedPtrList<CPtrList, CUpDownClient*> srclist;

	g_App.m_pUploadQueue->FindSourcesForFileById(&srclist, GetFileHash());

	CUpDownClient	*pClient;

	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL; )
	{
		pClient = srclist.GetNext(pos);
		pClient->SetCommentDirty();
	}

	if (IsPartFile())
		((CPartFile*)this)->SetHasComment(true);
#endif //NEW_SOCKETS_ENGINE

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	For File rate
void CKnownFile::SetFileRating(EnumPartFileRating eNewRating)
{
	EMULE_TRY

#ifndef NEW_SOCKETS_ENGINE
//	Avoid unrequired saving and server republish if rating is still the same
	if (m_eRating != eNewRating)
	{
		CString strFullPath(g_App.m_pPrefs->GetConfigDir());

		strFullPath += _T("fileinfo.ini");

		CIni ini(strFullPath);

		ini.SetDefaultCategory(HashToString(m_fileHash));

		ini.SetInt(_T("Rate"), eNewRating);
		m_eRating = eNewRating;

		ini.SaveAndClose();

	//	Republish a file on the server (if possible) to update server file rating
	//	lugdunummaster: server handles the republishing of a file, taking into account
	//	a change in the rating (but if an already rated file is republished without a rating,
	//	no change is done on the rating - server keeps the previous rating for this file and client)
		if (eNewRating != PF_RATING_NONE)
			g_App.m_pSharedFilesList->RepublishFile(this, SRV_TCPFLG_TYPETAGINTEGER/*check feature support*/);

		CTypedPtrList<CPtrList, CUpDownClient*> srclist;
		g_App.m_pUploadQueue->FindSourcesForFileById(&srclist, GetFileHash());

		CUpDownClient	*pClient;

		for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
		{
			pClient = srclist.GetNext(pos);
			pClient->SetCommentDirty();
		}

		if (IsPartFile())
			((CPartFile*)this)->SetHasRating(true);
	}
#endif //NEW_SOCKETS_ENGINE
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::initTraffic()
{
	EMULE_TRY

	if(partTraffic==NULL)
	{
		uint16 p=fileParent->GetPartCount();
		partTraffic=new uint32[p];
		partTrafficSession=new uint32[p];
		for(uint16 i=0; i<p; i++)
		{
			partTraffic[i]=0;
			partTrafficSession[i]=0;
		}
	}

	if(partAccepted==NULL)
	{
		uint16 p=fileParent->GetPartCount();
		partAccepted=new uint32[p];
		partAcceptedSession=new uint32[p];
		for(uint16 i=0; i<p; i++)
		{
			partAccepted[i]=0;
			partAcceptedSession[i]=0;
		}
	}

	if(blockTraffic==NULL)
	{
		uint32 p=fileParent->GetBlockCount();
		blockTraffic=new uint32[p];
		blockTrafficSession=new uint32[p];
		for(uint32 i=0; i<p; i++)
		{
			blockTraffic[i]=0;
			blockTrafficSession[i]=0;
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::recalcCompleteReleases()
{
//	Recalculate complete releases factor
	uint32	dwBlocks = fileParent->GetBlockCount();
	uint32	s, m = 0xFFFFFFFF;
	double	dMin;

//	Find lowest parttraffic
	for (uint32 i = 0; i < dwBlocks; i++)
	{
		if (blockTraffic[i] == 0)
			continue;
		s = fileParent->GetBlockSize(i);

		uint32 f = (blockTraffic[i] + s - 1u) / s;

		if (f < m)
			m = f;
	}

//	Get average
	dMin = static_cast<double>(m);
	completeReleases = 0;
	for (uint32 i = 0; i < dwBlocks; i++)
	{
		if (blockTraffic[i] == 0)
			continue;
		s = fileParent->GetBlockSize(i);

		double f = static_cast<double>(blockTraffic[i]) / s;

		if (f > dMin)
			f = dMin;
		completeReleases += f;
	}

	completeReleases /= dwBlocks;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	This function may not return a 100% accurate value, because you can't fill a part with blocks
double CFileStatistic::GetCompletePartReleases(uint16 part)
{
//	Check part
	if((partTraffic == NULL) || (part >= fileParent->GetPartCount()) || (partTraffic[part] == 0))
		return 0;

//	Start and end block
	uint32	dwStartBlk = static_cast<uint32>(static_cast<uint64>(part) * PARTSIZE / EMBLOCKSIZE);
	uint32	dwEndBlk = static_cast<uint32>((static_cast<uint64>(part) * PARTSIZE + PARTSIZE - 1ui64) / EMBLOCKSIZE);

	if (dwEndBlk >= fileParent->GetBlockCount())
		dwEndBlk = fileParent->GetBlockCount() - 1;

//	Find lowest parttraffic
	uint32	s, m = 0xFFFFFFFF;
	double	dMin, completePartReleases = 0;
	for (uint32 i = dwStartBlk; i <= dwEndBlk; i++)
	{
		if (blockTraffic[i] == 0)
			continue;
		s = fileParent->GetBlockSize(i);

		uint32 f = (blockTraffic[i] + s - 1u) / s;

		if (f < m)
			m = f;
	}

//	Get avg.
	dMin = static_cast<double>(m);
	for (uint32 i = dwStartBlk; i <= dwEndBlk; i++)
	{
		if (blockTraffic[i] == 0)
			continue;
		s = fileParent->GetBlockSize(i);

		double f = static_cast<double>(blockTraffic[i]) / s;
		if (f > dMin)
			f = dMin;
		completePartReleases += f;
	}

	return completePartReleases * EMBLOCKSZ32 / fileParent->GetPartSize(part);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::AddTraffic(uint32 dwPart, const uint64 &qwStart, uint32 dwBytes)
{
	EMULE_TRY

	initTraffic();

	sint32	btg = static_cast<sint32>(dwBytes);

	uint32 p1 = min(static_cast<uint32>((static_cast<uint64>(dwPart) + 1ui64) * PARTSIZE - qwStart), dwBytes);
	partTraffic[dwPart] += p1;
	partTrafficSession[dwPart] += p1;
	partAccepted[dwPart]++;
	partAcceptedSession[dwPart]++;
#if 0	// we upload only within one chunck!
	btg-=p1;
	dwPart++;
	while(btg > 0)
	{
		partTraffic[dwPart] += btg % PARTSZ32;
		partTrafficSession[dwPart] += btg % PARTSZ32;
		partAccepted[dwPart]++;
		partAcceptedSession[dwPart]++;
		dwPart++;
		btg -= PARTSZ32;
	}
#endif

//	Start block
	uint32	block = static_cast<uint32>(qwStart / EMBLOCKSIZE);
	btg = static_cast<sint32>(dwBytes);

	uint32 b1 = min(static_cast<uint32>((static_cast<uint64>(block) + 1ui64) * EMBLOCKSIZE - qwStart), dwBytes);
	blockTraffic[block]+=b1;
	blockTrafficSession[block]+=b1;
	btg-=b1;
	block++;
	while(btg>0)
	{
		blockTraffic[block] += btg % EMBLOCKSZ32;
		blockTrafficSession[block] += btg % EMBLOCKSZ32;
		block++;
		btg -= EMBLOCKSZ32;
	}

	recalcCompleteReleases();

	fileParent->UpdateSharedFileDisplay();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::SaveToFileTraffic(FILE* file)
{
	EMULE_TRY

	if(statistic.partTraffic)
	{
		uint16 parts=GetPartCount();
		if(fwrite(&parts, sizeof(parts), 1, file)!=1) return false;
		if(fwrite(statistic.partTraffic, 4, parts, file)!=parts) return false;

		uint32 blocks=GetBlockCount();
		if(fwrite(&blocks, sizeof(blocks), 1, file)!=1) return false;
		if(fwrite(statistic.blockTraffic, 4, blocks, file)!=blocks) return false;

		if(fwrite(statistic.partAccepted, 4, parts, file)!=parts) return false;
	}

//	This block shouldnt be used at all
	else
	{
		uint32	d=0;

		uint16 parts=GetPartCount();
		if(fwrite(&parts, sizeof(parts), 1, file)!=1) return false;
		for(uint16 part=0; part<parts; part++)
			if(fwrite(&d, sizeof(d), 1, file)!=1) return false;

		uint32 blocks=GetBlockCount();
		if(fwrite(&blocks, sizeof(blocks), 1, file)!=1) return false;
		for(uint32 block=0; block<blocks; block++)
			if(fwrite(&d, sizeof(d), 1, file)!=1) return false;

		for(uint16 part=0; part<parts; part++)
			if(fwrite(&d, sizeof(d), 1, file)!=1) return false;
	}

	return (ferror(file)) ? false : true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::LoadFromFileTraffic(FILE* file, byte version)
{
	EMULE_TRY

	// read parts
	uint16 parts=0;
	if(fread(&parts, sizeof(parts), 1, file)!=1) return false;

	// does part count match?
	if (parts != GetPartCount())
		return false;

	// get the memory
	statistic.initTraffic();

	// load all the parts traffic data
	if(fread(statistic.partTraffic, 4, parts, file)!=parts) return false;

	// read blocks
	uint32 blocks=0;
	if(fread(&blocks, sizeof(blocks), 1, file)!=1) return false;

	// does blocks count match?
	if (blocks != GetBlockCount())
		return false;

	// load all the parts traffic data
	if(fread(statistic.blockTraffic, 4, blocks, file)!=blocks) return false;

//	Versioning
	if(version>1)
	{
	//	Version 2 introduced part-accepted counter
		if(fread(statistic.partAccepted, 4, parts, file)!=parts) return false;
	}

//	Recalc something ---
	statistic.recalcCompleteReleases();

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CKnownFile::GetTrafficBlock(uint64 qwStart, uint64 qwEnd, bool bSession/*=false*/)
{
	uint32	*pdwData = (bSession) ? statistic.blockTrafficSession : statistic.blockTraffic;

	if ((pdwData == NULL) || (qwStart > GetFileSize()))
		return 0;

	if (qwEnd > GetFileSize())
		qwEnd = GetFileSize();

	uint32	block = static_cast<uint32>(qwStart / EMBLOCKSIZE);
	uint32	bftb = 0 - static_cast<uint32>(qwStart % EMBLOCKSIZE);
	uint32	dwEndMod = static_cast<uint32>(qwEnd % EMBLOCKSIZE);
	uint32	t = 0;

	while (qwStart < qwEnd)
	{
		if ((qwEnd - qwStart) > dwEndMod)
			bftb += EMBLOCKSZ32;
		else
			bftb += dwEndMod;

		if (bftb != EMBLOCKSZ32)
			t += static_cast<uint32>(pdwData[block] * (double)bftb / GetBlockSize(block));
		else
			t += pdwData[block];
		qwStart += bftb;
		bftb = 0;
		block++;
	}

	return t;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CKnownFile::GetTrafficPart(uint64 qwStart, uint64 qwEnd, bool bSession/*=false*/)
{
	uint32	*pdwData = (bSession) ? statistic.partTrafficSession : statistic.partTraffic;

	if ((pdwData == NULL) || (qwStart > GetFileSize()))
		return 0;

	if (qwEnd > GetFileSize())
		qwEnd = GetFileSize();

	uint32	dwPart = static_cast<uint32>(qwStart / PARTSIZE);
	uint32	bftp = 0 - static_cast<uint32>(qwStart % PARTSIZE);
	uint32	dwEndMod = static_cast<uint32>(qwEnd % PARTSIZE);
	uint32	t = 0;

	while (qwStart < qwEnd)
	{
		if ((qwEnd - qwStart) > dwEndMod)
			bftp += PARTSZ32;
		else
			bftp += dwEndMod;

		if (bftp != PARTSZ32)
			t += static_cast<uint32>(pdwData[dwPart] * (double)bftp / GetPartSize(dwPart));
		else
			t += pdwData[dwPart];
		qwStart += bftp;
		bftp = 0;
		dwPart++;
	}

	return t;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CKnownFile::GetPartTraffic(uint16 part, bool session)
{
	if(statistic.partTraffic==NULL)
		return 0;

	if(session)
		return statistic.partTrafficSession[part];
	else
		return statistic.partTraffic[part];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CKnownFile::GetBlockTraffic(uint16 block, bool session)
{
	if(statistic.partTraffic==NULL)
		return 0;

	if(session)
		return statistic.blockTrafficSession[block];
	else
		return statistic.blockTraffic[block];
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CKnownFile::GetPartSize(uint32 dwPart) const
{
	return ((dwPart + 1) < GetPartCount()) ? PARTSZ32 : m_dwLastPartSz;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CKnownFile::GetBlockSize(uint32 dwBlock) const
{
	return ((dwBlock + 1) < GetBlockCount()) ? EMBLOCKSZ32 : m_dwLastBlkSz;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::resetStats(bool all)
{
	EMULE_TRY

	if (all)
	{
		if (partTraffic)		delete[] partTraffic;	partTraffic = NULL;
		if (partAccepted)	delete[] partAccepted;	partAccepted = NULL;
		if (blockTraffic)	delete[] blockTraffic;	blockTraffic = NULL;

		if (partTrafficSession)		delete[] partTrafficSession;	partTrafficSession = NULL;
		if (partAcceptedSession)		delete[] partAcceptedSession;	partAcceptedSession = NULL;
		if (blockTrafficSession)		delete[] blockTrafficSession;	blockTrafficSession = NULL;

		completeReleases = 0.0;

		m_dwAllTimeRequested = m_dwAllTimeAccepted = 0;
		m_qwAllTimeTransferred = 0;
	}
	else
	{
		if (partTrafficSession)
			for (uint16 part = 0; part < fileParent->GetPartCount(); part++)
				partTrafficSession[part] = 0;

		if (partAcceptedSession)
			for (uint16 part = 0; part < fileParent->GetPartCount(); part++)
				partAcceptedSession[part] = 0;

		if (blockTrafficSession)
			for (uint32 block = 0; block < fileParent->GetBlockCount(); block++)
				blockTrafficSession[block] = 0;
	}

	m_iNumRequested = m_iNumAccepted = 0;
	m_qwNumTransferred = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFileStatistic::resetPartTraffic(uint16 part, bool all)
{
	EMULE_TRY

	if(partTraffic==NULL || part>=fileParent->GetPartCount())
		return;

	//--- this all is a little bit tricky, because the part and block size have no connections ---
	//--- so i have to remove some bytes only from the start and end block ---
	//--- all we try is to keep it as realistic as possible :) ---
	uint32	dwBlocks = fileParent->GetBlockCount();
	uint32	dwBlkStart = static_cast<uint32>((static_cast<uint64>(part) * PARTSIZE) / EMBLOCKSIZE);
	double	dPercentStart = static_cast<double>(static_cast<uint32>((static_cast<uint64>(part) * PARTSIZE) % EMBLOCKSIZE)) / EMBLOCKSZ32;
	uint32	dwBlkEnd = min(static_cast<uint32>((static_cast<uint64>(part + 1) * PARTSIZE) / EMBLOCKSIZE), dwBlocks - 1);
	double	dPercentEnd = 0.0;
	if (dwBlkEnd != (dwBlocks - 1))
		dPercentEnd = 1.0 - static_cast<double>(static_cast<uint32>((static_cast<uint64>(part + 1) * PARTSIZE) / EMBLOCKSIZE)) / EMBLOCKSZ32;

	if(all)
	{
		partTraffic[part] = 0;
		blockTraffic[dwBlkStart] = static_cast<uint32>(blockTraffic[dwBlkStart] * dPercentStart);
		blockTraffic[dwBlkEnd] = static_cast<uint32>(blockTraffic[dwBlkEnd] * dPercentEnd);
		for (uint32 block = dwBlkStart + 1; block < dwBlkEnd; block++)
			blockTraffic[block] = 0;
	}

	partTrafficSession[part] = 0;
	blockTrafficSession[dwBlkStart] = static_cast<uint32>(blockTrafficSession[dwBlkStart] * dPercentStart);
	blockTrafficSession[dwBlkEnd] = static_cast<uint32>(blockTrafficSession[dwBlkEnd] * dPercentEnd);
	for (uint32 block = dwBlkStart + 1; block < dwBlkEnd; block++)
		blockTrafficSession[block] = 0;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CFileStatistic::GetPartAccepted(uint16 part, bool session) const
{
	EMULE_TRY

	if(partTraffic==NULL || part>=fileParent->GetPartCount())
		return 0;

	if(session==false)
		return partAccepted[part];
	else
		return partAcceptedSession[part];

	EMULE_CATCH

	return 0;
}
//--- :xrmb ---
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--- xrmb:allyaknow ---
//-------------------------------------------------------------------------------------------------
bool CFileStatistic::merge(CFileStatistic *m)
{
	EMULE_TRY

	m_dwAllTimeRequested += m->m_dwAllTimeRequested;
	m_dwAllTimeAccepted += m->m_dwAllTimeAccepted;
	m_qwAllTimeTransferred += m->m_qwAllTimeTransferred;

	m_iNumRequested	= m_iNumRequested + m->m_iNumRequested;
	m_iNumAccepted = m_iNumAccepted + m->m_iNumAccepted;
	m_qwNumTransferred += m->m_qwNumTransferred;

	//--- merge parttraffic ---

	//--- i dont have traffic, but the to merge has ---
	if(partTraffic==NULL && m->partTraffic)
		initTraffic();

	//--- merge traffic only if the to merge has some ---
	if(m->partTraffic)
	{
		for(uint16 part=0; part<fileParent->GetPartCount(); part++)
			partTraffic[part]+=m->partTraffic[part];

		for(uint32 block=0; block<fileParent->GetBlockCount(); block++)
			blockTraffic[block]+=m->blockTraffic[block];
	}

	return true;

	EMULE_CATCH

	return false;
}
//--- :xrmb ---
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::HasHiddenParts()
{
	EMULE_TRY

	for (uint16 i = 0; i < GetPartCount(); i++)
	{
		if (!IsPartShared(i))
			return true;
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::WritePartStatus(CFile *file)
{
	EMULE_TRY

	uchar	byteTmpBuff[32];	// should be power of 2
	uint32	dwDone = 0, dwBuffIdx = 2, dwDiff, dwParts = m_partHashArray.GetCount();

	POKE_WORD(byteTmpBuff, static_cast<uint16>(dwParts));

	while ((dwDiff = (dwParts - dwDone)) != 0)
	{
		byte	byteMask = 1;
		uint32	dwLimit = dwDone + ((dwDiff > 8) ? 8 : dwDiff);

		byteTmpBuff[dwBuffIdx] = 0;
		do
		{
			if (IsPartShared(dwDone++))
				byteTmpBuff[dwBuffIdx] |= byteMask;
			byteMask <<= 1;
		} while (dwDone < dwLimit);
		dwBuffIdx = (dwBuffIdx + 1) & (sizeof(byteTmpBuff) - 1);
		if (dwBuffIdx == 0)
			file->Write(byteTmpBuff, sizeof(byteTmpBuff));
	}
	if (dwBuffIdx != 0)
		file->Write(byteTmpBuff, dwBuffIdx);

	EMULE_CATCH
}
//--- :xrmb/partprio ---
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::IsJsComplete()
{
#ifndef NEW_SOCKETS_ENGINE
	return CJumpstarter::IsJsCompleteForFile(this);
#else
	return true;
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::SetJumpstartEnabled(bool enabled)
{
#ifndef NEW_SOCKETS_ENGINE
	if(!enabled && m_Jumpstarter)
	{
		m_Jumpstarter->Disable();
		delete m_Jumpstarter;
		m_Jumpstarter = NULL;
	}
	else if(enabled && !m_Jumpstarter)
	{
		CJumpstarter::EnableForFile(this);
		m_Jumpstarter = new CJumpstarter(this);
	}
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::AddSentBlock(CUpDownClient *client, const uint64 &qwStartOffset, uint32 togo)
{
#ifndef NEW_SOCKETS_ENGINE
	m_Jumpstarter->AddSentBlock(client, qwStartOffset, togo);
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::WriteJumpstartPartStatus(CUpDownClient *client, CMemFile* data)
{
#ifndef NEW_SOCKETS_ENGINE
	m_Jumpstarter->WriteJumpstartPartStatus(client, data);
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFile::AllowChunkForClient(uint32 partNo, CUpDownClient* client)
{
#ifndef NEW_SOCKETS_ENGINE
	return (!m_Jumpstarter) || m_Jumpstarter->AllowChunkForClient(partNo, client);
#else
	return true;
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ReadFileForUpload() opens known file and read information.
//		Params:
//			qwOffset      - file offset to start reading from;
//			dwBytesToRead - number of bytes to read;
//			pbyteBuffer   - buffer to receive data.
//		Return:
//			< 0 in case of error, else 0.
int CKnownFile::ReadFileForUpload(uint64 qwOffset, uint32 dwBytesToRead, byte *pbyteBuffer)
{
	int	iRc = -1;

	EMULE_TRY

	CFile	file;

	if (file.Open(m_strFilePath, CFile::modeRead | CFile::shareDenyNone | CFile::osSequentialScan))
	{
		file.Seek(qwOffset, CFile::begin);
		if (file.Read(pbyteBuffer, dwBytesToRead) == dwBytesToRead)
			iRc = 0;
		file.Close();
	}

	EMULE_CATCH

	return iRc;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CKnownFile::GetPermissionString()
{
#ifndef NEW_SOCKETS_ENGINE
	static const uint16 s_auResTbl[] =
	{
		IDS_FSTATUS_PUBLIC,			//PERM_ALL
		IDS_FSTATUS_FRIENDSONLY,	//PERM_FRIENDS
		IDS_HIDDEN					//PERM_NOONE
	};
	unsigned	uiPerm = GetPermissions();

	if (uiPerm >= ARRSIZE(s_auResTbl))
		uiPerm = PERM_ALL;
	return GetResString(s_auResTbl[uiPerm]);
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::AddClientToSourceList(CUpDownClient* pClient)
{
	EnterCriticalSection(&m_csSourceList);

	if (find(m_SourceList.begin(), m_SourceList.end(), pClient) == m_SourceList.end())
		m_SourceList.push_back(pClient);

	LeaveCriticalSection(&m_csSourceList);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::RemoveClientFromSourceList(CUpDownClient* pClient)
{
	EnterCriticalSection(&m_csSourceList);

	m_SourceList.remove(pClient);

	LeaveCriticalSection(&m_csSourceList);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFile::GetCopySourceList(ClientList *pCopy)
{
	if (m_SourceList.empty())
		return;

	EnterCriticalSection(&m_csSourceList);

	pCopy->insert(pCopy->begin(), m_SourceList.begin(), m_SourceList.end());

	LeaveCriticalSection(&m_csSourceList);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetKnownFilePriorityString() returns file upload priority string
CString CKnownFile::GetKnownFilePriorityString() const
{
	static const uint16 s_auAutoPrioResID[] = {
		IDS_PRIOAUTOLOW,	//PR_LOW
		IDS_PRIOAUTONORMAL,	//PR_NORMAL
		IDS_PRIOAUTOHIGH,	//PR_HIGH
		IDS_PRIOAUTORELEASE	//PR_RELEASE
	};
	static const uint16 s_auPrioResID[] = {
		IDS_PRIOLOW,		//PR_LOW
		IDS_PRIONORMAL,		//PR_NORMAL
		IDS_PRIOHIGH,		//PR_HIGH
		IDS_PRIORELEASE,	//PR_RELEASE
		IDS_PRIOVERYLOW		//PR_VERYLOW
	};
	unsigned	uiPrio = m_bytePriority;
	uint32		dwResId;

	if (m_bAutoPriority)
	{
		if (uiPrio >= ARRSIZE(s_auAutoPrioResID))
			uiPrio = PR_LOW;
		dwResId = s_auAutoPrioResID[uiPrio];
	}
	else
	{
		if (uiPrio >= ARRSIZE(s_auPrioResID))
			uiPrio = PR_VERYLOW;
		dwResId = s_auPrioResID[uiPrio];
	}

	CString	strBuffer;

	if (GetJumpstartEnabled())
		strBuffer.Format(_T("JumpStart[%s]"), GetResString(dwResId));
	else
		GetResString(&strBuffer, dwResId);
	return strBuffer;
}
