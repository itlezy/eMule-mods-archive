//this file is part of NeoMule
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
#include <ws2tcpip.h>
#include <Iphlpapi.h>
#include "emule.h"
#include "Neo/NeoPreferences.h"
#include "Preferences.h"
#include "LanCast.h"
#include "LanSearch.h"
#include "packets.h"
#include "ED2KLink.h"
#include "otherfunctions.h"
#include "KnownFile.h"
#include "PartFile.h"
#include "UpDownClient.h"
#include "DownloadQueue.h"
#include "SharedFileList.h"
#include "SafeFile.h"
#include "Log.h"
#include "Clientlist.h"
#include "Searchlist.h"
#include "Neo/NeoOpcodes.h"
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --

// This is the multicast port and must be the same for all lancast using clients on the same LAN
//#define LANCAST_PORT	5000

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->

#ifndef NEO_BC
#error Neo LanCast requierd Neo Bandwidth Control!
#endif

CLanCast::CLanCast()
{
	bStarted = false;
	m_uSubNetMask = 0;
	m_uAdapterIP = 0;
	udp_timer = NULL;
}

bool CLanCast::IsLanIP(uint32 dwUserIP)
{
	if (!NeoPrefs.IsLanSupportEnabled())
		return false;

	static uint32 localhost = inet_addr("127.0.0.1");
	if(dwUserIP == localhost) // X!
		return true;

	if (!NeoPrefs.IsCustomLanCastAdapter()){
		for (POSITION pos = interface_list.GetHeadPosition();pos;) {
			multicast_interface mcif = interface_list.GetNext(pos);
			if ((mcif.address & mcif.subnet) == (dwUserIP & mcif.subnet))
				return true;
		}
	}else{
		if (!m_uAdapterIP || !m_uSubNetMask)
			return false;
		if ((m_uAdapterIP & m_uSubNetMask) == (dwUserIP & m_uSubNetMask))
			return true;
	}
	return false;
}

bool CLanCast::SendPacket(Packet* packet, uint32 addr, bool bSrc)
{
	char* pcSendBuffer = new char[packet->size+2];

	// Standard UDP Header
	memcpy(pcSendBuffer,packet->GetUDPHeader(),2);
	// The packet Data
	memcpy(pcSendBuffer+2,packet->pBuffer,packet->size);
	// Enter the proper Source IP
	if(bSrc)
		memcpy(pcSendBuffer+2,&addr,4);

	SOCKADDR_IN iface;
	memset(&iface, 0, sizeof(iface));
	iface.sin_family = AF_INET;
	iface.sin_addr.s_addr = addr;
	m_SendSocket.Bind((SOCKADDR*)&iface, sizeof(iface));
	// Send the LanCast packet

	bool ret = (m_SendSocket.SendTo(pcSendBuffer, packet->size+2, (SOCKADDR*)&m_saHostGroup, sizeof(m_saHostGroup), MSG_DONTROUTE) != SOCKET_ERROR);
	
	delete[] pcSendBuffer;
	return ret;
}

void CLanCast::BroadcastPacket(Packet* packet, bool bSrc)
{
	if (!NeoPrefs.IsCustomLanCastAdapter()){ 
		for (POSITION pos = interface_list.GetHeadPosition();pos;){
			SendPacket(packet,interface_list.GetNext(pos).address,bSrc);
		}
	}else{
		SendPacket(packet,m_uAdapterIP,bSrc);
	}
	delete packet;
}

void CLanCast::BroadcastHash(CKnownFile* cur_file)
{
	CSafeMemFile file(100);
	file.WriteUInt32(0); // IP will be fild by the sending function
	file.WriteUInt16(thePrefs.GetPort());
	file.WriteHash16(cur_file->GetFileHash());

	Packet* packet = new Packet(&file,OP_LANCASTPROT);
	packet->opcode = OP_HASH;

	BroadcastPacket(packet,true);
}

void CLanCast::ReceiveHash(uint8* pachPacket, uint32 nSize)
{
	CSafeMemFile offeredfiles((BYTE*)pachPacket,nSize);

	uint32 dwIP = offeredfiles.ReadUInt32();
	uint16 nPort = offeredfiles.ReadUInt16(); 
	uchar filehash[16];
	offeredfiles.ReadHash16(filehash);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if(NeoPrefs.IsVoodooEnabled() && theApp.voodoo->IsVoodooClientEd2k(dwIP,nPort))
		return;
#endif // VOODOO // NEO: VOODOO END

	CPartFile* file = theApp.downloadqueue->GetFileByID((uchar*)filehash);

	if (file) 
	{
		// Create a new Updown client for this source (with no server info), userid is the LAN ip address
		CUpDownClient* newsource = new CUpDownClient(file,nPort,dwIP,0,0,true);
		newsource->SetSourceFrom(SF_LANCAST);

		// Add this LAN client to the download queue, this also will check for duplicate sockets and "merge" them
		theApp.downloadqueue->CheckAndAddSource(file,newsource);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CLanCast::GetSources(CPartFile* pFile)
{
	CSafeMemFile file(100);

	file.WriteHash16(pFile->GetFileHash());
	if(pFile->IsLargeFile()){
		file.WriteUInt32(0); // indicates that a uint64 follows
		file.WriteUInt64(pFile->GetFileSize());
	}else{
		file.WriteUInt32((uint32)(uint64)pFile->GetFileSize());
	}

	Packet* packet = new Packet(&file,OP_LANCASTPROT);
	packet->opcode = OP_HASHSEARCH;

	BroadcastPacket(packet);
}

void CLanCast::ReceiveSourceRequest(uint8* pachPacket, uint32 nSize)
{
	CSafeMemFile request((BYTE*)pachPacket,nSize);

	uint64 filesize;
	uchar filehash[16];
	request.ReadHash16(filehash);
	filesize = request.ReadUInt32();
	if(filesize == 0)
		filesize = request.ReadUInt64();

	CKnownFile* file = theApp.sharedfiles->GetFileByID((uchar*)filehash);
	if(!file)
		file = theApp.downloadqueue->GetFileByID((uchar*)filehash);

	if (file && file->GetFileSize() == filesize && file->KnownPrefs->IsEnableLanCast())
		SendSourceAnswer(file);
}

void CLanCast::SendSourceAnswer(CKnownFile* kFile)
{
	CSafeMemFile file(100);

	file.WriteUInt32(0); // IP will be fild by the sending function
	file.WriteUInt16(thePrefs.GetPort());

	file.WriteHash16(kFile->GetFileHash());
	if(kFile->IsLargeFile()){
		file.WriteUInt32(0); // indicates that a uint64 follows
		file.WriteUInt64(kFile->GetFileSize());
	}else{
		file.WriteUInt32((uint32)(uint64)kFile->GetFileSize());
	}

	Packet* packet = new Packet(&file,OP_LANCASTPROT);
	packet->opcode = OP_HASHSEARCHRESPONSE;

	BroadcastPacket(packet,true);
}

void CLanCast::ReceiveSourceAnswer(uint8* pachPacket, uint32 nSize)
{
	CSafeMemFile answer((BYTE*)pachPacket,nSize);

	uint32 dwIP = answer.ReadUInt32();
	uint16 nPort = answer.ReadUInt16(); 

	uint64 filesize;
	uchar filehash[16];
	answer.ReadHash16(filehash);
	filesize = answer.ReadUInt32();
	if(filesize == 0)
		filesize = answer.ReadUInt64();

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	if(NeoPrefs.IsVoodooEnabled() && theApp.voodoo->IsVoodooClientEd2k(dwIP,nPort))
		return;
#endif // VOODOO // NEO: VOODOO END

	CPartFile* file = theApp.downloadqueue->GetFileByID((uchar*)filehash);

	if (file && file->GetFileSize() == filesize) 
	{
		// Create a new Updown client for this source (with no server info), userid is the LAN ip address
		CUpDownClient* newsource = new CUpDownClient(file,nPort,dwIP,0,0,true);
		newsource->SetSourceFrom(SF_LANCAST);

		// Add this LAN client to the download queue, this also will check for duplicate sockets and "merge" them
		theApp.downloadqueue->CheckAndAddSource(file,newsource);
	}
}

// File Seartch Implementation ////////////////////////////////////////////////////////////////////////////////////////////
void CLanCast::ReceiveFileSearch(uint8* pachPacket, uint32 nSize, uint32 sender)
{
	CSafeMemFile bio((BYTE*)pachPacket, nSize);
	SSearchRoot* pSearchRoot = new SSearchRoot;;
	try
	{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		_pstrDbgSearchExpr_ = (thePrefs.GetDebugServerSearchesLevel() > 0) ? new CString : NULL;
#endif
		pSearchRoot->pSearchTerms = CreateSearchTree(bio, pSearchRoot);
		if (_pstrDbgSearchExpr_) {
			Debug(_T("KadSearchTerm=%s\n"), *_pstrDbgSearchExpr_);
			delete _pstrDbgSearchExpr_;
			_pstrDbgSearchExpr_ = NULL;
		}
	}
	catch(...)
	{
		FreeTree(pSearchRoot->pSearchTerms);
		delete pSearchRoot;
		throw;
	}

#ifdef _DEBUG
	uint32 now = ::GetTickCount();
#endif
	CPtrList* list = PerformFileSearch(pSearchRoot);
	DEBUG_ONLY(ModLog(_T("Lancast search request processed in %u ms"), (::GetTickCount() - now)));
	if(list->GetCount())
		SendSearchResponce(list,sender);

	delete list;
	FreeTree(pSearchRoot->pSearchTerms);
	delete pSearchRoot;
}

CPtrList*  CLanCast::PerformFileSearch(const SSearchRoot* pSearch)
{
	CPtrList* list = new CPtrList;
	for (int i = 0; i < theApp.sharedfiles->GetCount(); i++)
	{
		CKnownFile* cur_file = theApp.sharedfiles->GetFileByIndex(i);

		if (cur_file && cur_file->KnownPrefs->IsEnableLanCast())
		{
			if(SearchRootMatch(pSearch,cur_file))
				list->AddTail((void*&)cur_file);
		}	
	}

	return list;
}

void CLanCast::SendSearchResponce(CPtrList* list, uint32 sender)
{
	// now create the memfile for the packet
	CSafeMemFile file(100);

	file.WriteUInt32(0); // IP will be fild by the sending function
	file.WriteUInt16(thePrefs.GetPort());
	file.WriteUInt32(sender);

	uint32 iTotalCount = list->GetCount();
	file.WriteUInt32(iTotalCount);
	while (list->GetCount())
	{
		theApp.sharedfiles->CreateOfferedFilePacket((CKnownFile*)list->GetHead(), &file, NULL);
		list->RemoveHead();
	}

	// create a packet and send it
	Packet* packet = new Packet(&file,OP_LANCASTPROT);
	packet->opcode = OP_FILESEARCHRESPONSE;

	BroadcastPacket(packet,true);
}

bool CLanCast::CheckSender(uint32 sender)
{
	if (!NeoPrefs.IsCustomLanCastAdapter()){ 
		for (POSITION pos = interface_list.GetHeadPosition();pos;){
			if(interface_list.GetNext(pos).address == sender)
				return true;
		}
	}else{
		if(m_uAdapterIP == sender)
			return true;
	}
	return false;
}

void CLanCast::ReceiveSearchResponce(uint8* pachPacket, uint32 nSize)
{
	CSafeMemFile offeredfiles((BYTE*)pachPacket,nSize);

	uint32 dwIP = offeredfiles.ReadUInt32();
	uint16 nPort = offeredfiles.ReadUInt16();
	uint32 dwSender = offeredfiles.ReadUInt32();

	if(!CheckSender(dwSender)) // The answer is not for us
		return;

	theApp.searchlist->ProcessSearchAnswer(pachPacket+(4+2+4),nSize-(4+2+4),true,0,0,NULL,dwIP,nPort);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
void CLanCast::SendVoodoo(uint8 uAction, uint8 uReply)
{
	CSafeMemFile file(100);

	file.WriteUInt32(0); // IP will be fild by the sending function
	file.WriteUInt16(NeoPrefs.GetVoodooPort());
	file.WriteUInt8(uAction);
	file.WriteUInt8(uReply);

	Packet* packet = new Packet(&file,OP_LANCASTPROT);
	packet->opcode = OP_VOODOOREQUEST;

	BroadcastPacket(packet,true);
}

void CLanCast::ReceiveVoodoo(uint8* pachPacket, uint32 nSize)
{
	if(!NeoPrefs.IsVoodooCastEnabled())
		return;

	CSafeMemFile request((BYTE*)pachPacket,nSize);

	uint32 dwIP = request.ReadUInt32();
	uint16 nPort = request.ReadUInt16(); 
	uint8  uAction = request.ReadUInt8();
	uint8  uReply = request.ReadUInt8();

	uint8 uResult = 0;
	if(uAction == VA_QUERY)
		uResult |= VA_QUERY;
	if((uAction & VA_SLAVE) && NeoPrefs.IsSlaveAllowed())
		uResult |= VA_SLAVE;
	if((uAction & VA_MASTER) && NeoPrefs.IsSlaveHosting())
		uResult |= VA_MASTER;

	if(uReply)
		theApp.voodoo->AddVoodooClient(dwIP,nPort,uAction);
	else if(uResult)
		SendVoodoo(uResult, TRUE);

}
#endif // VOODOO // NEO: VOODOO END

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CLanCast::~CLanCast()
{
}

void CLanCast::OnReceive(int /*nErrorCode*/)
{
	uint8 buffer[5000];
	SOCKADDR_IN sockAddr = {0};
	int iSockAddrLen = sizeof sockAddr;

	uint32 length = ReceiveFrom(buffer, sizeof buffer, (SOCKADDR*)&sockAddr, &iSockAddrLen);
	if (length == SOCKET_ERROR)
		return;

	if (!NeoPrefs.IsLancastEnabled())
		return;

	if (buffer[0] != OP_LANCASTPROT)
		return;

	// If this isnt a packet from ourselves
	uint32 addr = sockAddr.sin_addr.S_un.S_addr;
	if(CheckSender(addr))
		return;

	ProcessPacket(buffer+2,length-2,buffer[1],addr);
}

bool CLanCast::ProcessPacket(uint8* packet, uint32 size, uint8 opcode, uint32 sender){
	try{
		switch(opcode){
			case OP_HASH:{
				ReceiveHash(packet,size);
				break;
			}
			case OP_HASHSEARCH:{
				ReceiveSourceRequest(packet,size);
				break;
			}
			case OP_HASHSEARCHRESPONSE:{
				ReceiveSourceAnswer(packet,size);
				break;
			}
			case OP_FILESEARCH:{
				ReceiveFileSearch(packet,size,sender);
				break;
			}
			case OP_FILESEARCHRESPONSE:{
				ReceiveSearchResponce(packet,size);
				break;
			}
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
			case OP_VOODOOREQUEST:{
				ReceiveVoodoo(packet,size);
				break;
			}
#endif // VOODOO // NEO: VOODOO END
			default:
				DebugLogWarning(GetResString(IDS_X_LANCAST_UNK_OPCODE));
				return false;
		}

		return true;
	}
	catch(...){
		DebugLogWarning(GetResString(IDS_X_LANCAST_ERR_MULITC));
		return false;
	}
}

void CALLBACK CLanCast::UDPTimerProc(HWND /*hwnd*/, UINT /*uMsg*/,UINT_PTR /*idEvent*/,DWORD /*dwTime*/)
{
	if(!theApp.lancast->IsConnected())
		return;

	int count = theApp.sharedfiles->GetCount();
	static int current = 0;

	if (!count)
		return;

	for (int i = 0; i < 32; i++) {
		current = (current+1) % count;

		CKnownFile *file = theApp.sharedfiles->GetFileByIndex(i);

		if (file && file->KnownPrefs->IsEnableLanCast())
			theApp.lancast->BroadcastHash(file);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// CLanCast Private member functions - MultiCast Network Code

BOOL CLanCast::CreateReceivingSocket(LPCTSTR strGroupIP, UINT nGroupPort)
{
	// Create socket for receiving packets from multicast group
	if(!Create(nGroupPort, SOCK_DGRAM, FD_READ))
		return FALSE;

	BOOL bMultipleApps = TRUE;		// allow reuse of local port if needed
	SetSockOpt(SO_REUSEADDR, (void*)&bMultipleApps, sizeof(BOOL), SOL_SOCKET);

	// Fill m_saHostGroup_in for sending datagrams
	memset(&m_saHostGroup, 0, sizeof(m_saHostGroup));
	m_saHostGroup.sin_family = AF_INET;
	m_saHostGroup.sin_addr.s_addr = inet_addr(CStringA(strGroupIP));
	m_saHostGroup.sin_port = htons((USHORT)nGroupPort);

	// Join the multicast group on all interfaces
	ip_mreq mrMReq;
	mrMReq.imr_multiaddr.s_addr = inet_addr(CStringA(strGroupIP));	// group addr
	if (!NeoPrefs.IsCustomLanCastAdapter()){
		for (POSITION pos = interface_list.GetHeadPosition();pos;) {
			POSITION old_pos = pos;
			mrMReq.imr_interface.s_addr = interface_list.GetNext(pos).address;	// current interface
			if (SetSockOpt(IP_ADD_MEMBERSHIP, (char FAR *)&mrMReq, sizeof(mrMReq), IPPROTO_IP) == 0)
				interface_list.RemoveAt(old_pos);
		}
	}else{
		mrMReq.imr_interface.s_addr = m_uAdapterIP;
		if (SetSockOpt(IP_ADD_MEMBERSHIP, (char FAR *)&mrMReq, sizeof(mrMReq), IPPROTO_IP) == 0)
			return FALSE;
	}

	return TRUE;
}

BOOL CLanCast::CreateSendingSocket(UINT nTTL, BOOL bLoopBack)
{
	if(!m_SendSocket.Create(0, SOCK_DGRAM, 0))		// Create an unconnected UDP socket
		return FALSE;

	if(m_SendSocket.SetSockOpt(IP_MULTICAST_TTL, &nTTL, sizeof(int), IPPROTO_IP) == 0)
		DebugLogWarning(GetResString(IDS_X_ERR_LANCAST_TTL));

	SetLoopBack(bLoopBack);

	return TRUE;
}

bool CLanCast::Start()
{
	if (!bStarted){
		// Try and join the multicast group - 224.0.0.1, using the prefs port and no loopback
		if(!JoinGroup(NeoPrefs.GetLanCastGroup(), NeoPrefs.GetLanCastPort(), 50, false)){
			DebugLogWarning(GetResString(IDS_X_LANCAST_JOIN_FAILED),NeoPrefs.GetLanCastGroup(), NeoPrefs.GetLanCastPort());
			return false;
		}else
			DebugLogWarning(GetResString(IDS_X_LANCAST_JOIN_SUCCEEDED),NeoPrefs.GetLanCastGroup(), NeoPrefs.GetLanCastPort());
	}
	return true;
}

void CLanCast::Stop()
{
	if (bStarted){
		if(!LeaveGroup()) 
			DebugLogWarning(GetResString(IDS_X_LANCAST_LEAVE_FAILED));
		else 
			DebugLogWarning(GetResString(IDS_X_LANCAST_LEAVE_SUCCEEDED));
	}
}

BOOL CLanCast::GetAllAdapters(CList<multicast_interface> *temp_interface_list)
{
	// enumerate all interfaces
	PMIB_IPADDRTABLE pIPAddrTable;
	DWORD dwSize = 0;

	// get address table size
	if (GetIpAddrTable(NULL, &dwSize, 0) != ERROR_INSUFFICIENT_BUFFER)
		return FALSE;

	// get address table
	pIPAddrTable = (PMIB_IPADDRTABLE)new char[dwSize];
	if (GetIpAddrTable(pIPAddrTable, &dwSize, 0) != NO_ERROR) {
		delete pIPAddrTable;
		return FALSE;
	}

	uint32 localhost = inet_addr("127.0.0.1");
	uint32 publicsubnet = inet_addr("255.255.255.255");
	uint32 emptysubnet = inet_addr("0.0.0.0");

	// add interfaces
	temp_interface_list->RemoveAll();
	for (DWORD i = 0; i < pIPAddrTable->dwNumEntries; i++) {
		multicast_interface mcif;
		if ((mcif.address = pIPAddrTable->table[i].dwAddr) == localhost)
			continue;
		if ((mcif.subnet = pIPAddrTable->table[i].dwMask) == publicsubnet)
			continue;
		if (mcif.subnet == emptysubnet)
			continue;
		temp_interface_list->AddTail(mcif);
	}

	delete pIPAddrTable;

	return temp_interface_list->GetCount() ? TRUE : FALSE;
}

BOOL CLanCast::SelectAdapters()
{
	if(!GetAllAdapters(&interface_list))
		return FALSE;

	if(NeoPrefs.IsCustomLanCastAdapter())
	{
		uint32 AdapterIP = NeoPrefs.GetLanCastAdapterIPAdress();
		uint32 SubNetMask = NeoPrefs.GetLanCastAdapterSubNet();

		for (POSITION pos = interface_list.GetHeadPosition();pos;) {
			multicast_interface Adapter = interface_list.GetNext(pos);
			uint32 CurrentIP = Adapter.address;
			uint32 CurrentMask = (SubNetMask ? SubNetMask : Adapter.subnet);
			if ((AdapterIP & CurrentMask) == (CurrentIP & CurrentMask)){
				m_uAdapterIP = CurrentIP;
				m_uSubNetMask = CurrentMask;
				break;
			}
		}

		if(!m_uSubNetMask || !m_uAdapterIP)
			return FALSE;
	}

	return TRUE;
}

BOOL CLanCast::JoinGroup(LPCTSTR GroupIP, UINT nGroupPort, UINT nTTL, BOOL bLoopback)
{
	bStarted = false;

	//if(!SelectAdapters())								// Select adapters
	//	return FALSE;

	if(!CreateReceivingSocket(GroupIP, nGroupPort))		// Create Socket for receiving and join the host group
		return FALSE;

	if(!CreateSendingSocket(nTTL, bLoopback))			// Create Socket for sending
		return FALSE;

	if(NeoPrefs.IsAutoBroadcastLanFiles()) // for compatibility to the old version
		udp_timer = ::SetTimer(NULL, NULL, NeoPrefs.GetAutoBroadcastLanFilesMs()/*LANCAST_TIMER*/, CLanCast::UDPTimerProc);

	bStarted = true;

	return TRUE;
}


BOOL CLanCast::LeaveGroup()
{
	bStarted = false;
	if(udp_timer){
		::KillTimer(NULL, udp_timer);
		udp_timer = NULL;
	}

	// Close receving socket
	ip_mreq mrMReq;
	mrMReq.imr_multiaddr = m_saHostGroup.sin_addr;	// group addr
	if (!NeoPrefs.IsCustomLanCastAdapter()){
		for (POSITION pos = interface_list.GetHeadPosition();pos;) {
			mrMReq.imr_interface.s_addr = interface_list.GetNext(pos).address;	// current interface
			SetSockOpt(IP_DROP_MEMBERSHIP, (char FAR *)&mrMReq, sizeof(mrMReq), IPPROTO_IP);
		}
	}else{
		mrMReq.imr_interface.s_addr = m_uAdapterIP;
		SetSockOpt(IP_DROP_MEMBERSHIP, (char FAR *)&mrMReq, sizeof(mrMReq), IPPROTO_IP);
	}

	Close();

	// Close sending socket
	m_SendSocket.Close();

	return TRUE;
}

void CLanCast::SetLoopBack(BOOL bLoop)
{
	// Set LOOPBACK option to TRUE OR FALSE according to IsLoop parameter
	int nLoopBack = (int)bLoop;

	// Try to manually set the loopback, ie tell dozer to ignlore packets from itself (if setloopback is false)
	m_SendSocket.SetSockOpt(IP_MULTICAST_LOOP, &nLoopBack, sizeof(int), IPPROTO_IP);
	SetSockOpt(IP_MULTICAST_LOOP, &nLoopBack, sizeof(int), IPPROTO_IP);
}

#endif //LANCAST // NEO: NLC END <-- Xanatos --
