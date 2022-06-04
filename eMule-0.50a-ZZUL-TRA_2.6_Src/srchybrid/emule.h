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
class LastCommonRouteFinder;
class CemuleDlg;
class CClientList;
class CKnownFileList;
class CServerConnect;
class CServerList;
class CSharedFileList;
class CClientCreditsList;
class CFriendList;
class CClientUDPSocket;
class CIPFilter;
class CWebServer;
class CAbstractFile;
class CUpDownClient;
//class CPeerCacheFinder; // X: [RPC] - [Remove PeerCache]
class CFirewallOpener;
class CUPnPImplWrapper;
class CIP2Country; // ZZUL-TRA :: IP2Country
class CSpeedGraphWnd; // X: [SGW] - [SpeedGraphWnd]
#ifdef CLIENTANALYZER
class CAntiLeechDataList; //>>> WiZaRd::ClientAnalyzer
#endif
class CSysInfo; // ZZUL-TRA :: SysInfo

struct SLogItem;
class CSplashScreenEx; //ZZUL-TRA :: NewSplashscreen
class CDLP; // ZZUL-TRA :: DLP_OwnNickCheck

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
    LastCommonRouteFinder* lastCommonRouteFinder;
	// ZZ:UploadSpeedSense <--
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
	CIPFilter*			ipfilter;
	CWebServer*			webserver;
	CScheduler*			scheduler;
//	CPeerCacheFinder*	m_pPeerCache; // X: [RPC] - [Remove PeerCache]
	CFirewallOpener*	m_pFirewallOpener;
	CUPnPImplWrapper*	m_pUPnPFinder;
	CIP2Country*		ip2country; // ZZUL-TRA :: IP2Country
	CSpeedGraphWnd*		m_pSpeedGraphWnd; // X: [SGW] - [SpeedGraphWnd]
	CSplashScreenEx*	m_pSplashWnd; //ZZUL-TRA :: NewSplashscreen
#ifdef CLIENTANALYZER
	CAntiLeechDataList* antileechlist; //>>> WiZaRd::ClientAnalyzer
#endif
	CSysInfo*			pSysInfo; // ZZUL-TRA :: SysInfo
	CDLP*				dlp; // ZZUL-TRA :: DLP_OwnNickCheck

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
	UINT				m_uCurVersionShort;
	UINT				m_uCurVersionCheck;
	ULONGLONG			m_ullComCtrlVer;
	AppState			m_app_state; // defines application state for shutdown 
	CMutex				hashing_mut;
	CString*			pstrPendingLink;
	COPYDATASTRUCT		sendstruct;
	bool				m_bRestartApp; // ZZUL-TRA :: AutoRestartIfNecessary

// Implementierung
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL IsIdleMessage(MSG *pMsg);

//ZZUL-TRA :: NewSplashscreen :: Start
	void			ShowSplash(bool start=false);
	void			UpdateSplash(LPCTSTR Text);
	void			UpdateSplash2(LPCTSTR Text);
	void			DestroySplash();
	bool			IsSplash()			{ return (m_pSplashWnd != NULL); }
	void			SplashHide(int hide = SW_HIDE);
	bool			splashscreenfinished;
	uint32			m_dwSplashTime;
//ZZUL-TRA :: NewSplashscreen :: End

	// ed2k link functions
	//Xman [MoNKi: -Check already downloaded files-]
	/*
	void		AddEd2kLinksToDownload(CString strLinks, int cat);
	*/
	void		AddEd2kLinksToDownload(CString strLinks, int cat, bool askIfAlreadyDownloaded = false);
	//Xman end

	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	void		PasteClipboard(int cat = 0);
	bool		IsEd2kFileLinkInClipboard();
	bool		IsEd2kServerLinkInClipboard();
	// MORPH START - Added by Commander, Friendlinks [emulEspaa] - added by zz_fly
	bool		IsEd2kFriendLinkInClipboard();
	// MORPH END - Added by Commander, Friendlinks [emulEspaa]
	bool		IsEd2kLinkInClipboard(LPCSTR pszLinkType, int iLinkTypeLen);
	LPCTSTR		GetProfileFile()		{ return m_pszProfileName; }

	CString		CreateKadSourceLink(const CAbstractFile* f);

	// clipboard (text)
	bool		CopyTextToClipboard(CString strText);
	CString		CopyTextFromClipboard();

	void		OnlineSig();
	void		UpdateReceivedBytes(uint32 bytesToAdd);
	void		UpdateSentBytes(uint32 bytesToAdd, bool sentToFriend = false);
	int			GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength = -1, bool bNormalsSize = false);
	HIMAGELIST	GetSystemImageList() { return m_hSystemImageList; }
	HIMAGELIST	GetBigSystemImageList() { return m_hBigSystemImageList; }
	CSize		GetSmallSytemIconSize() { return m_sizSmallSystemIcon; }
	CSize		GetBigSytemIconSize() { return m_sizBigSystemIcon; }
	void		CreateBackwardDiagonalBrush();
	void		CreateAllFonts();
	const CString &GetDefaultFontFaceName();
	bool		IsPortchangeAllowed();
	bool		IsConnected(bool bIgnoreEd2k = false, bool bIgnoreKad = false);
	bool		IsFirewalled();
	bool		CanDoCallback( CUpDownClient *client );
	uint32		GetID();
	uint32		GetPublicIP(bool bIgnoreKadIP = false) const;	// return current (valid) public IP or 0 if unknown
	void		SetPublicIP(const uint32 dwIP);
	void		ResetStandByIdleTimer();

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
	bool		IsXPThemeActive() const;
	bool		IsVistaThemeActive() const;

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
    // Elandal:ThreadSafeLogging <--

	bool			DidWeAutoStart() { return m_bAutoStart; }

protected:
	bool ProcessCommandline();
	void SetTimeOnTransfer();
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

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
    // Elandal:ThreadSafeLogging <--

	uint32 m_dwPublicIP;
	bool m_bAutoStart;

private:
    UINT     m_wTimerRes;

// ZZUL-TRA :: ReAskSrcAfterIPChange :: Start
public:
	void			CheckIdChange();
private:
	uint32			m_uLastValidID[3];
	DWORD			m_dwLastIpCheckDetected;
// ZZUL-TRA :: ReAskSrcAfterIPChange :: End

// ZZUL-TRA :: AutoDropImmunity :: Start
public:
	DWORD	GetReAskTick()	{return m_dwReAskTick;}
	void	SetReAskTick(DWORD in) {m_dwReAskTick = in;}
private:
	DWORD	m_dwReAskTick;
// ZZUL-TRA :: AutoDropImmunity :: End

// ZZUL-TRA :: Winsock2 :: Start
protected:
	bool	m_bWinSock2;
public:
	bool WinSock2() {return m_bWinSock2;}
	WSADATA				m_wsaData;
// ZZUL-TRA :: Winsock2 :: End

// ZZUL-TRA :: AutoSharedFilesUpdater :: Start
private:
	static CEvent*				m_directoryWatcherCloseEvent;
	static CEvent*				m_directoryWatcherReloadEvent;
	static CCriticalSection		m_directoryWatcherCS;
	static UINT					CheckDirectoryForChangesThread(LPVOID pParam);
public:
	void ResetDirectoryWatcher();
	void EndDirectoryWatcher();
	void DirectoryWatcherExternalReload();
// ZZUL-TRA :: AutoSharedFilesUpdater :: End
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