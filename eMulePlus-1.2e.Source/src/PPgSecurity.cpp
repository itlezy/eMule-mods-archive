//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "emule.h"
#include "PPgSecurity.h"
#include "AddBuddy.h"
#include "IPFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
IMPLEMENT_DYNAMIC(CPPgSecurity, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgSecurity, CPropertyPage)
	ON_EN_CHANGE(IDC_FILTERLEVEL_EDIT, OnSettingsChange)
	ON_EN_CHANGE(IDC_IPFILTER_URL_EDIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_FILTERCLIENTS_CHECK, OnSettingsChange)
	ON_BN_CLICKED(IDC_FILTERSERVERS_CHECK, OnSettingsChange)
	ON_BN_CLICKED(IDC_FILESCAN_FILTER_CHECK, OnSettingsChange)
	ON_BN_CLICKED(IDC_DROP_FAKE_RX, OnSettingsChange)
	ON_BN_CLICKED(IDC_IPFILTER_UPDATE_BUTTON, OnBnClickedIpfilterUpdateButton)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_IPFILTER_UPDATEONSTART_CHECK, OnBnClickedIpfilterUpdateonstartCheck)
	ON_BN_CLICKED(IDC_AV_SCANCOMPLETED_CHECK, OnSettingsChange)
	ON_BN_CLICKED(IDC_AV_ENABLED_CHECK, OnBnClickedAvEnable)
	ON_BN_CLICKED(IDC_AV_BROWSE, OnBnClickedAvbrowse)
	ON_EN_CHANGE(IDC_AV_PARAMS_EDIT, OnSettingsChange)
	ON_EN_CHANGE(IDC_AV_PATH_EDIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_ENABLEOBFUSCATION, OnObfuscatedRequestedChange)
	ON_BN_CLICKED(IDC_ONLYOBFUSCATED, OnSettingsChange)
	ON_BN_CLICKED(IDC_DISABLEOBFUSCATION, OnObfuscatedDisabledChange)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////////////////////
CPPgSecurity::CPPgSecurity()
	: CPropertyPage(CPPgSecurity::IDD)
	, m_bFilterClientsCheck(FALSE)
	, m_bFilterServersCheck(FALSE)
	, m_bIPFilterUpdateOnStartCheck(FALSE)
	, m_bFileScanFilterCheck(FALSE)
	, m_bFakeDataFilterCheck(FALSE)
	, m_bAVEnabledCheck(FALSE)
	, m_bAVScanCompletedCheck(FALSE)
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
CPPgSecurity::~CPPgSecurity()
{
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_IPFILTER_URL_EDIT, m_strIPFilterURLEdit);
	DDX_Check(pDX, IDC_FILTERCLIENTS_CHECK, m_bFilterClientsCheck);
	DDX_Check(pDX, IDC_FILTERSERVERS_CHECK, m_bFilterServersCheck);
	DDX_Check(pDX, IDC_IPFILTER_UPDATEONSTART_CHECK, m_bIPFilterUpdateOnStartCheck);
	DDX_Check(pDX, IDC_FILESCAN_FILTER_CHECK, m_bFileScanFilterCheck);
	DDX_Check(pDX, IDC_DROP_FAKE_RX, m_bFakeDataFilterCheck);
	DDX_Control(pDX, IDC_FILTERLEVEL_SPIN, m_FilterLevelSpinCtrl);
	DDX_Control(pDX, IDC_UPDATE_DAYS_SLIDER, m_UpdateDaysSlider);
	DDX_Control(pDX, IDC_AV_BROWSE, m_AVBrowseButton);
	DDX_Check(pDX, IDC_AV_ENABLED_CHECK, m_bAVEnabledCheck);
	DDX_Check(pDX, IDC_AV_SCANCOMPLETED_CHECK, m_bAVScanCompletedCheck);
	DDX_Text(pDX, IDC_AV_PATH_EDIT, m_strAVPath);
	DDX_Control(pDX, IDC_AV_PATH_EDIT, m_strAVPathEdit);
	DDX_Text(pDX, IDC_AV_PARAMS_EDIT, m_strAVParams);
	DDX_Control(pDX, IDC_AV_PARAMS_EDIT, m_strAVParamsEdit);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_IPFILTER_GROUPBOX, IDS_IPFILTER_GROUPBOX },
		{ IDC_FILTERLEVEL_STATIC, IDS_FILTERLEVEL_STATIC },
		{ IDC_IPFILTER_URL_STATIC, IDS_IPFILTER_URL_STATIC },
		{ IDC_AV_GROUPBOX, IDS_AV_GROUPBOX },
		{ IDC_AV_PATH_TEXT, IDS_AV_PATH },
		{ IDC_AV_PARAMS_TEXT, IDS_AV_PARAMS },
		{ IDC_AV_ENABLED_CHECK, IDS_ENABLED },
		{ IDC_FILTERCLIENTS_CHECK, IDS_PW_FILTER },
		{ IDC_FILTERSERVERS_CHECK, IDS_FILTERSERVERSBYIP },
		{ IDC_IPFILTER_UPDATEONSTART_CHECK, IDS_IPFILTER_UPDATEONSTART_CHECK },
		{ IDC_FILESCAN_FILTER_CHECK, IDS_SCANFILTER },
		{ IDC_DROP_FAKE_RX, IDS_DROP_FAKE_RX },
		{ IDC_AV_SCANCOMPLETED_CHECK, IDS_AV_SCAN_COMPLETED },
		{ IDC_IPFILTER_UPDATE_BUTTON, IDS_SV_UPDATE },
		{ IDC_OBFUSCATION_GROUPBOX, IDS_OBFUSCATION },
		{ IDC_ENABLEOBFUSCATION, IDS_ENABLEOBFUSCATION },
		{ IDC_ONLYOBFUSCATED, IDS_ONLYOBFUSCATED },
		{ IDC_DISABLEOBFUSCATION, IDS_DISABLEOBFUSCATION }
	};

	if(::IsWindow(m_hWnd))
	{
		CString	strRes;

		SetDaysCaption(m_UpdateDaysSlider.GetPos());

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::LoadSettings(void)
{
	m_FilterLevelSpinCtrl.SetPos(m_pPrefs->GetIPFilterLevel());
	m_UpdateDaysSlider.SetPos(m_pPrefs->GetIPFilterUpdateFrequency());
	m_strIPFilterURLEdit			= m_pPrefs->GetIPFilterURL();
	m_bFilterClientsCheck			= m_pPrefs->IsFilterBadIPs();
	m_bFilterServersCheck			= m_pPrefs->IsFilterServersByIP();
	m_bIPFilterUpdateOnStartCheck	= m_pPrefs->IsIPFilterUpdateOnStart();
	m_bFileScanFilterCheck			= m_pPrefs->IsScanFilterEnabled();
	m_bFakeDataFilterCheck			= m_pPrefs->IsFakeRxDataFilterEnabled();

	GetDlgItem(IDC_UPDATE_DAYS_STATIC)->EnableWindow(m_bIPFilterUpdateOnStartCheck);
	m_UpdateDaysSlider.EnableWindow(m_bIPFilterUpdateOnStartCheck);

	m_bAVEnabledCheck				= m_pPrefs->IsAVEnabled();
	m_bAVScanCompletedCheck			= m_pPrefs->IsAVScanCompleted();
	m_strAVPath						= m_pPrefs->GetAVPath();
	m_strAVParams					= m_pPrefs->GetAVParams();

	GetDlgItem(IDC_ENABLEOBFUSCATION)->EnableWindow((m_pPrefs->IsClientCryptLayerSupported()) ? TRUE : FALSE);
	GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow((m_pPrefs->IsClientCryptLayerRequested()) ? TRUE : FALSE);
	CheckDlgButton(IDC_DISABLEOBFUSCATION, (m_pPrefs->IsClientCryptLayerSupported()) ? BST_UNCHECKED : BST_CHECKED);
	CheckDlgButton(IDC_ENABLEOBFUSCATION, (m_pPrefs->GetClientCryptLayerRequested()) ? BST_CHECKED : BST_UNCHECKED);
	CheckDlgButton(IDC_ONLYOBFUSCATED, (m_pPrefs->GetClientCryptLayerRequired()) ? BST_CHECKED : BST_UNCHECKED);

	UpdateData(FALSE);

	OnBnClickedAvEnable();

	SetModified(FALSE);
}
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPPgSecurity::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_FilterLevelSpinCtrl.SetRange(0, 255);
	m_UpdateDaysSlider.SetRange(0, 6);

	m_strAVPathEdit.SetLimitText(MAX_PATH);
	m_strAVParamsEdit.SetLimitText(256);

	AddBuddy(m_strAVPathEdit.m_hWnd, m_AVBrowseButton.m_hWnd, BDS_RIGHT);

	LoadSettings();
	Localize();

	return TRUE;
}
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPPgSecurity::OnApply()
{
	if(m_bModified)
	{
		uint32	dwVal;

		UpdateData();

		m_pPrefs->SetIPFilterLevel(((dwVal = m_FilterLevelSpinCtrl.GetPos()) > 255) ? 255 : dwVal);
		g_App.m_pIPFilter->ResetCachedFilteredIP();
		m_pPrefs->SetIPFilterUpdateFrequency(m_UpdateDaysSlider.GetPos());
		m_pPrefs->SetIPFilterURL(m_strIPFilterURLEdit);
		m_pPrefs->SetFilterBadIPs(B2b(m_bFilterClientsCheck));
		m_pPrefs->SetFilterServersByIP(B2b(m_bFilterServersCheck));
		m_pPrefs->SetIPFilterUpdateOnStart(B2b(m_bIPFilterUpdateOnStartCheck));
		m_pPrefs->SetScanFilterEnabled(B2b(m_bFileScanFilterCheck));
		m_pPrefs->SetFakeRxDataFilterEnabled(B2b(m_bFakeDataFilterCheck));

		m_pPrefs->SetAVEnabled(B2b(m_bAVEnabledCheck));
		m_pPrefs->SetAVScanCompleted(B2b(m_bAVScanCompletedCheck));
		m_pPrefs->SetAVPath(m_strAVPath);
		m_pPrefs->SetAVParams(m_strAVParams);

#ifdef _CRYPT_READY
		m_pPrefs->SetClientCryptLayerSupported(IsDlgButtonChecked(IDC_DISABLEOBFUSCATION) == BST_UNCHECKED);
		m_pPrefs->SetClientCryptLayerRequested(IsDlgButtonChecked(IDC_ENABLEOBFUSCATION) != BST_UNCHECKED);
		m_pPrefs->SetClientCryptLayerRequired(IsDlgButtonChecked(IDC_ONLYOBFUSCATED) != BST_UNCHECKED);
#endif

		SetModified(FALSE);
		LoadSettings();
	}

	return CPropertyPage::OnApply();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::OnBnClickedIpfilterUpdateButton()
{
	UpdateData(TRUE);

	m_pPrefs->SetIPFilterURL(m_strIPFilterURLEdit);

	if (g_App.m_pIPFilter->DownloadIPFilter())
	{
		m_pPrefs->SetLastIPFilterUpdate(static_cast<uint32>(mktime(CTime::GetCurrentTime().GetLocalTm(NULL))));
		GetDlgItem(IDC_IPFILTER_UPDATE_BUTTON)->EnableWindow(FALSE);
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == reinterpret_cast<CScrollBar*>(&m_UpdateDaysSlider))
	{
		int iSliderPos = m_UpdateDaysSlider.GetPos();

		SetDaysCaption(iSliderPos);
		SetModified();
	}

	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::SetDaysCaption(int iSliderPos)
{
	CString strRes;

	switch(iSliderPos)
	{
		case 0:
			GetResString(&strRes, IDS_IPFILTER_UPDATE_DAILY);
			break;
		case 6:
			GetResString(&strRes, IDS_IPFILTER_UPDATE_WEEKLY);
			break;
		default:
			strRes.Format(GetResString(IDS_IPFILTER_UPDATE_DAYS), iSliderPos + 1);
			break;
	}
	SetDlgItemText(IDC_UPDATE_DAYS_STATIC, strRes);
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::OnBnClickedIpfilterUpdateonstartCheck()
{
	UpdateData();
	GetDlgItem(IDC_UPDATE_DAYS_STATIC)->EnableWindow(m_bIPFilterUpdateOnStartCheck);
	m_UpdateDaysSlider.EnableWindow(m_bIPFilterUpdateOnStartCheck);
	SetModified();
}
/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPPgSecurity::OnSetActive()
{
	CTime LastUpdate(static_cast<time_t>(m_pPrefs->GetLastIPFilterUpdate()));
	time_t tLastUpdate = mktime(LastUpdate.GetLocalTm(NULL));
	time_t tNow = mktime(CTime::GetCurrentTime().GetLocalTm(NULL));

//	Disables Update button for 24h
	GetDlgItem(IDC_IPFILTER_UPDATE_BUTTON)->EnableWindow(
		(difftime(tNow, tLastUpdate) < (24 * 60 * 60)) ? FALSE : TRUE);

	return CPropertyPage::OnSetActive();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::OnBnClickedAvbrowse()
{
	UpdateData(TRUE);
	if (DialogBrowseFile(m_strAVPath, GetResString(IDS_AV_PATH) + _T(" (*.exe)|*.exe||"), NULL, OFN_FILEMUSTEXIST))
	{
		UpdateData(FALSE);
		SetModified();
	}
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::OnBnClickedAvEnable()
{
	UpdateData(TRUE);

	m_strAVPathEdit.EnableWindow(m_bAVEnabledCheck);
	m_strAVParamsEdit.EnableWindow(m_bAVEnabledCheck);
	GetDlgItem(IDC_AV_SCANCOMPLETED_CHECK)->EnableWindow(m_bAVEnabledCheck);
	m_AVBrowseButton.EnableWindow(m_bAVEnabledCheck);

	SetModified();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::OnObfuscatedDisabledChange()
{
	BOOL	bVal = (IsDlgButtonChecked(IDC_DISABLEOBFUSCATION) == BST_UNCHECKED) ? TRUE : FALSE;

	GetDlgItem(IDC_ENABLEOBFUSCATION)->EnableWindow(bVal);
	if (bVal)
		OnObfuscatedRequestedChange();
	else
		GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow(FALSE);
	SetModified();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CPPgSecurity::OnObfuscatedRequestedChange()
{
	GetDlgItem(IDC_ONLYOBFUSCATED)->EnableWindow((IsDlgButtonChecked(IDC_ENABLEOBFUSCATION) == BST_UNCHECKED) ? FALSE : TRUE);
	SetModified();
}
