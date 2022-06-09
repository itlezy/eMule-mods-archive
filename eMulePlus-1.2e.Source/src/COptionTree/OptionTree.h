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


#ifndef OT_OPTIONTREE
#define OT_OPTIONTREE

#pragma once

#include "OptionTreeInfo.h"
#include "OptionTreeList.h"
#include "OptionTreeItem.h"


// Definitions
typedef BOOL (CALLBACK* ENUM_OPTIONITEMPROC)(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
// CPropTree WM_NOTIFY notification structure
typedef struct _NMOPTIONTREE
{
    NMHDR hDR;
	COptionTreeItem* otiItem;
} NMOPTIONTREE, *PNMOPTIONTREE, FAR *LPNMOPTIONTREE;


// Global Functions
// -- Draw dark horizontal line
static void _DrawDarkHLine(HDC hdc, long lX, long lY, long lWidth)
{
	CRect rcPaint(lX, lY, lX + lWidth, lY + 1);
	int nOldBack = SetBkColor(hdc, GetSysColor(COLOR_BTNSHADOW));

	// GDI calls driver directly
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rcPaint, 0, 0, 0); 

	// Restore
	SetBkColor(hdc, nOldBack);	
}
// -- Draw dark vertical line
static void _DrawDarkVLine(HDC hdc, long lX, long lY, long lHeight)
{
	CRect rcPaint(lX, lY, lX + 1, lY + lHeight);
	int nOldBack = SetBkColor(hdc, GetSysColor(COLOR_BTNSHADOW));

	// GDI calls driver directly
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE, rcPaint, 0, 0, 0); 

	// Restore
	SetBkColor(hdc, nOldBack);	
}
// -- Draw lite horizontal line
static void _DrawLiteHLine(HDC hdc, long lX, long lY, long lWidth)
{
	HBRUSH hbr = (HBRUSH )CDC::GetHalftoneBrush()->GetSafeHandle();
	SetBrushOrgEx(hdc, 0, 0, NULL);
	UnrealizeObject(hbr);
	HBRUSH holdbr = (HBRUSH )SelectObject(hdc, hbr);
	COLORREF rcOldColor = SetTextColor(hdc, GetSysColor(COLOR_BTNSHADOW));
	int nOldBack = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	PatBlt(hdc, lX, lY, lWidth, 1, PATCOPY);
	SelectObject(hdc, holdbr);
	SetTextColor(hdc, rcOldColor);
	SetBkColor(hdc, nOldBack);
}
// -- Draw lite vertical line
static void _DrawLiteVLine(HDC hdc, long lX, long lY, long lHeight)
{
	HBRUSH hbr = (HBRUSH )CDC::GetHalftoneBrush()->GetSafeHandle();
	SetBrushOrgEx(hdc, 0, 0, NULL);
	UnrealizeObject(hbr);
	HBRUSH holdbr = (HBRUSH )SelectObject(hdc, hbr);
	COLORREF rcOldColor = SetTextColor(hdc, GetSysColor(COLOR_BTNSHADOW));
	int nOldBack = SetBkColor(hdc, GetSysColor(COLOR_WINDOW));
	PatBlt(hdc, lX, lY, 1, lHeight, PATCOPY);
	SelectObject(hdc, holdbr);
	SetTextColor(hdc, rcOldColor);
	SetBkColor(hdc, nOldBack);
}
// -- Draw selection rectangle
static void _DrawSelectRect(HDC hdc, long lX, long lY, long lWidth)
{
	HBRUSH hbr = (HBRUSH )CDC::GetHalftoneBrush()->GetSafeHandle();
	SetBrushOrgEx(hdc, 0, 0, NULL);
	UnrealizeObject(hbr);
	HBRUSH holdbr = (HBRUSH )SelectObject(hdc, hbr);
	COLORREF rcOldColor = SetTextColor(hdc, GetSysColor(COLOR_3DHILIGHT));
	int nOldBack = SetBkColor(hdc, GetSysColor(COLOR_3DFACE));
	PatBlt(hdc, lX, lY, lWidth, 1, PATCOPY);
	SelectObject(hdc, holdbr);
	SetTextColor(hdc, rcOldColor);
	SetBkColor(hdc, nOldBack);
}
// -- Draw splitter bar selected
static void _DrawXorBar(HDC hdc, int x1, int y1, int nWidth, int nHeight)
{
	// Declare variables
	static WORD _dotPatternBmp[8] = { 0x00aa, 0x0055, 0x00aa, 0x0055, 0x00aa, 0x0055, 0x00aa, 0x0055};
	HBITMAP hbm;
	HBRUSH  hbr, hbrushOld;

	// Create a monochrome checkered pattern
	hbm = CreateBitmap(8, 8, 1, 1, _dotPatternBmp);

	hbr = CreatePatternBrush(hbm);
    
	SetBrushOrgEx(hdc, x1, y1, 0);
	hbrushOld = (HBRUSH)SelectObject(hdc, hbr);
    
	// Draw the checkered rectangle to the screen
	PatBlt(hdc, x1, y1, nWidth, nHeight, PATINVERT);
    
	SelectObject(hdc, hbrushOld);
    
	DeleteObject(hbr);
	DeleteObject(hbm);
}

// Classes
class COptionTree;

/////////////////////////////////////////////////////////////////////////////
// COptionTree window

class COptionTree : public CWnd
{
// Construction
public:
	COptionTree();
	virtual ~COptionTree();
	BOOL Create(DWORD dwStyle, RECT rcRect, CWnd* pParentWnd, DWORD dwTreeOptions, UINT nID);

// Implementation
public:
	void Expand(COptionTreeItem *pItem, BOOL bExpand);
	void ExpandAllItems();
	void ShadeRootItems(BOOL bShade);
	BOOL GetShadeRootItems();
	void ShadeExpandColumn(BOOL bShade);
	BOOL GetShadeExpandColumn();
	void ClearAllLabelRect();
	CRect GetLargestVisibleLabel();
	void SetNotify(BOOL bNotify, CWnd *pWnd);
	BOOL GetNotify();
	void SetDefInfoTextNoSel(BOOL bNoSelect);
	BOOL GetDefInfoTextNoSel();
	DWORD GetTreeOptions();
	BOOL IsSingleSelection();
	COptionTreeItem * FocusNext();
	COptionTreeItem * FocusPrev();
	COptionTreeItem * FocusLast();
	BOOL IsDisableInput();
	COptionTreeItem * InsertItem(COptionTreeItem* otiItem, COptionTreeItem* otiParent = NULL);
	void ClearVisibleList();
	void SetColumn(long lColumn);
	long GetColumn();
	void DeleteItem(COptionTreeItem* otiItem);
	void DeleteAllItems();
	void SetOriginOffset(long lOffset);
	BOOL IsItemVisible(COptionTreeItem* otiItem);
	void UpdatedItems();
	void RefreshItems(COptionTreeItem* otiItem = NULL);
	void UpdateMoveAllItems();
	void EnsureVisible(COptionTreeItem* otiItem);
	void SetFocusedItem(COptionTreeItem* otiItem);
	COptionTreeItem * FocusFirst();
	void SelectItems(COptionTreeItem* otiItem, BOOL bSelect);
	void DisableInput(BOOL bDisable = TRUE);
	COptionTreeItem * FindItem(const POINT& pt);
	COptionTreeItem * FindItem(UINT uCtrlID);
	long HitTest(const POINT& pt);
	void AddToVisibleList(COptionTreeItem* otiItem);
	COptionTreeItem * GetVisibleList();
	COptionTreeItem * GetFocusedItem();
	COptionTreeItem * GetRootItem();
	BOOL GetShowInfoWindow();
	void ShowInfoWindow(BOOL bShow);
	static CFont* GetNormalFont();
	static CFont* GetBoldFont();
	const POINT& GetOrigin();
	CWnd* GetCtrlParent();
	LRESULT SendNotify(UINT uNotifyCode, COptionTreeItem* otiItem = NULL);
	

	// Generated message map functions
protected:
	BOOL EnumItems(COptionTreeItem* otiItem, ENUM_OPTIONITEMPROC enumProc, LPARAM lParam = 0L);
	static BOOL CALLBACK EnumFindItem(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
	static BOOL CALLBACK EnumMoveAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
	static BOOL CALLBACK EnumNotifyExpand(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);	
	static BOOL CALLBACK EnumRefreshAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
	static BOOL CALLBACK EnumExpandAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
	static BOOL CALLBACK EnumSelectAll(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
	static BOOL CALLBACK EnumGetLargestVisibleLabelRect(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
	static BOOL CALLBACK EnumClearAllLabelRect(COptionTree* otProp, COptionTreeItem* otiItem, LPARAM lParam);
	void DeleteGlobalResources();
	void InitGlobalResources();
	void ResizeAllWindows(int cx, int cy);
	void Delete(COptionTreeItem* otiItem);
	//{{AFX_MSG(COptionTree)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnEnable(BOOL bEnable);
	afx_msg void OnSysColorChange();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnSizing(UINT fwSide, LPRECT pRect);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

	// Variables
protected:
	static CFont* m_fNormalFont;
	static CFont* m_fBoldFont;
	static UINT	m_uInstanceCount;
	static COptionTreeItem *m_otiFound;
	DWORD m_dwTreeOptions;
	COptionTreeInfo m_otInfo;
	COptionTreeItem	m_otiRoot;
	COptionTreeItem* m_otiVisibleList;
	COptionTreeItem* m_otiFocus;
	CPoint m_ptOrigin;
	COptionTreeList m_otlList;
	BOOL m_bDisableInput;
	UINT m_uLastUID;
	static CRect m_rcLargestLabel;
	CWnd *m_pNotify;
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !OT_OPTIONTREE


