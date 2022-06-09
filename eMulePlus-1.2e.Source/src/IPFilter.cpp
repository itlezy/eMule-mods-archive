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
#include "emule.h"
#include "IPFilter.h"
#include "otherfunctions.h"
#include "Preferences.h"
#include "emuleDlg.h"
#include "HttpDownloadDlg.h"
#include <share.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define DFLT_FILTER_LEVEL	100 // default filter level if non specified
#define DFLT_TICKS_TO_BAN	(24u * 60u * 60u * 1000u)	//default ban time
#define IPF_STRPACK_SCALE	1u	// string alignment (shift) in the combined array

/////////////////////////////////////////////////////////////////////////////////////////////
CIPFilter::CIPFilter()
{
	m_pLastHit = NULL;
	m_dwLastFilteredIP = 0;
	m_pcDesc = NULL;
	m_acTempLastHit[0] = '\0';
}
/////////////////////////////////////////////////////////////////////////////////////////////
CIPFilter::~CIPFilter()
{
	RemoveAllIPs();
	m_TempIPList.clear();
}
/////////////////////////////////////////////////////////////////////////////////////////////
static int __cdecl CmpSIPFilterByStartAddr(const void* p1, const void* p2)
{
	const SIPFilter	*rng1 = (SIPFilter*)p1;
	const SIPFilter	*rng2 = (SIPFilter*)p2;

	return CompareUnsigned(rng1->dwStart, rng2->dwStart);
}
/////////////////////////////////////////////////////////////////////////////////////////////
int CIPFilter::LoadFromDefaultFile(bool bShowResponse/*=true*/)
{
	RemoveAllIPs();
	m_dwLastFilteredIP = 0;

	return AddFromFile(g_App.m_pPrefs->GetConfigDir() + DFLT_IPFILTER_FILENAME, bShowResponse);
}
/////////////////////////////////////////////////////////////////////////////////////////////
int CIPFilter::AddFromFile(LPCTSTR pstrFilePath, bool bShowResponse)
{
	EMULE_TRY

	SIPFilter	newFilter;
	POSITION	pos;
	FILE		*pReadFile = _tfsopen(pstrFilePath, _T("r"), _SH_DENYWR);

	if (pReadFile != NULL)
	{
		enum EIPFilterFileType
		{
			Unknown = 0,
			FilterDat = 1,		// ipfilter.dat/ip.prefix format
			PeerGuardian = 2	// PeerGuardian format
		} eFileType = Unknown;

		TCHAR	strNam[_MAX_FNAME];
		TCHAR	strExt[_MAX_EXT];

		_tsplitpath(pstrFilePath, NULL, NULL, strNam, strExt);
		if (_tcsicmp(strExt, _T(".p2p")) == 0 || (_tcsicmp(strNam, _T("guarding.p2p")) == 0 && _tcsicmp(strExt, _T(".txt")) == 0))
			eFileType = PeerGuardian;
		else if (_tcsicmp(strExt, _T(".prefix")) == 0)
			eFileType = FilterDat;

		int			iLine = 0, iFoundRanges = 0;
		int			iDuplicate = 0, iMerged = 0;
		CStringA	strBuf;
		char		acBuffer[1024];
		unsigned	uiLen, uiTmpSz, uiDescIdx = 0;
		CRBMap<CStringA, unsigned>				rbmapDescIdx(1024);
		const CRBMap<CStringA, unsigned>::CPair	*pPair;
		CIPFilterArray							aIPTmp;

		setvbuf(pReadFile, NULL, _IOFBF, 64*1024);
		while (fgets(acBuffer, ARRSIZE(acBuffer), pReadFile) != NULL)
		{
			iLine++;
			if ((*acBuffer == '#') || (*acBuffer == '/'))	// Ignore comments
				continue;
		//	Ignore too short lines
			if ((uiLen = fast_strlen(acBuffer)) < 5)
				continue;

			if (eFileType == Unknown)
			{
				strBuf.SetString(acBuffer, uiLen);
			//	Looks like html
				if ((strBuf.Find('>') >= 0) && (strBuf.Find('<') >= 0))
					strBuf.Delete(0, strBuf.ReverseFind('>') + 1);

			//	Check for <IP> - <IP> at start of line
				UINT	u1, u2, u3, u4, u5, u6, u7, u8;

				if (sscanf(strBuf, "%u.%u.%u.%u - %u.%u.%u.%u", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) == 8)
					eFileType = FilterDat;
				else
				{
				//	Check for <description> ':' <IP> '-' <IP>
					int iColon = strBuf.Find(':');
					if (iColon >= 0)
					{
						const char	*pcIPRange = strBuf.GetString() + iColon + 1;

						if (sscanf(pcIPRange, "%u.%u.%u.%u - %u.%u.%u.%u", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) == 8)
							eFileType = PeerGuardian;
					}
				}
			}

			bool	bValid = false;
			UINT	uiLevel;

			if (eFileType == FilterDat)
				bValid = ParseFilterLine1(acBuffer, uiLen, &strBuf, newFilter.dwStart, newFilter.dwEnd, uiLevel);
			else if (eFileType == PeerGuardian)
			{
				strBuf.SetString(acBuffer, uiLen);
				bValid = ParseFilterLine2(strBuf, newFilter.dwStart, newFilter.dwEnd, uiLevel);
			}

		//	Add a filter
			if (bValid)
			{
			//	Keep only unique strings to reduce memory consumption
				pPair = rbmapDescIdx.Lookup(strBuf);
				if (pPair != NULL)	//duplicate string found, use its index
					newFilter.uiPacked = (pPair->m_value << 8) | (uiLevel & 0xFF);
				else
				{
					if ((uiDescIdx & ~0xFFFFFF) != 0)
						g_App.AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("IP filter: reached internal buffer limit - IP range description can be wrong"));
					rbmapDescIdx.SetAt(strBuf, uiDescIdx);
					newFilter.uiPacked = (uiDescIdx << 8) | (uiLevel & 0xFF);
				// Aligned to SCALE char boundary to keep more strings in the packed field
					uiDescIdx += ((strBuf.GetLength() + 1 + (1 << IPF_STRPACK_SCALE) - 1) >> IPF_STRPACK_SCALE);
				}
				aIPTmp.push_back(newFilter);
				iFoundRanges++;
			}
			else
				g_App.AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("IP filter: parsing error - line %u ignored"), iLine);
		}
		fclose(pReadFile);

	//	Sort the IP filter list by IP range start addresses
		uiTmpSz = aIPTmp.size();
		qsort(&aIPTmp[0], uiTmpSz, sizeof(aIPTmp[0]), CmpSIPFilterByStartAddr);

	//	Build linear array containing all unique strings
		if (uiDescIdx != 0)
		{
			m_pcDesc = new char[uiDescIdx << IPF_STRPACK_SCALE];
			pos = rbmapDescIdx.GetHeadPosition();
			while (pos != NULL)
			{
				pPair = rbmapDescIdx.GetNext(pos);

				memcpy(m_pcDesc + (pPair->m_value << IPF_STRPACK_SCALE),
					pPair->m_key.GetString(), pPair->m_key.GetLength() + 1);
			}
		}

	//	Merge overlapping and adjacent filter ranges
		if (uiTmpSz >= 2)
		{
			SIPFilter	*pPrv = &aIPTmp[0];
			unsigned	ui = 1, uiNumToDelete = 0;
			byte		*pbyteToDelete = new byte[uiTmpSz];

			memzero(pbyteToDelete, uiTmpSz);
			while (ui < uiTmpSz)
			{
				SIPFilter	*pCur = &aIPTmp[ui];

				if ( ( ((pCur->dwStart >= pPrv->dwStart) && (pCur->dwStart <= pPrv->dwEnd))		// overlapping
					|| (pCur->dwStart == pPrv->dwEnd + 1) ) && (((pCur->uiPacked ^ pPrv->uiPacked) & 0xFF) == 0) )// adjacent ranges (plus the same level)
				{
					if (pCur->dwStart != pPrv->dwStart || pCur->dwEnd != pPrv->dwEnd) // don't merge identical entries
					{
					//	TODO: not yet handled, overlapping entries with different 'level'
						if (pCur->dwEnd > pPrv->dwEnd)
							pPrv->dwEnd = pCur->dwEnd;
						iMerged++;
					}
					else
					{
					//	If we have identical entries, use the lowest 'level'
						if ((pCur->uiPacked & 0xFF) < (pPrv->uiPacked & 0xFF))
							pPrv->uiPacked = (pPrv->uiPacked & ~0xFF) | (pCur->uiPacked & 0xFF);
						iDuplicate++;
					}
					pbyteToDelete[ui] = 1;		// mark this entry as 'to delete'
					uiNumToDelete++;
					ui++;
					continue;
				}
				pPrv = pCur;
				ui++;
			}
		//	Rebuild list keeping only required (and combined) entries of the filter
			if (uiNumToDelete != 0)
			{
			//	Reserve exact amount to avoid memory loss on automatic container allocation
				m_IPArr.reserve(uiTmpSz - uiNumToDelete);
				for (unsigned ui = 0; ui < uiTmpSz; ui++)
				{
					if (pbyteToDelete[ui] == 0)
						m_IPArr.push_back(aIPTmp[ui]);
				}
			}
			else
				m_IPArr = aIPTmp;
			delete[] pbyteToDelete;
		}
		else
			m_IPArr = aIPTmp;

		g_App.AddLogLine((bShowResponse ? LOG_FL_SBAR : 0) | LOG_RGB_SUCCESS, IDS_IPFILTERLOADED, m_IPArr.size());
		if (g_App.m_pPrefs->GetVerbose())
		{
			g_App.AddLogLine(LOG_FL_DBG, _T("Loaded IP filters from \"%s\""), pstrFilePath);
			g_App.AddLogLine(LOG_FL_DBG, _T("Parsed lines: %u, Found IP ranges: %u, Duplicate: %u, Merged: %u"), iLine, iFoundRanges, iDuplicate, iMerged);
		}
	}

	EMULE_CATCH

	return m_IPArr.size();
}
/////////////////////////////////////////////////////////////////////////////////////////////
static int OptimReverseFindA(const CStringA &strProc, char cCh)
{
	const char *pcBeg = strProc.GetString();
	const char *pcEnd = pcBeg + strProc.GetLength();

	while (--pcEnd >= pcBeg)
	{
		if (*pcEnd == cCh)
			break;
	}
	return static_cast<int>(pcEnd - pcBeg);
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool CIPFilter::ParseFilterLine1(const char *pcLine, unsigned uiLineLen, CStringA *pstrOut, uint32 &rdwIP1, uint32 &rdwIP2, UINT &ruiLevel) const
{
	UINT	u1, u2, u3, u4, u5, u6, u7, u8, uiLevel = DFLT_FILTER_LEVEL;
	int		iDescStart, iDescLen, iItems;

	iItems = sscanf(pcLine, "%u.%u.%u.%u - %u.%u.%u.%u , %u , %n", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8, &uiLevel, &iDescStart);
	if (iItems < 8)
		return false;

	if ((u1 | u2 | u3 | u4 | u5 | u6 | u7 | u8) > 255u)	//check for valid IP address
		return false;
	rdwIP1 = (u1 << 24) | (u2 << 16) | (u3 << 8) | u4;
	rdwIP2 = (u5 << 24) | (u6 << 16) | (u7 << 8) | u8;

	if (iItems == 8)
	{
		ruiLevel = DFLT_FILTER_LEVEL;	// set default level
		pstrOut->Truncate(0);
		return true;	 // no description available
	}
	ruiLevel = uiLevel;

	const char	*pcDescStart = pcLine + iDescStart;

	iDescLen = uiLineLen - iDescStart;
	while ( (--iDescLen >= 0) &&	// strip trailing whitespaces
		( ((static_cast<unsigned>(pcDescStart[iDescLen]) - 0x09u) <= (0x0Du - 0x09u))
			|| (pcDescStart[iDescLen] == ' ') ) )
	{}
	iDescLen++;
	memcpy(pstrOut->GetBufferSetLength(iDescLen), pcDescStart, iDescLen * sizeof(*pcDescStart));

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool CIPFilter::ParseFilterLine2(CStringA &rstrBuffer, uint32 &rdwIP1, uint32 &rdwIP2, UINT &ruiLevel) const
{
	int iPos = ::OptimReverseFindA(rstrBuffer, ':');
	if (iPos <= 0)
		return false;

	const char	*pcIPRange = rstrBuffer.GetString() + iPos + 1;
	UINT		u1, u2, u3, u4, u5, u6, u7, u8;

	if (sscanf(pcIPRange, "%u.%u.%u.%u - %u.%u.%u.%u", &u1, &u2, &u3, &u4, &u5, &u6, &u7, &u8) != 8)
		return false;

	if ((u1 | u2 | u3 | u4 | u5 | u6 | u7 | u8) > 255u)	//check for valid IP address
		return false;
	rdwIP1 = (u1 << 24) | (u2 << 16) | (u3 << 8) | u4;
	rdwIP2 = (u5 << 24) | (u6 << 16) | (u7 << 8) | u8;

	ruiLevel = DFLT_FILTER_LEVEL;

	rstrBuffer.Truncate(iPos);
	rstrBuffer.Replace("PGIPDB", "");
	rstrBuffer.Trim();

	return true;
}
/////////////////////////////////////////////////////////////////////////////////////////////
void CIPFilter::RemoveAllIPs()
{
	EMULE_TRY

	m_IPArr.clear();
	delete[] m_pcDesc;
	m_pcDesc = NULL;
	m_pLastHit = NULL;

	EMULE_CATCH
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool CIPFilter::IsCachedAndFiltered(uint32 dwIP)
{
	if (m_dwLastFilteredIP == dwIP
		|| IsFiltered(dwIP, g_App.m_pPrefs->GetIPFilterLevel()) 
		|| IsTemporaryFiltered(dwIP))
	{
		m_dwLastFilteredIP = dwIP;
		return true;
	}
	else
		return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool CIPFilter::IsFiltered(uint32 dwIP)
{
	return (IsFiltered(dwIP, g_App.m_pPrefs->GetIPFilterLevel()) || IsTemporaryFiltered(dwIP));
}
/////////////////////////////////////////////////////////////////////////////////////////////
static int __cdecl CmpSIPFilterByAddr(const void *pvKey, const void *pvElement)
{
	uint32	dwIP = *(uint32*)pvKey;
	const SIPFilter	*pIPFilter = (SIPFilter*)pvElement;

	if (dwIP < pIPFilter->dwStart)
		return -1;
	return (dwIP > pIPFilter->dwEnd) ? 1 : 0;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool CIPFilter::IsFiltered(uint32 dwIP, UINT uiLevel)
{
	EMULE_TRY

	if (m_IPArr.size() == 0 || dwIP == 0)
		return false;

	dwIP = fast_htonl(dwIP);

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
	SIPFilter	*pFound = (SIPFilter*)bsearch(&dwIP, &m_IPArr[0], m_IPArr.size(), sizeof(m_IPArr[0]), CmpSIPFilterByAddr);

	if ((pFound != NULL) && ((pFound->uiPacked & 0xFFu) < uiLevel))
	{
		m_pLastHit = pFound;
		memcpy(m_acTempLastHit, "N/A", sizeof("N/A"));
		return true;
	}

	EMULE_CATCH

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool CIPFilter::IsTemporaryFiltered(uint32 dwIP)
{
	EMULE_TRY

	if (m_TempIPList.size() == 0 || dwIP == 0)
		return false;

	hash_map<uint32, uint32>::iterator it = m_TempIPList.find(dwIP);

	if (it != m_TempIPList.end())
	{
		uint32 dwTicksBanned = ::GetTickCount() - it->second;

		if (dwTicksBanned > DFLT_TICKS_TO_BAN)
		{
			m_TempIPList.erase(it->first);
			if (dwIP == m_dwLastFilteredIP)
				m_dwLastFilteredIP = 0;
			return false;
		}
		else
		{
			uint32	dwSecLeft = (DFLT_TICKS_TO_BAN - dwTicksBanned) / 1000;
			uint32	dwHours = dwSecLeft / 3600u;
			uint32	dwMinSecs = dwSecLeft - dwHours * 3600u;

			_snprintf( m_acTempLastHit, sizeof(m_acTempLastHit),
				"temporarily blacklisted (%u:%02u:%02u remaining)",
				dwHours, dwMinSecs / 60u, dwMinSecs - (dwMinSecs / 60u) * 60u );
			m_acTempLastHit[ARRSIZE(m_acTempLastHit) - 1] = '\0';	//ensure proper termination
			m_pLastHit = NULL;
			return true;
		}
	}
	else
		return false;

	EMULE_CATCH

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
const char* CIPFilter::GetLastHit() const
{
	return (m_pLastHit != NULL) ? m_pcDesc + ((m_pLastHit->uiPacked >> 8u) << IPF_STRPACK_SCALE) : m_acTempLastHit;
}
/////////////////////////////////////////////////////////////////////////////////////////////
bool CIPFilter::DownloadIPFilter()
{
	EMULE_TRY

	int		iRc;
	bool	bSuccess = false;
	CString	strIPFilterURL = g_App.m_pPrefs->GetIPFilterURL();

	if (!strIPFilterURL.IsEmpty())
	{
		CHttpDownloadDlg dlgDownload;

		dlgDownload.m_strInitializingTitle	= GetResString(IDS_IPFILTER_HTTP_TITLE);
		dlgDownload.m_nIDPercentage			= IDS_IPFILTER_HTTP_PERCENTAGE;
		dlgDownload.m_sURLToDownload		= strIPFilterURL;
		dlgDownload.m_sFileToDownloadInto	= g_App.m_pPrefs->GetConfigDir() + DFLT_IPFILTER_FILENAME;

		CString strBackupFilePath = dlgDownload.m_sFileToDownloadInto + _T(".bak");

	//	Removes any existing backup file
		_tremove(strBackupFilePath);
	//	Backups ipfilter.dat in case the download fails
		MoveFile(dlgDownload.m_sFileToDownloadInto, strBackupFilePath);

		iRc = dlgDownload.DoModal();
		for (;;)
		{
			if (iRc == IDOK)
			{
				bSuccess = (LoadFromDefaultFile() != 0);	//check that something was loaded
				if (bSuccess)
				{
					_tremove(strBackupFilePath);	//	Delete backup file
					break;
				}
			}
			g_App.AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_IPFILTER_UPDATE_ERROR);

		//	Restores original file
			_tremove(dlgDownload.m_sFileToDownloadInto);
			MoveFile(strBackupFilePath, dlgDownload.m_sFileToDownloadInto);
			if (iRc == IDOK)	//	Reload previous IP filter file if invalid file was received
				LoadFromDefaultFile();
			break;
		}
	}

	return bSuccess;

	EMULE_CATCH

	return false;
}
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
