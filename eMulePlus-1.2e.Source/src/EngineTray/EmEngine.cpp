#include "stdafx.h"
#include "../Engine/Sockets/TaskProcessorSockets.h"

//////////////////////////////////////////////////////////////////////
// Constructor
CEmEngine::CEmEngine() 
	:m_hWndUI(NULL)
	,m_eXmlState(XML_DISCONNECTED)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEmEngine::Init(HWND hWndUI)
{
	// Base class init
	if(!CEmEngineBase::Init())
		return false;

	m_hWndUI = hWndUI;

	m_XmlEvents.SetStopEvent(m_hStop);

	return true;
}

