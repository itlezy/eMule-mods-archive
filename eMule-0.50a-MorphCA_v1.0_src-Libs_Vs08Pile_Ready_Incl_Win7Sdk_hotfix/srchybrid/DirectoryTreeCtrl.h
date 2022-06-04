#pragma once
/////////////////////////////////////////////
// written by robert rostek - tecxx@rrs.at //
/////////////////////////////////////////////

#define MP_SHAREDFOLDERS_FIRST	46901

class CDirectoryTreeCtrl : public CTreeCtrl
{
	DECLARE_DYNAMIC(CDirectoryTreeCtrl)

public:
	// initialize control
	void Init(void);
	// SLUGFILLER START: shareSubdir - two-list versions
	// get all shared directories
/* old code:
	// get all shared directories
	void GetSharedDirectories(CStringList* list);
	// set shared directories
	void SetSharedDirectories(CStringList* list);
  end old code */
	void GetSharedDirectories(CStringList* list, CStringList* listsubdir);
	// set shared directories
	void SetSharedDirectories(CStringList* list, CStringList* listsubdir);
	// SLUGFILLER END: shareSubdir

private:
	CImageList m_image; 
	// add a new item
	HTREEITEM AddChildItem(HTREEITEM hRoot, CString strText);
	// add subdirectory items
	void AddSubdirectories(HTREEITEM hRoot, CString strDir);
	// return the full path of an item (like C:\abc\somewhere\inheaven\)
	CString GetFullPath(HTREEITEM hItem);
	// returns true if strDir has at least one subdirectory
	bool HasSubdirectories(CString strDir);
	// check status of an item has changed
  /* old code
	void CheckChanged(HTREEITEM hItem, bool bChecked);
    end old */
	void CheckChanged(HTREEITEM hItem, bool bChecked, bool bWithSubdir);	// SLUGFILLER: shareSubdir - two-list version
	// returns true if a subdirectory of strDir is shared
	bool HasSharedSubdirectory(CString strDir);
	// when sharing a directory, make all parent directories bold
	void UpdateParentItems(HTREEITEM hChild);
	void ShareSubDirTree(HTREEITEM hItem, BOOL bShare);
	// SLUGFILLER START: shareSubdir - no double-shares
	// when sharing with subdirectories, prevent reshare of subdirectories
	void UnshareChildItems(CString strDir);
	// SLUGFILLER END: shareSubdir

	// share list access
/* sharesubdir start old code
	bool IsShared(CString strDir);
	void AddShare(CString strDir);
  end old code*/
	bool IsShared(CString strDir, bool bCheckParent = false);	// SLUGFILLER: shareSubdir - allow checking indirect share
	void AddShare(CString strDir, bool bWithSubdir);	// SLUGFILLER: shareSubdir - two-list version
	void DelShare(CString strDir);
  /* old code
	void MarkChilds(HTREEITEM hChild,bool mark);
  */
	// SLUGFILLER START: shareSubdir - for removing checkboxes
	bool IsDisabled(HTREEITEM hItem);
	void SetDisable(HTREEITEM hItem, bool bDisable);
	void DisableChilds(HTREEITEM hItem, bool bDisable);
	// SLUGFILLER END: shareSubdir

	CStringList m_lstShared;
	CStringList m_lstSharedSubdir;	// SLUGFILLER: shareSubdir - second list
	CString m_strLastRightClicked;
	bool m_bSelectSubDirs;

public:
	// construction / destruction
	CDirectoryTreeCtrl();
	virtual ~CDirectoryTreeCtrl();
	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam );

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnTvnItemexpanding(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnGetdispinfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnTvnDeleteItem(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	afx_msg LRESULT OnFoundNetworkDrive(WPARAM wParam,LPARAM lParam);	// SLUGFILLER: shareSubdir - Multi-threading
};


// SLUGFILLER START: shareSubdir - multi-threaded magic
class CNetworkEnumThread : public CWinThread
{
		DECLARE_DYNCREATE(CNetworkEnumThread)
protected:
	CNetworkEnumThread()	{}
public:
	virtual	BOOL	InitInstance() {return true;}
	virtual int		Run();
	void	SetTarget(HWND hTarget);
private:
	void EnumNetworkDrives(NETRESOURCE *source);
	HWND	m_hTarget;
}; //ssd
// SLUGFILLER END: shareSubdir
