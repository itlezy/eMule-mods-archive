#include "stdafx.h"
#include "eMule.h"
#include "otherfunctions.h"
//#include "emuleDlg.h"
//#include "KademliaWnd.h"
//#include "KadContactListCtrl.h"
#include "./kademlia/kademlia/Kademlia.h"
#include "./kademlia/routing/Contact.h"
#include "./kademlia/routing/RoutingZone.h"
#include "KadHelper.h"
// BEGIN netfinity: Safe KAD - Ensure we are actually searching for these nodes
#include "./kademlia/utils/UInt128.h"
#include "./kademlia/kademlia/SearchManager.h"
// END netfinity: Safe KAD
#include "opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#pragma warning (disable: 4018)

using namespace Kademlia;

// BEGIN netfinity: Safe KAD - Ensure we are actually searching for these nodes
void _CheckForValidTarget(LPSTR func, const CUInt128& uTarget)
{
	if (!CSearchManager::AlreadySearchingFor(uTarget))
	{
		CString strError;
		strError.Format(_T("***NOTE: Received nodes we didn't search for in %hs"), func);
		throw strError;
	}
}
// END netfinity: Safe KAD
//>>> WiZaRd::Filter bad contacts
CAutoKillPointerList<Contact_Data*> lbadContacts;
bool	AddBadContact(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort)
{
	Contact_Data* cur = GetBadContact(uIP, uUDPPort, uTCPPort);
	if(cur == NULL)
	{
		lbadContacts.AddTail(new Contact_Data(uIP, uUDPPort, uTCPPort));
		return true;
	}
	else
		cur->dwTimeStamp = ::GetTickCount();
	return false;
}

//we could/should remove the contact now...
//true: killed and added bad contact
bool	KillBadContact(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort)
{
	//mark as bad
	AddBadContact(uIP, uUDPPort, uTCPPort);

	//retrieve the bad node and kill it
	//TODO!
//	CContact* pContact = CKademlia::GetRoutingZone()->GetContact(uIP, uUDPPort, uTCPPort);
//	if(pContact)
//	{
//		//is this save!?
//		CKademlia::GetRoutingZone()->RemoveContact(pContact);
//		delete pContact;
//		pContact = NULL; 
//		return true;
//	}

	return false;
}

Contact_Data* GetBadContact(const CContact* pContact)
{
	return GetBadContact(ntohl(pContact->GetIPAddress()), pContact->GetUDPPort(), pContact->GetTCPPort());
}

Contact_Data* GetBadContact(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort)
{
	for(POSITION pos = lbadContacts.GetHeadPosition(); pos;)
	{
		POSITION posLast = pos;
		Contact_Data* cur = lbadContacts.GetNext(pos);
		if(::GetTickCount() - cur->dwTimeStamp > CLIENTBANTIME)
		{
			delete cur;
			lbadContacts.RemoveAt(posLast);			
			continue;
		}
		if(cur->uIP == uIP
			&& (uUDPPort == 0 || uUDPPort == cur->uUDPPort)
			&& (uTCPPort == 0 || uTCPPort == cur->uTCPPort))
			return cur;
	}
	return NULL;
}

bool IsBadContact(const CContact* pContact)
{
	return IsBadContact(ntohl(pContact->GetIPAddress()), pContact->GetUDPPort(), pContact->GetTCPPort());
}

bool IsBadContact(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort)
{
	for(POSITION pos = lbadContacts.GetHeadPosition(); pos;)
	{
		POSITION posLast = pos;
		Contact_Data* cur = lbadContacts.GetNext(pos);
		if(::GetTickCount() - cur->dwTimeStamp > CLIENTBANTIME)
		{
			delete cur;
			lbadContacts.RemoveAt(posLast);			
			continue;
		}
		if(cur->uIP == uIP
			&& (uUDPPort == 0 || uUDPPort == cur->uUDPPort)
			&& (uTCPPort == 0 || uTCPPort == cur->uTCPPort))
			return true;
	}
	return false;
}
//<<< WiZaRd::Filter bad contacts