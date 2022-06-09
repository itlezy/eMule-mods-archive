// Tux: others: Windows XP compat. [start]
// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//
#pragma once

#define _CRT_SECURE_NO_WARNINGS

#ifndef WINVER
#define WINVER 0x0502 // 0x0502 == Windows Server 2003, Windows XP (same as VS2005-MFC)
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT WINVER // same as VS2005-MFC
#endif

#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410 // 0x0410 == Windows 98
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0560 // 0x0560 == Internet Explorer 5.6 -> Comctl32.dll v5.8
#endif
// Tux: others: Windows XP compat. [end]