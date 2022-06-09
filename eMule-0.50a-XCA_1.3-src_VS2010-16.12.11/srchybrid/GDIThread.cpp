//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "GDIThread.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


// critical section to protect while drawing to the DC
Poco::FastMutex CGDIThread::m_csGDILock;

// m_hAnotherDead is used to signal that one or more threads have ended
//	(it is an auto-reset event; and starts out not signaled)
//HANDLE CGDIThread::m_hAnotherDead = CreateEvent(NULL, FALSE, FALSE, NULL);

/////////////////////////////////////////////////////////////////////////////
// CGDIThread

CGDIThread::CGDIThread(HDC hDC)
{
	m_hDC = hDC;

	m_nDelay = 50;
	m_nScrollInc = SCROLL_UP;
//	m_bWaitVRT = FALSE;

	// kill event starts out in the signaled state
	m_hEventKill = CreateEvent(NULL, TRUE, FALSE, NULL);
}

// The reason we don't just get a CDC from our owner and simply use it is because 
// MFC GDI objects can't be passed between threads.  So we must instead pass a 
// handle and then reconvert them back to MFC objects.  The reason  for this is 
// because MFC maintains a list of all GDI objects on a per thread basis.  So if 
// you pass a GDI object to another thread, it won't be in the correct list 
// and MFC will assert.

void CGDIThread::run()
{
	// thread setup
	m_dc.Attach(m_hDC);

	// loop but check for kill notification
	while (WaitForSingleObject(m_hEventKill, 0) == WAIT_TIMEOUT)
		SingleStep();

	// thread cleanup
	m_dc.Detach();
}

CGDIThread::~CGDIThread()
{
	CloseHandle(m_hEventKill);
	//CloseHandle(m_hAnotherDead);
}

/////////////////////////////////////////////////////////////////////////////
// CGDIThread message handlers

int CGDIThread::SetDelay(int nDelay)
{
	int nTmp = m_nDelay;
	m_nDelay = nDelay;
	return nTmp;
}

int CGDIThread::SetScrollDirection(int nDirection)
{
	int nTmp = m_nScrollInc;
	m_nScrollInc = nDirection;
	return nTmp;
}
/*
BOOL CGDIThread::SetWaitVRT(BOOL bWait)
{
	BOOL bTmp = m_bWaitVRT;
	m_bWaitVRT = bWait;
	return bTmp;
}
*/