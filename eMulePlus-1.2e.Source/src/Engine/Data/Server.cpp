// Server.cpp: implementation of the server classes
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Server.h"
#include "../../OtherStructs.h"
#include "../../OtherFunctions.h"

#ifdef USE_BERKELEY_DB

enum _EnumTagTypes
{
//	TAGTYPE_STRING	= 2,
	TAGTYPE_INT		= 3,
	TAGTYPE_FLOAT	= 4,
};

CServer::CServer(CServerKey *pKey, CServerData *pData)
{
	if(pKey && pData)
	{
		m_Key.byteVersion = pKey->byteVersion;
		m_Key.ulAddr = pKey->ulAddr;
		m_Key.uPort = pKey->uPort;
		memcpy2(m_Key.szDynIP, pKey->szDynIP, MAX_DYNIP_LEN*sizeof(TCHAR));
		m_Key.bStatic = pKey->bStatic;
		m_Key.uPriority = pKey->uPriority;

		m_Data.byteVersion = pData->byteVersion;
		memcpy2(m_Data.szName, pData->szName, MAX_SERVERNAME_LEN*sizeof(TCHAR));
		memcpy2(m_Data.szDescription, pData->szDescription, MAX_SERVERDESC_LEN*sizeof(TCHAR));
		memcpy2(m_Data.szVersion, pData->szVersion, MAX_SERVER_VER_LEN*sizeof(TCHAR));
		m_Data.dwTCPFlags = pData->dwTCPFlags;
		m_Data.dwUDPFlags = pData->dwUDPFlags;
		m_Data.dwChallenge = pData->dwChallenge;
		m_Data.ulPingTime = pData->ulPingTime;
		m_Data.uPingCount = pData->uPingCount;
		m_Data.ulLastPingTime = pData->ulLastPingTime;
		m_Data.uFailedCount = pData->uFailedCount;
		m_Data.dwFilesNum = pData->dwFilesNum;
		m_Data.dwUsersNum = pData->dwUsersNum;
		m_Data.dwMaxUsers = pData->dwMaxUsers;
		m_Data.dwSoftMaxFiles = pData->dwSoftMaxFiles;
		m_Data.dwHardMaxFiles = pData->dwHardMaxFiles;
		m_Data.dwCreditChangeTime = pData->dwCreditChangeTime;
		m_Data.uCreditsLeft = pData->uCreditsLeft;
	}
	else
	{
		m_Key.byteVersion = 1;
		m_Key.ulAddr = 0;
		m_Key.uPort = 0;
		memset(m_Key.szDynIP, 0, MAX_DYNIP_LEN*sizeof(TCHAR));
		m_Key.bStatic = false;
		m_Key.uPriority = 1;

		m_Data.byteVersion = 1;
		memset(m_Data.szName, 0, MAX_SERVERNAME_LEN*sizeof(TCHAR));
		memset(m_Data.szDescription, 0, MAX_SERVERDESC_LEN*sizeof(TCHAR));
		memset(m_Data.szVersion, 0, MAX_SERVER_VER_LEN*sizeof(TCHAR));
		m_Data.dwTCPFlags = 0;
		m_Data.dwUDPFlags = 0;
		m_Data.dwChallenge = 0;
		m_Data.ulPingTime = 0;
		m_Data.uPingCount = 0;
		m_Data.ulLastPingTime = 0;
		m_Data.uFailedCount = 0;
		m_Data.dwFilesNum = 0;
		m_Data.dwUsersNum = 0;
		m_Data.dwMaxUsers = 0;
		m_Data.dwSoftMaxFiles = 0;
		m_Data.dwHardMaxFiles = 0;
		m_Data.dwCreditChangeTime = 0;
		m_Data.uCreditsLeft = MAX_SERVER_CREDITS;
	}

	m_pCursor = NULL;
}

bool CServer::ImportFromServerMet(CString sFile)
{
	try
	{
		CFile file;
		if (!file.Open(sFile, GENERIC_READ))
		{
			AddLog(LOG_ERROR, _T("Can't open file: %s"), sFile);
//			AddLogLine(false, IDS_ERR_LOADSERVERMET);
			return false;
		}
		BYTE uServerListVersion;
		file.Read(&uServerListVersion, sizeof(BYTE));
		if (uServerListVersion != 0xE0 && uServerListVersion != MET_HEADER)
		{
			file.Close();
			AddLog(LOG_ERROR, _T("Bad server.met version: %u"), uServerListVersion);
//			AddLogLine(false, IDS_ERR_BADSERVERMETVERSION, m_uServerListVersion);
			return false;
		}

		uint32 dwServerCount;
		file.Read(&dwServerCount, 4);

		ServerMet_Struct stServerStruct;
		uint32 dwAddedServerCount = 0;
		CString strListName;

		for (uint32 j = 0; j != dwServerCount; j++)
		{
			//	get server
			file.Read(&stServerStruct, sizeof(ServerMet_Struct));

			CServer stServer;
			stServer.m_Key.ulAddr = stServerStruct.m_dwIP;
			stServer.m_Key.uPort = stServerStruct.m_uPort;

			// read tags
			for (uint32 i = 0;i != stServerStruct.m_dwTagCount;i++)
			{
				BYTE byteTagType;
				file.Read(&byteTagType, sizeof(BYTE));
				USHORT uLength;
				file.Read(&uLength, sizeof(USHORT));
				BYTE byteSpecialTag;
				CString sName;
				CString sValue;
				int iValue;
				float fValue;
				if (uLength == 1)
				{
					file.Read(&byteSpecialTag, sizeof(BYTE));
				}
				else
				{
					byteSpecialTag = 0;
					file.Read(sName.GetBuffer(uLength + 1), uLength);
					sName.ReleaseBuffer(uLength);
				}
				if (byteTagType == TAGTYPE_STRING)
				{
					file.Read(&uLength, sizeof(USHORT));		
					file.Read(sValue.GetBuffer(uLength + 1), uLength);
					sValue.ReleaseBuffer(uLength);
				}
				else if (byteTagType == TAGTYPE_INT)
				{
					file.Read(&iValue, sizeof(int));
				}
				else if (byteTagType == TAGTYPE_FLOAT)
				{
					file.Read(&fValue, sizeof(float));
				}
				switch(byteSpecialTag)
				{
				case ST_SERVERNAME:						
					_tcsncpy((TCHAR*)stServer.m_Data.szName, sValue.GetBuffer(0), MAX_SERVERNAME_LEN);
					break;
				case ST_DESCRIPTION:						
					_tcsncpy((TCHAR*)stServer.m_Data.szDescription, sValue.GetBuffer(0), MAX_SERVERDESC_LEN);
					break;
				case ST_PREFERENCE:
					stServer.m_Key.uPriority = iValue;
					break;
				case ST_PING:
					stServer.m_Data.ulPingTime = iValue;
					break;
				case ST_DYNIP:						
					_tcsncpy((TCHAR*)stServer.m_Key.szDynIP, sValue.GetBuffer(0), MAX_DYNIP_LEN);
					break;
				case ST_FAIL:
					//m_dwFailedCount = iValue;
					break;
				case ST_LASTPING:
					//lastpingedtime = iValue;
					break;
				case ST_MAXUSERS:
					stServer.m_Data.dwMaxUsers = iValue;
					break;
				case ST_SOFTFILES:
					stServer.m_Data.dwSoftMaxFiles = iValue;
					break;
				case ST_HARDFILES:
					stServer.m_Data.dwHardMaxFiles = iValue;
					break;
				case ST_VERSION:
					if (byteTagType == TAGTYPE_STRING)
						_tcsncpy((TCHAR*)stServer.m_Data.szVersion, sValue.GetBuffer(0), MAX_SERVER_VER_LEN);
					break;
				case ST_UDPFLAGS:
					if (byteTagType == TAGTYPE_INT)
						stServer.m_Data.dwUDPFlags  =  iValue;
					break;
				default:
					if (sName == _T("files"))
						stServer.m_Data.dwFilesNum = iValue;
					else if (sName == _T("users"))
						stServer.m_Data.dwUsersNum = iValue;
				}
			}
			//	Set name for server
			if (stServer.GetName().IsEmpty())
			{
				CString s;
				s.Format(_T("Server %s"), stServer.GetStrAddress());
				_tcsncpy((TCHAR*)stServer.m_Data.szName, s, MAX_SERVERNAME_LEN);
			}

			// Check that we don't have it already
			CServer *pFound = NULL;
			CString sDynIP = stServer.GetDynIP();
			if(!sDynIP.IsEmpty() && Find(sDynIP, stServer.m_Key.uPort, pFound))
			{
				delete pFound;
				continue;
			}
			if(Find(stServer.m_Key.ulAddr, stServer.m_Key.uPort, pFound))
			{
				delete pFound;
				continue;
			}

			stServer.Save();

			dwAddedServerCount++;
		}

//			AddLogLine(true, IDS_SERVERSFOUND, dwServerCount);
		file.Close();
		return true;
	}
	catch (...)
	{
		ASSERT(FALSE);
		return false;
	}
}

bool CServer::ListStartByAddr()
{
//	int nRet = stEngine.m_pDbServersAddr->cursor(NULL, &m_pCursor, 0);
//	return !nRet;
	return false;
}

bool CServer::ListStartByPriority()
{
//	int nRet = stEngine.m_pDbServersPriority->cursor(NULL, &m_pCursor, 0);
//	return !nRet;
	return false;
}

bool CServer::ListStartByStatic()
{
//	int nRet = stEngine.m_pDbServersStatic->cursor(NULL, &m_pCursor, 0);
//	return !nRet;
	return false;
}

bool CServer::ListGetNext()
{
	Dbt pkey, key, data;
	int nRet = 1;
	if(m_pCursor)
	{
		if((nRet = m_pCursor->pget(&key, &pkey, &data, DB_NEXT)) == 0)
		{
			CServerKey *pKey = (CServerKey *)pkey.get_data();
			CServerData *pData = (CServerData *)data.get_data();
			memcpy2(&m_Key, pKey, sizeof(m_Key));
			memcpy2(&m_Data, pData, sizeof(m_Data));
		}
	}
	return !nRet;
}

bool CServer::ListFinish()
{
	int nRet = 1;
	if(m_pCursor)
		nRet = m_pCursor->close();
	return !nRet;
}

bool CServer::Save()
{
/*	try
	{
		Dbt key(&m_Key, sizeof(m_Key));
		Dbt data(&m_Data, sizeof(m_Data));

		int nRet = g_stEngine.m_pDbServers->put(NULL, &key, &data, 0);
		if(nRet)
		{
			g_stEngine.m_pDbServers->err(nRet, "CServer::Save");
			ASSERT(FALSE);
			return false;
		}
		return true;
	}
	catch (DbRunRecoveryException &dbe)
	{
		TRACE(CString(dbe.what()));
		return false;
	}
	catch (DbException &dbe)
	{
		TRACE("Problems working with database objects: %s.\n", dbe.what());
		return false;
	}
	catch(...)
	{
		TRACE("Problems working with database objects.\n");
		return false;
	}*/
	return true;
}

ULONG CServer::GetAddress()
{
	return m_Key.ulAddr;
}

USHORT CServer::GetPort()
{
	return m_Key.uPort;
}

CString CServer::GetName()
{
	return CString((TCHAR*)m_Data.szName);
}

CString CServer::GetStrAddress()
{
	CString sDynIP = GetDynIP();
	if (!sDynIP.IsEmpty())
		return sDynIP;
	else
	{
		in_addr host;
		host.S_un.S_addr = m_Key.ulAddr;
		return CString(inet_ntoa(host));
	}
}

CString CServer::GetDynIP()
{
	return CString((TCHAR*)m_Key.szDynIP);
}

bool CServer::Find(ULONG ulAddr, USHORT uPort, CServer *pFound)
{
	try
	{
		// szDynIP should be empty
		CServerPrimaryKey stKey;
		memset(&stKey, 0, sizeof(stKey));
		stKey.ulAddr = ulAddr;
		stKey.uPort = uPort;

		CServerKey tempKey;
		memset(&tempKey, 0, sizeof(tempKey));
		CServerData tempData;
		memset(&tempData, 0, sizeof(tempData));

		Dbt key(&stKey, sizeof(stKey));
		Dbt pkey(&tempKey, sizeof(tempKey));
		pkey.set_ulen(sizeof(tempKey));
		pkey.set_flags(DB_DBT_USERMEM);
		Dbt data(&tempData, sizeof(tempData));
		data.set_ulen(sizeof(tempData));
		data.set_flags(DB_DBT_USERMEM);

/*		int nRet = stEngine.m_pDbServersAddr->pget(NULL, &key, &pkey, &data, 0);
		if(nRet)
		{
			pFound = NULL;
			return false;
		}
		pFound = new CServer(&tempKey, &tempData);
		return true;*/
		return false;
	}
	catch (DbRunRecoveryException &dbe)
	{
		AddLog(LOG_ERROR, _T("DB Recovery exception: %s"), dbe.what());
		return false;
	}
	catch (DbException &dbe)
	{
		AddLog(LOG_ERROR, _T("Problems working with database objects: %s."), dbe.what());
		return false;
	}
	catch(...)
	{
		AddLog(LOG_ERROR, _T("Problems working with database objects."));
		return false;
	}
}

bool CServer::Find(CString sDynIP, USHORT uPort, CServer *pFound)
{
	try
	{
		// ulAddr should be zero
		CServerPrimaryKey stKey;
		memset(&stKey, 0, sizeof(CServerPrimaryKey));
		_tcsncpy((TCHAR*)stKey.szDynIP, sDynIP.GetBuffer(0), MAX_DYNIP_LEN);
		stKey.uPort = uPort;

		CServerKey tempKey;
		memset(&tempKey, 0, sizeof(tempKey));
		CServerData tempData;
		memset(&tempData, 0, sizeof(tempData));

		Dbt key(&stKey, sizeof(stKey));
		Dbt pkey(&tempKey, sizeof(tempKey));
		pkey.set_ulen(sizeof(tempKey));
		pkey.set_flags(DB_DBT_USERMEM);
		Dbt data(&tempData, sizeof(tempData));
		data.set_ulen(sizeof(tempData));
		data.set_flags(DB_DBT_USERMEM);

/*		int nRet = stEngine.m_pDbServersAddr->pget(NULL, &key, &pkey, &data, 0);
		if(nRet)
		{
			pFound = NULL;
			return false;
		}
		pFound = new CServer(&tempKey, &tempData);
		return true;*/
		return false;
	}
	catch (DbRunRecoveryException &dbe)
	{
		AddLog(LOG_ERROR, _T("DB Recovery exception: %s"), dbe.what());
		return false;
	}
	catch (DbException &dbe)
	{
		AddLog(LOG_ERROR, _T("Problems working with database objects: %s."), dbe.what());
		return false;
	}
	catch(...)
	{
		AddLog(LOG_ERROR, _T("Problems working with database objects."));
		return false;
	}
}
/*
int CServer::PrimaryIndex(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey)
{
	CServerKey *pServerKey = (CServerKey *)(pKey->get_data());
	CServerPrimaryKey *pTempKey = new CServerPrimaryKey;
	memset(pTempKey, 0, sizeof(CServerPrimaryKey));
	pTempKey->ulAddr = pServerKey->ulAddr;
	pTempKey->uPort = pServerKey->uPort;
	_tcsncpy((TCHAR*)pTempKey->szDynIP, (TCHAR*)pServerKey->szDynIP, MAX_DYNIP_LEN);
	*pNewKey = Dbt();
	pNewKey->set_data(pTempKey);
	pNewKey->set_size(sizeof(CServerPrimaryKey));
	pNewKey->set_flags(DB_DBT_APPMALLOC);
	return 0;
}

int CServer::SecondaryIndexPriority(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey)
{
	CServerKey *pServerKey = (CServerKey *)(pKey->get_data());
	CServerSecondaryKeyPriority *pTempKey = new CServerSecondaryKeyPriority;
	memset(pTempKey, 0, sizeof(CServerSecondaryKeyPriority));
	pTempKey->uPriority = pServerKey->uPriority;
	pNewKey->set_data(pTempKey);
	pNewKey->set_size(sizeof(CServerSecondaryKeyPriority));
	pNewKey->set_flags(DB_DBT_APPMALLOC);
	return 0;
}

int CServer::SecondaryIndexStatic(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey)
{
	CServerKey *pServerKey = (CServerKey *)(pKey->get_data());
	CServerSecondaryKeyStatic *pTempKey = new CServerSecondaryKeyStatic;
	memset(pTempKey, 0, sizeof(CServerSecondaryKeyStatic));
	pTempKey->bStatic = pServerKey->bStatic;
	pNewKey->set_data(pTempKey);
	pNewKey->set_size(sizeof(CServerSecondaryKeyStatic));
	pNewKey->set_flags(DB_DBT_APPMALLOC);
	return 0;
}
*/
#endif //USE_BERKELEY_DB
