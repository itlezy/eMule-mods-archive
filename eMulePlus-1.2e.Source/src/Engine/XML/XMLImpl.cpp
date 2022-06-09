#include "stdafx.h"
#include "../Sockets/TasksSockets.h"
#include "../Database/TaskProcessorDB.h"
#include "../Files/TaskProcessorFiles.h"
#include "../../SharedFileList.h"
#include "../Data/Prefs.h"
#ifdef USE_BERKELEY_DB
	#include "../../BerkeleyDb/build_win32/db_cxx.h"
#endif //USE_BERKELEY_DB

#include "XML.h"
#include "XMLEvents.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_FUNC(connect)
{
	g_stEngine.ConnectToServer(m_sAddr, m_dwPort);

	//PrepareResult();
	return true;
}
XML_RESULT_EMPTY(connect)
XML_EVENT_EMPTY(connect)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_FUNC(disconnect)
{
	g_stEngine.DisconnectFromServer();

	//PrepareResult();
	return true;
}
XML_RESULT_EMPTY(disconnect)
XML_EVENT_EMPTY(disconnect)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_FUNC(hello)
{
	// Meanwhile automatically subscribe to events
	g_stEngine.XmlEvents.SubscribeToEvent(m_pClient->m_hSocket, XML_EVENT_SERVER);

	SendBuf((LPCTSTR)PrepareResult()->xml);

	return true;
}
XML_RESULT_EMPTY(hello)
XML_EVENT_EMPTY(hello)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_FUNC(quit)
{
//	::MessageBox(NULL, _T("quit"), _T(""), MB_OK|MB_TOPMOST);

	SendBuf((LPCTSTR)PrepareResult()->xml);

	g_stEngine.ShutDown();

	return true;
}
XML_RESULT_EMPTY(quit)
XML_EVENT_EMPTY(quit)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_FUNC(get_logs)
{
	switch(m_nState)
	{
	case XML_GET_START:
		m_spDoc = PrepareResult();
		m_nState = XML_GET_READ;
		g_stEngine.DB.Push(this);
		return false;
	case XML_GET_READ:
		// Get all records from logs
		for(LogLineList::iterator it = g_stEngine.DB.m_LogsList.begin(); it != g_stEngine.DB.m_LogsList.end(); it++)
		{
			if(it->eType >= m_dwTypeMin && it->eType <= m_dwTypeMax)
			{
				XmlElement spElem = XmlCreateElement(m_spDoc, m_spDoc->documentElement, _T("log"));
				XmlSetAttribute(spElem, _T("time"),		it->tmTime.Time);
				XmlSetAttribute(spElem, _T("precision"),it->tmTime.Precision);
				XmlSetAttribute(spElem, _T("debug"),	it->eType);
				XmlSetAttribute(spElem, _T("text"),		(LPCTSTR)(it->sLine));
			}
		}
		m_nState = XML_GET_SEND;
		g_stEngine.Sockets.Push(this);
		return false;
	case XML_GET_SEND:
		SendBuf((LPCTSTR)m_spDoc->xml);
		return true;
	}
	return true;
}

/*
#ifdef USE_BERKELEY_DB
	switch(m_nState)
	{
	case XML_GET_START:
		m_nState = XML_GET_READ;
		m_spDoc = PrepareResult();
		g_stEngine.DB.Push(this);
		return false;
	case XML_GET_READ:
		m_nState = XML_GET_SEND;
		// Get all records from logs
		try 
		{
			Dbc *dbcp;
			g_stEngine.DB.Logs.cursor(NULL, &dbcp, 0);
			Dbt key;
			Dbt data;
			while (dbcp->get(&key, &data, DB_NEXT) == 0) 
			{
				long lTime = *((long*)key.get_data());
				BYTE *pData = (BYTE*)data.get_data();
				BOOL bDebug = *((BOOL*)pData);
				LPCTSTR sLine = (LPCTSTR)(pData + sizeof(BOOL));
				XmlElement spElem = XmlCreateElement(m_spDoc, m_spDoc->documentElement, _T("log"));
				XmlSetAttribute(spElem, _T("time"), lTime);
				XmlSetAttribute(spElem, _T("debug"), bDebug);
				XmlSetAttribute(spElem, _T("text"), sLine);
			}
			dbcp->close();
		}
		catch (DbException &dbe) 
		{
			AddLog(LOG_ERROR, "Xml problems: %s", dbe.what());
		}
		catch(...)
		{
			AddLog(LOG_ERROR, "Xml problems read log from database.");
		}
		g_stEngine.Sockets.Push(this);
		return false;
	case XML_GET_SEND:
		SendBuf((LPCTSTR)m_spDoc->xml);
		return true;
	}
#endif //USE_BERKELEY_DB
*/

XML_RESULT_EMPTY(get_logs)
XML_EVENT_EMPTY(get_logs)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_FUNC(get_shared)
{
	switch(m_nState)
	{
	case XML_GET_START:
		m_nState = XML_GET_READ;
		m_spDoc = PrepareResult();
		g_stEngine.Files.Push(this);
		return false;
	case XML_GET_READ:
		m_nState = XML_GET_SEND;
		// Get all shared files
		try 
		{
			for(POSITION pos = g_stEngine.Files.SharedFiles.m_mapSharedFiles.GetStartPosition(); pos != NULL; )
			{
				CCKey		bufKey;
				CKnownFile*	pKnownFile = NULL;
				g_stEngine.Files.SharedFiles.m_mapSharedFiles.GetNextAssoc(pos, bufKey, pKnownFile);
				if(pKnownFile && !pKnownFile->IsPartFile())
				{
					XmlElement spElem = XmlCreateElement(m_spDoc, m_spDoc->documentElement, _T("file"));
					XmlSetAttribute(spElem, _T("hash"), (LPCTSTR)HashToString(pKnownFile->GetFileHash()));
					XmlSetAttribute(spElem, _T("name"), (LPCTSTR)pKnownFile->GetFileName());
					XmlSetAttribute(spElem, _T("size"), pKnownFile->GetFileSize());
					XmlSetAttribute(spElem, _T("type"), pKnownFile->GetFileType());
					XmlSetAttribute(spElem, _T("link"), (LPCTSTR)pKnownFile->CreateED2kLink());
				}
			}
		}
		catch(...)
		{
			AddLog(LOG_ERROR, "Xml problems read shared files.");
		}
		g_stEngine.Sockets.Push(this);
		return false;
	case XML_GET_SEND:
		SendBuf((LPCTSTR)m_spDoc->xml);
		return true;
	}
	return true;
}
XML_RESULT_EMPTY(get_shared)
XML_EVENT_EMPTY(get_shared)

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_RESULT_EMPTY(set_param)
XML_EVENT_EMPTY(set_param)

XML_PROCESS_FUNC(set_param)
{
	switch(m_dwType)
	{
	case PARAM_UPLOAD_LIMIT:
		g_stEngine.Prefs.SetMaxUpload(m_dwValue);
		AddLog(LOG_DEBUG, _T("Set upload limit to %ld.%ld"), m_dwValue / 10, m_dwValue % 10);
		break;
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_EMPTY(server_connected)
XML_RESULT_EMPTY(server_connected)
XML_EVENT_EMPTY(server_connected)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XML_PROCESS_EMPTY(server_disconnected)
XML_RESULT_EMPTY(server_disconnected)
XML_EVENT_EMPTY(server_disconnected)
