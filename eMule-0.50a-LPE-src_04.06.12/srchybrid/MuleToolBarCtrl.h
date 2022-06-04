#pragma once
#include "ToolBarCtrlX.h"

#define IDC_TOOLBAR			16127
#define IDC_TOOLBARBUTTON	16129

#define	TBBTN_SERVER	(IDC_TOOLBARBUTTON + 0)
#define	TBBTN_TRANSFERS	(IDC_TOOLBARBUTTON + 1)
#define	TBBTN_SEARCH	(IDC_TOOLBARBUTTON + 2)
#define	TBBTN_STATS		(IDC_TOOLBARBUTTON + 3)
#define	TBBTN_OPTIONS	(IDC_TOOLBARBUTTON + 4)

class CMuleToolbarCtrl : public CToolBarCtrlX
{
	DECLARE_DYNAMIC(CMuleToolbarCtrl)

public:
	CMuleToolbarCtrl();
	virtual ~CMuleToolbarCtrl();

	void Init();
	void Localize();
	void PressMuleButton(int nID);
	BOOL GetMaxSize(LPSIZE pSize) const;

protected:
	int			m_iLastPressedButton;
	int			m_buttoncount;

	void ChangeToolbarBitmap(bool bRefresh);
	void SetAllButtonsStrings();
#ifdef _DEBUG
	void Dump();
#endif

	DECLARE_MESSAGE_MAP()
	//afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	//afx_msg void OnSysColorChange();
};
