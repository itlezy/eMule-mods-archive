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
#include "map_inc.h"
#include "otherfunctions.h"

///////////////////////////////////////////////////////////////////////////////////////
//// CDeadSource

#ifdef REPLACE_MFCMAP
class CDeadSource{
#else
class CDeadSource
#ifdef _DEBUG
	: public CObject
#endif
{
#endif
public:
	CDeadSource(const CDeadSource& ds)			{*this = ds;}
	CDeadSource(uint32 dwID = 0, uint16 nPort = 0, uint32 dwServerIP = 0, uint16 nKadPort = 0);
	CDeadSource(const uchar* paucHash);

	CDeadSource& operator=(const CDeadSource& ds);
	friend bool operator==(const CDeadSource& ds1,const CDeadSource& ds2);
#ifdef REPLACE_MFCMAP
	size_t operator()(const CDeadSource&ds) const{
		size_t hash = 0;
		if (ds.m_dwID != 0){
			hash = ds.m_dwID;
			if (IsLowID(ds.m_dwID))
				hash ^= ds.m_dwServerIP;
		}
		else{
			ASSERT( isnulmd4(ds.m_aucHash) == 0 );
			hash++;
			for (size_t i = 0;i != 16;i++)
				hash += (ds.m_aucHash[i]+1)*((i*i)+1);
		}
		return hash;
	}
	bool operator()(const CDeadSource& ds1, const CDeadSource& ds2) const{
		return ds1 == ds2;
	}
#endif

	// netfinity: Rearranged for alignment reasons
	uchar			m_aucHash[16];			
	uint32			m_dwID;
	uint32			m_dwServerIP;
	uint16			m_nPort;
	uint16			m_nKadPort;
};

#ifdef REPLACE_MFCMAP
typedef unordered_map<CDeadSource, uint32, CDeadSource, CDeadSource> CDeadSourceMap;
#else
template<> inline UINT AFXAPI HashKey(const CDeadSource& ds){
	size_t hash = 0;
	if (ds.m_dwID != 0){
		hash = ds.m_dwID;
		if (IsLowID(ds.m_dwID))
			hash ^= ds.m_dwServerIP;
	}
	else{
		ASSERT( isnulmd4(ds.m_aucHash) == 0 );
		hash++;
		for (size_t i = 0;i != 16;i++)
			hash += (ds.m_aucHash[i]+1)*((i*i)+1);
	}
	return (UINT)hash;
};
typedef CMap<CDeadSource, const CDeadSource&, uint32, uint32> CDeadSourceMap;
#endif

///////////////////////////////////////////////////////////////////////////////////////
//// CDeadSourceList
class CUpDownClient;
class CDeadSourceList 
{
public:
	CDeadSourceList(void);
	~CDeadSourceList(void);
	void		AddDeadSource(const CUpDownClient* pToAdd);
	void		RemoveDeadSource(const CUpDownClient* client);
	bool		IsDeadSource(const CUpDownClient* pToCheck) const;
#ifdef REPLACE_ATLMAP
	size_t		GetDeadSourcesCount() const { return m_mapDeadSources.size(); }
#else
	UINT_PTR	GetDeadSourcesCount() const { return (UINT_PTR)m_mapDeadSources.GetCount(); }
#endif
	void		Init(bool bGlobalList);

protected:
	void		CleanUp();

private:
	CDeadSourceMap m_mapDeadSources;
	uint32	m_dwLastCleanUp;
	bool	m_bGlobalList;
};
