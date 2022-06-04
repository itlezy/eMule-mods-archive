#pragma once
#include "ResizableLib/ResizableSheet.h"
#include "ListCtrlItemWalk.h"

// CListViewWalkerPropertySheet

class CListViewWalkerPropertySheet : public CResizableSheet
{
	DECLARE_DYNAMIC(CListViewWalkerPropertySheet)

public:
	CListViewWalkerPropertySheet(CListCtrlItemWalk* pListCtrl)
	{
		m_pListCtrl = pListCtrl;
	}
	CListViewWalkerPropertySheet(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	CListViewWalkerPropertySheet(LPCTSTR pszCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CListViewWalkerPropertySheet();

	CPtrArray& GetPages() { return m_pages; }
	const CSimpleArray<void*> &GetItems() const { return m_aItems; }
	void InsertPage(int iIndex, CPropertyPage* pPage);

protected:
	CListCtrlItemWalk* m_pListCtrl;
	CSimpleArray<void*> m_aItems;
	CButton m_ctlPrev;
	CButton m_ctlNext;

	void ChangeData(void* pObj);

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNext();
	afx_msg void OnPrev();
};
