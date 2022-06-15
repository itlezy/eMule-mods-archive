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

#pragma once

// NEO: SCFS - [SmartClientFileStatus] -- Xanatos -->
#include "BlockMaps.h" // NEO: SCT - [SubChunkTransfer]

class CKnownFile;
class CUpDownClient;
class CSafeMemFile;
class CFileDataIO;
class CTag;

enum EPartStatus
{
	CFS_Normal = 0,
	CFS_Incomplete = 1, // NEO: ICS - [InteligentChunkSelection]
	CFS_History = 2, // NEO: PSH - [PartStatusHistory]
	CFS_COUNT = 3
};

/////////////////////////////////////////////////////////////
// CClientFileStatus
//

class CClientFileStatus : public CObject
{
public:

	CClientFileStatus(const CKnownFile* file);
	~CClientFileStatus();

	const uchar*			GetFileHash() const { return m_abyFileHash; }

	bool 					ReadFileStatus(CSafeMemFile* data, EPartStatus type = CFS_Normal, bool throwError = true);
	bool 					WriteFileStatus(CSafeMemFile* data, EPartStatus type = CFS_Normal);

	void					FillDefault();

	// NEO: SCT - [SubChunkTransfer]
	bool					ReadSubChunkMaps(CSafeMemFile* data);
	bool					WriteSubChunkMaps(CSafeMemFile* data);
	// NEO: SCT END

	uint8*					GetPartStatus(EPartStatus type = CFS_Normal) const {
								return m_abyPartStatus[type];
							}
	// NEO: PSH - [PartStatusHistory]
	uint8*					GetPartStatusHistory() const {
								if(m_abyPartStatus[CFS_History])
									return m_abyPartStatus[CFS_History];
								return m_abyPartStatus[CFS_Normal];
							}
	// NEO: PSH END
	bool					IsCompleteSource() const			{ return m_bCompleteSource; }
	bool					IsPartAvailable(UINT part) const;
	bool					IsIncPartAvailable(UINT part) const; // NEO: ICS - [InteligentChunkSelection]
	bool					GetBlockMap(UINT part, tBlockMap** map); // NEO: SCT - [SubChunkTransfer]

	UINT					GetPartCount() const				{ return m_nPartCount; }
	EMFileSize				GetFileSize() const					{ return m_nFileSize; }

	const CString&			GetFileName() const					{ return m_strFileName; }
	void					SetFileName(LPCTSTR pszFileName)	{m_strFileName = pszFileName;}

	//File Comment
	bool					HasFileComment() const				{ return !m_strFileComment.IsEmpty(); }
    const CString&			GetFileComment() const				{ return m_strFileComment; } 
    void					SetFileComment(LPCTSTR pszComment)	{ m_strFileComment = pszComment; }

	bool					HasFileRating() const				{ return m_uFileRating > 0; }
    uint8					GetFileRating() const				{ return m_uFileRating; }
    void					SetFileRating(uint8 uRating)		{ m_uFileRating = uRating; }

	uint16					GetCompleteSourcesCount() const		{ return m_nCompleteSourcesCount; }
	void					SetCompleteSourcesCount(uint16 n)	{ m_nCompleteSourcesCount = n; }

	UINT					m_uSCTpos; // NEO: SCT - [SubChunkTransfer]

#ifdef NEO_CD// NEO: SFL - [SourceFileList]
	void					SetArcived(bool bSet = true)		{m_bArchived = bSet;}
	bool					IsArcived() const					{return m_bArchived;}
	void					SetUsed(bool bSet = true)			{m_bUsed = bSet;}
	bool					IsUsed() const						{return m_bUsed;}
#endif // NEO_CD // NEO: SFL END

protected:
	friend class CUpDownClient;
#ifdef NEO_CD// NEO: SFL - [SourceFileList]
	friend class CKnownSource;

	CClientFileStatus(const uchar* abyFileHash, const EMFileSize nFileSize);

	bool					WriteFileStatusTag(EPartStatus type, CFileDataIO* file);
	bool					ReadFileStatusTag(CTag* tag);
#endif // NEO_CD // NEO: SFL END

private:
	void					Init();

	void					UpdateStatusHistory(); // NEO: PSH - [PartStatusHistory]

	// Main file informations
	uchar					m_abyFileHash[16];
	UINT					m_nPartCount;
	UINT					m_nED2KPartCount;
	EMFileSize				m_nFileSize;

	// fille status
	uint8*					m_abyPartStatus[CFS_COUNT];
	bool					m_bCompleteSource;

	CBlockMaps				m_BlockMaps; // NEO: SCT - [SubChunkTransfer]

	// additional informations
	CString					m_strFileName;
	CString					m_strFileComment;
	uint8					m_uFileRating;

	uint16					m_nCompleteSourcesCount;

#ifdef NEO_CD// NEO: SFL - [SourceFileList]
	bool					m_bArchived;
	bool					m_bUsed;
#endif // NEO_CD // NEO: SFL END
};

// NEO: SCFS END <-- Xanatos --