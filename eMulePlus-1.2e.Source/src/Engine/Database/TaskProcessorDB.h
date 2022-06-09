// TaskProcessorDB.h: interface for the CTaskProcessor_DB class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../../Engine/TaskProcessor.h"
//#include "../BerkeleyDb/build_win32/db_cxx.h"

class Db;
class DbEnv;

struct StructLogLine
{
	EnumLogType		eType;
	CPreciseTime	tmTime;
	CString			sLine;
};

typedef list<StructLogLine>	LogLineList;

struct CTask_AddToLog : public CTask
{
	CTask_AddToLog(EnumLogType eType, CPreciseTime tmTime, LPCTSTR sLine);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("AddToLog"); }

	StructLogLine	m_Log;

//	long	m_lTime;
//	BOOL	m_bDebug;
//	DbEnv	*m_pDbEnv;
//	Db		*m_pDb;
};

class CTaskProcessor_DB : public CTaskProcessor
{
	virtual bool Start();
	virtual void Stop();
	virtual void ProcessTimeout();

public:
	CTaskProcessor_DB();
	virtual ~CTaskProcessor_DB();

	LogLineList m_LogsList;

#ifdef USE_BERKELEY_DB
	__declspec(property(get=_GetLogs)) Db& Logs;
	Db&	_GetLogs() const { if(!m_pDbLogs) throw; return *m_pDbLogs; }
#endif //USE_BERKELEY_DB

private:

#ifdef USE_BERKELEY_DB
	DbEnv	*m_pDbEnv;		// Database environment
	Db		*m_pDbLogs;		// Logs DB
#endif //USE_BERKELEY_DB

//	Db		*m_pDbServers;	// Servers list DB
//	Db		*m_pDbFiles;	// Files DB
//	Db		*m_pDbMD4Parts;	// Parts DB (for standard ed2k with MD4)

	friend class CLoggable;
};

