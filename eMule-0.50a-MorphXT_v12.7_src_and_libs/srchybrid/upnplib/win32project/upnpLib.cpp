// upnpLib.cpp : Defines the entry point for the DLL application.
//

#ifndef UPNP_STATIC_LIB
#include <windows.h>
BOOL APIENTRY DllMain( HANDLE hModule, 
                       DWORD  ul_reason_for_call, 
                       LPVOID lpReserved
					 )
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
	}
    return TRUE;
}
#endif
