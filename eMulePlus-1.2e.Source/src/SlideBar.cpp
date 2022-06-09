#include "stdafx.h"
#include "SlideBar.h"


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//	CSlideBar

CSlideBarGroup::CSlideBarGroup(CString strName, INT iIconIndex, CListBoxST* pListBox)
	: m_strName(strName)
	, m_iIconIndex(iIconIndex)
	, m_pListBox(pListBox)
{
}

CSlideBarGroup::CSlideBarGroup(CSlideBarGroup& Group)
{
	CSlideBarGroup(Group.GetName(), Group.GetIconIndex(), Group.GetListBox());
}

CSlideBarGroup::~CSlideBarGroup()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//	CSlideBar

IMPLEMENT_DYNAMIC(CSlideBar, CWnd)
CSlideBar::CSlideBar()
{
	m_pImageList		= NULL;
	m_iSelectedGroup	= -1; // negative if no group is selected, i.e. no group still haven't been added
	m_iHilightedGroup	= -1; // negative if no group is hilighted, so that's what we want here ;)
	m_iClickedGroup		= -1;
	m_iGroupHeight		= 24; // default height
	m_iScrollTickCount	= 10; // speed of the scroll animation
	m_dwHAlignText		= DT_LEFT; // text is left aligned by default
	m_clr3DShadow		= GetSysColor(COLOR_3DSHADOW);
	m_clr3DHilight		= GetSysColor(COLOR_3DHILIGHT);
	m_clr3DFace			= GetSysColor(COLOR_3DFACE);
	m_clr3DDkShadow		= GetSysColor(COLOR_3DDKSHADOW);
	m_clr3DLight		= GetSysColor(COLOR_3DLIGHT);

//	Sets the fonts for group buttons and listboxes
	CFont	*pFont = CFont::FromHandle(static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)));
	ASSERT_VALID(pFont);
	LOGFONT	lf;

	pFont->GetLogFont(&lf);
	m_GroupFont.CreateFontIndirect(&lf);
	m_ItemFont.CreateFontIndirect(&lf);
	
	m_IconSize.SetSize(16, 16); // the height must be less than m_iGroupHeight
}

CSlideBar::~CSlideBar()
{
	ResetContent();
}


BEGIN_MESSAGE_MAP(CSlideBar, CWnd)
	ON_WM_PAINT()
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_WM_ERASEBKGND()
	ON_WM_SYSCOLORCHANGE()
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////////////////////
// Creates the SlideBar.
BOOL CSlideBar::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	return CreateEx(NULL, dwStyle, rect, pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Creates the SlideBar with extended styles.
BOOL CSlideBar::CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT&  rect, CWnd* pParentWnd, UINT nID)
{
	ASSERT((dwStyle & WS_CHILD) != 0);
	ASSERT_VALID(pParentWnd);

	return CWnd::CreateEx(dwExStyle, AfxRegisterWndClass(NULL), NULL, dwStyle, rect, pParentWnd, nID);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the rectangle of the group specified by its zero-based index
CRect CSlideBar::GetGroupRect(INT iIndex)
{
	ASSERT(iIndex >= 0);

	CRect	rcGroupRect, rcClientRect;

	GetClientRect(rcClientRect);

	if (iIndex <= m_iSelectedGroup)
		rcGroupRect.SetRect(rcClientRect.left, rcClientRect.top + iIndex * m_iGroupHeight, rcClientRect.right, rcClientRect.top + (1 + iIndex) * m_iGroupHeight);
	else
		rcGroupRect.SetRect(rcClientRect.left, rcClientRect.bottom - m_iGroupHeight * (GetNumberOfGroups() - iIndex), rcClientRect.right, rcClientRect.bottom - m_iGroupHeight * (GetNumberOfGroups() - iIndex - 1));

	return rcGroupRect;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the rectangle of the list box of the group specified by its zero-based index.
// The group is supposed to be selected (or "expanded")
CRect CSlideBar::GetGroupListBoxRect(INT iIndex)
{
	CRect rcClientRect, rcListBoxRect;

	GetClientRect(rcClientRect);

	rcListBoxRect.SetRect(rcClientRect.left, rcClientRect.top + (iIndex + 1) * m_iGroupHeight, rcClientRect.right, rcClientRect.bottom - m_iGroupHeight * (GetNumberOfGroups() - (iIndex + 1)));

	return rcListBoxRect;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the zero-based index of a group if the point lies within this group rect.
// If the point is outside a group rect, it returns -1.
int CSlideBar::GetGroupIndexFromPoint(CPoint Point)
{
	for (int i = 0; i < (int)GetNumberOfGroups(); i++)
	{
		if (GetGroupRect(i).PtInRect(Point))
			return i;
	}

	return -1;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Draws the group at the specified zero-based index with the normal state.
void CSlideBar::DrawNormalGroupButton(INT iIndex)
{
	ASSERT(iIndex >= 0);

	CClientDC	dc(this);
	CString	strText = GetGroupName(iIndex);
	CRect	rcGroupRect(GetGroupRect(iIndex));
	CRect	rcCaptionRect = rcGroupRect;
//	Draws the button
	CPen	Pen(PS_SOLID, 1, m_clr3DShadow);
	CPen	*pOldPen = dc.SelectObject(&Pen);

	dc.Draw3dRect(rcGroupRect, m_clr3DHilight, m_clr3DShadow);
	rcGroupRect.DeflateRect(1,1);
	dc.Draw3dRect(rcGroupRect, m_clr3DFace, m_clr3DFace);
	rcGroupRect.DeflateRect(1,1);
	dc.FillSolidRect(rcGroupRect, m_clr3DFace);

//	Draws the icon (if an image list is set)
	int	iOffset = static_cast<int>((m_iGroupHeight - m_IconSize.cx) / 2) - 1;

	if ((m_pImageList != NULL) && (GetGroupAt(iIndex).GetIconIndex() >= 0))
	{
		CPoint	point(iOffset, rcCaptionRect.top + iOffset);

		dc.DrawState(point, m_IconSize, m_pImageList->ExtractIcon(GetGroupAt(iIndex).GetIconIndex()), DSS_NORMAL, (CBrush*)NULL);

		rcCaptionRect.left += m_IconSize.cx;
		SetHAlignCaption(DT_LEFT);
	}

//	Draws the caption
	CFont	*pOldFont = dc.SelectObject(&m_GroupFont);

	rcCaptionRect.left  += (iOffset + 3);
	rcCaptionRect.right -= (iOffset + 3);

	dc.SetBkMode(TRANSPARENT);
	dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
	dc.DrawText(strText, rcCaptionRect, m_dwHAlignText | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

//	Restores the initial objects
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldPen);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Draws the group at the specified zero-based index with the highlighted state.
void CSlideBar::DrawHilightedGroupButton(INT iIndex)
{
	ASSERT(iIndex >= 0);

	CClientDC	dc(this);
	CString	strText = GetGroupName(iIndex);
	CRect	rcGroupRect(GetGroupRect(iIndex));
	CRect	rcCaptionRect = rcGroupRect;
//	Draws only the 3D edge of the button
	CPen	Pen(PS_SOLID, 1, m_clr3DShadow);
	CPen	*pOldPen = dc.SelectObject(&Pen);

	dc.Draw3dRect(rcGroupRect, m_clr3DHilight, m_clr3DDkShadow);
	rcGroupRect.DeflateRect(1,1);
	dc.Draw3dRect(rcGroupRect, m_clr3DLight, m_clr3DShadow);

//	Draws the icon (if an image list is set)
	int	iOffset = static_cast<int>((m_iGroupHeight - m_IconSize.cx) / 2) - 1;

	if ((m_pImageList != NULL) && (GetGroupAt(iIndex).GetIconIndex() >= 0))
	{
		rcCaptionRect.left += m_IconSize.cx;
		SetHAlignCaption(DT_LEFT);
	}

	//
	//	Draws the caption
	CFont	*pOldFont = dc.SelectObject(&m_GroupFont);

	rcCaptionRect.left  += (iOffset + 3);
	rcCaptionRect.right -= (iOffset + 3);

	dc.SetBkMode(TRANSPARENT);
	dc.DrawText(strText, rcCaptionRect, m_dwHAlignText | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

//	Restores the initial objects
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldPen);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Draws the group at the specified zero-based index with the clicked state.
void CSlideBar::DrawClickedGroupButton(INT iIndex)
{
	ASSERT(iIndex >= 0);

	CClientDC	dc(this);
	CString	strText = GetGroupName(iIndex);
	CRect	rcGroupRect(GetGroupRect(iIndex));
	CRect	rcCaptionRect = rcGroupRect;
//	Draws the button
	CPen	Pen(PS_SOLID, 1, m_clr3DShadow);
	CPen	*pOldPen = dc.SelectObject(&Pen);

	dc.Draw3dRect(rcGroupRect, m_clr3DDkShadow, m_clr3DHilight);
	rcGroupRect.DeflateRect(1,1);
	dc.FillSolidRect(rcGroupRect, m_clr3DLight);

//	Draws the icon (if an image list is set)
	int	iOffset = static_cast<int>((m_iGroupHeight - m_IconSize.cx) / 2) - 1;

	if ((m_pImageList != NULL) && (GetGroupAt(iIndex).GetIconIndex() >= 0))
	{
		CPoint	point(iOffset + 1, rcCaptionRect.top + iOffset + 1); // +1 to create a push effect

		dc.DrawState(point, m_IconSize, m_pImageList->ExtractIcon(GetGroupAt(iIndex).GetIconIndex()), DSS_NORMAL, (CBrush*)NULL);

		rcCaptionRect.left += m_IconSize.cx;
		SetHAlignCaption(DT_LEFT);
	}

//	Draws the caption
	CFont	*pOldFont = dc.SelectObject(&m_GroupFont);

	rcCaptionRect.left  += (iOffset + 3);
	rcCaptionRect.right -= (iOffset + 3);

	dc.SetBkMode(TRANSPARENT);
	rcCaptionRect.OffsetRect(1,1); // creates a push effect
	dc.DrawText(strText, rcCaptionRect, m_dwHAlignText | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS | DT_NOPREFIX);

//	Restores the initial objects
	dc.SelectObject(pOldFont);
	dc.SelectObject(pOldPen);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Draws all the groups with the normal state.
void CSlideBar::DrawAllGroups()
{
	for (int i = 0; i < GetNumberOfGroups(); i++)
		DrawNormalGroupButton(i);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the total number of items in all listboxes.
INT CSlideBar::GetNumberOfGroupItems(void)
{
	int	iNumberOfItems = 0;
	CListBoxST	*pListBox;

	for (int i = 0; i < GetNumberOfGroups(); i++)
	{
		pListBox = GetGroupListBox(i);
		ASSERT_VALID(pListBox);

		iNumberOfItems += pListBox->GetCount();
	}

	return iNumberOfItems;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the group at the specified zero-based index
__inline CSlideBarGroup& CSlideBar::GetGroupAt(INT iIndex)
{
	POSITION	pos = m_GroupList.FindIndex(iIndex);
	ASSERT(pos != NULL);

	return m_GroupList.GetAt(pos);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the list box of the group at the specified zero-based index.
__inline CListBoxST* CSlideBar::GetGroupListBox(INT iIndex)
{
	CListBoxST	*pListBox = GetGroupAt(iIndex).GetListBox();
	ASSERT_VALID(pListBox);

	return pListBox;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the global zero-based index of the current listbox, as if all listboxes were only
// one listbox.
INT CSlideBar::GetGlobalSelectedItem(void)
{
	int	iSelectedGroup = GetSelectedGroupIndex();
	CListBoxST	*pListBox = GetGroupListBox(iSelectedGroup);
	ASSERT_VALID(pListBox);

	int	iGlobalIndex = pListBox->GetCurSel();

	if (iGlobalIndex >= 0)
	{
		CListBoxST	*pTempListBox;

		for (int i = 0; i < iSelectedGroup; i++)
		{
			pTempListBox = GetGroupListBox(i);
			ASSERT_VALID(pTempListBox);

			iGlobalIndex += pTempListBox->GetCount();
		}
	}

	return iGlobalIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the zero-based index of the current listbox (same as the GetCurSel method of the
// listbox)
INT CSlideBar::GetLocalSelectedItem(void)
{
	CListBoxST	*pListBox = GetGroupListBox(GetSelectedGroupIndex());
	ASSERT_VALID(pListBox);

	return pListBox->GetCurSel();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Returns the biggest width of all strings of the slidebar (group names and item names)
INT CSlideBar::GetGreaterStringWidth(void)
{
	int	iGreaterWidth = 0;
	CClientDC	dc(this);
	CSize	size;
	CString	strText;
	CFont	*pGroupFont = GetGroupFont();
	CFont	*pItemFont = GetItemFont();
	ASSERT_VALID(pGroupFont);
	ASSERT_VALID(pItemFont);

	CFont	*pOldFont = dc.SelectObject(pGroupFont);

	for (int i = 0; i < GetNumberOfGroups(); i++)
	{
	//	First, we search in the group name
		size = dc.GetTextExtent(GetGroupName(i));

		if (size.cx > iGreaterWidth)
			iGreaterWidth = size.cx;

	//	Then, we search in item names of this group
		CListBoxST	*pListBox = GetGroupListBox(i);
		ASSERT_VALID(pListBox);

		dc.SelectObject(pItemFont);

		for (int j = 0; j < pListBox->GetCount(); j++)
		{
			pListBox->GetText(j, strText);
			size = dc.GetTextExtent(strText);

			if (size.cx > iGreaterWidth)
				iGreaterWidth = size.cx;
		}

		dc.SelectObject(pGroupFont);
	}

	dc.SelectObject(pOldFont);

	return iGreaterWidth;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adds a group and returns the zero-based index of this group.
// strName is the name of the group.
// iIconIndex is the zero-based index of the icon in the m_ImageList member.
// It has no icon by default (iIconIndex = -1).
INT CSlideBar::AddGroup(CString strName, INT iIconIndex)
{
	int	iSelectedGroup = GetSelectedGroupIndex();
	CSlideBarGroup	NewGroup(strName, iIconIndex);

	if (m_GroupList.AddTail(NewGroup))
	{
		CListBoxST	*pListBox = new CListBoxST();
		ASSERT_VALID(pListBox);

		iSelectedGroup = GetNumberOfGroups() - 1;
		GetGroupAt(iSelectedGroup).SetListBox(pListBox);
		pListBox->Create(WS_VSCROLL | WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_TABSTOP | LBS_HASSTRINGS | LBS_OWNERDRAWVARIABLE, CRect(0, 0, 0, 0), this, GetDlgCtrlID() + iSelectedGroup + 1);
		pListBox->SetFont(&m_ItemFont);
		ASSERT(this == pListBox->GetOwner());

		pListBox->SetImageList(m_pImageList);

	//	Selects this new added group
		SelectGroup(iSelectedGroup);
	}
	else
		AfxMessageBox(_T("Unable to add the group \'") + strName + _T("\'."));

	return iSelectedGroup;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Adds an item to a group listbox and returns its zero-based index in this listbox.
// strName is the name of the item.
// iGroupIndex is the zero-based of the group which owns the listbox.
// iIconIndex is the zero-based index of the icon in the m_ImageList member.
// It has no icon by default (iIconIndex = -1).
INT CSlideBar::AddGroupItem(CString strName, INT iGroupIndex, INT iIconIndex)
{
//	Verifies if the given group index is not out of range
	ASSERT((iGroupIndex >= 0) && (iGroupIndex < GetNumberOfGroups()));

	CListBoxST	*pListBox = GetGroupListBox(iGroupIndex);

	ASSERT_VALID(pListBox);

	int	iIndex = pListBox->AddString(strName, iIconIndex);

//	If this is the 1st string we add, then we select it.
	if (pListBox->GetCount() == 1)
		pListBox->SetCurSel(0);

	return iIndex;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Selects the group at the specified zero-based index.
void CSlideBar::SelectGroup(INT iIndex)
{
	if (iIndex < GetNumberOfGroups())
	{
		if ((m_iSelectedGroup >= 0) && (m_iSelectedGroup < GetNumberOfGroups())) // if there is already at least 1 group, we hide its listbox
		{
			CListBoxST	*pListBox = GetGroupListBox(m_iSelectedGroup);
			ASSERT_VALID(pListBox);

			pListBox->ModifyStyle(WS_VISIBLE, NULL);
		}

	//	Sets the new index for the selected item
		m_iSelectedGroup = iIndex;

	//	Show the listbox of the new selected group
		CListBoxST	*pListBox = GetGroupListBox(m_iSelectedGroup);
		ASSERT_VALID(pListBox);

		pListBox->MoveWindow(GetGroupListBoxRect(m_iSelectedGroup));
		pListBox->ModifyStyle(NULL, WS_VISIBLE);

	//	Redraws the listbox (to draw properly the scrollbar if there is one)
		CRect	rcListBoxRect;

		pListBox->GetWindowRect(rcListBoxRect);
		ScreenToClient(rcListBoxRect);
		InvalidateRect(rcListBoxRect);

	//	Sends a message to the owner window to tell it the selection has changed
		CWnd* pOwner = GetOwner();
		ASSERT_VALID(pOwner);
		pOwner->PostMessage(WM_SBN_SELCHANGED, static_cast<WPARAM>(m_iSelectedGroup), static_cast<LPARAM>(NULL));
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Selects a listbox item at the specified zero-based index (this 'global' index considers
// all listboxes as if they were only 1 listbox).
void CSlideBar::SelectGlobalItem(INT iIndex)
{
	ASSERT((iIndex >= 0) && (iIndex < GetNumberOfGroupItems()));

	if (GetNumberOfGroups() > 0)
	{
		CListBoxST	*pListBox;
		int	iTemp = 0;
		int	iGroupIndex = 0;

		for (iGroupIndex = 0; iTemp >= 0; iGroupIndex++)
		{
			pListBox = GetGroupListBox(iGroupIndex);
			ASSERT_VALID(pListBox);

			int	iItemCount = pListBox->GetCount();

			iTemp = iIndex - iItemCount;

			if (iTemp >= 0)
				iIndex = iTemp;
		}

		iGroupIndex--;

		pListBox = GetGroupListBox(iGroupIndex);
		ASSERT_VALID(pListBox);

		pListBox->SetCurSel(iIndex);
		SelectGroup(iGroupIndex);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// Removes all groups and listbox items from the slidebar.
void CSlideBar::ResetContent(void)
{
	m_iSelectedGroup	= -1;
//	Deletes all listboxes
	for (int i = 0; i < GetNumberOfGroups(); i++)
	{
		CListBoxST	*pListBox = GetGroupListBox(i);

		pListBox->ResetContent();
		delete pListBox;
	}
//	Remove all groups
	m_GroupList.RemoveAll();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// OnPaint
void CSlideBar::OnPaint()
{
	CPaintDC	dc(this);	// device context for painting

	DrawAllGroups();
}

/////////////////////////////////////////////////////////////////////////////////////////////
// OnMouseMove
void CSlideBar::OnMouseMove(UINT nFlags, CPoint point)
{
	int	iGroupIndex = GetGroupIndexFromPoint(point);

	if (iGroupIndex != m_iHilightedGroup)
	{
		if (iGroupIndex >= 0)
		{
			if (::GetCapture() == NULL)
			{
				SetCapture();
				ASSERT(this == GetCapture());
			}

			if (m_iHilightedGroup >= 0)
				DrawNormalGroupButton(m_iHilightedGroup);

			m_iHilightedGroup = iGroupIndex;

			if (nFlags & MK_LBUTTON)
			{
				if (iGroupIndex == m_iClickedGroup)
					DrawClickedGroupButton(iGroupIndex);
			}
			else
			{
				if (iGroupIndex != m_iSelectedGroup)
					DrawHilightedGroupButton(iGroupIndex);
			}
		}
		else
		{
			if (m_iHilightedGroup >= 0)
			{
				DrawNormalGroupButton(m_iHilightedGroup);
				m_iHilightedGroup = -1;
			}

			if (this == GetCapture())
				ReleaseCapture();
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// OnLButtonUp
void CSlideBar::OnLButtonUp(UINT nFlags, CPoint point)
{
	int	iGroupIndex = GetGroupIndexFromPoint(point);
	NOPRM(nFlags);

	if ((iGroupIndex >= 0) && (iGroupIndex != m_iSelectedGroup) && (iGroupIndex == m_iClickedGroup))
	{
		SelectGroup(iGroupIndex);
		m_iClickedGroup = -1;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// OnLButtonDown
void CSlideBar::OnLButtonDown(UINT nFlags, CPoint point)
{
	int	iGroupIndex = GetGroupIndexFromPoint(point);
	NOPRM(nFlags);

	if ((iGroupIndex >= 0) && (iGroupIndex != m_iSelectedGroup))
	{
		m_iClickedGroup = iGroupIndex;
		DrawClickedGroupButton(iGroupIndex);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////
// OnEraseBkgnd
BOOL CSlideBar::OnEraseBkgnd(CDC* pDC)
{
	if (pDC != NULL)
	{
		CRect	rcClientRect;

		GetClientRect(rcClientRect);

		pDC->FillSolidRect(rcClientRect, RGB(255, 255, 255));

		return TRUE;
	}

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
// OnCmdMsg
BOOL CSlideBar::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	if (nCode == LBN_SELCHANGE)
	{
		CListBoxST	*pListBox = GetGroupListBox(m_iSelectedGroup);
		ASSERT_VALID(pListBox);

		if (pListBox->GetDlgCtrlID() == static_cast<int>(nID))
		{
			CWnd	*pOwner = GetOwner();
			ASSERT_VALID(pOwner);

			return pOwner->PostMessage(WM_SBN_SELCHANGED, static_cast<WPARAM>(m_iSelectedGroup), static_cast<LPARAM>(NULL));
		}
	}

	return CWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}

/////////////////////////////////////////////////////////////////////////////////////////////
// OnSysColorChange
void CSlideBar::OnSysColorChange()
{
	CWnd::OnSysColorChange();

	m_clr3DShadow		= GetSysColor(COLOR_3DSHADOW);
	m_clr3DHilight		= GetSysColor(COLOR_3DHILIGHT);
	m_clr3DFace			= GetSysColor(COLOR_3DFACE);
	m_clr3DDkShadow		= GetSysColor(COLOR_3DDKSHADOW);
	m_clr3DLight		= GetSysColor(COLOR_3DLIGHT);
}

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
