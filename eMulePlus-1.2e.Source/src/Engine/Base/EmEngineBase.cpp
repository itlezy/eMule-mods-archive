// EmEngineBase.cpp: implementation of the CEmEngineBase class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#include "EmEngineBase.h"
#include "../Sockets/TaskProcessorSockets.h"
#include "../Sockets/TcpEngine.h"

//////////////////////////////////////////////////////////////////////
CEmEngineBase::CEmEngineBase()
	:m_hStop(NULL)
	,m_pTcpEngine(NULL)
{
}

//////////////////////////////////////////////////////////////////////
bool CEmEngineBase::Init()
{
	// Create an Event which can be asserted to stop the Engine. If successful...
	if (m_hStop = CreateEvent(NULL, TRUE, FALSE, NULL))
	{
		//	Initialize the Sockets TaskProcessor
		m_pSocketsProcessor = new CTaskProcessor_Sockets;
		if (m_pSocketsProcessor->Init())
		{
			m_pTcpEngine = new CTcpEngine;

			//	Initialize the TCP Engine
			if (GetTcpEngine()->Init())
				return true;
			else
			{
				AddLog(LOG_ERROR, _T("Failed to initialize sockets engine"));
				return false;
			}
		}
		else
		{
			AddLog(LOG_ERROR, _T("Failed to initialize sockets processor"));
			return false;
		}
	}
	else
	{
		AddLog(LOG_ERROR, _T("CreateEvent failed"));
		return false;
	}
}

//////////////////////////////////////////////////////////////////////
void CEmEngineBase::Uninit()
{
	if (m_hStop)
	{
		if (!SetEvent(m_hStop))
			AddLog(LOG_ERROR, _T("SetEvent failed"));

		GetTcpEngine()->Uninit(false);
		m_pSocketsProcessor->Uninit();

		// To stop main engine
		UninitMiddle();	

		// Final sockets destroy
		GetTcpEngine()->Uninit();
		delete m_pSocketsProcessor;

		// To delete all the rest before destroying stop event
		UninitFinal();	

		delete m_pTcpEngine;

		if (!CloseHandle(m_hStop))
			AddLog(LOG_ERROR, _T("CloseHandle failed"));
		m_hStop = NULL;
	}
}
