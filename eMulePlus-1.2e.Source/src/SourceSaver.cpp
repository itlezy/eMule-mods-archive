#include "stdafx.h"
#include "updownclient.h"
#include "SourceSaver.h"
#include "PartFile.h"
#ifndef NEW_SOCKETS_ENGINE
#include "emule.h"
#else
#include "otherfunctions.h"
#endif //NEW_SOCKETS_ENGINE

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CSourceData
{
public:
	CSourceData(uint32 dwIP, uint16 uPort, LPCTSTR pachExpDate);
#ifndef NEW_SOCKETS_ENGINE
	CSourceData(CUpDownClient *pClient, LPCTSTR pachExpDate);
#endif //NEW_SOCKETS_ENGINE

	bool Compare(const CSourceData *pSourceData);

	uint32	m_dwSrcIDHyb;
	uint32	m_dwPartsAvailable;
	uint16	m_uSourcePort;
	TCHAR	m_pachExpDate[7];
};

#ifndef NEW_SOCKETS_ENGINE
CSourceData::CSourceData(CUpDownClient *pClient, LPCTSTR pachExpDate)
{
	m_dwSrcIDHyb = pClient->GetUserIDHybrid();
	m_uSourcePort = pClient->GetUserPort();
	m_dwPartsAvailable = pClient->GetAvailablePartCount();
	_tcscpy(m_pachExpDate, pachExpDate);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif //NEW_SOCKETS_ENGINE
CSourceData::CSourceData(uint32 dwIP, uint16 uPort, LPCTSTR pachExpDate)
{
	m_dwSrcIDHyb = ntohl(dwIP);
	m_uSourcePort = uPort;
	m_dwPartsAvailable = 0;
	_tcscpy(m_pachExpDate, pachExpDate);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CSourceData::Compare(const CSourceData *pSrcData)
{
	return ( (m_dwSrcIDHyb == pSrcData->m_dwSrcIDHyb)
		&& (m_uSourcePort == pSrcData->m_uSourcePort) );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CSourceSaver::CSourceSaver()
{
	m_dwLastTimeSaved = ::GetTickCount() - (rand() >> 1);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	Process() saves/loads up to 'iMaxSourcesToSave' sources for 'pPartFile' if the auto save/load timer
//		have expired. It returns false if the sources were not saved.
bool CSourceSaver::Process(CPartFile *pPartFile, int iMaxSourcesToSave/* = 10*/, bool bIgnoreTimer/* = false */)
{
	uint32	dwTicks = ::GetTickCount();

//	If the auto-save time (fixed 120 minutes) has passed...
	if (bIgnoreTimer || ((dwTicks - m_dwLastTimeSaved) > RESAVETIME))
	{
	//	Set next auto-save time + random 0..16 seconds
		m_dwLastTimeSaved = dwTicks - (rand() >> 1);
		SaveSources(pPartFile, iMaxSourcesToSave);

		return true;
	}

	return false;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadSources() loads the sources for part file 'pPartFile' and adds them to the download queue
void CSourceSaver::LoadSources(CPartFile *pPartFile)
{
//	Load sources from file
	SourceList	srcList;

	LoadSourcesFromFile(pPartFile, &srcList);

#ifndef NEW_SOCKETS_ENGINE
	CSourceData		*pSrcData;
	CUpDownClient	*pNewSrc;
	uint32			dwAddedSources = 0;

//	For each SourceData...
	while (srcList.GetCount() > 0)
	{
		pSrcData = srcList.RemoveHead();

	//	If we reach the maximum number of sources allowed, stop adding more
		if (pPartFile->GetSourceCount() < g_App.m_pPrefs->GetMaxSourcePerFile())
		{
		 	pNewSrc = g_App.m_pDownloadQueue->CheckAndAddSource(pPartFile, pSrcData->m_dwSrcIDHyb, pSrcData->m_uSourcePort, 0, 0, NULL);

			if (pNewSrc != NULL)
			{
				pNewSrc->SetUserName(GetResString(IDS_SAVED_SOURCE));
				dwAddedSources++;
			}
		}
		delete pSrcData;
	}

	if (dwAddedSources != 0)
		g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T("Loaded %u sources for file %s"), dwAddedSources, pPartFile->GetFileName());
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadSourcesFromFile() loads the sources for part file 'pPartFile' from the sources file 'srcFile'
//		and adds them to the source list 'pSrcList'.
void CSourceSaver::LoadSourcesFromFile(CPartFile *pPartFile, SourceList *pSrcList)
{
//	Sources filename is .met filename with leading .txtsrc in temp directory
	CStdioFile	srcFile;
	CString		strSrcFile = pPartFile->GetFilePath();

	strSrcFile += _T(".txtsrc");

//	Open the sources file. If it can't be opened, just return.
	if (!srcFile.Open(strSrcFile, CFile::modeRead | CFile::shareDenyWrite | CFile::typeText))
		return;

	int		iPos;
	CString	strLine, strExpDate;
	uint32	dwIP;
	uint16	uPort;

//	Until we hit the EOF...
	while (srcFile.ReadString(strLine))
	{
	//	If it's a comment or an empty line, skip it
		if (strLine.IsEmpty() || strLine.GetAt(0) == _T('#'))
			continue;

	//	If the line doesn't contain the ':' for an ip:port pair, skip it
		if ((iPos = strLine.Find(_T(':'))) < 0)
			continue;

	//	Strip off the IP and convert it to a 32bit integer
	//	If the IP is invalid, skip the line
		if ((dwIP = inet_addr(strLine.Left(iPos))) == INADDR_NONE)
			continue;

		strLine = strLine.Mid(iPos + 1);

	//	If the line doesn't contain the ',' seperating the date, skip it
		if ((iPos = strLine.Find(_T(','))) < 0)
			continue;

	//	Strip off the port and convert it to an integer
	//	If the port is invalid, skip the line
		if ((uPort = static_cast<uint16>(_tstoi(strLine.Left(iPos)))) == 0)
			continue;

		strLine = strLine.Mid(iPos + 1);

	//	If the line doesn't contain the terminating ';', skip it
		if ((iPos = strLine.Find(_T(';'))) < 0)
			continue;

	//	Strip off the expiration date
		strExpDate = strLine.Left(iPos);

		if (IsExpired(strExpDate))
			continue;

	//	If we got a valid, unexpired source, add it to the 'srcList' list
		pSrcList->AddTail(new CSourceData(dwIP, uPort, strExpDate));
	}
	srcFile.Close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	SaveSources() writes to file 'srcFile' a list of up to 'iMaxSourcesToSave' sources composed of the
//		currently available sources with the highest number of available parts and the previously saved sources if needed.
void CSourceSaver::SaveSources(CPartFile *pPartFile, int iMaxSourcesToSave)
{
#ifndef NEW_SOCKETS_ENGINE
	POSITION		pos1, pos2;
	SourceList		srcList(iMaxSourcesToSave);
	CSourceData		*pSrcData;
	CString			strBuf1 = CalcExpiration(EXPIREIN);
	ClientList		clientListCopy;
	CUpDownClient	*pCurSrc;

//	Choose best sources for the file
	pPartFile->GetCopySourceLists(SLM_ALLOWED_TO_SAVE, &clientListCopy);
	for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
	{
		pCurSrc = *cIt;
		if ((pCurSrc == NULL) || pCurSrc->HasLowID())	//skip LowID sources
			continue;

		if (srcList.IsEmpty())
			srcList.AddHead(new CSourceData(pCurSrc, strBuf1));
	//	If the list isn't full or the source has more parts than the lowest in the list...
		else if ((srcList.GetCount() <= iMaxSourcesToSave) || (pCurSrc->GetAvailablePartCount() > srcList.GetTail()->m_dwPartsAvailable))
		{
		//	If the list is full, remove the source with the lowest number of parts
			if (srcList.GetCount() == iMaxSourcesToSave)
				delete srcList.RemoveTail();

			ASSERT(srcList.GetCount() < iMaxSourcesToSave);

		//	Add the new source to the list in "# avail parts" order
			bool	bInserted = false;

		//	For each SourceData, (backwards starting with the last)...
			for (pos1 = srcList.GetTailPosition(); (pos2 = pos1) != NULL;)
			{
				pSrcData = srcList.GetPrev(pos1);

				if (pSrcData->m_dwPartsAvailable > pCurSrc->GetAvailablePartCount())
				{
					srcList.InsertAfter(pos2, new CSourceData(pCurSrc, strBuf1));
					bInserted = true;
					break;
				}
			}
			if (!bInserted)
				srcList.AddHead(new CSourceData(pCurSrc, strBuf1));
		}
	}

//	Add A4AF sources if number of source is less than the limit
	if (srcList.GetCount() < iMaxSourcesToSave)
	{
		pPartFile->GetCopyA4AFSourceList(&clientListCopy);
		for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
		{
			pCurSrc = *cIt;
			if ((pCurSrc == NULL) || pCurSrc->HasLowID())	//skip LowID sources
				continue;
			srcList.AddTail(new CSourceData(pCurSrc, strBuf1));
		}
	}

//	Add previously saved sources if found sources does not reach the limit
	if (srcList.GetCount() < iMaxSourcesToSave)
	{
		SourceList	listOldSources(iMaxSourcesToSave);

		LoadSourcesFromFile(pPartFile, &listOldSources);

		bool	bFound;

		while (!listOldSources.IsEmpty())
		{
			pSrcData = listOldSources.RemoveHead();

			if (srcList.GetCount() < iMaxSourcesToSave)
			{
				bFound = false;

				for (pos1 = srcList.GetHeadPosition(); pos1 != NULL;)
				{
					if (srcList.GetNext(pos1)->Compare(pSrcData))
					{
						bFound = true;
						break;
					}
				}

				if (!bFound)
				{
					srcList.AddTail(pSrcData);
					pSrcData = NULL;	// Prevent deletion of needed object
				}
			}
			delete pSrcData;
		}
	}

	CStdioFile	srcFile;
	CString		strBuf2 = pPartFile->GetFilePath();

	strBuf2 += _T(".txtsrc");

//	Open the sources file. If it can't be opened, just return.
	if (!srcFile.Open(strBuf2, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite | CFile::typeText))
	{
		while (!srcList.IsEmpty())
			delete srcList.RemoveHead();
		return;
	}

	CString	strSrcList;

//	Create the ED2K source link and the formatted sources list
	strBuf2 = pPartFile->CreateED2kLink();

	if (!srcList.IsEmpty())
	{
		strBuf2 += _T("|sources@");
		strBuf2 += strBuf1;

		uint32	dwSrcCnt = 0, dwID;

		while (!srcList.IsEmpty())
		{
			pSrcData = srcList.RemoveHead();
			dwID = pSrcData->m_dwSrcIDHyb;
			strBuf1.Format( _T("%u.%u.%u.%u"),
				static_cast<byte>(dwID >> 24), static_cast<byte>(dwID >> 16),
				static_cast<byte>(dwID >> 8), static_cast<byte>(dwID) );

			if (dwSrcCnt++ < 10)
				strBuf2.AppendFormat(_T(",%s:%u"), strBuf1, pSrcData->m_uSourcePort);

			strSrcList.AppendFormat(_T("%s:%u,%s;\n"), strBuf1, pSrcData->m_uSourcePort, pSrcData->m_pachExpDate);

			delete pSrcData;
		}
	}

//	For some settings file name in the link can be localized (save it using local code page)
	CStringA	strBufA(strBuf2);

	srcFile.WriteString(_T("#link: "));
	srcFile.Write(strBufA, strBufA.GetLength());
	srcFile.WriteString(_T("|/\n\n#format: ip:port,expirationdate(yymmdd);\n"));
	srcFile.WriteString(strSrcList);
	srcFile.Close();
#endif //NEW_SOCKETS_ENGINE
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CalcExpiration() calculates a date 'dwDays' from the current date and returns it as a string in
//		'yymmdd' format.
CString CSourceSaver::CalcExpiration(uint32 dwDays)
{
	CTime	timeExpDate = CTime::GetCurrentTime() + CTimeSpan(dwDays, 0, 0, 0);

	return timeExpDate.Format(_T("%y%m%d"));
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	IsExpired() returns true if 'strExpirationDate' is less than the current date or if it has an invalid format.
bool CSourceSaver::IsExpired(const CString &strExpirationDate)
{
	bool	bResult = true;

//	Check strExpDate format
	if (strExpirationDate.GetLength() == 6)
	{
		const TCHAR	*pcStr = strExpirationDate.GetString();
		TCHAR		acNum[2][3];

		acNum[0][0] = pcStr[0];
		acNum[0][1] = pcStr[1];
		acNum[0][2] = _T('\0');
		acNum[1][0] = pcStr[2];
		acNum[1][1] = pcStr[3];
		acNum[1][2] = _T('\0');

		CTime			timeExpDate( _tstoi(acNum[0]) + 2000,
			_tstoi(acNum[1]), _tstoi(&pcStr[4]), 0, 0, 0 );

		bResult = (timeExpDate < CTime::GetCurrentTime());
	}

	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
