#pragma once
#include "MuleListCtrl.h"

class CServerList; 
class CServer;
class CToolTipCtrlX;

class CServerListCtrl : public CMuleListCtrl 
{
	DECLARE_DYNAMIC(CServerListCtrl)
public:
	CServerListCtrl();
	virtual ~CServerListCtrl();

	bool	Init();
	bool	AddServer(const CServer* pServer, bool bAddToList = true, bool bRandom = false);
	void	RemoveServer(const CServer* pServer);
	bool	AddServerMetToList(const CString& strFile);
	void	RefreshServer(const CServer* pServer);
	void	RefreshAllServer(); // ZZUL-TRA :: IP2Country
	void	RemoveAllDeadServers();
	void	RemoveAllFilteredServers();
	void	Hide()		{ ShowWindow(SW_HIDE); }
	void	Visable()	{ ShowWindow(SW_SHOW); }
	void	Localize();
	void	ShowServerCount();
	bool	StaticServerFileAppend(CServer* pServer);
	bool	StaticServerFileRemove(CServer* pServer);

// ZZUL-TRA :: ServerAutoDisconnect :: Start
    UINT_PTR m_hTimer2; 
	static void CALLBACK ServerAutoTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);
// ZZUL-TRA :: ServerAutoDisconnect :: End

protected:
	CToolTipCtrlX*	m_tooltip;

	CString CreateSelectedServersURLs();
	void DeleteSelectedServers();

	void SetSelectedServersPriority(UINT uPriority);
	void SetAllIcons();
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	// ZZUL-TRA :: IP2Country :: Start
	void	GetItemDisplayText(const CServer* server, int iSubItem, LPTSTR pszText, int cchTextMax) const;
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	CImageList imagelist;
	// ZZUL-TRA :: IP2Country :: End

	DECLARE_MESSAGE_MAP()
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnLvnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetDispInfo(NMHDR *pNMHDR, LRESULT *pResult); // ZZUL-TRA :: IP2Country
	afx_msg void OnNmCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNmDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnSysColorChange();
};
