#pragma once

enum EnumStatusBarBoxes
{
	SB_MESSAGETEXT = 0,
	SB_SESSIONTIME,
	SB_MESSAGESTATUS,
	SB_NUMUSERS,
	SB_UPLOADRATE,
	SB_DOWNLOADRATE,
	SB_SERVER,

	SB_NUMSBPARTS
};

class CMuleStatusBarCtrl : public CStatusBarCtrl
{
	DECLARE_DYNAMIC(CMuleStatusBarCtrl)

public:
	CMuleStatusBarCtrl();
	virtual ~CMuleStatusBarCtrl();

	int GetPaneAtPosition(CPoint& point);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
};