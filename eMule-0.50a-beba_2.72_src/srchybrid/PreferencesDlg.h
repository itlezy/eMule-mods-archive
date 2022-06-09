#pragma once
#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgServer.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgStats.h"
#include "PPgNotify.h"
// Tux: LiteMule: Remove IRC
#include "PPgTweaks.h"
#include "PPgDisplay.h"
#include "PPgSecurity.h"
#include "PPgWebServer.h"
// Tux: LiteMule: Remove Scheduler
#include "PPgProxy.h"
#include "PPgMessages.h"
#include "PPgBeba.h"	// Tux: others: beba prefs
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif
#include "otherfunctions.h"
#include "Modeless.h"	// Tux: Feature: Modeless dialogs: changed include

class CPreferencesDlg : public CModelessTreePropSheet	// Tux: Feature: Modeless dialogs
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
	// Tux: LiteMule: Remove IRC
	CPPgTweaks		m_wndTweaks;
	CPPgDisplay		m_wndDisplay;
	CPPgSecurity	m_wndSecurity;
	CPPgWebServer	m_wndWebServer;
	// Tux: LiteMule: Remove Scheduler
	CPPgProxy		m_wndProxy;
	CPPgMessages	m_wndMessages;
	CPPgBeba		m_wndBeba;	// Tux: others: beba prefs
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
	// Tux: LiteMule: Remove Help
};
