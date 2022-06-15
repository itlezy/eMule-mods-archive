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
#include "PeerCacheSocket.h"
#include "Sockets.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "TransferWnd.h"
#include "Log.h"
#include "Collection.h"
#include "Neo/ClientFileStatus.h" // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "Neo/NeoOpcodes.h" // NEO: XC - [ExtendedComments] <-- Xanatos --
#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
#include "Neo/Sources/SourceList.h"
#endif // NEO_CD // NEO: SFL END <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
#include "Neo/Functions.h" // NEO: MOD <-- Xanatos --
#include <math.h> // NEO: NFS - [NeoScoreSystem] <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#else
#include "LastCommonRouteFinder.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//	members of CUpDownClient
//	which are mainly used for uploading functions 

CBarShader CUpDownClient::s_UpStatusBar(16);

//void CUpDownClient::DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const
void CUpDownClient::DrawUpStatusBar(CDC* dc, RECT* rect, const uchar* fileid, bool  bFlat) const // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
{
    COLORREF crNeither;
	COLORREF crNextSending;
	COLORREF crClientPartial; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
	COLORREF crSeen; // NEO: PSH - [PartStatusHistory] <-- Xanatos --
	COLORREF crBoth;
	COLORREF crSending;
	COLORREF crBuffer; // NEO: MOD - [ShowBuffered] <-- Xanatos --

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	if (!HaveTrickleSlot() ||
#else
    if(GetSlotNumber() <= theApp.uploadqueue->GetActiveUploadsCount() ||
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --
       (GetUploadState() != US_UPLOADING && GetUploadState() != US_CONNECTING) ) {
        crNeither = RGB(224, 224, 224);
	    crNextSending = RGB(255,208,0);
	    crBoth = bFlat ? RGB(0, 0, 0) : RGB(104, 104, 104);
	    crSending = RGB(0, 150, 0);
		crClientPartial = RGB(170,50,224); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
		crSeen = RGB(255,240,240); // NEO: PSH - [PartStatusHistory] <-- Xanatos --
		crBuffer = RGB(255, 100, 100); // NEO: MOD - [ShowBuffered] <-- Xanatos --
    } else {
        // grayed out
        crNeither = RGB(248, 248, 248);
	    crNextSending = RGB(255,244,191);
	    crBoth = bFlat ? RGB(191, 191, 191) : RGB(191, 191, 191);
	    crSending = RGB(191, 229, 191);
		crClientPartial = RGB(170,50,224); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
		crSeen = RGB(255,240,240); // NEO: PSH - [PartStatusHistory] <-- Xanatos --
		crBuffer = RGB(255, 100, 100); // NEO: MOD - [ShowBuffered] <-- Xanatos --
    }

	// wistily: UpStatusFix
	//CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	// NEO: MFSB - [MultiFileStatusBars] -- Xanatos -->
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(fileid);
	if(!currequpfile)
		currequpfile = theApp.knownfiles->FindKnownFileByID(fileid);
	if(!currequpfile)
		return;
	// NEO: MFSB END <-- Xanatos --
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* status = GetFileStatus(currequpfile);
	uint8* m_abyUpPartStatus = status ? status->GetPartStatus() : NULL;
	uint8* m_abyUpIncPartStatus = status ? status->GetPartStatus(CFS_Incomplete) : NULL; // NEO: ICS - [InteligentChunkSelection]
	uint8* m_abyUpSeenPartStatus = status ? status->GetPartStatus(CFS_History) : NULL; // NEO: PSH - [PartStatusHistory] <-- Xanatos --
	uint16 m_nUpPartCount = status ? (uint16)status->GetPartCount() : 0;
	EMFileSize filesize = status ? status->GetFileSize() : 0x0ui64;
	// NEO: SCFS END <-- Xanatos --

	//EMFileSize filesize;
	//if (currequpfile)
	//	filesize = currequpfile->GetFileSize();
	//else
	//	filesize = (uint64)(PARTSIZE * (uint64)m_nUpPartCount);
	// wistily: UpStatusFix

	uint16 CompleteParts = 0; // NEO: MOD - [Percentage] <-- Xanatos --

    if(filesize > (uint64)0) {
	    s_UpStatusBar.SetFileSize(filesize); 
	    s_UpStatusBar.SetHeight(rect->bottom - rect->top); 
	    s_UpStatusBar.SetWidth(rect->right - rect->left); 
	    s_UpStatusBar.Fill(crNeither); 
	    //if (!onlygreyrect && m_abyUpPartStatus) { 
		if (m_abyUpPartStatus) {  // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
		    for (UINT i = 0; i < m_nUpPartCount; i++)
				if (m_abyUpPartStatus[i]){
				    s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crBoth);
					CompleteParts ++; // NEO: MOD - [Percentage] <-- Xanatos --
				}
	    }
		// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
		if (m_abyUpIncPartStatus) {
		    for (UINT i = 0; i < m_nUpPartCount; i++)
			    if (m_abyUpIncPartStatus[i])
				    s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crClientPartial);
	    }
		// NEO: ICS END <-- Xanatos --
		// NEO: PSH - [PartStatusHistory] -- Xanatos -->
		if(m_abyUpSeenPartStatus) {
		    for (UINT i = 0; i < m_nUpPartCount; i++)
			    if (m_abyUpSeenPartStatus[i])
				    s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crSeen);
		}
		// NEO: PSH END <-- Xanatos --

		if(fileid == GetUploadFileID()) // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
		{
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

			// NEO: MOD - [ShowBuffered] -- Xanatos -->
            // Also show what data is buffered (with color crBuffer)
            uint64 total = 0;
    
		    for(POSITION pos=m_DoneBlocks_list.GetTailPosition();pos!=0; ){
			    Requested_Block_Struct* block = m_DoneBlocks_list.GetPrev(pos);
    
                if(total + (block->EndOffset-block->StartOffset) <= GetQueueSessionPayloadUp()) {
                    // block is sent
			        s_UpStatusBar.FillRange(block->StartOffset, block->EndOffset, crSending);
                    total += block->EndOffset-block->StartOffset;
                }
                else if (total < GetQueueSessionPayloadUp()){
                    // block partly sent, partly in buffer
                    total += block->EndOffset-block->StartOffset;
                    uint64 rest = total - GetQueueSessionPayloadUp();
                    uint64 newEnd = block->EndOffset-rest;
    
    			    s_UpStatusBar.FillRange(block->StartOffset, newEnd, crSending);
    			    s_UpStatusBar.FillRange(newEnd, block->EndOffset, crBuffer);
                }
                else{
                    // entire block is still in buffer
                    total += block->EndOffset-block->StartOffset;
    			    s_UpStatusBar.FillRange(block->StartOffset, block->EndOffset, crBuffer);
                }
		    }
			// NEO: MOD END <-- Xanatos --
		}
   	    s_UpStatusBar.Draw(dc, rect->left, rect->top, bFlat);
	}

	// NEO: MOD - [Percentage] -- Xanatos -->
	if(NeoPrefs.ShowClientPercentage() && m_nUpPartCount)
	{
		float percent = (float)CompleteParts*100.0f/m_nUpPartCount;
		if(percent > 0.05f)
		{
			CString buffer;
			COLORREF oldclr = dc->SetTextColor(RGB(0,0,0));
			int iOMode = dc->SetBkMode(TRANSPARENT);
			buffer.Format(_T("%.1f%%"), percent);
			CFont *pOldFont = dc->SelectObject(&theApp.emuledlg->transferwnd->downloadlistctrl.m_fontSmall);

			#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
			OffsetRect(rect, -1,0);
			dc->DrawText(buffer, buffer.GetLength(), rect, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			OffsetRect(rect, 2,0);
			dc->DrawText(buffer, buffer.GetLength(), rect, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			OffsetRect(rect, -1,-1);
			dc->DrawText(buffer, buffer.GetLength(), rect, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			OffsetRect(rect, 0,2);
			dc->DrawText(buffer, buffer.GetLength(), rect, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			OffsetRect(rect, 0,-1);
			dc->SetTextColor(RGB(230,230,230));
			dc->DrawText(buffer, buffer.GetLength(), rect, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);

			dc->SelectObject(pOldFont);
			dc->SetBkMode(iOMode);
			dc->SetTextColor(oldclr);
		}
	}
	// NEO: MOD END <-- Xanatos --
} 

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
		if (eNewState == US_UPLOADING){
			m_fSentOutOfPartReqs = 0;
			// NEO: DSB - [DynamicSocketBuffer] -- Xanatos -->
			int sendBuff = 0;
 #ifdef LANCAST // NEO: NLC - [NeoLanCast]
			if(IsLanClient() && NeoPrefs.IsSetLanUploadBuffer())
				sendBuff = NeoPrefs.GetLanUploadBufferSize();
			else
 #endif //LANCAST // NEO: NLC END
			if(NeoPrefs.IsSetUploadBuffer())
				sendBuff = NeoPrefs.GetUploadBufferSize();

			if(sendBuff)
				socket->SetSendBufferSize(sendBuff);
			// NEO: DSB <-- Xanatos --
		}

		// don't add any final cleanups for US_NONE here
		m_nUploadState = (_EUploadState)eNewState;
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
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

	// NEO: NCS - [NeoCreditSystem] -- Xanatos -->
	float modif = 1.0F;
	if(thePrefs.UseCreditSystem())
		if (NeoPrefs.UseNeoCreditSystem())
			modif = credits->GetNeoScoreRatio(GetRequestFile() != NULL,GetIP());
		else
			modif = credits->GetScoreRatio(GetIP()); 

    return 10.0f * modif * (float)GetFilePrioAsNumber();
	// NEO: NCS END <-- Xanatos --
    //return 10.0f * credits->GetScoreRatio(GetIP()) * (float)GetFilePrioAsNumber();
}

/**
 * Gets the file multiplier for the file this client has requested.
 */
int CUpDownClient::GetFilePrioAsNumber() const {
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	
	// TODO coded by tecxx & herbert, one yet unsolved problem here:
	// sometimes a client asks for 2 files and there is no way to decide, which file the 
	// client finally gets. so it could happen that he is queued first because of a 
	// high prio file, but then asks for something completely different.
	int filepriority = 10; // standard
	//switch(currequpfile->GetUpPriority()){
	switch(currequpfile->GetUpPriorityEx()){ // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
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

// NEO: MQ - [MultiQueue] -- Xanatos -->
UINT CUpDownClient::GetFileScore(CKnownFile* currequpfile, bool isDownloading, uint32 downloadingTime) const
{
	// Score proportional to the waiting time since the last upload
	uint32 elapsedTime;
	if(isDownloading == false){		
		// Normal score based on the last upload of this file
		elapsedTime = ::GetTickCount() - currequpfile->GetStartUploadTime();
	}
	else {
		// We dont want one client to download forever
		if(downloadingTime < 900){ // NEO: CUT - [CleanUploadTiming]
			// Bonus during the first 15 minutes.
			return 1296000; // 15 days // NEO: CUT - [CleanUploadTiming]
		}
		else {
			// Normal score based on the current upload of this file
			// Remark: m_startUploadTime was actualized when the upload started (block sent)
			elapsedTime = time(NULL) - currequpfile->GetStartUploadTime(); // NEO: CUT - [CleanUploadTiming]
		}
	}

	uint32 filepriority = 4;
	//switch(currequpfile->GetUpPriority()){
	switch(currequpfile->GetUpPriorityEx()){ // NEO: NXC - [NewExtendedCategories] <-- Xanatos --
		case PR_VERYHIGH:	filepriority = elapsedTime/1;	break;
		case PR_HIGH:     	filepriority = elapsedTime/2;	break;
		case PR_NORMAL:   	filepriority = elapsedTime/4;	break;
		case PR_LOW:      	filepriority = elapsedTime/8;	break;
		case PR_VERYLOW:  	filepriority = elapsedTime/16;	break;
	} 

	return filepriority;
}
// NEO: MQ END <-- Xanatos --

// NEO: PRSF - [PushSmallRareFiles] -- Xanatos -->
float CUpDownClient::GetSmallFilePushRatio(CKnownFile* currequpfile) const {
	float boost = 1;
	float filesizeinchunks = ((float)currequpfile->GetFileSize() / (float)MB2B(NeoPrefs.GetPushSmallFilesSize()));
	if (filesizeinchunks < 1.0f) {
		if (filesizeinchunks > 0.001)
			boost = 1.0f / filesizeinchunks;
		else
			boost = 100.0f;
	}
	return boost < 1 ? 1 : boost; // 1 - 100
}

float CUpDownClient::GetRareFilePushRatio(CKnownFile* currequpfile) const{
	float boost = 1;
	uint32 QueuedCount = currequpfile->GetQueuedCount();
	if (QueuedCount)
		boost = (float)NeoPrefs.GetPushRareFilesValue() / QueuedCount;
	return boost < 1 ? 1 : boost; // 1 - 25
}

float CUpDownClient::GetRatioFilePushRatio(CKnownFile* currequpfile) const{
	float boost = 1;
	float ratio;
	if(currequpfile->IsPartFile()){
		if((uint64)((CPartFile*)currequpfile)->GetCompletedSize() == 0)
			return 1.0f;
		ratio = ((float)currequpfile->statistic.GetAllTimeTransferred()/(float)((CPartFile*)currequpfile)->GetCompletedSize());
	}else
		ratio = ((float)currequpfile->statistic.GetAllTimeTransferred()/(float)(uint64)currequpfile->GetFileSize());
	if(ratio < 1.0f)
		boost = (float)NeoPrefs.GetPushRatioFilesValue() * (1.0f - ratio);
	return boost < 1 ? 1 : boost; // 1 - 20
}
// NEO: PRSF END <-- Xanatos --

/**
 * Gets the current waiting score for this client, taking into consideration waiting
 * time, priority of requested file, and the client's credits.
 */
//uint32 CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue) const
uint32 CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue, bool forceRandomScore) const // NEO: RQ - [RandomQueue] <-- Xanatos --
{
	if (!m_pszUsername)
		return 0;

	if (credits == 0){
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		return 0;
	}
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	
	// bad clients (see note in function)
	if (credits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
		return 0;

	// friend slot
	// NEO: NMFS - [NiceMultiFriendSlots] -- Xanatos --
	//if (IsFriend() && GetFriendSlot() && !HasLowID())
	//	return 0x0FFFFFFF;

	if (IsBanned() || m_bGPLEvildoer)
		return 0;

	if (sysvalue && HasLowID() && !(socket && socket->IsConnected())){
		return 0;
	}

    int filepriority = GetFilePrioAsNumber();

	// calculate score, based on waitingtime and other factors
	float fBaseValue;
	if (onlybasevalue)
		fBaseValue = 100;
	// NEO: RQ - [RandomQueue] -- Xanatos -->
	else if (forceRandomScore)
		fBaseValue = 1;
	// NEO: RQ END <-- Xanatos --
	else if (!isdownloading)
		fBaseValue = (float)(::GetTickCount()-GetWaitStartTime())/1000;
	else{
		// we dont want one client to download forever
		// the first 15 min downloadtime counts as 15 min waitingtime and you get a 15 min bonus while you are in the first 15 min :)
		// (to avoid 20 sec downloads) after this the score won't raise anymore 
		fBaseValue = (float)(m_dwUploadTime-GetWaitStartTime());
		//ASSERT ( m_dwUploadTime-GetWaitStartTime() >= 0 ); //oct 28, 02: changed this from "> 0" to ">= 0" -> // 02-Okt-2006 []: ">=0" is always true!
		fBaseValue += (float)(::GetTickCount() - m_dwUploadTime > 900000)? 900000:1800000;
		//fBaseValue /= 1000; // NEO: CUT - [CleanUploadTiming] <-- Xanatos --
	}
	/*if(thePrefs.UseCreditSystem())
	{
		float modif = credits->GetScoreRatio(GetIP());
		fBaseValue *= modif;
	}*/

	// NEO: MQ - [MultiQueue] -- Xanatos -->
	if(NeoPrefs.UseMultiQueue()){
		// File Score
		// Remark: there is an overflow after ~49 days
		uint32 fileScore = GetFileScore(currequpfile, isdownloading, GetUpStartTimeDelay()); // about +1 point each mili second 

		// Final score	
		uint32 runTime = (GetTickCount() - theStats.starttime) / 1000;
		if(runTime <= 3600)
			// Less than 1 hour
			fBaseValue = fileScore + fBaseValue; // 1 second resolution
		if(runTime <= 2*3600)
			// Less than 2 hours
			fBaseValue = fileScore + fBaseValue/10; // 10 seconds resolution
		else
			// More than 2 hours
			fBaseValue = fileScore + fBaseValue/100; // 100 seconds resolution

	}else
	// NEO: MQ END <-- Xanatos --
	if (!onlybasevalue)
		fBaseValue *= (float(filepriority)/10.0f);

	// NEO: NCS - [NeoCreditSystem] -- Xanatos -->
	float modif = 1.0F;
	if(thePrefs.UseCreditSystem()){
		if (NeoPrefs.UseNeoCreditSystem()){
			modif = credits->GetNeoScoreRatio(GetRequestFile() != NULL,GetIP());
			if(NeoPrefs.UseNeoScoreSystem())
				modif *= 5;
		}else
			modif = credits->GetScoreRatio(GetIP());
	}
	// NEO: NCS END <-- Xanatos --

	// NEO: NFS - [NeoScoreSystem] -- Xanatos -->
	/* Xanatos:
	* New File Score system, the File score increase exponentialy to the waiting time.
	* For very long waiting times the credints loose thair importance.
	* This system is more Fair that the Official System.
	*/
	if(NeoPrefs.UseNeoScoreSystem())
		modif += (fBaseValue/900000);
	fBaseValue *= modif;
	// NEO: NFS <-- Xanatos --

	// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
	if(!(sysvalue == true && isdownloading == false) && currequpfile->GetReleasePriority())
	{
		fBaseValue += 1800000; // get it to work for very new users (add 30 Mins)
		fBaseValue *= currequpfile->GetReleaseModifyer(); // 1 - 200
	}
	// NEO: SRS END <-- Xanatos --

	// NEO: PRSF - [PushSmallRareFiles] -- Xanatos -->
	if (NeoPrefs.IsPushSmallFiles())
		fBaseValue *= GetSmallFilePushRatio(currequpfile); // 1 - 100

	if (NeoPrefs.IsPushRareFiles())
		fBaseValue *= GetRareFilePushRatio(currequpfile); // 1 - 25

	if (NeoPrefs.IsPushRatioFiles())
		fBaseValue *= GetRatioFilePushRatio(currequpfile); // 1 - 20
	// NEO: PRSF END <-- Xanatos --

	// NEO: FIX - [GetScore] -- Xanatos -->
	if(fBaseValue > 2073600000.0F) // 24 days, more would cause an overflow
		fBaseValue = 2073600000;
	// NEO: FIX END <-- Xanatos --

	// NEO: RQ - [RandomQueue] -- Xanatos -->
	if (!onlybasevalue && forceRandomScore && fBaseValue >= 1) // with fBaseValue < 1 an flaot underflow can occure !!!!
		fBaseValue = pow((float)(rand()+1)/(RAND_MAX+2), 1.0F/fBaseValue)*1000000;
	// NEO: RQ END <-- Xanatos --

	//if( (IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19 )
	//	fBaseValue *= 0.5f;
	return (uint32)fBaseValue;
}

// NEO: MOD -- Xanatos --
/*class CSyncHelper
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
};*/

void CUpDownClient::CreateNextBlockPackage()
{
	// NEO: RBT - [ReadBlockThread] -- Xanatos -->
	uint32 uBufferSize = 3 * EMBLOCKSIZE;
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	if(IsLanClient())
		uBufferSize = PARTSIZE;
	else
#endif //LANCAST // NEO: NLC END
	if(GetDatarate() >= KB2B(8))
		uBufferSize = (4 + GetDatarate()/KB2B(8))*EMBLOCKSIZE;
	// NEO: RBT END <-- Xanatos --

    // See if we can do an early return. There may be no new blocks to load from disk and add to buffer, or buffer may be large enough allready.
    if(m_BlockRequests_queue.IsEmpty() || // There are no new blocks requested
       m_addedPayloadQueueSession > GetQueueSessionPayloadUp() && m_addedPayloadQueueSession-GetQueueSessionPayloadUp() > /*50*1024*/ ((uBufferSize * 3) / 4)) { // the buffered data is large enough allready // NEO: RBT - [ReadBlockThread] <-- Xanatos --
        return;
    }

	UINT tmpPayloadQueue = 0; // NEO: RBT - [ReadBlockThread] <-- Xanatos --
    //CFile file;
	byte* filedata = 0;
	//CString fullname;
	//bool bFromPF = true; // Statistic to breakdown uploaded data by complete file vs. partfile.
	//CSyncHelper lockFile;
	try{
        // Buffer new data if current buffer is less than 100 KBytes
		// NEO: RBT - [ReadBlockThread] -- Xanatos -->
		POSITION pos = m_BlockRequests_queue.GetHeadPosition();
        while (pos && ((m_addedPayloadQueueSession + tmpPayloadQueue) <= GetQueueSessionPayloadUp() || (m_addedPayloadQueueSession + tmpPayloadQueue) - GetQueueSessionPayloadUp() < /*100*1024*/ uBufferSize)) 
		{
			// NEO: NUSM - [NeoUploadSlotManagement] -- Xanatos -->
			//Xman: At this point we do the check if it is time to kick the client if we kick soon, we don't add new packages
			m_bUpEndSoon=theApp.uploadqueue->CheckForTimeOver(this);
			if(m_bUpEndSoon==true)
				break;
			// NEO: NUSM END <-- Xanatos --
			POSITION removepos = pos;
			Requested_Block_Struct* currentblock = m_BlockRequests_queue.GetNext(pos);
			CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(currentblock->FileID);
			if (!srcfile)
				throw GetResString(IDS_ERR_REQ_FNF);
		// NEO: RBT END <-- Xanatos --

			/*while (!m_BlockRequests_queue.IsEmpty() &&
               (m_addedPayloadQueueSession <= GetQueueSessionPayloadUp() || m_addedPayloadQueueSession-GetQueueSessionPayloadUp() < 100*1024)) {
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
			}*/

			// NEO: MPS - [ManualPartSharing] -- Xanatos -->
			if(srcfile->GetPartState((UINT)(currentblock->StartOffset / PARTSIZE)) == PR_PART_OFF)
				throw StrLine(GetResString(IDS_X_PARTPERM_ACCESS_BLOCKED), GetUserName(), (UINT)(currentblock->StartOffset / PARTSIZE), srcfile->GetFileName());
			// NEO: MPS END <-- Xanatos --

			uint64 i64uTogo;
			if (currentblock->StartOffset > currentblock->EndOffset){
				i64uTogo = currentblock->EndOffset + (srcfile->GetFileSize() - currentblock->StartOffset);
			}
			else{
				i64uTogo = currentblock->EndOffset - currentblock->StartOffset;
				if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsComplete(currentblock->StartOffset,currentblock->EndOffset-1, true))
					throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
			}

			if( i64uTogo > EMBLOCKSIZE*3 )
				throw GetResString(IDS_ERR_LARGEREQBLOCK);
			uint32 togo = (uint32)i64uTogo;

			// NEO: RBT - [ReadBlockThread] -- Xanatos -->
			if(currentblock->filedata == RBT_ACTIVE){
				tmpPayloadQueue += (UINT)i64uTogo; // dont allow to many blocks to be queued
				continue; // data reading in progress skipp this block
			}else if(currentblock->filedata == RBT_ERROR) 
				throw GetResString(IDS_ERR_OPEN); // error data couldn't be read


			SetUploadFileID(srcfile); //Xman Fix Filtered Block Request
			
			ASSERT(!m_BlockRequests_queue.IsEmpty());
				
			if(currentblock->filedata == NULL)
			{
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
				if(srcfile->IsVoodooFile()){
					CVoodooSocket* master = srcfile->GetMaster(currentblock->StartOffset,currentblock->EndOffset-1);
					if(master == NULL)
						throw StrLine(GetResString(IDS_X_VOODOO_UL_FAILD_NO_MASTER),srcfile->GetFileName());
					ASSERT(theApp.voodoo->IsValidSocket(master));

					sDataRequest DataRequest;
					DataRequest.IdKey = (uint32) currentblock;
					DataRequest.StartOffset = currentblock->StartOffset;
					DataRequest.EndOffset  = currentblock->EndOffset;
					DataRequest.Client = (uint32) this;

					master->RequestFileData(srcfile,DataRequest);
				}else
#endif // VOODOO // NEO: VOODOO END
					srcfile->SetReadBlockFromFile(currentblock->StartOffset, togo, this, currentblock);

				currentblock->filedata = RBT_ACTIVE;

				tmpPayloadQueue += (UINT)i64uTogo; // dont allow to many blocks to be queued
				continue;
			}

			filedata = currentblock->filedata;
			currentblock->filedata = NULL;

			bool bFromPF = srcfile->IsPartFile(); // This is not a part file...
			// NEO: RBT END <-- Xanatos --

			/*
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
			if(srcfile->IsVoodooFile()){
				CVoodooSocket* master = srcfile->GetMaster(currentblock->StartOffset,currentblock->EndOffset-1);
				if(master == NULL)
					throw StrLine(GetResString(IDS_X_VOODOO_UL_FAILD_NO_MASTER),srcfile->GetFileName());

				sDataRequest DataRequest;
				DataRequest.IdKey = (uint32) currentblock;
				DataRequest.StartOffset = currentblock->StartOffset;
				DataRequest.EndOffset = currentblock->EndOffset;

				sDataBuffer* DataBuffer = master->GetBufferedUpload(srcfile,DataRequest);

				if(DataBuffer == NULL)
					return;
				
				filedata = DataBuffer->Data;
				uint32 size = DataBuffer->Size;
				delete DataBuffer;

				if(size != togo)
					throw StrLine(GetResString(IDS_X_VOODOO_UL_FAILD_BAD_SIZE),srcfile->GetFileName()); 
			}else
#endif // VOODOO // NEO: VOODO END <-- Xanatos --
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
			*/

			//if (!IsUploadingToPeerCache() && m_byDataCompVer == 1 && compFlag)
			if (!IsUploadingToPeerCache() 
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
			 && !IsLanClient() // compression is a real performance killer on a lan network
#endif //LANCAST // NEO: NLC END <-- Xanatos --
			 //&& m_byDataCompVer == 1 && compFlag)
			 && m_byDataCompVer == 1 && srcfile->IsCompressible() && !thePrefs.GetDontCompressBlocks()) // NEO: MOD - [IsCompressible] <-- Xanatos --
				CreatePackedPackets(filedata,togo,currentblock,bFromPF);
			else
				CreateStandartPackets(filedata,togo,currentblock,bFromPF);
			
#ifdef NEO_UBT // NEO: NUSM - [NeoUploadSlotManagement] -- Xanatos -->
			CEMSocket* cur_socket = GetFileUploadSocket();
			if(cur_socket != NULL && cur_socket->m_IsReady == false)
			{
				cur_socket->m_IsReady = true;
				theApp.uploadBandwidthThrottler->SetNoNeedSlot();
			}
#endif // NEO_UBT // NEO: NUSM END <-- Xanatos --

			// file statistic
			//srcfile->statistic.AddTransferred(togo);
			srcfile->statistic.AddTransferred(currentblock->StartOffset, togo);	// NEO: NPT - [NeoPartTraffic] <-- Xanatos --

            m_addedPayloadQueueSession += togo;

			//m_DoneBlocks_list.AddHead(m_BlockRequests_queue.RemoveHead());
			// NEO: RBT - [ReadBlockThread] -- Xanatos -->
			m_BlockRequests_queue.RemoveAt(removepos);
			m_DoneBlocks_list.AddHead(currentblock);
			// NEO: RBT END <-- Xanatos --

			delete[] filedata;
			filedata = 0;

			srcfile->UpdateStartUploadTime(); // NEO: MQ - [MultiQueue] <-- Xanatos --
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

//bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile)
bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile, bool bUDP) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
{
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* status = GetFileStatus(tempreqfile,true);

	if (GetExtendedRequestsVersion() == 0)
		return true;

	if(!status->ReadFileStatus(data,CFS_Normal,false))
		return false;

	bool bPartsNeeded = false; // NEO: BPS - [BetterPassiveSourceFinding]
	bool bRemotePartsNeeded = false; // NEO: SCT - [SubChunkTransfer]

	if(tempreqfile->IsPartFile()) 
	{
		uint8* abyPartStatus = status->GetPartStatus();
		ASSERT(abyPartStatus);
		UINT PartCount = status->GetPartCount();
		for (UINT i = 0; i < PartCount; i++)
		{
			if (((CPartFile*)tempreqfile)->IsComplete((uint64)i*PARTSIZE, ((uint64)(i+1)*PARTSIZE)-1, true)
			 && tempreqfile->GetPartState(i) == PR_PART_ON // NEO: IPS - [InteligentPartSharing]
			){
				// NEO: SCT - [SubChunkTransfer]
				if(!bUDP && !abyPartStatus[i])
				{
					bRemotePartsNeeded = true;
					break;
				}
				// NEO: SCT END
			}
			else
			{
				// NEO: BPS - [BetterPassiveSourceFinding]
				if(bUDP && abyPartStatus[i])
				{
					bPartsNeeded = true;
					break;
				}
				// NEO: BPS END
			}
		}

		// NEO: BPS - [BetterPassiveSourceFinding]
		if(bUDP && bPartsNeeded) // we need passiv source handling here only when we are udp
			HandleFileStatus(((CPartFile*)tempreqfile), true, true, true);
		// NEO: BPS END
	}
	
	// NEO: SCT - [SubChunkTransfer]
	if(!bUDP && !bRemotePartsNeeded) // if we are udp than the client have found some blocks he need so he can't be US_NONEEDEDPARTS
		SetUploadState(US_NONEEDEDPARTS);
	// NEO: SCT END
	// NEO: SCFS END <-- Xanatos --

	/*delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;
	if (GetExtendedRequestsVersion() == 0)
		return true;

	uint16 nED2KUpPartCount = data->ReadUInt16();
	if (!nED2KUpPartCount)
	{
		m_nUpPartCount = tempreqfile->GetPartCount();
		m_abyUpPartStatus = new uint8[m_nUpPartCount];
		memset(m_abyUpPartStatus, 0, m_nUpPartCount);
	}
	else
	{
		if (tempreqfile->GetED2KPartCount() != nED2KUpPartCount)
		{
			//We already checked if we are talking about the same file.. So if we get here, something really strange happened!
			m_nUpPartCount = 0;
			return false;
		}
		m_nUpPartCount = tempreqfile->GetPartCount();
		m_abyUpPartStatus = new uint8[m_nUpPartCount];
		uint16 done = 0;
		while (done != m_nUpPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (UINT i = 0; i != 8; i++)
			{
				m_abyUpPartStatus[done] = ((toread >> i) & 1) ? 1 : 0;
//				We may want to use this for another feature..
//				if (m_abyUpPartStatus[done] && !tempreqfile->IsComplete((uint64)done*PARTSIZE,((uint64)(done+1)*PARTSIZE)-1))
//					bPartsNeeded = true;
				done++;
				if (done == m_nUpPartCount)
					break;
			}
		}
	}*/

	if (GetExtendedRequestsVersion() > 1)
	{
		//uint16 nCompleteCountLast = GetUpCompleteSourcesCount();
		uint16 nCompleteCountLast = status->GetCompleteSourcesCount(); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
		uint16 nCompleteCountNew = data->ReadUInt16();
		//SetUpCompleteSourcesCount(nCompleteCountNew);
		status->SetCompleteSourcesCount(nCompleteCountNew); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
		if (nCompleteCountLast != nCompleteCountNew)
			tempreqfile->UpdatePartsInfo();
	}
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
	return true;
}

void CUpDownClient::CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	uint32 nPacketSize;
	CMemFile memfile((BYTE*)data,togo);

	//if (togo > 10240) 
	//	nPacketSize = togo/(uint32)(togo/10240);
	// NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	// David: to strong splitting is a real performance killer on very fast lines
	uint32 splittingsize = max(10*1024, GetDatarate()); // basic speed 10 KB/s
	if (togo > splittingsize) 
		nPacketSize = togo/(uint32)(togo/splittingsize);
	// NEO: NUBT END <-- Xanatos --
	else
		nPacketSize = togo;

	while (togo){
		if (togo < nPacketSize*2)
			nPacketSize = togo;
		ASSERT( nPacketSize );
		togo -= nPacketSize;

		uint64 statpos = (currentblock->EndOffset - togo) - nPacketSize;
		uint64 endpos = (currentblock->EndOffset - togo);
		if (IsUploadingToPeerCache())
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
				if (thePrefs.GetDebugClientTCPLevel() > 0){
					DebugSend("PeerCache-HTTP", this, GetUploadFileID());
					Debug(_T("  %hs\n"), str);
				}
			}
			dataHttp.Write(data, nPacketSize);
			data += nPacketSize;

			if (thePrefs.GetDebugClientTCPLevel() > 1){
				DebugSend("PeerCache-HTTP data", this, GetUploadFileID());
				Debug(_T("  Start=%I64u  End=%I64u  Size=%u\n"), statpos, endpos, nPacketSize);
			}

			UINT uRawPacketSize = (UINT)dataHttp.GetLength();
			LPBYTE pRawPacketData = dataHttp.Detach();
			CRawPacket* packet = new CRawPacket((char*)pRawPacketData, uRawPacketSize, bFromPF);
			m_pPCUpSocket->SendPacket(packet, true, false, nPacketSize);
			free(pRawPacketData);
		}
		else
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

			if (thePrefs.GetDebugClientTCPLevel() > 0){
				DebugSend("OP__SendingPart", this, GetUploadFileID());
				Debug(_T("  Start=%I64u  End=%I64u  Size=%u\n"), statpos, endpos, nPacketSize);
			}
			// put packet directly on socket
			
			socket->SendPacket(packet,true,false, nPacketSize);
		}
	}
}

void CUpDownClient::CreatePackedPackets(byte* data, uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	BYTE* output = new BYTE[togo+300];
	uLongf newsize = togo+300;
	// NEO: MOD - [CompressionLevel] -- Xanatos -->
	int	compressLevel = 9;
	if (theApp.uploadqueue->GetDatarate() > 102400)
		compressLevel = 1;
	else if (theApp.uploadqueue->GetDatarate() > 51200)
		compressLevel = 3;
	else if (theApp.uploadqueue->GetDatarate() > 20480)
		compressLevel = 6;
	UINT result = compress2(output, &newsize, data, togo, compressLevel);
	// NEO: MOD END <-- Xanatos --
	//UINT result = compress2(output, &newsize, data, togo, 9);
	if (result != Z_OK || togo <= newsize){
		delete[] output;
		CreateStandartPackets(data,togo,currentblock,bFromPF);
		return;
	}
	CMemFile memfile(output,newsize);
    uint32 oldSize = togo;
	togo = newsize;
	uint32 nPacketSize;
    //if (togo > 10240) 
    //    nPacketSize = togo/(uint32)(togo/10240);
	// NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
	// David: to strong splitting is a real performance killer on very fast lines
	uint32 splittingsize = max(10*1024, GetDatarate()); // basic speed 10 KB/s
	if (togo > splittingsize) 
		nPacketSize = togo/(uint32)(togo/splittingsize);
	// NEO: NUBT END <-- Xanatos --
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

		if (thePrefs.GetDebugClientTCPLevel() > 0){
			DebugSend("OP__CompressedPart", this, GetUploadFileID());
			Debug(_T("  Start=%I64u  BlockSize=%u  Size=%u\n"), statpos, newsize, nPacketSize);
		}
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

//#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
//	// David: Note if the master orders downlaod of a file we have in share without this fix wi will expirience an crash
//	if (newreqfile == oldreqfile || (newreqfile && !md4cmp(newreqfile->GetFileHash(),requpfileid)))
//#else
	if (newreqfile == oldreqfile)
//#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
		return;

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
	// clear old status
	//delete[] m_abyUpPartStatus;
	//m_abyUpPartStatus = NULL;
	//m_nUpPartCount = 0;
	//m_nUpCompleteSourcesCount= 0;

	if (newreqfile)
	{
		GetFileStatus(newreqfile, true); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
		newreqfile->AddUploadingClient(this);
		md4cpy(requpfileid, newreqfile->GetFileHash());
#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
		if(NeoPrefs.SaveSourceFileList() && source)
			source->AddSeenFile(newreqfile->GetFileHash(),newreqfile->GetFileSize());
#endif // NEO_CD // NEO: SFL END <-- Xanatos --
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
			//if ( !(CCollection::HasCollectionExtention(pDownloadingFile->GetFileName()) && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) ){
			if ( !(pDownloadingFile->IsCollection() && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) ){ // NEO: MOD - [IsCollection] <-- Xanatos --
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
    //DWORD curTick = ::GetTickCount(); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --

    uint64 sentBytesCompleteFile = 0;
    uint64 sentBytesPartFile = 0;
    uint64 sentBytesPayload = 0;

    if (GetFileUploadSocket() && (m_ePeerCacheUpState != PCUS_WAIT_CACHE_REPLY))
	{
		CEMSocket* s = GetFileUploadSocket();
		UINT uUpStatsPort;
        if (m_pPCUpSocket && IsUploadingToPeerCache())
		{
			uUpStatsPort = (UINT)-1;

            // Check if filedata has been sent via the normal socket since last call.
            uint64 sentBytesCompleteFileNormalSocket = socket->GetSentBytesCompleteFileSinceLastCallAndReset();
            uint64 sentBytesPartFileNormalSocket = socket->GetSentBytesPartFileSinceLastCallAndReset();

			if(thePrefs.GetVerbose() && (sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket > 0)) {
                AddDebugLogLine(false, _T("Sent file data via normal socket when in PC mode. Bytes: %I64i."), sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket);
			}
        }
		else
			uUpStatsPort = GetUserPort();

	    // Extended statistics information based on which client software and which port we sent this data to...
	    // This also updates the grand total for sent bytes, etc.  And where this data came from.
        sentBytesCompleteFile = s->GetSentBytesCompleteFileSinceLastCallAndReset();
        sentBytesPartFile = s->GetSentBytesPartFileSinceLastCallAndReset();
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
		if(!IsLanClient())
#endif //LANCAST // NEO: NLC END <-- Xanatos --
		{
#ifdef BW_MOD// NEO: BM - [BandwidthModeration] -- Xanatos -->
			bool bRelease;
			if(NeoPrefs.IsSeparateReleaseBandwidth())
				bRelease = socket && socket->m_eSlotType == ST_RELEASE;
			else
				bRelease = GetReleaseSlot();

			bool bFriend;
			if(NeoPrefs.IsSeparateFriendBandwidth())
				bFriend = socket && socket->m_eSlotType == ST_FRIEND;
			else
				bFriend = GetFriendSlot();
				
			thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, (UINT)sentBytesCompleteFile, bFriend, bRelease);
			thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, (UINT)sentBytesPartFile, bFriend, bRelease);
#else
			thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, (UINT)sentBytesCompleteFile, (IsFriend() && GetFriendSlot()));
			thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, (UINT)sentBytesPartFile, (IsFriend() && GetFriendSlot()));
#endif // BW_MOD // NEO: BM END <-- Xanatos --
		}

		m_nTransferredUp = (UINT)(m_nTransferredUp + sentBytesCompleteFile + sentBytesPartFile);
		AddUploadedSize(sentBytesCompleteFile + sentBytesPartFile); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --
        //credits->AddUploaded((UINT)(sentBytesCompleteFile + sentBytesPartFile), GetIP());
		credits->AddUploaded((UINT)(sentBytesCompleteFile + sentBytesPartFile), GetIP(), GetRequestFile() != NULL); // NEO: NCS - [NeoCreditSystem] <-- Xanatos --

        sentBytesPayload = s->GetSentPayloadSinceLastCallAndReset();
        m_nCurQueueSessionPayloadUp = (UINT)(m_nCurQueueSessionPayloadUp + sentBytesPayload);

		// NEO: NUSM - [NeoUploadSlotManagement] -- Xanatos -->
		//Xman: in CreateNextBlockPackage we saw this upload end soon, after all packets are send, we cancel this upload
		if (m_bUpEndSoon && socket->StandardPacketQueueIsEmpty()) {
			m_bUpEndSoon = false;
		// NEO: NUSM END <-- Xanatos --
        //if (theApp.uploadqueue->CheckForTimeOver(this)) {
            theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Completed transfer"), true);
			SendOutOfPartReqsAndAddToWaitingQueue();
        }
		else if(m_bUpEndSoon==false){  // NEO: NUBT - [NeoUploadBandwidthThrottler] <-- Xanatos --
		//else{
            // read blocks from file and put on socket
            CreateNextBlockPackage();
        }
    }

	// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos --
    /*if(sentBytesCompleteFile + sentBytesPartFile > 0 ||
        m_AvarageUDR_list.GetCount() == 0 || (curTick - m_AvarageUDR_list.GetTail().timestamp) > 1*1000) {
        // Store how much data we've transferred this round,
        // to be able to calculate average speed later
        // keep sum of all values in list up to date
        TransferredData newitem = {(UINT)(sentBytesCompleteFile + sentBytesPartFile), curTick};
        m_AvarageUDR_list.AddTail(newitem);
        m_nSumForAvgUpDataRate = (UINT)(m_nSumForAvgUpDataRate + sentBytesCompleteFile + sentBytesPartFile);
    }

    // remove to old values in list
    while (m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 10*1000) {
        // keep sum of all values in list up to date
        m_nSumForAvgUpDataRate -= m_AvarageUDR_list.RemoveHead().datalen;
    }

    // Calculate average speed for this slot
    //if(m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 0 && GetUpStartTimeDelay() > 2*1000) {
	if(m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 0 && GetUpStartTimeDelay() > 2) { // NEO: CUT - [CleanUploadTiming] <-- Xanatos --
        m_nUpDatarate = (UINT)(((ULONGLONG)m_nSumForAvgUpDataRate*1000) / (curTick - m_AvarageUDR_list.GetHead().timestamp));
    } else {
        // not enough values to calculate trustworthy speed. Use -1 to tell this
        m_nUpDatarate = 0; //-1;
    }

    // Check if it's time to update the display.
    if (curTick-m_lastRefreshedULDisplay > MINWAIT_BEFORE_ULDISPLAY_WINDOWUPDATE+(uint32)(rand()*800/RAND_MAX)) {
        // Update display
        theApp.emuledlg->transferwnd->uploadlistctrl.RefreshClient(this);
        theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
        m_lastRefreshedULDisplay = curTick;
    }*/

    return (UINT)(sentBytesCompleteFile + sentBytesPartFile);
}

// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
uint32 CUpDownClient::CalculateUploadRate(){
	// Add new sample
	TransferredData newSample;
	newSample.datalen = m_nSumForAvgUpDataRate;
	newSample.timestamp  = ::GetTickCount();
	m_AvarageUDR_list.AddHead(newSample);

	// Keep up to 21 samples (=> 20 seconds)
	while(m_AvarageUDR_list.GetSize() > 21){
		m_AvarageUDR_list.RemoveTail();
	}

	if(m_AvarageUDR_list.GetSize() > 1){	
		// Compute datarate (=> display)
		POSITION pos = m_AvarageUDR_list.FindIndex(NeoPrefs.GetDatarateSamples());
		if(pos == NULL)
			pos = m_AvarageUDR_list.GetTailPosition();
		TransferredData& oldestSample = m_AvarageUDR_list.GetAt(pos);
		uint32 deltaTime = newSample.timestamp - oldestSample.timestamp;
		uint32 deltaByte = newSample.datalen - oldestSample.datalen;
		if(deltaTime > 0)
			m_nUpDatarate = (UINT)((float)1000 * deltaByte / deltaTime);   // [bytes/s]
	}else
		m_nUpDatarate = 0;

	// We call this metod once per secund this is delay enough
    // Check if it's time to update the display.
	DWORD curTick = ::GetTickCount();
	if (curTick-m_lastRefreshedULDisplay > MINWAIT_BEFORE_ULDISPLAY_WINDOWUPDATE+m_random_update_wait) {
        // Update display
        theApp.emuledlg->transferwnd->uploadlistctrl.RefreshClient(this);
        theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
        m_lastRefreshedULDisplay = curTick;
    }

	return m_nUpDatarate;
}

UINT CUpDownClient::CheckUploadRate(int dataratestocheck)
{
	if(m_AvarageUDR_list.GetSize() > 4 && dataratestocheck > 2){	
		POSITION pos = m_AvarageUDR_list.FindIndex(dataratestocheck);
		if(pos == NULL)
			pos = m_AvarageUDR_list.GetTailPosition();
		TransferredData& oldestSample = m_AvarageUDR_list.GetAt(pos);
		//TransferredData& newSample = m_AvarageUDR_list.GetHead();
		//uint32 deltaTime = newSample.timestamp - oldestSample.timestamp;
		//uint32 deltaByte = newSample.datalen - oldestSample.datalen;
		uint32 deltaTime = ::GetTickCount() - oldestSample.timestamp;
		uint32 deltaByte = m_nSumForAvgUpDataRate - oldestSample.datalen;
		if(deltaTime > 0)
			return (UINT)((float)1000 * deltaByte / deltaTime);   // [bytes/s]
	}
	return 0;
}
// NEO: ASM END <-- Xanatos --

void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue()
{
	//OP_OUTOFPARTREQS will tell the downloading client to go back to OnQueue..
	//The main reason for this is that if we put the client back on queue and it goes
	//back to the upload before the socket times out... We get a situation where the
	//downloader thinks it already sent the requested blocks and the uploader thinks
	//the downloader didn't send any request blocks. Then the connection times out..
	//I did some tests with eDonkey also and it seems to work well with them also..
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__OutOfPartReqs", this);
	Packet* pPacket = new Packet(OP_OUTOFPARTREQS, 0);
	theStats.AddUpDataOverheadFileRequest(pPacket->size);
	socket->SendPacket(pPacket, true, true);
	m_fSentOutOfPartReqs = 1;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	if(!IsLanClient())
#endif //LANCAST // NEO: NLC END <-- Xanatos --
		theApp.uploadqueue->AddClientToQueue(this, true);
}

/**
 * See description for CEMSocket::TruncateQueues().
 */
void CUpDownClient::FlushSendBlocks(){ // call this when you stop upload, or the socket might be not able to send
    if (socket)      //socket may be NULL...
        socket->TruncateQueues();
}

void CUpDownClient::SendHashsetPacket(const uchar* forfileid)
{
	CKnownFile* file = theApp.sharedfiles->GetFileByID(forfileid);
	if (!file){
		CheckFailedFileIdReqs(forfileid);
		throw GetResString(IDS_ERR_REQ_FNF) + _T(" (SendHashsetPacket)");
	}

	CSafeMemFile data(1024);
	data.WriteHash16(file->GetFileHash());
	UINT parts = file->GetHashCount();
	data.WriteUInt16((uint16)parts);
	for (UINT i = 0; i < parts; i++)
		data.WriteHash16(file->GetPartHash(i));
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__HashSetAnswer", this, forfileid);
	Packet* packet = new Packet(&data);
	packet->opcode = OP_HASHSETANSWER;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true,true);
}

void CUpDownClient::ClearUploadBlockRequests()
{
	FlushSendBlocks();

	for (POSITION pos = m_BlockRequests_queue.GetHeadPosition();pos != 0;){
		// NEO: RBT - [ReadBlockThread] -- Xanatos -->
		if (m_BlockRequests_queue.GetAt(pos)->filedata != RBT_ACTIVE && m_BlockRequests_queue.GetAt(pos)->filedata != RBT_ERROR && m_BlockRequests_queue.GetAt(pos)->filedata != NULL) 
			delete[] m_BlockRequests_queue.GetAt(pos)->filedata;
		// NEO: RBT END <-- Xanatos --
		delete m_BlockRequests_queue.GetNext(pos);
	}
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
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__QueueRank", this);
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true,true);
}

void CUpDownClient::SendCommentInfo(/*const*/ CKnownFile *file)
{
	// NEO: XC - [ExtendedComments] -- Xanatos -->
	if (file == NULL || !ExtProtocolAvailable() || m_byAcceptCommentVer < 1)
		return;
	if (file != theApp.sharedfiles->GetFileByID(requpfileid)){
		if(!IsExtendedComments())	// not supported by remote user
			return;
	} else {
		if (!m_bCommentDirty)	// only send upfile comment once
			return;
		m_bCommentDirty = false;
	}
	// NEO: XC END <-- Xanatos --
	//if (!m_bCommentDirty || file == NULL || !ExtProtocolAvailable() || m_byAcceptCommentVer < 1)
	//	return;
	//m_bCommentDirty = false;

	UINT rating = file->GetFileRating();
	const CString& desc = file->GetFileComment();
	if (file->GetFileRating() == 0 && desc.IsEmpty())
		return;

	CSafeMemFile data(256);
	// NEO: XC - [ExtendedComments] -- Xanatos -->
	if(IsExtendedComments())
		data.WriteHash16(file->GetFileHash());
	// NEO: XC END <-- Xanatos --
	data.WriteUInt8((uint8)rating);
	data.WriteLongString(desc, GetUnicodeSupport());
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__FileDesc", this, file->GetFileHash());
	//Packet *packet = new Packet(&data,OP_EMULEPROT);
	Packet *packet = new Packet(&data,IsExtendedComments() ? OP_MODPROT : OP_EMULEPROT); // NEO: XC - [ExtendedComments] <-- Xanatos --
	packet->opcode = OP_FILEDESC;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true);
}

void CUpDownClient::AddRequestCount(const uchar* fileid)
{
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition(); pos != 0; ){
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		if (!md4cmp(cur_struct->fileid,fileid)){
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
	theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition();pos != 0;)
	{
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		cur_struct->badrequests = 0;
		cur_struct->lastasked = 0;	
	}
}

void CUpDownClient::Ban(LPCTSTR pszReason)
{
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
	SetDownloadState(DS_BANNED); // NEO: FIX - [SourceCount] <-- Xanatos --
	theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	theApp.emuledlg->transferwnd->queuelistctrl.RefreshClient(this);
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

// NEO: SQ - [SaveUploadQueue] -- Xanatos -->
void CUpDownClient::SaveQueueWaitTime(){
	if (credits == NULL){
		return;
	}
	credits->SaveQueueWaitTime();
}

void CUpDownClient::ClearQueueWaitTime(){
	if (credits == NULL){
		return;
	}
	credits->ClearQueueWaitTime();
}
// NEO: SQ END <-- Xanatos --

// NEO: SRS - [SmartReleaseSharing] -- Xanatos -->
bool CUpDownClient::GetReleaseSlot(bool bIntern) const
{
	if(bIntern)
	{
		int iLimit = 0;
		if(NeoPrefs.IsReleaseSlotLimit()){
			iLimit = NeoPrefs.GetReleaseSlotLimit();
		}else if(NeoPrefs.IsSeparateReleaseBandwidth()){
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
			float MaxUpload = theApp.bandwidthControl->GetMaxUpload();
#else
			float MaxUpload;
			if (thePrefs.IsDynUpEnabled())
				MaxUpload = theApp.lastCommonRouteFinder->GetUpload()/1024.0f;
			else
				MaxUpload = thePrefs.GetMaxUpload();
#endif // NEO_BC // NEO: NBC END
			iLimit = (int)((MaxUpload * NeoPrefs.GetReleaseBandwidthPercentage() / 100) / NeoPrefs.GetReleaseSlotSpeed());
			if(iLimit < 1) // just in case
				iLimit = 1; 
		}
		if(iLimit && theApp.uploadqueue->GetReleaseSlots() >= iLimit)
			return false;
	}

	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	return currequpfile && currequpfile->GetPowerShared();
}
// NEO: SRS END <-- Xanatos --

//bool CUpDownClient::GetFriendSlot() const
bool CUpDownClient::GetFriendSlot(bool bIntern) const // NEO: NMFS - [NiceMultiFriendSlots] <-- Xanatos --
{
	if (credits && theApp.clientcredits->CryptoAvailable()){
		switch(credits->GetCurrentIdentState(GetIP())){
			case IS_IDFAILED:
			case IS_IDNEEDED:
			case IS_IDBADGUY:
				return false;
		}
	}
	// NEO: NMFS - [NiceMultiFriendSlots] -- Xanatos -->
	// Note: we save the slots between sessions, so when the momentan max amount is used dont let the client be promoted handle it as usual
	if(bIntern)
	{
		int iLimit = 1;
		if(NeoPrefs.IsFriendSlotLimit()){
			iLimit = NeoPrefs.GetFriendSlotLimit();
		}else if(NeoPrefs.IsSeparateFriendBandwidth()){
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
			float MaxUpload = theApp.bandwidthControl->GetMaxUpload();
#else
			float MaxUpload;
			if (thePrefs.IsDynUpEnabled())
				MaxUpload = theApp.lastCommonRouteFinder->GetUpload()/1024.0f;
			else
				MaxUpload = thePrefs.GetMaxUpload();
#endif // NEO_BC // NEO: NBC END
			iLimit = (int)((MaxUpload * NeoPrefs.GetFriendBandwidthPercentage() / 100) / NeoPrefs.GetFriendSlotSpeed());
			if(iLimit < 1) // just in case
				iLimit = 1;
		}
		if(theApp.uploadqueue->GetFriendSlots() >= iLimit)
			return false;
	}
	// NEO: NMFS END <-- Xanatos --
	return m_bFriendSlot;
}

CEMSocket* CUpDownClient::GetFileUploadSocket(bool bLog)
{
    if (m_pPCUpSocket && (IsUploadingToPeerCache() || m_ePeerCacheUpState == PCUS_WAIT_CACHE_REPLY))
	{
        if (bLog && thePrefs.GetVerbose())
            AddDebugLogLine(false, _T("%s got peercache socket."), DbgGetClientInfo());
        return m_pPCUpSocket;
    }
	else
	{
        if (bLog && thePrefs.GetVerbose())
            AddDebugLogLine(false, _T("%s got normal socket."), DbgGetClientInfo());
        return socket;
    }
}

void CUpDownClient::SetCollectionUploadSlot(bool bValue){
	ASSERT( !IsDownloading() || bValue == m_bCollectionUploadSlot );
	m_bCollectionUploadSlot = bValue;
}

// NEO: MOD - [SendAcceptUpload] -- Xanatos -->
void CUpDownClient::SendAcceptUpload()
{
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler]
	socket->SetPriorityReceive(true);
#endif // NEO_DBT // NEO: NDBT END
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler]
	socket->SetPrioritySend(true);
#endif // NEO_UBT // NEO: NUBT END

	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__AcceptUploadReq", this);
	Packet* packet =  new Packet(OP_ACCEPTUPLOADREQ,0);
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true);
}
// NEO: MOD END <-- Xanatos --

#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler] -- Xanatos -->
bool CUpDownClient::HaveTrickleSlot() const {
	CEMSocket* socket = (const_cast<CUpDownClient*>(this))->GetFileUploadSocket();
	if (socket) 
		return socket->m_eSlotState != SS_FULL; 
	else 
		return false;
}
#endif // NEO_UBT // NEO: NUBT END <-- Xanatos --

// NEO: MOD - [IsValidSource] -- Xanatos -->
bool CUpDownClient::IsValidSource2() const
{
	bool valid = false;
	switch(GetUploadState())
	{
		case US_UPLOADING:
		case US_ONUPLOADQUEUE:
		case US_PENDING: // NEO: MOD - [NewUploadState]
		case US_NONEEDEDPARTS: // NEO: SCT - [SubChunkTransfer]
			valid = IsEd2kClient();
	}
	return valid;
}
// NEO: MOD <-- Xanatos --
