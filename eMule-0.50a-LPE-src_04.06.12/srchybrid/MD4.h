//
// MD4.h
//
// Copyright (c) Shareaza Development Team, 2002-2004.
// This file is part of SHAREAZA (www.shareaza.com)
//
// Shareaza is free software; you can redistribute it
// and/or modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2 of
// the License, or (at your option) any later version.
//
// Shareaza is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Shareaza; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
#pragma once
#ifndef NOT_USE__ASM
#define NOT_USE__ASM // X: Use cryptlib for all platforms
#endif

#ifdef NOT_USE__ASM// X: [64P] - [64BitPortable]
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <crypto/md4.h>
#endif
#include "OtherFunctions.h"

typedef union
{
	BYTE	n[16];
	BYTE	b[16];
	DWORD	w[4];
} MD4;

class CMD4
{
// Construction
public:
	CMD4();
	virtual ~CMD4();

	static bool VerifyImplementation();

// Attributes
protected:
	// NOTE: if you change this, modify the offsets in MD4_ASM.ASM accordingly
#ifdef NOT_USE__ASM// X: [64P] - [64BitPortable]
// netfinity: Use cryptlib for non X86 platforms
        CryptoPP::Weak::MD4 m_md4;
	MD4 m_hash;
#else
	uint32	m_nState[4];
	uint64	m_nCount;
	uchar	m_nBuffer[64];
#endif

// Operations
public:
	void	Reset();
	void	Add(LPCVOID pData, DWORD nLength);
	void	Finish();
	void	GetHash(MD4* pHash);
	const BYTE* GetHash() const;
};

inline bool operator==(const MD4& md4a, const MD4& md4b)
{
    return md4cmp(&md4a, &md4b) == 0;
}

inline bool operator!=(const MD4& md4a, const MD4& md4b)
{
    return md4cmp(&md4a, &md4b) != 0;
}
