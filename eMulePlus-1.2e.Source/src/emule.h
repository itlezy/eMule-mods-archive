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

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"
#include "emuleDlg.h"
#include "knownfilelist.h"
#include "preferences.h"
#ifdef OLD_SOCKETS_ENABLED
#include "sockets.h"
#endif //OLD_SOCKETS_ENABLED
#include "SearchList.h"
#ifdef OLD_SOCKETS_ENABLED
#include "ListenSocket.h"
#endif //OLD_SOCKETS_ENABLED
#include "UploadQueue.h"
#include "DownloadQueue.h"
#include "DownloadList.h"
#include "ClientList.h"
#include "ClientCredits.h"
#include "FriendList.h"
#include "filehashcontrol.h"
#ifdef OLD_SOCKETS_ENABLED
#include "clientudpsocket.h"
#endif //OLD_SOCKETS_ENABLED
#include "Loggable.h"
#include "fakecheck.h"

#ifdef OLD_SOCKETS_ENABLED
class CListenSocket;
#endif
class CFirewallOpener;
class CDownloadQueue;
class CUploadQueue;
class CSearchList;
class CServerList;
class CIP2Country;
class CWebServer;
class MiniDumper;
class CMMServer;
class CIPFilter;
class CAutoDL;
class DbEnv;
class Db;

class CEmuleApp : public CWinApp, public CLoggable
{
public:
	enum AppState
	{
		APP_STATE_RUNNING = 0,
		APP_STATE_SHUTTINGDOWN,
		APP_STATE_DONE
	};

	CEmuleApp();

	CEmuleDlg			*m_pMDlg;
	CDownloadList		*m_pDownloadList;
	CClientList			*m_pClientList;
	CKnownFileList		*m_pKnownFilesList;
	CPreferences		*m_pPrefs;
#ifdef OLD_SOCKETS_ENABLED
	CServerConnect		*m_pServerConnect;
#endif
	CServerList			*m_pServerList;
	CSharedFileList		*m_pSharedFilesList;
	CSearchList			*m_pSearchList;
#ifdef OLD_SOCKETS_ENABLED
	CListenSocket		*m_pListenSocket;
#endif
	CUploadQueue		*m_pUploadQueue;
	CDownloadQueue		*m_pDownloadQueue;
	CClientCreditsList	*m_pClientCreditList;
	CFriendList			*m_pFriendList;
#ifdef OLD_SOCKETS_ENABLED
	CClientUDPSocket	*m_pClientUDPSocket;
#endif
	CWebServer			*m_pWebServer;
	CIP2Country			*m_pIP2Country;
	CFirewallOpener		*m_pFirewallOpener;
	CAutoDL				*m_pAutoDL;

	CString				*m_pstrPendingLink;
	tagCOPYDATASTRUCT	m_sendStruct;
	CFileHashControl	m_fileHashControl;
	CCriticalSection	m_csPreventExtensiveHDAccess;
	CIPFilter			*m_pIPFilter;
	CFakeCheck			*m_pFakeCheck;
	CMMServer			*m_pMMServer;

	uint32				m_dwPublicIP;	// current (valid) public IP or 0 if unknown

	uint64				stat_sessionReceivedBytes;
	uint64				stat_sessionSentBytes;
	uint16				stat_reconnects;
	DWORD				stat_transferStarttime;
	DWORD				stat_serverConnectTime;
	DWORD				stat_starttime;
	ULONGLONG			m_qwComCtrlVer;
	int					m_iDfltImageListColorFlags;

//	Detailed filtering statistics
	LONG				m_lTotalFiltered;
	LONG				m_lIncomingFiltered;
	LONG				m_lOutgoingFiltered;
	LONG				m_lSXFiltered;

	DbEnv			   *pDbEnv;	// 	Database environment

	CArray<CString,CString> m_strWebServiceURLArray;

//	ICC customization
	void				InitURLs();
	CStringArray		m_ICCURLs;

	AppState			m_app_state; // defines application state for shutdown

//	ed2k link functions
	CString		StripInvalidFilenameChars(CString strText, bool bKeepSpaces = true);
	bool		CopyTextToClipboard(const CString &strText) const;
	bool		CopyTextFromClipboard(CString *pstrContent);

	void		OnlineSig();
	void		UpdateReceivedBytes(uint32 bytesToAdd);
	void		UpdateSentBytes(uint32 bytesToAdd);

	DECLARE_MESSAGE_MAP()

protected:
	int			ProcessCommandline();
	void		SetTimeOnTransfer();
	UINT		GetWindowModuleFileNameEx(HWND hwnd,LPTSTR pszFileName,UINT cchFileNameMax);
	static BOOL CALLBACK SearchEmuleWindow(HWND hWnd, LPARAM lParam);
	void		OnHelp();

	HIMAGELIST			m_hSystemImageList;
	CMapStringToPtr		m_aExtToSysImgIdx;
	CSize				m_sizSmallSystemIcon;

public:
	virtual BOOL		InitInstance();
	virtual int ExitInstance();
	virtual BOOL PreTranslateMessage(MSG *pMsg);

	uint32		GetPublicIP() const				{ return m_dwPublicIP; }
	void		SetPublicIP(uint32 dwIP);

	int			GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength = -1);
	HIMAGELIST	GetSystemImageList() const		{ return m_hSystemImageList; }
	CSize		GetSmallSytemIconSize()			{ return m_sizSmallSystemIcon; }

private:
	MiniDumper *m_pDump;
	HANDLE m_hMutexOneInstance;
};

extern CEmuleApp g_App;
