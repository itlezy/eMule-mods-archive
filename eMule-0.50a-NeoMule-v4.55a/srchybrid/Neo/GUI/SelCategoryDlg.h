
#pragma once

// NEO: NXC - [NewExtendedCategories] -- Xanatos -->
// CSelCategoryDlg dialog

class CSelCategoryDlg : public CDialog
{
	DECLARE_DYNAMIC(CSelCategoryDlg)

public:
	CSelCategoryDlg(CWnd* pWnd = NULL);
	virtual	~CSelCategoryDlg();

	virtual BOOL	OnInitDialog();
	afx_msg void	OnOK();
	afx_msg void	OnCancel();
	
	int				GetInput()		{ return m_cancel ? -1 : m_Return; }
	bool			CreatedNewCat()	{ return m_bCreatedNew; }
	bool			WasCancelled()	{ return m_cancel;} 
// Dialog Data
	enum { IDD = IDD_SELCATEGORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
private:
	int		m_Return;
	bool	m_cancel; 
	bool	m_bCreatedNew;
};

// NEO: NXC END <-- Xanatos --