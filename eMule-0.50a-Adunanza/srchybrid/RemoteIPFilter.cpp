//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
// Tigerjact fecit A.D. MMVI

#include "stdafx.h"
#include <share.h>
#include <fcntl.h>
#include <io.h>
#include "emule.h"
#include "RemoteIPFilter.h"
#include "OtherFunctions.h"
#include "StringConversion.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "Log.h"
#include "HTTPDownloadDlg.h"
#include "AdunanzA.h"
#include "Preferences.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SEC2MS(sec)		((sec)*1000)
#define	MIN2MS(min)		SEC2MS((min)*60)
#define	HR2MS(hr)		MIN2MS((hr)*60)
#define DAY2MS(day)		HR2MS((day)*24)

#define	DFLT_FILTER_LEVEL	100 // default filter level if non specified

#define RemoteIPFilterExpireTime 1209600 //tempo massimo di controllo 2 settimane

UINT CheckIpfilterCallThread(LPVOID lpParameter) {
  try {
		theApp.remoteipfilter->RemoteScaricaECarica();		
  } catch(...) {
  }
  ::AfxEndThread(0);
	return 0;
}




typedef struct {
unsigned int  a;
unsigned int  b;
unsigned int  c;
unsigned int  d;
unsigned int  e;
unsigned int  f;
unsigned int  g;
unsigned int  h;
char i[30];
} strutturaip;

CString CRemoteIPFilter::m_sFilename;
CString CRemoteIPFilter::tmpfile;


CRemoteIPFilter::CRemoteIPFilter()
{
	//m_pLastHit = NULL;
	//m_bModified = false;
	RemoteLoadFromDefaultFile(false);
    
RemoteIPFilterDTF = false;

RemoteIPFilterNextUpdate=thePrefs.m_RemoteIPFilterNextUpdate;
	
//verifico che il nuovo update sia eseguito in un tempo max di RemoteIPFilterExpireTime a partire da ora
if ( RemoteIPFilterNextUpdate > time(NULL) + RemoteIPFilterExpireTime )
	RemoteIPFilterNextUpdate = time(NULL) + RemoteIPFilterExpireTime ;
	::AfxBeginThread(CheckIpfilterCallThread, NULL, 0, 0, 0, NULL);	
}

CRemoteIPFilter::~CRemoteIPFilter()
{
	if (m_bModified)
	{
		try{
			RemoteSaveToDefaultFile();
		}
		catch(CString){
		}
	}
	RemoteRemoveAllIPFilters();
}

static int __cdecl CmpSIPFilterByStartAddr(const void* p1, const void* p2)
{
	const RemoteSIPFilter* rng1 = *(RemoteSIPFilter**)p1;
	const RemoteSIPFilter* rng2 = *(RemoteSIPFilter**)p2;
	return CompareUnsigned(rng1->start, rng2->start);
}

CString CRemoteIPFilter::RemoteGetDefaultFilePath() const
{
	return thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_REMOTEIPFILTER_FILENAME;
}

int CRemoteIPFilter::RemoteLoadFromDefaultFile(bool bShowResponse)
{
	RemoteRemoveAllIPFilters();
	return RemoteAddFromFile(RemoteGetDefaultFilePath(), bShowResponse);
}

int CRemoteIPFilter::RemoteAddFromFile(LPCTSTR pszFilePath, bool bShowResponse)
{
	DWORD dwStart = GetTickCount();
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
			_setmode(fileno(readFile), _O_BINARY);
			static const BYTE _aucP2Bheader[] = "\xFF\xFF\xFF\xFFP2B";
			BYTE aucHeader[sizeof _aucP2Bheader - 1];
			if (fread(aucHeader, sizeof aucHeader, 1, readFile) == 1)
			{
				if (memcmp(aucHeader, _aucP2Bheader, sizeof _aucP2Bheader - 1)==0)
					eFileType = PeerGuardian2;
				else
				{
					fseek(readFile, 0, SEEK_SET);
					_setmode(fileno(readFile), _O_TEXT); // ugly!
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
					// (nVersion == 2) ? OptUtf8ToStr(szName, iLen) : 
					RemoteAddIPRange(uStart, uEnd, DFLT_FILTER_LEVEL, CStringA(szName, iLen));
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
					bValid = RemoteParseFilterLine1(sbuffer, start, end, level, desc);
				else if (eFileType == PeerGuardian)
					bValid = RemoteParseFilterLine2(sbuffer, start, end, level, desc);

				// add a filter
				if (bValid)
				{
					RemoteAddIPRange(start, end, level, desc);
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
			// The 'CRemoteIPFilterArray::RemoveAt' call is way too expensive to get called during the merging,
			// thus we use temporary helper arrays to copy only the entries into the final list which
			// are not get deleted.

			// Reserve a byte array (its used as a boolean array actually) as large as the current 
			// IP-filter list, so we can set a 'to delete' flag for each entry in the current IP-filter list.
			char* pcToDelete = new char[m_iplist.GetCount()];
			memset(pcToDelete, 0, m_iplist.GetCount());
			int iNumToDelete = 0;

			RemoteSIPFilter* pPrv = m_iplist[0];
			int i = 1;
			while (i < m_iplist.GetCount())
			{
				RemoteSIPFilter* pCur = m_iplist[i];
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
				CRemoteIPFilterArray newList;
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
			//tigerjact 
			//AddDebugLogLine(false, _T("Loaded IP filters from \"%s\""), pszFilePath);
			//AddDebugLogLine(false, _T("Parsed lines/entries:%u  Found IP ranges:%u  Duplicate:%u  Merged:%u  Time:%s"), iLine, iFoundRanges, iDuplicate, iMerged, CastSecondsToHM((dwEnd-dwStart+500)/1000));
		}

		
		//elimino splash iniziale
		//AddLogLine(bShowResponse, GetResString(IDS_IPFILTERLOADED), m_iplist.GetCount());
		
	}

	return m_iplist.GetCount();
}

void CRemoteIPFilter::RemoteSaveToDefaultFile()
{
	CString strFilePath = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + DFLT_REMOTEIPFILTER_FILENAME;
	FILE* fp = _tfsopen(strFilePath, _T("wt"), _SH_DENYWR);
	if (fp != NULL)
	{
		for (int i = 0; i < m_iplist.GetCount(); i++)
		{
			const RemoteSIPFilter* flt = m_iplist[i];

			CHAR szStart[16];
			ipstrA(szStart, _countof(szStart), htonl(flt->start));

			CHAR szEnd[16];
			ipstrA(szEnd, _countof(szEnd), htonl(flt->end));

			if (fprintf(fp, "%-15s - %-15s , %3u , %s\n", szStart, szEnd, flt->level, flt->desc) == 0 || ferror(fp))
			{
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

bool CRemoteIPFilter::RemoteParseFilterLine1(const CStringA& sbuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& desc) const
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

bool CRemoteIPFilter::RemoteParseFilterLine2(const CStringA& sbuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& desc) const
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

void CRemoteIPFilter::RemoteRemoveAllIPFilters()
{
	for (int i = 0; i < m_iplist.GetCount(); i++)
		delete m_iplist[i];
	m_iplist.RemoveAll();
	m_pLastHit = NULL;
}

bool CRemoteIPFilter::RemoteIsFiltered(uint32 ip) /*const*/
{
	return RemoteIsFiltered(ip, thePrefs.GetIPFilterLevel());
}

static int __cdecl CmpSIPFilterByAddr(const void* pvKey, const void* pvElement)
{
	uint32 ip = *(uint32*)pvKey;
	const RemoteSIPFilter* pIPFilter = *(RemoteSIPFilter**)pvElement;

	if (ip < pIPFilter->start)
		return -1;
	if (ip > pIPFilter->end)
		return 1;
	return 0;
}

bool CRemoteIPFilter::RemoteIsFiltered(uint32 ip, UINT level) /*const*/
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
	RemoteSIPFilter** ppFound = (RemoteSIPFilter**)bsearch(&ip, m_iplist.GetData(), m_iplist.GetCount(), sizeof(m_iplist[0]), CmpSIPFilterByAddr);
	if (ppFound && (*ppFound)->level < level)
	{
		(*ppFound)->hits++;
		m_pLastHit = *ppFound;
		return true;
	}

	return false;
}

CString CRemoteIPFilter::GetLastHit() const
{
	return m_pLastHit ? CString(m_pLastHit->desc) : _T("Not available");
}

const CRemoteIPFilterArray& CRemoteIPFilter::GetIPFilter() const
{
	return m_iplist;
}

bool CRemoteIPFilter::RemoteRemoveIPFilter(const RemoteSIPFilter* pFilter)
{
	for (int i = 0; i < m_iplist.GetCount(); i++)
	{
		if (m_iplist[i] == pFilter)
		{
			delete m_iplist[i];
			m_iplist.RemoveAt(i);
			return true;
		}
	}
	return false;
}

void DoEvents() //By Anis
{ 
    MSG msg; 
 
    while ( ::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE ) ) 
    { 
        if ( ::GetMessage(&msg, NULL, 0, 0)) 
        { 
            ::TranslateMessage(&msg); 
            :: DispatchMessage(&msg); 
        } 
        else 
            break; 
    } 
} 

void Pause(long int Delay) { //Anis -> alternativa dello sleep prensente nella libreria kernel32. A differenza dell'altra, questa non blocca l'applicazione.
    long int TmrI;
    TmrI = 0;
	do {
        TmrI++;
        DoEvents();
        Sleep(1);
	} while(TmrI < Delay);
}

//scarico e carico il file
//Anis -> che tabulazione del codice da 4 soldi. Sistemata.
void CRemoteIPFilter::RemoteScaricaECarica()
{
m_sFilename = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("adu_remoteipfilter.dat");
tmpfile = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("tmp_adu_remoteipfilter.dat");

strutturaip prendoip;

	while(true){

		DoEvents(); //Anis -> spero questo risolva il crash all'avvio in definitiva che appare nel 2% dei casi.
rifai:
		try {
		
		if (RemoteIPFilterDTF) {//se devo aggiornare scarico il file
			
		//AddDebugLogLine(false, _T("SCARICO IPFILTER"));
			
			CHttpDownloadDlg dlgDownload;
			dlgDownload.m_sURLToDownload = ADU_REMOTE_FILTER;
			dlgDownload.m_sFileToDownloadInto = m_sFilename;
			if (dlgDownload.DoModal() == IDOK){
				  FILE* tmpfile1 = _tfsopen(m_sFilename, _T("r"), _SH_DENYWR);
				  FILE* tmpfile2 = _tfsopen(tmpfile, _T("w"), _SH_DENYWR);

						if ((tmpfile1 != NULL) && (tmpfile2 != NULL)){
							//Anis -> tabulazione del codice pari a 0. ok che non si programma in Whitespace ma un minimo.
							while(fread(&prendoip, sizeof(strutturaip),1, tmpfile1)!=0)
								fprintf(tmpfile2, "%u.%u.%u.%u - %u.%u.%u.%u , 0, %s\n", prendoip.a, prendoip.b, prendoip.c, prendoip.d, prendoip.e, prendoip.f, prendoip.g, prendoip.h, prendoip.i );
							fclose(tmpfile1);
							fclose(tmpfile2);
							RemoteLoadFromDefaultFile();
							DeleteFile(tmpfile);
							RemoteIPFilterNextUpdate = time(NULL) + RemoteIPFilterExpireTime;
							thePrefs.m_RemoteIPFilterNextUpdate = RemoteIPFilterNextUpdate;
							RemoteIPFilterDTF=false;  
							aspetta(true);
							RemoteIPFilterDTF=true;
						}			
						else //non riesco a leggere i file temporanei
						{	
							RemoteIPFilterDTF=true;
							fclose(tmpfile2); 
							DeleteFile(tmpfile);
						}
				}
				//else relativo a idok, se nn scarica il file
				else	
					aspetta(false);

		}
		else{//se non devo scaricare il file
			FILE* tmpfile1 = _tfsopen(m_sFilename, _T("r"), _SH_DENYWR);
			FILE* tmpfile2 = _tfsopen(tmpfile, _T("w"), _SH_DENYWR);

						if ((tmpfile1 != NULL) && (tmpfile2 != NULL)){
							while(fread(&prendoip, sizeof(strutturaip),1, tmpfile1)!=0)
								fprintf(tmpfile2, "%u.%u.%u.%u - %u.%u.%u.%u , 0, %s\n", prendoip.a, prendoip.b, prendoip.c, prendoip.d, prendoip.e, prendoip.f, prendoip.g, prendoip.h, prendoip.i );
							fclose(tmpfile1);
							fclose(tmpfile2);
							RemoteLoadFromDefaultFile();
							DeleteFile(tmpfile);
							aspetta(true);
							RemoteIPFilterDTF=true;
						}			
						else //non riesco a leggere i file temporanei	
						{	
							fclose(tmpfile2); 
							DeleteFile(tmpfile);
							RemoteIPFilterDTF=true;
						}
		}

		}
		catch(...) {
			//Anis -> caso crash per motivo ignoto se vuole crashare gli e lo impedisco con un bel catch e una bastardata. u.u
			goto rifai;
		}

}
}


void CRemoteIPFilter::aspetta(bool idfile){
	bool ipfwait= true;

     if (idfile){
		 while(ipfwait) {
			if (time(NULL) < RemoteIPFilterNextUpdate){
				//non devo aggiornare il file, aspetto 1 ora prima di ricontrollare
				try { 
					Sleep(HR2MS(1));
					} catch(...) {}
			}
            else {//aggiorno il file
				ipfwait=false; 
				}
		 }
	 }
	else
		try { 
				Sleep(MIN2MS(30));
	} catch(...) {}
	}

