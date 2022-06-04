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
#include <share.h>
#include <sys/stat.h>
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
#include "ini2.h"
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
#include "MuleStatusBarCtrl.h" //Xman Progress Hash (O2)
#include "UploadQueue.h" // Mephisto Upload - Mephisto

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

// Meta data version
// -----------------
//	0	untrusted meta data which was received via search results
//	1	trusted meta data, Unicode (strings where not stored correctly)
//	2	0.49c: trusted meta data, Unicode
#define	META_DATA_VER	2

IMPLEMENT_DYNAMIC(CKnownFile, CShareableFile)

CKnownFile::CKnownFile()
{
	m_iPartCount = 0;
	m_iED2KPartCount = 0;
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
	(void)m_strComment;
	m_PublishedED2K = false;
	kadFileSearchID = 0;
	SetLastPublishTimeKadSrc(0,0);
	m_nCompleteSourcesTime = time(NULL);
	m_nCompleteSourcesCount = 1;
	m_nCompleteSourcesCountLo = 1;
	m_nCompleteSourcesCountHi = 1;
	m_uMetaDataVer = 0;
	m_lastPublishTimeKadSrc = 0;
	m_lastPublishTimeKadNotes = 0;
	m_lastBuddyIP = 0;
	m_pCollection = NULL;
	m_timeLastSeen = 0;
	m_bAICHRecoverHashSetAvailable = false;
	onuploadqueue = 0; //Xman see OnUploadqueue
	// ==> Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle
	/*
	hideos=1; //Xman PowerRelease
	*/
	// <== Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle
	// Maella -One-queue-per-file- (idea bloodymad)
	m_startUploadTime = ::GetTickCount();
	// Maella end
	//Xman show virtual sources (morph)
	m_nVirtualCompleteSourcesCount = 0;

	//Xman advanced upload-priority
	pushfaktor=0;
	m_nVirtualUploadSources = 0;
	//Xman end

	// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
	m_iHideOS = -1;
	m_iSelectiveChunk = -1;
	m_iShareOnlyTheNeed = -1;
	// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	m_bpowershared = false;
	m_powershared = -1;
	m_bPowerShareAuthorized = true;
	m_bPowerShareAuto = false;
	m_iPowerShareLimit = -1;
	// <== PowerShare [ZZ/MorphXT] - Stulle

	m_iPsAmountLimit = -1; // Limit PS by amount of data uploaded [Stulle] - Stulle
}

CKnownFile::~CKnownFile()
{
	delete m_pCollection;
}

#ifdef _DEBUG
void CKnownFile::AssertValid() const
{
	CAbstractFile::AssertValid();

	(void)m_tUtcLastModified;
	(void)statistic;
	(void)m_nCompleteSourcesTime;
	(void)m_nCompleteSourcesCount;
	(void)m_nCompleteSourcesCountLo;
	(void)m_nCompleteSourcesCountHi;
	m_ClientUploadList.AssertValid();
	m_AvailPartFrequency.AssertValid();
	(void)m_strDirectory;
	(void)m_strFilePath;
	(void)m_iPartCount;
	(void)m_iED2KPartCount;
	//Xman PowerRelease
	/*
	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH );
	*/
	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH || m_iUpPriority == PR_POWER );
	//Xman end
	CHECK_BOOL(m_bAutoUpPriority);
	//Xman
	/*
	(void)s_ShareStatusBar;
	*/
	//Xman end
	CHECK_BOOL(m_PublishedED2K);
	(void)kadFileSearchID;
	(void)m_lastPublishTimeKadSrc;
	(void)m_lastPublishTimeKadNotes;
	(void)m_lastBuddyIP;
	(void)wordlist;
}

void CKnownFile::Dump(CDumpContext& dc) const
{
	CAbstractFile::Dump(dc);
}
#endif

// Xman -Code Improvement-
/*
CBarShader CKnownFile::s_ShareStatusBar(16);

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
*/
void CKnownFile::DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
{
	CBarShader statusBar(rect->bottom - rect->top, rect->right - rect->left);
	statusBar.SetFileSize(GetFileSize()); 
	//Xman 4.4
	//the official version is well coded, but most time, there is at least one complete source
	//and because of this, the really needed parts are never drawn in red
	//furthermore I don't trust the CompleteSourceCount-Value
	if(m_ClientUploadList.GetSize() > 0 && GetPartCount()>1) //looks bad if always red for files with only one chunk
	{
		const COLORREF crMissing = RGB(255, 0, 0);
		statusBar.Fill(crMissing);

		if (!onlygreyrect) 
		{
			for (UINT i = 0; i < GetPartCount(); i++)
			{
				uint32 frequency = 0;
				if(!m_AvailPartFrequency.IsEmpty()) {
					frequency = m_AvailPartFrequency[i];
				}

				if(frequency > 0 ){
					const COLORREF color = RGB(0, (22*(frequency-1) >= 210)? 0:210-(22*(frequency-1)), 255);
					statusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1),color);
				}
			}
		}
	}
	else if(m_nCompleteSourcesCountLo > 1)
	{
			uint32 frequency = m_nCompleteSourcesCountLo - 1;
			const COLORREF color = RGB(0, (22*(frequency-1) >= 210)? 0:210-(22*(frequency-1)), 255);
			statusBar.Fill(color);

	}
	//Xman end 4.4
	else {
		// We have no info about chunk frequency in the net, so just color the chunk we have as black.
		const COLORREF crNooneAsked = (bFlat) ? RGB(0, 0, 0) : RGB(104, 104, 104);
		statusBar.Fill(crNooneAsked);
	}
	statusBar.Draw(dc, rect->left, rect->top, bFlat); 
} 
//Xman end

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
	
	// Reset part counters
	if ((UINT)m_AvailPartFrequency.GetSize() < partcount)
		m_AvailPartFrequency.SetSize(partcount);
	for (UINT i = 0; i < partcount; i++)
		m_AvailPartFrequency[i] = 0;

	CArray<uint16, uint16> count;
	if (flag)
		count.SetSize(0, m_ClientUploadList.GetSize());

	UINT iCompleteSourcesCountInfoReceived = 0; // PowerShare [ZZ/MorphXT] - Stulle

	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		//This could be a partfile that just completed.. Many of these clients will not have this information.
		if (cur_src->m_abyUpPartStatus && cur_src->GetUpPartCount() == partcount)
		{
			for (UINT i = 0; i < partcount; i++)
			{
				if (cur_src->IsUpPartAvailable(i))
					m_AvailPartFrequency[i] += 1;
			}
			if (flag)
				count.Add(cur_src->GetUpCompleteSourcesCount());

			// ==> PowerShare [ZZ/MorphXT] - Stulle
			if (cur_src->GetUpCompleteSourcesCount()>0)
				++iCompleteSourcesCountInfoReceived;
			// <== PowerShare [ZZ/MorphXT] - Stulle
		}
		// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		if (cur_src->IsDownloading()){
			uint8* m_abyUpPartUploadingAndUploaded = new uint8[partcount];
			cur_src->GetUploadingAndUploadedPart(m_abyUpPartUploadingAndUploaded, partcount);
			for (UINT i = 0; i < partcount; i++)
				if (m_AvailPartFrequency[i] == 0 && m_abyUpPartUploadingAndUploaded[i]>0)
					m_AvailPartFrequency[i] = 1;
			delete[] m_abyUpPartUploadingAndUploaded;
		}
		// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
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
		//Xman show virtual sources
		m_nVirtualCompleteSourcesCount=m_nCompleteSourcesCount;
		//Xman end
	
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
				/* Xman Code Improvement
				m_nCompleteSourcesCountLo= m_nCompleteSourcesCount;
				m_nCompleteSourcesCount= count.GetAt(j);
				if( m_nCompleteSourcesCount < m_nCompleteSourcesCountLo )
					m_nCompleteSourcesCount = m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= count.GetAt(k);
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
				*/
				//the difference to partfiles is, that we don't see the complete sources
				//-> the official CountLo gives a too small number
				m_nCompleteSourcesCountLo= count.GetAt(i);
				if(m_nCompleteSourcesCountLo < m_nCompleteSourcesCount)
					m_nCompleteSourcesCountLo = m_nCompleteSourcesCount;
				m_nCompleteSourcesCount= count.GetAt(j);
				if( m_nCompleteSourcesCount < m_nCompleteSourcesCountLo )
					m_nCompleteSourcesCount = m_nCompleteSourcesCountLo;
				m_nCompleteSourcesCountHi= count.GetAt(k);
				if( m_nCompleteSourcesCountHi < m_nCompleteSourcesCount )
					m_nCompleteSourcesCountHi = m_nCompleteSourcesCount;
				//Xman end
			}
		}
		m_nCompleteSourcesTime = time(NULL) + (60);
	}
	// ==> PowerShare [ZZ/MorphXT] - Stulle
	/*
	UpdatePowerShareLimit(m_nVirtualCompleteSourcesCount<=1 || m_nCompleteSourcesCountHi<=GetPartCount(), m_nCompleteSourcesCountHi==1 && m_nVirtualCompleteSourcesCount==0 && iCompleteSourcesCountInfoReceived>1,m_nCompleteSourcesCountHi>((GetPowerShareLimit()>=0)?GetPowerShareLimit():thePrefs.GetPowerShareLimit()));
	*/
	int iPsAmountLimit = (GetPsAmountLimit()>=0 ? GetPsAmountLimit() : thePrefs.GetPsAmountLimit());
	int iPowerShareLimit = (GetPowerShareLimit()>=0 ? GetPowerShareLimit() : thePrefs.GetPowerShareLimit());
	bool bPowerShareLimit = true;
	if(iPsAmountLimit>0)
	{
		if((((double)statistic.GetAllTimeTransferred())/((double)GetFileSize())) < (((double)iPsAmountLimit)/100.0f))
			bPowerShareLimit = false;
	}
	if(iPowerShareLimit > 0)
		bPowerShareLimit = (m_nCompleteSourcesCountHi > iPowerShareLimit);
	UpdatePowerShareLimit(m_nVirtualCompleteSourcesCount<=1 || m_nCompleteSourcesCountHi<=GetPartCount(), m_nCompleteSourcesCountHi==1 && m_nVirtualCompleteSourcesCount==0 && iCompleteSourcesCountInfoReceived>1,bPowerShareLimit);
	// <== PowerShare [ZZ/MorphXT] - Stulle
	if (theApp.emuledlg->sharedfileswnd->m_hWnd)
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
}

void CKnownFile::AddUploadingClient(CUpDownClient* client){
	POSITION pos = m_ClientUploadList.Find(client); // to be sure
	if(pos == NULL){
		m_ClientUploadList.AddTail(client);
		//Xman done by see on uploadqueue
		/*
		UpdateAutoUpPriority();
		*/
		//Xman end
	}
}

void CKnownFile::RemoveUploadingClient(CUpDownClient* client){
	POSITION pos = m_ClientUploadList.Find(client); // to be sure
	if(pos != NULL){
		m_ClientUploadList.RemoveAt(pos);
		//Xman done by see on uploadqueue
		/*
		UpdateAutoUpPriority();
		*/
		//Xman end
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
	m_verifiedFileType = FILETYPE_UNKNOWN;

	wordlist.clear();
	if(m_pCollection)
	{
		CString sKeyWords;
		sKeyWords.Format(_T("%s %s"), m_pCollection->GetCollectionAuthorKeyString(), GetFileName());
		Kademlia::CSearchManager::GetWords(sKeyWords, &wordlist);
	}
	else
		Kademlia::CSearchManager::GetWords(GetFileName(), &wordlist);

	if (pFile && pFile == this)
		theApp.sharedfiles->AddKeywords(this);
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
	__int64 llFileSize = _filelengthi64(_fileno(file));
	if ((uint64)llFileSize > MAX_EMULE_FILE_SIZE){
		if (llFileSize == -1i64)
			LogError(_T("Failed to hash file \"%s\" - %s"), strFilePath, _tcserror(errno));
		else
			LogError(_T("Skipped hashing of file \"%s\" - File size exceeds limit."), strFilePath);
		fclose(file);
		return false; // not supported by network
	}
	SetFileSize((uint64)llFileSize);

	// we are reading the file data later in 8K blocks, adjust the internal file stream buffer accordingly
	setvbuf(file, NULL, _IOFBF, 1024*8*2);

	m_AvailPartFrequency.SetSize(GetPartCount());
	for (UINT i = 0; i < GetPartCount();i++)
		m_AvailPartFrequency[i] = 0;
	
	// create hashset
	CAICHRecoveryHashSet cAICHHashSet(this, m_nFileSize);
	uint64 togo = m_nFileSize;
	UINT hashcount;
	for (hashcount = 0; togo >= PARTSIZE; )
	{
		CAICHHashTree* pBlockAICHHashTree = cAICHHashSet.m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, PARTSIZE);
		ASSERT( pBlockAICHHashTree != NULL );

		uchar* newhash = new uchar[16];
		//Xman Nice Hash
		/*
		if (!CreateHash(file, PARTSIZE, newhash, pBlockAICHHashTree)) {
		*/
		if (!CreateHash(file, PARTSIZE, newhash, pBlockAICHHashTree, true)) {
		//Xman end
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

		//Xman Progress Hash (O2)
		if(GetPartCount()>0) //just to be sure
		{
			CString pourcent;
			pourcent.Format(_T("Hashing: %d%% - %s") ,(hashcount+1)*100/GetPartCount(),in_filename);		
			if (theApp.emuledlg->statusbar->m_hWnd) theApp.emuledlg->statusbar->SetText( pourcent ,0,0);
		}
		//Xman end

		m_FileIdentifier.GetRawMD4HashSet().Add(newhash);
		togo -= PARTSIZE;
		hashcount++;

		if (pvProgressParam && theApp.emuledlg && theApp.emuledlg->IsRunning()){
			ASSERT( ((CKnownFile*)pvProgressParam)->IsKindOf(RUNTIME_CLASS(CKnownFile)) );
			ASSERT( ((CKnownFile*)pvProgressParam)->GetFileSize() == GetFileSize() );
			UINT uProgress = (UINT)(uint64)(((uint64)(GetFileSize() - togo) * 100) / GetFileSize());
			ASSERT( uProgress <= 100 );
			VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pvProgressParam) );
		}
	}

	CAICHHashTree* pBlockAICHHashTree;
	if (togo == 0)
		pBlockAICHHashTree = NULL; // sha hashtree doesnt takes hash of 0-sized data
	else{
		pBlockAICHHashTree = cAICHHashSet.m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, togo);
		ASSERT( pBlockAICHHashTree != NULL );
	}
	
	uchar* lasthash = new uchar[16];
	md4clr(lasthash);
	//Xman Nice Hash
	/*
	if (!CreateHash(file, togo, lasthash, pBlockAICHHashTree)) {
	*/
	if (!CreateHash(file, togo, lasthash, pBlockAICHHashTree, true)) {
	//Xman end
		LogError(_T("Failed to hash file \"%s\" - %s"), strFilePath, _tcserror(errno));
		fclose(file);
		delete[] lasthash;
		return false;
	}
	
	cAICHHashSet.ReCalculateHash(false);
	if (cAICHHashSet.VerifyHashTree(true))
	{
		cAICHHashSet.SetStatus(AICH_HASHSETCOMPLETE);
		m_FileIdentifier.SetAICHHash(cAICHHashSet.GetMasterHash());
		if (!m_FileIdentifier.SetAICHHashSet(cAICHHashSet))
		{
			ASSERT( false );
			DebugLogError(_T("CreateFromFile() - failed to create AICH PartHashSet out of RecoveryHashSet - %s"), GetFileName());
		}
		if (!cAICHHashSet.SaveHashSet())
			LogError(LOG_STATUSBAR, GetResString(IDS_SAVEACFAILED));
		else
			SetAICHRecoverHashSetAvailable(true);
	}
	else{
		// now something went pretty wrong
		DebugLogError(LOG_STATUSBAR, _T("Failed to calculate AICH Hashset from file %s"), GetFileName());
	}

	if (!hashcount){
		m_FileIdentifier.SetMD4Hash(lasthash);
		delete[] lasthash;
	} 
	else {
		m_FileIdentifier.GetRawMD4HashSet().Add(lasthash);
		//Xman Nice Hash
		/*
		m_FileIdentifier.CalculateMD4HashByHashSet(false);
		*/
		m_FileIdentifier.CalculateMD4HashByHashSet(false, true, true);
		//Xman end
	}

	if (pvProgressParam && theApp.emuledlg && theApp.emuledlg->IsRunning()){
		ASSERT( ((CKnownFile*)pvProgressParam)->IsKindOf(RUNTIME_CLASS(CKnownFile)) );
		ASSERT( ((CKnownFile*)pvProgressParam)->GetFileSize() == GetFileSize() );
		UINT uProgress = 100;
		ASSERT( uProgress <= 100 );
		VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pvProgressParam) );
	}

	// set lastwrite date
	struct _stat fileinfo;
	if (_fstat(file->_file, &fileinfo) == 0){
		// ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		/*
		m_tUtcLastModified = fileinfo.st_mtime;
		*/
		m_tUtcLastModified = (uint32)fileinfo.st_mtime;
		// <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strFilePath);
	}

	fclose(file);
	file = NULL;

	// Add filetags
	UpdateMetaDataTags();

	UpdatePartsInfo();

	//Xman Progress Hash (O2)
	AddLogLine(true, GetResString(IDS_PROGRESSHASHDONE), GetFilePath()  );
	//Xman end

	return true;	
}

bool CKnownFile::CreateAICHHashSetOnly()
{
	ASSERT( !IsPartFile() );
	
	FILE* file = _tfsopen(GetFilePath(), _T("rbS"), _SH_DENYNO); // can not use _SH_DENYWR because we may access a completing part file
	if (!file){
		LogError(GetResString(IDS_ERR_FILEOPEN) + _T(" - %s"), GetFilePath(), _T(""), _tcserror(errno));
		return false;
	}
	// we are reading the file data later in 8K blocks, adjust the internal file stream buffer accordingly
	setvbuf(file, NULL, _IOFBF, 1024*8*2);

	// create aichhashset
	CAICHRecoveryHashSet cAICHHashSet(this, m_nFileSize);
	uint64 togo = m_nFileSize;
	UINT hashcount;
	for (hashcount = 0; togo >= PARTSIZE; )
	{
		CAICHHashTree* pBlockAICHHashTree = cAICHHashSet.m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, PARTSIZE);
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
		CAICHHashTree* pBlockAICHHashTree = cAICHHashSet.m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, togo);
		ASSERT( pBlockAICHHashTree != NULL );
		if (!CreateHash(file, togo, NULL, pBlockAICHHashTree)) {
			LogError(_T("Failed to hash file \"%s\" - %s"), GetFilePath(), _tcserror(errno));
			fclose(file);
			return false;
		}
	}

	cAICHHashSet.ReCalculateHash(false);
	if (cAICHHashSet.VerifyHashTree(true))
	{
		cAICHHashSet.SetStatus(AICH_HASHSETCOMPLETE);
		if (m_FileIdentifier.HasAICHHash() && m_FileIdentifier.GetAICHHash() != cAICHHashSet.GetMasterHash())
			theApp.knownfiles->AICHHashChanged(&m_FileIdentifier.GetAICHHash(), cAICHHashSet.GetMasterHash(), this);
		else
			theApp.knownfiles->AICHHashChanged(NULL, cAICHHashSet.GetMasterHash(), this);
		m_FileIdentifier.SetAICHHash(cAICHHashSet.GetMasterHash());
		if (!m_FileIdentifier.SetAICHHashSet(cAICHHashSet))
		{
			ASSERT( false );
			DebugLogError(_T("CreateAICHHashSetOnly() - failed to create AICH PartHashSet out of RecoveryHashSet - %s"), GetFileName());
		}
		if (!cAICHHashSet.SaveHashSet())
			LogError(LOG_STATUSBAR, GetResString(IDS_SAVEACFAILED));
		else
			SetAICHRecoverHashSetAvailable(true);
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

	// File size       Data parts      ED2K parts      ED2K part hashs		AICH part hashs
	// -------------------------------------------------------------------------------------------
	// 1..PARTSIZE-1   1               1               0(!)					0 (!)
	// PARTSIZE        1               2(!)            2(!)					0 (!)
	// PARTSIZE+1      2               2               2					2
	// PARTSIZE*2      2               3(!)            3(!)					2
	// PARTSIZE*2+1    3               3               3					3

	if (nFileSize == (uint64)0){
		ASSERT(0);
		m_iPartCount = 0;
		m_iED2KPartCount = 0;
		return;
	}

	// nr. of data parts
	ASSERT( (uint64)(((uint64)nFileSize + (PARTSIZE - 1)) / PARTSIZE) <= (UINT)USHRT_MAX );
	m_iPartCount = (uint16)(((uint64)nFileSize + (PARTSIZE - 1)) / PARTSIZE);

	// nr. of parts to be used with OP_FILESTATUS
	m_iED2KPartCount = (uint16)((uint64)nFileSize / PARTSIZE + 1);
}

bool CKnownFile::LoadTagsFromFile(CFileDataIO* file)
{
	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	CMap<UINT,UINT,uint64,uint64> spread_start_map;
	CMap<UINT,UINT,uint64,uint64> spread_end_map;
	CMap<UINT,UINT,uint64,uint64> spread_count_map;
	// <== Spread bars [Slugfiller/MorphXT] - Stulle

	UINT tagcount = file->ReadUInt32();
	bool bHadAICHHashSetTag = false;
	for (UINT j = 0; j < tagcount; j++){
		CTag* newtag = new CTag(file, false);
		switch (newtag->GetNameID()){
			case FT_FILENAME:{
				ASSERT( newtag->IsStr() );
				if (newtag->IsStr()){
					if (GetFileName().IsEmpty())
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
					for (UINT i = 0; i < GetPartCount();i++)
						m_AvailPartFrequency[i] = 0;
				}
				delete newtag;
				break;
			}
			case FT_ATTRANSFERRED:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.SetAllTimeTransferred(newtag->GetInt());
				delete newtag;
				break;
			}
			case FT_ATTRANSFERREDHI:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.SetAllTimeTransferred(((uint64)newtag->GetInt() << 32) | (UINT)statistic.GetAllTimeTransferred());
				delete newtag;
				break;
			}
			case FT_ATREQUESTED:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.SetAllTimeRequests(newtag->GetInt());
				delete newtag;
				break;
			}
 			case FT_ATACCEPTED:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.SetAllTimeAccepts(newtag->GetInt());
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
						//Xman PowerRelease
						/*
						if (m_iUpPriority != PR_VERYLOW && m_iUpPriority != PR_LOW && m_iUpPriority != PR_NORMAL && m_iUpPriority != PR_HIGH && m_iUpPriority != PR_VERYHIGH)
						*/
						if (m_iUpPriority != PR_VERYLOW && m_iUpPriority != PR_LOW && m_iUpPriority != PR_NORMAL && m_iUpPriority != PR_HIGH && m_iUpPriority != PR_VERYHIGH && m_iUpPriority != PR_POWER)
						//Xman end
							m_iUpPriority = PR_NORMAL;
						m_bAutoUpPriority = false;
					}
				}
				delete newtag;
				break;
			}
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
				//				0	untrusted meta data which was received via search results
				//				1	trusted meta data, Unicode (strings where not stored correctly)
				//				2	0.49c: trusted meta data, Unicode
				// Bits 31-4: Reserved
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					m_uMetaDataVer = newtag->GetInt() & 0x0F;
				delete newtag;
				break;
			// old tags: as long as they are not needed, take the chance to purge them
			case FT_PERMISSIONS:
				ASSERT( newtag->IsInt() );
				delete newtag;
				break;
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
					m_FileIdentifier.SetAICHHash(hash);
				else
					ASSERT( false );
				delete newtag;
				break;
			}
			case FT_LASTSHARED:
				if (newtag->IsInt())
					m_timeLastSeen = newtag->GetInt();
				else
					ASSERT( false );
				delete newtag;
				break;
			case FT_AICHHASHSET:
				if (newtag->IsBlob())
				{
					CSafeMemFile aichHashSetFile(newtag->GetBlob(), newtag->GetBlobSize());
					m_FileIdentifier.LoadAICHHashsetFromFile(&aichHashSetFile, false);
					aichHashSetFile.Detach();
					bHadAICHHashSetTag = true;
				}
				else
					ASSERT( false );
				delete newtag;
				break;

			//Xman advanced upload-priority
			case FT_NOTCOUNTEDTRANSFERREDLOW:
			{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.m_unotcountedtransferred = newtag->GetInt();
				delete newtag;
				break;
			}
			case FT_NOTCOUNTEDTRANSFERREDHIGH:
			{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.m_unotcountedtransferred = ((uint64)newtag->GetInt() << 32) | (UINT)statistic.m_unotcountedtransferred;
				delete newtag;
				break;
			}
			case FT_LASTDATAUPDATE:
			{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
					statistic.m_tlastdataupdate = newtag->GetInt();
				delete newtag;
				break;
			}
			//Xman end

			default:
				// ==> Take care of corrupted tags [Mighty Knife] - Stulle
				if(!newtag->GetNameID() && newtag->IsInt64(true) && newtag->GetName()){
				// <==  Take care of corrupted tags [Mighty Knife] - Stulle
					// ==> Spread bars [Slugfiller/MorphXT] - Stulle
					UINT spreadkey = atoi(&newtag->GetName()[1]);
					if (newtag->GetName()[0] == FT_SPREADSTART)
						spread_start_map.SetAt(spreadkey, newtag->GetInt64());
					else if (newtag->GetName()[0] == FT_SPREADEND)
						spread_end_map.SetAt(spreadkey, newtag->GetInt64());
					else if (newtag->GetName()[0] == FT_SPREADCOUNT)
						spread_count_map.SetAt(spreadkey, newtag->GetInt64());
					// <== Spread bars [Slugfiller/MorphXT] - Stulle
					// ==> PowerShare [ZZ/MorphXT] - Stulle
					else if(CmpED2KTagName(newtag->GetName(), FT_POWERSHARE) == 0)
						SetPowerShared((newtag->GetInt()<=3)?newtag->GetInt():-1);
					else if(CmpED2KTagName(newtag->GetName(), FT_POWERSHARE_LIMIT) == 0)
						SetPowerShareLimit((newtag->GetInt()<=200)?newtag->GetInt():-1);
					// <== PowerShare [ZZ/MorphXT] - Stulle
					// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
					else if(CmpED2KTagName(newtag->GetName(), FT_HIDEOS) == 0)
						SetHideOS((newtag->GetInt()<=10)?newtag->GetInt():-1);
					else if(CmpED2KTagName(newtag->GetName(), FT_SELECTIVE_CHUNK) == 0)
						SetSelectiveChunk(newtag->GetInt()<=1?newtag->GetInt():-1);
					else if(CmpED2KTagName(newtag->GetName(), FT_SHAREONLYTHENEED) == 0)
						SetShareOnlyTheNeed(newtag->GetInt()<=1?newtag->GetInt():-1);
					// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
					// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
					else if(CmpED2KTagName(newtag->GetName(), FT_PS_AMOUNT_LIMIT) == 0)
						SetPsAmountLimit(newtag->GetInt()<=MAX_PS_AMOUNT_LIMIT?newtag->GetInt():-1);
					// <== Limit PS by amount of data uploaded [Stulle] - Stulle
					// ==>  Take care of corrupted tags [Mighty Knife] - Stulle
					delete newtag;
					break;
				}
				// <==  Take care of corrupted tags [Mighty Knife] - Stulle

				ConvertED2KTag(newtag);
				if (newtag)
					taglist.Add(newtag);
		}
	}
	if (bHadAICHHashSetTag)
	{
		if (!m_FileIdentifier.VerifyAICHHashSet())
			DebugLogError(_T("Failed to load AICH Part HashSet for file %s"), GetFileName());
		//else
		//	DebugLog(_T("Succeeded to load AICH Part HashSet for file %s"), GetFileName());
	}

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	// Now to flush the map into the list
	for (POSITION pos = spread_start_map.GetStartPosition(); pos != NULL; ){
		UINT spreadkey;
		uint64 spread_start;
		uint64 spread_end;
		uint64 spread_count;
		spread_start_map.GetNextAssoc(pos, spreadkey, spread_start);
		if (!spread_end_map.Lookup(spreadkey, spread_end))
			continue;
		if (!spread_count_map.Lookup(spreadkey, spread_count))
			continue;
		if (!spread_count || spread_start >= spread_end)
			continue;
		statistic.AddBlockTransferred(spread_start, spread_end, spread_count);	// All tags accounted for
	}
	// <== Spread bars [Slugfiller/MorphXT] - Stulle

	// 05-Jän-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
	// the chance to clean any available meta data tags and provide only tags which were determined by us.
	// It's a brute force method, but that wrong meta data is driving me crazy because wrong meta data is even worse than
	// missing meta data.
	if (m_uMetaDataVer == 0)
		RemoveMetaDataTags();
	else if (m_uMetaDataVer == 1)
	{
		// Meta data tags v1 did not store Unicode strings correctly.
		// Remove broken Unicode string meta data tags from v1, but keep the integer tags.
		RemoveBrokenUnicodeMetaDataTags();
		m_uMetaDataVer = META_DATA_VER;
	}

	//Xman // SLUGFILLER: SafeHash - Must have a filesize tag
	/*
	return true;
	*/
	return m_nFileSize!=(uint64)0;
	//Xman end
}

bool CKnownFile::LoadDateFromFile(CFileDataIO* file){
	m_tUtcLastModified = file->ReadUInt32();
	return true;
}

bool CKnownFile::LoadFromFile(CFileDataIO* file){
	// SLUGFILLER: SafeHash - load first, verify later
	bool ret1 = LoadDateFromFile(file);
	bool ret2 = m_FileIdentifier.LoadMD4HashsetFromFile(file, false);
	bool ret3 = LoadTagsFromFile(file);
	UpdatePartsInfo();
	return ret1 && ret2 && ret3 && m_FileIdentifier.HasExpectedMD4HashCount();// Final hash-count verification, needs to be done after the tags are loaded.
	// SLUGFILLER: SafeHash
}

bool CKnownFile::WriteToFile(CFileDataIO* file)
{
	// date
	file->WriteUInt32(m_tUtcLastModified);

	// hashset
	m_FileIdentifier.WriteMD4HashsetToFile(file);

	uint32 uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

	CTag nametag(FT_FILENAME, GetFileName());
	nametag.WriteTagToFile(file, utf8strOptBOM);
	uTagCount++;
	
	CTag sizetag(FT_FILESIZE, m_nFileSize, IsLargeFile());
	sizetag.WriteTagToFile(file);
	uTagCount++;

	//AICH Filehash
	if (m_FileIdentifier.HasAICHHash())
	{
		CTag aichtag(FT_AICH_HASH, m_FileIdentifier.GetAICHHash().GetString());
		aichtag.WriteTagToFile(file);
		uTagCount++;
	}

	// last shared
	static bool sDbgWarnedOnZero = false;
	if (!sDbgWarnedOnZero && m_timeLastSeen == 0)
	{
		DebugLog(_T("Unknown last seen date on stored file(s), upgrading from old version?"));
		sDbgWarnedOnZero = true;
	}
	ASSERT( m_timeLastSeen <= time(NULL) );
	time_t timeLastShared = (m_timeLastSeen > 0 && m_timeLastSeen <= time(NULL)) ? m_timeLastSeen : time(NULL);
	CTag lastSharedTag(FT_LASTSHARED, (uint32)timeLastShared);
	lastSharedTag.WriteTagToFile(file);
	uTagCount++;

	if (!ShouldPartiallyPurgeFile())
	{
		// those tags are no longer stored for long time not seen (shared) known files to tidy up known.met and known2.met
		
		// AICH Part HashSet
		// no point in permanently storing the AICH part hashset if we need to rehash the file anyway to fetch the full recovery hashset
		// the tag will make the known.met incompatible with emule version prior 0.44a - but that one is nearly 6 years old 
		if (m_FileIdentifier.HasAICHHash() && m_FileIdentifier.HasExpectedAICHHashCount())
		{
			uint32 nAICHHashSetSize = (CAICHHash::GetHashSize() * (m_FileIdentifier.GetAvailableAICHPartHashCount() + 1)) + 2;
			BYTE* pHashBuffer = new BYTE[nAICHHashSetSize];
			CSafeMemFile hashSetFile(pHashBuffer, nAICHHashSetSize);
			bool bWriteHashSet = true;
			try
			{
				m_FileIdentifier.WriteAICHHashsetToFile(&hashSetFile);
			}
			catch (CFileException* pError)
			{
				ASSERT( false );
				DebugLogError(_T("Memfile Error while storing AICH Part HashSet"));
				bWriteHashSet = false;
				delete[] hashSetFile.Detach();
				pError->Delete();
			}
			if (bWriteHashSet)
			{
				CTag tagAICHHashSet(FT_AICHHASHSET, hashSetFile.Detach(), nAICHHashSetSize);
				tagAICHHashSet.WriteTagToFile(file);
				uTagCount++;
			}
		}

		// statistic
		if (statistic.GetAllTimeTransferred()){
			CTag attag1(FT_ATTRANSFERRED, (uint32)statistic.GetAllTimeTransferred());
			attag1.WriteTagToFile(file);
			uTagCount++;
			
			CTag attag4(FT_ATTRANSFERREDHI, (uint32)(statistic.GetAllTimeTransferred() >> 32));
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
			//				0	untrusted meta data which was received via search results
			//				1	trusted meta data, Unicode (strings where not stored correctly)
			//				2	0.49c: trusted meta data, Unicode
			// Bits 31-4: Reserved
			ASSERT( m_uMetaDataVer <= 0x0F );
			uint32 uFlags = m_uMetaDataVer & 0x0F;
			CTag tagFlags(FT_FLAGS, uFlags);
			tagFlags.WriteTagToFile(file);
			uTagCount++;
		}

		//Xman advanced upload-priority
		if (statistic.m_unotcountedtransferred)
		{
			CTag stag1(FT_NOTCOUNTEDTRANSFERREDLOW, (uint32)statistic.m_unotcountedtransferred);
			stag1.WriteTagToFile(file);
			uTagCount++;

			CTag stag2(FT_NOTCOUNTEDTRANSFERREDHIGH, (uint32)(statistic.m_unotcountedtransferred >> 32));
			stag2.WriteTagToFile(file);
			uTagCount++;
		}

		if (statistic.m_tlastdataupdate!=0)
		{
			CTag stag1(FT_LASTDATAUPDATE, statistic.m_tlastdataupdate);
			stag1.WriteTagToFile(file);
			uTagCount++;
		}
		//Xman end

		// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		if (GetHideOS()>=0){
			CTag hideostag(FT_HIDEOS, GetHideOS());
			hideostag.WriteTagToFile(file);
			uTagCount++;
		}
		if (GetSelectiveChunk()>=0){
			CTag selectivechunktag(FT_SELECTIVE_CHUNK, GetSelectiveChunk());
			selectivechunktag.WriteTagToFile(file);
			uTagCount++;
		}
		if (GetShareOnlyTheNeed()>=0){
			CTag shareonlytheneedtag(FT_SHAREONLYTHENEED, GetShareOnlyTheNeed());
			shareonlytheneedtag.WriteTagToFile(file);
			uTagCount++;
		}
		// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
		// ==> Limit PS by amount of data uploaded [Stulle] - Stulle
		if (GetPsAmountLimit()>=0){
			CTag psamountlimittag(FT_PS_AMOUNT_LIMIT, GetPsAmountLimit());
			psamountlimittag.WriteTagToFile(file);
			uTagCount++;
		}
		// <== Limit PS by amount of data uploaded [Stulle] - Stulle
		// ==> PowerShare [ZZ/MorphXT] - Stulle
		if (GetPowerSharedMode()>=0){
			CTag powersharetag(FT_POWERSHARE, GetPowerSharedMode());
			powersharetag.WriteTagToFile(file);
			uTagCount++;
		}
		if (GetPowerShareLimit()>=0){
			CTag powersharelimittag(FT_POWERSHARE_LIMIT, GetPowerShareLimit());
			powersharelimittag.WriteTagToFile(file);
			uTagCount++;
		}
		// <== PowerShare [ZZ/MorphXT] - Stulle
		// ==> Spread bars [Slugfiller/MorphXT] - Stulle
		if(thePrefs.GetSpreadbarSetStatus())
		{
			char namebuffer[10];
			char* number = &namebuffer[1];
			UINT i_pos = 0;
			if (IsLargeFile()) {
				uint64 hideOS = GetHideOS()>=0?GetHideOS():thePrefs.GetHideOvershares();
				for (POSITION pos = statistic.spreadlist.GetHeadPosition(); pos; ){
					uint64 count = statistic.spreadlist.GetValueAt(pos);
					if (!count) {
						statistic.spreadlist.GetNext(pos);
						continue;
					}
					uint64 start = statistic.spreadlist.GetKeyAt(pos);
					statistic.spreadlist.GetNext(pos);
					ASSERT(pos != NULL);	// Last value should always be 0
					uint64 end = statistic.spreadlist.GetKeyAt(pos);
					//MORPH - Smooth sample
					if (end - start < EMBLOCKSIZE && count > hideOS)
						continue;
					//MORPH - Smooth sample
					itoa(i_pos,number,10);
					namebuffer[0] = FT_SPREADSTART;
					CTag(namebuffer,start,true).WriteTagToFile(file);
					namebuffer[0] = FT_SPREADEND;
					CTag(namebuffer,end,true).WriteTagToFile(file);
					namebuffer[0] = FT_SPREADCOUNT;
					CTag(namebuffer,count,true).WriteTagToFile(file);
					uTagCount+=3;
					i_pos++;
				}
			} else {
				uint32 hideOS = GetHideOS()>=0?GetHideOS():thePrefs.GetHideOvershares();
				for (POSITION pos = statistic.spreadlist.GetHeadPosition(); pos; ){
					uint32 count = (uint32)statistic.spreadlist.GetValueAt(pos);
					if (!count) {
						statistic.spreadlist.GetNext(pos);
						continue;
					}
					uint32 start = (uint32)statistic.spreadlist.GetKeyAt(pos);
					statistic.spreadlist.GetNext(pos);
					ASSERT(pos != NULL);	// Last value should always be 0
					uint32 end = (uint32)statistic.spreadlist.GetKeyAt(pos);
					//MORPH - Smooth sample
					if (end - start < EMBLOCKSIZE && count > hideOS)
						continue;
					//MORPH - Smooth sample
					itoa(i_pos,number,10);
					namebuffer[0] = FT_SPREADSTART;
					CTag(namebuffer,start).WriteTagToFile(file);
					namebuffer[0] = FT_SPREADEND;
					CTag(namebuffer,end).WriteTagToFile(file);
					namebuffer[0] = FT_SPREADCOUNT;
					CTag(namebuffer,count).WriteTagToFile(file);
					uTagCount+=3;
					i_pos++;
				}
			}
		}
		// <== Spread bars [Slugfiller/MorphXT] - Stulle

		// other tags
		for (int j = 0; j < taglist.GetCount(); j++){
			if (taglist[j]->IsStr() || taglist[j]->IsInt()){
				taglist[j]->WriteTagToFile(file, utf8strOptBOM);
				uTagCount++;
			}
		}
	}

	file->Seek(uTagCountFilePos, CFile::begin);
	file->WriteUInt32(uTagCount);
	file->Seek(0, CFile::end);

	return true;
}

//Xman Nice Hash
/*
void CKnownFile::CreateHash(CFile* pFile, uint64 Length, uchar* pMd4HashOut, CAICHHashTree* pShaHashOut)
*/
void CKnownFile::CreateHash(CFile* pFile, uint64 Length, uchar* pMd4HashOut, CAICHHashTree* pShaHashOut, bool slowdown)
//Xman end
{
	ASSERT( pFile != NULL );
	ASSERT( pMd4HashOut != NULL || pShaHashOut != NULL );
	//Xman
	CSingleLock sLock1(&(theApp.hashing_mut), TRUE);	// SLUGFILLER: SafeHash - only one chunk-hash at a time

	uint64  Required = Length;
	uchar   X[64*128];
	uint64	posCurrentEMBlock = 0;
	uint64	nIACHPos = 0;
	CMD4 md4;
	CAICHHashAlgo* pHashAlg = NULL;
	if (pShaHashOut != NULL)
		pHashAlg = CAICHRecoveryHashSet::GetNewHashAlgo();

	//Xman Nice Hash
	uint32 timeStart = ::GetTickCount();  
	//Xman end

	while (Required >= 64){
        uint32 len; 
        if ((Required / 64) > sizeof(X)/(64 * sizeof(X[0]))) 
			len = sizeof(X)/(64 * sizeof(X[0]));
		else
			len = (uint32)Required / 64;
		pFile->Read(X, len*64);

		// SHA hash needs 180KB blocks
		if (pShaHashOut != NULL && pHashAlg != NULL){
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
		//Xman Nice Hash
		if(slowdown && ::GetTickCount() - timeStart >= 100)   
		{       
			Sleep(30);       
			timeStart = ::GetTickCount();       
		}       
		//Xman end
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
	sLock1.Unlock();// SLUGFILLER: SafeHash - only one chunk-hash at a time
}

//Xman Nice Hash
/*
bool CKnownFile::CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut)
*/
bool CKnownFile::CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut, bool slowdown)
//Xman end
{
	bool bResult = false;
	CStdioFile file(fp);
	try
	{
		//Xman Nice Hash
		/*
		CreateHash(&file, uSize, pucHash, pShaHashOut);
		*/
		CreateHash(&file, uSize, pucHash, pShaHashOut, slowdown);
		//Xman end
		bResult = true;
	}
	catch(CFileException* ex)
	{
		ex->Delete();
	}
	return bResult;
}

//Xman Nice Hash
/*
bool CKnownFile::CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut)
*/
bool CKnownFile::CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut, bool slowdown)
//Xman end
{
	bool bResult = false;
	CMemFile file(const_cast<uchar*>(pucData), uSize);
	try
	{
		//Xman Nice Hash
		/*
		CreateHash(&file, uSize, pucHash, pShaHashOut);
		*/
		CreateHash(&file, uSize, pucHash, pShaHashOut, slowdown);
		//Xman end
		bResult = true;
	}
	catch(CFileException* ex)
	{
		ex->Delete();
	}
	return bResult;
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

	// check whether client has either no download status at all or a download status which is valid for this file
	if (   !(forClient->GetUpPartCount()==0 && forClient->GetUpPartStatus()==NULL)
		&& !(forClient->GetUpPartCount()==GetPartCount() && forClient->GetUpPartStatus()!=NULL)) {
		// should never happen
		DEBUG_ONLY( DebugLogError(_T("*** %hs - part count (%u) of client (%s) does not match part count (%u) of file \"%s\""), __FUNCTION__, forClient->GetUpPartCount(), forClient->DbgGetClientInfo(), GetPartCount(), GetFileName()) );
		ASSERT(0);
		return NULL;
	}

	CSafeMemFile data(1024);
	
	uint8 byUsedVersion;
	bool bIsSX2Packet;
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
	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		/*const*/ CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		
		// some rare issue seen in a crashdumps, hopefully fixed already, but to be sure we double check here
		// TODO: remove check next version, as it uses ressources and shouldn't be necessary
		if (!theApp.clientlist->IsValidClient(cur_src))
		{
#ifdef _BETA
			throw new CUserException();
#endif
			ASSERT( false );
			DebugLogError(_T("Invalid client in uploading list for file %s"), GetFileName());
			return NULL;
		}

		if (cur_src->HasLowID() || cur_src == forClient || !(cur_src->GetUploadState() == US_UPLOADING || cur_src->GetUploadState() == US_ONUPLOADQUEUE))
			continue;
		if (!cur_src->IsEd2kClient())
			continue;

		bool bNeeded = false;
		const uint8* rcvstatus = forClient->GetUpPartStatus();
		if (rcvstatus)
		{
			ASSERT( forClient->GetUpPartCount() == GetPartCount() );
			const uint8* srcstatus = cur_src->GetUpPartStatus();
			if (srcstatus)
			{
				ASSERT( cur_src->GetUpPartCount() == GetPartCount() );
				if (cur_src->GetUpPartCount() == forClient->GetUpPartCount())
				{
					for (UINT x = 0; x < GetPartCount(); x++)
					{
						// ==> See chunk that we hide [SiRoB] - Stulle
						/*
						if (srcstatus[x] && !rcvstatus[x])
						*/
						if (srcstatus[x]&SC_AVAILABLE && !(rcvstatus[x]&SC_AVAILABLE))
						// <== See chunk that we hide [SiRoB] - Stulle
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
						DEBUG_ONLY( DebugLogError(_T("*** %hs - found source (%s) with wrong part count (%u) attached to file \"%s\" (partcount=%u)"), __FUNCTION__, cur_src->DbgGetClientInfo(), cur_src->GetUpPartCount(), GetFileName(), GetPartCount()));
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
			ASSERT( forClient->GetUpPartCount() == 0 );
			TRACE(_T("%hs, requesting client has no chunk status - %s"), __FUNCTION__, forClient->DbgGetClientInfo());
			// remote client does not support upload chunk status, search sources which have at least one complete part
			// we could even sort the list of sources by available chunks to return as much sources as possible which
			// have the most available chunks. but this could be a noticeable performance problem.
			const uint8* srcstatus = cur_src->GetUpPartStatus();
			if (srcstatus)
			{
				ASSERT( cur_src->GetUpPartCount() == GetPartCount() );
				for (UINT x = 0; x < GetPartCount(); x++ )
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
			if (nCount > 500)
				break;
		}
	}
	TRACE(_T("%hs: Out of %u clients, %u had no valid chunk status\n"), __FUNCTION__, m_ClientUploadList.GetCount(), cDbgNoSrc);
	if (!nCount)
		return 0;
	data.Seek(bIsSX2Packet ? 17 : 16, SEEK_SET);
	data.WriteUInt16((uint16)nCount);

	Packet* result = new Packet(&data, OP_EMULEPROT);
	result->opcode = bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES;
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

//Xman see on uploadqueue does this job now:
//Xman changed Auto-Prios
/*
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
*/
void CKnownFile::UpdateAutoUpPriority(){
	if( !IsAutoUpPriority() )
		return;
	
	//Xman advanced upload-priority
	if(thePrefs.UseAdvancedAutoPtio())
		return; 
	//Xman end

	//Xman : if we use the multiqueue, high uploadprio will be useless because
	//clients which ask for a rare file already are preferred
	// Maella -One-queue-per-file- (idea bloodymad)
	if(thePrefs.GetEnableMultiQueue())
	{
		if( GetUpPriority() != PR_NORMAL ){
			SetUpPriority( PR_NORMAL,false );
			//theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
		}
		return;
	}
	//Maella end
	if ( GetOnUploadqueue() > 100 ){
		if( GetUpPriority() != PR_LOW ){
			SetUpPriority( PR_LOW,false );
			//theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
		}
		return;
	}
	if ( GetOnUploadqueue() > 15 ){
		if( GetUpPriority() != PR_NORMAL ){
			SetUpPriority( PR_NORMAL,false );
			//theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
		}
		return;
	}
	if( GetUpPriority() != PR_HIGH){
		SetUpPriority( PR_HIGH,false );
		//theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
	}
}
//Xman end

void CKnownFile::SetUpPriority(uint8 iNewUpPriority, bool bSave)
{
	//Xman advanced upload-priority
	/*
	m_iUpPriority = iNewUpPriority;
	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH );
	*/
	if(m_iUpPriority!=iNewUpPriority)
	{
		m_iUpPriority = iNewUpPriority;
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this,true);
	}
	else
		m_iUpPriority = iNewUpPriority;

	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH || m_iUpPriority == PR_POWER ); //Xman PowerRelease
	//Xman end

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

void CKnownFile::RemoveMetaDataTags(UINT uTagType)
{
	static const struct
	{
		uint8	nID;
		uint8	nType;
	} _aEmuleMetaTags[] = 
	{
		{ FT_MEDIA_ARTIST,  TAGTYPE_STRING },
		{ FT_MEDIA_ALBUM,   TAGTYPE_STRING },
		{ FT_MEDIA_TITLE,   TAGTYPE_STRING },
		{ FT_MEDIA_LENGTH,  TAGTYPE_UINT32 },
		{ FT_MEDIA_BITRATE, TAGTYPE_UINT32 },
		{ FT_MEDIA_CODEC,   TAGTYPE_STRING }
	};

	// 05-Jän-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
	// the chance to clean any available meta data tags and provide only tags which were determined by us.
	// Remove all meta tags. Never ever trust the meta tags received from other clients or servers.
	for (int j = 0; j < _countof(_aEmuleMetaTags); j++)
	{
		if (uTagType == 0 || (uTagType == _aEmuleMetaTags[j].nType))
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
	}

	m_uMetaDataVer = 0;
}

void CKnownFile::RemoveBrokenUnicodeMetaDataTags()
{
	static const struct
	{
		uint8	nID;
		uint8	nType;
	} _aEmuleMetaTags[] = 
	{
		{ FT_MEDIA_ARTIST,  TAGTYPE_STRING },
		{ FT_MEDIA_ALBUM,   TAGTYPE_STRING },
		{ FT_MEDIA_TITLE,   TAGTYPE_STRING },
		{ FT_MEDIA_CODEC,   TAGTYPE_STRING }	// This one actually contains only ASCII
	};

	for (int j = 0; j < _countof(_aEmuleMetaTags); j++)
	{
		int i = 0;
		while (i < taglist.GetSize())
		{
			// Meta data strings of older eMule versions did store Unicode strings as MBCS strings,
			// which means that - depending on the Unicode string content - particular characters
			// got lost. Unicode characters which cannot get converted into the local codepage
			// will get replaced by Windows with a '?' character. So, to estimate if we have a
			// broken Unicode string (due to the conversion between Unicode/MBCS), we search the
			// strings for '?' characters. This is not 100% perfect, as it would also give
			// false results for strings which do contain the '?' character by intention. It also
			// would give wrong results for particular characters which got mapped to ASCII chars
			// due to the conversion from Unicode->MBCS. But at least it prevents us from deleting
			// all the existing meta data strings.
			const CTag* pTag = taglist[i];
			if (   pTag->GetNameID() == _aEmuleMetaTags[j].nID
				&& pTag->IsStr()
				&& _tcschr(pTag->GetStr(), _T('?')) != NULL)
			{
				delete pTag;
				taglist.RemoveAt(i);
			}
			else
				i++;
		}
	}
}

CStringA GetED2KAudioCodec(WORD wFormatTag)
{
	CStringA strCodec(GetAudioFormatCodecId(wFormatTag));
	strCodec.Trim();
	strCodec.MakeLower();
	return strCodec;
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
	else if (biCompression == BI_JPEG)
		return "jpeg";
	else if (biCompression == BI_PNG)
		return "png";

	LPCSTR pszCompression = (LPCSTR)&biCompression;
	for (int i = 0; i < 4; i++)
	{
		if (   !__iscsym((unsigned char)pszCompression[i])
			&& pszCompression[i] != '.' 
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

SMediaInfo *GetWMMediaInfo(LPCTSTR pszFullPath)
{
#ifdef HAVE_WMSDK_H
	bool bIsWM;
	SMediaInfo *mi = new SMediaInfo;
	if (!GetWMHeaders(pszFullPath, mi, bIsWM)) {
		delete mi;
		return NULL;
	}
	return mi;
#else//HAVE_WMSDK_H
	UNREFERENCED_PARAMETER(pszFullPath);
	return NULL;
#endif//HAVE_WMSDK_H
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
	_tsplitpath(GetFileName(), NULL, NULL, NULL, szExt);
	_tcslwr(szExt);
	if (_tcscmp(szExt, _T(".mp3"))==0 || _tcscmp(szExt, _T(".mp2"))==0 || _tcscmp(szExt, _T(".mp1"))==0 || _tcscmp(szExt, _T(".mpa"))==0)
	{
		TCHAR szFullPath[MAX_PATH];
		if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(), NULL)){
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
		if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(), NULL))
		{
			SMediaInfo* mi = NULL;
			try
			{
				mi = GetRIFFMediaInfo(szFullPath);
				if (mi == NULL)
					mi = GetRMMediaInfo(szFullPath);
				if (mi == NULL)
					mi = GetWMMediaInfo(szFullPath);
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

					TruncateED2KMetaData(mi->strAlbum);
					if (!mi->strAlbum.IsEmpty()){
						CTag* pTag = new CTag(FT_MEDIA_ALBUM, mi->strAlbum);
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
	return (ED2KFT_VIDEO == GetED2KFileTypeID(GetFileName()) );
}

// function assumes that this file is shared and that any needed permission to preview exists. checks have to be done before calling! 
bool CKnownFile::GrabImage(uint8 nFramesToGrab, double dStartTime, bool bReduceColor, uint16 nMaxWidth, void* pSender)
{
	return GrabImage(GetPath() + CString(_T("\\")) + GetFileName(), nFramesToGrab,  dStartTime, bReduceColor, nMaxWidth, pSender);
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

CString CKnownFile::GetInfoSummary(bool bNoFormatCommands) const
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
	CString dbgInfo;
#ifdef _DEBUG
	dbgInfo.Format(_T("\nAICH Part HashSet: %s\nAICH Rec HashSet: %s"), m_FileIdentifier.HasExpectedAICHHashCount() ? _T("Yes") : _T("No")
		, IsAICHRecoverHashSetAvailable() ? _T("Yes") : _T("No"));
#endif

	CString strHeadFormatCommand = bNoFormatCommands ? _T("") : _T("<br_head>");
	CString info;
	info.Format(_T("%s\n")
		+ CString(_T("eD2K ")) + GetResString(IDS_FD_HASH) + _T(" %s\n")
		+ GetResString(IDS_AICHHASH) + _T(": %s\n")
		+ GetResString(IDS_FD_SIZE) + _T(" %s\n") + strHeadFormatCommand + _T("\n")
		+ GetResString(IDS_TYPE) + _T(": %s\n")
		+ GetResString(IDS_FOLDER) + _T(": %s\n\n")
		+ GetResString(IDS_PRIORITY) + _T(": %s\n")
		+ GetResString(IDS_SF_REQUESTS) + _T(": %s\n")
		+ GetResString(IDS_SF_ACCEPTS) + _T(": %s\n")
		+ GetResString(IDS_SF_TRANSFERRED) + _T(": %s%s"),
		GetFileName(),
		md4str(GetFileHash()),
		m_FileIdentifier.GetAICHHash().GetString(),
		CastItoXBytes(GetFileSize(), false, false),
		strType,
		strFolder,
		GetUpPriorityDisplayString(),
		strRequests,
		strAccepts,
		strTransferred,
		dbgInfo);
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
			return GetResString(IDS_PRIORELEASE);
		//Xman PowerRelease
		case PR_POWER:
			return GetResString(IDS_POWERRELEASE);
		//Xman end
		default:
			return _T("");
	}
}

bool CKnownFile::ShouldPartiallyPurgeFile() const
{
	return thePrefs.DoPartiallyPurgeOldKnownFiles() && m_timeLastSeen > 0
		&& time(NULL) > m_timeLastSeen && time(NULL) - m_timeLastSeen > OLDFILES_PARTIALLYPURGE;
}

// ==> Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle
/*
//Xman PowerRelease
// SLUGFILLER: hideOS
uint32 *CKnownFile::CalcPartSpread(){
	uint16 parts = GetED2KPartCount();
	uint32 *ret = new uint32[parts];
	uint32 min;
	uint32 max = 0;
	uint64 last = 0;

	for (POSITION pos = statistic.spreadlist.GetHeadPosition(); pos != 0; statistic.spreadlist.GetNext(pos)){
		Spread_Struct* cur_spread = statistic.spreadlist.GetAt(pos);
		if (max < cur_spread->count)
			max = cur_spread->count;
	}
	for (uint16 i = 0; i < parts; i++)
		ret[i] = max;
	min = max;
	for (POSITION pos = statistic.spreadlist.GetHeadPosition(); pos != 0; statistic.spreadlist.GetNext(pos)){
		Spread_Struct* cur_spread = statistic.spreadlist.GetAt(pos);
		uint32 start = (uint32)(cur_spread->start/PARTSIZE);
		uint32 end = (uint32)((cur_spread->end-1)/PARTSIZE+1);
		if (end > parts)
			end = parts;
		if (start >= end)
			continue;
		for (uint32 i = start; i < end; i++)
			if (ret[i] > cur_spread->count)
				ret[i] = cur_spread->count;
		if (min > cur_spread->count)
			min = cur_spread->count;
		if (last < cur_spread->end)
			last = cur_spread->end;
	}
	if (last < GetFileSize()) {
		min = 0;
		uint32 start = (uint32)(last/PARTSIZE);
		uint32 end = parts;
		for (uint32 i = start; i < end; i++)
			ret[i] = 0;
	}
	if (parts != GetPartCount())
		parts--;
	if (IsPartFile()) {		// Special case, ignore unshareables for min calculation
		min = max;
		for (uint16 i = 0; i < parts; i++){
			//if (((CPartFile*)this)->IsPartShareable(i))	// SLUGFILLER: SafeHash
			if (min > ret[i])
				min = ret[i];
		}
	}
	for (uint16 i = 0; i < parts; i++)
		if (ret[i] > min)
			ret[i] -= min;
		else
			ret[i] = 0;
	return ret;
}

bool CKnownFile::HideOvershares(CSafeMemFile* file, CUpDownClient* client){
	uint16 parts = GetED2KPartCount();

	if (parts<=3 || GetUpPriority()!=PR_POWER) 
		return false;
	uint32 *partspread = CalcPartSpread();

	uint16 counthiddenchunks=0;
	uint16 oldhideos=hideos;
#pragma warning(disable:4127)
	while(true)
	{
		counthiddenchunks=0;
		for (uint16 i = 0; i < parts; i++)
			if (partspread[i] >= hideos)
				counthiddenchunks++;
		if(counthiddenchunks>(uint16)(2*parts/3)) // 2/3 of all parts are hidden
			hideos++;
		else 
			break;
	}
#pragma warning(default:4127)
	if(hideos!=oldhideos)
		AddDebugLogLine(false,_T("PowerRelease: HideOS increased for file %s: old value:%u new value:%u hidden chunks: %u"),GetFileName(), oldhideos, hideos, counthiddenchunks);

	if(counthiddenchunks==0)
	{
		delete[] partspread;
		return false;
	}

	if (client->m_abyUpPartStatus)
	{
		bool allhidden=true;
		for(uint16 i=0;i<parts;i++)
		{
			if (client->IsUpPartAvailable((UINT)i)==false && partspread[i]<hideos)
			{
				allhidden=false;
				break;
			}
		}
		if(allhidden)
		{
			delete[] partspread;
			return false;
		}
	}

	file->WriteUInt16(parts);
	uint16 done = 0;
	while (done != parts){
		uint8 towrite = 0;
		for (UINT i = 0;i != 8;i++){
			if (partspread[done] < hideos)
				towrite |= (1<<i);
			done++;
			if (done == parts)
				break;
		}
		file->WriteUInt8(towrite);
	}
	delete[] partspread;
	return true;
}
//Xman end
*/
// <== Removed Dynamic Hide OS [SlugFiller/Xman] - Stulle

//Xman
// Maella -One-queue-per-file- (idea bloodymad)
uint32 CKnownFile::GetFileScore(uint32 downloadingTime)
{

	// Score proportional to the waiting time since the last upload
	uint32 elapsedTime;
	if(downloadingTime==0 || downloadingTime > 900000)
	{
		// Normal score based on the last upload of this file
		elapsedTime = GetTickCount() - m_startUploadTime;
	}
	else
	{
		// Bonus during the first 15 minutes.
		return (0x6FFFFFFF/1000); // ~6*3.1 days
	}

	// Special boost for files that have never been uploaded. Without this, a file
	// with a very low priority might wait very long for its first upload.
	if(statistic.GetTransferred() == 0){
		// This files has never been uploaded
		switch(GetUpPriority()){ 
			case PR_POWER:	  return elapsedTime/1000;
			case PR_VERYHIGH: return elapsedTime/2000;
			case PR_HIGH:     return elapsedTime/3000;
			case PR_NORMAL:   return elapsedTime/4000;
			case PR_LOW:      return elapsedTime/5000;
			case PR_VERYLOW:  return elapsedTime/6000;
		} 
	}
	else{
		// This files has already been uploaded
		switch(GetUpPriority()){ 
			case PR_POWER:	  return elapsedTime/1000;
			case PR_VERYHIGH: return elapsedTime/3000;
			case PR_HIGH:     return elapsedTime/8000;
			case PR_NORMAL:   return elapsedTime/12000;
			case PR_LOW:      return elapsedTime/16000;
			case PR_VERYLOW:  return elapsedTime/22000;
		} 
	}
	return 0;
}
// Maella end

//Xman advanced upload-priority
double CKnownFile::CalculateUploadPriorityPercent()
{
	uint64 wantedUpload = GetWantedUpload();
	
	double percent = statistic.GetCountedTransferred() /(double)wantedUpload*100.0; 

	return percent;
}

void CKnownFile::CalculateAndSetUploadPriority()
{
	if (!IsAutoUpPriority())
		return;

	float avgpercent = theApp.sharedfiles->m_lastavgPercent;
	float changefactor= avgpercent / 100 * 23;
	if (changefactor < 3)  changefactor =3;
	//if (changefactor > 20) changefactor=20;

	uint8 wantedprio=0;

	if((uint64)GetFileSize() > 500*1024)
	{
		double uploadpercent = CalculateUploadPriorityPercent();
		if (GetUpPriority()==PR_NORMAL)
			changefactor *= 1.2f; //prefer to keep the level

		if (uploadpercent < avgpercent - changefactor)
		{
				wantedprio=PR_HIGH;
		}
		else if (uploadpercent > avgpercent + changefactor)
		{
				wantedprio=PR_LOW;
		}
		else
		{
				wantedprio=PR_NORMAL;
		}
		if(GetOnUploadqueue() < theApp.sharedfiles->m_avg_client_on_uploadqueue/8)
			wantedprio=PR_HIGH;
		else if(wantedprio==PR_LOW && GetOnUploadqueue() < theApp.sharedfiles->m_avg_client_on_uploadqueue/4)
			wantedprio=PR_NORMAL;
		SetUpPriority(wantedprio,false);
	}
	else
			SetUpPriority( PR_HIGH,false ); //files < 500k always high prio
}

//Xman this is the debug version
void CKnownFile::CalculateAndSetUploadPriority2()
{
	if (!IsAutoUpPriority())
		return;

	float avgpercent = theApp.sharedfiles->m_lastavgPercent;
	float changefactor= avgpercent / 100 * 23;
	if (changefactor < 3)  changefactor =3;
	//if (changefactor > 20) changefactor=20;
	
	uint8 wantedprio=0;

	if((uint64)GetFileSize() > 500*1024)
	{
		double uploadpercent = CalculateUploadPriorityPercent();
		if (GetUpPriority()==PR_NORMAL)
			changefactor *= 1.2f; //prefer to keep the level
		uint64 wantedUpload = GetWantedUpload();
		uint64 completedup= IsPartFile() ? ((CPartFile*)this)->GetCompletedSize() : GetFileSize();
		
		uint32 virtualsources=GetVirtualSourceIndicator();

		
		AddDebugLogLine(false, _T("avg: %0.2f%% percent: %0.2f%% , wanted: %s, counted upload: %s, completed: %s, pushfaktor: %i, avg virtuals: %u, own virtuals: %u, file: %s"),avgpercent,uploadpercent,  CastItoXBytes(wantedUpload), CastItoXBytes(statistic.GetCountedTransferred()),CastItoXBytes(completedup),(int16)((pushfaktor-1)*100),theApp.sharedfiles->m_avg_virtual_sources, virtualsources,this->GetFileName()); 

		if (uploadpercent < avgpercent - changefactor)
		{
			wantedprio=PR_HIGH;
			AddDebugLogLine(false, _T("setting high")); 
		}
		else if (uploadpercent > avgpercent + changefactor)
		{
			wantedprio=PR_LOW;
			AddDebugLogLine(false, _T("setting low")); 
		}
		else
		{
			wantedprio=PR_NORMAL;
			AddDebugLogLine(false, _T("setting normal")); 
		}
		if(wantedprio!=PR_HIGH && GetOnUploadqueue() < theApp.sharedfiles->m_avg_client_on_uploadqueue/8)
		{
			wantedprio=PR_HIGH;
			AddDebugLogLine(false, _T("setting high because low requests")); 
		}
		else if(wantedprio==PR_LOW && GetOnUploadqueue() < theApp.sharedfiles->m_avg_client_on_uploadqueue/4)
		{
			wantedprio=PR_NORMAL;
			AddDebugLogLine(false, _T("setting normal because low requests")); 
		}
		SetUpPriority(wantedprio,false);
	}
	else
	{
			SetUpPriority( PR_HIGH,false ); //files < 500k always high prio
		AddDebugLogLine(false, _T("small file->high Prio. file: %s"), GetFileName());
	}
}

uint32 CKnownFile::GetVirtualSourceIndicator() const
{
	if(IsPartFile())
	{
		uint32 fullsources;
		if(m_nCompleteSourcesCountHi > m_nCompleteSourcesCountLo)
			fullsources = m_nCompleteSourcesCountLo + (m_nCompleteSourcesCountHi-m_nCompleteSourcesCountLo)/4;
		else
			fullsources = m_nCompleteSourcesCountLo;
		return m_nVirtualUploadSources*2 + fullsources; 
	}
	else
	{
		uint32 fullsources;
		if(m_nCompleteSourcesCountHi > m_nCompleteSourcesCountLo)
			fullsources = (m_nCompleteSourcesCountLo + (m_nCompleteSourcesCountHi-m_nCompleteSourcesCountLo)/4);
		else
			fullsources = m_nCompleteSourcesCountLo;
		return m_nVirtualCompleteSourcesCount*2 + fullsources; 
	}
}

uint64 CKnownFile::GetWantedUpload()
{
	double returnvalue=0;

	uint64 filesize = IsPartFile() ? ((CPartFile*)this)->GetCompletedSize() : GetFileSize();
	if (filesize <=0) filesize = 1;
	{
		double factor = 1.0 + (60.63 / ( (filesize/(1024.0*1024.0)) + 2.2 ));
		returnvalue = (factor * (double)filesize);
	}

	if(!thePrefs.GetEnableMultiQueue()  )
	{

		uint32 virtualsources=GetVirtualSourceIndicator();
		
		pushfaktor=1;
		if(theApp.sharedfiles->m_avg_virtual_sources > 0)
		{
			if(virtualsources < theApp.sharedfiles->m_avg_virtual_sources)
			{
				virtualsources = theApp.sharedfiles->m_avg_virtual_sources - virtualsources;
				pushfaktor = 1 + (float)virtualsources / theApp.sharedfiles->m_avg_virtual_sources;
			}
			else
			{
				if(virtualsources > theApp.sharedfiles->m_avg_virtual_sources*3)
					pushfaktor = 0.67f; //-33%
				else if(virtualsources > theApp.sharedfiles->m_avg_virtual_sources*2)
					pushfaktor = 0.85f; //-15%
			}
		}
		returnvalue *= pushfaktor;
	}
	
	if(returnvalue <=1) //just to be sure
		returnvalue = 1;

	return (uint64)returnvalue;
}

void CKnownFile::UpdateVirtualUploadSources()
{
	UINT partcount = GetPartCount();

	CArray<uint16, uint16> tmp_AvailPartFrequency;

	tmp_AvailPartFrequency.SetSize(partcount);
	for (UINT i = 0; i < partcount; i++)
		tmp_AvailPartFrequency[i] = 0;

	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		if (cur_src->m_abyUpPartStatus && cur_src->GetUpPartCount() == partcount)
		{
			for (UINT i = 0; i < partcount; i++)
			{
				if (cur_src->IsUpPartAvailable(i))
					tmp_AvailPartFrequency[i] += 1;
			}
		}
	}


	UINT nVirtualUploadSources = (UINT)-1;
	for (UINT i = 0; i < partcount; i++){
		if(tmp_AvailPartFrequency[i] < nVirtualUploadSources)
			nVirtualUploadSources = tmp_AvailPartFrequency[i];
	}


	if (m_nVirtualUploadSources!=nVirtualUploadSources)
	{
		m_nVirtualUploadSources=nVirtualUploadSources;
		if(theApp.emuledlg->sharedfileswnd->m_hWnd)
			theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
	}
}

void CKnownFile::CheckAUPFilestats(bool allowUpdatePrio)
{
	if(statistic.m_tlastdataupdate==0 && statistic.GetAllTimeTransferred()==0)
	{
		//this is a new shared file. set NOW the auto-prio if user want it
		if(allowUpdatePrio)
		{
			if(thePrefs.GetNewAutoUp()){
				SetAutoUpPriority(true);
				SetUpPriority(PR_HIGH);
			}
			else
			{
				if(IsAutoUpPriority())
				{
					SetAutoUpPriority(false);
					SetUpPriority(PR_NORMAL);
				}
			}
		}
		statistic.m_tlastdataupdate=time(NULL);
	}
	else if(statistic.m_tlastdataupdate==0 || statistic.m_tlastdataupdate>(uint32)time(NULL))
	{
		//this file was uploaded with a previous version or there is anything wrong
		//count old upload 50%
		if(statistic.m_tlastdataupdate>(uint32)time(NULL))
			AddDebugLogLine(false,_T("found mismatch at date of file: %s"), GetFileName());
#ifdef _BETA
		if(statistic.m_tlastdataupdate==0)
			AddDebugLogLine(false,_T("found file without timestamp. Old upload is counted half. file: %s"), GetFileName());
#endif
		statistic.m_unotcountedtransferred=statistic.GetAllTimeTransferred()/2;
		statistic.m_tlastdataupdate=time(NULL);
	}
	else
	{
		//update normal:
		statistic.UpdateCountedTransferred();
	}
}

// ==> push rare file - Stulle
float CKnownFile::GetFileRatio() /*const*/
{
	if(!thePrefs.GetEnablePushRareFile())
		return 1.0f;
	// MatzeHH formula I think is better than herbert's one (Tarod)
	if (statistic.GetRequests() == 0) return 2.0f ;

	float ratio ;
	ratio = ((float)statistic.GetRequests()) / ((float)theApp.knownfiles->GetTotalRequested()) ;
	ratio = -4.0f * log10f(((float)theApp.sharedfiles->GetCount())) * ratio ;
	ratio = 2.0f * expf(ratio) ;

	if (ratio < 1.0f) {
		ratio = 1.0f;
	} else if (ratio > 2.0f)
		ratio = 2.0f ;

	return ratio ;
}
// <== push rare file - Stulle

// ==> push small files [sivka] - Stulle
bool CKnownFile::IsPushSmallFile()
{
	return(thePrefs.GetEnablePushSmallFile() && 
		GetFileSize() <= (uint64)thePrefs.GetPushSmallFileSize());
}
// <== push small files [sivka] - Stulle

// ==> Copy feedback feature [MorphXT] - Stulle
CString CKnownFile::GetFeedback(bool isUS)
{
	CStringList LabelList;
	CString feed;

	if(isUS)
	{
		LabelList.AddTail(_T("File name"));
		LabelList.AddTail(_T("File type"));
		LabelList.AddTail(_T("Size"));
		LabelList.AddTail(_T("Downloaded"));
		LabelList.AddTail(_T("Transferred"));
		LabelList.AddTail(_T("Requested"));
		LabelList.AddTail(_T("Accepted Requests"));
		if(IsPartFile()){
			LabelList.AddTail(_T("Total sources"));
			LabelList.AddTail(_T("Available sources"));
			LabelList.AddTail(_T("No Need Part sources"));
		}
		LabelList.AddTail(_T("On Queue"));
		LabelList.AddTail(_T("Complete sources"));
	}
	else
	{
		LabelList.AddTail(GetResString(IDS_FEEDBACK_FILENAME));
		LabelList.AddTail(GetResString(IDS_FEEDBACK_FILETYPE));
		LabelList.AddTail(GetResString(IDS_FEEDBACK_FILESIZE));
		LabelList.AddTail(GetResString(IDS_FEEDBACK_DOWNLOADED));
		LabelList.AddTail(GetResString(IDS_FEEDBACK_TRANSFERRED));
		LabelList.AddTail(GetResString(IDS_FEEDBACK_REQUESTED));
		LabelList.AddTail(GetResString(IDS_FEEDBACK_ACCEPTED));
		if(IsPartFile()){
			LabelList.AddTail(GetResString(IDS_FEEDBACK_TOTAL));
			LabelList.AddTail(GetResString(IDS_FEEDBACK_AVAILABLE));
			LabelList.AddTail(GetResString(IDS_FEEDBACK_NONEEDPART));
		}
		LabelList.AddTail(GetResString(IDS_FEEDBACK_ON_QUEUE));
		LabelList.AddTail(GetResString(IDS_FEEDBACK_COMPLETE));
	}

	POSITION pos=LabelList.GetHeadPosition();
	// ==> Feedback personalization [Stulle] - Stulle
	/*
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos),GetFileName());
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos),GetFileType());
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), CastItoXBytes(GetFileSize(),false,false,3,isUS));
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), (IsPartFile()==false)?GetResString(IDS_COMPLETE):CastItoXBytes(((CPartFile*)this)->GetCompletedSize(),false,false,3,isUS));
	feed.AppendFormat(_T("%s: %s (%s)\r\n"),LabelList.GetNext(pos), CastItoXBytes(statistic.GetTransferred(),false,false,3,isUS), CastItoXBytes(statistic.GetAllTimeTransferred(),false,false,3,isUS)); 
	feed.AppendFormat(_T("%s: %i (%i)\r\n"),LabelList.GetNext(pos), statistic.GetRequests(), statistic.GetAllTimeRequests()); 
	feed.AppendFormat(_T("%s: %i (%i)\r\n"),LabelList.GetNext(pos), statistic.GetAccepts(),statistic.GetAllTimeAccepts()); 
	if(IsPartFile()){
		feed.AppendFormat(_T("%s: %i\r\n"),LabelList.GetNext(pos),((CPartFile*)this)->GetSourceCount());
		feed.AppendFormat(_T("%s: %i\r\n"),LabelList.GetNext(pos),((CPartFile*)this)->GetAvailableSrcCount());
		feed.AppendFormat(_T("%s: %i\r\n"),LabelList.GetNext(pos),((CPartFile*)this)->GetSrcStatisticsValue(DS_NONEEDEDPARTS));
	}
	feed.AppendFormat(_T("%s: %i\r\n"),LabelList.GetNext(pos),onuploadqueue);
	feed.AppendFormat(_T("%s: %i\r\n"),LabelList.GetNext(pos),m_nCompleteSourcesCount);
	*/
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText(GetFileName(),style_f_names));
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText(GetFileType(),style_f_fileinfo));
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText(CastItoXBytes(GetFileSize(),false,false,3,isUS),style_f_fileinfo));
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText((IsPartFile()==false)?GetResString(IDS_COMPLETE):CastItoXBytes(((CPartFile*)this)->GetCompletedSize(),false,false,3,isUS),style_f_filestate));
	feed.AppendFormat(_T("%s: %s (%s)\r\n"),LabelList.GetNext(pos), GetColoredText(CastItoXBytes(statistic.GetTransferred(),false,false,3,isUS),style_f_transferred), GetColoredText(CastItoXBytes(statistic.GetAllTimeTransferred(),false,false,3,isUS),style_f_transferred)); 
	feed.AppendFormat(_T("%s: %s (%s)\r\n"),LabelList.GetNext(pos), GetColoredText(statistic.GetRequests(),style_f_requests), GetColoredText(statistic.GetAllTimeRequests(),style_f_requests)); 
	feed.AppendFormat(_T("%s: %s (%s)\r\n"),LabelList.GetNext(pos), GetColoredText(statistic.GetAccepts(),style_f_requests), GetColoredText(statistic.GetAllTimeAccepts(),style_f_requests)); 
	if(IsPartFile()){
		feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText(((CPartFile*)this)->GetSourceCount(),style_f_sources));
		feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText(((CPartFile*)this)->GetAvailableSrcCount(),style_f_sources));
		feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText(((CPartFile*)this)->GetSrcStatisticsValue(DS_NONEEDEDPARTS),style_f_sources));
	}
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText((UINT)onuploadqueue,style_f_clientsonqueue));
	feed.AppendFormat(_T("%s: %s\r\n"),LabelList.GetNext(pos), GetColoredText((UINT)m_nCompleteSourcesCount,style_f_compeltesrc));
	// <== Feedback personalization [Stulle] - Stulle
	return feed;
}
// <== Copy feedback feature [MorphXT] - Stulle

// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
void CKnownFile::CalcPartSpread(CArray<uint64>& partspread, CUpDownClient* client)
{
	UINT parts = GetPartCount();
	uint64 min = (uint64)-1;
	UINT mincount = 1;
	UINT mincount2 = 1;
	UINT i;

	ASSERT(client != NULL);

	partspread.RemoveAll();

	CArray<bool, bool> partsavail;
	bool usepartsavail = false;
	
	partspread.SetSize(parts);
	partsavail.SetSize(parts);
	for (i = 0; i < parts; i++) {
		partspread[i] = 0;
		partsavail[i] = true;
	}

	if (IsPartFile()) {
		uint32 somethingtoshare = 0;
		for (i = 0; i < parts; i++)
			if (!((CPartFile*)this)->IsPartShareable(i)){	// SLUGFILLER: SafeHash
				partsavail[i] = false;
				usepartsavail = true;
			} else
				++somethingtoshare;
		if (somethingtoshare<=2)
			return;
	}
	if (client->m_abyUpPartStatus) {
		uint32 somethingtoshare = 0;
		for (i = 0; i < parts; i++)
			if (client->IsUpPartAvailable(i)) {
				partsavail[i] = false;
				usepartsavail = true;
			} else if (partsavail[i])
				++somethingtoshare;
		if (somethingtoshare<=2)
			return;
	}
	
	UINT hideOS = HideOSInWork();
	if(hideOS && !statistic.spreadlist.IsEmpty())
	{
		POSITION pos = statistic.spreadlist.GetHeadPosition();
		uint16 last = 0;
		uint64 count = statistic.spreadlist.GetValueAt(pos);
		min = count;
		statistic.spreadlist.GetNext(pos);
		while (pos && last < parts){
			uint64 next = statistic.spreadlist.GetKeyAt(pos);
			if (next >= GetFileSize())
				break;
			next /= PARTSIZE;
			while (last < next) {
				partspread[last] = count;
				last++;
			}
			if (last >= parts || !(statistic.spreadlist.GetKeyAt(pos) % PARTSIZE)) {
				count = statistic.spreadlist.GetValueAt(pos);
				if (min > count)
					min = count;
				statistic.spreadlist.GetNext(pos);
				continue;
			}
			partspread[last] = count;
			while (next == last) {
				count = statistic.spreadlist.GetValueAt(pos);
				if (min > count)
					min = count;
				if (partspread[last] > count)
					partspread[last] = count;
				statistic.spreadlist.GetNext(pos);
				if (!pos)
					break;
				next = statistic.spreadlist.GetKeyAt(pos);
				if (next >= GetFileSize())
					break;
				next /= PARTSIZE;
			}
			last++;
		}
		while (last < parts) {
			partspread[last] = count;
			last++;
		}
		for (i = 0; i < parts; i++)
			if (partspread[i] >= hideOS && client->m_abyUpPartStatus)
				client->m_abyUpPartStatus[i] |= SC_HIDDENBYHIDEOS;
	}
	UINT SOTN = ((GetShareOnlyTheNeed()>=0)?GetShareOnlyTheNeed():thePrefs.GetShareOnlyTheNeed()); 
	if (SOTN) {
		/*
		if (IsPartFile()) {
			CPartFile* pfile = (CPartFile*)this;
			if (pfile->m_SrcpartFrequency.IsEmpty() == false) {
				for (i = 0; i < parts; i++)
					if (partsavail[i] && pfile->m_SrcpartFrequency[i]>partspread[i]) {
						partspread[i] = pfile->m_SrcpartFrequency[i];
						if (client->m_abyUpPartStatus)
							client->m_abyUpPartStatus[i] |= SC_HIDDENBYSOTN;
					}
			}
		} else {
		*/
			if (m_AvailPartFrequency.IsEmpty() == false) {
				for (i = 0; i < parts; i++)
					if (partsavail[i] && m_AvailPartFrequency[i]>partspread[i]) {
						partspread[i] = m_AvailPartFrequency[i];
						if (client->m_abyUpPartStatus)
							client->m_abyUpPartStatus[i] |= SC_HIDDENBYSOTN;
					}
			}
//		}
	}

	if (usepartsavail && !statistic.spreadlist.IsEmpty() || SOTN) {		// Special case, ignore unshareables for min calculation
		uint64 min2 = min = (uint64)-1;
		for (i = 0; i < parts; i++)
			if (partsavail[i]){
				if (min2>partspread[i])
					min2 = partspread[i];
				else if (min2==partspread[i])
					min = min2;
			}
	}

	for (i = 0; i < parts; i++) {
		if (partspread[i] > min)
			partspread[i] -= min;
		else {
			partspread[i] = 0;
			if (client->m_abyUpPartStatus)
				client->m_abyUpPartStatus[i] &= SC_AVAILABLE;
		}
	}

	if (!hideOS || ((GetSelectiveChunk()>=0)?!GetSelectiveChunk():!thePrefs.IsSelectiveShareEnabled()))
		return;

	if (client->m_nSelectedChunk > 0 && (int)client->m_nSelectedChunk <= partspread.GetSize() && partsavail[client->m_nSelectedChunk-1]) {
		for (i = 0; i < client->m_nSelectedChunk-1; i++) {
			partspread[i] = hideOS;
			// ==> See chunk that we hide [SiRoB] - Stulle
			if (client->m_abyUpPartStatus)
				client->m_abyUpPartStatus[i] |= SC_HIDDENBYHIDEOS;
			// <== See chunk that we hide [SiRoB] - Stulle
		}
		for (i = client->m_nSelectedChunk; i < parts; i++) {
			partspread[i] = hideOS;
			// ==> See chunk that we hide [SiRoB] - Stulle
			if (client->m_abyUpPartStatus)
				client->m_abyUpPartStatus[i] |= SC_HIDDENBYHIDEOS;
			// <== See chunk that we hide [SiRoB] - Stulle
		}
		return;
	}
	else
		client->m_nSelectedChunk = 0;

	bool resetSentCount = false;
	
	if (m_PartSentCount.GetSize() != partspread.GetSize() || m_PartSentCount.GetSize() == 0)
		resetSentCount = true;
	else {
		min = hideOS;
		for (i = 0; i < parts; i++) {
			if (!partsavail[i])
				continue;
			if (m_PartSentCount[i] < partspread[i])
				m_PartSentCount[i] = partspread[i];
			if (m_PartSentCount[i] < min) {
				min = m_PartSentCount[i];
				mincount = 1;
			}
			else if (m_PartSentCount[i] == min)
				mincount++;
		}
		if (min >= hideOS)
			resetSentCount = true;
	}

	if (resetSentCount) {
		min = 0;
		mincount = 0;
		mincount2 = 0;
		m_PartSentCount.SetSize(parts);
		for (i = 0; i < parts; i++){
			m_PartSentCount[i] = partspread[i];
			if (partsavail[i] && !partspread[i])
				mincount++;
		}
		if (!mincount)
		  return; // We're a no-needed source already
	}

	mincount = (rand() % mincount) + 1;
	mincount2 = mincount;
	for (i = 0; i < parts; i++) {
		if (!partsavail[i])
			continue;
		if (m_PartSentCount[i] > min)
			continue;
		ASSERT(m_PartSentCount[i] == min);
		mincount--;
		if (!mincount)
			break;
	}
	ASSERT(i < parts);
	if (mincount)
		return;
	m_PartSentCount[i]++;
	client->m_nSelectedChunk = i+1;
	mincount = i;
	for (i = 0; i < parts; i++) {
		if (!partsavail[i])
			continue;
		if (m_PartSentCount[i] > min)
			continue;
		ASSERT(m_PartSentCount[i] == min);
		mincount2--;
		if (!mincount2)
			break;
	}
	if (mincount2)
		return;
	m_PartSentCount[i]++;
	mincount2 = i;
	/*
	for (i = 0; i < mincount; i++)
		partspread[i] = hideOS;
	for (i = mincount+1; i < parts; i++)
		partspread[i] = hideOS;
	*/
	for (i = 0; i < parts; i++)
	{	
		if ( i != mincount && i != mincount2) {
			partspread[i] = hideOS;
			// ==> See chunk that we hide [SiRoB] - Stulle
			if (client->m_abyUpPartStatus)
				client->m_abyUpPartStatus[i] |= SC_HIDDENBYHIDEOS;
			// <== See chunk that we hide [SiRoB] - Stulle
		}
	}
	return;
};

bool CKnownFile::HideOvershares(CSafeMemFile* file, CUpDownClient* client){
	CArray<uint64> partspread;
	UINT parts = (UINT)GetPartCount();
	if (client->m_abyUpPartStatus == NULL) {
		client->SetPartCount((uint16)parts);
		client->m_abyUpPartStatus = new uint8[parts];
		memset(client->m_abyUpPartStatus,0,parts);
	}
	CalcPartSpread(partspread, client);

	uint64 max = partspread[0];
	for (UINT i = 1; i < parts; i++)
		if (partspread[i] > max)
			max = partspread[i];
	UINT hideOS = HideOSInWork();
	UINT SOTN = ((GetShareOnlyTheNeed()>=0)?GetShareOnlyTheNeed():thePrefs.GetShareOnlyTheNeed());
	if (!hideOS && SOTN)
		hideOS = 1;
	if (max < hideOS || !hideOS)
		return FALSE;
	UINT nED2kPartCount = GetED2KPartCount();
	file->WriteUInt16((uint16)nED2kPartCount);
	UINT done = 0;
	while (done != nED2kPartCount){
		uint8 towrite = 0;
		for (UINT i = 0;i < 8;i++){
			if (done < parts && partspread[done] < hideOS) {
				towrite |= (1<<i);
				// ==> See chunk that we hide [SiRoB] - Stulle
				client->m_abyUpPartStatus[done] &= SC_AVAILABLE;
				// <== See chunk that we hide [SiRoB] - Stulle
			}
			done++;
			if (done == nED2kPartCount)
				break;
		}
		file->WriteUInt8(towrite);
	}
	return TRUE;
}

UINT	CKnownFile::HideOSInWork() const
{
	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	if(thePrefs.GetSpreadbarSetStatus() == false)
		return 0;
	// <== Spread bars [Slugfiller/MorphXT] - Stulle
	if(IsPartFile() == true) // not for partfiles
		return 0;

	return  (m_iHideOS>=0)?m_iHideOS:thePrefs.GetHideOvershares();
}
// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

// ==> PowerShare [ZZ/MorphXT] - Stulle
void CKnownFile::SetPowerShared(int newValue) {
	int oldValue = m_powershared; // Mephisto Upload - Mephisto
    m_powershared = newValue;
	if (IsPartFile() == false)
		UpdatePartsInfo();
	// ==> Mephisto Upload - Mephisto
	if(theApp.uploadqueue && oldValue != newValue)
		theApp.uploadqueue->ReSortUploadSlots(true);
	// <== Mephisto Upload - Mephisto
}

void CKnownFile::UpdatePowerShareLimit(bool authorizepowershare,bool autopowershare, bool limitedpowershare)
{
	m_bPowerShareAuthorized = authorizepowershare;
	m_bPowerShareAuto = autopowershare;
	m_bPowerShareLimited = limitedpowershare;
	int temppowershared = (m_powershared>=0)?m_powershared:thePrefs.GetPowerShareMode();
	bool oldPowershare = m_bpowershared; // Mephisto Upload - Mephisto
	m_bpowershared = ((temppowershared&1) || ((temppowershared == 2) && m_bPowerShareAuto)) && m_bPowerShareAuthorized && !((temppowershared == 3) && m_bPowerShareLimited);
	// ==> Mephisto Upload - Mephisto
	if (theApp.uploadqueue && oldPowershare != m_bpowershared)
		theApp.uploadqueue->ReSortUploadSlots(true);
	// <== Mephisto Upload - Mephisto
}
bool CKnownFile::GetPowerShared() const
{
	return m_bpowershared;
}
// <== PowerShare [ZZ/MorphXT] - Stulle

// ==> Design Settings [eWombat/Stulle] - Stulle
int CKnownFile::GetKnownStyle() const
{
	int iKnownStyle = style_s_default;
	if(GetPowerShared() && thePrefs.GetStyleOnOff(share_styles, style_s_powershare)!=0)
		iKnownStyle = style_s_powershare;
	else if(IsAutoUpPriority() && thePrefs.GetStyleOnOff(share_styles, style_s_auto)!=0)
		iKnownStyle = style_s_auto;
	else if(GetUpPriority()==PR_VERYLOW && thePrefs.GetStyleOnOff(share_styles, style_s_verylow)!=0)
		iKnownStyle = style_s_verylow;
	else if(GetUpPriority()==PR_LOW && thePrefs.GetStyleOnOff(share_styles, style_s_low)!=0)
		iKnownStyle = style_s_low;
	else if(GetUpPriority()==PR_NORMAL && thePrefs.GetStyleOnOff(share_styles, style_s_normal)!=0)
		iKnownStyle = style_s_normal;
	else if(GetUpPriority()==PR_HIGH && thePrefs.GetStyleOnOff(share_styles, style_s_high)!=0)
		iKnownStyle = style_s_high;
	else if(GetUpPriority()==PR_VERYHIGH && thePrefs.GetStyleOnOff(share_styles, style_s_release)!=0)
		iKnownStyle = style_s_release;
	else if(GetUpPriority()==PR_POWER && thePrefs.GetStyleOnOff(share_styles, style_s_powerrelease)!=0)
		iKnownStyle = style_s_powerrelease;
	return iKnownStyle;
}
// <== Design Settings [eWombat/Stulle] - Stulle