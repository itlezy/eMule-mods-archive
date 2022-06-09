//	PreferencesDlg.cpp : implementation file
//
#include "stdafx.h"
#include "emule.h"
#include "PreferencesDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CPreferencesDlg, CPropertySheet)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_SBN_SELCHANGED, OnSlideBarSelChanged)
	ON_COMMAND(ID_APPLY_NOW, OnApplyNow)
	ON_COMMAND(IDOK, OnOk)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPreferencesDlg::CPreferencesDlg()
{
	static const uint16 s_auIconResID[] =
	{
		IDI_PREF_GENERAL,		//  0
		IDI_PREF_CONNECTION,	//  1
		IDI_PREF_PROXY,			//  2
		IDI_PREF_SERVER,		//  3
		IDI_PREF_ADVANCED,		//  4
		IDI_PREF_BACKUP,		//  5
		IDI_PREF_FOLDERS,		//  6
		IDI_PREF_FILES,			//  7
		IDI_PREF_IRC,			//  8
		IDI_PREF_WEBSERVER,		//  9
		IDI_PREF_STATISTICS,	// 10
		IDI_MPENDING,			// 11
		IDI_PREF_NOTIFICATIONS,	// 12
		IDI_PREF_SMTP,			// 13
		IDI_PREF_PARTTRAFFIC,	// 14
		IDI_PREF_SCHEDULER,		// 15
		IDI_PREF_SHORTCUTS,		// 16
		IDI_PREF_SORTING,		// 17
		IDI_PREF_WINDOW,		// 18
		IDI_PREF_LISTS,			// 19
		IDI_PREF_LOGS,			// 20
		IDI_PREF_SECURITY,		// 21
		IDI_USERS,				// 22
		IDI_PREF_AUTODL			// 23
	};

	m_bIsVisible = false;

	EnableStackedTabs(FALSE);

//	WARNING: Pages must be added with the same order as the slidebar group items.
	AddPage(&m_proppageGeneral);		// <-- General pages
	AddPage(&m_proppageDirectories);
	AddPage(&m_proppageFiles);
	AddPage(&m_proppageSources);
	AddPage(&m_proppageStats);
	AddPage(&m_proppageLogs);
	AddPage(&m_proppageWindow);
	AddPage(&m_proppageLists);
	AddPage(&m_proppageConnection);		// <-- Connection pages
	AddPage(&m_proppageProxy);
	AddPage(&m_proppageServer);
	AddPage(&m_proppageHTTPD);
	AddPage(&m_proppageSMTP);			// <-- Messaging pages
	AddPage(&m_proppageIRC);
	AddPage(&m_proppageMessaging);
	AddPage(&m_proppageNotify);
	AddPage(&m_proppageAdvanced);		// <-- Advanced pages
	AddPage(&m_proppageAutoDL);
	AddPage(&m_proppageBackup);
	AddPage(&m_proppagePartTraffic);
	AddPage(&m_proppageScheduler);
	AddPage(&m_proppageSecurity);
	AddPage(&m_proppageShortcuts);
	AddPage(&m_proppageSorting);

	m_dwActiveWnd = 0;

	m_imageList.Create(16, 16, g_App.m_iDfltImageListColorFlags | ILC_MASK, ARRSIZE(s_auIconResID), 0);
	FillImgLstWith16x16Icons(&m_imageList, s_auIconResID, ARRSIZE(s_auIconResID));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPreferencesDlg::~CPreferencesDlg()
{
	m_imageList.DeleteImageList();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
INT_PTR CPreferencesDlg::DoModal()
{
	if (!m_bIsVisible)
	{
		m_bIsVisible = true;
		return CPropertySheet::DoModal();
	}
	else
	{
		return FALSE;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferencesDlg::OnDestroy()
{
	m_bIsVisible = false;
	CPropertySheet::OnDestroy();
	m_dwActiveWnd = GetActiveIndex();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferencesDlg::OnApplyNow()
{
	Default();
	g_App.m_pPrefs->Save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferencesDlg::OnOk()
{
	Default();
	g_App.m_pPrefs->Save();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPreferencesDlg::OnInitDialog()
{
	m_slideBar.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER, CRect(0, 0, 0, 0), this, IDC_PREFS_SLIDEBAR);
	m_slideBar.SetImageList(&m_imageList);
	m_slideBar.SetHAlignCaption(DT_CENTER);

//	Sets a bold font for the group buttons
	CFont* pGroupFont = m_slideBar.GetGroupFont();
	ASSERT_VALID(pGroupFont);
	LOGFONT logFont;
	pGroupFont->GetLogFont(&logFont);
	logFont.lfWeight *= 2;
	if (logFont.lfWeight > FW_BLACK)
	{
		logFont.lfWeight = FW_BLACK;
	}
	pGroupFont->DeleteObject();
	pGroupFont->CreateFontIndirect(&logFont);

	ASSERT_VALID(pGroupFont);

	CPropertySheet::OnInitDialog();

	SetActivePage(m_dwActiveWnd);
	Localize();
	m_slideBar.SetFocus();

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
LRESULT CPreferencesDlg::OnSlideBarSelChanged(WPARAM wParam, LPARAM lParam)
{
	int iCurrentGlobalSel = m_slideBar.GetGlobalSelectedItem();
	NOPRM(wParam);
	NOPRM(lParam);

	SetActivePage(iCurrentGlobalSel);

	CListBoxST* pListBox = m_slideBar.GetGroupListBox(m_slideBar.GetSelectedGroupIndex());
	ASSERT_VALID(pListBox);

	CString strTemp = m_slideBar.GetGroupName(m_slideBar.GetSelectedGroupIndex());
	CString strTitle = GetResString(IDS_PREFERENCES);

	strTitle += _T(" -> ");
	strTitle += strTemp;
	strTitle += _T(" -> ");
	pListBox->GetText(pListBox->GetCurSel(), strTemp);
	strTitle += strTemp;
	SetWindowText(strTitle);

	pListBox->SetFocus();

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPreferencesDlg::Localize()
{
	m_proppageGeneral.Localize();
	m_proppageDirectories.Localize();
	m_proppageFiles.Localize();
	m_proppageSources.Localize();
	m_proppageStats.Localize();
	m_proppageLogs.Localize();
	m_proppageWindow.Localize();
	m_proppageLists.Localize();
	m_proppageConnection.Localize();
	m_proppageProxy.Localize();
	m_proppageServer.Localize();
	m_proppageHTTPD.Localize();
	m_proppageSMTP.Localize();
	m_proppageIRC.Localize();
	m_proppageMessaging.Localize();
	m_proppageNotify.Localize();
	m_proppageAdvanced.Localize();
	m_proppageAutoDL.Localize();
	m_proppageBackup.Localize();
	m_proppagePartTraffic.Localize();
	m_proppageScheduler.Localize();
	m_proppageSecurity.Localize();
	m_proppageShortcuts.Localize();
	m_proppageSorting.Localize();

	m_slideBar.ResetContent();

//	General group
	int iGroup = m_slideBar.AddGroup(GetResString(IDS_PW_GENERAL)/*, 0*/);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_GENERAL), iGroup, 0);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_DIR), iGroup, 6);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_FILES), iGroup, 7);
	m_slideBar.AddGroupItem(GetResString(IDS_DL_SOURCES), iGroup, 22);
	m_slideBar.AddGroupItem(GetResString(IDS_STATISTICS), iGroup, 10);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_LOGS), iGroup, 20);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_WINDOW), iGroup, 18);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_LISTS), iGroup, 19);

//	Connetion group
	iGroup = m_slideBar.AddGroup(GetResString(IDS_PW_CONNECTION)/*, 1*/);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_CONNECTION), iGroup, 1);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_PROXY), iGroup, 2);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SERVER), iGroup, 3);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_HTTPD), iGroup, 9);

//	Messaging group
	iGroup = m_slideBar.AddGroup(GetResString(IDS_PW_IM_OPTIONS)/*, 11*/);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SMTP), iGroup, 13);
	m_slideBar.AddGroupItem(GetResString(IDS_IRC), iGroup, 8);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_IM_OPTIONS), iGroup, 11);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_EKDEV_OPTIONS), iGroup, 12);

//	Advanced group
	iGroup = m_slideBar.AddGroup(GetResString(IDS_PW_ADVANCED)/*, 4*/);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_ADVANCED), iGroup, 4);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_AUTODL), iGroup, 23);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_BACKUP), iGroup, 5);
	m_slideBar.AddGroupItem(GetResString(IDS_SF_PARTTRAFFIC), iGroup, 14);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SCHEDULER), iGroup, 15);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SECURITY), iGroup, 21);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SHORTCUTS), iGroup, 16);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SORTING), iGroup, 17);

//	Determines the width needed to the slidebar, and its position
	int width = m_slideBar.GetGreaterStringWidth();
	CRect rectOld;
	int xoffset;
	int yoffset;

	width += 60;
	m_slideBar.GetWindowRect(rectOld);

	if (IsWindowVisible())
	{
		yoffset = 0;
		xoffset = width - rectOld.Width();
	}
	else
	{
		xoffset = width - rectOld.Width() + 10;
		GetActivePage()->GetWindowRect(rectOld);
		ScreenToClient (rectOld);
		yoffset = -rectOld.top;
	}

//	Resizes the Preferences window
	GetWindowRect(rectOld);
	SetWindowPos(NULL, 0, 0, rectOld.Width() + xoffset, rectOld.Height() + yoffset, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
	CTabCtrl* tab = GetTabControl();
	tab->GetWindowRect (rectOld);
	ScreenToClient (rectOld);
	tab->SetWindowPos(NULL, rectOld.left + xoffset, rectOld.top + yoffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	CPropertyPage* activepage = GetActivePage();
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	activepage->SetWindowPos(NULL, rectOld.left + xoffset, rectOld.top + yoffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
	GetClientRect(rectOld);
	m_slideBar.SetWindowPos(NULL, 6, 6, width, rectOld.Height() - 12, SWP_NOZORDER | SWP_NOACTIVATE);
	int _PropSheetButtons[] = {IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };
	CWnd* PropSheetButton;

	for (int i = 0; i < ARRSIZE(_PropSheetButtons); i++)
	{
		if ((PropSheetButton = GetDlgItem(_PropSheetButtons[i])) != NULL)
		{
			PropSheetButton->GetWindowRect(rectOld);
			ScreenToClient(rectOld);
			PropSheetButton->SetWindowPos(NULL, rectOld.left + xoffset, rectOld.top + yoffset, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
		}
	}
	tab->ShowWindow(SW_HIDE);
	m_slideBar.SelectGlobalItem(GetActiveIndex());
	OnSlideBarSelChanged(NULL, NULL);
	CenterWindow();
	Invalidate();
	RedrawWindow();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPreferencesDlg::PreTranslateMessage(MSG *pMsg)
{
	BOOL	bRc = CPropertySheet::PreTranslateMessage(pMsg);

//	Update current page view on Ctrl+Tab, Shift+Ctrl+Tab, Ctrl+PageUp and Ctrl+PageDown
	if ( (pMsg->message == WM_KEYDOWN) && ((pMsg->wParam == VK_TAB) || (pMsg->wParam == VK_PRIOR) || (pMsg->wParam == VK_NEXT)) &&
		(GetAsyncKeyState(VK_CONTROL) < 0) && (GetPageCount() > 1))
	{
		m_slideBar.SelectGlobalItem(m_dwActiveWnd = GetActiveIndex());
		OnSlideBarSelChanged(0, 0);
	}
	return bRc;
}
