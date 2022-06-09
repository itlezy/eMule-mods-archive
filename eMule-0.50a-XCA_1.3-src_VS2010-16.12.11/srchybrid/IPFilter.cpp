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
#include "RarFile.h"
#include "ServerWnd.h"
//Xman end
#include "TransferDlg.h"
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

#define TEMPIPFILTERCOUNT 5000
#define INITIPFILTERCOUNT 260000

bool GetMimeType(LPCTSTR pszFilePath, CString& rstrMimeType); //Xman auto update IPFilter

#define	DFLT_FILTER_LEVEL	100 // default filter level if non specified
char* ScanIPRange(char *_src, uint32& ip1, uint32& ip2){// X: [SUL] - [SpeedUpLoading]
	return ((_src = inet_addr_ntohl(_src, ip1)) &&
		(_src = strchr(_src, '-')) &&
		(_src = inet_addr_ntohl(_src+1, ip2)))?_src:NULL;
}

char*ReplaceAndTrim(char*_src){// X: [SUL] - [SpeedUpLoading]
#define substr "PGIPDB"
	char*_srcnext=strstr(_src,substr);
	if(_srcnext){
		char*strbeg=_srcnext+_countof(substr)-1;
		do{
			char*strend=strstr(strbeg,substr);
			size_t len = (strend)?strend-strbeg:strlen(strbeg);
			strncpy(_srcnext,strbeg,len);
			_srcnext+=len;
			if(!strend) break;
			strbeg=strend+_countof(substr)-1;
		}while(1);
	}
	else
		_srcnext=_src+strlen(_src);
	while(*--_srcnext==' '||*_srcnext=='\t')
		;
	*(_srcnext+1)=0;
	while(*_src==' '||*_src=='\t')
		++_src;
	return _src;
}
SIPFilter::SIPFilter(uint32 newStart, uint32 newEnd, UINT newLevel, const char* newDesc)
: start(newStart),
end(newEnd),
level(newLevel),
hits(0)
{ 
	desc = (newDesc)?nstrdup(newDesc):NULL;
}
LPCSTR SIPFilter::GetDescA() const{
	return desc?desc:"";
}

CString SIPFilter::GetDesc() const{
	return desc?CString(desc):GetResString(IDS_TEMPORARY);
}

CIPFilter::CIPFilter()
{
	hits = 0;// X: [SFH] - [Show IP Filter Hits]
	m_pLastHit = NULL;
	m_bModified = false;
	LoadFromDefaultFile(false);
}

CIPFilter::~CIPFilter()
{
	if (m_bModified)
	{
		try{
			SaveToDefaultFile();
		}
		catch(CString){
		}
	}
	RemoveAllIPFilters();
}
/*
static int __cdecl CmpSIPFilterByStartAddr(const void* p1, const void* p2)
{
	const SIPFilter* rng1 = *(SIPFilter**)p1;
	const SIPFilter* rng2 = *(SIPFilter**)p2;
	return CompareUnsigned(rng1->start, rng2->start);
}
*/
bool CmpSIPFilterByStartAddr(const SIPFilter* rng1, const SIPFilter* rng2){
	return rng1->start < rng2->start;
}

bool CmpSIPFilterByAddr(const SIPFilter* pIPFilter, const uint32 ip){
	return pIPFilter->start < ip;
}
typedef bool(*_Pr1) (const SIPFilter*, const uint32);

template<class _FwdIt>
	_FwdIt Lower_bound(_FwdIt _First, size_t _Count,
		const uint32 _Val, _Pr1 _Pred){	// find first element not before _Val, using _Pred
	for (; 0 < _Count; ){	// divide and conquer, find half that contains answer
		size_t _Count2 = _Count / 2;
		_FwdIt _Mid = _First + _Count2;

		if(_Pred(*_Mid, _Val))
			_First = _Mid+1, _Count -= _Count2 + 1;
		else
			_Count = _Count2;
	}
	return (_First);
}
//Xman dynamic IP-Filters
void CIPFilter::AddIPTemporary(uint32 addip)// X: [ITF] - [Index Temporary Filter]
{
	//m_tmpiplist.Add(new SIPFilter(addip, addip,1,"temporary"));
	SIPFilter*tmp = new SIPFilter(addip, addip,1, NULL);
	m_iplist.insert(
		Lower_bound(m_iplist.begin(), m_iplist.size(), addip, CmpSIPFilterByAddr)
		, tmp);
	m_tmpiplist.push_back(STempIPFilter(tmp));
	// sort the IP filter list by IP range start addresses
	//std::sort(m_iplist.begin(), m_iplist.end(), CmpSIPFilterByStartAddr);*/
	//qsort(m_tmpiplist.GetData(), m_tmpiplist.GetCount(), sizeof(m_tmpiplist[0]), CmpSIPFilterByStartAddr);
}
//Xman end

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
		m_iplist.reserve(INITIPFILTERCOUNT);

		uint_ptr iFoundRanges = 0;
		uint_ptr iLine = 0;
		uint32 lastip=0;// X: [SUL] - [SpeedUpLoading]
		bool isSorted = true;
		if (eFileType == PeerGuardian2)
		{
			// Version 1: strings are ISO-8859-1 encoded
			// Version 2: strings are UTF-8 encoded
			uint8 nVersion;
			if (fread(&nVersion, sizeof nVersion, 1, readFile)==1 && (nVersion==1 || nVersion==2))
			{
				char*szName = NULL;
				if(!thePrefs.noIPFilterDesc)// X: [NIPFD] - [No IPFilter Description]
					szName = new char[256];
				while (!feof(readFile))
				{
					if(thePrefs.noIPFilterDesc){// X: [NIPFD] - [No IPFilter Description]
						for (;;) // read until NUL or EOF
						{
							int iChar = getc(readFile);
							if (iChar == EOF || iChar == '\0')
								break;
						}
					}
					else{
						for (int iLen = 0;;) // read until NUL or EOF
						{
							int iChar = getc(readFile);
							if (iChar == EOF || iChar == '\0'){
								szName[iLen] = '\0';
								break;
							}
							if (iLen < sizeof szName - 1)
								szName[iLen++] = (char)iChar;
						}
						
					}
					
					uint32 uStart;
					if (fread(&uStart, sizeof uStart, 1, readFile) != 1)
						break;
					uStart = ntohl(uStart);

					uint32 uEnd;
					if (fread(&uEnd, sizeof uEnd, 1, readFile) != 1)
						break;
					uEnd = ntohl(uEnd);

					iLine++;
					if((iLine & 0x3fff) == 0 && iLine > 3*0x3fff){// X: [MSI] - [More Splash Info]
						CString strLine;
						strLine.Format(_T("Loading %u"), iLine);
						if(theApp.IsSplash())
							theApp.UpdateSplash2(strLine);
						else if (theApp.emuledlg->statusbar->m_hWnd)
							theApp.emuledlg->statusbar->SetText(strLine,0,0);
					}
					// (nVersion == 2) ? OptUtf8ToStr(szName, iLen) : 
					AddIPRange(uStart, uEnd, DFLT_FILTER_LEVEL, szName);
					if(lastip > uStart)// X: [SUL] - [SpeedUpLoading]
						isSorted = false;
					lastip = uStart;
					iFoundRanges++;
				}
				if(szName)
					delete [] szName;
			}
		}
		else// X: [SUL] - [SpeedUpLoading]
		{
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
				if((iLine & 0x3fff) == 0 && iLine > 3*0x3fff){// X: [MSI] - [More Splash Info]
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
							}
						}
					}
				}

				bool bValid = false;
				uint32 start;
				uint32 end;
				UINT level;
				char*desc = NULL;// X: [NIPFD] - [No IPFilter Description]
				if (eFileType == FilterDat)
					bValid = ParseFilterLine1(pbuffer, start, end, level, &desc);
				else if (eFileType == PeerGuardian)
					bValid = ParseFilterLine2(pbuffer, start, end, level, &desc);

				// add a filter
				if (bValid)
				{
#ifdef _OPENMP
#pragma omp critical
#endif
					AddIPRange(start, end, level, desc);
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

			SIPFilter* pPrv = m_iplist[0];
			for(size_t i = 1;i < m_iplist.size(); ++i){
				SIPFilter* pCur = m_iplist[i];
				if (   pCur->start >= pPrv->start && pCur->start <= pPrv->end	 // overlapping
					|| pCur->start == pPrv->end+1 && pCur->level == pPrv->level) // adjacent
				{
					if (pCur->start != pPrv->start || pCur->end != pPrv->end) // don't merge identical entries
					{
						//TODO: not yet handled, overlapping entries with different 'level'
						if (pCur->end > pPrv->end)
							pPrv->end = pCur->end;
						//pPrv->desc += _T("; ") + pCur->desc; // this may create a very very long description string...
						++iMerged;
					}
					else
					{
						// if we have identical entries, use the lowest 'level'
						if (pCur->level < pPrv->level)
							pPrv->level = pCur->level;
						++iDuplicate;
					}
					delete pCur;
					//m_iplist.RemoveAt(i);	// way too expensive (read above)
					if(pcToDelete == NULL){
						pcToDelete = new char[m_iplist.size()];
						memset(pcToDelete, 0, m_iplist.size());
					}
					pcToDelete[i] = 1;		// mark this entry as 'to delete'
					++iNumToDelete;
				}
				else
					pPrv = pCur;
			}

			// Create new IP-filter list which contains only the entries from the original IP-filter list
			// which are not to be deleted.
			if (iNumToDelete > 0)
			{
				//CIPFilterArray newList;
				//newList.SetSize(m_iplist.GetCount() - iNumToDelete);
				int iNewListIndex = 0;
				for (size_t i = 0; i < m_iplist.size(); i++) {
					if (!pcToDelete[i]){
						//newList[iNewListIndex++] = m_iplist[i];
						if(iNewListIndex != i)
							m_iplist[iNewListIndex] = m_iplist[i];
						++iNewListIndex;
					}
				}
				//ASSERT( iNewListIndex == newList.GetSize() );
				m_iplist.erase(m_iplist.end() - iNumToDelete, m_iplist.end());
				ASSERT( iNewListIndex == m_iplist.size() );

				// Replace current list with new list. Dump, but still fast enough (only 1 memcpy)
				//m_iplist.RemoveAll();
				//m_iplist.Append(newList);
				//newList.RemoveAll();
				m_bModified = true;
				delete[] pcToDelete;
			}
		}

		CIPFilterArray temp;
		temp.reserve(m_iplist.size() + TEMPIPFILTERCOUNT);
		temp.insert(temp.begin(), m_iplist.begin(), m_iplist.end());
		m_iplist.swap(temp);

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

void CIPFilter::SaveToDefaultFile()
{
	if(thePrefs.noIPFilterDesc)// X: [NIPFD] - [No IPFilter Description]
		return;
	CString strFilePath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IPFILTER_FILENAME;
	FILE* fp = _tfsopen(strFilePath, _T("wt"), _SH_DENYWR);
	if (fp != NULL)
	{
		for (size_t i = 0; i < m_iplist.size(); i++)
		{
			const SIPFilter* flt = m_iplist[i];

			//Xman dynamic IP-Filters
			if(flt->IsTemp())
				continue; //don't save temporary filters
			//Xman end

			CHAR szStart[16];
			ipstrA(szStart, _countof(szStart), htonl(flt->start));

			CHAR szEnd[16];
			ipstrA(szEnd, _countof(szEnd), htonl(flt->end));

			if (fprintf(fp, "%-15s - %-15s , %3u , %s\n", szStart, szEnd, flt->level, flt->GetDescA()) == 0 || ferror(fp))
			{
				fclose(fp);
				CString strError;
				strError.Format(_T("Failed to save IP filter to file \"%s\" - %s"), strFilePath, _tcserror(errno));
				throw strError;
			}
		}
		fclose(fp);
		m_bModified = false;
	}
	else
	{
		CString strError;
		strError.Format(_T("Failed to save IP filter to file \"%s\" - %s"), strFilePath, _tcserror(errno));
		throw strError;
	}
}

bool CIPFilter::ParseFilterLine1(char* sbuffer, uint32& ip1, uint32& ip2, UINT& level, char** desc) const// X: [SUL] - [SpeedUpLoading]
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
	_src = strchr(pnext, ',');
	if(_src == NULL)
		return false;
	if(!thePrefs.noIPFilterDesc){// X: [NIPFD] - [No IPFilter Description]
		if(*++_src==' ')
			++_src;
		*desc = _src;
	}
	return true;
}

bool CIPFilter::ParseFilterLine2(char* sbuffer, uint32& ip1, uint32& ip2, UINT& level, char** desc) const// X: [SUL] - [SpeedUpLoading]
{
	char * pColon = strrchr(sbuffer, ':');
	if (pColon == NULL)
		return false;
	*pColon = 0;

	if(ScanIPRange(pColon+1, ip1, ip2) == NULL)
		return false;

	level = DFLT_FILTER_LEVEL;

	if(!thePrefs.noIPFilterDesc)// X: [NIPFD] - [No IPFilter Description]
		*desc = ReplaceAndTrim(sbuffer);
	return true;
}

void CIPFilter::RemoveAllIPFilters(){
	m_pLastHit = NULL;
	CTempIPFilterArray().swap(m_tmpiplist);// X: [ITF] - [Index Temporary Filter]
	for (size_t i = 0; i < m_iplist.size(); i++)
		delete m_iplist[i];
	CIPFilterArray().swap(m_iplist);
}

//Xman dynamic IP-Filters
void CIPFilter::Process()// X: [ITF] - [Index Temporary Filter]
{
	if(m_tmpiplist.size()==0)
		return;
	if(theApp.ipdlgisopen)
		return; //don't process if user is working on ipfilter
	const uint32 lasttick = ::GetTickCount();
	static uint32 m_lastcleanup;
	if(lasttick - m_lastcleanup > (1000 * 60 * 60)) //every hour
	{
		m_lastcleanup=lasttick;
		int countall=0;
		int countdel=0;
		for (size_t i=0;i<m_tmpiplist.size();)
		{
			STempIPFilter&search = m_tmpiplist[i];
			if(/*search->timestamp>0 && */((lasttick - search.timestamp) >=  (1000 * 60 * 60 * 12))) //12 hours
			{
				countdel++;
				RemoveIPFilter(search.filter, false);
				m_tmpiplist.erase(m_tmpiplist.begin() + i);
			}
			else
			{
				countall++;
				i++;
			}
		}
		AddDebugLogLine(false,_T("%u temporary IPFilters deleted, %u left"),countdel, countall)	;
	}
}
//Xman end

bool CIPFilter::IsFiltered(uint32 ip) /*const*/
{
	return IsFiltered(ip, thePrefs.GetIPFilterLevel());
}

static int __cdecl CmpSIPFilterByAddr(const void* pvKey, const void* pvElement)
{
	uint32 ip = *(uint32*)pvKey;
	const SIPFilter* pIPFilter = *(SIPFilter**)pvElement;

	if (ip < pIPFilter->start)
		return -1;
	if (ip > pIPFilter->end)
		return 1;
	return 0;
}

bool CIPFilter::IsFiltered(uint32 ip, UINT level) /*const*/
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
	SIPFilter** ppFound = (SIPFilter**)bsearch(&ip, &m_iplist.at(0), m_iplist.size(), sizeof(m_iplist[0]), CmpSIPFilterByAddr);
	if(ppFound && (*ppFound)->level < level){
		(*ppFound)->hits++;
		++hits;// X: [SFH] - [Show IP Filter Hits]
		if(!thePrefs.noIPFilterDesc)// X: [NIPFD] - [No IPFilter Description]
			m_pLastHit = *ppFound;
		theApp.emuledlg->transferwnd->ShowQueueCount();
		return true;
	}
	return false;
}

CString CIPFilter::GetLastHit() const
{
	return m_pLastHit?m_pLastHit->GetDesc():_T("Not available");
}

const CIPFilterArray& CIPFilter::GetIPFilter() const
{
	return m_iplist;
}

bool CIPFilter::RemoveIPFilter(const SIPFilter* pFilter, bool temp)
{
	/*for (int i = 0; i < m_iplist.GetCount(); i++)
	{
		if (m_iplist[i] == pFilter)
		{
			//Xman Code Fix: deleting the description-String can throw an exception
			try 
			{
				delete m_iplist[i];
			}
			catch(...)
			{
				//nothing
			}
			m_iplist.RemoveAt(i);
			return true;
		}
	}*/
	if(temp){
		for (size_t i = 0; i < m_tmpiplist.size(); i++)
		{
			if (m_tmpiplist[i].filter == pFilter){
				m_tmpiplist.erase(m_tmpiplist.begin()+i);
				break;
			}
		}
	}
	SIPFilter** ppFound = (SIPFilter**)bsearch(&pFilter->start, &m_iplist.at(0), m_iplist.size(), sizeof(m_iplist[0]), CmpSIPFilterByAddr);
	if(!ppFound)
		return false;
	while(*ppFound != pFilter){
		if((*ppFound)->start>pFilter->start)
			return false;
		++ppFound;
	}
#ifdef _STLP_WIN32
	m_iplist.erase(ppFound);
#else
	m_iplist.erase(CIPFilterArray::iterator(ppFound , &m_iplist));
#endif
/*	CIPFilterArray::iterator itFound;
if(Lower_bound(m_iplist.begin(), m_iplist.size(), pFilter->start, CmpSIPFilterByAddr, itFound)){
	ASSERT(*itFound == pFilter);
	m_iplist.erase(itFound);*/
	if(m_pLastHit == pFilter)
		m_pLastHit = NULL;
	delete pFilter;
	return true;
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

	CString strMimeType;
	GetMimeType(strTempFilePath, strMimeType);

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
