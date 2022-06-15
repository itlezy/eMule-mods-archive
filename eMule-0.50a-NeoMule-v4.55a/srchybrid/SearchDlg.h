#pragma once

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "Neo\GUI\ToolTips\PPToolTip.h" 
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

struct SSearchParams;
class CSearchResultsWnd;
class CSearchParamsWnd;
class CSearchFile;
class CClosableTabCtrl;


///////////////////////////////////////////////////////////////////////////////
// CSearchDlg frame

class CSearchDlg : public CFrameWnd
{
	DECLARE_DYNCREATE(CSearchDlg)

public:
	CSearchDlg();           // protected constructor used by dynamic creation
	virtual ~CSearchDlg();
	CSearchResultsWnd* m_pwndResults;

	BOOL Create(CWnd* pParent);

	void Localize();
	void CreateMenus();

	void RemoveResult(const CSearchFile* toremove);

	bool DoNewEd2kSearch(SSearchParams* pParams);
	bool DoNewKadSearch(SSearchParams* pParams);
	void CancelEd2kSearch();
	void CancelKadSearch(UINT uSearchID);
	void SetNextSearchID(uint32 uNextID);

	bool CanSearchRelatedFiles() const;
	//void SearchRelatedFiles(CPtrList& listFiles);
	void SearchRelatedFiles(CPtrList& listFiles, bool bHash = false); // NEO: MOD - [SearchHash] <-- Xanatos --

	void DownloadSelected();
	void DownloadSelected(bool paused);

	bool CanDeleteSearch(uint32 nSearchID) const;
	bool CanDeleteAllSearches() const;
	void DeleteSearch(uint32 nSearchID);
	void DeleteAllSearches();

	void LocalEd2kSearchEnd(UINT count, bool bMoreResultsAvailable);
	void AddGlobalEd2kSearchResults(UINT count);

	bool CreateNewTab(SSearchParams* pParams, bool bActiveIcon = true);
	SSearchParams* GetSearchParamsBySearchID(uint32 nSearchID);
	void ShowSearchSelector(bool visible);
	CClosableTabCtrl& GetSearchSelector();

	int GetSelectedCat();
	void UpdateCatTabs();
	void SaveAllSettings();
	BOOL SaveSearchStrings();
	void ResetHistory();

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	void SetTTDelay();
#else
	void SetToolTipsDelay(UINT uDelay);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
	void DeleteAllSearchListCtrlItems();

	BOOL IsSearchParamsWndVisible() const;
	void OpenParametersWnd();
	void DockParametersWnd();

	void UpdateSearch(CSearchFile* pSearchFile);

protected:
	CSearchParamsWnd* m_pwndParams;

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnClose();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);

private:
	CPPToolTip m_ttip;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --
};
