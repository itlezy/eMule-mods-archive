#pragma once


#include "../Engine/Base/EmEngineBase.h"
#include "../Engine/XML/XMLEvents.h"

#define WM_UI_STATE	(WM_USER + 1001)

enum XmlStates
{
	XML_CONNECTED,
	XML_DISCONNECTED
};

struct CEmClient_Xml;

class CEmEngine : public CEmEngineBase
{
public:
	CEmClient_Xml*	m_pXmlClient;

	CEmEngine();

	bool Init(HWND hWndUI);

	inline void SetXmlState(XmlStates eState, DWORD dwData = 0)
	{
		m_eXmlState = eState;
		if(m_hWndUI)
			::PostMessage(m_hWndUI, WM_UI_STATE, (WPARAM)eState, (LPARAM)dwData);
	}
	inline XmlStates GetXmlState(){ return m_eXmlState; }


	HWND m_hMainWnd;
	void CloseApp(){ PostMessage(m_hMainWnd, WM_COMMAND, IDCANCEL, 0); }

	CXMLEvents	m_XmlEvents;

private:
	XmlStates	m_eXmlState;
	HWND		m_hWndUI;
};
