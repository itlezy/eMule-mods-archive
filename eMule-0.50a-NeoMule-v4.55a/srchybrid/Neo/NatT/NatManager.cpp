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

#include "stdafx.h"
#include "emule.h"
#include "NatManager.h"
#include "NatSocket.h"
#include "Neo/NeoOpcodes.h"
#include "ListenSocket.h"
#include "ClientUDPSocket.h"
#include "Packets.h"
#include "UpDownClient.h"
#include "Statistics.h"
#include "Preferences.h"
#include "Neo/NeoPreferences.h"
#include "Log.h"
#include "NatTunnel.h" // NEO: NATT - [NatTraversal]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CNatManager

/**
* creates the nat manager.
*/
CNatManager::CNatManager()
{

}

/**
* destroys the nat manager.
*/
CNatManager::~CNatManager()
{
	// Nat sockets should be cleared when destroying all AsyncSockets
	ASSERT(m_NatSockets.IsEmpty());

	// pending Nat tunels shell be deleted when the owning sockets are deleted
	ASSERT(m_NatPending.IsEmpty());

	// clear open nat tunnels
	sNatAddr addr;
	CNatTunnel* cur_tunnel;
	POSITION pos = m_NatTunnels.GetStartPosition();
	while (pos){
		m_NatTunnels.GetNextAssoc(pos, addr, cur_tunnel);
		delete cur_tunnel;
	}
	m_NatTunnels.RemoveAll();
}

/**
* Process is called form the main thread 10 times a secund.
*/
void CNatManager::Process()
{
	// all current nat sockets
	POSITION pos;
	for (pos = m_NatSockets.GetHeadPosition(); pos; )
	{
		CNatSocket* cur_socket = m_NatSockets.GetNext(pos);

		cur_socket->CheckForTimeOut();
	}

	// NEO: NATT - [NatTraversal]
	uint32 cur_tick = ::GetTickCount();

	// app pending tunels
	for (pos = m_NatPending.GetHeadPosition(); pos != NULL;)
	{
		CNatTunnel* cur_tunnel = m_NatPending.GetNext(pos);
		// To save overhead and handle better full cone nat's we first try a ping and only if it fails we use the callback
		// this also helps by other nat types if the port didn't changed and also to handle hardware firewalls, by saving one callback
		if(cur_tunnel->GetCallbackPending() && cur_tick - cur_tunnel->GetCallbackPending() > SEC2MS(3))
		{
			cur_tunnel->SendCallback();
		}
	}

	// all open nat tunnels
	sNatAddr addr;
	CNatTunnel* cur_tunnel;
	pos = m_NatTunnels.GetStartPosition();
	while (pos)
	{
		m_NatTunnels.GetNextAssoc(pos, addr, cur_tunnel);

		if(cur_tick - cur_tunnel->GetLastRecivedPing() > SEC2MS(10) * (uint32)(cur_tunnel->GetPendingPingCount() + 1))
		{
			cur_tunnel->IncrPendingPingCount();

			// We need this ping mechanism to keep the Nat Tunnel alive even when no UmTCP traffic is in progress
			if(cur_tunnel->GetState() == active_nt) // only active tunels ping
			{
				SendNatPing(cur_tunnel->GetTargetIP(),cur_tunnel->GetTargetPort());
			}
			// this is used to remove broken tunels, and also to removed unneded tunels in passive_nt state
			else if(cur_tunnel->GetPendingPingCount() > 1)
			{
				m_NatTunnels.RemoveKey(addr);
				delete cur_tunnel;
			}
		}
		// To save overhead and handle better full cone nat's we first try a ping and only if it fails we use the callback
		else if(cur_tunnel->GetCallbackPending() && cur_tick - cur_tunnel->GetCallbackPending() > SEC2MS(3))
		{
			SendNatPing(cur_tunnel->GetTargetIP(),cur_tunnel->GetTargetPort());
			cur_tunnel->SendCallback();
		}
	}
	// NEO: NATT END
}

/**
* AddSocket is called from the CNatSocket constructor,
*   it add the new socket to the managers list.
*
* @param toadd: nat socket to add
*/
void CNatManager::AddSocket(CNatSocket* toadd)
{
	m_NatSockets.AddTail(toadd);
}

/**
* RemoveSocket is called from the ~CNatSocket destructos as also when the socket is closed or reseted, 
*   it removes the socket from the managers list.
*
* @param torem: nat socket to remove
*/
void CNatManager::RemoveSocket(CNatSocket* torem)
{
	if(POSITION pos = m_NatSockets.Find(torem))
		m_NatSockets.RemoveAt(pos);

	// NEO: NATT - [NatTraversal]
	if(CNatTunnel* tunnel = torem->GetTunnel()) // if we have a tunel
		ClearTunnel(tunnel); // clear it, the socket is sheduled for distruction
	// NEO: NATT END
}

/**
* FindSocket returns a new or in use socket stored in the nat managers list, 
*   if the cosket was already closed it is not longer listed.
*
* @param natIP: IP of packet source
*
* @param natPort: UDP port of packet source
*/
CNatSocket* CNatManager::FindSocket(uint32 natIP, uint16 natPort)
{
	ASSERT(natIP && natPort);

	for (POSITION pos = m_NatSockets.GetHeadPosition(); pos != NULL;)
	{
		CNatSocket* cur_socket = m_NatSockets.GetNext(pos);
		if (cur_socket->GetTargetIP() == natIP && cur_socket->GetTargetPort() == natPort)
			return cur_socket;
	}
	return NULL;
}

// NEO: NATT - [NatTraversal]
/**
* PrepareConnect prepares a pending tunel for a UmTCP connection
*
* @param client: client to connect
*/
bool CNatManager::PrepareConnect(CUpDownClient* client)
{
	uint32 uIP = client->GetConnectIP();
	uint16 uPort = client->GetUDPPort();

	if(client->GetUserPort() == 0 && uIP != 0 && uPort != 0) // manual debug clients
	{
		// this is actualy a high ID without a TCP port but with a UDP one instead
		client->socket->GetNatLayer()->SetIPPort(uIP,uPort);
		return true; 
	}

	CNatTunnel* tunnel;
	// check if we already have an working tunnel
	if(m_NatTunnels.Lookup(sNatAddr(uIP,uPort),tunnel))
	{
		if(tunnel->GetState() == no_nt // is the tunel yet operativ at all
		 || tunnel->GetPendingPingCount() > 1){ // tunnel was active but a one or more pings failed
			m_NatTunnels.RemoveKey(sNatAddr(uIP,uPort)); // remove the tunel we will build a new one
			delete tunnel;
		}
		else // we have a active tunel
		{
			client->socket->GetNatLayer()->SetIPPort(uIP,uPort);
			tunnel->SetSocket(client->socket->GetNatLayer());
			return true; // we can connect right away
		}
	}
	return false;
}

/**
* PrepareTunnel cretes a new pending nat tunel, awaiting incomming nat ping.
* 
* @param client: client to callback
*/
CNatTunnel* CNatManager::PrepareTunnel(CUpDownClient* client)
{
	if(!client->socket || !client->socket->GetNatLayer())
		return NULL; // we got here most propobly form a kad buddy found answer but the socket is already gone.

	CNatTunnel* tunnel = FindPendingTunnel(client->socket->GetNatLayer());
	if(tunnel == NULL)
	{
		tunnel = new CNatTunnel();
		tunnel->SetSocket(client->socket->GetNatLayer());
		tunnel->SetState(connecting_nt);

		tunnel->SetIPPort(client->GetConnectIP(),client->GetUDPPort());

		//Obfuscation support
		if(thePrefs.IsClientCryptLayerSupported()){
			tunnel->SetUserHash(client->GetUserHash());
			tunnel->SetCryptLayerSupport(client->SupportsCryptLayer());
			tunnel->SetCryptLayerRequest(client->RequestsCryptLayer());
			tunnel->SetCryptLayerRequires(client->RequiresCryptLayer());
		}
		m_NatPending.AddTail(tunnel);
		return tunnel;
	}
	else
	{
		ASSERT(0); // this should not happen
		return NULL;
	}
}

/**
* ClearTunnel removes the not longet needed nat tunel.
*
* @param tunel: tunel to be cleared
*/
void CNatManager::ClearTunnel(CNatTunnel* tunnel)
{
	tunnel->SetSocket(NULL); // sets tunel state to passive_nt == we dont sent pings on our own

	// Note: we dont remove at this point established tunels, 
	//	cause if the remote side don't remove it's tunel at the same moment, 
	//	it may happen that we will get a ping and create a new tunel again.
	//	We and the remote side are just setting the tunel to passive_nt and wait for it to timeout.

	if(POSITION pos = m_NatPending.Find(tunnel)) // we always delete pending tunels when the socket is closed
	{
		m_NatPending.RemoveAt(pos);
		delete tunnel;
	}
}

/**
* FindPendingTunnel searches the list for a tunel by a given socket.
*
* @param socket: tunel owver socket
*/
CNatTunnel* CNatManager::FindPendingTunnel(CNatSocket* socket)
{
	for (POSITION pos = m_NatPending.GetHeadPosition(); pos != NULL;)
	{
		CNatTunnel* cur_tunnel = m_NatPending.GetNext(pos);
		if(cur_tunnel->GetSocket() == socket)
			return cur_tunnel;
	}
	return NULL;
}

/**
* FindPendingTunnel looks in the pending tunnels list for a tunnel where the ID maches the ID of the client.
*
* @param pID: ID of the client
*
* @param IDKind: type if ID
*/
CNatTunnel* CNatManager::FindPendingTunnel(const uchar* pID, const int IDKind)
{
	for (POSITION pos = m_NatPending.GetHeadPosition(); pos != NULL;)
	{
		CNatTunnel* cur_tunnel = m_NatPending.GetNext(pos);
		if(cur_tunnel->CompareID(pID,IDKind) == 0)
			return cur_tunnel;
	}
	return NULL;
}

/**
* FindPendingTunnel looks in the pending tunnels list for a tunnel where the IP and port match the source.
*
* @param ip: ip of the incomming ping
*
* @param port: source port
*/
CNatTunnel* CNatManager::FindPendingTunnel(uint32 ip, uint16 port)
{
	for (POSITION pos = m_NatPending.GetHeadPosition(); pos != NULL;)
	{
		CNatTunnel* cur_tunnel = m_NatPending.GetNext(pos);
		if(cur_tunnel->GetTargetIP() == ip && cur_tunnel->GetTargetPort() == port)
			return cur_tunnel;
	}
	return NULL;
}
// NEO: NATT END

/**
* DispatchUmTCPPacket redirects the incomming User Mode TCP packets to the associated sockets, 
*   it also handles Listen/Accept tasks.
*   It also is used to embed the UnserMode TCP in a container packet if we want.
*
* @param's: same as CClientUDPSocket::ProcessPacket
*/
void CNatManager::DispatchUmTCPPacket(const BYTE* packet, UINT size, uint8 opcode, uint32 ip, uint16 port)
{
	CNatSocket* socket = FindSocket(ip, port);

	// here we perform the Listen/Accept task
	if(opcode == OP_NAT_SYN)
	{
		if( socket != NULL )
		{
			NATTrace((uint32)socket,NLE_STATE,__FUNCTION__,"Nat SYN recived for an already existing socket");
			// Note: we arived here due to a bug or becouse the remote client tryed a reconnect to fast, 
			//			an connection attempt conflist is unlikly but also posssible.
			//			On the remote side the socket should be already removed, so we cant sent a RST.
			//			We silently ignore the problem and just remove the old socket
			socket->ShutDown(SD_BOTH);
			RemoveSocket(socket);
			socket = NULL;
		}

		CClientReqSocket* newclient = new CClientReqSocket;
		socket = newclient->InitNatSupport();
		newclient->AsyncSelect(FD_WRITE | FD_READ | FD_CLOSE);
		socket->SetIPPort(ip,port); // adds the socket on our nat socket list

		// NEO: NATT - [NatTraversal]
		// if this is a socket without a known tunel, look if there is a matching tunel and link them
		if(socket->GetTunnel() == NULL)
		{
			CNatTunnel* tunnel = NULL;
			if(m_NatTunnels.Lookup(sNatAddr(ip,port),tunnel))
				tunnel->SetSocket(socket);
		}
		// NEO: NATT END
	}

	// check for a valid socket
	if( socket == NULL || (socket->IsInit() == false && opcode != OP_NAT_SYN && opcode != OP_NAT_SYN_ACK))
	{
		if( socket )
		{
			NATTrace((uint32)socket,NLE_STATE,__FUNCTION__,"Nat 0x%08x packet for an unkonfigured socket recived, socket removed",opcode);
			RemoveSocket(socket);
		}
		else
			NATTrace(0,NLE_STATE,__FUNCTION__,"Nat 0x%08x packet for an unexisting socket recived",opcode);

		if(opcode != OP_NAT_RST)
			SendReset(ip, port, WSAENOTSOCK); // send reset, to force the sender to close the connection
		return;
	}

	// dispatch the packet to the apropriated socket
	socket->ProcessPacket(packet, size, opcode);
}

/**
* SendUmTCPPacket handless all sending, 
*   It also is used to embed the UnserMode TCP in a container packet if we want.
*
* @param socket: socket who wants to send the data
*
* @param packet: pointer to the packee data
*
* @param size: size of the packet
*
* @param opcode: opcode to send
*/
void CNatManager::SendUmTCPPacket(CNatSocket* socket, char* packet, UINT size, uint8 opcode)
{
	ASSERT(m_NatSockets.Find(socket));
	ASSERT(socket->GetOwner()->IsKindOf(RUNTIME_CLASS(CClientReqSocket)));

	CNatTunnel* tunnel = socket->GetTunnel(); //Obfuscation support

	Packet* Data = new Packet(OP_NAT_CARIER);
	Data->opcode = opcode;
	Data->size = size;
	Data->pBuffer = packet;

	theApp.clientudp->SendPacket(Data, socket->GetTargetIP(), socket->GetTargetPort(), 
		tunnel ? tunnel->ShouldReceiveCryptUDPPackets() : false, tunnel ? tunnel->GetUserHash() : NULL, false, 0);
}

/**
* SendReset send reset packet to an unknown or uninitialised client
*
* @param ip: ip of the remote client
*
* @param port: port of the remote client
*
* @param nErrorCode: error code to send (obtional)
*/
void CNatManager::SendReset(uint32 ip, uint16 port, int nErrorCode)
{
	Packet* packet = new Packet(OP_NAT_RST,nErrorCode ? 4 : 0,OP_NAT_CARIER,0);
	if(nErrorCode)
		PokeUInt32(packet->pBuffer, nErrorCode);
	theApp.clientudp->SendPacket(packet,ip,port,false,NULL,false,0); 
}

// NEO: NATT - [NatTraversal]

/**
* SendNatPing send a nat ping packet, witch contains a reping flag and an obtional ID as well as the IdType, 
*   additionaly obfuscation informations can be attached.
*
* @param ip: ip of the remote client
*
* @param port: port of the remote client
*
* @param bReqAnswer: specifyes does we request an remote ping answer
*
* @param pID: pointer to the ID
*
* @param IDKind: kind of ID that implicitly tels us the id size
*/
void CNatManager::SendNatPing(uint32 ip, uint16 port, bool bReqAnswer, const uchar* pID, const int IDKind, bool bObfuscation)
{
	uint32 IDSize = GetIDSize(IDKind);

	const uint32 obfuSize = (bObfuscation && thePrefs.IsClientCryptLayerSupported()) ? 17 : 0;

	const uint32 baseSize = (bReqAnswer || pID || obfuSize) ? 1 : 0; // the ping may be completly empty

	Packet* packet = new Packet(OP_NAT_PING,baseSize + IDSize + obfuSize,OP_NAT_CARIER,0);
	// Note: I thougt a long time about using here NanoTags or sommilar, but I desiced to use a more proffesional approache
	//			and use as less overhead as possible, we shell use packets with an ID only when we send a ping answer on a callback,
	//			what implicitly means that we know the ID type and that the remote side know it as well, there for we musn't send the ID length.
	//			An obfu hash and obtions is being attached always to the end of the packet and a look on the packet size tels us if there is one
	if(baseSize)
	{
		 const uint8 uPingOps = //(((uint8) & 0x0f) < 4)
								//(((uint8) & 0x01) < 1)
								(((uint8)IDKind		& 0x03) << 1) |
								(((uint8)bReqAnswer	& 0x01) << 0);
		 packet->pBuffer[0] = uPingOps;
	}

	if(IDSize)
		memcpy(packet->pBuffer+baseSize,pID,IDSize);

	//Obfuscation support
	// Note: in the usual case with a full symetric callback booth clients get this data form svr/kad/xs,
	//		but when we connect without one or booth callbacks we need this data in the first ping.
	if(obfuSize)
	{
		const UINT uSupportsCryptLayer	= thePrefs.IsClientCryptLayerSupported() ? 1 : 0;
		const UINT uRequestsCryptLayer	= thePrefs.IsClientCryptLayerRequested() ? 1 : 0;
		const UINT uRequiresCryptLayer	= thePrefs.IsClientCryptLayerRequired() ? 1 : 0;
		const UINT uObfuscation =
		(uRequiresCryptLayer	<<  2) |
		(uRequestsCryptLayer	<<  1) |
		(uSupportsCryptLayer	<<  0);
		packet->pBuffer[baseSize+IDSize] = (uint8)uObfuscation;
		md4cpy(packet->pBuffer+baseSize+IDSize+1,thePrefs.GetUserHash()); // the ping packet may not contain any ID but stil contain obfu settings so we send the hash anyway
	}

	CNatTunnel* tunnel = NULL; //Obfuscation support
	m_NatTunnels.Lookup(sNatAddr(ip,port),tunnel);
	theApp.clientudp->SendPacket(packet, ip, port, tunnel ? tunnel->ShouldReceiveCryptUDPPackets() : false, tunnel ? tunnel->GetUserHash() : NULL, false, 0);
}

/**
* ProcessNatPingPacket is responsible for establishing tunels and starting streaming connections.
*
* @param's: same as CClientUDPSocket::ProcessPacket
*/
void CNatManager::ProcessNatPingPacket(const BYTE* packet, UINT size, uint32 ip, uint16 port)
{
	int IDKind = no_id;
	int ReqAnswer = FALSE;
	if(size >= 1){
		const uint8 uPingOps = packet[0];
		IDKind		= ((uPingOps >> 1) & 0x03);
		ReqAnswer	= ((uPingOps >> 0) & 0x01);
	}

	uint32 IDSize = GetIDSize(IDKind);

	uchar* pID = NULL;
	if(IDSize){
		pID = new uchar[IDSize];
		memcpy(pID,packet+1,IDSize);
	}

	//Obfuscation support
	uint8 byCryptOptions = 0;
	uchar achUserHash[16];
	if(size >= 1+IDSize+17){
		byCryptOptions = packet[1+IDSize];
		md4cpy(achUserHash, packet+1+IDSize+1);
	}

	// find pending tunnel
	CNatTunnel* tunnel = NULL;
	if(IDKind != no_id)
	{
		tunnel = FindPendingTunnel(pID,IDKind);
		if(tunnel == NULL) // it tooke mor tine for the callback than for the scoket to timeout ???
			DebugLog(_T("*** Recived nat ping with ID but no pending tunel was found!"),ipstr(ip,port));
	}
	tunnel = FindPendingTunnel(ip,port);

	if(tunnel == NULL) // no panding tunel found
	{
		// find established tunnel
		if(!m_NatTunnels.Lookup(sNatAddr(ip,port),tunnel))
		{
			// add new established tunnel
			tunnel = new CNatTunnel(ip,port);
			m_NatTunnels.SetAt(sNatAddr(ip,port),tunnel);

			// look if there is a socket on this tunel, should not happen but in case
			if(CNatSocket* socket = FindSocket(ip,port))
				tunnel->SetSocket(socket);
		}
		if(tunnel->GetState() == no_nt) // Note: the tunel may be no_nt if it was created by the callback request handle, or avtive_nt if it is in use
			tunnel->SetState(passive_nt); // but if its no_nt we set to passive_nt to mark that the tinel is operativ now
	}
	else
	{
		// add pening tunnel was established
		tunnel->SetIPPort(ip,port);
		m_NatTunnels.SetAt(sNatAddr(ip,port),tunnel);
		// previuse state shall alwas be connecting_nt
		ASSERT(tunnel->GetState() == connecting_nt);
		
	}

	tunnel->SetPingRecived();

	//Obfuscation support
	if(byCryptOptions){
		tunnel->SetUserHash(achUserHash);
		tunnel->SetCryptLayerSupport((byCryptOptions & 0x01) != 0);
		tunnel->SetCryptLayerRequest((byCryptOptions & 0x02) != 0);
		tunnel->SetCryptLayerRequires((byCryptOptions & 0x04) != 0);
	}

	if(ReqAnswer) // do we have to answer on this ping
		SendNatPing(ip,port,false);

	// check if we are connecting (the requester)
	if(tunnel->GetState() == connecting_nt){
		// since we already have a socket set we hav to switch now to active_nt manualy
		tunnel->SetState(active_nt);

		// remove the pending tunel;
		if(POSITION pos = m_NatPending.Find(tunnel))
			m_NatPending.RemoveAt(pos);

		CNatSocket* socket = tunnel->GetSocket();
		ASSERT(socket); // we had state connecting_nt so we also have a set socket, if the socket have been deleted the state would be passiv_nt

		// we must first set a the valid up to date ip and port, befoure we can connect
		socket->SetIPPort(ip,port);

		CAsyncSocketEx* owner = socket->GetOwner();
		ASSERT(owner); // an nat socket should always have an owner

		CUpDownClient* client = ((CClientReqSocket*)owner)->client;
		if(client) // it is not clear if a CClientReqSocket still have a client or not, safe_delete may be in progres and the client set to NULL
			client->Connect(); // finaly connect the UmTCP connection
	}
}

/**
* HandleCallbackRequest handles an incomming callback request and tels if a remote callback is needed
*
* @param pID: pointer to the clients ID
*
* @param IDKind: clients id type
*
* @param uIP: ip of the requesting client as seen by the forwarding instance
*
* @param uPort: udp port of the requesting client as seen by the forwarding instance
*
* @param byCryptOptions: obfiscation obtions to sonfigure tunel
*
* @param achUserHash: userhash for ubfuscation
*/
CNatTunnel* CNatManager::HandleCallbackRequest(const uchar* pID, const int IDKind, uint32 uIP, uint16 uPort, const uint8 byCryptOptions, const uchar* achUserHash)
{
	// find the pending tunel
	CNatTunnel* tunnel = FindPendingTunnel(pID, IDKind);

	if(tunnel == NULL) // look for active tunels
		m_NatTunnels.Lookup(sNatAddr(uIP,uPort),tunnel); // it may be that we have a tunel if the requester uses reduces callback

	if(tunnel == NULL){ // create a tunel, we are the target
		tunnel = new CNatTunnel(uIP,uPort);
		tunnel->SetPingRecived(); // prevent frombeing removed to soon
		m_NatTunnels.SetAt(sNatAddr(uIP,uPort),tunnel);
		// the tunel state is at the moment no_nt
	}

	//Obfuscation support
	if(byCryptOptions){
		tunnel->SetUserHash(achUserHash);
		tunnel->SetCryptLayerSupport((byCryptOptions & 0x01) != 0);
		tunnel->SetCryptLayerRequest((byCryptOptions & 0x02) != 0);
		tunnel->SetCryptLayerRequires((byCryptOptions & 0x04) != 0);
	}

	// Now we check if we shell request a callback to, if we didn't asked for callback than we have to ask
	if(tunnel->IsCallbackDone())
	{
		tunnel->SetIPPort(uIP,uPort);
		// Note: we are the requester and if we got this calback than the target have already our ip/port so we dont sent our ID
		SendNatPing(uIP,uPort); // we send the ping to get a remote ping as confirmation that the tunel is operativ
		return NULL;
	}

	// we are the target
	return tunnel; // request remote callback, and set a ping with ID
}
// NEO: NATT END

#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --