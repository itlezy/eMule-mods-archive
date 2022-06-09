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
#include "OptionTreeItem.h"
#include "OptionTree.h"
#include "OptionTreeDef.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// -- Make short string
static LPCTSTR MakeShortString(CDC *pDC, const CString &strLong, int nWidth, int nOffset)
{
	static const TCHAR szThreeDots[] = _T("...");
	static TCHAR szShort[MAX_PATH];
	int nAddLen;

	// Validate length
	if (strLong.IsEmpty() || (pDC->GetTextExtent(strLong).cx + nOffset <= nWidth))
		return strLong.GetString();

	// Shorten
	lstrcpy(szShort, strLong.GetString());
	nAddLen = pDC->GetTextExtent(szThreeDots, CSTRLEN(szThreeDots)).cx;
/*	for(int i = nStringLen - 1; i > 0; i--)
	{
		szShort[i] = 0;
		if(pDC->GetTextExtent(szShort, i).cx + nOffset + nAddLen <= nWidth)
		{
			break;
		}
	}
*/
	// pooz
	LPTSTR lpszEnd = szShort + strLong.GetLength();
	while (lpszEnd > szShort)
	{
		lpszEnd = CharPrev(szShort, lpszEnd);
		*lpszEnd = 0;
		if(pDC->GetTextExtent(szShort, lpszEnd - szShort).cx + nOffset + nAddLen <= nWidth)
			break;
	}
	// end pooz
	lstrcat(szShort, szThreeDots);

	return szShort;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

COptionTreeItem::COptionTreeItem()
{
	// Initialize variables
	m_strInfoText = _T("");
	m_strLabel = _T("");
	m_uControlID = 0;
	m_otiParent = NULL;
	m_otiSibling = NULL;
	m_otiChild = NULL;
	m_otiNextVisible = NULL;
	m_rcExpand = CRect(0, 0, 0, 0);
	m_rcAttribute = CRect(0, 0, 0, 0);
	m_rcLabelRect = CRect(0, 0, 0, 0);
	m_otOption = NULL;
	m_bCommitOnce = FALSE;
	m_lParam = NULL;
	m_lItemHeight = OT_DEFHEIGHT;
	m_bSelected = FALSE;
	m_bExpanded = FALSE;
	m_bActivated = FALSE;
	m_bReadOnly = FALSE;
	m_nItemType = OT_ITEM_STATIC;
	m_bDrawMultiline = FALSE;
	m_crBackground = GetSysColor(COLOR_WINDOW);
	m_crLabelText = GetSysColor(COLOR_BTNTEXT);
	m_crRootBackground = GetSysColor(COLOR_BTNFACE);
	m_crLabelBackground = GetSysColor(COLOR_WINDOW);
	m_crText = GetSysColor(COLOR_BTNTEXT);
}

COptionTreeItem::~COptionTreeItem()
{

}

void COptionTreeItem::SetInfoText(CString strText)
{
	// Set info text
	m_strInfoText = strText;
}

CString COptionTreeItem::GetInfoText()
{
	// Return info text
	return m_strInfoText;
}

void COptionTreeItem::SetLabelText(CString strLabel)
{
	// Set label
	m_strLabel = strLabel;
}

CString COptionTreeItem::GetLabelText()
{
	// Return label
	return m_strLabel;
}

void COptionTreeItem::SetCtrlID(UINT nID)
{
	// Set the ID
	m_uControlID = nID;
}

UINT COptionTreeItem::GetCtrlID()
{
	// Return ID
	return m_uControlID;
}

COptionTreeItem * COptionTreeItem::GetParent()
{
	// Return parent
	return m_otiParent;
}

void COptionTreeItem::SetParent(COptionTreeItem *otiParent)
{
	// Set parent
	m_otiParent = otiParent;
}

COptionTreeItem * COptionTreeItem::GetSibling()
{
	// Return sibling
	return m_otiSibling;
}

void COptionTreeItem::SetSibling(COptionTreeItem *otiSibling)
{
	// Set sibling
	m_otiSibling = otiSibling;
}

COptionTreeItem * COptionTreeItem::GetChild()
{
	// Return child
	return m_otiChild;
}

void COptionTreeItem::SetChild(COptionTreeItem *otiChild)
{
	// Set child
	m_otiChild = otiChild;
}

COptionTreeItem * COptionTreeItem::GetNextVisible()
{
	// Return next visible
	return m_otiNextVisible;
}

void COptionTreeItem::SetNextVisible(COptionTreeItem *otiNetVisible)
{
	// Set child
	m_otiNextVisible = otiNetVisible;
}

BOOL COptionTreeItem::IsExpanded() const
{
	return m_bExpanded;
}

BOOL COptionTreeItem::IsSelected()
{
	// Return value
	return m_bSelected;
}

BOOL COptionTreeItem::IsReadOnly()
{
	// Return value
	return m_bReadOnly;
}

BOOL COptionTreeItem::IsActivated()
{
	// Return value
	return m_bActivated;
}

void COptionTreeItem::Select(BOOL bSelect)
{
	// Select
	m_bSelected = bSelect;

	// Send message
	if (bSelect == FALSE)
	{
		OnDeSelect();
	}
	else
	{
		OnSelect();
	}
}

void COptionTreeItem::Expand(BOOL bExpand)
{
	// Make sure not null
	if (m_otOption == NULL)
	{
		return;
	}

	// Expand
	m_bExpanded = bExpand;

	// On expand
	m_otOption->Expand(this, bExpand);
}

void COptionTreeItem::ReadOnly(BOOL bReadOnly)
{
	// Read Only
	m_bReadOnly = bReadOnly;
}

BOOL COptionTreeItem::HitExpand(const POINT &pt)
{
	// Hit
	return m_rcExpand.PtInRect(pt);
}

BOOL COptionTreeItem::IsRootLevel()
{
	// Make sure not null
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Return
	return GetParent() == m_otOption->GetRootItem();
}

long COptionTreeItem::GetTotalHeight()
{
	// Declare variables
	COptionTreeItem *otiItem;
	long lHeight;

	// Get height
	lHeight = GetHeight();

	// Add up total height if expanded
	if (IsExpanded() == TRUE)
	{
		// -- Go through all children
		for (otiItem = GetChild(); otiItem != NULL; otiItem = otiItem->GetSibling())
		{
			lHeight += otiItem->GetTotalHeight();
		}
	}

	return lHeight;
}

void COptionTreeItem::OnCommit()
{
	// Do nothing
}

void COptionTreeItem::OnRefresh()
{
// Do nothing
}

void COptionTreeItem::OnMove()
{
// Do nothing
}

void COptionTreeItem::SetOptionsOwner(COptionTree *otOption)
{
	// Save pointer
	m_otOption = otOption;
}

long COptionTreeItem::DrawItem(CDC *pDC, const RECT &rcRect, long x, long y)
{
	// Make sure not null
	if (m_otOption == NULL)
	{
		return FALSE;
	}

	// Declare variables
	COptionTreeItem* otiNext;
	CPoint ptPoint;
	long lTotal, lCol, lHeight;
	CRect rcDRC, rcIR, rcTemp, rcLabelRight, rcRegion;
	HGDIOBJ hOld, hHighlight;
	HRGN hRgn = NULL;
	COLORREF crOld;
	int nOldBack;
	CString strShortLabel;
	CBrush brBack;
	CRect rcBack;
	CFont *pOldFont;

	// Add TreeItem the list of visible items
	m_otOption->AddToVisibleList(this);

	// Store the item's location
	m_ptLocation = CPoint(x, y);

	// Store the items rectangle position
	// -- Attribute
	m_rcAttribute.SetRect(m_otOption->GetOrigin().x + OT_SPACE, m_ptLocation.y, rcRect.right, m_ptLocation.y + GetHeight() - 1);
	m_rcAttribute.OffsetRect(0, -m_otOption->GetOrigin().y);
	// -- Background
	rcBack.SetRect(m_otOption->GetOrigin().x, m_ptLocation.y, rcRect.right - 2, m_ptLocation.y + GetHeight() - 1);
	rcBack.OffsetRect(0, -m_otOption->GetOrigin().y);

	// Intialize temporary drawing variables
	lTotal = GetHeight();

	// Convert item coordinates to screen coordinates
	ptPoint = m_ptLocation;
	ptPoint.y -= m_otOption->GetOrigin().y;
	lCol = m_otOption->GetOrigin().x;

	// See if root level
	if (IsRootLevel() == TRUE)
	{
		rcDRC.SetRect(ptPoint.x + OT_EXPANDCOLUMN, ptPoint.y, rcRect.right, ptPoint.y + lTotal);
		rcRegion.SetRect(ptPoint.x + OT_EXPANDCOLUMN, ptPoint.y, rcRect.right, ptPoint.y + lTotal);
	}
	else
	{
		rcDRC.SetRect(ptPoint.x + OT_EXPANDCOLUMN, ptPoint.y, lCol, ptPoint.y + lTotal);
		rcRegion.SetRect(OT_EXPANDCOLUMN, ptPoint.y, lCol, ptPoint.y + lTotal);
	}

	// Shade root levels
	if (IsRootLevel() == TRUE)
	{
		// -- Create brush
		brBack.CreateSolidBrush(m_crRootBackground);

		// -- Select brush
		if (m_otOption->GetShadeRootItems() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
		{
			hOld = pDC->SelectObject(&brBack);
		}
		else
		{
			hOld = pDC->SelectObject(GetSysColorBrush(COLOR_WINDOW));
		}

		// -- Fill rectangle
		pDC->PatBlt(rcRect.left + OT_EXPANDCOLUMN, rcDRC.top, rcRect.right - rcRect.left + 1, rcDRC.Height(), PATCOPY);

		// -- Restore GDI objects
		if(m_otOption->GetShadeRootItems() == TRUE || m_otOption->IsWindowEnabled() == FALSE)
		{
			pDC->SelectObject((CBrush*)hOld);
		}
		else
		{
		pDC->SelectObject(hOld);
		}
		
		// -- Delete objects
		if (brBack.GetSafeHandle() != NULL)
		{
			brBack.DeleteObject();
		}
	}
	else
	{
		// -- Draw the item background
		brBack.CreateSolidBrush(m_crBackground);
		pDC->FillRect(&rcBack, &brBack);
		if (brBack.GetSafeHandle() != NULL)
		{
			brBack.DeleteObject();
		}
	}

	// Calc and draw expanded box position
	if (GetChild())
	{
		// -- Calculate expand rectangle
		m_rcExpand.left = OT_EXPANDCOLUMN/2 - OT_EXPANDBOXHALF;
		m_rcExpand.top = m_ptLocation.y + OT_SPACE;
		m_rcExpand.right = m_rcExpand.left + OT_EXPANDBOX - 1;
		m_rcExpand.bottom = m_rcExpand.top + OT_EXPANDBOX - 1;
		rcIR = m_rcExpand;
		rcIR.OffsetRect(0, -m_otOption->GetOrigin().y);

		// -- Draw expanded
		_DrawExpand(pDC->GetSafeHdc(), rcIR.left, rcIR.top, IsExpanded(), !IsRootLevel());
	}
	else
	{
		m_rcExpand.SetRectEmpty();
	}

	// Create a clipping region for the label
	if (IsRootLevel() == FALSE)
	{
		hRgn = CreateRectRgn(rcDRC.left, rcDRC.top, rcDRC.right, rcDRC.bottom);
		SelectClipRgn(pDC->GetSafeHdc(), hRgn);
	}

	// Calculate label position
	rcIR = rcDRC;
	rcIR.left += OT_SPACE;

	// Draw the label background
	if (IsRootLevel() == FALSE)
	{
		rcBack = rcDRC;
		brBack.CreateSolidBrush(m_crLabelBackground);
		pDC->FillRect(&rcBack, &brBack);
		if (brBack.GetSafeHandle() != NULL)
		{
			brBack.DeleteObject();
		}
	}

	// Draw the label
	if (IsStringEmpty(m_strLabel) == FALSE)
	{
		// -- Get the font
		if (IsRootLevel() == TRUE)
		{
			pOldFont = pDC->SelectObject(COptionTree::GetBoldFont());
		}
		else
		{
			pOldFont = pDC->SelectObject(COptionTree::GetNormalFont());
		}

		// -- Set text color and background mode
		crOld = pDC->SetTextColor(m_crLabelText);
		nOldBack = pDC->SetBkMode(TRANSPARENT);

		// -- Draw the text highlighted if selected
		if (IsSelected() == TRUE)
		{
			// -- -- Select th objects
			hOld = pDC->SelectObject(GetStockObject(NULL_PEN));
			hHighlight = pDC->SelectObject(GetSysColorBrush(COLOR_HIGHLIGHT));

			// -- -- Calculate highlighted rectangle
			rcTemp = rcDRC;
			rcTemp.left = OT_EXPANDCOLUMN;
			rcTemp.right += OT_COLRNG;

			// -- -- Draw rectangle
			pDC->Rectangle(&rcTemp);

			// -- -- Restore GDI objects
			pDC->SelectObject(hOld);
			pDC->SelectObject(hHighlight);

			// -- -- Set text color for highlighted
			pDC->SetTextColor(GetSysColor(COLOR_BTNHIGHLIGHT));
		}

		// -- Check if we need to draw the text as disabled
		if (m_otOption->IsWindowEnabled() == FALSE)
		{
			pDC->SetTextColor(GetSysColor(COLOR_GRAYTEXT));
		}

		// -- Draw text
		// -- -- Edit multiline
		if (GetDrawMultiline() == TRUE)
		{
			// -- -- -- Center rectangle
			rcLabelRight = rcIR;
			rcLabelRight.top += OT_SPACE;

			// -- -- -- Shorten string
			strShortLabel = MakeShortString(pDC, m_strLabel, rcLabelRight.Width(), 1);

			// -- -- -- Draw text
			pDC->DrawText(strShortLabel, &rcLabelRight, DT_SINGLELINE | DT_TOP);
			pDC->DrawText(strShortLabel, &rcLabelRight, DT_SINGLELINE | DT_TOP | DT_CALCRECT);

			// -- -- -- Set label right
			SetLabelRect(rcLabelRight);
		}
		// -- Normal
		else
		{
			// -- -- -- Calculate rectangle
			rcLabelRight = rcIR;

			// -- -- -- Shorten string
			strShortLabel = MakeShortString(pDC, m_strLabel, rcLabelRight.Width(), 1);

			// -- -- -- Draw text
			pDC->DrawText(strShortLabel, &rcLabelRight, DT_SINGLELINE | DT_VCENTER);
			pDC->DrawText(strShortLabel, &rcLabelRight, DT_SINGLELINE | DT_VCENTER | DT_CALCRECT);

			// -- -- -- Set label right
			SetLabelRect(rcLabelRight);
		}

		// -- Restore GDI objects
		pDC->SetTextColor(crOld);
		pDC->SetBkMode(nOldBack);
		pDC->SelectObject(pOldFont);
	}


	// Remove clip region
	if (hRgn != NULL)
	{
		// -- Select clip region
		SelectClipRgn(pDC->GetSafeHdc(), NULL);

		// -- Delete region
		DeleteObject(hRgn);
	}

	// Draw horizontal sep
	if (IsRootLevel() == TRUE)
	{
		//  - OT_EXPANDCOLUMN + 1
		_DrawDarkHLine(pDC->GetSafeHdc(), OT_EXPANDCOLUMN, ptPoint.y + lTotal - 1, rcRect.right);
		_DrawDarkHLine(pDC->GetSafeHdc(), OT_EXPANDCOLUMN, ptPoint.y - 1, rcRect.right);
	}
	else
	{
		_DrawDarkHLine(pDC->GetSafeHdc(), OT_EXPANDCOLUMN, ptPoint.y + lTotal - 1, rcRect.right);
	}

	// Draw vertical sep
	_DrawDarkVLine(pDC->GetSafeHdc(), OT_EXPANDCOLUMN, rcRect.top, rcRect.bottom);

	// Draw separators
	if (IsRootLevel() == FALSE)
	{
		_DrawDarkVLine(pDC->GetSafeHdc(), lCol, rcDRC.top, rcDRC.Height()); 
	}

	// Draw attribute
	if (!IsRootLevel())
	{
		// -- Create clip region
		hRgn = CreateRectRgn(m_rcAttribute.left, m_rcAttribute.top, m_rcAttribute.right, m_rcAttribute.bottom);
		SelectClipRgn(pDC->GetSafeHdc(), hRgn);
		
		// -- Draw attribute
		DrawAttribute(pDC, m_rcAttribute);

		// -- Select clip region
		SelectClipRgn(pDC->GetSafeHdc(), NULL);

		// -- Delete object
		DeleteObject(hRgn);
	}

	// Draw children
	if (GetChild() != NULL && IsExpanded() == TRUE)
	{
		// -- Add to Y
		y += lTotal;

		for (otiNext = GetChild(); otiNext != NULL; otiNext = otiNext->GetSibling())
		{
			// -- -- Draw child
			lHeight = otiNext->DrawItem(pDC, rcRect, x + (IsRootLevel() ? 0 : OT_PNINDENT), y);
			
			// -- -- Add to total
			lTotal += lHeight;

			// -- -- Add to Y
			y += lHeight;
		}
	}

	return lTotal;
}

const POINT& COptionTreeItem::GetLocation()
{
	return m_ptLocation;
}


long COptionTreeItem::GetHeight()
{
	// Return item height
	return m_lItemHeight;
}

void COptionTreeItem::_DrawExpand(HDC hdc, long lX, long lY, BOOL bExpand, BOOL bFill)
{
	// Declare variables
	HPEN hPen;
	HPEN oPen;
	HBRUSH oBrush;

	// Create pens
	hPen = CreatePen(PS_SOLID, 1, GetSysColor(COLOR_BTNSHADOW));
	oPen = (HPEN)SelectObject(hdc, hPen);
	if (bFill == TRUE)
	{
		oBrush = (HBRUSH)SelectObject(hdc, GetSysColorBrush(COLOR_WINDOW));
	}
	else
	{
		oBrush = (HBRUSH)SelectObject(hdc, GetStockObject(NULL_BRUSH));
	}

	// Draw rectangle
	Rectangle(hdc, lX, lY, lX + OT_EXPANDBOX, lY + OT_EXPANDBOX);
	SelectObject(hdc, GetStockObject(BLACK_PEN));

	// If not expanded
	if (bExpand == FALSE)
	{
		MoveToEx(hdc, lX + OT_EXPANDBOXHALF, lY + 2, NULL);
		LineTo(hdc, lX + OT_EXPANDBOXHALF, lY + OT_EXPANDBOX - 2);
	}

	// Draw lower line
	MoveToEx(hdc, lX + 2, lY + OT_EXPANDBOXHALF, NULL);
	LineTo(hdc, lX + OT_EXPANDBOX - 2, lY + OT_EXPANDBOXHALF);

	// Restore GDI objects
	SelectObject(hdc, oPen);
	SelectObject(hdc, oBrush);
	DeleteObject(hPen);
}

BOOL COptionTreeItem::IsStringEmpty(CString strString)
{
	// Declare variables
	BOOL bEmpty = TRUE;

	// Go through each character
	for (int i = 0; i < strString.GetLength(); i++)
	{
		if (strString.GetAt(i) != ' ')
		{
			bEmpty = FALSE;
			break;
		}
	}

	return bEmpty;
}

void COptionTreeItem::DrawAttribute(CDC *pDC, const RECT &rcRect)
{
	NOPRM(pDC); NOPRM(rcRect);
	// Do nothing here
}

void COptionTreeItem::Activate()
{
	// Set variables
	m_bActivated = TRUE;
	m_bCommitOnce = FALSE;

	// Activate
	OnActivate();
}

void COptionTreeItem::CommitChanges()
{
	// Set variables
	m_bActivated = FALSE;

	// Make sure we commit changes
	if (m_bCommitOnce == TRUE)
	{
		return;
	}

	// Commit changes
	m_bCommitOnce = TRUE;

	// Make sure valid
	if (m_otOption == NULL)
	{
		return;
	}

	// Commit
	OnCommit();

	// Send notify to user
	m_otOption->SendNotify(OT_NOTIFY_ITEMCHANGED, this);

	// Refresh items
	m_otOption->RefreshItems(this);
}

void COptionTreeItem::OnActivate()
{
	// No attributes, do nothing
}

void COptionTreeItem::SetItemHeight(long lHeight)
{
	// Save height
	m_lItemHeight = lHeight;
}

void COptionTreeItem::CleanDestroyWindow()
{
	// Do nothing here
}

void COptionTreeItem::OnDeSelect()
{
	// Do nothing here
}

void COptionTreeItem::OnSelect()
{
	// Do nothing here
}

void COptionTreeItem::OnExpand(BOOL bExpand)
{
	NOPRM(bExpand);
	// Do nothing here
}

CRect COptionTreeItem::GetLabelRect()
{
	// Return variable
	return m_rcLabelRect;
}

void COptionTreeItem::SetLabelRect(CRect rcLabel)
{
	// Save variable
	m_rcLabelRect = rcLabel;
}

void COptionTreeItem::SetItemType(int nType)
{
	// Save type
	m_nItemType = nType;
}

int COptionTreeItem::GetItemType()
{
	// Return
	return m_nItemType;
}

void COptionTreeItem::SetDrawMultiline(BOOL bMultiline)
{
	m_bDrawMultiline = bMultiline;
}

BOOL COptionTreeItem::GetDrawMultiline()
{
	return m_bDrawMultiline;
}

void COptionTreeItem::SetBackgroundColor(COLORREF crColor)
{
	m_crBackground = crColor;
}

COLORREF COptionTreeItem::GetBackgroundColor()
{
	return m_crBackground;
}

void COptionTreeItem::SetLabelTextColor(COLORREF crColor)
{
	m_crLabelText = crColor;
}

COLORREF COptionTreeItem::GetLabelTextColor()
{
	return m_crLabelText;
}

void COptionTreeItem::SetRootBackgroundColor(COLORREF crColor)
{
	m_crRootBackground = crColor;
}

COLORREF COptionTreeItem::GetRootBackgroundColor()
{
	return m_crRootBackground;
}

void COptionTreeItem::SetLabelBackgroundColor(COLORREF crColor)
{
	m_crLabelBackground = crColor;
}

COLORREF COptionTreeItem::GetLabelBackgroundColor()
{
	return m_crLabelBackground;
}

void COptionTreeItem::SetTextColor(COLORREF crColor)
{
	m_crText = crColor;
}

COLORREF COptionTreeItem::GetTextColor()
{
	return m_crText;
}
