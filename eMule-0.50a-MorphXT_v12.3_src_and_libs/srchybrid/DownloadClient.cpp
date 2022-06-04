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
#include "PartFile.h"
#include "OtherFunctions.h"
#include "ListenSocket.h"
#include "PeerCacheSocket.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Packets.h"
#include "Statistics.h"
#include "ClientCredits.h"
#include "DownloadQueue.h"
#include "ClientUDPSocket.h"
#include "emuledlg.h"
#include "TransferDlg.h"
#include "PeerCacheFinder.h"
#include "Exceptions.h"
#include "clientlist.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/Kademlia/Search.h"
#include "SHAHashSet.h"
#include "SharedFileList.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//	members of CUpDownClient
//	which are mainly used for downloading functions 
CBarShader CUpDownClient::s_StatusBar(16);
//MORPH - Changed by SiRoB, Keep A4AF infos
/*
void CUpDownClient::DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
*/
void CUpDownClient::DrawStatusBar(CDC* dc, LPCRECT rect,const CPartFile* file, bool  bFlat) const
//MORPH - Changed by SiRoB, Keep A4AF infos
{
	if (g_bLowColorDesktop)
		bFlat = true;

	COLORREF crNeither;
	if (bFlat) {
		if (g_bLowColorDesktop)
			crNeither = RGB(192, 192, 192);
		else
			crNeither = RGB(224, 224, 224);
	} else {
		crNeither = RGB(240, 240, 240);
	}

	
	//MORPH START - Changed by SiRoB, Keep A4AF infos
	/*
	ASSERT(reqfile);
	s_StatusBar.SetFileSize(reqfile->GetFileSize());
	*/
	ASSERT(file);
	s_StatusBar.SetFileSize(file->GetFileSize()); 
	//MORPH END   - Changed by SiRoB, Keep A4AF infos
	s_StatusBar.SetHeight(rect->bottom - rect->top);
	s_StatusBar.SetWidth(rect->right - rect->left);
	s_StatusBar.Fill(crNeither);

	//MORPH START - Changed by SiRoB, Keep A4AF infos
	/*
	if (!onlygreyrect && reqfile && m_abyPartStatus)
	*/
	uint8* thisAbyPartStatus;
	if(m_PartStatus_list.Lookup(file,thisAbyPartStatus) && thisAbyPartStatus)
	{
		COLORREF crBoth;
		COLORREF crClientOnly;
		COLORREF crPending;
		COLORREF crNextPending;
		COLORREF crMeOnly; //MORPH - Added by IceCream--- xrmb:seeTheNeed ---
		COLORREF crClientPartial; // enkeyDev: ICS //Morph - added by AndCycle, ICS
		if (g_bLowColorDesktop) {
			crBoth = RGB(0, 0, 0);
			crClientOnly = RGB(0, 0, 255);
			crPending = RGB(0, 191, 0); // was RGB(0, 255, 0);
			crNextPending = RGB(255, 255, 0);
			crMeOnly = RGB(127,127,127); //MORPH - Added by IceCream--- xrmb:seeTheNeed ---
			crClientPartial = RGB(255,191,0); // enkeyDev: ICS //Morph - added by AndCycle, ICS
		} else if (bFlat) {
			crBoth = RGB(0, 0, 0);
			crClientOnly = RGB(0, 100, 255);
			crPending = RGB(0, 150, 0);
			crNextPending = RGB(255, 208, 0);
			crMeOnly = RGB(171,171,171); //MORPH - Added by IceCream--- xrmb:seeTheNeed ---
			crClientPartial = RGB(255,191,255); // enkeyDev: ICS //Morph - added by AndCycle, ICS
		} else {
			crBoth = RGB(104, 104, 104);
			crClientOnly = RGB(0, 100, 255);
			crPending = RGB(0, 150, 0);
			crNextPending = RGB(255, 208, 0);
			crMeOnly = RGB(172,172,172); //MORPH - Added by IceCream--- xrmb:seeTheNeed ---
			crClientPartial = RGB(255,191,255); // enkeyDev: ICS //Morph - added by AndCycle, ICS
		}

		char* pcNextPendingBlks = NULL;
		//MORPH START - change
		/*
		if (m_nDownloadState == DS_DOWNLOADING){
		*/
		if (m_nDownloadState == DS_DOWNLOADING && file == reqfile){
		//MORPH END   - change
			pcNextPendingBlks = new char[m_nPartCount];
			memset(pcNextPendingBlks, 'N', m_nPartCount); // do not use '_strnset' for uninitialized memory!
			for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != 0; ){
				UINT uPart = (UINT)(m_PendingBlocks_list.GetNext(pos)->block->StartOffset / PARTSIZE);
				if (uPart < m_nPartCount)
					pcNextPendingBlks[uPart] = 'Y';
			}
		}

		//MORPH START - change
		/*
		for (UINT i = 0; i < m_nPartCount; i++){
			if (m_abyPartStatus[i]){
				uint64 uEnd;
				if ( PARTSIZE*(uint64)(i+1) > reqfile->GetFileSize())
					uEnd = reqfile->GetFileSize();
				else
					uEnd = PARTSIZE*(uint64)(i+1);

				if (reqfile->IsComplete(PARTSIZE*(uint64)i,PARTSIZE*(uint64)(i+1)-1, false))
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crBoth);
				else if (GetSessionDown() > 0 && m_nDownloadState == DS_DOWNLOADING && m_nLastBlockOffset >= PARTSIZE*(uint64)i && m_nLastBlockOffset < uEnd)
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crPending);
				else if (pcNextPendingBlks != NULL && pcNextPendingBlks[i] == 'Y')
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crNextPending);
		*/
		for (uint16 i = 0; i < file->GetPartCount(); i++){
			if (thisAbyPartStatus[i]&SC_AVAILABLE){
				uint64 uEnd = PARTSIZE*(uint64)(i+1);
				if ( uEnd > file->GetFileSize())
					uEnd = file->GetFileSize();
				if (file->IsPartShareable(i))
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crBoth);
				else if (m_nDownloadState == DS_DOWNLOADING && file == reqfile) {
					if (m_nLastBlockOffset >= PARTSIZE*(uint64)i && m_nLastBlockOffset < uEnd)
						s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crPending);
					else if (pcNextPendingBlks != NULL && pcNextPendingBlks[i] == 'Y')
						s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crNextPending);
					else
						s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crClientOnly);
				}
		//MORPH END   - change
				else
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crClientOnly);
			} 
			//MORPH - Added by IceCream --- xrmb:seeTheNeed ---
			//MORPH - Changed by SiRoB, use the shareable info faster
			/*
			else if (file->IsComplete(PARTSIZE*(uint64)i,PARTSIZE*(uint64)(i+1)-1, true)){ 
			*/
			else if (file->IsPartShareable(i)) {
				uint64 uEnd = PARTSIZE*(uint64)(i+1);
				if (uEnd > file->GetFileSize()) 
					uEnd = file->GetFileSize(); 
				s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crMeOnly);
			}
			//MORPH START - Added by SiRoB, ICS merged into parstatus
			else if (thisAbyPartStatus[i]==SC_PARTIAL) {
				uint64 uEnd = PARTSIZE*(uint64)(i+1);
				if (uEnd > file->GetFileSize()) 
					uEnd = file->GetFileSize(); 
				s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crClientPartial);
			}
			//MORPH END   - Added by SiRoB, ICS merged into parstatus
		}
		delete[] pcNextPendingBlks;
	} 
	//MORPH END   - Changed by SiRoB, Keep A4AF infos
	s_StatusBar.Draw(dc, rect->left, rect->top, bFlat); 
} 
//MORPH START - Added by SiRoB, Downloading Chunk Detail Display
void CUpDownClient::DrawStatusBarChunk(CDC* dc, LPCRECT rect,const CPartFile* file, bool  bFlat) const
{ 
	if (m_PendingBlocks_list.IsEmpty() && m_DownloadBlocks_list.IsEmpty() || file != reqfile || reqfile == NULL) 
		return;
	if (g_bLowColorDesktop)
		bFlat = true;
	COLORREF crBoth;
	COLORREF crClientOnly;
	COLORREF crPending;
	COLORREF crNextPending;
	COLORREF crNeither;
	COLORREF crBuffer;
	COLORREF crProgress;
	COLORREF crDot;
	if (g_bLowColorDesktop) {
		crBoth = RGB(0, 0, 0);
		crClientOnly = RGB(0, 0, 255);
		crPending = RGB(0, 191, 0);
		crNextPending = RGB(255, 255, 0);
		crNeither = RGB(191, 191, 191);
		crBuffer = RGB(255, 0, 0);
		crProgress = RGB(0, 255, 0);
		crDot = RGB(255, 255, 255);
	} else if (bFlat) {
		crBoth = RGB(0, 0, 0);
		crClientOnly = RGB(0, 100, 255);
		crPending = RGB(0, 150, 0);
		crNextPending = RGB(255, 208, 0);
		crNeither = RGB(224, 224, 224);
		crBuffer = RGB(255, 0, 0);
		crProgress = RGB(0, 224, 0);
		crDot = RGB(255, 255, 255);
	} else {
		crBoth = RGB(104, 104, 104);
		crClientOnly = RGB(0, 100, 255);
		crPending = RGB(0, 150, 0);
		crNextPending = RGB(255, 208, 0);
		crNeither = RGB(240, 240, 240);
		crBuffer = RGB(255, 100, 100);
		crProgress = RGB(0, 224, 0);
		crDot = RGB(255, 255, 255);
	}

	
	s_StatusBar.SetHeight(rect->bottom - rect->top);
	s_StatusBar.SetWidth(rect->right - rect->left);
	
	UINT cur_chunk = (uint32)-1;
	uint64 start = (uint64)-1;
	uint64 end = (uint64)-1;
	uint64 chunksize = PARTSIZE;
	const Requested_Block_Struct* block;
	//Find Sent Requested part
	if (!m_PendingBlocks_list.IsEmpty()) {
		block = m_PendingBlocks_list.GetHead()->block;
		cur_chunk = (UINT)(block->StartOffset / PARTSIZE);
		start = end = PARTSIZE*(uint64)cur_chunk;
		end += PARTSIZE-1;
		if ( end > file->GetFileSize()) {
			end = file->GetFileSize();
			chunksize = end - start;
			if(chunksize <= 0) chunksize = PARTSIZE;
		}
		//s_StatusBar.Fill(crProgress);
	}

	//Find reserved block
	if (!m_DownloadBlocks_list.IsEmpty()){
		block = m_DownloadBlocks_list.GetHead();
		if (cur_chunk == (uint32)-1) {
			cur_chunk = (uint32)(block->StartOffset/PARTSIZE);
			start = end = cur_chunk*PARTSIZE;
			end += PARTSIZE-1;
			if (end > file->GetFileSize()) {
				end = file->GetFileSize();
				chunksize = end - start;
				if(chunksize <= 0) chunksize = PARTSIZE;
			}
			//s_StatusBar.Fill(crProgress);
		}
	}

	s_StatusBar.SetFileSize(chunksize); 
	s_StatusBar.Fill(crProgress); // We would do it any way, if we left it in one of the above blocks.

	if (cur_chunk == (uint32)-1)
		return;

	// Draw Gap part of the chunk in blue for normal client and none for proxy
	for (POSITION pos = file->gaplist.GetHeadPosition();pos !=  0;){
		const Gap_Struct* cur_gap = file->gaplist.GetNext(pos);
		uint64 gapstart = cur_gap->start;
		uint64 gapend = cur_gap->end;
		if (gapstart <= end && gapend >= start) {
			s_StatusBar.FillRange((gapstart>start)?gapstart%PARTSIZE:(uint64)0, ((gapend<end)?(gapend+1):end)%PARTSIZE, crClientOnly);
		}
	}
	
	if (!m_DownloadBlocks_list.IsEmpty()){
		//Fill with white Block reserved but not requested yet
		for(POSITION pos=m_DownloadBlocks_list.GetHeadPosition();pos!=0;){
			block = m_DownloadBlocks_list.GetNext(pos);
			if (block->StartOffset <= end && block->EndOffset >= start) {
				s_StatusBar.FillRange((block->StartOffset>start)?block->StartOffset%PARTSIZE:0, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crDot);
			}
		}
	}

	if (!m_PendingBlocks_list.IsEmpty()){
		//Fill with red block part not received yet for the current recieving block
		//Fill with darkgreen block part received for the current receiving block
		//Fill with yellow block requested
		for(POSITION pos=m_PendingBlocks_list.GetTailPosition();pos!=0; ){
			Pending_Block_Struct* Pending = m_PendingBlocks_list.GetPrev(pos);
			block = Pending->block;
			if (block->StartOffset <= end && block->EndOffset >= start) {
				uint64 currentpos = m_nLastBlockOffset+Pending->totalUnzipped;
				if(currentpos <= block->StartOffset) {
					// block have not been started yet
					s_StatusBar.FillRange((block->StartOffset>start)?block->StartOffset%PARTSIZE:0, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crNextPending);
				}
				else if (currentpos > block->EndOffset) {
					// block have not been started yet
					s_StatusBar.FillRange((block->StartOffset>start)?block->StartOffset%PARTSIZE:0, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crNextPending);
				}
				else {
					uint64 newEnd = currentpos;
					// block partly received or partly in buffer
					if (newEnd>start) {
						if (newEnd<end) {
							s_StatusBar.FillRange((uint64)(block->StartOffset>start)?block->StartOffset%PARTSIZE:0, newEnd%PARTSIZE, crPending);
							s_StatusBar.FillRange((uint64)newEnd%PARTSIZE, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crBuffer);
						} else {
							s_StatusBar.FillRange((uint64)(block->StartOffset>start)?block->StartOffset%PARTSIZE:0, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crPending);
						}
					} else {
						s_StatusBar.FillRange((uint64)0, ((block->EndOffset < end)?block->EndOffset+1:end)%PARTSIZE, crNextPending);
					}
				}
			}
		}
	}
	s_StatusBar.Draw(dc, rect->left, rect->top, bFlat);

	if(thePrefs.m_bEnableChunkDots){
		s_StatusBar.SetHeight(3); 
		s_StatusBar.SetWidth(1); 
		s_StatusBar.SetFileSize((uint64)1);
		s_StatusBar.Fill(crDot);
		uint32	w=rect->right-rect->left+1;
		if (!m_DownloadBlocks_list.IsEmpty()){
			for(POSITION pos=m_DownloadBlocks_list.GetHeadPosition();pos!=0;){
				block = m_DownloadBlocks_list.GetNext(pos);
				if (block->StartOffset <= end && block->EndOffset >= start) {
					if (block->StartOffset >= start) {
						if (block->EndOffset <= end) {
							s_StatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
							s_StatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
						} else
							s_StatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
					} else if (block->EndOffset <= end)
						s_StatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
				}
			}
		}
		if (!m_PendingBlocks_list.IsEmpty()){
			for(POSITION pos=m_PendingBlocks_list.GetHeadPosition();pos!=0;){
				block = m_PendingBlocks_list.GetNext(pos)->block;
				if (block->StartOffset <= end && block->EndOffset >= start) {
					if (block->StartOffset >= start) {
						if (block->EndOffset <= end) {
							s_StatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
							s_StatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
						} else
							s_StatusBar.Draw(dc,rect->left+(int)((double)(block->StartOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
					} else if (block->EndOffset <= end)
						s_StatusBar.Draw(dc,rect->left+(int)((double)(block->EndOffset%PARTSIZE)*w/chunksize), rect->top, bFlat);
				}
			}
		}
	}
} 

float CUpDownClient::GetDownChunkProgressPercent() const
{
	if (m_PendingBlocks_list.IsEmpty() || reqfile == NULL) 
		return 0.0f;
	UINT cur_chunk = (uint32)-1;
	uint64 start = (uint64)-1;
	uint64 end = (uint64)-1;
	uint64 chunksize = PARTSIZE;
	const Requested_Block_Struct* block;

	//Find Sent Requested part
	block = m_PendingBlocks_list.GetHead()->block;
	cur_chunk = (UINT)(block->StartOffset / PARTSIZE);
	start = end = PARTSIZE*(uint64)cur_chunk;
	end += PARTSIZE-1;
	if ( end > reqfile->GetFileSize()) {
		end = reqfile->GetFileSize();
		chunksize = end - start;
		if(chunksize <= 0) chunksize = PARTSIZE;
	}
	return (float)(((double)(block->EndOffset%PARTSIZE)/(double)chunksize)*100.0f);
}
//MORPH END - Added by SiRoB, Downloading Chunk Detail Display

bool CUpDownClient::Compare(const CUpDownClient* tocomp, bool bIgnoreUserhash) const
{
	//Compare only the user hash..
	if(!bIgnoreUserhash && HasValidHash() && tocomp->HasValidHash())
	    return !md4cmp(this->GetUserHash(), tocomp->GetUserHash());

	if (HasLowID())
	{
		//User is firewalled.. Must do two checks..
		if (GetIP()!=0	&& GetIP() == tocomp->GetIP())
		{
			//The IP of both match
            if (GetUserPort()!=0 && GetUserPort() == tocomp->GetUserPort())
				//IP-UserPort matches
                return true;
			if (GetKadPort()!=0	&& GetKadPort() == tocomp->GetKadPort())
				//IP-KadPort Matches
				return true;
		}
        if (GetUserIDHybrid()!=0
			&& GetUserIDHybrid() == tocomp->GetUserIDHybrid()
			&& GetServerIP()!=0
			&& GetServerIP() == tocomp->GetServerIP()
			&& GetServerPort()!=0
			&& GetServerPort() == tocomp->GetServerPort())
			//Both have the same lowID, Same serverIP and Port..
            return true;

#if defined(_DEBUG)
		if ( HasValidBuddyID() && tocomp->HasValidBuddyID() )
		{
			//JOHNTODO: This is for future use to see if this will be needed...
			if(!md4cmp(GetBuddyID(), tocomp->GetBuddyID()))
				return true;
		}
#endif

		//Both IP, and Server do not match..
		return false;
    }

	//User is not firewalled.
    if (GetUserPort()!=0)
	{
		//User has a Port, lets check the rest.
		if (GetIP() != 0 && tocomp->GetIP() != 0)
		{
			//Both clients have a verified IP..
			if(GetIP() == tocomp->GetIP() && GetUserPort() == tocomp->GetUserPort())
				//IP and UserPort match..
				return true;
		}
		else
		{
			//One of the two clients do not have a verified IP
			if (GetUserIDHybrid() == tocomp->GetUserIDHybrid() && GetUserPort() == tocomp->GetUserPort())
				//ID and Port Match..
                return true;
		}
    }
	if(GetKadPort()!=0)
	{
		//User has a Kad Port.
		if(GetIP() != 0 && tocomp->GetIP() != 0)
		{
			//Both clients have a verified IP.
			if(GetIP() == tocomp->GetIP() && GetKadPort() == tocomp->GetKadPort())
				//IP and KadPort Match..
				return true;
		}
		else
		{
			//One of the users do not have a verified IP.
            if (GetUserIDHybrid() == tocomp->GetUserIDHybrid() && GetKadPort() == tocomp->GetKadPort())
				//ID and KadProt Match..
                return true;
		}
	}
	//No Matches..
	return false;
}

// Return bool is not if you asked or not..
// false = Client was deleted!
// true = client was not deleted!
bool CUpDownClient::AskForDownload()
{

	if (m_bUDPPending)
	{
		m_nFailedUDPPackets++;
		theApp.downloadqueue->AddFailedUDPFileReasks();
	}
	m_bUDPPending = false;
	if (!(socket && socket->IsConnected())) // already connected, skip all the special checks
	{
		if (theApp.listensocket->TooManySockets())
		{
			if (GetDownloadState() != DS_TOOMANYCONNS)
				SetDownloadState(DS_TOOMANYCONNS);
			return true;
		}
		m_dwLastTriedToConnect = ::GetTickCount();
		// if its a lowid client which is on our queue we may delay the reask up to 20 min, to give the lowid the chance to
		// connect to us for its own reask
		if (HasLowID() && GetUploadState() == US_ONUPLOADQUEUE && !m_bReaskPending && GetLastAskedTime() > 0){
			SetDownloadState(DS_ONQUEUE);
			m_bReaskPending = true;
			return true;
		}
		// if we are lowid <-> lowid but contacted the source before already, keep it in the hope that we might turn highid again
		if (HasLowID() && !theApp.CanDoCallback(this) && GetLastAskedTime() > 0){
			if (GetDownloadState() != DS_LOWTOLOWIP)
				SetDownloadState(DS_LOWTOLOWIP);
			m_bReaskPending = true;
			return true;
		}
	}

	m_dwLastTriedToConnect = ::GetTickCount();
	//MORPH START - Added by schnulli900, count failed TCP/IP connections [Xman]
	if(m_cFailed==0)
		theApp.downloadqueue->AddTCPFileReask();
	//MORPH END   - Added by schnulli900, count failed TCP/IP connections [Xman]

    SwapToAnotherFile(_T("A4AF check before tcp file reask. CUpDownClient::AskForDownload()"), true, false, false, NULL, true, true);
	uiDLAskingCounter +=1; //SLAHAM: ADDED Last Asked Counter
	SetDownloadState(DS_CONNECTING);
	return TryToConnect();
}

bool CUpDownClient::IsSourceRequestAllowed() const
{
    return IsSourceRequestAllowed(reqfile);
}

bool CUpDownClient::IsSourceRequestAllowed(CPartFile* partfile, bool sourceExchangeCheck) const
{
	DWORD dwTickCount = ::GetTickCount() + CONNECTION_LATENCY;
	unsigned int nTimePassedClient = dwTickCount - GetLastSrcAnswerTime();
	unsigned int nTimePassedFile   = dwTickCount - partfile->GetLastAnsweredTime();
	bool bNeverAskedBefore = GetLastAskedForSources() == 0;

	//MORPH START - Added by Stulle, Source cache [Xman]
	// let's search for more sources a bit earlier
	if(partfile->GetSourceCacheAmount()>1)
		return false;
	//MORPH END   - Added by Stulle, Source cache [Xman]

	//MORPH START - Added by Stulle, Global Source Limit
	if(partfile->IsSrcReqOrAddAllowed() == false)
		return false;
	//MORPH END   - Added by Stulle, Global Source Limit

	UINT uSources = partfile->GetSourceCount();
    UINT uValidSources = partfile->GetValidSourcesCount();

    if (partfile != reqfile) {
        uSources++;
        uValidSources++;
    }

    UINT uReqValidSources = reqfile->GetValidSourcesCount();

	return (
	         //if client has the correct extended protocol
			 ExtProtocolAvailable() && (SupportsSourceExchange2() || GetSourceExchange1Version() > 1) &&
	         //AND if we need more sources
	         reqfile->GetMaxSourcePerFileSoft() > uSources &&
	         //AND if...
	         (
	           //source is not complete and file is very rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     && (uSources <= RARE_FILE/5)
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < uReqValidSources && uReqValidSources > 3)
	           ) ||
	           //source is not complete and file is rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     && (uSources <= RARE_FILE || (!sourceExchangeCheck || partfile == reqfile) && uSources <= RARE_FILE / 2 + uValidSources)
				 && (nTimePassedFile > SOURCECLIENTREASKF)
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < SOURCECLIENTREASKS/SOURCECLIENTREASKF && uValidSources < uReqValidSources)
	           ) ||
	           // OR if file is not rare
			   ( (bNeverAskedBefore || nTimePassedClient > (unsigned)(SOURCECLIENTREASKS * MINCOMMONPENALTY)) 
				 && (nTimePassedFile > (unsigned)(SOURCECLIENTREASKF * MINCOMMONPENALTY))
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < SOURCECLIENTREASKS/SOURCECLIENTREASKF && uValidSources < uReqValidSources)
	           )
	         )
	       );
}

void CUpDownClient::SendFileRequest()
{
    // normally asktime has already been reset here, but then SwapToAnotherFile will return without much work, so check to make sure
    SwapToAnotherFile(_T("A4AF check before tcp file reask. CUpDownClient::SendFileRequest()"), true, false, false, NULL, true, true);

	ASSERT(reqfile != NULL);
	if (!reqfile)
		return;
	AddAskedCountDown();

	if (SupportMultiPacket() || SupportsFileIdentifiers())
	{
		CSafeMemFile dataFileReq(96);
		if (SupportsFileIdentifiers())
		{
			reqfile->GetFileIdentifier().WriteIdentifier(&dataFileReq);
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__MultiPacket_Ext2", this, reqfile->GetFileHash());			
		}
		else
		{
			dataFileReq.WriteHash16(reqfile->GetFileHash());
			if (SupportExtMultiPacket()){
				dataFileReq.WriteUInt64(reqfile->GetFileSize());
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__MultiPacket_Ext", this, reqfile->GetFileHash());
			}
			else{
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__MultiPacket", this, reqfile->GetFileHash());
			}
		}

		// OP_REQUESTFILENAME + ExtInfo
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__MPReqFileName", this, reqfile->GetFileHash());
		dataFileReq.WriteUInt8(OP_REQUESTFILENAME);
		if (GetExtendedRequestsVersion() > 0)
			reqfile->WritePartStatus(&dataFileReq);
		if (GetExtendedRequestsVersion() > 1)
			reqfile->WriteCompleteSourcesCount(&dataFileReq);

		// OP_SETREQFILEID
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__MPSetReqFileID", this, reqfile->GetFileHash());
		if (reqfile->GetPartCount() > 1)
			dataFileReq.WriteUInt8(OP_SETREQFILEID);

		if (IsEmuleClient())
		{
			SetRemoteQueueFull(true);
			SetRemoteQueueRank(0);
		}

		// OP_REQUESTSOURCES // OP_REQUESTSOURCES2
		if (IsSourceRequestAllowed())
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0) {
				DebugSend("OP__MPReqSources", this, reqfile->GetFileHash());
			    if (GetLastAskedForSources() == 0)
				    Debug(_T("  first source request\n"));
			    else
				    Debug(_T("  last source request was before %s\n"), CastSecondsToHM((GetTickCount() - GetLastAskedForSources())/1000));
			}
			if (SupportsSourceExchange2()){
				dataFileReq.WriteUInt8(OP_REQUESTSOURCES2);
				dataFileReq.WriteUInt8(SOURCEEXCHANGE2_VERSION);
				const uint16 nOptions = 0; // 16 ... Reserved
				dataFileReq.WriteUInt16(nOptions);
			}
			else{
				dataFileReq.WriteUInt8(OP_REQUESTSOURCES);
			}
			reqfile->SetLastAnsweredTimeTimeout();
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine(false, _T("SXSend (%s): Client source request; %s, File=\"%s\""),SupportsSourceExchange2() ? _T("Version 2") : _T("Version 1"), DbgGetClientInfo(), reqfile->GetFileName());
        }

		// OP_AICHFILEHASHREQ - deprecated with fileidentifiers
		if (IsSupportingAICH() && !SupportsFileIdentifiers())
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__MPAichFileHashReq", this, reqfile->GetFileHash());
			dataFileReq.WriteUInt8(OP_AICHFILEHASHREQ);
		}

		Packet* packet = new Packet(&dataFileReq, OP_EMULEPROT);
		if (SupportsFileIdentifiers())
			packet->opcode = OP_MULTIPACKET_EXT2;
		else if (SupportExtMultiPacket())
			packet->opcode = OP_MULTIPACKET_EXT;
		else
			packet->opcode = OP_MULTIPACKET;
		theStats.AddUpDataOverheadFileRequest(packet->size);
		SendPacket(packet, true);
	}
	else
	{
		CSafeMemFile dataFileReq(96);
		dataFileReq.WriteHash16(reqfile->GetFileHash());
		//This is extended information
		if (GetExtendedRequestsVersion() > 0)
			reqfile->WritePartStatus(&dataFileReq);
		if (GetExtendedRequestsVersion() > 1)
			reqfile->WriteCompleteSourcesCount(&dataFileReq);
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__FileRequest", this, reqfile->GetFileHash());
		Packet* packet = new Packet(&dataFileReq);
		packet->opcode = OP_REQUESTFILENAME;
		theStats.AddUpDataOverheadFileRequest(packet->size);
		SendPacket(packet, true);
	
		// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
		// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
		// know that the file is shared, we know also that the file is complete and don't need to request the file status.
		if (reqfile->GetPartCount() > 1)
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__SetReqFileID", this, reqfile->GetFileHash());
		    CSafeMemFile dataSetReqFileID(16);
			dataSetReqFileID.WriteHash16(reqfile->GetFileHash());
		    packet = new Packet(&dataSetReqFileID);
		    packet->opcode = OP_SETREQFILEID;
		    theStats.AddUpDataOverheadFileRequest(packet->size);
		   SendPacket(packet, true);
		}
	
		if (IsEmuleClient())
		{
			SetRemoteQueueFull(true);
			SetRemoteQueueRank(0);
		}

		if (IsSourceRequestAllowed())
		{
		    if (thePrefs.GetDebugClientTCPLevel() > 0) {
			    DebugSend("OP__RequestSources", this, reqfile->GetFileHash());
			    if (GetLastAskedForSources() == 0)
				    Debug(_T("  first source request\n"));
			    else
				    Debug(_T("  last source request was before %s\n"), CastSecondsToHM((GetTickCount() - GetLastAskedForSources())/1000));
		    }
			reqfile->SetLastAnsweredTimeTimeout();
			
			Packet* packet;
			if (SupportsSourceExchange2()){
				packet = new Packet(OP_REQUESTSOURCES2,19,OP_EMULEPROT);
				PokeUInt8(&packet->pBuffer[0], SOURCEEXCHANGE2_VERSION);
				const uint16 nOptions = 0; // 16 ... Reserved
				PokeUInt16(&packet->pBuffer[1], nOptions);
				md4cpy(&packet->pBuffer[3],reqfile->GetFileHash());
			}
			else{
				packet = new Packet(OP_REQUESTSOURCES,16,OP_EMULEPROT);
				md4cpy(packet->pBuffer,reqfile->GetFileHash());
			}

			theStats.AddUpDataOverheadSourceExchange(packet->size);
			SendPacket(packet, true);
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine(false, _T("SXSend (%s): Client source request; %s, File=\"%s\""),SupportsSourceExchange2() ? _T("Version 2") : _T("Version 1"), DbgGetClientInfo(), reqfile->GetFileName());
        }

		if (IsSupportingAICH())
		{
		    if (thePrefs.GetDebugClientTCPLevel() > 0)
			    DebugSend("OP__AichFileHashReq", this, reqfile->GetFileHash());
			Packet* packet = new Packet(OP_AICHFILEHASHREQ,16,OP_EMULEPROT);
			md4cpy(packet->pBuffer,reqfile->GetFileHash());
			theStats.AddUpDataOverheadFileRequest(packet->size);
			SendPacket(packet, true);
		}
	}
    SetLastAskedTime();
}

void CUpDownClient::SendStartupLoadReq()
{
	if (socket==NULL || reqfile==NULL)
	{
		ASSERT(0);
		return;
	}
	m_fQueueRankPending = 1;
	m_fUnaskQueueRankRecv = 0;
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__StartupLoadReq", this);
	CSafeMemFile dataStartupLoadReq(16);
	dataStartupLoadReq.WriteHash16(reqfile->GetFileHash());
	Packet* packet = new Packet(&dataStartupLoadReq);
	packet->opcode = OP_STARTUPLOADREQ;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	SetDownloadState(DS_ONQUEUE);
	SendPacket(packet, true);
}

void CUpDownClient::ProcessFileInfo(CSafeMemFile* data, CPartFile* file)
{
	if (file==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; file==NULL)");
	if (reqfile==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile==NULL)");
	if (file != reqfile)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile!=file)");
	m_strClientFilename = data->ReadString(GetUnicodeSupport()!=utf8strNone);
	reqfile->UpdateSourceFileName(this); // EastShare       - FollowTheMajority by AndCycle
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  Filename=\"%s\"\n"), m_strClientFilename);
	// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
	// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
	// know that the file is shared, we know also that the file is complete and don't need to request the file status.
	if (reqfile->GetPartCount() == 1)
	{
		//MORPH START - Changed by SiRoB,  Keep A4AF infos
		/*
		delete[] m_abyPartStatus;
		m_abyPartStatus = NULL;
		*/
		uint8* thisAbyPartStatus;
		if(m_PartStatus_list.Lookup(reqfile, thisAbyPartStatus))
		{
			delete[] thisAbyPartStatus;
			m_PartStatus_list.RemoveKey(reqfile);
		}
		m_abyPartStatus = NULL;
		//MORPH   END - Changed by SiRoB, Keep A4AF infos
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		//MORPH START - Added by SiRoB, Keep A4AF infos
		m_PartStatus_list.SetAt(reqfile,m_abyPartStatus);
		//MORPH END   - Added by SiRoB, Keep A4AF infos
		//MORPH - Changed by SiRoB, ICS merged into partstatus
		/*
		memset(m_abyPartStatus,1,m_nPartCount);
		*/
		memset(m_abyPartStatus,SC_AVAILABLE,m_nPartCount);
		m_bCompleteSource = true;

		if (thePrefs.GetDebugClientTCPLevel() > 0)
		{
		    int iNeeded = 0;
			UINT i;
			for (i = 0; i < m_nPartCount; i++) {
			    if (!reqfile->IsComplete((uint64)i*PARTSIZE, ((uint64)(i+1)*PARTSIZE)-1, false))
				    iNeeded++;
			}
			char* psz = new char[m_nPartCount + 1];
			for (i = 0; i < m_nPartCount; i++)
				//MORPH START - Changed by SiRoB, ICS merged into partstatus
				/*
				psz[i] = m_abyPartStatus[i] ? '#' : '.';
				*/
				psz[i] = (m_abyPartStatus[i]&SC_AVAILABLE) ? '#' : '.';
				//MORPH END   - Changed by SiRoB, ICS merged into partstatus
			psz[i] = '\0';
			Debug(_T("  Parts=%u  %hs  Needed=%u\n"), m_nPartCount, psz, iNeeded);
			delete[] psz;
		}
		UpdateDisplayedInfo();
		reqfile->UpdateAvailablePartsCount();
		// even if the file is <= PARTSIZE, we _may_ need the hashset for that file (if the file size == PARTSIZE)
		if (reqfile->m_bMD4HashsetNeeded || (reqfile->IsAICHPartHashSetNeeded() && SupportsFileIdentifiers() 
			&& GetReqFileAICHHash() != NULL && *GetReqFileAICHHash() == reqfile->GetFileIdentifier().GetAICHHash()))
			SendHashSetRequest();
		else
			SendStartupLoadReq();
		reqfile->UpdatePartsInfo();
	}
}

void CUpDownClient::ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file)
{
	if ( !reqfile || file != reqfile )
	{
		if (reqfile==NULL)
			throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile==NULL)");
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile!=file)");
	}
	uint16 nED2KPartCount = data->ReadUInt16();

	//MORPH START - Added by SiRoB, Keep A4AF infos
	/*
	delete[] m_abyPartStatus;
	m_abyPartStatus = NULL;
	*/
	uint8* thisAbyPartStatus;
	if(m_PartStatus_list.Lookup(reqfile, thisAbyPartStatus))
	{
		delete[] thisAbyPartStatus;
		m_PartStatus_list.RemoveKey(reqfile);
	}
	m_abyPartStatus = NULL;
	//MORPH   END - Added by SiRoB, Keep A4AF infos

	bool bPartsNeeded = false;
	int iNeeded = 0;
	if (!nED2KPartCount)
	{
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		//MORPH START - Added by SiRoB, Keep A4AF infos
		m_PartStatus_list.SetAt(reqfile, m_abyPartStatus);
		//MORPH - Changed by SiRoB, ICS merged into partstatus
		/*
		memset(m_abyPartStatus, 1, m_nPartCount);
		*/
		memset(m_abyPartStatus,SC_AVAILABLE,m_nPartCount);
		bPartsNeeded = true;
		m_bCompleteSource = true;
		if (bUdpPacket ? (thePrefs.GetDebugClientUDPLevel() > 0) : (thePrefs.GetDebugClientTCPLevel() > 0))
		{
			for (UINT i = 0; i < m_nPartCount; i++)
			{
				if (!reqfile->IsComplete((uint64)i*PARTSIZE, ((uint64)(i+1)*PARTSIZE)-1, false))
					iNeeded++;
			}
		}
	}
	else
	{
		if (reqfile->GetED2KPartCount() != nED2KPartCount) {
			if (thePrefs.GetVerbose()) {
				DebugLogWarning(_T("FileName: \"%s\""), m_strClientFilename);
				DebugLogWarning(_T("FileStatus: %s"), DbgGetFileStatus(nED2KPartCount, data));
			}
			CString strError;
			strError.Format(_T("ProcessFileStatus - wrong part number recv=%u  expected=%u  %s"), nED2KPartCount, reqfile->GetED2KPartCount(), DbgGetFileInfo(reqfile->GetFileHash()));
			m_nPartCount = 0;
			throw strError;
		}
		m_nPartCount = reqfile->GetPartCount();

		m_bCompleteSource = false;
		m_abyPartStatus = new uint8[m_nPartCount];
		//MORPH START - Added by SiRoB, Keep A4AF infos
		m_PartStatus_list.SetAt(reqfile, m_abyPartStatus);
		//MORPH   END - Added by SiRoB, Keep A4AF infos
		UINT done = 0;
		while (done != m_nPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (UINT i = 0; i != 8; i++)
			{
				//MORPH - Changed by SiRoB, ICS merged into partstatus
				/*
				m_abyPartStatus[done] = ((toread>>i)&1)? 1:0; 	
				*/
				m_abyPartStatus[done] = (uint8)(((toread>>i)&1)? SC_AVAILABLE:0); 	
				if (m_abyPartStatus[done])
				{
					if (!reqfile->IsComplete((uint64)done*PARTSIZE, ((uint64)(done+1)*PARTSIZE)-1, false)){
						bPartsNeeded = true;
						iNeeded++;
					}
				}
				done++;
				if (done == m_nPartCount)
					break;
			}
		}
	}
	
	if (bUdpPacket ? (thePrefs.GetDebugClientUDPLevel() > 0) : (thePrefs.GetDebugClientTCPLevel() > 0))
	{
		TCHAR* psz = new TCHAR[m_nPartCount + 1];
		UINT i;
		for (i = 0; i < m_nPartCount; i++)
			//MORPH - Changed by SiRoB, ICS merged into partstatus
			/*
			psz[i] = m_abyPartStatus[i] ? _T('#') : _T('.');
			*/
			psz[i] = (m_abyPartStatus[i]&SC_AVAILABLE) ? _T('#') : _T('.');
		psz[i] = _T('\0');
		Debug(_T("  Parts=%u  %s  Needed=%u\n"), m_nPartCount, psz, iNeeded);
		delete[] psz;
	}

	UpdateDisplayedInfo(bUdpPacket);
	reqfile->UpdateAvailablePartsCount();

	// NOTE: This function is invoked from TCP and UDP socket!
	if (!bUdpPacket)
	{
		if (!bPartsNeeded)
        {
			SetDownloadState(DS_NONEEDEDPARTS);
            SwapToAnotherFile(_T("A4AF for NNP file. CUpDownClient::ProcessFileStatus() TCP"), true, false, false, NULL, true, true);
        }
        else if (reqfile->m_bMD4HashsetNeeded || (reqfile->IsAICHPartHashSetNeeded() && SupportsFileIdentifiers() 
			&& GetReqFileAICHHash() != NULL && *GetReqFileAICHHash() == reqfile->GetFileIdentifier().GetAICHHash())) //If we are using the eMule filerequest packets, this is taken care of in the Multipacket!
			SendHashSetRequest();
		else
			SendStartupLoadReq();
	}
	else
	{
		if (!bPartsNeeded)
        {
			SetDownloadState(DS_NONEEDEDPARTS);
            //SwapToAnotherFile(_T("A4AF for NNP file. CUpDownClient::ProcessFileStatus() UDP"), true, false, false, NULL, true, false);
        }
		else
			SetDownloadState(DS_ONQUEUE);
	}
	reqfile->UpdatePartsInfo();
}

//MORPH START - Added by SiRoB, ICS merged into partstatus
void CUpDownClient::ProcessFileIncStatus(CSafeMemFile* data,uint32 , CPartFile* pFile){
	if (reqfile==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileIncStatus; reqfile==NULL)");
	if (pFile == NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileIncStatus; pFile not found in downloadfile list)");
	uint16 nED2KPartCount = data->ReadUInt16();

	if (nED2KPartCount){
		if (pFile->GetED2KPartCount() != nED2KPartCount){
			CString strError;
			strError.Format(_T("ProcessFileIncStatus - wrong part number recv=%u  expected=%u  %s"), nED2KPartCount, pFile->GetED2KPartCount(), DbgGetFileInfo(pFile->GetFileHash()));
			throw strError;
		}
		uint8* thisAbyPartStatus;
		if (m_PartStatus_list.Lookup(pFile, thisAbyPartStatus)) {
			UINT done = 0;
			while (done != nED2KPartCount){
				uint8 toread = data->ReadUInt8();
				for (UINT i = 0;i != 8;i++){
					if((toread>>i)&1)
						thisAbyPartStatus[done] |= SC_PARTIAL;
					done++;
					if (done == nED2KPartCount)
						break;
				}
			}
		}
	}
	reqfile->NewSrcIncPartsInfo();
}
//MORPH END  - Added By SiRoB, ICS merged into partstatus

bool CUpDownClient::AddRequestForAnotherFile(CPartFile* file){
	for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;){
		if (m_OtherNoNeeded_list.GetNext(pos) == file)
			return false;
	}
	for (POSITION pos = m_OtherRequests_list.GetHeadPosition();pos != 0;){
		if (m_OtherRequests_list.GetNext(pos) == file)
			return false;
	}
	m_OtherRequests_list.AddTail(file);
	file->A4AFsrclist.AddTail(this); // [enkeyDEV(Ottavio84) -A4AF-]

	return true;
}

void CUpDownClient::ClearDownloadBlockRequests()
{
	for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;){
		Requested_Block_Struct* cur_block = m_DownloadBlocks_list.GetNext(pos);
		if (reqfile){
			//MORPH - Optimization
			/*
			reqfile->RemoveBlockFromList(cur_block->StartOffset,cur_block->EndOffset);
			*/
			reqfile->RemoveBlockFromList(cur_block);
		}
		delete cur_block;
	}
	m_DownloadBlocks_list.RemoveAll();

	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition();pos != 0;){
		Pending_Block_Struct *pending = m_PendingBlocks_list.GetNext(pos);
		if (reqfile){
			//MORPH - Optimization
			/*
			reqfile->RemoveBlockFromList(pending->block->StartOffset, pending->block->EndOffset);
			*/
			reqfile->RemoveBlockFromList(pending->block);
		}

		delete pending->block;
		// Not always allocated
		if (pending->zStream){
			inflateEnd(pending->zStream);
			delete pending->zStream;
		}
		delete pending;
	}
	m_PendingBlocks_list.RemoveAll();
}

void CUpDownClient::SetDownloadState(EDownloadState nNewState, LPCTSTR pszReason){
	if (m_nDownloadState != nNewState){
		switch( nNewState )
		{
			case DS_CONNECTING:
	            m_dwLastTriedToConnect = ::GetTickCount();
				break;
			case DS_TOOMANYCONNSKAD:
				//This client had already been set to DS_CONNECTING.
				//So we reset this time so it isn't stuck at TOOMANYCONNS for 20mins.
				m_dwLastTriedToConnect = ::GetTickCount()-20*60*1000;
				break;
			case DS_WAITCALLBACKKAD:
			case DS_WAITCALLBACK:
				break;
            case DS_NONEEDEDPARTS:
                // Since tcp asks never sets reask time if the result is DS_NONEEDEDPARTS
                // If we set this, we will not reask for that file until some time has passed.
                SetLastAskedTime();
                //DontSwapTo(reqfile);

			//MORPH START - Changed by Stulle, Ensure we reset m_dwLastTriedToConnect for A4AF and more
			//Note: I don't know why but this code is required for Morph... some more research may show why but I am not up for that right now.
			default:
				switch( m_nDownloadState )
				{
					case DS_WAITCALLBACK:
					case DS_WAITCALLBACKKAD:
						break;
					default:
						m_dwLastTriedToConnect = ::GetTickCount()-20*60*1000;
						break;
				}
				break;
			//MORPH END   - Changed by Stulle, Ensure we reset m_dwLastTriedToConnect for A4AF and more
		}

		if (reqfile){
		    if(nNewState == DS_DOWNLOADING){
                if(thePrefs.GetLogUlDlEvents())
                    AddDebugLogLine(DLP_VERYLOW, false, _T("Download session started. User: %s in SetDownloadState(). New State: %i"), DbgGetClientInfo(), nNewState);

			    reqfile->AddDownloadingSource(this);
		    }
		    else if(m_nDownloadState == DS_DOWNLOADING){
			    reqfile->RemoveDownloadingSource(this);
		    }
		}

        if(nNewState == DS_DOWNLOADING && socket){
		    socket->SetTimeOut(CONNECTION_TIMEOUT*4);
        }

		bool bFailedSession = true; // MORPH
		if (m_nDownloadState == DS_DOWNLOADING ){
			if(socket)
				socket->SetTimeOut(CONNECTION_TIMEOUT);

			//MORPH START - Changed by SiRoB, don't overhide pszReason
			/*
			if (thePrefs.GetLogUlDlEvents()) {
				switch( nNewState )
				{
					case DS_NONEEDEDPARTS:
						pszReason = _T("NNP. You don't need any parts from this client.");
				}
			*/
			if (pszReason == NULL) {
				if (thePrefs.GetLogUlDlEvents()) {
					switch( nNewState )
					{
						case DS_NONEEDEDPARTS:
							pszReason = _T("NNP. You don't need any parts from this client.");
					}
				}
			} else {
				switch( nNewState )
				{
					case DS_NONEEDEDPARTS:
						bFailedSession = false;
				}
			} 
			//MORPH END   - Changed by SiRoB, don't overhide pszReason

                if(thePrefs.GetLogUlDlEvents())
                    AddDebugLogLine(DLP_VERYLOW, false, _T("Download session ended: %s User: %s in SetDownloadState(). New State: %i, Length: %s, Payload: %s, Transferred: %s, Req blocks not yet completed: %i."), pszReason, DbgGetClientInfo(), nNewState, CastSecondsToHM(GetDownTimeDifference(false)/1000), CastItoXBytes(GetSessionPayloadDown(), false, false), CastItoXBytes(GetSessionDown(), false, false), m_PendingBlocks_list.GetCount());
			//MORPH START - Changed by SiRoB, don't overhide pszReason
			/*
			}
			*/
			//MORPH END   - Changed by SiRoB, don't overhide pszReason

			//MORPH START - Changed by schnulli900, filter clients with failed downloads [Xman]
			/*
			ResetSessionDown();
			*/
			if(GetSessionDown()< (12*1024)) //12 kb
				m_uFailedDownloads++;
			else
				m_uFailedDownloads=0; 
			//MORPH END   - Changed by schnulli900, filter clients with failed downloads [Xman]

			// -khaos--+++> Extended Statistics (Successful/Failed Download Sessions)
			if ( m_bTransferredDownMini && nNewState != DS_ERROR )
				thePrefs.Add2DownSuccessfulSessions(); // Increment our counters for successful sessions (Cumulative AND Session)
			//MORPH - Added by SiRoB, not a failed session
			/*
			else
			*/
			else if (bFailedSession)
			//MORPH - Added by SiRoB, not a failed session
				thePrefs.Add2DownFailedSessions(); // Increment our counters failed sessions (Cumulative AND Session)
			//wistily start
			uint32 tempDownTimeDifference= GetDownTimeDifference();
			Add2DownTotalTime(tempDownTimeDifference);
			thePrefs.Add2DownSAvgTime(tempDownTimeDifference/1000);
			/*
			thePrefs.Add2DownSAvgTime(GetDownTimeDifference()/1000);
			*/
			//wistily stop
			// <-----khaos-

			//MORPH START - Added by schnulli900, filter clients with failed downloads [Xman]
			ResetSessionDown(); 
			//MORPH END   - Added by schnulli900, filter clients with failed downloads [Xman]

			m_nDownloadState = (_EDownloadState)nNewState;

			ClearDownloadBlockRequests();
				
			m_nDownDatarate = 0;
			m_AvarageDDR_list.RemoveAll();
			m_nSumForAvgDownDataRate = 0;

			if (nNewState == DS_NONE){
				//MORPH START - Removed by SiRoB, Keep A4AF infos
				/*
				delete[] m_abyPartStatus;
				*/
				//MORPH   END - Removed by SiRoB, Keep A4AF infos
				m_abyPartStatus = NULL;
				m_nPartCount = 0;
			}
			if (socket && nNewState != DS_ERROR )
				socket->DisableDownloadLimit();
		}
		m_nDownloadState = (_EDownloadState)nNewState;
		if( GetDownloadState() == DS_DOWNLOADING ){
			m_AvarageDDR_ListLastRemovedTimestamp = GetTickCount(); //MORPH - Added by SiRoB, Better Upload rate calcul
			if ( IsEmuleClient() )
				SetRemoteQueueFull(false);
			SetRemoteQueueRank(0);
			SetAskedCountDown(0);
		}
		UpdateDisplayedInfo(true);
	}
}

void CUpDownClient::ProcessHashSet(const uchar* packet, uint32 size, bool bFileIdentifiers)
{
	CSafeMemFile data(packet, size);
	if (bFileIdentifiers)
	{
		if (!m_fHashsetRequestingMD4 && !m_fHashsetRequestingAICH)
			throw CString(_T("unwanted hashset2"));
		CFileIdentifierSA fileIdent;
		if (!fileIdent.ReadIdentifier(&data))
			throw CString(_T("Invalid FileIdentifier"));
		if (reqfile == NULL || !reqfile->GetFileIdentifier().CompareRelaxed(fileIdent))
		{
			CheckFailedFileIdReqs(packet);
			throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessHashSet2)");	
		}
		bool bMD4 = m_fHashsetRequestingMD4 != 0;
		bool bAICH = m_fHashsetRequestingAICH != 0;
		if (!reqfile->GetFileIdentifier().ReadHashSetsFromPacket(&data, bMD4, bAICH))
		{
			if (m_fHashsetRequestingMD4)
				reqfile->m_bMD4HashsetNeeded = true;
			if (m_fHashsetRequestingAICH)
				reqfile->SetAICHHashSetNeeded(true);
			m_fHashsetRequestingMD4 = 0;
			m_fHashsetRequestingAICH = 0;
			throw GetResString(IDS_ERR_BADHASHSET);
		}
		if (m_fHashsetRequestingMD4 && !bMD4)
		{
			DebugLogWarning(_T("Client was unable to deliver requested MD4 hashset (shouldn't happen) - %s, file: %s"), DbgGetClientInfo(), reqfile->GetFileName());
			reqfile->m_bMD4HashsetNeeded = true;
		}
		else if (m_fHashsetRequestingMD4)
			DebugLog(_T("Received valid MD4 Hashset (FileIdentifiers) form %s, file: %s"), DbgGetClientInfo(), reqfile->GetFileName());
		
		bool bPerformFirstHash = true;		// SLUGFILLER: SafeHash - Rehash
		if (m_fHashsetRequestingAICH && !bAICH)
		{
			DebugLogWarning(_T("Client was unable to deliver requested AICH part hashset, asking other clients - %s, file: %s"), DbgGetClientInfo(), reqfile->GetFileName());
			reqfile->SetAICHHashSetNeeded(true);
			bPerformFirstHash = false;		// SLUGFILLER: SafeHash - Rehash
		}
		else if (m_fHashsetRequestingAICH)
			DebugLog(_T("Received valid AICH Part Hashset form %s, file: %s"), DbgGetClientInfo(), reqfile->GetFileName());
		m_fHashsetRequestingMD4 = 0;
		m_fHashsetRequestingAICH = 0;
		if(bPerformFirstHash) reqfile->PerformFirstHash();		// SLUGFILLER: SafeHash - Rehash
	}
	else
	{
		if (!m_fHashsetRequestingMD4)
			throw CString(_T("unwanted hashset"));
		if ( (!reqfile) || md4cmp(packet,reqfile->GetFileHash()))
		{
			CheckFailedFileIdReqs(packet);
			throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessHashSet)");	
		}
		m_fHashsetRequestingMD4 = 0;
		if (!reqfile->GetFileIdentifier().LoadMD4HashsetFromFile(&data, true))
		{
			reqfile->m_bMD4HashsetNeeded = true;
			throw GetResString(IDS_ERR_BADHASHSET);
		}
		reqfile->PerformFirstHash();		// SLUGFILLER: SafeHash - Rehash
	}
	SendStartupLoadReq();
}

//MORPH START - Enhanced DBR
uint64 CUpDownClient::GetRemainingReservedDataToDownload() const {
	uint64 reserveddata = 0;
	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != NULL;) {
		const Pending_Block_Struct* Pending = m_PendingBlocks_list.GetNext(pos);
		const Requested_Block_Struct* block = Pending->block;
		if (m_nLastBlockOffset < block->StartOffset && block->EndOffset > m_nLastBlockOffset)
			reserveddata += block->EndOffset - block->StartOffset + 1;
		else
			reserveddata += block->EndOffset - block->StartOffset + 1 - Pending->totalUnzipped;
	}
	for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition(); pos != NULL;) {
		const Requested_Block_Struct* block = m_DownloadBlocks_list.GetNext(pos);
		reserveddata += block->EndOffset - block->StartOffset + 1;
	}
	return reserveddata;
}
uint64	CUpDownClient::GetRemainingAvailableData(const CPartFile* file) const{
	uint8* thisAbyPartStatus;
	if (m_PartStatus_list.Lookup(file, thisAbyPartStatus))
		return file->GetRemainingAvailableData(thisAbyPartStatus);
	return 0;
}
//MORPH END   - Enhanced DBR

void CUpDownClient::CreateBlockRequests(int iMaxBlocks)
{
	ASSERT( iMaxBlocks >= 1 /*&& iMaxBlocks <= 3*/ );
	//MORPH START - Proper number of needed requested block
	/*
	if (m_DownloadBlocks_list.IsEmpty())
	{
		uint16 count;
        if(iMaxBlocks > m_PendingBlocks_list.GetCount()) {
            count = (uint16)(iMaxBlocks - m_PendingBlocks_list.GetCount());
        } else {
            count = 0;
        }

		Requested_Block_Struct** toadd = new Requested_Block_Struct*[count];
		if (reqfile->GetNextRequestedBlock(this, toadd, &count)){
			for (UINT i = 0; i < count; i++)
				m_DownloadBlocks_list.AddTail(toadd[i]);
		}
		delete[] toadd;
	}

	while (m_PendingBlocks_list.GetCount() < iMaxBlocks && !m_DownloadBlocks_list.IsEmpty())
	{
		Pending_Block_Struct* pblock = new Pending_Block_Struct;
		pblock->block = m_DownloadBlocks_list.RemoveHead();
		m_PendingBlocks_list.AddTail(pblock);
	}
	*/
	uint16 futurePossiblePendingBlock = (uint16)(m_PendingBlocks_list.GetCount()+m_DownloadBlocks_list.GetCount());
	if (futurePossiblePendingBlock < iMaxBlocks)
	{
		uint16 neededblock = (uint16)(iMaxBlocks - futurePossiblePendingBlock);
		if (iMaxBlocks > 3)
			neededblock = (neededblock/3+1)*3;
		Requested_Block_Struct** toadd = new Requested_Block_Struct*[neededblock];
		if (reqfile->GetNextRequestedBlock(this,toadd,&neededblock)){
			for (UINT i = 0; i < neededblock; i++)
				m_DownloadBlocks_list.AddTail(toadd[i]);
		}
		delete[] toadd;
	}
	while (!m_DownloadBlocks_list.IsEmpty())
	{
		Pending_Block_Struct* pblock = new Pending_Block_Struct;
		pblock->block = m_DownloadBlocks_list.RemoveHead();
		m_PendingBlocks_list.AddTail(pblock);
	}
	//MORPH END  - Proper number of needed requested block
	//MORPH START - Determine Remote Speed
	DWORD curTick = GetTickCount();
	if (curTick - m_dwDownDatarateAVG > SEC2MS(1)) {
		m_nDownDatarateAVG = max(m_nDownDatarateAVG, 1000*(m_nTransferredDown-m_nTransferredDownDatarateAVG)/(curTick+1 - m_dwDownDatarateAVG));
		m_nTransferredDownDatarateAVG = m_nTransferredDown;
		m_dwDownDatarateAVG = curTick;
	}
	//MORPH END   - Determine Remote Speed
}

void CUpDownClient::SendBlockRequests()
{
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__RequestParts", this, reqfile!=NULL ? reqfile->GetFileHash() : NULL);

	m_dwLastBlockReceived = ::GetTickCount();
	if (!reqfile)
		return;

    // prevent locking of too many blocks when we are on a slow (probably standby/trickle) slot
	//MORPH START
	/*
    int blockCount = 3;
    if(IsEmuleClient() && m_byCompatibleClient==0 && reqfile->GetFileSize()-reqfile->GetCompletedSize() <= (uint64)PARTSIZE*4) {
	*/
	int blockCount = GetDownloadDatarateAVG()/(3*EMBLOCKSIZE)+2;
	// MORPH END
        // if there's less than two chunks left, request fewer blocks for
        // slow downloads, so they don't lock blocks from faster clients.
        // Only trust eMule clients to be able to handle less blocks than three
	//MORPH START
	/*
        if(GetDownloadDatarate() < 600 || GetSessionPayloadDown() < 40*1024) {
            blockCount = 1;
        } else if(GetDownloadDatarate() < 1200) {
            blockCount = 2;
        }
    }
	*/
        if(GetDownloadDatarate() < 1200  || GetSessionPayloadDown() < 40*1024) {
            blockCount = 1;
        }
	// MORPH END
	CreateBlockRequests(blockCount);

	if (m_PendingBlocks_list.IsEmpty()){
		//MORPH START - SendCancelTransfer if we can't find an other file to download
		/*
		SendCancelTransfer();
		SetDownloadState(DS_NONEEDEDPARTS);
        SwapToAnotherFile(_T("A4AF for NNP file. CUpDownClient::SendBlockRequests()"), true, false, false, NULL, true, true);
		*/
		SetDownloadState(DS_NONEEDEDPARTS, _T("NNP. We can't ask more block request. Client don't have any block for us or file is already fully requested."));
		if (!SwapToAnotherFile(_T("A4AF for NNP file. CUpDownClient::SendBlockRequests()"), true, false, false, NULL, true, true))
			SendCancelTransfer();
		//MORPH END   - SendCancelTransfer if we can't find an other file to download
		return;
	}

	bool bI64Offsets = false;
	POSITION pos = m_PendingBlocks_list.GetHeadPosition();
	//MORPH START - Changed by SiRoB, Official fix to prevent sending same requested block
	/*
	for (uint32 i = 0; i != 3; i++){
		if (pos){
			Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
	*/
	uint32 numberofblocktorequest = 0;
	while (pos){
		Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
		if (pending->fQueued == 0) {
	//MORPH END   - Changed by SiRoB, Official fix to prevent sending same requested block
			ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
			if (pending->block->StartOffset > 0xFFFFFFFF || pending->block->EndOffset >= 0xFFFFFFFF){
				bI64Offsets = true;
				if (!SupportsLargeFiles()){
					ASSERT( false );
					SendCancelTransfer();
					SetDownloadState(DS_ERROR);
					return;
				}
			//MORPH START - Changed by SiRoB, Official fix to prevent sending same requested block
			/*
				break;
			}
			*/
			}
			++numberofblocktorequest;
			//MORPH END   - Changed by SiRoB, Official fix to prevent sending same requested block
		}
	}

	//MORPH START - Changed by SiRoB, Official fix to prevent sending same requested block
	if (numberofblocktorequest == 0) //mean there is no request to do
		return;
	//MORPH END   - Changed by SiRoB, Official fix to prevent sending same requested block
	Packet* packet;
	//MORPH START
	uint32 npacket = 0;
	const UINT nbpackettosend = (numberofblocktorequest+2)/3;
	//MORPH END
	if (bI64Offsets){
		//MORPH START
#if !defined DONT_USE_SEND_ARRAY_PACKET
		Packet** apacket = new Packet*[nbpackettosend];
#endif
		pos = m_PendingBlocks_list.GetHeadPosition();
		while (npacket<nbpackettosend) {
		//MORPH END
		const int iPacketSize = 16+(3*8)+(3*8); // 64
		packet = new Packet(OP_REQUESTPARTS_I64, iPacketSize, OP_EMULEPROT);
		CSafeMemFile data((const BYTE*)packet->pBuffer, iPacketSize);
		data.WriteHash16(reqfile->GetFileHash());
		//MORPH START
		/*
		pos = m_PendingBlocks_list.GetHeadPosition();
		for (uint32 i = 0; i != 3; i++){
		*/
		POSITION initialpos = pos;
		uint32 i = 0;
		while (i != 3){
		//MORPH END
			if (pos){
				Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
				//MORPH START
				/*
				ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
				//ASSERT( pending->zStream == NULL );
				//ASSERT( pending->totalUnzipped == 0 );
				pending->fZStreamError = 0;
				pending->fRecovered = 0;
				data.WriteUInt64(pending->block->StartOffset);
				*/
				if (pending->fQueued == 0) {
					ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
					//ASSERT( pending->zStream == NULL );
					//ASSERT( pending->totalUnzipped == 0 );
					pending->fZStreamError = 0;
					pending->fRecovered = 0;
					pending->fQueued = 1;
					data.WriteUInt64(pending->block->StartOffset);
					++i;
				}
				//MORPH END
			}
			else
			{ //MORPH
				data.WriteUInt64(0);
			//MORPH START
				++i;
			}
			//MORPH END
		}
		//MORPH START
		/*
		pos = m_PendingBlocks_list.GetHeadPosition();
		for (uint32 i = 0; i != 3; i++){
		*/
		pos = initialpos;
		i = 0;
		while (i != 3){
		//MORPH END
			if (pos){
				//MORPH START
				/*
				Requested_Block_Struct* block = m_PendingBlocks_list.GetNext(pos)->block;
				*/
				Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
				if (pending->fQueued == 1) {
					pending->fQueued = 2;
					Requested_Block_Struct* block = pending->block;
				//MORPH END
					uint64 endpos = block->EndOffset+1;
					data.WriteUInt64(endpos);
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						CString strInfo;
						strInfo.Format(_T("  Block request %u: "), i);
						strInfo += DbgGetBlockInfo(block);
						strInfo.AppendFormat(_T(",  Complete=%s"), reqfile->IsComplete(block->StartOffset, block->EndOffset, false) ? _T("Yes(NOTE:)") : _T("No"));
						strInfo.AppendFormat(_T(",  PureGap=%s"), reqfile->IsPureGap(block->StartOffset, block->EndOffset) ? _T("Yes") : _T("No(NOTE:)"));
						strInfo.AppendFormat(_T(",  AlreadyReq=%s"), reqfile->IsAlreadyRequested(block->StartOffset, block->EndOffset) ? _T("Yes") : _T("No(NOTE:)"));
						strInfo += _T('\n');
						Debug(strInfo);
					}
					++i; //MORPH
				}
			}
			else
			{
				data.WriteUInt64(0);
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					Debug(_T("  Block request %u: <empty>\n"), i);
				++i; //MORPH
			}
		}
		//MORPH START
			theStats.AddUpDataOverheadFileRequest(packet->size);
#if !defined DONT_USE_SEND_ARRAY_PACKET
			apacket[npacket++] = packet;
#else
			socket->SendPacket(packet,true,true);
#endif
		}
#if !defined DONT_USE_SEND_ARRAY_PACKET
		if (npacket) {
			socket->SendPacket(apacket, npacket, true, true);
		}
		delete[] apacket;
#endif
		//MORPH END
	}
	else{
		//MORPH START
#if !defined DONT_USE_SEND_ARRAY_PACKET
		Packet** apacket = new Packet*[nbpackettosend];
#endif
		pos = m_PendingBlocks_list.GetHeadPosition();
		while (npacket<nbpackettosend) {
		//MORPH END
		const int iPacketSize = 16+(3*4)+(3*4); // 40
		packet = new Packet(OP_REQUESTPARTS,iPacketSize);
		CSafeMemFile data((const BYTE*)packet->pBuffer, iPacketSize);
		data.WriteHash16(reqfile->GetFileHash());
		//MORPH START
		/*
		pos = m_PendingBlocks_list.GetHeadPosition();
		for (uint32 i = 0; i != 3; i++){
		*/
		POSITION initalpos = pos;
		uint32 i = 0;
		while (i != 3){
		//MORPH END
			if (pos){
				Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
				//MORPH START
				/*
				ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
				//ASSERT( pending->zStream == NULL );
				//ASSERT( pending->totalUnzipped == 0 );
				pending->fZStreamError = 0;
				pending->fRecovered = 0;
				data.WriteUInt32((uint32)pending->block->StartOffset);
				*/
				if (pending->fQueued == 0) {
					ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
					//ASSERT( pending->zStream == NULL );
					//ASSERT( pending->totalUnzipped == 0 );
					pending->fZStreamError = 0;
					pending->fRecovered = 0;
					pending->fQueued = 1;
					data.WriteUInt32((uint32)pending->block->StartOffset);
					++i;
				}
				//MORPH END
			}
			else
			{ //MORPH
				data.WriteUInt32(0);
			//MORPH START
				++i;
			}
			//MORPH END
		}
		//MORPH START
		/*
		pos = m_PendingBlocks_list.GetHeadPosition();
		for (uint32 i = 0; i != 3; i++){
		*/
		pos = initalpos;
		i = 0;
		while (i != 3){
		//MORPH END
			if (pos){
				//MORPH START
				/*
				Requested_Block_Struct* block = m_PendingBlocks_list.GetNext(pos)->block;
				*/
				Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
				if (pending->fQueued == 1) {
					pending->fQueued = 2;
					Requested_Block_Struct* block = pending->block;
				//MORPH END
					uint64 endpos = block->EndOffset+1;
					data.WriteUInt32((uint32)endpos);
					if (thePrefs.GetDebugClientTCPLevel() > 0){
						CString strInfo;
						strInfo.Format(_T("  Block request %u: "), i);
						strInfo += DbgGetBlockInfo(block);
						strInfo.AppendFormat(_T(",  Complete=%s"), reqfile->IsComplete(block->StartOffset, block->EndOffset, false) ? _T("Yes(NOTE:)") : _T("No"));
						strInfo.AppendFormat(_T(",  PureGap=%s"), reqfile->IsPureGap(block->StartOffset, block->EndOffset) ? _T("Yes") : _T("No(NOTE:)"));
						strInfo.AppendFormat(_T(",  AlreadyReq=%s"), reqfile->IsAlreadyRequested(block->StartOffset, block->EndOffset) ? _T("Yes") : _T("No(NOTE:)"));
						strInfo += _T('\n');
						Debug(strInfo);
					}
					++i; //MORPH
				}
			}
			else
			{
				data.WriteUInt32(0);
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					Debug(_T("  Block request %u: <empty>\n"), i);
				++i; //MORPH
			}
		}
//MORPH START
/*
	}

	theStats.AddUpDataOverheadFileRequest(packet->size);
	SendPacket(packet, true);
*/
			theStats.AddUpDataOverheadFileRequest(packet->size);
#if !defined DONT_USE_SEND_ARRAY_PACKET
			apacket[npacket++] = packet;
#else
			socket->SendPacket(packet,true,true);
#endif
		}
#if !defined DONT_USE_SEND_ARRAY_PACKET
		if (npacket) {
			socket->SendPacket(apacket, npacket, true, true);
		}
		delete[] apacket;
#endif
	}
//MORPH END  - Changed by SiRoB, Official fix to prevent sending same requested block
}

/* Barry - Originally this only wrote to disk when a full 180k block 
           had been received from a client, and only asked for data in 
		   180k blocks.

		   This meant that on average 90k was lost for every connection
		   to a client data source. That is a lot of wasted data.

		   To reduce the lost data, packets are now written to a buffer
		   and flushed to disk regularly regardless of size downloaded.
		   This includes compressed packets.

		   Data is also requested only where gaps are, not in 180k blocks.
		   The requests will still not exceed 180k, but may be smaller to
		   fill a gap.
*/
void CUpDownClient::ProcessBlockPacket(const uchar *packet, uint32 size, bool packed, bool bI64Offsets)
{
	if (!bI64Offsets) {
	    uint32 nDbgStartPos = *((uint32*)(packet+16));
	    if (thePrefs.GetDebugClientTCPLevel() > 1){
		    if (packed)
			    Debug(_T("  Start=%u  BlockSize=%u  Size=%u  %s\n"), nDbgStartPos, *((uint32*)(packet + 16+4)), size-24, DbgGetFileInfo(packet));
		    else
			    Debug(_T("  Start=%u  End=%u  Size=%u  %s\n"), nDbgStartPos, *((uint32*)(packet + 16+4)), *((uint32*)(packet + 16+4)) - nDbgStartPos, DbgGetFileInfo(packet));
	    }
	}

	// Ignore if no data required
	if (!(GetDownloadState() == DS_DOWNLOADING || GetDownloadState() == DS_NONEEDEDPARTS)){
		TRACE("%s - Invalid download state\n", __FUNCTION__);
		return;
	}

	

	// Update stats
	m_dwLastBlockReceived = ::GetTickCount();

	// Read data from packet
	CSafeMemFile data(packet, size);
	uchar fileID[16];
	data.ReadHash16(fileID);
	int nHeaderSize = 16;

	// Check that this data is for the correct file
	if ( (!reqfile) || md4cmp(packet, reqfile->GetFileHash()))
	{
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessBlockPacket)");
	}

	// Find the start & end positions, and size of this chunk of data
	uint64 nStartPos;
	uint64 nEndPos;
	uint32 nBlockSize = 0;
	
	if (bI64Offsets){
		nStartPos = data.ReadUInt64();
		nHeaderSize += 8;
	}
	else{
		nStartPos = data.ReadUInt32();
		nHeaderSize += 4;
	}
	if (packed)
	{
		nBlockSize = data.ReadUInt32();
		nHeaderSize += 4;
		nEndPos = nStartPos + (size - nHeaderSize);
	}
	else{
		if (bI64Offsets){
			nEndPos = data.ReadUInt64();
			nHeaderSize += 8;
		}
		else{
			nEndPos = data.ReadUInt32();
			nHeaderSize += 4;
		}
	}
	uint32 uTransferredFileDataSize = size - nHeaderSize;

	// Check that packet size matches the declared data size + header size (24)
	if (nEndPos == nStartPos || size != ((nEndPos - nStartPos) + nHeaderSize))
		throw GetResString(IDS_ERR_BADDATABLOCK) + _T(" (ProcessBlockPacket)");

	// -khaos--+++>
	// Extended statistics information based on which client and remote port sent this data.
	// The new function adds the bytes to the grand total as well as the given client/port.
	// bFromPF is not relevant to downloaded data.  It is purely an uploads statistic.
	thePrefs.Add2SessionTransferData(GetClientSoft(), GetUserPort(), false, false, uTransferredFileDataSize, false);
	// <-----khaos-

	m_nDownDataRateMS += uTransferredFileDataSize;
	//MORPH - Avoid Credits Accumulate faker
	/*
	if (credits)
		credits->AddDownloaded(uTransferredFileDataSize, GetIP());
	*/
	bool bpacketusefull = false;
	//MORPH - Avoid Credit faker

	// Move end back one, should be inclusive
	nEndPos--;

	// Loop through to find the reserved block that this is within
	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != NULL; )
	{
		POSITION posLast = pos;
		Pending_Block_Struct *cur_block = m_PendingBlocks_list.GetNext(pos);
		if ((cur_block->block->StartOffset <= nStartPos) && (cur_block->block->EndOffset >= nStartPos))
		{
			// Found reserved block

			if (cur_block->fZStreamError){
				if (thePrefs.GetVerbose())
					AddDebugLogLine(false, _T("PrcBlkPkt: Ignoring %u bytes of block starting at %I64u because of errornous zstream state for file \"%s\" - %s"), uTransferredFileDataSize, nStartPos, reqfile->GetFileName(), DbgGetClientInfo());
				//MORPH - Optimization
				/*
				reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
				*/
				reqfile->RemoveBlockFromList(cur_block->block);
				return;
			}

			// Remember this start pos, used to draw part downloading in list
			m_nLastBlockOffset = nStartPos;  

			// Occasionally packets are duplicated, no point writing it twice
			// This will be 0 in these cases, or the length written otherwise
			uint32 lenWritten = 0;

			// Handle differently depending on whether packed or not
			if (!packed)
			{
				// security sanitize check
				if (nEndPos > cur_block->block->EndOffset){
					DebugLogError(_T("Received Blockpacket exceeds requested boundaries (requested end: %I64u, Part %u, received end  %I64u, Part %u), file %s, client %s"), cur_block->block->EndOffset
						, (uint32)(cur_block->block->EndOffset / PARTSIZE), nEndPos, (uint32)(nEndPos / PARTSIZE), reqfile->GetFileName(), DbgGetClientInfo());
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
					return;
				}
				bpacketusefull = true; //MORPH
				// Write to disk (will be buffered in part file class)
				lenWritten = reqfile->WriteToBuffer(uTransferredFileDataSize, 
													packet + nHeaderSize,
													nStartPos,
													nEndPos,
													cur_block->block,
													this);
			}
			else // Packed
			{
				ASSERT( (int)size > 0 );
				// Create space to store unzipped data, the size is only an initial guess, will be resized in unzip() if not big enough
				uint32 lenUnzipped = (size * 2); 
				// Don't get too big
				if (lenUnzipped > (3*EMBLOCKSIZE + 300)) //MORPH - enhanced DBR
					lenUnzipped = (3*EMBLOCKSIZE + 300); //MORPH - enhanced DBR
				BYTE *unzipped = new BYTE[lenUnzipped];

				// Try to unzip the packet
				int result = unzip(cur_block, packet + nHeaderSize, uTransferredFileDataSize, &unzipped, &lenUnzipped);
				// no block can be uncompressed to >2GB, 'lenUnzipped' is obviously errornous.
				if (result == Z_OK && (int)lenUnzipped >= 0)
				{
					if (lenUnzipped > 0) // Write any unzipped data to disk
					{
						ASSERT( (int)lenUnzipped > 0 );

						// Use the current start and end positions for the uncompressed data
						nStartPos = cur_block->block->StartOffset + cur_block->totalUnzipped - lenUnzipped;
						nEndPos = cur_block->block->StartOffset + cur_block->totalUnzipped - 1;

						if (nStartPos > cur_block->block->EndOffset || nEndPos > cur_block->block->EndOffset){
							DebugLogError(_T("PrcBlkPkt: ") + GetResString(IDS_ERR_CORRUPTCOMPRPKG),reqfile->GetFileName(),666);
							//MORPH - Optimization
							/*
							reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
							*/
							reqfile->RemoveBlockFromList(cur_block->block);
							// There is no chance to recover from this error
						}
						else{
							bpacketusefull = true; //MORPH - Avoid Credit faker
							// Write uncompressed data to file
							lenWritten = reqfile->WriteToBuffer(uTransferredFileDataSize,
								unzipped,
								nStartPos,
								nEndPos,
								cur_block->block,
								this);
						}
					}
				}
				else
				{
					if (thePrefs.GetVerbose())
					{
						CString strZipError;
						if (cur_block->zStream && cur_block->zStream->msg)
							strZipError.Format(_T(" - %hs"), cur_block->zStream->msg);
						if (result == Z_OK && (int)lenUnzipped < 0){
							ASSERT(0);
							strZipError.AppendFormat(_T("; Z_OK,lenUnzipped=%d"), lenUnzipped);
						}
						DebugLogError(_T("PrcBlkPkt: ") + GetResString(IDS_ERR_CORRUPTCOMPRPKG) + strZipError, reqfile->GetFileName(), result);
					}
					//MORPH - Optimization
					/*
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
					*/
					reqfile->RemoveBlockFromList(cur_block->block);

					// If we had an zstream error, there is no chance that we could recover from it nor that we
					// could use the current zstream (which is in error state) any longer.
					if (cur_block->zStream){
						inflateEnd(cur_block->zStream);
						delete cur_block->zStream;
						cur_block->zStream = NULL;
					}

					// Although we can't further use the current zstream, there is no need to disconnect the sending 
					// client because the next zstream (a series of 10K-blocks which build a 180K-block) could be
					// valid again. Just ignore all further blocks for the current zstream.
					cur_block->fZStreamError = 1;
					cur_block->totalUnzipped = 0;
				}
				delete [] unzipped;
			}

			// These checks only need to be done if any data was written
			if (lenWritten > 0)
			{
				m_nTransferredDown += uTransferredFileDataSize;
                m_nCurSessionPayloadDown += lenWritten;
				SetTransferredDownMini();

				// If finished reserved block
				if (nEndPos >= cur_block->block->EndOffset)
				{
					ASSERT(nEndPos == cur_block->block->EndOffset);
					//MORPH - Optimization
					/*
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
					*/
					reqfile->RemoveBlockFromList(cur_block->block);
					delete cur_block->block;
					// Not always allocated
					if (cur_block->zStream){
						inflateEnd(cur_block->zStream);
						delete cur_block->zStream;
					}
					delete cur_block;
					m_PendingBlocks_list.RemoveAt(posLast);

					// Request next block
					if (thePrefs.GetDebugClientTCPLevel() > 0)
						DebugSend("More block requests", this);
					SendBlockRequests();	
				}
			}
			//MORPH START - Work arround I.C.H and other recovering processing responsible of stalled download 
			else if (nEndPos >= cur_block->block->EndOffset) { 
				ASSERT (nEndPos == cur_block->block->EndOffset);
				DebugLog(LOG_MORPH|LOG_SUCCESS,_T("[FIX STALLED DOWNLOAD] Often due to Data recovery, for '%s' with client: %s"), reqfile->GetFileName(), DbgGetClientInfo());
				//MORPH - Optimization
				/*
				reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
				*/
				reqfile->RemoveBlockFromList(cur_block->block);
				delete cur_block->block;
				// Not always allocated
				if (cur_block->zStream){
					inflateEnd(cur_block->zStream);
					delete cur_block->zStream;
				}
				delete cur_block;
				m_PendingBlocks_list.RemoveAt(posLast);
				
				// Request next block
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("More block requests", this);
				SendBlockRequests();	
			}
			//MORPH END   - Work arround I.C.H and other recovering processing responsible of stalled download
			//MORPH - Avoid Credits Accumulate faker
			if (bpacketusefull){
				if (credits)
					credits->AddDownloaded(uTransferredFileDataSize, GetIP());
			}
			//MORPH - Avoid Credits Accumulate faker

			// Stop looping and exit method
			return;
		}
	}
	//MORPH START
	if (thePrefs.GetVerbose())
		DebugLogError(LOG_MORPH, _T("PrcBlkPkt: Ignoring %u bytes of block starting at %I64u  because unasked for file \"%s\" - %s"), uTransferredFileDataSize, nStartPos, reqfile->GetFileName(), DbgGetClientInfo());
	//MORPH END

	TRACE("%s - Dropping packet\n", __FUNCTION__);
}

int CUpDownClient::unzip(Pending_Block_Struct* block, const BYTE* zipped, uint32 lenZipped, BYTE** unzipped, uint32* lenUnzipped, int iRecursion)
{
//#define TRACE_UNZIP	TRACE
#define TRACE_UNZIP	__noop
	TRACE_UNZIP("unzip: Zipd=%6u Unzd=%6u Rcrs=%d", lenZipped, *lenUnzipped, iRecursion);
  	int err = Z_DATA_ERROR;
  	try
	{
	    // Save some typing
	    z_stream *zS = block->zStream;
    
	    // Is this the first time this block has been unzipped
	    if (zS == NULL)
	    {
		    // Create stream
		    block->zStream = new z_stream;
		    zS = block->zStream;
    
		    // Initialise stream values
		    zS->zalloc = (alloc_func)0;
		    zS->zfree = (free_func)0;
		    zS->opaque = (voidpf)0;
    
		    // Set output data streams, do this here to avoid overwriting on recursive calls
		    zS->next_out = (*unzipped);
		    zS->avail_out = (*lenUnzipped);
    
		    // Initialise the z_stream
		    err = inflateInit(zS);
			if (err != Z_OK){
				TRACE_UNZIP("; Error: new stream failed: %d\n", err);
			    return err;
			}

			ASSERT( block->totalUnzipped == 0 );
		}

	    // Use whatever input is provided
		zS->next_in	 = const_cast<Bytef*>(zipped);
	    zS->avail_in = lenZipped;
    
	    // Only set the output if not being called recursively
	    if (iRecursion == 0)
	    {
		    zS->next_out = (*unzipped);
		    zS->avail_out = (*lenUnzipped);
	    }
    
	    // Try to unzip the data
		TRACE_UNZIP("; inflate(ain=%6u tin=%6u aout=%6u tout=%6u)", zS->avail_in, zS->total_in, zS->avail_out, zS->total_out);
	    err = inflate(zS, Z_SYNC_FLUSH);
    
	    // Is zip finished reading all currently available input and writing all generated output
	    if (err == Z_STREAM_END)
	    {
		    // Finish up
		    err = inflateEnd(zS);
			if (err != Z_OK){
				TRACE_UNZIP("; Error: end stream failed: %d\n", err);
			    return err;
			}
			TRACE_UNZIP("; Z_STREAM_END\n");

		    // Got a good result, set the size to the amount unzipped in this call (including all recursive calls)
		    (*lenUnzipped) = (zS->total_out - block->totalUnzipped);
		    block->totalUnzipped = zS->total_out;
	    }
	    else if ((err == Z_OK) && (zS->avail_out == 0) && (zS->avail_in != 0))
	    {
		    // Output array was not big enough, call recursively until there is enough space
			TRACE_UNZIP("; output array not big enough (ain=%u)\n", zS->avail_in);
    
		    // What size should we try next
		    uint32 newLength = (*lenUnzipped) *= 2;
		    if (newLength == 0)
			    newLength = lenZipped * 2;
    
		    // Copy any data that was successfully unzipped to new array
		    BYTE *temp = new BYTE[newLength];
			ASSERT( zS->total_out - block->totalUnzipped <= newLength );
		    memcpy(temp, (*unzipped), (zS->total_out - block->totalUnzipped));
		    delete[] (*unzipped);
		    (*unzipped) = temp;
		    (*lenUnzipped) = newLength;
    
		    // Position stream output to correct place in new array
		    zS->next_out = (*unzipped) + (zS->total_out - block->totalUnzipped);
		    zS->avail_out = (*lenUnzipped) - (zS->total_out - block->totalUnzipped);
    
		    // Try again
		    err = unzip(block, zS->next_in, zS->avail_in, unzipped, lenUnzipped, iRecursion + 1);
	    }
	    else if ((err == Z_OK) && (zS->avail_in == 0))
	    {
			TRACE_UNZIP("; all input processed\n");
		    // All available input has been processed, everything ok.
		    // Set the size to the amount unzipped in this call (including all recursive calls)
		    (*lenUnzipped) = (zS->total_out - block->totalUnzipped);
		    block->totalUnzipped = zS->total_out;
	    }
	    else
	    {
		    // Should not get here unless input data is corrupt
			if (thePrefs.GetVerbose())
			{
				CString strZipError;
				if (zS->msg)
					strZipError.Format(_T(" %d: '%hs'"), err, zS->msg);
				else if (err != Z_OK)
					strZipError.Format(_T(" %d: '%hs'"), err, zError(err));
				TRACE_UNZIP("; Error: %s\n", strZipError);
				DebugLogError(_T("Unexpected zip error%s in file \"%s\""), strZipError, reqfile ? reqfile->GetFileName() : NULL);
			}
	    }
    
	    if (err != Z_OK)
		    (*lenUnzipped) = 0;
  	}
  	catch (...){
		if (thePrefs.GetVerbose())
			DebugLogError(_T("Unknown exception in %hs: file \"%s\""), __FUNCTION__, reqfile ? reqfile->GetFileName() : NULL);
		err = Z_DATA_ERROR;
		ASSERT(0);
	}

	return err;
}

uint32 CUpDownClient::CalculateDownloadRate(){
	//MORPH START - Changed by SiRoB, Changed by SiRoB, Better datarate mesurement for low and high speed
	/*
	// Patch By BadWolf - Accurate datarate Calculation
	TransferredData newitem = {m_nDownDataRateMS,::GetTickCount()};
	m_AvarageDDR_list.AddTail(newitem);
	m_nSumForAvgDownDataRate += m_nDownDataRateMS;
	m_nDownDataRateMS = 0;

	while (m_AvarageDDR_list.GetCount()>500)
		m_nSumForAvgDownDataRate -= m_AvarageDDR_list.RemoveHead().datalen;
	
	if(m_AvarageDDR_list.GetCount() > 1){
		DWORD dwDuration = m_AvarageDDR_list.GetTail().timestamp - m_AvarageDDR_list.GetHead().timestamp;
		if (dwDuration)
			m_nDownDatarate = (UINT)(1000U * (ULONGLONG)m_nSumForAvgDownDataRate / dwDuration);
	}
	else
		m_nDownDatarate = 0;
	// END Patch By BadWolf
	*/
	uint32 cur_tick = ::GetTickCount();
	if(m_nDownDataRateMS > 0) {
		TransferredData newitem = {m_nDownDataRateMS,cur_tick};
		m_AvarageDDR_list.AddTail(newitem);
		m_nSumForAvgDownDataRate += m_nDownDataRateMS;
		m_nDownDataRateMS = 0;
    }

	while ((UINT)m_AvarageDDR_list.GetCount() > 1 && (cur_tick - m_AvarageDDR_list.GetHead().timestamp) > MAXAVERAGETIMEDOWNLOAD) {
		m_AvarageDDR_ListLastRemovedTimestamp = m_AvarageDDR_list.GetHead().timestamp;
		m_nSumForAvgDownDataRate -= m_AvarageDDR_list.RemoveHead().datalen;
	}
	if (m_AvarageDDR_list.GetCount() > 1) {
		DWORD dwDuration = m_AvarageDDR_list.GetTail().timestamp - m_AvarageDDR_ListLastRemovedTimestamp;
		if (dwDuration < 100) dwDuration = 100;
		DWORD dwAvgTickDuration = dwDuration / m_AvarageDDR_list.GetCount();
		if ((cur_tick - m_AvarageDDR_list.GetTail().timestamp) > dwAvgTickDuration)
			dwDuration += cur_tick - m_AvarageDDR_list.GetTail().timestamp - dwAvgTickDuration;
		m_nDownDatarate = (UINT)(1000U * (ULONGLONG)m_nSumForAvgDownDataRate / dwDuration);
	} else if (m_AvarageDDR_list.GetCount() == 1) {
		DWORD dwDuration = m_AvarageDDR_list.GetTail().timestamp - m_AvarageDDR_ListLastRemovedTimestamp;
		if (dwDuration < 100) dwDuration = 100;
		if ((cur_tick - m_AvarageDDR_list.GetTail().timestamp) > dwDuration)
			dwDuration = cur_tick - m_AvarageDDR_list.GetTail().timestamp;
		m_nDownDatarate = (UINT)(1000U * (ULONGLONG)m_nSumForAvgDownDataRate / dwDuration);
	} else
		m_nDownDatarate = 0;
	//MORPH END   - Changed by SiRoB, Changed by SiRoB, Better datarate mesurement for low and high speed

	//MORPH START - UpdateItemThread
	/*
	m_cShowDR++;
	if (m_cShowDR == 30){
		m_cShowDR = 0;
		UpdateDisplayedInfo();
	}
	*/
	UpdateDisplayedInfo();
	//MORPH END    - UpdateItemThread

	return m_nDownDatarate;
}

void CUpDownClient::CheckDownloadTimeout()
{
	if (IsDownloadingFromPeerCache() && m_pPCDownSocket && m_pPCDownSocket->IsConnected())
	{
		ASSERT( DOWNLOADTIMEOUT < m_pPCDownSocket->GetTimeOut() );
		if (GetTickCount() - m_dwLastBlockReceived > DOWNLOADTIMEOUT)
		{
			OnPeerCacheDownSocketTimeout();
		}
	}
	else
	{
		if ((::GetTickCount() - m_dwLastBlockReceived) > DOWNLOADTIMEOUT)
		{
			ASSERT( socket != NULL );
			if (socket != NULL)
			{
				ASSERT( !socket->IsRawDataMode() );
				if (!socket->IsRawDataMode())
					SendCancelTransfer();
			}
			SetDownloadState(DS_ONQUEUE, _T("Timeout. More than 100 seconds since last complete block was received."));
		}
	}
}

uint16 CUpDownClient::GetAvailablePartCount() const
{
	UINT result = 0;
	for (UINT i = 0; i < m_nPartCount; i++){
		if (IsPartAvailable(i))
			result++;
	}
	return (uint16)result;
}

//MORPH START - Added by SiRoB, Keep A4AF infos
uint16 CUpDownClient::GetAvailablePartCount(const CPartFile* file) const
{
	UINT result = 0;
	uint8* thisAbyPartStatus;
	if (m_PartStatus_list.Lookup(file, thisAbyPartStatus)) {
		for (UINT i = 0; i < file->GetPartCount(); i++){
			if (thisAbyPartStatus[i]&SC_AVAILABLE)
				result++;
		}
	}
	return (uint16)result;
}
//MORPH END   - Added by SiRoB, Keep A4AF infos
void CUpDownClient::SetRemoteQueueRank(UINT nr, bool bUpdateDisplay)
{
	//MORPH - RemoteQueueRank Estimated Time
	if (nr) {
		DWORD	curTick = GetTickCount();
		if (m_nRemoteQueueRankPrev) {
			if (m_nRemoteQueueRankPrev>nr) {
				m_dwRemoteQueueRankEstimatedTime = curTick+(curTick-m_dwRemoteQueueRankLastUpdate)*nr/((int)m_nRemoteQueueRankPrev-nr);
			} else if (m_nRemoteQueueRankPrev < nr) {
				m_dwRemoteQueueRankEstimatedTime = (DWORD)-1;
			}
		}
		m_nRemoteQueueRankPrev = nr;
		m_dwRemoteQueueRankLastUpdate = curTick;
	}
	//MORPH - RemoteQueueRank Estimated Time

	m_nRemoteQueueRank = nr;
	UpdateDisplayedInfo(bUpdateDisplay);
}

void CUpDownClient::UDPReaskACK(uint16 nNewQR)
{
	m_bUDPPending = false;
	SetRemoteQueueRank(nNewQR, true);
    SetLastAskedTime();
	uiDLAskingCounter +=1; //SLAHAM: ADDED Last Asked Counter
}

void CUpDownClient::UDPReaskFNF()
{
	m_bUDPPending = false;
	if (GetDownloadState() != DS_DOWNLOADING){ // avoid premature deletion of 'this' client
		if (thePrefs.GetVerbose())
			AddDebugLogLine(DLP_LOW, false, _T("UDP FNF-Answer: %s - %s"),DbgGetClientInfo(), DbgGetFileInfo(reqfile ? reqfile->GetFileHash() : NULL));
		if (reqfile)
			reqfile->m_DeadSourceList.AddDeadSource(this);
		switch (GetDownloadState()) {
			case DS_ONQUEUE:
			case DS_NONEEDEDPARTS:
                DontSwapTo(reqfile);
                if (SwapToAnotherFile(_T("Source says it doesn't have the file. CUpDownClient::UDPReaskFNF()"), true, true, true, NULL, false, false))
					break;
				/*fall through*/
			default:
				theApp.downloadqueue->RemoveSource(this);
				if (!socket){
					if (Disconnected(_T("UDPReaskFNF socket=NULL")))
						delete this;
				}
		}
	}
	else
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("UDP FNF-Answer: %s - did not remove client because of current download state"),GetUserName());
	}
}

void CUpDownClient::UDPReaskForDownload()
{
	ASSERT ( reqfile );
	if(!reqfile || m_bUDPPending)
		return;
	
	//TODO: This should be changed to determine if the last 4 UDP packets failed, not the total one.
	if( m_nTotalUDPPackets > 3 && ((float)(m_nFailedUDPPackets/m_nTotalUDPPackets) > .3))
		return;

	if (GetUDPPort() != 0 && GetUDPVersion() != 0 && thePrefs.GetUDPPort() != 0 &&
		!theApp.IsFirewalled() && !(socket && socket->IsConnected()) && !thePrefs.GetProxySettings().UseProxy)
	{ 
		if( !HasLowID() )
		{
			//don't use udp to ask for sources
			if(IsSourceRequestAllowed())
				return;
	
	        if(SwapToAnotherFile(_T("A4AF check before OP__ReaskFilePing. CUpDownClient::UDPReaskForDownload()"), true, false, false, NULL, true, true)) {
	            return; // we swapped, so need to go to tcp
	        }
	
			m_bUDPPending = true;
			CSafeMemFile data(128);
			data.WriteHash16(reqfile->GetFileHash());
			if (GetUDPVersion() > 3)
			{
				if (reqfile->IsPartFile())
					((CPartFile*)reqfile)->WritePartStatus(&data);
				else
					data.WriteUInt16(0);
			}
			if (GetUDPVersion() > 2)
				data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
			    DebugSend("OP__ReaskFilePing", this, reqfile->GetFileHash());
			Packet* response = new Packet(&data, OP_EMULEPROT);
			response->opcode = OP_REASKFILEPING;
			theStats.AddUpDataOverheadFileRequest(response->size);
			theApp.downloadqueue->AddUDPFileReasks();
			theApp.clientudp->SendPacket(response, GetIP(), GetUDPPort(), ShouldReceiveCryptUDPPackets(), GetUserHash(), false, 0);
			m_nTotalUDPPackets++;
		}
		else if (HasLowID() && GetBuddyIP() && GetBuddyPort() && HasValidBuddyID())
		{
			m_bUDPPending = true;
			CSafeMemFile data(128);
			data.WriteHash16(GetBuddyID());
			data.WriteHash16(reqfile->GetFileHash());
			if (GetUDPVersion() > 3)
			{
				if (reqfile->IsPartFile())
					((CPartFile*)reqfile)->WritePartStatus(&data);
				else
					data.WriteUInt16(0);
			}
			if (GetUDPVersion() > 2)
				data.WriteUInt16(reqfile->m_nCompleteSourcesCount);
			if (thePrefs.GetDebugClientUDPLevel() > 0)
				DebugSend("OP__ReaskCallbackUDP", this, reqfile->GetFileHash());
			Packet* response = new Packet(&data, OP_EMULEPROT);
			response->opcode = OP_REASKCALLBACKUDP;
			theStats.AddUpDataOverheadFileRequest(response->size);
			theApp.downloadqueue->AddUDPFileReasks();
			// FIXME: We dont know which kadversion the buddy has, so we need to send unencrypted
			theApp.clientudp->SendPacket(response, GetBuddyIP(), GetBuddyPort(), false, NULL, true, 0);
			m_nTotalUDPPackets++;
		}
	}
}

void CUpDownClient::UpdateDisplayedInfo(bool/* force */)
{
	//MORPH START - UpdateItemThread
	/*
#ifdef _DEBUG
	force = true;
#endif
    DWORD curTick = ::GetTickCount();
    if(force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait) {
	    theApp.emuledlg->transferwnd->GetDownloadList()->UpdateItem(this);
		theApp.emuledlg->transferwnd->GetClientList()->RefreshClient(this);
		theApp.emuledlg->transferwnd->GetDownloadClientsList()->RefreshClient(this);
        m_lastRefreshedDLDisplay = curTick;
    }
	*/
	theApp.emuledlg->transferwnd->GetDownloadList()->UpdateItem(this);
	theApp.emuledlg->transferwnd->GetClientList()->RefreshClient(this);
	theApp.emuledlg->transferwnd->GetDownloadClientsList()->RefreshClient(this);
    //MORPH END   - UpdateItemThread
}

const bool CUpDownClient::IsInNoNeededList(const CPartFile* fileToCheck) const
{
    for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos))
	{
        if (m_OtherNoNeeded_list.GetAt(pos) == fileToCheck)
            return true;
    }

    return false;
}

const bool CUpDownClient::SwapToRightFile(CPartFile* SwapTo, CPartFile* cur_file, bool ignoreSuspensions, bool SwapToIsNNPFile, bool curFileisNNPFile, bool& wasSkippedDueToSourceExchange, bool doAgressiveSwapping, bool debug)
{
    bool printDebug = debug && thePrefs.GetLogA4AF();

    if(printDebug) {
        AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: SwapToRightFile. Start compare SwapTo: %s and cur_file %s"), SwapTo?SwapTo->GetFileName():_T("null"), cur_file->GetFileName());
        AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: doAgressiveSwapping: %s"), doAgressiveSwapping?_T("true"):_T("false"));
    }

    if (!SwapTo) {
        return true;
    }

    if(!curFileisNNPFile && cur_file->GetSourceCount() < cur_file->GetMaxSources() ||
        curFileisNNPFile && cur_file->GetSourceCount() < cur_file->GetMaxSources()*.8) {

            if(printDebug)
                AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: cur_file does probably not have too many sources."));

            if(SwapTo->GetSourceCount() > SwapTo->GetMaxSources() ||
               SwapTo->GetSourceCount() >= SwapTo->GetMaxSources()*.8 &&
               SwapTo == reqfile &&
               (
                GetDownloadState() == DS_LOWTOLOWIP ||
                GetDownloadState() == DS_REMOTEQUEUEFULL
               )
              ) {
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: SwapTo is about to be deleted due to too many sources on that file, so we can steal it."));
                return true;
            }

            if(ignoreSuspensions  || !IsSwapSuspended(cur_file, doAgressiveSwapping, curFileisNNPFile)) {
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: No suspend block."));

                DWORD tempTick = ::GetTickCount();
                bool rightFileHasHigherPrio = CPartFile::RightFileHasHigherPrio(SwapTo, cur_file);
                uint32 allNnpReaskTime = FILEREASKTIME*2*(m_OtherNoNeeded_list.GetSize() + ((GetDownloadState() == DS_NONEEDEDPARTS)?1:0)); // wait two reask interval for each nnp file before reasking an nnp file

                if(!SwapToIsNNPFile && (!curFileisNNPFile || GetLastAskedTime(cur_file) == 0 || tempTick-GetLastAskedTime(cur_file) > allNnpReaskTime) && rightFileHasHigherPrio ||
                   SwapToIsNNPFile && curFileisNNPFile &&
                   (
                    GetLastAskedTime(SwapTo) != 0 &&
                    (
                     GetLastAskedTime(cur_file) == 0 ||
                     tempTick-GetLastAskedTime(SwapTo) < tempTick-GetLastAskedTime(cur_file) && (tempTick-GetLastAskedTime(cur_file) > allNnpReaskTime || rightFileHasHigherPrio && tempTick-GetLastAskedTime(SwapTo) < allNnpReaskTime)
                    ) ||
                    rightFileHasHigherPrio && GetLastAskedTime(SwapTo) == 0 && GetLastAskedTime(cur_file) == 0
                   ) ||
                   SwapToIsNNPFile && !curFileisNNPFile) {
                    if(printDebug)
                        if(!SwapToIsNNPFile && !curFileisNNPFile && rightFileHasHigherPrio)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Higher prio."));
                        else if(!SwapToIsNNPFile && (GetLastAskedTime(cur_file) == 0 || tempTick-GetLastAskedTime(cur_file) > allNnpReaskTime) && rightFileHasHigherPrio)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Time to reask nnp and it had higher prio."));
                        else if(GetLastAskedTime(SwapTo) != 0 &&
                                (
                                 GetLastAskedTime(cur_file) == 0 ||
                                 tempTick-GetLastAskedTime(SwapTo) < tempTick-GetLastAskedTime(cur_file) && (tempTick-GetLastAskedTime(cur_file) > allNnpReaskTime || rightFileHasHigherPrio && tempTick-GetLastAskedTime(SwapTo) < allNnpReaskTime)
                                )
                               )
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Both nnp and cur_file has longer time since reasked."));
                        else if(SwapToIsNNPFile && !curFileisNNPFile)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: SwapToIsNNPFile && !curFileisNNPFile"));
                        else
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Higher prio for unknown reason!"));

                    if(IsSourceRequestAllowed(cur_file) && (cur_file->AllowSwapForSourceExchange() || cur_file == reqfile && RecentlySwappedForSourceExchange()) ||
                       !(IsSourceRequestAllowed(SwapTo) && (SwapTo->AllowSwapForSourceExchange() || SwapTo == reqfile && RecentlySwappedForSourceExchange())) ||
                       (GetDownloadState()==DS_ONQUEUE && GetRemoteQueueRank() <= 50)) {
                        if(printDebug)
                            AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: Source Request check ok."));
                        return true;
                    } else {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Source Request check failed."));
                        wasSkippedDueToSourceExchange = true;
                    }
                }

                if(IsSourceRequestAllowed(cur_file, true) && (cur_file->AllowSwapForSourceExchange() || cur_file == reqfile && RecentlySwappedForSourceExchange()) &&
                   !(IsSourceRequestAllowed(SwapTo, true) && (SwapTo->AllowSwapForSourceExchange() || SwapTo == reqfile && RecentlySwappedForSourceExchange())) &&
                   (GetDownloadState()!=DS_ONQUEUE || GetDownloadState()==DS_ONQUEUE && GetRemoteQueueRank() > 50)) {
                    wasSkippedDueToSourceExchange = true;

                    if(printDebug)
                        AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: Source Exchange."));
                    return true;
                }
            } else if(printDebug) {
                AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: Suspend block."));
            }
    } else if(printDebug) {
        AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: cur_file probably has too many sources."));
    }

    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("oooo Debug: Return false"));

    return false;
}

bool CUpDownClient::SwapToAnotherFile(LPCTSTR reason, bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile, bool allowSame, bool isAboutToAsk, bool debug)
{
    bool printDebug = debug && thePrefs.GetLogA4AF();

    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Switching source %s Remove = %s; bIgnoreNoNeeded = %s; allowSame = %s; Reason = \"%s\""), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No")), (bIgnoreNoNeeded ? _T("Yes") : _T("No")), (allowSame ? _T("Yes") : _T("No")), reason);

    if(!bRemoveCompletely && allowSame && thePrefs.GetA4AFSaveCpu()) {
        // Only swap if we can't keep the old source
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false since prefs setting to save cpu is enabled."));
        return false;
    }

	bool doAgressiveSwapping = (bRemoveCompletely || !allowSame || isAboutToAsk);
    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: doAgressiveSwapping: %s"), doAgressiveSwapping?_T("true"):_T("false"));

    if(!bRemoveCompletely && !ignoreSuspensions && allowSame && GetTimeUntilReask(reqfile, doAgressiveSwapping, true, false) > 0 && (GetDownloadState() != DS_NONEEDEDPARTS || m_OtherRequests_list.IsEmpty())) {
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false due to not reached reask time: GetTimeUntilReask(...) > 0"));

        return false;
    }

    if(!bRemoveCompletely && allowSame && m_OtherRequests_list.IsEmpty() && (/* !bIgnoreNoNeeded ||*/ m_OtherNoNeeded_list.IsEmpty())) {
        // no file to swap too, and it's ok to keep it
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false due to no file to swap too, and it's ok to keep it."));
        return false;
    }

    if (!bRemoveCompletely &&
        (GetDownloadState() != DS_ONQUEUE &&
         GetDownloadState() != DS_NONEEDEDPARTS &&
         GetDownloadState() != DS_TOOMANYCONNS &&
         GetDownloadState() != DS_REMOTEQUEUEFULL &&
         GetDownloadState() != DS_CONNECTED
        )) {
        if(printDebug)
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: return false due to wrong state."));
		return false;
    }

	CPartFile* SwapTo = NULL;
	CPartFile* cur_file = NULL;
	POSITION finalpos = NULL;
	CTypedPtrList<CPtrList, CPartFile*>* usedList = NULL;

    if(allowSame && !bRemoveCompletely) {
        SwapTo = reqfile;
        if(printDebug)
            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: allowSame: File %s SourceReq: %s"), reqfile->GetFileName(), IsSourceRequestAllowed(reqfile)?_T("true"):_T("false"));
    }

    bool SwapToIsNNP = (SwapTo != NULL && SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS);

    CPartFile* skippedDueToSourceExchange = NULL;
    bool skippedIsNNP = false;

	if (!m_OtherRequests_list.IsEmpty()){
        if(printDebug)
            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: m_OtherRequests_list"));

		for (POSITION pos = m_OtherRequests_list.GetHeadPosition();pos != 0;m_OtherRequests_list.GetNext(pos)){
			cur_file = m_OtherRequests_list.GetAt(pos);

            if(printDebug)
                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Checking file: %s SoureReq: %s"), cur_file->GetFileName(), IsSourceRequestAllowed(cur_file)?_T("true"):_T("false"));

            if(!bRemoveCompletely && !ignoreSuspensions && allowSame && IsSwapSuspended(cur_file, doAgressiveSwapping, false)) {
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: continue due to IsSwapSuspended(file) == true"));
                continue;
            }

            if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY))	
			{
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: It's a partfile, not stopped, etc."));

				if (toFile != NULL){
					if (cur_file == toFile){
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Found toFile."));

                        SwapTo = cur_file;
                        SwapToIsNNP = false;
                		usedList = &m_OtherRequests_list;
						finalpos = pos;
						break;
					}
				} else {
                    bool wasSkippedDueToSourceExchange = false;
                    if(SwapToRightFile(SwapTo, cur_file, ignoreSuspensions, SwapToIsNNP, false, wasSkippedDueToSourceExchange, doAgressiveSwapping, debug)) {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapping to file %s"), cur_file->GetFileName());

                        if(SwapTo && wasSkippedDueToSourceExchange) {
                            if(debug && thePrefs.GetLogA4AF()) AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapped due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, SwapTo, ignoreSuspensions, skippedIsNNP, SwapToIsNNP, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = SwapTo;
                                skippedIsNNP = skippedIsNNP?true:(SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS);
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }

                        SwapTo = cur_file;
                        SwapToIsNNP = false;
                		usedList = &m_OtherRequests_list;
					    finalpos=pos;
                    } else {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Keeping file %s"), SwapTo->GetFileName());
                        if(wasSkippedDueToSourceExchange) {
                            if(printDebug)
                                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Kept the file due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, cur_file, ignoreSuspensions, skippedIsNNP, false, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = cur_file;
                                skippedIsNNP = false;
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }
                    }
                }
			}
		}
	}

    //if ((!SwapTo || SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS) && bIgnoreNoNeeded){
        if(printDebug)
            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: m_OtherNoNeeded_list"));

		for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos)){
			cur_file = m_OtherNoNeeded_list.GetAt(pos);

            if(printDebug)
                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Checking file: %s "), cur_file->GetFileName());

            if(!bRemoveCompletely && !ignoreSuspensions && allowSame && IsSwapSuspended(cur_file, doAgressiveSwapping, true)) {
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: continue due to !IsSwapSuspended(file) == true"));
                continue;
            }

			if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY) )	
			{
                if(printDebug)
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: It's a partfile, not stopped, etc."));

				if (toFile != NULL){
					if (cur_file == toFile){
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Found toFile."));
    
                        SwapTo = cur_file;
                		usedList = &m_OtherNoNeeded_list;
						finalpos = pos;
						break;
					}
				} else {
                    bool wasSkippedDueToSourceExchange = false;
                    if(SwapToRightFile(SwapTo, cur_file, ignoreSuspensions, SwapToIsNNP, true, wasSkippedDueToSourceExchange, doAgressiveSwapping, debug)) {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapping to file %s"), cur_file->GetFileName());

                        if(SwapTo && wasSkippedDueToSourceExchange) {
                            if(printDebug)
                                AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Swapped due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, SwapTo, ignoreSuspensions, skippedIsNNP, SwapToIsNNP, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = SwapTo;
                                skippedIsNNP = skippedIsNNP?true:(SwapTo == reqfile && GetDownloadState() == DS_NONEEDEDPARTS);
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }

                        SwapTo = cur_file;
                        SwapToIsNNP = true;
                		usedList = &m_OtherNoNeeded_list;
					    finalpos=pos;
                    } else {
                        if(printDebug)
                            AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Keeping file %s"), SwapTo->GetFileName());
                        if(wasSkippedDueToSourceExchange) {
                            if(debug && thePrefs.GetVerbose()) AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Kept the file due to source exchange possibility"));
                            bool discardSkipped = false;
                            if(SwapToRightFile(skippedDueToSourceExchange, cur_file, ignoreSuspensions, skippedIsNNP, true, discardSkipped, doAgressiveSwapping, debug)) {
                                skippedDueToSourceExchange = cur_file;
                                skippedIsNNP = true;
                                if(printDebug)
                                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Skipped file was better than last skipped file."));
                            }
                        }
                    }
				}
			}
		}
	//}

    if (SwapTo){
        if(printDebug) {
            if(SwapTo != reqfile) {
                AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Found file to swap to %s"), SwapTo->GetFileName());
            } else {
                AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Will keep current file. %s"), SwapTo->GetFileName());
            }
        }

		CString strInfo(reason);
        if(skippedDueToSourceExchange) {
            bool wasSkippedDueToSourceExchange = false;
            bool skippedIsBetter = SwapToRightFile(SwapTo, skippedDueToSourceExchange, ignoreSuspensions, SwapToIsNNP, skippedIsNNP, wasSkippedDueToSourceExchange, doAgressiveSwapping, debug);
            if(skippedIsBetter || wasSkippedDueToSourceExchange) {
                SwapTo->SetSwapForSourceExchangeTick();
                SetSwapForSourceExchangeTick();

                strInfo = _T("******SourceExchange-Swap****** ") + strInfo;
                if(printDebug) {
                    AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Due to sourceExchange."));
                } else if(thePrefs.GetLogA4AF() && reqfile == SwapTo) {
                    AddDebugLogLine(DLP_LOW, false, _T("ooo Didn't swap source due to source exchange possibility. %s Remove = %s '%s' Reason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), strInfo);
                }
            } else if(printDebug) {
				AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Normal. SwapTo better than skippedDueToSourceExchange."));
            }
        } else if(printDebug) {
			AddDebugLogLine(DLP_VERYLOW, false, _T("ooo Debug: Normal. skippedDueToSourceExchange == NULL"));
        }

		if (SwapTo != reqfile && DoSwap(SwapTo,bRemoveCompletely, strInfo)){
            if(debug && thePrefs.GetLogA4AF()) AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Swap successful."));
            if(usedList && finalpos) {
			    usedList->RemoveAt(finalpos);
            }
			return true;
        } else if(printDebug) {
            AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Swap didn't happen."));
        }
    }

    if(printDebug)
        AddDebugLogLine(DLP_LOW, false, _T("ooo Debug: Done %s"), DbgGetClientInfo());

	return false;
}

bool CUpDownClient::DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason)
{
    if (thePrefs.GetLogA4AF())
        AddDebugLogLine(DLP_LOW, false, _T("ooo Swapped source %s Remove = %s '%s'   -->   %s Reason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), SwapTo->GetFileName(), reason);

	// 17-Dez-2003 [bc]: This "reqfile->srclists[sourcesslot].Find(this)" was the only place where 
	// the usage of the "CPartFile::srclists[100]" is more effective than using one list. If this
	// function here is still (again) a performance problem there is a more effective way to handle
	// the 'Find' situation. Hint: usage of a node ptr which is stored in the CUpDownClient.
	POSITION pos = reqfile->srclist.Find(this);
	if(pos)
    {
		reqfile->RemoveSourceFileName(reqfile->srclist.GetAt(pos)); // EastShare       - FollowTheMajority by AndCycle
    	reqfile->srclist.RemoveAt(pos);
    } else {
        AddDebugLogLine(DLP_HIGH, true, _T("o-o Unsync between parfile->srclist and client otherfiles list. Swapping client where client has file as reqfile, but file doesn't have client in srclist. %s Remove = %s '%s'   -->   '%s'  SwapReason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), SwapTo->GetFileName(), reason);
    }

	// remove this client from the A4AF list of our new reqfile
	POSITION pos2 = SwapTo->A4AFsrclist.Find(this);
	if (pos2) {
		SwapTo->A4AFsrclist.RemoveAt(pos2);
    } else {
        AddDebugLogLine(DLP_HIGH, true, _T("o-o Unsync between parfile->srclist and client otherfiles list. Swapping client where client has file in another list, but file doesn't have client in a4af srclist. %s Remove = %s '%s'   -->   '%s'  SwapReason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), SwapTo->GetFileName(), reason);
    }
	theApp.emuledlg->transferwnd->GetDownloadList()->RemoveSource(this,SwapTo);

	reqfile->RemoveDownloadingSource(this);

	if(!bRemoveCompletely)
	{
        reqfile->A4AFsrclist.AddTail(this);
		if (GetDownloadState() == DS_NONEEDEDPARTS)
			m_OtherNoNeeded_list.AddTail(reqfile);
		else
			m_OtherRequests_list.AddTail(reqfile);

		theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(reqfile,this,true);
    } else {
        m_fileReaskTimes.RemoveKey(reqfile);
		//MORPH START - Changed by SiRoB, Keep A4AF infos
		uint8* PartStatus;
		if(m_PartStatus_list.Lookup(reqfile,PartStatus)){
			delete[] PartStatus;
			m_PartStatus_list.RemoveKey(reqfile);
		}
		m_nUpCompleteSourcesCount_list.RemoveKey(reqfile);
		//MORPH END   - Changed by SiRoB, Keep A4AF infos
    }

	SetDownloadState(DS_NONE);
	CPartFile* pOldRequestFile = reqfile;
	SetRequestFile(SwapTo);	
	pOldRequestFile->UpdatePartsInfo();
	pOldRequestFile->UpdateAvailablePartsCount();

	SwapTo->srclist.AddTail(this);
	theApp.emuledlg->transferwnd->GetDownloadList()->AddSource(SwapTo,this,false);

	return true;
}

void CUpDownClient::DontSwapTo(/*const*/ CPartFile* file)
{
	DWORD dwNow = ::GetTickCount();

	for (POSITION pos = m_DontSwap_list.GetHeadPosition(); pos != 0; m_DontSwap_list.GetNext(pos))
		if(m_DontSwap_list.GetAt(pos).file == file) {
			m_DontSwap_list.GetAt(pos).timestamp = dwNow ;
			return;
		}
	PartFileStamp newfs = {file, dwNow };
	m_DontSwap_list.AddHead(newfs);
}

bool CUpDownClient::IsSwapSuspended(const CPartFile* file, const bool allowShortReaskTime, const bool fileIsNNP)
{
    if(file == reqfile) {
        return false;
    }

    // Don't swap if we have reasked this client too recently
    if(GetTimeUntilReask(file, allowShortReaskTime, true, fileIsNNP) > 0)
        return true;

	if (m_DontSwap_list.GetCount()==0)
		return false;

	for (POSITION pos = m_DontSwap_list.GetHeadPosition(); pos != 0 && m_DontSwap_list.GetCount()>0; m_DontSwap_list.GetNext(pos)){
		if(m_DontSwap_list.GetAt(pos).file == file){
			if ( ::GetTickCount() - m_DontSwap_list.GetAt(pos).timestamp  >= PURGESOURCESWAPSTOP ) {
				m_DontSwap_list.RemoveAt(pos);
				return false;
			}
			else
				return true;
		}
		else if (m_DontSwap_list.GetAt(pos).file == NULL) // in which cases should this happen?
			m_DontSwap_list.RemoveAt(pos);
	}

	return false;
}

uint32 CUpDownClient::GetTimeUntilReask(const CPartFile* file, const bool allowShortReaskTime, const bool useGivenNNP, const bool givenNNP) const {
    DWORD lastAskedTimeTick = GetLastAskedTime(file);
    if(lastAskedTimeTick != 0) {
        DWORD tick = ::GetTickCount();

        DWORD reaskTime;
        if(allowShortReaskTime || file == reqfile && GetDownloadState() == DS_NONE) {
            reaskTime = MIN_REQUESTTIME;
        } else if(useGivenNNP && givenNNP ||
                  file == reqfile && GetDownloadState() == DS_NONEEDEDPARTS ||
                  file != reqfile && IsInNoNeededList(file)) {
            reaskTime = FILEREASKTIME*2;
        } else {
            reaskTime = FILEREASKTIME;
        }

        if(tick-lastAskedTimeTick < reaskTime) {
            return reaskTime-(tick-lastAskedTimeTick);
        } else {
            return 0;
        }
    } else {
        return 0;
    }
}

uint32 CUpDownClient::GetTimeUntilReask(const CPartFile* file) const {
    return GetTimeUntilReask(file, false);
}

uint32 CUpDownClient::GetTimeUntilReask() const {
    return GetTimeUntilReask(reqfile);
}

bool CUpDownClient::IsValidSource() const
{
	bool valid = false;
	switch(GetDownloadState())
	{
		case DS_DOWNLOADING:
		case DS_ONQUEUE:
		case DS_CONNECTED:
		case DS_NONEEDEDPARTS:
		case DS_REMOTEQUEUEFULL:
		case DS_REQHASHSET:
			valid = IsEd2kClient();
	}
	return valid;
}

void CUpDownClient::StartDownload()
{
	SetDownloadState(DS_DOWNLOADING);
	InitTransferredDownMini();
	SetDownStartTime();
	m_lastPartAsked = (uint16)-1;
	SendBlockRequests();
}

void CUpDownClient::SendCancelTransfer(Packet* packet)
{
	if (socket == NULL || !IsEd2kClient()){
		ASSERT(0);
		return;
	}
	
	if (!GetSentCancelTransfer())
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__CancelTransfer", this);

		bool bDeletePacket;
		Packet* pCancelTransferPacket;
		if (packet)
		{
			pCancelTransferPacket = packet;
			bDeletePacket = false;
		}
		else
		{
			pCancelTransferPacket = new Packet(OP_CANCELTRANSFER, 0);
			bDeletePacket = true;
		}
		theStats.AddUpDataOverheadFileRequest(pCancelTransferPacket->size);
		socket->SendPacket(pCancelTransferPacket,bDeletePacket,true);
		SetSentCancelTransfer(1);
	}

	if (m_pPCDownSocket)
	{
		m_pPCDownSocket->Safe_Delete();
		m_pPCDownSocket = NULL;
		SetPeerCacheDownState(PCDS_NONE);
	}
}

void CUpDownClient::SetRequestFile(CPartFile* pReqFile)
{
	if (pReqFile != reqfile || reqfile == NULL)
		ResetFileStatusInfo();
	//MORPH START - Added by SiRoB, Keep A4AF infos
	if (pReqFile != reqfile) {
		uint8* PartStatus;
		if (pReqFile == NULL) {
			if(m_PartStatus_list.Lookup(reqfile,PartStatus)){
				delete[] PartStatus;
				m_PartStatus_list.RemoveKey(reqfile);
			}
			m_nUpCompleteSourcesCount_list.RemoveKey(reqfile);
		} else {
			if(m_PartStatus_list.Lookup(pReqFile,PartStatus)){
				m_nPartCount = pReqFile->GetPartCount();
				m_abyPartStatus = PartStatus;
			}
			m_nUpCompleteSourcesCount_list.Lookup(pReqFile, m_nUpCompleteSourcesCount);
		}
	}
	//MORPH END   - Added by SiRoB, Keep A4AF infos
	
	reqfile = pReqFile;
}

void CUpDownClient::ProcessAcceptUpload()
{
	m_fQueueRankPending = 1;

	//EastShare Start - Added by AndCycle, Only download complete files v2.1 (shadow)
	/*
	if (reqfile && !reqfile->IsStopped() && (reqfile->GetStatus()==PS_READY || reqfile->GetStatus()==PS_EMPTY))
	*/
	if (reqfile && !reqfile->IsStopped() && (reqfile->GetStatus()==PS_READY || reqfile->GetStatus()==PS_EMPTY) && !reqfile->notSeenCompleteSource())
	//EastShare End - Added by AndCycle, Only download complete files v2.1 (shadow)
	{
		SetSentCancelTransfer(0);
		if (GetDownloadState() == DS_ONQUEUE)
		{
			// PC-TODO: If remote client does not answer the PeerCache query within a timeout, 
			// automatically fall back to ed2k download.
			if (   !SupportPeerCache()						// client knows peercache protocol
				|| !thePrefs.IsPeerCacheDownloadEnabled()	// user has enabled peercache downloads
				|| !theApp.m_pPeerCache->IsCacheAvailable() // we have found our cache and its usable
				|| !theApp.m_pPeerCache->IsClientPCCompatible(GetVersion(), GetClientSoft()) // the client version is accepted by the cache
				|| !SendPeerCacheFileRequest()) // request made
			{
				StartDownload();
			}
			//MORPH START - Added by SiRoB, Debug To catch the failed up/dw reason
			else{
				if(thePrefs.GetLogUlDlEvents()){
					DebugLog(DLP_VERYLOW,_T("---- %s: PeerCache Transfert Attempt ----"),DbgGetClientInfo());
				}
			}
			//MORPH END   - Added by SiRoB, Debug To catch the failed up/dw reason
		}
	}
	else
	{
		//MORPH START - Added by SiRoB, Debug To catch the failed up/dw reason
		if(thePrefs.GetLogUlDlEvents()){
			CString Buffer;
			if (!reqfile)
				Buffer = _T("reqfile == NULL");
			else if (!reqfile->IsStopped())
				Buffer = _T("file stopped");
			else if (reqfile->GetStatus()==PS_READY || reqfile->GetStatus()==PS_EMPTY)
				Buffer = reqfile->getPartfileStatus();
			else
				Buffer = _T("'Only download Complet file' is checked in pref");
			DebugLog(LOG_WARNING | DLP_VERYLOW,_T("---- %s: Accept Upload aborted: %s ----"),DbgGetClientInfo(),Buffer);
		}
		//MORPH END   - Added by SiRoB, Debug To catch the failed up/dw reason
		SendCancelTransfer();
		SetDownloadState((reqfile==NULL || reqfile->IsStopped()) ? DS_NONE : DS_ONQUEUE);
	}
}

void CUpDownClient::ProcessEdonkeyQueueRank(const uchar* packet, UINT size)
{
	CSafeMemFile data(packet, size);
	uint32 rank = data.ReadUInt32();
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  QR=%u (prev. %d)\n"), rank, IsRemoteQueueFull() ? (UINT)-1 : (UINT)GetRemoteQueueRank());
	SetRemoteQueueRank(rank, GetDownloadState() == DS_ONQUEUE);
	CheckQueueRankFlood();
}

void CUpDownClient::ProcessEmuleQueueRank(const uchar* packet, UINT size)
{
	if (size != 12)
		throw GetResString(IDS_ERR_BADSIZE);
	uint16 rank = PeekUInt16(packet);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  QR=%u\n"), rank); // no prev. QR available for eMule clients
	SetRemoteQueueFull(false);
	SetRemoteQueueRank(rank, GetDownloadState() == DS_ONQUEUE);
	CheckQueueRankFlood();
}

void CUpDownClient::CheckQueueRankFlood()
{
	if (m_fQueueRankPending == 0)
	{
		if (GetDownloadState() != DS_DOWNLOADING)
		{
			if (m_fUnaskQueueRankRecv < 3) // NOTE: Do not increase this nr. without increasing the bits for 'm_fUnaskQueueRankRecv'
			{ //MORPH
				m_fUnaskQueueRankRecv++;
			//MORPH START
				AddDebugLogLine(false, _T("Clients: %s (%s),m_fUnaskQueueRankRecv =%d (3+badrequest=ban) "), GetUserName(), ipstr(GetConnectIP()),m_fUnaskQueueRankRecv); //morph extra logging
			}
			//MORPH END
			if (m_fUnaskQueueRankRecv == 3)
			{
				if (theApp.clientlist->GetBadRequests(this) < 2)
					theApp.clientlist->TrackBadRequest(this, 1);
				if (theApp.clientlist->GetBadRequests(this) == 2){
					theApp.clientlist->TrackBadRequest(this, -2); // reset so the client will not be rebanned right after the ban is lifted
					Ban(_T("QR flood"));
				}
				throw CString(thePrefs.GetLogBannedClients() ? _T("QR flood") : _T(""));
			}
		}
	}
	else
	{
		m_fQueueRankPending = 0;
		m_fUnaskQueueRankRecv = 0;
	}
}

uint32 CUpDownClient::GetLastAskedTime(const CPartFile* partFile) const
{
	CPartFile* file = (CPartFile*)partFile;
	if (file == NULL) {
		file = reqfile;
	}

	DWORD lastChangedTick;
	return m_fileReaskTimes.Lookup(file, lastChangedTick)? lastChangedTick : 0;
}

void CUpDownClient::SetReqFileAICHHash(CAICHHash* val)
{	// TODO fileident optimize to save some memory
	if (m_pReqFileAICHHash != NULL && m_pReqFileAICHHash != val)
		delete m_pReqFileAICHHash;
	m_pReqFileAICHHash = val;
}

void CUpDownClient::SendAICHRequest(CPartFile* pForFile, uint16 nPart)
{
	CAICHRequestedData request;
	request.m_nPart = nPart;
	request.m_pClient = this;
	request.m_pPartFile = pForFile;
	CAICHRecoveryHashSet::m_liRequestedData.AddTail(request);
	m_fAICHRequested = TRUE;
	CSafeMemFile data;
	data.WriteHash16(pForFile->GetFileHash());
	data.WriteUInt16(nPart);
	pForFile->GetAICHRecoveryHashSet()->GetMasterHash().Write(&data);
	Packet* packet = new Packet(&data, OP_EMULEPROT, OP_AICHREQUEST);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__AichRequest", this, (uchar*)packet->pBuffer);
	theStats.AddUpDataOverheadFileRequest(packet->size);
	SafeConnectAndSendPacket(packet);
}

void CUpDownClient::ProcessAICHAnswer(const uchar* packet, UINT size)
{
	if (m_fAICHRequested == FALSE){
		throw CString(_T("Received unrequested AICH Packet"));
	}
	m_fAICHRequested = FALSE;

	CSafeMemFile data(packet, size);
	if (size <= 16){	
		CAICHRecoveryHashSet::ClientAICHRequestFailed(this);
		return;
	}
	uchar abyHash[16];
	data.ReadHash16(abyHash);
	CPartFile* pPartFile = theApp.downloadqueue->GetFileByID(abyHash);
	CAICHRequestedData request = CAICHRecoveryHashSet::GetAICHReqDetails(this);
	uint16 nPart = data.ReadUInt16();
	if (pPartFile != NULL && request.m_pPartFile == pPartFile && request.m_pClient == this && nPart == request.m_nPart){
		CAICHHash ahMasterHash(&data);
		if ( (pPartFile->GetAICHRecoveryHashSet()->GetStatus() == AICH_TRUSTED || pPartFile->GetAICHRecoveryHashSet()->GetStatus() == AICH_VERIFIED)
			 && ahMasterHash == pPartFile->GetAICHRecoveryHashSet()->GetMasterHash())
		{
			if(pPartFile->GetAICHRecoveryHashSet()->ReadRecoveryData((uint64)request.m_nPart*PARTSIZE, &data)){
				// finally all checks passed, everythings seem to be fine
// WebCache ////////////////////////////////////////////////////////////////////////////////////
				if(thePrefs.GetLogICHEvents()) //JP log ICH events
				AddDebugLogLine(DLP_DEFAULT, false, _T("AICH Packet Answer: Succeeded to read and validate received recoverydata"));
				CAICHRecoveryHashSet::RemoveClientAICHRequest(this);
				pPartFile->AICHRecoveryDataAvailable(request.m_nPart);
				return;
			}
			else
				DebugLogError(_T("AICH Packet Answer: Failed to read and validate received recoverydata"));
		}
		else
// WebCache ////////////////////////////////////////////////////////////////////////////////////
			if(thePrefs.GetLogICHEvents()) //JP log ICH events
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Answer: Masterhash differs from packethash or hashset has no trusted Masterhash"));
	}
	else
// WebCache ////////////////////////////////////////////////////////////////////////////////////
		if(thePrefs.GetLogICHEvents()) //JP log ICH events
		AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Answer: requested values differ from values in packet"));

	CAICHRecoveryHashSet::ClientAICHRequestFailed(this);
}

void CUpDownClient::ProcessAICHRequest(const uchar* packet, UINT size)
{
	if (size != (UINT)(16 + 2 + CAICHHash::GetHashSize()))
		throw CString(_T("Received AICH Request Packet with wrong size"));
	
	CSafeMemFile data(packet, size);
	uchar abyHash[16];
	data.ReadHash16(abyHash);
	uint16 nPart = data.ReadUInt16();
	CAICHHash ahMasterHash(&data);
	CKnownFile* pKnownFile = theApp.sharedfiles->GetFileByID(abyHash);
	if (pKnownFile != NULL){
		if (pKnownFile->IsAICHRecoverHashSetAvailable() && pKnownFile->GetFileIdentifier().HasAICHHash()
			&& pKnownFile->GetFileIdentifier().GetAICHHash() == ahMasterHash && pKnownFile->GetPartCount() > nPart
			&& pKnownFile->GetFileSize() > (uint64)EMBLOCKSIZE && (uint64)pKnownFile->GetFileSize() - PARTSIZE*(uint64)nPart > EMBLOCKSIZE)
		{
			CSafeMemFile fileResponse;
			fileResponse.WriteHash16(pKnownFile->GetFileHash());
			fileResponse.WriteUInt16(nPart);
			pKnownFile->GetFileIdentifier().GetAICHHash().Write(&fileResponse);
			CAICHRecoveryHashSet recHashSet(pKnownFile, pKnownFile->GetFileSize());
			recHashSet.SetMasterHash(pKnownFile->GetFileIdentifier().GetAICHHash(), AICH_HASHSETCOMPLETE);
			if (recHashSet.CreatePartRecoveryData((uint64)nPart*PARTSIZE, &fileResponse)){
// WebCache ////////////////////////////////////////////////////////////////////////////////////
				if(thePrefs.GetLogICHEvents()) //JP log ICH events
				AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Successfully created and send recoverydata for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__AichAnswer", this, pKnownFile->GetFileHash());
				Packet* packAnswer = new Packet(&fileResponse, OP_EMULEPROT, OP_AICHANSWER);
				theStats.AddUpDataOverheadFileRequest(packAnswer->size);
				SafeConnectAndSendPacket(packAnswer);
				return;
			}
			else
// WebCache ////////////////////////////////////////////////////////////////////////////////////
				if(thePrefs.GetLogICHEvents()) //JP log ICH events
				AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to create recoverydata for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
		}
		else{
// WebCache ////////////////////////////////////////////////////////////////////////////////////
			if(thePrefs.GetLogICHEvents()) //JP log ICH events
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to create recoverydata - Hashset not ready or requested Hash differs from Masterhash for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
		}

	}
	else
// WebCache ////////////////////////////////////////////////////////////////////////////////////
		if(thePrefs.GetLogICHEvents()) //JP log ICH events
		AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to find requested shared file -  %s"), DbgGetClientInfo());
	
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__AichAnswer", this, abyHash);
	Packet* packAnswer = new Packet(OP_AICHANSWER, 16, OP_EMULEPROT);
	md4cpy(packAnswer->pBuffer, abyHash);
	theStats.AddUpDataOverheadFileRequest(packAnswer->size);
	SafeConnectAndSendPacket(packAnswer);
}

void CUpDownClient::ProcessAICHFileHash(CSafeMemFile* data, CPartFile* file, const CAICHHash* pAICHHash)
{
	CPartFile* pPartFile = file;
	if (pPartFile == NULL && data != NULL){
		uchar abyHash[16];
		data->ReadHash16(abyHash);
		pPartFile = theApp.downloadqueue->GetFileByID(abyHash);
	}
	CAICHHash ahMasterHash;
	if (pAICHHash == NULL && data != NULL)
		ahMasterHash.Read(data);
	else
		ahMasterHash = *pAICHHash;
	if(pPartFile != NULL && pPartFile == GetRequestFile()){
		SetReqFileAICHHash(new CAICHHash(ahMasterHash));
		pPartFile->GetAICHRecoveryHashSet()->UntrustedHashReceived(ahMasterHash, GetConnectIP());

		if (pPartFile->GetFileIdentifierC().HasAICHHash() && pPartFile->GetFileIdentifierC().GetAICHHash() != ahMasterHash)
		{
			// this an legacy client and he sent us a hash different from our verified one, which menas the fileidentifiers
			// are different. We handle this just like a FNF-Answer to our downloadrequest and remove the client from our sourcelist, because we
			// sure don't want to download from him
			pPartFile->m_DeadSourceList.AddDeadSource(this);
			DebugLogWarning(_T("Client answered with different AICH hash than local verified on in ProcessAICHFileHash, removing source. File %s, client %s"), pPartFile->GetFileName(), DbgGetClientInfo());
			// if that client does not have my file maybe has another different
			// we try to swap to another file ignoring no needed parts files
			switch (GetDownloadState())
			{
				case DS_REQHASHSET:
					// for the love of eMule, don't accept a hashset from him :)
					if (m_fHashsetRequestingMD4)
					{
						DebugLogWarning(_T("... also cancelled hash set request from client due to AICH mismatch"));
						pPartFile->m_bMD4HashsetNeeded = true;
					}
					if (m_fHashsetRequestingAICH)
					{
						ASSERT( false );
						pPartFile->SetAICHHashSetNeeded(true);
					}
					m_fHashsetRequestingMD4 = false;
					m_fHashsetRequestingAICH = false;
				case DS_CONNECTED:
				case DS_ONQUEUE:
				case DS_NONEEDEDPARTS:
				case DS_DOWNLOADING:
					DontSwapTo(pPartFile); // ZZ:DownloadManager
					if (!SwapToAnotherFile(_T("Source says it doesn't have the file (AICH mismatch). CUpDownClient::ProcessAICHFileHash"), true, true, true, NULL, false, false)) { // ZZ:DownloadManager
						theApp.downloadqueue->RemoveSource(this);
					}
				return;
			}
		}
	}
	else
// WebCache ////////////////////////////////////////////////////////////////////////////////////
		if(thePrefs.GetLogICHEvents()) //JP log ICH events
		AddDebugLogLine(DLP_HIGH, false, _T("ProcessAICHFileHash(): PartFile not found or Partfile differs from requested file, %s"), DbgGetClientInfo());
}

void CUpDownClient::SendHashSetRequest()
{
	if (socket && socket->IsConnected())
	{
		Packet* packet = NULL;
		if (SupportsFileIdentifiers())
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__HashSetRequest2", this, reqfile->GetFileHash());
			CSafeMemFile filePacket(60);
			reqfile->GetFileIdentifier().WriteIdentifier(&filePacket);
			// 6 Request Options - RESERVED
			// 1 Request AICH HashSet
			// 1 Request MD4 HashSet
			uint8 byOptions = 0;
			if (reqfile->m_bMD4HashsetNeeded)
			{
				m_fHashsetRequestingMD4 = 1;
				byOptions |= 0x01;
				reqfile->m_bMD4HashsetNeeded = false;
			}
			if (reqfile->IsAICHPartHashSetNeeded() && GetReqFileAICHHash() != NULL && *GetReqFileAICHHash() == reqfile->GetFileIdentifier().GetAICHHash())
			{
				m_fHashsetRequestingAICH = 1;
				byOptions |= 0x02;
				reqfile->SetAICHHashSetNeeded(false);
			}
			if (byOptions == 0)
			{
				ASSERT( false );
				return;
			}
			DEBUG_ONLY( DebugLog(_T("Sending HashSet Request: MD4 %s, AICH %s to client %s"), m_fHashsetRequestingMD4 ? _T("Yes") : _T("No")
				, m_fHashsetRequestingAICH ? _T("Yes") : _T("No"), DbgGetClientInfo()) );
			filePacket.WriteUInt8(byOptions);
			packet = new Packet(&filePacket, OP_EMULEPROT, OP_HASHSETREQUEST2);
		}
		else
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__HashSetRequest", this, reqfile->GetFileHash());
			packet = new Packet(OP_HASHSETREQUEST,16);
			md4cpy(packet->pBuffer,reqfile->GetFileHash());
			m_fHashsetRequestingMD4 = 1;
			reqfile->m_bMD4HashsetNeeded = false;
		}
		theStats.AddUpDataOverheadFileRequest(packet->size);
		SendPacket(packet, true);
		SetDownloadState(DS_REQHASHSET);
	}
	else
		ASSERT(0);
}

//MORPH START - Added by SIRoB, GetAverage Donwload from client Wistily idea
uint32 CUpDownClient::GetAvDownDatarate() const
{
	uint32 tempDownCurrentTotalTime = GetDownTotalTime();
	if (GetDownloadState() == DS_DOWNLOADING)
		tempDownCurrentTotalTime += GetTickCount() - m_dwDownStartTime;
	if (tempDownCurrentTotalTime > 999)
		return	GetTransferredDown()/(tempDownCurrentTotalTime/1000);
	else
		return 0;
}
//MORPH END  - Added by SIRoB, GetAverage Donwload from client Wistily idea