#pragma once

enum EStatusBarPane
{
	// X-Ray :: Statusbar :: Start
	/*
	SBarLog = 0,
	SBarUsers,
	SBarUpDown,
	SBarConnected,
	SBarChatMsg
	*/
	SBarLog = 0,
	SBarSysInfo,
	SBarUsers,
	SBarFiles,
	SBarUp,
	SBarDown,
	SBarServer,
	SBarKad,
	SBarMessage,
	SBarPing
	// X-Ray :: Statusbar :: End
};

class CMuleStatusBarCtrl : public CStatusBarCtrl
{
	DECLARE_DYNAMIC(CMuleStatusBarCtrl)

public:
	CMuleStatusBarCtrl();
	virtual ~CMuleStatusBarCtrl();

	void Init(void);

protected:
	int GetPaneAtPosition(CPoint& point) const;
	CString GetPaneToolTipText(EStatusBarPane iPane) const;

	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags,CPoint point);
};
