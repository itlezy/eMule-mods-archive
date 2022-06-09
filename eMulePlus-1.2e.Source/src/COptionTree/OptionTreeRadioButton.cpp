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
#include "OptionTreeRadioButton.h"


// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeRadioButton

COptionTreeRadioButton::COptionTreeRadioButton()
{
	// Initialize variables
	m_nAllNodes = NULL;
	m_otRadioOption = NULL;
}

COptionTreeRadioButton::~COptionTreeRadioButton()
{
	// Delete all nodes
	Node_DeleteAll();
}


BEGIN_MESSAGE_MAP(COptionTreeRadioButton, CWnd)
	//{{AFX_MSG_MAP(COptionTreeRadioButton)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_MOVE()
	ON_WM_SIZE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTreeRadioButton message handlers

void COptionTreeRadioButton::Node_Insert(CString strText, BOOL bChecked)
{
	// Declare variables
	OT_RADIO_NODE *NewNode = new OT_RADIO_NODE;

	// Set up the New Node structure
	NewNode->m_bChecked = bChecked;
	NewNode->m_strText = strText;
	NewNode->m_rcHitRect = CRect(0, 0, 0, 0);
	NewNode->m_nNextNode = NULL;

	// Add new node to list
	// -- Do have any other node?
	if (m_nAllNodes == NULL)
	{
		m_nAllNodes = NewNode;
		return;
	}

	// -- Find the end of the list to add the new node to
	OT_RADIO_NODE *curr = m_nAllNodes;
	OT_RADIO_NODE *last = NULL;
	while (curr != NULL)
	{
		// -- -- Save this node
		last = curr;

		// -- -- Follow the link to the next node
		curr = curr->m_nNextNode;
	}

	// -- Link the new nod to the place we found
	last->m_nNextNode = NewNode;
}

void COptionTreeRadioButton::Node_DeleteAll()
{
	// Do have any other results
	if (m_nAllNodes == NULL)
	{
		return;
	}

	
	// Declare variables
	OT_RADIO_NODE *curr = m_nAllNodes;
	OT_RADIO_NODE *last = NULL;
	
	// Check to see if we have only 1
	if (m_nAllNodes->m_nNextNode == NULL)
	{
		// -- Delete m_AllResults (head)
		curr = m_nAllNodes;
		delete curr;
		m_nAllNodes = NULL;

		return;
	}
	

	// Find the end of the list to add the new result to
	while (curr->m_nNextNode != NULL)
	{
		// -- Save this node
		last = curr;

		// -- Follow the link to the next node
		curr = curr->m_nNextNode;
	}

	// Delete this node and set last->m_nNextNode to NULL
	delete curr;
	last->m_nNextNode = NULL;

	// Check to see if we are at second to beginning
	if (m_nAllNodes->m_nNextNode == last)
	{
		// -- Delete last (which is second to head)
		delete last;

		// -- Delete m_AllResults (head)
		curr = m_nAllNodes;
		delete curr;
		m_nAllNodes = NULL;

	}

	// Call this again
	Node_DeleteAll();
}

OT_RADIO_NODE * COptionTreeRadioButton::Node_FindNode(int nIndex)
{
	// Declare variables
	int i = 0;

	// Cycle through all of the nodes
	OT_RADIO_NODE *curr = m_nAllNodes;
	while (curr != NULL)
	{		
		// -- See if this is us
		if (i == nIndex)
		{			
			return curr;
		}

		// -- Follow the link to the next node
		curr = curr->m_nNextNode;

		// -- Increase i
		i++;
	}

	return NULL;
}

OT_RADIO_NODE *COptionTreeRadioButton::Node_FindNode(CString strText)
{
	// Cycle through all of the nodes
	OT_RADIO_NODE *curr = m_nAllNodes;
	while (curr != NULL)
	{		
		// -- See if this is us
		if (curr->m_strText == strText)
		{			
			return curr;
		}

		// -- Follow the link to the next node
		curr = curr->m_nNextNode;
	}

	return NULL;
}

BOOL COptionTreeRadioButton::OnEraseBkgnd(CDC* pDC) 
{
	// Naa, we like flicker free better
	return FALSE;
}

void COptionTreeRadioButton::OnPaint() 
{
	// Make sure options aren't NULL
	if (m_otRadioOption == NULL)
	{
		return;
	}

	// Declare variables
	CPaintDC dc(this);
	CDC* pDCMem = new CDC;
	CBitmap bpMem;
	CBitmap *bmOld;
	HGDIOBJ hOldBrush;
	int nOldBack;
	CRect rcText, rcRadio, rcClient;
	HGDIOBJ hOld;
	OT_RADIO_NODE *nNode = NULL;
	int nIndex = 0;
	long lLastRadio = 0;
	COLORREF crOld;

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
	hOld = pDCMem->SelectObject(m_otRadioOption->GetNormalFont());

	// Draw control background
	if (m_otRadioOption->IsWindowEnabled() == FALSE)
	{
		hOldBrush = pDCMem->SelectObject(GetSysColorBrush(COLOR_BTNFACE));
	}
	else
	{
		hOldBrush = pDCMem->SelectObject(GetSysColorBrush(COLOR_WINDOW));
	}
	pDCMem->PatBlt(rcClient.left, rcClient.top, rcClient.Width(), rcClient.Height(), PATCOPY);

	// Calculate radio rect
	rcRadio.left = rcClient.left;
	rcRadio.right = rcClient.left + (long) OT_RADIO_SIZE;

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
		rcText.right = rcClient.right;

		// -- Save last radio
		lLastRadio = rcRadio.bottom;

		// -- Draw the radio
		if (nNode->m_bChecked == TRUE)
		{
			pDCMem->DrawFrameControl(&rcRadio, DFC_BUTTON, DFCS_BUTTONRADIO | DFCS_CHECKED);
		}
		else
		{
			pDCMem->DrawFrameControl(&rcRadio, DFC_BUTTON, DFCS_BUTTONRADIO);
		}

		// -- Draw text
		pDCMem->DrawText(nNode->m_strText, rcText, DT_SINGLELINE | DT_VCENTER);
		pDCMem->DrawText(nNode->m_strText, rcText, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);

		// -- Set hit test rect
		nNode->m_rcHitRect.left = rcRadio.left;
		nNode->m_rcHitRect.top = rcRadio.top;
		nNode->m_rcHitRect.bottom = rcRadio.bottom;
		nNode->m_rcHitRect.right = rcText.right;

		// -- Increase index
		nIndex++;

		// -- Get next node
		nNode = Node_FindNode(nIndex);
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

void COptionTreeRadioButton::SetRadioOptionsOwner(COptionTree *otOption)
{
	// Save pointer
	m_otRadioOption = otOption;
}

void COptionTreeRadioButton::Node_UnCheckAll()
{
	// Cycle through all of the nodes
	OT_RADIO_NODE *curr = m_nAllNodes;
	while (curr != NULL)
	{
		// -- Un Check
		curr->m_bChecked = FALSE;

		// -- Follow the link to the next node
		curr = curr->m_nNextNode;
	}
}

void COptionTreeRadioButton::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Run a hit test on all radios
	OT_RADIO_NODE *curr = m_nAllNodes;
	while (curr != NULL)
	{
		// -- See if checked
		if (curr->m_rcHitRect.PtInRect(point) == TRUE)
		{
			// -- -- Uncheck all
			Node_UnCheckAll();

			// -- -- Check this radio
			curr->m_bChecked = TRUE;

			// -- -- Force redaw
			Invalidate();

			// -- -- Update window
			UpdateWindow();

			break;
		}

		// -- Follow the link to the next node
		curr = curr->m_nNextNode;
	}
	
	CWnd::OnLButtonUp(nFlags, point);
}

void COptionTreeRadioButton::OnMove(int x, int y) 
{
	CWnd::OnMove(x, y);
	
	// TODO: Add your message handler code here
	
}

void COptionTreeRadioButton::OnSize(UINT nType, int cx, int cy) 
{
	CWnd::OnSize(nType, cx, cy);
	
	// TODO: Add your message handler code here
	
}

int COptionTreeRadioButton::Node_GetChecked()
{
	// Declare variables
	int i = 0;

	// Cycle through all of the nodes
	OT_RADIO_NODE *curr = m_nAllNodes;
	while (curr != NULL)
	{		
		// -- See if this is us
		if (curr->m_bChecked == TRUE)
		{			
			return i;
		}

		// -- Follow the link to the next node
		curr = curr->m_nNextNode;

		// -- Increase i
		i++;
	}

	return -1;
}
