//this file is part of NeoMule
//Copyright (C)2007 David Xanatos ( Xanatos@Lycos.at / http://neomule.sourceforge.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#pragma once

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->

class CNatSocket;
class CNatTunnel;
class CUpDownClient;

// NEO: NATT - [NatTraversal]
struct sNatAddr
{
	uint32 uIP;
	uint16 uPort;

	sNatAddr(uint32 IP = 0, uint16 Port = 0){
		uIP = IP;
		uPort = Port;
	}

	friend bool operator==(const sNatAddr& na1,const sNatAddr& na2){
		return (na1.uIP == na2.uIP && na1.uPort == na2.uPort);
	}
};

template<> inline UINT AFXAPI HashKey(const sNatAddr& na){
	uint32 hash = 0;
	hash = na.uIP;
	hash ^= (na.uPort << 16);
	return hash;
};

enum { no_id = 0, ed2k_id = 1, kad_id = 2, xs_id = 3 };

__inline uint32 GetIDSize(int IDKind)
{
	if(IDKind == ed2k_id)
		return 4; // 32 bit
	else if(IDKind == kad_id || IDKind == xs_id)
		return 16; // 128 bit
	return 0;
}
// NEO: NATT END

///////////////////////////////////////////////////////////////////////////////
// CNatManager

class CNatManager
{
public:
	CNatManager();
	virtual ~CNatManager();

	void	Process();

	void	DispatchUmTCPPacket(const BYTE* packet, UINT size, uint8 opcode, uint32 ip, uint16 port);
	// NEO: NATT - [NatTraversal]
	bool	PrepareConnect(CUpDownClient* client);
	CNatTunnel* PrepareTunnel(CUpDownClient* client);
	CNatTunnel* FindPendingTunnel(const uchar* pID, const int IDKind);
	CNatTunnel* FindPendingTunnel(uint32 ip, uint16 port);

	void	SendNatPing(uint32 ip, uint16 port, bool bReqAnswer = true, const uchar* pID = NULL, const int IDKind = no_id, bool bObfuscation = false);
	void	ProcessNatPingPacket(const BYTE* packet, UINT size, uint32 ip, uint16 port);
	CNatTunnel* HandleCallbackRequest(const uchar* pID, const int IDKind, uint32 uIP, uint16 uPort, const uint8 byCryptOptions = 0, const uchar* achUserHash = NULL);
	// NEO: NATT END

protected:
	friend class CNatSocket;

	void	AddSocket(CNatSocket* toadd);
	void	RemoveSocket(CNatSocket* torem);
	void	ClearTunnel(CNatTunnel* tunnel); // NEO: NATT - [NatTraversal]

	void	SendUmTCPPacket(CNatSocket* socket, char* packet, UINT size, uint8 opcode);

private:
	CNatSocket* FindSocket(uint32 natIP, uint16 natPort);
	CNatTunnel* FindPendingTunnel(CNatSocket* socket); // NEO: NATT - [NatTraversal]

	void	SendReset(uint32 ip, uint16 port, int nErrorCode = 0);

	CTypedPtrList<CPtrList, CNatSocket*> m_NatSockets;
	// NEO: NATT - [NatTraversal]
	CMap<sNatAddr,const sNatAddr&,CNatTunnel*,CNatTunnel*> m_NatTunnels;
	CTypedPtrList<CPtrList, CNatTunnel*> m_NatPending;
	// NEO: NATT END
};

#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --