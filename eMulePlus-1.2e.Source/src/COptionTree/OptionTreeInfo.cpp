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
#include "OptionTreeInfo.h"
#include "OptionTree.h"
#include "OptionTreeDef.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeInfo

COptionTreeInfo::COptionTreeInfo()
{
	// Initialize variables
	m_otOption = NULL;
}

COptionTreeInfo::~COptionTreeInfo()
{
}


BEGIN_MESSAGE_MAP(COptionTreeInfo, CStatic)
	//{{AFX_MSG_MAP(COptionTreeInfo)
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COptionTreeInfo message handlers

void COptionTreeInfo::OnPaint() 
{
	// Make sure option is valid
	if (m_otOption == NULL)
	{
		return;
	}

	// Declare variables
	CPaintDC dc(this);
	CRect rcClient, rcText, rcOrgClient;
	CDC* pDCMem = new CDC;
	CBitmap bpMem;
	CBitmap *bmOld;
	CBrush brBack, *brOldBrush;
	COptionTreeItem *otiItem;
	COLORREF crOld;
	int nOldBack;
	HGDIOBJ hOld;
	CString strLabel, strInfo;

	// Get client rectangle
	GetClientRect(rcClient);
	rcOrgClient = rcClient;

	// Create pens and brushes
	brBack.CreateSolidBrush(GetSysColor(COLOR_BTNFACE));

	// Create DC
	pDCMem->CreateCompatibleDC(&dc);

	// Create bitmap
	bpMem.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());

	// Select bitmap
	bmOld = pDCMem->SelectObject(&bpMem);

	// Select brush
	brOldBrush = pDCMem->SelectObject(&brBack);

	// Paint the rectangle
	pDCMem->PatBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), PATCOPY);

	// Draw the edge
	pDCMem->DrawEdge(&rcClient, BDR_SUNKENOUTER, BF_RECT);
	
	// Deflate client rectangle
	rcClient.DeflateRect(4, 4);

	// Get the focused item
	otiItem = m_otOption->GetFocusedItem();

	// Set the text color
	if (m_otOption->IsWindowEnabled() == FALSE)
	{
		crOld = pDCMem->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
	}
	else
	{
		crOld = pDCMem->SetTextColor(GetSysColor(COLOR_BTNTEXT));
	}

	// Set the background mode
	nOldBack = pDCMem->SetBkMode(TRANSPARENT);

	// See if we have a focused item and get text
	// -- Default text
	if (otiItem == NULL)
	{
		if (m_otOption->GetDefInfoTextNoSel() == TRUE)
		{
			strLabel = OT_DEFLABEL;
			strInfo = OT_DEFINFO;
		}
	}
	// -- Items text
	else
	{
		strLabel = otiItem->GetLabelText();
		strInfo = otiItem->GetInfoText();
	}

	// Select the bold font
	hOld = pDCMem->SelectObject(m_otOption->GetBoldFont());

	// Calculate label rectangle
	rcText = rcClient;

	// Draw label
	pDCMem->DrawText(strLabel, &rcText, DT_SINGLELINE | DT_CALCRECT);
	pDCMem->DrawText(strLabel, &rcText, DT_SINGLELINE);
	
	// Select normal font
	pDCMem->SelectObject(m_otOption->GetNormalFont());

	// Calculate label rectangle
	rcText.top = rcText.bottom;
	rcText.bottom = rcClient.bottom;
	rcText.right = rcClient.right;

	// Draw info
	pDCMem->DrawText(strInfo, &rcText, DT_WORDBREAK);

	// Copy to screen
	dc.BitBlt(0, 0, rcOrgClient.Width(), rcOrgClient.Height(), pDCMem, 0, 0, SRCCOPY);

	// Restore the old GDI objects
	pDCMem->SelectObject(hOld);
	pDCMem->SelectObject(bmOld);
	pDCMem->SelectObject(brOldBrush);
	pDCMem->SetTextColor(crOld);
	pDCMem->SetBkMode(nOldBack);

	// Delete objects
	if (brBack.GetSafeHandle() != NULL)
	{
		brBack.DeleteObject();
	}
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

void COptionTreeInfo::SetOptionsOwner(COptionTree *otOption)
{
	// Save pointer
	m_otOption = otOption;
}


BOOL COptionTreeInfo::OnEraseBkgnd(CDC* pDC) 
{
	NOPRM(pDC);
	return FALSE;
}
