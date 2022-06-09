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
#include "CDScores.h"
#include "..\updownclient.h"
#include "..\emule.h"


IMPLEMENT_DYNCREATE(CCDScores, CPropertyPage)

CCDScores::CCDScores() : CPropertyPage(CCDScores::IDD)
{
}

CCDScores::~CCDScores()
{
}

void CCDScores::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCORES_LBL, m_ctrlScores);
	DDX_Control(pDX, IDC_REMOTESCORES_LBL, m_ctrlRemoteScores);
	DDX_Control(pDX, IDC_DLUPMODIFIER_VAL, m_ctrlDlUpModifier);
	DDX_Control(pDX, IDC_COMMUNITYUSER_VAL, m_ctrlCommunityUser);
	DDX_Control(pDX, IDC_RATING_VAL, m_ctrlRating);
	DDX_Control(pDX, IDC_UPQUEUESCORE_VAL, m_ctrlUpQueueScore);
	DDX_Control(pDX, IDC_QUEUETIME_VAL, m_ctrlQueueTime);
	DDX_Control(pDX, IDC_REMDLUPMODIFIER_VAL, m_ctrlRemDlUpModifier);
	DDX_Control(pDX, IDC_REMRATING_VAL, m_ctrlRemRating);
}


BEGIN_MESSAGE_MAP(CCDScores, CPropertyPage)
END_MESSAGE_MAP()


BOOL CCDScores::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Localize();
	Update();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCDScores::Update(void)
{
	EMULE_TRY

	if ((m_pClient == NULL) || !::IsWindow(GetSafeHwnd()))
		return;

	CString		strBuffer = GetResString(IDS_DISABLED);

	if (g_App.m_pPrefs->CommunityEnabled())
		strBuffer = YesNoStr(m_pClient->IsCommunity());
	m_ctrlCommunityUser.SetWindowText(strBuffer);

	if (m_pClient->Credits())
	{
		strBuffer.Format(_T("%.1f"), (double)m_pClient->Credits()->GetScoreRatio(m_pClient->GetIP()));
		m_ctrlDlUpModifier.SetWindowText(strBuffer);

		strBuffer.Format(_T("%.1f"), m_pClient->GetRemoteBaseModifier());
		m_ctrlRemDlUpModifier.SetWindowText(strBuffer);

		strBuffer.Format(_T("%u"), m_pClient->GetRemoteRatio());
		m_ctrlRemRating.SetWindowText(strBuffer);
	}
	else
	{
		m_ctrlDlUpModifier.SetWindowText(_T("?"));
		m_ctrlRemDlUpModifier.SetWindowText(_T("?"));
		m_ctrlRemRating.SetWindowText(_T("?"));
	}

	bool	bLoadedSourceName = (_tcsstr(m_pClient->GetUserName(), ::GetResString(IDS_SAVED_SOURCE)) != NULL) ||
		(_tcsstr(m_pClient->GetUserName(), ::GetResString(IDS_EXCHANGEDSOURCE)) != NULL);

	if (!bLoadedSourceName)
	{
		if (m_pClient->IsDownloading())
			strBuffer = _T("-");
		else
			strBuffer.Format(_T("%u"), m_pClient->GetScore(true));
		m_ctrlRating.SetWindowText(strBuffer);
	}
	else
		m_ctrlRating.SetWindowText(_T("?"));

	if (m_pClient->GetUploadState() != US_NONE && !m_pClient->IsDownloading())
	{
		strBuffer.Format(_T("%u"), m_pClient->GetScore(false));
		m_ctrlUpQueueScore.SetWindowText(strBuffer);
	}
	else
		m_ctrlUpQueueScore.SetWindowText(_T("-"));

	if (m_pClient->IsInWaitingQueue())
		m_ctrlQueueTime.SetWindowText(::CastSecondsToHM((::GetTickCount() - m_pClient->GetWaitStartTime())/1000));
	else
		m_ctrlQueueTime.SetWindowText(_T("-"));

	EMULE_CATCH
}

void CCDScores::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_SCORES_LBL, IDS_CD_SCORES },
		{ IDC_DLUPMODIFIER_LBL, IDS_CD_MOD },
		{ IDC_UPQUEUESCORE_LBL, IDS_CD_USCORE },
		{ IDC_REMOTESCORES_LBL, IDS_INFLST_REMOTE_SCORES },
		{ IDC_REMDLUPMODIFIER_LBL, IDS_CD_MOD }
	};
	static const uint16 s_auResTbl2[][2] =
	{
		{ IDC_COMMUNITYUSER_LBL, IDS_COMMUNITY },
		{ IDC_RATING_LBL, IDS_RATING },
		{ IDC_REMRATING_LBL, IDS_RATING },
		{ IDC_QUEUETIME_LBL, IDS_WAITED }
	};

	if (GetSafeHwnd())
	{
		CString strBuffer;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strBuffer);
		}

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl2); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl2[i][1]));
			strBuffer += _T(":");
			SetDlgItemText(s_auResTbl2[i][0], strBuffer);
		}
	}
}
