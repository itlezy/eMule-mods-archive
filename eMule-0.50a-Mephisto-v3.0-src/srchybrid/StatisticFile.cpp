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
#include "KnownFile.h" //Xman PowerRelease

//Xman advanced upload-priority
#ifdef _BETA
#include "Log.h"
#include "otherfunctions.h"
#endif
//Xman end
// ==> Spread bars [Slugfiller/MorphXT] - Stulle
#include "Preferences.h"
#include "Log.h"
// <== Spread bars [Slugfiller/MorphXT] - Stulle

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// ==> Removed Spreadbars (old version) [SlugFiller] - Stulle
/*
//Xman PowerRelease
// SLUGFILLER: Spreadbars (old version)
CStatisticFile::~CStatisticFile(){
	for (POSITION pos = spreadlist.GetHeadPosition();pos != 0;spreadlist.GetNext(pos))
		delete spreadlist.GetAt(pos);
}
//Xman end
*/
// <== Removed Spreadbars (old version) [SlugFiller] - Stulle

void CStatisticFile::MergeFileStats( CStatisticFile *toMerge )
{
	requested += toMerge->GetRequests();
	accepted += toMerge->GetAccepts();
	transferred += toMerge->GetTransferred();
	SetAllTimeRequests(alltimerequested + toMerge->GetAllTimeRequests());
	SetAllTimeTransferred(alltimetransferred + toMerge->GetAllTimeTransferred());
	SetAllTimeAccepts(alltimeaccepted + toMerge->GetAllTimeAccepts());
	//Xman advanced upload-priority
	m_unotcountedtransferred += toMerge->m_unotcountedtransferred;
	if(m_unotcountedtransferred > alltimetransferred)
		m_unotcountedtransferred = alltimetransferred;
	//Xman end

#ifdef _BETA
	AddDebugLogLine(false,_T("merged file stats: %s"), toMerge->fileParent->GetFileName());
#endif

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
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
	// <== Spread bars [Slugfiller/MorphXT] - Stulle
}

void CStatisticFile::AddRequest(){
	requested++;
	alltimerequested++;
	theApp.knownfiles->requested++;
	//Xman Code Improvement -> don't update to often
	m_uFileupdatetime=::GetTickCount(); 
	theApp.sharedfiles->UpdateFile(fileParent);
	//Xman end
}
	
void CStatisticFile::AddAccepted(){
	accepted++;
	alltimeaccepted++;
	theApp.knownfiles->accepted++;
	//Xman Code Improvement -> don't update to often
	m_uFileupdatetime=::GetTickCount(); 
	theApp.sharedfiles->UpdateFile(fileParent);
	//Xman end
}
	
//Xman PowerRelease
/*
void CStatisticFile::AddTransferred(uint64 bytes){
*/
// ==> Removed Spreadbars (old version) [SlugFiller] - Stulle
/*
void CStatisticFile::AddTransferred(uint64 start, uint32 bytes){
*/
// <== Removed Spreadbars (old version) [SlugFiller] - Stulle
void CStatisticFile::AddTransferred(uint64 start, uint64 bytes){ // Spread bars [Slugfiller/MorphXT] - Stulle
//Xman end
	transferred += bytes;
	alltimetransferred += bytes;
	theApp.knownfiles->transferred += bytes;
	//Xman PowerRelease
	// ==> Removed Spreadbars (old version) [SlugFiller] - Stulle
	/*
	if(!fileParent->IsPartFile() && fileParent->GetED2KPartCount()>3)
	*/
	// <== Removed Spreadbars (old version) [SlugFiller] - Stulle
		AddBlockTransferred(start, start+bytes/*+1*/, 1); //Xman David
	//Xman end
	//Xman Code Improvement -> don't update to often
	if(m_uFileupdatetime + 1000 < ::GetTickCount()) //once per second
	{
		m_uFileupdatetime=::GetTickCount();
		theApp.sharedfiles->UpdateFile(fileParent);
	}
	//Xman end
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

// ==> Removed Spreadbars (old version) [SlugFiller] - Stulle
/*
//Xman PowerRelease
// SLUGFILLER: Spreadbars (old version)
void CStatisticFile::AddBlockTransferred(uint64 start, uint64 end, uint32 count){
	if (start >= end || !count)
		return;

	if (spreadlist.IsEmpty()) {
		if (start > 0) {
			Spread_Struct* gap_spread = new Spread_Struct;
			gap_spread->start = 0;
			gap_spread->end = start;
			gap_spread->count = 0;
			spreadlist.AddTail(gap_spread);
		}
		Spread_Struct* new_spread = new Spread_Struct;
		new_spread->start = start;
		new_spread->end = end;
		new_spread->count = count;
		spreadlist.AddTail(new_spread);
		return;
	}

	POSITION pos = spreadlist.GetTailPosition();
	Spread_Struct* cur_spread = spreadlist.GetAt(pos);

	if (cur_spread->end <= end) {
		if (cur_spread->end <= start) {
			if (cur_spread->end == start) {
				if (cur_spread->count == count) {
					cur_spread->end = end;
					return;
				}
			} else {
				Spread_Struct* gap_spread = new Spread_Struct;
				gap_spread->start = cur_spread->end;
				gap_spread->end = start;
				gap_spread->count = 0;
				spreadlist.AddTail(gap_spread);
			}
			Spread_Struct* new_spread = new Spread_Struct;
			new_spread->start = start;
			new_spread->end = end;
			new_spread->count = count;
			spreadlist.AddTail(new_spread);
			return;
		}
		if (cur_spread->end == end) {
			if (cur_spread->start == start) {
				cur_spread->count += count;
				return;
			}
			if (cur_spread->start < start) {
				cur_spread->end = start;
				Spread_Struct* new_spread = new Spread_Struct;
				new_spread->start = start;
				new_spread->end = end;
				new_spread->count = count+cur_spread->count;
				spreadlist.AddTail(new_spread);
				return;
			}
		} else {
			Spread_Struct* new_spread = new Spread_Struct;
			new_spread->start = cur_spread->end;
			new_spread->end = end;
			new_spread->count = count;
			spreadlist.AddTail(new_spread);
		}
		while (cur_spread->start > start) {
			cur_spread->count += count;
			spreadlist.GetPrev(pos);
			cur_spread = spreadlist.GetAt(pos);
		}
		if (cur_spread->start == start) {
			cur_spread->count += count;
			spreadlist.GetPrev(pos);
			if (!pos)
				return;
			Spread_Struct* next_spread = spreadlist.GetAt(pos);
			if (next_spread->count != cur_spread->count)
				return;
			cur_spread->start = next_spread->start;
			spreadlist.RemoveAt(pos);
			delete next_spread;
			return;
		}
		Spread_Struct* new_spread = new Spread_Struct;
		new_spread->start = start;
		new_spread->end = cur_spread->end;
		new_spread->count = count+cur_spread->count;
		spreadlist.InsertAfter(pos,new_spread);
		cur_spread->end = start;
		return;
	}

	pos = spreadlist.GetHeadPosition();
	cur_spread = spreadlist.GetAt(pos);
	while (cur_spread->end < start) {
		spreadlist.GetNext(pos);
		cur_spread = spreadlist.GetAt(pos);
	}
	if (cur_spread->end == start) {
		POSITION pos1 = pos;
		spreadlist.GetNext(pos);
		Spread_Struct* next_spread = spreadlist.GetAt(pos);
		if (next_spread->count+count != cur_spread->count)
			cur_spread = next_spread;
		else {
			cur_spread->count = next_spread->count;
			cur_spread->end = next_spread->end;
			spreadlist.RemoveAt(pos);
			delete next_spread;
			pos = pos1;
		}
	} else if (cur_spread->start < start) {
		Spread_Struct* new_spread = new Spread_Struct;
		new_spread->start = start;
		new_spread->end = cur_spread->end;
		new_spread->count = cur_spread->count;
		spreadlist.InsertAfter(pos,new_spread);
		cur_spread->end = start;
		spreadlist.GetNext(pos);
		cur_spread = new_spread;
	}
	while (cur_spread->end < end) {
		cur_spread->count += count;
		spreadlist.GetNext(pos);
		cur_spread = spreadlist.GetAt(pos);
	}
	if (cur_spread->end == end) {
		cur_spread->count += count;
		spreadlist.GetNext(pos);
		Spread_Struct* next_spread = spreadlist.GetAt(pos);
		if (next_spread->count != cur_spread->count)
			return;
		cur_spread->end = next_spread->end;
		spreadlist.RemoveAt(pos);
		delete next_spread;
		return;
	}
	Spread_Struct* new_spread = new Spread_Struct;
	new_spread->start = end;
	new_spread->end = cur_spread->end;
	new_spread->count = cur_spread->count;
	spreadlist.InsertAfter(pos,new_spread);
	cur_spread->count += count;
	cur_spread->end = end;
}
//Xman end
*/
// <== Removed Spreadbars (old version) [SlugFiller] - Stulle

//Xman advanced upload-priority
void CStatisticFile::UpdateCountedTransferred() 
{
	ASSERT(m_tlastdataupdate!=0);
	ASSERT(m_unotcountedtransferred <= alltimetransferred);
	if(time(NULL) - m_tlastdataupdate > 3600 * 12) // every 12 hours
	{
#ifdef _BETA
		uint64 oldcounted=GetCountedTransferred();
#endif
		//we subtract every day 10% from the counted upload
		//this means, if you close emule for >=10 days and restart, no old upload is counted for Auto-Prio
		//also if there are no uploads for longer time the file get pushed via Auto-Prio
		//remark: when emule is running 10 days, the counted upload will never reach 0, because always subtracting 10%
		//however this method is the best one without adding lot of more information to the met files
		//and it's crash-safe
		uint32 difference = (time(NULL)-m_tlastdataupdate)/(3600*12); //in half days
		if(difference>20) difference = 20;
		//every half day = 5%
		difference *=5;
		if(difference>=100) //manual to avoid overflow because rounding errors
			m_unotcountedtransferred = alltimetransferred;
		else
			m_unotcountedtransferred += (uint64)((double)GetCountedTransferred() * ((double)difference/100));
#ifdef _BETA
		AddDebugLogLine(false,_T("--> Calc file stats: old counted upload: %s, new counted upload: %s, half days difference: %u%%, file: %s"),CastItoXBytes(oldcounted),CastItoXBytes(GetCountedTransferred()),difference,fileParent->GetFileName());
#endif
		m_tlastdataupdate=time(NULL);
	}
}
//Xman end

// ==> Spread bars [Slugfiller/MorphXT] - Stulle
void CStatisticFile::AddBlockTransferred(uint64 start, uint64 end, uint64 count){
	if (start >= end || !count)
		return;

	if(thePrefs.GetSpreadbarSetStatus() == false)
		return;

	InChangedSpreadSortValue = false;
	InChangedFullSpreadCount = false;
	InChangedSpreadBar = false;
	
	if (spreadlist.IsEmpty())
		spreadlist.SetAt(0, 0);

	POSITION endpos = spreadlist.FindFirstKeyAfter(end+1);

	if (endpos)
		spreadlist.GetPrev(endpos);
	else
		endpos = spreadlist.GetTailPosition();

	ASSERT(endpos != NULL);
	if (endpos == NULL) { //Fafner: corrupted spreadbarinfo? - 080317
		AddDebugLogLine(false, _T("AddBlockTransferred: No endpos in spreadbarinfo for file %s - %I64u, %I64u, %I64u"), fileParent->GetFileName(), start, end, count);
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
		AddDebugLogLine(false, _T("AddBlockTransferred: No startpos in spreadbarinfo for file %s - %I64u, %I64u, %I64u"), fileParent->GetFileName(), start, end, count);
		return;
	}
	ASSERT(startpos != endpos);
	if (startpos == endpos) { //Fafner: corrupted spreadbarinfo? - 080317
		AddDebugLogLine(false, _T("AddBlockTransferred: startpos == endpos in spreadbarinfo for file %s - %I64u, %I64u, %I64u"), fileParent->GetFileName(), start, end, count);
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

float CStatisticFile::GetSpreadSortValue() /*const*/
{
	if (InChangedSpreadSortValue) return lastSpreadSortValue;
	InChangedSpreadSortValue=true;
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
	lastSpreadSortValue = calc;
	return calc;
}

float CStatisticFile::GetFullSpreadCount() /*const*/
{
	if (InChangedFullSpreadCount) return lastFullSpreadCount;
	InChangedFullSpreadCount=true;
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
	return lastFullSpreadCount = min+next;
}

void CStatisticFile::ResetSpreadBar()
{
	spreadlist.RemoveAll();
	spreadlist.SetAt(0, 0);
	InChangedSpreadSortValue = false;
	InChangedFullSpreadCount = false;
	InChangedSpreadBar = false;
	return;
}
// <== Spread bars [Slugfiller/MorphXT] - Stulle

// ==> Fair Play [AndCycle/Stulle] - Stulle
bool	CStatisticFile::GetFairPlay() const
{
	//should only judge simple UL or is there any better replacement?
	//Stulle: rewrote the code to ensure bug free compiling
	double dShareFactor = ((double)GetAllTimeTransferred())/((double)fileParent->GetFileSize());
	if(dShareFactor < thePrefs.GetFairPlay())
		return true;
	return false;
}
// <== Fair Play [AndCycle/Stulle] - Stulle