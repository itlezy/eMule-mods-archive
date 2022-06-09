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
#pragma once

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <hash_map>
#pragma warning(pop)

#ifndef VS2002
using namespace stdext;
#endif

typedef struct
{
	uint32		dwStart;
	uint32		dwEnd;
// D0-D7 ( 8): level
// D8-D31(24): string index
	unsigned	uiPacked;
} SIPFilter;

#define	DFLT_IPFILTER_FILENAME	_T("IPFilter.dat")

typedef std::vector<SIPFilter> CIPFilterArray;

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
class CIPFilter
{
public:
	CIPFilter();
	~CIPFilter();

	void	AddIPRange(uint32 dwIPFrom, uint32 dwIPTo, UINT uiLevel, const CStringA& rstrDesc);
	void	AddTemporaryBannedIP(uint32 dwIP) { m_TempIPList[dwIP] = ::GetTickCount(); }
	void	RemoveAllIPs();
	int		AddFromFile(LPCTSTR pstrFilePath, bool bShowResponse = true);
	int		LoadFromDefaultFile(bool bShowResponse = true);
	bool	DownloadIPFilter();

	bool	IsFiltered(uint32 dwIP);
	bool	IsFiltered(uint32 dwIP, UINT uiLevel);
	bool	IsCachedAndFiltered(uint32 dwIP);
	void	ResetCachedFilteredIP()					{m_dwLastFilteredIP = 0;}
	bool	IsTemporaryFiltered(uint32 dwIP);
	const char*	GetLastHit() const;

private:
	bool	ParseFilterLine1(const char *pcLine, unsigned uiLineLen, CStringA *pstrOut, uint32 &rdwIP1, uint32 &rdwIP2, UINT &ruiLevel) const;
	bool	ParseFilterLine2(CStringA &rstrBuffer, uint32 &rdwIP1, uint32 &rdwIP2, UINT &ruiLevel) const;

private:
	uint32					m_dwLastFilteredIP;
	const SIPFilter			*m_pLastHit;
	CIPFilterArray			m_IPArr;
	char					*m_pcDesc;	//combined description strings for all permanent ranges
	hash_map<uint32,uint32>	m_TempIPList;
	char					m_acTempLastHit[64];
};
/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
