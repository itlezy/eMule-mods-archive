//this file is part of NeoMule
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

// NEO: FCFG - [FileConfiguration] -- Xanatos -->

#include "stdafx.h"
#include "emule.h"
#include "OtherFunctions.h"
#include "FileInfoDialog.h"
#include "FilePreferencesDialog.h"
#include "Preferences.h"
#include "Neo/NeoPreferences.h"
#include "UpDownClient.h"
#include "PartFile.h"
#include "AbstractFile.h"
#include "SearchFile.h"
#include "StringConversion.h"
#include "shahashset.h"
#include "HighColorTab.hpp"
#include "UserMsgs.h"
#include "EnBitmap.h"
#include "SharedFilesCtrl.h"
#include "DownloadListCtrl.h"
#include "SearchListCtrl.h"
#include "CollectionListCtrl.h"
#include "Downloadqueue.h"
#include "knownfilelist.h"
#include "Neo/Ini2.h" // NEO: INI - [PlusIniClass] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOOn - [VoodooForNeo] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOOn END <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CFilePreferencesDialog

//LPCTSTR CFilePreferencesDialog::m_pPshStartPage;

IMPLEMENT_DYNAMIC(CFilePreferencesDialog, CListViewWalkerPreferenceSheet)

BEGIN_MESSAGE_MAP(CFilePreferencesDialog, CModListViewWalkerPreferenceSheet) // NEO: MLD - [ModelesDialogs]
	ON_WM_DESTROY()
	ON_MESSAGE(UM_DATA_CHANGED, OnDataChanged)
	ON_LBN_SELCHANGE(IDC_PREFS_LISTBOX,OnSelChanged)
	ON_WM_CTLCOLOR()
	ON_WM_HELPINFO()
	// NEO: FCFG - [FileConfiguration]
	ON_BN_CLICKED(IDC_CPY, OnCopy)
	ON_BN_CLICKED(IDC_PST, OnPaste)
	ON_BN_CLICKED(IDC_RST, OnReset)
	// NEO: FCFG END
	ON_BN_CLICKED(IDOK, OnBnClickedOk) 
END_MESSAGE_MAP()

CFilePreferencesDialog::CFilePreferencesDialog(const CSimpleArray<CKnownFile*>* paFiles, UINT /*uPshInvokePage*/, CListCtrlItemWalk* pListCtrl)
	: CModListViewWalkerPreferenceSheet(pListCtrl) // NEO: MLD - [ModelesDialogs]
{
	m_Category = NULL;

	//m_uPshInvokePage = uPshInvokePage;
	for (int i = 0; i < paFiles->GetSize(); i++)
		m_aItems.Add((*paFiles)[i]);
	m_psh.dwFlags &= ~PSH_HASHELP;
	
	ASSERT(m_pListCtrl);

	if(m_pListCtrl->GetListCtrl()->IsKindOf(RUNTIME_CLASS(CSharedFilesCtrl)))
		m_bShared = true;
	else if(m_pListCtrl->GetListCtrl()->IsKindOf(RUNTIME_CLASS(CDownloadListCtrl)))
		m_bShared = false;
	else
		ASSERT(0);

	m_wndRelease.m_psp.dwFlags &= ~PSP_HASHELP;
	//m_wndRelease.m_psp.dwFlags |= PSP_USEICONID;
	//m_wndRelease.m_psp.pszIcon = _T("NEOTWEAKS");
	m_wndRelease.SetFiles(&m_aItems);
	AddPage(&m_wndRelease);

	if(m_bShared == false)
	{
		m_wndSources.m_psp.dwFlags &= ~PSP_HASHELP;
		//m_wndSources.m_psp.dwFlags |= PSP_USEICONID;
		//m_wndSources.m_psp.pszIcon = _T("SOURCE");
		m_wndSources.SetFiles(&m_aItems);
		AddPage(&m_wndSources);

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		m_wndSourceStorage.m_psp.dwFlags &= ~PSP_HASHELP;
		//m_wndSourceStorage.m_psp.dwFlags |= PSP_USEICONID;
		//m_wndSourceStorage.m_psp.pszIcon = _T("FILEINFO");
		m_wndSourceStorage.SetFiles(&m_aItems);
		AddPage(&m_wndSourceStorage);
#endif // NEO: NSS END
	}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_wndLancast.m_psp.dwFlags &= ~PSP_HASHELP;
	//m_wndLancast.m_psp.dwFlags |= PSP_USEICONID;
	//m_wndLancast.m_psp.pszIcon = _T("NEOTWEAKS");
	m_wndLancast.SetFiles(&m_aItems);
	AddPage(&m_wndLancast);
#endif //LANCAST // NEO: NLC END

	m_nActiveWnd = 0;
	m_iPrevPage = -1;

	/*LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}*/
}

CFilePreferencesDialog::CFilePreferencesDialog(Category_Struct* category, bool shared)
	: CModListViewWalkerPreferenceSheet((CListCtrlItemWalk*)NULL) // NEO: MLD - [ModelesDialogs]
{
	m_Category = category;
	m_bShared = shared;

	//m_uPshInvokePage = uPshInvokePage;
	m_psh.dwFlags &= ~PSH_HASHELP;

	m_wndRelease.m_psp.dwFlags &= ~PSP_HASHELP;
	//m_wndRelease.m_psp.dwFlags |= PSP_USEICONID;
	//m_wndRelease.m_psp.pszIcon = _T("NEOTWEAKS");
	m_wndRelease.SetCategory(category);
	AddPage(&m_wndRelease);

	if(m_bShared == false)
	{
		m_wndSources.m_psp.dwFlags &= ~PSP_HASHELP;
		//m_wndSources.m_psp.dwFlags |= PSP_USEICONID;
		//m_wndSources.m_psp.pszIcon = _T("SOURCE");
		m_wndSources.SetCategory(category);
		AddPage(&m_wndSources);

#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		m_wndSourceStorage.m_psp.dwFlags &= ~PSP_HASHELP;
		//m_wndSourceStorage.m_psp.dwFlags |= PSP_USEICONID;
		//m_wndSourceStorage.m_psp.pszIcon = _T("FILEINFO");
		m_wndSourceStorage.SetCategory(category);
		AddPage(&m_wndSourceStorage);
#endif // NEO: NSS END
	}

#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_wndLancast.m_psp.dwFlags &= ~PSP_HASHELP;
	//m_wndLancast.m_psp.dwFlags |= PSP_USEICONID;
	//m_wndLancast.m_psp.pszIcon = _T("NEOTWEAKS");
	m_wndLancast.SetCategory(category);
	AddPage(&m_wndLancast);
#endif //LANCAST // NEO: NLC END

	m_nActiveWnd = 0;
	m_iPrevPage = -1;

	/*LPCTSTR pPshStartPage = m_pPshStartPage;
	if (m_uPshInvokePage != 0)
		pPshStartPage = MAKEINTRESOURCE(m_uPshInvokePage);
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == pPshStartPage)
		{
			m_psh.nStartPage = i;
			break;
		}
	}*/
}

CFilePreferencesDialog::~CFilePreferencesDialog()
{
}

void CFilePreferencesDialog::OnDestroy()
{
	//if (m_uPshInvokePage == 0)
	//	m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
	CListViewWalkerPreferenceSheet::OnDestroy();
	m_nActiveWnd = GetActiveIndex();
}

BOOL CFilePreferencesDialog::OnInitDialog()
{		
	EnableStackedTabs(FALSE);
	BOOL bResult = CModListViewWalkerPreferenceSheet::OnInitDialog(); // NEO: MLD - [ModelesDialogs]
	//HighColorTab::UpdateImageList(*this);
	InitWindowStyles(this);
	//EnableSaveRestore(_T("FilePreferencesDialog")); // call this after(!) OnInitDialog
	UpdateTitle();

	m_listbox.CreateEx(WS_EX_CLIENTEDGE,_T("Listbox"),0,WS_CHILD|WS_VISIBLE|LBS_NOTIFY|WS_TABSTOP|LBS_HASSTRINGS|LBS_OWNERDRAWVARIABLE|WS_BORDER,CRect(0,0,0,0),this,IDC_PREFS_LISTBOX);
	::SendMessage(m_listbox.m_hWnd, WM_SETFONT, (WPARAM) ::GetStockObject(DEFAULT_GUI_FONT),0);
	m_groupbox.Create(0,BS_GROUPBOX|WS_CHILD|WS_VISIBLE|BS_FLAT,CRect(0,0,0,0),this,666);
	::SendMessage(m_groupbox.m_hWnd, WM_SETFONT, (WPARAM) ::GetStockObject(DEFAULT_GUI_FONT),0);
	InitWindowStyles(this);

	SetActivePage(m_nActiveWnd);

	// Localise ->
	ImageList.DeleteImageList();
	ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	ImageList.Add(CTempIconLoader(_T("RELEASE"))); 
	if(m_bShared == false)
	{
		ImageList.Add(CTempIconLoader(_T("SOURCE"))); 
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		ImageList.Add(CTempIconLoader(_T("SOURCESAVER")));
#endif // NEO: NSS END
	}
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	ImageList.Add(CTempIconLoader(_T("LANCAST"))); 
#endif //LANCAST // NEO: NLC END
	m_listbox.SetImageList(&ImageList);

	CString title = GetResString(IDS_EM_PREFS); 
	title.Remove(_T('&')); 
	SetTitle(title); 

	//m_wndSources.Localize(); // <- Here

	TC_ITEM item; 
	item.mask = TCIF_TEXT; 

	CStringArray buffer; 
	buffer.Add(GetResString(IDS_X_PW_RELEASE));
	if(m_bShared == false)
	{
		buffer.Add(GetResString(IDS_X_PW_SOURCE));
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
		buffer.Add(GetResString(IDS_X_PW_SOURCE_STORAGE));
#endif // NEO: NSS END
	}
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	buffer.Add(GetResString(IDS_X_PW_LANCAST));
#endif //LANCAST // NEO: NLC END

	for (int i = 0; i < buffer.GetCount(); i++)
		buffer[i].Remove(_T('&'));

	m_listbox.ResetContent();
	int width = 0;
	CTabCtrl* tab = GetTabControl();
	CClientDC dc(this);
	CFont *pOldFont = dc.SelectObject(m_listbox.GetFont());
	CSize sz;
	for(int i = 0; i < GetPageCount(); i++) 
	{ 
		item.pszText = buffer[i].GetBuffer(); 
		tab->SetItem (i, &item); 
		buffer[i].ReleaseBuffer();
		m_listbox.AddString(buffer[i].GetBuffer(),i);
		sz = dc.GetTextExtent(buffer[i]);
		if(sz.cx > width)
			width = sz.cx;
	}
	m_groupbox.SetWindowText(GetResString(IDS_PW_GENERAL));
	width+=50;
	CRect rectOld;
	m_listbox.GetWindowRect(&rectOld);
	int xoffset, yoffset;
	if(IsWindowVisible())
	{
		yoffset=0;
		xoffset=width-rectOld.Width();
	}
	else
	{
		xoffset=width-rectOld.Width()+10;
		tab->GetItemRect(0,rectOld);
		yoffset=-rectOld.Height();
	}
	GetWindowRect(rectOld);
	SetWindowPos(NULL,0,0,rectOld.Width()+xoffset,rectOld.Height()+yoffset,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	tab->GetWindowRect (rectOld);
	ScreenToClient (rectOld);
	tab->SetWindowPos(NULL,rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	CPropertyPage* activepage = GetActivePage();
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	activepage->SetWindowPos(NULL,rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	m_groupbox.SetWindowPos(NULL,rectOld.left,2,rectOld.Width()+4,rectOld.Height()+10,SWP_NOZORDER|SWP_NOACTIVATE);
	m_groupbox.GetWindowRect(rectOld);
	ScreenToClient(rectOld);
	m_listbox.SetWindowPos(NULL,6,rectOld.top+5,width,rectOld.Height()-4,SWP_NOZORDER|SWP_NOACTIVATE);
	int _PropSheetButtons[] = {IDC_PREV, IDC_NEXT, IDC_CPY, IDC_PST, IDC_RST, IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP }; 
	CWnd* PropSheetButton;
	for (int i = 0; i < sizeof (_PropSheetButtons) / sizeof(_PropSheetButtons[0]); i++)
	{
		if ((PropSheetButton = GetDlgItem(_PropSheetButtons[i])) != NULL)
		{
			PropSheetButton->GetWindowRect (rectOld);
			ScreenToClient (rectOld);
			PropSheetButton->SetWindowPos (NULL, rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
		}
	}
	tab->ShowWindow(SW_HIDE);
	m_listbox.SetCurSel(GetActiveIndex());		
	CenterWindow();
	m_banner.UpdateSize(); // NEO: NPB - [PrefsBanner] <-- Xanatos --
	Invalidate();
	RedrawWindow();
	dc.SelectObject(pOldFont); //restore default font object
	// <- Localise

	UpdateTitle();

	m_listbox.SetFocus();
	CString currenttext;
	int curSel=m_listbox.GetCurSel();
	m_listbox.GetText(curSel,currenttext);
	m_groupbox.SetWindowText(currenttext);
	m_iPrevPage = curSel;

	// NEO: NPB - [PrefsBanner]
	if(NeoPrefs.ShowBanner()){
		//m_banner.SetColBkg2( RGB(0,0,0) );
		//m_banner.SetColBkg( RGB(0,0,0) );
		//m_banner.SetColEdge( RGB(0,0,0) );
		CEnBitmap bmp;
		bmp.LoadImage(_T("BANNER"),_T("JPG"));
		m_banner.SetTexture((HBITMAP)bmp.Detach());	
		m_banner.SetFillFlag(KCSB_FILL_TEXTURE);
		m_banner.SetSize(58);
		m_banner.SetTitle(_T(""));
		m_banner.SetCaption(_T(""));
		m_banner.Attach(this, KCSB_ATTACH_RIGHT);
		//int p = m_banner.GetSize();	
	}
	// NEO: NPB END

	return bResult;
}

void CFilePreferencesDialog::OnSelChanged()
{
	int curSel=m_listbox.GetCurSel();
	if (!SetActivePage(curSel)){
		if (m_iPrevPage != -1){
			m_listbox.SetCurSel(m_iPrevPage);
			return;
		}
	}
	CString currenttext;
	m_listbox.GetText(curSel,currenttext);
	m_groupbox.SetWindowText(currenttext);
	m_listbox.SetFocus();
	m_iPrevPage = curSel;
}

LRESULT CFilePreferencesDialog::OnDataChanged(WPARAM, LPARAM)
{
	UpdateTitle();
	return 1;
}

void CFilePreferencesDialog::UpdateTitle()
{
	if (m_aItems.GetSize() == 1)
		SetWindowText(GetResString(IDS_DETAILS) + _T(": ") + STATIC_DOWNCAST(CKnownFile, m_aItems[0])->GetFileName());
	else
		SetWindowText(GetResString(IDS_DETAILS));
}

HBRUSH CFilePreferencesDialog::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
	if (m_groupbox.m_hWnd == pWnd->m_hWnd) 
	{
		pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
		hbr = GetSysColorBrush(COLOR_BTNFACE);
	}
	return hbr;
}

void CFilePreferencesDialog::OnHelp()
{
	int iCurSel = m_listbox.GetCurSel();
	if (iCurSel >= 0)
	{
		CPropertyPage* pPage = GetPage(iCurSel);
		if (pPage)
		{
			HELPINFO hi = {0};
			hi.cbSize = sizeof hi;
			hi.iContextType = HELPINFO_WINDOW;
			hi.iCtrlId = 0;
			hi.hItemHandle = pPage->m_hWnd;
			hi.dwContextId = 0;
			pPage->SendMessage(WM_HELP, 0, (LPARAM)&hi);
			return;
		}
	}

	theApp.ShowHelp(0, HELP_CONTENTS);
}

BOOL CFilePreferencesDialog::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CFilePreferencesDialog::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CFilePreferencesDialog::OpenPage(UINT uResourceID)
{
	int iCurActiveWnd = m_nActiveWnd;
	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		CPropertyPage* pPage = GetPage(i);
		if (pPage->m_psp.pszTemplate == MAKEINTRESOURCE(uResourceID))
		{
			m_nActiveWnd = i;
			break;
		}
	}
	DoModal();
	m_nActiveWnd = iCurActiveWnd;
}

void CFilePreferencesDialog::OnBnClickedOk()
{
	SendMessage(WM_COMMAND, ID_APPLY_NOW);
	SendMessage(WM_CLOSE);
	
#ifdef VOODOO // NEO: VOODOOn - [VoodooForNeo] -- Xanatos -->
	if(NeoPrefs.UseVoodooTransfer())
	{
		if(m_Category)
		{
			int cat = thePrefs.FindCategory(m_Category);
			if(cat != -1)
				theApp.voodoo->ManifestNeoPreferences(CFP_CATEGORY,NULL,cat);	
		}
		else
		{
			CKnownFile* KnownFile;
			for (int i = 0; i < m_aItems.GetSize(); i++)
			{
				// check if the file is still valid
				if(!theApp.downloadqueue->IsPartFile((CKnownFile*)m_aItems[i]) && !theApp.knownfiles->IsFilePtrInList((CKnownFile*)m_aItems[i]))
					continue;

				KnownFile = STATIC_DOWNCAST(CKnownFile, m_aItems[i]);
				
				theApp.voodoo->ManifestNeoPreferences(CFP_FILE,KnownFile);	
			}
		}
	}
#endif // VOODOO // NEO: VOODOOn END <-- Xanatos --
}

void CFilePreferencesDialog::GetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs)
{
	for (int iPage = 0; iPage < GetPageCount(); iPage++)
	{
		CPropertyPage* pPage = GetPage(iPage);
		if (pPage && pPage->m_hWnd){
			if(pPage == &m_wndRelease)
				m_wndRelease.GetFilePreferences(KnownPrefs,PartPrefs,true);
			if(pPage == &m_wndSources)
				m_wndSources.GetFilePreferences(KnownPrefs,PartPrefs,true);
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			if(pPage == &m_wndSourceStorage)
				m_wndSourceStorage.GetFilePreferences(KnownPrefs,PartPrefs,true);
#endif // NEO: NSS END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
			if(pPage == &m_wndLancast)
				m_wndLancast.GetFilePreferences(KnownPrefs,PartPrefs,true);
#endif //LANCAST // NEO: NLC END
		}
	}
}

void CFilePreferencesDialog::SetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs)
{
	for (int iPage = 0; iPage < GetPageCount(); iPage++)
	{
		CPropertyPage* pPage = GetPage(iPage);
		if (pPage && pPage->m_hWnd){
			if(pPage == &m_wndRelease)
				m_wndRelease.SetFilePreferences(KnownPrefs,PartPrefs,true);
			if(pPage == &m_wndSources)
				m_wndSources.SetFilePreferences(KnownPrefs,PartPrefs,true);
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
			if(pPage == &m_wndSourceStorage)
				m_wndSourceStorage.SetFilePreferences(KnownPrefs,PartPrefs,true);
#endif // NEO: NSS END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
			if(pPage == &m_wndLancast)
				m_wndLancast.SetFilePreferences(KnownPrefs,PartPrefs,true);
#endif //LANCAST // NEO: NLC END
		}
	}
}

void CFilePreferencesDialog::OnCopy()
{
	CString IniBuffer;

	CKnownPreferences* KnownPrefs = new CKnownPreferencesEx(m_Category ? CFP_CATEGORY : CFP_FILE);
	CPartPreferences* PartPrefs = m_bShared ? NULL : new CPartPreferencesEx(m_Category ? CFP_CATEGORY : CFP_FILE);
	GetFilePreferences(KnownPrefs,PartPrefs);

	CIni ini;

	if(!m_bShared && !PartPrefs->IsEmpty()){
		ini.SetSection(_T("PartPrefs"));
		PartPrefs->Save(ini);
	}

	if(!KnownPrefs->IsEmpty()){
		ini.SetSection(_T("KnownPrefs"));
		KnownPrefs->Save(ini);
	}

	delete KnownPrefs;
	delete PartPrefs;

	theApp.CopyTextToClipboard(ini.GetBuffer());
}
void CFilePreferencesDialog::OnPaste()
{
	int Answer = AfxMessageBox(GetResString(m_bShared ? IDS_X_FCFG_PASTE_Q_YESNO : IDS_X_FCFG_PASTE_Q_YESNOCANCEL),MB_OK | (m_bShared ?  MB_YESNO : MB_YESNOCANCEL) | MB_ICONQUESTION, NULL);
	if(Answer == (m_bShared ? IDCANCEL : IDNO))
		return;

	CIni ini;
	ini.SetBuffer(theApp.CopyTextFromClipboard());

	CKnownPreferences* KnownPrefs = NULL;
	CPartPreferences* PartPrefs = NULL;

	if(m_bShared || Answer == IDYES)
	{
		if(ini.CategoryExist(_T("KnownPrefs")))
		{
			KnownPrefs = new CKnownPreferencesEx(m_Category ? CFP_CATEGORY : CFP_FILE);
			ini.SetSection(_T("KnownPrefs"));
			KnownPrefs->Load(ini);
		}		
	}

	if(!m_bShared)
	{
		if(ini.CategoryExist(_T("PartPrefs")))
		{
			PartPrefs = new CPartPreferencesEx(m_Category ? CFP_CATEGORY : CFP_FILE);
			ini.SetSection(_T("PartPrefs"));
			PartPrefs->Load(ini);
		}
	}

	SetFilePreferences(KnownPrefs,PartPrefs);

	delete KnownPrefs;
	delete PartPrefs;
}
void CFilePreferencesDialog::OnReset()
{
	int Answer = AfxMessageBox(GetResString(m_bShared ? IDS_X_FCFG_RESET_Q_YESNO : IDS_X_FCFG_RESET_Q_YESNOCANCEL),MB_OK | (m_bShared ?  MB_YESNO : MB_YESNOCANCEL) | MB_ICONQUESTION, NULL);
	if(Answer == (m_bShared ? IDCANCEL : IDNO))
		return;

	CString data = theApp.CopyTextFromClipboard();

	CKnownPreferences* KnownPrefs = NULL;
	CPartPreferences* PartPrefs = NULL;

	if(m_bShared || Answer == IDYES)
		KnownPrefs = new CKnownPreferencesEx(m_Category ? CFP_CATEGORY : CFP_FILE);

	if(!m_bShared)
		PartPrefs = new CPartPreferencesEx(m_Category ? CFP_CATEGORY : CFP_FILE);

	SetFilePreferences(KnownPrefs,PartPrefs);

	delete KnownPrefs;
	delete PartPrefs;
}
// NEO: FCFG END <-- Xanatos --