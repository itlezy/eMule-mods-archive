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

#pragma once

// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
#include "Neo/GUI/CP/SlideBar.h" // NEO: NSB - [SlideBar] <-- Xanatos --
#include "Neo/GUI/CP/KCSideBannerWnd.h" // NEO: NPB - [PrefsBanner] <-- Xanatos --

#include "otherfunctions.h"
#include "ListBoxST.h"
#include "Neo\Gui\ModeLess.h" // NEO: MLD - [ModelesDialogs] <-- Xanatos --

#include "PPgGeneral.h"
#include "PPgConnection.h"
#include "PPgServer.h"
#include "PPgDirectories.h"
#include "PPgFiles.h"
#include "PPgStats.h"
#include "PPgNotify.h"
#include "PPgIRC.h"
#include "PPgMessages.h"
#include "PPgTweaks.h"
#include "PPgDisplay.h"
#include "PPgSecurity.h"
#include "PPgWebServer.h"
#include "PPgScheduler.h"
#include "PPgProxy.h"
// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
#include "PPgNeo.h" 
#include "PPgRelease.h" 
#include "PPgSources.h" 
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
#include "PPgSourceStorage.h"
#endif // NEO: NSS END
#ifdef ARGOS // NEO: NA - [NeoArgos]
#include "PPgArgos.h"
#endif // ARGOS // NEO: NA END
#include "PPgNetwork.h" 
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
#include "PPgBandwidth.h"
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
#include "PPgLancast.h"
#endif //LANCAST // NEO: NLC END
#include "PPgInterface.h" 
#include "PPgVirtual.h" // NEO: VSF - [VirtualSharedFiles]
#include "PPgTweaks2.h" // NEO: MOD - [MissingPrefs]
// NEO: NCFG END < -- Xanatos --

#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
#include "PPgDebug.h"
#endif


#define IDC_PREFS_SLIDEBAR	111	// NEO - [SlideBar] <-- Xanatos --

class CPreferencesDlg : public CModPropertySheet // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	DECLARE_DYNAMIC(CPreferencesDlg)

public:
	CPreferencesDlg();
	virtual ~CPreferencesDlg();
	
	CPPgGeneral		m_wndGeneral;
	CPPgConnection	m_wndConnection;
	CPPgServer		m_wndServer;
	CPPgDirectories	m_wndDirectories;
	CPPgFiles		m_wndFiles;
	CPPgStats		m_wndStats;
	CPPgNotify		m_wndNotify;
	CPPgIRC			m_wndIRC;
	CPPgMessages	m_wndMessages;
	CPPgTweaks		m_wndTweaks;
	CPPgDisplay		m_wndDisplay;
	CPPgSecurity	m_wndSecurity;
	CPPgWebServer	m_wndWebServer;
	CPPgScheduler	m_wndScheduler;
	CPPgProxy		m_wndProxy;
	// NEO: NCFG - [NeoConfiguration] -- Xanatos -->
	CPPgNeo			m_wndNeo;
	CPPgRelease		m_wndRelease;
	CPPgSources		m_wndSources;
#ifdef NEO_SS // NEO: NSS - [NeoSourceStorage]
	CPPgSourceStorage m_wndSourceStorage;
#endif // NEO: NSS END
#ifdef ARGOS // NEO: NA - [NeoArgos]
	CPPgArgos		m_wndArgos;
#endif // ARGOS // NEO: NA END
	CPPgNetwork		m_wndNetwork;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
	CPPgBandwidth	m_wndBandwidth;
#endif // NEO_BC // NEO: NBC END
#ifdef LANCAST // NEO: NLC - [NeoLanCast]
	CPPgLancast		m_wndLancast;
#endif //LANCAST // NEO: NLC END
	CPPgInterface	m_wndInterface;
	CPPgVirtual		m_wndVirtual; // NEO: VSF - [VirtualSharedFiles]
	CPPgTweaks2		m_wndTweaks2; // NEO: MOD - [MissingPrefs]
	// NEO: NCFG END < -- Xanatos --
#if defined(_DEBUG) || defined(USE_DEBUG_DEVICE)
	CPPgDebug		m_wndDebug;
#endif

	CImageList		ImageList;
	CSlideBar	 	m_slideBar; // NEO: NSB - [SlideBar] <-- Xanatos --

	void Localize();

protected:
	INT m_nActiveWnd;

	virtual BOOL OnInitDialog();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	
	afx_msg void OnDestroy();
	afx_msg void OnBnClickedOk(); 
	afx_msg LRESULT	OnSlideBarSelChanged(WPARAM wParam, LPARAM lParam); // NEO: NSB - [SlideBar] <-- Xanatos --
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	CKCSideBannerWnd m_banner; // NEO: NPB - [PrefsBanner] <-- Xanatos --
};

// NEO: NCFG END <-- Xanatos --
