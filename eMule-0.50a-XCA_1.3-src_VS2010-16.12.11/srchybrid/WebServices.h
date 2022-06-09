#pragma once

#define	WEBSVC_GEN_URLS		0x0001
#define	WEBSVC_FILE_URLS	0x0002

class CTitleMenu;

class CWebServices
{
public:
	CWebServices();

	CString GetDefaultServicesFile() const;
	void ReadAllServices();
	void RemoveAllServices();

	size_t GetFileMenuEntries(CTitleMenu* pMenu) { return GetAllMenuEntries(pMenu, WEBSVC_FILE_URLS); }
	size_t GetGeneralMenuEntries(CTitleMenu* pMenu) { return GetAllMenuEntries(pMenu, WEBSVC_GEN_URLS); }
	size_t GetAllMenuEntries(CTitleMenu* pMenu, DWORD dwFlags = WEBSVC_GEN_URLS | WEBSVC_FILE_URLS);
	bool RunURL(const CAbstractFile* file, UINT_PTR uMenuID);
	void Edit();

protected:
	struct SEd2kLinkService
	{
		UINT_PTR uMenuID;
		CString strMenuLabel;
		CString strUrl;
		BOOL bFileMacros;
	};
	CAtlArray<SEd2kLinkService> m_aServices;
	time_t m_tDefServicesFileLastModified;
};

extern CWebServices theWebServices;
