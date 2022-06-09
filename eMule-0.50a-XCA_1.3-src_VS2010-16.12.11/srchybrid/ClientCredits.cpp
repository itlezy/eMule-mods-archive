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
#include <math.h>
#include "emule.h"
#include "ClientCredits.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Opcodes.h"
#include "Sockets.h"
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
//Xman
#include <crypto/base64.h>
#include <crypto/osrng.h>
#include <crypto/files.h>
#include <crypto/sha.h>
//Xman end
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#include "emuledlg.h"
#include "Log.h"
//Xman
#include "updownclient.h"
#include "ClientList.h" //Xman Extened credit- table-arragement
#include "Statistics.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define CLIENTS_MET_FILENAME	_T("clients.met")

CClientCredits::CClientCredits(CreditStruct* in_credits)
{
	m_pCredits = in_credits;
	InitalizeIdent();
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
	m_dwWaitTimeIP = 0;
	m_bmarktodelete=false; //Xman Extened credit- table-arragement

}

CClientCredits::CClientCredits(const uchar* key, CreditStruct* in_credits)
{
	//m_pCredits = new CreditStruct;
	m_pCredits = in_credits;
	memset(m_pCredits, 0, sizeof(CreditStruct));
	md4cpy(m_pCredits->abyKey, key);
	InitalizeIdent();
	m_dwUnSecureWaitTime = //::GetTickCount();
	m_dwSecureWaitTime = ::GetTickCount();
	m_dwWaitTimeIP = 0;
	m_bmarktodelete=false; //Xman Extened credit- table-arragement

}

/*
CClientCredits::~CClientCredits()
{
	delete m_pCredits;
}
*/

void CClientCredits::AddDownloaded(uint32 bytes, uint32 dwForIP) {
	// ==> Code Optimization [SiRoB] - Stulle
	/*
	if ((GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable()) {
	*/
	if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
	// <== Code Optimization [SiRoB] - Stulle
		return;
	}

	//encode
	uint64 current = (((uint64)m_pCredits->nDownloadedHi << 32) | m_pCredits->nDownloadedLo) + bytes;

	//recode
	m_pCredits->nDownloadedLo = (uint32)current;
	m_pCredits->nDownloadedHi = (uint32)(current >> 32);
	TestPayBackFirstStatus(); // Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
}

void CClientCredits::AddUploaded(uint32 bytes, uint32 dwForIP) {
	// ==> Code Optimization [SiRoB] - Stulle
	/*
	if ((GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable()) {
	*/
	if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
	// <== Code Optimization [SiRoB] - Stulle
		return;
	}

	//encode
	uint64 current = (((uint64)m_pCredits->nUploadedHi << 32) | m_pCredits->nUploadedLo) + bytes;

	//recode
	m_pCredits->nUploadedLo = (uint32)current;
	m_pCredits->nUploadedHi = (uint32)(current >> 32);
	TestPayBackFirstStatus(); // Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
}

uint64 CClientCredits::GetUploadedTotal() const {
	return ((uint64)m_pCredits->nUploadedHi << 32) | m_pCredits->nUploadedLo;
}

uint64 CClientCredits::GetDownloadedTotal() const {
	return ((uint64)m_pCredits->nDownloadedHi << 32) | m_pCredits->nDownloadedLo;
}

float CClientCredits::GetScoreRatio(uint32 dwForIP) const
{
     // check the client ident status
	 // ==> Code Optimization [SiRoB] - Stulle
	 /*
	 if ((GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable()) {
	 */
	 if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && theApp.clientcredits->CryptoAvailable() ){
	 // <== Code Optimization [SiRoB] - Stulle
			// bad guy - no credits for you
			//return 1.0f;
			return 0.8f; //Xman 80% for non SUI-clients.. (and also bad guys)
		}

//Lovelace +
	        double result = 0.0;//leuk_he:double to prevent underflow in CS_LOVELACE.
			double cl_up,cl_down; 

			cl_up = GetUploadedTotal()/(double)1048576;
			cl_down = GetDownloadedTotal()/(double)1048576;
			result=(float)(3.0 * cl_down * cl_down - cl_up * cl_up);
			if (fabs(result)>20000.0f) 
				result*=20000.0/fabs(result);
			    result=100.0*pow((1-1/(1.0f+exp(result*0.001))),6.6667);
			if (result<0.1) 
				result=0.1;
			if (result>10.0 && IdentState == IS_NOTAVAILABLE)
				result=10.0;
            return (float)result;
/* 
	if (GetDownloadedTotal() < 1048576)
		return 1.0F;
	float result = 0.0F;
	if (!GetUploadedTotal())
		result = 10.0F;
	else
		result = (float)(((double)GetDownloadedTotal()*2.0)/(double)GetUploadedTotal());
	
	// exponential calcualtion of the max multiplicator based on uploaded data (9.2MB = 3.34, 100MB = 10.0)
	float result2 = 0.0F;
	result2 = (float)(GetDownloadedTotal()/1048576.0);
	result2 += 2.0F;
	result2 = (float)sqrt(result2);

	// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
	float result3 = 10.0F;
	if (GetDownloadedTotal() < 9646899){
		result3 = (((float)(GetDownloadedTotal() - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
	}

	// take the smallest result
	result = min(result, min(result2, result3));

	if (result < 1.0F)
		return 1.0F;
	else if (result > 10.0F)
		return 10.0F;
	return result;
*/
//Lovelace -
}

//Xman
// See own credits - VQB
const float CClientCredits::GetMyScoreRatio(uint32 dwForIP) const
{
	// check the client ident status
	// ==> Code Optimization [SiRoB] - Stulle
	/*
	if ( ( GetCurrentIdentState(dwForIP) == IS_IDFAILED || GetCurrentIdentState(dwForIP) == IS_IDBADGUY || GetCurrentIdentState(dwForIP) == IS_IDNEEDED) && theApp.clientcredits->CryptoAvailable() ){
	*/
	if ( GetCurrentIdentState(dwForIP) != IS_IDENTIFIED  && GetCurrentIdentState(dwForIP) != IS_NOTAVAILABLE && GetCurrentIdentState(dwForIP) != IS_IDBADGUY && theApp.clientcredits->CryptoAvailable() ){
	// <== Code Optimization [SiRoB] - Stulle
		// bad guy - no credits for... me?
		return 1.0f;
	}

	if (GetUploadedTotal() < 1048576)
		return 1.0f;
	float result = 0;
	if (!GetDownloadedTotal())
		result = 10.0f;
	else
		result = (float)(((double)GetUploadedTotal()*2.0)/(double)GetDownloadedTotal());
	float result2 = 0;
	result2 = (float)(GetUploadedTotal()/1048576.0);
	result2 += 2.0f;
	result2 = (float)sqrt(result2);

	// linear calcualtion of the max multiplicator based on uploaded data for the first chunk (1MB = 1.01, 9.2MB = 3.34)
	float result3 = 10.0F;
	if (GetUploadedTotal() < 9646899){
		result3 = (((float)(GetUploadedTotal() - 1048576) / 8598323.0F) * 2.34F) + 1.0F;
	}

	// take the smallest result
	result = min(result, min(result2, result3));

	if (result < 1.0f)
		return 1.0f;
	else if (result > 10.0f)
		return 10.0f;
	return result;
}
// See own credits - VQB END
//Xman end


CClientCreditsList::CClientCreditsList()
{
	m_nLastSaved = ::GetTickCount();
	LoadList();

	InitalizeCrypting();
}

CClientCreditsList::~CClientCreditsList()
{
	SaveList();
#ifdef REPLACE_ATLMAP
	for(CClientCreditMap::const_iterator it = m_mapClients.begin(); it != m_mapClients.end(); ++it)
	{
		ClientCreditContainer* cur_credit = it->second;
#else
	ClientCreditContainer* cur_credit;
	CCKey tmpkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos){
		m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);
#endif
		if(cur_credit->clientCredit)
			delete cur_credit->clientCredit;
		delete cur_credit;
	}
	delete m_pSignkey;
}

void CClientCreditsList::LoadList()
{
	CString strFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CLIENTS_MET_FILENAME;
	const int iOpenFlags = CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(strFileName, iOpenFlags, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			LogError(LOG_STATUSBAR, _T("%s"), strError);
		}
#ifdef REPLACE_ATLMAP
		CClientCreditMap(DEFAULT_CLIENTS_TABLE_SIZE).swap(m_mapClients);
#else
		m_mapClients.InitHashTable(DEFAULT_CLIENTS_TABLE_SIZE);// X: [CI] - [Code Improvement] Init-Hashtable optimization
#endif
		return;
	}

	ASSERT((CREDITFILE_VERSION & I64TIMEMASK) == 0);// X: [E64T] - [Enable64BitTime]
	ASSERT((CREDITFILE_VERSION_29 & I64TIMEMASK) == 0);

	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try{
		uint8 version = file.ReadUInt8();
		if (version != CREDITFILE_VERSION && version != CREDITFILE_VERSION_29 && version != (CREDITFILE_VERSION|I64TIMEMASK)){
			LogWarning(GetResString(IDS_ERR_CREDITFILEOLD));
			file.Close();
#ifdef REPLACE_ATLMAP
			CClientCreditMap(DEFAULT_CLIENTS_TABLE_SIZE).swap(m_mapClients);
#else
			m_mapClients.InitHashTable(DEFAULT_CLIENTS_TABLE_SIZE);// X: [CI] - [Code Improvement] Init-Hashtable optimization
#endif
			return;
		}

		// everything is ok, lets see if the backup exist...
		CString strBakFileName;
		strBakFileName.Format(_T("%s") CLIENTS_MET_FILENAME _T(".bak"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));

		DWORD dwBakFileSize = 0;
		BOOL bCreateBackup = TRUE;

		HANDLE hBakFile = ::CreateFile(strBakFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hBakFile != INVALID_HANDLE_VALUE)
		{
			// Ok, the backup exist, get the size
			dwBakFileSize = ::GetFileSize(hBakFile, NULL); //debug
			if (dwBakFileSize > (DWORD)file.GetLength())
			{
				// the size of the backup was larger then the org. file, something is wrong here, don't overwrite old backup..
				bCreateBackup = FALSE;
			}
			//else: backup is smaller or the same size as org. file, proceed with copying of file
			::CloseHandle(hBakFile);
		}
		//else: the backup doesn't exist, create it

		if (bCreateBackup ) 
		{
			file.Close(); // close the file before copying

			//Xman don't overwrite bak files if last sessions crashed
			if(thePrefs.eMuleCrashedLastSession())
				::CopyFile(strFileName, strBakFileName, TRUE); //allow one copy
			else
			//Xman end
			if (!::CopyFile(strFileName, strBakFileName, FALSE))
				LogError(GetResString(IDS_ERR_MAKEBAKCREDITFILE));

			// reopen file
			CFileException fexp;
			if (!file.Open(strFileName, iOpenFlags, &fexp)){
				CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
					strError += _T(" - ");
					strError += szError;
				}
				LogError(LOG_STATUSBAR, _T("%s"), strError);
#ifdef REPLACE_ATLMAP
				CClientCreditMap(DEFAULT_CLIENTS_TABLE_SIZE).swap(m_mapClients);
#else
				m_mapClients.InitHashTable(DEFAULT_CLIENTS_TABLE_SIZE);// X: [CI] - [Code Improvement] Init-Hashtable optimization
#endif
				return;
			}
			setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
			file.Seek(1, CFile::begin); //set filepointer behind file version byte
		}

		UINT count = file.ReadUInt32();
#ifdef REPLACE_ATLMAP
		CClientCreditMap(GetPrime(((count + 13000)*12+9))/10).swap(m_mapClients);
#else
		//Xman Extened credit- table-arragement
		//UINT calc=UINT(count*1.2f);
		//zz_fly :: prime table :: start
		/*
		calc = calc + calc%2 + 1;
		m_mapClients.InitHashTable(calc + 20000); //optimized for 20 000 new contacts
		*/
		//m_mapClients.InitHashTable(GetPrime(calc + 19000)); //GetPrime will increase the number about 1k in average
		//zz_fly :: prime table :: end
		m_mapClients.InitHashTable(GetPrime(((count + 13000)*12+9))/10); //optimized for 13 000 new contacts

		//m_mapClients.InitHashTable(count+5000); // TODO: should be prime number... and 20% larger
		//Xman end
#endif

		const uint64 dwExpired = time(NULL) - 12960000; // today - 150 day
		uint32 cDeleted = 0;
		for (UINT i = 0; i < count; i++){
			ClientCreditContainer* newcstruct_parent = new ClientCreditContainer;
			CreditStruct* newcstruct = &newcstruct_parent->credit;
			memset(newcstruct, 0, sizeof(CreditStruct));
			if (version == CREDITFILE_VERSION_29){
				file.Read(newcstruct, 28);
				file.Read((BYTE*)newcstruct+32, 10);
			}
			else if (version == CREDITFILE_VERSION){
				file.Read(newcstruct, 28);
				file.Read((BYTE*)newcstruct+32, 91);
			}
			else
				file.Read(newcstruct, sizeof(CreditStruct));
			if (newcstruct->nLastSeen < dwExpired){
				cDeleted++;
				delete newcstruct_parent;
				continue;
			}
			newcstruct_parent->clientCredit = NULL;

#ifdef REPLACE_ATLMAP
			m_mapClients[newcstruct->abyKey] = newcstruct_parent;
#else
			//CClientCredits* newcredits = new CClientCredits(newcstruct);
			//m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
			m_mapClients.SetAt(CCKey(newcstruct->abyKey), newcstruct_parent);
#endif
		}
		file.Close();

		if (cDeleted>0)
			AddLogLine(false, GetResString(IDS_CREDITFILELOADED) + GetResString(IDS_CREDITSEXPIRED), count-cDeleted,cDeleted);
		else
			AddLogLine(false, GetResString(IDS_CREDITFILELOADED), count);
	}
	catch(CFileException* error){
		if (error->m_cause == CFileException::endOfFile)
			LogError(LOG_STATUSBAR, GetResString(IDS_CREDITFILECORRUPT));
		else{
			TCHAR buffer[MAX_CFEXP_ERRORMSG];
			error->GetErrorMessage(buffer, ARRSIZE(buffer));
			LogError(LOG_STATUSBAR, GetResString(IDS_ERR_CREDITFILEREAD), buffer);
		}
		error->Delete();
#ifdef REPLACE_ATLMAP
		if(m_mapClients.bucket_count()<DEFAULT_CLIENTS_TABLE_SIZE)
			CClientCreditMap(DEFAULT_CLIENTS_TABLE_SIZE).swap(m_mapClients);
#else
		if(m_mapClients.GetHashTableSize()<DEFAULT_CLIENTS_TABLE_SIZE)// X: [CI] - [Code Improvement] Init-Hashtable optimization
			m_mapClients.InitHashTable(DEFAULT_CLIENTS_TABLE_SIZE);
#endif
	}
}

void CClientCreditsList::SaveList()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, _T("Saving clients credit list file \"%s\""), CLIENTS_MET_FILENAME);
	m_nLastSaved = ::GetTickCount();

	CString name = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + CLIENTS_MET_FILENAME;
	CFile file;// no buffering needed here since we swap out the entire array
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp)){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		return;
	}

	bool I64Time=thePrefs.m_bEnable64BitTime;// X: [E64T] - [Enable64BitTime]
	size_t szCreditStruct=I64Time?sizeof(CreditStruct):119;// X: [E64T] - [Enable64BitTime]
#ifdef REPLACE_ATLMAP
	size_t count = m_mapClients.size();
	BYTE* pBuffer = new BYTE[count*szCreditStruct];
	count = 0;
	for(CClientCreditMap::const_iterator it = m_mapClients.begin(); it != m_mapClients.end(); ++it)
	{
		ClientCreditContainer* cur_credit = it->second;
#else
	UINT_PTR count = (UINT_PTR)m_mapClients.GetCount();
	BYTE* pBuffer = new BYTE[count*szCreditStruct];
	count = 0;
	ClientCreditContainer* cur_credit;
	CCKey tempkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos)
	{
		m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
#endif
		CreditStruct* credit = &cur_credit->credit;

		if (credit->nUploadedHi || credit->nUploadedLo/*cur_credit->GetUploadedTotal()*/
			|| credit->nDownloadedHi || credit->nDownloadedLo/*cur_credit->GetDownloadedTotal()*/)
		{
			if(I64Time)// X: [E64T] - [Enable64BitTime]
				memcpy(pBuffer+(count*szCreditStruct), credit/*cur_credit->GetDataStruct()*/, szCreditStruct);
			else{
				memcpy(pBuffer+(count*szCreditStruct), credit/*cur_credit->GetDataStruct()*/, 28);
				memcpy(pBuffer+(count*szCreditStruct)+28, (BYTE*)(credit/*cur_credit->GetDataStruct()*/)+32, 91);
			}
			count++; 
		}
	}

	try{
		uint8 version = I64Time?(CREDITFILE_VERSION|I64TIMEMASK):CREDITFILE_VERSION;// X: [E64T] - [Enable64BitTime]
		file.Write(&version, 1);
		file.Write(&count, 4);
		file.Write(pBuffer, count*szCreditStruct);
		if (thePrefs.GetCommitFiles() >= 2 || (thePrefs.GetCommitFiles() >= 1 && !CemuleDlg::IsRunning()))
			file.Flush();
		file.Close();
	}
	catch(CFileException* error){
		CString strError(GetResString(IDS_ERR_FAILED_CREDITSAVE));
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, ARRSIZE(szError))){
			strError += _T(" - ");
			strError += szError;
		}
		LogError(LOG_STATUSBAR, _T("%s"), strError);
		error->Delete();
	}

	delete[] pBuffer;
}

CClientCredits* CClientCreditsList::GetCredit(const uchar* key)
{
	ClientCreditContainer* result;
#ifdef REPLACE_ATLMAP
	CClientCreditMap::const_iterator it = m_mapClients.find(key);
	if(it == m_mapClients.end()){
		result = new ClientCreditContainer;
		result->clientCredit = new CClientCredits(key, &result->credit);
		m_mapClients[result->clientCredit->GetKey()] = result;
	}
	else{
		result = it->second;
#else
	CCKey tkey(key);
	if (!m_mapClients.Lookup(tkey, result)){
		result = new ClientCreditContainer;
		result->clientCredit = new CClientCredits(key, &result->credit);
		m_mapClients.SetAt(CCKey(result->clientCredit->GetKey()), result);
	}
	else
	{
#endif
		if(result->clientCredit)
			result->clientCredit->UnMarkToDelete(); //Xman Extened credit- table-arragement
		else
			result->clientCredit = new CClientCredits(&result->credit);
	}
	result->clientCredit->SetLastSeen();

	return result->clientCredit;
}

void CClientCreditsList::Process()
{
#ifdef _DEBUG
#define HOURS_KEEP_IN_MEMORY 1	//Xman Extened credit- table-arragement
#else
#define HOURS_KEEP_IN_MEMORY 4	//Xman Extened credit- table-arragement
#endif
	if (::GetTickCount() - m_nLastSaved > MIN2MS(13))
	{
		if(GetTickCount() - theStats.starttime > HR2MS(HOURS_KEEP_IN_MEMORY)){
			//Xman Extened credit- table-arragement
			uint64 tmNow = time(NULL);
			uint_ptr credit_count = 0, delkey_count = 0, delcredit_count = 0;
#ifdef REPLACE_ATLMAP
			for(CClientCreditMap::const_iterator it = m_mapClients.begin(); it != m_mapClients.end();)
			{
				ClientCreditContainer* result = it->second;
#else
			ClientCreditContainer* result;
			CCKey tmpkey(0);
			POSITION pos = m_mapClients.GetStartPosition();
			while (pos){
				m_mapClients.GetNextAssoc(pos, tmpkey, result);
#endif
				CClientCredits* cur_credit = result->clientCredit;
				if(cur_credit){
					if(cur_credit->GetMarkToDelete() && (tmNow - cur_credit->GetLastSeen() > HR2S(HOURS_KEEP_IN_MEMORY))) //not seen for > 4 hours
					{
						//two security-checks, it can happen that there is a second user using this hash
						if(cur_credit->GetUploadedTotal()==0 && cur_credit->GetDownloadedTotal()==0
							&& theApp.clientlist->FindClientByUserHash(cur_credit->GetKey())==NULL)
						{
#ifdef REPLACE_ATLMAP
							it = m_mapClients.erase(it);
							delete cur_credit;
							delete result;
							++delkey_count;
#else
							//this key isn't longer used
							m_mapClients.RemoveKey(CCKey(cur_credit->GetKey()));
							delete cur_credit;
							delete result;
							++delkey_count;
#endif
							continue;
						}
						if(tmNow - cur_credit->GetLastSeen() > HR2S(HOURS_KEEP_IN_MEMORY*2) //not seen for > 8 hours
							&& theApp.clientlist->FindClientByUserHash(cur_credit->GetKey())==NULL)
						{
							//this credit isn't longer used
							result->clientCredit = NULL;
							delete cur_credit;
							++delcredit_count;
						}
						//else
							//cur_credit->UnMarkToDelete();
					}
					++credit_count;
				}
#ifdef REPLACE_ATLMAP
				++it;
#endif
			}
			//Xman end
			AddDebugLogLine( false, _T("Cleaned ClientCreditsList, entries: %i, removed %i not used keys, %i not used credits"), credit_count - delkey_count - delcredit_count, delkey_count, delcredit_count);
		}
		SaveList();
	}
}

#ifdef _DEBUG
void	CClientCreditsList::PrintStatistic()
{
	AddLogLine(false,_T("used Credit-Objects: %u"), m_mapClients.GetSize());
}
#endif

void CClientCredits::InitalizeIdent()
{
	if (m_pCredits->nKeySize == 0 ){
		/*
		memset(m_abyPublicKey,0,80); // for debugging
		*/	//Enig123::ACAT optimization
		m_nPublicKeyLen = 0;
		IdentState = IS_NOTAVAILABLE;
	}
	else{
		m_nPublicKeyLen = m_pCredits->nKeySize;
		// ==> Catch oversized public key in credit.met file [SiRoB] - Stulle
		if (m_nPublicKeyLen > MAXPUBKEYSIZE)
			throw CString(_T("Public Key of one client is larger than MAXPUBKEYSIZE"));
		// <== Catch oversized public key in credit.met file [SiRoB] - Stulle
		/*
		memcpy(m_abyPublicKey, m_pCredits->abySecureIdent, m_nPublicKeyLen);
		*/	//Enig123::ACAT optimization
		IdentState = IS_IDNEEDED;
	}
	m_dwCryptRndChallengeFor = 0;
	m_dwCryptRndChallengeFrom = 0;
	m_dwIdentIP = 0;
}

void CClientCredits::Verified(uint32 dwForIP)
{
	m_dwIdentIP = dwForIP;
	// client was verified, copy the keyto store him if not done already
	if (m_pCredits->nKeySize == 0){
		m_pCredits->nKeySize = m_nPublicKeyLen;
		/*
		memcpy(m_pCredits->abySecureIdent, m_abyPublicKey, m_nPublicKeyLen);
		*/	//Enig123::ACAT optimization
		if (GetDownloadedTotal() > 0){
			// for security reason, we have to delete all prior credits here
			m_pCredits->nDownloadedHi = 0;
			m_pCredits->nDownloadedLo = 1;
			m_pCredits->nUploadedHi = 0;
			m_pCredits->nUploadedLo = 1; // in order to safe this client, set 1 byte
			if (thePrefs.GetVerbose())
				DEBUG_ONLY(AddDebugLogLine(false, _T("Credits deleted due to new SecureIdent")));
		}
	}
	IdentState = IS_IDENTIFIED;
}

bool CClientCredits::SetSecureIdent(const uchar* pachIdent, uint8 nIdentLen)  // verified Public key cannot change, use only if there is not public key yet
{
	if (MAXPUBKEYSIZE < nIdentLen || m_pCredits->nKeySize != 0 )
		return false;
	/*
	memcpy(m_abyPublicKey,pachIdent, nIdentLen);
	*/	//Enig123::ACAT optimization
	memcpy(m_pCredits->abySecureIdent, pachIdent, nIdentLen);

	m_nPublicKeyLen = nIdentLen;
	IdentState = IS_IDNEEDED;
	return true;
}

EIdentState	CClientCredits::GetCurrentIdentState(uint32 dwForIP) const
{
	if (IdentState != IS_IDENTIFIED)
		return IdentState;
	else{
		if (dwForIP == m_dwIdentIP)
			return IS_IDENTIFIED;
		else
			return IS_IDBADGUY; 
		// mod note: clients which just reconnected after an IP change and have to ident yet will also have this state for 1-2 seconds
		//		 so don't try to spam such clients with "bad guy" messages (besides: spam messages are always bad)
	}
}

using namespace CryptoPP;

void CClientCreditsList::InitalizeCrypting()
{
	m_nMyPublicKeyLen = 0;
	memset(m_abyMyPublicKey,0,80); // not really needed; better for debugging tho
	m_pSignkey = NULL;

#ifndef CLIENTANALYZER
	if (!thePrefs.IsSecureIdentEnabled())
		return;
#endif

	// check if keyfile is there
	bool bCreateNewKey = false;
	HANDLE hKeyFile = ::CreateFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("cryptkey.dat"), GENERIC_READ, FILE_SHARE_READ, NULL,
		OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hKeyFile != INVALID_HANDLE_VALUE)
	{
		if (::GetFileSize(hKeyFile, NULL) == 0)
			bCreateNewKey = true;
		::CloseHandle(hKeyFile);
	}
	else
		bCreateNewKey = true;
	if (bCreateNewKey)
		CreateKeyPair();

	// load key
	try{
		// load private key
		FileSource filesource(CStringA(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("cryptkey.dat")), true,new Base64Decoder);
		m_pSignkey = new RSASSA_PKCS1v15_SHA_Signer(filesource);
		// calculate and store public key
		RSASSA_PKCS1v15_SHA_Verifier pubkey(*m_pSignkey);
		ArraySink asink(m_abyMyPublicKey, 80);
		pubkey.DEREncode(asink);
		m_nMyPublicKeyLen = (uint8)asink.TotalPutLength();
		asink.MessageEnd();
	}
	catch(...)
	{
		delete m_pSignkey;
		m_pSignkey = NULL;
		LogError(LOG_STATUSBAR, GetResString(IDS_CRYPT_INITFAILED));
		ASSERT(0);
	}
	ASSERT( Debug_CheckCrypting() );
}

bool CClientCreditsList::CreateKeyPair()
{
	try{
		AutoSeededRandomPool rng;
		InvertibleRSAFunction privkey;
		privkey.Initialize(rng,RSAKEYSIZE);

		Base64Encoder privkeysink(new FileSink(CStringA(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("cryptkey.dat"))));
		privkey.DEREncode(privkeysink);
		privkeysink.MessageEnd();

		if (thePrefs.GetLogSecureIdent())
			AddDebugLogLine(false, _T("Created new RSA keypair"));
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Failed to create new RSA keypair"));
		ASSERT ( false );
		return false;
	}
	return true;
}

uint8 CClientCreditsList::CreateSignature(CClientCredits* pTarget, uchar* pachOutput, uint8 nMaxSize, 
										  uint32 ChallengeIP, uint8 byChaIPKind, 
										  CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey)
{
	// sigkey param is used for debug only
	if (sigkey == NULL)
		sigkey = m_pSignkey;

	// create a signature of the public key from pTarget
	ASSERT( pTarget );
	ASSERT( pachOutput );
	uint8 nResult;
	if ( !CryptoAvailable() )
		return 0;
	try{

		SecByteBlock sbbSignature(sigkey->SignatureLength());
		AutoSeededRandomPool rng;
		byte abyBuffer[MAXPUBKEYSIZE+9];
		uint32 keylen = pTarget->GetSecIDKeyLen();
		memcpy(abyBuffer,pTarget->GetSecureIdent(),keylen);
		// 4 additional bytes random data send from this client
		uint32 challenge = pTarget->m_dwCryptRndChallengeFrom;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+keylen, challenge);
		uint16 ChIpLen = 0;
		if ( byChaIPKind != 0){
			ChIpLen = 5;
			PokeUInt32(abyBuffer+keylen+4, ChallengeIP);
			PokeUInt8(abyBuffer+keylen+4+4, byChaIPKind);
		}
		sigkey->SignMessage(rng, abyBuffer ,keylen+4+ChIpLen , sbbSignature.begin());
		ArraySink asink(pachOutput, nMaxSize);
		asink.Put(sbbSignature.begin(), sbbSignature.size());
		nResult = (uint8)asink.TotalPutLength();			
	}
	catch(...)
	{
		ASSERT ( false );
		nResult = 0;
	}
	return nResult;
}

bool CClientCreditsList::VerifyIdent(CClientCredits* pTarget, const uchar* pachSignature, uint8 nInputSize, 
									 uint32 dwForIP, uint8 byChaIPKind)
{
	ASSERT( pTarget );
	ASSERT( pachSignature );
	if ( !CryptoAvailable() ){
		pTarget->IdentState = IS_NOTAVAILABLE;
		return false;
	}
	bool bResult;
	try{
		StringSource ss_Pubkey((byte*)pTarget->GetSecureIdent(),pTarget->GetSecIDKeyLen(),true,0);
		RSASSA_PKCS1v15_SHA_Verifier pubkey(ss_Pubkey);
		// 4 additional bytes random data send from this client +5 bytes v2
		byte abyBuffer[MAXPUBKEYSIZE+9];
		memcpy(abyBuffer,m_abyMyPublicKey,m_nMyPublicKeyLen);
		uint32 challenge = pTarget->m_dwCryptRndChallengeFor;
		ASSERT ( challenge != 0 );
		PokeUInt32(abyBuffer+m_nMyPublicKeyLen, challenge);

		// v2 security improvments (not supported by 29b, not used as default by 29c)
		uint8 nChIpSize = 0;
		if (byChaIPKind != 0){
			nChIpSize = 5;
			uint32 ChallengeIP = 0;
			switch (byChaIPKind){
				case CRYPT_CIP_LOCALCLIENT:
					ChallengeIP = dwForIP;
					break;
				case CRYPT_CIP_REMOTECLIENT:
					if (theApp.serverconnect->GetClientID() == 0 || theApp.serverconnect->IsLowID()){
						if (thePrefs.GetLogSecureIdent())
							AddDebugLogLine(false, _T("Warning: Maybe SecureHash Ident fails because LocalIP is unknown"));
						ChallengeIP = theApp.serverconnect->GetLocalIP();
					}
					else
						ChallengeIP = theApp.serverconnect->GetClientID();
					break;
				case CRYPT_CIP_NONECLIENT: // maybe not supported in future versions
					ChallengeIP = 0;
					break;
			}
			PokeUInt32(abyBuffer+m_nMyPublicKeyLen+4, ChallengeIP);
			PokeUInt8(abyBuffer+m_nMyPublicKeyLen+4+4, byChaIPKind);
		}
		//v2 end

		bResult = pubkey.VerifyMessage(abyBuffer, m_nMyPublicKeyLen+4+nChIpSize, pachSignature, nInputSize);
	}
	catch(...)
	{
		if (thePrefs.GetVerbose())
			AddDebugLogLine(false, _T("Error: Unknown exception in %hs"), __FUNCTION__);
		//ASSERT(0);
		bResult = false;
	}
	if (!bResult){
		if (pTarget->IdentState == IS_IDNEEDED)
			pTarget->IdentState = IS_IDFAILED;
	}
	else{
		pTarget->Verified(dwForIP);
	}
	return bResult;
}

bool CClientCreditsList::CryptoAvailable()
{
#ifdef CLIENTANALYZER
	return (m_nMyPublicKeyLen > 0 && m_pSignkey != 0 );
#else
	return (m_nMyPublicKeyLen > 0 && m_pSignkey != 0 && thePrefs.IsSecureIdentEnabled() );
#endif
}


#ifdef _DEBUG
bool CClientCreditsList::Debug_CheckCrypting()
{
	// create random key
	AutoSeededRandomPool rng;

	RSASSA_PKCS1v15_SHA_Signer priv(rng, 384);
	RSASSA_PKCS1v15_SHA_Verifier pub(priv);

	byte abyPublicKey[80];
	ArraySink asink(abyPublicKey, 80);
	pub.DEREncode(asink);
	uint8 PublicKeyLen = (uint8)asink.TotalPutLength();
	asink.MessageEnd();
	uint32 challenge = t_rng->getUInt32();
	// create fake client which pretends to be this emule
	CreditStruct* newcstruct = new CreditStruct;
	memset(newcstruct, 0, sizeof(CreditStruct));
	CClientCredits* newcredits = new CClientCredits(newcstruct);
	newcredits->SetSecureIdent(m_abyMyPublicKey,m_nMyPublicKeyLen);
	newcredits->m_dwCryptRndChallengeFrom = challenge;
	// create signature with fake priv key
	uchar pachSignature[200];
	memset(pachSignature,0,200); // Avi-3k: fixed args order
	uint8 sigsize = CreateSignature(newcredits,pachSignature,200,0,false, &priv);


	// next fake client uses the random created public key
	CreditStruct* newcstruct2 = new CreditStruct;
	memset(newcstruct2, 0, sizeof(CreditStruct));
	CClientCredits* newcredits2 = new CClientCredits(newcstruct2);
	newcredits2->m_dwCryptRndChallengeFor = challenge;

	// if you uncomment one of the following lines the check has to fail
	//abyPublicKey[5] = 34;
	//m_abyMyPublicKey[5] = 22;
	//pachSignature[5] = 232;

	newcredits2->SetSecureIdent(abyPublicKey,PublicKeyLen);

	//now verify this signature - if it's true everything is fine
	bool bResult = VerifyIdent(newcredits2,pachSignature,sigsize,0,0);

	delete newcredits;
	delete newcstruct;
	delete newcredits2;
	delete newcstruct2;

	return bResult;
}
#endif
uint32 CClientCredits::GetSecureWaitStartTime(uint32 dwForIP)
{
	if (m_dwUnSecureWaitTime == 0 || m_dwSecureWaitTime == 0)
		SetSecWaitStartTime(dwForIP);

	if (m_pCredits->nKeySize != 0){	// this client is a SecureHash Client
		if (GetCurrentIdentState(dwForIP) == IS_IDENTIFIED){ // good boy
			return m_dwSecureWaitTime;
		}
		else{	// not so good boy
			if (dwForIP == m_dwWaitTimeIP){
				return m_dwUnSecureWaitTime;
			}
			else{	// bad boy
				// this can also happen if the client has not identified himself yet, but will do later - so maybe he is not a bad boy :) .
				CString buffer2, buffer;
				/*for (uint16 i = 0;i != 16;i++){
					buffer2.Format("%02X",this->m_pCredits->abyKey[i]);
					buffer+=buffer2;
				}
				if (thePrefs.GetLogSecureIdent())
					AddDebugLogLine(false,"Warning: WaitTime resetted due to Invalid Ident for Userhash %s",buffer);*/

				m_dwUnSecureWaitTime = ::GetTickCount();
				m_dwWaitTimeIP = dwForIP;
				return m_dwUnSecureWaitTime;
			}	
		}
	}
	else{	// not a SecureHash Client - handle it like before for now (no security checks)
		return m_dwUnSecureWaitTime;
	}
}

void CClientCredits::SetSecWaitStartTime(uint32 dwForIP)
{
	m_dwUnSecureWaitTime = //::GetTickCount()-1;
	m_dwSecureWaitTime = ::GetTickCount()-1;
	m_dwWaitTimeIP = dwForIP;
}

void CClientCredits::ClearWaitStartTime()
{
	m_dwUnSecureWaitTime = 0;
	m_dwSecureWaitTime = 0;
}

//Xman Xtreme Full Chunk
void CClientCredits::SetWaitStartTimeBonus(uint32 dwForIP, uint32 timestamp)
{
	m_dwUnSecureWaitTime = timestamp-1;
	m_dwSecureWaitTime = timestamp-1;
	m_dwWaitTimeIP = dwForIP;
}
//Xman end

//zz_fly :: prime table :: start
UINT CClientCreditsList::GetPrime(UINT calc) const
{
	//prime table from primes.utm.edu
	//Table1 300k+, 20k per prime
	static const UINT primeTable1[] = {
			320009, 340007, 360007, 380041, 400009,	420001, 440009, 460013, 480013, 500009, 
			520019, 540041, 560017, 580001,	600011, 620003, 640007, 660001, 680003, 700001, 
			720007, 740011, 760007,	780029, 800011, 820037, 840023, 860009, 880001, 900001, 
			920011, 940001,	960017, 980027, 1000003};
	//Table2 100k~300k, 5k per prime
	static const UINT primeTable2[] = {105019, 110017, 115001, 120011,
			125003,	130003, 135007, 140009, 145007, 150001, 155003, 160001,	165001, 170003,	
			175003, 180001, 185021, 190027, 195023, 200003, 205019, 210011, 215051,	220009,
			225023, 230003, 235003, 240007, 245023, 250007, 255007, 260003,	265003, 270001,
			275003, 280001, 285007, 290011, 295007, 300007};
	//Table3 16k~100k, 2k per prime
	static const UINT primeTable3[] = {16001, 18013, 20011,
			22003, 24001, 26003, 28001, 30011, 32003, 34019, 36007, 38011, 40009,
			42013, 44017, 46021, 48017, 50021, 52009, 54001, 56003, 58013, 60013,
			62003, 64007, 66029, 68023, 70001, 72019, 74017, 76001, 78007, 80021,
			82003, 84011, 86011, 88001, 90001, 92003, 94007, 96001, 98009, 100003};
	if(calc>1000000)//1M contacts? let them eat cake...
		return (calc + calc%2 + 1);
	if(calc>300000)
		return primeTable1[(calc-300000)/20000];
	if(calc>100000)
		return primeTable2[(calc-100000)/5000];
	if(calc>16001)
		return primeTable3[(calc-16000)/2000];
	return 16001;
}
//zz_fly :: prime table :: end

// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
//init will be triggered at 
//1. client credit create, 
//2. when reach 10MB Transferred, between first time remove check and second time remove check
//anyway, this just make a check at "check point" :p

void CClientCredits::InitPayBackFirstStatus(){
	//m_bPayBackFirst2 = false;
	m_bPayBackFirst = false;
	TestPayBackFirstStatus();
}

//test will be triggered at client have up/down Transferred
void CClientCredits::TestPayBackFirstStatus(){

	uint64 clientUpload = GetDownloadedTotal();
	uint64 clientDownload = GetUploadedTotal();
	if(clientUpload > clientDownload+((uint64)thePrefs.GetPayBackFirstLimit()<<20)){
		m_bPayBackFirst = true;
	}
	else if(clientUpload < clientDownload){
		m_bPayBackFirst = false;
	}
/*
	if(clientUpload > clientDownload+((uint64)thePrefs.GetPayBackFirstLimit2()<<20))
		m_bPayBackFirst2 = true;
	else if(clientUpload < clientDownload)
		m_bPayBackFirst2 = false;
*/
}
// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle