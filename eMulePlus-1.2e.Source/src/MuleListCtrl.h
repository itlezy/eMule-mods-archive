#pragma once
#include "Preferences.h"
#include "resource.h"

#define MLC_SORTASC		0
#define MLC_SORTDESC	0x80
#define MLC_SORTALT		0x40	// Sort on alternate value
#define MLC_COLUMNMASK	0x3F
#define MLC_DONTSORT	0xFF

// Quite unique size to separate own search requests from list control ones
#define ML_SEARCH_SZ	510

//////////////////////////////////
//	CMuleListCtrl

class CMuleListCtrl : public CListCtrl
{
	DECLARE_DYNAMIC(CMuleListCtrl)

public:
	CMuleListCtrl(PFNLVCOMPARE pfnCompare = SortProc, DWORD dwParamSort = 0);
	virtual ~CMuleListCtrl();

//	Default sort proc, this does nothing
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

//	Save to preferences
	void SaveSettings(CPreferences::EnumTable tID);

//	Load from preferences
	void LoadSettings(CPreferences::EnumTable tID);

//	Hide the column
	void HideColumn(int iColumn);

//	Unhide the column
	void ShowColumn(int iColumn);

//	Check to see if the column is hidden
	bool IsColumnHidden(int iColumn) const
	{
		if (iColumn < 1 || iColumn >= m_iColumnsTracked)
			return false;

		return m_aColumns[iColumn].bHidden;
	}

//	Get the correct column width even if column is hidden
	int GetColumnWidth(int iColumn) const
	{
		if (iColumn < 0 || iColumn >= m_iColumnsTracked)
			return 0;

		if (m_aColumns[iColumn].bHidden)
			return m_aColumns[iColumn].iWidth;
		else
			return CListCtrl::GetColumnWidth(iColumn);
	}

//	Call SetRedraw to allow changes to be redrawn or to prevent changes from being redrawn.
	void SetRedraw(BOOL bRedraw = TRUE)
	{
		bool	bCall;

		EnterCriticalSection(&m_csRedraw);
	//	Don't call SetRedraw under protection, because of possible deadlock
	//	if dispatch thread also calls SetRedraw inside SendMessage processing
		if (bRedraw)
		{
			bCall = ((m_lRedrawCount > 0) && (--m_lRedrawCount == 0));
			LeaveCriticalSection(&m_csRedraw);
			if (bCall)
				CListCtrl::SetRedraw(TRUE);
		}
		else
		{
			bCall = (m_lRedrawCount++ == 0);
			LeaveCriticalSection(&m_csRedraw);
			if (bCall)
				CListCtrl::SetRedraw(FALSE);
		}
	}
//eklmn: items redraw during resorting is disable, since we have a wrapper function
//	protected by critical section it should not give us any problem.

//	Sorts the list
	BOOL SortItems(PFNLVCOMPARE pfnCompare, DWORD_PTR dwData)
	{
		SetRedraw(FALSE);
		BOOL bResult = CListCtrl::SortItems(pfnCompare, dwData);
		SetRedraw(TRUE);
		return bResult;
	}

//	Sorts the list
	BOOL SortItems(DWORD dwData)
	{
		SetRedraw(FALSE);
		BOOL bResult = CListCtrl::SortItems(m_SortProc, dwData);
		SetRedraw(TRUE);
		return bResult;
	}

//	Sets the sorting procedure
	void SetSortProcedure(PFNLVCOMPARE funcSortProcedure)
	{
		m_SortProc = (funcSortProcedure != NULL) ? funcSortProcedure : SortProc;
	}

//	Gets the sorting procedure
	PFNLVCOMPARE GetSortProcedure()		{ return m_SortProc; }

//	Retrieves the data (lParam) associated with a particular item
	DWORD_PTR GetItemData(int iItem);
//	Returns list positions for consecutive GetItemDataByPos()
	POSITION GetItemDataHeadPos() const	{ return m_Params.GetHeadPosition(); }
	POSITION GetItemDataPos(int iItem) const	{ return m_Params.FindIndex(iItem); }
//	Fast item data retrieval (list sorting must be disabled)
	DWORD_PTR GetItemDataByPos(POSITION &pos, int iIdx)
	{
		POSITION	posPrev = pos;
		LPARAM		lParam = m_Params.GetNext(pos);

		if (lParam == 0xFEEBDEEF) //same as MLC_MAGIC!
			m_Params.SetAt(posPrev, lParam = CListCtrl::GetItemData(iIdx));
		return lParam;
	}

//	Retrieves the number of items in the control
	int GetItemCount() const			{ return m_Params.GetCount(); }

	enum ArrowType
	{
		arrowDown = IDB_DOWN, arrowUp = IDB_UP,
		arrowDoubleDown = IDB_DOWN2X, arrowDoubleUp = IDB_UP2X,
		arrowDown1 = IDB_DOWN1, arrowDown2 = IDB_DOWN2, arrowDown3 = IDB_DOWN3,
		arrowUp1 = IDB_UP1, arrowUp2 = IDB_UP2, arrowUp3 = IDB_UP3,
	};

//	Places a sort arrow in a column
	void SetSortArrow(int iColumn, ArrowType atType);
	void SetSortArrow(int iColumn, bool bAscending, int iColumnIndex = 0);

	void ApplyImageList(HIMAGELIST himl);

//	General purpose listview find dialog+functions (optional)
	void SetGeneralPurposeFind(bool bEnable)	{ m_bGeneralPurposeFind = bEnable; }
	void DoFind(int iStartItem, int iDirection /*1=down, 0 = up*/, BOOL bShowError);
	void DoFindNext(BOOL bShowError);
	uint32 GetSortParam() const		{ return m_dwParamSort; }
	void PostUniqueMessage(UINT uiMsg);
	CHeaderCtrl*	GetHeaderCtrl()
	{
		if (m_pHeaderCtrl == NULL)
			m_pHeaderCtrl = CListCtrl::GetHeaderCtrl();

		return m_pHeaderCtrl;
	}

protected:
	virtual void PreSubclassWindow();
	virtual BOOL OnWndMsg(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnChildNotify(UINT message, WPARAM wParam, LPARAM lParam, LRESULT* pResult);

	virtual void OnNMDividerDoubleClick(NMHEADER *pNMHDR);

	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSysColorChange();

	DECLARE_MESSAGE_MAP()

//	Checks the item to see if it is in order
	int UpdateLocation(int iItem);
//	Get location of the item into the sorted list
	int GetNewLocation(const DWORD_PTR dwpItemData, int iItem, bool bInsertion);
//	Moves the item in list and returns the new index
	int MoveItem(int iOldIndex, int iNewIndex);
//	Update the colors
	void SetColors();
	DWORD SetExtendedStyle(DWORD dwNewStyle)
	{
		return CListCtrl::SetExtendedStyle(dwNewStyle | LVS_EX_HEADERDRAGDROP);
	}

	CHeaderCtrl		*m_pHeaderCtrl;
	PFNLVCOMPARE	m_SortProc;
	DWORD			m_dwParamSort;
	COLORREF		m_crWindow;
	COLORREF		m_crWindowText;
	COLORREF		m_crWindowTextBk;
	COLORREF		m_crHighlight;
	COLORREF		m_crGlow;
	COLORREF		m_crGlowText;
	COLORREF		m_crDimmedText;
	COLORREF		m_crFocusLine;
	COLORREF		m_crNoHighlight;
	COLORREF		m_crNoFocusLine;
	NMLVCUSTOMDRAW	m_lvcd;
	bool			m_bCustomDraw;
	CImageList		m_imlHeaderCtrl;

//	General purpose listview find dialog+functions (optional)
	bool m_bGeneralPurposeFind;
	CString m_strFindText;
	int m_iFindDirection;
	int m_iFindColumn;
	void OnFindStart();
	void OnFindNext();
	void OnFindPrev();

private:
	void TextDraw(CDC* pDC, CString &strText, LPRECT lpRect, UINT iFormat, COLORREF crForeground, COLORREF crBackground);

	static int IndexToOrder(CHeaderCtrl* pHeader, int iIndex);

	struct MULE_COLUMN
	{
		int iWidth;
		int iLocation;
		bool bHidden;
	};

	int m_iColumnsTracked;
	MULE_COLUMN *m_aColumns;
	bool m_bMovingItem;

	int GetHiddenColumnCount() const
	{
		int iHidden = 0;

		for (int i = 0; i < m_iColumnsTracked; i++)
			if (m_aColumns[i].bHidden)
				iHidden++;
		return iHidden;
	}

	int			m_iCurrentSortItem[3];
	ArrowType	m_atSortArrow[3];

	long m_lRedrawCount;
	CRITICAL_SECTION m_csRedraw;
	CList<DWORD_PTR> m_Params;

	DWORD_PTR GetParamAt(POSITION pos, int iPos)
	{
		LPARAM lParam = m_Params.GetAt(pos);

		if (lParam == 0xFEEBDEEF) //same as MLC_MAGIC!
			m_Params.SetAt(pos, lParam = CListCtrl::GetItemData(iPos));
		return lParam;
	}
};
