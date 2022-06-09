//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "SharedFilesWnd.h"
#include "SharedFileList.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CSharedFilesWnd, CDialog)
CSharedFilesWnd::CSharedFilesWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CSharedFilesWnd::IDD, pParent)
{
}

CSharedFilesWnd::~CSharedFilesWnd()
{
}

void CSharedFilesWnd::DoDataExchange(CDataExchange* pDX)
{
	CResizableDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SFLIST, m_ctlSharedFilesList);
	DDX_Control(pDX, IDC_POPBAR, pop_bar);
	DDX_Control(pDX, IDC_POPBAR2, pop_baraccept);
	DDX_Control(pDX, IDC_POPBAR3, pop_bartrans);
	DDX_Control(pDX, IDC_STATISTICS, m_ctrlStatisticsFrm);
}

BOOL CSharedFilesWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	m_ctlSharedFilesList.Init();

	pop_bar.SetGradientColors(GetSysColor(COLOR_INACTIVECAPTION),GetSysColor(COLOR_ACTIVECAPTION));
	pop_bar.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
	pop_baraccept.SetGradientColors(GetSysColor(COLOR_INACTIVECAPTION),GetSysColor(COLOR_ACTIVECAPTION));
	pop_baraccept.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));
	pop_bartrans.SetGradientColors(GetSysColor(COLOR_INACTIVECAPTION),GetSysColor(COLOR_ACTIVECAPTION));
	pop_bartrans.SetTextColor(GetSysColor(COLOR_CAPTIONTEXT));

	oldFilesIcon = GetFilesStatic()->SetIcon(GetFilesIcon());

	LOGFONT lf;
	GetFont()->GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	bold.CreateFontIndirect(&lf);
	m_ctrlStatisticsFrm.Init(IDI_SMALLSTATISTICS, &g_App.m_pMDlg->m_themeHelper);
	m_ctrlStatisticsFrm.SetFont(&bold);
	GetDlgItem(IDC_FILEBOX)->SetFont(&bold);
	GetDlgItem(IDC_CURSESSION_LBL)->SetFont(&bold);
	GetDlgItem(IDC_TOTAL_LBL)->SetFont(&bold);

	AddAnchor(IDC_FILES_ICO, TOP_LEFT);
	AddAnchor(IDC_TRAFFIC_TEXT, TOP_LEFT);
	AddAnchor(IDC_SFLIST,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(IDC_RELOADSHAREDFILES, BOTTOM_RIGHT);
	AddAnchor(IDC_OPENINCOMINGFOLDER,TOP_RIGHT);
	AddAnchor(IDC_BN_SWITCHALLKNOWN,TOP_RIGHT);
	AddAnchor(IDC_STATISTICS,BOTTOM_LEFT);
	AddAnchor(IDC_FILEBOX, BOTTOM_LEFT);
	AddAnchor(IDC_CURSESSION_LBL, BOTTOM_LEFT);
	AddAnchor(IDC_TOTAL_LBL, BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC4, BOTTOM_LEFT);
	AddAnchor(IDC_SREQUESTED,BOTTOM_LEFT);
	AddAnchor(IDC_POPBAR,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC7,BOTTOM_LEFT);
	AddAnchor(IDC_SREQUESTED2,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC5,BOTTOM_LEFT);
	AddAnchor(IDC_SACCEPTED,BOTTOM_LEFT);
	AddAnchor(IDC_POPBAR2,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC8,BOTTOM_LEFT);
	AddAnchor(IDC_SACCEPTED2,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC6,BOTTOM_LEFT);
	AddAnchor(IDC_STRANSFERRED,BOTTOM_LEFT);
	AddAnchor(IDC_POPBAR3,BOTTOM_LEFT);
	AddAnchor(IDC_FSTATIC9,BOTTOM_LEFT);
	AddAnchor(IDC_STRANSFERRED2,BOTTOM_LEFT);

	Localize();

	m_ttip.Create(this);
	m_ttip.SetDelayTime(TTDT_AUTOPOP, 15000);
	m_ttip.SetDelayTime(TTDT_INITIAL, g_App.m_pPrefs->GetToolTipDelay()*1000);
	m_ttip.SendMessage(TTM_SETMAXTIPWIDTH, 0, SHRT_MAX);
	m_ttip.SetBehaviour(PPTOOLTIP_MULTIPLE_SHOW);
	m_ttip.SetNotify(m_hWnd);
	m_ttip.AddTool(&m_ctlSharedFilesList, _T(""));

	return true;
}

BEGIN_MESSAGE_MAP(CSharedFilesWnd, CResizableDialog)
	ON_BN_CLICKED(IDC_RELOADSHAREDFILES, OnBnClickedReloadsharedfiles)
	ON_NOTIFY(LVN_ITEMACTIVATE, IDC_SFLIST, OnLvnItemActivateSflist)
	ON_NOTIFY(NM_CLICK, IDC_SFLIST, OnNMClickSflist)
	ON_BN_CLICKED(IDC_OPENINCOMINGFOLDER, OnBnClickedOpenincomingfolder)
	ON_BN_CLICKED(IDC_BN_SWITCHALLKNOWN, OnBnClickedSwitchAllKnown)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

void CSharedFilesWnd::OnBnClickedReloadsharedfiles()
{
	g_App.m_pSharedFilesList->Reload();
}

void CSharedFilesWnd::OnLvnItemActivateSflist(NMHDR *pNMHDR, LRESULT *pResult)
{
	NOPRM(pNMHDR);

	if (m_ctlSharedFilesList.GetSelectionMark() != (-1))
	{
		sfl_itemdata	*itemdata = (sfl_itemdata*)m_ctlSharedFilesList.GetItemData(m_ctlSharedFilesList.GetSelectionMark());

		ShowDetails(itemdata->knownFile);
	}
	*pResult = 0;
}

void CSharedFilesWnd::ShowDetails(CKnownFile* cur_file)
{
	ASSERT(cur_file != NULL);

	CString buffer;

	pop_bartrans.SetShowPercent();			
	pop_bartrans.SetRange32(0, static_cast<uint32>(g_App.m_pKnownFilesList->m_qwNumTransferred / 1024ui64));
	pop_bartrans.SetPos(static_cast<uint32>(cur_file->statistic.GetTransferred() / 1024ui64));
	
	SetDlgItemText(IDC_STRANSFERRED, CastItoXBytes(cur_file->statistic.GetTransferred()));

	pop_bar.SetShowPercent();			
	pop_bar.SetRange32(0,g_App.m_pKnownFilesList->m_iNumRequested);
	pop_bar.SetPos(cur_file->statistic.GetRequests());
	buffer.Format(_T("%u"),cur_file->statistic.GetRequests());
	SetDlgItemText(IDC_SREQUESTED, buffer);

	buffer.Format(_T("%u"),cur_file->statistic.GetAccepts());
	pop_baraccept.SetShowPercent();
	pop_baraccept.SetRange32(0,g_App.m_pKnownFilesList->m_iNumAccepted);
	pop_baraccept.SetPos(cur_file->statistic.GetAccepts());
	SetDlgItemText(IDC_SACCEPTED, buffer);
	SetDlgItemText(IDC_STRANSFERRED2, CastItoXBytes(cur_file->statistic.GetAllTimeTransferred()));

	buffer.Format(_T("%u"),cur_file->statistic.GetAllTimeRequests());
	SetDlgItemText(IDC_SREQUESTED2, buffer);

	buffer.Format(_T("%u"),cur_file->statistic.GetAllTimeAccepts());
	SetDlgItemText(IDC_SACCEPTED2, buffer);

	SetDlgItemText(IDC_FILEBOX, cur_file->GetFileName());
}

void CSharedFilesWnd::OnNMClickSflist(NMHDR *pNMHDR, LRESULT *pResult)
{
	OnLvnItemActivateSflist(pNMHDR, pResult);
	*pResult = 0;
}

BOOL CSharedFilesWnd::PreTranslateMessage(MSG* pMsg) 
{
	if (pMsg->message == WM_KEYUP && pMsg->hwnd == GetDlgItem(IDC_SFLIST)->m_hWnd)
	{
		LRESULT	lResult = 0;

		OnLvnItemActivateSflist(NULL, &lResult);
	}

	if (g_App.m_pPrefs->GetToolTipDelay() != 0)
		m_ttip.RelayEvent(pMsg);

	return CResizableDialog::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CSharedFilesWnd::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	NMHDR	*pNMHDR = (NMHDR*)lParam;

	switch (pNMHDR->code)
	{
		case UDM_TOOLTIP_DISPLAY:
		{
			NM_PPTOOLTIP_DISPLAY *pNotify = (NM_PPTOOLTIP_DISPLAY*)lParam;

			GetInfo4ToolTip(pNotify);
			return TRUE;
		}
		case UDM_TOOLTIP_POP:
		{
			m_ttip.Pop();
			return TRUE;
		}
	}

	return CResizableDialog::OnNotify(wParam, lParam, pResult);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CSharedFilesWnd::GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify)
{
	int	iControlId = CWnd::FromHandle(pNotify->ti->hWnd)->GetDlgCtrlID();

	if (iControlId == IDC_SFLIST)
	{
		int	iSel = GetItemUnderMouse(&m_ctlSharedFilesList);

		if (iSel < 0 || iSel == 65535)
			return;

		sfl_itemdata	*pItemData = reinterpret_cast<sfl_itemdata*>(m_ctlSharedFilesList.GetItemData(iSel));

		if (pItemData == NULL)
			return;

		CKnownFile	*pKnownFile = pItemData->knownFile;

		if (pKnownFile == NULL || !pItemData->isFile)
			return;

		CString	strTemp;
		CString	strInfo;

		if (m_ctlSharedFilesList.IsKnownFilesView())
			GetResString(&strTemp, (!pKnownFile->GetSharedFile()) ? IDS_KNOWNFILES : IDS_SHAREDFILES);
		else
		{
			if (pKnownFile->IsPartFile())
				strTemp.Format(_T("%s (%s)"), GetResString(IDS_TREE_DL), ((CPartFile*)pKnownFile)->GetPartFileStatus());
			else
				GetResString(&strTemp, IDS_SHAREDFILES);
		}

		CString	strPriority(pKnownFile->GetKnownFilePriorityString());

		strInfo.Format(_T("<t=2><b>%s</b><br><t=2>%s<br><t=2>%s<br><hr=100%%><br><b>%s:<t></b>%s (%s %s)<br><b>%s:<t></b>%s<br><b>%s:<t></b>%s<br><b>%s:<t></b>%s"),
			pKnownFile->GetFileName(),
			pKnownFile->GetFileTypeString(),
			strTemp,
			GetResString(IDS_DL_SIZE), CastItoXBytes(pKnownFile->GetFileSize()), CastItoThousands(pKnownFile->GetFileSize()), GetResString(IDS_BYTES),
			GetResString(IDS_PRIORITY), strPriority,
			GetResString(IDS_PERMISSION), pKnownFile->GetPermissionString(),
			GetResString(IDS_FILEHASH), HashToString(pKnownFile->GetFileHash()));

		EnumPartFileRating	eRated = pKnownFile->GetFileRating();

		if (eRated != PF_RATING_NONE)
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_TT_CMT_RATING), GetRatingString(eRated));

		strTemp = pKnownFile->GetFileComment();
		strTemp.Replace(_T("<"), _T("<<"));

		if (!strTemp.IsEmpty())
			strInfo.AppendFormat(_T("<br><b>%s<t></b>%s"), GetResString(IDS_CMT_READ), strTemp);

		uint64	qwA = pKnownFile->statistic.GetRequests();
		uint64	qwB = pKnownFile->statistic.GetAllTimeRequests();

		if (qwA != 0 || qwB != 0)
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%I64u (%I64u)"), GetResString(IDS_SF_REQUESTS), qwA, qwB);

		qwA = pKnownFile->statistic.GetAccepts();
		qwB = pKnownFile->statistic.GetAllTimeAccepts();
		if (qwA != 0 || qwB != 0)
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%I64u (%I64u)"), GetResString(IDS_SF_ACCEPTS), qwA, qwB);

		qwA = pKnownFile->statistic.GetTransferred();
		qwB = pKnownFile->statistic.GetAllTimeTransferred();
		if (qwA != 0 || qwB != 0)
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s (%s)"), GetResString(IDS_SF_TRANSFERRED), CastItoXBytes(qwA), CastItoXBytes(qwB));

		qwA = pKnownFile->GetFileSize();

		double	dblCR = pKnownFile->statistic.GetCompleteReleases();

		if (qwA != 0 && (dblCR + qwB != 0.0))
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%0.2f (%0.2f)"), GetResString(IDS_SF_COLUPLOADS), dblCR, (double)qwB / qwA);

		if (pKnownFile->IsPartFile())
		{
			strTemp.Format(_T("%u"), ((CPartFile*)pKnownFile)->GetCompleteSourcesCount());
		}
		else
		{
			uint16	nCompleteSourcesCountLo, nCompleteSourcesCountHi;

			pKnownFile->GetCompleteSourcesRange(&nCompleteSourcesCountLo, &nCompleteSourcesCountHi);
			if (nCompleteSourcesCountLo == 0)
			{
				if (nCompleteSourcesCountHi == 0)
					strTemp = _T("");
				else
					strTemp.Format(_T("<< %u"), nCompleteSourcesCountHi);
			}
			else if (nCompleteSourcesCountLo == nCompleteSourcesCountHi)
				strTemp.Format(_T("%u"), nCompleteSourcesCountLo);
			else
				strTemp.Format(_T("%u - %u"), nCompleteSourcesCountLo, nCompleteSourcesCountHi);
		}

		if (!strTemp.IsEmpty())
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_SF_COMPLETESRC), strTemp);

		strTemp = pKnownFile->GetPath();
		if (!strTemp.IsEmpty())
			strInfo.AppendFormat(_T("<br><b>%s:<t></b>%s"), GetResString(IDS_SF_FOLDER), strTemp);

		SHFILEINFO	shfi;

		memzero(&shfi, sizeof(shfi));
		SHGetFileInfo(pKnownFile->GetFileName(), FILE_ATTRIBUTE_NORMAL, &shfi, sizeof(shfi), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES);
		pNotify->ti->hIcon = shfi.hIcon;
		pNotify->ti->sTooltip = strInfo;
	}
}

void CSharedFilesWnd::Localize()
{
	static const int s_aiResTbl[][2] =
	{
		{ IDC_RELOADSHAREDFILES, IDS_SF_RELOAD },
		{ IDC_OPENINCOMINGFOLDER, IDS_SF_OPENINCOMINGFOLDER },
		{ IDC_FILEBOX, IDS_FILEBOX },
		{ IDC_CURSESSION_LBL, IDS_SF_CURRENT },
		{ IDC_TOTAL_LBL, IDS_SF_TOTAL2 },
		{ IDC_FSTATIC6, IDS_SF_TRANS },
		{ IDC_FSTATIC5, IDS_SF_ACCEPTED },
		{ IDC_FSTATIC4, IDS_SF_REQUESTED },
		{ IDC_FSTATIC9, IDS_SF_TRANS },
		{ IDC_FSTATIC8, IDS_SF_ACCEPTED },
		{ IDC_FSTATIC7, IDS_SF_REQUESTED }
	};

	for (uint32 i = 0; i < ARRSIZE(s_aiResTbl); i++)
		SetDlgItemText(s_aiResTbl[i][0], GetResString(static_cast<UINT>(s_aiResTbl[i][1])));

	m_ctrlStatisticsFrm.SetText(GetResString(IDS_STATISTICS));
	m_ctlSharedFilesList.Localize();
}

void CSharedFilesWnd::OnBnClickedOpenincomingfolder()
{
	ShellOpenFile(g_App.m_pPrefs->GetIncomingDir());
}

void CSharedFilesWnd::OnBnClickedSwitchAllKnown()
{
	if (m_ctlSharedFilesList.IsKnownFilesView())
		m_ctlSharedFilesList.ShowFileList(g_App.m_pSharedFilesList);
	else
		m_ctlSharedFilesList.ShowKnownList();
}

void CSharedFilesWnd::OnDestroy()
{
	CResizableDialog::OnDestroy();
	DestroyIcon(GetFilesStatic()->SetIcon(oldFilesIcon));	
}
