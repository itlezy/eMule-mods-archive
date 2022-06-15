y//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif

#include <io.h>
#include "FilePreferences.h"
#include "Preferences.h"
#include "NeoPreferences.h"
#include "PartFile.h"
#include "SafeFile.h"
#include "Packets.h"
#include "emule.h"
#include "Log.h"
#include "emuledlg.h"
#include "Defaults.h"
#include "NeoOpCodes.h"
#include "Functions.h"
#include "KnownFileList.h"
#include "Neo/Ini2.h" // NEO: INI - [PlusIniClass] <-- Xanatos --
//#include "Ini2.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NEO: FCFG - [FileConfiguration] -- Xanatos -->

#define KNOWNPREFS_MET_FILENAME	_T("KnownPrefs.met")

////////////////////////////////////////////////////////////////////////////////////////
// CPartFile
//

bool CPartFile::SaveNeoFile()
{
	CString strNeoFile(m_fullname);
	strNeoFile += PARTNEO_EXT;

	CString strTmpFile(strNeoFile);
	strTmpFile += PARTNEO_TMP_EXT;

	// save tweak to part.neo file
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strTmpFile, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError;
		strError.Format(GetResString(IDS_X_ERR_SAVENEO), strNeoFile, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		//version
		file.WriteUInt8(NEOFILE_VERSION);

		SaveNeoFile(&file);

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError;
		strError.Format(GetResString(IDS_X_ERR_SAVENEO), strNeoFile, GetFileName());
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();

		// remove the partially written or otherwise damaged temporary file
		file.Abort(); // need to close the file before removing it. call 'Abort' instead of 'Close', just to avoid an ASSERT.
		(void)_tremove(strTmpFile);
		return false;
	}

	// after successfully writing the temporary part.neo file...
	if (_tremove(strNeoFile) != 0 && errno != ENOENT){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to remove \"%s\" - %s"), strNeoFile, _tcserror(errno));
	}

	if (_trename(strTmpFile, strNeoFile) != 0){
		int iErrno = errno;
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to move temporary part.met.neo file \"%s\" to \"%s\" - %s"), strTmpFile, strNeoFile, _tcserror(iErrno));

		CString strError;
		strError.Format(GetResString(IDS_X_ERR_SAVENEO), strNeoFile, GetFileName());
		strError += _T(" - ");
		strError += strerror(iErrno);
		ModLogError(_T("%s"), strError);
		return false;
	}

	// create a backup of the successfully written part.met file
	CString BAKName(strNeoFile);
	BAKName.Append(PARTNEO_BAK_EXT);
	if (!::CopyFile(strNeoFile, BAKName, FALSE)){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to create backup of %s (%s) - %s"), strNeoFile, GetFileName(), GetErrorMessage(GetLastError()));
	}

	return true;
}

bool CPartFile::SaveNeoFile(CFileDataIO* file)
{
	
	/*
	* Save All Neo File settings here
	*/
	if(PartPrefs->IsFilePrefs() && !PartPrefs->IsEmpty()){
		file->WriteUInt8(PARTPREFSFILE_VERSION);

		ULONGLONG pos = file->GetPosition();
		file->WriteUInt16(0);

		PartPrefs->Save(file);
		
		file->Seek(pos, CFile::begin);
		ASSERT((file->GetLength() - (file->GetPosition()+2)) < 0xFFFF);
		file->WriteUInt16((uint16)(file->GetLength() - (file->GetPosition()+2)));
		file->Seek(0, CFile::end);
	}
	if(KnownPrefs->IsFilePrefs() && !KnownPrefs->IsEmpty()){
		file->WriteUInt8(KNOWNPREFSFILE_VERSION);

		ULONGLONG pos = file->GetPosition();
		file->WriteUInt16(0);

		KnownPrefs->Save(file);
		
		file->Seek(pos, CFile::begin);
		ASSERT((file->GetLength() - (file->GetPosition()+2)) < 0xFFFF);
		file->WriteUInt16((uint16)(file->GetLength() - (file->GetPosition()+2)));
		file->Seek(0, CFile::end);
	}

	// NEO: NPT - [NeoPartTraffic]
	if(statistic.HasTraffic())
	{
		file->WriteUInt8(TRAFFICFILE_VERSION);

		ULONGLONG pos = file->GetPosition();
		file->WriteUInt16(0);

		statistic.SaveTraffic(file);
		
		file->Seek(pos, CFile::begin);
		ASSERT((file->GetLength() - (file->GetPosition()+2)) < 0xFFFF);
		file->WriteUInt16((uint16)(file->GetLength() - (file->GetPosition()+2)));
		file->Seek(0, CFile::end);
	}
	// NEO: NPT END

	// NEO: SCV - [SubChunkVerification]
	if(NeoPrefs.UseSubChunkTransfer()){
		file->WriteUInt8(AICHBLOCKMAP_VERSION);

		ULONGLONG pos = file->GetPosition();
		file->WriteUInt16(0);

		SaveAICHMap(file);
		
		file->Seek(pos, CFile::begin);
		ASSERT((file->GetLength() - (file->GetPosition()+2)) < 0xFFFF);
		file->WriteUInt16((uint16)(file->GetLength() - (file->GetPosition()+2)));
		file->Seek(0, CFile::end);
	}
	// NEO: SCV END

	// NEO: XCs - [SaveComments]
	if(NeoPrefs.UseSaveComments() && HasComments()){
		file->WriteUInt8(COMMENTSFILE_VERSION);

		ULONGLONG pos = file->GetPosition();
		file->WriteUInt16(0);

		SaveComments(file);
		
		file->Seek(pos, CFile::begin);
		ASSERT((file->GetLength() - (file->GetPosition()+2)) < 0xFFFF);
		file->WriteUInt16((uint16)(file->GetLength() - (file->GetPosition()+2)));
		file->Seek(0, CFile::end);
	}
	// NEO: XCs END

	return true;
}

bool CPartFile::LoadNeoFile()
{
	CString strNeoFile(m_fullname);
	strNeoFile += PARTNEO_EXT;

	uint8 version;
	
	// readfile tweaks form part.neo file
	CSafeBufferedFile file;
	CFileException fexpMet;
	if (!file.Open(strNeoFile, CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexpMet)){
		if (fexpMet.m_cause != CFileException::fileNotFound){
			CString strError;
			strError.Format(GetResString(IDS_X_ERR_OPENNEO), strNeoFile, _T(""));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexpMet.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			ModLogError(LOG_STATUSBAR, _T("%s"), strError);
			return false;
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		version = file.ReadUInt8();
		
		if (version > NEOFILE_VERSION || version <= NEOFILE_VERSION_OLD){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_BADNEOVERSION), strNeoFile, GetFileName());
			file.Close();
			return false;
		}
		
		LoadNeoFile(&file);

		file.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_NEOMEOCORRUPT), strNeoFile, GetFileName());
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_FILEERROR), strNeoFile, GetFileName(), buffer);
		}
		error->Delete();
		return false;
	}
	catch(...){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), strNeoFile, GetFileName());
		ASSERT(0);
		return false;
	}

	return true;
}

bool CPartFile::LoadNeoFile(CFileDataIO* file)
{
	/*
	* Load All Neo File settings here
	*/
	uint8 segment;
	UINT length;
	while (file->GetLength()-file->GetPosition())
	{
		segment = file->ReadUInt8();
		length = file->ReadUInt16();
		if(length == 0xFFFF) // just in case
			length = file->ReadUInt32();

		switch(segment)
		{
			case 0: // kill opcode in case we want append some other data below this list
				ASSERT(length == 0);
				return true;

			case PARTPREFSFILE_VERSION:{
				CPartPreferences* prevPartPrefs = PartPrefs; // can be cat prefs or global prefs, doesn't mater
				ASSERT(PartPrefs->IsFilePrefs() == false);
				PartPrefs = new CPartPreferencesEx(CFP_FILE);
				((CPartPreferencesEx*)PartPrefs)->PartFile = this;
				((CPartPreferencesEx*)PartPrefs)->PartPrefs = prevPartPrefs;
				PartPrefs->Load(file);
				break;
			}
			case KNOWNPREFSFILE_VERSION:{
				CKnownPreferences* prevKnownPrefs = KnownPrefs; // can be cat prefs or global prefs, doesn't mater
				ASSERT(KnownPrefs->IsFilePrefs() == false);
				KnownPrefs = new CKnownPreferencesEx(CFP_FILE);
				((CKnownPreferencesEx*)KnownPrefs)->KnownFile = this;
				((CKnownPreferencesEx*)KnownPrefs)->KnownPrefs = prevKnownPrefs;
				KnownPrefs->Load(file);
				break;
			}
			// NEO: NPT - [NeoPartTraffic]
			case TRAFFICFILE_VERSION:{
				statistic.LoadTraffic(file);
				break;
			}
			// NEO: NPT END
			// NEO: SCV - [SubChunkVerification]
			case AICHBLOCKMAP_VERSION:{
				LoadAICHMap(file);
				break;
			}
			// NEO: SCV END
			// NEO: XCs - [SaveComments]
			case COMMENTSFILE_VERSION:{
				LoadComments(file);
				break;
			}
			// NEO: XCs END
			default:
				if(file->GetPosition() + length > file->GetLength())
					AfxThrowFileException(CFileException::endOfFile, 0, _T("MemoryFile"));
				DebugLog(_T("Unknown NEO File segment ID 0x%02x received"), segment);
				file->Seek(length, CFile::current);
		}
	}

	return true;
}



////////////////////////////////////////////////////////////////////////////////////////
// CKnownFileList
//

bool CKnownFileList::LoadKnownPreferences()
{
	CString fullpath=thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath.Append(KNOWNPREFS_MET_FILENAME);
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath,CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") KNOWNPREFS_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			ModLogError(LOG_STATUSBAR, _T("%s"), strError);
			return false;
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try {
		uint8 version = file.ReadUInt8();
		if (version > KNOWNPREFSFILE_VERSION || version <= KNOWNPREFSFILE_VERSION_OLD){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_KNOWNPREFSMET_UNKNOWN_VERSION));
			file.Close();
			return false;
		}
		
		UINT RecordsNumber = file.ReadUInt32();
		uchar cur_hash[16];
		CKnownFile* cuf_file=NULL;
		for (UINT i = 0; i < RecordsNumber; i++) {
			file.ReadHash16(cur_hash);
			if((cuf_file = FindKnownFileByID(cur_hash)) != NULL){
				CKnownPreferences* prevKnownPrefs = cuf_file->KnownPrefs; // can be cat prefs or global prefs, doesn't mater
				cuf_file->KnownPrefs = new CKnownPreferencesEx(CFP_FILE);
				((CKnownPreferencesEx*)cuf_file->KnownPrefs)->KnownFile = cuf_file;
				((CKnownPreferencesEx*)cuf_file->KnownPrefs)->KnownPrefs = prevKnownPrefs;
				if(!cuf_file->KnownPrefs->Load(&file)){
					ModLogError(GetResString(IDS_X_ERR_KNOWNPREFSMET_ENTRY_CORRUPT), cuf_file->GetFileName());
				}
			}else{
				ModLogError(GetResString(IDS_X_ERR_KNOWNPREFSASYNCHRONIZED), md4str(cur_hash)); 
				ClearPreferencesEntry(&file);
			}
		}
		file.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_KNOWNPREFSMET_BAD));
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,MAX_CFEXP_ERRORMSG);
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_KNOWNPREFSMET_UNKNOWN),buffer);
		}
		error->Delete();
		return false;
	}

	return true;
}

bool CKnownFileList::ClearPreferencesEntry(CFileDataIO* file)
{
	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
        delete newtag;
	}
	return true;
}

bool CKnownFileList::SaveKnownPreferences()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false,_T("Saving KnownPrefs files list file \"%s\""), KNOWNPREFS_MET_FILENAME);
	CString fullpath=thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath += KNOWNPREFS_MET_FILENAME;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(_T("Failed to save ") KNOWNPREFS_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		file.WriteUInt8(KNOWNPREFSFILE_VERSION);

		uint32 uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		POSITION pos = m_Files_map.GetStartPosition();
		while( pos != NULL )
		{
			CKnownFile* pFile;
			CCKey key;
			m_Files_map.GetNextAssoc( pos, key, pFile );
			if(!pFile->KnownPrefs->IsEmpty()){
				file.WriteHash16(pFile->GetFileHash());
				pFile->KnownPrefs->Save(&file);
				uTagCount++;
			}
		}

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.Seek(0, CFile::end);

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError(_T("Failed to save ") KNOWNPREFS_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////
// CKnownPreferences
//

CKnownPreferences::CKnownPreferences(){

}

CKnownPreferences::~CKnownPreferences(){
	ClearTags();
}

void CKnownPreferences::Save(CFileDataIO* file)
{
	UINT uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_B_SET(m_EnableLanCast,OP_ENABLE_LAN_CAST)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_B_SET(m_EnableVoodoo,OP_ENABLE_VOODOO)
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	IMPLEMENT_FP_B_SET(m_UseInteligentPartSharing,OP_USE_INTELIGENT_PART_SHARING)
	IMPLEMENT_FP_B_SET(m_InteligentPartSharingTimer,OP_INTELIGENT_PART_SHARING_TIMER)

	IMPLEMENT_FP_B_SET(m_MaxProzentToHide,OP_MAX_PROZENT_TO_HIDE)

	// OverAvalibly
	IMPLEMENT_FP_B_SET(m_HideOverAvaliblyParts,OP_HIDE_OVER_AVALIBLY_PARTS)
	IMPLEMENT_FP_B_SET(m_HideOverAvaliblyMode,OP_HIDE_OVER_AVALIBLY_MODE)
	IMPLEMENT_FP_B_SET(m_HideOverAvaliblyValue,OP_HIDE_OVER_AVALIBLY_VALUE)

	IMPLEMENT_FP_B_SET(m_BlockHighOverAvaliblyParts,OP_BLOCK_HIGH_OVERAVALIBLY_PARTS)
	IMPLEMENT_FP_B_SET(m_BlockHighOverAvaliblyFactor,OP_BLOCK_HIGH_OVER_AVALIBLY_FACTOR)

	// OverShared
	IMPLEMENT_FP_B_SET(m_HideOverSharedParts,OP_HIDE_OVER_SHARED_PARTS)
	IMPLEMENT_FP_B_SET(m_HideOverSharedMode,OP_HIDE_OVER_SHARED_MODE)
	IMPLEMENT_FP_B_SET(m_HideOverSharedValue,OP_HIDE_OVER_SHARED_VALUE)
	IMPLEMENT_FP_B_SET(m_HideOverSharedCalc,OP_HIDE_OVER_SHARED_CALC)

	IMPLEMENT_FP_B_SET(m_BlockHighOverSharedParts,OP_BLOCK_HIGH_OVER_SHARED_PARTS)
	IMPLEMENT_FP_B_SET(m_BlockHighOverSharedFactor,OP_BLOCK_HIGH_OVER_SHARED_FACTOR)

	// DontHideUnderAvalibly
	IMPLEMENT_FP_B_SET(m_DontHideUnderAvaliblyParts,OP_DONT_HIDE_UNDER_AVALIBLY_PARTS)
	IMPLEMENT_FP_B_SET(m_DontHideUnderAvaliblyMode,OP_DONT_HIDE_UNDER_AVALIBLY_MODE)
	IMPLEMENT_FP_B_SET(m_DontHideUnderAvaliblyValue,OP_DONT_HIDEUNDER_AVALIBLY_VALUE)

	// Other
	IMPLEMENT_FP_B_SET(m_ShowAlwaysSomeParts,OP_SHOW_ALWAYS_SOME_PARTS)
	IMPLEMENT_FP_B_SET(m_ShowAlwaysSomePartsValue,OP_SHOW_ALWAYS_SOME_PARTS_VALUE)

	IMPLEMENT_FP_B_SET(m_ShowAlwaysIncompleteParts,OP_SHOW_ALWAYS_INCOMPLETE_PARTS)
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	IMPLEMENT_FP_B_SET(m_ReleaseMode,OP_RELEASE_MODE)
	IMPLEMENT_FP_B_SET(m_ReleaseLevel,OP_RELEASE_LEVEL)
	IMPLEMENT_FP_B_SET(m_ReleaseTimer,OP_RELEASE_TIMER)

	// release limit
	IMPLEMENT_FP_B_SET(m_ReleaseLimit,OP_RELEASE_LIMIT)
	IMPLEMENT_FP_B_SET(m_ReleaseLimitMode,OP_RELEASE_LIMIT_MODE)
	IMPLEMENT_FP_B_SET(m_ReleaseLimitHigh,OP_RELEASE_LIMIT_HIGH)
	IMPLEMENT_FP_B_SET(m_ReleaseLimitLow,OP_RELEASE_LIMIT_LOW)

	IMPLEMENT_FP_B_SET(m_ReleaseLimitLink,OP_RELEASE_LIMIT_LINK)

	IMPLEMENT_FP_B_SET(m_ReleaseLimitComplete,OP_RELEASE_LIMIT_COMPLETE)
	IMPLEMENT_FP_B_SET(m_ReleaseLimitCompleteMode,OP_RELEASE_LIMIT_COMPLETE_MODE)
	IMPLEMENT_FP_B_SET(m_ReleaseLimitCompleteHigh,OP_RELEASE_LIMIT_COMPLETE_HIGH)
	IMPLEMENT_FP_B_SET(m_ReleaseLimitCompleteLow,OP_RELEASE_LIMIT_COMPLETE_LOW)

	// limit
	IMPLEMENT_FP_B_SET(m_LimitLink,OP_LIMIT_LINK)

	// source limit
	IMPLEMENT_FP_B_SET(m_SourceLimit,OP_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_SourceLimitMode,OP_SOURCE_LIMIT_MODE)
	IMPLEMENT_FP_B_SET(m_SourceLimitHigh,OP_SOURCE_LIMIT_HIGH)
	IMPLEMENT_FP_B_SET(m_SourceLimitLow,OP_SOURCE_LIMIT_LOW)

	IMPLEMENT_FP_B_SET(m_SourceLimitLink,OP_SOURCE_LIMIT_LINK)

	IMPLEMENT_FP_B_SET(m_SourceLimitComplete,OP_SOURCE_LIMIT_COMPLETE)
	IMPLEMENT_FP_B_SET(m_SourceLimitCompleteMode,OP_SOURCE_LIMIT_COMPLETE_MODE)
	IMPLEMENT_FP_B_SET(m_SourceLimitCompleteHigh,OP_SOURCE_LIMIT_COMPLETE_HIGH)
	IMPLEMENT_FP_B_SET(m_SourceLimitCompleteLow,OP_SOURCE_LIMIT_COMPLETE_LOW)
	// NEO: SRS END

	// NEO: MPS - [ManualPartSharing]
	POSITION	pos = m_ManagedParts.GetStartPosition();
	UINT		part;
	uint8		status;
	while(pos)
	{
		m_ManagedParts.GetNextAssoc(pos, part, status);
		if(status != PR_PART_NORMAL)
		{
			uint8 TagID;
			switch(status){
				case PR_PART_ON: TagID = OP_PR_PART_ON; break;
				case PR_PART_HIDEN: TagID = OP_PR_PART_HIDEN; break;
				case PR_PART_OFF: TagID = OP_PR_PART_OFF; break;
				default: ASSERT(0); continue;
			}
			CTag tag(TagID, part);
			tag.WriteNewEd2kTag(file);
			uTagCount++;
		}
	}
	// NEO: MPS END

	for (int j = 0; j < taglist.GetCount(); j++){
		if (taglist[j]->IsStr() || taglist[j]->IsInt()){
			taglist[j]->WriteNewEd2kTag(file);
			uTagCount++;
		}
	}

	file->Seek(uTagCountFilePos, CFile::begin);
	file->WriteUInt32(uTagCount);
	file->Seek(0, CFile::end);
}

void CKnownPreferences::ClearTags()
{
	for (int i = 0; i < taglist.GetSize(); i++)
		delete taglist[i];
	taglist.RemoveAll();
}

bool CKnownPreferences::Load(CFileDataIO* file)
{
	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
		switch (newtag->GetNameID()){

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
			IMPLEMENT_FP_B_GET(m_EnableLanCast,OP_ENABLE_LAN_CAST)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			IMPLEMENT_FP_B_GET(m_EnableVoodoo,OP_ENABLE_VOODOO)
#endif // VOODOO // NEO: VOODOO END

			// NEO: IPS - [InteligentPartSharing]
			IMPLEMENT_FP_B_GET(m_UseInteligentPartSharing,OP_USE_INTELIGENT_PART_SHARING)
			IMPLEMENT_FP_B_GET(m_InteligentPartSharingTimer,OP_INTELIGENT_PART_SHARING_TIMER)

			IMPLEMENT_FP_B_GET(m_MaxProzentToHide,OP_MAX_PROZENT_TO_HIDE)

			// OverAvalibly
			IMPLEMENT_FP_B_GET(m_HideOverAvaliblyParts,OP_HIDE_OVER_AVALIBLY_PARTS)
			IMPLEMENT_FP_B_GET(m_HideOverAvaliblyMode,OP_HIDE_OVER_AVALIBLY_MODE)
			IMPLEMENT_FP_B_GET(m_HideOverAvaliblyValue,OP_HIDE_OVER_AVALIBLY_VALUE)

			IMPLEMENT_FP_B_GET(m_BlockHighOverAvaliblyParts,OP_BLOCK_HIGH_OVERAVALIBLY_PARTS)
			IMPLEMENT_FP_B_GET(m_BlockHighOverAvaliblyFactor,OP_BLOCK_HIGH_OVER_AVALIBLY_FACTOR)

			// OverShared
			IMPLEMENT_FP_B_GET(m_HideOverSharedParts,OP_HIDE_OVER_SHARED_PARTS)
			IMPLEMENT_FP_B_GET(m_HideOverSharedMode,OP_HIDE_OVER_SHARED_MODE)
			IMPLEMENT_FP_B_GET(m_HideOverSharedValue,OP_HIDE_OVER_SHARED_VALUE)
			IMPLEMENT_FP_B_GET(m_HideOverSharedCalc,OP_HIDE_OVER_SHARED_CALC)

			IMPLEMENT_FP_B_GET(m_BlockHighOverSharedParts,OP_BLOCK_HIGH_OVER_SHARED_PARTS)
			IMPLEMENT_FP_B_GET(m_BlockHighOverSharedFactor,OP_BLOCK_HIGH_OVER_SHARED_FACTOR)

			// DontHideUnderAvalibly
			IMPLEMENT_FP_B_GET(m_DontHideUnderAvaliblyParts,OP_DONT_HIDE_UNDER_AVALIBLY_PARTS)
			IMPLEMENT_FP_B_GET(m_DontHideUnderAvaliblyMode,OP_DONT_HIDE_UNDER_AVALIBLY_MODE)
			IMPLEMENT_FP_B_GET(m_DontHideUnderAvaliblyValue,OP_DONT_HIDEUNDER_AVALIBLY_VALUE)

			// Other
			IMPLEMENT_FP_B_GET(m_ShowAlwaysSomeParts,OP_SHOW_ALWAYS_SOME_PARTS)
			IMPLEMENT_FP_B_GET(m_ShowAlwaysSomePartsValue,OP_SHOW_ALWAYS_SOME_PARTS_VALUE)

			IMPLEMENT_FP_B_GET(m_ShowAlwaysIncompleteParts,OP_SHOW_ALWAYS_INCOMPLETE_PARTS)
			// NEO: IPS END

			// NEO: SRS - [SmartReleaseSharing]
			IMPLEMENT_FP_B_GET(m_ReleaseMode,OP_RELEASE_MODE)
			IMPLEMENT_FP_B_GET(m_ReleaseLevel,OP_RELEASE_LEVEL)
			IMPLEMENT_FP_B_GET(m_ReleaseTimer,OP_RELEASE_TIMER)

			// release limit
			IMPLEMENT_FP_B_GET(m_ReleaseLimit,OP_RELEASE_LIMIT)
			IMPLEMENT_FP_B_GET(m_ReleaseLimitMode,OP_RELEASE_LIMIT_MODE)
			IMPLEMENT_FP_B_GET(m_ReleaseLimitHigh,OP_RELEASE_LIMIT_HIGH)
			IMPLEMENT_FP_B_GET(m_ReleaseLimitLow,OP_RELEASE_LIMIT_LOW)

			IMPLEMENT_FP_B_GET(m_ReleaseLimitLink,OP_RELEASE_LIMIT_LINK)

			IMPLEMENT_FP_B_GET(m_ReleaseLimitComplete,OP_RELEASE_LIMIT_COMPLETE)
			IMPLEMENT_FP_B_GET(m_ReleaseLimitCompleteMode,OP_RELEASE_LIMIT_COMPLETE_MODE)
			IMPLEMENT_FP_B_GET(m_ReleaseLimitCompleteHigh,OP_RELEASE_LIMIT_COMPLETE_HIGH)
			IMPLEMENT_FP_B_GET(m_ReleaseLimitCompleteLow,OP_RELEASE_LIMIT_COMPLETE_LOW)

			// limit
			IMPLEMENT_FP_B_GET(m_LimitLink,OP_LIMIT_LINK)

			// source limit
			IMPLEMENT_FP_B_GET(m_SourceLimit,OP_SOURCE_LIMIT)
			IMPLEMENT_FP_B_GET(m_SourceLimitMode,OP_SOURCE_LIMIT_MODE)
			IMPLEMENT_FP_B_GET(m_SourceLimitHigh,OP_SOURCE_LIMIT_HIGH)
			IMPLEMENT_FP_B_GET(m_SourceLimitLow,OP_SOURCE_LIMIT_LOW)

			IMPLEMENT_FP_B_GET(m_SourceLimitLink,OP_SOURCE_LIMIT_LINK)

			IMPLEMENT_FP_B_GET(m_SourceLimitComplete,OP_SOURCE_LIMIT_COMPLETE)
			IMPLEMENT_FP_B_GET(m_SourceLimitCompleteMode,OP_SOURCE_LIMIT_COMPLETE_MODE)
			IMPLEMENT_FP_B_GET(m_SourceLimitCompleteHigh,OP_SOURCE_LIMIT_COMPLETE_HIGH)
			IMPLEMENT_FP_B_GET(m_SourceLimitCompleteLow,OP_SOURCE_LIMIT_COMPLETE_LOW)
			// NEO: SRS END

			// NEO: MPS - [ManualPartSharing]
			case OP_PR_PART_ON:{
                ASSERT( newtag->IsInt() );
                SetManagedPart(newtag->GetInt(), PR_PART_ON);
                delete newtag;
                break;
            }
			case OP_PR_PART_HIDEN:{
                ASSERT( newtag->IsInt() );
                SetManagedPart(newtag->GetInt(), PR_PART_HIDEN);
                delete newtag;
                break;
            }
			case OP_PR_PART_OFF:{
                ASSERT( newtag->IsInt() );
                SetManagedPart(newtag->GetInt(), PR_PART_OFF);
                delete newtag;
                break;
            }
			// NEO: MPS END

			default:{
				taglist.Add(newtag);
			}
		}
	}

	CheckTweaks();

	return true;
}

void CKnownPreferences::Save(CIni& ini)
{
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_I_SET(m_EnableLanCast,_T("EnableLanCast"))
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_I_SET(m_EnableVoodoo,_T("EnableVoodoo"))
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	IMPLEMENT_FP_I_SET(m_UseInteligentPartSharing,_T("UseInteligentPartSharing"))
	IMPLEMENT_FP_I_SET(m_InteligentPartSharingTimer,_T("InteligentPartSharingTimer"))

	IMPLEMENT_FP_I_SET(m_MaxProzentToHide,_T("MaxProzentToHide"))

	// OverAvalibly
	IMPLEMENT_FP_I_SET(m_HideOverAvaliblyParts,_T("HideOverAvaliblyParts"))
	IMPLEMENT_FP_I_SET(m_HideOverAvaliblyMode,_T("HideOverAvaliblyMode"))
	IMPLEMENT_FP_I_SET(m_HideOverAvaliblyValue,_T("HideOverAvaliblyValue"))

	IMPLEMENT_FP_I_SET(m_BlockHighOverAvaliblyParts,_T("BlockHighOverAvaliblyParts"))
	IMPLEMENT_FP_I_SET(m_BlockHighOverAvaliblyFactor,_T("BlockHighOverAvaliblyFactor"))

	// OverShared
	IMPLEMENT_FP_I_SET(m_HideOverSharedParts,_T("HideOverSharedParts"))
	IMPLEMENT_FP_I_SET(m_HideOverSharedMode,_T("HideOverSharedMode"))
	IMPLEMENT_FP_I_SET(m_HideOverSharedValue,_T("HideOverSharedValue"))
	IMPLEMENT_FP_I_SET(m_HideOverSharedCalc,_T("HideOverSharedCalc"))

	IMPLEMENT_FP_I_SET(m_BlockHighOverSharedParts,_T("BlockHighOverSharedParts"))
	IMPLEMENT_FP_I_SET(m_BlockHighOverSharedFactor,_T("BlockHighOverSharedFactor"))

	// DontHideUnderAvalibly
	IMPLEMENT_FP_I_SET(m_DontHideUnderAvaliblyParts,_T("DontHideUnderAvaliblyParts"))
	IMPLEMENT_FP_I_SET(m_DontHideUnderAvaliblyMode,_T("DontHideUnderAvaliblyMode"))
	IMPLEMENT_FP_I_SET(m_DontHideUnderAvaliblyValue,_T("DontHideUnderAvaliblyValue"))

	// Other
	IMPLEMENT_FP_I_SET(m_ShowAlwaysSomeParts,_T("ShowAlwaysSomeParts"))
	IMPLEMENT_FP_I_SET(m_ShowAlwaysSomePartsValue,_T("ShowAlwaysSomePartsValue"))

	IMPLEMENT_FP_I_SET(m_ShowAlwaysIncompleteParts,_T("ShowAlwaysIncompleteParts"))
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	IMPLEMENT_FP_I_SET(m_ReleaseMode,_T("ReleaseMode"))
	IMPLEMENT_FP_I_SET(m_ReleaseLevel,_T("ReleaseLevel"))
	IMPLEMENT_FP_I_SET(m_ReleaseTimer,_T("ReleaseTimer"))

	// release limit
	IMPLEMENT_FP_I_SET(m_ReleaseLimit,_T("ReleaseLimit"))
	IMPLEMENT_FP_I_SET(m_ReleaseLimitMode,_T("ReleaseLimitMode"))
	IMPLEMENT_FP_I_SET(m_ReleaseLimitHigh,_T("ReleaseLimitHigh"))
	IMPLEMENT_FP_I_SET(m_ReleaseLimitLow,_T("ReleaseLimitLow"))

	IMPLEMENT_FP_I_SET(m_ReleaseLimitLink,_T("ReleaseLimitLink"))

	IMPLEMENT_FP_I_SET(m_ReleaseLimitComplete,_T("ReleaseLimitComplete"))
	IMPLEMENT_FP_I_SET(m_ReleaseLimitCompleteMode,_T("ReleaseLimitCompleteMode"))
	IMPLEMENT_FP_I_SET(m_ReleaseLimitCompleteHigh,_T("ReleaseLimitCompleteHigh"))
	IMPLEMENT_FP_I_SET(m_ReleaseLimitCompleteLow,_T("ReleaseLimitCompleteLow"))

	// limit
	IMPLEMENT_FP_I_SET(m_LimitLink,_T("LimitLink"))

	// source limit
	IMPLEMENT_FP_I_SET(m_SourceLimit,_T("SourceLimit"))
	IMPLEMENT_FP_I_SET(m_SourceLimitMode,_T("SourceLimitMode"))
	IMPLEMENT_FP_I_SET(m_SourceLimitHigh,_T("SourceLimitHigh"))
	IMPLEMENT_FP_I_SET(m_SourceLimitLow,_T("SourceLimitLow"))

	IMPLEMENT_FP_I_SET(m_SourceLimitLink,_T("SourceLimitLink"))

	IMPLEMENT_FP_I_SET(m_SourceLimitComplete,_T("SourceLimitComplete"))
	IMPLEMENT_FP_I_SET(m_SourceLimitCompleteMode,_T("SourceLimitCompleteMode"))
	IMPLEMENT_FP_I_SET(m_SourceLimitCompleteHigh,_T("SourceLimitCompleteHigh"))
	IMPLEMENT_FP_I_SET(m_SourceLimitCompleteLow,_T("SourceLimitCompleteLow"))
	// NEO: SRS END
}

bool CKnownPreferences::Load(CIni& ini)
{
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_I_GET(m_EnableLanCast,_T("EnableLanCast"),TRUE)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_I_GET(m_EnableVoodoo,_T("EnableVoodoo"),TRUE)
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	IMPLEMENT_FP_I_GET(m_UseInteligentPartSharing,_T("UseInteligentPartSharing"),FALSE)
	IMPLEMENT_FP_I_GET(m_InteligentPartSharingTimer,_T("InteligentPartSharingTimer"),DEF_INTELIGENT_PART_SHARING_TIMER)

	IMPLEMENT_FP_I_GET(m_MaxProzentToHide,_T("MaxProzentToHide"),DEF_MAX_PROZENT_TO_HIDE)

	// OverAvalibly
	IMPLEMENT_FP_I_GET(m_HideOverAvaliblyParts,_T("HideOverAvaliblyParts"),IPS_KF)
	IMPLEMENT_FP_I_GET(m_HideOverAvaliblyMode,_T("HideOverAvaliblyMode"),IPS_MLT)
	IMPLEMENT_FP_I_GET2(m_HideOverAvaliblyValue,_T("HideOverAvaliblyValue"),HIDE_OVER_AVALIBLY_VALUE,m_HideOverAvaliblyMode,IPS_MLT)

	IMPLEMENT_FP_I_GET(m_BlockHighOverAvaliblyParts,_T("BlockHighOverAvaliblyParts"),IPS_OFF)
	IMPLEMENT_FP_I_GET(m_BlockHighOverAvaliblyFactor,_T("BlockHighOverAvaliblyFactor"),DEF_BLOCK_HIGH_OVER_AVALIBLY_FACTOR)

	// OverShared
	IMPLEMENT_FP_I_GET(m_HideOverSharedParts,_T("HideOverSharedParts"),IPS_OFF)
	IMPLEMENT_FP_I_GET(m_HideOverSharedMode,_T("HideOverSharedMode"),IPS_MLT)
	IMPLEMENT_FP_I_GET2(m_HideOverSharedValue,_T("HideOverSharedValue"),HIDE_OVER_SHARED_VALUE,m_HideOverSharedMode,IPS_MLT)
	IMPLEMENT_FP_I_GET(m_HideOverSharedCalc,_T("HideOverSharedCalc"),IPS_LO)

	IMPLEMENT_FP_I_GET(m_BlockHighOverSharedParts,_T("BlockHighOverSharedParts"),IPS_OFF)
	IMPLEMENT_FP_I_GET(m_BlockHighOverSharedFactor,_T("BlockHighOverSharedFactor"),DEF_BLOCK_HIGH_OVER_SHARED_FACTOR)

	// DontHideUnderAvalibly
	IMPLEMENT_FP_I_GET(m_DontHideUnderAvaliblyParts,_T("DontHideUnderAvaliblyParts"),IPS_ON)
	IMPLEMENT_FP_I_GET(m_DontHideUnderAvaliblyMode,_T("DontHideUnderAvaliblyMode"),IPS_MLT)
	IMPLEMENT_FP_I_GET2(m_DontHideUnderAvaliblyValue,_T("DontHideUnderAvaliblyValue"),DONT_HIDEUNDER_AVALIBLY_VALUE,m_DontHideUnderAvaliblyMode,IPS_MLT)

	// Other
	IMPLEMENT_FP_I_GET(m_ShowAlwaysSomeParts,_T("ShowAlwaysSomeParts"),IPS_ON)
	IMPLEMENT_FP_I_GET(m_ShowAlwaysSomePartsValue,_T("ShowAlwaysSomePartsValue"),DEF_SHOW_ALWAYS_SOME_PARTS_VALUE)

	IMPLEMENT_FP_I_GET(m_ShowAlwaysIncompleteParts,_T("ShowAlwaysIncompleteParts"),IPS_ON)
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	IMPLEMENT_FP_I_GET(m_ReleaseMode,_T("ReleaseMode"),REL_MIXED)
	IMPLEMENT_FP_I_GET(m_ReleaseLevel,_T("ReleaseLevel"),DEF_RELEASE_LEVEL)
	IMPLEMENT_FP_I_GET(m_ReleaseTimer,_T("ReleaseTimer"),DEF_RELEASE_TIMER)

	// release limit
	IMPLEMENT_FP_I_GET(m_ReleaseLimit,_T("ReleaseLimit"),LIM_BOOTH)
	IMPLEMENT_FP_I_GET(m_ReleaseLimitMode,_T("ReleaseLimitMode"),REL_EXPONENTIAL)
	IMPLEMENT_FP_I_GET(m_ReleaseLimitHigh,_T("ReleaseLimitHigh"),DEF_RELEASE_LIMIT_HIGH)
	IMPLEMENT_FP_I_GET(m_ReleaseLimitLow,_T("ReleaseLimitLow"),DEF_RELEASE_LIMIT_LOW)

	IMPLEMENT_FP_I_GET(m_ReleaseLimitLink,_T("ReleaseLimitLink"),LNK_AND)

	IMPLEMENT_FP_I_GET(m_ReleaseLimitComplete,_T("ReleaseLimitComplete"),LIM_BOOTH)
	IMPLEMENT_FP_I_GET(m_ReleaseLimitCompleteMode,_T("ReleaseLimitCompleteMode"),REL_LINEAR)
	IMPLEMENT_FP_I_GET(m_ReleaseLimitCompleteHigh,_T("ReleaseLimitCompleteHigh"),DEF_RELEASE_LIMIT_COMPLETE_HIGH)
	IMPLEMENT_FP_I_GET(m_ReleaseLimitCompleteLow,_T("ReleaseLimitCompleteLow"),DEF_RELEASE_LIMIT_COMPLETE_LOW)

	// limit
	IMPLEMENT_FP_I_GET(m_LimitLink,_T("LimitLink"),LNK_OR)

	// source limit
	IMPLEMENT_FP_I_GET(m_SourceLimit,_T("SourceLimit"),LIM_BOOTH)
	IMPLEMENT_FP_I_GET(m_SourceLimitMode,_T("SourceLimitMode"),REL_EXPONENTIAL)
	IMPLEMENT_FP_I_GET(m_SourceLimitHigh,_T("SourceLimitHigh"),DEF_SOURCE_LIMIT_HIGH)
	IMPLEMENT_FP_I_GET(m_SourceLimitLow,_T("SourceLimitLow"),DEF_SOURCE_LIMIT_LOW)

	IMPLEMENT_FP_I_GET(m_SourceLimitLink,_T("SourceLimitLink"),LNK_AND)

	IMPLEMENT_FP_I_GET(m_SourceLimitComplete,_T("SourceLimitComplete"),LIM_BOOTH)
	IMPLEMENT_FP_I_GET(m_SourceLimitCompleteMode,_T("SourceLimitCompleteMode"),REL_LINEAR)
	IMPLEMENT_FP_I_GET(m_SourceLimitCompleteHigh,_T("SourceLimitCompleteHigh"),DEF_SOURCE_LIMIT_COMPLETE_HIGH)
	IMPLEMENT_FP_I_GET(m_SourceLimitCompleteLow,_T("SourceLimitCompleteLow"),DEF_SOURCE_LIMIT_COMPLETE_LOW)
	// NEO: SRS END

	CheckTweaks();

	return true;
}

void CKnownPreferences::CheckTweaks(){
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_CHK_FLAG(m_EnableLanCast,TRUE,1)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_CHK_FLAG(m_EnableVoodoo,TRUE,1)
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	IMPLEMENT_FP_CHK_FLAG(m_UseInteligentPartSharing,FALSE,1)
	IMPLEMENT_FP_CHK_VAL(m_InteligentPartSharingTimer,INTELIGENT_PART_SHARING_TIMER)

	IMPLEMENT_FP_CHK_VAL(m_MaxProzentToHide,MAX_PROZENT_TO_HIDE)

	// OverAvalibly
	IMPLEMENT_FP_CHK_FLAG(m_HideOverAvaliblyParts,IPS_KF,3)
	IMPLEMENT_FP_CHK_FLAG(m_HideOverAvaliblyMode,IPS_MLT,1)
	IMPLEMENT_FP_CHK_VAL2(m_HideOverAvaliblyValue,HIDE_OVER_AVALIBLY_VALUE,m_HideOverAvaliblyMode)

	IMPLEMENT_FP_CHK_FLAG(m_BlockHighOverAvaliblyParts,IPS_OFF,3)
	IMPLEMENT_FP_CHK_VAL(m_BlockHighOverAvaliblyFactor,BLOCK_HIGH_OVER_AVALIBLY_FACTOR)

	// OverShared
	IMPLEMENT_FP_CHK_FLAG(m_HideOverSharedParts,IPS_OFF,3)
	IMPLEMENT_FP_CHK_FLAG(m_HideOverSharedMode,IPS_MLT,1)
	IMPLEMENT_FP_CHK_VAL2(m_HideOverSharedValue,HIDE_OVER_SHARED_VALUE,m_HideOverSharedMode)
	IMPLEMENT_FP_CHK_FLAG(m_HideOverSharedCalc,IPS_LO,1)

	IMPLEMENT_FP_CHK_FLAG(m_BlockHighOverSharedParts,IPS_OFF,3)
	IMPLEMENT_FP_CHK_VAL(m_BlockHighOverSharedFactor,BLOCK_HIGH_OVER_SHARED_FACTOR)

	// DontHideUnderAvalibly
	IMPLEMENT_FP_CHK_FLAG(m_DontHideUnderAvaliblyParts,IPS_ON,3)
	IMPLEMENT_FP_CHK_FLAG(m_DontHideUnderAvaliblyMode,IPS_MLT,1)
	IMPLEMENT_FP_CHK_VAL2(m_DontHideUnderAvaliblyValue,DONT_HIDEUNDER_AVALIBLY_VALUE,m_DontHideUnderAvaliblyMode)

	// Other
	IMPLEMENT_FP_CHK_FLAG(m_ShowAlwaysSomeParts,IPS_ON,3)
	IMPLEMENT_FP_CHK_VAL(m_ShowAlwaysSomePartsValue,SHOW_ALWAYS_SOME_PARTS_VALUE)

	IMPLEMENT_FP_CHK_FLAG(m_ShowAlwaysIncompleteParts,IPS_ON,3)
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	IMPLEMENT_FP_CHK_FLAG(m_ReleaseMode,REL_MIXED,2)
	IMPLEMENT_FP_CHK_VAL(m_ReleaseLevel,RELEASE_LEVEL)
	IMPLEMENT_FP_CHK_VAL(m_ReleaseTimer,RELEASE_TIMER)

	// release limit
	IMPLEMENT_FP_CHK_FLAG(m_ReleaseLimit,LIM_BOOTH,2)
	IMPLEMENT_FP_CHK_FLAG(m_ReleaseLimitMode,REL_EXPONENTIAL,2)
	IMPLEMENT_FP_CHK_VAL(m_ReleaseLimitHigh,RELEASE_LIMIT_HIGH)
	IMPLEMENT_FP_CHK_VAL(m_ReleaseLimitLow,RELEASE_LIMIT_LOW)

	IMPLEMENT_FP_CHK_FLAG(m_ReleaseLimitLink,LNK_AND,1)

	IMPLEMENT_FP_CHK_FLAG(m_ReleaseLimitComplete,LIM_BOOTH,2)
	IMPLEMENT_FP_CHK_FLAG(m_ReleaseLimitCompleteMode,REL_LINEAR,2)
	IMPLEMENT_FP_CHK_VAL(m_ReleaseLimitCompleteHigh,RELEASE_LIMIT_COMPLETE_HIGH)
	IMPLEMENT_FP_CHK_VAL(m_ReleaseLimitCompleteLow,RELEASE_LIMIT_COMPLETE_LOW)

	// limit
	IMPLEMENT_FP_CHK_FLAG(m_LimitLink,LNK_OR,1)

	// source limit
	IMPLEMENT_FP_CHK_FLAG(m_SourceLimit,LIM_BOOTH,2)
	IMPLEMENT_FP_CHK_FLAG(m_SourceLimitMode,REL_EXPONENTIAL,2)
	IMPLEMENT_FP_CHK_VAL(m_SourceLimitHigh,SOURCE_LIMIT_HIGH)
	IMPLEMENT_FP_CHK_VAL(m_SourceLimitLow,SOURCE_LIMIT_LOW)

	IMPLEMENT_FP_CHK_FLAG(m_SourceLimitLink,LNK_AND,1)

	IMPLEMENT_FP_CHK_FLAG(m_SourceLimitComplete,LIM_BOOTH,2)
	IMPLEMENT_FP_CHK_FLAG(m_SourceLimitCompleteMode,REL_LINEAR,2)
	IMPLEMENT_FP_CHK_VAL(m_SourceLimitCompleteHigh,SOURCE_LIMIT_COMPLETE_HIGH)
	IMPLEMENT_FP_CHK_VAL(m_SourceLimitCompleteLow,SOURCE_LIMIT_COMPLETE_LOW)
	// NEO: SRS END
}

////////////////////////////////////////////////////////////////////////////////////////
// CPartPreferences
//

CPartPreferences::CPartPreferences(){

}

CPartPreferences::~CPartPreferences(){
	ClearTags();
}

void CPartPreferences::Save(CFileDataIO* file)
{
	UINT uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_B_SET(m_LcIntervals,OP_LC_INTERVALS)

	IMPLEMENT_FP_B_SET(m_LanSourceReaskTime,OL_LAN_SOURCE_REASK_TIME)
	IMPLEMENT_FP_B_SET(m_LanNNPSourceReaskTime,OL_LAN_NNP_SOURCE_REASK_TIME)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_B_SET(m_VoodooXS,OP_VOODOO_XS)
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	IMPLEMENT_FP_B_SET(m_SourceLimit,OP_MAX_SOURCE)

	// Management
	IMPLEMENT_FP_B_SET(m_SwapLimit,OP_SWAP_LIMIT)

	//XS
	IMPLEMENT_FP_B_SET(m_XsEnable,OP_XS_ENABLE)
	IMPLEMENT_FP_B_SET(m_XsLimit,OP_XS_LIMIT)
	IMPLEMENT_FP_B_SET(m_XsIntervals,OP_XS_INTERVALS)
	IMPLEMENT_FP_B_SET(m_XsClientIntervals,OP_XS_CLIENT_INTERVALS)
	IMPLEMENT_FP_B_SET(m_XsCleintDelay,OP_XS_CLEINT_DELAY)
	IMPLEMENT_FP_B_SET(m_XsRareLimit,OP_XS_RARE_LIMIT)

	// SVR
	IMPLEMENT_FP_B_SET(m_SvrEnable,OP_SVR_ENABLE)
	IMPLEMENT_FP_B_SET(m_SvrLimit,OP_SVR_LIMIT)
	IMPLEMENT_FP_B_SET(m_SvrIntervals,OP_SVR_INTERVALS)

	//KAD
	IMPLEMENT_FP_B_SET(m_KadEnable,OP_KAD_ENABLE)
	IMPLEMENT_FP_B_SET(m_KadLimit,OP_KAD_LIMIT)
	IMPLEMENT_FP_B_SET(m_KadIntervals,OP_KAD_INTERVALS)
	IMPLEMENT_FP_B_SET(m_KadMaxFiles,OP_KAD_MAX_FILES)
	IMPLEMENT_FP_B_SET(m_KadRepeatDelay,OP_KAD_REPEAT_DELAY)

	//UDP
	IMPLEMENT_FP_B_SET(m_UdpEnable,OP_UDP_ENABLE)
	IMPLEMENT_FP_B_SET(m_UdpLimit,OP_UDP_LIMIT)
	IMPLEMENT_FP_B_SET(m_UdpIntervals,OP_UDP_INTERVALS)
	IMPLEMENT_FP_B_SET(m_UdpGlobalIntervals,OP_UDP_GLOBAL_INTERVALS)
	IMPLEMENT_FP_B_SET(m_UdpFilesPerServer,OP_UDP_FILES_PER_SERVER)
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	IMPLEMENT_FP_B_SET(m_UseSourceCache,OP_USE_SOURCE_CACHE)
	IMPLEMENT_FP_B_SET(m_SourceCacheLimit,OP_SOURCE_CACHE_LIMIT)
	IMPLEMENT_FP_B_SET(m_SourceCacheTime,OP_SOURCE_CACHE_TIME)
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	IMPLEMENT_FP_B_SET(m_AutoSoftLock,OP_AUTO_SOFT_LOCK)
	IMPLEMENT_FP_B_SET(m_AutoSoftLockLimit,OP_AUTO_SOFT_LOCK_LIMIT)
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	IMPLEMENT_FP_B_SET(m_AutoHardLimit,OP_AUTO_HARD_LIMIT)
	IMPLEMENT_FP_B_SET(m_AutoHardLimitTime,OP_AUTO_HARD_LIMIT_TIME)
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	IMPLEMENT_FP_B_SET(m_CategorySourceLimit,OP_CATEGORY_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_CategorySourceLimitLimit,OP_CATEGORY_SOURCE_LIMIT_LIMIT)
	IMPLEMENT_FP_B_SET(m_CategorySourceLimitTime,OP_CATEGORY_SOURCE_LIMIT_TIME)
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	IMPLEMENT_FP_B_SET(m_GlobalSourceLimit,OP_GLOBAL_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_GlobalSourceLimitLimit,OP_GLOBAL_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_GlobalSourceLimitTime,OP_GLOBAL_SOURCE_LIMIT_TIME)
	// NEO: GSL END
	IMPLEMENT_FP_B_SET(m_MinSourcePerFile,OP_MIN_SOURCE_PER_FILE)

	IMPLEMENT_FP_B_SET(m_TCPConnectionRetry,OP_TCP_CONNECTION_RETRY) // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	IMPLEMENT_FP_B_SET(m_SpreadReaskEnable,OP_SPREAD_REASK_ENABLE)
	IMPLEMENT_FP_B_SET(m_SpreadReaskTime,OP_SPREAD_REASK_TIME)
	IMPLEMENT_FP_B_SET(m_SourceReaskTime,OP_SOURCE_REASK_TIME)
	IMPLEMENT_FP_B_SET(m_FullQSourceReaskTime,OP_FULLQ_SOURCE_REASK_TIME)
	IMPLEMENT_FP_B_SET(m_NNPSourceReaskTime,OP_NNP_SOURCE_REASK_TIME)
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	IMPLEMENT_FP_B_SET(m_DropTime,OP_DROP_TIME)

	//Bad
	IMPLEMENT_FP_B_SET(m_BadSourceDrop,OP_BAD_SOURCE_DROP)
	IMPLEMENT_FP_B_SET(m_BadSourceLimitMode,OP_BAD_SOURCE_LIMIT_MODE)
	IMPLEMENT_FP_B_SET(m_BadSourceLimit,OP_BAD_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_BadSourceDropMode,OP_BAD_SOURCE_DROP_MODE)
	IMPLEMENT_FP_B_SET(m_BadSourceDropTime,OP_BAD_SOURCE_DROP_TIME)

	//NNP
	IMPLEMENT_FP_B_SET(m_NNPSourceDrop,OP_NNP_SOURCE_DROP)
	IMPLEMENT_FP_B_SET(m_NNPSourceLimitMode,OP_NNP_SOURCE_LIMIT_MODE)
	IMPLEMENT_FP_B_SET(m_NNPSourceLimit,OP_NNP_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_NNPSourceDropMode,OP_NNP_SOURCE_DROP_MODE)
	IMPLEMENT_FP_B_SET(m_NNPSourceDropTime,OP_NNP_SOURCE_DROP_TIME)

	//FullQ
	IMPLEMENT_FP_B_SET(m_FullQSourceDrop,OP_FULLQ_SOURCE_DROP)
	IMPLEMENT_FP_B_SET(m_FullQSourceLimitMode,OP_FULLQ_SOURCE_LIMIT_MODE)
	IMPLEMENT_FP_B_SET(m_FullQSourceLimit,OP_FULLQ_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_FullQSourceDropMode,OP_FULLQ_SOURCE_DROP_MODE)
	IMPLEMENT_FP_B_SET(m_FullQSourceDropTime,OP_FULLQ_SOURCE_DROP_TIME)

	//HighQ
	IMPLEMENT_FP_B_SET(m_HighQSourceDrop,OP_HIGHQ_SOURCE_DROP)
	IMPLEMENT_FP_B_SET(m_HighQSourceLimitMode,OP_HIGHQ_SOURCE_LIMIT_MODE)
	IMPLEMENT_FP_B_SET(m_HighQSourceLimit,OP_HIGHQ_SOURCE_LIMIT)
	IMPLEMENT_FP_B_SET(m_HighQSourceDropMode,OP_HIGHQ_SOURCE_DROP_MODE)
	IMPLEMENT_FP_B_SET(m_HighQSourceDropTime,OP_HIGHQ_SOURCE_DROP_TIME)
	IMPLEMENT_FP_B_SET(m_HighQSourceRankMode,OP_HIGHQ_SOURCE_RANK_MODE)
	IMPLEMENT_FP_B_SET(m_HighQSourceMaxRank,OP_HIGHQ_SOURCE_MAX_RANK)

	IMPLEMENT_FP_B_SET(m_DeadTime,OP_DEAD_TIME)
	IMPLEMENT_FP_B_SET(m_DeadTimeFWMulti,OP_DEAD_TIME_FW_MILTU)
	IMPLEMENT_FP_B_SET(m_GlobalDeadTime,OP_GLOBAL_DEAD_TIME)
	IMPLEMENT_FP_B_SET(m_GlobalDeadTimeFWMulti,OP_GLOBAL_DEAD_TIME_FW_MILTU)
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	IMPLEMENT_FP_B_SET(m_AutoSaveSources,OP_AUTO_SAVE_SOURCES)
	IMPLEMENT_FP_B_SET(m_AutoSaveSourcesIntervals,OP_AUTO_SAVE_SOURCES_INTERVALS)
	IMPLEMENT_FP_B_SET(m_SourceStorageLimit,OP_SOURCE_STORAGE_LIMIT)

	IMPLEMENT_FP_B_SET(m_StoreAlsoA4AFSources,OP_STORE_ALSO_A4AF_SOURCES)

	IMPLEMENT_FP_B_SET(m_AutoLoadSources,OP_AUTO_LOAD_SOURCES)
	IMPLEMENT_FP_B_SET(m_LoadedSourceCleanUpTime,OP_LOADED_SOURCE_CLEAN_UP_TIME)
	IMPLEMENT_FP_B_SET(m_SourceStorageReaskLimit,OP_SOURCE_STORAGE_REASK_LIMIT)

	IMPLEMENT_FP_B_SET(m_TotalSourceRestore,OP_TOTAL_SOURCE_RESTORE)

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	IMPLEMENT_FP_B_SET(m_ReaskPropability,OP_REASK_PROPABILITY)
	IMPLEMENT_FP_B_SET(m_UnpredictedPropability,OP_UNPREDICTED_PROPABILITY)
	IMPLEMENT_FP_B_SET(m_UnpredictedReaskPropability,OP_UNPREDICTED_REASK_PROPABILITY)
 #endif // NEO_SA // NEO: NSA END

	IMPLEMENT_FP_B_SET(m_AutoReaskStoredSourcesDelay,OP_AUTO_REASK_STORED_SOURCES_DELAY)

	IMPLEMENT_FP_B_SET(m_GroupStoredSourceReask,OP_GROUP_STORED_SOURCE_REASK)
	IMPLEMENT_FP_B_SET(m_StoredSourceGroupIntervals,OP_STORED_SOURCE_GROUP_INTERVALS)
	IMPLEMENT_FP_B_SET(m_StoredSourceGroupSize,OP_STORED_SOURCE_GROUP_SIZE)
#endif // NEO_SS // NEO: NSS END

	IMPLEMENT_FP_B_SET(m_ForceA4AF,OP_FORCE_A4AF) // NEO: NXC - [NewExtendedCategories] 

	// NEO: MCS - [ManualChunkSelection]
	POSITION	pos = m_WantedParts.GetStartPosition();
	UINT		part;
	uint8		status;
	while(pos)
	{
		m_WantedParts.GetNextAssoc(pos, part, status);
		if(status != PR_PART_NORMAL)
		{
			CTag tag(OP_PR_PART_WANTED, part);
			tag.WriteNewEd2kTag(file);
			uTagCount++;
		}
	}
	// NEO: MCS END

	for (int j = 0; j < taglist.GetCount(); j++){
		if (taglist[j]->IsStr() || taglist[j]->IsInt()){
			taglist[j]->WriteNewEd2kTag(file);
			uTagCount++;
		}
	}

	file->Seek(uTagCountFilePos, CFile::begin);
	file->WriteUInt32(uTagCount);
	file->Seek(0, CFile::end);
}

void CPartPreferences::ClearTags()
{
	for (int i = 0; i < taglist.GetSize(); i++)
		delete taglist[i];
	taglist.RemoveAll();
}

bool CPartPreferences::Load(CFileDataIO* file)
{
	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
		switch (newtag->GetNameID()){

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
			IMPLEMENT_FP_B_GET(m_LcIntervals,OP_LC_INTERVALS)

			IMPLEMENT_FP_B_GET(m_LanSourceReaskTime,OL_LAN_SOURCE_REASK_TIME)
			IMPLEMENT_FP_B_GET(m_LanNNPSourceReaskTime,OL_LAN_NNP_SOURCE_REASK_TIME)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			IMPLEMENT_FP_B_GET(m_VoodooXS,OP_VOODOO_XS)
#endif // VOODOO // NEO: VOODOO END

			// NEO: SRT - [SourceRequestTweaks]
			// General
			IMPLEMENT_FP_B_GET(m_SourceLimit,OP_MAX_SOURCE)

			// Management
			IMPLEMENT_FP_B_GET(m_SwapLimit,OP_SWAP_LIMIT)

			//XS
			IMPLEMENT_FP_B_GET(m_XsEnable,OP_XS_ENABLE)
			IMPLEMENT_FP_B_GET(m_XsLimit,OP_XS_LIMIT)
			IMPLEMENT_FP_B_GET(m_XsIntervals,OP_XS_INTERVALS)
			IMPLEMENT_FP_B_GET(m_XsClientIntervals,OP_XS_CLIENT_INTERVALS)
			IMPLEMENT_FP_B_GET(m_XsCleintDelay,OP_XS_CLEINT_DELAY)
			IMPLEMENT_FP_B_GET(m_XsRareLimit,OP_XS_RARE_LIMIT)

			// SVR
			IMPLEMENT_FP_B_GET(m_SvrEnable,OP_SVR_ENABLE)
			IMPLEMENT_FP_B_GET(m_SvrLimit,OP_SVR_LIMIT)
			IMPLEMENT_FP_B_GET(m_SvrIntervals,OP_SVR_INTERVALS)

			//KAD
			IMPLEMENT_FP_B_GET(m_KadEnable,OP_KAD_ENABLE)
			IMPLEMENT_FP_B_GET(m_KadLimit,OP_KAD_LIMIT)
			IMPLEMENT_FP_B_GET(m_KadIntervals,OP_KAD_INTERVALS)
			IMPLEMENT_FP_B_GET(m_KadMaxFiles,OP_KAD_MAX_FILES)
			IMPLEMENT_FP_B_GET(m_KadRepeatDelay,OP_KAD_REPEAT_DELAY)

			//UDP
			IMPLEMENT_FP_B_GET(m_UdpEnable,OP_UDP_ENABLE)
			IMPLEMENT_FP_B_GET(m_UdpLimit,OP_UDP_LIMIT)
			IMPLEMENT_FP_B_GET(m_UdpIntervals,OP_UDP_INTERVALS)
			IMPLEMENT_FP_B_GET(m_UdpGlobalIntervals,OP_UDP_GLOBAL_INTERVALS)
			IMPLEMENT_FP_B_GET(m_UdpFilesPerServer,OP_UDP_FILES_PER_SERVER)
			// NEO: SRT END

			// NEO: XSC - [ExtremeSourceCache]
			IMPLEMENT_FP_B_GET(m_UseSourceCache,OP_USE_SOURCE_CACHE)
			IMPLEMENT_FP_B_GET(m_SourceCacheLimit,OP_SOURCE_CACHE_LIMIT)
			IMPLEMENT_FP_B_GET(m_SourceCacheTime,OP_SOURCE_CACHE_TIME)
			// NEO: XSC END

			// NEO: ASL - [AutoSoftLock]
			IMPLEMENT_FP_B_GET(m_AutoSoftLock,OP_AUTO_SOFT_LOCK)
			IMPLEMENT_FP_B_GET(m_AutoSoftLockLimit,OP_AUTO_SOFT_LOCK_LIMIT)
			// NEO: ASL END

			// NEO: AHL - [AutoHardLimit]
			IMPLEMENT_FP_B_GET(m_AutoHardLimit,OP_AUTO_HARD_LIMIT)
			IMPLEMENT_FP_B_GET(m_AutoHardLimitTime,OP_AUTO_HARD_LIMIT_TIME)
			// NEO: AHL END

			// NEO: CSL - [CategorySourceLimit]
			IMPLEMENT_FP_B_GET(m_CategorySourceLimit,OP_CATEGORY_SOURCE_LIMIT)
			IMPLEMENT_FP_B_GET(m_CategorySourceLimitLimit,OP_CATEGORY_SOURCE_LIMIT_LIMIT)
			IMPLEMENT_FP_B_GET(m_CategorySourceLimitTime,OP_CATEGORY_SOURCE_LIMIT_TIME)
			// NEO: CSL END

			// NEO: GSL - [GlobalSourceLimit]
			IMPLEMENT_FP_B_GET(m_GlobalSourceLimit,OP_GLOBAL_SOURCE_LIMIT)
			IMPLEMENT_FP_B_GET(m_GlobalSourceLimitLimit,OP_GLOBAL_SOURCE_LIMIT_LIMIT)
			IMPLEMENT_FP_B_GET(m_GlobalSourceLimitTime,OP_GLOBAL_SOURCE_LIMIT_TIME)
			// NEO: GSL END

			IMPLEMENT_FP_B_GET(m_MinSourcePerFile,OP_MIN_SOURCE_PER_FILE)

			IMPLEMENT_FP_B_GET(m_TCPConnectionRetry,OP_TCP_CONNECTION_RETRY) // NEO: TCR - [TCPConnectionRetry]

			// NEO: DRT - [DownloadReaskTweaks]
			IMPLEMENT_FP_B_GET(m_SpreadReaskEnable,OP_SPREAD_REASK_ENABLE)
			IMPLEMENT_FP_B_GET(m_SpreadReaskTime,OP_SPREAD_REASK_TIME)
			IMPLEMENT_FP_B_GET(m_SourceReaskTime,OP_SOURCE_REASK_TIME)
			IMPLEMENT_FP_B_GET(m_FullQSourceReaskTime,OP_FULLQ_SOURCE_REASK_TIME)
			IMPLEMENT_FP_B_GET(m_NNPSourceReaskTime,OP_NNP_SOURCE_REASK_TIME)
			// NEO: DRT END

			// NEO: SDT - [SourcesDropTweaks]
			IMPLEMENT_FP_B_GET(m_DropTime,OP_DROP_TIME)

			//Bad
			IMPLEMENT_FP_B_GET(m_BadSourceDrop,OP_BAD_SOURCE_DROP)
			IMPLEMENT_FP_B_GET(m_BadSourceLimitMode,OP_BAD_SOURCE_LIMIT_MODE)
			IMPLEMENT_FP_B_GET(m_BadSourceLimit,OP_BAD_SOURCE_LIMIT)
			IMPLEMENT_FP_B_GET(m_BadSourceDropMode,OP_BAD_SOURCE_DROP_MODE)
			IMPLEMENT_FP_B_GET(m_BadSourceDropTime,OP_BAD_SOURCE_DROP_TIME)

			//NNP
			IMPLEMENT_FP_B_GET(m_NNPSourceDrop,OP_NNP_SOURCE_DROP)
			IMPLEMENT_FP_B_GET(m_NNPSourceLimitMode,OP_NNP_SOURCE_LIMIT_MODE)
			IMPLEMENT_FP_B_GET(m_NNPSourceLimit,OP_NNP_SOURCE_LIMIT)
			IMPLEMENT_FP_B_GET(m_NNPSourceDropMode,OP_NNP_SOURCE_DROP_MODE)
			IMPLEMENT_FP_B_GET(m_NNPSourceDropTime,OP_NNP_SOURCE_DROP_TIME)

			//FullQ
			IMPLEMENT_FP_B_GET(m_FullQSourceDrop,OP_FULLQ_SOURCE_DROP)
			IMPLEMENT_FP_B_GET(m_FullQSourceLimitMode,OP_FULLQ_SOURCE_LIMIT_MODE)
			IMPLEMENT_FP_B_GET(m_FullQSourceLimit,OP_FULLQ_SOURCE_LIMIT)
			IMPLEMENT_FP_B_GET(m_FullQSourceDropMode,OP_FULLQ_SOURCE_DROP_MODE)
			IMPLEMENT_FP_B_GET(m_FullQSourceDropTime,OP_FULLQ_SOURCE_DROP_TIME)

			//HighQ
			IMPLEMENT_FP_B_GET(m_HighQSourceDrop,OP_HIGHQ_SOURCE_DROP)
			IMPLEMENT_FP_B_GET(m_HighQSourceLimitMode,OP_HIGHQ_SOURCE_LIMIT_MODE)
			IMPLEMENT_FP_B_GET(m_HighQSourceLimit,OP_HIGHQ_SOURCE_LIMIT)
			IMPLEMENT_FP_B_GET(m_HighQSourceDropMode,OP_HIGHQ_SOURCE_DROP_MODE)
			IMPLEMENT_FP_B_GET(m_HighQSourceDropTime,OP_HIGHQ_SOURCE_DROP_TIME)
			IMPLEMENT_FP_B_GET(m_HighQSourceRankMode,OP_HIGHQ_SOURCE_RANK_MODE)
			IMPLEMENT_FP_B_GET(m_HighQSourceMaxRank,OP_HIGHQ_SOURCE_MAX_RANK)

			IMPLEMENT_FP_B_GET(m_DeadTime,OP_DEAD_TIME)
			IMPLEMENT_FP_B_GET(m_DeadTimeFWMulti,OP_DEAD_TIME_FW_MILTU)
			IMPLEMENT_FP_B_GET(m_GlobalDeadTime,OP_GLOBAL_DEAD_TIME)
			IMPLEMENT_FP_B_GET(m_GlobalDeadTimeFWMulti,OP_GLOBAL_DEAD_TIME_FW_MILTU)
			// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			IMPLEMENT_FP_B_GET(m_AutoSaveSources,OP_AUTO_SAVE_SOURCES)
			IMPLEMENT_FP_B_GET(m_AutoSaveSourcesIntervals,OP_AUTO_SAVE_SOURCES_INTERVALS)
			IMPLEMENT_FP_B_GET(m_SourceStorageLimit,OP_SOURCE_STORAGE_LIMIT)

			IMPLEMENT_FP_B_GET(m_StoreAlsoA4AFSources,OP_STORE_ALSO_A4AF_SOURCES)

			IMPLEMENT_FP_B_GET(m_AutoLoadSources,OP_AUTO_LOAD_SOURCES)
			IMPLEMENT_FP_B_GET(m_LoadedSourceCleanUpTime,OP_LOADED_SOURCE_CLEAN_UP_TIME)
			IMPLEMENT_FP_B_GET(m_SourceStorageReaskLimit,OP_SOURCE_STORAGE_REASK_LIMIT)

			IMPLEMENT_FP_B_GET(m_TotalSourceRestore,OP_TOTAL_SOURCE_RESTORE)

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
			IMPLEMENT_FP_B_GET(m_ReaskPropability,OP_REASK_PROPABILITY)
			IMPLEMENT_FP_B_GET(m_UnpredictedPropability,OP_UNPREDICTED_PROPABILITY)
			IMPLEMENT_FP_B_GET(m_UnpredictedReaskPropability,OP_UNPREDICTED_REASK_PROPABILITY)
 #endif // NEO_SA // NEO: NSA END

			IMPLEMENT_FP_B_GET(m_AutoReaskStoredSourcesDelay,OP_AUTO_REASK_STORED_SOURCES_DELAY)

			IMPLEMENT_FP_B_GET(m_GroupStoredSourceReask,OP_GROUP_STORED_SOURCE_REASK)
			IMPLEMENT_FP_B_GET(m_StoredSourceGroupIntervals,OP_STORED_SOURCE_GROUP_INTERVALS)
			IMPLEMENT_FP_B_GET(m_StoredSourceGroupSize,OP_STORED_SOURCE_GROUP_SIZE)
#endif // NEO_SS // NEO: NSS END

			IMPLEMENT_FP_B_GET(m_ForceA4AF,OP_FORCE_A4AF) // NEO: NXC - [NewExtendedCategories] 

			// NEO: MCS - [ManualChunkSelection]
			case OP_PR_PART_WANTED:{
                ASSERT( newtag->IsInt() );
                SetWantedPart(newtag->GetInt(), PR_PART_WANTED);
                delete newtag;
                break;
            }
			// NEO: MCS END

			default:{
				taglist.Add(newtag);
			}
		}
	}

	CheckTweaks();

	return true;
}

void CPartPreferences::Save(CIni& ini)
{
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_I_SET(m_LcIntervals,_T("LcIntervals"))

	IMPLEMENT_FP_I_SET(m_LanSourceReaskTime,_T("LanSourceReaskTime"))
	IMPLEMENT_FP_I_SET(m_LanNNPSourceReaskTime,_T("LanNNPSourceReaskTime"))
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_I_SET(m_VoodooXS,_T("VoodooXS"))
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	IMPLEMENT_FP_I_SET(m_SourceLimit,_T("SourceLimit"))

	// Management
	IMPLEMENT_FP_I_SET(m_SwapLimit,_T("SwapLimit"))

	//XS
	IMPLEMENT_FP_I_SET(m_XsEnable,_T("XsEnable"))
	IMPLEMENT_FP_I_SET(m_XsLimit,_T("XsLimit"))
	IMPLEMENT_FP_I_SET(m_XsIntervals,_T("XsIntervals"))
	IMPLEMENT_FP_I_SET(m_XsClientIntervals,_T("XsClientIntervals"))
	IMPLEMENT_FP_I_SET(m_XsCleintDelay,_T("XsCleintDelay"))
	IMPLEMENT_FP_I_SET(m_XsRareLimit,_T("XsRareLimit"))

	// SVR
	IMPLEMENT_FP_I_SET(m_SvrEnable,_T("SvrEnable"))
	IMPLEMENT_FP_I_SET(m_SvrLimit,_T("SvrLimit"))
	IMPLEMENT_FP_I_SET(m_SvrIntervals,_T("SvrIntervals"))

	//KAD
	IMPLEMENT_FP_I_SET(m_KadEnable,_T("KadEnable"))
	IMPLEMENT_FP_I_SET(m_KadLimit,_T("KadLimit"))
	IMPLEMENT_FP_I_SET(m_KadIntervals,_T("KadIntervals"))
	IMPLEMENT_FP_I_SET(m_KadMaxFiles,_T("KadMaxFiles"))
	IMPLEMENT_FP_I_SET(m_KadRepeatDelay,_T("KadRepeatDelay"))

	//UDP
	IMPLEMENT_FP_I_SET(m_UdpEnable,_T("UdpEnable"))
	IMPLEMENT_FP_I_SET(m_UdpLimit,_T("UdpLimit"))
	IMPLEMENT_FP_I_SET(m_UdpIntervals,_T("UdpIntervals"))
	IMPLEMENT_FP_I_SET(m_UdpGlobalIntervals,_T("UdpGlobalIntervals"))
	IMPLEMENT_FP_I_SET(m_UdpFilesPerServer,_T("UdpFilesPerServer"))
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	IMPLEMENT_FP_I_SET(m_UseSourceCache,_T("UseSourceCache"))
	IMPLEMENT_FP_I_SET(m_SourceCacheLimit,_T("SourceCacheLimit"))
	IMPLEMENT_FP_I_SET(m_SourceCacheTime,_T("SourceCacheTime"))
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	IMPLEMENT_FP_I_SET(m_AutoSoftLock,_T("AutoSoftLock"))
	IMPLEMENT_FP_I_SET(m_AutoSoftLockLimit,_T("AutoSoftLockLimit"))
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	IMPLEMENT_FP_I_SET(m_AutoHardLimit,_T("AutoHardLimit"))
	IMPLEMENT_FP_I_SET(m_AutoHardLimitTime,_T("AutoHardLimitTime"))
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	IMPLEMENT_FP_I_SET(m_CategorySourceLimit,_T("CategorySourceLimit"))
	IMPLEMENT_FP_I_SET(m_CategorySourceLimitLimit,_T("CategorySourceLimitLimit"))
	IMPLEMENT_FP_I_SET(m_CategorySourceLimitTime,_T("CategorySourceLimitTime"))
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	IMPLEMENT_FP_I_SET(m_GlobalSourceLimit,_T("GlobalSourceLimit"))
	IMPLEMENT_FP_I_SET(m_GlobalSourceLimitLimit,_T("GlobalSourceLimitLimit"))
	IMPLEMENT_FP_I_SET(m_GlobalSourceLimitTime,_T("GlobalSourceLimitTime"))
	// NEO: GSL END

	IMPLEMENT_FP_I_SET(m_MinSourcePerFile,_T("MinSourcePerFile"))

	IMPLEMENT_FP_I_SET(m_TCPConnectionRetry,_T("TCPConnectionRetry")) // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	IMPLEMENT_FP_I_SET(m_SpreadReaskEnable,_T("SpreadReaskEnable"))
	IMPLEMENT_FP_I_SET(m_SpreadReaskTime,_T("SpreadReaskTime"))
	IMPLEMENT_FP_I_SET(m_SourceReaskTime,_T("SourceReaskTime"))
	IMPLEMENT_FP_I_SET(m_FullQSourceReaskTime,_T("FullQSourceReaskTime"))
	IMPLEMENT_FP_I_SET(m_NNPSourceReaskTime,_T("NNPSourceReaskTime"))
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	IMPLEMENT_FP_I_SET(m_DropTime,_T("DropTime"))

	//Bad
	IMPLEMENT_FP_I_SET(m_BadSourceDrop,_T("BadSourceDrop"))
	IMPLEMENT_FP_I_SET(m_BadSourceLimitMode,_T("BadSourceLimitMode"))
	IMPLEMENT_FP_I_SET(m_BadSourceLimit,_T("BadSourceLimit"))
	IMPLEMENT_FP_I_SET(m_BadSourceDropMode,_T("BadSourceDropMode"))
	IMPLEMENT_FP_I_SET(m_BadSourceDropTime,_T("BadSourceDropTime"))

	//NNP
	IMPLEMENT_FP_I_SET(m_NNPSourceDrop,_T("NNPSourceDrop"))
	IMPLEMENT_FP_I_SET(m_NNPSourceLimitMode,_T("NNPSourceLimitMode"))
	IMPLEMENT_FP_I_SET(m_NNPSourceLimit,_T("NNPSourceLimit"))
	IMPLEMENT_FP_I_SET(m_NNPSourceDropMode,_T("NNPSourceDropMode"))
	IMPLEMENT_FP_I_SET(m_NNPSourceDropTime,_T("NNPSourceDropTime"))

	//FullQ
	IMPLEMENT_FP_I_SET(m_FullQSourceDrop,_T("FullQSourceDrop"))
	IMPLEMENT_FP_I_SET(m_FullQSourceLimitMode,_T("FullQSourceLimitMode"))
	IMPLEMENT_FP_I_SET(m_FullQSourceLimit,_T("FullQSourceLimit"))
	IMPLEMENT_FP_I_SET(m_FullQSourceDropMode,_T("FullQSourceDropMode"))
	IMPLEMENT_FP_I_SET(m_FullQSourceDropTime,_T("FullQSourceDropTime"))

	//HighQ
	IMPLEMENT_FP_I_SET(m_HighQSourceDrop,_T("HighQSourceDrop"))
	IMPLEMENT_FP_I_SET(m_HighQSourceLimitMode,_T("HighQSourceLimitMode"))
	IMPLEMENT_FP_I_SET(m_HighQSourceLimit,_T("HighQSourceLimit"))
	IMPLEMENT_FP_I_SET(m_HighQSourceDropMode,_T("HighQSourceDropMode"))
	IMPLEMENT_FP_I_SET(m_HighQSourceDropTime,_T("HighQSourceDropTime"))
	IMPLEMENT_FP_I_SET(m_HighQSourceRankMode,_T("HighQSourceRankMode"))
	IMPLEMENT_FP_I_SET(m_HighQSourceMaxRank,_T("HighQSourceMaxRank"))

	IMPLEMENT_FP_I_SET(m_DeadTime,_T("DeadTime"))
	IMPLEMENT_FP_I_SET(m_DeadTimeFWMulti,_T("DeadTimeFWMulti"))
	IMPLEMENT_FP_I_SET(m_GlobalDeadTime,_T("GlobalDeadTime"))
	IMPLEMENT_FP_I_SET(m_GlobalDeadTimeFWMulti,_T("GlobalDeadTimeFWMulti"))
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	IMPLEMENT_FP_I_SET(m_AutoSaveSources,_T("AutoSaveSources"))
	IMPLEMENT_FP_I_SET(m_AutoSaveSourcesIntervals,_T("AutoSaveSourcesIntervals"))
	IMPLEMENT_FP_I_SET(m_SourceStorageLimit,_T("SourceStorageLimit"))

	IMPLEMENT_FP_I_SET(m_StoreAlsoA4AFSources,_T("StoreAlsoA4AFSources"))

	IMPLEMENT_FP_I_SET(m_AutoLoadSources,_T("AutoLoadSources"))
	IMPLEMENT_FP_I_SET(m_LoadedSourceCleanUpTime,_T("LoadedSourceCleanUpTime"))
	IMPLEMENT_FP_I_SET(m_SourceStorageReaskLimit,_T("SourceStorageReaskLimit"))

	IMPLEMENT_FP_I_SET(m_TotalSourceRestore,_T("TotalSourceRestore"))

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	IMPLEMENT_FP_I_SET(m_ReaskPropability,_T("ReaskPropability"))
	IMPLEMENT_FP_I_SET(m_UnpredictedPropability,_T("UnpredictedPropability"))
	IMPLEMENT_FP_I_SET(m_UnpredictedReaskPropability,_T("UnpredictedReaskPropability"))
 #endif // NEO_SA // NEO: NSA END

	IMPLEMENT_FP_I_SET(m_AutoReaskStoredSourcesDelay,_T("AutoReaskStoredSourcesDelay"))

	IMPLEMENT_FP_I_SET(m_GroupStoredSourceReask,_T("GroupStoredSourceReask"))
	IMPLEMENT_FP_I_SET(m_StoredSourceGroupIntervals,_T("StoredSourceGroupIntervals"))
	IMPLEMENT_FP_I_SET(m_StoredSourceGroupSize,_T("StoredSourceGroupSize"))
#endif // NEO_SS // NEO: NSS END

	IMPLEMENT_FP_I_SET(m_ForceA4AF,_T("ForceA4AF")) // NEO: NXC - [NewExtendedCategories] 

}

bool CPartPreferences::Load(CIni& ini)
{
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_I_GET(m_LcIntervals,_T("LcIntervals"),DEF_LC_INTERVALS)

	IMPLEMENT_FP_I_GET(m_LanSourceReaskTime,_T("LanSourceReaskTime"),DEF_LAN_SOURCE_REASK_TIME)
	IMPLEMENT_FP_I_GET(m_LanNNPSourceReaskTime,_T("LanNNPSourceReaskTime"),DEF_LAN_NNP_SOURCE_REASK_TIME)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_I_GET(m_VoodooXS,_T("VoodooXS"),TRUE)
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	IMPLEMENT_FP_I_GET(m_SourceLimit,_T("SourceLimit"),DEF_SOURCE_LIMIT)

	// Management
	IMPLEMENT_FP_I_GET(m_SwapLimit,_T("SwapLimit"),DEF_SWAP_LIMIT)

	//XS
	IMPLEMENT_FP_I_GET(m_XsEnable,_T("XsEnable"),TRUE )
	IMPLEMENT_FP_I_GET(m_XsLimit,_T("XsLimit"),DEF_XS_LIMIT)
	IMPLEMENT_FP_I_GET(m_XsIntervals,_T("XsIntervals"),DEF_XS_INTERVALS)
	IMPLEMENT_FP_I_GET(m_XsClientIntervals,_T("XsClientIntervals"),DEF_XS_CLIENT_INTERVALS)
	IMPLEMENT_FP_I_GET(m_XsCleintDelay,_T("XsCleintDelay"),DEF_XS_CLEINT_DELAY)
	IMPLEMENT_FP_I_GET(m_XsRareLimit,_T("XsRareLimit"),DEF_XS_RARE_LIMIT)

	// SVR
	IMPLEMENT_FP_I_GET(m_SvrEnable,_T("SvrEnable"),TRUE)
	IMPLEMENT_FP_I_GET(m_SvrLimit,_T("SvrLimit"),DEF_SVR_LIMIT)
	IMPLEMENT_FP_I_GET(m_SvrIntervals,_T("SvrIntervals"),DEF_SVR_INTERVALS)

	//KAD
	IMPLEMENT_FP_I_GET(m_KadEnable,_T("KadEnable"),TRUE)
	IMPLEMENT_FP_I_GET(m_KadLimit,_T("KadLimit"),DEF_KAD_LIMIT)
	IMPLEMENT_FP_I_GET(m_KadIntervals,_T("KadIntervals"),DEF_KAD_INTERVALS)
	IMPLEMENT_FP_I_GET(m_KadMaxFiles,_T("KadMaxFiles"),DEF_KAD_MAX_FILES)
	IMPLEMENT_FP_I_GET(m_KadRepeatDelay,_T("KadRepeatDelay"),DEF_KAD_REPEAT_DELAY)

	//UDP
	IMPLEMENT_FP_I_GET(m_UdpEnable,_T("UdpEnable"),TRUE)
	IMPLEMENT_FP_I_GET(m_UdpLimit,_T("UdpLimit"),DEF_UDP_LIMIT)
	IMPLEMENT_FP_I_GET(m_UdpIntervals,_T("UdpIntervals"),DEF_UDP_INTERVALS)
	IMPLEMENT_FP_I_GET(m_UdpGlobalIntervals,_T("UdpGlobalIntervals"),DEF_UDP_GLOBAL_INTERVALS)
	IMPLEMENT_FP_I_GET(m_UdpFilesPerServer,_T("UdpFilesPerServer"),DEF_UDP_FILES_PER_SERVER)
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	IMPLEMENT_FP_I_GET(m_UseSourceCache,_T("UseSourceCache"),TRUE)
	IMPLEMENT_FP_I_GET(m_SourceCacheLimit,_T("SourceCacheLimit"),DEF_SOURCE_CACHE_LIMIT)
	IMPLEMENT_FP_I_GET(m_SourceCacheTime,_T("SourceCacheTime"),DEF_SOURCE_CACHE_TIME)
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	IMPLEMENT_FP_I_GET(m_AutoSoftLock,_T("AutoSoftLock"), FALSE)
	IMPLEMENT_FP_I_GET(m_AutoSoftLockLimit,_T("AutoSoftLockLimit"), DEF_AUTO_SOFT_LOCK_LIMIT)
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	IMPLEMENT_FP_I_GET(m_AutoHardLimit,_T("AutoHardLimit"), FALSE)
	IMPLEMENT_FP_I_GET(m_AutoHardLimitTime,_T("AutoHardLimitTime"), DEF_AUTO_HARD_LIMIT_TIME)
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	IMPLEMENT_FP_I_GET(m_CategorySourceLimit,_T("CategorySourceLimit"), FALSE)
	IMPLEMENT_FP_I_GET(m_CategorySourceLimitLimit,_T("CategorySourceLimitLimit"), DEF_CATEGORY_SOURCE_LIMIT_LIMIT)
	IMPLEMENT_FP_I_GET(m_CategorySourceLimitTime,_T("CategorySourceLimitTime"), DEF_CATEGORY_SOURCE_LIMIT_TIME)
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	IMPLEMENT_FP_I_GET(m_GlobalSourceLimit,_T("GlobalSourceLimit"), FALSE)
	IMPLEMENT_FP_I_GET(m_GlobalSourceLimitLimit,_T("GlobalSourceLimitLimit"), DEF_GLOBAL_SOURCE_LIMIT_LIMIT)
	IMPLEMENT_FP_I_GET(m_GlobalSourceLimitTime,_T("GlobalSourceLimitTime"), DEF_GLOBAL_SOURCE_LIMIT_TIME)
	// NEO: GSL END

	IMPLEMENT_FP_I_GET(m_MinSourcePerFile,_T("MinSourcePerFile"), DEF_MIN_SOURCE_PER_FILE)

	IMPLEMENT_FP_I_GET(m_TCPConnectionRetry,_T("TCPConnectionRetry"),DEF_TCP_CONNECTION_RETRY) // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	IMPLEMENT_FP_I_GET(m_SpreadReaskEnable,_T("SpreadReaskEnable"),TRUE)
	IMPLEMENT_FP_I_GET(m_SpreadReaskTime,_T("SpreadReaskTime"),DEF_SPREAD_REASK_TIME)
	IMPLEMENT_FP_I_GET(m_SourceReaskTime,_T("SourceReaskTime"),DEF_SOURCE_REASK_TIME)
	IMPLEMENT_FP_I_GET(m_FullQSourceReaskTime,_T("FullQSourceReaskTime"),DEF_FULLQ_SOURCE_REASK_TIME)
	IMPLEMENT_FP_I_GET(m_NNPSourceReaskTime,_T("NNPSourceReaskTime"),DEF_NNP_SOURCE_REASK_TIME)
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	IMPLEMENT_FP_I_GET(m_DropTime,_T("DropTime"),DEF_DROP_TIME)

	//Bad
	IMPLEMENT_FP_I_GET(m_BadSourceDrop,_T("BadSourceDrop"),TRUE)
	IMPLEMENT_FP_I_GET(m_BadSourceLimitMode,_T("BadSourceLimitMode"),SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET3(m_BadSourceLimit,_T("BadSourceLimit"),BAD_SOURCE_LIMIT,m_BadSourceLimitMode,SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET(m_BadSourceDropMode,_T("BadSourceDropMode"),SDT_TIME_MODE_PROGRESSIV)
	IMPLEMENT_FP_I_GET3(m_BadSourceDropTime,_T("BadSourceDropTime"),BAD_SOURCE_DROP_TIME,m_BadSourceDropMode,SDT_TIME_MODE_PROGRESSIV)

	//NNP
	IMPLEMENT_FP_I_GET(m_NNPSourceDrop,_T("NNPSourceDrop"),TRUE)
	IMPLEMENT_FP_I_GET(m_NNPSourceLimitMode,_T("NNPSourceLimitMode"),SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET3(m_NNPSourceLimit,_T("NNPSourceLimit"),NNP_SOURCE_LIMIT,m_NNPSourceLimitMode,SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET(m_NNPSourceDropMode,_T("NNPSourceDropMode"),SDT_TIME_MODE_PROGRESSIV)
	IMPLEMENT_FP_I_GET3(m_NNPSourceDropTime,_T("NNPSourceDropTime"),NNP_SOURCE_DROP_TIME,m_NNPSourceDropMode,SDT_TIME_MODE_PROGRESSIV)

	//FullQ
	IMPLEMENT_FP_I_GET(m_FullQSourceDrop,_T("FullQSourceDrop"),TRUE)
	IMPLEMENT_FP_I_GET(m_FullQSourceLimitMode,_T("FullQSourceLimitMode"),SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET3(m_FullQSourceLimit,_T("FullQSourceLimit"),FULLQ_SOURCE_LIMIT,m_FullQSourceLimitMode,SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET(m_FullQSourceDropMode,_T("FullQSourceDropMode"),SDT_TIME_MODE_PROGRESSIV)
	IMPLEMENT_FP_I_GET3(m_FullQSourceDropTime,_T("FullQSourceDropTime"),FULLQ_SOURCE_DROP_TIME,m_FullQSourceDropMode,SDT_TIME_MODE_PROGRESSIV)

	//HighQ
	IMPLEMENT_FP_I_GET(m_HighQSourceDrop,_T("HighQSourceDrop"),FALSE)
	IMPLEMENT_FP_I_GET(m_HighQSourceLimitMode,_T("HighQSourceLimitMode"),SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET3(m_HighQSourceLimit,_T("HighQSourceLimit"),HIGHQ_SOURCE_LIMIT,m_HighQSourceLimitMode,SDT_LIMIT_MODE_TOTAL)
	IMPLEMENT_FP_I_GET(m_HighQSourceDropMode,_T("HighQSourceDropMode"),SDT_TIME_MODE_PROGRESSIV)
	IMPLEMENT_FP_I_GET3(m_HighQSourceDropTime,_T("HighQSourceDropTime"),HIGHQ_SOURCE_DROP_TIME,m_HighQSourceDropMode,SDT_TIME_MODE_PROGRESSIV)
	IMPLEMENT_FP_I_GET(m_HighQSourceRankMode,_T("HighQSourceRankMode"),SDT_HIGHQ_MODE_NORMAL)
	IMPLEMENT_FP_I_GET2(m_HighQSourceMaxRank,_T("HighQSourceMaxRank"),HIGHQ_SOURCE_MAX_RANK,m_HighQSourceRankMode,SDT_HIGHQ_MODE_NORMAL)

	IMPLEMENT_FP_I_GET(m_DeadTime,_T("DeadTime"),DEF_DEAD_TIME)
	IMPLEMENT_FP_I_GET(m_DeadTimeFWMulti,_T("DeadTimeFWMulti"),DEF_DEAD_TIME_FW_MILTU)
	IMPLEMENT_FP_I_GET(m_GlobalDeadTime,_T("GlobalDeadTime"),DEF_GLOBAL_DEAD_TIME)
	IMPLEMENT_FP_I_GET(m_GlobalDeadTimeFWMulti,_T("GlobalDeadTimeFWMulti"),DEF_GLOBAL_DEAD_TIME_FW_MILTU)
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	IMPLEMENT_FP_I_GET(m_AutoSaveSources,_T("AutoSaveSources"),TRUE)
	IMPLEMENT_FP_I_GET(m_AutoSaveSourcesIntervals,_T("AutoSaveSourcesIntervals"),DEF_AUTO_SAVE_SOURCES_INTERVALS)
	IMPLEMENT_FP_I_GET(m_SourceStorageLimit,_T("SourceStorageLimit"),DEF_SOURCE_STORAGE_LIMIT)

	IMPLEMENT_FP_I_GET(m_StoreAlsoA4AFSources,_T("StoreAlsoA4AFSources"),TRUE)

	IMPLEMENT_FP_I_GET(m_AutoLoadSources,_T("AutoLoadSources"),TRUE)
	IMPLEMENT_FP_I_GET(m_LoadedSourceCleanUpTime,_T("LoadedSourceCleanUpTime"),DEF_LOADED_SOURCE_CLEAN_UP_TIME)
	IMPLEMENT_FP_I_GET(m_SourceStorageReaskLimit,_T("SourceStorageReaskLimit"),DEF_SOURCE_STORAGE_REASK_LIMIT)

	IMPLEMENT_FP_I_GET(m_TotalSourceRestore,_T("TotalSourceRestore"),2)

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	IMPLEMENT_FP_I_GET(m_ReaskPropability,_T("ReaskPropability"),DEF_REASK_PROPABILITY)
	IMPLEMENT_FP_I_GET(m_UnpredictedPropability,_T("UnpredictedPropability"),TRUE)
	IMPLEMENT_FP_I_GET(m_UnpredictedReaskPropability,_T("UnpredictedReaskPropability"),DEF_UNPREDICTED_REASK_PROPABILITY)
 #endif // NEO_SA // NEO: NSA END

	IMPLEMENT_FP_I_GET(m_AutoReaskStoredSourcesDelay,_T("AutoReaskStoredSourcesDelay"),DEF_AUTO_REASK_STORED_SOURCES_DELAY)

	IMPLEMENT_FP_I_GET(m_GroupStoredSourceReask,_T("GroupStoredSourceReask"),TRUE)
	IMPLEMENT_FP_I_GET(m_StoredSourceGroupIntervals,_T("StoredSourceGroupIntervals"),DEF_STORED_SOURCE_GROUP_INTERVALS)
	IMPLEMENT_FP_I_GET(m_StoredSourceGroupSize,_T("StoredSourceGroupSize"),DEF_STORED_SOURCE_GROUP_SIZE)
#endif // NEO_SS // NEO: NSS END

	IMPLEMENT_FP_I_GET(m_ForceA4AF,_T("ForceA4AF"),0) // NEO: NXC - [NewExtendedCategories] 

	CheckTweaks();

	return true;
}

void CPartPreferences::CheckTweaks(){
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	IMPLEMENT_FP_CHK_VAL(m_LcIntervals,LC_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_LanSourceReaskTime,LAN_SOURCE_REASK_TIME)
	IMPLEMENT_FP_CHK_VAL(m_LanNNPSourceReaskTime,LAN_NNP_SOURCE_REASK_TIME)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	IMPLEMENT_FP_CHK_FLAG(m_VoodooXS,TRUE,2)
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	IMPLEMENT_FP_CHK_VAL(m_SourceLimit,SOURCE_LIMIT)

	// Management
	IMPLEMENT_FP_CHK_VAL(m_SwapLimit,SWAP_LIMIT)

	//XS
	IMPLEMENT_FP_CHK_FLAG(m_XsEnable,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_XsLimit,XS_LIMIT)
	IMPLEMENT_FP_CHK_VAL(m_XsIntervals,XS_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_XsClientIntervals,XS_CLIENT_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_XsCleintDelay,XS_CLEINT_DELAY)
	IMPLEMENT_FP_CHK_VAL(m_XsRareLimit,XS_RARE_LIMIT)

	// SVR
	IMPLEMENT_FP_CHK_FLAG(m_SvrEnable,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_SvrLimit,SVR_LIMIT)
	IMPLEMENT_FP_CHK_VAL(m_SvrIntervals,SVR_INTERVALS)

	//KAD
	IMPLEMENT_FP_CHK_FLAG(m_KadEnable,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_KadLimit,KAD_LIMIT)
	IMPLEMENT_FP_CHK_VAL(m_KadIntervals,KAD_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_KadMaxFiles,KAD_MAX_FILES)
	IMPLEMENT_FP_CHK_VAL(m_KadRepeatDelay,KAD_REPEAT_DELAY)

	//UDP
	IMPLEMENT_FP_CHK_FLAG(m_UdpEnable,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_UdpLimit,UDP_LIMIT)
	IMPLEMENT_FP_CHK_VAL(m_UdpIntervals,UDP_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_UdpGlobalIntervals,UDP_GLOBAL_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_UdpFilesPerServer,UDP_FILES_PER_SERVER)
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	IMPLEMENT_FP_CHK_FLAG(m_UseSourceCache,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_SourceCacheLimit,SOURCE_CACHE_LIMIT)
	IMPLEMENT_FP_CHK_VAL(m_SourceCacheTime,SOURCE_CACHE_TIME)
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	IMPLEMENT_FP_CHK_FLAG(m_AutoSoftLock,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_AutoSoftLockLimit,AUTO_SOFT_LOCK_LIMIT)
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	IMPLEMENT_FP_CHK_FLAG(m_AutoHardLimit,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_AutoHardLimitTime,AUTO_HARD_LIMIT_TIME)
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	IMPLEMENT_FP_CHK_FLAG(m_CategorySourceLimit,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_CategorySourceLimitLimit,CATEGORY_SOURCE_LIMIT_LIMIT)
	IMPLEMENT_FP_CHK_VAL(m_CategorySourceLimitTime,CATEGORY_SOURCE_LIMIT_TIME)
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	IMPLEMENT_FP_CHK_FLAG(m_GlobalSourceLimit,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_GlobalSourceLimitLimit,GLOBAL_SOURCE_LIMIT_LIMIT)
	IMPLEMENT_FP_CHK_VAL(m_GlobalSourceLimitTime,GLOBAL_SOURCE_LIMIT_TIME)
	// NEO: GSL END

	IMPLEMENT_FP_CHK_VAL(m_MinSourcePerFile,MIN_SOURCE_PER_FILE)

	IMPLEMENT_FP_CHK_VAL(m_TCPConnectionRetry,TCP_CONNECTION_RETRY) // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	IMPLEMENT_FP_CHK_FLAG(m_SpreadReaskEnable,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_SpreadReaskTime,SPREAD_REASK_TIME)
	IMPLEMENT_FP_CHK_VAL(m_SourceReaskTime,SOURCE_REASK_TIME)
	IMPLEMENT_FP_CHK_VAL(m_FullQSourceReaskTime,FULLQ_SOURCE_REASK_TIME)
	IMPLEMENT_FP_CHK_VAL(m_NNPSourceReaskTime,NNP_SOURCE_REASK_TIME)
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	IMPLEMENT_FP_CHK_VAL(m_DropTime,DROP_TIME)

	//Bad
	IMPLEMENT_FP_CHK_FLAG(m_BadSourceDrop,TRUE,1)
	IMPLEMENT_FP_CHK_FLAG(m_BadSourceLimitMode,SDT_LIMIT_MODE_TOTAL,2)
	IMPLEMENT_FP_CHK_VAL3(m_BadSourceLimit,BAD_SOURCE_LIMIT,m_BadSourceLimitMode)
	IMPLEMENT_FP_CHK_FLAG(m_BadSourceDropMode,SDT_TIME_MODE_PROGRESSIV,2)
	IMPLEMENT_FP_CHK_VAL3(m_BadSourceDropTime,BAD_SOURCE_DROP_TIME,m_BadSourceDropMode)

	//NNP
	IMPLEMENT_FP_CHK_FLAG(m_NNPSourceDrop,TRUE,2)
	IMPLEMENT_FP_CHK_FLAG(m_NNPSourceLimitMode,SDT_LIMIT_MODE_TOTAL,2)
	IMPLEMENT_FP_CHK_VAL3(m_NNPSourceLimit,NNP_SOURCE_LIMIT,m_NNPSourceLimitMode)
	IMPLEMENT_FP_CHK_FLAG(m_NNPSourceDropMode,SDT_TIME_MODE_PROGRESSIV,2)
	IMPLEMENT_FP_CHK_VAL3(m_NNPSourceDropTime,NNP_SOURCE_DROP_TIME,m_NNPSourceDropMode)

	//FullQ
	IMPLEMENT_FP_CHK_FLAG(m_FullQSourceDrop,TRUE,2)
	IMPLEMENT_FP_CHK_FLAG(m_FullQSourceLimitMode,SDT_LIMIT_MODE_TOTAL,2)
	IMPLEMENT_FP_CHK_VAL3(m_FullQSourceLimit,FULLQ_SOURCE_LIMIT,m_FullQSourceLimitMode)
	IMPLEMENT_FP_CHK_FLAG(m_FullQSourceDropMode,SDT_TIME_MODE_PROGRESSIV,2)
	IMPLEMENT_FP_CHK_VAL3(m_FullQSourceDropTime,FULLQ_SOURCE_DROP_TIME,m_FullQSourceDropMode)

	//HighQ
	IMPLEMENT_FP_CHK_FLAG(m_HighQSourceDrop,FALSE,2)
	IMPLEMENT_FP_CHK_FLAG(m_HighQSourceLimitMode,SDT_LIMIT_MODE_TOTAL,2)
	IMPLEMENT_FP_CHK_VAL3(m_HighQSourceLimit,HIGHQ_SOURCE_LIMIT,m_HighQSourceLimitMode)
	IMPLEMENT_FP_CHK_FLAG(m_HighQSourceDropMode,SDT_TIME_MODE_PROGRESSIV,2)
	IMPLEMENT_FP_CHK_VAL3(m_HighQSourceDropTime,HIGHQ_SOURCE_DROP_TIME,m_HighQSourceDropMode)
	IMPLEMENT_FP_CHK_FLAG(m_HighQSourceRankMode,SDT_HIGHQ_MODE_NORMAL,1)
	IMPLEMENT_FP_CHK_VAL2(m_HighQSourceMaxRank,HIGHQ_SOURCE_MAX_RANK,m_HighQSourceRankMode)

	IMPLEMENT_FP_CHK_VAL(m_DeadTime,DEAD_TIME)
	IMPLEMENT_FP_CHK_VAL(m_DeadTimeFWMulti,DEAD_TIME_FW_MILTU)
	IMPLEMENT_FP_CHK_VAL(m_GlobalDeadTime,GLOBAL_DEAD_TIME)
	IMPLEMENT_FP_CHK_VAL(m_GlobalDeadTimeFWMulti,GLOBAL_DEAD_TIME_FW_MILTU)
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	IMPLEMENT_FP_CHK_FLAG(m_AutoSaveSources,TRUE,2)
	IMPLEMENT_FP_CHK_VAL(m_AutoSaveSourcesIntervals,AUTO_SAVE_SOURCES_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_SourceStorageLimit,SOURCE_STORAGE_LIMIT)

	IMPLEMENT_FP_CHK_FLAG(m_StoreAlsoA4AFSources,TRUE,1)

	IMPLEMENT_FP_CHK_FLAG(m_AutoLoadSources,TRUE,2)
	IMPLEMENT_FP_CHK_VAL(m_LoadedSourceCleanUpTime,LOADED_SOURCE_CLEAN_UP_TIME)
	IMPLEMENT_FP_CHK_VAL(m_SourceStorageReaskLimit,SOURCE_STORAGE_REASK_LIMIT)

	IMPLEMENT_FP_CHK_FLAG(m_TotalSourceRestore,2,2)	

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	IMPLEMENT_FP_CHK_VAL(m_ReaskPropability,REASK_PROPABILITY)
	IMPLEMENT_FP_CHK_FLAG(m_UnpredictedPropability,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_UnpredictedReaskPropability,UNPREDICTED_REASK_PROPABILITY)
 #endif // NEO_SA // NEO: NSA END

	IMPLEMENT_FP_CHK_VAL(m_AutoReaskStoredSourcesDelay,AUTO_REASK_STORED_SOURCES_DELAY)

	IMPLEMENT_FP_CHK_FLAG(m_GroupStoredSourceReask,TRUE,1)
	IMPLEMENT_FP_CHK_VAL(m_StoredSourceGroupIntervals,STORED_SOURCE_GROUP_INTERVALS)
	IMPLEMENT_FP_CHK_VAL(m_StoredSourceGroupSize,STORED_SOURCE_GROUP_SIZE)
#endif // NEO_SS // NEO: NSS END

	IMPLEMENT_FP_CHK_FLAG(m_ForceA4AF,0,2) // NEO: NXC - [NewExtendedCategories] 
}

///////////////////////////////////////////////////////////////////////////////////////
// CKnownPreferencesEx 
//

CKnownPreferencesEx::CKnownPreferencesEx(EFilePrefsLevel Level){
	KnownFile = NULL;
	KnownPrefs = NULL;
	m_Level = Level;

	ResetTweaks();
}

void CKnownPreferencesEx::ResetTweaks(){
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_EnableLanCast = FCFG_DEF;
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_EnableVoodoo = FCFG_DEF;
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	m_UseInteligentPartSharing = FCFG_DEF;
	m_InteligentPartSharingTimer = FCFG_DEF;

	m_MaxProzentToHide = FCFG_DEF;

	// OverAvalibly
	m_HideOverAvaliblyParts = FCFG_DEF;
	m_HideOverAvaliblyMode = FCFG_DEF;
	m_HideOverAvaliblyValue = FCFG_DEF;

	m_BlockHighOverAvaliblyParts = FCFG_DEF;
	m_BlockHighOverAvaliblyFactor = FCFG_DEF;

	// OverShared
	m_HideOverSharedParts = FCFG_DEF;
	m_HideOverSharedMode = FCFG_DEF;
	m_HideOverSharedValue = FCFG_DEF;
	m_HideOverSharedCalc = FCFG_DEF;

	m_BlockHighOverSharedParts = FCFG_DEF;
	m_BlockHighOverSharedFactor = FCFG_DEF;

	// DontHideUnderAvalibly
	m_DontHideUnderAvaliblyParts = FCFG_DEF;
	m_DontHideUnderAvaliblyMode = FCFG_DEF;
	m_DontHideUnderAvaliblyValue = FCFG_DEF;

	// Other
	m_ShowAlwaysSomeParts = FCFG_DEF;
	m_ShowAlwaysSomePartsValue = FCFG_DEF;

	m_ShowAlwaysIncompleteParts = FCFG_DEF;
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	m_ReleaseMode = FCFG_DEF;
	m_ReleaseLevel = FCFG_DEF;
	m_ReleaseTimer = FCFG_DEF;

	// release limit
	m_ReleaseLimit = FCFG_DEF;
	m_ReleaseLimitMode = FCFG_DEF;
	m_ReleaseLimitHigh = FCFG_DEF;
	m_ReleaseLimitLow = FCFG_DEF;

	m_ReleaseLimitLink = FCFG_DEF;

	m_ReleaseLimitComplete = FCFG_DEF;
	m_ReleaseLimitCompleteMode = FCFG_DEF;
	m_ReleaseLimitCompleteHigh = FCFG_DEF;
	m_ReleaseLimitCompleteLow = FCFG_DEF;

	// limit
	m_LimitLink = FCFG_DEF;

	// source limit
	m_SourceLimit = FCFG_DEF;
	m_SourceLimitMode = FCFG_DEF;
	m_SourceLimitHigh = FCFG_DEF;
	m_SourceLimitLow = FCFG_DEF;

	m_SourceLimitLink = FCFG_DEF;

	m_SourceLimitComplete = FCFG_DEF;
	m_SourceLimitCompleteMode = FCFG_DEF;
	m_SourceLimitCompleteHigh = FCFG_DEF;
	m_SourceLimitCompleteLow = FCFG_DEF;
	// NEO: SRS END

	m_ManagedParts.RemoveAll(); // NEO: MCS - [ManualChunkSelection]
}

bool CKnownPreferencesEx::IsEmpty() const {
	return (
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_EnableLanCast == FCFG_DEF &&
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_EnableVoodoo == FCFG_DEF &&
#endif // VOODOO // NEO: VOODOO END

	// NEO: IPS - [InteligentPartSharing]
	m_UseInteligentPartSharing == FCFG_DEF &&
	m_InteligentPartSharingTimer == FCFG_DEF &&

	m_MaxProzentToHide == FCFG_DEF &&

	// OverAvalibly
	m_HideOverAvaliblyParts == FCFG_DEF &&
	m_HideOverAvaliblyMode == FCFG_DEF &&
	m_HideOverAvaliblyValue == FCFG_DEF &&

	m_BlockHighOverAvaliblyParts == FCFG_DEF &&
	m_BlockHighOverAvaliblyFactor == FCFG_DEF &&

	// OverShared
	m_HideOverSharedParts == FCFG_DEF &&
	m_HideOverSharedMode == FCFG_DEF &&
	m_HideOverSharedValue == FCFG_DEF &&
	m_HideOverSharedCalc == FCFG_DEF &&

	m_BlockHighOverSharedParts == FCFG_DEF &&
	m_BlockHighOverSharedFactor == FCFG_DEF &&

	// DontHideUnderAvalibly
	m_DontHideUnderAvaliblyParts == FCFG_DEF &&
	m_DontHideUnderAvaliblyMode == FCFG_DEF &&
	m_DontHideUnderAvaliblyValue == FCFG_DEF &&

	// Other
	m_ShowAlwaysSomeParts == FCFG_DEF &&
	m_ShowAlwaysSomePartsValue == FCFG_DEF &&

	m_ShowAlwaysIncompleteParts == FCFG_DEF &&
	// NEO: IPS END

	// NEO: SRS - [SmartReleaseSharing]
	m_ReleaseMode == FCFG_DEF &&
	m_ReleaseLevel == FCFG_DEF &&
	m_ReleaseTimer == FCFG_DEF &&

	// release limit
	m_ReleaseLimit == FCFG_DEF &&
	m_ReleaseLimitMode == FCFG_DEF &&
	m_ReleaseLimitHigh == FCFG_DEF &&
	m_ReleaseLimitLow == FCFG_DEF &&

	m_ReleaseLimitLink == FCFG_DEF &&

	m_ReleaseLimitComplete == FCFG_DEF &&
	m_ReleaseLimitCompleteMode == FCFG_DEF &&
	m_ReleaseLimitCompleteHigh == FCFG_DEF &&
	m_ReleaseLimitCompleteLow == FCFG_DEF &&

	// limit
	m_LimitLink == FCFG_DEF &&

	// source limit
	m_SourceLimit == FCFG_DEF &&
	m_SourceLimitMode == FCFG_DEF &&
	m_SourceLimitHigh == FCFG_DEF &&
	m_SourceLimitLow == FCFG_DEF &&

	m_SourceLimitLink == FCFG_DEF &&

	m_SourceLimitComplete == FCFG_DEF &&
	m_SourceLimitCompleteMode == FCFG_DEF &&
	m_SourceLimitCompleteHigh == FCFG_DEF &&
	m_SourceLimitCompleteLow == FCFG_DEF &&
	// NEO: SRS END

	m_ManagedParts.IsEmpty() // NEO: MCS - [ManualChunkSelection]
		);
}
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_EnableLanCast,IsEnableLanCast,KnownPrefs)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_EnableVoodoo,IsEnableVoodoo,KnownPrefs)
#endif // VOODOO // NEO: VOODOO END

// NEO: IPS - [InteligentPartSharing]
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_UseInteligentPartSharing,UseInteligentPartSharing,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_InteligentPartSharingTimer,GetInteligentPartSharingTimer,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_MaxProzentToHide,GetMaxProzentToHide,KnownPrefs)

// OverAvalibly
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_HideOverAvaliblyParts,IsHideOverAvaliblyParts,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_HideOverAvaliblyMode,GetHideOverAvaliblyMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_HideOverAvaliblyValue,GetHideOverAvaliblyValue,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_BlockHighOverAvaliblyParts,IsBlockHighOverAvaliblyParts,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_BlockHighOverAvaliblyFactor,GetBlockHighOverAvaliblyFactor,KnownPrefs)

// OverShared
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_HideOverSharedParts,IsHideOverSharedParts,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_HideOverSharedMode,GetHideOverSharedMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_HideOverSharedValue,GetHideOverSharedValue,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_HideOverSharedCalc,GetHideOverSharedCalc,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_BlockHighOverSharedParts,IsBlockHighOverSharedParts,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_BlockHighOverSharedFactor,GetBlockHighOverSharedFactor,KnownPrefs)

// DontHideUnderAvalibly
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_DontHideUnderAvaliblyParts,IsDontHideUnderAvaliblyParts,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_DontHideUnderAvaliblyMode,GetDontHideUnderAvaliblyMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_DontHideUnderAvaliblyValue,GetDontHideUnderAvaliblyValue,KnownPrefs)

// Other
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ShowAlwaysSomeParts,IsShowAlwaysSomeParts,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ShowAlwaysSomePartsValue,GetShowAlwaysSomePartsValue,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ShowAlwaysIncompleteParts,IsShowAlwaysIncompleteParts,KnownPrefs)
// NEO: IPS END

// NEO: SRS - [SmartReleaseSharing]
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseMode,GetReleaseMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLevel,GetReleaseLevel,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseTimer,GetReleaseTimer,KnownPrefs)

// release limit
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimit,IsReleaseLimit,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitMode,GetReleaseLimitMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitHigh,GetReleaseLimitHigh,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitLow,GetReleaseLimitLow,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitLink,IsReleaseLimitLink,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitComplete,IsReleaseLimitComplete,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitCompleteMode,GetReleaseLimitCompleteMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitCompleteHigh,GetReleaseLimitCompleteHigh,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_ReleaseLimitCompleteLow,GetReleaseLimitCompleteLow,KnownPrefs)

// limit
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_LimitLink,IsLimitLink,KnownPrefs)

// source limit
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimit,IsSourceLimit,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitMode,GetSourceLimitMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitHigh,GetSourceLimitHigh,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitLow,GetSourceLimitLow,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitLink,IsSourceLimitLink,KnownPrefs)

IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitComplete,IsSourceLimitComplete,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitCompleteMode,GetSourceLimitCompleteMode,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitCompleteHigh,GetSourceLimitCompleteHigh,KnownPrefs)
IMPLEMENT_CFPEX(CKnownPreferencesEx,m_SourceLimitCompleteLow,GetSourceLimitCompleteLow,KnownPrefs)
// NEO: SRS END

// NEO: MPS - [ManualPartSharing]
void CKnownPreferencesEx::SetManagedPart(UINT part, uint8 status)
{
	if(status != PR_PART_NORMAL)
		m_ManagedParts.SetAt(part,status);
	else
		m_ManagedParts.RemoveKey(part);
}

uint8 CKnownPreferencesEx::GetManagedPart(UINT part) const 
{ 
	uint8 status = PR_PART_NORMAL; 
	if(m_ManagedParts.Lookup(part, status)) 
		return status; 
	return PR_PART_NORMAL; 
}
// NEO: MPS END

///////////////////////////////////////////////////////////////////////////////////////
// CPartPreferencesEx 
//

CPartPreferencesEx::CPartPreferencesEx(EFilePrefsLevel Level){
	PartFile = NULL;
	PartPrefs = NULL;
	m_Level = Level;

	ResetTweaks();
}

void CPartPreferencesEx::ResetTweaks()
{
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_LcIntervals = FCFG_DEF;

	m_LanSourceReaskTime = FCFG_DEF;
	m_LanNNPSourceReaskTime = FCFG_DEF;
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_VoodooXS = FCFG_DEF;
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	m_SourceLimit = FCFG_DEF;

	// Management
	m_SwapLimit = FCFG_DEF;

	//XS
	m_XsEnable = FCFG_DEF;
	m_XsLimit = FCFG_DEF;
	m_XsIntervals = FCFG_DEF;
	m_XsClientIntervals = FCFG_DEF;
	m_XsCleintDelay = FCFG_DEF;
	m_XsRareLimit = FCFG_DEF;

	// SVR
	m_SvrEnable = FCFG_DEF;
	m_SvrLimit = FCFG_DEF;
	m_SvrIntervals = FCFG_DEF;

	//KAD
	m_KadEnable = FCFG_DEF;
	m_KadLimit = FCFG_DEF;
	m_KadIntervals = FCFG_DEF;
	m_KadMaxFiles = FCFG_DEF;
	m_KadRepeatDelay = FCFG_DEF;

	//UDP
	m_UdpEnable = FCFG_DEF;
	m_UdpLimit = FCFG_DEF;
	m_UdpIntervals = FCFG_DEF;
	m_UdpGlobalIntervals = FCFG_DEF;
	m_UdpFilesPerServer = FCFG_DEF;
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	m_UseSourceCache = FCFG_DEF;
	m_SourceCacheLimit = FCFG_DEF;
	m_SourceCacheTime = FCFG_DEF;
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	m_AutoSoftLock = FCFG_DEF;
	m_AutoSoftLockLimit = FCFG_DEF;
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	m_AutoHardLimit = FCFG_DEF;
	m_AutoHardLimitTime = FCFG_DEF;
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	m_CategorySourceLimit = FCFG_DEF;
	m_CategorySourceLimitLimit = FCFG_DEF;
	m_CategorySourceLimitTime = FCFG_DEF;
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	m_GlobalSourceLimit = FCFG_DEF;
	m_GlobalSourceLimitLimit = FCFG_DEF;
	m_GlobalSourceLimitTime = FCFG_DEF;
	// NEO: GSL END

	m_MinSourcePerFile = FCFG_DEF;

	m_TCPConnectionRetry = FCFG_DEF; // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	m_SpreadReaskEnable = FCFG_DEF;
	m_SpreadReaskTime = FCFG_DEF;
	m_SourceReaskTime = FCFG_DEF;
	m_FullQSourceReaskTime = FCFG_DEF;
	m_NNPSourceReaskTime = FCFG_DEF;
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	m_DropTime = FCFG_DEF;

	//Bad
	m_BadSourceDrop = FCFG_DEF;
	m_BadSourceLimit = FCFG_DEF;
	m_BadSourceLimitMode = FCFG_DEF;
	m_BadSourceDropTime = FCFG_DEF;
	m_BadSourceDropMode = FCFG_DEF;

	//NNP
	m_NNPSourceDrop = FCFG_DEF;
	m_NNPSourceLimit = FCFG_DEF;
	m_NNPSourceLimitMode = FCFG_DEF;
	m_NNPSourceDropTime = FCFG_DEF;
	m_NNPSourceDropMode = FCFG_DEF;

	//FullQ
	m_FullQSourceDrop = FCFG_DEF;
	m_FullQSourceLimit = FCFG_DEF;
	m_FullQSourceLimitMode = FCFG_DEF;
	m_FullQSourceDropTime = FCFG_DEF;
	m_FullQSourceDropMode = FCFG_DEF;

	//HighQ
	m_HighQSourceDrop = FCFG_DEF;
	m_HighQSourceLimit = FCFG_DEF;
	m_HighQSourceLimitMode = FCFG_DEF;
	m_HighQSourceDropTime = FCFG_DEF;
	m_HighQSourceDropMode = FCFG_DEF;
	m_HighQSourceMaxRank = FCFG_DEF;
	m_HighQSourceRankMode = FCFG_DEF;

	m_DeadTime = FCFG_DEF;
	m_DeadTimeFWMulti = FCFG_DEF;
	m_GlobalDeadTime = FCFG_DEF;
	m_GlobalDeadTimeFWMulti = FCFG_DEF;
	// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_AutoSaveSources = FCFG_DEF;
	m_AutoSaveSourcesIntervals = FCFG_DEF;
	m_SourceStorageLimit = FCFG_DEF;

	m_StoreAlsoA4AFSources = FCFG_DEF;

	m_AutoLoadSources = FCFG_DEF;
	m_LoadedSourceCleanUpTime = FCFG_DEF;
	m_SourceStorageReaskLimit = FCFG_DEF;

	m_TotalSourceRestore = FCFG_DEF;

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_ReaskPropability = FCFG_DEF;
	m_UnpredictedPropability = FCFG_DEF;
	m_UnpredictedReaskPropability = FCFG_DEF;
 #endif // NEO_SA // NEO: NSA END

	m_AutoReaskStoredSourcesDelay = FCFG_DEF;

	m_GroupStoredSourceReask = FCFG_DEF;
	m_StoredSourceGroupIntervals = FCFG_DEF;
	m_StoredSourceGroupSize = FCFG_DEF;
#endif // NEO_SS // NEO: NSS END

	m_ForceA4AF = FCFG_DEF; // NEO: NXC - [NewExtendedCategories] 

	m_WantedParts.RemoveAll(); // NEO: MCS - [ManualChunkSelection]
}

bool CPartPreferencesEx::IsEmpty() const {
	return (
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_LcIntervals == FCFG_DEF &&

	m_LanSourceReaskTime == FCFG_DEF &&
	m_LanNNPSourceReaskTime == FCFG_DEF &&
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_VoodooXS == FCFG_DEF &&
#endif // VOODOO // NEO: VOODOO END

	// NEO: SRT - [SourceRequestTweaks]
	// General
	m_SourceLimit == FCFG_DEF &&

	// Management
	m_SwapLimit == FCFG_DEF &&

	//XS
	m_XsEnable == FCFG_DEF &&
	m_XsLimit == FCFG_DEF &&
	m_XsIntervals == FCFG_DEF &&
	m_XsClientIntervals == FCFG_DEF &&
	m_XsCleintDelay == FCFG_DEF &&
	m_XsRareLimit == FCFG_DEF &&

	// SVR
	m_SvrEnable == FCFG_DEF &&
	m_SvrLimit == FCFG_DEF &&
	m_SvrIntervals == FCFG_DEF &&

	//KAD
	m_KadEnable == FCFG_DEF &&
	m_KadLimit == FCFG_DEF &&
	m_KadIntervals == FCFG_DEF &&
	m_KadMaxFiles == FCFG_DEF &&
	m_KadRepeatDelay == FCFG_DEF &&

	//UDP
	m_UdpEnable == FCFG_DEF &&
	m_UdpLimit == FCFG_DEF &&
	m_UdpIntervals == FCFG_DEF &&
	m_UdpGlobalIntervals == FCFG_DEF &&
	m_UdpFilesPerServer == FCFG_DEF &&
	// NEO: SRT END

	// NEO: XSC - [ExtremeSourceCache]
	m_UseSourceCache == FCFG_DEF &&
	m_SourceCacheLimit == FCFG_DEF &&
	m_SourceCacheTime == FCFG_DEF &&
	// NEO: XSC END

	// NEO: ASL - [AutoSoftLock]
	m_AutoSoftLock == FCFG_DEF &&
	m_AutoSoftLockLimit == FCFG_DEF &&
	// NEO: ASL END

	// NEO: AHL - [AutoHardLimit]
	m_AutoHardLimit == FCFG_DEF &&
	m_AutoHardLimitTime == FCFG_DEF &&
	// NEO: AHL END

	// NEO: CSL - [CategorySourceLimit]
	m_CategorySourceLimit == FCFG_DEF &&
	m_CategorySourceLimitLimit == FCFG_DEF &&
	m_CategorySourceLimitTime == FCFG_DEF &&
	// NEO: CSL END

	// NEO: GSL - [GlobalSourceLimit]
	m_GlobalSourceLimit == FCFG_DEF &&
	m_GlobalSourceLimitLimit == FCFG_DEF &&
	m_GlobalSourceLimitTime == FCFG_DEF &&
	// NEO: GSL END

	m_MinSourcePerFile == FCFG_DEF &&

	m_TCPConnectionRetry == FCFG_DEF && // NEO: TCR - [TCPConnectionRetry]

	// NEO: DRT - [DownloadReaskTweaks]
	m_SpreadReaskEnable == FCFG_DEF &&
	m_SpreadReaskTime == FCFG_DEF &&
	m_SourceReaskTime == FCFG_DEF &&
	m_FullQSourceReaskTime == FCFG_DEF &&
	m_NNPSourceReaskTime == FCFG_DEF &&
	// NEO: DRT END

	// NEO: SDT - [SourcesDropTweaks]
	m_DropTime == FCFG_DEF &&

	//Bad
	m_BadSourceDrop == FCFG_DEF &&
	m_BadSourceLimit == FCFG_DEF &&
	m_BadSourceLimitMode == FCFG_DEF &&
	m_BadSourceDropTime == FCFG_DEF &&
	m_BadSourceDropMode == FCFG_DEF &&

	//NNP
	m_NNPSourceDrop == FCFG_DEF &&
	m_NNPSourceLimit == FCFG_DEF &&
	m_NNPSourceLimitMode == FCFG_DEF &&
	m_NNPSourceDropTime == FCFG_DEF &&
	m_NNPSourceDropMode == FCFG_DEF &&

	//FullQ
	m_FullQSourceDrop == FCFG_DEF &&
	m_FullQSourceLimit == FCFG_DEF &&
	m_FullQSourceLimitMode == FCFG_DEF &&
	m_FullQSourceDropTime == FCFG_DEF &&
	m_FullQSourceDropMode == FCFG_DEF &&

	//HighQ
	m_HighQSourceDrop == FCFG_DEF &&
	m_HighQSourceLimit == FCFG_DEF &&
	m_HighQSourceLimitMode == FCFG_DEF &&
	m_HighQSourceDropTime == FCFG_DEF &&
	m_HighQSourceDropMode == FCFG_DEF &&
	m_HighQSourceMaxRank == FCFG_DEF &&
	m_HighQSourceRankMode == FCFG_DEF &&

	m_DeadTime == FCFG_DEF &&
	m_DeadTimeFWMulti == FCFG_DEF &&
	m_GlobalDeadTime == FCFG_DEF &&
	m_GlobalDeadTimeFWMulti == FCFG_DEF &&
	// NEO: SDT END
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_AutoSaveSources == FCFG_DEF &&
	m_AutoSaveSourcesIntervals == FCFG_DEF &&
	m_SourceStorageLimit == FCFG_DEF &&

	m_StoreAlsoA4AFSources == FCFG_DEF &&

	m_AutoLoadSources == FCFG_DEF &&
	m_LoadedSourceCleanUpTime == FCFG_DEF &&
	m_SourceStorageReaskLimit == FCFG_DEF &&

	m_TotalSourceRestore == FCFG_DEF &&

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	m_ReaskPropability == FCFG_DEF &&
	m_UnpredictedPropability == FCFG_DEF &&
	m_UnpredictedReaskPropability == FCFG_DEF &&
 #endif // NEO_SA // NEO: NSA END

	m_AutoReaskStoredSourcesDelay == FCFG_DEF &&

	m_GroupStoredSourceReask == FCFG_DEF &&
	m_StoredSourceGroupIntervals == FCFG_DEF &&
	m_StoredSourceGroupSize == FCFG_DEF &&
#endif // NEO_SS // NEO: NSS END

	m_ForceA4AF == FCFG_DEF && // NEO: NXC - [NeoExtendedCategories]

	m_WantedParts.IsEmpty() // NEO: MCS - [ManualChunkSelection]
		);
}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_LcIntervals,GetLcIntervals,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_LanSourceReaskTime,GetLanSourceReaskTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_LanNNPSourceReaskTime,GetLanNNPSourceReaskTime,PartPrefs)
#endif //LANCAST // NEO: NLC END

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_VoodooXS,IsVoodooXS,PartPrefs)
#endif // VOODOO // NEO: VOODOO END

// NEO: SRT - [SourceRequestTweaks]
// General
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SourceLimit,GetSourceLimit,PartPrefs)

// Management
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SwapLimit,GetSwapLimit,PartPrefs)

//XS
IMPLEMENT_CFPEX(CPartPreferencesEx,m_XsEnable,IsXsEnable,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_XsLimit,GetXsLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_XsIntervals,GetXsIntervals,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_XsClientIntervals,GetXsClientIntervals,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_XsCleintDelay,GetXsCleintDelay,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_XsRareLimit,GetXsRareLimit,PartPrefs)

// SVR
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SvrEnable,IsSvrEnable,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SvrLimit,GetSvrLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SvrIntervals,GetSvrIntervals,PartPrefs)

//KAD
IMPLEMENT_CFPEX(CPartPreferencesEx,m_KadEnable,IsKadEnable,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_KadLimit,GetKadLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_KadIntervals,GetKadIntervals,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_KadMaxFiles,GetKadMaxFiles,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_KadRepeatDelay,GetKadRepeatDelay,PartPrefs)

//UDP
IMPLEMENT_CFPEX(CPartPreferencesEx,m_UdpEnable,IsUdpEnable,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_UdpLimit,GetUdpLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_UdpIntervals,GetUdpIntervals,PartPrefs)
// NEO: SRT END

// NEO: XSC - [ExtremeSourceCache]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_UseSourceCache,UseSourceCache,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SourceCacheLimit,GetSourceCacheLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SourceCacheTime,GetSourceCacheTime,PartPrefs)
// NEO: XSC END

// NEO: ASL - [AutoSoftLock]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoSoftLock,UseAutoSoftLock,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoSoftLockLimit,GetAutoSoftLockLimit,PartPrefs)
// NEO: ASL END

// NEO: AHL - [AutoHardLimit]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoHardLimit,UseAutoHardLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoHardLimitTime,GetAutoHardLimitTime,PartPrefs)
// NEO: AHL END

// NEO: CSL - [CategorySourceLimit]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_CategorySourceLimit,UseCategorySourceLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_CategorySourceLimitLimit,GetCategorySourceLimitLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_CategorySourceLimitTime,GetCategorySourceLimitTime,PartPrefs)
// NEO: CSL END

// NEO: GSL - [GlobalSourceLimit]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_GlobalSourceLimit,UseGlobalSourceLimit,PartPrefs)
// NEO: GSL END

IMPLEMENT_CFPEX(CPartPreferencesEx,m_MinSourcePerFile,GetMinSourcePerFile,PartPrefs)

IMPLEMENT_CFPEX(CPartPreferencesEx,m_TCPConnectionRetry,GetTCPConnectionRetry,PartPrefs) // NEO: TCR - [TCPConnectionRetry]

// NEO: DRT - [DownloadReaskTweaks]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SourceReaskTime,GetSourceReaskTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_FullQSourceReaskTime,GetFullQSourceReaskTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_NNPSourceReaskTime,GetNNPSourceReaskTime,PartPrefs)
// NEO: DRT END

// NEO: SDT - [SourcesDropTweaks]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_DropTime,GetDropTime,PartPrefs)

//Bad
IMPLEMENT_CFPEX(CPartPreferencesEx,m_BadSourceDrop,UseBadSourceDrop,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_BadSourceLimit,GetBadSourceLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_BadSourceLimitMode,GetBadSourceLimitMode,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_BadSourceDropTime,GetBadSourceDropTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_BadSourceDropMode,GetBadSourceDropMode,PartPrefs)

//NNP
IMPLEMENT_CFPEX(CPartPreferencesEx,m_NNPSourceDrop,UseNNPSourceDrop,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_NNPSourceLimit,GetNNPSourceLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_NNPSourceLimitMode,GetNNPSourceLimitMode,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_NNPSourceDropTime,GetNNPSourceDropTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_NNPSourceDropMode,GetNNPSourceDropMode,PartPrefs)

//FullQ
IMPLEMENT_CFPEX(CPartPreferencesEx,m_FullQSourceDrop,UseFullQSourceDrop,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_FullQSourceLimit,GetFullQSourceLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_FullQSourceLimitMode,GetFullQSourceLimitMode,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_FullQSourceDropTime,GetFullQSourceDropTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_FullQSourceDropMode,GetFullQSourceDropMode,PartPrefs)

//HighQ
IMPLEMENT_CFPEX(CPartPreferencesEx,m_HighQSourceDrop,UseHighQSourceDrop,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_HighQSourceLimit,GetHighQSourceLimit,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_HighQSourceLimitMode,GetHighQSourceLimitMode,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_HighQSourceDropTime,GetHighQSourceDropTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_HighQSourceDropMode,GetHighQSourceDropMode,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_HighQSourceMaxRank,GetHighQSourceMaxRank,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_HighQSourceRankMode,GetHighQSourceRankMode,PartPrefs)

IMPLEMENT_CFPEX(CPartPreferencesEx,m_DeadTime,GetDeadTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_DeadTimeFWMulti,GetDeadTimeFWMulti,PartPrefs)
// NEO: SDT END

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoSaveSources,AutoSaveSources,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoSaveSourcesIntervals,GetAutoSaveSourcesIntervals,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SourceStorageLimit,GetSourceStorageLimit,PartPrefs)

IMPLEMENT_CFPEX(CPartPreferencesEx,m_StoreAlsoA4AFSources,StoreAlsoA4AFSources,PartPrefs)

IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoLoadSources,AutoLoadSources,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_LoadedSourceCleanUpTime,GetLoadedSourceCleanUpTime,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_SourceStorageReaskLimit,GetSourceStorageReaskLimit,PartPrefs)

IMPLEMENT_CFPEX(CPartPreferencesEx,m_TotalSourceRestore,UseTotalSourceRestore,PartPrefs)

 #ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
IMPLEMENT_CFPEX(CPartPreferencesEx,m_ReaskPropability,GetReaskPropability,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_UnpredictedPropability,UseUnpredictedPropability,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_UnpredictedReaskPropability,GetUnpredictedReaskPropability,PartPrefs)
 #endif // NEO_SA // NEO: NSA END

IMPLEMENT_CFPEX(CPartPreferencesEx,m_AutoReaskStoredSourcesDelay,GetAutoReaskStoredSourcesDelay,PartPrefs)

IMPLEMENT_CFPEX(CPartPreferencesEx,m_GroupStoredSourceReask,GroupStoredSourceReask,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_StoredSourceGroupIntervals,GetStoredSourceGroupIntervals,PartPrefs)
IMPLEMENT_CFPEX(CPartPreferencesEx,m_StoredSourceGroupSize,GetStoredSourceGroupSize,PartPrefs)
#endif // NEO_SS // NEO: NSS END

// NEO: MCS - [ManualChunkSelection]
void CPartPreferencesEx::SetWantedPart(UINT part, uint8 status)
{
	if(status != PR_PART_NORMAL)
		m_WantedParts.SetAt(part,status);
	else
		m_WantedParts.RemoveKey(part);
}

uint8 CPartPreferencesEx::GetWantedPart(UINT part) const 
{ 
	uint8 status = PR_PART_NORMAL; 
	if(m_WantedParts.Lookup(part, status)) 
		return status; 
	return PR_PART_NORMAL; 
}
// NEO: MCS END

///////////////////////////////////////////////////////////////////////////////////////
// Helpers
//

int GetRightVal (int &mode, int def, int max, int val1, int val2, int val3)
{
	if(mode > FCFG_BASE)
		return FCFG_DEF;

	if(mode > max)
		mode = def;

	switch(mode){
		case 0:	return val1;
		case 1:	return val2;
		case 2:	return val3;
		default:
			ASSERT(0);
			return 0;
	}
}

CString EncodeFPValue(int Value)
{
	CString Text;
	switch(Value){
	case FCFG_STD:
	case FCFG_UNK:
	case FCFG_DEF:
		Text = FCFG_INI_DEF;
		break;
	case FCFG_GLB:
		Text = FCFG_INI_GLB;
		break;
	case FCFG_AUT:
		Text = FCFG_INI_AUT;
		break;
	default:
		Text.Format(_T("%d"),Value);
	} 
	return Text;
}

int DecodeFPValue(CString Text, int Default)
{
	Text.MakeLower();
	if(Text.Find(FCFG_INI_DEF) != -1)
		return FCFG_DEF;
	else if(Text.Find(FCFG_INI_GLB) != -1)
		return FCFG_GLB;
	else if(Text.Find(FCFG_INI_AUT) != -1)
		return FCFG_AUT;
	else if(Text == _T(""))
		return Default;
	else if(Text == _T("0"))
		return 0;

	int Value = _tstoi(Text);
	if(Value == 0) // means the value is invalid
		return Default;
	return Value;
}

void CheckFPValue(int val, int def, bool glb, bool cat)
{
	if(glb){ // the most special values are not valid for the base
		if(val != FCFG_AUT || def != FCFG_AUT) // the only valid one it AUT, and if it is, it is also the default
			val = def;
	} else {
		if(val != FCFG_DEF || val != FCFG_GLB || !(val == FCFG_AUT && def == FCFG_AUT)) // all the rest is only valid temporary in the prefs tree
			val = FCFG_DEF;
		else if(val == FCFG_GLB && !cat) // glb is not valid for category prefs
			val = FCFG_DEF;
	}
}

bool CheckModes(int &val, int &mod) // this one checks the validiti of limit values that depand on a setting mode
{
	if((val == FCFG_DEF) != (mod == FCFG_DEF)
	|| (val == FCFG_GLB) != (mod == FCFG_GLB)){
		val = FCFG_DEF;
		mod = FCFG_DEF;
		return false;
	}

	return true;
}
// NEO: FCFG END <-- Xanatos --