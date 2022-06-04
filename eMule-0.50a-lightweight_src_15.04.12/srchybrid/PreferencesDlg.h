#pragma once
#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgTweaks.h"
#include "PPgDisplay.h"
#include "PPgSecurity.h"
#include "PPgProxy.h"
//#include "KCSideBannerWnd.h" //Xman Preferences Banner 

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif
#include "otherfunctions.h"
#include "TreePropSheet.h"
#include "Neo\ModeLess.h" // NEO: MLD - [ModelesDialogs] <-- Xanatos --

class CPreferencesDlg : public CModTreePropSheet // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg();
	virtual ~CPreferencesDlg();
	
	CPPgGeneral		m_wndGeneral;
	CPPgConnection	m_wndConnection;
	CPPgDirectories	m_wndDirectories;
	CPPgFiles		m_wndFiles;
	CPPgTweaks		m_wndTweaks;
	CPPgDisplay		m_wndDisplay;
	CPPgSecurity	m_wndSecurity;
	CPPgProxy		m_wndProxy;
//	CKCSideBannerWnd m_banner; //Xman Preferences Banner
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CPPgDebug		m_wndDebug;
#endif

	void Localize();
	void SetStartPage(UINT uStartPageID);

protected:
	LPCTSTR m_pPshStartPage;
	bool m_bSaveIniFile;

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
};
