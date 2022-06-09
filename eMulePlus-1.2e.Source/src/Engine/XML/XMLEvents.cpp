#include "stdafx.h"
#include "XMLEvents.h"
#include "../Sockets/TasksSockets.h"
#include "XML.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CXMLEvents::SetXmlResponse(ULONG ulId, LPCTSTR sResponse)
{
	//	CMutexLock stLock(m_mutexXmlResponses, true);
	m_stXmlResponses[ulId] = sResponse;
	//	stLock.Unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This can be implemented better
CString CXMLEvents::WaitForXmlResponse(ULONG ulId, DWORD dwTimeout)
{
	CPreciseTime tmStart = CPreciseTime::GetCurrentTime();
	while(CPreciseTime::GetCurrentTime() - tmStart < (long)dwTimeout)
	{
		if(WaitForSingleObject(m_hStop, 100) != WAIT_TIMEOUT)
			return _T("");
		//		CMutexLock stLock(m_mutexXmlResponses, true);
		XmlRespMap::iterator it = m_stXmlResponses.find(ulId);
		if(it != m_stXmlResponses.end())
		{
			CString sRes = m_stXmlResponses[ulId];
			m_stXmlResponses.erase(ulId);
			//			stLock.Unlock();
			return sRes;
		}
		//		stLock.Unlock();
	}
	return _T("");
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CXMLEvents::SubscribeToEvent(SOCKET hSocket, EnumXmlEvents eType)
{
	m_stEventMap.insert(EventSocketPair(eType, hSocket));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CXMLEvents::FireEventToSubscribed(EnumXmlEvents eType, CString sData)
{
	pair<EventSocketMap::iterator, EventSocketMap::iterator> 
		itRange = m_stEventMap.equal_range(eType);
	for(EventSocketMap::iterator it = itRange.first; it != itRange.second; it++)
	{
		CTask_SendXml* pTask = new CTask_SendXml(it->second, sData);
		if(pTask)
			g_stEngine.Sockets.Push(pTask);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CXMLEvents::Fire_OnConnectedToServer(DWORD dwAddr, USHORT uPort, long lClientID)
{
	XmlDoc spDoc = CXmlTask::PrepareDocument(_T("server_connected"), _T("event"));
	if(spDoc) 
	{
		XmlElement spElem = spDoc->documentElement;
		XmlSetAttribute(spElem, _T("Addr"), (long)dwAddr);
		XmlSetAttribute(spElem, _T("Port"), (long)uPort);
		XmlSetAttribute(spElem, _T("ClientID"), lClientID);
		FireEventToSubscribed(XML_EVENT_SERVER, (LPCTSTR)(spDoc->xml));
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CXMLEvents::Fire_OnDisconnectedFromServer()
{
	XmlDoc spDoc = CXmlTask::PrepareDocument(_T("server_disconnected"), _T("event"));
	if(spDoc) 
	{
		FireEventToSubscribed(XML_EVENT_SERVER, (LPCTSTR)(spDoc->xml));
	}
}

