//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#include "types.h"
#include "otherfunctions.h"

class CCKey // : public CObject
{
public:
	CCKey(const uchar* key = 0)	{m_key = key;}
	CCKey(const CCKey& k1)		{m_key = k1.m_key;}

	CCKey& operator=(const CCKey& k1)
	{
		if (&k1 != this)
		{
			m_key = k1.m_key;
		}
		return *this;
	}
	friend bool operator==(const CCKey& k1,const CCKey& k2)	{return !md4cmp(k1.m_key,k2.m_key);}
	
	const uchar* m_key;
};
/*
template<> inline UINT AFXAPI HashKey(const CCKey& key)
{
   uint32 hash = 1;
   for (int i = 0;i != 16;i++)
	   hash += (key.m_key[i]+1)*((i*i)+1);
   return hash;
};*/

//eklmn: "One-at-a-Time Hash" has no funnels according to http://burtleburtle.net/bob/hash/doobs.html
template<> inline UINT AFXAPI HashKey(const CCKey& key)
{
	uint32   dwHash, i;

	for (dwHash=0, i=0; i < 16; ++i)
	{
		dwHash += key.m_key[i];
		dwHash += (dwHash << 10);
		dwHash ^= (dwHash >> 6);
	}
	dwHash += (dwHash << 3);
	dwHash ^= (dwHash >> 11);
	dwHash += (dwHash << 15);

	return dwHash;
};
