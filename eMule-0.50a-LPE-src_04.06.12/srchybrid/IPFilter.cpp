//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include "emule.h"
#include "IPFilter.h"
#include "OtherFunctions.h"
#include "StringConversion.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "Log.h"
//Xman auto update IPFilter
#include "HttpDownloadDlg.h"
#include "ZipFile.h"
#include "GZipFile.h"
//#include "RarFile.h"
#include "ServerWnd.h"
//Xman end
#include "TransferWnd.h"
#include <algorithm>
#include "MuleStatusBarCtrl.h"// X: [MSI] - [More Splash Info]

#ifdef _OPENMP
#include "omp.h"
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define INITIPFILTERCOUNT 260000
#define	DFLT_FILTER_LEVEL	100 // default filter level if non specified
char* ScanIPRange(char *_src, uint32& ip1, uint32& ip2){// X: [SUL] - [SpeedUpLoading]
	return ((_src = inet_addr_ntohl(_src, ip1)) &&
		(_src = strchr(_src, '-')) &&
		(_src = inet_addr_ntohl(_src+1, ip2)))?_src:NULL;
}

CIPFilter::CIPFilter()
{
	hits = 0;// X: [SFH] - [Show IP Filter Hits]
	LoadFromDefaultFile(false);
}

CIPFilter::~CIPFilter()
{
	RemoveAllIPFilters();
}
bool CmpSIPFilterByStartAddr(const SIPFilter& rng1, const SIPFilter& rng2){
	return rng1.start < rng2.start;
}

CString CIPFilter::GetDefaultFilePath() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IPFILTER_FILENAME;
}

void CIPFilter::LoadFromDefaultFile(bool bShowResponse)
{
	RemoveAllIPFilters();
	AddFromFile(GetDefaultFilePath(), bShowResponse);
}

size_t CIPFilter::AddFromFile(LPCTSTR pszFilePath, bool bShowResponse)
{
	const DWORD dwStart = GetTickCount();
	FILE* readFile = _tfsopen(pszFilePath, _T("rS"), _SH_DENYWR);
	if (readFile != NULL)
	{
		enum EIPFilterFileType
		{
			Unknown = 0,
			FilterDat = 1,		// ipfilter.dat/ip.prefix format
			PeerGuardian = 2,	// PeerGuardian text format
			PeerGuardian2 = 3	// PeerGuardian binary format
		} eFileType = Unknown;

		setvbuf(readFile, NULL, _IOFBF, 32768);

		TCHAR szNam[_MAX_FNAME];
		TCHAR szExt[_MAX_EXT];
		_tsplitpath_s(pszFilePath, NULL, 0, NULL, 0, szNam, _countof(szNam), szExt, _countof(szExt));
		if (_tcsicmp(szExt, _T(".p2p")) == 0 || (_tcsicmp(szNam, _T("guarding.p2p")) == 0 && _tcsicmp(szExt, _T(".txt")) == 0))
			eFileType = PeerGuardian;
		else if (_tcsicmp(szExt, _T(".prefix")) == 0)
			eFileType = FilterDat;
		else
		{
			VERIFY( _setmode(_fileno(readFile), _O_BINARY) != -1 );
			static const BYTE _aucP2Bheader[] = "\xFF\xFF\xFF\xFFP2B";
			BYTE aucHeader[sizeof _aucP2Bheader - 1];
			if (fread(aucHeader, sizeof aucHeader, 1, readFile) == 1)
			{
				if (memcmp(aucHeader, _aucP2Bheader, sizeof _aucP2Bheader - 1)==0)
					eFileType = PeerGuardian2;
				else
				{
					DWORD wBOM = *((DWORD*)&aucHeader);// X: ensure file is ANSI
					if(((WORD)wBOM == 0xFEFF) || ((WORD)wBOM == 0xFFFE) || ((wBOM & 0xFFFFFF) == 0xBFBBEF)){
						fclose(readFile);
						LogError(_T("Failed to load %s, the file is not ANSI"), pszFilePath);
						return 0;
					}
					(void)fseek(readFile, 0, SEEK_SET);
					VERIFY( _setmode(_fileno(readFile), _O_TEXT) != -1 ); // ugly!
				}
			}
		}

		uint_ptr iFoundRanges = 0;
		uint_ptr iLine = 0;
		uint32 lastip=0;// X: [SUL] - [SpeedUpLoading]
		bool isSorted = true;
		if (eFileType == PeerGuardian2)
		{
			if(thePrefs.GetIPFilterLevel() > DFLT_FILTER_LEVEL){
				m_iplist.reserve(INITIPFILTERCOUNT);
			// Version 1: strings are ISO-8859-1 encoded
			// Version 2: strings are UTF-8 encoded
			uint8 nVersion;
			if (fread(&nVersion, sizeof nVersion, 1, readFile)==1 && (nVersion==1 || nVersion==2))
			{
				while (!feof(readFile))
				{
					//CHAR szName[256];
					//int iLen = 0;
					for (;;) // read until NUL or EOF
					{
						int iChar = getc(readFile);
						if (iChar == EOF)
							break;
						//if (iLen < sizeof szName - 1)
						//	szName[iLen++] = (CHAR)iChar;
						if (iChar == '\0')
							break;
					}
					//szName[iLen] = '\0';
					
					uint32 uStart;
					if (fread(&uStart, sizeof uStart, 1, readFile) != 1)
						break;
					uStart = ntohl(uStart);

					uint32 uEnd;
					if (fread(&uEnd, sizeof uEnd, 1, readFile) != 1)
						break;
					uEnd = ntohl(uEnd);

					iLine++;
						if((iLine & 0x7fff) == 0 && iLine > 2*0x7fff && theApp.IsSplash()){// X: [MSI] - [More Splash Info]
						CString strLine;
							strLine.Format(_T("Loading %u"),iLine);
							theApp.UpdateSplash2(strLine);
					}
					// (nVersion == 2) ? OptUtf8ToStr(szName, iLen) : 
						AddIPRange(uStart, uEnd/*, DFLT_FILTER_LEVEL, CStringA(szName, iLen)*/);
					if(lastip > uStart)// X: [SUL] - [SpeedUpLoading]
						isSorted = false;
					lastip = uStart;
					iFoundRanges++;
				}
			}
		}
		}
		else if (eFileType != PeerGuardian || thePrefs.GetIPFilterLevel() > DFLT_FILTER_LEVEL)// X: [SUL] - [SpeedUpLoading]
		{
			m_iplist.reserve(INITIPFILTERCOUNT);
#ifdef _OPENMP
#pragma omp parallel
		{
#endif
			char szBuffer[1024];

			while (fgets(szBuffer, _countof(szBuffer), readFile) != NULL)
			{
#ifdef _OPENMP
#pragma omp atomic
#endif
				iLine++;
#ifndef _OPENMP
				if((iLine & 0x7fff) == 0 && iLine > 2*0x7fff && theApp.IsSplash()){// X: [MSI] - [More Splash Info]
					CString strLine;
					strLine.Format(_T("Loading %u"), iLine);
					if(theApp.IsSplash())
						theApp.UpdateSplash2(strLine);
					else if (theApp.emuledlg->statusbar->m_hWnd)
						theApp.emuledlg->statusbar->SetText(strLine,0,0);
				}
#endif
				size_t len = strlen(szBuffer);

				if (szBuffer[0] == '#' || szBuffer[0] == '/' || len < 5)
					continue;

				char*pbuffer = szBuffer;

				if (pbuffer[len-1] == '\n')
					pbuffer[len-1] = 0;

				if (eFileType == Unknown)
				{
					// looks like html
					if (strchr(pbuffer, '>') != NULL && strchr(pbuffer, '<') != NULL)
						pbuffer = strrchr(pbuffer, '>') + 1;

					// check for <IP> - <IP> at start of line
					uint32 ip1,ip2;
					if(ScanIPRange(pbuffer, ip1, ip2) != NULL)
					{
						eFileType = FilterDat;
					}
					else
					{
						// check for <description> ':' <IP> '-' <IP>
						char*pColon  = strchr(pbuffer, ':');
						if (pColon != NULL)
						{
							if(ScanIPRange(pColon+1, ip1, ip2) != NULL)
							{
								eFileType = PeerGuardian;
								if(thePrefs.GetIPFilterLevel() <= DFLT_FILTER_LEVEL)
									break;
							}
						}
					}
				}

				bool bValid = false;
				uint32 start;
				uint32 end;
				UINT level;
				if (eFileType == FilterDat)
					bValid = ParseFilterLine1(pbuffer, start, end, level);
				else if (eFileType == PeerGuardian)
					bValid = ParseFilterLine2(pbuffer, start, end, level);

				// add a filter
				if (bValid && thePrefs.GetIPFilterLevel() > (int)level){
#ifdef _OPENMP
#pragma omp critical
#endif
					AddIPRange(start, end);
#ifdef _OPENMP
#pragma omp atomic
#endif
					iFoundRanges++;
//#ifndef _OPENMP
					if(lastip > start)// X: [SUL] - [SpeedUpLoading]
						isSorted = false;
					lastip = start;					
//#endif
				}
				}
#ifdef _OPENMP
			}
			//isSorted = false;
#endif
		}
		fclose(readFile);
		/*CString strLine;// X: [MSI] - [More Splash Info]
		strLine.Format(_T("%u Lines Loaded"), iLine);
		if(theApp.IsSplash())
			theApp.UpdateSplash2(strLine);
		else if (theApp.emuledlg->statusbar->m_hWnd)
			theApp.emuledlg->statusbar->SetText(strLine,0,0);*/
		
		if(!isSorted)// X: [SUL] - [SpeedUpLoading]
			// sort the IP filter list by IP range start addresses
			std::sort(m_iplist.begin(), m_iplist.end(), CmpSIPFilterByStartAddr);
			//qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByStartAddr);

		// merge overlapping and adjacent filter ranges
		uint_ptr iDuplicate = 0;
		uint_ptr iMerged = 0;
		if (m_iplist.size() >= 2)
		{
			// On large IP-filter lists there is a noticeable performance problem when merging the list.
			// The 'CIPFilterArray::RemoveAt' call is way too expensive to get called during the merging,
			// thus we use temporary helper arrays to copy only the entries into the final list which
			// are not get deleted.

			// Reserve a byte array (its used as a boolean array actually) as large as the current 
			// IP-filter list, so we can set a 'to delete' flag for each entry in the current IP-filter list.
			char* pcToDelete = NULL;
			int iNumToDelete = 0;

			SIPFilter* pPrv = &m_iplist[0];
			for(size_t i = 1;i < m_iplist.size(); ++i){
				SIPFilter& pCur = m_iplist[i];
				if (   pCur.start >= pPrv->start && pCur.start <= pPrv->end)	 // overlapping
				{
					if (pCur.start != pPrv->start || pCur.end != pPrv->end) // don't merge identical entries
					{
						if (pCur.end > pPrv->end)
							pPrv->end = pCur.end;
						//pPrv->desc += _T("; ") + pCur.desc; // this may create a very very long description string...
						++iMerged;
					}
					else
					{
						++iDuplicate;
					}
					//m_iplist.RemoveAt(i);	// way too expensive (read above)
					if(pcToDelete == NULL){
						pcToDelete = new char[m_iplist.size()];
						memset(pcToDelete, 0, m_iplist.size());
					}
					pcToDelete[i] = 1;		// mark this entry as 'to delete'
					++iNumToDelete;
				}
				else
					pPrv = &pCur;
			}

			// Create new IP-filter list which contains only the entries from the original IP-filter list
			// which are not to be deleted.
			if (iNumToDelete > 0)
			{
				int iNewListIndex = 0;
				for (size_t i = 0; i < m_iplist.size(); i++) {
					if (!pcToDelete[i]){
						if(iNewListIndex != i)
							m_iplist[iNewListIndex] = m_iplist[i];
						++iNewListIndex;
					}
				}
				m_iplist.erase(m_iplist.end() - iNumToDelete, m_iplist.end());
				ASSERT( iNewListIndex == m_iplist.size() );

				// Replace current list with new list. Dump, but still fast enough (only 1 memcpy)
				delete[] pcToDelete;
			}
		}

	if(m_iplist.capacity() != m_iplist.size())
		CIPFilterArray(m_iplist).swap(m_iplist);

		if (thePrefs.GetVerbose())
		{
			const DWORD dwEnd = GetTickCount();
			AddDebugLogLine(false, _T("Loaded IP filters from \"%s\""), pszFilePath);
			AddDebugLogLine(false, _T("Parsed lines/entries:%u  Found IP ranges:%u  Duplicate:%u  Merged:%u  Time:%ums"), iLine, iFoundRanges, iDuplicate, iMerged, dwEnd-dwStart);
		}
		AddLogLine(bShowResponse, GetResString(IDS_IPFILTERLOADED), m_iplist.size());
	}
	return m_iplist.size();
}

bool CIPFilter::ParseFilterLine1(char* sbuffer, uint32& ip1, uint32& ip2, UINT& level) const// X: [SUL] - [SpeedUpLoading]
{
	char*_src = ScanIPRange(sbuffer, ip1, ip2);
	if(_src == NULL)
		return false;

	if(*_src == 0){
		level = DFLT_FILTER_LEVEL;	// set default level
		return true;
	}

	_src = strchr(_src, ',');
	if(_src == NULL)
		return false;
	char*pnext;
	level = (BYTE)strtoul(_src+1, (const char**)&pnext/*, 10*/);
	return true;
}

bool CIPFilter::ParseFilterLine2(char* sbuffer, uint32& ip1, uint32& ip2, UINT& level) const// X: [SUL] - [SpeedUpLoading]
{
	char * pColon = strrchr(sbuffer, ':');
	if (pColon == NULL)
		return false;

	if(ScanIPRange(pColon+1, ip1, ip2) == NULL)
		return false;

	level = DFLT_FILTER_LEVEL;

	return true;
}

void CIPFilter::RemoveAllIPFilters(){
	CIPFilterArray().swap(m_iplist);
}

static int __cdecl CmpSIPFilterByAddr(const void* pvKey, const void* pvElement)
{
	uint32 ip = *(uint32*)pvKey;
	const SIPFilter*pIPFilter = (SIPFilter*)pvElement;

	if (ip < pIPFilter->start)
		return -1;
	if (ip > pIPFilter->end)
		return 1;
	return 0;
}

bool CIPFilter::IsFiltered(uint32 ip) /*const*/
{
	if (m_iplist.size() == 0 || ip == 0)
		return false;
	ip = htonl(ip);

	// to speed things up we use a binary search
	//	*)	the IP filter list must be sorted by IP range start addresses
	//	*)	the IP filter list is not allowed to contain overlapping IP ranges (see also the IP range merging code when
	//		loading the list)
	//	*)	the filter 'level' is ignored during the binary search and is evaluated only for the found element
	//
	// TODO: this can still be improved even more:
	//	*)	use a pre assembled list of IP ranges which contains only the IP ranges for the currently used filter level
	//	*)	use a dumb plain array for storing the IP range structures. this will give more cach hits when processing
	//		the list. but(!) this would require to also use a dumb SIPFilter structure (don't use data items with ctors).
	//		otherwise the creation of the array would be rather slow.
	SIPFilter* ppFound = (SIPFilter*)bsearch(&ip, &m_iplist.at(0), m_iplist.size(), sizeof(m_iplist[0]), CmpSIPFilterByAddr);
	if(ppFound){
		++hits;// X: [SFH] - [Show IP Filter Hits]
		theApp.emuledlg->transferwnd->ShowQueueCount();
		return true;
	}
	return false;
}

//Xman auto update IPFilter
void CIPFilter::UpdateIPFilterURL(CString url)
{
	//Xman auto update IPFilter
	if(url.IsEmpty()){
		url=thePrefs.GetAutoUpdateIPFilter_URL();
		if(url.IsEmpty())
			return;
		if (PathFileExists(GetDefaultFilePath()) && thePrefs.m_last_ipfilter_check!=0) {
			CTime last(thePrefs.m_last_ipfilter_check);
			struct tm tmTemp;
			time_t tLast=safe_mktime(last.GetLocalTm(&tmTemp));
			time_t tNow=safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
			if ( (difftime(tNow,tLast) / 86400)< 2/*thePrefs.GetUpdateDays()*/ )
				return;
		}
		theApp.SplashHide(); //Xman new slpash-screen arrangement
	}
	//Xman end

	bool bHaveNewFilterFile = false;
//	CString url = thePrefs.GetAutoUpdateIPFilter_URL();
	SYSTEMTIME SysTime;
//	if (!url.IsEmpty()){
	CString strTempFilePath;
	_tmakepathlimit(strTempFilePath.GetBuffer(MAX_PATH), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IPFILTER_FILENAME, _T("tmp"));
	strTempFilePath.ReleaseBuffer();

	CHttpDownloadDlg dlgDownload;
	dlgDownload.m_strTitle = GetResString(IDS_DWL_IPFILTERFILE);
	dlgDownload.m_sURLToDownload = url;
	dlgDownload.m_sFileToDownloadInto = strTempFilePath;

	if (PathFileExists(GetDefaultFilePath()))
		memcpy(&SysTime, &thePrefs.m_IPfilterVersion, sizeof(SYSTEMTIME));
	else
		memset(&SysTime, 0, sizeof(SYSTEMTIME));
	dlgDownload.m_pLastModifiedTime = &SysTime; //Xman remark: m_pLastModifiedTime is a pointer which points to the SysTime-struct

	if (dlgDownload.DoModal() != IDOK)
	{
		(void)_tremove(strTempFilePath);
		if(!theApp.DidWeAutoStart())
		{
			CString strError = GetResString(IDS_DWLIPFILTERFAILED);
			if (!dlgDownload.GetError().IsEmpty())
				strError += _T("\r\n\r\n") + dlgDownload.GetError();
			AfxMessageBox(strError, MB_ICONERROR);
		}
		return;
	}
	if (dlgDownload.m_pLastModifiedTime == NULL)
	{
		//Xman auto update IPFilter
		struct tm tmTemp;
		thePrefs.m_last_ipfilter_check = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
		//Xman end
		return;
	}

	//CString strMimeType;
	//GetMimeType(strTempFilePath, strMimeType);

	bool bIsArchiveFile = false;
	bool bUncompressed = false;
	CZIPFile zip;
	if (zip.Open(strTempFilePath))
	{
		bIsArchiveFile = true;

		CZIPFile::File* zfile = zip.GetFile(_T("ipfilter.dat"));
		if (zfile == NULL)
			zfile = zip.GetFile(_T("guarding.p2p"));
		if (zfile == NULL)
			zfile = zip.GetFile(_T("guardian.p2p"));
		if (zfile)
		{
			CString strTempUnzipFilePath;
			_tmakepathlimit(strTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IPFILTER_FILENAME, _T(".unzip.tmp"));
			strTempUnzipFilePath.ReleaseBuffer();

			if (zfile->Extract(strTempUnzipFilePath))
			{
				//zip.Close();
				//zfile = NULL;

				if (_tremove(GetDefaultFilePath()) != 0)
					TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %s\n"), GetDefaultFilePath(), _tcserror(errno));
				if (_trename(strTempUnzipFilePath, GetDefaultFilePath()) != 0)
					TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %s\n"), strTempUnzipFilePath, GetDefaultFilePath(), _tcserror(errno));
				if (_tremove(strTempFilePath) != 0)
					TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %s\n"), strTempFilePath, _tcserror(errno));
				bUncompressed = true;
				bHaveNewFilterFile = true;
			}
			else if(!theApp.DidWeAutoStart()){
				CString strError;
				strError.Format(GetResString(IDS_ERR_IPFILTERZIPEXTR), strTempFilePath);
				AfxMessageBox(strError, MB_ICONERROR);
			}
		}
		else if(!theApp.DidWeAutoStart()){
			CString strError;
			strError.Format(GetResString(IDS_ERR_IPFILTERCONTENTERR), strTempFilePath);
			AfxMessageBox(strError, MB_ICONERROR);
		}

		zip.Close();
	}
	/*
	else if (strMimeType.CompareNoCase(_T("application/x-rar-compressed")) == 0)
	{
		bIsArchiveFile = true;

		CRARFile rar;
		if (rar.Open(strTempFilePath))
		{
			CString strFile;
			if (rar.GetNextFile(strFile)
				&& (   strFile.CompareNoCase(_T("ipfilter.dat")) == 0 
				    || strFile.CompareNoCase(_T("guarding.p2p")) == 0
					|| strFile.CompareNoCase(_T("guardian.p2p")) == 0))
			{
				CString strTempUnzipFilePath;
				_tmakepathlimit(strTempUnzipFilePath.GetBuffer(MAX_PATH), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IPFILTER_FILENAME, _T(".unzip.tmp"));
				strTempUnzipFilePath.ReleaseBuffer();
				if (rar.Extract(strTempUnzipFilePath))
				{
					//rar.Close();

					if (_tremove(GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %s\n"), GetDefaultFilePath(), _tcserror(errno));
					if (_trename(strTempUnzipFilePath, GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %s\n"), strTempUnzipFilePath, GetDefaultFilePath(), _tcserror(errno));
					if (_tremove(strTempFilePath) != 0)
						TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %s\n"), strTempFilePath, _tcserror(errno));
					bUncompressed = true;
					bHaveNewFilterFile = true;
				}
				else if(!theApp.DidWeAutoStart())
				{
					CString strError;
					strError.Format(_T("Failed to extract IP filter file from RAR file \"%s\"."), strTempFilePath);
					AfxMessageBox(strError, MB_ICONERROR);
				}
			}
			else if(!theApp.DidWeAutoStart())
			{
				CString strError;
				strError.Format(_T("Failed to find IP filter file \"guarding.p2p\" or \"ipfilter.dat\" in RAR file \"%s\"."), strTempFilePath);
				AfxMessageBox(strError, MB_ICONERROR);
			}
			rar.Close();
		}
		else if(!theApp.DidWeAutoStart())
		{
			CString strError;
			strError.Format(_T("Failed to open file \"%s\".\r\n\r\nInvalid file format?\r\n\r\nDownload latest version of UNRAR.DLL from http://www.rarlab.com and copy UNRAR.DLL into eMule installation folder."), url);
			AfxMessageBox(strError, MB_ICONERROR);
		}
	}
    */
	else
	{
		CGZIPFile gz;
		if (gz.Open(strTempFilePath))
		{
			bIsArchiveFile = true;

			CString strTempUnzipFilePath;
			_tmakepathlimit(strTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IPFILTER_FILENAME, _T(".unzip.tmp"));
			strTempUnzipFilePath.ReleaseBuffer();

			// add filename and extension of uncompressed file to temporary file
			CString strUncompressedFileName = gz.GetUncompressedFileName();
			if (!strUncompressedFileName.IsEmpty())
			{
				strTempUnzipFilePath += _T('.');
				strTempUnzipFilePath += strUncompressedFileName;
			}

			if (gz.Extract(strTempUnzipFilePath))
			{
				//gz.Close();

				if (_tremove(GetDefaultFilePath()) != 0)
					TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %s\n"), GetDefaultFilePath(), _tcserror(errno));
				if (_trename(strTempUnzipFilePath, GetDefaultFilePath()) != 0)
					TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %s\n"), strTempUnzipFilePath, GetDefaultFilePath(), _tcserror(errno));
				if (_tremove(strTempFilePath) != 0)
					TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %s\n"), strTempFilePath, _tcserror(errno));
				bUncompressed = true;
				bHaveNewFilterFile = true;
			}
			else if(!theApp.DidWeAutoStart()){
				CString strError;
				strError.Format(GetResString(IDS_ERR_IPFILTERZIPEXTR), strTempFilePath);
				AfxMessageBox(strError, MB_ICONERROR);
			}
			gz.Close();
		}
	}

	if (!bIsArchiveFile && !bUncompressed)
	{
		// Check first lines of downloaded file for potential HTML content (e.g. 404 error pages)
		bool bValidIPFilterFile = true;
		FILE* fp = _tfsopen(strTempFilePath, _T("rbS"), _SH_DENYWR);
		if (fp)
		{
			char szBuff[16384];
			size_t iRead = fread(szBuff, 1, _countof(szBuff)-1, fp);
			if (iRead <= 0)
				bValidIPFilterFile = false;
			else
			{
				szBuff[iRead-1] = '\0';

				const char* pc = szBuff;
				while (*pc == ' ' || *pc == '\t' || *pc == '\r' || *pc == '\n')
					pc++;
				if (_strnicmp(pc, "<html", 5) == 0
					|| _strnicmp(pc, "<xml", 4) == 0
					|| _strnicmp(pc, "<!doc", 5) == 0)
				{
					bValidIPFilterFile = false;
				}
			}
			fclose(fp);
		}

		if (bValidIPFilterFile)
		{
			(void)_tremove(GetDefaultFilePath());
			VERIFY( _trename(strTempFilePath, GetDefaultFilePath()) == 0 );
			bHaveNewFilterFile = true;
		}
		else if(!theApp.DidWeAutoStart())
		{
			AfxMessageBox(GetResString(IDS_DWLIPFILTERFAILED), MB_ICONERROR);
		}
	}

	// In case we received an invalid IP-filter file (e.g. an 404 HTML page with HTTP status "OK"),
	// warn the user that there are no IP-filters available any longer.
//	if (bHaveNewFilterFile && m_iplist.GetCount() == 0){
	if (bHaveNewFilterFile){
		//Xman auto update IPFilter
		struct tm tmTemp;
		thePrefs.m_last_ipfilter_check = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
		//Xman end

		LoadFromDefaultFile();
		//if (thePrefs.GetFilterServerByIP())
		theApp.emuledlg->serverwnd->serverlistctrl.RemoveAllFilteredServers();
		if(m_iplist.size() == 0)
		{
			if(!theApp.DidWeAutoStart())
			{
				CString strLoaded;
				strLoaded.Format(GetResString(IDS_IPFILTERLOADED), 0/*m_iplist.GetCount()*/);
				CString strError;
				strError.Format(_T("%s\r\n\r\n%s"), GetResString(IDS_DWLIPFILTERFAILED), strLoaded);
				AfxMessageBox(strError, MB_ICONERROR);
//				return;
			}
		}

		//everything fine. update the stored version
//	if (bHaveNewFilterFile)
		else
			memcpy(&thePrefs.m_IPfilterVersion, &SysTime, sizeof SysTime); 
	}
}//Xman end