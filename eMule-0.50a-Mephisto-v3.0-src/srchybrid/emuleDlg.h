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
#pragma once
#include "TrayDialog.h"
#include "MeterIcon.h"
// ==> XP Style Menu [Xanatos] - Stulle
/*
#include "TitleMenu.h"
*/
#include "MenuXP.h"
// <== XP Style Menu [Xanatos] - Stulle
#include "SpeedGraph.h" // High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88

namespace Kademlia {
	class CSearch;
	class CContact;
	class CEntry;
	class CUInt128;
};

class CChatWnd;
class CIrcWnd;
class CKademliaWnd;
class CKnownFileList; 
class CMainFrameDropTarget;
class CMuleStatusBarCtrl;
class CMuleToolbarCtrl;
class CPreferencesDlg;
class CSearchDlg;
class CServerWnd;
class CSharedFilesWnd;
class CStatisticsDlg;
class CTaskbarNotifier;
class CTransferDlg;
struct Status;
//Xman Splashscreen
/*
class CSplashScreen;
*/
class CSplashScreenEx;
//Xman end
class CMuleSystrayDlg;
class CMiniMule;

// emuleapp <-> emuleapp
#define OP_ED2KLINK				12000
#define OP_CLCOMMAND			12001
#define OP_COLLECTION			12002

#define	EMULE_HOTMENU_ACCEL		'x'
#define	EMULSKIN_BASEEXT		_T("eMuleSkin")

class CemuleDlg : public CTrayDialog
{
	friend class CTBHMM; // TBH: minimule - Max
	friend class CMuleToolbarCtrl;
	friend class CMiniMule;

public:
	CemuleDlg(CWnd* pParent = NULL);
	~CemuleDlg();

	enum { IDD = IDD_EMULE_DIALOG };

	static bool IsRunning();
	void ShowConnectionState();
	void ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink = NULL, bool bForceSoundOFF = false);
	void SendNotificationMail(int iMsgType, LPCTSTR pszText);
	void ShowUserCount();
	void ShowMessageState(UINT iconnr);
	void SetActiveDialog(CWnd* dlg);
	CWnd* GetActiveDialog() const																{ return activewnd; }
	void ShowTransferRate(bool forceAll=false);
    void ShowPing();
	void Localize();

#ifdef HAVE_WIN7_SDK_H
	void UpdateStatusBarProgress();
	void UpdateThumbBarButtons(bool initialAddToDlg=false);
	void OnTBBPressed(UINT id);
	void EnableTaskbarGoodies(bool enable);

	enum TBBIDS {
		TBB_FIRST,
		TBB_CONNECT=TBB_FIRST,
		TBB_DISCONNECT,
		TBB_THROTTLE,
		TBB_UNTHROTTLE,
		TBB_PREFERENCES,
		TBB_LAST = TBB_PREFERENCES
	};
#endif

	// Logging
	void AddLogText(UINT uFlags, LPCTSTR pszText);
	void AddServerMessageLine(UINT uFlags, LPCTSTR pszText);
	void ResetLog();
	void ResetDebugLog();
	void ResetServerInfo();
	void ResetLeecherLog();		//Xman Anti-Leecher-Log
	CString GetLastLogEntry();
	CString	GetLastDebugLogEntry();
	CString	GetAllLogEntries();
	CString	GetAllDebugLogEntries();
	CString GetServerInfoText();

	CString	GetConnectionStateString();
	UINT GetConnectionStateIconIndex() const;
	CString	GetTransferRateString();
	CString	GetUpDatarateString(UINT uUpDatarate = -1);
	CString	GetDownDatarateString(UINT uDownDatarate = -1);

	void StopTimer();
	void DoVersioncheck(bool manual);
	//Xman versions check
	void DoMVersioncheck(bool manual);
	//Xman end
	// ==> ScarAngel Version Check - Stulle
	void DoSVersioncheck(bool manual);
	// <== ScarAngel Version Check - Stulle
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	void DoDLPVersioncheck();
	void DoIPFilterVersioncheck();
	// <== Advanced Updates [MorphXT/Stulle] - Stulle
	void ApplyHyperTextFont(LPLOGFONT pFont);
	void ApplyLogFont(LPLOGFONT pFont);
	void ProcessED2KLink(LPCTSTR pszData);
	void SetStatusBarPartsSize();
	int ShowPreferences(UINT uStartPageID = (UINT)-1);
	bool IsPreferencesDlgOpen() const;
	bool IsTrayIconToFlash()	{ return m_iMsgIcon!=0; }
	void SetToolTipsDelay(UINT uDelay);
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	void StartUPnP(bool bReset = true, uint16 nForceTCPPort = 0, uint16 nForceUDPPort = 0);
	void RefreshUPnP(bool bRequestAnswer = false);
	*/
	// <== UPnP support [MoNKi] - leuk_he
	HBRUSH GetCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void RunMiniMule(); // TBH: minimule (open on tray) - Stulle

	virtual void TrayMinimizeToTrayChange();
	virtual void RestoreWindow();
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);

	CTransferDlg*	transferwnd;
	CServerWnd*		serverwnd;
	CPreferencesDlg* preferenceswnd;
	CSharedFilesWnd* sharedfileswnd;
	CSearchDlg*		searchwnd;
	CChatWnd*		chatwnd;
	CMuleStatusBarCtrl* statusbar;
	CStatisticsDlg*  statisticswnd;
	CIrcWnd*		ircwnd;
	CTaskbarNotifier* m_wndTaskbarNotifier;
	CReBarCtrl		m_ctlMainTopReBar;
	CMuleToolbarCtrl* toolbar;
	CKademliaWnd*	kademliawnd;
	CWnd*			activewnd;
	uint8			status;

protected:
	HICON			m_hIcon;
	bool			ready;
	bool			m_bStartMinimizedChecked;
	bool			m_bStartMinimized;
	WINDOWPLACEMENT m_wpFirstRestore;
	HICON			connicons[9];
	HICON			transicons[4];
	HICON			imicons[3];
	HICON			m_icoSysTrayCurrent;
	HICON			usericon;
	CMeterIcon		m_TrayIcon;
	HICON			m_icoSysTrayConnected;		// do not use those icons for anything else than the traybar!!!
	HICON			m_icoSysTrayDisconnected;	// do not use those icons for anything else than the traybar!!!
	HICON			m_icoSysTrayLowID;	// do not use those icons for anything else than the traybar!!!
	int				m_iMsgIcon;
	UINT			m_uLastSysTrayIconCookie;
	uint32			m_uUpDatarate;
	uint32			m_uDownDatarate;
	//Xman
	// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	uint32			m_uploadOverheadRate;
	uint32			m_downloadOverheadRate;
	//Xman end
	CImageList		imagelist;
	CTitleMenu		trayPopup;
	CMuleSystrayDlg* m_pSystrayDlg;
	CMainFrameDropTarget* m_pDropTarget;
	CMenu			m_SysMenuOptions;
	CMenu			m_menuUploadCtrl;
	CMenu			m_menuDownloadCtrl;
	char			m_acVCDNSBuffer[MAXGETHOSTSTRUCT];
	bool			m_iMsgBlinkState;
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	bool			m_bConnectRequestDelayedForUPnP;
	*/
	// <== UPnP support [MoNKi] - leuk_he
	bool			m_bKadSuspendDisconnect;
	bool			m_bEd2kSuspendDisconnect;
	bool			m_bInitedCOM;
#ifdef HAVE_WIN7_SDK_H
	CComPtr<ITaskbarList3>	m_pTaskbarList;
	THUMBBUTTON		m_thbButtons[TBB_LAST+1];

	TBPFLAG			m_currentTBP_state;
	float			m_prevProgress;
	HICON			m_ovlIcon;
#endif

	//Xman versions check
	char			m_acMVCDNSBuffer[MAXGETHOSTSTRUCT];
	char			m_acDLPBuffer[MAXGETHOSTSTRUCT]; //Xman DLP
	bool			m_bCheckwasDone;
	//Xman end
	// ==> ScarAngel Version Check - Stulle
	char			m_acSVCDNSBuffer[MAXGETHOSTSTRUCT];
	// <== ScarAngel Version Check - Stulle
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	char			m_acDLPAutoBuffer[MAXGETHOSTSTRUCT];
	char			m_acIPFilterAutoBuffer[MAXGETHOSTSTRUCT];
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

	// Splash screen
	//Xman new slpash-screen arrangement
	/*
	CSplashScreen *m_pSplashWnd;
	DWORD m_dwSplashTime;
	void ShowSplash();
	void DestroySplash();
	*/
	//Xman end

	// Mini Mule
	CMiniMule* m_pMiniMule;
	void DestroyMiniMule();

	CMap<UINT, UINT, LPCTSTR, LPCTSTR> m_mapTbarCmdToIcon;
	void CreateToolbarCmdIconMap();
	LPCTSTR GetIconFromCmdId(UINT uId);

	// Startup Timer
	UINT_PTR m_hTimer;
	static void CALLBACK StartupTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);

	// ==> UPnP support [MoNKi] - leuk_he
	/*
	// UPnP TimeOutTimer
	UINT_PTR m_hUPnPTimeOutTimer;
	static void CALLBACK UPnPTimeOutTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);
	*/
	// <== UPnP support [MoNKi] - leuk_he

public: // Run eMule as NT Service [leuk_he/Stulle] - Stulle
	void StartConnection();
	void CloseConnection();
protected: // Run eMule as NT Service [leuk_he/Stulle] - Stulle
	void MinimizeWindow();
	void PostStartupMinimized();
	void UpdateTrayIcon(int iPercent);
	void ShowConnectionStateIcon();
	void ShowTransferStateIcon();
	void ShowUserStateIcon();
	void AddSpeedSelectorMenus(CMenu* addToMenu);
	//Xman
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	/*
	int  GetRecMaxUpload();
	*/
public:
	float  GetRecMaxUpload();
protected:
	//Xman end
	void LoadNotifier(CString configuration);
	bool notifierenabled;
	void ShowToolPopup(bool toolsonly = false);
	void SetAllIcons();
	bool CanClose();
	int MapWindowToToolbarButton(CWnd* pWnd) const;
	CWnd* MapToolbarButtonToWindow(int iButtonID) const;
	int GetNextWindowToolbarButton(int iButtonID, int iDirection = 1) const;
	bool IsWindowToolbarButton(int iButtonID) const;
	void SetTaskbarIconColor();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual void OnCancel();
	virtual void OnOK();
	virtual void OnTrayRButtonUp(CPoint pt);
	virtual void OnTrayLButtonUp(CPoint pt);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnClose();
	afx_msg void OnDestroy();
	afx_msg void OnSize(UINT nType,int cx,int cy);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBnClickedConnect();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedHotmenu();
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnDeviceChange(UINT nEventType, DWORD_PTR dwData);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg LRESULT OnUserChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnKickIdle(UINT nWhy, long lIdleCount);
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg BOOL OnChevronPushed(UINT id, NMHDR *pnm, LRESULT *pResult);
	afx_msg LRESULT OnPowerBroadcast(WPARAM wParam, LPARAM lParam);

	// ==> XP Style Menu [Xanatos] - Stulle
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	// <== XP Style Menu [Xanatos] - Stulle

	// quick-speed changer -- based on xrmb
	afx_msg void QuickSpeedUpload(UINT nID);
	afx_msg void QuickSpeedDownload(UINT nID);
	afx_msg void QuickSpeedOther(UINT nID);
	// end of quick-speed changer
	
	afx_msg LRESULT OnTaskbarNotifierClicked(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnWMData(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileHashed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnHashFailed(WPARAM wParam,LPARAM lParam);
	//Xman
	// BEGIN SLUGFILLER: SafeHash
	afx_msg LRESULT OnPartHashedOK(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnPartHashedOKNoAICH(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnPartHashedCorrupt(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnPartHashedCorruptNoAICH(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnPartHashedOKAICHRecover(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnPartHashedCorruptAICHRecover(WPARAM wParam,LPARAM lParam);
	// END SLUGFILLER: SafeHash
	// BEGIN SiRoB: ReadBlockFromFileThread
	afx_msg LRESULT OnReadBlockFromFileDone(WPARAM wParam,LPARAM lParam);
	// END SiRoB: ReadBlockFromFileThread
	// BEGIN SiRoB: Flush Thread
	afx_msg LRESULT OnFlushDone(WPARAM wParam,LPARAM lParam);
	// END SiRoB: Flush Thread
	//MORPH START - Added by SiRoB, Import Parts - added by zz_fly
	afx_msg LRESULT OnImportPart(WPARAM wParam,LPARAM lParam);
	//MORPH END   - Added by SiRoB, Import Parts
	afx_msg LRESULT OnSaveDone(WPARAM wParam,LPARAM lParam); // File Settings [sivka/Stulle] - Stulle
	afx_msg LRESULT OnSaveKnownDone(WPARAM wParam,LPARAM lParam); // Threaded Known Files Saving [Stulle] - Stulle

	afx_msg LRESULT OnFileAllocExc(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileCompleted(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileOpProgress(WPARAM wParam,LPARAM lParam);

	//Framegrabbing
	afx_msg LRESULT OnFrameGrabFinished(WPARAM wParam,LPARAM lParam);

	afx_msg LRESULT OnAreYouEmule(WPARAM, LPARAM);

#ifdef HAVE_WIN7_SDK_H
	afx_msg LRESULT OnTaskbarBtnCreated (WPARAM, LPARAM);
#endif

	//Webinterface
	afx_msg LRESULT OnWebGUIInteraction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerFileRename(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebAddDownloads(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebSetCatPrio(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddRemoveFriend(WPARAM wParam, LPARAM lParam);

	// VersionCheck DNS
	afx_msg LRESULT OnVersionCheckResponse(WPARAM wParam, LPARAM lParam);

	//Xman versions check
	afx_msg LRESULT OnMVersionCheckResponse(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDLPVersionCheckResponse(WPARAM wParam, LPARAM lParam); //Xman DLP
	//Xman end

	afx_msg LRESULT DoTimer(WPARAM wParam, LPARAM lParam); //Xman process timer code via messages (Xanatos)

	// ScarAngel Version Check - Stulle
	afx_msg	LRESULT	OnSVersionCheckResponse(WPARAM wParam, LPARAM lParam);
	// Advanced Updates [MorphXT/Stulle] - Stulle
	afx_msg LRESULT OnDLPAutoVerCheckResponse(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnIPFilterAutoVerCheckResponse(WPARAM wParam, LPARAM lParam);

	// Run eMule as NT Service [leuk_he/Stulle] - Stulle
	afx_msg LRESULT  OnServiceStatus(WPARAM wParam, LPARAM lParam);

	// Peercache DNS
	afx_msg LRESULT OnPeerCacheResponse(WPARAM wParam, LPARAM lParam);

	// Mini Mule
	afx_msg LRESULT OnCloseMiniMule(WPARAM wParam, LPARAM lParam);

	// Terminal Services
	afx_msg LRESULT OnConsoleThreadEvent(WPARAM wParam, LPARAM lParam);

	// UPnP
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	afx_msg LRESULT OnUPnPResult(WPARAM wParam, LPARAM lParam);
	*/
	// <== UPnP support [MoNKi] - leuk_he

// ==> Show in MSN7 [TPT] - Stulle
protected:
	DWORD m_dwMSNtime;
	uint8 m_uMSNup2Date;
// <== Show in MSN7 [TPT] - Stulle

	// ==> TBH: Backup [TBH/EastShare/MorphXT] - Stulle
public:
	void SaveSettings (bool _shutdown=false);
	// <== TBH: Backup [TBH/EastShare/MorphXT] - Stulle

	// ==> Invisible Mode [TPT/MoNKi] - Stulle
	BOOL	RegisterInvisibleHotKey();
	BOOL	UnRegisterInvisibleHotKey();
protected:
	LRESULT	OnHotKey(WPARAM wParam, LPARAM lParam);

	// Allows "invisible mode" on multiple instances of eMule
	afx_msg LRESULT OnRestoreWindowInvisibleMode(WPARAM, LPARAM);
	static BOOL CALLBACK AskEmulesForInvisibleMode(HWND hWnd, LPARAM lParam);

private:
	void	ToggleShow();
	void	ToggleHide();
	BOOL	b_TrayWasVisible;
	BOOL	b_WindowWasVisible;
	bool	b_HideApp;

	// <== Invisible Mode [TPT/MoNKi] - Stulle

// ==> High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88
public:
	CSpeedGraph m_co_UpTrafficGraph;
	CSpeedGraph m_co_DownTrafficGraph;

	void Update_TrafficGraph();
	void Reposition_TrafficGraph();
// <== High resolution speedmeter on toolbar [eFMod/Stulle] - Myth88

	// ==> Design Settings [eWombat/Stulle] - Max
private:
	HBRUSH	m_hbrWndClr;
public:
	HBRUSH	GetWndClr()	{return m_hbrWndClr;}
	// <== Design Settings [eWombat/Stulle] - Max

	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	void	DownloadDLP();
	void	CheckIPFilter();
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

	// ==> Completed in Tray [Stulle] - Stulle
protected:
	HICON m_icoSysTrayConnectedPlus;
	HICON m_icoSysTrayDisconnectedPlus;
	HICON m_icoSysTrayLowIDPlus;
	DWORD m_dwTrayTime;
	bool m_bTrayBool;
	bool m_bTrayBoolOld;
	// <== Completed in Tray [Stulle] - Stulle

public:
	void	CloseeMule(); // Diabolic Easteregg [Stulle] - Mephisto
};


enum EEMuleAppMsgs
{
	//thread messages
	TM_FINISHEDHASHING = WM_APP + 10,
	TM_HASHFAILED,
	//Xman
	// BEGIN SLUGFILLER: SafeHash - new handling
	TM_PARTHASHEDOK,
	TM_PARTHASHEDOKNOAICH,
	TM_PARTHASHEDCORRUPT,
	TM_PARTHASHEDCORRUPTNOAICH,
	TM_PARTHASHEDOKAICHRECOVER,
	TM_PARTHASHEDCORRUPTAICHRECOVER,
	// END SLUGFILLER: SafeHash
	TM_READBLOCKFROMFILEDONE, // SiRoB: ReadBlockFromFileThread
	TM_FLUSHDONE, // SiRoB: Flush Thread
	TM_IMPORTPART, //MORPH - Added by SiRoB, Import Parts - added by zz_fly
	TM_DOTIMER, //Xman process timer code via messages (Xanatos)
	TM_SAVEDONE, // File Settings [sivka/Stulle] - Stulle
	TM_SAVEKNOWNDONE, // Threaded Known Files Saving [Stulle] - Stulle
	TM_FRAMEGRABFINISHED,
	TM_FILEALLOCEXC,
	TM_FILECOMPLETED,
	TM_FILEOPPROGRESS,
	TM_CONSOLETHREADEVENT
};

enum EWebinterfaceOrders
{
	WEBGUIIA_UPDATEMYINFO = 1,
	WEBGUIIA_WINFUNC,
	WEBGUIIA_UPD_CATTABS,
	WEBGUIIA_UPD_SFUPDATE,
	WEBGUIIA_UPDATESERVER,
	WEBGUIIA_STOPCONNECTING,
	WEBGUIIA_CONNECTTOSERVER,
	WEBGUIIA_DISCONNECT,
	WEBGUIIA_SERVER_REMOVE,
	WEBGUIIA_SHARED_FILES_RELOAD,
	WEBGUIIA_ADD_TO_STATIC,
	WEBGUIIA_REMOVE_FROM_STATIC,
	WEBGUIIA_UPDATESERVERMETFROMURL,
	WEBGUIIA_SHOWSTATISTICS,
	WEBGUIIA_DELETEALLSEARCHES,
	WEBGUIIA_KAD_BOOTSTRAP,
	WEBGUIIA_KAD_START,
	WEBGUIIA_KAD_STOP,
	WEBGUIIA_KAD_RCFW
};

// ==> Invisible Mode [TPT/MoNKi] - Stulle
enum EEmuleHotKeysIDs
{
	HOTKEY_INVISIBLEMODE_ID
};

enum EEMuleInvisibleModeEnumOptions
{
	INVMODE_RESTOREWINDOW,
	INVMODE_REGISTERHOTKEY,
	INVMODE_HIDEWINDOW
};
// <== Invisible Mode [TPT/MoNKi] - Stulle