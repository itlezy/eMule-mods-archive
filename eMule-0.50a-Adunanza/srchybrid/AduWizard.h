#pragma once
//Anis -> Classe totalmente riscritta da zero.

// finestra di dialogo AduWizard

class AduWizard : public CDialog
{
	DECLARE_DYNAMIC(AduWizard)

public:
	AduWizard(CWnd* pParent = NULL);   // costruttore standard
	virtual ~AduWizard();
	typedef struct {
		LPCTSTR name;
		int down, up;
	} ConnectionEntry;

// Dati della finestra di dialogo
	enum { IDD = IDD_WIZARD };

protected:
	int last_selected;
	virtual void DoDataExchange(CDataExchange* pDX);
	BOOL PreTranslateMessage(MSG* pMsg);
	void SetCustomItemsActivation();
	CListCtrl m_provider;
	HICON m_icnWnd;
	static ConnectionEntry m_connections[];
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedWizApplyButton();
	afx_msg void OnNMClickProviders(NMHDR *pNMHDR, LRESULT *pResult);
};
