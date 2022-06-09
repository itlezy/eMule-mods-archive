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
#include "OptionTree.h"
#include "OptionTreeDef.h"
#include "..\otherfunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COptionTree

// Static variables
CFont* COptionTree::m_fNormalFont;
CFont* COptionTree::m_fBoldFont;
UINT COptionTree::m_uInstanceCount;
COptionTreeItem *COptionTree::m_otiFound;
CRect COptionTree::m_rcLargestLabel = CRect(0, 0, 0, 0);

// Font callback
static int CALLBACK _FontFamilyProcFonts(const LOGFONT FAR* lplf, const TEXTMETRIC FAR*, ULONG, LPARAM)
{
	ASSERT(lplf != NULL);

	CString strFont = lplf->lfFaceName;

	return strFont.CollateNoCase (_T("Tahoma")) == 0 ? 0 : 1;
}


COptionTree::COptionTree()
{
	// Initialize variables
	m_dwTreeOptions = 0;
	m_otiVisibleList = NULL;
	m_otiFocus = NULL;
	m_bDisableInput = FALSE;
	m_uLastUID = 0;
	m_ptOrigin = CPoint(150, 0);
	m_pNotify = NULL;

	// Initialize global resources for all COptionTree
	// -- Initialize global resources
	if (!m_uInstanceCount)
	{
		InitGlobalResources();
	}
	// -- Increase instance number
	m_uInstanceCount++;
}

COptionTree::~COptionTree()
{
	// Delete all items
	DeleteAllItems();

	// Delete global resources for all COptionTree
	// -- Decrease instance number
	m_uInstanceCount--;
	// -- Delete global resources
	if (!m_uInstanceCount)
	{
		DeleteGlobalResources();
	}
}


BEGIN_MESSAGE_MAP(COptionTree, CWnd)
	//{{AFX_MSG_MAP(COptionTree)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_ENABLE()
	ON_WM_SYSCOLORCHANGE()
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_SIZING()
	ON_WM_SETFOCUS()
	ON_WM_KILLFOCUS()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COptionTree message handlers

BOOL COptionTree::Create(DWORD dwStyle, RECT rcRect, CWnd* pParentWnd, DWORD dwTreeOptions, UINT nID)
{
	// Save tree options
	m_dwTreeOptions = dwTreeOptions;

	// Add style
	dwStyle |= WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	// Create the window
	return CWnd::Create(AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW, ::LoadCursor(NULL, IDC_ARROW)), _T(""), dwStyle, rcRect, pParentWnd, nID);
}

int COptionTree::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	// Create window
	if (CWnd::OnCreate(lpCreateStruct) == -1)
	{
		return -1;
	}
	
	// Declare variables
	DWORD dwStyle;
	CRect rcClient;

	// Get client rectangle
	GetClientRect(rcClient);

	// Setup the window style
	dwStyle = WS_VISIBLE | WS_CHILD;

	// Create the information window
	m_otInfo.SetOptionsOwner(this);
	if(m_otInfo.Create(_T(""), dwStyle, rcClient, this) == FALSE)
	{
		return -1;
	}

	// Modify style
	dwStyle |= WS_VSCROLL;

	// Create the list window
	m_otlList.SetOptionsOwner(this);
	if (m_otlList.Create(dwStyle, rcClient, this, OT_TREELIST_ID) == FALSE)
	{
		return -1;
	}

	// Hide info window
	if (GetShowInfoWindow() == FALSE)
	{
		if (::IsWindow(m_otInfo.GetSafeHwnd()))
		{
			m_otInfo.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		if (::IsWindow(m_otInfo.GetSafeHwnd()))
		{
			m_otInfo.ShowWindow(SW_SHOW);
		}
	}

	// Resize all windows
	ResizeAllWindows(rcClient.Width(), rcClient.Height());

	// Show window
	ShowWindow(SW_SHOW);

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();

	return 0;
}

void COptionTree::ShowInfoWindow(BOOL bShow)
{
	// Declare variables
	CRect rcClient;

	// Get client rectangle
	GetClientRect(rcClient);

	// Set option
	if (bShow == TRUE)
	{
		m_dwTreeOptions |= OT_OPTIONS_SHOWINFOWINDOW;
	}
	else
	{
		m_dwTreeOptions &= ~OT_OPTIONS_SHOWINFOWINDOW;
	}

	// Hide info window
	if (GetShowInfoWindow() == FALSE)
	{
		if (::IsWindow(m_otInfo.GetSafeHwnd()))
		{
			m_otInfo.ShowWindow(SW_HIDE);
		}
	}
	else
	{
		if (::IsWindow(m_otInfo.GetSafeHwnd()))
		{
			m_otInfo.ShowWindow(SW_SHOW);
		}
	}

	// Resize all windows
	ResizeAllWindows(rcClient.Width(), rcClient.Height());

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}	

BOOL COptionTree::GetShowInfoWindow()
{
	// Return option
	return (m_dwTreeOptions & OT_OPTIONS_SHOWINFOWINDOW) ? TRUE : FALSE;
}

void COptionTree::ResizeAllWindows(int cx, int cy)
{
	// Resize information window
	if ((cx > 0) && (cy > 0))
	{
		// -- Information window
		if (GetShowInfoWindow() == TRUE)
		{
			// -- -- Move information window
			if (::IsWindow(m_otInfo.GetSafeHwnd()))
			{
				m_otInfo.MoveWindow(0, cy - OT_INFOWINDOWHEIGHT, cx, OT_INFOWINDOWHEIGHT);
			}

			// -- -- Move list window
			if (::IsWindow(m_otlList.GetSafeHwnd()))
			{
				m_otlList.MoveWindow(0, 0, cx, cy - OT_INFOWINDOWHEIGHT);
			}
		}
		// -- No information window
		else
		{
			// -- -- Move list window
			if (::IsWindow(m_otlList.GetSafeHwnd()))
			{
				m_otlList.MoveWindow(0, 0, cx, cy);
			}
		}

		// Force redraw
		Invalidate();

		// Update window
		UpdateWindow();
		
	}
}

void COptionTree::OnSize(UINT nType, int cx, int cy) 
{	
	// Resize windows
	ResizeAllWindows(cx, cy);

	CWnd::OnSize(nType, cx, cy);	
}

CFont* COptionTree::GetNormalFont()
{
	return m_fNormalFont;
}


CFont* COptionTree::GetBoldFont()
{
	return m_fBoldFont;
}

void COptionTree::InitGlobalResources()
{
	// Declare variables
	CWindowDC dc(NULL);
	NONCLIENTMETRICS cmInfo;
	cmInfo.cbSize = sizeof(cmInfo);
	LOGFONT lfFont;
//	BOOL bUseSystemFont;

	// Delete global resources
	DeleteGlobalResources();
	
	// Get system parameter information
	::SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(cmInfo), &cmInfo, 0);

	// Initialize logfont
	memzero(&lfFont, sizeof(LOGFONT));
	lfFont.lfCharSet = (BYTE)GetTextCharsetInfo(dc.GetSafeHdc(), NULL, 0);
	lfFont.lfHeight = cmInfo.lfMenuFont.lfHeight;
	lfFont.lfWeight = cmInfo.lfMenuFont.lfWeight;
	lfFont.lfItalic = cmInfo.lfMenuFont.lfItalic;

	// Check if we should use system font
	_tcscpy(lfFont.lfFaceName, cmInfo.lfMenuFont.lfFaceName);

/*	// pooz
	// Use system font
	bUseSystemFont = (cmInfo.lfMenuFont.lfCharSet > SYMBOL_CHARSET);
	if (!bUseSystemFont)
	{
		// -- Check for "Tahoma" font existance:
		if (::EnumFontFamilies(dc.GetSafeHdc(), NULL, _FontFamilyProcFonts, 0) == 0)
		{
			// -- -- Found! Use MS Office font
			_tcscpy(lfFont.lfFaceName, _T("Tahoma"));
		}
		else
		{
			// -- -- Not found. Use default font
			_tcscpy(lfFont.lfFaceName, _T("MS Sans Serif"));
		}
	}
*/
	// Normal font
	m_fNormalFont = new CFont;
	m_fNormalFont->CreateFontIndirect(&lfFont);

	// Bold font
	lfFont.lfWeight = FW_BOLD;
	m_fBoldFont = new CFont;
	m_fBoldFont->CreateFontIndirect(&lfFont);
}

void COptionTree::DeleteGlobalResources()
{
	// Delete normal font
	if (m_fNormalFont->GetSafeHandle() != NULL)
	{
		m_fNormalFont->DeleteObject();

		delete m_fNormalFont;
	}

	// Delete bold font
	if (m_fBoldFont->GetSafeHandle() != NULL)
	{
		m_fBoldFont->DeleteObject();

		delete m_fBoldFont;
	}
}

COptionTreeItem * COptionTree::GetRootItem()
{
	// Return variable
	return &m_otiRoot;
}

COptionTreeItem * COptionTree::GetFocusedItem()
{
	// Return variable
	return m_otiFocus;
}

const POINT& COptionTree::GetOrigin()
{
	// Return variable
	return m_ptOrigin;
}

CWnd* COptionTree::GetCtrlParent()
{
	// Return variable
	return &m_otlList;
}

COptionTreeItem * COptionTree::GetVisibleList()
{
	// Return variable
	return m_otiVisibleList;
}

void COptionTree::AddToVisibleList(COptionTreeItem *otiItem)
{
	// Declare variables
	COptionTreeItem *otiNext;
	
	// Make sure item is not NULL
	if (!otiItem)
	{
		return;
	}

	// Check for an empty visible list
	if (!m_otiVisibleList)
	{
		m_otiVisibleList = otiItem;
	}
	else
	{
		// -- Add the new item to the end of the list
		otiNext = m_otiVisibleList;
		while (otiNext->GetNextVisible())
		{
			otiNext = otiNext->GetNextVisible();
		}
		otiNext->SetNextVisible(otiItem);
	}

	// Set next visible
	otiItem->SetNextVisible(NULL);
}

long COptionTree::HitTest(const POINT &pt)
{
	// Declare variables
	COptionTreeItem* otiItem;
	POINT ptPoint = pt;
	CRect rcLabel;

	// Convert screen to tree coordinates
	ptPoint.y += m_ptOrigin.y;

	// Run the hit test
	if ((otiItem = FindItem(pt)) != NULL)
	{
		// -- Column
		if (!otiItem->IsRootLevel() && pt.x >= m_ptOrigin.x - OT_COLRNG && pt.x <= m_ptOrigin.x + OT_COLRNG)
		{
			return OT_HIT_COLUMN;
		}

		// -- Attribute
		if (pt.x > m_ptOrigin.x + OT_COLRNG)
		{
			return OT_HIT_ATTRIBUTE;
		}

		// -- Expand
		if (otiItem->HitExpand(ptPoint))
		{
			return OT_HIT_EXPAND;
		}

		// -- Label
		return OT_HIT_LABEL;
	}

	// -- Client
	return OT_HIT_CLIENT;
}

COptionTreeItem * COptionTree::FindItem(UINT uCtrlID)
{
	// Mark found as NULL
	m_otiFound = NULL;

	// Enumerate items
	EnumItems(&m_otiRoot, EnumFindItem, uCtrlID);

	return m_otiFound;
}

CRect COptionTree::GetLargestVisibleLabel()
{
	// Set as negative
	m_rcLargestLabel = CRect(0, 0, 0, 0);

	// Enumerate items
	EnumItems(&m_otiRoot, EnumGetLargestVisibleLabelRect, NULL);

	return m_rcLargestLabel;
}


BOOL COptionTree::EnumItems(COptionTreeItem* otiItem, ENUM_OPTIONITEMPROC enumProc, LPARAM lParam)
{
	// Declare variables
	COptionTreeItem* otiNext;
	BOOL bRet = TRUE;

	// Validate items
	if (!otiItem || !enumProc)
	{
		return FALSE;
	}

	// Don't count the root item in any enumerations
	if (otiItem != &m_otiRoot && !enumProc(this, otiItem, lParam))
	{
		return FALSE;
	}

	// Recurse thru all child items
	otiNext = otiItem->GetChild();
	while (otiNext != NULL)
	{
		if (!EnumItems(otiNext, enumProc, lParam))
		{
			bRet = FALSE;
		}

		otiNext = otiNext->GetSibling();
	}

	return bRet;
}

BOOL CALLBACK COptionTree::EnumFindItem(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	NOPRM(otProp);
	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}

	// Find item
	if (otiItem->GetCtrlID() == (UINT) lParam)
	{
		m_otiFound = otiItem;

		return FALSE;
	}

	return TRUE;
}

BOOL CALLBACK COptionTree::EnumGetLargestVisibleLabelRect(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	// Declare variables
	COptionTreeItem *otParent;
	NOPRM(otProp); NOPRM(lParam);

	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}

	// Make sure not root
	if (otiItem->IsRootLevel())
	{
		return TRUE;
	}

	// Get parent
	otParent = otiItem->GetParent();

	// Validate parent
	if (otParent == NULL)
	{
		return TRUE;
	}

	if (otParent->IsExpanded() == FALSE)
	{
		return TRUE;
	}

	// Declare variables
	CRect rcRect;

	// Get lable rect
	rcRect = otiItem->GetLabelRect();

	// See if label right is greater
	if (rcRect.right > m_rcLargestLabel.right)
	{
		m_rcLargestLabel = rcRect;
	}

	return TRUE;
}

BOOL CALLBACK COptionTree::EnumClearAllLabelRect(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	NOPRM(otProp); NOPRM(lParam);
	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}

	// Clear rectangle
	otiItem->SetLabelRect(CRect(0, 0, 0, 0));

	return TRUE;
}

BOOL CALLBACK COptionTree::EnumMoveAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	NOPRM(otProp); NOPRM(lParam);
	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}
	
	// Move item
	otiItem->OnMove();

	return TRUE;
}

BOOL CALLBACK COptionTree::EnumNotifyExpand(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	NOPRM(otProp);
	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}
	
	// Move item
	otiItem->OnExpand((BOOL) lParam);

	return TRUE;
}

BOOL CALLBACK COptionTree::EnumRefreshAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	NOPRM(otProp); NOPRM(lParam);
	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}

	// Refresh item
	otiItem->OnRefresh();

	return TRUE;
}

BOOL CALLBACK COptionTree::EnumExpandAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	NOPRM(otProp); NOPRM(lParam);
	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}

	// Expand item
	otiItem->Expand();

	return TRUE;
}

BOOL CALLBACK COptionTree::EnumSelectAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam)
{
	NOPRM(otProp);
	// Validate items
	if (otiItem == NULL)
	{
		return FALSE;
	}

	// Select item
	otiItem->Select((BOOL)lParam);

	return TRUE;
}

COptionTreeItem* COptionTree::FindItem(const POINT& pt)
{
	// Delcare variables
	COptionTreeItem* otiItem;
	CPoint ptPoint = pt;
	CPoint ptLoc;

	// Convert screen to tree coordinates
	ptPoint.y += m_ptOrigin.y;

	// Search the visible list for the item
	for (otiItem = m_otiVisibleList; otiItem; otiItem = otiItem->GetNextVisible())
	{
		// -- Get item location
		ptLoc = otiItem->GetLocation();
		if (ptPoint.y >= ptLoc.y && ptPoint.y < ptLoc.y + otiItem->GetHeight())
		{
			return otiItem;
		}
	}

	return NULL;
}

void COptionTree::DisableInput(BOOL bDisable)
{
	// Declare variables
	CWnd* pWnd;
	
	// Save variable
	m_bDisableInput = bDisable;
	
	// Get parent window
	pWnd = GetParent();

	// Enable window
	if (pWnd != NULL)
	{
		pWnd->EnableWindow(!bDisable);
	}
}

void COptionTree::SelectItems(COptionTreeItem *otiItem, BOOL bSelect)
{
	// Declare variables
	if (otiItem == NULL)
	{
		otiItem = &m_otiRoot;
	}

	// Enum items
	EnumItems(otiItem, EnumSelectAll, (LPARAM) bSelect);
}

COptionTreeItem * COptionTree::FocusFirst()
{
	// Declare variable
	COptionTreeItem *otiOld;

	// Set old to focus
	otiOld = m_otiFocus;

	// Set focused item
	SetFocusedItem(m_otiVisibleList);

	// Select items
	if (m_otiFocus != NULL)
	{
		SelectItems(NULL, FALSE);
		m_otiFocus->Select();
	}

	// Notify of selection change
	if (otiOld != m_otiFocus)
	{
		SendNotify(OT_NOTIFY_SELCHANGE, m_otiFocus);
	}

	return m_otiFocus;
}

void COptionTree::SetFocusedItem(COptionTreeItem *otiItem)
{
	// Save focused
	m_otiFocus = otiItem;
	
	// Ensure focus is visible
	EnsureVisible(m_otiFocus);

	// Make sure this is a valid window
	if (!IsWindow(GetSafeHwnd()))
	{
		return;
	}

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}

void COptionTree::EnsureVisible(COptionTreeItem *otiItem)
{
	// Declare variables
	COptionTreeItem* otiParent;
	CRect rcClient;
	CPoint ptPoint;
	long lOY;
	
	// Make sure valid
	if (otiItem == NULL)
	{
		return;
	}

	// Item is not scroll visible (expand all parents)
	if (IsItemVisible(otiItem) == FALSE)
	{
		otiParent = otiItem->GetParent();
		while (otiParent != NULL)
		{
			otiParent->Expand();
			
			otiParent = otiParent->GetParent();
		}

		UpdatedItems();
		UpdateWindow();
	}

	// Item should be visible
	if (IsItemVisible(otiItem) == FALSE)
	{
		return;
	}

	// Calculate list client rectangle
	m_otlList.GetClientRect(rcClient);
	rcClient.OffsetRect(0, m_ptOrigin.y);
	rcClient.bottom -= otiItem->GetHeight();

	// Get item location
	ptPoint = otiItem->GetLocation();

	//eklmn: the PtInRect function is not enught in this case
	// cause MSDN says "A point on the right or bottom side is outside CRect"
	// i.e. we gonna have a 0 for a last item, even if our item inside CRect.
	if (!rcClient.PtInRect(ptPoint) && rcClient.bottom != ptPoint.y)
	{
		if (ptPoint.y < rcClient.top)
		{
			lOY = ptPoint.y;
		}
		else
		{
			lOY = ptPoint.y - rcClient.Height() + otiItem->GetHeight();
		}

		m_otlList.OnVScroll(SB_THUMBTRACK, lOY, NULL);
	}
}

LRESULT COptionTree::SendNotify(UINT uNotifyCode, COptionTreeItem* otiItem)
{
	// Make sure this is a valid window
	if (!IsWindow(GetSafeHwnd()))
	{
		return 0L;
	}

	// See if the user wants to be notified
	if (GetNotify() == FALSE)
	{
		return 0L;
	}

	// Make sure we have a window
	if (m_pNotify == NULL)
	{
		return 0L;
	}
	if (!IsWindow(m_pNotify->GetSafeHwnd()))
	{
		return 0L;
	}

	// Declare variables
	NMOPTIONTREE nmmp;
	LPNMHDR lpnm = NULL;
	UINT uID;

	switch (uNotifyCode)
	{
		case OT_NOTIFY_INSERTITEM:
		case OT_NOTIFY_DELETEITEM:
		case OT_NOTIFY_DELETEALLITEMS:
		case OT_NOTIFY_ITEMCHANGED:
		case OT_NOTIFY_ITEMBUTTONCLICK:
		case OT_NOTIFY_SELCHANGE:
		case OT_NOTIFY_ITEMEXPANDING:
		case OT_NOTIFY_COLUMNCLICK:
		case OT_NOTIFY_PROPCLICK:
			lpnm = (LPNMHDR)&nmmp;
			nmmp.otiItem = otiItem;
			break;
	}

	// Send notification
	if (lpnm != NULL)
	{
		uID = (UINT)::GetMenu(GetSafeHwnd());
		lpnm->code = uNotifyCode;
		lpnm->hwndFrom = GetSafeHwnd();
		lpnm->idFrom = uID;
	
		return m_pNotify->SendMessage(WM_NOTIFY, (WPARAM)uID, (LPARAM)lpnm);
	}

	return 0L;
}

void COptionTree::UpdateMoveAllItems()
{
	// Enum move all
	EnumItems(&m_otiRoot, EnumMoveAll);
}

void COptionTree::RefreshItems(COptionTreeItem *otiItem)
{
	// If item is NULL, refresh from root
	if (otiItem == NULL)
	{
		otiItem = &m_otiRoot;
	}

	// Enum refresh all
	EnumItems(otiItem, EnumRefreshAll);

	// Update items
	UpdatedItems();
}

void COptionTree::UpdatedItems()
{
	// Make sure window is valid
	if (!IsWindow(GetSafeHwnd()))
	{
		return;
	}

	// Update list
	m_otlList.UpdateResize();

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}

BOOL COptionTree::IsItemVisible(COptionTreeItem *otiItem)
{
	// Declare varibles
	COptionTreeItem *otiNext = NULL;
	
	// Make sure item is valid
	if (otiItem == NULL)
	{
		return FALSE;
	}

	// Search fr visible
	for (otiNext = m_otiVisibleList; otiNext; otiNext = otiNext->GetNextVisible())
	{
		if (otiNext == otiItem)
		{
			return TRUE;
		}
	}

	return FALSE;
}

void COptionTree::SetOriginOffset(long lOffset)
{
	// Offset
	m_ptOrigin.y = lOffset;
}

void COptionTree::DeleteAllItems()
{
	// Delete from root
	Delete(NULL);

	// Update items
	UpdatedItems();

	// Reset UID counter
	m_uLastUID = 1;
}

void COptionTree::Delete(COptionTreeItem *otiItem)
{
	// Declare variables
	COptionTreeItem* otiIter;
	COptionTreeItem* otiNext;

	// Clear visible list
	ClearVisibleList();

	// Send notify to user
	SendNotify(OT_NOTIFY_DELETEITEM, otiItem);

	// Passing in a NULL deletes frm root
	if (otiItem == NULL)
	{
		otiItem = &m_otiRoot;
	}

	// Delete children
	otiIter = otiItem->GetChild();
	while (otiIter != NULL)
	{
		// -- Get sibling
		otiNext = otiIter->GetSibling();
		
		// -- Delete
		DeleteItem(otiIter);

		// -- Get next
		otiIter = otiNext;
	}

	// Unlink from tree
	if (otiItem->GetParent() != NULL)
	{
		if (otiItem->GetParent()->GetChild() == otiItem)
		{
			otiItem->GetParent()->SetChild(otiItem->GetSibling());
		}
		else
		{
			otiIter = otiItem->GetParent()->GetChild();

			while (otiIter->GetSibling() && otiIter->GetSibling() != otiItem)
			{
				otiIter = otiIter->GetSibling();
			}

			if (otiIter->GetSibling())
			{
				otiIter->SetSibling(otiItem->GetSibling());
			}
		}
	}

	// Delete item
	if (otiItem != &m_otiRoot)
	{
		if (otiItem == GetFocusedItem())
		{
			SetFocusedItem(NULL);
		}

		otiItem->CleanDestroyWindow();

		delete otiItem;
	}
}

void COptionTree::DeleteItem(COptionTreeItem *otiItem)
{
	// Delete iutem
	Delete(otiItem);

	// Update items
	UpdatedItems();
}

long COptionTree::GetColumn()
{
	return m_ptOrigin.x;
}

void COptionTree::SetColumn(long lColumn)
{
	if ( !GetSafeHwnd() )
	{
		return;
	}
	// Delclare variables
	CRect rcClient;

	// Get client rectnagle
	GetClientRect(rcClient);
	
	// Make column
	if (rcClient.IsRectEmpty())
	{
		lColumn = __max(OT_EXPANDCOLUMN, lColumn);
	}
	else
	{
		lColumn = __min(__max(OT_EXPANDCOLUMN, lColumn), rcClient.Width() - OT_EXPANDCOLUMN);
	}

	m_ptOrigin.x = lColumn;

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}

void COptionTree::ClearVisibleList()
{
	// Clear pointer
	m_otiVisibleList = NULL;
}

COptionTreeItem * COptionTree::InsertItem(COptionTreeItem *otiItem, COptionTreeItem *otiParent)
{
	// Declare variables
	COptionTreeItem* otiNext;
	
	// Make sure item is not NULL
	if (otiItem == NULL)
	{
		return NULL;
	}

	// If parent is NULL, becomes root
	if (otiParent == NULL)
	{
		otiParent = &m_otiRoot;
	}

	// Set child
	if (otiParent->GetChild() == NULL)
	{
		otiParent->SetChild(otiItem);
	}
	else
	{
		// -- Add to end of the sibling list	
		otiNext = otiParent->GetChild();
		while (otiNext->GetSibling() != NULL)
		{
			otiNext = otiNext->GetSibling();
		}
		otiNext->SetSibling(otiItem);
	}

	// Auto generate a default ID
	m_uLastUID++;
	otiItem->SetCtrlID(m_uLastUID);

	// Set item information
	otiItem->SetParent(otiParent);
	otiItem->SetOptionsOwner(this);

	// Send notification to user
	SendNotify(OT_NOTIFY_INSERTITEM, otiItem);

	// Updated items
	UpdatedItems();

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();

	return otiItem;
}

BOOL COptionTree::IsDisableInput()
{
	// Return variable
	return m_bDisableInput;
}

COptionTreeItem * COptionTree::FocusLast()
{
	// Declare variables
	COptionTreeItem* otiNext;
	COptionTreeItem* otiChange;

	// Set pointers
	otiChange = m_otiFocus;
	otiNext = m_otiVisibleList;

	// Set focu on last
	if (otiNext != NULL)
	{
		while (otiNext->GetNextVisible())
		{
			otiNext = otiNext->GetNextVisible();
		}
		SetFocusedItem(otiNext);

		if (m_otiFocus != NULL)
		{
			SelectItems(NULL, FALSE);
			m_otiFocus->Select();
		}
	}

	// Send notify to user
	if (otiChange != m_otiFocus)
	{
		SendNotify(OT_NOTIFY_SELCHANGE, m_otiFocus);
	}

	return otiNext;
}

COptionTreeItem * COptionTree::FocusPrev()
{
	// Declare variables
	COptionTreeItem* otiNext;
	COptionTreeItem* otiChange;

	// Set pointers
	otiChange = m_otiFocus;

	// Get the last visible item
	if (m_otiFocus == NULL)
	{
		otiNext = m_otiVisibleList;
		while (otiNext && otiNext->GetNextVisible())
		{
			otiNext = otiNext->GetNextVisible();
		}
	}
	else
	{
		otiNext = m_otiVisibleList;
		while (otiNext && otiNext->GetNextVisible() != m_otiFocus)
		{
			otiNext = otiNext->GetNextVisible();
		}
	}

	// Set focus items
	if (otiNext)
	{
		SetFocusedItem(otiNext);
	}
	
	// Select items
	if (m_otiFocus != NULL)
	{
		SelectItems(NULL, FALSE);
		m_otiFocus->Select();
	}

	// Send notify to user
	if (otiChange != m_otiFocus)
	{
		SendNotify(OT_NOTIFY_SELCHANGE, m_otiFocus);
	}

	return otiNext;
}

COptionTreeItem * COptionTree::FocusNext()
{
	// Declare variables
	COptionTreeItem* otiNext;
	COptionTreeItem* otiChange;

	// Set pointers
	otiChange = m_otiFocus;

	// Get the next item
	if (m_otiFocus == NULL)
	{
		otiNext = m_otiVisibleList;
	}
	else 
	{
		if (m_otiFocus->GetNextVisible())
		{
			otiNext = m_otiFocus->GetNextVisible();
		}
		else
		{
			otiNext = NULL;
		}
	}

	// Set focus item
	if (otiNext)
	{
		SetFocusedItem(otiNext);
	}

	// Select items
	if (m_otiFocus != NULL)
	{
		SelectItems(NULL, FALSE);
		m_otiFocus->Select();
	}
	
	// Send notify to user
	if (otiChange!=m_otiFocus)
	{
		SendNotify(OT_NOTIFY_SELCHANGE, m_otiFocus);
	}

	return otiNext;
}

void COptionTree::OnEnable(BOOL bEnable) 
{	
	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
	
	CWnd::OnEnable(bEnable);	
}

void COptionTree::OnSysColorChange() 
{	
	// Reload global resources
	InitGlobalResources();

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
	
	CWnd::OnSysColorChange();	
}

BOOL COptionTree::IsSingleSelection()
{
	// Only single select
	return TRUE;
}

DWORD COptionTree::GetTreeOptions()
{
	// Return options
	return m_dwTreeOptions;
}

BOOL COptionTree::GetDefInfoTextNoSel()
{
	// Return option
	return (m_dwTreeOptions & OT_OPTIONS_DEFINFOTEXTNOSEL) ? TRUE : FALSE;
}

void COptionTree::SetDefInfoTextNoSel(BOOL bNoSelect)
{
	// Declare variables
	CRect rcClient;

	// Get client rectangle
	GetClientRect(rcClient);

	// Set option
	if (bNoSelect == TRUE)
	{
		m_dwTreeOptions |= OT_OPTIONS_DEFINFOTEXTNOSEL;
	}
	else
	{
		m_dwTreeOptions &= ~OT_OPTIONS_DEFINFOTEXTNOSEL;
	}

	// Resize all windows
	ResizeAllWindows(rcClient.Width(), rcClient.Height());

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}

BOOL COptionTree::GetNotify()
{
	// Return option
	return (m_dwTreeOptions & OT_OPTIONS_NOTIFY) ? TRUE : FALSE;
}

void COptionTree::SetNotify(BOOL bNotify, CWnd *pWnd)
{
	// Declare variables
	CRect rcClient;

	// Get client rectangle
	GetClientRect(rcClient);

	// Get window
	m_pNotify = pWnd;

	// Set option
	if (bNotify == TRUE)
	{
		m_dwTreeOptions |= OT_OPTIONS_NOTIFY;
	}
	else
	{
		m_dwTreeOptions &= ~OT_OPTIONS_NOTIFY;
	}

	// Resize all windows
	ResizeAllWindows(rcClient.Width(), rcClient.Height());

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}

BOOL COptionTree::OnEraseBkgnd(CDC* pDC) 
{
	NOPRM(pDC);
	// Ha, Ha
	return FALSE;
}

void COptionTree::OnPaint() 
{
	// Declare varaibles
	PAINTSTRUCT ps;

	// Begine paint
	BeginPaint(&ps);

	// Redraw list window
	if (::IsWindow(m_otlList.GetSafeHwnd()))
	{
		m_otlList.Invalidate();
		m_otlList.UpdateWindow();
	}

	// Redraw information window
	if (::IsWindow(m_otInfo.GetSafeHwnd()))
	{
		m_otInfo.Invalidate();
		m_otInfo.UpdateWindow();
	}

	// End paint
	EndPaint(&ps);
}

void COptionTree::ClearAllLabelRect()
{
	// Enumerate items
	EnumItems(&m_otiRoot, EnumClearAllLabelRect, NULL);
}


BOOL COptionTree::GetShadeExpandColumn()
{
	// Return option
	return (m_dwTreeOptions & OT_OPTIONS_SHADEEXPANDCOLUMN) ? TRUE : FALSE;
}

void COptionTree::ShadeExpandColumn(BOOL bShade)
{
	// Declare variables
	CRect rcClient;

	// Get client rectangle
	GetClientRect(rcClient);

	// Set option
	if (bShade == TRUE)
	{
		m_dwTreeOptions |= OT_OPTIONS_SHADEEXPANDCOLUMN;
	}
	else
	{
		m_dwTreeOptions &= ~OT_OPTIONS_SHADEEXPANDCOLUMN;
	}

	// Resize all windows
	ResizeAllWindows(rcClient.Width(), rcClient.Height());

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}

BOOL COptionTree::GetShadeRootItems()
{
	// Return option
	return (m_dwTreeOptions & OT_OPTIONS_SHADEROOTITEMS) ? TRUE : FALSE;
}

void COptionTree::ShadeRootItems(BOOL bShade)
{
	// Declare variables
	CRect rcClient;

	// Get client rectangle
	GetClientRect(rcClient);

	// Set option
	if (bShade == TRUE)
	{
		m_dwTreeOptions |= OT_OPTIONS_SHADEROOTITEMS;
	}
	else
	{
		m_dwTreeOptions &= ~OT_OPTIONS_SHADEROOTITEMS;
	}

	// Resize all windows
	ResizeAllWindows(rcClient.Width(), rcClient.Height());

	// Force redraw
	Invalidate();

	// Update window
	UpdateWindow();
}

void COptionTree::ExpandAllItems()
{
	// Declare variables
	COptionTreeItem *otiItem;

	// If item is NULL, refresh from root
	otiItem = &m_otiRoot;

	// Enum refresh all
	EnumItems(otiItem, EnumExpandAll);

	// Update items
	UpdatedItems();
}

void COptionTree::OnSizing(UINT fwSide, LPRECT pRect) 
{
	// Resize windows
	ResizeAllWindows(pRect->right, pRect->bottom);

	CWnd::OnSizing(fwSide, pRect);
}

void COptionTree::Expand(COptionTreeItem *pItem, BOOL bExpand)
{
	// Enum move all
	EnumItems(pItem, EnumNotifyExpand, (LPARAM) bExpand);
}

void COptionTree::OnSetFocus(CWnd* pOldWnd) 
{
	// Invalidate
	Invalidate();

	// Update Window
	UpdateWindow();

	CWnd::OnSetFocus(pOldWnd);	
}

void COptionTree::OnKillFocus(CWnd* pNewWnd) 
{
	CWnd::OnKillFocus(pNewWnd);	
}
