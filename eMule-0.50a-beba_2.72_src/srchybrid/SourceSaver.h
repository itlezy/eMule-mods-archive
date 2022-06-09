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
	bool Process(CPartFile* file);
	void DeleteFile(CPartFile* file);

protected:
	// khaos::kmod+ Source Exchange Version
	class CSourceData
	{
	public:
		CSourceData(uint32 dwID, uint16 wPort,uint32 dwserverip, uint16 wserverport, const TCHAR* exp, uint8 srcexver) {	sourceID = dwID; 
																					sourcePort = wPort; 
																					serverip = dwserverip;
																					serverport = wserverport;
																					memcpy(expiration, exp, 11*sizeof(TCHAR));
																					expiration[10] = 0;
																					nSrcExchangeVer = srcexver;}

		CSourceData(CUpDownClient* client, const TCHAR* exp);

		CSourceData(CSourceData* pOld) {							sourceID = pOld->sourceID; 
																	sourcePort = pOld->sourcePort; 
																	serverip = pOld->serverip;
																	serverport = pOld->serverport;
																	memcpy(expiration, pOld->expiration, 11*sizeof(TCHAR)); 
																	partsavailable = pOld->partsavailable;
																	expiration[10] = 0;
																	nSrcExchangeVer = pOld->nSrcExchangeVer;}

		bool Compare(CSourceData* tocompare) {						return ((sourceID == tocompare->sourceID) 
																	 && (sourcePort == tocompare->sourcePort)); }

		uint32	sourceID;
		uint16	sourcePort;
		uint32	serverip;
		uint16	serverport;
		uint32	partsavailable;
		TCHAR	expiration[11];
		uint8	nSrcExchangeVer;
	};
	// khaos::kmod-
	typedef CTypedPtrList<CPtrList, CSourceData*> SourceList;

	void LoadSourcesFromFile(CPartFile* file, SourceList* sources, LPCTSTR slsfile);
	void SaveSources(CPartFile* file, SourceList* prevsources, LPCTSTR slsfile, UINT maxSourcesToSave);
	void AddSourcesToDownload(CPartFile* file, SourceList* sources);
	
	uint32	m_dwLastTimeLoaded;
	uint32  m_dwLastTimeSaved;

	CString CalcExpiration(int nDays);
	bool IsExpired(CString expirationdate);
};
