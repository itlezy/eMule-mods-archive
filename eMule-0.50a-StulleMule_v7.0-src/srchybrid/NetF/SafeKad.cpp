/**
 *
  */
#include "stdafx.h"
#include "SafeKad.h"
#include "Opcodes.h"

#ifdef USE_SAFEKAD
using namespace Kademlia;

namespace Kademlia
{
	CSafeKad	safeKad;
}

CSafeKad::CSafeKad()
{
	m_uLastCleanup = time(NULL);
}

CSafeKad::~CSafeKad()
{
	for (std::map<NodeAddress, Tracked*>::const_iterator it = m_mapTrackedNodes.begin(); it != m_mapTrackedNodes.end(); ++it)
		delete it->second;
	for (std::map<uint32, Banned*>::const_iterator it = m_mapBannedIPs.begin(); it != m_mapBannedIPs.end(); ++it)
		delete it->second;
}

void CSafeKad::TrackContact(CContact* contact)
{
	m_mapContactIPs[contact->GetIPAddress()] = contact;
	contact->m_bSafeKadRefs = true;
}

void CSafeKad::UntrackContact(CContact* contact)
{
	// Iterate the entire list just in case
	for (std::map<uint32, CContact*>::iterator it = m_mapContactIPs.begin(); it != m_mapContactIPs.end();)
	{
		if (it->second == contact)
			m_mapContactIPs.erase(it++);
		else
			++it;
	}
}

void CSafeKad::TrackNode(uint32 uIP, uint16 uPort, const CUInt128& uID)
{
	if (m_mapBannedIPs.find(uIP) != m_mapBannedIPs.end())
		return;
	Tracked* tracked;
	std::map<NodeAddress, Tracked*>::const_iterator it = m_mapTrackedNodes.find(NodeAddress(uIP, uPort));
	if (it == m_mapTrackedNodes.end())
	{
		if (m_mapTrackedNodes.size() >= 10000) // Don't track more than 10000 suspect nodes
			return;
		tracked = new Tracked;
		tracked->m_uLastID = uID;
		tracked->m_uLastIDChange = time(NULL);
	}
	else
		tracked = it->second;
	if (uID != tracked->m_uLastID)
	{
		if (time(NULL) - tracked->m_uLastIDChange < HR2S(1)) // If change occurs more often than once an hour, ban this IP
		{
			BanIP(uIP);
			m_mapTrackedNodes.erase(NodeAddress(uIP, uPort));
			delete tracked;
			return;
		}
		tracked->m_uLastID = uID;
		tracked->m_uLastIDChange = time(NULL);
	}
	tracked->m_uLastReferenced = time(NULL);
	m_mapTrackedNodes[NodeAddress(uIP, uPort)] = tracked;
}

// NOTE!!! 
//  Use this functon with great caution! 
//  Only use if you can with great certanity tell that the IP is used by a bad node.
//  Make sure you haven't been decived by another node to think this IP is bad!
void CSafeKad::BanIP(uint32 uIP)
{
	Banned* banned;
	std::map<uint32, Banned*>::const_iterator it = m_mapBannedIPs.find(uIP);
	if (it == m_mapBannedIPs.end())
	{
		if (m_mapBannedIPs.size() >= 1000) // Don't ban more than 1000 IPs (if we reach this we are probably in deep shit!)
			return;
		banned = new Banned;
	}
	else
		banned = it->second;
	banned->m_uBanned = time(NULL);
	banned->m_uLastReferenced = time(NULL);
	m_mapBannedIPs[uIP] = banned;
}

bool CSafeKad::IsBadNode(uint32 uIP, uint16 uPort, const CUInt128& uID, bool isNodeResponse)
{
	if (time(NULL) - m_uLastCleanup > MIN2S(10))
		Cleanup();

	std::map<uint32, Banned*>::iterator itBanned = m_mapBannedIPs.find(uIP);
	std::map<NodeAddress, Tracked*>::iterator itTracked = m_mapTrackedNodes.find(NodeAddress(uIP, uPort));
	if (itBanned != m_mapBannedIPs.end())
	{
		itBanned->second->m_uLastReferenced = time(NULL);
		if (time(NULL) - itBanned->second->m_uBanned > HR2S(4)) // Max ban time is 4 hours
		{
			delete itBanned->second;
			m_mapBannedIPs.erase(itBanned);
		}
		else
		{
			if (itTracked != m_mapTrackedNodes.end())
			{
				delete itTracked->second;
				m_mapTrackedNodes.erase(itTracked);
			}
			return true;
		}
	}
	if (itTracked != m_mapTrackedNodes.end())
	{
		itTracked->second->m_uLastReferenced = time(NULL);
		if (itTracked->second->m_uLastID != uID)
		{
			if (isNodeResponse)
				TrackNode(uIP, uPort, uID); // Update ID and ban if appropriate
			return true;
		}
	} 
	return false;
}

bool CSafeKad::IsBanned(uint32 uIP)
{
	std::map<uint32, Banned*>::iterator itBanned = m_mapBannedIPs.find(uIP);
	if (itBanned != m_mapBannedIPs.end())
	{
		itBanned->second->m_uLastReferenced = time(NULL);
		if (time(NULL) - itBanned->second->m_uBanned > HR2S(4)) // Max ban time is 4 hours
		{
			delete itBanned->second;
			m_mapBannedIPs.erase(itBanned);
		}
		else
			return true;
	}
	return false;
}

bool CSafeKad::IsIPInUse(uint32 uIP)
{
	if (m_mapContactIPs.count(uIP) > 0)
		return true;
	return false;
}

void CSafeKad::Cleanup()
{
	for (std::map<NodeAddress, Tracked*>::iterator it = m_mapTrackedNodes.begin(); it != m_mapTrackedNodes.end();)
	{
		if (time(NULL) - it->second->m_uLastReferenced > HR2S(1))
		{
			delete it->second;
			m_mapTrackedNodes.erase(it++);
		}
		else
			++it;
	}
	for (std::map<uint32, Banned*>::iterator it = m_mapBannedIPs.begin(); it != m_mapBannedIPs.end();)
	{
		if (time(NULL) - it->second->m_uLastReferenced > HR2S(1))
		{
			delete it->second;
			m_mapBannedIPs.erase(it++);
		}
		else
			++it;
	}
}
#endif
