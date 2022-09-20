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

/**
 * The *Zone* is just a node in a binary tree of *Zone*s.
 * Each zone is either an internal node or a leaf node.
 * Internal nodes have "bin == null" and "subZones[i] != null",
 * leaf nodes have "subZones[i] == null" and "bin != null".
 * 
 * All key unique id's are relative to the center (self), which
 * is considered to be 000..000
 */

#include "stdafx.h"
#include <math.h>
#include "./RoutingZone.h"
#include "./RoutingBin.h"
#include "../utils/MiscUtils.h"
#include "../kademlia/Kademlia.h"
#include "../kademlia/Prefs.h"
#include "../kademlia/SearchManager.h"
#include "../kademlia/Defines.h"
#include "../net/KademliaUDPListener.h"
#include "../../Opcodes.h"
#include "../../emule.h"
#include "../../emuledlg.h"
#include "../../KadContactListCtrl.h"
#include "../../kademliawnd.h"
#include "../../SafeFile.h"
#include "../../Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Kademlia;

void DebugSend(LPCTSTR pszMsg, uint32 uIP, uint16 uUDPPort);

CString CRoutingZone::m_sFilename;
CUInt128 CRoutingZone::uMe = (ULONG)0;

CRoutingZone::CRoutingZone()
{
	// Can only create routing zone after prefs
	// Set our KadID for creating the contact tree
	CKademlia::GetPrefs()->GetKadID(&uMe);
	// Set the preference file name.
	m_sFilename = CMiscUtils::GetAppDir();
	m_sFilename.Append(CONFIGFOLDER);
	m_sFilename.Append(_T("nodes.dat"));
	// Init our root node.
	Init(NULL, 0, CUInt128((ULONG)0));
}

CRoutingZone::CRoutingZone(LPCSTR szFilename)
{
	// Can only create routing zone after prefs
	// Set our KadID for creating the contact tree
	CKademlia::GetPrefs()->GetKadID(&uMe);
	m_sFilename = szFilename;
	// Init our root node.
	Init(NULL, 0, CUInt128((ULONG)0));
}

CRoutingZone::CRoutingZone(CRoutingZone *pSuper_zone, int iLevel, const CUInt128 &uZone_index)
{
	// Create a new leaf.
	Init(pSuper_zone, iLevel, uZone_index);
}

void CRoutingZone::Init(CRoutingZone *pSuper_zone, int iLevel, const CUInt128 &uZone_index)
{
	// Init all Zone vars
	// Set this zones parent
	m_pSuperZone = pSuper_zone;
	// Set this zones level
	m_uLevel = iLevel;
	// Set this zones CUInt128 Index
	m_uZoneIndex = uZone_index;
	// Mark this zone has having now leafs.
	m_pSubZones[0] = NULL;
	m_pSubZones[1] = NULL;
	// Create a new contact bin as this is a leaf.
	m_pBin = new CRoutingBin();

	// Set timer so that zones closer to the root are processed earlier.
	m_tNextSmallTimer = time(NULL) + m_uZoneIndex.Get32BitChunk(3);

	// Start this zone.
	StartTimer();

	// If we are initializing the root node, read in our saved contact list.
	if ((m_pSuperZone == NULL) && (m_sFilename.GetLength() > 0))
		ReadFile();
}

CRoutingZone::~CRoutingZone()
{
	// Root node is processed first so that we can write our contact list and delete all branches.
	if ((m_pSuperZone == NULL) && (m_sFilename.GetLength() > 0))
	{
		// Hide contacts in the GUI
		theApp.emuledlg->kademliawnd->HideContacts();
		WriteFile();
	}
	// If this zone is a leaf, delete our contact bin.
	if (IsLeaf())
		delete m_pBin;
	else
	{
		// If this zone is branch, delete it's leafs.
		delete m_pSubZones[0];
		delete m_pSubZones[1];
	}
	// All branches are deleted, show the contact list in the GUI.
	if (m_pSuperZone == NULL)
		theApp.emuledlg->kademliawnd->ShowContacts();
}

void CRoutingZone::ReadFile()
{
	// Read in the saved contact list.
	try
	{
		// Hide contact list in the GUI
		theApp.emuledlg->kademliawnd->HideContacts();
		CSafeBufferedFile file;
		CFileException fexp;
		if (file.Open(m_sFilename, CFile::modeRead | CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp))
		{
			setvbuf(file.m_pStream, NULL, _IOFBF, 32768);

			// Get how many contacts in the saved list.
			// NOTE: Older clients put the number of contacts here..
			//       Newer clients always have 0 here to prevent older clients from reading it.
			uint32 uNumContacts = file.ReadUInt32();
			uint32 uVersion = 0;
			if (uNumContacts == 0)
			{
				try
				{
					uVersion = file.ReadUInt32();
					if(uVersion == 1)
						uNumContacts = file.ReadUInt32();
				}
				catch(...)
				{
					AddDebugLogLine( false, GetResString(IDS_ERR_KADCONTACTS));
				}
			}
			if (uNumContacts != 0)
			{
				uint32 uValidContacts = 0;
				CUInt128 uID;
				while ( uNumContacts )
				{
					file.ReadUInt128(&uID);
					uint32 uIP = file.ReadUInt32();
					uint16 uUDPPort = file.ReadUInt16();
					uint16 uTCPPort = file.ReadUInt16();
					uint8 uContactVersion = 0;
					byte byType = 0;
					if(uVersion == 1)
						uContactVersion = file.ReadUInt8();
					else
						byType = file.ReadUInt8();
					// IP Appears valid
					if( byType < 4)
					{
						// This was not a dead contact, Inc counter if add was successful
						if( Add(uID, uIP, uUDPPort, uTCPPort, uContactVersion) )
							uValidContacts++;
					}
					uNumContacts--;
				}
				AddLogLine( false, GetResString(IDS_KADCONTACTSREAD), uValidContacts);
			}
			file.Close();
		}
	}
	catch (CFileException* e)
	{
		e->Delete();
		AddDebugLogLine(false, _T("CFileException in CRoutingZone::readFile"));
	}
	// Show contact list in GUI
	theApp.emuledlg->kademliawnd->ShowContacts();
}

void CRoutingZone::WriteFile()
{
	try
	{
		// Write a saved contact list.
		CUInt128 uID;
		CSafeBufferedFile file;
		CFileException fexp;
		if (file.Open(m_sFilename, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary|CFile::shareDenyWrite, &fexp))
		{
			setvbuf(file.m_pStream, NULL, _IOFBF, 32768);

			// The bootstrap method gets a very nice sample of contacts to save.
			ContactList listContacts;
			GetBootstrapContacts(&listContacts, 200);
			// Start file with 0 to prevent older clients from reading it.
			file.WriteUInt32(0);
			// Now tag it with a version which happens to be 1.
			file.WriteUInt32(1);
			file.WriteUInt32((uint32)listContacts.size());
			for (ContactList::const_iterator itContactList = listContacts.begin(); itContactList != listContacts.end(); ++itContactList)
			{
				CContact* pContact = *itContactList;
				pContact->GetClientID(&uID);
				file.WriteUInt128(&uID);
				file.WriteUInt32(pContact->GetIPAddress());
				file.WriteUInt16(pContact->GetUDPPort());
				file.WriteUInt16(pContact->GetTCPPort());
				file.WriteUInt8(pContact->GetVersion());
			}
			file.Close();
			AddDebugLogLine( false, _T("Wrote %ld contact%s to file."), listContacts.size(), ((listContacts.size() == 1) ? _T("") : _T("s")));
		}
	}
	catch (CFileException* e)
	{
		e->Delete();
		AddDebugLogLine(false, _T("CFileException in CRoutingZone::writeFile"));
	}
}

bool CRoutingZone::CanSplit() const
{
	// Max levels allowed.
	if (m_uLevel >= 127)
		return false;

	// Check if this zone is allowed to split.
	if ( (m_uZoneIndex < KK || m_uLevel < KBASE) && m_pBin->GetSize() == K)
		return true;
	return false;
}

bool CRoutingZone::Add(const CUInt128 &uID, uint32 uIP, uint16 uUDPPort, uint16 uTCPPort, uint8 uVersion )
{
	if(::IsGoodIPPort(ntohl(uIP), uUDPPort))
	{
		if(uID != uMe)
		{
			// JOHNTODO -- How do these end up leaking at times?
			CContact* pContact = new CContact(uID, uIP, uUDPPort, uTCPPort, uVersion);
			if(Add(pContact))
				return true;
			delete pContact;
		}
	}
	return false;
}

bool CRoutingZone::Add(CContact* pContact)
{
	// If we are not a leaf, call add on the correct branch.
	if (!IsLeaf())
		return m_pSubZones[pContact->GetDistance().GetBitNumber(m_uLevel)]->Add(pContact);
	else
	{
		// Do we already have a contact with this KadID?
		if (m_pBin->GetContact(pContact->GetClientID()))
			return false;
		else if (m_pBin->GetRemaining())
		{
			// This bin is not full, so add the new contact.
			if(m_pBin->AddContact(pContact))
			{
				// Add was successful, add to the GUI and let contact know it's listed in the gui.
				if (theApp.emuledlg->kademliawnd->ContactAdd(pContact))
					pContact->SetGuiRefs(true);
				return true;
			}
			return false;
		}
		else if (CanSplit())
		{
			// This bin was full and split, call add on the correct branch.
			Split();
			return m_pSubZones[pContact->GetDistance().GetBitNumber(m_uLevel)]->Add(pContact);
		}
		else
			return false;
	}
}

void CRoutingZone::SetAlive(uint32 uIP, uint16 uUDPPort)
{
	if (IsLeaf())
		m_pBin->SetAlive(uIP, uUDPPort);
	else
	{
		m_pSubZones[0]->SetAlive(uIP, uUDPPort);
		m_pSubZones[1]->SetAlive(uIP, uUDPPort);
	}
}

CContact *CRoutingZone::GetContact(const CUInt128 &uID) const
{
	if (IsLeaf())
		return m_pBin->GetContact(uID);
	else
		return m_pSubZones[uID.GetBitNumber(m_uLevel)]->GetContact(uID);
}

//Xman official patch
void CRoutingZone::GetClosestTo(uint32 uMaxType, const CUInt128 &uTarget, const CUInt128 &uDistance, uint32 uMaxRequired, ContactMap *pmapResult, bool bEmptyFirst, bool bInUse) const
{
	// If leaf zone, do it here
	if (IsLeaf())
	{
		m_pBin->GetClosestTo(uMaxType, uTarget, uMaxRequired, pmapResult, bEmptyFirst, bInUse);
		return;
	}

	// otherwise, recurse in the closer-to-the-target subzone first
	int iCloser = uDistance.GetBitNumber(m_uLevel);
	m_pSubZones[iCloser]->GetClosestTo(uMaxType, uTarget, uDistance, uMaxRequired, pmapResult, bEmptyFirst, bInUse);

	// if still not enough tokens found, recurse in the other subzone too
	if (pmapResult->size() < uMaxRequired)
		m_pSubZones[1-iCloser]->GetClosestTo(uMaxType, uTarget, uDistance, uMaxRequired, pmapResult, false, bInUse);
}

void CRoutingZone::GetAllEntries(ContactList *pmapResult, bool bEmptyFirst)
{
	if (IsLeaf())
		m_pBin->GetEntries(pmapResult, bEmptyFirst);
	else
	{
		m_pSubZones[0]->GetAllEntries(pmapResult, bEmptyFirst);
		m_pSubZones[1]->GetAllEntries(pmapResult, false);
	}
}

void CRoutingZone::TopDepth(int iDepth, ContactList *pmapResult, bool bEmptyFirst)
{
	if (IsLeaf())
		m_pBin->GetEntries(pmapResult, bEmptyFirst);
	else if (iDepth <= 0)
		RandomBin(pmapResult, bEmptyFirst);
	else
	{
		m_pSubZones[0]->TopDepth(iDepth-1, pmapResult, bEmptyFirst);
		m_pSubZones[1]->TopDepth(iDepth-1, pmapResult, false);
	}
}

void CRoutingZone::RandomBin(ContactList *pmapResult, bool bEmptyFirst)
{
	if (IsLeaf())
		m_pBin->GetEntries(pmapResult, bEmptyFirst);
	else
		m_pSubZones[rand()&1]->RandomBin(pmapResult, bEmptyFirst);
}

uint32 CRoutingZone::GetMaxDepth() const
{
	if (IsLeaf())
		return 0;
	return 1 + max(m_pSubZones[0]->GetMaxDepth(), m_pSubZones[1]->GetMaxDepth());
}

void CRoutingZone::Split()
{
	StopTimer();

	m_pSubZones[0] = GenSubZone(0);
	m_pSubZones[1] = GenSubZone(1);

	ContactList listEntries;
	m_pBin->GetEntries(&listEntries);
	for (ContactList::const_iterator itContactList = listEntries.begin(); itContactList != listEntries.end(); ++itContactList)
	{
		int iSuperZone = (*itContactList)->m_uDistance.GetBitNumber(m_uLevel);
		m_pSubZones[iSuperZone]->m_pBin->AddContact(*itContactList);
	}
	m_pBin->m_bDontDeleteContacts = true;
	delete m_pBin;
	m_pBin = NULL;
}

uint32 CRoutingZone::Consolidate()
{
	uint32 uMergeCount = 0;
	if( IsLeaf() )
		return uMergeCount;
	ASSERT(m_pBin==NULL);
	if( !m_pSubZones[0]->IsLeaf() )
		uMergeCount += m_pSubZones[0]->Consolidate();
	if( !m_pSubZones[1]->IsLeaf() )
		uMergeCount += m_pSubZones[1]->Consolidate();
	if( m_pSubZones[0]->IsLeaf() && m_pSubZones[1]->IsLeaf() && GetNumContacts() < K/2 )
	{
		m_pBin = new CRoutingBin();
		m_pSubZones[0]->StopTimer();
		m_pSubZones[1]->StopTimer();
		if (GetNumContacts() > 0)
		{
			ContactList list0;
			ContactList list1;
			m_pSubZones[0]->m_pBin->GetEntries(&list0);
			m_pSubZones[1]->m_pBin->GetEntries(&list1);
			for (ContactList::const_iterator itContactList = list0.begin(); itContactList != list0.end(); ++itContactList)
				m_pBin->AddContact(*itContactList);
			for (ContactList::const_iterator itContactList = list1.begin(); itContactList != list1.end(); ++itContactList)
				m_pBin->AddContact(*itContactList);
		}
		m_pSubZones[0]->m_pSuperZone = NULL;
		m_pSubZones[1]->m_pSuperZone = NULL;
		m_pSubZones[0]->m_pBin->m_bDontDeleteContacts = true;
		m_pSubZones[1]->m_pBin->m_bDontDeleteContacts = true;
		delete m_pSubZones[0];
		delete m_pSubZones[1];
		m_pSubZones[0] = NULL;
		m_pSubZones[1] = NULL;
		StartTimer();
		uMergeCount++;
	}
	return uMergeCount;
}

bool CRoutingZone::IsLeaf() const
{
	return (m_pBin != NULL);
}

CRoutingZone *CRoutingZone::GenSubZone(int iSide)
{
	CUInt128 uNewIndex(m_uZoneIndex);
	uNewIndex.ShiftLeft(1);
	if (iSide != 0)
		uNewIndex.Add(1);
	return new CRoutingZone(this, m_uLevel+1, uNewIndex);
}

void CRoutingZone::StartTimer()
{
	time_t tNow = time(NULL);
	// Start filling the tree, closest bins first.
	m_tNextBigTimer = tNow + SEC(10);
	CKademlia::AddEvent(this);
}

void CRoutingZone::StopTimer()
{
	CKademlia::RemoveEvent(this);
}

bool CRoutingZone::OnBigTimer()
{
	if ( IsLeaf() && (m_uZoneIndex < KK || m_uLevel < KBASE || m_pBin->GetRemaining() >= (K*.8)))
	{
		RandomLookup();
		return true;
	}

	return false;
}

//This is used when we find a leaf and want to know what this sample looks like.
//We fall back two levels and take a sample to try to minimize any areas of the
//tree that will give very bad results.
uint32 CRoutingZone::EstimateCount()
{
	if( !IsLeaf() )
		return 0;
	if( m_uLevel < KBASE )
		return (UINT)(pow(2, m_uLevel)*K);
	CRoutingZone* pCurZone = m_pSuperZone->m_pSuperZone->m_pSuperZone;
	// Find out how full this part of the tree is.
	float fModify = ((float)pCurZone->GetNumContacts())/(float)(K*2);
	// First calculate users assuming the tree is full.
	// Modify count by bin size.
	// Modify count by how full the tree is.
	// Modify count by assuming 20% of the users are firewalled and can't be a contact.
	return (UINT)((pow( 2, m_uLevel-2))*(float)K*fModify*(1.20F));
}

void CRoutingZone::OnSmallTimer()
{
	if (!IsLeaf())
		return;

	CContact *pContact = NULL;
	time_t tNow = time(NULL);
	ContactList listEntries;
	// Remove dead entries
	m_pBin->GetEntries(&listEntries);
	for (ContactList::iterator itContactList = listEntries.begin(); itContactList != listEntries.end(); ++itContactList)
	{
		pContact = *itContactList;
		if ( pContact->GetType() == 4)
		{
			if (((pContact->m_tExpires > 0) && (pContact->m_tExpires <= tNow)))
			{
				if(!pContact->InUse())
				{
					m_pBin->RemoveContact(pContact);
					delete pContact;
				}
				continue;
			}
		}
		if(pContact->m_tExpires == 0)
			pContact->m_tExpires = tNow;
	}
	pContact = m_pBin->GetOldest();
	if( pContact != NULL )
	{
		if ( pContact->m_tExpires >= tNow || pContact->GetType() == 4)
		{
			m_pBin->RemoveContact(pContact);
			m_pBin->m_listEntries.push_back(pContact);
			pContact = NULL;
		}
	}
	if(pContact != NULL)
	{
		pContact->CheckingType();
		if(pContact->GetVersion() != 0)
		{
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugSend("KADEMLIA2_HELLO_REQ", pContact->GetIPAddress(), pContact->GetUDPPort());
			CKademlia::GetUDPListener()->SendMyDetails_KADEMLIA2(KADEMLIA2_HELLO_REQ, pContact->GetIPAddress(), pContact->GetUDPPort());
		}
		else
		{
			if (thePrefs.GetDebugClientKadUDPLevel() > 0)
				DebugSend("KADEMLIA_HELLO_REQ", pContact->GetIPAddress(), pContact->GetUDPPort());
			CKademlia::GetUDPListener()->SendMyDetails(KADEMLIA_HELLO_REQ, pContact->GetIPAddress(), pContact->GetUDPPort());
		}
	}
}

void CRoutingZone::RandomLookup()
{
	// Look-up a random client in this zone
	CUInt128 uPrefix(m_uZoneIndex);
	uPrefix.ShiftLeft(128 - m_uLevel);
	CUInt128 uRandom(uPrefix, m_uLevel);
	uRandom.Xor(uMe);
	CSearchManager::FindNode(uRandom, false);
}

uint32 CRoutingZone::GetNumContacts() const
{
	if (IsLeaf())
		return m_pBin->GetSize();
	else
		return m_pSubZones[0]->GetNumContacts() + m_pSubZones[1]->GetNumContacts();
}

uint32 CRoutingZone::GetBootstrapContacts(ContactList *plistResult, uint32 uMaxRequired)
{
	ASSERT(m_pSuperZone == NULL);
	plistResult->clear();
	uint32 uRetVal = 0;
	try
	{
		ContactList top;
		TopDepth(LOG_BASE_EXPONENT, &top);
		if (top.size() > 0)
		{
			for (ContactList::const_iterator itContactList = top.begin(); itContactList != top.end(); ++itContactList)
			{
				plistResult->push_back(*itContactList);
				uRetVal++;
				if (uRetVal == uMaxRequired)
					break;
			}
		}
	}
	catch (...)
	{
		AddDebugLogLine(false, _T("Exception in CRoutingZone::getBoostStrapContacts"));
	}
	return uRetVal;
}
