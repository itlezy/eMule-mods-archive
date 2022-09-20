/*
Copyright (C)2003 Barry Dunne (http://www.emule-project.net)
 
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the offical client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#include "stdafx.h"
#include "./KademliaUDPListener.h"
#include "../kademlia/Prefs.h"
#include "../kademlia/Kademlia.h"
#include "../kademlia/SearchManager.h"
#include "../kademlia/Indexed.h"
#include "../kademlia/Defines.h"
#include "../kademlia/Entry.h"
#include "../routing/RoutingZone.h"
#include "../io/ByteIO.h"
#include "../../emule.h"
#include "../../ClientUDPSocket.h"
#include "../../Packets.h"
#include "../../emuledlg.h"
#include "../../KadContactListCtrl.h"
#include "../../kademliawnd.h"
#include "../../clientlist.h"
#include "../../Statistics.h"
#include "../../updownclient.h"
#include "../../listensocket.h"
#include "../../Log.h"
#include "../../opcodes.h"
#include "../../ipfilter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern LPCSTR _aszInvKadKeywordCharsA;
extern LPCWSTR _awszInvKadKeywordChars;

using namespace Kademlia;

// Used by Kad1.0 and Kad 2.0
void CKademliaUDPListener::Bootstrap(LPCTSTR szHost, uint16 uUDPPort, bool bKad2)
{
	USES_CONVERSION;
	uint32 uRetVal = 0;
	if (_istalpha((_TUCHAR)szHost[0]))
	{
		hostent *php = gethostbyname(T2CA(szHost));
		if (php == NULL)
			return;
		memcpy (&uRetVal, php->h_addr, sizeof(uRetVal));
	}
	else
		uRetVal = inet_addr(T2CA(szHost));
	Bootstrap(ntohl(uRetVal), uUDPPort, bKad2);
}

// Used by Kad1.0 and Kad 2.0
void CKademliaUDPListener::Bootstrap(uint32 uIP, uint16 uUDPPort, bool bKad2)
{
	if(bKad2)
	{
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA2_BOOTSTRAP_REQ", uIP, uUDPPort);
		CSafeMemFile fileIO(0);
		SendPacket(&fileIO, KADEMLIA2_BOOTSTRAP_REQ, uIP, uUDPPort);
	}
	else
	{
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA_BOOTSTRAP_REQ", uIP, uUDPPort);
		SendMyDetails(KADEMLIA_BOOTSTRAP_REQ, uIP, uUDPPort, bKad2);
	}
}

// Used by Kad1.0 and Kad 2.0
void CKademliaUDPListener::SendMyDetails(byte byOpcode, uint32 uIP, uint16 uUDPPort, bool bKad2)
{
	if(bKad2)
	{
		byte byPacket[1024];
		CByteIO byteIOResponse(byPacket, sizeof(byPacket));
		byteIOResponse.WriteByte(OP_KADEMLIAHEADER);
		byteIOResponse.WriteByte(byOpcode);
		byteIOResponse.WriteUInt128(CKademlia::GetPrefs()->GetKadID());
		byteIOResponse.WriteUInt16(thePrefs.GetPort());
		byteIOResponse.WriteUInt8(KADEMLIA_VERSION);
		// Tag Count.
		byteIOResponse.WriteUInt8(2);
		byteIOResponse.WriteTag(&CKadTagUInt(TAG_USER_COUNT, CKademlia::GetPrefs()->GetKademliaUsers()));
		byteIOResponse.WriteTag(&CKadTagUInt(TAG_FILE_COUNT, CKademlia::GetPrefs()->GetKademliaFiles()));
		uint32 uLen = sizeof(byPacket) - byteIOResponse.GetAvailable();
		SendPacket(byPacket, uLen,  uIP, uUDPPort);
	}
	else
	{
		CSafeMemFile fileIO(25);
		fileIO.WriteUInt128(&CKademlia::GetPrefs()->GetKadID());
		fileIO.WriteUInt32(CKademlia::GetPrefs()->GetIPAddress());
		fileIO.WriteUInt16(thePrefs.GetUDPPort());
		fileIO.WriteUInt16(thePrefs.GetPort());
		fileIO.WriteUInt8(0);
		SendPacket(&fileIO, byOpcode, uIP, uUDPPort);
	}
}

// Kad1.0 & Kad2.0 currently.
void CKademliaUDPListener::FirewalledCheck(uint32 uIP, uint16 uUDPPort)
{
	CSafeMemFile fileIO(2);
	fileIO.WriteUInt16(thePrefs.GetPort());
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA_FIREWALLED_REQ", uIP, uUDPPort);
	SendPacket(&fileIO, KADEMLIA_FIREWALLED_REQ, uIP, uUDPPort);
	theApp.clientlist->AddKadFirewallRequest(ntohl(uIP));
}

// JOHNTODO - Currently only used for Firewalled Ack.. Probably best to remove this method!
void CKademliaUDPListener::SendNullPacket(byte byOpcode,uint32 uIP, uint16 uUDPPort)
{
	CSafeMemFile fileIO(0);
	SendPacket(&fileIO, byOpcode, uIP, uUDPPort);
}

void CKademliaUDPListener::SendPublishSourcePacket(CContact* pContact, const CUInt128 &uTargetID, const CUInt128 &uContactID, const TagList& tags)
{
	//We need to get the tag lists working with CSafeMemFiles..
	byte byPacket[1024];
	CByteIO byteIO(byPacket, sizeof(byPacket));
	byteIO.WriteByte(OP_KADEMLIAHEADER);
	if (pContact->GetVersion() >= 4/*47c*/)
	{
		byteIO.WriteByte(KADEMLIA2_PUBLISH_SOURCE_REQ);
		byteIO.WriteUInt128(uTargetID);
		byteIO.WriteUInt128(uContactID);
		byteIO.WriteTagList(tags);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		{
			DebugSend("KADEMLIA2_PUBLISH_SOURCE_REQ", pContact->GetIPAddress(), pContact->GetUDPPort());
		}
	}
	else
	{
		byteIO.WriteByte(KADEMLIA_PUBLISH_REQ);
		byteIO.WriteUInt128(uTargetID);
		//We only use this for publishing sources now.. So we always send one here..
		byteIO.WriteUInt16(1);
		byteIO.WriteUInt128(uContactID);
		byteIO.WriteTagList(tags);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		{
			DebugSend("KADEMLIA_PUBLISH_REQ", pContact->GetIPAddress(), pContact->GetUDPPort());
		}
	}
	uint32 uLen = sizeof(byPacket) - byteIO.GetAvailable();
	SendPacket(byPacket, uLen,  pContact->GetIPAddress(), pContact->GetUDPPort());
}

void CKademliaUDPListener::ProcessPacket(const byte* pbyData, uint32 uLenData, uint32 uIP, uint16 uUDPPort)
{
	//Update connection state only when it changes.
	bool bCurCon = CKademlia::GetPrefs()->HasHadContact();
	CKademlia::GetPrefs()->SetLastContact();
	if( bCurCon != CKademlia::GetPrefs()->HasHadContact())
		theApp.emuledlg->ShowConnectionState();

	byte byOpcode = pbyData[1];
	const byte *pbyPacketData = pbyData + 2;
	uint32 uLenPacket = uLenData - 2;

	//	AddDebugLogLine( false, _T("Processing UDP Packet from %s port %ld : opcode length %ld", ipstr(senderAddress->sin_addr), ntohs(senderAddress->sin_port), uLenPacket);
	//	CMiscUtils::debugHexDump(pbyPacketData, uLenPacket);

	switch (byOpcode)
	{
		case KADEMLIA_BOOTSTRAP_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_BOOTSTRAP_REQ", uIP, uUDPPort);
			Process_KADEMLIA_BOOTSTRAP_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_BOOTSTRAP_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_BOOTSTRAP_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_BOOTSTRAP_REQ(uIP, uUDPPort);
			break;
		case KADEMLIA_BOOTSTRAP_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_BOOTSTRAP_RES", uIP, uUDPPort);
			Process_KADEMLIA_BOOTSTRAP_RES(pbyPacketData, uLenPacket, uIP);
			break;
		case KADEMLIA2_BOOTSTRAP_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_BOOTSTRAP_RES", uIP, uUDPPort);
			Process_KADEMLIA2_BOOTSTRAP_RES(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_HELLO_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_HELLO_REQ", uIP, uUDPPort);
			Process_KADEMLIA_HELLO_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_HELLO_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_HELLO_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_HELLO_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_HELLO_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_HELLO_RES", uIP, uUDPPort);
			Process_KADEMLIA_HELLO_RES(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_HELLO_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_HELLO_RES", uIP, uUDPPort);
			Process_KADEMLIA2_HELLO_RES(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_REQ", uIP, uUDPPort);
			Process_KADEMLIA_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_RES", uIP, uUDPPort);
			Process_KADEMLIA_RES(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_RES", uIP, uUDPPort);
			Process_KADEMLIA2_RES(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_SEARCH_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_SEARCH_REQ", uIP, uUDPPort);
			Process_KADEMLIA_SEARCH_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_SEARCH_NOTES_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_SEARCH_NOTES_REQ", uIP, uUDPPort);
			Process_KADEMLIA_SEARCH_NOTES_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_SEARCH_NOTES_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_SEARCH_NOTES_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_SEARCH_NOTES_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_SEARCH_KEY_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_SEARCH_KEY_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_SEARCH_KEY_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_SEARCH_SOURCE_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_SEARCH_SOURCE_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_SEARCH_SOURCE_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_SEARCH_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_SEARCH_RES", uIP, uUDPPort);
			Process_KADEMLIA_SEARCH_RES(pbyPacketData, uLenPacket);
			break;
		case KADEMLIA_SEARCH_NOTES_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_SEARCH_NOTES_RES", uIP, uUDPPort);
			Process_KADEMLIA_SEARCH_NOTES_RES(pbyPacketData, uLenPacket, uIP);
			break;
		case KADEMLIA2_SEARCH_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_SEARCH_RES", uIP, uUDPPort);
			Process_KADEMLIA2_SEARCH_RES(pbyPacketData, uLenPacket);
			break;
		case KADEMLIA_PUBLISH_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_PUBLISH_REQ", uIP, uUDPPort);
			Process_KADEMLIA_PUBLISH_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_PUBLISH_NOTES_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_PUBLISH_NOTES_REQ", uIP, uUDPPort);
			Process_KADEMLIA_PUBLISH_NOTES_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_PUBLISH_KEY_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_PUBLISH_KEY_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_PUBLISH_KEY_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_PUBLISH_SOURCE_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_PUBLISH_SOURCE_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_PUBLISH_SOURCE_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA2_PUBLISH_NOTES_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_PUBLISH_NOTES_REQ", uIP, uUDPPort);
			Process_KADEMLIA2_PUBLISH_NOTES_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_PUBLISH_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_PUBLISH_RES", uIP, uUDPPort);
			Process_KADEMLIA_PUBLISH_RES(pbyPacketData, uLenPacket, uIP);
			break;
		case KADEMLIA_PUBLISH_NOTES_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_PUBLISH_NOTES_RES", uIP, uUDPPort);
			Process_KADEMLIA_PUBLISH_NOTES_RES(pbyPacketData, uLenPacket, uIP);
			break;
		case KADEMLIA2_PUBLISH_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA2_PUBLISH_RES", uIP, uUDPPort);
			Process_KADEMLIA2_PUBLISH_RES(pbyPacketData, uLenPacket, uIP);
			break;
		case KADEMLIA_FIREWALLED_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_FIREWALLED_REQ", uIP, uUDPPort);
			Process_KADEMLIA_FIREWALLED_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_FIREWALLED_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_FIREWALLED_RES", uIP, uUDPPort);
			Process_KADEMLIA_FIREWALLED_RES(pbyPacketData, uLenPacket, uIP);
			break;
		case KADEMLIA_FIREWALLED_ACK_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_FIREWALLED_ACK_RES", uIP, uUDPPort);
			Process_KADEMLIA_FIREWALLED_ACK_RES(uLenPacket);
			break;
		case KADEMLIA_FINDBUDDY_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_FINDBUDDY_REQ", uIP, uUDPPort);
			Process_KADEMLIA_FINDBUDDY_REQ(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_FINDBUDDY_RES:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_FINDBUDDY_RES", uIP, uUDPPort);
			Process_KADEMLIA_FINDBUDDY_RES(pbyPacketData, uLenPacket, uIP, uUDPPort);
			break;
		case KADEMLIA_CALLBACK_REQ:
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugRecv("KADEMLIA_CALLBACK_REQ", uIP, uUDPPort);
			Process_KADEMLIA_CALLBACK_REQ(pbyPacketData, uLenPacket, uIP);
			break;
		default:
			{
				CString strError;
				strError.Format(_T("Unknown opcode %02x"), byOpcode);
				throw strError;
			}
	}
}

// Used only for Kad1.0
void CKademliaUDPListener::AddContact( const byte *pbyData, uint32 uLenData, uint32 uIP, uint16 uUDPPort, uint16 uTCPPort, bool bUpdate)
{
	CSafeMemFile fileIO( pbyData, uLenData);
	CUInt128 uID;
	fileIO.ReadUInt128(&uID);
	fileIO.ReadUInt32();
	fileIO.ReadUInt16();
	if( uTCPPort )
		fileIO.ReadUInt16();
	else
		uTCPPort = fileIO.ReadUInt16();
	fileIO.ReadUInt8();
	CKademlia::GetRoutingZone()->Add(uID, uIP, uUDPPort, uTCPPort, 0, bUpdate);
}

// Used only for Kad2.0
void CKademliaUDPListener::AddContact_KADEMLIA2 (const byte* pbyData, uint32 uLenData, uint32 uIP, uint16 uUDPPort, bool bUpdate)
{
	CByteIO byteIO(pbyData, uLenData);
	CUInt128 uID;
	byteIO.ReadUInt128(&uID);
	uint16 uTCPPort = byteIO.ReadUInt16();
	uint8 uVersion = byteIO.ReadByte();
	uint8 uTags = byteIO.ReadByte();
	while(uTags)
	{
		CKadTag* pTag = byteIO.ReadTag();
		delete pTag;
		--uTags;
	}
	CKademlia::GetRoutingZone()->Add(uID, uIP, uUDPPort, uTCPPort, uVersion, bUpdate);
}

// Used only for Kad1.0
void CKademliaUDPListener::AddContacts( const byte *pbyData, uint32 uLenData, uint16 uNumContacts, bool bUpdate)
{
	CSafeMemFile fileIO( pbyData, uLenData );
	CRoutingZone *pRoutingZone = CKademlia::GetRoutingZone();
	CUInt128 uID;
	for (uint16 iIndex=0; iIndex<uNumContacts; iIndex++)
	{
		fileIO.ReadUInt128(&uID);
		uint32 uIP = fileIO.ReadUInt32();
		uint16 uUDPPort = fileIO.ReadUInt16();
		uint16 uTCPPort = fileIO.ReadUInt16();
		fileIO.ReadUInt8();
		pRoutingZone->Add(uID, uIP, uUDPPort, uTCPPort, 0, bUpdate);
	}
}

// Used only for Kad1.0
void CKademliaUDPListener::Process_KADEMLIA_BOOTSTRAP_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket != 25)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	// Add the sender to the list of contacts
	AddContact(pbyPacketData, uLenPacket, uIP, uUDPPort, 0, true);

	// Get some contacts to return
	ContactList contacts;
	uint16 uNumContacts = 1 + (uint16)CKademlia::GetRoutingZone()->GetBootstrapContacts(&contacts, 20);

	// Create response packet
	//We only collect a max of 20 contacts here.. Max size is 527.
	//2 + 25(20) + 25
	CSafeMemFile fileIO(527);

	// Write packet info
	fileIO.WriteUInt16(uNumContacts);
	for (ContactList::const_iterator iContactList = contacts.begin(); iContactList != contacts.end(); ++iContactList)
	{
		CContact* pContact = *iContactList;
		fileIO.WriteUInt128(&pContact->GetClientID());
		fileIO.WriteUInt32(pContact->GetIPAddress());
		fileIO.WriteUInt16(pContact->GetUDPPort());
		fileIO.WriteUInt16(pContact->GetTCPPort());
		fileIO.WriteUInt8(pContact->GetType());
	}
	fileIO.WriteUInt128(&CKademlia::GetPrefs()->GetKadID());
	fileIO.WriteUInt32(CKademlia::GetPrefs()->GetIPAddress());
	fileIO.WriteUInt16(thePrefs.GetUDPPort());
	fileIO.WriteUInt16(thePrefs.GetPort());
	fileIO.WriteUInt8(0);

	// Send response
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA_BOOTSTRAP_RES", uIP, uUDPPort);

	SendPacket(&fileIO, KADEMLIA_BOOTSTRAP_RES, uIP, uUDPPort);
}

// Used only for Kad2.0
void CKademliaUDPListener::Process_KADEMLIA2_BOOTSTRAP_REQ (uint32 uIP, uint16 uUDPPort)
{
	// Get some contacts to return
	ContactList contacts;
	uint16 uNumContacts = (uint16)CKademlia::GetRoutingZone()->GetBootstrapContacts(&contacts, 20);

	// Create response packet
	//We only collect a max of 20 contacts here.. Max size is 527.
	//2 + 25(20) + 19
	CSafeMemFile fileIO(521);

	fileIO.WriteUInt128(&CKademlia::GetPrefs()->GetKadID());
	fileIO.WriteUInt16(thePrefs.GetPort());
	fileIO.WriteUInt8(KADEMLIA_VERSION);

	// Write packet info
	fileIO.WriteUInt16(uNumContacts);
	for (ContactList::const_iterator iContactList = contacts.begin(); iContactList != contacts.end(); ++iContactList)
	{
		CContact* pContact = *iContactList;
		fileIO.WriteUInt128(&pContact->GetClientID());
		fileIO.WriteUInt32(pContact->GetIPAddress());
		fileIO.WriteUInt16(pContact->GetUDPPort());
		fileIO.WriteUInt16(pContact->GetTCPPort());
		fileIO.WriteUInt8(pContact->GetVersion());
	}

	// Send response
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA2_BOOTSTRAP_RES", uIP, uUDPPort);

	SendPacket(&fileIO, KADEMLIA2_BOOTSTRAP_RES, uIP, uUDPPort);
}

// Used only for Kad1.0
void CKademliaUDPListener::Process_KADEMLIA_BOOTSTRAP_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP)
{
	// Verify packet is expected size
	if (uLenPacket < 27)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	else if (!IsOnTrackList(uIP, KADEMLIA_BOOTSTRAP_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	// How many contacts were given
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	uint16 uNumContacts = fileIO.ReadUInt16();

	// Verify packet is expected size
	if (uLenPacket != (UINT)(2 + 25*uNumContacts))
		return;

	// Add these contacts to the list.
	AddContacts(pbyPacketData+2, uLenPacket-2, uNumContacts, false);
}

// Used only for Kad2.0
void CKademliaUDPListener::Process_KADEMLIA2_BOOTSTRAP_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	if (!IsOnTrackList(uIP, KADEMLIA2_BOOTSTRAP_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	CRoutingZone *pRoutingZone = CKademlia::GetRoutingZone();

	// How many contacts were given
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	CUInt128 uContactID;
	fileIO.ReadUInt128(&uContactID);
	uint16 uTCPPort = fileIO.ReadUInt16();
	uint8 uVersion = fileIO.ReadUInt8();
	pRoutingZone->Add(uContactID, uIP, uUDPPort, uTCPPort, uVersion, true);

	uint16 uNumContacts = fileIO.ReadUInt16();
	while(uNumContacts)
	{
		fileIO.ReadUInt128(&uContactID);
		uint32 uIP = fileIO.ReadUInt32();
		uint16 uUDPPort = fileIO.ReadUInt16();
		uint16 uTCPPort = fileIO.ReadUInt16();
		uint8 uVersion = fileIO.ReadUInt8();
		pRoutingZone->Add(uContactID, uIP, uUDPPort, uTCPPort, uVersion, false);
		uNumContacts--;
	}
}

// Used in Kad1.0 only.
void CKademliaUDPListener::Process_KADEMLIA_HELLO_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket != 25)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	// Add the sender to the list of contacts
	AddContact(pbyPacketData, uLenPacket, uIP, uUDPPort, 0, true);

	// Send response
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA_HELLO_RES", uIP, uUDPPort);
	SendMyDetails(KADEMLIA_HELLO_RES, uIP, uUDPPort, false);

	// Check if firewalled
	if(CKademlia::GetPrefs()->GetRecheckIP())
		FirewalledCheck(uIP, uUDPPort);
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_HELLO_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	AddContact_KADEMLIA2(pbyPacketData, uLenPacket, uIP, uUDPPort, true);

	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA2_HELLO_RES", uIP, uUDPPort);
	SendMyDetails(KADEMLIA2_HELLO_RES, uIP, uUDPPort, true);

	// Check if firewalled
	if(CKademlia::GetPrefs()->GetRecheckIP())
		FirewalledCheck(uIP, uUDPPort);
}

// Used in Kad1.0 only
void CKademliaUDPListener::Process_KADEMLIA_HELLO_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	if (!IsOnTrackList(uIP, KADEMLIA_HELLO_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	// Verify packet is expected size
	if (uLenPacket != 25)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	// Add or Update contact.
	AddContact(pbyPacketData, uLenPacket, uIP, uUDPPort, 0, true);
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_HELLO_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	if (!IsOnTrackList(uIP, KADEMLIA2_HELLO_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	// Add or Update contact.
	AddContact_KADEMLIA2(pbyPacketData, uLenPacket, uIP, uUDPPort, true);
}

// Used in Kad1.0 only
void CKademliaUDPListener::Process_KADEMLIA_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket != 33)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	//RecheckIP and firewall status
	if(CKademlia::GetPrefs()->GetRecheckIP())
	{
		FirewalledCheck(uIP, uUDPPort);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA_HELLO_REQ", uIP, uUDPPort);
		SendMyDetails(KADEMLIA_HELLO_REQ, uIP, uUDPPort, false);
	}

	// Get target and type
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	byte byType = fileIO.ReadUInt8();
	//		bool flag1 = (byType >> 6); //Reserved
	//		bool flag2 = (byType >> 7); //Reserved
	//		bool flag3 = (byType >> 8); //Reserved

	byType = byType & 0x1F;
	if( byType == 0 )
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong type (0x%02x) in %hs"), byType, __FUNCTION__);
		throw strError;
	}

	//This is the target node trying to be found.
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
	uDistance.Xor(uTarget);

	//This makes sure we are not mistaken identify. Some client may have fresh installed and have a new hash.
	CUInt128 uCheck;
	fileIO.ReadUInt128(&uCheck);
	if(CKademlia::GetPrefs()->GetKadID() == uCheck)
	{
		// Get required number closest to target
		ContactMap results;
		CKademlia::GetRoutingZone()->GetClosestTo(2, uTarget, uDistance, (int)byType, &results);
		uint16 uCount = (uint16)results.size();

		// Write response
		// Max count is 32. size 817..
		// 16 + 1 + 25(32)
		CSafeMemFile fileIO2( 817 );
		fileIO2.WriteUInt128(&uTarget);
		fileIO2.WriteUInt8((byte)uCount);
		CUInt128 uID;
		for (ContactMap::const_iterator itContactMap = results.begin(); itContactMap != results.end(); ++itContactMap)
		{
			CContact* pContact = itContactMap->second;
			pContact->GetClientID(&uID);
			fileIO2.WriteUInt128(&uID);
			fileIO2.WriteUInt32(pContact->GetIPAddress());
			fileIO2.WriteUInt16(pContact->GetUDPPort());
			fileIO2.WriteUInt16(pContact->GetTCPPort());
			fileIO2.WriteUInt8(pContact->GetType());
		}

		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSendF("KADEMLIA_RES", uIP, uUDPPort, _T("Count=%u"), uCount);

		SendPacket(&fileIO2, KADEMLIA_RES, uIP, uUDPPort);
	}
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Get target and type
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	byte byType = fileIO.ReadUInt8();
	byType = byType & 0x1F;
	if( byType == 0 )
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong type (0x%02x) in %hs"), byType, __FUNCTION__);
		throw strError;
	}

	//This is the target node trying to be found.
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	//Convert Target to Distance as this is how we store contacts.
	CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
	uDistance.Xor(uTarget);

	//This makes sure we are not mistaken identify. Some client may have fresh installed and have a new KadID.
	CUInt128 uCheck;
	fileIO.ReadUInt128(&uCheck);
	if(CKademlia::GetPrefs()->GetKadID() == uCheck)
	{
		// Get required number closest to target
		ContactMap results;
		CKademlia::GetRoutingZone()->GetClosestTo(2, uTarget, uDistance, (uint32)byType, &results);
		uint8 uCount = (uint8)results.size();

		// Write response
		// Max count is 32. size 817..
		// 16 + 1 + 25(32)
		CSafeMemFile fileIO2( 817 );
		fileIO2.WriteUInt128(&uTarget);
		fileIO2.WriteUInt8(uCount);
		CUInt128 uID;
		for (ContactMap::const_iterator itContactMap = results.begin(); itContactMap != results.end(); ++itContactMap)
		{
			CContact* pContact = itContactMap->second;
			pContact->GetClientID(&uID);
			fileIO2.WriteUInt128(&uID);
			fileIO2.WriteUInt32(pContact->GetIPAddress());
			fileIO2.WriteUInt16(pContact->GetUDPPort());
			fileIO2.WriteUInt16(pContact->GetTCPPort());
			fileIO2.WriteUInt8(pContact->GetVersion()); //<- Kad Version inserted to allow backward compatability.
		}

		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSendF("KADEMLIA2_RES", uIP, uUDPPort, _T("Count=%u"), uCount);

		SendPacket(&fileIO2, KADEMLIA2_RES, uIP, uUDPPort);
	}
}

// Used in Kad1.0 only
void CKademliaUDPListener::Process_KADEMLIA_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket < 17)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	else if (!IsOnTrackList(uIP, KADEMLIA_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	//Used Pointers
	CRoutingZone *pRoutingZone = CKademlia::GetRoutingZone();

	if(CKademlia::GetPrefs()->GetRecheckIP())
	{
		FirewalledCheck(uIP, uUDPPort);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA_HELLO_REQ", uIP, uUDPPort);
		SendMyDetails(KADEMLIA_HELLO_REQ, uIP, uUDPPort, false);
	}

	// What search does this relate to
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	uint16 uNumContacts = fileIO.ReadUInt8();

	// Verify packet is expected size
	if (uLenPacket != (UINT)(16+1 + (16+4+2+2+1)*uNumContacts))
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	ContactList *pResults = new ContactList;
	CUInt128 uIDResult;
	try
	{
		for (uint16 iIndex=0; iIndex<uNumContacts; iIndex++)
		{
			fileIO.ReadUInt128(&uIDResult);
			uint32 uIPResult = fileIO.ReadUInt32();
			uint16 uUDPPortResult = fileIO.ReadUInt16();
			uint16 uTCPPortResult = fileIO.ReadUInt16();
			fileIO.ReadUInt8();
			uint32 uhostIPResult = ntohl(uIPResult);
			if (::IsGoodIPPort(uhostIPResult, uUDPPortResult))
			{
				if (!::theApp.ipfilter->IsFiltered(uhostIPResult)) {
					pRoutingZone->AddUnfiltered(uIDResult, uIPResult, uUDPPortResult, uTCPPortResult, 0, false);
					pResults->push_back(new CContact(uIDResult, uIPResult, uUDPPortResult, uTCPPortResult, uTarget, 0));
				}
				else if (::thePrefs.GetLogFilteredIPs())
					AddDebugLogLine(false, _T("Ignored kad contact (IP=%s) - IP filter (%s)") , ipstr(uhostIPResult), ::theApp.ipfilter->GetLastHit());
			}
			else if (::thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Ignored kad contact (IP=%s) - Bad IP"), ipstr(uhostIPResult));
		}
	}
	catch(...)
	{
		for (ContactList::const_iterator it = pResults->begin(); it != pResults->end(); ++it)
			delete *it;
		delete pResults;
		throw;
	}
	CSearchManager::ProcessResponse(uTarget, uIP, uUDPPort, pResults);
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	if (!IsOnTrackList(uIP, KADEMLIA2_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	//Used Pointers
	CRoutingZone *pRoutingZone = CKademlia::GetRoutingZone();

	if(CKademlia::GetPrefs()->GetRecheckIP())
	{
		FirewalledCheck(uIP, uUDPPort);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA2_HELLO_REQ", uIP, uUDPPort);
		SendMyDetails(KADEMLIA2_HELLO_REQ, uIP, uUDPPort, true);
	}

	// What search does this relate to
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	uint8 uNumContacts = fileIO.ReadUInt8();

	// Verify packet is expected size
	if (uLenPacket != (UINT)(16+1 + (16+4+2+2+1)*uNumContacts))
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	ContactList *pResults = new ContactList;
	CUInt128 uIDResult;
	try
	{
		for (uint8 iIndex=0; iIndex<uNumContacts; iIndex++)
		{
			fileIO.ReadUInt128(&uIDResult);
			uint32 uIPResult = fileIO.ReadUInt32();
			uint16 uUDPPortResult = fileIO.ReadUInt16();
			uint16 uTCPPortResult = fileIO.ReadUInt16();
			uint8 uVersion = fileIO.ReadUInt8();
			uint32 uhostIPResult = ntohl(uIPResult);
			if (::IsGoodIPPort(uhostIPResult, uUDPPortResult))
			{
				if (!::theApp.ipfilter->IsFiltered(uhostIPResult)) {
					pRoutingZone->AddUnfiltered(uIDResult, uIPResult, uUDPPortResult, uTCPPortResult, uVersion, false);
					pResults->push_back(new CContact(uIDResult, uIPResult, uUDPPortResult, uTCPPortResult, uTarget, uVersion));
				}
				else if (::thePrefs.GetLogFilteredIPs())
					AddDebugLogLine(false, _T("Ignored kad contact (IP=%s) - IP filter (%s)") , ipstr(uhostIPResult), ::theApp.ipfilter->GetLastHit());
			}
			else if (::thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Ignored kad contact (IP=%s) - Bad IP"), ipstr(uhostIPResult));
		}
	}
	catch(...)
	{
		for (ContactList::const_iterator it = pResults->begin(); it != pResults->end(); ++it)
			delete *it;
		delete pResults;
		throw;
	}
	CSearchManager::ProcessResponse(uTarget, uIP, uUDPPort, pResults);
}

void CKademliaUDPListener::Free(SSearchTerm* pSearchTerms)
{
	if(pSearchTerms)
	{
		if (pSearchTerms->m_pLeft)
			Free(pSearchTerms->m_pLeft);
		if (pSearchTerms->m_pRight)
			Free(pSearchTerms->m_pRight);
		delete pSearchTerms;
	}
}

static void TokenizeOptQuotedSearchTerm(LPCTSTR pszString, CStringWArray* lst)
{
	LPCTSTR pch = pszString;
	while (*pch != _T('\0'))
	{
		if (*pch == _T('\"'))
		{
			// Start of quoted string found. If there is no terminating quote character found,
			// the start quote character is just skipped. If the quoted string is empty, no
			// new entry is added to 'list'.
			//
			pch++;
			LPCTSTR pchNextQuote = _tcschr(pch, _T('\"'));
			if (pchNextQuote)
			{
				size_t nLenQuoted = pchNextQuote - pch;
				if (nLenQuoted)
					lst->Add(CString(pch, nLenQuoted));
				pch = pchNextQuote + 1;
			}
		}
		else
		{
			// Search for next delimiter or quote character
			//
			size_t nNextDelimiter = _tcscspn(pch, _T(INV_KAD_KEYWORD_CHARS) _T("\""));
			if (nNextDelimiter)
			{
				lst->Add(CString(pch, nNextDelimiter));
				pch += nNextDelimiter;
				if (*pch == _T('\0'))
					break;
				if (*pch == _T('\"'))
					continue;
			}
			pch++;
		}
	}
}

static CString* _pstrDbgSearchExpr;

SSearchTerm* CKademliaUDPListener::CreateSearchExpressionTree(CSafeMemFile& fileIO, int iLevel)
{
	// the max. depth has to match our own limit for creating the search expression
	// (see also 'ParsedSearchExpression' and 'GetSearchPacket')
	if (iLevel >= 24)
	{
		AddDebugLogLine(false, _T("***NOTE: Search expression tree exceeds depth limit!"));
		return NULL;
	}
	iLevel++;

	uint8 uOp = fileIO.ReadUInt8();
	if (uOp == 0x00)
	{
		uint8 uBoolOp = fileIO.ReadUInt8();
		if (uBoolOp == 0x00) // AND
		{
			SSearchTerm* pSearchTerm = new SSearchTerm;
			pSearchTerm->m_type = SSearchTerm::AND;
			if (_pstrDbgSearchExpr)
				_pstrDbgSearchExpr->Append(_T(" AND"));
			if ((pSearchTerm->m_pLeft = CreateSearchExpressionTree(fileIO, iLevel)) == NULL)
			{
				ASSERT(0);
				delete pSearchTerm;
				return NULL;
			}
			if ((pSearchTerm->m_pRight = CreateSearchExpressionTree(fileIO, iLevel)) == NULL)
			{
				ASSERT(0);
				Free(pSearchTerm->m_pLeft);
				delete pSearchTerm;
				return NULL;
			}
			return pSearchTerm;
		}
		else if (uBoolOp == 0x01) // OR
		{
			SSearchTerm* pSearchTerm = new SSearchTerm;
			pSearchTerm->m_type = SSearchTerm::OR;
			if (_pstrDbgSearchExpr)
				_pstrDbgSearchExpr->Append(_T(" OR"));
			if ((pSearchTerm->m_pLeft = CreateSearchExpressionTree(fileIO, iLevel)) == NULL)
			{
				ASSERT(0);
				delete pSearchTerm;
				return NULL;
			}
			if ((pSearchTerm->m_pRight = CreateSearchExpressionTree(fileIO, iLevel)) == NULL)
			{
				ASSERT(0);
				Free(pSearchTerm->m_pLeft);
				delete pSearchTerm;
				return NULL;
			}
			return pSearchTerm;
		}
		else if (uBoolOp == 0x02) // NOT
		{
			SSearchTerm* pSearchTerm = new SSearchTerm;
			pSearchTerm->m_type = SSearchTerm::NOT;
			if (_pstrDbgSearchExpr)
				_pstrDbgSearchExpr->Append(_T(" NOT"));
			if ((pSearchTerm->m_pLeft = CreateSearchExpressionTree(fileIO, iLevel)) == NULL)
			{
				ASSERT(0);
				delete pSearchTerm;
				return NULL;
			}
			if ((pSearchTerm->m_pRight = CreateSearchExpressionTree(fileIO, iLevel)) == NULL)
			{
				ASSERT(0);
				Free(pSearchTerm->m_pLeft);
				delete pSearchTerm;
				return NULL;
			}
			return pSearchTerm;
		}
		else
		{
			AddDebugLogLine(false, _T("*** Unknown boolean search operator 0x%02x (CreateSearchExpressionTree)"), uBoolOp);
			return NULL;
		}
	}
	else if (uOp == 0x01) // String
	{
		CKadTagValueString str(fileIO.ReadStringUTF8());

		KadTagStrMakeLower(str); // make lowercase, the search code expects lower case strings!
		if (_pstrDbgSearchExpr)
			_pstrDbgSearchExpr->AppendFormat(_T(" \"%ls\""), str);

		SSearchTerm* pSearchTerm = new SSearchTerm;
		pSearchTerm->m_type = SSearchTerm::String;
		pSearchTerm->m_pastr = new CStringWArray;

		// pre-tokenize the string term (care about quoted parts)
		TokenizeOptQuotedSearchTerm(str, pSearchTerm->m_pastr);

		return pSearchTerm;
	}
	else if (uOp == 0x02) // Meta tag
	{
		// read tag value
		CKadTagValueString strValue(fileIO.ReadStringUTF8());

		KadTagStrMakeLower(strValue); // make lowercase, the search code expects lower case strings!

		// read tag name
		CStringA strTagName;
		uint16 lenTagName = fileIO.ReadUInt16();
		fileIO.Read(strTagName.GetBuffer(lenTagName), lenTagName);
		strTagName.ReleaseBuffer(lenTagName);

		SSearchTerm* pSearchTerm = new SSearchTerm;
		pSearchTerm->m_type = SSearchTerm::MetaTag;
		pSearchTerm->m_pTag = new Kademlia::CKadTagStr(strTagName, strValue);
		if (_pstrDbgSearchExpr)
		{
			if (lenTagName == 1)
				_pstrDbgSearchExpr->AppendFormat(_T(" Tag%02X=\"%ls\""), (BYTE)strTagName[0], strValue);
			else
				_pstrDbgSearchExpr->AppendFormat(_T(" \"%s\"=\"%ls\""), strTagName, strValue);
		}
		return pSearchTerm;
	}
	else if (uOp == 0x03 || uOp == 0x08) // Numeric Relation (0x03=32-bit or 0x08=64-bit)
	{
		static const struct
		{
			SSearchTerm::ESearchTermType eSearchTermOp;
			LPCTSTR pszOp;
		}
		_aOps[] =
		    {
		        { SSearchTerm::OpEqual,			_T("=")		}, // mmop=0x00
		        { SSearchTerm::OpGreater,		_T(">")		}, // mmop=0x01
		        { SSearchTerm::OpLess,			_T("<")		}, // mmop=0x02
		        { SSearchTerm::OpGreaterEqual,	_T(">=")	}, // mmop=0x03
		        { SSearchTerm::OpLessEqual,		_T("<=")	}, // mmop=0x04
		        { SSearchTerm::OpNotEqual,		_T("<>")	}  // mmop=0x05
		    };

		// read tag value
		uint64 ullValue = (uOp == 0x03) ? fileIO.ReadUInt32() : fileIO.ReadUInt64();

		// read integer operator
		uint8 mmop = fileIO.ReadUInt8();
		if (mmop >= ARRSIZE(_aOps))
		{
			AddDebugLogLine(false, _T("*** Unknown integer search op=0x%02x (CreateSearchExpressionTree)"), mmop);
			return NULL;
		}

		// read tag name
		CStringA strTagName;
		uint16 uLenTagName = fileIO.ReadUInt16();
		fileIO.Read(strTagName.GetBuffer(uLenTagName), uLenTagName);
		strTagName.ReleaseBuffer(uLenTagName);

		SSearchTerm* pSearchTerm = new SSearchTerm;
		pSearchTerm->m_type = _aOps[mmop].eSearchTermOp;
		pSearchTerm->m_pTag = new Kademlia::CKadTagUInt64(strTagName, ullValue);

		if (_pstrDbgSearchExpr)
		{
			if (uLenTagName == 1)
				_pstrDbgSearchExpr->AppendFormat(_T(" Tag%02X%s%I64u"), (BYTE)strTagName[0], _aOps[mmop].pszOp, ullValue);
			else
				_pstrDbgSearchExpr->AppendFormat(_T(" \"%s\"%s%I64u"), strTagName, _aOps[mmop].pszOp, ullValue);
		}

		return pSearchTerm;
	}
	else
	{
		AddDebugLogLine(false, _T("*** Unknown search op=0x%02x (CreateSearchExpressionTree)"), uOp);
		return NULL;
	}
}

// Used in Kad1.0 only
void CKademliaUDPListener::Process_KADEMLIA_SEARCH_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket < 17)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	uint8 uRestrictive = fileIO.ReadUInt8();

	if(uLenPacket == 17 )
	{
		if(uRestrictive)
			CKademlia::GetIndexed()->SendValidSourceResult(uTarget, uIP, uUDPPort, false, 0, 0);
		else
			CKademlia::GetIndexed()->SendValidKeywordResult(uTarget, NULL, uIP, uUDPPort, true, false, 0);
	}
	else if(uLenPacket > 17)
	{
		SSearchTerm* pSearchTerms = NULL;
		bool bOldClient = true;
		if (uRestrictive)
		{
			try
			{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
				_pstrDbgSearchExpr = (thePrefs.GetDebugServerSearchesLevel() > 0) ? new CString : NULL;
#endif

				pSearchTerms = CreateSearchExpressionTree(fileIO, 0);
				if (_pstrDbgSearchExpr)
				{
					Debug(_T("KadSearchTerm=%s\n"), *_pstrDbgSearchExpr);
					delete _pstrDbgSearchExpr;
					_pstrDbgSearchExpr = NULL;
				}
			}
			catch(...)
			{
				delete _pstrDbgSearchExpr;
				_pstrDbgSearchExpr = NULL;
				Free(pSearchTerms);
				throw;
			}
			if (pSearchTerms == NULL)
				throw CString(_T("Invalid search expression"));
			if(uRestrictive>1)
				bOldClient = false;
		}
		else
			bOldClient = false;

		//Keyword request with added options.
		CKademlia::GetIndexed()->SendValidKeywordResult(uTarget, pSearchTerms, uIP, uUDPPort, bOldClient, false, 0x0000);
		Free(pSearchTerms);
	}
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_SEARCH_KEY_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	uint16 uStartPosition = fileIO.ReadUInt16();
	bool uRestrictive = ((uStartPosition & 0x8000) == 0x8000);
	uStartPosition = uStartPosition & 0x7FFF;
	SSearchTerm* pSearchTerms = NULL;
	if (uRestrictive)
	{
		try
		{
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
			_pstrDbgSearchExpr = (thePrefs.GetDebugServerSearchesLevel() > 0) ? new CString : NULL;
#endif
			pSearchTerms = CreateSearchExpressionTree(fileIO, 0);
			if (_pstrDbgSearchExpr)
			{
				Debug(_T("KadSearchTerm=%s\n"), *_pstrDbgSearchExpr);
				delete _pstrDbgSearchExpr;
				_pstrDbgSearchExpr = NULL;
			}
		}
		catch(...)
		{
			delete _pstrDbgSearchExpr;
			_pstrDbgSearchExpr = NULL;
			Free(pSearchTerms);
			throw;
		}
		if (pSearchTerms == NULL)
			throw CString(_T("Invalid search expression"));
	}
	CKademlia::GetIndexed()->SendValidKeywordResult(uTarget, pSearchTerms, uIP, uUDPPort, false, true, uStartPosition);
	if(pSearchTerms)
		Free(pSearchTerms);
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_SEARCH_SOURCE_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	uint16 uStartPosition = (fileIO.ReadUInt16() & 0x7FFF);
	uint64 uFileSize = fileIO.ReadUInt64();
	CKademlia::GetIndexed()->SendValidSourceResult(uTarget, uIP, uUDPPort, true, uStartPosition, uFileSize);
}

// Used in Kad1.0 only
void CKademliaUDPListener::Process_KADEMLIA_SEARCH_RES (const byte *pbyPacketData, uint32 uLenPacket)
{
	// Verify packet is expected size
	if (uLenPacket < 37)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	// What search does this relate to
	CByteIO byteIO(pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	byteIO.ReadUInt128(&uTarget);

	// How many results.. Not supported yet..
	uint16 uCount = byteIO.ReadUInt16();
	CUInt128 uAnswer;
	while( uCount > 0 )
	{
		// What is the answer
		byteIO.ReadUInt128(&uAnswer);

		// Get info about answer
		// NOTE: this is the one and only place in Kad where we allow string conversion to local code page in
		// case we did not receive an UTF8 string. this is for backward compatibility for search results which are
		// supposed to be 'viewed' by user only and not feed into the Kad engine again!
		// If that tag list is once used for something else than for viewing, special care has to be taken for any
		// string conversion!
		TagList* pTags = new TagList;
		try
		{
			byteIO.ReadTagList(pTags, true);
		}
		catch(...)
		{
			deleteTagListEntries(pTags);
			delete pTags;
			pTags = NULL;
			throw;
		}
		CSearchManager::ProcessResult(uTarget, uAnswer, pTags);
		uCount--;
	}
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_SEARCH_RES (const byte *pbyPacketData, uint32 uLenPacket)
{
	CByteIO byteIO(pbyPacketData, uLenPacket);

	// Who sent this packet.
	CUInt128 uSource;
	byteIO.ReadUInt128(&uSource);

	// What search does this relate to
	CUInt128 uTarget;
	byteIO.ReadUInt128(&uTarget);

	// Total results.
	uint16 uCount = byteIO.ReadUInt16();
	CUInt128 uAnswer;
	while( uCount > 0 )
	{
		// What is the answer
		byteIO.ReadUInt128(&uAnswer);

		// Get info about answer
		// NOTE: this is the one and only place in Kad where we allow string conversion to local code page in
		// case we did not receive an UTF8 string. this is for backward compatibility for search results which are
		// supposed to be 'viewed' by user only and not feed into the Kad engine again!
		// If that tag list is once used for something else than for viewing, special care has to be taken for any
		// string conversion!
		TagList* pTags = new TagList;
		try
		{
			byteIO.ReadTagList(pTags, true);
		}
		catch(...)
		{
			deleteTagListEntries(pTags);
			delete pTags;
			pTags = NULL;
			throw;
		}
		CSearchManager::ProcessResult(uTarget, uAnswer, pTags);
		uCount--;
	}
}

// Used in Kad1.0 only
void CKademliaUDPListener::Process_KADEMLIA_PUBLISH_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	//There are different types of publishing..
	//Keyword and File are Stored..
	// Verify packet is expected size
	if (uLenPacket < 37)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	//Used Pointers
	CIndexed *pIndexed = CKademlia::GetIndexed();

	if( CKademlia::GetPrefs()->GetFirewalled() )
		//We are firewalled. We should not index this entry and give publisher a false report.
		return;

	CByteIO byteIO(pbyPacketData, uLenPacket);
	CUInt128 uFile;
	byteIO.ReadUInt128(&uFile);

	CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
	uDistance.Xor(uFile);

	if( thePrefs.FilterLANIPs() && uDistance.Get32BitChunk(0) > SEARCHTOLERANCE)
		return;

	bool bDbgInfo = (thePrefs.GetDebugClientKadUDPLevel() > 0);
	CString sInfo;
	uint16 uCount = byteIO.ReadUInt16();
	bool bFlag = false;
	uint8 uLoad = 0;
	CUInt128 uTarget;
	while( uCount > 0 )
	{
		sInfo.Empty();

		byteIO.ReadUInt128(&uTarget);

		CEntry* pEntry = new Kademlia::CEntry();
		try
		{
			pEntry->m_uIP = uIP;
			pEntry->m_uUDPPort = uUDPPort;
			pEntry->m_uKeyID.SetValue(uFile);
			pEntry->m_uSourceID.SetValue(uTarget);
			uint32 uTags = byteIO.ReadByte();
			while(uTags > 0)
			{
				CKadTag* pTag = byteIO.ReadTag();
				if(pTag)
				{
					if (!pTag->m_name.Compare(TAG_SOURCETYPE))
					{
						if( pEntry->m_bSource == false )
						{
							pEntry->m_listTag.push_back(new CKadTagUInt(TAG_SOURCEIP, pEntry->m_uIP));
							pEntry->m_listTag.push_back(new CKadTagUInt(TAG_SOURCEUPORT, pEntry->m_uUDPPort));
							pEntry->m_listTag.push_back(pTag);
							pEntry->m_bSource = true;
						}
						else
						{
							//More then one sourcetype tag found.
							delete pTag;
						}
					}
					else if (!pTag->m_name.Compare(TAG_FILENAME))
					{
						if ( pEntry->m_fileName.IsEmpty() )
						{
							pEntry->m_fileName = pTag->GetStr();
							KadTagStrMakeLower(pEntry->m_fileName); // make lowercase, the search code expects lower case strings!
							if (bDbgInfo)
								sInfo.AppendFormat(_T("  Name=\"%ls\""), pEntry->m_fileName);
							// NOTE: always add the 'name' tag, even if it's stored separately in 'fileName'. the tag is still needed for answering search request
							pEntry->m_listTag.push_back(pTag);
						}
						else
						{
							//More then one Name tag found.
							delete pTag;
						}
					}
					else if (!pTag->m_name.Compare(TAG_FILESIZE))
					{
						if( pEntry->m_uSize == 0 )
						{
							if(pTag->IsBsob() && pTag->GetBsobSize() == 8)
							{
								pEntry->m_uSize = *((uint64*)pTag->GetBsob());
								delete pTag;
								pTag = new CKadTagUInt(TAG_FILESIZE, pEntry->m_uSize);
							}
							else
								pEntry->m_uSize = pTag->GetInt();
							if (bDbgInfo)
								sInfo.AppendFormat(_T("  Size=%u"), pEntry->m_uSize);
							// NOTE: always add the 'size' tag, even if it's stored separately in 'size'. the tag is still needed for answering search request
							pEntry->m_listTag.push_back(pTag);
						}
						else
						{
							//More then one size tag found
							delete pTag;
						}
					}
					else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
					{
						if( pEntry->m_uTCPPort == 0 )
						{
							pEntry->m_uTCPPort = (uint16)pTag->GetInt();
							pEntry->m_listTag.push_back(pTag);
						}
						else
						{
							//More then one port tag found
							delete pTag;
						}
					}
					else
					{
						//TODO: Filter tags
						pEntry->m_listTag.push_back(pTag);
					}
				}
				uTags--;
			}
			if (bDbgInfo && !sInfo.IsEmpty())
				Debug(_T("%s\n"), sInfo);
		}
		catch(...)
		{
			delete pEntry;
			throw;
		}

		if( pEntry->m_bSource == true )
		{
			pEntry->m_tLifetime = (uint32)time(NULL)+KADEMLIAREPUBLISHTIMES;
			if( pIndexed->AddSources(uFile, uTarget, pEntry, uLoad ) )
				bFlag = true;
			else
			{
				delete pEntry;
				pEntry = NULL;
			}
		}
		else
		{
			pEntry->m_tLifetime = (uint32)time(NULL)+KADEMLIAREPUBLISHTIMEK;
			if( pIndexed->AddKeyword(uFile, uTarget, pEntry, uLoad) )
			{
				//This makes sure we send a publish response.. This also makes sure we index all the files for this keyword.
				bFlag = true;
			}
			else
			{
				//We already indexed the maximum number of keywords.
				//We do not index anymore but we still send a success..
				//Reason: Because if a VERY busy node tells the publisher it failed,
				//this busy node will spread to all the surrounding nodes causing popular
				//keywords to be stored on MANY nodes..
				//So, once we are full, we will periodically clean our list until we can
				//begin storing again..
				bFlag = true;
				delete pEntry;
				pEntry = NULL;
			}
		}
		uCount--;
	}
	if( bFlag )
	{
		CSafeMemFile fileIO2(17);
		fileIO2.WriteUInt128(&uFile);
		fileIO2.WriteUInt8(uLoad);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA_PUBLISH_RES", uIP, uUDPPort);

		SendPacket( &fileIO2, KADEMLIA_PUBLISH_RES, uIP, uUDPPort);
	}
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_PUBLISH_KEY_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	//Used Pointers
	CIndexed *pIndexed = CKademlia::GetIndexed();

	if( CKademlia::GetPrefs()->GetFirewalled() )
		//We are firewalled. We should not index this entry and give publisher a false report.
		return;

	CByteIO byteIO(pbyPacketData, uLenPacket);
	CUInt128 uFile;
	byteIO.ReadUInt128(&uFile);

	CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
	uDistance.Xor(uFile);

	// Shouldn't LAN IPs already be filtered?
	if( thePrefs.FilterLANIPs() && uDistance.Get32BitChunk(0) > SEARCHTOLERANCE)
		return;

	bool bDbgInfo = (thePrefs.GetDebugClientKadUDPLevel() > 0);
	CString sInfo;
	uint16 uCount = byteIO.ReadUInt16();
	uint8 uLoad = 0;
	CUInt128 uTarget;
	while( uCount > 0 )
	{
		sInfo.Empty();

		byteIO.ReadUInt128(&uTarget);

		CEntry* pEntry = new Kademlia::CEntry();
		try
		{
			pEntry->m_uIP = uIP;
			pEntry->m_uUDPPort = uUDPPort;
			pEntry->m_uKeyID.SetValue(uFile);
			pEntry->m_uSourceID.SetValue(uTarget);
			pEntry->m_tLifetime = (uint32)time(NULL)+KADEMLIAREPUBLISHTIMEK;
			pEntry->m_bSource = false;
			uint32 uTags = byteIO.ReadByte();
			while(uTags > 0)
			{
				CKadTag* pTag = byteIO.ReadTag();
				if(pTag)
				{
					if (!pTag->m_name.Compare(TAG_FILENAME))
					{
						if ( pEntry->m_fileName.IsEmpty() )
						{
							pEntry->m_fileName = pTag->GetStr();
							KadTagStrMakeLower(pEntry->m_fileName); // make lowercase, the search code expects lower case strings!
							if (bDbgInfo)
								sInfo.AppendFormat(_T("  Name=\"%ls\""), pEntry->m_fileName);
							// NOTE: always add the 'name' tag, even if it's stored separately in 'fileName'. the tag is still needed for answering search request
							pEntry->m_listTag.push_back(pTag);
						}
						else
						{
							//More then one Name tag found.
							delete pTag;
						}
					}
					else if (!pTag->m_name.Compare(TAG_FILESIZE))
					{
						if( pEntry->m_uSize == 0 )
						{
							pEntry->m_uSize = pTag->GetInt();
							if (bDbgInfo)
								sInfo.AppendFormat(_T("  Size=%u"), pEntry->m_uSize);
							// NOTE: always add the 'size' tag, even if it's stored separately in 'size'. the tag is still needed for answering search request
							pEntry->m_listTag.push_back(pTag);
						}
						else
						{
							//More then one size tag found
							delete pTag;
						}
					}
					else
					{
						//TODO: Filter tags
						pEntry->m_listTag.push_back(pTag);
					}
				}
				uTags--;
			}
			if (bDbgInfo && !sInfo.IsEmpty())
				Debug(_T("%s\n"), sInfo);
		}
		catch(...)
		{
			delete pEntry;
			throw;
		}

		if( !pIndexed->AddKeyword(uFile, uTarget, pEntry, uLoad) )
		{
			//We already indexed the maximum number of keywords.
			//We do not index anymore but we still send a success..
			//Reason: Because if a VERY busy node tells the publisher it failed,
			//this busy node will spread to all the surrounding nodes causing popular
			//keywords to be stored on MANY nodes..
			//So, once we are full, we will periodically clean our list until we can
			//begin storing again..
			delete pEntry;
			pEntry = NULL;
		}
		uCount--;
	}
	CSafeMemFile fileIO2(17);
	fileIO2.WriteUInt128(&uFile);
	fileIO2.WriteUInt8(uLoad);
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA2_PUBLISH_RES", uIP, uUDPPort);
	SendPacket( &fileIO2, KADEMLIA2_PUBLISH_RES, uIP, uUDPPort);
}

// Used in Kad2.0 only
void CKademliaUDPListener::Process_KADEMLIA2_PUBLISH_SOURCE_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	//Used Pointers
	CIndexed *pIndexed = CKademlia::GetIndexed();

	if( CKademlia::GetPrefs()->GetFirewalled() )
		//We are firewalled. We should not index this entry and give publisher a false report.
		return;

	CByteIO byteIO(pbyPacketData, uLenPacket);
	CUInt128 uFile;
	byteIO.ReadUInt128(&uFile);

	CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
	uDistance.Xor(uFile);

	if( thePrefs.FilterLANIPs() && uDistance.Get32BitChunk(0) > SEARCHTOLERANCE)
		return;

	bool bDbgInfo = (thePrefs.GetDebugClientKadUDPLevel() > 0);
	CString sInfo;
	sInfo.Empty();
	uint8 uLoad = 0;
	bool bFlag = false;
	CUInt128 uTarget;
	byteIO.ReadUInt128(&uTarget);
	CEntry* pEntry = new Kademlia::CEntry();
	try
	{
		pEntry->m_uIP = uIP;
		pEntry->m_uUDPPort = uUDPPort;
		pEntry->m_uKeyID.SetValue(uFile);
		pEntry->m_uSourceID.SetValue(uTarget);
		pEntry->m_bSource = false;
		pEntry->m_tLifetime = (uint32)time(NULL)+KADEMLIAREPUBLISHTIMES;
		uint32 uTags = byteIO.ReadByte();
		while(uTags > 0)
		{
			CKadTag* pTag = byteIO.ReadTag();
			if(pTag)
			{
				if (!pTag->m_name.Compare(TAG_SOURCETYPE))
				{
					if( pEntry->m_bSource == false )
					{
						pEntry->m_listTag.push_back(new CKadTagUInt(TAG_SOURCEIP, pEntry->m_uIP));
						pEntry->m_listTag.push_back(new CKadTagUInt(TAG_SOURCEUPORT, pEntry->m_uUDPPort));
						pEntry->m_listTag.push_back(pTag);
						pEntry->m_bSource = true;
					}
					else
					{
						//More then one sourcetype tag found.
						delete pTag;
					}
				}
				else if (!pTag->m_name.Compare(TAG_FILESIZE))
				{
					if( pEntry->m_uSize == 0 )
					{
						pEntry->m_uSize = pTag->GetInt();
						if (bDbgInfo)
							sInfo.AppendFormat(_T("  Size=%u"), pEntry->m_uSize);
						// NOTE: always add the 'size' tag, even if it's stored separately in 'size'. the tag is still needed for answering search request
						pEntry->m_listTag.push_back(pTag);
					}
					else
					{
						//More then one size tag found
						delete pTag;
					}
				}
				else if (!pTag->m_name.Compare(TAG_SOURCEPORT))
				{
					if( pEntry->m_uTCPPort == 0 )
					{
						pEntry->m_uTCPPort = (uint16)pTag->GetInt();
						pEntry->m_listTag.push_back(pTag);
					}
					else
					{
						//More then one port tag found
						delete pTag;
					}
				}
				else
				{
					//TODO: Filter tags
					pEntry->m_listTag.push_back(pTag);
				}
			}
			uTags--;
		}
		if (bDbgInfo && !sInfo.IsEmpty())
			Debug(_T("%s\n"), sInfo);
	}
	catch(...)
	{
		delete pEntry;
		throw;
	}

	if( pEntry->m_bSource == true )
	{
		if( pIndexed->AddSources(uFile, uTarget, pEntry, uLoad ) )
			bFlag = true;
		else
		{
			delete pEntry;
			pEntry = NULL;
		}
	}
	else
	{
		delete pEntry;
		pEntry = NULL;
	}
	if( bFlag )
	{
		CSafeMemFile fileIO2(17);
		fileIO2.WriteUInt128(&uFile);
		fileIO2.WriteUInt8(uLoad);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA2_PUBLISH_RES", uIP, uUDPPort);
		SendPacket( &fileIO2, KADEMLIA2_PUBLISH_RES, uIP, uUDPPort);
	}
}

// Used only by Kad1.0
void CKademliaUDPListener::Process_KADEMLIA_PUBLISH_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP)
{
	// Verify packet is expected size
	if (uLenPacket < 16)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	else if (!IsOnTrackList(uIP, KADEMLIA_PUBLISH_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	CUInt128 uFile;
	fileIO.ReadUInt128(&uFile);

	bool bLoadResponse = false;
	uint8 uLoad = 0;
	if( fileIO.GetLength() > fileIO.GetPosition() )
	{
		bLoadResponse = true;
		uLoad = fileIO.ReadUInt8();
	}

	CSearchManager::ProcessPublishResult(uFile, uLoad, bLoadResponse);
}

// Used only by Kad2.0
void CKademliaUDPListener::Process_KADEMLIA2_PUBLISH_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP)
{
	if (!IsOnTrackList(uIP, KADEMLIA2_PUBLISH_KEY_REQ) && !IsOnTrackList(uIP, KADEMLIA2_PUBLISH_SOURCE_REQ) && !IsOnTrackList(uIP, KADEMLIA2_PUBLISH_NOTES_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	CUInt128 uFile;
	fileIO.ReadUInt128(&uFile);
	uint8 uLoad = fileIO.ReadUInt8();
	CSearchManager::ProcessPublishResult(uFile, uLoad, true);
}

// Used only by Kad1.0
void CKademliaUDPListener::Process_KADEMLIA_SEARCH_NOTES_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket < 32)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	CSafeMemFile fileIO( pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	//	This info is currently not used.
	//    CUInt128 uSource;
	//    fileIO.ReadUInt128(&uSource);

	CKademlia::GetIndexed()->SendValidNoteResult(uTarget, uIP, uUDPPort, false, 0);
}

// Used only by Kad2.0
void CKademliaUDPListener::Process_KADEMLIA2_SEARCH_NOTES_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	fileIO.ReadUInt128(&uTarget);
	uint64 uFileSize = fileIO.ReadUInt64();
	CKademlia::GetIndexed()->SendValidNoteResult(uTarget, uIP, uUDPPort, true, uFileSize);
}

// Used only by Kad1.0
void CKademliaUDPListener::Process_KADEMLIA_SEARCH_NOTES_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP)
{
	// Verify packet is expected size
	if (uLenPacket < 37)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	else if (!IsOnTrackList(uIP, KADEMLIA_SEARCH_NOTES_REQ, true)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	// What search does this relate to
	CByteIO byteIO(pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	byteIO.ReadUInt128(&uTarget);

	uint16 uCount = byteIO.ReadUInt16();
	CUInt128 uAnswer;
	while( uCount > 0 )
	{
		// What is the answer
		byteIO.ReadUInt128(&uAnswer);

		// Get info about answer
		// NOTE: this is the one and only place in Kad where we allow string conversion to local code page in
		// case we did not receive an UTF8 string. this is for backward compatibility for search results which are
		// supposed to be 'viewed' by user only and not feed into the Kad engine again!
		// If that tag list is once used for something else than for viewing, special care has to be taken for any
		// string conversion!
		TagList* pTags = new TagList;
		try
		{
			byteIO.ReadTagList(pTags, true);
		}
		catch(...)
		{
			deleteTagListEntries(pTags);
			delete pTags;
			pTags = NULL;
			throw;
		}
		CSearchManager::ProcessResult(uTarget, uAnswer, pTags);
		uCount--;
	}
}

// Used only by Kad1.0
void CKademliaUDPListener::Process_KADEMLIA_PUBLISH_NOTES_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket < 37)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	if( CKademlia::GetPrefs()->GetFirewalled() )
		//We are firewalled. We should not index this entry and give publisher a false report.
		return;

	CByteIO byteIO(pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	byteIO.ReadUInt128(&uTarget);

	CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
	uDistance.Xor(uTarget);

	// Shouldn't LAN IPs already be filtered?
	if( thePrefs.FilterLANIPs() && uDistance.Get32BitChunk(0) > SEARCHTOLERANCE)
		return;

	CUInt128 uSource;
	byteIO.ReadUInt128(&uSource);

	Kademlia::CEntry* pEntry = new Kademlia::CEntry();
	try
	{
		pEntry->m_uIP = uIP;
		pEntry->m_uUDPPort = uUDPPort;
		pEntry->m_uKeyID.SetValue(uTarget);
		pEntry->m_uSourceID.SetValue(uSource);
		byteIO.ReadTagList(&pEntry->m_listTag);
		pEntry->m_bSource = false;
	}
	catch(...)
	{
		delete pEntry;
		pEntry = NULL;
		throw;
	}

	if( pEntry == NULL )
	{
		throw CString(_T("CKademliaUDPListener::Process_KADEMLIA_PUBLISH_NOTES_REQ: pEntry == NULL"));
	}
	else if( pEntry->m_listTag.size() == 0 || pEntry->m_listTag.size() > 5)
	{
		delete pEntry;
		throw CString(_T("CKademliaUDPListener::Process_KADEMLIA_PUBLISH_NOTES_REQ: pEntry->m_listTag.size() == 0 || pEntry->m_listTag.size() > 5"));
	}

	uint8 uLoad = 0;
	if( CKademlia::GetIndexed()->AddNotes(uTarget, uSource, pEntry, uLoad ) )
	{
		CSafeMemFile fileIO2(17);
		fileIO2.WriteUInt128(&uTarget);
		fileIO2.WriteUInt8(uLoad);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA_PUBLISH_NOTES_RES", uIP, uUDPPort);

		SendPacket( &fileIO2, KADEMLIA_PUBLISH_NOTES_RES, uIP, uUDPPort);
	}
	else
		delete pEntry;
}

// Used only by Kad2.0
void CKademliaUDPListener::Process_KADEMLIA2_PUBLISH_NOTES_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	if( CKademlia::GetPrefs()->GetFirewalled() )
		//We are firewalled. We should not index this entry and give publisher a false report.
		return;

	CByteIO byteIO(pbyPacketData, uLenPacket);
	CUInt128 uTarget;
	byteIO.ReadUInt128(&uTarget);

	CUInt128 uDistance(CKademlia::GetPrefs()->GetKadID());
	uDistance.Xor(uTarget);

	// Shouldn't LAN IPs already be filtered?
	if( thePrefs.FilterLANIPs() && uDistance.Get32BitChunk(0) > SEARCHTOLERANCE)
		return;

	CUInt128 uSource;
	byteIO.ReadUInt128(&uSource);

	Kademlia::CEntry* pEntry = new Kademlia::CEntry();
	try
	{
		pEntry->m_uIP = uIP;
		pEntry->m_uUDPPort = uUDPPort;
		pEntry->m_uKeyID.SetValue(uTarget);
		pEntry->m_uSourceID.SetValue(uSource);
		pEntry->m_bSource = false;
		uint32 uTags = byteIO.ReadByte();
		while(uTags > 0)
		{
			CKadTag* pTag = byteIO.ReadTag();
			if(pTag)
			{
				if (!pTag->m_name.Compare(TAG_FILENAME))
				{
					if ( pEntry->m_fileName.IsEmpty() )
					{
						pEntry->m_fileName = pTag->GetStr();
						KadTagStrMakeLower(pEntry->m_fileName); // make lowercase, the search code expects lower case strings!
						// NOTE: always add the 'name' tag, even if it's stored separately in 'fileName'. the tag is still needed for answering search request
						pEntry->m_listTag.push_back(pTag);
					}
					else
					{
						//More then one Name tag found.
						delete pTag;
					}
				}
				else if (!pTag->m_name.Compare(TAG_FILESIZE))
				{
					if( pEntry->m_uSize == 0 )
					{
						pEntry->m_uSize = pTag->GetInt();
						// NOTE: always add the 'size' tag, even if it's stored separately in 'size'. the tag is still needed for answering search request
						pEntry->m_listTag.push_back(pTag);
					}
					else
					{
						//More then one size tag found
						delete pTag;
					}
				}
				else
				{
					//TODO: Filter tags
					pEntry->m_listTag.push_back(pTag);
				}
			}
			uTags--;
		}
	}
	catch(...)
	{
		delete pEntry;
		pEntry = NULL;
		throw;
	}

	uint8 uLoad = 0;
	if( CKademlia::GetIndexed()->AddNotes(uTarget, uSource, pEntry, uLoad ) )
	{
		CSafeMemFile fileIO2(17);
		fileIO2.WriteUInt128(&uTarget);
		fileIO2.WriteUInt8(uLoad);
		if (thePrefs.GetDebugClientKadUDPLevel() > 0)
			DebugSend("KADEMLIA2_PUBLISH_RES", uIP, uUDPPort);

		SendPacket( &fileIO2, KADEMLIA2_PUBLISH_RES, uIP, uUDPPort);
	}
	else
		delete pEntry;
}

// Used only by Kad1.0
void CKademliaUDPListener::Process_KADEMLIA_PUBLISH_NOTES_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP)
{
	// Verify packet is expected size
	if (uLenPacket < 16)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	else if (!IsOnTrackList(uIP, KADEMLIA_PUBLISH_NOTES_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	CUInt128 uFile;
	fileIO.ReadUInt128(&uFile);

	bool bLoadResponse = false;
	uint8 uLoad = 0;
	if( fileIO.GetLength() > fileIO.GetPosition() )
	{
		bLoadResponse = true;
		uLoad = fileIO.ReadUInt8();
	}

	CSearchManager::ProcessPublishResult(uFile, uLoad, bLoadResponse);
}

// Used by Kad1.0 and Kad2.0
void CKademliaUDPListener::Process_KADEMLIA_FIREWALLED_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket != 2)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	uint16 uTCPPort = fileIO.ReadUInt16();

	CContact contact;
	contact.SetIPAddress(uIP);
	contact.SetTCPPort(uTCPPort);
	contact.SetUDPPort(uUDPPort);
	theApp.clientlist->RequestTCP(&contact);

	// Send response
	CSafeMemFile fileIO2(4);
	fileIO2.WriteUInt32(uIP);
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA_FIREWALLED_RES", uIP, uUDPPort);

	SendPacket(&fileIO2, KADEMLIA_FIREWALLED_RES, uIP, uUDPPort);
}

// Used by Kad1.0 and Kad2.0
void CKademliaUDPListener::Process_KADEMLIA_FIREWALLED_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP)
{
	// Verify packet is expected size
	if (uLenPacket != 4)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	else if (!theApp.clientlist->IsKadFirewallCheckIP(ntohl(uIP))){
		CString strError;
		strError.Format(_T("Received unrequested firewall response packet in %hs"), __FUNCTION__);
		throw strError;		
	}

	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	uint32 uFirewalledIP = fileIO.ReadUInt32();

	//Update con state only if something changes.
	if( CKademlia::GetPrefs()->GetIPAddress() != uFirewalledIP )
	{
		CKademlia::GetPrefs()->SetIPAddress(uFirewalledIP);
		theApp.emuledlg->ShowConnectionState();
	}
	CKademlia::GetPrefs()->IncRecheckIP();
}

// Used by Kad1.0 and Kad2.0
void CKademliaUDPListener::Process_KADEMLIA_FIREWALLED_ACK_RES (uint32 uLenPacket)
{
	// Verify packet is expected size
	if (uLenPacket != 0)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	CKademlia::GetPrefs()->IncFirewalled();
}

// Used by Kad1.0 and Kad2.0
void CKademliaUDPListener::Process_KADEMLIA_FINDBUDDY_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket < 34)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	if( CKademlia::GetPrefs()->GetFirewalled() )
		//We are firewalled but somehow we still got this packet.. Don't send a response..
		return;
	else if (theApp.clientlist->GetBuddyStatus() == Connected)
		// we aready have a buddy
		return;

	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	CUInt128 BuddyID;
	fileIO.ReadUInt128(&BuddyID);
	CUInt128 userID;
	fileIO.ReadUInt128(&userID);
	uint16 uTCPPort = fileIO.ReadUInt16();

	CContact contact;
	contact.SetIPAddress(uIP);
	contact.SetTCPPort(uTCPPort);
	contact.SetUDPPort(uUDPPort);
	contact.SetClientID(userID);
	theApp.clientlist->IncomingBuddy(&contact, &BuddyID);

	CSafeMemFile fileIO2(34);
	fileIO2.WriteUInt128(&BuddyID);
	fileIO2.WriteUInt128(&CKademlia::GetPrefs()->GetClientHash());
	fileIO2.WriteUInt16(thePrefs.GetPort());
	if (thePrefs.GetDebugClientKadUDPLevel() > 0)
		DebugSend("KADEMLIA_FINDBUDDY_RES", uIP, uUDPPort);

	SendPacket(&fileIO2, KADEMLIA_FINDBUDDY_RES, uIP, uUDPPort);
}

// Used by Kad1.0 and Kad2.0
void CKademliaUDPListener::Process_KADEMLIA_FINDBUDDY_RES (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP, uint16 uUDPPort)
{
	// Verify packet is expected size
	if (uLenPacket < 34)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}
	else if (!IsOnTrackList(uIP, KADEMLIA_FINDBUDDY_REQ)){
		CString strError;
		strError.Format(_T("***NOTE: Received unrequested repsonse packet, size (%u) in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}


	CSafeMemFile fileIO(pbyPacketData, uLenPacket);
	CUInt128 uCheck;
	fileIO.ReadUInt128(&uCheck);
	uCheck.Xor(CUInt128(true));
	if(CKademlia::GetPrefs()->GetKadID() == uCheck)
	{
		CUInt128 userID;
		fileIO.ReadUInt128(&userID);
		uint16 uTCPPort = fileIO.ReadUInt16();

		CContact contact;
		contact.SetIPAddress(uIP);
		contact.SetTCPPort(uTCPPort);
		contact.SetUDPPort(uUDPPort);
		contact.SetClientID(userID);
		theApp.clientlist->RequestBuddy(&contact);
	}
}

// Used by Kad1.0 and Kad2.0
void CKademliaUDPListener::Process_KADEMLIA_CALLBACK_REQ (const byte *pbyPacketData, uint32 uLenPacket, uint32 uIP)
{
	// Verify packet is expected size
	if (uLenPacket < 34)
	{
		CString strError;
		strError.Format(_T("***NOTE: Received wrong size (%u) packet in %hs"), uLenPacket, __FUNCTION__);
		throw strError;
	}

	CUpDownClient* pBuddy = theApp.clientlist->GetBuddy();
	if( pBuddy != NULL )
	{
		CSafeMemFile fileIO(pbyPacketData, uLenPacket);
		CUInt128 uCheck;
		fileIO.ReadUInt128(&uCheck);
		//		JOHNTODO: Begin filtering bad buddy ID's..
		//		CUInt128 bud(buddy->GetBuddyID());
		CUInt128 uFile;
		fileIO.ReadUInt128(&uFile);
		uint16 uTCP = fileIO.ReadUInt16();
		CSafeMemFile fileIO2(uLenPacket+6);
		fileIO2.WriteUInt128(&uCheck);
		fileIO2.WriteUInt128(&uFile);
		fileIO2.WriteUInt32(uIP);
		fileIO2.WriteUInt16(uTCP);
		Packet* pPacket = new Packet(&fileIO2, OP_EMULEPROT, OP_CALLBACK);
		if (pBuddy->socket)
		{
			if (thePrefs.GetDebugClientKadUDPLevel() > 0 || thePrefs.GetDebugClientTCPLevel() > 0)
				DebugSend("OP_CALLBACK", pBuddy);
			theStats.AddUpDataOverheadFileRequest(pPacket->size);
			pBuddy->socket->SendPacket(pPacket);
		}
		else
			ASSERT(0);
	}
}

void CKademliaUDPListener::SendPacket(const byte *pbyData, uint32 uLenData, uint32 uDestinationHost, uint16 uDestinationPort)
{
	//This is temp.. The entire Kad code will be rewritten using CMemFile and send a Packet object directly.
	AddTrackedPacket(uDestinationHost, pbyData[1]);
	Packet* pPacket = new Packet(OP_KADEMLIAHEADER);
	pPacket->opcode = pbyData[1];
	pPacket->pBuffer = new char[uLenData+8];
	memcpy(pPacket->pBuffer, pbyData+2, uLenData-2);
	pPacket->size = uLenData-2;
	if( uLenData > 200 )
		pPacket->PackPacket();
	theStats.AddUpDataOverheadKad(pPacket->size);
	theApp.clientudp->SendPacket(pPacket, ntohl(uDestinationHost), uDestinationPort, false, NULL);  // kad doesnt supports obfuscation yet
}

void CKademliaUDPListener::SendPacket(const byte *pbyData, uint32 uLenData, byte byOpcode, uint32 uDestinationHost, uint16 uDestinationPort)
{
	AddTrackedPacket(uDestinationHost, byOpcode);
	Packet* pPacket = new Packet(OP_KADEMLIAHEADER);
	pPacket->opcode = byOpcode;
	pPacket->pBuffer = new char[uLenData];
	memcpy(pPacket->pBuffer, pbyData, uLenData);
	pPacket->size = uLenData;
	if( uLenData > 200 )
		pPacket->PackPacket();
	theStats.AddUpDataOverheadKad(pPacket->size);
	theApp.clientudp->SendPacket(pPacket, ntohl(uDestinationHost), uDestinationPort, false, NULL);  // kad doesnt supports obfuscation yet
}

void CKademliaUDPListener::SendPacket(CSafeMemFile *pbyData, byte byOpcode, uint32 uDestinationHost, uint16 uDestinationPort)
{
	AddTrackedPacket(uDestinationHost, byOpcode);
	Packet* pPacket = new Packet(pbyData, OP_KADEMLIAHEADER);
	pPacket->opcode = byOpcode;
	if( pPacket->size > 200 )
		pPacket->PackPacket();
	theStats.AddUpDataOverheadKad(pPacket->size);
	theApp.clientudp->SendPacket(pPacket, ntohl(uDestinationHost), uDestinationPort, false, NULL);  // kad doesnt supports obfuscation yet
}

void CKademliaUDPListener::AddTrackedPacket(uint32 dwIP, uint8 byOpcode){
	TrackPackets_Struct sTrack = {dwIP, ::GetTickCount(), byOpcode};
	listTrackedRequests.AddHead(sTrack);
	while (!listTrackedRequests.IsEmpty()){
		if (::GetTickCount() - listTrackedRequests.GetTail().dwInserted > SEC2MS(180))
			listTrackedRequests.RemoveTail();
		else
			break;
	}
}

bool CKademliaUDPListener::IsOnTrackList(uint32 dwIP, uint8 byOpcode, bool bDontRemove){
	for (POSITION pos = listTrackedRequests.GetHeadPosition(); pos != NULL; listTrackedRequests.GetNext(pos)){
		if (listTrackedRequests.GetAt(pos).dwIP == dwIP && listTrackedRequests.GetAt(pos).byOpcode == byOpcode && ::GetTickCount() - listTrackedRequests.GetAt(pos).dwInserted < SEC2MS(180)){
			if (!bDontRemove)
				listTrackedRequests.RemoveAt(pos);
			return true;
		}
	}
	return false;
}
