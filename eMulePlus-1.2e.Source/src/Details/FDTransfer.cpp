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
#include "..\resource.h"
#include "FDTransfer.h"
#include "..\PartFile.h"
#include "..\emule.h"
#include "..\otherfunctions.h"

IMPLEMENT_DYNCREATE(CFDTransfer, CPropertyPage)

CFDTransfer::CFDTransfer() : CPropertyPage(CFDTransfer::IDD)
{
	m_pFile = NULL;
}

CFDTransfer::~CFDTransfer()
{
}

BEGIN_MESSAGE_MAP(CFDTransfer, CPropertyPage)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFDTransfer message handlers
/////////////////////////////////////////////////////////////////////////////

void CFDTransfer::Update()
{
	EMULE_TRY

	if ((m_pFile == NULL) || !::IsWindow(GetSafeHwnd()))
		return;

	CString strBuffer;

	strBuffer.Format(_T("%u"), m_pFile->GetSourceCount());
	SetDlgItemText(IDC_FOUNDSRC_VAL, strBuffer);
	strBuffer.Format(_T("%u"), m_pFile->GetCompleteSourcesCount());
	SetDlgItemText(IDC_COMPLETESRC_VAL, strBuffer);
	strBuffer.Format(_T("%u"), m_pFile->GetTransferringSrcCount());
	SetDlgItemText(IDC_TRANSFERRINGSRC_VAL, strBuffer);

	strBuffer.Format(_T("%u (%u)"), m_pFile->GetPartCount(), m_pFile->GetHashCount());
	SetDlgItemText(IDC_PARTCNT_VAL, strBuffer);

	double		dblPartsPercent = 0.0;

	if (m_pFile->GetPartCount() != 0)
		dblPartsPercent = static_cast<double>(m_pFile->GetAvailablePartCount() * 100) / static_cast<double>(m_pFile->GetPartCount());
	strBuffer.Format(_T("%u (%.1f%%)"), m_pFile->GetAvailablePartCount(), dblPartsPercent);
	SetDlgItemText(IDC_AVAILABLEPARTS_VAL, strBuffer);

	if (m_pFile->lastseencomplete == NULL)
		GetResString(&strBuffer, IDS_NEVER);
	else
		strBuffer = m_pFile->LocalizeLastSeenComplete();
	SetDlgItemText(IDC_LASTSEENCOMPLETE_VAL, strBuffer);

	SetDlgItemText(IDC_TRANSFERRED_VAL, strBuffer = CastItoXBytes(m_pFile->GetTransferred()));
	SetDlgItemText(IDC_COMPLETEDSIZE_VAL, strBuffer = CastItoXBytes(m_pFile->GetCompletedSize()));

	strBuffer.Format(_T("%.2f %%"), m_pFile->GetPercentCompleted2());
	SetDlgItemText(IDC_COMPPERC_VAL, strBuffer);

	strBuffer.Format(_T("%.2f %s"), static_cast<double>(m_pFile->GetDataRate())/1024.0, GetResString(IDS_KBYTESEC));
	SetDlgItemText(IDC_DATARATE_VAL, strBuffer);

	if (m_pFile->GetTransferred() == 0)
		GetResString(&strBuffer, IDS_NEVER);
	else
		strBuffer = m_pFile->LocalizeLastDownTransfer();
	SetDlgItemText(IDC_LASTRECEPTION_VAL, strBuffer);

	SetDlgItemText(IDC_CORRUPTIONLOSS_VAL, strBuffer = CastItoXBytes(m_pFile->GetLostDueToCorruption()));
	SetDlgItemText(IDC_COMPRESSIONGAIN_VAL, strBuffer = CastItoXBytes(m_pFile->GetGainDueToCompression()));

	EnumPartFileStatuses	eFileStatus = m_pFile->GetStatus();
	uint64	qwFileSz = ((eFileStatus == PS_COMPLETING) || (eFileStatus == PS_COMPLETE)) ? m_pFile->GetFileSize() : m_pFile->GetCompletedSize();

	strBuffer.Format(_T("%.2f %%"), (qwFileSz != 0) ? (100 * static_cast<double>(m_pFile->GetLostDueToCorruption()) / static_cast<double>(qwFileSz)) : 0);
	SetDlgItemText(IDC_CORRUPTIONLOSS_PERC, strBuffer);
	strBuffer.Format(_T("%.2f %%"), (qwFileSz != 0) ? (100 * static_cast<double>(m_pFile->GetGainDueToCompression()) / static_cast<double>(qwFileSz)) : 0);
	SetDlgItemText(IDC_COMPRESSIONGAIN_PERC, strBuffer);
	strBuffer.Format(_T("%u"), m_pFile->TotalPacketsSavedDueToICH());
	SetDlgItemText(IDC_ICHRECOVERED_VAL, strBuffer);
	SetDlgItemText(IDC_SIZEONDISK_VAL, strBuffer = CastItoXBytes(m_pFile->GetRealFileSize()));

	EMULE_CATCH
}

BOOL CFDTransfer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Localize();
	Update();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CFDTransfer::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_TRANSFERRINGSRC_LBL, IDS_FD_TRANSI },
		{ IDC_PARTCNT_LBL, IDS_FD_PARTS },
		{ IDC_TRANSFERRED_LBL, IDS_SF_TRANS },
		{ IDC_COMPLETEDSIZE_LBL, IDS_FD_COMPSIZE },
		{ IDC_DATARATE_LBL, IDS_FD_DATARATE },
		{ IDC_CORRUPTIONLOSS_LBL, IDS_FD_CORRUPTION },
		{ IDC_COMPRESSIONGAIN_LBL, IDS_FD_COMPRESSION },
		{ IDC_ICHRECOVERED_LBL, IDS_FD_RECOVERED }
	};
	static const uint16 s_auResTbl2[][2] =
	{
		{ IDC_FOUNDSRC_LBL, IDS_FD_SOURCES },
		{ IDC_COMPLETESRC_LBL, IDS_SF_COMPLETESRC },
		{ IDC_AVAILABLEPARTS_LBL, IDS_INFLST_FILE_PARTAVAILABLE },
		{ IDC_LASTSEENCOMPLETE_LBL, IDS_LASTSEENCOMPLETE },
		{ IDC_LASTRECEPTION_LBL, IDS_LASTRECEPTION },
		{ IDC_SIZEONDISK_LBL, IDS_SIZE_ON_DISK }
	};

	EMULE_TRY

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

	EMULE_CATCH
}
