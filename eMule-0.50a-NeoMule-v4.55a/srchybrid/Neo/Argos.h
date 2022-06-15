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

#pragma once

#include "MapKey.h"

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->

#define DFLT_DLP_FILENAME _T("dlp.dll")

struct TTestClient{
	uint32		IP;

	CString		strNick;
	CString		strMod;
	CString		strSoft;
	uchar		abyHash[16]; 
};

struct TArgosResult{
	TArgosResult();
	TArgosResult(CString Com, bool noGPL){
		GPLBreaker = noGPL;
		Comment = Com;
	}
	bool		GPLBreaker; // allow 0-score
	CString		Comment;
};

class CArgos: public CWinThread
{
public:
	CArgos();
	~CArgos();

	//void	Process();

	//void	ArgosCheckSoftware(CUpDownClient* Client);
	void	CheckForNickThief(CUpDownClient* Client);
	void	CheckForModThief(CUpDownClient* Client);

	void	CheckClient(CUpDownClient* Client);

	CString	GetAntiNickThiefNick();

	bool	LoadDLPlibrary();
	void	UnLoadDLPlibrary();

	CString GetDefaultFilePathDLP() const;
	
	void	EndThread();

private:
	
	// Scanner Engine
	static UINT RunProc(LPVOID pParam);
	UINT	RunInternal();

	// DLP
	TArgosResult* PerformDLPCheck(TTestClient* TestClient);

	void	CreateAntiNickThiefTag();

	// Test queue
    CTypedPtrList<CPtrList, TTestClient*> m_TestClientQueue;

	// Nick Tag
	CString m_sAntiNickThiefTag;

	// Thread handling
	CCriticalSection threadLocker;
	CCriticalSection QueueLocker;

	CEvent* threadEndedEvent;
	bool doRun;

	// DLP
	HINSTANCE m_dlpInstance;

	typedef uint32 (__cdecl *DLPGETVERSION)();
	typedef bool (__cdecl *DLPINITIALISE)(UINT preferences, void(*log)(int,CString));
	typedef UINT (__cdecl *DLPCHECKCLIENT)(CString strNick, CString strMod, CString strSoft, unsigned char abyHash[16], CStringW& result);
	DLPCHECKCLIENT m_dlpCheckClient;
};

#endif // ARGOS // NEO: NA END <-- Xanatos --