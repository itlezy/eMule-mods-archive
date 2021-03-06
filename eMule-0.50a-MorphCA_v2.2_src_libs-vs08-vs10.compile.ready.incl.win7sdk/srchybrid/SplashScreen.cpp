//this file is part of eMule
//Copyright (C)2002-2010 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "SplashScreen.h"
#include "OtherFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CSplashScreen, CDialog)

BEGIN_MESSAGE_MAP(CSplashScreen, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()

CSplashScreen::CSplashScreen(CWnd* pParent /*=NULL*/)
	: CDialog(CSplashScreen::IDD, pParent)
{
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
	InitWindowStyles(this);

	VERIFY( m_imgSplash.Attach(theApp.LoadImage(_T("SPLASH"), _T("JPG"))) );
	if (m_imgSplash.GetSafeHandle())
	{
		BITMAP bmp = {0};
		if (m_imgSplash.GetBitmap(&bmp) > 0)
		{
			WINDOWPLACEMENT wp;
			GetWindowPlacement(&wp);
			wp.rcNormalPosition.right = wp.rcNormalPosition.left + bmp.bmWidth;
			wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + bmp.bmHeight;
			SetWindowPlacement(&wp);
		}
	}

	return TRUE;
}

BOOL CSplashScreen::PreTranslateMessage(MSG* pMsg)
{
	BOOL bResult = CDialog::PreTranslateMessage(pMsg);

	if ((pMsg->message == WM_KEYDOWN	   ||
		 pMsg->message == WM_SYSKEYDOWN	   ||
		 pMsg->message == WM_LBUTTONDOWN   ||
		 pMsg->message == WM_RBUTTONDOWN   ||
		 pMsg->message == WM_MBUTTONDOWN   ||
		 pMsg->message == WM_NCLBUTTONDOWN ||
		 pMsg->message == WM_NCRBUTTONDOWN ||
		 pMsg->message == WM_NCMBUTTONDOWN))
	{
		DestroyWindow();
	}

	return bResult;
}

void CSplashScreen::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (m_imgSplash.GetSafeHandle())
	{
		CDC dcMem;
		if (dcMem.CreateCompatibleDC(&dc))
		{
			CBitmap* pOldBM = dcMem.SelectObject(&m_imgSplash);
			BITMAP BM;
			m_imgSplash.GetBitmap(&BM);
			dc.BitBlt(0, 0, BM.bmWidth, BM.bmHeight, &dcMem, 0, 0, SRCCOPY);
			if (pOldBM)
				dcMem.SelectObject(pOldBM);
			long htext = dc.GetTextExtent(_T("T")).cy-2;
			CRect rc(2, BM.bmHeight - htext, BM.bmWidth, BM.bmHeight);//Commander - Changed

			//Commander - Removed
			//dc.FillSolidRect(rc.left+1, rc.top+1, rc.Width()-2, rc.Height()-2, RGB(255,255,255));

            rc.top += -6;//morph4u 
			LOGFONT lf = {0};
			lf.lfHeight = 15;//morph4u 14->15
			lf.lfWeight = FW_BOLD;
			/* morph no win98
			lf.lfQuality = afxIsWin95 ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
			*/
			lf.lfQuality = ANTIALIASED_QUALITY;
			// end
			_tcscpy(lf.lfFaceName, _T("Arial"));
			COLORREF oldclr = dc.SetTextColor(RGB(192,192,192));//Commander: Set white text color
			int iOMode = dc.SetBkMode(TRANSPARENT);//Commander: Make bg transparent
			CFont font;
			font.CreateFontIndirect(&lf);
			CFont* pOldFont = dc.SelectObject(&font);
			CString strAppVersion(_T("eMule ") + theApp.m_strCurVersionLong);
			dc.DrawText(strAppVersion, &rc, DT_CENTER | DT_NOPREFIX | DT_TOP);
			if (pOldFont)
				dc.SelectObject(pOldFont);
			dc.SetBkMode(iOMode);
			dc.SetTextColor(oldclr);
			font.DeleteObject();
		}
	}
}
