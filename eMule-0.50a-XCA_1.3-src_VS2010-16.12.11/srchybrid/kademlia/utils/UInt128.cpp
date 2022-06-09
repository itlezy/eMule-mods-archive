/*
Copyright (C)2003 Barry Dunne (http://www.emule-project.net)
 
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
 
You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// Note To Mods //
/*
Please do not change anything here and release it..
There is going to be a new forum created just for the Kademlia side of the client..
If you feel there is an error or a way to improve something, please
post it in the forum first and let us look at it.. If it is a real improvement,
it will be added to the official client.. Changing something without knowing
what all it does can cause great harm to the network if released in mass form..
Any mod that changes anything within the Kademlia side will not be allowed to advertise
there client on the eMule forum..
*/

#include "stdafx.h"
/*#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
#include <crypto/osrng.h> //Xman
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative*/
#include "./UInt128.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Kademlia;
//using namespace CryptoPP;
static const TCHAR _acHexDigits[] = _T("0123456789ABCDEF");

// netfinity: Moved this inline for speed!
/*CUInt128::CUInt128()
{
	SetValue((ULONG)0);
}*/

CUInt128::CUInt128(bool bFill)
{
	if( bFill )
	{
#ifdef _WIN64
		((uint64*)m_uData)[0] = (uint64)-1;
		((uint64*)m_uData)[1] = (uint64)-1;
#else
		m_uData[0] = (ULONG)-1;
		m_uData[1] = (ULONG)-1;
		m_uData[2] = (ULONG)-1;
		m_uData[3] = (ULONG)-1;
#endif
	}
	else
		SetValue((ULONG)0);
}

CUInt128::CUInt128(ULONG uValue)
{
	SetValue(uValue);
}

CUInt128::CUInt128(const byte *pbyValueBE)
{
	SetValueBE(pbyValueBE);
}

CUInt128::CUInt128(const CUInt128 &uValue, uint_ptr uNumBits)
{
	/* 32 bit version
	// Copy the whole ULONGs
	uint_ptr uNumULONGs = uNumBits / 32;
	for (uint_ptr iIndex=0; iIndex<uNumULONGs; iIndex++)
		m_uData[iIndex] = uValue.m_uData[iIndex];

	// Copy the remaining bits
	//for (uint_ptr iIndex=(32*uNumULONGs); iIndex<uNumBits; iIndex++)
		//SetBitNumber(iIndex, uValue.GetBitNumber(iIndex));
	if(uNumBits % 32){
		uint32 mask = ~((1 << (32 - uNumBits % 32)) - 1);
		m_uData[uNumULONGs] = uValue.m_uData[uNumULONGs] & mask;
	}

	SFMT&rng = *t_rng;
	// Pad with random bytes (Not seeding based on time to allow multiple different ones to be created in quick succession)
	for (uint_ptr iIndex=uNumBits; iIndex<128; iIndex++)
		SetBitNumber(iIndex, (rng.getUInt8()%2));*/
	// Copy the High ULONGs
	uint64 uint128hi = (uint64)uValue.m_uData[0]<<32|uValue.m_uData[1];
	uint64 uint128lo = 0;
	SFMT&rng = *t_rng;
	if(uNumBits < 64){
		if(uNumBits > 0)
			uint128hi >>= 64 - uNumBits;
		// Pad with random bytes (Not seeding based on time to allow multiple different ones to be created in quick succession)
		for (uint_ptr iIndex=uNumBits; iIndex<64; iIndex++){
			uint128hi <<= 1;
			uint128hi |=(rng.getUInt8()%2);
		}
		uNumBits = 64;
	}
	else if(uNumBits > 64)
	{
		// Copy the Low ULONGs
		uint128lo = ((uint64)uValue.m_uData[2]<<32|uValue.m_uData[3]) >> (128 - uNumBits);
	}
	// Pad with random bytes (Not seeding based on time to allow multiple different ones to be created in quick succession)
	for (uint_ptr iIndex=uNumBits; iIndex<128; iIndex++){
		uint128lo <<= 1;
		uint128lo |=(rng.getUInt8()%2);
	}
	m_uData[0] = uint128hi>> 32;m_uData[1] = (uint32)uint128hi;m_uData[2] = uint128lo>> 32;m_uData[3] = (uint32)uint128lo;
}

// netfinity: Moved this inline for speed!
/*CUInt128& CUInt128::SetValue(const CUInt128 &uValue)
{
	m_uData[0] = uValue.m_uData[0];
	m_uData[1] = uValue.m_uData[1];
	m_uData[2] = uValue.m_uData[2];
	m_uData[3] = uValue.m_uData[3];
	return *this;
}

CUInt128& CUInt128::SetValue(ULONG uValue)
{
	m_uData[0] = 0;
	m_uData[1] = 0;
	m_uData[2] = 0;
	m_uData[3] = uValue;
	return *this;
}*/

void CUInt128::SetValueBE(const byte *pbyValueBE)
{
#if /*defined(_M_IX86) && */(_MSC_FULL_VER > 13009037)
	m_uData[0] = _byteswap_ulong(((uint32*)pbyValueBE)[0]);
	m_uData[1] = _byteswap_ulong(((uint32*)pbyValueBE)[1]);
	m_uData[2] = _byteswap_ulong(((uint32*)pbyValueBE)[2]);
	m_uData[3] = _byteswap_ulong(((uint32*)pbyValueBE)[3]);
#else
	for (size_t iIndex=0; iIndex<16; iIndex++)
		m_uData[iIndex/4] |= ((ULONG)pbyValueBE[iIndex]) << (8*(3-(iIndex%4)));
#endif
}

void CUInt128::SetValueRandom()
{
	//AutoSeededRandomPool rng;
	//byte byRandomBytes[16];
	t_rng->fill((uint8*)m_uData, 16);
	//rng.GenerateBlock((byte*)m_uData, 16);
	//SetValueBE( byRandomBytes );
}
/*
void CUInt128::SetValueGUID()
{
	SetValue((ULONG)0);
	GUID guid;
	if (CoCreateGuid(&guid) != S_OK)
		return *this;
	m_uData[0] = guid.Data1;
	m_uData[1] = ((ULONG)guid.Data2) << 16 | guid.Data3;
	m_uData[2] = ((ULONG)guid.Data4[0]) << 24 | ((ULONG)guid.Data4[1]) << 16 | ((ULONG)guid.Data4[2]) << 8 | ((ULONG)guid.Data4[3]);
	m_uData[3] = ((ULONG)guid.Data4[4]) << 24 | ((ULONG)guid.Data4[5]) << 16 | ((ULONG)guid.Data4[6]) << 8 | ((ULONG)guid.Data4[7]);
	return *this;
}
*/
uint_ptr CUInt128::GetBitNumber(uint_ptr uBit) const
{
	if (uBit > 127)
		return 0;
	sint_ptr iLongNum = uBit / 32;
	sint_ptr iShift = 31 - (uBit % 32);
	return ((m_uData[iLongNum] >> iShift) & 1);
}
/*
void CUInt128::SetBitNumber(sint_ptr uBit, sint_ptr uValue)
{
	sint_ptr iLongNum = uBit / 32;
	sint_ptr iShift = 31 - (uBit % 32);
	m_uData[iLongNum] |= (1 << iShift);
	if (uValue == 0)
		m_uData[iLongNum] ^= (1 << iShift);
}
*/
// netfinity: Moved this inline for speed!
/*CUInt128& CUInt128::Xor(const CUInt128 &uValue)
{
	for (sint_ptr iIndex=0; iIndex<4; iIndex++)
		m_uData[iIndex] ^= uValue.m_uData[iIndex];
	return *this;
}*/
/*
CUInt128& CUInt128::XorBE(const byte *pbyValueBE)
{
	return Xor(CUInt128(pbyValueBE));
}
*/
void CUInt128::ToHexString(CString *pstr) const
{
	/*pstr->Empty();
	// netfinity: Reduced CPU usage
	//CString element;
	//element.Format(_T("%08X%08X%08X%08X"), m_uData[0], m_uData[1], m_uData[2], m_uData[3]);// X: [CI] - [Code Improvement]
	wchar_t element[9];
	for (size_t i=0; i<4; ++i)
	{
		// netfinity: Reduced CPU usage
		for (size_t j=0; j<8; ++j)
		{
			TCHAR	digit = static_cast<TCHAR>((m_uData[i] >> (j*4)) & 0xF);
			element[7-j] = digit + ((digit < 10)?_T('0'):_T('A') - 10);
		}
		element[8] = _T('\0');
		pstr->Append(element);
	}*/
	uint64 uint128hi = (uint64)m_uData[0] << 32 | m_uData[1];
	uint64 uint128lo = (uint64)m_uData[2] << 32 | m_uData[3];
	wchar_t buffer[32];
	for (sint_ptr i = 31; i >= 0; --i)
	{
		buffer[i] = _acHexDigits[uint128lo & 0xF];
		uint128lo >>= 4;
		if(i == 16)
			uint128lo = uint128hi;
	}
	pstr->SetString(buffer, 32);
}

CString CUInt128::ToHexString() const
{
	/*CString str;
	// netfinity: Reduced CPU usage
	//CString element;
	//element.Format(_T("%08X%08X%08X%08X"), m_uData[0], m_uData[1], m_uData[2], m_uData[3]);// X: [CI] - [Code Improvement]
	wchar_t element[9];
	for (size_t i=0; i<4; ++i)
	{
		// netfinity: Reduced CPU usage
		for (size_t j=0; j<8; ++j)
		{
			TCHAR	digit = (TCHAR) (m_uData[i] >> (j*4)) & 0xF;
			element[7-j] = digit + ((digit < 10)?_T('0'):_T('A') - 10);
		}
		element[8] = _T('\0');
		str.Append(element);
	}
	return str;*/

	CString str;
	uint64 uint128hi = (uint64)m_uData[0] << 32 | m_uData[1];
	uint64 uint128lo = (uint64)m_uData[2] << 32 | m_uData[3];
	LPTSTR buffer = str.GetBuffer(32);
	for (sint_ptr i = 31; i >= 0; --i)
	{
		buffer[i] = _acHexDigits[uint128lo & 0xF];
		uint128lo >>= 4;
		if(i == 16)
			uint128lo = uint128hi;
	}
	str.ReleaseBuffer(32);
	return str;
}

void CUInt128::ToBinaryString(CString *pstr, bool bTrim) const
{
	/*pstr->Empty();
	//CString sElement; //Xman
	sint_ptr iBit;
	for (sint_ptr iIndex=0; iIndex<128; iIndex++)
	{
		iBit = GetBitNumber(iIndex);
		if ((!bTrim) || (iBit != 0))
		{
			pstr->AppendChar((iBit == 1)?_T('1'):_T('0'));// X: [CI] - [Code Improvement]
			bTrim = false;
		}
	}
	if (pstr->GetLength() == 0)
		pstr->SetString(_T("0"));
	*/
	static const TCHAR _acBinDigits[] = _T("01");
	uint64 uint128hi = (uint64)m_uData[0] << 32 | m_uData[1];
	uint64 uint128lo = (uint64)m_uData[2] << 32 | m_uData[3];
	wchar_t element[129];
	for (sint_ptr i = 127; i >= 0; --i)
	{
		element[i] = _acBinDigits[uint128lo & 1];
		uint128lo >>= 1;
		if(i == 64)
			uint128lo = uint128hi;

	}
	if(!bTrim){
		pstr->SetString(element, 128);
		return;
	}
	LPTSTR curchar = element;
	element[128] = _T('\0');
	while(*curchar++ == _T('0')){;}
	if(*--curchar == _T('\0'))
		--curchar;
	pstr->SetString(curchar, 128 - (curchar - element));
}

void CUInt128::ToByteArray(byte *pby) const
{
#if /*defined(_M_IX86) && */(_MSC_FULL_VER > 13009037)
	((uint32*)pby)[0] = _byteswap_ulong(m_uData[0]);
	((uint32*)pby)[1] = _byteswap_ulong(m_uData[1]);
	((uint32*)pby)[2] = _byteswap_ulong(m_uData[2]);
	((uint32*)pby)[3] = _byteswap_ulong(m_uData[3]);
#else
	for (size_t iIndex=0; iIndex<16; iIndex++)
		pby[iIndex] = (byte)(m_uData[iIndex/4] >> (8*(3-(iIndex%4))));
#endif
}

int CUInt128::CompareTo(const CUInt128 &uOther) const
{
	for (size_t iIndex=0; iIndex<4; iIndex++)
	{
		if (m_uData[iIndex] < uOther.m_uData[iIndex])
			return -1;
		if (m_uData[iIndex] > uOther.m_uData[iIndex])
			return 1;
	}
	return 0;
}

int CUInt128::CompareTo(ULONG uValue) const
{
	if ((m_uData[0] > 0) || (m_uData[1] > 0) || (m_uData[2] > 0) || (m_uData[3] > uValue))
		return 1;
	if (m_uData[3] < uValue)
		return -1;
	return 0;
}

void CUInt128::Add(const CUInt128 &uValue)
{
	if (uValue == 0)
		return;
	/* original version
	__int64 iSum = 0;
	for (int iIndex=3; iIndex>=0; iIndex--)
	{
		iSum += m_uData[iIndex];
		iSum += uValue.m_uData[iIndex];
		m_uData[iIndex] = (ULONG)iSum;
		iSum = iSum >> 32;
	}*/
	// 32 bit version
	bool bOverflow = false;
	for (sint_ptr iIndex=3; iIndex>=0; iIndex--)
	{
		ULONG tempval = m_uData[iIndex];
		bOverflow = (bOverflow && ++tempval == 0) || (tempval > ULONG_MAX - uValue.m_uData[iIndex]);
		m_uData[iIndex] = tempval + uValue.m_uData[iIndex];
	}
}
/*
void CUInt128::Add(ULONG uValue)
{
	if (uValue == 0)
		return;
	Add(CUInt128(uValue));
}

void CUInt128::Subtract(const CUInt128 &uValue)
{
	if (uValue == 0)
		return;
	/* original version
	__int64 iSum = 0;
	for (int iIndex=3; iIndex>=0; iIndex--)
	{
		iSum += m_uData[iIndex];
		iSum -= uValue.m_uData[iIndex];
		m_uData[iIndex] = (ULONG)iSum;
		iSum = iSum >> 32;
	}*
	// 32 bit version
	bool bOverflow = false;
	for (sint_ptr iIndex=3; iIndex>=0; iIndex--)
	{
		ULONG tempval = m_uData[iIndex];
		bOverflow = (bOverflow && --tempval == ULONG_MAX) || (tempval < uValue.m_uData[iIndex]);
		m_uData[iIndex] = tempval - uValue.m_uData[iIndex];
	}
}

void CUInt128::Subtract(ULONG uValue)
{
	if (uValue == 0)
		return;
	Subtract(CUInt128(uValue));
}

/* Untested
void CUInt128::Div(ULONG uValue)
{
	ULONG uBit, uRemain = 0;
	for (i = 0; i < 128; i++)
	{
		uBit = GetBitNumber(0);
		uRemain <<= 1;
		if (uBit)
			uRemain |= 1;
		ShiftLeft(1);
		if (uRemain >= uValue)
		{
			uRemain -= uValue;
			SetBitNumber(127, 1);
		}
	}
}
*/

void CUInt128::ShiftLeft(sint_ptr uBits)
{
	if ((uBits == 0) || (CompareTo(0) == 0))
		return;
	if (uBits > 127)
	{
		SetValue((ULONG)0);
		return;
	}

	/* original version
	ULONG uResult[] = {0,0,0,0};
	int iIndexShift = (int)uBits / 32;
	__int64 iShifted = 0;
	for (int iIndex=3; iIndex>=iIndexShift; iIndex--)
	{
		iShifted += ((__int64)m_uData[iIndex]) << (uBits % 32);
		uResult[iIndex-iIndexShift] = (ULONG)iShifted;
		iShifted = iShifted >> 32;
	}
	for (int iIndex=0; iIndex<4; iIndex++)
		m_uData[iIndex] = uResult[iIndex];

	// 32 bit version
	sint_ptr iIndex = 0;
	sint_ptr srcIndex = uBits / 32;
	uBits %= 32;
	for(;srcIndex < 3;++srcIndex,++iIndex)
	{
		m_uData[iIndex] = (m_uData[srcIndex] << uBits | m_uData[srcIndex+1] >> (32 - uBits));
	}

	m_uData[iIndex++] = m_uData[srcIndex] << uBits;

	for (;iIndex<4;++iIndex)
		m_uData[iIndex] = 0;*/
	uint64 uint128hi;
	uint64 uint128lo = (uint64)m_uData[2] << 32 | m_uData[3];
	if(uBits < 64){
		uint128hi = ((uint64)m_uData[0] << 32 | m_uData[1]) << uBits | uint128lo >> (64 - uBits);
		uint128lo = uint128lo << uBits;
	}
	else{
		uint128hi = uint128lo << (uBits - 64);
		uint128lo = 0;
	}
	m_uData[0] = uint128hi >> 32; m_uData[1] = (uint32)uint128hi;
	m_uData[2] = uint128lo >> 32; m_uData[3] = (uint32)uint128lo;
}

const byte* CUInt128::GetData() const
{
	return (byte*)m_uData;
}

byte* CUInt128::GetDataPtr() const
{
	return (byte*)m_uData;
}

ULONG CUInt128::Get32BitChunk(sint_ptr iVal) const
{
	return m_uData[iVal];
}
/*
void CUInt128::operator+  (const CUInt128 &uValue)
{
	Add(uValue);
}
void CUInt128::operator-  (const CUInt128 &uValue)
{
	Subtract(uValue);
}*/
// netfinity: Moved this inline for speed!
/*void CUInt128::operator=  (const CUInt128 &uValue)
{
	SetValue(uValue);
}
bool CUInt128::operator<  (const CUInt128 &uValue) const
{
	return (CompareTo(uValue) <  0);
}
bool CUInt128::operator>  (const CUInt128 &uValue) const
{
	return (CompareTo(uValue) >  0);
}
bool CUInt128::operator<= (const CUInt128 &uValue) const
{
	return (CompareTo(uValue) <= 0);
}
bool CUInt128::operator>= (const CUInt128 &uValue) const
{
	return (CompareTo(uValue) >= 0);
}
bool CUInt128::operator== (const CUInt128 &uValue) const
{
	return (CompareTo(uValue) == 0);
}
bool CUInt128::operator!= (const CUInt128 &uValue) const
{
	return (CompareTo(uValue) != 0);
}*/
/*
void CUInt128::operator+  (ULONG uValue)
{
	Add(uValue);
}
void CUInt128::operator-  (ULONG uValue)
{
	Subtract(uValue);
}
*/
void CUInt128::operator=  (ULONG uValue)
{
	SetValue(uValue);
}

bool CUInt128::operator<  (ULONG uValue) const
{
	return (CompareTo(uValue) <  0);
}
bool CUInt128::operator>  (ULONG uValue) const
{
	return (CompareTo(uValue) >  0);
}
bool CUInt128::operator<= (ULONG uValue) const
{
	return (CompareTo(uValue) <= 0);
}
bool CUInt128::operator>= (ULONG uValue) const
{
	return (CompareTo(uValue) >= 0);
}
bool CUInt128::operator== (ULONG uValue) const
{
	return (CompareTo(uValue) == 0);
}
bool CUInt128::operator!= (ULONG uValue) const
{
	return (CompareTo(uValue) != 0);
}

void CUInt128::Inc()
{
	for (sint_ptr iIndex=3; iIndex>=0; iIndex--)
	{
		if(++m_uData[iIndex] != 0)
			break;
	}
}