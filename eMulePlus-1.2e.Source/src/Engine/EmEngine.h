// EmEngine.h: interface for the CEmEngine class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include "Base/EmEngineBase.h"

class CServer;
struct CTask;
struct CEmClient;
struct CEmClient_Server;
struct CTcpCompletionTask;
class COpCode;
class CTaskProcessor_Main;
class CTaskProcessor_Files;
class CTaskProcessor_Logger;
class CTaskProcessor_DB;
class CPrefs;
class CClientList;
class CTcpEngineMule;
class CXMLEvents;

enum EnumServerStates
{
	SERVER_DISCONNECTED,
	SERVER_CONNECTING,
	SERVER_WAITFORLOGIN,
	SERVER_CONNECTED
};

// Server TCP flags
const UINT SRV_TCPFLG_COMPRESSION		= 0x00000001;
const UINT SRV_TCPFLG_NEWTAGS			= 0x00000008;	//	Server accepts newtags (16.46+)
const UINT SRV_TCPFLG_UNICODE			= 0x00000010;
const UINT SRV_TCPFLG_EXT_GETSOURCES	= 0x00000020;	//	Server accepts OP_GETSOURCES containing serveral files, plus <HASH 16><SIZE 4>

// Server UDP flags
const UINT SRV_UDPFLG_EXT_GETSOURCES	= 0x00000001;	//	Server accepts the UDP AskSource coalescing (several files in one OP_GLOBGETSOURCES)
const UINT SRV_UDPFLG_EXT_GETFILES		= 0x00000002;
const UINT SRV_UDPFLG_NEWTAGS			= 0x00000008;	//	Server accepts newtags (16.46+)
const UINT SRV_UDPFLG_UNICODE			= 0x00000010;
const UINT SRV_UDPFLG_EXT_GETSOURCES2	= 0x00000020;	//	Server accepts OP_GLOBGETSOURCES2

struct ServerState_Struct
{
	SOCKET		hSocket;
	DWORD		dwAddr;
	USHORT		uPort;
	short		nConnState;
	long		nClientID;
	DWORD		dwServerFlags;

	bool IsConnecting()
	{
		return (nConnState == SERVER_CONNECTING	|| nConnState == SERVER_WAITFORLOGIN);
	}
	bool IsConnected()
	{ 
		return (nConnState == SERVER_CONNECTED);
	}
	bool IsLowID()
	{ 
		return (nClientID < 0x1000000); 
	}
};

class CEmEngine : public CEmEngineBase
{
public:
	CEmEngine();

	virtual bool Init();
	virtual void UninitMiddle();
	virtual void UninitFinal();

	// Public methods
	void ConnectToServer(CString sAddr = "", ULONG ulPort = 0);
	void ConnectToAnyServer();
	void DisconnectFromServer(SOCKET hPrevServer = NULL);

	// Data
	SOCKET GetServerSocket(){ return m_stServer.hSocket; }
	long GetClientID(){ return m_stServer.nClientID; }
	bool IsLocalServer(ULONG nAddr, USHORT uPort);

	// Update Server Info
	void SetConnectionState(short nConnectionState);
	void SetClientID(long cID);
	void ConnectedTo(CEmClient_Server* pClient);

	void SetServerSupportedFeatures(DWORD dwFlags)	{ m_stServer.dwServerFlags = dwFlags; }
	bool IsServerSupport(DWORD dwFeature)	const	{ return (m_stServer.dwServerFlags & dwFeature); }

	inline void ShutDown()
	{
		PostThreadMessage(m_dwThreadId, WM_QUIT, 0, 0);
	}

	bool SendOpCode(SOCKET hSocket, const COpCode &stOpCode, CEmClient* pClient, EnumQueuePriority ePriority);

	virtual void ProcessSocketsTimeout();

	virtual bool AlertOnErrors() const;

public:
	__declspec(property(get=_GetFilesProcessor))	CTaskProcessor_Files&	Files;
	__declspec(property(get=_GetPreferences))		CPrefs&					Prefs;
	__declspec(property(get=_GetClientList))		CClientList&			ClientList;
	__declspec(property(get=_GetServerState))		ServerState_Struct&		ServerState;
	__declspec(property(get=_GetTcpEngineMule))		CTcpEngineMule&			TcpEngineMule;
	__declspec(property(get=_GetDbProcessor))		CTaskProcessor_DB&		DB;
	__declspec(property(get=_GetXmlEvents))			CXMLEvents&				XmlEvents;

	CTaskProcessor_Files& 	_GetFilesProcessor()	const { if(!m_pFilesProcessor) throw;	return *m_pFilesProcessor; }
	CTaskProcessor_DB& 		_GetDbProcessor()		const { if(!m_pDbProcessor) throw;	return *m_pDbProcessor; }
	CPrefs& 				_GetPreferences()		const { if(!m_pPreferences) throw;		return *m_pPreferences; }
	CClientList&			_GetClientList()		const { if(!m_pClientList) throw;		return *m_pClientList; }
	ServerState_Struct&		_GetServerState()		      { return m_stServer; }
	CTcpEngineMule&			_GetTcpEngineMule()		const { if(!m_pTcpEngineMule) throw;	return *m_pTcpEngineMule; }
	CXMLEvents&				_GetXmlEvents()			const { if(!m_pXmlEvents) throw;		return *m_pXmlEvents; }

	virtual CTcpEngine*		GetTcpEngine()			const { if(!m_pTcpEngineMule) throw;	return reinterpret_cast<CTcpEngine*>(m_pTcpEngineMule); }


private:
	// Task processors
	CTaskProcessor_Main*	m_pMainProcessor;
	CTaskProcessor_Files*	m_pFilesProcessor;
	CTaskProcessor_Logger*	m_pLoggerProcessor;
	CTaskProcessor_DB*		m_pDbProcessor;

	CTcpEngineMule*			m_pTcpEngineMule;

	CClientList*			m_pClientList;

	CXMLEvents*				m_pXmlEvents;

	CPrefs*					m_pPreferences;

	// Low-level classes might have want to access our private vars
	friend class CLoggable;
	friend class COpCode;
	friend struct CTask_LogOpcode;

	ServerState_Struct	m_stServer;

	DWORD	m_dwThreadId;
};
