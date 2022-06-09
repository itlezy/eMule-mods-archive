//	This file is part of eMule Plus
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
/*
Sample configuration file
(autodl.ini in Config/ directory)
-----------------------------------
[General]
Count=1
[DL#1]
URL=http://yourdomain.com/file.xml
Interval=60
-----------------------------------

Sample file.xml at yourdomain.com
-----------------------------------
<autodl min_interval="1440">
	<item link="ed2k://|file|Filename1.avi|123|786AFC89F4748C83DF3488D246C337E7|/" category="Movies" />
	<item link="ed2k://|file|Filename2.avi|456|886AFC89F4748C83DF3488D246C337E7|/" category="Animation" />
</autodl>
-----------------------------------
*/

#include "stdafx.h"
#include "emule.h"
#include "ini2.h"
#include "Engine/Other/XMLhelper.h"
#include "AutoDL.h"
#include "ED2KLink.h"
#include "SharedFileList.h"
#include <wininet.h>

CAutoDL::CAutoDL()
	:m_hWorkerThread(NULL)
	,m_hFinish(NULL)
	,m_bUseIt(false)
{
	LoadPrefs();
	Start();
}

CAutoDL::~CAutoDL()
{
	SavePrefs();
	Finish();
}

void CAutoDL::GetConfigFilename(CString *pstrOut)
{
	EMULE_TRY
	pstrOut->Format(_T("%sautodl.ini"), g_App.m_pPrefs->GetConfigDir());
	return;
	EMULE_CATCH
	*pstrOut = _T("");
}

bool CAutoDL::LoadPrefs()
{
	EMULE_TRY
	// Can't load data in the middle of work
	if(m_hWorkerThread != NULL)
		return false;

	CString	strTmp;
	GetConfigFilename(&strTmp);

	// Check if file exists
	if(!PathFileExists(strTmp))
		return false;

	// Load data
	CIni dlini(strTmp, INI_MODE_ANSIONLY | INI_MODE_READONLY);
	dlini.SetDefaultCategory(_T("General"));
	m_bUseIt = dlini.GetBool(_T("UseIt"), false);

	int iCount = dlini.GetInt(_T("Count"), 0);

	for(int i = 1; i <= iCount; i++)
	{
		strTmp.Format(_T("DL#%u"), i);
		dlini.SetDefaultCategory(strTmp);

		CAutoDLData data;
 		_tcsncpy(data.acUrl, dlini.GetString(_T("URL"), _T("")), DLURLMAX);
		data.acUrl[DLURLMAX - 1] = _T('\0');
		data.ulInterval = max(dlini.GetUInt32(_T("Interval"), 0), 30);	// 30 minutes minimum
		data.lLastCheck = 0;

		m_UrlList.Add(data);
	}
	return true;
	EMULE_CATCH
	return false;
}

bool CAutoDL::SavePrefs()
{
	EMULE_TRY

	CString	strTmp;
	GetConfigFilename(&strTmp);

	// If we don't have anything to save, just delete the file
	DeleteFile(strTmp);
	if(m_UrlList.GetSize() == 0)
		return false;

	// Save data
	CIni dlini(strTmp, INI_MODE_ANSIONLY);
	dlini.SetDefaultCategory(_T("General"));
	dlini.SetBool(_T("UseIt"), m_bUseIt);
	dlini.SetInt(_T("Count"), m_UrlList.GetSize());
	for(int i = 0; i < m_UrlList.GetSize(); i++)
	{
		strTmp.Format(_T("DL#%u"), i + 1);
		dlini.SetDefaultCategory(strTmp);
		dlini.SetString(_T("URL"), m_UrlList[i].acUrl);
		dlini.SetUInt32(_T("Interval"), m_UrlList[i].ulInterval);
	}
	dlini.SaveAndClose();
	return true;
	EMULE_CATCH
	return false;
}

void CAutoDL::Start()
{
	// Close any previous work
	Finish();
	// If we have nothing to check, don't start
	if(m_UrlList.GetCount() == 0 || !m_bUseIt)
		return;
	m_hFinish = CreateEvent(NULL, FALSE, FALSE, NULL);
	m_hWorkerThread = (HANDLE)_beginthread(WorkerThread, 0, reinterpret_cast<void*>(this));
}

void CAutoDL::Finish()
{
	EMULE_TRY
	if(m_hWorkerThread != NULL)
	{
		SetEvent(m_hFinish);
		// Wait 1 second for thread completion, then terminate it
		if(WaitForSingleObject(m_hWorkerThread, 1000) == WAIT_TIMEOUT)
		{
			if(!TerminateThread(m_hWorkerThread, 0))
				AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Can't terminate AutoDL thread!"));
		}
		m_hWorkerThread = NULL;
	}
	if (m_hFinish != NULL)
	{
		CloseHandle(m_hFinish);
		m_hFinish = NULL;
	}
	EMULE_CATCH
}

void CAutoDL::WorkerThread(void *pT)
{
	EMULE_TRY
	::CoInitialize(NULL);
	Sleep(5000);	// Wait until we load all things, etc
	CAutoDL* pThis = static_cast<CAutoDL*>(pT);
	while(WaitForSingleObject(pThis->m_hFinish, 1000) == WAIT_TIMEOUT)
	{
		for(int i = 0; i < pThis->m_UrlList.GetCount(); i++)
		{
			long lCurTime = time(NULL), lLastCheck = pThis->m_UrlList[i].lLastCheck;

			if ((lLastCheck == 0) || (lCurTime - lLastCheck) >= static_cast<long>(pThis->m_UrlList[i].ulInterval))
			{
				ULONG ulMinInterval = pThis->CheckUrl(pThis->m_UrlList[i].acUrl);
				if(ulMinInterval != 0 && ulMinInterval > pThis->m_UrlList[i].ulInterval)
					pThis->m_UrlList[i].ulInterval = ulMinInterval;
				pThis->m_UrlList[i].lLastCheck = lCurTime;
				// Check one URL at once
				break;
			}
		}
	}
	EMULE_CATCH
}

ULONG CAutoDL::CheckUrl(const TCHAR *pcUrl)
{
	ULONG ulMinInterval = 0;
	BYTE *pBuf = NULL;

	EMULE_TRY

	// Download file
	HINTERNET hOpen = InternetOpen(HTTP_USERAGENT, INTERNET_OPEN_TYPE_PRECONFIG , NULL, NULL, 0);
	if(hOpen)
	{
		HINTERNET hURL = InternetOpenUrl(hOpen, pcUrl, _T(""), NULL, INTERNET_FLAG_NO_CACHE_WRITE, NULL);
		if (hURL)
		{
			DWORD dwFileLen = 0, dwSize = sizeof(dwFileLen);
			HttpQueryInfo(hURL, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER, &dwFileLen, &dwSize, NULL);
			DWORD dwRead;
			pBuf = new BYTE[dwFileLen + 1];
			if(InternetReadFile(hURL, pBuf, dwFileLen, &dwRead))
				pBuf[dwRead] = 0;
		}
	}
	if(pBuf == NULL)
		return 0;
	// Load XML
	XmlDoc spDoc = XmlLoadDocumentFromStr((LPCTSTR)pBuf);
	if(spDoc)
	{
		XmlElement spRoot = spDoc->documentElement;
		if(spRoot)
		{
			ulMinInterval = XmlGetAttributeLong(spRoot, _T("min_interval"), 0);
			for(int i = 0; i < spRoot->childNodes->length; i++)
			{
				XmlElement spElem = spRoot->childNodes->item[i];
				CString sLink = XmlGetAttributeStr(spElem, _T("link"));
				CString sCategory = XmlGetAttributeStr(spElem, _T("category"));
				DownloadLink(sLink, sCategory);
			}
		}
	}
	EMULE_CATCH

	delete[] pBuf;
	return ulMinInterval;
}

void CAutoDL::DownloadLink(CString sLink, CString sCategory)
{
	EMULE_TRY

	// Check if we already have that file
	bool bSkip = false;
	CED2KLink* pLink = CED2KLink::CreateLinkFromUrl(sLink);
	if(pLink)
	{
		if(pLink->GetKind() == CED2KLink::kFile)
		{
			CKnownFile		*pKnownFile;
			CED2KFileLink	*pFileLink = pLink->GetFileLink();

			if (pFileLink != NULL)
			{
				if ((pKnownFile = g_App.m_pSharedFilesList->GetFileByID(pFileLink->GetHashKey())) != NULL)
					bSkip = true;
				else if ((pKnownFile = g_App.m_pDownloadQueue->GetFileByID(pFileLink->GetHashKey())) != NULL)
					bSkip = true;
				delete pFileLink;
			}
			else
				delete pLink;
		}
		else
			delete pLink;
	}
	if(bSkip)
		return;

	// Find category
	EnumCategories eCatID = CAT_NONE;
	for (int i = 0; i < CCat::GetNumCats(); i++)
	{
		if (i >= CCat::GetNumPredefinedCats())
			if(sCategory == CCat::GetCatByIndex(i)->GetTitle())
				eCatID = CCat::GetCatByIndex(i)->GetID();
	}

	// Add link
	g_App.m_pMDlg->m_dlgSearch.AddEd2kLinksToDownload(sLink, eCatID);

	EMULE_CATCH
}

bool CAutoDL::get_UseIt()
{
	return m_bUseIt;
}

void CAutoDL::put_UseIt(bool bUseIt)
{
	m_bUseIt = bUseIt;
}

long CAutoDL::get_UrlCount()
{
	return m_UrlList.GetSize();
}

CAutoDLData CAutoDL::get_UrlItem(long nIndex)
{
	return m_UrlList[nIndex];
}

void CAutoDL::ClearUrlList()
{
	m_UrlList.RemoveAll();
}

void CAutoDL::AddUrlItem(CAutoDLData& data)
{
	m_UrlList.Add(data);
}
