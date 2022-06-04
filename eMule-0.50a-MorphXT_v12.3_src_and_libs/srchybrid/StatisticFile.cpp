// parts of this file are based on work from pan One (http://home-3.tiscali.nl/~meost/pms/)
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
#include "stdafx.h"
#include "StatisticFile.h"
#include "emule.h"
#include "KnownFileList.h"
#include "SharedFileList.h"
#include "Uploadqueue.h" //MORPH START - Added by SiRoB, Equal Chance For Each File
#include "Statistics.h" //MORPH START - Added by SiRoB, Equal Chance For Each File
#include "Preferences.h" //MORPH START - Added by SiRoB, Equal Chance For Each File
#include "Log.h" //Fafner: corrupted spreadbarinfo? - 080317

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void CStatisticFile::MergeFileStats( CStatisticFile *toMerge )
{
	requested += toMerge->GetRequests();
	accepted += toMerge->GetAccepts();
	transferred += toMerge->GetTransferred();
	SetAllTimeRequests(alltimerequested + toMerge->GetAllTimeRequests());
	SetAllTimeTransferred(alltimetransferred + toMerge->GetAllTimeTransferred());
	SetAllTimeAccepts(alltimeaccepted + toMerge->GetAllTimeAccepts());

	// SLUGFILLER: Spreadbars
	if (!toMerge->spreadlist.IsEmpty()) {
		POSITION pos = toMerge->spreadlist.GetHeadPosition();
		uint64 start = toMerge->spreadlist.GetKeyAt(pos);
		uint64 count = toMerge->spreadlist.GetValueAt(pos);
		toMerge->spreadlist.GetNext(pos);
		while (pos){
			uint64 end = toMerge->spreadlist.GetKeyAt(pos);
			if (count)
				AddBlockTransferred(start, end, count);
			start = end;
			count = toMerge->spreadlist.GetValueAt(pos);
			toMerge->spreadlist.GetNext(pos);
		}
	}
	// SLUGFILLER: Spreadbars
}

void CStatisticFile::AddRequest(){
	requested++;
	alltimerequested++;
	theApp.knownfiles->requested++;
	theApp.sharedfiles->UpdateFile(fileParent);
}
	
void CStatisticFile::AddAccepted(){
	accepted++;
	alltimeaccepted++;
	theApp.knownfiles->accepted++;
	theApp.sharedfiles->UpdateFile(fileParent);
}
	
void CStatisticFile::AddTransferred(uint64 start, uint64 bytes){	//MORPH - Added by IceCream, SLUGFILLER: Spreadbars
	transferred += bytes;
	alltimetransferred += bytes;
	theApp.knownfiles->transferred += bytes;
	AddBlockTransferred(start, start+bytes/*FIX?+1*/, 1);	//MORPH - Added by IceCream, SLUGFILLER: Spreadbars
	theApp.sharedfiles->UpdateFile(fileParent);
	m_bInChangedEqualChanceValue = false;	//Morph - added by AndCycle, Equal Chance For Each File
}

void CStatisticFile::SetAllTimeRequests(uint32 nVal)
{
	alltimerequested = nVal;
}

void CStatisticFile::SetAllTimeAccepts(uint32 nVal)
{
	alltimeaccepted = nVal;
}

void CStatisticFile::SetAllTimeTransferred(uint64 nVal)
{
	alltimetransferred = nVal;
}

//MORPH START - Added by IceCream, SLUGFILLER: Spreadbars
void CStatisticFile::AddBlockTransferred(uint64 start, uint64 end, uint64 count){
	if (start >= end || !count)
		return;

	//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	if(fileParent->GetSpreadbarSetStatus() == 0 || (fileParent->GetSpreadbarSetStatus() == -1 && thePrefs.GetSpreadbarSetStatus() == 0))
		return;
	//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file

	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	InChangedSpreadSortValue = false;
	InChangedFullSpreadCount = false;
	InChangedSpreadBar = false;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	
	if (spreadlist.IsEmpty())
		spreadlist.SetAt(0, 0);

	POSITION endpos = spreadlist.FindFirstKeyAfter(end+1);

	if (endpos)
		spreadlist.GetPrev(endpos);
	else
		endpos = spreadlist.GetTailPosition();

	ASSERT(endpos != NULL);
	if (endpos == NULL) { //Fafner: corrupted spreadbarinfo? - 080317
		DebugLog(LOG_MORPH|LOG_ERROR, _T("AddBlockTransferred: No endpos in spreadbarinfo for file %s - %I64u, %I64u, %I64u"), fileParent->GetFileName(), start, end, count);
		return;
	}

	uint64 endcount = spreadlist.GetValueAt(endpos);
	endpos = spreadlist.SetAt(end, endcount);

	//Fafner: fix vs2005 corrupted spreadbarinfo? - 080317
	//Fafner: note: FindFirstKeyAfter seems to work differently under vs2005 than under vs2003
	//Fafner: note: see also similar code in CBarShader::FillRange
	//Fafner: note: also look for the keywords 'spreadbarinfo', 'barshaderinfo'
#if _MSC_VER < 1400
	POSITION startpos = spreadlist.FindFirstKeyAfter(start+1);
#else
	POSITION startpos = spreadlist.FindFirstKeyAfter(start);
#endif

	for (POSITION pos = startpos; pos != endpos && pos != NULL; spreadlist.GetNext(pos)) {
		spreadlist.SetValueAt(pos, spreadlist.GetValueAt(pos)+count);
	}

	spreadlist.GetPrev(startpos);

	ASSERT(startpos != NULL);
	if (startpos == NULL) { //Fafner: corrupted spreadbarinfo? - 080317
		DebugLog(LOG_MORPH|LOG_ERROR, _T("AddBlockTransferred: No startpos in spreadbarinfo for file %s - %I64u, %I64u, %I64u"), fileParent->GetFileName(), start, end, count);
		return;
	}
	ASSERT(startpos != endpos);
	if (startpos == endpos) { //Fafner: corrupted spreadbarinfo? - 080317
		DebugLog(LOG_MORPH|LOG_WARNING, _T("AddBlockTransferred: startpos == endpos in spreadbarinfo for file %s - %I64u, %I64u, %I64u"), fileParent->GetFileName(), start, end, count);
	}

	uint64 startcount = spreadlist.GetValueAt(startpos)+count;
	startpos = spreadlist.SetAt(start, startcount);

	POSITION prevpos = startpos;
	spreadlist.GetPrev(prevpos);
	if (prevpos && spreadlist.GetValueAt(prevpos) == startcount)
		spreadlist.RemoveAt(startpos);

	prevpos = endpos;
	spreadlist.GetPrev(prevpos);
	if (prevpos && spreadlist.GetValueAt(prevpos) == endcount)
		spreadlist.RemoveAt(endpos);
}

CBarShader CStatisticFile::s_SpreadBar(16);

//MORPH START - Changed by SiRoB, Reduce SpreadBar CPU consumption
void CStatisticFile::DrawSpreadBar(CDC* dc, RECT* rect, bool bFlat) /*const*/
{
	int iWidth=rect->right - rect->left;
	if (iWidth <= 0)	return;
	int iHeight=rect->bottom - rect->top;
	uint64 filesize = fileParent->GetFileSize()>(uint64)0?fileParent->GetFileSize():(uint64)1;
	if (m_bitmapSpreadBar == (HBITMAP)NULL)
		VERIFY(m_bitmapSpreadBar.CreateBitmap(1, 1, 1, 8, NULL)); 
	CDC cdcStatus;
	HGDIOBJ hOldBitmap;
	cdcStatus.CreateCompatibleDC(dc);

	if(!InChangedSpreadBar || lastSize!=iWidth || lastbFlat!= bFlat){
		InChangedSpreadBar = true;
		lastSize=iWidth;
		lastbFlat=bFlat;
		m_bitmapSpreadBar.DeleteObject();
		m_bitmapSpreadBar.CreateCompatibleBitmap(dc,  iWidth, iHeight); 
		m_bitmapSpreadBar.SetBitmapDimension(iWidth,  iHeight); 
		hOldBitmap = cdcStatus.SelectObject(m_bitmapSpreadBar);
			
		s_SpreadBar.SetHeight(iHeight);
		s_SpreadBar.SetWidth(iWidth);
			
		s_SpreadBar.SetFileSize(filesize);
		s_SpreadBar.Fill(RGB(0, 0, 0));

		for(POSITION pos = spreadlist.GetHeadPosition(); pos; ){
			uint64 count = spreadlist.GetValueAt(pos);
			uint64 start = spreadlist.GetKeyAt(pos);
			spreadlist.GetNext(pos);
			if (!pos)
				break;
			uint64 end = spreadlist.GetKeyAt(pos);
			if (count)
				s_SpreadBar.FillRange(start, end, RGB(0,
				(232<22*count)? 0:232-22*count
				,255));
		}
		s_SpreadBar.Draw(&cdcStatus, 0, 0, bFlat);
	}
	else
		hOldBitmap = cdcStatus.SelectObject(m_bitmapSpreadBar);
	dc->BitBlt(rect->left, rect->top, iWidth, iHeight, &cdcStatus, 0, 0, SRCCOPY);
	cdcStatus.SelectObject(hOldBitmap);
}
//MORPH END  - Changed by SiRoB, Reduce SpreadBar CPU consumption

float CStatisticFile::GetSpreadSortValue() /*const*/
{
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	if (InChangedSpreadSortValue)
		return lastSpreadSortValue;
	InChangedSpreadSortValue=true;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	float avg, calc;
	uint64 total = 0;
	uint64 filesize = fileParent->GetFileSize();

	if (!filesize || spreadlist.IsEmpty())
		return 0;

	POSITION pos = spreadlist.GetHeadPosition();
	uint64 start = spreadlist.GetKeyAt(pos);
	uint64 count = spreadlist.GetValueAt(pos);
	spreadlist.GetNext(pos);
	while (pos){
		uint64 end = spreadlist.GetKeyAt(pos);
		total += (end-start)*count;
		start = end;
		count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	}

	avg = (float)total/filesize;
	calc = 0;
	pos = spreadlist.GetHeadPosition();
	start = spreadlist.GetKeyAt(pos);
	count = spreadlist.GetValueAt(pos);
	spreadlist.GetNext(pos);
	while (pos){
		uint64 end = spreadlist.GetKeyAt(pos);
		if ((float)count > avg)
			calc += avg*(end-start);
		else
			calc += count*(end-start);
		start = end;
		count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	}
	calc /= filesize;
	//MORPH START - Added by SiRoB, Reduce Spread CPU consumption
	lastSpreadSortValue = calc;
	return calc;
	//MORPH START - Added by SiRoB, Reduce Spread CPU consumption
}

float CStatisticFile::GetFullSpreadCount() /*const*/
{
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	if (InChangedFullSpreadCount) return lastFullSpreadCount;
	InChangedFullSpreadCount=true;
	//MORPH START - Added by SiRoB, Reduce SpreedBar CPU consumption
	float next;
	uint64 min;
	uint64 filesize = fileParent->GetFileSize();

	if (!filesize || spreadlist.IsEmpty())
		return 0;

	POSITION pos = spreadlist.GetHeadPosition();
	min = spreadlist.GetValueAt(pos);
	spreadlist.GetNext(pos);
	while (pos && spreadlist.GetKeyAt(pos) < filesize){
		uint64 count = spreadlist.GetValueAt(pos);
		if (min > count)
			min = count;
		spreadlist.GetNext(pos);
	}

	next = 0;
	pos = spreadlist.GetHeadPosition();
	uint64 start = spreadlist.GetKeyAt(pos);
	uint64 count = spreadlist.GetValueAt(pos);
	spreadlist.GetNext(pos);
	while (pos){
		uint64 end = spreadlist.GetKeyAt(pos);
		if (count > min)
			next += end-start;
		start = end;
		count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	}
	next /= filesize;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	return lastFullSpreadCount = min+next;
	//MORPH END   - Added by SiRoB, Reduce SpreadBar CPU consumption
}
//MORPH END   - Added by IceCream, SLUGFILLER: Spreadbars

//MORPH	Start	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
void CStatisticFile::ResetSpreadBar()
{
	spreadlist.RemoveAll();
	spreadlist.SetAt(0, 0);
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	InChangedSpreadSortValue = false;
	InChangedFullSpreadCount = false;
	InChangedSpreadBar = false;
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	return;
}
//MORPH	End	- Added by AndCycle, SLUGFILLER: Spreadbars - per file

//Morph Start - added by AndCycle, Equal Chance For Each File
double CStatisticFile::GetEqualChanceValue()
{
	if(!thePrefs.IsEqualChanceEnable()){
		return 0;
	}
	//Morph - Added by AndCycle, Equal Chance For Each File, reduce CPU power
	else if(m_bInChangedEqualChanceValue && (lastCheckEqualChanceSemiValue + theApp.uploadqueue->GetAverageUpTime()) > (uint32)time(NULL)){
		return m_dLastEqualChanceSemiValue/GetSessionShareTime();
	}
	lastCheckEqualChanceSemiValue = time(NULL);
	m_bInChangedEqualChanceValue = true;
	//Morph - Added by AndCycle, Equal Chance For Each File, reduce CPU power

	//smaller value means greater priority
	m_dLastEqualChanceSemiValue = ((double)GetTransferred()/(uint64)fileParent->GetFileSize());

	//weight adjustment
	if(theApp.uploadqueue->GetSuccessfullUpCount() > 0){
		uint64 threshold = (UINT)(theStats.GetAvgUploadRate(AVG_SESSION)*1024*theApp.uploadqueue->GetAverageUpTime());
		if(fileParent->GetFileSize() < threshold){
			m_dLastEqualChanceBiasValue = 1+log((double)threshold/((uint64)fileParent->GetFileSize()%threshold+1));
			m_dLastEqualChanceSemiValue /= m_dLastEqualChanceBiasValue;
		}
	}
	return m_dLastEqualChanceSemiValue/GetSessionShareTime();
}

CString CStatisticFile::GetEqualChanceValueString(bool detail) const
{
	CString tempString;

	if(thePrefs.IsEqualChanceEnable())	{
		if(m_dLastEqualChanceBiasValue != 1){
			detail ?
				tempString.Format(_T("%s : %.2f*%.2f = %s/%s"), CastSecondsToHM(GetSessionShareTime()), m_dLastEqualChanceSemiValue, m_dLastEqualChanceBiasValue, CastItoXBytes(GetTransferred()), CastItoXBytes(fileParent->GetFileSize())) :
				tempString.Format(_T("%s : %.2f*%.2f"), CastSecondsToHM(GetSessionShareTime()), m_dLastEqualChanceSemiValue, m_dLastEqualChanceBiasValue) ;
		}
		else{
			detail ?
				tempString.Format(_T("%s : %.2f = %s/%s"), CastSecondsToHM(GetSessionShareTime()), m_dLastEqualChanceSemiValue, CastItoXBytes(GetTransferred()), CastItoXBytes(fileParent->GetFileSize())) :
				tempString.Format(_T("%s : %.2f"), CastSecondsToHM(GetSessionShareTime()), m_dLastEqualChanceSemiValue) ;
		}
	}
	else{
		tempString.Empty();
	}

	return tempString;
}
//Morph End - added by AndCycle, Equal Chance For Each File

//EastShare	Start - FairPlay by AndCycle
bool	CStatisticFile::GetFairPlay() const
{
	//should only judge simple UL or is there any better replacement?
	//Stulle: rewrote the code to ensure bug free compiling
	double dShareFactor = ((double)GetAllTimeTransferred())/((double)fileParent->GetFileSize());
	if(dShareFactor < thePrefs.GetFairPlay())
		return true;
	return false;
}
//EastShare	End   - FairPlay by AndCycle
