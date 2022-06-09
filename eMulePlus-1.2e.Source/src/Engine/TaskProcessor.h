// TaskProcessor.h: interface for the CTaskProcessor class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_TASKPROCESSOR_H__186018D8_26C9_4A91_9F7C_DDC406696088__INCLUDED_)
#define AFX_TASKPROCESSOR_H__186018D8_26C9_4A91_9F7C_DDC406696088__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "Other/ThreadPool.h"

// Basic task definition
struct CTask : public CLoggable2 {
	virtual ~CTask();
	virtual bool Process() = 0;
	virtual bool OnException();		// Exception handling
	virtual LPCTSTR TaskName() = 0;	// Task name for debug purposes
};

// Useful for tasks states
enum
{
	TASK_START	= 0,
	TASK_READ	= 1,
	TASK_SEND	= 2
};

struct CStateMachine
{
	CStateMachine() : m_byteState(TASK_START){ }
	__declspec(property(get=_GetState, put=_PutState)) BYTE State;
	BYTE _GetState() const { return m_byteState; }
	void _PutState(BYTE byteState){ m_byteState = byteState; }
private:
	BYTE m_byteState;
};

class CTaskProcessor : public CLoggable2
{
	HANDLE m_hThread;
	HANDLE m_hStop;

	CTask* m_pCurrent;

	struct THREAD_INIT_PARAM
	{
		CTaskProcessor	   *m_pThis;
		HANDLE				m_hInitEvent;
		bool				m_bInitResult;
	};

	static UINT WINAPI ProcessingFunc(PVOID pPtr);

	CPtrQueue<CTask> m_queueTasks;

	virtual bool Start();
	virtual void Stop();
	virtual void ProcessTimeout();
	bool ProcessTask(CTask&);

protected:
	DWORD m_dwWaitTimeout;
	DWORD m_dwStartupTimeout;

public:

	bool Init();
	void Uninit();

	CTaskProcessor() :
		 m_hThread(NULL)
		,m_hStop(NULL)
		,m_pCurrent(NULL)
		,m_dwWaitTimeout(INFINITE)
		,m_dwStartupTimeout(1000)
	{}
	~CTaskProcessor() { Uninit(); };

	inline void Push(CTask* pTask) 
	{
		if (pTask) 
			m_queueTasks.Push(pTask); 
		else
		{
			ASSERT(FALSE);
			AddDebugLogLine(_T("no memory"));
		}
	}
	inline bool IsInContext(const CTask& stTask) { return &stTask == m_pCurrent; }
};

class CTaskProcessor_Main : public CTaskProcessor
{
	virtual bool Start(){ m_dwWaitTimeout = INFINITE; return true;	}
	virtual void Stop(){ }
	virtual void ProcessTimeout(){ }

public:
	CTaskProcessor_Main(){ }
};

#endif // !defined(AFX_TASKPROCESSOR_H__186018D8_26C9_4A91_9F7C_DDC406696088__INCLUDED_)
