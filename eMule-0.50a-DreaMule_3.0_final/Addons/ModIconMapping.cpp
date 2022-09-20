//////////////////////////////////////////////////////////////////////////
//ModIconDLL
// 
//Copyright (C)2006 WiZaRd ( thewizardofdos@gmail.com / http://kademlia-net.de )
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
//
//////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "ModIconMapping.h"
#include "emule.h"
#include "emuleDlg.h"
#include "Preferences.h"
#include "Log.h"
#include "updownclient.h"
#include "ClientList.h"
#include "HttpDownloadDlg.h"
//>>> WiZaRd::ModIconDLL Update
#include "./Addons/extractfile.h"
//<<< WiZaRd::ModIconDLL Update

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool	DownloadFromURLToFile(const CString& strURL, const CString& strFile)
{
	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle.Format(_T("Downloading %s from %s..."), GetResString(IDS_FILE), strURL);
	dlgDownload.m_sURLToDownload = strURL;
	dlgDownload.m_sFileToDownloadInto = strFile;
	if (dlgDownload.DoModal() != IDOK)
	{
		_tremove(strFile);
		LogError(NULL, _T("Download from %s failed!"), strURL);
		return false;
	}
	return true;
}

CModIconMapper::CModIconMapper()
{
	m_bDLLAvailable = false;
	m_hDLLInstance = NULL;
	DLLGetVersion = NULL;
	DLLFillModIconList = NULL;
	DLLGetIconIndexForModstring = NULL;
	Reload();
}

CModIconMapper::~CModIconMapper()
{
	Unload();	
}

void CModIconMapper::Update()
{
	bool bReloaded = false;
	if(DownloadFromURLToFile(MODICONDLL_VER_URL, VER_TXT))
	{			
		CStdioFile versionFile;
		if(versionFile.Open(VER_TXT, CFile::modeRead | CFile::shareDenyWrite))
		{
			CString strVersion = _T("");	
			versionFile.ReadString(strVersion);					
			strVersion.Trim();
			versionFile.Close();

			if((DWORD)_tstoi(strVersion)  > GetVersion()
				&& DownloadFromURLToFile(MODICONDLL_URL, NEW_DLL))
			{
//>>> WiZaRd::ModIconDLL Update
				if(Extract(NEW_DLL, NEW_DLL, DFLT_MODICONDLL_FILENAME, DFLT_MODICONDLL_FILENAME, true))
//<<< WiZaRd::ModIconDLL Update
				{
					Reload();
					bReloaded = true;
				}
			}
		}
		_tremove(VER_TXT);
	}
	if(!bReloaded)
		Reload(); //maybe the user put up a new version - check that...
}

void CModIconMapper::Unload()
{	
	modimagelist.DeleteImageList();
	if(DLLFillModIconList)
	{
		if(theApp.emuledlg->IsRunning())
			theApp.QueueLogLineEx(LOG_WARNING, _T("Unloading all iconmappings from .dll"));		
		DLLFillModIconList(NULL);
		DLLFillModIconList = NULL;
	}
	DLLGetVersion = NULL;
	DLLGetIconIndexForModstring = NULL;
	if(m_hDLLInstance != NULL)
	{
		::FreeLibrary(m_hDLLInstance);
		m_hDLLInstance = NULL;
	}	
	m_bDLLAvailable = false;
}

void CModIconMapper::Load(const CString& strDLLToLoad)
{
	if(m_hDLLInstance == NULL)
	{
		m_hDLLInstance = ::LoadLibrary(strDLLToLoad);
		if(m_hDLLInstance)
		{
			DLLGetVersion = (GETDLLVERSION)GetProcAddress(m_hDLLInstance, "GetDLLVersion");
			DLLFillModIconList = (DLLFILLMODICONLIST)GetProcAddress(m_hDLLInstance, "FillModIconList");
			DLLGetIconIndexForModstring = (DLLGETICONINDEXFORMODSTRING)GetProcAddress(m_hDLLInstance, "GetIconIndexForModstring");

			if(DLLGetVersion 
				&& DLLFillModIconList 
				&& DLLGetIconIndexForModstring 
				)
			{
				m_bDLLAvailable = true;

				theApp.QueueLogLineEx(LOG_SUCCESS, _T("*** ModIconDLL.dll %s loaded"), GetVersionString());

				FillModIconList();
			}
			else
			{
				theApp.QueueLogLineEx(LOG_ERROR, _T("*** failed to initialize ModIconDLL.dll"));
				Unload();
			}
		}
		else
			theApp.QueueLogLineEx(LOG_ERROR, _T("*** failed to load ModIconDLL.dll (ERROR: %s)"), GetErrorMessage(GetLastError()));
	}
	else
	{
		theApp.QueueLogLineEx(LOG_WARNING, _T("*** no new ModIconDLL.dll found"));
		m_bDLLAvailable = true;
	}
}

void CModIconMapper::Reload()
{
	m_bDLLAvailable = false;
	bool bErrorOccured = false;

	const DWORD old_ver = GetVersion();

	if(PathFileExists(NEW_DLL))
	{
		theApp.QueueLogLineEx(LOG_WARNING, _T("*** found new version of ModIconDLL.dll"));
		Unload(); //new version exists, try to unload the old and load the new one
		if(PathFileExists(CUR_DLL))
		{
			//backup current dll
			if(_tremove(OLD_DLL) != 0 && errno != ENOENT)
				bErrorOccured = true;
			if(!bErrorOccured && _trename(CUR_DLL, OLD_DLL) != 0)
				bErrorOccured = true;
		}

		//rename the new one into currentdll
		if(!bErrorOccured && _trename(NEW_DLL, CUR_DLL) != 0)
			bErrorOccured = true;

		if(bErrorOccured)
			theApp.QueueLogLineEx(LOG_ERROR, _T("*** error during copying ModIconDLL.dll, trying to load the old one"));
	}
	
	Load(CUR_DLL);

	//fall back on strange events...
	const DWORD new_ver = GetVersion();
	if(new_ver < old_ver)		
	{
		bErrorOccured = false;
		theApp.QueueLogLineEx(LOG_WARNING, _T("*** new version seems to be older - loading newer one"));
		Unload(); //new version exists, try to unload the old and load the new one

		//currentdll == newdll - just delete it :)
		if(_tremove(OLD_DLL) != 0 && errno != ENOENT)
			bErrorOccured = true;

		//rename the old one into currentdll
		if(!bErrorOccured && _trename(OLD_DLL, CUR_DLL) != 0)
			bErrorOccured = true;

		if(bErrorOccured)
			theApp.QueueLogLineEx(LOG_ERROR, _T("*** error during copying ModIconDLL.dll, trying to load the old one"));

		Load(CUR_DLL);
	}
}

DWORD CModIconMapper::GetVersion() const
{
	return m_bDLLAvailable ? DLLGetVersion() : 0;
}

CString CModIconMapper::GetVersionString() const
{
	UINT version = GetVersion();
	const UINT major = version / 1000;
	version -= major*1000;

	const UINT minor = version / 100;
	version -= minor*100;

	const UINT sub = version / 10;
	version -= sub*10;

	const UINT build = version /*/ 1*/;
	//version -= build;

	CString ret;
	ret.Format(L"v%u.%u.%u.%u", major, minor, sub, build);
	return ret;
}

void	CModIconMapper::FillModIconList()
{	
	modimagelist.DeleteImageList(); //just to be sure
	modimagelist.Create(16, 16, theApp.m_iDfltImageListColorFlags|ILC_MASK, 0, 1);
	modimagelist.SetBkColor(CLR_NONE);
	if(m_bDLLAvailable && DLLFillModIconList)
	{
		int iLoaded = DLLFillModIconList(&modimagelist);
		theApp.QueueLogLineEx(LOG_WARNING, _T("Loaded %i iconmappings from .dll"), iLoaded);
	}
	else //we have to keep the "badguy" icon local
		modimagelist.Add(CTempIconLoader(L"BADGUY"));
	theApp.clientlist->UpdateModIconIndexes();
}

int		CModIconMapper::GetIconIndexForModstring(const CString& strMod)
{
	if(m_bDLLAvailable && DLLGetIconIndexForModstring)
		return DLLGetIconIndexForModstring(strMod);
	return MODMAP_NONE;
}

BOOL	CModIconMapper::DrawModIcon(CDC* pDC, const int& nImage, POINT pt, const UINT& nStyle)
{
	if(!m_bDLLAvailable 
		|| nImage > modimagelist.GetImageCount())
		return FALSE;

	return modimagelist.Draw(pDC, nImage, pt, nStyle);
}