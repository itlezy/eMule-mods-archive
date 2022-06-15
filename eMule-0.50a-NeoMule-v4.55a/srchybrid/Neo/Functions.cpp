//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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

#include "stdafx.h"
#include "Emule.h"
#include "Opcodes.h"
#include "Functions.h"
#include "resource.h"
#include "Preferences.h"
#include "MenuCmds.h"
#include "OtherFunctions.h"
#include "TitleMenu.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --

int Str2Int(CString Buffer, int Min, int Def, int Max){
	int itemp = _tstol(Buffer);
	if (!itemp)
		itemp = Def;
	else
		if(Min && Max)
			MinMax (&itemp,Min,Max);
		else if(Min)
			Minimal (&itemp,Min);
	return itemp;
}
CString Int2Str(int Nummer) { 
	CString buffer=_T("0"); 
	buffer.Format(_T("%d"), Nummer); 
	return buffer; 
}

uint32 Str2UInt(CString Buffer, uint32 Min, uint32 Def, uint32 Max){
	uint32 itemp = _tstol(Buffer);
	if (!itemp)
		itemp = Def;
	else
		if(Min && Max)
			MinMax (&itemp,Min,Max);
		else if(Min)
			Minimal (&itemp,Min);
	return itemp;
}
CString UInt2Str(uint32 Nummer) { 
	CString buffer=_T("0"); 
	buffer.Format(_T("%u"), Nummer); 
	return buffer; 
}

float Str2FloatNum(CString Buffer, float Min, float Def, float Max){
	float ftemp = (float)_tstof(Buffer);
	if (!ftemp)
		ftemp = (float)Def;
	else
		if(Min && Max)
			MinMax (&ftemp,(float)Min,(float)Max);
		else if(Min)
			Minimal (&ftemp,(float)Min);
	return ftemp;
}
CString FloatNum2Str(float Nummer) { 
	CString buffer=_T("0.00"); 
	buffer.Format(_T("%.2f"),Nummer); 
	return buffer; 
}

uint32 MinStr2Time (CString Buffer, uint32 Min, uint32 Def, uint32 Max){
	double dtemp = _tstof(Buffer);
	uint32 itemp = (uint32)MIN2MS(dtemp);
	if (!dtemp)
		itemp = Def;
	else
		if(Min && Max)
			MinMax (&itemp,Min,Max);
		else if(Min)
			Minimal (&itemp,Min);
	return itemp;
}
CString Time2MinStr(uint32 Time) { 
	CString buffer=_T("0.00"); 
	buffer.Format(_T("%.2f"),MS2MIN((float)Time)); 
	return buffer; 
}

uint32 SecStr2Time (CString Buffer, uint32 Min, uint32 Def, uint32 Max){
	double dtemp = _tstof(Buffer);
	uint32 itemp = (uint32)SEC2MS(dtemp);
	if (!dtemp)
		itemp = Def;
	else
		if(Min && Max)
			MinMax (&itemp,Min,Max);
		else if(Min)
			Minimal (&itemp,Min);
	return itemp;
}
CString Time2SecStr(uint32 Time) { 
	CString buffer=_T("0.00"); 
	buffer.Format(_T("%.2f"),MS2SEC((float)Time)); 
	return buffer; 
}

CString StrLine(LPCTSTR line, ...)
{
	ASSERT(line != NULL);

	va_list argptr;
	va_start(argptr, line);
	const size_t bufferSize = 1000;
	TCHAR bufferline[bufferSize];	
	if (_vsnwprintf(bufferline, bufferSize, line, argptr) == -1)
		bufferline[bufferSize - 1] = _T('\0');
	va_end(argptr);	

	return bufferline;
}

int CompareSubDirectories(const CString& rstrDir1, const CString& rstrDir2) // full file path // master directory
{
	if(rstrDir2.IsEmpty()) // for NSC
		return -3;

	if (CompareDirectories(rstrDir1,rstrDir2)==0)
		return 0;

	if(rstrDir1.GetLength() <= rstrDir2.GetLength())
		return -1;

	CString strDir2 = rstrDir2;
	if (strDir2.Right(1) == "\\")
		strDir2 = strDir2.Left(strDir2.GetLength()-1);

	if(StrStrI(rstrDir1,strDir2))
		return 0;
	else
		return -2;
}

CString MkPath(CString Dir, CString file)
{
	CString path = Dir;
	if(path.Right(1) != _T("\\"))
		path.Append(_T("\\"));
	path.Append(file);
	return path;
}

// NEO: MTD - [MultiTempDirectories] -- Xanatos -->
void UpdateMTDMenu(CMenu &menu, CString curr_folder, bool bFile)
{
	bool bFound = false;

	if(!bFile){
		menu.AppendMenu(MF_STRING,MP_TEMPAUTO, GetResString(IDS_X_AUTO_TEMP_DIR));
		menu.AppendMenu(MF_SEPARATOR);
		if(NeoPrefs.GetUsedTempDir() == AUTO_TEMPDIR){
			menu.CheckMenuItem(MP_TEMPAUTO, MF_CHECKED);
			bFound = true;
		}
	}

	CString label;
	if (thePrefs.GetTempDirCount()) {
		for (int i = 0; i < thePrefs.GetTempDirCount(); i++){
			CString folder = thePrefs.GetTempDir(i);
			menu.AppendMenu(MF_STRING,MP_TEMPLIST+i, folder );
			if (!bFound && !CompareDirectories(curr_folder, folder)){
				bFound = true;
				menu.CheckMenuItem(MP_TEMPLIST+i, MF_CHECKED);
			}
		}
	}

	//if (!bFile && !bFound){
	//	thePrefs.SetUsedTempDir(0);
	//	menu.CheckMenuItem(MP_TEMPLIST, MF_CHECKED);
	//}
}
// NEO: MTD END <-- Xanatos --

// NEO: NSC - [NeoSharedCategories] -- Xanatos -->
void UpdateCatMenu(CMenu &menu, int curr_cat, bool shared)
{
	bool bFound = false;
	CString label;

	if(thePrefs.GetFullCatCount() > 1)
		menu.AppendMenu(MF_STRING,MP_ASSIGNCAT, GetResString(IDS_CAT_UNASSIGN));

	if (thePrefs.GetCatCount()>1) {
		for (int i = 1; i < thePrefs.GetCatCount(); i++){
			label=thePrefs.GetCategory(i)->strTitle;
			label.Replace(_T("&"), _T("&&") );
			menu.AppendMenu(MF_STRING,MP_ASSIGNCAT+i, label);
			if (!bFound && i == curr_cat){
				bFound = true;
				menu.CheckMenuItem(MP_ASSIGNCAT+i, MF_CHECKED);
			}
		}
	}

	if (shared && thePrefs.GetFullCatCount() > thePrefs.GetCatCount()) {
		menu.AppendMenu(MF_SEPARATOR);
		for (int i = thePrefs.GetCatCount(); i < thePrefs.GetFullCatCount(); i++){

			label=thePrefs.GetCategory(i)->strTitle;
			label.Replace(_T("&"), _T("&&") );
			
			menu.AppendMenu(MF_STRING,MP_ASSIGNCAT+i, label);
			if (!bFound && i == curr_cat){
				bFound = true;
				menu.CheckMenuItem(MP_ASSIGNCAT+i, MF_CHECKED);
			}
		}
	}

	if (!bFound)
		menu.CheckMenuItem(MP_ASSIGNCAT, MF_CHECKED);
}
// NEO: NSC END <-- Xanatos --

// NEO: NMP - [NeoModProt] -- Xanatos -->
void WriteNanoTagIDLen(byte* &pBuffer, uint8 uID, uint8 uLen)
{
	ASSERT((uLen <= 4 ) || (uID >= 32)); // be carefull with the id's
	// Note: we allow Long ID's with less than 5 chars content

	bool bShort = (uID < 32);

	(byte&)*pBuffer  = (bShort ? 0x00 : 0x01				<< 0); 
	(byte&)*pBuffer |= ((uID	 & (bShort ? 0x1f : 0x7f))	<< 1);

	if(bShort){
		(byte&)*pBuffer |= (((uLen-1) & 0x03)				<< 6);
	}else{
		pBuffer++;
		(byte&)*pBuffer = uLen;
	}
	pBuffer++;
}

uint8 GetNanoTagID(byte* pBuffer)
{
	return ((byte)*pBuffer >> 1) & (((byte)*pBuffer & 0x01) ? 0x7f : 0x1f);
}

uint8 GetNanoTagLen(byte* &pBuffer)
{
	uint8 uLen;
	if((byte)*pBuffer & 0x01){
		pBuffer++;
		uLen = (byte)*pBuffer;
	}else{
		uLen = (((byte)*pBuffer >> 6) & 0x03)+1;
	}
	pBuffer++;
	return uLen;
}
// NEO: NMP END <-- Xanatos --

CString CastSecondsToDate(uint32 Secounds)
{
	time_t timer = Secounds;
	WCHAR* tmp = _wctime(&timer);
	CString ret = tmp ? tmp : _T("?");
	ret.Replace(_T("\n"),_T(""));
	return ret;
}

// Compares strings using wildcards * and ?.
int wildcmp(TCHAR *wild, TCHAR *string)
{
	TCHAR *cp = NULL;
	TCHAR *mp = NULL;

	while ((*string) && (*wild != '*'))
	{
		if ((*wild != *string) && (*wild != '?'))
			return 0;
		wild++;
		string++;
	}

	while (*string)
	{
		if (*wild == '*')
		{
			if (!*++wild)
				return 1;
			mp = wild;
			cp = string+1;
		}
		else if ((*wild == *string) || (*wild == '?'))
		{
			wild++;
			string++;
		}
		else
		{
			wild = mp;
			string = cp++;
		}
	}

	while (*wild == '*')
		wild++;
	return !*wild;
}


ULONG CastXBytesToI(const CString& strExpr)
{
	ULONG ulNum;
	TCHAR szUnit[40];
	int iArgs = _stscanf(strExpr, _T("%u%s"), &ulNum, szUnit);
	if (iArgs <= 0)
		return 0;
	if (iArgs == 2){
		CString strUnits(szUnit);
		strUnits.Trim();
		if (!strUnits.IsEmpty()){
			CString strBytes = GetResString(IDS_BYTES);
			if (strUnits.CompareNoCase(strBytes.Left(1)) == 0 || strUnits.CompareNoCase(strBytes) == 0)
				return ulNum * 1U; // Bytes
			else if (strUnits.CompareNoCase(_T("k")) == 0 || strUnits.CompareNoCase(GetResString(IDS_KBYTES)) == 0 || strUnits.CompareNoCase(_T("k")+strBytes) == 0)
				return ulNum * 1024U; // KBytes
			else if (strUnits.CompareNoCase(_T("m")) == 0 || strUnits.CompareNoCase(GetResString(IDS_MBYTES)) == 0 || strUnits.CompareNoCase(_T("m")+strBytes) == 0)
				return ulNum * 1024U*1024; // MBytes
			else if (strUnits.CompareNoCase(_T("g")) == 0 || strUnits.CompareNoCase(GetResString(IDS_GBYTES)) == 0 || strUnits.CompareNoCase(_T("g")+strBytes) == 0)
				return ulNum * 1024U*1024U*1024U; // GBytes
			else
				return 0;
		}
	}

	return ulNum * 1024U*1024U; // Default = MBytes
}

CString CastItoUIXBytes(uint64 count)
{
	CString buffer;
	if (count < 1024)
		buffer.Format(_T("%I64u%s"), count, GetResString(IDS_BYTES));
	else if (count < 1048576)
		buffer.Format(_T("%I64u%s"), (uint64)(count/1024), GetResString(IDS_KBYTES));
	else if (count < 1073741824)
		buffer.Format(_T("%I64u%s"), (uint64)(count/1048576), GetResString(IDS_MBYTES));
	else
		buffer.Format(_T("%I64u%s"), (uint64)(count/1073741824), GetResString(IDS_GBYTES));
	return buffer;
}
