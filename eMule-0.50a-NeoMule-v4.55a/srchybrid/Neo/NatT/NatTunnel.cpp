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
#include "NatTunnel.h"
#include "NatSocket.h"
#include "Neo/NeoOpcodes.h"
#include "ListenSocket.h"
#include "ClientUDPSocket.h"
#include "Packets.h"
#include "UpDownClient.h"
#include "Statistics.h"
#include "Preferences.h"
#include "Neo/NeoPreferences.h"
#include "ClientList.h"
#include "Server.h"
#include "Sockets.h"
#include "ServerList.h"
#include "Log.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Kademlia/Utils/UInt128.h"
#include "Kademlia/Kademlia/Prefs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NATTUNNELING // NEO: NATT - [NatTraversal] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CNatTunnel

/**
* creates the nat tunnel.
*/
CNatTunnel::CNatTunnel(uint32 ip, uint16 port)
{
	m_uIP = ip;
	m_uPort = port;

	m_IDKind = no_id;
	m_pID = NULL;

	m_State = no_nt;
	
	m_uLastRecivedPing = 0;
	m_uPendingPingCount = 0;

	m_uRelayIP = 0;
	m_uRelayPort = 0;
	m_uCallbackPending = 0;
	m_bSendObfu = false;

	m_Socket = NULL;
	
	//Obfuscation support
	m_pUserHash = NULL;
	m_fRequestsCryptLayer = 0;
	m_fSupportsCryptLayer = 0;
	m_fRequiresCryptLayer = 0;
}

/**
* destroys the nat tunnel.
*/
CNatTunnel::~CNatTunnel()
{
	SetSocket(NULL);

	delete [] m_pID;

	//Obfiscation support
	delete [] m_pUserHash;
}

/**
* CompareID checks if a given ID match the ID of the currently performed callback operation.
* 
* @param pID: ID of the cleint
*
* @param IDKind: type if ID
*/
int CNatTunnel::CompareID(const uchar* pID, const int IDKind)
{
	uint32 IDSize = GetIDSize(m_IDKind);

	if(m_IDKind != IDKind || IDSize == 0)
		return 1;
	return memcmp(m_pID,pID,IDSize);
}

/**
* PrepareCallback marks the tunel as pending and sets the curently used ID.
* 
* @param pID: ID of the cleint
*
* @param IDKind: type if ID
*
* @param uRelayIP: ip of the relaying instance
*
* @param uRelayPort: port of the relaying instance
*
* @param bSendObfu: should obfuscation infos be sent
*
* @param pPrepare: schedule a reask
*/
void CNatTunnel::PrepareCallback(const uchar* pID, const int IDKind, uint32 uRelayIP, uint16 uRelayPort, bool bSendObfu, bool pPrepare)
{
	uint32 IDSize = GetIDSize(IDKind);

	m_IDKind = IDKind;
	if(IDSize) // set
	{
		ASSERT(m_pID == NULL);
		m_pID = new uchar[IDSize];
		memcpy(m_pID,pID,IDSize);

		m_uRelayIP = uRelayIP;
		m_uRelayPort = uRelayPort;
		m_bSendObfu = bSendObfu;
		if(pPrepare)
			m_uCallbackPending = ::GetTickCount() + SEC2MS(3);
	}
	else // unset
	{
		delete [] m_pID;
		m_pID = NULL;
		m_uRelayIP = 0;
		m_uRelayPort = 0;
		m_uCallbackPending = 0;
	}
}

/**
* SendCallback sends a pending delayed remote callback using a preset relay node/server.
*/
void CNatTunnel::SendCallback()
{
	m_uCallbackPending = 0;
	switch(m_IDKind){
		case ed2k_id:
		{
			if(CServer* cur_server = theApp.serverconnect->GetCurrentServer())
			{
				if(CServer* server = theApp.serverlist->GetServerByIPTCP(cur_server->GetIP(),cur_server->GetPort()))
				{
					Packet* packet = new Packet(OP_NAT_CALLBACKREQUEST, 4+4);
					PokeUInt32(packet->pBuffer,*(uint32*)m_pID); // target ID
					PokeUInt32(packet->pBuffer+4,theApp.serverconnect->GetClientID()); // requester ID
					theApp.serverconnect->SendUDPPacket(packet,server,true);
				}
			}
			break;
		}
		case kad_id:
		{
			CSafeMemFile data(128);
			data.WriteHash16(m_pID);

			// Since all datas are pased unparsed over the buddy to our target, we can send whatever we need
			uchar hash[16]; md4clr(&hash[0]);
			data.WriteHash16(&hash[0]); // a invalid NULL hash indicates that this is a Mod Prot Packet, not a file reask

			// write mod opcode
			data.WriteUInt8(OP_NAT_CALLBACKREQUEST_KAD);

			// write our buddyID
			Kademlia::CUInt128 uBuddyID(true);
			uBuddyID.Xor(Kademlia::CKademlia::GetPrefs()->GetKadID());
			byte cID[16];
			uBuddyID.ToByteArray(cID);
			data.WriteHash16(cID); // my kad id

			// write the ip/udpport of our buddy
			if(theApp.clientlist->GetBuddy()) // should not happen but in case
			{
				data.WriteUInt32(theApp.clientlist->GetBuddy()->GetIP());
				data.WriteUInt16(theApp.clientlist->GetBuddy()->GetKadPort());
			}
			else
			{
				data.WriteUInt32(0);
				data.WriteUInt16(0);
			}

			//Obfuscation support
			// Note: On a first callback we must tell the cleint our crypto prefs,
			//		on a remote callback we don't send this data as the requester already have them
			if(m_bSendObfu)
			{
				const UINT uSupportsCryptLayer	= thePrefs.IsClientCryptLayerSupported() ? 1 : 0;
				const UINT uRequestsCryptLayer	= thePrefs.IsClientCryptLayerRequested() ? 1 : 0;
				const UINT uRequiresCryptLayer	= thePrefs.IsClientCryptLayerRequired() ? 1 : 0;
				const UINT uObfuscation =
				(uRequiresCryptLayer	<<  2) |
				(uRequestsCryptLayer	<<  1) |
				(uSupportsCryptLayer	<<  0);
				data.WriteUInt8((uint8)uObfuscation);
				data.WriteHash16(thePrefs.GetUserHash());
			}

			// send packet over buddy
			Packet* packet = new Packet(&data, OP_EMULEPROT);
			packet->opcode = OP_REASKCALLBACKUDP;
			theStats.AddUpDataOverheadFileRequest(packet->size);
			theApp.clientudp->SendPacket(packet,  m_uRelayIP, m_uRelayPort, false, NULL, true, 0);
			break;
		}
		case xs_id:
		{
			CSafeMemFile data(128);
			data.WriteHash16(m_pID);

			// write multi callback opcode
			data.WriteUInt8(OP_NAT_CALLBACKREQUEST_XS);

			// write our hash
			data.WriteHash16(thePrefs.GetUserHash());

			// write the ip/udpport of our xs buddy
			if(theApp.clientlist->GetXsBuddy()) // should not happen but in case
			{
				data.WriteUInt32(theApp.clientlist->GetXsBuddy()->GetIP());
				data.WriteUInt16(theApp.clientlist->GetXsBuddy()->GetKadPort());
			}
			else
			{
				data.WriteUInt32(0);
				data.WriteUInt16(0);
			}

			//Obfuscation support
			// Note: On a first callback we must tell the cleint our crypto prefs,
			//		on a remote callback we don't send this data as the requester already have them
			if(m_bSendObfu)
			{
				const UINT uSupportsCryptLayer	= thePrefs.IsClientCryptLayerSupported() ? 1 : 0;
				const UINT uRequestsCryptLayer	= thePrefs.IsClientCryptLayerRequested() ? 1 : 0;
				const UINT uRequiresCryptLayer	= thePrefs.IsClientCryptLayerRequired() ? 1 : 0;
				const UINT uObfuscation =
				(uRequiresCryptLayer	<<  2) |
				(uRequestsCryptLayer	<<  1) |
				(uSupportsCryptLayer	<<  0);
				data.WriteUInt8((uint8)uObfuscation);
				//data.WriteHash16(thePrefs.GetUserHash()); // the hash is our id so dont send it twice !
			}

			// send packet over buddy
			Packet* packet = new Packet(&data, OP_MODPROT);
			packet->opcode = OP_XS_MULTICALLBACKUDP;
			theStats.AddUpDataOverheadFileRequest(packet->size);
			theApp.clientudp->SendPacket(packet, m_uRelayIP, m_uRelayPort, false, NULL, true, 0);
			break;
		}
		default:
			ASSERT(0);
	}
}

/**
* SetSocket sets the associated nat socket and sets the tunel pointer of the socket to this object.
*
* @param socket: nat socket using this tunel
*/
void CNatTunnel::SetSocket(CNatSocket* socket)
{
	if(m_Socket){
		ASSERT(m_Socket->GetTunnel() == this); // if we have a socket than it s tunel must point on us
		m_Socket->SetTunnel(NULL);
	}

	if(socket){ // set new socket
		socket->SetTunnel(this);
		m_State = active_nt;
	}else // remove old socket
		m_State = passive_nt;

	m_Socket = socket;
}

//Obfiscation support
void CNatTunnel::SetUserHash(const uchar* pUserHash)
{ 
	m_pUserHash = new uchar[16]; 
	memcpy(m_pUserHash,pUserHash,16); 
}

bool CNatTunnel::ShouldReceiveCryptUDPPackets() const {
	return (thePrefs.IsClientCryptLayerSupported() && SupportsCryptLayer() && theApp.GetPublicIP() != 0
		&& m_pUserHash && (thePrefs.IsClientCryptLayerRequested() || RequestsCryptLayer()) );
}

#endif //NATTUNNELING // NEO: NATT END <-- Xanatos --