// khaos::categorymod+
#pragma once
//#include "PPGtooltipped.h"

// CSelCategoryDlg dialog

class CSelCategoryDlg : public CPropertyPage
{
	DECLARE_DYNAMIC(CSelCategoryDlg)

public:
	CSelCategoryDlg(CWnd* pWnd = NULL);
	virtual	~CSelCategoryDlg();

	virtual BOOL	OnInitDialog();
	afx_msg void	OnOK();
	afx_msg void	OnCancel();
	
	int			GetInput()		{ return m_Return; }
	bool			CreatedNewCat()	{ return m_bCreatedNew; }
	bool			WasCancelled() { return m_cancel;} //MORPH - Added by SiRoB
// Dialog Data
	enum { IDD = IDD_SELCATEGORY };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
private:
	int	m_Return;
	bool	m_cancel; //MORPH - Added by SiRoB
	bool	m_bCreatedNew;
};
