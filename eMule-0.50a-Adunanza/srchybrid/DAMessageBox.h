#pragma once

#include "enbitmap.h"

class CDAMessageBox : public CDialog
{
	DECLARE_DYNAMIC(CDAMessageBox)

public:
	CDAMessageBox(CWnd* pParent = NULL, LPCTSTR cap = _T("..."), BOOL da = true, BOOL link = false);
	virtual ~CDAMessageBox();

	void Localize();

	enum { IDD = IDS_SPAM };

protected:
	HICON m_icnWnd;
	LPCTSTR caption;
	CBitmap m_iconIn;
	BOOL enableda;
	BOOL enablelink;

	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);

	afx_msg void OnBnClickedFatto();

	DECLARE_MESSAGE_MAP()
	void OnPaint(); 
public:
	afx_msg void OnStnClickedDaTesto();
	afx_msg void OnBnClickedButton1();
	afx_msg void OnTtnGetDispInfoCustom1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTtnTooltipLinkClickCustom1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedDaCheck();
};
