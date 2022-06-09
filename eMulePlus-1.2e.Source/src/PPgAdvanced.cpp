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
#include "PPgAdvanced.h"
#include "Inputbox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgAdvanced, CPropertyPage)
CPPgAdvanced::CPPgAdvanced()
	: CPropertyPage(CPPgAdvanced::IDD)
	, m_bBanMessage(FALSE)
	, m_bBanEnabled(FALSE)
	, m_bNoBanEnabled(FALSE)
	, m_bComEnabled(FALSE)
	, m_bSlsEnabled(FALSE)
	, m_bFNameCleanup(FALSE)
	, m_bFNameCleanupTag(FALSE)
	, m_bCountermeasures(FALSE)
	, m_bAutoNewVerChk(FALSE)
{
}

CPPgAdvanced::~CPPgAdvanced()
{
}

void CPPgAdvanced::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_BANTIMES, m_strBanTimes);
	DDX_Text(pDX, IDC_MINREQUESTTIME, m_strMinRequestTime);
	DDX_Text(pDX, IDC_BANTIMEINMINS, m_strBanTimeInMins);
	DDX_Check(pDX, IDC_BANMESSAGE, m_bBanMessage);
	DDX_Check(pDX, IDC_BANENABLED, m_bBanEnabled);
	DDX_Text(pDX, IDC_COMSTRING, m_strComString);
	DDX_Check(pDX, IDC_NOBANENABLED, m_bNoBanEnabled);
	DDX_Check(pDX, IDC_COMENABLED, m_bComEnabled);
	DDX_Text(pDX, IDC_SLSMAXSOURCES, m_strSlsMaxSources);
	DDX_Text(pDX, IDC_OUTDATED, m_strOutdated);
	DDX_Check(pDX, IDC_SLSENABLED, m_bSlsEnabled);
	DDX_Check(pDX, IDC_FNCLEANUP, m_bFNameCleanup);
	DDX_Check(pDX, IDC_FNCLEANUPTAG, m_bFNameCleanupTag);
	DDX_Check(pDX, IDC_COUNTERMEASURES, m_bCountermeasures);
	DDX_Check(pDX, IDC_AUTO_CHECK_NEW, m_bAutoNewVerChk);
}

BEGIN_MESSAGE_MAP(CPPgAdvanced, CPropertyPage)
	ON_BN_CLICKED(IDC_BANENABLED, OnBnClickedBanEnabled)
	ON_EN_CHANGE(IDC_BANTIMES, OnSettingsChange)
	ON_EN_CHANGE(IDC_MINREQUESTTIME, OnSettingsChange)
	ON_EN_CHANGE(IDC_BANTIMEINMINS, OnSettingsChange)
	ON_BN_CLICKED(IDC_BANMESSAGE, OnSettingsChange)
	ON_BN_CLICKED(IDC_COMENABLED, OnBnClickedComEnabled)
	ON_EN_CHANGE(IDC_COMSTRING, OnSettingsChange)
	ON_BN_CLICKED(IDC_NOBANENABLED, OnSettingsChange)
	ON_EN_CHANGE(IDC_SLSMAXSOURCES, OnSettingsChange)
	ON_EN_CHANGE(IDC_OUTDATED, OnSettingsChange)
	ON_BN_CLICKED(IDC_SLSENABLED, OnBnClickedSLSEnabled)
	ON_BN_CLICKED(IDC_COUNTERMEASURES, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTO_CHECK_NEW, OnSettingsChange)
	ON_BN_CLICKED(IDC_FNCLEANUP, OnSettingsChange)
	ON_BN_CLICKED(IDC_FNCLEANUPTAG, OnSettingsChange)
	ON_BN_CLICKED(IDC_FNC, OnSetCleanupFilter)
END_MESSAGE_MAP()

BOOL CPPgAdvanced::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgAdvanced::LoadSettings(void)
{
	m_strBanTimes.Format(_T("%u"), m_pPrefs->BadClientMinRequestNum());
	m_strBanTimeInMins.Format(_T("%u"), m_pPrefs->BadClientBanTime() / 60000);
	
//	Don't add "mins" into textbox (don't use CastSecondsToHM)
	uint32		dwSecs = m_pPrefs->BadClientMinRequestTime() / 1000;

	m_strMinRequestTime.Format(_T("%u:%02u"), dwSecs / 60, dwSecs - (dwSecs / 60) * 60);
	m_bBanMessage = m_pPrefs->IsBanMessageEnabled();
	m_bBanEnabled = m_pPrefs->BanEnabled();
	m_strComString = m_pPrefs->CommunityString();
	m_bComEnabled = m_pPrefs->CommunityEnabled();
	m_bNoBanEnabled = m_pPrefs->CommunityNoBanEnabled();
	m_strSlsMaxSources.Format(_T("%u"), m_pPrefs->SLSMaxSourcesPerFile());
	m_strOutdated.Format(_T("%u"), m_pPrefs->WhenSourcesOutdated());
	m_bSlsEnabled = m_pPrefs->SLSEnable();
	m_bCountermeasures = m_pPrefs->IsCounterMeasures();
	m_bAutoNewVerChk = m_pPrefs->IsAutoCheckForNewVersion();
	m_bFNameCleanup = m_pPrefs->GetAutoFilenameCleanup();
	m_bFNameCleanupTag = m_pPrefs->GetFilenameCleanupTags();

	UpdateData(FALSE);

	OnBnClickedBanEnabled();
	OnBnClickedComEnabled();
	OnBnClickedSLSEnabled();

	SetModified(FALSE);
}

BOOL CPPgAdvanced::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);
		
		uint32	dwTmp;
		
		dwTmp = static_cast<uint32>(_tstoi(m_strBanTimes));
		if (dwTmp < PREF_MIN_BANTIMES)
			dwTmp = PREF_MIN_BANTIMES;
		if (dwTmp > PREF_MAX_BANTIMES)
			dwTmp = PREF_MAX_BANTIMES;
		m_pPrefs->SetBadClientMinRequestNum(static_cast<byte>(dwTmp));
		m_strBanTimes.Format(_T("%u"), dwTmp);
			
		dwTmp = static_cast<uint32>(_tstoi(m_strBanTimeInMins));
		if (dwTmp > PREF_MAX_MINS2BAN)
			dwTmp = PREF_MAX_MINS2BAN;
		if (dwTmp < PREF_MIN_MINS2BAN)
			dwTmp = PREF_MIN_MINS2BAN;
		m_pPrefs->SetBadClientBanTime(MIN2MS(dwTmp));
		m_strBanTimeInMins.Format(_T("%u"), dwTmp);

		m_pPrefs->SetBanMessageEnabled(B2b(m_bBanMessage));

		uint32	dwSec, dwMin = static_cast<uint32>(_tstoi(m_strMinRequestTime));

		CString strBuffer(m_strMinRequestTime);

		strBuffer.Replace(_T(":"), GetLocalDecimalPoint());
		if ((dwSec = static_cast<uint32>(_tstof(strBuffer) * 100.0 + .05) - dwMin * 100) > 59)
			dwSec = 59;
		if ((dwMin > 9) || ((dwMin == 9) && (dwSec > 50)))
		{
			dwMin = 9;
			dwSec = 50;
		}
		if (dwMin < 2)
		{
			dwMin = 2;
			dwSec = 0;
		}
		m_pPrefs->SetBadClientMinRequestTime((dwMin * 60 + dwSec) * 1000);
	//	Don't add "mins" for textbox (don't use CastSecondsToHM)
		m_strMinRequestTime.Format(_T("%u:%02u"), dwMin, dwSec);
		
		m_pPrefs->SetBanEnable(B2b(m_bBanEnabled));
		m_pPrefs->SetCommunityString(m_strComString);
		m_pPrefs->SetCommunityEnabled(B2b(m_bComEnabled));
		m_pPrefs->SetCommunityNoBanEnabled(B2b(m_bNoBanEnabled));

		dwTmp = static_cast<uint32>(_tstoi(m_strSlsMaxSources));
		if (dwTmp > PREF_MAX_MAXFILESLS)
			dwTmp = PREF_MAX_MAXFILESLS;
		if (dwTmp < PREF_MIN_MAXFILESLS)
			dwTmp = PREF_MIN_MAXFILESLS;
		m_pPrefs->SetSLSMaxSourcesPerFile(dwTmp);
		m_strSlsMaxSources.Format(_T("%u"), dwTmp);

		dwTmp = static_cast<uint32>(_tstoi(m_strOutdated));
		if (dwTmp > PREF_MAX_FILESLSDAYS)
			dwTmp = PREF_MAX_FILESLSDAYS;
		if (dwTmp < PREF_MIN_FILESLSDAYS)
			dwTmp = PREF_MIN_FILESLSDAYS;
		m_pPrefs->SetWhenSourcesOutdated(dwTmp);
		m_strOutdated.Format(_T("%u"), dwTmp);

		m_pPrefs->SetSLSEnable(B2b(m_bSlsEnabled));
		m_pPrefs->SetCounterMeasures(B2b(m_bCountermeasures));
		m_pPrefs->SetAutoCheckForNewVersion(B2b(m_bAutoNewVerChk));
		m_pPrefs->SetAutoFilenameCleanup(B2b(m_bFNameCleanup));
		m_pPrefs->SetFilenameCleanupTags(B2b(m_bFNameCleanupTag));

		UpdateData(FALSE);
		SetModified(FALSE);
	}
	return CPropertyPage::OnApply();
}

void CPPgAdvanced::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_BAN, IDS_BAN },
		{ IDC_BANENABLED, IDS_ENABLED },
		{ IDC_CLIENTREQUESTLBL, IDS_CLIENTREQUESTLBL },
		{ IDC_BANFORLABEL, IDS_BANFORLABEL },
		{ IDC_TIMESINLBL, IDS_TIMESINLBL },
		{ IDC_MINUTES_BAN1, IDS_MINS },
		{ IDC_MINUTES_BAN2, IDS_MINS },
		{ IDC_COM, IDS_COM },
		{ IDC_COMENABLED, IDS_ENABLED },
		{ IDC_NOBANENABLED, IDS_NOBANENABLED },
		{ IDC_BANMESSAGE, IDS_BANMESSAGE },
		{ IDC_SAVELOADSOURCESGROUPBOX, IDS_SAVELOADSOURCESGROUPBOX },
		{ IDC_SLSMAXSOURCESLBL, IDS_PW_MAXSOURCES },
		{ IDC_SLS_OUTDATED, IDS_SLS_OUTDATED },
		{ IDC_SLS_SOURCES, IDS_SOURCES_PF_FILES },
		{ IDC_SLS_DAYS, IDS_LONGDAYS },
		{ IDC_SLSENABLED, IDS_ENABLED },
		{ IDC_COUNTERMEASURES, IDS_COUNTERMEASURES },
		{ IDC_AUTO_CHECK_NEW, IDS_AUTO_CHECK_NEW },
		{ IDC_FNCLEANUPGROUPBOX, IDS_FNCLEANUPGROUPBOX },
		{ IDC_FNC, IDS_FNCEDIT },
		{ IDC_FNCLEANUP, IDS_AUTOCLEANUPFN },
		{ IDC_FNCLEANUPTAG, IDS_CLEANUPTAGS }
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

void CPPgAdvanced::OnBnClickedComEnabled()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_COMSTRING)->EnableWindow(m_bComEnabled);
	GetDlgItem(IDC_NOBANENABLED)->EnableWindow(m_bComEnabled);

	SetModified();
}

void CPPgAdvanced::OnBnClickedSLSEnabled()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_SLSMAXSOURCES)->EnableWindow(m_bSlsEnabled);
	GetDlgItem(IDC_OUTDATED)->EnableWindow(m_bSlsEnabled);

	SetModified();
}

void CPPgAdvanced::OnSetCleanupFilter()
{
	InputBox inputbox(GetResString(IDS_FNFILTERTITLE), g_App.m_pPrefs->GetFilenameCleanups());

	inputbox.DoModal();
	if (!inputbox.WasCancelled())
	{
		g_App.m_pPrefs->SetFilenameCleanups(inputbox.GetInput());
		SetModified();
	}
}

void CPPgAdvanced::OnBnClickedBanEnabled()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_BANTIMES)->EnableWindow(m_bBanEnabled);
	GetDlgItem(IDC_MINREQUESTTIME)->EnableWindow(m_bBanEnabled);
	GetDlgItem(IDC_BANTIMEINMINS)->EnableWindow(m_bBanEnabled);
	GetDlgItem(IDC_BANMESSAGE)->EnableWindow(m_bBanEnabled);
	
	SetModified();
}
