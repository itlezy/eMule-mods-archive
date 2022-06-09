//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

#include "Loggable.h"

class CPartFile;
struct Gap_Struct;

#define ZIP_LOCAL_HEADER_MAGIC		0x04034b50
#define ZIP_LOCAL_HEADER_EXT_MAGIC	0x08074b50
#define ZIP_CD_MAGIC				0x02014b50
#define ZIP_END_CD_MAGIC			0x06054b50
#define ZIP_COMMENT					"Recovered by eMule Plus"

#define RAR_HEAD_FILE 0x74

#pragma pack(1)
struct ZIP_Entry
{
	uint32	header;
	uint16	versionToExtract;
	uint16	generalPurposeFlag;
	uint16	compressionMethod;
	uint16	lastModFileTime;
	uint16	lastModFileDate;
	uint32	crc32;
	uint32	lenCompressed;
	uint32	lenUncompressed;
	uint16	lenFilename;
	uint16	lenExtraField;
	BYTE	*filename;
	BYTE	*extraField;
	BYTE	*compressedData;
};
#pragma pack()

#pragma pack(1)
struct ZIP_CentralDirectory
{
	uint32	header;
	uint16	versionMadeBy;
	uint16	versionToExtract;
	uint16	generalPurposeFlag;
	uint16	compressionMethod;
	uint16	lastModFileTime;
	uint16	lastModFileDate;
	uint32	crc32;
	uint32	lenCompressed;
	uint32	lenUncompressed;
	uint16	lenFilename;
	uint16	lenExtraField;
	uint16	lenComment;
	uint16	diskNumberStart;
	uint16	internalFileAttributes;
	uint32	externalFileAttributes;
	uint32	relativeOffsetOfLocalHeader;
	BYTE	*filename;
	BYTE	*extraField;
	BYTE	*comment;
};
#pragma pack()

#pragma pack(1)
struct RAR_BlockFile
{
	RAR_BlockFile()
	{
		EXT_DATE = NULL;
		EXT_DATE_SIZE = 0;
	}
	~RAR_BlockFile()
	{
		delete[] EXT_DATE;
	}

	// This indicates the position in the input file just after the filename
	ULONGLONG offsetData; 
	// This indicates how much of the block is after this offset
	uint32 dataLength;

	uint16	HEAD_CRC;
	BYTE	HEAD_TYPE;
	uint16	HEAD_FLAGS;
	uint16	HEAD_SIZE;
	uint32	PACK_SIZE;
	uint32	UNP_SIZE;
	BYTE	HOST_OS;
	uint32	FILE_CRC;
	uint32	FTIME;
	BYTE	UNP_VER;
	BYTE	METHOD;
	uint16	NAME_SIZE;
	uint32	ATTR;
	uint32	HIGH_PACK_SIZE;
	uint32	HIGH_UNP_SIZE;
	BYTE	*FILE_NAME;
	BYTE	*EXT_DATE;
	uint32	EXT_DATE_SIZE;
	BYTE	SALT[8];
};
#pragma pack()

struct ThreadParam
{
	CPartFile *partFile;
	CTypedPtrList<CPtrList, Gap_Struct*> *filled;
};

class CArchiveRecovery : public CLoggable
{
public:
	static void recover(CPartFile *partFile);

private:
	CArchiveRecovery(void); // Just use static recover method

	static UINT AFX_CDECL run(LPVOID lpParam);
	static bool performRecovery(CPartFile *partFile, CTypedPtrList<CPtrList, Gap_Struct*> *filled);

	static int recoverZip(CFile *zipInput, CFile *zipOutput, CTypedPtrList<CPtrList, Gap_Struct*> *filled, bool fullSize);
	static int recoverRar(CFile *rarInput, CFile *rarOutput, CTypedPtrList<CPtrList, Gap_Struct*> *filled);

	static bool scanForZipMarker(CFile *input, uint32 marker, uint32 available);
	static bool processZipEntry(CFile *zipInput, CFile *zipOutput, uint32 available, CTypedPtrList<CPtrList, ZIP_CentralDirectory*> *centralDirectoryEntries);
	static bool readZipCentralDirectory(CFile *zipInput, CTypedPtrList<CPtrList, ZIP_CentralDirectory*> *centralDirectoryEntries, CTypedPtrList<CPtrList, Gap_Struct*> *filled);

	static RAR_BlockFile *scanForRarFileHeader(CFile *input, uint64 qwAvailable);
	static bool validateRarFileBlock(RAR_BlockFile *block);
	static void writeRarBlock(CFile *input, CFile *output, RAR_BlockFile *block);

	static bool CopyFile(CPartFile *partFile, CTypedPtrList<CPtrList, Gap_Struct*> *filled, CString tempFileName);
	static void DeleteMemory(ThreadParam *tp);
	static bool IsFilled(uint64 start, uint64 end, CTypedPtrList<CPtrList, Gap_Struct*> *filled);

	static uint16 readUInt16(CFile *input);
	static uint32 readUInt32(CFile *input);
	static uint16 calcUInt16(BYTE *input);
	static uint32 calcUInt32(BYTE *input);
	static void writeUInt16(CFile *output, uint16 val);
	static void writeUInt32(CFile *output, uint32 val);
};
