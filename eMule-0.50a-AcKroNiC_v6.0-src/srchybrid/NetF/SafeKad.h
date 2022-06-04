/**
 *
  */
#ifndef	SAFEKAD_H
#define	SAFEKAD_H

#include "Kademlia/routing/Contact.h"
#include "Kademlia/routing/Maps.h"

#ifdef USE_SAFEKAD
namespace Kademlia
{
	class CSafeKad
	{
	private:
		class NodeAddress
		{
		public:
			NodeAddress(uint32 uIP, uint16 uPort) : m_uIP(uIP), m_uPort(uPort) {}
			bool operator< (const NodeAddress &value) const {return (m_uIP < value.m_uIP || (m_uIP == value.m_uIP && m_uPort < value.m_uPort));}
			bool operator<= (const NodeAddress &value) const {return (m_uIP < value.m_uIP || (m_uIP == value.m_uIP && m_uPort <= value.m_uPort));}
			bool operator> (const NodeAddress &value) const {return (m_uIP > value.m_uIP || (m_uIP == value.m_uIP && m_uPort > value.m_uPort));}
			bool operator>= (const NodeAddress &value) const {return (m_uIP > value.m_uIP || (m_uIP == value.m_uIP && m_uPort >= value.m_uPort));}
			bool operator== (const NodeAddress &value) const {return (m_uIP == value.m_uIP && m_uPort == value.m_uPort);}
			bool operator!= (const NodeAddress &value) const {return (m_uIP != value.m_uIP || m_uPort != value.m_uPort);}
		private:
			uint32 m_uIP;
			uint16 m_uPort;
		};		
		struct Tracked
		{
			CUInt128	m_uLastID;
			time_t	m_uLastIDChange;
			time_t	m_uLastReferenced;
		};
		struct Banned
		{
			time_t	m_uBanned;
			time_t	m_uLastReferenced;
		};
	
	public:
		CSafeKad();
		~CSafeKad();
		void TrackContact(CContact* contact);
		void UntrackContact(CContact* contact);
		void TrackNode(uint32 uIP, uint16 uPort, const CUInt128& uID);
		void BanIP(uint32 uIP);
		bool IsBadNode(uint32 uIP, uint16 uPort, const CUInt128& uID, bool isNodeResponse = false);
		bool IsBanned(uint32 uIP);
		bool IsIPInUse(uint32 uIP);

	private:
		void Cleanup();
		std::map<NodeAddress, Tracked*> m_mapTrackedNodes;
		std::map<uint32, Banned*> m_mapBannedIPs;
		std::map<uint32, CContact*> m_mapContactIPs;
		time_t m_uLastCleanup;
	};

	extern CSafeKad safeKad;
}
#endif	//SAFEKAD_H
#endif //USE_SAFEKAD
