#pragma once

#include "..\Resource.h"
#include "ScrollStatic.h"

// CCDTransfer dialog

class CUpDownClient;
class CCDTransfer : public CPropertyPage
{
	DECLARE_DYNCREATE(CCDTransfer)

// Construction
public:
	CCDTransfer();
	~CCDTransfer();

// Dialog Data
	//{{AFX_DATA(CCDTransfer)
	enum { IDD = IDD_CDPPG_TRANSFER };
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCDTransfer)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCDTransfer)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CUpDownClient* m_pClient;
	void Localize(void);
	void Update(void);
	virtual BOOL OnInitDialog();
	
protected:
	CScrollStatic m_ctrlCurrentDownload;
	CStatic		 m_ctrlDownloadedSession;
	CStatic		 m_ctrlDownloadedTotal;
	CStatic		 m_ctrlAverageDownloadRate;
	CStatic		 m_ctrlUploadedSession;
	CStatic		 m_ctrlUploadedTotal;
	CStatic		 m_ctrlAverageUploadRate;
};

