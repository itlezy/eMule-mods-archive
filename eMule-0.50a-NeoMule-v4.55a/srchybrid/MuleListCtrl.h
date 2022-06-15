#pragma once
#include "Preferences.h"
#include "resource.h"

class CIni;

///////////////////////////////////////////////////////////////////////////////
// CMuleListCtrl

class CMuleListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CMuleListCtrl)

public:
	CMuleListCtrl(PFNLVCOMPARE pfnCompare = SortProc, DWORD dwParamSort = 0);
	virtual ~CMuleListCtrl();

	// Default sort proc, this does nothing
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	// Sets the list name, used for hide/show menu
	void SetName(LPCTSTR lpszName);

	// Save to preferences
	//void SaveSettings(CPreferences::Table tID);
	void SaveSettings();

	// Load from preferences
	//void LoadSettings(CPreferences::Table tID);
	void LoadSettings();

	DWORD SetExtendedStyle(DWORD dwNewStyle);

	// Hide the column
	void HideColumn(int iColumn);

	// Unhide the column
	void ShowColumn(int iColumn);

	// Check to see if the column is hidden
	BOOL IsColumnHidden(int iColumn) const {
		if(iColumn < 1 || iColumn >= m_iColumnsTracked)
			return false;

		return m_aColumns[iColumn].bHidden;
	}

	// Get the correct column width even if column is hidden
	int GetColumnWidth(int iColumn) const {
		if(iColumn < 0 || iColumn >= m_iColumnsTracked)
			return 0;
		
		if(m_aColumns[iColumn].bHidden)
			return m_aColumns[iColumn].iWidth;
		else
			return CListCtrl::GetColumnWidth(iColumn);
	}

	// Call SetRedraw to allow changes to be redrawn or to prevent changes from being redrawn.
	void SetRedraw(BOOL bRedraw = TRUE) {
		if(bRedraw) {
			if(m_iRedrawCount > 0 && --m_iRedrawCount == 0)
				CListCtrl::SetRedraw(TRUE);
		} else {
			if(m_iRedrawCount++ == 0)
				CListCtrl::SetRedraw(FALSE);
		}
	}

	UINT DisableAutoSort(); // NEO: SE - [SortExtension] <-- Xanatos --

	// Sorts the list
	BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD_PTR dwData) {
		// NEO: SE - [SortExtension] -- Xanatos -->
		m_bStopSort = false; 
		BOOL ret = CListCtrl::SortItems(pfnCompare, dwData);
		m_bStopSort = (m_bPermanentSort ? DisableAutoSort() == TRUE : DisableAutoSort() != FALSE);
		return ret;
		// NEO: SE END <-- Xanatos --
	}

	// Sorts the list
	BOOL SortItems(DWORD dwData) { 
		// NEO: SE - [SortExtension] -- Xanatos -->
		m_bStopSort = false; 
		BOOL ret = CListCtrl::SortItems(m_SortProc, dwData);
		m_bStopSort = (m_bPermanentSort ? DisableAutoSort() == TRUE : DisableAutoSort() != FALSE);
		return ret;
		// NEO: SE END <-- Xanatos --
	} 

	// Sets the sorting procedure
	void SetSortProcedure(PFNLVCOMPARE funcSortProcedure) { m_SortProc = funcSortProcedure; }

	// Gets the sorting procedure
	PFNLVCOMPARE GetSortProcedure() { return m_SortProc; }

	// Retrieves the data (lParam) associated with a particular item.
	DWORD_PTR GetItemData(int iItem);

	// Retrieves the number of items in the control.
	int GetItemCount() const { return m_Params.GetCount(); };

	enum ArrowType { arrowDown = IDB_DOWN, arrowUp = IDB_UP,
		arrowDoubleDown = IDB_DOWN2X, arrowDoubleUp = IDB_UP2X };

	int	GetSortType(ArrowType at);
	ArrowType	GetArrowType(int iat);
	// Places a sort arrow in a column
	void SetSortArrow(int iColumn, ArrowType atType);
	void SetSortArrow()		{SetSortArrow(m_bAlt ? m_iCurrentSortItemAlt : m_iCurrentSortItem, m_bAlt ? m_atSortArrowAlt : m_atSortArrow); } // NEO: SEa - [SortAltExtension] <-- Xanatos --

	// Places a sort arrow in a column
	void SetSortArrow(int iColumn, bool bAscending) {
		SetSortArrow(iColumn, bAscending ? arrowUp : arrowDown);
	}
	// NEO: SEa - [SortAltExtension] -- Xanatos -->
	int GetSortItem() const { return m_bAlt ? m_iCurrentSortItemAlt : m_iCurrentSortItem; }
	void SetSortItem(int SortItem) {
		if(m_bAlt)	
			m_iCurrentSortItemAlt = SortItem;
		else
			m_iCurrentSortItem = SortItem;
	}
	bool GetSortAscending() const { return (m_bAlt ? m_atSortArrowAlt : m_atSortArrow) == arrowUp || (m_bAlt ? m_atSortArrowAlt : m_atSortArrow) == arrowDoubleUp; }
	bool GetSortSecondValue() const { return (m_bAlt ? m_atSortArrowAlt : m_atSortArrow) == arrowDoubleDown || (m_bAlt ? m_atSortArrowAlt : m_atSortArrow) == arrowDoubleUp; }
	void AlterSortArrow();
	// NEO: SEa - END <-- Xanatos --

	// NEO: SEa - [SortAltExtension] -- Xanatos -->
	void EnableDual() { m_bDual = true;}
	void SetAlternate(BOOL Alt) {
		ASSERT(m_bDual); 
		if(Alt)
			m_bAlt++;
		else if(m_bAlt > 0)
			m_bAlt--;
		else
			ASSERT(0);
	}
	bool IsAlternate() {return (m_bAlt != FALSE);}
	// NEO: SEa END <-- Xanatos --

	HIMAGELIST ApplyImageList(HIMAGELIST himl);

	// General purpose listview find dialog+functions (optional)
	void	SetGeneralPurposeFind(bool bEnable, bool bCanSearchInAllColumns = true) { m_bGeneralPurposeFind = bEnable; m_bCanSearchInAllColumns = bCanSearchInAllColumns; }
	void	DoFind(int iStartItem, int iDirection /*1=down, 0 = up*/, BOOL bShowError);
	void	DoFindNext(BOOL bShowError);

	void	AutoSelectItem();
	//int		GetNextSortOrder(int dwCurrentSortOrder) const; // NEO: SE - [SortExtension] <-- Xanatos --
	// NEO: SEa - [SortAltExtension] -- Xanatos -->
	void	UpdateSortHistory(int dwNewOrder, int dwInverseValue = 100) {
		if(m_bAlt) 
			UpdateSortHistoryAlt(dwNewOrder,dwInverseValue); 
		else 
			UpdateSortHistoryMain(dwNewOrder,dwInverseValue); 
	}
	void	UpdateSortHistoryMain(int dwNewOrder, int dwInverseValue);
	void	UpdateSortHistoryAlt(int dwNewOrder, int dwInverseValue);
	// NEO: SEa END <-- Xanatos --
	void	PermanentSort(bool bPermanent = true ) {m_bPermanentSort = bPermanent;} // NEO: SE - [SortExtension] <-- Xanatos --

	enum EUpdateMode {
		lazy,
		direct,
		none
	};
	enum EUpdateMode SetUpdateMode(enum EUpdateMode eMode);

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	int GetItemUnderMouse(CPoint* pt = NULL);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void MeasureItem(LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysColorChange();
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct); // NEO: NMX - [NeoMenuXP] <-- Xanatos --

	// Checks the item to see if it is in order
	int          UpdateLocation(int iItem);
	// Moves the item in list and returns the new index
	int          MoveItem(int iOldIndex, int iNewIndex);
	// Update the colors
	void         SetColors(LPCTSTR pszLvKey = NULL);

	CString         m_Name;
	PFNLVCOMPARE    m_SortProc;
	DWORD           m_dwParamSort;
	COLORREF        m_crWindow;
	COLORREF        m_crWindowText;
	COLORREF        m_crWindowTextBk;
	COLORREF        m_crHighlight;
	COLORREF		m_crHighlightText;
	COLORREF		m_crGlow;
	COLORREF        m_crFocusLine;
	COLORREF        m_crNoHighlight;
	COLORREF        m_crNoFocusLine;
	NMLVCUSTOMDRAW  m_lvcd;
	BOOL            m_bCustomDraw;
	CImageList		m_imlHeaderCtrl;
	CList<int, int>	m_liSortHistory;
	CList<int, int>	m_liSortHistoryAlt; // NEO: SEa - [SortAltExtension] <-- Xanatos --
	UINT			m_uIDAccel;
	HACCEL			m_hAccel;
	enum EUpdateMode m_eUpdateMode;
	// NEO: SE - [SortExtension] -- Xanatos -->
	bool			m_bStopSort;
	bool			m_bPermanentSort;
	// NEO: SE END <-- Xanatos --

	// General purpose listview find dialog+functions (optional)
	bool m_bGeneralPurposeFind;
	bool m_bCanSearchInAllColumns;
	CString m_strFindText;
	bool m_bFindMatchCase;
	int m_iFindDirection;
	int m_iFindColumn;
	void OnFindStart();
	void OnFindNext();
	void OnFindPrev();

private:
	static int	IndexToOrder(CHeaderCtrl* pHeader, int iIndex);

	struct MULE_COLUMN {
		int iWidth;
		int iLocation;
		bool bHidden;
	};

	int          m_iColumnsTracked;
	MULE_COLUMN *m_aColumns;

	int GetHiddenColumnCount() const {
		int iHidden = 0;
		for(int i = 0; i < m_iColumnsTracked; i++)
			if(m_aColumns[i].bHidden)
				iHidden++;
		return iHidden;
	}

	int       m_iCurrentSortItem;
	ArrowType m_atSortArrow;
	// NEO: SEa - [SortAltExtension] -- Xanatos -->
	bool	  m_bDual;
	BOOL	  m_bAlt;
	int       m_iCurrentSortItemAlt;
	ArrowType m_atSortArrowAlt;
	// NEO: SEa END <-- Xanatos --

	int m_iRedrawCount;
	CList<DWORD_PTR> m_Params;

	DWORD_PTR GetParamAt(POSITION pos, int iPos) {
		LPARAM lParam = m_Params.GetAt(pos);
		if(lParam == 0xFEEBDEEF) //same as MLC_MAGIC!
			m_Params.SetAt(pos, lParam = CListCtrl::GetItemData(iPos));
		return lParam;
	}

	// NEO: SE - [SortExtension] -- Xanatos -->
	int MultiSortProc(LPARAM lParam1, LPARAM lParam2);

	static int CALLBACK MultiSortCallback(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
		return ((CMuleListCtrl*)lParamSort)->MultiSortProc(lParam1, lParam2);
	}
	// NEO: SE END <-- Xanatos --
};

void GetContextMenuPosition(CListCtrl& lv, CPoint& point);
