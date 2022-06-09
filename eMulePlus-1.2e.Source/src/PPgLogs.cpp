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
#include "PPgLogs.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgLogs, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgLogs, CPropertyPage)
	ON_BN_CLICKED(IDC_LOGTOFILE, OnSettingsChange)
	ON_BN_CLICKED(IDC_VERBOSE, OnChangeDebugLogging)
	ON_BN_CLICKED(IDC_CM_NOTLOG, OnSettingsChange)
	ON_BN_CLICKED(IDC_UPLOAD_LOG, OnSettingsChange)
	ON_BN_CLICKED(IDC_DOWNLOAD_LOG, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOSRC_LOG, OnSettingsChange)
	ON_BN_CLICKED(IDC_CLIENT_LOG, OnSettingsChange)
END_MESSAGE_MAP()

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPPgLogs::CPPgLogs()
	: CPropertyPage(CPPgLogs::IDD)
	, m_bWriteLogToFile(FALSE)
	, m_bDebugLog(FALSE)
	, m_bCmNotLog(FALSE)
	, m_bUploadLog(FALSE)
	, m_bDownloadLog(FALSE)
	, m_bAutoSrcLog(FALSE)
	, m_bClientTransferLog(FALSE)
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CPPgLogs::~CPPgLogs()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgLogs::DoDataExchange(CDataExchange *pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_LOGTOFILE, m_bWriteLogToFile);
	DDX_Check(pDX, IDC_VERBOSE, m_bDebugLog);
	DDX_Check(pDX, IDC_CM_NOTLOG, m_bCmNotLog);
	DDX_Check(pDX, IDC_UPLOAD_LOG, m_bUploadLog);
	DDX_Check(pDX, IDC_DOWNLOAD_LOG, m_bDownloadLog);
	DDX_Check(pDX, IDC_AUTOSRC_LOG, m_bAutoSrcLog);
	DDX_Check(pDX, IDC_CLIENT_LOG, m_bClientTransferLog);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPPgLogs::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	LoadSettings();
	Localize();

	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgLogs::LoadSettings(void)
{
	m_bWriteLogToFile = m_pPrefs->LogToFile();
	m_bDebugLog = m_pPrefs->GetVerbose();
	m_bCmNotLog = m_pPrefs->IsCMNotLog();
	m_bUploadLog = m_pPrefs->LogUploadToFile();
	m_bDownloadLog = m_pPrefs->LogDownloadToFile();
	m_bAutoSrcLog = m_pPrefs->IsAutoSourcesLogEnabled();
	m_bClientTransferLog = m_pPrefs->IsClientTransferLogEnabled();

	UpdateData(FALSE);

	OnChangeDebugLogging();

	SetModified(FALSE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CPPgLogs::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		m_pPrefs->SetLogToFile(B2b(m_bWriteLogToFile));
		if (m_pPrefs->GetVerbose() != B2b(m_bDebugLog))
		{
			m_pPrefs->SetVerbose(B2b(m_bDebugLog));
			g_App.m_pMDlg->m_wndServer.ToggleDebugWindow();
			if (m_bDebugLog)
				g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T("Debug log is active"));
		}
		m_pPrefs->SetCMNotLog(B2b(m_bCmNotLog));
		m_pPrefs->SetLogUploadToFile(B2b(m_bUploadLog));
		m_pPrefs->SetLogDownloadToFile(B2b(m_bDownloadLog));
		m_pPrefs->SetAutoSourcesLog(B2b(m_bAutoSrcLog));
		m_pPrefs->SetClientTransferLog(B2b(m_bClientTransferLog));

		SetModified(FALSE);
	}

	return CPropertyPage::OnApply();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgLogs::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_LOGTOFILE, IDS_LOGTOFILE },
		{ IDC_VERBOSE, IDS_VERBOSE },
		{ IDC_CM_NOTLOG, IDS_CM_NOTLOG },
		{ IDC_UPLOAD_LOG, IDS_UPLOAD_LOG },
		{ IDC_DOWNLOAD_LOG, IDS_DOWNLOAD_LOG },
		{ IDC_AUTOSRC_LOG, IDS_AUTOSRC_LOG_LBL },
		{ IDC_CLIENT_LOG, IDS_CLIENT_LOG }
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CPPgLogs::OnChangeDebugLogging()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_AUTOSRC_LOG)->EnableWindow(m_bDebugLog);
	GetDlgItem(IDC_CLIENT_LOG)->EnableWindow(m_bDebugLog);
	SetModified();
}
