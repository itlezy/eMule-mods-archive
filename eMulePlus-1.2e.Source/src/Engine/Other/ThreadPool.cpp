// ThreadPool.cpp: implementation of the CThreadPool class.
//
//////////////////////////////////////////////////////////////////////

#include "StdAfx.h"

#include <process.h>
#include "ThreadPool.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CThreadPool::~CThreadPool()
{
	UninitPool();
}

DWORD CThreadPool::InitPool(DWORD dwThreads /* = 0 */, DWORD dwMaxConcurrent /* = 0 */)
{
	ASSERT(!m_hIoCompletionPort && !m_dwThreads & !m_pThreads);

	if (!CEmWinNT::s_stWinNT.IsInitialized())
		return 0; // not running on windows NT. Cannot create the thread pool.

	if (!dwThreads)
	{
		// Determine the number of processors on this machine
		SYSTEM_INFO stSysInfo;
		GetSystemInfo(&stSysInfo);
		dwThreads = stSysInfo.dwNumberOfProcessors;
	}

	m_hIoCompletionPort = CEmWinNT::s_stWinNT.m_pfnCreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, dwMaxConcurrent);
	if (!m_hIoCompletionPort)
		return 0;

	// Create now threads.
	m_pThreads = new HANDLE[dwThreads];
	if (m_pThreads)
	{
		for (; m_dwThreads < dwThreads; m_dwThreads++)
		{
			UINT nThreadID;
			m_pThreads[m_dwThreads] = (HANDLE) _beginthreadex(NULL, 0, CompletionFunc, this, 0, &nThreadID);
			if (!m_pThreads[m_dwThreads])
				break;

		}

		if (!m_dwThreads)
		{
			// cannot create even one thread!
			delete[] m_pThreads;
			m_pThreads = NULL;
		}
	}

	return m_dwThreads;
}

void CThreadPool::UninitPool(DWORD dwMaxWaitTimeout /* = 500 */)
{
	if (m_hIoCompletionPort)
	{
		if (m_pThreads)
		{
			if (m_dwThreads)
			{
				// post all threads instruction to terminate
				for (DWORD dwThread = 0; dwThread < m_dwThreads; dwThread++)
					VERIFY(CEmWinNT::s_stWinNT.m_pfnPostQueuedCompletionStatus(m_hIoCompletionPort, 0, 0xFFFFFFFF, NULL));

				BOOL bAllShutDown = TRUE;
				if (m_dwThreads <= MAXIMUM_WAIT_OBJECTS)
				{
					DWORD dwRes = WaitForMultipleObjects(m_dwThreads, m_pThreads, TRUE, dwMaxWaitTimeout);
					bAllShutDown = (dwRes >= WAIT_OBJECT_0) && (dwRes < WAIT_OBJECT_0 + m_dwThreads);
				}
				else
				{
					// because there is a huge amount of threads - we can't use WaitForMultipleObjects.
					// Ensure manually that threads are terminated
					DWORD dwTicksBeg = GetTickCount();
					for (dwThread = 0; dwThread < m_dwThreads; dwThread++)
					{
						DWORD dwWait = GetTickCount() - dwTicksBeg;
						if (dwWait < dwMaxWaitTimeout)
							dwWait = dwMaxWaitTimeout - dwWait;
						else
							dwWait = 0;

						if (WAIT_OBJECT_0 != WaitForSingleObject(m_pThreads[dwThread], dwWait))
							bAllShutDown = FALSE;
					}
				}

				for (dwThread = 0; dwThread < m_dwThreads; dwThread++)
				{
					if (!bAllShutDown && (WAIT_OBJECT_0 != WaitForSingleObject(m_pThreads[dwThread], 0)))
						VERIFY(TerminateThread(m_pThreads[dwThread], -1));

					VERIFY(CloseHandle(m_pThreads[dwThread]));
				}

				m_dwThreads = 0;
			}

			delete[] m_pThreads;
			m_pThreads = NULL;
		}

		VERIFY(CloseHandle(m_hIoCompletionPort));
		m_hIoCompletionPort = NULL;
	}
}

UINT WINAPI CThreadPool::CompletionFunc(PVOID pPtr)
{
	CThreadPool* pThis = (CThreadPool*) pPtr;
	ASSERT(pThis);
	if (pThis)
		while (true)
		{
			DWORD dwBytes = 0, dwKey = 0;
			OVERLAPPED* pOverlapped = NULL;

			BOOL bRes = CEmWinNT::s_stWinNT.m_pfnGetQueuedCompletionStatus(pThis->m_hIoCompletionPort, &dwBytes, &dwKey, &pOverlapped, pThis->m_dwWaitTimeout);
			if (bRes && !dwBytes && !pOverlapped && (0xFFFFFFFF == dwKey))
				break;

			pThis->ProcessCompletion(bRes, dwBytes, dwKey, pOverlapped);
		}
	return 0;
}


//////////////////////////////////////////////////////////////////////
// Implementation of CDataQueue
//////////////////////////////////////////////////////////////////////


bool CDataQueue::Push(PVOID pData, DWORD dwDataSize)
{
	if (!IsInitialized())
	{
		ASSERT(FALSE);
		return false; // not initialized!
	}

	// allocate a new tail
	TAIL* pTail = new (dwDataSize) TAIL;
	ASSERT(pTail);
	if (!pTail)
		return false;

	pTail->m_dwSize = dwDataSize;
	CopyMemory(pTail->m_pData, pData, dwDataSize);

	CPtrQueue<TAIL>::Push(pTail);
	return true;
}
