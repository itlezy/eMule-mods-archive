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
//Xman
#include "ReadWriteLock.h"	// SLUGFILLER: SafeHash
#include "Version.h"		// netfinity: Mod version

// ==> UPnP support [MoNKi] - leuk_he
/*
#ifdef DUAL_UPNP //zz_fly :: dual upnp
#include "UPnP_acat.h" //ACAT UPnP
#endif //zz_fly :: dual upnp
*/
#include "UPnP_IGDControlPoint.h" //[MoNKi: -UPnPNAT Support-]
// <== UPnP support [MoNKi] - leuk_he


#include ".\MiniMule\SystemInfo.h" // CPU/MEM usage [$ick$/Stulle] - Max 
#include ".\MiniMule\TBHMM.h" // TBH: minimule - Max

// ==> Mephisto mod [Stulle] - Mephisto
/*
#define	DEFAULT_NICK		_T("ScarAngel @ http://scarangel.sourceforge.net")
*/
#define	DEFAULT_NICK		_T("Mephisto @ http://mephisto.emule-web.de")
// <== Mephisto mod [Stulle] - Mephisto
#define	DEFAULT_TCP_PORT_OLD	4662
#define	DEFAULT_UDP_PORT_OLD	(DEFAULT_TCP_PORT_OLD+10)

#define PORTTESTURL			_T("http://porttest.emule-project.net/connectiontest.php?tcpport=%i&udpport=%i&lang=%i")

class CSearchList;
class CUploadQueue;
class CListenSocket;
class CDownloadQueue;
class CScheduler;
class UploadBandwidthThrottler;
//Xman
/*
class LastCommonRouteFinder;
*/
//Xman end
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
class CMMServer;
class CAbstractFile;
class CUpDownClient;
class CPeerCacheFinder;
class CFirewallOpener;
// ==> UPnP support [MoNKi] - leuk_he
/*
class CUPnPImplWrapper;
*/
// <== UPnP support [MoNKi] - leuk_he

//Xman
class CBandWidthControl; // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
struct SLogItem;

class CIP2Country; //EastShare - added by AndCycle, IP to Country
class CDLP;	//Xman DLP
class CSplashScreenEx; //Xman new slpash-screen arrangement

class CSystemInfo;  // CPU/MEM usage [$ick$/Stulle] - Max 

enum AppState{
	APP_STATE_RUNNING = 0,
   	APP_STATE_SHUTTINGDOWN,
	APP_STATE_DONE
};

class CemuleApp : public CWinApp
{

	friend class CTBHMM; // TBH: minimule - Max

public:
	CemuleApp(LPCTSTR lpszAppName = NULL);

	// ZZ:UploadSpeedSense -->
    UploadBandwidthThrottler* uploadBandwidthThrottler;
	//Xman
	/*
    LastCommonRouteFinder* lastCommonRouteFinder;
	*/
	//Xman end
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
	CMMServer*			mmserver;
	CPeerCacheFinder*	m_pPeerCache;
	CFirewallOpener*	m_pFirewallOpener;
	// ==> UPnP support [MoNKi] - leuk_he
	/*
	CUPnPImplWrapper*	m_pUPnPFinder;
	*/
	// <== UPnP support [MoNKi] - leuk_he

	//Xman
	// - Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
	CBandWidthControl*	pBandWidthControl;
	// Maella end

	CSplashScreenEx*	m_pSplashWnd; //Xman new slpash-screen arrangement

	//Xman dynamic IP-Filters
	bool				ipdlgisopen;

	CIP2Country*		ip2country; //EastShare - added by AndCycle, IP to Country

	CDLP*				dlp;
	//Xman end

	// ==> TBH: minimule - Max/ leuk_he
	CTBHMM*				minimule;
	// <== TBH: minimule - Max/ leuk_he

	CSystemInfo*		sysinfo; // CPU/MEM usage [$ick$/Stulle] - Max 

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
	//MORPH
	/*
	AppState			m_app_state; // defines application state for shutdown 
	*/
	volatile AppState		m_app_state; // defines application state for shutdown 
	//MORPH END
	CMutex				hashing_mut;
	//Xman
	CReadWriteLock		m_threadlock;	// SLUGFILLER: SafeHash - This will ensure eMule goes last
	CString*			pstrPendingLink;
	COPYDATASTRUCT		sendstruct;

// Implementierung
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL IsIdleMessage(MSG *pMsg);

	// ed2k link functions
	//Xman [MoNKi: -Check already downloaded files-]
	/*
	void		AddEd2kLinksToDownload(CString strLinks, int cat);
	*/
	void		AddEd2kLinksToDownload(CString strLinks, int cat, bool askIfAlreadyDownloaded = false);
	//Xman end

	//Xman new slpash-screen arrangement
	void			ShowSplash(bool start=false);
	void			UpdateSplash(LPCTSTR Text);
	void			DestroySplash();
	bool			IsSplash()			{ return (m_pSplashWnd != NULL); }
	bool			spashscreenfinished;
	uint32			m_dwSplashTime;
	//Xman end

	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	// ==> Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	/*
	void		PasteClipboard(int cat = 0);
	*/
	void		PasteClipboard(int cat = -1);
	// <== Smart Category Control (SCC) [khaos/SiRoB/Stulle] - Stulle
	bool		IsEd2kFileLinkInClipboard();
	bool		IsEd2kServerLinkInClipboard();
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
    // Elandal:ThreadSafeLogging <--

	bool			DidWeAutoStart() { return m_bAutoStart; }

	WSADATA				m_wsaData; //eWombat [WINSOCK2]

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
    // Elandal:ThreadSafeLogging <--

	uint32 m_dwPublicIP;
	bool m_bAutoStart;

private:
    UINT     m_wTimerRes;
//Xman -Reask sources after IP change- v4 
public:
	bool m_bneedpublicIP; 
	uint32 last_ip_change;
	uint32 last_valid_serverid;
	uint32 last_valid_ip;
	uint32 recheck_ip;
	uint32	last_traffic_reception;
	uint8	internetmaybedown;
//Xman end

	// ==> UPnP support [MoNKi] - leuk_he
	/*
#ifdef DUAL_UPNP //zz_fly :: dual upnp
//ACAT UPnP
public:
	MyUPnP* m_pUPnPNat;
	BOOL  AddUPnPNatPort(MyUPnP::UPNPNAT_MAPPING *mapping, bool tryRandom = false);
	BOOL  RemoveUPnPNatPort(MyUPnP::UPNPNAT_MAPPING *mapping);
#endif //zz_fly :: dual upnp
	*/
	CUPnP_IGDControlPoint *m_UPnP_IGDControlPoint;
	// <== UPnP support [MoNKi] - leuk_he

	//Xman queued disc-access for read/flushing-threads
	/* zz_fly :: drop, use Morph's synchronization method instead.
	note: this feature can reduce the diskio. but it is hard to synchronize the threads.
		  when synchronization failed, emule will crash. 
		  i can not let this feature work properly in .50 codebase.
		  so, my only choice is drop this feature.
	void AddNewDiscAccessThread(CWinThread* threadtoadd);
	void ResumeNextDiscAccessThread();
	void ForeAllDiscAccessThreadsToFinish();
private:
	CTypedPtrList<CPtrList, CWinThread*> threadqueue;
	CCriticalSection					 threadqueuelock;
	volatile uint16						 m_uRunningNonBlockedDiscAccessThreads;
	*/
	//Xman end

// MORPH START - Added by Commander, Friendlinks [emulEspaa] - added by zz_fly
public:
	bool	IsEd2kFriendLinkInClipboard();
// MORPH END - Added by Commander, Friendlinks [emulEspaa]

	// ==> ModID [itsonlyme/SiRoB] - Stulle
public:
	static const UINT	m_nMVersionMjr;
	static const UINT	m_nMVersionMin;
	static const UINT	m_nMVersionBld;
	static const TCHAR	m_szMVersionLong[];
	static const TCHAR	m_szMVersion[];
	CString		m_strModVersion;
	CString		m_strModLongVersion;
	CString		m_strModVersionPure;
	uint8		m_uModLength;
	// <== ModID [itsonlyme/SiRoB] - Stulle

	// ==> Design Settings [eWombat/Stulle] - Stulle
	void	CreateExtraFonts(CFont *font);
	void	DestroyExtraFonts();
	CFont *GetFontByStyle(DWORD nStyle,bool bNarrow);
protected:
	CFont			m_ExtraFonts[15];
	// <== Design Settings [eWombat/Stulle] - Stulle

	// ==> Automatic shared files updater [MoNKi] - Stulle
private:
	static CEvent*				m_directoryWatcherCloseEvent;
	static CEvent*				m_directoryWatcherReloadEvent;
	static CCriticalSection		m_directoryWatcherCS;
	static UINT					CheckDirectoryForChangesThread(LPVOID pParam);
public:
	void ResetDirectoryWatcher();
	void EndDirectoryWatcher();
	void DirectoryWatcherExternalReload();
	// <== Automatic shared files updater [MoNKi] - Stulle

	void RebindUPnP(); // UPnP support [MoNKi] - leuk_he

	// ==> Run eMule as NT Service [leuk_he/Stulle] - Stulle
#define  SVC_NO_OPT 0
#define	 SVC_LIST_OPT 4
#define  SVC_SVR_OPT 6
#define SVC_FULL_OPT 10
	bool	IsRunningAsService(int OptimizeLevel = SVC_NO_OPT );// MORPH leuk_he:run as ntservice v1..
	// <== Run eMule as NT Service [leuk_he/Stulle] - Stulle
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
