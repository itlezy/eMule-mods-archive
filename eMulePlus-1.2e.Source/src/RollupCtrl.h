//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// CRollupCtrl & CRollupHeader
// (c) 2002 by FoRcHa (a.k.a. NO)  [seppforcher38@hotmail.com]
//
// I would appreciate a notification of any bugs or bug fixes to help the control grow.
///////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#define USRMSG_STATECHANGED		WM_USER + 1
#define USRMSG_RIGHTCLICK		WM_USER + 3

#define RUP_HEADERHEIGHT	22
#define RUP_BORDERSIZES		 7
#define RUP_ENTRYMINHEIGHT	40

class CRollupGripper;

/////////////////////////////////////////////////////////////////////////////
// CRollupHeader window
class CRollupHeader : public CWnd
{
public:
	CRollupHeader();
	virtual ~CRollupHeader();

	bool IsExpanded() const
	{
		return m_bExpanded;
	}
	void Expand(bool bExpand = false, bool bInvalidate = true)
	{
		m_bExpanded = bExpand;
		if(m_hWnd && bInvalidate)
			Invalidate();
	}
	void SetLeftText(const CString &strLeftText)
	{
		m_strLeftText = strLeftText;
		if(m_hWnd)
			Invalidate();
	}
	void SetRightText(const CString &strRightText)
	{
		m_strRightText = strRightText;
		if(m_hWnd)
			Invalidate();
	}
	void SetHeight(int iHeight)
	{
		m_iHeight = iHeight;
	}
	void SetBackColor(COLORREF crBackColor)
	{
		m_crBackColor = crBackColor;
	}
	void SetTextColor(COLORREF crTextColor)
	{
		m_crTextColor = crTextColor;
	}
	void SetBorderColor(COLORREF crBorderColor)
	{
		m_crBorderColor = crBorderColor;
	}

protected:
	COLORREF m_crBackColor;
	COLORREF m_crTextColor;
	COLORREF m_crBorderColor;

	CFont	m_cfTextFont;
	CPen	m_cpBorderPen;
	CPen	m_cpArrowPen;

	CString	m_strLeftText;
	CString	m_strRightText;

	CDC		m_MemDC;
	CBitmap	m_MemBMP;
	CBitmap	*m_pOldMemBMP;

	CRect	m_rClientRect;
	CRect	m_rTextRect;
	CRect	m_rArrowRect;
	CPoint	m_cpArrowPoint;

	int		m_iHeight;
	bool	m_bInit;
	bool	m_bExpanded;

	void CreateMemDC(CDC *pDC, const CRect *pRect)
	{
		if(m_pOldMemBMP && m_MemDC.m_hDC)
		{
			m_MemDC.SelectObject(m_pOldMemBMP);
			m_pOldMemBMP = NULL;
		}
		if(m_MemDC.m_hDC)
			m_MemDC.DeleteDC();
		m_MemDC.CreateCompatibleDC(pDC);
		if(m_MemBMP.m_hObject)
			m_MemBMP.DeleteObject();
		m_MemBMP.CreateCompatibleBitmap(pDC, pRect->Width(), pRect->Height());
		m_pOldMemBMP = m_MemDC.SelectObject(&m_MemBMP);
	}

	void DrawArrow(CDC* pDC, const CPoint* pTopLeft, bool bDown);

	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSysColorChange();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()
};

typedef struct
{
	double			adSizes[8];
	CRollupHeader	*pHeader;
	CRollupGripper	*pGripper;
	CWnd			*pClient;
	int				iMinHeight;
} RollupEntry;

/////////////////////////////////////////////////////////////////////////////
// CRollupCtrl window
class CRollupCtrl : public CWnd
{
public:
	CRollupCtrl();
	virtual ~CRollupCtrl();

	int InsertItem(LPCTSTR strLeft, LPCTSTR strRight, CWnd *pClient,
					int iIndex = -1, bool bExpanded = FALSE);

	int SetItemHeights(int iItem, double *pHeights, unsigned uiSize);
	int SetText(int iItem, const CString &strText, bool bLeft = FALSE);
	int SetHeaderColor(int iItem, int iColor, COLORREF crColor);
	int SetItemClient(int iItem, CWnd *pClient);
	int ExpandItem(int iItem, bool bExpand = TRUE);

	int Recalc(HWND hWnd);
	CWnd* GetItemClient(int iItem);
	RollupEntry* GetItem(int iItem);
	unsigned GetCount() const { return m_List.GetCount(); }

protected:
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void ChildStateChanged(HWND hChild, bool bInvalidate = true);

	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

	DECLARE_MESSAGE_MAP()

protected:
	CArray<RollupEntry*, RollupEntry*> m_List;
	int m_iExpandedItems;
	int m_iHeaderHeight;
	int m_iExpandedMsk;	// max. 8 items
};
