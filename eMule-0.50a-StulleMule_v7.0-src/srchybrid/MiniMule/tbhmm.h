#pragma once
#include "DblScope.h"
#include "SnapDialog.h"
#include "sysinfo.h"
#include "BtnST.h" // added - Stulle

class CTBHMM : public CSnapDialog
{
	//DECLARE_DYNAMIC(CTBHMM)

public:
	CTBHMM(CWnd* pParent = NULL);   // standard constructor
	virtual ~CTBHMM();
// Dialog Data
	enum { IDD = IDD_MINI_MULE };
	CStatic	m_ctrlMMConnState;
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

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnMenuButtonClicked();
	// ==> added - Stulle
	afx_msg void OnIncomingClicked();
	afx_msg void OnRestoreClicked();
	// <== added - Stulle
	virtual BOOL OnInitDialog();
	CDblScope m_ctrlSpeedMeter;
	DECLARE_MESSAGE_MAP()
	HICON m_hConState[9];
	uint32			m_nLastUpdate;
	CSysInfo sysinfo;
	void DoMenu(CPoint doWhere);
	// ==> added - Stulle
	CButtonST	m_btnIncoming;
	CButtonST	m_btnRestore;
	// <== added - Stulle

	// [TPT] - Improved minimule
	static UINT run(LPVOID p);
	void run();
	volatile BOOL running;
	// [TPT] - Improved minimule
	bool reset;
	void MMUpdate(void);

public:
	void RunMiniMule(bool resetMiniMule = false);
	int smmin;
	int smmax;
};
