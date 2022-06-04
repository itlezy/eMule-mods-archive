#pragma once

enum EStatusBarPane
{
	SBarLog = 0,
	SBarKadCount, //KadCount
	SBarUpDown,
	SBarConnected,
        SBarGripper
};

class CMuleStatusBarCtrl : public CStatusBarCtrl
{
	DECLARE_DYNAMIC(CMuleStatusBarCtrl)

public:
	CMuleStatusBarCtrl();
	virtual ~CMuleStatusBarCtrl();


protected:
	int GetPaneAtPosition(CPoint& point) const;
	CString GetPaneToolTipText(EStatusBarPane iPane) const; //status tooltip

	virtual INT_PTR OnToolHitTest(CPoint point, TOOLINFO* pTI) const; //status tooltip

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags,CPoint point);
	afx_msg void OnToolTipNotify( UINT id, NMHDR * pNotifyStruct, LRESULT * result ); //status tooltip
};
