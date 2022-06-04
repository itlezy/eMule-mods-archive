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
 
 
This work is based on the java implementation of the Kademlia protocol.
Kademlia: Peer-to-peer routing based on the XOR metric
Copyright (C) 2002  Petar Maymounkov [petar@post.harvard.edu]
http://kademlia.scs.cs.nyu.edu
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
#include "./RoutingBin.h"
#include "./Contact.h"
#include "../kademlia/Defines.h"
#include "../../Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Kademlia;

CRoutingBin::CRoutingBin()
{
	// Init delete contact flag.
	m_bDontDeleteContacts = false;
}

CRoutingBin::~CRoutingBin()
{
	try
	{
		if (!m_bDontDeleteContacts)
		{
			// Delete all contacts
			for (ContactList::const_iterator itContactList = m_listEntries.begin(); itContactList != m_listEntries.end(); ++itContactList)
				delete *itContactList;
		}
		// Remove all contact entries.
		m_listEntries.clear();
	}
	catch (...)
	{
		AddDebugLogLine(false, _T("Exception in ~CRoutingBin"));
	}
}

bool CRoutingBin::AddContact(CContact *pContact)
{
	ASSERT(pContact != NULL);
	// Check if we already have a contact with this ID in the list.
	CContact *pContactTest = GetContact(pContact->GetClientID());
	if (pContactTest == NULL)
	{
		// If not full, add to end of list
		if ( m_listEntries.size() < K)
		{
			m_listEntries.push_back(pContact);
			return true;
		}
	}
	return false;
}

void CRoutingBin::SetAlive(CContact *pContact)
{
	ASSERT(pContact != NULL);
	// Check if we already have a contact with this ID in the list.
	CContact *pContactTest = GetContact(pContact->GetClientID());
	ASSERT(pContact == pContactTest);
	if (pContactTest)
	{
		// Mark contact as being alive.
		pContactTest->UpdateType();
		// Move to the end of the list
		RemoveContact(pContactTest);
		m_listEntries.push_back(pContactTest);
	}
}

void CRoutingBin::SetTCPPort(uint32 uIP, uint16 uUDPPort, uint16 uTCPPort)
{
	// Find contact with IP/Port
	for (ContactList::iterator itContactList = m_listEntries.begin(); itContactList != m_listEntries.end(); ++itContactList)
	{
		CContact* pContact = *itContactList;
		if ((uIP == pContact->GetIPAddress()) && (uUDPPort == pContact->GetUDPPort()))
		{
			// Set TCPPort and mark as alive.
			pContact->SetTCPPort(uTCPPort);
			pContact->UpdateType();
			// Move to the end of the list
			RemoveContact(pContact);
			m_listEntries.push_back(pContact);
			break;
		}
	}
}

void CRoutingBin::RemoveContact(CContact *pContact)
{
	m_listEntries.remove(pContact);
}

CContact *CRoutingBin::GetContact(const CUInt128 &uID)
{
	// Find contact by ID.
	for (ContactList::const_iterator itContactList = m_listEntries.begin(); itContactList != m_listEntries.end(); ++itContactList)
	{
		if (uID == (*itContactList)->m_uClientID)
			return *itContactList;
	}
	return NULL;
}

UINT CRoutingBin::GetSize() const
{
	return (UINT)m_listEntries.size();
}

UINT CRoutingBin::GetRemaining() const
{
	return (UINT)K - m_listEntries.size();
}

void CRoutingBin::GetEntries(ContactList *plistResult, bool bEmptyFirst)
{
	// Clear results if requested first.
	if (bEmptyFirst)
		plistResult->clear();
	// Append all entries to the results.
	if (m_listEntries.size() > 0)
		plistResult->insert(plistResult->end(), m_listEntries.begin(), m_listEntries.end());
}

CContact *CRoutingBin::GetOldest()
{
	// All new/updated entries are appended to the back.
	if (m_listEntries.size() > 0)
		return m_listEntries.front();
	return NULL;
}

void CRoutingBin::GetClosestTo(uint32 uMaxType, const CUInt128 &uTarget, uint32 uMaxRequired, ContactMap *pmapResult, bool bEmptyFirst, bool bInUse)
{
	// Empty list if requested.
	if (bEmptyFirst)
		pmapResult->clear();

	// Return 0 since we have no entries.
	if (m_listEntries.size() == 0)
		return;

	// First put results in sort order for uTarget so we can insert them correctly.
	// We don't care about max results at this time.
	for (ContactList::const_iterator itContactList = m_listEntries.begin(); itContactList != m_listEntries.end(); ++itContactList)
	{
		if((*itContactList)->GetType() <= uMaxType)
		{
			CUInt128 uTargetDistance((*itContactList)->m_uClientID);
			uTargetDistance.Xor(uTarget);
			(*pmapResult)[uTargetDistance] = *itContactList;
			// This list will be used for an unknown time, Inc in use so it's not deleted.
			if( bInUse )
				(*itContactList)->IncUse();
		}
	}

	// Remove any extra results by least wanted first.
	while(pmapResult->size() > uMaxRequired)
	{
		// Dec in use count.
		if( bInUse )
			(--pmapResult->end())->second->DecUse();
		// remove from results
		pmapResult->erase(--pmapResult->end());
	}
	// Return result count to the caller.
	return;
}
