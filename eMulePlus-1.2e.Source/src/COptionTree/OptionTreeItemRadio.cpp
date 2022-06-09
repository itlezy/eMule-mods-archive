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
#include "OptionTreeItemRadio.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemRadio

COptionTreeItemRadio::COptionTreeItemRadio()
{
	// Initialize variables
	m_bFocus = FALSE;

	// Set item type
	SetItemType(OT_ITEM_RADIO);
}

COptionTreeItemRadio::~COptionTreeItemRadio()
{
}


BEGIN_MESSAGE_MAP(COptionTreeItemRadio, COptionTreeRadioButton)
	//{{AFX_MSG_MAP(COptionTreeItemRadio)
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemRadio message handlers

void COptionTreeItemRadio::DrawAttribute(CDC *pDC, const RECT &rcRect)
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
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_HIDEWINDOW);
	}

	// Declare variables
	int nOldBack;
	CRect rcText, rcRadio;
	HGDIOBJ hOld;
	OT_RADIO_NODE *nNode = NULL;
	int nIndex = 0;
	long lLastRadio = 0;
	COLORREF crOld;
	COLORREF crOldBack;

	// Select font
	hOld = pDC->SelectObject(m_otOption->GetNormalFont());
	
	// Set text color
	if (IsReadOnly() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
	{
		crOld = pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	else
	{
		crOld = pDC->SetTextColor(GetTextColor());
	}

	// Set background mode
	nOldBack = pDC->SetBkMode(TRANSPARENT);

	// Set background color
	crOldBack = pDC->SetBkColor(GetBackgroundColor());	

	// Calculate radio rect
	lLastRadio = rcRect.top; 
	rcRadio.left = rcRect.left;
	rcRadio.right = rcRect.left + (long) OT_RADIO_SIZE;

	// Go through and draw all nodes
	nNode = Node_FindNode(nIndex);
	while (nNode != NULL)
	{
		// -- Calculate radio rect
		rcRadio.top = lLastRadio + OT_RADIO_VSPACE;
		rcRadio.bottom = rcRadio.top + (long) OT_RADIO_SIZE;

		// -- Calculate text rect
		rcText.top = lLastRadio + OT_RADIO_VSPACE;
		rcText.bottom = rcRadio.top + (long) OT_RADIO_SIZE;
		rcText.left = rcRadio.right + OT_SPACE;
		rcText.right = rcRect.right;

		// -- Save last radio
		lLastRadio = rcRadio.bottom;

		// -- Draw the radio
		if (nNode->m_bChecked == TRUE)
		{
			pDC->DrawFrameControl(&rcRadio, DFC_BUTTON, DFCS_FLAT | DFCS_BUTTONRADIO | DFCS_CHECKED);
		}
		else
		{
			pDC->DrawFrameControl(&rcRadio, DFC_BUTTON, DFCS_FLAT | DFCS_BUTTONRADIO);
		}

		// -- Draw text
		pDC->DrawText(nNode->m_strText, rcText, DT_SINGLELINE | DT_VCENTER);
		pDC->DrawText(nNode->m_strText, rcText, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);

		// -- Increase index
		nIndex++;

		// -- Get next node
		nNode = Node_FindNode(nIndex);
	}

	// Restore GDI ojects
	pDC->SetBkMode(nOldBack);
	pDC->SelectObject(hOld);
	pDC->SetTextColor(crOld);
	pDC->SetBkColor(crOldBack);

}

void COptionTreeItemRadio::OnCommit()
{
	// Hide edit control
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemRadio::OnRefresh()
{
	// Set the window positiion
	if (IsWindow(GetSafeHwnd()))
	{
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void COptionTreeItemRadio::OnMove()
{
	// Set window position
	if (IsWindow(GetSafeHwnd()))
	{
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}

	// Hide window
	if (m_bFocus == FALSE && IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}

	// Recalculate height
	ReCalculateHeight();
}

void COptionTreeItemRadio::OnActivate()
{
	// Make sure window is valid
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_SHOW);

		// -- Set window position
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

		// -- Set focus
		SetFocus();
	}
}

void COptionTreeItemRadio::OnSetFocus(CWnd* pOldWnd) 
{
	// Mark focus
	m_bFocus = TRUE;		
	
	COptionTreeRadioButton::OnSetFocus(pOldWnd);	
}

void COptionTreeItemRadio::OnKillFocus(CWnd* pNewWnd) 
{
	// Validate
	if (m_otOption == NULL)
	{
		COptionTreeRadioButton::OnKillFocus(pNewWnd);
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
	
	COptionTreeRadioButton::OnKillFocus(pNewWnd);	
}

void COptionTreeItemRadio::CleanDestroyWindow()
{
	// Destroy window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Destroy window
		DestroyWindow();
	}
}

BOOL COptionTreeItemRadio::CreateRadioItem()
{
	// Declare variables
	DWORD dwStyle = WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	BOOL bRet = FALSE;

	// Make sure options is not NULL
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Create edit control
	if (!IsWindow(GetSafeHwnd()))
	{
		// -- Create the edit view
		bRet = Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW)), _T(""), dwStyle, m_rcAttribute, m_otOption->GetCtrlParent(), GetCtrlID());

		// -- Setup window
		if (bRet == TRUE)
		{

			// -- -- Set draw multiline
			SetDrawMultiline(TRUE);		

			// -- -- Set font
			SetFont(m_otOption->GetNormalFont(), TRUE);

			// -- -- Set check options owner
			SetRadioOptionsOwner(m_otOption);

			// -- -- Set window position
			SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

			// -- -- Hide window
			ShowWindow(SW_HIDE);
		}
	}

	return bRet;
}

void COptionTreeItemRadio::ReCalculateHeight()
{
	// Declare variables
	OT_RADIO_NODE *nNode = NULL;
	int nIndex = 0;
	long lHeight = 0;

	// Go through all nodes
	nNode = Node_FindNode(nIndex);
	while (nNode != NULL)
	{
		// -- Add to height
		lHeight += (OT_RADIO_VSPACE + (long) OT_RADIO_SIZE);
		
		// -- Increase index
		nIndex++;

		// -- Get next node
		nNode = Node_FindNode(nIndex);
	}

	// Add last space to height
	lHeight += OT_RADIO_VSPACE;

	// Set item height
	SetItemHeight(lHeight);

	// Force redraw
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Force redraw
		Invalidate();	

		// -- Update window
		UpdateWindow();
	}
}

void COptionTreeItemRadio::InsertNewRadio(CString strText, BOOL bChecked)
{
	// Uncheck all
	if (bChecked == TRUE)
	{
		Node_UnCheckAll();
	}

	// Insert new node
	Node_Insert(strText, bChecked);

	// Recalculate height
	ReCalculateHeight();
}

BOOL COptionTreeItemRadio::GetMultiline()
{
	// Return true always
	return TRUE;
}

void COptionTreeItemRadio::OnDeSelect()
{
	// Commit changes
	CommitChanges();
}

void COptionTreeItemRadio::OnSelect()
{
	// Commit changes
	CommitChanges();
}

int COptionTreeItemRadio::GetCheckedRadio()
{
	return Node_GetChecked();
}
