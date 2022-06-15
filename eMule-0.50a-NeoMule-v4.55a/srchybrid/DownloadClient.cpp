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
#include "TransferWnd.h"
#include "PeerCacheFinder.h"
#include "Exceptions.h"
#include "clientlist.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Kademlia/Prefs.h"
#include "Kademlia/Kademlia/Search.h"
#include "SHAHashSet.h"
#include "SharedFileList.h"
#include "Log.h"
#include "Neo/ClientFileStatus.h"// NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#include "Neo/NeoOpcodes.h" // NEO: NMP - [NeoModProt] <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
#include "Neo/Sources/SourceList.h"
#endif // NEO_CD // NEO: SFL END <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
#include "Neo/Argos.h"
#endif // ARGOS // NEO: NA END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//	members of CUpDownClient
//	which are mainly used for downloading functions 
CBarShader CUpDownClient::s_StatusBar(16);
//void CUpDownClient::DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
void CUpDownClient::DrawStatusBar(CDC* dc, LPCRECT rect, CPartFile* file, bool  bFlat) const // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
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

	//ASSERT(reqfile);
	ASSERT(file); // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
	//s_StatusBar.SetFileSize(reqfile->GetFileSize());
	s_StatusBar.SetFileSize(file->GetFileSize()); // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
	s_StatusBar.SetHeight(rect->bottom - rect->top);
	s_StatusBar.SetWidth(rect->right - rect->left);
	s_StatusBar.Fill(crNeither);

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* status = GetFileStatus(file); // NEO: MFSB - [MultiFileStatusBars]
	uint16 m_nPartCount = status ? (uint16)status->GetPartCount() : 0;
	uint8* m_abyPartStatus = status ? status->GetPartStatus() : NULL;
	uint8* m_abyIncPartStatus = status ? status->GetPartStatus(CFS_Incomplete) : NULL; // NEO: ICS - [InteligentChunkSelection]
	uint8* m_abySeenPartStatus = status ? status->GetPartStatus(CFS_History) : NULL; // NEO: PSH - [PartStatusHistory]
	// NEO: SCFS END <-- Xanatos --

	uint16 CompleteParts = 0; // NEO: MOD - [Percentage] <-- Xanatos --

	//if (!onlygreyrect && reqfile && m_abyPartStatus)
	if (m_abyPartStatus // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
		|| m_abyIncPartStatus // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
		|| m_abySeenPartStatus  // NEO: PSH - [PartStatusHistory] <-- Xanatos --
	){
		COLORREF crBoth;
		COLORREF crClientOnly;
		COLORREF crClientOnlyBlocks; // NEO: SCT - [SubChunkTransfer] <-- Xanatos --
		COLORREF crPending;
		COLORREF crNextPending;
		COLORREF crClientPartial; // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
		COLORREF crSeen; // NEO: PSH - [PartStatusHistory] <-- Xanatos --
		COLORREF crMeOnly; // NEO: MOD - [SeeTheNeed] <-- Xanatos --
		if (g_bLowColorDesktop) {
			crBoth = RGB(0, 0, 0);
			crClientOnly = RGB(0, 0, 255);
			crClientOnlyBlocks = RGB(128, 128, 255); // NEO: SCT - [SubChunkTransfer] <-- Xanatos --
			crPending = RGB(0, 255, 0); 
			crNextPending = RGB(255, 255, 0);
			crClientPartial = RGB(170,50,224); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
			crSeen = RGB(255,240,240); // NEO: PSH - [PartStatusHistory] <-- Xanatos --
			crMeOnly = RGB(112,112,112); // NEO: MOD - [SeeTheNeed] <-- Xanatos --
		} else if (bFlat) {
			crBoth = RGB(0, 0, 0);
			crClientOnly = RGB(0, 100, 255);
			crClientOnlyBlocks = RGB(128, 128, 255); // NEO: SCT - [SubChunkTransfer] <-- Xanatos --
			crPending = RGB(0, 150, 0);
			crNextPending = RGB(255, 208, 0);
			crClientPartial = RGB(170,50,224); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
			crSeen = RGB(255,240,240); // NEO: PSH - [PartStatusHistory] <-- Xanatos --
			crMeOnly = RGB(112,112,112); // NEO: MOD - [SeeTheNeed] <-- Xanatos --
		} else {
			crBoth = RGB(104, 104, 104);
			crClientOnlyBlocks = RGB(128, 128, 255); // NEO: SCT - [SubChunkTransfer] <-- Xanatos --
			crClientOnly = RGB(0, 100, 255);
			crPending = RGB(0, 150, 0);
			crNextPending = RGB(255, 208, 0);
			crClientPartial = RGB(170,50,224); // NEO: ICS - [InteligentChunkSelection] <-- Xanatos --
			crSeen = RGB(255,240,240); // NEO: PSH - [PartStatusHistory] <-- Xanatos --
			crMeOnly = RGB(112,112,112); // NEO: MOD - [SeeTheNeed] <-- Xanatos --
		}

		char* pcNextPendingBlks = NULL;
		//if (m_nDownloadState == DS_DOWNLOADING){
		if(file == reqfile && m_nDownloadState == DS_DOWNLOADING){ // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
			pcNextPendingBlks = new char[m_nPartCount];
			memset(pcNextPendingBlks, 'N', m_nPartCount); // do not use '_strnset' for uninitialized memory!
			for (POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != 0; ){
				UINT uPart = (UINT)(m_PendingBlocks_list.GetNext(pos)->block->StartOffset / PARTSIZE);
				if (uPart < m_nPartCount)
					pcNextPendingBlks[uPart] = 'Y';
			}
		}

		tBlockMap* BlockMap; // NEO: SCT - [SubChunkTransfer] <-- Xanatos --

		for (UINT i = 0; i < m_nPartCount; i++){
			//if (m_abyPartStatus[i]){
			if (m_abyPartStatus && m_abyPartStatus[i]){  // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
				uint64 uEnd;
				//if ( PARTSIZE*(uint64)(i+1) > reqfile->GetFileSize())
				if ( PARTSIZE*(uint64)(i+1) > file->GetFileSize()) // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
					//uEnd = reqfile->GetFileSize();
					uEnd = file->GetFileSize(); // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
				else
					uEnd = PARTSIZE*(uint64)(i+1);

				//if (reqfile->IsComplete(PARTSIZE*(uint64)i,PARTSIZE*(uint64)(i+1)-1, false))
				if (file->IsComplete(PARTSIZE*(uint64)i,PARTSIZE*(uint64)(i+1)-1, false)) // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crBoth);
				//else if (GetSessionDown() > 0 && m_nDownloadState == DS_DOWNLOADING && m_nLastBlockOffset >= PARTSIZE*(uint64)i && m_nLastBlockOffset < uEnd)
				else if (file == reqfile && // NEO: MFSB - [MultiFileStatusBars] <-- Xanatos --
					      (GetSessionDown() > 0 && m_nDownloadState == DS_DOWNLOADING && m_nLastBlockOffset >= PARTSIZE*(uint64)i && m_nLastBlockOffset < uEnd))
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crPending);
				else if (pcNextPendingBlks != NULL && pcNextPendingBlks[i] == 'Y')
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crNextPending);
				else
					s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crClientOnly);

				CompleteParts ++; // NEO: MOD - [Percentage] <-- Xanatos --
			}
			// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
			else if (m_abyIncPartStatus && m_abyIncPartStatus[i]){
				uint64 uEnd;
				if ((uint32)PARTSIZE*(uint64)(i+1) > file->GetFileSize()) 
					uEnd = file->GetFileSize(); 
				else 
					uEnd = PARTSIZE*(uint64)(i+1); 
				s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crClientPartial);
			}
			// NEO: ICS END <-- Xanatos --
			// NEO: SCT - [SubChunkTransfer] -- Xanatos -->
			else if (status->GetBlockMap(i,&BlockMap))
			{
				for (uint8 j = 0; j < 53; j++)
				{
					uint64 uStart;
					if(PARTSIZE*(uint64)i + EMBLOCKSIZE*(uint64)j < PARTSIZE*(uint64)i)
						uStart = PARTSIZE*(uint64)i;
					else
						uStart = PARTSIZE*(uint64)i + EMBLOCKSIZE*(uint64)j;
					uint64 uEnd;
					if (PARTSIZE*(uint64)i + EMBLOCKSIZE*(uint64)(j+1) > (uint64) file->GetFileSize())
						uEnd = file->GetFileSize();
					else
						uEnd = PARTSIZE*(uint64)i + EMBLOCKSIZE*(uint64)(j+1);
					if (BlockMap->IsBlockDone(j))
					{
						if (file->IsComplete(uStart, uEnd - 1, false))
							s_StatusBar.FillRange(uStart, uEnd, crBoth);
						else if (m_nDownloadState == DS_DOWNLOADING && m_nLastBlockOffset >= uStart && m_nLastBlockOffset < uEnd  && file == reqfile)
							s_StatusBar.FillRange(uStart, uEnd, crPending);
						else if (pcNextPendingBlks != NULL && pcNextPendingBlks[i] == 'Y')
							s_StatusBar.FillRange(uStart, uEnd, crNextPending);
						else
							s_StatusBar.FillRange(uStart, uEnd, crClientOnlyBlocks);
					}
				}
			}
			// NEO: SCT END <-- Xanatos --
			// NEO: PSH - [PartStatusHistory] -- Xanatos -->
			else if (m_abySeenPartStatus && m_abySeenPartStatus[i]){
				uint64 uEnd;
				if (PARTSIZE*(uint64)(i+1) > file->GetFileSize())
					uEnd = file->GetFileSize(); 
				else 
					uEnd = PARTSIZE*(uint64)(i+1); 

				s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crSeen);				
			}
			// NEO: PSH END <-- Xanatos --
			// NEO: MOD - [SeeTheNeed] -- Xanatos -->
			else if (file->IsComplete(PARTSIZE*(uint64)i,PARTSIZE*(uint64)(i+1)-1,false)){ 
				uint64 uEnd; 
				if ((uint64)(PARTSIZE*(uint64)(i+1)) > file->GetFileSize()) 
					uEnd = file->GetFileSize(); 
				else 
					uEnd = PARTSIZE*(uint64)(i+1); 

				s_StatusBar.FillRange(PARTSIZE*(uint64)i, uEnd, crMeOnly);
			} 
			// NEO: MOD END <-- Xanatos --
		}
		delete[] pcNextPendingBlks;
	}
	s_StatusBar.Draw(dc, rect->left, rect->top, bFlat);

	// NEO: MOD - [Percentage] -- Xanatos -->
	if(NeoPrefs.ShowClientPercentage() && file->GetPartCount())
	{
		float percent = (float)CompleteParts*100.0f/file->GetPartCount();
		if (percent > 0.05f)
		{
			CString buffer;
			CRect rc(rect);
			COLORREF oldclr = dc->SetTextColor(RGB(0,0,0));
			int iOMode = dc->SetBkMode(TRANSPARENT);
			buffer.Format(_T("%.1f%%"), percent);
			CFont *pOldFont = dc->SelectObject(&theApp.emuledlg->transferwnd->downloadlistctrl.m_fontSmall);

			#define DLC_DT_TEXT (DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_NOPREFIX|DT_END_ELLIPSIS)
			rc.OffsetRect(-1,0);
			dc->DrawText(buffer, buffer.GetLength(), rc, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			rc.OffsetRect(2,0);
			dc->DrawText(buffer, buffer.GetLength(), rc, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			rc.OffsetRect(-1,-1);
			dc->DrawText(buffer, buffer.GetLength(), rc, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			rc.OffsetRect(0,2);
			dc->DrawText(buffer, buffer.GetLength(), rc, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);
			rc.OffsetRect(0,-1);
			dc->SetTextColor(RGB(230,230,230));
			dc->DrawText(buffer, buffer.GetLength(), rc, (DLC_DT_TEXT & ~DT_LEFT) | DT_CENTER);

			dc->SelectObject(pOldFont);
			dc->SetBkMode(iOMode);
			dc->SetTextColor(oldclr);
		}
	}
	// NEO: MOD END <-- Xanatos --
} 

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
	// NEO: SD - [StandByDL] -- Xanatos -->
	if(m_byFileRequestState == FR_STANDBY && (socket && socket->IsConnected())){
		SendStartupLoadReq(); // we have a socket and we had asked, so we can nw finish the reask
		return true;
	}
	// NEO: SD END <-- Xanatos -->
    SwapToAnotherFile(_T("A4AF check before tcp file reask. CUpDownClient::AskForDownload()"), true, false, false, NULL, true, true);
	SetDownloadState(DS_CONNECTING);
	return TryToConnect();
}

bool CUpDownClient::IsSourceRequestAllowed() const
{
	// NEO: MSR - [ManualSourceRequest] -- Xanatos -->
	if(reqfile->IsCollectXSSources())
		return true;
	// NEO: MSR END <-- Xanatos --
    return IsSourceRequestAllowed(reqfile);
}

bool CUpDownClient::IsSourceRequestAllowed(CPartFile* partfile, bool sourceExchangeCheck) const
{
	DWORD dwTickCount = ::GetTickCount() + CONNECTION_LATENCY;
	//unsigned int nTimePassedClient = dwTickCount - GetLastSrcAnswerTime();
	unsigned int nTimePassedClient = dwTickCount - GetLastAskedForSources(); // GetLastSrcAnswerTime(); // NEO: FIX - [XsInconsistency] <-- Xanatos --
	unsigned int nTimePassedFile   = dwTickCount - partfile->GetLastAnsweredTime();
	bool bNeverAskedBefore = GetLastAskedForSources() == 0;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* status = GetFileStatus(partfile);
	bool m_bCompleteSource = (status && status->IsCompleteSource());
	// NEO: SCFS END <-- Xanatos --
	UINT uSources = partfile->GetSourceCount();
    UINT uValidSources = partfile->GetValidSourcesCount();

    if (partfile != reqfile) {
        uSources++;
        uValidSources++;
    }

    UINT uReqValidSources = reqfile->GetValidSourcesCount();

	// NEO: SRT - [SourceRequestTweaks] -- Xanatos -->
	if(!partfile->PartPrefs->IsXsEnable() || partfile->IsCollectingHalted()) // NEO: XSC - [ExtremeSourceCache]
		return false;

	UINT iLimit = partfile->GetXsSourceLimit();
	UINT iIntervals = partfile->PartPrefs->GetXsIntervalsMs();
	UINT iCLientIntervals = partfile->PartPrefs->GetXsClientIntervalsMs();
	UINT iRateFile = partfile->PartPrefs->GetXsRareLimit();
	UINT iXSDelay = partfile->PartPrefs->GetXsCleintDelay();
	// NEO: END <-- Xanatos --

	return (
	         //if client has the correct extended protocol
	         ExtProtocolAvailable() && (SupportsSourceExchange2() || GetSourceExchange1Version() > 1) &&
	         //AND if we need more sources
	         iLimit > uSources && // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
	         //AND if...
	         (
	           //source is not complete and file is very rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > iCLientIntervals /*SOURCECLIENTREASKS*/) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
			     && (uSources <= (UINT)iRateFile /*RARE_FILE*/ /5)
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < uReqValidSources && uReqValidSources > 3)
	           ) ||
	           //source is not complete and file is rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > iCLientIntervals /*SOURCECLIENTREASKS*/) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
			     && (uSources <= iRateFile /*RARE_FILE*/  || (!sourceExchangeCheck || partfile == reqfile) && uSources <= RARE_FILE / 2 + uValidSources)
				 && (nTimePassedFile > iIntervals /*SOURCECLIENTREASKF*/) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < /*SOURCECLIENTREASKS*/iCLientIntervals/iIntervals/*SOURCECLIENTREASKF*/ && uValidSources < uReqValidSources) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
	           ) ||
	           // OR if file is not rare
			   ( (bNeverAskedBefore || nTimePassedClient > (unsigned)(iCLientIntervals /*SOURCECLIENTREASKS*/ * iXSDelay /*MINCOMMONPENALTY*/)) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
				 && (nTimePassedFile > (unsigned)(/*SOURCECLIENTREASKF*/ iIntervals * iXSDelay /*MINCOMMONPENALTY*/)) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
				 && (!sourceExchangeCheck || partfile == reqfile || uValidSources < /*SOURCECLIENTREASKS*/iCLientIntervals/iIntervals/*SOURCECLIENTREASKF*/ && uValidSources < uReqValidSources) // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
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

	CSafeMemFile dataFileReq(16+16);
	dataFileReq.WriteHash16(reqfile->GetFileHash());

	if (SupportMultiPacket())
	{
		bool bUseExtMultiPacket = SupportExtMultiPacket();
		if (bUseExtMultiPacket){
			dataFileReq.WriteUInt64(reqfile->GetFileSize());
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__MultiPacket_Ext", this, reqfile->GetFileHash());
		}
		else{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__MultiPacket", this, reqfile->GetFileHash());
		}

		// OP_REQUESTFILENAME + ExtInfo
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__MPReqFileName", this, reqfile->GetFileHash());
		dataFileReq.WriteUInt8(OP_REQUESTFILENAME);
		if (GetExtendedRequestsVersion() > 0)
			//reqfile->WritePartStatus(&dataFileReq);
			reqfile->WritePartStatus(&dataFileReq, this); // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
		if (GetExtendedRequestsVersion() > 1)
			reqfile->WriteCompleteSourcesCount(&dataFileReq);

		// OP_SETREQFILEID
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__MPSetReqFileID", this, reqfile->GetFileHash());
		if (reqfile->GetPartCount() > 1)
			dataFileReq.WriteUInt8(OP_SETREQFILEID);

		if (IsEmuleClient())
		{
			//SetRemoteQueueFull(true); // NEO: FIX - [SourceCount] <-- Xanatos --
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
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
			if(NeoPrefs.IsXsExploitDetection()){
				CClientFileStatus* status = GetFileStatus(reqfile); // NEO: SCFS - [SmartClientFileStatus]
				// David: because official client has a bug, only count the XSReqs under special conditions
				// Note: This will be fixed in future official releases so we will need to add a version check here to
				// remark: this fails for complete sources the first time.. but this doesn't hurt
				if(((status && status->IsCompleteSource()) || !md4cmp(GetUploadFileID(), reqfile->GetFileHash())) //complete source of we want the same file
				&& reqfile->GetValidSourcesCount() > 10) //if we know at least 10 good sources, the remote client should know at least one
					IncXSReqs();
			}
#endif // ARGOS // NEO: NA END <-- Xanatos --
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

		// OP_AICHFILEHASHREQ
		if (IsSupportingAICH())
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP__MPAichFileHashReq", this, reqfile->GetFileHash());
			dataFileReq.WriteUInt8(OP_AICHFILEHASHREQ);
		}

		Packet* packet = new Packet(&dataFileReq, OP_EMULEPROT);
		packet->opcode = bUseExtMultiPacket ? OP_MULTIPACKET_EXT : OP_MULTIPACKET;
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true);
	}
	else
	{
		//This is extended information
		if (GetExtendedRequestsVersion() > 0)
			//reqfile->WritePartStatus(&dataFileReq);
			reqfile->WritePartStatus(&dataFileReq, this); // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
		if (GetExtendedRequestsVersion() > 1)
			reqfile->WriteCompleteSourcesCount(&dataFileReq);
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__FileRequest", this, reqfile->GetFileHash());
		Packet* packet = new Packet(&dataFileReq);
		packet->opcode = OP_REQUESTFILENAME;
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet, true);
	
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
		    socket->SendPacket(packet, true);
		}
	
		if (IsEmuleClient())
		{
			//SetRemoteQueueFull(true); // NEO: FIX - [SourceCount] <-- Xanatos --
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
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
			if(NeoPrefs.IsXsExploitDetection()){
				CClientFileStatus* status = GetFileStatus(reqfile); // NEO: SCFS - [SmartClientFileStatus]
				// David: because official client has a bug, only count the XSReqs under special conditions
				// Note: This will be fixed in future official releases so we will need to add a version check here to
				// remark: this fails for complete sources the first time.. but this doesn't hurt
				if(((status && status->IsCompleteSource()) || !md4cmp(GetUploadFileID(), reqfile->GetFileHash())) //complete source of we want the same file
				&& reqfile->GetValidSourcesCount() > 10) //if we know at least 10 good sources, the remote client should know at least one
					IncXSReqs();
			}
#endif // ARGOS // NEO: NA END <-- Xanatos --
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
			socket->SendPacket(packet, true, true);
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
			socket->SendPacket(packet,true,true);
		}
	}

	// NEO: NMPm - [NeoModProtMultiPacket] -- Xanatos -->
	if(SupportsModProt())
	{
		CSafeMemFile dataMod(16+16);
		dataMod.WriteHash16(reqfile->GetFileHash());

		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__ModMultiPacket", this, reqfile->GetFileHash());

		WriteModMultiPacket(&dataMod, reqfile, true);

		if(dataMod.GetLength() > 16){ // send it only when we realy wroten something
			Packet* modpacket = new Packet(&dataMod, OP_MODPROT);
			modpacket->opcode = OP_MODMULTIPACKET;
			theStats.AddUpDataOverheadFileRequest(modpacket->size);
			socket->SendPacket(modpacket, true);
		}
	}
	// NEO: NMPm END <-- Xanatos --

    SetLastAskedTime();
	// NEO: SR - [SpreadReask] -- Xanatos -->
	if(NeoPrefs.PartPrefs.UseSpreadReaskEnable())
		SetSpreadReaskModyfier();
	// NEO: SR END <-- Xanatos --

	m_byFileRequestState = (uint8) (reqfile->IsStandBy() ? FR_STANDBY : FR_INPROGRES); // NEO: SD - [StandByDL] <-- Xanatos --
}

void CUpDownClient::SendStartupLoadReq()
{
	if (socket==NULL || reqfile==NULL)
	{
		ASSERT(0);
		return;
	}
	//SetDownloadState(DS_ONQUEUE);
	SetDownloadState(IsEmuleClient() ? DS_REMOTEQUEUEFULL : DS_ONQUEUE); // NEO: FIX - [SourceCount] <-- Xanatos --
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__StartupLoadReq", this);
	CSafeMemFile dataStartupLoadReq(16);
	dataStartupLoadReq.WriteHash16(reqfile->GetFileHash());
	Packet* packet = new Packet(&dataStartupLoadReq);
	packet->opcode = OP_STARTUPLOADREQ;
	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet, true, true);
	m_fQueueRankPending = 1;
	m_fUnaskQueueRankRecv = 0;

	m_byFileRequestState = FR_COMPLETED; // NEO: USPS - [UnSolicitedPartStatus] <-- Xanatos --
}

void CUpDownClient::ProcessFileInfo(CSafeMemFile* data, CPartFile* file)
{
	if (file==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; file==NULL)");
	if (reqfile==NULL)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile==NULL)");
	if (file != reqfile)
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileInfo; reqfile!=file)");

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* status = GetFileStatus(file,true);

	CString	m_strClientFilename = data->ReadString(GetUnicodeSupport()!=utf8strNone);
	status->SetFileName(m_strClientFilename);
	// NEO: SCFS END <-- Xanatos --

	//m_strClientFilename = data->ReadString(GetUnicodeSupport()!=utf8strNone);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  Filename=\"%s\"\n"), m_strClientFilename);
	// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
	// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
	// know that the file is shared, we know also that the file is complete and don't need to request the file status.
	if (reqfile->GetPartCount() == 1)
	{
		status->FillDefault(); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --

		/*delete[] m_abyPartStatus;
		m_abyPartStatus = NULL;
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		memset(m_abyPartStatus,1,m_nPartCount);
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
				psz[i] = m_abyPartStatus[i] ? '#' : '.';
			psz[i] = '\0';
			Debug(_T("  Parts=%u  %hs  Needed=%u\n"), m_nPartCount, psz, iNeeded);
			delete[] psz;
		}*/
		UpdateDisplayedInfo();
		reqfile->UpdateAvailablePartsCount();
		// even if the file is <= PARTSIZE, we _may_ need the hashset for that file (if the file size == PARTSIZE)
		if (reqfile->hashsetneeded)
		{
			RequestHashset(); // NEO: MOD - [RequestHashset] <-- Xanaots --
		}
		// NEO: SD - [StandByDL] -- Xanatos -->
		else if(m_byFileRequestState == FR_STANDBY)
		{
			SetDownloadState(DS_HALTED);
		}
		// NEO: SD END <-- Xanatos -->
		//else
		else if(m_byFileRequestState == FR_INPROGRES) // NEO: USPS - [UnSolicitedPartStatus] <-- Xanatos --
		{
			SendStartupLoadReq();
		}
		reqfile->UpdatePartsInfo();
		// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
		if(GetIncompletePartVersion() && NeoPrefs.UseIncompletePartStatus())
			reqfile->UpdatePartsInfoEx(CFS_Incomplete);
		// NEO: ICS END <-- Xanatos --
	}

	// NEO: XC - [ExtendedComments] -- Xanatos -->
	if(IsExtendedComments())
		SendCommentInfo(file);
	// NEO: XC END <-- Xanatos --
}

void CUpDownClient::ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file)
{
	if ( !reqfile || file != reqfile )
	{
		if (reqfile==NULL)
			throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile==NULL)");
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile!=file)");
	}

	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* status = GetFileStatus(file, true);

	status->ReadFileStatus(data,CFS_Normal);

	bool bPartsNeeded = false;

	uint8* abyPartStatus = status->GetPartStatus();
	if(abyPartStatus)
	{
		UINT PartCount = status->GetPartCount();
		for (UINT i = 0; i < PartCount; i++)
		{
			if (abyPartStatus[i])
			{
				if (!file->IsComplete((uint64)i*PARTSIZE, ((uint64)(i+1)*PARTSIZE)-1, false)){
					bPartsNeeded = true;
					break;
					//iNeeded++;
				}
			}
		}
	}
	// NEO: SCFS END <-- Xanatos --

	/*uint16 nED2KPartCount = data->ReadUInt16();
	delete[] m_abyPartStatus;
	m_abyPartStatus = NULL;
	bool bPartsNeeded = false;
	int iNeeded = 0;
	if (!nED2KPartCount)
	{
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		memset(m_abyPartStatus, 1, m_nPartCount);
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
		UINT done = 0;
		while (done != m_nPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (UINT i = 0; i != 8; i++)
			{
				m_abyPartStatus[done] = ((toread>>i)&1)? 1:0; 	
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
			psz[i] = m_abyPartStatus[i] ? _T('#') : _T('.');
		psz[i] = _T('\0');
		Debug(_T("  Parts=%u  %s  Needed=%u\n"), m_nPartCount, psz, iNeeded);
		delete[] psz;
	}*/

	UpdateDisplayedInfo(bUdpPacket);
	/*req*/file->UpdateAvailablePartsCount(); // NEO: CI#4 - [CodeImprovement] <-- Xanatos --

	HandleFileStatus(file, bUdpPacket, bPartsNeeded); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --

	reqfile->UpdatePartsInfo();
}

// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
void CUpDownClient::ProcessModFileStatus(bool bUdpPacket, CPartFile* file)
{
	CClientFileStatus* status = GetFileStatus(file); // NEO: SCFS - [SmartClientFileStatus]
	if(!status)
		return;

	if(GetDownloadState() != DS_NONEEDEDPARTS)
		return; // nothing to do here
	else if(file != reqfile && reqfile) // if we have swpaed to an other file
		return; // we also does have nithing to do here

	if (reqfile==NULL) 
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessModStatus; reqfile==NULL)");

	bool bPartsNeeded = false;

	// NEO: SCT - [SubChunkTransfer] 
	if(NeoPrefs.UseSubChunkTransfer() == TRUE && SupportsSubChunks()) // only when needed/supported
	{
		UINT PartCount = status->GetPartCount();
		for (UINT uPart = 0; uPart < PartCount; uPart++)
		{
			tBlockMap* RemoteBlockMap = NULL;
			if (status->GetBlockMap(uPart,&RemoteBlockMap))
			{
				tBlockMap* LocalBlockMap = NULL;
				if(file->GetBlockMap((uint16)uPart,&LocalBlockMap))
				{
					for(uint8 uBlock = 0; uBlock < 53;uBlock++)
					{
						if (RemoteBlockMap->IsBlockDone(uBlock) && !LocalBlockMap->IsBlockDone(uBlock))
						{
							bPartsNeeded = true;
						}
					}
				}
				else if(file->IsPureGap((uint64)uPart*PARTSIZE, (uint64)(uPart + 1)*PARTSIZE - 1)) // at this point it can be only pureGap or Complete
				{
					bPartsNeeded = true;
				}
			}
		}
	}
	// NEO: SCT END
	
	if(bPartsNeeded) // only when we found a part that interrests us
		HandleFileStatus(file, bUdpPacket, true);
}
// NEO: SCFS END <-- Xanatos --

void CUpDownClient::HandleFileStatus(CPartFile* file, bool bUdpPacket, bool bPartsNeeded, bool bCheckPassive) // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
{
	// NEO: BPS - [BetterPassiveSourceFinding] -- Xanatos -->
	ASSERT(bCheckPassive || reqfile == file);

	//Xman better passive source finding
	//problem is: if a client just began to download a file, we receive an FNF
	//later, if it has some chunks we don't find it via passive source finding because 
	//that works only on TCP-reask but not via UDP
	if(bCheckPassive)
	{
		if(bPartsNeeded)
		{
			//the client was a NNS but isn't any more
			if(GetDownloadState() == DS_NONEEDEDPARTS && reqfile == file)
				SetSafeReAskTime();
			else
			//the client maybe isn't in our downloadqueue.. let's look if we should add the client
			if (file->GetMaxSources() > file->GetSourceCount()) // NEO: NST - [NeoSourceTweaks]
				theApp.downloadqueue->CheckAndAddKnownSource(file, this, true);
		}
	}
	else
	// NEO: BPS END <-- Xanatos --

	// NOTE: This function is invoked from TCP and UDP socket!
	if (!bUdpPacket)
	{
		if (!bPartsNeeded)
        {
			SetDownloadState(DS_NONEEDEDPARTS);
            SwapToAnotherFile(_T("A4AF for NNP file. CUpDownClient::ProcessFileStatus() TCP"), true, false, false, NULL, true, true);
        }
        else if (reqfile->hashsetneeded) //If we are using the eMule filerequest packets, this is taken care of in the Multipacket!
		{
			RequestHashset(); // NEO: MOD - [RequestHashset] <-- Xanaots --
		}
		// NEO: SD - [StandByDL] -- Xanatos -->
		else if(m_byFileRequestState == FR_STANDBY)
		{
			SetDownloadState(DS_HALTED);
		}
		// NEO: SD END <-- Xanatos -->
		//else
		else if(m_byFileRequestState == FR_INPROGRES) // NEO: USPS - [UnSolicitedPartStatus] <-- Xanaots --
		{
			SendStartupLoadReq();
		}
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
}

// NEO: MOD - [RequestHashset] -- Xanaots -->
void CUpDownClient::RequestHashset()
{
	if (socket)
	{
		if (thePrefs.GetDebugClientTCPLevel() > 0)
			DebugSend("OP__HashSetRequest", this, reqfile->GetFileHash());
		Packet* packet = new Packet(OP_HASHSETREQUEST,16);
		md4cpy(packet->pBuffer,reqfile->GetFileHash());
		theStats.AddUpDataOverheadFileRequest(packet->size);
		socket->SendPacket(packet,true,true);
		SetDownloadState(DS_REQHASHSET);
		m_fHashsetRequesting = 1;
		reqfile->hashsetneeded = false;
	}
	else
		ASSERT(0);
}
// NEO: MOD END <-- Xanaots --


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

#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
	if(NeoPrefs.SaveSourceFileList() && source)
		source->AddSeenFile(file->GetFileHash(),file->GetFileSize());
#endif // NEO_CD // NEO: SFL END <-- Xanatos --

	return true;
}

void CUpDownClient::ClearDownloadBlockRequests()
{
	// NEO: DBR - [DynamicBlockRequest] -- Xanatos --
	//for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;){
	//	Requested_Block_Struct* cur_block = m_DownloadBlocks_list.GetNext(pos);
	//	if (reqfile){
	//		reqfile->RemoveBlockFromList(cur_block->StartOffset,cur_block->EndOffset);
	//	}
	//	delete cur_block;
	//}
	//m_DownloadBlocks_list.RemoveAll();

	for (POSITION pos = m_PendingBlocks_list.GetHeadPosition();pos != 0;){
		Pending_Block_Struct *pending = m_PendingBlocks_list.GetNext(pos);
		if (reqfile){
			reqfile->RemoveBlockFromList(pending->block->StartOffset, pending->block->EndOffset);
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
			case DS_HALTED: // NEO: SD - [StandByDL] <-- Xanatos --
			case DS_TOOMANYCONNSKAD:
				//This client had already been set to DS_CONNECTING.
				//So we reset this time so it isn't stuck at TOOMANYCONNS for 20mins.
				//m_dwLastTriedToConnect = ::GetTickCount()-20*60*1000;
				m_dwLastTriedToConnect = 0; // NEO: DRT - [DownloadReaskTweaks] <-- Xanatos --
				break;
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage] -- Xanatos -->
			case DS_LOADED:
#endif // NEO_SS // NEO: NSS END <-- Xanatos --
			case DS_WAITCALLBACKKAD:
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
			case DS_WAITCALLBACKXS:
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
			case DS_WAITCALLBACK:
				break;
			// NEO: XSC - [ExtremeSourceCache] -- Xanatos -->
			case DS_CACHED:
				SetLastSeen(); // NEO: MOD - [LastSeen]
				break;
			// NEO: XSC END <-- Xanatos --
            case DS_NONEEDEDPARTS:
                // Since tcp asks never sets reask time if the result is DS_NONEEDEDPARTS
                // If we set this, we will not reask for that file until some time has passed.
                SetLastAskedTime();
				// NEO: SR - [SpreadReask] -- Xanatos -->
				if(NeoPrefs.PartPrefs.UseSpreadReaskEnable())
					SetSpreadReaskModyfier();
				// NEO: SR END <-- Xanatos --
                //DontSwapTo(reqfile);
			/*default:
				switch( m_nDownloadState )
				{
					case DS_WAITCALLBACK:
#ifdef NATTUNNELING // NEO: XSB - [XSBuddy] -- Xanatos -->
					case DS_WAITCALLBACKXS:
#endif //NATTUNNELING // NEO: XSB END <-- Xanatos --
					case DS_WAITCALLBACKKAD:
						break;
					default:
						//m_dwLastTriedToConnect = ::GetTickCount()-20*60*1000;
						m_dwLastTriedToConnect = 0; // NEO: DRT - [DownloadReaskTweaks] <-- Xanatos --
						break;
				}
				break;*/
		}

		if (reqfile){
			// NEO: FIX - [SourceCount] -- Xanatos -->
			reqfile->DecrSrcStatisticsValue((EDownloadState)m_nDownloadState); 
			reqfile->IncrSrcStatisticsValue(nNewState); 

			reqfile->NotifyStatusChange();
			// NEO: FIX END <-- Xanatos --

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

			// NEO: DSB - [DynamicSocketBuffer] -- Xanatos -->
			int recvBuff = 0;
 #ifdef LANCAST // NEO: NLC - [NeoLanCast]
			if(IsLanClient() && NeoPrefs.IsSetLanDownloadBuffer())
				recvBuff = NeoPrefs.GetLanDownloadBufferSize();
			else
 #endif //LANCAST // NEO: NLC END
			if(NeoPrefs.IsSetDownloadBuffer())
				recvBuff = NeoPrefs.GetDownloadBufferSize();

			if(recvBuff)
				socket->SetRecvBufferSize(recvBuff);
			// NEO: DSB <-- Xanatos --
        }

		if (m_nDownloadState == DS_DOWNLOADING ){
			if(socket)
				socket->SetTimeOut(CONNECTION_TIMEOUT);

			if (thePrefs.GetLogUlDlEvents()) {
				switch( nNewState )
				{
					case DS_NONEEDEDPARTS:
						pszReason = _T("NNP. You don't need any parts from this client.");
				}

                if(thePrefs.GetLogUlDlEvents())
                    AddDebugLogLine(DLP_VERYLOW, false, _T("Download session ended: %s User: %s in SetDownloadState(). New State: %i, Length: %s, Payload: %s, Transferred: %s, Req blocks not yet completed: %i."), pszReason, DbgGetClientInfo(), nNewState, CastSecondsToHM(GetDownTimeDifference(false)/1000), CastItoXBytes(GetSessionPayloadDown(), false, false), CastItoXBytes(GetSessionDown(), false, false), m_PendingBlocks_list.GetCount());
			}

#ifdef ARGOS // NEO: NA -- Xanatos -->
			if(NeoPrefs.IsUploadFakerDetection()){
				//Xman filter clients with failed downloads
				if(GetSessionDown() < KB2B(12)){
					m_faileddownloads++;
					if(m_faileddownloads > 3){
						if (thePrefs.GetLogBannedClients())
							AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Uploader Faker (or bogus)"), GetUserName(), ipstr(GetConnectIP()));
						Ban(_T("Uploader Faker (or bogus)"));
					}
				}else
					m_faileddownloads=0; 
			}
#endif // ARGOS // NEO: NA END <-- Xanatos --

			ResetSessionDown();

			// -khaos--+++> Extended Statistics (Successful/Failed Download Sessions)
			if ( m_bTransferredDownMini && nNewState != DS_ERROR )
				thePrefs.Add2DownSuccessfulSessions(); // Increment our counters for successful sessions (Cumulative AND Session)
			else
				thePrefs.Add2DownFailedSessions(); // Increment our counters failed sessions (Cumulative AND Session)
			thePrefs.Add2DownSAvgTime(GetDownTimeDifference()/1000);
			// <-----khaos-

			m_nDownloadState = (_EDownloadState)nNewState;

			ClearDownloadBlockRequests();
				
			m_nDownDatarate = 0;
			m_AvarageDDR_list.RemoveAll();
			m_nSumForAvgDownDataRate = 0;

			// NEO: SCFS - [SmartClientFileStatus] -- Xanatos --
			/*if (nNewState == DS_NONE){
				delete[] m_abyPartStatus;
				m_abyPartStatus = NULL;
				m_nPartCount = 0;
			}*/
			if (socket && nNewState != DS_ERROR )
				socket->DisableDownloadLimit();
		}
		m_nDownloadState = (_EDownloadState)nNewState;
		if( GetDownloadState() == DS_DOWNLOADING ){
			// NEO: FIX - [SourceCount] -- Xanatos --
			//if ( IsEmuleClient() )
			//	SetRemoteQueueFull(false);
			SetRemoteQueueRank(0);
			SetAskedCountDown(0);
		}

		UpdateDisplayedInfo(true);
	}
}

void CUpDownClient::ProcessHashSet(const uchar* packet,uint32 size)
{
	if (!m_fHashsetRequesting)
		throw CString(_T("unwanted hashset"));
	if ( (!reqfile) || md4cmp(packet,reqfile->GetFileHash())){
		CheckFailedFileIdReqs(packet);
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessHashSet)");	
	}
	CSafeMemFile data(packet, size);
	if (reqfile->LoadHashsetFromFile(&data,true)){
		m_fHashsetRequesting = 0;
		reqfile->SetSinglePartHash((uint16)-1); // SLUGFILLER: SafeHash - Rehash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	}
	else{
		reqfile->hashsetneeded = true;
		throw GetResString(IDS_ERR_BADHASHSET);
	}
	// NEO: SD - [StandByDL] -- Xanatos -->
	if(m_byFileRequestState == FR_STANDBY)
		SetDownloadState(DS_HALTED);
	// NEO: SD END <-- Xanatos -->
	else if(m_byFileRequestState == FR_INPROGRES) // NEO: USPS - [UnSolicitedPartStatus] <-- Xanatos --
		SendStartupLoadReq();
}

void CUpDownClient::CreateBlockRequests(int iMaxBlocks)
{
	// NEO: DBR - [DynamicBlockRequest] -- Xanatos -->
	uint16 count;
    if(iMaxBlocks > m_PendingBlocks_list.GetCount() || iMaxBlocks == 0 && m_PendingBlocks_list.GetCount() == 0) {
        uint32 blocksize = EMBLOCKSIZE;
        if(iMaxBlocks > 0) {
            count = (uint16)(iMaxBlocks - m_PendingBlocks_list.GetCount());

			if(iMaxBlocks > 3 && IsEmuleClient() /*&& m_byCompatibleClient==0*/ && ((GetVersion() >> 10) & 0x7f) >= 40) // only when the cleint supports it
				blocksize = EMBLOCKSIZE*3;
        } else {
            // when iMaxBlocks == 0 it means just a little remains of the file, and it's important
            // not to lock too much of the file for really slow downloads. If we lock too much
            // those slow downloads can prevent us from using faster connections, since there will
            // be no blocks left of the file to request from those faster clients.

            // so find one block to request
            count = 1;

            // but a small one (how small depends on how fast the client has sent us data so far)
            if(GetSessionPayloadDown() >= 20*1024) {
                blocksize = min(EMBLOCKSIZE, 123*GetDownloadDatarate());

                blocksize = blocksize - (blocksize % 10*1024) + 10*1024;
            } else {
                blocksize = 20*1024;
            }
        }

		ASSERT(blocksize > 1);
        Requested_Block_Struct** toadd = new Requested_Block_Struct*[count];
		if (NeoPrefs.UseIncompletePartStatus() == TRUE	? reqfile->GetNextRequestedBlockICS(this,toadd,&count,blocksize) // NEO: ICS - [InteligentChunkSelection]
															: reqfile->GetNextRequestedBlock(this,toadd,&count,blocksize)
		    ){
            for (UINT i = 0; i < count; i++) {
		        Pending_Block_Struct* pblock = new Pending_Block_Struct;
		        pblock->block = toadd[i];
		        m_PendingBlocks_list.AddTail(pblock);
            }
		}
		delete[] toadd;
    }
	// NEO: DBR END <-- Xanatos --

	//ASSERT( iMaxBlocks >= 1 /*&& iMaxBlocks <= 3*/ );
	//if (m_DownloadBlocks_list.IsEmpty())
	//{
	//	uint16 count;
    //    if(iMaxBlocks > m_PendingBlocks_list.GetCount()) {
    //        count = (uint16)(iMaxBlocks - m_PendingBlocks_list.GetCount());
    //    } else {
    //        count = 0;
    //    }
	//
	//	Requested_Block_Struct** toadd = new Requested_Block_Struct*[count];
	//	if (reqfile->GetNextRequestedBlock(this, toadd, &count)){
	//		for (UINT i = 0; i < count; i++)
	//			m_DownloadBlocks_list.AddTail(toadd[i]);
	//	}
	//	delete[] toadd;
	//}
	//
	//while (m_PendingBlocks_list.GetCount() < iMaxBlocks && !m_DownloadBlocks_list.IsEmpty())
	//{
	//	Pending_Block_Struct* pblock = new Pending_Block_Struct;
	//	pblock->block = m_DownloadBlocks_list.RemoveHead();
	//	m_PendingBlocks_list.AddTail(pblock);
	//}
}

void CUpDownClient::SendBlockRequests()
{
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__RequestParts", this, reqfile!=NULL ? reqfile->GetFileHash() : NULL);

	m_dwLastBlockReceived = ::GetTickCount();
	if (!reqfile)
		return;

    // prevent locking of too many blocks when we are on a slow (probably standby/trickle) slot
    int blockCount = 3;
	// NEO: DBR - [DynamicBlockRequest] -- Xanatos -->
    if(IsEmuleClient() /*&& m_byCompatibleClient==0 && reqfile->GetFileSize()-reqfile->GetCompletedSize() <= (uint64)PARTSIZE*4*/) { // ZZUL tries to make clients request from same chunk, so it makes sense preventing also a slow client from preventing a chunk to be completed too long.
        // request fewer blocks for
        // slow downloads, so they don't lock blocks from faster clients.
        // Only trust eMule clients to be able to handle less blocks than three
		if((GetDownloadDatarate() < 1500 || GetSessionPayloadDown() < 20*1024) && ::GetTickCount() - m_dwDownStartTime > SEC2MS(3)) {
            // request one smaller block for slow downloads, so they lock even less from faster clients.

            // the special blockcount = 0 causes one, smaller, block to be requested
            blockCount = 0;
        }
		else if(GetDownloadDatarate() < 2000 || reqfile->GetFileSize()-reqfile->GetCompletedSize() <= PARTSIZE && reqfile->GetTransferringSrcCount() > 1)
            blockCount = 1;
        else if(GetDownloadDatarate() < 2500)
            blockCount = 2;
        else
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
		if(IsLanClient())
            blockCount = 53; // on lan request entier chunks
		else 
#endif //LANCAST // NEO: NLC END
		// If we have a fast source request more blocks at one to avoid latency problems
		if(GetDownloadDatarate() >= KB2B(8))
			blockCount = 4 + GetDownloadDatarate()/KB2B(8);
    }
	// NEO: DBR END <-- Xanatos --
    //if(IsEmuleClient() && m_byCompatibleClient==0 && reqfile->GetFileSize()-reqfile->GetCompletedSize() <= (uint64)PARTSIZE*4) {
    //    // if there's less than two chunks left, request fewer blocks for
    //    // slow downloads, so they don't lock blocks from faster clients.
    //    // Only trust eMule clients to be able to handle less blocks than three
    //    if(GetDownloadDatarate() < 600 || GetSessionPayloadDown() < 40*1024) {
    //        blockCount = 1;
    //    } else if(GetDownloadDatarate() < 1200) {
    //        blockCount = 2;
    //    }
    //}
	CreateBlockRequests(blockCount);

	if (m_PendingBlocks_list.IsEmpty()){
		SendCancelTransfer();
		SetDownloadState(DS_NONEEDEDPARTS);
        SwapToAnotherFile(_T("A4AF for NNP file. CUpDownClient::SendBlockRequests()"), true, false, false, NULL, true, true);
		return;
	}

	// NEO: DBR - [DynamicBlockRequest] -- Xanatos -->
	if(blockCount <= 3)
		SendBlockRequestsPacket(m_PendingBlocks_list.GetHeadPosition()); 
	else
	{
		POSITION pos = m_PendingBlocks_list.GetHeadPosition();
		while(pos != NULL)
		{
			POSITION startPos = pos;
			Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
			if(pending->fQueued)
				continue;
			SendBlockRequestsPacket(startPos);
		}
	}
	// NEO: DBR END <-- Xanatos --
}

POSITION CUpDownClient::SendBlockRequestsPacket(POSITION startPos) // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
{
	bool bI64Offsets = false;
	//POSITION pos = m_PendingBlocks_list.GetHeadPosition();
	POSITION pos = startPos; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
	for (uint32 i = 0; i != 3; i++){
		if (pos){
			Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
			ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
			if (pending->block->StartOffset > 0xFFFFFFFF || pending->block->EndOffset >= 0xFFFFFFFF){
				bI64Offsets = true;
				if (!SupportsLargeFiles()){
					ASSERT( false );
					SendCancelTransfer();
					SetDownloadState(DS_ERROR);
					return NULL; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
				}
				//break; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
			}
			pending->fQueued = 1; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
		}
	}
	POSITION endPos = pos; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --

	Packet* packet;
	if (bI64Offsets){
		const int iPacketSize = 16+(3*8)+(3*8); // 64
		packet = new Packet(OP_REQUESTPARTS_I64, iPacketSize, OP_EMULEPROT);
		CSafeMemFile data((const BYTE*)packet->pBuffer, iPacketSize);
		data.WriteHash16(reqfile->GetFileHash());
		//pos = m_PendingBlocks_list.GetHeadPosition();
		pos = startPos; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
		for (uint32 i = 0; i != 3; i++){
			if (pos){
				Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
				ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
				//ASSERT( pending->zStream == NULL );
				//ASSERT( pending->totalUnzipped == 0 );
				pending->fZStreamError = 0;
				pending->fRecovered = 0;
				data.WriteUInt64(pending->block->StartOffset);
			}
			else
				data.WriteUInt64(0);
		}
		//pos = m_PendingBlocks_list.GetHeadPosition();
		pos = startPos; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
		for (uint32 i = 0; i != 3; i++){
			if (pos){
				Requested_Block_Struct* block = m_PendingBlocks_list.GetNext(pos)->block;
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
			}
			else
			{
				data.WriteUInt64(0);
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					Debug(_T("  Block request %u: <empty>\n"), i);
			}
		}
	}
	else{
		const int iPacketSize = 16+(3*4)+(3*4); // 40
		packet = new Packet(OP_REQUESTPARTS,iPacketSize);
		CSafeMemFile data((const BYTE*)packet->pBuffer, iPacketSize);
		data.WriteHash16(reqfile->GetFileHash());
		//pos = m_PendingBlocks_list.GetHeadPosition();
		pos = startPos; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
		for (uint32 i = 0; i != 3; i++){
			if (pos){
				Pending_Block_Struct* pending = m_PendingBlocks_list.GetNext(pos);
				ASSERT( pending->block->StartOffset <= pending->block->EndOffset );
				//ASSERT( pending->zStream == NULL );
				//ASSERT( pending->totalUnzipped == 0 );
				pending->fZStreamError = 0;
				pending->fRecovered = 0;
				data.WriteUInt32((uint32)pending->block->StartOffset);
			}
			else
				data.WriteUInt32(0);
		}
		//pos = m_PendingBlocks_list.GetHeadPosition();
		pos = startPos; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
		for (uint32 i = 0; i != 3; i++){
			if (pos){
				Requested_Block_Struct* block = m_PendingBlocks_list.GetNext(pos)->block;
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
			}
			else
			{
				data.WriteUInt32(0);
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					Debug(_T("  Block request %u: <empty>\n"), i);
			}
		}
	}

	theStats.AddUpDataOverheadFileRequest(packet->size);
	socket->SendPacket(packet,true,true);

	return endPos; // NEO: DBR - [DynamicBlockRequest] <-- Xanatos --
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
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	if(!IsLanClient())
#endif //LANCAST // NEO: NLC END <-- Xanatos --
		thePrefs.Add2SessionTransferData(GetClientSoft(), GetUserPort(), false, false, uTransferredFileDataSize, false);
	// <-----khaos-

	//m_nDownDataRateMS += uTransferredFileDataSize;
	AddDownloadSize(uTransferredFileDataSize); // NEO: ASM - [AccurateSpeedMeasure] <-- Xanatos --

	if (credits)
		credits->AddDownloaded(uTransferredFileDataSize, GetIP(), GetRequestFile() != NULL); // NEO: NCS - [NeoCreditSystem] <-- Xanatos --
		//credits->AddDownloaded(uTransferredFileDataSize, GetIP());

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
				reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
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
				// Write to disk (will be buffered in part file class)
				lenWritten = reqfile->WriteToBuffer(uTransferredFileDataSize, 
													packet + nHeaderSize,
													nStartPos,
													nEndPos,
													cur_block->block,
													GetIP()); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
													//this);
			}
			else // Packed
			{
				ASSERT( (int)size > 0 );
				// Create space to store unzipped data, the size is only an initial guess, will be resized in unzip() if not big enough
				uint32 lenUnzipped = (size * 2); 
				// Don't get too big
				if (lenUnzipped > (EMBLOCKSIZE + 300))
					lenUnzipped = (EMBLOCKSIZE + 300);
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
							reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
							// There is no chance to recover from this error
						}
						else{
							// Write uncompressed data to file
							lenWritten = reqfile->WriteToBuffer(uTransferredFileDataSize,
								unzipped,
								nStartPos,
								nEndPos,
								cur_block->block,
								GetIP()); // NEO: VOODOO - [UniversalPartfileInterface] <-- Xanatos --
								//this);
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
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);

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
				if (nEndPos == cur_block->block->EndOffset)
				{
					reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
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

			// Stop looping and exit method
			return;
		}
	}

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

// NEO: ASM - [AccurateSpeedMeasure] -- Xanatos -->
uint32 CUpDownClient::CalculateDownloadRate(){
	// Add new sample
	TransferredData newSample;
	newSample.datalen = m_nSumForAvgDownDataRate;
	newSample.timestamp  = ::GetTickCount();
	m_AvarageDDR_list.AddHead(newSample);

	// Keep up to 21 samples (=> 20 seconds)
	while(m_AvarageDDR_list.GetSize() > 21){
		m_AvarageDDR_list.RemoveTail();
	}

	if(m_AvarageDDR_list.GetSize() > 1){	
		// Compute datarate (=> display)
		POSITION pos = m_AvarageDDR_list.FindIndex(NeoPrefs.GetDatarateSamples()); 
		if(pos == NULL)
			pos = m_AvarageDDR_list.GetTailPosition();
		TransferredData& oldestSample = m_AvarageDDR_list.GetAt(pos);
		uint32 deltaTime = newSample.timestamp - oldestSample.timestamp;
		uint32 deltaByte = newSample.datalen - oldestSample.datalen;
		if(deltaTime > 0)
			m_nDownDatarate = (UINT)((float)1000 * deltaByte / deltaTime);   // [bytes/s]
	}else
		m_nDownDatarate = 0;

    // Check if it's time to update the display.
	UpdateDisplayedInfo(true);

	return m_nDownDatarate;
}

UINT CUpDownClient::CheckDownloadRate(int dataratestocheck)
{
	if(m_AvarageDDR_list.GetSize() > 4 && dataratestocheck > 2){	
		POSITION pos = m_AvarageDDR_list.FindIndex(dataratestocheck);
		if(pos == NULL)
			pos = m_AvarageDDR_list.GetTailPosition();
		TransferredData& oldestSample = m_AvarageDDR_list.GetAt(pos);
		//TransferredData& newSample = m_AvarageDDR_list.GetHead();
		//uint32 deltaTime = newSample.timestamp - oldestSample.timestamp;
		//uint32 deltaByte = newSample.datalen - oldestSample.datalen;
		uint32 deltaTime = ::GetTickCount() - oldestSample.timestamp;
		uint32 deltaByte = m_nSumForAvgDownDataRate - oldestSample.datalen;
		if(deltaTime > 0)
			return (UINT)((float)1000 * deltaByte / deltaTime);   // [bytes/s]
	}
	return 0;
}
// NEO: ASM END <-- Xanatos --

/*uint32 CUpDownClient::CalculateDownloadRate(){
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
	m_cShowDR++;
	if (m_cShowDR == 30){
		m_cShowDR = 0;
		UpdateDisplayedInfo();
	}

	return m_nDownDatarate;
}*/

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
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	CClientFileStatus* status = GetFileStatus(reqfile);
	if(status == NULL || status->GetPartStatus() == NULL)
		return 0;

	for (UINT i = 0; i < status->GetPartCount(); i++){
		if (status->GetPartStatus()[i])
			result++;
	}
	// NEO: SCFS END <-- Xanatos --
	/*for (UINT i = 0; i < m_nPartCount; i++){
		if (IsPartAvailable(i))
			result++;
	}*/
	return (uint16)result;
}

void CUpDownClient::SetRemoteQueueRank(UINT nr, bool bUpdateDisplay)
{
	m_nRemoteQueueRankOld = m_nRemoteQueueRank; // NEO: CQR - [CollorQueueRank] <-- Xanatos --
	m_nRemoteQueueRank = nr;
	UpdateDisplayedInfo(bUpdateDisplay);
}

void CUpDownClient::UDPReaskACK(uint16 nNewQR)
{
	m_bUDPPending = false;
	SetRemoteQueueRank(nNewQR, true);
    SetLastAskedTime();
}

void CUpDownClient::UDPReaskFNF()
{
	m_bUDPPending = false;
	if (GetDownloadState() != DS_DOWNLOADING){ // avoid premature deletion of 'this' client
		if (thePrefs.GetVerbose())
			AddDebugLogLine(DLP_LOW, false, _T("UDP FNF-Answer: %s - %s"),DbgGetClientInfo(), DbgGetFileInfo(reqfile ? reqfile->GetFileHash() : NULL));
		if (reqfile)
			reqfile->m_DeadSourceList.AddDeadSource(this);
		ClearFileStatus(reqfile); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
		if(NeoPrefs.SaveSourceFileList() && source && reqfile)
			source->RemoveSeenFile(reqfile->GetFileHash());
#endif // NEO_CD // NEO: SFL END <-- Xanatos --
		switch (GetDownloadState()) {
			case DS_ONQUEUE:
			case DS_REMOTEQUEUEFULL: // NEO: FIX - [SourceCount] <-- Xanatos --
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
		//!theApp.IsFirewalled() && !(socket && socket->IsConnected()) && !thePrefs.GetProxySettings().UseProxy)
		(!theApp.IsFirewalled() || IsLowIDUDPPingSupport()) && !(socket && socket->IsConnected()) && !thePrefs.GetProxySettings().UseProxy) // NEO: NMP - [NeoModProt] <-- Xanatos --
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
				//if (reqfile->IsPartFile())
				//	((CPartFile*)reqfile)->WritePartStatus(&data);
				//else
				//	data.WriteUInt16(0);
				reqfile->WritePartStatus(&data, this); // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
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
				//if (reqfile->IsPartFile())
				//	((CPartFile*)reqfile)->WritePartStatus(&data);
				//else
				//	data.WriteUInt16(0);
				reqfile->WritePartStatus(&data, this); // NEO: IPS - [InteligentPartSharing] <-- Xanatos --
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

void CUpDownClient::UpdateDisplayedInfo(bool force)
{
#ifdef _DEBUG
	force = true;
#endif
    DWORD curTick = ::GetTickCount();
    if(force || curTick-m_lastRefreshedDLDisplay > MINWAIT_BEFORE_DLDISPLAY_WINDOWUPDATE+m_random_update_wait) {
	    theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
		theApp.emuledlg->transferwnd->downloadclientsctrl.RefreshClient(this);
        m_lastRefreshedDLDisplay = curTick;
    }
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
    //    curFileisNNPFile && cur_file->GetSourceCount() < cur_file->GetMaxSources()*.8) {
		curFileisNNPFile && cur_file->GetSourceCount() < cur_file->GetSwapSourceLimit()) { // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --

            if(printDebug)
                AddDebugLogLine(DLP_VERYLOW, false, _T("oooo Debug: cur_file does probably not have too many sources."));

            if(SwapTo->GetSourceCount() > SwapTo->GetMaxSources() ||
            //   SwapTo->GetSourceCount() >= SwapTo->GetMaxSources()*.8 &&
               SwapTo->GetSourceCount() >= cur_file->GetSwapSourceLimit() && // NEO: SRT - [SourceRequestTweaks] <-- Xanatos --
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

//bool CUpDownClient::DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason)
bool CUpDownClient::DoSwap(CPartFile* SwapTo, bool bRemoveCompletely, LPCTSTR reason, bool bHandleClientReq) // NEO: MCM - [ManualClientManagement] <-- Xanatos --
{
    if (thePrefs.GetLogA4AF())
        AddDebugLogLine(DLP_LOW, false, _T("ooo Swapped source %s Remove = %s '%s'   -->   %s Reason: %s"), DbgGetClientInfo(), (bRemoveCompletely ? _T("Yes") : _T("No") ), (this->reqfile)?this->reqfile->GetFileName():_T("null"), SwapTo->GetFileName(), reason);

	// NEO: MCM - [ManualClientManagement] -- Xanatos -->
	// Note: the part below is very very important
	if(bHandleClientReq){
		POSITION pos3 = m_OtherRequests_list.Find(SwapTo);
		if(pos3){
			m_OtherRequests_list.RemoveAt(pos3);
		}else{		
			POSITION pos4 = m_OtherNoNeeded_list.Find(SwapTo);
			if(pos4){
				m_OtherNoNeeded_list.RemoveAt(pos4);
			}
		}
	}
	// NEO: MCM END <-- Xanatos --

	// 17-Dez-2003 [bc]: This "reqfile->srclists[sourcesslot].Find(this)" was the only place where 
	// the usage of the "CPartFile::srclists[100]" is more effective than using one list. If this
	// function here is still (again) a performance problem there is a more effective way to handle
	// the 'Find' situation. Hint: usage of a node ptr which is stored in the CUpDownClient.
	POSITION pos = reqfile->srclist.Find(this);
	if(pos)
    {
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
	theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(this,SwapTo);

	reqfile->RemoveDownloadingSource(this);

	if(!bRemoveCompletely)
	{
        reqfile->A4AFsrclist.AddTail(this);
		if (GetDownloadState() == DS_NONEEDEDPARTS)
			m_OtherNoNeeded_list.AddTail(reqfile);
		else
			m_OtherRequests_list.AddTail(reqfile);

		theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(reqfile,this,true);
    } else {
        m_fileReaskTimes.RemoveKey(reqfile);
		//ClearFileStatus(reqfile); // NEO: SCFS - [SmartClientFileStatus] <-- Xanatos --
    }

	SetDownloadState(DS_NONE);
	CPartFile* pOldRequestFile = reqfile;
	SetRequestFile(SwapTo);	
	pOldRequestFile->UpdatePartsInfo();
	// NEO: ICS - [InteligentChunkSelection] -- Xanatos -->
	if(GetIncompletePartVersion() && NeoPrefs.UseIncompletePartStatus())
		pOldRequestFile->UpdatePartsInfoEx(CFS_Incomplete);
	// NEO: ICS END <-- Xanatos --
	pOldRequestFile->UpdateAvailablePartsCount();

	SwapTo->srclist.AddTail(this);
	theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(SwapTo,this,false);

	// NEO: MFSB - [MultiFileStatusBars] -- Xanatos -->
	SwapTo->UpdatePartsInfo();
	// NEO: ICS - [InteligentChunkSelection]
	if(GetIncompletePartVersion() && NeoPrefs.UseIncompletePartStatus())
		SwapTo->UpdatePartsInfoEx(CFS_Incomplete);
	// NEO: ICS END
	SwapTo->UpdateAvailablePartsCount();
	// NEO: MFSB END <-- Xanatos --

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

	// NEO: MCM - [ManualClientManagement] -- Xanatos -->
	if(IsSwapingDisabled()){
		if(!reqfile || reqfile->IsStopped() || !reqfile->IsPartFile()) // if the owner does not need this source clear the flag
			DisableSwaping(false);
		else
			return true;
	}
	// NEO: MCM END <-- Xanatos --

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

		// NEO: SD - [StandByDL] -- Xanatos -->
		if(file->IsStandBy())
			return 0x7FFFFFFF; // file is in styndby and we hav asked once thats enough
		else if(m_byFileRequestState == FR_STANDBY)
			return 0; // File was in styndby and we havn't finished the reask, finish it now
		// NEO: SD END <-- Xanatos -->

        DWORD tick = ::GetTickCount();

        DWORD reaskTime;
        if(allowShortReaskTime || file == reqfile && GetDownloadState() == DS_NONE) {
            reaskTime = MIN_REQUESTTIME;
        } else if(useGivenNNP && givenNNP ||
					file == reqfile && GetDownloadState() == DS_NONEEDEDPARTS ||
					file != reqfile && IsInNoNeededList(file)) 
		{
            //reaskTime = FILEREASKTIME*2;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
			if(IsLanClient())
				reaskTime = reqfile->PartPrefs->GetLanNNPSourceReaskTimeMs();
			else
#endif //LANCAST // NEO: NLC END <-- Xanatos --
				reaskTime = reqfile->PartPrefs->GetNNPSourceReaskTimeMs(); // NEO: DR - [DownloadReask] <-- Xanatos --
        } else {
            //reaskTime = FILEREASKTIME;
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
			if(IsLanClient())
				reaskTime = reqfile->PartPrefs->GetLanSourceReaskTimeMs();
			else
#endif //LANCAST // NEO: NLC END <-- Xanatos --
				reaskTime = GetDownloadState() == DS_REMOTEQUEUEFULL ? reqfile->PartPrefs->GetFullQSourceReaskTimeMs() : reqfile->PartPrefs->GetSourceReaskTimeMs(); // NEO: DR - [DownloadReask] // NEO: FIX - [SourceCount] <-- Xanatos --
        }

		// NEO: SR - [SpreadReask] -- Xanatos -->
		// failsafe
		if(NeoPrefs.PartPrefs.UseSpreadReaskEnable()){
			reaskTime += m_uSpreadReaskModyfier;
			if(reaskTime + MIN2MS(4) > MAX_PURGEQUEUETIME) 
				reaskTime = MAX_PURGEQUEUETIME - MIN2MS(4);
			else if(m_bReaskPending && (reaskTime*2)-(tick-lastAskedTimeTick) + MIN2MS(4) > MAX_PURGEQUEUETIME)
				return 0;
		}
		// NEO: SR END <-- Xanatos --

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
	// NEO: MOD - [IsSurceSuspended] -- Xanatos -->
	if(IsSurceSuspended()) // Don't send unavalibly/unproved sources
		return false;
	// NEO: MOD END <-- Xanatos --
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
	// NEO: FIX - [SourceCount] -- Xanatos -->
	if(reqfile)
		reqfile->DecrSrcStatisticsValue((EDownloadState)m_nDownloadState); 
	if(pReqFile)
		pReqFile->IncrSrcStatisticsValue((EDownloadState)m_nDownloadState); 
	// NEO: FIX END <-- Xanatos --
	reqfile = pReqFile;
	// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
	if(pReqFile)
		GetFileStatus(pReqFile, true);
	// NEO: SCFS END <-- Xanatos --
	m_dwLastUsableDownloadState = ::GetTickCount(); // NEO: SDT - [SourcesDropTweaks] <-- Xanatos --

#ifdef NEO_CD // NEO: SFL - [SourceFileList] -- Xanatos -->
	if(NeoPrefs.SaveSourceFileList() && source && reqfile)
		source->AddSeenFile(reqfile->GetFileHash(),reqfile->GetFileSize());
#endif // NEO_CD // NEO: SFL END <-- Xanatos --
}

void CUpDownClient::ProcessAcceptUpload()
{
	m_fQueueRankPending = 1;
	//if (reqfile && !reqfile->IsStopped() && (reqfile->GetStatus()==PS_READY || reqfile->GetStatus()==PS_EMPTY))
	if (reqfile 
	&& !reqfile->IsStopped() 
	&& !reqfile->IsStandBy() // NEO: SD - [StandByDL] <-- Xanatos --
	&& (reqfile->GetStatus()==PS_READY || reqfile->GetStatus()==PS_EMPTY))
	{
		SetSentCancelTransfer(0);
		//if (GetDownloadState() == DS_ONQUEUE)
		if (GetDownloadState() == DS_ONQUEUE || GetDownloadState() == DS_REMOTEQUEUEFULL) // NEO: FIX - [SourceCount] <-- Xanatos --
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
		}
	}
	else
	{
		SendCancelTransfer();
		SetDownloadState((reqfile==NULL || reqfile->IsStopped()) ? DS_NONE : DS_ONQUEUE);
	}
}

void CUpDownClient::ProcessEdonkeyQueueRank(const uchar* packet, UINT size)
{
	CSafeMemFile data(packet, size);
	uint32 rank = data.ReadUInt32();
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		//Debug(_T("  QR=%u (prev. %d)\n"), rank, IsRemoteQueueFull() ? (UINT)-1 : (UINT)GetRemoteQueueRank());
		Debug(_T("  QR=%u (prev. %d)\n"), rank, GetDownloadState() == DS_REMOTEQUEUEFULL ? (UINT)-1 : (UINT)GetRemoteQueueRank()); // NEO: FIX - [SourceCount] <-- Xanatos --
	//SetRemoteQueueRank(rank, GetDownloadState() == DS_ONQUEUE);
	SetRemoteQueueRank(rank, GetDownloadState() == DS_ONQUEUE || GetDownloadState() == DS_REMOTEQUEUEFULL); // NEO: FIX - [SourceCount] <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	if(NeoPrefs.IsRankFloodDetection())
#endif // ARGOS // NEO: NA END <-- Xanatos --
		CheckQueueRankFlood();
}

void CUpDownClient::ProcessEmuleQueueRank(const uchar* packet, UINT size)
{
	if (size != 12)
		throw GetResString(IDS_ERR_BADSIZE);
	uint16 rank = PeekUInt16(packet);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  QR=%u\n"), rank); // no prev. QR available for eMule clients
	//SetRemoteQueueFull(false);
	// NEO: FIX - [SourceCount] -- Xanatos -->
	if(GetDownloadState() == DS_REMOTEQUEUEFULL)
		SetDownloadState(DS_ONQUEUE);
	// NEO: FIX END <-- Xanatos --
	SetRemoteQueueRank(rank, GetDownloadState() == DS_ONQUEUE);
	// NEO: SDT - [SourcesDropTweaks] -- Xanatos -->
	if(GetDownloadState() == DS_ONQUEUE && reqfile && (!reqfile->PartPrefs->UseHighQSourceDrop() || !reqfile->IsHighQState(this)))
		m_dwLastUsableDownloadState = ::GetTickCount();
	// NEO: SDT END <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	if(NeoPrefs.IsRankFloodDetection())
#endif // ARGOS // NEO: NA END <-- Xanatos --
		CheckQueueRankFlood();
}

void CUpDownClient::CheckQueueRankFlood()
{
	if (m_fQueueRankPending == 0)
	{
		if (GetDownloadState() != DS_DOWNLOADING)
		{
			if (m_fUnaskQueueRankRecv < 3) // NOTE: Do not increase this nr. without increasing the bits for 'm_fUnaskQueueRankRecv'
				m_fUnaskQueueRankRecv++;
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
{
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
	CAICHHashSet::m_liRequestedData.AddTail(request);
	m_fAICHRequested = TRUE;
	CSafeMemFile data;
	data.WriteHash16(pForFile->GetFileHash());
	data.WriteUInt16(nPart);
	CSingleLock sLockA(pForFile->GetSCV_mut(), TRUE); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	pForFile->GetAICHHashset()->GetMasterHash().Write(&data);
	sLockA.Unlock(); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	Packet* packet = new Packet(&data, OP_EMULEPROT, OP_AICHREQUEST);
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__AichRequest", this, (uchar*)packet->pBuffer);
	theStats.AddUpDataOverheadFileRequest(packet->size);
	SafeSendPacket(packet);
}

void CUpDownClient::ProcessAICHAnswer(const uchar* packet, UINT size)
{
	if (m_fAICHRequested == FALSE){
		throw CString(_T("Received unrequested AICH Packet"));
	}
	m_fAICHRequested = FALSE;

	CSafeMemFile data(packet, size);
	if (size <= 16){	
		CAICHHashSet::ClientAICHRequestFailed(this);
		return;
	}
	uchar abyHash[16];
	data.ReadHash16(abyHash);
	CPartFile* pPartFile = theApp.downloadqueue->GetFileByID(abyHash);
	CAICHRequestedData request = CAICHHashSet::GetAICHReqDetails(this);
	uint16 nPart = data.ReadUInt16();
	if (pPartFile != NULL && request.m_pPartFile == pPartFile && request.m_pClient == this && nPart == request.m_nPart){
		CAICHHash ahMasterHash(&data);
		CSingleLock sLockA(pPartFile->GetSCV_mut(), TRUE); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
		if ( (pPartFile->GetAICHHashset()->GetStatus() == AICH_TRUSTED || pPartFile->GetAICHHashset()->GetStatus() == AICH_VERIFIED)
			 && ahMasterHash == pPartFile->GetAICHHashset()->GetMasterHash())
		{
			if(pPartFile->GetAICHHashset()->ReadRecoveryData((uint64)request.m_nPart*PARTSIZE, &data)){
				// finally all checks passed, everythings seem to be fine
				AddDebugLogLine(DLP_DEFAULT, false, _T("AICH Packet Answer: Succeeded to read and validate received recoverydata"));
				CAICHHashSet::RemoveClientAICHRequest(this);
				sLockA.Unlock(); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
				pPartFile->AICHRecoveryDataAvailable(request.m_nPart);
				return;
			}
			else
				AddDebugLogLine(DLP_DEFAULT, false, _T("AICH Packet Answer: Succeeded to read and validate received recoverydata"));
		}
		else
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Answer: Masterhash differs from packethash or hashset has no trusted Masterhash"));
		sLockA.Unlock(); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	}
	else
		AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Answer: requested values differ from values in packet"));

	CAICHHashSet::ClientAICHRequestFailed(this);
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
		if (pKnownFile->GetAICHHashset()->GetStatus() == AICH_HASHSETCOMPLETE && pKnownFile->GetAICHHashset()->HasValidMasterHash()
			&& pKnownFile->GetAICHHashset()->GetMasterHash() == ahMasterHash && pKnownFile->GetPartCount() > nPart
			&& pKnownFile->GetFileSize() > (uint64)EMBLOCKSIZE && (uint64)pKnownFile->GetFileSize() - PARTSIZE*(uint64)nPart > EMBLOCKSIZE)
		{
			CSafeMemFile fileResponse;
			fileResponse.WriteHash16(pKnownFile->GetFileHash());
			fileResponse.WriteUInt16(nPart);
			pKnownFile->GetAICHHashset()->GetMasterHash().Write(&fileResponse);
			if (pKnownFile->GetAICHHashset()->CreatePartRecoveryData((uint64)nPart*PARTSIZE, &fileResponse)){
				AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Successfully created and send recoverydata for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__AichAnswer", this, pKnownFile->GetFileHash());
				Packet* packAnswer = new Packet(&fileResponse, OP_EMULEPROT, OP_AICHANSWER);
				theStats.AddUpDataOverheadFileRequest(packAnswer->size);
				SafeSendPacket(packAnswer);
				return;
			}
			else
				AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to create recoverydata for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
		}
		else{
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to create recoverydata - Hashset not ready or requested Hash differs from Masterhash for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
		}

	}
	else
		AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to find requested shared file -  %s"), DbgGetClientInfo());
	
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__AichAnswer", this, abyHash);
	Packet* packAnswer = new Packet(OP_AICHANSWER, 16, OP_EMULEPROT);
	md4cpy(packAnswer->pBuffer, abyHash);
	theStats.AddUpDataOverheadFileRequest(packAnswer->size);
	SafeSendPacket(packAnswer);
}

void CUpDownClient::ProcessAICHFileHash(CSafeMemFile* data, CPartFile* file)
{
	CPartFile* pPartFile = file;
	if (pPartFile == NULL){
		uchar abyHash[16];
		data->ReadHash16(abyHash);
		pPartFile = theApp.downloadqueue->GetFileByID(abyHash);
	}
	CAICHHash ahMasterHash(data);
	if(pPartFile != NULL && pPartFile == GetRequestFile()){
		SetReqFileAICHHash(new CAICHHash(ahMasterHash));
		CSingleLock sLockA(pPartFile->GetSCV_mut(), TRUE); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
		pPartFile->GetAICHHashset()->UntrustedHashReceived(ahMasterHash, GetConnectIP());
		sLockA.Unlock(); // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	}
	else
		AddDebugLogLine(DLP_HIGH, false, _T("ProcessAICHFileHash(): PartFile not found or Partfile differs from requested file, %s"), DbgGetClientInfo());
}

// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
const CString CUpDownClient::GetClientFilename() const{ 
	CClientFileStatus* status = GetFileStatus(reqfile);
	if(status)
		return status->GetFileName();
	return _T("");
}
// NEO: SCFS END <-- Xanatos --

// NEO: SR - [SpreadReask] -- Xanatos -->
void CUpDownClient::SetSpreadReaskModyfier()
{ 
	m_uSpreadReaskModyfier = ((uint32)(((float)(rand()%100))/100.0f*NeoPrefs.PartPrefs.GetSpreadReaskTimeMs()));
}
// NEO: SR END <-- Xanatos --

// NEO: DR - [DownloadReask] -- Xanatos -->
uint32 CUpDownClient::GetReqFileReaskIntervals()
{
	uint32 FileReaskTime;

	if(reqfile && reqfile->IsPartFile())
	{
		switch(GetDownloadState())
		{
			case DS_REMOTEQUEUEFULL: // NEO: FIX - [SourceCount]
				FileReaskTime = reqfile->PartPrefs->GetFullQSourceReaskTimeMs();
				break;
			case DS_NONEEDEDPARTS:
				FileReaskTime = reqfile->PartPrefs->GetNNPSourceReaskTimeMs();
				break;
			default:
			FileReaskTime = reqfile->PartPrefs->GetSourceReaskTimeMs();
		}
	}
	else
		FileReaskTime = NeoPrefs.PartPrefs.GetSourceReaskTimeMs();

	return FileReaskTime;
}

void CUpDownClient::SetNextAskedTime(uint32 uNextReask)
{ 
	m_fileReaskTimes.SetAt(reqfile, uNextReask ? ((::GetTickCount() - GetReqFileReaskIntervals()) + uNextReask) : 0); 
	// We reset this time so the clinet don't stuck for 20mins.
	m_dwLastTriedToConnect = 0;
}

bool CUpDownClient::SetSafeReAskTime()
{
	if(!reqfile)
		return false;

	if (GetLastAskedTime() == 0)
		return true;

	if(::GetTickCount() - GetLastAskedTime(reqfile) > MIN_REQUESTTIME){
		SetNextAskedTime(); // to now
		return true;
	}
	SetNextAskedTime(MIN_REQUESTTIME);
	return false;
}
// NEO: DR END <-- Xanatos --

#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
CEMSocket* CUpDownClient::GetFileDownloadSocket(bool bLog) 
{
    if(m_pPCDownSocket && (IsDownloadingFromPeerCache() || m_ePeerCacheDownState == PCDS_WAIT_CACHE_REPLY)) {
        if(thePrefs.GetVerbose() && bLog)
            AddDebugLogLine(false, _T("%s got peercache socket."), DbgGetClientInfo());
        return m_pPCDownSocket;
	}
	else {
        if(thePrefs.GetVerbose() && bLog)
            AddDebugLogLine(false, _T("%s got normal socket."), DbgGetClientInfo());
        return socket;
    }
}
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
