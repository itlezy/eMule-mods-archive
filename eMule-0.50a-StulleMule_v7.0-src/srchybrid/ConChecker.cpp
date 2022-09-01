#include "StdAfx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "conchecker.h"
#include "otherfunctions.h"
#include "Log.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

const UINT UWM_CONCHECKER = ::RegisterWindowMessage(_T("UWM_CONCHECKER_{C44CF9E8-06B0-4ce4-A422-53DAE6802A1E}"));
//WPARAM 0 = started
//WPARAM 1 = stopped
//WRAPAM 2 = status		LPARAM (bool)connected
//WRAPAM 3 = new status LPARAM (bool)connected
namespace WombatAgent
{
	CConChecker::CConChecker()
	{
		m_hThread	= NULL;
		m_dwID		= 0;
		m_hEvent[cmd_step]		=::CreateEvent(NULL,FALSE,FALSE,_T("cmd_step"));
		m_hEvent[cmd_exit]		=::CreateEvent(NULL,FALSE,FALSE,_T("cmd_exit"));
		//
		m_hKilled				=::CreateEvent(NULL,FALSE,FALSE,_T("thread_iskilled"));
		m_bRun=false;
		m_dwIP	=0;
		m_strIP.Empty();
		m_dwLastCheck=0;

		m_dwState		= CONSTATE_NULL;

		m_bSettingsChanged=true;
		m_nCounter=0;

		m_bICMP			= false;
		m_nPingTimeout	= 1000;
		m_nPingTTL		= 10;
		m_nPingTime		= 0;
		//<<< For internal tests only
		m_nForceMode = CONFORCE_NULL;
		//>>> For internal tests only
	}

	CConChecker::~CConChecker(void)
	{
		m_bRun=false;
		if (IsActive())
		{
			::TerminateThread(m_hThread,0);
			CloseHandle(m_hThread);
		}
		for (int i=0;i<cmd_num;i++)
		{
			if (m_hEvent[i])
				CloseHandle(m_hEvent[i]);
		}
		if (m_hKilled)
			CloseHandle(m_hKilled);
	}

	void CConChecker::SetPreferences()
	{
		m_bICMP = thePrefs.GetICMP();
		m_nPingTimeout = thePrefs.GetPingTimeout()*1000;
		m_nPingTimeout = max(8, m_nPingTimeout);
		m_nPingTimeout = min(m_nPingTimeout, 64);
		m_nPingTTL = thePrefs.GetPingTTL();
		m_nPingTTL = max(8, m_nPingTTL);
		m_nPingTTL = min(m_nPingTTL, 64);
	}

	void CConChecker::SettingsChanged()
	{
		CSingleLock lock(&m_DataLock);
		lock.Lock();
		m_bSettingsChanged=true;
		lock.Unlock();
	}

	//<<< For internal tests only
	DWORD CConChecker::GetForceMode()
	{
		DWORD mode;
		CSingleLock lock(&m_ForceLock);
		lock.Lock();
		mode=m_nForceMode;
		lock.Unlock();
		return mode;
	}
	void CConChecker::SetForceMode(DWORD mode)
	{
		if (mode > 2)
			mode = 0;
		CSingleLock lock(&m_ForceLock);
		lock.Lock();
		if (m_nForceMode!=mode)
		{
			m_dwIP=0;
			m_strIP.Empty();
		}
		m_nForceMode = mode;
		lock.Unlock();
	}
	//>>> For internal tests only
	DWORD CConChecker::GetPingTime()
	{
		DWORD time;
		CSingleLock lock(&m_PingTimeLock);
		lock.Lock();
		time=m_nPingTime;
		lock.Unlock();
		return time;
	}
	void CConChecker::SetPingTime(DWORD time)
	{
		CSingleLock lock(&m_PingTimeLock);
		lock.Lock();
		m_nPingTime = time;
		lock.Unlock();
	}

	DWORD CConChecker::GetState()
	{
		CSingleLock lock(&m_StateLock);
		lock.Lock();
		DWORD state = m_dwState;
		lock.Unlock();
		return state;
	}
	BOOL CConChecker::PassivePing()
	{
		BOOL rc=false;
		//<<< For internal tests only
		DWORD mode=GetForceMode();
		if (mode==CONFORCE_OFFLINE)
			return false;
		if (mode==CONFORCE_ONLINE)
			return true;
		//>>> For internal tests only
		CPingReply pr;
		for (int i=0;i<4 && !rc;i++)
		{
//			if (theApp.WinSock2() && !m_bICMP) // this is always false...
//				rc = ping.Ping2(m_strIP, pr, m_nPingTTL, m_nPingTimeout); //Ping trough winsock2
//			else
				rc = ping.Ping1(m_strIP, pr, m_nPingTTL, m_nPingTimeout); //Ping trough ICMP
		}
		if (!rc)
		{
			m_dwIP=0;
			m_strIP.Empty();
			SetPingTime(0);
		}
		//else
		//	SetPingTime(pr.RTT);
		return rc;
	}
	bool CConChecker::ActivePing()
	{
		//<<< For internal tests only
		DWORD mode=GetForceMode();
		if (mode==CONFORCE_OFFLINE)
			return false;
		if (mode==CONFORCE_ONLINE)
			return true;
		//<<< For internal tests only

		//First Ping my favorite ISP to check if Internet-connection is available
		CSingleLock lock(&m_DataLock);
		lock.Lock();
		m_bSettingsChanged=false;
		lock.Unlock();
		if (200==FindWebAddress("",m_dwIP,&m_strIP))
		{
			return true;
		}
		else
		{
			m_dwIP=0;
			m_strIP.Empty();
			return false;
		}
		}

	BOOL CConChecker::Check(void)
	{
		BOOL rc=false;
		bool bBlocked=false;
		if (m_dwIP==0 || m_bSettingsChanged)
		{
			if (ActivePing())
			{
				rc=PassivePing();
				bBlocked=!rc;
			}
		}
		else
			rc=PassivePing();

		CSingleLock lock(&m_StateLock);
		lock.Lock();
		if (bBlocked)
			m_dwState=CONSTATE_BLOCKED;
		else if (rc)
			m_dwState=CONSTATE_ONLINE;
		else
			m_dwState=CONSTATE_OFFLINE;

		if (::IsWindow(theApp.emuledlg->GetSafeHwnd()))
			theApp.emuledlg->PostMessage(UWM_CONCHECKER,(WPARAM)2,(LPARAM)m_dwState);

		lock.Unlock();
		return rc;
	}
	DWORD CConChecker::InitialCheck(void)
	{
		BOOL rc=false;
		bool bBlocked=false;
		if (ActivePing())
		{
			rc=PassivePing();
			bBlocked=!rc;
		}

		CSingleLock lock(&m_StateLock);
		lock.Lock();
		if (bBlocked)
			m_dwState=CONSTATE_BLOCKED;
		else if (rc!=0)
			m_dwState=CONSTATE_ONLINE;
		else
			m_dwState=CONSTATE_OFFLINE;
		lock.Unlock();
		return m_dwState;
	}

	bool CConChecker::Start(void)
	{
		if (IsActive())
			return false;

		for (int i=0;i<cmd_num;i++)
		{
			if (m_hEvent[i])
				::ResetEvent(m_hEvent[i]);
		}
		m_bRun=false;
		m_hThread = ::CreateThread(NULL,
			NULL,
			(LPTHREAD_START_ROUTINE)ThreadProc,
			this,
			CREATE_SUSPENDED,
			&m_dwID);
		if (m_hThread==NULL)
			return false;
		::SetThreadPriority(m_hThread,THREAD_PRIORITY_NORMAL);
		::ResumeThread(m_hThread);
		m_bRun=true;
		return true;
	}
	bool CConChecker::Stop(bool bWait)
	{
		m_bRun=false;
		if (!IsActive())
			return true;
		// ...and signal kill event...
		::SetEvent(m_hEvent[cmd_exit]);

		if (!bWait)
			return true;

		if (::WaitForSingleObject(m_hKilled, 5000) != WAIT_OBJECT_0)
		{
			DWORD dwExitCode = 0;		
			if (!::GetExitCodeThread(m_hThread, &dwExitCode))
				return false;
			if (!::TerminateThread(m_hThread, dwExitCode))
				return false;
		}
		::CloseHandle(m_hThread);
		return true;
	}
	bool CConChecker::IsActive()
	{
		if (m_hThread==NULL)
			return false;
		DWORD dwExitCode = 0;		
		GetExitCodeThread(m_hThread,&dwExitCode);
		if (dwExitCode==STILL_ACTIVE)
			return true;
		return false;
	}
	void CConChecker::Process(DWORD dwTime)
	{
		if (!thePrefs.GetCheckCon())
			return;
		if (!IsActive() || !m_bRun)
			return;

		if (dwTime==0 || m_dwLastCheck==0 || (dwTime-m_dwLastCheck >= CC_INTERVAL))
		{
			::SetEvent(m_hEvent[cmd_step]);
			m_dwLastCheck=dwTime;
		}
	}

	UINT CConChecker::ThreadProc(LPVOID pProcParam)
	{

		CConChecker* pThis = reinterpret_cast<CConChecker*>(pProcParam);
		AddLogLine(false,_T("CConChecker (ID: 0x%x) started..."),::GetCurrentThreadId());
		pThis->Init();
		DWORD dwResult = 0;

		//Loop
#pragma warning(disable:4127) //condition is constant by IONIX-Team [cyrex2001]
		while (true)
#pragma warning(default:4127) //condition is constant by IONIX-Team [cyrex2001]
		{
			dwResult = ::WaitForMultipleObjects(cmd_num,pThis->m_hEvent,FALSE,500);// - WAIT_OBJECT_0;

			dwResult-=WAIT_OBJECT_0;

			if (dwResult == CConChecker::cmd_exit)
				break;

			if (dwResult == CConChecker::cmd_step)
				pThis->Check();
		}


		pThis->Exit();
		::SetEvent(pThis->m_hKilled);
		AddLogLine(false,_T("CConChecker (ID: 0x%x) stopped..."),::GetCurrentThreadId());
		return 0;
	}
	void CConChecker::Init(void)
	{
		//WPARAM 0 = started
		//WPARAM 1 = stopped
		//WRAPAM 2 = status		LPARAM dwState
		if (::IsWindow(theApp.emuledlg->GetSafeHwnd()))
			theApp.emuledlg->PostMessage(UWM_CONCHECKER,(WPARAM)0,(LPARAM)0);
	}
	void CConChecker::Exit(void)
	{
		//WPARAM 0 = started
		//WPARAM 1 = stopped
		//WRAPAM 2 = status		LPARAM dwState
		if (::IsWindow(theApp.emuledlg->GetSafeHwnd()))
			theApp.emuledlg->PostMessage(UWM_CONCHECKER,(WPARAM)1,(LPARAM)0);
	}
}
