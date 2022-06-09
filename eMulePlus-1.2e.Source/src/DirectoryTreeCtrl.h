#pragma once
/////////////////////////////////////////////
// written by robert rostek - tecxx@rrs.at //
/////////////////////////////////////////////

#define DIRLIST_ITEMSTATECHANGED	161279
#define MP_SHAREDFOLDERS_FIRST	46901

class CDirectoryTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CDirectoryTreeCtrl)

public:
//	Initialize control
	void Init(bool bAllowCDROM = true);

	CStringList m_lstShared;

private:
	bool m_bCtrlPressed;
//	Add a new item
	HTREEITEM AddChildItem(HTREEITEM hRoot, CString strText);
//	Add subdirectory items
	void AddSubdirectories(HTREEITEM hRoot, CString strDir);
//	Return the full path of an item (like C:\abc\somewhere\inheaven\)
	CString GetFullPath(HTREEITEM hItem);
//	Returns true if strDir has at least one subdirectory
	bool HasSubdirectories(CString strDir);
//	Check status of an item has changed
	void CheckChanged(HTREEITEM hItem, bool bChecked);
//	Returns true if a subdirectory of strDir is shared
	bool HasSharedSubdirectory(CString strDir);
//	When sharing a directory, make all parent directories bold
	void UpdateParentItems(HTREEITEM hChild);

//	Share list access
	bool IsShared(CString strDir);
	void AddShare(CString strDir);
	void DelShare(CString strDir);
	void MarkChilds(HTREEITEM hChild,bool mark);

	CString m_strLastRightClicked;
	bool m_bSelectSubDirs;

public:
	CDirectoryTreeCtrl();
	virtual ~CDirectoryTreeCtrl();
	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMRclickSharedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTvnKeydown(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
};
