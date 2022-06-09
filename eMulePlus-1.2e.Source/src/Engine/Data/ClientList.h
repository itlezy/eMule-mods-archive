// ClientList.h: interface for the CClientList class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "Client.h"

typedef map<CClient*, bool>	ClientMap;
typedef deque<CClient*>		ClientQueue;

class CClientList : public CLoggable2
{
public:
	CClientList();
	virtual ~CClientList();

	void AddClient(CClient* pClient);
	void RemoveClient(CClient* pClient);

	CClientMule* FindMuleClient(AddrPort Addr);
	CClientMule* FindMuleClient(HashType Hash);
	bool IsValidClient(CClient* pClient);
	int GetWaitingPosition(CClientMule* pClient);

	void AddToWaitingQueue(CClient* pClient);
	bool RemoveFromWaitingQueue(CClient* pClient);

	void CheckAcceptNewClient();
	void PurgeBadClients();
	bool RemoveFromUploadQueue(CClient* pClient, EnumEndTransferSession eReason);

	void ProcessUpload();

private:
	ClientMap m_Clients;
	ClientMap m_Waiting;
	ClientQueue m_Uploading;
};
