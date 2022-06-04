#pragma once
class Kademlia::CContact;
// BEGIN netfinity: Safe KAD - Ensure we are actually searching for these nodes
class Kademlia::CUInt128;
// END netfinity: Safe KAD

// BEGIN netfinity: Safe KAD - Ensure we are actually searching for these nodes
void _CheckForValidTarget(LPSTR func, const Kademlia::CUInt128& uTarget);
// END netfinity: Safe KAD
//>>> WiZaRd::Filter bad contacts
template <class T> class CAutoKillPointerList : public CAtlList<T>
{
public:
	virtual ~CAutoKillPointerList()
	{
		for(POSITION pos = GetHeadPosition(); pos;)
			delete GetNext(pos);
		RemoveAll();
	}
};

struct Contact_Data
{
	Contact_Data(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort)
	{
		this->uIP = uIP;
		this->uUDPPort = uUDPPort;
		this->uTCPPort = uTCPPort;
		dwTimeStamp = ::GetTickCount();
	}
	UINT	uIP;
	uint16	uUDPPort;
	uint16	uTCPPort;
	DWORD	dwTimeStamp;
};

bool KillBadContact(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort);
Contact_Data*	GetBadContact(const Kademlia::CContact* pContact);
Contact_Data*	GetBadContact(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort);
bool IsBadContact(const Kademlia::CContact* pContact);
bool IsBadContact(const UINT uIP, const uint16 uUDPPort, const uint16 uTCPPort);
//<<< WiZaRd::Filter bad contacts
