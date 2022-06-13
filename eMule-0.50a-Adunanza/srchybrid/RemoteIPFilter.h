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
//
// Tigerjact fecit A.D. MMVI
#pragma once

struct RemoteSIPFilter
{
	RemoteSIPFilter(uint32 newStart, uint32 newEnd, UINT newLevel, const CStringA& newDesc)
		: start(newStart),
		  end(newEnd),
		  level(newLevel),
		  desc(newDesc),
		  hits(0)
	{ }
	uint32		start;
	uint32		end;
	UINT		level;
	CStringA	desc;
	UINT		hits;
};

#define	DFLT_REMOTEIPFILTER_FILENAME	_T("tmp_adu_remoteipfilter.dat")



// 'CArray' would give us more cach hits, but would also be slow in array element creation 
// (because of the implicit ctor in 'RemoteSIPFilter'
//typedef CArray<SIPFilter, SIPFilter> CRemoteIPFilterArray; 
typedef CTypedPtrArray<CPtrArray, RemoteSIPFilter*> CRemoteIPFilterArray;



class CRemoteIPFilter
{
public:
	CRemoteIPFilter();
	~CRemoteIPFilter();

    int RemoteIPFilterNextUpdate;
	int RemoteIPFilterExpireTime;
	bool RemoteIPFilterDTF;//download the file
	
	CString RemoteGetDefaultFilePath() const;
	void RemoteAddIPRange(uint32 start, uint32 end, UINT level, const CStringA& rstrDesc) {
		m_iplist.Add(new RemoteSIPFilter(start, end, level, rstrDesc));
	}
	void RemoteRemoveAllIPFilters();
	bool RemoteRemoveIPFilter(const RemoteSIPFilter* pFilter);
	void RemoteSetModified(bool bModified = true) { m_bModified = bModified; }
	
	int RemoteAddFromFile(LPCTSTR pszFilePath, bool bShowResponse = true);
	int RemoteLoadFromDefaultFile(bool bShowResponse = true);
	void RemoteSaveToDefaultFile();

	bool RemoteIsFiltered(uint32 IP) /*const*/;
	bool RemoteIsFiltered(uint32 IP, UINT level) /*const*/;
	CString GetLastHit() const;
	const CRemoteIPFilterArray& GetIPFilter() const;
	
	//scarico e carico il file
	void RemoteScaricaECarica();
	void aspetta(bool idfile);


	static CString m_sFilename;
	static CString tmpfile;


private:
	const RemoteSIPFilter* m_pLastHit;
	CRemoteIPFilterArray m_iplist;
	bool m_bModified;

	bool RemoteParseFilterLine1(const CStringA& rstrBuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& rstrDesc) const;
	bool RemoteParseFilterLine2(const CStringA& rstrBuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& rstrDesc) const;
};

