//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "zlib/zlib.h"
#include "updownclient.h"
#include "PartFile.h"
#include "server.h"
#include "opcodes.h"
#include "packets.h"
#include "emule.h"
#include "otherfunctions.h"
#include "IPFilter.h"
#include "SafeFile.h"
#include <share.h>
#include <io.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

struct Pending_Block_Struct
{
	Requested_Block_Struct	*pReqBlock;		// requested block
	z_stream				*pZStream;		// zlib context
	BYTE					*pbyteRxDate;	// buffer with buffered rx data
	uint32					dwReceived;		// bytes received for requested block
	int						iZStreamErr;
};

//	members of CUpDownClient
//	which are mainly used for downloading functions

void CUpDownClient::DrawStatusBar(CDC* dc, RECT* rect, bool bOnlyGreyRect, bool bFlat)
{
	EMULE_TRY

	const COLORREF crPending = RGB(222, 160, 0);
	const COLORREF crToDo = RGB(255, 208, 0);
	const COLORREF crNextPending = RGB(255, 255, 100);
	const COLORREF crMeOnly = RGB(190, 190, 255);
	COLORREF crBoth, crNeither, crClientOnly;

	if (bFlat)
	{
		crBoth = RGB(0, 150, 0);
		crNeither = RGB(224, 224, 224);
		crClientOnly = RGB(0, 0, 0);
	}
	else
	{
		crBoth = RGB(0, 192, 0);
		crNeither = RGB(240, 240, 240);
		crClientOnly = RGB(95, 95, 95);
	}

	uint64		qwFileSz = (m_pReqPartFile != NULL) ? m_pReqPartFile->GetFileSize() : 1ui64;
	CBarShader	statusBar(rect->bottom - rect->top, rect->right - rect->left, crNeither, qwFileSz);

	if (!bOnlyGreyRect && (m_pReqPartFile != NULL) && m_pbytePartStatuses)
	{
		_Bvector gettingParts;
		bool isDLing = (m_eDownloadState == DS_DOWNLOADING);

		if (isDLing)
			ShowDownloadingParts(gettingParts);

		uint64 qwStart, qwEnd;

		for (uint32 i = 0; i < m_uPartCount; i++)
		{
			bool bIsComplete = m_pReqPartFile->IsPartComplete(i, &qwStart, &qwEnd);

			qwEnd++;

		//	They have this part. Do I have it?
			if (m_pbytePartStatuses[i])
			{ 
				if (bIsComplete)
					statusBar.FillRange(qwStart, qwEnd, crBoth);
				else if (isDLing && (m_qwLastBlockOffset < qwEnd) && (m_qwLastBlockOffset >= qwStart))
				{
					statusBar.FillRange(qwStart, m_qwLastBlockOffset, crPending);
					statusBar.FillRange(m_qwLastBlockOffset, qwEnd, crToDo);
				}
				else if (isDLing && gettingParts[i])
					statusBar.FillRange(qwStart, qwEnd, crNextPending);
				else
					statusBar.FillRange(qwStart, qwEnd, crClientOnly);
			}
		//	They don't have a part. Do I have it?
			else if (bIsComplete)
				statusBar.FillRange(qwStart, qwEnd, crMeOnly);
		}
	}

	statusBar.Draw(dc, rect->left, rect->top, bFlat);

	EMULE_CATCH
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CUpDownClient::Compare(CUpDownClient* pClient) const
{
	uint32 dwResult = 0;

	EMULE_TRY

	if (GetIP() != 0 && pClient->GetIP() != 0)
	{
		if (GetIP() == pClient->GetIP() && GetUserPort() == pClient->GetUserPort())
			dwResult |= CLIENT_COMPARE_SAME_IP;
	}
	else if ((GetUserIDHybrid() != 0) && (GetUserIDHybrid() == pClient->GetUserIDHybrid()))
	{
		if (HasLowID())
		{
			if ( (GetServerIP() != 0) && (GetServerIP() == pClient->GetServerIP()) &&
				(GetServerPort() != 0) && (GetServerPort() == pClient->GetServerPort()) )
			{
				dwResult |= CLIENT_COMPARE_SAME_ID;
			}
		}
		else if (GetUserPort() == pClient->GetUserPort())
			dwResult |= CLIENT_COMPARE_SAME_ID;
	}

	if ((md4cmp(GetUserHash(), pClient->GetUserHash()) == 0) && HasValidHash())
		dwResult |= CLIENT_COMPARE_SAME_HASH;

	EMULE_CATCH

	return dwResult;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
// Return TRUE if client still connected, otherwise return FALSE
bool CUpDownClient::AskForDownload()
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
//	Continue anyway when socket connection already exists
//	(even if opened connection threshold was reached)
	if ( g_App.m_pListenSocket->TooManySockets() && ( (m_pRequestSocket == NULL) ||
		!(m_pRequestSocket->IsConnected() || m_pRequestSocket->IsConnecting()) ) )
	{
		return false;
	}
#endif
	m_bUDPPending = false;

	SetLastAskedTime();
	SetDownloadState(DS_CONNECTING);

	return TryToConnect();

	EMULE_CATCH

	return false;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::IsSourceRequestAllowed()
{
	EMULE_TRY

//	Avoid source request if ...
//	... remote client does not support eMule protocol
	if (!ExtProtocolAvailable())
		return false;
//	... remote client doesn't support correct SX version
	if (!SupportsSourceExchange2() && GetSourceExchange1Version() == 0)
		return false;
//	... source exchange was disabled by the user
	if (g_App.m_pPrefs->IsDisabledXS())
		return false;

	uint32	dwSources = m_pReqPartFile->GetSourceCount();

//	... source exchange for popular file was disabled by the user
	if (g_App.m_pPrefs->DisableXSUpTo() && g_App.m_pPrefs->XSUpTo() < dwSources)
		return false;

	uint32	dwCurrentTick = ::GetTickCount();

//	... we have lot of sources and source drop is not allowed
	if (g_App.m_pPrefs->GetMaxSourcePerFileSoft() <= dwSources &&
			(m_pReqPartFile->GetLastPurgeTime() + PURGE_TIME) >= dwCurrentTick)
		return false;

	uint32	dwTickCount = dwCurrentTick + CONNECTION_LATENCY;
	uint32	dwTimePassedClient = dwTickCount - GetLastSrcAnswerTime();
	uint32	dwTimePassedFile = dwTickCount - m_pReqPartFile->GetLastAnsweredTime();
	bool	bNeverAskedBefore = GetLastSrcReqTime() == 0;

	return (
	//	source is not complete and file is rare, allow once every 10 minutes
		( !IsCompleteSource()
			&& (dwSources <= RARE_FILE * 2
				|| m_pReqPartFile->GetValidSourcesCount() <= RARE_FILE / 4 )
			&& (bNeverAskedBefore || dwTimePassedClient > SOURCECLIENTREASK) )
	//	otherwise, allow every 90 minutes, but only if we haven't
	//	asked someone else in last 10 minutes
		|| ( (bNeverAskedBefore || dwTimePassedClient > (unsigned)(SOURCECLIENTREASK * MINCOMMONPENALTY)) &&
			(dwTimePassedFile > SOURCECLIENTREASK) )
	);

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendFileRequest()
{
	EMULE_TRY

	if (m_pReqPartFile == NULL)
		return;

	AddAskedCountDown();

	CMemFile	pckStrm(512);

	pckStrm.Write(m_pReqPartFile->GetFileHash(), 16);

	if (m_fSupportsMultiPacket == 1)
	{
		byte	abyteData[8];
	
		if (m_fSupportsExtMultiPacket == 1)
		{
			POKE_QWORD(abyteData, m_pReqPartFile->GetFileSize());
			pckStrm.Write(abyteData, sizeof(uint64));
		}

		abyteData[0] = OP_REQUESTFILENAME;
		pckStrm.Write(abyteData, sizeof(byte));
		if (GetExtendedRequestsVersion() > 0)
		{
			m_pReqPartFile->WritePartStatus(&pckStrm);
			if (GetExtendedRequestsVersion() > 1)
				m_pReqPartFile->WriteCompleteSourcesCount(&pckStrm);
		}

	//	Don't request file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
	//	if the remote client answers OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
	//	know that the file is shared, we know also that the file is complete and don't need to request the file status.
		if (m_pReqPartFile->GetPartCount() > 1)
		{
			abyteData[0] = OP_SETREQFILEID;
			pckStrm.Write(abyteData, sizeof(byte));
		}

		if (IsSourceRequestAllowed())
		{
			m_pReqPartFile->SetLastAnsweredTimeTimeout();

			if (SupportsSourceExchange2())
			{
				static const byte	s_abyteSx2Req[] = { OP_REQUESTSOURCES2, SOURCEEXCHANGE2_VERSION, 0, 0 };	//last 2: Options (Reserved)

				pckStrm.Write(s_abyteSx2Req, sizeof(s_abyteSx2Req));
			}
			else
			{
				abyteData[0] = OP_REQUESTSOURCES;
				pckStrm.Write(abyteData, sizeof(byte));
			}
		}

		Packet		*pPacket = new Packet(&pckStrm, OP_EMULEPROT);

		pPacket->m_eOpcode = (m_fSupportsExtMultiPacket == 1) ? OP_MULTIPACKET_EXT : OP_MULTIPACKET;
		g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
		m_pRequestSocket->SendPacket(pPacket, true, true);
#endif
	}
	else
	{
		if (GetExtendedRequestsVersion() > 0)
		{
			m_pReqPartFile->WritePartStatus(&pckStrm);
			if (GetExtendedRequestsVersion() > 1)
				m_pReqPartFile->WriteCompleteSourcesCount(&pckStrm);
		}

		Packet		*pPacket = new Packet(&pckStrm);

		pPacket->m_eOpcode = OP_REQUESTFILENAME;
		g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
		m_pRequestSocket->SendPacket(pPacket, true);
#endif //OLD_SOCKETS_ENABLED

	//	Don't request file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
	//	if the remote client answers OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
	//	know that the file is shared, we know also that the file is complete and don't need to request the file status.
		if (m_pReqPartFile->GetPartCount() > 1)
		{
			pPacket = new Packet(OP_SETREQFILEID, 16);

			md4cpy(pPacket->m_pcBuffer, m_pReqPartFile->GetFileHash());
			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
			m_pRequestSocket->SendPacket(pPacket, true);
#endif //OLD_SOCKETS_ENABLED
		}

		if (IsSourceRequestAllowed())
		{
			char	*pcPktPos;

			m_pReqPartFile->SetLastAnsweredTimeTimeout();

			if (SupportsSourceExchange2())
			{
				pPacket = new Packet(OP_REQUESTSOURCES2, 19, OP_EMULEPROT);

				pPacket->m_pcBuffer[0] = SOURCEEXCHANGE2_VERSION;
				POKE_WORD(&pPacket->m_pcBuffer[1], 0);	// Options (Reserved)
				pcPktPos = &pPacket->m_pcBuffer[3];
			}
			else
			{
				pPacket = new Packet(OP_REQUESTSOURCES, 16, OP_EMULEPROT);
				pcPktPos = pPacket->m_pcBuffer;
			}
			md4cpy(pcPktPos, m_pReqPartFile->GetFileHash());
			g_App.m_pUploadQueue->AddUpDataOverheadSourceExchange(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
			m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Extract file hash from packet stream and find associated file object
void CUpDownClient::ProcessFileHash(CSafeMemFile &packetStream)
{
	CPartFile	*pPartFile;
	uchar		fileHash[16];

	if (m_pReqPartFile == NULL)
		throw CString(_T("no requested file stated"));

	packetStream.Read(fileHash, sizeof(fileHash));

//	Verify incoming file hash through our download list
	if ((pPartFile = g_App.m_pDownloadQueue->GetFileByID(fileHash)) == NULL)
		throw CString(_T("offered file not found in DL list, hash: ")) + HashToString(fileHash);

	if (md4cmp(fileHash, m_pReqPartFile->GetFileHash()) != 0)
	{
		CString	strErr;

		strErr.Format(_T("offered file not requested (offered: %s (%s) | requested: %s (%s))"),
			pPartFile->GetFileName(), HashToString(pPartFile->GetFileHash()),
			m_pReqPartFile->GetFileName(), HashToString(m_pReqPartFile->GetFileHash()));
		throw strErr;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ProcessFileInfo() processes the contents of a File Request Answer packet
void CUpDownClient::ProcessFileInfo(CSafeMemFile &packetStream)
{
	EMULE_TRY

	SetLastAskedTime();

	uint16	uNameLength;

	packetStream.Read(&uNameLength, sizeof(uint16));
	ReadMB2Str(m_eStrCodingFormat, &m_strClientFilename, packetStream, uNameLength);
//	Shrink class member, as allocated container of returned string can be larger than string length
	m_strClientFilename.FreeExtra();

//	Removed requesting the file status for files <= PARTSIZE for better compatibility with ed2k protocol (eDonkeyHybrid).
//	if the remote client answers OP_REQUESTFILENAME with OP_REQFILENAMEANSWER the file is shared by the remote client. if we
//	know that the file is shared, we know also that the file is complete and don't need to request the file status.

	if (m_pReqPartFile->GetPartCount() == 1)
	{
		m_uAvailPartCount = 0;	// keep it zero while doing preparation
		delete[] m_pbytePartStatuses;
		m_pbytePartStatuses = NULL;
		m_pbytePartStatuses = new byte[1];
		memset(m_pbytePartStatuses, 1, 1);
		m_uPartCount = m_uAvailPartCount = m_uNeededParts = 1;

	//	even if the file is <= PARTSIZE, we _may_ need the hashset for that file (if the file size == PARTSIZE)
		if (m_pReqPartFile->m_bHashSetNeeded)
			SendHashsetRequest();
		else if (GetDownloadState() != DS_DOWNLOADING) 
			SendStartUploadRequest();

		UpdateDownloadStateAfterFileReask();

		m_pReqPartFile->NewSrcPartsInfo();
		UpdateDisplayedInfo();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ProcessFileStatus(CSafeMemFile &packetStream, bool bUDPPacket/*=false*/)
{
	EMULE_TRY

	uint16 uED2KPartCount;

	packetStream.Read(&uED2KPartCount, 2);

	m_uAvailPartCount = 0;
	if (m_pbytePartStatuses != NULL)
	{
		delete[] m_pbytePartStatuses;
		m_pbytePartStatuses = NULL;
	}

	m_uPartCount = m_pReqPartFile->GetPartCount();
	if (uED2KPartCount == 0)
	{
		m_pbytePartStatuses = new byte[m_uPartCount];
		memset2(m_pbytePartStatuses, 1, m_uPartCount);
		m_uAvailPartCount = m_uNeededParts = m_uPartCount;
	}
	else
	{
		m_uNeededParts = 0;
		if (m_pReqPartFile->GetED2KPartCount() != uED2KPartCount)
		{
			m_uPartCount = 0;
			CString strTemp;

			strTemp.Format( _T("wrong part number, file: %s ED2KParts/Parts: %u/%u %s %s %s:%u"),
				m_pReqPartFile->GetFileName(), m_pReqPartFile->GetED2KPartCount(), uED2KPartCount,
				GetClientNameWithSoftware(), HashToString(m_userHash), m_strFullUserIP, m_uUserPort );
			throw strTemp;
		}

		m_pbytePartStatuses = new byte[m_uPartCount];

		uint32	dwNeeded = 0, dwCompleteParts = 0, dwDone = 0;
		uint32	dwPartCnt = static_cast<uint32>(m_uPartCount);
		byte	byteToRead;

		while (dwDone < dwPartCnt)
		{
			packetStream.Read(&byteToRead, 1);
			for (unsigned ui = 0; ui < 8; ui++)
			{
				m_pbytePartStatuses[dwDone] = static_cast<byte>((byteToRead >> ui) & 1);
				if (m_pbytePartStatuses[dwDone])
				{
					dwCompleteParts++;
					if (!m_pReqPartFile->IsPartComplete(dwDone))
						dwNeeded++;
				}
				if (++dwDone == dwPartCnt)
					break;
			}
		}

		m_uAvailPartCount = static_cast<uint16>(dwCompleteParts);
		m_uNeededParts = static_cast<uint16>(dwNeeded);
	}

//	if it is TCP connection & we don't download, we can send something back
	if (!bUDPPacket)
	{
		if (m_pReqPartFile->m_bHashSetNeeded)
			SendHashsetRequest();
	//	Request the file & enter to the queue only if we required a file from remote client
		else if ((GetDownloadState() != DS_DOWNLOADING) && (m_uNeededParts != 0))
			SendStartUploadRequest();
	}

	UpdateDownloadStateAfterFileReask();

	m_pReqPartFile->NewSrcPartsInfo();
	UpdateDisplayedInfo();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::UpdateDownloadStateAfterFileReask()
{
//	If our client sent the file request at the same time when remote client invited us to download,
//	then an addition check of the download state is required to prevent download abort.
	if (GetDownloadState() == DS_DOWNLOADING || m_fRequestingHashSet == 1)
		return;
	UpdateOnqueueDownloadState();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::UpdateOnqueueDownloadState()
{
//	If remote client is lowID
	if (HasLowID())
	{
	//	Check LowID properties of local server connection if it exists,
	//	otherwise temporarily put client in active state. The proper state will be set in TryToConnect()
		if (g_App.m_pServerConnect->IsConnected())
		{
			if (g_App.m_pServerConnect->IsLowID())
			{
				SetDownloadState(DS_LOWTOLOWID);
				return;
			}
			else if (g_App.m_pServerConnect->GetCurrentServer()->GetIP() != GetServerIP())
			{
				if (m_uNeededParts != 0)
					SetDownloadState(DS_LOWID_ON_OTHER_SERVER);
				else
					SetDownloadState(DS_NONEEDEDPARTS);
				return;
			}
		}
	}

	if (m_uNeededParts == 0)
		SetDownloadState(DS_NONEEDEDPARTS);
	else if (GetDownloadState() != DS_ONQUEUE)
		SetDownloadState(DS_ONQUEUE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::AddRequestForAnotherFile(CPartFile* pPartFile)
{
	EMULE_TRY

	if (pPartFile == NULL)
		return false;

//	Check Other list
	if (m_otherRequestsList.Find(pPartFile) != NULL)
		return false;
	else
		m_otherRequestsList.AddTail(pPartFile);

	pPartFile->AddClientToA4AFSourceList(this);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::RemoveRequestForAnotherFile(CPartFile* pPartFile)
{
	EMULE_TRY

	if (pPartFile == NULL)
		return;

//	Check Other list
	POSITION posPartFile = m_otherRequestsList.Find(pPartFile);

	if (posPartFile != NULL)
	{
		m_otherRequestsList.RemoveAt(posPartFile);
	}

	m_otherNoNeededMap.RemoveKey(pPartFile);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SetDownloadState(EnumDLQState byNewState)
{
	EMULE_TRY

	if (m_eDownloadState != byNewState)
	{
	//	Clear variables before switching
		switch (m_eDownloadState)
		{
			case DS_DOWNLOADING:
			{
				uint32	dwCurTick = ::GetTickCount();
				uint32	dwSessionDuration = (dwCurTick - m_dwDownStartTime) / 1000;

			//	Update connection time not to drop sources accidently (we might transfer for hours)
			//	as this time is used to remove inactive LowID sources in PartFile:Process()
				m_dwEnteredConnectedState = dwCurTick;

				g_App.m_pPrefs->Add2DownSAvgTime(dwSessionDuration);

				ClearDownloadBlocksList();
				ClearPendingBlocksList();

				if (m_bTransferredDownMini && byNewState != DS_ERROR)
					g_App.m_pPrefs->Add2DownSuccessfulSessions();
				else
				{
					g_App.m_pPrefs->Add2DownFailedSessions();
					if (m_fBlocksWereRequested == 0)
					{
						AddLogLine(LOG_FL_DBG, _T("Failed download session with the NNS client %s%s"), GetClientNameWithSoftware(), HasLowID() ? _T(" LowID") : _T(""));
						g_App.m_pPrefs->Add2DownFailedSessionsNoRequiredData();
					}
				}

				if (g_App.m_pPrefs->LogDownloadToFile())
				{
					FILE	*pLogFile = _tfsopen(g_App.m_pPrefs->GetDownloadLogName(), _T("ab"), _SH_DENYWR);

					if (pLogFile != NULL)
					{
						COleDateTime	currentTime(COleDateTime::GetCurrentTime());
						CString			strLogLine;

						strLogLine.Format( _T("%s,\"%s\",%s,%s,\"%s\",%u,%u,%u,%s,%s,%s\r\n"),
											currentTime.Format(_T("%c")),
											GetUserName(),
											HashToString(m_userHash),
											GetFullSoftVersionString(),
											(m_pReqPartFile) ? m_pReqPartFile->GetFileName() : _T(""),
											m_dwSessionDownloadedData,
											GetLastDownPartAsked(),
											dwSessionDuration,
											HasLowID() ? _T("LowID") : _T("HighID"),
											YesNoStr(IsOnLAN()),
											GetResString((m_bTransferredDownMini) ? IDS_IDENTOK : IDS_FAILED).MakeLower() );

#ifdef _UNICODE
					//	Write the Unicode BOM in the beginning if file was created
						if (_filelength(_fileno(pLogFile)) == 0)
							fputwc(0xFEFF, pLogFile);
#endif
						_fputts(strLogLine, pLogFile);
						fclose(pLogFile);
					}
				}

				m_dwDownDataRate = 0;
				m_dwClientSumDLDataRateOverLastNMeasurements = 0;
				m_dwClientSumDLTickOverLastNMeasurements = 0;
				m_averageDLTickList.clear();
				m_averageDLDataRateList.clear();

				m_uLastPartAsked = 0xFFFF;

#ifdef OLD_SOCKETS_ENABLED

				if (m_pRequestSocket && byNewState != DS_ERROR)
					m_pRequestSocket->DisableDownloadLimit(true);
#endif //OLD_SOCKETS_ENABLED
				break;
			}
		}

	//	Note: it's required to update the variable used for timeout check before we set DS_DOWNLOADING,
	//	to prevent cancel of the session if DL-state-queue is processed before block is requested
		if (byNewState == DS_DOWNLOADING)
		{
			m_dwLastDataRateCalculationTime = m_dwDownStartTime = m_dwLastBlockReceived = ::GetTickCount();
		}

	//	Switch the state & update info
		if (m_pReqPartFile != NULL)
			m_pReqPartFile->SwapClientBetweenSourceLists(this, m_eDownloadState, byNewState);
		else
			m_eDownloadState = byNewState;

	//	Prepare variables for new state
	//	note: please remember that the value of member variable must be already updated.
		switch (m_eDownloadState)
		{
			case DS_DOWNLOADING:
				m_qwLastBlockOffset = ~0ui64;
				m_bTransferredDownMini = false;
				m_fBlocksWereRequested = 0;
				m_fSentCancelTransfer = 0;

				m_dwSessionDownloadedData = 0;

				SetAskedCountDown(0);
				SetRemoteQueueRank(0, false);
				break;

			case DS_CONNECTED:
				m_dwEnteredConnectedState = ::GetTickCount();
				break;

			case DS_ERROR:
			case DS_NONE:
				ClearPartStatuses();
			//	Update counters as a client left the download queue
				if (IsBanned())
					g_App.m_pClientList->UpdateBanCounters();
				break;
		}

		SetNextFileReaskTime();

		UpdateDisplayedInfo();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Returns download queue client status based as string
CString CUpDownClient::GetDownloadStateAsString()
{
#ifndef NEW_SOCKETS_ENGINE
	static const unsigned s_auiResTbl[] =
	{
		IDS_TRANSFERRING,			//DS_DOWNLOADING
		IDS_CONNECTING,				//DS_CONNECTING
		IDS_CONNVIASERVER,			//DS_WAITCALLBACK
		IDS_ASKING,					//DS_CONNECTED
		IDS_ONQUEUE,				//DS_ONQUEUE
		IDS_NONEEDEDPARTS,			//DS_NONEEDEDPARTS
		IDS_WAITFILEREQ,			//DS_WAIT_FOR_FILE_REQUEST
		IDS_ANOTHER_SERVER_LOWID,	//DS_LOWID_ON_OTHER_SERVER
		IDS_RECHASHSET,				//DS_REQHASHSET
		IDS_NOCONNECTLOW2LOW,		//DS_LOWTOLOWID
		IDS_UNKNOWN,				//DS_LAST_QUEUED_STATE
		IDS_ERROR_STATE,			//DS_ERROR
		IDS_UNKNOWN					//DS_NONE
	};
	unsigned	uiState = GetDownloadState();

	if (uiState >= ARRSIZE(s_auiResTbl))
		uiState = DS_NONE;
	return GetResString(s_auiResTbl[uiState]);
#else
	return _T("");
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ProcessHashSet(byte *pbytePacket, uint32 size)
{
	EMULE_TRY

	if (!m_fRequestingHashSet)
		throw CString(_T("unwanted hashset received"));

	m_fRequestingHashSet = 0;

	if (m_pReqPartFile == NULL)
		throw CString(_T("wrong fileID sent, file: NULL"));

	if (md4cmp(pbytePacket, m_pReqPartFile->GetFileHash()))
	{
		m_pReqPartFile->m_bHashSetNeeded = true;
		throw CString(_T("wrong fileID sent, file: ") + m_pReqPartFile->GetFileName());
	}

	CSafeMemFile data1(pbytePacket, size);

	if (m_pReqPartFile->LoadHashsetFromFile(data1, true))
	{
		SendStartUploadRequest();
	}
	else
	{
		m_pReqPartFile->m_bHashSetNeeded = true;
		AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_ERR_BADHASHSET, m_pReqPartFile->GetFileName());
	}

	UpdateDownloadStateAfterFileReask();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendBlockRequests()
{
	EMULE_TRY

	m_dwLastBlockReceived = ::GetTickCount();

	if (m_pReqPartFile == NULL)
		return;

//	Get blocks from the same chunk first
	if (m_downloadBlocksList.IsEmpty())
	{
	//	Get part for request. If actual part still has empty blocks, the we get that part,
	//	otherwise it will be a new part.
		m_uLastPartAsked = m_pReqPartFile->GetNextRequiredPart(this);

	//	Previous active part unknown
		if (m_uLastPartAsked != 0xFFFF)
		{
			Requested_Block_Struct	*apReqBlocksToAdd[3];
			uint32	dwCount, dwSavedCount;

			dwSavedCount = 3 - m_pendingBlocksList.GetCount();
			dwCount = m_pReqPartFile->GetGapsInPart(m_uLastPartAsked, apReqBlocksToAdd, dwSavedCount);

		//	Finished block (part ?)
			if (dwCount == 0)
			{
			//	Check if part was completed
				if ( m_pReqPartFile->IsPartFull(m_uLastPartAsked)
				  || m_pReqPartFile->IsCorruptedPart(m_uLastPartAsked) )
				{
					m_uLastPartAsked = 0xFFFF;
				}

			//	Get next part
				m_uLastPartAsked = m_pReqPartFile->GetNextRequiredPart(this);

			//	More parts ?
				if (m_uLastPartAsked != 0xFFFF)
					dwCount = m_pReqPartFile->GetGapsInPart(m_uLastPartAsked, apReqBlocksToAdd, dwSavedCount);
			}

			for (uint32 i = 0; i < dwCount; i++)
				m_downloadBlocksList.AddTail(apReqBlocksToAdd[i]);
		}
	}

//	Why are unfinished blocks requested again, not just new ones?
	while (m_pendingBlocksList.GetCount() < 3 && !m_downloadBlocksList.IsEmpty())
	{
		Pending_Block_Struct		*pPendingBlock = new Pending_Block_Struct;

		pPendingBlock->pReqBlock = m_downloadBlocksList.RemoveHead();
		pPendingBlock->pZStream = NULL;
		pPendingBlock->pbyteRxDate = NULL;
		pPendingBlock->dwReceived = 0;
		pPendingBlock->iZStreamErr = 0;
		m_pendingBlocksList.AddTail(pPendingBlock);
	}

	if (m_pendingBlocksList.IsEmpty())
	{
		EnumDLQState eOldDowloadState = GetDownloadState();

		SendCancelTransfer();

	//	If we have nothing to request from a full source, then probably all blocks were
	//	already requested (file will be complete soon), thus no need to send any reasks
		if (IsCompleteSource())
		{
			m_dwLastAskedTime = ::GetTickCount();	// No need to reask in the nearest future
		//	NextFileReaskTime is calculated inside after state change
			UpdateOnqueueDownloadState();
		}
		else
		{
			if ((GetTickCount() - GetLastAskedTime()) > FILEREASKTIME)
				AskForDownload();

			if (eOldDowloadState == GetDownloadState())
				SetDownloadState(DS_NONEEDEDPARTS);
		}
		return;
	}

	Packet		*pPacket;
	POSITION	pos = m_pendingBlocksList.GetHeadPosition();
	bool		bI64Offsets = false;
	Requested_Block_Struct	*pReqBlk;

	for (uint32 i = 0; i < 3; i++)
	{
		if (pos != NULL)
		{
			pReqBlk = m_pendingBlocksList.GetNext(pos)->pReqBlock;
		//	We use larger or equal condition to avoid wrap-around block requests (End < Start)
			if (pReqBlk->qwEndOffset >= 0xFFFFFFFFui64)		//	Start is always <= End
			{
				bI64Offsets = true;
				if (SupportsLargeFiles())
					break;
				SendCancelTransfer();
				SetDownloadState(DS_ERROR);
				return;
			}
		}
	}

	pos = m_pendingBlocksList.GetHeadPosition();
	if (bI64Offsets)
	{
		pPacket = new Packet(OP_REQUESTPARTS_I64, 64, OP_EMULEPROT);	// 64 = 16+(3*8)+(3*8)

		CMemFile	packetStream(reinterpret_cast<BYTE*>(pPacket->m_pcBuffer), 64);
		uint64	aqwStartEnd[6];

		packetStream.Write(m_pReqPartFile->GetFileHash(), 16);
		for (uint32 i = 0; i < 3; i++)
		{
			if (pos != NULL)
			{
				pReqBlk = m_pendingBlocksList.GetNext(pos)->pReqBlock;
				aqwStartEnd[i] = pReqBlk->qwStartOffset;
				aqwStartEnd[i + ARRSIZE(aqwStartEnd) / 2] = pReqBlk->qwEndOffset + 1ui64;
			}
			else
			{
				aqwStartEnd[i] = 0;
				aqwStartEnd[i + ARRSIZE(aqwStartEnd) / 2] = 0;
			}
		}
		packetStream.Write(aqwStartEnd, sizeof(aqwStartEnd));
	}
	else
	{
		pPacket = new Packet(OP_REQUESTPARTS, 40);	// 40 = 16+(3*4)+(3*4)

		CMemFile	packetStream(reinterpret_cast<BYTE*>(pPacket->m_pcBuffer), 40);
		uint32	adwStartEnd[6];

		packetStream.Write(m_pReqPartFile->GetFileHash(), 16);
		for (uint32 i = 0; i < 3; i++)
		{
			if (pos != NULL)
			{
				pReqBlk = m_pendingBlocksList.GetNext(pos)->pReqBlock;
				adwStartEnd[i] = static_cast<uint32>(pReqBlk->qwStartOffset);
				adwStartEnd[i + ARRSIZE(adwStartEnd) / 2] = static_cast<uint32>(pReqBlk->qwEndOffset) + 1;
			}
			else
			{
				adwStartEnd[i] = 0;
				adwStartEnd[i + ARRSIZE(adwStartEnd) / 2] = 0;
			}
		}
		packetStream.Write(adwStartEnd, sizeof(adwStartEnd));
	}

	g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED

	m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED

	m_fBlocksWereRequested = 1;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/* Barry - Originally this only wrote to disk when a full 180k block
           had been received from a client, and only asked for data in
           180k blocks.

           This meant that on average 90k was lost for every connection
           to a client data source. That is a lot of wasted data.

           To reduce the lost data, packets are now written to a buffer
           and flushed to disk regularly regardless of size downloaded.
           This doesn't include compressed packets.

           Data is also requested only where gaps are, not in 180k blocks.
           The requests will still not exceed 180k, but may be smaller to
           fill a gap.
		Return:
			!= 0 - this client should be deleted by caller, else 0.
*/
int CUpDownClient::ProcessBlockPacket(byte *pbytePacket, uint32 dwPacketSize, bool bPacked, bool b64bOffsets)
{
	EMULE_TRY

//	Update timer (eklmn: don't remove this value cause it use to timeout check)
	m_dwLastBlockReceived = ::GetTickCount();

//	Read data from packet
	CSafeMemFile	packetStream(pbytePacket, dwPacketSize);
	uchar			fileHash[16];

	packetStream.Read(fileHash, sizeof(fileHash));				//<HASH>

//	Check that this data is for the correct file
	if (m_pReqPartFile == NULL || md4cmp(fileHash, m_pReqPartFile->GetFileHash()))
	{
		throw CString(_T("wrong fileID sent, file: ") + ((m_pReqPartFile != NULL) ? m_pReqPartFile->GetFileName() : _T("NULL")));
	}

//	Find the start & end positions, and size of this chunk of data
	uint64			qwStartPos = 0, qwEndPos = 0;
	uint32			dwBlockSize = 0, dwHdrSz = 24;

	if (b64bOffsets)
	{
		packetStream.Read(&qwStartPos, sizeof(uint64));
		dwHdrSz += 4;
	}
	else
		packetStream.Read(&qwStartPos, sizeof(uint32));				//<BYTEOFFSET:DWORD> start position
	if (bPacked)
	{
		packetStream.Read(&dwBlockSize, sizeof(uint32));		//<BYTELENGTH:DWORD> packed block size
	}
	else
	{
		if (b64bOffsets)
		{
			packetStream.Read(&qwEndPos, sizeof(uint64));
			dwHdrSz += 4;
		}
		else
			packetStream.Read(&qwEndPos, sizeof(uint32));			//<BYTEOFFSET:DWORD> end position

	//	Check that packet size matches the declared packetStream packet + header size
		if (static_cast<uint64>(dwPacketSize) != (static_cast<uint64>(dwHdrSz) + qwEndPos - qwStartPos))
			throw CString(_T("corrupted or invalid data block received"));
	}
	const uint32	dwDataTransferdInPacket = dwPacketSize - dwHdrSz;

//	Protect from 0 data size, because it spoils statistics
	if (dwDataTransferdInPacket == 0)
		return 0;

//	Extended statistics information based on which client and remote port sent this data.
//	The new function adds the bytes to the grand total as well as the given client/port.
//	bFromPF is not relevant to downloaded data. It is purely an uploads statistic.
	g_App.m_pPrefs->Add2SessionDownTransferData(GetClientSoft(), dwDataTransferdInPacket);

	m_dwTransferredDown += dwDataTransferdInPacket;
	m_dwSessionDownloadedData += dwDataTransferdInPacket;
	m_dwTransferredInLastPeriod += dwDataTransferdInPacket;

//	Increment amount of data transferred for a file
	m_pReqPartFile->AddRxAmount(dwDataTransferdInPacket);

//	Loop through to find the reserved block that this is within
	Pending_Block_Struct	*cur_block;
	Requested_Block_Struct	*pReqBlk;

	POSITION pos1, pos2;

	for (pos1 = m_pendingBlocksList.GetHeadPosition(); (pos2 = pos1) != NULL; )
	{
		cur_block = m_pendingBlocksList.GetNext(pos1);
		pReqBlk = cur_block->pReqBlock;

		if ((pReqBlk->qwStartOffset <= qwStartPos) && (pReqBlk->qwEndOffset >= qwStartPos))
		{
		//	Found reserved block

			m_pCredits->AddDownloaded(dwDataTransferdInPacket, this->GetIP());

			if (cur_block->iZStreamErr != 0)
			{
			//	Increment file corruption statistics
				m_pReqPartFile->AddRxCorruptedAmount(dwDataTransferdInPacket);
			//	All data of corrupted block was received, delete the block to allow its reception on retransmission
				if ((cur_block->dwReceived += dwDataTransferdInPacket) >= dwBlockSize)
				{
				//	zstream is already deleted at that moment
					delete pReqBlk;
					delete cur_block;
					m_pendingBlocksList.RemoveAt(pos2);
				}
				break;
			}

		//	Remember this start pos, used to draw part downloading in list
			m_qwLastBlockOffset = qwStartPos;

		//	Occasionally packets are duplicated, no point writing it twice
		//	This will be 0 in these cases, or the length written otherwise
			uint32 lenWritten = 0;

		//	Handle differently depending on whether bPacked or not
			if (!bPacked)
			{
			//	Move end back one, should be inclusive
				qwEndPos--;
				cur_block->dwReceived += dwDataTransferdInPacket;

			//	Make sure only requested range is accepted to avoid corruptions
			//	Similar protection for compressed stream is done inside unzip() through
			//	output buffer size verification which is always not more than a block size
				if ( (qwEndPos > pReqBlk->qwEndOffset) ||
				//	Remove Hybrid, as it can generate such condition in rare cases
					( g_App.m_pPrefs->IsFakeRxDataFilterEnabled() &&
					(dwDataTransferdInPacket < 64) && (GetClientSoft() != SO_EDONKEYHYBRID) &&
					(static_cast<uint32>(pReqBlk->qwEndOffset - pReqBlk->qwStartOffset) > cur_block->dwReceived) ) )
				{
				//	Increment file corruption statistics
					m_pReqPartFile->AddRxCorruptedAmount(dwDataTransferdInPacket);
				//	Filter clients sending incorrect packets
					g_App.m_pIPFilter->AddTemporaryBannedIP(GetIP());
				//	If we're logging countermeasures...
					if (!g_App.m_pPrefs->IsCMNotLog())
						AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Client %s (IP: %s) added to filtered clients due to bad data transmission"),
											GetClientNameWithSoftware(), GetFullIP() );
					return -1;	/*request deletion of this client by caller*/
				}

			//	Write to disk (will be buffered in part file class)
				lenWritten = m_pReqPartFile->WriteToBuffer( 0,
					pbytePacket + dwHdrSz, qwStartPos, qwEndPos,
					(qwEndPos >= pReqBlk->qwEndOffset) ? PF_WR2BUF_FL_ENDOFBLOCK : 0 );
#ifdef _DEBUG
				if (lenWritten == 0)
					AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Client %s"), GetClientNameWithSoftware());
#endif
			}
			else // Packed
			{
				uint32 lenUnzipped;

				cur_block->dwReceived += dwDataTransferdInPacket;

			//	Try to unzip
				int result = unzip(cur_block, pbytePacket + dwHdrSz, dwDataTransferdInPacket, &lenUnzipped);

				if (result == Z_OK)
				{
				//	Write any unzipped packetStream to disk
					if (lenUnzipped != 0)
					{
					//	Use the current start and end positions for the uncompressed packetStream
						qwStartPos = pReqBlk->qwStartOffset;
						qwEndPos = qwStartPos + static_cast<uint64>(lenUnzipped - 1);
					//	Write decompressed data to the file
						lenWritten = m_pReqPartFile->WriteToBuffer(
							lenUnzipped - cur_block->dwReceived, cur_block->pbyteRxDate,
							qwStartPos, qwEndPos, PF_WR2BUF_FL_FREEBUFFER | PF_WR2BUF_FL_ENDOFBLOCK );
					//	The buffer will be freed while flushing to the file
						cur_block->pbyteRxDate = NULL;
#ifdef _DEBUG
						if (lenWritten == 0)
							AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Client %s"), GetClientNameWithSoftware());
#endif
					//	Imitate end of block, if someone will send less data or more than one compression stream
						qwEndPos = pReqBlk->qwEndOffset;
					}
				}
				else
				{
					cur_block->iZStreamErr = result;
					AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Corrupted compressed packet for %s received (error %i)"), m_pReqPartFile->GetFileName(), result);
					m_pReqPartFile->RemoveBlockFromList(pReqBlk->qwStartOffset, pReqBlk->qwEndOffset);

				//	Release resources in case of decompression error
					if (cur_block->pZStream != NULL)
					{
						inflateEnd(cur_block->pZStream);
						delete cur_block->pZStream;
						cur_block->pZStream = NULL;
					}
					delete[] cur_block->pbyteRxDate;
					cur_block->pbyteRxDate = NULL;

				//	Increment file corruption statistics
					m_pReqPartFile->AddRxCorruptedAmount(cur_block->dwReceived);
				//	All data of corrupted block was received, delete the block to allow its reception on retransmission
					if (cur_block->dwReceived >= dwBlockSize)
					{
						delete pReqBlk;
						delete cur_block;
						m_pendingBlocksList.RemoveAt(pos2);
					}
					break;
				}
			}

		//	Check result of WriteToBuffer, if we save some bytes then
		//	this DL session can be counted as successful
			if (lenWritten > 0)
			{
			//	To determine whether the current download session was successful or not
				m_bTransferredDownMini = true;
			}

		//	We need to check "END of the block" condition every time, because if a remote client
		//	sends us data which was already written we will not delete the block & packet

			if (qwEndPos >= pReqBlk->qwEndOffset)
			{
				m_pReqPartFile->RemoveBlockFromList(pReqBlk->qwStartOffset, pReqBlk->qwEndOffset);
				delete pReqBlk;

			//	That should never happen
				if (cur_block->pZStream != NULL)
				{
					inflateEnd(cur_block->pZStream);
					delete cur_block->pZStream;
				}

				delete cur_block;
				m_pendingBlocksList.RemoveAt(pos2);

#if 0	// disable as newer versions were fixed
			//	MLdonkey messes up and sends data several times when standard request
			//	scheme with repeated block requests is in use, thus request only when
			//	there's no more pending requests to avoid transmission of useless data
				if ((GetClientSoft() != SO_MLDONKEY) || (m_pendingBlocksList.GetCount() == 0))
#endif
				SendBlockRequests();	//	Request next block
			}

		//	Stop looping and exit method
			break;
		}
	}

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CUpDownClient::unzip(Pending_Block_Struct *pPendingBlk, BYTE *pbyteZipped, uint32 dwLenZipped, uint32 *pdwLenUnzipped)
{
	int iErr = Z_MEM_ERROR;

	EMULE_TRY

	z_stream *zS = pPendingBlk->pZStream;

//	Is this the first time this block has been unzipped
	if (zS == NULL)
	{
	//	Allocate stream
		zS = pPendingBlk->pZStream = new z_stream;

	//	Initialize stream values
		zS->zalloc = (alloc_func)0;
		zS->zfree = (free_func)0;

	//	Allocate and initialize output buffer
		zS->avail_out = static_cast<unsigned>(pPendingBlk->pReqBlock->qwEndOffset - pPendingBlk->pReqBlock->qwStartOffset + 1);
		zS->next_out = pPendingBlk->pbyteRxDate = new BYTE[zS->avail_out];

	//	Initialise the z_stream
		iErr = inflateInit(zS);

		if (iErr != Z_OK)
			return iErr;
	}

//	Initialize input buffer
	zS->next_in = pbyteZipped;
	zS->avail_in = dwLenZipped;

//	Unzip the data
	iErr = inflate(zS, Z_SYNC_FLUSH);

	*pdwLenUnzipped = 0;
	if (iErr == Z_OK)
	{
		if (zS->avail_out == 0)
		{
		//	Output buffer is full, more than required data was sent or error occurred
			iErr = Z_BUF_ERROR;
		}
	}
//	Is zip finished reading all currently available input and writing all generated output
	else if (iErr == Z_STREAM_END)
	{
	//	Finish up
		inflateEnd(zS);
		pPendingBlk->pZStream = NULL;

		iErr = Z_OK;
	//	Got the good result, return unzipped amount
		*pdwLenUnzipped = zS->total_out;
		delete zS;
	}

	EMULE_CATCH

	return iErr;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CUpDownClient::DownloadProcess(uint32 dwReduceDownload /*in percent*/)
{
	EMULE_TRY

	uint32	dwCurTick = GetTickCount();

	if ((dwCurTick - m_dwLastBlockReceived) > DOWNLOADTIMEOUT)
	{
	//	Wait a little bit more for slow clients, what helps to save some downloads
		if ((m_dwDownDataRate == 0) || (m_dwDownDataRate > 1000) || ((dwCurTick - m_dwLastBlockReceived) >= 150000))
		{
			UpdateOnqueueDownloadState();
			SendCancelTransfer();
		}
	}
	else
	{
		uint32 time_diff_ms = dwCurTick - m_dwLastDataRateCalculationTime;
		uint32 dwLastTransferred = m_dwTransferredInLastPeriod;

	//	Update time & reset data transferred over last period
		m_dwTransferredInLastPeriod = 0;
		m_dwLastDataRateCalculationTime = dwCurTick;

	//	Update average sum
		m_dwClientSumDLDataRateOverLastNMeasurements += dwLastTransferred;
		m_dwClientSumDLTickOverLastNMeasurements += time_diff_ms;
	//	Update lists
		m_averageDLDataRateList.push_front(dwLastTransferred);
		m_averageDLTickList.push_front(time_diff_ms);

	//	Check lists size
		while (m_averageDLTickList.size() > (DOWNLOADTIMEOUT / 100))
		{
			m_dwClientSumDLDataRateOverLastNMeasurements -= m_averageDLDataRateList.back();
			m_averageDLDataRateList.pop_back();
			m_dwClientSumDLTickOverLastNMeasurements -= m_averageDLTickList.back();
			m_averageDLTickList.pop_back();
		}

		uint32 dwAverageTransferTimePeriod;

		if (m_dwClientSumDLTickOverLastNMeasurements != 0)
		{
		//	Calculate average time period between measuremets (ms)
			dwAverageTransferTimePeriod = m_dwClientSumDLTickOverLastNMeasurements / m_averageDLTickList.size();

		//	Calculate a DL rate (b/s)
			m_dwDownDataRate = static_cast<uint32>((1000 * static_cast<uint64>(m_dwClientSumDLDataRateOverLastNMeasurements)) / static_cast<uint64>(m_dwClientSumDLTickOverLastNMeasurements));
		}
		else
		{
			dwAverageTransferTimePeriod = 100;
		//	Calculate a DL rate (b/s) assuming 100 ms interval
			m_dwDownDataRate = 10 * m_dwClientSumDLDataRateOverLastNMeasurements;
		}

	//	now based on measured speed we can control the bandwidth
		if (dwReduceDownload)
		{
			m_bLimitlessDL = false;
		//	the limit for next period is calculated as required DL per millisecond multiplicated by the time period.
		//		required DL (Byte/s) = percent * measured DL (Byte/s)
		//		required DL (Byte/ms) = required DL (Byte/s) / 1000
		//		Limit = required DL (Byte/ms) * sample period (ms)
		//	The previous formula assumed 100ms period between two speed calculations
		//		uint32 limit = dwReduceDownload * dwCurrentDataRate / 1000;
		//			where dwReduceDownload = percent * 100
		//	Since we are calculating the average period value, then we gonna use this value
			uint32 dwUpperLimit = (m_dwDownDataRate * dwAverageTransferTimePeriod) / 100 * dwReduceDownload / 1000;
			uint32 dwLowerLimit = (200 * dwAverageTransferTimePeriod)/1000;

			if (dwUpperLimit < 1000 && dwReduceDownload == 200)
				dwUpperLimit += 1000;
			else if (dwUpperLimit < dwLowerLimit)
				dwUpperLimit = dwLowerLimit;
#ifdef OLD_SOCKETS_ENABLED
			m_pRequestSocket->SetDownloadLimit(dwUpperLimit);
#endif
		}
#ifdef OLD_SOCKETS_ENABLED
		else if (!m_bLimitlessDL)
		{
			m_pRequestSocket->DisableDownloadLimit();
			m_bLimitlessDL = true;
		}
#endif
	}

	UpdateDisplayedInfo();

	return m_dwDownDataRate;

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SetRemoteQueueRank(uint16 nr, bool bUpdateGUI /*= true*/)
{
	EMULE_TRY

	m_iDeltaQueueRank = static_cast<int>(nr) - static_cast<int>(m_uRemoteQueueRank);
	if (m_iDeltaQueueRank != 0)
	{
		//	if client sent us an opcode with non-0 rank, that it means we are on his WQ
		//	so if we had 0 rank before just enter into his WQ
		if (m_uRemoteQueueRank == 0)
			StartDLQueueWaitTimer();

		m_uRemoteQueueRank = nr;
		if (bUpdateGUI)
			UpdateDisplayedInfo();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A4AF switching
bool CUpDownClient::SwapToAnotherFile(CPartFile* SwapTo, uint32 dwOptions /*= 0*/)
{
	EMULE_TRY

//	Check if we are downloading something from client
	if (GetDownloadState() == DS_DOWNLOADING)
		return false;

//	Check: does client have another file?
	if (!m_otherRequestsList.IsEmpty())
	{
		CPartFile	*pPausedPartFile = NULL;
		POSITION	PausedFilePos = 0;
		POSITION	SwapToPos = 0;
		uint32		dwCurTick = ::GetTickCount();
		uint32		dwLastNNSCheckedTime = dwCurTick;
		bool		bIsFileReaskAllowed = true;
		BOOL		bNNS;
	
	//	Check: target file is specified? if is not, then find another one...
		if (!SwapTo)
		{
		//	Try to find a file in the same category
			CPartFile		*pCurFile;
			POSITION		cur_FilePos;
			POSITION		pos;
			uint32			dwA4AFRating;
			uint32			dwBestA4AFRating = 0;
			bool		bFoundInSameCat = false;

			for (pos = m_otherRequestsList.GetHeadPosition(); pos != NULL;)
			{
				pCurFile = m_otherRequestsList.GetNext(pos);
				if (CCat::FileBelongsToGivenCat(pCurFile, m_pReqPartFile->GetCatID()))
				{
					bFoundInSameCat = true;
					break;
				}
			}

		//	Find a file to switch to (now taking in account alphabetic order inside categories
		//	Do not change the way it's done (for loop and pCurFile assignment) because, I don't know why,
		//	it produces unexpected results. :(
			for (cur_FilePos = m_otherRequestsList.GetHeadPosition();cur_FilePos != 0;m_otherRequestsList.GetNext(cur_FilePos))
			{
				pCurFile = m_otherRequestsList.GetAt(cur_FilePos);

			//	Get first high priority files
				if (pCurFile != m_pReqPartFile && g_App.m_pDownloadQueue->IsInDLQueue(pCurFile))
				{
					EnumPartFileStatuses eStatus = pCurFile->GetStatus();

				//	allow switch to files with active status with 
					if (eStatus == PS_READY || eStatus == PS_EMPTY)
					{
					//	Check: is new file in NNS-Map?
						bNNS = m_otherNoNeededMap.Lookup(pCurFile, dwLastNNSCheckedTime);

					//	Calculate A4AF rating
						if (bNNS)
						{
							dwA4AFRating = (pCurFile->GetPriority() + 1) +
											((dwCurTick - dwLastNNSCheckedTime) / FILEREASKTIME);
							bIsFileReaskAllowed = (dwCurTick - dwLastNNSCheckedTime) > FILEREASKTIME;
						}
						else
						{
							dwA4AFRating = (pCurFile->GetPriority() + 1) * 3;
							bIsFileReaskAllowed = true;
						}

						if ((dwA4AFRating >= dwBestA4AFRating) 
							&& (bIsFileReaskAllowed || (dwOptions & A4AF_IGNORE_TIME_LIMIT)))
						{
							if((SwapTo == NULL) || !bFoundInSameCat || (SwapTo->GetFileName() > pCurFile->GetFileName()))
							{
								SwapTo = pCurFile;
								SwapToPos = cur_FilePos;
								dwBestA4AFRating = dwA4AFRating;
							}
						}
					}
					else if ((eStatus == PS_PAUSED) && (dwOptions & A4AF_TO_PAUSED))
					{
						pPausedPartFile = pCurFile;
						PausedFilePos = cur_FilePos;
					}
				}
			}
		}
		else if (SwapTo->GetStatus() == PS_READY || SwapTo->GetStatus() == PS_EMPTY)
		{
			bNNS = m_otherNoNeededMap.Lookup(SwapTo, dwLastNNSCheckedTime);

			if (bNNS)
			{
				bIsFileReaskAllowed = (dwCurTick - dwLastNNSCheckedTime) > FILEREASKTIME;
			}
			if (!bNNS || bIsFileReaskAllowed || (dwOptions & A4AF_IGNORE_TIME_LIMIT))
			{
				SwapToPos = m_otherRequestsList.Find(SwapTo);
			}
		}

	// if after one way swap only paused files exist, switch to the paused files
		if ((dwOptions & A4AF_TO_PAUSED) && (SwapTo == NULL) && (pPausedPartFile != NULL))
		{
			SwapTo = pPausedPartFile;
			SwapToPos = PausedFilePos;
		}

	//	Now let's swap the file to another
		if ((SwapTo != NULL) && (SwapTo != m_pReqPartFile) && (SwapToPos != 0))
		{
		//	Remove new file from OtherRequests list
			m_otherRequestsList.RemoveAt(SwapToPos);

		//	To prevent loop swapping remove a choosen part from NNS-Map
			m_otherNoNeededMap.RemoveKey(SwapTo);

		//	Remove current source from A4AF list in new file
			SwapTo->RemoveClientFromA4AFSourceList(this);

		//	To prevent loop swapping remember NNS-Part & time when it was switched
			if (dwOptions & A4AF_ONE_WAY)
				m_otherNoNeededMap.RemoveKey(m_pReqPartFile);
			else if ( m_pReqPartFile != NULL /* && GetDownloadState() == DS_NONEEDEDPARTS */ )
			{
				m_otherNoNeededMap[m_pReqPartFile] = dwCurTick;
			}

			ClearPartStatuses();
		//	Reset the filename to prevent the user misinformation in case of delayed request (for example LowID)
			m_strClientFilename.Empty();

			ResetLastAskedTime();
			m_uLastPartAsked = 0xFFFF;

			if (m_pReqPartFile != NULL)
			{
			//	Add to past comment
				m_pReqPartFile->CheckAndAddPastComment(this);
				m_eRating = PF_RATING_NONE;
				m_strComment.Empty();

			//	Remove current file
				m_pReqPartFile->RemoveClientFromDLSourceList(this);

			// add the client to the list if it's not required to remove him
				if (!(dwOptions & A4AF_ONE_WAY))
				{
				//	Add them in A4AF list
					m_pReqPartFile->AddClientToA4AFSourceList(this);

				//	Add old file to m_otherRequestsList
					if (!m_otherRequestsList.Find(m_pReqPartFile))
						m_otherRequestsList.AddTail(m_pReqPartFile);
				}

			//	Update download list (old file update)
				m_pReqPartFile->UpdateDisplayedInfo();
			}

		//	Actual switch
			SetDLRequiredFile(SwapTo);

		//	Add to the source list
		//	Note: don't change the state if a client is in connection state,
		//	in order to prevent the second connection to the same client
			if (GetDownloadState() == DS_CONNECTING
				|| GetDownloadState() == DS_WAITCALLBACK
				|| GetDownloadState() == DS_WAIT_FOR_FILE_REQUEST)
			{
				SwapTo->AddClientToSourceList(this, GetDownloadState());
			}
			else
			{
			//	The client will be added indirectly, but only in case if m_pReqPartFile was defined
				SetDownloadState(DS_WAIT_FOR_FILE_REQUEST);
			}

		//	Remove from past comment
			SwapTo->RemovePastComment(this);

		//	Update download list (new file update)
			SwapTo->UpdateDisplayedInfo();

		//	Now update the client item
			UpdateDisplayedInfo();

			return (m_pReqPartFile != NULL);
		}
	}

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::UDPReaskACK(uint16 nNewQR)
{
	EMULE_TRY

	m_bUDPPending = false;
	m_byteNumUDPPendingReqs = 0;
	SetRemoteQueueRank(nNewQR);
	SetLastAskedTime();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::UDPReaskFNF()
{
	EMULE_TRY

	m_bUDPPending = false;
	m_byteNumUDPPendingReqs = 0;

	if (GetDownloadState() != DS_DOWNLOADING)
	{
	//	Copy the current value as SwapToAnotherFile will change it
		CPartFile	*pPartFile = m_pReqPartFile;

	//	Try to swap the source to another file before removing it from DL-queue
		if (!SwapToAnotherFile(NULL, A4AF_REMOVE))
		{
#ifdef OLD_SOCKETS_ENABLED
		//	if client have socket or was not removed indirectly over disconnection, remove him from DLQ
			if ((m_pRequestSocket == NULL) && !Disconnected())
			{
				if (m_eUploadState == US_NONE)
					delete this;
				else
					g_App.m_pDownloadQueue->RemoveSource(this, true);
			}
#endif //OLD_SOCKETS_ENABLED
		}
		else
			g_App.m_pDownloadList->RemoveSource(this, pPartFile);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::UDPReaskForDownload()
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	uint32 dwCurTick = ::GetTickCount();

	m_bUDPPending = true;
	m_byteNumUDPPendingReqs++;
	m_dwNextFileReaskTime = dwCurTick + ((IsOnLAN()) ? LANCASTFILEREASKTIME : UDPFILEREASKTIMEOUT);

	CSafeMemFile	packetStream(128);

	packetStream.Write(m_pReqPartFile->GetFileHash(), 16);
	if (GetUDPVersion() > 3)
	{
		if (m_pReqPartFile->IsPartFile())
			((CPartFile*)m_pReqPartFile)->WritePartStatus(&packetStream);
		else
		{
			uint16	uNull = 0;

			packetStream.Write(&uNull, 2);
		}
	}
	if (GetUDPVersion() > 2)
	{
		uint16	uCompleteSourcesCount = m_pReqPartFile->GetCompleteSourcesCount();
		packetStream.Write(&uCompleteSourcesCount, 2);
	}

	Packet		*pPacket = new Packet(&packetStream, OP_EMULEPROT);

	pPacket->m_eOpcode = OP_REASKFILEPING;

	g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
	g_App.m_pClientUDPSocket->SendPacket(pPacket, GetIP(), GetUDPPort());

	AddAskedCountDown();

	return true;
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Sets the vector of bool 'parts' to show the pending status of each part.
void CUpDownClient::ShowDownloadingParts(_Bvector& parts)
{
	EMULE_TRY

	parts.resize(m_uPartCount);

	for (POSITION pos = m_pendingBlocksList.GetHeadPosition(); pos != NULL; )
		parts[static_cast<uint32>(m_pendingBlocksList.GetNext(pos)->pReqBlock->qwStartOffset / PARTSIZE)] = true;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::UpdateDisplayedInfo()
{
	g_App.m_pDownloadList->UpdateSource(this);
	g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.UpdateClient(this);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ClearPendingBlocksList(void)
{
	EMULE_TRY

	Pending_Block_Struct*		pPendingBlock;

	while (!m_pendingBlocksList.IsEmpty() )
	{
		pPendingBlock = m_pendingBlocksList.RemoveHead();

		if (pPendingBlock != NULL)
		{
			CPartFile *pReqFile = g_App.m_pDownloadQueue->GetFileByID(pPendingBlock->pReqBlock->m_fileHash);

			if (pPendingBlock->pZStream != NULL)
			{
				uint32	dwTotalOut;

				if ((dwTotalOut = pPendingBlock->pZStream->total_out) != 0)
				{
				//	Some data was decompressed

				//	Force file write of partly received compressed block
				//	The data buffer will be freed while flushing to the file
					if ( pReqFile->WriteToBuffer( dwTotalOut - pPendingBlock->pZStream->total_in,
						pPendingBlock->pbyteRxDate, pPendingBlock->pReqBlock->qwStartOffset,
						pPendingBlock->pReqBlock->qwStartOffset + static_cast<uint64>(dwTotalOut - 1),
						PF_WR2BUF_FL_FREEBUFFER | PF_WR2BUF_FL_ENDOFBLOCK ) != 0 )
					{
					//	As we save some bytes then this DL session can be counted as successful
						m_bTransferredDownMini = true;
					}
				}
				inflateEnd(pPendingBlock->pZStream);
				delete pPendingBlock->pZStream;
			}
			pReqFile->RemoveBlockFromList(pPendingBlock->pReqBlock->qwStartOffset,
										pPendingBlock->pReqBlock->qwEndOffset);
			delete pPendingBlock->pReqBlock;
			delete pPendingBlock;
		}
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	If downloadBlocks are existing free block reserved in pPartFile
void CUpDownClient::ClearDownloadBlocksList(void)
{
	while (!m_downloadBlocksList.IsEmpty())
	{
		Requested_Block_Struct	*pDLBlock = m_downloadBlocksList.RemoveHead();

		if (pDLBlock != NULL)
		{
			CPartFile* pReqFile = g_App.m_pDownloadQueue->GetFileByID(pDLBlock->m_fileHash);

			pReqFile->RemoveBlockFromList(pDLBlock->qwStartOffset, pDLBlock->qwEndOffset);
			delete pDLBlock;
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendStartUploadRequest()
{
	Packet	*pPacket = NULL;

	try
	{
		if (m_pReqPartFile != NULL && m_pRequestSocket != NULL)
		{
			pPacket = new Packet(OP_STARTUPLOADREQ, 16);

			md4cpy(pPacket->m_pcBuffer, m_pReqPartFile->GetFileHash());
			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
			m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED

			SetLastAskedTime();
		}
	}
	catch (CException *error)
	{
		safe_delete(pPacket);
		g_App.m_pMDlg->AddBugReport(_T(__FUNCTION__), _T(__FILE__), __LINE__, GetErrorMessage(error));
		error->Delete();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendHashsetRequest()
{
	Packet	*pPacket = NULL;

	try
	{
		if (m_pReqPartFile != NULL)
		{
			pPacket = new Packet(OP_HASHSETREQUEST, 16);

			md4cpy(pPacket->m_pcBuffer, m_pReqPartFile->GetFileHash());
			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
			m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED

			SetDownloadState(DS_REQHASHSET);
			m_pReqPartFile->m_bHashSetNeeded = false;
			m_fRequestingHashSet = 1;

			AddLogLine(LOG_FL_DBG, _T("Sent hashset request with filehash %s to client %s"), HashToString(m_pReqPartFile->GetFileHash()), GetClientNameWithSoftware());
		}
	}
	catch (CException *error)
	{
		safe_delete(pPacket);
		g_App.m_pMDlg->AddBugReport(_T(__FUNCTION__), _T(__FILE__), __LINE__, GetErrorMessage(error));
		error->Delete();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// return the remainig download time for the current chunk in seconds
uint32 CUpDownClient::GetRemainingTimeForCurrentPart()
{
	uint32	dwDownloadingPart, dwSecond = 0;

	if ( (m_pReqPartFile != NULL) && (m_eDownloadState == DS_DOWNLOADING) &&
		!m_pendingBlocksList.IsEmpty() && (m_dwDownDataRate != 0) )
	{
		dwDownloadingPart = static_cast<uint32>(m_pendingBlocksList.GetHead()->pReqBlock->qwStartOffset / PARTSIZE);
		dwSecond = m_pReqPartFile->GetPartLeftToDLSize(dwDownloadingPart) / m_dwDownDataRate;
	}

	return dwSecond;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// return the remainig download size for the current chunk in bytes as well as currently downloading part
uint32 CUpDownClient::GetRemainingSizeForCurrentPart(uint32 *pdwDownloadingPart)
{
	uint32 dwSize = 0;

	if ((m_pReqPartFile != NULL) && (m_eDownloadState == DS_DOWNLOADING) && !m_pendingBlocksList.IsEmpty())
	{
		*pdwDownloadingPart = static_cast<uint32>(m_pendingBlocksList.GetHead()->pReqBlock->qwStartOffset / PARTSIZE);
		dwSize = m_pReqPartFile->GetPartLeftToDLSize(*pdwDownloadingPart);
	}

	return dwSize;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendCancelTransfer()
{
	if (m_pRequestSocket != NULL)
	{
		Packet	*pPacket = NULL;

		if (m_fSentCancelTransfer)
			return;
		try
		{
			pPacket = new Packet(OP_CANCELTRANSFER, 0);

			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);

#ifdef OLD_SOCKETS_ENABLED
			m_pRequestSocket->SendPacket(pPacket, true, true);
#endif
			m_fSentCancelTransfer = 1;
		}
		catch (CException *error)
		{
			safe_delete(pPacket);
			g_App.m_pMDlg->AddBugReport(_T(__FUNCTION__), _T(__FILE__), __LINE__, GetErrorMessage(error));
			error->Delete();
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ClearPartStatuses()
{
	delete[] m_pbytePartStatuses;
	m_pbytePartStatuses = NULL;
	m_uPartCount = m_uAvailPartCount = m_uNeededParts = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SetLastAskedTime()
{
	m_dwLastAskedTime = ::GetTickCount();

	SetNextFileReaskTime();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SetNextFileReaskTime()
{
	if (IsOnLAN())
	{
		m_dwNextFileReaskTime = m_dwLastAskedTime + LANCASTFILEREASKTIME;
		return;
	}

	m_dwNextFileReaskTime = m_dwLastAskedTime + FILEREASKTIME;

	if (GetDownloadState() == DS_ONQUEUE)
	{
		uint32	dwQueueRank = static_cast<uint32>(GetRemoteQueueRank());

	//	Increase the interval if remote queue is full or we have a low rank
	//	In case when rank is unknown (dwQueueRank = 0), normal reask period is used
		if (dwQueueRank > QUEUERANK_LOW)
			m_dwNextFileReaskTime += g_App.m_pServerConnect->IsLowID() ? (FILEREASKTIME / 4) : (FILEREASKTIME / 2);
	//	Increase reask time to LowID clients to give the remote client a chance to connect first
		else if (HasLowID() && !g_App.m_pServerConnect->IsLowID())
			m_dwNextFileReaskTime += HIGH2LOW_REASKDELAY;
	}
	else if (GetDownloadState() == DS_NONEEDEDPARTS)
	{
		m_dwNextFileReaskTime += FILEREASKTIME / 2;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::IsUDPRequestAllowed()
{
//	Disable UDP...
//	...if we have more than 3 pending UDP requests in a row (UDP is blocked)
	if (m_bUDPPending || (m_byteNumUDPPendingReqs > 3))
		return false;
//	...for old versions
	if (m_byteEmuleVersion < 0x23)
		return false;
//	...in case of wrong UDP port settings
	if ((m_uUDPPort == 0) || (g_App.m_pPrefs->GetUDPPort() == 0))
		return false;
//	...if local client is already connected to remote client over TCP
	if ((m_pRequestSocket != NULL) && (m_pRequestSocket->IsConnected() || m_pRequestSocket->IsConnecting()))
		return false;
//	...if one of clients has LowID
	if (g_App.m_pServerConnect->IsLowID() || HasLowID())
		return false;
//	...when connecting to a PROXY
	if (g_App.m_pPrefs->GetProxySettings().m_bUseProxy)
		return false;

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendReask()
{
//	Try to request the file status over UDP before TCP request
	if (IsUDPRequestAllowed() && !IsSourceRequestAllowed())
		UDPReaskForDownload();
	else
		AskForDownload();
}
