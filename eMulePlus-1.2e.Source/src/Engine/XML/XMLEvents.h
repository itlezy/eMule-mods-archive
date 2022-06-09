// XMLEvents.h: interface for the CXMLEvents class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

enum EnumXmlEvents
{
	XML_EVENT_SERVER
};

class CXMLEvents : public CLoggable2
{
public:
	// Control functions
	void SetStopEvent(HANDLE hStop){ m_hStop = hStop; }

	void SetXmlResponse(ULONG ulId, LPCTSTR sResponse);
	CString WaitForXmlResponse(ULONG ulId, DWORD dwTimeout = 5000);

	// Events subscriptions
	void SubscribeToEvent(SOCKET hSocket, EnumXmlEvents eType);
	void UnsubscribeFromAllEvents(SOCKET hSocket);

	// Events
	void Fire_OnConnectedToServer(DWORD dwAddr, USHORT uPort, long lClientID);
	void Fire_OnDisconnectedFromServer();

protected:
	void FireEventToSubscribed(EnumXmlEvents eType, CString sData);

protected:
	typedef map<ULONG, CString> XmlRespMap;
	XmlRespMap	m_stXmlResponses;
//	CMutex		m_mutexXmlResponses;
	HANDLE		m_hStop;

	typedef multimap<EnumXmlEvents, SOCKET>	EventSocketMap;
	typedef pair<EnumXmlEvents, SOCKET> EventSocketPair;
	EventSocketMap	m_stEventMap;
};