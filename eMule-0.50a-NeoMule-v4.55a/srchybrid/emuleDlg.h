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
#include "TitleMenu.h"
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
#include "Neo\GUI\ToolTips\PPToolTip.h"
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

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
class CTransferWnd;
struct Status;
//class CSplashScreen; // NEO: SS - [SplashScreen] <-- Xanatos --
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
	friend class CMuleToolbarCtrl;
	friend class CMiniMule;

public:
	CemuleDlg(CWnd* pParent = NULL);
	~CemuleDlg();

	enum { IDD = IDD_EMULE_DIALOG };

	bool IsRunning();
	bool IsMainThread(); // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	void ShowConnectionState();
	void ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink = NULL, bool bForceSoundOFF = false);
	void SendNotificationMail(int iMsgType, LPCTSTR pszText);
	void ShowUserCount();
	void ShowMessageState(UINT iconnr);
	void SetActiveDialog(CWnd* dlg);
	void ShowTransferRate(bool forceAll=false);
    void ShowPing();
	void Localize();

	// Logging
	void AddLogText(UINT uFlags, LPCTSTR pszText);
	void AddServerMessageLine(UINT uFlags, LPCTSTR pszText);
	void ResetLog();
	void ResetDebugLog();
	void ResetServerInfo();
	void ResetModLog(); // NEO: ML - [ModLog] <-- Xanatos --
	CString GetLastLogEntry();
	CString	GetLastDebugLogEntry();
	CString	GetLastModLogEntry(); // NEO: ML - [ModLog] <-- Xanatos --
	CString	GetAllLogEntries();
	CString	GetAllDebugLogEntries();
	CString	GetAllModLogEntries(); // NEO: ML - [ModLog] <-- Xanatos --
	CString GetServerInfoText();

	CString	GetConnectionStateString();
	UINT GetConnectionStateIconIndex() const;
	CString	GetTransferRateString();
	CString	GetUpDatarateString(UINT uUpDatarate = -1);
	CString	GetDownDatarateString(UINT uDownDatarate = -1);

	void StopTimer();
	void DoVersioncheck(bool manual);
	void DoNeoVersioncheck(bool manual); // NEO: NVC - [NeoVersionCheck] <-- Xanatos --
	void ApplyHyperTextFont(LPLOGFONT pFont);
	void ApplyLogFont(LPLOGFONT pFont);
	void ProcessED2KLink(LPCTSTR pszData);
	void SetStatusBarPartsSize();
	int ShowPreferences(UINT uStartPageID = (UINT)-1);
	bool IsPreferencesDlgOpen() const;
	bool IsTrayIconToFlash()	{ return m_iMsgIcon!=0; }
	void UpdateTrayBarsColors(); // NEO: NSTI - [NewSystemTrayIcon] <-- Xanatos --
	// NEO: IM - [InvisibelMode] -- Xanatos -->
	BOOL RegisterInvisibleHotKey();
	BOOL UnRegisterInvisibleHotKey();
	// NEO: IM END <-- Xanatos --
	void SetToolTipsDelay(UINT uDelay);
	void StartUPnP(bool bReset = true, uint16 nForceTCPPort = 0, uint16 nForceUDPPort = 0);
	HBRUSH GetCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	void SetTTDelay();
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	virtual void TrayMinimizeToTrayChange();
	virtual void RestoreWindow();
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);

	CTransferWnd*	transferwnd;
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

	// NEO: TPP - [TrayPasswordProtection] -- Xanatos -->
	bool			m_TrayLocked;
	CString			m_TrayPassword; 
	// NEO: TPP END <-- Xanatos --

#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
private:
	CPPToolTip		m_ttip;
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

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
	CImageList		imagelist;
	CTitleMenu		trayPopup;
	CMuleSystrayDlg* m_pSystrayDlg;
	CMainFrameDropTarget* m_pDropTarget;
	CMenu			m_SysMenuOptions;
	CMenu			m_menuUploadCtrl;
	CMenu			m_menuDownloadCtrl;
	char			m_acVCDNSBuffer[MAXGETHOSTSTRUCT];
	char			m_acNVCDNSBuffer[MAXGETHOSTSTRUCT]; // NEO: NVC - [NeoVersionCheck] <-- Xanatos --
	bool			m_iMsgBlinkState;
	bool			m_bConnectRequestDelayedForUPnP;
	bool			m_bKadSuspendDisconnect;
	bool			m_bEd2kSuspendDisconnect;

	// NEO: SS - [SplashScreen] -- Xanatos --
	// Splash screen
	/*CSplashScreen *m_pSplashWnd;
	DWORD m_dwSplashTime;
	void ShowSplash();
	void DestroySplash();*/

	// Mini Mule
	CMiniMule* m_pMiniMule;
	void DestroyMiniMule();

	CMap<UINT, UINT, LPCTSTR, LPCTSTR> m_mapTbarCmdToIcon;
	void CreateToolbarCmdIconMap();
	LPCTSTR GetIconFromCmdId(UINT uId);

	// Startup Timer
	UINT_PTR m_hTimer;
	static void CALLBACK StartupTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);
	static void			 StartupTimer(); // NEO: ND - [NeoDebug] <-- Xanatos --
	static BOOL CALLBACK AskEmulesForInvisibleMode(HWND hWnd, LPARAM lParam); // NEO: IM - [InvisibelMode] <-- Xanatos --

	// UPnP TimeOutTimer
	UINT_PTR m_hUPnPTimeOutTimer;
	static void CALLBACK UPnPTimeOutTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);

	BOOL ShowWindow(int nCmdShow); // NEO: TPP - [TrayPasswordProtection] <-- Xanatos --
	void StartConnection();
	void CloseConnection();
	void MinimizeWindow();
	void PostStartupMinimized();
	//void UpdateTrayIcon(int iPercent);
	void UpdateTrayIcon(int iDown, int iUp); // NEO: NSTI - [NewSystemTrayIcon] <-- Xanatos --
	void ShowConnectionStateIcon();
	void ShowTransferStateIcon();
	void ShowUserStateIcon();
	void AddSpeedSelectorMenus(CMenu* addToMenu);
	int  GetRecMaxUpload();
	void LoadNotifier(CString configuration);
	bool notifierenabled;
	void ShowToolPopup(bool toolsonly = false);
	void SetAllIcons();
	bool CanClose();
	int MapWindowToToolbarButton(CWnd* pWnd) const;
	CWnd* MapToolbarButtonToWindow(int iButtonID) const;
	int GetNextWindowToolbarButton(int iButtonID, int iDirection = 1) const;
	bool IsWindowToolbarButton(int iButtonID) const;

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
	afx_msg void OnBnClickedButton2();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnBnClickedHotmenu();
	afx_msg LRESULT OnMenuChar(UINT nChar, UINT nFlags, CMenu* pMenu);
	afx_msg void OnSysColorChange();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg LRESULT OnUserChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnKickIdle(UINT nWhy, long lIdleCount);
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );
	afx_msg BOOL OnChevronPushed(UINT id, NMHDR *pnm, LRESULT *pResult);
	afx_msg LRESULT OnPowerBroadcast(WPARAM wParam, LPARAM lParam);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct); // NEO: NMX - [NeoMenuXP] <-- Xanatos --
#ifdef NEWTOOLTIPS // NEO: NTT - [NewToolTips] -- Xanatos -->
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMH, LRESULT *pResult);
#endif // NEWTOOLTIPS // NEO: NTT END <-- Xanatos --

	// quick-speed changer -- based on xrmb
	afx_msg void QuickSpeedUpload(UINT nID);
	afx_msg void QuickSpeedDownload(UINT nID);
	afx_msg void QuickSpeedOther(UINT nID);
	// end of quick-speed changer
	
	afx_msg LRESULT OnTaskbarNotifierClicked(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnWMData(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT DoTimer(WPARAM wParam,LPARAM lParam); // NEO: ND - [NeoDebug] <-- Xanatos --
	afx_msg LRESULT OnFileHashed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnHashFailed(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFlushDone(WPARAM wParam,LPARAM lParam); //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
	afx_msg LRESULT OnPartHashed(WPARAM wParam,LPARAM lParam); // SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	afx_msg LRESULT OnBlockHashed(WPARAM wParam,LPARAM lParam); // NEO: SafeHash // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	afx_msg LRESULT OnReadBlockFromFileDone(WPARAM wParam,LPARAM lParam); // NEO: RBT - [ReadBlockThread] <-- Xanatos --
	afx_msg LRESULT OnImportPart(WPARAM wParam,LPARAM lParam); // NEO: PIX - [PartImportExport] <-- Xanatos --
	afx_msg LRESULT OnFileImported(WPARAM wParam, LPARAM lParam); // NEO: PIX - [PartImportExport] <-- Xanatos --
	afx_msg LRESULT OnFileMoved(WPARAM wParam,LPARAM lParam); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	afx_msg LRESULT OnFileAllocExc(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileCompleted(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileOpProgress(WPARAM wParam,LPARAM lParam);

	//Framegrabbing
	afx_msg LRESULT OnFrameGrabFinished(WPARAM wParam,LPARAM lParam);

	afx_msg LRESULT OnAreYouEmule(WPARAM, LPARAM);

	//Webinterface
	afx_msg LRESULT OnWebGUIInteraction(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerFileRename(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebAddDownloads(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebSetCatPrio(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAddRemoveFriend(WPARAM wParam, LPARAM lParam);

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	afx_msg LRESULT OnArgosResult(WPARAM wParam, LPARAM lParam);
#endif // ARGOS // NEO: NA END <-- Xanatos --

	// VersionCheck DNS
	afx_msg LRESULT OnVersionCheckResponse(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnNeoVersionCheckResponse(WPARAM wParam, LPARAM lParam); // NEO: NVC - [NeoVersionCheck] <-- Xanatos --

	// Peercache DNS
	afx_msg LRESULT OnPeerCacheResponse(WPARAM wParam, LPARAM lParam);

	// Mini Mule
	afx_msg LRESULT OnCloseMiniMule(WPARAM wParam, LPARAM lParam);

	// Terminal Services
	afx_msg LRESULT OnConsoleThreadEvent(WPARAM wParam, LPARAM lParam);

	// UPnP
	afx_msg LRESULT OnUPnPResult(WPARAM wParam, LPARAM lParam);

	// NEO: IM - [InvisibelMode] -- Xanatos -->
	LRESULT	OnHotKey(WPARAM wParam, LPARAM lParam);
	// Allows "invisible mode" on multiple instances of eMule
	afx_msg LRESULT OnRestoreWindowInvisibleMode(WPARAM, LPARAM);
	// NEO: IM END <-- Xanatos --
};


enum EEMuleAppMsgs
{
	//thread messages
	TM_DOTIMER = WM_APP + 9, // NEO: ND - [NeoDebug] <-- Xanatos --
	TM_FINISHEDHASHING = WM_APP + 10,
	TM_HASHFAILED,
	TM_FLUSHDONE, //MORPH - Added by SiRoB, Flush Thread // NEO: FFT - [FileFlushThread] <-- Xanatos --
	TM_PARTHASHED, // SLUGFILLER: SafeHash - new handling // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	TM_BLOCKHASHED, // NEO: SafeHash // NEO: SCV - [SubChunkVerification] <-- Xanatos --
	TM_READBLOCKFROMFILEDONE, // NEO: RBT - [ReadBlockThread] <-- Xanatos --
	TM_IMPORTPART, // NEO: PIX - [PartImportExport] <-- Xanatos --
	TM_FILEIMPORTED, // NEO: PIX - [PartImportExport] <-- Xanatos --
	TM_FILEMOVED, // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
	TM_FRAMEGRABFINISHED,
	TM_FILEALLOCEXC,
	TM_FILECOMPLETED,
	TM_FILEOPPROGRESS,
	TM_CONSOLETHREADEVENT,
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	TM_ARGOS_RESULT,
#endif // ARGOS // NEO: NA END <-- Xanatos --
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

// NEO: IM - [InvisibelMode] -- Xanatos -->
enum EEmuleHotKeysIDs
{
	HOTKEY_INVISIBLEMODE_ID
};

enum EEMuleInvisibleModeEnumOptions
{
	INVMODE_RESTOREWINDOW,
	INVMODE_REGISTERHOTKEY
};
// NEO: IM END <-- Xanatos --
