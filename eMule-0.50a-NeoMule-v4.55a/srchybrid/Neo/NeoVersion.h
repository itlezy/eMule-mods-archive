//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#ifndef __NEO_VERSION_H__
#define __NEO_VERSION_H__

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _T
#define _T(x)	x
#endif

#define _chSTR(x)		_T(#x)
#define chSTR(x)		_chSTR(x)

// NEO: NV - [NeoVersion] -- Xanatos -->
// Specify the version of Neo mule with the following defines.
#define MOD_VERSION	MOD_NAME _T(" v") chSTR(MOD_VERSION_PRIMARY) _T(".") chSTR(MOD_VERSION_SECUNDARY)

#if defined(MODER) && (MODER == XANATOS)
 #define NEO_MODER		_T("David Xanatos")
#endif

#define MOD_ID			_T("Neo")
#define MOD_NAME		_T("Neo Mule")
#define MOD_TITLE		_T("Neo eMule")

//#define MOD_EXT			_T("")

#define MOD_VERSION_PRIMARY		4
#define MOD_VERSION_SECUNDARY	55
#define MOD_VERSION_TERTIARY	0
#define MOD_VERSION_UPDATE		1
#define MOD_VERSION_BUILD		172

#define _FINAL
// _ALFA	- Internal Alpha Release
// _BETA	- Beta Test Release
// _FINAL	- Final Release

#ifdef _BETA
 #define MOD_BETA_BUILD			0
 #define MOD_BETA_UPDATE		0
#endif

//////////////////////////
//  0=''   1='a'  2='b' //
//  3='c'  4='d'  5='e' //
//  6='f'  7='g'  8='h' //
//  9='i' 10='j' 11='k' //
// 12='l' 13='m' 14='n' //
// 15='o' 16='p' 17='q' //
// 18='r' 19='s' 29='t' //
// 21='u' 22='v' 23='w' //
// 24='x' 25='y' 26='z' //
//////////////////////////

// NEO: NV END <-- Xanatos --

#endif /* !__NEO_VERSION_H__ */