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
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	void	RefreshAllServer();
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
	void	RemoveAllDeadServers();
	void	RemoveAllFilteredServers();
	void	Hide()		{ ShowWindow(SW_HIDE); }
	void	Visable()	{ ShowWindow(SW_SHOW); }
	void	Localize();
	void	ShowServerCount();
	bool	StaticServerFileAppend(CServer* pServer);
	bool	StaticServerFileRemove(CServer* pServer);

protected:
#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	CToolTipCtrlX*	m_tooltip;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	CString CreateSelectedServersURLs();
	void DeleteSelectedServers();

	void SetSelectedServersPriority(UINT uPriority);
	void SetAllIcons();
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	virtual void DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct);
	CImageList imagelist;
	CString	GetItemTextEx(int nItem, int nSubItem); // NEO: LF - [ListFind]
	void DrawServerItem(CDC *dc, int iColumn, LPRECT cur_rec, const CServer* server);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

	DECLARE_MESSAGE_MAP()
	afx_msg void OnSysColorChange();
	afx_msg	void OnColumnClick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMDblClk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnGetInfoTip(NMHDR *pNMHDR, LRESULT *pResult);
#ifndef IP2COUNTRY // NEO: IP2C -- Xanatos -->
	afx_msg void OnNMCustomDraw(NMHDR *pNMHDR, LRESULT *pResult);
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
};
