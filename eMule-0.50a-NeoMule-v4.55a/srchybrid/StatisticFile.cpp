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
// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
#include "KnownFile.h" 
#include "Preferences.h"
#include "Log.h"
#include "emuledlg.h"
#include "Neo/NeoOpCodes.h"
#include <io.h>
#include "Packets.h"
#include "SafeFile.h"
// NEO: NPT END <-- Xanatos --
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --
#include "Neo/FilePreferences.h" // NEO: FCFG - [FileConfiguration] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define TRAFFIC_MET_FILENAME	_T("traffic.met") // NEO: NPT - [NeoPartTraffic] <-- Xanatos --

void CStatisticFile::MergeFileStats( CStatisticFile *toMerge )
{
	requested += toMerge->GetRequests();
	accepted += toMerge->GetAccepts();
	transferred += toMerge->GetTransferred();
	alltimerequested += toMerge->GetAllTimeRequests();
	alltimetransferred += toMerge->GetAllTimeTransferred();
	alltimeaccepted += toMerge->GetAllTimeAccepts();

	// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
	MergeSpreadList(sessionspreadlist,toMerge->sessionspreadlist);
	MergeSpreadList(alltimespreadlist,toMerge->alltimespreadlist);
	// NEO: NPT END <-- Xanatos --
}

void CStatisticFile::AddRequest(){
	requested++;
	alltimerequested++;
	theApp.knownfiles->requested++;
	// NEO: CI#10 - [CodeImprovement]  -- Xanatos -->
	if(guifileupdatetime + 1000 < ::GetTickCount()) //once per second
	{
		guifileupdatetime = ::GetTickCount();
		theApp.sharedfiles->UpdateFile(fileParent);
	}
	// NEO: CI#10 END <-- Xanatos --
	//theApp.sharedfiles->UpdateFile(fileParent);
}
	
void CStatisticFile::AddAccepted(){
	accepted++;
	alltimeaccepted++;
	theApp.knownfiles->accepted++;
	// NEO: CI#10 - [CodeImprovement]  -- Xanatos -->
	if(guifileupdatetime + 1000 < ::GetTickCount()) //once per second
	{
		guifileupdatetime = ::GetTickCount();
		theApp.sharedfiles->UpdateFile(fileParent);
	}
	// NEO: CI#10 END <-- Xanatos --
	//theApp.sharedfiles->UpdateFile(fileParent);
}
	
//void CStatisticFile::AddTransferred(uint64 bytes){
void CStatisticFile::AddTransferred(uint64 start, uint32 bytes){ // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	transferred += bytes;
	alltimetransferred += bytes;
	theApp.knownfiles->transferred += bytes;
	// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
	if(NeoPrefs.UsePartTraffic()){
		AddBlockTransferred(start, start+bytes, 1, alltimespreadlist); 
		AddBlockTransferred(start, start+bytes, 1, sessionspreadlist); 
		RecalcCompleteReleases();
	}
	// NEO: NPT END <-- Xanatos --
	// NEO: CI#10 - [CodeImprovement]  -- Xanatos -->
	if(guifileupdatetime + 1000 < ::GetTickCount()) //once per second
	{
		guifileupdatetime = ::GetTickCount();
		theApp.sharedfiles->UpdateFile(fileParent);
	}
	// NEO: CI#10 END <-- Xanatos --
	//theApp.sharedfiles->UpdateFile(fileParent);
}

// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
void CStatisticFile::AddBlockTransferred(uint64 start, uint64 end, uint32 count, CRBMap<uint64, uint32>& spreadlist)
{
	if (start >= end || !count)
		return;

	if (spreadlist.IsEmpty()){
		spreadlist.SetAt(0, 0);
		//spreadlist.SetAt(0, fileParent->GetFileSize());
	}

	POSITION endpos = spreadlist.FindFirstKeyAfter(end+1);

	if (endpos)
		spreadlist.GetPrev(endpos);
	else
		endpos = spreadlist.GetTailPosition();

	ASSERT(endpos != NULL);

	uint32 endcount = spreadlist.GetValueAt(endpos);
	endpos = spreadlist.SetAt(end, endcount);

	POSITION startpos = spreadlist.FindFirstKeyAfter(start+1);

	for (POSITION pos = startpos; pos != endpos; spreadlist.GetNext(pos)) {
		spreadlist.SetValueAt(pos, spreadlist.GetValueAt(pos)+count);
	}

	spreadlist.GetPrev(startpos);

	ASSERT(startpos != NULL);

	uint32 startcount = spreadlist.GetValueAt(startpos)+count;
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

void CStatisticFile::MergeSpreadList(CRBMap<uint64, uint32>& targetspreadlist,CRBMap<uint64, uint32>& sourcespreadlist)
{
	if (!sourcespreadlist.IsEmpty()) {
		POSITION pos = sourcespreadlist.GetHeadPosition();
		uint64 start = sourcespreadlist.GetKeyAt(pos);
		uint32 count = sourcespreadlist.GetValueAt(pos);
		sourcespreadlist.GetNext(pos);
		while (pos){
			uint64 end = sourcespreadlist.GetKeyAt(pos);
			if (count)
				AddBlockTransferred(start, end, count, targetspreadlist);
			start = end;
			count = sourcespreadlist.GetValueAt(pos);
			sourcespreadlist.GetNext(pos);
		}
		RecalcCompleteReleases();
	}
}

//COLORREF GetTrafficColor(float f)
//{
//	return RGB(0, (210-(22*(f-1)) <  0)? 0:210-(22*(f-1)), 255);
//}

CBarShader CStatisticFile::s_TrafficPartStatusBar(4);
CBarShader CStatisticFile::s_TrafficBlockStatusBar(16);
CBarShader CStatisticFile::s_SessionTrafficBlockStatusBar(4);

#define GrayIt(gray, color) gray?(0x444444+0x010101*((GetRValue(color)*30+GetGValue(color)*59+GetBValue(color)*11)/0xFF)):color

void CStatisticFile::DrawTrafficStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool  bFlat, COLORREF(*GetTrafficColor)(float), uint16 part) const
{	
	bool chunk = part != 0xffff;

	COLORREF crMissing = RGB(200, 200, 200);
	COLORREF crDot = RGB(255, 255, 255); // NEO: MOD - [ChunkDots]
	COLORREF color;

	bool bFill = (chunk ? (uint64)fileParent->GetPartSize(part) : fileParent->GetFileSize()) < PARTSIZE;
	uint64 fSize = (bFill) ? PARTSIZE : (chunk ? (uint64)fileParent->GetPartSize(part) : fileParent->GetFileSize());
	uint64 rfSize = chunk ? (uint64)fileParent->GetPartSize(part) : fileParent->GetFileSize();

	uint64 start = chunk ? part*PARTSIZE : 0;
	uint64 end = chunk ? part*PARTSIZE+fileParent->GetPartSize(part) : fSize;

	//--- draw alltime blocktraffic bar ---
	s_TrafficBlockStatusBar.SetFileSize(chunk ? PARTSIZE : fSize); 
	s_TrafficBlockStatusBar.SetHeight(rect->bottom - rect->top); 
	s_TrafficBlockStatusBar.SetWidth(rect->right - rect->left); 
	s_TrafficBlockStatusBar.Fill(crMissing); 
	POSITION pos = NULL;
	if(chunk){
		pos = alltimespreadlist.FindFirstKeyAfter(start+1);
		if (pos)
			alltimespreadlist.GetPrev(pos);
	}
	if(pos == NULL)
		pos = alltimespreadlist.GetHeadPosition();
	if (!onlygreyrect && pos)
	{ 
		uint32 count = alltimespreadlist.GetValueAt(pos);
		uint64 startpos = alltimespreadlist.GetKeyAt(pos);
		if(startpos < start) // get the part fiting in our part
			startpos = start;
		uint64 endpos = startpos;
		alltimespreadlist.GetNext(pos);
		while (pos && endpos != end){ // loop untill list is empty or we reached the end of the part
			endpos = alltimespreadlist.GetKeyAt(pos);
			if(endpos > end) // check for part end
				endpos = end;
			uint16 curPart = (uint16)(startpos/PARTSIZE);
			uint16 nextPart = (uint16)(endpos/PARTSIZE);
			while(curPart <= nextPart && count)
			{
				uint64 curendpos = min(curPart*PARTSIZE + fileParent->GetPartSize(curPart), endpos);
				color = GrayIt((fileParent->GetPartState(curPart) != PR_PART_ON), GetTrafficColor((float)count)); // NEO: MPS - [ManualPartSharing]
				s_TrafficBlockStatusBar.FillRange(startpos-start,curendpos-start,color);
				curPart++;
				startpos = curendpos;
			}

			count = alltimespreadlist.GetValueAt(pos);
			startpos = endpos;
			alltimespreadlist.GetNext(pos);
		}
	}
	if (bFill && !onlygreyrect) s_TrafficBlockStatusBar.FillRange(rfSize,chunk ? PARTSIZE : fSize,RGB(255,255,255));
   	s_TrafficBlockStatusBar.Draw(dc, rect->left, rect->top, bFlat); 

	//--- draw alltime parttraffic bar ---
	s_TrafficPartStatusBar.SetFileSize(chunk ? PARTSIZE : fSize);  
	s_TrafficPartStatusBar.SetHeight(/*rect->bottom - rect->top*/ 4); 
	s_TrafficPartStatusBar.SetWidth(rect->right - rect->left); 
	s_TrafficPartStatusBar.Fill(crMissing); 
	if (!onlygreyrect) { 
		for (uint16 i = (uint16)(chunk ? (start/PARTSIZE) : 0);i < (chunk ? ((start/PARTSIZE)+1) : fileParent->GetPartCount());i++){
			uint32 Traffic = const_cast <CStatisticFile*>(this)->GetPartTraffic(i);
			if(Traffic > 0 ){
				color = GetTrafficColor((float)Traffic/fileParent->GetPartSize(i));
				s_TrafficPartStatusBar.FillRange(PARTSIZE*(i)-start,PARTSIZE*(i)+fileParent->GetPartSize(i)-start,color);
			}
		}
	}
	if (bFill && !onlygreyrect) s_TrafficPartStatusBar.FillRange(rfSize,chunk ? PARTSIZE : fSize,RGB(255,255,255));
   	s_TrafficPartStatusBar.Draw(dc, rect->left, rect->top, bFlat); 

	//--- draw session blocktraffic bar ---
	s_TrafficBlockStatusBar.SetFileSize(chunk ? PARTSIZE : fSize); 
	s_SessionTrafficBlockStatusBar.SetHeight(/*rect->bottom - rect->top*/ 4); 
	s_SessionTrafficBlockStatusBar.SetWidth(rect->right - rect->left); 
	s_SessionTrafficBlockStatusBar.Fill(crMissing); 
	pos = NULL;
	if(chunk){
		pos = sessionspreadlist.FindFirstKeyAfter(start+1);
		if (pos)
			sessionspreadlist.GetPrev(pos);
	}
	if(pos == NULL)
		pos = sessionspreadlist.GetHeadPosition();
	if (!onlygreyrect && pos) 
	{ 
		uint32 count = sessionspreadlist.GetValueAt(pos);
		uint64 startpos = sessionspreadlist.GetKeyAt(pos);
		if(startpos < start) // get the part fiting in our part
			startpos = start;
		uint64 endpos = startpos;
		sessionspreadlist.GetNext(pos);
		while (pos && endpos != end){ // loop untill list is empty or we reached the end of the part
			endpos = sessionspreadlist.GetKeyAt(pos);
			if(endpos > end) // check for part end
				endpos = end;
			uint16 curPart = (uint16)(startpos/PARTSIZE);
			uint16 nextPart = (uint16)(endpos/PARTSIZE);
			while(curPart <= nextPart && count)
			{
				uint64 curendpos = min(curPart*PARTSIZE + fileParent->GetPartSize(curPart), endpos);
				color = GrayIt((fileParent->GetPartState(curPart) != PR_PART_ON), GetTrafficColor((float)count)); // NEO: MPS - [ManualPartSharing]
				s_TrafficBlockStatusBar.FillRange(startpos-start,curendpos-start,color);
				curPart++;
				startpos = curendpos;
			}

			count = sessionspreadlist.GetValueAt(pos);
			startpos = endpos;
			sessionspreadlist.GetNext(pos);
		}
	}
	if (bFill && !onlygreyrect) s_SessionTrafficBlockStatusBar.FillRange(rfSize,chunk ? PARTSIZE : fSize,RGB(255,255,255));
   	s_SessionTrafficBlockStatusBar.Draw(dc, rect->left, /*rect->top*/ rect->bottom - 4, bFlat); 

	// NEO: MOD - [ChunkDots]
	if(NeoPrefs.UseChunkDots() && fSize > PARTSIZE)
	{
		uint32	w=rect->right-rect->left+1;
		RECT gaprect;
		gaprect.top = rect->top;
		gaprect.bottom = gaprect.top + 4;
		gaprect.left = rect->left;
		if(!bFlat) {
				s_TrafficPartStatusBar.SetWidth(1);
				s_TrafficPartStatusBar.SetFileSize((uint64)1);
				s_TrafficPartStatusBar.Fill(crDot);
				for(ULONGLONG i=0; i<rfSize; i+=PARTSIZE)
					s_TrafficPartStatusBar.Draw(dc, gaprect.left+(int)((double)i*w/rfSize), gaprect.top, false);
		} else {
			for(ULONGLONG i = 0; i<fSize; i+=PARTSIZE){
				gaprect.left = gaprect.right = (LONG)(rect->left+(uint64)((float)i*w/rfSize));
				gaprect.right++;
				dc->FillRect(&gaprect, &CBrush(RGB(128,128,128)));
			}
		}
	}
	// NEO: END
} 

float CStatisticFile::CalcReleases(uint64 start, uint64 end, CRBMap<uint64, uint32>& spreadlist) const
{
	if (spreadlist.IsEmpty() || end <= start)
		return 0;

	// get the position of the first interesting entry
	POSITION beginpos = spreadlist.FindFirstKeyAfter(start+1);
	if (beginpos)
		spreadlist.GetPrev(beginpos);
	else
		beginpos = spreadlist.GetHeadPosition();

	// find lowest parttraffic
	POSITION pos = beginpos;
	uint32 minimal = spreadlist.GetValueAt(pos);
	spreadlist.GetNext(pos);
	while (pos && spreadlist.GetKeyAt(pos) < end){
		uint32 count = spreadlist.GetValueAt(pos);
		if (count < minimal)
			minimal = count;
		spreadlist.GetNext(pos);
	}

	// get avg
	float releases = 0;

	pos = beginpos;
	uint32 count = spreadlist.GetValueAt(pos);
	uint64 startpos = spreadlist.GetKeyAt(pos);
	if(startpos < start) // get the part fiting in our part
		startpos = start;
	uint64 endpos = startpos;
	spreadlist.GetNext(pos);
	while (pos && endpos != end){
		endpos = spreadlist.GetKeyAt(pos);
		if(endpos <= start) // there are no dtas for this and following parts
			break;
		if(endpos > end) // check for part end
			endpos = end;
		if (count > minimal+1)
			releases += (endpos-startpos)*(minimal+1);
		else
			releases += (endpos-startpos)*count;	
		startpos = endpos;
		count = spreadlist.GetValueAt(pos);
		spreadlist.GetNext(pos);
	}
	releases /= (end-start);
	return releases;
}

float CStatisticFile::GetPartRelease(uint16 part)
{
	return CalcReleases(part*PARTSIZE,part*PARTSIZE+fileParent->GetPartSize(part),alltimespreadlist);
}

void CStatisticFile::RecalcCompleteReleases()
{
	completereleases = CalcReleases(0,fileParent->GetFileSize(),alltimespreadlist);
}

uint32 CStatisticFile::CalcTraffic(uint64 start, uint64 end, CRBMap<uint64, uint32>& spreadlist) const
{
	if (spreadlist.IsEmpty() || end <= start)
		return 0;

	uint32 traffic = 0;

	// get the position of the first interesting entry
	POSITION pos = spreadlist.FindFirstKeyAfter(start+1);
	if (pos)
		spreadlist.GetPrev(pos);
	else
		pos = spreadlist.GetHeadPosition();

	// calculate te traffic
	uint32 count = spreadlist.GetValueAt(pos);
	uint64 startpos = spreadlist.GetKeyAt(pos);
	if(startpos < start) // get the part fiting in our part
		startpos = start;
	uint64 endpos = startpos;
	spreadlist.GetNext(pos);
	while (pos && endpos != end){ // loop untill list is empty or we reached the end of the part
		endpos = spreadlist.GetKeyAt(pos);
		if(endpos <= start) // there are no dtas for this and following parts
			break;
		if(endpos > end) // check for part end
			endpos = end;
		traffic += (uint32)((endpos-startpos)*count); // X! we calc only for one part obove 4 gb traffic for a simgle part is highly unlikly
		count = spreadlist.GetValueAt(pos);
		startpos = endpos;
		spreadlist.GetNext(pos);
	}
	return traffic;
}

uint32 CStatisticFile::GetPartTraffic(uint16 part)
{
	return CalcTraffic(part*PARTSIZE,part*PARTSIZE+fileParent->GetPartSize(part),alltimespreadlist);
}

uint32 CStatisticFile::GetPartTrafficSession(uint16 part)
{
	return CalcTraffic(part*PARTSIZE,part*PARTSIZE+fileParent->GetPartSize(part),sessionspreadlist);
}

void CStatisticFile::ResetStats(bool all)
{
	requested = 0;
	transferred = 0;
	accepted = 0;

	sessionspreadlist.RemoveAll();

	if(all){
		alltimerequested = 0;
		alltimetransferred = 0;
		alltimeaccepted = 0;

		alltimespreadlist.RemoveAll();
		completereleases = 0.0f;
	}
}

bool CKnownFileList::LoadPartTraffic()
{
	m_bPartTrafficLoaded = true;
	//--- load collected traffic data in our file ---
	CString fullpath=thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath.Append(TRAFFIC_MET_FILENAME);
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath,CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(_T("Failed to load ") TRAFFIC_MET_FILENAME _T(" file"));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			ModLogError(LOG_STATUSBAR, _T("%s"), strError);
			return false;
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try {
		uint8 version = file.ReadUInt8();
		if (version > TRAFFICFILE_VERSION || version <= TRAFFICFILE_VERSION_OLD){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_TRAFFICMET_UNKNOWN_VERSION));
			file.Close();
			return false;
		}

		UINT RecordsNumber = file.ReadUInt32();
		uchar cur_hash[16];
		CKnownFile* cuf_file=NULL;
		for (UINT i = 0; i < RecordsNumber; i++) {
			file.ReadHash16(cur_hash);
			if((cuf_file = FindKnownFileByID(cur_hash)) != NULL){
				if(!cuf_file->statistic.LoadTraffic(&file)){
					ModLogError(GetResString(IDS_X_ERR_TRAFFICMET_ENTRY_CORRUPT), cuf_file->GetFileName());
				}
			}else{
				ModLogError(GetResString(IDS_X_ERR_TRAFFICASYNCHRONIZED), md4str(cur_hash));
				ClearTrafficEntry(&file);
			}
		}
		file.Close();
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile){
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_TRAFFICMET_BAD));
		}else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer,ARRSIZE(buffer));
			ModLogError(LOG_STATUSBAR, GetResString(IDS_X_ERR_TRAFFICMET_UNKNOWN),buffer);
		}
		error->Delete();
		return false;
	}

	return true;
}

bool CKnownFileList::ClearTrafficEntry(CFileDataIO* file)
{
	UINT uEntryCount = file->ReadUInt32();
	uint8 isLarge = file->ReadUInt8();

	ULONG total = uEntryCount*(4+(isLarge ? (8+8) : (4+4)));
	file->Seek(total,CFile::current);

	return true;
}

bool CKnownFileList::SavePartTraffic()
{
	//--- save traffic data to our file ---
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false,_T("Saving Traffic files list file \"%s\""), TRAFFIC_MET_FILENAME);
	CString fullpath=thePrefs.GetMuleDirectory(EMULE_CONFIGDIR);
	fullpath += TRAFFIC_MET_FILENAME;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(fullpath, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(_T("Failed to save ") TRAFFIC_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		file.WriteUInt8(TRAFFICFILE_VERSION);

		uint32 uTagCount = 0;
		ULONG uTagCountFilePos = (ULONG)file.GetPosition();
		file.WriteUInt32(uTagCount);

		POSITION pos = m_Files_map.GetStartPosition();
		while( pos != NULL )
		{
			CKnownFile* pFile;
			CCKey key;
			m_Files_map.GetNextAssoc( pos, key, pFile );
			if(pFile->statistic.HasTraffic()){			// only save for known-files with traffic
				file.WriteHash16(pFile->GetFileHash());
				pFile->statistic.SaveTraffic(&file); 
				uTagCount++;
			}
		}

		file.Seek(uTagCountFilePos, CFile::begin);
		file.WriteUInt32(uTagCount);
		file.Seek(0, CFile::end);

		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning())){
			file.Flush(); // flush file stream buffers to disk buffers
			if (_commit(_fileno(file.m_pStream)) != 0) // commit disk buffers to disk
				AfxThrowFileException(CFileException::hardIO, GetLastError(), file.GetFileName());
		}
		file.Close();
	}
	catch(CFileException* error){
		CString strError(_T("Failed to save ") TRAFFIC_MET_FILENAME _T(" file"));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		ModLogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}

	return true;
}

void CStatisticFile::SaveTraffic(CFileDataIO* file)
{
	UINT uEntryCount = 0;
	ULONG uEntryCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uEntryCount);
	bool isLarge = fileParent->IsLargeFile();
	file->WriteUInt8(isLarge);

	for (POSITION pos = alltimespreadlist.GetHeadPosition(); pos; ){
		uint32 count = alltimespreadlist.GetValueAt(pos);
		if (!count) {
			alltimespreadlist.GetNext(pos);
			continue;
		}
		uint64 start = alltimespreadlist.GetKeyAt(pos);
		alltimespreadlist.GetNext(pos);
		ASSERT(pos != NULL);	// Last value should always be 0
		uint64 end = alltimespreadlist.GetKeyAt(pos);

		if(isLarge){
			file->WriteUInt64(start);
			file->WriteUInt64(end);
		}else{
			file->WriteUInt32((uint32)start);
			file->WriteUInt32((uint32)end);
		}
		file->WriteUInt32(count);

		uEntryCount++;
	}

	file->Seek(uEntryCountFilePos, CFile::begin);
	file->WriteUInt32(uEntryCount);
	file->Seek(0, CFile::end);
}

bool CStatisticFile::LoadTraffic(CFileDataIO* file)
{
	UINT uEntryCount = file->ReadUInt32();
	uint8 isLarge = file->ReadUInt8();
	
	for (UINT j = 0; j < uEntryCount; j++){
		uint64 start = isLarge ? file->ReadUInt64() : file->ReadUInt32();
		uint64 end = isLarge ? file->ReadUInt64() : file->ReadUInt32();
		uint32 count = file->ReadUInt32();

		AddBlockTransferred(start, end, count, alltimespreadlist);
	}

	RecalcCompleteReleases();

	return true;
}

// NEO: NPT END <-- Xanatos --
