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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

class CPartFile;
class CUpDownClient;

class CSourceSaver
{
public:
	CSourceSaver(void);
	~CSourceSaver(void);
	void Process(CPartFile* file);
	void DeleteFile(CPartFile* file);

protected:
	// khaos::kmod+ Source Exchange Version
	class CSourceData
	{
	public:
		CSourceData(const UINT dwID, const uint16 wPort, const UINT dwserverip, const uint16 wserverport, const TCHAR* exp, const uint8 srcexver)
		{
			sourceID = dwID; 
			sourcePort = wPort; 
			serverip = dwserverip;
			serverport = wserverport;
			memcpy(expiration, exp, 11*sizeof(TCHAR));
			expiration[10] = 0;
			nSrcExchangeVer = srcexver;
		}
		CSourceData(CUpDownClient* client, const uint16 partsavail, const TCHAR* exp);
		CSourceData(CSourceData* pOld)
		{
			sourceID = pOld->sourceID; 
			sourcePort = pOld->sourcePort; 
			serverip = pOld->serverip;
			serverport = pOld->serverport;
			memcpy(expiration, pOld->expiration, 11*sizeof(TCHAR)); 
			partsavailable = pOld->partsavailable;
			expiration[10] = 0;
			nSrcExchangeVer = pOld->nSrcExchangeVer;
		}
		bool Compare(CSourceData* tocompare) const		{return sourceID == tocompare->sourceID && sourcePort == tocompare->sourcePort;}

		UINT	sourceID;
		uint16	sourcePort;
		UINT	serverip;
		uint16	serverport;
		uint16	partsavailable;
		TCHAR	expiration[11];
		uint8	nSrcExchangeVer;
	};
	// khaos::kmod-
	typedef CTypedPtrList<CPtrList, CSourceData*> SourceList;

	void	LoadSourcesFromFile(CPartFile* file, SourceList* sources, LPCTSTR slsfile);
	void	SaveSources(CPartFile* file, SourceList* prevsources, LPCTSTR slsfile);
	void	AddSourcesToDownload(CPartFile* file, SourceList* sources);
	
	UINT	m_dwLastTimeLoaded;
	UINT	m_dwLastTimeSaved;

	CString CalcExpiration(const int nDays) const;
	bool	IsExpired(const CString& expirationdate) const;
};

