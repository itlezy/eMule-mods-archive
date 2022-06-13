#pragma once
#include <map>
using namespace std;

class CConnectionWizardDlg : public CDialog
{
	DECLARE_DYNAMIC(CConnectionWizardDlg)
public:
	CConnectionWizardDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CConnectionWizardDlg();

	enum { IDD = IDD_WIZARD };

	void Localize();

protected:
	HICON m_icnWnd;
	CListCtrl m_provider;
	int m_iOS;
	int m_iTotalDownload;

	void SetCustomItemsActivation();

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	typedef struct {
		LPCTSTR name;
		int down, up, maxConnections, maxSourcePerfile, maxUpSlots, maxConperFive;
		float pDown, pUp, pSlotBoost, pExt;
	} ConnectionEntry;

	float pup, pdown;
	BOOL ending;
	int last_selected;

	static ConnectionEntry m_connections[];

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedApply();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedWizHighdownloadRadio();
	afx_msg void OnBnClickedWizLowdownloadRadio();
	afx_msg void OnBnClickedWizMediumdownloadRadio();
	afx_msg void OnBnClickedWizRadioOsNtxp();
	afx_msg void OnBnClickedWizRadioUs98me();
	afx_msg void OnBnClickedWizResetButton();
	afx_msg void OnNmClickProviders(NMHDR *pNMHDR, LRESULT *pResult);
};
