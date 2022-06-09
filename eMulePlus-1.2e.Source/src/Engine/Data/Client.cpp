// Client.cpp: implementation of the CClient class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "ClientList.h"
#include "../Sockets/OpCode.h"
#include "../Sockets/TasksOpcodes.h"
#include "../Files/TaskProcessorFiles.h"

//////////////////////////////////////////////////////////////////////
CClient::CClient()
	:m_pParent(NULL)
	,m_bConnected(false)
	,m_uCountry(0)
	,m_sFullIP(_T(""))
	,m_sUserName(_T(""))
	,m_pReqFile(NULL)
	,m_eUploadState(US_NONE)
	,m_eDownloadState(DS_NONE)
{
	m_Addr.Addr = 0;
	m_Addr.Port = 0;
}

//////////////////////////////////////////////////////////////////////
CClient::~CClient()
{
}

//////////////////////////////////////////////////////////////////////
bool CClient::operator==(const AddrPort Addr)
{
	return (m_Addr.Addr == Addr.Addr && m_Addr.Port == Addr.Port);
}

//////////////////////////////////////////////////////////////////////
CClientMule::CClientMule(AddrPort Addr, HashType Hash)
{
	m_Addr.Addr = Addr.Addr;
	m_Addr.Port = Addr.Port;
	md4cpy(&m_Hash, &Hash);
	m_eClientSoft = SO_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////
CClientMule::~CClientMule()
{
}

//////////////////////////////////////////////////////////////////////
bool CClientMule::operator==(const CClient* pClient)
{
	return false;
}

//////////////////////////////////////////////////////////////////////
bool CClientMule::operator==(const HashType Hash)
{
	return !md4cmp(&m_Hash, &Hash);
}

//////////////////////////////////////////////////////////////////////
void CClientMule::_PutMuleVersion(BYTE nMuleVersion)
{
	m_dwClientVersion = nMuleVersion;
	m_nCompatibleClient = (m_dwClientVersion >> 24);
	m_dwClientVersion &= 0x00FFFFFF;
	m_nEmuleVersion = 0x99;
//	m_fSupportsAskSharedDirs = 1;
}

//////////////////////////////////////////////////////////////////////
EnumClientTypes CClientMule::GetHashType()
{
	if (m_Hash.hash[5] == 14 && m_Hash.hash[14] == 111)
		return SO_EMULE;
	else if (m_Hash.hash[5] == 13 && m_Hash.hash[14] == 110)
		return SO_OLDEMULE;
	else if (m_Hash.hash[5] == 'M' && m_Hash.hash[14] == 'L')
		return SO_MLDONKEY;
	else
		return SO_UNKNOWN;
}

//////////////////////////////////////////////////////////////////////
void CClientMule::OnConnected()
{
	CClient::OnConnected();
}

//////////////////////////////////////////////////////////////////////
void CClientMule::OnDisconnected()
{
	CClient::OnDisconnected();
	// change state, take care of upload/download statuses, etc
}

//////////////////////////////////////////////////////////////////////
void CClientMule::RequestFile(CKnownFile* pFile)
{
	m_pReqFile = pFile;
	m_tmLastUpRequest = CPreciseTime::GetCurrentTime();

	// Add client to waiting queue
	g_stEngine.ClientList.AddToWaitingQueue(this);

	// Check requests limit and ban - CUpDownClient::AddRequestCount
}

//////////////////////////////////////////////////////////////////////
void CClientMule::RequestFileBlock(CKnownFile* pFile, DWORD dwStart, DWORD dwEnd)
{
	if(pFile)
	{
		// Check that we don't have already this request
		pair <RequestedBlocksMap::iterator, RequestedBlocksMap::iterator> 
			itRange = m_RequestedBlocks.equal_range(pFile);
		for(RequestedBlocksMap::iterator it = itRange.first; it != itRange.second; it++)
		{
			if(it->second.dwStart == dwStart && it->second.dwEnd == dwEnd)
				break;
		}
		// If not found, add it to the map
		m_RequestedBlocks.insert(BlocksMapPair(pFile, RequestRange(dwStart, dwEnd)));
	}
}

//////////////////////////////////////////////////////////////////////
bool CClientMule::SendNextBlockData(DWORD dwMaxSize, bool bPrioritized)
{
	RequestedBlocksMap::iterator it = m_RequestedBlocks.begin();
	// If we don't have blocks to send...
	if(it == m_RequestedBlocks.end())
		return false;

	CKnownFile* pFile	= it->first;
	DWORD dwStart		= it->second.dwStart;
	DWORD dwEnd			= it->second.dwEnd;
	if((dwEnd - dwStart) > dwMaxSize)
	{
		// Only cut [dwMaxSize] block from request
		dwEnd = dwStart + dwMaxSize;
		it->second.dwStart += dwMaxSize;
	}
	else
	{
		// Request completed and can be erased
		m_RequestedBlocks.erase(it);
	}
	CTask_SendBlock* pTask = new CTask_SendBlock(pFile, m_pParent, dwStart, dwEnd);
	if(pTask)
	{
		g_stEngine.Files.Push(pTask);
		m_LastUploadTime = CPreciseTime::GetCurrentTime();
	}
	else
		AddLog(LOG_ERROR, _T("No memory"));

	return true;
}

const UINT MAXTIMESLOTISALLOWEDTOGOWITHOUTDATAMS = 2540;

//////////////////////////////////////////////////////////////////////
bool CClientMule::IsNeedMercyPacket()
{
	return (CPreciseTime::GetCurrentTime() - m_LastUploadTime) > MAXTIMESLOTISALLOWEDTOGOWITHOUTDATAMS;
}

//////////////////////////////////////////////////////////////////////
void CClientMule::SendRankingInfo()
{
	if(!MuleProtocol)
		return;

	USHORT uRank = g_stEngine.ClientList.GetWaitingPosition(this);

	if (uRank == 0)
		return;

	COpCode_QUEUERANKING stMsg;
	stMsg._Rank = uRank;
	g_stEngine.SendOpCode(Parent->m_hSocket, stMsg, Parent, QUE_HIGH);
}


