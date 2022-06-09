// Server.h: interface for the server classes
//
//////////////////////////////////////////////////////////////////////

#pragma once

#ifdef USE_BERKELEY_DB

#include "../../BerkeleyDb/build_win32/db_cxx.h"

#define MAX_SERVER_CREDITS	1200

// Strings lengths
#define MAX_DYNIP_LEN		50
#define MAX_SERVERNAME_LEN	50
#define MAX_SERVERDESC_LEN	150
#define MAX_SERVER_VER_LEN	50

struct CServerKey
{
	BYTE	byteVersion;	// Structure version. Should be 1 until we change this structure
	ULONG	ulAddr;			// Server address
	USHORT	uPort;			// Server port (key should be unique and we can have many servers on the same IP)
	BYTE	szDynIP[MAX_DYNIP_LEN*sizeof(TCHAR)];	// Dynamic IP address string
	bool	bStatic;		// Is static
	USHORT	uPriority;		// Priority - can be 0, 1 or 2
};

struct CServerPrimaryKey
{
	ULONG	ulAddr;			// Server address
	USHORT	uPort;			// Server port
	BYTE	szDynIP[MAX_DYNIP_LEN*sizeof(TCHAR)];	// Dynamic IP address string
};

struct CServerSecondaryKeyPriority
{
	USHORT	uPriority;
};

struct CServerSecondaryKeyStatic
{
	bool	bStatic;
};

struct CServerData
{
	BYTE	byteVersion;	// Structure version. Should be 1 until we change this structure

	BYTE	szName[MAX_SERVERNAME_LEN*sizeof(TCHAR)];			// List name
	BYTE	szDescription[MAX_SERVERDESC_LEN*sizeof(TCHAR)];	// Description
	BYTE	szVersion[MAX_SERVER_VER_LEN*sizeof(TCHAR)];		// Server version

	DWORD	dwTCPFlags;		// TCP flags
	DWORD	dwUDPFlags;		// UDP flags

	DWORD	dwChallenge;	// Unique identifier for UDP requests
	ULONG	ulPingTime;		// Ping time
	USHORT	uPingCount;		// Pings counter
	ULONG	ulLastPingTime;	// Last time when we ping'ed server
	USHORT	uFailedCount;	// Number of failed connection tries

	DWORD	dwFilesNum;		// Number of files
	DWORD	dwUsersNum;		// Number of users
	DWORD	dwMaxUsers;		// Maximum users
	DWORD	dwSoftMaxFiles;	// Soft files limit
	DWORD	dwHardMaxFiles;	// Hard files limit

	DWORD	dwCreditChangeTime;	// Last time we changed credits
	USHORT	uCreditsLeft;	// Credits left
};

class CServer : public CLoggable2
{
public:
	// Constructor
	CServer(CServerKey *pKey = NULL, CServerData *pData = NULL);

	// Operations
	static bool Find(ULONG ulAddr, USHORT uPort, CServer *pFound);
	static bool Find(CString sDynIP, USHORT uPort, CServer *pFound);
	bool Save();
	//	bool Delete();

	// Import
	static bool ImportFromServerMet(CString sFile);

	// List operations
	bool ListStartByAddr();
	bool ListStartByPriority();
	bool ListStartByStatic();
	bool ListGetNext();
	bool ListFinish();

	// Properties
	ULONG	GetAddress();
	USHORT	GetPort();
	CString GetName();
	CString GetStrAddress();
	CString GetDynIP();

	// Index callbacks
/*	static int PrimaryIndex(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey);
	static int SecondaryIndexPriority(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey);
	static int SecondaryIndexStatic(Db *pDb, const Dbt *pKey, const Dbt *pData, Dbt *pNewKey);
*/
	// Data
	CServerKey	m_Key;
	CServerData	m_Data;

private:
	Dbc	*m_pCursor;
};
#endif //USE_BERKELEY_DB
