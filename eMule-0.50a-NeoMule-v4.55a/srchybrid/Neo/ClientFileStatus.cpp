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

#include "StdAfx.h"
#include "emule.h"
#include "ClientFileStatus.h"
#include "KnownFile.h"
#include "SafeFile.h"
#include "Preferences.h"
#include "NeoPreferences.h"
#include "otherfunctions.h"
#include "log.h"
#include "packets.h"
#include "Neo/NeoOpCodes.h"

// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->

/////////////////////////////////////////////////////////////
// CClientFileStatus
//

CClientFileStatus::CClientFileStatus(const CKnownFile* file)
{
	md4cpy(m_abyFileHash, file->GetFileHash());
	m_nPartCount = file->GetPartCount();
	m_nED2KPartCount = file->GetED2KPartCount();
	m_nFileSize = file->GetFileSize();

	Init();

#ifdef NEO_CD// NEO: SFL - [SourceFileList]
	m_bArchived = false;
	m_bUsed = true;
#endif // NEO_CD // NEO: SFL END
}

void CClientFileStatus::Init()
{
	for(uint16 i = 0; i<CFS_COUNT; i++)
		m_abyPartStatus[i] = NULL;
	m_bCompleteSource = false;

	m_strFileName.Empty();
	m_strFileComment.Empty();
	m_uFileRating = 0;

	m_nCompleteSourcesCount = 0;

	m_uSCTpos = 0; // NEO: SCT - [SubChunkTransfer]
}

#ifdef NEO_CD// NEO: SFL - [SourceFileList]
CClientFileStatus::CClientFileStatus(const uchar* abyFileHash, const EMFileSize nFileSize)
{
	md4cpy(m_abyFileHash, abyFileHash);
	m_nPartCount = (uint16)(((uint64)nFileSize + (PARTSIZE - 1)) / PARTSIZE);
	m_nED2KPartCount = (uint16)((uint64)nFileSize / PARTSIZE + 1);
	m_nFileSize = nFileSize;

	Init();

	m_bArchived = true;
	m_bUsed = false;
}
#endif // NEO_CD // NEO: SFL END

CClientFileStatus::~CClientFileStatus()
{
	for(uint16 i = 0; i<CFS_COUNT; i++)
		delete[] m_abyPartStatus[i];
	m_BlockMaps.RemoveAll(); // NEO: SCT - [SubChunkTransfer]
}

bool CClientFileStatus::ReadFileStatus(CSafeMemFile* data, EPartStatus type, bool throwError)
{
	UINT nED2KPartCount = data->ReadUInt16();

	if (m_abyPartStatus[type] == NULL) // allocate file status
		m_abyPartStatus[type] = new uint8[m_nPartCount];
	
	if (!nED2KPartCount) // no status
	{
		if(type == CFS_Normal){
			m_bCompleteSource = true;
			// NEO: SCT - [SubChunkTransfer]
			if(NeoPrefs.UseSubChunkTransfer())
				m_BlockMaps.RemoveAll();
			// NEO: SCT END
		}
		memset(m_abyPartStatus[type],(type == CFS_Normal),m_nPartCount); // set default status state
	}
	else
	{
		if(type == CFS_Normal)
			m_bCompleteSource = false;
		if (m_nED2KPartCount != nED2KPartCount) {
			if (thePrefs.GetVerbose()) {
				DebugLogWarning(_T("FileName: \"%s\""), GetFileName());
				DebugLogWarning(_T("FileStatus: %s"), DbgGetFileStatus(nED2KPartCount, data));
			}
			CString strError;
			strError.Format(_T("ReadFileStatus %u - wrong part number recv=%u  expected=%u  %s"), (UINT)type, nED2KPartCount, m_nED2KPartCount, DbgGetFileInfo(GetFileHash()));
			if(throwError)
				throw strError;
			return false;
		}
		
		const bool bCleanUpSCT = (type != CFS_Incomplete && NeoPrefs.UseSubChunkTransfer()); // NEO: SCT - [SubChunkTransfer]
		// read the status
		uint16 done = 0;
		while (done != m_nPartCount){
			uint8 toread = data->ReadUInt8();
			for (sint32 i = 0;i != 8;i++){
				if((m_abyPartStatus[type][done] = ((toread>>i) & 1) ? 1 : 0) == TRUE){
					// NEO: SCT - [SubChunkTransfer]
					if(bCleanUpSCT)
						m_BlockMaps.RemoveKey(done);
					// NEO: SCT END
				}
				done++;
				if (done == m_nPartCount)
					break;
			}
		}
	}

	// NEO: PSH - [PartStatusHistory]
	if(NeoPrefs.UsePartStatusHistory() && type == CFS_Normal)
		UpdateStatusHistory();
	// NEO: PSH END

	return true;
}

bool CClientFileStatus::WriteFileStatus(CSafeMemFile* data, EPartStatus type)
{
	if(m_abyPartStatus[type] == NULL)
		return false;

	if(m_bCompleteSource && type == CFS_Normal)
	{
		data->WriteUInt16(0);
	}
	else
	{
		data->WriteUInt16((uint16)m_nED2KPartCount);

		UINT done = 0;
		while (done != m_nED2KPartCount){
			uint8 towrite = 0;
			for (UINT i = 0; i < 8; i++){
				if (m_abyPartStatus[type][done])
					towrite |= (1<<i);
				done++;
				if (done == m_nED2KPartCount)
					break;
			}
			data->WriteUInt8(towrite);
		}
	}
	
	return true;
}

// NEO: PSH - [PartStatusHistory]
void CClientFileStatus::UpdateStatusHistory()
{
	if (m_abyPartStatus[CFS_History] == NULL){ // allocate file status
		m_abyPartStatus[CFS_History] = new uint8[m_nPartCount];
		memset(m_abyPartStatus[CFS_History],0,m_nPartCount);
	}

	for (UINT i = 0; i < m_nPartCount; i++)
		if (m_abyPartStatus[CFS_Normal][i])
			m_abyPartStatus[CFS_History][i] = 1;
}
// NEO: PSH END

void CClientFileStatus::FillDefault()
{
	if (m_abyPartStatus[CFS_Normal] == NULL) // allocate file status
		m_abyPartStatus[CFS_Normal] = new uint8[m_nPartCount];
	memset(m_abyPartStatus[CFS_Normal],1,m_nPartCount); // set default status state
}

// NEO: SCT - [SubChunkTransfer]
bool CClientFileStatus::ReadSubChunkMaps(CSafeMemFile* data)
{
	uint8 uOpts = data->ReadUInt8()*8;
	//				= (uOpts >> 3) & 0x1F;
	uint8 uDiv		= (uOpts >> 0) & 0x07;
	if(uDiv == 0) // just in case
		uDiv = 1; // maby return false would be beter

	uint16 count = data->ReadUInt8();
	if(count == 0xFF)
		count = data->ReadUInt16();

	UINT BitMapLen = (53/(8*uDiv)) + ((53%(8*uDiv)) ? 1 : 0);
	uint8 BitMap[7];

	uint16 part;
	tBlockMap blockmap;
	for (uint16 i = 0; i < count; i++)
	{
		part = data->ReadUInt16();
		if(uDiv == 1)
			data->Read(&blockmap.map,7);
		else
		{
			data->Read(&BitMap[0],BitMapLen);
			blockmap.Read(BitMap,uDiv);
		}
		m_BlockMaps.SetAt(part,blockmap); // copy the map into our global map
	}
	return true;
}

bool CClientFileStatus::WriteSubChunkMaps(CSafeMemFile* data)
{
	uint8 uOpts =   //((			& 0x1F) << 3) | // reserved
					((1			& 0x07) << 0); // div
	data->WriteUInt8(uOpts);

	uint16 uMapCount = (uint16)m_BlockMaps.GetCount();
	if(uMapCount >= 0xFF) {
		data->WriteUInt8(0xFF); // indicates 16 bit counter
		data->WriteUInt16(uMapCount);
	}
	data->WriteUInt8((uint8)uMapCount);
	
	tBlockMap* blockMap;
	uint16 Part;
	for (CBlockMaps::CPair *pCurVal = m_BlockMaps.PGetFirstAssoc(); pCurVal != NULL; pCurVal = m_BlockMaps.PGetNextAssoc(pCurVal)){
		blockMap = &pCurVal->value;
		Part = pCurVal->key;
		
		data->WriteUInt16(Part);
		data->Write(&blockMap->map,7);
	}

	return uMapCount != 0;
}

bool CClientFileStatus::GetBlockMap(UINT part, tBlockMap** map)
{
	*map = NULL; // it is important to reset the map pointer, otherwice GetNextEmptyBlockInPart will fail
	CBlockMaps::CPair *pCurVal;
	pCurVal = m_BlockMaps.PLookup((uint16)part);
	if(pCurVal)
		*map = &pCurVal->value; // return reference to object storred in our global map
	return *map != NULL;
}
// NEO: SCT END

bool CClientFileStatus::IsPartAvailable(UINT part) const
{
	uint8* abyPartStatus = m_abyPartStatus[CFS_Normal];
	return (abyPartStatus && abyPartStatus[part]);
}

// NEO: ICS - [InteligentChunkSelection]
bool CClientFileStatus::IsIncPartAvailable(UINT part) const
{
	uint8* abyPartStatus = m_abyPartStatus[CFS_Incomplete];
	return (abyPartStatus && abyPartStatus[part]);
}
// NEO: ICS END

#ifdef NEO_CD // NEO: SFL - [SourceFileList]
bool CClientFileStatus::WriteFileStatusTag(EPartStatus type, CFileDataIO* file)
{
	CSafeMemFile data(16+16);
	if(!WriteFileStatus(&data,type))
		return false;

	uint8 name;
	switch(type){
	case CFS_Normal:		name = FFT_PART_STATUS; break;
	case CFS_Incomplete:	name = FFT_INC_PART_STATUS; break; // NEO: ICS - [InteligentChunkSelection]
	case CFS_History:		name = FFT_SEEN_PART_STATUS; break; // NEO: PSH - [PartStatusHistory]
	default: return false;
	}

	uint32 size = (UINT)data.GetLength();
	BYTE* tmp = data.Detach();
	CTag tag(name,size,tmp);
	free(tmp);
	tag.WriteNewEd2kTag(file);
	return true;
}

bool CClientFileStatus::ReadFileStatusTag(CTag* tag)
{
	if(!tag->IsBlob())
		return false;

	EPartStatus type;
	switch(tag->GetNameID()){
	case FFT_PART_STATUS:			type = CFS_Normal; break;
	case FFT_INC_PART_STATUS:		type = CFS_Incomplete; break; // NEO: ICS - [InteligentChunkSelection]
	case FFT_SEEN_PART_STATUS:		type = CFS_History; break; // NEO: PSH - [PartStatusHistory]
	default: return false;
	}

	CSafeMemFile data(tag->GetBlob(),tag->GetBlobSize());
	return ReadFileStatus(&data,type,false);
}

//void CClientFileStatus::Update(CClientFileStatus* status)
//{
//	if(status->m_nPartCount != m_nPartCount){
//		ASSERT(0);
//		return;
//	}
//
//	for(uint16 i = 0; i<CFS_COUNT; i++){
//		if(status->m_abyPartStatus[i]){
//			if(m_abyPartStatus[i] == NULL)
//				m_abyPartStatus[i] = new uint8[m_nPartCount];
//			memcpy(m_abyPartStatus[i],status->m_abyPartStatus[i],m_nPartCount);
//		}
//	}
//
//	m_bCompleteSource = status->m_bCompleteSource;
//
//	m_strFileName = status->m_strFileName;
//	m_strFileComment = status->m_strFileComment;
//	m_uFileRating = status->m_uFileRating;
//
//	m_nCompleteSourcesCount = status->m_nCompleteSourcesCount;
//
//	m_uSCTpos = status->m_uSCTpos; // NEO: SCT - [SubChunkTransfer]
//}
#endif // NEO_CD // NEO: SFL END

// NEO: SCFS END <-- Xanatos --