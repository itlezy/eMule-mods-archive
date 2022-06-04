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
#include "PreferencesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


//MORPH START - Preferences groups [ePlus/Sirob]
/*
IMPLEMENT_DYNAMIC(CPreferencesDlg, CTreePropSheet)

BEGIN_MESSAGE_MAP(CPreferencesDlg, CTreePropSheet)
	ON_WM_DESTROY()
*/
IMPLEMENT_DYNAMIC(CPreferencesDlg, CPropertySheet)

BEGIN_MESSAGE_MAP(CPreferencesDlg, CPropertySheet)
	ON_WM_DESTROY()
		ON_MESSAGE(WM_SBN_SELCHANGED, OnSlideBarSelChanged) //MORPH - Changed by SiRoB, ePlus Group
	ON_WM_CTLCOLOR()
//MORPH END   - Preferences groups [ePlus/Sirob]
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
	m_wndWebServer.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndTweaks.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndSecurity.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndScheduler.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndProxy.m_psp.dwFlags &= ~PSH_HASHELP;
	m_wndMessages.m_psp.dwFlags &= ~PSH_HASHELP;
	/* morph moved:
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_wndDebug.m_psp.dwFlags &= ~PSH_HASHELP;
#endif
   */

	m_wndMorph.m_psp.dwFlags &= ~PSH_HASHELP; //MORPH - Added by IceCream, Morph Prefs
	m_wndMorphShare.m_psp.dwFlags &= ~PSH_HASHELP; //MORPH - Added by SiRoB, Morph Prefs
	m_wndMorph2.m_psp.dwFlags &= ~PSH_HASHELP; //MORPH - Added by SiRoB, Morph Prefs
	m_wndBackup.m_psp.dwFlags &= ~PSH_HASHELP; //EastShare - Added by Pretender, TBH-AutoBackup
	m_wndEastShare.m_psp.dwFlags &= ~PSH_HASHELP; //EastShare - Added by Pretender, ES Prefs
	m_wndEmulespana.m_psp.dwFlags &= ~PSH_HASHELP; //MORPH - Added by SiRoB, emulEspaña preferency
//MORPH START - Preferences groups [ePlus/Sirob]
/*
	CTreePropSheet::SetPageIcon(&m_wndGeneral, _T("Preferences"));
	CTreePropSheet::SetPageIcon(&m_wndDisplay, _T("DISPLAY"));
	CTreePropSheet::SetPageIcon(&m_wndConnection, _T("CONNECTION"));
	CTreePropSheet::SetPageIcon(&m_wndProxy, _T("PROXY"));
	CTreePropSheet::SetPageIcon(&m_wndServer, _T("SERVER"));
	CTreePropSheet::SetPageIcon(&m_wndDirectories, _T("FOLDERS"));
	CTreePropSheet::SetPageIcon(&m_wndFiles, _T("Transfer"));
	CTreePropSheet::SetPageIcon(&m_wndNotify, _T("NOTIFICATIONS"));
	CTreePropSheet::SetPageIcon(&m_wndStats, _T("STATISTICS"));
	CTreePropSheet::SetPageIcon(&m_wndIRC, _T("IRC"));
	CTreePropSheet::SetPageIcon(&m_wndSecurity, _T("SECURITY"));
	CTreePropSheet::SetPageIcon(&m_wndScheduler, _T("SCHEDULER"));
	CTreePropSheet::SetPageIcon(&m_wndWebServer, _T("WEB"));
	CTreePropSheet::SetPageIcon(&m_wndTweaks, _T("TWEAK"));
	CTreePropSheet::SetPageIcon(&m_wndMessages, _T("MESSAGES"));
	#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CTreePropSheet::SetPageIcon(&m_wndDebug, _T("Preferences"));
    #endif
*/
    m_wndIonixWebServer.m_psp.dwFlags &= ~PSH_HASHELP;  //ionix advanced webserver
    m_wndNTService.m_psp.dwFlags &= ~PSH_HASHELP; 
//MORPH END   - Preferences groups [ePlus/Sirob]



	AddPage(&m_wndGeneral);
	AddPage(&m_wndDisplay);
	AddPage(&m_wndConnection);
	/* MORPH moved:
	AddPage(&m_wndProxy);
	*/
	AddPage(&m_wndServer);
	AddPage(&m_wndDirectories);
	AddPage(&m_wndFiles);
	AddPage(&m_wndNotify);
	/* MORPH moved:
	AddPage(&m_wndStats);
	AddPage(&m_wndIRC);
	*/
	AddPage(&m_wndMessages);
	AddPage(&m_wndSecurity);
	/* morph moved:
	AddPage(&m_wndScheduler);
	AddPage(&m_wndWebServer);
	*/
	AddPage(&m_wndProxy); // moved. morph
	AddPage(&m_wndIRC);	 // moved, morph
	AddPage(&m_wndStats); // moved, morph
	AddPage(&m_wndScheduler); // moved, morph
	AddPage(&m_wndWebServer); // moved, morph
	AddPage(&m_wndTweaks);
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif
	AddPage(&m_wndBackup); //EastShare - Added by Pretender, TBH-AutoBackup
	AddPage(&m_wndMorph); //MORPH - Added by IceCream, Morph Prefs
	AddPage(&m_wndMorphShare); //MORPH - Added by SiRoB, Morph Prefs
	AddPage(&m_wndMorph2); //MORPH - Added by SiRoB, Morph Prefs
	AddPage(&m_wndEastShare); //EastShare - Added by Pretender, ES Prefs
	AddPage(&m_wndEmulespana); //MORPH - Added by SiRoB, emulEspaña preferency
	
	/* morph moved:
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	AddPage(&m_wndDebug);
#endif
	*/
     AddPage(&m_wndIonixWebServer); // Morph - ionix advanced webserver	  tab
	 AddPage(&m_wndNTService); // MORPH leuk_he:run as ntservice v1.. tab


//MORPH START - Preferences groups [ePlus/Sirob]
/*
	// The height of the option dialog is already too large for 640x480. To show as much as
	// possible we do not show a page caption (which is an decorative element only anyway).
	SetTreeViewMode(TRUE, GetSystemMetrics(SM_CYSCREEN) >= 600, TRUE);
	SetTreeWidth(170);
*/
	m_nActiveWnd = 0;
	m_iPrevPage = -1;
//MORPH END   - Preferences groups [ePlus/Sirob]

	m_pPshStartPage = NULL;
	m_bSaveIniFile = false;
	// MORPH start tabbed options [leuk_he]
	ActivePageWebServer			= 0;
	StartPageWebServer			= 0;
	NTService                   = 0; //MORPH leuk_he:run as ntservice v1..
	// end tabbed

}

CPreferencesDlg::~CPreferencesDlg()
{
}

void CPreferencesDlg::OnDestroy()
{
//MORPH START - Preferences groups [ePlus/Sirob]
/*
	CTreePropSheet::OnDestroy();
*/
	CPropertySheet::OnDestroy();
//MORPH END   - Preferences groups [ePlus/Sirob]
	if (m_bSaveIniFile)
	{
		thePrefs.Save();
		m_bSaveIniFile = false;
	}
	m_nActiveWnd = GetActiveIndex(); //MORPH - Preferences groups [ePlus/Sirob]

	m_pPshStartPage = GetPage(GetActiveIndex())->m_psp.pszTemplate;
}

BOOL CPreferencesDlg::OnInitDialog()
{		
	ASSERT( !m_bSaveIniFile );
//MORPH START - Preferences groups [ePlus/Sirob]
/*
	BOOL bResult = CTreePropSheet::OnInitDialog();
*/
	EnableStackedTabs(FALSE);
	BOOL bResult = CPropertySheet::OnInitDialog();
	m_slideBar.CreateEx(WS_EX_CLIENTEDGE, WS_CHILD | WS_VISIBLE | WS_BORDER, CRect(0, 0, 0, 0), this, 111);
	m_slideBar.SetImageList(&ImageList);
	m_slideBar.SetHAlignCaption(DT_CENTER);
	m_groupbox.Create(0,BS_GROUPBOX|WS_CHILD|WS_VISIBLE|BS_FLAT,CRect(0,0,0,0),this,666);
	::SendMessage(m_groupbox.m_hWnd, WM_SETFONT, (WPARAM) ::GetStockObject(DEFAULT_GUI_FONT),0);

	//sets a bold font for the group buttons
	CFont* pGroupFont = m_slideBar.GetGroupFont();
	ASSERT_VALID(pGroupFont);
	LOGFONT logFont;
	pGroupFont->GetLogFont(&logFont);
	logFont.lfWeight *= 2;
	if (logFont.lfWeight > FW_BLACK)
	{
		logFont.lfWeight = FW_BLACK;
	}
	pGroupFont->DeleteObject();
	pGroupFont->CreateFontIndirect(&logFont);
	ASSERT_VALID(pGroupFont);

//MORPH END   - Preferences groups [ePlus/Sirob]
	InitWindowStyles(this);

	for (int i = 0; i < m_pages.GetSize(); i++)
	{
		if (GetPage(i)->m_psp.pszTemplate == m_pPshStartPage)
		{
			SetActivePage(i);
			// MORPH start tabbed options [leuk_he]
			if ((i==Multiwebserver)||(i==NTService) ) //MORPH leuk_he:run as ntservice v1..
			{
				m_wndWebServer.InitTab(false,StartPageWebServer);
				m_wndIonixWebServer.InitTab(false,StartPageWebServer);
			    m_wndNTService.InitTab(false,StartPageWebServer);//MORPH leuk_he:run as ntservice v1..
			}
			// MORPH end tabbed options [leuk_he]
			break;
		}
		
			
	}


	//MORPH START - Added by SiRoB, Load a jpg
	CBitmap bmp;
	VERIFY( bmp.Attach(theApp.LoadImage(_T("BANNER"), _T("JPG"))) );
	if (bmp.GetSafeHandle())
	{
	//MORPH END   - Added by SiRoB, Load a jpg
		//Commander - Added: Preferences Banner [TPT] - Start			
		m_banner.SetTexture((HBITMAP)bmp.Detach());	
		m_banner.SetFillFlag(KCSB_FILL_TEXTURE);
		m_banner.SetSize(thePrefs.sidebanner?70:0);
		m_banner.SetTitle(_T(""));
		m_banner.SetCaption(_T(""));
		m_banner.Attach(this, KCSB_ATTACH_RIGHT);
		//Commander - Added: Preferences Banner [TPT] - End
	}
	Localize();			  // Morph: move after load jpg.  (xman) 
	m_slideBar.SetFocus(); //MORPH - Preferences groups [ePlus/Sirob]

	return bResult;
}

void CPreferencesDlg::Localize()
{
//MORPH START - Preferences groups [ePlus/Sirob]
/*
	SetTitle(RemoveAmbersand(GetResString(IDS_EM_PREFS))); 
*/
	ImageList.DeleteImageList();
	ImageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	ImageList.Add(CTempIconLoader(_T("PREFERENCES")));			//0
	ImageList.Add(CTempIconLoader(_T("DISPLAY")));				//1
	ImageList.Add(CTempIconLoader(_T("CONNECTION")));			//2
	ImageList.Add(CTempIconLoader(_T("SERVER")));				//4
	ImageList.Add(CTempIconLoader(_T("FOLDERS")));				//5
	ImageList.Add(CTempIconLoader(_T("TRANSFER")));				//6
	ImageList.Add(CTempIconLoader(_T("NOTIFICATIONS")));		//7
	ImageList.Add(CTempIconLoader(_T("MESSAGES")));				//10
	ImageList.Add(CTempIconLoader(_T("SECURITY")));				//11

	ImageList.Add(CTempIconLoader(_T("PROXY")));				//3
	ImageList.Add(CTempIconLoader(_T("IRC")));					//9
	ImageList.Add(CTempIconLoader(_T("STATISTICS")));			//8
	ImageList.Add(CTempIconLoader(_T("SCHEDULER")));			//12
	ImageList.Add(CTempIconLoader(_T("WEB")));					//13
	ImageList.Add(CTempIconLoader(_T("TWEAK")));				//14
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	ImageList.Add(CTempIconLoader(_T("PREFERENCES")));			//15
#endif
	ImageList.Add(CTempIconLoader(_T("BACKUP"))); //EastShare - Added by Pretender, TBH-AutoBackup
	ImageList.Add(CTempIconLoader(_T("MORPH")));  //MORPH - Added by IceCream, Morph Prefs
	ImageList.Add(CTempIconLoader(_T("MORPH")));  //MORPH - Added by SiRoB, Morph Prefs
	ImageList.Add(CTempIconLoader(_T("MORPH")));  //MORPH - Added by SiRoB, Morph Prefs
	ImageList.Add(CTempIconLoader(_T("EASTSHARE")));  //MORPH - Added by IceCream, Morph Prefs  //EastShare - Modified by Pretender
	ImageList.Add(CTempIconLoader(_T("EMULESPANA")));  //MORPH - Added by IceCream, eMulEspaña Preferency
	
	m_slideBar.SetImageList(&ImageList);
//MORPH END   - Preferences groups [ePlus/Sirob]

	m_wndGeneral.Localize();
	m_wndDisplay.Localize();
	m_wndConnection.Localize();
	m_wndServer.Localize();
	m_wndDirectories.Localize();
	m_wndFiles.Localize();
	m_wndStats.Localize();
	m_wndNotify.Localize();
	m_wndIRC.Localize();
	m_wndSecurity.Localize();
	m_wndTweaks.Localize();
	m_wndWebServer.Localize();
	m_wndScheduler.Localize();
	m_wndProxy.Localize();
	m_wndMessages.Localize();
	m_wndMorph.Localize();//MORPH - Added by IceCream, Morph Prefs
	m_wndMorphShare.Localize();//MORPH - Added by SiRoB, Morph Prefs
	m_wndMorph2.Localize();//MORPH - Added by SiRoB, Morph Prefs
	m_wndEastShare.Localize();
	m_wndEmulespana.Localize(); //MORPH - Added by SiRoB, emulEspaña preferency
    m_wndIonixWebServer.Localize(); //MORPH ionix advanced webserver
	m_wndNTService.Localize(); //MORPH leuk_he:run as ntservice v1..
	m_slideBar.ResetContent(); //MORPH - Preferences groups [ePlus/Sirob]
	int c = 0;

//MORPH START - Preferences groups [ePlus/Sirob]
/*
	CTreeCtrl* pTree = GetPageTreeControl();
	if (pTree)
	{
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_GENERAL)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_DISPLAY))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_CONNECTION))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_PROXY))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_SERVER))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_DIR))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_FILES))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_EKDEV_OPTIONS))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_STATSSETUPINFO))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_IRC)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_MESSAGESCOMMENTS)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_SECURITY))); 
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_SCHEDULER)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_WS)));
		pTree->SetItemText(GetPageTreeItem(c++), RemoveAmbersand(GetResString(IDS_PW_TWEAK)));
    	#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
		pTree->SetItemText(GetPageTreeItem(c++), _T("Debug"));
	     #endif
	}

	UpdateCaption();
*/
//	Official group
	int iGroup = m_slideBar.AddGroup( GetResString(IDS_PREF_GROUPGENERAL )/*, 1*/);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_GENERAL), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_DISPLAY), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_CONNECTION), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_SERVER), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_DIR), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_FILES), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_EKDEV_OPTIONS), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_MESSAGESCOMMENTS), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_SECURITY), iGroup, c++);
// advanced...	official. 
	iGroup = m_slideBar.AddGroup(GetResString(IDS_PREF_GROUPEXTENDED));
	m_slideBar.AddGroupItem(GetResString(IDS_PW_PROXY), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_IRC), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_STATSSETUPINFO), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_SCHEDULER), iGroup, c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_WS), iGroup, Webserver=c++);
	m_slideBar.AddGroupItem(GetResString(IDS_PW_TWEAK), iGroup, c++);


#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	m_slideBar.AddGroupItem(_T("Debug"), iGroup, c++);
#endif

	//	MOD group
	iGroup = m_slideBar.AddGroup(GetResString(IDS_PREF_GROUPMOD));
	m_slideBar.AddGroupItem(GetResString(IDS_BACKUP), iGroup, c++);
	m_slideBar.AddGroupItem(_T("Morph"), iGroup, c++);
	m_slideBar.AddGroupItem(_T("Morph Share"), iGroup, c++);
	m_slideBar.AddGroupItem(_T("Morph Update"), iGroup, c++);
	m_slideBar.AddGroupItem(_T("EastShare"), iGroup, c++);
	m_slideBar.AddGroupItem(_T("emulEspaña"), iGroup, c++); //MORPH - Added by SiRoB, emulEspaña preferency
	//m_slideBar.AddGroupItem(_T(" "), iGroup, Multiwebserver=c++); // ionix advnaced webserver
	Multiwebserver=c++;
	NTService=c++;

	//	Determines the width needed to the slidebar, and its position
	int width = m_slideBar.GetGreaterStringWidth();
	CTabCtrl* tab = GetTabControl();
	width += 60;
	CRect rectOld;

	m_slideBar.GetWindowRect(rectOld);

	int xoffset, yoffset;
	if(IsWindowVisible())
	{
		yoffset=0;
		xoffset=width-rectOld.Width();
	}
	else
	{
		xoffset=width-rectOld.Width()+10;
		GetActivePage()->GetWindowRect(rectOld);
		tab->GetItemRect(0,rectOld);
		yoffset=-rectOld.Height();
	}
	GetWindowRect(rectOld);
	SetWindowPos(NULL,0,0,rectOld.Width()+xoffset,rectOld.Height()+yoffset,SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	tab->GetWindowRect (rectOld);
	ScreenToClient (rectOld);
	tab->SetWindowPos(NULL,rectOld.left+xoffset+2,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	CPropertyPage* activepage = GetActivePage();
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	activepage->SetWindowPos(NULL,rectOld.left+xoffset,rectOld.top+yoffset,0,0,SWP_NOSIZE|SWP_NOZORDER|SWP_NOACTIVATE);
	activepage->GetWindowRect(rectOld);
	ScreenToClient (rectOld);
	m_groupbox.SetWindowPos(NULL,rectOld.left,2,rectOld.Width()+4,rectOld.Height()+10,SWP_NOZORDER|SWP_NOACTIVATE);
	m_groupbox.GetWindowRect(rectOld);
	ScreenToClient(rectOld);
	GetClientRect(rectOld);
	m_slideBar.SetWindowPos(NULL, 6, 6, width, rectOld.Height() - 12, SWP_NOZORDER | SWP_NOACTIVATE);
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
	m_slideBar.SelectGlobalItem(GetActiveIndex());
	OnSlideBarSelChanged(NULL, NULL);
	m_banner.UpdateSize(); //Commander - Added: Preferences Banner [TPT]
	CenterWindow();
	Invalidate();
	RedrawWindow();
//MORPH END   - Preferences groups [ePlus/Sirob]
}

void CPreferencesDlg::OnHelp()
{
//MORPH START - Preferences groups [ePlus/Sirob]
/*
	int iCurSel = GetActiveIndex();
*/
	int iCurSel = m_listbox.GetCurSel();
//MORPH END   - Preferences groups [ePlus/Sirob]
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
	if (wParam == IDOK || wParam == ID_APPLY_NOW)
		m_bSaveIniFile = true;
	return __super::OnCommand(wParam, lParam);
}

BOOL CPreferencesDlg::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CPreferencesDlg::SetStartPage(UINT uStartPageID)
{
	m_pPshStartPage = MAKEINTRESOURCE(uStartPageID);
}


// MORPH start tabbed option [leuk_he]
void CPreferencesDlg::SwitchTab(int Page)
{
	
	if(m_hWnd && IsWindowVisible()){
		CPropertyPage* activepage = GetActivePage();
								   
		CString strTitle = GetResString(IDS_EM_PREFS);
		strTitle.Remove('&');
		CString strGroupTitle = GetResString(IDS_PREF_GROUPEXTENDED);
		strGroupTitle.Remove('&');
		// webServer 1-2 -3
		if (activepage == &m_wndWebServer || activepage == &m_wndIonixWebServer || activepage == &m_wndNTService){
			if (Page == 0) {
				SetActivePage(&m_wndWebServer);
				ActivePageWebServer = 0;
				StartPageWebServer = 0;
				m_wndWebServer.InitTab(false,0);
				SetWindowText(strTitle + _T(" -> ") + strGroupTitle + _T(" -> ") + GetResString(IDS_TAB_WEB_SERVER));
			}
			if (Page == 1) {
				SetActivePage(&m_wndIonixWebServer);
				ActivePageWebServer = Multiwebserver;
				StartPageWebServer = 1;
				m_wndIonixWebServer.InitTab(false,1);
				SetWindowText(strTitle + _T(" -> ") + strGroupTitle + _T(" -> ") + GetResString(IDS_TAB_MULTI_USER) + _T(" ") + GetResString(IDS_PW_WS));
			}			
			// MORPH leuk_he:run as ntservice v1..
			if (Page == 2) {
				SetActivePage(&m_wndNTService);
				ActivePageWebServer = NTService;
				StartPageWebServer = 2;
				m_wndNTService.InitTab(false,2);
				SetWindowText(strTitle + _T(" -> ") + strGroupTitle + _T(" -> ") + GetResString(IDS_TAB_NT_SERVICE));
			}		
			// MORPH leuk_he:run as ntservice v1..
		}
	}
}
// MORPH end tabbed option [leuk_he]

//MORPH START - Preferences groups [ePlus/Sirob]
LRESULT CPreferencesDlg::OnSlideBarSelChanged(WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	int iCurrentGlobalSel	= m_slideBar.GetGlobalSelectedItem();
	CString strCurrentItemText;

	// MORPH start tabbed option [leuk_he]
	if(iCurrentGlobalSel == Webserver)
	{
		if(ActivePageWebServer == Multiwebserver)
		{
			SetActivePage(&m_wndIonixWebServer);
			strCurrentItemText = GetResString(IDS_TAB_MULTI_USER) + _T(" ") + GetResString(IDS_PW_WS);
		}
		else if(ActivePageWebServer == NTService)
		{
			SetActivePage(&m_wndNTService);
			strCurrentItemText = GetResString(IDS_TAB_NT_SERVICE);
		}
		else
			SetActivePage(&m_wndWebServer);
	}
	else
	// MORPH end tabbed option [leuk_he]
	SetActivePage(iCurrentGlobalSel);

	CListBoxST* pListBox = m_slideBar.GetGroupListBox(m_slideBar.GetSelectedGroupIndex());
	ASSERT_VALID(pListBox);

	if(strCurrentItemText.IsEmpty())
		pListBox->GetText(pListBox->GetCurSel(), strCurrentItemText);

	CString strCurrentGroupText = m_slideBar.GetGroupName(m_slideBar.GetSelectedGroupIndex());
	strCurrentGroupText.Remove('&');

	CString strTitle = GetResString(IDS_EM_PREFS);
	strTitle.Remove('&');
	SetWindowText(strTitle + _T(" -> ") + strCurrentGroupText + _T(" -> ") + strCurrentItemText);

//	m_groupbox.SetWindowText(strCurrentItemText);

	pListBox->SetFocus();

	return true;
}

HBRUSH CPreferencesDlg::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = CPropertySheet::OnCtlColor(pDC, pWnd, nCtlColor);
	if (m_groupbox.m_hWnd == pWnd->m_hWnd) 
	{
		pDC->SetBkColor(GetSysColor(COLOR_BTNFACE));
		hbr = GetSysColorBrush(COLOR_BTNFACE);
	}
	return hbr;
}
//MORPH END   - Preferences groups [ePlus/Sirob]