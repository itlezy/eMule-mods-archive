//this file is part of eMule
//Copyright (C)2006 David Xanatos ( Xanatos@Lycos.at / http://neomule.sourceforge.net )
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
#include "Addons/ImportParts/functions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define PART_FILEEXTENSION	_T(".part") //morph4u

////////////////////////////////////////////////
// Common functions

bool ESCBreakIE(){
	if (GetAsyncKeyState(VK_ESCAPE) < 0)
		if (AfxMessageBox(GetResString(IDS_IMPORTPARTS_ABORT),MB_YESNO,NULL)==IDYES){
			Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_ABORTED));
			return true;
		}
	return false;
}

////////////////////////////////////////////////
// Import functions

struct ImportInfo
{
	CPartFile* PartFile;
	CString ImportPath;
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
	SendMessage(theApp.emuledlg->m_hWnd, TM_IMPORTPART, (WPARAM)importpart, (LPARAM)partfile);
}

UINT AFX_CDECL RunImportParts(LPVOID lpParam)
{
	ImportInfo* Instructions = (ImportInfo*) lpParam;

	ASSERT(Instructions->PartFile);
	if(!Instructions->PartFile)
		Log(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_ERR_NULL));
	else
		Instructions->PartFile->PerformImportParts(Instructions);

	Instructions->PartFile->SetFileOp(PFOP_NONE);
	VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEIMPORTED, NULL, (LPARAM)Instructions->PartFile) );

	delete Instructions;

	return 0;
}

bool CPartFile::ImportParts()
{
	if(!IsPartFile()){
		Log(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_ERR_ALREADYCOMPLETE));
		return false;
	}

	// Disallow files without hashset, unless it is one part long file
	if (GetPartCount() != 1 && GetFileIdentifier().GetAvailableMD4PartHashCount() < GetPartCount()){
		Log(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_ERR_HASHSETINCOMPLETE), GetFileName(), GetFileIdentifier().GetAvailableMD4PartHashCount(), GetPartCount()); // do not try to import to files without hashset.
		return false;
	}

	// Disallow very small files
	// Maybe I make them allowed in future when I make my program insert slices in partially downloaded parts.
	if ((uint64)GetFileSize() < EMBLOCKSIZE){
		Log(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_ERR_FILETOOSMALL));
		return false;
	}

	//CFileDialog dlg(true, NULL, NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY);//morph4u
	CFileDialog dlg(true, PART_FILEEXTENSION, NULL, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY, GetResString(IDS_PARTS) + _T(" (*") PART_FILEEXTENSION _T(")|*") PART_FILEEXTENSION _T("||"));

	if(dlg.DoModal() != IDOK)
		return false;
	CString pathName = dlg.GetPathName();

	// import single chunk files always in serial mode
	SetFileOp(PFOP_IMPORTING);
	SetFileOpProgress(0);

	SetStatus(PS_IMPORTING);

	ImportInfo* Instructions = new ImportInfo;
	Instructions->ImportPath = pathName;
	Instructions->PartFile = this;
	//Instructions->SerialMode = Serial;

	AfxBeginThread(RunImportParts, (LPVOID)Instructions, THREAD_PRIORITY_BELOW_NORMAL);

	return true;
}

BOOL CPartFile::PerformImportParts(ImportInfo* Instructions)
{
	CFile f;
	if(!f.Open(Instructions->ImportPath, CFile::modeRead)){
		Log(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_ERR_CANTOPENFILE), Instructions->ImportPath);
		return 0;
	}

	FlushBuffer(true);

	UINT uPartSuccess = 0;
	UINT uIncPartSuccess = 0;
	UINT uBadPartSuccess = 0;
	ULONGLONG fileSize = f.GetLength();
	uchar *partData = new uchar[PARTSIZE];
	UINT uPartSize = 0;
	UINT uPart = 0;
	UINT uPartCount = 0;
	uchar hash[16];
	CKnownFile *kfcall = new CKnownFile;
	// David: Auto selection, we need Out Of Order mode only when the file is an export
	// and then the file is equal n*PARTSIZE obtionaliy + lastpartsize (< PARTSIZE)
	// when the file is not an export there is no reason to assume that the chunks are not in the default order
	// so we can use the Serial mode that allows us to import incomplete parts to
	bool bOutOfOrder = (GetED2KPartCount() && (f.GetLength() % PARTSIZE == 0 || f.GetLength() % PARTSIZE == GetPartSize(GetPartCount()-1)) && f.GetLength() != GetFileSize()); // ZZUL-TRA :: SafeHash
	// Allow import op incomplete parts that we have completely empty, in Out Of Order mode we can not import unidentified parts
	BOOL bPartialMode = bOutOfOrder ? 0 : 2;
	// Allow import of single blocks to incomplete parts, user will be asked if needed, 
	//		answer will be remembered for this import, in Out Of Order mode we can not import blocks
	BOOL bBlockMode = bOutOfOrder ? 0 : 2;

	Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_IMPORTSTART), (UINT)(fileSize/PARTSIZE), GetFileName());

	try{
		do
		{
			// in case of shutdown while still importing
			if (theApp.emuledlg == NULL || !theApp.emuledlg->IsRunning())
				break;

			if (ESCBreakIE())
				break;

			if(m_nTotalBufferData > PARTSIZE*10){ // never buffer more than 10 parts !!!
				SendMessage(theApp.emuledlg->m_hWnd, TM_IMPORTPART, (WPARAM)0, (LPARAM)this); // Ping the FlushBuffer to check is space allocation done
				Sleep(500);
				continue;
			}

			CSingleLock sLock1(&theApp.hashing_mut, TRUE);	// SLUGFILLER: SafeHash - only file operation at a time

			// read part
			uPartSize = f.Read(partData, PARTSIZE);
			if(uPartSize == 0)
				break;

			// create hash of our part
			kfcall->CreateHash(partData, uPartSize, hash);

			if(bOutOfOrder){
				// find maching part in our file
				for(uPart = 0; uPart < GetPartCount(); uPart++){
					if(md4cmp(hash, GetFileIdentifier().GetMD4PartHash(uPart)) == 0){
						Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_FOUND), uPartCount, uPart);
						break;
					}
				}
			}
			else
				uPart = uPartCount; // serial

			if(uPart >= GetPartCount()) // part not found nr source file to large
			{
				Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_NOT_FOUND), uPartCount);
				uBadPartSuccess++;
			}
			else if(IsComplete((uint64)uPart*PARTSIZE, (uint64)(uPart+1)*PARTSIZE-1, false)) // we have the part already complete
			{
				Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_PARTSKIPPEDALREADYCOMPLETE), uPart);
			}
			else if(bOutOfOrder || md4cmp(hash, (GetPartCount() == 1 ? GetFileHash() : GetFileIdentifier().GetMD4PartHash(uPart))) == 0) // part is matching import it
			{
				WriteToPartFile(this, partData, (uint64)uPart*PARTSIZE, (uint64)uPart*PARTSIZE + uPartSize - 1);
				Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_PARTIMPORTEDGOOD), uPart);
				uPartSuccess++;
			}
			else if((uint32)hash[0] == 215 && (uint32)hash[1] == 222 && (uint32)hash[2] == 242 && (uint32)hash[3] == 98) // empty
			{
				// this part is completely empty drop it
			}
			else if(IsPureGap((uint64)uPart*PARTSIZE, (uint64)(uPart+1)*PARTSIZE-1)) // part does not match (incomplete?) import it if we don't have any data yet
			{
				if(bPartialMode && bPartialMode == 1 || (bPartialMode = (AfxMessageBox(StrLine(GetResString(IDS_IMPORTPARTS_INCOMPLETE), Instructions->ImportPath),MB_YESNO,NULL) == IDYES)) == true){
					WriteToPartFile(this, partData, (uint64)uPart*PARTSIZE, (uint64)uPart*PARTSIZE + uPartSize - 1);
					Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_PARTIMPORTEDBAD), uPart);
					uIncPartSuccess++;
				} else {
					Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_PARTSKIPPEDDONTMATCH), uPart);
					uBadPartSuccess++;
				}
			} 
			else // part incomplete but we have an own incomplete one so dont overwrite our's
			{
				// David: We can in gegeraly not veryfy the blocks on the fly, 
				//		because we need to request the proper AICH recovery data before,
				//		and wait some time for the answer, but we can import them now and verify later.
				if(bBlockMode && bBlockMode == 1 || (bBlockMode = (AfxMessageBox(StrLine(GetResString(IDS_IMPORTPARTS_BLOCKS), Instructions->ImportPath), MB_YESNO, NULL) == IDYES)) == true){
					uint32 length = PARTSIZE;
					if ((ULONGLONG)PARTSIZE*(uint64)(uPart+1) > f.GetLength()){
						length = (UINT)(f.GetLength() - ((ULONGLONG)PARTSIZE*(uint64)uPart));
						ASSERT( length <= PARTSIZE );
					}

					for(uint8 nBlock = 0; nBlock != 53; nBlock++){
						const uint32 nBlockOffset = nBlock*EMBLOCKSIZE;
						const uint64 nBlockStart = (uint64)uPart*PARTSIZE + (uint64)nBlock*EMBLOCKSIZE;
						const uint64 nBlockSize = min(EMBLOCKSIZE, length - (uint64)nBlock*EMBLOCKSIZE);
						if (IsPureGap(nBlockStart, nBlockStart + nBlockSize - 1))
							WriteToPartFile(this, partData+nBlockOffset, nBlockStart, nBlockStart + nBlockSize - 1);
					}
					Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_PARTIMPORTEDBAD), uPart);
					uIncPartSuccess++;
				} else {
					Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_PARTSKIPPEDDONTMATCH), uPart);
					uBadPartSuccess++;
				}
			}

			if (theApp.emuledlg && theApp.emuledlg->IsRunning()){
				UINT uProgress = (UINT)(uint64)(((uint64)(uPartCount*PARTSIZE + uPartSize) * 100) / fileSize);
				ASSERT( uProgress <= 100 );
				VERIFY(PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)this));
			}
			uPartCount++;
		}
		while(uPartSize == PARTSIZE);
	}
	catch(CFileException* e){
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		e->GetErrorMessage(szError, ARRSIZE(szError));
		Log(LOG_ERROR | LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_ERR_FILE), szError, uPart, Instructions->ImportPath, GetFileName());
		e->Delete();
	}

	f.Close();

	delete[] partData;
	delete kfcall;
	
	Log(LOG_STATUSBAR, GetResString(IDS_IMPORTPARTS_IMPORTFINISH), GetFileName(), uPartSuccess, uIncPartSuccess, uBadPartSuccess);
	
	return 0;
}

uint32 CKnownFile::GetPartSize(UINT uPart) const
{
	if(uPart+1 < GetPartCount())
		return PARTSIZE;

	uint32 size = (uint32)(uint64)m_nFileSize % PARTSIZE;
	if(size)
		return size;
	return PARTSIZE;
}