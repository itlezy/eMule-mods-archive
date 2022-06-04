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
#include "TransferDlg.h"
#include "Log.h"
#include "Collection.h"
#include <math.h> //Xman

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//	members of CUpDownClient
//	which are mainly used for uploading functions 

//Xman
/*
CBarShader CUpDownClient::s_UpStatusBar(16);
*/
//Xman end

void CUpDownClient::DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const
{
    COLORREF crNeither;
	COLORREF crNextSending;
	COLORREF crBoth;
	COLORREF crSending;
	// ==> See chunk that we hide [SiRoB] - Stulle
	COLORREF crHiddenPartBySOTN;
	COLORREF crHiddenPartByHideOS;
	COLORREF crHiddenPartBySOTNandHideOS;
	// <== See chunk that we hide [SiRoB] - Stulle

	//Xman Xtreme Upload
	/*
    if(GetSlotNumber() <= theApp.uploadqueue->GetActiveUploadsCount() ||
       (GetUploadState() != US_UPLOADING && GetUploadState() != US_CONNECTING) ) {
        crNeither = RGB(224, 224, 224);
	    crNextSending = RGB(255,208,0);
	    crBoth = bFlat ? RGB(0, 0, 0) : RGB(104, 104, 104);
	    crSending = RGB(0, 150, 0);
	*/
    if(GetUploadState() == US_UPLOADING  ) {
        crNeither = RGB(224, 224, 224);
	    crNextSending = RGB(255,208,0);
	    crBoth = bFlat ? RGB(0, 0, 0) : RGB(104, 104, 104);
	    crSending = RGB(0, 150, 0); //RGB(186, 240, 0);
	//Xman end
		// ==> See chunk that we hide [SiRoB] - Stulle
		crHiddenPartBySOTN = RGB(192, 96, 255);
		crHiddenPartByHideOS = RGB(96, 192, 255);
		crHiddenPartBySOTNandHideOS = RGB(96, 96, 255);
		// <== See chunk that we hide [SiRoB] - Stulle
    } else {
        // grayed out
        crNeither = RGB(248, 248, 248);
	    crNextSending = RGB(255,244,191);
	    crBoth = bFlat ? RGB(191, 191, 191) : RGB(191, 191, 191);
	    crSending = RGB(191, 229, 191);
		// ==> See chunk that we hide [SiRoB] - Stulle
		crHiddenPartBySOTN = RGB(224, 128, 255);
		crHiddenPartByHideOS = RGB(128, 224, 255);
		crHiddenPartBySOTNandHideOS = RGB(128, 128, 255);
		// <== See chunk that we hide [SiRoB] - Stulle
    }

	// wistily: UpStatusFix
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	EMFileSize filesize;
	if (currequpfile)
		filesize = currequpfile->GetFileSize();
	else
		filesize = (uint64)(PARTSIZE * (uint64)m_nUpPartCount);
	// wistily: UpStatusFix

	//Xman
	/*
    if(filesize > (uint64)0) {
	    s_UpStatusBar.SetFileSize(filesize); 
	    s_UpStatusBar.SetHeight(rect->bottom - rect->top); 
	    s_UpStatusBar.SetWidth(rect->right - rect->left); 
	    s_UpStatusBar.Fill(crNeither); 
	    if (!onlygreyrect && m_abyUpPartStatus) { 
		    for (UINT i = 0; i < m_nUpPartCount; i++)
			    if (m_abyUpPartStatus[i])
				    s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crBoth);
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
	*/
    if(filesize > (uint64)0) {
		// Set size and fill with default color (grey)
		CBarShader statusBar(rect->bottom - rect->top, rect->right - rect->left);
		statusBar.SetFileSize(filesize); 
		statusBar.Fill(crNeither); 
		// ==> See chunk that we hide [SiRoB] - Stulle
		/*
	    if (!onlygreyrect && m_abyUpPartStatus) { 
		    for (UINT i = 0;i < m_nUpPartCount;i++)
			    if(m_abyUpPartStatus[i])
				    statusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crBoth);
	    }
		*/
		if (!onlygreyrect && m_abyUpPartStatus && currequpfile) { 
			UINT i;
			for (i = 0;i < currequpfile->GetPartCount();i++) {
				if (m_abyUpPartStatus[i] == 0)
					continue;
				COLORREF crChunk;
				if (m_abyUpPartStatus[i]&SC_AVAILABLE)
					crChunk = crBoth;
				else if (m_abyUpPartStatus[i]&SC_HIDDENBYSOTN && m_abyUpPartStatus[i]&SC_HIDDENBYHIDEOS)
					crChunk = crHiddenPartBySOTNandHideOS;
				else if (m_abyUpPartStatus[i]&SC_HIDDENBYSOTN)
					crChunk = crHiddenPartBySOTN;
				else if (m_abyUpPartStatus[i]&SC_HIDDENBYHIDEOS)
					crChunk = crHiddenPartByHideOS;
				else 
					crChunk = crBoth;
				statusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crChunk);
			}
			
		}
		// <== See chunk that we hide [SiRoB] - Stulle
	    const Requested_Block_Struct* block;
	    if (!m_BlockRequests_queue.IsEmpty()){
		    block = m_BlockRequests_queue.GetHead();
		    if(block){
			    uint32 start = (uint32)(block->StartOffset/PARTSIZE);
			    statusBar.FillRange((uint64)start*PARTSIZE, (uint64)(start+1)*PARTSIZE, crNextSending);
		    }
	    }
	    if (!m_DoneBlocks_list.IsEmpty()){
		    block = m_DoneBlocks_list.GetHead(); 
		    if(block){
			    uint32 start = (uint32)(block->StartOffset/PARTSIZE);
			    statusBar.FillRange((uint64)start*PARTSIZE, (uint64)(start+1)*PARTSIZE, crNextSending);
		    }
	    }
	    if (!m_DoneBlocks_list.IsEmpty()){
		    for(POSITION pos=m_DoneBlocks_list.GetHeadPosition();pos!=0;){
			    block = m_DoneBlocks_list.GetNext(pos);
			    statusBar.FillRange(block->StartOffset, block->EndOffset + 1, crSending);
		    }
	    }
   	    statusBar.Draw(dc, rect->left, rect->top, bFlat);
    }
	//Xman end
} 

void CUpDownClient::SetUploadState(EUploadState eNewState)
{
	if (eNewState != m_nUploadState)
	{
		//MORPH START - ReadBlockFromFileThread
		if (m_readblockthread) {
			m_readblockthread->StopReadBlock();
			m_readblockthread = NULL;
		}
		//MORPH END   - ReadBlockFromFileThread

		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		if (m_nUploadState == US_UPLOADING)
		{
			// Reset upload data rate computation
			m_nUpDatarate = 0;
			m_nSumForAvgUpDataRate = 0;
			m_AvarageUDR_list.RemoveAll();
		}
		if (eNewState == US_UPLOADING)
			m_fSentOutOfPartReqs = 0;
		*/
		if(m_nUploadState == US_UPLOADING || eNewState == US_UPLOADING || m_nUploadState== US_CONNECTING){
			m_nUpDatarate = 0;
			m_nUpDatarate10 = 0;
			m_nUpDatarateMeasure = 0;
			m_upHistory_list.RemoveAll();
		//Xman end

			if (eNewState == US_UPLOADING)
			{ //Xman
				m_fSentOutOfPartReqs = 0;
		//Xman
				int newValue = thePrefs.GetSendbuffersize(); // default: 8192;  
				//int oldValue = 0;
				//int size = sizeof(newValue);

				if(socket != NULL)
				{
				   socket->SetSockOpt(SO_SNDBUF, &newValue, sizeof(newValue), SOL_SOCKET);
				   //socket->GetSockOpt(SO_SNDBUF, &oldValue, &size, SOL_SOCKET);
				   //AddDebugLogLine(false,_T("socketbuffer: %u"), oldValue);
					
				   //Xman
				   // Pawcio: BC
				   BOOL noDelay=true;
				   socket->SetSockOpt(TCP_NODELAY,&noDelay,sizeof(BOOL),IPPROTO_TCP);
				   // <--- Pawcio: BC

				}
			}
		}
		//Xman end
		
		//Xman remove banned. remark: this method is called recursive
		if(eNewState == US_BANNED && (m_nUploadState == US_UPLOADING || m_nUploadState == US_CONNECTING))
		{
			theApp.uploadqueue->RemoveFromUploadQueue(this, _T("banned client"));
		}
		//Xman end

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
		//zz_fly :: in the Optimized on ClientCredits, banned client has no credits
		/*
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		*/
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) || (GetUploadState()==US_BANNED) );
		//zz_fly :: in the Optimized on ClientCredits, banned client has no credits
		return 0.0F;
	}

	//Xman
	/*
    return 10.0f * credits->GetScoreRatio(GetIP()) * (float)GetFilePrioAsNumber();
	*/
    return 10.0f*credits->GetScoreRatio(this)*float(GetFilePrioAsNumber());
	//Xman end
}

/**
 * Gets the file multiplier for the file this client has requested.
 */
//Xman modified:
/*
int CUpDownClient::GetFilePrioAsNumber() const {
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	
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
*/
int CUpDownClient::GetFilePrioAsNumber() const {
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	
	int filepriority = 10;
	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	if (theApp.clientcredits->IsSaveUploadQueueWaitTime()){
		switch(currequpfile->GetUpPriority()){
		// --> Moonlight: SUQWT - Changed the priority distribution for a wider spread.
			case PR_POWER:
				if(currequpfile->statistic.GetAllTimeTransferred() < 100 * 1024 * 1024 || (currequpfile->statistic.GetAllTimeTransferred() < (uint64)((uint64)currequpfile->GetFileSize()*1.5f)))
					filepriority=320; // 200, 60% boost
				else
					filepriority=160; // 100, 60% boost
				break;
			case PR_VERYHIGH:
				filepriority = 27;  // 18, 50% boost    <-- SUQWT - original values commented.
				break;
			case PR_HIGH: 
				filepriority = 12;  // 9, 33% boost
				break; 
			case PR_LOW: 
				filepriority = 5;   // 6, 17% reduction
				break; 
			case PR_VERYLOW:
				filepriority = 2;   // 2, no change
				break;
			case PR_NORMAL: 
				default: 
				filepriority = 8;   // 7, 14% boost
			break; 
		// <-- Moonlight: SUQWT
		} 
	}
	else{
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	switch(currequpfile->GetUpPriority()){
		//Xman PowerRelease
		case PR_POWER:
			if(currequpfile->statistic.GetAllTimeTransferred() < 100 * 1024 * 1024 || (currequpfile->statistic.GetAllTimeTransferred() < (uint64)((uint64)currequpfile->GetFileSize()*1.5f)))
				filepriority=200;
			else
				filepriority=100;
			break;
		//Xman end
		case PR_VERYHIGH:
			filepriority = 25;
			break;
		case PR_HIGH: 
			filepriority = thePrefs.UseAdvancedAutoPtio() ? 15 : 14; //Xman advanced upload-priority
			break; 
		case PR_LOW: 
			filepriority = thePrefs.UseAdvancedAutoPtio() ? 6 : 7; //Xman advanced upload-priority
			break; 
		case PR_VERYLOW:
			filepriority = 2;
			break;
		case PR_NORMAL: 
			default: 
			filepriority = 10; 
		break; 
	} 
	} // SUQWT [Moonlight/EastShare/ MorphXT] - Stulle

    return filepriority;
}
//Xman end

/**
 * Gets the current waiting score for this client, taking into consideration waiting
 * time, priority of requested file, and the client's credits.
 */
uint32 CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue) const
{
	//Xman Code Improvement
	/*
	if (!m_pszUsername)
	*/
	if (m_pszUsername == NULL || GetUploadFileID() == NULL)
	//Xman end
		return 0;

	if (credits == 0){
		//zz_fly :: in the Optimized on ClientCredits, banned client has no credits
		/*
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		*/
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) || (GetUploadState()==US_BANNED) );
		//zz_fly :: in the Optimized on ClientCredits, banned client has no credits
		return 0;
	}
	//Xman Code Improvement
	/*
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	*/
	//Xman end
	
	// bad clients (see note in function)
	if (credits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
		return 0;

	//Xman Code Improvement
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;
	//Xman end

	// friend slot
	if (IsFriend() && GetFriendSlot() && !HasLowID())
		return 0x0FFFFFFF;

	//Xman Code Improvement
	/*
	if (IsBanned() || m_bGPLEvildoer)
	*/
	if (GetUploadState()==US_BANNED || m_bGPLEvildoer)
	//Xman end
		return 0;

	if (sysvalue && HasLowID() && !(socket && socket->IsConnected())){
		return 0;
	}

	//Xman
	/*
    int filepriority = GetFilePrioAsNumber();
	*/
	//Xman end

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
		//Xman Xtreme Upload
		/*
		fBaseValue += (float)(::GetTickCount() - m_dwUploadTime > 900000)? 900000:1800000;
		*/
		//we can not give a high bonus during the first 15 minutes, because of varying slotspeed
		//Xtreme Mod avoids too short uploadsessions in CheckFortimeover()
		fBaseValue +=(float)900000;
		//Xman end
		fBaseValue /= 1000;
	}

	// ==> Release Bonus [sivka] - Stulle
	if (!onlybasevalue && // we actually need the complete score with waiting time
		currequpfile->IsPartFile() == false && // no bonus for partfiles
		(currequpfile->GetUpPriority() == PR_VERYHIGH || currequpfile->GetUpPriority() == PR_POWER) && // release upload prio
		!IsLeecher()) // not a sucker
		fBaseValue += 43200.0f * thePrefs.GetReleaseBonus(); // 12h*factor
	// <== Release Bonus [sivka] - Stulle

	// ==> CreditSystems [EastShare/ MorphXT] - Stulle
	/*
	if(thePrefs.UseCreditSystem())
	{
		//Xman
		/*
		float modif = credits->GetScoreRatio(GetIP());
		*//*
		float modif = credits->GetScoreRatio(this);
		//Xman end
		fBaseValue *= modif;
	}
	*/
	float modif = credits->GetScoreRatio(this);

	// ==> Modified FineCS [CiccioBastardo/Stulle] - Stulle
	if (thePrefs.FineCS() && currequpfile->IsPartFile() && !IsFriend())
	{ // we have requested file
		bool FineCS = false;
		switch (thePrefs.GetCreditSystem())	{
			case CS_LOVELACE:{
					if (modif <= 0.984290578f){ // default!
						FineCS = true;
						modif = 1.0f;
					}
				}break;
			case CS_PAWCIO:{
					if (modif <= 3.0f){ // default!
						FineCS = true;
						modif = 1.0f;
					}
				}break;
			case CS_EASTSHARE:{
					if (modif <= 1.0f){ // default!
						FineCS = true;
						if (credits->GetCurrentIdentState(GetIP()) != IS_NOTAVAILABLE)
							modif = 1.0f;
					}
				}break;
			case CS_SIVKA:{
					if (modif <= 1.0f){ // default!
						FineCS = true;
						if (credits->GetCurrentIdentState(GetIP()) != IS_IDNEEDED && theApp.clientcredits->CryptoAvailable() ||
							credits->GetCurrentIdentState(GetIP()) != IS_IDFAILED ||
							credits->GetCurrentIdentState(GetIP()) != IS_IDBADGUY)
							modif = 1.0f;
					}
				}break;
			case CS_TK4:{
					if (modif <= 10.0f){ // default!
						FineCS = true;
						modif = 1.0f;
					}
				}break;
			case CS_XMAN:{
					if (modif <= 1.0f){ // default!
						FineCS = true;
						if((credits->GetCurrentIdentState(GetIP()) != IS_IDFAILED ||
							credits->GetCurrentIdentState(GetIP()) != IS_IDBADGUY ||
							credits->GetCurrentIdentState(GetIP()) != IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable())
							modif = 1.0f;
					}
				}break;
			case CS_RATIO:
			case CS_SWAT:
			case CS_ZZUL:
			case CS_OFFICIAL:
			default:{
					if (modif <= 1.0f){ // default!
						FineCS = true;
						modif = 1.0f;
					}
				}break;
		} // current modif is smaller but default modif
		// bad clients will be calculated with their penalty

		if (FineCS)
		{
			#define UP_DOWN_GAP_LIMIT (4*PARTSIZE)

			sint64 diff = credits->GetUploadedTotal() - credits->GetDownloadedTotal(); // sint64 handles signed sizes > 2GB (yet, I had a case which was handled wrongly)
		
			if (credits->GetDownloadedTotal() < PARTSIZE) // If not received at least a chunk, limit "risk" to 3 chunks instead of 4
				diff += PARTSIZE;

			if (diff > UP_DOWN_GAP_LIMIT) {
				modif *= (float)UP_DOWN_GAP_LIMIT/diff; // This is surely smaller than 1
				modif *= modif;
			}
		}
	}
	// <== Modified FineCS [CiccioBastardo/Stulle] - Stulle

	// ==> Release Score Assurance [Stulle] - Stulle
	if((thePrefs.GetReleaseScoreAssurance() &&
		currequpfile->IsPartFile() == false && // no bonus for partfiles
		(currequpfile->GetUpPriority() == PR_VERYHIGH || currequpfile->GetUpPriority() == PR_POWER) && // release upload prio
		!IsLeecher()) || // not a sucker
		IsFriend()) // or a friend
	{
		switch (thePrefs.GetCreditSystem())	{
			case CS_LOVELACE:{
					if (modif <= 0.984290578f)
						modif = 0.984290578f;
				}break;
			case CS_PAWCIO:{
					if (modif <= 3.0f)
						modif = 3.0f;
				}break;
			case CS_TK4:{
				if(modif <= 10.0f)
					modif = 10.0f;
			}break;
			case CS_RATIO:
			case CS_EASTSHARE:
			case CS_SIVKA:
			case CS_SWAT:
			case CS_XMAN:
			case CS_ZZUL:
			case CS_OFFICIAL:
			default:{
					if (modif <= 1.0f)
						modif = 1.0f;
				}break;
		}
	}
	// <== Release Score Assurance [Stulle] - Stulle

	fBaseValue *= modif;
	// <== CreditSystems [EastShare/ MorphXT] - Stulle

	if (!onlybasevalue)
	//Xman
	// Maella -One-queue-per-file- (idea bloodymad)
	/*
		fBaseValue *= (float(filepriority)/10.0f);
	*/
	{
		if(thePrefs.GetEnableMultiQueue() == false)
			fBaseValue *= (float(GetFilePrioAsNumber())/10.0f);
		else
		{
			// File Score
			// Remark: there is an overflow after ~49 days
			uint32 fileScore = currequpfile->GetFileScore(isdownloading ? GetUpStartTimeDelay() : 0); // about +1 point each second 
			// Final score	
			// Remark: The whole timing of eMule should be rewritten. 
			//         The rollover of the main timer (GetTickCount) is not supported.
			//         A logarithmic scale would fit better here, but it might be slower.
			uint32 runTime = (GetTickCount() - theStats.starttime) / 1000;
			if(runTime <= 2*3600)
				// Less than 2 hour
			{
				fBaseValue = 1000*fileScore + fBaseValue; // 1 second resolution
			}	
			else
				fBaseValue = 1000*fileScore + fBaseValue/10; // 10 seconds resolution
		}
	}
	//Xman end

	if( (IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19 )
		fBaseValue *= 0.5f;
	//Xman Xtreme Mod : 80% score for non SI clients
	else if(credits->GetCurrentIdentState(GetIP()) != IS_IDENTIFIED)
		fBaseValue *= 0.8f;
	//Xman end

	// ==> push small files [sivka] - Stulle
	if(GetSmallFilePush())
		fBaseValue *= thePrefs.GetPushSmallFileBoost();
	// <== push small files [sivka] - Stulle

	fBaseValue *= GetRareFilePushRatio() ; // push rare file - Stulle

	//Xman Anti-Leecher
	if(IsLeecher()>0)
		fBaseValue *=0.33f;
	//Xman end

	return (uint32)fBaseValue;
}

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

//Xman Code Improvement
// BEGIN SiRoB: ReadBlockFromFileThread
/*
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

			if (!IsUploadingToPeerCache() && m_byDataCompVer == 1 && compFlag)
				CreatePackedPackets(filedata,togo,currentblock,bFromPF);
			else
				CreateStandartPackets(filedata,togo,currentblock,bFromPF);
			
			// file statistic
			srcfile->statistic.AddTransferred(togo);

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
*/
void CUpDownClient::CreateNextBlockPackage(){
	//Xman Full Chunk
	if(upendsoon)
		return;
	//Xman end

	// See if we can do an early return. There may be no new blocks to load from disk and add to buffer, or buffer may be large enough allready.
	// ==> Mephisto Upload - Mephisto
	/*
	const uint32 nBufferLimit = theApp.uploadqueue->UseHighSpeedUpload() ? (800 * 1024) : (160 * 1024); //Xman changed - 160 was 50
	*/
	//Note: The following solution allows up to two seconds for reading a 180 kB block which should be well enough
	const uint32 nBufferLimit = max(GetUploadDatarate()<<1, 50*1024);
	// <== Mephisto Upload - Mephisto
    if(m_BlockRequests_queue.IsEmpty() || // There is no new blocks requested
       m_abyfiledata == (byte*)-2 || //we are still waiting for a block read from disk
	   m_abyfiledata != (byte*)-1 && //Make sur we don't have something to do
	    m_addedPayloadQueueSession > GetQueueSessionPayloadUp() && GetPayloadInBuffer() > nBufferLimit) { // the buffered data is large enough already according to client datarate
		return;
	}

	CString fullname;
	bool bFromPF = true; // Statistic to breakdown uploaded data by complete file vs. partfile.

	try{
		// Buffer new data if current buffer is less than 180 KBytes
		// ==> Mephisto Upload - Mephisto
		/*
		while (!m_BlockRequests_queue.IsEmpty() && m_abyfiledata != (byte*)-2 &&
				(m_addedPayloadQueueSession <= GetQueueSessionPayloadUp() || m_addedPayloadQueueSession-GetQueueSessionPayloadUp() < EMBLOCKSIZE)) { //Xman changed  
		*/
		while (!m_BlockRequests_queue.IsEmpty() && m_abyfiledata != (byte*)-2) {
		// <== Mephisto Upload - Mephisto
			//Xman Full Chunk
			//at this point we do the check if it is time to kick the client
			//if we kick soon, we don't add new packages
			//Xman ReadBlockFromFileThread:
			//-->we first have to check if we have unprocessed data (can happen if full chunk is disabled)
			//-->first process it, then check for timeOver
			//-->in case of an exception, allow to throw it
			// Stulle remarks:
			// This used to be "filedata==null" but then it was only filedata. Now we distinguish
			// in m_abyfiledata and filedata_ReadFromDisk. So back then we reset the filedata member
			// variable to NULL when we created new block packages from the read data. Now we use
			// m_abyfiledata to read blocks from disk to memory. The pointer to that data is then
			// copied to the local filedata_ReadFromDisk inside this function (see below). At that
			// point we request new data to be read and later on proceed to create the new packages
			// using the data filedata_ReadFromDisk is pointing to. When this is done we set that
			// pointer back to NULL and we begin anew.
			// So what can we gather from this? Well, first of all, m_abyfiledata is only getting NULL
			// when we get to ClearUploadBlockRequests() or have an exception. So we can't just check
			// for m_abyfiledata==NULL because this will seldomly ever return true. In order to adapt
			// our best bet is to check whenever we don't read at this time or have an error. We can
			// then break if m_abyfiledata is empty and if it isn't we will at least stop new requests
			// from being carried out.
			if(m_abyfiledata != (byte*)-1) // we don't get here if m_abyfiledata != (byte*)-2 returns true
				upendsoon = theApp.uploadqueue->CheckForTimeOver(this);
			if(upendsoon && m_abyfiledata == NULL)
				break;
			//Xman end

			if (m_abyfiledata == (byte*)-1) {
				//An error occured //Fafner: note: this error is common during file copying - 080421
				m_readblockthread = NULL; //thread already terminated at this point
				m_abyfiledata = NULL;
				if(!m_BlockRequests_queue.IsEmpty() && !theApp.sharedfiles->IsUnsharedFile(m_BlockRequests_queue.GetHead()->FileID))
					theApp.sharedfiles->Reload();
				throw GetResString(IDS_ERR_OPEN);
			}

			POSITION pos = m_BlockRequests_queue.GetHeadPosition();
			Requested_Block_Struct* currentblock_ReadFromDisk = m_BlockRequests_queue.GetNext(pos);
			CKnownFile* srcfile_ReadFromDisk = theApp.sharedfiles->GetFileByID(currentblock_ReadFromDisk->FileID);
			if (!srcfile_ReadFromDisk)
				throw GetResString(IDS_ERR_REQ_FNF);

			Requested_Block_Struct* currentBlock = currentblock_ReadFromDisk;
			BYTE* filedata_ReadFromDisk = NULL;
			uint32 togo_ReadFromDisk = 0;
			if (m_abyfiledata != NULL) {
				//A block was succefully read from disk, performe data transfer new var to let a possible read of other remaining block
				filedata_ReadFromDisk = m_abyfiledata;
				m_abyfiledata = NULL;
				togo_ReadFromDisk = m_utogo;
				if (pos) //if we have more than one block in queue get the second one
					currentBlock = m_BlockRequests_queue.GetAt(pos);
			}
			//Xman Full Chunk
			/*
			if (filedata_ReadFromDisk == NULL || m_BlockRequests_queue.GetCount()>1) {
			*/
			// We don't read new data if we are about to end the upload
			if (!upendsoon && (filedata_ReadFromDisk == NULL || m_BlockRequests_queue.GetCount()>1)) {
			//Xman end
				CKnownFile* srcFile = theApp.sharedfiles->GetFileByID(currentBlock->FileID);
				if (!srcFile)
					throw GetResString(IDS_ERR_REQ_FNF);
				uint64 i64uTogo;
				if (currentBlock->StartOffset > currentBlock->EndOffset){
					i64uTogo = currentBlock->EndOffset + (srcFile->GetFileSize() - currentBlock->StartOffset);
				}
				else{
					i64uTogo = currentBlock->EndOffset - currentBlock->StartOffset;
					//MORPH START - Changed by SiRoB, SLUGFILLER: SafeHash
					/*
					if (srcFile->IsPartFile() && !((CPartFile*)srcFile)->IsComplete(currentBlock->StartOffset,currentBlock->EndOffset-1, true))
					*/
					if (srcFile->IsPartFile() && !((CPartFile*)srcFile)->IsRangeShareable(currentBlock->StartOffset,currentBlock->EndOffset-1))	// SLUGFILLER: SafeHash - final safety precaution
					//MORPH END  - Changed by SiRoB, SLUGFILLER: SafeHash
					{
						CString error;
						error.Format(_T("%s: %I64u = %I64u - %I64u "), GetResString(IDS_ERR_INCOMPLETEBLOCK), i64uTogo, currentBlock->EndOffset, currentBlock->StartOffset);
						throw error;
					}
					// ==> Anti Anti HideOS & SOTN [SiRoB] - Stulle
					if (m_abyUpPartStatus) {
						for (UINT i = (UINT)(currentBlock->StartOffset/PARTSIZE); i < (UINT)((currentBlock->EndOffset-1)/PARTSIZE+1); i++)
						if (m_abyUpPartStatus[i]>SC_AVAILABLE)
							{
								CString error;
									error.Format(_T("%s: Part %u, %I64u = %I64u - %I64u "), GetResString(IDS_ERR_HIDDENBLOCK), i, i64uTogo, currentBlock->EndOffset, currentBlock->StartOffset);
								throw error;
							}
					} else {
						CString	error;
						error.Format(_T("%s: Part %u, %I64u = %I64u - %I64u "), GetResString(IDS_ERR_HIDDENSOURCE), (UINT)(currentBlock->StartOffset/PARTSIZE), i64uTogo, currentBlock->EndOffset, currentBlock->StartOffset);
						throw error;
					
					}
					// <== Anti Anti HideOS & SOTN [SiRoB] - Stulle
				}

				if( i64uTogo > EMBLOCKSIZE*3 )
					throw GetResString(IDS_ERR_LARGEREQBLOCK);
				uint32 togo = (uint32)i64uTogo;

				CSyncObject* lockhandle = NULL;  
				CString fullname;
				if (srcFile->IsPartFile() && ((CPartFile*)srcFile)->GetStatus() != PS_COMPLETE){
					// Do not access a part file, if it is currently moved into the incoming directory.
					// Because the moving of part file into the incoming directory may take a noticable 
					// amount of time, we can not wait for 'm_FileCompleteMutex' and block the main thread.
					/*if (!((CPartFile*)srcfile)->m_FileCompleteMutex.Lock(0)){ // just do a quick test of the mutex's state and return if it's locked.
						return;
					}
					((CPartFile*)srcFile)->m_FileCompleteMutex.Lock();
					lockFile.m_pObject = &((CPartFile*)srcFile)->m_FileCompleteMutex;
					*/
					lockhandle = &((CPartFile*)srcFile)->m_FileCompleteMutex;
					// If it's a part file which we are uploading the file remains locked until we've read the
					// current block. This way the file completion thread can not (try to) "move" the file into
					// the incoming directory.

					fullname = RemoveFileExtension(((CPartFile*)srcFile)->GetFullName());
				}
				else{
					fullname.Format(_T("%s\\%s"),srcFile->GetPath(),srcFile->GetFileName());
				}

				if (m_readblockthread == NULL) {
					m_readblockthread = (CReadBlockFromFileThread*) AfxBeginThread(RUNTIME_CLASS(CReadBlockFromFileThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
					m_readblockthread->SetReadBlockFromFile(fullname, currentBlock->StartOffset, togo, this, lockhandle);
					m_readblockthread->ResumeThread();
				} else {
					m_readblockthread->SetReadBlockFromFile(fullname, currentBlock->StartOffset, togo, this, lockhandle);
				}
				SetUploadFileID(srcFile); //MORPH - Moved by SiRoB, Fix Filtered Block Request
				m_abyfiledata = (byte*)-2;
				m_utogo = togo;
			}

			if (filedata_ReadFromDisk) {
				if (!srcfile_ReadFromDisk->IsPartFile())
					bFromPF = false; // This is not a part file.

				//Xman - Removed by SiRoB, Fix Filtered Block Request
				/*
				SetUploadFileID(srcfile);
				*/

				// check extension to decide whether to compress or not
				//Xman Code Improvement for choosing to use compression
				// Decide whether to compress the packets or not
				bool compFlag = (m_byDataCompVer == 1) && (IsUploadingToPeerCache() == false);

				//Xman disable compression
				if(thePrefs.m_bUseCompression==false)
					compFlag=false;
				//Xman end

				if(compFlag == true)
				{
					if(srcfile_ReadFromDisk->IsCompressible()==false)
						compFlag=false;
				}

				if (compFlag == true)
					CreatePackedPackets(filedata_ReadFromDisk,togo_ReadFromDisk,currentblock_ReadFromDisk,bFromPF);
				else
					CreateStandartPackets(filedata_ReadFromDisk,togo_ReadFromDisk,currentblock_ReadFromDisk,bFromPF);
				//Xman end Code Improvement for choosing to use compression

				//Xman Xtreme Upload 
				if(GetFileUploadSocket() && GetFileUploadSocket()->isready==false)
				{
					GetFileUploadSocket()->isready=true;
					theApp.uploadBandwidthThrottler->SetNoNeedSlot();
				}

				// file statistic
				srcfile_ReadFromDisk->statistic.AddTransferred(currentblock_ReadFromDisk->StartOffset, togo_ReadFromDisk); //Xman PowerRelease

				m_addedPayloadQueueSession += togo_ReadFromDisk;

				m_DoneBlocks_list.AddHead(m_BlockRequests_queue.RemoveHead());

				// Maella -One-queue-per-file- (idea bloodymad)
				srcfile_ReadFromDisk->UpdateStartUploadTime();
				// Maella end

				delete[] filedata_ReadFromDisk;
				filedata_ReadFromDisk = NULL;

				//Xman Full Chunk
				if(upendsoon)
					break;
				//Xman end
			}
			//now process error from nextblock to read
		}
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), GetUserName(), error);
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Client error: ") + error, CUpDownClient::USR_EXCEPTION); // Maella -Upload Stop Reason-
		if (m_abyfiledata != (byte*)-2 && m_abyfiledata != (byte*)-1 && m_abyfiledata != NULL) {
			delete[] m_abyfiledata;
			m_abyfiledata = NULL;
		}
		return;
	}
	catch(CFileException* e)
	{
		//Xman Reload shared files on filenotfound exception
		if( e->m_cause == CFileException::fileNotFound )
			if(!theApp.sharedfiles->IsUnsharedFile(m_BlockRequests_queue.GetHead()->FileID)) //zz_fly :: Fixes :: DolphinX :: don't reload sharedfiles when we need not
				theApp.sharedfiles->Reload();
		//Xman end
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("Failed to create upload package for %s - %s"), GetUserName(), szError);
		theApp.uploadqueue->RemoveFromUploadQueue(this, ((CString)_T("Failed to create upload package.")) + szError, CUpDownClient::USR_EXCEPTION); // Maella -Upload Stop Reason-
		if (m_abyfiledata != (byte*)-2 && m_abyfiledata != (byte*)-1 && m_abyfiledata != NULL) {
			delete[] m_abyfiledata;
			m_abyfiledata = NULL;
		}
		e->Delete();
		return;
	}
}
// END SiRoB: ReadBlockFromFileThread
//Xman end

//Xman better passive source finding
/*
bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile)
*/
bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile, bool isUDP)
//Xman end
{

	//Xman client percentage
	sint32 hisfinishedparts=-1;
	hiscompletedparts_percent_up=-1;
	//Xman end

	delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;
	if (GetExtendedRequestsVersion() == 0)
		return true;

	//Xman better passive source finding
	bool bPartsNeeded=false;
	bool shouldbechecked=isUDP && tempreqfile->IsPartFile() 
		&& (((CPartFile*)tempreqfile)->GetStatus()==PS_EMPTY || ((CPartFile*)tempreqfile)->GetStatus()==PS_READY) 
		&& !(GetDownloadState()==DS_ONQUEUE && reqfile==tempreqfile) 
		&& (droptime + HR2MS(3) < ::GetTickCount());
	//Xman end

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
		//Xman client percentage
		hisfinishedparts=0;
		//Xman end

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
				//Xman better passive source finding
				if (shouldbechecked && bPartsNeeded==false && m_abyUpPartStatus[done] && !((CPartFile*)tempreqfile)->IsComplete((uint64)done*PARTSIZE,((uint64)(done+1)*PARTSIZE)-1,false))
					bPartsNeeded = true;
				//Xman end

				//Xman client percentage
				if(m_abyUpPartStatus[done])
					hisfinishedparts++;
				//Xman end

				done++;
				if (done == m_nUpPartCount)
					break;
			}
		}
	}
	if (GetExtendedRequestsVersion() > 1)
	{
		uint16 nCompleteCountLast = GetUpCompleteSourcesCount();
		uint16 nCompleteCountNew = data->ReadUInt16();
		SetUpCompleteSourcesCount(nCompleteCountNew);
		if (nCompleteCountLast != nCompleteCountNew)
			tempreqfile->UpdatePartsInfo();
	}
	theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(this);

	//Xman client percentage
	if(hisfinishedparts>=0)
		hiscompletedparts_percent_up= (sint8)((float)hisfinishedparts/tempreqfile->GetPartCount()*100.0f);	
	else
		hiscompletedparts_percent_up= -1;
	//Xman end

	theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(this);

	
	//Xman
	// Maella -Code Improvement-
	if(tempreqfile->IsPartFile() == true && m_nUpPartCount != 0){
		// Check if a source has now chunk that we can need
		POSITION pos = m_OtherNoNeeded_list.Find(tempreqfile);
		if(pos != NULL){
			for(uint16 i = 0; i < m_nUpPartCount; i++){ 
				if(m_abyUpPartStatus[i] != 0){ 
					const uint64 uStart = PARTSIZE*(uint64)i;
					const uint64 uEnd = ((uint64)tempreqfile->GetFileSize()-1 <= (uStart+PARTSIZE-1)) ? ((uint64)tempreqfile->GetFileSize()-1) : (uStart+PARTSIZE-1);
					if(((CPartFile*)tempreqfile)->IsComplete(uStart, uEnd, false) == false){
						// Swap source to the other list
						m_OtherNoNeeded_list.RemoveAt(pos);
						m_OtherRequests_list.AddHead((CPartFile*)tempreqfile);

						if (thePrefs.GetVerbose())
							AddDebugLogLine(false, _T("->a source has now parts available. %s, file: %s"), DbgGetClientInfo(), tempreqfile->GetFileName());
						//break; 
						return true; //we are ready here
					}
				}
			}
		}	
	}
	// Maella end
	
	//Xman better passive source finding
	//problem is: if a client just began to download a file, we receive an FNF
	//later, if it has some chunks we don't find it via passive source finding because 
	//that works only on TCP-reask but not via UDP
	if(bPartsNeeded )
	{
		//the client was a NNS but isn't any more
		if(GetDownloadState()==DS_NONEEDEDPARTS && reqfile==tempreqfile)
			TrigNextSafeAskForDownload(reqfile);
		else if(GetDownloadState()!=DS_ONQUEUE)
		{
			//the client maybe isn't in our downloadqueue.. let's look if we should add the client
			if((credits && credits->GetMyScoreRatio(GetIP())>=1.8f && ((CPartFile*)tempreqfile)->GetSourceCount() < ((CPartFile*)tempreqfile)->GetMaxSources())
				|| ((CPartFile*)tempreqfile)->GetSourceCount() < ((CPartFile*)tempreqfile)->GetMaxSources()*0.8f + 1)
			{
				if(((CPartFile*)tempreqfile)->IsGlobalSourceAddAllowed()) //Xman GlobalMaxHarlimit for fairness
					if(theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)tempreqfile,this, true))
						AddDebugLogLine(false, _T("->found new source on reask-ping: %s, file: %s"), DbgGetClientInfo(), tempreqfile->GetFileName());
			}
		}
		else
		{
			if (AddRequestForAnotherFile((CPartFile*)tempreqfile))
			{
				theApp.emuledlg->transferwnd->GetDownloadList()->AddSource((CPartFile*)tempreqfile,this,true);
				AddDebugLogLine(false, _T("->found new A4AF source on reask-ping: %s, file: %s"), DbgGetClientInfo(), tempreqfile->GetFileName());
			}
		}
	}
	//Xman end
	return true;
}

void CUpDownClient::CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	uint32 nPacketSize;
	CMemFile memfile((BYTE*)data,togo);
	
	//Xman flexible splittingsize
	/*
	if (togo > 10240) 
		nPacketSize = togo/(uint32)(togo/10240);
	*/
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
	uint32 splittingsize = max(m_nUpDatarate10, 10240);
	if (togo > splittingsize)
		nPacketSize = togo/(uint32)(togo/splittingsize);
#else
	uint32 splittingsize = 10240;
	if( m_nUpDatarate10 > 5120 && !IsUploadingToPeerCache() && GetDownloadState()!=DS_DOWNLOADING)
	{
		splittingsize = m_nUpDatarate10 << 1;
		if (splittingsize > 36000)
			splittingsize = 36000;
	}
	
	if (togo > splittingsize) 
		nPacketSize = togo/(uint32)(togo/splittingsize);
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
	//Xman end
	else
		nPacketSize = togo;
	// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
	uint32 npacket = 0;
	uint32 Size = togo;
	//Packet* apacket[EMBLOCKSIZE*3/10240];
	Packet** apacket = new Packet*[togo/nPacketSize];
#endif
	// <== Send Array Packet [SiRoB] - Mephisto
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
			// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
			apacket[npacket++] = packet;
#else
			m_pPCUpSocket->SendPacket(packet, true, false, nPacketSize);
#endif
			// <== Send Array Packet [SiRoB] - Mephisto
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
			
			// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
			apacket[npacket++] = packet;
#else
			socket->SendPacket(packet,true,false, nPacketSize);
#endif
			// <== Send Array Packet [SiRoB] - Mephisto
		}
	}
	// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
	if (npacket) {
		if (IsUploadingToPeerCache())
			m_pPCUpSocket->SendPacket(apacket, npacket, true, false, Size);
		else
			socket->SendPacket(apacket, npacket, true, false, Size);
		delete[] apacket;
	}
#endif
	// <== Send Array Packet [SiRoB] - Mephisto
}

void CUpDownClient::CreatePackedPackets(byte* data, uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	BYTE* output = new BYTE[togo+300];
	uLongf newsize = togo+300;
	//Xman used different values!
	// BEGIN netfinity: Variable compression - Reduce CPU usage for high bandwidth connections
	//  Preferably this should take CPU speed into account
	/*
	UINT result = compress2(output, &newsize, data, togo, 9);
	*/
	// ==> Adjust Compress Level [Stulle] - Stulle
	/*
	int	compressLevel = 9;
	if (thePrefs.GetMaxUpload() > 500.0f)
		compressLevel = 1;
	else if (thePrefs.GetMaxUpload() > 200.0f)
		compressLevel = 3;
	else if (thePrefs.GetMaxUpload() > 80.0f)
		compressLevel = 6;
	UINT result = compress2(output, &newsize, data, togo, compressLevel);
	// END netfinity: Variable compression
	if (result != Z_OK || togo <= newsize){
		delete[] output;
		CreateStandartPackets(data,togo,currentblock,bFromPF);
		return;
	}
	*/
	int compressLevel = thePrefs.GetCompressLevel();
	if(compressLevel != 0)
	{
		int compressLevelTemp = 9;
		if (thePrefs.GetMaxUpload() > 500.0f)
			compressLevelTemp = 1;
		else if (thePrefs.GetMaxUpload() > 200.0f)
			compressLevelTemp = 3;
		else if (thePrefs.GetMaxUpload() > 80.0f)
			compressLevelTemp = 6;

		if(compressLevelTemp < compressLevel)
			compressLevel = compressLevelTemp;

		UINT result = compress2(output, &newsize, data, togo, compressLevel);
		if (result != Z_OK || togo <= newsize){
			delete[] output;
			CreateStandartPackets(data,togo,currentblock,bFromPF);
			return;
		}
	}
	else{
		delete[] output;
		CreateStandartPackets(data,togo,currentblock,bFromPF);
		return;
	}
	// <== Adjust Compress Level [Stulle] - Stulle
	CMemFile memfile(output,newsize);
    uint32 oldSize = togo;
	togo = newsize;
	uint32 nPacketSize;

	//Xman flexible splittingsize
	/*
    if (togo > 10240) 
        nPacketSize = togo/(uint32)(togo/10240);
	*/
	// ==> Dynamic Socket Buffering [SiRoB] - Mephisto
#ifndef DONT_USE_SOCKET_BUFFERING
	uint32 splittingsize = max(m_nUpDatarate10, 10240);
	if (togo > splittingsize)
		nPacketSize = togo/(uint32)(togo/splittingsize);
#else
	uint32 splittingsize = 10240;
	if( m_nUpDatarate10 > 5120 && GetDownloadState()!=DS_DOWNLOADING)
	{
		splittingsize = m_nUpDatarate10 << 1; //one packet can be send between 2 - 4 seconds
		if (splittingsize > 36000)
			splittingsize = 36000;
	}

	if (togo > splittingsize) 
		nPacketSize = togo/(uint32)(togo/splittingsize);
#endif
	// <== Dynamic Socket Buffering [SiRoB] - Mephisto
	//Xman end
    else
        nPacketSize = togo;
    
	// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
	uint32 npacket = 0;
	//Packet* apacket[EMBLOCKSIZE*3/10240];
	Packet** apacket = new Packet*[togo/nPacketSize];
#else
    uint32 totalPayloadSize = 0;
#endif
	// <== Send Array Packet [SiRoB] - Mephisto

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
			theStats.AddUpDataOverheadFileRequest(28); //Xman fix
		}
		else{
			packet = new Packet(OP_COMPRESSEDPART,nPacketSize+24,OP_EMULEPROT,bFromPF);
			md4cpy(&packet->pBuffer[0],GetUploadFileID());
			PokeUInt32(&packet->pBuffer[16], (uint32)statpos);
			PokeUInt32(&packet->pBuffer[20], newsize);
			memfile.Read(&packet->pBuffer[24],nPacketSize);
			theStats.AddUpDataOverheadFileRequest(24); //Xman fix
		}

		if (thePrefs.GetDebugClientTCPLevel() > 0){
			DebugSend("OP__CompressedPart", this, GetUploadFileID());
			Debug(_T("  Start=%I64u  BlockSize=%u  Size=%u\n"), statpos, newsize, nPacketSize);
		}
		// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
		apacket[npacket++] = packet;
#else
        // approximate payload size
        uint32 payloadSize = nPacketSize*oldSize/newsize;

        if(togo == 0 && totalPayloadSize+payloadSize < oldSize) {
            payloadSize = oldSize-totalPayloadSize;
        }
        totalPayloadSize += payloadSize;

        // put packet directly on socket
		//Xman fix: we have different sizes , moved up
		/*
		theStats.AddUpDataOverheadFileRequest(24);
		*/
		//Xman end
        socket->SendPacket(packet,true,false, payloadSize);
#endif
		// <== Send Array Packet [SiRoB] - Mephisto
	}
	// ==> Send Array Packet [SiRoB] - Mephisto
#ifndef DONT_USE_SEND_ARRAY_PACKET
	if (npacket) {
		socket->SendPacket(apacket, npacket, true, false, oldSize);
		delete[] apacket;
	}
#endif
	// <== Send Array Packet [SiRoB] - Mephisto
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
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;
	m_nSelectedChunk = 0;	// HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

	if (newreqfile)
	{
		newreqfile->AddUploadingClient(this);
		md4cpy(requpfileid, newreqfile->GetFileHash());
	}
	else
		md4clr(requpfileid);

	if (oldreqfile)
	{ //Xman
		oldreqfile->RemoveUploadingClient(this);
	//Xman - Added by SiRoB, Fix Filtered Block Request
		ClearUploadBlockRequests(false);
	}
	//Xman end
	
}

//Xman see OnUploadqueue
void CUpDownClient::SetOldUploadFileID(){
	if( requpfileid )
		md4cpy(oldfileid,requpfileid);
}
//Xman end

void CUpDownClient::AddReqBlock(Requested_Block_Struct* reqblock)
{
    if(GetUploadState() != US_UPLOADING) {
        //Xman Full Chunk:
	//a normal behavior don't need a log message
	/*
        if(thePrefs.GetLogUlDlEvents())
            AddDebugLogLine(DLP_LOW, false, _T("UploadClient: Client tried to add req block when not in upload slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
	*/
	//Xman end
		delete reqblock;
        return;
    }

	if(HasCollectionUploadSlot()){
		CKnownFile* pDownloadingFile = theApp.sharedfiles->GetFileByID(reqblock->FileID);
		if(pDownloadingFile != NULL){
			//Xman Code Improvement for HasCollectionExtention
			/*
			if ( !(CCollection::HasCollectionExtention(pDownloadingFile->GetFileName()) && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) ){
			*/
			if ( !(pDownloadingFile->HasCollectionExtenesion_Xtreme() /*CCollection::HasCollectionExtention(pDownloadingFile->GetFileName())*/ && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) )
			{
			//Xman end
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
		//zz_fly :: Don't transmit for nested/overlapping data requests :: emuleplus :: start
		/*
        if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset){
		*/
		//zz_fly :: Don't transmit for nested/overlapping data requests :: emuleplus :: end
		if (reqblock->StartOffset >= cur_reqblock->StartOffset && reqblock->EndOffset <= cur_reqblock->EndOffset){
            delete reqblock;
            return;
        }
    }
    for (POSITION pos = m_BlockRequests_queue.GetHeadPosition(); pos != 0; ){
        const Requested_Block_Struct* cur_reqblock = m_BlockRequests_queue.GetNext(pos);
		//zz_fly :: Don't transmit for nested/overlapping data requests :: emuleplus :: start
		/*
        if (reqblock->StartOffset == cur_reqblock->StartOffset && reqblock->EndOffset == cur_reqblock->EndOffset){
		*/
		//zz_fly :: Don't transmit for nested/overlapping data requests :: emuleplus :: end
		if (reqblock->StartOffset >= cur_reqblock->StartOffset && reqblock->EndOffset <= cur_reqblock->EndOffset){
            delete reqblock;
            return;
        }
    }

    m_BlockRequests_queue.AddTail(reqblock);
}

uint32 CUpDownClient::SendBlockData(){
	//Xman 
	/*
    DWORD curTick = ::GetTickCount();
	*/
	//Xman end

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
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, (UINT)sentBytesCompleteFile, (IsFriend() && GetFriendSlot()));
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, (UINT)sentBytesPartFile, (IsFriend() && GetFriendSlot()));

		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		/*
		m_nTransferredUp = (UINT)(m_nTransferredUp + sentBytesCompleteFile + sentBytesPartFile);
        credits->AddUploaded((UINT)(sentBytesCompleteFile + sentBytesPartFile), GetIP());
		*/
		AddUploadRate((UINT)(sentBytesCompleteFile + sentBytesPartFile));
		//Xman end

        sentBytesPayload = s->GetSentPayloadSinceLastCallAndReset();
        m_nCurQueueSessionPayloadUp = (UINT)(m_nCurQueueSessionPayloadUp + sentBytesPayload);

		//Xman Full Chunk
		//in CreateNextBlockPackage we saw this upload end soon,
		//after all packets are send, we cancel this upload
		/*
        if (theApp.uploadqueue->CheckForTimeOver(this)) {
            theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Completed transfer"), true);
			SendOutOfPartReqsAndAddToWaitingQueue();
		*/
		if (upendsoon && s->StandardPacketQueueIsEmpty()) {
			credits->InitPayBackFirstStatus(); // Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

			// ==> Spread Credits Slot [Stulle] - Stulle
			if(GetSpreadClient()>0)
				SetSpreadClient(0);
			// <== Spread Credits Slot [Stulle] - Stulle

			theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Completed transfer"),CUpDownClient::USR_COMPLETEDRANSFER ,true ); // Maella -Upload Stop Reason-
			SendOutOfPartReqsAndAddToWaitingQueue(thePrefs.TransferFullChunks() ? true:false); //Xman Full Chunk
		//Xman end
        } 
		else {
            if(upendsoon==false) //Xman Full Chunk
            // read blocks from file and put on socket
            CreateNextBlockPackage();
        }
    }

	//Xman
	/*
    if(sentBytesCompleteFile + sentBytesPartFile > 0 ||
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
    if(m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 0 && GetUpStartTimeDelay() > 2*1000) {
        m_nUpDatarate = (UINT)(((ULONGLONG)m_nSumForAvgUpDataRate*1000) / (curTick - m_AvarageUDR_list.GetHead().timestamp));
    } else {
        // not enough values to calculate trustworthy speed. Use -1 to tell this
        m_nUpDatarate = 0; //-1;
    }

    // Check if it's time to update the display.
    if (curTick-m_lastRefreshedULDisplay > MINWAIT_BEFORE_ULDISPLAY_WINDOWUPDATE+(uint32)(rand()*800/RAND_MAX)) {
        // Update display
        theApp.emuledlg->transferwnd->GetUploadList()->RefreshClient(this);
        theApp.emuledlg->transferwnd->GetClientList()->RefreshClient(this);
        m_lastRefreshedULDisplay = curTick;
    }
	*/
	//Xman end

    return (UINT)(sentBytesCompleteFile + sentBytesPartFile);
}

//Xtreme Full Chunk
/*
void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue()
*/
// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
/*
void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue(bool givebonus)
*/
void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue(bool /*givebonus*/)
// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
//Xman end
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
	SendPacket(pPacket, true);
	m_fSentOutOfPartReqs = 1;
    
	//Xtreme Full Chunk
	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	/*
	//if we gave less than 9 MB payload, we give back some waiting time in relation to payload
	uint32 bonus=0;
	uint32 waitingtime= (uint32)(GetWaitTime() ); //only half will be counted 
	if(givebonus)
	{
		//calculate the time
		if(GetQueueSessionPayloadUp() < 9*1024*1024)
		{
			bonus = (uint32)(((PARTSIZE) - GetQueueSessionPayloadUp()) / (double)(PARTSIZE) * (waitingtime/2));
		}
	}
	*/
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	//Xman end

    theApp.uploadqueue->AddClientToQueue(this, true);

	//Xtreme Full Chunk
	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	/*
	if(bonus>0)
	{
		if(credits)
		{
			credits->SetWaitStartTimeBonus(GetIP(),::GetTickCount()-bonus);
			AddDebugLogLine(false, _T("giving client bonus. old waitingtime: %s, new waitingtime: %s, client: %s"), CastSecondsToHM(waitingtime/1000), CastSecondsToHM((::GetTickCount() - GetWaitStartTime())/1000),DbgGetClientInfo()); 
		}
	}
	*/
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	//Xman end
}

/**
 * See description for CEMSocket::TruncateQueues().
 */
void CUpDownClient::FlushSendBlocks(){ // call this when you stop upload, or the socket might be not able to send
    if (socket)      //socket may be NULL...
        socket->TruncateQueues();

	//Xman Code Fix
	if(m_pPCUpSocket)
		m_pPCUpSocket->TruncateQueues();
	//Xman end
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
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__HashSetAnswer", this, file->GetFileIdentifier().GetMD4Hash());
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
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__HashSetAnswer", this, pData);
		packet = new Packet(&fileResponse, OP_EDONKEYPROT, OP_HASHSETANSWER);
	}
	theStats.AddUpDataOverheadFileRequest(packet->size);
	SendPacket(packet, true);
}

//Xman - Fix Filtered Block Request
/*
void CUpDownClient::ClearUploadBlockRequests()
*/
void CUpDownClient::ClearUploadBlockRequests(bool truncatequeues)
//Xman end
{
	//Xman - Fix Filtered Block Request
	if(truncatequeues)
	//Xman end
		FlushSendBlocks();

	for (POSITION pos = m_BlockRequests_queue.GetHeadPosition();pos != 0;)
		delete m_BlockRequests_queue.GetNext(pos);
	m_BlockRequests_queue.RemoveAll();
	
	for (POSITION pos = m_DoneBlocks_list.GetHeadPosition();pos != 0;)
		delete m_DoneBlocks_list.GetNext(pos);
	m_DoneBlocks_list.RemoveAll();

	//Xman
	// BEGIN SiRoB: ReadBlockFromFileThread
	if (m_abyfiledata != (byte*)-1 && m_abyfiledata != (byte*)-2 && m_abyfiledata != NULL) {
		delete[] m_abyfiledata;
		m_abyfiledata = NULL;
	}
	// END SiRoB: ReadBlockFromFileThread
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
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__FileDesc", this, file->GetFileHash());
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
			//zz_fly :: fix possible overflow :: start
			//note: in some special case, ::GetTickCount() may lesser than cur_struct->lastasked
			/*
			if (::GetTickCount() - cur_struct->lastasked < MIN_REQUESTTIME && !GetFriendSlot()){ 
			*/
			uint32 DeltaTime = 30000 + ::GetTickCount() - cur_struct->lastasked;
			if (DeltaTime < (MIN_REQUESTTIME + 30000) && !GetFriendSlot()){ 
			//zz_fly :: end
				if (GetDownloadState() != DS_DOWNLOADING)
					cur_struct->badrequests++;
				if (cur_struct->badrequests == BADCLIENTBAN){
					Ban(_T("Request too fast"));
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

	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	if(theApp.clientcredits->IsSaveUploadQueueWaitTime()) {
		ClearWaitStartTime();
		if (credits != NULL){
			credits->ClearUploadQueueWaitTime();
		}
	}
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle

	if (!IsBanned()){
		//Xman
		/*
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false,_T("Banned: %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
		*/
		AddLeecherLogLine(false,_T("Banned: %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
		//Xman end
	}
#ifdef _DEBUG
	else{
		if (thePrefs.GetLogBannedClients())
		//Xman
		/*
			AddDebugLogLine(false,_T("Banned: (refreshed): %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
		*/
		AddLeecherLogLine(false,_T("Banned: (refreshed): %s; %s"), pszReason==NULL ? _T("Aggressive behaviour") : pszReason, DbgGetClientInfo());
		//Xman end
	}
#endif
	//Xman
	/*
	theApp.clientlist->AddBannedClient(GetIP());
	*/
	theApp.clientlist->AddBannedClient(GetConnectIP());
	//Xman end
	SetUploadState(US_BANNED);
	theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(this);
	if (socket != NULL && socket->IsConnected())
		socket->ShutDown(SD_RECEIVE); // let the socket timeout, since we dont want to risk to delete the client right now. This isnt acutally perfect, could be changed later
}

// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
// Moonlight: SUQWT - Compare linear time instead of time indexes to avoid overflow-induced false positives.
/*
uint32 CUpDownClient::GetWaitStartTime() const
*/
sint64 CUpDownClient::GetWaitStartTime() const
// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
{
	if (credits == NULL){
		ASSERT ( false );
		return 0;
	}
	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	/*
	uint32 dwResult = credits->GetSecureWaitStartTime(GetIP());
	if (dwResult > m_dwUploadTime && IsDownloading()){
		//this happens only if two clients with invalid securehash are in the queue - if at all
		dwResult = m_dwUploadTime-1;
	*/
	sint64 dwResult = credits->GetSecureWaitStartTime(GetIP());
	uint32 now = ::GetTickCount();
	if ( dwResult > now) { 
		dwResult = now - 1;
	}

	if (IsDownloading() && (dwResult > m_dwUploadTime)) {
	//this happens only if two clients with invalid securehash are in the queue - if at all
			dwResult = m_dwUploadTime-1;
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle

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

//Xman Xtreme Upload: Peercache-part
/*
CEMSocket* CUpDownClient::GetFileUploadSocket(bool bLog)
{
    if (m_pPCUpSocket && (IsUploadingToPeerCache() || m_ePeerCacheUpState == PCUS_WAIT_CACHE_REPLY))
*/
CClientReqSocket* CUpDownClient::GetFileUploadSocket(bool bLog) const
{
    if (m_pPCUpSocket && IsUploadingToPeerCache())
//Xman end
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

//Xman Full Chunk
// Checks if it is next requested block from another chunk of the actual file or from another file 
// 
// [Returns] 
//   true : Next requested block is from another different chunk or file than last downloaded block 
//   false: Next requested block is from same chunk that last downloaded block 
// ==> Mephisto Upload - Mephisto
/*
bool CUpDownClient::IsDifferentPartBlock()
*/
bool CUpDownClient::IsDifferentPartBlock(bool bCheckForMulti)
// <== Mephisto Upload - Mephisto
{ 
	Requested_Block_Struct* lastBlock;
	Requested_Block_Struct* currBlock;
	uint32 lastDone = 0;
	uint32 currRequested = 0;
	
	bool different = false;
	
	//try {
		// Check if we have good lists and proceed to check for different chunks
		// ==> Mephisto Upload - Mephisto
		/*
		if (GetSessionUp() >= 3145728 //Xman-Full-Chunk: Client is allowed to get min 3.0 MB
		*/
		if ((GetSessionUp() >= 3145728 || bCheckForMulti)
		// <== Mephisto Upload - Mephisto
			&& !m_BlockRequests_queue.IsEmpty() && !m_DoneBlocks_list.IsEmpty())
		{
			// Calculate corresponding parts to blocks
			//lastBlock = m_DoneBlocks_list.GetTail(); //Xman: with this method we give 1 chunk min and 2.8MB max if chunk border was reached
			lastBlock = m_DoneBlocks_list.GetHead();
			lastDone = (uint32)(lastBlock->StartOffset / PARTSIZE);
			currBlock = m_BlockRequests_queue.GetHead(); 
			currRequested = (uint32)(currBlock->StartOffset / PARTSIZE); 
             
			// Test is we are asking same file and same part
			//
			// ==> Mephisto Upload - Mephisto
			if (!bCheckForMulti)
			{
			// <== Mephisto Upload - Mephisto
				if ( lastDone != currRequested )  
				{ 
					different = true;
					
					if(thePrefs.GetLogUlDlEvents()){
						AddDebugLogLine(false, _T("%s: Upload session will end soon due to new chunk."), this->GetUserName());
					}				
				}
				if (md4cmp(lastBlock->FileID, currBlock->FileID) != 0 ) 
				{ 
					different = true;
					
					if(thePrefs.GetLogUlDlEvents()){
						AddDebugLogLine(false, _T("%s: Upload session will end soon due to different file."), this->GetUserName());
					}
				}
			// ==> Mephisto Upload - Mephisto
			}
			else
			{
				if (lastDone != currRequested || md4cmp(lastBlock->FileID, currBlock->FileID) != 0) 
					different = true;
			}
			// <== Mephisto Upload - Mephisto
		} 
   /*
	}
   	catch(...)
   	{ 
			AddDebugLogLine(false, _T("%s: Upload session ended due to error."), this->GetUserName());
      		different = true; 
   	} 
	*/

	return different; 
}


void CUpDownClient::CompUploadRate(){
	// Add new sample
	TransferredData newSample;
	newSample.dataLength = m_nUpDatarateMeasure;
	newSample.timeStamp  = ::GetTickCount();
	m_upHistory_list.AddHead(newSample);

	// Keep up to 21 samples (=> 20 seconds)
	while(m_upHistory_list.GetSize() > 21){
		m_upHistory_list.RemoveTail();
	}

	if(m_upHistory_list.GetSize() > 1){	
		// Compute datarate (=> display)
		POSITION pos = m_upHistory_list.FindIndex(thePrefs.GetDatarateSamples());
		if(pos == NULL){
			pos = m_upHistory_list.GetTailPosition();
		}
		TransferredData& oldestSample = m_upHistory_list.GetAt(pos);
		uint32 deltaTime = newSample.timeStamp - oldestSample.timeStamp;
		UINT deltaByte = newSample.dataLength - oldestSample.dataLength;
		m_nUpDatarate = (deltaTime > 0) ? (UINT)(1000.0 * deltaByte / deltaTime) : 0;   // [bytes/s]
	}

	if(m_upHistory_list.GetSize() > 3){	
		// Compute datarate (=> display)
		POSITION pos = m_upHistory_list.FindIndex(10);
		if(pos == NULL){
			pos = m_upHistory_list.GetTailPosition();
		}
		TransferredData& oldestSample = m_upHistory_list.GetAt(pos);
		uint32 deltaTime = newSample.timeStamp - oldestSample.timeStamp;
		UINT deltaByte = newSample.dataLength - oldestSample.dataLength;
		m_nUpDatarate10 = (deltaTime > 0) ? (UINT)(1000.0 * deltaByte / deltaTime) : 0;   // [bytes/s]
	}

	// Check and then refresh GUI
	m_displayUpDatarateCounter++;
	//Xman Code Improvement: slower refresh for clientlist
	if(m_displayUpDatarateCounter%DISPLAY_REFRESH == 0 ){
		theApp.emuledlg->transferwnd->GetUploadList()->RefreshClient(this);
	}
	if(m_displayUpDatarateCounter%DISPLAY_REFRESH_CLIENTLIST == 0 ){
		theApp.emuledlg->transferwnd->GetClientList()->RefreshClient(this);
		m_displayUpDatarateCounter = 0;
	}

}
// Maella end

// ==> Mephisto Upload - Mephisto
/*
bool CUpDownClient::CheckDatarate(uint8 dataratestocheck)
{
	//Xman Xtreme Upload
	//look if a slot is over the wanted speed (+ tolerance) 
	if(m_upHistory_list.GetSize() > 4 && dataratestocheck >= 2)
	{	
		// Compute datarate (=> display)
		POSITION pos = m_upHistory_list.FindIndex(dataratestocheck); //avg of 10 seconds
		if(pos == NULL){
			pos = m_upHistory_list.GetTailPosition();
		}
		TransferredData& oldestSample = m_upHistory_list.GetAt(pos);
		TransferredData& newSample = m_upHistory_list.GetHead();
		uint32 deltaTime = newSample.timeStamp - oldestSample.timeStamp;
		UINT deltaByte = newSample.dataLength - oldestSample.dataLength;
		uint32 proofUpDatarate = (deltaTime > 0) ? (UINT)(1000.0 * deltaByte / deltaTime) : 0;   // [bytes/s]
		//
		uint32 toleranceValue; 
		if(theApp.uploadqueue->currentuploadlistsize > (uint16)ceil(thePrefs.GetMaxUpload()/thePrefs.m_slotspeed) + 1 ) //we are 2 slots over MinSlots
			toleranceValue=(uint32)(thePrefs.m_slotspeed*1024*1.33f); //33%
		else
			toleranceValue=(uint32)(thePrefs.m_slotspeed*1024*1.25f); //25%
		if(GetFileUploadSocket())
		{
			if(GetFileUploadSocket()->IsFull() && proofUpDatarate > toleranceValue) 
			{
				return true;
			}
			else if(GetFileUploadSocket()->IsTrickle() 
				&& (proofUpDatarate > (thePrefs.m_slotspeed*1024/2) || proofUpDatarate > 2.5f*1024) 
				)
			{
				return true;
			}
		}
	}
	return false;
}
*/
// <== Mephisto Upload - Mephisto

void CUpDownClient::AddUploadRate(UINT size)
{
	m_nUpDatarateMeasure += size; 
	m_nTransferredUp += size;
	credits->AddUploaded(size, GetIP());
}
//Xman end

//Xman Anti-Leecher
void CUpDownClient::BanLeecher(LPCTSTR pszReason, uint8 leechercategory){
	//possible categories:
	//0 = no leecher
	//1 = bad hello + reduce score
	//2 = snafu
	//3 = ghost
	//4 = modstring soft
	//5 = modstring/username hard
	//6 = mod thief
	//7 = spammer
	//8 = XS-Exploiter
	//9 = other (fake emule version/ Credit Hack)
	//10 = username soft
	//11 = nick thief
	//12 = emcrypt
	//13 = bad hello + ban
	//14 = wrong HashSize + reduce score (=new united)
	//15 = snafu = m4 string
	//16 = wrong Startuploadrequest (bionic community)
	//17 = wrong m_fSupportsAICH (applejuice )
	//18 = detected by userhash (AJ) (ban)
	//19 = filefaker (in deadsourcelist but still requesting the file)
	//20 = Fincan Community

	m_strBanMessage.Empty();
	bool reducescore=false;

	// ==> Anti Uploader Ban [Stulle] - Stulle
	if (AntiUploaderBanActive())
	{
		m_bLeecher = 0; // no leecher
		return;
	}
	// <== Anti Uploader Ban [Stulle] - Stulle

	switch(leechercategory) 
	{
	case 1:
	case 4:
	case 10:
	case 14:
	case 15:
	case 17:
		reducescore=thePrefs.GetAntiLeecherCommunity_Action();
		break;
	case 12: //emcrypt
		reducescore=true;
		break;
	case 3:
		//Xman always ban ghost mods
		//reducescore=thePrefs.GetAntiLeecherGhost_Action();
		break;
	case 6:
	case 11:
		reducescore=thePrefs.GetAntiLeecherThief_Action();
		break;
	}

	if (m_bLeecher!=leechercategory){
		theStats.leecherclients++;
		m_bLeecher = leechercategory;
		strBanReason_permament=pszReason;

		if(reducescore)
		{
			m_strBanMessage.Format(_T("[%s](reduce score)- Client %s"),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
			//AddLeecherLogLine(false,_T("[%s](reduce score)- Client %s"),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
		}
		else
			m_strBanMessage.Format(_T("[%s](ban)- Client %s"),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
			//AddLeecherLogLine(false,_T("[%s](ban)- Client %s"),pszReason==NULL ? _T("No Reason") : pszReason, DbgGetClientInfo());
	}

	if(reducescore)
		return;

	// ==> SUQWT [Moonlight/EastShare/ MorphXT] - Stulle
	if(theApp.clientcredits->IsSaveUploadQueueWaitTime()) {
		ClearWaitStartTime();
		if (credits != NULL){
			credits->ClearUploadQueueWaitTime();
		}
	}
	// <== SUQWT [Moonlight/EastShare/ MorphXT] - Stulle

	SetChatState(MS_NONE);
	theApp.clientlist->AddTrackClient(this);
	theApp.clientlist->AddBannedClient( GetConnectIP() );
	SetUploadState(US_BANNED);
	theApp.emuledlg->transferwnd->ShowQueueCount(theApp.uploadqueue->GetWaitingUserCount());
	theApp.emuledlg->transferwnd->GetQueueList()->RefreshClient(this);
	if (socket != NULL && socket->IsConnected())
		socket->ShutDown(SD_RECEIVE); // let the socket timeout, since we dont want to risk to delete the client right now. This isnt acutally perfect, could be changed later
}
//Xman end


//Xman
//MORPH START - Changed by SiRoB, ReadBlockFromFileThread
IMPLEMENT_DYNCREATE(CReadBlockFromFileThread, CWinThread)

void CReadBlockFromFileThread::SetReadBlockFromFile(LPCTSTR filepath, uint64 startOffset, uint32 toread, CUpDownClient* client, CSyncObject* lockhandle) {
	fullname = filepath;
	StartOffset = startOffset;
	togo = toread;
	m_client = client;
	m_clientname = m_client->GetUserName(); //Fafner: avoid possible crash - 080421
	m_lockhandle = lockhandle;
	pauseEvent.SetEvent();
} 

void CReadBlockFromFileThread::StopReadBlock() {
	doRun = false;
	pauseEvent.SetEvent();
} 

int CReadBlockFromFileThread::Run() {
	DbgSetThreadName("CReadBlockFromFileThread");

	//InitThreadLocale(); //Performance killer
	// SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash

	CFile file;
	byte* filedata = NULL;
	//Xman queued disc-access for read/flushing-threads
	//bool hastoresumenextthread=true;
	//Xman end
	doRun = true;
	pauseEvent.Lock();
	while(doRun) {
		CSyncHelper lockFile;
		try{
			if (m_lockhandle) {
				lockFile.m_pObject = m_lockhandle;
				if (m_lockhandle->Lock(1000) == 0) { //Fafner: Lock() == Lock(INFINITE): waits forever - 080421
					CString str;
					str.Format(_T("file is locked: %s"), fullname);
					throw str;
				}
			}

			if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
				throw GetResString(IDS_ERR_OPEN);

			file.Seek(StartOffset,0);

			filedata = new byte[togo+500];
			//zz_fly :: bug fix :: DolphinX :: start
			/* maybe this is not a bug. this fix is more readable.
			if (uint32 done = file.Read(filedata,togo) != togo){
			*/
			uint32 done = file.Read(filedata, togo);
			if (done != togo){
			//zz_fly :: end
				file.SeekToBegin();
				file.Read(filedata + done, togo - done);
			}
			file.Close();

			if (lockFile.m_pObject){
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}

			if (theApp.emuledlg && theApp.emuledlg->IsRunning())
				PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE, (WPARAM)filedata,(LPARAM)m_client);
			else {
				delete[] filedata;
				filedata = NULL;
			}
		}
		catch(CString error)
		{
			if (lockFile.m_pObject) { //Fafner: missing? - 080421
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}
			if (thePrefs.GetVerbose())
				DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), m_client->GetUserName(), error);
			if (theApp.emuledlg && theApp.emuledlg->IsRunning())
				PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE,(WPARAM)-1,(LPARAM)m_client);
			else if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL){
				delete[] filedata;
				filedata = NULL;
			}
			return 1;
		}
		catch(CFileException* e)
		{
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			e->GetErrorMessage(szError, ARRSIZE(szError));
			if (lockFile.m_pObject) { //Fafner: missing? - 080421
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}
			if (thePrefs.GetVerbose())
				DebugLogWarning(_T("Failed to create upload package for %s - %s"), m_client->GetUserName(), szError);
			if (theApp.emuledlg && theApp.emuledlg->IsRunning())
				PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE,(WPARAM)-1,(LPARAM)m_client);
			else if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL){
				delete[] filedata;
				filedata = NULL;
			}
			e->Delete();
			return 2;
		}
		catch(...)
		{
			if (lockFile.m_pObject) { //Fafner: missing? - 080421
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}
			if (theApp.emuledlg && theApp.emuledlg->IsRunning())
				PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE,(WPARAM)-1,(LPARAM)m_client);
			else if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL){
				delete[] filedata;
				filedata = NULL;
			}
			return 3;
		}
		pauseEvent.Lock();
	}
	return 0;
}
//MORPH END    - Changed by SiRoB, ReadBlockFromFileThread

// ==> push small files [sivka] - Stulle
bool CUpDownClient::GetSmallFilePush() const
{
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	return(currequpfile &&
		currequpfile->IsPushSmallFile());
}
// <== push small files [sivka] - Stulle

// ==> push rare file - Stulle
float CUpDownClient::GetRareFilePushRatio() const {
	if(!thePrefs.GetEnablePushRareFile())
		return 1.0f;
	CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(/*(uchar*)*/GetUploadFileID());
	if (srcfile == (CKnownFile*)NULL)
		return 4.0f;
	
	// keep the FileRatio
	/*
	float ratio = 0+srcfile->GetFileRatio() ;
	return (ratio < 1.0f ? 1.0f :((ratio>100.0f)?100.0f: ratio)) ;	
	*/
	return srcfile->GetFileRatio();
}
// <== push rare file - Stulle

// ==> HideOS & SOTN [Slugfiller/ MorphXT] - Stulle
void CUpDownClient::GetUploadingAndUploadedPart(uint8* m_abyUpPartUploadingAndUploaded, uint32 partcount) const
{
	memset(m_abyUpPartUploadingAndUploaded,0,partcount);
	const Requested_Block_Struct* block;
	if (!m_BlockRequests_queue.IsEmpty()){
		block = m_BlockRequests_queue.GetHead();
		if(block){
			uint32 start = (UINT)(block->StartOffset/PARTSIZE);
			m_abyUpPartUploadingAndUploaded[start] = 1;
		}
	}
	if (!m_DoneBlocks_list.IsEmpty()){
		for(POSITION pos=m_DoneBlocks_list.GetHeadPosition();pos!=0;){
			block = m_DoneBlocks_list.GetNext(pos);
			uint32 start = (UINT)(block->StartOffset/PARTSIZE);
			m_abyUpPartUploadingAndUploaded[start] = 1;
		}
	}
}
// <== HideOS & SOTN [Slugfiller/ MorphXT] - Stulle

// ==> Superior Client Handling [Stulle] - Stulle
/* This function ist meant to keep full compatibility for further cases */
/* that could make a client superior to others. This includes features  */
/* like PBF or similar.                                                 */
/* Friends have 0x0FFFFFFF as the score they will exceed the score of   */
/* other superior clients, so they will get the upload slot.            */
/* Only restriction for friends is an existing reqfile.                 */
/* No bad guys will ever get this status!                               */
/* So far included are the following features:                          */
/* PowerShare                                                           */
/* Pay Back First (for insecure clients)                                */
/* FairPlay                                                             */
bool CUpDownClient::IsSuperiorClient() const
{
	CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID(GetUploadFileID());

	// only clients requesting a valid file can be superior
	if(currentReqFile == NULL)
		return false;

	// friend with friendslot
	if(IsFriend() && GetFriendSlot() && !HasLowID())
		return true;

	// no bad guys!
	if(GetUploadState()==US_BANNED || m_bGPLEvildoer || IsLeecher())
		return false;

	// only identified or not available SUI
	if(credits)
	{
		EIdentState currentIDstate =  credits->GetCurrentIdentState(GetIP());
		if(currentIDstate != IS_IDENTIFIED  &&
			currentIDstate != IS_NOTAVAILABLE &&
			theApp.clientcredits->CryptoAvailable())
			return false;
	}

	// no thing else is allowed if the requested file is incomplete
	if(currentReqFile->IsPartFile())
		return false;

	// ==> PowerShare [ZZ/MorphXT] - Stulle
	// powershared
	if(currentReqFile->GetPowerShared())
		return true;
	// <== PowerShare [ZZ/MorphXT] - Stulle

	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
	if(credits)
	{
		bool bIsSecure = theApp.clientcredits->CryptoAvailable() && credits->GetCurrentIdentState(GetIP()) == IS_IDENTIFIED;
		bool bIsNotAvail = theApp.clientcredits->CryptoAvailable() && credits->GetCurrentIdentState(GetIP()) == IS_NOTAVAILABLE;

		if(credits->GetPayBackFirstStatus() && thePrefs.IsPayBackFirst() && bIsSecure)
			return true;
		if(credits->GetPayBackFirstStatus2() && thePrefs.IsPayBackFirst2() && bIsNotAvail)
			return true;
	}
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

	// ==> Fair Play [AndCycle/Stulle] - Stulle
	if(currentReqFile->statistic.GetFairPlay())
		return true;
	// <== Fair Play [AndCycle/Stulle] - Stulle

	return false;
}
// <== Superior Client Handling [Stulle] - Stulle

// ==> PowerShare [ZZ/MorphXT] - Stulle
bool CUpDownClient::GetPowerShared() const {
	CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
	return currentReqFile && currentReqFile->GetPowerShared();
}

bool CUpDownClient::GetPowerShared(const CKnownFile* file) const {
	return file->GetPowerShared();
}
// <== PowerShare [ZZ/MorphXT] - Stulle

// ==> Design Settings [eWombat/Stulle] - Stulle
bool CUpDownClient::GetPowerReleased() const {
	CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
	return currentReqFile && currentReqFile->GetUpPriority()==PR_POWER;
}
// <== Design Settings [eWombat/Stulle] - Stulle

// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
bool CUpDownClient::IsPBFClient() const
{
	CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID(GetUploadFileID());

	// only clients requesting a valid file, which is not PartFile can be superior
	if(currentReqFile == NULL || currentReqFile->IsPartFile())
		return false;

	// no bad guys!
	if(GetUploadState()==US_BANNED || m_bGPLEvildoer || IsLeecher())
		return false;

	if(credits)
	{
		bool bIsSecure = theApp.clientcredits->CryptoAvailable() && credits->GetCurrentIdentState(GetIP()) == IS_IDENTIFIED;
		bool bIsNotAvail = theApp.clientcredits->CryptoAvailable() && credits->GetCurrentIdentState(GetIP()) == IS_NOTAVAILABLE;

		if(credits->GetPayBackFirstStatus() && thePrefs.IsPayBackFirst() && bIsSecure)
			return true;
		if(credits->GetPayBackFirstStatus2() && thePrefs.IsPayBackFirst2() && bIsNotAvail)
			return true;
	}

	return false;
}

bool CUpDownClient::IsSecure() const
{
	return credits && theApp.clientcredits->CryptoAvailable() && credits->GetCurrentIdentState(GetIP()) == IS_IDENTIFIED;
}
// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle

// ==> Display remaining upload time [Stulle] - Mephisto
CString CUpDownClient::GetRemainingUploadTime() const
{
	if(IsFriend() && GetFriendSlot() || GetUploadDatarate() <= 0 || upendsoon)
		return _T("?");

	sint64 sTogo = 0;

	if(IsPBFClient() && Credits())
	{
		sTogo = sint64(Credits()->GetDownloadedTotal()) - sint64(Credits()->GetUploadedTotal());
		if(sTogo > 0)
			return CastSecondsToHM(time_t(UINT(sTogo)/GetUploadDatarate()));
		else
			return _T("?");
	}

	// This is way too expensive! We just use usual way and have the sTogo <= 0 fallback to _T("?")
	/*
	if(IsSuperiorClient())
	{
		CUpDownClient* bestClient = theApp.uploadqueue->FindBestClientInQueue(true);
		if(!bestClient || bestClient->IsSuperiorClient()==false)
			return _T("?");
	}
	*/

	switch(thePrefs.GetChunksMode())
	{
		case CHUNK_SCORE:
		{
			// At least display how much time till min
			if(GetSessionUp() < 2097152)
				sTogo = 2097152 - GetSessionUp();
			break;
		}
		case CHUNK_XMAN:
		case CHUNK_FINISH:
		{
			const UINT uMaxBytesToUpload = (thePrefs.GetChunksMode()==CHUNK_XMAN) ? SESSIONMAXTRANS : (thePrefs.GetChunksToFinish()*PARTSIZE);
			if(!m_DoneBlocks_list.IsEmpty())
			{
				CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
				if(currentReqFile)
				{
					Requested_Block_Struct* lastBlock = m_DoneBlocks_list.GetHead();
					const uint32 lastDone = (uint32)(lastBlock->StartOffset / PARTSIZE);

					// Bytes to go for current part
					sTogo = (lastDone+1)*PARTSIZE-1;
					if((uint64)sTogo > currentReqFile->GetFileSize())
						sTogo = (sint64)currentReqFile->GetFileSize();
					sTogo -= lastBlock->EndOffset;

					if(thePrefs.GetChunksMode()==CHUNK_FINISH)
					{
						// Bytes to go for coming up parts
						const uint32 uPartsToGo = thePrefs.GetChunksToFinish() - 1 - GetFinishedChunks();
						sTogo += ((uPartsToGo>0)?uPartsToGo:0)*PARTSIZE;
					}

					if(GetSessionUp() + sTogo < 3145728) // If the part ends too early
						sTogo = uMaxBytesToUpload - GetQueueSessionPayloadUp(); // we assume full uMaxBytesToUpload will be sent.
					else if(GetQueueSessionPayloadUp() > uMaxBytesToUpload) // If we Uploaded too much already
						sTogo = -1; // we don't display anything anymore.
					else if(GetSessionUp() + sTogo > uMaxBytesToUpload) // If we uploaded too much to finish the current chunk
						sTogo = uMaxBytesToUpload - GetQueueSessionPayloadUp(); // we will send uMaxBytesToUpload at the most.
				}
			}
			else // Fallback when we haven't finished uploading any block
				sTogo = uMaxBytesToUpload - GetQueueSessionPayloadUp();
			break;
		}
		case CHUNK_FULL:
		{
			sTogo = thePrefs.GetChunksToUpload()*PARTSIZE - GetQueueSessionPayloadUp();
			break;
		}
	}
	
	if(sTogo>0)
		return CastSecondsToHM(time_t(UINT(sTogo)/GetUploadDatarate()));

	return _T("?");
}
// <== Display remaining upload time [Stulle] - Mephisto