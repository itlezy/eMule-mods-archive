//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "updownclient.h"
#include "ClientList.h"
#include "emule.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClientList::CClientList() : m_clientList(100)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClientList::~CClientList()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientList::GetStatistics(uint32 &totalclient, uint32 stats[], ClientsData *pData, uint32 &totalMODs, uint32 *pdwTotalPlusMODs, CMap<POSITION, POSITION, uint32, uint32> *MODs, CMap<POSITION, POSITION, uint32, uint32> *PlusMODs, CMap<int, int, uint32, uint32> *pCountries)
{
	EMULE_TRY
	if (!pData)
		return;

	POSITION			pos_MOD;
	CUpDownClient		*pClient;
	CString				strMODName;
	uint32				dwCount;
	int					i, iCountryIdx;

	totalclient = m_clientList.GetCount();

	for (i = 0; i < SO_LAST; i++)
	{
		pData->m_pClients[i].RemoveAll();
		stats[i] = 0;
	}
	pData->m_pClients[i].RemoveAll(); // reset the last one

//	Reset MODs variables
	totalMODs = 0;
	*pdwTotalPlusMODs = 0;
	MODs->RemoveAll();
	PlusMODs->RemoveAll();

//	Reset Countries variables
	pCountries->RemoveAll();

//	Reset SUI, LowID and Problematic clients info
	stats[14] = 0;	//	SUI success
	stats[15] = 0;	//	SUI failed
	stats[16] = 0;	//	LowID
	stats[17] = 0;	//	Problematic clients (DS_ERROR)

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL; )
	{
		pClient = m_clientList.GetNext(pos);

		int		iVersion = pClient->GetVersion();
		uint32	dwClientSoft = pClient->GetClientSoft();

		if (dwClientSoft >= SO_UNKNOWN)
			stats[SO_UNKNOWN]++;
		else
		{
			stats[dwClientSoft]++;
			(pData->m_pClients[dwClientSoft])[iVersion]++;
		}

		(pData->m_pClients[SO_LAST])[pClient->GetDownloadState()]++;

		if (pClient->Credits() != NULL)
		{
			switch(pClient->Credits()->GetCurrentIdentState(pClient->GetIP()))
			{
				case IS_IDENTIFIED:
					stats[14]++;
					break;
				case IS_IDFAILED:
				case IS_IDNEEDED:
				case IS_IDBADGUY:
					stats[15]++;
			}
		}

		if (pClient->HasLowID())
			stats[16]++;

		if (pClient->GetDownloadState() == DS_ERROR)
			stats[17]++;

	//	Count MODs
		if ((dwClientSoft == SO_EMULE) || (dwClientSoft == SO_OLDEMULE))
		{
			strMODName = pClient->GetModString();
			if (!strMODName.IsEmpty())
			{
				totalMODs++;
				pos_MOD = liMODsTypes.Find(strMODName);
				if (!pos_MOD)
				{
					pos_MOD = liMODsTypes.AddTail(strMODName);
					MODs->SetAt(pos_MOD, 1);
				}
				else
				{
					dwCount = 0;
					MODs->Lookup(pos_MOD, dwCount);
					MODs->SetAt(pos_MOD, ++dwCount);
				}
			}
		}
		else if (dwClientSoft == SO_PLUS)
		{
			strMODName = pClient->GetModString();
			if (!strMODName.IsEmpty())
			{
				++*pdwTotalPlusMODs;
				pos_MOD = liMODsTypes.Find(strMODName);
				if (!pos_MOD)
				{
					pos_MOD = liMODsTypes.AddTail(strMODName);
					PlusMODs->SetAt(pos_MOD, 1);
				}
				else
				{
					dwCount = 0;
					PlusMODs->Lookup(pos_MOD, dwCount);
					PlusMODs->SetAt(pos_MOD, ++dwCount);
				}
			}
		}

	//	Count Countries
		CMap<int, int, uint32, uint32>::CPair *pPair;

		iCountryIdx = pClient->GetCountryIndex();
		pPair = pCountries->PLookup(iCountryIdx);
		if (pPair != NULL)
			pPair->value++;
		else
			pCountries->SetAt(iCountryIdx, 1);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientList::AddClient(CUpDownClient* toadd, bool bSkipDupTest)
{
	EMULE_TRY

	if (toadd == NULL)
		return false;

	if ( !bSkipDupTest)
	{
		if (m_clientList.Find(toadd))
			return false;
	}
	if (m_pctlClientList != NULL && m_pctlClientList->m_hWnd != NULL)
		m_pctlClientList->AddClient(toadd);
	m_clientList.AddTail(toadd);

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientList::RemoveClient(CUpDownClient *pClient)
{
	EMULE_TRY

	POSITION		pos = m_clientList.Find(pClient);

	if (pos != NULL)
	{
		g_App.m_pUploadQueue->RemoveFromUploadQueue(pClient, ETS_DISCONNECT);
		g_App.m_pUploadQueue->RemoveFromWaitingQueue(pClient);
		g_App.m_pDownloadQueue->RemoveSource(pClient);
		m_pctlClientList->RemoveClient(pClient);
		pClient->LeaveSourceLists();
		m_clientList.RemoveAt(pos);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientList::DeleteAll()
{
	EMULE_TRY

	g_App.m_pUploadQueue->DeleteAll();
	g_App.m_pDownloadQueue->DeleteAll();

	POSITION			pos1, pos2;
	CUpDownClient	   *pClient;

	for (pos1 = m_clientList.GetHeadPosition(); (pos2 = pos1) != NULL; )
	{
		pClient = m_clientList.GetNext(pos1);
		m_clientList.RemoveAt(pos2);
		if (pClient != NULL)
			delete pClient;
	}
	liMODsTypes.RemoveAll();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OLD_SOCKETS_ENABLED
bool CClientList::AttachToAlreadyKnown(CUpDownClient **ppClient, CClientReqSocket *pSocket)
{
	EMULE_TRY

	if (pSocket == NULL)
		return false;

	CUpDownClient	*pClient = (*ppClient);
	CUpDownClient	*pClientInList;
	CUpDownClient	*pFoundClient = NULL;
	uint32 		dwRes = 0;

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL;)
	{
		pClientInList = m_clientList.GetNext(pos);
		if (pClientInList != NULL && pClient != pClientInList)
		{
			dwRes = pClient->Compare(pClientInList);

			if ((dwRes & (CLIENT_COMPARE_SAME_ID | CLIENT_COMPARE_SAME_IP)) != 0)
			{
				pFoundClient = pClientInList;
				break;
			}
			else if ((dwRes & CLIENT_COMPARE_SAME_HASH) != 0)
			{
				pFoundClient = pClientInList;
			}
		}
	}

	if (pFoundClient != NULL)
	{
	//	don't allow to reattach the socket 
		if (pFoundClient == pClient)
			return true;

		if (pFoundClient->m_pRequestSocket != NULL)
		{
		//	Check if in the list client is connected to us and new client has only same hash
			if (pFoundClient->m_pRequestSocket->IsConnected()
				&& (dwRes & (CLIENT_COMPARE_SAME_ID | CLIENT_COMPARE_SAME_IP)) == 0)
			{
				if (pFoundClient->m_pCredits != NULL &&
					pFoundClient->m_pCredits->GetCurrentIdentState(pFoundClient->GetIP()) == IS_IDENTIFIED)
				{
				//	If client in the list was properly identified then ban new client with same hash
					pClient->Ban(BAN_CLIENT_HASH_STEALER);
					if (!g_App.m_pPrefs->IsCMNotLog())
					{
						AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Ban new client instance %s (%s:%u) that used protected userhash (identified client %s, %s:%u)"),
							pClient->GetClientNameWithSoftware(), pClient->GetFullIP(), pClient->GetUserPort(),
							pFoundClient->GetClientNameWithSoftware(), pFoundClient->GetFullIP(), pFoundClient->GetUserPort() );
					}
				}

				return false;
			}
#if 0
		//	Check special case when both clients are trying to connect at same time
			else if (pFoundClient->m_pRequestSocket->IsConnecting()
				|| (pFoundClient->m_pRequestSocket->IsConnected()
					&& ((dwRes & (CLIENT_COMPARE_SAME_ID | CLIENT_COMPARE_SAME_IP)) != 0) ))
			{
				AddLogLine( LOG_FL_DBG | LOG_RGB_WARNING, _T("Connection collision: try to attach incoming client (%s, %s:%u) to the client with active socket (%s, %s:%u)"),
					pClient->GetClientNameWithSoftware(), pClient->GetFullIP(), pClient->GetUserPort(),
					pFoundClient->GetClientNameWithSoftware(), pFoundClient->GetFullIP(), pFoundClient->GetUserPort() );
			}
#endif

			pFoundClient->m_pRequestSocket->Safe_Delete();
		}
	//	Check if remote client tries to change a hash
		else if ( ((dwRes & (CLIENT_COMPARE_SAME_ID | CLIENT_COMPARE_SAME_IP)) != 0)
				&& ((dwRes & CLIENT_COMPARE_SAME_HASH) == 0)
				&& pFoundClient->HasValidHash())
		{
			pFoundClient->Ban(BAN_CLIENT_HASH_STEALER);
			if (!g_App.m_pPrefs->IsCMNotLog())
			{
				AddLogLine( LOG_FL_DBG | LOG_RGB_DIMMED, _T("Ban client instance %s (%s:%u) with userhash %s that changes userhash to %s"),
					pFoundClient->GetClientNameWithSoftware(),
					pFoundClient->GetFullIP(), pFoundClient->GetUserPort(),
					HashToString(pFoundClient->GetUserHash()),
					HashToString(pClient->GetUserHash()) );
			}
		}

		pFoundClient->m_pRequestSocket = pSocket;
		pClient->m_pRequestSocket = NULL;

	// overwrite the pointer
		*ppClient = pFoundClient;
	// now we can delete the client
		delete pClient;
		return true;
	}

	EMULE_CATCH

	return false;
}
#endif //OLD_SOCKETS_ENABLED
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient* CClientList::FindClientByIP(uint32 clientip, uint16 port)
{
	EMULE_TRY

	CUpDownClient	   *pClient;

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL; )
	{
		pClient = m_clientList.GetNext(pos);
		if (pClient)
			if (pClient->GetIP() == clientip && pClient->GetUserPort() == port)
				return pClient;
	}

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient* CClientList::FindClientByUserHash(uchar* clienthash)
{
	EMULE_TRY

	CUpDownClient	   *pClient;

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL; )
	{
		pClient = m_clientList.GetNext(pos);
		if (pClient)
			if (!md4cmp(pClient->GetUserHash() , clienthash))
				return pClient;
	}

	EMULE_CATCH

	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CUpDownClient* CClientList::FindClient(uint32 dwUserIDHyb, uint16 uUserPort, uint32 dwSrvIP, uint16 uSrvPort, uchar *pbyteUserHash)
{
	EMULE_TRY

	CUpDownClient	*pClient;
	CUpDownClient	*pFoundClient = NULL;
	bool			bValidSrv = ((dwSrvIP != 0) && (uSrvPort != 0));

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL;)
	{
		pClient = m_clientList.GetNext(pos);
		if (pClient == NULL)
			continue;

		if (IsLowID(dwUserIDHyb))
		{
			if (bValidSrv && (dwSrvIP == pClient->GetServerIP())
				&& uSrvPort == pClient->GetServerPort())
			{
				if (pClient->HasLowID())
				{
				//	Compare the real LowID clients
					if (dwUserIDHyb == pClient->GetUserIDHybrid())
					{
						pFoundClient = pClient;
						break;
					}
				}
				else
				{
				//	Compare new LowID source from server with x.x.x.0 HighID client
				//	that was already added over SX
					if (dwUserIDHyb == ntohl(pClient->GetUserIDHybrid()))
					{
						pFoundClient = pClient;
						break;
					}
				}
			}
		}
		else if ((dwUserIDHyb != 0) && (uUserPort == pClient->GetUserPort()))
		{
			if (dwUserIDHyb == pClient->GetUserIDHybrid())
			{
				pFoundClient = pClient;
				break;
			}

			if (pClient->HasLowID() && bValidSrv
				&& dwSrvIP == pClient->GetServerIP()
				&& uSrvPort == pClient->GetServerPort())
			{
				if (pClient->GetIP() == 0)
				{
				//	A x.x.x.0 HighID client has been already received from the server,
				//	added to the list as LowID and wasn't connected yet
					if (dwUserIDHyb == ntohl(pClient->GetUserIDHybrid()))
					{
						pClient->SetUserIDHybrid(dwUserIDHyb);
						pFoundClient = pClient;
						break;
					}
				}
				else
				{
				//	When a user changed LowID to HighID on same server and we got
				//	him as a HighID source over SX or from server over UDP request
					if (dwUserIDHyb == ntohl(pClient->GetIP()))
					{
						pClient->SetUserIDHybrid(dwUserIDHyb);
						pFoundClient = pClient;
						break;
					}
				}
			}
		}

		if (pbyteUserHash != NULL && (md4cmp(pbyteUserHash, pClient->GetUserHash()) == 0) && pClient->HasValidHash())
			pFoundClient = pClient;
	}

	return pFoundClient;

	EMULE_CATCH

	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#ifdef OLD_SOCKETS_ENABLED
void CClientList::Debug_SocketDeleted(CClientReqSocket *pSocket)
{
	EMULE_TRY

	CUpDownClient	   *pClient;

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL; )
	{

		pClient = m_clientList.GetNext(pos);

		if (!AfxIsValidAddress(pClient, sizeof(CUpDownClient)))
		{
			AfxDebugBreak();
		}
		if (pClient && pClient->m_pRequestSocket == pSocket)
		{
		//	AfxDebugBreak();
		}
	}

	EMULE_CATCH
}
#endif //OLD_SOCKETS_ENABLED
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientList::IsValidClient(CUpDownClient *pClient)
{
	EMULE_TRY

	if (pClient == NULL)
		return false;

	return (m_clientList.Find(pClient) != NULL);

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientList::GetClientListByFileID(CTypedPtrList<CPtrList, CUpDownClient*> *pClientList, const uchar *fileid)
{
	EMULE_TRY

	pClientList->RemoveAll();

	CUpDownClient	   *pClient;

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL; )
	{
		pClient = m_clientList.GetNext(pos);
		if (md4cmp(pClient->GetUploadFileID(), fileid) == 0)
			pClientList->AddTail(pClient);
	}

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CClientList::GetA4AFSourcesCount()
{
	EMULE_TRY

	CUpDownClient	   *pClient;
	uint32				dwCountA4AF = 0;

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL; )
	{
		pClient = m_clientList.GetNext(pos);
		if (!pClient->m_otherRequestsList.IsEmpty())
			dwCountA4AF++;
	}

	return dwCountA4AF;

	EMULE_CATCH
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientList::UpdateClient(CUpDownClient *pClient)
{
	if (pClient != NULL)
	{
		POSITION		posClient = m_clientList.Find(pClient);

	//	If the client is in the list...
		if (posClient != NULL)
		{
			g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.UpdateClient(pClient);
			g_App.m_pDownloadList->UpdateSource(pClient);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientList::ResetIP2Country()
{
	CUpDownClient *pClient;

	for(POSITION pos = m_clientList.GetHeadPosition(); pos != NULL; m_clientList.GetNext(pos))
	{ 
		pClient = g_App.m_pClientList->m_clientList.GetAt(pos); 
		pClient->ResetIP2Country();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientList::UpdateBanCounters()
{
	EMULE_TRY

	uint32			dwUploadCount = 0, dwDownloadCount = 0;
	CUpDownClient	*pClient;

	for (POSITION pos = m_clientList.GetHeadPosition(); pos != NULL;)
	{
		pClient = m_clientList.GetNext(pos);
		if ((pClient != NULL) && pClient->IsBanned())
		{
			if (pClient->GetUploadState() == US_ONUPLOADQUEUE)
				dwUploadCount++;
			if (pClient->GetDownloadState() != DS_NONE && pClient->GetDownloadState() != DS_ERROR)
				dwDownloadCount++;
		}
	}

	g_App.m_pUploadQueue->SetBanCount(dwUploadCount);
	g_App.m_pDownloadQueue->SetBanCount(dwDownloadCount);

	EMULE_CATCH
}
