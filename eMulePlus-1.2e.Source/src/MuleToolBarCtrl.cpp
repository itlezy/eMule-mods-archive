// MuleToolBarCtrl.cpp : implementation file
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MuleToolbarCtrl.h"
#include "emule.h"
#include "EnBitmap.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#if (_WIN32_IE < 0x0500)
#define TBN_INITCUSTOMIZE       (TBN_FIRST - 23)
#define    TBNRF_HIDEHELP       0x00000001
#endif

BEGIN_MESSAGE_MAP(CMuleToolbarCtrl, CToolBarCtrl)
	ON_WM_SIZE()
	ON_NOTIFY_REFLECT(NM_RCLICK, OnNMRclick)
	ON_NOTIFY_REFLECT(TBN_QUERYDELETE, OnTbnQueryDelete)
	ON_NOTIFY_REFLECT(TBN_QUERYINSERT, OnTbnQueryInsert)
	ON_NOTIFY_REFLECT(TBN_GETBUTTONINFO, OnTbnGetButtonInfo)
	ON_NOTIFY_REFLECT(TBN_TOOLBARCHANGE, OnTbnToolbarChange)
	ON_NOTIFY_REFLECT(TBN_RESET, OnTbnReset)
	ON_NOTIFY_REFLECT(TBN_INITCUSTOMIZE, OnTbnInitCustomize)
END_MESSAGE_MAP()

IMPLEMENT_DYNAMIC(CMuleToolbarCtrl, CToolBarCtrl)

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMuleToolbarCtrl::CMuleToolbarCtrl()
{
	m_bUseSpeedMeter = false;
	m_iLastPressedButton = -1;
	m_iToolbarLabelSettings = 4;

	m_oldcx = 0;
	m_oldcy = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CMuleToolbarCtrl::~CMuleToolbarCtrl()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMuleToolbarCtrl::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
//	Some of those styles needs to be set before the window is shown
//		in order to prevent gui bugs when refreshing
	dwStyle |= TBSTYLE_FLAT | CCS_ADJUSTABLE | CCS_NODIVIDER | ((g_App.m_qwComCtrlVer >= MAKEDLLVERULL(6,0,0,0)) ? TBSTYLE_TRANSPARENT : 0);

	return CToolBarCtrl::Create(dwStyle, rect, pParentWnd, nID);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::Init(void)
{
	int	i;

#ifdef _UNICODE
//	Win9x: Explicitly set to Unicode to receive Unicode notifications.
	SendMessage(CCM_SETUNICODEFORMAT, TRUE);
#endif

	ChangeToolbarBitmap(g_App.m_pPrefs->GetToolbarBitmapSettings(), false);
//	Add buttons
	for (i = 0; i < MTB_NUMBUTTONS; i++)
	{
		m_buttonDefs[i].fsState	= TBSTATE_ENABLED;
		m_buttonDefs[i].fsStyle	= TBSTYLE_CHECK;
		m_buttonDefs[i].iBitmap	= i;
		m_buttonDefs[i].idCommand = IDC_TOOLBARBUTTON + i;
		m_buttonDefs[i].iString	= -1;

		switch (i)
		{
			case MTB_CONNECT: // Connect button
			case MTB_PREFS:
				m_buttonDefs[i].fsStyle = TBSTYLE_BUTTON;
				break;
			case MTB_SERVERS:
				m_buttonDefs[i].fsState |= TBSTATE_CHECKED;
		}
	}
	m_iLastPressedButton = IDC_TOOLBARBUTTON + MTB_SERVERS;

	TBBUTTON	sepButton;

	sepButton.idCommand = 0;
	sepButton.fsStyle = TBSTYLE_SEP;
	sepButton.fsState = TBSTATE_ENABLED;
	sepButton.iString = -1;
	sepButton.iBitmap = -1;

	CString		config = g_App.m_pPrefs->GetToolbarSettings();

	for (i = 0; i < config.GetLength(); i += 2)
	{
		int			iIndex = _tstoi(config.Mid(i,2));
		TBBUTTON	*pButton;

		if (iIndex == 99)
			pButton = &sepButton;
		else if (iIndex >= ARRSIZE(m_buttonDefs))
		//	Unknown button index, skip it
			continue;
		else
			pButton = &m_buttonDefs[iIndex];
		AddButtons(1, pButton);
	}

//	Recalc toolbar-size:
	Localize();		// At first we have to localize the button-text!!!
	ChangeTextLabelStyle(g_App.m_pPrefs->GetToolbarLabelSettings(), false);
	SetBtnWidth();		// Then calc and set the button width
	AutoSize();		// And finally call the original (but maybe obsolete) function

//	Add speed-meter to upper-right corner
	CRect		rClient;

	GetClientRect(&rClient);
	rClient.DeflateRect(7, 7);

	rClient.left = rClient.right - rClient.Height() * 2;
	if (m_bUseSpeedMeter)
		m_ctrlSpeedMeter.CreateEx(WS_EX_STATICEDGE, NULL, NULL, WS_CHILD | WS_VISIBLE, rClient, this, 123);
	else
		m_ctrlSpeedMeter.CreateEx(WS_EX_STATICEDGE, NULL, NULL, WS_CHILD, rClient, this, 123);
	m_ctrlSpeedMeter.SetRange(0, 100);

//	Resize speed-meter
	CRect		r;

	GetWindowRect(&r);
	OnSize(0,r.Width(),r.Height());
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::Localize(void)
{
	if (m_hWnd != NULL)
	{
		static const uint16	s_auTBStrIDs[] =
		{
			IDS_SERVERS,
			IDS_EM_TRANS,
			IDS_SEARCH_NOUN,
			IDS_EM_FILES,
			IDS_MESSAGES,
			IDS_IRC,
			IDS_STATISTICS,
			IDS_PREFERENCES
		};
		TBBUTTONINFO		tbi;

		tbi.dwMask = TBIF_TEXT;
		tbi.cbSize = sizeof(TBBUTTONINFO);

		CString		strBuffer;

#ifdef OLD_SOCKETS_ENABLED
		if (g_App.m_pServerConnect->IsConnected())
			GetResString(&strBuffer, IDS_MAIN_BTN_DISCONNECT);
		else if(g_App.m_pServerConnect->IsConnecting())
			GetResString(&strBuffer, IDS_MAIN_BTN_CANCEL);
		else
#endif //OLD_SOCKETS_ENABLED
			GetResString(&strBuffer, IDS_MAIN_BTN_CONNECT);
		tbi.pszText = lstrcpy(m_tstrButtonTitles[0], strBuffer);
		SetButtonInfo(IDC_TOOLBARBUTTON + MTB_CONNECT, &tbi);

		for (int i = 1; i < MTB_NUMBUTTONS; i++)
		{
			GetResString(&strBuffer, s_auTBStrIDs[i - 1]);
			tbi.pszText = lstrcpy(m_tstrButtonTitles[i], strBuffer);
			SetButtonInfo(IDC_TOOLBARBUTTON + i, &tbi);
		}

		m_iToolbarLabelSettings = 4;
		ChangeTextLabelStyle(g_App.m_pPrefs->GetToolbarLabelSettings(), false);

		SetBtnWidth();
		AutoSize();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnSize(UINT nType, int cx, int cy)
{
	CToolBarCtrl::OnSize(nType, cx, cy);

	if ((!cx && !cy) || (cx==m_oldcx && cy==m_oldcy)) return;

	m_oldcx=cx; m_oldcy=cy;

	if (m_bUseSpeedMeter && ::IsWindow(m_ctrlSpeedMeter.GetSafeHwnd()))
	{
		CRect		rClient;

		GetClientRect(&rClient);
		rClient.DeflateRect(7,7);

		int		iHeight = rClient.Height();
		int		iLeft  = rClient.right - iHeight;

		CSize	csMaxSize;

		GetMaxSize(&csMaxSize);
		if (rClient.left + csMaxSize.cx + 7 > iLeft)
		{
			m_ctrlSpeedMeter.ShowWindow(SW_HIDE);
			return;
		}

		iLeft -= iHeight;
		if (iLeft <= rClient.left + csMaxSize.cx + 7)
			iLeft = rClient.left + csMaxSize.cx + 7;
		rClient.left = iLeft;

		m_ctrlSpeedMeter.SetWindowPos( NULL, rClient.left, rClient.top,
									   rClient.Width(), rClient.Height(),
									   SWP_NOZORDER | SWP_SHOWWINDOW );
	}

	SetBtnWidth();
	AutoSize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::ShowSpeedMeter(bool bShow)
{
	m_bUseSpeedMeter = bShow;

	if (bShow)
	{
		if (!m_ctrlSpeedMeter.IsWindowVisible())
		{
			m_ctrlSpeedMeter.EnableWindow(true);

		//	Resize speed-meter
			CRect		r;

			m_oldcx = m_oldcy = 0;
			GetWindowRect(&r);
			OnSize(0,r.Width(),r.Height());
		}
	}
	else
	{
		m_ctrlSpeedMeter.EnableWindow(false);
		m_ctrlSpeedMeter.ShowWindow(SW_HIDE);

		CRect		rInvalidate;

		m_ctrlSpeedMeter.GetWindowRect(&rInvalidate);
		ScreenToClient(rInvalidate);
		InvalidateRect(rInvalidate);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::SetBtnWidth()
{
	if (m_iToolbarLabelSettings == 1)
	{
		CDC		*pDC = GetDC();
		CFont	*pFnt = GetFont();
		CFont	*pOldFnt = pDC->SelectObject(pFnt);
		CRect	r(0,0,0,0);

	//	Calculate the max. possible button-size
		int		iCalcSize = 0;

		for (int i = 0; i < MTB_NUMBUTTONS; i++)
		{
			if (!IsButtonHidden(IDC_TOOLBARBUTTON + i))
			{
				pDC->DrawText(m_tstrButtonTitles[i], -1, r, DT_SINGLELINE | DT_CALCRECT);
 				if (r.Width() > iCalcSize)
					iCalcSize = r.Width();
			}
		}

		iCalcSize += 10;

		pDC->SelectObject(pOldFnt);
		ReleaseDC(pDC);

		GetClientRect(&r);

		int		iMaxPossible = r.Width() / MTB_NUMBUTTONS;

	//	If the buttons are to big, reduze their size
		if (iCalcSize > iMaxPossible)
			iCalcSize = iMaxPossible;

		SetButtonWidth(iCalcSize, iCalcSize);
	}
	else if(m_iToolbarLabelSettings == 2)
	{
		CRect rcItem;
		GetItemRect(0, &rcItem); // includes the width of the text
		SetButtonSize(CSize(rcItem.Width(), 0));
		SetButtonWidth(0,0);
	}
	else
	{
		SetButtonSize(CSize(0,0));
		SetButtonWidth(0,0);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	EMULE_TRY

	POINT		point;
	CMenu		menuToolbar, menuBitmaps, menuTextLabels;

	GetCursorPos(&point);

	menuToolbar.CreatePopupMenu();

	menuBitmaps.CreateMenu();
	menuBitmaps.AppendMenu(MF_STRING,MP_SELECTTOOLBARBITMAP, GetResString(IDS_SELECTTOOLBARBITMAP));
	menuBitmaps.AppendMenu(MF_STRING,MP_SELECTTOOLBARBITMAPDIR, GetResString(IDS_SELECTTOOLBARBITMAPDIR));
	menuBitmaps.AppendMenu(MF_SEPARATOR);
	m_strBitmapPaths.RemoveAll();

	CString		strCurrBitmapSettings = g_App.m_pPrefs->GetToolbarBitmapSettings();
	bool		bChecked = false;
	UINT		dwFlags = MF_STRING;

	if (strCurrBitmapSettings.IsEmpty())
	{
		dwFlags |= (MF_CHECKED | MF_DISABLED);
		bChecked = true;
	}
	menuBitmaps.AppendMenu(dwFlags, MP_TOOLBARBITMAP, GetResString(IDS_DEFAULT));
	m_strBitmapPaths.Add(_T(""));

	int		i = 1;

	if (!g_App.m_pPrefs->GetToolbarBitmapFolderSettings().IsEmpty())
	{
		WIN32_FIND_DATA	FileData;
		HANDLE			hSearch;
		CString			strBitmapFileName;
		CString			strConfigDir = g_App.m_pPrefs->GetToolbarBitmapFolderSettings();

		strConfigDir += _T("\\");
		hSearch = ::FindFirstFile(strConfigDir + _T("*.eMuleToolbar.bmp"), &FileData);
		if (hSearch != INVALID_HANDLE_VALUE)
		{
			do
			{
				strBitmapFileName = FileData.cFileName;
				m_strBitmapPaths.Add(strConfigDir + strBitmapFileName);

				dwFlags = MF_STRING;
				if (!bChecked && strCurrBitmapSettings == m_strBitmapPaths[i])
				{
					dwFlags |= (MF_CHECKED | MF_DISABLED);
					bChecked = true;
				}
				menuBitmaps.AppendMenu( dwFlags, MP_TOOLBARBITMAP + i,
					strBitmapFileName.Left(strBitmapFileName.GetLength() - CSTRLEN(_T(".eMuleToolbar.bmp"))) );
			} while ((++i < 50) && ::FindNextFile(hSearch, &FileData));

			::FindClose(hSearch);
		}
	}
	if (!bChecked)
	{
		menuBitmaps.AppendMenu(MF_STRING | MF_CHECKED | MF_DISABLED, MP_TOOLBARBITMAP + i, strCurrBitmapSettings);
		m_strBitmapPaths.Add(strCurrBitmapSettings);
	}
	menuToolbar.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)menuBitmaps.m_hMenu, GetResString(IDS_TOOLBARSKINS));

	menuTextLabels.CreateMenu();
	menuTextLabels.AppendMenu(MF_STRING, MP_NOTEXTLABELS, GetResString(IDS_NOTEXTLABELS));
	menuTextLabels.AppendMenu(MF_STRING, MP_TEXTLABELS, GetResString(IDS_ENABLETEXTLABELS));
	menuTextLabels.AppendMenu(MF_STRING, MP_TEXTLABELSONRIGHT, GetResString(IDS_TEXTLABELSONRIGHT));

	UINT		dwPos = static_cast<UINT>(g_App.m_pPrefs->GetToolbarLabelSettings());

	menuTextLabels.CheckMenuItem(dwPos, MF_BYPOSITION | MF_CHECKED);
	menuTextLabels.EnableMenuItem(dwPos, MF_BYPOSITION | MF_DISABLED);
	menuToolbar.AppendMenu(MF_STRING|MF_POPUP, (UINT_PTR)menuTextLabels.m_hMenu, GetResString(IDS_TEXTLABELS));
	menuToolbar.AppendMenu(MF_STRING, MP_CUSTOMIZETOOLBAR, GetResString(IDS_CUSTOMIZETOOLBAR));
	menuToolbar.TrackPopupMenuEx(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this, NULL);

	*pResult = TRUE;

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnTbnQueryDelete(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	*pResult = TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnTbnQueryInsert(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	*pResult = TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnTbnGetButtonInfo(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTOOLBAR		pNMTB = reinterpret_cast<LPNMTOOLBAR>(pNMHDR);

	if (pNMTB->iItem < MTB_NUMBUTTONS)
	{
		lstrcpy(pNMTB->pszText, m_tstrButtonTitles[pNMTB->iItem]);
		pNMTB->tbButton = m_buttonDefs[pNMTB->iItem];
		*pResult = TRUE;
	}
	else
	{
		*pResult = FALSE;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnTbnToolbarChange(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	TBBUTTON	buttoninfo;
	CString		strConfig;

	for (int i = 0; i < GetButtonCount(); i++)
	{
		if (GetButton(i, &buttoninfo))
		{
			switch (buttoninfo.idCommand)
			{
				case 0:
					strConfig += _T("99");
					break;
				default:
					strConfig.AppendFormat(_T("%02u"), buttoninfo.idCommand - IDC_TOOLBARBUTTON);
					break;
			}
		}
	}
	g_App.m_pPrefs->SetToolbarSettings(strConfig);
	Localize();
#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pMDlg->ShowConnectionState(g_App.m_pServerConnect->IsConnected(), _T(""), true);
#endif //OLD_SOCKETS_ENABLED
	SetBtnWidth();
	AutoSize();
	*pResult = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::ChangeToolbarBitmap(const CString &strPath, bool bRefresh)
{
	CImageList		imageList;
	CEnBitmap		Bitmap;

	if (strPath.IsEmpty() || !Bitmap.LoadImage(strPath))
		Bitmap.LoadBitmap(IDB_TOOLBAR);

	BITMAP			bm;

	Bitmap.GetBitmap(&bm);

	if (imageList.Create(bm.bmWidth/11, bm.bmHeight, g_App.m_iDfltImageListColorFlags | ILC_MASK, 11, 1))
		imageList.Add(&Bitmap, RGB(255,0,255));

	CImageList	*pimlOld = SetImageList(&imageList);

	imageList.Detach();
	if (pimlOld != NULL)
		pimlOld->DeleteImageList();

	if (bRefresh)
		Refresh();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CMuleToolbarCtrl::OnCommand(WPARAM wParam, LPARAM lParam)
{
	NOPRM(lParam);
	EMULE_TRY

	switch (wParam)
	{
		case MP_SELECTTOOLBARBITMAPDIR:
		{
			g_App.m_pPrefs->SetToolbarBitmapFolderSettings(BrowseFolder(m_hWnd,GetResString(IDS_SELECT_TOOLBARBITMAPDIR),g_App.m_pPrefs->GetToolbarBitmapFolderSettings()));
			break;
		}
		case MP_CUSTOMIZETOOLBAR:
		{
			Customize();
			break;
		}
		case MP_SELECTTOOLBARBITMAP:
		{
			CFileDialog dialog(TRUE, _T("eMuleToolbar.bmp"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("*.eMuleToolbar.bmp|*.eMuleToolbar.bmp||"));
			if (IDOK == dialog.DoModal())
			{
				if (g_App.m_pPrefs->GetToolbarBitmapSettings() != dialog.GetPathName())
				{
					g_App.m_pPrefs->SetToolbarBitmapSettings(dialog.GetPathName());
					ChangeToolbarBitmap(dialog.GetPathName(), true);
				}
			}
			break;
		}
		case MP_NOTEXTLABELS:
		{
			ChangeTextLabelStyle(0, true);
			g_App.m_pPrefs->SetToolbarLabelSettings(0);
			break;
		}
		case MP_TEXTLABELS:
		{
			ChangeTextLabelStyle(1, true);
			g_App.m_pPrefs->SetToolbarLabelSettings(1);
			break;
		}
		case MP_TEXTLABELSONRIGHT:
		{
			ChangeTextLabelStyle(2, true);
			g_App.m_pPrefs->SetToolbarLabelSettings(2);
			break;
		}
		default:
		{
			if (wParam >= MP_TOOLBARBITMAP && wParam <= MP_TOOLBARBITMAP + 50)
			{
				if (g_App.m_pPrefs->GetToolbarBitmapSettings() != m_strBitmapPaths[wParam - MP_TOOLBARBITMAP])
				{
					g_App.m_pPrefs->SetToolbarBitmapSettings(m_strBitmapPaths[wParam - MP_TOOLBARBITMAP]);
					ChangeToolbarBitmap(m_strBitmapPaths[wParam - MP_TOOLBARBITMAP], true);
				}
			}
			break;
		}
	}

	return true;

	EMULE_CATCH

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::ChangeTextLabelStyle(int settings, bool refresh)
{
	if (m_iToolbarLabelSettings != settings)
	{
		switch (settings)
		{
			case 0:
				SetStyle(GetStyle() & ~TBSTYLE_LIST);
				SetMaxTextRows(0);
				break;
			case 1:
				SetStyle(GetStyle() & ~TBSTYLE_LIST);
				SetMaxTextRows(1);
				break;
			case 2:
				SetStyle(GetStyle() | TBSTYLE_LIST);
				SetMaxTextRows(1);
				break;
		}
		if ((m_iToolbarLabelSettings + settings) != 2) //if not changing between no labels and labels on right
		{
			for (int i = 0; i < MTB_NUMBUTTONS; i++)
			{
				TBBUTTONINFO		buttonInfo;

				buttonInfo.cbSize=sizeof(buttonInfo);
				buttonInfo.dwMask=TBIF_STYLE;
				GetButtonInfo(IDC_TOOLBARBUTTON + i, &buttonInfo);

				if (settings == 1)
					buttonInfo.fsStyle &= ~TBSTYLE_AUTOSIZE;
				else
					buttonInfo.fsStyle |= TBSTYLE_AUTOSIZE;
				SetButtonInfo(IDC_TOOLBARBUTTON + i, &buttonInfo);
			}
		}
		m_iToolbarLabelSettings = settings;
		if (refresh)
			Refresh();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::Refresh()
{
	SetBtnWidth();
	AutoSize();

	CRect		rToolbarRect;

	GetWindowRect(&rToolbarRect);

	static int	previousheight = 0;

	if (previousheight == rToolbarRect.Height())
	{
		Invalidate();
		RedrawWindow();
	}
	else
	{
		previousheight = rToolbarRect.Height();
#ifdef USE_REBAR
		REBARBANDINFO	rbbi;
		CSize			sizeBar;

		GetMaxSize(&sizeBar);
		rbbi.cbSize = sizeof(rbbi);
		rbbi.fMask = RBBIM_SIZE | RBBIM_CHILDSIZE;
		rbbi.cxMinChild = 0;
		rbbi.cyMinChild = sizeBar.cy;
		rbbi.cx = 0;
		g_App.m_pMDlg->m_ctlReBar.SetBandInfo(0, &rbbi);

		CRect		rReBarRect;

		g_App.m_pMDlg->m_ctlReBar.GetWindowRect(&rReBarRect);

		CRect		rClientRect;

		g_App.m_pMDlg->GetClientRect(&rClientRect);

		CRect		rStatusbarRect;

		g_App.m_pMDlg->m_ctlStatusBar.GetWindowRect(&rStatusbarRect);
		rClientRect.top += rReBarRect.Height();
		rClientRect.bottom -= rStatusbarRect.Height();
#else
		CRect		rClientRect;

		g_App.m_pMDlg->GetClientRect(&rClientRect);

		CRect		rStatusbarRect;

		g_App.m_pMDlg->m_ctlStatusBar.GetWindowRect(&rStatusbarRect);
		rClientRect.top += rToolbarRect.Height();
		rClientRect.bottom -= rStatusbarRect.Height();
#endif
		static CWnd *const	s_pwndWndsTbl[] =
		{
			&g_App.m_pMDlg->m_wndServer,
			&g_App.m_pMDlg->m_wndTransfer,
			&g_App.m_pMDlg->m_wndSharedFiles,
			&g_App.m_pMDlg->m_dlgSearch,
			&g_App.m_pMDlg->m_wndChat,
			&g_App.m_pMDlg->m_wndIRC,
			&g_App.m_pMDlg->m_dlgStatistics
		};

		for (int i = 0; i < _countof(s_pwndWndsTbl); i++)
		{
			s_pwndWndsTbl[i]->SetWindowPos(NULL, rClientRect.left, rClientRect.top, rClientRect.Width(), rClientRect.Height(), SWP_NOZORDER);
			g_App.m_pMDlg->RemoveAnchor(s_pwndWndsTbl[i]->m_hWnd);
			g_App.m_pMDlg->AddAnchor(s_pwndWndsTbl[i]->m_hWnd, TOP_LEFT, BOTTOM_RIGHT);
		}
		g_App.m_pMDlg->Invalidate();
		g_App.m_pMDlg->RedrawWindow();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnTbnReset(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR); NOPRM(pResult);
	int			i, iIdx;
	TBBUTTON	sepButton;

//	First get rid of old buttons while saving their states
	for (i = GetButtonCount() - 1; i >= 0; i--)
		DeleteButton(i);

	sepButton.idCommand = 0;
	sepButton.fsStyle = TBSTYLE_SEP;
	sepButton.fsState = TBSTATE_ENABLED;
	sepButton.iString = -1;
	sepButton.iBitmap = -1;

//	Set default configuration
	for (i = 0; i < g_App.m_pPrefs->GetDefaultToolbarSettings().GetLength(); i += 2)
	{
		iIdx = _tstoi(g_App.m_pPrefs->GetDefaultToolbarSettings().Mid(i, 2));
		AddButtons(1, (iIdx == 99) ? &sepButton : &m_buttonDefs[iIdx]);
	}
//	Save new (default) configuration
	g_App.m_pPrefs->SetToolbarSettings(g_App.m_pPrefs->GetDefaultToolbarSettings());

	Localize();		// we have to localize the button-text
#ifdef OLD_SOCKETS_ENABLED
	g_App.m_pMDlg->ShowConnectionState(g_App.m_pServerConnect->IsConnected(), _T(""), true);
#endif //OLD_SOCKETS_ENABLED
	m_iToolbarLabelSettings = 4;
	ChangeTextLabelStyle(g_App.m_pPrefs->GetToolbarLabelSettings(), false);
	SetBtnWidth();		// then calc and set the button width
	AutoSize();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMuleToolbarCtrl::OnTbnInitCustomize(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);
	TBBUTTON	Button;
	unsigned	uiIdx;

//	Update saved button statuses
	for (int i = GetButtonCount() - 1; i >= 0; i--)
	{
		GetButton(i, &Button);
		if ((uiIdx = (Button.idCommand - IDC_TOOLBARBUTTON)) < static_cast<unsigned>(MTB_NUMBUTTONS))
		{
			m_buttonDefs[uiIdx].fsState = Button.fsState;
			m_buttonDefs[uiIdx].fsStyle = Button.fsStyle;
		}
	}
	*pResult = TBNRF_HIDEHELP;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
