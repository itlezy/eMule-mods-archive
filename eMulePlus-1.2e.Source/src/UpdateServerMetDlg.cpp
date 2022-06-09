// UpdateServerMetDlg.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "UpdateServerMetDlg.h"
#include "ServerWnd.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CUpdateServerMetDlg dialog

IMPLEMENT_DYNAMIC(CUpdateServerMetDlg, CDialog)
CUpdateServerMetDlg::CUpdateServerMetDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CUpdateServerMetDlg::IDD, pParent)
	, m_strServerMetURL(_T(""))
{
	m_cpPosition.x = 0;
	m_cpPosition.y = 0;

	m_iCorner = 0;	// top-left

	m_bUsePos = false;
}

CUpdateServerMetDlg::~CUpdateServerMetDlg()
{
}

void CUpdateServerMetDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_SERVERMETURL, m_strServerMetURL);
	DDX_Control(pDX, IDC_SERVERMETURL, m_ComboBox);
}


BEGIN_MESSAGE_MAP(CUpdateServerMetDlg, CDialog)
	ON_BN_CLICKED(IDC_UPDATESERVERMETFROMURL, OnBnClickedUpdateservermetfromurl)
	ON_CBN_SELCHANGE(IDC_SERVERMET_URL, OnCbnSelchangeServerMetURL)
END_MESSAGE_MAP()


// CUpdateServerMetDlg message handlers

BOOL CUpdateServerMetDlg::OnInitDialog()
{
	EMULE_TRY
	CDialog::OnInitDialog();
	if(m_bUsePos)
	{
		CRect rWnd;
		GetWindowRect(&rWnd);
		
		switch(m_iCorner)
		{
			case 0: // top-left
				SetWindowPos(NULL, m_cpPosition.x, m_cpPosition.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				break;
			case 1:	// top-right
				SetWindowPos(NULL, m_cpPosition.x - rWnd.Width(), m_cpPosition.y, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				break;
			case 2: // bottom-right
				SetWindowPos(NULL, m_cpPosition.x - rWnd.Width(), m_cpPosition.y - rWnd.Height(), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				break;
			case 3: // bottom-left
				SetWindowPos(NULL, m_cpPosition.x, m_cpPosition.y - rWnd.Height(), 0, 0, SWP_NOZORDER | SWP_NOSIZE);
				break;
			default:
				break;
		}
	}
	Localize();
    // 26.5 EC
	if (g_App.m_pPrefs->m_addressesList.IsEmpty()) return TRUE;
	POSITION Pos = g_App.m_pPrefs->m_addressesList.GetHeadPosition();
	while (Pos != NULL)
	     m_ComboBox.AddString(g_App.m_pPrefs->m_addressesList.GetNext(Pos));
    // EC Ends

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
	EMULE_CATCH
	return FALSE;
}

void CUpdateServerMetDlg::SetInitialPos(CPoint pos, int iCorner)
{
	m_iCorner = iCorner;
	m_cpPosition = pos;
	m_bUsePos = true;
}

void CUpdateServerMetDlg::SetParent(CServerWnd* pParent)
{
	m_pParent = pParent;
}

void CUpdateServerMetDlg::Localize()
{
	if(m_hWnd)
	{		
		SetWindowText(GetResString(IDS_SV_MET));
		SetDlgItemText(IDC_URL_LBL, GetResString(IDS_SV_URL));
		SetDlgItemText(IDC_UPDATESERVERMETFROMURL, GetResString(IDS_SV_UPDATE));
	}
}

void CUpdateServerMetDlg::OnBnClickedUpdateservermetfromurl()
{
	UpdateData();

	if(!m_pParent)
		return;

	//if(m_strServerMetURL.IsEmpty())
	//	return;
	
	EndDialog(0x00);
	m_pParent->UpdateServerMetFromURL(m_strServerMetURL);
}

void CUpdateServerMetDlg::OnOK()
{
	OnBnClickedUpdateservermetfromurl();
}

void CUpdateServerMetDlg::OnCbnSelchangeServerMetURL()
{
}
