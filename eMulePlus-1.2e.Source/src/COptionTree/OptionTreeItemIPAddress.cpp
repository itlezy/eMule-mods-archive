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
#include "OptionTreeItemIPAddress.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemIPAddress

COptionTreeItemIPAddress::COptionTreeItemIPAddress()
{
	// Initialize variables
	m_bFocus = FALSE;

	// Set item type
	SetItemType(OT_ITEM_IPADDRESS);
}

COptionTreeItemIPAddress::~COptionTreeItemIPAddress()
{
}


BEGIN_MESSAGE_MAP(COptionTreeItemIPAddress, CIPAddressCtrl)
	//{{AFX_MSG_MAP(COptionTreeItemIPAddress)
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_MESSAGE(OT_IPADDRESS_KILLFOCUS, OnEditLooseFocus)
	ON_MESSAGE(OT_IPADDRESS_NEXTEDIT, OnEditNextEdit)
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemIPAddress message handlers

void COptionTreeItemIPAddress::OnActivate()
{
	// Declare variables
	int nSpace, nWidth, nLeft;

	// Make sure window is valid
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_SHOW);

		// -- Set window position
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

		// -- Calculate
		nSpace = 5;
		nWidth = ((m_rcAttribute.Width() - (nSpace * 5)) / 4);	
		
		// -- Move edits
		// -- -- 1
		nLeft = nSpace;
		m_ctlEdit1.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 2
		nLeft += nWidth + nSpace;
		m_ctlEdit2.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 3
		nLeft += nWidth + nSpace;
		m_ctlEdit3.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 4
		nLeft += nWidth + nSpace;
		m_ctlEdit4.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());	

		// -- Set focus
		SetFocus();
	}
}

void COptionTreeItemIPAddress::DrawAttribute(CDC *pDC, const RECT &rcRect)
{
	// Declare variables
	int nSpace, nWidth, nLeft;

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
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);

		// -- Calculate
		nSpace = 5;
		nWidth = ((m_rcAttribute.Width() - (nSpace * 5)) / 4);	
		
		// -- Move edits
		// -- -- 1
		nLeft = nSpace;
		m_ctlEdit1.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 2
		nLeft += nWidth + nSpace;
		m_ctlEdit2.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 3
		nLeft += nWidth + nSpace;
		m_ctlEdit3.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 4
		nLeft += nWidth + nSpace;
		m_ctlEdit4.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());	
	}

	// Declare variables
	HGDIOBJ hOld;
	COLORREF crOld;
	int nOldBack;
	CRect rcText;
	CString strText;
	BYTE nField0, nField1, nField2, nField3;
	COLORREF crOldBack;

	// Make text
	GetAddress(nField0, nField1, nField2, nField3);
	strText.Format(_T("%d.%d.%d.%d"), nField0, nField1, nField2, nField3);

	// Select font
	hOld = pDC->SelectObject(m_otOption->GetNormalFont());
	
	// Set text color
	if (IsReadOnly() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
	{
		crOld = pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	else
	{
		crOld = pDC->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));
	}

	// Set background mode
	nOldBack = pDC->SetBkMode(TRANSPARENT);

	// Set background color
	crOldBack = pDC->SetBkColor(GetBackgroundColor());	

	// Get rectangle
	rcText = rcRect;

	// Draw text
	pDC->DrawText(strText, rcText, DT_SINGLELINE | DT_VCENTER);

	// Restore GDI ojects
	pDC->SelectObject(hOld);
	pDC->SetTextColor(crOld);
	pDC->SetBkMode(nOldBack);
	pDC->SetBkColor(crOldBack);
}

void COptionTreeItemIPAddress::OnCommit()
{
	// Hide edit control
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}

}

void COptionTreeItemIPAddress::OnRefresh()
{
	// Declare variables
	int nSpace, nWidth, nLeft;

	// Set the window text
	if (IsWindow(GetSafeHwnd()))
	{
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		// -- Calculate
		nSpace = 5;
		nWidth = ((m_rcAttribute.Width() - (nSpace * 5)) / 4);	
		
		// -- Move edits
		// -- -- 1
		nLeft = nSpace;
		m_ctlEdit1.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 2
		nLeft += nWidth + nSpace;
		m_ctlEdit2.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 3
		nLeft += nWidth + nSpace;
		m_ctlEdit3.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 4
		nLeft += nWidth + nSpace;
		m_ctlEdit4.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());		
	}
}

void COptionTreeItemIPAddress::OnMove()
{
	// Declare variables
	int nSpace, nWidth, nLeft;

	// Set window position
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Move window
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_NOACTIVATE);

		// -- Calculate
		nSpace = 5;
		nWidth = ((m_rcAttribute.Width() - (nSpace * 5)) / 4);

		// -- Move edits
		// -- -- 1
		nLeft = nSpace;
		m_ctlEdit1.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 2
		nLeft += nWidth + nSpace;
		m_ctlEdit2.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 3
		nLeft += nWidth + nSpace;
		m_ctlEdit3.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());
		// -- -- 4
		nLeft += nWidth + nSpace;
		m_ctlEdit4.MoveWindow(nLeft, 0, nWidth, m_rcAttribute.Height());

	}

	// Hide window
	if (m_bFocus == FALSE && IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemIPAddress::CleanDestroyWindow()
{
	// Destroy window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Destroy window
		DestroyWindow();
	}
}

void COptionTreeItemIPAddress::OnDeSelect()
{
	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemIPAddress::OnSelect()
{
	// Do nothing here
}

void COptionTreeItemIPAddress::OnSetFocus(CWnd* pOldWnd) 
{
	// Mark focus
	m_bFocus = TRUE;
	
	CIPAddressCtrl::OnSetFocus(pOldWnd);	
}

void COptionTreeItemIPAddress::OnKillFocus(CWnd* pNewWnd) 
{
	// Validate
	if (m_otOption == NULL)
	{
		CIPAddressCtrl::OnKillFocus(pNewWnd);
		return;
	}

	// See if new window is an edit
	if (m_ctlEdit1.GetSafeHwnd() == pNewWnd->GetSafeHwnd() || m_ctlEdit2.GetSafeHwnd() == pNewWnd->GetSafeHwnd() || m_ctlEdit3.GetSafeHwnd() == pNewWnd->GetSafeHwnd() || m_ctlEdit4.GetSafeHwnd() == pNewWnd->GetSafeHwnd())
	{
		CIPAddressCtrl::OnKillFocus(pNewWnd);
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
	
	CIPAddressCtrl::OnKillFocus(pNewWnd);	
}

BOOL COptionTreeItemIPAddress::CreateIPAddressItem(DWORD dwAddStyle)
{
	// Declare variables
	DWORD dwStyle = WS_CHILD | WS_VISIBLE;
	BOOL bRet = FALSE;


	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Add style
	if (dwAddStyle != 0)
	{
		dwStyle |= dwAddStyle;
	}

	// Create edit control
	if (!IsWindow(GetSafeHwnd()))
	{
		// -- Create the edit view
		bRet = Create(dwStyle, m_rcAttribute, m_otOption->GetCtrlParent(), GetCtrlID());

		// -- Setup IP address
		if (bRet == TRUE)
		{
			// -- -- Set font
			SetFont(m_otOption->GetNormalFont(), TRUE);

			// -- -- Set window position
			SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

			// -- -- Hide window
			ShowWindow(SW_HIDE);

			// -- -- Get edit fields
			m_ctlEdit4.SubclassWindow(GetNextWindow(GW_CHILD)->GetSafeHwnd());
			m_ctlEdit3.SubclassWindow(m_ctlEdit4.GetNextWindow(GW_HWNDNEXT)->GetSafeHwnd());
			m_ctlEdit2.SubclassWindow(m_ctlEdit3.GetNextWindow(GW_HWNDNEXT)->GetSafeHwnd());
			m_ctlEdit1.SubclassWindow(m_ctlEdit2.GetNextWindow(GW_HWNDNEXT)->GetSafeHwnd());
			m_ctlEdit4.SetNotifyWnd(GetSafeHwnd());
			m_ctlEdit3.SetNotifyWnd(GetSafeHwnd());
			m_ctlEdit2.SetNotifyWnd(GetSafeHwnd());
			m_ctlEdit1.SetNotifyWnd(GetSafeHwnd());
		}
	}

	return bRet;
}

BOOL COptionTreeItemIPAddress::OnEraseBkgnd(CDC* pDC) 
{	
	return FALSE;
}

void COptionTreeItemIPAddress::OnPaint() 
{
	// Declare variables
	int nSpace, nWidth, nLeft;
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

	// Get client rectangle
	GetClientRect(rcClient);

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
	pOldFont = pDCMem->SelectObject(m_otOption->GetBoldFont());

	// Set text color
	rcOldColor = pDCMem->SetTextColor(GetTextColor());

    // Setbackground
    nOldBk = pDCMem->SetBkMode(TRANSPARENT);

	// Calculate dots
	nSpace = 5;
	nWidth = ((m_rcAttribute.Width() - (nSpace * 5)) / 4);

	// Draw dots
	// -- 1
	nLeft = nSpace + nWidth;
	rcText.SetRect(nLeft, 0, 100, rcClient.Height());
	pDCMem->DrawText(_T("."), rcText, DT_SINGLELINE | DT_CENTER | DT_CALCRECT);
	pDCMem->DrawText(_T("."), rcText, DT_SINGLELINE | DT_CENTER);
	// -- 2
	nLeft += nSpace + nWidth;
	rcText.SetRect(nLeft, 0, 100, rcClient.Height());
	pDCMem->DrawText(_T("."), rcText, DT_SINGLELINE | DT_CENTER | DT_CALCRECT);
	pDCMem->DrawText(_T("."), rcText, DT_SINGLELINE | DT_CENTER);
	// -- 3
	nLeft += nSpace + nWidth;
	rcText.SetRect(nLeft, 0, 100, rcClient.Height());
	pDCMem->DrawText(_T("."), rcText, DT_SINGLELINE | DT_CENTER | DT_CALCRECT);
	pDCMem->DrawText(_T("."), rcText, DT_SINGLELINE | DT_CENTER);

	// Copy to screen
	dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), pDCMem, 0, 0, SRCCOPY);

	// Restore GDI
	pDCMem->SetTextColor(nOldBk);
	pDCMem->SelectObject(hOldBrush);
	pDCMem->SelectObject(pOldFont);
	pDCMem->SelectObject(bmOld);
	pDCMem->SetTextColor(rcOldColor);

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

long COptionTreeItemIPAddress::OnEditNextEdit(UINT lParam, long wParam)
{
	// Declare variables
	CEdit *pEdit = (CEdit*) lParam;

	// Validate
	if (m_otOption == NULL || pEdit == NULL)
	{
		return FALSE;
	}

	// See what edit it is
	// -- 1
	if (pEdit->GetSafeHwnd() == m_ctlEdit1.GetSafeHwnd())
	{
		m_ctlEdit2.SetFocus();
	}
	// -- 2
	else if (pEdit->GetSafeHwnd() == m_ctlEdit2.GetSafeHwnd())
	{
		m_ctlEdit3.SetFocus();
	}
	// -- 3
	else if (pEdit->GetSafeHwnd() == m_ctlEdit3.GetSafeHwnd())
	{
		m_ctlEdit4.SetFocus();
	}

	return TRUE;
}

long COptionTreeItemIPAddress::OnEditLooseFocus(UINT lParam, long wParam)
{
	// Declare variables
	CWnd *pNewWnd = (CWnd*) lParam;

	// Validate
	if (m_otOption == NULL || pNewWnd == NULL)
	{
		return FALSE;
	}

	// See if new window is an edit
	if (m_ctlEdit1.GetSafeHwnd() == pNewWnd->GetSafeHwnd() || m_ctlEdit2.GetSafeHwnd() == pNewWnd->GetSafeHwnd() || m_ctlEdit3.GetSafeHwnd() == pNewWnd->GetSafeHwnd() || m_ctlEdit4.GetSafeHwnd() == pNewWnd->GetSafeHwnd())
	{
		return FALSE;
	}

	// See if new window is tree of list
	if (m_otOption->IsChild(pNewWnd) == TRUE)
	{
		// -- Mark focus
		m_bFocus = FALSE;

		// -- Commit changes
		CommitChanges();
	}

	return TRUE;
}

void COptionTreeItemIPAddress::OnDestroy() 
{
	// Un subsclass
	m_ctlEdit4.UnsubclassWindow();
	m_ctlEdit3.UnsubclassWindow();
	m_ctlEdit2.UnsubclassWindow();
	m_ctlEdit1.UnsubclassWindow();	
	
	CIPAddressCtrl::OnDestroy();	
}
