#pragma once

class CRegistry
{
protected:
	CRegistry();
	virtual ~CRegistry();

public:
	static void RegisterUrlProtocol(void);
	static void UnregisterUrlProtocol(void);

	static bool WriteProfile(const tstring strSection, const tstring strEntry, const tstring strValue);
	static bool WriteProfile(const tstring strSection, const tstring strEntry, const int64_t nValue);
	static bool WriteProfile(const tstring strSection, const tstring strEntry, const bool bValue);
	static tstring GetProfileString(const tstring strSection, const tstring strEntry, const tstring strDefault = _T(""));
	static int64_t GetProfileInt(const tstring strSection, const tstring strEntry, const int64_t nDefault = 0);
	static bool GetProfileBool(const tstring strSection, const tstring strEntry, const bool bDefault = true);

	static tstring GetBrowserAppTypeName(void);
	static tstring GetBrowserAppName(void);
	static tstring GetBCToolbarSearchID(void);
	static int GetBCToolbarSearchCount(void);
	static tstring GetBCInstallDate(void);
	static void ClearBCInstallDate(void);
	static int GetCandeoSearchCount(void);

	static bool GetInstallSettingNeedGoogleToolbar(void);
	static void ClearInstallSettingNeedGoogleToolbar(void);
	static bool GetGoogleToolbarInstalled(void);
	static void SetGoogleToolbarInstalled(void);

	static bool GetInstallSettingNeedGooglePinyin(void);
	static void ClearInstallSettingNeedGooglePinyin(void);
	static bool GetGooglePinyinInstalled(void);
	static void SetGooglePinyinInstalled(void);

	static bool GetNewInstallFlag(void);
	static void ClearNewInstallFlag(void);
	static tstring GetBitCometInstallPath(void);
	static bool GetEMuleNewInstallFlag(void);
	static void ClearEMuleNewInstallFlag(void);
	static wstring GetEMuleInstallLang(void);
	static tstring GetInstallPackageName(void);
	static void ClearInstallPackageName(void);
	
	static void set_bc_install_path(tstring path);
	static void set_ie_search_band_name(tstring name);

	static bool is_file_type_associated(const tstring& file_ext_name);

	// for CometBrowser
	static void set_should_show_video_download_hint(bool show);
	static bool should_show_video_download_hint();

protected:
	static tstring GetAppFileName(void);

protected:
	static bool SetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, const tstring strValue, HKEY base_key = HKEY_CLASSES_ROOT);
	static bool SetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, const DWORD strValue, HKEY base_key = HKEY_CLASSES_ROOT);
	static bool GetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, wstring& strValue, HKEY base_key = HKEY_CLASSES_ROOT );
	static bool GetRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, string& strValue, HKEY base_key = HKEY_CLASSES_ROOT );
	static bool DeleteRegistryValue( const LPCTSTR strKeyPath, const LPCTSTR strName, HKEY base_key = HKEY_CLASSES_ROOT );
	static bool DeleteRegistryKeyWithAllSub(const LPCTSTR strKeyPath, const LPCTSTR strSubKeyName, HKEY base_key = HKEY_CLASSES_ROOT );
	static bool IsWisVista();

};
