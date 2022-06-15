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

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->

#pragma once

class CKnownFile;
class CPartFile;
class Packet;
struct SSearchRoot;

class CLanCast : public CAsyncSocket
{
public:
	CLanCast();
	~CLanCast();

	void GetSources(CPartFile* pFile);
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	void SendVoodoo(uint8 uAction, uint8 uReply = FALSE);
#endif // VOODOO // NEO: VOODOO END

	bool SendPacket(Packet* packet, uint32 addr, bool bSrc = false);
	void BroadcastPacket(Packet* packet, bool bSrc = false);
	virtual void OnReceive(int nErrorCode);

	bool Start();
	void Stop();
	bool IsConnected() { return bStarted; }

	bool IsLanIP(uint32 dwUserIP);

	BOOL SelectAdapters();

private:
	bool CheckSender(uint32 sender);

	void BroadcastHash(CKnownFile* cur_file);
	void ReceiveHash(uint8* pachPacket, uint32 nSize);

	void ReceiveSourceRequest(uint8* pachPacket, uint32 nSize);
	void SendSourceAnswer(CKnownFile* kFile);
	void ReceiveSourceAnswer(uint8* pachPacket, uint32 nSize);

	void ReceiveFileSearch(uint8* pachPacket, uint32 nSize, uint32 sender);
	CPtrList* PerformFileSearch(const SSearchRoot* pSearch);
	void SendSearchResponce(CPtrList* list, uint32 sender);
	void ReceiveSearchResponce(uint8* pachPacket, uint32 nSize);

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	void ReceiveVoodoo(uint8* pachPacket, uint32 nSize);
#endif // VOODOO // NEO: VOODOO END

	struct multicast_interface {
		uint32 address;
		uint32 subnet;
	};
	BOOL GetAllAdapters(CList<multicast_interface> *temp_interface_list);
	BOOL JoinGroup(LPCTSTR, UINT, UINT, BOOL);
	BOOL LeaveGroup();

	void SetLoopBack(BOOL);
	BOOL CreateSendingSocket(UINT, BOOL);
	BOOL CreateReceivingSocket(LPCTSTR, UINT);

	void static CALLBACK UDPTimerProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime);
	bool ProcessPacket(uint8* packet, uint32 size, uint8 opcode, uint32 sender);

	CList<multicast_interface> interface_list;
	uint32	m_uAdapterIP;
	uint32	m_uSubNetMask;
	addrinfo *m_aiList;
	SOCKADDR_IN m_saHostGroup;	// SOCKADDR structure to hold IP/Port of the Host group to send data to it

	CAsyncSocket m_SendSocket;	// Socket for sending data to the host group

	bool bStarted;
	uint32 udp_timer;			// Timer to fire packet sends

};

#endif //LANCAST // NEO: NLC END <-- Xanatos --
