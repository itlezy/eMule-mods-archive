/*
Copyright (C)2003 Barry Dunne (http://www.emule-project.net)
 
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the offical client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#include "stdafx.h"
#include "./ThreadName.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

namespace Kademlia
{
	void SetThreadName(DWORD dwThreadID, LPCSTR szThreadName, ...)
	{
#ifdef DEBUG
		if (dwThreadID == NULL)
			return;
		__try
		{
		    va_list args;
		    va_start(args, szThreadName);
		    int iLenBuf = 0;
		    char *pcharBuffer = NULL;
		    int iLenResult;
		    do
		    {
			    iLenBuf += 128;
			    delete[] pcharBuffer;
			    pcharBuffer = new char[iLenBuf];
			    // ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
#if   _MSC_VER  <= 1310 
			    iLenResult = _vsnprintf(pcharBuffer, iLenBuf, szThreadName, args);
#else
			    iLenResult = _vsnprintf_s(pcharBuffer, iLenBuf, iLenBuf, szThreadName, args);
#endif
			    // <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		    }
		    while (iLenResult == -1);
		    va_end(args);
		    THREADNAME_INFO info;
		    info.dwType = 0x1000;
		    info.szName = pcharBuffer;
		    info.dwThreadID = dwThreadID;
		    info.dwFlags = 0;
		    __try
		    {
		        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (DWORD *)&info);
		    } __except (EXCEPTION_CONTINUE_EXECUTION)
	    { }
	    delete[] pcharBuffer;
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
		{}
#else

		UNREFERENCED_PARAMETER(dwThreadID);
		UNREFERENCED_PARAMETER(szThreadName);
#endif

	}

	void SetThreadName(LPCSTR szThreadName, ...)
	{
#ifdef DEBUG
		__try
		{
		    va_list args;
		    va_start(args, szThreadName);
		    int iLenBuf = 0;
		    char *pcharBuffer = NULL;
		    int iLenResult;
		    do
		    {
			    iLenBuf += 128;
			    delete [] pcharBuffer;
			    pcharBuffer = new char[iLenBuf];
			    // ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
#if   _MSC_VER  <= 1310 
			    iLenResult = _vsnprintf(pcharBuffer, iLenBuf, szThreadName, args);
#else
			    iLenResult = _vsnprintf_s(pcharBuffer, iLenBuf, iLenBuf, szThreadName, args);
#endif
			    // <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		    }
		    while (iLenResult == -1);
		    va_end(args);
		    THREADNAME_INFO info;
		    info.dwType = 0x1000;
		    info.szName = pcharBuffer;
		    info.dwThreadID = (DWORD)-1;
		    info.dwFlags = 0;
		    __try
		    {
		        // ==> Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		        /*
		        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (DWORD *)&info);
		        */
		        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(DWORD), (ULONG_PTR*) &info);
		        // <== Make code VS 2005 and VS 2008 ready [MorphXT] - Stulle
		    } __except (EXCEPTION_CONTINUE_EXECUTION)
	    { }
	    delete [] pcharBuffer;
	}
	__except (EXCEPTION_CONTINUE_EXECUTION)
		{}
#else

		UNREFERENCED_PARAMETER(szThreadName);
#endif

	}
}
