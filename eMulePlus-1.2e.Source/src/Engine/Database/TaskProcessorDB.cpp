// TaskProcessorDB.cpp: implementation of the CTaskProcessor_DB class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TaskProcessorDB.h"
#include <time.h>
#ifdef USE_BERKELEY_DB
	#include "../../BerkeleyDb/build_win32/db_cxx.h"
#endif //USE_BERKELEY_DB

//#define SERVERLIST_DB _T("Servers.db")
//#define FILES_DB _T("files.db")

CTaskProcessor_DB::CTaskProcessor_DB()
#ifdef USE_BERKELEY_DB
	: m_pDbEnv(NULL)
	,m_pDbLogs(NULL)
#endif //USE_BERKELEY_DB
//	,m_pDbServers(NULL)
//	,m_pDbFiles(NULL)
//	,m_pDbMD4Parts(NULL)

{
	m_dwStartupTimeout = 10000;	// 10 seconds timeout to open all databases
}

CTaskProcessor_DB::~CTaskProcessor_DB()
{
	Stop();
}

bool CTaskProcessor_DB::Start()
{
	m_dwWaitTimeout = 1000;

#ifdef USE_BERKELEY_DB
	try
	{
		CString sDbHome = _T("./Db"); //CString(m_pGlobPrefs->GetAppDir()) + _T("Db");

		::CreateDirectory(sDbHome, NULL);	// In case it doesn't exist

		// Create environment
		m_pDbEnv = new DbEnv(0);
		m_pDbEnv->set_cachesize(0, 256 * 1024, 0);	// 256kb cache size
//		m_pDbEnv->set_alloc(&malloc,&realloc,&free);
		USES_CONVERSION;
		m_pDbEnv->open( CT2CA(sDbHome), DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG
			| DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER
			| DB_THREAD, 0 );

		m_pDbEnv->set_flags(DB_AUTO_COMMIT, 1);

		// Servers list
		m_pDbLogs = new Db(m_pDbEnv, 0);
		m_pDbLogs->set_pagesize(16 * 1024); // 16Kb page size
		m_pDbLogs->open(NULL, _T("Logs.db"), _T("Logs"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);


/*
		// Servers list
		m_pDbServers = new Db(m_pDbEnv, 0);
		m_pDbServers->set_pagesize(16 * 1024); // 16Kb page size
		m_pDbServers->open(NULL, SERVERLIST_DB, _T("Server-List"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);
*/
/*		// Servers list - primary index by addr/port
		m_pDbServersAddr = new Db(m_pDbEnv, 0);
		m_pDbServersAddr->open(NULL, SERVERLIST_DB, _T("Server-List-Addr"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);

		m_pDbServers->associate(NULL, m_pDbServersAddr, CServer::PrimaryIndex, NULL);

		// Servers list - secondary index by 'static' field
		m_pDbServersStatic = new Db(m_pDbEnv, 0);
		m_pDbServersStatic->set_flags(DB_DUP | DB_DUPSORT);
		m_pDbServersStatic->open(NULL, SERVERLIST_DB, _T("Server-List-Static"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);

		m_pDbServers->associate(NULL, m_pDbServersStatic, CServer::SecondaryIndexStatic, NULL);

		// Servers list - secondary index by 'priority' field
		m_pDbServersPriority = new Db(m_pDbEnv, 0);
		m_pDbServersPriority->set_flags(DB_DUP | DB_DUPSORT);
		m_pDbServersPriority->open(NULL, SERVERLIST_DB, _T("Server-List-Priority"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);

		m_pDbServers->associate(NULL, m_pDbServersPriority, CServer::SecondaryIndexPriority, NULL);
*/
/*		// files DB
		m_pDbFiles = new Db(m_pDbEnv, 0);
		m_pDbFiles->set_pagesize(16 * 1024); // 16Kb page m_lSize
		m_pDbFiles->open(NULL, FILES_DB, _T("FileDB"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);
*/
/*		// files DB - secondary index by hash
		m_pDbFilesHash = new Db(m_pDbEnv, 0);
		m_pDbFilesHash->set_flags(DB_DUP | DB_DUPSORT);
		m_pDbFilesHash->open(NULL, FILES_DB, _T("FileDB-Hash"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);

///		m_pDbFiles->associate(NULL, m_pDbFilesHash, CKnownFile::FileHashIndex, NULL);
*/
/*		// part DB
		m_pDbMD4Parts = new Db(m_pDbEnv, 0);
		m_pDbMD4Parts->set_pagesize(16 * 1024); // 16Kb page size
		m_pDbMD4Parts->open(NULL, FILES_DB, _T("MD4PartDB"), DB_BTREE, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0);
*/
	}
	catch (DbRunRecoveryException &dbe)
	{
		AddLog(LOG_ERROR, _T("DB recovery exception: "), dbe.what());
		return false;
	}
	catch (DbException &dbe)
	{
//		m_pDbServers = NULL;
		m_pDbLogs = NULL;
		m_pDbEnv = NULL;
		AddLog(LOG_ERROR, _T("Problems create database objects: %s. Closing."), dbe.what());
		return false;
	}
	catch(...)
	{
//		m_pDbServers = NULL;
		m_pDbLogs = NULL;
		m_pDbEnv = NULL;
		AddLog(LOG_ERROR, _T("Problems create database objects. Closing."));
		return false;
	}
#endif //USE_BERKELEY_DB

	return true;
}

void CTaskProcessor_DB::Stop()
{
#ifdef USE_BERKELEY_DB
	try
	{
/*		if(m_pDbServers)
		{
			m_pDbServers->close(0);
			safe_delete(m_pDbServers);
		}

		//close file DB
		if(m_pDbFilesHash)
		{
			m_pDbFilesHash->close(0);
			safe_delete(m_pDbFilesHash);
		}

		if(m_pDbFiles)
		{
			m_pDbFiles->close(0);
			safe_delete(m_pDbFiles);
		}
*/
		if(m_pDbLogs)
		{
			m_pDbLogs->close(0);
			// [kuchin] see below
//			safe_delete(m_pDbLogs);
		}

/*		if(m_pDbMD4Parts)
		{
			m_pDbMD4Parts->close(0);
			safe_delete(m_pDbMD4Parts);
		}*/

	//	Always close m_pDbEnv last to avoid strange behaviors
//		FILE *f = fopen("_a", "w");
//		m_pDbEnv->set_errfile(f);

		if(m_pDbEnv)
		{
			m_pDbEnv->txn_checkpoint(0,0,0);
			m_pDbEnv->close(0);
			// [kuchin] if i delete this object, program crash
			// i don't have time to debug it now, anyway it won't hurt anything since everything is already closed
//			safe_delete(m_pDbEnv);
		}
	}
	catch(DbException &dbe)
	{
		AddLog(LOG_ERROR, _T("Problems delete database objects: %s."), dbe.what());
	}
	catch(...)
	{
	//	AddLog(LOG_ERROR, _T("Problems delete database objects."));
	}
#endif //USE_BERKELEY_DB
}

void CTaskProcessor_DB::ProcessTimeout()
{
	m_dwWaitTimeout = 1000; // perform check once per second
}

CTask_AddToLog::CTask_AddToLog(EnumLogType eType, CPreciseTime tmTime, LPCTSTR sLine)
{
	m_Log.eType  = eType;
	m_Log.tmTime = tmTime;
	m_Log.sLine  = sLine;
}

bool CTask_AddToLog::Process()
{
	g_stEngine.DB.m_LogsList.push_back(m_Log);
	return true;
}

/*CTask_AddToLog::CTask_AddToLog(DbEnv *pDbEnv, Db *pDb, BOOL bDebug, LPCTSTR sLine)
	:m_lTime(time(NULL))
	,m_bDebug(bDebug)
	,m_sLine(sLine)
#ifdef USE_BERKELEY_DB
	,m_pDbEnv(pDbEnv)
	,m_pDb(pDb)
#endif //USE_BERKELEY_DB
{
}

bool CTask_AddToLog::Process()
{
#ifdef USE_BERKELEY_DB
	try
	{
		if(m_pDbEnv && m_pDb)
		{
//			DbTxn *tid;
//			m_pDbEnv->txn_begin(NULL, &tid, 0);

			DWORD dwSize = sizeof(BOOL) + m_sLine.GetLength() + 1;
			BYTE *pData = new BYTE[dwSize];
			CopyMemory(pData, &m_bDebug, sizeof(m_bDebug));
			CopyMemory(pData + sizeof(m_bDebug), (LPCTSTR)m_sLine, m_sLine.GetLength() + 1);
			Dbt key((void*)&m_lTime, sizeof(m_lTime));
			Dbt data((void*)pData, dwSize);

			m_pDb->put(NULL, &key, &data, DB_AUTO_COMMIT);

			delete[] pData;

//			tid->commit(0);
		}
	}
	catch(DbException &dbe)
	{
		AddLog(LOG_ERROR, _T("DB Problems: %s."), dbe.what());
	}
	catch(...)
	{
		AddLog(LOG_ERROR, _T("Problems add log to database."));
	}
#endif //USE_BERKELEY_DB
	return true;
}
*/