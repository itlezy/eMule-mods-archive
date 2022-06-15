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


#pragma once
#include "Neo\GUI\CP\TreeOptionsCtrl.h" // NEO - [TreeControl] <-- Xanatos --

struct tNeoButton
{
	HTREEITEM m_htiButton;
	HTREEITEM m_htiGroups[4];
};

class CPPgInterface : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgInterface)

public:
	CPPgInterface();
	virtual ~CPPgInterface();

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

protected:
	CTreeOptionsCtrl m_ctrlTreeOptions; // NEO - [TreeControl] <-- Xanatos --
	bool m_bInitializedTreeOpts;
	// NEO: NXC - [NewExtendedCategories]
	HTREEITEM m_htiMorphCats;
		HTREEITEM m_htiShowCatNames;
		HTREEITEM m_htiShowCategoryFlags;
		HTREEITEM m_htiActiveCatDefault;
		HTREEITEM m_htiSelCatOnAdd;
		HTREEITEM m_htiAutoSetResumeOrder;
		HTREEITEM m_htiSmallFileDLPush;
		HTREEITEM m_htiStartDLInEmptyCats;
		HTREEITEM m_htiUseAutoCat;
		HTREEITEM m_htiCheckAlreadyDownloaded;
		HTREEITEM m_htiStartNextFileByPriority;
	// NEO: NXC END
	// NEO: NTB - [NeoToolbarButtons]
	HTREEITEM m_htiNeoToolbar;
		tNeoButton *m_htiNeoToolbarButtons; // !!
		int			m_cntNeoToolbarButtons; // we must cache this value
	// NEO: NTB END
	HTREEITEM m_htiShowBanner; // NEO: NPB - [PrefsBanner]
	HTREEITEM m_htiCollorShareFiles; // NEO: NSC - [NeoSharedCategories]
	HTREEITEM m_htiSmoothStatisticsGraphs; // NEO: NBC - [NeoBandwidthControl] 
	HTREEITEM m_htiDisableAutoSort; // NEO: SE - [SortExtension]
	// NEO: NSTI - [NewSystemTrayIcon]
	HTREEITEM m_htiNewSystemTrayIcon;
		HTREEITEM m_htiShowSystemTrayUpload;
		HTREEITEM m_htiThinSystemTrayBars;
		HTREEITEM m_htiTrayBarsMaxCollor;
	// NEO: NSTI END
	HTREEITEM m_htiStaticTrayIcon; // NEO: STI - [StaticTray]
	// NEO: IM - [InvisibelMode]
	HTREEITEM m_htiInvisibelMode;
		HTREEITEM m_htiInvisibelModeHotKeyModifier;
		HTREEITEM m_htiInvisibelModeHotKey;
	// NEO: IM END
	HTREEITEM m_htiTrayPasswordProtection; // NEO: TPP - [TrayPasswordProtection]
	HTREEITEM m_htiUsePlusSpeedMeter; // NEO: PSM - [PlusSpeedMeter]
	HTREEITEM m_htiUseRelativeChunkDisplay; // NEO: MOD - [RelativeChunkDisplay]
	// NEO: SI - [SysInfo]
	HTREEITEM m_htiDrawSysInfoGraph;
	HTREEITEM m_htiShowSysInfoOnTitle;
	// NEO: SI END
	HTREEITEM m_htiUseChunkDots;	// NEO: MOD - [ChunkDots]
	HTREEITEM m_htiUseTreeStyle; 	// NEO: NTS - [NeoTreeStyle]
	HTREEITEM m_htiShowClientPercentage; // NEO: MOD - [Percentage]
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	HTREEITEM m_htiIP2Country;
		HTREEITEM m_htiIP2CountryNameDisable; 
		HTREEITEM m_htiIP2CountryNameShort; 
		HTREEITEM m_htiIP2CountryNameMidle; 
		HTREEITEM m_htiIP2CountryNameLong; 
		HTREEITEM m_htiIP2CountryShowFlag; 
#endif // IP2COUNTRY // NEO: IP2C END
	// NEO: NMX - [NeoMenuXP]
	HTREEITEM m_htiXPMenuStyle;
		HTREEITEM m_htiXPMenuStyleOffice;
		HTREEITEM m_htiXPMenuStyleStartMenu;
		HTREEITEM m_htiXPMenuStyleXP;
		HTREEITEM m_htiShowXPSideBar;
		HTREEITEM m_htiShowXPBitmap;
		HTREEITEM m_htiGrayMenuIcon;
	// NEO: NMX END

	// NEO: NXC - [NewExtendedCategories]
	bool	m_bShowCatNames;
	UINT	m_uShowCategoryFlags;
	bool	m_bActiveCatDefault;
	bool	m_bSelCatOnAdd;
	bool	m_bAutoSetResumeOrder;
	bool	m_bSmallFileDLPush;
	int		m_iSmallFileDLPush;
	bool	m_bStartDLInEmptyCats;
	int		m_iStartDLInEmptyCats;
	bool	m_bUseAutoCat;
	bool	m_bCheckAlreadyDownloaded;
	bool	m_bStartNextFileByPriority;
	// NEO: NXC END

	// NEO: NTB - [NeoToolbarButtons]
	UINT	m_uNeoToolbar;
	int		m_iNeoToolbarButtonCount;
	CArray<UINT> m_NeoToolbarButtons;
	// NEO: NTB END

	bool	m_bShowBanner; // NEO: NPB - [PrefsBanner]
	bool	m_bCollorShareFiles; // NEO: NSC - [NeoSharedCategories]
	bool	m_bSmoothStatisticsGraphs; // NEO: NBC - [NeoBandwidthControl] 
	UINT	m_uDisableAutoSort; // NEO: SE - [SortExtension]

	// NEO: NSTI - [NewSystemTrayIcon]
	bool	m_bShowSystemTrayUpload;
	bool	m_bThinSystemTrayBars;
	int		m_iTrayBarsMaxCollor;
	// NEO: NSTI END

	bool	m_bStaticTrayIcon; // NEO: STI - [StaticTray]

	// NEO: IM - [InvisibelMode]
	bool	m_bInvisibleMode;
	UINT	m_iInvisibleModeHotKeyModifier;
	TCHAR	m_cInvisibleModeHotKey;
	// NEO: IM END

	// NEO: TPP - [TrayPasswordProtection]
	bool	m_bTrayPasswordProtection;
	CString	m_sTrayPassword;
	// NEO: TPP END

	bool	m_bUsePlusSpeedMeter; // NEO: PSM - [PlusSpeedMeter]

	bool	m_bUseRelativeChunkDisplay; // NEO: MOD - [RelativeChunkDisplay]

	// NEO: SI - [SysInfo]
	bool	m_bDrawSysInfoGraph;
	bool	m_bShowSysInfoOnTitle;
	// NEO: SI END
	
	bool	m_bUseChunkDots;	// NEO: MOD - [ChunkDots]

	bool	m_bUseTreeStyle; 	// NEO: NTS - [NeoTreeStyle]

	bool	m_bShowClientPercentage; // NEO: MOD - [Percentage]

#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry]
	int		m_iIP2CountryNameMode; 
	UINT	m_uIP2CountryShowFlag; 
#endif // IP2COUNTRY // NEO: IP2C END

	// NEO: NMX - [NeoMenuXP]
	bool	m_bShowXPSideBar;
	bool	m_bShowXPBitmap;
	int		m_iXPMenuStyle;
	bool	m_bGrayMenuIcon;
	// NEO: NMX END

	void ClearAllMembers();

	//void Localize();
	void LoadSettings();

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT DrawTreeItemHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	// NEO: NTB - [NeoToolbarButtons]
	void ClearNeoToolbarMembers(bool bTree = false);
	void CreateNeoToolbarMembers(int *iImgNeoBtn);
	// NEO: NTB END

};
