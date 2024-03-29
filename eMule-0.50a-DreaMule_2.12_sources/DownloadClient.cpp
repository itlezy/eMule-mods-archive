//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "UploadQueue.h" //Xman UDPReaskFNF-Fix against Leechers (idea by WiZaRd)		

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// Xman Maella -Code Improvement-
void CUpDownClient::DrawStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat) const
{ 

	if (g_bLowColorDesktop)
		bFlat = true;

	const COLORREF crBoth = (bFlat) ? RGB(0, 0, 0) : RGB(104, 104, 104); 
	const COLORREF crNeither = (bFlat) ? (g_bLowColorDesktop) ? RGB(192, 192, 192) : RGB(224, 224, 224) : RGB(240, 240, 240); // Flat => Grey
	const COLORREF crClientOnly = (g_bLowColorDesktop) ? RGB(0, 0, 255) : RGB(0, 170, 245);
	const COLORREF crPending = RGB(255, 160, 0); // Flat => yellow-orange
	const COLORREF crNextPending = RGB(255,255,100); // Flat => yellow

	// Set size and fill with default color (grey)
	CBarShader statusBar(rect->bottom - rect->top, rect->right - rect->left);
	statusBar.SetFileSize((reqfile != NULL) ? (reqfile->GetFileSize()-(uint64)1) : (uint64)1); 
	statusBar.Fill(crNeither); 

	if(onlygreyrect == false && reqfile != NULL && m_abyPartStatus > 0) { 
		// Create a linear array with all pending blocks
		// Remark: The class std::vector has a much more powerfull constructor than the MFC CArray
		std::vector<bool> gettingParts(m_nPartCount, false);
		for(POSITION pos = m_PendingBlocks_list.GetHeadPosition(); pos != NULL; ){
			UINT uPart = (UINT)(m_PendingBlocks_list.GetNext(pos)->block->StartOffset / PARTSIZE);
			if (uPart < m_nPartCount)
				gettingParts[uPart] = true;
		}

		// Fill each block with a color matching the status
		for(UINT i = 0; i < m_nPartCount; i++){ 
			if(m_abyPartStatus[i] != 0){ 
				const uint64 uStart = PARTSIZE*(uint64)i;
				const uint64 uEnd = (reqfile->GetFileSize()-(uint64)1 <= (uStart+PARTSIZE-1)) ? (reqfile->GetFileSize()-(uint64)1) : (uStart+PARTSIZE-1);

				if(reqfile->IsComplete(uStart, uEnd, false) == true) 
					statusBar.FillRange(uStart, uEnd, crBoth);
				else if(GetSessionDown() > 0 && m_nDownloadState == DS_DOWNLOADING && m_nLastBlockOffset < uEnd &&
					m_nLastBlockOffset >= uStart)
					statusBar.FillRange(uStart, uEnd, crPending); 
				else if(gettingParts[i] == true)
					statusBar.FillRange(uStart, uEnd, crNextPending); 
				else
					statusBar.FillRange(uStart, uEnd, crClientOnly); 
			} 
		} 
	} 

	// Finally draw the graphical object
	statusBar.Draw(dc, rect->left, rect->top, bFlat); 
} 
// Xman end


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
	//Xman askfordownload priority
	if(!(socket && socket->IsConnected())) 
	{
		if(m_downloadpriority>theApp.downloadqueue->GetMaxDownPrioNew())
			theApp.downloadqueue->SetMaxDownPrioNew(m_downloadpriority);

		if(theApp.downloadqueue->GetTooManyConnections() > (thePrefs.GetMaxConperFive()+20))
		{
			m_downloadpriority=1;
			if(GetRemoteQueueRank()!=0)
			{
				if(GetRemoteQueueRank()<120)
					m_downloadpriority++;
				if(GetRemoteQueueRank()<=30){
					m_downloadpriority++;
					m_downloadpriority++;
				}
			}
			if(GetUploadState()==US_ONUPLOADQUEUE)
			{
				m_downloadpriority++;
				if(::GetTickCount() - GetLastUpRequest() > (100 * 60 * 192)) // 19.2 min time we are in tcp-reaskwindow, and lower the time old clients use for reask
					m_downloadpriority++;
			}
			if(GetLastAskedTime()!=0) //never asked before
			{
				if(::GetTickCount() - GetLastAskedTime() > (1000 * 60 * 40)) //40 min
					m_downloadpriority++;
				if(::GetTickCount() - GetLastAskedTime() > (1000 * 60 * 50)){ //50 min
					m_downloadpriority++;
					m_downloadpriority++;
				}
			}
			m_downloadpriority= (sint8)(m_downloadpriority - m_cFailed);
			if(m_downloadpriority<0) m_downloadpriority=0;

			if(m_downloadpriority<theApp.downloadqueue->GetMaxDownPrio())
			{
				//we don't allow, first take the clients with higher prio
				if (GetDownloadState() != DS_TOOMANYCONNS)
					SetDownloadState(DS_TOOMANYCONNS);
				return true;
			}
		}


		if (theApp.listensocket->TooManySockets() ){
			if (GetDownloadState() != DS_TOOMANYCONNS)
				SetDownloadState(DS_TOOMANYCONNS);
			return true;
		}

	}
	//finaly we allow to connect the highest prios
	m_downloadpriority=1;
	//Xman end

	//Xman Xtreme Mod moved to ConnectionEsteblished
	/*
	if (m_bUDPPending)
	{
		m_nFailedUDPPackets++;
		theApp.downloadqueue->AddFailedUDPFileReasks();
	}
	m_bUDPPending = false;
	*/

	//Xman Xtreme Mod: count the TCP sucessfull/failed connections
	if(m_cFailed==0)
		theApp.downloadqueue->AddTCPFileReask();

    //SwapToAnotherFile(_T("A4AF check before tcp file reask. CUpDownClient::AskForDownload()"), true, false, false, NULL, true, true); // ZZ:DownloadManager
	
	// Maella -Unnecessary Protocol Overload-
	// Delay the next refresh of the download session initiated from CPartFile::Process()
	m_dwLastAskedTime = ::GetTickCount();
	
	//Xman 5.1
	//increase the reask-time for  non UDP-clients
	//and give them the chance to connect first. Xtreme will use the connection, see partfile->process
	//remark: don't do this for LowID-Clients, because they aren't reasked, but flagged
	//see TryToConnect
	if((!HasLowID() && HasTooManyFailedUDP()) && GetJitteredFileReaskTime()< MIN2MS(30) )
		CalculateJitteredFileReaskTime(true);
	else if((HasLowID()) && GetJitteredFileReaskTime() > MIN2MS(30))
		CalculateJitteredFileReaskTime(false);
	m_dwNextTCPAskedTime = m_dwLastAskedTime + GetJitteredFileReaskTime();
	// Maella end


	SetDownloadState(DS_CONNECTING);
	return TryToConnect();
}

bool CUpDownClient::IsSourceRequestAllowed() const
{
	//Xman Anti-Leecher
	//>>> Anti-XS-Exploit (Xman)
	if(thePrefs.GetAntiLeecherXSExploiter() && IsXSExploiter())
		return false; //this client doesn't answer
	//Xman end
    return IsSourceRequestAllowed(reqfile); 
} 

//Xman Xtreme mod: modified version:
bool CUpDownClient::IsSourceRequestAllowed(CPartFile* partfile, bool /*sourceExchangeCheck*/) const // ZZ:DownloadManager
{ // ZZ:DownloadManager
	DWORD dwTickCount = ::GetTickCount() + CONNECTION_LATENCY;

	//unsigned int nTimePassedClient = dwTickCount - GetLastSrcAnswerTime();
	//Xman if the remote client doesn't have any sources to send and we do an XS-request for
	//the next file, it won't send sources, because it thinks it has already answered 
	//because of this official bug we refer to GetLastAskedForSources() instead of GetLastSrcAnswerTime()
	unsigned int nTimePassedClient = dwTickCount - GetLastAskedForSources();
	//Xman end
	
	unsigned int nTimePassedFile   = dwTickCount - partfile->GetLastAnsweredTime(); // ZZ:DownloadManager
	

	
	
	bool bNeverAskedBefore = GetLastAskedForSources() == 0;

	//Xman sourcecache
	//if there are cached sources available we don't need new XS
	if(partfile->GetSourceCacheAmount()>0)
		return false;
	//Xman end

	//Xman GlobalMaxHarlimit for fairness
	if(partfile->IsSourceSearchAllowed()==false)
		return false;
	//Xman end

// ZZ:DownloadManager -->
	UINT uSources = partfile->GetSourceCount();
    UINT uValidSources = partfile->GetValidSourcesCount();
	
	return (
	         //if client has the correct extended protocol
	         ExtProtocolAvailable() && GetSourceExchangeVersion() > 1 &&
	         //AND if we need more sources
	         reqfile->GetMaxSourcePerFileSoft() > uSources &&
	         //AND if...
	         (
	           //source is not complete and file is very rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     && (uSources <= RARE_FILE/5)
				 //&& (!sourceExchangeCheck || partfile == reqfile || uValidSources < uReqValidSources && uReqValidSources > 3) // ZZ:DownloadManager
	           ) ||
	           //source is not complete and file is rare
	           ( !m_bCompleteSource
				 && (bNeverAskedBefore || nTimePassedClient > SOURCECLIENTREASKS)
			     //&& (uSources <= RARE_FILE || (!sourceExchangeCheck || partfile == reqfile) && uSources <= RARE_FILE / 2 + uValidSources) // ZZ:DownloadManager
				 //Xman:
				 && (uSources <= RARE_FILE || uSources <= RARE_FILE / 2 + uValidSources ) 
				 && (nTimePassedFile > SOURCECLIENTREASKF)
				 //&& (!sourceExchangeCheck || partfile == reqfile || uValidSources < SOURCECLIENTREASKS/SOURCECLIENTREASKF && uValidSources < uReqValidSources) // ZZ:DownloadManager
	           ) ||
	           // OR if file is not rare
			   ( (bNeverAskedBefore || nTimePassedClient > (unsigned)(SOURCECLIENTREASKS * MINCOMMONPENALTY)) 
				 && (nTimePassedFile > (unsigned)(SOURCECLIENTREASKF * MINCOMMONPENALTY))
				 //&& (!sourceExchangeCheck || partfile == reqfile || uValidSources < SOURCECLIENTREASKS/SOURCECLIENTREASKF && uValidSources < uReqValidSources) // ZZ:DownloadManager
	           )
	         )
	       );
}
//xman end

void CUpDownClient::SendFileRequest()
{
    // normally asktime has already been reset here, but then SwapToAnotherFile will return without much work, so check to make sure
    //SwapToAnotherFile(_T("A4AF check before tcp file reask. CUpDownClient::SendFileRequest()"), true, false, false, NULL, true, true); // ZZ:DownloadManager

	//Xman Xtreme Downloadmanager
	//the purpose of this method is, to balance the clients(sources) between the partfiles
	//example: one file has 60%of hardlimt sources, other only 20%. we should move our sources 
	//to the file with low sources, if the fileprio is the same.
	if(isduringswap==false && GetDownloadState()!=DS_DOWNLOADING //do nothing if new source or we are already during a swapping operation
		&& !reqfile->IsA4AFAuto()) //Xman Xtreme Downloadmanager: Auto-A4AF-check
	{
		CPartFile* cur_file = NULL;
		CPartFile* swaptofile = reqfile;

		if(!m_OtherRequests_list.IsEmpty())
			for (POSITION pos = m_OtherRequests_list.GetHeadPosition();pos != 0;m_OtherRequests_list.GetNext(pos))
			{
				cur_file = m_OtherRequests_list.GetAt(pos);
				if (theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() && (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY)	
					&& (::GetTickCount() - GetLastFileAskedTime(cur_file))>(MIN_REQUESTTIME + 60000) //wasn't requested to early //shouln't happen, but to be sure
					&& cur_file->GetSourceCount() < (0.75 * cur_file->GetMaxSources())	//don't swap to file which has already enough sources
					&& !IsSwapSuspended(cur_file) //we may not swap to this file
					&& cur_file != reqfile
					&& (CPartFile::RightFileHasHigherPrio(swaptofile, cur_file))) //better file found
				{
					//remeber this file:
					swaptofile=cur_file;
				}
			}
			if(!m_OtherNoNeeded_list.IsEmpty())
				for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos))
				{
					cur_file = m_OtherNoNeeded_list.GetAt(pos);
					if (theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() && (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY)	
						&& (::GetTickCount() - GetLastFileAskedTime(cur_file))>(MIN_REQUESTTIME + 60000) //wasn't requested to early //shouln't happen, but to be sure
						&& cur_file->GetSourceCount() < (0.75 * cur_file->GetMaxSources())	//don't swap to file which has already enough sources
						&& !IsSwapSuspended(cur_file) //we may not swap to this file
						&& cur_file != reqfile
						&& (CPartFile::RightFileHasHigherPrio(swaptofile, cur_file))) //better file found
					{
						//remeber this file:
						swaptofile=cur_file;
					}
				}

			//swap if useful:
			if(swaptofile && swaptofile!=reqfile)
			{
				if(thePrefs.GetLogA4AF())
					AddDebugLogLine(false, _T("advanced swapping: client %s, %s swapped from %s to %s"), DbgGetFullClientSoftVer(),GetUserName(), reqfile->GetFileName(), swaptofile->GetFileName());
				SwapToAnotherFile(true, false, false, swaptofile);
			}
	}

	isduringswap=false; //indicates, that we are during a swap operation
	if(enterqueuetime ==0)	//indicates, when a source was first asked
		enterqueuetime = ::GetTickCount();
	//Xman end

	ASSERT(reqfile != NULL);
	if(!reqfile)
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
			SetRemoteQueueRank(0,false); //Xman
		}

		// OP_REQUESTSOURCES
		if (IsSourceRequestAllowed())
		{
			if (thePrefs.GetDebugClientTCPLevel() > 0) {
				DebugSend("OP__MPReqSources", this, reqfile->GetFileHash());
				if (GetLastAskedForSources() == 0)
					Debug(_T("  first source request\n"));
				else
					Debug(_T("  last source request was before %s\n"), CastSecondsToHM((GetTickCount() - GetLastAskedForSources())/1000));
			}
			
			//Xman because official client has a bug, only count the XSReqs under special conditions
			//remark: this fails for complete sources the first time.. but this doesn't hurt
			if( (m_bCompleteSource || !md4cmp(GetUploadFileID(), reqfile->GetFileHash())) //complete source of we want the same file
				&& reqfile->GetValidSourcesCount() > 10) //if we know at least 10 good sources, the remote client should know at least one
				IncXSReqs();//>>> Anti-XS-Exploit (Xman)

			dataFileReq.WriteUInt8(OP_REQUESTSOURCES);
			reqfile->SetLastAnsweredTimeTimeout();
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine(false, _T("SXSend: Client source request; %s, File=\"%s\""), DbgGetClientInfo(), reqfile->GetFileName());
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
			reqfile->WritePartStatus(&dataFileReq);
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
			SetRemoteQueueFull(true);
			SetRemoteQueueRank(0,false); //Xman
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
			//Xman because official client has a bug, only count the XSReqs under special conditions
			//remark: this fails for complete sources the first time.. but this doesn't hurt
			if( (m_bCompleteSource || !md4cmp(GetUploadFileID(), reqfile->GetFileHash())) //complete source of we want the same file
				&& reqfile->GetValidSourcesCount() > 10) //if we know at least 10 good sources, the remote client should know at least one
				IncXSReqs();//>>> Anti-XS-Exploit (Xman)

			reqfile->SetLastAnsweredTimeTimeout();
			Packet* packet = new Packet(OP_REQUESTSOURCES,16,OP_EMULEPROT);
			md4cpy(packet->pBuffer,reqfile->GetFileHash());
			theStats.AddUpDataOverheadSourceExchange(packet->size);
			socket->SendPacket(packet, true, true);
			SetLastAskedForSources();
			if (thePrefs.GetDebugSourceExchange())
				AddDebugLogLine(false, _T("SXSend: Client source request; %s, File=\"%s\""), DbgGetClientInfo(), reqfile->GetFileName());
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

	// Maella -Unnecessary Protocol Overload-
	// Delay the next refresh of the download session initiated from CPartFile::Process() 
	m_dwLastAskedTime = ::GetTickCount();
	m_dwNextTCPAskedTime = m_dwLastAskedTime + GetJitteredFileReaskTime();

	//we will look at m_partstatusmap when swapping
	// Keep a track when this file was asked for the last time 
	m_partStatusMap[reqfile].dwStartUploadReqTime = GetTickCount();

	// Maella end

}

void CUpDownClient::SendStartupLoadReq()
{
	if (socket==NULL || reqfile==NULL)
	{
		ASSERT(0);
		return;
	}

	//Xman 4.4
	//in very rare cases we want sendStartupLoadReq on Downloading
	//this case is NOT right after starting a download... it happend during a downloadsession
	//no idea what was going on here, because the protocolstepflag should handle all cases
	if(GetDownloadState()==DS_DOWNLOADING && protocolstepflag1==false)
	{
		//AddDebugLogLine(false,_T("--> SendStartuploadReq on DS-DOWNLOADING: %s"), DbgGetClientInfo());
		return;
	}

	//Xman fix for startupload (downloading side)
	if(protocolstepflag1)	
	{
		//we know now, client was sending OP_ACCEPTUPLOADREQ
		//he wants do upload to use, so we don't send SendStartupLoadReq
		//but we send now the blockrequest
		StartDownload();
		//AddDebugLogLine(false, _T("-->don't send StartupLoadReq, client: %s, File: %s"), DbgGetClientInfo(), GetClientFilename());
		protocolstepflag1=false;
		return;
	}
	protocolstepflag1=false;
	//Xman end


	SetDownloadState(DS_ONQUEUE);
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

	// Maella -Unnecessary Protocol Overload-
	// Remark: force a TCP refresh of the download session in 2 hours
	//Xman: we increase this time if udpver >3, because we have a valid partstatus
	if(GetUDPVersion()>3)
		m_dwNextTCPAskedTime = m_dwLastAskedTime + 6 * GetJitteredFileReaskTime();
	else
		m_dwNextTCPAskedTime = m_dwLastAskedTime + 4 * GetJitteredFileReaskTime();
	// Maella end

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
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  Filename=\"%s\"\n"), m_strClientFilename);
	//TK4 Mod - [FDC] Check for filename disparity - if checks enabled.
	if(!reqfile->DissimilarName() && thePrefs.BadNameCheck) reqfile->CheckFilename(m_strClientFilename);
	// 26-Jul-2003: removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
	// if the remote client answers the OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
	// know that the file is shared, we know also that the file is complete and don't need to request the file status.
	if (reqfile->GetPartCount() == 1)
	{
		delete[] m_abyPartStatus;
		m_abyPartStatus = NULL;
		m_nPartCount = reqfile->GetPartCount();
		m_abyPartStatus = new uint8[m_nPartCount];
		memset(m_abyPartStatus,1,m_nPartCount);
		m_bCompleteSource = true;
		//Xman client percentage
		hiscompletedparts_percent_down=100;
		//Xman end

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
		}
		UpdateDisplayedInfo();
		reqfile->UpdateAvailablePartsCount();
		// even if the file is <= PARTSIZE, we _may_ need the hashset for that file (if the file size == PARTSIZE)
		if (reqfile->hashsetneeded)
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
		else
		{
			SendStartupLoadReq();
		}
		reqfile->UpdatePartsInfo();
	}
}

void CUpDownClient::ProcessFileStatus(bool bUdpPacket, CSafeMemFile* data, CPartFile* file)
{
	//Xman client percentage
	hiscompletedparts_percent_down=-1;
	//Xman end

	if ( !reqfile || file != reqfile )
	{
		if (reqfile==NULL)
			throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile==NULL)");
		throw GetResString(IDS_ERR_WRONGFILEID) + _T(" (ProcessFileStatus; reqfile!=file)");
	}
	uint16 nED2KPartCount = data->ReadUInt16();
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
		//Xman client percentage
		hiscompletedparts_percent_down=100;
		//Xman end
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
		//Xman client percentage
		sint32 hisfinishedparts=0;
		//Xman end
		while (done != m_nPartCount)
		{
			uint8 toread = data->ReadUInt8();
			for (UINT i = 0; i != 8; i++)
			{
				m_abyPartStatus[done] = ((toread>>i)&1)? 1:0; 	
				if (m_abyPartStatus[done])
				{
					//Xman client percentage
					hisfinishedparts++;
					//Xman end
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
		//Xman client percentage
		hiscompletedparts_percent_down= (sint8)((float)hisfinishedparts/m_nPartCount*100.0f);
		//Xman end
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
	}

	reqfile->UpdateAvailablePartsCount();

	// NOTE: This function is invoked from TCP and UDP socket!
	if (!bUdpPacket)
	{
		if (!bPartsNeeded)
		{
			SetDownloadState(DS_NONEEDEDPARTS, _T("No Needed Parts"), CUpDownClient::DSR_NONEEDEDPARTS); // - Maella -Download Stop Reason-
			//Xman 0.46b
			//Xman Xtreme Downloadmanager
			CPartFile* oldreqfile= reqfile;
			if(SwapToAnotherFile(false, false, false))
			{
				DontSwapTo(oldreqfile);
				if(thePrefs.GetLogA4AF())
					AddDebugLogLine(false, _T("-o- ProcessFileStatus swapping NNS: client %s, %s swaped from %s to %s"), DbgGetFullClientSoftVer(),GetUserName(), oldreqfile->GetFileName(), reqfile->GetFileName());
			}
			//Xman end
		}
		//If we are using the eMule filerequest packets, this is taken care of in the Multipacket!
		else if (reqfile->hashsetneeded)
		{
			if (socket)
			{
				if (thePrefs.GetDebugClientTCPLevel() > 0)
					DebugSend("OP__HashSetRequest", this, reqfile->GetFileHash());
				Packet* packet = new Packet(OP_HASHSETREQUEST,16);
				md4cpy(packet->pBuffer,reqfile->GetFileHash());
				theStats.AddUpDataOverheadFileRequest(packet->size);
				socket->SendPacket(packet, true, true);
				SetDownloadState(DS_REQHASHSET);
				m_fHashsetRequesting = 1;
				reqfile->hashsetneeded = false;
			}
			else
				ASSERT(0);
		}
		else
		{
			SendStartupLoadReq();
		}
		//Xman 4.8.2 moved the update display here. Because in most  cases it has been already updated, if not it will be done now:
		UpdateDisplayedInfo();
		//Xman end
	}
	else
	{
		if (!bPartsNeeded)
		{
			SetDownloadState(DS_NONEEDEDPARTS, _T("No Needed Parts"), CUpDownClient::DSR_NONEEDEDPARTS); // - Maella -Download Stop Reason-
		}
		else
			SetDownloadState(DS_ONQUEUE);
	}
	reqfile->UpdatePartsInfo();
}
//Xman
// Maella -Code Improvement-
bool CUpDownClient::AddRequestForAnotherFile(CPartFile* file){
	if(m_OtherRequests_list.Find(file) != NULL) return false; // Found
	if(m_OtherNoNeeded_list.Find(file) != NULL) return false; // Found
	m_OtherRequests_list.AddTail(file);

	file->A4AFsrclist.AddTail(this); // [enkeyDEV(Ottavio84) -A4AF-]

	return true;
}
//Xman end

void CUpDownClient::ClearDownloadBlockRequests()
{
	for (POSITION pos = m_DownloadBlocks_list.GetHeadPosition();pos != 0;){
		Requested_Block_Struct* cur_block = m_DownloadBlocks_list.GetNext(pos);
		if (reqfile){
			reqfile->RemoveBlockFromList(cur_block->StartOffset,cur_block->EndOffset);
		}
		delete cur_block;
	}
	m_DownloadBlocks_list.RemoveAll();

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

void CUpDownClient::SetDownloadState(EDownloadState nNewState, LPCTSTR pszReason, DownStopReason reason){ // Maella -Download Stop Reason-
	if (m_nDownloadState != nNewState){
		//Xman
		// Maella -Unnecessary Protocol Overload-
		if(nNewState==DS_TOOMANYCONNSKAD)
			//This client had already been set to DS_CONNECTING.
			//So we reset this time so it isn't stuck at TOOMANYCONNS for 20mins.
			SetNextTCPAskedTime(::GetTickCount()+60000); //wait 60 sec before the next retry 
		//Maella end

		//Xman fix for startupload (downloading side)
		if(nNewState==DS_NONEEDEDPARTS)
		{
			protocolstepflag1=false;
			//Xman DiffQR + official bugfix 
			SetRemoteQueueRank(0,false); //Xman display is updated few lines below
			oldQR=0; 
			//Xman end
		}
		//Xman end

		if (reqfile){
		    if(nNewState == DS_DOWNLOADING){
				if(thePrefs.GetLogUlDlEvents())
					AddDebugLogLine(DLP_VERYLOW, false, _T("Download session started. User: %s in SetDownloadState()."), DbgGetClientInfo()); //Xman Code Improvement

			    reqfile->AddDownloadingSource(this);
		    }
		    else if(m_nDownloadState == DS_DOWNLOADING){
			    reqfile->RemoveDownloadingSource(this);
		    }
		}

        if(nNewState == DS_DOWNLOADING && socket){
		    socket->SetTimeOut(CONNECTION_TIMEOUT*4);
			
			//Xman Xtreme Mod: improved socket-options
			int newValue = 16*1024;
			//int size = sizeof(newValue);
			if(socket != NULL)
			{
				socket->SetSockOpt(SO_RCVBUF, &newValue, sizeof(newValue), SOL_SOCKET);
			
				/*
				int oldValue=0;
				int size=sizeof(oldValue);
				socket->GetSockOpt(SO_RCVBUF, &oldValue, &size, SOL_SOCKET);
				AddDebugLogLine(false, _T("buffer changed to %i"), oldValue);
				*/
				// Pawcio: BC
				BOOL noDelay=true;
				if(!socket->SetSockOpt(TCP_NODELAY,&noDelay,sizeof(BOOL),IPPROTO_TCP))
					AddDebugLogLine(false,_T("failed to set NODELAY"));
			}
			//Xman end
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

				AddDebugLogLine(DLP_VERYLOW, false, _T("Download session ended: %s User: %s in SetDownloadState(). New State: %i, Length: %s, Payload: %s, Transferred: %s, Req blocks not yet completed: %i."), pszReason, DbgGetClientInfo(), nNewState, CastSecondsToHM(GetDownTimeDifference(false)/1000), CastItoXBytes(GetSessionPayloadDown(), false, false), CastItoXBytes(GetSessionDown(), false, false), m_PendingBlocks_list.GetCount());
			}


			//Xman filter clients with failed downloads
			if(GetSessionDown()< (12*1024)) //12 kb
				m_faileddownloads++;
			else
				m_faileddownloads=0; 
			//Xman end


			// -khaos--+++> Extended Statistics (Successful/Failed Download Sessions)
			if ( m_bTransferredDownMini && nNewState != DS_ERROR )
			{
				CUpDownClient::AddDownStopCount(false, reason); // - Maella -Download Stop Reason-
				thePrefs.Add2DownSuccessfulSessions(); // Increment our counters for successful sessions (Cumulative AND Session)
			}
			else
			{
				CUpDownClient::AddDownStopCount(true, reason); // - Maella -Download Stop Reason-
				thePrefs.Add2DownFailedSessions(); // Increment our counters failed sessions (Cumulative AND Session)
			}
			thePrefs.Add2DownSAvgTime(GetDownTimeDifference()/1000);
			// <-----khaos-

			enterqueuetime = 0; //Xman Xtreme Downloadmanager

			ResetSessionDown(); //Xman filter clients with failed downloads

			m_nDownloadState = (_EDownloadState)nNewState;

			ClearDownloadBlockRequests();
			
			//Xman				
			// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
			m_nDownDatarate = 0;
			m_nDownDatarate10=0;
			m_nDownDatarateMeasure = 0;
			m_downHistory_list.RemoveAll();
			// Maella end

			//Xman
			// Maella -Unnecessary Protocol Overload-
			if(reqfile != NULL){
				// Reask for the file as soon as possible.
				//remark: we do this for all states, even NoNeededPart, because we don't know if we are up to date
				TrigNextSafeAskForDownload(reqfile);
			}
			//Xman new trick at this point:
			//problem is... the client was transferring and then we don't need any parts
			//although we don't have an actual partstatus... we wait double reasktime until next request
			//better way: we request a new partstatus as long as we have a socket
			if(nNewState==DS_NONEEDEDPARTS)
				nNewState=DS_ONQUEUE; //will fall back to noneeded if we realy no need parts
			//Xman end

			if (nNewState == DS_NONE){
				delete[] m_abyPartStatus;
				m_abyPartStatus = NULL;
				m_nPartCount = 0;
			}
			if (socket && nNewState != DS_ERROR )
				socket->DisableDownloadLimit();
		}
		m_nDownloadState = (_EDownloadState)nNewState;
		if( GetDownloadState() == DS_DOWNLOADING ){
			if ( IsEmuleClient() )
				SetRemoteQueueFull(false);
			SetRemoteQueueRank(0,false); //Xman display is updated few lines below
			oldQR=0; //Xman DiffQR
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
		//Xman
		reqfile->PerformFirstHash();		// SLUGFILLER: SafeHash - Rehash
	}
	else{
		reqfile->hashsetneeded = true;
		throw GetResString(IDS_ERR_BADHASHSET);
	}
	SendStartupLoadReq();
}

void CUpDownClient::CreateBlockRequests(int iMaxBlocks)
{
	ASSERT( iMaxBlocks >= 1 /*&& iMaxBlocks <= 3*/ );
	if (m_DownloadBlocks_list.IsEmpty())
	{
		uint16 count;
		//Xman bugfix
		if(iMaxBlocks > m_PendingBlocks_list.GetCount()) {
			count = (uint16)(iMaxBlocks - m_PendingBlocks_list.GetCount());
		 
		
			Requested_Block_Struct** toadd = new Requested_Block_Struct*[count];
			//Xman chunk chooser
			bool result = thePrefs.GetChunkChooseMethod()==1 ? reqfile->GetNextRequestedBlock_Maella(this,toadd,&count) : reqfile->GetNextRequestedBlock_zz(this,toadd,&count);
			if (result){
			//Xman end chunk chooser
				for (UINT i = 0; i < count; i++)
					m_DownloadBlocks_list.AddTail(toadd[i]);
			}
			delete[] toadd;
		}
		//Xman end
	}

	while (m_PendingBlocks_list.GetCount() < iMaxBlocks && !m_DownloadBlocks_list.IsEmpty())
	{
		Pending_Block_Struct* pblock = new Pending_Block_Struct;
		pblock->block = m_DownloadBlocks_list.RemoveHead();
		m_PendingBlocks_list.AddTail(pblock);
	}
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
	if(IsEmuleClient() && m_byCompatibleClient==0 && reqfile->GetFileSize()-reqfile->GetCompletedSize() <= (uint64)PARTSIZE*4) {
		// if there's less than two chunks left, request fewer blocks for
		// slow downloads, so they don't lock blocks from faster clients.
		// Only trust eMule clients to be able to handle less blocks than three
		if(m_nDownDatarate10 < 600 || GetSessionPayloadDown() < 40*1024) { //Xman Xtreme Mod
			blockCount = 1;
		} else if(m_nDownDatarate10 < 2400) { //Xman Xtreme Mod, Xman 5.1 raised from 1200
			blockCount = 2;
		}
	}
	CreateBlockRequests(blockCount);
	
	if (m_PendingBlocks_list.IsEmpty()){
		/*
		//Xman 0.46b
		//Xman Xtreme Downloadmanager
		
		SendCancelTransfer();
		SetDownloadState(DS_NONEEDEDPARTS, _T("No Needed Parts") , CUpDownClient::DSR_NONEEDEDPARTS); // - Maella -Download Stop Reason-
		
		
		CPartFile* oldreqfile= reqfile;
		if(SwapToAnotherFile(false, false, false))
		{
			DontSwapTo(oldreqfile);
			if(thePrefs.GetLogA4AF())
				AddDebugLogLine(false, _T("SendBlockRequests swapping NNS: client %s, %s swaped from %s to %s"), DbgGetFullClientSoftVer(),GetUserName(), oldreqfile->GetFileName(), reqfile->GetFileName());
		}
		*/
		
		//Xman 4.2 just in time swapping
		//try do swap just in time
		CPartFile* oldreqfile=reqfile;
		if(GetDownloadState()==DS_DOWNLOADING && socket!=NULL && socket->IsConnected() && SwapToAnotherFile(false,false,false, NULL, false, true)  )
		{
			protocolstepflag1=true;
			SendFileRequest(); //ask for the file we swapped to
			if(thePrefs.GetLogA4AF())
				AddDebugLogLine(false, _T("-o- SendBlockRequests just in time swapping NNS: client %s, %s swapped from %s to %s"), DbgGetFullClientSoftVer(),GetUserName(), oldreqfile->GetFileName(), reqfile->GetFileName());
		}
		else
		{
			SendCancelTransfer();
			SetDownloadState(DS_NONEEDEDPARTS, _T("No Needed Parts") , CUpDownClient::DSR_NONEEDEDPARTS); // - Maella -Download Stop Reason-
		}
		//to be sure not to fall in an endless loop (which theoretically can't happen):
		DontSwapTo(oldreqfile);
		

		//Xman end
		return;
	}

	bool bI64Offsets = false;
	POSITION pos = m_PendingBlocks_list.GetHeadPosition();
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
					return; //Xman Bugfix - we may not send the blockrequest
				}
				break;
			}
		}
	}

	Packet* packet;
	if (bI64Offsets){
		const int iPacketSize = 16+(3*8)+(3*8); // 64
		packet = new Packet(OP_REQUESTPARTS_I64, iPacketSize, OP_EMULEPROT);
		CSafeMemFile data((const BYTE*)packet->pBuffer, iPacketSize);
		data.WriteHash16(reqfile->GetFileHash());
		pos = m_PendingBlocks_list.GetHeadPosition();
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
		pos = m_PendingBlocks_list.GetHeadPosition();
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
		pos = m_PendingBlocks_list.GetHeadPosition();
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
		pos = m_PendingBlocks_list.GetHeadPosition();
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

	//Xman moved down
	//MORPH::Avoid Credits Accumulate Fakers
	//if (credits)
	//	credits->AddDownloaded(uTransferredFileDataSize, GetIP());
	bool bPacketUsefull = false;
	//Xman end

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
					AddDebugLogLine(false, _T("PrcBlkPkt: Ignoring %u bytes of block starting at %u because of errornous zstream state for file \"%s\" - %s"), uTransferredFileDataSize, nStartPos, reqfile->GetFileName(), DbgGetClientInfo());
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
				bPacketUsefull = true; //Xman MORPH::Avoid Credits Accumulate Fakers
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
							if (thePrefs.GetVerbose())
								DebugLogError(_T("PrcBlkPkt: ") + GetResString(IDS_ERR_CORRUPTCOMPRPKG),reqfile->GetFileName(),666);
							reqfile->RemoveBlockFromList(cur_block->block->StartOffset, cur_block->block->EndOffset);
							// There is no chance to recover from this error
						}
						else{
							bPacketUsefull = true; //Xman MORPH::Avoid Credits Accumulate Fakers
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

			//Xman
			//MORPH::Avoid Credits Accumulate Fakers
			if(bPacketUsefull)
			{
				if (credits)
					credits->AddDownloaded(uTransferredFileDataSize, GetIP());
			}
			//Xman end

			// Stop looping and exit method
			return;
		}
	}

	TRACE("%s - Dropping packet\n", __FUNCTION__);
}

int CUpDownClient::unzip(Pending_Block_Struct* block, const BYTE* zipped, uint32 lenZipped, BYTE** unzipped, uint32* lenUnzipped, int iRecursion)
{
#define TRACE_UNZIP	/*TRACE*/

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


// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
void CUpDownClient::CompDownloadRate(){
	// Add new sample
	TransferredData newItem;
	newItem.dataLength = m_nDownDatarateMeasure;
	newItem.timeStamp  = ::GetTickCount();
	m_downHistory_list.AddHead(newItem);

	// Remove old sample(s)
	while(m_downHistory_list.GetSize() > max(thePrefs.GetDatarateSamples() +1, 11)){ //Xman 0.46b hold at least 10 samples
		m_downHistory_list.RemoveTail();
	}

	if(m_downHistory_list.GetSize() > 1){	
		// Compute datarate (=> display)
		POSITION pos = m_downHistory_list.FindIndex(thePrefs.GetDatarateSamples());
		if(pos == NULL){
			pos = m_downHistory_list.GetTailPosition();
		}
		TransferredData& oldestSample = m_downHistory_list.GetAt(pos);
		uint32 deltaTime = newItem.timeStamp - oldestSample.timeStamp;
		uint32 deltaByte = newItem.dataLength - oldestSample.dataLength;
		m_nDownDatarate = (deltaTime > 0) ? (UINT)(1000.0 * deltaByte / deltaTime) : 0;   // [bytes/s]
	}

	//Xman 0.46b for faster endgame
	if(m_downHistory_list.GetSize() > 3){	//wait 4 seconds to have a valid value
		// Compute datarate (=> avg10 seconds for faster endgame)
		POSITION pos = m_downHistory_list.FindIndex(10);
		if(pos == NULL){
			pos = m_downHistory_list.GetTailPosition();
		}
		TransferredData& oldestSample = m_downHistory_list.GetAt(pos);
		uint32 deltaTime = newItem.timeStamp - oldestSample.timeStamp;
		uint32 deltaByte = newItem.dataLength - oldestSample.dataLength;
		m_nDownDatarate10 = (deltaTime > 0) ? (UINT)(1000.0 * deltaByte / deltaTime) : 0;   // [bytes/s]
	}


	// Check and then refresh GUI
	m_displayDownDatarateCounter++;
	//Xman Code Improvement: slower refresh for clientlist
	if(m_displayDownDatarateCounter%DISPLAY_REFRESH == 0 ){
		theApp.emuledlg->transferwnd->downloadlistctrl.UpdateItem(this);
		theApp.emuledlg->transferwnd->downloadclientsctrl.RefreshClient(this);
	}
	if(m_displayDownDatarateCounter%DISPLAY_REFRESH_CLIENTLIST == 0 ){
		theApp.emuledlg->transferwnd->clientlistctrl.RefreshClient(this);
		m_displayDownDatarateCounter = 0;
	}
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
			SetDownloadState(DS_ONQUEUE, _T("Timeout. More than 100 seconds since last complete block was received.") , CUpDownClient::DSR_TIMEOUT); // - Maella -Download Stop Reason-
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

//Xman Xtreme Downloadmanager
//0.46a: remark: since 0.46a the official emuel also uses a updatedisplay-flag. The one of Xtreme-Mod is used in a different way!
void CUpDownClient::SetRemoteQueueRank(UINT nr, bool updatedisplay){ 
	if(reqfile) reqfile->CalcAvgQr(nr, m_nRemoteQueueRank);
	//Xman DiffQR 
	if(m_nRemoteQueueRank!=0) oldQR=m_nRemoteQueueRank;
	//Xman end
	m_nRemoteQueueRank = nr;
	if(updatedisplay)
		UpdateDisplayedInfo(true);
}
//Xman end

void CUpDownClient::UDPReaskACK(uint16 nNewQR)
{
	m_bUDPPending = false;
	if(GetDownloadState()!=DS_NONEEDEDPARTS) //Xman diffQR and official bugfix, which would sort this source wrong
		SetRemoteQueueRank(nNewQR); //Xman

	// Maella -Unnecessary Protocol Overload-
	// Delay the next refresh of the download session initiated from CPartFile::Process()
	m_dwLastAskedTime = ::GetTickCount();
	// Keep a track when this file was asked for the last time to avoid a Ban()
	if(reqfile)
		m_partStatusMap[reqfile].dwStartUploadReqTime = GetTickCount();
	// Maella end

}

void CUpDownClient::UDPReaskFNF()
{
	m_bUDPPending = false;
	if (GetDownloadState() != DS_DOWNLOADING){ // avoid premature deletion of 'this' client
		//if (thePrefs.GetVerbose())
			//AddDebugLogLine(DLP_LOW, false, _T("UDP FNF-Answer: %s - %s"),DbgGetClientInfo(), DbgGetFileInfo(reqfile ? reqfile->GetFileHash() : NULL)); //Xman final version

		//Xman UDPReaskFNF-Fix against Leechers (idea by WiZaRd)		
		if(reqfile && GetUploadState()!=US_NONE)
		{
			CKnownFile* upfile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
			if(upfile && upfile == reqfile) //we speak about the same file
			{
				AddDebugLogLine(false,_T("Dropped src: (%s) does not seem to have own reqfile!"), DbgGetClientInfo());
				theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Src says he does not have the file he's dl'ing"));
				theApp.uploadqueue->RemoveFromWaitingQueue(this);
			}
		}
		//Xman end

		if (reqfile)
			reqfile->m_DeadSourceList.AddDeadSource(this);
		switch (GetDownloadState()) {
			case DS_ONQUEUE:
			case DS_NONEEDEDPARTS:
                //Xman Xtreme Downloadmanager
				if (SwapToAnotherFile(true, true, true, NULL))
				{
					break;
				}
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

// Maella -Unnecessary Protocol Overload-
void CUpDownClient::UDPReaskForDownload()
{
	ASSERT ( reqfile );
	// This method can be called only every 30 seconds
	if((reqfile == NULL) || // No file to ping
		(m_abyPartStatus == NULL) || // File status unknown yet
		(GetTickCount() - m_dwLastUDPReaskTime < 30000)) // Every 30 seconds only
		return;


	if( HasTooManyFailedUDP()) //Xman own method
		return;


	//Xman 4.8.2 code-Improvement
	//it can happen that our UDP-packet is received by remote client 
	//but the answer get always lost. In this case it makes no sence to send the second UDP-packet
	if(m_bUDPPending==true && m_nTotalUDPPackets-1==m_nFailedUDPPackets)
		return; //not one UDP-answer until now->don't try it again
	//Xman end

	// Time stamp
	m_dwLastUDPReaskTime = GetTickCount();


	if (GetUDPPort() != 0 && GetUDPVersion() != 0 && thePrefs.GetUDPPort() != 0 &&
		!theApp.IsFirewalled() && !(socket && socket->IsConnected()) && !thePrefs.GetProxySettings().UseProxy)
	{ 
		if( !HasLowID() )
		{
			//don't use udp to ask for sources
			if(IsSourceRequestAllowed())
			{
				SetNextTCPAskedTime(0); //Xman force TCP
				return;
			}


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
			if(!m_bUDPPending) //Xman don't add the second udpreask, otherwise wrong statistic
			{			
				theApp.downloadqueue->AddUDPFileReasks();
				m_nTotalUDPPackets++;
			}
			m_bUDPPending = true;
			theApp.clientudp->SendPacket(response,GetIP(),GetUDPPort(), ShouldReceiveCryptUDPPackets(), GetUserHash());
			//Xman make more per UDP at  clients with new UDP protocol, but let them the possibility to passive find you
			if(GetUDPVersion()>3 && m_OtherNoNeeded_list.IsEmpty() && m_OtherRequests_list.IsEmpty() && (GetNextTCPAskedTime()  < MIN2MS(45) + ::GetTickCount())
				&& (m_bCompleteSource || GetUploadState()==US_ONUPLOADQUEUE))
				SetNextTCPAskedTime(GetNextTCPAskedTime() + GetJitteredFileReaskTime());

		}
		else if (HasLowID() && GetBuddyIP() && GetBuddyPort() && HasValidBuddyID())
		{
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
			if(!m_bUDPPending) //Xman don't add the second udpreask, otherwise wrong statistic
			{
				theApp.downloadqueue->AddUDPFileReasks();
				m_nTotalUDPPackets++;
			}
			m_bUDPPending = true;
			theApp.clientudp->SendPacket(response, GetBuddyIP(), GetBuddyPort(), false, NULL);  // kad doesnt supports obfuscation yet
			//Xman make more per UDP at  clients with new UDP protocol, but let them the possibility to passive find you
			if(GetUDPVersion()>3 && m_OtherNoNeeded_list.IsEmpty() && m_OtherRequests_list.IsEmpty() && (GetNextTCPAskedTime()  < MIN2MS(45) + ::GetTickCount())
				&& (m_bCompleteSource || GetUploadState()==US_ONUPLOADQUEUE))
				SetNextTCPAskedTime(GetNextTCPAskedTime() + GetJitteredFileReaskTime());
		}
	}
}
// Maella end

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

//Xman used for downloadlistctrl
const bool CUpDownClient::IsInNoNeededList(const CPartFile* fileToCheck) const
{
    for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos)) {
        if(m_OtherNoNeeded_list.GetAt(pos) == fileToCheck) {
            return true;
        }
    }

    return false;
}


//Xman Xtreme Downloadmanager
// IgnoreNoNeeded = will switch to files of which this source has no needed parts (if no better fiels found)
// ignoreSuspensions = ignore timelimit for A4Af jumping
// bRemoveCompletely = do not readd the file which the source is swapped from to the A4AF lists (needed if deleting or stopping a file)
// toFile = Try to swap to this partfile only
// lookatmintime = proof, if the swapto file wasn't asket the last minrequesttime //Xman
// allow_go_over_hardlimit = may swap a source to a file which has already enough sources
bool CUpDownClient::SwapToAnotherFile(bool bIgnoreNoNeeded, bool ignoreSuspensions, bool bRemoveCompletely, CPartFile* toFile, bool lookatmintime, bool allow_go_over_hardlimit){

	//Xman 4.2 just in time swapping
	//if (GetDownloadState() == DS_DOWNLOADING)
	//	return false;

	CPartFile* SwapTo = NULL;
	CPartFile* cur_file = NULL;
	POSITION finalpos = NULL;
	CTypedPtrList<CPtrList, CPartFile*>* usedList=&m_OtherRequests_list; //Xman just to disable the warning

	if (!m_OtherRequests_list.IsEmpty()){
		usedList = &m_OtherRequests_list;
		for (POSITION pos = m_OtherRequests_list.GetHeadPosition();pos != 0;m_OtherRequests_list.GetNext(pos)){
			cur_file = m_OtherRequests_list.GetAt(pos);
			if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY)	
				//Xman: lookatmintime, only allow to swap, if the swapto wasn't asked the last 10 minutes
				//Xman: lookatmintime is set at manuel swapping
				&& (lookatmintime==false || (::GetTickCount() - GetLastFileAskedTime(cur_file))>(MIN_REQUESTTIME + 60000)))
			{
				if (toFile != NULL){
					if (cur_file == toFile){
						SwapTo = cur_file;
						finalpos = pos;
						break;
					}
				}
				else if (CPartFile::RightFileHasHigherPrio(SwapTo, cur_file, allow_go_over_hardlimit)
					&& (ignoreSuspensions  || (!ignoreSuspensions && !IsSwapSuspended(cur_file)) ) )
				{
					SwapTo = cur_file;
					finalpos=pos;
				}
			}
		}
	}


	if (!SwapTo && bIgnoreNoNeeded){
		usedList = &m_OtherNoNeeded_list;
		for (POSITION pos = m_OtherNoNeeded_list.GetHeadPosition();pos != 0;m_OtherNoNeeded_list.GetNext(pos)){
			cur_file = m_OtherNoNeeded_list.GetAt(pos);
			if (cur_file != reqfile && theApp.downloadqueue->IsPartFile(cur_file) && !cur_file->IsStopped() 
				&& (cur_file->GetStatus(false) == PS_READY || cur_file->GetStatus(false) == PS_EMPTY) 	
				//Xman: lookatmintime, only allow to swap, if the swapto wasn't asked the last 10 minutes
				//Xman: lookatmintime is set at manuel swapping
				&& (lookatmintime==false || (::GetTickCount() - GetLastFileAskedTime(cur_file))>(MIN_REQUESTTIME + 60000)))
			{
				if (toFile != NULL){
					if (cur_file == toFile){
						SwapTo = cur_file;
						finalpos = pos;
						break;
					}
				}
				else if ( CPartFile::RightFileHasHigherPrio(SwapTo, cur_file, allow_go_over_hardlimit)
					&& (ignoreSuspensions  || (!ignoreSuspensions && !IsSwapSuspended(cur_file)) ) )
				{
					SwapTo = cur_file;
					finalpos=pos;
				}
			}
		}
	}

	if (SwapTo ){
		//AddDebugLogLine(false, "Swapped source '%s'; Status %i; Remove %s to %s", this->GetUserName(), this->GetDownloadState(), (bRemoveCompletely ? "Yes" : "No" ), SwapTo->GetFileName());		
		if (DoSwap(SwapTo,bRemoveCompletely)){

			usedList->RemoveAt(finalpos);
			return true;
		}
	}

	return false;
}

//Xman end

//Xman Xtreme Downloadmanager
bool CUpDownClient::DoSwap(CPartFile* SwapTo, bool bRemoveCompletely) {
	POSITION pos = reqfile->srclist.Find(this);
	if(pos)
	{
		// remove this client from the A4AF list of our new reqfile
		POSITION pos2 = SwapTo->A4AFsrclist.Find(this);
		if (pos2){
			SwapTo->A4AFsrclist.RemoveAt(pos2);
			theApp.emuledlg->transferwnd->downloadlistctrl.RemoveSource(this,SwapTo);
		}

		reqfile->srclist.RemoveAt(pos);
		reqfile->RemoveDownloadingSource(this);

		if(!bRemoveCompletely)
		{
			reqfile->A4AFsrclist.AddTail(this);
			if (GetDownloadState() == DS_NONEEDEDPARTS)
				m_OtherNoNeeded_list.AddTail(reqfile);
			else
				m_OtherRequests_list.AddTail(reqfile);

				theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(reqfile,this,true);
		}
		else
		{
			// Maella -Unnecessary Protocol Overload-
			PartStatusMap::iterator it = m_partStatusMap.find(reqfile); 
			if(it != m_partStatusMap.end()){
				m_partStatusMap.erase(it);
			}
			//Maella end
		}

		//Xman 4.2 just in time swapping
		if(GetDownloadState()==DS_DOWNLOADING)
		{
			ClearDownloadBlockRequests();
			m_lastPartAsked = (uint16)-1; //0xFFFF
		}
		else
		//Xman just in time swapping end
            SetDownloadState(DS_NONE);
		
		CPartFile* pOldRequestFile = reqfile;
		SetRequestFile(SwapTo);	
		pOldRequestFile->UpdatePartsInfo();
		pOldRequestFile->UpdateAvailablePartsCount();

		SwapTo->srclist.AddTail(this);
		theApp.emuledlg->transferwnd->downloadlistctrl.AddSource(SwapTo,this,false);
		
		//Xman 4.2 just in time swapping
		if(GetDownloadState()==DS_DOWNLOADING)
			SwapTo->AddDownloadingSource(this);
		//Xman just in time swapping end

		isduringswap=true;


		return true;
	}
	return false;
}
//Xman end

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
//Xman Xtreme Downloadmanager
bool CUpDownClient::IsSwapSuspended(CPartFile* file){
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
		/* Xman x4.1 Code Improvement
		//this part would cause emule to hang! Not needed because of the new cleanup
		else if (m_DontSwap_list.GetAt(pos).file == NULL) // in which cases should this happen?
		{
			m_DontSwap_list.RemoveAt(pos);
		}*/
	}

	return false;
}
//Xman end

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
	m_lastPartAsked = (uint16)-1; // =0xffff
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
	reqfile = pReqFile;
}

void CUpDownClient::ProcessAcceptUpload()
{
	m_fQueueRankPending = 1;
	if (reqfile && !reqfile->IsStopped() && (reqfile->GetStatus()==PS_READY || reqfile->GetStatus()==PS_EMPTY))
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
		}
		//Xman Xtreme Mod
		//Xman fix for startupload (downloading side)
		else if(protocolstepflag1)
		{
			//do nothing
			//it seems there are buggy clients which are sending OP_ACCEPTUPLOADREQ each time after receiving some opcodes like OP_SETREQUFILEID
			//in this case we shouldn't repeat our below actions 
			//AddDebugLogLine(false, _T("-->warning! OP_ACCEPTUPLOADREQ from protocolstepflag1, clientversion:%s, File: %s"), DbgGetClientInfo(), GetClientFilename());
		}
		else if(GetDownloadState() == DS_CONNECTED)
		{
			//the problem here is: we are during sending our packtes (e.g. OP_SETREQUFILEID) and the download begin
			//until we send OP_STARTUPLOADREQ. In this case, there is no need to send OP_STARTUPLOADREQ
			protocolstepflag1=true;
			//AddDebugLogLine(false, _T("-->OP_ACCEPTUPLOADREQ from DS_CONNECTED, clientversion:%s, File: %s"), DbgGetClientInfo(), GetClientFilename());
		}
		//Xman end fix for startupload (downloading side)
		else if (GetDownloadState() == DS_DOWNLOADING)
		{
			//don't know why this happens, but it happens
			//only send a new blockrequest
			SendBlockRequests();
			//AddDebugLogLine(false, _T("-->OP_ACCEPTUPLOADREQ from DS_DOWNLOADING, clientversion:%s, File: %s"), DbgGetClientInfo(), GetClientFilename());
		}
		else if (theApp.IsConnected()==false)
		{
			//anti-leecher protection
			SendCancelTransfer();
			SetDownloadState(DS_NONE  , _T("paused file"), CUpDownClient::DSR_PAUSED); // - Maella -Download Stop Reason-
			return;
		}
		else if(GetDownloadState() == DS_NONEEDEDPARTS)
		{
			//this case only happens in very rare situations (and not with Xtreme Mod)
						
			//try do swap just in time
			CPartFile* oldreqfile=reqfile;
			if(SwapToAnotherFile(false,false,false, NULL, false, true))
			{
				protocolstepflag1=true;
				SendFileRequest(); //ask for the file we swapped to
				if(thePrefs.GetLogA4AF())
					AddDebugLogLine(false, _T("-o- ProcessAcceptUpload just in time swapping NNS: client %s, %s swapped from %s to %s"), DbgGetFullClientSoftVer(),GetUserName(), oldreqfile->GetFileName(), reqfile->GetFileName());
			}
			//to be sure not to fall in an endless loop (which theoretically can't happen):
			DontSwapTo(oldreqfile);
		}
		else if(m_abyPartStatus)
		{
			//if we have a partstatus, we can try to download (can happen at toomanyconnections)
			//AddDebugLogLine(false, _T("-->OP_ACCEPTUPLOADREQ from bad state but we have partstatus, clientversion:%s, File: %s"), DbgGetClientInfo(), GetClientFilename());
			SetSentCancelTransfer(0);
			StartDownload();
		}
		else
		{
			//AddDebugLogLine(false, _T("-->OP_ACCEPTUPLOADREQ from bad state no partstatus-->reask the client, clientversion:%s, File: %s"), DbgGetClientInfo(), GetClientFilename());
			protocolstepflag1=true;
			SendFileRequest();
		}
		//Xman end Xtreme Mod
		
	}
	else
	{
		SendCancelTransfer();
		SetDownloadState((reqfile==NULL || reqfile->IsStopped()) ? DS_NONE : DS_ONQUEUE , _T("paused file"), CUpDownClient::DSR_PAUSED); // - Maella -Download Stop Reason-
	}
}

void CUpDownClient::ProcessEdonkeyQueueRank(const uchar* packet, UINT size)
{
	CSafeMemFile data(packet, size);
	uint32 rank = data.ReadUInt32();
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		Debug(_T("  QR=%u (prev. %d)\n"), rank, IsRemoteQueueFull() ? (UINT)-1 : (UINT)GetRemoteQueueRank());
	//Xman DiffQR + official bugfix
	//remark: at NONEEDEDPARTS we don't send STARTUPLOADREQUEST and therefore we shouldn't receive a querank
	//but some versions (shareazas) send it. here I filter it out
	if(GetDownloadState()!=DS_NONEEDEDPARTS)
		SetRemoteQueueRank(rank); //Xman	
	//Xman end
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
	//Xman DiffQR + official bugfix
	//remark: at NONEEDEDPARTS we don't send STARTUPLOADREQUEST and therefore we shouldn't receive a querank
	//but some versions (shareazas) send it. here I filter it out
	if(GetDownloadState()!=DS_NONEEDEDPARTS)
		SetRemoteQueueRank(rank); //Xman	
	//Xman end

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
					//Ban(_T("QR flood")); //Xman we filter!
				}
				throw CString(/*thePrefs.GetLogBannedClients() ?*/ _T("QR flood") /*: _T("")*/); //Xman
			}
		}
	}
	else
	{
		m_fQueueRankPending = 0;
		m_fUnaskQueueRankRecv = 0;
	}
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
	pForFile->GetAICHHashset()->GetMasterHash().Write(&data);
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
		if ( (pPartFile->GetAICHHashset()->GetStatus() == AICH_TRUSTED || pPartFile->GetAICHHashset()->GetStatus() == AICH_VERIFIED)
			 && ahMasterHash == pPartFile->GetAICHHashset()->GetMasterHash())
		{
			if(pPartFile->GetAICHHashset()->ReadRecoveryData((uint64)request.m_nPart*PARTSIZE, &data)){
				// finally all checks passed, everythings seem to be fine
				AddDebugLogLine(DLP_DEFAULT, false, _T("AICH Packet Answer: Succeeded to read and validate received recoverydata"));
				CAICHHashSet::RemoveClientAICHRequest(this);
				pPartFile->AICHRecoveryDataAvailable(request.m_nPart);
				return;
			}
			else
				AddDebugLogLine(DLP_DEFAULT, false, _T("AICH Packet Answer: Succeeded to read and validate received recoverydata"));
		}
		else
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Answer: Masterhash differs from packethash or hashset has no trusted Masterhash"));
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
				AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Sucessfully created and send recoverydata for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
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
			AddDebugLogLine(DLP_HIGH, false, _T("AICH Packet Request: Failed to create ecoverydata - Hashset not ready or requested Hash differs from Masterhash for %s to %s"), pKnownFile->GetFileName(), DbgGetClientInfo());
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
		pPartFile->GetAICHHashset()->UntrustedHashReceived(ahMasterHash, GetConnectIP());
	}
	else
		AddDebugLogLine(DLP_HIGH, false, _T("ProcessAICHFileHash(): PartFile not found or Partfile differs from requested file, %s"), DbgGetClientInfo());
}

//Xman
// Maella -Extended clean-up II-
void CUpDownClient::CleanUp(CPartFile* pDeletedFile){
	// Check if all pointers to the delete file have been removed
	if(reqfile == pDeletedFile){
		ASSERT(FALSE);
		reqfile = NULL;
		AddDebugLogLine(false, _T("CleanUp() reports an error with reqfile"));
	}

	//Xman x4.1
	for (POSITION pos = m_DontSwap_list.GetHeadPosition(); pos != 0 ; )
	{
		POSITION cur_pos=pos;
		CPartFile* cur_file = m_DontSwap_list.GetNext(pos).file;
		if(cur_file == pDeletedFile)
		{
				m_DontSwap_list.RemoveAt(cur_pos);
		}
	}

	for(POSITION pos = m_OtherRequests_list.GetHeadPosition(); pos != 0; ){
		POSITION cur_pos = pos;
		CPartFile* cur_file = m_OtherRequests_list.GetNext(pos);
		if(cur_file == pDeletedFile){
			m_OtherRequests_list.RemoveAt(cur_pos);
			AddDebugLogLine(false, _T("CleanUp() reports an error with m_OtherRequests_list"));
		}
	}

	for(POSITION pos = m_OtherNoNeeded_list.GetHeadPosition(); pos != 0; ){
		POSITION cur_pos = pos;
		CPartFile* cur_file = m_OtherNoNeeded_list.GetNext(pos);
		if(cur_file == pDeletedFile){
			m_OtherNoNeeded_list.RemoveAt(cur_pos);
			AddDebugLogLine(false, _T("CleanUp() reports an error with m_OtherNoNeeded_list"));
		}
	}

	// Maella -Unnecessary Protocol Overload-
	PartStatusMap::iterator it = m_partStatusMap.find(pDeletedFile); 
	if(it != m_partStatusMap.end()){
		m_partStatusMap.erase(it);
	}
	// Maella end

}
// Maella end

// Maella -Unnecessary Protocol Overload-
void CUpDownClient::TrigNextSafeAskForDownload(CPartFile* pFile){
	// Check when the specified file has been asked for the last time and
	// define when the file can be asked again without risking to be banished.
	if(pFile != NULL){
		PartStatusMap::const_iterator it = m_partStatusMap.find(pFile);
		if(it != m_partStatusMap.end()){
			// Compute then the next AskForDownload() might be 
			// performed without risk of Ban() (=> 11 minutes)
			if(it->second.dwStartUploadReqTime == 0){
				// File has never been asked before
				m_dwNextTCPAskedTime = 0; // Safe immediate reask
			}
			else {
				m_dwNextTCPAskedTime = it->second.dwStartUploadReqTime + MIN_REQUESTTIME + 60000;
			}
		}
		else {
			// File has never been asked before
			m_dwNextTCPAskedTime = 0; // Safe immediate reask
		}

		/*
		if(m_dwNextTCPAskedTime != 0 && m_dwNextTCPAskedTime > GetTickCount()){
			// Maella -Filter verbose messages-
			if(theApp.glob_prefs->GetBlockMaellaSpecificDebugMsg() == false){
				AddDebugLogLine(false, _T("Reask '%s' for '%s' delayed to %u seconds"),
					GetUserName(),
					pFile->GetFileName(), 
					(m_dwNextTCPAskedTime - GetTickCount()) / 1000);
			}
			// Maella end
		}*/
	}
}
// Maella end
