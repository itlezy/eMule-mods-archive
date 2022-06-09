#pragma once

#include "ScrollStatic.h"
#include "..\PartFile.h"
#include "..\resource.h"

// CFDGeneral dialog

class CFDGeneral : public CPropertyPage
{
	DECLARE_DYNCREATE(CFDGeneral)

// Construction
public:
	CFDGeneral();
	~CFDGeneral();

	void Update();

	CPartFile* m_pFile;

// Dialog Data
	//{{AFX_DATA(CFDGeneral)
	enum { IDD = IDD_FDPPG_GENERAL };
	CStatic			m_ctrlPartFileStatus;
	CScrollStatic	m_ctrlMetFile;
	CStatic			m_ctrlHash;
	CScrollStatic	m_ctrlFolder;
	CScrollStatic	m_ctrlFiletype;
	CStatic			m_ctrlFilesize;
	CScrollStatic	m_ctrlDescription;
	CStatic			m_ctrlCategory;
	CStatic			m_ctrlFileIcon;
	CEdit			m_ctrlFilename;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFDGeneral)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hFileIcon;

	// Generated message map functions
	//{{AFX_MSG(CFDGeneral)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedCommentsBtn();
	virtual BOOL OnInitDialog();
	void Localize(bool bIsExecuteable);
};
