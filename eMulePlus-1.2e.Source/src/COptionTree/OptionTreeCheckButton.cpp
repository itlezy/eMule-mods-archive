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
#include "OptionTreeCheckButton.h"

// Added Headers
#include "OptionTree.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeCheckButton

COptionTreeCheckButton::COptionTreeCheckButton()
{
	// Initialize variables
	m_strUnChecked = OT_CHECKBOX_DEFUNCHECKTEXT;
	m_strChecked = OT_CHECKBOX_DEFCHECKTEXT;
	m_bShowText = FALSE;
	m_bCheck = FALSE;
	m_bShowCheck = FALSE;
	m_rcCheck = CRect(0, 0, 0, 0);
	m_otCheckOption = NULL;
	m_dwOptions = NULL;
}

COptionTreeCheckButton::~COptionTreeCheckButton()
{
}


BEGIN_MESSAGE_MAP(COptionTreeCheckButton, CWnd)
	//{{AFX_MSG_MAP(COptionTreeCheckButton)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTreeCheckButton message handlers

void COptionTreeCheckButton::SetCheckText(CString strChecked, CString strUnChecked)
{
	// Save variables
	m_strChecked = strChecked;
	m_strUnChecked = strUnChecked;
}

CString COptionTreeCheckButton::GetCheckedText()
{
	// Return variables
	return m_strChecked;
}

CString COptionTreeCheckButton::GetUnCheckedText()
{
	// Return variables
	return m_strUnChecked;
}

void COptionTreeCheckButton::OnPaint() 
{
	// Make sure options aren't NULL
	if (m_otCheckOption == NULL)
	{
		return;
	}

	// Declare variables
	CPaintDC dc(this);
	CDC* pDCMem = new CDC;
	CBitmap bpMem;
	CBitmap *bmOld;
	COLORREF crOld;
	HGDIOBJ hOldBrush;
	int nOldBack;
	CRect rcText, rcCheck, rcClient;
	CString strText;
	HGDIOBJ hOld;

	// Get client rectangle
	GetClientRect(rcClient);

	// Create DC
	pDCMem->CreateCompatibleDC(&dc);
	
	// Create bitmap
	bpMem.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());

	// Select bitmap
	bmOld = pDCMem->SelectObject(&bpMem);

	// Set background mode
	nOldBack = pDCMem->SetBkMode(TRANSPARENT);

	// Set text color
	crOld = pDCMem->SetTextColor(GetSysColor(COLOR_WINDOWTEXT));

	// Select font
	hOld = pDCMem->SelectObject(m_otCheckOption->GetNormalFont());

	// Draw control background
	if (m_otCheckOption->IsWindowEnabled() == FALSE)
	{
		hOldBrush = pDCMem->SelectObject(GetSysColorBrush(COLOR_BTNFACE));
	}
	else
	{
		hOldBrush = pDCMem->SelectObject(GetSysColorBrush(COLOR_WINDOW));
	}
	pDCMem->PatBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), PATCOPY);

	// Get rectangle
	rcCheck.left  = rcClient.left + 1;
	rcCheck.right = rcCheck.left + (long) OT_CHECKBOX_SIZE;
	rcCheck.top = rcClient.top + OT_SPACE - 1;
	rcCheck.bottom = rcCheck.top + (long) OT_CHECKBOX_SIZE;
		
	// Draw check
	if (GetCheck() == TRUE)
	{
		pDCMem->DrawFrameControl(&rcCheck, DFC_BUTTON, DFCS_BUTTONCHECK | DFCS_CHECKED);
	}
	else
	{
		pDCMem->DrawFrameControl(&rcCheck, DFC_BUTTON, DFCS_BUTTONCHECK);
	}

	// Draw text
	if (GetOption(OT_CHECKBOX_SHOWTEXT) == TRUE)
	{
		// -- Get text
		if (GetCheck() == TRUE)
		{
			strText = GetCheckedText();
		}
		else
		{
			strText = GetUnCheckedText();
		}

		// -- Get rectangle
		rcText.left  = rcCheck.right + OT_SPACE;
		rcText.right = rcClient.right;
		rcText.top = rcClient.top;
		rcText.bottom = rcClient.bottom;

		// -- Draw text
		pDCMem->DrawText(strText, rcText, DT_SINGLELINE | DT_VCENTER);
	}

	// Save check rectangle
	m_rcCheck = rcCheck;
	if (GetOption(OT_CHECKBOX_SHOWTEXT) == TRUE)
	{
		m_rcCheck.right = rcText.right;
	}

	// Copy to screen
	dc.BitBlt(0, 0, rcClient.Width(), rcClient.Height(), pDCMem, 0, 0, SRCCOPY);

	// Restore GDI ojects
	pDCMem->SelectObject(bmOld);
	pDCMem->SelectObject(hOldBrush);
	pDCMem->SetBkMode(nOldBack);
	pDCMem->SelectObject(hOld);
	pDCMem->SetTextColor(crOld);	

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

BOOL COptionTreeCheckButton::OnEraseBkgnd(CDC* pDC) 
{
	// Naa, we like flicker free better
	return FALSE;
}

void COptionTreeCheckButton::SetCheck(BOOL bCheck)
{
	// Set variable
	m_bCheck = bCheck;
}


BOOL COptionTreeCheckButton::GetCheck()
{
	// Return check
	return m_bCheck;
}

void COptionTreeCheckButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// See if check was pressed
	if (m_rcCheck.PtInRect(point) == TRUE)
	{
		// -- Reverse check
		SetCheck(!GetCheck());

		// -- Force redraw
		Invalidate();

		// -- Update window
		UpdateWindow();
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void COptionTreeCheckButton::SetCheckOptionsOwner(COptionTree *otOption)
{
	// Save pointer
	m_otCheckOption = otOption;
}

BOOL COptionTreeCheckButton::GetOption(DWORD dwOption)
{
	// Return option
	return (m_dwOptions & dwOption) ? TRUE : FALSE;
}

void COptionTreeCheckButton::SetOption(DWORD dwOption, BOOL bSet)
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
