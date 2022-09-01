#pragma once
#include "Ping.h"
#include "Preferences.h"

#define CONSTATE_NULL			0
#define CONSTATE_ONLINE			1
#define CONSTATE_OFFLINE		2
#define CONSTATE_BLOCKED		3

#define CONFORCE_NULL			0
#define CONFORCE_OFFLINE		1
#define CONFORCE_ONLINE			2

namespace WombatAgent
{
#define CC_INTERVAL		1000 //1 Seconds
	class CConChecker
	{
		enum commands {cmd_step	=0,cmd_exit,cmd_num};
	public:
		CConChecker();
		~CConChecker(void);
		void SetPreferences();
	private:
		static UINT ThreadProc(LPVOID pParam);
	protected:
		void	Exit(void);
		void	Init(void);
		bool	m_bRun;

		DWORD	m_dwID;
		DWORD	m_dwState;


		HANDLE	m_hThread;
		HANDLE	m_hEvent[cmd_num];
		HANDLE  m_hKilled;

		CPing	ping;
		CMutex	m_StateLock;

	public:
		bool Start(void);
		bool Stop(bool bWait=false);
		bool IsActive();
		void Process(DWORD dwTime);
		DWORD GetState();	

		//
		//Pinger components
		//
	public:
//		bool	SpookyAvailable() {return (bool)(m_dwIP!=0 && thePrefs.GetCheckCon());}
		DWORD	GetIP()			  {return m_dwIP;}
		CString GetIPString()	  {return m_strIP;}
		DWORD	GetID()			  {return m_dwIP;}
		BOOL	Check();
		DWORD	InitialCheck();
		void	SettingsChanged();
	protected:
		bool			ActivePing();
		BOOL			PassivePing();
		bool			m_bSettingsChanged;
		bool			m_bICMP;

		DWORD			m_nCounter;
		DWORD			m_dwIP;
		DWORD			m_dwLastCheck;
		uint16			m_nPingTimeout;
		uint8			m_nPingTTL;
		CString			m_strIP;

		CMutex			m_DataLock;

		//<<< For internal tests only
	public:
		void SetForceMode(DWORD mode);
		DWORD GetForceMode();
		DWORD  GetPingTime();
		void SetPingTime(DWORD time);
	protected:
		DWORD	m_nForceMode;
		CMutex	m_ForceLock;
		DWORD	m_nPingTime;
		CMutex	m_PingTimeLock;
		//>>> For internal tests only
	};

	//uint16 GetDownloadLevel(uint16 upload);
}
extern const UINT UWM_CONCHECKER;