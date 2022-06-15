//this file is part of eMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#include "SourceInfo.h"
#include "OtherFunctions.h"
#include "ClientList.h"
#include "DownloadQueue.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "UserMsgs.h"
#include "MenuCmds.h"
#include "KnownFileList.h"
#include "SharedFileList.h"
#include "ClientCredits.h"
#include "Neo/NeoPreferences.h"
#include "Opcodes.h"
#include "StringConversion.h"
#include "Neo/Sources/SourceList.h"
#include "Neo/FilePreferences.h"
#include "Neo/Functions.h"
#include "HighColorTab.hpp"
#include "TitleMenu.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->

///////////////////////////////////////////////////////////////////////////////
// CSourceDetailPage

IMPLEMENT_DYNAMIC(CSourceDetailPage, CResizablePage)

BEGIN_MESSAGE_MAP(CSourceDetailPage, CResizablePage)
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_WM_CONTEXTMENU()
	ON_BN_CLICKED(IDC_ANALISE, OnBnClickedAnalyze) 
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CSourceDetailPage::CSourceDetailPage()
	: CResizablePage(CSourceDetailPage::IDD,0)
{
	Source = NULL;
	Client = NULL;
	m_paClients = NULL;
	m_bDataChanged = false;
	m_strCaption = GetResString(IDS_X_SOURCE_INFO);
	m_psp.pszTitle = m_strCaption;
	m_psp.dwFlags |= PSP_USETITLE;

}

CSourceDetailPage::~CSourceDetailPage()
{
}

void CSourceDetailPage::DoDataExchange(CDataExchange* pDX)
{
	CResizablePage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_IPTABLES, m_IPTablesList);
	DDX_Control(pDX, IDC_SEENFILES, m_SeenFilesList); // NEO: SFL - [SourceFileList]
}

BOOL CSourceDetailPage::OnInitDialog()
{
	CResizablePage::OnInitDialog();
	InitWindowStyles(this);

	AddAnchor(IDC_STATIC72, TOP_LEFT, TOP_RIGHT);
	AddAnchor(IDC_STATIC38, TOP_LEFT);
	AddAnchor(IDC_STATIC69, TOP_LEFT);
	AddAnchor(IDC_IPTABLES, TOP_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_SEENFILES, BOTTOM_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_SEENFILESCOUNT, BOTTOM_LEFT, BOTTOM_RIGHT);

	Localize();

	m_IPTablesList.SetName(_T("IPTables"));
	m_IPTablesList.ModifyStyle(LVS_SINGLESEL,0);
	m_IPTablesList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_IPTablesList.InsertColumn(0, GetResString(IDS_X_IP_PORT), LVCFMT_LEFT, 150, -1); 

	m_IPTablesList.InsertColumn(1, GetResString(IDS_X_FIRSTSEEN), LVCFMT_LEFT, 100, -1); 
	m_IPTablesList.InsertColumn(2, GetResString(IDS_X_LASTSEEN), LVCFMT_LEFT, 100, -1); 

	m_IPTablesList.InsertColumn(3, GetResString(IDS_X_UNREAHABLE), LVCFMT_LEFT, 50, -1); 

	m_IPTablesList.LoadSettings();


	// NEO: SFL - [SourceFileList]
	m_SeenFilesList.SetName(_T("SeenFiles"));
	m_SeenFilesList.ModifyStyle(LVS_SINGLESEL,0);
	m_SeenFilesList.SetExtendedStyle(LVS_EX_FULLROWSELECT);
	m_SeenFilesList.InsertColumn(0, GetResString(IDS_DL_FILENAME), LVCFMT_LEFT, 400, -1); 
	m_SeenFilesList.InsertColumn(1, GetResString(IDS_DL_SIZE), LVCFMT_LEFT, 150, -1); 

	m_SeenFilesList.InsertColumn(2, GetResString(IDS_LASTSEEN), LVCFMT_LEFT, 100, -1); 

	m_SeenFilesList.LoadSettings();
	// NEO: SFL END
	return TRUE;
}

void CSourceDetailPage::OnDestroy()
{
	m_IPTablesList.SaveSettings();
	m_SeenFilesList.SaveSettings(); // NEO: SFL - [SourceFileList]
	CResizablePage::OnDestroy();
}


BOOL CSourceDetailPage::OnSetActive()
{
	if (!CResizablePage::OnSetActive())
		return FALSE;

	if (m_bDataChanged)
	{
		if (!m_paClients || m_paClients->GetSize() == 0)
			return FALSE;

		if((*m_paClients)[0]->IsKindOf(RUNTIME_CLASS(CKnownSource))){
			Client = NULL;
			Source = STATIC_DOWNCAST(CKnownSource, (*m_paClients)[0]);
			if(!theApp.sourcelist->IsSourcePtrInList(Source))
				Source = NULL; // Source was deleted, so set the pointer NULL
		}else if((*m_paClients)[0]->IsKindOf(RUNTIME_CLASS(CUpDownClient))){
			Client = STATIC_DOWNCAST(CUpDownClient, (*m_paClients)[0]);
			Source = Client->Source();
		}else
			return FALSE; // What is it then?

		if(Source == NULL)
			return TRUE;

		GetDlgItem(IDC_DNAME)->SetWindowText(Source->GetUserName());
		GetDlgItem(IDC_DHASH)->SetWindowText(md4str(Source->GetUserHash()));
		GetDlgItem(IDC_DIPZONE)->SetWindowText(ipstr(Source->GetIPZone()));

		if(!Client)
			GetDlgItem(IDC_DLASTSEEN)->SetWindowText(CastSecondsToHM(time(NULL) - Source->GetLastSeen()));
		else
			GetDlgItem(IDC_DLASTSEEN)->SetWindowText(StrLine(_T("%s(%s)"),CastSecondsToHM(time(NULL) - Client->GetLastSeen()),CastSecondsToHM(time(NULL) - Source->GetLastSeen())));

		UpdateAnalisis();

		m_IPTablesList.DeleteAllItems();
		GetDlgItem(IDC_IPTABLESCOUNT)->SetWindowText(StrLine(GetResString(IDS_X_IPTABLESCOUNT),Source->m_IPTables.GetCount()));

		for (POSITION pos = Source->m_IPTables.GetHeadPosition(); pos != NULL;){
			IPTableStruct* curIPTable = Source->m_IPTables.GetNext(pos);
			AddIPtable(curIPTable);
		}

		// NEO: SFL - [SourceFileList]
		m_SeenFilesList.DeleteAllItems();
		GetDlgItem(IDC_SEENFILESCOUNT)->SetWindowText(StrLine(GetResString(IDS_X_SEENFILESCOUNT),Source->m_SeenFiles.GetCount()));

		SeenFileStruct* cur_seenfile;
		CCKey tmpkey(0);
		POSITION pos2 = Source->m_SeenFiles.GetStartPosition();
		while (pos2){
			Source->m_SeenFiles.GetNextAssoc(pos2, tmpkey, cur_seenfile);
			AddSeenFile(cur_seenfile);
		}
		// NEO: SFL END
		m_bDataChanged = false;
	}
	return TRUE;
}

void CSourceDetailPage::UpdateAnalisis()
{
	if(Source->GetLastAnalisis())
	{
		int iQuality = Source->GetAnalisisQuality();
		//if(iQuality < 0)
		//	GetDlgItem(IDC_DAQUALITY)->SetWindowText(StrLine(GetResString(IDS_X_RAND_PORT),(-1*iQuality)));
		//else
			GetDlgItem(IDC_DAQUALITY)->SetWindowText(StrLine(_T("%i"),iQuality));

		switch(Source->GetIPType()){
			case IP_Static:
				GetDlgItem(IDC_DIPTYPE)->SetWindowText(GetResString(IDS_X_STATIC));
				break;
			case IP_Dynamic:
				GetDlgItem(IDC_DIPTYPE)->SetWindowText(GetResString(IDS_X_DYNAMIC));
				break;
			case IP_Temporary:
				GetDlgItem(IDC_DIPTYPE)->SetWindowText(GetResString(IDS_X_TEMPORARY));
				break;
		}

		GetDlgItem(IDC_DLASTSEENDURATION)->SetWindowText(StrLine(_T("%s%s"),CastSecondsToHM(Source->GetLastSeenDuration()), 
													(NeoPrefs.UseLinkTimePropability() && Source->GetLastLinkTime() && Source->GetLastLinkTime() < (UINT)NeoPrefs.GetLinkTimeThreshold()) 
													? GetResString(IDS_X_LINKED) : _T("")));
			
		GetDlgItem(IDC_DTOTALSEENDURATION)->SetWindowText(CastSecondsToHM(Source->GetTotalSeenDuration()));

		GetDlgItem(IDC_DIPVALIDITYT)->SetWindowText(StrLine(_T("%s(%s/%s/%s)")
													,CastSecondsToHM((Source->GetMidleIPTime() + Source->GetLongestIPTime()) / 2)
													,CastSecondsToHM(Source->GetShortestIPTime())
													,CastSecondsToHM(Source->GetMidleIPTime())
													,CastSecondsToHM(Source->GetLongestIPTime())
													));
		GetDlgItem(IDC_DONLINET)->SetWindowText(StrLine(_T("%s(%s/%s/%s)")
													,CastSecondsToHM((Source->GetMidleOnTime() + Source->GetLongestOnTime()) / 2)
													,CastSecondsToHM(Source->GetShortestOnTime())
													,CastSecondsToHM(Source->GetMidleOnTime())
													,CastSecondsToHM(Source->GetLongestOnTime())
													));
		GetDlgItem(IDC_DOFFLINET)->SetWindowText(StrLine(_T("%s(%s/%s/%s)")
													,CastSecondsToHM((Source->GetMidleOffTime() + Source->GetLongestOffTime()) / 2)
													,CastSecondsToHM(Source->GetShortestOffTime())
													,CastSecondsToHM(Source->GetMidleOffTime())
													,CastSecondsToHM(Source->GetLongestOffTime())
													));
		GetDlgItem(IDC_DRESUMEC)->SetWindowText(StrLine(_T("%i(%i/%i/%i)")
													,(Source->GetMidleFaildCount() + Source->GetLargestFaildCount()) / 2
													,Source->GetSmallestFaildCount()
													,Source->GetMidleFaildCount()
													,Source->GetLargestFaildCount()
													));
		
		Source->CalculateAvalibilityProbability(); // calculate up to date value
		GetDlgItem(IDC_DPAVALIBILITY)->SetWindowText(StrLine(_T("%i%%"),Source->GetAvalibilityProbability()));

		GetDlgItem(IDC_DLASTA)->SetWindowText(CastSecondsToHM(time(NULL) - Source->GetLastAnalisis()));
	}
	else
	{
		GetDlgItem(IDC_DAQUALITY)->SetWindowText(GetResString(IDS_X_UNKNOWN));

		GetDlgItem(IDC_DIPTYPE)->SetWindowText(GetResString(IDS_X_UNKNOWN));

		GetDlgItem(IDC_DLASTSEENDURATION)->SetWindowText(GetResString(IDS_X_UNKNOWN));

		GetDlgItem(IDC_DTOTALSEENDURATION)->SetWindowText(GetResString(IDS_X_UNKNOWN));

		GetDlgItem(IDC_DIPVALIDITYT)->SetWindowText(GetResString(IDS_UNKNOWN));
		GetDlgItem(IDC_DONLINET)->SetWindowText(GetResString(IDS_UNKNOWN));
		GetDlgItem(IDC_DOFFLINET)->SetWindowText(GetResString(IDS_UNKNOWN));
		GetDlgItem(IDC_DRESUMEC)->SetWindowText(GetResString(IDS_UNKNOWN));

		GetDlgItem(IDC_DPAVALIBILITY)->SetWindowText(GetResString(IDS_UNKNOWN));

		GetDlgItem(IDC_DLASTA)->SetWindowText(GetResString(IDS_NEVER));
	}
}

void CSourceDetailPage::AddIPtable(IPTableStruct* IPTable)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)IPTable;
	int iItem = m_IPTablesList.FindItem(&find);
	if (iItem == -1)
	{
		int index = m_IPTablesList.InsertItem(LVIF_TEXT|LVIF_PARAM,0,StrLine(_T("%s:%u"),ipstr(IPTable->uIP),IPTable->uPort),0,0,1,(LPARAM)IPTable);
		m_IPTablesList.SetItemText(index, 1, CastSecondsToDate(IPTable->tFirstSeen)); 
		m_IPTablesList.SetItemText(index, 2, CastSecondsToDate(IPTable->tLastSeen)); 
		m_IPTablesList.SetItemText(index, 3, StrLine(_T("%u"),IPTable->uUnreachable)/*IPTable->uUnreachable ? GetResString(IDS_YES) : GetResString(IDS_NO)*/);
	}
}

// NEO: SFL - [SourceFileList]
void CSourceDetailPage::AddSeenFile(SeenFileStruct* SeenFile)
{
	LVFINDINFO find;
	find.flags = LVFI_PARAM;
	find.lParam = (LPARAM)SeenFile;
	int iItem = m_SeenFilesList.FindItem(&find);
	if (iItem == -1)
	{
		CKnownFile* kFile = theApp.knownfiles->FindKnownFileByID(SeenFile->abyFileHash);
		if(!kFile)
			kFile = theApp.downloadqueue->GetFileByID(SeenFile->abyFileHash);
		int index = m_SeenFilesList.InsertItem(LVIF_TEXT|LVIF_PARAM,0,(kFile ? kFile->GetFileName() : md4str(SeenFile->abyFileHash)),0,0,1,(LPARAM)SeenFile);
		m_SeenFilesList.SetItemText(index, 1, CastItoXBytes(SeenFile->uFileSize)); 
		m_SeenFilesList.SetItemText(index, 2, CastSecondsToDate(SeenFile->tLastSeen)); 
	}
}
// NEO: SFL END

LRESULT CSourceDetailPage::OnDataChanged(WPARAM, LPARAM)
{
	m_bDataChanged = true;
	return 1;
}

void CSourceDetailPage::OnContextMenu(CWnd* pWnd, CPoint point)
{
	CTitleMenu popupMenu;
	popupMenu.CreatePopupMenu();
	UINT flag = MF_STRING;

	if(pWnd == GetDlgItem(IDC_IPTABLES)){
		if (m_IPTablesList.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED) == -1)
			flag = MF_GRAYED;
		popupMenu.AppendMenu(MF_STRING | flag, MP_REMOVE, GetResString(IDS_X_ERASE));
	}

	// NEO: SFL - [SourceFileList]
	if(pWnd == GetDlgItem(IDC_SEENFILES)){
		if (m_SeenFilesList.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED) == -1)
			flag = MF_GRAYED;
		popupMenu.AppendMenu(MF_STRING | flag, MP_GETED2KLINK, GetResString(IDS_DL_LINK1));
	}
	// NEO: SFL END

	GetPopupMenuPos(m_IPTablesList, point);
	popupMenu.TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( popupMenu.DestroyMenu() );
}

BOOL CSourceDetailPage::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if(Client == NULL && !theApp.sourcelist->IsSourcePtrInList(Source))
		Source = NULL; // Source was deleted, so set the pointer NULL

	int iSel = m_IPTablesList.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED); 
	if (iSel != -1 && Source)
	{
		switch (wParam)
		{
			case MP_REMOVE:{
				IPTableStruct* toremove = (IPTableStruct*)m_IPTablesList.GetItemData(iSel);
				if(toremove == Source->m_CurrentIPTable){
					MessageBox(GetResString(IDS_X_REMOVE_IPTABLE_DENIDED));
					return TRUE;
				}
				POSITION pos = Source->m_IPTables.Find(toremove);
				if(pos){
					Source->m_IPTables.RemoveAt(pos);
					delete toremove;
					m_IPTablesList.DeleteItem(iSel);
				}
				return TRUE;
			}
		}
	}

	// NEO: SFL - [SourceFileList]
	int iSelF = m_SeenFilesList.GetNextItem(-1, LVIS_SELECTED | LVIS_FOCUSED); 
	if (iSelF != -1 && Source)
	{
		switch (wParam)
		{
			case MP_GETED2KLINK:{
				SeenFileStruct* tolink = (SeenFileStruct*)m_SeenFilesList.GetItemData(iSelF);
				SeenFileStruct* cur_seenfile;
				CCKey tmpkey(0);
				POSITION pos = Source->m_SeenFiles.GetStartPosition();
				while (pos){
					Source->m_SeenFiles.GetNextAssoc(pos, tmpkey, cur_seenfile);
					if(cur_seenfile == tolink){
						CString strLink;
						strLink.Format(_T("ed2k://|file|%s|%u|%s|"),
							EncodeUrlUtf8(StripInvalidFilenameChars(m_SeenFilesList.GetItemText(iSel, 0), false)),
							cur_seenfile->tLastSeen,
							EncodeBase16(cur_seenfile->abyFileHash,16));
						strLink += _T("/");
						theApp.CopyTextToClipboard(strLink);
					}
				}
				return TRUE;
			}
		}
	}
	// NEO: SFL END

	return CResizablePage::OnCommand(wParam, lParam);
}

void CSourceDetailPage::OnBnClickedAnalyze()
{
	if(Client == NULL && !theApp.sourcelist->IsSourcePtrInList(Source))
		Source = NULL; // Source was deleted, so set the pointer NULL

	if(Source){
		Source->AnalyzeIPBehavior();
		UpdateAnalisis();
	}
}

void CSourceDetailPage::Localize()
{
	GetDlgItem(IDC_STATIC72)->SetWindowText(GetResString(IDS_X_CIDENTITY));
	GetDlgItem(IDC_STATIC31)->SetWindowText(GetResString(IDS_QL_USERNAME));
	GetDlgItem(IDC_STATIC32)->SetWindowText(GetResString(IDS_CD_UHASH));
	GetDlgItem(IDC_STATIC36)->SetWindowText(GetResString(IDS_X_IP_ZONE));
	GetDlgItem(IDC_STATIC38)->SetWindowText(GetResString(IDS_X_GENERALINFO));
	GetDlgItem(IDC_STATIC55)->SetWindowText(GetResString(IDS_X_LAST_SEEN));
	GetDlgItem(IDC_STATIC62)->SetWindowText(GetResString(IDS_X_LAST_SEEN_DURATION));
	GetDlgItem(IDC_STATIC67)->SetWindowText(GetResString(IDS_X_TOTAL_SEEN_DURATION));
	//GetDlgItem(IDC_IPTABLESCOUNT)->SetWindowText(GetResString(IDS_X_));
	GetDlgItem(IDC_STATIC69)->SetWindowText(GetResString(IDS_X_ANALYSIS));
	GetDlgItem(IDC_STATIC59)->SetWindowText(GetResString(IDS_X_ANALYSIS_QUALITY));
	GetDlgItem(IDC_STATIC61)->SetWindowText(GetResString(IDS_X_EIPTYPE));
	GetDlgItem(IDC_STATIC58)->SetWindowText(GetResString(IDS_X_PAVALIBILITY));

	GetDlgItem(IDC_STATIC63)->SetWindowText(GetResString(IDS_X_IP_TIME));
	GetDlgItem(IDC_STATIC64)->SetWindowText(GetResString(IDS_X_ON_TIME));
	GetDlgItem(IDC_STATIC65)->SetWindowText(GetResString(IDS_X_OFF_TIME));
	GetDlgItem(IDC_STATIC66)->SetWindowText(GetResString(IDS_X_RESUME_COUNT));

	GetDlgItem(IDC_STATIC60)->SetWindowText(GetResString(IDS_X_LAST_ANALYSIS));
	GetDlgItem(IDC_ANALISE)->SetWindowText(GetResString(IDS_X_ANALISE_NOW));	
	//GetDlgItem(IDC_SEENFILESCOUNT)->SetWindowText(GetResString(IDS_X_));
}


///////////////////////////////////////////////////////////////////////////////
// CSourceDetailDialog

IMPLEMENT_DYNAMIC(CSourceDetailDialog, CListViewWalkerPropertySheet)

BEGIN_MESSAGE_MAP(CSourceDetailDialog, CModListViewWalkerPropertySheet) // NEO: MLD - [ModelesDialogs] 
	ON_WM_DESTROY()
END_MESSAGE_MAP()

CSourceDetailDialog::CSourceDetailDialog(CKnownSource* pSource,CListCtrlItemWalk* pListCtrl) 
	: CModListViewWalkerPropertySheet(pListCtrl) // NEO: MLD - [ModelesDialogs] 
{
	m_aItems.Add(pSource);
	Construct();
}

void CSourceDetailDialog::Construct()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_psh.dwFlags |= PSH_NOAPPLYNOW;

	m_wndSource.m_psp.dwFlags &= ~PSP_HASHELP;
	m_wndSource.m_psp.dwFlags |= PSP_USEICONID;
	m_wndSource.m_psp.pszIcon = _T("SOURCESAVER");
	m_wndSource.SetClients(&m_aItems);
	AddPage(&m_wndSource);
}

CSourceDetailDialog::~CSourceDetailDialog()
{
}

void CSourceDetailDialog::OnDestroy()
{
	CListViewWalkerPropertySheet::OnDestroy(); 
}

BOOL CSourceDetailDialog::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult =  CModListViewWalkerPropertySheet::OnInitDialog(); // NEO: MLD - [ModelesDialogs] 
	HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	EnableSaveRestore(_T("CSourceDetailDialog")); // call this after(!) OnInitDialog
	SetWindowText(GetResString(IDS_CD_TITLE));
	return bResult;
}

BOOL CSourceDetailDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	// NEO: MLD - [ModelesDialogs]
	#define IDC_PREV	100
	#define IDC_NEXT	101

	if(!m_pListCtrl){
		if(wParam == IDC_NEXT || wParam == IDC_PREV){
			GetDlgItem(IDC_NEXT)->EnableWindow(FALSE);
			GetDlgItem(IDC_PREV)->EnableWindow(FALSE);
			return TRUE;
		}
	}
	// NEO: MLD END

	return CListViewWalkerPropertySheet::OnCommand(wParam, lParam);
}
#endif // NEO_CD // NEO: NCD END <-- Xanatos --