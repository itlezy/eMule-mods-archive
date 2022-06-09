//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#ifdef OLD_SOCKETS_ENABLED
#include "sockets.h"
#endif //OLD_SOCKETS_ENABLED
#include "ServerListCtrl.h"
#include "KnownFileList.h"
#include "TransferWnd.h"
#include "ServerWnd.h"
#include "PreferencesDlg.h"
#include "SharedFilesWnd.h"
#include "StringConversion.h"
#include "SearchDlg.h"
#include "ChatWnd.h"
#include "TrayDialog.h"
#include "BtnST.h"
#include "StatisticsDlg.h"
#include "MeterIcon.h"
#include "MuleToolBarCtrl.h"
#include "MuleStatusBarCtrl.h"
#include "IrcWnd.h"
#include "TaskbarNotifier.h"
#include "MuleSystrayDlg.h"
#include "Loggable.h"
#include "ToolTips\PPToolTip.h"

#define MP_RESTORE		4001
#define MP_CONNECT		4002
#define MP_DISCONNECT	4003
#define MP_EXIT			4004

class CKnownFileList;

class CEmuleDlg : public CTrayDialog, public CLoggable
{
friend class CMuleToolbarCtrl;

public:
	CEmuleDlg(CWnd* pParent = NULL);
	~CEmuleDlg();
	enum { IDD = IDD_EMULE_DIALOG };

	void			AddServerMessageLine(LPCTSTR line);
	void			AddBugReport(LPCTSTR strFunctionName, LPCTSTR sFile, uint32 dwLine, LPCTSTR pcMsg);
	void			OutputLogText(const CString& strLogText, CHTRichEditCtrl* pRichEditCtrl);
	void			AddLogText(int iMode, const CString &strTxt);
	void			ShowConnectionState();
	void			ShowConnectionState(bool bConnected, const CString &strServer = _T(""), bool bIconOnly = false);
	void			ShowNotifier(const CString &strText, int iMsgType, bool bForceSoundOFF = false, bool bMessageEnabled = false);
	void			ShowUserCount(uint32 dwCount);
	void			ShowMessageState(byte iconnr);
	void			ShowSessionTime();
	void			SetActiveDialog(CDialog* dlg);
	void			ShowTransferRate(bool bUpdateAll = false);
	void			Localize();
	void			ResizeStatusBar();
	bool			IsRunning();
	void			RunBackupNow(bool automated);
	void			DisableAutoBackup();
	void			PostUniqueMessage(UINT uiMsg);
	void			SendMail(LPCTSTR pcText, bool bMsgEnabled, bool bSendEnabled);
	static UINT		CheckCurrentVersionAtInet(void *);

	afx_msg BOOL OnQueryEndSession();
	afx_msg void OnNcDestroy();
	afx_msg void OnBnClickedButton2();
	BOOL TrayShow(void);

	CTransferWnd		m_wndTransfer;
	CServerWnd			m_wndServer;
	CPreferencesDlg		m_dlgPreferences;
	CSharedFilesWnd		m_wndSharedFiles;
	CSearchDlg			m_dlgSearch;
	CChatWnd			m_wndChat;
	CMuleStatusBarCtrl	m_ctlStatusBar;
	CMuleToolbarCtrl	m_ctlToolBar;
#ifdef USE_REBAR
	CReBarCtrl			m_ctlReBar;
#endif
	CDialog				*m_pdlgActive;
	CStatisticsDlg		m_dlgStatistics;
	CIrcWnd				m_wndIRC;
	CTaskbarNotifier	m_wndTaskbarNotifier;
	int					m_iStatus;
	CFont				m_fontDefault;
	CPPToolTip			m_ttip;
	CImageList			m_clientImgLists[CLIENT_IMGLST_COUNT];	//destroyed automatically in destructors
	HICON				m_hiconSourceTray;
#ifdef NEW_SOCKETS
//	CEngineData*	m_pEngineData;
#endif

protected:
	HICON				m_hIcon;

	virtual void		DoDataExchange(CDataExchange* pDX);
	virtual BOOL		OnInitDialog();
	virtual BOOL		PreTranslateMessage(MSG* pMsg);
	virtual void		OnTrayRButtonUp(CPoint pt);
	virtual BOOL		OnCommand(WPARAM wParam, LPARAM lParam);
	virtual BOOL		OnWndMsg(UINT iMessage,WPARAM wParam, LPARAM lParam, LRESULT *pResult);

	afx_msg void		OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void		OnPaint();
	afx_msg HCURSOR		OnQueryDragIcon();
	afx_msg void		OnTimer(UINT_PTR nIDEvent);
	afx_msg int			OnCreate(LPCREATESTRUCT lpCreateStruct);

	void QuickSpeedOther(UINT nID);

	afx_msg LRESULT OnProcessTaskUI(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnWebServerConnect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerRemove(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebSharedFilesReload(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerAddToStatic(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerRemoveFromStatic(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerClearCompleted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnWebServerFileRename(WPARAM wParam, LPARAM lParam);

	afx_msg LRESULT OnTaskbarNotifierClicked(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnWMData(WPARAM wParam,LPARAM lParam);
	afx_msg LRESULT OnFileHashed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnHashFailed(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFileHashingStarted(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAreYouEmule(WPARAM, LPARAM);
	afx_msg void OnActivate(UINT nState, CWnd *pWndOther, BOOL bMinimized);

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual void OnSize(UINT nType, int cx, int cy);
	void			OnOK()	{}
	void			OnClose();
	void			OnCancel();
	void			DisConnect();

	DECLARE_MESSAGE_MAP()

private:
	CString			m_strDebugLogFilePath;
	CString			m_strLogFilePath;

	CString			*m_pstrNewLogTextLines;
	CString			*m_pstrNewDebugLogTextLines;

	HICON			m_hiconConn;
	HICON			m_hiconTrans[4];
	HICON			m_hiconIM[3];
	HICON			m_hiconMyTray;
	HICON			m_hiconUsers;

	CMeterIcon		m_trayMeterIcon;

	HICON			m_hiconSourceTrayLowID;
	HICON			m_hiconSourceTrayGrey;

	uint32			lastuprate;
	uint32			lastdownrate;
	CImageList		m_imageList;
	CMuleSystrayDlg *m_pSystrayDlg;

	UINT_PTR		m_hTimer;
	bool			m_bStartUpMinimized;
	bool			m_bCliExit;

private:
	static void CALLBACK Timer(HWND hwnd, UINT uiMsg, UINT idEvent, DWORD dwTime);

	void			StartConnection();
	void			CloseConnection();
	void			RestoreWindow();
	void			UpdateTrayIcon(int procent);
	void			AddSpeedSelectorSys(CMenu* addToMenu);

	void			BackupFromAppDir(LPCTSTR pcExtensionToBack, int iMode);
	void			BackupFromTempDir(LPCTSTR pcExtensionToBack, int iMode);
	void			BackupFiles(LPCTSTR pcExtensionToBack, int iMode, const CString &strPath, const CString &strBackupPath);

	CString*		GetNewLogTextLines();
	CString*		GetNewDebugLogTextLines();

	void			GetInfo4ToolTip(NM_PPTOOLTIP_DISPLAY *pNotify);
};
