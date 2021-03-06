//
// Mutex.h
//
// $Id: //poco/1.3/Foundation/include/Poco/Mutex.h#2 $
//
// Library: Foundation
// Package: Threading
// Module:  Mutex
//
// Definition of the Mutex and FastMutex classes.
//
// Copyright (c) 2004-2008, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// Permission is hereby granted, free of charge, to any person or organization
// obtaining a copy of the software and accompanying documentation covered by
// this license (the "Software") to use, reproduce, display, distribute,
// execute, and transmit the Software, and to prepare derivative works of the
// Software, and to permit third-parties to whom the Software is furnished to
// do so, all subject to the following:
// 
// The copyright notices in the Software and this entire statement, including
// the above license grant, this restriction and the following disclaimer,
// must be included in all copies of the Software, in whole or in part, and
// all derivative works of the Software, unless such copies or derivative
// works are solely in the form of machine-executable object code generated by
// a source language processor.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
// SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
// FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
// DEALINGS IN THE SOFTWARE.
//


#ifndef Foundation_Mutex_INCLUDED
#define Foundation_Mutex_INCLUDED


#include "Poco/ScopedLock.h"


namespace Poco {


class Mutex
	/// A Mutex (mutual exclusion) is a synchronization 
	/// mechanism used to control access to a shared resource
	/// in a concurrent (multithreaded) scenario.
	/// Mutexes are recursive, that is, the same mutex can be 
	/// locked multiple times by the same thread (but, of course,
	/// not by other threads).
	/// Using the ScopedLock class is the preferred way to automatically
	/// lock and unlock a mutex.
{
public:
	typedef Poco::ScopedLock<Mutex> ScopedLock;
	typedef Poco::SingleLock<Mutex> SingleLock;
	
	Mutex();
		/// creates the Mutex.
		
	~Mutex();
		/// destroys the Mutex.

	bool Lock();
		/// Locks the mutex. Blocks if the mutex
		/// is held by another thread.
		
	bool Lock(long milliseconds);
		/// Locks the mutex. Blocks up to the given number of milliseconds
		/// if the mutex is held by another thread.
		/// Returns true if the mutex was successfully locked.
		///
		/// Performance Note: On most platforms (including Windows), this member function is 
		/// implemented using a loop calling (the equivalent of) tryLock() and Thread::sleep().
		/// On POSIX platforms that support pthread_mutex_timedlock(), this is used.

	bool tryLock();
		/// Tries to Lock the mutex. Returns false immediately
		/// if the mutex is already held by another thread.
		/// Returns true if the mutex was successfully locked.

	void Unlock();
		/// Unlocks the mutex so that it can be acquired by
		/// other threads.
	
private:
	Mutex(const Mutex&);
	Mutex& operator = (const Mutex&);
	CRITICAL_SECTION _cs;
};


typedef Mutex FastMutex;


//
// inlines
//
inline bool Mutex::Lock()
{
	try
	{
		EnterCriticalSection(&_cs);
		return true;
	}
	catch (...)
	{
		ATLTRACE("Poco::Event - cannot lock mutex");
		return false;
	}
}

inline bool Mutex::tryLock()
{
	try
	{
		return TryEnterCriticalSection(&_cs) != 0;
	}
	catch (...)
	{
		ATLTRACE("Poco::Event - cannot lock mutex");
		return false;
	}
}


inline void Mutex::Unlock()
{
	LeaveCriticalSection(&_cs);
}


} // namespace Poco


#endif // Foundation_Mutex_INCLUDED
