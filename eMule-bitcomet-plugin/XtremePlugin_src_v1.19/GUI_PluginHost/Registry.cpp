// Registry.cpp

#include "bitcomet_inc.h"
#include "Registry.h"
#include "shlwapi.h"
#include "Core_Common_include/FileSystem.h"
#include "Core_Common_include/System.h"
#include "Core_Common_include/string_conv.h"
using namespace Core_Common;

static const LPCTSTR COMETBROWSER_KEY_PATH = _T("SoftWare\\BitComet\\CometBrowser");

CRegistry::CRegistry()
{
}

CRegistry::~CRegistry()
{
}

bool CRegistry::WriteProfile(const tstring strSection, const tstring strEntry, const tstring strValue)
{
	CWinApp* pApp = AfxGetApp();
	return pApp->WriteProfileString(strSection.c_str(), strEntry.c_str(), strValue.c_str());
}

bool CRegistry::WriteProfile(const tstring strSection, const tstring strEntry, const int64_t nValue)
{
	CString str;
	str.Format( _T("%I64d"),nValue);
	return WriteProfile(strSection, strEntry, tstring(str) );
}

bool CRegistry::WriteProfile(const tstring strSection, const tstring strEntry, const bool bValue)
{
	tstring str = bValue ? _T("true") : _T("false");
	return WriteProfile(strSection, strEntry, str);
}

tstring CRegistry::GetProfileString(const tstring strSection, const tstring strEntry, const tstring strDefault /*= _T("")*/)
{
	CWinApp* pApp = AfxGetApp();
	return tstring( pApp->GetProfileString(strSection.c_str(), strEntry.c_str(), strDefault.c_str()) );
}

int64_t CRegistry::GetProfileInt(const tstring strSection, const tstring strEntry, const int64_t nDefault /*= 0*/)
{
	tstring str = GetProfileString(strSection, strEntry);
	if ( str.empty() )
		return nDefault;
	else
		return  string_conv::to_number<int64_t>( str.c_str() );
}

bool CRegistry::GetProfileBool(const tstring strSection, const tstring strEntry, const bool bDefault /*= true*/)
{
	tstring str = GetProfileString(strSection, strEntry);
	if ( str.empty() )
		return bDefault;
	else
		return ( str == _T("true") );
}

bool CRegistry::SetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, const tstring strValue, HKEY base_key /*= HKEY_CLASSES_ROOT*/ )
{
	HKEY hkey = NULL;

#ifndef _WIN32_WCE
	if ( ERROR_SUCCESS != ::RegCreateKey(base_key, strKeyPath, &hkey ) )
		return false;
#else
	DWORD dwDisposition = 0;
	if ( ERROR_SUCCESS != ::RegCreateKeyEx(base_key, strKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &hkey, &dwDisposition ) )
		return false;
#endif	// _WIN32_WCE

	bool ok = false;
	if ( ERROR_SUCCESS == ::RegSetValueEx( hkey, strName, NULL, REG_SZ, (const BYTE*)strValue.c_str(), strValue.length() * sizeof(TCHAR) ) )
		ok = true;

	::RegCloseKey(hkey);
	return ok;
}

bool CRegistry::SetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, const DWORD strValue, HKEY base_key /*= HKEY_CLASSES_ROOT*/ )
{
	HKEY hkey = NULL;

#ifndef _WIN32_WCE
	if ( ERROR_SUCCESS != ::RegCreateKey(base_key, strKeyPath, &hkey ) )
		return false;
#else
	DWORD dwDisposition = 0;
	if ( ERROR_SUCCESS != ::RegCreateKeyEx(base_key, strKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &hkey, &dwDisposition ) )
		return false;
#endif	// _WIN32_WCE

	bool ok = false;
	if ( ERROR_SUCCESS == ::RegSetValueEx( hkey, strName, NULL, REG_DWORD, (const BYTE*)&strValue, sizeof(strValue) ) )
		ok = true;

	::RegCloseKey(hkey);
	return ok;
}

bool CRegistry::GetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, string& strValue, HKEY base_key /*= HKEY_CLASSES_ROOT*/ )
{
	HKEY hkey = NULL;
	if ( ERROR_SUCCESS != ::RegOpenKeyEx(base_key, strKeyPath, 0, KEY_QUERY_VALUE, &hkey ) )
		return false;

	bool ok = false;
	DWORD len = 500;
	DWORD type = NULL;
	// query size
	long ret = ::RegQueryValueEx( hkey, strName, NULL, &type, NULL, &len );
	if ( ret == ERROR_SUCCESS )
	{
		BYTE* buf = new BYTE[len+1];
		if ( buf && ::RegQueryValueEx( hkey, strName, NULL, &type, buf, &len ) == ERROR_SUCCESS )
		{
			if ( len > sizeof(char) )
			{
				if ( buf[len-1] == 0 )
					strValue.assign( (char *)buf, len / sizeof(char) - 1 );
				else
					strValue.assign( (char *)buf, len / sizeof(char) );
			}
			else
			{
				strValue.clear();
			}
			ok = true;
		}
		delete []buf;
	}
	::RegCloseKey(hkey);
	return ok;
}

bool CRegistry::GetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, wstring& strValue, HKEY base_key /*= HKEY_CLASSES_ROOT*/ )
{
	HKEY hkey = NULL;
	if ( ERROR_SUCCESS != ::RegOpenKeyEx(base_key, strKeyPath, 0, KEY_QUERY_VALUE, &hkey ) )
		return false;

	bool ok = false;
	DWORD len = 500;
	DWORD type = NULL;
	// query size
	long ret = ::RegQueryValueEx( hkey, strName, NULL, &type, NULL, &len );
	if ( ret == ERROR_SUCCESS )
	{
		BYTE* buf = new BYTE[len+1];
		if ( buf && ::RegQueryValueEx( hkey, strName, NULL, &type, buf, &len ) == ERROR_SUCCESS )
		{
			if ( len > sizeof(WCHAR) )
				strValue.assign( (WCHAR *)buf, len / sizeof(WCHAR) - 1 );
			else
				strValue.clear();
			ok = true;
		}
		delete []buf;
	}
	::RegCloseKey(hkey);
	return ok;
}

bool CRegistry::DeleteRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, HKEY base_key /*= HKEY_CLASSES_ROOT*/ )
{
	HKEY hkey = NULL;

#ifndef _WIN32_WCE
	if ( ERROR_SUCCESS != ::RegCreateKey(base_key, strKeyPath, &hkey ) )
		return false;
#else
	DWORD dwDisposition = 0;
	if ( ERROR_SUCCESS != ::RegCreateKeyEx(base_key, strKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &hkey, &dwDisposition ) )
		return false;
#endif	// _WIN32_WCE

	bool ok = false;
	if ( ERROR_SUCCESS == ::RegDeleteValue( hkey, strName ) )
		ok = true;

	::RegCloseKey(hkey);
	return ok;
}

bool CRegistry::DeleteRegistryKeyWithAllSub(const LPCTSTR strKeyPath, const LPCTSTR strSubKeyName, HKEY base_key /*= HKEY_CLASSES_ROOT*/ )
{
	HKEY hkey = NULL;

#ifndef _WIN32_WCE
	if ( ERROR_SUCCESS != ::RegCreateKey(base_key, strKeyPath, &hkey ) )
		return false;
#else
	DWORD dwDisposition = 0;
	if ( ERROR_SUCCESS != ::RegCreateKeyEx(base_key, strKeyPath, 0, NULL, REG_OPTION_NON_VOLATILE, 0, NULL, &hkey, &dwDisposition ) )
		return false;
#endif	// _WIN32_WCE

	bool ok = false;
#ifndef _WIN32_WCE
	if ( ERROR_SUCCESS == ::SHDeleteKey( hkey, strSubKeyName ) )
		ok = true;
#else
	if ( ERROR_SUCCESS == ::RegDeleteKey( hkey, strSubKeyName ) )
		ok = true;
#endif	// _WIN32_WCE

	::RegCloseKey(hkey);
	return ok;
}

tstring CRegistry::GetAppFileName(void)
{
	CString strAppFileName;
	DWORD len = ::GetModuleFileName(NULL, strAppFileName.GetBuffer(MAX_PATH*2) , MAX_PATH*2);
	strAppFileName.ReleaseBuffer( len );
	return _T("\"") + tstring(strAppFileName) + _T("\"");
}

tstring CRegistry::GetBrowserAppTypeName(void)
{
	tstring strBrowserApp;
	GetRegistryValue(_T("HTTP\\shell\\open\\ddeexec\\Application"), _T(""), strBrowserApp, HKEY_CLASSES_ROOT);

	return strBrowserApp;
}

tstring CRegistry::GetBrowserAppName(void)
{
	tstring strBrowserApp;
	GetRegistryValue(_T("HTTP\\shell\\open\\command"), _T(""), strBrowserApp, HKEY_CLASSES_ROOT);

	return strBrowserApp;
}

tstring CRegistry::GetBCToolbarSearchID(void)
// return value:
//		""		- BCToolbar not installed
//		default	- User not choose
//		other	- User choice
{
	CString strKeyPath = _T("Software\\BitComet\\BitCometBar");
	CString strName = _T("Search Engine");
	tstring strValue;

	HKEY hkey = NULL;
	if ( ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CURRENT_USER, strKeyPath, 0, KEY_QUERY_VALUE, &hkey ) )
		return strValue;

	DWORD len = 500;
	DWORD type = NULL;
	strValue = _T("default");
	// query size
	long ret = ::RegQueryValueEx( hkey, strName, NULL, &type, NULL, &len );
	if ( ret == ERROR_SUCCESS )
	{
		BYTE* buf = new BYTE[len+1];
		if ( buf && ::RegQueryValueEx( hkey, strName, NULL, &type, buf, &len ) == ERROR_SUCCESS )
		{
			if ( len > sizeof(TCHAR) )
				strValue.assign( (TCHAR *)buf, len / sizeof(TCHAR) - 1 );
			else
				strValue.clear();
		}
		delete []buf;
	}

	::RegCloseKey(hkey);
	return strValue;
}

int CRegistry::GetBCToolbarSearchCount(void)
// return value:
//		-1		- BCToolbar not installed
//		>=0  	- User search count
{
	CString strKeyPath = _T("Software\\BitComet\\BitCometBar");
	CString strName = _T("Search Count");

	HKEY hkey = NULL;
	if ( ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CURRENT_USER, strKeyPath, 0, KEY_QUERY_VALUE, &hkey ) )
		return -1;

	DWORD dwValue = 0;
	DWORD dwType = NULL;
	DWORD dwCount = sizeof(DWORD);
	long ret = RegQueryValueEx(hkey, strName, NULL, &dwType, (LPBYTE)&dwValue, &dwCount);
	if ( ret != ERROR_SUCCESS )
	{
		dwValue = 0;
	}

	::RegCloseKey(hkey);
	return dwValue;
}

tstring CRegistry::GetBCInstallDate(void)
// the registry entry is written by install script
// return value:
//		""		    - install date string not found
//		"20060102"	- User not choose
{
	CString strKeyPath = _T("Software\\BitComet");
	CString strName = _T("Install Date");
	tstring strValue;

	HKEY hkey = NULL;
	if ( ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CURRENT_USER, strKeyPath, 0, KEY_QUERY_VALUE, &hkey ) )
		return strValue;

	DWORD len = 500;
	DWORD type = NULL;
	// query size
	long ret = ::RegQueryValueEx( hkey, strName, NULL, &type, NULL, &len );
	if ( ret == ERROR_SUCCESS )
	{
		BYTE* buf = new BYTE[len+1];
		if ( buf && ::RegQueryValueEx( hkey, strName, NULL, &type, buf, &len ) == ERROR_SUCCESS )
		{
			if ( len > sizeof(TCHAR) )
				strValue.assign( (TCHAR *)buf, len / sizeof(TCHAR) - 1 );
			else
				strValue.clear();
		}
		delete []buf;
	}

	::RegCloseKey(hkey);
	return strValue;
}

void CRegistry::ClearBCInstallDate(void)
{
	DeleteRegistryValue( _T("Software\\BitComet"), _T("Install Date"), HKEY_CURRENT_USER );
}

tstring CRegistry::GetInstallPackageName()
{
	tstring strName;
	GetRegistryValue( _T("Software\\BitComet"), _T("PackageName"), strName, HKEY_CURRENT_USER );

	return strName;
}

void CRegistry::ClearInstallPackageName(void)
{
	DeleteRegistryValue( _T("Software\\BitComet"), _T("PackageName"), HKEY_CURRENT_USER );
}

bool CRegistry::GetInstallSettingNeedGoogleToolbar(void)
{
	string strNeed;
	GetRegistryValue( _T("Software\\BitComet"), _T("NeedGoogleToolBar"), strNeed, HKEY_CURRENT_USER );

	if(!strNeed.empty() && strNeed[0] == 1)
	{
		return true;
	}

	return false;
}

void CRegistry::ClearInstallSettingNeedGoogleToolbar(void)
{
	DeleteRegistryValue( _T("Software\\BitComet"), _T("NeedGoogleToolBar"), HKEY_CURRENT_USER );
}

bool CRegistry::GetGoogleToolbarInstalled(void)
{
	string strInstalled;
	GetRegistryValue( _T("Software\\BitComet"), _T("GoogleToolBarInstalled"), strInstalled, HKEY_CURRENT_USER );

	if(!strInstalled.empty() && strInstalled[0] == 1)
	{
		return true;
	}

	return false;
}

void CRegistry::SetGoogleToolbarInstalled(void)
{
	SetRegistryValue( _T("Software\\BitComet"), _T("GoogleToolBarInstalled"), (DWORD)1, HKEY_CURRENT_USER );
}

bool CRegistry::GetInstallSettingNeedGooglePinyin(void)
{
	string strNeed;
	GetRegistryValue( _T("Software\\BitComet"), _T("NeedGooglePinYin"), strNeed, HKEY_CURRENT_USER );

	if(!strNeed.empty() && strNeed[0] == 1)
	{
		return true;
	}

	return false;
}

void CRegistry::ClearInstallSettingNeedGooglePinyin(void)
{
	DeleteRegistryValue( _T("Software\\BitComet"), _T("NeedGooglePinYin"), HKEY_CURRENT_USER );
}

bool CRegistry::GetGooglePinyinInstalled(void)
{
	string strInstalled;
	GetRegistryValue( _T("Software\\BitComet"), _T("GooglePinYinInstalled"), strInstalled, HKEY_CURRENT_USER );

	if(!strInstalled.empty() && strInstalled[0] == 1)
	{
		return true;
	}

	return false;
}

void CRegistry::SetGooglePinyinInstalled(void)
{
	SetRegistryValue( _T("Software\\BitComet"), _T("GooglePinYinInstalled"), (DWORD)1, HKEY_CURRENT_USER );
}

bool CRegistry::GetNewInstallFlag(void)
{
	string strInstalled;
	GetRegistryValue( _T("Software\\BitComet"), _T("NewInstall"), strInstalled, HKEY_CURRENT_USER );

	if(!strInstalled.empty() && strInstalled[0] == 1)
	{
		return true;
	}

	return false;
}

void CRegistry::ClearNewInstallFlag(void)
{
	DeleteRegistryValue( _T("Software\\BitComet"), _T("NewInstall"), HKEY_CURRENT_USER );
}

bool CRegistry::GetEMuleNewInstallFlag(void)
{
	string strInstalled;
	GetRegistryValue( _T("Software\\BitComet eDonkey plugin"), _T("NewInstall"), strInstalled, HKEY_CURRENT_USER );

	if(!strInstalled.empty() && strInstalled[0] == 1)
	{
		return true;
	}

	GetRegistryValue( _T("Software\\BitCometBeta eDonkey plugin"), _T("NewInstall"), strInstalled, HKEY_CURRENT_USER );

	if(!strInstalled.empty() && strInstalled[0] == 1)
	{
		return true;
	}

	return false;
}

void CRegistry::ClearEMuleNewInstallFlag(void)
{
	DeleteRegistryValue( _T("Software\\BitComet eDonkey plugin"), _T("NewInstall"), HKEY_CURRENT_USER );
	DeleteRegistryValue( _T("Software\\BitCometBeta eDonkey plugin"), _T("NewInstall"), HKEY_CURRENT_USER );
}

wstring CRegistry::GetEMuleInstallLang(void)
{
	wstring install_lang;
	GetRegistryValue( _T("Software\\BitComet eDonkey plugin"), _T("Installer Language"), install_lang, HKEY_CURRENT_USER );

	if(!install_lang.empty())
	{
		return install_lang;
	}

	GetRegistryValue( _T("Software\\BitCometBeta eDonkey plugin"), _T("Installer Language"), install_lang, HKEY_CURRENT_USER );

	if(!install_lang.empty() )
	{
		return install_lang;
	}

	return install_lang;
}

int CRegistry::GetCandeoSearchCount(void)
// return value:
//		-1		- Toolbar not installed
//		>=0		- User search count
{
	CString strKeyPath = _T("Software\\BitComet\\BitComet Toolbar");
	CString strName = _T("SearchCount");

	HKEY hkey = NULL;
	if ( ERROR_SUCCESS != ::RegOpenKeyEx(HKEY_CURRENT_USER, strKeyPath, 0, KEY_QUERY_VALUE, &hkey ) )
		return -1;

	DWORD dwValue = 0;
	DWORD dwType = NULL;
	DWORD dwCount = sizeof(DWORD);

	long ret = RegQueryValueEx(hkey, strName, NULL, &dwType, (LPBYTE)&dwValue, &dwCount);
	if ( ret != ERROR_SUCCESS )
	{
		dwValue = 0;
	}

	::RegCloseKey(hkey);
	return dwValue;
}

bool CRegistry::IsWisVista()
{
	return Core_Common::System::system_os_version().version == Core_Common::System::VER_WIN_VISTA;
}

void CRegistry::RegisterUrlProtocol(void)
{
	BOOL bIsVista = IsWisVista();
	tstring value = GetAppFileName();
	value += _T(" /url \"%1\"");

	CString strType = _T("bctp");

	SetRegistryValue( strType, _T(""), _T("URL: BitComet Transfer Protocol") );
	SetRegistryValue( strType, _T("URL Protocol"), _T("") );
	SetRegistryValue( strType+_T("\\DefaultIcon"), _T(""), GetAppFileName() + _T(",1") );
	DeleteRegistryKeyWithAllSub( strType+_T("\\shell\\open"), _T("ddeexec") );	// delete DDE

	// to fix #1136
	if (!bIsVista)
	{
		SetRegistryValue( strType+_T("\\shell\\open\\command"), _T(""), GetAppFileName() );
		SetRegistryValue( strType+_T("\\shell\\open\\ddeexec"), _T(""), _T("[openlink(\"%1\")]") );
		SetRegistryValue( strType+_T("\\shell\\open\\ddeexec\\Application"), _T(""), _T("BitComet") );
		SetRegistryValue( strType+_T("\\shell\\open\\ddeexec\\Topic"), _T(""), _T("TORRENT") );
	}
	else
	{		
		SetRegistryValue( strType+_T("\\shell\\open\\command"), _T(""),  value.c_str());
	}

	strType = _T("bc");

	SetRegistryValue( strType, _T(""), _T("URL: BitComet Transfer Protocol") );
	SetRegistryValue( strType, _T("URL Protocol"), _T("") );
	SetRegistryValue( strType+_T("\\DefaultIcon"), _T(""), GetAppFileName() + _T(",1") );
	DeleteRegistryKeyWithAllSub( strType+_T("\\shell\\open"), _T("ddeexec") );	// delete DDE

	if (!bIsVista)
	{
		SetRegistryValue( strType+_T("\\shell\\open\\command"), _T(""), GetAppFileName() );
		SetRegistryValue( strType+_T("\\shell\\open\\ddeexec"), _T(""), _T("[openlink(\"%1\")]") );
		SetRegistryValue( strType+_T("\\shell\\open\\ddeexec\\Application"), _T(""), _T("BitComet") );
		SetRegistryValue( strType+_T("\\shell\\open\\ddeexec\\Topic"), _T(""), _T("TORRENT") );
	}
	else
	{
		SetRegistryValue( strType+_T("\\shell\\open\\command"), _T(""),  value.c_str());
	}
}

void CRegistry::UnregisterUrlProtocol(void)
{
	DeleteRegistryKeyWithAllSub( _T("bctp"), _T("shell") );

	DeleteRegistryKeyWithAllSub( _T("bc"), _T("shell") );
}

void CRegistry::set_bc_install_path(tstring path)
{
	SetRegistryValue(_T("Software\\BitComet"), _T(""), path, HKEY_CURRENT_USER );
}

void CRegistry::set_ie_search_band_name(tstring name)
{
	SetRegistryValue(_T("BitCometBHO.VerticalBar"), _T(""), name, HKEY_CLASSES_ROOT );
	SetRegistryValue(_T("BitCometBHO.VerticalBar.1"), _T(""), name, HKEY_CLASSES_ROOT );
	SetRegistryValue(_T("CLSID\\{E7A829CC-671F-4C3D-B590-8C0AEA72E6B2}"), _T(""), name, HKEY_CLASSES_ROOT );
}

void CRegistry::set_should_show_video_download_hint(bool show)
{
	DWORD dwShow = (show?1:0);
	SetRegistryValue(COMETBROWSER_KEY_PATH, _T("ShowVideoHint"), dwShow, HKEY_CURRENT_USER);
}

bool CRegistry::should_show_video_download_hint()
{	
	DWORD show = 1;
	string str;
	GetRegistryValue(COMETBROWSER_KEY_PATH, _T("ShowVideoHint"), str, HKEY_CURRENT_USER);
	if (!str.empty())
	{
		show = static_cast<DWORD>(str[0]);
	}
	return 0 != show;
}

tstring CRegistry::GetBitCometInstallPath(void)
{
	tstring strURL;
	GetRegistryValue( _T("Software\\BitComet\\"), NULL, strURL, HKEY_CURRENT_USER );

	return strURL;
}

bool CRegistry::is_file_type_associated(const tstring& file_ext_name)
// e.g. : file_ext_name = ".flv"
{
	tstring prog_id;
	GetRegistryValue( file_ext_name.data(), _T(""), prog_id );
	if(prog_id.empty())
		return false;

	tstring prog_id_name;
	GetRegistryValue( prog_id.c_str(), _T(""), prog_id_name );
	if(prog_id_name.empty())
		return false;

	tstring open_cmd_path = prog_id + _T("\\shell\\open\\command");
	tstring open_cmd;
	GetRegistryValue( open_cmd_path.c_str(), _T(""), open_cmd );
	if(open_cmd.empty())
		return false;

	return true;
}