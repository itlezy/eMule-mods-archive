// PPgTweaks2.cpp

//this file is part of eMule
//Copyright (C)2002 Merkur ( devs@emule-project.net / http://www.emule-project.net )
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
#include "stdafx.h"
#include "emule.h"
#include "PPgTweaks2.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "HelpIDs.h"
#include "Log.h"
#include "UserMsgs.h"
#include "neo/functions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// NEO: MOD - [MissingPrefs] -- Xanatos -->
static const LPCTSTR _asNetSecurityZones[] = {
	_T("Untrusted"),
	_T("Internet"),
	_T("Intranet"),
	_T("Trusted"),
	_T("LocalMachine")
};


///////////////////////////////////////////////////////////////////////////////
// CPPgTweaks2 dialog

IMPLEMENT_DYNAMIC(CPPgTweaks2, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgTweaks2, CPropertyPage)
	ON_WM_DESTROY()
	ON_MESSAGE(WM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify) 
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgTweaks2::CPPgTweaks2()
	: CPropertyPage(CPPgTweaks2::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
	TreeItemsToNull();
}

CPPgTweaks2::~CPPgTweaks2()
{
}

void CPPgTweaks2::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MOD_OPTS, m_ctrlTreeOptions);
	if (!m_bInitializedTreeOpts)
	{
		GetDlgItem(IDC_MOD_OPTS_INFO)->SetWindowText(GetResString(IDS_X_TWEAKS2_WARNING));
		m_htiHighresTimer = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_HIGHRES_TIMER), TVI_ROOT, m_bHighresTimer);
		m_htiFileBufferSize = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_FILE_BUFFER_SIZE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiFileBufferSize, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiFileBufferTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_FILE_BUFFER_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiFileBufferTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiICH = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_ICH), TVI_ROOT, m_ICH);
		m_htidontcompressblocks = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_DONTCOMPRESSBLOCKS), TVI_ROOT, m_dontcompressblocks);
		m_htidontcompressavi = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_DONTCOMPRESSAVI), TVI_ROOT, m_dontcompressavi);
		m_htiRemove2bin = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_REMOVE2BIN), TVI_ROOT, m_bRemove2bin);
		m_htiShowCopyEd2kLinkCmd = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_SHOWCOPYED2KLINK), TVI_ROOT, m_bShowCopyEd2kLinkCmd);
		m_htiServerUDPPort = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_SERVERUDPPORT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiServerUDPPort, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiRestoreLastMainWndDlg = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_RESTORELASTMAINWNDDLG), TVI_ROOT, m_bRestoreLastMainWndDlg);
		m_htiRestoreLastLogPane = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_RESTORELASTLOGPANE), TVI_ROOT, m_bRestoreLastLogPane);
		m_htiIconflashOnNewMessage = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_ICON_FLASH_ON_NEW_MESSAGE), TVI_ROOT, m_bIconflashOnNewMessage);
		m_htiMiniMuleAutoClose = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_MINIMULEAUTOCLOSE), TVI_ROOT, m_bMiniMuleAutoClose);
		m_htiMiniMuleTransparency = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_MINIMULETRANSPARENCY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiMiniMuleTransparency, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiPreviewSmallBlocks = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_PREVIEWSMALLBLOCKS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiPreviewSmallBlocks, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiPreviewCopiedArchives = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_PREVIEWCOPIEDARCHIVES), TVI_ROOT, m_bPreviewCopiedArchives);
		m_htiPreviewOnIconDblClk = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_PREVIEWONICONDBLCLK), TVI_ROOT, m_bPreviewOnIconDblClk);
		m_htiShowActiveDownloadsBold = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_SHOWACTIVEDOWNLOADSBOLD), TVI_ROOT, m_bShowActiveDownloadsBold);
		m_htiRTLWindowsLayout = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_RTLWINDOWSLAYOUT), TVI_ROOT, m_bRTLWindowsLayout);
		m_htiReBarToolbar = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_REBARTOOLBAR), TVI_ROOT, m_bReBarToolbar);		
		m_htiStraightWindowStyles = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_STRAIGHTWINDOWSTYLES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiStraightWindowStyles, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxChatHistory = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_MAXCHATHISTORY), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxChatHistory, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htimaxmsgsessions = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_MAXMSGSESSIONS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htimaxmsgsessions, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiPreferRestrictedOverUser = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_PREFERRESTRICTEDOVERUSER), TVI_ROOT, m_bPreferRestrictedOverUser);
		m_htiTrustEveryHash = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_TRUSTEVERYHASH), TVI_ROOT, m_bTrustEveryHash);
		m_htiShowVerticalHourMarkers = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_SHOWVERTICALHOURMARKERS), TVI_ROOT, m_bShowVerticalHourMarkers);
		m_htiInspectAllFileTypes = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_INSPECTALLFILETYPES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiInspectAllFileTypes, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiWebFileUploadSizeLimitMB = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_WEBFILEUPLOADSIZELIMITMB), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiWebFileUploadSizeLimitMB, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAllowedRemoteAccessIPs = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_ALLOWEDREMOTEACCESSIPS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiAllowedRemoteAccessIPs, RUNTIME_CLASS(CTreeOptionsEdit));
		m_htiMaxLogFileSize = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_MAXLOGFILESIZE), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxLogFileSize, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxLogBuff = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_MAXLOGBUFFER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxLogBuff, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiLogFileFormat = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_LOGFILEFORMAT), TVI_ROOT, m_iLogFileFormat);
		m_htiTxtEditor = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_TXTEDITOR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiTxtEditor, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_htiUseUserSortedServerList = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_USEUSERSORTEDSERVERLIST),TVI_ROOT,m_bUseUserSortedServerList);

		m_htiCryptTCPPaddingLength = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_CRYPTTCPPADDINGLENGTH),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiCryptTCPPaddingLength, RUNTIME_CLASS(CNumTreeOptionsEdit));													   

		m_htiDebugSearchResultDetailLevel = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_DEBUGSEARCHDETAILLEVEL),TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT,TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiDebugSearchResultDetailLevel, RUNTIME_CLASS(CTreeOptionsEdit));

		m_htiAdjustNTFSDaylightFileTime = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_ADJUSTNTFSDAYLIGHTFILETIME), TVI_ROOT, m_bAdjustNTFSDaylightFileTime);

		m_htiPeerCacheShow = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_PEERCACHESHOW), TVI_ROOT, m_bPeerCacheShow);
		m_htidatetimeformat = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htidatetimeformat, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htidatetimeformat4log = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_DATETIMEFORMAT4LOG), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htidatetimeformat4log, RUNTIME_CLASS(CNumTreeOptionsEdit));


		m_htiLogError = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_LOGERROR), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddColorSelector(m_htiLogError, RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiLogWarning = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_LOGWARNING), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddColorSelector(m_htiLogWarning, RUNTIME_CLASS(CTreeOptionsBrowseButton));
		m_htiLogSuccess = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_LOGSUCCESS), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddColorSelector(m_htiLogSuccess, RUNTIME_CLASS(CTreeOptionsBrowseButton));

#ifdef WS2 // NEO: WS2 - [WINSOCK2]
		m_htiWinSock2 = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_WINSOCK2), TVI_ROOT, m_bWinSock2);
#endif // WS2 // NEO: WS2 END
		m_htiCreateCrashDump = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_CREATECRASHDUMP), TVI_ROOT, m_bCreateCrashDump);
		m_htiInternetSecurityZone = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_X_INTERNETSECURITYZONE), TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		for (int i = 0; i < ARRSIZE(_asNetSecurityZones); i++)
			m_ctrlTreeOptions.InsertRadioButton(_asNetSecurityZones[i], m_htiInternetSecurityZone, m_iInternetSecurityZone == i);

		m_htiEncryptCertName = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_ENCRYPTCERTNAME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiEncryptCertName, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_htiCheckComctl32 = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_CHECKCOMCTL32), TVI_ROOT, m_bCheckComctl32);
		m_htiCheckShell32 = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_CHECKSHELL32), TVI_ROOT, m_bCheckShell32);
		m_htiIgnoreInstances = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_IGNOREINSTANCES), TVI_ROOT, m_bIgnoreInstances);

		m_htiMediaInfo_MediaInfoDllPath = m_ctrlTreeOptions.InsertItem(GetResString(IDS_X_MEDIAINFODLLPATH), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, TVI_ROOT);
		m_ctrlTreeOptions.AddEditBox(m_htiMediaInfo_MediaInfoDllPath, RUNTIME_CLASS(CNumTreeOptionsEdit));

		m_htiMediaInfo_RIFF = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_MEDIAINFO_RIFF), TVI_ROOT, m_bMediaInfo_RIFF);
		m_htiMediaInfo_RM = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_MEDIAINFO_RM), TVI_ROOT, m_bMediaInfo_RM);
		m_htiMediaInfo_ID3LIB = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_X_MEDIAINFO_ID3LIB), TVI_ROOT, m_bMediaInfo_ID3LIB);

		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;
	}

	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiHighresTimer, m_bHighresTimer);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiFileBufferSize, m_iFileBufferSize);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiFileBufferTime, m_iFileBufferTime);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiICH, m_ICH);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htidontcompressblocks, m_dontcompressblocks);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htidontcompressavi, m_dontcompressavi);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRemove2bin, m_bRemove2bin);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowCopyEd2kLinkCmd, m_bShowCopyEd2kLinkCmd);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRestoreLastMainWndDlg, m_bRestoreLastMainWndDlg);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRestoreLastLogPane, m_bRestoreLastLogPane);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiIconflashOnNewMessage, m_bIconflashOnNewMessage);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiMiniMuleAutoClose, m_bMiniMuleAutoClose);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiMiniMuleTransparency, m_iMiniMuleTransparency);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPreviewCopiedArchives, m_bPreviewCopiedArchives);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPreviewOnIconDblClk, m_bPreviewOnIconDblClk);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowActiveDownloadsBold, m_bShowActiveDownloadsBold);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiRTLWindowsLayout, m_bRTLWindowsLayout);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiReBarToolbar, m_bReBarToolbar);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPreferRestrictedOverUser, m_bPreferRestrictedOverUser);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiTrustEveryHash, m_bTrustEveryHash);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiShowVerticalHourMarkers, m_bShowVerticalHourMarkers);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiServerUDPPort, m_nServerUDPPort);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiPreviewSmallBlocks, m_iPreviewSmallBlocks);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiStraightWindowStyles, m_iStraightWindowStyles);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiMaxChatHistory, m_iMaxChatHistory);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htimaxmsgsessions, m_maxmsgsessions);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiInspectAllFileTypes, m_iInspectAllFileTypes);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiWebFileUploadSizeLimitMB, m_iWebFileUploadSizeLimitMB);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiAllowedRemoteAccessIPs, m_sAllowedRemoteAccessIPs);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiMaxLogFileSize, m_uMaxLogFileSize);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiMaxLogBuff, m_iMaxLogBuff);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiLogFileFormat, m_iLogFileFormat);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiTxtEditor, m_strTxtEditor);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiPeerCacheShow, m_bPeerCacheShow);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiUseUserSortedServerList, m_bUseUserSortedServerList);
	DDX_TreeEdit(pDX,IDC_MOD_OPTS,m_htiCryptTCPPaddingLength,m_iCryptTCPPaddingLength );
	DDX_TreeEdit(pDX,IDC_MOD_OPTS,m_htiDebugSearchResultDetailLevel,m_iDebugSearchResultDetailLevel);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiAdjustNTFSDaylightFileTime, m_bAdjustNTFSDaylightFileTime);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htidatetimeformat, m_strDateTimeFormat);
	DDX_Text(pDX, IDC_MOD_OPTS, m_htidatetimeformat4log, m_strDateTimeFormat4Log);
	DDX_TreeColor(pDX, IDC_MOD_OPTS, m_htiLogError, m_crLogError);
	DDX_TreeColor(pDX, IDC_MOD_OPTS, m_htiLogWarning, m_crLogWarning);
	DDX_TreeColor(pDX, IDC_MOD_OPTS, m_htiLogSuccess, m_crLogSuccess);

#ifdef WS2 // NEO: WS2 - [WINSOCK2]
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiWinSock2, m_bWinSock2);
#endif // WS2 // NEO: WS2 END
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCreateCrashDump, m_bCreateCrashDump);	
	DDX_TreeRadio(pDX, IDC_MOD_OPTS, m_htiInternetSecurityZone, m_iInternetSecurityZone);

	DDX_Text(pDX, IDC_MOD_OPTS, m_htiEncryptCertName, m_strEncryptCertName);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCheckComctl32, m_bCheckComctl32);	
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiCheckShell32, m_bCheckShell32);	
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiIgnoreInstances, m_bIgnoreInstances);	
	DDX_Text(pDX, IDC_MOD_OPTS, m_htiMediaInfo_MediaInfoDllPath, m_MediaInfo_MediaInfoDllPath);
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiMediaInfo_RIFF, m_bMediaInfo_RIFF);	
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiMediaInfo_RM, m_bMediaInfo_RM);	
	DDX_TreeCheck(pDX, IDC_MOD_OPTS, m_htiMediaInfo_ID3LIB, m_bMediaInfo_ID3LIB);	
}

BOOL CPPgTweaks2::OnInitDialog()
{
	m_bHighresTimer = thePrefs.m_bHighresTimer;
	m_iFileBufferSize = thePrefs.m_iFileBufferSize;
	m_iFileBufferTime = MS2SEC(thePrefs.m_iFileBufferTime);
	m_ICH = thePrefs.ICH;
	m_dontcompressblocks = thePrefs.dontcompressblocks;
	m_dontcompressavi = thePrefs.dontcompressavi;
	m_bRemove2bin = thePrefs.m_bRemove2bin;
	m_bShowCopyEd2kLinkCmd = thePrefs.m_bShowCopyEd2kLinkCmd;
	m_nServerUDPPort = thePrefs.nServerUDPPort;
	m_bRestoreLastMainWndDlg = thePrefs.m_bRestoreLastMainWndDlg;
	m_bRestoreLastLogPane = thePrefs.m_bRestoreLastLogPane;
	m_bIconflashOnNewMessage = thePrefs.m_bIconflashOnNewMessage;
	m_bMiniMuleAutoClose = thePrefs.m_bMiniMuleAutoClose;
	m_iMiniMuleTransparency = thePrefs.m_iMiniMuleTransparency;
	m_iPreviewSmallBlocks = thePrefs.m_iPreviewSmallBlocks;
	m_bPreviewCopiedArchives = thePrefs.m_bPreviewCopiedArchives;
	m_bPreviewOnIconDblClk = thePrefs.m_bPreviewOnIconDblClk;
	m_bShowActiveDownloadsBold = thePrefs.m_bShowActiveDownloadsBold;
	m_bRTLWindowsLayout = thePrefs.m_bRTLWindowsLayout;
	m_bReBarToolbar = thePrefs.m_bReBarToolbar;
	m_iStraightWindowStyles = thePrefs.m_iStraightWindowStyles;
	m_iMaxChatHistory = thePrefs.m_iMaxChatHistory;
	m_maxmsgsessions = thePrefs.maxmsgsessions;
	m_bPreferRestrictedOverUser = thePrefs.m_bPreferRestrictedOverUser;
	m_bTrustEveryHash = thePrefs.m_bTrustEveryHash;
	m_bShowVerticalHourMarkers = thePrefs.m_bShowVerticalHourMarkers;
	m_iInspectAllFileTypes = thePrefs.m_iInspectAllFileTypes;
	m_iWebFileUploadSizeLimitMB = thePrefs.m_iWebFileUploadSizeLimitMB;
	m_uMaxLogFileSize = thePrefs.uMaxLogFileSize;
	m_iMaxLogBuff = thePrefs.iMaxLogBuff;
	m_iLogFileFormat = thePrefs.m_iLogFileFormat;
	m_strTxtEditor = thePrefs.m_strTxtEditor;
	m_bPeerCacheShow = thePrefs.m_bPeerCacheShow;
	m_bUseUserSortedServerList = thePrefs.m_bUseUserSortedServerList;
	m_iDebugSearchResultDetailLevel = thePrefs.m_iDebugSearchResultDetailLevel;
	m_iCryptTCPPaddingLength = thePrefs.m_byCryptTCPPaddingLength;
	m_bAdjustNTFSDaylightFileTime = thePrefs.m_bAdjustNTFSDaylightFileTime;
	m_strDateTimeFormat = thePrefs.m_strDateTimeFormat;
	m_strDateTimeFormat4Log = thePrefs.m_strDateTimeFormat4Log;
	m_crLogError = thePrefs.m_crLogError;
	m_crLogWarning = thePrefs.m_crLogWarning;
	m_crLogSuccess = thePrefs.m_crLogSuccess;

	m_sAllowedRemoteAccessIPs.Empty();
	for (int i = 0; thePrefs.m_aAllowedRemoteAccessIPs.GetCount(); i++)
		m_sAllowedRemoteAccessIPs += ipstr(thePrefs.m_aAllowedRemoteAccessIPs[i]) + _T(";");
	m_sAllowedRemoteAccessIPs.TrimRight(_T(";"));

	CString strZone = theApp.GetProfileString(_T("eMule"), _T("InternetSecurityZone"), _T("Untrusted"));
	if (strZone.CompareNoCase(_T("LocalMachine"))==0)
		m_iInternetSecurityZone = URLZONE_LOCAL_MACHINE;
	else if (strZone.CompareNoCase(_T("Intranet"))==0)
		m_iInternetSecurityZone = URLZONE_INTRANET;
	else if (strZone.CompareNoCase(_T("Trusted"))==0)
		m_iInternetSecurityZone = URLZONE_TRUSTED;
	else if (strZone.CompareNoCase(_T("Internet"))==0)
		m_iInternetSecurityZone = URLZONE_INTERNET;
	else {
		ASSERT( strZone.CompareNoCase(_T("Untrusted"))==0 );
		m_iInternetSecurityZone = URLZONE_UNTRUSTED;
	}

#ifdef WS2 // NEO: WS2 - [WINSOCK2]
	m_bWinSock2 = theApp.GetProfileInt(_T("Neo"), _T("WinSock2"), 0)!=0;
#endif // WS2 // NEO: WS2 END
	m_bCreateCrashDump = theApp.GetProfileInt(_T("eMule"), _T("CreateCrashDump"), 0)!=0;

	m_strEncryptCertName = theApp.GetProfileString(_T("eMule"), _T("NotifierMailEncryptCertName")).Trim();
	m_bCheckComctl32 = GetProfileInt(_T("eMule"), _T("CheckComctl32"), 1) != 0;
	m_bCheckShell32 = GetProfileInt(_T("eMule"), _T("CheckShell32"), 1) != 0;
	m_bIgnoreInstances = GetProfileInt(_T("eMule"), _T("IgnoreInstances"), 0) != 0;
	m_MediaInfo_MediaInfoDllPath = theApp.GetProfileString(_T("eMule"), _T("MediaInfo_MediaInfoDllPath"), _T("MEDIAINFO.DLL"));
	m_bMediaInfo_RIFF = theApp.GetProfileInt(_T("eMule"), _T("MediaInfo_RIFF"), 1) != 0;
	m_bMediaInfo_RM = theApp.GetProfileInt(_T("eMule"), _T("MediaInfo_RM"), 1) != 0;
	m_bMediaInfo_ID3LIB = theApp.GetProfileInt(_T("eMule"), _T("MediaInfo_ID3LIB"), 1) != 0;

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CPPgTweaks2::OnKillActive()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

BOOL CPPgTweaks2::OnApply()
{
	// if prop page is closed by pressing ENTER we have to explicitly commit any possibly pending
	// data from an open edit control
	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	if (!UpdateData())
		return FALSE;

	if(thePrefs.m_bHighresTimer != m_bHighresTimer)
		theApp.SetHighTimer(m_bHighresTimer);
	thePrefs.m_bHighresTimer = m_bHighresTimer;
	if(m_iFileBufferSize > 10*1024*1024)
		m_iFileBufferSize = 10*1024*1024;
	thePrefs.m_iFileBufferSize = m_iFileBufferSize;
	if(m_iFileBufferTime > 120)
		m_iFileBufferTime = 120;
	thePrefs.m_iFileBufferTime = SEC2MS(m_iFileBufferTime);
	thePrefs.ICH = m_ICH;
	thePrefs.dontcompressblocks = m_dontcompressblocks;
	thePrefs.dontcompressavi = m_dontcompressavi;
	thePrefs.m_bRemove2bin = m_bRemove2bin;
	thePrefs.m_bShowCopyEd2kLinkCmd = m_bShowCopyEd2kLinkCmd;
	thePrefs.nServerUDPPort = (uint16)m_nServerUDPPort;
	thePrefs.m_bRestoreLastMainWndDlg = m_bRestoreLastMainWndDlg;
	thePrefs.m_bRestoreLastLogPane = m_bRestoreLastLogPane;
	thePrefs.m_bIconflashOnNewMessage = m_bIconflashOnNewMessage;
	thePrefs.m_bMiniMuleAutoClose = m_bMiniMuleAutoClose;
	thePrefs.m_iMiniMuleTransparency = m_iMiniMuleTransparency;
	thePrefs.m_iPreviewSmallBlocks = m_iPreviewSmallBlocks;
	thePrefs.m_bPreviewCopiedArchives = m_bPreviewCopiedArchives;
	thePrefs.m_bPreviewOnIconDblClk = m_bPreviewOnIconDblClk;
	thePrefs.m_bShowActiveDownloadsBold = m_bShowActiveDownloadsBold;
	thePrefs.m_bRTLWindowsLayout = m_bRTLWindowsLayout;
	thePrefs.m_bReBarToolbar = m_bReBarToolbar;
	thePrefs.m_iStraightWindowStyles = m_iStraightWindowStyles;
	thePrefs.m_iMaxChatHistory = m_iMaxChatHistory;
	thePrefs.maxmsgsessions = m_maxmsgsessions;
	thePrefs.m_bPreferRestrictedOverUser = m_bPreferRestrictedOverUser;
	thePrefs.m_bTrustEveryHash = m_bTrustEveryHash;
	thePrefs.m_bShowVerticalHourMarkers = m_bShowVerticalHourMarkers;
	thePrefs.m_iInspectAllFileTypes = m_iInspectAllFileTypes;
	thePrefs.m_iWebFileUploadSizeLimitMB = m_iWebFileUploadSizeLimitMB;
	thePrefs.uMaxLogFileSize = m_uMaxLogFileSize;
	thePrefs.iMaxLogBuff = m_iMaxLogBuff;
	thePrefs.m_iLogFileFormat = (ELogFileFormat)m_iLogFileFormat;
	thePrefs.m_strTxtEditor = m_strTxtEditor;
	thePrefs.m_bPeerCacheShow = m_bPeerCacheShow;
	thePrefs.m_bUseUserSortedServerList = m_bUseUserSortedServerList;
	thePrefs.m_iDebugSearchResultDetailLevel = m_iDebugSearchResultDetailLevel;
	thePrefs.m_byCryptTCPPaddingLength = (uint8)m_iCryptTCPPaddingLength;
	thePrefs.m_bAdjustNTFSDaylightFileTime = m_bAdjustNTFSDaylightFileTime;
	thePrefs.m_strDateTimeFormat = m_strDateTimeFormat;
	thePrefs.m_strDateTimeFormat4Log = m_strDateTimeFormat4Log;
	thePrefs.m_crLogError = m_crLogError;
	thePrefs.m_crLogWarning = m_crLogWarning;
	thePrefs.m_crLogSuccess = m_crLogSuccess;

	thePrefs.m_aAllowedRemoteAccessIPs.RemoveAll();
	int iPos = 0;
	CString strIP = m_sAllowedRemoteAccessIPs.Tokenize(_T(";"), iPos);
	while (!strIP.IsEmpty())
	{
		u_long nIP = inet_addr(CStringA(strIP));
		if (nIP != INADDR_ANY && nIP != INADDR_NONE)
			thePrefs.m_aAllowedRemoteAccessIPs.Add(nIP);
		strIP = m_sAllowedRemoteAccessIPs.Tokenize(_T(";"), iPos);
	}

#ifdef WS2 // NEO: WS2 - [WINSOCK2]
	theApp.WriteProfileInt(_T("Neo"), _T("WinSock2"), m_bWinSock2 ? 1 : 0);
#endif // WS2 // NEO: WS2 END
	theApp.WriteProfileString(_T("eMule"), _T("InternetSecurityZone"), _asNetSecurityZones[m_iInternetSecurityZone]);
	theApp.WriteProfileInt(_T("eMule"), _T("CreateCrashDump"), m_bCreateCrashDump ? 1 : 0);

	theApp.WriteProfileString(_T("eMule"), _T("NotifierMailEncryptCertName"), m_strEncryptCertName);
	theApp.WriteProfileInt(_T("eMule"), _T("CheckComctl32"), m_bCheckComctl32 ? 1 : 0);
	theApp.WriteProfileInt(_T("eMule"), _T("CheckShell32"), m_bCheckShell32 ? 1 : 0);
	theApp.WriteProfileInt(_T("eMule"), _T("IgnoreInstances"), m_bIgnoreInstances ? 1 : 0);
	theApp.WriteProfileString(_T("eMule"), _T("MediaInfo_MediaInfoDllPath"), m_MediaInfo_MediaInfoDllPath);
	theApp.WriteProfileInt(_T("eMule"), _T("MediaInfo_RIFF"), m_bMediaInfo_RIFF ? 1 : 0);
	theApp.WriteProfileInt(_T("eMule"), _T("MediaInfo_RM"), m_bMediaInfo_RM ? 1 : 0);
	theApp.WriteProfileInt(_T("eMule"), _T("MediaInfo_ID3LIB"), m_bMediaInfo_ID3LIB ? 1 : 0);

	SetModified(FALSE);
	return CPropertyPage::OnApply();
}

void CPPgTweaks2::Localize()
{	
	if (!m_hWnd)
		return;
	SetWindowText(GetResString(IDS_X_PW_TWEAK2));
}

void CPPgTweaks2::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	CPropertyPage::OnDestroy();
}

LRESULT CPPgTweaks2::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam)
{
	if (wParam == IDC_MOD_OPTS)
	{
		TREEOPTSCTRLNOTIFY* pton = (TREEOPTSCTRLNOTIFY*)lParam;
		if(pton->nmhdr.code != EN_KILLFOCUS)
			SetModified();
	}
	return 0;
}

void CPPgTweaks2::OnHelp()
{
	//theApp.ShowHelp(0);
}

BOOL CPPgTweaks2::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgTweaks2::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}

void CPPgTweaks2::TreeItemsToNull()
{
	m_htiHighresTimer = NULL;
	m_htiFileBufferSize = NULL;
	m_htiFileBufferTime = NULL;
	m_htiICH = NULL;
	m_htidontcompressblocks = NULL;
	m_htidontcompressavi = NULL;
	m_htiRemove2bin = NULL;
	m_htiShowCopyEd2kLinkCmd = NULL;
	m_htiServerUDPPort = NULL;
	m_htiRestoreLastMainWndDlg = NULL;
	m_htiRestoreLastLogPane = NULL;
	m_htiIconflashOnNewMessage = NULL;
	m_htiPreviewSmallBlocks = NULL;
	m_htiPreviewCopiedArchives = NULL;
	m_htiPreviewOnIconDblClk = NULL;
	m_htiShowActiveDownloadsBold = NULL;
	m_htiRTLWindowsLayout = NULL;
	m_htiReBarToolbar = NULL;
	m_htiStraightWindowStyles = NULL;
	m_htiMaxChatHistory = NULL;
	m_htimaxmsgsessions = NULL;
	m_htiPreferRestrictedOverUser = NULL;
	m_htiTrustEveryHash = NULL;
	m_htiShowVerticalHourMarkers = NULL;
	m_htiInspectAllFileTypes = NULL;
	m_htiWebFileUploadSizeLimitMB = NULL;
	m_htiAllowedRemoteAccessIPs = NULL;
	m_htiMaxLogFileSize = NULL;
	m_htiMaxLogBuff = NULL;
	m_htiLogFileFormat = NULL;
	m_htiTxtEditor = NULL;
	m_htiPeerCacheShow = NULL;
	m_htiUseUserSortedServerList = NULL;
	m_htiDebugSearchResultDetailLevel = NULL;
	m_htiCryptTCPPaddingLength = NULL;
	m_htidatetimeformat = NULL;
	m_htidatetimeformat4log = NULL;
	m_htiLogError = NULL;
	m_htiLogWarning = NULL;
	m_htiLogSuccess = NULL;

#ifdef WS2 // NEO: WS2 - [WINSOCK2]
	m_htiWinSock2 = NULL;
#endif // WS2 // NEO: WS2 END
	m_htiInternetSecurityZone = NULL;
	m_htiCreateCrashDump = NULL;
	m_htiMiniMuleAutoClose = NULL;
	m_htiMiniMuleTransparency = NULL;
	m_htiAdjustNTFSDaylightFileTime = NULL;

	m_htiEncryptCertName = NULL;
	m_htiCheckComctl32 = NULL;
	m_htiCheckShell32 = NULL;
	m_htiIgnoreInstances = NULL;
	m_htiMediaInfo_MediaInfoDllPath = NULL;
	m_htiMediaInfo_RIFF = NULL;
	m_htiMediaInfo_RM = NULL;
	m_htiMediaInfo_ID3LIB = NULL;
}
// NEO: MOD END <-- Xanatos --