#pragma once

#include "ListBoxST.h"


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// Slide Bar Notifications
//
// Use WM_SBN_SELCHANGED to know when the selected item changes (i.e. when a user clicks on a
// listbox item or a group).
//
// message map: ON_MESSAGE(WM_SBN_SELCHANGED, memberFxn)
// function prototype: afx_msg LRESULT memberFxn(WPARAM wParam, LPARAM lParam);
//
// wParam will contain the zero-based index of the selected group.
// lParam will be NULL.

#define WM_SBN_SELCHANGED (WM_USER + 0x0666)

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// CSlideBarGroup

class CSlideBarGroup
{
public:
	CSlideBarGroup(CString strName = _T(""), INT iIconIndex = -1, CListBoxST* pListBox = NULL);
	CSlideBarGroup(CSlideBarGroup& Group);
	~CSlideBarGroup();

private:
	CString		m_strName;
	CListBoxST*	m_pListBox;
	int			m_iIconIndex; // icon index for the image list

protected:
	// nothing

public:
	CString		GetName()							{ return m_strName; }
	INT			GetIconIndex()						{ return m_iIconIndex; }
	CListBoxST*	GetListBox()						{ return m_pListBox; }

	void		SetName(const CString &strName)			{ m_strName = strName; }
	void		SetIconIndex(INT iIconIndex)		{ m_iIconIndex = iIconIndex; }
	void		SetListBox(CListBoxST* pListBox)	{ m_pListBox = pListBox; }
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
// CSlideBar

class CSlideBar : public CWnd
{
	DECLARE_DYNAMIC(CSlideBar)

private:
	// nothing

protected:
	CList<CSlideBarGroup, CSlideBarGroup&> m_GroupList;
	CImageList*	m_pImageList;
	CSize		m_IconSize;
	CFont		m_GroupFont;
	CFont		m_ItemFont;
	int			m_iSelectedGroup;
	int			m_iHilightedGroup;
	int			m_iClickedGroup;
	int			m_iGroupHeight;
	int			m_iScrollTickCount;
	DWORD		m_dwHAlignText;
	COLORREF	m_clr3DShadow;
	COLORREF	m_clr3DHilight;
	COLORREF	m_clr3DFace;
	COLORREF	m_clr3DDkShadow;
	COLORREF	m_clr3DLight;

	CRect	GetGroupRect(INT iIndex);
	CRect	GetGroupListBoxRect(INT iIndex);
	int		GetGroupIndexFromPoint(CPoint Point);
	void	DrawNormalGroupButton(INT iIndex);
	void	DrawHilightedGroupButton(INT iIndex);
	void	DrawClickedGroupButton(INT iIndex);
	void	DrawAllGroups();

	afx_msg void OnPaint();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	afx_msg void OnSysColorChange();
	DECLARE_MESSAGE_MAP()

public:
	CSlideBar();
	virtual ~CSlideBar();
	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);
	virtual BOOL CreateEx(DWORD dwExStyle, DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	INT			GetNumberOfGroups(void)			{ return static_cast<INT>(m_GroupList.GetCount()); }
	INT			GetNumberOfGroupItems(void);
	CSlideBarGroup&	GetGroupAt(INT iIndex);
	CListBoxST*	GetGroupListBox(INT iIndex);
	CString		GetGroupName(INT iIndex)		{ return GetGroupAt(iIndex >= 0 ? iIndex : 0).GetName(); }
	INT			GetSelectedGroupIndex()			{ return m_iSelectedGroup; }
	INT			GetGroupHeight()				{ return m_iGroupHeight; }
	CFont*		GetGroupFont()					{ return &m_GroupFont; }
	CFont*		GetItemFont()					{ return &m_ItemFont; }
	INT			GetGlobalSelectedItem();
	INT			GetLocalSelectedItem();
	INT			GetGreaterStringWidth();

	void	SetGroupHeight(INT iGroupHeight)		{ m_iGroupHeight = iGroupHeight; }
	void	SetImageList(CImageList* pImageList)	{ m_pImageList = pImageList; }
	void	SetHAlignCaption(DWORD dwHAlignement)	{ (dwHAlignement != DT_LEFT) && (dwHAlignement != DT_CENTER) && (dwHAlignement !=  DT_RIGHT) ? m_dwHAlignText = DT_LEFT : m_dwHAlignText = dwHAlignement; }
	
	INT		AddGroup(CString strName, INT iIconIndex = -1);
	INT		AddGroupItem(CString strName, INT iGroupIndex, INT iIconIndex = -1L);
	void	SelectGroup(INT iIndex);
	void	SelectGlobalItem(INT iIndex);
	void	ResetContent();
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////