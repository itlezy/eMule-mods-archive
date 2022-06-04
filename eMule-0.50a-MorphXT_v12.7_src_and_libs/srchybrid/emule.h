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
#include "ReadWriteLock.h"	// SLUGFILLER: SafeHash
#include "UPnP_IGDControlPoint.h" //MORPH - Added by SiRoB, [MoNKi: -UPnPNAT Support-]
#include "WapServer/WapServer.h" //MORPH START - Added by SiRoB / Commander, Wapserver [emulEspaña]

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
class CMMServer;
class CAbstractFile;
class CUpDownClient;
class CPeerCacheFinder;
class CFirewallOpener;
#ifdef USE_OFFICIAL_UPNP
class CUPnPImplWrapper;
#endif

struct SLogItem;
class CFakecheck; //MORPH - Added by milobac, FakeCheck, FakeReport, Auto-updating
class CIP2Country; //EastShare - added by AndCycle, IP to Country
class CPPgBackup; //EastShare - Added by Pretender, TBH-AutoBackup

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
	CMMServer*			mmserver;
	CPeerCacheFinder*	m_pPeerCache;
	CFirewallOpener*	m_pFirewallOpener;
#ifdef USE_OFFICIAL_UPNP
	CUPnPImplWrapper*	m_pUPnPFinder;
#endif
        //MORPH START - Added by schnulli900, dynamic IP-Filters [Xman]
	bool				m_bIsIPDlgOpen;
        //MORPH END   - Added by schnulli900, dynamic IP-Filters [Xman]

	CFakecheck*			FakeCheck; //MORPH - Added by milobac, FakeCheck, FakeReport, Auto-updating
	CPPgBackup*			ppgbackup; //EastShare - Added by Pretender, TBH-AutoBackup
	CIP2Country*		ip2country; //EastShare - added by AndCycle, IP to Country


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
	CReadWriteLock		m_threadlock;	// SLUGFILLER: SafeHash - This will ensure eMule goes last
	CString*			pstrPendingLink;
	COPYDATASTRUCT		sendstruct;

	//MORPH START - Added by SiRoB, [-modname-]
	static const UINT	m_nMVersionMjr;
	static const UINT	m_nMVersionMin;
	static const UINT	m_nMVersionBld;
	static const TCHAR	m_szMVersionLong[];
	static const TCHAR	m_szMVersion[];
	static const TCHAR	m_szMMVersion[];
	CString		m_strModVersion;
	CString		m_strModVersionPure;
	CString		m_strModLongVersion;
	uint8		m_uModLength;
	//MORPH END   - Added by SiRoB, [-modname-]
	
// Implementierung
	virtual BOOL InitInstance();
	virtual int ExitInstance();
	virtual BOOL IsIdleMessage(MSG *pMsg);
	virtual BOOL OnIdle(LONG lCount); // MORPH

	// ed2k link functions
	void		AddEd2kLinksToDownload(CString strLinks, int cat, bool fromclipboard=false);
	void		SearchClipboard();
	void		IgnoreClipboardLinks(CString strLinks) {m_strLastClipboardContents = strLinks;}
	//MORPH START - Changed by SiRoB, Selection category support khaos::categorymod+
	/*
	void		PasteClipboard(int cat = 0);
	*/
	void		PasteClipboard(int Cat = -1);
	//MORPH END   - Changed by SiRoB, Selection category support khaos::categorymod+
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
	WSADATA				m_wsaData; //MORPH - Added by SiRoB, eWombat [WINSOCK2]

private:
    UINT     m_wTimerRes;

public:
	// Commander - Added: Custom incoming folder icon [emulEspaña] - Start
	void	AddIncomingFolderIcon();
	void	RemoveIncomingFolderIcon();
	void	AddTempFolderIcon();
	void	RemoveTempFolderIcon();
	BOOL	IsCustomIncomingFolderIcon();
	// Commander - Added: Custom incoming folder icon [emulEspaña] - End

	//MORPH START - Added by SiRoB, [MoNKi: -UPnPNAT Support-]
public:
	CUPnP_IGDControlPoint *m_UPnP_IGDControlPoint;
	//MORPH END   - Added by SiRoB, [MoNKi: -UPnPNAT Support-]

//MORPH START - Added by SiRoB / Commander, Wapserver [emulEspaña]
public:
	CWapServer*		wapserver;
//MORPH END - Added by SiRoB / Commander, Wapserver [emulEspaña]
// Morph start lh: lh require obfuscated server connection 
   // When you want to connect to a server obfuscated you need
   // to be able to cryptping, else no server will have a valid Key
	bool IsWaitingForCryptPingConnect();  
// Morph end lh: lh require obfuscated server connection 
// MORPH START - Added by Commander, Friendlinks [emulEspaña]
public:
	bool	IsEd2kFriendLinkInClipboard();
// MORPH END - Added by Commander, Friendlinks [emulEspaña]
#define  SVC_NO_OPT 0
#define	 SVC_BASIC_OPT 2
#define  SVC_LIST_OPT 6
#define  SVC_FULL_OPT 10
	bool	IsRunningAsService(int OptimizeLevel = SVC_BASIC_OPT );// MORPH leuk_he:run as ntservice v1..

	void RebindUPnP(); //emulEspaa: Added by MoNKi [MoNKi: -UPnPNAT Support-]
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
