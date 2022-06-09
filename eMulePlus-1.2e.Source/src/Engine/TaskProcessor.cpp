// TaskProcessor.cpp: implementation of the CTaskProcessor class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TaskProcessor.h"
#include <process.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CTaskProcessor::Init()
{
	Uninit(); // if already initialized

//	Initialize the Task Queue. If successful...
	if (m_queueTasks.Init())
	{
	//	Create an Event which can be asserted to stop the processor. If successful...
		if (m_hStop = CreateEvent(NULL, TRUE, FALSE, NULL))
		{
			THREAD_INIT_PARAM		stParam;

			stParam.m_pThis = this;
			stParam.m_bInitResult = false;
		//	Create an Event which, when asserted, will signal that the thread is initialized.
		//	If successful...
			if (stParam.m_hInitEvent = CreateEvent(NULL, TRUE, FALSE, NULL))
			{
				UINT nThread = 0;

				if (m_hThread = (HANDLE) _beginthreadex(NULL, 0, ProcessingFunc, &stParam, 0, &nThread))
				{
					DWORD dwRes = WaitForSingleObject(stParam.m_hInitEvent, m_dwStartupTimeout);

					if (WAIT_OBJECT_0 == dwRes)
					{
						if (stParam.m_bInitResult)
							return true;
						else
							AddLog(LOG_ERROR, _T("Task processor failed to initialize"));
					}
					else
					{
						if (WAIT_TIMEOUT == dwRes)
							AddLog(LOG_ERROR, _T("Task processor initialization timed out"));
						else
							AddLog(LOG_ERROR, _T("WaitForSingleObject failed"));
					}
				} 
				else
					AddLog(LOG_ERROR, _T("_beginthreadex failed"));
			} 
			else
				AddLog(LOG_ERROR, _T("CreateEvent failed"));
		} 
		else
			AddLog(LOG_ERROR, _T("CreateEvent failed"));
	}
	else
		AddLog(LOG_ERROR, _T("Failed to create the queue"));

	Uninit();
	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// destruction
void CTaskProcessor::Uninit()
{
	if (m_hStop)
	{
		if (m_hThread)
		{
			if (!SetEvent(m_hStop))
				AddLog(LOG_ERROR, _T("SetEvent failed"));

			if (WAIT_TIMEOUT == WaitForSingleObject(m_hThread, 2000))
			{
				AddLog(LOG_ERROR, _T("Failed to uninitialize. Terminating"));
				if (!TerminateThread(m_hThread, -1))
					AddLog(LOG_ERROR, _T("TerminateThread failed"));
			}

			if (!CloseHandle(m_hThread))
				AddLog(LOG_ERROR, _T("CloseHandle failed"));
			m_hThread = NULL;
		}

		if (!CloseHandle(m_hStop))
			AddLog(LOG_ERROR, _T("CloseHandle failed"));

		m_hStop = NULL;
	}
	m_queueTasks.Uninit();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// processing
UINT WINAPI CTaskProcessor::ProcessingFunc(PVOID pPtr)
{
	THREAD_INIT_PARAM* pParam = (THREAD_INIT_PARAM*)pPtr;

	if (pParam != NULL)
	{
		CoInitialize(NULL);

		CTaskProcessor*	pThis = pParam->m_pThis;
		bool			bRes  = pParam->m_bInitResult = pThis->Start();

		SetEvent(pParam->m_hInitEvent);

		if (bRes)
		{
			for (DWORD dwWaitLast = GetTickCount(); ; )
			{
				DWORD dwWait = INFINITE;

			//	If a timeout has been specified...
				if (INFINITE != pThis->m_dwWaitTimeout)
				{
					DWORD dwElapsed = GetTickCount() - dwWaitLast;

				//	If less than the timeout period has elapsed...
					if (dwElapsed < pThis->m_dwWaitTimeout)
					{
						dwWait = pThis->m_dwWaitTimeout - dwElapsed;
					}
					else
					{
						pThis->ProcessTimeout();
						dwWaitLast = GetTickCount();
						dwWait = pThis->m_dwWaitTimeout;
					}
				}
			//	MOREVIT: Ah grasshopper. Completely inscrutable are we? Is it really helpful to hide the 
			//		synchronization code this way?

			//	Get the next Task or wait up to 'dwWait' milliseconds for one to become available or for
			//		the processor to be stopped.
				CTask* pTask = pThis->m_queueTasks.PopEx(pThis->m_hStop, dwWait); 

			//	If the processor hasn't been stopped and a Task became available before the timeout...
				if (pTask != NULL)
				{
					pThis->m_pCurrent = pTask;
					if (pThis->ProcessTask(*pTask))
					{
						delete pTask;
					}
					pThis->m_pCurrent = NULL;
				}
				else
				{
					if (g_stEngine.IsShuttingDown())
						break; // finished!
				}
			}
		}

		pThis->Stop();
	}
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// those functions are defined in child classes
bool CTaskProcessor::Start()
{
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTaskProcessor::Stop()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CTaskProcessor::ProcessTimeout()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// process single task
bool CTaskProcessor::ProcessTask(CTask& stTask)
{
	TCHAR strTaskName[100];
	try
	{
		_tcscpy(strTaskName, stTask.TaskName());
		return stTask.Process();
	}
	catch (_com_error& e)
	{
		AddLog(LOG_ERROR, _T("ATL Exception while processing task: Source=%s; Description=%s"), (LPCTSTR) e.Source(), (PCTSTR) e.Description());
	}
	catch (...)
	{
		AddLog(LOG_ERROR, _T("Unhandled exception while processing task %s"), strTaskName);
	}

	try
	{
		// give it another chance
		return stTask.OnException();
	}
	catch(_com_error& e)
	{
		AddLog(LOG_ERROR, _T("Second ATL exception while processing task: Source=%s; Description=%s"), (LPCTSTR) e.Source(), (PCTSTR) e.Description());
	}
	catch (...)
	{
		AddLog(LOG_ERROR, _T("Second unhandled exception while processing task %s"), strTaskName);
	}

	return true; // delete this buggy task!
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
