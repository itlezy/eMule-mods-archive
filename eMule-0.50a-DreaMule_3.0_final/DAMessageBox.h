#pragma once

#include "enbitmap.h"

using namespace std;
// CDAMessageBox dialog

class CDAMessageBox : public CDialog
{
	DECLARE_DYNAMIC(CDAMessageBox)

public:
	CDAMessageBox(CWnd* pParent = NULL, LPCTSTR cap = _T("..."), BOOL da = true);   // standard constructor
	virtual ~CDAMessageBox();

	void Localize();

// Dialog Data
	enum { IDD = IDD_DA_MESSAGEBOX };

protected:
	HICON m_icnWnd;
	LPCTSTR caption;
//	CBitmap m_iconIn;
	BOOL enableda;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	afx_msg void OnBnClickedFatto();

	DECLARE_MESSAGE_MAP()
	void OnPaint(); 
};
