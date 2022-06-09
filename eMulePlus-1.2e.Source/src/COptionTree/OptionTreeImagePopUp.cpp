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
#include "OptionTreeImagePopUp.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeImagePopUp

COptionTreeImagePopUp::COptionTreeImagePopUp(CPoint pPoint, CSize sImageSize, int nNumColumns, CImageList *imgImageList, COptionTree *otImageOption, CStringArray &strText, int nSelected, CWnd* pParentWnd)
{
	// Initialize variables
	m_pHover = pPoint;
	m_otImageOption = otImageOption;
	m_pImageList = imgImageList;
	m_pParent = pParentWnd;
	m_sImageSizes = sImageSize;
	m_nNumRows = 0;
	m_nSelected = nSelected;

	// Copy text
	for (int i = 0; i < strText.GetSize(); i++)
	{
		m_strText.Add(strText.GetAt(i));
	}

	// Validate number of columns
	if (nNumColumns >= 1)
	{
		m_nNumColumns = nNumColumns;
	}
	else
	{
		m_nNumColumns = 3;
	}

	// Create
	COptionTreeImagePopUp::Create(pPoint);
}

COptionTreeImagePopUp::~COptionTreeImagePopUp()
{
	// Reallocate
	m_strText.RemoveAll();
}


BEGIN_MESSAGE_MAP(COptionTreeImagePopUp, CWnd)
	//{{AFX_MSG_MAP(COptionTreeImagePopUp)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_NCDESTROY()
	ON_WM_ACTIVATEAPP()
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_KEYDOWN()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTreeImagePopUp message handlers

BOOL COptionTreeImagePopUp::Create(CPoint pPoint)
{
	// Declare variables
	CString strClassName;

    // Get the class name and create the window
    strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, 0, (HBRUSH) (COLOR_BTNFACE+1), 0);

	// Create window
    if (!CWnd::CreateEx(0, strClassName, _T(""), WS_POPUP | WS_VISIBLE, pPoint.x, pPoint.y, 100, 100, m_pParent->GetSafeHwnd(), 0, NULL))
	{
        return FALSE;
	}

	// Show window
	ShowWindow(SW_SHOWNA);

    // Set the window size
    SetWindowSize();
	
	// Setup images
	SetUpImages();

	// Setup tooltips
	SetUpToolTips();

    // Capture all mouse events for the life of this window
    SetCapture();

	return TRUE;
}

void COptionTreeImagePopUp::SetWindowSize()
{
	// Declare variables
	int nNumImages;
	CRect rcClient, rcMove;
	int nRow, nCol;

	// Get window rect
	GetWindowRect(rcClient);
	rcMove = rcClient;

	// Get the number of images
	nNumImages = m_pImageList->GetImageCount();

    // Get the number of rows
	// -- No images make square
    if (nNumImages == 0)
	{
		m_nNumRows = m_nNumColumns;
	}
	// -- Find number of rows
	else
	{
		m_nNumRows = nNumImages / m_nNumColumns;
		if (nNumImages % m_nNumColumns) 
		{
			m_nNumRows++;
		}
	}
	
	// Go through all rows
	rcMove.bottom = rcClient.top + OT_IMAGE_MARGIN;
	for (nRow = 1; nRow <= m_nNumRows; nRow++)
	{
		// -- Add size of image
		rcMove.bottom += m_sImageSizes.cy;

		// -- Add the image space
		if (nRow < m_nNumRows)
		{
			rcMove.bottom += OT_IMAGE_IMAGESPACE;
		}
	}
	rcMove.bottom += OT_IMAGE_MARGIN;

	// Go through all columns
	rcMove.right = rcClient.left +  OT_IMAGE_MARGIN;
	for (nCol = 1; nCol <= m_nNumColumns; nCol++)
	{
		// -- Add size of image
		rcMove.right += m_sImageSizes.cx;

		// -- Add the image space
		if (nCol < m_nNumColumns)
		{
			rcMove.right += OT_IMAGE_IMAGESPACE;
		}
	}
	rcMove.right += OT_IMAGE_MARGIN;

    // Set the window size and position
    MoveWindow(rcMove, TRUE);
}

void COptionTreeImagePopUp::SetUpToolTips()
{
	// Create toop tip
	if (m_ttToolTip.Create(this) == FALSE)
	{
		return;
	}

	// Go through all images
	for (int i = 0; i < m_pImageList->GetImageCount(); i++)
	{
		// -- Set tooltip
		m_ttToolTip.AddTool(this, (LPCTSTR)m_strText.GetAt(i), m_rcRectangles.GetAt(i), 1);
	}

	// Create inactive
	m_ttToolTip.Activate(TRUE);
}

void COptionTreeImagePopUp::OnDestroy() 
{
	// Destroy tooltips
	if (IsWindow(m_ttToolTip.GetSafeHwnd()))
	{
		m_ttToolTip.DestroyWindow();
	}

	// ALWAYS send message to parent
	if (IsWindow(m_pParent->GetSafeHwnd()))
	{
		m_pParent->SendMessage(OT_IMAGE_CLOSE, 0, m_nSelected);
	}
	
	CWnd::OnDestroy();	
}

void COptionTreeImagePopUp::SetUpImages()
{
	// Declare variables
	CRect rcClient, rcImage;
	int nRow, nCol;
	long lTop, lLeft;

	// Get window rect
	GetClientRect(rcClient);

	// Setup top
	lTop = rcClient.top +  OT_IMAGE_MARGIN;
	
	// Go through all rows
	for (nRow = 1; nRow <= m_nNumRows; nRow++)
	{
		// -- Reset left
		lLeft = rcClient.left + OT_IMAGE_MARGIN;

		// -- Go through all columns
		for (nCol = 1; nCol <= m_nNumColumns; nCol++)
		{
			// -- -- Set image rect
			rcImage.SetRect(lLeft, lTop, lLeft + m_sImageSizes.cx, lTop + m_sImageSizes.cy);

			// -- -- Add rect
			m_rcRectangles.Add(rcImage);

			// -- -- Add to left
			lLeft += m_sImageSizes.cx;
			if (nCol < m_nNumColumns)
			{
				lLeft += OT_IMAGE_IMAGESPACE;
			}

		}

		// -- Add to top of total image rectangle
		if (nRow < m_nNumRows)
		{
			lTop += m_sImageSizes.cy + OT_IMAGE_IMAGESPACE;
		}
	}
}

BOOL COptionTreeImagePopUp::OnEraseBkgnd(CDC* pDC) 
{
	return FALSE;
}

void COptionTreeImagePopUp::OnNcDestroy() 
{
	CWnd::OnNcDestroy();
	
	// Delete this
	delete this;
	
}

void COptionTreeImagePopUp::OnActivateApp(BOOL bActive, HTASK hTask) 
{
	CWnd::OnActivateApp(bActive, hTask);
	
	// Release capture
	ReleaseCapture();
	
}

BOOL COptionTreeImagePopUp::PreTranslateMessage(MSG* pMsg) 
{
	// Relay tooltip
	if (IsWindow(m_ttToolTip.GetSafeHwnd()))
	{
		m_ttToolTip.RelayEvent(pMsg);
	}

    // Sometimes if the picker loses focus it is never destroyed
    if (GetCapture()->GetSafeHwnd() != GetSafeHwnd())
    {   
		SetCapture(); 
	}
	
	return CWnd::PreTranslateMessage(pMsg);
}

void COptionTreeImagePopUp::OnPaint() 
{
	// Make sure options aren't NULL
	if (m_otImageOption == NULL)
	{
		return;
	}

	// Declare variables
	CPaintDC dc(this);
	CDC* pDCMem = new CDC;
	CBitmap bpMem;
	CBitmap *bmOld;
	CRect rcClient, rcButton, rcImage;
	COLORREF crOld;
	HGDIOBJ hOldBrush;
	HGDIOBJ hOld;
	int nOldBack;

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
	hOld = pDCMem->SelectObject(m_otImageOption->GetNormalFont());

	// Select brush
	hOldBrush = pDCMem->SelectObject(GetSysColorBrush(COLOR_BTNFACE));

	// Fill background
	pDCMem->PatBlt(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom, PATCOPY);

	// Go through all images
	for (int i = 0; i < m_pImageList->GetImageCount(); i++)
	{
		// -- Get rectangle
		rcImage = m_rcRectangles.GetAt(i);

		// -- Button
		// -- -- Hover
		if (rcImage.PtInRect(m_pHover) == TRUE)
		{
			rcButton = rcImage;
			rcButton.InflateRect(3, 3, 3, 3);

			pDCMem->DrawEdge(rcButton, BDR_RAISEDINNER, BF_RECT);
		}
		// -- -- Selected
		else if (m_nSelected == i)
		{
			rcButton = rcImage;
			rcButton.InflateRect(3, 3, 3, 3);

			for (long i = rcButton.top; i < rcButton.bottom; i++)
			{
				_DrawSelectRect(pDCMem->GetSafeHdc(), rcButton.left, i, rcButton.Width());
			}

			pDCMem->DrawEdge(rcButton, BDR_SUNKENOUTER, BF_RECT);
		}
		
		// -- Draw image
		m_pImageList->Draw(pDCMem, i, CPoint(rcImage.left, rcImage.top), ILD_NORMAL);
	}

	// Draw text
	if (m_pImageList->GetImageCount() == 0)
	{
		pDCMem->DrawText(_T("No Images."), rcClient, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
	}

	// Draw edge
	pDCMem->DrawEdge(rcClient, EDGE_RAISED, BF_RECT);

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

void COptionTreeImagePopUp::OnMouseMove(UINT nFlags, CPoint point) 
{
	// Hit test
	for (int i = 0; i < m_pImageList->GetImageCount(); i++)
	{
		if (m_rcRectangles.GetAt(i).PtInRect(point))
		{
			break;
		}
	}

	// Redraw
	if (m_nSelected != i)
	{
		// -- Save point
		m_pHover = point;

		// -- Force redraw
		Invalidate();

		// -- Update window
		UpdateWindow();
	}

	CWnd::OnMouseMove(nFlags, point);
}

void COptionTreeImagePopUp::OnLButtonUp(UINT nFlags, CPoint point) 
{
	// Declare variables
	BOOL bFound = FALSE;
	CRect rcImage;
	CRect rcClient;

	// Get client rect
	GetClientRect(rcClient);

	// See if user clicked outside window
	if (rcClient.PtInRect(point) == FALSE)
	{
		// ALWAYS send message to parent
		if (IsWindow(m_pParent->GetSafeHwnd()))
		{
			m_pParent->SendMessage(OT_IMAGE_CLOSE, 0, m_nSelected);
		}

		ReleaseCapture();
		DestroyWindow();
		return;
	}

	// Go through all images
	for (int i = 0; i < m_pImageList->GetImageCount(); i++)
	{
		// -- Get rectangle
		rcImage = m_rcRectangles.GetAt(i);

		// -- See if selected
		if (m_nSelected == i)
		{
			rcImage.InflateRect(3, 3, 3, 3);
		}

		// -- Hit test
		if (rcImage.PtInRect(point) == TRUE)
		{
			// -- -- Select this image
			m_nSelected = i;

			// -- -- Found
			bFound = TRUE;

			break;
		}
	}

	// Close
	if (bFound == TRUE)
	{
		// ALWAYS send message to parent
		if (IsWindow(m_pParent->GetSafeHwnd()))
		{
			m_pParent->SendMessage(OT_IMAGE_CLOSE, 0, m_nSelected);
		}

		ReleaseCapture();
		DestroyWindow();
		return;
	}


	// Handle left click
	CWnd::OnLButtonUp(nFlags, point);
}

void COptionTreeImagePopUp::OnKillFocus(CWnd* pNewWnd) 
{
	// ALWAYS send message to parent
	if (IsWindow(m_pParent->GetSafeHwnd()))
	{
		m_pParent->SendMessage(OT_IMAGE_CLOSE, 0, m_nSelected);
	}

	// Release capture
	ReleaseCapture();

	CWnd::OnKillFocus(pNewWnd);
	
}

void COptionTreeImagePopUp::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) 
{
	// Validate selected index
	if (m_nSelected == -1)
	{
		CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	// Get key pressed
	// -- Enter
	if (nChar == VK_RETURN) 
	{
		// -- ALWAYS send message to parent
		if (IsWindow(m_pParent->GetSafeHwnd()))
		{
			m_pParent->SendMessage(OT_IMAGE_CLOSE, 0, m_nSelected);
		}

		// -- Close
		ReleaseCapture();
		DestroyWindow();
		return;
	}
	// -- Esc
	else if (nChar == VK_ESCAPE) 
	{
		// -- Close
		ReleaseCapture();
		DestroyWindow();
		return;
	}
	// -- Left
	else if (nChar == VK_LEFT) 
	{
		// -- -- Move to the left
		if (m_nSelected != 0)
		{
			m_nSelected--;
		}

		// -- -- Force redraw
		Invalidate();

		// -- -- Update window
		UpdateWindow();
	}
	// -- Right
	else if (nChar == VK_RIGHT) 
	{
		// -- -- Move to the right
		if (m_nSelected + 1 < m_pImageList->GetImageCount())
		{
			m_nSelected++;
		}

		// -- -- Force redraw
		Invalidate();

		// -- -- Update window
		UpdateWindow();
	}	
	// -- Up
	else if (nChar == VK_UP) 
	{
		// -- -- Move up
		if (m_nSelected > (m_nNumColumns - 1))
		{
			m_nSelected -= m_nNumColumns;
		}

		// -- -- Force redraw
		Invalidate();

		// -- -- Update window
		UpdateWindow();
	}
	// -- Down
	else if (nChar == VK_DOWN) 
	{
		// -- -- Move down
		if (m_nSelected + m_nNumColumns < m_pImageList->GetImageCount())
		{
			m_nSelected += m_nNumColumns;
		}

		// -- -- Force redraw
		Invalidate();

		// -- -- Update window
		UpdateWindow();
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}
