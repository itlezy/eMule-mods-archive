#pragma once
#include "ResizableLib\ResizableDialog.h"
#include "WebTool.h"
#include "browsertoolbarctrl.h"
#include "comboboxenter.h"
#include "afxcmn.h"
#include "afxwin.h"


typedef enum  // ����������ͬҳ��״̬
{
	EB_PT_LOADER, // ����ҳ
	EB_PT_PAGE    // ��ͨҳ��
}EM_BROWSER_PAGETYPE;

// CWebBrowserWnd �Ի���

class CWebBrowserWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CWebBrowserWnd)

public:
	CWebBrowserWnd(CWnd* pParent = NULL);   // ��׼���캯��
	virtual ~CWebBrowserWnd();
	afx_msg void OnHomePage();
// �Ի�������
	enum { IDD = IDD_WEBBROWSER };
public:
	HICON          m_browsericon;
protected:
	CHtmlCtrl*			m_pExplorer; //Added by thilon 2006.10.12
	EM_BROWSER_PAGETYPE m_pagetype;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNewAddress();
	afx_msg void OnNewAddressEnter();

	afx_msg	void OnBack();
	afx_msg void OnForward();
	afx_msg void OnStop();
	//afx_msg void OnHomePage();
	afx_msg void OnRefresh();

public:
	virtual BOOL OnInitDialog();
	CBrowserToolbarCtrl m_toolbar;
	void Localize(void);
	CComboBoxEnter m_addressbox;
	CAnimateCtrl m_animate;
	CStatic m_status;
	CStatic m_staticError; // VC-SearchDream[2006-12-26]: Added for Runtime Error 
	UINT	m_uStridDisableReason;

	BOOL	IsBrowserCanUse(){return NULL != m_pExplorer;}
	void	DisableBrowser(UINT uStridReason);
public:
	EM_BROWSER_PAGETYPE GetPageType(void) { return m_pagetype;}

	void SetAddress(LPCTSTR lpszURL);
	void StartAnimation();
	void StopAnimation();
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	CString ResourceToURL(LPCTSTR lpszURL);
	//Chocobo Start
	//���������ָ��ҳ�棬added by Chocobo on 2006.08.07
	//��������������������������ʾ
	void Navigate(LPCTSTR lpszURL);
	//Chocobo End
	afx_msg void OnSize(UINT nType, int cx, int cy);
};