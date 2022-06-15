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
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <share.h>
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif
#include "emule.h"
#include "KnownFile.h"
#include "KnownFileList.h"
#include "SharedFileList.h"
#include "UpDownClient.h"
#include "MMServer.h"
#include "ClientList.h"
#include "opcodes.h"
#include "Neo/Ini2.h" // NEO: INI - [PlusIniClass] <-- Xanatos --
//#include "ini2.h"
#include "FrameGrabThread.h"
#include "CxImage/xImage.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "PartFile.h"
#include "Packets.h"
#include "Kademlia/Kademlia/SearchManager.h"
#include "Kademlia/Kademlia/Entry.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "SafeFile.h"
#include "shahashset.h"
#include "Log.h"
#include "MD4.h"
#include "Collection.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "MediaInfo.h"
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "Neo/NeoOpcodes.h" // NEO: NMP - [NeoModProt] <-- Xanatos --

// id3lib
#pragma warning(disable:4100) // unreferenced formal parameter
#include <id3/tag.h>
#include <id3/misc_support.h>
#pragma warning(default:4100) // unreferenced formal parameter
extern wchar_t *ID3_GetStringW(const ID3_Frame *frame, ID3_FieldID fldName);

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	META_DATA_VER	1

IMPLEMENT_DYNAMIC(CKnownFile, CAbstractFile)

CKnownFile::CKnownFile()
{
	m_iPartCount = 0;
	m_iED2KPartCount = 0;
	m_iED2KPartHashCount = 0;
	m_tUtcLastModified = (UINT)-1;
	if(thePrefs.GetNewAutoUp()){
		m_iUpPriority = PR_HIGH;
		m_bAutoUpPriority = true;
	}
	else{
		m_iUpPriority = PR_NORMAL;
		m_bAutoUpPriority = false;
	}
	statistic.fileParent = this;
	m_iPermissions = PERM_DEFAULT; // NEO: SSP - [ShowSharePermissions] <-- Xanatos --
	(void)m_strComment;
	m_PublishedED2K = false;
	//kadFileSearchID = 0; // NEO: KII - [KadInterfaceImprovement] <-- Xanatos --
	SetLastPublishTimeKadSrc(0,0);
	m_nCompleteSourcesTime = time(NULL);
	m_nCompleteSourcesSize = 0; // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
	m_nCompleteSourcesCount = 1;
	m_nCompleteSourcesCountLo = 1;
	m_nCompleteSourcesCountHi = 1;
	m_uMetaDataVer = 0;
	m_lastPublishTimeKadSrc = 0;
	m_lastPublishTimeKadNotes = 0;
	m_lastBuddyIP = 0;
	m_pAICHHashSet = new CAICHHashSet(this);
	m_pCollection = NULL;
	m_verifiedFileType=FILETYPE_UNKNOWN;

	m_ReadThread = NULL; // NEO: RBT - [ReadBlockThread] <-- Xanatos --
	m_category=0;  // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	KnownPrefs = &NeoPrefs.KnownPrefs; // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
	m_uLastIPSCalcTime = 0;
	m_IPSPartsInfo = NULL;
	// NEO: IPS END <-- Xanatos --
	// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
	m_uReleasePriority = FALSE;
	m_uLastReleaseModifyer = 0;
	m_fReleaseModifyer = 0;
	m_bPowerShared = false;
	// NEO: SRS END <-- Xanatos --
	m_startUploadTime = time(NULL); // NEO: MQ - [MultiQueue] <-- Xanatos --
	// NEO: PP - [PasswordProtection] -- Xanatos -->
	m_pwProt.Empty();
	m_isPWProtShow = false;
	m_bProtectionLoaded = false;
	// NEO: PP END <-- Xanatos --
	memset(&m_CRC32,0,sizeof(BYTE)*4); // NEO: CRC - [MorphCRCTag] <-- Xanatos --
}

CKnownFile::~CKnownFile()
{
	for (int i = 0; i < hashlist.GetSize(); i++)
		delete[] hashlist[i];
	delete m_pAICHHashSet;
	delete m_pCollection;

	// NEO: RBT - [ReadBlockThread] -- Xanatos -->
	if(m_ReadThread != NULL)
	{
		m_BlocksToReadLocker.Lock();
		((CReadBlockFromFileThread*)m_ReadThread)->file.Close();
		TerminateThread(m_ReadThread->m_hThread, 100);
		m_ReadThread = NULL;
		for (POSITION pos = m_BlocksToRead.GetHeadPosition();pos != 0;){
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			ReadBlockOrder* readOrder = m_BlocksToRead.GetNext(pos);
			if(readOrder->m_voodoo)
				delete (sDataRequest*)readOrder->m_request;
			delete readOrder;
#else
			delete m_BlocksToRead.GetNext(pos);
#endif // VOODOO // NEO: VOODOO END
		}
		m_BlocksToReadLocker.Unlock();
	}
	// NEO: RBT END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	CMasterDatas* Datas;
	CVoodooSocket* Master;
	POSITION pos = m_MasterMap.GetStartPosition();
	while (pos){
		m_MasterMap.GetNextAssoc(pos, Master, Datas);
		delete Datas;
	}
	m_MasterMap.RemoveAll();
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

	// NEO: IPS - [InteligentPartSharing] -- Xanatos -->
	if(m_IPSPartsInfo){
		m_IPSPartsInfo->RemoveAll();
		delete m_IPSPartsInfo;
	}
	// NEO: IPS END <-- Xanatos --

	// NEO: FCFG - [FileConfiguration] -- Xanatos -->
	if(KnownPrefs->IsFilePrefs())
		delete KnownPrefs;
	// NEO: FCFG END <-- Xanatos --
}

#ifdef _DEBUG
void CKnownFile::AssertValid() const
{
	CAbstractFile::AssertValid();

	(void)m_tUtcLastModified;
	(void)statistic;
	(void)m_nCompleteSourcesTime;
	(void)m_nCompleteSourcesSize; // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
	(void)m_nCompleteSourcesCount;
	(void)m_nCompleteSourcesCountLo;
	(void)m_nCompleteSourcesCountHi;
	m_ClientUploadList.AssertValid();
	m_AvailPartFrequency.AssertValid();
	m_AvailIncPartFrequency.AssertValid(); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	hashlist.AssertValid();
	(void)m_strDirectory;
	(void)m_strFilePath;
	(void)m_iPartCount;
	(void)m_iED2KPartCount;
	(void)m_iED2KPartHashCount;
	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH );
	CHECK_BOOL(m_bAutoUpPriority);
	(void)s_ShareStatusBar;
	CHECK_BOOL(m_PublishedED2K);
	//(void)kadFileSearchID; // NEO: KII - [KadInterfaceImprovement] <-- Xanatos --
	(void)m_lastPublishTimeKadSrc;
	(void)m_lastPublishTimeKadNotes;
	(void)m_lastBuddyIP;
	(void)wordlist;
	(void)m_category; // NEO: NSC - [NeoSharedCategories] <-- Xanatos --
	// NEO: PP - [PasswordProtection] -- Xanatos -->
	(void)m_pwProt;;
	CHECK_BOOL(m_isPWProtShow);
	CHECK_BOOL(m_bProtectionLoaded);
	// NEO: PP END <-- Xanatos --
}

void CKnownFile::Dump(CDumpContext& dc) const
{
	CAbstractFile::Dump(dc);
}
#endif

CBarShader CKnownFile::s_ShareStatusBar(16);

// NEO: MOD - [ClassicShareStatusBar] -- Xanatos -->
void CKnownFile::DrawClassicShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
{
	const COLORREF crMissing = RGB(255, 0, 0);
	s_ShareStatusBar.SetFileSize(GetFileSize());
	s_ShareStatusBar.SetHeight(rect->bottom - rect->top);
	s_ShareStatusBar.SetWidth(rect->right - rect->left);
	s_ShareStatusBar.Fill(crMissing);

	if (!onlygreyrect && !m_AvailPartFrequency.IsEmpty()) {
		COLORREF crProgress;
		COLORREF crHave;
		COLORREF crPending;
		if(bFlat) { 
			crProgress = RGB(0, 150, 0);
			crHave = RGB(0, 0, 0);
			crPending = RGB(255,208,0);
		} else { 
			crProgress = RGB(0, 224, 0);
			crHave = RGB(104, 104, 104);
			crPending = RGB(255, 208, 0);
		} 
		for (int i = 0; i < GetPartCount(); i++){
			if(m_AvailPartFrequency[i] > 0 ){
				COLORREF color = RGB(0, (210-(22*(m_AvailPartFrequency[i]-1)) <	0)? 0:210-(22*(m_AvailPartFrequency[i]-1)), 255);
				s_ShareStatusBar.FillRange(PARTSIZE*(i),PARTSIZE*(i+1),color);
			}
		}
	}
   	s_ShareStatusBar.Draw(dc, rect->left, rect->top, bFlat); 
} 
// NEO: MOD END <-- Xanatos --

void CKnownFile::DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
{
	s_ShareStatusBar.SetFileSize(GetFileSize());
	s_ShareStatusBar.SetHeight(rect->bottom - rect->top);
	s_ShareStatusBar.SetWidth(rect->right - rect->left);

    if(m_ClientUploadList.GetSize() > 0 || m_nCompleteSourcesCountHi > 1) {
        // We have info about chunk frequency in the net, so we will color the chunks we have after perceived availability.
    	const COLORREF crMissing = RGB(255, 0, 0);
	    s_ShareStatusBar.Fill(crMissing);

	    if (!onlygreyrect) {
		    COLORREF crProgress;
		    COLORREF crHave;
		    COLORREF crPending;
		    if(bFlat) { 
			    crProgress = RGB(0, 150, 0);
			    crHave = RGB(0, 0, 0);
			    crPending = RGB(255,208,0);
		    } else { 
			    crProgress = RGB(0, 224, 0);
			    crHave = RGB(104, 104, 104);
			    crPending = RGB(255, 208, 0);
	        }

            uint32 tempCompleteSources = m_nCompleteSourcesCountLo;
            if(tempCompleteSources > 0) {
                tempCompleteSources--;
            }

		    for (UINT i = 0; i < GetPartCount(); i++){
                uint32 frequency = tempCompleteSources;
                if(!m_AvailPartFrequency.IsEmpty()) {
                    frequency = max(m_AvailPartFrequency[i], tempCompleteSources);
                }

			    if(frequency > 0 ){
				    COLORREF color = RGB(0, (22*(frequency-1) >= 210)? 0:210-(22*(frequency-1)), 255);
				    s_ShareStatusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1),color);
			    }
	        }
	    }
    } else {
        // We have no info about chunk frequency in the net, so just color the chunk we have as black.
        COLORREF crNooneAsked;
		if(bFlat) { 
		    crNooneAsked = RGB(0, 0, 0);
		} else { 
		    crNooneAsked = RGB(104, 104, 104);
	    }
		s_ShareStatusBar.Fill(crNooneAsked);
    }

   	s_ShareStatusBar.Draw(dc, rect->left, rect->top, bFlat); 
} 

// SLUGFILLER: heapsortCompletesrc
static void HeapSort(CArray<uint16,uint16> &count, UINT first, UINT last){
	UINT r;
	for ( r = first; !(r & (UINT)INT_MIN) && (r<<1) < last; ){
		UINT r2 = (r<<1)+1;
		if (r2 != last)
			if (count[r2] < count[r2+1])
				r2++;
		if (count[r] < count[r2]){
			uint16 t = count[r2];
			count[r2] = count[r];
			count[r] = t;
			r = r2;
		}
		else
			break;
	}
}
// SLUGFILLER: heapsortCompletesrc

void CKnownFile::UpdateFileRatingCommentAvail(bool bForceUpdate)
{
	bool bOldHasComment = m_bHasComment;
	UINT uOldUserRatings = m_uUserRating;

	m_bHasComment = false;
	UINT uRatings = 0;
	UINT uUserRatings = 0;

	// NEO: XC - [ExtendedComments] -- Xanatos -->
	for(POSITION pos = m_CommentList.GetHeadPosition(); pos != NULL; )
	{
		KnownComment* comment = m_CommentList.GetNext(pos);

		if (!m_bHasComment && !comment->m_strComment.IsEmpty())
			m_bHasComment = true;
		uint16 rating = comment->m_uRating;
		if(rating!=0)
		{
			uRatings++;
			uUserRatings += rating;
		}
	}
	// NEO: XC END <-- Xanatos --

	for(POSITION pos = m_kadNotes.GetHeadPosition(); pos != NULL; )
	{
		Kademlia::CEntry* entry = m_kadNotes.GetNext(pos);
		if (!m_bHasComment && !entry->GetStrTagValue(TAG_DESCRIPTION).IsEmpty())
			m_bHasComment = true;
		UINT rating = (UINT)entry->GetIntTagValue(TAG_FILERATING);
		if (rating != 0)
		{
			uRatings++;
			uUserRatings += rating;
		}
	}

	if (uRatings)
		m_uUserRating = (uint32)ROUND((float)uUserRatings / uRatings);
	else
		m_uUserRating = 0;

	if (bOldHasComment != m_bHasComment || uOldUserRatings != m_uUserRating || bForceUpdate)
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
}

void CKnownFile::UpdatePartsInfo()
{
	// Cache part count
	UINT partcount = GetPartCount();
	bool flag = (time(NULL) - m_nCompleteSourcesTime > 0); 
	// NEO: MOD - [RelativeChunkDisplay] -- Xanatos -->
	if(m_nCompleteSourcesSize * 5 / 4 < (uint32)m_ClientUploadList.GetSize()) // if teh source amount havly changed force a recalc, also for the release prio
		flag = true;
	// NEO: MOD END <-- Xanatos --

	// Reset part counters
	if ((UINT)m_AvailPartFrequency.GetSize() < partcount)
		m_AvailPartFrequency.SetSize(partcount);
	for (UINT i = 0; i < partcount; i++)
		m_AvailPartFrequency[i] = 0;

	CArray<uint16, uint16> count;
	if (flag)
		count.SetSize(0, m_ClientUploadList.GetSize());
	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		//This could be a partfile that just completed.. Many of these clients will not have this information.
		// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if(status == NULL || status->GetPartCount() != partcount)
			continue;
		uint8* abyUpPartStatus = status->GetPartStatus();
		if(abyUpPartStatus == NULL)
			continue;
		for (UINT i = 0; i < partcount; i++)
		// NEO: SCFS END <-- Xanatos --
		//if (cur_src->m_abyUpPartStatus && cur_src->GetUpPartCount() == partcount)
		{
			for (UINT i = 0; i < partcount; i++)
			{
				if(abyUpPartStatus[i]) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
				//if (cur_src->IsUpPartAvailable(i))
					m_AvailPartFrequency[i] += 1;
			}
			if (flag)
				//count.Add(cur_src->GetUpCompleteSourcesCount());
				count.Add(status->GetCompleteSourcesCount()); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
		}
	}

	if (flag)
	{
		m_nCompleteSourcesCount = m_nCompleteSourcesCountLo = m_nCompleteSourcesCountHi = 0;

		if (partcount > 0)
			m_nCompleteSourcesCount = m_AvailPartFrequency[0];
		for (UINT i = 1; i < partcount; i++)
		{
			if (m_nCompleteSourcesCount > m_AvailPartFrequency[i])
				m_nCompleteSourcesCount = m_AvailPartFrequency[i];
		}
	
		count.Add(m_nCompleteSourcesCount+1); // plus 1 since we have the file complete too
	
		int n = count.GetSize();
		if (n > 0)
		{
			// SLUGFILLER: heapsortCompletesrc
			int r;
			for (r = n/2; r--; )
				HeapSort(count, r, n-1);
			for (r = n; --r; ){
				uint16 t = count[r];
				count[r] = count[0];
				count[0] = t;
				HeapSort(count, 0, r-1);
			}
			// SLUGFILLER: heapsortCompletesrc
			
			// calculate range
			int i = n >> 1;			// (n / 2)
			int j = (n * 3) >> 2;	// (n * 3) / 4
			int k = (n * 7) >> 3;	// (n * 7) / 8

			//For complete files, trust the people your uploading to more...

			//For low guess and normal guess count
			//	If we see more sources then the guessed low and normal, use what we see.
			//	If we see less sources then the guessed low, adjust network accounts for 100%, we account for 0% with what we see and make sure we are still above the normal.
			//For high guess
			//  Adjust 100% network and 0% what we see.
			if (n < 20)
			{
				if ( count.GetAt(i) < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountLo = m_nCompleteSourcesCount;
				else
					m_nCompleteSourcesCountLo = count.GetAt(i);
				m_nCompleteSourcesCount= m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= count.GetAt(j);
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
			}
			else
			//Many sources..
			//For low guess
			//	Use what we see.
			//For normal guess
			//	Adjust network accounts for 100%, we account for 0% with what we see and make sure we are still above the low.
			//For high guess
			//  Adjust network accounts for 100%, we account for 0% with what we see and make sure we are still above the normal.
			{
				m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
				m_nCompleteSourcesCount= count.GetAt(j);
				if( m_nCompleteSourcesCount < m_nCompleteSourcesCountLo )
					m_nCompleteSourcesCount = m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= count.GetAt(k);
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
			}
		}
		m_nCompleteSourcesTime = time(NULL) + (60);
		m_nCompleteSourcesSize = m_ClientUploadList.GetSize(); // NEO: MOD - [RelativeChunkDisplay] <-- Xanatos --
	}

	CalcRelease(flag); // NEO: SRS - [SmartReleaseSharing] <-- Xanatos --

	if (theApp.emuledlg->sharedfileswnd->m_hWnd)
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
}

// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
void CKnownFile::UpdatePartsInfoEx(EPartStatus type)
{
	CArray<uint16, uint16>* PartFrequency;
	switch(type)
	{
	case CFS_Incomplete: PartFrequency = &m_AvailIncPartFrequency; break; // NEO: ICS - [InteligentChunkSelection]
	//case CFS_Hiden: PartFrequency = &m_AvailHidenPartFrequency; break; // NEO: RPS - [RealPartStatus]
	//case CFS_Blocked: PartFrequency = &m_AvailBlockedPartFrequency; break; // NEO: RPS - [RealPartStatus]
	default:
		ASSERT(0);
		return;
	}

	// Cache part count
	UINT partcount = GetPartCount();

	// Reset part counters
	if ((UINT)PartFrequency->GetSize() < partcount)
		PartFrequency->SetSize(partcount);
	for(UINT i = 0; i < partcount; i++)
		PartFrequency->GetAt(i) = 0;

	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != NULL;){
		CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		CClientFileStatus* status = cur_src->GetFileStatus(this);
		if(status == NULL || status->GetPartCount() != partcount)
			continue;
		uint8* abyUpPartStatus = status->GetPartStatus(type);
		if(abyUpPartStatus == NULL)
			continue;
		for (UINT i = 0; i < partcount; i++){
			if (abyUpPartStatus[i])
				PartFrequency->GetAt(i)+=1;
		}
	}

	//if (theApp.emuledlg->sharedfileswnd->m_hWnd)
	//	theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this); // Not displayed
}
// NEO: SCFS END <-- Xanatos --

void CKnownFile::AddUploadingClient(CUpDownClient* client){
	POSITION pos = m_ClientUploadList.Find(client); // to be sure
	if(pos == NULL){
		m_ClientUploadList.AddTail(client);
		UpdateAutoUpPriority();
	}
}

void CKnownFile::RemoveUploadingClient(CUpDownClient* client){
	POSITION pos = m_ClientUploadList.Find(client); // to be sure
	if(pos != NULL){
		m_ClientUploadList.RemoveAt(pos);
		UpdateAutoUpPriority();
	}
}

#ifdef _DEBUG
void Dump(const Kademlia::WordList& wordlist)
{
	Kademlia::WordList::const_iterator it;
	for (it = wordlist.begin(); it != wordlist.end(); it++)
	{
		const CStringW& rstrKeyword = *it;
		TRACE("  %ls\n", rstrKeyword);
	}
}
#endif

// NEO: PP - [PasswordProtection] -- Xanatos -->
const CString& CKnownFile::GetFileName(bool forceReal) const
{
	static CString Unknown = _T("000.part.met");
	if(!forceReal && IsPWProtHidden()){
		if(IsPartFile())
			return ((CPartFile*)this)->GetPartMetFileName();
		else
			return Unknown;
	}
	
	return m_strFileName; 
} 
// NEO: PP END <-- Xanatos --

void CKnownFile::SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars, bool bRemoveControlChars)
{ 
	CKnownFile* pFile = NULL;

	// If this is called within the sharedfiles object during startup,
	// we cannot reference it yet..

	if(theApp.sharedfiles)
		pFile = theApp.sharedfiles->GetFileByID(GetFileHash());

	if (pFile && pFile == this)
		theApp.sharedfiles->RemoveKeywords(this);

	CAbstractFile::SetFileName(pszFileName, bReplaceInvalidFileSystemChars, true, bRemoveControlChars);

	wordlist.clear();
	if(m_pCollection)
	{
		CString sKeyWords;
		//sKeyWords.Format(_T("%s %s"), m_pCollection->GetCollectionAuthorKeyString(), GetFileName());
		sKeyWords.Format(_T("%s %s"), m_pCollection->GetCollectionAuthorKeyString(), GetFileName(true)); // NEO: PP - [PasswordProtection] <-- Xanatos --
		Kademlia::CSearchManager::GetWords(sKeyWords, &wordlist);
	}
	else
		//Kademlia::CSearchManager::GetWords(GetFileName(), &wordlist);
		Kademlia::CSearchManager::GetWords(GetFileName(true), &wordlist); // NEO: PP - [PasswordProtection] <-- Xanatos --

	if (pFile && pFile == this)
		theApp.sharedfiles->AddKeywords(this);
} 

// NEO: VSF - [VirtualSharedFiles] -- Xanatos -->
// If returnVirtual, the searching for virtual path is done as follows:
// get the first existing entry from the following:
// - a virtual dir for a specific file
// - a virtual dir for the file's directory
// - while (parent directory exists)
//   - a virtual subdir-dir for the parent directory
// - incoming psuedo-directory for part files
// - physical directory
CString CKnownFile::GetPath(bool returnVirtual) const
{
	if (returnVirtual) {
		CString thisDir = m_strDirectory, virtDir, fileID;
		thisDir.MakeLower();
		thisDir.TrimRight(_T("\\"));
		fileID.Format(_T("%I64u:%s"), (uint64)GetFileSize(), EncodeBase16(GetFileHash(),16));
		if (thePrefs.m_fileToVDir_map.Lookup(fileID, virtDir))
			return virtDir; // file-virt dir
		if (thePrefs.m_dirToVDir_map.Lookup(thisDir, virtDir))
			return virtDir; // dir-virt dir
		int pos = thisDir.GetLength();
		while (pos != -1) { // subdir-virt dir
			if (thePrefs.m_dirToVDirWithSD_map.Lookup(thisDir.Left(pos), virtDir))
				return virtDir + thisDir.Right(thisDir.GetLength()-pos);
			pos = thisDir.Left(pos).ReverseFind(_T('\\'));
		}
		//if (IsPartFile()) // NEO: MTD - [MultiTempDirectories]
		//	return _T(OP_INCOMPLETE_SHARED_FILES);
	}
	return m_strDirectory;
}
// NEO: VSF END <-- Xanatos --

void CKnownFile::SetPath(LPCTSTR path)
{
	m_strDirectory = path;
}

void CKnownFile::SetFilePath(LPCTSTR pszFilePath)
{
	m_strFilePath = pszFilePath;
}

bool CKnownFile::CreateFromFile(LPCTSTR in_directory, LPCTSTR in_filename, LPVOID pvProgressParam)
{
	SetPath(in_directory);
	SetFileName(in_filename);

	// open file
	CString strFilePath;
	if (!_tmakepathlimit(strFilePath.GetBuffer(MAX_PATH), NULL, in_directory, in_filename, NULL)){
		LogError(GetResString(IDS_ERR_FILEOPEN), in_filename, _T(""));
		return false;
	}
	strFilePath.ReleaseBuffer();
	SetFilePath(strFilePath);
	FILE* file = _tfsopen(strFilePath, _T("rbS"), _SH_DENYNO); // can not use _SH_DENYWR because we may access a completing part file
	if (!file){
		LogError(GetResString(IDS_ERR_FILEOPEN) + _T(" - %s"), strFilePath, _T(""), _tcserror(errno));
		return false;
	}

	// set filesize
	if (_filelengthi64(file->_file) > MAX_EMULE_FILE_SIZE){
		fclose(file);
		return false; // not supported by network
	}
	SetFileSize((uint64)_filelengthi64(file->_file));

	// we are reading the file data later in 8K blocks, adjust the internal file stream buffer accordingly
	setvbuf(file, NULL, _IOFBF, 1024*8*2);

	m_AvailPartFrequency.SetSize(GetPartCount());
	m_AvailIncPartFrequency.SetSize(GetPartCount()); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	for (UINT i = 0; i < GetPartCount();i++){
		m_AvailPartFrequency[i] = 0;
		m_AvailIncPartFrequency[i] = 0; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	}
	
	// create hashset
	uint64 togo = m_nFileSize;
	UINT hashcount;
	for (hashcount = 0; togo >= PARTSIZE; )
	{
		CAICHHashTree* pBlockAICHHashTree = m_pAICHHashSet->m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, PARTSIZE);
		ASSERT( pBlockAICHHashTree != NULL );

		uchar* newhash = new uchar[16];
		if (!CreateHash(file, PARTSIZE, newhash, pBlockAICHHashTree)) {
			LogError(_T("Failed to hash file \"%s\" - %s"), strFilePath, _tcserror(errno));
			fclose(file);
			delete[] newhash;
			return false;
		}

		if (theApp.emuledlg==NULL || !theApp.emuledlg->IsRunning()){ // in case of shutdown while still hashing
			fclose(file);
			delete[] newhash;
			return false;
		}

		hashlist.Add(newhash);
		togo -= PARTSIZE;
		hashcount++;

		//if (pvProgressParam && theApp.emuledlg && theApp.emuledlg->IsRunning()){
		if (theApp.emuledlg && theApp.emuledlg->IsRunning()){ // NEO: MOD - [HashProgress] <-- Xanatos --
			//ASSERT( ((CKnownFile*)pvProgressParam)->IsKindOf(RUNTIME_CLASS(CKnownFile)) );
			//ASSERT( ((CKnownFile*)pvProgressParam)->GetFileSize() == GetFileSize() );
			UINT uProgress = (UINT)(uint64)(((uint64)(GetFileSize() - togo) * 100) / GetFileSize());
			ASSERT( uProgress <= 100 );
			VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pvProgressParam) );
		}
	}

	CAICHHashTree* pBlockAICHHashTree;
	if (togo == 0)
		pBlockAICHHashTree = NULL; // sha hashtree doesnt takes hash of 0-sized data
	else{
		pBlockAICHHashTree = m_pAICHHashSet->m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, togo);
		ASSERT( pBlockAICHHashTree != NULL );
	}
	
	uchar* lasthash = new uchar[16];
	md4clr(lasthash);
	if (!CreateHash(file, togo, lasthash, pBlockAICHHashTree)) {
		LogError(_T("Failed to hash file \"%s\" - %s"), strFilePath, _tcserror(errno));
		fclose(file);
		delete[] lasthash;
		return false;
	}
	
	m_pAICHHashSet->ReCalculateHash(false);
	if (m_pAICHHashSet->VerifyHashTree(true))
	{
		m_pAICHHashSet->SetStatus(AICH_HASHSETCOMPLETE);
		if (!m_pAICHHashSet->SaveHashSet())
			LogError(LOG_STATUSBAR, GetResString(IDS_SAVEACFAILED));
	}
	else{
		// now something went pretty wrong
		DebugLogError(LOG_STATUSBAR, _T("Failed to calculate AICH Hashset from file %s"), GetFileName());
	}

	if (!hashcount){
		md4cpy(m_abyFileHash, lasthash);
		delete[] lasthash;
	} 
	else {
		hashlist.Add(lasthash);
		uchar* buffer = new uchar[hashlist.GetCount()*16];
		for (int i = 0; i < hashlist.GetCount(); i++)
			md4cpy(buffer+(i*16), hashlist[i]);
		CreateHash(buffer, hashlist.GetCount()*16, m_abyFileHash);
		delete[] buffer;
	}

	//if (pvProgressParam && theApp.emuledlg && theApp.emuledlg->IsRunning()){
	if (theApp.emuledlg && theApp.emuledlg->IsRunning()){ // NEO: MOD - [HashProgress] <-- Xanatos --
		//ASSERT( ((CKnownFile*)pvProgressParam)->IsKindOf(RUNTIME_CLASS(CKnownFile)) );
		//ASSERT( ((CKnownFile*)pvProgressParam)->GetFileSize() == GetFileSize() );
		UINT uProgress = 100;
		ASSERT( uProgress <= 100 );
		VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pvProgressParam) );
	}

	// set lastwrite date
	struct _stat fileinfo;
	if (_fstat(file->_file, &fileinfo) == 0){
		m_tUtcLastModified = fileinfo.st_mtime;
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strFilePath);
	}

	fclose(file);
	file = NULL;

	// Add filetags
	UpdateMetaDataTags();

	UpdatePartsInfo();

	return true;	
}

bool CKnownFile::CreateAICHHashSetOnly()
{
	ASSERT( !IsPartFile() );
	m_pAICHHashSet->FreeHashSet();
	FILE* file = _tfsopen(GetFilePath(), _T("rbS"), _SH_DENYNO); // can not use _SH_DENYWR because we may access a completing part file
	if (!file){
		LogError(GetResString(IDS_ERR_FILEOPEN) + _T(" - %s"), GetFilePath(), _T(""), _tcserror(errno));
		return false;
	}
	// we are reading the file data later in 8K blocks, adjust the internal file stream buffer accordingly
	setvbuf(file, NULL, _IOFBF, 1024*8*2);

	// create aichhashset
	uint64 togo = m_nFileSize;
	UINT hashcount;
	for (hashcount = 0; togo >= PARTSIZE; )
	{
		CAICHHashTree* pBlockAICHHashTree = m_pAICHHashSet->m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, PARTSIZE);
		ASSERT( pBlockAICHHashTree != NULL );
		if (!CreateHash(file, PARTSIZE, NULL, pBlockAICHHashTree)) {
			LogError(_T("Failed to hash file \"%s\" - %s"), GetFilePath(), _tcserror(errno));
			fclose(file);
			return false;
		}

		if (theApp.emuledlg==NULL || !theApp.emuledlg->IsRunning()){ // in case of shutdown while still hashing
			fclose(file);
			return false;
		}

		togo -= PARTSIZE;
		hashcount++;
	}

	if (togo != 0)
	{
		CAICHHashTree* pBlockAICHHashTree = m_pAICHHashSet->m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, togo);
		ASSERT( pBlockAICHHashTree != NULL );
		if (!CreateHash(file, togo, NULL, pBlockAICHHashTree)) {
			LogError(_T("Failed to hash file \"%s\" - %s"), GetFilePath(), _tcserror(errno));
			fclose(file);
			return false;
		}
	}

	m_pAICHHashSet->ReCalculateHash(false);
	if (m_pAICHHashSet->VerifyHashTree(true))
	{
		m_pAICHHashSet->SetStatus(AICH_HASHSETCOMPLETE);
		if (!m_pAICHHashSet->SaveHashSet())
			LogError(LOG_STATUSBAR, GetResString(IDS_SAVEACFAILED));
	}
	else{
		// now something went pretty wrong
		DebugLogError(LOG_STATUSBAR, _T("Failed to calculate AICH Hashset from file %s"), GetFileName());
	}

	fclose(file);
	file = NULL;

	return true;	
}

void CKnownFile::SetFileSize(EMFileSize nFileSize)
{
	CAbstractFile::SetFileSize(nFileSize);
	m_pAICHHashSet->SetFileSize(nFileSize);

	// Examples of parthashs, hashsets and filehashs for different filesizes
	// according the ed2k protocol
	//----------------------------------------------------------------------
	//
	//File size: 3 bytes
	//File hash: 2D55E87D0E21F49B9AD25F98531F3724
	//Nr. hashs: 0
	//
	//
	//File size: 1*PARTSIZE
	//File hash: A72CA8DF7F07154E217C236C89C17619
	//Nr. hashs: 2
	//Hash[  0]: 4891ED2E5C9C49F442145A3A5F608299
	//Hash[  1]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 1*PARTSIZE + 1 byte
	//File hash: 2F620AE9D462CBB6A59FE8401D2B3D23
	//Nr. hashs: 2
	//Hash[  0]: 121795F0BEDE02DDC7C5426D0995F53F
	//Hash[  1]: C329E527945B8FE75B3C5E8826755747
	//
	//
	//File size: 2*PARTSIZE
	//File hash: A54C5E562D5E03CA7D77961EB9A745A4
	//Nr. hashs: 3
	//Hash[  0]: B3F5CE2A06BF403BFB9BFFF68BDDC4D9
	//Hash[  1]: 509AA30C9EA8FC136B1159DF2F35B8A9
	//Hash[  2]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 3*PARTSIZE
	//File hash: 5E249B96F9A46A18FC2489B005BF2667
	//Nr. hashs: 4
	//Hash[  0]: 5319896A2ECAD43BF17E2E3575278E72
	//Hash[  1]: D86EF157D5E49C5ED502EDC15BB5F82B
	//Hash[  2]: 10F2D5B1FCB95C0840519C58D708480F
	//Hash[  3]: 31D6CFE0D16AE931B73C59D7E0C089C0	*special part hash*
	//
	//
	//File size: 3*PARTSIZE + 1 byte
	//File hash: 797ED552F34380CAFF8C958207E40355
	//Nr. hashs: 4
	//Hash[  0]: FC7FD02CCD6987DCF1421F4C0AF94FB8
	//Hash[  1]: 2FE466AF8A7C06DA3365317B75A5ACFE
	//Hash[  2]: 873D3BF52629F7C1527C6E8E473C1C30
	//Hash[  3]: BCE50BEE7877BB07BB6FDA56BFE142FB
	//

	// File size       Data parts      ED2K parts      ED2K part hashs
	// ---------------------------------------------------------------
	// 1..PARTSIZE-1   1               1               0(!)
	// PARTSIZE        1               2(!)            2(!)
	// PARTSIZE+1      2               2               2
	// PARTSIZE*2      2               3(!)            3(!)
	// PARTSIZE*2+1    3               3               3

	if (nFileSize == (uint64)0){
		ASSERT(0);
		m_iPartCount = 0;
		m_iED2KPartCount = 0;
		m_iED2KPartHashCount = 0;
		return;
	}

	// nr. of data parts
	ASSERT( (uint64)(((uint64)nFileSize + (PARTSIZE - 1)) / PARTSIZE) <= (UINT)USHRT_MAX );
	m_iPartCount = (uint16)(((uint64)nFileSize + (PARTSIZE - 1)) / PARTSIZE);

	// nr. of parts to be used with OP_FILESTATUS
	m_iED2KPartCount = (uint16)((uint64)nFileSize / PARTSIZE + 1);

	// nr. of parts to be used with OP_HASHSETANSWER
	m_iED2KPartHashCount = (uint16)((uint64)nFileSize / PARTSIZE);
	if (m_iED2KPartHashCount != 0)
		m_iED2KPartHashCount += 1;
}

// needed for memfiles. its probably better to switch everything to CFile...
bool CKnownFile::LoadHashsetFromFile(CFileDataIO* file, bool checkhash){
	uchar checkid[16];
	file->ReadHash16(checkid);
	//TRACE("File size: %u (%u full parts + %u bytes)\n", GetFileSize(), GetFileSize()/PARTSIZE, GetFileSize()%PARTSIZE);
	//TRACE("File hash: %s\n", md4str(checkid));
	ASSERT( hashlist.GetCount() == 0 );
	UINT parts = file->ReadUInt16();
	//TRACE("Nr. hashs: %u\n", (UINT)parts);
	for (UINT i = 0; i < parts; i++){
		uchar* cur_hash = new uchar[16];
		file->ReadHash16(cur_hash);
		//TRACE("Hash[%3u]: %s\n", i, md4str(cur_hash));
		hashlist.Add(cur_hash);
	}

	// SLUGFILLER: SafeHash - always check for valid hashlist
	if (!checkhash){
		md4cpy(m_abyFileHash, checkid);
		if (parts <= 1)	// nothing to check
			return true;
	}
	else if (md4cmp(m_abyFileHash, checkid)){
		// delete hashset
		for (int i = 0; i < hashlist.GetSize(); i++)
			delete[] hashlist[i];
		hashlist.RemoveAll();
		return false;	// wrong file?
	}
	else{
		if (parts != GetED2KPartHashCount()){
			// delete hashset
			for (int i = 0; i < hashlist.GetSize(); i++)
				delete[] hashlist[i];
			hashlist.RemoveAll();
			return false;
		}
	}
	// SLUGFILLER: SafeHash

	if (!hashlist.IsEmpty()){
		uchar* buffer = new uchar[hashlist.GetCount()*16];
		for (int i = 0; i < hashlist.GetCount(); i++)
			md4cpy(buffer+(i*16), hashlist[i]);
		CreateHash(buffer, hashlist.GetCount()*16, checkid);
		delete[] buffer;
	}
	if (!md4cmp(m_abyFileHash, checkid))
		return true;
	else{
		// delete hashset
		for (int i = 0; i < hashlist.GetSize(); i++)
			delete[] hashlist[i];
		hashlist.RemoveAll();
		return false;
	}
}

bool CKnownFile::SetHashset(const CArray<uchar*, uchar*>& aHashset)
{
	// delete hashset
	for (int i = 0; i < hashlist.GetSize(); i++)
		delete[] hashlist[i];
	hashlist.RemoveAll();

	// set new hash
	for (int i = 0; i < aHashset.GetSize(); i++)
	{
		uchar* pucHash = new uchar[16];
		md4cpy(pucHash, aHashset.GetAt(i));
		hashlist.Add(pucHash);
	}

	// verify new hash
	if (hashlist.IsEmpty())
		return true;

	uchar aucHashsetHash[16];
	uchar* buffer = new uchar[hashlist.GetCount()*16];
	for (int i = 0; i < hashlist.GetCount(); i++)
		md4cpy(buffer+(i*16), hashlist[i]);
	CreateHash(buffer, hashlist.GetCount()*16, aucHashsetHash);
	delete[] buffer;

	bool bResult = (md4cmp(aucHashsetHash, m_abyFileHash) == 0);
	if (!bResult)
	{
		// delete hashset
		for (int i = 0; i < hashlist.GetSize(); i++)
			delete[] hashlist[i];
		hashlist.RemoveAll();
	}
	return bResult;
}
 
bool CKnownFile::LoadTagsFromFile(CFileDataIO* file)
{
	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
		switch (newtag->GetNameID()){
			case FT_FILENAME:{
				ASSERT( newtag->IsStr() );
				if (newtag->IsStr()){
					//if (GetFileName().IsEmpty())
					if (GetFileName(true).IsEmpty()) // NEO: PP - [PasswordProtection] <-- Xanatos --
						SetFileName(newtag->GetStr());
				}
				delete newtag;
				break;
			}
			case FT_FILESIZE:{
				ASSERT( newtag->IsInt64(true) );
				if (newtag->IsInt64(true))
				{
					SetFileSize(newtag->GetInt64());
					m_AvailPartFrequency.SetSize(GetPartCount());
					m_AvailIncPartFrequency.SetSize(GetPartCount()); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
					for (UINT i = 0; i < GetPartCount();i++){
						m_AvailPartFrequency[i] = 0;
						m_AvailIncPartFrequency[i] = 0; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
					}
				}
				delete newtag;
				break;
			}
			case FT_ATTRANSFERRED:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.alltimetransferred = newtag->GetInt();
				delete newtag;
				break;
			}
			case FT_ATTRANSFERREDHI:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.alltimetransferred = ((uint64)newtag->GetInt() << 32) | (UINT)statistic.alltimetransferred;
				delete newtag;
				break;
			}
			case FT_ATREQUESTED:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.alltimerequested = newtag->GetInt();
				delete newtag;
				break;
			}
 			case FT_ATACCEPTED:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.alltimeaccepted = newtag->GetInt();
				delete newtag;
				break;
			}
			case FT_ULPRIORITY:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
				{
					m_iUpPriority = (uint8)newtag->GetInt();
					if( m_iUpPriority == PR_AUTO ){
						m_iUpPriority = PR_HIGH;
						m_bAutoUpPriority = true;
					}
					else{
						if (m_iUpPriority != PR_VERYLOW && m_iUpPriority != PR_LOW && m_iUpPriority != PR_NORMAL && m_iUpPriority != PR_HIGH && m_iUpPriority != PR_VERYHIGH)
							m_iUpPriority = PR_NORMAL;
						m_bAutoUpPriority = false;
					}
				}
				delete newtag;
				break;
			}
			// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
			case FT_RELEASE:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					m_uReleasePriority = (uint8)newtag->GetInt();
				delete newtag;
				break;
			}
			// NEO: SRS END <-- Xanatos --
			case FT_KADLASTPUBLISHSRC:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					SetLastPublishTimeKadSrc( newtag->GetInt(), 0 );
				if(GetLastPublishTimeKadSrc() > (uint32)time(NULL)+KADEMLIAREPUBLISHTIMES)
				{
					//There may be a posibility of an older client that saved a random number here.. This will check for that..
					SetLastPublishTimeKadSrc(0,0);
				}
				delete newtag;
				break;
			}
			case FT_KADLASTPUBLISHNOTES:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					SetLastPublishTimeKadNotes( newtag->GetInt() );
				delete newtag;
				break;
			}
			case FT_FLAGS:
				// Misc. Flags
				// ------------------------------------------------------------------------------
				// Bits  3-0: Meta data version
				//				0 = Unknown
				//				1 = we have created that meta data by examining the file contents.
				// Bits 31-4: Reserved
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					m_uMetaDataVer = newtag->GetInt() & 0x0F;
				delete newtag;
				break;
			// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
		    case FT_CATEGORY:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					SetCategory(newtag->GetInt(),2); // NEO: MOD - [SetCategory]
					//m_category = newtag->GetInt();
			    delete newtag;
			    break;
		    }
			// NEO: NSC END <-- Xanatos --
			// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
			case FT_PERMISSIONS:
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt()){
					m_iPermissions = (uint8)newtag->GetInt();
					if (m_iPermissions != PERM_ALL && m_iPermissions != PERM_FRIENDS && m_iPermissions != PERM_NONE && m_iPermissions != PERM_DEFAULT)
						m_iPermissions = PR_NORMAL;
				}
				delete newtag;
				break;
			// NEO: SSP END <-- Xanatos --
			// old tags: as long as they are not needed, take the chance to purge them
			//case FT_PERMISSIONS:
			//	ASSERT( newtag->IsInt() );
			//	delete newtag;
			//	break;
			case FT_KADLASTPUBLISHKEY:
				ASSERT( newtag->IsInt() );
				delete newtag;
				break;
			case FT_AICH_HASH:{
				if(!newtag->IsStr()){
					//ASSERT( false ); uncomment later
					break;
				}
				CAICHHash hash;
				if (DecodeBase32(newtag->GetStr(),hash) == (UINT)CAICHHash::GetHashSize())
					m_pAICHHashSet->SetMasterHash(hash, AICH_HASHSETCOMPLETE);
				else
					ASSERT( false );
				delete newtag;
				break;
			}
			// NEO: MQ - [MultiQueue] // NEO: SQ - [SaveUploadQueue] -- Xanatos -->
		    case FT_STARTUPLOADTIME:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt()){
					m_startUploadTime = time(NULL) - newtag->GetInt();
				}
			    delete newtag;
			    break;
		    }
			// NEO: MQ END // NEO: SQ END <-- Xanatos --
			default:
				ConvertED2KTag(newtag);
				if (newtag)
					taglist.Add(newtag);
		}
	}

	// 05-Jän-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
	// the chance to clean any available meta data tags and provide only tags which were determined by us.
	// It's a brute force method, but that wrong meta data is driving me crazy because wrong meta data is even worse than
	// missing meta data.
	if (m_uMetaDataVer == 0)
		RemoveMetaDataTags();

	return m_nFileSize!=(uint64)0;	// SLUGFILLER: SafeHash - Must have a filesize tag // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
}

bool CKnownFile::LoadDateFromFile(CFileDataIO* file){
	m_tUtcLastModified = file->ReadUInt32();
	return true;
}

bool CKnownFile::LoadFromFile(CFileDataIO* file){
	// SLUGFILLER: SafeHash - load first, verify later
	bool ret1 = LoadDateFromFile(file);
	bool ret2 = LoadHashsetFromFile(file,false);
	bool ret3 = LoadTagsFromFile(file);
	UpdatePartsInfo();
	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->	
	// David: Note the current implementation of SlugFiller's SafeHash is causing an incompatybility for < 1 chunk files, 
	// this part will return us the compatybility
	if (GetED2KPartHashCount() == 0 && hashlist.GetSize() != 0) {
		ASSERT(hashlist.GetSize() == 1);
		for (int i = 0; i < hashlist.GetSize(); i++)
			delete[] hashlist[i];
		hashlist.RemoveAll();
	} 
	// NEO: SSH END <-- Xanatos --
	return ret1 && ret2 && ret3 && GetED2KPartHashCount()==GetHashCount();// Final hash-count verification, needs to be done after the tags are loaded.
	// SLUGFILLER: SafeHash
}

bool CKnownFile::WriteToFile(CFileDataIO* file)
{
	// date
	file->WriteUInt32(m_tUtcLastModified);

	// hashset
	file->WriteHash16(m_abyFileHash);
	UINT parts = hashlist.GetCount();
	file->WriteUInt16((uint16)parts);
	for (UINT i = 0; i < parts; i++)
		file->WriteHash16(hashlist[i]);

	uint32 uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

	//if (WriteOptED2KUTF8Tag(file, GetFileName(), FT_FILENAME))
	if (WriteOptED2KUTF8Tag(file, GetFileName(true), FT_FILENAME)) // NEO: PP - [PasswordProtection] <-- Xanatos --
		uTagCount++;
	//CTag nametag(FT_FILENAME, GetFileName());
	CTag nametag(FT_FILENAME, GetFileName(true)); // NEO: PP - [PasswordProtection] <-- Xanatos --
	nametag.WriteTagToFile(file);
	uTagCount++;
	
	CTag sizetag(FT_FILESIZE, m_nFileSize, IsLargeFile());
	sizetag.WriteTagToFile(file);
	uTagCount++;
	
	// statistic
	if (statistic.alltimetransferred){
		CTag attag1(FT_ATTRANSFERRED, (uint32)statistic.alltimetransferred);
		attag1.WriteTagToFile(file);
		uTagCount++;
		
		CTag attag4(FT_ATTRANSFERREDHI, (uint32)(statistic.alltimetransferred >> 32));
		attag4.WriteTagToFile(file);
		uTagCount++;
	}

	if (statistic.GetAllTimeRequests()){
		CTag attag2(FT_ATREQUESTED, statistic.GetAllTimeRequests());
		attag2.WriteTagToFile(file);
		uTagCount++;
	}
	
	if (statistic.GetAllTimeAccepts()){
		CTag attag3(FT_ATACCEPTED, statistic.GetAllTimeAccepts());
		attag3.WriteTagToFile(file);
		uTagCount++;
	}

	// priority N permission
	CTag priotag(FT_ULPRIORITY, IsAutoUpPriority() ? PR_AUTO : m_iUpPriority);
	priotag.WriteTagToFile(file);
	uTagCount++;
	
	// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
	CTag releasetag(FT_RELEASE, m_uReleasePriority);
	releasetag.WriteTagToFile(file);
	uTagCount++;
	// NEO: SRS END <-- Xanatos --

	//AICH Filehash
	if (m_pAICHHashSet->HasValidMasterHash() && (m_pAICHHashSet->GetStatus() == AICH_HASHSETCOMPLETE || m_pAICHHashSet->GetStatus() == AICH_VERIFIED)){
		CTag aichtag(FT_AICH_HASH, m_pAICHHashSet->GetMasterHash().GetString());
		aichtag.WriteTagToFile(file);
		uTagCount++;
	}


	if (m_lastPublishTimeKadSrc){
		CTag kadLastPubSrc(FT_KADLASTPUBLISHSRC, m_lastPublishTimeKadSrc);
		kadLastPubSrc.WriteTagToFile(file);
		uTagCount++;
	}

	if (m_lastPublishTimeKadNotes){
		CTag kadLastPubNotes(FT_KADLASTPUBLISHNOTES, m_lastPublishTimeKadNotes);
		kadLastPubNotes.WriteTagToFile(file);
		uTagCount++;
	}

	if (m_uMetaDataVer > 0)
	{
		// Misc. Flags
		// ------------------------------------------------------------------------------
		// Bits  3-0: Meta data version
		//				0 = Unknown
		//				1 = we have created that meta data by examining the file contents.
		// Bits 31-4: Reserved
		ASSERT( m_uMetaDataVer <= 0x0F );
		uint32 uFlags = m_uMetaDataVer & 0x0F;
		CTag tagFlags(FT_FLAGS, uFlags);
		tagFlags.WriteTagToFile(file);
		uTagCount++;
	}

	// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
	if (m_category){
		CTag categorytag(FT_CATEGORY, m_category);
		categorytag.WriteTagToFile(file);
		uTagCount++;
	}
	// NEO: NSC END <-- Xanatos --

	// NEO: SSP - [ShowSharePermissions] -- Xanatos -->
	CTag permtag(FT_PERMISSIONS, m_iPermissions);
	permtag.WriteTagToFile(file);
	uTagCount++;
	// NEO: SSP END <-- Xanatos --

	// NEO: MQ - [MultiQueue] // NEO: SQ - [SaveUploadQueue] -- Xanatos -->
	if(NeoPrefs.SaveUploadQueueWaitTime()){
		CTag lsctag(FT_STARTUPLOADTIME, time(NULL) - m_startUploadTime);
		lsctag.WriteTagToFile(file);
		uTagCount++;
	}
	// NEO: MQ END // NEO: SQ END <-- Xanatos --

	//other tags
	for (int j = 0; j < taglist.GetCount(); j++){
		if (taglist[j]->IsStr() || taglist[j]->IsInt()){
			taglist[j]->WriteTagToFile(file);
			uTagCount++;
		}
	}

	file->Seek(uTagCountFilePos, CFile::begin);
	file->WriteUInt32(uTagCount);
	file->Seek(0, CFile::end);

	return true;
}

void CKnownFile::CreateHash(CFile* pFile, uint64 Length, uchar* pMd4HashOut, CAICHHashTree* pShaHashOut) const
{
	ASSERT( pFile != NULL );
	ASSERT( pMd4HashOut != NULL || pShaHashOut != NULL );

	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	CSingleLock sLock(&theApp.hashing_mut);	// SLUGFILLER: SafeHash - only one chunk-hash at a time 
	if(theApp.emuledlg->IsMainThread() == false) // make sure only secundary threads lock the hashing_mut
		sLock.Lock();
	// NEO: SSH END <-- Xanatos --

	uint64  Required = Length;
	uchar   X[64*128];
	uint64	posCurrentEMBlock = 0;
	uint64	nIACHPos = 0;
	CAICHHashAlgo* pHashAlg = m_pAICHHashSet->GetNewHashAlgo();
	CMD4 md4;

	while (Required >= 64){
        uint32 len; 
        if ((Required / 64) > sizeof(X)/(64 * sizeof(X[0]))) 
			len = sizeof(X)/(64 * sizeof(X[0]));
		else
			len = (uint32)Required / 64;
		pFile->Read(X, len*64);

		// SHA hash needs 180KB blocks
		if (pShaHashOut != NULL){
			if (nIACHPos + len*64 >= EMBLOCKSIZE){
				uint32 nToComplete = (uint32)(EMBLOCKSIZE - nIACHPos);
				pHashAlg->Add(X, nToComplete);
				ASSERT( nIACHPos + nToComplete == EMBLOCKSIZE );
				pShaHashOut->SetBlockHash(EMBLOCKSIZE, posCurrentEMBlock, pHashAlg);
				posCurrentEMBlock += EMBLOCKSIZE;
				pHashAlg->Reset();
				pHashAlg->Add(X+nToComplete,(len*64) - nToComplete);
				nIACHPos = (len*64) - nToComplete;
			}
			else{
				pHashAlg->Add(X, len*64);
				nIACHPos += len*64;
			}
		}

		if (pMd4HashOut != NULL){
			md4.Add(X, len*64);
		}
		Required -= len*64;
	}

	Required = Length % 64;
	if (Required != 0){
		pFile->Read(X, (uint32)Required);

		if (pShaHashOut != NULL){
			if (nIACHPos + Required >= EMBLOCKSIZE){
				uint32 nToComplete = (uint32)(EMBLOCKSIZE - nIACHPos);
				pHashAlg->Add(X, nToComplete);
				ASSERT( nIACHPos + nToComplete == EMBLOCKSIZE );
				pShaHashOut->SetBlockHash(EMBLOCKSIZE, posCurrentEMBlock, pHashAlg);
				posCurrentEMBlock += EMBLOCKSIZE;
				pHashAlg->Reset();
				pHashAlg->Add(X+nToComplete, (uint32)(Required - nToComplete));
				nIACHPos = Required - nToComplete;
			}
			else{
				pHashAlg->Add(X, (uint32)Required);
				nIACHPos += Required;
			}
		}
	}
	if (pShaHashOut != NULL){
		if(nIACHPos > 0){
			pShaHashOut->SetBlockHash(nIACHPos, posCurrentEMBlock, pHashAlg);
			posCurrentEMBlock += nIACHPos;
		}
		ASSERT( posCurrentEMBlock == Length );
		VERIFY( pShaHashOut->ReCalculateHash(pHashAlg, false) );
	}

	if (pMd4HashOut != NULL){
		md4.Add(X, (uint32)Required);
		md4.Finish();
		md4cpy(pMd4HashOut, md4.GetHash());
	}

	delete pHashAlg;
}

bool CKnownFile::CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut) const
{
	bool bResult = false;
	CStdioFile file(fp);
	try
	{
		CreateHash(&file, uSize, pucHash, pShaHashOut);
		bResult = true;
	}
	catch(CFileException* ex)
	{
		ex->Delete();
	}
	return bResult;
}

bool CKnownFile::CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut) const
{
	bool bResult = false;
	CMemFile file(const_cast<uchar*>(pucData), uSize);
	try
	{
		CreateHash(&file, uSize, pucHash, pShaHashOut);
		bResult = true;
	}
	catch(CFileException* ex)
	{
		ex->Delete();
	}
	return bResult;
}

uchar* CKnownFile::GetPartHash(UINT part) const
{
	if (part >= (UINT)hashlist.GetCount())
	// NEO: SSH - [SlugFillerSafeHash] -- Xanatos -->
	{	// SLUGFILLER: SafeHash
		if(GetED2KPartHashCount() == 0 || GetHashCount() == 0)
			return const_cast<uchar*>(m_abyFileHash);	
		return NULL;
	}
	// NEO: SSH END <-- Xanatos --
	//return NULL;
	return hashlist[part];
}

Packet*	CKnownFile::CreateSrcInfoPacket(const CUpDownClient* forClient, uint8 byRequestedVersion, uint16 nRequestedOptions) const
{
	if (m_ClientUploadList.IsEmpty())
		return NULL;

	if (md4cmp(forClient->GetUploadFileID(), GetFileHash())!=0) {
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - client (%s) upload file \"%s\" does not match file \"%s\""), __FUNCTION__, forClient->DbgGetClientInfo(), DbgGetFileInfo(forClient->GetUploadFileID()), GetFileName()) );
		ASSERT(0);
		return NULL;
	}

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	// check whether client has either no download status at all or a download status which is valid for this file
	/*if (   !(forClient->GetUpPartCount()==0 && forClient->GetUpPartStatus()==NULL)
		&& !(forClient->GetUpPartCount()==GetPartCount() && forClient->GetUpPartStatus()!=NULL)) {
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - part count (%u) of client (%s) does not match part count (%u) of file \"%s\""), __FUNCTION__, forClient->GetUpPartCount(), forClient->DbgGetClientInfo(), GetPartCount(), GetFileName()) );
		ASSERT(0);
		return NULL;
	}*/

	CSafeMemFile data(1024);
	
	uint8 byUsedVersion;
	bool bIsSX2Packet;
	// NEO: NXS - [NeoXS] -- Xanatos -->
	if (forClient->SupportsNeoXS()){
		byUsedVersion = 3;
		bIsSX2Packet = false;
	}
	else
	// NEO: NXS END <-- Xanatos --
	if (forClient->SupportsSourceExchange2() && byRequestedVersion > 0){
		// the client uses SourceExchange2 and requested the highest version he knows
		// and we send the highest version we know, but of course not higher than his request
		byUsedVersion = min(byRequestedVersion, (uint8)SOURCEEXCHANGE2_VERSION);
		bIsSX2Packet = true;
		data.WriteUInt8(byUsedVersion);

		// we don't support any special SX2 options yet, reserved for later use
		if (nRequestedOptions != 0)
			DebugLogWarning(_T("Client requested unknown options for SourceExchange2: %u (%s)"), nRequestedOptions, forClient->DbgGetClientInfo());
	}
	else{
		byUsedVersion = forClient->GetSourceExchange1Version();
		bIsSX2Packet = false;
		if (forClient->SupportsSourceExchange2())
			DebugLogWarning(_T("Client which announced to support SX2 sent SX1 packet instead (%s)"), forClient->DbgGetClientInfo());
	}

	uint16 nCount = 0;
	data.WriteHash16(forClient->GetUploadFileID());
	data.WriteUInt16(nCount);
	uint32 cDbgNoSrc = 0;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* rcvFileStatus = forClient->GetFileStatus(this);
	const uint8* rcvstatus = rcvFileStatus ? rcvFileStatus->GetPartStatus() : NULL;
	// NEO: SCFS END <-- Xanatos --
	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		const CUpDownClient *cur_src = m_ClientUploadList.GetNext(pos);
		//if (cur_src->HasLowID() || cur_src == forClient || !(cur_src->GetUploadState() == US_UPLOADING || cur_src->GetUploadState() == US_ONUPLOADQUEUE))
		if ((cur_src->HasLowID() && !forClient->SupportsNeoXS()) || !cur_src->IsValidSource2()) // NEO: NXS - [NeoXS] <-- Xanatos --
			continue;
		//if (!cur_src->IsEd2kClient())
			//continue;

		bool bNeeded = false;
		//const uint8* rcvstatus = forClient->GetUpPartStatus(); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
		if (rcvstatus)
		{
			//ASSERT( forClient->GetUpPartCount() == GetPartCount() );
			//const uint8* srcstatus = cur_src->GetUpPartStatus();
			// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
			CClientFileStatus* srcFileStatus = cur_src->GetFileStatus(this);
			const uint8* srcstatus = srcFileStatus ? srcFileStatus->GetPartStatus() : NULL;
			// NEO: SCFS END <-- Xanatos --
			if (srcstatus)
			{
				//ASSERT( cur_src->GetUpPartCount() == GetPartCount() );
				//if (cur_src->GetUpPartCount() == forClient->GetUpPartCount())
				if (srcFileStatus->GetPartCount() == rcvFileStatus->GetPartCount()) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
				{
					for (UINT x = 0; x < GetPartCount(); x++)
					{
						if (srcstatus[x] && !rcvstatus[x])
						{
							// We know the recieving client needs a chunk from this client.
							bNeeded = true;
							break;
						}
					}
				}
				else
				{
					// should never happen
					//if (thePrefs.GetVerbose())
						//DEBUG_ONLY( DebugLogError(_T("*** %hs - found source (%s) with wrong part count (%u) attached to file \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), cur_src->GetUpPartCount(), GetFileName(), GetPartCount()));
						DEBUG_ONLY( DebugLogError(_T("*** %hs - found source (%s) with wrong part count (%u) attached to file \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), srcFileStatus->GetPartCount(), GetFileName(), GetPartCount())); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
				}
			}
			else
			{
				cDbgNoSrc++;
				// This client doesn't support upload chunk status. So just send it and hope for the best.
				bNeeded = true;
			}
		}
		else
		{
			//ASSERT( forClient->GetUpPartCount() == 0 );
			TRACE(_T("%hs, requesting client has no chunk status - %s"), __FUNCTION__, forClient->DbgGetClientInfo());
			// remote client does not support upload chunk status, search sources which have at least one complete part
			// we could even sort the list of sources by available chunks to return as much sources as possible which
			// have the most available chunks. but this could be a noticeable performance problem.
			//const uint8* srcstatus = cur_src->GetUpPartStatus();
			// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
			CClientFileStatus* srcFileStatus = cur_src->GetFileStatus(this);
			const uint8* srcstatus = srcFileStatus ? srcFileStatus->GetPartStatus() : NULL;
			// NEO: SCFS END <-- Xanatos --
			if (srcstatus)
			{
				//ASSERT( cur_src->GetUpPartCount() == GetPartCount() ); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
				//for (UINT x = 0; x < GetPartCount(); x++ )
				for (UINT x = 0; x < srcFileStatus->GetPartCount(); x++ ) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
				{
					if (srcstatus[x])
					{
						// this client has at least one chunk
						bNeeded = true;
						break;
					}
				}
			}
			else
			{
				// This client doesn't support upload chunk status. So just send it and hope for the best.
				bNeeded = true;
			}
		}

		if (bNeeded)
		{
			nCount++;
			uint32 dwID;
			if (byUsedVersion >= 3)
				dwID = cur_src->GetUserIDHybrid();
			else
				dwID = cur_src->GetIP();
		    data.WriteUInt32(dwID);
		    data.WriteUInt16(cur_src->GetUserPort());
			// NEO: NXS - [NeoXS] -- Xanatos -->
			if(forClient->SupportsNeoXS()){
				data.WriteHash16(cur_src->GetUserHash());
				cur_src->WriteNeoXSTags(&data);
			}else
			// NEO: NXS END <-- Xanatos --
			{
				data.WriteUInt32(cur_src->GetServerIP());
				data.WriteUInt16(cur_src->GetServerPort());
				if (byUsedVersion >= 2)
					data.WriteHash16(cur_src->GetUserHash());
				if (byUsedVersion >= 4){
					// ConnectSettings - SourceExchange V4
					// 4 Reserved (!)
					// 1 DirectCallback Supported/Available 
					// 1 CryptLayer Required
					// 1 CryptLayer Requested
					// 1 CryptLayer Supported
					const uint8 uSupportsCryptLayer	= cur_src->SupportsCryptLayer() ? 1 : 0;
					const uint8 uRequestsCryptLayer	= cur_src->RequestsCryptLayer() ? 1 : 0;
					const uint8 uRequiresCryptLayer	= cur_src->RequiresCryptLayer() ? 1 : 0;
					//const uint8 uDirectUDPCallback	= cur_src->SupportsDirectUDPCallback() ? 1 : 0;
					const uint8 byCryptOptions = /*(uDirectUDPCallback << 3) |*/ (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
					data.WriteUInt8(byCryptOptions);
				}
			}
			if (nCount > 500)
				break;
		}
	}
	TRACE(_T("%hs: Out of %u clients, %u had no valid chunk status\n"), __FUNCTION__, m_ClientUploadList.GetCount(), cDbgNoSrc);
	if (!nCount)
		return 0;
	data.Seek(bIsSX2Packet ? 17 : 16, SEEK_SET);
	data.WriteUInt16((uint16)nCount);

	//Packet* result = new Packet(&data, OP_EMULEPROT);
	//result->opcode = bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES;
	// NEO: NXS - [NeoXS] -- Xanatos -->
	Packet* result = new Packet(&data, forClient->SupportsNeoXS() ? OP_MODPROT : OP_EMULEPROT);
	result->opcode = forClient->SupportsNeoXS() ? OP_NEO_ANSWERSOURCES : (bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES);
	// NEO: NXS END <-- Xanatos --
	// (1+)16+2+501*(4+2+4+2+16+1) = 14547 (14548) bytes max.
	if (result->size > 354)
		result->PackPacket();
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXSend: Client source response SX2=%s, Version=%u; Count=%u, %s, File=\"%s\""), bIsSX2Packet ? _T("Yes") : _T("No"), byUsedVersion, nCount, forClient->DbgGetClientInfo(), GetFileName());
	return result;
}

void CKnownFile::SetFileComment(LPCTSTR pszComment)
{
	if (m_strComment.Compare(pszComment) != 0)
	{
		SetLastPublishTimeKadNotes(0);
		CIni ini(thePrefs.GetFileCommentsFilePath(), md4str(GetFileHash()));
		ini.WriteStringUTF8(_T("Comment"), pszComment);
		m_strComment = pszComment;

		for (POSITION pos = m_ClientUploadList.GetHeadPosition();pos != 0;)
			m_ClientUploadList.GetNext(pos)->SetCommentDirty();
	}
}

void CKnownFile::SetFileRating(UINT uRating)
{
	if (m_uRating != uRating)
	{
		SetLastPublishTimeKadNotes(0);
		CIni ini(thePrefs.GetFileCommentsFilePath(), md4str(GetFileHash()));
		ini.WriteInt(_T("Rate"), uRating);
		m_uRating = uRating;

		for (POSITION pos = m_ClientUploadList.GetHeadPosition();pos != 0;)
			m_ClientUploadList.GetNext(pos)->SetCommentDirty();
	}
}

// NEO: PP - [PasswordProtection] -- Xanatos -->
void CKnownFile::SetPWProt( CString iNewPW ){
	if (m_pwProt.Compare(iNewPW) != 0)
	{
		CIni ini(thePrefs.GetFileCommentsFilePath(), md4str(GetFileHash()));
		ini.WriteString(_T("Protection"), iNewPW);
		m_pwProt = iNewPW;
	}
}

bool CKnownFile::IsPWProt() const	
{ 
	if (!m_bProtectionLoaded) 
		(const_cast <CKnownFile*>(this))->LoadProtection(); 

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if (IsVoodooFile() && NeoPrefs.IsHideVoodooFiles())
		return true;
#endif // VOODOO // NEO: VOODOO END
	
	return m_pwProt.GetLength() > 0;
}

void CKnownFile::LoadProtection()
{
	CIni ini(thePrefs.GetFileCommentsFilePath(), md4str(GetFileHash()));
	m_pwProt = ini.GetStringUTF8(_T("Protection"), _T(""));
	m_bProtectionLoaded = true;
}
// NEO: PP END <-- Xanatos --

void CKnownFile::UpdateAutoUpPriority(){
	if( !IsAutoUpPriority() )
		return;
	if ( GetQueuedCount() > 20 ){
		if( GetUpPriority() != PR_LOW ){
			SetUpPriority( PR_LOW );
			theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
		}
		return;
	}
	if ( GetQueuedCount() > 1 ){
		if( GetUpPriority() != PR_NORMAL ){
			SetUpPriority( PR_NORMAL );
			theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
		}
		return;
	}
	if( GetUpPriority() != PR_HIGH){
		SetUpPriority( PR_HIGH );
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
	}
}

void CKnownFile::SetUpPriority(uint8 iNewUpPriority, bool bSave)
{
	m_iUpPriority = iNewUpPriority;
	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH );

	if( IsPartFile() && bSave )
		((CPartFile*)this)->SavePartFile();
}

void SecToTimeLength(unsigned long ulSec, CStringA& rstrTimeLength)
{
	// this function creates the content for the "length" ed2k meta tag which was introduced by eDonkeyHybrid 
	// with the data type 'string' :/  to save some bytes we do not format the duration with leading zeros
	if (ulSec >= 3600){
		UINT uHours = ulSec/3600;
		UINT uMin = (ulSec - uHours*3600)/60;
		UINT uSec = ulSec - uHours*3600 - uMin*60;
		rstrTimeLength.Format("%u:%02u:%02u", uHours, uMin, uSec);
	}
	else{
		UINT uMin = ulSec/60;
		UINT uSec = ulSec - uMin*60;
		rstrTimeLength.Format("%u:%02u", uMin, uSec);
	}
}

void SecToTimeLength(unsigned long ulSec, CStringW& rstrTimeLength)
{
	// this function creates the content for the "length" ed2k meta tag which was introduced by eDonkeyHybrid 
	// with the data type 'string' :/  to save some bytes we do not format the duration with leading zeros
	if (ulSec >= 3600){
		UINT uHours = ulSec/3600;
		UINT uMin = (ulSec - uHours*3600)/60;
		UINT uSec = ulSec - uHours*3600 - uMin*60;
		rstrTimeLength.Format(L"%u:%02u:%02u", uHours, uMin, uSec);
	}
	else{
		UINT uMin = ulSec/60;
		UINT uSec = ulSec - uMin*60;
		rstrTimeLength.Format(L"%u:%02u", uMin, uSec);
	}
}

void CKnownFile::RemoveMetaDataTags()
{
	static const struct
	{
		uint8	nID;
		uint8	nType;
	} _aEmuleMetaTags[] = 
	{
		{ FT_MEDIA_ARTIST,  2 },
		{ FT_MEDIA_ALBUM,   2 },
		{ FT_MEDIA_TITLE,   2 },
		{ FT_MEDIA_LENGTH,  3 },
		{ FT_MEDIA_BITRATE, 3 },
		{ FT_MEDIA_CODEC,   2 }
	};

	// 05-Jän-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
	// the chance to clean any available meta data tags and provide only tags which were determined by us.
	// Remove all meta tags. Never ever trust the meta tags received from other clients or servers.
	for (int j = 0; j < ARRSIZE(_aEmuleMetaTags); j++)
	{
		int i = 0;
		while (i < taglist.GetSize())
		{
			const CTag* pTag = taglist[i];
			if (pTag->GetNameID() == _aEmuleMetaTags[j].nID)
			{
				delete pTag;
				taglist.RemoveAt(i);
			}
			else
				i++;
		}
	}

	m_uMetaDataVer = 0;
}

CStringA GetED2KAudioCodec(WORD wFormatTag)
{
	if (wFormatTag == 0x0055)
		return "mp3";
	else if (wFormatTag == 0x0130)
		return "sipr";
	else if (wFormatTag == 0x2000)
		return "ac3";
	else if (wFormatTag == 0x2004)
		return "cook";
	return "";
}

CStringA GetED2KVideoCodec(DWORD biCompression)
{
	if (biCompression == BI_RGB)
		return "rgb";
	else if (biCompression == BI_RLE8)
		return "rle8";
	else if (biCompression == BI_RLE4)
		return "rle4";
	else if (biCompression == BI_BITFIELDS)
		return "bitfields";

	LPCSTR pszCompression = (LPCSTR)&biCompression;
	for (int i = 0; i < 4; i++)
	{
		if (   !isalnum((unsigned char)pszCompression[i])
			&& pszCompression[i] != '.' 
			&& pszCompression[i] != '_' 
			&& pszCompression[i] != ' ')
			return "";
	}

	CStringA strCodec;
	memcpy(strCodec.GetBuffer(4), &biCompression, 4);
	strCodec.ReleaseBuffer(4);
	strCodec.Trim();
	if (strCodec.GetLength() < 2)
		return "";
	strCodec.MakeLower();
	return strCodec;
}

SMediaInfo *GetRIFFMediaInfo(LPCTSTR pszFullPath)
{
	bool bIsAVI;
	SMediaInfo *mi = new SMediaInfo;
	if (!GetRIFFHeaders(pszFullPath, mi, bIsAVI)) {
		delete mi;
		return NULL;
	}
	return mi;
}

SMediaInfo *GetRMMediaInfo(LPCTSTR pszFullPath)
{
	bool bIsRM;
	SMediaInfo *mi = new SMediaInfo;
	if (!GetRMHeaders(pszFullPath, mi, bIsRM)) {
		delete mi;
		return NULL;
	}
	return mi;
}

// Max. string length which is used for string meta tags like TAG_MEDIA_TITLE, TAG_MEDIA_ARTIST, ...
#define	MAX_METADATA_STR_LEN	80

void TruncateED2KMetaData(CString& rstrData)
{
	rstrData.Trim();
	if (rstrData.GetLength() > MAX_METADATA_STR_LEN)
	{
		rstrData.Truncate(MAX_METADATA_STR_LEN);
		rstrData.Trim();
	}
}

void CKnownFile::UpdateMetaDataTags()
{
	// 05-Jän-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
	// the chance to clean any available meta data tags and provide only tags which were determined by us.
	RemoveMetaDataTags();

	if (thePrefs.GetExtractMetaData() == 0)
		return;

	TCHAR szExt[_MAX_EXT];
	//_tsplitpath(GetFileName(), NULL, NULL, NULL, szExt);
	_tsplitpath(GetFileName(true), NULL, NULL, NULL, szExt); // NEO: PP - [PasswordProtection] <-- Xanatos --
	_tcslwr(szExt);
	if (_tcscmp(szExt, _T(".mp3"))==0 || _tcscmp(szExt, _T(".mp2"))==0 || _tcscmp(szExt, _T(".mp1"))==0 || _tcscmp(szExt, _T(".mpa"))==0)
	{
		TCHAR szFullPath[MAX_PATH];
		//if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(), NULL)){
		if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(true), NULL)){ // NEO: PP - [PasswordProtection] <-- Xanatos --
			
			try{
				// ID3LIB BUG: If there are ID3v2 _and_ ID3v1 tags available, id3lib
				// destroys (actually corrupts) the Unicode strings from ID3v2 tags due to
				// converting Unicode to ASCII and then convertion back from ASCII to Unicode.
				// To prevent this, we force the reading of ID3v2 tags only, in case there are 
				// also ID3v1 tags available.
				ID3_Tag myTag;
				CStringA strFilePathA(szFullPath);
				size_t id3Size = myTag.Link(strFilePathA, ID3TT_ID3V2);
				if (id3Size == 0) {
					myTag.Clear();
					myTag.Link(strFilePathA, ID3TT_ID3V1);
				}

				const Mp3_Headerinfo* mp3info;
				mp3info = myTag.GetMp3HeaderInfo();
				if (mp3info)
				{
					// length
					if (mp3info->time){
						CTag* pTag = new CTag(FT_MEDIA_LENGTH, (uint32)mp3info->time);
						AddTagUnique(pTag);
						m_uMetaDataVer = META_DATA_VER;
					}

					// here we could also create a "codec" ed2k meta tag.. though it would probable not be worth the
					// extra bytes which would have to be sent to the servers..

					// bitrate
					UINT uBitrate = (mp3info->vbr_bitrate ? mp3info->vbr_bitrate : mp3info->bitrate) / 1000;
					if (uBitrate){
						CTag* pTag = new CTag(FT_MEDIA_BITRATE, (uint32)uBitrate);
						AddTagUnique(pTag);
						m_uMetaDataVer = META_DATA_VER;
					}
				}

				ID3_Tag::Iterator* iter = myTag.CreateIterator();
				const ID3_Frame* frame;
				while ((frame = iter->GetNext()) != NULL)
				{
					ID3_FrameID eFrameID = frame->GetID();
					switch (eFrameID)
					{
						case ID3FID_LEADARTIST:{
							wchar_t* pszText = ID3_GetStringW(frame, ID3FN_TEXT);
							CString strText(pszText);
							TruncateED2KMetaData(strText);
							if (!strText.IsEmpty()){
								CTag* pTag = new CTag(FT_MEDIA_ARTIST, strText);
								AddTagUnique(pTag);
								m_uMetaDataVer = META_DATA_VER;
							}
							delete[] pszText;
							break;
						}
						case ID3FID_ALBUM:{
							wchar_t* pszText = ID3_GetStringW(frame, ID3FN_TEXT);
							CString strText(pszText);
							TruncateED2KMetaData(strText);
							if (!strText.IsEmpty()){
								CTag* pTag = new CTag(FT_MEDIA_ALBUM, strText);
								AddTagUnique(pTag);
								m_uMetaDataVer = META_DATA_VER;
							}
							delete[] pszText;
							break;
						}
						case ID3FID_TITLE:{
							wchar_t* pszText = ID3_GetStringW(frame, ID3FN_TEXT);
							CString strText(pszText);
							TruncateED2KMetaData(strText);
							if (!strText.IsEmpty()){
								CTag* pTag = new CTag(FT_MEDIA_TITLE, strText);
								AddTagUnique(pTag);
								m_uMetaDataVer = META_DATA_VER;
							}
							delete[] pszText;
							break;
						}
					}
				}
				delete iter;
			}
			catch(...){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Unhandled exception while extracting file meta (MP3) data from \"%s\""), szFullPath);
				ASSERT(0);
			}
		}
	}
	else
	{
		TCHAR szFullPath[MAX_PATH];

		//if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(), NULL))
		if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(true), NULL)) // NEO: PP - [PasswordProtection] <-- Xanatos --
		{

			SMediaInfo* mi = NULL;
			try
			{
				mi = GetRIFFMediaInfo(szFullPath);
				if (mi == NULL)
					mi = GetRMMediaInfo(szFullPath);
				if (mi)
				{
					mi->InitFileLength();
					UINT uLengthSec = (UINT)mi->fFileLengthSec;

					CStringA strCodec;
					uint32 uBitrate = 0;
					if (mi->iVideoStreams) {
						strCodec = GetED2KVideoCodec(mi->video.bmiHeader.biCompression);
						uBitrate = (mi->video.dwBitRate + 500) / 1000;
					}
					else if (mi->iAudioStreams) {
						strCodec = GetED2KAudioCodec(mi->audio.wFormatTag);
						uBitrate = (DWORD)(((mi->audio.nAvgBytesPerSec * 8.0) + 500.0) / 1000.0);
					}

					if (uLengthSec) {
						CTag* pTag = new CTag(FT_MEDIA_LENGTH, (uint32)uLengthSec);
						AddTagUnique(pTag);
						m_uMetaDataVer = META_DATA_VER;
					}

					if (!strCodec.IsEmpty()) {
						CTag* pTag = new CTag(FT_MEDIA_CODEC, CString(strCodec));
						AddTagUnique(pTag);
						m_uMetaDataVer = META_DATA_VER;
					}

					if (uBitrate) {
						CTag* pTag = new CTag(FT_MEDIA_BITRATE, (uint32)uBitrate);
						AddTagUnique(pTag);
						m_uMetaDataVer = META_DATA_VER;
					}

					TruncateED2KMetaData(mi->strTitle);
					if (!mi->strTitle.IsEmpty()){
						CTag* pTag = new CTag(FT_MEDIA_TITLE, mi->strTitle);
						AddTagUnique(pTag);
						m_uMetaDataVer = META_DATA_VER;
					}

					TruncateED2KMetaData(mi->strAuthor);
					if (!mi->strAuthor.IsEmpty()){
						CTag* pTag = new CTag(FT_MEDIA_ARTIST, mi->strAuthor);
						AddTagUnique(pTag);
						m_uMetaDataVer = META_DATA_VER;
					}

					delete mi;
					return;
				}
			}
			catch(...){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("Unhandled exception while extracting file meta (AVI) data from \"%s\""), szFullPath);
				ASSERT(0);
			}
			delete mi;
		}

#ifdef HAVE_QEDIT_H
		if (thePrefs.GetExtractMetaData() >= 2)
		{
			// starting the MediaDet object takes a noticeable amount of time.. avoid starting that object
			// for files which are not expected to contain any Audio/Video data.
			// note also: MediaDet does not work well for too short files (e.g. 16K)
			//EED2KFileType eFileType = GetED2KFileTypeID(GetFileName());
			EED2KFileType eFileType = GetED2KFileTypeID(GetFileName(true)); // NEO: PP - [PasswordProtection] <-- Xanatos --
			if ((eFileType == ED2KFT_AUDIO || eFileType == ED2KFT_VIDEO) && GetFileSize() >= (uint64)32768)
			{
				// Avoid processing of some file types which are known to crash due to bugged DirectShow filters.
				TCHAR szExt[_MAX_EXT];
				//_tsplitpath(GetFileName(), NULL, NULL, NULL, szExt);
				_tsplitpath(GetFileName(true), NULL, NULL, NULL, szExt); // NEO: PP - [PasswordProtection] <-- Xanatos --
				_tcslwr(szExt);
				if (_tcscmp(szExt, _T(".ogm"))!=0 && _tcscmp(szExt, _T(".ogg"))!=0 && _tcscmp(szExt, _T(".mkv"))!=0)
				{
					TCHAR szFullPath[MAX_PATH];
					//if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(), NULL))
					if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(true), NULL)) // NEO: PP - [PasswordProtection] <-- Xanatos --
					{
						try{
							CComPtr<IMediaDet> pMediaDet;
							HRESULT hr = pMediaDet.CoCreateInstance(__uuidof(MediaDet));
							if (SUCCEEDED(hr))
							{
								USES_CONVERSION;
								if (SUCCEEDED(hr = pMediaDet->put_Filename(CComBSTR(T2W(szFullPath)))))
								{
									// Get the first audio/video streams
									long lAudioStream = -1;
									long lVideoStream = -1;
									double fVideoStreamLengthSec = 0.0;
									DWORD dwVideoBitRate = 0;
									DWORD dwVideoCodec = 0;
									double fAudioStreamLengthSec = 0.0;
									DWORD dwAudioBitRate = 0;
									WORD wAudioCodec = 0;
									long lStreams;
									if (SUCCEEDED(hr = pMediaDet->get_OutputStreams(&lStreams)))
									{
										for (long i = 0; i < lStreams; i++)
										{
											if (SUCCEEDED(hr = pMediaDet->put_CurrentStream(i)))
											{
												GUID major_type;
												if (SUCCEEDED(hr = pMediaDet->get_StreamType(&major_type)))
												{
													if (major_type == MEDIATYPE_Video)
													{
														if (lVideoStream == -1){
															lVideoStream = i;
															pMediaDet->get_StreamLength(&fVideoStreamLengthSec);

															AM_MEDIA_TYPE mt = {0};
															if (SUCCEEDED(hr = pMediaDet->get_StreamMediaType(&mt))){
																if (mt.formattype == FORMAT_VideoInfo){
																	VIDEOINFOHEADER* pVIH = (VIDEOINFOHEADER*)mt.pbFormat;
																	// do not use that 'dwBitRate', whatever this number is, it's not
																	// the bitrate of the encoded video stream. seems to be the bitrate
																	// of the uncompressed stream divided by 2 !??
																	//dwVideoBitRate = pVIH->dwBitRate / 1000;

																	// for AVI files this gives that used codec
																	// for MPEG(1) files this just gives "Y41P"
																	dwVideoCodec = pVIH->bmiHeader.biCompression;
																}
															}

															if (mt.pUnk != NULL)
																mt.pUnk->Release();
															if (mt.pbFormat != NULL)
																CoTaskMemFree(mt.pbFormat);
														}
													}
													else if (major_type == MEDIATYPE_Audio)
													{
														if (lAudioStream == -1){
															lAudioStream = i;
															pMediaDet->get_StreamLength(&fAudioStreamLengthSec);

															AM_MEDIA_TYPE mt = {0};
															if (SUCCEEDED(hr = pMediaDet->get_StreamMediaType(&mt))){
																if (mt.formattype == FORMAT_WaveFormatEx){
																	WAVEFORMATEX* wfx = (WAVEFORMATEX*)mt.pbFormat;
																	dwAudioBitRate = (DWORD)(((wfx->nAvgBytesPerSec * 8.0) + 500.0) / 1000.0);
																	wAudioCodec = wfx->wFormatTag;
																}
															}

															if (mt.pUnk != NULL)
																mt.pUnk->Release();
															if (mt.pbFormat != NULL)
																CoTaskMemFree(mt.pbFormat);
														}
													}
													else{
														TRACE("%s - Unknown stream type\n", GetFileName());
													}

													if (lVideoStream != -1 && lAudioStream != -1)
														break;
												}
											}
										}
									}

									UINT uLengthSec = 0;
									CStringA strCodec;
									uint32 uBitrate = 0;
									if (fVideoStreamLengthSec > 0.0){
										uLengthSec = (UINT)fVideoStreamLengthSec;
										strCodec = GetED2KVideoCodec(dwVideoCodec);
										uBitrate = dwVideoBitRate;
									}
									else if (fAudioStreamLengthSec > 0.0){
										uLengthSec = (UINT)fAudioStreamLengthSec;
										strCodec = GetED2KAudioCodec(wAudioCodec);
										uBitrate = dwAudioBitRate;
									}

									if (uLengthSec){
										CTag* pTag = new CTag(FT_MEDIA_LENGTH, (uint32)uLengthSec);
										AddTagUnique(pTag);
										m_uMetaDataVer = META_DATA_VER;
									}

									if (!strCodec.IsEmpty()){
										CTag* pTag = new CTag(FT_MEDIA_CODEC, CString(strCodec));
										AddTagUnique(pTag);
										m_uMetaDataVer = META_DATA_VER;
									}

									if (uBitrate){
										CTag* pTag = new CTag(FT_MEDIA_BITRATE, (uint32)uBitrate);
										AddTagUnique(pTag);
										m_uMetaDataVer = META_DATA_VER;
									}
								}
							}
						}
						catch(...){
							if (thePrefs.GetVerbose())
								AddDebugLogLine(false, _T("Unhandled exception while extracting meta data (MediaDet) from \"%s\""), szFullPath);
							ASSERT(0);
						}
					}
				}
			}
		}
#else//HAVE_QEDIT_H
#pragma message("WARNING: Missing 'qedit.h' header file - some features will get disabled. See the file 'emule_site_config.h' for more information.")
#endif//HAVE_QEDIT_H
	}
}

void CKnownFile::SetPublishedED2K(bool val){
	m_PublishedED2K = val;
	theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
}

bool CKnownFile::PublishNotes()
{
	if(m_lastPublishTimeKadNotes > (uint32)time(NULL))
	{
		return false;
	}
	if(GetFileComment() != _T(""))
	{
		m_lastPublishTimeKadNotes = (uint32)time(NULL)+KADEMLIAREPUBLISHTIMEN;
		return true;
	}
	if(GetFileRating() != 0)
	{
		m_lastPublishTimeKadNotes = (uint32)time(NULL)+KADEMLIAREPUBLISHTIMEN;
		return true;
	}

	return false;
}

bool CKnownFile::PublishSrc()
{
	uint32 lastBuddyIP = 0;
	if( theApp.IsFirewalled() && 
		(Kademlia::CUDPFirewallTester::IsFirewalledUDP(true) || !Kademlia::CUDPFirewallTester::IsVerified()))
	{
		CUpDownClient* buddy = theApp.clientlist->GetBuddy();
		if( buddy )
		{
			lastBuddyIP = theApp.clientlist->GetBuddy()->GetIP();
			if( lastBuddyIP != m_lastBuddyIP )
			{
				SetLastPublishTimeKadSrc( (uint32)time(NULL)+KADEMLIAREPUBLISHTIMES, lastBuddyIP );
				return true;
			}
		}
		else
			return false;
	}

	if(m_lastPublishTimeKadSrc > (uint32)time(NULL))
		return false;

	SetLastPublishTimeKadSrc((uint32)time(NULL)+KADEMLIAREPUBLISHTIMES,lastBuddyIP);
	return true;
}

bool CKnownFile::IsMovie() const
{
	//return (ED2KFT_VIDEO == GetED2KFileTypeID(GetFileName()) );
	return (ED2KFT_VIDEO == GetED2KFileTypeID(GetFileName(true)) ); // NEO: PP - [PasswordProtection] <-- Xanatos --
}

// function assumes that this file is shared and that any needed permission to preview exists. checks have to be done before calling! 
bool CKnownFile::GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender)
{
	//return GrabImage(GetPath() + CString(_T("\\")) + GetFileName(), nFramesToGrab,  dStartTime, bReduceColor, nMaxWidth, pSender);
	return GrabImage(GetPath() + CString(_T("\\")) + GetFileName(true), nFramesToGrab,  dStartTime, bReduceColor, nMaxWidth, pSender); // NEO: PP - [PasswordProtection] <-- Xanatos --
}

bool CKnownFile::GrabImage(CString strFileName,uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender)
{
	if (!IsMovie())
		return false;
	CFrameGrabThread* framegrabthread = (CFrameGrabThread*) AfxBeginThread(RUNTIME_CLASS(CFrameGrabThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
	framegrabthread->SetValues(this, strFileName, nFramesToGrab, dStartTime, bReduceColor, nMaxWidth, pSender);
	framegrabthread->ResumeThread();
	return true;
}

// imgResults[i] can be NULL
void CKnownFile::GrabbingFinished(CxImage** imgResults, uint8 nFramesGrabbed, void* pSender)
{
	// continue processing
	if (pSender == theApp.mmserver){
		theApp.mmserver->PreviewFinished(imgResults, nFramesGrabbed);
	}
	else if (theApp.clientlist->IsValidClient((CUpDownClient*)pSender)){
		((CUpDownClient*)pSender)->SendPreviewAnswer(this, imgResults, nFramesGrabbed);
	}
	else{
		//probably a client which got deleted while grabbing the frames for some reason
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Couldn't find Sender of FrameGrabbing Request"));
	}
	//cleanup
	for (int i = 0; i != nFramesGrabbed; i++)
		delete imgResults[i];
	delete[] imgResults;
}

CString CKnownFile::GetInfoSummary() const
{
	CString strFolder = GetPath();
	PathRemoveBackslash(strFolder.GetBuffer());
	strFolder.ReleaseBuffer();

	CString strAccepts, strRequests, strTransferred;
    strRequests.Format(_T("%u (%u)"), statistic.GetRequests(), statistic.GetAllTimeRequests());
	strAccepts.Format(_T("%u (%u)"), statistic.GetAccepts(), statistic.GetAllTimeAccepts());
	strTransferred.Format(_T("%s (%s)"), CastItoXBytes(statistic.GetTransferred(), false, false), CastItoXBytes(statistic.GetAllTimeTransferred(), false, false));
	CString strType = GetFileTypeDisplayStr();
	if (strType.IsEmpty())
		strType = _T("-");

	CString info;
	info.Format(_T("%s\n")
		+ GetResString(IDS_FD_HASH) + _T(" %s\n")
		+ GetResString(IDS_FD_SIZE) + _T(" %s\n<br_head>\n")
		+ GetResString(IDS_TYPE) + _T(": %s\n")
		+ GetResString(IDS_FOLDER) + _T(": %s\n\n")
		+ GetResString(IDS_PRIORITY) + _T(": %s\n")
		+ GetResString(IDS_SF_REQUESTS) + _T(": %s\n")
		+ GetResString(IDS_SF_ACCEPTS) + _T(": %s\n")
		+ GetResString(IDS_SF_TRANSFERRED) + _T(": %s"),
		GetFileName(),
		md4str(GetFileHash()),
		CastItoXBytes(GetFileSize(), false, false),
		strType,
		strFolder,
		GetUpPriorityDisplayString(),
		strRequests,
		strAccepts,
		strTransferred);
	return info;
}

CString CKnownFile::GetUpPriorityDisplayString() const {
	switch (GetUpPriority()) {
		case PR_VERYLOW :
			return GetResString(IDS_PRIOVERYLOW);
		case PR_LOW :
			if (IsAutoUpPriority())
				return GetResString(IDS_PRIOAUTOLOW);
			else
				return GetResString(IDS_PRIOLOW);
		case PR_NORMAL :
			if (IsAutoUpPriority())
				return GetResString(IDS_PRIOAUTONORMAL);
			else
				return GetResString(IDS_PRIONORMAL);
		case PR_HIGH :
			if (IsAutoUpPriority())
				return GetResString(IDS_PRIOAUTOHIGH);
			else
				return GetResString(IDS_PRIOHIGH);
		case PR_VERYHIGH :
			return GetResString(IDS_X_PRIOVERYHIGH); // NEO: MOD - [ForSRS] <-- Xanatos --
		default:
			return _T("");
	}
}

// NEO: RBT - [ReadBlockThread] -- Xanatos -->
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
bool CKnownFile::SetReadBlockFromFile(uint64 startOffset, uint32 toread, CObject* client, void* request, bool voodoo)
#else
bool CKnownFile::SetReadBlockFromFile(uint64 startOffset, uint32 toread, CObject* client, void* request)
#endif // VOODOO // NEO: VOODOO END
{
	if (!theApp.emuledlg->IsRunning()) // Don't start any last-minute hashing
		return false;

	m_BlocksToReadLocker.Lock();
	if(m_ReadThread == NULL){ // we have no activ hash thread start one
		m_ReadThread = AfxBeginThread(RUNTIME_CLASS(CReadBlockFromFileThread), THREAD_PRIORITY_BELOW_NORMAL,0, CREATE_SUSPENDED);
		if(m_ReadThread == NULL){
			ASSERT(0);
			return false;
		}
		((CReadBlockFromFileThread*)m_ReadThread)->SetKnownFile(this);
		m_ReadThread->ResumeThread();
	}

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	m_BlocksToRead.AddTail(new ReadBlockOrder(startOffset, toread, client, request, voodoo));
#else
	m_BlocksToRead.AddTail(new ReadBlockOrder(startOffset, toread, client, request));
#endif // VOODOO // NEO: VOODOO END

	m_BlocksToReadLocker.Unlock();

	return true;
} 

IMPLEMENT_DYNCREATE(CReadBlockFromFileThread, CWinThread)
void CReadBlockFromFileThread::SetKnownFile(CKnownFile* pOwner)
{
	m_pOwner = pOwner;
	if (m_pOwner->IsPartFile())
		fullname = RemoveFileExtension(((CPartFile*)m_pOwner)->GetFullName());
	else
		fullname.Format(_T("%s\\%s"),m_pOwner->GetPath(),m_pOwner->GetFileName());
}

int CReadBlockFromFileThread::Run() {
	DbgSetThreadName("CReadBlockFromFileThread");
	
	InitThreadLocale();

	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END

	m_pOwner->m_BlocksToReadLocker.Lock();
	while(!m_pOwner->m_BlocksToRead.IsEmpty())
	{
		ReadBlockOrder* readOrder = m_pOwner->m_BlocksToRead.RemoveHead();

		if (!theApp.emuledlg->IsRunning())	// in case of shutdown while still hashing
			break;

		m_pOwner->m_BlocksToReadLocker.Unlock();
	
		byte* filedata = NULL;
		CSyncHelper lockFile;
		try{

			//CSingleLock sLock1(&(theApp.hashing_mut), TRUE); //SafeHash - wait a current hashing process end before read the chunk

			if (m_pOwner->IsPartFile() /*&& ((CPartFile*)m_pOwner)->GetStatus() != PS_COMPLETE*/){
				((CPartFile*)m_pOwner)->m_FileCompleteMutex.Lock();
				lockFile.m_pObject = &((CPartFile*)m_pOwner)->m_FileCompleteMutex;
				// If it's a part file which we are uploading the file remains locked until we've read the
				// current block. This way the file completion thread can not (try to) "move" the file into
				// the incoming directory.
			}
		
			//if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
			if (!file.Open(fullname,CFile::modeRead|CFile::osRandomAccess|CFile::shareDenyNone)) //MORPH - Optimization
				AfxThrowFileException(CFileException::fileNotFound, 0, fullname);

			file.Seek(readOrder->StartOffset,0);
			
			filedata = new byte[readOrder->togo+500];
			if (uint32 done = file.Read(filedata,readOrder->togo) != readOrder->togo){
				file.SeekToBegin();
				file.Read(filedata + done,readOrder->togo-done);
			}
			file.Close();

			if (lockFile.m_pObject){
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}
		}
		catch(CFileException* e)
		{
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			e->GetErrorMessage(szError, ARRSIZE(szError));
			if (thePrefs.GetVerbose())
				DebugLogWarning(_T("Failed to create upload package %s"), szError);
			e->Delete();
		
			delete[] filedata;
			filedata = RBT_ERROR;
		}
		catch(...)
		{
			delete[] filedata;
			filedata = RBT_ERROR;
			ASSERT(0);
		}

		if (theApp.emuledlg && theApp.emuledlg->IsRunning())
			PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE, (WPARAM)filedata,(LPARAM)readOrder);
		else {
			if (filedata != RBT_ERROR && filedata != RBT_ACTIVE && filedata != NULL)
				delete[] filedata;
			filedata = NULL;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			if(readOrder->m_voodoo)
				delete (sDataRequest*)readOrder->m_request;
#endif // VOODOO // NEO: VOODOO END
			delete readOrder;
		}

		m_pOwner->m_BlocksToReadLocker.Lock();
	}

	if(m_pOwner->m_ReadThread == this) 
		m_pOwner->m_ReadThread = NULL;
	m_pOwner->m_BlocksToReadLocker.Unlock();

	return 0;
}
// NEO: RBT END <-- Xanatos --

// NEO: MOD -- Xanatos -->
uint32 CKnownFile::GetPartSize(UINT part) const
{
	if(part+1<GetPartCount())
		return PARTSIZE;

	uint32 size = (uint32)(uint64)m_nFileSize % PARTSIZE;
	if(size)
		return size;
	return PARTSIZE;
}
// NEO: MOD END <-- Xanatos --

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
void CKnownFile::AddMaster(CVoodooSocket* Master)
{
	ASSERT(!IsRealFile());
	if(IsRealFile()) // file is real no master needed
		return;

	CMasterDatas* Datas = GetMasterDatas(Master);
	if(Datas) // master is already on list
		return;

	if(!theApp.sharedfiles->IsFilePtrInList(this))
		theApp.sharedfiles->SafeAddKFile(this); // add to share

	Datas = new CMasterDatas;
	m_MasterMap.SetAt(Master,Datas);
}

void CKnownFile::RemoveMaster(CVoodooSocket* Master)
{
	ASSERT(!IsRealFile());

	CMasterDatas* Datas = GetMasterDatas(Master);
	if(Datas == NULL) // master is not on list nothing to do
		return;

	m_MasterMap.RemoveKey(Master);
	delete Datas;

	if(m_MasterMap.IsEmpty()) // file is not real becouse GetMasterDatas != NULL and now all masters are gone
		theApp.sharedfiles->RemoveFile(this); // unshare it
}

CMasterDatas* CKnownFile::GetMasterDatas(CVoodooSocket* Master)
{
	CMasterDatas* Datas;
	return m_MasterMap.Lookup(Master, Datas) ? Datas : NULL;
}

CVoodooSocket* CKnownFile::GetMaster(uint64 start, uint64 end)
{
	CMasterDatas* Datas = NULL;
	CVoodooSocket* Master = NULL;
	POSITION pos = m_MasterMap.GetStartPosition();

	if(m_MasterMap.GetCount() > 1){ // if we have more tha one master we must find one who have the needed part complete
		while (pos){
			m_MasterMap.GetNextAssoc(pos, Master, Datas);

			if(Datas->HaveGapList() && !Datas->IsComplete(start,end))
				continue;
		
			return Master;
		}
	}

	if(pos != NULL)
		m_MasterMap.GetNextAssoc(pos, Master, Datas);

	return Master;
}

bool CKnownFile::IsRealFile() const
{
	return theApp.sharedfiles->IsFilePtrInList(this) && !IsVoodooFile(); // if the file is in shere and dont ahve any master it must be a real file
}
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
UINT CKnownFile::GetCategory() const
{
	if (m_category > (UINT)(IsPartFile() ? thePrefs.GetCatCount() : thePrefs.GetFullCatCount()) - 1)
		return 0;
	return m_category;
}

void CKnownFile::SetCategory(UINT cat, uint8 /*init*/) // NEO: MOD - [SetCategory]
{
	m_category=cat;

	// NEO: FCFG - [FileConfiguration]
	Category_Struct* Category = thePrefs.GetCategory(cat);
	CKnownPreferences* newKnownPrefs = Category ? Category->KnownPrefs : NULL;
	if(newKnownPrefs){
		if(KnownPrefs->IsGlobalPrefs() || KnownPrefs->IsCategoryPrefs())
			KnownPrefs = newKnownPrefs;
		else
			((CKnownPreferencesEx*)KnownPrefs)->KnownPrefs = newKnownPrefs;		
	}else{
		if(KnownPrefs->IsCategoryPrefs())
			KnownPrefs = &NeoPrefs.KnownPrefs;
		else if(KnownPrefs->IsFilePrefs())
			((CKnownPreferencesEx*)KnownPrefs)->KnownPrefs = &NeoPrefs.KnownPrefs;
	}
	// NEO: FCFG END
}
// NEO: NSC END <-- Xanatos --

// NEO: FCFG - [FileConfiguration] -- Xanatos -->
void CKnownFile::UpdateKnownPrefs(CKnownPreferences* cfgKnownPrefs)
{
	if(cfgKnownPrefs->IsEmpty())
	{
		if(cfgKnownPrefs != KnownPrefs)
		{
			ASSERT(!KnownPrefs->IsFilePrefs());

			delete cfgKnownPrefs;
		}
		else if(cfgKnownPrefs == KnownPrefs)
		{
			Category_Struct* Category = thePrefs.GetCategory(GetCategory());
			CKnownPreferences* newKnownPrefs = Category ? Category->KnownPrefs : NULL;
			if(newKnownPrefs)
				KnownPrefs = newKnownPrefs;
			else
				KnownPrefs = &NeoPrefs.KnownPrefs;
			delete cfgKnownPrefs;
		}
	}
	else //if(!cfgKnownPrefs->IsEmpty())
	{
		if(cfgKnownPrefs != KnownPrefs)
		{
			ASSERT(!KnownPrefs->IsFilePrefs());

			((CKnownPreferencesEx*)cfgKnownPrefs)->KnownFile = this;
			KnownPrefs = cfgKnownPrefs;

			Category_Struct* Category = thePrefs.GetCategory(GetCategory());
			CKnownPreferences* newKnownPrefs = Category ? Category->KnownPrefs : NULL;
			if(newKnownPrefs)
				((CKnownPreferencesEx*)cfgKnownPrefs)->KnownPrefs = newKnownPrefs;
			else
				((CKnownPreferencesEx*)cfgKnownPrefs)->KnownPrefs = &NeoPrefs.KnownPrefs;
		}
		else // if(cfgKnownPrefs == KnownPrefs)
		{
			ASSERT(KnownPrefs->IsFilePrefs());
		}
	}
}
// NEO: FCFG END <-- Xanatos --

// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
// adjust basis priority according to category settings
uint8 CKnownFile::GetUpPriorityEx() const 
{ 
	Category_Struct* pCatStruct = thePrefs.GetCategory(GetCategory());
	uint8 UpPriority = pCatStruct ? (uint8)pCatStruct->boost : PR_NORMAL;

	if(UpPriority == PR_NORMAL){
		return m_iUpPriority; 
	}
	else if(UpPriority == PR_HIGH){
		switch(m_iUpPriority){
			case PR_VERYHIGH:	return PR_VERYHIGH;
			case PR_HIGH:
			case PR_NORMAL:   	return PR_HIGH;
			case PR_LOW:      	return PR_NORMAL;
			case PR_VERYLOW:  	return PR_LOW;
		} 
	}
	else if(UpPriority == PR_LOW){
		switch(m_iUpPriority){
			case PR_VERYHIGH:	return PR_HIGH;
			case PR_HIGH:     	return PR_NORMAL;
			case PR_NORMAL:
			case PR_LOW:		return PR_NORMAL;
			case PR_VERYLOW:  	return PR_VERYLOW;
		} 
	}

	return PR_NORMAL;
}
// NEO: NXC END <-- Xanatos --

// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
float CalcLimit(int Limit, int LimitMode, int LimitLow, int LimitHigh, int Value)
{
	if(Limit == 0)
		return 1;
	else if(LimitHigh == 0)
		return 0;

	if(LimitMode == REL_SIMPLY){
		if(LimitHigh > Value)
			return 1;
		return  0;
	}
	else if(LimitMode == REL_LINEAR){
		if(Limit == LIM_SINGLE || LimitHigh <= LimitLow)
			return (1 - ((float)Value/LimitHigh));
		else if(Limit == LIM_BOOTH)
			return (Value <= LimitLow) ? 1 : (1 - ((float)(Value-LimitLow)/(LimitHigh-LimitLow)));
		else
			return 1;
	}
	else if(LimitMode == REL_EXPONENTIAL){
		if(Limit == LIM_SINGLE || LimitHigh <= LimitLow)
			return (1 - ((float)(Value^2)/(LimitHigh^2)));
		else if(Limit == LIM_BOOTH)
			return (Value <= LimitLow) ? 1 : (1 - ((float)((Value-LimitLow)^2)/((LimitHigh-LimitLow)^2)));
		else
			return 1;
	}

	ASSERT(FALSE);
	return 1;
}

void CKnownFile::CalcRelease(bool bFlag)
{
	if(!IsReleasePriorityEx()) // NEO: NXC - [NewExtendedCategories]
		return;

	if (!bFlag && (m_uLastReleaseModifyer + KnownPrefs->GetReleaseTimer() > ::GetTickCount()) || (uint64)GetFileSize() == 0)
		return;
	m_uLastReleaseModifyer = ::GetTickCount();

	// Note: we calculate this value also in REL_POWER mode in order to to can beter chose between cleints in with PS files

	// release limit
	float ReleaseLimit = CalcLimit(KnownPrefs->IsReleaseLimit(), KnownPrefs->GetReleaseLimitMode(), KnownPrefs->GetReleaseLimitHigh(), KnownPrefs->GetReleaseLimitLow(), (int)(((float)statistic.GetAllTimeTransferred()/(uint64)GetFileSize())*100));
	float ReleaseLimitComplete = CalcLimit(KnownPrefs->IsReleaseLimitComplete() && NeoPrefs.UsePartTraffic(), KnownPrefs->GetReleaseLimitCompleteMode(), KnownPrefs->GetReleaseLimitCompleteHigh(), KnownPrefs->GetReleaseLimitCompleteLow(), (int)(statistic.GetCompleteReleases()*100));
	if(KnownPrefs->IsReleaseLimitLink())
		ReleaseLimit = ((ReleaseLimit > ReleaseLimitComplete) ? ReleaseLimit : ReleaseLimitComplete); // Take Hier
	else
		ReleaseLimit = ((ReleaseLimit > ReleaseLimitComplete) ? ReleaseLimitComplete : ReleaseLimit); // Take Lower
	
	// source limit
	float SourceLimit = CalcLimit(KnownPrefs->IsSourceLimit(), KnownPrefs->GetSourceLimitMode(), KnownPrefs->GetSourceLimitHigh(), KnownPrefs->GetSourceLimitLow(), IsPartFile() ? ((CPartFile*)this)->GetSourceCount() : this->GetQueuedCount());
	float SourceLimitComplete = CalcLimit(KnownPrefs->IsSourceLimitComplete(), KnownPrefs->GetSourceLimitCompleteMode(), KnownPrefs->GetSourceLimitCompleteHigh(), KnownPrefs->GetSourceLimitCompleteLow(), m_nCompleteSourcesCount);
	if(KnownPrefs->IsSourceLimitLink())
		SourceLimit = ((SourceLimit > SourceLimitComplete) ? SourceLimit : SourceLimitComplete); // Take Hier
	else
		SourceLimit = ((SourceLimit > SourceLimitComplete) ? SourceLimitComplete : SourceLimit); // Take Lower

	// combinet limit
	float CombinedLimit = 1.0F;
	if(KnownPrefs->IsLimitLink())
		CombinedLimit = ((ReleaseLimit > SourceLimit) ? ReleaseLimit : SourceLimit); // Take Hier
	else
		CombinedLimit = ((ReleaseLimit > SourceLimit) ? SourceLimit : ReleaseLimit); // Take Lower

	// update release modifyed
	if(CombinedLimit > 1.0F)
		CombinedLimit = 1.0F;
	m_fReleaseModifyer = (float)KnownPrefs->GetReleaseLevel() * CombinedLimit;


	// check powershare
	if(KnownPrefs->GetReleaseMode() == REL_MIXED)
	{
		// release check
		bool ReleaseCheck = !KnownPrefs->IsReleaseLimit() || KnownPrefs->GetReleaseLimitLow() < (float)statistic.GetAllTimeTransferred()/(uint64)GetFileSize();
		bool ReleaseCheckComplete = !KnownPrefs->IsReleaseLimitComplete() || KnownPrefs->GetReleaseLimitCompleteLow() < statistic.GetCompleteReleases();
		if(KnownPrefs->IsReleaseLimitLink())
			ReleaseCheck = (ReleaseCheck || ReleaseCheckComplete); // one not false
		else
			ReleaseCheck = (ReleaseCheck && ReleaseCheckComplete); // booth true

		// source check
		bool SourceCheck = !KnownPrefs->IsSourceLimit() || KnownPrefs->GetSourceLimitLow() < (int)(IsPartFile() ? ((CPartFile*)this)->GetSourceCount() : this->GetQueuedCount());
		bool SourceCheckComplete = !KnownPrefs->IsSourceLimitComplete() || KnownPrefs->GetSourceLimitCompleteLow() < m_nCompleteSourcesCount;
		if(KnownPrefs->IsSourceLimitLink())
			SourceCheck = (SourceCheck || SourceCheckComplete); // one not false
		else
			SourceCheck = (SourceCheck && SourceCheckComplete); // booth true

		// conbined chack
		bool CombinedCheck = false;
		if(KnownPrefs->IsSourceLimitLink())
			CombinedCheck = (ReleaseCheck || SourceCheck); // one not false
		else
			CombinedCheck = (ReleaseCheck && SourceCheck); // booth true

		// update powershare flag
		m_bPowerShared = CombinedCheck;
	}
	else if(KnownPrefs->GetReleaseMode() == REL_POWER){
		m_bPowerShared = (m_fReleaseModifyer > 1);
	}else /*if(KnownPrefs->GetReleaseMode() == REL_BOOST)*/{
		m_bPowerShared = false;
	}
}

void CKnownFile::SetReleasePriority(uint8 uReleasePriority, bool bSave)
{
	m_uReleasePriority = uReleasePriority;

	if(m_uReleasePriority)
		CalcRelease(true);

	if( IsPartFile() && bSave )
		((CPartFile*)this)->SavePartFile();
}

// NEO: NXC - [NewExtendedCategories]
uint8  CKnownFile::IsReleasePriorityEx() const
{
	Category_Struct* pCatStruct = thePrefs.GetCategory(GetCategory());
	bool release = pCatStruct ? (uint8)pCatStruct->release : false;
	return IsReleasePriority() || release;
}
// NEO: NXC END
// NEO: SRS END <-- Xanatos --

bool CKnownFile::HasPreferences(){
	return KnownPrefs->IsFilePrefs();
}
// NEO: FCFG END <-- Xanatos --

// NEO: XCs - [SaveComments] -- Xanatos -->
bool CKnownFile::LoadComments(CFileDataIO* file){
	uint16 count = file->ReadUInt16();

	for(uint16 i = 0; i < count; i++)
	{
		KnownComment cs;
		file->ReadHash16(cs.m_achUserHash);
		cs.m_strUserName = file->ReadString(true);
		cs.m_strFileName = file->ReadString(true);
		cs.m_uRating = file->ReadUInt8();
		cs.m_strComment = file->ReadString(true);
		//cs.m_iUserIP = 0;
		//cs.m_iUserPort = 0;

		AddComment(cs);
	}

	return true;
}

bool CKnownFile::SaveComments(CFileDataIO* file){

	file->WriteUInt16((uint16)m_CommentList.GetCount());

	for (POSITION pos = m_CommentList.GetHeadPosition(); pos != NULL;){
		KnownComment* comment = m_CommentList.GetNext(pos);
		file->WriteHash16(comment->m_achUserHash);
		file->WriteString(comment->m_strUserName,utf8strRaw);
		file->WriteString(comment->m_strFileName,utf8strRaw);
		file->WriteUInt8(comment->m_uRating);
		file->WriteString(comment->m_strComment,utf8strRaw);
	}

	return true;
}
// NEO: XC END <-- Xanatos --

// NEO: CRC - [MorphCRCTag] -- Xanatos -->
bool CKnownFile::IsCRCOk() const
{
	CString FName = GetFileName(true); // NEO: PP - [PasswordProtection] <-- Xanatos --
	FName.MakeUpper (); // Uppercase the filename ! 
						// Our CRC is upper case...
	CString buffer;
	buffer.Format(_T("%02X%02X%02X%02X"),	(int) m_CRC32 [3],
											(int) m_CRC32 [2],
											(int) m_CRC32 [1],
											(int) m_CRC32 [0]);
	return (FName.Find (buffer) != -1);
}
// NEO: CRC END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
void CKnownFile::GetTooltipFileInfo(CString &info)
{
	CAbstractFile::GetTooltipFileInfo(info);

	CString buffer;
	if(m_nCompleteSourcesCountLo == m_nCompleteSourcesCountHi)
		buffer.Format(_T("%u"), m_nCompleteSourcesCountLo);
	else if(m_nCompleteSourcesCountLo == 0)
		buffer.Format(_T("~ %u"), m_nCompleteSourcesCountHi);
	else
		buffer.Format(_T("%u - %u"), m_nCompleteSourcesCountLo, m_nCompleteSourcesCountHi);
	buffer.AppendFormat(_T(" (%u)"), GetQueuedCount());
	info.AppendFormat(GetResString(IDS_X_QUEUE), buffer);

	// NEO: NPT - [NeoPartTraffic]
	if(NeoPrefs.UsePartTraffic()){
		double CompHi = (double)statistic.GetAllTimeTransferred()/(uint64)GetFileSize();
		double CompLo = statistic.GetCompleteReleases();
		if (CompLo == 0){
			buffer.Format(_T("~ %.2f"), CompHi);
		}else if (CompLo == CompHi){
			buffer.Format(_T("%.2f"), CompLo);
		}else{
			buffer.Format(_T("%.2f - %.2f"), CompLo, CompHi);
		}
		info.AppendFormat(GetResString(IDS_X_TRAFFIC), buffer);
	}
	// NEO: NPT END
}
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --