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
#include "OptionTreeItemImage.h"

// Added Headers
#include "OptionTree.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemImage

COptionTreeItemImage::COptionTreeItemImage()
{	
	// Initalize variables
	m_bFocus = FALSE;
	m_sImageSizes.cx = 0;
	m_sImageSizes.cy = 0;
	m_nSelected = -1;
	m_nNumColumns = 0;
	m_dwOptions = NULL;

	// Set item type
	SetItemType(OT_ITEM_IMAGE);
}

COptionTreeItemImage::~COptionTreeItemImage()
{
	// Reallocate
	if (m_imgImages.GetSafeHandle() != NULL)
	{
		m_imgImages.DeleteImageList();
	}
	m_strText.RemoveAll();
}


BEGIN_MESSAGE_MAP(COptionTreeItemImage, CWnd)
	//{{AFX_MSG_MAP(COptionTreeItemImage)
	ON_WM_LBUTTONUP()
	ON_WM_KILLFOCUS()
	ON_WM_SETFOCUS()
	ON_WM_PAINT()
	ON_WM_ERASEBKGND()
	//}}AFX_MSG_MAP
	ON_MESSAGE(OT_IMAGE_CLOSE, OnPopUpClose)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTreeItemImage message handlers

void COptionTreeItemImage::DrawAttribute(CDC *pDC, const RECT &rcRect)
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

	// Draw control
	DrawControl(pDC, rcRect);
}

void COptionTreeItemImage::OnCommit()
{
	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemImage::OnRefresh()
{
	// Set the window positiion
	if (IsWindow(GetSafeHwnd()))
	{
		SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	}
}

void COptionTreeItemImage::OnMove()
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
}

void COptionTreeItemImage::OnActivate()
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

void COptionTreeItemImage::CleanDestroyWindow()
{
	// Destroy window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Destroy window
		DestroyWindow();
	}
}

void COptionTreeItemImage::OnDeSelect()
{
	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}
}

void COptionTreeItemImage::OnSelect()
{
}

void COptionTreeItemImage::OnLButtonUp(UINT nFlags, CPoint point) 
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

	// Show icon popup
	new COptionTreeImagePopUp(ptPoint, m_sImageSizes, m_nNumColumns, &m_imgImages, m_otOption, m_strText, m_nSelected, this);

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}

	CWnd::OnLButtonUp(nFlags, point);
}

void COptionTreeItemImage::OnKillFocus(CWnd* pNewWnd) 
{
	// Mark focus
	m_bFocus = FALSE;

	// Commit changes
	CommitChanges();	
	
	CWnd::OnKillFocus(pNewWnd);	
}

void COptionTreeItemImage::OnSetFocus(CWnd* pOldWnd) 
{
	// Mark focus
	m_bFocus = TRUE;
	
	CWnd::OnSetFocus(pOldWnd);
}

void COptionTreeItemImage::OnPaint() 
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

BOOL COptionTreeItemImage::OnEraseBkgnd(CDC* pDC) 
{
	// Naa, we like flicker free better
	return FALSE;
}

void COptionTreeItemImage::DrawControl(CDC *pDC, const RECT &rcRect)
{
	// Declare variables
	HGDIOBJ hOld;
	COLORREF crOld;
	int nOldBack;
	CRect rcText, rcImage, rcClient;
	CString strText;
	CBrush bBrush;
	HICON hGreyScale = NULL, hSelected;
	COLORREF crOldBack;

	// Get window rect
	GetClientRect(rcClient);

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

	// Make sure we have a selected item
	if (m_nSelected != -1 && m_nSelected < m_imgImages.GetImageCount())
	{
		// -- Get icon rectangle
		rcImage.left  = rcRect.left + 1;
		rcImage.right = rcImage.left + m_sImageSizes.cx;
		rcImage.top = rcRect.top + OT_IMAGE_IMAGESPACE;
		rcImage.bottom = rcImage.top + + m_sImageSizes.cy;

		// -- Draw image
		if (IsReadOnly() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
		{
			hSelected = m_imgImages.ExtractIcon(m_nSelected);
			hGreyScale = CreateGreyScaleIcon(hSelected);
			pDC->DrawState(CPoint(rcImage.left, rcImage.top), m_sImageSizes, (HICON)hGreyScale, DST_ICON | DSS_NORMAL, (CBrush*)NULL);
		}
		else
		{
			m_imgImages.Draw(pDC, m_nSelected, CPoint(rcImage.left, rcImage.top), ILD_NORMAL);
		}


		// -- Get text rectangle
		rcText.left  = rcImage.right + OT_SPACE;
		rcText.right = rcRect.right;
		rcText.top = rcRect.top;
		rcText.bottom = rcRect.bottom;

		// -- Get text
		if (GetOption(OT_IMAGE_SHOWTEXT) == TRUE)
		{
			strText = m_strText.GetAt(m_nSelected);
		}
	}
	else
	{
		// -- Get text rectangle
		rcText.left  = rcRect.left + 1;
		rcText.right = rcRect.right;
		rcText.top = rcRect.top;
		rcText.bottom = rcRect.bottom;

		// -- No image selected
		strText = OT_IMAGE_NOSELECTION;
	}


	// Draw text
	pDC->DrawText(strText, rcText, DT_SINGLELINE | DT_VCENTER);
	pDC->DrawText(strText, rcText, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);

	// Restore GDI ojects
	pDC->SelectObject(hOld);
	pDC->SetTextColor(crOld);
	pDC->SetBkMode(nOldBack);
	pDC->SetBkColor(crOldBack);

	// Delete grey icon
	if (hSelected != NULL)
	{
		::DestroyIcon(hSelected);
	}
	if (hGreyScale != NULL)
	{
		::DestroyIcon(hGreyScale);
	}
}

BOOL COptionTreeItemImage::CreateImageItem(DWORD dwOptions, CSize sImageSizes, int nNumberColumns)
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

			// -- -- Save options
			m_dwOptions = dwOptions;

			// -- -- Create imagelist
			if (m_imgImages.GetSafeHandle() != NULL)
			{
				m_imgImages.DeleteImageList();
			}
			if (!m_imgImages.Create(sImageSizes.cx, sImageSizes.cy, ILC_COLORDDB | ILC_MASK, 1, OT_IMAGE_MAXIMAGES))
			{
				bRet = FALSE;
			}	

			// -- -- Set item height
			SetItemHeight((long) sImageSizes.cy + (OT_IMAGE_IMAGESPACE * 2));

			// -- -- Set number of columns
			SetNumberColumns(nNumberColumns);

			// Set image height
			SetImageSize(sImageSizes);

			// -- -- Set window position
			SetWindowPos(NULL, m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.Width(), m_rcAttribute.Height(), SWP_NOZORDER | SWP_SHOWWINDOW);

			// -- -- Hide window
			ShowWindow(SW_HIDE);
		}
	}

	return bRet;
}

void COptionTreeItemImage::SetImageSize(CSize sSize)
{
	m_sImageSizes = sSize;
}

CSize COptionTreeItemImage::GetImageSize()
{
	return m_sImageSizes;
}

void COptionTreeItemImage::SetNumberColumns(int nNum)
{
	m_nNumColumns = nNum;
}

BOOL COptionTreeItemImage::GetOption(DWORD dwOption)
{
	// Return option
	return (m_dwOptions & dwOption) ? TRUE : FALSE;
}

void COptionTreeItemImage::SetOption(DWORD dwOption, BOOL bSet)
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

void COptionTreeItemImage::AddIcon(HICON hIcon, CString strText)
{
	// Add image
	m_imgImages.Add(hIcon);

	// Add text
	m_strText.Add(strText);
}

void COptionTreeItemImage::AddIcon(UINT uIcon, CString strText)
{
	// Add icon
	AddIcon((HICON) LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(uIcon), IMAGE_ICON, m_sImageSizes.cx, m_sImageSizes.cy, LR_DEFAULTCOLOR), strText);
}

void COptionTreeItemImage::AddIcon(CString strIcon, CString strText)
{
	// Add icon
	AddIcon((HICON) LoadImage(AfxGetInstanceHandle(), strIcon, IMAGE_ICON, m_sImageSizes.cx, m_sImageSizes.cy, LR_DEFAULTCOLOR), strText);
}

void COptionTreeItemImage::AddBitmap(CBitmap &bBitmap, COLORREF crMask, CString strText)
{
	// Add image
	m_imgImages.Add(&bBitmap, crMask);

	// Add text
	m_strText.Add(strText);
}

void COptionTreeItemImage::AddBitmap(UINT uBitmap, COLORREF crMask, CString strText)
{
	// Declare variables
	CBitmap bBitmap;

	// Load bitmap
	bBitmap.LoadBitmap(uBitmap);

	// Add bitmap
	AddBitmap(bBitmap, crMask, strText);
}

void COptionTreeItemImage::AddBitmap(CString strBitmap, COLORREF crMask, CString strText)
{
	// Declare variables
	CBitmap bBitmap;

	// Load bitmap
	bBitmap.LoadBitmap(strBitmap);

	// Add bitmap
	AddBitmap(bBitmap, crMask, strText);
}

int COptionTreeItemImage::GetSelection()
{
	return m_nSelected;
}

void COptionTreeItemImage::SetSelection(int nSelection)
{
	m_nSelected = nSelection;
}

BOOL COptionTreeItemImage::GetBitmapFromImageList(CDC *pDC, HICON hIcon, CBitmap &bmp)
{
	// Declare varaibles
	CDC dc;
	CBrush brush;

	dc.CreateCompatibleDC(pDC);
	bmp.CreateCompatibleBitmap(pDC, m_sImageSizes.cx, m_sImageSizes.cy);
	CBitmap* pOldBmp = dc.SelectObject(&bmp);
	brush.CreateSolidBrush(GetSysColor(COLOR_MENU));
	::DrawIconEx(dc.GetSafeHdc(), 0, 0, hIcon, m_sImageSizes.cx, m_sImageSizes.cy, 0, (HBRUSH)brush,DI_NORMAL);

	// Restore
	dc.SelectObject(pOldBmp);

	// Delete
	if (brush.GetSafeHandle() != NULL)
	{
		brush.DeleteObject();
	}
	if (dc.GetSafeHdc() != NULL)
	{
		dc.DeleteDC();
	}

	// The icon is not longer needed
	DestroyIcon(hIcon);

	return TRUE;
}

void COptionTreeItemImage::DitherBlt(CDC *drawdc, int nXDest, int nYDest, int nWidth, int nHeight, CBitmap &bmp, int nXSrc, int nYSrc)
{
	// Create a monochrome memory DC
	CDC ddc;
	ddc.CreateCompatibleDC(0);
	CBitmap bwbmp;
	bwbmp.CreateCompatibleBitmap(&ddc, nWidth, nHeight);
	CBitmap * pddcOldBmp = ddc.SelectObject(&bwbmp);
	CDC dc;
	dc.CreateCompatibleDC(0);
	CBitmap * pdcOldBmp = dc.SelectObject(&bmp);
	COLORREF crOldback, crOldText;
	
	// Build a mask
	ddc.PatBlt(0, 0, nWidth, nHeight, WHITENESS);
	crOldback = dc.SetBkColor(GetSysColor(COLOR_BTNFACE));
	ddc.BitBlt(0, 0, nWidth, nHeight, &dc, nXSrc,nYSrc, SRCCOPY);
	dc.SetBkColor(GetSysColor(COLOR_BTNHILIGHT));
	ddc.BitBlt(0, 0, nWidth, nHeight, &dc, nXSrc,nYSrc, SRCPAINT);

	// Copy the image from the toolbar into the memory DC
	// and draw it (grayed) back into the toolbar.
	dc.FillSolidRect(0,0, nWidth, nHeight, GetSysColor((TRUE/*IsNewShell()*/)?COLOR_3DFACE:COLOR_MENU));
	
	// SK: Looks better on the old shell
	dc.SetBkColor(RGB(0, 0, 0));
	crOldText = dc.SetTextColor(RGB(255, 255, 255));
	CBrush brShadow, brHilight;
	brHilight.CreateSolidBrush(GetSysColor(COLOR_BTNHILIGHT));
	brShadow.CreateSolidBrush(GetSysColor(COLOR_BTNSHADOW));
	CBrush * pOldBrush = dc.SelectObject(&brHilight);
	dc.BitBlt(0,0, nWidth, nHeight, &ddc, 0, 0, 0x00E20746L);
	drawdc->BitBlt(nXDest+1,nYDest+1,nWidth, nHeight, &dc,0,0,SRCCOPY);
	dc.BitBlt(1,1, nWidth, nHeight, &ddc, 0, 0, 0x00E20746L);
	dc.SelectObject(&brShadow);
	dc.BitBlt(0,0, nWidth, nHeight, &ddc, 0, 0, 0x00E20746L);
	drawdc->BitBlt(nXDest,nYDest,nWidth, nHeight, &dc,0,0,SRCCOPY);
	
	// Restore DCs
	ddc.SelectObject(pddcOldBmp);
	dc.SelectObject(pOldBrush);
	dc.SelectObject(pdcOldBmp);
	dc.SetBkColor(crOldback);
	dc.SetTextColor(crOldText);

	// Delete
	if (bwbmp.GetSafeHandle() != NULL)
	{
		bwbmp.DeleteObject();
	}
	if (brHilight.GetSafeHandle() != NULL)
	{
		brHilight.DeleteObject();
	}
	if (brShadow.GetSafeHandle() != NULL)
	{
		brShadow.DeleteObject();
	}
	if (ddc.GetSafeHdc() != NULL)
	{
		ddc.DeleteDC();
	}
	if (dc.GetSafeHdc() != NULL)
	{
		dc.DeleteDC();
	}
}

long COptionTreeItemImage::OnPopUpClose(UINT lParam, long wParam)
{
	// Get selected
	SetSelection((int) wParam);

	// Hide window
	if (IsWindow(GetSafeHwnd()))
	{
		// -- Show window
		ShowWindow(SW_HIDE);
	}

	// Kill focus so we can draw attribute
	m_bFocus = FALSE;

	// Update items
	if (m_otOption != NULL)
	{
		m_otOption->UpdatedItems();
	}

    return TRUE;
}

HICON COptionTreeItemImage::CreateGreyScaleIcon(HICON hIcon)
{
	// Declare variables
    HICON hGrayIcon = NULL;
    HDC hMainDC = NULL, hMemDC1 = NULL, hMemDC2 = NULL;
    BITMAP bmp;
    HBITMAP hOldBmp1 = NULL, hOldBmp2 = NULL;
    ICONINFO csII, csGrayII;
    BOOL bRetValue = FALSE;

    bRetValue = ::GetIconInfo(hIcon, &csII);
    if (bRetValue == FALSE) return NULL;

	hMainDC = ::GetDC(::GetDesktopWindow());  
	if (hMainDC == NULL) 
		return NULL;  
	hMemDC1 = ::CreateCompatibleDC(hMainDC);  
	hMemDC2 = ::CreateCompatibleDC(hMainDC);  
	if (hMemDC1 == NULL || hMemDC2 == NULL) 
		return NULL; 
  
    if (::GetObject(csII.hbmColor, sizeof(BITMAP), &bmp))
    {
        csGrayII.hbmColor = ::CreateBitmap(csII.xHotspot*2, csII.yHotspot*2, 
                                           bmp.bmPlanes, bmp.bmBitsPixel, NULL);
        if (csGrayII.hbmColor)
        {
            hOldBmp1 = (HBITMAP)::SelectObject(hMemDC1, csII.hbmColor);
            hOldBmp2 = (HBITMAP)::SelectObject(hMemDC2, csGrayII.hbmColor);

            ::BitBlt(hMemDC2, 0, 0, csII.xHotspot*2, csII.yHotspot*2, hMemDC1, 
                     0, 0, SRCCOPY);

            DWORD    dwLoopY = 0, dwLoopX = 0;
            COLORREF crPixel = 0;
            BYTE     byNewPixel = 0;

            for (dwLoopY = 0; dwLoopY < csII.yHotspot*2; dwLoopY++)
            {
                for (dwLoopX = 0; dwLoopX < csII.xHotspot*2; dwLoopX++)
                {
                    crPixel = ::GetPixel(hMemDC2, dwLoopX, dwLoopY);

                    byNewPixel = (BYTE)((GetRValue(crPixel) * 0.299) + 
                                        (GetGValue(crPixel) * 0.587) + 
                                        (GetBValue(crPixel) * 0.114));
                    if (crPixel) ::SetPixel(hMemDC2, dwLoopX, dwLoopY, 
                                            RGB(byNewPixel, byNewPixel, byNewPixel));
                } // for
            } // for

            ::SelectObject(hMemDC1, hOldBmp1);
            ::SelectObject(hMemDC2, hOldBmp2);

            csGrayII.hbmMask = csII.hbmMask;

            csGrayII.fIcon = TRUE;
            hGrayIcon = ::CreateIconIndirect(&csGrayII);
        } // if

        ::DeleteObject(csGrayII.hbmColor);
        //::DeleteObject(csGrayII.hbmMask);
    } // if

    ::DeleteObject(csII.hbmColor);
    ::DeleteObject(csII.hbmMask);
    ::DeleteDC(hMemDC1);
    ::DeleteDC(hMemDC2);
    //::ReleaseDC(m_hWnd, hMainDC);

    return hGrayIcon;
}
