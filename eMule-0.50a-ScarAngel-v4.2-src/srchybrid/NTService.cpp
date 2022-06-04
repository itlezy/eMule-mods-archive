// NTService.cpp : Run as a windows nt service
//this file is part of eMule morphXT
//Copyright (C)2006 leuk_he ( strEmail.Format("%s@%s", "leukhe", "gmail.com") / http://emulemorph.sf.net )
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
#include <winsvc.h>
#include <afxwin.h>
#include <afxinet.h> 

#include "userMsgs.h"
#include "emule.h"
#include "preferences.h"
#include "emuledlg.h"
#include "log.h"
#include "NTservice.h"
#include "Modversion.h" // for service name
#include "OtherFunctions.h"
#include "Opcodes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


bool RunningAsServiceStat;
bool NtserviceStartwhenclose=false;

SERVICE_STATUS          ssStatus;       // current status of the service
SERVICE_STATUS_HANDLE   sshStatusHandle;
HANDLE  hServerStopEvent = NULL;
HANDLE  hWaitForServiceToStart=NULL;
CWinThread* pThread ;
CWinApp* pApp ;
static HANDLE s_hServiceMutex;

void terminateService(int wincode);
BOOL StartServiceThread();
extern int AFXAPI AfxWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPTSTR lpCmdLine, int nCmdShow);
static void  SetSeviceMutex(); 

void CALLBACK ServiceMain( DWORD dwArgc,  LPTSTR* lpszArgv);
UINT ServiceExecutionThread( int  pParam );
static const TCHAR szAfxOldWndProc[] = _T("AfxOldWndProc423");  


UINT  RegisterServicePoint( LPVOID /* Param */ ) 
{
	int retcode;
	SERVICE_TABLE_ENTRY dispatchTable[] =
	{
		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		/*
		{ TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		*/
		{ (!thePrefs.GetServiceName().IsEmpty())?const_cast<LPWSTR>((LPCWSTR)thePrefs.GetServiceName()):TEXT(SZSERVICENAME), (LPSERVICE_MAIN_FUNCTION)ServiceMain},
		// <== Adjustable NT Service Strings [Stulle] - Stulle
		{ NULL, NULL }
	};
	RunningAsServiceStat=1;
	retcode=StartServiceCtrlDispatcher(dispatchTable);
	return retcode;
}

void OnStartAsService()
{ 
	hWaitForServiceToStart= CreateEvent(
			NULL,    // no security attributes
			TRUE,    // manual reset event
			FALSE,   // not-signalled
			NULL);   // no name
	AfxBeginThread(RegisterServicePoint,0);
	if (hWaitForServiceToStart)
		WaitForSingleObject( hWaitForServiceToStart,INFINITE );

	return;
}


bool RunningAsService()
{
	return 	RunningAsServiceStat;
}



void CALLBACK ServiceMain( DWORD ,  LPTSTR* )
{ 
	int  success;
	RunningAsServiceStat=1;
	// First we must call the Registration function
	// ==> Adjustable NT Service Strings [Stulle] - Stulle
	/*
	sshStatusHandle= RegisterServiceCtrlHandler(_T(SZSERVICENAME),
	*/
	sshStatusHandle= RegisterServiceCtrlHandler((!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():_T(SZSERVICENAME),
	// <== Adjustable NT Service Strings [Stulle] - Stulle
						(LPHANDLER_FUNCTION) service_ctrl);
	if (!sshStatusHandle)
	{
		terminateService(GetLastError());
		return;
	}

	SetSeviceMutex(); //tell any gui versions starting.
	// Now create the our service termination event to block on
	hServerStopEvent = CreateEvent (0, TRUE, FALSE, 0);
	if (!hServerStopEvent )
	{
		terminateService(GetLastError());
		return;
	}
	// Notify the SCM of progress again
	success = ReportStatusToSCMgr(SERVICE_START_PENDING, 0, 60000); // 60 secs! 
	if (!success)
	{
		terminateService(GetLastError());
		return;
	}

	// Start the service execution thread by calling our StartServiceThread function...
	//success = 	ServiceExecutionThread(SW_HIDE);
	// Note:
	if ( hWaitForServiceToStart)
		SetEvent(hWaitForServiceToStart); // continue main. 


	if (!success)
	{
		terminateService(GetLastError());
		return;
	}
	// Now just wait for 
	// terminates the service!
	WaitForSingleObject( hServerStopEvent,INFINITE );
	//cleanup:

	if (hServerStopEvent)
		CloseHandle(hServerStopEvent);
	terminateService(0);
}




void ServiceStartedSuccesfully()  // when connected 
{
	int success ;
	// The service is now running.  Notify the SCM of this fact.
	if ( ssStatus.dwCurrentState!= SERVICE_RUNNING)
	{
		ssStatus.dwCurrentState = SERVICE_RUNNING;
		success = ReportStatusToSCMgr(SERVICE_RUNNING,  0, 0);
		if (!success)
		{
			terminateService(GetLastError());
			AddLogLine(false,GetResString(IDS_SVC_STRT_MGR_FAIL)); 
			return;
		}
	}
}

void ServiceStartedPaused()  // when autoconnect is set to false
{
	int success ;
	ServiceStartedSuccesfully();
	Sleep(1000);  // let it realize it was started. 
	// pause it, since not connected. 
	if ( ssStatus.dwCurrentState!= SERVICE_PAUSED)
	{
		ssStatus.dwCurrentState = SERVICE_PAUSED;
		success = ReportStatusToSCMgr(SERVICE_PAUSED,  0, 0);
		if (!success)
		{
			terminateService(GetLastError());
			AddLogLine(false,GetResString(IDS_SVC_PAUSE_MGR_FAIL)); 
			return;
		}
	}
}

VOID CALLBACK service_ctrl(DWORD dwCtrlCode)
{
	// Handle the requested control code.
	//
	switch(dwCtrlCode)
	{
		// Stop the service.
		//
		// SERVICE_STOP_PENDING should be reported before
		// setting the Stop Event - hServerStopEvent - in
		// ServiceStop().  This avoids a race condition
		// which may result in a 1053 - The Service did not respond...
		// error.
		case SERVICE_CONTROL_STOP:
			ReportStatusToSCMgr(SERVICE_STOP_PENDING, NO_ERROR, 0);
			theApp.m_app_state = APP_STATE_SHUTTINGDOWN; // no ask. 
			theApp.emuledlg->SendMessage(WM_CLOSE); // close it. 
			ServiceStop();
			return;
		case SERVICE_CONTROL_PAUSE:
			if 	(!theApp.IsConnected())
				theApp.emuledlg->CloseConnection();
			ssStatus.dwCurrentState =SERVICE_PAUSED;
			break;
		case  SERVICE_CONTROL_CONTINUE:
			theApp.emuledlg->StartConnection(); // Connect;
			ssStatus.dwCurrentState = SERVICE_START_PENDING;
		// Update the service status.
		//
		case SERVICE_CONTROL_INTERROGATE:
			// do in main loop to check if main loop is still running. 
			SendMessage(theApp.emuledlg->m_hWnd,UM_SERVERSTATUS,ssStatus.dwCurrentState,0); 
			return ;
		// invalid control code
		//
		default:
			break;
	}

	ReportStatusToSCMgr(ssStatus.dwCurrentState, NO_ERROR, 0);
}

//
//  FUNCTION: ReportStatusToSCMgr()
//
//  PURPOSE: Sets the current status of the service and
//           reports it to the Service Control Manager
//

BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint)
{
	static DWORD dwCheckPoint = 1;
	int  fResult;
	ssStatus.dwServiceType        = SERVICE_WIN32; 
	if (dwCurrentState == SERVICE_START_PENDING)
		ssStatus.dwControlsAccepted = 0;
	else
		ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP|SERVICE_ACCEPT_PAUSE_CONTINUE;
	ssStatus.dwCurrentState = dwCurrentState;
	ssStatus.dwWin32ExitCode = dwWin32ExitCode;
	ssStatus.dwServiceSpecificExitCode =   0;
	ssStatus.dwWaitHint = dwWaitHint;

	if	( ( dwCurrentState == SERVICE_RUNNING ) ||
		  ( dwCurrentState == SERVICE_STOPPED ) ||
		  ( dwCurrentState == SERVICE_PAUSED )
		)
			ssStatus.dwCheckPoint = 0;
	else
			ssStatus.dwCheckPoint = dwCheckPoint++;

		// Report the status of the service to the service control manager.
		//

		fResult = SetServiceStatus( sshStatusHandle, &ssStatus);
		return fResult ;
}

//  Installs the service

int CmdInstallService(bool b_autostart)
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;
	int retval=0;
	TCHAR szPath[512];
	CString ErrString;

    
	if ( GetModuleFileName( NULL, szPath, 512 ) == 0 )
	{
		GetSystemErrorString(GetLastError(),ErrString);
		AddLogLine(false,GetResString(IDS_SVC_INST_PATH_FAIL),ErrString);
		return 10;
	}
	_tcscat(szPath,_T(" -AsAService"));
	schSCManager = OpenSCManager(
						NULL,                   // machine (NULL == local)
						NULL,                   // database (NULL == default)
						SC_MANAGER_ALL_ACCESS   // access required
						);
	if ( schSCManager )
	{
		CString DisplayName;
		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		/*
		DisplayName.Format(_T(SZSERVICEDISPLAYNAME),MOD_VERSION);
		schService = CreateService(
			schSCManager,               // SCManager database
			TEXT(SZSERVICENAME),        // name of service
		*/
		if(!thePrefs.GetServiceDispName().IsEmpty())
			DisplayName.Format(thePrefs.GetServiceDispName());
		else
			DisplayName.Format(_T(SZSERVICEDISPLAYNAME),MOD_VERSION);
		schService = CreateService(
			schSCManager,               // SCManager database
			(!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():TEXT(SZSERVICENAME),        // name of service
		// <== Adjustable NT Service Strings [Stulle] - Stulle
			DisplayName,				// name to display
			SERVICE_ALL_ACCESS,         // desired access
			SERVICE_WIN32_OWN_PROCESS /* | SERVICE_INTERACTIVE_PROCESS */ ,  // service type can be emulesecure also ....
			b_autostart?SERVICE_AUTO_START:SERVICE_DEMAND_START,       // start type
			SERVICE_ERROR_NORMAL,       // error control type
			szPath,                     // service's binary
			NULL,                       // no load ordering group
			NULL,                       // no tag identifier
			TEXT(SZDEPENDENCIE),        // dependencies: tcpip + ? 
			NULL,                       // LocalSystem account
			NULL);                      // no password

		if ( schService )
		{
			SERVICE_DESCRIPTION sdBuf;
			// ==> Adjustable NT Service Strings [Stulle] - Stulle
			if(!thePrefs.GetServiceDescr().IsEmpty()) 
				sdBuf.lpDescription = const_cast<LPWSTR>((LPCWSTR)thePrefs.GetServiceDescr());
			else
			// <== Adjustable NT Service Strings [Stulle] - Stulle
				sdBuf.lpDescription = SZSERVICEDESCR;
			(void)!ChangeServiceConfig2( // I don't care if it works :P
				schService,                 // handle to service
				SERVICE_CONFIG_DESCRIPTION, // change: description
				&sdBuf);
			AddLogLine(false,GetResString(IDS_SVC_INST_SUCCESS));
			CloseServiceHandle(schService);
		}
		else
		{
			GetSystemErrorString(GetLastError(),ErrString);
			AddLogLine(true,GetResString(IDS_SVC_INST_FAIL),ErrString);
			retval=10;
		}

		CloseServiceHandle(schSCManager);
	}
	else
	{
		GetSystemErrorString(GetLastError(),ErrString);
		AddLogLine(true,GetResString(IDS_SVC_SVCCTRL_FAIL),ErrString);
		retval=11;
	}
	return retval;
}

int CmdRemoveService() //  Stops and removes the service
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;
	int retval =0;
	CString ErrString;

	schSCManager = OpenSCManager(
						NULL,                   // machine (NULL == local)
						NULL,                   // database (NULL == default)
						SC_MANAGER_ALL_ACCESS   // access required
						);

	if ( schSCManager )
	{
		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		/*
		schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);
		*/
		schService = OpenService(schSCManager, (!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);
		// <== Adjustable NT Service Strings [Stulle] - Stulle

		if (schService)
		{
			// try to stop the service
			if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
			{
				AddLogLine(true,GetResString(IDS_SVC_STOPPING));
				Sleep( 400 );

				while( QueryServiceStatus( schService, &ssStatus ) )
				{
					if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
					{
						AddLogLine(true,GetResString(IDS_SVC_STOP_PNDNG));
						Sleep( 400 );
					}
					else
						break;
				}

				if ( ssStatus.dwCurrentState == SERVICE_STOPPED )
					AddLogLine(true,GetResString(IDS_SVC_STOPPED));
				else
				{
					GetSystemErrorString(GetLastError(),ErrString);
					AddLogLine(true,GetResString(IDS_SVC_STOP_FAIL),ErrString);
					retval=10;
				}
			}

			// now remove the service
			if( DeleteService(schService) )
				AddLogLine(true,GetResString(IDS_SVC_DELETED));
			else
			{
				GetSystemErrorString(GetLastError(),ErrString);
				AddLogLine(true,GetResString(IDS_SVC_DELETE_FAIL),ErrString);
				retval=10;
			}

			CloseServiceHandle(schService);
		}
		else
		{
			GetSystemErrorString(GetLastError(),ErrString);
			AddLogLine(true,GetResString(IDS_SVC_OPEN_FAIL),ErrString);
			retval=10;
		}

		CloseServiceHandle(schSCManager);
	}
	else
	{
		GetSystemErrorString(GetLastError(),ErrString);
		AddLogLine(true,GetResString(IDS_SVC_SVCCTRL_FAIL),ErrString);
		retval=10;
	}
	return retval;
}

//  FUNCTION: ServiceStop
//
//  PURPOSE: Signal the main servicce thread to fisnish. 

VOID ServiceStop()
{
	if ( hServerStopEvent )
		SetEvent(hServerStopEvent);
}

/* oops we are down */
void terminateService(int wincode)
{
	CString ErrString;
	GetSystemErrorString(wincode,ErrString);
	if(s_hServiceMutex) CloseHandle(s_hServiceMutex); // close mutex 
	AddLogLine(false,GetResString(IDS_SVC_TERMINATE),wincode,ErrString);
	if (wincode)
		ReportStatusToSCMgr(SERVICE_STOPPED, ERROR_SERVICE_SPECIFIC_ERROR, 0);
	else
		ReportStatusToSCMgr(SERVICE_STOPPED, 0, 0);
	return;
}

int	NTServiceGet(int  &b_installed,	int	&i_startupmode,	int	&i_enoughrights)
{
	SC_HANDLE	schService;
	SC_HANDLE	schSCManager;
	QUERY_SERVICE_CONFIG * lpssServiceConfig;
	char buffer[4096];
	int	merror=0;
	DWORD dummy;
	b_installed=-1; // unknown. 

	schSCManager = OpenSCManager(
		NULL,					// machine (NULL ==	local)
		NULL,					// database	(NULL == default)
		GENERIC_READ			// access required
		);
	if ( schSCManager )
	{   
		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		/*
		schService = OpenService(schSCManager, TEXT(SZSERVICENAME),	GENERIC_READ);
		*/
		schService = OpenService(schSCManager, (!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():TEXT(SZSERVICENAME),	GENERIC_READ);
		// <== Adjustable NT Service Strings [Stulle] - Stulle
		if (schService ) {
			lpssServiceConfig= (QUERY_SERVICE_CONFIG *)buffer;
			if	(QueryServiceConfig(schService,	lpssServiceConfig,sizeof(buffer),&dummy) ){
				b_installed=1;
				if (lpssServiceConfig->dwStartType==SERVICE_DEMAND_START)
					i_startupmode =	0 ;	//manual;
				if (lpssServiceConfig->dwStartType==SERVICE_AUTO_START)
					i_startupmode =	1 ;	//auto;
				if (lpssServiceConfig->dwStartType==SERVICE_DISABLED)
					i_startupmode =	4 ;	// disabled or marked for deletion
				i_enoughrights=0;
			}
			else {
				merror=GetLastError(); // queryservice config failed
				if (merror==ERROR_ACCESS_DENIED) {
					b_installed=1;
					i_startupmode=0;
					i_enoughrights=-4; // cannot retrieve status.
				}
			}
			CloseServiceHandle(schService);
		}
		else
		{
			merror=GetLastError(); // openservice failed.
			if (merror==ERROR_SERVICE_DOES_NOT_EXIST) {
				b_installed=0;
				i_startupmode=0;
				i_enoughrights=0;
				i_enoughrights=ERROR_SERVICE_DOES_NOT_EXIST;
			}
			else if	(merror==ERROR_ACCESS_DENIED){
				i_startupmode=0;
				i_enoughrights=-3; //NO	rights to query	service
			}	 
			else
			{
				i_startupmode=0;
				i_enoughrights=-1; //unknown ERROR
			}
		}
		CloseServiceHandle(schSCManager);
	}
	else
	{
		merror=GetLastError(); // cannot connect to	service	manager.
		i_startupmode=0;
		if (merror==ERROR_ACCESS_DENIED)
			i_enoughrights=-2; //  NO rights to	connect. 
		else 
			i_enoughrights=-1; //	unknown	error. 
		return merror;
	}
	return merror;
}

int NTServiceSetStartupMode(int i_startupmode){
	SC_HANDLE	schService;
	SC_HANDLE	schSCManager;
	CString ErrString;

	schSCManager = OpenSCManager(
		NULL,					// machine (NULL ==	local)
		NULL,					// database	(NULL == default)
		GENERIC_WRITE|GENERIC_READ// access required
		);
	if ( schSCManager )
	{   
		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		/*
		schService = OpenService(schSCManager, TEXT(SZSERVICENAME),GENERIC_WRITE|GENERIC_READ);
		*/
		schService = OpenService(schSCManager, (!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():TEXT(SZSERVICENAME),GENERIC_WRITE|GENERIC_READ);
		// <== Adjustable NT Service Strings [Stulle] - Stulle
		if (schService ) {
			if (ChangeServiceConfig( 
				schService,        // handle of service 
				SERVICE_NO_CHANGE, // service type: no change 
				(i_startupmode!=0)?SERVICE_AUTO_START:SERVICE_DEMAND_START,// change service start type 
				SERVICE_NO_CHANGE, // error control: no change 
				NULL,              // binary path: no change 
				NULL,              // load order group: no change 
				NULL,              // tag ID: no change 
				NULL,              // dependencies: no change 
				NULL,              // account name: no change 
				NULL,              // password: no change 
				NULL)!=0 )            // display name: no change
				AddLogLine(false,GetResString(IDS_SVC_STRTUP_SUCCESS));
			else{ 
				GetSystemErrorString(GetLastError(),ErrString);
				AddLogLine(false,GetResString(IDS_SVC_STRTUP_FAIL),ErrString);
			}
			CloseServiceHandle(schService);
		}
		else {
			GetSystemErrorString(GetLastError(),ErrString);
			AddLogLine(false,GetResString(IDS_SVC_STRTUP_OPN_FAIL),ErrString);
		}
		CloseServiceHandle(schSCManager);
	}
	else {
		GetSystemErrorString(GetLastError(),ErrString);
		AddLogLine(false,GetResString(IDS_SVC_STRTUP_CTRL_FAIL),ErrString);
	}
	return 0; 
}

// ==> Adjustable NT Service Strings [Stulle] - Stulle
int NTServiceChangeDisplayStrings(CString strDisplayName, CString strServiceDescr)
{
	SC_HANDLE	schService;
	SC_HANDLE	schSCManager;
	CString ErrString;
	int iResult = 0;

	schSCManager = OpenSCManager(
		NULL,					// machine (NULL ==	local)
		NULL,					// database	(NULL == default)
		GENERIC_WRITE|GENERIC_READ// access required
		);

	if ( schSCManager )
	{   
		schService = OpenService(schSCManager, (!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():TEXT(SZSERVICENAME),GENERIC_WRITE|GENERIC_READ);
		if (schService)
		{
			// changed displayed name?
			if (strDisplayName.Compare(thePrefs.GetServiceDispName()) != 0)
			{
				if(strDisplayName.IsEmpty())
					strDisplayName.Format(_T(SZSERVICEDISPLAYNAME),MOD_VERSION);

				if (ChangeServiceConfig( 
					schService,        // handle of service 
					SERVICE_NO_CHANGE, // service type: no change 
					SERVICE_NO_CHANGE,// change service start type 
					SERVICE_NO_CHANGE, // error control: no change 
					NULL,              // binary path: no change 
					NULL,              // load order group: no change 
					NULL,              // tag ID: no change 
					NULL,              // dependencies: no change 
					NULL,              // account name: no change 
					NULL,              // password: no change 
					strDisplayName)!=0 )            // display name: no change
					AddLogLine(false,GetResString(IDS_SVC_DISP_SUCCESS));
				else
				{ 
					GetSystemErrorString(GetLastError(),ErrString);
					AddLogLine(false,GetResString(IDS_SVC_DISP_FAIL),ErrString);
					iResult = 1;
				}
			}

			// changed discription?
			if (strServiceDescr.Compare(thePrefs.GetServiceDescr()) != 0)
			{
				SERVICE_DESCRIPTION sdBuf;
				if(!strServiceDescr.IsEmpty()) 
					sdBuf.lpDescription = const_cast<LPWSTR>((LPCWSTR)strServiceDescr);
				else
					sdBuf.lpDescription = SZSERVICEDESCR;

				if( ChangeServiceConfig2( // I don't care if it works :P
					schService,                 // handle to service
					SERVICE_CONFIG_DESCRIPTION, // change: description
					&sdBuf)!=0 )
					AddLogLine(false,GetResString(IDS_SVC_DESCR_SUCCESS));
				else
				{ 
					GetSystemErrorString(GetLastError(),ErrString);
					AddLogLine(false,GetResString(IDS_SVC_DESCR_FAIL),ErrString);
					iResult = 1;
				}
			}

			CloseServiceHandle(schService);
		}
		else {
			GetSystemErrorString(GetLastError(),ErrString);
			AddLogLine(false,GetResString(IDS_SVC_STR_OPN_FAIL),ErrString);
			iResult = 1;
		}
		CloseServiceHandle(schSCManager);
	}
	else {
		GetSystemErrorString(GetLastError(),ErrString);
		AddLogLine(false,GetResString(IDS_SVC_STR_CTRL_FAIL),ErrString);
		iResult = 1;
	}
	return iResult; 
}
// <== Adjustable NT Service Strings [Stulle] - Stulle

/* for interactive windows, see ttp://support.microsoft.com/kb/164166 */

 BOOL CALLBACK EnumProc( HWND hWnd, LPARAM /*lParam */ )
{
	//check for property and unsubclass if necessary
	WNDPROC oldWndProc = (WNDPROC)::GetProp(hWnd, szAfxOldWndProc);
	if (oldWndProc!=NULL)
	{
		SetWindowLong(hWnd, GWL_WNDPROC, (DWORD)oldWndProc);
		RemoveProp(hWnd, szAfxOldWndProc);
	}

	return TRUE;
}

BOOL PassLinkToWebService(int iCommand,CString & StrData)
{
	HINTERNET mSession ;
	HINTERNET hHttpFile;
	char szSizeBuffer[500];
	DWORD dwBytesRead;
	BOOL bSuccessful=false;
	CString Url;
	CString ResultData;


	// Initialize the Win32 Internet functions
	mSession = ::InternetOpen(AfxGetAppName(),
		INTERNET_OPEN_TYPE_DIRECT, // Use registry settings.
		NULL, 
		NULL, 
		INTERNET_FLAG_NO_CACHE_WRITE ) ; // nocache

	if (mSession == NULL)
		AddLogLine(false,GetResString(IDS_SVC_LINK_PASS_FAIL));

	Url.Format(_T("http://%s:%d/?w=nologin&c=%s&commandData=%d"),_T("127.0.0.1"),
		                    thePrefs.GetWSPort(),
							StrData,iCommand);

	// Open the url.
	hHttpFile = InternetOpenUrl(mSession,  Url, NULL, 0, 0, 0);

	if (hHttpFile)
	{   
		{
			// And send the url:
			BOOL bRead = ::InternetReadFile(hHttpFile, szSizeBuffer, 10 /* all i need is status */, &dwBytesRead);

			if (bRead)
				bSuccessful = TRUE;

			::InternetCloseHandle(hHttpFile); // Close the connection.
		}
	}
	::InternetCloseHandle(mSession);
	return       bSuccessful ;
}

/* InterfaceToService()															*/ 
/* Depending on a setting either stops the running service (returning false) or */
/* run a webbroser settion to 127.0.0.1										    */

bool  InterfaceToService()
{
	if (thePrefs.GetServiceStartupMode() == 1){
		CString LocalWs;
		LocalWs.Format(_T("http://127.0.0.1:%d"),(int)thePrefs.GetWSPort());
		ShellExecute(NULL, NULL,LocalWs, NULL, thePrefs.GetMuleDirectory(EMULE_EXECUTEABLEDIR), SW_SHOWDEFAULT);
		return true; // no start of emule gui
	}
	if (thePrefs.GetServiceStartupMode() == 2){
		SC_HANDLE   schService;
		SC_HANDLE   schSCManager;
		// TODO: error logging, but where to log it? log facily might not have been started....
		schSCManager = OpenSCManager(
			NULL,                   // machine (NULL == local)
			NULL,                   // database (NULL == default)
			SC_MANAGER_ALL_ACCESS   // access required
			);
		if ( schSCManager )
		{
			// ==> Adjustable NT Service Strings [Stulle] - Stulle
			/*
			schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);
			*/
			schService = OpenService(schSCManager, (!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);
			// <== Adjustable NT Service Strings [Stulle] - Stulle

			if (schService)
			{
				// try to stop the service
				if ( ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus ) )
				{
					Sleep( 400 );
					while( QueryServiceStatus( schService, &ssStatus ) )
					{
						if ( ssStatus.dwCurrentState == SERVICE_STOP_PENDING )
						{
							Sleep( 400 );
						}
						else
							break;
					}
				}
				CloseServiceHandle(schService);
			}
			CloseServiceHandle(schSCManager);
		}
		return false; // lock mutex AGAIN, and start gui.... 
	}
	return false; // 
}


int NtServiceStart()
{
	SC_HANDLE   schService;
	SC_HANDLE   schSCManager;

	schSCManager = OpenSCManager(
			NULL,                   // machine (NULL == local)
			NULL,                   // database (NULL == default)
			SC_MANAGER_ALL_ACCESS   // access required --> start sevice, todo, maybe less rights required. 
			);
	if ( schSCManager )
	{
		// ==> Adjustable NT Service Strings [Stulle] - Stulle
		/*
		schService = OpenService(schSCManager, TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);
		*/
		schService = OpenService(schSCManager, (!thePrefs.GetServiceName().IsEmpty())?thePrefs.GetServiceName():TEXT(SZSERVICENAME), SERVICE_ALL_ACCESS);
		// <== Adjustable NT Service Strings [Stulle] - Stulle

		if (schService)
		{
			LPCWSTR args[1]= {_T("-AsAService")};
			// try to start  the service (fire and forget.., no error handling)
			StartService( schService,  1, args );
			CloseServiceHandle(schService);
		}
		CloseServiceHandle(schSCManager);
	}
	return 0;
}

// should be really in otherfunctions.cpp, but this merges simpler... 
BOOL Is_Terminal_Services() 
{
	BOOL    bResult = FALSE;
	DWORD   dwVersion;
/*
	OSVERSIONINFOEXA osVersion;
	DWORDLONG dwlCondition = 0;
	HMODULE hmodK32 = NULL;
	HMODULE hmodNtDll = NULL;
*/
	typedef ULONGLONG (WINAPI *PFnVerSetCondition) (ULONGLONG, ULONG, UCHAR);
	typedef BOOL (WINAPI *PFnVerifyVersionA) (POSVERSIONINFOEXA, DWORD, DWORDLONG);
//	PFnVerSetCondition pfnVerSetCondition;
//	PFnVerifyVersionA pfnVerifyVersionA;

	dwVersion = GetVersion();

	// Is Windows NT running?
	  if (!(dwVersion & 0x80000000)) 
	  {
		// Is it Windows 2000 or greater?
		if (LOBYTE(LOWORD(dwVersion)) > 4) 
		{
			return true;
			// On Windows 2000 and later, use the VerifyVersionInfo and 
			// VerSetConditionMask functions. Don't static link because 
			// it won't load on earlier systems.

			/* does not work for fast user swtiching = ts..... *

			hmodNtDll = GetModuleHandleA( "ntdll.dll" );
			if (hmodNtDll) 
			{	
				pfnVerSetCondition = (PFnVerSetCondition) GetProcAddress( 
				hmodNtDll, "VerSetConditionMask");
				if (pfnVerSetCondition != NULL) 
				{
					dwlCondition = (*pfnVerSetCondition) (dwlCondition, 
					VER_SUITENAME, VER_AND);

					// Get a VerifyVersionInfo pointer.

					hmodK32 = GetModuleHandleA( "KERNEL32.DLL" );
					if (hmodK32 != NULL) 
					{
						pfnVerifyVersionA = (PFnVerifyVersionA) GetProcAddress(
						hmodK32, "VerifyVersionInfoA") ;
						if (pfnVerifyVersionA != NULL) 
						{
							ZeroMemory(&osVersion, sizeof(osVersion));
							osVersion.dwOSVersionInfoSize = sizeof(osVersion);
							osVersion.wSuiteMask = VER_SUITE_TERMINAL;
							bResult = (*pfnVerifyVersionA) (&osVersion,
							VER_SUITENAME, dwlCondition);
						}
					}
				}
			}
			*/
		}
//		else  // This is Windows NT 4.0 or earlier.
//			bResult = false ;// false  ValidateProductSuite( "Terminal Server" );
	}
	return bResult;
}

static void SetSeviceMutex()
{
	CString strMutextName;
	if (Is_Terminal_Services())
		strMutextName.Format(_T("Global\\%s_SERVICE"), EMULE_GUID);
	else
		strMutextName.Format(_T("%s:%us_SERVICE"), EMULE_GUID); 
	// TODO? security? 
	s_hServiceMutex=CreateMutex(NULL, FALSE, strMutextName);
}


// check if there is a eMule is running as service. 
int IsServiceRunningMutexActive() 
{
	HANDLE CanOpen;
	CString strMutextName;
	if (Is_Terminal_Services())
		strMutextName.Format(_T("Global\\%s_SERVICE"), EMULE_GUID);
	else
		strMutextName.Format(_T("%s_SERVICE"), EMULE_GUID); 

	CanOpen=OpenMutex(READ_CONTROL,	false,strMutextName); 
	if  (CanOpen!=NULL) {
		CloseHandle(CanOpen);
		return 1;
	}
#ifdef DEBUG
	int LastError=::GetLastError();
#endif
	return 0; // we might just not have enough rights?
}




