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
#include "ClientList.h"
#include "opcodes.h"
#include "ini2.h"
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
#include "MuleStatusBarCtrl.h" //Xman Progress Hash (O2)
#include "TransferWnd.h"

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

//IMPLEMENT_DYNAMIC(CKnownFile, CShareableFile)

CKnownFile::CKnownFile()
{
	m_iPartCount = 0;
	m_iED2KPartCount = 0;
	m_tUtcLastModified = 0;
	if(thePrefs.GetNewAutoUp()){
		m_iUpPriority = PR_HIGH;
		m_bAutoUpPriority = true;
	}
	else{
		m_iUpPriority = PR_NORMAL;
		m_bAutoUpPriority = false;
	}
	statistic.fileParent = this;
	m_PublishedED2K = false;
	kadFileSearchID = 0;
	SetLastPublishTimeKadSrc(0,0);
	m_nCompleteSourcesTime = time(NULL);
	m_nCompleteSourcesCount = 1;
	m_nCompleteSourcesCountLo = 1;
	m_nCompleteSourcesCountHi = 1;
	m_uMetaDataVer = 0;
	m_lastPublishTimeKadSrc = 0;
	m_lastBuddyIP = 0;
	m_pCollection = NULL;
	m_bAICHRecoverHashSetAvailable = false;
	onuploadqueue = 0; //Xman see OnUploadqueue	
	// Maella -One-queue-per-file- (idea bloodymad)
	m_startUploadTime = ::GetTickCount();
	// Maella end
	//Xman show virtual sources (morph)
	m_nVirtualCompleteSourcesCount = 0;
	m_iPermissions = -1;//Upload Permission
	//Xman advanced upload-priority
	pushfaktor=0;
	m_nVirtualUploadSources = 0;
	//Xman end
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
	m_SOTNAvailPartFrequency.AssertValid(); // morph4u :: SOTN
	(void)m_strDirectory;
	(void)m_strFilePath;
	(void)m_iPartCount;
	(void)m_iED2KPartCount;
	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH || m_iUpPriority == PR_POWER ); //Xman PowerRelease
	CHECK_BOOL(m_bAutoUpPriority);
	//(void)s_ShareStatusBar; //Xman
	CHECK_BOOL(m_PublishedED2K);
	(void)kadFileSearchID;
	(void)m_lastPublishTimeKadSrc;
	(void)m_lastBuddyIP;
	(void)wordlist;
}

void CKnownFile::Dump(CDumpContext& dc) const
{
	CAbstractFile::Dump(dc);
}
#endif

void CKnownFile::UpdatePartsInfo()
{
	// Cache part count
	size_t partcount = GetPartCount();
	bool flag = (time(NULL) - m_nCompleteSourcesTime > 0); 

	// Reset part counters
	if (m_AvailPartFrequency.GetCount() < partcount)
		m_AvailPartFrequency.SetCount(partcount);
// morph4u :: SOTN :: Start
	if ((size_t)m_SOTNAvailPartFrequency.GetCount() < partcount)
		m_SOTNAvailPartFrequency.SetCount(partcount);
// morph4u :: SOTN :: End
	for (size_t i = 0; i < partcount; i++)
        {
		m_AvailPartFrequency[i] = 0;
		m_SOTNAvailPartFrequency[i] = 0; // morph4u :: SOTN 
	}

	CAtlArray<uint16> count;
	if (flag)
		count.SetCount(0, m_ClientUploadList.GetCount());
	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		//This could be a partfile that just completed.. Many of these clients will not have this information.
		if (cur_src->m_abyUpPartStatus && cur_src->GetUpPartCount() == partcount)
		{
			for (size_t i = 0; i < partcount; i++)
			{
				if (cur_src->IsUpPartAvailable(i))
					m_AvailPartFrequency[i] += 1;
			}
			if (flag)
				count.Add(cur_src->GetUpCompleteSourcesCount());
		}
		cur_src->GetUploadingAndUploadedPart(m_AvailPartFrequency, m_SOTNAvailPartFrequency); // morph4u :: SOTN 
	}

	if (flag)
	{
		m_nCompleteSourcesCount = m_nCompleteSourcesCountLo = m_nCompleteSourcesCountHi = 0;

		if (partcount > 0)
			m_nCompleteSourcesCount = m_AvailPartFrequency[0];
		for (size_t i = 1; i < partcount; i++)
		{
			if (m_nCompleteSourcesCount > m_AvailPartFrequency[i])
				m_nCompleteSourcesCount = m_AvailPartFrequency[i];
		}
		//Xman show virtual sources
		m_nVirtualCompleteSourcesCount=m_nCompleteSourcesCount;
		//Xman end

		count.Add(m_nCompleteSourcesCount+1); // plus 1 since we have the file complete too

		size_t n = count.GetCount();
		if (n > 0)
		{
			// SLUGFILLER: heapsortCompletesrc
			size_t r;
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
			INT_PTR i = n >> 1;			// (n / 2)
			INT_PTR j = (n * 3) >> 2;	// (n * 3) / 4
			INT_PTR k = (n * 7) >> 3;	// (n * 7) / 8

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
	for (size_t i = 0; i < partcount; i++){
		if(m_AvailPartFrequency[i] < m_nVirtualCompleteSourcesCount)
			m_nVirtualCompleteSourcesCount = m_AvailPartFrequency[i];
	}
	*/
	//Xman end

	//>>> WiZaRd::Optimization
	if (theApp.sharedfiles)
		theApp.sharedfiles->UpdateFile(this);
	//if (theApp.emuledlg->transferwnd->m_hWnd)
	//	theApp.emuledlg->transferwnd->sharedfilesctrl.UpdateFile(this);
	//<<< WiZaRd::Optimization
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

	m_AvailPartFrequency.SetCount(GetPartCount());
	m_SOTNAvailPartFrequency.SetCount(GetPartCount()); // morph4u :: SOTN :: Start
	for (uint16 i = 0; i < GetPartCount();i++)
{
		m_AvailPartFrequency[i] = 0;
		m_SOTNAvailPartFrequency[i] = 0; // morph4u :: SOTN :: Start
	}
	
	// create hashset
	CAICHRecoveryHashSet cAICHHashSet(this, m_nFileSize);
	uint64 togo = m_nFileSize;
	UINT hashcount;
	for (hashcount = 0; togo >= PARTSIZE; )
	{
		CAICHHashTree* pBlockAICHHashTree = cAICHHashSet.m_pHashTree.FindHash((uint64)hashcount*PARTSIZE, PARTSIZE);
		ASSERT( pBlockAICHHashTree != NULL );

		uchar* newhash = new uchar[16];
		if (!CreateHash(file, PARTSIZE, newhash, pBlockAICHHashTree, true)) { //Xman Nice Hash
			LogError(_T("Failed to hash file \"%s\" - %s"), strFilePath, _tcserror(errno));
			fclose(file);
			delete[] newhash;
			return false;
		}

		if (!CemuleDlg::IsRunning()){ // in case of shutdown while still hashing
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

		if (pvProgressParam && theApp.emuledlg && CemuleDlg::IsRunning()){
			ASSERT( IsKindOfCKnownFile((CKnownFile*)pvProgressParam)/*((CKnownFile*)pvProgressParam)->IsKindOf(RUNTIME_CLASS(CKnownFile))*/ );
			ASSERT( ((CKnownFile*)pvProgressParam)->GetFileSize() == GetFileSize() );
			uint_ptr uProgress = (uint_ptr)(uint64)(((uint64)(GetFileSize() - togo) * 100) / GetFileSize());
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
	if (!CreateHash(file, togo, lasthash, pBlockAICHHashTree, true)) { //Xman Nice Hash
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

	if (pvProgressParam && theApp.emuledlg && CemuleDlg::IsRunning()){
		ASSERT( IsKindOfCKnownFile((CKnownFile*)pvProgressParam)/*((CKnownFile*)pvProgressParam)->IsKindOf(RUNTIME_CLASS(CKnownFile))*/ );
		ASSERT( ((CKnownFile*)pvProgressParam)->GetFileSize() == GetFileSize() );
		uint_ptr uProgress = 100;
		ASSERT( uProgress <= 100 );
		VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)pvProgressParam) );
	}

	// set lastwrite date
	struct _stat64 fileinfo;
	if (_fstat64(file->_file, &fileinfo) == 0){
		m_tUtcLastModified = fileinfo.st_mtime;
		AdjustNTFSDaylightFileTime(m_tUtcLastModified, strFilePath);
	}
	else if (thePrefs.GetVerbose())
		AddDebugLogLine(false, _T("Failed to get file date of \"%s\""), strFilePath);
	
	fclose(file);


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

		if (!CemuleDlg::IsRunning()){ // in case of shutdown while still hashing
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
					m_AvailPartFrequency.SetCount(GetPartCount());
					m_SOTNAvailPartFrequency.SetCount(GetPartCount()); // morph4u :: SOTN 
					for (uint16 i = 0; i < GetPartCount();i++)
                                         {
						m_AvailPartFrequency[i] = 0;
						m_SOTNAvailPartFrequency[i] = 0; // morph4u :: SOTN 
					}
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
						if (m_iUpPriority != PR_VERYLOW && m_iUpPriority != PR_LOW && m_iUpPriority != PR_NORMAL && m_iUpPriority != PR_HIGH && m_iUpPriority != PR_VERYHIGH && m_iUpPriority != PR_POWER) //Xman PowerRelease
							m_iUpPriority = PR_NORMAL;
						m_bAutoUpPriority = false;
					}
				}
				delete newtag;
				break;
			}
			case FT_KADLASTPUBLISHSRC:{
				ASSERT( newtag->IsInt64() );
				if (newtag->IsInt64())
					SetLastPublishTimeKadSrc( newtag->GetInt64(), 0 );
				if(GetLastPublishTimeKadSrc() > (uint64)time(NULL)+KADEMLIAREPUBLISHTIMES)
				{
					//There may be a posibility of an older client that saved a random number here.. This will check for that..
					SetLastPublishTimeKadSrc(0,0);
				}
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

//Upload Permission +
			case FT_PERMISSIONS:{
				ASSERT( newtag->IsInt() );
				if (newtag->IsInt())
				{
					m_iPermissions = newtag->GetInt();
					if (m_iPermissions != PERM_ALL  && m_iPermissions != PERM_NOONE && m_iPermissions != PERM_UPMANA)
						m_iPermissions = -1;
                                }
				delete newtag;
				break;
			}
//Upload Permission -
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
				if (DecodeBase32(newtag->GetStr(),hash) == HASHSIZE)
					m_FileIdentifier.SetAICHHash(hash);
				else
					ASSERT( false );
				delete newtag;
				break;
			}
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
			case FT_LASTSHARED:
				if(statistic.m_tlastdataupdate != 0)
					break;
			case FT_LASTDATAUPDATE:
			{
				ASSERT( newtag->IsInt64(true) );// X: [64T] - [64BitTime]
				if (newtag->IsInt64(true))
					statistic.m_tlastdataupdate = newtag->GetInt64();
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
			// ==>  Take care of corrupted tags [Mighty Knife] - Stulle
					delete newtag;
					break;
				}
				// <==  Take care of corrupted tags [Mighty Knife] - Stulle
				ConvertED2KTag(newtag);
				if (newtag)
					taglist.Add(newtag);
				break;
		}
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
	if (bHadAICHHashSetTag)
	{
		if (!m_FileIdentifier.VerifyAICHHashSet())
			DebugLogError(_T("Failed to load AICH Part HashSet for file %s"), GetFileName());
		//else
		//	DebugLog(_T("Succeeded to load AICH Part HashSet for file %s"), GetFileName());
	}

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
	//Xman
	return m_nFileSize!=(uint64)0;		// SLUGFILLER: SafeHash - Must have a filesize tag
}

bool CKnownFile::LoadDateFromFile(CFileDataIO* file, bool I64Time){// X: [E64T] - [Enable64BitTime]
	m_tUtcLastModified = I64Time?file->ReadUInt64():file->ReadUInt32();// X: [E64T] - [Enable64BitTime]
	return true;
}

bool CKnownFile::LoadFromFile(CFileDataIO* file, bool I64Time){// X: [E64T] - [Enable64BitTime]
	// SLUGFILLER: SafeHash - load first, verify later
	bool ret1 = LoadDateFromFile(file,I64Time);// X: [E64T] - [Enable64BitTime]
	bool ret2 = m_FileIdentifier.LoadMD4HashsetFromFile(file, false);
	bool ret3 = LoadTagsFromFile(file);
	UpdatePartsInfo();
	return ret1 && ret2 && ret3 && m_FileIdentifier.HasExpectedMD4HashCount();// Final hash-count verification, needs to be done after the tags are loaded
	// SLUGFILLER: SafeHash
}

bool CKnownFile::WriteToFile(CFileDataIO* file,bool I64Time)// X: [E64T] - [Enable64BitTime]
{
	// date
	I64Time?file->WriteUInt64(m_tUtcLastModified):file->WriteUInt32((uint32)m_tUtcLastModified);// X: [E64T] - [Enable64BitTime]

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
	/*static bool sDbgWarnedOnZero = false;
	if (!sDbgWarnedOnZero && m_timeLastSeen == 0)
	{
		DebugLog(_T("Unknown last seen date on stored file(s), upgrading from old version?"));
		sDbgWarnedOnZero = true;
	}
	ASSERT( m_timeLastSeen <= time(NULL) );
	time_t timeLastShared = (m_timeLastSeen > 0 && m_timeLastSeen <= time(NULL)) ? m_timeLastSeen : time(NULL);
	CTag lastSharedTag(FT_LASTSHARED, timeLastShared, I64Time);// X: [E64T] - [Enable64BitTime]
	lastSharedTag.WriteTagToFile(file);
	uTagCount++;*/
	if (statistic.m_tlastdataupdate!=0)
	{
		CTag stag1(FT_LASTSHARED, statistic.m_tlastdataupdate,I64Time);// X: [E64T] - [Enable64BitTime]
		stag1.WriteTagToFile(file);
	uTagCount++;		
	}

	if (!ShouldPartiallyPurgeFile())
	{
		// those tags are no longer stored for long time not seen (shared) known files to tidy up known.met and known2.met

		// AICH Part HashSet
		// no point in permanently storing the AICH part hashset if we need to rehash the file anyway to fetch the full recovery hashset
		// the tag will make the known.met incompatible with emule version prior 0.44a - but that one is nearly 6 years old 
		if (m_FileIdentifier.HasAICHHash() && m_FileIdentifier.HasExpectedAICHHashCount())
		{
			uint32 nAICHHashSetSize = (HASHSIZE * (m_FileIdentifier.GetAvailableAICHPartHashCount() + 1)) + 2;
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
			CTag kadLastPubSrc(FT_KADLASTPUBLISHSRC, m_lastPublishTimeKadSrc,I64Time);// X: [E64T] - [Enable64BitTime]
			kadLastPubSrc.WriteTagToFile(file);
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
//Upload Permission +
		if (GetPermissions()>=0){
			CTag permtag(FT_PERMISSIONS, GetPermissions());
			permtag.WriteTagToFile(file);
			uTagCount++;
		}
//Upload Permission -
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
		/*if (statistic.m_tlastdataupdate!=0)
		{
			CTag stag1(FT_LASTDATAUPDATE, statistic.m_tlastdataupdate,I64Time);// X: [E64T] - [Enable64BitTime]
			stag1.WriteTagToFile(file);
			uTagCount++;
		}*/
		//Xman end

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			char namebuffer[10];
			char* number = &namebuffer[1];
			UINT i_pos = 0;
			if (IsLargeFile()) {
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
					_itoa(i_pos,number,10); //Fafner: avoid C4996 (as in 0.49b vanilla) - 080731
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
					_itoa(i_pos,number,10); //Fafner: avoid C4996 (as in 0.49b vanilla) - 080731
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
		// <== Spread bars [Slugfiller/MorphXT] - Stulle

		// other tags
		for (size_t j = 0; j < taglist.GetCount(); j++){
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

void CKnownFile::CreateHash(CFile* pFile, uint64 Length, uchar* pMd4HashOut, CAICHHashTree* pShaHashOut, bool slowdown) //Xman Nice Hash
{
	ASSERT( pFile != NULL );
	ASSERT( pMd4HashOut != NULL || pShaHashOut != NULL );
	//Xman
	Poco::FastMutex::SingleLock sLock1(theApp.hashing_mut, true);	// SLUGFILLER: SafeHash - only one chunk-hash at a time

	uint64  Required = Length;
	uchar   X[64*128];
	uint64	posCurrentEMBlock = 0;
	uint64	nAICHPos = 0;
	CMD4 md4;
	CAICHHashAlgo* pHashAlg = NULL;
	if (pShaHashOut != NULL)
		pHashAlg = CAICHRecoveryHashSet::GetNewHashAlgo();

	//Xman Nice Hash
	uint32 timeStart = ::GetTickCount();  
	//Xman end

	while (Required >= 64){
		uint32 len; 
		if ((Required / 64) > _countof(X)/64) 
			len = _countof(X)/64;
		else
			len = (uint32)Required / 64;
		pFile->Read(X, len*64);

		// SHA hash needs 180KB blocks
		if (pShaHashOut != NULL && pHashAlg != NULL){
			if (nAICHPos + len*64 >= EMBLOCKSIZE){
				uint32 nToComplete = (uint32)(EMBLOCKSIZE - nAICHPos);
				pHashAlg->Add(X, nToComplete);
				ASSERT( nAICHPos + nToComplete == EMBLOCKSIZE );
				pShaHashOut->SetBlockHash(EMBLOCKSIZE, posCurrentEMBlock, pHashAlg);
				posCurrentEMBlock += EMBLOCKSIZE;
				pHashAlg->Reset();
				pHashAlg->Add(X+nToComplete,(len*64) - nToComplete);
				nAICHPos = (len*64) - nToComplete;
			}
			else{
				pHashAlg->Add(X, len*64);
				nAICHPos += len*64;
			}
		}

		if (pMd4HashOut != NULL){
			md4.Add(X, len*64);
		}
		Required -= len*64;
		//Xman Nice Hash
		if(slowdown && ::GetTickCount() - timeStart >= 100)   
		{
			Sleep(10);       
			timeStart = ::GetTickCount();       
		}       
		//Xman end
	}

	Required = Length % 64;
	if (Required != 0){
		pFile->Read(X, (uint32)Required);

		if (pShaHashOut != NULL){
			if (nAICHPos + Required >= EMBLOCKSIZE){
				uint32 nToComplete = (uint32)(EMBLOCKSIZE - nAICHPos);
				pHashAlg->Add(X, nToComplete);
				ASSERT( nAICHPos + nToComplete == EMBLOCKSIZE );
				pShaHashOut->SetBlockHash(EMBLOCKSIZE, posCurrentEMBlock, pHashAlg);
				posCurrentEMBlock += EMBLOCKSIZE;
				pHashAlg->Reset();
				pHashAlg->Add(X+nToComplete, (uint32)(Required - nToComplete));
				nAICHPos = Required - nToComplete;
			}
			else{
				pHashAlg->Add(X, (uint32)Required);
				nAICHPos += Required;
			}
		}
	}
	if (pShaHashOut != NULL){
		if(nAICHPos > 0){
			pShaHashOut->SetBlockHash(nAICHPos, posCurrentEMBlock, pHashAlg);
			posCurrentEMBlock += nAICHPos;
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

bool CKnownFile::CreateHash(FILE* fp, uint64 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut, bool slowdown) //Xman Nice Hash
{
	CStdioFile file(fp);
	try
	{
		CreateHash(&file, uSize, pucHash, pShaHashOut, slowdown); //Xman Nice Hash
		return true;
	}
	catch(CFileException* ex)
	{
		ex->Delete();
	}
	return false;
}

bool CKnownFile::CreateHash(const uchar* pucData, uint32 uSize, uchar* pucHash, CAICHHashTree* pShaHashOut, bool slowdown) //Xman Nice Hash
{
	CMemFile file(const_cast<uchar*>(pucData), uSize);
	try
	{
		CreateHash(&file, uSize, pucHash, pShaHashOut, slowdown); //Xman Nice Hash
		return true;
	}
	catch(CFileException* ex)
	{
		ex->Delete();
	}
	return false;
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
		const CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		//*const*/ CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		/*
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
		}*/

		if (cur_src->HasLowID() || cur_src == forClient || !(cur_src->GetUploadState() == US_UPLOADING || cur_src->GetUploadState() == US_ONUPLOADQUEUE))
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
			uint32 dwID = (byUsedVersion >= 3)?cur_src->GetUserIDHybrid():cur_src->GetIP();
			data.WriteUInt32(dwID);
			data.WriteUInt16(cur_src->GetUserPort());
			data.WriteUInt32(cur_src->GetServerIP());
			data.WriteUInt16(cur_src->GetServerPort());
			if (byUsedVersion >= 2){
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

	Packet* result = new Packet(&data, OP_EMULEPROT, bIsSX2Packet ? OP_ANSWERSOURCES2 : OP_ANSWERSOURCES);
	// (1+)16+2+501*(4+2+4+2+16+1) = 14547 (14548) bytes max.
	if (result->size > 354)
		result->PackPacket();
	if (thePrefs.GetDebugSourceExchange())
		AddDebugLogLine(false, _T("SXSend: Client source response SX2=%s, Version=%u; Count=%u, %s, File=\"%s\""), bIsSX2Packet ? _T("Yes") : _T("No"), byUsedVersion, nCount, forClient->DbgGetClientInfo(), GetFileName());
	return result;
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
		}
		return;
	}
	//Maella end
	if ( GetOnUploadqueue() > 100 ){
		if( GetUpPriority() != PR_LOW ){
			SetUpPriority( PR_LOW,false );
		}
		return;
	}
	if ( GetOnUploadqueue() > 15 ){
		if( GetUpPriority() != PR_NORMAL ){
			SetUpPriority( PR_NORMAL,false );
		}
		return;
	}
	if( GetUpPriority() != PR_HIGH){
		SetUpPriority( PR_HIGH,false );
	}
}
//Xman end

//Upload Permission +
void CKnownFile::SetPermissions(int iNewPermissions)
{
	ASSERT( m_iPermissions == -1 || m_iPermissions == PERM_ALL || m_iPermissions == PERM_NOONE || m_iPermissions == PERM_UPMANA);
	m_iPermissions = iNewPermissions;
}
//Upload Permission -

void CKnownFile::SetUpPriority(uint8 iNewUpPriority, bool bSave)
{
	//Xman advanced upload-priority
	if(m_iUpPriority!=iNewUpPriority)
	{
		m_iUpPriority = iNewUpPriority;
		//>>> WiZaRd::Optimization
		theApp.sharedfiles->UpdateFile(this);
		//theApp.emuledlg->transferwnd->sharedfilesctrl.UpdateFile(this);
		//<<< WiZaRd::Optimization
	}
	//else
		//m_iUpPriority = iNewUpPriority;
	//Xman end

	ASSERT( m_iUpPriority == PR_VERYLOW || m_iUpPriority == PR_LOW || m_iUpPriority == PR_NORMAL || m_iUpPriority == PR_HIGH || m_iUpPriority == PR_VERYHIGH || m_iUpPriority == PR_POWER ); //Xman PowerRelease

	if( IsPartFile() && bSave )
		((CPartFile*)this)->SavePartFile();
}

void SecToTimeLength(uint_ptr ulSec, CStringA& rstrTimeLength)
{
	// this function creates the content for the "length" ed2k meta tag which was introduced by eDonkeyHybrid 
	// with the data type 'string' :/  to save some bytes we do not format the duration with leading zeros
	if (ulSec >= 3600){
		uint32 uHours =(uint32) ulSec/3600;
		uint32 uMin =(uint32) (ulSec - uHours*3600)/60;
		uint32 uSec =(uint32) (ulSec - uHours*3600 - uMin*60);
		rstrTimeLength.Format("%u:%02u:%02u", uHours, uMin, uSec);
	}
	else{
		uint32 uMin =(uint32) ulSec/60;
		uint32 uSec =(uint32) (ulSec - uMin*60);
		rstrTimeLength.Format("%u:%02u", uMin, uSec);
	}
}

void SecToTimeLength(uint_ptr ulSec, CStringW& rstrTimeLength)
{
	// this function creates the content for the "length" ed2k meta tag which was introduced by eDonkeyHybrid 
	// with the data type 'string' :/  to save some bytes we do not format the duration with leading zeros
	if (ulSec >= 3600){
		uint32 uHours =(uint32) ulSec/3600;
		uint32 uMin =(uint32) (ulSec - uHours*3600)/60;
		uint32 uSec =(uint32) (ulSec - uHours*3600 - uMin*60);
		rstrTimeLength.Format(L"%u:%02u:%02u", uHours, uMin, uSec);
	}
	else{
		UINT uMin =(uint32) ulSec/60;
		UINT uSec =(uint32) (ulSec - uMin*60);
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
			size_t i = 0;
			while (i < taglist.GetCount())
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

	for (size_t j = 0; j < _countof(_aEmuleMetaTags); j++)
	{
		size_t i = 0;
		while (i < taglist.GetCount())
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

void CKnownFile::SetPublishedED2K(bool val){
	m_PublishedED2K = val;
	//>>> WiZaRd::Optimization
	theApp.sharedfiles->UpdateFile(this);
	//theApp.emuledlg->transferwnd->sharedfilesctrl.UpdateFile(this);
	//<<< WiZaRd::Optimization
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
				SetLastPublishTimeKadSrc( (uint64)time(NULL)+KADEMLIAREPUBLISHTIMES, lastBuddyIP );
				return true;
			}
		}
		else
			return false;
	}

	if(m_lastPublishTimeKadSrc > (uint64)time(NULL))
		return false;

	SetLastPublishTimeKadSrc((uint64)time(NULL)+KADEMLIAREPUBLISHTIMES,lastBuddyIP);
	return true;
}

bool CKnownFile::IsMovie() const
{
	return (ED2KFT_VIDEO == GetED2KFileTypeID(GetFileName()) );
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
		strType = _T('-');
#ifdef _DEBUG
	CString dbgInfo;
	dbgInfo.Format(_T("\nAICH Part HashSet: %s\nAICH Rec HashSet: %s"), m_FileIdentifier.HasExpectedAICHHashCount() ? _T("Yes") : _T("No")
		, IsAICHRecoverHashSetAvailable() ? _T("Yes") : _T("No"));
#endif

	CString info;
	info.Format(_T("%s\n")
		_T("eD2K ") + GetResString(IDS_FD_HASH) + _T(" %s\n")
		+ GetResString(IDS_AICHHASH) + _T(": %s\n")
		+ GetResString(IDS_FD_SIZE) + _T(" %s\n<br>\n")
		+ GetResString(IDS_TYPE) + _T(": %s\n")
		+ GetResString(IDS_FOLDER) + _T(": %s\n\n")
		+ GetResString(IDS_PRIORITY) + _T(": %s\n")
		+ GetResString(IDS_SF_REQUESTS) + _T(": %s\n")
		+ GetResString(IDS_SF_ACCEPTS) + _T(": %s\n")
		+ GetResString(IDS_SF_TRANSFERRED) + _T(": %s")
#ifdef _DEBUG
		_T("%s")
#endif
		,
		GetFileName(),
		md4str(GetFileHash()),
		m_FileIdentifier.GetAICHHash().GetString(),
		CastItoXBytes(GetFileSize(), false, false),
		strType,
		strFolder,
		GetUpPriorityDisplayString(),
		strRequests,
		strAccepts,
		strTransferred
#ifdef _DEBUG
		,dbgInfo
#endif
		);
	return info;
}

CString CKnownFile::GetUpPriorityDisplayString() const {
	switch (GetUpPriority()) {
		case PR_VERYLOW :
			return GetResString(IDS_PRIOVERYLOW);
		case PR_LOW :
			return GetResString(IsAutoUpPriority()?IDS_PRIOAUTOLOW:IDS_PRIOLOW);
		case PR_NORMAL :
			return GetResString(IsAutoUpPriority()?IDS_PRIOAUTONORMAL:IDS_PRIONORMAL);
		case PR_HIGH :
			return GetResString(IsAutoUpPriority()?IDS_PRIOAUTOHIGH:IDS_PRIOHIGH);
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
	return thePrefs.DoPartiallyPurgeOldKnownFiles() && statistic.m_tlastdataupdate/*m_timeLastSeen*/ > 0
		&& (uint64)time(NULL) > statistic.m_tlastdataupdate/*m_timeLastSeen*/ && (uint64)time(NULL) - statistic.m_tlastdataupdate/*m_timeLastSeen*/ > OLDFILES_PARTIALLYPURGE;
}

//Xman
// Maella -One-queue-per-file- (idea bloodymad)
uint_ptr CKnownFile::GetFileScore(uint32 downloadingTime)
{

	// Score proportional to the waiting time since the last upload
	uint_ptr elapsedTime;
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
#ifdef _DEBUG
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
		
		uint_ptr virtualsources=GetVirtualSourceIndicator();

		
		AddDebugLogLine(false, _T("avg: %0.2f%% percent: %0.2f%% , wanted: %s, counted upload: %s, completed: %s, pushfaktor: %i, avg virtuals: %u, own virtuals: %u, file: %s"),avgpercent,uploadpercent,  CastItoXBytes(wantedUpload), CastItoXBytes(statistic.GetCountedTransferred()),CastItoXBytes(completedup),(sint16)((pushfaktor-1)*100),theApp.sharedfiles->m_avg_virtual_sources, virtualsources,this->GetFileName()); 

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
#endif
uint_ptr CKnownFile::GetVirtualSourceIndicator() const
	{
	uint_ptr fullsources;
		if(m_nCompleteSourcesCountHi > m_nCompleteSourcesCountLo)
			fullsources = m_nCompleteSourcesCountLo + (m_nCompleteSourcesCountHi-m_nCompleteSourcesCountLo)/4;
		else
			fullsources = m_nCompleteSourcesCountLo;
	if(IsPartFile())
	{
		return m_nVirtualUploadSources*2 + fullsources; 
	}
	else
	{
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

		uint_ptr virtualsources=GetVirtualSourceIndicator();
		
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
	size_t partcount = GetPartCount();

	CAtlArray<uint16> tmp_AvailPartFrequency;

	tmp_AvailPartFrequency.SetCount(partcount);
	for (size_t i = 0; i < partcount; i++)
		tmp_AvailPartFrequency[i] = 0;

	for (POSITION pos = m_ClientUploadList.GetHeadPosition(); pos != 0; )
	{
		CUpDownClient* cur_src = m_ClientUploadList.GetNext(pos);
		if (cur_src->m_abyUpPartStatus && cur_src->GetUpPartCount() == partcount)
		{
			for (size_t i = 0; i < partcount; i++)
			{
				if (cur_src->IsUpPartAvailable(i))
					tmp_AvailPartFrequency[i] += 1;
			}
		}
	}


	UINT nVirtualUploadSources = (UINT)-1;
	for (size_t i = 0; i < partcount; i++){
		if(tmp_AvailPartFrequency[i] < nVirtualUploadSources)
			nVirtualUploadSources = tmp_AvailPartFrequency[i];
	}


	if (m_nVirtualUploadSources!=nVirtualUploadSources)
	{
		m_nVirtualUploadSources=nVirtualUploadSources;
		//>>> WiZaRd::Optimization
		theApp.sharedfiles->UpdateFile(this);
		//if(theApp.emuledlg->transferwnd->m_hWnd)
			//theApp.emuledlg->transferwnd->sharedfilesctrl.UpdateFile(this);
		//<<< WiZaRd::Optimization
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
	else if(statistic.m_tlastdataupdate==0 || statistic.m_tlastdataupdate>(uint64)time(NULL))
	{
		//this file was uploaded with a previous version or there is anything wrong
		//count old upload 50%
		if(statistic.m_tlastdataupdate>(uint64)time(NULL))
			AddDebugLogLine(false,_T("found mismatch at date of file: %s"), GetFileName());
#ifdef _DEBUG
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

// morph4u :: SOTN :: Start
//true: something was hidden (i.e. partial file)
//false: nothing was hidden (i.e. complete file)
bool CKnownFile::WriteSafePartStatus(CSafeMemFile* data_out, CUpDownClient* sender, const bool bUDP)
{
	if (WritePartStatus(data_out, sender, bUDP))
		return true;
	data_out->WriteUInt16(0); //i.e. "complete file"
	return false;
}

bool CKnownFile::WritePartStatus(CSafeMemFile* file, CUpDownClient* client, const bool bUDP) 
{
	//First check whether we hide something...	
	uint8* m_abyTmpPartStatus = GetPartStatus(client);
	//if we do not hide something and the file is complete, return here
	const bool bPartfile = IsPartFile();
	if(!m_abyTmpPartStatus && !bPartfile)
		return false;

	//and now write it down...
	uint16 done = 0;
	uint16 parts = GetED2KPartCount();
	file->WriteUInt16(parts);
	const uint16 partcount = GetPartCount();
	//if the data is sent via UDP it is not guaranteed to arrive... so what to do!?
	const uint8 toSet = bUDP ? 2 : 1;
//>>> WiZaRd::FiX
	//check if we sent the status for the same file as we requested to download off that client
	//if NOT then we can't write the data because the partcount may differ, etc.
	//The best way to "fix" that would be to save all partstatus' we receive
	CKnownFile* upfile = client ? theApp.sharedfiles->GetFileByID(client->GetUploadFileID()) : NULL;
	const bool bSameFile = upfile ? (upfile == this) : false;
	const bool bWriteStatus = bSameFile && client && client->m_abyUpPartStatusHidden;
//<<< WiZaRd::FiX
	while (done != parts)
	{
		//fill one uint8 with data...
		uint8 towrite = 0;
		for (uint8 i = 0; i < 8; ++i)
		{
			if(done < partcount	//valid part
				//of course it's complete if it's a complete file
				//or the part is complete
				&& (!bPartfile || ((CPartFile*)this)->IsComplete(done*PARTSIZE, (done + 1)*PARTSIZE - 1, true)))
	{
				//and it's not hidden
				if(!m_abyTmpPartStatus || m_abyTmpPartStatus[done] == 0)
				{
					towrite |= (1<<i);				
//>>> WiZaRd::FiX
//					if(client && client->m_abyUpPartStatusHidden)
					if(bWriteStatus)
//<<< WiZaRd::FiX
						client->m_abyUpPartStatusHidden[done] = 0;  //not hidden anymore
				}
//>>> WiZaRd::FiX
//				else if(client && client->m_abyUpPartStatusHidden)
				else if(bWriteStatus)
//<<< WiZaRd::FiX
					client->m_abyUpPartStatusHidden[done] = toSet;
			}

			++done;
			if (done == parts)
				break;
		}
		file->WriteUInt8(towrite);
	}

	//cleanup
	delete[] m_abyTmpPartStatus;
	m_abyTmpPartStatus = NULL;

	return true;
        }

//this function is to retrieve the "show parts" map for a specific client
//WiZaRd: this works pretty good but there is ONE big issue:
//imagine that we send our partstatus to the first client... we are using SOTN so he will see only 2 chunks (max)
//now other clients will send their requests and we will answer but as we are still the only source, SOTN will return the same scheme
//and thus we will show the same parts (again)
//this could cause that the remote clients download the same parts until parts got completed and we are reasked :(
//solution!? well... we *could* parse the queue and count any part that has already been shown in some way...
//m_SOTNAvailPartFrequency counts up how often a chunk is visible to remote users
uint8* CKnownFile::GetPartStatus(CUpDownClient* client) const
{
	const uint16 parts = GetED2KPartCount();

	bool bSOTN = GetShareOnlyTheNeed();
//>>> Create tmp array
	//the problem is that m_AvailPartFrequency will only check clients in our queue!
	//so create a tmparray and update its values
	CAtlArray<uint16/*, uint16*/> tmparray;
	const uint16 partcount = GetPartCount();
	tmparray.SetCount(partcount);
	const bool bUsePart = IsPartFile();
	for(uint16 i = 0; i < partcount; i++)
	{
		if(bUsePart)
			tmparray[i] = ((CPartFile*)this)->GetSrcPartFrequency(i) + ((CPartFile*)this)->IsComplete(PARTSIZE*i, PARTSIZE*(i+1)-1, true);
		else if(!m_AvailPartFrequency.IsEmpty())
			tmparray[i] = m_AvailPartFrequency[i];
		if(!m_SOTNAvailPartFrequency.IsEmpty())
			tmparray[i] = tmparray[i] + m_SOTNAvailPartFrequency[i];
	}
//<<< Create tmp array
	uint16 iMinAvailablePartFrenquency = _UI16_MAX; 
	uint16 iMinAvailablePartFrenquencyPrev = _UI16_MAX;
	if(bSOTN)
	{
		//for (uint16 i = 0; i < parts; ++)i
		for (uint16 i = 0; i < partcount; ++i)
		{
			if ((!client						//we don't have to check client
				|| !client->IsPartAvailable(i))	//or he does not own that part
				//&& i < GetPartCount()			//and it's a valid part								
				&& (!bUsePart					//of course it's complete if it's a complete file
				|| ((CPartFile*)this)->IsComplete(PARTSIZE*i, PARTSIZE*(i+1)-1, true)) //or the part is complete
				)
			{
				if (tmparray[i] < iMinAvailablePartFrenquency)
					iMinAvailablePartFrenquency = tmparray[i];				
				else if (tmparray[i] < iMinAvailablePartFrenquencyPrev)
					iMinAvailablePartFrenquencyPrev = tmparray[i];
	}
			}
		if (iMinAvailablePartFrenquency == _UI16_MAX)
			bSOTN = false;
		}

	//First check if we hide something...
	bool bNeedThisFunction = bUsePart; //we need it for part files in any case...

	uint8* m_abyTmpPartStatus = new uint8[parts];
	memset(m_abyTmpPartStatus, 0, parts*sizeof(uint8)); //show all

	uint16 done = 0;
	while (done != parts)
	{
		for (uint8 i = 0; i < 8; ++i)
		{
			//We will send this chunk if either...
			//SOTN is activated and it's the rarest chunk...
			const bool bSOTNOK = !bSOTN || (tmparray[done] <= iMinAvailablePartFrenquencyPrev);

			//ok now the selection
			if(!bSOTNOK)
			{
				m_abyTmpPartStatus[done] = 1;
				bNeedThisFunction = true;
			}

			++done;
			if (done == parts)
				break;
		}	
	}

	if(!bNeedThisFunction)
	{
		delete[] m_abyTmpPartStatus;
		m_abyTmpPartStatus = NULL;
		}

	return m_abyTmpPartStatus;
}


bool CKnownFile::GetShareOnlyTheNeed() const
{
	if(IsPartFile())
		return false;
	
	return true;
}
// morph4u :: SOTN :: End