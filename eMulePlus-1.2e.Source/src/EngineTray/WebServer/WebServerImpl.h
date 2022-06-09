#pragma once

#include "WebServer.h"

class CWebServerImpl : public CWebServer
{
public:
	void ProcessDynamicItem(XmlDoc spDoc, const CString sItem, const vector<CString> arrParams, const CString sArgs);
	void ProcessFinalize(XmlDoc spDoc, const vector<CString> arrParams, const CString sArgs);
};