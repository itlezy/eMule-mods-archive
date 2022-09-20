// HtmlCtrl.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "WebTool.h"
#include "WebBrowser.h"

// CHtmlCtrl

IMPLEMENT_DYNCREATE(CHtmlCtrl, CHtmlView)


void CHtmlCtrl::DoDataExchange(CDataExchange* pDX)
{
	CHtmlView::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CHtmlCtrl, CHtmlView)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_MOUSEACTIVATE()
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CHtmlCtrl 诊断

#ifdef _DEBUG
void CHtmlCtrl::AssertValid() const
{
	CHtmlView::AssertValid();
}

void CHtmlCtrl::Dump(CDumpContext& dc) const
{
	CHtmlView::Dump(dc);
}
#endif //_DEBUG


// CHtmlCtrl 消息处理程序
BOOL CHtmlCtrl::CreateFromStatic(UINT nID, CWnd* pParent)
{
	CStatic wndStatic;
	if (!wndStatic.SubclassDlgItem(nID, pParent))
		return FALSE;

	// Get static control rect, convert to parent's client coords.
	CRect rc;
	wndStatic.GetWindowRect(&rc);
	pParent->ScreenToClient(&rc);
	wndStatic.DestroyWindow();

	// create HTML control (CHtmlView)
	return Create(NULL,						 // class name
		NULL,										 // title
		(WS_CHILD | WS_VISIBLE ),			 // style
		rc,										 // rectangle
		pParent,									 // parent
		nID,										 // control ID
		NULL);									 // frame/doc context not used
}

//开始链接时会触发此事件
void CHtmlCtrl::OnBeforeNavigate2(LPCTSTR lpszURL, DWORD /*nFlags*/, LPCTSTR /*lpszTargetFrameName*/, CByteArray& /*baPostedData*/, LPCTSTR /*lpszHeaders*/, BOOL* pbCancel)
{
	((CWebBrowserWnd*)GetParent())->SetAddress(lpszURL);
		pbCancel = FALSE;
}

void CHtmlCtrl::OnDestroy()
{
	// This is probably unnecessary since ~CHtmlView does it, but
	// safer to mimic CHtmlView::OnDestroy.
	if (m_pBrowserApp) 
	{
		m_pBrowserApp = NULL;
	}
	CWnd::OnDestroy(); // bypass CView doc/frame stuff
}

void CHtmlCtrl::OnDocumentComplete(LPCTSTR /*lpszURL*/)
{
	CWebBrowserWnd* pBrowser = ((CWebBrowserWnd*)GetParent());
    pBrowser->StopAnimation();
}

BOOL CHtmlCtrl::OnEraseBkgnd(CDC* /*pDC*/)
{
	return true;
}

int CHtmlCtrl::OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message)
{
	return CHtmlView::OnMouseActivate(pDesktopWnd, nHitTest, message);
}

void CHtmlCtrl::OnNavigateComplete2(LPCTSTR /*strURL*/)
{
	CWebBrowserWnd* pBrowser = ((CWebBrowserWnd*)GetParent());
	pBrowser->SetAddress(GetLocationURL());
}

void CHtmlCtrl::OnSize(UINT nType, int cx, int cy)
{
	CFormView::OnSize(nType, cx, cy);			//Changed by thilon on 2006.08.10

	if (::IsWindow(m_wndBrowser.m_hWnd)) 
	{ 
		CRect rect; 
		GetClientRect(rect); 
		// 就这一句与CHtmlView的不同
		::AdjustWindowRectEx(rect, GetStyle(), FALSE, WS_EX_CLIENTEDGE);
		m_wndBrowser.SetWindowPos(NULL, rect.left, rect.top, rect.Width(), rect.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
	} 
}

void CHtmlCtrl::OnStatusTextChange(LPCTSTR lpszText)
{
	((CWebBrowserWnd*)GetParent())->m_status.SetWindowText(lpszText);
}

void CHtmlCtrl::PostNcDestroy()
{
	CHtmlView::PostNcDestroy();
}
