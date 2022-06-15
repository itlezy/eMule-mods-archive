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
#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif
#include "resource.h"
#include "Neo\ReadWriteLock.h"	// SLUGFILLER: SafeHash // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --

#define	DEFAULT_NICK		thePrefs.GetHomepageBaseURL()
#define	DEFAULT_TCP_PORT_OLD	4662
#define	DEFAULT_UDP_PORT_OLD	(DEFAULT_TCP_PORT_OLD+10)

#define PORTTESTURL			_T("http://porttest.emule-project.net/connectiontest.php?tcpport=%i&udpport=%i&lang=%i")

class CSearchList;
class CUploadQueue;
class CListenSocket;
class CDownloadQueue;
class CScheduler;
class UploadBandwidthThrottler;
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
class DownloadBandwidthThrottler;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
class CBandwidthControl;
#else
class LastCommonRouteFinder;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
class CemuleDlg;
class CClientList;
class CKnownFileList;
class CServerConnect;
class CServerList;
class CSharedFileList;
class CClientCreditsList;
class CFriendList;
class CClientUDPSocket;
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
class CNatManager;
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
class CIPFilter;
class CWebServer;
class CMMServer;
class CAbstractFile;
class CUpDownClient;
class CPeerCacheFinder;
class CFirewallOpener;
class CUPnPImplWrapper;
class CSplashScreenEx; // NEO: SS - [SplashScreen] <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
class CSourceList;
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
class CLanCast;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
class CVoodoo;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
class CSystemInfo; // NEO: SI - [SysInfo] <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
class CArgos;
#endif // ARGOS // NEO: NA END <-- Xanatos --
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
class CIP2Country;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --

struct SLogItem;

enum AppState{
	APP_STATE_RUNNING = 0,
   	APP_STATE_SHUTTINGDOWN,
	APP_STATE_DONE
};

class CemuleApp : public CWinApp
{
public:
	CemuleApp(LPCTSTR lpszAppName = NULL);

	// ZZ:UploadSpeedSense -->
    UploadBandwidthThrottler* uploadBandwidthThrottler;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	CBandwidthControl*	bandwidthControl;
#else
    LastCommonRouteFinder* lastCommonRouteFinder;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	// ZZ:UploadSpeedSense <--
#ifdef NEO_DBT // NEO: NDBT - [NeoDownloadBandwidthThrottler] -- Xanatos -->
	DownloadBandwidthThrottler* downloadBandwidthThrottler;
#endif // NEO_DBT // NEO: NDBT END <-- Xanatos --
	CemuleDlg*			emuledlg;
	CClientList*		clientlist;
	CKnownFileList*		knownfiles;
	CServerConnect*		serverconnect;
	CServerList*		serverlist;	
	CSharedFileList*	sharedfiles;
	CSearchList*		searchlist;
	CListenSocket*		listensocket;
	CUploadQueue*		uploadqueue;
	CDownloadQueue*		downloadqueue;
	CClientCreditsList*	clientcredits;
	CFriendList*		friendlist;
	CClientUDPSocket*	clientudp;
#ifdef NATTUNNELING // NEO: UTCP - [UserModeTCP] -- Xanatos -->
	CNatManager*		natmanager;
#endif //NATTUNNELING // NEO: UTCP END <-- Xanatos --
#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->
	CLanCast*			lancast;
#endif //LANCAST // NEO: NLC END <-- Xanatos --
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface] -- Xanatos -->
	CVoodoo*				voodoo;
#endif // VOODOO // NEO: VOODOO END <-- Xanatos --
	CIPFilter*			ipfilter;
#ifdef IP2COUNTRY // NEO: IP2C - [IPtoCountry] -- Xanatos -->
	CIP2Country*		ip2country;
#endif // IP2COUNTRY // NEO: IP2C END <-- Xanatos --
	CWebServer*			webserver;
	CScheduler*			scheduler;
	CMMServer*			mmserver;
	CPeerCacheFinder*	m_pPeerCache;
	CFirewallOpener*	m_pFirewallOpener;
	CUPnPImplWrapper*	m_pUPnPFinder;
	CSplashScreenEx*	m_pSplashWnd; // NEO: SS - [SplashScreen] <-- Xanatos --
#ifdef NEO_CD // NEO: NCD - [NeoClientDatabase] -- Xanatos -->
	CSourceList*		sourcelist;
#endif // NEO_CD // NEO: NCD END <-- Xanatos --
	CSystemInfo*		sysinfo; // NEO: SI - [SysInfo] <-- Xanatos --
#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->
	CArgos*				argos;
#endif // ARGOS // NEO: NA END <-- Xanatos --

	HANDLE				m_hMutexOneInstance;
	int					m_iDfltImageListColorFlags;
	CFont				m_fontHyperText;
	CFont				m_fontDefaultBold;
	CFont				m_fontSymbol;
	CFont				m_fontLog;
	CFont				m_fontChatEdit;
	CBrush				m_brushBackwardDiagonal;
	static const UINT	m_nVersionMjr;
	static const UINT	m_nVersionMin;
	static const UINT	m_nVersionUpd;
	static const UINT	m_nVersionBld;
	DWORD				m_dwProductVersionMS;
	DWORD				m_dwProductVersionLS;
	CString				m_strCurVersionLong;
	CString				m_strCurVersionLongDbg;
	CString				m_strNeoVersionLong; // NEO: NV - [NeoVersion] <-- Xanatos --
	UINT				m_uCurVersionShort;
	UINT				m_uCurVersionCheck;
	ULONGLONG			m_ullComCtrlVer;
	AppState			m_app_state; // defines application state for shutdown 
	CMutex				hashing_mut;
	CReadWriteLock		m_threadlock;	// SLUGFILLER: SafeHash - This will ensure eMule goes last // NEO: SSH - [SlugFillerSafeHash] <-- Xanatos --
	CString*			pstrPendingLink;
	COPYDATASTRUCT		sendstruct;
	CString				m_LocalBindAddress; // NEO: MOD - [BindToAdapter] <-- Xanatos --

// Implementierung
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL IsIdleMessage(MSG *pMsg);

	// ed2k link functions
	void		AddEd2kLinksToDownload(CString strLinks, int cat);
	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	void		PasteClipboard(int cat = 0);
	bool		IsEd2kFileLinkInClipboard();
	bool		IsEd2kServerLinkInClipboard();
	bool		IsEd2kLinkInClipboard(LPCSTR pszLinkType, int iLinkTypeLen);
	bool		IsEd2kFriendLinkInClipboard(); // NEO: TFL - [TetraFriendLinks] <-- Xanatos --
	LPCTSTR		GetProfileFile()		{ return m_pszProfileName; }

	CString		CreateED2kSourceLink(const CAbstractFile* f);
//	CString		CreateED2kHostnameSourceLink(const CAbstractFile* f);
	CString		CreateKadSourceLink(const CAbstractFile* f);

	// clipboard (text)
	bool		CopyTextToClipboard(CString strText);
	CString		CopyTextFromClipboard();

	void		OnlineSig();
	void		UpdateReceivedBytes(uint32 bytesToAdd);
#ifdef BW_MOD// NEO: BM - [BandwidthModeration] -- Xanatos -->
	void		UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend = false, bool sentToRelease = false);
#else
	void		UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend = false);
#endif // BW_MOD // NEO: BM END <-- Xanatos --
	int			GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength = -1, bool bNormalsSize = false);
	HIMAGELIST	GetSystemImageList() { return m_hSystemImageList; }
	HIMAGELIST	GetBigSystemImageList() { return m_hBigSystemImageList; }
	CSize		GetSmallSytemIconSize() { return m_sizSmallSystemIcon; }
	CSize		GetBigSytemIconSize() { return m_sizBigSystemIcon; }
	void		CreateBackwardDiagonalBrush();
	void		CreateAllFonts();
	const CString &GetDefaultFontFaceName();
	bool		IsPortchangeAllowed();
	bool		IsConnected();
	// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
	bool		GetConState(bool bInetOnly = false); 
	void		CountConnectionFailed() {m_uCountConnectionFailed ++;}
	void		SetConnectionEstablished() { m_uLastConnectionEstablished = ::GetTickCount(); m_uCountConnectionFailed = 0; }
	bool		IsConnectionFailed();
	// NEO: NCC END <-- Xanatos --
	bool		IsFirewalled();
	bool		CanDoCallback( CUpDownClient *client );
	void		SetHighTimer(bool bHigh); // NEO: MOD - [HighTimer] <-- Xanatos --
	uint32		GetID();
	uint32		GetPublicIP(bool bIgnoreKadIP = false) const;	// return current (valid) public IP or 0 if unknown
	void		SetPublicIP(const uint32 dwIP);
	void		ResetStandByIdleTimer();
#ifdef NATTUNNELING // NEO: RTP - [ReuseTCPPort] -- Xanatos -->
	uint16		GetPublicPort() { return m_uPublicPort; }
	void		SetPublicPort(const uint16 uPort) { m_uPublicPort = uPort;}
#endif //NATTUNNELING // NEO: RTP END <-- Xanatos --
	// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
	void		CheckIDChange(const uint32 dwIP, int iFrom = 0);
	uint32		GetLastIDChange() {return m_uLastChangeID;} 
	// NEO: RIC END <-- Xanatos --
	bool		IsPublicIP() {return (m_dwPublicIP != 0);} // NEO: MOD - [IsPublicIP] <-- Xanatos --
	// NEO: MOD - [BindToAdapter] -- Xanatos -->
	LPCTSTR		GetBindAddress() { if (m_LocalBindAddress.IsEmpty()) { return NULL; } else { return m_LocalBindAddress; } }
	void		BindToAddress(LPCTSTR LocalBindAddress = NULL);
	// NEO: MOD END <-- Xanatos --

	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	HICON		LoadIcon(LPCTSTR lpszResourceName, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR) const;
	HICON		LoadIcon(UINT nIDResource) const;
	HBITMAP		LoadImage(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	HBITMAP		LoadImage(UINT nIDResource, LPCTSTR pszResourceType) const;
	bool		LoadSkinColor(LPCTSTR pszKey, COLORREF& crColor) const;
	bool		LoadSkinColorAlt(LPCTSTR pszKey, LPCTSTR pszAlternateKey, COLORREF& crColor) const;
	CString		GetSkinFileItem(LPCTSTR lpszResourceName, LPCTSTR pszResourceType) const;
	void		ApplySkin(LPCTSTR pszSkinProfile);
	void		EnableRTLWindowsLayout();
	void		DisableRTLWindowsLayout();
	void		UpdateDesktopColorDepth();
	void		UpdateLargeIconSize();
	bool		IsVistaThemeActive() const;

	bool		GetLangHelpFilePath(CString& strResult);
	void		SetHelpFilePath(LPCTSTR pszHelpFilePath);
	void		ShowHelp(UINT uTopic, UINT uCmd = HELP_CONTEXT);
	bool		ShowWebHelp(UINT uTopic);

    // Elandal:ThreadSafeLogging -->
    // thread safe log calls
    void			QueueDebugLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueDebugLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleDebugLogQueue();
    void			ClearDebugLogQueue(bool bDebugPendingMsgs = false);

	void			QueueLogLine(bool bAddToStatusBar, LPCTSTR line,...);
    void			QueueLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleLogQueue();
    void			ClearLogQueue(bool bDebugPendingMsgs = false);
	// NEO: ML - [ModLog] -- Xanatos -->
    void			QueueModLogLine(bool addtostatusbar, LPCTSTR line,...);
	void			QueueModLogLineEx(UINT uFlags, LPCTSTR line,...);
    void			HandleModLogQueue();
    void			ClearModLogQueue(bool bDebugPendingMsgs = false);
	// NEO: ML END <-- Xanatos --
    // Elandal:ThreadSafeLogging <--

	bool			DidWeAutoStart() { return m_bAutoStart; }

	CString			GetAppTitle(bool bE = false); // NEO: NV - [NeoVersion] <-- Xanatos --

	// NEO: SS - [SplashScreen] -- Xanatos -->
	void			ShowSplash();
	void			UpdateSplash(LPCTSTR Text);
	void			HideSplash();
	void			AttachSplash();
	bool			IsSplash()		{ return (m_pSplashWnd != NULL); }
	// NEO SS END <-- Xanatos --

#ifdef WS2 // NEO: WS2 - [WINSOCK2] -- Xanatos -->
	bool			m_bWinSock2;
	WSADATA			m_wsaData;
#endif // WS2 // NEO: WS2 END <-- Xanatos --

protected:
	bool ProcessCommandline();
	void SetTimeOnTransfer();
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHelp();

	HIMAGELIST m_hSystemImageList;
	CMapStringToPtr m_aExtToSysImgIdx;
	CSize m_sizSmallSystemIcon;

	HIMAGELIST m_hBigSystemImageList;
	CMapStringToPtr m_aBigExtToSysImgIdx;
	CSize m_sizBigSystemIcon;

	CString		m_strDefaultFontFaceName;
	bool		m_bGuardClipboardPrompt;
	CString		m_strLastClipboardContents;

    // Elandal:ThreadSafeLogging -->
    // thread safe log calls
    CCriticalSection m_queueLock;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueDebugLog;
    CTypedPtrList<CPtrList, SLogItem*> m_QueueLog;
	CTypedPtrList<CPtrList, SLogItem*> m_QueueModLog; // NEO: ML - [ModLog] <-- Xanatos --
    // Elandal:ThreadSafeLogging <--

	uint32 m_dwPublicIP;
#ifdef NATTUNNELING // NEO: RTP - [ReuseTCPPort] -- Xanatos -->
	uint16 m_uPublicPort;
#endif //NATTUNNELING // NEO: RTP END <-- Xanatos --
	bool m_bAutoStart;

	// NEO: NCC - [NeoConnectionChecker] -- Xanatos -->
	uint16 m_uCountConnectionFailed;
	uint32 m_uLastConnectionEstablished;
	// NEO: NCC END <-- Xanatos --

private:
    UINT     m_wTimerRes;
	// NEO: RIC - [ReaskOnIDChange] -- Xanatos -->
	uint32	m_uLastIP;
	bool	m_bLastIDLow;
	uint32	m_uLastChangeID;
	// NEO: RIC END <-- Xanatos --
};

extern CemuleApp theApp;


//////////////////////////////////////////////////////////////////////////////
// CTempIconLoader

class CTempIconLoader
{
public:
	// because nearly all icons we are loading are 16x16, the default size is specified as 16 and not as 32 nor LR_DEFAULTSIZE
	CTempIconLoader(LPCTSTR pszResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	CTempIconLoader(UINT uResourceID, int cx = 16, int cy = 16, UINT uFlags = LR_DEFAULTCOLOR);
	~CTempIconLoader();

	operator HICON() const{
		return this == NULL ? NULL : m_hIcon;
	}

protected:
	HICON m_hIcon;
};
