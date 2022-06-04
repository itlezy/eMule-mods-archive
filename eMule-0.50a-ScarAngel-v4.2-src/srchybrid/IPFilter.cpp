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
// ==> Advanced Updates [MorphXT/Stulle] - Stulle
#include "PreferencesDlg.h"
#include "PPgScar.h"
// <== Advanced Updates [MorphXT/Stulle] - Stulle
#include "MuleStatusBarCtrl.h" // Show (un-)loading status of IPFilter [Stulle] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool GetMimeType(LPCTSTR pszFilePath, CString& rstrMimeType); //Xman auto update IPFilter

#define	DFLT_FILTER_LEVEL	100 // default filter level if non specified

CIPFilter::CIPFilter()
{
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

static int __cdecl CmpSIPFilterByStartAddr(const void* p1, const void* p2)
{
	const SIPFilter* rng1 = *(SIPFilter**)p1;
	const SIPFilter* rng2 = *(SIPFilter**)p2;
	return CompareUnsigned(rng1->start, rng2->start);
}

CString CIPFilter::GetDefaultFilePath() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IPFILTER_FILENAME;
}

int CIPFilter::LoadFromDefaultFile(bool bShowResponse)
{
	RemoveAllIPFilters();

	// ==> IP Filter White List [Stulle] - Stulle
	AddFromFileWhite(GetDefaultWhiteFilePath());
	// <== IP Filter White List [Stulle] - Stulle

	// ==> Static IP Filter [Stulle] - Stulle
	AddFromFile2(GetDefaultStaticFilePath());
	// <== Static IP Filter [Stulle] - Stulle

	return AddFromFile(GetDefaultFilePath(), bShowResponse);
}

int CIPFilter::AddFromFile(LPCTSTR pszFilePath, bool bShowResponse)
{
	DWORD dwStart = GetTickCount();
	// ==> Show (un-)loading status of IPFilter [Stulle] - Stulle
	int iLineCount = 0;
	int iLastPercent = 0;
	CStdioFile countFile;
	if(countFile.Open(pszFilePath, CFile::modeRead)==TRUE)
	{
		CString strBuffer;
		while(countFile.ReadString(strBuffer)!=FALSE)
			iLineCount++;
		countFile.Close();
	}
	// <== Show (un-)loading status of IPFilter [Stulle] - Stulle
	FILE* readFile = _tfsopen(pszFilePath, _T("r"), _SH_DENYWR);
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
		_tsplitpath(pszFilePath, NULL, NULL, szNam, szExt);
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
					(void)fseek(readFile, 0, SEEK_SET);
					VERIFY( _setmode(_fileno(readFile), _O_TEXT) != -1 ); // ugly!
				}
			}
		}

		int iFoundRanges = 0;
		int iLine = 0;
		if (eFileType == PeerGuardian2)
		{
			// Version 1: strings are ISO-8859-1 encoded
			// Version 2: strings are UTF-8 encoded
			uint8 nVersion;
			if (fread(&nVersion, sizeof nVersion, 1, readFile)==1 && (nVersion==1 || nVersion==2))
			{
				while (!feof(readFile))
				{
					CHAR szName[256];
					int iLen = 0;
					for (;;) // read until NUL or EOF
					{
						int iChar = getc(readFile);
						if (iChar == EOF)
							break;
						if (iLen < sizeof szName - 1)
							szName[iLen++] = (CHAR)iChar;
						if (iChar == '\0')
							break;
					}
					szName[iLen] = '\0';
					
					uint32 uStart;
					if (fread(&uStart, sizeof uStart, 1, readFile) != 1)
						break;
					uStart = ntohl(uStart);

					uint32 uEnd;
					if (fread(&uEnd, sizeof uEnd, 1, readFile) != 1)
						break;
					uEnd = ntohl(uEnd);

					iLine++;
					// ==> Show (un-)loading status of IPFilter [Stulle] - Stulle
					if(iLineCount)
					{
						int iPercent = (int)((float(iLine)*100.0f)/float(iLineCount));
						if(iPercent - iLastPercent > 1)
						{
							CString strPercent;
							strPercent.Format(_T("Loading IPfilter (%i %%) ..."),iPercent);
							if(theApp.IsSplash())
								theApp.UpdateSplash(strPercent);
							else if (theApp.emuledlg->statusbar->m_hWnd)
								theApp.emuledlg->statusbar->SetText(strPercent,0,0);
							iLastPercent = iPercent;
						}
					}
					// <== Show (un-)loading status of IPFilter [Stulle] - Stulle
					// (nVersion == 2) ? OptUtf8ToStr(szName, iLen) : 
					AddIPRange(uStart, uEnd, DFLT_FILTER_LEVEL, CStringA(szName, iLen));
					iFoundRanges++;
				}
			}
		}
		else
		{
			CStringA sbuffer;
			CHAR szBuffer[1024];
			while (fgets(szBuffer, _countof(szBuffer), readFile) != NULL)
			{
				iLine++;
				// ==> Show (un-)loading status of IPFilter [Stulle] - Stulle
				if(iLineCount)
				{
					int iPercent = (int)((float(iLine)*100.0f)/float(iLineCount));
					if(iPercent - iLastPercent > 1)
					{
						CString strPercent;
						strPercent.Format(_T("Loading IPfilter (%i %%) ..."),iPercent);
						if(theApp.IsSplash())
							theApp.UpdateSplash(strPercent);
						else if (theApp.emuledlg->statusbar->m_hWnd)
							theApp.emuledlg->statusbar->SetText(strPercent,0,0);
						iLastPercent = iPercent;
					}
				}
				// <== Show (un-)loading status of IPFilter [Stulle] - Stulle
				sbuffer = szBuffer;
				
				// ignore comments & too short lines
				if (sbuffer.GetAt(0) == '#' || sbuffer.GetAt(0) == '/' || sbuffer.GetLength() < 5) {
					sbuffer.Trim(" \t\r\n");
					DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter: ignored line %u\n", iLine) : 0 );
					continue;
				}

				if (eFileType == Unknown)
				{
					// looks like html
					if (sbuffer.Find('>') > -1 && sbuffer.Find('<') > -1)
						sbuffer.Delete(0, sbuffer.ReverseFind('>') + 1);

					// check for <IP> - <IP> at start of line
					UINT u1, u2, u3, u4, u5, u6, u7, u8;
					if (sscanf(sbuffer, "%u.%u.%u.%u - %u.%u.%u.%u", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) == 8)
					{
						eFileType = FilterDat;
					}
					else
					{
						// check for <description> ':' <IP> '-' <IP>
						int iColon = sbuffer.Find(':');
						if (iColon > -1)
						{
							CStringA strIPRange = sbuffer.Mid(iColon + 1);
							UINT u1, u2, u3, u4, u5, u6, u7, u8;
							if (sscanf(strIPRange, "%u.%u.%u.%u - %u.%u.%u.%u", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) == 8)
							{
								eFileType = PeerGuardian;
							}
						}
					}
				}

				bool bValid = false;
				uint32 start = 0;
				uint32 end = 0;
				UINT level = 0;
				CStringA desc;
				if (eFileType == FilterDat)
					bValid = ParseFilterLine1(sbuffer, start, end, level, desc);
				else if (eFileType == PeerGuardian)
					bValid = ParseFilterLine2(sbuffer, start, end, level, desc);

				// add a filter
				if (bValid)
				{
					AddIPRange(start, end, level, desc);
					iFoundRanges++;
				}
				else
				{
					sbuffer.Trim(" \t\r\n");
					DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter: ignored line %u\n", iLine) : 0 );
				}
			}
		}
		fclose(readFile);

		// sort the IP filter list by IP range start addresses
		qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByStartAddr);

		// merge overlapping and adjacent filter ranges
		int iDuplicate = 0;
		int iMerged = 0;
		if (m_iplist.GetCount() >= 2)
		{
			// On large IP-filter lists there is a noticeable performance problem when merging the list.
			// The 'CIPFilterArray::RemoveAt' call is way too expensive to get called during the merging,
			// thus we use temporary helper arrays to copy only the entries into the final list which
			// are not get deleted.

			// Reserve a byte array (its used as a boolean array actually) as large as the current 
			// IP-filter list, so we can set a 'to delete' flag for each entry in the current IP-filter list.
			char* pcToDelete = new char[m_iplist.GetCount()];
			memset(pcToDelete, 0, m_iplist.GetCount());
			int iNumToDelete = 0;

			SIPFilter* pPrv = m_iplist[0];
			int i = 1;
			while (i < m_iplist.GetCount())
			{
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
						iMerged++;
					}
					else
					{
						// if we have identical entries, use the lowest 'level'
						if (pCur->level < pPrv->level)
							pPrv->level = pCur->level;
						iDuplicate++;
					}
					delete pCur;
					//m_iplist.RemoveAt(i);	// way too expensive (read above)
					pcToDelete[i] = 1;		// mark this entry as 'to delete'
					iNumToDelete++;
					i++;
					continue;
				}
				pPrv = pCur;
				i++;
			}

			// Create new IP-filter list which contains only the entries from the original IP-filter list
			// which are not to be deleted.
			if (iNumToDelete > 0)
			{
				CIPFilterArray newList;
				newList.SetSize(m_iplist.GetCount() - iNumToDelete);
				int iNewListIndex = 0;
				for (int i = 0; i < m_iplist.GetCount(); i++) {
					if (!pcToDelete[i])
						newList[iNewListIndex++] = m_iplist[i];
				}
				ASSERT( iNewListIndex == newList.GetSize() );

				// Replace current list with new list. Dump, but still fast enough (only 1 memcpy)
				m_iplist.RemoveAll();
				m_iplist.Append(newList);
				newList.RemoveAll();
				m_bModified = true;
			}
			delete[] pcToDelete;
		}

		if (thePrefs.GetVerbose())
		{
			DWORD dwEnd = GetTickCount();
			AddDebugLogLine(false, _T("Loaded IP filters from \"%s\""), pszFilePath);
			AddDebugLogLine(false, _T("Parsed lines/entries:%u  Found IP ranges:%u  Duplicate:%u  Merged:%u  Time:%s"), iLine, iFoundRanges, iDuplicate, iMerged, CastSecondsToHM((dwEnd-dwStart+500)/1000));
		}
		AddLogLine(bShowResponse, GetResString(IDS_IPFILTERLOADED), m_iplist.GetCount());
	}
	return m_iplist.GetCount();
}

void CIPFilter::SaveToDefaultFile()
{
	CString strFilePath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_IPFILTER_FILENAME;
	FILE* fp = _tfsopen(strFilePath, _T("wt"), _SH_DENYWR);
	if (fp != NULL)
	{
		for (int i = 0; i < m_iplist.GetCount(); i++)
		{
			const SIPFilter* flt = m_iplist[i];

			//Xman dynamic IP-Filters
			if(flt->timestamp!=0)
				continue; //don't save temporary filerts
			//Xman end

			CHAR szStart[16];
			ipstrA(szStart, _countof(szStart), htonl(flt->start));

			CHAR szEnd[16];
			ipstrA(szEnd, _countof(szEnd), htonl(flt->end));

			if (fprintf(fp, "%-15s - %-15s , %3u , %s\n", szStart, szEnd, flt->level, flt->desc) == 0 || ferror(fp))
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

bool CIPFilter::ParseFilterLine1(const CStringA& sbuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& desc) const
{
	UINT u1, u2, u3, u4, u5, u6, u7, u8, uLevel = DFLT_FILTER_LEVEL;
	int iDescStart = 0;
	int iItems = sscanf(sbuffer, "%u.%u.%u.%u - %u.%u.%u.%u , %u , %n", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8, &uLevel, &iDescStart);
	if (iItems < 8)
		return false;

	((BYTE*)&ip1)[0] = (BYTE)u4;
	((BYTE*)&ip1)[1] = (BYTE)u3;
	((BYTE*)&ip1)[2] = (BYTE)u2;
	((BYTE*)&ip1)[3] = (BYTE)u1;

	((BYTE*)&ip2)[0] = (BYTE)u8;
	((BYTE*)&ip2)[1] = (BYTE)u7;
	((BYTE*)&ip2)[2] = (BYTE)u6;
	((BYTE*)&ip2)[3] = (BYTE)u5;

	if (iItems == 8) {
		level = DFLT_FILTER_LEVEL;	// set default level
		return true;
	}

	level = uLevel;

	if (iDescStart > 0)
	{
		LPCSTR pszDescStart = (LPCSTR)sbuffer + iDescStart;
		int iDescLen = sbuffer.GetLength() - iDescStart;
		if (iDescLen > 0) {
			if (*(pszDescStart + iDescLen - 1) == '\n')
				--iDescLen;
		}
		memcpy(desc.GetBuffer(iDescLen), pszDescStart, iDescLen * sizeof(pszDescStart[0]));
		desc.ReleaseBuffer(iDescLen);
	}

	return true;
}

bool CIPFilter::ParseFilterLine2(const CStringA& sbuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& desc) const
{
	int iPos = sbuffer.ReverseFind(':');
	if (iPos < 0)
		return false;

	desc = sbuffer.Left(iPos);
	desc.Replace("PGIPDB", "");
	desc.Trim();

	CStringA strIPRange = sbuffer.Mid(iPos + 1, sbuffer.GetLength() - iPos);
	UINT u1, u2, u3, u4, u5, u6, u7, u8;
	if (sscanf(strIPRange, "%u.%u.%u.%u - %u.%u.%u.%u", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) != 8)
		return false;

	((BYTE*)&ip1)[0] = (BYTE)u4;
	((BYTE*)&ip1)[1] = (BYTE)u3;
	((BYTE*)&ip1)[2] = (BYTE)u2;
	((BYTE*)&ip1)[3] = (BYTE)u1;

	((BYTE*)&ip2)[0] = (BYTE)u8;
	((BYTE*)&ip2)[1] = (BYTE)u7;
	((BYTE*)&ip2)[2] = (BYTE)u6;
	((BYTE*)&ip2)[3] = (BYTE)u5;

	level = DFLT_FILTER_LEVEL;

	return true;
}

void CIPFilter::RemoveAllIPFilters()
{
	// ==> Show (un-)loading status of IPFilter [Stulle] - Stulle
	int iLine = 0;
	int iLineCount = m_iplist.GetCount();
	int iLastPercent = 0;
	// <== Show (un-)loading status of IPFilter [Stulle] - Stulle

	for (int i = 0; i < m_iplist.GetCount(); i++)
	//Xman Code Fix: deleting the description-String can throw an exception
	/*
		delete m_iplist[i];
	*/
	{
		
		try 
		{
			// ==> Show (un-)loading status of IPFilter [Stulle] - Stulle
			iLine++;
			if(iLineCount)
			{
				int iPercent = (int)((float(iLine)*100.0f)/float(iLineCount));
				if(iPercent - iLastPercent > 1)
				{
					CString strPercent;
					strPercent.Format(_T("Unloading IPfilter (%i %%) ..."),iPercent);
					if(theApp.IsSplash())
						theApp.UpdateSplash(strPercent);
					else if (theApp.emuledlg->statusbar->m_hWnd)
						theApp.emuledlg->statusbar->SetText(strPercent,0,0);
					iLastPercent = iPercent;
				}
			}
			// <== Show (un-)loading status of IPFilter [Stulle] - Stulle
			delete m_iplist[i];
		}
		catch(...)
		{
			//nothing
		}
	}
	//Xman end
	m_iplist.RemoveAll();
	// ==> IP Filter White List [Stulle] - Stulle
	for (int i = 0; i < m_iplist_White.GetCount(); i++)
	{
		
		try 
		{
			delete m_iplist_White[i];
		}
		catch(...)
		{
			//nothing
		}
	}
	//Xman end
	m_iplist_White.RemoveAll();
	// <== IP Filter White List [Stulle] - Stulle
	m_pLastHit = NULL;
}

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
	if (m_iplist.GetCount() == 0 || ip == 0)
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

	// ==> IP Filter White List [Stulle] - Stulle
	if(m_iplist_White.GetCount() > 0)
	{
		SIPFilter** ppFound_White = (SIPFilter**)bsearch(&ip, m_iplist_White.GetData(), m_iplist_White.GetCount(), sizeof(m_iplist_White[0]), CmpSIPFilterByAddr);
		if (ppFound_White && (*ppFound_White)->level < level)
		{
			(*ppFound_White)->hits++;
			if(thePrefs.GetVerbose() && thePrefs.GetLogFilteredIPs())
				AddDebugLogLine(false, _T("Prevented filtering IP %s in range: %s - %s Description: %s Hits: %u"),ipstr_rev(ip),ipstr_rev((*ppFound_White)->start),ipstr_rev((*ppFound_White)->end),CString((*ppFound_White)->desc),(*ppFound_White)->hits);
			return false;
		}
	}
	// <== IP Filter White List [Stulle] - Stulle

	SIPFilter** ppFound = (SIPFilter**)bsearch(&ip, m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByAddr);
	if (ppFound && (*ppFound)->level < level)
	{
		(*ppFound)->hits++;
		m_pLastHit = *ppFound;
		return true;
	}

	return false;
}

CString CIPFilter::GetLastHit() const
{
	return m_pLastHit ? CString(m_pLastHit->desc) : _T("Not available");
}

const CIPFilterArray& CIPFilter::GetIPFilter() const
{
	return m_iplist;
}

bool CIPFilter::RemoveIPFilter(const SIPFilter* pFilter)
{
	for (int i = 0; i < m_iplist.GetCount(); i++)
	{
		if (m_iplist[i] == pFilter)
		{
			//Xman Code Fix: deleting the description-String can throw an exception
			/*
			delete m_iplist[i];
			*/
			try 
			{
				delete m_iplist[i];
			}
			catch(...)
			{
				//nothing
			}
			//Xman end
			m_iplist.RemoveAt(i);
			return true;
		}
	}
	return false;
}

//Xman dynamic IP-Filters
void CIPFilter::Process()
{
	if(m_iplist.GetCount()==0)
		return;
	if(theApp.ipdlgisopen)
		return; //don't process if user is working on ipfilter
	uint32 lasttick=::GetTickCount();
	static uint32 m_lastcleanup;
	if(lasttick - m_lastcleanup > (1000 * 60 * 60)) //every hour
	{
		m_lastcleanup=lasttick;
		int countall=0;
		int countdel=0;
		for (int i=0;i<m_iplist.GetCount();)
		{
			SIPFilter* search = m_iplist[i];
			if(search->timestamp>0 && ((lasttick - search->timestamp) >=  (1000 * 60 * 60 * 12))) //12 hours
			{
				countdel++;
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

//Xman dynamic IP-Filters
void CIPFilter::AddIPTemporary(uint32 addip)
{
	SIPFilter* newFilter = new SIPFilter;
	newFilter->start = addip;
	newFilter->end = addip;
	newFilter->level = 1;
	newFilter->desc = "temporary";
	newFilter->hits = 0;
	newFilter->timestamp=::GetTickCount();
	m_iplist.Add(newFilter);
	// sort the IP filter list by IP range start addresses
	qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByStartAddr);
}
//Xman end

//Xman auto update IPFilter
// ==> Advanced Updates [MorphXT/Stulle] - Stulle
/*
void CIPFilter::UpdateIPFilterURL()
*/
void CIPFilter::UpdateIPFilterURL(uint32 uNewVersion)
// <== Advanced Updates [MorphXT/Stulle] - Stulle
{
	bool bHaveNewFilterFile = false;
	CString url = thePrefs.GetAutoUpdateIPFilter_URL();
	SYSTEMTIME SysTime;
	if (!url.IsEmpty())
	{
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
		// ==> Advanced Updates [MorphXT/Stulle] - Stulle
		if(thePrefs.IsIPFilterViaDynDNS())
			dlgDownload.m_pLastModifiedTime = NULL;
		else
		// <== Advanced Updates [MorphXT/Stulle] - Stulle
			dlgDownload.m_pLastModifiedTime = &SysTime; //Xman remark: m_pLastModifiedTime is a pointer which points to the SysTime-struct

		if (dlgDownload.DoModal() != IDOK)
		{
			(void)_tremove(strTempFilePath);
			CString strError = GetResString(IDS_DWLIPFILTERFAILED);
			if (!dlgDownload.GetError().IsEmpty())
				strError += _T("\r\n\r\n") + dlgDownload.GetError();
			AfxMessageBox(strError, MB_ICONERROR);
			return;
		}
		// ==> Advanced Updates [MorphXT/Stulle] - Stulle
		/*
		if (dlgDownload.m_pLastModifiedTime == NULL)
		*/
		if (thePrefs.IsIPFilterViaDynDNS() == false && dlgDownload.m_pLastModifiedTime == NULL)
		// <== Advanced Updates [MorphXT/Stulle] - Stulle
			return;

		CString strMimeType;
		GetMimeType(strTempFilePath, strMimeType);

		bool bIsArchiveFile = false;
		bool bUncompressed = false;
		CZIPFile zip;
		if (zip.Open(strTempFilePath))
		{
			bIsArchiveFile = true;

			CZIPFile::File* zfile = zip.GetFile(_T("guarding.p2p"));
			if (zfile == NULL)
				zfile = zip.GetFile(_T("ipfilter.dat"));
			if (zfile)
			{
				CString strTempUnzipFilePath;
				_tmakepathlimit(strTempUnzipFilePath.GetBuffer(_MAX_PATH), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IPFILTER_FILENAME, _T(".unzip.tmp"));
				strTempUnzipFilePath.ReleaseBuffer();

				if (zfile->Extract(strTempUnzipFilePath))
				{
					zip.Close();
					zfile = NULL;

					if (_tremove(theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %hs\n"), theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_trename(strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %hs\n"), strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_tremove(strTempFilePath) != 0)
						TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %hs\n"), strTempFilePath, strerror(errno));
					bUncompressed = true;
					bHaveNewFilterFile = true;
				}
				else {
					CString strError;
					strError.Format(GetResString(IDS_ERR_IPFILTERZIPEXTR), strTempFilePath);
					AfxMessageBox(strError, MB_ICONERROR);
				}
			}
			else {
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
					&& (strFile.CompareNoCase(_T("ipfilter.dat")) == 0 || strFile.CompareNoCase(_T("guarding.p2p")) == 0))
				{
					CString strTempUnzipFilePath;
					_tmakepathlimit(strTempUnzipFilePath.GetBuffer(MAX_PATH), NULL, thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), DFLT_IPFILTER_FILENAME, _T(".unzip.tmp"));
					strTempUnzipFilePath.ReleaseBuffer();
					if (rar.Extract(strTempUnzipFilePath))
					{
						rar.Close();

						if (_tremove(theApp.ipfilter->GetDefaultFilePath()) != 0)
							TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %hs\n"), theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
						if (_trename(strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath()) != 0)
							TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %hs\n"), strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
						if (_tremove(strTempFilePath) != 0)
							TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %hs\n"), strTempFilePath, strerror(errno));
						bUncompressed = true;
						bHaveNewFilterFile = true;
					}
					else
					{
						CString strError;
						strError.Format(_T("Failed to extract IP filter file from RAR file \"%s\"."), strTempFilePath);
						AfxMessageBox(strError, MB_ICONERROR);
					}
				}
				else
				{
					CString strError;
					strError.Format(_T("Failed to find IP filter file \"guarding.p2p\" or \"ipfilter.dat\" in RAR file \"%s\"."), strTempFilePath);
					AfxMessageBox(strError, MB_ICONERROR);
				}
				rar.Close();
			}
			else
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
					gz.Close();

					if (_tremove(theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to remove default IP filter file \"%s\" - %hs\n"), theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_trename(strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath()) != 0)
						TRACE(_T("*** Error: Failed to rename uncompressed IP filter file \"%s\" to default IP filter file \"%s\" - %hs\n"), strTempUnzipFilePath, theApp.ipfilter->GetDefaultFilePath(), strerror(errno));
					if (_tremove(strTempFilePath) != 0)
						TRACE(_T("*** Error: Failed to remove temporary IP filter file \"%s\" - %hs\n"), strTempFilePath, strerror(errno));
					bUncompressed = true;
					bHaveNewFilterFile = true;
				}
				else {
					CString strError;
					strError.Format(GetResString(IDS_ERR_IPFILTERZIPEXTR), strTempFilePath);
					AfxMessageBox(strError, MB_ICONERROR);
				}
			}
			gz.Close();
		}

		if (!bIsArchiveFile && !bUncompressed)
		{
			// Check first lines of downloaded file for potential HTML content (e.g. 404 error pages)
			bool bValidIPFilterFile = true;
			FILE* fp = _tfsopen(strTempFilePath, _T("rb"), _SH_DENYWR);
			if (fp)
			{
				char szBuff[16384];
				int iRead = fread(szBuff, 1, _countof(szBuff)-1, fp);
				if (iRead <= 0)
					bValidIPFilterFile = false;
				else
				{
					szBuff[iRead-1] = '\0';

					const char* pc = szBuff;
					while (*pc == ' ' || *pc == '\t' || *pc == '\r' || *pc == '\n')
						pc++;
					if (strnicmp(pc, "<html", 5) == 0
						|| strnicmp(pc, "<xml", 4) == 0
						|| strnicmp(pc, "<!doc", 5) == 0)
					{
						bValidIPFilterFile = false;
					}
				}
				fclose(fp);
			}

			if (bValidIPFilterFile)
			{
				(void)_tremove(theApp.ipfilter->GetDefaultFilePath());
				VERIFY( _trename(strTempFilePath, theApp.ipfilter->GetDefaultFilePath()) == 0 );
				bHaveNewFilterFile = true;
			}
			else
			{
				AfxMessageBox(GetResString(IDS_DWLIPFILTERFAILED), MB_ICONERROR);
			}
		}
	}
	else
	{
		AfxMessageBox(_T("Failed to auto-update IPFilter. No URL given"), MB_ICONERROR);
		return;
	}

	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	/*
	struct tm tmTemp;
	thePrefs.m_last_ipfilter_check = safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp));
	*/
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

	if (bHaveNewFilterFile)
	{
		LoadFromDefaultFile();
		if (thePrefs.GetFilterServerByIP())
			theApp.emuledlg->serverwnd->serverlistctrl.RemoveAllFilteredServers();
	}

	// In case we received an invalid IP-filter file (e.g. an 404 HTML page with HTTP status "OK"),
	// warn the user that there are no IP-filters available any longer.
	if (bHaveNewFilterFile && theApp.ipfilter->GetIPFilter().GetCount() == 0)
	{
		CString strLoaded;
		strLoaded.Format(GetResString(IDS_IPFILTERLOADED), theApp.ipfilter->GetIPFilter().GetCount());
		CString strError;
		strError.Format(_T("%s\r\n\r\n%s"), GetResString(IDS_DWLIPFILTERFAILED), strLoaded);
		AfxMessageBox(strError, MB_ICONERROR);
		return;
	}

	//everything fine. update the stored version
	if (bHaveNewFilterFile)
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	/*
		memcpy(&thePrefs.m_IPfilterVersion, &SysTime, sizeof SysTime); 
	*/
	{
		thePrefs.m_uIPFilterVersionNum = uNewVersion;
		if(thePrefs.IsIPFilterViaDynDNS())
		{
			memset(&SysTime, 0, sizeof(SYSTEMTIME));
			if(theApp.emuledlg->preferenceswnd &&
				theApp.emuledlg->preferenceswnd->m_wndScar &&
				theApp.emuledlg->preferenceswnd->m_wndScar.m_IpFilterTime)
			{
				CString strBuffer = NULL;
				strBuffer.Format(_T("v%u"), thePrefs.GetIPFilterVersionNum());
				theApp.emuledlg->preferenceswnd->m_wndScar.m_IpFilterTime.SetWindowText(strBuffer);
			}
		}
		else
			memcpy(&thePrefs.m_IPfilterVersion, &SysTime, sizeof SysTime);
	}
	else
	{
		thePrefs.m_uIPFilterVersionNum = 0;
		memset(&SysTime, 0, sizeof(SYSTEMTIME));
		if(thePrefs.IsIPFilterViaDynDNS())
		{
			if(theApp.emuledlg->preferenceswnd &&
				theApp.emuledlg->preferenceswnd->m_wndScar &&
				theApp.emuledlg->preferenceswnd->m_wndScar.m_IpFilterTime)
				theApp.emuledlg->preferenceswnd->m_wndScar.m_IpFilterTime.SetWindowText(GetResString(IDS_DL_NONE));
		}
	}
	if(thePrefs.IsIPFilterViaDynDNS() == false &&
		theApp.emuledlg->preferenceswnd &&
		theApp.emuledlg->preferenceswnd->m_wndScar &&
		theApp.emuledlg->preferenceswnd->m_wndScar.m_IpFilterTime)
	{
		TCHAR sTime[30];
		sTime[0] = _T('\0');
		SysTimeToStr(thePrefs.GetIPfilterVersion(), sTime);
		theApp.emuledlg->preferenceswnd->m_wndScar.m_IpFilterTime.SetWindowText(sTime);
	}
	// <== Advanced Updates [MorphXT/Stulle] - Stulle
}
//Xman end

// ==> Static IP Filter [Stulle] - Stulle
void CIPFilter::AddFromFile2(LPCTSTR pszFilePath)
{
	FILE* readFile = _tfsopen(pszFilePath, _T("r"), _SH_DENYWR);
	if (readFile != NULL)
	{
		_setmode(fileno(readFile), _O_TEXT);

		int iLine = 0;
		CStringA sbuffer;
		CHAR szBuffer[1024];
		while (fgets(szBuffer, _countof(szBuffer), readFile) != NULL)
		{
			iLine++;
			sbuffer = szBuffer;
			
			// ignore comments & too short lines
			if (sbuffer.GetAt(0) == '#' || sbuffer.GetAt(0) == '/' || sbuffer.GetLength() < 5) {
				sbuffer.Trim(" \t\r\n");
				DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter (static): ignored line %u\n", iLine) : 0 );
				continue;
			}

			bool bValid = false;
			uint32 start = 0;
			uint32 end = 0;
			UINT level = 0;
			CStringA desc;
			bValid = ParseFilterLine1(sbuffer, start, end, level, desc);

			// add a filter
			if (bValid)
			{
				AddIPRange(start, end, level, desc);
			}
			else
			{
				sbuffer.Trim(" \t\r\n");
				DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter (static): ignored line %u\n", iLine) : 0 );
			}
		}
		fclose(readFile);

		// sort the IP filter list by IP range start addresses
		qsort(m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByStartAddr);

		// merge overlapping and adjacent filter ranges
		if (m_iplist.GetCount() >= 2)
		{
			// On large IP-filter lists there is a noticeable performance problem when merging the list.
			// The 'CIPFilterArray::RemoveAt' call is way too expensive to get called during the merging,
			// thus we use temporary helper arrays to copy only the entries into the final list which
			// are not get deleted.

			// Reserve a byte array (its used as a boolean array actually) as large as the current 
			// IP-filter list, so we can set a 'to delete' flag for each entry in the current IP-filter list.
			char* pcToDelete = new char[m_iplist.GetCount()];
			memset(pcToDelete, 0, m_iplist.GetCount());
			int iNumToDelete = 0;

			SIPFilter* pPrv = m_iplist[0];
			int i = 1;
			while (i < m_iplist.GetCount())
			{
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
					}
					else
					{
						// if we have identical entries, use the lowest 'level'
						if (pCur->level < pPrv->level)
							pPrv->level = pCur->level;
					}
					delete pCur;
					//m_iplist.RemoveAt(i);	// way too expensive (read above)
					pcToDelete[i] = 1;		// mark this entry as 'to delete'
					iNumToDelete++;
					i++;
					continue;
				}
				pPrv = pCur;
				i++;
			}

			// Create new IP-filter list which contains only the entries from the original IP-filter list
			// which are not to be deleted.
			if (iNumToDelete > 0)
			{
				CIPFilterArray newList;
				newList.SetSize(m_iplist.GetCount() - iNumToDelete);
				int iNewListIndex = 0;
				for (int i = 0; i < m_iplist.GetCount(); i++) {
					if (!pcToDelete[i])
						newList[iNewListIndex++] = m_iplist[i];
				}
				ASSERT( iNewListIndex == newList.GetSize() );

				// Replace current list with new list. Dump, but still fast enough (only 1 memcpy)
				m_iplist.RemoveAll();
				m_iplist.Append(newList);
				newList.RemoveAll();
				m_bModified = true;
			}
			delete[] pcToDelete;
		}
	}
	return;
}

CString CIPFilter::GetDefaultStaticFilePath() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_STATIC_IPFILTER_FILENAME;
}
// <== Static IP Filter [Stulle] - Stulle

// ==> IP Filter White List [Stulle] - Stulle
void CIPFilter::AddFromFileWhite(LPCTSTR pszFilePath)
{
	FILE* readFile = _tfsopen(pszFilePath, _T("r"), _SH_DENYWR);
	if (readFile != NULL)
	{
		_setmode(fileno(readFile), _O_TEXT);

		int iLine = 0;
		CStringA sbuffer;
		CHAR szBuffer[1024];
		while (fgets(szBuffer, _countof(szBuffer), readFile) != NULL)
		{
			iLine++;
			sbuffer = szBuffer;
			
			// ignore comments & too short lines
			if (sbuffer.GetAt(0) == '#' || sbuffer.GetAt(0) == '/' || sbuffer.GetLength() < 5) {
				sbuffer.Trim(" \t\r\n");
				DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter (white): ignored line %u\n", iLine) : 0 );
				continue;
			}

			bool bValid = false;
			uint32 start = 0;
			uint32 end = 0;
			UINT level = 0;
			CStringA desc;
			bValid = ParseFilterLine1(sbuffer, start, end, level, desc);

			// add a filter
			if (bValid)
			{
				AddIPRangeWhite(start, end, level, desc);
				DEBUG_ONLY( TRACE("Added White Entry - start: %u end: %u level: %u desc: %s\n", start, end, level, desc));
			}
			else
			{
				sbuffer.Trim(" \t\r\n");
				DEBUG_ONLY( (!sbuffer.IsEmpty()) ? TRACE("IP filter (white list): ignored line %u\n", iLine) : 0 );
			}
		}
		fclose(readFile);

		// sort the IP filter list by IP range start addresses
		qsort(m_iplist_White.GetData(), m_iplist_White.GetCount(), sizeof(m_iplist_White[0]), CmpSIPFilterByStartAddr);

		// merge overlapping and adjacent filter ranges
		if (m_iplist_White.GetCount() >= 2)
		{
			// On large IP-filter lists there is a noticeable performance problem when merging the list.
			// The 'CIPFilterArray::RemoveAt' call is way too expensive to get called during the merging,
			// thus we use temporary helper arrays to copy only the entries into the final list which
			// are not get deleted.

			// Reserve a byte array (its used as a boolean array actually) as large as the current 
			// IP-filter list, so we can set a 'to delete' flag for each entry in the current IP-filter list.
			char* pcToDelete = new char[m_iplist_White.GetCount()];
			memset(pcToDelete, 0, m_iplist_White.GetCount());
			int iNumToDelete = 0;

			SIPFilter* pPrv = m_iplist_White[0];
			int i = 1;
			while (i < m_iplist_White.GetCount())
			{
				SIPFilter* pCur = m_iplist_White[i];
				if (   pCur->start >= pPrv->start && pCur->start <= pPrv->end	 // overlapping
					|| pCur->start == pPrv->end+1 && pCur->level == pPrv->level) // adjacent
				{
					if (pCur->start != pPrv->start || pCur->end != pPrv->end) // don't merge identical entries
					{
						//TODO: not yet handled, overlapping entries with different 'level'
						if (pCur->end > pPrv->end)
							pPrv->end = pCur->end;
						//pPrv->desc += _T("; ") + pCur->desc; // this may create a very very long description string...
					}
					else
					{
						// if we have identical entries, use the lowest 'level'
						if (pCur->level < pPrv->level)
							pPrv->level = pCur->level;
					}
					delete pCur;
					//m_iplist_White.RemoveAt(i);	// way too expensive (read above)
					pcToDelete[i] = 1;		// mark this entry as 'to delete'
					iNumToDelete++;
					i++;
					continue;
				}
				pPrv = pCur;
				i++;
			}

			// Create new IP-filter list which contains only the entries from the original IP-filter list
			// which are not to be deleted.
			if (iNumToDelete > 0)
			{
				CIPFilterArray newList;
				newList.SetSize(m_iplist_White.GetCount() - iNumToDelete);
				int iNewListIndex = 0;
				for (int i = 0; i < m_iplist_White.GetCount(); i++) {
					if (!pcToDelete[i])
						newList[iNewListIndex++] = m_iplist_White[i];
				}
				ASSERT( iNewListIndex == newList.GetSize() );

				// Replace current list with new list. Dump, but still fast enough (only 1 memcpy)
				m_iplist_White.RemoveAll();
				m_iplist_White.Append(newList);
				newList.RemoveAll();
				m_bModified = true;
			}
			delete[] pcToDelete;
		}
	}
	if(m_iplist_White.GetCount()>0)
		AddLogLine(false, GetResString(IDS_IPFILTERWHITELOADED), m_iplist_White.GetCount());
	return;
}

CString CIPFilter::GetDefaultWhiteFilePath() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_WHITE_IPFILTER_FILENAME;
}
// <== IP Filter White List [Stulle] - Stulle