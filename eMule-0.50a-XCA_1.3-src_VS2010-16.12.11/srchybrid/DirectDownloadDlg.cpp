//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "DirectDownloadDlg.h"
#include "OtherFunctions.h"
#include "emuleDlg.h"
#include "DownloadQueue.h"
#include "ED2KLink.h"
#include "Preferences.h"
#include "KnownFileList.h" //Xman [MoNKi: -Check already downloaded files-]
#include "TransferDlg.h"
#include "DownloadListCtrl.h"
#include "emule.h" // X: [UIC] - [UIChange] allow change cat

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//////////////////////////////////////////////////////////////////////////////
// COLUMN_INIT -- List View Columns

enum ELinkCols// X: [PL] - [Preview Links]
{
	LINK_COL_NAME = 0,
	LINK_COL_SIZE,
	LINK_COL_HASH
};

static LCX_COLUMN_INIT _aColumns[] =
{
	{ LINK_COL_NAME,	_T("File Name"),	IDS_DL_FILENAME,	LVCFMT_LEFT,	-1, 0, ASCENDING,  NONE, _T("long long long long long long long long file name.avi") },
	{ LINK_COL_SIZE,	_T("Size"),			IDS_DL_SIZE,		LVCFMT_RIGHT,	-1, 1, ASCENDING,  NONE, _T("9999 MByte")},
	{ LINK_COL_HASH,	_T("Hash"),			0,					LVCFMT_LEFT,	-1, 2, ASCENDING,  NONE, _T("1234ABCD1234ABCD1234ABCD1234ABCD") }
};

#define	PREF_INI_SECTION	_T("DirectDownloadDlg")

IMPLEMENT_DYNAMIC(CDirectDownloadDlg, CDialog)

BEGIN_MESSAGE_MAP(CDirectDownloadDlg, CModResizableDialog) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	ON_WM_DESTROY()
	ON_EN_KILLFOCUS(IDC_ELINK, OnEnKillfocusElink)
	ON_EN_UPDATE(IDC_ELINK, UpdateControls)// X: [CI] - [Code Improvement]
	ON_BN_CLICKED(IDC_PREVIEW, OnBnClickedPreview)// X: [PL] - [Preview Links]
	ON_NOTIFY(LVN_GETDISPINFO, IDC_FILELIST, OnLvnGetDispInfoLink)
END_MESSAGE_MAP()

CDirectDownloadDlg::CDirectDownloadDlg(CWnd* pParent /*=NULL*/)
	: CModResizableDialog(CDirectDownloadDlg::IDD, pParent) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	m_icnWnd = NULL;
	clipboardPromptMode = isShowList = isInited = false;
	m_LinkList.m_pParent = this;
	m_LinkList.SetRegistryKey(PREF_INI_SECTION);
	m_LinkList.SetRegistryPrefix(_T("LinkList_"));
	//m_LinkList.m_pfnFindItem = FindItem;
	//m_LinkList.m_lFindItemParam = (DWORD_PTR)this;
}

CDirectDownloadDlg::~CDirectDownloadDlg()
{
	for(std::vector<CED2KFileLink*>::const_iterator it = m_pLinkItems.begin(); it!=m_pLinkItems.end(); ++it)
		delete *it;
	if (m_icnWnd)
		VERIFY( DestroyIcon(m_icnWnd) );
}

void CDirectDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CModResizableDialog::DoDataExchange(pDX);
	//DDX_Control(pDX, IDC_DDOWN_FRM, m_ctrlDirectDlFrm);
	DDX_Control(pDX, IDC_CATS, m_cattabs);
	DDX_Control(pDX, IDC_FILELIST, m_LinkList);
}

void CDirectDownloadDlg::UpdateControls()
{
	if(GetDlgItem(IDC_ELINK)->GetWindowTextLength() > 0){
		isInited = false;
		if(clipboardPromptMode)
			theApp.m_bGuardClipboardPrompt = true;
		GetDlgItem(IDOK)->EnableWindow(TRUE);
	}
	else
		GetDlgItem(IDOK)->EnableWindow(FALSE);
}
/*
void CDirectDownloadDlg::OnEnUpdateElink()
{
	UpdateControls();
}
*/
void CDirectDownloadDlg::OnEnKillfocusElink()
{
	//CString strLinks;
	GetDlgItemText(IDC_ELINK,m_LinkText);
	if (m_LinkText.IsEmpty() || m_LinkText.Find(_T('\n')) == -1)
		return;
	m_LinkText.Replace(_T("\n"), _T("\r\n"));
	m_LinkText.Replace(_T("\r\r"), _T("\r"));
	SetDlgItemText(IDC_ELINK,m_LinkText);
}

BOOL CDirectDownloadDlg::OnInitDialog()
{
	CModResizableDialog::OnInitDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	InitWindowStyles(this);
	SetIcon(m_icnWnd = theApp.LoadIcon(_T("PasteLink")), FALSE);

	if(clipboardPromptMode){
		SetWindowText(GetResString(IDS_ADDDOWNLOADSFROMCB));
		GetDlgItem(IDOK)->EnableWindow(TRUE);
		OnBnClickedPreview();
	}
	else{
		SetWindowText(GetResString(IDS_SW_DIRECTDOWNLOAD));
		GetDlgItem(IDOK)->EnableWindow(false);
		SetDlgItemText(IDC_PREVIEW, GetResString(IDS_DL_PREVIEW));
	}

	if (theApp.IsVistaThemeActive())
		m_cattabs.ModifyStyle(0, TCS_HOTTRACK);

	//AddAnchor(m_ctrlDirectDlFrm, TOP_LEFT, BOTTOM_RIGHT);// X: [CI] - [Code Improvement]
	AddAnchor(m_LinkList, TOP_LEFT, BOTTOM_RIGHT);// X: [PL] - [Preview Links]
	AddAnchor(IDC_ELINK, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDCANCEL, BOTTOM_RIGHT);
	AddAnchor(IDOK, BOTTOM_RIGHT);
	AddAnchor(IDC_PREVIEW, BOTTOM_LEFT);
	AddAnchor(IDC_CATLABEL, BOTTOM_LEFT);
	AddAnchor(m_cattabs, BOTTOM_LEFT,BOTTOM_RIGHT);// X: [CI] - [Code Improvement]

	EnableSaveRestore(PREF_INI_SECTION, !thePrefs.prefReadonly); // X: [ROP] - [ReadOnlyPreference]

	ASSERT( m_LinkList.GetStyle() & LVS_OWNERDATA );// X: [PL] - [Preview Links]
	// Win98: Explicitly set to Unicode to receive Unicode notifications.
	//m_LinkList.SendMessage(CCM_SETUNICODEFORMAT, TRUE);
	m_LinkList.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_INFOTIP);
	m_LinkList.EnableHdrCtrlSortBitmaps();
	m_LinkList.ReadColumnStats(ARRSIZE(_aColumns), _aColumns);
	m_LinkList.CreateColumns(ARRSIZE(_aColumns), _aColumns);
  	m_LinkList.InitColumnOrders(ARRSIZE(_aColumns), _aColumns);
	//m_LinkList.UpdateSortColumn(ARRSIZE(_aColumns), _aColumns);

	//m_ctrlDirectDlFrm.SetIcon(_T("Download"));
	//m_ctrlDirectDlFrm.SetWindowText(GetResString(IDS_SW_DIRECTDOWNLOAD));
	SetDlgItemText(IDC_FSTATIC2,GetResString(IDS_SW_LINK));
	SetDlgItemText(IDC_CATLABEL,GetResString(IDS_CAT)+_T(':'));
	//SetDlgItemText(IDC_PREVIEW,GetResString (IDS_DL_PREVIEW));

	SetDlgItemText(IDOK,GetResString(IDS_DOWNLOAD));
	SetDlgItemText(IDCANCEL,GetResString(IDS_CANCEL));

	if (thePrefs.GetCatCount()>1) {
		UpdateCatTabs();
		if (theApp.m_fontSymbol.m_hObject){
			GetDlgItem(IDC_CATLABEL)->SetFont(&theApp.m_fontSymbol);
			SetDlgItemText(IDC_CATLABEL,GetExStyle() & WS_EX_LAYOUTRTL ? _T("3") : _T("4")); // show a right-arrow
		}
	}
	else {
		GetDlgItem(IDC_CATLABEL)->ShowWindow(SW_HIDE);
		m_cattabs.ShowWindow(SW_HIDE);// X: [CI] - [Code Improvement]
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CDirectDownloadDlg::UpdateCatTabs() {
	m_cattabs.DeleteAllItems();
	for (size_t ix=0;ix<thePrefs.GetCatCount();ix++) {
		CString label=/*(ix==0)?GetResString(IDS_ALL):*/thePrefs.GetCategory(ix)->strTitle;// X: [UIC] - [UIChange] change cat0 Title
		label.Replace(_T("&"),_T("&&"));
		m_cattabs.InsertItem(ix,label);
	}
	m_cattabs.SetCurSel(theApp.emuledlg->transferwnd->GetDownloadList()->curTab);// X: [UIC] - [UIChange] follow transfer tab
}

void CDirectDownloadDlg::OnBnClickedPreview()// X: [PL] - [Preview Links]
{
	if(isShowList = !isShowList){
		InitLink();
		m_LinkList.ShowWindow(SW_SHOW);
		GetDlgItem(IDC_ELINK)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_FSTATIC2)->ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_PREVIEW, GetResString(IDS_DL_SHOWED2KLINK));
		if(clipboardPromptMode && isInited)
			theApp.m_bGuardClipboardPrompt = false;
	}
	else{
		GetDlgItem(IDC_ELINK)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_FSTATIC2)->ShowWindow(SW_SHOW);
		m_LinkList.ShowWindow(SW_HIDE);
		SetDlgItemText(IDC_PREVIEW, GetResString(IDS_DL_PREVIEW));
	}
}

void CDirectDownloadDlg::OnOK()
{
	if(clipboardPromptMode)
		theApp.m_bGuardClipboardPrompt = true;

	InitLink();
	ShowWindow(SW_HIDE);
	size_t cat = (thePrefs.GetCatCount() > 1) ? m_cattabs.GetCurSel() : 0;
	for(std::vector<CED2KFileLink*>::const_iterator it = m_pLinkItems.begin(); it!=m_pLinkItems.end(); ++it){
		//Xman [MoNKi: -Check already downloaded files-]
		if(theApp.knownfiles->CheckAlreadyDownloadedFileQuestion((*it)->GetHashKey(),(*it)->GetName()))
			theApp.downloadqueue->AddFileLinkToDownload(*it, cat);
		//Xman end
	}
	CModResizableDialog::OnOK(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --
}

void CDirectDownloadDlg::InitLink()// X: [PL] - [Preview Links]
{
	if(isInited)
		return;
	for(std::vector<CED2KFileLink*>::const_iterator it = m_pLinkItems.begin(); it!=m_pLinkItems.end(); ++it)
		delete *it;
	std::vector<CED2KFileLink*>().swap(m_pLinkItems);
	//if(GetDlgItem(IDC_ELINK)->GetWindowTextLength() == 0)
	if(m_LinkText.GetLength() == 0){
		isInited = true;
		return;
	}
	//CString strLinks;
	//GetDlgItemText(IDC_ELINK,strLinks);
	AddLink2Vector(m_LinkText);
}

void CDirectDownloadDlg::AddLink2Vector(CString&appendLink) // X: [UIC] - [UIChange] allow change cat
{
	int curPos = 0;
	CString strTok = appendLink.Tokenize(_T(" \t\r\n"), curPos); // tokenize by whitespaces
	while (!strTok.IsEmpty())
	{
		if (strTok.Right(1) != _T('/'))
			strTok += _T('/');
		try
		{
			CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(strTok);
			if (pLink){
				if (pLink->GetKind() == CED2KLink::kFile){
					m_pLinkItems.push_back((CED2KFileLink*)pLink);
				}
				else{
					delete pLink;
					throw GetResString(IDS_ERR_NOTAFILELINK);
				}
			}
		}
		catch(CString error)
		{
			if(clipboardPromptMode)
				theApp.m_bGuardClipboardPrompt = true;
			TCHAR szBuffer[200];
			_sntprintf(szBuffer, _countof(szBuffer) - 1, GetResString(IDS_ERR_INVALIDLINK), error);
			szBuffer[_countof(szBuffer) - 1] = _T('\0');
			CString strError;
			strError.Format(GetResString(IDS_ERR_LINKERROR), szBuffer);
			AfxMessageBox(strError);
			m_LinkList.SetItemCount(m_pLinkItems.size());
			return;
		}
		strTok = appendLink.Tokenize(_T(" \t\r\n"), curPos); // tokenize by whitespaces
	}
	m_LinkList.SetItemCount(m_pLinkItems.size());
	isInited = true;
}

void CDirectDownloadDlg::OnDestroy()// X: [PL] - [Preview Links]
{
	if(clipboardPromptMode){
		theApp.emuledlg->directdowndlg = NULL;
		theApp.m_bGuardClipboardPrompt = false;
	}
	if(!thePrefs.prefReadonly)// X: [ROP] - [ReadOnlyPreference]
		m_LinkList.WriteColumnStats(ARRSIZE(_aColumns), _aColumns);
	CModResizableDialog::OnDestroy();
}

void CDirectDownloadDlg::OnLvnGetDispInfoLink(NMHDR *pNMHDR, LRESULT *pResult)// X: [PL] - [Preview Links]
{
	NMLVDISPINFO* pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	if (pDispInfo->item.mask & LVIF_TEXT && pDispInfo->item.cchTextMax > 0) // *have* to check that flag!!// X: [CI] - [Code Improvement]
	{
		switch (pDispInfo->item.iSubItem)
		{
			case LINK_COL_NAME:
				_tcsncpy(pDispInfo->item.pszText, m_pLinkItems[pDispInfo->item.iItem]->GetName(), pDispInfo->item.cchTextMax);
				break;
			case LINK_COL_SIZE:
				_tcsncpy(pDispInfo->item.pszText,CastItoXBytes(m_pLinkItems[pDispInfo->item.iItem]->GetSize()), pDispInfo->item.cchTextMax);
				break;
			case LINK_COL_HASH:
				_tcsncpy(pDispInfo->item.pszText, md4str(m_pLinkItems[pDispInfo->item.iItem]->GetHashKey()), pDispInfo->item.cchTextMax);
				break;
		}
		pDispInfo->item.pszText[pDispInfo->item.cchTextMax - 1] = _T('\0');
	}
	*pResult = 0;
}

void CDirectDownloadDlg::ClipboardPromptMode(LPCTSTR pszLinks) // X: [UIC] - [UIChange] allow change cat
{
	clipboardPromptMode = true;
}

void CDirectDownloadDlg::AddLink(LPCTSTR pszLinks) // X: [UIC] - [UIChange] allow change cat
{
	if(!isInited)
		return;
	CString appendLink = pszLinks;
	AddLink2Vector(appendLink);
	m_LinkText.Append(_T("\r\n"));
	m_LinkText.Append(pszLinks);
	SetDlgItemText(IDC_ELINK, m_LinkText);
}
