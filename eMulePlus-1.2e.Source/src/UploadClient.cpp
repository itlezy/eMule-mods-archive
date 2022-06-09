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
#include "UpDownClient.h"
#include "opcodes.h"
#include "packets.h"
#include "emule.h"
#include "UploadQueue.h"
#include "otherstructs.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::DrawUpStatusBar(CDC *pDC, RECT *pRect, bool bFlat)
{
	EMULE_TRY

	const COLORREF	crSentData = RGB(222, 160, 0);
	const COLORREF	crSendingPart = RGB(255, 208, 0);
	const COLORREF	crMeOnly = RGB(190, 190, 255);
	COLORREF		crBoth, crNeither, crClientOnly;

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

//	Get and set the file's size
	CPartFile	   *pUploadingFile = reinterpret_cast<CPartFile*>(g_App.m_pSharedFilesList->GetFileByID(m_reqFileHash));
	uint64			qwFileSz = (pUploadingFile != NULL) ? pUploadingFile->GetFileSize() : 1ui64;
	CBarShader		statusBar(pRect->bottom - pRect->top, pRect->right - pRect->left, crNeither, qwFileSz);

	if ((m_pbyteUpPartStatuses != NULL) && (pUploadingFile != NULL))
	{
		uint32		dwUploadingPart = GetCurrentlyUploadingPart();
		uint64		qwStartPos, qwEndPos;

		for (uint32 i = 0; i < m_uUpPartCount; i++)
		{
			qwStartPos = static_cast<uint64>(i) * PARTSIZE;
			qwEndPos = ((qwStartPos + PARTSIZE) > qwFileSz) ? qwFileSz : (qwStartPos + PARTSIZE);
			if (i == dwUploadingPart)
			{
			//	Add the current uploading part
				statusBar.FillRange(qwStartPos, qwEndPos, crSendingPart);

				ReqBlockDeque::iterator	itBlock;
				Requested_Block_Struct	*pDoneBlock;

			//	Add already uploaded pieces
				for (itBlock = m_doneBlocksList.begin(); itBlock != m_doneBlocksList.end(); itBlock++)
				{
					pDoneBlock = *itBlock;
					if (dwUploadingPart == static_cast<uint32>(pDoneBlock->qwStartOffset / PARTSIZE))
						statusBar.FillRange(pDoneBlock->qwStartOffset, pDoneBlock->qwEndOffset, crSentData);
				}
			}
			else if (!pUploadingFile->IsPartFile() || pUploadingFile->IsPartComplete(i))	// We've this part
			{
				statusBar.FillRange(qwStartPos, qwEndPos, (m_pbyteUpPartStatuses[i] != 0) ? crBoth : crMeOnly);
			}
			else if (m_pbyteUpPartStatuses[i] != 0)	// The remote user has this part
			{
				statusBar.FillRange(qwStartPos, qwEndPos, crClientOnly);
			}
		}
	}
	statusBar.Draw(pDC, pRect->left, pRect->top, bFlat);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::IsCommunity()
{
	EMULE_TRY
	
	if (m_byteActionsOnNameChange & AONC_COMMUNITY_CHECK)
	{
		m_bIsCommunity = false;

		if (g_App.m_pPrefs->CommunityEnabled())
		{
			CString		strLowerCommunity = g_App.m_pPrefs->CommunityString();

			strLowerCommunity.MakeLower();
			strLowerCommunity.Remove(_T('['));
			strLowerCommunity.Remove(_T(']'));

			if (!strLowerCommunity.IsEmpty())
			{
				CString		strProperCommunity;

				strProperCommunity.Format(_T("[%s]"), strLowerCommunity);

			//	Ignore unsuitable tags which are not meant to be used as community
				CString		strLowerUserName = m_strUserName;

				strLowerUserName.MakeLower();
				strLowerUserName.Replace(_T("[eplus]"), _T(""));

			//	Only the first community-tag is evaluated to be valid
				int		iFirstTagPos = strLowerUserName.Find(_T('['));

				if (iFirstTagPos >= 0 && iFirstTagPos == strLowerUserName.Find(strProperCommunity))
					m_bIsCommunity = true;
			}
		}

		m_byteActionsOnNameChange &= ~AONC_COMMUNITY_CHECK;
	}

	return m_bIsCommunity;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CUpDownClient::GetScore(bool bOnlyBaseValue /*= false*/)
{
	EMULE_TRY

	if ((m_eBanState != BAN_CLIENT_NONE) || m_strUserName.IsEmpty())
		return 0;

	if (HasUserNameForbiddenStrings() || HasMODNameForbiddenStrings())
	{
		Ban(BAN_CLIENT_KWOWN_LEECHER);
		return 0;
	}

	if (m_pCredits != NULL && m_pCredits->GetCurrentIdentState(GetIP()) == IS_IDBADGUY)
		return 0;

	if (IsDownloading())
		return 0;

	CKnownFile	*pSharedFile = g_App.m_pSharedFilesList->GetFileByID(m_reqFileHash);

	if (pSharedFile == NULL)
		return 0;

	ASSERT(m_reqFileHash != NULL);

	double	dBaseValue, dFilePriority;

	switch (pSharedFile->GetULPriority())
	{
		case PR_RELEASE:
			if (pSharedFile->IsPartFile())
				dFilePriority = 5.0;
			else
				dFilePriority = 10.0;
			break;

		case PR_HIGH:
			dFilePriority = 2.0;
			break;

		case PR_LOW:
			dFilePriority = 0.5;
			break;

		case PR_VERYLOW:
			dFilePriority = 0.2;
			break;

		case PR_NORMAL:
		default:
			dFilePriority = 1.0;	// standard
			break;
	}

	double	dPopularityRatio = pSharedFile->GetPopularityRatio();

//	Calculate score, based on waiting time and other factors
	if (bOnlyBaseValue)
		dBaseValue = 100.0;
	else
	{
#ifdef OLD_SOCKETS_ENABLED
		if (HasLowID() && !(m_pRequestSocket && m_pRequestSocket->IsConnected()))
		{
			if (!g_App.m_pServerConnect->IsConnected() || g_App.m_pServerConnect->IsLowID())
				return 0;
		}
#endif //OLD_SOCKETS_ENABLED
		dBaseValue = static_cast<double>(::GetTickCount() - GetWaitStartTime()) / 1000.0;
	}

	if (IsFriend())			//	Client is on friendlist
		dBaseValue *= 2.0;
	else if (IsCommunity())	//	Client is a community user
		dBaseValue *= 1.5;
	else if ((m_fIdenThief == 1) && g_App.m_pPrefs->IsCounterMeasures())
		dBaseValue *= 0.8;

//	Client has credits (modifier between 1.0 and 10.0)
	if (m_pCredits != NULL)
		dBaseValue *= m_pCredits->GetScoreRatio(GetIP());

	dBaseValue *= dFilePriority * pSharedFile->GetSizeRatio() * dPopularityRatio;

	return static_cast<uint32>(dBaseValue);

	EMULE_CATCH

	return 1;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Checks if it is next requested block from another chunk of the actual file or from another file
//	Return:
//		> 0 : Next requested block is from another chunk or file than last downloaded block
//		= 0 : Next requested block is from same chunk that last downloaded block
//		< 0 : Next requested block is from another chunk or file, but allow to continue
int CUpDownClient::IsDifferentPartBlock(bool *pbJSBlocked)
{
	int			iDiffPart = 0;
	CKnownFile	*pKFile = NULL;

	*pbJSBlocked = false;
	try
	{
	//	Check if we have good lists and proceed to check for different chunks
		if (!m_blockRequestsQueue.empty())
		{
		//	Get next pending block
			Requested_Block_Struct	*pNextReqBlk = m_blockRequestsQueue.front();
		//	Calculate corresponding part for the block
			uint32		dwNextReqPart = static_cast<uint32>(pNextReqBlk->qwStartOffset / PARTSIZE);

			if (!m_doneBlocksList.empty())
			{
			//	Get last transferred block
				Requested_Block_Struct	*pLastDoneBlk = m_doneBlocksList.front();
				uint32	dwLastDonePart = static_cast<uint32>(pLastDoneBlk->qwStartOffset / PARTSIZE);

			//	Check that the same file and part are asked
				if (dwLastDonePart != dwNextReqPart || md4cmp(pLastDoneBlk->m_fileHash, pNextReqBlk->m_fileHash) != 0)
					iDiffPart = 1;
			}

			pKFile = g_App.m_pSharedFilesList->GetFileByID(pNextReqBlk->m_fileHash);
			if ((pKFile != NULL) && !pKFile->AllowChunkForClient(dwNextReqPart, this))
				*pbJSBlocked = true;	//	Forbidden by Jumpstart
		}
	}
	catch (...)
	{
		iDiffPart = 1;
	}

	if (iDiffPart > 0)
	{
		if (GetSessionUp() < (PARTSZ32 / 4))
			iDiffPart = -1;
		else if (GetSessionUp() < (3 * PARTSZ32 / 4))
		{
			if (IsFriend())
				iDiffPart = -1;
			else if (pKFile != NULL)
			{
				if (pKFile->IsPartFile())
				{
					uint32	dwSrcNum = reinterpret_cast<CPartFile*>(pKFile)->GetSourceCount();

				//	File is rare if number of sources != 0 and < RARE_FILE
					if ((dwSrcNum - 1u) < (RARE_FILE - 1))
						iDiffPart = -1;
				}
				else
				{
					if (pKFile->GetCompleteSourcesCount() < 5)
						iDiffPart = -1;
				}
			}
		}
	}

	return iDiffPart;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Check if we should kick this client
//	- Always kick if transfer is longer than SESSIONMAXTIME
//	- Never kick friends before a chunck was transferred
//	- Never allow more than the size of a chunk (+500 KBytes for small resends of invalid data) to be transferred
//	- We never kick a lancast user
//	- Full chunk transfers are being enforced, is not more an option
//
EnumBlockPacketCreationResult CUpDownClient::CreateNextBlockPackage()
{
	EMULE_TRY

//	If there are no new blocks requested, we will happily transfer more if the client requested it
	if (m_blockRequestsQueue.empty())
		return BPCR_FAILED_NO_REQUESTED_BLOCKS;

	bool	bJumpstartBlocked;
	int		iDiffPart = IsDifferentPartBlock(&bJumpstartBlocked);

	if (bJumpstartBlocked)
		return BPCR_FAILED_BLOCKED_PART;

	bool	bAllowedMore = IsOnLAN();
	uint32	dwTimeout = 0;

//	If user is from Internet & Full chunk (or timeout)
	if ( !bAllowedMore &&
		((iDiffPart > 0) || ((dwTimeout = GetUpStartTimeDelay()) > SESSIONMAXTIME)) )
	{
		if ((dwTimeout > SESSIONMAXTIME) && IsFriend())
		{
			bAllowedMore = true;
			if (g_App.m_pPrefs->IsClientTransferLogEnabled())
				AddLogLine(LOG_FL_DBG, _T("Client %s downloaded %u KB so far. Allowed to continue downloading (should go back on queue) because it's a friend"), GetClientNameWithSoftware(), this->GetSessionUp() / 1024);
		}
		else
		{
			if (iDiffPart > 0)
				return BPCR_FAILED_BLOCKED_PART;
			else
				return BPCR_FAILED_TIME_LIMIT;
		}
	}

	CFile			file;
	CKnownFile*		srcfile;
	byte*			filedata = NULL;
	EnumBlockPacketCreationResult eBPCResult = BPCR_FAILED_TIME_LIMIT;

	try
	{
		if ((iDiffPart < 0) && !bAllowedMore && g_App.m_pPrefs->IsClientTransferLogEnabled())
			AddLogLine(LOG_FL_DBG, _T("Client %s continues downloading (should be back on queue) because of a low transferred size"), GetClientNameWithSoftware());
	//	Note: since this function is called when upload packets queue is empty,
	//	the upload can be broken during packets creation for requested blocks,
	//	therefore in order to decrease the probability of connection loss,
	//	we gonna create just one requested block packet. This leads to:
	//	1) faster response after initial request -> better number of successful upload sessions
	//	2) reduced delay between some packets -> smoother upload -> less possibility of connection loss
		Requested_Block_Struct	   *pCurrBlk = m_blockRequestsQueue.front();

		srcfile = g_App.m_pSharedFilesList->GetFileByID(pCurrBlk->m_fileHash);

		if (srcfile == NULL)
		{
			CString		strError;

			eBPCResult = BPCR_FAILED_FILE_ERROR;
			strError.Format(_T("requested file not found, hash: %s"), HashToString(pCurrBlk->m_fileHash));
			throw strError;
		}

	//	Update the file hash to the requested file
		md4cpy(m_reqFileHash, pCurrBlk->m_fileHash);

	//	Check that block start is within the file
		if (pCurrBlk->qwStartOffset >= srcfile->GetFileSize())
		{
			CString		strError;

			eBPCResult = BPCR_FAILED_FILE_ERROR;
			strError.Format( _T("request data %#I64x-%#I64x beyond the file end %#I64x '%s'"),
				pCurrBlk->qwStartOffset, pCurrBlk->qwEndOffset, srcfile->GetFileSize(), srcfile->GetFileName());
			throw strError;
		}

		uint32 dwPart = static_cast<uint32>(pCurrBlk->qwStartOffset / PARTSIZE);

	//	Accessing a blocked part ?
		if (!srcfile->GetJumpstartEnabled() && !srcfile->IsPartShared(dwPart))
		{
			CString		strError;

			AddLogLine(0, IDS_PARTPRIO_DENIED, dwPart, srcfile->GetFileName());
			strError.Format(GetResString(IDS_PARTPRIO_DENIED), dwPart, srcfile->GetFileName());
			eBPCResult = BPCR_FAILED_BLOCKED_PART;
			throw strError;
		}

		uint32 dwToGo;
		uint64 qwStart = static_cast<uint64>(dwPart) * PARTSIZE;
		uint64 qwEnd = ((qwStart + PARTSIZE) > srcfile->GetFileSize()) ? srcfile->GetFileSize() : (qwStart + PARTSIZE);

	//	Everyone is limited to a single chunk (some functions below were also optimized for that)
	//	Check that m_dwStartOffset and m_dwEndOffset are in the same chunk and
	//	make sure we don't pass the end of the file.
		if ( (dwPart != static_cast<uint32>((pCurrBlk->qwEndOffset - 1ui64) / PARTSIZE)) ||
			(pCurrBlk->qwEndOffset > qwEnd) )
		{
		//	m_dwEndOffset goes into the next chunk or is beyond the file end.
		//	Set it to the end of the chunk that m_dwStartOffset is in.
			pCurrBlk->qwEndOffset = qwEnd;
		}

	//	This can't be a wrapped around request, since it has been limited to a single chunk
		dwToGo = static_cast<uint32>(pCurrBlk->qwEndOffset - pCurrBlk->qwStartOffset);

		if (srcfile->IsPartFile())
		{
			if (!(reinterpret_cast<CPartFile*>(srcfile))->IsPartComplete(dwPart))
			{
				CString		strError;

				strError.Format(_T("asked for incomplete block from '%s'"), srcfile->GetFileName());
				eBPCResult = BPCR_FAILED_BLOCKED_PART;
				throw strError;
			}
		}

	//	Create a buffer for file data before we start to work with the file
		filedata = new byte[dwToGo + 500];

		int	iRc = srcfile->ReadFileForUpload(pCurrBlk->qwStartOffset, dwToGo, filedata);

		if (iRc < 0)
		{
		//	Error occurred
			CString		strError;

			strError.Format(_T("failed to open '%s'"), srcfile->GetFileName());
			eBPCResult = BPCR_FAILED_FILE_ERROR;
			throw strError;
		}

		md4cpy(m_reqFileHash, pCurrBlk->m_fileHash);

		bool bFromPF = (iRc) ? true : false;
		bool bPacketCreated, bCompress = false;

		if (m_byteDataCompVer == 1)
		{
			bCompress = srcfile->IsCompressedTransferAllowed();
		}

		if (bCompress)
			bPacketCreated = CreatePackedPackets(filedata, dwToGo, pCurrBlk, srcfile->GetULPriority(), bFromPF);
		else
			bPacketCreated = CreateStandardPackets(filedata, dwToGo, pCurrBlk, srcfile->GetULPriority(), bFromPF);

	//	Update the data only if packet was created
		if (bPacketCreated)
		{
		//	File statistic
			srcfile->statistic.AddTransferred(dwToGo);
			srcfile->statistic.AddTraffic(dwPart, pCurrBlk->qwStartOffset, dwToGo);

			if (srcfile->GetJumpstartEnabled())
				srcfile->AddSentBlock(this, pCurrBlk->qwStartOffset, dwToGo);

			m_doneBlocksList.push_front(m_blockRequestsQueue.front());

			eBPCResult = BPCR_OK;
		}

		m_blockRequestsQueue.pop_front();
		delete[] filedata;
		filedata = NULL;
	}
//	Check for exception that will be throw by "new"
	catch (CMemoryException *error)
	{
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to create upload package for %s - %s"), GetUserName(), GetErrorMessage(error));
		error->Delete();
		SetUploadFileID(NULL);
		return BPCR_FAILED_FILE_ERROR;
	}
	catch (CFileException *error)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Failed to create upload package for %s - %s"), GetUserName(), GetErrorMessage(error));
		SetUploadFileID(NULL);
		delete[] filedata;
		error->Delete();
		return BPCR_FAILED_FILE_ERROR;
	}
	catch (CString &strError)
	{
		OUTPUT_DEBUG_TRACE();
		AddLogLine(LOG_FL_DBG, _T("Client %s caused an error while creating package (%s), disconnecting!"), GetClientNameWithSoftware(), strError);
		SetUploadFileID(NULL);
		delete[] filedata;
		return eBPCResult;
	}

	return eBPCResult;

	EMULE_CATCH

	return BPCR_FAILED_FILE_ERROR;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::CheckForUpPartsInfo()
{
	EMULE_TRY

	if ( m_uPartCount == 0 || m_pReqPartFile == NULL
		|| md4cmp(m_reqFileHash, m_pReqPartFile->GetFileHash())
		|| m_pbytePartStatuses == NULL )
	{
		return;
	}

	delete[] m_pbyteUpPartStatuses;
	m_pbyteUpPartStatuses = NULL;

	m_uUpPartCount = m_uPartCount;
	m_uAvailUpPartCount = 0;
	m_pbyteUpPartStatuses = new byte[m_uUpPartCount];

	for (int i = 0; i < m_uUpPartCount; i++)
	{
		m_pbyteUpPartStatuses[i] = m_pbytePartStatuses[i];

		if (m_pbyteUpPartStatuses[i] != 0)
			m_uAvailUpPartCount++;
	}
	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	ProcessExtendedInfo - processes extended information about uploading file
//	Return:	false - received packet is for a wrong file, otherwise true
bool CUpDownClient::ProcessExtendedInfo(byte *pbytePacket, uint32 dwPacketSz, CKnownFile *pKnownFile)
{
	delete[] m_pbyteUpPartStatuses;
	m_pbyteUpPartStatuses = NULL;
	m_uUpPartCount = 0;
	m_uAvailUpPartCount = 0;
	m_uUpCompleteSourcesCount = 0;
	if (GetExtendedRequestsVersion() == 0)
	{
		CheckForUpPartsInfo();
		return true;
	}

//	Skip hash, it was already processed by the caller
	CSafeMemFile	packetStream(reinterpret_cast<BYTE*>(pbytePacket + 16), dwPacketSz - 16);
	uint16			uED2KUpPartCount;

	packetStream.Read(&uED2KUpPartCount, 2);
	m_uUpPartCount = pKnownFile->GetPartCount();

	if (uED2KUpPartCount == 0)
	{
		m_pbyteUpPartStatuses = new byte[m_uUpPartCount];
		memzero(m_pbyteUpPartStatuses, m_uUpPartCount);
	}
	else
	{
		m_fNoDataForRemoteClient = 1;
		if (pKnownFile->GetED2KPartCount() != uED2KUpPartCount)
		{
			m_uUpPartCount = 0;
			return false;
		}

		m_pbyteUpPartStatuses = new byte[m_uUpPartCount];

		uint32	dwDone = 0, dwUpPartCnt = static_cast<uint32>(m_uUpPartCount);
		byte	byteToRead;

		while (dwDone < dwUpPartCnt)
		{
			packetStream.Read(&byteToRead, 1);
			for (unsigned ui = 0; ui < 8; ui++)
			{
				m_pbyteUpPartStatuses[dwDone] = static_cast<byte>((byteToRead >> ui) & 1);
				if (m_pbyteUpPartStatuses[dwDone])
					m_uAvailUpPartCount++;
				else
				{
					if (pKnownFile->IsPartShared(dwDone))
						m_fNoDataForRemoteClient = 0;
				}
				if (++dwDone == dwUpPartCnt)
					break;
			}
		}

		if (GetExtendedRequestsVersion() > 1)
		{
			uint16	uCompleteCountLast = GetUpCompleteSourcesCount();
			uint16	uCompleteCountNew;

			packetStream.Read(&uCompleteCountNew, 2);
			SetUpCompleteSourcesCount(uCompleteCountNew);
			if (uCompleteCountLast != uCompleteCountNew)
				pKnownFile->CalculateCompleteSources();
		}
	}
	g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.UpdateClient(this);

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::CreateStandardPackets(byte *pbyteFileData, uint32 dwToGo, Requested_Block_Struct *pCurrBlk, byte byteFilePriority, bool bFromPF /*= true*/)
{
	EMULE_TRY

	uint32		dwPacketSize;
	CMemFile	memfile(pbyteFileData, dwToGo);
	PacketDeque	blockSendTempQueue;
	Packet		*pPacket;

	if (dwToGo > 10240)
		dwPacketSize = dwToGo / (dwToGo / 10240u);
	else
		dwPacketSize = dwToGo;

	while (dwToGo != 0)
	{
		if (dwToGo < dwPacketSize * 2)
			dwPacketSize = dwToGo;

		dwToGo -= dwPacketSize;

		uint64 qwEnd = pCurrBlk->qwEndOffset - static_cast<uint64>(dwToGo);
		uint64 qwStart = qwEnd - static_cast<uint64>(dwPacketSize);

		if (qwEnd > 0xFFFFFFFFui64)		//	Start is always < End
		{
			pPacket = new Packet(OP_SENDINGPART_I64, dwPacketSize + 32, OP_EMULEPROT, byteFilePriority, bFromPF);
			md4cpy(&pPacket->m_pcBuffer[0], m_reqFileHash);
			POKE_QWORD(&pPacket->m_pcBuffer[16], qwStart);
			POKE_QWORD(&pPacket->m_pcBuffer[24], qwEnd);
		//	Append the file data to the packet
			memfile.Read(&pPacket->m_pcBuffer[32], dwPacketSize);
			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(32);
		}
		else
		{
			pPacket = new Packet(OP_SENDINGPART, dwPacketSize + 24, OP_EDONKEYPROT, byteFilePriority, bFromPF);
			md4cpy(&pPacket->m_pcBuffer[0], m_reqFileHash);
			POKE_DWORD(&pPacket->m_pcBuffer[16], static_cast<uint32>(qwStart));
			POKE_DWORD(&pPacket->m_pcBuffer[20], static_cast<uint32>(qwEnd));
		//	Append the file data to the packet
			memfile.Read(&pPacket->m_pcBuffer[24], dwPacketSize);
			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(24);
		}

	//	If the split was successful, then move the split packets to the global queue
		if (SplitAndAddPacketsToSendQueue(pPacket, &blockSendTempQueue, byteFilePriority, bFromPF))
		{
		//	Attach split packets to the global queue
			while (!blockSendTempQueue.empty())
			{
				m_blockSendQueue.push_back(blockSendTempQueue.front());
				blockSendTempQueue.pop_front();
			}
		}
		else
		{
			while (!blockSendTempQueue.empty())
			{
				delete blockSendTempQueue.front();
				blockSendTempQueue.pop_front();
			}
		}
	}

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CUpDownClient::CreatePackedPackets(byte *pbyteFileData, uint32 dwToGo, Requested_Block_Struct *pCurrBlk, byte byteFilePriority, bool bFromPF /*= true*/)
{
	EMULE_TRY

	BYTE	*output = new BYTE[dwToGo + 300];
	uLongf	newsize = dwToGo + 300;
	int		iResult = compress2(output, &newsize, pbyteFileData, dwToGo, 9);

	if ((iResult != Z_OK) || (dwToGo <= newsize))
	{
		delete[] output;
		return CreateStandardPackets(pbyteFileData, dwToGo, pCurrBlk, byteFilePriority, bFromPF);
	}

	CMemFile	memfile(output, newsize);
	PacketDeque	blockSendTempQueue;
	uint32		dwPacketSize;
	Packet		*pPacket;

//	Update the compression statistic
	m_dwCompressionGain += (dwToGo - newsize);
	m_dwUncompressed += dwToGo;

	dwToGo = newsize;
	if (dwToGo > 10240)
		dwPacketSize = dwToGo / (uint32)(dwToGo / 10240);
	else
		dwPacketSize = dwToGo;

	while (dwToGo != 0)
	{
		if (dwToGo < dwPacketSize * 2)
			dwPacketSize = dwToGo;

		dwToGo -= dwPacketSize;

		if (pCurrBlk->qwEndOffset > 0xFFFFFFFFui64)		//	Start is always < End
		{
			pPacket = new Packet(OP_COMPRESSEDPART_I64, dwPacketSize + 28, OP_EMULEPROT, byteFilePriority, bFromPF);
			md4cpy(&pPacket->m_pcBuffer[0], m_reqFileHash);
			POKE_QWORD(&pPacket->m_pcBuffer[16], pCurrBlk->qwStartOffset);
			POKE_DWORD(&pPacket->m_pcBuffer[24], newsize);
			memfile.Read(&pPacket->m_pcBuffer[28], dwPacketSize);
			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(28);
		}
		else
		{
			pPacket = new Packet(OP_COMPRESSEDPART, dwPacketSize + 24, OP_EMULEPROT, byteFilePriority, bFromPF);
			md4cpy(&pPacket->m_pcBuffer[0], m_reqFileHash);
			POKE_DWORD(&pPacket->m_pcBuffer[16], static_cast<uint32>(pCurrBlk->qwStartOffset));
			POKE_DWORD(&pPacket->m_pcBuffer[20], newsize);
			memfile.Read(&pPacket->m_pcBuffer[24], dwPacketSize);
			g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(24);
		}

	//	If one of the split action was failed, then cancel whole packet creation, cause otherwise Z-stream on other side will be corrupted
		if (!SplitAndAddPacketsToSendQueue(pPacket, &blockSendTempQueue, byteFilePriority, bFromPF))
		{
			while (!blockSendTempQueue.empty())
			{
				delete blockSendTempQueue.front();
				blockSendTempQueue.pop_front();
			}
			delete[] output;
			return false;
		}
	}
	delete[] output;

//	Attach split packets to global queue
	while (!blockSendTempQueue.empty())
	{
		m_blockSendQueue.push_back(blockSendTempQueue.front());
		blockSendTempQueue.pop_front();
	}

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// A packet should never be larger than MAXFRAGSIZE. This method splits up the
// given packets into several packets with the size MAXFRAGSIZE, if necessary.
bool CUpDownClient::SplitAndAddPacketsToSendQueue(Packet *pPacket, PacketDeque *pSendQueue, byte byteFilePriority, bool bFromPF)
{
	EMULE_TRY

	if (pPacket->GetRealPacketSize() > 0 && pPacket->GetRealPacketSize() > MAXFRAGSIZE * 2)
	{
	//	Splitting packets
		uint32		dwSize = pPacket->GetRealPacketSize();
		char	   *m_pcBuffer = pPacket->DetachPacket();

		delete pPacket;

		uint32	dwPos = 0;
		int		iAllocationTries = 0;

		while (dwPos < dwSize && iAllocationTries < 3)
		{
			char	   *pBuffer2 = NULL;
			Packet	   *pPacket = NULL;
			uint32		dwNewSz = (dwSize - dwPos < MAXFRAGSIZE) ? dwSize - dwPos : MAXFRAGSIZE;

			try
			{
				pBuffer2 = new char[dwNewSz];
				pPacket = new Packet(pBuffer2, dwNewSz, (dwSize - dwPos) <= MAXFRAGSIZE, byteFilePriority, bFromPF);
			//	To optimize handling in case of failed packet allocation, let's copy the data after packet creation
				memcpy2(pBuffer2, m_pcBuffer + dwPos, dwNewSz);
			//	Put packet in temporary queue
				pSendQueue->push_back(pPacket);
			//	Update the data pointer for next fragment only if packet allocation was successful
			//	in this case we can retry to allocate the packet again
				dwPos += dwNewSz;
				iAllocationTries = 0;
			}
			catch (CMemoryException *error)
			{
				error->Delete();
				delete[] pBuffer2;
				iAllocationTries++;
			}
		}
		delete[] m_pcBuffer;

	//	If the split was failed due to problem with memory allocation report it back
		if (iAllocationTries != 0)
			return false;
	}
	else
	{
		m_blockSendQueue.push_back(pPacket);
	}

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SetUploadFileID(uchar *pbyteReqFileID)
{
	if (pbyteReqFileID != NULL)
		md4cpy(m_reqFileHash, pbyteReqFileID);
	else
		md4clr(m_reqFileHash);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::AddReqBlock(Requested_Block_Struct *pReqBlock)
{
	EMULE_TRY

	ReqBlockDeque::iterator		itBlock;
	Requested_Block_Struct	   *pTmpBlock;

	for (itBlock = m_doneBlocksList.begin(); itBlock != m_doneBlocksList.end(); itBlock++)
	{
		pTmpBlock = *itBlock;
		if ( (pTmpBlock != NULL) && (pReqBlock->qwStartOffset == pTmpBlock->qwStartOffset)
			&& (pReqBlock->qwEndOffset == pTmpBlock->qwEndOffset) )
		{
			return;
		}
	}

	for (itBlock = m_blockRequestsQueue.begin(); itBlock != m_blockRequestsQueue.end(); itBlock++)
	{
		pTmpBlock = *itBlock;
		if ( (pTmpBlock != NULL) && (pReqBlock->qwStartOffset == pTmpBlock->qwStartOffset)
			&& (pReqBlock->qwEndOffset == pTmpBlock->qwEndOffset) )
		{
			return;
		}
	}

	pTmpBlock = new Requested_Block_Struct;

	memcpy2(pTmpBlock, pReqBlock, sizeof(Requested_Block_Struct));
	m_blockRequestsQueue.push_back(pTmpBlock);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SetWaitStartTime()
{
	if (m_pCredits == NULL)
		return;

	m_pCredits->SetSecWaitStartTime(GetIP());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// m_dwWaitTime
uint32 CUpDownClient::GetWaitStartTime() const
{
	if (m_pCredits == NULL)
		return 0;

	uint32 dwSecureTime = m_pCredits->GetSecureWaitStartTime(GetIP());

	if (IsDownloading() && (dwSecureTime > m_dwUploadTime))
		dwSecureTime = m_dwUploadTime - 1;

	return dwSecureTime;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CUpDownClient::SendBlockData(uint32 dwMaxAmount, EnumBlockSendResult& eResult)
{
	eResult = BSR_FAILED_CLIENT_LEFT_UQ;

	uint32	dwTimePeriod, dwAmountTransferred = 0;
	DWORD	curTick = GetTickCount();

	try
	{
	//	Check for 'stuck' clients
		if ((curTick - GetLastGotULData() > 2 * 60 * 1000) && (GetUpStartTimeDelay() > 2 * 60 * 1000))
		{
		//	We were unable to transfer for 2 minutes, just remove the client from UL queue
		//	If client stays online, he will request file & be added to the queue
			if (g_App.m_pPrefs->IsClientTransferLogEnabled())
				AddLogLine(LOG_FL_DBG, _T("Client %s appears to be stuck, putting back on queue"), GetClientNameWithSoftware());
			g_App.m_pUploadQueue->RemoveFromUploadQueue(this, ETS_TIMEOUT);
			return 0;
		}

#ifdef OLD_SOCKETS_ENABLED
	//	Only feed this slot new data if it has been able to empty its buffer since we last offered data
		if (m_pRequestSocket != NULL)
		{
			if (m_pRequestSocket->IsBusy())
			{
			//	Check block queues to separate the active client from just invited to upload
				if (m_blockRequestsQueue.empty() && m_blockSendQueue.empty())
					eResult = BSR_FAILED_NO_REQUESTED_BLOCKS;
				else
					eResult = BSR_BUSY;
			}
			else
			{
			//	1) Measure only already transferred data! the speed should be calculated before data transfer, otherwise
			//		not yet transferred data from current iteration will lead to higher(incorrect) speed value
			//	2) Use socket status to find out the correct time for the speed measurements, because any calculation of the average inbetween
			//		will decrease the calculated value, what does not reflect real situation
				m_averageUDRList.push_front(m_dwTransferredUp);
				m_averageULTickList.push_front(curTick);

			//	Don't store too much statistics. maximal 40 sec ~ socket timeout
				while ((dwTimePeriod = (curTick - m_averageULTickList.back())) > CONNECTION_TIMEOUT)
				{
					m_averageUDRList.pop_back();
					m_averageULTickList.pop_back();
				}

			//	Calculate average data rate
				uint32 dwCurUpDataRate = 0;

			//	Leave this slot's speed as 0 unless there's a good chunk of stored statistics for it (at least 10 samples ~ 1 sec).
				if (dwTimePeriod > 1000)
				{
					dwCurUpDataRate = static_cast<uint32>((static_cast<double>(m_averageUDRList.front() - m_averageUDRList.back())) * 1000.0 / dwTimePeriod);
				}

			//	Assign & update only if values are different
				if (m_dwUpDataRate != dwCurUpDataRate)
				{
					m_dwUpDataRate = dwCurUpDataRate;
					g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.UpdateClient(this);
				}

				if (GetUploadState() == US_UPLOADING)
				{
					if (m_blockSendQueue.empty())
					{
					// If all packets were sent, try to create a new one
						EnumBlockPacketCreationResult	eBPCResult = CreateNextBlockPackage();

						if (eBPCResult != BPCR_OK)
						{
						//	If packets creation was failed, client can be removed without data loss
							if (eBPCResult == BPCR_FAILED_NO_REQUESTED_BLOCKS)
							{
							//	If client did not request data, keep him in the queue & give him a chance to request something
							//	The client will be removed from UL queue after 2 min.
								eResult = BSR_FAILED_NO_REQUESTED_BLOCKS;
								return 0;
							}
							else if (eBPCResult == BPCR_FAILED_BLOCKED_PART)
							{
								if (g_App.m_pPrefs->IsClientTransferLogEnabled())
									AddLogLine(LOG_FL_DBG, _T("Client %s left UL queue after receiving %u KB. Asked for new or blocked chunk/file"), GetClientNameWithSoftware(), GetSessionUp() / 1024);
								g_App.m_pUploadQueue->RemoveFromUploadQueue(this, ETS_BLOCKED_CHUNK);
							}
							else if (eBPCResult == BPCR_FAILED_FILE_ERROR)
							{
								if (g_App.m_pPrefs->IsClientTransferLogEnabled())
									AddLogLine(LOG_FL_DBG, _T("Client %s left UL queue after receiving %u KB. Asked for inaccessible data (file removed, wrong request, etc)"), GetClientNameWithSoftware(), GetSessionUp() / 1024);
								g_App.m_pUploadQueue->RemoveFromUploadQueue(this, ETS_FILE_ERROR);
							}
							else
							{
								if (g_App.m_pPrefs->IsClientTransferLogEnabled())
									AddLogLine(LOG_FL_DBG, _T("Client %s left UL queue after receiving %u KB. Upload timeout (low speed, no block request)"), GetClientNameWithSoftware(), GetSessionUp() / 1024);
								g_App.m_pUploadQueue->RemoveFromUploadQueue(this, ETS_TIMEOUT);
							}

						//	Notify the remote client about the end of the session
						//	Note (offical): 
						//		OP_OUTOFPARTREQS will tell the downloading client to go back to OnQueue..
						//		The main reason for this is that if we put the client back on queue and it goes
						//		back to the upload before the socket times out... We get a situation where the
						//		downloader thinks it already sent the requested blocks and the uploader thinks
						//		the downloader didn't send any request blocks. Then the connection times out..
						//	Note (eMule Plus):
						//		eMule Plus is also using OP_QUEUERANKING to detect the end of the session
							Packet	   *pCancelTransferPacket = new Packet(OP_OUTOFPARTREQS, 0);

							g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pCancelTransferPacket->m_dwSize);
							m_pRequestSocket->SendPacket(pCancelTransferPacket, true, true);

							g_App.m_pUploadQueue->AddClientToWaitingQueue(this, true);

							return 0;
						}
					}

				//	Feed data to this slot, one block at a time, as long as there's enough bandwidth left
				//	for the entire next block. There must also be blocks to send (not empty request queue)
					while (!m_blockSendQueue.empty())
					{
					//	Get next block and give it to the Request Socket for transfer
						Packet	   *pPacket = m_blockSendQueue.front();
						uint32		dwBlockSize = pPacket->GetRealPacketSize();

						if (dwBlockSize + dwAmountTransferred > dwMaxAmount)
							break;

						m_blockSendQueue.pop_front();

					//	Extended statistics information based on which client software and which port we sent this data to.
					//	This also updates the grand total for sent bytes, etc. And where this data came from.
					//	We have to call function before packet will be sent, otherwise packet will be deleted.
						g_App.m_pPrefs->Add2SessionUpTransferData( GetClientSoft(), pPacket->IsFromPF(),
							dwBlockSize, IsCommunity(), pPacket->GetFilePriority() );
						m_pRequestSocket->SendPacket(pPacket, true, false);
						pPacket = NULL;

					//	We've transferred some data. Store this in relevant statistics variables
						m_dwTransferredUp += dwBlockSize;
						dwAmountTransferred += dwBlockSize;
						m_pCredits->AddUploaded(dwBlockSize, this->GetIP());

					//	If we're out of prepared blocks, create some more
						if (m_blockSendQueue.empty())
							CreateNextBlockPackage();
					}

				//	Check result of the data transfer
					if (dwAmountTransferred == 0)
					{
					//	The size of the packet is larger than allowed bandwidth
						eResult = BSR_FAILED_NOT_ENOUGHT_BANDWIDTH;
					}
					else
					{
						if (!m_blockSendQueue.empty())
						//	If this slot wasn't given enough data to empty its request queue, it wants more data.
							eResult = BSR_OK_WANTS_MORE_BANDWIDTH;
						else
						//	Very rare case, the size of the packets fits the bandwidth
							eResult = BSR_OK;

					//	Remember when this connection was last fed some data, so we
					//	can make sure we give it data often enough to not time out.
						SetLastGotULData();
					}
				}
			}
		}
		else
		{
			AddLogLine(LOG_FL_DBG, _T("CUpDownClient::SendBlockData (socket doesn't exist)"));
			Disconnected(false);
			return 0;
		}
#endif //OLD_SOCKETS_ENABLED
	}
//	In case of exception, disconnect the client
	catch(CException *err)
	{
		g_App.m_pMDlg->AddBugReport(_T(__FUNCTION__), _T(__FILE__), __LINE__, GetErrorMessage(err));
		err->Delete();
		Disconnected(false);
	}
	catch(...)
	{
		Disconnected(false);
	}

	return dwAmountTransferred;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	FlushSendBlocks() sends the rest of the last block queued for upload to this client if it has been split
//	across multiple packets. Call this when you stop uploading or the request socket is not able to send.
void CUpDownClient::FlushSendBlocks()
{
	EMULE_TRY

	if ((m_pRequestSocket != NULL) && m_pRequestSocket->TruncateQueues())
		return;

	bool	bBreak = false;

#ifdef OLD_SOCKETS_ENABLED
	while ( !m_blockSendQueue.empty() && m_blockSendQueue.front()->IsSplit() &&
			m_pRequestSocket != NULL && m_pRequestSocket->IsConnected() && !bBreak )
	{
		Packet	   *pPacket = m_blockSendQueue.front();

		m_blockSendQueue.pop_front();
		bBreak = pPacket->IsLastSplit();

		g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
		m_pRequestSocket->SendPacket(pPacket, true, false);
	}
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendHashsetPacket(byte *pbytePacket)
{
	EMULE_TRY

	CKnownFile		*pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pbytePacket);

	if (pKnownFile == NULL)
	{
		pKnownFile = g_App.m_pDownloadQueue->GetFileByID(pbytePacket);

	// File isn't in the download list either or hashset wasn't received yet
		if ((pKnownFile == NULL) || (pKnownFile->GetHashSet() == NULL))
		{
			AddLogLine(LOG_FL_DBG | LOG_RGB_ERROR, _T("Client %s: requested file not found (hash: %s)"), GetClientNameWithSoftware(), HashToString(pbytePacket));
		//	Cause OP_HASHSETREQUEST can be abused for file-scanning
			UpdateFailedFileRequests();
			return;
		}
	//	Hybrids request hashset even when we don't have complete chunks
	//	If hashset is available, give them what they want to stop wave of requests
	}

	uint16		uPartCount = pKnownFile->GetHashCount();
	uint32		dwHashSetLength = 16 * uPartCount;
	CMemFile	pbyteFileData(16 + 2 + dwHashSetLength);

	pbyteFileData.Write(pKnownFile->GetFileHash(), 16);
	pbyteFileData.Write(&uPartCount, 2);

	if (uPartCount != 0 && pKnownFile->GetHashSet() != NULL)
		pbyteFileData.Write(pKnownFile->GetHashSet(), dwHashSetLength);

	Packet	   *pPacket = new Packet(&pbyteFileData);

	pPacket->m_eOpcode = OP_HASHSETANSWER;
	g_App.m_pUploadQueue->AddUpDataOverheadFileRequest(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED

	m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::ClearUploadBlockRequests()
{
	EMULE_TRY

	while (!m_blockRequestsQueue.empty())
	{
		delete m_blockRequestsQueue.front();
		m_blockRequestsQueue.pop_front();
	}

	while (!m_blockSendQueue.empty())
	{
		delete m_blockSendQueue.front();
		m_blockSendQueue.pop_front();
	}

	while (!m_doneBlocksList.empty())
	{
		delete m_doneBlocksList.front();
		m_doneBlocksList.pop_front();
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendRankingInfo()
{
	EMULE_TRY

#ifdef OLD_SOCKETS_ENABLED
	if (m_pRequestSocket == NULL)
		return;

#endif //OLD_SOCKETS_ENABLED
	if (!ExtProtocolAvailable())
		return;

	uint16 uRank = g_App.m_pUploadQueue->GetWaitingPosition(this);

	if (uRank == 0)
		return;

	Packet	   *pPacket = new Packet(OP_QUEUERANKING, 12, OP_EMULEPROT);

	memset(pPacket->m_pcBuffer, 0, 12);
	*reinterpret_cast<uint16 *>(pPacket->m_pcBuffer) = uRank;

	g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
	m_pRequestSocket->SendPacket(pPacket, true, true);
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendCommentInfo(CKnownFile *pKnownFile)
{
	EMULE_TRY

	if (!m_bCommentDirty || pKnownFile == NULL || !ExtProtocolAvailable() || m_byteAcceptCommentVer < 1)
		return;

	m_bCommentDirty = false;

	byte 		byteRating = static_cast<byte>(pKnownFile->GetFileRating());

	if (byteRating == PF_RATING_NONE && pKnownFile->GetFileComment().IsEmpty())
		return;

	CMemFile	packetStream(256);
	uint32		dwLength;
	CStringA	strEncoded;

	packetStream.Write(&byteRating, sizeof(byteRating));			// <rating 1>
	dwLength = Str2MB(m_eStrCodingFormat, &strEncoded, pKnownFile->GetFileComment());
	packetStream.Write(&dwLength, sizeof(dwLength));		// <commentlen 4>
	packetStream.Write(strEncoded.GetString(), dwLength);	// (<commentchars 1>)*commentlen

	Packet	   *pPacket = new Packet(&packetStream, OP_EMULEPROT);

	pPacket->m_eOpcode = OP_FILEDESC;
	g_App.m_pUploadQueue->AddUpDataOverheadOther(pPacket->m_dwSize);
#ifdef OLD_SOCKETS_ENABLED
	m_pRequestSocket->SendPacket(pPacket, true);
#endif //OLD_SOCKETS_ENABLED

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::AddRequestCount(uchar *pbyteFileId)
{
	EMULE_TRY

	uint32 dwCurTick = ::GetTickCount();

	for (POSITION pos = m_requestedFilesList.GetHeadPosition(); pos != NULL;)
	{
		Requested_File_Struct	   *pReqFile = static_cast<Requested_File_Struct*>(m_requestedFilesList.GetNext(pos));

		if (!md4cmp(pReqFile->m_fileHash, pbyteFileId))
		{
			if (dwCurTick - pReqFile->m_dwLastAskedTime < g_App.m_pPrefs->BadClientMinRequestTime())
			{
				if (GetDownloadState() != DS_DOWNLOADING)
					pReqFile->m_byteNumBadRequests++;
				if (pReqFile->m_byteNumBadRequests == g_App.m_pPrefs->BadClientMinRequestNum())
					Ban(BAN_CLIENT_AGGRESSIVE);
			}
			else if (pReqFile->m_byteNumBadRequests != 0)
				pReqFile->m_byteNumBadRequests--;
			pReqFile->m_dwLastAskedTime = dwCurTick;
			return;
		}
	}

//	If file was not found create a new file
	Requested_File_Struct	   *pNewFile = new Requested_File_Struct;

	md4cpy(pNewFile->m_fileHash, pbyteFileId);
	pNewFile->m_dwLastAskedTime = dwCurTick;
	pNewFile->m_byteNumBadRequests = 0;
	m_requestedFilesList.AddHead(pNewFile);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::UnBan()
{
	EMULE_TRY

	m_eBanState = BAN_CLIENT_NONE;
	m_dwBanTime = 0;

	SetWaitStartTime();
	g_App.m_pClientList->UpdateBanCounters();
	g_App.m_pMDlg->m_wndTransfer.UpdateUploadHeader();

	for (POSITION pos = m_requestedFilesList.GetHeadPosition(); pos != NULL;)
	{
		Requested_File_Struct	   *pReqFile = static_cast<Requested_File_Struct*>(m_requestedFilesList.GetNext(pos));

		pReqFile->m_byteNumBadRequests = 0;
		pReqFile->m_dwLastAskedTime = 0;
	}
	g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.UpdateClient(this);

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CUpDownClient::GetBanString()
{
	static const UINT s_adwBanString[BAN_CLIENT_LAST] =
	{
		IDS_NO,					//BAN_CLIENT_NONE
		IDS_BAN_AGGRESSIVECLI,	//BAN_CLIENT_AGGRESSIVE
		IDS_BAN_LEECHERSPAM,	//BAN_CLIENT_SPAMMING
		IDS_BAN_HASHIMP,		//BAN_CLIENT_USE_OUR_HASH
		IDS_BAN_HASHSTEAL,		//BAN_CLIENT_HASH_STEALER
		IDS_BAN_LEECHERADV		//BAN_CLIENT_KWOWN_LEECHER
	};

	return GetResString(s_adwBanString[m_eBanState]);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::Ban(EnumBanState eReason)
{
	EMULE_TRY

	if (!g_App.m_pPrefs->BanEnabled())
	{
	//	If the ban function is disabled, no need to ban a client (default is enabled!)
		m_eBanState = BAN_CLIENT_NONE;
		m_dwBanTime = 0;
		return;
	}

	EnumBanState	ePrevBanState = m_eBanState;

	m_eBanState = eReason;	// Set it before queue operations and GetBanString()
	if (m_eUploadState == US_UPLOADING)
	{
	//	Kick already downloading client back to waitingqueue
		AddLogLine(LOG_FL_DBG | LOG_RGB_DIMMED, _T("Client %s banned and kicked out of uploadqueue (%s)"),
						GetClientNameWithSoftware(), GetBanString());
		g_App.m_pUploadQueue->RemoveFromUploadQueue(this, ETS_BAN, true);
		g_App.m_pUploadQueue->AddClientToWaitingQueue(this, true);
	}

	if (ePrevBanState != BAN_CLIENT_NONE)
	{
	//	No need to ban a client (and send a message) again except for extending the bantime
		m_dwBanTime = ::GetTickCount();
		return;
	}

	if (g_App.m_pPrefs->CommunityNoBanEnabled() && g_App.m_pPrefs->CommunityEnabled() && IsCommunity())
	{
		m_eBanState = ePrevBanState;	// Undo
		return;
	}

	SetChatState(MS_NONE);
	m_dwBanTime = ::GetTickCount();

	if (!g_App.m_pPrefs->IsCMNotLog())
		AddLogLine(LOG_RGB_DIMMED, IDS_CLIENTBLOCKED, GetUserName(), GetBanString());

	g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.UpdateClient(this);
	g_App.m_pClientList->UpdateBanCounters();
	g_App.m_pMDlg->m_wndTransfer.UpdateUploadHeader();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SendBanMessage()
{
	const TCHAR	*pcMsg;
	
	switch (m_eBanState)
	{
		case BAN_CLIENT_NONE:
			return;

		case BAN_CLIENT_AGGRESSIVE:
			pcMsg =
				_T("AUTOMATED MESSAGE: You're being banned because your client is ")
				_T("too aggressive.")
				_T(" Get more info at http://emuleplus.info/ban/");
			break;
		case BAN_CLIENT_SPAMMING:
			pcMsg =
				_T("AUTOMATED MESSAGE: You're being banned because your client is ")
				_T("spamming.")
				_T(" Get more info at http://emuleplus.info/ban/");
			break;
		case BAN_CLIENT_USE_OUR_HASH:
		case BAN_CLIENT_HASH_STEALER:
		case BAN_CLIENT_KWOWN_LEECHER:
			pcMsg =
				_T("AUTOMATED MESSAGE: You're being banned because your client is ")
				_T("leeching!")
				_T(" Get more info at http://emuleplus.info/ban/");
			break;
	}

	g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.StartSession(this, false);
	g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.SendMessage(pcMsg);
	g_App.m_pMDlg->m_wndChat.m_ctlChatSelector.EndSession(this, false);

#if 1
	AddLogLine(LOG_FL_DBG | LOG_RGB_BLUE_GRAY, _T("Send a ban message to remote client %s (h=%s, bs=%x)"),
					GetClientNameWithSoftware(), HashToString(GetUserHash()), m_eBanState);
#endif
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CUpDownClient::GetRemoteRatio()
{
	EMULE_TRY

//	This calculation is not very accurate for several reasons:
//	- We do not know the file priority
//	- We do not save the client info if he has nothing uploaded to us
//	- Other factores like community etc

	double dblBaseValue = 100.0;

#ifdef OLD_SOCKETS_ENABLED
	if (g_App.m_pServerConnect->IsLowID())
		dblBaseValue *= 0.8;

#endif //OLD_SOCKETS_ENABLED
	CKnownFile	   *pKnownReqFile = g_App.m_pSharedFilesList->GetFileByID(m_reqFileHash);

	dblBaseValue *= GetRemoteBaseModifier();
//	Official client doesn't have SF/RF push
	if ((pKnownReqFile != NULL) && (GetClientSoft() == SO_PLUS))
	{
		dblBaseValue *= pKnownReqFile->GetSizeRatio();
		dblBaseValue *= pKnownReqFile->GetPopularityRatio();
	}
	return static_cast<uint32>(dblBaseValue);

	EMULE_CATCH

	return 100;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CUpDownClient::GetRemoteBaseModifier()
{
	EMULE_TRY

//	This calculation is not very accurate for some reasons...
//	Crashes can cause that either client can not write its client data.
//	This can cause differences of the client data at the two clients.
	if (m_pCredits == reinterpret_cast<CClientCredits*>(-1))
		m_pCredits = NULL;

	if (m_pCredits != NULL)
		return m_pCredits->GetScoreRatio(GetIP(), true);

	EMULE_CATCH

	return 1.0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CUpDownClient::GetCurrentlyUploadingPart()
{
	EMULE_TRY

	if (!m_doneBlocksList.empty())
	{
	//	Do we need to check upload state here?
		Requested_Block_Struct		*pFirstDoneBlock = m_doneBlocksList.front();

	//	Calculate which part the block is in
		return static_cast<uint32>(pFirstDoneBlock->qwStartOffset / PARTSIZE);
	}

	EMULE_CATCH

	return 0xFFFF;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::SetUploadState(EnumULQState eUploadState)
{
	if (eUploadState != m_eUploadState)
	{
		if (m_eUploadState == US_UPLOADING)
		{
		//	Update connection time not to drop sources accidently (we might transfer for hours)
		//	as this time is used to remove inactive LowID sources in PartFile:Process()
			m_dwEnteredConnectedState = ::GetTickCount();
		}
		if (eUploadState == US_UPLOADING)
		{
		//	Erase previous session upload rate statistics
			m_dwUpDataRate = 0;
			m_averageUDRList.clear();
			m_averageULTickList.clear();
		}
		m_eUploadState = eUploadState;
		g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.UpdateClient(this);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CUpDownClient::LeaveSourceLists()
{
	for (POSITION pos = m_requestedFilesList.GetHeadPosition(); pos != NULL;)
	{
		Requested_File_Struct	   *pReqFile = static_cast<Requested_File_Struct*>(m_requestedFilesList.GetNext(pos));
		CKnownFile *pSharedFile = g_App.m_pSharedFilesList->GetFileByID((uchar*)pReqFile->m_fileHash);

		if (pSharedFile != NULL)
			pSharedFile->RemoveClientFromSourceList(this);
	}
}
