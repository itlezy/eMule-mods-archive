#pragma once

enum EStatusBarPane
{
	SBarLog = 0,
	SBarUsers,
	SBarUpDown,
	SBarConnected,
	SBarChatMsg
};

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#define SB_LOG			0
#define SB_MSG			4
#define SB_UP_SPEED		1
#define SB_DN_SPEED		2
#define SB_SERVER		3
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

class CMuleStatusBarCtrl : public CStatusBarCtrl
{
	DECLARE_DYNAMIC(CMuleStatusBarCtrl)

public:
	CMuleStatusBarCtrl();
	virtual ~CMuleStatusBarCtrl();

	void Init(void);
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	HICON GetTipInfo(CString &info);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

protected:
	int GetPaneAtPosition(CPoint& point) const;

#ifndef NEWTOOLTIPS // NEO: NTT - [NewToolTips] <-- Xanatos --
	CString GetPaneToolTipText(EStatusBarPane iPane) const;
	virtual int OnToolHitTest(CPoint point, TOOLINFO* pTI) const;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags,CPoint point);
};
