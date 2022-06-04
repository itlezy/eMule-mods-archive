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
#include "emule.h"
#include <zlib/zlib.h>
#include "UpDownClient.h"
#include "UrlClient.h"
#include "Opcodes.h"
#include "Packets.h"
#include "UploadQueue.h"
#include "Statistics.h"
#include "ClientList.h"
#include "ClientUDPSocket.h"
#include "SharedFileList.h"
#include "KnownFileList.h"
#include "PartFile.h"
#include "ClientCredits.h"
#include "ListenSocket.h"
//#include "PeerCacheSocket.h" // X: [RPC] - [Remove PeerCache]
#include "Sockets.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "Log.h"
#include "Collection.h"
#ifdef CLIENTANALYZER
#include "Addons/AntiLeech/ClientAnalyzer.h" //>>> WiZaRd::ClientAnalyzer
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//	members of CUpDownClient
//	which are mainly used for uploading functions 

CBarShader CUpDownClient::s_UpStatusBar(16);

void CUpDownClient::DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const
{
    COLORREF crNeither;
	COLORREF crNextSending;
	COLORREF crBoth;
	COLORREF crSending;

    if(GetSlotNumber() <= theApp.uploadqueue->GetActiveUploadsCount() ||
       (GetUploadState() != US_UPLOADING && GetUploadState() != US_CONNECTING) ) {
        crNeither = RGB(224, 224, 224);
	    crNextSending = RGB(255,208,0);
	    crBoth = bFlat ? RGB(0, 0, 0) : RGB(104, 104, 104);
	    crSending = RGB(0, 150, 0);
   } else {
        // grayed out
        crNeither = RGB(248, 248, 248);
	    crNextSending = RGB(255,244,191);
	    crBoth = bFlat ? RGB(191, 191, 191) : RGB(191, 191, 191);
	    crSending = RGB(191, 229, 191);
  }

	// wistily: UpStatusFix
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	EMFileSize filesize;
	if (currequpfile)
		filesize = currequpfile->GetFileSize();
	else
		filesize = (uint64)(PARTSIZE * (uint64)m_nUpPartCount);
	// wistily: UpStatusFix

    if(filesize > (uint64)0) {
	    s_UpStatusBar.SetFileSize(filesize); 
	    s_UpStatusBar.SetHeight(rect->bottom - rect->top); 
	    s_UpStatusBar.SetWidth(rect->right - rect->left); 
	    s_UpStatusBar.Fill(crNeither); 
	    if (!onlygreyrect && m_abyUpPartStatus) { 
            for (UINT i = 0; i < m_nUpPartCount; i++)
			    if (m_abyUpPartStatus[i])
				    s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crBoth);
// ZZUL-TRA :: SOTN :: Start
				else if (m_abyUpPartStatusHidden)
				{
					if(m_abyUpPartStatusHidden[i] == 1) //hidden via TCP
						s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1), RGB(0, 192, 192));
					else if(m_abyUpPartStatusHidden[i] == 2) //hidden via UDP
						s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1), RGB(0, 100, 100));
				}
// ZZUL-TRA :: SOTN :: End
	    }
        const Requested_Block_Struct* block;
	    if (!m_BlockRequests_queue.IsEmpty()){
		    block = m_BlockRequests_queue.GetHead();
		    if(block){
			    uint32 start = (uint32)(block->StartOffset/PARTSIZE);
			    s_UpStatusBar.FillRange((uint64)start*PARTSIZE, (uint64)(start+1)*PARTSIZE, crNextSending);
		    }
	    }
	    if (!m_DoneBlocks_list.IsEmpty()){
		    block = m_DoneBlocks_list.GetHead();
		    if(block){
			    uint32 start = (uint32)(block->StartOffset/PARTSIZE);
			    s_UpStatusBar.FillRange((uint64)start*PARTSIZE, (uint64)(start+1)*PARTSIZE, crNextSending);
		    }
	    }
	    if (!m_DoneBlocks_list.IsEmpty()){
		    for(POSITION pos=m_DoneBlocks_list.GetHeadPosition();pos!=0;){
			    block = m_DoneBlocks_list.GetNext(pos);
			    s_UpStatusBar.FillRange(block->StartOffset, block->EndOffset + 1, crSending);
		    }
	    }
   	    s_UpStatusBar.Draw(dc, rect->left, rect->top, bFlat);
	    }
		    }

//MORPH START - Display current uploading chunk
void CUpDownClient::DrawUpStatusBarChunk(CDC* dc, RECT* rect, bool /*onlygreyrect*/, bool  bFlat) const
{
	COLORREF crNeither;
	COLORREF crNextSending;
	COLORREF crBoth;
	COLORREF crSending;
	COLORREF crBuffer;
	COLORREF crProgress;
#ifdef CHUNK_DOTS
	COLORREF crDot;
#endif
    if(GetSlotNumber() <= theApp.uploadqueue->GetActiveUploadsCount() ||
       (GetUploadState() != US_UPLOADING && GetUploadState() != US_CONNECTING) ) {
        crNeither = RGB(224, 224, 224);
	    crNextSending = RGB(255,208,0);
	    crBoth = bFlat ? RGB(0, 0, 0) : RGB(104, 104, 104);
	    crSending = RGB(0, 150, 0);
		crBuffer = RGB(255, 100, 100);
		crProgress = RGB(0, 224, 0);
#ifdef CHUNK_DOTS
		crDot = RGB(255, 255, 255);
#endif
    } else {
        // grayed out
        crNeither = RGB(248, 248, 248);
	    crNextSending = RGB(255,244,191);
	    crBoth = bFlat ? RGB(191, 191, 191) : RGB(191, 191, 191);
	    crSending = RGB(191, 229, 191);
		crBuffer = RGB(255, 216, 216);
		crProgress = RGB(191, 255, 191);
#ifdef CHUNK_DOTS
		crDot = RGB(255, 255, 255);
#endif
	    }

	// wistily: UpStatusFix
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	EMFileSize filesize;
	uint64 chunksize = PARTSIZE;
	if (currequpfile)
		filesize=currequpfile->GetFileSize();
	else
		filesize = (uint64)(PARTSIZE * (uint64)m_nUpPartCount);
	// wistily: UpStatusFix

	if(filesize <= (uint64)0)
		return;
    if (!m_BlockRequests_queue.IsEmpty() || !m_DoneBlocks_list.IsEmpty()) {
		uint32 cur_chunk = (uint32)-1;
		uint64 start = (uint64)-1;
		uint64 end = (uint64)-1;
		const Requested_Block_Struct* block;
		if (!m_DoneBlocks_list.IsEmpty()){
		    block = m_DoneBlocks_list.GetHead();
			if (cur_chunk == (uint32)-1) {
				cur_chunk = (uint32)(block->StartOffset/PARTSIZE);
				start = end = cur_chunk*PARTSIZE;
				end += PARTSIZE-1;
				if (end > filesize)
				{
					end = filesize;
					chunksize = end - start;
					if(chunksize <= 0) chunksize = PARTSIZE;
				}
				s_UpStatusBar.SetFileSize(chunksize);
				s_UpStatusBar.SetHeight(rect->bottom - rect->top); 
				s_UpStatusBar.SetWidth(rect->right - rect->left); 
				/*
				if (end > filesize) {
					end = filesize;
					s_UpStatusBar.Reset();
					s_UpStatusBar.FillRange(0, end%PARTSIZE, crNeither);
				} else
				*/
					s_UpStatusBar.Fill(crNeither);
			}
		}
		if (!m_BlockRequests_queue.IsEmpty()){
			for(POSITION pos=m_BlockRequests_queue.GetHeadPosition();pos!=0;){
				block = m_BlockRequests_queue.GetNext(pos);
			    if (cur_chunk == (uint32)-1) {
				    cur_chunk = (uint32)(block->StartOffset/PARTSIZE);
				    start = end = cur_chunk*PARTSIZE;
				    end += PARTSIZE-1;
					if (end > filesize)
					{
						end = filesize;
						chunksize = end - start;
						if(chunksize <= 0) chunksize = PARTSIZE;
					}
					s_UpStatusBar.SetFileSize(chunksize);
				    s_UpStatusBar.SetHeight(rect->bottom - rect->top); 
				    s_UpStatusBar.SetWidth(rect->right - rect->left); 
					/*
				    if (end > filesize) {
					    end = filesize;
					    s_UpStatusBar.Reset();
					    s_UpStatusBar.FillRange(0, end%PARTSIZE, crNeither);
				    } else
					*/
					s_UpStatusBar.Fill(crNeither);
			    }
				if (block->StartOffset <= end && block->EndOffset >= start) {
					s_UpStatusBar.FillRange((block->StartOffset > start)?block->StartOffset%PARTSIZE:(uint64)0, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crNextSending);
				}
			}
		}
		
		if (!m_DoneBlocks_list.IsEmpty() && cur_chunk != (uint32)-1){
		// Also show what data is buffered (with color crBuffer)
        uint64 total = 0;
    
		    for(POSITION pos=m_DoneBlocks_list.GetTailPosition();pos!=0; ){
			    block = m_DoneBlocks_list.GetPrev(pos);
				if (block->StartOffset <= end && block->EndOffset >= start) {
					if(total + (block->EndOffset-block->StartOffset) <= GetSessionPayloadUp()) {
						// block is sent
						s_UpStatusBar.FillRange((block->StartOffset > start)?block->StartOffset%PARTSIZE:(uint64)0, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crProgress);
						total += block->EndOffset-block->StartOffset;
					}
					else if (total < GetSessionPayloadUp()){
						// block partly sent, partly in buffer
						total += block->EndOffset-block->StartOffset;
						uint64 rest = total - GetSessionPayloadUp();
						uint64 newEnd = (block->EndOffset-rest);
						if (newEnd>=start) {
							if (newEnd<=end) {
								uint64 uNewEnd = newEnd%PARTSIZE;
								s_UpStatusBar.FillRange(block->StartOffset%PARTSIZE, uNewEnd, crSending);
								if (block->EndOffset <= end)
									s_UpStatusBar.FillRange(uNewEnd, block->EndOffset%PARTSIZE, crBuffer);
								else
									s_UpStatusBar.FillRange(uNewEnd, end%PARTSIZE, crBuffer);
							} else 
								s_UpStatusBar.FillRange(block->StartOffset%PARTSIZE, end%PARTSIZE, crSending);
						} else if (block->EndOffset <= end)
							s_UpStatusBar.FillRange((uint64)0, block->EndOffset%PARTSIZE, crBuffer);
					}
					else{
						// entire block is still in buffer
						total += block->EndOffset-block->StartOffset;
						s_UpStatusBar.FillRange((block->StartOffset>start)?block->StartOffset%PARTSIZE:(uint64)0, ((block->EndOffset < end)?block->EndOffset:end)%PARTSIZE, crBuffer);
					}
				} else
					total += block->EndOffset-block->StartOffset;
		    }
	    }
        s_UpStatusBar.Draw(dc, rect->left, rect->top, bFlat);
  
#ifdef CHUNK_DOTS    
		s_UpStatusBar.SetHeight(3); 
		s_UpStatusBar.SetWidth(1); 
		s_UpStatusBar.SetFileSize((uint64)1);
		s_UpStatusBar.Fill(crDot);
		uint32	w=rect->right-rect->left+1;
        if (!m_BlockRequests_queue.IsEmpty()){
			for(POSITION pos=m_BlockRequests_queue.GetHeadPosition();pos!=0;){
				block = m_BlockRequests_queue.GetNext(pos);
					if (block->StartOffset <= end && block->EndOffset >= start) {
					if (block->StartOffset >= start) {
						if (block->EndOffset <= end) {
								s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
								s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
						} else
								s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
					    } else if (block->EndOffset <= end)
							s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
				}
			}
		}	
		if (!m_DoneBlocks_list.IsEmpty()){
			for(POSITION pos=m_DoneBlocks_list.GetHeadPosition();pos!=0;){
				block = m_DoneBlocks_list.GetNext(pos);
					if (block->StartOffset <= end && block->EndOffset >= start) {
					if (block->StartOffset >= start) {
						if (block->EndOffset <= end) {
								s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
								s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
						} else
								s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
					    } else if (block->EndOffset <= end)
							s_UpStatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/(uint64)chunksize), rect->top, bFlat);
					}
			    }
			}
#endif
		}
	}
//MORPH END   - Display current uploading chunk

void CUpDownClient::SetUploadState(EUploadState eNewState)
{
	if (eNewState != m_nUploadState)
	{
		if (m_nUploadState == US_UPLOADING)
		{
			// Reset upload data rate computation
			m_nUpDatarate = 0;
			m_nSumForAvgUpDataRate = 0;
			m_AvarageUDR_list.RemoveAll();
		}
		if (eNewState == US_UPLOADING)
			m_fSentOutOfPartReqs = 0;

		// don't add any final cleanups for US_NONE here
		m_nUploadState = (_EUploadState)eNewState;
		theApp.emuledlg->transferwnd->GetClientList()->RefreshClient(this);
	}
}

/**
 * Gets the queue score multiplier for this client, taking into consideration client's credits
 * and the requested file's priority.
 */
float CUpDownClient::GetCombinedFilePrioAndCredit() {
	if (credits == 0){
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		return 0.0F;
	}

#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
	switch(thePrefs.UseCreditSystem())
	{
		case 1:	
    return 10.0f * credits->GetScoreRatio(GetIP()) * (float)GetFilePrioAsNumber();
		case 2:
	if (pAntiLeechData)
    return 10.0f * pAntiLeechData->GetScore() * (float)GetFilePrioAsNumber();
		default: //no antileech data? flow over to "no credit system"
		case 0: //no credits system selected... so why should the boost be applied?!
			return 10.0f * /*credits->GetScoreRatio(GetIP()) **/ (float)GetFilePrioAsNumber();
	}
//<<< WiZaRd::ClientAnalyzer
#else
    return 10.0f * credits->GetScoreRatio(GetIP()) * (float)GetFilePrioAsNumber();
#endif
}

/**
 * Gets the file multiplier for the file this client has requested.
 */
#ifdef CLIENTANALYZER
int GetFilePrio(const CKnownFile* currequpfile);
#endif
int CUpDownClient::GetFilePrioAsNumber() const {
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	
#ifdef CLIENTANALYZER
	return GetFilePrio(currequpfile);
}

//WiZaRd: optimized! there's no need to retrieve the reqfile if we already HAVE it
int GetFilePrio(const CKnownFile* currequpfile)
{
#endif
	// TODO coded by tecxx & herbert, one yet unsolved problem here:
	// sometimes a client asks for 2 files and there is no way to decide, which file the 
	// client finally gets. so it could happen that he is queued first because of a 
	// high prio file, but then asks for something completely different.
	int filepriority = 10; // standard
	switch(currequpfile->GetUpPriority()){
		case PR_VERYHIGH:
			filepriority = 18;
			break;
		case PR_HIGH: 
			filepriority = 9; 
			break; 
		case PR_LOW: 
			filepriority = 6; 
			break; 
		case PR_VERYLOW:
			filepriority = 2;
			break;
		case PR_NORMAL: 
			default: 
			filepriority = 7; 
		break; 
	} 

    return filepriority;
}

/**
 * Gets the current waiting score for this client, taking into consideration waiting
 * time, priority of requested file, and the client's credits.
 */
uint32 CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue) const
{
	if (!m_pszUsername)
		return 0;

	if (credits == 0){
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		return 0;
	}

#ifndef CLIENTANALYZER
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
#endif
	
	// bad clients (see note in function)
	if (credits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
		return 0;
	// friend slot
	if (IsFriend() && GetFriendSlot() && !HasLowID())
		return 0x0FFFFFFF;

#ifdef CLIENTANALYZER
	//if (IsBanned() || m_bGPLEvildoer)
	if (GetUploadState()==US_BANNED || m_bGPLEvildoer || //Xman Code Improvement 
/*		return 0;

	if */(sysvalue && HasLowID() && !(socket && socket->IsConnected())))
		return 0;

	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
#else
	//if (IsBanned() || m_bGPLEvildoer)
	if (GetUploadState()==US_BANNED || m_bGPLEvildoer || //Xman Code Improvement 
/*		return 0;

	if */(sysvalue && HasLowID() && !(socket && socket->IsConnected())))
		return 0;

    //int filepriority = GetFilePrioAsNumber();
#endif

	// calculate score, based on waitingtime and other factors
	float fBaseValue;
	if (onlybasevalue)
		fBaseValue = 100;
	else if (!isdownloading)
		fBaseValue = (float)(::GetTickCount()-GetWaitStartTime())/1000;
	else{
		// we dont want one client to download forever
		// the first 15 min downloadtime counts as 15 min waitingtime and you get a 15 min bonus while you are in the first 15 min :)
		// (to avoid 20 sec downloads) after this the score won't raise anymore 
		fBaseValue = (float)(m_dwUploadTime-GetWaitStartTime());
		//ASSERT ( m_dwUploadTime-GetWaitStartTime() >= 0 ); //oct 28, 02: changed this from "> 0" to ">= 0" -> // 02-Okt-2006 []: ">=0" is always true!
		fBaseValue += (float)(::GetTickCount() - m_dwUploadTime > 900000)? 900000:1800000;
		fBaseValue /= 1000;
	}

#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
	switch(thePrefs.UseCreditSystem())
	{
		case 0:
			break;
		case 1:		
			fBaseValue *= credits->GetScoreRatio(GetIP());
			break;
		default:
		case 2:
			if(pAntiLeechData)
				fBaseValue *= pAntiLeechData->GetScore();
			else
			{
				ASSERT(0); //should never happen... flow over to default scoring
				fBaseValue *= credits->GetScoreRatio(GetIP());
			}
			break;
	}
//<<< WiZaRd::ClientAnalyzer
#else
	if(thePrefs.UseCreditSystem())
	{
		float modif = credits->GetScoreRatio(GetIP());
		fBaseValue *= modif;
		}
#endif
	if (!onlybasevalue)
	{
#ifdef CLIENTANALYZER
		//WiZaRd - small optimization
		//it's better to put that HERE instead!
		int filepriority = GetFilePrio(currequpfile);
#endif
		fBaseValue *= (float(filepriority)/10.0f);
	}

	if( (IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19 )
		fBaseValue *= 0.5f;
	return (uint32)fBaseValue;
}

// ZZUL-TRA :: PowerShare :: Start
bool CUpDownClient::GetPowerShared() const 
{
	const uint8 iSuperiorStates = GetSuperiorClientStates();// ZZUL-TRA :: PaybackFirst
	CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
    if (currentReqFile == NULL)
		return false;
	if (currentReqFile->GetPowerShared() 
			|| iSuperiorStates & SCS_PBF_SUI // ZZUL-TRA :: PaybackFirst
			)
		    return true;
	    return false;
}
// ZZUL-TRA :: PowerShare :: End

// ZZUL-TRA :: PartImportExport :: Start
// moved to PartFile.h
/*
class CSyncHelper
{
public:
	CSyncHelper()
	{
		m_pObject = NULL;
	}
	~CSyncHelper()
	{
		if (m_pObject)
			m_pObject->Unlock();
	}
	CSyncObject* m_pObject;
};
*/
// ZZUL-TRA :: PartImportExport :: End

void CUpDownClient::CreateNextBlockPackage(bool bBigBuffer){
    // See if we can do an early return. There may be no new blocks to load from disk and add to buffer, or buffer may be large enough allready.
	const uint32 nBufferLimit = bBigBuffer ? (800 * 1024) : (50 * 1024);
	if(m_BlockRequests_queue.IsEmpty() || // There are no new blocks requested
       (m_addedPayloadQueueSession > GetQueueSessionPayloadUp() && GetPayloadInBuffer() > nBufferLimit)) 
	{ // the buffered data is large enough allready
        return;
    }

    CFile file;
	byte* filedata = 0;
	CString fullname;
	bool bFromPF = true; // Statistic to breakdown uploaded data by complete file vs. partfile.
	CSyncHelper lockFile;
	try{
        // Buffer new data if current buffer is less than nBufferLimit Bytes
        while (!m_BlockRequests_queue.IsEmpty() &&
               (m_addedPayloadQueueSession <= GetQueueSessionPayloadUp() || GetPayloadInBuffer() < nBufferLimit)) {

			Requested_Block_Struct* currentblock = m_BlockRequests_queue.GetHead();
			CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(currentblock->FileID);
			if (!srcfile)
				throw GetResString(IDS_ERR_REQ_FNF);

			if (srcfile->IsPartFile() && ((CPartFile*)srcfile)->GetStatus() != PS_COMPLETE){
				// Do not access a part file, if it is currently moved into the incoming directory.
				// Because the moving of part file into the incoming directory may take a noticable 
				// amount of time, we can not wait for 'm_FileCompleteMutex' and block the main thread.
				if (!((CPartFile*)srcfile)->m_FileCompleteMutex.Lock(0)){ // just do a quick test of the mutex's state and return if it's locked.
					return;
				}
				lockFile.m_pObject = &((CPartFile*)srcfile)->m_FileCompleteMutex;
				// If it's a part file which we are uploading the file remains locked until we've read the
				// current block. This way the file completion thread can not (try to) "move" the file into
				// the incoming directory.

				fullname = RemoveFileExtension(((CPartFile*)srcfile)->GetFullName());
			}
			else{
				fullname.Format(_T("%s\\%s"),srcfile->GetPath(),srcfile->GetFileName());
			}
		
			uint64 i64uTogo;
			if (currentblock->StartOffset > currentblock->EndOffset){
				i64uTogo = currentblock->EndOffset + (srcfile->GetFileSize() - currentblock->StartOffset);
			}
			else{
				i64uTogo = currentblock->EndOffset - currentblock->StartOffset;
				if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsComplete(currentblock->StartOffset,currentblock->EndOffset-1, true))
					throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
// ZZUL-TRA :: SOTN :: Start
				if(m_abyUpPartStatus == NULL)
				{
					CString err;
					err.Format(L"Client (%s) asked for blocks without telling us what he needs (should never happen!)", DbgGetClientInfo());
					throw err;
				}
				if(m_abyUpPartStatusHidden)
				{
					for (uint16 i = (uint16)(currentblock->StartOffset/PARTSIZE); i < srcfile->GetPartCount() && i < (uint16)((currentblock->EndOffset-1)/PARTSIZE+1); ++i)
				{
						if(m_abyUpPartStatusHidden[i] != 0)
						{
//							CString err;
//							err.Format(L"Client (%s) requested part %u of %s which is actually hidden!?", DbgGetClientInfo(), i, srcfile->GetFileName());
//							throw err;
							theApp.QueueDebugLogLineEx(LOG_ERROR, L"Client (%s) requested part %u of %s which is actually hidden via %s!?", DbgGetClientInfo(), i, srcfile->GetFileName(), m_abyUpPartStatusHidden[i] == 1 ? L"TCP" : L"UDP");
						}
				}
						}
// ZZUL-TRA :: SOTN :: End
			}

			if( i64uTogo > EMBLOCKSIZE*3 )
				throw GetResString(IDS_ERR_LARGEREQBLOCK);
			uint32 togo = (uint32)i64uTogo;

			if (!srcfile->IsPartFile()){
				bFromPF = false; // This is not a part file...
				if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
					throw GetResString(IDS_ERR_OPEN);
				file.Seek(currentblock->StartOffset,0);
				
				filedata = new byte[togo+500];
				if (uint32 done = file.Read(filedata,togo) != togo){
					file.SeekToBegin();
					file.Read(filedata + done,togo-done);
				}
				file.Close();
			}
			else{
				CPartFile* partfile = (CPartFile*)srcfile;

				partfile->m_hpartfile.Seek(currentblock->StartOffset,0);
				
				filedata = new byte[togo+500];
				if (uint32 done = partfile->m_hpartfile.Read(filedata,togo) != togo){
					partfile->m_hpartfile.SeekToBegin();
					partfile->m_hpartfile.Read(filedata + done,togo-done);
				}
			}
			if (lockFile.m_pObject){
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}

			SetUploadFileID(srcfile);

			// check extension to decide whether to compress or not
			CString ext = srcfile->GetFileName();
			ext.MakeLower();
			int pos = ext.ReverseFind(_T('.'));
			if (pos>-1)
				ext = ext.Mid(pos);
			bool compFlag = (ext!=_T(".zip") && ext!=_T(".cbz") && ext!=_T(".rar") && ext!=_T(".cbr") && ext!=_T(".ace") && ext!=_T(".ogm"));
			if (ext==_T(".avi") && thePrefs.GetDontCompressAvi())
				compFlag=false;

			if (/*!IsUploadingToPeerCache() &&*/ m_byDataCompVer == 1 && compFlag) // X: [RPC] - [Remove PeerCache]
				CreatePackedPackets(filedata,togo,currentblock,bFromPF);
			else
				CreateStandartPackets(filedata,togo,currentblock,bFromPF);
			
			// file statistic
// ZZUL-TRA :: Spreadbars :: Start
			//srcfile->statistic.AddTransferred(togo);
			srcfile->statistic.AddTransferred(currentblock->StartOffset, togo);
// ZZUL-TRA :: Spreadbars :: End

            m_addedPayloadQueueSession += togo;

			m_DoneBlocks_list.AddHead(m_BlockRequests_queue.RemoveHead());
			delete[] filedata;
			filedata = 0;
		}
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), GetUserName(), error);
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Client error: ") + error);
		delete[] filedata;
		return;
	}
	catch(CFileException* e)
	{
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("Failed to create upload package for %s - %s"), GetUserName(), szError);
		theApp.uploadqueue->RemoveFromUploadQueue(this, ((CString)_T("Failed to create upload package.")) + szError);
		delete[] filedata;
		e->Delete();
		return;
	}
}

bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile)
{

	//ZZUL-TRA :: ClientPercentage :: Start
	sint32 hisfinishedparts=-1;
	hiscompletedparts_percent_up=-1;
	//ZZUL-TRA :: ClientPercentage :: End

	delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;
// ZZUL-TRA :: SOTN :: Start
	//No need to clear it here - might be useful later!
	//	delete[] m_abyUpPartStatusHidden;
	//	m_abyUpPartStatusHidden = NULL;
	const UINT nOldUpPartCount = m_nUpPartCount;
// ZZUL-TRA :: SOTN :: End
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;
	if (GetExtendedRequestsVersion() == 0)
	{
// ZZUL-TRA :: SOTN :: Start
		//returning here means that we already lost the partstatus, 
		//so we should delete the hidden status, too (I guess?)
		delete[] m_abyUpPartStatusHidden;
		m_abyUpPartStatusHidden = NULL;
// ZZUL-TRA :: SOTN :: End
		return true;
	}// ZZUL-TRA :: SOTN

	uint16 nED2KUpPartCount = data->ReadUInt16();
	if (!nED2KUpPartCount)
	{
		m_nUpPartCount = tempreqfile->GetPartCount();
		m_abyUpPartStatus = new uint8[m_nUpPartCount];
		memset(m_abyUpPartStatus, 0, m_nUpPartCount);
// ZZUL-TRA :: SOTN :: Start
//>>> taz::fix & optimization
/*
		if(m_abyUpPartStatusHidden == NULL || nOldUpPartCount != m_nUpPartCount)
		{
			delete[] m_abyUpPartStatusHidden;
			m_abyUpPartStatusHidden = NULL;
			m_abyUpPartStatusHidden = new uint8[m_nUpPartCount];
			memset(m_abyUpPartStatusHidden, 0, m_nUpPartCount);
		}
*/
//<<< taz::fix & optimization
// ZZUL-TRA :: SOTN :: End
	}
	else
	{
		if (tempreqfile->GetED2KPartCount() != nED2KUpPartCount)
		{
			//We already checked if we are talking about the same file.. So if we get here, something really strange happened!
			m_nUpPartCount = 0;
// ZZUL-TRA :: SOTN :: Start
			//as stated above - if we get here, something's REALLY wrong!
			delete[] m_abyUpPartStatusHidden;
			m_abyUpPartStatusHidden = NULL;
// ZZUL-TRA :: SOTN :: End
			return false;
		}
		//ZZUL-TRA :: ClientPercentage :: Start
		hisfinishedparts=0;
		//ZZUL-TRA :: ClientPercentage :: End

		m_nUpPartCount = tempreqfile->GetPartCount();
		m_abyUpPartStatus = new uint8[m_nUpPartCount];
		uint16 done = 0;
// ZZUL-TRA :: SOTN :: Start
//>>> taz::fix & optimization
/*
		if(m_abyUpPartStatusHidden == NULL || nOldUpPartCount != m_nUpPartCount)
		{
			delete[] m_abyUpPartStatusHidden;
			m_abyUpPartStatusHidden = NULL;
			m_abyUpPartStatusHidden = new uint8[m_nUpPartCount];
			memset(m_abyUpPartStatusHidden, 0, m_nUpPartCount);
		}
*/
//<<< taz::fix & optimization
// ZZUL-TRA :: SOTN :: End
#ifdef CLIENTANALYZER
		uint16 complcount = 0; //>>> WiZaRd::ClientAnalyzer
#endif
		while (done != m_nUpPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (UINT i = 0; i != 8; i++)
			{
				m_abyUpPartStatus[done] = ((toread >> i) & 1) ? 1 : 0;
//				We may want to use this for another feature..
//				if (m_abyUpPartStatus[done] && !tempreqfile->IsComplete((uint64)done*PARTSIZE,((uint64)(done+1)*PARTSIZE)-1))
//					bPartsNeeded = true;
				
				//ZZUL-TRA :: ClientPercentage :: Start
				if(m_abyUpPartStatus[done])
					hisfinishedparts++;
				//ZZUL-TRA :: ClientPercentage :: End
#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
				if (m_abyUpPartStatus[done])
					++complcount;
//<<< WiZaRd::ClientAnalyzer
#endif
				done++;
				if (done == m_nUpPartCount)
					break;
			}
		}
#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
		if(pAntiLeechData)
		{
			if(complcount == m_nUpPartCount)
				pAntiLeechData->SetBadForThisSession(AT_FILEFAKER);
			else
				pAntiLeechData->ClearBadForThisSession(AT_FILEFAKER);
		}
//<<< WiZaRd::ClientAnalyzer
#endif
	}
// ZZUL-TRA :: SOTN :: Start
//>>> taz::optimization
	if (m_abyUpPartStatusHidden == NULL || nOldUpPartCount != m_nUpPartCount)
	{
		if(m_abyUpPartStatusHidden != NULL)
			delete[] m_abyUpPartStatusHidden;
		m_abyUpPartStatusHidden = new uint8[m_nUpPartCount];
		memset(m_abyUpPartStatusHidden, 0, m_nUpPartCount);
	}
//<<< taz::optimization
// ZZUL-TRA :: SOTN :: End
	if (GetExtendedRequestsVersion() > 1)
	{
		uint16 nCompleteCountLast = GetUpCompleteSourcesCount();
		uint16 nCompleteCountNew = data->ReadUInt16();
		SetUpCompleteSourcesCount(nCompleteCountNew);
		if (nCompleteCountLast != nCompleteCountNew)
			tempreqfile->UpdatePartsInfo();
	}
	theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(this);

//ZZUL-TRA :: ClientPercentage :: Start
	if(hisfinishedparts>=0)
		hiscompletedparts_percent_up= (sint8)((float)hisfinishedparts/tempreqfile->GetPartCount()*100.0f);	
	else
		hiscompletedparts_percent_up= -1;
	//ZZUL-TRA :: ClientPercentage :: End

	return true;
}

void CUpDownClient::CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	uint32 nPacketSize;
	CMemFile memfile((BYTE*)data,togo);
	if (togo > 10240) 
		nPacketSize = togo/(uint32)(togo/10240);
	else
		nPacketSize = togo;
	while (togo){
		if (togo < nPacketSize*2)
			nPacketSize = togo;
		ASSERT( nPacketSize );
		togo -= nPacketSize;

		uint64 statpos = (currentblock->EndOffset - togo) - nPacketSize;
		uint64 endpos = (currentblock->EndOffset - togo);
		// X: [RPC] - [Remove PeerCache]
		/*if (IsUploadingToPeerCache())
		{
			if (m_pPCUpSocket == NULL){
				ASSERT(0);
				CString strError;
				strError.Format(_T("Failed to upload to PeerCache - missing socket; %s"), DbgGetClientInfo());
				throw strError;
			}
			CSafeMemFile dataHttp(10240);
			if (m_iHttpSendState == 0)
			{
				CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
				CStringA str;
				str.AppendFormat("HTTP/1.0 206\r\n");
				str.AppendFormat("Content-Range: bytes %I64u-%I64u/%I64u\r\n", currentblock->StartOffset, currentblock->EndOffset - 1, srcfile->GetFileSize());
				str.AppendFormat("Content-Type: application/octet-stream\r\n");
				str.AppendFormat("Content-Length: %u\r\n", (uint32)(currentblock->EndOffset - currentblock->StartOffset));
				str.AppendFormat("Server: eMule/%s\r\n", CStringA(theApp.m_strCurVersionLong));
				str.AppendFormat("\r\n");
				dataHttp.Write((LPCSTR)str, str.GetLength());
				theStats.AddUpDataOverheadFileRequest((UINT)dataHttp.GetLength());

				m_iHttpSendState = 1;
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
				if (thePrefs.GetDebugClientTCPLevel() > 0){
					DebugSend("PeerCache-HTTP", this, GetUploadFileID());
					Debug(_T("  %hs\n"), str);
				}
#endif
			}
			dataHttp.Write(data, nPacketSize);
			data += nPacketSize;

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (thePrefs.GetDebugClientTCPLevel() > 1){
				DebugSend("PeerCache-HTTP data", this, GetUploadFileID());
				Debug(_T("  Start=%I64u  End=%I64u  Size=%u\n"), statpos, endpos, nPacketSize);
			}
#endif

			UINT uRawPacketSize = (UINT)dataHttp.GetLength();
			LPBYTE pRawPacketData = dataHttp.Detach();
			CRawPacket* packet = new CRawPacket((char*)pRawPacketData, uRawPacketSize, bFromPF);
			m_pPCUpSocket->SendPacket(packet, true, false, nPacketSize);
			free(pRawPacketData);
		}
		else*/
		{
			Packet* packet;
			if (statpos > 0xFFFFFFFF || endpos > 0xFFFFFFFF){
				packet = new Packet(OP_SENDINGPART_I64,nPacketSize+32, OP_EMULEPROT, bFromPF);
				md4cpy(&packet->pBuffer[0],GetUploadFileID());
				PokeUInt64(&packet->pBuffer[16], statpos);
				PokeUInt64(&packet->pBuffer[24], endpos);
				memfile.Read(&packet->pBuffer[32],nPacketSize);
				theStats.AddUpDataOverheadFileRequest(32);
			}
			else{
				packet = new Packet(OP_SENDINGPART,nPacketSize+24, OP_EDONKEYPROT, bFromPF);
				md4cpy(&packet->pBuffer[0],GetUploadFileID());
				PokeUInt32(&packet->pBuffer[16], (uint32)statpos);
				PokeUInt32(&packet->pBuffer[20], (uint32)endpos);
				memfile.Read(&packet->pBuffer[24],nPacketSize);
				theStats.AddUpDataOverheadFileRequest(24);
			}

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
			if (thePrefs.GetDebugClientTCPLevel() > 0){
				DebugSend("OP__SendingPart", this, GetUploadFileID());
				Debug(_T("  Start=%I64u  End=%I64u  Size=%u\n"), statpos, endpos, nPacketSize);
			}
#endif
			// put packet directly on socket
			
			socket->SendPacket(packet,true,false, nPacketSize);
		}
	}
}

void CUpDownClient::CreatePackedPackets(byte* data, uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	BYTE* output = new BYTE[togo+300];
	uLongf newsize = togo+300;

	// ZZUL-TRA :: VariableCompression :: Start
	/*
	UINT result = compress2(output, &newsize, data, togo, 9);
	*/
	int	compressLevel = 9;
	if (thePrefs.GetMaxUpload() > 500.0f)
		compressLevel = 1;
	else if (thePrefs.GetMaxUpload() > 200.0f)
		compressLevel = 3;
	else if (thePrefs.GetMaxUpload() > 80.0f)
		compressLevel = 5;
	else if (thePrefs.GetMaxUpload() > 50.0f)
		compressLevel = 7;
	UINT result = compress2(output, &newsize, data, togo, compressLevel);
	// ZZUL-TRA :: VariableCompression :: End

	if (result != Z_OK || togo <= newsize){
		delete[] output;
		CreateStandartPackets(data,togo,currentblock,bFromPF);
		return;
	}

	// ZZUL-TRA :: ShowCompression :: Start
	compressiongain += (togo-newsize);
	notcompressed += togo;
	// ZZUL-TRA :: ShowCompression :: End

	CMemFile memfile(output,newsize);
    uint32 oldSize = togo;
	togo = newsize;
	uint32 nPacketSize;
    if (togo > 10240) 
        nPacketSize = togo/(uint32)(togo/10240);
    else
        nPacketSize = togo;
    
    uint32 totalPayloadSize = 0;

    while (togo){
		if (togo < nPacketSize*2)
			nPacketSize = togo;
		ASSERT( nPacketSize );
		togo -= nPacketSize;
		uint64 statpos = currentblock->StartOffset;
		Packet* packet;
		if (currentblock->StartOffset > 0xFFFFFFFF || currentblock->EndOffset > 0xFFFFFFFF){
			packet = new Packet(OP_COMPRESSEDPART_I64,nPacketSize+28,OP_EMULEPROT,bFromPF);
			md4cpy(&packet->pBuffer[0],GetUploadFileID());
			PokeUInt64(&packet->pBuffer[16], statpos);
			PokeUInt32(&packet->pBuffer[24], newsize);
			memfile.Read(&packet->pBuffer[28],nPacketSize);
		}
		else{
			packet = new Packet(OP_COMPRESSEDPART,nPacketSize+24,OP_EMULEPROT,bFromPF);
			md4cpy(&packet->pBuffer[0],GetUploadFileID());
			PokeUInt32(&packet->pBuffer[16], (uint32)statpos);
			PokeUInt32(&packet->pBuffer[20], newsize);
			memfile.Read(&packet->pBuffer[24],nPacketSize);
		}

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0){
			DebugSend("OP__CompressedPart", this, GetUploadFileID());
			Debug(_T("  Start=%I64u  BlockSize=%u  Size=%u\n"), statpos, newsize, nPacketSize);
		}
#endif
        // approximate payload size
        uint32 payloadSize = nPacketSize*oldSize/newsize;

        if(togo == 0 && totalPayloadSize+payloadSize < oldSize) {
            payloadSize = oldSize-totalPayloadSize;
        }
        totalPayloadSize += payloadSize;

        // put packet directly on socket
		theStats.AddUpDataOverheadFileRequest(24);
        socket->SendPacket(packet,true,false, payloadSize);
	}
	delete[] output;
}

void CUpDownClient::SetUploadFileID(CKnownFile* newreqfile)
{
	CKnownFile* oldreqfile;
	//We use the knownfilelist because we may have unshared the file..
	//But we always check the download list first because that person may have decided to redownload that file.
	//Which will replace the object in the knownfilelist if completed.
	if ((oldreqfile = theApp.downloadqueue->GetFileByID(requpfileid)) == NULL)
		oldreqfile = theApp.knownfiles->FindKnownFileByID(requpfileid);
	else
	{
		// In some _very_ rare cases it is possible that we have different files with the same hash in the downloadlist
		// as well as in the sharedlist (redownloading a unshared file, then resharing it before the first part has been downloaded)
		// to make sure that in no case a deleted client object is left on the list, we need to doublecheck
		// TODO: Fix the whole issue properly
		CKnownFile* pCheck = theApp.sharedfiles->GetFileByID(requpfileid);
		if (pCheck != NULL && pCheck != oldreqfile)
		{
			ASSERT( false );
			pCheck->RemoveUploadingClient(this);
		}
	}

	if (newreqfile == oldreqfile)
		return;

	// clear old status
	delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;
// ZZUL-TRA :: SOTN :: Start
	delete[] m_abyUpPartStatusHidden;
	m_abyUpPartStatusHidden = NULL;
// ZZUL-TRA :: SOTN :: End
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;

	if (newreqfile)
	{
		newreqfile->AddUploadingClient(this);
		md4cpy(requpfileid, newreqfile->GetFileHash());
	}
	else
		md4clr(requpfileid);

	if (oldreqfile)
		oldreqfile->RemoveUploadingClient(this);
}

void CUpDownClient::AddReqBlock(Requested_Block_Struct* reqblock)
{
    if(GetUploadState() != US_UPLOADING) {
        if(thePrefs.GetLogUlDlEvents())
            AddDebugLogLine(DLP_LOW, false, _T("UploadClient: Client tried to add req block when not in upload slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
		delete reqblock;
        return;
    }

	if(HasCollectionUploadSlot()){
		CKnownFile* pDownloadingFile = theApp.sharedfiles->GetFileByID(reqblock->FileID);
		if(pDownloadingFile != NULL){
			if ( !(CCollection::HasCollectionExtention(pDownloadingFile->GetFileName()) && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) ){
				AddDebugLogLine(DLP_HIGH, false, _T("UploadClient: Client tried to add req block for non collection while having a collection slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
				delete reqblock;
				return;
			}
		}
		else
			ASSERT( false );
	}

    for (POSITION pos = m_DoneBlocks_list.GetHeadPosition(); pos != 0; ){
        const Requested_Block_Struct* cur_reqblock = m_DoneBlocks_list.GetNext(pos);
        if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset){
            delete reqblock;
            return;
        }
    }
    for (POSITION pos = m_BlockRequests_queue.GetHeadPosition(); pos != 0; ){
        const Requested_Block_Struct* cur_reqblock = m_BlockRequests_queue.GetNext(pos);
        if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset){
            delete reqblock;
            return;
        }
    }

    m_BlockRequests_queue.AddTail(reqblock);
}

uint32 CUpDownClient::SendBlockData(){
 
   DWORD curTick = ::GetTickCount();
//ZZUL +
    bool sumavgUDRChanged = false;
//ZZUL -
    uint64 sentBytesCompleteFile = 0;
    uint64 sentBytesPartFile = 0;
    uint64 sentBytesPayload = 0;

    if (GetFileUploadSocket() /*&& (m_ePeerCacheUpState != PCUS_WAIT_CACHE_REPLY)*/) // X: [RPC] - [Remove PeerCache]
	{
		CEMSocket* s = GetFileUploadSocket();
		UINT uUpStatsPort;
        /*if (m_pPCUpSocket && IsUploadingToPeerCache())
		{
			uUpStatsPort = (UINT)-1;

            // Check if filedata has been sent via the normal socket since last call.
            uint64 sentBytesCompleteFileNormalSocket = socket->GetSentBytesCompleteFileSinceLastCallAndReset();
            uint64 sentBytesPartFileNormalSocket = socket->GetSentBytesPartFileSinceLastCallAndReset();

			if(thePrefs.GetVerbose() && (sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket > 0)) {
                AddDebugLogLine(false, _T("Sent file data via normal socket when in PC mode. Bytes: %I64i."), sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket);
			}
        }
		else*/
			uUpStatsPort = GetUserPort();

	    // Extended statistics information based on which client software and which port we sent this data to...
	    // This also updates the grand total for sent bytes, etc.  And where this data came from.
        sentBytesCompleteFile = s->GetSentBytesCompleteFileSinceLastCallAndReset();
        sentBytesPartFile = s->GetSentBytesPartFileSinceLastCallAndReset();
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, (UINT)sentBytesCompleteFile, (IsFriend() && GetFriendSlot()));
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, (UINT)sentBytesPartFile, (IsFriend() && GetFriendSlot()));

		m_nTransferredUp = (UINT)(m_nTransferredUp + sentBytesCompleteFile + sentBytesPartFile);
        credits->AddUploaded((UINT)(sentBytesCompleteFile + sentBytesPartFile), GetIP());

#ifdef CLIENTANALYZER
		if (pAntiLeechData)
		{
			UINT srccount = _UI32_MAX; //no file? then default to non-rare...
			if (GetUploadFileID())
			{
				CKnownFile* file = theApp.sharedfiles->GetFileByID(GetUploadFileID());
			if (file)
			{
				srccount = file->m_nCompleteSourcesCount;
//					srccount += file->GetRealQueuedCount(); //if you implemented that...
				}
			}
			pAntiLeechData->AddUploaded(sentBytesCompleteFile, false, srccount);
			pAntiLeechData->AddUploaded(sentBytesPartFile, true, srccount);
		}
#endif

        sentBytesPayload = s->GetSentPayloadSinceLastCallAndReset();
        m_nCurQueueSessionPayloadUp = (UINT)(m_nCurQueueSessionPayloadUp + sentBytesPayload);

// ZZUL-TRA :: PowersahreStat :: Start
		CKnownFile* file = theApp.sharedfiles->GetFileByID(GetUploadFileID());
		if(file && file->GetPowerShared())
		{
			thePrefs.AddupUploadedPowersharePartfile(sentBytesPartFile);
			thePrefs.AddupUploadedPowershareComplete(sentBytesCompleteFile);
		}
// ZZUL-TRA :: PowersahreStat :: End

//ZZUL +
	if(GetUploadState() == US_UPLOADING) {
            bool wasRemoved = false;

            if(!IsScheduledForRemoval() && /*wasRemoved == false &&*/ GetQueueSessionPayloadUp() > GetCurrentSessionLimit()) {
                // Should we end this upload?

                // first clear the average speed, to show ?? as speed in upload slot display
                m_AvarageUDR_list.RemoveAll();
                m_nSumForAvgUpDataRate = 0;
                sumavgUDRChanged = true;

                // Give clients in queue a chance to kick this client out.
                // It will be kicked out only if queue contains a client
                // of same/higher class as this client, and that new
                // client must either be a high ID client, or a low ID
                // client that is currently connected.
                wasRemoved = theApp.uploadqueue->RemoveOrMoveDown(this);

                if(!wasRemoved) {
                    // It wasn't removed, so it is allowed to pass into the next amount.
                    m_curSessionAmountNumber++;
                }
            }

            if(wasRemoved) {
			    //if (thePrefs.GetDebugClientTCPLevel() > 0)
				   // DebugSend("OP__OutOfPartReqs", this);
			    //Packet* pCancelTransferPacket = new Packet(OP_OUTOFPARTREQS, 0);
			    //theStats.AddUpDataOverheadFileRequest(pCancelTransferPacket->size);
			    //socket->SendPacket(pCancelTransferPacket,true,true);
			} else {
 //ZZUL -
           	// read blocks from file and put on socket
            CreateNextBlockPackage(); 
			}
        }
//ZZUL +
    }
//ZZUL -
    if(sentBytesCompleteFile + sentBytesPartFile > 0 ||
        m_AvarageUDR_list.GetCount() == 0 || (curTick - m_AvarageUDR_list.GetTail().timestamp) > 1*1000) {
        // Store how much data we've transferred this round,
        // to be able to calculate average speed later
        // keep sum of all values in list up to date
        TransferredData newitem = {(UINT)(sentBytesCompleteFile + sentBytesPartFile), curTick};
        m_AvarageUDR_list.AddTail(newitem);
        m_nSumForAvgUpDataRate = (UINT)(m_nSumForAvgUpDataRate + sentBytesCompleteFile + sentBytesPartFile);
//ZZUL + 
       sumavgUDRChanged = true;
//ZZUL -
    }

    // remove to old values in list
    while (m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 10*1000) {
        // keep sum of all values in list up to date
        m_nSumForAvgUpDataRate -= m_AvarageUDR_list.RemoveHead().datalen;
//ZZUL +
        sumavgUDRChanged = true;
//ZZUL -
    }

//ZZUL +
    bool avgChanged = false;
    // Calculate average speed for this slot
    if(m_AvarageUDR_list.GetCount() > 1) {
        if(sumavgUDRChanged) {
            DWORD dwDuration = curTick - m_AvarageUDR_list.GetHead().timestamp;

            if(dwDuration > 0 && GetUpStartTimeDelay() > 2*1000) {
                m_nUpDatarate = (m_nSumForAvgUpDataRate*1000) / (curTick-m_AvarageUDR_list.GetHead().timestamp);
                avgChanged = true;
            } else  if(m_nUpDatarate != -1) {
                // not enough values to calculate trustworthy speed. Use -1 to tell this
                m_nUpDatarate = -1;
                avgChanged = true;
            }
        }
    } else if(m_nUpDatarate != -1) {
        // not enough values to calculate trustworthy speed. Use -1 to tell this
        m_nUpDatarate = -1;
        avgChanged = true;
    }

    // Check if it's time to update the display.
  if (avgChanged && curTick-m_lastRefreshedULDisplay > MINWAIT_BEFORE_ULDISPLAY_WINDOWUPDATE+(uint32)(rand()*800/RAND_MAX)) {
//ZZUL -
      // Update display
        theApp.emuledlg->transferwnd->GetUploadList()->RefreshClient(this);
        theApp.emuledlg->transferwnd->GetClientList()->RefreshClient(this);
        m_lastRefreshedULDisplay = curTick;
    }

    return (UINT)(sentBytesCompleteFile + sentBytesPartFile);
}

void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue()
{
	//OP_OUTOFPARTREQS will tell the downloading client to go back to OnQueue..
	//The main reason for this is that if we put the client back on queue and it goes
	//back to the upload before the socket times out... We get a situation where the
	//downloader thinks it already sent the requested blocks and the uploader thinks
	//the downloader didn't send any request blocks. Then the connection times out..
	//I did some tests with eDonkey also and it seems to work well with them also..
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__OutOfPartReqs", this);
#endif
	Packet* pPacket = new Packet(OP_OUTOFPARTREQS, 0);
	theStats.AddUpDataOverheadFileRequest(pPacket->size);
	SendPacket(pPacket, true);
	m_fSentOutOfPartReqs = 1;
    theApp.uploadqueue->AddClientToQueue(this, true);
}

/**
 * See description for CEMSocket::TruncateQueues().
 */
void CUpDownClient::FlushSendBlocks(){ // call this when you stop upload, or the socket might be not able to send
    if (socket)      //socket may be NULL...
        socket->TruncateQueues();
}

void CUpDownClient::SendHashsetPacket(const uchar* pData, uint32 nSize, bool bFileIdentifiers)
{
	Packet* packet;
	CSafeMemFile fileResponse(1024);
	if (bFileIdentifiers)
	{
		CSafeMemFile data(pData, nSize);
		CFileIdentifierSA fileIdent;
		if (!fileIdent.ReadIdentifier(&data))
			throw _T("Bad FileIdentifier (OP_HASHSETREQUEST2)");
		CKnownFile* file = theApp.sharedfiles->GetFileByIdentifier(fileIdent, false);
		if (file == NULL)
		{
			CheckFailedFileIdReqs(fileIdent.GetMD4Hash());
			throw GetResString(IDS_ERR_REQ_FNF) + _T(" (SendHashsetPacket2)");
		}
		uint8 byOptions = data.ReadUInt8();
		bool bMD4 = (byOptions & 0x01) > 0;
		bool bAICH = (byOptions & 0x02) > 0;
		if (!bMD4 && !bAICH)
		{
			DebugLogWarning(_T("Client sent HashSet request with none or unknown HashSet type requested (%u) - file: %s, client %s")
				, byOptions, file->GetFileName(), DbgGetClientInfo());
			return;
		}
		file->GetFileIdentifier().WriteIdentifier(&fileResponse);
		// even if we don't happen to have an AICH hashset yet for some reason we send a proper (possible empty) response
		file->GetFileIdentifier().WriteHashSetsToPacket(&fileResponse, bMD4, bAICH); 
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__HashSetAnswer", this, file->GetFileIdentifier().GetMD4Hash());
#endif
		packet = new Packet(&fileResponse, OP_EMULEPROT, OP_HASHSETANSWER2);
	}
	else
	{
		if (nSize != 16)
		{
			ASSERT( false );
			return;
		}
		CKnownFile* file = theApp.sharedfiles->GetFileByID(pData);
		if (!file){
			CheckFailedFileIdReqs(pData);
			throw GetResString(IDS_ERR_REQ_FNF) + _T(" (SendHashsetPacket)");
		}
		file->GetFileIdentifier().WriteMD4HashsetToFile(&fileResponse);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__HashSetAnswer", this, pData);
#endif
		packet = new Packet(&fileResponse, OP_EDONKEYPROT, OP_HASHSETANSWER);
	}
	theStats.AddUpDataOverheadFileRequest(packet->size);
	SendPacket(packet, true);
}

void CUpDownClient::ClearUploadBlockRequests()
{
	FlushSendBlocks();

	for (POSITION pos = m_BlockRequests_queue.GetHeadPosition();pos != 0;)
		delete m_BlockRequests_queue.GetNext(pos);
	m_BlockRequests_queue.RemoveAll();
	
	for (POSITION pos = m_DoneBlocks_list.GetHeadPosition();pos != 0;)
		delete m_DoneBlocks_list.GetNext(pos);
	m_DoneBlocks_list.RemoveAll();
}

void CUpDownClient::SendRankingInfo(){
	if (!ExtProtocolAvailable())
		return;
	UINT nRank = theApp.uploadqueue->GetWaitingPosition(this);
	if (!nRank)
		return;
	Packet* packet = new Packet(OP_QUEUERANKING,12,OP_EMULEPROT);
	PokeUInt16(packet->pBuffer+0, (uint16)nRank);
	memset(packet->pBuffer+2, 0, 10);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__QueueRank", this);
#endif
	theStats.AddUpDataOverheadFileRequest(packet->size);
	SendPacket(packet, true);
}

void CUpDownClient::SendCommentInfo(/*const*/ CKnownFile *file)
{
	if (!m_bCommentDirty || file == NULL || !ExtProtocolAvailable() || m_byAcceptCommentVer < 1)
		return;
	m_bCommentDirty = false;

	UINT rating = file->GetFileRating();
	const CString& desc = file->GetFileComment();
	if (file->GetFileRating() == 0 && desc.IsEmpty())
		return;

	CSafeMemFile data(256);
	data.WriteUInt8((uint8)rating);
	data.WriteLongString(desc, GetUnicodeSupport());
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__FileDesc", this, file->GetFileHash());
#endif
	Packet *packet = new Packet(&data,OP_EMULEPROT);
	packet->opcode = OP_FILEDESC;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	SendPacket(packet, true);
}

void CUpDownClient::AddRequestCount(const uchar* fileid)
{
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition(); pos != 0; ){
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		if (!md4cmp(cur_struct->fileid,fileid)){
#ifdef CLIENTANALYZER
//>>> WiZaRd::ClientAnalyzer
			if(pAntiLeechData)
				pAntiLeechData->AddReask(::GetTickCount()-cur_struct->lastasked);
//<<< WiZaRd::ClientAnalyzer
#endif
			if (::GetTickCount() - cur_struct->lastasked < MIN_REQUESTTIME && !GetFriendSlot()){ 
				if (GetDownloadState() != DS_DOWNLOADING)
					cur_struct->badrequests++;
				if (cur_struct->badrequests == BADCLIENTBAN){
					Ban();
				}
			}
			else{
				if (cur_struct->badrequests)
					cur_struct->badrequests--;
			}
			cur_struct->lastasked = ::GetTickCount();
			return;
		}
	}
	Requested_File_Struct* new_struct = new Requested_File_Struct;
	md4cpy(new_struct->fileid,fileid);
	new_struct->lastasked = ::GetTickCount();
	new_struct->badrequests = 0;
	m_RequestedFiles_list.AddHead(new_struct);
}

void  CUpDownClient::UnBan()
{
	theApp.clientlist->AddTrackClient(this);
	theApp.clientlist->RemoveBannedClient(GetIP());
	SetUploadState(US_NONE);
	ClearWaitStartTime();
	//theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	theApp.emuledlg->transferwnd->ShowQueueCount(); // X: [SFH] - [Show IP Filter Hits]

#ifdef CLIENTANALYZER
	//we can safely delete this data... we reset it anyhow!
	//also, on a sidenote, resetting the timestamp to 0 would cause problems with the CA (see "AddReask")
	while(!m_RequestedFiles_list.IsEmpty())
		delete m_RequestedFiles_list.RemoveHead();
#else
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition();pos != 0;)
	{
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		cur_struct->badrequests = 0;
		cur_struct->lastasked = 0;	
	}
#endif
}

void CUpDownClient::Ban(LPCTSTR pszReason)
{
        if(IsFriend())
	   return;

	SetChatState(MS_NONE);
	theApp.clientlist->AddTrackClient(this);
	if (!IsBanned()){
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false,_T("Banned: %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
	}
#ifdef _DEBUG
	else{
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false,_T("Banned: (refreshed): %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
	}
#endif
	theApp.clientlist->AddBannedClient(GetIP());
	SetUploadState(US_BANNED);
	//theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	theApp.emuledlg->transferwnd->ShowQueueCount(); // X: [SFH] - [Show IP Filter Hits]
	theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(this);
	if (socket != NULL && socket->IsConnected())
		socket->ShutDown(SD_RECEIVE); // let the socket timeout, since we dont want to risk to delete the client right now. This isnt acutally perfect, could be changed later
}

uint32 CUpDownClient::GetWaitStartTime() const
{
	if (credits == NULL){
		ASSERT ( false );
		return 0;
	}
	uint32 dwResult = credits->GetSecureWaitStartTime(GetIP());
	if (dwResult > m_dwUploadTime && IsDownloading()){
		//this happens only if two clients with invalid securehash are in the queue - if at all
		dwResult = m_dwUploadTime-1;

		if (thePrefs.GetVerbose())
			DEBUG_ONLY(AddDebugLogLine(false,_T("Warning: CUpDownClient::GetWaitStartTime() waittime Collision (%s)"),GetUserName()));
	}
	return dwResult;
}

void CUpDownClient::SetWaitStartTime(){
	if (credits == NULL){
		return;
	}
	credits->SetSecWaitStartTime(GetIP());
}

void CUpDownClient::ClearWaitStartTime(){
	if (credits == NULL){
		return;
	}
	credits->ClearWaitStartTime();
}

bool CUpDownClient::GetFriendSlot() const
{
	if (credits && theApp.clientcredits->CryptoAvailable()){
		switch(credits->GetCurrentIdentState(GetIP())){
			case IS_IDFAILED:
			case IS_IDNEEDED:
			case IS_IDBADGUY:
				return false;
		}
	}
	return m_bFriendSlot;
}

CEMSocket* CUpDownClient::GetFileUploadSocket(/*bool bLog*/)// X: [RPC] - [Remove PeerCache]
{
    /*if (m_pPCUpSocket && IsUploadingToPeerCache()) //Xman Xtreme Upload: Peercache-part //(IsUploadingToPeerCache() || m_ePeerCacheUpState == PCUS_WAIT_CACHE_REPLY))
	{
        if (bLog && thePrefs.GetVerbose())
            AddDebugLogLine(false, _T("%s got peercache socket."), DbgGetClientInfo());
        return m_pPCUpSocket;
    }
	else*/
	{
        //if (bLog && thePrefs.GetVerbose())
            //AddDebugLogLine(false, _T("%s got normal socket."), DbgGetClientInfo());
        return socket;
    }
}

void CUpDownClient::SetCollectionUploadSlot(bool bValue){
	ASSERT( !IsDownloading() || bValue == m_bCollectionUploadSlot );
	m_bCollectionUploadSlot = bValue;
}

// ZZUL-TRA :: SOTN :: Start
void CUpDownClient::GetUploadingAndUploadedPart(CArray<uint16>& arr, CArray<uint16>& arrHidden)
{
	//count any part that has NOT been hidden so we have an array that counts the visibility of the chunks
	//with that information, we can select the least visible chunk later on
	if(m_abyUpPartStatusHidden)
	{
		for (UINT i = 0; i < m_nUpPartCount; ++i)
			if(m_abyUpPartStatusHidden[i] == 0)
				++arrHidden[i];
	}

	if(!IsDownloading())
		return;

	const Requested_Block_Struct* block = NULL;
	//uploading...
	if (!m_BlockRequests_queue.IsEmpty())
	{
		block = m_BlockRequests_queue.GetHead();
		if(block)
		{
			const UINT part = (UINT)block->StartOffset/PARTSIZE;
			arr[part] = max(1, arr[part]);
		}
	}
	//... and uploaded :)
	for(POSITION pos = m_DoneBlocks_list.GetHeadPosition(); pos;)
	{
		block = m_DoneBlocks_list.GetNext(pos);
		if(block)
		{
			const UINT part = (UINT)block->StartOffset/PARTSIZE;
			arr[part] = max(1, arr[part]);
		}
	}
}
// ZZUL-TRA :: SOTN :: End

// ZZUL-TRA :: SeeOnQueue :: Start
void CUpDownClient::SetOldUploadFileID()
{
	if(requpfileid)
		md4cpy(oldfileid, requpfileid);
}
// ZZUL-TRA :: SeeOnQueue :: End

// ZZUL-TRA :: SuperiorClientHandling :: Start
uint8 CUpDownClient::GetSuperiorClientStates() const
{
    CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID((uchar*)GetUploadFileID());
	uint8 iSuperiorStates = 0;

        // only clients requesting a valid file can be superior
	if(currentReqFile == NULL) 
		return SCS_NONE;

    // nothing else is allowed if the requested file is incomplete
	if(currentReqFile->IsPartFile())
		return iSuperiorStates;

	// ZZUL-TRA :: PaybackFirst :: Start
	if(Credits()){
		const bool bIsSecure = theApp.clientcredits->CryptoAvailable() && Credits()->GetCurrentIdentState(GetIP()) == IS_IDENTIFIED;
		if(credits->GetPayBackFirstStatus() && thePrefs.IsPayBackFirst() && bIsSecure)
			iSuperiorStates |= SCS_PBF_SUI; 
	}
	// ZZUL-TRA :: PaybackFirst :: End

	return iSuperiorStates;
}
// ZZUL-TRA :: SuperiorClientHandling :: End