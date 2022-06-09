//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "emule.h"
#include "ServerWnd.h"
#include "NewServerDlg.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CNewServerDlg, CDialog)
CNewServerDlg::CNewServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CNewServerDlg::IDD, pParent)
	, m_strServerAddr(_T(""))
	, m_strPort(_T("4661"))
	, m_strServerName(_T(""))
	, m_bAddAuxPort(FALSE)
	, m_strAuxPort(_T("0"))
{
	m_cpPosition.x = 0;
	m_cpPosition.y = 0;
	m_iCorner = 0;	// top-left
	m_bUsePos = false;
	m_pParent = NULL;
}

CNewServerDlg::~CNewServerDlg()
{
}

void CNewServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_IPADDRESS, m_strServerAddr);
	DDX_Text(pDX, IDC_SPORT, m_strPort);
	DDX_Text(pDX, IDC_SNAME, m_strServerName);
	DDX_Check(pDX, IDC_AUXPORT_CHECKBOX, m_bAddAuxPort);
	DDX_Text(pDX, IDC_AUXPORT, m_strAuxPort);
}


BEGIN_MESSAGE_MAP(CNewServerDlg, CDialog)
	ON_BN_CLICKED(IDC_ADDSERVER, OnBnClickedAddserver)
	ON_BN_CLICKED(IDC_AUXPORT_CHECKBOX, OnBnClickedAuxportCheckbox)
END_MESSAGE_MAP()


BOOL CNewServerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();
	if(m_bUsePos)
	{
		CRect rWnd;
		GetWindowRect(rWnd);
		
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
	UpdateData(false);

	if (!g_App.m_pPrefs->IsServerAuxPortUsed())
	{
		GetDlgItem(IDC_AUXPORT_CHECKBOX)->EnableWindow(FALSE);
		GetDlgItem(IDC_AUXPORT)->EnableWindow(FALSE);
	}
	else
	{
		if (m_strAuxPort.Compare(_T("0")) == 0)
		{
			CheckDlgButton(IDC_AUXPORT_CHECKBOX, BST_UNCHECKED);
			GetDlgItem(IDC_AUXPORT)->EnableWindow(FALSE);
		}
		else
		{
			CheckDlgButton(IDC_AUXPORT_CHECKBOX, BST_CHECKED);
		}
	}

	if (m_bServerEditMode)
	{
		static_cast<CEdit*>(GetDlgItem(IDC_IPADDRESS))->SetReadOnly();
	}

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CNewServerDlg::SetInitialPos(CPoint pos, int iCorner)
{
	m_iCorner = iCorner;
	m_cpPosition = pos;
	m_bUsePos = true;
}

void CNewServerDlg::OnBnClickedAddserver()
{
	UpdateData();
	if(!m_pParent)
		return;

	if ((m_strPort.Compare(m_strAuxPort) == 0) || (m_strAuxPort.Compare(_T("0")) == 0))
	{
		m_strAuxPort = _T("");
	}

	m_pParent->AddServer(m_strServerAddr, m_strPort, m_strServerName, m_strAuxPort, m_bServerEditMode);
	OnOK();
}

void CNewServerDlg::SetParent(CServerWnd* pParent)
{
	m_pParent = pParent;
}

void CNewServerDlg::SetLabels(CString strAddress, CString strPort, CString strName, CString strAuxPort)
{
	m_strServerAddr	= strAddress;
	m_strPort		= strPort;
	m_strServerName	= strName;
	m_strAuxPort	= strAuxPort;
}

void CNewServerDlg::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_IPADDRESS_LBL, IDS_SV_ADDRESS },
		{ IDC_PORT_LBL, IDS_PORT },
		{ IDC_NAME_LBL, IDS_SW_NAME },
		{ IDC_ADDSERVER, IDS_SV_ADD },
		{ IDC_AUXPORT_CHECKBOX, IDS_SV_AUXPORT_CHECKBOX },
		{ IDCANCEL, IDS_CANCEL }
	};

	if(m_hWnd)
	{		
		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
			SetDlgItemText(s_auResTbl[i][0], GetResString(static_cast<UINT>(s_auResTbl[i][1])));

		if (m_bServerEditMode)
		{
			SetDlgItemText(IDC_ADDSERVER, GetResString(IDS_OK_BUTTON));
			SetWindowText(GetResString(IDS_SV_EDITSERVER));
		}
		else
		{
			SetWindowText(GetResString(IDS_SV_NEWSERVER));
		}
	}
}

void CNewServerDlg::OnBnClickedAuxportCheckbox()
{
	UpdateData();
	GetDlgItem(IDC_AUXPORT)->EnableWindow(m_bAddAuxPort);
}
