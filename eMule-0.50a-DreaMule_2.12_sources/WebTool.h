#pragma once


// CWebTool dialog
#include "xSkinButton.h"
class CWebTool : public CDialog
{
	DECLARE_DYNAMIC(CWebTool)

public:
	CWebTool(CWnd* pParent = NULL);   // standard constructor
	virtual ~CWebTool();
	CxSkinButton	m_ButtonBack;
	CxSkinButton	m_ButtonNext;
	CxSkinButton	m_ButtonStop;
	CxSkinButton	m_ButtonRefresh;	
	CxSkinButton	m_ButtonGoHome;

// Dialog Data
	enum { IDD = IDD_WEBTOOL };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support


	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedBack();
	afx_msg void OnBnClickedNext();
	afx_msg void OnBnClickedStop();
	afx_msg void OnBnClickedRefresh();
	afx_msg void OnBnClickedGoHome();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
};
