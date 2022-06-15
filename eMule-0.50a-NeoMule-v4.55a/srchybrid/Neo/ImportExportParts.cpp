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
#include "resource.h"
#include "emule.h"
#include "emuleDlg.h"
#include "Log.h"
#include "Opcodes.h"
#include "PartFile.h"
#include "SHAHashSet.h"
#include "SharedFileList.h"
#include "SafeFile.h"
#include "Functions.h"
#include "NeoPreferences.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// NEO: PIX - [PartImportExport] -- Xanatos -->

////////////////////////////////////////////////
// Common functions

bool ESCBreakIE(){
	if (GetAsyncKeyState(VK_ESCAPE) < 0)
		if (AfxMessageBox(GetResString(IDS_X_IMPORTPARTS_ABORT),MB_YESNO,NULL)==IDYES){
			ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_ABORTED));
			return true;
		}
	return false;
}

////////////////////////////////////////////////
// Export functions

struct ExportInfo
{
	CKnownFile* KnownFile;
	CString ExportPath;
	const CList<uint16>* PartList;
};

UINT AFX_CDECL RunExportParts(LPVOID lpParam)
{
	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END

	ExportInfo* Instructions = (ExportInfo*) lpParam;

	ASSERT(Instructions->KnownFile);
	ASSERT(Instructions->PartList->GetCount());
	if(!Instructions->KnownFile || !Instructions->PartList->GetCount()){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_NULL));
	}else{
		// NEO: SSH - [SlugFillerSafeHash]
		// SLUGFILLER: SafeHash remove - locking code removed, unnecessary, now locking in CreateHash
		//CSingleLock sLock1(&(theApp.hashing_mut), TRUE); // only one file operation at a time

		Instructions->KnownFile->PerformExportParts(Instructions);
	}

	delete Instructions->PartList;
	delete Instructions;

	return 0;
}

bool CKnownFile::ExportParts(const CList<uint16>* PartList)
{
	// Disallow very small files
	// Maybe I make them allowed in future when I make my program insert slices in partially downloaded parts.
	if ((uint64)GetFileSize() < EMBLOCKSIZE){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_FILETOOSMALL)); // do not try to import to files without hashset.
		return false;
	}

	CFileDialog dlg(false, NULL, NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_EXPLORER);
	if(dlg.DoModal() != IDOK){
		delete PartList;
		return false;
	}
	CString pathName = dlg.GetPathName();

	ExportInfo* Instructions = new ExportInfo;
	Instructions->ExportPath = pathName;
	Instructions->KnownFile = this;
	Instructions->PartList = PartList;

	AfxBeginThread(RunExportParts,(LPVOID)Instructions,THREAD_PRIORITY_BELOW_NORMAL);

	return true;
}

BOOL CKnownFile::PerformExportParts(ExportInfo* Instructions)
{
	CFile f;
	if(!f.Open(Instructions->ExportPath, CFile::modeCreate | CFile::modeWrite)){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_CANTOPENFILE), Instructions->ExportPath);
		return 0;
	}

	UINT	partsuccess = 0;
	UINT	badpartsuccess = 0;
	uchar	*partData = new uchar[PARTSIZE];
	UINT	partSize;
	UINT	part = 0;
	CSyncHelper lockFile;
	CFile file;
	CString fullname;

	POSITION pos = Instructions->PartList->GetHeadPosition();
	try{
		while(pos)
		{
			// in case of shutdown while still importing
			if (theApp.emuledlg==NULL || !theApp.emuledlg->IsRunning())
				break;

			if (ESCBreakIE()) 
				break;

			part = Instructions->PartList->GetNext(pos);
			partSize = (UINT)GetPartSize(part);

			CSingleLock sLock1(&theApp.hashing_mut, TRUE);	// SLUGFILLER: SafeHash - only file operation at a time // NEO: SSH - [SlugFillerSafeHash]

			// get file mutex
			if (IsPartFile() && ((CPartFile*)this)->GetStatus() != PS_COMPLETE){
				if (!((CPartFile*)this)->m_FileCompleteMutex.Lock(0)){
					ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_COMPLETING));
					break;
				}
				lockFile.m_pObject = &((CPartFile*)this)->m_FileCompleteMutex;

				fullname = RemoveFileExtension(((CPartFile*)this)->GetFullName());
			}
			else{
				//fullname.Format(_T("%s\\%s"),GetPath(),GetFileName());
				fullname.Format(_T("%s\\%s"),GetPath(),GetFileName(true)); // NEO: PP - [PasswordProtection]
			}

			// in case the part is not complete skip ist
			if (IsPartFile() && !((CPartFile*)this)->IsComplete(PARTSIZE*part, PARTSIZE*part+PARTSIZE-1, true)){
				ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_INCOMPLET), part);
				badpartsuccess ++;
				continue;
			}
			
			// open file and read the content
			//if (!IsPartFile()){
			if (!file.Open(fullname,CFile::modeRead|CFile::osSequentialScan|CFile::shareDenyNone)){
				break;
			}
			file.Seek(PARTSIZE*part,CFile::begin);
			
			if (file.Read(partData,partSize) != partSize){
				file.Close();
				ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_CANTREAD), fullname, part);
				break;
			}
			file.Close();
			/*}
			else{
				((CPartFile*)this)->m_hpartfile.Seek(PARTSIZE*part,CFile::begin);
				
				if (((CPartFile*)this)->m_hpartfile.Read(partData,partSize) != partSize){
					ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_CANTREAD), fullname);
					break;
				}
			}*/

			// release file mutex
			if (lockFile.m_pObject){
				lockFile.m_pObject->Unlock(); // Unlock the (part) file as soon as we are done with accessing it.
				lockFile.m_pObject = NULL;
			}

			// write part
			f.Write(partData, partSize);
		
			ModLog(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_PARTIMPORTEDGOOD), part);
			partsuccess++;
		}
	}
	catch(CFileException* e){
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_ERR_FILE), szError, part, fullname, Instructions->ExportPath);
		e->Delete();
	}

	delete[] partData;

	f.Close();

	ModLog(LOG_STATUSBAR, GetResString(IDS_X_EXPORTPARTS_EXPORTFINISH), partsuccess, badpartsuccess, fullname);

	return 0;
}

////////////////////////////////////////////////
// Import functions

struct ImportInfo
{
	CPartFile* PartFile;
	CString ImportPath;
	//BOOL SerialMode;
};

inline void WriteToPartFile(CPartFile *partfile, const BYTE *data, uint64 start, uint64 end)
{
	uint32 lenData = (uint32)(end - start + 1);
	BYTE *partData = new BYTE[lenData];
	memcpy(partData, data, lenData);

	ImportPart_Struct* importpart = new ImportPart_Struct;
	importpart->start = start;
	importpart->end = end;
	importpart->data = partData;
	SendMessage(theApp.emuledlg->m_hWnd,TM_IMPORTPART,(WPARAM)importpart,(LPARAM)partfile);
}

UINT AFX_CDECL RunImportParts(LPVOID lpParam)
{
	// SLUGFILLER: SafeHash // NEO: STS - [SlugFillerThreadSafe]
	CReadWriteLock lock(&theApp.m_threadlock);
	if (!lock.ReadLock(0))
		return 0;
	// SLUGFILLER: SafeHash // NEO: STS END

	ImportInfo* Instructions = (ImportInfo*) lpParam;

	ASSERT(Instructions->PartFile);
	if(!Instructions->PartFile){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_ERR_NULL));
	}else{
		// NEO: SSH - [SlugFillerSafeHash]
		// SLUGFILLER: SafeHash remove - locking code removed, unnecessary, now locking in CreateHash
		//CSingleLock sLock1(&(theApp.hashing_mut), TRUE); // only one file operation at a time

		Instructions->PartFile->PerformImportParts(Instructions);
	}

	Instructions->PartFile->SetFileOp(PFOP_NONE);
	VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEIMPORTED, NULL, (LPARAM)Instructions->PartFile) );

	delete Instructions;

	return 0;
}

bool CPartFile::ImportParts()
{
	if(!IsPartFile()){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_ERR_ALREADYCOMPLETE));
		return false;
	}

	// Disallow files without hashset, unless it is one part long file
	if (GetPartCount() != 1 && GetHashCount() < GetPartCount()){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_ERR_HASHSETINCOMPLETE), GetFileName(), GetHashCount(), GetPartCount()); // do not try to import to files without hashset.
		return false;
	}

	// Disallow very small files
	// Maybe I make them allowed in future when I make my program insert slices in partially downloaded parts.
	if ((uint64)GetFileSize() < EMBLOCKSIZE){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_ERR_FILETOOSMALL));
		return false;
	}

	CFileDialog dlg(true, NULL, NULL, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY);
	if(dlg.DoModal() != IDOK)
		return false;
	CString pathName = dlg.GetPathName();

	// import single chunk files always in serial mode
	//BOOL Serial = (GetPartCount() == 1 || (AfxMessageBox(StrLine(GetResString(IDS_X_IMPORTPARTS_SERIAL),pathName),MB_YESNO,NULL) == IDYES) );

	SetFileOp(PFOP_IMPORTING);
	SetFileOpProgress(0);

	SetStatus(PS_IMPORTING);

	ImportInfo* Instructions = new ImportInfo;
	Instructions->ImportPath = pathName;
	Instructions->PartFile = this;
	//Instructions->SerialMode = Serial;

	AfxBeginThread(RunImportParts,(LPVOID)Instructions,THREAD_PRIORITY_BELOW_NORMAL);

	return true;
}

BOOL CPartFile::PerformImportParts(ImportInfo* Instructions)
{
	CFile f;
	if(!f.Open(Instructions->ImportPath, CFile::modeRead)){
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_ERR_CANTOPENFILE), Instructions->ImportPath);
		return 0;
	}

	FlushBuffer(true);

	UINT	partsuccess = 0;
	UINT	incpartsuccess = 0;
	UINT	badpartsuccess = 0;
	ULONGLONG fileSize = f.GetLength();
	uchar	*partData = new uchar[PARTSIZE];
	UINT	partSize = 0;
	UINT	part = 0;
	UINT	partCount = 0;
	uchar	hash[16];
	CKnownFile *kfcall = new CKnownFile;
	//BOOL	OutOfOrder = (Instructions->SerialMode == FALSE);
	// David: Auto selection, we need Out Of Order mode only when the file is an export
	//		and then the file is equal n*PARTSIZE obtionaliy + lastpartsize (< PARTSIZE)
	//		when the file is not an export there is no reason to assume that the chunks are not in the default order
	//		so we can use the Serial mode that llows us to import incomplete parts to
	BOOL	OutOfOrder = (GetED2KPartHashCount() && (f.GetLength() % PARTSIZE == 0 || f.GetLength() % PARTSIZE == GetPartSize(GetPartCount()-1)) && f.GetLength() != GetFileSize());
	// Allow import op imcomplet parts that we have completly empty, in Out Of Order mode we cen not import unidentyfyed parts
	BOOL	PartialMode = OutOfOrder ? 0 : 2;
	// Allow import of single blocks to imcomplete parts, user will be asked if needed, 
	//		answer will be remembered for this import, in Out Of Order mode we cen not import blocks
	BOOL	BlockMode = OutOfOrder ? 0 : 2;

	ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_IMPORTSTART), (UINT)(fileSize/PARTSIZE), GetFileName());

	try{
		do
		{
			// in case of shutdown while still importing
			if (theApp.emuledlg==NULL || !theApp.emuledlg->IsRunning())
				break;

			if (ESCBreakIE())
				break;

			if(m_nTotalBufferData > PARTSIZE*10){ // never buffer more than 10 parts !!!
				SendMessage(theApp.emuledlg->m_hWnd,TM_IMPORTPART,(WPARAM)0,(LPARAM)this); // Ping the FlushBuffer to check is space allocation done
				Sleep(500);
				continue;
			}

			CSingleLock sLock1(&theApp.hashing_mut, TRUE);	// SLUGFILLER: SafeHash - only file operation at a time // NEO: SSH - [SlugFillerSafeHash]

			// read part
			partSize = f.Read(partData, PARTSIZE);
			if(partSize == 0)
				break;

			// create hash of our part
			kfcall->CreateHash(partData, partSize, hash);

			if(OutOfOrder)
			{
				// find maching part in our file
				for(part = 0; part < GetPartCount(); part++)
				{
					if(md4cmp(hash, GetPartHash(part)) == 0)
					{
						ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_FOUND), partCount, part);
						break;
					}
				}
			}
			else
			{
				part = partCount; // serial
			}

			if(part >= GetPartCount()) // part not found nr source file to large
			{
				ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_NOT_FOUND), partCount);
				badpartsuccess ++;
			}
			else if(IsComplete((uint64)part*PARTSIZE, (uint64)(part+1)*PARTSIZE-1, false)) // we have the part already complete
			{
				ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_PARTSKIPPEDALREADYCOMPLETE), part);
			}
			else if(OutOfOrder || md4cmp(hash, (GetPartCount() == 1 ? GetFileHash() : GetPartHash(part)))==0) // part is maching import it
			{
				WriteToPartFile(this, partData, (uint64)part*PARTSIZE, (uint64)part*PARTSIZE+partSize-1);
				ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_PARTIMPORTEDGOOD), part);
				partsuccess ++;
			}
			else if((uint32)hash[0] == 215 && (uint32)hash[1] == 222 && (uint32)hash[2] == 242 && (uint32)hash[3] == 98) // empty
			{
				// this part is compleetly empty drop it
			}
			else if(IsPureGap((uint64)part*PARTSIZE, (uint64)(part+1)*PARTSIZE-1)) // part does not mach (incomplete?) import it if we dont have any data yet
			{
				if(PartialMode && PartialMode == 1 || (PartialMode = (AfxMessageBox(StrLine(GetResString(IDS_X_IMPORTPARTS_INCOMPLETE),Instructions->ImportPath),MB_YESNO,NULL) == IDYES)) == true){
					WriteToPartFile(this, partData, (uint64)part*PARTSIZE, (uint64)part*PARTSIZE+partSize-1);
					ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_PARTIMPORTEDBAD), part);
					incpartsuccess ++;
				}else{
					ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_PARTSKIPPEDDONTMATCH), part);
					badpartsuccess ++;
				}
			} 
			else // part incomplete but we have an own incomplete one so dont overwrite our's
			{
				// David: We can in gegeraly not veryfy the blocks on the fly, 
				//		becouse we need to request the proper AICH recovery data bevoure,
				//		and wait some time for the answer, but we can import them now and veryfy later.
				if(BlockMode && BlockMode == 1 || (BlockMode = (AfxMessageBox(StrLine(GetResString(IDS_X_IMPORTPARTS_BLOCKS),Instructions->ImportPath),MB_YESNO,NULL) == IDYES)) == true){

					uint32 length = PARTSIZE;
					if ((ULONGLONG)PARTSIZE*(uint64)(part+1) > f.GetLength()){
						length = (UINT)(f.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)part));
						ASSERT( length <= PARTSIZE );
					}

					for(uint8 nBlock = 0; nBlock != 53; nBlock++)
					{
						const uint32 nBlockOffset = nBlock*EMBLOCKSIZE;
						const uint64 nBlockStart = (uint64)part*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE;
						const uint64 nBlockSize = min(EMBLOCKSIZE, length - (uint64)nBlock*EMBLOCKSIZE);
						if (IsPureGap(nBlockStart, nBlockStart + nBlockSize - 1))
							WriteToPartFile(this, partData+nBlockOffset, nBlockStart, nBlockStart + nBlockSize - 1);
						//if (IsPureGap((uint64)part*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE, (uint64)part*PARTSIZE + (uint64)(nBlock + 1)*EMBLOCKSIZE - 1))
						//	WriteToPartFile(this, partData+(nBlock*EMBLOCKSIZE), (uint64)part*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE, (uint64)part*PARTSIZE + (uint64)(nBlock + 1)*EMBLOCKSIZE - 1);
					}
					ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_PARTIMPORTEDBAD), part);
					incpartsuccess ++;
				}else{
					ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_PARTSKIPPEDDONTMATCH), part);
					badpartsuccess ++;
				}
			}

			if (theApp.emuledlg && theApp.emuledlg->IsRunning()){
				UINT uProgress = (UINT)(uint64)(((uint64)(partCount*PARTSIZE + partSize) * 100) / fileSize);
				ASSERT( uProgress <= 100 );
				VERIFY(PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)this));
			}
			partCount ++;
		}
		while(partSize == PARTSIZE);
	}
	catch(CFileException* e){
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		ModLogError(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_ERR_FILE), szError, part, Instructions->ImportPath, GetFileName());
		e->Delete();
	}

	f.Close();

	delete[] partData;
	delete kfcall;
	
	ModLog(LOG_STATUSBAR, GetResString(IDS_X_IMPORTPARTS_IMPORTFINISH), GetFileName(), partsuccess, incpartsuccess, badpartsuccess);
	
	return 0;
}
// NEO: PIX END <-- Xanatos --