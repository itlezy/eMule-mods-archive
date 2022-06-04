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
#include "PPgIonixWebServer.h"
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif
#include "otherfunctions.h"
//MORPH START - Preferences groups [ePlus/Sirob]
/*
#include "TreePropSheet.h"
*/
#include "ListBoxST.h"
//MORPH END   - Preferences groups [ePlus/Sirob]
#include "PPgMorph.h" //MORPH - Added by IceCream, Morph Prefs
#include "PPgMorphShare.h" //MORPH - Added by SiRoB, Morph Prefs
#include "PPgMorph2.h" //MORPH - Added by SiRoB, Morph Prefs
//#include "PPgMorph3.h" //Commander - Added: Morph III
#include "PPgBackup.h" //EastShare - Added by Pretender, TBH-AutoBackup
#include "PPgEastShare.h" //EastShare - Added by Pretender
#include "PPgEmulEspana.h" //MORPH - Added by SiRoB, eMulEspana Preferency
#include "KCSideBannerWnd.h" //Commander - Added: Preferences Banner [TPT]
#include "SlideBar.h" //MORPH - Added by SiRoB, ePLus Group
#include "PPGNTServer.h" //MORPH leuk_he:run as ntservice v1.. 

//MORPH START - Preferences groups [ePlus/Sirob]
/*
class CPreferencesDlg : public CTreePropSheet
*/
class CPreferencesDlg : public CPropertySheet
//MORPH END   - Preferences groups [ePlus/Sirob]
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
	// START ionix advanced webserver
	CPPgIonixWebServer	m_wndIonixWebServer;
	// END ionix advanced webserver
	CPPgNTService	m_wndNTService; //MORPH leuk_he:run as ntservice v1..
	// MORPH start tabbed options [leuk_he]
    void SwitchTab(int page);
 	int ActivePageWebServer;
	int StartPageWebServer;
	int Webserver; 
	int Multiwebserver;
	int NTService; // MORPH leuk_he:run as ntservice v1..
	// MORPH end tabbed option [leuk_he]
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CPPgDebug		m_wndDebug;
#endif
	CPPgMorph		m_wndMorph; //MORPH - Added by IceCream, Morph Prefs
	CPPgMorphShare	m_wndMorphShare; //MORPH - Added by SiRoB, Morph Prefs
	CPPgMorph2		m_wndMorph2; //MORPH - Added by SiRoB, Morph Prefs
	CPPgBackup		m_wndBackup; //EastShare - Added by Pretender, TBH-AutoBackup
	CPPgEastShare	m_wndEastShare; //EastShare - Added by Pretender, ES Prefs
	CPPgEmulespana	m_wndEmulespana; //MORPH - Added by SiRoB, emulEspaña preferency
	
	void Localize();
	void SetStartPage(UINT uStartPageID);

protected:
	LPCTSTR m_pPshStartPage;
	bool m_bSaveIniFile;

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
public:
	CKCSideBannerWnd m_banner;	//Commander - Added: Preferences Banner [TPT]	

//MORPH START - Preferences groups [ePlus/Sirob]
	CListBoxST		m_listbox;
	CButton			m_groupbox;
	CImageList		ImageList;
	int				m_iPrevPage;
	CSlideBar	 	m_slideBar;
protected:
	UINT m_nActiveWnd;
	afx_msg LRESULT		OnSlideBarSelChanged(WPARAM wParam, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
//MORPH END   - Preferences groups [ePlus/Sirob]
};
