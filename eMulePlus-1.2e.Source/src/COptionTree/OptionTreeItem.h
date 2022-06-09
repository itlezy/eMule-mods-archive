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

#ifndef OT_TREEITEM
#define OT_TREEITEM

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// Classes
class COptionTree;

class COptionTreeItem  
{
public:
	COLORREF GetTextColor();
	void SetTextColor(COLORREF crColor);
	COLORREF GetLabelBackgroundColor();
	void SetLabelBackgroundColor(COLORREF crColor);
	COLORREF GetRootBackgroundColor();
	void SetRootBackgroundColor(COLORREF crColor);
	COLORREF GetLabelTextColor();
	void SetLabelTextColor(COLORREF crColor);
	COLORREF GetBackgroundColor();
	void SetBackgroundColor(COLORREF crColor);
	BOOL GetDrawMultiline();
	void SetDrawMultiline(BOOL bMultiline);
	int GetItemType();
	void SetItemType(int nType);
	void SetLabelRect(CRect rcLabel);
	CRect GetLabelRect();
	virtual void DrawAttribute(CDC *pDC, const RECT &rcRect);
	virtual void OnActivate();
	virtual void OnMove();
	virtual void OnRefresh();
	virtual void OnCommit();
	virtual void CleanDestroyWindow();
	virtual void OnDeSelect();
	virtual void OnSelect();
	virtual void OnExpand(BOOL bExpand);


	void SetItemHeight(long lHeight);
	void CommitChanges();
	void Activate();
	long GetHeight();
	long DrawItem(CDC* pDC, const RECT &rcRect, long x, long y);
	void SetOptionsOwner(COptionTree* otOption);
	long GetTotalHeight();
	BOOL IsRootLevel();
	BOOL HitExpand(const POINT& pt);
	void ReadOnly(BOOL bReadOnly = TRUE);
	void Expand(BOOL bExpand = TRUE);
	void Select(BOOL bSelect = TRUE);
	BOOL IsActivated();
	BOOL IsReadOnly();
	BOOL IsSelected();
	BOOL IsExpanded() const;
	void SetNextVisible(COptionTreeItem *otiNextVisible);
	COptionTreeItem * GetNextVisible();
	void SetChild(COptionTreeItem *otiChild);
	COptionTreeItem * GetChild();
	void SetSibling(COptionTreeItem *otiSibling);
	COptionTreeItem * GetSibling();
	void SetParent(COptionTreeItem *otiParent);
	COptionTreeItem * GetParent();
	UINT GetCtrlID();
	void SetCtrlID(UINT nID);
	CString GetLabelText();
	void SetLabelText(CString strLabel);
	CString GetInfoText();
	void SetInfoText(CString strText);
	const POINT& GetLocation();
	COptionTreeItem();
	virtual ~COptionTreeItem();

protected:
	void _DrawExpand(HDC hdc, long lX, long lY, BOOL bExpand, BOOL bFill);
	BOOL IsStringEmpty(CString strString);

protected:
	CString m_strLabel;
	CString m_strInfoText;
	UINT m_uControlID;
	COptionTreeItem *m_otiParent;
	COptionTreeItem *m_otiSibling;
	COptionTreeItem *m_otiChild;
	COptionTreeItem *m_otiNextVisible;
	CRect m_rcExpand;
	COptionTree *m_otOption;
	CPoint m_ptLocation;
	CRect m_rcAttribute;
	BOOL m_bCommitOnce;
	LPARAM m_lParam;
	long m_lItemHeight;
	CRect m_rcLabelRect;
	BOOL m_bSelected;
	BOOL m_bExpanded;
	BOOL m_bActivated;
	BOOL m_bReadOnly;
	int m_nItemType;
	BOOL m_bDrawMultiline;
	COLORREF m_crBackground;
	COLORREF m_crText;
	COLORREF m_crRootBackground;
	COLORREF m_crLabelText;
	COLORREF m_crLabelBackground;
};

#endif // !OT_TREEITEM
