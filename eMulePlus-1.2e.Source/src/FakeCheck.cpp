//	this file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

//	General idea based on eMule Morph's CFakecheck

#include "stdafx.h"
#include "FakeCheck.h"
#include "emule.h"
#include "otherfunctions.h"
#include "ED2KLink.h"
#include <wininet.h>

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////////////////////
CFakeCheck::CFakeCheck()
{
}

/////////////////////////////////////////////////////////////////////////////////////////////
CFakeCheck::~CFakeCheck()
{
	RemoveAllFakes();
}

/////////////////////////////////////////////////////////////////////////////////////////////
bool CFakeCheck::AddFake(const CString &strHash, const CString &strLength, const CString &strRealTitle)
{
	CString					strToken = strHash + strLength;
	_mapFakeMap::iterator	it = m_mapFakeMap.find(strToken);
	bool					bReturn = (it == m_mapFakeMap.end());

	if (bReturn)
		m_mapFakeMap[strToken] = strRealTitle;

	return bReturn;
}

/////////////////////////////////////////////////////////////////////////////////////////////
INT CFakeCheck::LoadFromDatFile()
{
	uint32			dwFakesCount = 0;

	EMULE_TRY

	RemoveAllFakes();

	CStdioFile		fileFakeList;

	if (fileFakeList.Open(g_App.m_pPrefs->GetConfigDir() + _T("fakes.dat"), CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
	{
		int				iPos1, iPos2;
		CString			strBuffer, strHash, strLength, strTitle;

		while (fileFakeList.ReadString(strBuffer))
		{
			if (strBuffer.GetAt(0) == _T('#') || strBuffer.GetAt(0) == _T('/') || strBuffer.GetLength() < 5)
				continue;

			if ( (iPos1 = strBuffer.Find(_T(','))) < 0
				|| (iPos2 = strBuffer.Find(_T(','), iPos1 + 1)) < 0 )
				continue;

			strHash = strBuffer.Left(iPos1).Trim().MakeUpper();
			strLength = strBuffer.Mid(iPos1 + 1, iPos2 - iPos1 - 1).Trim();
			strTitle = strBuffer.Mid(iPos2 + 1, strBuffer.GetLength() - iPos2 - 1).Trim();

			if (AddFake(strHash, strLength, strTitle))
				dwFakesCount++;
		}

		fileFakeList.Close();
		g_App.AddLogLine(LOG_FL_DBG, _T("Loaded %u fakes from fakes list"), dwFakesCount);
	}

	EMULE_CATCH

	return dwFakesCount;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CFakeCheck::RemoveAllFakes()
{
	m_mapFakeMap.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CFakeCheck::GetFakeComment(const CString &strFileHash, const uint64 qwFileLength, CString *pstrComment)
{
	if (m_mapFakeMap.empty())
	{
		pstrComment->Truncate(0);	// clear without reallocation
		return;
	}
	
	CString		strToken;

	strToken.Format(_T("%s%I64u"), strFileHash, qwFileLength);

	_mapFakeMap::iterator		it = m_mapFakeMap.find(strToken);

	if (it != m_mapFakeMap.end())
		*pstrComment = (*it).second;
	else
		pstrComment->Truncate(0);	// clear without reallocation
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFakeCheck::UpdateFakeList()
{
	EMULE_TRY

	uint32		dwVersion = 0;
	CString		strED2KLink;

	if (!RetrieveFakesDotTxt(dwVersion, strED2KLink))
	{
		g_App.AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_FAKE_LATEST_VERSION_ERROR);
		g_App.AddLogLine(LOG_FL_DBG, _T("Failed to download fakes.txt!"));
		return FALSE;
	}

	g_App.AddLogLine( LOG_FL_DBG, _T("Latest fake list version found: v%u, Your version: v%u"),
								dwVersion, g_App.m_pPrefs->GetFakesDatVersion() );

	if (g_App.m_pPrefs->GetFakesDatVersion() >= dwVersion)
	{
		g_App.AddLogLine(LOG_FL_SBAR, IDS_FAKE_GOT_LATEST_VERSION, g_App.m_pPrefs->GetFakesDatVersion());
		return FALSE;
	}

	bool		bUpdated = false;

	if (!strED2KLink.IsEmpty())
	{
		CED2KLink	   *pLink = CED2KLink::CreateLinkFromUrl(strED2KLink);

		if (pLink != NULL)
		{
			CED2KFileLink	   *pFileLink = pLink->GetFileLink();

			if (pFileLink != NULL)
			{
				if (!g_App.m_pDownloadQueue->FileExists(pFileLink->GetHashKey()))
				{
					g_App.m_pDownloadQueue->AddFileLinkToDownload(pFileLink);

					CPartFile	   *pFakesDotRarFile = g_App.m_pDownloadQueue->GetFileByID(pFileLink->GetHashKey());

					if (pFakesDotRarFile != NULL)
					{
						pFakesDotRarFile->SetFakesDotRar();
						pFakesDotRarFile->ResumeFile();
						g_App.m_pPrefs->SetDLingFakeListVersion(dwVersion);
						g_App.m_pPrefs->SetDLingFakeListLink(strED2KLink);
						bUpdated = true;
					}
					else
					{
						g_App.AddLogLine(LOG_RGB_ERROR, IDS_FAKE_CHECKUPERROR);
					}
				}
			}
		}

		delete pLink;
	}

	return bUpdated;

	EMULE_CATCH

	return FALSE;
}


/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFakeCheck::ExtractRARArchive(const CString &strArchivePath, CString strDestFolder)
{
	EMULE_TRY

	HINSTANCE	hUnRARDLL = LoadLibrary(_T("unrar.dll"));

	if (hUnRARDLL == NULL)
	{
		g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Can't find or load Unrar.dll"));
		return FALSE;
	}

	g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Unrar.dll loaded"));

	(FARPROC&)pfnRAROpenArchiveEx	= GetProcAddress(hUnRARDLL, "RAROpenArchiveEx");
	(FARPROC&)pfnRARCloseArchive	= GetProcAddress(hUnRARDLL, "RARCloseArchive");
	(FARPROC&)pfnRARReadHeader		= GetProcAddress(hUnRARDLL, "RARReadHeader");
#ifndef _UNICODE
	(FARPROC&)pfnRARProcessFile		= GetProcAddress(hUnRARDLL, "RARProcessFile");
#else
	(FARPROC&)pfnRARProcessFile		= GetProcAddress(hUnRARDLL, "RARProcessFileW");
#endif

	if ( (pfnRAROpenArchiveEx == NULL) || (pfnRARCloseArchive == NULL) ||
		 (pfnRARReadHeader    == NULL) || (pfnRARProcessFile  == NULL) )
	{
		FreeLibrary(hUnRARDLL);
		g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Unrar.dll unloaded"));
		return FALSE;
	}

	RAROpenArchiveDataEx	OpenArchiveData;

	memzero(&OpenArchiveData, sizeof(OpenArchiveData));
#ifndef _UNICODE
	OpenArchiveData.ArcName		= strArchivePath.GetString();
#else
	OpenArchiveData.ArcNameW	= strArchivePath.GetString();
#endif
	OpenArchiveData.CmtBuf		= NULL;
	OpenArchiveData.OpenMode	= RAR_OM_EXTRACT;

	HANDLE	hArcData = pfnRAROpenArchiveEx(&OpenArchiveData);

	if (OpenArchiveData.OpenResult != 0)
	{
		OutOpenArchiveError(OpenArchiveData.OpenResult, strArchivePath);
		FreeLibrary(hUnRARDLL);
		g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Unrar.dll unloaded"));
		return FALSE;
	}

	RARHeaderData	HeaderData;

	HeaderData.CmtBuf = NULL;

	int	iReadHeaderCode;

	while ((iReadHeaderCode = pfnRARReadHeader(hArcData, &HeaderData)) == 0)
	{
#ifndef _UNICODE
	//	The following line is important to properly support some characters
	//	so we decompress fakes.dat in the correct directory
		strDestFolder.AnsiToOem();
#endif

		int	iProcessFileCode = pfnRARProcessFile(hArcData, RAR_EXTRACT, const_cast<LPTSTR>(strDestFolder.GetString()), NULL);

		if (iProcessFileCode != 0)
		{
			OutProcessFileError(iProcessFileCode);
			break;
		}
	}

	pfnRARCloseArchive(hArcData);

	FreeLibrary(hUnRARDLL);
	g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Unrar.dll unloaded"));

	return (iReadHeaderCode != ERAR_BAD_DATA);

	EMULE_CATCH

	return FALSE;
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CFakeCheck::OutOpenArchiveError(int iError, const CString &strArchivePath)
{
	TCHAR	*pcError = NULL;

	switch (iError)
	{
		case ERAR_NO_MEMORY:
			pcError = _T(__FUNCTION__) _T(": Not enough memory to extract %s");
			break;

		case ERAR_EOPEN:
			pcError = _T(__FUNCTION__) _T(": Cannot open %s");
			break;

		case ERAR_BAD_ARCHIVE:
			pcError = _T(__FUNCTION__) _T(": %s is not RAR archive");
			break;

		case ERAR_BAD_DATA:
			pcError = _T(__FUNCTION__) _T(": %s: archive header broken");
			break;

		case ERAR_UNKNOWN:
			pcError = _T(__FUNCTION__) _T(": Unknown error when extracting %s");
			break;
	}

	if (pcError != NULL)
		g_App.AddLogLine(LOG_FL_DBG, pcError, strArchivePath);
}

/////////////////////////////////////////////////////////////////////////////////////////////
void CFakeCheck::OutProcessFileError(int iError)
{
	TCHAR	*pcError = NULL;

	switch (iError)
	{
		case ERAR_UNKNOWN_FORMAT:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: Unknown archive format");
			break;

		case ERAR_BAD_ARCHIVE:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: Bad volume");
			break;

		case ERAR_ECREATE:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: File create error");
			break;

		case ERAR_EOPEN:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: Volume open error");
			break;

		case ERAR_ECLOSE:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: File close error");
			break;

		case ERAR_EREAD:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: Read error");
			break;

		case ERAR_EWRITE:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: Write error");
			break;

		case ERAR_BAD_DATA:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: CRC error");
			break;

		case ERAR_UNKNOWN:
			pcError = _T(__FUNCTION__) _T(": fakes.rar: Unknown error");
			break;
	}

	if (pcError != NULL)
		g_App.AddLogLine(LOG_FL_DBG, pcError);
}

/////////////////////////////////////////////////////////////////////////////////////////////
BOOL CFakeCheck::RetrieveFakesDotTxt(OUT uint32 &dwVersion, OUT CString &strED2KLink)
{
	bool			bReturn = false;

	EMULE_TRY

	CString			strURL = g_App.m_pPrefs->GetFakeListURL();
	HINTERNET		hOpen = ::InternetOpen(HTTP_USERAGENT, INTERNET_OPEN_TYPE_PRECONFIG , NULL, NULL, 0);

	if (hOpen == NULL)
	{
		g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Error opening connection (error code: %u)"), GetLastError());
	}
	else
	{
		HINTERNET		hURL = ::InternetOpenUrl(hOpen, strURL, _T(""), NULL, INTERNET_FLAG_NO_CACHE_WRITE, NULL);

		if (hURL == NULL)
		{
			g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Error opening URL %s (error code: %u)"), strURL, GetLastError());
		}
		else
		{
			char	acBuf[256];
			DWORD	dwSize;

			if (!::InternetReadFile(hURL, acBuf, sizeof(acBuf), &dwSize))
				g_App.AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Error downloading fakes.txt (error code: %u)"), GetLastError());
			else
			{
				CString	strInput(acBuf, dwSize);
				int		iCurPos = 0;

				dwVersion = _tstoi(strInput.Tokenize(_T("\n"), iCurPos).Trim());
				strED2KLink = URLDecode(strInput.Tokenize(_T("\n"), iCurPos).Trim());

				bReturn = (dwVersion > 0);
			}
			::InternetCloseHandle(hURL);
		}
		::InternetCloseHandle(hOpen);
	}

	EMULE_CATCH

	return bReturn;
}


/////////////////////////////////////////////////////////////////////////////////////////////
void CFakeCheck::Init()
{
	EMULE_TRY

	LoadFromDatFile();

	if (g_App.m_pPrefs->IsUpdateFakeStartupEnabled())
		g_App.m_pFakeCheck->UpdateFakeList();

	if (!g_App.m_pPrefs->GetDLingFakeListLink().IsEmpty())
	{
		CED2KLink	   *pLink = CED2KLink::CreateLinkFromUrl(g_App.m_pPrefs->GetDLingFakeListLink());

		if (pLink != NULL)
		{
			CED2KFileLink	   *pFileLink = pLink->GetFileLink();

			if (pFileLink != NULL)
			{
				CPartFile	   *pFakesDotRarFile = g_App.m_pDownloadQueue->GetFileByID(pFileLink->GetHashKey());

				if (pFakesDotRarFile != NULL)
				{
					pFakesDotRarFile->SetFakesDotRar();
				}
			}
		}

		delete pLink;
	}

	EMULE_CATCH
}
/////////////////////////////////////////////////////////////////////////////////////////////
