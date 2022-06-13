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
//#include "Collection.h" Anis -> non serve +

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//	members of CUpDownClient
//	which are mainly used for uploading functions 
//	Anis -> ditemi voi se c'è qualche scarsezza da sistemare nel codice.


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

	CKnownFile* currequpfile = CheckAndGetReqUpFile();
	EMFileSize filesize;
	if (currequpfile)
		filesize = currequpfile->GetFileSize();
	else
		filesize = (uint64)(PARTSIZE * (uint64)m_nUpPartCount);

    if(filesize > (uint64)0) {
	    s_UpStatusBar.SetFileSize(filesize); 
	    s_UpStatusBar.SetHeight(rect->bottom - rect->top); 
	    s_UpStatusBar.SetWidth(rect->right - rect->left); 
	    s_UpStatusBar.Fill(crNeither); 
	    if (!onlygreyrect && m_abyUpPartStatus) { 
		    for (UINT i = 0; i < m_nUpPartCount; i++)
			    if (m_abyUpPartStatus[i])
				    s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i), PARTSIZE*(uint64)(i+1), crBoth);

				else if (m_abyUpPartStatusHidden)
				{
					if(m_abyUpPartStatusHidden[i] == 1) //hidden via TCP
						s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1), RGB(0, 192, 192));
					else if(m_abyUpPartStatusHidden[i] == 2) //hidden via UDP
						s_UpStatusBar.FillRange(PARTSIZE*(uint64)(i),PARTSIZE*(uint64)(i+1), RGB(0, 100, 100));
				}
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

void CUpDownClient::SetUploadState(EUploadState eNewState)
{
	if (eNewState != m_nUploadState)
	{
		if (m_readblockthread) {
			m_readblockthread->StopReadBlock();
			m_readblockthread = NULL;
		}
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
float CUpDownClient::GetCombinedFilePrioAndCredit() 
{
	if (credits == 0)
	{
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		return 0.0F;
	}

    return 10.0f*credits->GetScoreRatio(this)*float(GetFilePrioAsNumber()); // Anis Crediti AdunanzA
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
	switch(currequpfile->GetUpPriority()){
		case PR_VERYHIGH:
			// Mod Adu
			// Emanem
			// La priorita' Release ora e' stata portata da 18 a 36
		    // lupz
			// 36 -> 1000
			filepriority = 1000;
			break;
		case PR_HIGH: 
			// lupz
			// 9 -> 70
			filepriority = 70; 
			break; 
			// fine mod Adu
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
**/
uint32 CUpDownClient::GetScore(bool sysvalue, bool isdownloading, bool onlybasevalue) const
{
	if (!m_pszUsername)
		return 0;

	if (credits == 0)
	{
		ASSERT ( IsKindOf(RUNTIME_CLASS(CUrlClient)) );
		return 0;
	}

	CKnownFile* currequpfile = CheckAndGetReqUpFile();
	if(!currequpfile)
		return 0;
	
	// bad clients (see note in function)
	if (credits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
		return 0;
	// friend slot
	if (IsFriend() && GetFriendSlot() && !HasLowID())
		return 0x0FFFFFFF;

	if (IsBanned() || m_bGPLEvildoer)
		return 0;

	if (sysvalue && HasLowID() && !(socket && socket->IsConnected()))
	{
		return 0;
	}

    int filepriority = GetFilePrioAsNumber();

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
		fBaseValue += (float)(::GetTickCount() - m_dwUploadTime > 900000)? 900000:1800000;
		fBaseValue /= 1000;
	}
	if(thePrefs.UseCreditSystem())
	{
		float modif = credits->GetScoreRatio(this); //Anis -> Crediti AdunanzA
		fBaseValue *= modif;
	}
	if (!onlybasevalue)
		fBaseValue *= (float(filepriority)/10.0f);

	if( (IsEmuleClient() || this->GetClientSoft() < 10) && m_byEmuleVersion <= 0x19 )
		fBaseValue *= 0.5f;
	return (uint32)fBaseValue;
}

struct CSyncHelper {
	CSyncHelper() { m_pObject = NULL; }
	~CSyncHelper() { if (m_pObject) m_pObject->Unlock(); }
	CSyncObject* m_pObject;
};

// VQB: fullChunk

void CUpDownClient::ResetRemainingUp()
{
	// Metodo di Anis Hireche
	CKnownFile* srcfile = theApp.sharedfiles->GetFileByID(GetUploadFileID());
	if (!srcfile)
	{
		m_nRemainingUp = MIN_BLOCK_SIZE;
		return;
	}

	if ((theApp.uploadqueue->waitinglist.IsEmpty() && IsAduClient()) || GetFriendSlot())
		m_nRemainingUp = (UINT)max(MIN_BLOCK_SIZE, min((uint64)_UI32_MAX, srcfile->GetFileSize())); //Anis -> tutto il file
	else if(IsAduClient())
		m_nRemainingUp = (UINT)(max(MIN_BLOCK_SIZE, min(PARTSIZE, srcfile->GetFileSize())) * 2); //Anis -> Doppio Chunk per Aduners
	else
		m_nRemainingUp = (UINT)max(MIN_BLOCK_SIZE, min(PARTSIZE, srcfile->GetFileSize())); //Anis -> Solo un chunk ne di + ne di -
}

void CUpDownClient::UpdateRemainingUp()
{
	UINT nRemainingUpOld = m_nRemainingUp;
	ResetRemainingUp(); //resets m_nRemainingUp to the proper value

	if(m_nRemainingUp > nRemainingUpOld)
		m_nRemainingUp = m_nRemainingUp - nRemainingUpOld;
	else 
		m_nRemainingUp = MIN_BLOCK_SIZE; //try to get rid of that client now
}
// VQB: fullChunk

//>>> Anis::PowerShare
bool CUpDownClient::IsPowerShared() const
{
	const CKnownFile* currequpfile = theApp.sharedfiles->GetFileByID(requpfileid);
	return currequpfile && currequpfile->IsPowerShared();
}
//<<< Anis::PowerShare

bool CUpDownClient::ProcessExtendedInfo(CSafeMemFile* data, CKnownFile* tempreqfile, bool isUDP)
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
	bool shouldbechecked=isUDP && tempreqfile->IsPartFile() && (((CPartFile*)tempreqfile)->GetStatus()==PS_EMPTY || ((CPartFile*)tempreqfile)->GetStatus()==PS_READY) && !(GetDownloadState()==DS_ONQUEUE && reqfile==tempreqfile) && (droptime + HR2MS(3) < ::GetTickCount());
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
				if (shouldbechecked && bPartsNeeded==false && m_abyUpPartStatus[done] && !((CPartFile*)tempreqfile)->IsComplete((uint64)done*PARTSIZE,((uint64)(done+1)*PARTSIZE)-1,false))
					bPartsNeeded = true;

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
	if (GetExtendedRequestsVersion() > 1) {
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
	//Anis -> miglioramento della ricerca fonti. Codice adattato per Adu.
	//Xman better passive source finding
	//problem is: if a client just began to download a file, we receive an FNF
	//later, if it has some chunks we don't find it via passive source finding because 
	//that works only on TCP-reask but not via UDP
	if(bPartsNeeded )
	{
		//the client was a NNS but isn't any more
		if(GetDownloadState() != DS_ONQUEUE)
		{
			//the client maybe isn't in our downloadqueue.. let's look if we should add the client
			if((credits && credits->GetMioScoreAdu(GetIP())>=1.8f && ((CPartFile*)tempreqfile)->GetSourceCount() < ((CPartFile*)tempreqfile)->GetMaxSources()) || ((CPartFile*)tempreqfile)->GetSourceCount() < ((CPartFile*)tempreqfile)->GetMaxSources()*0.8f + 1)
			{
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
	if (togo > 10240) 
		nPacketSize = togo/(uint32)(togo/10240);
	else
		nPacketSize = togo;
	while (togo)
	{
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

void CUpDownClient::CreatePackedPackets(byte* data, uint32 togo, Requested_Block_Struct* currentblock, bool bFromPF)
{
	BYTE* output = new BYTE[togo+300];
	uLongf newsize = togo+300;
	UINT result = compress2(output, &newsize, data, togo, 9);
	if (result != Z_OK || togo <= newsize){
		delete[] output;
		CreateStandartPackets(data,togo,currentblock,bFromPF);
		return;
	}
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
	CKnownFile* oldreqfile = requpfile;
	if (!theApp.downloadqueue->IsPartFile(requpfile))
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
	if (m_abyUpPartStatus) { //Fafner: missing? - 080325
		delete[] m_abyUpPartStatus;
		m_abyUpPartStatus = NULL;
	}
	m_nUpPartCount = 0;
	m_nUpCompleteSourcesCount= 0;

	if (newreqfile)
	{
		newreqfile->AddUploadingClient(this);
		md4cpy(requpfileid, newreqfile->GetFileHash());
	}
	else
		md4clr(requpfileid);

	//MORPH START - Added by SiRoB, Optimization requpfile
	requpfile = newreqfile;
	requpfileid_lasttimeupdated = theApp.sharedfiles->GetLastTimeFileMapUpdated();
	//MORPH END   - Added by SiRoB, Optimization requpfile

	if (oldreqfile) {
		oldreqfile->RemoveUploadingClient(this);
		ClearUploadBlockRequests(); //MORPH - Added by SiRoB, Fix Filtered Block Request
	}
}

//>>> Anis::Intelligent SOTN
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
//<<< Anis::Intelligent SOTN

bool CUpDownClient::AddReqBlock(Requested_Block_Struct* reqblock, const bool bChecksNecessary)  //>>> Anis::Optimization
{
//>>> Anis::Optimization
	if(bChecksNecessary)
	{
//<<< Anis::Optimization
		if(GetUploadState() != US_UPLOADING) 
		{
			if(thePrefs.GetLogUlDlEvents())
				AddDebugLogLine(DLP_LOW, false, _T("UploadClient: Client tried to add req block when not in upload slot! Prevented req blocks from being added. %s"), DbgGetClientInfo());
			delete reqblock;
			return false; //>>> Anis::Optimization
		}

//>>> Anis::Optimization
		CKnownFile* pDownloadingFile = theApp.sharedfiles->GetFileByID(reqblock->FileID);
		if (pDownloadingFile == NULL)
		{
			theApp.QueueDebugLogLineEx(LOG_WARNING, L"Client %s tried to add a blockrequest for a non-existing file!", DbgGetClientInfo());
			delete reqblock;
			return false;
		}
		if(pDownloadingFile && pDownloadingFile->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE) {
				if(md4cmp(reqblock->FileID, requpfileid) != 0)
				{
					const bool bigNew = pDownloadingFile->GetFileSize() >= (uint64)MAXPRIORITYCOLL_SIZE;
					theApp.QueueDebugLogLineEx(bigNew ? LOG_ERROR : LOG_WARNING, L"Client %s tried to add a blockrequest for a changed small file - request was %s", DbgGetClientInfo(), bigNew ? L"blocked" : L"granted");
					if(bigNew)
					{
						delete reqblock;
						return false; //>>> Anis::Optimization
					}
				}
		}
	} 

    for (POSITION pos = m_DoneBlocks_list.GetHeadPosition(); pos != 0; )
	{
        const Requested_Block_Struct* cur_reqblock = m_DoneBlocks_list.GetNext(pos);
		if(!md4cmp(reqblock->FileID, cur_reqblock->FileID)) {
			if (reqblock->StartOffset >= cur_reqblock->StartOffset && reqblock->EndOffset <= cur_reqblock->EndOffset){
			    delete reqblock;
   				return true; //>>> Anis::Optimization - already added... no error
			}
        }
    }
    for (POSITION pos = m_BlockRequests_queue.GetHeadPosition(); pos != 0; )
	{
        const Requested_Block_Struct* cur_reqblock = m_BlockRequests_queue.GetNext(pos);
		if(!md4cmp(reqblock->FileID, cur_reqblock->FileID))
		{
			if (reqblock->StartOffset >= cur_reqblock->StartOffset && reqblock->EndOffset <= cur_reqblock->EndOffset)
			{
				delete reqblock;
				return true; //>>> Anis::Optimization - already added... no error
			}
		}
	}

    m_BlockRequests_queue.AddTail(reqblock);
	return true; //>>> Anis::Optimization - alrighty :)
}

uint32 CUpDownClient::SendBlockData()
{
    DWORD curTick = ::GetTickCount();

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

			if(thePrefs.GetVerbose() && (sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket > 0))
                AddDebugLogLine(false, _T("Sent file data via normal socket when in PC mode. Bytes: %I64i."), sentBytesCompleteFileNormalSocket + sentBytesPartFileNormalSocket);
        }
		else
			uUpStatsPort = GetUserPort();

	    // Extended statistics information based on which client software and which port we sent this data to...
	    // This also updates the grand total for sent bytes, etc.  And where this data came from.
        sentBytesCompleteFile = s->GetSentBytesCompleteFileSinceLastCallAndReset();
        sentBytesPartFile = s->GetSentBytesPartFileSinceLastCallAndReset();
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, false, true, (UINT)sentBytesCompleteFile, (IsFriend() && GetFriendSlot()));
		thePrefs.Add2SessionTransferData(GetClientSoft(), uUpStatsPort, true, true, (UINT)sentBytesPartFile, (IsFriend() && GetFriendSlot()));

		m_nTransferredUp = (UINT)(m_nTransferredUp + sentBytesCompleteFile + sentBytesPartFile);
        credits->AddUploaded((UINT)(sentBytesCompleteFile + sentBytesPartFile), GetIP());

		// mod Adu
		// Emanem
		// Aggiungo i dati trasferiti all'elenco AdunanzA
		if (IsAduClient())
			theStats.stat_Adu_sessionSentBytes += sentBytesCompleteFile + sentBytesPartFile;
		// Fine mod Adu

        sentBytesPayload = s->GetSentPayloadSinceLastCallAndReset();
        m_nCurQueueSessionPayloadUp = (UINT)(m_nCurQueueSessionPayloadUp + sentBytesPayload);

        if (theApp.uploadqueue->CheckForTimeOver(this)) 
		{
            theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Completed transfer"), true);
			SendOutOfPartReqsAndAddToWaitingQueue();
        } 
		else
            CreateNextBlockPackage();
    }

    if(sentBytesCompleteFile + sentBytesPartFile > 0 || m_AvarageUDR_list.GetCount() == 0 || (curTick - m_AvarageUDR_list.GetTail().timestamp) > 1*1000) 
	{
        // Store how much data we've transferred this round,
        // to be able to calculate average speed later
        // keep sum of all values in list up to date
        TransferredData newitem = {(UINT)(sentBytesCompleteFile + sentBytesPartFile), curTick};
        m_AvarageUDR_list.AddTail(newitem);
        m_nSumForAvgUpDataRate = (UINT)(m_nSumForAvgUpDataRate + sentBytesCompleteFile + sentBytesPartFile);
    }

    // remove to old values in list
    while (m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 10*1000)
        // keep sum of all values in list up to date
        m_nSumForAvgUpDataRate -= m_AvarageUDR_list.RemoveHead().datalen;

    // Calculate average speed for this slot
    if(m_AvarageUDR_list.GetCount() > 0 && (curTick - m_AvarageUDR_list.GetHead().timestamp) > 0 && GetUpStartTimeDelay() > 2*1000)
        m_nUpDatarate = (UINT)(((ULONGLONG)m_nSumForAvgUpDataRate*1000) / (curTick - m_AvarageUDR_list.GetHead().timestamp));
	else
        // not enough values to calculate trustworthy speed. Use -1 to tell this
        m_nUpDatarate = 0; //-1;

    // Check if it's time to update the display.
    if (curTick-m_lastRefreshedULDisplay > MINWAIT_BEFORE_ULDISPLAY_WINDOWUPDATE+(uint32)(rand()*800/RAND_MAX)) {
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
	if (thePrefs.GetDebugClientTCPLevel() > 0)
		DebugSend("OP__OutOfPartReqs", this);
	Packet* pPacket = new Packet(OP_OUTOFPARTREQS, 0);
	theStats.AddUpDataOverheadFileRequest(pPacket->size);
	SendPacket(pPacket, true);
	m_fSentOutOfPartReqs = 1;
    theApp.uploadqueue->AddClientToQueue(this, true);
}

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

void CUpDownClient::ClearUploadBlockRequests()
{
	FlushSendBlocks();

	for (POSITION pos = m_BlockRequests_queue.GetHeadPosition();pos != 0;)
		delete m_BlockRequests_queue.GetNext(pos);
	m_BlockRequests_queue.RemoveAll();
	
	for (POSITION pos = m_DoneBlocks_list.GetHeadPosition();pos != 0;)
		delete m_DoneBlocks_list.GetNext(pos);
	m_DoneBlocks_list.RemoveAll();

	if (m_abyfiledata != (byte*)-1 && m_abyfiledata != (byte*)-2 && m_abyfiledata != NULL) {
		delete[] m_abyfiledata;
		m_abyfiledata = NULL;
	}
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
			// Kaiser -- 10/04/2004 15.09
			// Sistema anti ban: condizione sufficiente per non esser bannato è che si stia usando una
			// mod adu 3!
			if (::GetTickCount() - cur_struct->lastasked < MIN_REQUESTTIME && !GetFriendSlot() && !IsAduClient()){ 
				if (GetDownloadState() != DS_DOWNLOADING)
					cur_struct->badrequests++;
				if (cur_struct->badrequests == BADCLIENTBAN)
					Ban();
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
	theApp.clientlist->AddBannedClient(GetIP());
	SetUploadState(US_BANNED);
	theApp.emuledlg->transferwnd->ShowQueueCount();
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

//>>> Anis::Small File Slot
bool CUpDownClient::HasSmallFileUploadSlot() const
{
	if(GetUploadFileID() == NULL)
		return false;
	CKnownFile* file = theApp.sharedfiles->GetFileByID(GetUploadFileID());
	return file && file->GetFileSize() < (uint64)MAXPRIORITYCOLL_SIZE;
}
//<<< Anis::Small File Slot

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
	
	//InitThreadLocale();
	// Anis: SafeHash
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// Anis: SafeHash

	CFile file;
	byte* filedata = NULL;
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
			if (uint32 done = file.Read(filedata,togo) != togo){
				file.SeekToBegin();
				file.Read(filedata + done,togo-done);
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
				theApp.QueueDebugLogLine(false,GetResString(IDS_ERR_CLIENTERRORED), m_clientname, error); //Fafner: avoid possible crash - 080421
			if (theApp.emuledlg && theApp.emuledlg->IsRunning())
				PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE,(WPARAM)-1,(LPARAM)m_client);
			else if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL) {
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
				theApp.QueueDebugLogLine(false,_T("Failed to create upload package for %s - %s"), m_clientname, szError); //Fafner: avoid possible crash - 080421
			if (theApp.emuledlg && theApp.emuledlg->IsRunning())
				PostMessage(theApp.emuledlg->m_hWnd,TM_READBLOCKFROMFILEDONE,(WPARAM)-1,(LPARAM)m_client);
			else if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL) {
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
			else if (filedata != (byte*)-1 && filedata != (byte*)-2 && filedata != NULL) {
				delete[] filedata;
				filedata = NULL;
			}
			return 3;
		}
		
		pauseEvent.Lock();
	}
	return 0;
}

CKnownFile* CUpDownClient::CheckAndGetReqUpFile() const { //Anis

	if (requpfileid_lasttimeupdated < theApp.sharedfiles->GetLastTimeFileMapUpdated()) {
		return theApp.sharedfiles->GetFileByID(requpfileid);
	}
	return requpfile;
}

void CUpDownClient::CreateNextBlockPackage()
{
    // See if we can do an early return. There may be no new blocks to load from disk and add to buffer, or buffer may be large enough allready.
    if(m_BlockRequests_queue.IsEmpty() || m_abyfiledata == (byte*)-2 || m_abyfiledata != (byte*)-1 && m_addedPayloadQueueSession > GetQueueSessionPayloadUp() && m_addedPayloadQueueSession-GetQueueSessionPayloadUp() > max(GetDatarate()<<1, 50*1024)) 
	// the buffered data is large enough already according to client datarate
		return;
	CString fullname;
	bool bFromPF = true; // Statistic to breakdown uploaded data by complete file vs. partfile.
	try
	{
	// Buffer new data if current buffer is less than 100 KBytes
        while (!m_BlockRequests_queue.IsEmpty() && m_abyfiledata != (byte*)-2) 
		{
			if (m_abyfiledata == (byte*)-1) 
			{
				//An error occured //Fafner: note: this error is common during file copying - 080421
				m_readblockthread = NULL; //thread already terminated at this point
				m_abyfiledata = NULL;
				if (CheckAndGetReqUpFile() && CheckAndGetReqUpFile()->IsPartFile() && ((CPartFile*)CheckAndGetReqUpFile())->GetFileOp() == PFOP_COPYING)
					; //do nothing, no real error here since the partfile is just copying and hence locked
				else
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
			if (filedata_ReadFromDisk == NULL || m_BlockRequests_queue.GetCount()>1) {
				CKnownFile* srcFile = theApp.sharedfiles->GetFileByID(currentBlock->FileID);
				if (!srcFile)
					throw GetResString(IDS_ERR_REQ_FNF);
				uint64 i64uTogo;
				if (currentBlock->StartOffset > currentBlock->EndOffset)
				{
					i64uTogo = currentBlock->EndOffset + (srcFile->GetFileSize() - currentBlock->StartOffset);
				}
				else
				{
					i64uTogo = currentBlock->EndOffset - currentBlock->StartOffset;
					if (srcFile->IsPartFile() && !((CPartFile*)srcFile)->IsRangeShareable(currentBlock->StartOffset,currentBlock->EndOffset-1))	// Anis: SafeHash - final safety precaution
					//MORPH END  - Changed by SiRoB, Anis: SafeHash
					{
						CString error;
						error.Format(_T("%s: %I64u = %I64u - %I64u "), GetResString(IDS_ERR_INCOMPLETEBLOCK), i64uTogo, currentBlock->EndOffset, currentBlock->StartOffset);
						throw error;
					}
					//MORPH START - Added by SiRoB, Anti HideOS & SOTN :p 
					if (m_abyUpPartStatus) 
					{
						for (UINT i = (UINT)(currentBlock->StartOffset/PARTSIZE); i < (UINT)((currentBlock->EndOffset-1)/PARTSIZE+1); i++)
						//if (m_abyUpPartStatus[i]>SC_AVAILABLE)
						if (m_abyUpPartStatus[i]&2 || m_abyUpPartStatus[i]&4) //Fafner: mark transferred parts (here: take care of) - 080325
							{
								CString error;
									error.Format(_T("%s: Part %u, %I64u = %I64u - %I64u "), _T("Asked for hidden block"), i, i64uTogo, currentBlock->EndOffset, currentBlock->StartOffset);
								throw error;
							}
					} 
					else 
					{
						CString	error;
						error.Format(_T("%s: Part %u, %I64u = %I64u - %I64u "), _T("Asked Block without telling us what he need"), (UINT)(currentBlock->StartOffset/PARTSIZE), i64uTogo, currentBlock->EndOffset, currentBlock->StartOffset);
						throw error;
					}
					//MORPH END   - Added by SiRoB, Anti HideOS & SOTN :p 
				}

				if( i64uTogo > EMBLOCKSIZE*3 )
					throw GetResString(IDS_ERR_LARGEREQBLOCK);
				uint32 togo = (uint32)i64uTogo;

				if (togo > m_nRemainingUp)	// Anis - Transfer Session Clipping
					togo = m_nRemainingUp;
			
				CSyncObject* lockhandle = NULL;  
				CString fullname;
				if (srcFile->IsPartFile() && ((CPartFile*)srcFile)->GetStatus() != PS_COMPLETE)
				{
					// Do not access a part file, if it is currently moved into the incoming directory.
					// Because the moving of part file into the incoming directory may take a noticable 
					// amount of time, we can not wait for 'm_FileCompleteMutex' and block the main thread.
					lockhandle = &((CPartFile*)srcFile)->m_FileCompleteMutex;
					// If it's a part file which we are uploading the file remains locked until we've read the
					// current block. This way the file completion thread can not (try to) "move" the file into
					// the incoming directory.
					fullname = RemoveFileExtension(((CPartFile*)srcFile)->GetFullName());
				}
				else
				{
					fullname.Format(_T("%s\\%s"),srcFile->GetPath(),srcFile->GetFileName());
				}

				if (m_readblockthread == NULL) 
				{
					m_readblockthread = (CReadBlockFromFileThread*) AfxBeginThread(RUNTIME_CLASS(CReadBlockFromFileThread), THREAD_PRIORITY_NORMAL,0, CREATE_SUSPENDED);
					m_readblockthread->SetReadBlockFromFile(fullname, currentBlock->StartOffset, togo, this, lockhandle);
					m_readblockthread->ResumeThread();
				} 
				else 
				{
					m_readblockthread->SetReadBlockFromFile(fullname, currentBlock->StartOffset, togo, this, lockhandle);
				}
				SetUploadFileID(srcFile); //MORPH - Moved by SiRoB, Fix Filtered Block Request
				m_abyfiledata = (byte*)-2;
				m_utogo = togo;
			}
			if (filedata_ReadFromDisk) 
			{
				if (!srcfile_ReadFromDisk->IsPartFile())
					bFromPF = false; // This is not a part file...

				// check extension to decide whether to compress or not
				CString ext = srcfile_ReadFromDisk->GetFileName();
				ext.MakeLower();
				int pos = ext.ReverseFind(_T('.'));
				if (pos>-1)
					ext = ext.Mid(pos);
				bool compFlag = GetDatarate()<EMBLOCKSIZE && (ext!=_T(".zip") && ext!=_T(".cbz") && ext!=_T(".rar") && ext!=_T(".ace") && ext!=_T(".ogm") && ext!=_T(".tar"));//no need to try compressing tar compressed files... [Yun.SF3]
				if (ext==_T(".avi") && thePrefs.GetDontCompressAvi())
					compFlag=false;

				if (!IsUploadingToPeerCache() && m_byDataCompVer == 1 && compFlag)
					CreatePackedPackets(filedata_ReadFromDisk,togo_ReadFromDisk,currentblock_ReadFromDisk,bFromPF);
				else
					CreateStandartPackets(filedata_ReadFromDisk,togo_ReadFromDisk,currentblock_ReadFromDisk,bFromPF);
				// <-----khaos-
			
				srcfile_ReadFromDisk->statistic.AddTransferred(togo_ReadFromDisk);

				if (togo_ReadFromDisk < m_nRemainingUp)
					m_nRemainingUp -= togo_ReadFromDisk;
				else
					m_nRemainingUp = 0;


				m_addedPayloadQueueSession += togo_ReadFromDisk;

				m_DoneBlocks_list.AddHead(m_BlockRequests_queue.RemoveHead());
				delete[] filedata_ReadFromDisk;
			}
			//now process error from nextblock to read
		}
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose())
			DebugLogWarning(GetResString(IDS_ERR_CLIENTERRORED), GetUserName(), error);
		theApp.uploadqueue->RemoveFromUploadQueue(this, _T("Client error: ") + error);
		if (m_abyfiledata != (byte*)-2 && m_abyfiledata != (byte*)-1 && m_abyfiledata != NULL) {
			delete[] m_abyfiledata;
			m_abyfiledata = NULL;
		}
		return;
	}
	catch(CFileException* e)
	{
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("Failed to create upload package for %s - %s"), GetUserName(), szError);
		theApp.uploadqueue->RemoveFromUploadQueue(this, ((CString)_T("Failed to create upload package.")) + szError);
		if (m_abyfiledata != (byte*)-2 && m_abyfiledata != (byte*)-1 && m_abyfiledata != NULL) {
			delete[] m_abyfiledata;
			m_abyfiledata = NULL;
		}
		e->Delete();
		return;
	}
}