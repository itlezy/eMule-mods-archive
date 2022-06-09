// ThreadPool.h: interface for the CThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#ifdef NEW_SOCKETS
#if !defined(AFX_THREADPOOL_H__92402536_5AEB_467C_9302_98CD441A7981__INCLUDED_)
#define AFX_THREADPOOL_H__92402536_5AEB_467C_9302_98CD441A7981__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "EmMt.h"

class CThreadPool  
{
	HANDLE m_hIoCompletionPort;
	HANDLE* m_pThreads;
	DWORD m_dwThreads;

	static UINT WINAPI CompletionFunc(PVOID);

protected:
	virtual void ProcessCompletion(BOOL bResult, DWORD dwBytes, DWORD dwKey, OVERLAPPED*) = NULL;

	DWORD m_dwWaitTimeout; // infinite by default

public:
	
	CThreadPool() :
		m_hIoCompletionPort(NULL),
		m_pThreads(NULL),
		m_dwThreads(0),
		m_dwWaitTimeout(INFINITE)
	{}
	virtual ~CThreadPool();

	DWORD InitPool(DWORD dwThreads = 0, DWORD dwMaxConcurrent = 0);
	void UninitPool(DWORD dwMaxWaitTimeout = 1000);

	inline HANDLE zget_CompletionPort() const { return m_hIoCompletionPort; }
	__declspec (property(get=zget_CompletionPort)) HANDLE _CompletionPort;

	inline DWORD zget_Threads() const { return m_dwThreads; }
	__declspec (property(get=zget_Threads)) DWORD _Threads;
};

template <class T>
class CPtrQueue {
	CCriticalSection_INL m_csLock;
	HANDLE m_hSemaphore; // semaphore
	std::queue<T*> m_queueData;

	T* FetchFront()
	{
		CCriticalSection_INL::CScope stScope(m_csLock);
		ASSERT(!m_queueData.empty());

		T* pObject = m_queueData.front();
		m_queueData.pop();

		return pObject;
	}
public:

	CPtrQueue() : m_hSemaphore(NULL) {}
	~CPtrQueue() { Uninit(); }

	bool Init()
	{
		Uninit();

		m_hSemaphore = CreateSemaphore(NULL, 0, 0xFFFF, NULL);
		return NULL != m_hSemaphore;
	}
	void Uninit()
	{
		if (m_hSemaphore)
		{
			VERIFY(CloseHandle(m_hSemaphore));
			m_hSemaphore = NULL;
		}

		CCriticalSection_INL::CScope stScope(m_csLock);

		for (; !m_queueData.empty(); m_queueData.pop())
		{
			T* pObject = m_queueData.front();
			if (pObject)
				delete pObject;
		}
	}
	bool IsInitialized() { return NULL != m_hSemaphore; }

	void Push(T* pObject)
	{
		ASSERT(m_hSemaphore);	// LOL, now is this confusing or what?

		m_csLock.Enter();
		m_queueData.push(pObject);
		m_csLock.Leave();

		VERIFY(ReleaseSemaphore(m_hSemaphore, 1, NULL));
	}
	T* Pop(DWORD dwTimeout = INFINITE)
	{
	//	If the queue is empty, wait for the next push until the timeout. If the queue isn't
	//		empty or it's fed in time, return the next queue entry...
		if (m_hSemaphore && (WAIT_OBJECT_0 == WaitForSingleObject(m_hSemaphore, dwTimeout)))
			return FetchFront();
		return NULL;
	}
	T* PopEx(HANDLE hInterrupt, DWORD dwTimeout = INFINITE)
	{
		if (m_hSemaphore)
		{
			HANDLE pWait[] = { m_hSemaphore, hInterrupt };
		//	If the queue is empty or 'hInterrupt' is signaled, wait until the timeout
		//		for the next push or for 'hInterrupt' to become unsignaled . If the queue isn't
		//		empty or it's fed in time, return the next queue entry...
			if (WAIT_OBJECT_0 == WaitForMultipleObjects(2, pWait, FALSE, dwTimeout))
				return FetchFront();
		}
		return NULL;
	}
};

struct TAIL
{
	DWORD m_dwSize;
	char m_pData[0]; // variable length
	inline void* operator new (size_t nSize, DWORD dwSizeExtra)
	{
		return malloc(nSize + dwSizeExtra);
		nSize += dwSizeExtra;
	}
	inline void operator delete (void* pPtr)
	{
		free(pPtr);
	}
};

class CDataQueue : public CPtrQueue<TAIL> {

public:

	bool Push(PVOID pData, DWORD dwDataSize);
};

#endif // !defined(AFX_THREADPOOL_H__92402536_5AEB_467C_9302_98CD441A7981__INCLUDED_)
#endif