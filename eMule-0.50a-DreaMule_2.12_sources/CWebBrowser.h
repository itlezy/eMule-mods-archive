// webbrowserView.h : interface of the CWebBrowser class
//


#pragma once
#include "afxhtml.h"

class CWebBrowser : public CHtmlView
{
protected: // create from serialization only
	CWebBrowser();
	DECLARE_DYNCREATE(CWebBrowser)


// Operations
public:

// Overrides

// Implementation
public:
	virtual ~CWebBrowser();
	static CWebBrowser* GetHtmlView();

protected:
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);

};


