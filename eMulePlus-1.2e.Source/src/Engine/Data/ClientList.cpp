// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientList.h"
#include "Prefs.h"
#include "../Sockets/OpCode.h"
#include "../Sockets/TcpEngineMule.h"

//////////////////////////////////////////////////////////////////////
// ClientList
CClientList::CClientList()
{
}

//////////////////////////////////////////////////////////////////////
CClientList::~CClientList()
{
	for(ClientMap::iterator it = m_Clients.begin(); it != m_Clients.end(); it++)
	{
		CClient* pClient = it->first;
		if(pClient)
			delete pClient;
	}
	m_Clients.erase(m_Clients.begin(), m_Clients.end());
}

//////////////////////////////////////////////////////////////////////
void CClientList::AddClient(CClient* pClient)
{
	m_Clients[pClient] = true;
}

//////////////////////////////////////////////////////////////////////
void CClientList::RemoveClient(CClient* pClient)
{
}

//////////////////////////////////////////////////////////////////////
CClientMule* CClientList::FindMuleClient(AddrPort Addr)
{
	for(ClientMap::iterator it = m_Clients.begin(); it != m_Clients.end(); it++)
	{
		CClient* pClient = it->first;
		if(pClient && pClient->Type == CLIENT_MULE)
		{
			if(*pClient == Addr)
				return reinterpret_cast<CClientMule*>(pClient);
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////
CClientMule* CClientList::FindMuleClient(HashType Hash)
{
	for(ClientMap::iterator it = m_Clients.begin(); it != m_Clients.end(); it++)
	{
		CClient* pClient = it->first;
		if(pClient && pClient->Type == CLIENT_MULE)
		{
			CClientMule* pMule = reinterpret_cast<CClientMule*>(pClient);
			if(*pMule == Hash)
				return pMule;
		}
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////
bool CClientList::IsValidClient(CClient* pClient)
{
	return (m_Clients.find(pClient) != m_Clients.end());
}

//////////////////////////////////////////////////////////////////////
void CClientList::AddToWaitingQueue(CClient* pClient)
{
	if(pClient && pClient->Type == CLIENT_MULE)
	{
		CClientMule* pMule = reinterpret_cast<CClientMule*>(pClient);

		// Filtering invalid eMule clients
		if(pMule->MuleVersion == 0 && pMule->ClientVersion == 0 &&
			(pMule->ClientSoft == SO_EMULE || pMule->ClientSoft == SO_PLUS || pMule->ClientSoft == SO_OLDEMULE))
		{
			AddLog(LOG_DEBUG, _T("Client (%s:%d) was filtered as invalid"), inet_ntoa(pMule->ClientAddr), pMule->ClientPort);
			return;
		}

		// If we're already uploading to that client, it probably asks for another file
		if(pMule->UploadState == US_UPLOADING && pMule->Parent)
		{
			COpCode_ACCEPTUPLOADREQ stMsg;
			g_stEngine.SendOpCode(pMule->Parent->m_hSocket, stMsg, pMule->Parent, QUE_HIGH);
			AddLog(LOG_DEBUG, _T("Client (%s:%d) is already downloading from us, accept request"), inet_ntoa(pMule->ClientAddr), pMule->ClientPort);
			return;
		}

		// Add to queue
		m_Waiting[pClient] = true;

		// Set states
		pMule->UploadState = US_ONUPLOADQUEUE;
		pMule->WaitStartTime = CPreciseTime::GetCurrentTime();

		// Send ranking info back to client
		pMule->SendRankingInfo();

		AddLog(LOG_DEBUG, _T("Client (%s:%d) has been added to waiting queue"), inet_ntoa(pMule->ClientAddr), pMule->ClientPort);
	}
}

//////////////////////////////////////////////////////////////////////
bool CClientList::RemoveFromWaitingQueue(CClient* pClient)
{
	if(pClient == NULL)
		return false;
	ClientMap::iterator it = m_Waiting.find(pClient);
	if(it != m_Waiting.end())
	{
		m_Waiting.erase(it);

		AddLog(LOG_DEBUG, _T("Client (%s:%d) has been removed from waiting queue"), inet_ntoa(pClient->ClientAddr), pClient->ClientPort);
		return true;
	}
	else
		AddLog(LOG_ERROR, _T("Can't find client (%s:%d) to remove from waiting queue"), inet_ntoa(pClient->ClientAddr), pClient->ClientPort);
	return false;
}

//////////////////////////////////////////////////////////////////////
int CClientList::GetWaitingPosition(CClientMule* pClient)
{
	//	if (!IsOnUploadQueue(client))
	//		return 0;

	USHORT uRank = 1;

	/*	if (client->IsBanned())
	{
	uRank = waitinglist.GetCount();
	}
	else*/
	{
		int nClientScore = pClient->Score;

		for(ClientMap::iterator it = m_Waiting.begin(); it != m_Waiting.end(); it++)
		{
			CClient* pClient = it->first;
			if(pClient && pClient->Type == CLIENT_MULE)
			{
				CClientMule* pMule = reinterpret_cast<CClientMule*>(pClient);
				if(pMule->Score > nClientScore)
					uRank++;
			}
		}
	}
	return uRank;
}

//////////////////////////////////////////////////////////////////////
void CClientList::PurgeBadClients()
{
	for(ClientMap::iterator it = m_Waiting.begin(); it != m_Waiting.end();)
	{
		CClient* pClient = it->first;
		if(CPreciseTime::GetCurrentTime() - pClient->LastUploadRequest > MAX_PURGEQUEUETIME)
			//			|| !g_stEngine.SharedFiles.GetFileByID(pClient->ReqFile) )
		{
			RemoveFromWaitingQueue(pClient);
			if(pClient->Parent)
				g_stEngine.Sockets.KillClient(pClient->Parent);

			it = m_Waiting.begin();
		}
		else
			it++;
	}
}

//////////////////////////////////////////////////////////////////////
void CClientList::CheckAcceptNewClient()
{
	if (m_Waiting.size() > 0 && 
		m_Uploading.size() < MAX_UP_CLIENTS_ALLOWED &&
		m_Uploading.size() < g_stEngine.Prefs.MaxUploadSlots)
	{
		//		if (g_eMuleApp.m_pListenSocket->TooManySockets())
		//			return;

		// Find best rating client
		CClientMule* pFound = NULL;
		for(ClientMap::iterator it = m_Waiting.begin(); it != m_Waiting.end(); it++)
		{
			CClient* pClient = it->first;
			if(pClient && pClient->Type == CLIENT_MULE)
			{
				CClientMule* pMule = reinterpret_cast<CClientMule*>(pClient);
				if(pFound == NULL || pMule->Score > pFound->Score)
					pFound = pMule;
			}
		}
		if(pFound)
		{
			RemoveFromWaitingQueue(pFound);
			if(pFound->Parent)
			{
				// If we're already connected
				COpCode_ACCEPTUPLOADREQ stMsg;
				g_stEngine.SendOpCode(pFound->Parent->m_hSocket, stMsg, pFound->Parent, QUE_HIGH);
				pFound->UploadState = US_UPLOADING;
			}
			else
			{
				pFound->UploadState = US_CONNECTING;
				// Try to connect to this client
				// CUpDownClient::TryToConnect
			}
			m_Uploading.push_back(pFound);
			AddLog(LOG_DEBUG, _T("Client (%s:%d) has been added to uploading queue, %s"), inet_ntoa(pFound->ClientAddr), pFound->ClientPort, pFound->UploadState == US_CONNECTING ? _T("connecting") : _T("uploading"));
		}
	}
}

//////////////////////////////////////////////////////////////////////
bool CClientList::RemoveFromUploadQueue(CClient* pClient, EnumEndTransferSession eReason)
{
	if(pClient == NULL)
		return false;

	CString sReason = _T("");
	switch(eReason)
	{
	case ETS_CANCELED:
		pClient->UploadState = US_NONE;

		sReason = _T("cancelled");
		break;
	}

	ClientQueue::iterator it = m_Uploading.begin();
	while(it != m_Uploading.end())
	{
		if(*it == pClient)
			break;
		it++;
	}
	if(it != m_Uploading.end())
	{
		m_Uploading.erase(it);

		AddLog(LOG_DEBUG, _T("Client (%s:%d) has been removed from upload queue (reason: %s)"), inet_ntoa(pClient->ClientAddr), pClient->ClientPort, sReason);
		return true;
	}
	else
		AddLog(LOG_ERROR, _T("Can't find client (%s:%d) to remove from upload queue (reason: %s)"), inet_ntoa(pClient->ClientAddr), pClient->ClientPort, sReason);
	return false;
}

const UINT CHUNKSIZE = 10240;

//////////////////////////////////////////////////////////////////////
void CClientList::ProcessUpload()
{
	DWORD dwBytesToSend = g_stEngine.TcpEngineMule.GetAvailableBandwidth();

	for(ClientQueue::iterator it = m_Uploading.begin(); it != m_Uploading.end(); it++)
	{
		CClientMule* pMule = reinterpret_cast<CClientMule*>(*it);

		if(dwBytesToSend > 0)
		{
			DWORD dwBlockSize = min(dwBytesToSend, CHUNKSIZE);

			if(pMule->SendNextBlockData(dwBlockSize, false))
				dwBytesToSend -= dwBlockSize;
		}
		else
		{
			// If we already used all available bandwidth, we start to
			// trickle the unneeded uploads (just give them enough to not time out)
			// These downloads are kept connected, in a ready-to-go state, just in case
			// one of the fully activated uploads completes/timeouts/ends.
			// As soon as there's a little bandwidth leftover, the first one of these
			// uploads will go to fully activated state

			if(pMule->IsNeedMercyPacket())
				pMule->SendNextBlockData(MAXFRAGSIZE, true);
		}
	}
}
