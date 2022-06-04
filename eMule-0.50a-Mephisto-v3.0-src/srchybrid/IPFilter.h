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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

struct SIPFilter
{
	SIPFilter() {} //Xman dynamic IP-Filters
	SIPFilter(uint32 newStart, uint32 newEnd, UINT newLevel, const CStringA& newDesc)
		: start(newStart),
		  end(newEnd),
		  level(newLevel),
		  desc(newDesc),
		  hits(0),
		  timestamp(0)		//Xman dynamic IP-Filters
	{ }
	uint32		start;
	uint32		end;
	UINT		level;
	CStringA	desc;
	UINT		hits;
	uint32	timestamp; //Xman dynamic IP-Filters
};

#define	DFLT_IPFILTER_FILENAME	_T("ipfilter.dat")
#define	DFLT_STATIC_IPFILTER_FILENAME	_T("ipfilter_static.dat") // Static IP Filter [Stulle] - Stulle
#define	DFLT_WHITE_IPFILTER_FILENAME	_T("ipfilter_white.dat") // IP Filter White List [Stulle] - Stulle

// 'CArray' would give us more cach hits, but would also be slow in array element creation 
// (because of the implicit ctor in 'SIPFilter'
//typedef CArray<SIPFilter, SIPFilter> CIPFilterArray; 
typedef CTypedPtrArray<CPtrArray, SIPFilter*> CIPFilterArray;

class CIPFilter
{
public:
	CIPFilter();
	~CIPFilter();

	CString GetDefaultFilePath() const;

	void AddIPRange(uint32 start, uint32 end, UINT level, const CStringA& rstrDesc) {
		m_iplist.Add(new SIPFilter(start, end, level, rstrDesc));
	}
	//Xman dynamic IP-Filters
	void AddIPTemporary(uint32 addip);
	void Process(); 
	//Xman end

	void RemoveAllIPFilters();
	bool RemoveIPFilter(const SIPFilter* pFilter);
	void SetModified(bool bModified = true) { m_bModified = bModified; }

	int AddFromFile(LPCTSTR pszFilePath, bool bShowResponse = true);
	int LoadFromDefaultFile(bool bShowResponse = true);
	void SaveToDefaultFile();

	bool IsFiltered(uint32 IP) /*const*/;
	bool IsFiltered(uint32 IP, UINT level) /*const*/;
	CString GetLastHit() const;
	const CIPFilterArray& GetIPFilter() const;
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	/*
	void    UpdateIPFilterURL(); //Xman auto update IPFilter
	*/
	void    UpdateIPFilterURL(uint32 uNewVersion = 0);
	// <== Advanced Updates [MorphXT/Stulle] - Stulle

private:
	const SIPFilter* m_pLastHit;
	CIPFilterArray m_iplist;
	bool m_bModified;

	bool ParseFilterLine1(const CStringA& rstrBuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& rstrDesc) const;
	bool ParseFilterLine2(const CStringA& rstrBuffer, uint32& ip1, uint32& ip2, UINT& level, CStringA& rstrDesc) const;

	// ==> Static IP Filter [Stulle] - Stulle
	void AddFromFile2(LPCTSTR pszFilePath);
	CString GetDefaultStaticFilePath() const;
	// <== Static IP Filter [Stulle] - Stulle

	// ==> IP Filter White List [Stulle] - Stulle
	void AddFromFileWhite(LPCTSTR pszFilePath);
	CString GetDefaultWhiteFilePath() const;

	void AddIPRangeWhite(uint32 start, uint32 end, UINT level, const CStringA& rstrDesc) {
		m_iplist_White.Add(new SIPFilter(start, end, level, rstrDesc));
	}
	CIPFilterArray m_iplist_White;
	// <== IP Filter White List [Stulle] - Stulle
};
