#include "stdafx.h"
#include "SnapDialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSnapDialog dialog


CSnapDialog::CSnapDialog(UINT IDD, CWnd* pParent /*=NULL*/)
: CDialogSK(IDD, pParent)
{
	//{{AFX_DATA_INIT(CSnapDialog)
	// NOTE: the ClassWizard will add member initialization here
	m_nOffset = 25;
	//}}AFX_DATA_INIT
}

void CSnapDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogSK::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSnapDialog)
	// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CSnapDialog, CDialogSK)
	//{{AFX_MSG_MAP(CSnapDialog)
	// NOTE: the ClassWizard will add message map macros here
	ON_WM_WINDOWPOSCHANGING()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSnapDialog message handlers

void CSnapDialog::OnWindowPosChanging( WINDOWPOS* wndPos )
{
	RECT rcScrn;
	SystemParametersInfo (SPI_GETWORKAREA, 0, &rcScrn, 0);

	// Snap X axis
	if (abs(wndPos->x - rcScrn.left) <= 15)
	{
		wndPos->x = rcScrn.left;
	}
	else if (abs(wndPos->x + wndPos->cx - rcScrn.right) <= m_nOffset)
	{
		wndPos->x = rcScrn.right - wndPos->cx;
	}

	// Snap Y axis
	if (abs(wndPos->y - rcScrn.top) <= m_nOffset)
	{
		wndPos->y = rcScrn.top;
	}
	else if (abs(wndPos->y + wndPos->cy - rcScrn.bottom) <= m_nOffset)
	{
		wndPos->y = rcScrn.bottom - wndPos->cy;
	}
}