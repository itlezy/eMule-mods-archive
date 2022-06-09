// XML.h: helper class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../TaskProcessor.h"
#include "../Other/XMLhelper.h"

enum
{
	XML_GET_START	= 0,
	XML_GET_READ	= 1,
	XML_GET_SEND	= 2
};

enum EnumSetParam
{
	PARAM_UPLOAD_LIMIT	= 0
};

//////////////////////////////////////////////////////////////////////
struct CTask_SendXml : public CTask
{
	CTask_SendXml(SOCKET hSocket, CString sXml);
	virtual bool Process();
	virtual LPCTSTR TaskName(){ return _T("SendXml"); }

	SOCKET	m_hSocket;
	CString	m_sXml;
};

//////////////////////////////////////////////////////////////////////
class CXmlTask : public CTcpCompletionTask
{
public:
	CXmlTask();
	virtual bool LoadParams(XmlElement spElem) = 0;
	virtual XmlDoc PrepareResult();

	void SendBuf(CString sXml);

	static CXmlTask* ParseXml(CString sXml);

	ULONG	GetId(){ return m_ulId; }

	// for debug
	void SetReceived(bool bReceived){ m_bReceived = bReceived; }

	static XmlDoc PrepareDocument(LPCTSTR sTaskName, LPCTSTR sTag);

protected:
	ULONG	m_ulId;
	bool	m_bReceived;
	bool	m_bResult;
	bool	m_bEvent;

	CEmClient_Xml *m_pClient;

	CString	m_sName;

	CString m_sContents;

	static long	s_lNextId;
};

// Defining Xml task classes
#define BEGIN_XML_CMD(name)		class CXml_##name : public CXmlTask \
									{ \
									public: \
									CXml_##name(); \
									virtual bool Process(); \
									bool ProcessResult(); \
									bool ProcessReceived(); \
									bool ProcessEvent(); \
									virtual void SetClient(CEmClient *pClient) { m_pClient = dynamic_cast<CEmClient_Xml*>(pClient); } \
									bool LoadParams(XmlElement spElem); \
									virtual LPCTSTR TaskName(){ return _T("Xml_##name"); } \

#define XML_PARAM_STRING(var)		CString m_s##var;
#define XML_PARAM_DWORD(var)		DWORD m_dw##var;
#define XML_INTERNAL_VAR(var,init)	var;
#define END_XML_CMD				};
#include "XMLMsgs.h"

#define XML_PROCESS_FUNC(name)	bool CXml_##name::ProcessReceived()
#define XML_PROCESS_EMPTY(name)	bool CXml_##name::ProcessReceived(){ return true; }
#define XML_RESULT_FUNC(name)	bool CXml_##name::ProcessResult()
#define XML_RESULT_EMPTY(name)	bool CXml_##name::ProcessResult(){ return true; }
#define XML_EVENT_FUNC(name)	bool CXml_##name::ProcessEvent()
#define XML_EVENT_EMPTY(name)	bool CXml_##name::ProcessEvent(){ return true; }
