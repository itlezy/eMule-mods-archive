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

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Xman PowerRelease
// SLUGFILLER: Spreadbars (old version)
CStatisticFile::~CStatisticFile(){
	for (POSITION pos = spreadlist.GetHeadPosition();pos != 0;spreadlist.GetNext(pos))
		delete spreadlist.GetAt(pos);
}
//Xman end

void CStatisticFile::MergeFileStats( CStatisticFile *toMerge )
{
	requested += toMerge->GetRequests();
	accepted += toMerge->GetAccepts();
	transferred += toMerge->GetTransferred();
	alltimerequested += toMerge->GetAllTimeRequests();
	alltimetransferred += toMerge->GetAllTimeTransferred();
	alltimeaccepted += toMerge->GetAllTimeAccepts();
	//Xman advanced upload-priority
	m_unotcountedtransferred += toMerge->m_unotcountedtransferred;
	if(m_unotcountedtransferred > alltimetransferred)
		m_unotcountedtransferred = alltimetransferred;
	//Xman end
#ifdef _BETA
	AddDebugLogLine(false,_T("merged file stats: %s"), toMerge->fileParent->GetFileName());
#endif
}

void CStatisticFile::AddRequest(){
	requested++;
	alltimerequested++;
	theAppPtr->knownfiles->requested++;
	//Xman Code Improvement -> don't update to often
	m_uFileupdatetime=::GetTickCount(); 
	theAppPtr->sharedfiles->UpdateFile(fileParent);
	//Xman end
}
	
void CStatisticFile::AddAccepted(){
	accepted++;
	alltimeaccepted++;
	theAppPtr->knownfiles->accepted++;
	//Xman Code Improvement -> don't update to often
	m_uFileupdatetime=::GetTickCount(); 
	theAppPtr->sharedfiles->UpdateFile(fileParent);
	//Xman end
}
	
void CStatisticFile::AddTransferred(uint64 start, uint32 bytes){ //Xman PowerRelease
	transferred += bytes;
	alltimetransferred += bytes;
	theAppPtr->knownfiles->transferred += bytes;
	//Xman PowerRelease
	if(!fileParent->IsPartFile() && fileParent->GetED2KPartCount()>3)
		AddBlockTransferred(start, start+bytes/*+1*/, 1); //Xman David
	//Xman end
	//Xman Code Improvement -> don't update to often
	if(m_uFileupdatetime + 1000 < ::GetTickCount()) //once per second
	{
		m_uFileupdatetime=::GetTickCount();
		theAppPtr->sharedfiles->UpdateFile(fileParent);
	}
	//Xman end
}	

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
