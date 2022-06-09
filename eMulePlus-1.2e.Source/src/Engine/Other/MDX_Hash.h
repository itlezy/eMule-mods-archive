////////////////////////////////////////////////////////////////////////////////////////
/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */
////////////////////////////////////////////////////////////////////////////////////////
#pragma once

////////////////////////////////////////////////////////////////////////////////////////
typedef unsigned char *POINTER;
typedef unsigned short int UINT2;
typedef unsigned long int UINT4;

typedef struct {
	UINT4 state[4];
	UINT4 count[2];
	unsigned char buffer[64];
} MD5_CTX;
////////////////////////////////////////////////////////////////////////////////////////
#define _CreateHashFromFile(pFile, dwLength, pbyteHash) {CreateMD4HashFromInput(pFile, NULL, dwLength, pbyteHash);}
#define _CreateHashFromString(pbyteMem, dwLength, pbyteHash) {CreateMD4HashFromInput(NULL, pbyteMem, dwLength, pbyteHash);}
////////////////////////////////////////////////////////////////////////////////////////
class MD5Sum
{
public:
	CString GetHash()	{return m_strMD5Hash;}
	MD5Sum();
	MD5Sum(CString sSource);
	CString Calculate(CString sSource);
	CString GetMD5Hash();

private:
	CString	m_strMD5Hash;
};
////////////////////////////////////////////////////////////////////////////////////////
void MD5Init (MD5_CTX *);
void MD5Update (MD5_CTX *, unsigned char *, unsigned int);
void MD5Final (unsigned char [16], MD5_CTX *);
static void __fastcall MD5Transform (UINT4 [4], unsigned char [64]);
static void Encode (unsigned char *, UINT4 *, unsigned int);
static void Decode (UINT4 *, unsigned char *, unsigned int);
static void MD5_memcpy (POINTER, POINTER, unsigned int);
static void MD5_memset (POINTER, int, unsigned int);

static void __fastcall MD4Transform(ULONG Hash[4], ULONG x[16]);
void CreateMD4HashFromInput(CFile* pFile, UCHAR *pbyteMem, ULONG dwLength, UCHAR *pbyteHash);

CString HashToString(int iHashLength, const UCHAR *hash);
