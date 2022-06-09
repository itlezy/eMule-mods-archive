#pragma once

#include "..\resource.h"
#include "Separator.h"
#include "scrollstatic.h"

// CCDScores dialog

class CUpDownClient;
class CCDScores : public CPropertyPage
{
	DECLARE_DYNCREATE(CCDScores)

// Construction
public:
	CCDScores();
	~CCDScores();

// Dialog Data
	//{{AFX_DATA(CCDScores)
	enum { IDD = IDD_CDPPG_SCORES };
	CSeparator	m_ctrlScores;
	CSeparator	m_ctrlRemoteScores;
	//}}AFX_DATA


// Overrides
	// ClassWizard generate virtual function overrides
	//{{AFX_VIRTUAL(CCDScores)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	// Generated message map functions
	//{{AFX_MSG(CCDScores)
		// NOTE: the ClassWizard will add member functions here
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

public:
	CUpDownClient* m_pClient;

	virtual BOOL OnInitDialog();
	void Update(void);
	void Localize(void);
protected:
	CStatic m_ctrlDlUpModifier;
	CStatic m_ctrlCommunityUser;
	CStatic m_ctrlRating;
	CStatic m_ctrlUpQueueScore;
	CStatic m_ctrlQueueTime;
	CStatic m_ctrlRemDlUpModifier;
	CStatic m_ctrlRemRating;
};
