//this file is part of eMule
// added by SLUGFILLER
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

// SLUGFILLER: SafeHash

#include "StdAfx.h"
#include "ReadWriteLock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CReadWriteLock::CReadWriteLock()
{
	m_nReadLocks = 0;
	m_nWriteLocks = 0;
	m_sState = 0;
	m_hAccessLock = CreateMutex(NULL, FALSE, NULL);
	m_hCanWrite = CreateMutex(NULL, FALSE, NULL);
	m_hCanRead = CreateEvent(NULL, TRUE, TRUE, NULL);
	m_other = NULL;
}

CReadWriteLock::CReadWriteLock(CReadWriteLock* other)
{
	m_nReadLocks = 0;
	m_nWriteLocks = 0;
	m_other = other;
}

CReadWriteLock::~CReadWriteLock()
{
	if (!m_other) {
		CloseHandle(m_hAccessLock);
		CloseHandle(m_hCanRead);
		CloseHandle(m_hCanWrite);
	}
	else {
		while (m_nReadLocks) {
			m_nReadLocks--;
			m_other->ReadUnlock();
		}
		while (m_nWriteLocks) {
			m_nWriteLocks--;
			m_other->WriteUnlock();
		}
	}
}

bool CReadWriteLock::ReadLock(DWORD dwMilliseconds)
{
	if (m_other) {
		if (m_other->ReadLock(dwMilliseconds)) {
			m_nReadLocks++;
			return true;
		}
		else
			return false;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nReadLocks++;
	if (m_sState == 2) {
		ReleaseMutex(m_hAccessLock);
		if (WaitForSingleObject(m_hCanRead, dwMilliseconds) == WAIT_TIMEOUT) {
			ReadUnlock();
			return false;
		}
	}
	else {
		if (m_sState == 0) {
			m_sState = 1;
			WaitForSingleObject(m_hCanWrite, 0);	// Just reset it, in case it's signalled
		}
		ReleaseMutex(m_hAccessLock);
	}
	return true;
}

void CReadWriteLock::ReadUnlock()
{
	if (m_other) {
		m_nReadLocks--;
		m_other->ReadUnlock();
		return;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nReadLocks--;
	if (!m_nReadLocks) {
		if (m_nWriteLocks) {
			m_sState = 2;
			ResetEvent(m_hCanRead);
		}
		else
			m_sState = 0;
		ReleaseMutex(m_hCanWrite);
	}
	ReleaseMutex(m_hAccessLock);
}

bool CReadWriteLock::WriteLock(DWORD dwMilliseconds)
{
	if (m_other) {
		if (m_other->WriteLock(dwMilliseconds)) {
			m_nWriteLocks++;
			return true;
		}
		else
			return false;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nWriteLocks++;
	if (m_sState == 1) {
		ReleaseMutex(m_hAccessLock);
		if (WaitForSingleObject(m_hCanWrite, dwMilliseconds) == WAIT_TIMEOUT) {
			WriteUnlock();
			return false;
		}
	}
	else {
		if (m_sState == 0) {
			m_sState = 2;
			ResetEvent(m_hCanRead);
		}
		ReleaseMutex(m_hAccessLock);
		// Now, wait for any other write threads to finish
		if (WaitForSingleObject(m_hCanWrite, dwMilliseconds) == WAIT_TIMEOUT) {
			WriteUnlock();
			return false;
		}
	}
	return true;
}

void CReadWriteLock::WriteUnlock()
{
	if (m_other) {
		m_nWriteLocks--;
		m_other->WriteUnlock();
		return;
	}
	WaitForSingleObject(m_hAccessLock, INFINITE);
	m_nWriteLocks--;
	ReleaseMutex(m_hCanWrite);	// Allow other write threads to run
	if (!m_nWriteLocks) {
		if (m_nReadLocks) {
			m_sState = 1;
			WaitForSingleObject(m_hCanWrite, 0);	// Just reset it, should always be signalled
		}
		else
			m_sState = 0;
		SetEvent(m_hCanRead);
	}
	ReleaseMutex(m_hAccessLock);
}
