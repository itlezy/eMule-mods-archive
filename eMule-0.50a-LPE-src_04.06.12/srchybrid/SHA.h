/*
 ---------------------------------------------------------------------------
 Copyright (c) 2002, Dr Brian Gladman <brg@gladman.me.uk>, Worcester, UK.
 All rights reserved.

 LICENSE TERMS

 The free distribution and use of this software in both source and binary 
 form is allowed (with or without changes) provided that:

   1. distributions of this source code include the above copyright 
      notice, this list of conditions and the following disclaimer;

   2. distributions in binary form include the above copyright
      notice, this list of conditions and the following disclaimer
      in the documentation and/or other associated materials;

   3. the copyright holder's name is not used to endorse products 
      built using this software without specific written permission. 

 ALTERNATIVELY, provided that this notice is retained in full, this product
 may be distributed under the terms of the GNU General Public License (GPL),
 in which case the provisions of the GPL apply INSTEAD OF those given above.
 
 DISCLAIMER

 This software is provided 'as is' with no explicit or implied warranties
 in respect of its properties, including, but not limited to, correctness 
 and/or fitness for purpose.
 ---------------------------------------------------------------------------
 Issue Date: 30/11/2002

 This is a byte oriented version of SHA1 that operates on arrays of bytes
 stored in memory. It runs at 22 cycles per byte on a Pentium P4 processor
*/
#pragma once
#include "shahashset.h"

#ifdef NOT_USE__ASM// X: [64P] - [64BitPortable]
#include <crypto/sha.h>
#endif
typedef struct
{
	BYTE	b[20];
} SHA1;

#define SHA1_BLOCK_SIZE		64
#define SHA1_DIGEST_SIZE	20

class CSHA : public CAICHHashAlgo
{
// Construction
public:
	CSHA();
	virtual ~CSHA();

	static bool VerifyImplementation();

// Attributes
protected:
	// NOTE: if you change this, modify the offsets in SHA_ASM.ASM accordingly
#ifdef NOT_USE__ASM// X: [64P] - [64BitPortable]
// netfinity: Use cryptlib for non X86 platforms
	CryptoPP::SHA1 m_sha;
	SHA1 m_hash;
#else
	uint64	m_nCount;
	uint32	m_nHash[ 5 ];
	uchar m_nBuffer[ SHA1_BLOCK_SIZE ];
#endif
// Operations
public:
	// CAICHHashAlgo interface
	virtual void	Reset();
	virtual void	Add(LPCVOID pData, DWORD nLength);
	virtual void	Finish(CAICHHash& Hash);
	virtual void	GetHash(CAICHHash& Hash);

	void	Finish();
	void	GetHash(SHA1* pHash);
/*	CString	GetHashString(BOOL bURN = FALSE);

	static CString	HashToString(const SHA1* pHash, BOOL bURN = FALSE);
	static CString	HashToHexString(const SHA1* pHash, BOOL bURN = FALSE);
	static BOOL		HashFromString(LPCTSTR pszHash, SHA1* pHash);
	static BOOL		HashFromURN(LPCTSTR pszHash, SHA1* pHash);
	static BOOL		IsNull(SHA1* pHash);*/
};

inline bool operator==(const SHA1& sha1a, const SHA1& sha1b)
{
    return memcmp( &sha1a, &sha1b, HASHSIZE ) == 0;
}

inline bool operator!=(const SHA1& sha1a, const SHA1& sha1b)
{
    return memcmp( &sha1a, &sha1b, HASHSIZE ) != 0;
}
