#pragma once
#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgServer.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgStats.h"
#include "PPgNotify.h"
#include "PPgIRC.h"
#include "PPgTweaks.h"
#include "PPgDisplay.h"
#include "PPgSecurity.h"
#include "PPgWebServer.h"
#include "PPgScheduler.h"
#include "PPgProxy.h"
#include "PPgMessages.h"
#include "PPgXtreme.h" //Xman Xtreme Mod
#include "PPgXtreme2.h" //Xman Xtreme Mod
#include "KCSideBannerWnd.h" //Xman Preferences Banner 
#include "PPgScar.h" // ScarAngel Preferences window - Stulle

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif
#include "otherfunctions.h"
#include "TreePropSheet.h"

class CPreferencesDlg : public CTreePropSheet
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg();
	virtual ~CPreferencesDlg();
	
	CPPgGeneral		m_wndGeneral;
	CPPgConnection	m_wndConnection;
	CPPgServer		m_wndServer;
	CPPgDirectories	m_wndDirectories;
	CPPgFiles		m_wndFiles;
	CPPgStats		m_wndStats;
	CPPgNotify		m_wndNotify;
	CPPgIRC			m_wndIRC;
	CPPgTweaks		m_wndTweaks;
	CPPgDisplay		m_wndDisplay;
	CPPgSecurity	m_wndSecurity;
	CPPgWebServer	m_wndWebServer;
	CPPgScheduler	m_wndScheduler;
	CPPgProxy		m_wndProxy;
	CPPgMessages	m_wndMessages;
	CPPgXtreme		m_wndXtreme; //Xman Xtreme Mod
	CPPgXtreme2		m_wndXtreme2; //Xman Xtreme Mod
	CKCSideBannerWnd m_banner; //Xman Preferences Banner
	CPPgScar		m_wndScar; // ScarAngel Preferences window - Stulle
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CPPgDebug		m_wndDebug;
#endif

	void Localize();
	void SetStartPage(UINT uStartPageID);

	CPPgWebServer::eTab m_WebServerTab; // Tabbed WebInterface settings panel [Stulle] - Stulle
	CPPgScar::eTab m_ScarTab; // Tabbed Preferences [TPT] - Stulle

protected:
	LPCTSTR m_pPshStartPage;
	bool m_bSaveIniFile;

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
};
