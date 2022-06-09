// EmEngineBase.h: interface for the CEmEngineBase class.
//
//////////////////////////////////////////////////////////////////////

#pragma once


#include <vector>
#include <map>
#include <queue>

#include "../Other/Time.h"
#include "../Other/Loggable2.h"

// Client types
enum T_CLIENT_TYPE {
	T_CLIENT_PEER,
	T_CLIENT_SERVER,
	T_CLIENT_PEER_SERVER,
	T_CLIENT_WEB,
	T_CLIENT_XML
};

using namespace std;


#define PCVOID LPCVOID
#ifndef _countof
#	define _countof(string) (sizeof((string)) / sizeof((string)[0]))
#endif // _countof
#pragma warning (disable : 4200)
#pragma warning (disable : 4355)


// Queue priorities
enum EnumQueuePriority
{
	QUE_IMMEDIATE	= 0,
	QUE_HIGH		= 1,
	QUE_NORMAL		= 2,
	QUE_LOW			= 3
};
const UINT QUEUE_SIZE = 4;

class CTcpEngine;
class CTaskProcessor_Sockets;
struct CEmClient_Peer;

class CEmEngineBase : public CLoggable2
{
public:
	CEmEngineBase();
	virtual ~CEmEngineBase() { Uninit(); }

	virtual bool Init();
	virtual void Uninit();
	virtual void UninitMiddle(){ }
	virtual void UninitFinal(){ }

	inline bool IsShuttingDown() 
	{
		return WAIT_TIMEOUT != WaitForSingleObject(m_hStop, 0); 
	}

	virtual void ProcessSocketsTimeout(){ }

	virtual bool AlertOnErrors() const	{ return true; }

	__declspec(property(get=_GetSocketsProcessor))	CTaskProcessor_Sockets&	Sockets;
	CTaskProcessor_Sockets&	_GetSocketsProcessor()	const { if(!m_pSocketsProcessor)throw;return *m_pSocketsProcessor; }

	virtual CTcpEngine*		GetTcpEngine()			const { if(!m_pTcpEngine) throw;	return reinterpret_cast<CTcpEngine*>(m_pTcpEngine); }

protected:
	CTaskProcessor_Sockets*	m_pSocketsProcessor;
	CTcpEngine*				m_pTcpEngine;
	HANDLE					m_hStop; // termination event
};
