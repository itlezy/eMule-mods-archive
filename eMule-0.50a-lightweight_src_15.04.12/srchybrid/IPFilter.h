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
	SIPFilter(uint32 newStart, uint32 newEnd)
		: start(newStart),
		  end(newEnd)
	{ 
	}
	uint32		start;
	uint32		end;
};

#define	DFLT_IPFILTER_FILENAME	_T("ipfilter.dat")

// 'CAtlArray' would give us more cach hits, but would also be slow in array element creation 
// (because of the implicit ctor in 'SIPFilter'
//typedef CAtlArray<SIPFilter> CIPFilterArray; 
typedef std::vector<SIPFilter> CIPFilterArray;

class CIPFilter
{
public:
	CIPFilter();
	~CIPFilter();

	CString GetDefaultFilePath() const;

	void AddIPRange(uint32 start, uint32 end) {
		m_iplist.push_back(SIPFilter(start, end));
	}

	void RemoveAllIPFilters();

	size_t AddFromFile(LPCTSTR pszFilePath, bool bShowResponse = true);
	void LoadFromDefaultFile(bool bShowResponse = true);

	bool IsFiltered(uint32 IP) /*const*/;
	void    UpdateIPFilterURL(CString url); //Xman auto update IPFilter// X: [CI] - [Code Improvement]
	uint_ptr hits;// X: [SFH] - [Show IP Filter Hits]
private:
	CIPFilterArray m_iplist;

	bool ParseFilterLine1(char* rstrBuffer, uint32& ip1, uint32& ip2, UINT& level) const;
	bool ParseFilterLine2(char* rstrBuffer, uint32& ip1, uint32& ip2, UINT& level) const;
};