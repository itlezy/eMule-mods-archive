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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#ifndef NEW_SOCKETS_ENGINE
#include "emule.h"
#endif //NEW_SOCKETS_ENGINE
#include "PartFile.h"
#include "ArchiveRecovery.h"
#include "otherfunctions.h"
#include "zlib/zlib.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


#pragma pack(push,1)
typedef struct {
	BYTE	type;
	WORD	flags;
	WORD	size;
} RARMAINHDR;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
	BYTE	type;
	WORD	flags;
	WORD	size;
	DWORD	packetSize;
	DWORD	unpacketSize;
	BYTE	hostOS;
	DWORD	fileCRC;
	DWORD	fileTime;
	BYTE	unpVer;
	BYTE	method;
	WORD	nameSize;
	DWORD	fileAttr;
} RARFILEHDR;
#pragma pack(pop)

// At some point it may be worth displaying messages to alert the user if there were errors, or where to find the file.

void CArchiveRecovery::recover(CPartFile *partFile)
{
	if (partFile->m_bPreviewing || partFile->m_bRecoveringArchive)
		return;
	partFile->m_bRecoveringArchive = true;

	AddLogLine(LOG_FL_SBAR, IDS_ATTEMPTING_RECOVERY);

// Get the current filled list for this file
	CTypedPtrList<CPtrList, Gap_Struct*> *filled = new CTypedPtrList<CPtrList, Gap_Struct*>;
	partFile->GetFilledList(filled);

	// The rest of the work can be safely done in a new thread
	ThreadParam *tp = new ThreadParam;
	tp->partFile = partFile;
	tp->filled = filled;

//	Do NOT use Windows API 'CreateThread' to create a thread which uses MFC/CRT -> lots of mem leaks!
	if (!AfxBeginThread(run, (LPVOID)tp))
	{
		partFile->m_bRecoveringArchive = false;
		AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_RECOVERY_FAILED);
	//	Need to delete the memory here as it won't be done in thread
		DeleteMemory(tp);
	}
}

UINT AFX_CDECL CArchiveRecovery::run(LPVOID lpParam)
{
	g_App.m_pPrefs->InitThreadLocale();
	ThreadParam *tp = (ThreadParam *)lpParam;

//	If app is closed while preview running, main objects (e.g. Prefs) can be released when
//	we reach this place (happens seldom), so avoid crash dump generation because of that
	try
	{
		if (!performRecovery(tp->partFile, tp->filled))
			AddLogLine(LOG_FL_SBAR | LOG_RGB_WARNING, IDS_RECOVERY_FAILED);

		tp->partFile->m_bRecoveringArchive = false;
	}
	catch(...)
	{
	}

	// Delete memory used by copied gap list
	DeleteMemory(tp);

	return 0;
}

bool CArchiveRecovery::performRecovery(CPartFile *partFile, CTypedPtrList<CPtrList, Gap_Struct*> *filled)
{
	int	iRC = -1;

	try
	{
		// Copy the file
		CString tempFileName = partFile->GetTempDir() + _T("\\") + partFile->GetFileName().Mid(0, 5) + _T("-rec.tmp");
		if (!CopyFile(partFile, filled, tempFileName))
			return false;

		// Open temp file for reading
		CFile temp;
		if (!temp.Open(tempFileName, CFile::modeRead | CFile::shareDenyWrite | CFile::osSequentialScan | CFile::typeBinary))
			return false;

		// Open output file
		CString	ext = partFile->GetFileExtension();
		CString	outputFileName = partFile->GetTempDir() + _T("\\") + partFile->GetFileName().Mid(0, 5) + _T("-rec.") + ext;
		CFile	output;
		BOOL	bOutCreated = output.Open(outputFileName, CFile::modeWrite | CFile::shareDenyWrite | CFile::modeCreate | CFile::typeBinary);

		if (bOutCreated)
		{
			// Process the output file
			if ((ext.CompareNoCase(_T("zip")) == 0) || (ext.CompareNoCase(_T("cbz")) == 0))
				iRC = recoverZip(&temp, &output, filled, IsFilled(partFile->GetFileSize() - 22ui64, partFile->GetFileSize(), filled));
			else if ((ext.CompareNoCase(_T("rar")) == 0) || (ext.CompareNoCase(_T("cbr")) == 0))
				iRC = recoverRar(&temp, &output, filled);

			output.Close();
		}
		temp.Close();

		::DeleteFile(tempFileName);

	//	Run an application even if nothing was recovered to notify user about completion
		if (iRC >= 0)
		{
			if (iRC > 0)	// something useful was recovered
				AddLogLine(LOG_FL_SBAR, IDS_RECOVERY_SUCCESSFUL);

			SHELLEXECUTEINFO SE;
			memzero(&SE, sizeof(SE));
			SE.fMask = SEE_MASK_NOCLOSEPROCESS;
			SE.lpVerb = _T("open");
			SE.lpFile = outputFileName;
			SE.nShow = SW_SHOW;
			SE.cbSize = sizeof(SE);
			ShellExecuteEx(&SE);
			if (SE.hProcess)
			{
				WaitForSingleObject(SE.hProcess, INFINITE);
				CloseHandle(SE.hProcess);
			}
		}
		if (bOutCreated)
			::DeleteFile(outputFileName);
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...){
	}

	return (iRC > 0);	// something useful was recovered
}

int CArchiveRecovery::recoverZip(CFile *zipInput, CFile *zipOutput, CTypedPtrList<CPtrList, Gap_Struct*> *filled, bool fullSize)
{
	int	iRet = -1, fileCount = 0;
	try
	{
		CTypedPtrList<CPtrList, ZIP_CentralDirectory*> centralDirectoryEntries;
		Gap_Struct *fill;

		// If the central directory is intact this is simple
		if (fullSize && readZipCentralDirectory(zipInput, &centralDirectoryEntries, filled))
		{
			if (centralDirectoryEntries.GetCount() == 0)
				return false;
			ZIP_CentralDirectory *cdEntry;
			POSITION del, pos = centralDirectoryEntries.GetHeadPosition();
			bool deleteCD;
			for (int i = centralDirectoryEntries.GetCount(); i > 0; i--)
			{
				deleteCD = false;
				del = pos;
				cdEntry = centralDirectoryEntries.GetNext(pos);
				uint32 lenEntry = sizeof(ZIP_Entry) + cdEntry->lenFilename + cdEntry->lenExtraField + cdEntry->lenCompressed;
				if (IsFilled(cdEntry->relativeOffsetOfLocalHeader, static_cast<uint64>(cdEntry->relativeOffsetOfLocalHeader) + lenEntry, filled))
				{
					zipInput->Seek(cdEntry->relativeOffsetOfLocalHeader, CFile::begin);
					// Update offset
					cdEntry->relativeOffsetOfLocalHeader = static_cast<uint32>(zipOutput->GetPosition());
					if (!processZipEntry(zipInput, zipOutput, lenEntry, NULL))
						deleteCD = true;
				}
				else
					deleteCD = true;

				if (deleteCD)
				{
					delete [] cdEntry->filename;
					if (cdEntry->lenExtraField > 0)
						delete [] cdEntry->extraField;
					if (cdEntry->lenComment > 0)
						delete [] cdEntry->comment;
					delete cdEntry;
					centralDirectoryEntries.RemoveAt(del);
				}
			}
		}
		else // Have to scan the file the hard way
		{
			// Loop through filled areas of the file looking for entries
			POSITION pos = filled->GetHeadPosition();
			while (pos != NULL)
			{
				fill = filled->GetNext(pos);
				uint64 qwFilePos = zipInput->GetPosition();
				// The file may have been positioned to the next entry in ScanForMarker() or processZipEntry()
				if (qwFilePos > fill->qwEndOffset)
					continue;
				if (qwFilePos < fill->qwStartOffset)
					zipInput->Seek(fill->qwStartOffset, CFile::begin);

				// If there is any problem, then don't bother checking the rest of this part
				for (;;)
				{
					// Scan for entry marker within this filled area
					if (!scanForZipMarker(zipInput, ZIP_LOCAL_HEADER_MAGIC, static_cast<uint32>(fill->qwEndOffset - zipInput->GetPosition() + 1)))
						break;
					if (zipInput->GetPosition() > fill->qwEndOffset)
						break;
					if (!processZipEntry(zipInput, zipOutput, static_cast<uint32>(fill->qwEndOffset - zipInput->GetPosition() + 1), &centralDirectoryEntries))
						break;
				}
			}
		}

		// Remember offset before CD entries
		uint32 startOffset = static_cast<uint32>(zipOutput->GetPosition());

		// Write all central directory entries
		fileCount = centralDirectoryEntries.GetCount();
		if (fileCount > 0)
		{
			ZIP_CentralDirectory *cdEntry;
			POSITION pos = centralDirectoryEntries.GetHeadPosition();
			while (pos != NULL)
			{
				cdEntry = centralDirectoryEntries.GetNext(pos);

				writeUInt32(zipOutput, ZIP_CD_MAGIC);
				writeUInt16(zipOutput, cdEntry->versionMadeBy);
				writeUInt16(zipOutput, cdEntry->versionToExtract);
				writeUInt16(zipOutput, cdEntry->generalPurposeFlag);
				writeUInt16(zipOutput, cdEntry->compressionMethod);
				writeUInt16(zipOutput, cdEntry->lastModFileTime);
				writeUInt16(zipOutput, cdEntry->lastModFileDate);
				writeUInt32(zipOutput, cdEntry->crc32);
				writeUInt32(zipOutput, cdEntry->lenCompressed);
				writeUInt32(zipOutput, cdEntry->lenUncompressed);
				writeUInt16(zipOutput, cdEntry->lenFilename);
				writeUInt16(zipOutput, cdEntry->lenExtraField);
				writeUInt16(zipOutput, cdEntry->lenComment);
				writeUInt16(zipOutput, 0); // Disk number start
				writeUInt16(zipOutput, cdEntry->internalFileAttributes);
				writeUInt32(zipOutput, cdEntry->externalFileAttributes);
				writeUInt32(zipOutput, cdEntry->relativeOffsetOfLocalHeader);
				zipOutput->Write(cdEntry->filename, cdEntry->lenFilename);
				if (cdEntry->lenExtraField > 0)
					zipOutput->Write(cdEntry->extraField, cdEntry->lenExtraField);
				if (cdEntry->lenComment > 0)
					zipOutput->Write(cdEntry->comment, cdEntry->lenComment);

				delete [] cdEntry->filename;
				if (cdEntry->lenExtraField > 0)
					delete [] cdEntry->extraField;
				if (cdEntry->lenComment > 0)
					delete [] cdEntry->comment;
				delete cdEntry;
			}

			// Remember offset before CD entries
			uint32 endOffset = static_cast<uint32>(zipOutput->GetPosition());

			// Write end of central directory
			writeUInt32(zipOutput, ZIP_END_CD_MAGIC);
			writeUInt16(zipOutput, 0); // Number of this disk
			writeUInt16(zipOutput, 0); // Number of the disk with the start of the central directory
			writeUInt16(zipOutput, static_cast<uint16>(fileCount));
			writeUInt16(zipOutput, static_cast<uint16>(fileCount));
			writeUInt32(zipOutput, endOffset - startOffset);
			writeUInt32(zipOutput, startOffset);
			writeUInt16(zipOutput, CSTRLEN(ZIP_COMMENT));
			zipOutput->Write(ZIP_COMMENT, CSTRLEN(ZIP_COMMENT));

			centralDirectoryEntries.RemoveAll();
		}
		iRet = fileCount;
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...) {}

	// Tell the user how many files were recovered
	AddLogLine(LOG_FL_SBAR, IDS_RECOVER_MULTIPLE, fileCount);

	return iRet;
}

bool CArchiveRecovery::readZipCentralDirectory(CFile *zipInput, CTypedPtrList<CPtrList, ZIP_CentralDirectory*> *centralDirectoryEntries, CTypedPtrList<CPtrList, Gap_Struct*> *filled)
{
	bool retVal = false;
	try
	{
		// Ideally this zip file will not have a comment and the End-CD will be easy to find
		zipInput->Seek(-22, CFile::end);
		if (!(readUInt32(zipInput) == ZIP_END_CD_MAGIC))
		{
			// Have to look for it, comment could be up to 65535 chars but only try with less than 1k
			zipInput->Seek(-1046, CFile::end);
			if (!scanForZipMarker(zipInput, ZIP_END_CD_MAGIC, 1046))
				return false;
			// Skip it again
			readUInt32(zipInput);
		}

		// Found End-CD
		// Only interested in offset of first CD
		zipInput->Seek(12, CFile::current);
		uint32 startOffset = readUInt32(zipInput);
		if (!IsFilled(startOffset, zipInput->GetLength(), filled))
			return false;

		// Goto first CD and start reading
		zipInput->Seek(startOffset, CFile::begin);
		ZIP_CentralDirectory *cdEntry;
		while (readUInt32(zipInput) == ZIP_CD_MAGIC)
		{
			cdEntry = new ZIP_CentralDirectory;
			cdEntry->versionMadeBy				= readUInt16(zipInput);
			cdEntry->versionToExtract			= readUInt16(zipInput);
			cdEntry->generalPurposeFlag			= readUInt16(zipInput);
			cdEntry->compressionMethod			= readUInt16(zipInput);
			cdEntry->lastModFileTime			= readUInt16(zipInput);
			cdEntry->lastModFileDate			= readUInt16(zipInput);
			cdEntry->crc32						= readUInt32(zipInput);
			cdEntry->lenCompressed				= readUInt32(zipInput);
			cdEntry->lenUncompressed			= readUInt32(zipInput);
			cdEntry->lenFilename				= readUInt16(zipInput);
			cdEntry->lenExtraField				= readUInt16(zipInput);
			cdEntry->lenComment					= readUInt16(zipInput);
			cdEntry->diskNumberStart			= readUInt16(zipInput);
			cdEntry->internalFileAttributes		= readUInt16(zipInput);
			cdEntry->externalFileAttributes		= readUInt32(zipInput);
			cdEntry->relativeOffsetOfLocalHeader= readUInt32(zipInput);

			if (cdEntry->lenFilename > 0)
			{
				cdEntry->filename					= new BYTE[cdEntry->lenFilename];
				zipInput->Read(cdEntry->filename, cdEntry->lenFilename);
			}
			if (cdEntry->lenExtraField > 0)
			{
				cdEntry->extraField					= new BYTE[cdEntry->lenExtraField];
				zipInput->Read(cdEntry->extraField, cdEntry->lenExtraField);
			}
			if (cdEntry->lenComment > 0)
			{
				cdEntry->comment					= new BYTE[cdEntry->lenComment];
				zipInput->Read(cdEntry->comment, cdEntry->lenComment);
			}

			centralDirectoryEntries->AddTail(cdEntry);
		}

		retVal = true;
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...) {}

	return retVal;
}

bool CArchiveRecovery::processZipEntry(CFile *zipInput, CFile *zipOutput, uint32 available, CTypedPtrList<CPtrList, ZIP_CentralDirectory*> *centralDirectoryEntries)
{
	if (available < 26)
		return false;

	bool retVal = false;
	try
	{
		// Need to know where it started
		uint32	dwStartOffset = static_cast<uint32>(zipOutput->GetPosition());

		// Entry format :
		//  4      2 bytes  Version needed to extract
		//  6      2 bytes  General purpose bit flag
		//  8      2 bytes  Compression method
		// 10      2 bytes  Last mod file time
		// 12      2 bytes  Last mod file date
		// 14      4 bytes  CRC-32
		// 18      4 bytes  Compressed size (n)
		// 22      4 bytes  Uncompressed size
		// 26      2 bytes  Filename length (f)
		// 28      2 bytes  Extra field length (e)
		//        (f)bytes  Filename
		//        (e)bytes  Extra field
		//        (n)bytes  Compressed data

		// Read header
		if (readUInt32(zipInput) != ZIP_LOCAL_HEADER_MAGIC)
			return false;

		ZIP_Entry entry={0};
		entry.versionToExtract		= readUInt16(zipInput);
		entry.generalPurposeFlag	= readUInt16(zipInput);
		entry.compressionMethod		= readUInt16(zipInput);
		entry.lastModFileTime		= readUInt16(zipInput);
		entry.lastModFileDate		= readUInt16(zipInput);
		entry.crc32					= readUInt32(zipInput);
		entry.lenCompressed			= readUInt32(zipInput);
		entry.lenUncompressed		= readUInt32(zipInput);
		entry.lenFilename			= readUInt16(zipInput);
		entry.lenExtraField			= readUInt16(zipInput);

		// Do some quick checks at this stage that data is looking ok
		if ((entry.crc32 == 0) && (entry.lenCompressed == 0) && (entry.lenUncompressed == 0) && (entry.lenFilename != 0))
			; // this is a directory entry
		else if ((entry.crc32 == 0) || (entry.lenCompressed == 0) || (entry.lenUncompressed == 0) || (entry.lenFilename == 0))
			return false;

		// Is this entry complete
		if ((entry.lenFilename + entry.lenExtraField + entry.lenCompressed) > (available - 26))
		{
			// Move the file pointer to the start of the next entry
			zipInput->Seek((entry.lenFilename + entry.lenExtraField + entry.lenCompressed), CFile::current);
			return false;
		}

		// Filename
		if (entry.lenFilename > MAX_PATH)
			return false; // Possibly corrupt, don't allocate lots of memory
		entry.filename = new BYTE[entry.lenFilename];
		if (zipInput->Read(entry.filename, entry.lenFilename) != entry.lenFilename)
		{
			delete[] entry.filename;
			return false;
		}

		// Extra data
		if (entry.lenExtraField > 0)
		{
			entry.extraField = new BYTE[entry.lenExtraField];
			zipInput->Read(entry.extraField, entry.lenExtraField);
		}

		// Output
		writeUInt32(zipOutput, ZIP_LOCAL_HEADER_MAGIC);
		writeUInt16(zipOutput, entry.versionToExtract);
		writeUInt16(zipOutput, entry.generalPurposeFlag);
		writeUInt16(zipOutput, entry.compressionMethod);
		writeUInt16(zipOutput, entry.lastModFileTime);
		writeUInt16(zipOutput, entry.lastModFileDate);
		writeUInt32(zipOutput, entry.crc32);
		writeUInt32(zipOutput, entry.lenCompressed);
		writeUInt32(zipOutput, entry.lenUncompressed);
		writeUInt16(zipOutput, entry.lenFilename);
		writeUInt16(zipOutput, entry.lenExtraField);
		if (entry.lenFilename > 0)
			zipOutput->Write(entry.filename, entry.lenFilename);
		if (entry.lenExtraField > 0)
			zipOutput->Write(entry.extraField, entry.lenExtraField);

		// Read and write compressed data to avoid reading all into memory
		uint32 written = 0;
		BYTE buf[4096];
		uint32 lenChunk;
		while (written < entry.lenCompressed)
		{
			lenChunk = (entry.lenCompressed - written);
			if (lenChunk > 4096)
				lenChunk = 4096;
			lenChunk = zipInput->Read(buf, lenChunk);
			if (lenChunk == 0)
				break;
			written += lenChunk;
			zipOutput->Write(buf, lenChunk);
		}

		//Central directory:
		if (centralDirectoryEntries != NULL)
		{
			ZIP_CentralDirectory *cdEntry = new ZIP_CentralDirectory;
			cdEntry->header = ZIP_CD_MAGIC;
			cdEntry->versionMadeBy = entry.versionToExtract;
			cdEntry->versionToExtract = entry.versionToExtract;
			cdEntry->generalPurposeFlag = entry.generalPurposeFlag;
			cdEntry->compressionMethod = entry.compressionMethod;
			cdEntry->lastModFileTime = entry.lastModFileTime;
			cdEntry->lastModFileDate = entry.lastModFileDate;
			cdEntry->crc32 = entry.crc32;
			cdEntry->lenCompressed = entry.lenCompressed;
			cdEntry->lenUncompressed = entry.lenUncompressed;
			cdEntry->lenFilename = entry.lenFilename;
			cdEntry->lenExtraField = entry.lenExtraField;
			cdEntry->lenComment = static_cast<uint16>(CSTRLEN(ZIP_COMMENT));
			cdEntry->diskNumberStart = 0;
			cdEntry->internalFileAttributes = 1;
			cdEntry->externalFileAttributes = 0x81B60020;
			cdEntry->relativeOffsetOfLocalHeader = dwStartOffset;
			cdEntry->filename = entry.filename;
			if (entry.lenExtraField > 0)
				cdEntry->extraField = entry.extraField;
			cdEntry->comment = new BYTE[cdEntry->lenComment];
			memcpy(cdEntry->comment, ZIP_COMMENT, CSTRLEN(ZIP_COMMENT));

			centralDirectoryEntries->AddTail(cdEntry);
		}
		else
		{
			delete[] entry.filename;
			if (entry.lenExtraField > 0)
				delete[] entry.extraField;
		}
		retVal = true;
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...) {}

	return retVal;
}

void CArchiveRecovery::DeleteMemory(ThreadParam *tp)
{
	POSITION pos = tp->filled->GetHeadPosition();
	while (pos != NULL)
		delete tp->filled->GetNext(pos);
	tp->filled->RemoveAll();
	delete tp->filled;
	delete tp;
}

bool CArchiveRecovery::CopyFile(CPartFile *partFile, CTypedPtrList<CPtrList, Gap_Struct*> *filled, CString tempFileName)
{
	bool	retVal = false;
	CFile	*srcFile = NULL;

	try
	{
	// Get a new handle to the part file
		srcFile = partFile->GetPartFileHandle().Duplicate();

		// Open destination file and set length to last filled end position
		CFile destFile;
		destFile.Open(tempFileName, CFile::modeWrite | CFile::shareDenyWrite | CFile::modeCreate | CFile::typeBinary);
		Gap_Struct *fill = filled->GetTail();

		if (fill != NULL)
			destFile.SetLength(fill->qwEndOffset + 1ui64);

		BYTE buffer[4096];
		uint32 read;
		uint64 qwCopied;

		// Loop through filled areas and copy data
		partFile->m_bPreviewing = true;
		POSITION pos = filled->GetHeadPosition();
		while (pos != NULL)
		{
			fill = filled->GetNext(pos);
			qwCopied = 0;
			srcFile->Seek(fill->qwStartOffset, CFile::begin);
			destFile.Seek(fill->qwStartOffset, CFile::begin);
			while ((read = srcFile->Read(buffer, 4096)) > 0)
			{
				destFile.Write(buffer, read);
				qwCopied += static_cast<uint64>(read);
				// Stop when finished fill (don't worry about extra)
				if ((fill->qwStartOffset + qwCopied) > fill->qwEndOffset)
					break;
			}
		}
		destFile.Close();
		srcFile->Close();
		partFile->m_bPreviewing = false;

		retVal = true;
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...) {}

	delete srcFile;

	return retVal;
}

int CArchiveRecovery::recoverRar(CFile *rarInput, CFile *rarOutput, CTypedPtrList<CPtrList, Gap_Struct*> *filled)
{
	int			iRet = -1;
	unsigned	uiFileCnt = 0, uiRescueCnt = 0;

	try
	{
		// Try to get file header and main header
		//
		bool bValidFileHeader = false;
		bool bValidMainHeader = false;
		BYTE fileHeader[7];
		RARMAINHDR mainHeader;

		if (rarInput->Read(fileHeader, sizeof(fileHeader)) == sizeof(fileHeader))
		{
			if (fileHeader[0] == 0x52)
			{
				if (fileHeader[1] == 0x45 && fileHeader[2] == 0x7e && fileHeader[3] == 0x5e)
				{
					bValidFileHeader = true;
				}
				else if (fileHeader[1] == 0x61 && fileHeader[2] == 0x72 && fileHeader[3] == 0x21 && fileHeader[4] == 0x1a && fileHeader[5] == 0x07 && fileHeader[6] == 0x00)
				{
					bValidFileHeader = true;

					WORD checkCRC;

					if (rarInput->Read(&checkCRC, sizeof(checkCRC)) == sizeof(checkCRC))
					{
						if (rarInput->Read(&mainHeader, sizeof(mainHeader)) == sizeof(mainHeader))
						{
							if (mainHeader.type == 0x73)
							{
								byte	byteBuf[6];	// sizeof(WORD) + sizeof(DWORD)
								DWORD	crc = crc32(0, (Bytef*)&mainHeader, sizeof(mainHeader));

								if (rarInput->Read(&byteBuf, sizeof(byteBuf)) == sizeof(byteBuf))
								{
									crc = crc32(crc, byteBuf, sizeof(byteBuf));
									if (checkCRC == (WORD)crc)
										bValidMainHeader = true;
								}
							}
						}
					}
				}
			}
			rarInput->SeekToBegin();
		}

		static BYTE start[] =
		{
			// RAR file header
			0x52, 0x61, 0x72, 0x21, 0x1a, 0x07, 0x00,

			// main header
			0x00, 0x00,			// CRC
			0x73,				// type
			0x02, 0x00,			// flags
			0x3B, 0x00,			// size
			0x00, 0x00,			// AV
			0x00, 0x00,			// AV
			0x00, 0x00,			// AV

			// main comment
			0x37, 0xDC,			// CRC
			0x75,				// type
			0x00, 0x00,			// flags
			0x2E, 0x00,			// size

			0x17, 0x00, 0x1D, 0x33, 0x94, 0xF9,
			0x08, 0x15, 0x48, 0xBE, 0x90, 0xF1, 0x5F, 0xE4, 0x30, 0x25, 0x82, 0x4B, 0xEE, 0x68, 0x41, 0x23,
			0x05, 0xBD, 0xDB, 0xFF, 0x6C, 0x8F, 0xBE, 0x9C, 0x21, 0x6B, 0xC8, 0xC5, 0xC8, 0x9A, 0x6E, 0xBE,
			0x80
		};

		start[10] = 0x02;	/*MHD_COMMENT*/
		if (bValidFileHeader && bValidMainHeader && (mainHeader.flags & 0x0008/*MHD_SOLID*/))
		{
		// If this is a 'solid' archive the chance to successfully decompress any entries gets higher,
		// when we pass the 'solid' main header bit to the temp. archive
			start[10] |= 0x08;	/*MHD_SOLID*/
		}
		*((short*)&start[7]) = (short)crc32(0, &start[9], 11);

		rarOutput->Write(start, sizeof(start));

		RAR_BlockFile *block;
		while ((block = scanForRarFileHeader(rarInput, rarInput->GetLength())) != NULL)
		{
		//	Don't check availablity of the next byte in a file for zero data size entries,
		//	which are directories and zero size files
			if ( (block->dataLength == 0) ||
				IsFilled(block->offsetData, block->offsetData + static_cast<uint64>(block->dataLength), filled) )
			{
				uiRescueCnt++;
				// Don't include directories in file count
				if ((block->HEAD_FLAGS & 0xE0) != 0xE0/*LHD_DIRECTORY*/)
					uiFileCnt++;
				writeRarBlock(rarInput, rarOutput, block);
			}
			else
			{
				rarInput->Seek(block->offsetData + block->dataLength, CFile::begin);
			}
			delete[] block->FILE_NAME;
			delete block;
		}
		iRet = uiRescueCnt;
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...) {}

	// Tell the user how many files were recovered
	AddLogLine(LOG_FL_SBAR, IDS_RECOVER_MULTIPLE, uiFileCnt);

	return iRet;
}

bool CArchiveRecovery::IsFilled(uint64 start, uint64 end, CTypedPtrList<CPtrList, Gap_Struct*> *filled)
{
	POSITION	pos = filled->GetHeadPosition();
	Gap_Struct	*fill;
	uint64		qwEnd = end - 1ui64;	// ranges in the list are inclusive

	while (pos != NULL)
	{
		fill = filled->GetNext(pos);
		if (fill->qwStartOffset > start)
			return false;
		if (fill->qwEndOffset >= qwEnd)
			return true;
	}
	return false;
}

// This will find the marker in the file and leave it positioned at the position to read the marker again
bool CArchiveRecovery::scanForZipMarker(CFile *input, uint32 marker, uint32 available)
{
	try
	{
		//uint32 originalOffset = input->GetPosition();
		int lenChunk = 51200; // 50k buffer
		BYTE chunk[51200];
		BYTE *foundPos = NULL;
		int pos = 0;

		while ((available > 0) && ((lenChunk = input->Read(chunk, lenChunk)) > 0))
		{
			available -= lenChunk;
			foundPos = &chunk[0];
			// Move back one, will be incremented in loop
			foundPos--;
			while (foundPos != NULL)
			{
				// Find first matching byte
				foundPos = (BYTE*)memchr( foundPos+1, (marker & 0xFF), (lenChunk - (foundPos+1 - (&chunk[0]))) );
				if (foundPos == NULL)
					continue;

				// Test for end of buffer
				pos = foundPos - &chunk[0];
				if ((pos + 3) > lenChunk)
				{
					// Re-read buffer starting from found first byte position
					input->Seek(pos - lenChunk, CFile::current);
					break;
				}

				// Check for other bytes
				if (chunk[pos + 1] == ((marker >> 8) & 0xFF))
				{
					if (chunk[pos + 2] == ((marker >> 16) & 0xFF))
					{
						if (chunk[pos + 3] == ((marker >> 24) & 0xFF))
						{
							// Found it
							input->Seek(pos - lenChunk, CFile::current);
							return true;
						}
					}
				}
			}
		}
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...) {}

	return false;
}

// This will find a file block in the file and leave it positioned at the end of the filename
RAR_BlockFile *CArchiveRecovery::scanForRarFileHeader(CFile *input, uint64 qwAvailable)
{
	RAR_BlockFile *retVal = NULL;
	try
	{
		int lenChunk = 51200; // 50k buffer
		BYTE chunk[51200];
		BYTE *foundPos = NULL;
		int pos = 0;
		ULONGLONG searchOffset;
		uint16 headCRC;
		BYTE checkCRC[sizeof(RARFILEHDR) + 2 * sizeof(DWORD) + 8 + 32];
		unsigned checkCRCsize;
		uint16 lenFileName;
		BYTE *fileName;
		uint32 crc;

		while ((qwAvailable > 0) && ((lenChunk = input->Read(chunk, lenChunk)) > 0))
		{
			qwAvailable -= static_cast<uint64>(lenChunk);
			foundPos = &chunk[0];
			searchOffset = input->GetPosition() - static_cast<uint64>(lenChunk);

			// Move back one, will be incremented in loop
			foundPos--;
			while (foundPos != NULL)
			{
				// Find rar head block marker
				foundPos = (BYTE*)memchr(foundPos+1, RAR_HEAD_FILE, (lenChunk - (foundPos+1 - &chunk[0])));
				if (foundPos == NULL)
					continue;

				// Move back 2 bytes to get crc and read block
				pos = (int)(foundPos - &chunk[0] - 2);
				input->Seek(pos - lenChunk, CFile::current);

				// CRC of fields from HEAD_TYPE to ATTR + filename + ext. stuff
				headCRC = readUInt16(input);

				RARFILEHDR* hdr = (RARFILEHDR*)checkCRC;
				input->Read(checkCRC, checkCRCsize = sizeof(*hdr));

				// get high parts of 64-bit file size fields
				if (hdr->flags & 0x0100/*LHD_LARGE*/)
				{
					input->Read(&checkCRC[checkCRCsize], sizeof(DWORD) * 2);
					checkCRCsize += sizeof(DWORD) * 2;
				}

				// get filename
				lenFileName = hdr->nameSize;
				fileName = new BYTE[lenFileName];
				input->Read(fileName, lenFileName);

				// get encryption params
				unsigned saltPos = 0;
				if (hdr->flags & 0x0400/*LHD_SALT*/)
				{
					saltPos = checkCRCsize;
					input->Read(&checkCRC[checkCRCsize], 8);
					checkCRCsize += 8;
				}

				// get ext. file date/time
				unsigned extTimePos = 0;
				unsigned extTimeSize = 0;
				if (hdr->flags & 0x1000/*LHD_EXTTIME*/)
				{
					extTimePos = checkCRCsize;
					input->Read(&checkCRC[checkCRCsize], sizeof(WORD));
					unsigned short Flags = *((WORD*)&checkCRC[checkCRCsize]);
					checkCRCsize += sizeof(WORD);
					for (int i = 0; i < 4; i++)
					{
						unsigned int rmode = Flags >> (3 - i) * 4;
						if ((rmode & 8) == 0)
							continue;
						if (i != 0)
						{
							input->Read(&checkCRC[checkCRCsize], sizeof(DWORD));
							checkCRCsize += sizeof(DWORD);
						}
						int count = (rmode & 3) * sizeof(BYTE);

						input->Read(&checkCRC[checkCRCsize], count);
						checkCRCsize += count;
					}
					extTimeSize = checkCRCsize - extTimePos;
				}

				crc = crc32(0, checkCRC, sizeof(*hdr));
				crc = crc32(crc, fileName, lenFileName);
				if (checkCRCsize > sizeof(*hdr))
					crc = crc32(crc, &checkCRC[sizeof(*hdr)], checkCRCsize - sizeof(*hdr));
				if ((crc & 0xFFFF) == headCRC)
				{
					// Found valid crc, build block and return
					// Note that it may still be invalid data, so more checks should be performed
					retVal = new RAR_BlockFile;
					retVal->HEAD_CRC		= headCRC;
					retVal->HEAD_TYPE		= 0x74;
					retVal->HEAD_FLAGS		= calcUInt16(&checkCRC[ 1]);
					retVal->HEAD_SIZE		= calcUInt16(&checkCRC[ 3]);
					retVal->PACK_SIZE		= calcUInt32(&checkCRC[ 5]);
					retVal->UNP_SIZE		= calcUInt32(&checkCRC[ 9]);
					retVal->HOST_OS			= checkCRC[13];
					retVal->FILE_CRC		= calcUInt32(&checkCRC[14]);
					retVal->FTIME			= calcUInt32(&checkCRC[18]);
					retVal->UNP_VER			= checkCRC[22];
					retVal->METHOD			= checkCRC[23];
					retVal->NAME_SIZE		= lenFileName;
					retVal->ATTR			= calcUInt32(&checkCRC[26]);
					// Optional values, present only if bit 0x100 in HEAD_FLAGS is set.
					if ((retVal->HEAD_FLAGS & 0x100) == 0x100/*LHD_LARGE*/)
					{
						retVal->HIGH_PACK_SIZE	= calcUInt32(&checkCRC[30]);
						retVal->HIGH_UNP_SIZE	= calcUInt32(&checkCRC[34]);
					}
					retVal->FILE_NAME		= fileName;
					if (saltPos != 0)
						memcpy(retVal->SALT, &checkCRC[saltPos], sizeof(retVal->SALT));
					if (extTimePos != 0 && extTimeSize != 0)
					{
						retVal->EXT_DATE = new BYTE[extTimeSize];
						memcpy(retVal->EXT_DATE, &checkCRC[extTimePos], retVal->EXT_DATE_SIZE = extTimeSize);
					}

					// Run some quick checks
					if (validateRarFileBlock(retVal))
					{
						// Set some useful markers in the block
						retVal->offsetData = input->GetPosition();
						uint32 dataLength = retVal->PACK_SIZE;
						// If comment present find length
						if ((retVal->HEAD_FLAGS & 0x08) == 0x08/*LHD_COMMENT*/)
						{
							// Skip start of comment block
							input->Seek(5, CFile::current);
							// Read comment length
							dataLength += readUInt16(input);
						}
						retVal->dataLength = dataLength;

						return retVal;
					}
				}
				// If not valid, return to original position, re-read and continue searching
				delete[] fileName;
				delete retVal;
				retVal = NULL;
				input->Seek(searchOffset, CFile::begin);
				input->Read(chunk, lenChunk);
			}
		}
	}
	catch (CFileException* error)
	{
		error->Delete();
	}
	catch (...) {}

	return NULL;
}

// This assumes that head crc has already been checked
bool CArchiveRecovery::validateRarFileBlock(RAR_BlockFile *block)
{
	if (block->HEAD_TYPE != 0x74)
		return false;
	if ((block->HEAD_FLAGS & 0x0400/*LHD_SALT*/) == 0 && block->UNP_SIZE < block->PACK_SIZE)
		return false;
	if (block->HOST_OS > 5)
		return false;
	switch (block->METHOD)
	{
		case 0x30: // storing
		case 0x31: // fastest compression
		case 0x32: // fast compression
		case 0x33: // normal compression
		case 0x34: // good compression
		case 0x35: // best compression
			break;
		default:
			return false;
	}
//	0x0200 - FILE_NAME contains both usual and encoded
//	Unicode name separated by zero. In this case
//	NAME_SIZE field is equal to the length
//	of usual name plus encoded Unicode name plus 1.
	if (block->HEAD_FLAGS & 0x0200/*LHD_UNICODE*/)
	{
		// ANSI+'\0'+Unicode name
		if (block->NAME_SIZE > (MAX_PATH + MAX_PATH*sizeof(WCHAR) + 1))
			return false;
	}
	else if (block->NAME_SIZE > MAX_PATH)	// ANSI
		return false;
	// Check directory entry has no size
	if (((block->HEAD_FLAGS & 0xE0) == 0xE0/*LHD_DIRECTORY*/) && ((block->PACK_SIZE + block->UNP_SIZE + block->FILE_CRC) > 0))
		return false;

	return true;
}

void CArchiveRecovery::writeRarBlock(CFile *input, CFile *output, RAR_BlockFile *block)
{
	ULONGLONG offsetStart = output->GetPosition();
	try
	{
		writeUInt16(output, block->HEAD_CRC);
		output->Write(&block->HEAD_TYPE, 1);
		writeUInt16(output, block->HEAD_FLAGS);
		writeUInt16(output, block->HEAD_SIZE);
		writeUInt32(output, block->PACK_SIZE);
		writeUInt32(output, block->UNP_SIZE);
		output->Write(&block->HOST_OS, 1);
		writeUInt32(output, block->FILE_CRC);
		writeUInt32(output, block->FTIME);
		output->Write(&block->UNP_VER, 1);
		output->Write(&block->METHOD, 1);
		writeUInt16(output, block->NAME_SIZE);
		writeUInt32(output, block->ATTR);
		// Optional values, present only if bit 0x100 in HEAD_FLAGS is set.
		if ((block->HEAD_FLAGS & 0x100) == 0x100/*LHD_LARGE*/)
		{
			writeUInt32(output, block->HIGH_PACK_SIZE);
			writeUInt32(output, block->HIGH_UNP_SIZE);
		}
		output->Write(block->FILE_NAME, block->NAME_SIZE);
		if (block->HEAD_FLAGS & 0x0400/*LHD_SALT*/)
			output->Write(block->SALT, sizeof(block->SALT));
		output->Write(block->EXT_DATE, block->EXT_DATE_SIZE);

		// Now copy compressed data from input file
		uint32 lenToCopy = block->dataLength;
		if (lenToCopy > 0)
		{
			input->Seek(block->offsetData, CFile::begin);
			uint32 written = 0;
			BYTE chunk[4096];
			uint32 lenChunk;
			while (written < lenToCopy)
			{
				lenChunk = (lenToCopy - written);
				if (lenChunk > 4096)
					lenChunk = 4096;
				lenChunk = input->Read(chunk, lenChunk);
				if (lenChunk == 0)
					break;
				written += lenChunk;
				output->Write(chunk, lenChunk);
			}
		}
	}
	catch (CFileException* error)
	{
		error->Delete();
		try { output->SetLength(offsetStart); } catch (...) {}
	}
	catch (...)
	{
		try { output->SetLength(offsetStart); } catch (...) {}
	}
}

uint16 CArchiveRecovery::readUInt16(CFile *input)
{
	uint16 retVal = 0;
	BYTE b[2];
	if (input->Read(b, 2) > 0)
		retVal = (b[1] << 8) + b[0];
	return retVal;
}

uint32 CArchiveRecovery::readUInt32(CFile *input)
{
	uint32 retVal = 0;
	BYTE b[4];
	if (input->Read(b, 4) > 0)
		retVal = (b[3] << 24) + (b[2] << 16) + (b[1] << 8) + b[0];
	return retVal;
}

uint16 CArchiveRecovery::calcUInt16(BYTE *input)
{
	return (((uint16)input[1]) << 8) + ((uint16)input[0]);
}

uint32 CArchiveRecovery::calcUInt32(BYTE *input)
{
	return (((uint32)input[3]) << 24) + (((uint32)input[2]) << 16) + (((uint32)input[1]) << 8) + ((uint32)input[0]);
}

void CArchiveRecovery::writeUInt16(CFile *output, uint16 val)
{
	BYTE b[2];
	b[0] = static_cast<BYTE>(val & 0x000000ff);
	b[1] = static_cast<BYTE>((val & 0x0000ff00) >>  8);
	output->Write(b, 2);
}

void CArchiveRecovery::writeUInt32(CFile *output, uint32 val)
{
	BYTE b[4];
	b[0] = static_cast<BYTE>(val & 0x000000ff);
	b[1] = static_cast<BYTE>((val & 0x0000ff00) >>  8);
	b[2] = static_cast<BYTE>((val & 0x00ff0000) >> 16);
	b[3] = static_cast<BYTE>((val & 0xff000000) >> 24);
	output->Write(b, 4);
}
