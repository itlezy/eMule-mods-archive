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

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "server.h"
#include "opcodes.h"
#include "WebServer.h"
#include "Jumpstarter.h"
#include <Tlhelp32.h>
#include "IP2Country.h"
#include "ed2k_filetype.h"
#include "SharedFileList.h"
#include "FirewallOpener.h"
#include "ServerList.h"
#include "AutoDL.h"
#include "MMServer.h"
#include "mdump.h"
#include "BerkeleyDb/build_win32/db_cxx.h"
#include "IPFilter.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


BEGIN_MESSAGE_MAP(CEmuleApp, CWinApp)
	ON_COMMAND(ID_HELP, OnHelp)
END_MESSAGE_MAP()

CEmuleApp				g_App;
const static UINT		UWM_ARE_YOU_EMULE = RegisterWindowMessage(EMULE_GUID);
#ifdef _DEBUG
static CMemoryState		oldMemState, newMemState, diffMemState;
#endif _DEBUG

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	MSLU (Microsoft Layer for Unicode) support - UnicoWS
//	Details:
//	http://en.wikipedia.org/wiki/Microsoft_Layer_for_Unicode
//	http://www.trigeminal.com/usenet/usenet031.asp?1033
extern "C" HMODULE __stdcall ExplicitPreLoadUnicows()
{
	static const char pcUnicowsError[] =
		"This eMule Plus version requires the \"Microsoft(R) Layer for Unicode(TM) on Windows(R) 95/98/ME Systems\".\n"
		"\n"
		"Download the MSLU package from Microsoft(R) here:\n"
		"        http://www.microsoft.com/downloads/details.aspx?FamilyId=73BA7BD7-ED06-4F0D-80A4-2A7EEAEE17E2\n"
		"or\n"
		"        search the Microsoft(R) Download Center http://www.microsoft.com/downloads/ for \"MSLU\" or \"unicows\".\n"
		"\n"
		"\n"
		"After downloading the MSLU package, run the \"unicows.exe\" program and specify your eMule Plus installation folder "
		"where to place the extracted files from the package.\n"
		"\n"
		"Ensure that the file \"unicows.dll\" was placed in your eMule Plus installation folder and start eMule Plus again.";

#ifdef _AFXDLL
	// UnicoWS support *requires* statically linked MFC and C-RTL.

	// NOTE: Do *NOT* use any MFC nor W-functions here!
	// NOTE: Do *NOT* use eMule's localization functions here!
	MessageBoxA(NULL, 
				"This eMule Plus version (Unicode, MSLU, shared MFC) does not run with this version of Windows.", 
				"eMule Plus", 
				MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
	exit(1);
#endif

	// Pre-Load UnicoWS -- needed for proper initialization of MFC/C-RTL
	HMODULE hUnicoWS = LoadLibraryA("unicows.dll");

	if (hUnicoWS == NULL)
	{
		MessageBoxA(NULL, pcUnicowsError, "eMule Plus", MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		exit(1);
	}

	return hUnicoWS;
}

// NOTE: Do *NOT* change the name of this function. It *HAS* to be named "_PfnLoadUnicows" !
extern "C" HMODULE (__stdcall *_PfnLoadUnicows)(void) = ExplicitPreLoadUnicows;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CEmuleApp::CEmuleApp()
{
	m_dwPublicIP = 0;
	m_pDump = NULL;
	m_qwComCtrlVer = MAKEDLLVERULL(4,0,0,0);
	m_hSystemImageList = NULL;
	m_sizSmallSystemIcon.cx = GetSystemMetrics(SM_CXSMICON);
	m_sizSmallSystemIcon.cy = GetSystemMetrics(SM_CYSMICON);
	m_iDfltImageListColorFlags = ILC_COLOR;

//	Get session start time offset
	stat_starttime = ::GetTickCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CEmuleApp Initialising
BOOL CEmuleApp::InitInstance()
{
#ifdef _DEBUG
	oldMemState.Checkpoint();
#endif

#ifdef EP_SPIDERWEB
//	Setup Structured Exception handler for the main thread
	_set_se_translator(StructuredExceptionHandler);
#endif

//	We have to catch exception by ourselves!
	m_pDump = new MiniDumper();

//#ifdef OLD_SOCKETS_ENABLED
//	We need sockets started before we proccess the preferences
	if (!AfxSocketInit())
	{
		AfxMessageBox(IDS_SOCKETS_INIT_FAILED);
		return FALSE;
	}
//#endif //OLD_SOCKETS_ENABLED

	AfxOleInit();

//	Create & initialize all the important stuff
	m_pstrPendingLink = NULL;

	bool	bIsRunningButNoCmd = false;
	int		iResult = ProcessCommandline();

	if (iResult == 0)
		return false;
	else if (iResult == 1)
		bIsRunningButNoCmd = true;

	_tsetlocale(LC_ALL, _T(""));
	m_pPrefs = new CPreferences();

//	If we don't want to allow multiple instances...
	if (bIsRunningButNoCmd && !m_pPrefs->GetMultiple())
		return false;

//	Must be called after reading preference as language identification is used for string loading
	m_pDump->LoadStrings();

//	InitCommonControls() is required on Windows XP if an application
//	manifest specifies use of ComCtl32.dll version 6 or later to enable
//	visual styles.  Otherwise, any window creation will fail.
	InitCommonControls();

	DWORD	dwComCtrlMjr = 4;
	DWORD	dwComCtrlMin = 0;

	AtlGetCommCtrlVersion(&dwComCtrlMjr, &dwComCtrlMin);
	m_qwComCtrlVer = MAKEDLLVERULL(dwComCtrlMjr,dwComCtrlMin,0,0);

	m_iDfltImageListColorFlags = GetAppImageListColorFlag();

//	Don't use 32bit color resources if not supported by comctl
	if (m_iDfltImageListColorFlags == ILC_COLOR32 && m_qwComCtrlVer < MAKEDLLVERULL(6,0,0,0))
		m_iDfltImageListColorFlags = ILC_COLOR16;
#if (_MSC_VER < 1500)	// W9x support removed in VC 9.0, VS 2008
//	Don't use >8bit color resources with OSs with restricted memory for GDI resources
	if (afxData.bWin95)
		m_iDfltImageListColorFlags = ILC_COLOR8;
#endif

	CWinApp::InitInstance();

//	Enable containment of OLE controls
	AfxEnableControlContainer();

//	Set the priority of the processes main thread
	SetThreadPriority(THREAD_PRIORITY_NORMAL + m_pPrefs->GetMainProcessPriority());

	CEmuleDlg	dlg;

	m_pMDlg = &dlg;
	m_pMainWnd = &dlg;

	EMULE_TRY

//	Set main process priority
	switch (m_pPrefs->GetHashingPriority())
	{
		case 0:
			iResult = THREAD_PRIORITY_IDLE;
			break;
		case 1:
			iResult = THREAD_PRIORITY_LOWEST;
			break;
		default:
			iResult = THREAD_PRIORITY_BELOW_NORMAL;
			break;
	}
	m_fileHashControl.SetThreadPriority(iResult);

// 	ICC (Internet Connection Check) customization
	InitURLs();

	m_fileHashControl.Init();

	if (m_pPrefs->AutoTakeED2KLinks())
		Ask4RegFix(true);

	try
	{
		CString	sDbHome = m_pPrefs->GetAppDir();

		sDbHome += _T("Db");

		::CreateDirectory(sDbHome, NULL);	// In case it doesn't exist

		pDbEnv = new DbEnv(0);
		pDbEnv->set_cachesize(0, 256 * 1024, 0);	// 256kb cache size
		USES_CONVERSION;
		pDbEnv->open( CT2CA(sDbHome), DB_CREATE | DB_INIT_LOCK | DB_INIT_LOG
									| DB_INIT_MPOOL | DB_INIT_TXN | DB_RECOVER
									| DB_THREAD, 0 );
		CJumpstarter::OpenDatabases(pDbEnv);
	}
	catch (DbRunRecoveryException &DbRecEx)
	{
		AfxMessageBox(CString(DbRecEx.what()));
		return FALSE;
	}
#ifdef _DEBUG
	catch (DbException &DbEx)
	{
		pDbEnv = NULL;
		TRACE1("Problems create database objects: %hs. Closing.\n", DbEx.what());
		return FALSE;
	}
#endif
	catch(...)
	{
		pDbEnv = NULL;
		TRACE0("Problems create database objects. Closing.\n");
		return FALSE;
	}

//	Intialize filetype array
	InitFileTypeArray();

	m_pFirewallOpener = new CFirewallOpener();
	m_pFirewallOpener->Init(true);	// We need to init it now (even if we may not use it yet) because of CoInitializeSecurity - which kinda ruins the sense of the class interface but ooohh well :P
//	Open WinXP firewall ports if set in preferences and possible
	if (m_pPrefs->GetOpenPorts() && m_pFirewallOpener->DoesFWConnectionExist())
	{
	//	Delete old rules added by eMule
		m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_UDP);
		m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_TCP);
	//	Open port for this session
		if (m_pFirewallOpener->OpenPort(m_pPrefs->GetPort(), NAT_PROTOCOL_TCP, EMULE_DEFAULTRULENAME_TCP, true))
			AddLogLine(LOG_RGB_SUCCESS, IDS_FO_PORT_OK, m_pPrefs->GetPort(), _T("TCP"));
		else
			AddLogLine(LOG_RGB_ERROR, IDS_FO_PORT_FAIL, m_pPrefs->GetPort(), _T("TCP"));

		if (m_pPrefs->GetUDPPort())
		{
		//	Open port for this session
			if (m_pFirewallOpener->OpenPort(m_pPrefs->GetUDPPort(), NAT_PROTOCOL_UDP, EMULE_DEFAULTRULENAME_UDP, true))
				AddLogLine(LOG_RGB_SUCCESS, IDS_FO_PORT_OK, m_pPrefs->GetUDPPort(), _T("UDP"));
			else
				AddLogLine(LOG_RGB_ERROR, IDS_FO_PORT_FAIL, m_pPrefs->GetUDPPort(), _T("UDP"));
		}
	}

	m_pIP2Country	= new CIP2Country();
	m_pDownloadList = new CDownloadList();
	m_pClientList = new CClientList();
//	Construct the Friends List from "emfriends.met"
	m_pFriendList = new CFriendList();
	m_pSearchList = new CSearchList();
//	Construct the Known Files list from "known.met"
	m_pKnownFilesList = new CKnownFileList(m_pPrefs->GetConfigDir());
//	Construct an _empty_ Server list.
	m_pServerList = new CServerList();
#ifdef OLD_SOCKETS_ENABLED
	m_pServerConnect = new CServerConnect();
	m_pSharedFilesList = new CSharedFileList(m_pPrefs, m_pServerConnect, m_pKnownFilesList);
	m_pListenSocket = new CListenSocket();
	m_pClientUDPSocket	= new CClientUDPSocket();
#else
	m_pSharedFilesList = new CSharedFileList(m_pPrefs,NULL,m_pKnownFilesList);
#endif //OLD_SOCKETS_ENABLED
	m_pClientCreditList = new CClientCreditsList();
	m_pDownloadQueue = new CDownloadQueue(m_pSharedFilesList);	// bugfix - do this before creating the m_pUploadQueue
	m_pUploadQueue = new CUploadQueue();
	m_pIPFilter 	= new CIPFilter();
	m_pWebServer = new CWebServer();
#ifdef OLD_SOCKETS_ENABLED
	m_pMMServer = new CMMServer();
#endif //OLD_SOCKETS_ENABLED
	m_pFakeCheck 	= new CFakeCheck();
	m_pAutoDL		= new CAutoDL();

	EMULE_CATCH

//	Reset statistics values
	stat_sessionReceivedBytes = 0;
	stat_sessionSentBytes = 0;
	stat_reconnects = 0;
	stat_transferStarttime = 0;
	stat_serverConnectTime = 0;

//	Detailed filter stats
	m_lTotalFiltered = 0;
	m_lIncomingFiltered = 0;
	m_lOutgoingFiltered = 0;
	m_lSXFiltered = 0;

	dlg.DoModal();

#ifdef _DEBUG
	newMemState.Checkpoint();
	if (diffMemState.Difference(oldMemState, newMemState))
	{
		TRACE("Memory usage:\n");
		diffMemState.DumpStatistics();
	}
#endif //_DEBUG

	return FALSE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CEmuleApp::ProcessCommandline()
{
	static const TCHAR pcHelp[] =
		CLIENT_NAME _T(" Command-line Interface (CLI)\n")
		_T("\n")
		_T("Usage: emule [{option | \"ed2k-link\"}]\n")
		_T("\n")
		_T("Options:\n")
		_T("connect\t\tconnect to any server\n")
		_T("disconnect\tdisconnect from the current server\n")
		_T("limits=[up][:down]\tset upload/download limits; Ex: limits=10:64\n")
		_T("preferences\tsave preferences settings to the disk\n")
		_T("reload\t\treload shared files (hash newly added files)\n")
		_T("restore\t\tmaximize application window\n")
		_T("resume\t\tresume a paused download\n")
		_T("status\t\tsave download status to status.log file\n")
		_T("help\t\tthis message\n")
		_T("exit\t\texit application");
	HWND	hMainInst = NULL;
	bool	bAlreadyRunning = false;
	int		iRc;

	m_hMutexOneInstance = CreateMutex(NULL, FALSE, EMULE_GUID);

	bAlreadyRunning = (::GetLastError() == ERROR_ALREADY_EXISTS || ::GetLastError() == ERROR_ACCESS_DENIED);
	if (bAlreadyRunning)
		EnumWindows(SearchEmuleWindow, (LPARAM)&hMainInst);

//	Parameters are passed and the first one isn't empty string
	if ((__argc > 1) && (__targv[1][0] != _T('\0')))
	{
		const TCHAR	*pcArg = __targv[1];
		CString		strCmd(((*pcArg == _T('/')) || (*pcArg == _T('-'))) ? (pcArg + 1) : pcArg);

	//	If the file name to open is in the form of an URL...
		if (strCmd.Find(_T("://")) > 0)
		{
		//	Just forward the URL to the existing eMule instance
			m_sendStruct.cbData = (strCmd.GetLength() + 1) * sizeof(TCHAR);
			m_sendStruct.dwData = OP_ED2KLINK;
			m_sendStruct.lpData = const_cast<LPTSTR>(strCmd.GetString());
 			if (hMainInst != NULL)
			{
				SendMessage(hMainInst, WM_COPYDATA, 0, (LPARAM)(PCOPYDATASTRUCT)&m_sendStruct);
				return 0;
			}
			m_pstrPendingLink = new CString(strCmd);
		}
		else
		{
		//	Forward the command line to the existing eMule instance
			m_sendStruct.cbData = (strCmd.GetLength() + 1) * sizeof(TCHAR);
			m_sendStruct.dwData = OP_CLCOMMAND;
			m_sendStruct.lpData = const_cast<LPTSTR>(strCmd.GetString());
			if (hMainInst != NULL)
				iRc = SendMessage(hMainInst, WM_COPYDATA, 0, (LPARAM)(PCOPYDATASTRUCT)&m_sendStruct);
			else
				iRc = ((strCmd != _T("help")) && (strCmd != _T("h")) && (strCmd != _T("?")));
			if (iRc == 0)
				MessageBox(NULL, pcHelp, CLIENT_NAME, MB_ICONSTOP | MB_SYSTEMMODAL | MB_SETFOREGROUND);
		//	Don't start application if main instance wasn't found when CLI command was in use
		//	All CLI commands intended only to be used with main application already running
			return 0;
		}
	}

	if (hMainInst != NULL)
		return 0;
	else if (bAlreadyRunning)
		return 1;
	else
		return 2;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SearchEmuleWindow() is an EnumWindows callback proc. It sends an eMule Windows user message to the
//		window 'hWnd' and waits for a response.
BOOL CALLBACK CEmuleApp::SearchEmuleWindow(HWND hWnd, LPARAM lParam)
{
	DWORD dwMsgResult;
	LRESULT res = ::SendMessageTimeout(hWnd,UWM_ARE_YOU_EMULE,0, 0,SMTO_BLOCK |SMTO_ABORTIFHUNG,10000,&dwMsgResult);

//	If the window didn't respond, tell EnumWindows to continue with the next window
	if(res == 0)
		return TRUE;

//	If we got a response...
	if (dwMsgResult == UWM_ARE_YOU_EMULE)
	{
	//  Obtain directories (running and destination)
		TCHAR strSourcePath[MAX_PATH];
		TCHAR strDestPath[MAX_PATH];
		::GetModuleFileName(NULL, strSourcePath, MAX_PATH);
		g_App.GetWindowModuleFileNameEx(hWnd, strDestPath, MAX_PATH);

	//	Compare and if equal we should send the command to this window
		if (_tcsicmp(strSourcePath, strDestPath) == 0)
		{
		//	Return the window handle in '*lParam'.
			*reinterpret_cast<HWND*>(lParam) = hWnd;
			return FALSE;
		}
		else
			return TRUE;
	}
	return TRUE;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleApp::UpdateReceivedBytes(uint32 bytesToAdd)
{
	SetTimeOnTransfer();
	stat_sessionReceivedBytes+=bytesToAdd;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleApp::UpdateSentBytes(uint32 bytesToAdd)
{
	SetTimeOnTransfer();
	stat_sessionSentBytes+=bytesToAdd;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleApp::SetTimeOnTransfer()
{
	if (stat_transferStarttime > 0)
		return;

	stat_transferStarttime=GetTickCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CEmuleApp::StripInvalidFilenameChars(CString strText, bool bKeepSpaces)
{
	static const TCHAR acReservDevNames[][9] =
	{
		_T("NUL"), _T("CON"), _T("PRN"), _T("AUX"),
		_T("COM1"),_T("COM2"),_T("COM3"),_T("COM4"),_T("COM5"),_T("COM6"),_T("COM7"),_T("COM8"),_T("COM9"),
		_T("LPT1"),_T("LPT2"),_T("LPT3"),_T("LPT4"),_T("LPT5"),_T("LPT6"),_T("LPT7"),_T("LPT8"),_T("LPT9"),
		_T("CLOCK$"),	_T("CONFIG$"), _T("$MMXXXX0"), _T("XMSXXXX0"), _T("IFS$HLP$")	// Win9x only
	};
	LPTSTR		pcSrc, pcDst;
	TCHAR		cCh;
	unsigned	uiLen;

	pcSrc = pcDst = strText.GetBuffer();

	while ((cCh = *pcSrc) != _T('\0'))
	{
		if ( !( (static_cast<unsigned>(cCh) <= 31u)	// lots of invalid chars for filenames in windows :=)
			|| cCh == _T('\"') || cCh == _T('*') || cCh == _T('<')  || cCh == _T('>')
			|| cCh == _T('?')  || cCh == _T('|') || cCh == _T('\\') || cCh == _T('/')
			|| cCh == _T(':') ) )
		{
			if (!bKeepSpaces && cCh == _T(' '))
				cCh = _T('.');
			*pcDst = cCh;
			pcDst++;
		}
		pcSrc++;
	}
	*pcDst = _T('\0');

	strText.ReleaseBuffer();

	for (unsigned i = 0; i < ARRSIZE(acReservDevNames); i++)
	{
		uiLen = _tcslen(reinterpret_cast<const TCHAR*>(&acReservDevNames[i]));
		if (_tcsnicmp(strText, reinterpret_cast<const TCHAR*>(&acReservDevNames[i]), uiLen) == 0)
		{
			if (static_cast<unsigned>(strText.GetLength()) == uiLen)
			{
				strText += _T("_");	// Filename is a reserved file name
				break;
			}
			else if (strText.GetString()[uiLen] == _T('.'))
			{
				strText.SetAt(uiLen, _T('_'));	//	Starts with a reserved file name followed by '.'
				break;
			}
		}
	}

	return strText;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Note: since Win9x doesn't support automatic ANSI->Unicode conversion, we need to add using both formats
bool CEmuleApp::CopyTextToClipboard(const CString &strText) const
{
	if (strText.IsEmpty())
		return false;

//	Open the Clipboard and insert the handle into the global memory
	if (OpenClipboard(NULL) == NULL)
		return false;

	uint32	iCopied = 0;

	if (EmptyClipboard())
	{
		CStringA	strTextA(strText);

	//	Copy ANSI text
		HGLOBAL	hGlobalA = GlobalAlloc(GHND | GMEM_SHARE, (strTextA.GetLength() + 1) * sizeof(CHAR));

		if (hGlobalA != NULL)
		{
			LPSTR	pGlobalA = static_cast<LPSTR>(GlobalLock(hGlobalA));

			if (pGlobalA != NULL)
			{
				strcpy(pGlobalA, strTextA);
				GlobalUnlock(hGlobalA);
				if (SetClipboardData(CF_TEXT, hGlobalA) != NULL)
					iCopied |= 1;
			}
			if ((iCopied & 1) == 0)
				GlobalFree(hGlobalA);
		}

#ifdef _UNICODE
	//	Copy Unicode text
		HGLOBAL	hGlobalU = GlobalAlloc(GHND | GMEM_SHARE, (strText.GetLength() + 1) * sizeof(WCHAR));

		if (hGlobalU != NULL)
		{
			LPWSTR	pGlobalU = static_cast<LPWSTR>(GlobalLock(hGlobalU));

			if (pGlobalU != NULL)
			{
				wcscpy(pGlobalU, strText);
				GlobalUnlock(hGlobalU);
				if (SetClipboardData(CF_UNICODETEXT, hGlobalU) != NULL)
					iCopied |= 2;
			}
			if ((iCopied & 2) == 0)
				GlobalFree(hGlobalU);
		}
#endif
	}

	CloseClipboard();

//	This is here so eMule won't think the clipboard has ed2k links for adding
	if (iCopied != 0)
		g_App.m_pMDlg->m_dlgSearch.IgnoreClipBoardLinks(strText);

	return (iCopied != 0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CEmuleApp::CopyTextFromClipboard(CString *pstrContent)
{
	HGLOBAL	hGlobal;
	bool	bIsCopied = false;
#ifdef _UNICODE
	UINT	uiFormat = CF_UNICODETEXT;
#else
	UINT	uiFormat = CF_TEXT;
#endif

	if (!pstrContent->IsEmpty())
		pstrContent->Truncate(0);

	do
	{
		if (IsClipboardFormatAvailable(uiFormat))
		{
			if (OpenClipboard(NULL) == NULL)
				break;

			hGlobal = GetClipboardData(uiFormat);
			if (hGlobal != NULL)
			{
				const void	*pGlobal = GlobalLock(hGlobal);

				if (pGlobal != NULL)
				{
					if (uiFormat == CF_UNICODETEXT)
						*pstrContent = (const TCHAR*)pGlobal;
					else
						*pstrContent = (const char*)pGlobal;
					GlobalUnlock(hGlobal);
					bIsCopied = true;
				}
			}
			CloseClipboard();
		}
		else
			uiFormat = CF_TEXT;
	}
	while ((uiFormat != CF_TEXT) && !bIsCopied);

	return bIsCopied;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleApp::OnlineSig()
{
	if (!m_pPrefs->IsOnlineSignatureEnabled())
		return;

	CStdioFile	file;
	char		acBuf[20];
	CString		strFullPath = m_pPrefs->GetAppDir();

	strFullPath += _T("onlinesig.dat");
	if (!file.Open(strFullPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeBinary))
	{
		AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERROR_SAVEFILE, _T("onlinesig.dat"));
		return;
	}

#ifdef OLD_SOCKETS_ENABLED
	if (m_pServerConnect->IsConnected())
	{
		CServer		*pCurServer = m_pServerConnect->GetCurrentServer();
		CStringA	strBufA(pCurServer->GetListName());

		file.Write("1|", 2);
		file.Write(strBufA, strBufA.GetLength());

		file.Write("|", 1);
		strBufA = pCurServer->GetFullIP();
		file.Write(strBufA, strBufA.GetLength());
		file.Write("|", 1);
		file.Write(acBuf, strlen(_itoa(pCurServer->GetPort(), acBuf, 10)));
	}
	else
#endif //OLD_SOCKETS_ENABLED
		file.Write("0", 1);

	file.Write("\n", 1);
	file.Write(acBuf, sprintf(acBuf, "%.1f", static_cast<double>(m_pDownloadQueue->GetDataRate()) / 1024.0));
	file.Write("|", 1);
	file.Write(acBuf, sprintf(acBuf, "%.1f", static_cast<double>(m_pUploadQueue->GetDataRate()) / 1024.0));
	file.Write("|", 1);
	file.Write(acBuf, strlen(_itoa(m_pUploadQueue->GetWaitingUserCount(), acBuf, 10)));

	file.Close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Handling a better process exit
int CEmuleApp::ExitInstance()
{
	EMULE_TRY
	m_pMDlg = NULL;

	m_fileHashControl.Destroy();

	delete m_pFakeCheck;
	m_pFakeCheck = NULL;
#ifdef OLD_SOCKETS_ENABLED
	delete m_pListenSocket;
	m_pListenSocket = NULL;
	delete m_pClientUDPSocket;
	m_pClientUDPSocket = NULL;
	delete m_pServerConnect;
	m_pServerConnect = NULL;
#endif //OLD_SOCKETS_ENABLED
	delete m_pServerList;
	m_pServerList = NULL;
	delete m_pKnownFilesList;
	m_pKnownFilesList = NULL;
	delete m_pDownloadQueue;
	m_pDownloadQueue = NULL;
	delete m_pUploadQueue;
	m_pUploadQueue = NULL;
	delete m_pSharedFilesList;
	m_pSharedFilesList = NULL;
	delete m_pSearchList;
	m_pSearchList = NULL;
	delete m_pClientCreditList;
	m_pClientCreditList = NULL;
	delete m_pClientList;
	m_pClientList = NULL;
	delete m_pDownloadList;
	m_pDownloadList = NULL;
	delete m_pFriendList;
	m_pFriendList = NULL;
	delete m_pAutoDL;
	m_pAutoDL = NULL;
	delete m_pPrefs;
	m_pPrefs = NULL;
	delete m_pIPFilter;
	m_pIPFilter = NULL;
	delete m_pIP2Country;
	m_pIP2Country = NULL;
	delete m_pWebServer;
	m_pWebServer = NULL;
#ifdef OLD_SOCKETS_ENABLED
	delete m_pMMServer;
	m_pMMServer = NULL;
#endif //OLD_SOCKETS_ENABLED
	delete m_pFirewallOpener;
	m_pFirewallOpener = NULL;

	try
	{
		CJumpstarter::CloseDatabases();
	//	This is true after client couldn't start properly,
	//	e.g. disallowed to start due to disabled multiple instances
		if (pDbEnv != NULL)
		{
			pDbEnv->txn_checkpoint(0, 0, 0);
			pDbEnv->close(0);
			delete pDbEnv;
		}
	}
#ifdef _DEBUG
	catch (DbException &pDbEnv)
	{
		TRACE1("Problems delete database objects: %hs.\n", pDbEnv.what());
	}
#endif
	catch (...)
	{
		TRACE("Problems delete database objects.\n");
	}

	m_app_state = APP_STATE_DONE;

	delete m_pDump;
	m_pDump = NULL;

	CloseHandle(m_hMutexOneInstance);

	return CWinApp::ExitInstance();
	EMULE_CATCH2
	return 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	OnHelp() is the handler for the Windows Help message
void CEmuleApp::OnHelp()
{
	ShellExecute(NULL, _T("open"), _T("http://emuleplus.info/forum/index.php?showforum=23"), NULL, NULL, SW_SHOW);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int CEmuleApp::GetFileTypeSystemImageIdx(LPCTSTR pszFilePath, int iLength /* = -1 */)
{
//	TODO: This has to be MBCS aware..
	DWORD dwFileAttributes;
	LPCTSTR pszCacheExt = NULL;
	if (iLength == -1)
		iLength = _tcslen(pszFilePath);
	if (iLength > 0 && (pszFilePath[iLength - 1] == _T('\\') || pszFilePath[iLength - 1] == _T('/')))
	{
	//	It's a directory
		pszCacheExt = _T("\\");
		dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
	}
	else
	{
		dwFileAttributes = FILE_ATTRIBUTE_NORMAL;
	//	Search last '.' character *after* the last '\\' character
		for (int i = iLength - 1; i >= 0; i--)
		{
			if (pszFilePath[i] == _T('\\') || pszFilePath[i] == _T('/'))
				break;
			if (pszFilePath[i] == _T('.'))
			{
			//	Point to 1st character of extension (skip the '.')
				pszCacheExt = &pszFilePath[i + 1];
				break;
			}
		}
		if (pszCacheExt == NULL)
			pszCacheExt = _T(""); // empty extension
	}

//	Search extension in "ext->idx" cache.
	LPVOID vData;
	if (!m_aExtToSysImgIdx.Lookup(pszCacheExt, vData))
	{
	//	Get index for the system's small icon image list
		SHFILEINFO	sfi;
		DWORD	dwResult = SHGetFileInfo( pszFilePath, dwFileAttributes, &sfi, sizeof(sfi),
			SHGFI_USEFILEATTRIBUTES | SHGFI_SYSICONINDEX | SHGFI_SMALLICON );

		if (dwResult == 0)
			return 0;
		ASSERT( m_hSystemImageList == NULL || m_hSystemImageList == (HIMAGELIST)dwResult );
		m_hSystemImageList = (HIMAGELIST)dwResult;

	//	Store icon index in local cache
		m_aExtToSysImgIdx.SetAt(pszCacheExt, (LPVOID)sfi.iIcon);
		return sfi.iIcon;
	}
//	Return already cached value
//	Elandal: Assumes sizeof(void*) == sizeof(int)
	return (int)vData;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	InitURLs() parses the domains for the Internet Callback Check out of the pref string into
//		the string array 'm_ICCURLs'.
void CEmuleApp::InitURLs()
{
	CString	strToken, strList(m_pPrefs->GetURLsForICC().MakeLower());
	int		iCurPos = 0;

	m_ICCURLs.RemoveAll();
	for (;;)
	{
		strToken = strList.Tokenize(_T("|"), iCurPos);
		if (strToken.IsEmpty())
			break;
		m_ICCURLs.Add(strToken);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CEmuleApp::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN || pMsg->message == WM_SYSKEYDOWN)
	{
		int iMessage = 0;
		short nCode = GetCodeFromPressedKeys(pMsg);

		if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_MINIMIZE))
		{
			m_pMDlg->TrayShow();
			m_pMDlg->ShowWindow(SW_HIDE);
			return TRUE;
		}
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_SWITCH))
		{
			if (m_pMDlg->m_pdlgActive == &m_pMDlg->m_wndServer)
				iMessage = IDC_TOOLBARBUTTON + MTB_TRANSFER;
			else if (m_pMDlg->m_pdlgActive == &m_pMDlg->m_wndTransfer)
				iMessage = IDC_TOOLBARBUTTON + MTB_SEARCH;
			else if (m_pMDlg->m_pdlgActive == &m_pMDlg->m_dlgSearch)
				iMessage = IDC_TOOLBARBUTTON + MTB_SHAREDFILES;
			else if (m_pMDlg->m_pdlgActive == &m_pMDlg->m_wndSharedFiles)
				iMessage = IDC_TOOLBARBUTTON + MTB_MESSAGES;
			else if (m_pMDlg->m_pdlgActive == &m_pMDlg->m_wndChat)
				iMessage = IDC_TOOLBARBUTTON + MTB_IRC;
			else if (m_pMDlg->m_pdlgActive == &m_pMDlg->m_wndIRC)
				iMessage = IDC_TOOLBARBUTTON + MTB_STATISTICS;
			else if (m_pMDlg->m_pdlgActive == &m_pMDlg->m_dlgStatistics)
				iMessage = IDC_TOOLBARBUTTON + MTB_SERVERS;
		}
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_SRV))
			iMessage = IDC_TOOLBARBUTTON + MTB_SERVERS;
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_TRANSFER))
			iMessage = IDC_TOOLBARBUTTON + MTB_TRANSFER;
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_SEARCH))
			iMessage = IDC_TOOLBARBUTTON + MTB_SEARCH;
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_SHAREDFILES))
			iMessage = IDC_TOOLBARBUTTON + MTB_SHAREDFILES;
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_CHAT))
			iMessage = IDC_TOOLBARBUTTON + MTB_MESSAGES;
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_IRC))
			iMessage = IDC_TOOLBARBUTTON + MTB_IRC;
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_STATS))
			iMessage = IDC_TOOLBARBUTTON + MTB_STATISTICS;
		else if (nCode == m_pPrefs->GetShortcutCode(SCUT_WIN_PREFS))
			iMessage = IDC_TOOLBARBUTTON + MTB_PREFS;

		if (iMessage > 0)
		{
			m_pMDlg->PostMessage(WM_COMMAND, iMessage);
			return TRUE;
		}
	}

	return CWinApp::PreTranslateMessage(pMsg);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	GetWindowModuleFileNameEx obtains the directory of a running process in any platform
//	so we can choose which app to send the CLI command to.
UINT CEmuleApp::GetWindowModuleFileNameEx(HWND hwnd, LPTSTR pszFileName, UINT cchFileNameMax)
{
	UINT uiRetval = 0;
	DWORD dwThreadId, dwProcessId;
	dwThreadId = GetWindowThreadProcessId(hwnd,&dwProcessId);
	HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwProcessId);

	if (hProcess == NULL)
		return 0;

	OSVERSIONINFO osver;

//	Check to see if were running under Windows95 or Windows NT.
	osver.dwOSVersionInfoSize = sizeof(osver);
	if (!GetVersionEx(&osver))
		goto Out2;

//	If Windows NT:
	if (osver.dwPlatformId == VER_PLATFORM_WIN32_NT)
	{
		HINSTANCE hPSAPI_Dll = LoadLibrary(_T("PSAPI.DLL"));

		if (hPSAPI_Dll == NULL)
			goto Out2;

		BOOL (WINAPI *lpfEnumProcessModules)(HANDLE, HMODULE *, DWORD, LPDWORD);
		DWORD (WINAPI *lpfGetModuleFileNameEx)(HANDLE, HMODULE, LPTSTR, DWORD);

		lpfEnumProcessModules = (BOOL(WINAPI *)(HANDLE, HMODULE *, DWORD, LPDWORD))
			GetProcAddress(hPSAPI_Dll, "EnumProcessModules");
#ifndef _UNICODE
		lpfGetModuleFileNameEx = (DWORD (WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD))
			GetProcAddress(hPSAPI_Dll, "GetModuleFileNameExA");
#else
		lpfGetModuleFileNameEx = (DWORD (WINAPI *)(HANDLE, HMODULE, LPTSTR, DWORD))
			GetProcAddress(hPSAPI_Dll, "GetModuleFileNameExW");
#endif
		HMODULE hModule;
		DWORD dwSize = sizeof(HMODULE);

		if (lpfEnumProcessModules(hProcess, &hModule, sizeof(HMODULE), &dwSize))
			uiRetval = lpfGetModuleFileNameEx(hProcess, hModule, pszFileName, cchFileNameMax);

		FreeLibrary(hPSAPI_Dll);
	}
	else if (osver.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS)
	{
		HINSTANCE hKernel32 = LoadLibrary(_T("PSAPI.DLL"));

		if (hKernel32 == NULL)
			goto Out2;

	//	ToolHelp Function Pointers
		HANDLE (WINAPI *lpfCreateToolhelp32Snapshot)(DWORD, DWORD);
		BOOL (WINAPI *lpfProcess32First)(HANDLE, LPPROCESSENTRY32);
		BOOL (WINAPI *lpfProcess32Next)(HANDLE, LPPROCESSENTRY32);

		lpfCreateToolhelp32Snapshot = (HANDLE(WINAPI *)(DWORD,DWORD)) GetProcAddress(hKernel32, "CreateToolhelp32Snapshot");
		lpfProcess32First = (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32)) GetProcAddress(hKernel32, "Process32First");
		lpfProcess32Next = (BOOL(WINAPI *)(HANDLE,LPPROCESSENTRY32)) GetProcAddress(hKernel32, "Process32Next");

		if (lpfProcess32Next == NULL || lpfProcess32First == NULL || lpfCreateToolhelp32Snapshot == NULL)
			goto Out1;

		PROCESSENTRY32 procentry;
		procentry.dwSize = sizeof(PROCESSENTRY32);
		HANDLE hSnapshot = lpfCreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		if (hSnapshot == NULL)
			goto Out1;

		BOOL bFlag;
		bFlag = lpfProcess32First(hSnapshot,&procentry);

		while (bFlag)
		{
			if (procentry.th32ProcessID == dwProcessId)
			{
				_tcsncpy(pszFileName, procentry.szExeFile, cchFileNameMax);
				pszFileName[cchFileNameMax - 1] = _T('\0');
				uiRetval = _tcslen(pszFileName) + 1;
				break;
			}
			procentry.dwSize = sizeof(PROCESSENTRY32);
			bFlag = lpfProcess32Next(hSnapshot, &procentry);
		}

		CloseHandle(hSnapshot);
Out1:
		FreeLibrary(hKernel32);
	}
Out2:
	CloseHandle(hProcess);
	return uiRetval;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CEmuleApp::SetPublicIP(uint32 dwIP)
{
	if (dwIP != 0)
	{
		if (GetPublicIP() == 0)
			AddLogLine(LOG_FL_DBG, _T("My public IP address: %s"), ipstr(dwIP));
		m_dwPublicIP = dwIP;
		if (m_pServerList != NULL)
			m_pServerList->CheckForExpiredUDPKeys();
	}
	else
	{
		m_dwPublicIP = dwIP;
		AddLogLine(LOG_FL_DBG, _T("Deleted public IP"));
	}
}
