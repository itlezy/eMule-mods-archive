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
#include "PPgSources.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgSources, CPropertyPage)
CPPgSources::CPPgSources()
	: CPropertyPage(CPPgSources::IDD)
	, m_bAutoSrcEnabled(FALSE)
	, m_dwMinAutoSrcPerFile(0)
	, m_dwMaxAutoSrcPerFile(0)
	, m_dwMaxAutoSrcTotal(0)
	, m_dwMaxAutoXchgSources(0)
	, m_bDisableXS(FALSE)
	, m_bEnableXSUpTo(FALSE)
{
}

CPPgSources::~CPPgSources()
{
}

void CPPgSources::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_AUTOSRC_ENABLED, m_bAutoSrcEnabled);
	DDX_Check(pDX, IDC_DISABLE_XS, m_bDisableXS);
	DDX_Check(pDX, IDC_ENABLE_XSUPTO, m_bEnableXSUpTo);
	DDX_Text(pDX, IDC_MAXSOURCEPERFILE, m_strMaxSrcPerFile);
	DDX_Text(pDX, IDC_AUTOSRC_MIN_PER_FILE, m_dwMinAutoSrcPerFile);
	DDX_Text(pDX, IDC_AUTOSRC_MAX_PER_FILE, m_dwMaxAutoSrcPerFile);
	DDX_Text(pDX, IDC_AUTOSRC_MAXTOTAL, m_dwMaxAutoSrcTotal);
	DDX_Text(pDX, IDC_AUTOSRC_SE_UPTO, m_dwMaxAutoXchgSources);
	DDX_Text(pDX, IDC_XSUPTO, m_strXsUpTo);
}

BEGIN_MESSAGE_MAP(CPPgSources, CPropertyPage)
	ON_EN_CHANGE(IDC_MAXSOURCEPERFILE, OnSettingsChange)
	ON_EN_KILLFOCUS(IDC_MAXSOURCEPERFILE, OnSourcesChange)
	ON_BN_CLICKED(IDC_AUTOSRC_ENABLED, OnBnClickedAutoSources)
	ON_BN_CLICKED(IDC_DISABLE_XS, OnBnClickedDisableXS)
	ON_EN_CHANGE(IDC_AUTOSRC_MIN_PER_FILE, OnSettingsChange)
	ON_EN_CHANGE(IDC_AUTOSRC_MAX_PER_FILE, OnSettingsChange)
	ON_EN_CHANGE(IDC_AUTOSRC_MAXTOTAL, OnSettingsChange)
	ON_EN_CHANGE(IDC_AUTOSRC_SE_UPTO, OnSettingsChange)
	ON_BN_CLICKED(IDC_ENABLE_XSUPTO, OnBnClickedDisableXSUpTo)
	ON_EN_CHANGE(IDC_XSUPTO, OnSettingsChange)
END_MESSAGE_MAP()

BOOL CPPgSources::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgSources::LoadSettings(void)
{
	m_strMaxSrcPerFile.Format(_T("%u"), m_pPrefs->GetMaxSourcePerFile());

	m_bAutoSrcEnabled = m_pPrefs->IsAutoSourcesEnabled();
	m_dwMinAutoSrcPerFile = m_pPrefs->GetMinAutoSourcesPerFile();
	m_dwMaxAutoSrcPerFile = m_pPrefs->GetMaxAutoSourcesPerFile();
	m_dwMaxAutoSrcTotal = m_pPrefs->GetMaxAutoSourcesTotal();
	m_dwMaxAutoXchgSources = m_pPrefs->GetMaxAutoExchangeSources();

	m_bDisableXS = m_pPrefs->IsDisabledXS();
	m_bEnableXSUpTo = m_pPrefs->DisableXSUpTo();
	m_strXsUpTo.Format(_T("%u"), m_pPrefs->XSUpTo());

	UpdateData(FALSE);

	OnBnClickedAutoSources();

	SetModified(FALSE);
}

BOOL CPPgSources::OnApply()
{
	if(m_bModified)
	{
		int	iVal, iMax;

		UpdateData(TRUE);

		m_pPrefs->SetMaxSourcePerFile(_tstoi(m_strMaxSrcPerFile));
		m_pPrefs->SetAutoSourcesEnabled(B2b(m_bAutoSrcEnabled));
		m_pPrefs->SetMinAutoSourcesPerFile(m_dwMinAutoSrcPerFile);
		m_pPrefs->SetMaxAutoSourcesPerFile(m_dwMaxAutoSrcPerFile);
		m_pPrefs->SetMaxAutoSourcesTotal(m_dwMaxAutoSrcTotal);
		m_pPrefs->SetMaxAutoExchangeSources(m_dwMaxAutoXchgSources);

		m_pPrefs->SetDisabledXS(B2b(m_bDisableXS));
		m_pPrefs->SetDisableXSUpTo(B2b(m_bEnableXSUpTo));

		if ((iVal = _tstoi(m_strXsUpTo)) > (iMax = m_pPrefs->GetMaxSourcePerFileSoft()))
			m_strXsUpTo.Format(_T("%u"), iVal = iMax);
		m_pPrefs->SetXSUpTo(iVal);

		UpdateData(FALSE);
		SetModified(FALSE);
	}
	return CPropertyPage::OnApply();
}

void CPPgSources::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_AUTOSRC_ENABLED, IDS_ENABLED },
		{ IDC_DISABLE_XS, IDS_DISABLE_XS },
		{ IDC_ENABLE_XSUPTO, IDS_ENABLE_XSUPTO },
		{ IDC_AUTOSRC_GRP_LBL, IDS_AUTOSRC_GRP_LBL },
		{ IDC_AUTOSRC_FROM_LBL, IDS_AUTOSRC_FROM_LBL },
		{ IDC_AUTOSRC_TO_LBL, IDS_AUTOSRC_TO_LBL },
		{ IDC_AUTOSRC_MAXTOTAL_LBL, IDS_AUTOSRC_MAXTOTAL_LBL },
		{ IDC_AUTOSRC_SE_UPTO_LBL, IDS_AUTOSRC_SE_UPTO_LBL },
		{ IDC_MAXSRC_LBL, IDS_PW_MAXSOURCES },
		{ IDC_XSSOURCES, IDS_SOURCES_PF_FILES }
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

void CPPgSources::OnSourcesChange()
{
	UpdateData(TRUE);

	uint32	dwNum = _tstoi(m_strMaxSrcPerFile);

	if (dwNum < PREF_MIN_MAXFILESRC)
		dwNum = PREF_MIN_MAXFILESRC;
	if (dwNum > PREF_MAX_MAXFILESRC)
		dwNum = PREF_MAX_MAXFILESRC;
	if (dwNum != m_pPrefs->GetMaxSourcePerFile())
	{
		m_strMaxSrcPerFile.Format(_T("%u"), dwNum);
		UpdateData(FALSE);
	}
}

void CPPgSources::OnBnClickedAutoSources()
{
	OnBnClickedDisableXS();

	GetDlgItem(IDC_AUTOSRC_MIN_PER_FILE)->EnableWindow(m_bAutoSrcEnabled);
	GetDlgItem(IDC_AUTOSRC_MAX_PER_FILE)->EnableWindow(m_bAutoSrcEnabled);
	GetDlgItem(IDC_AUTOSRC_MAXTOTAL)->EnableWindow(m_bAutoSrcEnabled);
	GetDlgItem(IDC_AUTOSRC_SE_UPTO)->EnableWindow(m_bAutoSrcEnabled);
	GetDlgItem(IDC_MAXSOURCEPERFILE)->EnableWindow(!m_bAutoSrcEnabled);

	SetModified();
}

void CPPgSources::OnBnClickedDisableXS()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_ENABLE_XSUPTO)->EnableWindow(!m_bDisableXS && !m_bAutoSrcEnabled);
	GetDlgItem(IDC_XSUPTO)->EnableWindow(!m_bDisableXS && m_bEnableXSUpTo && !m_bAutoSrcEnabled);
	SetModified();
}

void CPPgSources::OnBnClickedDisableXSUpTo()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_XSUPTO)->EnableWindow(m_bEnableXSUpTo);

	SetModified();
}
