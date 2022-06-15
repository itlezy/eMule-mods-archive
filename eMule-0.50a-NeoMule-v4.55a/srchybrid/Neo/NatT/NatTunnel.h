//this file is part of NeoMule
//Copyright (C)2007 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->

class CNatSocket;

enum { no_nt = 0, passive_nt = 1, active_nt = 2, connecting_nt = 3 };

///////////////////////////////////////////////////////////////////////////////
// CNatTunnel

class CNatTunnel
{
public:
	CNatTunnel(uint32 ip = 0, uint16 port = 0);
	virtual ~CNatTunnel();

	void	PrepareCallback(const uchar* pID, const int IDKind = no_id, uint32 uRelayIP = 0, uint16 uRelayPort = 0, bool bSendObfu = false, bool pPrepare = true);
	void	SendCallback();
	bool	IsCallbackDone() const				{ return m_State != no_nt; }

protected:
	friend class CNatManager; 
	friend class CNatSocket;

	uint32	GetTargetIP() const					{ return m_uIP; }
	uint16	GetTargetPort() const				{ return m_uPort; }
	void	SetIPPort(uint32 IP, uint16 Port) {
				m_uIP = IP; 
				m_uPort = Port;
			}

	int		CompareID(const uchar* pID, const int IDKind);

	void	SetPingRecived(){
				m_uLastRecivedPing = ::GetTickCount();
				m_uPendingPingCount = 0;
				m_uCallbackPending = 0; // unset all eventualy pending pings
			}
	uint32	GetLastRecivedPing() const			{ return m_uLastRecivedPing; }
	void	IncrPendingPingCount()				{ m_uPendingPingCount++; }
	uint16	GetPendingPingCount() const			{ return m_uPendingPingCount; }

	uint32	GetCallbackPending() const			{ return m_uCallbackPending; }

	void	SetState(int State) 				{ m_State = State; }
	int		GetState() const					{ return m_State; }

	void	SetSocket(CNatSocket* socket);
	CNatSocket*	GetSocket()						{ return m_Socket; }


	//Obfuscation support
	void			SetUserHash(const uchar* pUserHash);
	const uchar*	GetUserHash() const								{ return m_pUserHash; }
	bool			SupportsCryptLayer() const						{ return m_fSupportsCryptLayer; }
	bool			RequestsCryptLayer() const						{ return SupportsCryptLayer() && m_fRequestsCryptLayer; }
	bool			RequiresCryptLayer() const						{ return RequestsCryptLayer() && m_fRequiresCryptLayer; }
	void			SetCryptLayerSupport(bool bVal)					{ m_fSupportsCryptLayer = bVal ? 1 : 0; }
	void			SetCryptLayerRequest(bool bVal)					{ m_fRequestsCryptLayer = bVal ? 1 : 0; }
	void			SetCryptLayerRequires(bool bVal)				{ m_fRequiresCryptLayer = bVal ? 1 : 0; }
	bool			ShouldReceiveCryptUDPPackets() const;

private:
	uint32	m_uIP;
	uint16	m_uPort;

	int		m_IDKind;
	uchar*	m_pID;
	uint32	m_uRelayIP;
	uint16	m_uRelayPort;
	uint32	m_uCallbackPending;
	bool	m_bSendObfu;

	int		m_State;
	
	uint32	m_uLastRecivedPing;
	uint16	m_uPendingPingCount;

	CNatSocket* m_Socket;
	
	//Obfuscation support
	uchar* m_pUserHash;
	uint8 
		 m_fRequestsCryptLayer: 1,
	     m_fSupportsCryptLayer: 1,
		 m_fRequiresCryptLayer: 1;
};

#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --