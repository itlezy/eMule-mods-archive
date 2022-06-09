#pragma once

#include "BarShader.h"
#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgProxy.h"
#include "PPgServer.h"
#include "PPgAdvanced.h"
#include "PPgBackup.h"
#include "PPgLists.h"
#include "PPgLogs.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgSources.h"
#include "PPgIRC.h"
#include "PPgHTTPD.h"
#include "PPgStats.h"
#include "PPgMessaging.h"
#include "PPgNotify.h"
#include "PPgSMTP.h"
#include "PPgPartTraffic.h"
#include "PPgScheduler.h"
#include "PPgSecurity.h"
#include "PPgShortcuts.h"
#include "PPgSorting.h"
#include "PPgWindow.h"
#include "PPgAutoDL.h"
#include "otherfunctions.h"
#include "ListBoxST.h"
#include "SlideBar.h"

// CPreferencesDlg

class CPreferencesDlg : public CPropertySheet
{
public:
						CPreferencesDlg();
	virtual			   ~CPreferencesDlg();
	INT_PTR				CPreferencesDlg::DoModal();
	void				Localize();
	void				SetPrefs(CPreferences *pPrefs)
	{
		m_proppageGeneral.SetPrefs(pPrefs);
		m_proppageConnection.SetPrefs(pPrefs);
		m_proppageLists.SetPrefs(pPrefs);
		m_proppageLogs.SetPrefs(pPrefs);
		m_proppageProxy.SetPrefs(pPrefs);
		m_proppageServer.SetPrefs(pPrefs);
		m_proppageAdvanced.SetPrefs(pPrefs);
		m_proppageBackup.SetPrefs(pPrefs);
		m_proppageDirectories.SetPrefs(pPrefs);
		m_proppageFiles.SetPrefs(pPrefs);
		m_proppageSources.SetPrefs(pPrefs);
		m_proppageIRC.SetPrefs(pPrefs);
		m_proppageHTTPD.SetPrefs(pPrefs);
		m_proppageStats.SetPrefs(pPrefs);
		m_proppageMessaging.SetPrefs(pPrefs);
		m_proppageNotify.SetPrefs(pPrefs);
		m_proppageSMTP.SetPrefs(pPrefs);
		m_proppagePartTraffic.SetPrefs(pPrefs);
		m_proppageScheduler.SetPrefs(pPrefs);
		m_proppageSecurity.SetPrefs(pPrefs);
		m_proppageShortcuts.SetPrefs(pPrefs);
		m_proppageSorting.SetPrefs(pPrefs);
		m_proppageWindow.SetPrefs(pPrefs);
		m_proppageAutoDL.SetPrefs(pPrefs);
	}

	afx_msg void		OnApplyNow();
	afx_msg void		OnOk();
	afx_msg void		OnDestroy();
	afx_msg LRESULT		OnSlideBarSelChanged(WPARAM wParam, LPARAM lParam);
	virtual BOOL		OnInitDialog();
	virtual BOOL		PreTranslateMessage(MSG *pMsg);

	DECLARE_MESSAGE_MAP()

private:
	CPPgGeneral			m_proppageGeneral;
	CPPgConnection		m_proppageConnection;
	CPPgProxy			m_proppageProxy;
	CPPgServer			m_proppageServer;
	CPPgAdvanced		m_proppageAdvanced;
	CPPgBackup			m_proppageBackup;
	CPPgDirectories		m_proppageDirectories;
	CPPgLists			m_proppageLists;
	CPPgLogs			m_proppageLogs;
	CPPgFiles			m_proppageFiles;
	CPPgSources			m_proppageSources;
	CPPgIRC				m_proppageIRC;
	CPPgHTTPD			m_proppageHTTPD;
	CPPgStats			m_proppageStats;
	CPPgMessaging		m_proppageMessaging;
	CPPgNotify			m_proppageNotify;
	CPPgSMTP			m_proppageSMTP;
	CPPgPartTraffic		m_proppagePartTraffic;
	CPPgScheduler		m_proppageScheduler;
	CPPgSecurity		m_proppageSecurity;
	CPPgShortcuts		m_proppageShortcuts;
	CPPgSorting			m_proppageSorting;
	CPPgWindow			m_proppageWindow;
	CPPgAutoDL			m_proppageAutoDL;
	CSlideBar	 		m_slideBar;
	CImageList			m_imageList;
	UINT				m_dwActiveWnd;
	bool				m_bIsVisible;
};
