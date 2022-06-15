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

#include "stdafx.h"
#ifdef _DEBUG
#include "DebugHelpers.h"
#endif

#include <share.h>
#include <io.h>
#include "emule.h"
#include "otherfunctions.h"
#include "preferences.h"
#include "Neopreferences.h"
#include "updownclient.h"
#include "Functions.h"
#include "clientlist.h"
#include "clientcredits.h"
#include "uploadqueue.h"
#include "Version.h"
#include "NeoVersion.h"
#include "EmuleDlg.h"
#include "Log.h"
#include "StringConversion.h"
#include "OpCodes.h"
#include "SafeFile.h"
#include "Argos.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#ifdef ARGOS // NEO: NA - [NeoArgos] -- Xanatos -->

CArgos::CArgos(){
	CreateAntiNickThiefTag();

	m_dlpInstance = NULL;
	m_dlpCheckClient = NULL;

	threadEndedEvent = new CEvent(0, 1);
	doRun = true;

	if(NeoPrefs.UseDLPScanner())
		LoadDLPlibrary();

	AfxBeginThread(RunProc, (LPVOID)this, THREAD_PRIORITY_BELOW_NORMAL); // THREAD_PRIORITY_LOWEST
}

CArgos::~CArgos(){
	//EndThread();
	UnLoadDLPlibrary();
	delete threadEndedEvent;
}

//void CArgos::Process()
//{
//	
//}

void CArgos::CheckClient(CUpDownClient* Client)
{
	if(!doRun)
		return;

	TTestClient* TestClient = new TTestClient;

	TestClient->IP		= Client->GetConnectIP();

	TestClient->strNick	= CString(Client->GetUserName());
	TestClient->strMod	= CString(Client->GetClientModVer());
	TestClient->strSoft = CString(Client->GetClientSoftVer());
	if(Client->HasValidHash())
		md4cpy(TestClient->abyHash,Client->GetUserHash());

	QueueLocker.Lock();
    m_TestClientQueue.AddTail(TestClient);
	QueueLocker.Unlock();
}

/*void CArgos::ArgosCheckSoftware(CUpDownClient* Client)
{
	if (
		((Client->GetVersion()>589) && ((Client->GetSourceExchange1Version()>0) || (Client->GetExtendedRequestsVersion()>0)) && (Client->GetClientSoft()==SO_EDONKEY)) //Fake Donkeys
	 || ((Client->GetClientSoft() == SO_EDONKEYHYBRID) &&  ((Client->GetSourceExchange1Version()>0) || (Client->GetExtendedRequestsVersion()>0))) //Fake Hybrids
	 || (Client->SupportSecIdent()!=0 && (Client->GetClientSoft()==SO_EDONKEYHYBRID || Client->GetClientSoft()==SO_EDONKEY)) // this clients don't support sui
	 || (Client->GetVersion() > MAKE_CLIENT_VERSION(0, 30, 0) && Client->GetMuleVersion() > 0 && Client->GetMuleVersion()!=0x99 && Client->GetClientSoft()==SO_EMULE) //Fake emuleVersion
	 || (Client->GetVersion() == MAKE_CLIENT_VERSION(0,44,3) && Client->GetClientModVer().IsEmpty() && Client->GetUnicodeSupport()==utf8strNone && Client->GetClientSoft()==SO_EMULE)
	){
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Invalid Software Version"), Client->GetUserName(), ipstr(Client->GetConnectIP()));
		Client->Ban(_T("Invalid Software Version"));
	}
}*/

void CArgos::CheckForModThief(CUpDownClient* Client)
{
	CString tocomp = Client->GetClientModVer();
	if(tocomp.IsEmpty())
		return;

	CString OurMod = CString(MOD_VERSION); //cache it
	CString OurVersion = StrLine(_T("eMule v%u.%u%c"), VERSION_MJR, VERSION_MIN, _T('a') + VERSION_UPDATE);
	if (StrStrI(tocomp, OurMod) //uses our string
	 && (tocomp.GetLength() != OurMod.GetLength() //but not the correct length
	 || !StrStr(tocomp, OurMod) //but not in correct case		
	 || (Client->GetClientSoftVer() != OurVersion) //or wrong version
	 || Client->SupportsModProt() != 1 // Mod Prot Missing
	 )){
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Mod Thief"), Client->GetUserName(), ipstr(Client->GetConnectIP()));
		Client->Ban(_T("Mod Thief"));		
	 }
}

void CArgos::CheckForNickThief(CUpDownClient* Client)
{
	CString tocomp = Client->GetUserName();
	//is he mirroring our current tag?
	if(tocomp.Find(m_sAntiNickThiefTag) != -1){
		if (thePrefs.GetLogBannedClients())
			AddDebugLogLine(false, _T("Clients: %s (%s), Banreason: Nick Thief"), Client->GetUserName(), ipstr(Client->GetConnectIP()));
		Client->Ban(_T("Nick Thief"));		
	}
}

void CArgos::CreateAntiNickThiefTag()
{
	m_sAntiNickThiefTag.Empty();
	int maxchar = 4+rand()%4;
	for(int i = 0; i < maxchar; ++i)
		m_sAntiNickThiefTag.AppendFormat(_T("%c"), ((rand()%2) ? _T('A') : _T('a')) + rand()%25);
}

CString	CArgos::GetAntiNickThiefNick()
{
	return StrLine(NeoPrefs.IsNickThiefDetection() == 1 ? _T("%s [%s]") : _T("%s %s"), thePrefs.GetUserNick(), m_sAntiNickThiefTag);
}

/////////////////////////////////////////////////////////////////////////////////////////
// Scanner Engine:

UINT AFX_CDECL CArgos::RunProc(LPVOID pParam)
{
	DbgSetThreadName("Neo Argos Engine");
	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe] -- Xanatos -->
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END <-- Xanatos --
    CArgos* cargos = (CArgos*)pParam;

	return cargos->RunInternal();
}

UINT CArgos::RunInternal()
{
	CTypedPtrList<CPtrList, TTestClient*> TestClientQueue;

	while(doRun){
        QueueLocker.Lock();
        while(!m_TestClientQueue.IsEmpty()) {
            TTestClient* TestClient = m_TestClientQueue.RemoveHead();
            TestClientQueue.AddTail(TestClient);
        }
        QueueLocker.Unlock();

		if(TestClientQueue.IsEmpty())
			Sleep(100);
		
		threadLocker.Lock();

		TTestClient* TestClient;
		while(!TestClientQueue.IsEmpty())
		{
			TestClient = TestClientQueue.RemoveHead();
		
			if(TArgosResult* Result = PerformDLPCheck(TestClient))
				VERIFY( PostMessage(theApp.emuledlg->m_hWnd, TM_ARGOS_RESULT, (WPARAM)Result, (LPARAM)TestClient->IP) );

			delete TestClient;
		}

		threadLocker.Unlock();
    }

    QueueLocker.Lock();
	while(!m_TestClientQueue.IsEmpty())
        delete m_TestClientQueue.RemoveHead();
    QueueLocker.Unlock();

	threadEndedEvent->SetEvent();

	return FALSE;
}

void CArgos::EndThread()
{
	threadLocker.Lock();
	doRun = false;
	threadLocker.Unlock();

	threadEndedEvent->Lock();
}

////////////////////////////////////////////////////////////////////////////////////////
// DLP - Dynamic leecher protection
TArgosResult* CArgos::PerformDLPCheck(TTestClient* TestClient)
{
	if(m_dlpCheckClient)
	{
		try
		{
			CString Comment;
			if(UINT Found = m_dlpCheckClient(TestClient->strNick, TestClient->strMod, TestClient->strSoft, TestClient->abyHash, Comment))
				return new TArgosResult(Comment, Found == 2);
		}
		catch(...){
			ASSERT(0);
			DebugLogError(_T("ARGOS-DLP: Unknown exception while performing dlp check, dlp.dll may be corrupted..."));
		}
	}
	return NULL;
}

void DLPLoger(int uType,CStringW strMessage)
{
	switch(uType)
	{
	case  1: 
		ModLog(LOG_SUCCESS,GetResString(IDS_X_DLP_LOG),strMessage); break;
	case -1: 
		ModLogError(GetResString(IDS_X_DLP_LOG),strMessage); break;
	case -2: 
		ModLogWarning(GetResString(IDS_X_DLP_LOG),strMessage);	break;
	case  0: 
	default:
		ModLog(GetResString(IDS_X_DLP_LOG),strMessage);
	}
}

bool CArgos::LoadDLPlibrary()
{
	if (!PathFileExists(GetDefaultFilePathDLP())){
		ModLog(LOG_WARNING, GetResString(IDS_X_ARGOS_DLP_FILE_MISSING));
		return false;
	}

	threadLocker.Lock();

	uint8 uError = 0;
	
	if(m_dlpInstance)
		UnLoadDLPlibrary();

	try
	{
		m_dlpInstance = ::LoadLibrary(GetDefaultFilePathDLP());
		if(m_dlpInstance)
		{
			DLPGETVERSION dlpGetVersion = (DLPGETVERSION)GetProcAddress(m_dlpInstance,("dlpGetVersion"));
			if(dlpGetVersion && dlpGetVersion() >= 2) // 1 is not compatible anymore
			{
				DLPINITIALISE dlpInitialise	= (DLPINITIALISE)GetProcAddress(m_dlpInstance,("dlpInitialise"));
				if(dlpInitialise)
				{
					const UINT uDetectionLevel	= NeoPrefs.GetDetectionLevel();
					const UINT uModDetection	= NeoPrefs.IsLeecherModDetection();
					const UINT uNickDetection	= NeoPrefs.IsLeecherNickDetection();
					const UINT uHashDetection	= NeoPrefs.IsLeecherHashDetection();
					UINT uPreferences = 
					//	((u					& 0xff)	<<  24) |
					//	((u					& 0x03)	<<  22) |
					//	((u					& 0x03)	<<  20) |
					//	((u					& 0x03)	<<  18) |
					//	((u					& 0x03)	<<  16) |
					//	((u					& 0x03)	<<  14) |
						((uHashDetection	& 0x03)	<<  12) |
						((uNickDetection	& 0x03)	<<  10) |
						((uModDetection		& 0x03)	<<  8 ) |
					//	((u					& 0x0f)	<<  4 ) |
						((uDetectionLevel	& 0x0f)	<<  0 ) ;

					if(dlpInitialise(uPreferences, &DLPLoger))
					{
						m_dlpCheckClient = (DLPCHECKCLIENT)GetProcAddress(m_dlpInstance,("dlpCheckClient"));
						if(m_dlpCheckClient)
						{
							ModLog(GetResString(IDS_X_ARGOS_DLP_LOADED));
							threadLocker.Unlock();
							return true;
						}
						else
							uError = 1;
					}
					else
						uError = 3;
				}
				else
					uError = 1;
			}
			else
				uError = 1;
		}
		else
			uError = 1;
		
		if(m_dlpInstance)
			::FreeLibrary(m_dlpInstance);
		m_dlpInstance = NULL;
		m_dlpCheckClient = NULL;
	}
	catch(...){
		ASSERT(0);
		uError = 2;
		if(m_dlpInstance){
			HINSTANCE dlpInstance = m_dlpInstance;
			m_dlpInstance = NULL;
			::FreeLibrary(dlpInstance);
		}
		m_dlpCheckClient = NULL;
	}

	ModLog(LOG_ERROR,GetResString(IDS_X_ARGOS_DLP_LOAD_FAILED),uError == 2 ? GetResString(IDS_X_FATAL) : uError == 3 ? GetResString(IDS_X_INTERNAL) : GetResString(IDS_X_GENERIC));

	threadLocker.Unlock();
	return false;
}

void CArgos::UnLoadDLPlibrary()
{
	if(m_dlpInstance){
		::FreeLibrary(m_dlpInstance);
		ModLog(GetResString(IDS_X_ARGOS_DLP_UNLOADED));
	}

	m_dlpInstance = NULL;
	m_dlpCheckClient = NULL;
}

CString CArgos::GetDefaultFilePathDLP() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) +  DFLT_DLP_FILENAME;
}
#endif // ARGOS // NEO: NA END <-- Xanatos --