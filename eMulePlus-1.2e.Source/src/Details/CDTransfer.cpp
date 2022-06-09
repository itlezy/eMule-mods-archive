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
#include "..\emule.h"
#include "..\updownclient.h"
#include "CDTransfer.h"
#include "..\KnownFile.h"
#include "..\SharedFileList.h"

IMPLEMENT_DYNCREATE(CCDTransfer, CPropertyPage)

CCDTransfer::CCDTransfer() : CPropertyPage(CCDTransfer::IDD)
{
}

CCDTransfer::~CCDTransfer()
{
}

void CCDTransfer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CURRENTDOWN_VAL, m_ctrlCurrentDownload);
	DDX_Control(pDX, IDC_SESSIONDOWN_VAL, m_ctrlDownloadedSession);
	DDX_Control(pDX, IDC_TOTALDOWN_VAL, m_ctrlDownloadedTotal);
	DDX_Control(pDX, IDC_AVERAGEDOWNRATE_VAL, m_ctrlAverageDownloadRate);
	DDX_Control(pDX, IDC_SESSIONUP_VAL, m_ctrlUploadedSession);
	DDX_Control(pDX, IDC_TOTALUP_VAL, m_ctrlUploadedTotal);
	DDX_Control(pDX, IDC_AVERAGEUPRATE_VAL, m_ctrlAverageUploadRate);
}


BEGIN_MESSAGE_MAP(CCDTransfer, CPropertyPage)
END_MESSAGE_MAP()

BOOL CCDTransfer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Localize();
	Update();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCDTransfer::Update(void)
{
	EMULE_TRY

	if ((m_pClient == NULL) || !::IsWindow(GetSafeHwnd()))
		return;

	CKnownFile* pFile = g_App.m_pSharedFilesList->GetFileByID(m_pClient->m_reqFileHash);
	if (pFile != NULL)
		m_ctrlCurrentDownload.SetWindowText(pFile->GetFileName());
	else
		m_ctrlCurrentDownload.SetWindowText(_T("-"));

	CString strBuffer;

	// up = down & down = up ;)
	m_ctrlDownloadedSession.SetWindowText(::CastItoXBytes(m_pClient->GetTransferredUp()));
	strBuffer.Format(_T("%.1f %s"), static_cast<double>(m_pClient->GetUpDataRate()) / 1024,
						::GetResString(IDS_KBYTESEC));
	m_ctrlAverageDownloadRate.SetWindowText(strBuffer);
	
	m_ctrlUploadedSession.SetWindowText(::CastItoXBytes(m_pClient->GetTransferredDown()));
	strBuffer.Format(_T("%.1f %s"), (double)m_pClient->GetDownloadDataRate() / 1024,
						::GetResString(IDS_KBYTESEC));
	m_ctrlAverageUploadRate.SetWindowText(strBuffer);

	if(m_pClient->Credits())
	{
		m_ctrlDownloadedTotal.SetWindowText(::CastItoXBytes(m_pClient->Credits()->GetUploadedTotal()));
		m_ctrlUploadedTotal.SetWindowText(::CastItoXBytes(m_pClient->Credits()->GetDownloadedTotal()));
	}
	else
	{
		m_ctrlDownloadedTotal.SetWindowText(_T("?"));
		m_ctrlUploadedTotal.SetWindowText(_T("?"));
	}

	EMULE_CATCH
}

void CCDTransfer::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_CURRENTDOWN_LBL, IDS_CD_CDOWN },
		{ IDC_SESSIONDOWN_LBL, IDS_CD_DOWN },
		{ IDC_TOTALDOWN_LBL, IDS_CD_TDOWN },
		{ IDC_AVERAGEDOWNRATE_LBL, IDS_CD_ADOWN },
		{ IDC_SESSIONUP_LBL, IDS_CD_UP },
		{ IDC_TOTALUP_LBL, IDS_CD_TUP },
		{ IDC_AVERAGEUPRATE_LBL, IDS_CD_AUP }
	};

	EMULE_TRY

	if (GetSafeHwnd())
	{
		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
			SetDlgItemText(s_auResTbl[i][0], GetResString(static_cast<UINT>(s_auResTbl[i][1])));
	}

	EMULE_CATCH
}
