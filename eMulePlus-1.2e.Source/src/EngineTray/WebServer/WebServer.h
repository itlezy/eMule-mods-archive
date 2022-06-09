#pragma once

#define WEBSERVER_ENABLED	true	//g_eMuleApp.m_pGlobPrefs->GetWSIsEnabled()
#define WEBSERVER_PORT		8080	//g_eMuleApp.m_pGlobPrefs->GetWSPort()
#define WEBSERVER_DIR		_T("./WebServer/Files/")	//g_eMuleApp.m_pGlobPrefs->GetAppDir()
#define WEBSERVER_GZIP		false	//g_eMuleApp.m_pGlobPrefs->GetWebUseGzip();

#include "../../Engine/Other/XMLhelper.h"

class CWebSocket;

typedef struct
{
	CString			sURL;
	CString			sArgs;
	void			*pThis;
	CWebSocket		*pSocket;
} ThreadData;

class CWebServer
{
	friend class CWebSocket;

public:
	CWebServer(void);
	virtual ~CWebServer(void);

	bool Start();
	void Stop();
	void Restart();
	const bool IsRunning()	{ return m_bServerWorking; }

	// Override methods
	virtual void ProcessDynamicItem(XmlDoc spDoc, const CString sItem, const vector<CString> arrParams, const CString sArgs) = 0;
	virtual void ProcessFinalize(XmlDoc spDoc, const vector<CString> arrParams, const CString sArgs) = 0;

protected:
	bool LoadTemplates();
	XmlDoc CreateFromTemplate(const vector<CString> arrParams, const CString sArgs);

	CString ReadParam(const CString sData, const CString sParamName);

	void ProcessURL(ThreadData data);
	void ProcessFileRequest(ThreadData data);

	CString GetContentType(CString sExt);

protected:
	bool	m_bServerWorking;
	XmlDoc	m_spSkeleton;
	XmlDoc	m_spConfig;
};
