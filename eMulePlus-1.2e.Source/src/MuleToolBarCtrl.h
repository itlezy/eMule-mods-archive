#pragma once

#include "DblScope.h"

#define IDC_TOOLBAR			16127
#define IDC_TOOLBARBUTTON	16129

//#define USE_TEXTURE
#define USE_REBAR

enum EnumMuleToolbarButtons
{
	MTB_CONNECT,
	MTB_SERVERS,
	MTB_TRANSFER,
	MTB_SEARCH,
	MTB_SHAREDFILES,
	MTB_MESSAGES,
	MTB_IRC,
	MTB_STATISTICS,
	MTB_PREFS,

	MTB_NUMBUTTONS
};

class CMuleToolbarCtrl : public CToolBarCtrl
{
	DECLARE_DYNAMIC(CMuleToolbarCtrl)

public:
					CMuleToolbarCtrl();
	virtual		   ~CMuleToolbarCtrl();

	BOOL			Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID);

	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnNMRclick(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTbnQueryDelete(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTbnQueryInsert(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTbnGetButtonInfo(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTbnToolbarChange(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg UINT	OnGetDlgCode() { return DLGC_WANTALLKEYS; }
	afx_msg void	OnSysKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) {NOPRM(nChar); NOPRM(nRepCnt); NOPRM(nFlags);};
	afx_msg void	OnTbnReset(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void	OnTbnInitCustomize(NMHDR *pNMHDR, LRESULT *pResult);

	void			Init(void);
	void			Localize(void);
	void SetSpeedMeterValues(int iValue1, int iValue2)
	{
		m_ctrlSpeedMeter.AddValues(iValue1,iValue2);
	}
	void SetSpeedMeterRange(UINT nMax, UINT nMin)
	{
		m_ctrlSpeedMeter.SetRange(nMin, nMax);
	}
	void GetSpeedMeterRange(UINT& nMax, UINT& nMin)
	{
		m_ctrlSpeedMeter.GetRange(nMax, nMin);
	}
	void ShowSpeedMeter(bool bShow = true);
	void SetBtnWidth();

//	Customization might split up the button-group, so we have to (un-)press them on our own
	void PressMuleButton(int nID)
	{
		if (m_iLastPressedButton != -1)
			CheckButton(m_iLastPressedButton, FALSE);
		CheckButton(nID, TRUE);
		m_iLastPressedButton = nID;
	}
	void		ChangeToolbarBitmap(const CString &strPath, bool bRefresh);
	void		ChangeTextLabelStyle(int settings, bool refresh);
	void		Refresh();

protected:
	CDblScope		m_ctrlSpeedMeter;

	int				m_iLastPressedButton;
	TBBUTTON		m_buttonDefs[MTB_NUMBUTTONS];
	__declspec(align(2)) TCHAR			m_tstrButtonTitles[MTB_NUMBUTTONS][128];
	CStringArray	m_strBitmapPaths;
	int				m_iToolbarLabelSettings;

	virtual BOOL	OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

	bool			m_bUseSpeedMeter;

private:
	uint32	m_dwSeparatorCnt;
	int m_oldcx;
	int m_oldcy;
};
