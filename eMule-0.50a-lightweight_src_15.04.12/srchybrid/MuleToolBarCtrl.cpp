//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "emule.h"
#include "MuleToolbarCtrl.h"
#include "SearchDlg.h"
#include "OtherFunctions.h"
#include "Sockets.h"
#include "MenuCmds.h"
#include "MuleStatusbarCtrl.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "StatisticsDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define	NUM_BUTTON	6
#define	NUM_BUTTON_BITMAPS	5

// CMuleToolbarCtrl

IMPLEMENT_DYNAMIC(CMuleToolbarCtrl, CToolBarCtrlX)
 
BEGIN_MESSAGE_MAP(CMuleToolbarCtrl, CToolBarCtrlX)
	//ON_WM_SYSCOLORCHANGE()
	//ON_WM_SETTINGCHANGE()
END_MESSAGE_MAP()


CMuleToolbarCtrl::CMuleToolbarCtrl()
{
	m_iLastPressedButton = -1;
	m_buttoncount = 0;
}

CMuleToolbarCtrl::~CMuleToolbarCtrl()
{
}

void CMuleToolbarCtrl::Init(void)
{
	// Win98: Explicitly set to Unicode to receive Unicode notifications.
	//SendMessage(CCM_SETUNICODEFORMAT, TRUE);

	ModifyStyle(0, TBSTYLE_FLAT | TBSTYLE_ALTDRAG | CCS_ADJUSTABLE | TBSTYLE_TRANSPARENT | TBSTYLE_TOOLTIPS | CCS_NODIVIDER);

	ChangeToolbarBitmap(false);

	SetMaxTextRows(0);
	// initialize buttons:	
	TBBUTTON TBButtons[NUM_BUTTON];
	memset(&TBButtons, 0, sizeof(TBBUTTON));
	TBButtons[0].iBitmap = 0;
	TBButtons[0].idCommand	= IDC_TOOLBARBUTTON;
	TBButtons[0].fsState	= TBSTATE_ENABLED;
	TBButtons[0].fsStyle = TBSTYLE_CHECKGROUP &~TBSTYLE_AUTOSIZE;
	TBButtons[0].iString	= -1;
	for(size_t i = 1; ; i++){
		memcpy(&TBButtons[i], &TBButtons[i-1], sizeof(TBBUTTON));
		if(i >= 5) break;
		++TBButtons[i].iBitmap;
		++TBButtons[i].idCommand;
	}
	TBButtons[4].idCommand	= 0;
	TBButtons[4].fsState	= 0;
	TBButtons[4].fsStyle = TBSTYLE_SEP;
	TBButtons[5].fsStyle = TBSTYLE_BUTTON;
    AddButtons(_countof(TBButtons), TBButtons);
	
	// recalc toolbar-size
	//SetButtonWidth(16, 16);
	//SetButtonSize(CSize(16, 16));
	Localize();
}

void CMuleToolbarCtrl::SetAllButtonsStrings()
{
	static const int TBStringIDs[NUM_BUTTON_BITMAPS] =
	{
		IDS_SERVER,
		IDS_EM_TRANS,
		IDS_EM_SEARCH,
		IDS_EM_STATISTIC,
		IDS_EM_PREFS
	};
	for (int i = 0; i < NUM_BUTTON_BITMAPS; i++)
		SetBtnText(IDC_TOOLBARBUTTON+i, GetResString(TBStringIDs[i]));
}

void CMuleToolbarCtrl::Localize()
{
	if (m_hWnd)
	{
		SetAllButtonsStrings();
	}
}

void CMuleToolbarCtrl::ChangeToolbarBitmap(bool bRefresh)
{
	CImageList ImageList;
	// load from icon ressources
	ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	ImageList.Add(CTempIconLoader(_T("KadServer"), 16, 16));
	ImageList.Add(CTempIconLoader(_T("TRANSFER"), 16, 16));
	ImageList.Add(CTempIconLoader(_T("SEARCH"), 16, 16));
	ImageList.Add(CTempIconLoader(_T("STATISTICS"), 16, 16));
	ImageList.Add(CTempIconLoader(_T("PREFERENCES"), 16, 16));
	ASSERT( ImageList.GetImageCount() == NUM_BUTTON_BITMAPS );
	CImageList* pimlOld = SetImageList(&ImageList);
	ImageList.Detach();
	if (pimlOld)
		pimlOld->DeleteImageList();
	if (bRefresh)
	{
	RedrawWindow();
}
}
/*
void CMuleToolbarCtrl::OnSysColorChange()
{
	CToolBarCtrlX::OnSysColorChange();
	ChangeToolbarBitmap(true);
}

void CMuleToolbarCtrl::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
{
	CToolBarCtrlX::OnSettingChange(uFlags, lpszSection);

	// Vista: There are certain situations where the toolbar control does not redraw/resize
	// correctly under Vista. Unfortunately Vista just sends a WM_SETTINGCHANGE when certain
	// system settings have changed. Furthermore Vista sends that particular message way 
	// more often than WinXP.
	// Whenever the toolbar control receives a WM_SETTINGCHANGE, it tries to resize itself 
	// (most likely because it thinks that some system font settings have changed). However, 
	// that resizing does fail when the toolbar control has certain non-standard metrics 
	// applied (see the table below).
	//
	// Toolbar configuration		Redraw due to WM_SETTINGCHANGE
	// ----------------------------------------------------------
	// Large Icons + No Text		Fail
	// Small Icons + No Text		Fail
	//
	// Large Icons + Text on Right	Ok
	// Small Icons + Text on Right	Fail
	//
	// Large Icons + Text on Bottom	Ok
	// Small Icons + Text on Bottom	Ok
	//
	// The problem with this kind of 'correction' is that the WM_SETTINGCHANGE message is
	// sometimes sent very often and we need to try to invoke our 'correction' code as seldom
	// as possible to avoid too much window flickering.
	//
	// The toolbar control seems to *not* evaluate the "lpszSection" parameter of the WM_SETTINGCHANGE
	// message to determine if it really needs to resize itself, it seems to just resize itself
	// whenever a WM_SETTINGCHANGE is received, regardless the value of that parameter. Thus, we can
	// not use the value of that parameter to limit the invokation of our correction code.
	//

	if (   theApp.m_ullComCtrlVer >= MAKEDLLVERULL(6,16,0,0)
		/*&& (m_eLabelType == NoLabels || (m_eLabelType == LabelsRight && m_sizBtnBmp.cx == 16))*) {
		ChangeToolbarBitmap(true);
	}
}
*/
void CMuleToolbarCtrl::PressMuleButton(int nID)
{
	// Customization might splits up the button-group, so we have to (un-)press them on our own
	if (m_iLastPressedButton != -1)
		CheckButton(m_iLastPressedButton, FALSE);
	CheckButton(nID, TRUE);
	m_iLastPressedButton = nID;
}

#ifdef _DEBUG
void CMuleToolbarCtrl::Dump()
{
	TRACE("---\n");
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	TRACE("Wnd =%4d,%4d-%4d,%4d (%4d x %4d)\n", rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom, rcWnd.Width(), rcWnd.Height());

	CRect rcClnt;
	GetClientRect(&rcClnt);
	TRACE("Clnt=%4d,%4d-%4d,%4d (%4d x %4d)\n", rcClnt.left, rcClnt.top, rcClnt.right, rcClnt.bottom, rcClnt.Width(), rcClnt.Height());

	// Total size of all of the visible buttons and separators in the toolbar.
	CSize siz;
	GetMaxSize(&siz);
	TRACE("MaxSize=                  %4d x %4d\n", siz.cx, siz.cy);

	int iButtons = GetButtonCount();	// Count of the buttons currently in the toolbar.
	int iRows = GetRows();				// Number of rows of buttons in a toolbar with the TBSTYLE_WRAPABLE style
	int iMaxTextRows = GetMaxTextRows();// Maximum number of text rows that can be displayed on a toolbar button.
	TRACE("ButtonCount=%d  Rows=%d  MaxTextRows=%d\n", iButtons, iRows, iMaxTextRows);

	// Current width and height of toolbar buttons, in pixels.
	DWORD dwButtonSize = GetButtonSize();
	TRACE("ButtonSize=%dx%d\n", LOWORD(dwButtonSize), HIWORD(dwButtonSize));

	// Padding for a toolbar control.
	LRESULT dwPadding = SendMessage(TB_GETPADDING);
	TRACE("Padding=%dx%d\n", LOWORD(dwPadding), HIWORD(dwPadding));

	DWORD dwBitmapFlags = GetBitmapFlags(); // TBBF_LARGE=0x0001
	TRACE("BitmapFlags=%u\n", dwBitmapFlags);

	// Bounding rectangle of a button in a toolbar.
	TRACE("ItemRects:");
	for (int i = 0; i < iButtons; i++)
	{
		CRect rcButton(0,0,0,0);
		GetItemRect(i, &rcButton);
		TRACE(" %2dx%2d", rcButton.Width(), rcButton.Height());
	}
	TRACE("\n");

	// Bounding rectangle for a specified toolbar button.
	TRACE("Rects    :");
	for (int i = 0; i < iButtons; i++)
	{
		CRect rcButton(0,0,0,0);
		GetRect(IDC_TOOLBARBUTTON + i, &rcButton);
		TRACE(" %2dx%2d", rcButton.Width(), rcButton.Height());
	}
	TRACE("\n");

	TRACE("Info     :");
	for (int i = 0; i < iButtons; i++)
	{
		TCHAR szLabel[256];
		TBBUTTONINFO tbi = {0};
		tbi.cbSize = sizeof(tbi);
		tbi.dwMask |= TBIF_BYINDEX | TBIF_COMMAND | TBIF_IMAGE | TBIF_LPARAM | TBIF_SIZE | TBIF_STATE | TBIF_STYLE | TBIF_TEXT;
		tbi.cchText = _countof(szLabel);
		tbi.pszText = szLabel;
		GetButtonInfo(i, &tbi);
		szLabel[_countof(szLabel) - 1] = _T('\0');
		TRACE(" %2d ", tbi.cx);
	}
	TRACE("\n");
}
#endif

BOOL CMuleToolbarCtrl::GetMaxSize(LPSIZE pSize) const
{
	BOOL bResult = CToolBarCtrlX::GetMaxSize(pSize);
	if (theApp.m_ullComCtrlVer <= MAKEDLLVERULL(5,81,0,0))
	{
		int iWidth = 0;
		int iButtons = GetButtonCount();
		for (int i = 0; i < iButtons; i++)
		{
			CRect rcButton;
			if (GetItemRect(i, &rcButton))
				iWidth += rcButton.Width();
		}
		if (iWidth > pSize->cx)
			pSize->cx = iWidth;
	}
	return bResult;
}