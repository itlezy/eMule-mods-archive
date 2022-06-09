// COptionTree
//
// License
// -------
// This code is provided "as is" with no expressed or implied warranty.
// 
// You may use this code in a commercial product with or without acknowledgement.
// However you may not sell this code or any modification of this code, this includes 
// commercial libraries and anything else for profit.
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
//
// History:
// --------
//	See License.txt for full history information.
//
//
// Copyright (c) 1999-2002 
// ComputerSmarts.net 
// mattrmiller@computersmarts.net

#include "stdafx.h"
#include "OptionTreeItemHyperLink.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemHyperLink

COptionTreeItemHyperLink::COptionTreeItemHyperLink()
{
	// Initialize variables
	m_bFocus = FALSE;
	m_crLink = RGB(0, 0, 255);
	m_crHover = RGB(0, 0, 255);
	m_crVisited = RGB(0, 0, 255);
	m_bVisited = FALSE;
	m_dwOptions = 0;
	m_strLink = _T("");
	m_hLinkCursor = NULL;
	m_bHover = FALSE;
	m_rcHover = CRect(0, 0, 0, 0);
}

COptionTreeItemHyperLink::~COptionTreeItemHyperLink()
{
	// Reallocate
	if (m_fUnderlineFont.GetSafeHandle() != NULL)
	{
		m_fUnderlineFont.DeleteObject();
	}
	//eklmn: destroy copied cursor on exit
	if (m_hLinkCursor) DestroyCursor(m_hLinkCursor);
}


BEGIN_MESSAGE_MAP(COptionTreeItemHyperLink, CWnd)
	//{{AFX_MSG_MAP(COptionTreeItemHyperLink)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemHyperLink message handlers

BOOL COptionTreeItemHyperLink::CreateHyperlinkItem(DWORD dwOptions, CString strLink, COLORREF crLink, COLORREF crHover, COLORREF crVisited)
{
	// Declare variables
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	BOOL bRet = FALSE;
	LOGFONT lf;
	CFont *pFont;

	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Create window
	if (!IsWindow(GetSafeHwnd()))
	{
		// -- Create the edit view
		bRet = Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW)), _T(""), dwStyle, m_rcAttribute, m_otOption->GetCtrlParent(), GetCtrlID());

		// -- Setup window
		if (bRet == TRUE)
		{
			// -- -- Set font
			SetFont(m_otOption->GetNormalFont(), TRUE);

			// -- Get underline font
			pFont = m_otOption->GetNormalFont();
			pFont->GetLogFont(&lf);
			lf.lfUnderline = TRUE;
			m_fUnderlineFont.CreateFontIndirect(&lf);

			// -- -- Set colors
			SetLinkColor(crLink);
			SetHoverColor(crHover);
			SetVisitedColor(crVisited);

			// -- -- Set link
			SetLink(strLink);

			// -- -- Set default cursor
			SetDefaultCursor();

			// -- -- Save options
			m_dwOptions = dwOptions;

			// -- -- Set window position
			MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());

			// -- -- Show window
			ShowWindow(SW_HIDE);
		}
	}

	return bRet;
}

void COptionTreeItemHyperLink::SetLinkCursor(HCURSOR hCursor)
{
	// Validate
	if (hCursor == NULL)
	{
		SetDefaultCursor();
	}

	// Destroy current cursor
	if (m_hLinkCursor != NULL)
	{
		DestroyCursor(m_hLinkCursor);
	}

	// Save cursor
	m_hLinkCursor = hCursor;
}

void COptionTreeItemHyperLink::SetDefaultCursor()
{
	// Set cursor
    if (m_hLinkCursor == NULL)
    {
        // -- Get the windows directory
        CString strWndDir;
        GetWindowsDirectory(strWndDir.GetBuffer(MAX_PATH), MAX_PATH);
        strWndDir.ReleaseBuffer();

        strWndDir += _T("\\winhlp32.exe");

        // -- This retrieves cursor #106 from winhlp32.exe, which is a hand pointer
        HMODULE hModule = LoadLibrary(strWndDir);
        if (hModule) 
		{
            HCURSOR hHandCursor = ::LoadCursor(hModule, MAKEINTRESOURCE(106));
            if (hHandCursor)
			{
                m_hLinkCursor = CopyCursor(hHandCursor);
				DestroyCursor(hHandCursor);
			}
        }

        FreeLibrary(hModule);
    }
}

HCURSOR COptionTreeItemHyperLink::GetLinkCursor()
{
	return m_hLinkCursor;
}

BOOL COptionTreeItemHyperLink::GetVisited()
{
	return m_bVisited;
}

BOOL COptionTreeItemHyperLink::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void COptionTreeItemHyperLink::SetLinkColor(COLORREF crColor)
{
	m_crLink = crColor;
}

void COptionTreeItemHyperLink::SetHoverColor(COLORREF crColor)
{
	m_crHover = crColor;
}

void COptionTreeItemHyperLink::SetVisitedColor(COLORREF crColor)
{
	m_crVisited = crColor;
}

COLORREF COptionTreeItemHyperLink::GetLinkColor()
{
	return m_crLink;
}

COLORREF COptionTreeItemHyperLink::GetHoverColor()
{
	return m_crHover;
}

COLORREF COptionTreeItemHyperLink::GetVisitedColor()
{
	return m_crVisited;
}

CString COptionTreeItemHyperLink::GetLink()
{
	return m_strLink;
}

void COptionTreeItemHyperLink::SetLink(CString strLink)
{
	m_strLink = strLink;
}

BOOL COptionTreeItemHyperLink::GetOption(DWORD dwOption)
{
	// Return option
	return (m_dwOptions & dwOption) ? TRUE : FALSE;
}

void COptionTreeItemHyperLink::SetOption(DWORD dwOption, BOOL bSet)
{
	// Set option
	if (bSet == TRUE)
	{
		m_dwOptions |= dwOption;
	}
	else
	{
		m_dwOptions &= ~dwOption;
	}
}

void COptionTreeItemHyperLink::DrawAttribute(CDC *pDC, const RECT &rcRect)
{
	// Make sure options aren't NULL
	if (m_otOption == NULL)
	{
		return;
	}

	// Make sure there is a window
	if (!IsWindow(GetSafeHwnd()))
	{
		return;
	}

	// Set window position
	if (IsWindow(GetSafeHwnd()))
	{
		MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());
	}
}

void COptionTreeItemHyperLink::OnCommit()
{
}

void COptionTreeItemHyperLink::OnExpand(BOOL bExpand)
{
	// Show window
	if (bExpand == TRUE)
	{
		ShowWindow(SW_SHOW);
	}
	else
	{
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemHyperLink::OnRefresh()
{
	// Set the window positiion
	if (IsWindow(GetSafeHwnd()))
	{
		MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());
	}
}

void COptionTreeItemHyperLink::OnMove()
{
	// Set window position
	if (IsWindow(GetSafeHwnd()))
	{
		MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());
	}
}

void COptionTreeItemHyperLink::OnActivate()
{
	// Make sure window is valid
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Set window position
		MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());

		// -- Set focus
		SetFocus();

		// -- Go to link
		if (IsReadOnly() == FALSE)
		{
			ShellExecute(NULL, _T("open"), m_strLink, NULL, NULL, SW_SHOW);
		}
	}
}

void COptionTreeItemHyperLink::CleanDestroyWindow()
{
	// Destroy window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Destroy window
		DestroyWindow();
	}
}

void COptionTreeItemHyperLink::OnDeSelect()
{
}

void COptionTreeItemHyperLink::OnSelect()
{
}

void COptionTreeItemHyperLink::OnPaint() 
{
	// Declare variables
	HGDIOBJ hOldBrush;
	CRect rcClient;
	CPaintDC dc(this);
	CDC* pDCMem = new CDC;
	CFont *pOldFont;
	CRect rcText;
	CBitmap bpMem;
	CBitmap *bmOld;
	COLORREF rcOldColor;
	int nOldBk;
	COLORREF crOldBack;

	// Get client rectangle
	GetClientRect(rcClient);
	rcText = rcClient;

	// Create DC
	pDCMem->CreateCompatibleDC(&dc);

	// Create bitmap
	bpMem.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());

	// Select bitmap
	bmOld = pDCMem->SelectObject(&bpMem);

	// Select brush
	hOldBrush = dc.SelectObject(GetSysColorBrush(COLOR_WINDOW));

	// Fill background
	pDCMem->PatBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), PATCOPY);

	// Select fonts
	if ((GetOption(OT_HL_UNDERLINEHOVER) == TRUE && m_bHover == TRUE) || GetOption(OT_HL_UNDERLINE) == TRUE)
	{
		pOldFont = pDCMem->SelectObject(&m_fUnderlineFont);
	}
	else
	{
		pOldFont = pDCMem->SelectObject(m_otOption->GetNormalFont());
	}

    // Setbackground
    nOldBk = pDCMem->SetBkMode(TRANSPARENT);

	// Set background color
	crOldBack = pDCMem->SetBkColor(GetBackgroundColor());	

	// Set text color
	// -- Read only
	if (IsReadOnly() == TRUE)
	{
		rcOldColor = pDCMem->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	// -- Hover
	else if (m_bHover == TRUE && GetOption(OT_HL_HOVER) == TRUE)
	{
		rcOldColor = pDCMem->SetTextColor(m_crHover);
	}
	// -- Visited
	else if (m_bVisited == TRUE && GetOption(OT_HL_VISITED) == TRUE)
	{
		rcOldColor = pDCMem->SetTextColor(m_crVisited);
	}
	else
	{
		rcOldColor = pDCMem->SetTextColor(m_crLink);
	}

	// Draw text
	pDCMem->DrawText(m_strLink, rcText, DT_SINGLELINE | DT_VCENTER);
	pDCMem->DrawText(m_strLink, rcText, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
	m_rcHover = rcText;

	// Copy to screen
	dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), pDCMem, 0, 0, SRCCOPY);

	// Restore GDI
	pDCMem->SetBkMode(nOldBk);
	pDCMem->SelectObject(hOldBrush);
	pDCMem->SelectObject(pOldFont);
	pDCMem->SelectObject(bmOld);
	pDCMem->SetTextColor(rcOldColor);
	pDCMem->SetBkColor(crOldBack);

	// Delete objects
	if (pDCMem->GetSafeHdc() != NULL)
	{
		pDCMem->DeleteDC();
	}
	delete pDCMem;
	if (bpMem.GetSafeHandle() != NULL)
	{
		bpMem.DeleteObject();
	}
}

void COptionTreeItemHyperLink::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Read only
	if (IsReadOnly() == TRUE)
	{
		CWnd::OnLButtonUp(nFlags, point);
		return;
	}

	// Kill timer
	KillTimer(OT_TIMER);

	// Mark visited
	if (m_rcHover.PtInRect(point) == TRUE)
	{
		m_bVisited = TRUE;

		// -- Go to link
		ShellExecute(NULL, _T("open"), m_strLink, NULL, NULL, SW_SHOW);

		// -- Set timer
		SetTimer(OT_TIMER, 100, NULL);
	}

	// Force redraw
	Invalidate();	

	// Update window
	UpdateWindow();

	CWnd::OnLButtonUp(nFlags, point);
}

void COptionTreeItemHyperLink::OnMouseMove(UINT nFlags, CPoint point) 
{	
	// Read only
	if (IsReadOnly() == TRUE)
	{
		CWnd::OnMouseMove(nFlags, point);
		return;
	}

	// Clear hover
	m_bHover = FALSE;

	// Kill timer
	KillTimer(OT_TIMER);

	// Mark hover
	if (m_rcHover.PtInRect(point) == TRUE)
	{
		m_bHover = TRUE;

		SetCursor(m_hLinkCursor);

		SetTimer(OT_TIMER, 100, NULL);
	}

	// Force redraw
	Invalidate();	

	// Update window
	UpdateWindow();

	CWnd::OnMouseMove(nFlags, point);
}

void COptionTreeItemHyperLink::OnTimer(UINT nIDEvent) 
{
	// Declare variables
	CPoint ptPoint;
	CRect rcHover = m_rcHover;

	// Get cursor and rectangle
	GetCursorPos(&ptPoint);
	ClientToScreen(rcHover);

	// Timer event
	if (nIDEvent == OT_TIMER)
	{
		// -- Clear hover
		if (rcHover.PtInRect(ptPoint) == FALSE)
		{
			KillTimer(OT_TIMER);
			
			m_bHover = FALSE;	
			
			Invalidate();

			// -- Update window
			UpdateWindow();
		}
	}

	
	CWnd::OnTimer(nIDEvent);
}

void COptionTreeItemHyperLink::SetVisited(BOOL bVisited)
{
	m_bVisited = bVisited;
}

void COptionTreeItemHyperLink::OnKillFocus(CWnd* pNewWnd) 
{
	// Validate
	if (m_otOption == NULL)
	{
		CWnd::OnKillFocus(pNewWnd);
		return;
	}

	// See if new window is tree of list
	if (m_otOption->IsChild(pNewWnd) == TRUE)
	{
		// -- Mark focus
		m_bFocus = FALSE;

		// -- Commit changes
		CommitChanges();
	}

	CWnd::OnKillFocus(pNewWnd);
}

void COptionTreeItemHyperLink::OnSetFocus(CWnd* pOldWnd) 
{
	// Mark focus
	m_bFocus = TRUE;
	
	CWnd::OnSetFocus(pOldWnd);	
}
