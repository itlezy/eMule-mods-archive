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
#include "resource.h"
#include "opcodes.h"
#include "memcpy_amd.h"
#include "SplashScreen.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CSplashScreen, CDialog)
CSplashScreen::CSplashScreen(CWnd* pParent /*=NULL*/)
	: CDialog(CSplashScreen::IDD, pParent)
{
	m_bLButtonDown = false;
}

CSplashScreen::~CSplashScreen()
{
	m_imgSplash.DeleteObject();
}

void CSplashScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CSplashScreen::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_Layered.AddLayeredStyle(m_hWnd);
	m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, 0, LWA_ALPHA);

	VERIFY(m_imgSplash.LoadImage(IDR_LOGO, _T("JPG")));
	(static_cast<CStatic*>(GetDlgItem(IDC_BACKGROUND)))->SetBitmap(m_imgSplash);

#ifndef _DEBUG
#define VERSION_FONTSIZE _T("22")
#else
#define VERSION_FONTSIZE _T("14")
#endif //_DEBUG

	CString s =
		_T("<font face='arial' size='10' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>")
		_T("<br><br><br>")
		_T("<font face='Arial Bold' size='22' style='b' color='0,0,0' background='255,255,255' align='center' valign='bottom'>") CLIENT_NAME _T("</font>")
		_T("<br>")
		_T("<font face='Arial Bold' size='") VERSION_FONTSIZE _T("' style='b' color='128,128,128' background='255,255,255' align='center' valign='bottom'>v") CURRENT_VERSION_LONG;
	switch (get_cpu_type())
	{
		case 2:
			s += _T(" MMX");
			break;
		case 3:
			s += _T(" AMD");
			break;
		case 4:
		case 5:
			s += _T(" SSE");
			break;
	}
	s += _T("</font><br>")
		_T("<font face='Verdana Regular' size='11' style='u | -b' color='0,0,255' background='255,255,255' align='center' valign='bottom'><a href='http://emuleplus.info'>emuleplus.info</a></font>");

	m_ctrlText.SetDataString(s);
	m_ctrlText.SetDefaultBkColor(RGB(255,255,255));

//	Resize dialog window to the picture size to look well for any system font size
	CRect rWndRect, rBmpRect;

	GetWindowRect(rWndRect);
	GetDlgItem(IDC_BACKGROUND)->GetWindowRect(rBmpRect);
	MoveWindow(rWndRect.left, rWndRect.top, rBmpRect.Width(), rBmpRect.Height(), FALSE);

	CRect rClient;
	GetClientRect(rClient);
	rClient.top = 147;
	rClient.left += 10;
	rClient.bottom = rClient.top + 120;
	rClient.right = rClient.left + 220;

	m_ctrlText.m_nTimerSpeed = 0;
	m_ctrlText.Create(NULL, WS_CHILD | WS_VISIBLE, rClient, this);

	m_iTranslucency = 0;
	m_nTimeOut = 0;
	SetTimer(1, (256/7)+2, 0);
	SetCapture();
	return true;
}

BEGIN_MESSAGE_MAP(CSplashScreen, CDialog)
	ON_WM_TIMER()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CSplashScreen::OnTimer(UINT nIDEvent)
{
	if (m_iTranslucency < 255)
	{
		m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, static_cast<BYTE>(m_iTranslucency), LWA_ALPHA);
		m_iTranslucency += 7;
	}
	else
	{
		if (m_nTimeOut == 0)
		{
			m_nTimeOut = SetTimer(300, 1500, 0);
			m_Layered.SetLayeredWindowAttributes(m_hWnd, 0, 255, LWA_ALPHA);
		}
		else
		{
			if(nIDEvent == m_nTimeOut)
			{
				KillTimer(m_nTimeOut);
				ReleaseCapture();
				OnOK();
				GetParent()->RedrawWindow();
			}
		}
	}
	CDialog::OnTimer(nIDEvent);
}

void CSplashScreen::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	OnTimer(m_nTimeOut);
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CSplashScreen::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rClient;
	GetClientRect(rClient);
	if(point.x >= rClient.left && point.x <= rClient.right &&
		point.y >= rClient.top && point.y <= rClient.bottom)
	{
		m_bLButtonDown = true;
	}
	CDialog::OnLButtonDown(nFlags, point);
}

void CSplashScreen::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rClient;
	GetClientRect(rClient);
	if(point.x >= rClient.left && point.x <= rClient.right &&
		point.y >= rClient.top && point.y <= rClient.bottom && m_bLButtonDown)
	{
		OnTimer(m_nTimeOut);
	}

	m_bLButtonDown = false;
	CDialog::OnLButtonUp(nFlags, point);
}
