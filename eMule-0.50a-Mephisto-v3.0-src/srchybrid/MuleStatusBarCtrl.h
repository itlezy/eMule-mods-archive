#pragma once

enum EStatusBarPane
{
	SBarLog = 0,
	SBarUsers,
	SBarUpDown,
	SBarConnected,
	SBarChatMsg
};

class CMuleStatusBarCtrl : public CStatusBarCtrl
{
	DECLARE_DYNAMIC(CMuleStatusBarCtrl)

public:
	CMuleStatusBarCtrl();
	virtual ~CMuleStatusBarCtrl();

	void Init(void);

	void UpdateColor(); // Design Settings [eWombat/Stulle] - Max

protected:
	int GetPaneAtPosition(CPoint& point) const;
	CString GetPaneToolTipText(EStatusBarPane iPane) const;

	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags,CPoint point);
	// ==> Enforce Ratio [Stulle] - Stulle
	afx_msg void OnToolTipNotify( UINT id, NMHDR * pNotifyStruct, LRESULT * result );
	// <== Enforce Ratio [Stulle] - Stulle
};
