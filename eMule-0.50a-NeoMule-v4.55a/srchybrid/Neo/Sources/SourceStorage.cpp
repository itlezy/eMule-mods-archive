//this file is part of NeoMule
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
#include "Preferences.h"
#include "PartFile.h"
#include "SafeFile.h"
#include "Packets.h"
#include "emule.h"
#include "Log.h"
#include "emuledlg.h"
#include "friend.h"
#include "friendlist.h"
#include "UpDownClient.h"
#include "ClientList.h"
#include "Transferwnd.h"
#include "ClientCredits.h"
#include "downloadqueue.h"
#include "Neo/NeoVersion.h"
#include "Neo/Defaults.h"
#include "Neo/NeoOpCodes.h"
#include "Neo/Functions.h"
#include "Neo/FilePreferences.h"
#include "Neo/NeoPreferences.h"
//#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
#include "Neo/Sources/SourceList.h"
//#endif // NEO_CD // NEO: NCD END

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
bool CPartFile::SortCompare(POSITION pos1, POSITION pos2){
	CUpDownClient* src1 = srclist.GetAt(pos1);
	CUpDownClient* src2 = srclist.GetAt(pos2);
	if(src2->Source() == NULL)
		return false;
	if(src1->Source() == NULL)
		return true;

	// handle unpredicted propability
	const bool ValidQuality1 = src1->Source()->GetAnalisisQuality() > 0;
	const bool ValidQuality2 = src2->Source()->GetAnalisisQuality() > 0;
	if(ValidQuality1 != ValidQuality2)
		return ValidQuality2;
	
	const int Probability1 = src1->Source()->GetAvalibilityProbability();
	const int Probability2 = src2->Source()->GetAvalibilityProbability();
	if(Probability1 == Probability2)
		return (src1->Source()->GetRemindingIPTime() < src2->Source()->GetRemindingIPTime());
	return (Probability1 < Probability2);
	
}

void CPartFile::SortSwap(POSITION pos1, POSITION pos2){
	CUpDownClient* src1 = srclist.GetAt(pos1);
	CUpDownClient* src2 = srclist.GetAt(pos2);
	srclist.SetAt(pos1, src2);
	srclist.SetAt(pos2, src1);
}

void CPartFile::HeapSortList(UINT first, UINT last){
	UINT r;
	POSITION pos1 = srclist.FindIndex(first);
	for ( r = first; !(r & 0x8000) && (r<<1) < last; ){
		UINT r2 = (r<<1)+1;
		POSITION pos2 = srclist.FindIndex(r2);
		if (r2 != last){
			POSITION pos3 = pos2;
			srclist.GetNext(pos3);
			if (!SortCompare(pos2, pos3)){
				pos2 = pos3;
				r2++;
			}
		}
		if (!SortCompare(pos1, pos2)) {
			SortSwap(pos1, pos2);
			r = r2;
			pos1 = pos2;
		}
		else
			break;
	}
}

void CPartFile::SortSourceList(){
	UINT n = srclist.GetCount();
	if (!n)
		return;
	UINT i;
	for ( i = n/2; i--; )
		HeapSortList(i, n-1);
	for ( i = n; --i; ){
		SortSwap(srclist.FindIndex(0), srclist.FindIndex(i));
		HeapSortList(0, i-1);
	}
}

void CPartFile::AnalizerSources()
{
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if(cur_src->GetDownloadState() != DS_LOADED)
			continue;

		// if analyser is active but no source object drop the source right away
		if(cur_src->Source() == 0){
			theApp.downloadqueue->RemoveSource( cur_src );
			continue;
		}

		// Never reask low ID sources, the propobility for the same server/kad buddy to stay constant is near to 0
		if(cur_src->HasLowID()){
			theApp.downloadqueue->RemoveSource( cur_src );
			continue;
		}

		// calculate the availibility propability, the value will be valid for at least an hour
		// after this time all loaded but yet not reasked soruces will be dropped automaticly
		cur_src->Source()->CalculateAvalibilityProbability();

		// if the propability of availibility is 0 or below remove.
		if(cur_src->Source()->GetAvalibilityProbability() <= 0){
			theApp.downloadqueue->RemoveSource( cur_src );
			continue;
		}
	}
}
#endif // NEO_SA // NEO: NSA END

bool CPartFile::SaveSources()
{
	if (IsStopped()){
		return false;
	}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if(m_fullname.IsEmpty()) // keep voodoo files completly virtual
		return true;
#endif // VOODOO // NEO: VOODOO END

	CString strSrcFile;
	strSrcFile.Format(m_fullname);
	strSrcFile += PARTSRC_EXT;

	CString strTmpFile(strSrcFile);
	strTmpFile += PARTSRC_TMP_EXT;

	// save sources to part.src file
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strTmpFile, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError;
		strError.Format(GetResString(IDS_X_ERR_SAVESRC), strSrcFile, GetFileName());
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
		file.WriteUInt8(SRCFILE_VERSION);

		SaveSources(&file);

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError;
		strError.Format(GetResString(IDS_X_ERR_SAVESRC), strSrcFile, GetFileName());
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

	// after successfully writing the temporary part.src file...
	if (_tremove(strSrcFile) != 0 && errno != ENOENT){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to remove \"%s\" - %s"), strSrcFile, _tcserror(errno));
	}

	if (_trename(strTmpFile, strSrcFile) != 0){
		int iErrno = errno;
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to move temporary part.met.src file \"%s\" to \"%s\" - %s"), strTmpFile, strSrcFile, _tcserror(iErrno));

		CString strError;
		strError.Format(GetResString(IDS_X_ERR_SAVESRC), strSrcFile, GetFileName());
		strError += _T(" - ");
		strError += strerror(iErrno);
		ModLogError(_T("%s"), strError);
		return false;
	}

	// create a backup of the successfully written part.met file
	CString BAKName(strSrcFile);
	BAKName.Append(PARTSRC_BAK_EXT);
	if (!::CopyFile(strSrcFile, BAKName, FALSE)){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Failed to create backup of %s (%s) - %s"), strSrcFile, GetFileName(), GetErrorMessage(GetLastError()));
	}

	m_uLastSaveSource = ::GetTickCount();

	return true;
}

bool CPartFile::SaveSources(CFileDataIO* file)
{
	UINT uSourceCount = 0;
	ULONG uSourceCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uSourceCount);

	/*
	* Save Sources here
	*/
	for (POSITION pos = srclist.GetHeadPosition(); pos != NULL;)
	{
		CUpDownClient* cur_src = srclist.GetNext(pos);
		if( !cur_src->IsSourceConfirmed() // It must be a Real client
			|| !cur_src->IsEd2kClient()) // This system Saves only ED2K clinets, http sources will be saved by an other procedure
			continue;
		if(!PartPrefs->UseTotalSourceRestore() == TRUE && cur_src->HasLowID())
			continue;

		cur_src->StoreToFile(file); // Save general client informations
		uSourceCount++;
		if(uSourceCount > GetSourceStorageSourceLimit())
			break;
	}

	if(PartPrefs->StoreAlsoA4AFSources())
	{
		for (POSITION pos = A4AFsrclist.GetHeadPosition(); pos != 0; )
		{
			CUpDownClient* cur_src = A4AFsrclist.GetNext(pos);
			if( !cur_src->IsSourceConfirmed() // It must be a Real client
				|| !cur_src->IsEd2kClient()) // This system Saves only ED2K clinets, http sources will be saved by an other procedure
				continue;
			if(!PartPrefs->UseTotalSourceRestore() == TRUE && cur_src->HasLowID())
				continue;

			cur_src->StoreToFile(file); // Save general client informations
			uSourceCount++;
			if(uSourceCount > GetSourceStorageSourceLimit())
				break;
		}
	}

	file->Seek(uSourceCountFilePos, CFile::begin);
	file->WriteUInt32(uSourceCount);
	file->Seek(0, CFile::end);

	return true;
}

bool CPartFile::LoadSources()
{
	if (IsStopped()){
		return false;
	}

	CString strSrcFile;
	strSrcFile.Format(m_fullname);
	strSrcFile += PARTSRC_EXT;

	uint8 version;
	
	// readfile tweaks form part.src file
	CSafeBufferedFile file;
	CFileException fexpMet;
	if (!file.Open(strSrcFile, CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexpMet)){
		if (fexpMet.m_cause != CFileException::fileNotFound){
			CString strError;
			strError.Format(GetResString(IDS_X_ERR_OPENSRC), strSrcFile, _T(""));
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
		
		if (version != SRCFILE_VERSION /*version > SRCFILE_VERSION || version < SRCFILE_VERSION_OLD*/){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_BADSRCVERSION), strSrcFile, GetFileName());
			file.Close();
			return false;
		}
		
		LoadSources(&file);
		
		file.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_SRCMEOCORRUPT), strSrcFile, GetFileName());
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_FILEERROR), strSrcFile, GetFileName(), buffer);
		}
		error->Delete();
		return false;
	}
	catch(...){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_ERR_METCORRUPT), strSrcFile, GetFileName());
		ASSERT(0);
		return false;
	}

#ifdef NEO_SA // NEO: NSA - [NeoSourceAnalyzer]
	UpdateAutoDownPriority(); // for auto mode sellection
	if(NeoPrefs.EnableSourceAnalizer()){
		AnalizerSources();
		SortSourceList();
	}
#endif // NEO_SA // NEO: NSA END

	m_uLastLoadSource = ::GetTickCount();

	return true;
}

bool CPartFile::LoadSources(CFileDataIO* file)
{
	CUpDownClient* toadd = NULL;
	try{
		UINT RecordsNumber = file->ReadUInt32();
		for (UINT i = 0; i < RecordsNumber; i++) {
			/*
			* Load All Sources here
			*/
			toadd = new CUpDownClient();
			CPartFile* tempreqfile = toadd->CreateFromFile(file);

			if (!theApp.clientlist->AttachToAlreadyKnown(&toadd,0,ATTACH_LOAD)){
				// here we know that the client instance 'source' is a new created client instance (see callers) 
				// which is therefor not already in the clientlist, we can avoid the check for duplicate client list entries 
				// when adding this client
				theApp.clientlist->AddClient(toadd);
			}

			// to prevent haveing clinets with reqfile = NULL but files in other request lists
			// we will add the source immidetly to the proper reqfile, even if we are currently in an other PartFile
			// or when the right owner is gone, take the source over
			if(tempreqfile == NULL || tempreqfile->IsStopped())
				tempreqfile = this;

			// The source may be already added ba an other a4af list, or was found for an other file, bevoure this file was resumed
			// Add client as source if the clinet isn't already a source already for some file
			if(toadd->GetRequestFile() == NULL){
				ASSERT(tempreqfile->srclist.Find(toadd) == NULL);
				toadd->SetSourceFrom(SF_STORAGE);
				toadd->SetRequestFile(tempreqfile);
				if(!PartPrefs->UseTotalSourceRestore() || toadd->IsLongTermStorred()) // short term storred sources goes active imminetly if total restore is enabled
					toadd->SetDownloadState(DS_LOADED);
				tempreqfile->srclist.AddTail(toadd);
				toadd->SetSafeReAskTime(); // fix 
				theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(tempreqfile,toadd,false);
			}

			// if an other file already have this source, add it to our A4AF List
			if(toadd->GetRequestFile() != this){
				if (toadd->AddRequestForAnotherFile(this))
					theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(this,toadd,true);
			}
		}
	}
	catch(CFileException* error){
		if(toadd)
			delete toadd;
		throw error;
	}
	return true;
}

void CUpDownClient::StoreToFile(CFileDataIO* file)
{
	UINT uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

	// Note: We need this informations to load a source properly
	if(reqfile){
		CTag tag(SFT_SOURCE_REQFILE, reqfile->GetFileHash());
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_dwUserIP){
		CTag tag(SFT_IP, m_dwUserIP);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}else if(m_nConnectIP){
		CTag tag(SFT_IP2, m_nConnectIP);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_dwServerIP){
		CTag tag(SFT_SERVER_IP, m_dwServerIP);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_nUserIDHybrid){
		CTag tag(SFT_HYBRID_ID, m_nUserIDHybrid);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_nUserPort){
		CTag tag(SFT_PORT, m_nUserPort);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_nServerPort){
		CTag tag(SFT_SERVER_PORT, m_nServerPort);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (GetUserName()){
		CTag tag(SFT_USER_NAME, GetUserName());
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (HasValidHash()){
		CTag tag(SFT_USER_HASH, m_achUserHash);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_nUDPPort){
		CTag tag(SFT_UDP_PORT, m_nUDPPort);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_nKadPort){
		CTag tag(SFT_KAD_PORT, m_nKadPort);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (HasValidBuddyID()){
		CTag tag(SFT_BUDDY_ID, m_achBuddyID);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_nBuddyIP){
		CTag tag(SFT_BUDDY_IP, m_nBuddyIP);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_nBuddyPort){
		CTag tag(SFT_BUDDY_PORT, m_nBuddyPort);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_uLastSeen){
		CTag tag(SFT_LAST_SEEN, m_uLastSeen);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (m_clientSoft != SO_UNKNOWN){
		CTag tag(SFT_CLIENT_SOFTWARE, m_clientSoft);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (!m_strClientSoftware.IsEmpty()){
		CTag tag(SFT_SOFTWARE_VERSION, m_strClientSoftware);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}
	
	if (m_nClientVersion){
		CTag tag(SFT_CLIENT_VERSION, m_nClientVersion);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}

	if (!m_strModVersion.IsEmpty()){
		CTag tag(SFT_CLIENT_MODIFICATION, m_strModVersion);
		tag.WriteNewEd2kTag(file);
		uTagCount++;
	}


	// Handle unknown tags
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

CPartFile* CUpDownClient::CreateFromFile(CFileDataIO* file)
{
	CPartFile* tempreqfile = NULL;
	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
		switch (newtag->GetNameID()){

			case SFT_SOURCE_REQFILE:{
                ASSERT( newtag->IsHash() );
				if(newtag->IsHash())
					tempreqfile = theApp.downloadqueue->GetFileByID(newtag->GetHash());
                delete newtag;
                break;
            }

			case SFT_IP:{
                ASSERT( newtag->IsInt() );
				SetIP( newtag->GetInt() ); // m_nConnectIP = m_dwUserIP
                delete newtag;
                break;
            }

			case SFT_IP2:{
                ASSERT( newtag->IsInt() );
				m_nConnectIP = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_SERVER_IP:{
                ASSERT( newtag->IsInt() );
				m_dwServerIP = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_HYBRID_ID:{
                ASSERT( newtag->IsInt() );
                m_nUserIDHybrid = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_PORT:{
                ASSERT( newtag->IsInt() );
				m_nUserPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_SERVER_PORT:{
                ASSERT( newtag->IsInt() );
				m_nServerPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_USER_NAME:{
                ASSERT( newtag->IsStr() );
				if(newtag->IsStr())
					SetUserName(newtag->GetStr());
                delete newtag;
                break;
            }

			case SFT_USER_HASH:{
                ASSERT( newtag->IsHash() );
				if(newtag->IsHash())
					SetUserHash(newtag->GetHash());
                delete newtag;
                break;
            }

			case SFT_UDP_PORT:{
                ASSERT( newtag->IsInt() );
                m_nUDPPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_KAD_PORT:{
                ASSERT( newtag->IsInt() );
                m_nKadPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_BUDDY_ID:{
                ASSERT( newtag->IsHash() );
				if(newtag->IsHash())
					SetBuddyID(newtag->GetHash());
                delete newtag;
                break;
            }

			case SFT_BUDDY_IP:{
                ASSERT( newtag->IsInt() );
				m_nBuddyIP = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_BUDDY_PORT:{
                ASSERT( newtag->IsInt() );
				m_nBuddyPort = (uint16)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_LAST_SEEN:{
                ASSERT( newtag->IsInt() );
				m_uLastSeen = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_CLIENT_SOFTWARE:{
                ASSERT( newtag->IsInt() );
                m_clientSoft = (_EClientSoftware)newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_SOFTWARE_VERSION:{
                ASSERT( newtag->IsStr() );
				if(newtag->IsStr())
					m_strClientSoftware = newtag->GetStr();
                delete newtag;
                break;
            }

			case SFT_CLIENT_VERSION:{
                ASSERT( newtag->IsInt() );
                m_nClientVersion = newtag->GetInt();
                delete newtag;
                break;
            }

			case SFT_CLIENT_MODIFICATION:{
                ASSERT( newtag->IsStr() );
				if(newtag->IsStr())
					m_strModVersion = newtag->GetStr();
                delete newtag;
                break;
            }

			// Handle unknown tags
			default:{
				taglist.Add(newtag);
			}
		}
	}

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase]
	if(NeoPrefs.EnableSourceList())
	{
		if(HasValidHash())
			LinkSource(theApp.sourcelist->GetSource(m_achUserHash));
	}
#endif // NEO_CD // NEO: NCD END

	if(HasValidHash())
		credits = theApp.clientcredits->GetCredit(m_achUserHash);

	if(m_dwUserIP && m_nUserPort && HasValidHash()){
		m_Friend = theApp.friendlist->SearchFriend(m_achUserHash, m_dwUserIP, m_nUserPort);
		if(m_Friend)
			m_Friend->SetLinkedClient(this);// Link the friend to that client
	}

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	m_structUserCountry = theApp.ip2country->GetCountryFromIP(m_dwUserIP);
#endif // IP2COUNTRY // NEO: IP2C END

	return tempreqfile;
}

void CUpDownClient::ClearTags()
{
	for (int i = 0; i < taglist.GetSize(); i++)
		delete taglist[i];
	taglist.RemoveAll();

}
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
