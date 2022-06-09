// XML.cpp: implementation of the CXmlTask class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "../Sockets/TasksSockets.h"
#include "XML.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTask_SendXml::CTask_SendXml(SOCKET hSocket, CString sXml)
	:m_hSocket(hSocket)
	,m_sXml(sXml)
{ }

bool CTask_SendXml::Process()
{
	AddLog(LOG_DEBUG_XML, "Xml sent: %s", m_sXml);
	BYTE *pBuf = new BYTE[m_sXml.GetLength() + 4];
	CopyMemory(pBuf + sizeof(DWORD), (LPCTSTR)m_sXml, m_sXml.GetLength());
	*((DWORD*)pBuf) = m_sXml.GetLength();
	g_stEngine.Sockets.AllocSend(m_hSocket, pBuf, m_sXml.GetLength() + sizeof(DWORD), QUE_IMMEDIATE);
	delete[] pBuf;

	return true;
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
long CXmlTask::s_lNextId = 0;

CXmlTask::CXmlTask()
	:m_pClient(NULL)
	,m_ulId(++s_lNextId)
	,m_sName(_T(""))
	,m_sContents(_T(""))
	,m_bReceived(false)
	,m_bResult(false)
	,m_bEvent(false)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XmlDoc CXmlTask::PrepareDocument(LPCTSTR sTaskName, LPCTSTR sTag)
{
	EMULE_TRY
	XmlDoc spDoc = XmlCreateDocument();
	if(spDoc)
	{
		XmlElement spElem = XmlCreateElement(spDoc, NULL, sTag);
		if(spElem)
			XmlSetAttribute(spElem, _T("name"), sTaskName);
	}
	CString sXml = (LPCTSTR)(spDoc->xml);
	if (sXml.IsEmpty())
		throw CString(_T("Can't create XML"));
	return spDoc;
	EMULE_CATCH
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
XmlDoc CXmlTask::PrepareResult()
{
	EMULE_TRY
	XmlDoc spDoc = PrepareDocument(m_sName, _T("result"));
	if(spDoc)
	{
		XmlElement spElem = spDoc->documentElement;
		XmlSetAttribute(spElem, _T("id"), m_ulId);
		CString sXml = (LPCTSTR)(spDoc->xml);
		if (sXml.IsEmpty())
			throw CString(_T("Can't create XML"));
	}
	return spDoc;
	EMULE_CATCH
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CXmlTask::SendBuf(CString sXml)
{
	AddLog(LOG_DEBUG_XML, "Xml sent: %s", sXml);
	BYTE *pBuf = new BYTE[sXml.GetLength() + 4];
	CopyMemory(pBuf + sizeof(DWORD), (LPCTSTR)sXml, sXml.GetLength());
	*((DWORD*)pBuf) = sXml.GetLength();
	g_stEngine.Sockets.AllocSend(m_pClient->m_hSocket, pBuf, sXml.GetLength() + sizeof(DWORD), QUE_IMMEDIATE);
	delete[] pBuf;
}

//////////////////////////////////////////////////////////////////////

// Defining constructor
#include "XMLUndef.h"
#define BEGIN_XML_CMD(name)		CXml_##name::CXml_##name() { 
#define XML_PARAM_STRING(var)		m_s##var = _T("");
#define XML_PARAM_DWORD(var)		m_dw##var = 0;
#define XML_INTERNAL_VAR(var,init)	init;
#define END_XML_CMD				}
#include "XMLMsgs.h"

//////////////////////////////////////////////////////////////////////

// Defining LoadParams function
#include "XMLUndef.h"
#define BEGIN_XML_CMD(name)		bool CXml_##name::LoadParams(XmlElement spElem) { 
#define XML_PARAM_STRING(var)		m_s##var = XmlGetAttributeStr(spElem, _T(#var));
#define XML_PARAM_DWORD(var)		m_dw##var = XmlGetAttributeLong(spElem, _T(#var));
#define XML_INTERNAL_VAR(var,init)
#define END_XML_CMD				return true; }
#include "XMLMsgs.h"

//////////////////////////////////////////////////////////////////////

// Defining LoadParams function
#include "XMLUndef.h"
#define BEGIN_XML_CMD(name)		bool CXml_##name::Process() { \
									if(m_bEvent)	return ProcessEvent(); \
									if(m_bResult)	return ProcessResult(); \
									if(m_bReceived)	return ProcessReceived(); \
									XmlDoc spDoc = XmlCreateDocument(); \
									if(spDoc) { \
										XmlElement spElem = XmlCreateElement(spDoc, NULL, _T("cmd")); \
										if(spElem) { \
											XmlSetAttribute(spElem, _T("name"), _T(#name)); \
											XmlSetAttribute(spElem, _T("id"), m_ulId);
#define XML_PARAM_STRING(var)				XmlSetAttribute(spElem, _T(#var), (LPCTSTR)m_s##var);
#define XML_PARAM_DWORD(var)				XmlSetAttribute(spElem, _T(#var), m_dw##var);
#define XML_INTERNAL_VAR(var,init)
#define END_XML_CMD							SendBuf((LPCTSTR)(spDoc->xml)); \
										} \
									} \
								return true; }
#include "XMLMsgs.h"


CXmlTask* CXmlTask::ParseXml(CString sXml)
{
	try
	{
		AddLog(LOG_DEBUG_XML, "Xml received: %s", sXml);

		XmlDoc spDoc = XmlLoadDocumentFromStr(sXml);
		if(spDoc == NULL)
			throw CString(_T("Bad XML"));

		CString sCmd = _T("");
		ULONG ulId = 0;
	
		XmlElement spElem = spDoc->documentElement;
		if(spElem == NULL)
			throw CString(_T("Can't find main tag"));

		CString sType = (LPCTSTR)(spElem->tagName);

		sCmd = XmlGetAttributeStr(spElem, _T("name"));
		if(sCmd.IsEmpty())
			throw CString(_T("Error reading \"name\" parameter of main tag"));

		if(sType == _T("cmd"))
		{
			ulId = XmlGetAttributeLong(spElem, _T("id"), 0);
			if(!ulId)
				throw CString(_T("Error reading \"id\" parameter of <cmd> tag"));
		}
		else if(sType == _T("result"))
		{
			ulId = XmlGetAttributeLong(spElem, _T("id"), 0);
			if(!ulId)
				throw CString(_T("Error reading \"id\" parameter of <result> tag"));
		}
		else if(sType == _T("event"))
		{
		}

/*		// Is it result? No?
		XmlElement spRes = spDoc->selectSingleNode(_T("./result"));
		if(spRes == NULL)
		{
			// Get basic parameters - <cmd name="xxx" id="nnn" ... />
			XmlElement spElem = spDoc->selectSingleNode(_T("./cmd"));
			if(spElem == NULL)
				throw CString(_T("Can't find <cmd> tag"));
			sCmd = XmlGetAttributeStr(spElem, _T("name"));
			if(sCmd.IsEmpty())
				throw CString(_T("Error reading \"name\" parameter of <cmd> tag"));
			ulId = XmlGetAttributeLong(spElem, _T("id"), 0);
			if(!ulId)
				throw CString(_T("Error reading \"id\" parameter of <cmd> tag"));
		}
		else
		{
			sCmd = XmlGetAttributeStr(spRes, _T("name"));
			if(sCmd.IsEmpty())
				throw CString(_T("Error reading \"name\" parameter of <result> tag"));
			ulId = XmlGetAttributeLong(spRes, _T("id"), 0);
			if(!ulId)
				throw CString(_T("Error reading \"id\" parameter of <result> tag"));
		}*/

		CXmlTask *pTask = NULL;

#include "XMLUndef.h"

		// Switch by command name, creating appropriate task and load its parameters
#define BEGIN_XML_CMD(name)		if(!sCmd.CompareNoCase(_T(#name))) pTask = new CXml_##name;
#define XML_PARAM_STRING(var)
#define XML_PARAM_DWORD(var)
#define XML_INTERNAL_VAR(var,init)
#define END_XML_CMD
#include "XMLMsgs.h"

		if(pTask)
		{
			pTask->m_ulId = ulId;
			pTask->m_bReceived = true;
			pTask->m_sName = sCmd;
			pTask->m_sContents = sXml;

			if(sType == _T("cmd") || sType == _T("event"))
			{
				if(!pTask->LoadParams(spElem))
				{
					delete pTask;
					pTask = NULL;
				}
				if(sType == _T("event"))
					pTask->m_bEvent = true;
			}
			else if(sType == _T("result"))
				pTask->m_bResult = true;

			return pTask;
		}
	}
	catch (CString &obj)
	{
		// should return <result id="0" error="..." />
		AddLog(LOG_ERROR, "Xml error: %s", obj);
	}
	catch(...)
	{
		// should return <result id="0" error="bad_xml errorcode" />
		AddLog(LOG_ERROR, "Xml error: bad xml\n");
	}
	return NULL;
}
