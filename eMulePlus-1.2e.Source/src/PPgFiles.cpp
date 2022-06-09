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
#include "PPgFiles.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CPPgFiles dialog

IMPLEMENT_DYNAMIC(CPPgFiles, CPropertyPage)
CPPgFiles::CPPgFiles()
	: CPropertyPage(CPPgFiles::IDD)
	, m_bDLAutoPrio(FALSE)
	, m_bUpAutoPrio(FALSE)
	, m_bStartPaused(FALSE)
	, m_bStartPausedOnComplete(FALSE)
	, m_bResumeOtherCat(FALSE)
	, m_iSeeShare(0)
	, m_iFilePermission(0)
	, m_iHashPriority(0)
	, m_iFileBufferSize(0)
	, m_bAutoClearCompleted(FALSE)
{
}

CPPgFiles::~CPPgFiles()
{
}

void CPPgFiles::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_DAP_ENABLE, m_bDLAutoPrio);
	DDX_Check(pDX, IDC_UAP, m_bUpAutoPrio);
	DDX_Check(pDX, IDC_START_PAUSED, m_bStartPaused);
	DDX_Check(pDX, IDC_START_PAUSED_ON_COMPLETE, m_bStartPausedOnComplete);
	DDX_Check(pDX, IDC_RESUME_OTHER_CAT, m_bResumeOtherCat);
	DDX_Control(pDX, IDC_FILEBUFFERSIZE, m_FileBufferSizeSlider);
	DDX_Control(pDX, IDC_SLOWCOMPLETE_BLOCK, m_FileCopySizeSlider);
	DDX_Text(pDX, IDC_AP_HIGH, m_strPrioHigh);
	DDX_Text(pDX, IDC_AP_LOW, m_strPrioLow);
	DDX_Radio(pDX, IDC_SEESHARE1, m_iSeeShare);
	DDX_Radio(pDX, IDC_PERM_PUBLIC, m_iFilePermission);
	DDX_Radio(pDX, IDC_HASHPRIO_STANDARD, m_iHashPriority);
	DDX_Slider(pDX, IDC_FILEBUFFERSIZE, m_iFileBufferSize);
	DDX_Slider(pDX, IDC_SLOWCOMPLETE_BLOCK, m_iCopyBufferSz);
	DDX_Check(pDX, IDC_AUTO_CLEAR_COMPLETED, m_bAutoClearCompleted);
}

BEGIN_MESSAGE_MAP(CPPgFiles, CPropertyPage)
	ON_BN_CLICKED(IDC_SEESHARE1, OnSettingsChange)
	ON_BN_CLICKED(IDC_SEESHARE2, OnSettingsChange)
	ON_BN_CLICKED(IDC_SEESHARE3, OnSettingsChange)
	ON_BN_CLICKED(IDC_PERM_PUBLIC, OnSettingsChange)
	ON_BN_CLICKED(IDC_PERM_FRIENDS_ONLY, OnSettingsChange)
	ON_BN_CLICKED(IDC_PERM_HIDDEN, OnSettingsChange)
	ON_BN_CLICKED(IDC_HASHPRIO_STANDARD, OnSettingsChange)
	ON_BN_CLICKED(IDC_HASHPRIO_LOWER, OnSettingsChange)
	ON_BN_CLICKED(IDC_HASHPRIO_IDLE, OnSettingsChange)
	ON_EN_CHANGE(IDC_AP_HIGH, OnSettingsChange)
	ON_EN_CHANGE(IDC_AP_LOW, OnSettingsChange)
	ON_BN_CLICKED(IDC_UAP, OnSettingsChange)
	ON_BN_CLICKED(IDC_DAP_ENABLE, OnBnClickedDAP)
	ON_BN_CLICKED(IDC_START_PAUSED, OnSettingsChange)
	ON_BN_CLICKED(IDC_START_PAUSED_ON_COMPLETE, OnBnClickedStartPaused)
	ON_BN_CLICKED(IDC_RESUME_OTHER_CAT, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTO_CLEAR_COMPLETED, OnSettingsChange)
	ON_WM_HSCROLL()
END_MESSAGE_MAP()

BOOL CPPgFiles::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgFiles::LoadSettings(void)
{
	switch(m_pPrefs->CanSeeShares())
	{
		case SEE_SHARE_EVERYBODY:
			m_iSeeShare = 0;
			break;
		case SEE_SHARE_FRIENDS:
			m_iSeeShare = 1;
			break;
		case SEE_SHARE_NOONE:
		default:
			m_iSeeShare = 2;
			break;
	}

	switch(m_pPrefs->GetHashingPriority())
	{
		case HASH_PRIORITY_IDLE:
			m_iHashPriority = 2;
			break;
		case HASH_PRIORITY_LOWER:
			m_iHashPriority = 1;
			break;
		case HASH_PRIORITY_STANDARD:
		default:
			m_iHashPriority = 0;
			break;
	}

	m_iFilePermission = static_cast<int>(m_pPrefs->GetFilePermission());

	m_bUpAutoPrio = m_pPrefs->IsUAPEnabled();
	m_bDLAutoPrio = m_pPrefs->IsDAPEnabled();

	m_strPrioHigh.Format(_T("%u"), m_pPrefs->PriorityHigh());
	m_strPrioLow.Format(_T("%u"), m_pPrefs->PriorityLow());

	m_bStartPaused = m_pPrefs->StartDownloadPaused();
	m_bStartPausedOnComplete = m_pPrefs->DownloadPausedOnComplete();
	m_bResumeOtherCat = m_pPrefs->IsResumeOtherCat();
	m_bAutoClearCompleted = m_pPrefs->IsAutoClearCompleted();

	m_FileBufferSizeSlider.SetRange(1, PARTSZ32 / (32 * 1024) + 1, true);
	m_iFileBufferSize = m_pPrefs->GetFileBufferSize() / (32u * 1024u);

//	Scale step is 4 KB
	m_FileCopySizeSlider.SetRange(PREF_MIN_COMPLETEBLOCKSZ / 4, PREF_MAX_COMPLETEBLOCKSZ / 4, true);
	m_iCopyBufferSz = static_cast<unsigned>(m_pPrefs->SlowCompleteBlockSize() / 4);

	UpdateData(FALSE);

	OnBnClickedDAP();
	OnBnClickedStartPaused();

	SetModified(FALSE);
}

BOOL CPPgFiles::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		uint32	dwVal, dwVal2;

		switch (m_iSeeShare)
		{
			case 0:
				m_pPrefs->SetCanSeeShares(SEE_SHARE_EVERYBODY);
				break;
			case 1:
				m_pPrefs->SetCanSeeShares(SEE_SHARE_FRIENDS);
				break;
			case 2:
			default:
				m_pPrefs->SetCanSeeShares(SEE_SHARE_NOONE);
				break;
		}

		switch (m_iHashPriority)
		{
			case 2:
				m_pPrefs->SetHashingPriority(HASH_PRIORITY_IDLE);
				g_App.m_fileHashControl.SetThreadPriority(THREAD_PRIORITY_IDLE);
				break;
			case 1:
				m_pPrefs->SetHashingPriority(HASH_PRIORITY_LOWER);
				g_App.m_fileHashControl.SetThreadPriority(THREAD_PRIORITY_LOWEST);
				break;
			case 0:
			default:
				m_pPrefs->SetHashingPriority(HASH_PRIORITY_STANDARD);
				g_App.m_fileHashControl.SetThreadPriority(THREAD_PRIORITY_BELOW_NORMAL);
				break;
		}
		
		m_pPrefs->SetFilePermission(static_cast<byte>(m_iFilePermission));
		m_pPrefs->SetUAPEnabled(B2b(m_bUpAutoPrio));
		m_pPrefs->SetDAPEnabled(B2b(m_bDLAutoPrio));
		m_pPrefs->SetStartDownloadPaused(B2b(m_bStartPaused));
		m_pPrefs->SetDownloadPausedOnComplete(B2b(m_bStartPausedOnComplete));
		m_pPrefs->SetResumeOtherCat(B2b(m_bResumeOtherCat));
		m_pPrefs->SetAutoClearCompleted(B2b(m_bAutoClearCompleted));

		dwVal = _tstoi(m_strPrioHigh);
		dwVal2 = _tstoi(m_strPrioLow);
		if (dwVal2 < dwVal)
		{
			dwVal2 = (dwVal * 2) & 0xFFFF;
			if (dwVal2 <= dwVal)	// high value is too big
				dwVal = 0;
		}
		if (dwVal == 0)
		{
			dwVal = PREF_DEF_DLSRC4HIGHPRIO;
			dwVal2 = PREF_DEF_DLSRC4LOWPRIO;
		}
		m_pPrefs->SetPriorityHigh(static_cast<uint16>(dwVal));
		m_pPrefs->SetPriorityLow(static_cast<uint16>(dwVal2));
		m_strPrioHigh.Format(_T("%u"), dwVal);
		m_strPrioLow.Format(_T("%u"), dwVal2);

		m_pPrefs->SetFileBufferSize(static_cast<unsigned>(m_iFileBufferSize) * 32u * 1024u);
		m_pPrefs->SetSlowCompleteBlockSize(static_cast<unsigned>(m_iCopyBufferSz) * 4u);

		UpdateData(FALSE);
		SetModified(FALSE);
	}
	return CPropertyPage::OnApply();
}

void CPPgFiles::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_SEEMYSHARE_FRM, IDS_PW_SHARE },
		{ IDC_SEESHARE1, IDS_PW_EVER },
		{ IDC_SEESHARE2, IDS_PW_FRIENDS },
		{ IDC_SEESHARE3, IDS_PW_NOONE },
		{ IDC_PERMISSION_FRM, IDS_DEF_PERMISSION },
		{ IDC_PERM_HIDDEN, IDS_HIDDEN },
		{ IDC_PERM_FRIENDS_ONLY, IDS_FSTATUS_FRIENDSONLY },
		{ IDC_PERM_PUBLIC, IDS_FSTATUS_PUBLIC },
		{ IDC_AP_GRP, IDS_AP_GRP },
		{ IDC_AP_HIGH_LBL, IDS_AP_HIGH_LBL },
		{ IDC_AP_LOW_LBL, IDS_AP_LOW_LBL },
		{ IDC_DAP_ENABLE, IDS_ENABLED },
		{ IDC_UAP, IDS_UAP },
		{ IDC_START_PAUSED, IDS_START_PAUSED },
		{ IDC_START_PAUSED_ON_COMPLETE, IDS_START_PAUSED_ON_COMPLETE },
		{ IDC_RESUME_OTHER_CAT, IDS_RESUME_OTHER_CAT },
		{ IDC_AP_SOURCES1, IDS_SOURCES_PF_FILES },
		{ IDC_AP_SOURCES2, IDS_SOURCES_PF_FILES },
		{ IDC_HASHING_FRM, IDS_HASHING_FRM },
		{ IDC_HASHPRIO_STANDARD, IDS_HASHPRIO_STANDARD },
		{ IDC_HASHPRIO_LOWER, IDS_HASHPRIO_LOWER },
		{ IDC_HASHPRIO_IDLE, IDS_HASHPRIO_IDLE },
		{ IDC_AUTO_CLEAR_COMPLETED, IDS_AUTO_CLEAR_COMPLETED }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
		strRes.Format(GetResString(IDS_FILEBUFFERSIZE), static_cast<unsigned>(m_iFileBufferSize) * 32u);
		SetDlgItemText(IDC_FILEBUFFERSIZE_LBL, strRes);
		strRes.Format(GetResString(IDS_SLOWCOMPLETE), static_cast<unsigned>(m_iCopyBufferSz) * 4u);
		SetDlgItemText(IDC_SLOWCOMPLETE, strRes);
	}
}

void CPPgFiles::OnBnClickedDAP()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_AP_HIGH)->EnableWindow(m_bDLAutoPrio);
	GetDlgItem(IDC_AP_LOW)->EnableWindow(m_bDLAutoPrio);

	SetModified();
}

void CPPgFiles::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_FileBufferSizeSlider)
	{
		SetModified(TRUE);
		UpdateData(TRUE);

		CString	strRes;

		strRes.Format(GetResString(IDS_FILEBUFFERSIZE), static_cast<unsigned>(m_iFileBufferSize) * 32u);
		SetDlgItemText(IDC_FILEBUFFERSIZE_LBL, strRes);
	}
	else if (pScrollBar == (CScrollBar*)&m_FileCopySizeSlider)
	{
		SetModified(TRUE);
		UpdateData(TRUE);

		CString	strRes;

		strRes.Format(GetResString(IDS_SLOWCOMPLETE), static_cast<unsigned>(m_iCopyBufferSz) * 4u);
		SetDlgItemText(IDC_SLOWCOMPLETE, strRes);
	}
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgFiles::OnBnClickedStartPaused()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_RESUME_OTHER_CAT)->EnableWindow(m_bStartPausedOnComplete);

	SetModified();
}
