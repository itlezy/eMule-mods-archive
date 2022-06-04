//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "TrayDialog.h"
#include "emuledlg.h"
#include "MenuCmds.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	NOTIFYICONDATA_V2_TIP_SIZE	128
#define	NOTIFYICONDATA_TITLE_SIZE	64
#define	NOTIFYICONDATA_INFO_SIZE	256

/////////////////////////////////////////////////////////////////////////////
// CTrayDialog dialog

const UINT WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));

BEGIN_MESSAGE_MAP(CTrayDialog, CResizableDialog)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_REGISTERED_MESSAGE(WM_TASKBARCREATED, OnTaskBarCreated)
	ON_WM_TIMER()
END_MESSAGE_MAP()

CTrayDialog::CTrayDialog(UINT uIDD,CWnd* pParent /*=NULL*/)
	: CResizableDialog(uIDD, pParent)
{
	m_nidIconData.cbSize = NOTIFYICONDATA_V2_SIZE;
	m_nidIconData.hWnd = 0;
	m_nidIconData.uID = 1;
	m_nidIconData.uFlags = 0;
	m_nidIconData.uCallbackMessage = UM_TRAY_ICON_NOTIFY_MESSAGE;
	m_nidIconData.hIcon = 0;
	m_nidIconData.szTip[0] = _T('\0');
	m_nidIconData.szInfo[0] = _T('\0');
	m_nidIconData.szInfoTitle[0] = _T('\0');
	m_bTrayIconVisible = FALSE;
	m_uBallonTimer = 0;
}

int CTrayDialog::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CResizableDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	ASSERT( WM_TASKBARCREATED );
	m_nidIconData.hWnd = m_hWnd;
	m_nidIconData.uID = 1;
	return 0;
}

void CTrayDialog::OnDestroy()
{
	KillBallonTimer();
	CResizableDialog::OnDestroy();

	// shouldn't that be done before passing the message to DefWinProc?
	if (m_nidIconData.hWnd && m_nidIconData.uID > 0 && TrayIsVisible())
	{
		VERIFY( Shell_NotifyIcon(NIM_DELETE, &m_nidIconData) );
	}
}

void CTrayDialog::TraySetIcon(HICON hIcon)
{
	ASSERT( hIcon );
	if (hIcon)
	{
		m_nidIconData.hIcon = hIcon;

		if (m_bTrayIconVisible)
		{
			m_nidIconData.uFlags = NIF_ICON;
			Shell_NotifyIcon(NIM_MODIFY, &m_nidIconData);
		}
    
	}
}

void CTrayDialog::TraySetToolTip(LPCTSTR lpszToolTip)
{
	//ASSERT( _tcslen(lpszToolTip) > 0 && _tcslen(lpszToolTip) < NOTIFYICONDATA_V2_TIP_SIZE );
	_tcsncpy(m_nidIconData.szTip, lpszToolTip, NOTIFYICONDATA_V2_TIP_SIZE);
	m_nidIconData.szTip[NOTIFYICONDATA_V2_TIP_SIZE - 1] = _T('\0');
	m_nidIconData.uFlags = NIF_TIP;

	Shell_NotifyIcon(NIM_MODIFY, &m_nidIconData);
}

void CTrayDialog::TraySetBalloonToolTip(LPCTSTR lpszTitle, LPCTSTR lpszInfo, UINT infoflag)
{
	if(m_bTrayIconVisible){
		if(lpszInfo[0] != 0){
			KillBallonTimer();
			m_uBallonTimer = SetTimer(IDT_BALLON, 5000, NULL);
}

		ASSERT(_tcslen(lpszTitle) < NOTIFYICONDATA_TITLE_SIZE );
		_tcsncpy(m_nidIconData.szInfoTitle, lpszTitle, NOTIFYICONDATA_TITLE_SIZE);

		//ASSERT(_tcslen(lpszInfo) < NOTIFYICONDATA_INFO_SIZE );
		_tcsncpy(m_nidIconData.szInfo, lpszInfo, NOTIFYICONDATA_INFO_SIZE);
		m_nidIconData.szInfo[NOTIFYICONDATA_INFO_SIZE - 1] = 0;
		m_nidIconData.uFlags = NIF_INFO;
		//m_nidIconData.uTimeout = 10000;// X: not work
		m_nidIconData.dwInfoFlags = infoflag;

	Shell_NotifyIcon(NIM_MODIFY, &m_nidIconData);
}
}

BOOL CTrayDialog::TrayShow()
{
	BOOL bSuccess = FALSE;
	//if (!m_bTrayIconVisible)
	{
		m_nidIconData.uFlags = NIF_MESSAGE | NIF_ICON;
		m_nidIconData.uVersion = NOTIFYICON_VERSION;
		bSuccess = Shell_NotifyIcon(NIM_ADD, &m_nidIconData);
		if (bSuccess)
			m_bTrayIconVisible = TRUE;
	}
	return bSuccess;
}

BOOL CTrayDialog::TrayHide()
{
	BOOL bSuccess = FALSE;
	if (m_bTrayIconVisible)
	{
		bSuccess = Shell_NotifyIcon(NIM_DELETE, &m_nidIconData);
		if (bSuccess)
			m_bTrayIconVisible = FALSE;
	}
	return bSuccess;
}

void CTrayDialog::KillBallonTimer()
{
	if (m_uBallonTimer)
{
		VERIFY( KillTimer(m_uBallonTimer) );
		m_uBallonTimer = 0;
}
}

void CTrayDialog::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == m_uBallonTimer)
{
		KillBallonTimer();
		TraySetBalloonToolTip(_T(""), _T(""));
}
	CResizableDialog::OnTimer(nIDEvent);
}

LRESULT CTrayDialog::OnTaskBarCreated(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	if (m_bTrayIconVisible)
	{
		m_nidIconData.uFlags = NIF_MESSAGE | NIF_ICON;
		m_nidIconData.uVersion = NOTIFYICON_VERSION;
		BOOL bResult = Shell_NotifyIcon(NIM_ADD, &m_nidIconData);
		if (!bResult)
			m_bTrayIconVisible = false;
	}
	return 0;
}

