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
#include "Sockets.h"
#include "OtherFunctions.h"
#include "SafeFile.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "TransferWnd.h"
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


void CUpDownClient::DrawUpStatusBar(CDC* dc, RECT* rect, bool onlygreyrect, bool  bFlat) const
{
    COLORREF crNeither;
	COLORREF crNextSending;
	COLORREF crBoth;
	COLORREF crSending;

    if(GetUploadState() == US_UPLOADING  ) { //Xman Xtreme Upload
        crNeither = RGB(224, 224, 224);
	    crNextSending = RGB(255,208,0);
	    crBoth = bFlat ? RGB(32, 32, 32) : RGB(104, 104, 104);
	    crSending = RGB(0, 150, 0); //RGB(186, 240, 0);
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
		filesize=currequpfile->GetFileSize();
	else
		filesize = (uint64)(PARTSIZE * (uint64)m_nUpPartCount);
	// wistily: UpStatusFix

    if(filesize > (uint64)0) {
		// Set size and fill with default color (grey)
		CBarShader statusBar(rect->bottom - rect->top);// X: [CI] - [Code Improvement] BarShader
		statusBar.SetFileSize(filesize); 
		statusBar.Fill(crNeither); 
	    if (!onlygreyrect && m_abyUpPartStatus) { 
		    for (UINT i = 0;i < m_nUpPartCount;i++)
			    if(m_abyUpPartStatus[i])
				    statusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crBoth);
// morph4u :: SOTN :: Start
				else if (m_abyUpPartStatusHidden)
				{
					if(m_abyUpPartStatusHidden[i] == 1) //hidden via TCP
						statusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1), RGB(0, 192, 192));
					else if(m_abyUpPartStatusHidden[i] == 2) //hidden via UDP
						statusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1), RGB(0, 100, 100));
				}
// morph4u :: SOTN :: End
	    }
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
   	    statusBar.Draw(dc, rect->left, rect->top, rect->right - rect->left, bFlat);// X: [CI] - [Code Improvement] BarShader
    }
} 

void CUpDownClient::SetUploadState(EUploadState eNewState)
{
	if (eNewState != m_nUploadState)
	{
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		if(m_nUploadState == US_UPLOADING || eNewState == US_UPLOADING || m_nUploadState== US_CONNECTING){
			m_nUpDatarate = 0;
			m_nUpDatarate10 = 0;
			m_nUpDatarateMeasure = 0;
			m_upHistory_list.RemoveAll();

			if (eNewState == US_UPLOADING)
			{
				m_fSentOutOfPartReqs = 0;

				if(socket != NULL)
				{
					//int newValue = thePrefs.GetSendbuffersize(); // default: 8192;
					sendbuffersize = 6000;// X: [DSRB] - [Dynamic Send and Receive Buffer]
					//int oldValue = 0;
					//int size = sizeof(newValue);
					socket->SetSockOpt(SO_SNDBUF, &sendbuffersize, sizeof(sendbuffersize), SOL_SOCKET);
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
			SetUploadFileID(NULL); 
		}

		// don't add any final cleanups for US_NONE here
		m_nUploadState = (_EUploadState)eNewState;
	}
}

/**
 * Gets the queue score multiplier for this client, taking into consideration client's credits
 * and the requested file's priority.
 */
float CUpDownClient::GetCombinedFilePrioAndCredit() {
	if (credits == 0){
		ASSERT ( (GetUploadState()==US_BANNED) ); //zz_fly :: in the Optimized on ClientCredits, banned client has no credits
		return 0.0f;
	}
	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;

    return 100.0f*credits->GetScoreRatio(GetIP())*GetFilePrioAsNumber(currequpfile);
}

/**
 * Gets the file multiplier for the file this client has requested.
 */
//Xman modified:
float CUpDownClient::GetFilePrioAsNumber(const CKnownFile* currequpfile) const { // WiZaRd
	switch(currequpfile->GetUpPriority()){
		//Xman PowerRelease
		case PR_POWER:
			if(currequpfile->statistic.GetAllTimeTransferred() < 100 * 1024 * 1024 || (currequpfile->statistic.GetAllTimeTransferred() < (uint64)((uint64)currequpfile->GetFileSize() * 3 / 2/**1.5f*/)))
				return 20.0f;
			else
				return 10.0f;
			break;
		//Xman end
		case PR_VERYHIGH:
			return 2.5f;
			break;
		case PR_HIGH: 
			return thePrefs.UseAdvancedAutoPtio() ? 1.5f : 1.4f; //Xman advanced upload-priority
			break; 
		case PR_LOW: 
			return thePrefs.UseAdvancedAutoPtio() ? 0.6f : 0.7f; //Xman advanced upload-priority
			break; 
		case PR_VERYLOW:
			return 2.0f;
			break;
		case PR_NORMAL: 
			default: 
			return 1.0f; 
		break; 
	} 
}

/**
 * Gets the current waiting score for this client, taking into consideration waiting
 * time, priority of requested file, and the client's credits.
 */
//Xman Code Improvement
uint_ptr CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue) const
{
	if (m_pszUsername == NULL || GetUploadFileID() == NULL)
		return 0;

	if (credits == 0){
		ASSERT ( (GetUploadState()==US_BANNED) ); //zz_fly :: in the Optimized on ClientCredits, banned client has no credits
		return 0;
	}
	
	// bad clients (see note in function)
	if (credits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
		return 0;

	CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	if(!currequpfile)
		return 0;

// ==> Pay Back First
	// PBF slot
	if (IsPBFClient() && !HasLowID())
		return 0x0FFFFFFF;
// <== Pay Back First
	//if (IsBanned() || m_bGPLEvildoer)
	if (GetUploadState()==US_BANNED || m_bGPLEvildoer || //Xman Code Improvement 
    /*return 0;

	if */(sysvalue && HasLowID() && !(socket && socket->IsConnected())))
		return 0;

    //int filepriority = GetFilePrioAsNumber();

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
		//fBaseValue += (float)(::GetTickCount() - m_dwUploadTime > 900000)? 900000:1800000;
		//Xman Xtreme Upload
		//we can not give a high bonus during the first 15 minutes, because of varying slotspeed
		//Xtreme Mod avoids too short uploadsessions in CheckFortimeover()
		fBaseValue +=(float)900000;
		//Xman end
		fBaseValue /= 1000;
	}
	
		float modif = credits->GetScoreRatio(GetIP());
	fBaseValue *= modif;

		if (!onlybasevalue)
	{
		//Xman
		// Maella -One-queue-per-file- (idea bloodymad)
		if(thePrefs.GetEnableMultiQueue() == false)
			fBaseValue *= GetFilePrioAsNumber(currequpfile);
		else
		{
			// File Score
			// Remark: there is an overflow after ~49 days
			uint_ptr fileScore = currequpfile->GetFileScore(isdownloading ? GetUpStartTimeDelay() : 0); // about +1 point each second 
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
		//Xman end
	}

	if( (IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19 )
		fBaseValue *= 0.5f;
	//Xman Xtreme Mod : 80% score for non SI clients
	else if(credits->GetCurrentIdentState(GetIP()) != IS_IDENTIFIED)
		fBaseValue *= 0.8f;
	//Xman end

	return (uint_ptr)fBaseValue;
}
//Xman end

class CMutexHelper
{
public:
	CMutexHelper()
	{
		m_pMutex = NULL;
	}
	~CMutexHelper()
	{
		if (m_pMutex)
			m_pMutex->Unlock();
	}
	Poco::FastMutex* m_pMutex;
};


//Xman Code Improvement
// BEGIN SiRoB: ReadBlockFromFileThread
void CUpDownClient::CreateNextBlockPackage(){

	//Xman ReadBlockFromFileThread Improvement
	if(filedata == (byte*)-2)
		return; //operation in progress
	//Xman end

	// See if we can do an early return. There may be no new blocks to load from disk and add to buffer, or buffer may be large enough allready.
	if(upendsoon || //Xman Full Chunk
		m_BlockRequests_queue.IsEmpty()) // There are no new blocks requested
	{
		if (filedata != NULL && filedata != (byte*)-1)
		{
			ATLASSERT(0);
			delete[] filedata;
			filedata = NULL;
		}
			return;
	}
	// the buffered data is large enough allready
    if (m_addedPayloadQueueSession > GetQueueSessionPayloadUp() && GetPayloadInBuffer() > (uint32)sendbuffersize << 3)  // X: [DSRB] - [Dynamic Send and Receive Buffer] 6000 * 8 - 128*1024 * 8 
        return;

	bool bFromPF = true; // Statistic to breakdown uploaded data by complete file vs. partfile.
	try{
		do {
			ATLASSERT(filedata != (byte*)-2);
				//Xman Full Chunk
				//at this point we do the check if it is time to kick the client
				//if we kick soon, we don't add new packages
				//Xman ReadBlockFromFileThread:
				//-->we first have to check if we have unprocessed data (can happen if full chunk is disabled)
				//-->first process it, then check for timeOver
				//-->in case of an exception, allow to throw it
				if(filedata==NULL)
			{
					upendsoon=theApp.uploadqueue->CheckForTimeOver(this);
				if(upendsoon==true)
					break;
			}
			else if (filedata == (byte*)-1) {
				filedata = NULL;
				//An error occured
				if(!theApp.sharedfiles->IsUnsharedFile(m_BlockRequests_queue.GetHead()->FileID))
					theApp.sharedfiles->Reload(false);// X: [QOH] - [QueryOnHashing]
				throw GetResString(IDS_ERR_OPEN);
			}
				//Xman end

				Requested_Block_Struct* currentblock = m_BlockRequests_queue.GetHead();
				CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(currentblock->FileID);
				if (!srcfile)
					throw GetResString(IDS_ERR_REQ_FNF);

				uint64 i64uTogo;
				if (currentblock->StartOffset > currentblock->EndOffset){
					i64uTogo = currentblock->EndOffset + (srcfile->GetFileSize() - currentblock->StartOffset);
				}
				else{
					i64uTogo = currentblock->EndOffset - currentblock->StartOffset;
					// BEGIN SiRoB, SLUGFILLER: SafeHash
					/*
					if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsComplete(currentblock->StartOffset,currentblock->EndOffset-1, true))
					*/
					if (srcfile->IsPartFile() && !((CPartFile*)srcfile)->IsRangeShareable(currentblock->StartOffset,currentblock->EndOffset-1))	// SLUGFILLER: SafeHash - final safety precaution
					// END SiRoB, SLUGFILLER: SafeHash
						throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
// morph4u :: SOTN :: Start
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
// morph4u :: SOTN :: End
				}

				if( i64uTogo > EMBLOCKSIZE*3 )
					throw GetResString(IDS_ERR_LARGEREQBLOCK);
				uint32 togo = (uint32)i64uTogo;


				if (filedata == NULL) {
					SetUploadFileID(srcfile); //Xman - Moved by SiRoB, Fix Filtered Block Request
					filedata = (byte*)-2;
				CReadBlockFromFileThread* readblockthread = new CReadBlockFromFileThread(srcfile, currentblock->StartOffset, togo, this);
				// X: queued disc-access for read/flushing-threads
				theApp.AddNewDiscAccessThread(readblockthread);
					return;
				}

				if (!srcfile->IsPartFile())
					bFromPF = false; // This is not a part file...

				//Xman - Removed by SiRoB, Fix Filtered Block Request
				/*
				SetUploadFileID(srcfile);
				*/

				// check extension to decide whether to compress or not
				//Xman Code Improvement for choosing to use compression
				// Decide whether to compress the packets or not
				if((m_byDataCompVer == 1)
				//Xman disable compression
					&& thePrefs.m_bUseCompression
					/* moved to abstractfile
					// Check extension
					*/
					&& srcfile->IsCompressible())
				//Xman end
					CreatePackedPackets(filedata,togo,currentblock,bFromPF);
				else
					CreateStandartPackets(filedata,togo,currentblock,bFromPF);
				//Xman end Code Improvement for choosing to use compression

				//Xman Xtreme Upload 
				if(socket && socket->isready==false)
				{
					socket->isready=true;
					theApp.uploadBandwidthThrottler->SetNoNeedSlot();
				}

				// file statistic
// ==> Spread bars [Slugfiller/MorphXT] - Stulle
			//srcfile->statistic.AddTransferred(togo);
			srcfile->statistic.AddTransferred(currentblock->StartOffset, togo);
// <== Spread bars [Slugfiller/MorphXT] - Stulle

				m_addedPayloadQueueSession += togo;

				m_DoneBlocks_list.AddHead(m_BlockRequests_queue.RemoveHead());

				// Maella -One-queue-per-file- (idea bloodymad)
				srcfile->UpdateStartUploadTime();
				// Maella end

				delete[] filedata;
				filedata = NULL;
		// Buffer new data if current buffer is less than nBufferLimit Bytes
		} while (!m_BlockRequests_queue.IsEmpty() && /*filedata != (byte*)-2 &&*/
			(m_addedPayloadQueueSession <= GetQueueSessionPayloadUp() || GetPayloadInBuffer() < (uint32)sendbuffersize << 3));
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), GetUserName(), error);
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Client error: ") + error, CUpDownClient::USR_EXCEPTION); // Maella -Upload Stop Reason-
		ATLASSERT(filedata != (byte*)-1);
		if (filedata != (byte*)-2 && filedata != (byte*)-1 && filedata != NULL) {
			delete[] filedata;
			filedata = NULL;
		}
	}
	catch(CFileException* e)
	{
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("Failed to create upload package for %s - %s"), GetUserName(), szError);
		theApp.uploadqueue->RemoveFromUploadQueue(this, ((CString)_T("Failed to create upload package.")) + szError, CUpDownClient::USR_EXCEPTION); // Maella -Upload Stop Reason-
		ATLASSERT(filedata != (byte*)-1);
		if (filedata != (byte*)-2 && filedata != (byte*)-1 && filedata != NULL) {
			delete[] filedata;
			filedata = NULL;
		}
		e->Delete();
	}
}
// END SiRoB: ReadBlockFromFileThread
//Xman end

bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile, bool isUDP) //Xman better passive source finding
{

	//Xman client percentage
	hiscompletedparts_percent_up=-1;
	//Xman end

	delete[] m_abyUpPartStatus;
	m_abyUpPartStatus = NULL;	
// morph4u :: SOTN :: Start
	//No need to clear it here - might be useful later!
	//	delete[] m_abyUpPartStatusHidden;
	//	m_abyUpPartStatusHidden = NULL;
	const UINT nOldUpPartCount = m_nUpPartCount;
// morph4u :: SOTN :: End
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;
	if (GetExtendedRequestsVersion() == 0)
	{
// morph4u :: SOTN :: Start
		//returning here means that we already lost the partstatus, 
		//so we should delete the hidden status, too (I guess?)
		delete[] m_abyUpPartStatusHidden;
		m_abyUpPartStatusHidden = NULL;
// morph4u :: SOTN :: End
		return true;
	}// morph4u :: SOTN

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
// morph4u :: SOTN :: Start
			//as stated above - if we get here, something's REALLY wrong!
			delete[] m_abyUpPartStatusHidden;
			m_abyUpPartStatusHidden = NULL;
// morph4u :: SOTN :: End
			return false;
		}
		//Xman client percentage
		size_t hisfinishedparts=0;
		//Xman end

		m_nUpPartCount = tempreqfile->GetPartCount();
		m_abyUpPartStatus = new uint8[m_nUpPartCount];
		size_t done = 0;
		while (done != m_nUpPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (UINT i = 0;i != 8;i++)
			{
				m_abyUpPartStatus[done] = ((toread >> i) & 1) ? 1 : 0;
				//Xman better passive source finding
				if (shouldbechecked && bPartsNeeded==false && m_abyUpPartStatus[done]
					&& (!((CPartFile*)tempreqfile)->IsPartShareable(i) &&  // X: [CI] - [Code Improvement]
						!((CPartFile*)tempreqfile)->IsComplete((uint64)done*PARTSIZE,((uint64)(done+1)*PARTSIZE)-1,false)))
					bPartsNeeded = true;
				//Xman end

				//Xman client percentage
				if(m_abyUpPartStatus[done])
					hisfinishedparts++;
				//Xman end

				done++;
				if (done == m_nUpPartCount){
					//Xman client percentage
					if(m_abyUpPartStatus[done-1]) // X: more accuracy
						hiscompletedparts_percent_up= (sint8)(((uint64)tempreqfile->GetFileSize() - (m_nUpPartCount - hisfinishedparts) * PARTSIZE)*100 / (uint64)tempreqfile->GetFileSize());
					else
						hiscompletedparts_percent_up= (sint8)(hisfinishedparts * PARTSIZE * 100 / (uint64)tempreqfile->GetFileSize());
					//Xman end
					break;
				}
			}
		}
	}
// morph4u :: SOTN :: Start
//>>> taz::optimization
	if (m_abyUpPartStatusHidden == NULL || nOldUpPartCount != m_nUpPartCount)
	{
		if(m_abyUpPartStatusHidden != NULL)
			delete[] m_abyUpPartStatusHidden;
		m_abyUpPartStatusHidden = new uint8[m_nUpPartCount];
		memset(m_abyUpPartStatusHidden, 0, m_nUpPartCount);
	}
//<<< taz::optimization
// morph4u :: SOTN :: End
	if (GetExtendedRequestsVersion() > 1)
	{
		uint16 nCompleteCountLast = GetUpCompleteSourcesCount();
		uint16 nCompleteCountNew = data->ReadUInt16();
		SetUpCompleteSourcesCount(nCompleteCountNew);
		if (nCompleteCountLast != nCompleteCountNew)
			tempreqfile->UpdatePartsInfo();
		}

	
	//Xman
	// Maella -Code Improvement-
	if(tempreqfile->IsPartFile() == true && m_nUpPartCount != 0){
		// Check if a source has now chunk that we can need
		POSITION pos = m_OtherNoNeeded_list.Find((CPartFile*)tempreqfile);
		if(pos != NULL){
			for(uint16 i = 0; i < m_nUpPartCount; i++){ 
				if(m_abyUpPartStatus[i] != 0){ 
					const uint64 uStart = PARTSIZE*(uint64)i;
					const uint64 uEnd = /*((uint64)tempreqfile->GetFileSize()-1 <= (uStart+PARTSIZE-1)) ? ((uint64)tempreqfile->GetFileSize()-1) : */(uStart+PARTSIZE-1);
					if(!((CPartFile*)tempreqfile)->IsPartShareable(i) &&  // X: [CI] - [Code Improvement]
						((CPartFile*)tempreqfile)->IsComplete(uStart, uEnd, false) == false){
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
				|| ((CPartFile*)tempreqfile)->GetSourceCount() < (((CPartFile*)tempreqfile)->GetMaxSources() * 4 / 5/**0.8f*/ + 1))
			{
					if(theApp.downloadqueue->CheckAndAddKnownSource((CPartFile*)tempreqfile,this, true))
						AddDebugLogLine(false, _T("->found new source on reask-ping: %s, file: %s"), DbgGetClientInfo(), tempreqfile->GetFileName());
			}
		}
		else
		{
			if (AddRequestForAnotherFile((CPartFile*)tempreqfile))
			{
				theApp.emuledlg->transferwnd->downloadclientsctrl.AddClient(this,(CPartFile*)tempreqfile);
				AddDebugLogLine(false, _T("->found new A4AF source on reask-ping: %s, file: %s"), DbgGetClientInfo(), tempreqfile->GetFileName());
			}
		}
	}
	return true;
}

void CUpDownClient::CreateStandartPackets(byte* data,uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	uint32 nPacketSize;
	CMemFile memfile((BYTE*)data,togo);
	
	//Xman flexible splittingsize
	uint32 splittingsize = 10240;
	if( m_nUpDatarate10 > 5120 && GetDownloadState()!=DS_DOWNLOADING)
	{
		splittingsize = m_nUpDatarate10 << 1;
		if (splittingsize > 36000)
			splittingsize = 36000;
	}
	
	if (togo > splittingsize) 
		nPacketSize = togo/(uint32)(togo/splittingsize);
	else
		nPacketSize = togo;
	//Xman end
	while (togo){
		if (togo < nPacketSize*2)
			nPacketSize = togo;
		ASSERT( nPacketSize );
		togo -= nPacketSize;

		uint64 statpos = (currentblock->EndOffset - togo) - nPacketSize;
		uint64 endpos = (currentblock->EndOffset - togo);
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

void CUpDownClient::CreatePackedPackets(byte* data, uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF){
	BYTE* output = new BYTE[togo+300];
	uLongf newsize = togo+300;
	//Xman used different values!
	// BEGIN netfinity: Variable compression - Reduce CPU usage for high bandwidth connections
	//  Preferably this should take CPU speed into account
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
	CMemFile memfile(output,newsize);
	uint32 oldSize = togo;
	togo = newsize;
	uint32 nPacketSize;

	//Xman flexible splittingsize
	uint32 splittingsize = 10240;
	if( m_nUpDatarate10 > 5120 && GetDownloadState()!=DS_DOWNLOADING)
	{
		splittingsize = m_nUpDatarate10 << 1; //one packet can be send between 2 - 4 seconds
		if (splittingsize > 36000)
			splittingsize = 36000;
	}

	if (togo > splittingsize) 
		nPacketSize = togo/(uint32)(togo/splittingsize);
	else
		nPacketSize = togo;
	//Xman end

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
		//theStats.AddUpDataOverheadFileRequest(24); //Xman fix: we have different sizes , moved up
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
// morph4u :: SOTN :: Start
	delete[] m_abyUpPartStatusHidden;
	m_abyUpPartStatusHidden = NULL;
// morph4u :: SOTN :: End
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
	{
		oldreqfile->RemoveUploadingClient(this);
		ClearUploadBlockRequests(false); //Xman - Added by SiRoB, Fix Filtered Block Request
	}
	
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
		//if(thePrefs.GetLogUlDlEvents())
           // AddDebugLogLine(DLP_LOW, false, _T("UploadClient: Client tried to add req block when not in upload slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
		delete reqblock;
        return;
    }

	if(HasCollectionUploadSlot()){
		CKnownFile* pDownloadingFile = theApp.sharedfiles->GetFileByID(reqblock->FileID);
		if(pDownloadingFile != NULL){
			if ( !(pDownloadingFile->HasCollectionExtenesion_Xtreme() /*CCollection::HasCollectionExtention(pDownloadingFile->GetFileName())*/ && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) ) //Xman Code Improvement for HasCollectionExtention
			{
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
//Upload Permission +
            if (GetUploadFileID())
			{
			CKnownFile* file = theApp.sharedfiles->GetFileByID(GetUploadFileID());
			if (file)
			{
            int Perm =file->GetPermissions()>=0?file->GetPermissions():thePrefs.GetPermission();
			 if (Perm == PERM_NOONE)
					theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Client has no longer Permission to be in Upload"));
			  }
			}
//Upload Permission -
    //DWORD curTick = ::GetTickCount(); //Xman 

    uint64 sentBytesCompleteFile = 0;
    uint64 sentBytesPartFile = 0;
    uint64 sentBytesPayload = 0;

    if (socket)
	{
		CEMSocket* s = socket;
		UINT uUpStatsPort = GetUserPort();

	    // Extended statistics information based on which client software and which port we sent this data to...
	    // This also updates the grand total for sent bytes, etc.  And where this data came from.
        sentBytesCompleteFile = s->GetSentBytesCompleteFileSinceLastCallAndReset();
        sentBytesPartFile = s->GetSentBytesPartFileSinceLastCallAndReset();
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, (UINT)sentBytesCompleteFile, IsPBFClient());
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, (UINT)sentBytesPartFile, IsPBFClient());

		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		AddUploadRate((UINT)(sentBytesCompleteFile + sentBytesPartFile));
		//m_nTransferredUp += sentBytesCompleteFile + sentBytesPartFile;
        //credits->AddUploaded(sentBytesCompleteFile + sentBytesPartFile, GetIP());
		//Xman end

        sentBytesPayload = s->GetSentPayloadSinceLastCallAndReset();
		m_nCurQueueSessionPayloadUp = (UINT)(m_nCurQueueSessionPayloadUp + sentBytesPayload);

		//Xman Full Chunk
		//in CreateNextBlockPackage we saw this upload end soon,
		//after all packets are send, we cancel this upload
		if (upendsoon && s->StandardPacketQueueIsEmpty()) {
			credits->InitPayBackFirstStatus(); // Pay Back First
			theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Completed transfer"),CUpDownClient::USR_COMPLETEDRANSFER ,true ); // Maella -Upload Stop Reason-
			SendOutOfPartReqsAndAddToWaitingQueue(thePrefs.TransferFullChunks() ? true:false); //Xman Full Chunk
        } 
		else {
            if(upendsoon==false) //Xman Full Chunk
				// read blocks from file and put on socket
				CreateNextBlockPackage();
        }
    }

	return (UINT)(sentBytesCompleteFile + sentBytesPartFile);
}

void CUpDownClient::SendOutOfPartReqsAndAddToWaitingQueue(bool givebonus) //Xtreme Full Chunk
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
    
	//Xtreme Full Chunk
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
	
	theApp.uploadqueue->AddClientToQueue(this, true);

	if(bonus>0)
	{
		if(credits)
		{
			credits->SetWaitStartTimeBonus(GetIP(),::GetTickCount()-bonus);
			AddDebugLogLine(false, _T("giving client bonus. old waitingtime: %s, new waitingtime: %s, client: %s"), CastSecondsToHM(waitingtime/1000), CastSecondsToHM((::GetTickCount() - GetWaitStartTime())/1000),DbgGetClientInfo()); 
		}
	}
}

/**
 * See description for CEMSocket::TruncateQueues().
 */
void CUpDownClient::FlushSendBlocks(){ // call this when you stop upload, or the socket might be not able to send
    //Xman Code Fix
	if (socket)      //socket may be NULL...
        socket->TruncateQueues();
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

void CUpDownClient::ClearUploadBlockRequests(bool truncatequeues) //Xman - Fix Filtered Block Request
{
	//Xman - Fix Filtered Block Request
	if(truncatequeues)
		FlushSendBlocks();
	//Xman end

	for (POSITION pos = m_BlockRequests_queue.GetHeadPosition();pos != 0;)
		delete m_BlockRequests_queue.GetNext(pos);
	m_BlockRequests_queue.RemoveAll();
	
	for (POSITION pos = m_DoneBlocks_list.GetHeadPosition();pos != 0;)
		delete m_DoneBlocks_list.GetNext(pos);
	m_DoneBlocks_list.RemoveAll();

	//Xman
	// BEGIN SiRoB: ReadBlockFromFileThread
	if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL) {
		delete[] filedata;
		filedata = NULL;
	}
	// END SiRoB: ReadBlockFromFileThread

}

void CUpDownClient::SendRankingInfo(){ 
	if (!ExtProtocolAvailable()) 
		return; 
//Upload Permission +
        UINT nRank = NULL;
	    CKnownFile* file = theApp.sharedfiles->GetFileByID(GetUploadFileID());
	    if (file)
	    {
        int Perm =file->GetPermissions()>=0?file->GetPermissions():thePrefs.GetPermission();
        if(RandomQrGen && !theApp.uploadqueue->GetWaitingPosition(this) && (Perm == PERM_NOONE || Perm == PERM_UPMANA))
		{ 
        int diffQR = rand()%10+1; 
        nRank = RandomQrGen - diffQR; 
		//AddLogLine(false, L"[Send FakeRank] User: {%s} Qrs = (%i) %s", this->GetUserName(), nRank, file->GetFileName()); 
		RandomQrGen = nRank; 
        }
		}
        else
        {
        nRank = theApp.uploadqueue->GetWaitingPosition(this);
        }  
//Upload Permission -
	if (!nRank)
		return;
	Packet* packet = new Packet(OP_QUEUERANKING,12,OP_EMULEPROT); 
	PokeUInt16(packet->pBuffer, (uint16)nRank);
	memset(packet->pBuffer+2, 0, 10); 
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)// X: [RDL] - [Remove Debug Log]
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__QueueRank", this);
#endif
	theStats.AddUpDataOverheadFileRequest(packet->size); 
	SendPacket(packet, true);
} 

void CUpDownClient::AddRequestCount(const uchar* fileid)
{
	const DWORD cur_tick = GetTickCount();
	for (POSITION pos = m_RequestedFiles_list.GetHeadPosition(); pos != 0; ){
		Requested_File_Struct* cur_struct = m_RequestedFiles_list.GetNext(pos);
		if (!md4cmp(cur_struct->fileid,fileid)){
			if (cur_tick - cur_struct->lastasked < MIN_REQUESTTIME && !IsPBFClient()){ 
				if (GetDownloadState() != DS_DOWNLOADING)
					cur_struct->badrequests++;
				if (cur_struct->badrequests == BADCLIENTBAN){
					//Ban();
					Ban(_T("Bad Requests"));
				}
			}
			else{
				if (cur_struct->badrequests)
					cur_struct->badrequests--;
			}
			cur_struct->lastasked = cur_tick;
			return;
		}
	}
	Requested_File_Struct* new_struct = new Requested_File_Struct;
	md4cpy(new_struct->fileid,fileid);
	new_struct->lastasked = cur_tick;
	new_struct->badrequests = 0;
	m_RequestedFiles_list.AddHead(new_struct);
}

void  CUpDownClient::UnBan()
{
	theApp.clientlist->AddTrackClient(this);
	theApp.clientlist->RemoveBannedClient(GetIP());
	SetUploadState(US_NONE);
	ClearWaitStartTime();
	theApp.emuledlg->transferwnd->ShowQueueCount();
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
	theApp.clientlist->AddBannedClient(GetConnectIP());
	SetUploadState(US_BANNED);
	theApp.emuledlg->transferwnd->ShowQueueCount();
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
bool CUpDownClient::IsDifferentPartBlock()
{ 
	Requested_Block_Struct* lastBlock;
	Requested_Block_Struct* currBlock;
	uint32 lastDone = 0;
	uint32 currRequested = 0;
	
	bool different = false;
	
	//try {
		// Check if we have good lists and proceed to check for different chunks
		if (GetSessionUp() >= 3145728 //Xman-Full-Chunk: Client is allowed to get min 3.0 MB
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
	TransferredData newSample = {m_nUpDatarateMeasure, ::GetTickCount()};
	m_upHistory_list.AddHead(newSample);

	// Keep up to 21 samples (=> 20 seconds)
	while(m_upHistory_list.GetCount() > 21){
		m_upHistory_list.RemoveTail();
	}

	if(m_upHistory_list.GetCount() > 1){	
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

	if(m_upHistory_list.GetCount() > 3){	
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
	ASSERT(DISPLAY_REFRESH_CLIENTLIST % DISPLAY_REFRESH == 0);
	if(m_displayUpDatarateCounter%DISPLAY_REFRESH == 0 ){
		theApp.emuledlg->transferwnd->uploadlistctrl.RefreshClient(this);
		if(GetDownloadState() != DS_DOWNLOADING)
			theApp.emuledlg->transferwnd->downloadclientsctrl.RefreshClient(this, reqfile);
	if(m_displayUpDatarateCounter%DISPLAY_REFRESH_CLIENTLIST == 0 ){
			if(GetDownloadState() != DS_DOWNLOADING){
		m_displayUpDatarateCounter = 0;
				m_lastRefreshedDLDisplay = ::GetTickCount();
			}
			if (socket)// X: [DSRB] - [Dynamic Send and Receive Buffer]
			{
				uint32 upDatarate = m_nUpDatarate >> 1;
				if(upDatarate > (uint32)sendbuffersize || (sendbuffersize != 6000 && upDatarate << 2 <= (uint32)sendbuffersize))
				{
					int newval;
					if(upDatarate <=5000)
						newval = 6000;
					else if(upDatarate <= 7*1024)
						newval = 8*1024;
					else if(upDatarate <= 15*1024)
						newval = 16*1024;
					else if(upDatarate <= 30*1024)
						newval = 32*1024;
					else if(upDatarate <= 61*1024)
						newval = 64*1024;
					else
						newval = 128*1024;
					if (newval != sendbuffersize)
					{
					socket->SetSockOpt(SO_SNDBUF, &newval, sizeof(newval), SOL_SOCKET);
#ifdef _DEBUG
					int newsendbuffersize = 0;
					int size = sizeof(sendbuffersize);
					socket->GetSockOpt(SO_SNDBUF, &newsendbuffersize, &size, SOL_SOCKET);
					if (newsendbuffersize == newval)
						theApp.QueueDebugLogLine(false, _T("Changed Sendbuffer from %uKB to %uKB, %uKB/s"), sendbuffersize/1024, newval/1024, m_nUpDatarate/1024);
					else
						theApp.QueueDebugLogLine(false, _T("Failed to change Sendbuffer, stays at %uKB, %uKB/s"), sendbuffersize/1024, m_nUpDatarate/1024);
#endif
					sendbuffersize = newval;
				}
			}
		}
	}
  }
}
// Maella end

// morph4u :: SOTN :: Start
void CUpDownClient::GetUploadingAndUploadedPart(CAtlArray<uint16>& arr, CAtlArray<uint16>& arrHidden)
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
// morph4u :: SOTN :: End

bool CUpDownClient::CheckDatarate(uint8 dataratestocheck)
{
	//Xman Xtreme Upload
	//look if a slot is over the wanted speed (+ tolerance) 
	if(m_upHistory_list.GetCount() > 4 && dataratestocheck >= 2)
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
		uint_ptr proofUpDatarate = (deltaTime > 0) ? (uint_ptr)(1000.0 * deltaByte / deltaTime) : 0;   // [bytes/s]
		//
		uint_ptr toleranceValue; 
		if(theApp.uploadqueue->currentuploadlistsize > (uint16)ceil(thePrefs.GetMaxUpload()/thePrefs.m_slotspeed) + 1 ) //we are 2 slots over MinSlots
			toleranceValue=(uint_ptr)(thePrefs.m_slotspeed*1024*1.33f); //33%
		else
			toleranceValue=(uint_ptr)(thePrefs.m_slotspeed*1024*1.25f); //25%
		if(socket)
		{
			if(socket->IsFull() && proofUpDatarate > toleranceValue) 
			{
				return true;
			}
			else if(socket->IsTrickle() 
				&& (proofUpDatarate > (thePrefs.m_slotspeed*1024/2) || proofUpDatarate > 2.5f*1024) 
				)
			{
				return true;
			}
		}
	}
	return false;
}

void CUpDownClient::AddUploadRate(UINT size)
{
	m_nUpDatarateMeasure += size; 
	m_nTransferredUp += size;
	credits->AddUploaded(size, GetIP());
}
//Xman end

//Xman
//MORPH START - Changed by SiRoB, ReadBlockFromFileThread
CReadBlockFromFileThread::CReadBlockFromFileThread(CKnownFile* pfile, uint64 startOffset, uint32 toread, CUpDownClient* client)
{
	srcfile = pfile;
	StartOffset = startOffset;
	togo = toread;
	m_client = client;
} 

bool CReadBlockFromFileThread::run() {
	//DbgSetThreadName("CReadBlockFromFileThread");

	if(CemuleDlg::IsRunning() == false)
		return true;

	/*// SLUGFILLER: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return;
	// SLUGFILLER: SafeHash*/
	//InitThreadLocale(); //Performance killer

	CFile file;
	byte* filedata = NULL;
	CMutexHelper lockFile;
	try{
		CString fullname;
		if (srcfile->IsPartFile() && ((CPartFile*)srcfile)->GetStatus() != PS_COMPLETE){
			lockFile.m_pMutex = &((CPartFile*)srcfile)->m_FileCompleteMutex;
			// If it's a part file which we are uploading the file remains locked until we've read the
			// current block. This way the file completion thread can not (try to) "move" the file into
			// the incoming directory.

			if(lockFile.m_pMutex->tryLock()) //we got the mutex at once
			{
				fullname = RemoveFileExtension(((CPartFile*)srcfile)->GetFullName());
			}
			else
			{
				if (!lockFile.m_pMutex->Lock(1000))
					throw GetResString(IDS_ERR_OPEN);
				//we waited for the mutex which means we maybe completed this file
				if(((CPartFile*)srcfile)->GetStatus() == PS_COMPLETE)
				{
					//everything was fine with completing
					fullname.Format(_T("%s\\%s"),srcfile->GetPath(),srcfile->GetFileName());
				}
				else
				{
					//an error occurred or the mutex was from other thread !?
					fullname = RemoveFileExtension(((CPartFile*)srcfile)->GetFullName());
				}
			}
			//Xman end

		}
		else{
			fullname.Format(_T("%s\\%s"),srcfile->GetPath(),srcfile->GetFileName());
		}

		if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
			throw GetResString(IDS_ERR_OPEN);

		file.Seek(StartOffset,0);

		filedata = new byte[togo+500];
		if (uint32 done = file.Read(filedata,togo) != togo){
			ASSERT(0);
			file.SeekToBegin();
			file.Read(filedata + done,togo-done);
		}
		file.Close();

		if (lockFile.m_pMutex){
			lockFile.m_pMutex->Unlock(); // unlock the (part) file as soon as we are done with accessing it.
			lockFile.m_pMutex = NULL;
		}

		if (theApp.emuledlg && CemuleDlg::IsRunning())
			PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE, (WPARAM)filedata,(LPARAM)m_client);
		else
			delete[] filedata;
		return true;
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), theApp.uploadqueue->IsDownloading(m_client)?m_client->GetUserName():_T(""), error); // client might not exist
	}
	catch(CFileException* e)
	{
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("Failed to create upload package for %s - %s"), theApp.uploadqueue->IsDownloading(m_client)?m_client->GetUserName():_T(""), szError);
		e->Delete();
	}
	catch(...)
	{
		}
		if (theApp.emuledlg && CemuleDlg::IsRunning())
			PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE,(WPARAM)-1,(LPARAM)m_client);
		else if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL)
			delete[] filedata;
	return true;
}
//MORPH END    - Changed by SiRoB, ReadBlockFromFileThread

void CReadBlockFromFileThread::setRemoved(void*file)
{
	if (srcfile == file)
		removed = true;
}

//Xman Xtreme Mod
// Close Backdoor v2 (idea Maella)
//after seeing that many official clients swap the file just when they get an uploadslot
//I decided to allow the upload if the new requested file
//has same or higher priority
//
// Remark: There is a security leak that a leecher mod could exploit here.
//         A client might send reqblock for another file than the one it 
//         was granted to download. As long as the file ID in reqblock
//         is the same in all reqblocks, it won't be rejected.  
//         With this a client might be in a waiting queue with a high 
//         priority but download block of a file set to a lower priority.
bool CUpDownClient::CloseBackdoor(const uchar* reqfilehash)
{
	CKnownFile* reqfileNr1 = theApp.sharedfiles->GetFileByID(reqfilehash);
	CKnownFile* reqfileNr2 = theApp.sharedfiles->GetFileByID(GetUploadFileID());
	if(reqfileNr1==NULL) 
	{
		//We don't know the requesting file, this can happen when we delete the file during upload
		//the prevent to run in a file exception when creating next block
		//send a cancel and remove client from queue
		Packet* packet = new Packet(OP_OUTOFPARTREQS, 0); 
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true, true);
		theApp.uploadqueue->RemoveFromUploadQueue(this,_T("Client requested unknown file"),CUpDownClient::USR_DIFFERENT_FILE); // Maella -Upload Stop Reason-
		SetUploadFileID(NULL); 
		return true;
	}							
	if(reqfileNr2!=NULL && reqfileNr1->GetUpPriorityEx()  < reqfileNr2->GetUpPriorityEx())
	{
		if(thePrefs.GetLogUlDlEvents()){
			AddDebugLogLine(false, _T("--> Upload session ended due wrong requested FileID (client=%s, expected=%s, asked=%s)"), 
				GetUserName(),reqfileNr2->GetFileName(), reqfileNr1->GetFileName());
		}
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("wrong file"), CUpDownClient::USR_DIFFERENT_FILE,true); // Maella -Upload Stop Reason-
		SetUploadFileID(reqfileNr1); //Xman Fix!  (needed for see onUploadqueue)
		SetLastAction(OP_STARTUPLOADREQ); //Xman fix for startupload  special case!
		SendOutOfPartReqsAndAddToWaitingQueue();
		SetWaitStartTime(); // Penality (soft punishement)
		return true;
	}
	if(reqfileNr2!=reqfileNr1)
	{
		// Maella -Filter verbose messages-
		if(thePrefs.GetLogUlDlEvents()){
			AddDebugLogLine(false, _T("--> we allow to upload different file: (client=%s, expected=%s, asked=%s)"), 
				GetUserName(),reqfileNr2->GetFileName(), reqfileNr1->GetFileName());
		}
	}
	return false;
}
	//Xman end

// ==> Pay Back First
bool CUpDownClient::IsPBFClient() const
{
	CKnownFile* currentReqFile = theApp.sharedfiles->GetFileByID(GetUploadFileID());

    // only clients requesting a valid file can be superior
	if(currentReqFile == NULL)
		return false;

	// no bad guys!
	if(GetUploadState()==US_BANNED || m_bGPLEvildoer)
		return false;

	if(credits)
	{
		bool bIsSecure = theApp.clientcredits->CryptoAvailable() && credits->GetCurrentIdentState(GetIP()) == IS_IDENTIFIED;
		if(credits->GetPayBackFirstStatus() && thePrefs.IsPayBackFirst() && bIsSecure)
			return true;
	}

	return false;
}

bool CUpDownClient::IsSecure() const
{
	return credits && theApp.clientcredits->CryptoAvailable() && credits->GetCurrentIdentState(GetIP()) == IS_IDENTIFIED;
}
// <== Pay Back First