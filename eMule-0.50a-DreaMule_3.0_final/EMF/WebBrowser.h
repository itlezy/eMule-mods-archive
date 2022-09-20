#pragma once
#include "ResizableLib\ResizableDialog.h"
#include "WebTool.h"
#include "browsertoolbarctrl.h"
#include "comboboxenter.h"
#include "afxcmn.h"
#include "afxwin.h"


typedef enum  // 浏览器浏览不同页面状态
{
	EB_PT_LOADER, // 启动页
	EB_PT_PAGE    // 普通页面
}EM_BROWSER_PAGETYPE;

// CWebBrowserWnd 对话框

class CWebBrowserWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CWebBrowserWnd)

public:
	CWebBrowserWnd(CWnd* pParent = NULL);   // 标准构造函数
	virtual ~CWebBrowserWnd();
	afx_msg void OnHomePage();
// 对话框数据
	enum { IDD = IDD_WEBBROWSER };
public:
	HICON          m_browsericon;
protected:
	CHtmlCtrl*			m_pExplorer; //Added by thilon 2006.10.12
	EM_BROWSER_PAGETYPE m_pagetype;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

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
	//浏览器访问指定页面，added by Chocobo on 2006.08.07
	//方便帮助链接在内置浏览器中显示
	void Navigate(LPCTSTR lpszURL);
	//Chocobo End
	afx_msg void OnSize(UINT nType, int cx, int cy);
};