// PPgTweaks2.h

// NEO: MOD - [MissingPrefs] -- Xanatos -->
#pragma once
#include "Neo\GUI\CP\TreeOptionsCtrl.h" 

class CPPgTweaks2 : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgTweaks2)

public:
	CPPgTweaks2();
	virtual ~CPPgTweaks2();

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

	void Localize();

protected:
	bool m_bHighresTimer;
	UINT m_iFileBufferSize;
	UINT m_iFileBufferTime;
	bool m_ICH;
	bool m_dontcompressblocks;
	bool m_dontcompressavi;
	bool m_bRemove2bin;
	bool m_bShowCopyEd2kLinkCmd;
	int m_nServerUDPPort;
	bool m_bRestoreLastMainWndDlg;
	bool m_bRestoreLastLogPane;
	bool m_bIconflashOnNewMessage;
	bool m_bMiniMuleAutoClose;
	int m_iMiniMuleTransparency;
	int m_iPreviewSmallBlocks;
	bool m_bPreviewCopiedArchives;
	bool m_bPreviewOnIconDblClk;
	bool m_bShowActiveDownloadsBold;
	bool m_bRTLWindowsLayout;
	bool m_bReBarToolbar;
	int m_iStraightWindowStyles;
	int m_iMaxChatHistory;
	int m_maxmsgsessions;
	bool m_bPreferRestrictedOverUser;
	bool m_bTrustEveryHash;
	bool m_bShowVerticalHourMarkers;
	int m_iInspectAllFileTypes;
	int m_iWebFileUploadSizeLimitMB;
	CString m_sAllowedRemoteAccessIPs;
	int m_uMaxLogFileSize;
	int m_iMaxLogBuff;
	UINT m_iLogFileFormat;
	CString m_strTxtEditor;
	bool m_bPeerCacheShow;
	bool m_bUseUserSortedServerList;
	int m_iDebugSearchResultDetailLevel;
	int m_iCryptTCPPaddingLength;
	bool m_bAdjustNTFSDaylightFileTime;
	CString m_strDateTimeFormat;
	CString m_strDateTimeFormat4Log;
	COLORREF m_crLogError;
	COLORREF m_crLogWarning;
	COLORREF m_crLogSuccess;

#ifdef WS2 // NEO: WS2 - [WINSOCK2]
	bool m_bWinSock2;
#endif // WS2 // NEO: WS2 END
	int m_iInternetSecurityZone;
	bool m_bCreateCrashDump;

	CString m_strEncryptCertName;
	bool m_bCheckComctl32;
	bool m_bCheckShell32;
	bool m_bIgnoreInstances;
	CString m_MediaInfo_MediaInfoDllPath;
	bool m_bMediaInfo_RIFF;
	bool m_bMediaInfo_RM;
	bool m_bMediaInfo_ID3LIB;


	CTreeOptionsCtrl m_ctrlTreeOptions;
	bool m_bInitializedTreeOpts;

	HTREEITEM m_htiHighresTimer;
	HTREEITEM m_htiFileBufferSize;
	HTREEITEM m_htiFileBufferTime;
	HTREEITEM m_htiICH;
	HTREEITEM m_htidontcompressblocks;
	HTREEITEM m_htidontcompressavi;
	HTREEITEM m_htiRemove2bin;
	HTREEITEM m_htiShowCopyEd2kLinkCmd;
	HTREEITEM m_htiServerUDPPort;
	HTREEITEM m_htiRestoreLastMainWndDlg;
	HTREEITEM m_htiRestoreLastLogPane;
	HTREEITEM m_htiIconflashOnNewMessage;
	HTREEITEM m_htiPreviewSmallBlocks;
	HTREEITEM m_htiPreviewCopiedArchives;
	HTREEITEM m_htiPreviewOnIconDblClk;
	HTREEITEM m_htiShowActiveDownloadsBold;
	HTREEITEM m_htiRTLWindowsLayout;
	HTREEITEM m_htiReBarToolbar;
	HTREEITEM m_htiStraightWindowStyles;
	HTREEITEM m_htiMaxChatHistory;
	HTREEITEM m_htimaxmsgsessions;
	HTREEITEM m_htiPreferRestrictedOverUser;
	HTREEITEM m_htiTrustEveryHash;
	HTREEITEM m_htiShowVerticalHourMarkers;
	HTREEITEM m_htiInspectAllFileTypes;
	HTREEITEM m_htiWebFileUploadSizeLimitMB;
	HTREEITEM m_htiAllowedRemoteAccessIPs;
	HTREEITEM m_htiPeerCacheShow;
	HTREEITEM m_htiUseUserSortedServerList;
	HTREEITEM m_htiDebugSearchResultDetailLevel;
	HTREEITEM m_htiCryptTCPPaddingLength;
	HTREEITEM m_htidatetimeformat;
	HTREEITEM m_htidatetimeformat4log;
	HTREEITEM m_htiLogError;
	HTREEITEM m_htiLogWarning;
	HTREEITEM m_htiLogSuccess;

#ifdef WS2 // NEO: WS2 - [WINSOCK2]
	HTREEITEM m_htiWinSock2;
#endif // WS2 // NEO: WS2 END
	HTREEITEM m_htiInternetSecurityZone;
	HTREEITEM m_htiCreateCrashDump;
	HTREEITEM m_htiMiniMuleAutoClose;
	HTREEITEM m_htiMiniMuleTransparency;
	HTREEITEM m_htiAdjustNTFSDaylightFileTime;
	HTREEITEM m_htiMaxLogFileSize;
	HTREEITEM m_htiMaxLogBuff;
	HTREEITEM m_htiLogFileFormat;
	HTREEITEM m_htiTxtEditor;

	HTREEITEM m_htiEncryptCertName;
	HTREEITEM m_htiCheckComctl32;
	HTREEITEM m_htiCheckShell32;
	HTREEITEM m_htiIgnoreInstances;
	HTREEITEM m_htiMediaInfo_MediaInfoDllPath;
	HTREEITEM m_htiMediaInfo_RIFF;
	HTREEITEM m_htiMediaInfo_RM;
	HTREEITEM m_htiMediaInfo_ID3LIB;

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	void TreeItemsToNull();
};
// NEO: MOD END <-- Xanatos --
