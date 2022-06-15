//this file is part of NeoMule
//Copyright (C)2006 David Xanatos ( Xanatos@Lycos.at / http://NeoMule.sf.net )
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

class CTitleMenu;

#include "PrefFunctions.h"

template<class TYPE_PTR, class TYPE1, class TYPE2>
#ifdef _TEST // NEO: TEST - [TestEdition]
void MinMax(TYPE_PTR* /*data*/, TYPE1 /*min*/, TYPE2 /*max*/){}
#else
void MinMax(TYPE_PTR *data, TYPE1 min, TYPE2 max)
{
	if (*data < min)
		*data=min;
	else if (*data > max)
		*data=max;
}
#endif // _TEST // NEO: TEST END

template<class TYPE_PTR, class TYPE>
#ifdef _TEST // NEO: TEST - [TestEdition]
void Minimal (TYPE_PTR* /*data*/, TYPE /*min*/) {}
#else
void Minimal (TYPE_PTR *data, TYPE min)
{
	if (*data < min)
		*data=min;
}
#endif // _TEST // NEO: TEST END

template<class TYPE_PTR, class TYPE>
#ifdef _TEST // NEO: TEST - [TestEdition]
void Maximal (TYPE_PTR* /*data*/, TYPE /*max*/) {}
#else
void Maximal (TYPE_PTR *data, TYPE max)
{
	if (*data > max)
		*data=max;
}
#endif // _TEST // NEO: TEST END

#define UN_INT		(INT_MAX - 1)
#define UN_FLOAT	(-1)

//////////////////////////////////
int Str2Int(CString Buffer, int Min, int Def, int Max);
CString Int2Str(int Nummer);
uint32 Str2UInt(CString Buffer, uint32 Min, uint32 Def, uint32 Max);
CString UInt2Str(uint32 Nummer);
float Str2FloatNum(CString Buffer, float Min, float Def, float Max);
CString FloatNum2Str(float Nummer);

uint32 MinStr2Time (CString Buffer, uint32 Min, uint32 Def, uint32 Max);
CString Time2MinStr(uint32 Time);
uint32 SecStr2Time (CString Buffer, uint32 Min, uint32 Def, uint32 Max);
CString Time2SecStr(uint32 Time);
/////////////////////////////////

// A very usefull container
template<class TYPE>
struct array{
	array()	{data = NULL;}
	array(int size)	{data = new TYPE[size];}
	~array()		{delete [] data;}
	TYPE* data;
	//TYPE* operator=(TYPE* t)	{ data = t; return data; } 
	operator TYPE*&()			{ return data; }
};

CString StrLine(LPCTSTR line, ...);

int CompareSubDirectories(const CString& rstrDir1, const CString& rstrDir2);

CString MkPath(CString Dir, CString file);

void UpdateMTDMenu(CMenu &menu, CString curr_folder, bool bFile); // NEO: MTD - [MultiTempDirectories] <-- Xanatos --
void UpdateCatMenu(CMenu &menu, int curr_cat, bool shared = false); // NEO: NSC - [NeoSharedCategories] <-- Xanatos --

// NEO: NMP - [NeoModProt] -- Xanatos -->
enum NanoTagID
{
	// short ID's, 1-4 bytes content 
	//				= 0
	NT_NATT			= 1,
//	NT_ISPT			= 10,
	NT_OBFU			= 15,
	//				= 31

	// long ID's, max 256 bytes content
	NT_BuddyID		= 32,
	NT_BuddyIPPort	= 33,
	NT_ServerIPPort = 34,
	NT_XsBuddyIPPort= 35,
//	NT_SecPort		= 40,
	//				= 127
};


void WriteNanoTagIDLen(byte* &pBuffer, uint8 uID, uint8 uLen = 1);
uint8 GetNanoTagID(byte* pBuffer);
uint8 GetNanoTagLen(byte* &pBuffer);
// NEO: NMP END <-- Xanatos --

CString CastSecondsToDate(uint32 Secounds);

int wildcmp(TCHAR *wild, TCHAR *string);
ULONG	CastXBytesToI(const CString& strExpr);
CString CastItoUIXBytes(uint64 count);