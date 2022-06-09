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
#include "OptionTreeItemColor.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemColor

COptionTreeItemColor::COptionTreeItemColor()
{
	// Initialize variables
	m_bFocus = FALSE;
	m_crColor = RGB(0, 0, 0);
	m_crAutomatic = RGB(0, 0, 0);
	m_dwOptions = NULL;

	// Set item type
	SetItemType(OT_ITEM_COLOR);
}

COptionTreeItemColor::~COptionTreeItemColor()
{
}


BEGIN_MESSAGE_MAP(COptionTreeItemColor, CWnd)
	//{{AFX_MSG_MAP(COptionTreeItemColor)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_LBUTTONUP()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	//}}AFX_MSG_MAP
    ON_MESSAGE(OT_COLOR_SELENDOK, OnSelEndOK)
    ON_MESSAGE(OT_COLOR_SELENDCANCEL, OnSelEndCancel)
    ON_MESSAGE(OT_COLOR_SELCHANGE, OnSelChange)
	ON_MESSAGE(OT_COLOR_CLOSEUP, OnCloseColorPopUp)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemColor message handlers

void COptionTreeItemColor::OnKillFocus(CWnd* pNewWnd) 
{
	// Mark focus
	m_bFocus = FALSE;

	// Commit changes
	CommitChanges();

	CWnd::OnKillFocus(pNewWnd);	
}

void COptionTreeItemColor::OnSetFocus(CWnd* pOldWnd) 
{
	// Mark focus
	m_bFocus = TRUE;		
	
	CWnd::OnSetFocus(pOldWnd);	
}

void COptionTreeItemColor::DrawAttribute(CDC *pDC, const RECT &rcRect)
{
	// If we don't have focus, text is drawn.
	if (m_bFocus == TRUE)
	{
		return;
	}

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

	// Draw control
	DrawControl(pDC, rcRect);
}

void COptionTreeItemColor::OnCommit()
{
	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemColor::OnRefresh()
{
	// Set the window positiion
	if (IsWindow(GetSafeHwnd()))
	{
		MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());
	}
}

void COptionTreeItemColor::OnMove()
{
	// Set window position
	if (IsWindow(GetSafeHwnd()))
	{
		MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());
	}

	// Hide window
	if (m_bFocus == FALSE && IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemColor::OnActivate()
{
	// Make sure window is valid
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_SHOW);

		// -- Set window position
		MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());

		// -- Set focus
		SetFocus();
	}
}

void COptionTreeItemColor::CleanDestroyWindow()
{
	// Destroy window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Destroy window
		DestroyWindow();
	}
}

void COptionTreeItemColor::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Declare variables
	CPoint ptPoint;

	// Get cursor position
	GetCursorPos(&ptPoint);

	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}

	// Show color popup
	new COptionTreeColorPopUp(ptPoint, m_crColor, m_crAutomatic, this, OT_COLOR_AUTOMATIC, OT_COLOR_MORECOLORS);

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

COLORREF COptionTreeItemColor::GetColor()
{
	// Return variable
	return m_crColor;
}

void COptionTreeItemColor::SetColor(COLORREF crColor)
{
	// Save variable
	m_crColor = crColor;
}

BOOL COptionTreeItemColor::CreateColorItem(DWORD dwOptions, COLORREF rcColor, COLORREF rcAutomatic)
{
	// Declare variables
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	BOOL bRet = FALSE;

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

			// -- -- Set color
			SetColor(rcColor);

			// -- -- Set automatic color
			SetAutomaticColor(rcAutomatic);

			// -- -- Save options
			m_dwOptions = dwOptions;

			// -- -- Set window position
			MoveWindow(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height());

			// -- -- Hide window
			ShowWindow(SW_HIDE);
		}
	}

	return bRet;
}

long COptionTreeItemColor::OnSelEndOK(UINT lParam, long wParam)
{
	// Get color	
	m_crColor = (COLORREF)lParam;

	// Commit changes
	CommitChanges();

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}

    return TRUE;
}

long COptionTreeItemColor::OnSelEndCancel(UINT lParam, long wParam)
{
	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}

    return TRUE;
}

long COptionTreeItemColor::OnCloseColorPopUp(UINT lParam, long wParam)
{
	// Get color	
	m_crColor = (COLORREF)lParam;

	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}

    return TRUE;
}

long COptionTreeItemColor::OnSelChange(UINT lParam, long wParam)
{
	// Get color
	if (GetOption(OT_COLOR_LIVEUPDATE) == TRUE)
	{
		m_crColor = (COLORREF)lParam;
	}

	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}

    return TRUE;
}

void COptionTreeItemColor::OnDeSelect()
{
	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemColor::OnSelect()
{
}

BOOL COptionTreeItemColor::OnEraseBkgnd(CDC* pDC) 
{
	// Naa, we like flicker free better
	return FALSE;
}

void COptionTreeItemColor::OnPaint() 
{
	// Check focus
	if (m_bFocus == FALSE)
	{
		return;
	}

	// Declare variables
	CPaintDC dc(this);
	CRect rcClient;

	// Get client rect
	GetClientRect(rcClient);

	// Draw control
	DrawControl(&dc, rcClient);
}

void COptionTreeItemColor::SetAutomaticColor(COLORREF crAutomatic)
{
	// Save variable
	m_crAutomatic = crAutomatic;
}

COLORREF COptionTreeItemColor::GetAutomaticColor()
{
	// Return variable
	return m_crAutomatic;
}

void COptionTreeItemColor::DrawControl(CDC *pDC, const RECT &rcRect)
{
	// Declare variables
	HGDIOBJ hOld;
	HGDIOBJ hOldBrush;
	COLORREF crOld;
	int nOldBack;
	CRect rcText, rcColor, rcClient;
	CString strText;
	CBrush bBrush;
	COLORREF crOldBack;

	// Get window rect
	GetClientRect(rcClient);

	// Select font
	hOld = pDC->SelectObject(m_otOption->GetNormalFont());

	// Create bush
	bBrush.CreateSolidBrush(m_crColor);
	
	// Set text color
	if (IsReadOnly() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
	{
		crOld = pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	else
	{
		crOld = pDC->SetTextColor(GetTextColor());
	}

	// Set background color
	crOldBack = pDC->SetBkColor(GetBackgroundColor());	

	// Set background mode
	nOldBack = pDC->SetBkMode(TRANSPARENT);

	// Select brush
	hOldBrush = pDC->SelectObject(GetSysColorBrush(COLOR_BTNSHADOW));

	// Get color rectangle
	rcColor.left  = rcRect.left + 1;
	rcColor.right = rcColor.left + (long) OT_COLOR_SIZE;
	rcColor.top = rcRect.top + OT_SPACE - 2;
	rcColor.bottom = rcColor.top + (long) OT_COLOR_SIZE;

	// Draw color border
	rcColor.InflateRect(1, 1, 1, 1);
	pDC->PatBlt(rcColor.left, rcColor.top, rcColor.Width(), rcColor.Height(), PATCOPY);

	// Draw color
	rcColor.DeflateRect(1, 1, 1, 1);
	pDC->FillRect(rcColor, &bBrush);

	// Get text rectangle
	rcText.left  = rcColor.right + OT_SPACE;
	rcText.right = rcRect.right;
	rcText.top = rcRect.top;
	rcText.bottom = rcRect.bottom;

	// Get text
	if (GetOption(OT_COLOR_SHOWHEX) == TRUE)
	{
		strText.Format("#%.6X", m_crColor);
	}
	else
	{
		strText.Format("RGB (%d, %d, %d)", GetRValue(m_crColor), GetGValue(m_crColor), GetBValue(m_crColor));
	}

	// Draw text
	pDC->DrawText(strText, rcText, DT_SINGLELINE | DT_VCENTER);
	pDC->DrawText(strText, rcText, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);
	
	// Delete brush
	if (bBrush.GetSafeHandle() != NULL)
	{
		bBrush.DeleteObject();
	}

	// Restore GDI ojects
	pDC->SelectObject(hOldBrush);
	pDC->SelectObject(hOld);
	pDC->SetTextColor(crOld);
	pDC->SetBkMode(nOldBack);
	pDC->SetBkColor(crOldBack);
}

BOOL COptionTreeItemColor::GetOption(DWORD dwOption)
{
	// Return option
	return (m_dwOptions & dwOption) ? TRUE : FALSE;
}

void COptionTreeItemColor::SetOption(DWORD dwOption, BOOL bSet)
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
