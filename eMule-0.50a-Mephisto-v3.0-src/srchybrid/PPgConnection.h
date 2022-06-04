#pragma once

#include "NumEdit.h" // Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-

class CPPgConnection : public CPropertyPage
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
	//Xman
	/*
	CSliderCtrl m_ctlMaxDown;
	*/
	//Xman end
	CSliderCtrl m_ctlMaxUp;

	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	CNumEdit m_maxUpload;
	CNumEdit m_maxUploadCapacity; // for Graph, Quick speed selector
	CNumEdit m_maxDownload;
	CNumEdit m_maxDownloadCapacity; // for Graph, Quick speed selector
	// Maella end

	//Xman Xtreme Upload
	void CalculateMaxUpSlotSpeed();

	void ShowLimitValues();
	//Xman
	/*
	void SetRateSliderTicks(CSliderCtrl& rRate);
	*/
	//Xman end

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSettingsChange()					{ SetModified(); }
	afx_msg void OnEnChangeUDPDisable();
	//Xman
	/*
	afx_msg void OnLimiterChange();
	*/
	//Xman end
	afx_msg void OnBnClickedWizard();
	afx_msg void OnBnClickedNetworkKademlia();
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg void OnBnClickedOpenports();
	afx_msg void OnStartPortTest();
	afx_msg void OnEnChangeTCP();
	afx_msg void OnEnChangeUDP();
	afx_msg void OnEnChangePorts(uint8 istcpport);
	//Xman Xtreme Upload
	afx_msg void OnEnKillfocusMaxup();
};
