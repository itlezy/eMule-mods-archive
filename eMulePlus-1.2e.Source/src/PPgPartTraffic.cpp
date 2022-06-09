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
#include "PPgPartTraffic.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgPartTraffic, CPropertyPage)

CPPgPartTraffic::CPPgPartTraffic()
	: CPropertyPage(CPPgPartTraffic::IDD)
	, m_iUploadBarStyle(0)
	, m_iUploadBarColor(0)
	, m_bDisplayPartTraffic(FALSE)
{
}

CPPgPartTraffic::~CPPgPartTraffic()
{
}

void CPPgPartTraffic::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_UPBARST1, m_iUploadBarStyle);
	DDX_Radio(pDX, IDC_UPBARCL1, m_iUploadBarColor);
	DDX_Check(pDX, IDC_PT_USEIT, m_bDisplayPartTraffic);
}

BEGIN_MESSAGE_MAP(CPPgPartTraffic, CPropertyPage)
	ON_BN_CLICKED(IDC_UPBARST1, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPBARST2, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPBARST3, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPBARST4, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPBARCL1, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPBARCL2, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPBARCL3, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPBARCL4, OnSettingsChange)
	ON_BN_CLICKED(IDC_PT_USEIT, OnBnClickedPtUseit)
END_MESSAGE_MAP()

BOOL CPPgPartTraffic::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgPartTraffic::LoadSettings(void)
{
	if(::IsWindow(m_hWnd))
	{
		m_bDisplayPartTraffic = m_pPrefs->DoUsePT();

		m_iUploadBarStyle = m_pPrefs->GetUpbarStyle();
		m_iUploadBarColor = m_pPrefs->GetUpbarColor();

		UpdateData(FALSE);

		OnBnClickedPtUseit();

		SetModified(FALSE);
	}
}

BOOL CPPgPartTraffic::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		if (!m_bDisplayPartTraffic == m_pPrefs->DoUsePT())
		{
			m_pPrefs->SetUsePT(B2b(m_bDisplayPartTraffic));
			if(m_pPrefs->DoUsePT())
				g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.ShowColumn(SFL_COLUMN_PARTTRAFFIC);
			else
				g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.HideColumn(SFL_COLUMN_PARTTRAFFIC);
		}

		m_pPrefs->SetUpbarStyle(static_cast<byte>(m_iUploadBarStyle));
		m_pPrefs->SetUpbarColor(static_cast<byte>(m_iUploadBarColor));
		
		g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.SetDisplay(m_pPrefs->GetUpbarStyle(), true);
		g_App.m_pMDlg->m_wndSharedFiles.m_ctlSharedFilesList.SetColoring(m_pPrefs->GetUpbarColor());

		SetModified(FALSE);
	}
	return CPropertyPage::OnApply();
}

void CPPgPartTraffic::Localize(void)
{	
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_PT_USEIT, IDS_PT_USEIT },
		{ IDC_UPBARST, IDS_UPBARST },
		{ IDC_UPBARST1, IDS_UPBARST1 },
		{ IDC_UPBARST2, IDS_UPBARST2 },
		{ IDC_UPBARST3, IDS_UPBARST3 },
		{ IDC_UPBARCL, IDS_UPBARCL },
		{ IDC_UPBARCL1, IDS_UPBARCL1 },
		{ IDC_UPBARCL2, IDS_UPBARCL2 },
		{ IDC_UPBARCL3, IDS_UPBARCL3 },
		{ IDC_UPBARCL4, IDS_UPBARCL4 }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
	}
}

void CPPgPartTraffic::OnBnClickedPtUseit()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_UPBARST1)->EnableWindow(m_bDisplayPartTraffic);
	GetDlgItem(IDC_UPBARST2)->EnableWindow(m_bDisplayPartTraffic);
	GetDlgItem(IDC_UPBARST3)->EnableWindow(m_bDisplayPartTraffic);
	GetDlgItem(IDC_UPBARST4)->EnableWindow(m_bDisplayPartTraffic);
	GetDlgItem(IDC_UPBARCL1)->EnableWindow(m_bDisplayPartTraffic);
	GetDlgItem(IDC_UPBARCL2)->EnableWindow(m_bDisplayPartTraffic);
	GetDlgItem(IDC_UPBARCL3)->EnableWindow(m_bDisplayPartTraffic);
	GetDlgItem(IDC_UPBARCL4)->EnableWindow(m_bDisplayPartTraffic);

	SetModified();
}
