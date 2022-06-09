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
#include <vector>
struct SIPFilter
{
	SIPFilter(uint32 newStart, uint32 newEnd, UINT newLevel, const char* newDesc);
	inline bool IsTemp() const{ return desc == NULL; }
	inline LPCSTR GetDescA() const;
	inline CString GetDesc() const;
	~SIPFilter(){
		if(desc)
			delete [] desc;
	}
	uint32		start;
	uint32		end;
	UINT		level;
	UINT		hits;
	LPSTR		desc;
};
struct STempIPFilter
{
	STempIPFilter(SIPFilter*ipfilter)
		: timestamp(GetTickCount()),		//Xman dynamic IP-Filters
		  filter(ipfilter)
	{ 
	}
	SIPFilter*filter;
	uint32	timestamp; //Xman dynamic IP-Filters
};

#define	DFLT_IPFILTER_FILENAME	_T("ipfilter.dat")

// 'CAtlArray' would give us more cach hits, but would also be slow in array element creation 
// (because of the implicit ctor in 'SIPFilter'
//typedef CAtlArray<SIPFilter> CIPFilterArray; 
typedef std::vector<SIPFilter*> CIPFilterArray;
typedef std::vector<STempIPFilter> CTempIPFilterArray;

class CIPFilter
{
public:
	CIPFilter();
	~CIPFilter();

	CString GetDefaultFilePath() const;

	void AddIPRange(uint32 start, uint32 end, UINT level, const char* rstrDesc) {
		m_iplist.push_back(new SIPFilter(start, end, level, rstrDesc));
	}
	//Xman dynamic IP-Filters
	void AddIPTemporary(uint32 addip);
	void Process(); 
	//Xman end

	void RemoveAllIPFilters();
	bool RemoveIPFilter(const SIPFilter* pFilter, bool temp);
	void SetModified(bool bModified = true) { m_bModified = bModified; }

	size_t AddFromFile(LPCTSTR pszFilePath, bool bShowResponse = true);
	void LoadFromDefaultFile(bool bShowResponse = true);
	void SaveToDefaultFile();

	bool IsFiltered(uint32 IP) /*const*/;
	bool IsFiltered(uint32 IP, UINT level) /*const*/;
	CString GetLastHit() const;
	const CIPFilterArray& GetIPFilter() const;
	void    UpdateIPFilterURL(CString url); //Xman auto update IPFilter// X: [CI] - [Code Improvement]
	uint_ptr hits;// X: [SFH] - [Show IP Filter Hits]
private:
	const SIPFilter* m_pLastHit;
	CIPFilterArray m_iplist;
	CTempIPFilterArray m_tmpiplist;// X: [ITF] - [Index Temporary Filter]
	bool m_bModified;

	bool ParseFilterLine1(char* rstrBuffer, uint32& ip1, uint32& ip2, UINT& level, char** rstrDesc) const;
	bool ParseFilterLine2(char* rstrBuffer, uint32& ip1, uint32& ip2, UINT& level, char** rstrDesc) const;
};
