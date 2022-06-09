#pragma once

#include "ScrollStatic.h"
#include "Separator.h"
#include "..\PartFile.h"
#include "..\Resource.h"

// CFDAviInfo dialog

class CFDAviInfo : public CPropertyPage
{
	DECLARE_DYNCREATE(CFDAviInfo)

// Construction
public:
	CFDAviInfo();
	~CFDAviInfo();

// Dialog Data
	//{{AFX_DATA(CFDAviInfo)
	enum { IDD = IDD_FDPPG_AVIINFO };
	CStatic		m_ctrlWidth;
	CStatic		m_ctrlVideoCodec;
	CStatic		m_ctrlVideoBitrate;
	CStatic		m_ctrlSamplerate;
	CStatic		m_ctrlHeight;
	CStatic		m_ctrlFPS;
	CStatic		m_ctrlChannel;
	CStatic		m_ctrlAudioCodec;
	CStatic		m_ctrlAudioBitrate;
	CStatic		m_ctrlFilesize;
	CStatic		m_ctrlFilelength;
	CSeparator	m_ctrlVideoLbl;
	CSeparator	m_ctrlPictureLbl;
	CSeparator	m_ctrlAudioLbl;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CFDAviInfo)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CFDAviInfo)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	void Update();
	void Localize();

	CPartFile	*m_pFile;
	uint32		m_dwAbitrate;
	BOOL		m_bARoundBitrate;

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedRoundbitrateBtn();
};
