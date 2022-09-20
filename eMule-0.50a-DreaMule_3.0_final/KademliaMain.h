#pragma once
#include "loggable.h"

struct Status;

class CKademliaMain : public CLoggable
{
	friend UINT AFX_CDECL KadStopFunc(LPVOID pvParams);
public:
	CKademliaMain(void);
	~CKademliaMain(void);

	void	setStatus(Status* val);
	Status*	getStatus();
	bool	isConnected();
	bool	isFirewalled();
	void	Connect();
	void	DisConnect();
	DWORD	GetThreadID();
	void Bootstrap(CString ip,uint16 port);
	void Bootstrap(uint32 ip,uint16 port);

	uint32	getIP();
	uint16	getUdpPort();
	uint16	getTcpPort();
private:
	//Most likely delete these and only keep the Status Object..
	Status* m_status;
	time_t m_bootstrapTimer;
};
