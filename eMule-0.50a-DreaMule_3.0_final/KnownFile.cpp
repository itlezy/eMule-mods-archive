// parts of this file are based on work from pan One (http://home-3.tiscali.nl/~meost/pms/)
//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
//remove moblie mule
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
#include "SafeFile.h"
#include "shahashset.h"
#include "Log.h"
#include "MD4.h"
#include "Collection.h"
#include "emuledlg.h"
#include "SharedFilesWnd.h"
#include "MediaInfo.h"
#include "MuleStatusBarCtrl.h" //Xman Progress Hash (O2)
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
	(void)m_strComment;
	m_PublishedED2K = false;
	//kadFileSearchID = 0;	// Tux: Improvement: Kademlia interface improvement
	SetLastPublishTimeKadSrc(0,0);
	m_nCompleteSourcesTime = time(NULL);
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
	onuploadqueue = 0; //Xman see OnUploadqueue
	hideos=1; //Xman PowerRelease
	// Maella -One-queue-per-file- (idea bloodymad)
	m_startUploadTime = ::GetTickCount();
	// Maella end
	//Xman show virtual sources (morph)
	m_nVirtualCompleteSourcesCount = 0;

	//Xman advanced upload-priority
	pushfaktor=0;
	m_nVirtualUploadSources = 0;
	//Xman end
}

CKnownFile::~CKnownFile()
{
	for (int i = 0; i < hashlist.GetSize(); i++)
		delete[] hashlist[i];
	delete m_pAICHHashSet;
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
	hashlist.AssertValid();
	(void)m_strDirectory;
	(void)m_strFilePath;
	(void)m_iPartCount;
	(void)m_iED2KPartCount;
	(void)m_iED2KPartHashCount;
	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH || m_iUpPriority == PR_POWER ); //Xman PowerRelease
	CHECK_BOOL(m_bAutoUpPriority);
	//(void)s_ShareStatusBar; //Xman
	CHECK_BOOL(m_PublishedED2K);
	//(void)kadFileSearchID;	// Tux: Improvement: Kademlia interface improvement
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

//CBarShader CKnownFile::s_ShareStatusBar(16);
// Xman -Code Improvement-
void CKnownFile::DrawShareStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
{
	CBarShader statusBar(rect->bottom - rect->top, rect->right - rect->left);
	statusBar.SetFileSize(GetFileSize());


	/*
	if(m_ClientUploadList.GetSize() > 0 || m_nCompleteSourcesCountHi > 1) {
		// We have info about chunk frequency in the net, so we will color the chunks we have after perceived availability.
		const COLORREF crMissing = RGB(255, 0, 0);
		statusBar.Fill(crMissing);

		if (!onlygreyrect) {
			uint32 tempCompleteSources = m_nCompleteSourcesCountLo;
			if(tempCompleteSources > 0) {
				tempCompleteSources--;
			}

			for (int i = 0; i < GetPartCount(); i++){
				uint32 frequency = tempCompleteSources;
				if(!m_AvailPartFrequency.IsEmpty()) {
					frequency = max(m_AvailPartFrequency[i], tempCompleteSources);
				}

				if(frequency > 0 ){
					const COLORREF color = RGB(0, (22*(frequency-1) >= 210)? 0:210-(22*(frequency-1)), 255);
					statusBar.FillRange(PARTSIZE*(i),PARTSIZE*(i+1),color);
				}
			}
		}
	}*/
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

				//Xman Code Improvement:
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

	//Xman show virtual sources (morph)
	/* //Xman 5.4.1 moved up
	m_nVirtualCompleteSourcesCount = (UINT)-1;
	for (UINT i = 0; i < partcount; i++){
		if(m_AvailPartFrequency[i] < m_nVirtualCompleteSourcesCount)
			m_nVirtualCompleteSourcesCount = m_AvailPartFrequency[i];
	}
	*/
	//Xman end

	if (theApp.emuledlg->sharedfileswnd->m_hWnd)
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this);
}

void CKnownFile::AddUploadingClient(CUpDownClient* client){
	POSITION pos = m_ClientUploadList.Find(client); // to be sure
	if(pos == NULL){
		m_ClientUploadList.AddTail(client);
		//UpdateAutoUpPriority(); //Xman done by see on uploadqueue
	}
}

void CKnownFile::RemoveUploadingClient(CUpDownClient* client){
	POSITION pos = m_ClientUploadList.Find(client); // to be sure
	if(pos != NULL){
		m_ClientUploadList.RemoveAt(pos);
		//UpdateAutoUpPriority(); //Xman done by see on uploadqueue
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

void CKnownFile::SetFileName(LPCTSTR pszFileName, bool bReplaceInvalidFileSystemChars)
{
	CKnownFile* pFile = NULL;

	// If this is called within the sharedfiles object during startup,
	// we cannot reference it yet..

	if(theApp.sharedfiles)
		pFile = theApp.sharedfiles->GetFileByID(GetFileHash());

	if (pFile && pFile == this)
		theApp.sharedfiles->RemoveKeywords(this);

	CAbstractFile::SetFileName(pszFileName, bReplaceInvalidFileSystemChars);

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
	for (UINT i = 0; i < GetPartCount();i++)
		m_AvailPartFrequency[i] = 0;

	// create hashset
	uint64 togo = m_nFileSize;
	UINT hashcount;
	for (hashcount = 0; togo >= PARTSIZE; )
	{
		CAICHHashTree* pBlockAICHHashTree = m_pAICHHashSet->m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, PARTSIZE);
		ASSERT( pBlockAICHHashTree != NULL );

		uchar* newhash = new uchar[16];
		if (!CreateHash(file, PARTSIZE, newhash, pBlockAICHHashTree, true)) { //Xman Nice Hash
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
		CString pourcent;
		if(GetPartCount()>0) //just to be sure
		{
			pourcent.Format(_T("Hashing  :%d%% - %s") ,(hashcount+1)*100/GetPartCount(),in_filename);
			if (theApp.emuledlg->statusbar->m_hWnd) theApp.emuledlg->statusbar->SetText( pourcent ,0,0);
		}
		//Xman end

		hashlist.Add(newhash);
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
		pBlockAICHHashTree = m_pAICHHashSet->m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, togo);
		ASSERT( pBlockAICHHashTree != NULL );
	}

	uchar* lasthash = new uchar[16];
	md4clr(lasthash);
	if (!CreateHash(file, togo, lasthash, pBlockAICHHashTree, true)) { //Xman Nice Hash
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
		//Xman remove unused AICH-hashes
		//we must apply the status a second time, because aich-synthread could be running
		//and change the Status,
		//however the hashset is saved correctly because saving is using the mutex
		//better solution would be to move the mutex here
		m_pAICHHashSet->SetStatus(AICH_HASHSETCOMPLETE);
		//Xman end
	}
	else{
		// now something went pretty wrong
		DebugLogError(LOG_STATUSBAR, _T("Failed to calculate AICH Hashset from file %s"), GetFileName());
	}

	//Xman
	hashlist.Add(lasthash);		// SLUGFILLER: SafeHash - better handling of single-part files
	if (!hashcount){
		md4cpy(m_abyFileHash, lasthash);
		//delete[] lasthash;
		// SLUGFILLER: SafeHash remove - removed delete
	}
	else {
		//hashlist.Add(lasthash);
		// SLUGFILLER: SafeHash remove - moved up
		uchar* buffer = new uchar[hashlist.GetCount()*16];
		for (int i = 0; i < hashlist.GetCount(); i++)
			md4cpy(buffer+(i*16), hashlist[i]);
		CreateHash(buffer, hashlist.GetCount()*16, m_abyFileHash, NULL, true); //Xman Nice Hash
		delete[] buffer;
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
		m_tUtcLastModified = fileinfo.st_mtime;
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strFilePath);
	}

	fclose(file);
	file = NULL;

	// Add filetags
	UpdateMetaDataTags();


	//Xman Progress Hash (O2)
	AddLogLine(true,_T("Acabou o Hash: %s"), GetFilePath()  );
	//Xman end

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
		//Xman
		if (parts != GetED2KPartCount()){	// SLUGFILLER: SafeHash - use GetED2KPartCount
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
						if (m_iUpPriority != PR_VERYLOW && m_iUpPriority != PR_LOW && m_iUpPriority != PR_NORMAL && m_iUpPriority != PR_HIGH && m_iUpPriority != PR_VERYHIGH && m_iUpPriority != PR_POWER) //Xman PowerRelease
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
				//				0 = Unknown
				//				1 = we have created that meta data by examining the file contents.
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
					m_pAICHHashSet->SetMasterHash(hash, AICH_HASHSETCOMPLETE);
				else
					ASSERT( false );
				delete newtag;
				break;
			}
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
				ConvertED2KTag(newtag);
				if (newtag)
					taglist.Add(newtag);
		}
	}

	// 05-J�n-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
	// the chance to clean any available meta data tags and provide only tags which were determined by us.
	// It's a brute force method, but that wrong meta data is driving me crazy because wrong meta data is even worse than
	// missing meta data.
	if (m_uMetaDataVer == 0)
		RemoveMetaDataTags();

	//Xman
	return m_nFileSize!=(uint64)0;		// SLUGFILLER: SafeHash - Must have a filesize tag
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
	//Xman
	if (GetED2KPartCount() <= 1) {	// ignore loaded hash for 1-chunk files
		for (int i = 0; i < hashlist.GetSize(); i++)
			delete[] hashlist[i];
		hashlist.RemoveAll();
		uchar* cur_hash = new uchar[16];
		md4cpy(cur_hash, m_abyFileHash);
		hashlist.Add(cur_hash);
		ret2 = true;
	} else if (GetED2KPartCount()!=GetHashCount())
		ret2 = false;	// Final hash-count verification, needs to be done after the tags are loaded.
	return ret1 && ret2 && ret3;
	// SLUGFILLER: SafeHash
}

bool CKnownFile::WriteToFile(CFileDataIO* file)
{
	// date
	file->WriteUInt32(m_tUtcLastModified);

	// hashset
	//Xman SafeHash compatibility fix
	//with this patch we write the same format as official
	file->WriteHash16(m_abyFileHash);
	UINT parts = hashlist.GetCount();
	if(parts > 1){
	file->WriteUInt16((uint16)parts);
	for (UINT i = 0; i < parts; i++)
		file->WriteHash16(hashlist[i]);
	}else
		file->WriteUInt16(0);
	/*
	file->WriteHash16(m_abyFileHash);
	UINT parts = hashlist.GetCount();
	file->WriteUInt16((uint16)parts);
	for (UINT i = 0; i < parts; i++)
		file->WriteHash16(hashlist[i]);
	*/
	//Xman end

	uint32 uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

	if (WriteOptED2KUTF8Tag(file, GetFileName(), FT_FILENAME))
		uTagCount++;
	CTag nametag(FT_FILENAME, GetFileName());
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

void CKnownFile::CreateHash(CFile* pFile, uint64 Length, uchar* pMd4HashOut, CAICHHashTree* pShaHashOut, bool slowdown) const //Xman Nice Hash
{
	ASSERT( pFile != NULL );
	ASSERT( pMd4HashOut != NULL || pShaHashOut != NULL );
	//Xman
	CSingleLock sLock1(&(theApp.hashing_mut), TRUE);	// SLUGFILLER: SafeHash - only one chunk-hash at a time

	uint64  Required = Length;
	uchar   X[64*128];
	uint64	posCurrentEMBlock = 0;
	uint64	nIACHPos = 0;
	CAICHHashAlgo* pHashAlg = m_pAICHHashSet->GetNewHashAlgo();
	CMD4 md4;

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
}

bool CKnownFile::CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut, bool slowdown) const //Xman Nice Hash
{
	bool bResult = false;
	CStdioFile file(fp);
	try
	{
		CreateHash(&file, uSize, pucHash, pShaHashOut, slowdown); //Xman Nice Hash
		bResult = true;
	}
	catch(CFileException* ex)
	{
		ex->Delete();
	}
	return bResult;
}

bool CKnownFile::CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut, bool slowdown) const //Xman Nice Hash
{
	bool bResult = false;
	CMemFile file(const_cast<uchar*>(pucData), uSize);
	try
	{
		CreateHash(&file, uSize, pucHash, pShaHashOut, slowdown); //Xman Nice Hash
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
		return NULL;
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
	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		const CUpDownClient *cur_src = m_ClientUploadList.GetNext(pos);
			//if (cur_src->HasLowID() || cur_src == forClient || !(cur_src->GetUploadState() == US_UPLOADING || cur_src->GetUploadState() == US_ONUPLOADQUEUE))
			if ((cur_src->HasLowID() && !forClient->SupportsNeoXS()) || cur_src == forClient || !(cur_src->GetUploadState() == US_UPLOADING || cur_src->GetUploadState() == US_ONUPLOADQUEUE)) // NEO: NXS - [NeoXS] <-- Xanatos --
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
				// CryptSettings - SourceExchange V4
				// 5 Reserved (!)
				// 1 CryptLayer Required
				// 1 CryptLayer Requested
				// 1 CryptLayer Supported
				const uint8 uSupportsCryptLayer	= cur_src->SupportsCryptLayer() ? 1 : 0;
				const uint8 uRequestsCryptLayer	= cur_src->RequestsCryptLayer() ? 1 : 0;
				const uint8 uRequiresCryptLayer	= cur_src->RequiresCryptLayer() ? 1 : 0;
				const uint8 byCryptOptions = (uRequiresCryptLayer << 2) | (uRequestsCryptLayer << 1) | (uSupportsCryptLayer << 0);
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
	if ( result->size > 354 )
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
	if(m_iUpPriority!=iNewUpPriority)
	{
	m_iUpPriority = iNewUpPriority;
		theApp.emuledlg->sharedfileswnd->sharedfilesctrl.UpdateFile(this,true);
	}
	else
		m_iUpPriority = iNewUpPriority;
	//Xman end

	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH || m_iUpPriority == PR_POWER ); //Xman PowerRelease

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

	// 05-J�n-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
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
	// 05-J�n-2004 [bc]: ed2k and Kad are already full of totally wrong and/or not properly attached meta data. Take
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

#if _MSC_VER<1400
		if (thePrefs.GetExtractMetaData() >= 2)
		{
			// starting the MediaDet object takes a noticeable amount of time.. avoid starting that object
			// for files which are not expected to contain any Audio/Video data.
			// note also: MediaDet does not work well for too short files (e.g. 16K)
			EED2KFileType eFileType = GetED2KFileTypeID(GetFileName());
			if ((eFileType == ED2KFT_AUDIO || eFileType == ED2KFT_VIDEO) && GetFileSize() >= (uint64)32768)
			{
				// Avoid processing of some file types which are known to crash due to bugged DirectShow filters.
				TCHAR szExt[_MAX_EXT];
				_tsplitpath(GetFileName(), NULL, NULL, NULL, szExt);
				_tcslwr(szExt);
				if (_tcscmp(szExt, _T(".ogm"))!=0 && _tcscmp(szExt, _T(".ogg"))!=0 && _tcscmp(szExt, _T(".mkv"))!=0)
				{
					TCHAR szFullPath[MAX_PATH];
					if (_tmakepathlimit(szFullPath, NULL, GetPath(), GetFileName(), NULL))
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
#endif
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
	if(GetFileComment() != "")
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

	if( theApp.IsFirewalled() )
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

//MORPH START - Added by IceCream, music preview
bool CKnownFile::IsMusic() const
{
	return (ED2KFT_AUDIO == GetED2KFileTypeID(GetFileName()) );
}
//MORPH END   - Added by IceCream, music preview

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
//remove mobile
	 if (theApp.clientlist->IsValidClient((CUpDownClient*)pSender)){

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
			return GetResString(IDS_PRIORELEASE);
		//Xman PowerRelease
		case PR_POWER:
			return GetResString(IDS_POWERRELEASE);
		//Xman end
		default:
			return _T("");
	}
}

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
