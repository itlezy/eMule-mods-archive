
#if !defined(_NTSERVICE123_INCLUDED)
#define _NTSERVICE123_INCLUDED

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "resource.h"



// internal name of the service
#define SZSERVICENAME        "eMule"
// displayed name of the service
#define SZSERVICEDISPLAYNAME "eMule %s as a service"
// displayed descrption for the service
#define SZSERVICEDESCR		 L"Provides capability to let eMule run as a NT system service."
// list of service dependencies - "dep1\0dep2\0\0"
#define SZDEPENDENCIE     "Tcpip\0"
//////////////////////////////////////////////////////////////////////////////

extern bool RunningAsServiceStat;
bool RunningAsService();
void OnStartAsService();
int	NTServiceGet(int  &b_installed,	int	&i_startupmode,	int	&i_enoughrights);
int NTServiceSetStartupMode(int i_startupmode);
int NTServiceChangeDisplayStrings(CString strDisplayName, CString strServiceDescr);

int CmdInstallService(bool b_autostart=false);
int CmdRemoveService();
VOID ServiceStop();
extern bool NtserviceStartwhenclose;

void WINAPI ServiceMain();
VOID WINAPI service_ctrl(DWORD dwCtrlCode);
void ServiceStartedSuccesfully();  // when connected
void ServiceStartedPaused(); // when started but not connected. 
BOOL CALLBACK EnumProc( HWND hWnd, LPARAM lParam) ;    // for MFC as a service workarround
BOOL PassLinkToWebService(int iCommand,CString & StrData);
bool InterfaceToService();
int NtServiceStart();
BOOL ReportStatusToSCMgr(DWORD dwCurrentState,
                         DWORD dwWin32ExitCode,
                         DWORD dwWaitHint);
BOOL Is_Terminal_Services () ;
int IsServiceRunningMutexActive() ;

#endif // !defined(_NTSERVICE123_INCLUDED)
