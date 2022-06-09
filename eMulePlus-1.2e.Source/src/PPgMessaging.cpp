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
#include "PPgMessaging.h"

IMPLEMENT_DYNAMIC(CPPgMessaging, CPropertyPage)

CPPgMessaging::CPPgMessaging()
	: CPropertyPage(CPPgMessaging::IDD)
	, m_iAcceptMessages(0)
	, m_bPutMeInAwayState(FALSE)
{	
}

CPPgMessaging::~CPPgMessaging()
{}

void CPPgMessaging::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RB_IM_FROMALLUSERS, m_iAcceptMessages);
	DDX_Check(pDX, IDC_CB_IM_PUTMEAWAY, m_bPutMeInAwayState);
	DDX_Text(pDX, IDC_EDIT_IM_AWAYMESSAGE, m_strAwayMessage);
	DDX_Text(pDX, IDC_FILTER, m_strMsgFilter);
	DDX_Text(pDX, IDC_FILTER2, m_strCommentFilter);
}

void CPPgMessaging::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_GB_IM_ACCEPT, IDS_PW_IM_ACCEPT },
		{ IDC_GB_IM_OTHER, IDS_PW_IM_OTHER },
		{ IDC_RB_IM_FROMALLUSERS, IDS_PW_IM_FROMALLUSERS },
		{ IDC_RB_IM_ONLYFROMFRIENDS, IDS_PW_IM_ONLYFROMFRIENDS },
		{ IDC_RB_IM_ONLYFROMFRIENDS2, IDS_PW_IM_ONLYFROMFRIENDS2 },
		{ IDC_RB_IM_FROMNONE, IDS_PW_IM_FROMNONE },
		{ IDC_CB_IM_PUTMEAWAY, IDS_PW_IM_PUTMEAWAY },
		{ IDC_LBL_IM_AWAYMESSAGE, IDS_PW_IM_AWAYMESSAGE },
		{ IDC_FILTERLABEL, IDS_FILTERLABEL },
		{ IDC_MSG, IDS_MESSAGES },
		{ IDC_FILTERLABEL2, IDS_FILTERLABEL2 },
		{ IDC_CMT, IDS_COMMENTS }
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

void CPPgMessaging::LoadSettings(void)
{
	m_iAcceptMessages = m_pPrefs->GetAcceptMessagesFrom() - 1;
	m_bPutMeInAwayState = m_pPrefs->GetAwayState();
	m_strAwayMessage = m_pPrefs->GetAwayStateMessage();
	GetDlgItem(IDC_EDIT_IM_AWAYMESSAGE)->EnableWindow(m_bPutMeInAwayState);
	m_strMsgFilter = m_pPrefs->GetMessageFilter();
	m_strCommentFilter = m_pPrefs->GetCommentFilter();

	UpdateData(FALSE);
}

BEGIN_MESSAGE_MAP(CPPgMessaging, CPropertyPage)
	ON_BN_CLICKED(IDC_RB_IM_FROMALLUSERS, OnSettingsChange)
	ON_BN_CLICKED(IDC_RB_IM_ONLYFROMFRIENDS, OnSettingsChange)
	ON_BN_CLICKED(IDC_RB_IM_ONLYFROMFRIENDS2, OnSettingsChange)
	ON_BN_CLICKED(IDC_RB_IM_FROMNONE, OnSettingsChange)
	ON_BN_CLICKED(IDC_CB_IM_PUTMEAWAY, OnBnClickedCbImPutmeaway)
	ON_EN_CHANGE(IDC_EDIT_IM_AWAYMESSAGE, OnSettingsChange)
	ON_EN_CHANGE(IDC_FILTER, OnSettingsChange)
	ON_EN_CHANGE(IDC_FILTER2, OnSettingsChange)
END_MESSAGE_MAP()


BOOL CPPgMessaging::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}

BOOL CPPgMessaging::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		m_pPrefs->SetAcceptMessagesFrom(m_iAcceptMessages + 1);
		m_pPrefs->SetAwayState(B2b(m_bPutMeInAwayState));
		m_pPrefs->SetAwayStateMessage(m_strAwayMessage);
		m_pPrefs->SetMessageFilter(m_strMsgFilter);
		m_pPrefs->SetCommentFilter(m_strCommentFilter);

		SetModified(FALSE);
	}
	return CPropertyPage::OnApply();
}

void CPPgMessaging::OnBnClickedCbImPutmeaway()
{
	UpdateData(TRUE);
	
	CWnd	*pWnd = GetDlgItem(IDC_EDIT_IM_AWAYMESSAGE);

	pWnd->EnableWindow(m_bPutMeInAwayState);
	if (m_bPutMeInAwayState)
		pWnd->SetFocus();
	SetModified();
}
