#include "stdafx.h"
#include "MD5Sum.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


MD5Sum::MD5Sum()
{
}

MD5Sum::MD5Sum(const CString& sSource)
{
	Calculate(sSource);
}

MD5Sum::MD5Sum(const unsigned char* pachSource, uint_ptr nLen)
{
	Calculate(pachSource, nLen);
}

void MD5Sum::Calculate(const CString& sSource)
{
	return Calculate((const unsigned char*)(LPCTSTR)sSource, sSource.GetLength()*sizeof(TCHAR));
}

void MD5Sum::Calculate(const unsigned char* pachSource, uint_ptr nLen)
{
	// X: [CI] - [Code Improvement] use crypto++'s md5 implementation
	m_md5.Restart();
	m_md5.Update((const byte *) pachSource, nLen);
	m_md5.Final(m_hash.b);
}

CString MD5Sum::GetHash() const
{
	return md4str(m_hash.b); // X: [CI] - [Code Improvement]
}
