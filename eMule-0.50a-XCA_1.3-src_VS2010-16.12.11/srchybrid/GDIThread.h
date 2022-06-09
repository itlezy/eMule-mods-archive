#pragma once

// GDIThread.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CGDIThread thread

class CGDIThread : public Poco::Thread
{
public:
	CGDIThread(HDC hDC);

// Attributes
public:
	HDC m_hDC;
	CDC m_dc;
	HANDLE m_hEventKill;
	//static HANDLE m_hAnotherDead;

	// options
	int m_nDelay;
	int m_nScrollInc;
//	BOOL m_bWaitVRT;

	enum {SCROLL_UP = 1, SCROLL_PAUSE = 0, SCROLL_DOWN = -1};

	static Poco::FastMutex m_csGDILock;

// Operations
public:
	virtual void SingleStep() = 0;

// Implementation
public:
//	BOOL SetWaitVRT(BOOL bWait = TRUE);
	int SetScrollDirection(int nDirection);
	int SetDelay(int nDelay);
	virtual ~CGDIThread();
	virtual void run();
};
