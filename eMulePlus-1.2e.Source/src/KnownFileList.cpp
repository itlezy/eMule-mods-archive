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
#include "KnownFileList.h"
#include "opcodes.h"
#ifndef NEW_SOCKETS_ENGINE
	#include "emule.h"
#else
	#include "otherfunctions.h"
#endif //NEW_SOCKETS_ENGINE
#include "SafeFile.h"
#include <share.h>
#include <io.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CKnownFileList::CKnownFileList(const CString& in_appdir)
{
	m_strFilesPath.Format(_T("%sknown.met"), in_appdir);
	m_strTrafficPath.Format(_T("%straffic.dat"), in_appdir);
	m_strPartPermissionsPath.Format(_T("%spartperm.dat"), in_appdir);
	m_iNumAccepted = 0;
	m_iNumRequested = 0;
	m_qwNumTransferred = 0;
	m_bInSave	=	false;
	Init();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CKnownFileList::~CKnownFileList()
{
	Clear();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFileList::Init()
{
	EMULE_TRY

	if (!PathFileExists(m_strFilesPath))
		Save();

	if (!Load())
		return false;
	
	LoadPartTraffic();

//	Restore the hidden/blocked settings
	LoadPartPrio();

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFileList::Load()
{
	CSafeBufferedFile	file;
	CKnownFile			*pKnownFile = NULL;

	try
	{
		//	Try to open the "Known Files" .met. If we fail...
		if (!file.Open(m_strFilesPath, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
		{
			return false;
		}
		
		byte	byteFileHdr;

		setvbuf(file.m_pStream, NULL, _IOFBF, 16*1024);
		file.Read(&byteFileHdr, 1);

		if ((byteFileHdr != MET_HEADER) && (byteFileHdr != MET_HEADER_I64TAGS))
		{
			file.Close();
			return false;
		}

		CSingleLock		sLock(&m_mutexList, true); // to make sure that its thread-safe
		uint32			dwNumKnownFiles;

		file.Read(&dwNumKnownFiles, 4);				// <count:DWORD>

		for (uint32 i = 0; i < dwNumKnownFiles; i++)
		{
			try
			{
				pKnownFile = new CKnownFile();
				if (pKnownFile != NULL)
				{
					if (pKnownFile->LoadFromFile(file))
						Add(pKnownFile);
					else
						safe_delete(pKnownFile);
				}
			}
			catch (CMemoryException *pError)
			{
				OUTPUT_DEBUG_TRACE();
				AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Error by KnownFileList init %s"), GetErrorMessage(pError));
				pError->Delete();
			}
		}

		file.Close();
		sLock.Unlock();
	}
	catch (CFileException *pError)
	{
		OUTPUT_DEBUG_TRACE();
#ifndef NEW_SOCKETS_ENGINE
		if (pError->m_cause == CFileException::endOfFile)
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_KNOWNMET_BAD);
		else
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_KNOWNMET_UNKNOWN, GetErrorMessage(pError));
		g_App.m_pMDlg->DisableAutoBackup();
#endif //NEW_SOCKETS_ENGINE
		pError->Delete();

		return false;
	}
	
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFileList::Save()
{
	// to avoid situation when sources are saved every 30 min and on exit
	// eklmn: in order to save CPU load must be checked first
	if (m_bInSave)
		return;

//	We need to do it thread safe because now it's called under CUploadQueue::RemoveFromUploadQueue()
//	Also now it's called from separate thread every 30 minutes
	CSingleLock sLock(&m_mutexList, true);

	EMULE_TRY

	try
	{
		CStdioFile	file;
		CKnownFile	*pKnownFile;
		byte		abyteBuf[5];

		if (!file.Open(m_strFilesPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
			return;

		// file was opened successfully, set save flag
		m_bInSave = true;

		//get a list number
		uint32	dwOldRecordsNumber = GetCount();
		uint32	dwNewRecordsNumber = dwOldRecordsNumber;
		byte	byteActualFileHdr = MET_HEADER;

	//	Save a header
		abyteBuf[0] = byteActualFileHdr;
		POKE_DWORD(&abyteBuf[1], dwOldRecordsNumber);
		file.Write(abyteBuf, sizeof(abyteBuf));

		// This routine was changed to take in account possible NULL pointers
		// I think is not needed anymore because now is thread safe
		uint32 i = 0;
		while (i < dwNewRecordsNumber)
		{
			pKnownFile = ElementAt(i);
			if (pKnownFile == NULL) // if NULL, remove it!
			{
				dwNewRecordsNumber--;
				RemoveAt(i);
			}
			else // element is not NULL, write it and go for next one
			{
				pKnownFile->WriteToFile(file);
				if (pKnownFile->IsLargeFile())
					byteActualFileHdr = MET_HEADER_I64TAGS;
				i++;
			}
		}

	//	Update file header and number of records
		abyteBuf[0] = byteActualFileHdr;
		POKE_DWORD(&abyteBuf[1], dwNewRecordsNumber);
		file.SeekToBegin();
		file.Write(abyteBuf, sizeof(abyteBuf));

		//eklmn: by shutdown force OS flush data direct to disk [SlugFiller]
#ifndef NEW_SOCKETS_ENGINE
		if ( !g_App.m_pMDlg->IsRunning() )
			_commit(_fileno(file.m_pStream));
#endif //NEW_SOCKETS_ENGINE

		file.Close();

		//--- xrmb:parttraffic ---
		SavePartTraffic();

		//--- xrmb:partprio ---
		SavePartPrio();
	}
	catch (CFileException *error)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to save %s - %s"), m_strFilesPath, GetErrorMessage(error));
		error->Delete();
	}
	EMULE_CATCH

	sLock.Unlock();
	m_bInSave	=	false;
}
 ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFileList::Clear()
{
	EMULE_TRY

	for (int i = 0; i < GetCount();i++)
		safe_delete(ElementAt(i));
	RemoveAll();
	SetSize(0);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CKnownFile* CKnownFileList::FindKnownFile(const CString &strFileName, uint32 dwDate, uint64 qwSize)
{
	EMULE_TRY

	for (int i = 0; i < GetCount(); i++)
	{
		CKnownFile* pKnownFile = ElementAt(i);
	
		if ( (pKnownFile != NULL) && (pKnownFile->GetFileDate() == dwDate) &&
			(pKnownFile->GetFileSize() == qwSize) && (strFileName == pKnownFile->GetFileName()) )
		{
			return pKnownFile;
		}
	}

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//SyruS show completed files (0.28b)
CKnownFile* CKnownFileList::FindKnownFileByID(const uchar* hash)
{
	EMULE_TRY

	for (int i=0; i < GetCount(); i++)
	{
		CKnownFile* pCurKnownFile = ElementAt(i);
		if (!md4cmp(pCurKnownFile->GetFileHash(), hash))
			return pCurKnownFile;
	}
	return NULL;

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CKnownFileList::SafeAddKnownFile(CKnownFile* pKnownFileToAdd)
{
	if (!pKnownFileToAdd)
		return;

	CSingleLock sLock(&m_mutexList,true);
	Add(pKnownFileToAdd);
	sLock.Unlock();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFileList::merge()
{
	EMULE_TRY

	CSingleLock sLock(&m_mutexList,true);

	int totalcnt=0;
	int items=GetCount();

//	Go through all the items
	for(int i=0; i<items; i++)
	{
		int cnt=0;

	//	Compare with all other items
		for (int j = (i + 1); j < items; j++)
		{
		//	Same size and file hash...
			if(	(ElementAt(i)->GetFileSize() == ElementAt(j)->GetFileSize()) &&
				(md4cmp(ElementAt(i)->GetFileHash(), ElementAt(j)->GetFileHash()) == 0) )
			{
#ifndef NEW_SOCKETS_ENGINE
				AddLogLine(0, IDS_MERGE, ElementAt(i)->GetFileName(), ElementAt(j)->GetFileName());
#endif

			//	If one file is shared the other should be known
				if (ElementAt(j)->GetSharedFile())
				{
					if (ElementAt(i)->GetSharedFile())
					{
					//	Duplicate shared files are found, throw a warning and continue merging
#ifndef NEW_SOCKETS_ENGINE
						AddLogLine(LOG_RGB_WARNING, IDS_MERGE_WARNING);
#endif
						continue;
					}

					CKnownFile *t=ElementAt(i);
					SetAt(i, ElementAt(j));
					SetAt(j, t);
				}

			//	Merge into first item
				if(ElementAt(i)->statistic.merge(&ElementAt(j)->statistic) == false)
				{
					sLock.Unlock();
#ifndef NEW_SOCKETS_ENGINE
					AddLogLine(LOG_RGB_ERROR, IDS_MERGE_FAILED);
#endif //NEW_SOCKETS_ENGINE
					return false;
				}

			//	Get original file Priority and set it to the merged file
				ElementAt(i)->SetULPriority(ElementAt(j)->GetULPriority());

			//	Remove second item
				CKnownFile	*d=ElementAt(j);
				RemoveAt(j);
			//	Try to clear file from download list if it's where as complete file not to crash
			//	during download list update, because file object will be destroyed now.
				g_App.m_pDownloadList->ClearCompleted(d->GetFileHash());
				safe_delete(d);
				items--;
				j--;

				cnt++;
				totalcnt++;
			}
		}

		if(cnt > 1)
		{
#ifndef NEW_SOCKETS_ENGINE
			AddLogLine(0, IDS_MERGE_CNT, i, cnt);
#endif
		}
	}

	sLock.Unlock();
#ifndef NEW_SOCKETS_ENGINE
	AddLogLine(0, IDS_MERGE_TOTAL, totalcnt);
#endif

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFileList::RemoveFile(CKnownFile* toRemove)
{
	EMULE_TRY

	CSingleLock sLock(&m_mutexList, true);

	//--- go thru all items ---
	for(int i=0; i<GetCount(); i++)
	{
		if(ElementAt(i)==toRemove)
		{
			RemoveAt(i);
			delete toRemove;
		
			sLock.Unlock();
			return true;
		}
	}

	sLock.Unlock();

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--- xrmb:partprio ---
bool CKnownFileList::LoadPartPrio()
{
//	Save the part permission
	FILE	*file = _tfsopen(m_strPartPermissionsPath, _T("rb"), _SH_DENYWR);
	if(file == NULL)
		return false;

	byte version = 1;
	fread(&version, sizeof(version), 1, file);

	uint32	RecordsNumber=0;
	fread(&RecordsNumber, sizeof(RecordsNumber), 1, file);

	CKnownFile	*kf;
	uchar		hash[16];
	uint64		qwFileSz = 0;
	uint32		fpos, dwFileSzLen = (version == 1) ? 4 : 8;
	for (uint32 i = 0; i < RecordsNumber; i++) 
	{
		fread(hash, 16, 1, file);	// read hash
		fread(&qwFileSz, dwFileSzLen, 1, file);	//read file size

		fpos = ftell(file);	// save file pos

	//	Search for a file in the list
		for (int j = 0; j < GetCount(); j++)
		{
			kf = ElementAt(j);
		//	Right file?
			if ((kf->GetFileSize() == qwFileSz) && (md4cmp(kf->GetFileHash(), hash) == 0))
			{
				fseek(file, fpos, SEEK_SET);
				fread(kf->GetPartStatusArr(), kf->GetPartCount() * sizeof(byte), 1, file);
			}
		}
	}
	fclose(file);

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SavePartPrio() is called under m_mutexList protection
bool CKnownFileList::SavePartPrio()
{
	//--- save the part-permission ---
	FILE	*file = _tfsopen(m_strPartPermissionsPath, _T("wb"), _SH_DENYWR);
	if(file == NULL)
		return false;

//	ver.1 -- 32bit file size
//	ver.2 (starting from v1.2a) -- 64bit file size
	byte version = 2;
	fwrite(&version, sizeof(version), 1, file);

	//--- preserve the space for the recordnumers ---
	CKnownFile	*kf;
	uint32	RecordsNumber=0;
	uint32	rnpos=ftell(file);
	fwrite(&RecordsNumber, sizeof(RecordsNumber), 1, file);
	for (int i = 0; i < GetCount(); i++)
	{
		kf = ElementAt(i);
	//	Save files only with hidden parts
		if(kf->HasHiddenParts())
		{
			fwrite(kf->GetFileHash(), 16, 1, file);	// write hash

			uint64 fs = kf->GetFileSize();
			fwrite(&fs, sizeof(fs), 1, file);	// write file size
		//	Save prio
			fwrite(kf->GetPartStatusArr(), kf->GetPartCount() * sizeof(byte), 1, file);

			RecordsNumber++;
		}
	}

	//--- save the number of stored records ---
	fseek(file, rnpos, SEEK_SET);
	fwrite(&RecordsNumber, sizeof(RecordsNumber), 1, file);

	fclose(file);

	return true;
}
//--- :xrmb/partprio ---
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//--- xrmb:parttraffic ---
bool CKnownFileList::LoadPartTraffic()
{
//	Load collected traffic data
	FILE	*fh = _tfsopen(m_strTrafficPath, _T("rb"), _SH_DENYWR);

	if (fh == NULL)
		return false;

	setvbuf(fh, NULL, _IOFBF, 64*1024);

	CSingleLock		sLock(&m_mutexList,true); // to make sure that its thread-safe

	byte	version=0;
	fread(&version, sizeof(version), 1, fh);

	uint32	records;
	fread(&records, sizeof(records), 1, fh);

	uchar	abyteFileHash[16];
	uint64	qwFileSz = 0;
	uint32	dwFileSzLen = (version < 3) ? 4 : 8;

	for (uint32 i = 0; i < records; i++) 
	{
		byte hasData=0;
		fread(&hasData, sizeof(hasData), 1, fh);

		if(hasData)
		{
			fread(abyteFileHash, 16, 1, fh);
			fread(&qwFileSz, dwFileSzLen, 1, fh);
			if ( (qwFileSz == ElementAt(i)->GetFileSize()) &&
				(md4cmp(abyteFileHash, ElementAt(i)->GetFileHash()) == 0) )
			{
				if (!ElementAt(i)->LoadFromFileTraffic(fh, version))
				{
#ifndef NEW_SOCKETS_ENGINE
					AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_TRAFFICDAT_CORRUPT, _T("traffic.dat"));
					g_App.m_pMDlg->DisableAutoBackup();
#endif
					break;
				}
			}
			else
			{
#ifndef NEW_SOCKETS_ENGINE
				AddLogLine(LOG_RGB_WARNING, IDS_TRAFFIC_KNOWN_ASYNC, i);
				g_App.m_pMDlg->DisableAutoBackup();
#endif
				break;
			}
		}
	}
	fclose(fh);

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CKnownFileList::SavePartTraffic()
{
	//--- save traffic data to our file ---
	FILE	*file = _tfsopen(m_strTrafficPath, _T("wb"), _SH_DENYWR);

	if (file == NULL)
		return false;

	setvbuf(file, NULL, _IOFBF, 64*1024);

//	ver.1 -- initial version
//	ver.2 -- introduced part-accepted counter statistics
//	ver.3 (starting from v1.2a) -- 64bit file size
	byte	version = 3;
	fwrite(&version, sizeof(version), 1, file);	

	uint32	RecordsNumber=GetCount();
	fwrite(&RecordsNumber, sizeof(RecordsNumber), 1, file);
	for(uint32 i=0; i<RecordsNumber; i++) 
	{
		// only save for known-files with traffic
		if(ElementAt(i)->statistic.partTraffic)
		{
			byte hasData=1;
			fwrite(&hasData, sizeof(hasData), 1, file);

			//--- write hash ---
			fwrite(ElementAt(i)->GetFileHash(), 16, 1, file);

			//--- write filesize ---
			uint64 fs = ElementAt(i)->GetFileSize();
			fwrite(&fs, sizeof(fs), 1, file);
			
			//--- write part/block traffic ---
			ElementAt(i)->SaveToFileTraffic(file);
		}
		else
		{
			byte hasData=0;
			fwrite(&hasData, sizeof(hasData), 1, file);
		}
	}

	fclose(file);

	return true;
}
//--- :xrmb/parttraffic ---
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
