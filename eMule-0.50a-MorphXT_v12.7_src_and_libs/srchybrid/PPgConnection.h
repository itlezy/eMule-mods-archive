#pragma once
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped

/* MORPH START leuk_he tooltipped
class CPPgConnection : public CPropertyPage
*/
class CPPgConnection : public CPPgtooltipped  
// MORPH END leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgConnection)

public:
	CPPgConnection();
	virtual ~CPPgConnection();

// Dialog Data
	enum { IDD = IDD_PPG_CONNECTION };

	void Localize(void);
	void LoadSettings(void);

protected:
	bool guardian;
	CSliderCtrl m_ctlMaxDown;
	CSliderCtrl m_ctlMaxUp;

	void ShowLimitValues();
	void SetRateSliderTicks(CSliderCtrl& rRate);

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnEnChangeUDPDisable();
	afx_msg void OnLimiterChange();
	afx_msg void OnBnClickedWizard();
	afx_msg void OnBnClickedNetworkKademlia();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnBnClickedOpenports();
	afx_msg void OnStartPortTest();
	afx_msg void OnEnChangeTCP();
	afx_msg void OnEnChangeUDP();
	afx_msg void OnEnChangePorts(uint8 istcpport);

//MORPH START - Added by SiRoB, [MoNKi: -Random Ports-]
private:
	afx_msg void OnRandomPortsChange();
	CEdit m_minRndPort;
	CEdit m_maxRndPort;
	CSpinButtonCtrl m_minRndPortSpin;
	CSpinButtonCtrl m_maxRndPortSpin;
//MORPH END   - Added by SiRoB, [MoNKi: -Random Ports-]
// MORPH START leuk_he zzratio active warning
	CStatic	m_ZZratioIcon;
// MORPH END

};
