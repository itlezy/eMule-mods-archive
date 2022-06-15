//this file is part of eMule
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
#include "PreferencesDlg.h"
#include "Neo/NeoPreferences.h" // NEO: NPB - [PrefsBanner] <-- Xanatos --
#include "EnBitmap.h"
#ifdef VOODOO // NEO: VOODOOn - [VoodooForNeo] -- Xanatos -->
#include "Neo/VooDoo/voodoo.h"
#endif // VOODOO // NEO: VOODOOn END <-- Xanatos --

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// NEO: NCFG - [NeoConfiguration] -- Xanatos -->

enum EnumPreferencesListItems
{
	PW_PREFERENCES = 0,
	PW_DISPLAY,
	PW_CONNECTION,
	PW_PROXY,
	PW_SERVER,
	PW_FOLDERS,
	PW_SHAREDFILES,
	PW_NOTIFICATIONS,
	PW_STATISTICS,
	PW_IRC,
	PW_MESSAGES,
	PW_SECURITY,
	PW_SCHEDULER,
	PW_WEB,
	PW_TWEAK,
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	PW_EMPTY,
#endif
	// NEO: NCFG - [NeoConfiguration]
	PW_NEOTWEAKS,
	PW_RELEASE,
	PW_SOURCE,
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	PW_SOURCESTORAGE,
#endif // NEO: NSS END
#ifdef ARGOS // NEO: NA - [NeoArgos]
	PW_ARGOS, 
#endif // ARGOS // NEO: NA END
	PW_NETWORK, 
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	PW_BANDWIDTH, 
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	PW_LANCAST,
#endif //LANCAST // NEO: NLC END
	PW_INTERFACE,
	PW_VIRTUALDIR, // NEO: VSF - [VirtualSharedFiles]
	PW_TWEAK2 // NEO: MOD - [MissingPrefs]
	// NEO: NCFG END
};

IMPLEMENT_DYNAMIC(CPreferencesDlg, CPropertySheet)

BEGIN_MESSAGE_MAP(CPreferencesDlg, CModPropertySheet) // NEO: MLD - [ModelesDialogs] <-- Xanatos --
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDOK, OnBnClickedOk) 
	ON_MESSAGE(WM_SBN_SELCHANGED, OnSlideBarSelChanged) // NEO: NSB - [SlideBar] <-- Xanatos --
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPreferencesDlg::CPreferencesDlg()
{
	m_psh.dwFlags &= ~PSH_HASHELP;
	m_wndGeneral.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDisplay.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndConnection.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndDirectories.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndFiles.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndStats.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndIRC.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndMessages.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndWebServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndScheduler.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP; // deadlake PROXYSUPPORT
	// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
	m_wndNeo.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndRelease.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSources.m_psp.dwFlags &= ~PSH_HASHELP;
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_wndSourceStorage.m_psp.dwFlags &= ~PSH_HASHELP;
#endif // NEO: NSS END
#ifdef ARGOS // NEO: NA - [NeoArgos]
	m_wndArgos.m_psp.dwFlags &= ~PSH_HASHELP;
#endif // ARGOS // NEO: NA END
	m_wndNetwork.m_psp.dwFlags &= ~PSH_HASHELP;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	m_wndBandwidth.m_psp.dwFlags &= ~PSH_HASHELP;
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_wndLancast.m_psp.dwFlags &= ~PSH_HASHELP;
#endif //LANCAST // NEO: NLC END
	m_wndInterface.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndVirtual.m_psp.dwFlags &= ~PSH_HASHELP; // NEO: VSF - [VirtualSharedFiles]
	m_wndTweaks2.m_psp.dwFlags &= ~PSH_HASHELP; // NEO: MOD - [MissingPrefs]
	// NEO: NCFG END < -- Xanatos --
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif

	EnableStackedTabs(FALSE); // NEO: NSB - [SlideBar] <-- Xanatos --

	//	WARNING: Pages must be added with the same order as the slidebar group items.
	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	AddPage(&m_wndProxy);
	AddPage(&m_wndServer);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndNotify);
	AddPage(&m_wndStats);
	AddPage(&m_wndIRC);
	AddPage(&m_wndMessages);
	AddPage(&m_wndSecurity);
	AddPage(&m_wndScheduler);
	AddPage(&m_wndWebServer);
	AddPage(&m_wndTweaks);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif
	// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
	AddPage(&m_wndNeo);
	AddPage(&m_wndRelease);
	AddPage(&m_wndSources);
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	AddPage(&m_wndSourceStorage);
#endif // NEO: NSS END
#ifdef ARGOS // NEO: NA - [NeoArgos]
	AddPage(&m_wndArgos);
#endif // ARGOS // NEO: NA END
	AddPage(&m_wndNetwork);
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	AddPage(&m_wndBandwidth);
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	AddPage(&m_wndLancast);
#endif //LANCAST // NEO: NLC END
	AddPage(&m_wndInterface);
	AddPage(&m_wndVirtual); // NEO: VSF - [VirtualSharedFiles]
	AddPage(&m_wndTweaks2); // NEO: MOD - [MissingPrefs]
	// NEO: NCFG END < -- Xanatos --

	m_nActiveWnd = 0;
}

CPreferencesDlg::~CPreferencesDlg()
{
	ImageList.DeleteImageList(); // NEO: NSB - [SlideBar] <-- Xanatos --
}

void CPreferencesDlg::OnDestroy()
{
	CPropertySheet::OnDestroy();
	m_nActiveWnd = GetActiveIndex();
}

void CPreferencesDlg::OnBnClickedOk()
{
	SendMessage(WM_COMMAND, ID_APPLY_NOW);
	SendMessage(WM_CLOSE);
	thePrefs.Save();
	NeoPrefs.Save(); // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#ifdef VOODOO // NEO: VOODOOn - [VoodooForNeo] -- Xanatos -->
	if(NeoPrefs.UseVoodooTransfer())
		theApp.voodoo->ManifestNeoPreferences(CFP_GLOBAL);
#endif // VOODOO // NEO: VOODOOn END <-- Xanatos --
}

BOOL CPreferencesDlg::OnInitDialog()
{		
	// NEO: NSB - [SlideBar] -- Xanatos -->
	m_slideBar.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER, CRect(0, 0, 0, 0), this, IDC_PREFS_SLIDEBAR);
	m_slideBar.SetImageList(&ImageList);
	m_slideBar.SetHAlignCaption(DT_CENTER);

//	Sets a bold font for the group buttons
	CFont* pGroupFont = m_slideBar.GetGroupFont();
	ASSERT_VALID(pGroupFont);
	LOGFONT logFont;
	pGroupFont->GetLogFont(&logFont);
	logFont.lfWeight *= 2;
	if (logFont.lfWeight > FW_BLACK)
		logFont.lfWeight = FW_BLACK;
	pGroupFont->DeleteObject();
	pGroupFont->CreateFontIndirect(&logFont);
	ASSERT_VALID(pGroupFont);

	BOOL bResult = CModPropertySheet::OnInitDialog(); // NEO: MLD - [ModelesDialogs] <-- Xanatos --

	SetActivePage(m_nActiveWnd);

	Localize();
	m_slideBar.SetFocus();
	// NEO: NSB END <-- Xanatos --

	// NEO: NPB - [PrefsBanner] -- Xanatos -->
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
	// NEO: NPB END <-- Xanatos --

	return bResult;
}

// NEO: NSB - [SlideBar] <-- Xanatos --
LRESULT CPreferencesDlg::OnSlideBarSelChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	int iCurrentGlobalSel = m_slideBar.GetGlobalSelectedItem();

	SetActivePage(iCurrentGlobalSel);

	CListBoxST* pListBox = m_slideBar.GetGroupListBox(m_slideBar.GetSelectedGroupIndex());
	ASSERT_VALID(pListBox);

	CString strCurrentItemText;
	pListBox->GetText(pListBox->GetCurSel(), strCurrentItemText);

	CString strCurrentGroupText = m_slideBar.GetGroupName(m_slideBar.GetSelectedGroupIndex());
	strCurrentGroupText.Remove('&');

	CString strTitle = GetResString(IDS_EM_PREFS);
	strTitle.Remove('&');
	SetWindowText(strTitle + _T(" -> ") + strCurrentGroupText + _T(" -> ") + strCurrentItemText);

	pListBox->SetFocus();

	return TRUE;
}
// NEO: NSB END <-- Xanatos --

void CPreferencesDlg::Localize()
{
	ImageList.DeleteImageList();
	ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	ImageList.Add(CTempIconLoader(_T("Preferences")));
	ImageList.Add(CTempIconLoader(_T("DISPLAY")));
	ImageList.Add(CTempIconLoader(_T("CONNECTION")));
	ImageList.Add(CTempIconLoader(_T("PROXY")));
	ImageList.Add(CTempIconLoader(_T("SERVER")));
	ImageList.Add(CTempIconLoader(_T("FOLDERS")));
	ImageList.Add(CTempIconLoader(_T("SharedFiles")));
	ImageList.Add(CTempIconLoader(_T("NOTIFICATIONS")));
	ImageList.Add(CTempIconLoader(_T("STATISTICS")));
	ImageList.Add(CTempIconLoader(_T("IRC")));
	ImageList.Add(CTempIconLoader(_T("MESSAGES")));
	ImageList.Add(CTempIconLoader(_T("SECURITY")));
	ImageList.Add(CTempIconLoader(_T("SCHEDULER")));
	ImageList.Add(CTempIconLoader(_T("WEB")));
	ImageList.Add(CTempIconLoader(_T("TWEAK")));
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	ImageList.Add(CTempIconLoader(_T("EMPTY")));
#endif
	// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
	ImageList.Add(CTempIconLoader(_T("NEOTWEAKS")));
	ImageList.Add(CTempIconLoader(_T("RELEASE")));
	ImageList.Add(CTempIconLoader(_T("SOURCE")));
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	ImageList.Add(CTempIconLoader(_T("SOURCESAVER")));
#endif // NEO: NSS END
#ifdef ARGOS // NEO: NA - [NeoArgos]
	ImageList.Add(CTempIconLoader(_T("ARGOS"))); 
#endif // ARGOS // NEO: NA END
	ImageList.Add(CTempIconLoader(_T("NETWORK"))); 
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	ImageList.Add(CTempIconLoader(_T("BANDWIDTHCONTROL"))); 
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	ImageList.Add(CTempIconLoader(_T("LANCAST"))); 
#endif //LANCAST // NEO: NLC END
	ImageList.Add(CTempIconLoader(_T("INTERFACE")));
	ImageList.Add(CTempIconLoader(_T("VIRTUALDIR"))); // NEO: VSF - [VirtualSharedFiles]
	ImageList.Add(CTempIconLoader(_T("SYSTEM")));
	// NEO: NCFG END < -- Xanatos --

	CString title = GetResString(IDS_EM_PREFS); 
	title.Remove(_T('&')); 
	SetTitle(title); 

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndServer.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndStats.Localize();
	m_wndNotify.Localize();
	m_wndIRC.Localize();
	m_wndMessages.Localize();
	m_wndSecurity.Localize();
	m_wndTweaks.Localize();
	m_wndWebServer.Localize();
	m_wndScheduler.Localize();
	m_wndProxy.Localize();

	// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
	m_wndVirtual.Localize(); // NEO: VSF - [VirtualSharedFiles]
	m_wndTweaks2.Localize(); // NEO: MOD - [MissingPrefs]
	// NEO: NCFG END <-- Xanatos --

	// NEO: NSB - [SlideBar] -- Xanatos -->
	m_slideBar.ResetContent();

	int iGroupNormal = m_slideBar.AddGroup(GetResString(IDS_X_CFG_NORMAL)/*, 0*/);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_GENERAL), iGroupNormal, PW_PREFERENCES); 
	m_slideBar.AddGroupItem(GetResString(IDS_PW_DISPLAY), iGroupNormal, PW_DISPLAY); 
	m_slideBar.AddGroupItem(GetResString(IDS_PW_CONNECTION), iGroupNormal, PW_CONNECTION); 
	m_slideBar.AddGroupItem(GetResString(IDS_PW_PROXY), iGroupNormal, PW_PROXY); 
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SERVER), iGroupNormal,PW_SERVER); 
	m_slideBar.AddGroupItem(GetResString(IDS_PW_DIR), iGroupNormal, PW_FOLDERS); 
	m_slideBar.AddGroupItem(GetResString(IDS_PW_FILES), iGroupNormal, PW_SHAREDFILES); 
	m_slideBar.AddGroupItem(GetResString(IDS_PW_EKDEV_OPTIONS), iGroupNormal, PW_NOTIFICATIONS); 
	m_slideBar.AddGroupItem(GetResString(IDS_STATSSETUPINFO), iGroupNormal, PW_STATISTICS); 
	m_slideBar.AddGroupItem(GetResString(IDS_IRC), iGroupNormal, PW_IRC);
	m_slideBar.AddGroupItem(GetResString(IDS_MESSAGESCOMMENTS), iGroupNormal, PW_MESSAGES);
	m_slideBar.AddGroupItem(GetResString(IDS_SECURITY), iGroupNormal, PW_SECURITY); 
	m_slideBar.AddGroupItem(GetResString(IDS_SCHEDULER), iGroupNormal, PW_SCHEDULER);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_WS), iGroupNormal, PW_WEB);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_TWEAK), iGroupNormal, PW_TWEAK); 

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_slideBar.AddGroupItem(_T("Debug"), iGroupNormal, PW_EMPTY);	//NoIcon
#endif

	int iGroupNeo = m_slideBar.AddGroup(GetResString(IDS_X_CFG_NEO)/*, 1*/);
	// NEO: NCFG - [NeoConfiguration]
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_NEO), iGroupNeo, PW_NEOTWEAKS);
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_RELEASE), iGroupNeo, PW_RELEASE);
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_SOURCE), iGroupNeo, PW_SOURCE);
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_SOURCE_STORAGE), iGroupNeo, PW_SOURCESTORAGE);
#endif // NEO: NSS END
#ifdef ARGOS // NEO: NA - [NeoArgos]
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_ARGOS), iGroupNeo, PW_ARGOS);
#endif // ARGOS // NEO: NA END
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_NETWORK), iGroupNeo, PW_NETWORK);
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_BANDWIDTH), iGroupNeo, PW_BANDWIDTH);
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_LANCAST), iGroupNeo, PW_LANCAST);
#endif //LANCAST // NEO: NLC END
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_INTERGACE), iGroupNeo, PW_INTERFACE);
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_VIRTUAL), iGroupNeo, PW_VIRTUALDIR); // NEO: VSF - [VirtualSharedFiles]
	m_slideBar.AddGroupItem(GetResString(IDS_X_PW_TWEAK2), iGroupNeo, PW_TWEAK2); // NEO: MOD - [MissingPrefs]
	// NEO: NCFG END
	
	int width = m_slideBar.GetGreaterStringWidth();
	width+=60;

	CRect rectOld;
	m_slideBar.GetWindowRect(rectOld);
	// NEO: NSB END <-- Xanatos --

	int xoffset, yoffset;
	if(IsWindowVisible())
	{
		yoffset=0;
		xoffset=width-rectOld.Width();
	}
	else
	{
		xoffset=width-rectOld.Width()+10;
		// NEO: NSB - [SlideBar] <-- Xanatos --
		GetActivePage()->GetWindowRect(rectOld);
		ScreenToClient (rectOld);
		yoffset = -rectOld.top;
		// NEO: NSB END <-- Xanatos --
	}
	GetWindowRect(rectOld);
	SetWindowPos(NULL,0,0,rectOld.Width()+xoffset,rectOld.Height()+yoffset,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	CTabCtrl* tab = GetTabControl(); // NEO: NSB - [SlideBar] <-- Xanatos --
	tab->GetWindowRect (rectOld);
	ScreenToClient (rectOld);
	tab->SetWindowPos(NULL,rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	CPropertyPage* activepage = GetActivePage();
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	activepage->SetWindowPos(NULL,rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	// NEO: NSB - [SlideBar] <-- Xanatos --
	GetClientRect(rectOld);
	m_slideBar.SetWindowPos(NULL, 6, 6, width, rectOld.Height() - 12, SWP_NOZORDER | SWP_NOACTIVATE);
	// NEO: NSB END <-- Xanatos --
	int _PropSheetButtons[] = {IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };
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
	// NEO: NSB - [SlideBar] <-- Xanatos --
	m_slideBar.SelectGlobalItem(m_nActiveWnd);
	OnSlideBarSelChanged(NULL, NULL);
	// NEO: NSB END <-- Xanatos --
	CenterWindow();
	m_banner.UpdateSize(); // NEO: NPB - [PrefsBanner] <-- Xanatos --
	Invalidate();
	RedrawWindow();
}

void CPreferencesDlg::OnHelp()
{
	int iCurSel = m_slideBar.GetGlobalSelectedItem(); // NEO: NSB - [SlideBar] <-- Xanatos --
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

BOOL CPreferencesDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPreferencesDlg::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

// NEO: NCFG END <-- Xanatos --
