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
#include "Preferences.h"

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
	m_phtmlAds = NULL;
    m_pimgSplash = NULL;
}

CSplashScreen::~CSplashScreen()
{
    if (NULL != m_pimgSplash)
    {
        m_pimgSplash->DeleteObject();      
        delete m_pimgSplash;     
        m_pimgSplash = NULL;
    }
}

void CSplashScreen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CSplashScreen::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);

	if (thePrefs.adsDisable2)
    {// Disable the advertisment now.
	    //Mod Adu
	    //Ded
	    //Splash screen remoto
        if (NULL == m_pimgSplash)
            m_pimgSplash = new CEnBitmap;
        if (NULL == m_pimgSplash)
            return FALSE;
        
		if (!m_pimgSplash->LoadImage(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("splash.jpg")))
		
			VERIFY(m_pimgSplash->LoadImage(_T("ABOUT"),_T("JPG")));
	    //Fine Mod Adu

	    if (m_pimgSplash->GetSafeHandle())
	    {
		    BITMAP bmp = {0};
		    if (m_pimgSplash->GetBitmap(&bmp) > 0)
		    {
			    WINDOWPLACEMENT wp;
			    GetWindowPlacement(&wp);
			    wp.rcNormalPosition.right = wp.rcNormalPosition.left + bmp.bmWidth;
			    wp.rcNormalPosition.bottom = wp.rcNormalPosition.top + bmp.bmHeight;
			    SetWindowPlacement(&wp);
		    }
	    }
    }
    else
    {// Allow it.
        MoveWindow(0,0, 365, 315);
        if (NULL == m_phtmlAds)
            m_phtmlAds = new CHtmlCtrl;
        if (NULL == m_phtmlAds)
            return FALSE;
	    CRect rc;
        GetClientRect(&rc);
        m_phtmlAds->Create(rc, this, IDC_HTMLVIEW_ADS);
	    m_phtmlAds->LoadFromResource(IDR_HTML_SPLASHWND);			  // load HTML from resource
    }
    return TRUE;
}

BOOL CSplashScreen::PreTranslateMessage(MSG* pMsg)
{
	if (thePrefs.adsDisable2)	{
		if ((pMsg->message == WM_KEYDOWN	   ||
			pMsg->message == WM_SYSKEYDOWN	   ||
			pMsg->message == WM_LBUTTONDOWN	   ||
			pMsg->message == WM_RBUTTONDOWN    ||
			pMsg->message == WM_MBUTTONDOWN    ||
			pMsg->message == WM_NCLBUTTONDOWN  ||
			pMsg->message == WM_NCRBUTTONDOWN  ||
			pMsg->message == WM_NCMBUTTONDOWN))
	{
		DestroyWindow();
	}
        
		if (pMsg->message ==  WM_PAINT && pMsg->hwnd == m_hWnd)
        {
            OnPaint();
        }
    }

	return CDialog::PreTranslateMessage(pMsg);
}

void CSplashScreen::OnPaint() 
{	
	if (!thePrefs.adsDisable2)
		return;

	CPaintDC dc(this); // device context for painting

	if (m_pimgSplash->GetSafeHandle())
	{
		CDC dcMem;
		if (dcMem.CreateCompatibleDC(&dc))
		{
			CBitmap* pOldBM = dcMem.SelectObject(m_pimgSplash);
			BITMAP BM;
			m_pimgSplash->GetBitmap(&BM);
			dc.BitBlt(0, 0, BM.bmWidth, BM.bmHeight, &dcMem, 0, 0, SRCCOPY);
			if (pOldBM)
				dcMem.SelectObject(pOldBM);

			CRect rc(0, (int)(BM.bmHeight * 0.8), BM.bmWidth, BM.bmHeight);
			dc.FillSolidRect(rc.left+1, rc.top+1, rc.Width()-2, rc.Height()-2, RGB(255,255,255));

			LOGFONT lf = {0};
			lf.lfHeight = 30;
			lf.lfWeight = FW_BOLD;
			lf.lfQuality = afxIsWin95() ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
			_tcscpy(lf.lfFaceName, _T("MS Shell Dlg"));
			CFont font;
			font.CreateFontIndirect(&lf);
			CFont* pOldFont = dc.SelectObject(&font);
			CString strAppVersion(_T("eMule ") + theApp.m_strCurVersionLong);
			rc.top += dc.DrawText(strAppVersion, &rc, DT_CENTER | DT_NOPREFIX);
			if (pOldFont)
				dc.SelectObject(pOldFont);
			font.DeleteObject();

			rc.top += 10;

			lf.lfHeight = 15;
			lf.lfWeight = FW_NORMAL;
			lf.lfQuality = afxIsWin95() ? NONANTIALIASED_QUALITY : ANTIALIASED_QUALITY;
			_tcscpy(lf.lfFaceName, _T("MS Shell Dlg"));
			font.CreateFontIndirect(&lf);
			pOldFont = dc.SelectObject(&font);
			dc.DrawText(_T("Hammon, Anis Hireche, tigerjact"), &rc, DT_CENTER | DT_NOPREFIX);
			if (pOldFont)
				dc.SelectObject(pOldFont);
			font.DeleteObject();
		}
	}
}
