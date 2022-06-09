// Prefs.cpp: implementation of the CPrefs class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Prefs.h"
#include "../../ini2.h"

CPrefs::CPrefs()
	:m_uMaxUploadSlots(0)
	,m_uPortXml(9090)
	,m_bXmlLocalBind(true)
	,m_bAlertOnErrors(false)
	,m_bSaveLogsIO(false)
{
	LoadPrefs();
}

CPrefs::~CPrefs()
{
	SavePrefs();
}

void CPrefs::LoadPrefs()
{
	CIni ini(GetConfigDir() + _T("preferences2.ini"));

	ini.SetDefaultCategory(_T("Connection"));
	m_uPortXml		= ini.GetUInt32(_T("PortXml"), 9090);
	m_bXmlLocalBind	= ini.GetBool(_T("XmlLocalBind"), true);

	ini.SetDefaultCategory(_T("Debug"));
	m_bAlertOnErrors	= ini.GetBool(_T("AlertOnErrors"), true);
	m_bSaveLogsIO		= ini.GetBool(_T("SaveLogsIO"), false);

	ini.CloseWithoutSave();
}

void CPrefs::SavePrefs()
{
	CIni ini(GetConfigDir() + _T("preferences2.ini"));

	ini.SetDefaultCategory(_T("Connection"));
	ini.SetUInt32(	_T("PortXml"),		m_uPortXml);
	ini.SetBool(	_T("XmlLocalBind"), m_bXmlLocalBind);

	ini.SetDefaultCategory(_T("Debug"));
	ini.SetBool(_T("AlertOnErrors"),	m_bAlertOnErrors);
	ini.SetBool(_T("SaveLogsIO"),		m_bSaveLogsIO);

	ini.SaveAndClose();
}

USHORT CPrefs::_GetMaxUpSlots() const
{
	if(m_uMaxUploadSlots)
		return m_uMaxUploadSlots;
	else
	{
		if(GetMaxUpload() == UNLIMITED)
			return 10; //(GetDataRate() / UPLOAD_CLIENT_DATARATE);
		else
			return (GetMaxUpload() * 1024 / (UPLOAD_LOW_CLIENT_DR * 10) + 1);
	}
}
