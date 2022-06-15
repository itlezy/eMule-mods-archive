//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#include "StdAfx.h"
#include <share.h>
#include "DebugHelpers.h"
#include "emule.h"
#include "Voodoo.h"
#include "Preferences.h"
#include "Log.h"
#include "packets.h"
#include "downloadqueue.h"
#include "PartFile.h"
#include "Neo/LanCast/LanCast.h"
#include "eMuleDlg.h"
#include "downloadlistctrl.h"
#include "sharedfilelist.h"
#include "knownfilelist.h"
#include "transferwnd.h"
#include "Neo/functions.h"
#include "OpCodes.h"
#include "Neo/NeoOpCodes.h"
#include "UpDownClient.h"
#include "Uploadqueue.h"
// NEO: VOODOOs - [VoodooSearchForwarding]
#include "Sockets.h"
#include "SearchDlg.h"
#include "SearchResultsWnd.h"
#include "SearchParams.h"
#include "Statistics.h"
#include "Searchlist.h"
#include "Exceptions.h"
#include "Server.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "kademlia/kademlia/SearchManager.h"
#include "kademlia/kademlia/search.h"
// NEO: VOODOOs END
#include "Neo/BC\DownloadBandwidthThrottler.h"
#include "Neo/BC\UploadBandwidthThrottler.h"
#include "Neo/BC\BandwidthControl.h"
#include "clientlist.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --
#include "sharedfileswnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define VOODOO_TIMEOUT_TIME MIN2MS(5)
#define VOODOO_GOODBY_TIME SEC2MS(5)

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->

//////////////////////
// Hierarchy Exception

/* David:
* The hierarchy model is a securinty feature to protect the master and ensure the protocol will be respected.
*/
IMPLEMENT_DYNAMIC(CHierarchyExceptio, CException)

FileID::FileID(CSafeMemFile &packet, bool b64){
	packet.ReadHash16(Hash);
	if(b64)
		Size = packet.ReadUInt64();
	else
		Size = packet.ReadUInt32();
}

bool WriteFileID(CSafeMemFile &packet, CKnownFile* file, bool b64)
{
	if(!b64 && file->IsLargeFile()) // unfortunatly this client does not supports our file size
		return false;

	packet.WriteHash16(file->GetFileHash());
	if(b64)
		packet.WriteUInt64((uint64)file->GetFileSize());
	else
		packet.WriteUInt32((uint32)(uint64)file->GetFileSize());
	return true;
}

void WriteFileID(CSafeMemFile &packet, FileID& file, bool b64)
{
	ASSERT(b64 || file.Size < OLD_MAX_EMULE_FILE_SIZE);
	packet.WriteHash16(file.Hash);
	if(b64)
		packet.WriteUInt64(file.Size);
	else
		packet.WriteUInt32((uint32)file.Size);
}

//////////////////////
// CMasterDatas

CMasterDatas::CMasterDatas()
{
	gaplist = NULL;
}

CMasterDatas::~CMasterDatas()
{
	if(gaplist){
		while (!gaplist->IsEmpty())
			delete gaplist->RemoveHead();
		delete gaplist;
	}
}

bool CMasterDatas::IsComplete(uint64 start, uint64 end) const
{
	ASSERT( start <= end );
	if(gaplist == NULL)
		return true;

	//if (end >= m_nFileSize)
	//	end = m_nFileSize-(uint64)1;
	for (POSITION pos = gaplist->GetHeadPosition();pos != 0;)
	{
		const Gap_Struct* cur_gap = gaplist->GetNext(pos);
		if (   (cur_gap->start >= start          && cur_gap->end   <= end)
			|| (cur_gap->start >= start          && cur_gap->start <= end)
			|| (cur_gap->end   <= end            && cur_gap->end   >= start)
			|| (start          >= cur_gap->start && end            <= cur_gap->end)
		   )
		{
			return false;	
		}
	}
	return true;
}


////////////////////////////////////////////////////////////////////////////////
// CVoodooSocket
//

IMPLEMENT_DYNCREATE(CVoodooSocket, CEMSocket)

CVoodooSocket::CVoodooSocket()
{
	m_StreamCryptState = ECS_NONE; // Dissable obfuscation voodoo does not support it

	ResetTimeOutTimer();
	theApp.voodoo->AddSocket(this);
	deletethis = false;
	deltimer = 0;
	clientIP = 0;
	clientPort = 0;

	m_uAction = VA_NONE;
	m_bIsMaster = false;
	m_bIsSlave = false;

	m_uVoodooVersion = 0;
	m_u64FileSizeSupport = 0;
	m_uCorruptionHandling = 0;
	m_uAdvDownloadSync = 0;

	m_uSupportsStatistics = 0;
	m_uVoodooSearch = 0; // NEO: VOODOOs - [VoodooSearchForwarding]
	m_uVoodooXS = 0; // NEO: VOODOOx - [VoodooSourceExchange]
	m_uNeoFilePrefs = 0; // NEO: VOODOOn - [VoodooForNeo]
	m_uNeoCommandVersion = 0; // NEO: VOODOOn - [VoodooForNeo]

	m_sName.Empty();
	m_uType = CT_UNKNOWN;
	m_nVoodoPort = 0;
	m_uPerm = 0;
	m_sSpell.Empty();

	m_nPortED2K = 0;

	m_bHaveMoreResults = false; // NEO: VOODOOs - [VoodooSearchForwarding]

	// stats
	m_uLastStatUpdate = ::GetTickCount();
}

void CVoodooSocket::SetOnLan() 
{ 
	theApp.uploadBandwidthThrottler->RemoveFromAllQueues(this);
	theApp.uploadBandwidthThrottler->QueueForLanPacket(this);

	if(NeoPrefs.UseDownloadBandwidthThrottler()){
		theApp.downloadBandwidthThrottler->RemoveFromAllQueues(this);
		theApp.downloadBandwidthThrottler->QueueForLanPacket(this);
		theApp.downloadqueue->AddToProcessQueue(this);
	}
}

void CVoodooSocket::ConfigSocket()
{
	if(NeoPrefs.IsSetLanUploadBuffer()){
		int sendBuff = NeoPrefs.GetLanUploadBufferSize();
		SetSockOpt(SO_SNDBUF, &sendBuff, sizeof(sendBuff), SOL_SOCKET);
	}

	if(NeoPrefs.IsSetLanDownloadBuffer()){
		int recvBuff = NeoPrefs.GetLanDownloadBufferSize();
		SetSockOpt(SO_RCVBUF, &recvBuff, sizeof(recvBuff), SOL_SOCKET);
	}

	if(NeoPrefs.IsTCPDisableNagle()){
		BOOL noDelay = true;
		SetSockOpt(TCP_NODELAY, &noDelay, sizeof(noDelay), IPPROTO_TCP);
	}
}

CVoodooSocket::~CVoodooSocket()
{
	theApp.voodoo->RemoveSocket(this);
	//CleanUpUploadBuffer(true); // NO RBT

	sFileError* FileErr;
	CCKey bufKey;
	for (POSITION pos = m_FileErrors.GetStartPosition();pos != 0;){
		m_FileErrors.GetNextAssoc(pos,bufKey,FileErr);
		delete FileErr;
	}
	m_FileErrors.RemoveAll();
}

void CVoodooSocket::SendPacket(CSafeMemFile &data, uint8 opcode, bool /*bControlPacket*/)
{
	if(data.GetLength() > 1536*1024){
		DebugLog(LOG_ERROR,_T("VOODOO: Tryed to send a to large packet packet dropped"));
		ASSERT(0);
		return; // Drop packet to avoid disconnect
	}
	// Note: All voodoo packets are control packets...
	Packet* packet = new Packet(&data,OP_VOODOOPROT);
	packet->opcode = opcode;
	CEMSocket::SendPacket(packet,true);
}

void CVoodooSocket::OnClose(int nErrorCode){
	ASSERT (theApp.voodoo->IsValidSocket(this));
	CEMSocket::OnClose(nErrorCode);

	LPCTSTR pszReason;
	CString* pstrReason = NULL;
	if (nErrorCode == 0)
		pszReason = _T("Close");
	else if (thePrefs.GetVerbose()){
		pstrReason = new CString;
		*pstrReason = GetErrorMessage(nErrorCode, 1);
		pszReason = *pstrReason;
	}
	else
		pszReason = NULL;
	Disconnect(pszReason);
	delete pstrReason;
}

void CVoodooSocket::OnError(int nErrorCode)
{
	CString strTCPError;
	if (thePrefs.GetVerbose())
	{
		if (nErrorCode == ERR_WRONGHEADER)
			strTCPError = _T("Error: Wrong header");
		else if (nErrorCode == ERR_TOOBIG)
			strTCPError = _T("Error: Too much data sent");
		else
			strTCPError = GetErrorMessage(nErrorCode);
		DebugLogWarning(_T("Client TCP socket: %s; %s"), strTCPError, DbgGetClientInfo());
	}
	Disconnect(strTCPError);
}

void CVoodooSocket::Disconnect(LPCTSTR pszReason){
	AsyncSelect(0);
	byConnected = ES_DISCONNECTED;

	AddDebugLogLine(DLP_VERYLOW, true,_T("Disconnecting voodoo client: %s; reason: %s"), DbgGetClientInfo(), pszReason);

	theApp.voodoo->CleanUpKnown(this);

	if(m_bIsMaster || m_bIsSlave){
		ModLog(LOG_WARNING,GetResString(IDS_X_VOODOO_CLIENT_CONENCT_LOST), GetClientDesc(), 
			clientAddress.IsEmpty() ? ipstr(clientIP) : clientAddress, clientPort);

		DetachFiles();

	}else if(m_uAction != VA_NONE){
		ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_CLIENT_CONENCT_FAILED), GetClientDesc(m_uAction), 
			clientAddress.IsEmpty() ? ipstr(clientIP) : clientAddress, clientPort);
	}else{
		ModLog(GetResString(IDS_X_VOODOO_CLIENT_CONENCT_CLOSED), GetClientDesc(), 
			clientAddress.IsEmpty() ? ipstr(clientIP) : clientAddress, clientPort);
	}

	Safe_Delete();
};

bool CVoodooSocket::CheckTimeOut()
{
	if (::GetTickCount() - timeout_timer > GetTimeOut()){
		timeout_timer = ::GetTickCount();
		CString str;
		str.Format(_T("Timeout"));
		Disconnect(str);
		return true;
	}else if((IsMaster() || IsSlave()) && ::GetTickCount() - timeout_timer > GetTimeOut()/2){
		SendPing(); // to keep connection alive
	}
	return false;
}

void CVoodooSocket::ResetTimeOutTimer(){
	timeout_timer = ::GetTickCount();
}

void CVoodooSocket::Delete_Timed(){
	// it seems that MFC Sockets call socketfunctions after they are deleted, even if the socket is closed
	// and select(0) is set. So we need to wait some time to make sure this doesn't happens
	if (::GetTickCount() - deltimer > 10000)
		delete this;
}

void CVoodooSocket::Safe_Delete()
{
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler]
	SetPriorityReceive(false);
#endif // NEO_DBT // NEO: NDBT END
#ifdef NEO_UBT // NEO: NUBT - [NeoUploadBandwidthThrottler]
	SetPrioritySend(false);
#endif // NEO_UBT // NEO: NUBT END
	ASSERT (theApp.voodoo->IsValidSocket(this));
	AsyncSelect(0);
	deltimer = ::GetTickCount();
	if (m_SocketData.hSocket != INVALID_SOCKET)
		ShutDown(SD_BOTH);
	byConnected = ES_DISCONNECTED;
	deletethis = true;
}

void CVoodooSocket::OnSend(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnSend(nErrorCode);
}

void CVoodooSocket::OnReceive(int nErrorCode)
{
	ResetTimeOutTimer();
	CEMSocket::OnReceive(nErrorCode);
}

void CVoodooSocket::OnConnect(int nErrorCode)
{
	CEMSocket::OnConnect(nErrorCode);
	if (nErrorCode)
	{
	    CString strTCPError;
		if (thePrefs.GetVerbose())
		{
		    strTCPError = GetErrorMessage(nErrorCode, 1);
		    if (nErrorCode != WSAECONNREFUSED && nErrorCode != WSAETIMEDOUT)
			    DebugLogError(_T("Client TCP socket (OnConnect): %s; %s"), strTCPError, DbgGetClientInfo());
		}
		Disconnect(strTCPError);
	}
	else
	{
		//This socket may have been delayed by SP2 protection, lets make sure it doesn't time out instantly.
		ResetTimeOutTimer();

		ConfigSocket();

		SOCKADDR_IN sockAddr = {0};
		int nSockAddrLen = sizeof(sockAddr);
		GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
		if (sockAddr.sin_addr.S_un.S_addr != 0){
			clientIP = sockAddr.sin_addr.S_un.S_addr;
		}
	}
}



////////////////////////
// Connection begin

void CVoodooSocket::SendVoodooHello(uint8 uHello)
{
	CSafeMemFile data(128);
	data.WriteUInt8(VOODOOVERSION);

	data.WriteUInt8(uHello);

	if(uHello != VH_QUERY){
		uint32 uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)data.GetPosition();
		data.WriteUInt32(uTagCount);
	
		CTag tagName(VT_NAME,thePrefs.GetUserNick());
		tagName.WriteTagToFile(&data);
		uTagCount++;

		CTag tagType(VT_TYPE,CT_ED2K);
		tagType.WriteTagToFile(&data);
		uTagCount++;

		CTag tagPort(VT_PORT,NeoPrefs.IsVoodooAllowed() ?  NeoPrefs.GetVoodooPort() : 0);
		tagPort.WriteTagToFile(&data);
		uTagCount++;

		CTag tagED2K(VT_ED2K,thePrefs.GetPort());
		tagED2K.WriteTagToFile(&data);
		uTagCount++;

		CTag tagPerm(VT_PERM,NeoPrefs.IsSlaveAllowed() ? VA_SLAVE : 0 | NeoPrefs.IsSlaveHosting() ? VA_MASTER : 0);
		tagPerm.WriteTagToFile(&data);
		uTagCount++;

		const UINT uSupportsStatistics	= 2;
		const UINT u64FileSizeSupport	= (OLD_MAX_EMULE_FILE_SIZE < MAX_EMULE_FILE_SIZE)	? 1 : 0;
		const UINT uCorruptionHandling	= 2;
		const UINT uAdvDownloadSync		= 3;
		UINT uFeatures = 
			//(u						<< 24) |
			//(u						<< 16) |
			//(u						<< 12) | // 4 bits
			(uSupportsStatistics		<<  9) | // 3 bits // moved from VT_FEATURES_EX as it is a general feature and works with any cleint software not only ed2k
			(uAdvDownloadSync			<<  5) | // 4 bits
			(uCorruptionHandling		<<  1) | // 4 bits
			(u64FileSizeSupport			<<  0) ; // 1 bit
		CTag tagAdvFeatures(VT_FEATURES,uFeatures);
		tagAdvFeatures.WriteTagToFile(&data);
		uTagCount++;

		
		const UINT uNeoCommandVersion	= 1; // NEO: VOODOOn - [VoodooForNeo]
		const UINT uNeoFilePrefs		= 1; // NEO: VOODOOn - [VoodooForNeo]
		const UINT uVoodooXS			= 1; // NEO: VOODOOx - [VoodooSourceExchange]
		const UINT uVoodooSearch		= 1; // NEO: VOODOOs - [VoodooSearchForwarding]
		UINT uFeaturesEd2k = 
			//(u						<< 24) |
			//(u						<< 16) |
			//(u						<< 11) | // 4 bits
			(uNeoCommandVersion			<<  9) | // 3 bits
			(uNeoFilePrefs				<<  6) | // 3 bits
			(uVoodooXS					<<  3) | // 3 bits
			(uVoodooSearch				<<  0) ; // 3 bits
		CTag tagFeatures(VT_FEATURES_ED2K,uFeaturesEd2k);
		tagFeatures.WriteTagToFile(&data);
		uTagCount++;

		data.Seek(uTagCountFilePos, CFile::begin);
		data.WriteUInt32(uTagCount);
		data.Seek(0, CFile::end);
	}

	SendPacket(data,OP_VOODOO_HELLO);
}


void CVoodooSocket::RecivedVoodooHello(CSafeMemFile &packet)
{
	m_uVoodooVersion = packet.ReadUInt8();
	
	uint8 uHello = packet.ReadUInt8();
	if(uHello == VH_QUERY){
		SendVoodooHello(VH_ANSWER);
		return;
	}

	uint32 tagcount = packet.ReadUInt32();
	for (uint32 i = 0;i < tagcount; i++)
	{
		CTag temptag(&packet, true);
		switch (temptag.GetNameID())
		{
			case VT_NAME:
				ASSERT (temptag.IsStr());
				m_sName = temptag.GetStr();
				break;

			case VT_TYPE:
				ASSERT (temptag.IsInt());
				m_uType = (uint8)temptag.GetInt();
				break;

			case VT_PORT:
				ASSERT (temptag.IsInt());
				m_nVoodoPort = (uint16)temptag.GetInt();
				break;

			case VT_ED2K:
				ASSERT (temptag.IsInt());
				m_nPortED2K = (uint16)temptag.GetInt();
				break;

			case VT_PERM:
				ASSERT (temptag.IsInt());
				m_uPerm = (uint8)temptag.GetInt();
				break;

			case VT_FEATURES:{
				ASSERT (temptag.IsInt());
				UINT uFeatures = temptag.GetInt();
				//m_u					= (uFeatures >> 24)
				//m_u					= (uFeatures >> 16)
				//m_u					= (uFeatures >> 12) & 0x0F; // 4 bit
				m_uSupportsStatistics	= (uFeatures >> 9 ) & 0x07; // 3 bits
				m_uAdvDownloadSync		= (uFeatures >> 5 ) & 0x0F; // 4 bits
				m_uCorruptionHandling	= (uFeatures >> 3 ) & 0x0F; // 4 bits
				m_u64FileSizeSupport	= (uFeatures >> 0 ) & 0x01; // 1 bit
				break;
			}

			case VT_FEATURES_ED2K:{
				ASSERT (temptag.IsInt());
				UINT uFeaturesEd2k = temptag.GetInt();
				//m_u					= (uFeaturesEd2k >> 24)
				//m_u					= (uFeaturesEd2k >> 16)
				//m_u					= (uFeaturesEd2k >> 11 ) // 4 bits
				m_uNeoCommandVersion	= (uFeaturesEd2k >> 9 ) & 0x07; // 3 bits // NEO: VOODOOn - [VoodooForNeo]
				m_uNeoFilePrefs			= (uFeaturesEd2k >> 6 ) & 0x07; // 3 bits // NEO: VOODOOn - [VoodooForNeo]
				m_uVoodooXS				= (uFeaturesEd2k >> 3 ) & 0x07; // 3 bits // NEO: VOODOOx - [VoodooSourceExchange]
				m_uVoodooSearch			= (uFeaturesEd2k >> 0 ) & 0x07; // 3 bits // NEO: VOODOOs - [VoodooSearchForwarding]
				break;
			}

			// Old
			case VT_NEO:
			case VT_FEATURES_EX:
				break;
			default:
				DebugLogWarning(_T("Unknown Tag Voodoo Hello Packet: %s"), temptag.GetFullInfo());
		}
	}

	if(uHello == VH_HELLO)
		SendVoodooHello(VH_ANSWER);

	ConnectionEstablished();
}

void CVoodooSocket::ConnectionEstablished()
{
	SetPort(m_nVoodoPort);
	ModLog(GetResString(IDS_X_VOODOO_CLIENT_CONNECTED), clientAddress.IsEmpty() ? ipstr(clientIP) : clientAddress, clientPort);

	//if(m_uVoodooVersion <= VOODOOVERSION_OLD){
	//	ModLog(LOG_WARNING,GetResString(IDS_X_VOODOO_TO_OLD),DbgGetClientInfo());
	//	Disconnect(_T("To old version"));
	//	return;
	//}

	if(!theApp.voodoo->AddKnown(this)){
		SendGoodBy(true);
		Disconnect(_T("Blocked clinet"));
	}else if(m_uAction == VA_QUERY)
		SetTimeOut(VOODOO_GOODBY_TIME); // let the socket timeout
	else if(!IsUsable() && (m_uAction & VA_SLAVE))
		ModLog(GetResString(IDS_X_VOODOO_NOT_SLAVE),DbgGetClientInfo());
	else if(m_uAction & VA_PARTNER) // true for VA_SLAVE and VA_MASTER
		SendSpell();
}

/////////////////////////
// Spell

void CVoodooSocket::SendSpell()
{
	CSafeMemFile data(128);
	uint8 uAction = m_uAction;
	if(m_uVoodooVersion < 0x02){
		if(uAction == VA_QUERY)
			uAction = 0x03;
		else if(uAction == VA_PARTNER)
			uAction = VA_SLAVE;
	}
	data.WriteUInt8(uAction);
	data.WriteString(m_sSpell.IsEmpty() ? NeoPrefs.GetVoodooSpell() : m_sSpell,utf8strNone/*utf8strRaw*/);
	SendPacket(data,OP_SPELL);
}

void CVoodooSocket::SpellRecived(CSafeMemFile &packet)
{
	uint8 uAction = packet.ReadUInt8();
	CString sSpell = packet.ReadString(false/*true*/);
	bool bResult = (sSpell.Compare(NeoPrefs.GetVoodooSpell()) == 0);

	if(bResult){
		if(m_uVoodooVersion < 0x02){
			if(uAction == 0x03)
				uAction = VA_QUERY;
		}

		uint8 uResult = VA_NONE;

		if((uAction & VA_SLAVE) && NeoPrefs.IsSlaveAllowed()){
			m_bIsMaster = true;
			ModLog(GetResString(IDS_X_ACCEPT_SPELL),GetResString(IDS_X_VOODOO_MASTER),DbgGetClientInfo());
			uResult |= VA_SLAVE;
		}

		if((uAction & VA_MASTER) && NeoPrefs.IsSlaveHosting()){
			m_bIsSlave = true;
			ModLog(GetResString(IDS_X_ACCEPT_SPELL),GetResString(IDS_X_VOODOO_SLAVE),DbgGetClientInfo());
			uResult |= VA_MASTER;
		}

		SendSpellResult(uResult);

		if(IsSlave()){
			if(m_uVoodooVersion >= 0x02){
				SendDownloadQueueV2();
				SendSharedFilesListV2();
			}else{
				SendDownloadQueue();
				SendSharedFilesList();			
			}
		}
	}else
		SendSpellResult(VA_NONE);
}

void CVoodooSocket::SendSpellResult(uint8 uResult)
{
	CSafeMemFile data(128);
	data.WriteUInt8(uResult);
	SendPacket(data,OP_SPELL_RESULT);
}

void CVoodooSocket::SpellResultRecived(CSafeMemFile &packet)
{
	uint8 uResult = packet.ReadUInt8();
	if(uResult){
		ModLog(uResult == m_uAction ? LOG_SUCCESS : LOG_WARNING, // we may only partialy succeded !!!
			GetResString(IDS_X_SPELL_ACCEPTED), GetClientDesc(uResult), DbgGetClientInfo());

		if(uResult & VA_SLAVE){
			m_bIsSlave = true;

			// NEO: VOODOOn - [VoodooForNeo]
			if(GetNeoFilePrefsVersion() > 0){
				SendNeoPreferences(CFP_GLOBAL);
				for (int i = 0; i < thePrefs.GetFullCatCount(); i++)
					SendNeoPreferences(CFP_CATEGORY,NULL,i);
			}
			// NEO: VOODOOn END

			// the new sync metod assumes that the files are virtual (only sensefull with the same software)
			if(m_uVoodooVersion >= 0x02 && IsED2K()){ 
				SendDownloadQueueV2();
				SendSharedFilesListV2();
			}else{
				SendDownloadQueue();
				SendSharedFilesList();			
			}
		}

		if(uResult & VA_MASTER){
			m_bIsMaster = true;
		}
	}

	m_uAction = VA_NONE;
}

////////////////////////////////////////
// Ping Pong

void CVoodooSocket::SendPing(uint8 uPong)
{
	CSafeMemFile data(128);
	data.WriteUInt8(uPong);
	SendPacket(data,OP_VOODOO_PING);
}

void CVoodooSocket::RecivedPing(CSafeMemFile &packet)
{
	uint8 uPong = packet.ReadUInt8();

	if(uPong == FALSE)
		SendPing(TRUE);

	ResetTimeOutTimer();
}

////////////////////////////////////////
// Disconnect

void CVoodooSocket::SendGoodBy(bool bFlash)
{
	if(m_uVoodooVersion >= 0x02){
		CEMSocket::SendPacket(new Packet(OP_VOODOO_GOODBY,0,OP_VOODOOPROT));
		if(bFlash) // do we need to flash the buffered packet
			CEMSocket::Send(0x7fffffff,MAXFRAGSIZE);
	}

	PrepDisconnect();
}

void CVoodooSocket::PrepDisconnect()
{
	SetAction(VA_NONE);
	DetachFiles();
	SetTimeOut(VOODOO_GOODBY_TIME);
}

void CVoodooSocket::DetachFiles()
{
	if(m_bIsMaster){
		CTypedPtrList<CPtrList, CPartFile*>* filelist = theApp.downloadqueue->GetFileList();
		for (POSITION pos = filelist->GetHeadPosition(); pos != 0; ){
			CPartFile* pFile = filelist->GetNext(pos);
			if(pFile->IsVoodooFile() && pFile->GetMasterDatas(this)){
				if(NeoPrefs.UseVirtualVoodooFiles() == TRUE)
					pFile->DeleteFile(false,false); // voodoo files are now completly virtual // Disable resort
				else
					pFile->RemoveMaster(this);
			}
		}

		CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*>* filemap = theApp.sharedfiles->GetFileMap();
		CKnownFile* kFile;
		CCKey bufKey;
		for (POSITION pos = filemap->GetStartPosition();pos != 0;){
			filemap->GetNextAssoc(pos,bufKey,kFile);
			if(kFile->IsVoodooFile() && kFile->GetMasterDatas(this))
				kFile->RemoveMaster(this);
		}
	}

	m_bIsMaster = false;
	m_bIsSlave = false;
}

///////////////////////////////////////////
// Snychronizing download queue

void CVoodooSocket::SendDownloadQueue()
{
	CSafeMemFile list(128);

	uint32 RecordsNumber = 0;
	ULONG RecordsNumberFilePos = (ULONG)list.GetPosition();
	list.WriteUInt32(RecordsNumber);

	CTypedPtrList<CPtrList, CPartFile*>* filelist = theApp.downloadqueue->GetFileList();
	for (POSITION pos = filelist->GetHeadPosition(); pos != 0; ){
		CPartFile* pFile = filelist->GetNext(pos);
		if(pFile->IsVoodooFile() || !pFile->KnownPrefs->IsEnableVoodoo())
			continue;
		if(!WriteFileID(list,pFile,Use64Size()))
			continue;

		uint8 state;
		if(pFile->IsStopped())
			state = INST_STOP;
		else if(pFile->IsPaused())
			state = INST_PAUSE;
		else
			state = INST_RESUME;
		
		list.WriteUInt8(state);
		RecordsNumber++;
	}

	list.Seek(RecordsNumberFilePos, CFile::begin);
	list.WriteUInt32(RecordsNumber);
	list.Seek(0, CFile::end);

	SendPacket(list,OP_DOWNLOAD_QUEUE);
}

void CVoodooSocket::SynchronizeDownloadQueue(CSafeMemFile &packet)
{
	uint32 RecordsNumber = packet.ReadUInt32();
	for (uint32 i = 0; i < RecordsNumber; i++)
	{
		CPartFile* pFile = GetDownloadingFile(packet, true, false);
		uint8 state = packet.ReadUInt8();
		if(!pFile)
			continue;

		//pFile->AddMaster(this); // is already done by GetDownloadingFile
		RequestGapList(pFile);
		ExecuteDownloadInstruction(pFile, state);
	}
}

void CVoodooSocket::SendDownloadQueueV2()
{
	CTypedPtrList<CPtrList, CPartFile*>* filelist = theApp.downloadqueue->GetFileList();
	for (POSITION pos = filelist->GetHeadPosition(); pos != 0; ){
		CPartFile* pFile = filelist->GetNext(pos);
		if(pFile->IsVoodooFile() || !pFile->KnownPrefs->IsEnableVoodoo())
			continue;

		uint8 state;
		if(pFile->IsStopped())
			state = INST_STOP;
		else if(pFile->IsPaused())
			state = INST_PAUSE;
		else
			state = INST_RESUME;

		SendDownloadInstruction(pFile,state);
	}
}

////////////////////////////////////
// Synchronize Files

void CVoodooSocket::SendFileUnavalible(FileID &fileID, uint8 uErr)
{
	CSafeMemFile data(128);
	WriteFileID(data,fileID,Use64Size());
	data.WriteUInt8(uErr);
	SendPacket(data,OP_FILE_UNAVALIBLE);
}

void CVoodooSocket::FileUnavalibleRecived(CSafeMemFile &packet)
{
	FileID fileID(packet,Use64Size());
	CPartFile* pFile = theApp.downloadqueue->GetFileByID(fileID.Hash);
	CKnownFile* kFile = pFile ? pFile : theApp.knownfiles->FindKnownFileByID(fileID.Hash);

	if(!kFile)
		throw StrLine(GetResString(IDS_X_VOODOO_FILE_FAILED),md4str(fileID.Hash),DbgGetClientInfo());
	else if(kFile->GetFileSize() != fileID.Size)
		throw StrLine(GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());

	uint8 uErr = packet.ReadUInt8();
	switch(uErr){
		case RET_NONE:
			// Note: the first voodoo version used this opcode for an other purpose, but its not longer supported
			return;
		case RET_UNKNOWN:
			if(!IsSlave()) throw new CHierarchyExceptio(OP_FILE_UNAVALIBLE,RET_UNKNOWN);

			if(m_uType == CT_ED2K){
				//ModLog(GetResString(IDS_X_VOODOO_FILE_ORDER),DbgGetClientInfo(),kFile->GetFileName());
				if(pFile)
					SendDownloadOrder(pFile); 
				else
					SendShareOrder(kFile); 

				// NEO: VOODOOn - [VoodooForNeo]
				if(GetNeoFilePrefsVersion() > 0)
					SendNeoPreferences(CFP_FILE, kFile);
				// NEO: VOODOOn END

				// NEO: VOODOOx - [VoodooSourceExchange]
				if(pFile && pFile->IsVoodooXS(true) && GetVoodooXSVersion() > 0)
					SendSourceList(pFile);
				// NEO: VOODOOx END

				return; // don't log in FileErrors Map becouse it isn't an error
			}else
				ModLog(GetResString(IDS_X_VOODOO_UNKNOWN_FILE),DbgGetClientInfo(),kFile->GetFileName());
			break;
		case RET_UNAVALIBLY:
			if(!IsMaster()) throw new CHierarchyExceptio(OP_FILE_UNAVALIBLE,RET_UNAVALIBLY);

			// That's the only relevant error code comming from a master and if we get it we remove the errourness voodoo file
			ModLog(GetResString(IDS_X_VOODOO_FILE_MISSING),kFile->GetFileName());
			if(kFile->IsVoodooFile()){
				kFile->RemoveMaster(this); // remove master
				if(pFile && !pFile->HaveMasters()) // if this was the last master remove the file
					pFile->DeleteFile(); // CleanUp, delete file

				return; // don't log in FileErrors Map
			}else
				throw StrLine(GetResString(IDS_X_NOT_VOODOO_FILE),DbgGetClientInfo(),kFile->GetFileName()); // Shouldn't happen
			break;
		case RET_REAL:
			if(!IsSlave()) throw new CHierarchyExceptio(OP_FILE_UNAVALIBLE,RET_REAL);
			ModLog(GetResString(IDS_X_VOODOO_FILE_REAL_ERROR), kFile->GetFileName(),DbgGetClientInfo());
			break;
		case RET_NEW:
			if(!IsSlave()) throw new CHierarchyExceptio(OP_FILE_UNAVALIBLE,RET_NEW);
			ModLog(GetResString(IDS_X_VOODOO_FILE_NEW_ERROR), kFile->GetFileName(),DbgGetClientInfo());
			break;
		case RET_BADSIZE:
			if(!IsSlave() && !IsMaster()) throw new CHierarchyExceptio(OP_FILE_UNAVALIBLE,RET_BADSIZE);
			ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_REMOTE_FAILED), kFile->GetFileName(),DbgGetClientInfo());
			break;
		case RET_EXIST:
			if(!IsSlave()) throw new CHierarchyExceptio(OP_FILE_UNAVALIBLE,RET_EXIST);
			ModLog(LOG_WARNING,GetResString(IDS_X_VOODOO_FILE_EXIST_FAILED), kFile->GetFileName(),DbgGetClientInfo());
			break;
		case RET_COMPLETE:
			if(!IsSlave()) throw new CHierarchyExceptio(OP_FILE_UNAVALIBLE,RET_COMPLETE);
			ModLog(LOG_WARNING,GetResString(IDS_X_VOODOO_COMPLETE_FAILED), kFile->GetFileName(),DbgGetClientInfo());
			break;
		default:
			ModLog(GetResString(IDS_X_VOODOO_FILE_UNAVALIBLE_ERROR), kFile->GetFileName(),
				GetClientDesc(), DbgGetClientInfo());
	}

	// Note: the only loged error that comes from a master is RET_BADSIZE and it's a very very bad one
	CCKey tmpkey(fileID.Hash);
	sFileError* FileErr;
	if(!m_FileErrors.Lookup(tmpkey,FileErr))
	{
		sFileError* FileErr = new sFileError(fileID, uErr);
		CCKey newkey(FileErr->ID.Hash);
		m_FileErrors.SetAt(newkey, FileErr);
	}
	else
	{
		FileErr->Error = uErr;
		ASSERT(0); // we shouldn't get a secund error
	}
}

bool CVoodooSocket::IsFileErr(CKnownFile* kFile)
{
	CCKey key(kFile->GetFileHash());
	sFileError* FileErr;
	return m_FileErrors.Lookup(key,FileErr) && FileErr->Error != RET_NONE; // check flag state just in case
}

///////////////////////////////////////////
// Snychronizing shared file

void CVoodooSocket::SendSharedFilesList()
{
	CSafeMemFile list(128);

	uint32 RecordsNumber = 0;
	ULONG RecordsNumberFilePos = (ULONG)list.GetPosition();
	list.WriteUInt32(RecordsNumber);

	CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*>* filemap = theApp.sharedfiles->GetFileMap();
	CKnownFile* kFile;
	CCKey bufKey;
	for (POSITION pos = filemap->GetStartPosition();pos != 0;){
		filemap->GetNextAssoc(pos,bufKey,kFile);
		if(kFile->IsVoodooFile())
			continue;
		if(kFile->IsPartFile())
			continue;
		if(!WriteFileID(list,kFile,Use64Size()))
			continue;
		RecordsNumber++;
	}

	list.Seek(RecordsNumberFilePos, CFile::begin);
	list.WriteUInt32(RecordsNumber);
	list.Seek(0, CFile::end);

	SendPacket(list,OP_SHARED_FILE_LIST);
}

void CVoodooSocket::SynchronizeSharedFileList(CSafeMemFile &packet)
{
	uint32 RecordsNumber = packet.ReadUInt32();
	for (uint32 i = 0; i < RecordsNumber; i++)
	{
		CKnownFile* kFile = GetSharedFile(packet);
		if(!kFile)
			continue;
		kFile->AddMaster(this);
	}
}

void CVoodooSocket::SendSharedFilesListV2()
{

	CMap<CCKey,const CCKey&,CKnownFile*,CKnownFile*>* filemap = theApp.sharedfiles->GetFileMap();
	CKnownFile* kFile;
	CCKey bufKey;
	for (POSITION pos = filemap->GetStartPosition();pos != 0;){
		filemap->GetNextAssoc(pos,bufKey,kFile);
		if(kFile->IsVoodooFile())
			continue;
		if(kFile->IsPartFile())
			continue;
		
		SendShareInstruction(kFile,INST_SHARE);
	}
}

/////////////////////////
// New Download Order

void CVoodooSocket::SendDownloadOrder(CPartFile* pFile)
{
	ASSERT(m_uType == CT_ED2K); // Only ED2K compatible clients support this

	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;
	pFile->WriteToTempFile(&data);
	SendPacket(data,OP_DOWNLOAD_ORDER);
}

void CVoodooSocket::DownloadOrderRecived(CSafeMemFile &packet)
{
	FileID fileID(packet,Use64Size());

	CPartFile* pFile = theApp.downloadqueue->GetFileByID(fileID.Hash);
	if(pFile){
		if(pFile->GetFileSize() != fileID.Size){
			SendFileUnavalible(fileID,RET_BADSIZE);
			ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),pFile->GetFileName(),fileID.Size,pFile->GetFileSize(),DbgGetClientInfo());
		}else if(pFile->IsVoodooFile()){
			if(pFile->GetMasterDatas(this) == NULL)
				pFile->AddMaster(this);
		}else{
			SendFileUnavalible(fileID, RET_REAL);
			ModLog(LOG_WARNING,GetResString(IDS_X_NOT_VOODOO_FILE),DbgGetClientInfo(),pFile->GetFileName());
		}
		return;
	}else{
		CKnownFile* kFile = theApp.sharedfiles->GetFileByID(fileID.Hash);
		if(kFile){
			SendFileUnavalible(fileID, RET_COMPLETE);
			ModLog(LOG_WARNING,GetResString(IDS_X_VOODOO_ERR_COMPLETE_FILE),DbgGetClientInfo(),kFile->GetFileName());
		}
	}

	pFile = new CPartFile;

	pFile->SetVoodoo();

	if(NeoPrefs.UseVirtualVoodooFiles() == FALSE)
		pFile->CreatePartFile();

	try{
		if (pFile->LoadFromTempFile(&packet)){
			pFile->SetStatus(PS_PAUSED);

			theApp.downloadqueue->GetFileList()->AddTail(pFile);
			theApp.sharedfiles->SafeAddKFile(pFile); // NEO: SAFS - [ShowAllFilesInShare]
			theApp.emuledlg->transferwnd->downloadlistctrl.AddFile(pFile); // show in downloadwindow
			// NEO: PP - [PasswordProtection]
			if (NeoPrefs.IsHideVoodooFiles())
				theApp.emuledlg->transferwnd->downloadlistctrl.HideFile(pFile);
			// NEO: PP END

			pFile->AddMaster(this);

			RequestGapList(pFile); // I know we got the list in the file, but anyway
		}else{
			delete pFile;
			SendFileUnavalible(fileID, RET_NEW);
			m_LastErrFile = fileID;
			return;
		}
	}catch(CFileException* error){
		delete pFile;
		SendFileUnavalible(fileID, RET_NEW);
		m_LastErrFile = fileID;
		throw error;
	}
}

//////////////////////
// New Share Order

void CVoodooSocket::SendShareOrder(CKnownFile* kFile)
{
	ASSERT(m_uType == CT_ED2K); // Only ED2K compatible clients support this

	CSafeMemFile data(128);
	if(!WriteFileID(data,kFile,Use64Size()))
		return;
	kFile->WriteToFile(&data);
	SendPacket(data,OP_SHARE_ORDER);
}

void CVoodooSocket::ShareOrderRecived(CSafeMemFile &packet)
{
	FileID fileID(packet,Use64Size());

	CKnownFile* kFile = theApp.knownfiles->FindKnownFileByID(fileID.Hash);
	if(!kFile)
		kFile = theApp.downloadqueue->GetFileByID(fileID.Hash);

	if(kFile){
		if(kFile->GetFileSize() != fileID.Size){
			SendFileUnavalible(fileID,RET_BADSIZE);
			ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());
		}else if(kFile->IsPartFile()){
			// don't report to master 
			ModLog(GetResString(IDS_X_COMPLETE_VOODOO_FILE),DbgGetClientInfo(),kFile->GetFileName());
		}else if(!kFile->IsRealFile()){
			// we can add more masters for this complete file without any problems
			if(kFile->GetMasterDatas(this) == NULL)
				kFile->AddMaster(this);
		}
		return;
	}

	kFile = new CKnownFile();
	try{
		if (kFile->LoadFromFile(&packet)){
			theApp.knownfiles->SafeAddKFile(kFile);
			kFile->AddMaster(this);
		}else{
			delete kFile;
			SendFileUnavalible(fileID, RET_NEW);
			m_LastErrFile = fileID;
			return;
		}
	}catch(CFileException* error){
		delete kFile;
		SendFileUnavalible(fileID, RET_NEW);
		m_LastErrFile = fileID;
		throw error;
	}
}

////////////////////////////////////
// Updating gap List

void CVoodooSocket::RequestGapList(CPartFile* pFile)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;
	SendPacket(data,OP_GAP_LIST_REQUEST);
}

void CVoodooSocket::GapListRequestRecived(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet,false);
	if(!pFile)
		return;
	SendGapList(pFile);
}

void CVoodooSocket::SendGapList(CPartFile* pFile)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;

	CTypedPtrList<CPtrList, Gap_Struct*>& gaplist = pFile->GetGapList();
	data.WriteUInt32(gaplist.GetCount());
	for (POSITION pos = gaplist.GetHeadPosition();pos != NULL;)
	{
		Gap_Struct* gap = gaplist.GetNext(pos);
		if(Use64Size()){
			data.WriteUInt64(gap->start);
			data.WriteUInt64(gap->end);
		}else{
			data.WriteUInt32((uint32)gap->start);
			data.WriteUInt32((uint32)gap->end);
		}
	}

	SendPacket(data,OP_GAP_LIST);
}

void CVoodooSocket::GapListRecived(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet);
	if(!pFile)
		return;

	CMasterDatas* Datas = pFile->GetMasterDatas(this);
	ASSERT(Datas);
	
	while (!Datas->gaplist->IsEmpty())
		delete Datas->gaplist->RemoveHead();

	UINT tagcount = packet.ReadUInt32();
	for (UINT j = 0; j < tagcount; j++)
	{
		Gap_Struct* gap = new Gap_Struct;
		if(Use64Size()){
			gap->start = packet.ReadUInt64();
			gap->end = packet.ReadUInt64();
		}else{
			gap->start = packet.ReadUInt32();
			gap->end = packet.ReadUInt32();
		}
		Datas->gaplist->AddTail(gap);
	}

	pFile->ReCombinateGapList();
}

/////////////////////////////
// Download Instructions

void CVoodooSocket::SendDownloadInstruction(CPartFile* pFile, uint8 uInstruction, uint32 Flag1, uint32 Flag2)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;
	data.WriteUInt8(uInstruction);
	if(m_uVoodooVersion >= 0x02){
		data.WriteUInt32(Flag1);
		data.WriteUInt32(Flag2);
	}
	SendPacket(data,OP_DOWNLOAD_INSTRUCTION);
}

CPartFile* CVoodooSocket::GetDownloadingFile(CSafeMemFile &packet, bool bRequest, bool bLogUnknown)
{
	FileID fileID(packet,Use64Size());
	CPartFile* pFile = theApp.downloadqueue->GetFileByID(fileID.Hash);
	if(!pFile){
		if(fileID != m_LastErrFile)
			SendFileUnavalible(fileID, bRequest ? RET_UNKNOWN : RET_UNAVALIBLY);
		else if (thePrefs.GetVerbose())
			DebugLog(_T("Droped operation for Erroneous file %s, from client %s "),md4str(fileID.Hash),DbgGetClientInfo());
			
		if(bRequest && bLogUnknown)
			ModLog(LOG_WARNING,GetResString(IDS_X_VOODOO_FILE_FAILED_SYNC),md4str(fileID.Hash),DbgGetClientInfo());
		return NULL;
	}
	else if(pFile->GetFileSize() != fileID.Size){
		ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),pFile->GetFileName(),fileID.Size,pFile->GetFileSize(),DbgGetClientInfo());
		SendFileUnavalible(fileID,RET_BADSIZE);
		return NULL;
	}
	else if(IsMaster() && !pFile->IsVoodooFile()){
		ModLog(LOG_WARNING,GetResString(IDS_X_NOT_VOODOO_FILE),DbgGetClientInfo(),pFile->GetFileName());
		SendFileUnavalible(fileID, RET_REAL);
		return NULL;
	}

	// update master on the fly
	if(IsMaster()){
		if(pFile->GetMasterDatas(this) == NULL)
			pFile->AddMaster(this);
	}

	return pFile;
}

void CVoodooSocket::DownloadInstructionRecived(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet,true,false);
	if(!pFile)
		return;

	uint8 uInstruction = packet.ReadUInt8();
	uint32 Flag1 = NULL;
	uint32 Flag2 = NULL;
	if(m_uVoodooVersion >= 0x02){
		Flag1 = packet.ReadUInt32();
		Flag2 = packet.ReadUInt32();
	}
	ExecuteDownloadInstruction(pFile, uInstruction, Flag1, Flag2);
}

void CVoodooSocket::ExecuteDownloadInstruction(CPartFile* pFile, uint8 uInstruction, uint32 Flag1, uint32 Flag2)
{
	switch(uInstruction){
		case INST_RESUME:{
			RequestGapList(pFile);
			// NEO: VOODOOx - [VoodooSourceExchange]
			if(pFile->IsVoodooXS(true) && GetVoodooXSVersion() > 0 && pFile->IsStopped())
				RequestSourceList(pFile);
			// NEO: VOODOOx END
			UINT uState = pFile->GetStatus();
			if(pFile->IsPaused() || pFile->IsStopped() 
			 || (uState != PS_READY && uState != PS_EMPTY))
				pFile->ResumeFile();
			break;
			}
		case INST_PAUSE:
			if(!pFile->IsPaused() && !pFile->IsStopped())
				pFile->PauseFile();
			break;
		case INST_STOP:
			if(NeoPrefs.UseVirtualVoodooFiles() == TRUE)
				pFile->DeleteFile(); 
			else if(!pFile->IsStopped())
				pFile->StopFile();
			break;
		case INST_DELETE:
			pFile->DeleteFile();
			break;
		case INST_COMPLETE:
			pFile->VoodooComplete(true);
			break;

		case INST_DL_PRIO:
			if(Flag1 == PR_AUTO){
				pFile->SetAutoDownPriority(true);
				pFile->SetDownPriority(PR_HIGH);
			}else{
				pFile->SetAutoDownPriority(false);
				if (Flag1 != PR_LOW && Flag1 != PR_NORMAL && Flag1 != PR_HIGH)
					pFile->SetDownPriority(PR_NORMAL);
				else
					pFile->SetDownPriority((uint8)Flag1);
			}
			break;

		default:
			DebugLogError(_T("Unknown Download Instruction Recived: %u; %u/%u"),(uint32)uInstruction,Flag1,Flag2);
	}
}

/////////////////////
// Share instructions

void CVoodooSocket::SendShareInstruction(CKnownFile* kFile, uint8 uInstruction, uint32 Flag1, uint32 Flag2)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,kFile,Use64Size()))
		return;
	data.WriteUInt8(uInstruction);
	if(m_uVoodooVersion >= 0x02){
		data.WriteUInt32(Flag1);
		data.WriteUInt32(Flag2);
	}
	SendPacket(data,OP_SHARE_INSTRUCTION);
}

CKnownFile*	CVoodooSocket::GetSharedFile(CSafeMemFile &packet)
{
	FileID fileID(packet,Use64Size());
	CKnownFile* kFile = theApp.knownfiles->FindKnownFileByID(fileID.Hash);
	if(!kFile)
		kFile = theApp.downloadqueue->GetFileByID(fileID.Hash);
	if(!kFile){
		if(fileID != m_LastErrFile)
			SendFileUnavalible(fileID,RET_UNKNOWN);
		else if (thePrefs.GetVerbose())
			DebugLog(_T("Droped operation for Erroneous file %s, from client %s "),md4str(fileID.Hash),DbgGetClientInfo());

		//if(bLogUnknown)
		//	ModLog(LOG_WARNING, GetResString(IDS_X_VOODOO_FILE_FAILED_ERR),md4str(fileID.Hash),DbgGetClientInfo());
		return NULL;
	}
	else if(kFile->GetFileSize() != fileID.Size){
		SendFileUnavalible(fileID,RET_BADSIZE);
		ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());
		return NULL;
	}
	else if(kFile->IsRealFile()){
		ModLog(LOG_WARNING,GetResString(IDS_X_NOT_VOODOO_FILE),DbgGetClientInfo(),kFile->GetFileName());
		//SendFileUnavalible(fileID, RET_REAL); // for shared files it is no problem
		return NULL;
	}
	/*else if(kFile->IsPartFile()){ // For now allowed for ul priority setting
		ModLog(LOG_WARNING,GetResString(IDS_X_NOT_VOODOO_KNOWN_FILE),DbgGetClientInfo(),kFile->GetFileName());
		return NULL;
	}*/
	return kFile;
}

void CVoodooSocket::ShareInstructionRecived(CSafeMemFile &packet)
{
	CKnownFile* kFile = GetSharedFile(packet);
	if(!kFile)
		return;

	uint8 uInstruction = packet.ReadUInt8();
	uint32 Flag1 = NULL;
	uint32 Flag2 = NULL;
	if(m_uVoodooVersion >= 0x02){
		Flag1 = packet.ReadUInt32();
		Flag2 = packet.ReadUInt32();
	}
	switch(uInstruction){
		case INST_SHARE:
			if(!kFile->IsPartFile()){
				if(kFile->GetMasterDatas(this) == NULL)
					kFile->AddMaster(this);
			}else
				ModLog(LOG_WARNING,GetResString(IDS_X_NOT_VOODOO_KNOWN_FILE),DbgGetClientInfo(),kFile->GetFileName());
			break;
		case INST_UNSHARE:
			if(!kFile->IsPartFile())
				kFile->RemoveMaster(this);
			else
				ModLog(LOG_WARNING,GetResString(IDS_X_NOT_VOODOO_KNOWN_FILE),DbgGetClientInfo(),kFile->GetFileName());
			break;
		case INST_UL_PRIO:
			if(Flag1 == 10){
				kFile->SetReleasePriority(I2B(Flag2));
			}else if(Flag1 == PR_AUTO){
				kFile->SetAutoUpPriority(true);
				kFile->UpdateAutoUpPriority();
			}else{
				kFile->SetAutoUpPriority(false);
				if (Flag1 != PR_VERYLOW && Flag1 != PR_LOW && Flag1 != PR_NORMAL && Flag1 != PR_HIGH && Flag1 != PR_VERYHIGH)
					kFile->SetUpPriority(PR_NORMAL);
				else
					kFile->SetUpPriority((uint8)Flag1);
			}
			break;
		default:
			DebugLogError(_T("Unknown Share Instruction Recived %u"),uInstruction);
	}
}

///////////////////////////
// Hash Set exchange

void CVoodooSocket::SendHashSetRequest(CPartFile* pFile)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;
	SendPacket(data,OP_HASH_SET_REQUEST);
}

void CVoodooSocket::HashSetRequestRecived(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet);
	if(!pFile)
		return;
	SendHashSetResponde(pFile);
}

void CVoodooSocket::SendHashSetResponde(CKnownFile* kFile)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,kFile,Use64Size()))
		return;

	data.WriteHash16(kFile->GetFileHash());
	UINT parts = kFile->GetHashCount();
	data.WriteUInt16((uint16)parts);
	for (UINT i = 0; i < parts; i++)
		data.WriteHash16(kFile->GetPartHash(i));

	SendPacket(data,OP_HASH_SET_RESPONSE);
}

void CVoodooSocket::HashSetRespondeRecived(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet, false);
	if(!pFile)
		return;

	if(pFile->hashsetneeded){
		if (pFile->LoadHashsetFromFile(&packet,true))
			pFile->hashsetneeded = false;
		else
			throw GetResString(IDS_ERR_BADHASHSET);
	} // we ask all slaves at once, we may got already the answer from an other slave
}

///////////////////////////
// Packet Reciver

bool CVoodooSocket::PacketReceived(Packet* packet)
{
	bool bResult = true;
	switch (packet->prot){
		case OP_VOODOOPROT:
			bResult = ProcessVoodooPacket((const BYTE*)packet->pBuffer, packet->size, packet->opcode);
			break;
		default:{
			if (thePrefs.GetVerbose())
				DebugLogError(_T("Failed to decompress client TCP packet; %s; %s"), DbgGetClientTCPPacket(packet->prot, packet->opcode, packet->size), DbgGetClientInfo());
			//if(!m_bIsMaster && !m_bIsSlave){ // Don't disconnect a connected vooodo socket on error
				Disconnect(_T("Unknown protocol"));
				bResult = false;
			//}
		}
	}
	return bResult;
}


bool CVoodooSocket::ProcessVoodooPacket(const BYTE* packet, uint32 size, UINT opcode)
{
	try
	{
		try
		{
			if ((!IsMaster() && (
			   opcode == OP_DOWNLOAD_QUEUE
			|| opcode == OP_SHARED_FILE_LIST
			|| opcode == OP_DOWNLOAD_ORDER 
			|| opcode == OP_SHARE_ORDER 
			|| opcode == OP_GAP_LIST 
			|| opcode == OP_DOWNLOAD_INSTRUCTION 
			|| opcode == OP_SHARE_INSTRUCTION 
			|| opcode == OP_CORRUPT_SENDER
			|| opcode == OP_UPLOAD_DATA
			// NEO: VOODOOs - [VoodooSearchForwarding]
			|| opcode == OP_SEARCH_REQUEST
			|| opcode == OP_SEARCH_COMMAND
			// NEO: VOODOOs END
			)) ||
				(!IsSlave() && (
			   opcode == OP_GAP_LIST_REQUEST 
			|| opcode == OP_DOWNLOAD_DATA 
			|| opcode == OP_DATA_REQUEST
			|| opcode == OP_STATISTICS
			|| opcode == OP_FILE_STATISTICS
			// NEO: VOODOOs - [VoodooSearchForwarding]
			|| opcode == OP_SEARCH_STATE
			|| opcode == OP_SEARCH_RESULT
			// NEO: VOODOOs END
			)) ||
				(!IsMaster() && !IsSlave() && (
			   opcode == OP_FILE_UNAVALIBLE
			|| opcode == OP_THROTTLE_BLOCK
			//|| opcode == OP_DOWNLOAD_UNAVALIBLE // old
			// NEO: VOODOOx - [VoodooSourceExchange]
			|| opcode == OP_SOURCE_LIST_REQUEST
			|| opcode == OP_SOURCE_LIST
			// NEO: VOODOOx END
			// NEO: VOODOOn - [VoodooForNeo]
			|| opcode == OP_DOWNLOAD_COMMAND
			|| opcode == OP_NEO_PREFERENCES
			// NEO: VOODOOn END
			))
			)
				throw new CHierarchyExceptio((uint8)opcode);

			CSafeMemFile data(packet, size);
			switch(opcode)
			{
				case OP_VOODOO_HELLO:{
					RecivedVoodooHello(data);
					break;
				}
				case OP_SPELL:{
					SpellRecived(data);
					break;
				}
				case OP_SPELL_RESULT:{
					SpellResultRecived(data);
					break;
				}
				case OP_VOODOO_PING:{
					RecivedPing(data);
					break;
				}
				case OP_VOODOO_GOODBY:{
					ModLog(GetResString(IDS_X_VOODOO_GOODBY),DbgGetClientInfo());
					PrepDisconnect();
					break;
				}
				case OP_DOWNLOAD_QUEUE:{
					SynchronizeDownloadQueue(data);
					break;
				}
				//case OP_DOWNLOAD_UNAVALIBLE:
				case OP_FILE_UNAVALIBLE:{
					FileUnavalibleRecived(data);
					break;
				}
				case OP_NEW_DOWNLOAD_ORDER:{
					ModLog(LOG_WARNING,GetResString(IDS_X_VOODOO_OLD_OPCODE),DbgGetClientInfo(),_T("OP_NEW_DOWNLOAD_ORDER"));
					break;
				}
				case OP_DOWNLOAD_ORDER:{
					DownloadOrderRecived(data);
					break;
				}
				case OP_SHARE_ORDER:{
					ShareOrderRecived(data);
					break;
				}
				case OP_SHARED_FILE_LIST:{
					SynchronizeSharedFileList(data);
					break;
				}
				case OP_GAP_LIST_REQUEST:{
					GapListRequestRecived(data);
					break;
				}
				case OP_GAP_LIST:{
					GapListRecived(data);
					break;
				}
				case OP_DOWNLOAD_INSTRUCTION:{
					DownloadInstructionRecived(data);
					break;
				}
				case OP_SHARE_INSTRUCTION:{
					ShareInstructionRecived(data);
					break;
				}
				// hashset
				case OP_HASH_SET_REQUEST:{
					HashSetRequestRecived(data);
					break;
				}
				case OP_HASH_SET_RESPONSE:{
					HashSetRespondeRecived(data);
					break;
				}
				// Data DownLink
				case OP_DOWNLOAD_DATA:{
					RecivedFileData(data);
					break;
				}
				case OP_THROTTLE_BLOCK:{
					RecivedThrottleBlock(data);
					break;
				}

				// corruption handling
				case OP_CORRUPT_SENDER:{
					RecivedCorruptedSenderWarning(data);
					break;
				}

				// Data UpLink
				case OP_DATA_REQUEST:{
					FileDataRequestRecived(data);
					break;
				}
				case OP_UPLOAD_DATA:{
					FileDataAnswerRecived(data);
					break;
				}
				// stats
				case OP_STATISTICS:{
					StatisticsRecived(data);
					break;
				}
				case OP_FILE_STATISTICS:{
					FileStatisticsRecived(data);
					break;
				}
				// NEO: VOODOOs - [VoodooSearchForwarding]
				// voodoo search forwarding
				case OP_SEARCH_REQUEST:{
					NewSearchRecived(data);
					break;
				}
				case OP_SEARCH_STATE:{
					SearchStateRecived(data);
					break;
				}
				case OP_SEARCH_COMMAND:{
					SearchCommandRecived(data);
					break;
				}
				case OP_SEARCH_RESULT:{
					SearchResultRecived(data);
					break;
				}
				// NEO: VOODOOs END
				// NEO: VOODOOx - [VoodooSourceExchange]
				// Source exchange
				case OP_SOURCE_LIST_REQUEST:{
					SourceListRequestRecived(data);
					break;
				}
				case OP_SOURCE_LIST:{
					SourceListRecived(data);
					break;
				}
				// NEO: VOODOOx END
				// NEO: VOODOOn - [VoodooForNeo]
				case OP_DOWNLOAD_COMMAND:{
					DownloadCommandRecived(data);
					break;
				}
				case OP_NEO_PREFERENCES:{
					NeoPreferencesRecived(data);
					break;
				}
				// NEO: VOODOOn END
				default:
					PacketToDebugLogLine(_T("Voodoo"), packet, size, opcode);
			}
		}
		catch(CHierarchyExceptio* error)
		{
			uint8 uOpCode = error->m_uOpCode;
			uint8 uTagCode = error->m_uTagCode;
			error->Delete();
			ASSERT(0);
			throw StrLine(GetResString(IDS_X_VOODOO_HIERARCHY_VIOLATION),DbgGetClientInfo(), uOpCode, uTagCode, GetClientDesc());
		}
		catch(CFileException* error)
		{
			error->Delete();
			throw GetResString(IDS_ERR_INVALIDPACKAGE);
		}
		catch(CMemoryException* error)
		{
			error->Delete();
			throw CString(_T("Memory exception"));
		}
	}
	catch(CString error)
	{
		if (thePrefs.GetVerbose() && !error.IsEmpty())
			DebugLogWarning(_T("VOODOO: %s - while processing voodoo packet: opcode=%s  size=%u; %s"), error, DbgGetMuleClientTCPOpcode(opcode), size, DbgGetClientInfo());
		if(!m_bIsMaster && !m_bIsSlave){ // Don't disconnect a connected vooodo socket on error
			Disconnect(_T("ProcessVoodooPacket error. ") + error);
			return false;
		}
	}
	return true;
}

//////////////////////////////////
// Data DownLink

void CVoodooSocket::SplitPart(CPartFile* pFile, PartFileBufferedData *item)
{
	uint64 lenData = (item->end - item->start + 1);
	uint64 start = item->start;
	uint32 lenPart;
	uint64 offSet = 0;
	while(lenData){
		PartFileBufferedData* newItem = new PartFileBufferedData;
		newItem->start = start;
		lenPart = (uint32)min(lenData,KB2B(512));
		newItem->end = lenPart + newItem->start - 1;
		start = newItem->end + 1;
		
		newItem->data = new BYTE[lenPart];
		memcpy(newItem->data, item->data + offSet, lenPart);
		lenData -= lenPart;
		offSet += lenPart;

		TransferFileData(pFile,newItem, 0); // packets to split arrives not from real sources but from part import *only*
	}
}

void CVoodooSocket::TransferFileData(CPartFile* pFile, PartFileBufferedData *item, DWORD dwIP)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;

	if(GetCorruptionHandlingVersion())
		data.WriteUInt32(dwIP);

	if(Use64Size()){
		data.WriteUInt64(item->start);
		data.WriteUInt64(item->end);
	}else{
		data.WriteUInt32((uint32)item->start);
		data.WriteUInt32((uint32)item->end);
	}

	uint32 lenData = (uint32)(item->end - item->start + 1);
	data.Write(item->data, lenData);

	SendPacket(data,OP_DOWNLOAD_DATA,false);

	delete[] item->data;
	delete item;
}

void CVoodooSocket::RecivedFileData(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet,false);
	if(!pFile)
		return;
	
	DWORD dwIP = 0;
	if(GetCorruptionHandlingVersion())
		dwIP = packet.ReadUInt32();

	PartFileBufferedData item;
	if(Use64Size()){
		item.start = packet.ReadUInt64();
		item.end = packet.ReadUInt64();
	}else{
		item.start = packet.ReadUInt32();
		item.end = packet.ReadUInt32();
	}

	if (pFile->IsComplete(item.start, item.end, false))
	{
		DebugLog(LOG_ERROR,GetResString(IDS_X_VOODOO_GOT_COMPLETED_DATA),DbgGetClientInfo(), pFile->GetFileName());
		SendGapList(pFile);
		return;
	}

	uint32 lenData = (uint32)(item.end - item.start + 1);
	item.data = new BYTE[lenData];
	packet.Read(item.data, lenData);

	pFile->WriteToBuffer(lenData, item.data, item.start, item.end, NULL, dwIP);
	delete [] item.data;

	if (m_uType == CT_ED2K && pFile->GetHashCount() != pFile->GetED2KPartHashCount()) // do we need to get the hashset from our slave?
		SendHashSetRequest(pFile);
}

// Download synchronisation
void CVoodooSocket::SendThrottleBlock(CPartFile* pFile, uint64 start, uint64 end, bool bRelease)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;

	if(GetAdvDownloadSyncVersion() < 3){ // old version
		data.WriteUInt16((uint16)(start/PARTSIZE));
	}else
	if(Use64Size()){
		data.WriteUInt64(start);
		data.WriteUInt64(end);
	}else{
		data.WriteUInt32((uint32)start);
		data.WriteUInt32((uint32)end);
	}
	
	data.WriteUInt8(bRelease);
	SendPacket(data,OP_THROTTLE_BLOCK,true);
}

void CVoodooSocket::RecivedThrottleBlock(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet,IsMaster());
	if(!pFile)
		return;

	uint64 start;
	uint64 end;
	if(GetAdvDownloadSyncVersion() < 3){ // old version
		uint16 partNumber = packet.ReadUInt16();
		start = partNumber * PARTSIZE;
		end = start + PARTSIZE;
	}else
	if(Use64Size()){
		start = packet.ReadUInt64();
		end = packet.ReadUInt64();
	}else{
		start = packet.ReadUInt32();
		end = packet.ReadUInt32();
	}

	bool bRelease = false;
	if(GetAdvDownloadSyncVersion() > 1)
		bRelease = packet.ReadUInt8() != 0;

	if(bRelease)
		pFile->RemoveThrottleBlock(start,end);
	else
		pFile->AddThrottleBlock(start,end);

	theApp.voodoo->ManifestThrottleBlock(pFile, start, end, bRelease, this); // Manifest Throttle instruction to remainding slaves
}

// Corruption handling
void CVoodooSocket::SendCorruptedSenderWarning(DWORD dwIP)
{
	CSafeMemFile data(128);
	data.WriteUInt32(dwIP);
	SendPacket(data,OP_CORRUPT_SENDER,true);
}

void CVoodooSocket::RecivedCorruptedSenderWarning(CSafeMemFile &packet)
{
	DWORD dwIP = packet.ReadUInt32();

	CUpDownClient* pEvilClient = theApp.clientlist->FindClientByIP(dwIP);
	if (pEvilClient != NULL){
		AddDebugLogLine(DLP_HIGH, false, _T("CorruptionBlackBox: Banning: Found client which send corrupted data, %s"), pEvilClient->DbgGetClientInfo());
		theApp.clientlist->AddTrackClient(pEvilClient);
		pEvilClient->Ban(_T("Identified as sender of corrupt data"));
	}
	else{
		AddDebugLogLine(DLP_HIGH, false, _T("CorruptionBlackBox: Banning: Found client which send corrupted data, %s"), ipstr(dwIP));
		theApp.clientlist->AddBannedClient(dwIP);
	}
}


/////////////////////////
// Data UpLink

void CVoodooSocket::RequestFileData(CKnownFile* kFile, sDataRequest &DataRequest)
{
	// NO RBT BEGIN
	//uint32 &requested = m_DataRequested[DataRequest.IdKey];
	//if(requested && ::GetTickCount() - requested < SEC2MS(10))
	//	return;
	//requested = ::GetTickCount();
	// NO RBT END

	CSafeMemFile data(128);
	if(!WriteFileID(data,kFile,Use64Size()))
		return;

	data.WriteUInt32(DataRequest.IdKey);
	if(Use64Size()){
		data.WriteUInt64(DataRequest.StartOffset);
		data.WriteUInt64(DataRequest.EndOffset);
	}else{
		data.WriteUInt32((uint32)DataRequest.StartOffset);
		data.WriteUInt32((uint32)DataRequest.EndOffset);
	}
	data.WriteUInt32(DataRequest.Client);

	SendPacket(data,OP_DATA_REQUEST,false);
}

void CVoodooSocket::FileDataRequestRecived(CSafeMemFile &packet)
{
	FileID fileID(packet,Use64Size());

	CKnownFile* kFile = theApp.sharedfiles->GetFileByID(fileID.Hash);
	if(!kFile){
		DebugLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_FAILED_NO),md4str(fileID.Hash),DbgGetClientInfo());
		SendFileUnavalible(fileID,RET_UNAVALIBLY);
		return;
	}else if(kFile->GetFileSize() != fileID.Size){
		ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());
		SendFileUnavalible(fileID,RET_BADSIZE);
		return;
	}

	sDataRequest* DataRequest = new sDataRequest;
	DataRequest->IdKey = packet.ReadUInt32();
	if(Use64Size()){
		DataRequest->StartOffset = packet.ReadUInt64();
		DataRequest->EndOffset = packet.ReadUInt64();
	}else{
		DataRequest->StartOffset = packet.ReadUInt32();
		DataRequest->EndOffset = packet.ReadUInt32();
	}
	DataRequest->ID = fileID;
	if(packet.GetLength() - packet.GetPosition() >= sizeof(uint32))
		DataRequest->Client = packet.ReadUInt32();
	else
		DataRequest->Client = NULL;
	
	// NO RBT BEGIN
	//sDataBuffer* DataBuffer = GetFileData(kFile, *DataRequest);
	//
	//if(!DataBuffer)
	//	return;
	//
	//CSafeMemFile data(128);
	//WriteFileID(data,DataRequest->ID,Use64Size());
	//
	//data.WriteUInt32(DataRequest->IdKey);
	//data.WriteUInt32(DataBuffer->Size);
	//data.Write(DataBuffer->Data,DataBuffer->Size);
	//
	//SendPacket(data,OP_UPLOAD_DATA,false);

	//delete [] DataBuffer->Data;
	//delete DataBuffer;
	//delete DataRequest;
	// NO RBT END

	if(kFile->IsVoodooFile()){ // cascading dissalowed, but check just in case
		delete DataRequest;
		SendFileUnavalible(fileID,RET_UNAVALIBLY);
	}else
		GetFileData(kFile, DataRequest);
}

// NO RBT NEGIN
//sDataBuffer* CVoodooSocket::GetFileData(CKnownFile* kFile, sDataRequest &DataRequest)
//{
//	static uint32 cleanuptimer = ::GetTickCount();
//	if(::GetTickCount() - cleanuptimer > MIN2MS(5))
//	{
//		CleanUpUploadBuffer();
//		cleanuptimer = ::GetTickCount();
//	}
//
//	CFile file;
//	sDataBuffer* DataBuffer = NULL;
//	CString fullname;
//	CSyncHelper lockFile;
//	uint32 togo = 0;
//	try{
//		if (kFile->IsPartFile() && ((CPartFile*)kFile)->GetStatus() != PS_COMPLETE){
//			// Do not access a part file, if it is currently moved into the incoming directory.
//			// Because the moving of part file into the incoming directory may take a noticable 
//			// amount of time, we can not wait for 'm_FileCompleteMutex' and block the main thread.
//			if (!((CPartFile*)kFile)->m_FileCompleteMutex.Lock(0)){ // just do a quick test of the mutex's state and return if it's locked.
//				return NULL;
//			}
//			lockFile.m_pObject = &((CPartFile*)kFile)->m_FileCompleteMutex;
//			// If it's a part file which we are uploading the file remains locked until we've read the
//			// current block. This way the file completion thread can not (try to) "move" the file into
//			// the incoming directory.
//			fullname = RemoveFileExtension(((CPartFile*)kFile)->GetFullName());
//		}
//		else{
//			fullname.Format(_T("%s\\%s"),kFile->GetPath(),kFile->GetFileName());
//		}
//
//		uint64 i64uTogo = 0;
//		if (DataRequest.StartOffset > DataRequest.EndOffset){
//			i64uTogo = DataRequest.EndOffset + (kFile->GetFileSize() - DataRequest.StartOffset);
//		}
//		else{
//			i64uTogo = DataRequest.EndOffset - DataRequest.StartOffset;
//			if (kFile->IsPartFile() && !((CPartFile*)kFile)->IsComplete(DataRequest.StartOffset,DataRequest.EndOffset-1, true))
//				throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
//		}
//
//		togo = (uint32)i64uTogo;
//
//		//if( togo > EMBLOCKSIZE*3 )
//		//	throw GetResString(IDS_ERR_LARGEREQBLOCK);
//		
//		if(kFile->IsVoodooFile()){ // Caskade voodoo Requests, not recomended
//			throw GetResString(IDS_X_VOODOO_CASCADE);
//		}else
//		if (!kFile->IsPartFile()){
//			if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone))
//				throw GetResString(IDS_ERR_OPEN);
//			file.Seek(DataRequest.StartOffset,0);
//			
//			DataBuffer = new sDataBuffer;
//			DataBuffer->Size = togo;
//			DataBuffer->Data = new byte[togo+500];
//			if (uint32 done = file.Read(DataBuffer->Data,togo) != togo){
//				file.SeekToBegin();
//				file.Read(DataBuffer->Data + done,togo-done);
//			}
//			file.Close();
//		}
//		else{
//			CPartFile* pFile = (CPartFile*)kFile;
//			if (pFile->m_hpartfile.m_hFile == INVALID_HANDLE_VALUE)
//				throw GetResString(IDS_ERR_OPEN);
//
//			pFile->m_hpartfile.Seek(DataRequest.StartOffset,0);
//			
//			DataBuffer = new sDataBuffer;
//			DataBuffer->Size = togo;
//			DataBuffer->Data = new byte[togo+500];
//			if (uint32 done = pFile->m_hpartfile.Read(DataBuffer->Data,togo) != togo){
//				pFile->m_hpartfile.SeekToBegin();
//				pFile->m_hpartfile.Read(DataBuffer->Data + done,togo-done);
//			}
//		}
//
//		if (lockFile.m_pObject){
//			lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
//			lockFile.m_pObject = NULL;
//		}
//
//		return DataBuffer;
//	}
//	catch(CString error)
//	{
//		if (thePrefs.GetVerbose())
//			DebugLogWarning(_T("Failed to create voodoo upload package for %s (%s) - %s"), DbgGetClientInfo(), kFile->GetFileName(), error);
//	}
//	catch(CFileException* e)
//	{
//		TCHAR szError[MAX_CFEXP_ERRORMSG];
//		e->GetErrorMessage(szError, ARRSIZE(szError));
//		if (thePrefs.GetVerbose())
//			DebugLogWarning(_T("Failed to create voodoo upload package for %s (%s) - %s"), DbgGetClientInfo(), kFile->GetFileName(), szError);
//		e->Delete();
//	}
//
//	if(DataBuffer){
//		if(DataBuffer->Data)
//			delete[] DataBuffer->Data;
//		delete DataBuffer;
//	}
//	return NULL;
//}
//
//void CVoodooSocket::FileDataAnswerRecived(CSafeMemFile &packet)
//{
//	FileID fileID(packet,Use64Size());
//
//	CKnownFile* kFile = theApp.sharedfiles->GetFileByID(fileID.Hash);
//	if(!kFile)
//		throw StrLine(GetResString(IDS_X_VOODOO_FILE_FAILED),md4str(fileID.Hash),DbgGetClientInfo()); // Shouldn't happen
//	else if(kFile->GetFileSize() != fileID.Size)
//		throw StrLine(GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());
//	
//	sDataBuffer* DataBuffer = new sDataBuffer;
//
//	DataBuffer->Time = ::GetTickCount();
//	uint32 IdKey = packet.ReadUInt32();
//	DataBuffer->Size = packet.ReadUInt32();
//	DataBuffer->Data = new byte[DataBuffer->Size+500];
//	if(DataBuffer->Size == 0)
//		return;
//
//	packet.Read(DataBuffer->Data,DataBuffer->Size);
//
//	m_DataRequested.RemoveKey(IdKey);
//
//	sDataBuffer* OldBuffer;
//	if(m_UploadBuffer.Lookup(IdKey,OldBuffer)){ // just in case
//		ASSERT(0);
//		delete [] OldBuffer->Data;
//		delete OldBuffer;
//	}
//
//	m_UploadBuffer.SetAt(IdKey,DataBuffer);
//}
//
//sDataBuffer* CVoodooSocket::GetBufferedUpload(CKnownFile* kFile, sDataRequest &DataRequest)
//{
//	sDataBuffer* DataBuffer;
//	if(m_UploadBuffer.Lookup(DataRequest.IdKey,DataBuffer)){
//		m_UploadBuffer.RemoveKey(DataRequest.IdKey);
//		return DataBuffer;
//	}
//
//	RequestFileData(kFile,DataRequest);
//
//	return NULL;
//}
//
//void CVoodooSocket::CleanUpUploadBuffer(bool bClear)
//{
//	uint32 currTick = ::GetTickCount();
//
//	sDataBuffer* DataBuffer;
//	uint32 IdKey;
//	POSITION pos2 = m_UploadBuffer.GetStartPosition();
//	while (pos2){
//		m_UploadBuffer.GetNextAssoc(pos2, IdKey, DataBuffer);
//		if(bClear || (currTick - DataBuffer->Time > SEC2MS(10))){
//			m_UploadBuffer.RemoveKey(IdKey);
//			delete [] DataBuffer->Data;
//			delete DataBuffer;
//		}
//	}
//
//	if(bClear)
//		m_DataRequested.RemoveAll();
//}
// NO RBT END

// NEO: RBT - [ReadBlockThread]
void CVoodooSocket::GetFileData(CKnownFile* kFile, sDataRequest* DataRequest)
{
	CString fullname;
	try{
		uint64 i64uTogo = 0;
		if (DataRequest->StartOffset > DataRequest->EndOffset){
			i64uTogo = DataRequest->EndOffset + (kFile->GetFileSize() - DataRequest->StartOffset);
		}
		else{
			i64uTogo = DataRequest->EndOffset - DataRequest->StartOffset;
			if (kFile->IsPartFile() && !((CPartFile*)kFile)->IsComplete(DataRequest->StartOffset,DataRequest->EndOffset-1, true))
				throw GetResString(IDS_ERR_INCOMPLETEBLOCK);
		}

		DataRequest->togo = (uint32)i64uTogo;

		// file statistic
		kFile->statistic.AddTransferred(DataRequest->StartOffset, DataRequest->togo); // NEO: NPT - [NeoPartTraffic]

		kFile->SetReadBlockFromFile(DataRequest->StartOffset, DataRequest->togo, this, DataRequest, true);
	}
	catch(CString error)
	{
		delete DataRequest;
		if (thePrefs.GetVerbose())
			DebugLogWarning(_T("Failed to create voodoo upload package for %s (%s) - %s"), DbgGetClientInfo(), kFile->GetFileName(), error);
	}
}

void CVoodooSocket::SendFileData(sDataRequest* DataRequest, byte* Data)
{
	if(Data == RBT_ERROR && thePrefs.GetVerbose()){ //An error occured
		CKnownFile* kFile = theApp.sharedfiles->GetFileByID(DataRequest->ID.Hash);
		DebugLogWarning(_T("Failed to create voodoo upload package for %s - %s"), DbgGetClientInfo(), kFile ? kFile->GetFileName() : _T("n/a"), GetResString(IDS_ERR_OPEN));
	}

	CSafeMemFile data(128);
	WriteFileID(data,DataRequest->ID,Use64Size());

	data.WriteUInt32(DataRequest->IdKey);
	if(Data != RBT_ERROR && Data != RBT_ACTIVE && Data != NULL){
		data.WriteUInt32(DataRequest->togo);
		data.Write(Data,DataRequest->togo);
		delete [] Data;
		Data = NULL;
	}else{
		data.WriteUInt32(0);
		data.WriteUInt32((uint32)Data);
	}
	data.WriteUInt32(DataRequest->Client);

	SendPacket(data,OP_UPLOAD_DATA,false);

	delete DataRequest;
}

void CVoodooSocket::FileDataAnswerRecived(CSafeMemFile &packet)
{
	FileID fileID(packet,Use64Size());

	CKnownFile* kFile = theApp.sharedfiles->GetFileByID(fileID.Hash);
	if(!kFile)
		throw StrLine(GetResString(IDS_X_VOODOO_FILE_FAILED),md4str(fileID.Hash),DbgGetClientInfo()); // Shouldn't happen
	else if(kFile->GetFileSize() != fileID.Size)
		throw StrLine(GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());
	
	Requested_Block_Struct* request = (Requested_Block_Struct*)packet.ReadUInt32();
	uint32 size = packet.ReadUInt32();
	byte* Data = NULL;
	
	if(size != 0){
		Data = new byte[size+500];
		packet.Read(Data,size);
	}else
		Data = (byte*)packet.ReadUInt32();

	CUpDownClient* clinet = NULL;
	if(packet.GetLength() - packet.GetPosition() >= sizeof(uint32))
		clinet = (CUpDownClient*)packet.ReadUInt32();
	else
	{
		// for compatybility to older masters, it isn't pritty but nessesery
		CUpDownClient* cur_client;
		for (POSITION pos = theApp.uploadqueue->GetFirstFromUploadList();
			pos != 0;theApp.uploadqueue->GetNextFromUploadList(pos))
		{
			cur_client = theApp.uploadqueue->GetQueueClientAt(pos);
			if(cur_client->m_BlockRequests_queue.Find(request)){
				clinet = cur_client;
				break;
			}
		}
	}
	
	if (theApp.uploadqueue->IsDownloading(clinet) // could have been canceld
	 && clinet->m_BlockRequests_queue.Find(request)) // check is this block pointer still valid
		request->filedata = Data;
	else if (Data != RBT_ERROR && Data != RBT_ACTIVE && Data != NULL)
		delete [] Data;
}
// NEO: RBT END

/////////////////////////
// Statistics

void CVoodooSocket::SendStatistics()
{
	CSafeMemFile data(128);

	SVoodooStats Stats;
	CMap<CKnownFile*,CKnownFile*,SFileVoodooStats,SFileVoodooStats> FileStats; // geathet file specyfic stats

	CTypedPtrList<CPtrList, CPartFile*>* filelist = theApp.downloadqueue->GetFileList();
	POSITION pos1 = filelist->GetHeadPosition();
	while(pos1 != NULL){
		CPartFile* pFile = filelist->GetNext(pos1);
		if(pFile->IsVoodooFile() && pFile->GetMasterDatas(this)){
			if(uint32 uDownDatarate = pFile->GetDatarate())	{
				Stats.uDownDatarate += uDownDatarate;
				FileStats[pFile].uDownDatarate += uDownDatarate;
			}
		}
	}

	POSITION pos2 = theApp.uploadqueue->uploadinglist.GetHeadPosition();
	while(pos2 != NULL){
		CUpDownClient* cur_client = theApp.uploadqueue->uploadinglist.GetNext(pos2);
		CKnownFile* kFile = theApp.sharedfiles->GetFileByID(cur_client->GetUploadFileID());
		if(kFile && kFile->IsVoodooFile() && kFile->GetMasterDatas(this)){
			if(uint32 uUpDatarate = cur_client->GetDatarate()){
				Stats.uUpDatarate += uUpDatarate;
				FileStats[kFile].uUpDatarate += uUpDatarate;
			}
		}
	}

	uint32 uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)data.GetPosition();
	data.WriteUInt32(uTagCount);

	CTag tagDown(ST_DOWN_DATARATE,Stats.uDownDatarate);
	tagDown.WriteTagToFile(&data);
	uTagCount++;

	CTag tagUp(ST_UP_DATARATE,Stats.uUpDatarate);
	tagUp.WriteTagToFile(&data);
	uTagCount++;

	data.Seek(uTagCountFilePos, CFile::begin);
	data.WriteUInt32(uTagCount);
	data.Seek(0, CFile::end);

	SendPacket(data,OP_STATISTICS);

	// send stats per file
	if(m_uSupportsStatistics > 1)
		SendFileStatistics(FileStats);
}

void CVoodooSocket::StatisticsRecived(CSafeMemFile &packet)
{
	m_Stats.uLastStatUpdate = ::GetTickCount();

	uint32 tagcount = packet.ReadUInt32();
	for (uint32 i = 0;i < tagcount; i++)
	{
		CTag temptag(&packet, true);
		switch (temptag.GetNameID())
		{
			case ST_DOWN_DATARATE:
				ASSERT (temptag.IsInt());
				m_Stats.uDownDatarate = temptag.GetInt();
				break;

			case ST_UP_DATARATE:
				ASSERT (temptag.IsInt());
				m_Stats.uUpDatarate = temptag.GetInt();
				break;

			default:
				DebugLogWarning(_T("Unknown Tag Voodoo Statistics Packet: %s"), temptag.GetFullInfo());
		}
	}
}

void CVoodooSocket::SendFileStatistics(CMap<CKnownFile*,CKnownFile*,SFileVoodooStats,SFileVoodooStats> &FileStats)
{
	SFileVoodooStats Stats;
	CKnownFile* File;
	for (POSITION pos = FileStats.GetStartPosition();pos != 0;){
		FileStats.GetNextAssoc(pos,File,Stats);

		CSafeMemFile data(128);

		if(!WriteFileID(data,File,Use64Size()))
			continue;

		uint32 uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)data.GetPosition();
		data.WriteUInt32(uTagCount);

		CTag tagDown(ST_DOWN_DATARATE,Stats.uDownDatarate);
		tagDown.WriteTagToFile(&data);
		uTagCount++;

		// not needed
		//CTag tagUp(ST_UP_DATARATE,Stats.uUpDatarate);
		//tagUp.WriteTagToFile(&data);
		//uTagCount++;

		data.Seek(uTagCountFilePos, CFile::begin);
		data.WriteUInt32(uTagCount);
		data.Seek(0, CFile::end);

		SendPacket(data,OP_FILE_STATISTICS);
	}
}

void CVoodooSocket::FileStatisticsRecived(CSafeMemFile &packet)
{
	FileID fileID(packet,Use64Size());

	CPartFile* pFile = theApp.downloadqueue->GetFileByID(fileID.Hash);
	CKnownFile* kFile = pFile ? pFile : theApp.knownfiles->FindKnownFileByID(fileID.Hash);

	if(!kFile){
		if(fileID != m_LastErrFile)
			SendFileUnavalible(fileID, RET_UNAVALIBLY);
		else if (thePrefs.GetVerbose())
			DebugLog(_T("Droped operation for Erroneous file %s, from client %s "),md4str(fileID.Hash),DbgGetClientInfo());
		return;
	}else if(kFile->GetFileSize() != fileID.Size){
		SendFileUnavalible(fileID,RET_BADSIZE);
		ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());
		return;
	}

	SFileVoodooStats &Stats = m_FileStats[kFile];

	// reset the values
	Stats.uDownDatarate = 0;
	Stats.uUpDatarate = 0;
	Stats.uLastStatUpdate = ::GetTickCount();

	uint32 tagcount = packet.ReadUInt32();
	for (uint32 i = 0;i < tagcount; i++)
	{
		CTag temptag(&packet, true);
		switch (temptag.GetNameID())
		{
			case ST_DOWN_DATARATE:
				ASSERT (temptag.IsInt());
				Stats.uDownDatarate = temptag.GetInt();
				break;

			case ST_UP_DATARATE:
			//	ASSERT (temptag.IsInt());
			//	Stats.uUpDatarate = temptag.GetInt();
				break;

			default:
				DebugLog(_T("Unknown Tag Voodoo File Statistics Packet: %s"), temptag.GetFullInfo());
		}
	}
}

// NEO: VOODOOs - [VoodooSearchForwarding]
//////////////////////////
// Search forwarding

void CVoodooSocket::SendNewSearch(SSearchParams* pParams)
{
	CSafeMemFile data(128);
	data.WriteUInt8((uint8)pParams->eType);
	data.WriteUInt32(pParams->dwSearchID);
	data.WriteString(pParams->strExpression);
	data.WriteString(pParams->strFileType);
	data.WriteString(pParams->strMinSize);
	if(Use64Size())
		data.WriteUInt64(pParams->ullMinSize);
	else
		data.WriteUInt32((uint32)pParams->ullMinSize);
	data.WriteString(pParams->strMaxSize);
	if(Use64Size())
		data.WriteUInt64(pParams->ullMaxSize);
	else
		data.WriteUInt32((uint32)pParams->ullMaxSize);
	data.WriteUInt32(pParams->uAvailability);
	data.WriteString(pParams->strExtension);
	data.WriteUInt32(pParams->uComplete);
	data.WriteString(pParams->strCodec);
	data.WriteUInt32(pParams->ulMinBitrate);
	data.WriteUInt32(pParams->ulMinLength);
	data.WriteString(pParams->strTitle);
	data.WriteString(pParams->strAlbum);
	data.WriteString(pParams->strArtist);
	data.WriteUInt8(pParams->bUnicode);
	SendPacket(data,OP_SEARCH_REQUEST);
}

extern LPCTSTR _aszInvKadKeywordChars;
void CVoodooSocket::NewSearchRecived(CSafeMemFile &packet)
{
	SSearchParams pParams;
	pParams.eType = (ESearchType)packet.ReadUInt8();
	pParams.dwSearchID = packet.ReadUInt32();
	pParams.strExpression = packet.ReadString(true);
	pParams.strFileType = packet.ReadString(true);
	pParams.strMinSize = packet.ReadString(true);
	if(Use64Size())
		pParams.ullMinSize = packet.ReadUInt64();
	else
		pParams.ullMinSize = packet.ReadUInt32();	
	pParams.strMaxSize = packet.ReadString(true);
	if(Use64Size())
		pParams.ullMaxSize = packet.ReadUInt64();
	else
		pParams.ullMaxSize = packet.ReadUInt32();
	pParams.uAvailability = packet.ReadUInt32();
	pParams.strExtension = packet.ReadString(true);
	pParams.uComplete = packet.ReadUInt32();
	pParams.strCodec = packet.ReadString(true);
	pParams.ulMinBitrate = packet.ReadUInt32();
	pParams.ulMinLength = packet.ReadUInt32();
	pParams.strTitle = packet.ReadString(true);
	pParams.strAlbum = packet.ReadString(true);
	pParams.strArtist = packet.ReadString(true);
	pParams.bUnicode = I2B(packet.ReadUInt8());

	if (pParams.eType == SearchTypeEd2kServer || pParams.eType == SearchTypeEd2kGlobal)
	{
		if (!theApp.serverconnect->IsConnected()){
			SendSearchState(pParams.dwSearchID, SS_OFFLINE);
			return;
		}

		bool bServerSupports64Bit = theApp.serverconnect->GetCurrentServer() != NULL
								&& (theApp.serverconnect->GetCurrentServer()->GetTCPFlags() & SRV_TCPFLG_LARGEFILES);
		bool bPacketUsing64Bit = false;
		CSafeMemFile data(100);
		if (!GetSearchPacket(&data, &pParams, bServerSupports64Bit, &bPacketUsing64Bit) || data.GetLength() == 0){
			SendSearchState(pParams.dwSearchID, SS_ERROR, SE_GENERIC);
			return;
		}

		m_bHaveMoreResults = false;
		theApp.emuledlg->searchwnd->m_pwndResults->CancelEd2kSearch();

		theApp.searchlist->SetCurED2KSearchID(NULL);

		Packet* packet = new Packet(&data);
		packet->opcode = OP_SEARCHREQUEST;
		
		if (thePrefs.GetDebugServerTCPLevel() > 0)
			Debug(_T(">>> Sending OP__SearchRequest\n"));
		theStats.AddUpDataOverheadServer(packet->size);
		theApp.serverconnect->SendPacket(packet,false);

		if (pParams.eType == SearchTypeEd2kGlobal && theApp.serverconnect->IsUDPSocketAvailable())
			theApp.emuledlg->searchwnd->m_pwndResults->StartGlobalSearch(packet,bPacketUsing64Bit);
		else
			delete packet;

		SSearchMaster Master;
		Master.MasterSocket = this;
		Master.MasterID = pParams.dwSearchID;
		Master.KillTimer = NULL;
		theApp.voodoo->m_SearchMasterMap.SetAt(NULL,Master);
	}
	else if(pParams.eType == SearchTypeKademlia)
	{
		if (!Kademlia::CKademlia::IsConnected()){ // NEO: EGS - [ExtendetGlobalSearch]
			SendSearchState(pParams.dwSearchID, SS_OFFLINE);
			return;
		}

		int iPos = 0;
		pParams.strKeyword = pParams.strExpression.Tokenize(_T(" "), iPos);
		pParams.strKeyword.Trim();
		if (pParams.strKeyword.IsEmpty() || pParams.strKeyword.FindOneOf(_aszInvKadKeywordChars) != -1){
			SendSearchState(pParams.dwSearchID, SS_ERROR, SE_INVALID);
			return;
		}

		CSafeMemFile data(100);
		if (!GetSearchPacket(&data, &pParams, true, NULL)/* || (!pParams.strBooleanExpr.IsEmpty() && data.GetLength() == 0)*/){
			SendSearchState(pParams.dwSearchID, SS_ERROR, SE_GENERIC);
			return;
		}

		LPBYTE pSearchTermsData = NULL;
		UINT uSearchTermsSize = (UINT)data.GetLength();
		if (uSearchTermsSize){
			pSearchTermsData = new BYTE[uSearchTermsSize];
			data.SeekToBegin();
			data.Read(pSearchTermsData, uSearchTermsSize);
		}

		Kademlia::CSearch* pSearch = NULL;
		try{
			pSearch = Kademlia::CSearchManager::PrepareFindKeywords(pParams.bUnicode, pParams.strKeyword, uSearchTermsSize, pSearchTermsData);
			delete pSearchTermsData;
			if (!pSearch){
				ASSERT(0);
				return;
			}
		}catch (CString strException){
			SendSearchState(pParams.dwSearchID, SS_ERROR, SE_KAD);
			return;
		}
		DWORD dwSlaveSearchID = pSearch->GetSearchID();

		SSearchMaster Master;
		Master.MasterSocket = this;
		Master.MasterID = pParams.dwSearchID;
		Master.KillTimer = NULL;
		theApp.voodoo->m_SearchMasterMap.SetAt(dwSlaveSearchID,Master);
	}
	else
		return; // unsupported search type
}

void CVoodooSocket::SendSearchState(DWORD dwSearchID, uint8 uState, uint32 uValue)
{
	CSafeMemFile data(128);
	data.WriteUInt32(dwSearchID);
	data.WriteUInt8(uState);
	data.WriteUInt32(uValue);
	SendPacket(data,OP_SEARCH_STATE);
}

void CVoodooSocket::SearchStateRecived(CSafeMemFile &packet)
{
	DWORD dwSearchID = packet.ReadUInt32();
	uint8 uState = packet.ReadUInt8();
	uint32 uValue = packet.ReadUInt32();

	switch(uState){
		case SS_ERROR:
			switch(uValue)
			{
				case SE_GENERIC:
					ModLog(LOG_ERROR | LOG_STATUSBAR,GetResString(IDS_X_GENERIC_VOODOO_SEARCH_ERROR),DbgGetClientInfo());
					break;
				case SE_INVALID:
					ModLog(LOG_WARNING | LOG_STATUSBAR,GetResString(IDS_X_VOODOO_PREFIX),StrLine(GetResString(IDS_KAD_SEARCH_KEYWORD_INVALID),_aszInvKadKeywordChars));
					break;
				case SE_KAD:
					ModLog(LOG_WARNING | LOG_STATUSBAR,GetResString(IDS_X_VOODOO_PREFIX),GetResString(IDS_KAD_SEARCH_KEYWORD_ALREADY_SEARCHING));
					break;
				default:
					DebugLogWarning(_T("Unknown State Voodoo Search State Packet: %u;%u"), (uint32)uState, uValue);
			}
			theApp.emuledlg->searchwnd->m_pwndResults->FinishVoodooSearch(this,dwSearchID,SF_ERROR);
			break;
		case SS_OFFLINE:
			ModLog(LOG_WARNING | LOG_STATUSBAR,GetResString(IDS_X_VOODOO_ERR_NOTCONNECTED),DbgGetClientInfo());
			theApp.emuledlg->searchwnd->m_pwndResults->FinishVoodooSearch(this,dwSearchID,SF_ERROR);
			break;
		case SS_FINISH:
			ModLog(LOG_STATUSBAR,GetResString(IDS_X_VOODOO_SEARCH_DONE),DbgGetClientInfo());
			theApp.emuledlg->searchwnd->m_pwndResults->FinishVoodooSearch(this,dwSearchID,uValue);
			break;
		default:
			DebugLogWarning(_T("Unknown State Voodoo Search State Packet: %u;%u"), (uint32)uState, uValue);
	}
}

void CVoodooSocket::SendSearchCommand(DWORD dwSearchID, uint8 uCommand)
{
	CSafeMemFile data(128);
	data.WriteUInt8(uCommand);
	data.WriteUInt32(dwSearchID);
	SendPacket(data,OP_SEARCH_COMMAND);
}

void CVoodooSocket::SearchCommandRecived(CSafeMemFile &packet)
{
	uint8 uCommand = packet.ReadUInt8();
	DWORD dwSearchID = packet.ReadUInt32();
	switch(uCommand){
		case SC_HOLD:
			theApp.emuledlg->searchwnd->m_pwndResults->HoldGlobalSearch();
			break;
		case SC_CANCEL:
			if(dwSearchID){ // we synchronise only KAD searches becouse only thay can be more than one at once
				SSearchMaster Master;
				DWORD SlaveID;
				for (POSITION pos = theApp.voodoo->m_SearchMasterMap.GetStartPosition();pos != 0;){
					theApp.voodoo->m_SearchMasterMap.GetNextAssoc(pos,SlaveID,Master);
					if(Master.MasterID == dwSearchID){
						Kademlia::CSearchManager::StopSearch(SlaveID, false);
						theApp.voodoo->m_SearchMasterMap.RemoveKey(SlaveID);
						break;
					}
				}
			}else
				theApp.emuledlg->searchwnd->m_pwndResults->CancelEd2kSearch();
			break;
		case SC_MORE:
			theApp.emuledlg->searchwnd->m_pwndResults->SearchMore();
			break;
		default:
			DebugLogWarning(_T("Unknown Command Voodoo Search Command Packet: %u"), uCommand);
	}
}

void CVoodooSocket::SendSearchResult(DWORD dwSearchID,ESearchType eType,CSafeMemFile &packet, uint32 nServerIP, uint16 nServerPort)
{
	CSafeMemFile data(128);
	data.WriteUInt32(dwSearchID);
	data.WriteUInt8((uint8)eType);
	data.WriteUInt32(nServerIP);
	data.WriteUInt16(nServerPort);
	data.Write(packet.GetBuffer(),(UINT)packet.GetLength());
	SendPacket(data,OP_SEARCH_RESULT);
}

void CVoodooSocket::SearchResultRecived(CSafeMemFile &packet)
{
	DWORD dwSearchID = packet.ReadUInt32();
	ESearchType eType = (ESearchType)packet.ReadUInt8();
	uint32 nServerIP = packet.ReadUInt32();
	uint16 nServerPort = packet.ReadUInt16();

	CSafeMemFile data(128);
	data.Write(packet.GetBuffer()+4+1+4+2,(UINT)packet.GetLength()-4-1-4-2);
	data.SeekToBegin();
	switch(eType)
	{
		case SearchTypeEd2kServer:
			theApp.searchlist->ProcessSearchAnswer(data.GetBuffer(),(UINT)data.GetLength(),true,nServerIP,nServerPort,&m_bHaveMoreResults);
			break;
		case SearchTypeEd2kGlobal:{
			UINT count = theApp.searchlist->ProcessUDPSearchAnswer(data,true,nServerIP,nServerPort);
			theApp.emuledlg->searchwnd->m_pwndResults->CheckGlobalSearch(this,count);
			break;
		}
		case SearchTypeKademlia:{
			CSearchFile* tempFile = new CSearchFile(&data, true, dwSearchID, 0, 0, 0, true);
			theApp.searchlist->AddToList(tempFile);
			break;
		}
		default:
			return;
	}
}
// NEO: VOODOOs END

// NEO: VOODOOx - [VoodooSourceExchange]
/////////////////////////
// Source Exchange

void CVoodooSocket::RequestSourceList(CPartFile* pFile)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;
	SendPacket(data,OP_SOURCE_LIST_REQUEST);
}

void CVoodooSocket::SourceListRequestRecived(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet,IsMaster());
	if(!pFile)
		return;
	SendSourceList(pFile);
}

void CVoodooSocket::SendSingleSource(CPartFile* pFile, CUpDownClient* sClient)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;

	data.WriteUInt16(1); // single source packet
	data.WriteUInt32(sClient->GetUserIDHybrid());
	data.WriteUInt16(sClient->GetUserPort());
	data.WriteHash16(sClient->GetUserHash());
	sClient->WriteNeoXSTags(&data);

	SendPacket(data,OP_SOURCE_LIST);
}

void CVoodooSocket::SendSourceList(CPartFile* pFile)
{
	CSafeMemFile data(128);
	if(!WriteFileID(data,pFile,Use64Size()))
		return;

	UINT nCount = 0;
	ULONG uCountFilePos = (ULONG)data.GetPosition();
	data.WriteUInt16((uint16)nCount);
	
	for (POSITION pos = pFile->srclist.GetHeadPosition();pos != 0;){
		const CUpDownClient* cur_src = pFile->srclist.GetNext(pos);
		if (!cur_src->IsValidSource())
			continue;

		data.WriteUInt32(cur_src->GetUserIDHybrid());	// 4
		data.WriteUInt16(cur_src->GetUserPort());		// 2
		data.WriteHash16(cur_src->GetUserHash());		// 16
		cur_src->WriteNeoXSTags(&data);					// 51
														// 73 * 21,546.0 = 1,572,864.0
		nCount++;
		// just in case
		if(nCount == 0xFFFF || data.GetLength() > 1572864) // Its all on lan so why not up to 1.5 MB :p
			break;
	}
	if (!nCount)
		return;

	data.Seek(uCountFilePos, CFile::begin);
	data.WriteUInt16((uint16)nCount);
	data.Seek(0, CFile::end);

	SendPacket(data,OP_SOURCE_LIST);
}

void CVoodooSocket::SourceListRecived(CSafeMemFile &packet)
{
	CPartFile* pFile = GetDownloadingFile(packet,IsMaster());
	if(!pFile || !pFile->PartPrefs->IsVoodooXS())
		return;

	uint16 nCount = packet.ReadUInt16();

	CUpDownClient::tNeoXSTags NeoXSTags;
	for (UINT i = 0; i < nCount; i++)
	{
		uint32 dwID = packet.ReadUInt32();
		uint16 nPort = packet.ReadUInt16();
		uchar achUserHash[16];
		packet.ReadHash16(achUserHash);
		CUpDownClient::ReadNeoXSTags(&packet, &NeoXSTags);

		// Clients send ID's in the Hyrbid format so highID clients with *.*.*.0 won't be falsely switched to a lowID..
		uint32 dwIDED2K = ntohl(dwID);

		if(!pFile->CheckSourceID(dwIDED2K))
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received from server"), ipstr(userid));
			return;
		}

		// additionally check for LowID and own IP
#ifdef NATTUNNELING // NEO: NATT - [NatTraversal]
		if (!pFile->CanAddSource(dwID, nPort, NeoXSTags.dwServerIP, NeoXSTags.nServerPort, NULL, false, NeoXSTags.uSupportsNatTraversal != 0))
#else
		if (!pFile->CanAddSource(dwID, nPort, NeoXSTags.dwServerIP, NeoXSTags.nServerPort, NULL, false))
#endif //NATTUNNELING // NEO: NATT END
		{
			//if (thePrefs.GetLogFilteredIPs())
			//	AddDebugLogLine(false, _T("Ignored source (IP=%s) received via source exchange"), ipstr(dwIDED2K));
			return;
		}

		//if (pFile->GetMaxSources() > pFile->GetSourceCount())
		// NEO: XSC - [ExtremeSourceCache]
		bool bCache = false;
		if ( (bCache = !(pFile->GetMaxSources() > pFile->GetSourceCount())) == false // NEO: SRT - [SourceRequestTweaks]
			|| pFile->PartPrefs->UseSourceCache() && pFile->GetSourceCacheSourceLimit() > pFile->GetSrcStatisticsValue(DS_CACHED))
		// NEO: XSC END
		{
			CUpDownClient* newsource = new CUpDownClient(pFile, nPort, dwID, 0, 0, false);
			newsource->SetUserHash(achUserHash);
			NeoXSTags.Attach(newsource);

			newsource->SetSourceFrom(SF_VOODOO);
			// NEO: XSC - [ExtremeSourceCache]
			if(bCache)
				newsource->SetDownloadState(DS_CACHED);
			// NEO: XSC END
			theApp.downloadqueue->CheckAndAddSource(pFile, newsource);
		}
		else
			break;
	}
}
// NEO: VOODOOx END

// NEO: VOODOOn - [VoodooForNeo]
/////////////////////////////
// File Properties

void CVoodooSocket::SendNeoPreferences(EFilePrefsLevel Kind, CKnownFile* kFile, int Cat)
{
	CKnownPreferences* KnownPrefs = NULL;
	CPartPreferences* PartPrefs = NULL;

	CSafeMemFile data(128);
	data.WriteUInt8((uint8)Kind);

	switch(Kind)
	{
	case CFP_GLOBAL:
		KnownPrefs = &NeoPrefs.KnownPrefs;
		PartPrefs = &NeoPrefs.PartPrefs;
		break;
	case CFP_CATEGORY:{
		uint16 CatData = (uint16)((Cat & 0x7FFF) | ((thePrefs.GetCatCount() <= Cat) ? 0x8000 : 0));
		data.WriteUInt16(CatData);

		Category_Struct* Category = thePrefs.GetCategory(Cat);
		if(!Category)
			return;

		KnownPrefs = Category->KnownPrefs;
		PartPrefs = Category->PartPrefs;
		break;
	}
	case CFP_FILE:{
		CPartFile* pFile = kFile->IsPartFile() ? (CPartFile*)kFile : NULL;

		if(!WriteFileID(data,kFile,Use64Size()))
			return;

		KnownPrefs = kFile->KnownPrefs->IsFilePrefs() ? kFile->KnownPrefs : NULL;
		PartPrefs = (pFile && pFile->PartPrefs->IsFilePrefs()) ? pFile->PartPrefs : NULL;
		break;
	}
	default:
		ASSERT(0);
		return;
	}

	if(PartPrefs){
		data.WriteUInt8(PARTPREFSFILE_VERSION);

		ULONGLONG pos = data.GetPosition();
		data.WriteUInt16(0);

		PartPrefs->Save(&data);
		
		data.Seek(pos, CFile::begin);
		ASSERT((data.GetLength() - (data.GetPosition()+2)) < 0xFFFF);
		data.WriteUInt16((uint16)(data.GetLength() - (data.GetPosition()+2)));
		data.Seek(0, CFile::end);
	}
	if(KnownPrefs){
		data.WriteUInt8(KNOWNPREFSFILE_VERSION);

		ULONGLONG pos = data.GetPosition();
		data.WriteUInt16(0);

		KnownPrefs->Save(&data);
		
		data.Seek(pos, CFile::begin);
		ASSERT((data.GetLength() - (data.GetPosition()+2)) < 0xFFFF);
		data.WriteUInt16((uint16)(data.GetLength() - (data.GetPosition()+2)));
		data.Seek(0, CFile::end);
	}

	SendPacket(data,OP_NEO_PREFERENCES);
}

void CVoodooSocket::NeoPreferencesRecived(CSafeMemFile &packet)
{

	CKnownPreferences* KnownPrefs = NULL;
	CPartPreferences* PartPrefs = NULL;

	EFilePrefsLevel Kind  = (EFilePrefsLevel)packet.ReadUInt8();

	int Cat = -1;

	CPartFile* pFile = NULL;
	CKnownFile* kFile = NULL;

	switch(Kind)
	{
	case CFP_GLOBAL:
		KnownPrefs = &NeoPrefs.KnownPrefs;
		PartPrefs = &NeoPrefs.PartPrefs;
		break;
	case CFP_CATEGORY:{
		uint16 CatData = packet.ReadUInt16();
		Cat = CatData & 0x7FFFF;
		bool Shared = (CatData & 0x8000) != 0;

		while(thePrefs.GetFullCatCount() <= Cat){ // add needed cat's
			int iNewIndex = theApp.emuledlg->transferwnd->AddCategory(_T("?"),thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),_T(""),_T(""),!Shared,Shared);
			// NEO: NSC - [NeoSharedCategories]
			if(!Shared){
				theApp.knownfiles->ShiftCatParts(iNewIndex); 
				theApp.emuledlg->sharedfileswnd->Reload(true);
			}
			// NEO: NSC END
			theApp.emuledlg->searchwnd->UpdateCatTabs();
		}

		Category_Struct* Category = thePrefs.GetCategory(Cat);
		ASSERT(Category);

		KnownPrefs = Category->KnownPrefs ? Category->KnownPrefs : new CKnownPreferencesEx(CFP_CATEGORY);
		PartPrefs = Category->PartPrefs ? Category->PartPrefs : new CPartPreferencesEx(CFP_CATEGORY);
		break;
	}
	case CFP_FILE:{
		FileID fileID(packet,Use64Size());

		pFile = theApp.downloadqueue->GetFileByID(fileID.Hash);
		kFile = pFile ? pFile : theApp.knownfiles->FindKnownFileByID(fileID.Hash);

		if(!kFile){
			if(fileID != m_LastErrFile)
				SendFileUnavalible(fileID,RET_UNKNOWN);
			else if (thePrefs.GetVerbose())
				DebugLog(_T("Droped operation for Erroneous file %s, from client %s "),md4str(fileID.Hash),DbgGetClientInfo());
			//if(m_uType != CT_ED2K)
			//	ModLog(LOG_WARNING, GetResString(IDS_X_VOODOO_FILE_FAILED_ERR),md4str(fileID.Hash),DbgGetClientInfo());
			return;
		}else if(kFile->GetFileSize() != fileID.Size){
			SendFileUnavalible(fileID,RET_BADSIZE);
			ModLog(LOG_ERROR,GetResString(IDS_X_VOODOO_FILE_SIZE_FAILED),kFile->GetFileName(),fileID.Size,kFile->GetFileSize(),DbgGetClientInfo());
			return;
		}else if(pFile){
			if(!pFile->IsVoodooFile()){
				ModLog(LOG_WARNING,GetResString(IDS_X_NOT_VOODOO_FILE),DbgGetClientInfo(),pFile->GetFileName());
				SendFileUnavalible(fileID, RET_REAL);
				return;
			}
		}else if(kFile){
			if(kFile->IsRealFile()){
				ModLog(GetResString(IDS_X_NOT_VOODOO_FILE),DbgGetClientInfo(),kFile->GetFileName());
				//SendFileUnavalible(fileID, RET_REAL);
				return;
			}
		}

		KnownPrefs = kFile->KnownPrefs->IsFilePrefs() ? kFile->KnownPrefs : new CKnownPreferencesEx(CFP_FILE);
		PartPrefs = pFile ? (pFile->PartPrefs->IsFilePrefs() ? pFile->PartPrefs : new CPartPreferencesEx(CFP_FILE)) : NULL;
		break;
	}
	default:
		//ASSERT(0);
		return;
	}
	
	// reset old tweaks
	if(Kind != CFP_GLOBAL)
	{
		KnownPrefs->ResetTweaks();
		if(PartPrefs)
			PartPrefs->ResetTweaks();
	}

	uint8 segment;
	UINT length;
	while (packet.GetLength()-packet.GetPosition())
	{
		segment = packet.ReadUInt8();
		length = packet.ReadUInt16();
		if(length == 0xFFFF) // just in case
			length = packet.ReadUInt32();

		switch(segment)
		{
			case PARTPREFSFILE_VERSION:{
				if(PartPrefs){
					PartPrefs->Load(&packet);
					PartPrefs->CheckTweaks();
				}else{
					packet.Seek(length, CFile::current);
					ASSERT(0);
				}
				break;
			}
			case KNOWNPREFSFILE_VERSION:{
				if(KnownPrefs){
					KnownPrefs->Load(&packet);
					KnownPrefs->CheckTweaks();
				}else{
					packet.Seek(length, CFile::current);
					ASSERT(0);
				}
				break;
			}
			default:
				if(packet.GetPosition() + length > packet.GetLength())
					AfxThrowFileException(CFileException::endOfFile, 0, _T("MemoryFile"));
				DebugLog(_T("Unknown NEO File segment ID 0x%02x received"), segment);
				packet.Seek(length, CFile::current);
		}
	}

	if(Kind == CFP_FILE)
	{	
		// valiate pointers and update (!)
		kFile->UpdateKnownPrefs(KnownPrefs);
		if(pFile)
			pFile->UpdatePartPrefs(PartPrefs);
	}
	else if(Kind == CFP_CATEGORY)
	{
		// valiate pointers and update (!)
		theApp.downloadqueue->UpdatePartPrefs(PartPrefs, KnownPrefs, (UINT)Cat); // may delete PartPrefs
		theApp.knownfiles->UpdateKnownPrefs(KnownPrefs, (UINT)Cat); // may delete KnownPrefs

		thePrefs.SaveCats();
	}
	else if(Kind == CFP_GLOBAL)
		NeoPrefs.Save();
}

//////////////////////////
// Download commands

void CVoodooSocket::SendDownloadCommand(CTypedPtrList<CPtrList, CPartFile*>& FileQueue, uint8 uCommand, uint32 Flag1, uint32 Flag2)
{
	CSafeMemFile data(128);
	data.WriteUInt8(uCommand);
	data.WriteUInt32(Flag1);
	data.WriteUInt32(Flag2);

	uint32 RecordsNumber = 0;
	ULONG RecordsNumberFilePos = (ULONG)data.GetPosition();
	data.WriteUInt32(RecordsNumber);

	for (POSITION pos = FileQueue.GetHeadPosition(); pos != 0; ){
		CPartFile* pFile = FileQueue.GetNext(pos);
		if(!pFile->KnownPrefs->IsEnableVoodoo())
			continue;
		if(pFile->GetMasterDatas(this))
			continue;
		if(!WriteFileID(data,pFile,Use64Size()))
			continue;
		RecordsNumber++;
	}

	data.Seek(RecordsNumberFilePos, CFile::begin);
	data.WriteUInt32(RecordsNumber);
	data.Seek(0, CFile::end);

	SendPacket(data,OP_DOWNLOAD_COMMAND);
}

void CVoodooSocket::DownloadCommandRecived(CSafeMemFile &packet)
{
	uint8 uCommand = packet.ReadUInt8();
	uint32 Flag1= packet.ReadUInt32();
	uint32 Flag2 = packet.ReadUInt32();

	CTypedPtrList<CPtrList, CPartFile*> FileQueue;

	uint32 RecordsNumber = packet.ReadUInt32();
	for (uint32 i = 0; i < RecordsNumber; i++)
	{
		CPartFile* pFile = GetDownloadingFile(packet,true);
		if(!pFile)
			continue;
		FileQueue.AddTail(pFile);
	}

	theApp.downloadqueue->ExecuteNeoCommand(FileQueue,uCommand,(uint8)Flag1); 
	UNREFERENCED_PARAMETER(Flag2); // flag 2 is not used yet
}


// NEO: VOODOOn END

//////////////////////////
// Process

void CVoodooSocket::Process()
{
	// Send stats
	if(m_uSupportsStatistics > 0){
		if(IsMaster()){
			if(::GetTickCount() - m_uLastStatUpdate > STAT_REFRESH){
				SendStatistics();
				m_uLastStatUpdate = ::GetTickCount();
			}
		}
		if(IsSlave()){
			if(::GetTickCount() - m_Stats.uLastStatUpdate > STAT_REFRESH * 10){ // to long delay, zero stats
				m_Stats.uUpDatarate = 0;
				m_Stats.uDownDatarate = 0;
			}
		}
	}
}

///////////////////////////////
// Debug helpers

void CVoodooSocket::PacketToDebugLogLine(LPCTSTR protocol, const uchar* packet, uint32 size, UINT opcode)
{
	if (thePrefs.GetVerbose())
	{
		CString buffer; 
	    buffer.Format(_T("Unknown %s Protocol Opcode: 0x%02x, Size=%u, Data=["), protocol, opcode, size);
		uint32 i;
		for (i = 0; i < size && i < 50; i++){
			if (i > 0)
				buffer += _T(' ');
			TCHAR temp[3];
		    _stprintf(temp, _T("%02x"), packet[i]);
			buffer += temp;
		}
		buffer += (i == size) ? _T("]") : _T("..]");
		buffer += _T("; ");
		buffer += DbgGetClientInfo();
		DebugLogWarning(_T("%s"), buffer);
	}
}

CString CVoodooSocket::DbgGetClientInfo()
{
	CString str;
	SOCKADDR_IN sockAddr = {0};
	int nSockAddrLen = sizeof(sockAddr);
	GetPeerName((SOCKADDR*)&sockAddr, &nSockAddrLen);
	str.Format(m_sName.IsEmpty() ? _T("???") : m_sName);
	if (sockAddr.sin_addr.S_un.S_addr != 0)
		str.AppendFormat(_T(" IP=%s"), ipstr(sockAddr.sin_addr));
	return str;
}

CString CVoodooSocket::GetClientDesc(uint8 uAction)
{
	return GetResString((uAction == VA_PARTNER) ? IDS_X_VOODOO_PARTNER : (uAction == VA_MASTER) ? IDS_X_VOODOO_MASTER : (uAction == VA_SLAVE) ? IDS_X_VOODOO_SLAVE : IDS_X_VOODOO_UNKNOWN);
}

CString CVoodooSocket::GetClientDesc()
{
	return GetResString((IsMaster() && IsSlave()) ? IDS_X_VOODOO_PARTNER : IsSlave() ? IDS_X_VOODOO_SLAVE : IsMaster() ? IDS_X_VOODOO_MASTER : IDS_X_VOODOO_UNKNOWN);
}

#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
