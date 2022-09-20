// webbrowserView.cpp : implementation of the CWebBrowser class
//

#include "stdafx.h"
#include "CWebBrowser.h"
#include ".\cwebbrowser.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWebBrowser

IMPLEMENT_DYNCREATE(CWebBrowser, CHtmlView)


// CWebBrowser construction/destruction

CWebBrowser::CWebBrowser()
{
	// TODO: add construction code here	 
}

CWebBrowser::~CWebBrowser()
{
}


// CWebBrowser printing
CWebBrowser* CWebBrowser::GetHtmlView()
{
	CWebBrowser* pView = new CWebBrowser;
	return pView;
}


// CWebBrowser message handlers

BEGIN_MESSAGE_MAP(CWebBrowser, CHtmlView)	
	ON_WM_SIZE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

void CWebBrowser::OnSize(UINT nType, int cx, int cy)
{		
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	//by tax99
	CRect rcClient;
	AfxGetMainWnd()->GetClientRect(&rcClient);
	SetWindowPos(NULL, rcClient.left+9, rcClient.top+ 103, rcClient.Width()-18, rcClient.Height()-123, SWP_NOZORDER);
	Invalidate();
	CHtmlView::OnSize(nType, cx, cy);
}


BOOL CWebBrowser::OnEraseBkgnd(CDC* pDC)
{
	// TODO: Add your message handler code here and/or call default
	CRect rc;
	GetClientRect(&rc);
	pDC->FillSolidRect(&rc , RGB(255,255,255) );
	//return CHtmlView::OnEraseBkgnd(pDC);
	return TRUE;
}
