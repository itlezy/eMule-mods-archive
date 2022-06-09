// X: [CI] - [Code Improvement] use crypto++'s md5 implementation
//
// MD5.h
//
#pragma once
#define CRYPTOPP_ENABLE_NAMESPACE_WEAK 1
#include <crypto/md5.h>

typedef union
{
	BYTE	n[16];
	BYTE	b[16];
	DWORD	w[4];
} MD5;



class MD5Sum
{
public:
	MD5Sum();
	MD5Sum(const CString& sSource);
	MD5Sum(const unsigned char* pachSource, uint_ptr nLen);

	void Calculate(const CString& sSource);
	void Calculate(const unsigned char* pachSource, uint_ptr nLen);

	CString GetHash() const;
	const BYTE* GetRawHash() const { return (const BYTE*) m_hash.b; }

private:
	MD5 m_hash;
	CryptoPP::Weak::MD5 m_md5;
};
