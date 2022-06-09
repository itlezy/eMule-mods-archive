// Implementation of all virtual functions of WebServer class
// e.g. implementation of eMule Plus web-interface

#include "StdAfx.h"
#include "WebServerImpl.h"
#include "../../opcodes.h"
#include "../../Engine/Sockets/TasksSockets.h"
#include "../../Engine/XML/XML.h"
#include <time.h>

void CWebServerImpl::ProcessDynamicItem(XmlDoc spDoc, const CString sItem, const vector<CString> arrParams, const CString sArgs)
{
	CString sInternalCmd = ReadParam(sArgs, _T("internal_op"));
	if(!sInternalCmd.IsEmpty())
	{
		if(sInternalCmd == _T("connect"))
		{
			CString sAddr = ReadParam(sArgs, _T("addr"));
			UINT nPort = _ttol(ReadParam(sArgs, _T("port")));

			CXml_hello *pCompletionTask = new CXml_hello;
			pCompletionTask->m_sClient = _T("tray");
			CTask_Connect *pTask = new CTask_Connect(sAddr, nPort, T_CLIENT_XML, pCompletionTask);
			g_stEngine.Sockets.Push(pTask);
		}

		if(sInternalCmd == _T("exit"))
		{
			CXml_quit* pTask = new CXml_quit;
			pTask->SetClient(g_stEngine.m_pXmlClient);
			g_stEngine.Sockets.Push(pTask);
		}

		if(sInternalCmd == _T("shutdown"))
		{
			g_stEngine.CloseApp();
			return;
		}

		XmlSetAttribute(spDoc->documentElement, _T("refresh"), 1);

		return;
	}

	if(sItem == _T("header"))
	{
		XmlElement spHeader = XmlCreateElement(spDoc, spDoc->documentElement, _T("header"));
		XmlSetAttribute(spHeader, _T("version"), (long)CURRENT_PLUS_VERSION);
		XmlSetAttribute(spHeader, _T("version_text"), CURRENT_VERSION_LONG);
		return;
	}

	if(sItem == _T("logs"))
	{
		if(g_stEngine.GetXmlState() != XML_CONNECTED)
			return;

		DWORD dwLogMin, dwLogMax;
		if(arrParams.size() > 1)
		{
			if(arrParams[1] == _T("debug"))
			{
				dwLogMin = LOG_DEBUG;
				dwLogMax = LOG_END;
			}
			else // "all"
			{
				dwLogMin = LOG_START;
				dwLogMax = LOG_END;
			}
		}
		else
		{
			dwLogMin = LOG_START;
			dwLogMax = LOG_NORMAL_END;
		}


		XmlElement spLogs = XmlCreateElement(spDoc, spDoc->documentElement, _T("logs"));

		CXml_get_logs *pTask = new CXml_get_logs;
		if(pTask)
		{
			pTask->m_dwTypeMin = dwLogMin;
			pTask->m_dwTypeMax = dwLogMax;
			pTask->SetClient(g_stEngine.m_pXmlClient);
			ULONG ulId = pTask->GetId();
			g_stEngine.Sockets.Push(pTask);
			CString sXml = g_stEngine.m_XmlEvents.WaitForXmlResponse(ulId);
			XmlDoc spXmlDoc = XmlLoadDocumentFromStr(sXml);
			if(spXmlDoc != NULL)
			{
				XmlNodes spNodes = spXmlDoc->selectNodes(_T("//result/log"));
				for(int i = 0; i < spNodes->length; i++)
				{
					XmlElement spElem = spNodes->item[i];
					long lTime = XmlGetAttributeLong(spElem, _T("time"), 0);
					if(lTime > 0)
					{
						short nPrecision = XmlGetAttributeLong(spElem, _T("precision"), 0);
						TCHAR sTime[128], sOutTime[128];
						_tcsftime(sTime, 128, _T("%d/%m/%y %H:%M:%S"), localtime(&lTime));
						_stprintf(sOutTime, _T("%s.%03d"), sTime, nPrecision);
						XmlSetAttribute(spElem, _T("time_text"), sOutTime);
					}
					spLogs->appendChild(spElem);
				}
			}
		}
		return;
	}
	if(sItem == _T("debug"))
	{
		CString strCmd = ReadParam(sArgs, _T("debug_cmd"));
		if(!strCmd.IsEmpty())
		{
			CXmlTask* pTask = CXmlTask::ParseXml(strCmd);
			if(pTask)
			{
				pTask->SetReceived(false);
				pTask->SetClient(g_stEngine.m_pXmlClient);
				g_stEngine.Sockets.Push(pTask);		
			}
		}
		return;
	}
	if(sItem == _T("shared"))
	{
		if(g_stEngine.GetXmlState() != XML_CONNECTED)
			return;

		XmlElement spShared = XmlCreateElement(spDoc, spDoc->documentElement, _T("shared"));

		CXml_get_shared *pTask = new CXml_get_shared;
		if(pTask)
		{
			pTask->SetClient(g_stEngine.m_pXmlClient);
			ULONG ulId = pTask->GetId();
			g_stEngine.Sockets.Push(pTask);
			CString sXml = g_stEngine.m_XmlEvents.WaitForXmlResponse(ulId);
			XmlDoc spXmlDoc = XmlLoadDocumentFromStr(sXml);
			if(spXmlDoc != NULL)
			{
				XmlNodes spNodes = spXmlDoc->selectNodes(_T("//result/file"));
				for(int i = 0; i < spNodes->length; i++)
				{
					XmlElement spElem = spNodes->item[i];
					spShared->appendChild(spElem);
				}
			}
		}
		return;
	}
}

void CWebServerImpl::ProcessFinalize(XmlDoc spDoc, const vector<CString> arrParams, const CString sArgs)
{
	XmlSetAttribute(spDoc->documentElement, _T("connected"), g_stEngine.GetXmlState() == XML_CONNECTED);
	if(g_stEngine.GetXmlState() != XML_CONNECTED)
	{
		XmlSetAttribute(spDoc->documentElement, _T("def_addr"), (LPCTSTR)DEF_ADDR);
		XmlSetAttribute(spDoc->documentElement, _T("def_port"), DEF_PORT);
	}
#ifdef _DEBUG
	XmlSetAttribute(spDoc->documentElement, _T("debug"), true);
#endif //_DEBUG
}
