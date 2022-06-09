#pragma once

#include "MuleListCtrl.h"
#include "Loggable.h"

class CServerList;
class CServer;

class CServerListCtrl : public CMuleListCtrl, public CLoggable
{
	DECLARE_DYNAMIC(CServerListCtrl)

private:
	bool			m_bSortAscending[SL_COLUMN_NUMCOLUMNS + 2];

public:
					CServerListCtrl();
	virtual		   ~CServerListCtrl();
	bool			Init();
	bool			AddServer(CServer *pServer, bool bAddToList = true, bool bulkLoad = false, bool bChangeServerInfo = false);
	void			RemoveServer(CServer *pServer);
	void			RefreshServer(CServer& server);
	void			RefreshAllServer();
	void			RemoveDeadServer();
	void			SortFirstInit();
	void			SortInit(int sortCode);
	void			Localize();
	void			ShowFilesCount();
	int				FindIndex(const CServer *pServer) const;

//	Purity (moved from private to public because of the web server)
	bool			StaticServerFileAppend(CServer *pServer);
	bool			StaticServerFileRemove(CServer *pServer);

	CServer*		GetServerAt(int iIndex) { return reinterpret_cast<CServer*>(GetItemData(iIndex)); }
	CServer*		GetSelectedServer() { return GetServerAt(GetSelectionMark()); }
	CImageList		m_imageList;

protected:
	afx_msg void OnLvnColumnclickServlist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnNMLdblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDestroy();
	static int CALLBACK SortProc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort);
	virtual BOOL OnCommand(WPARAM wParam,LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG *pMsg);

	DECLARE_MESSAGE_MAP()

private:
	void			AppendServer(CServer *pServer);
	void			RefreshServerDesc(int iIndex, const CServer& server);
};
