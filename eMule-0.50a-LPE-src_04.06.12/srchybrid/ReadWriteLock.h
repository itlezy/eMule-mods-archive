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

#pragma once

class CReadWriteLock
{
public:
	CReadWriteLock();
	CReadWriteLock(CReadWriteLock* other);
	~CReadWriteLock();
	bool ReadLock(DWORD dwMilliseconds = INFINITE);
	void ReadUnlock();
	bool WriteLock(DWORD dwMilliseconds = INFINITE);
	void WriteUnlock();
private: // netfinity: Rearranged for alignment reasons
	//HANDLE m_hAccessLock;
	CRITICAL_SECTION m_hAccessLock;
	HANDLE m_hCanRead;
	HANDLE m_hCanWrite;
	CReadWriteLock* m_other;
	sint_ptr m_nReadLocks;
	sint_ptr m_nWriteLocks;
	sint_ptr m_sState;
};
