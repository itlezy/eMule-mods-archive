//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

#include "Addons/SpeedGraph/SpeedGraph.h" // X-Ray :: Speedgraph

// X-Ray :: Toolbar :: Start
#include "EnBitmap.h"
#include "Addons/SkinButtons/xSkinButton.h"
// X-Ray :: Toolbar :: End
//>>> WiZaRd::WebBrowser [Pruna]
#include "./EMF/WebBrowser.h" 
#include "./EMF/WebTool.h" 
//<<< WiZaRd::WebBrowser [Pruna]

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
class CPreferencesDlg;
class CSearchDlg;
class CServerWnd;
class CSharedFilesWnd;
class CMediaPlayerWnd; //>>> WiZaRd::MediaPlayer
class CStatisticsDlg;
class CTaskbarNotifier;
class CTransferWnd;
struct Status;
//class CSplashScreen; //Xman Splashscreen
class CSplashScreenEx; //Xman Splashscreen
class CWebBrowserWnd; // Added by thilon on 2006.08.01



class CMuleSystrayDlg;
class CMiniMule;

// emuleapp <-> emuleapp
#define OP_ED2KLINK				12000
#define OP_CLCOMMAND			12001
#define OP_COLLECTION			12002

#define	EMULE_HOTMENU_ACCEL		'x'

// #define	EMULSKIN_BASEEXT		_T("eMuleSkin") // X-Ray :: Toolbar :: Cleanup

class CemuleDlg : public CTrayDialog
{
	// friend class CMuleToolbarCtrl; // X-Ray :: Toolbar :: Cleanup
	friend class CMiniMule;

public:
	CemuleDlg(CWnd* pParent = NULL);
	~CemuleDlg();
	BOOL OnSystemPowerOff(bool bOff); //Dreamule PowerOff
	enum { IDD = IDD_EMULE_DIALOG };
	bool IsRunning();
	void ShowConnectionState();
	void ShowNotifier(LPCTSTR pszText, int iMsgType, LPCTSTR pszLink = NULL, bool bForceSoundOFF = false);
	void SendNotificationMail(int iMsgType, LPCTSTR pszText);
	void ShowUserCount();
	void ShowMessageState(UINT iconnr);
	void SetActiveDialog(CWnd* dlg);
	void boi();
	void ShowTransferRate(bool forceAll=false);
    void ShowPing();
	void Localize();

	// Logging
	void AddLogText(UINT uFlags, LPCTSTR pszText);
	void AddServerMessageLine(UINT uFlags, LPCTSTR pszText);
	void ResetLog();
	void ResetDebugLog();
	void ResetServerInfo();
	void ResetLeecherLog();		//Xman Anti-Leecher-Log
	CString Arquivotocando();//Dream BugFix- Runnign file goes hashing
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
	// Tux: LiteMule: Remove Version Check
	void ApplyHyperTextFont(LPLOGFONT pFont);
	void ApplyLogFont(LPLOGFONT pFont);
	void ProcessED2KLink(LPCTSTR pszData);
	void SetStatusBarPartsSize();
	int ShowPreferences(UINT uStartPageID = (UINT)-1);
	bool IsPreferencesDlgOpen() const;
	bool IsTrayIconToFlash()	{ return m_iMsgIcon!=0; }
	void SetToolTipsDelay(UINT uDelay);

	virtual void TrayMinimizeToTrayChange();
	virtual void RestoreWindow();
	virtual void Mostar();
	virtual void HtmlHelp(DWORD_PTR dwData, UINT nCmd = 0x000F);

	CTransferWnd*	transferwnd;
	CServerWnd*		serverwnd;
	CPreferencesDlg* preferenceswnd;
	CSharedFilesWnd* sharedfileswnd;
	CMediaPlayerWnd* mediaplayerwnd; //>>> WiZaRd::MediaPlayer
	CSearchDlg*		searchwnd;
	CChatWnd*		chatwnd;
	CMuleStatusBarCtrl* statusbar;
	CStatisticsDlg*  statisticswnd;
	CIrcWnd*		ircwnd;
	CTaskbarNotifier* m_wndTaskbarNotifier;
	// X-Ray :: Toolbar :: Cleanup :: Start
	/*
	CReBarCtrl		m_ctlMainTopReBar;
	CMuleToolbarCtrl* toolbar;
	*/
	// X-Ray :: Toolbar :: Cleanup :: Start
	CKademliaWnd*	kademliawnd;
	CWnd*			activewnd;
	uint8			status;
//>>> WiZaRd::WebBrowser [Pruna]
	CWebBrowserWnd*	webbrowser; //Added by thilon on 2006.08.01
//<<< WiZaRd::WebBrowser [Pruna]
	// X-Ray :: Toolbar :: Start
	CEnBitmap m_co_ToolLeft;
	CEnBitmap m_co_ToolMid;
	CEnBitmap m_co_ToolRight;
	void InvalidateButtons();
	// X-Ray :: Toolbar :: End

	// X-Ray :: Speedgraph :: Start 
	CSpeedGraph m_co_UpTrafficGraph;
	CSpeedGraph m_co_DownTrafficGraph;
	void Update_TrafficGraph();
	void SetSpeedMeterRange(int iValue1, int iValue2){
		m_co_UpTrafficGraph.Init_Graph(_T("Up"),iValue1); 
		m_co_DownTrafficGraph.Init_Graph(_T("Down"),iValue2);
	}
	// X-Ray :: Speedgraph :: End

protected:
	HICON			m_hIcon;
	bool			ready;
	bool			m_bStartMinimizedChecked;
	bool			m_bStartMinimized;
	bool			m_bTrayState;	//TK4 Mod V1.3a onward
	bool			m_bWasMiniMule; //TK4 Mod V1.3a onward
	WINDOWPLACEMENT m_wpFirstRestore;
	// X-Ray :: Statusbar :: Start
	/*
	HICON			connicons[9];
	HICON			transicons[4];
	*/
	HICON			connicons[6];
	HICON			transicons[2];
	// X-Ray :: Statusbar :: End
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
	// Tux: LiteMule: Remove Version Check
	bool			m_iMsgBlinkState;
		// X-Ray :: Statusbar :: Start
	HICON			fileicon;
	HICON			sysinfoicon;
	// X-Ray :: Statusbar :: End

	// Splash screen
	//Xman new slpash-screen arrangement
	/*
	CSplashScreenEx *m_pSplashWnd; //Xman Splashscreen
	DWORD m_dwSplashTime;
	void ShowSplash();
	void DestroySplash();
	*/

	// Mini Mule
	CMiniMule* m_pMiniMule;
	void DestroyMiniMule();

	// X-Ray :: Toolbar :: Cleanup :: Start
	/*
	CMap<UINT, UINT, LPCTSTR, LPCTSTR> m_mapTbarCmdToIcon;
	void CreateToolbarCmdIconMap();
	LPCTSTR GetIconFromCmdId(UINT uId);
	*/
	// X-Ray :: Toolbar :: Cleanup :: End

	// Startup Timer
	UINT_PTR m_hTimer;
	static void CALLBACK StartupTimer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);

	void StartConnection();
	void CloseConnection();
	void MinimizeWindow();
	void PostStartupMinimized();
	void UpdateTrayIcon(int iPercent);
	void ShowConnectionStateIcon();
	void ShowTransferStateIcon();
	void ShowUserStateIcon();
	void AddSpeedSelectorMenus(CMenu* addToMenu);
	//Xman
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
public:
	float  GetRecMaxUpload();
protected:
	//Xman end

	void LoadNotifier(CString configuration);
	bool notifierenabled;
	void ShowToolPopup(bool toolsonly = false);
	void SetAllIcons();
	bool CanClose();
	// X-Ray :: Toolbar :: Cleanup :: Start
	/*
	int MapWindowToToolbarButton(CWnd* pWnd) const;
	CWnd* MapToolbarButtonToWindow(int iButtonID) const;
	int GetNextWindowToolbarButton(int iButtonID, int iDirection = 1) const;
	bool IsWindowToolbarButton(int iButtonID) const;
	*/
	// X-Ray :: Toolbar :: Cleanup :: End

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
	afx_msg void OnSettingChange(UINT uFlags, LPCTSTR lpszSection);
	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnEndSession(BOOL bEnding);
	afx_msg LRESULT OnUserChanged(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnKickIdle(UINT nWhy, long lIdleCount);
	afx_msg void OnShowWindow( BOOL bShow, UINT nStatus );

	//boi
	afx_msg void OnActivate(UINT nState, CWnd* pWndOther, BOOL bMinimized);
	//boi

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
	afx_msg LRESULT OnPartHashedCorrupt(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnPartHashedOKAICHRecover(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnPartHashedCorruptAICHRecover(WPARAM wParam,LPARAM lParam);
	// END SLUGFILLER: SafeHash
	// BEGIN SiRoB: ReadBlockFromFileThread
	afx_msg LRESULT OnReadBlockFromFileDone(WPARAM wParam,LPARAM lParam);
	// BEGIN SiRoB: ReadBlockFromFileThread
	// END SiRoB: Flush Thread
	afx_msg LRESULT OnFlushDone(WPARAM wParam,LPARAM lParam);
	// END SiRoB: Flush Thread

	afx_msg LRESULT OnFileAllocExc(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileCompleted(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileOpProgress(WPARAM wParam,LPARAM lParam);

	//TK4 Mod 1.3a Onward - Handle hot key
	afx_msg LRESULT CemuleDlg::OnHotKey(WPARAM wp, LPARAM);

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

	// Tux: LiteMule: Remove Version Check
	afx_msg LRESULT DoTimer(WPARAM wParam, LPARAM lParam); //Xman process timer code via messages (Xanatos)

	// Peercache DNS
	afx_msg LRESULT OnPeerCacheResponse(WPARAM wParam, LPARAM lParam);

	// Mini Mule
	afx_msg LRESULT OnCloseMiniMule(WPARAM wParam, LPARAM lParam);

	// Terminal Services
	afx_msg LRESULT OnConsoleThreadEvent(WPARAM wParam, LPARAM lParam);

	// X-Ray :: Toolbar :: Start
	CxSkinButton	m_co_ConnectBtn;
	CxSkinButton	m_co_TransferBtn;
	CxSkinButton	m_co_SearchBtn;
	CxSkinButton	m_co_FilesBtn;
	CxSkinButton	m_co_MediaPlayerBtn; //>>> WiZaRd::MediaPlayer
	CxSkinButton	m_co_MessagesBtn;
	CxSkinButton	m_co_IrcBtn;
	CxSkinButton	m_co_KademliaBtn;
	CxSkinButton	m_co_ServerBtn;
	CxSkinButton	m_co_StatisticBtn;
	CxSkinButton	m_co_PreferencesBtn;

	// X-Ray :: Toolbar :: End

//>>> WiZaRd::Minimize on Close
public:
	afx_msg void	DoClose(); 
//<<< WiZaRd::Minimize on Close
	afx_msg void OnBnClickedTbBtnMediaplayer();
};


enum EEMuleAppMsgs
{
	//thread messages
	TM_FINISHEDHASHING = WM_APP + 10,
	TM_HASHFAILED,
	//Xman
	// BEGIN SLUGFILLER: SafeHash - new handling
	TM_PARTHASHEDOK,
	TM_PARTHASHEDCORRUPT,
	TM_PARTHASHEDOKAICHRECOVER,
	TM_PARTHASHEDCORRUPTAICHRECOVER,
	// END SLUGFILLER: SafeHash
	TM_READBLOCKFROMFILEDONE, // SiRoB: ReadBlockFromFileThread
	TM_FLUSHDONE, // SiRoB: Flush Thread
	TM_DOTIMER, //Xman process timer code via messages (Xanatos)
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
