#include "stdafx.h"
#include "resource.h"
#include "SR13-ImportParts.h"
#include "emule.h"
#include "emuleDlg.h"
#include "Log.h"
#include "Opcodes.h"
#include "PartFile.h"
#include "SHAHashSet.h"
#include "SharedFileList.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

bool CKnownFile::SR13_ImportParts(){
	// General idea from xmrb's CKnownFile::ImportParts()
	// Unlike xmrb's version which scans entire file designated for import and then tries
	// to match each PARTSIZE bytes with all parts of partfile, my version assumes that
	// in file you're importing all parts stay on same place as they should be in partfile
	// (for example you're importing damaged version of file to recover some parts from ED2K)
	// That way it works much faster and almost always it is what expected from this
	// function. --Rowaa[SR13].
	// CHANGE BY SIROB, Only comput missing full chunk part
	if(!IsPartFile()){
		LogError(LOG_STATUSBAR, GetResString(IDS_SR13_IMPORTPARTS_ERR_ALREADYCOMPLETE));
		return false;
	}

	CPartFile* partfile=(CPartFile*)this;
	if (partfile->GetFileOp() == PFOP_SR13_IMPORTPARTS) {
		partfile->SetFileOp(PFOP_NONE);
		return false;
	}
	//MORPH - Import Part, not needed we import every thing we can now.
	/*
	// Disallow files without hashset, unless it is one part long file
	if (GetPartCount() != 1 && GetHashCount() < GetPartCount()){
		LogError(LOG_STATUSBAR, GetResString(IDS_SR13_IMPORTPARTS_ERR_HASHSETINCOMPLETE), GetFileName(), GetHashCount(), GetPartCount()); // do not try to import to files without hashset.
		return false;
	}


	// Disallow very small files
	// Maybe I make them allowed in future when I make my program insert slices in partially downloaded parts.
	if (GetFileSize() < (uint64)EMBLOCKSIZE){
		LogError(LOG_STATUSBAR, GetResString(IDS_SR13_IMPORTPARTS_ERR_FILETOOSMALL)); // do not try to import to files without hashset.
		return false;
	}
	*/
	CFileDialog dlg(true, NULL, NULL, OFN_FILEMUSTEXIST|OFN_HIDEREADONLY);
	if(dlg.DoModal()!=IDOK)
		return false;
	CString pathName=dlg.GetPathName();

	CAddFileThread* addfilethread = (CAddFileThread*) AfxBeginThread(RUNTIME_CLASS(CAddFileThread), THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED);
	if (addfilethread){
		partfile->SetFileOpProgress(0);
		addfilethread->SetValues(theApp.sharedfiles, partfile->GetPath(), partfile->m_hpartfile.GetFileName(), _T(""), partfile);
		if (addfilethread->SetPartToImport(pathName))
			partfile->SetFileOp(PFOP_SR13_IMPORTPARTS);
		else
			partfile->SetFileOp(PFOP_HASHING);
		addfilethread->ResumeThread();
	}
	return true;
}

// Special case for SR13-ImportParts
uint16 CAddFileThread::SetPartToImport(LPCTSTR import)
{
	if (m_partfile->GetFilePath() == import) {
		return 0;
	}
	m_strImport = import;

	for (uint16 i = 0; i < m_partfile->GetPartCount(); i++)
		if (!m_partfile->IsComplete((uint64)i*PARTSIZE,(uint64)(i+1)*PARTSIZE-1,false)){
			m_PartsToImport.Add(i);
		}
	return (uint16)m_PartsToImport.GetSize();
}

bool CAddFileThread::SR13_ImportParts(){

	uint16 partsuccess=0;
	//	uint16 badpartsuccess=0;

	CFile f;
	if(!f.Open(m_strImport, CFile::modeRead |CFile::shareDenyNone   )){
		LogError(LOG_STATUSBAR, GetResString(IDS_SR13_IMPORTPARTS_ERR_CANTOPENFILE), m_strImport);
		return false;
	}

	uint64 fileSize=f.GetLength();
	uint32 partSize;
	uchar hash[16];
	CKnownFile *kfcall=new CKnownFile;

	CString strFilePath;
	_tmakepath(strFilePath.GetBuffer(MAX_PATH), NULL, m_strDirectory, m_strFilename, NULL);
	strFilePath.ReleaseBuffer();

	Log(LOG_STATUSBAR, GetResString(IDS_SR13_IMPORTPARTS_IMPORTSTART), m_PartsToImport.GetSize(), strFilePath);

	BYTE* partData=NULL;
	for (UINT i = 0; i < (UINT)m_PartsToImport.GetSize(); i++){
		uint16 partnumber = m_PartsToImport[i];
		if (PARTSIZE*partnumber > fileSize) {
			break;
		}
		try {
			try {
				if (partData==NULL ) 
					partData=new BYTE[PARTSIZE];
				CSingleLock sLock1(&(theApp.hashing_mut), TRUE);	//SafeHash - wait a current hashing process end before read the chunk
				f.Seek((LONGLONG)PARTSIZE*partnumber,0);
				partSize=f.Read(partData, PARTSIZE);
			} catch (...) {
				LogWarning(LOG_STATUSBAR, _T("Part %i: Not accessible (You may have to run scandisk to correct a bad cluster on your harddisk)."), partnumber);
				delete[] partData;	 partData = NULL;
				continue;
			}
			kfcall->CreateHash(partData, partSize, hash);
				ImportPart_Struct* importpart = new ImportPart_Struct;
				importpart->start = (uint64)partnumber*PARTSIZE;
				importpart->end = (uint64)partnumber*PARTSIZE+partSize-1;
				importpart->data = partData;
				VERIFY( PostMessage(theApp.emuledlg->m_hWnd,TM_IMPORTPART,(WPARAM)importpart,(LPARAM)m_partfile) );
				partData = NULL; // Delete will happen in async write thread.
				//Log(LOG_STATUSBAR, GetResString(IDS_SR13_IMPORTPARTS_PARTIMPORTEDGOOD), partnumber);
				partsuccess++;

			if (theApp.emuledlg->IsRunning()){
				UINT uProgress = (UINT)(i * 100 / m_PartsToImport.GetSize());
				VERIFY( PostMessage(theApp.emuledlg->GetSafeHwnd(), TM_FILEOPPROGRESS, uProgress, (LPARAM)m_partfile) );
				//zz_fly :: increase sleep time to 150ms
				/*
				Sleep(100); // sleep very short to give write time to write (or else mem grows!)
				*/
				Sleep(150); // sleep very short to give write time to write (or else mem grows!)

			}

			if(!theApp.emuledlg->IsRunning() || partSize!=PARTSIZE || m_partfile->GetFileOp() != PFOP_SR13_IMPORTPARTS) {
				break;
			}
		} catch (...) {
			if (partData != NULL)
			{delete[] partData;partData = NULL;}
			continue;
		}
	}
	if (partData) {delete[] partData;partData = NULL;}
	try {
		bool importaborted = m_partfile->GetFileOp() == PFOP_NONE || !theApp.emuledlg->IsRunning();
		if (m_partfile->GetFileOp() == PFOP_SR13_IMPORTPARTS)
			m_partfile->SetFileOp(PFOP_NONE);
		if (importaborted)
			Log(LOG_STATUSBAR, _T("Import aborted. %i parts imported to %s."), partsuccess, m_strFilename);
		else
			Log(LOG_STATUSBAR, _T("Import finished. %i parts imported to %s."), partsuccess, m_strFilename);
	} catch (...) {
		//This could happen if we delete the part instance
	}

	f.Close();
	delete kfcall;

	return true;
}