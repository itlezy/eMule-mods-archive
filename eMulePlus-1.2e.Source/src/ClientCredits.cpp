//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "ClientCredits.h"
#include "opcodes.h"
#include "otherfunctions.h"
#include "emule.h"
#include "SafeFile.h"
#pragma warning(push, 3)	// preserve current state, then set warning level 3
#include "crypto51/base64.h"
#include "crypto51/osrng.h"
#include "crypto51/files.h"
#include "crypto51/sha.h"
#pragma warning(pop)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

#define CLIENTS_MET_FILENAME	_T("clients.met")

CClientCredits::CClientCredits(const CreditStruct *pInCredits)
{
	m_Credits = *pInCredits;
	InitalizeIdent();
}

CClientCredits::CClientCredits(const uchar* key)
{
	memzero(&m_Credits, sizeof(CreditStruct));
	md4cpy(m_Credits.abyKey, key);
	InitalizeIdent();
}

CClientCredits::~CClientCredits()
{
}

void CClientCredits::AddDownloaded(uint32 dwBytes, uint32 dwForIP)
{
	EIdentState		eIdentState = GetCurrentIdentState(dwForIP);

	if ( ( (eIdentState != IS_IDFAILED) && (eIdentState != IS_IDBADGUY) &&
		(eIdentState != IS_IDNEEDED) ) || !g_App.m_pClientCreditList->CryptoAvailable() )
	{
		uint32		dwLow = m_Credits.nDownloadedLo;

		m_Credits.nDownloadedLo += dwBytes;
		if (m_Credits.nDownloadedLo < dwLow)
			m_Credits.nDownloadedHi++;
	}
}

void CClientCredits::AddUploaded(uint32 dwBytes, uint32 dwForIP)
{
	EIdentState		eIdentState = GetCurrentIdentState(dwForIP);

	if ( ( (eIdentState != IS_IDFAILED) && (eIdentState != IS_IDBADGUY) &&
		(eIdentState != IS_IDNEEDED) ) || !g_App.m_pClientCreditList->CryptoAvailable() )
	{
		uint32		dwLow = m_Credits.nUploadedLo;

		m_Credits.nUploadedLo += dwBytes;
		if (m_Credits.nUploadedLo < dwLow)
			m_Credits.nUploadedHi++;
	}
}

uint64 CClientCredits::GetUploadedTotal() const
{
	return ((uint64)m_Credits.nUploadedHi << 32) + m_Credits.nUploadedLo;
}

uint64 CClientCredits::GetDownloadedTotal() const
{
	return ((uint64)m_Credits.nDownloadedHi << 32) + m_Credits.nDownloadedLo;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
double CClientCredits::GetScoreRatio(uint32 dwForIP, bool bInverted/*=false*/) const
{
	EMULE_TRY

	EIdentState	eIdentState = GetCurrentIdentState(dwForIP);

	if ( (eIdentState == IS_IDFAILED || eIdentState == IS_IDBADGUY ||
		 eIdentState == IS_IDNEEDED) && g_App.m_pClientCreditList->CryptoAvailable() )
	{
		return 1.0;	//	Bad guy - no credits for you
	}

//	Added Inversion to calculate the clients own ratio at the remote client from its own data
	uint64	qwDownTotal;
	uint32	dwUpHi, dwUpLo;

	if (bInverted)
	{
	//	(qwDownTotal < 1048576)
		if ((m_Credits.nUploadedLo < 1024*1024) && (m_Credits.nUploadedHi == 0))
			return 1.0;
		qwDownTotal = GetUploadedTotal();
		dwUpHi = m_Credits.nDownloadedHi;
		dwUpLo = m_Credits.nDownloadedLo;
	}
	else
	{
	//	(qwDownTotal < 1048576)
		if ((m_Credits.nDownloadedLo < 1024*1024) && (m_Credits.nDownloadedHi == 0))
			return 1.0;
		qwDownTotal = GetDownloadedTotal();
		dwUpHi = m_Credits.nUploadedHi;
		dwUpLo = m_Credits.nUploadedLo;
	}

	double	dResult, dResult2, dDownTotal = static_cast<double>(qwDownTotal);

	dResult2 = sqrt(dDownTotal / 1048576.0 + 2.0);	// min value ~1.7321...
	if ((dwUpHi | dwUpLo) == 0)
		dResult = dResult2;
	else
	{
		dResult = dDownTotal * 2.0 / static_cast<double>(((uint64)dwUpHi << 32) + dwUpLo);
		if (dResult > dResult2)
			dResult = dResult2;
		else if (dResult < 1.0)
			dResult = 1.0;
	}

	if (dResult > 10.0)
		dResult = 10.0;

	return dResult;

	EMULE_CATCH

	return 1.0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	HasHigherScoreRatio - returns true if score > 1.0
bool CClientCredits::HasHigherScoreRatio(uint32 dwForIP) const
{
	if (this == NULL)
		return false;

	if ((m_Credits.nDownloadedLo < 1000000) && (m_Credits.nDownloadedHi == 0))	// (qwDownTotal < 1000000)
		return false;

	EIdentState	eIdentState = GetCurrentIdentState(dwForIP);

	if ( (eIdentState == IS_IDFAILED || eIdentState == IS_IDBADGUY ||
		 eIdentState == IS_IDNEEDED) && g_App.m_pClientCreditList->CryptoAvailable() )
	{
		return false;	//	Bad guy - no credits for you
	}

	if ((m_Credits.nUploadedHi | m_Credits.nUploadedLo) != 0)
	{
		uint32	dwUpHi = m_Credits.nUploadedHi >> 1u;
		uint32	dwUpLo = m_Credits.nUploadedLo >> 1u;

	//	(2 * GetDownloadedTotal() <= GetUploadedTotal())
		if ( (m_Credits.nDownloadedHi < dwUpHi) ||
			((m_Credits.nDownloadedHi == dwUpHi) && (m_Credits.nDownloadedLo <= dwUpLo)) )
		{
			return false;
		}
	}
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CClientCreditsList::CClientCreditsList()
{
	m_nLastSaved = ::GetTickCount();
	LoadList();

	InitalizeCrypting();
}

CClientCreditsList::~CClientCreditsList()
{
	SaveList();
	CClientCredits* cur_credit;
	CCKey tmpkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos){
		m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);
		delete cur_credit;
	}
	m_mapClients.RemoveAll();
	delete m_pSignkey;
	m_pSignkey = NULL;
}

void CClientCreditsList::LoadList()
{
	CString	strFileName = g_App.m_pPrefs->GetConfigDir();

	strFileName += CLIENTS_MET_FILENAME;

	const int	iOpenFlags = CFile::modeRead | CFile::osSequentialScan | CFile::typeBinary | CFile::shareDenyWrite;
	CSafeBufferedFile	file;
	CFileException		fexp;

	if (!file.Open(strFileName, iOpenFlags, &fexp)){
		if (fexp.m_cause != CFileException::fileNotFound){
			CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
			TCHAR szError[MAX_CFEXP_ERRORMSG];
			if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
				strError += _T(" - ");
				strError += szError;
			}
			g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, strError);
			g_App.m_pMDlg->DisableAutoBackup();
		}
		return;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 32*1024);

	try
	{
		byte	version;

		file.Read(&version, 1);
		if (version != CREDITFILE_VERSION)
		{
			g_App.m_pMDlg->AddLogLine(LOG_RGB_WARNING, IDS_ERR_CREDITFILEOLD);
			file.Close();
			return;
		}

	//	Everything is ok, lets see if a backup exists...
		bool	bCreateBackup = true;
		CString	strBakFileName = g_App.m_pPrefs->GetConfigDir();

		strBakFileName += CLIENTS_MET_FILENAME _T(".bak");

		HANDLE	hBakFile = ::CreateFile(strBakFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
										OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hBakFile != INVALID_HANDLE_VALUE)
		{
			FILETIME	ftModify, ftBakModify;

			if ( ::GetFileTime(file.m_hFile, NULL, NULL, &ftModify) &&
				::GetFileTime(hBakFile, NULL, NULL, &ftBakModify) )
			{
				CTime		ModTm(ftModify), BakModTm(ftBakModify);
				CTimeSpan	DiffTm(ModTm - BakModTm);
				uint32		dwSecs = static_cast<uint32>(DiffTm.GetTotalSeconds());

			//	Backup once per week (when file time difference is more than a week)
				if (dwSecs < MIN2S(7 * 24 * 60))
					bCreateBackup = FALSE;
			}
			::CloseHandle(hBakFile);
		}

	//	else: the backup doesn't exist, create it
		if (bCreateBackup)
		{
		 //	Close the file before copying
			file.Close();

			if (!::CopyFile(strFileName, strBakFileName, FALSE))
				g_App.m_pMDlg->AddLogLine(LOG_RGB_ERROR, IDS_ERR_MAKEBAKCREDITFILE);

		//	Reopen file
			CFileException fexp;
			if (!file.Open(strFileName, iOpenFlags, &fexp)){
				CString strError(GetResString(IDS_ERR_LOADCREDITFILE));
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, ARRSIZE(szError))){
					strError += _T(" - ");
					strError += szError;
				}
				g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, strError);
				g_App.m_pMDlg->DisableAutoBackup();
				return;
			}
			setvbuf(file.m_pStream, NULL, _IOFBF, 32*1024);
		//	Set filepointer behind file version byte
			file.Seek(1, CFile::begin);
		}

		uint32 count;
		file.Read(&count, 4);
		m_mapClients.InitHashTable(count+5000); // TODO: should be prime number... and 20% larger

		const uint32 dwExpired = time(NULL) - 12960000; // today - 150 day
		uint32 cDeleted = 0;
		for (uint32 i = 0; i < count; i++)
		{
			CreditStruct	NewCStruct;

			memzero(&NewCStruct, sizeof(CreditStruct));
			file.Read(&NewCStruct, sizeof(CreditStruct));

		//	Check time expiration as well as check for incorrect structure to avoid data corruption
			if ((NewCStruct.nLastSeen < dwExpired) || (NewCStruct.nKeySize > MAXPUBKEYSIZE))
			{
				cDeleted++;
				continue;
			}

			CClientCredits* newcredits = new CClientCredits(&NewCStruct);
			m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
		}
		file.Close();

		if (cDeleted > 0)
			g_App.m_pMDlg->AddLogLine(0, GetResString(IDS_CREDITFILELOADED) + GetResString(IDS_CREDITSEXPIRED), count - cDeleted, cDeleted);
		else
			g_App.m_pMDlg->AddLogLine(0, IDS_CREDITFILELOADED, count);
	}
	catch(CFileException *pError)
	{
		OUTPUT_DEBUG_TRACE();
		if (pError->m_cause == CFileException::endOfFile)
			g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_CREDITFILECORRUPT);
		else
		{
			TCHAR	acBuffer[MAX_CFEXP_ERRORMSG];

			pError->GetErrorMessage(acBuffer, MAX_CFEXP_ERRORMSG);
			g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR, IDS_ERR_CREDITFILEREAD, acBuffer);
		}
		pError->Delete();
		g_App.m_pMDlg->DisableAutoBackup();
	}
}

void CClientCreditsList::SaveList()
{
	m_nLastSaved = ::GetTickCount();

	CString	strFileName = g_App.m_pPrefs->GetConfigDir();

	strFileName += CLIENTS_MET_FILENAME;

//	No buffering needed here since we swap out the entire array
	CFile	file;
	BYTE	*pbyteBuffer = NULL;

	try
	{
	//	Let's open the file, since "Open" generates an exception, no need for any check
		file.Open(strFileName, CFile::modeWrite | CFile::modeCreate | CFile::typeBinary | CFile::shareDenyWrite);

		pbyteBuffer = new BYTE[m_mapClients.GetCount() * sizeof(CreditStruct)];

		CClientCredits	*cur_credit;
		CCKey		tempkey(0);
		POSITION	pos = m_mapClients.GetStartPosition();
		uint32		iCount = 0;

		while (pos != NULL)
		{
			m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
			if (cur_credit->GetUploadedTotal() || cur_credit->GetDownloadedTotal())
			{
				memcpy2(pbyteBuffer + (iCount * sizeof(CreditStruct)), cur_credit->GetDataStruct(), sizeof(CreditStruct));
				iCount++;
			}
		}

		const byte	version = CREDITFILE_VERSION;

		file.Write(&version, 1);
		file.Write(&iCount, 4);
		file.Write(pbyteBuffer, iCount * sizeof(CreditStruct));
		if (!g_App.m_pMDlg->IsRunning())
			file.Flush();
		file.Close();
	}
	catch (CFileException *pFlEx)
	{
		g_App.m_pMDlg->AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERROR_SAVEFILE2, CLIENTS_MET_FILENAME, GetErrorMessage(pFlEx));
		pFlEx->Delete();
	}
	catch(...) {}

	delete[] pbyteBuffer;
}

CClientCredits* CClientCreditsList::GetCredit(const uchar* key)
{
	CClientCredits* result;
	CCKey tkey(key);
	if (!m_mapClients.Lookup(tkey, result)){
		result = new CClientCredits(key);
		m_mapClients.SetAt(CCKey(result->GetKey()), result);
	}
	result->SetLastSeen();
	return result;
}

void CClientCreditsList::Process()
{
	if (::GetTickCount() - m_nLastSaved > MIN2MS(18))
		SaveList();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientCredits::InitalizeIdent()
{
	if (m_Credits.nKeySize == 0)
	{
		memzero(m_abyPublicKey, MAXPUBKEYSIZE);
		m_nPublicKeyLen = 0;
		IdentState = IS_NOTAVAILABLE;
	}
	else{
		m_nPublicKeyLen = m_Credits.nKeySize;
		memcpy2(m_abyPublicKey, m_Credits.abySecureIdent, m_nPublicKeyLen);
		IdentState = IS_IDNEEDED;
	}
	m_dwCryptRndChallengeFor = 0;
	m_dwCryptRndChallengeFrom = 0;
	m_dwIdentIP = 0;

	m_dwSecureWaitTime = 0;
	m_dwUnSecureWaitTime = 0;
	m_dwWaitTimeIP = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientCredits::Verified(uint32 dwForIP)
{
	m_dwIdentIP = dwForIP;
//	Client was verified, copy the key to store him if not done already
	if (m_Credits.nKeySize == 0)
	{
		m_Credits.nKeySize = m_nPublicKeyLen;
		memcpy2(m_Credits.abySecureIdent, m_abyPublicKey, m_nPublicKeyLen);
		if (GetDownloadedTotal() > 0)
		{
		//	For a security reason we have to delete all prior credits here
			m_Credits.nDownloadedHi = 0;
			m_Credits.nDownloadedLo = 1;
			m_Credits.nUploadedHi = 0;
			m_Credits.nUploadedLo = 1;
			DEBUG_ONLY(g_App.m_pMDlg->AddLogLine(LOG_FL_DBG, _T(__FUNCTION__) _T(": Credits deleted due to new SecureIdent")));
		}
	}
	IdentState = IS_IDENTIFIED;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CClientCredits::SetSecureIdent(const uchar *pachIdent, byte nIdentLen)
{
//	A verified Public key cannot change. If there is already a public key...
	if ((MAXPUBKEYSIZE < nIdentLen) || (m_Credits.nKeySize != 0))
		return false;
	memcpy2(m_abyPublicKey,pachIdent, nIdentLen);
	m_nPublicKeyLen = nIdentLen;
	IdentState = IS_IDNEEDED;
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
EIdentState CClientCredits::GetCurrentIdentState(uint32 dwForIP) const
{
//	Check we're not calling this function before we have an IP
	if(dwForIP == 0)
		return IS_IDNEEDED;

	if (IdentState != IS_IDENTIFIED)
		return IdentState;
	else
	{
		if (dwForIP == m_dwIdentIP)
			return IS_IDENTIFIED;
		else
			return IS_IDBADGUY;
			// mod note: clients which just reconnected after an IP change and have to ident yet will also have this state for
			// 1-2 seconds so don't try to spam such clients with "bad guy" messages (besides: spam messages are always bad)
	}
}

USING_NAMESPACE(CryptoPP)

void CClientCreditsList::InitalizeCrypting()
{
	m_nMyPublicKeyLen = 0;
	memzero(m_abyMyPublicKey, MAXPUBKEYSIZE);
	m_pSignkey = NULL;

//	Check if the key file exists
	bool	bCreateNewKey = false;
	HANDLE	hKeyFile = ::CreateFile( g_App.m_pPrefs->GetConfigDir() + _T("cryptkey.dat"), GENERIC_READ,
		FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );

//	If the key file doesn't exists or it is empty, we need to create a new one
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

//	Load key
	try
	{
	//	Load private key
		FileSource		filesource(CStringA(g_App.m_pPrefs->GetConfigDir() + _T("cryptkey.dat")), true, new Base64Decoder);

		m_pSignkey = new RSASSA_PKCS1v15_SHA_Signer(filesource);

	//	Calculate and store public key
		RSASSA_PKCS1v15_SHA_Verifier		pubkey(*m_pSignkey);
		ArraySink							asink(m_abyMyPublicKey, MAXPUBKEYSIZE);

		pubkey.DEREncode(asink);
		m_nMyPublicKeyLen = static_cast<byte>(asink.TotalPutLength());
		asink.MessageEnd();
	}
	catch(...)
	{
		delete m_pSignkey;
		m_pSignkey = NULL;
		g_App.m_pMDlg->AddLogLine(LOG_RGB_ERROR, IDS_CRYPT_INITFAILED);
	}
//	Debug_CheckCrypting();
}

bool CClientCreditsList::CreateKeyPair()
{
	try
	{
		AutoSeededRandomPool	rng;
		InvertibleRSAFunction	privkey;

		privkey.Initialize(rng, RSAKEYSIZE);

		Base64Encoder	privkeysink(new FileSink(CStringA(g_App.m_pPrefs->GetConfigDir() + _T("cryptkey.dat"))));

		privkey.DEREncode(privkeysink);
		privkeysink.MessageEnd();

		g_App.m_pMDlg->AddLogLine(LOG_FL_DBG | LOG_RGB_SUCCESS, _T("Created new RSA keypair"));
	}
	catch(...)
	{
		g_App.m_pMDlg->AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Failed to create new RSA keypair"));
		ASSERT ( false );
		return false;
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
byte CClientCreditsList::CreateSignature( CClientCredits *pTarget, uchar *pachOutput, byte nMaxSize, uint32 ChallengeIP,
										   byte byChaIPKind, RSASSA_PKCS1v15_SHA_Signer *sigkey /*= NULL*/ )
{
//	sigkey param is used for debug only
	if (sigkey == NULL)
		sigkey = m_pSignkey;

//	Create a signature of the public key from pTarget
	ASSERT( pTarget );
	ASSERT( pachOutput );

	byte		nResult;

	if ( !CryptoAvailable() )
	{
		return 0;
	}
	try
	{
		SecByteBlock			sbbSignature(sigkey->SignatureLength());
		AutoSeededRandomPool	rng;

		byte		abyteSignature[MAXPUBKEYSIZE+9];
		uint32		keylen = pTarget->GetSecIDKeyLen();

	//	Start by adding our public key which we are going to use as the signature "string".
		memcpy2(abyteSignature,pTarget->GetSecureIdent(),keylen);

	//	Append 4 additional bytes of data sent by the other client as extra verification
	//	that the signature is from us.
		uint32		challenge = pTarget->m_dwCryptRndChallengeFrom;

		ASSERT ( challenge != 0 );
		memcpy2(abyteSignature+keylen,&challenge,4);

		uint16		ChIpLen = 0;

		if (byChaIPKind != 0)
		{
			ChIpLen = sizeof(ChallengeIP) + sizeof(byChaIPKind);
		//	Append the challenge IP to the signature
			memcpy2(abyteSignature+keylen+4,&ChallengeIP,4);
		//	Append the challenge IP type (i.e. local client IP or remote client IP) to the signature
			memcpy2(abyteSignature+keylen+4+4,&byChaIPKind,1);
		}
	//	Encrypt the signature using our private key (only our public key can decrypt)
		sigkey->SignMessage(rng, abyteSignature, keylen+sizeof(challenge)+ChIpLen, sbbSignature.begin());

		ArraySink		asink(pachOutput, nMaxSize);

		asink.Put(sbbSignature.begin(), sbbSignature.size());
		nResult = static_cast<byte>(asink.TotalPutLength());
	}
	catch(...)
	{
		ASSERT ( false );
		nResult = 0;
	}
	return nResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	VerifyIdent() verifies that the sender of the signature is who he says he is given a signature string encrypted with
//	the client's private key
bool CClientCreditsList::VerifyIdent(CClientCredits* pTarget, const uchar *pachSignature, byte nInputSize, uint32 dwForIP, byte byChaIPKind)
{
	ASSERT( pTarget );
	ASSERT( pachSignature );
	if (!CryptoAvailable())
	{
		pTarget->IdentState = IS_NOTAVAILABLE;
		return false;
	}

	bool		bResult = false;

	try
	{
	//	We have all the information that the client did when its signature was created and we know
	//	how it was created. Given that we can reproduce the unencrypted signature that we should receive.
		StringSource					ss_Pubkey((byte*)pTarget->GetSecureIdent(),pTarget->GetSecIDKeyLen(),true,0);
		RSASSA_PKCS1v15_SHA_Verifier	pubkey(ss_Pubkey);

		byte		abyteTestSig[MAXPUBKEYSIZE+9];

		memcpy2(abyteTestSig,m_abyMyPublicKey,m_nMyPublicKeyLen);

		uint32		challenge = pTarget->m_dwCryptRndChallengeFor;

		ASSERT ( challenge != 0 );

		memcpy2(abyteTestSig+m_nMyPublicKeyLen,&challenge,4);

	//	v2 security improvements (not supported by 29b, not used as default by 29c)
		byte		nChIpSize = 0;

		if (byChaIPKind != 0)
		{
			uint32		ChallengeIP = 0;

			nChIpSize = sizeof(ChallengeIP) + sizeof(byChaIPKind);

			switch (byChaIPKind)
			{
				case CRYPT_CIP_LOCALCLIENT:
					ChallengeIP = dwForIP;
					break;
				case CRYPT_CIP_REMOTECLIENT:
#ifdef OLD_SOCKETS_ENABLED
					if (g_App.m_pServerConnect->GetClientID() == 0 || g_App.m_pServerConnect->IsLowID())
					{
						g_App.m_pMDlg->AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("WARNING: Maybe SecureHash Ident fails because LocalIP is unknown"));
						ChallengeIP = g_App.m_pServerConnect->GetLocalIP();
					}
					else
					{
						ChallengeIP = g_App.m_pServerConnect->GetClientID();
					}
#endif OLD_SOCKETS_ENABLED
					break;
				//	Maybe not supported in future versions
 	 				case CRYPT_CIP_NONECLIENT:
					ChallengeIP = 0;
					break;
			}
			memcpy2(abyteTestSig+m_nMyPublicKeyLen+4,&ChallengeIP,4);
			memcpy2(abyteTestSig+m_nMyPublicKeyLen+4+4,&byChaIPKind,1);
		}
	//	Verify that when we decrypt the signature sent to us that it matches our test signature.
    	bResult = pubkey.VerifyMessage(abyteTestSig, m_nMyPublicKeyLen+4+nChIpSize, pachSignature, nInputSize);
	}
	catch(...)
	{
		bResult = false;
	}
	if (!bResult)
	{
		if (pTarget->IdentState == IS_IDNEEDED)
			pTarget->IdentState = IS_IDFAILED;
	}
	else
	{
		pTarget->Verified(dwForIP);
	}
	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	CryptoAvailable() returns true if we have sufficient information to verify a signature
bool CClientCreditsList::CryptoAvailable()
{
	return (m_nMyPublicKeyLen > 0 && m_pSignkey != NULL);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
bool CClientCreditsList::Debug_CheckCrypting()
{
// 	Create random key
	AutoSeededRandomPool rng;

	RSASSA_PKCS1v15_SHA_Signer priv(rng, 384);
	RSASSA_PKCS1v15_SHA_Verifier pub(priv);

	byte abyPublicKey[MAXPUBKEYSIZE];
	ArraySink asink(abyPublicKey, MAXPUBKEYSIZE);
	pub.DEREncode(asink);
	byte PublicKeyLen = asink.TotalPutLength();
	asink.MessageEnd();
	uint32 challenge = rand();
//	Create fake client which pretends to be this eMule
	CreditStruct	NewCStruct;

	memzero(&NewCStruct, sizeof(CreditStruct));
	CClientCredits* newcredits = new CClientCredits(&NewCStruct);
	newcredits->SetSecureIdent(m_abyMyPublicKey,m_nMyPublicKeyLen);
	newcredits->m_dwCryptRndChallengeFrom = challenge;
//	Create signature with fake priv key
	uchar pachSignature[200];
	memzero(pachSignature, 200);
	byte sigsize = CreateSignature(newcredits,pachSignature,200,0,false, &priv);

//	Next fake client uses the random created public key
	memzero(&NewCStruct, sizeof(CreditStruct));
	CClientCredits* newcredits2 = new CClientCredits(&NewCStruct);
	newcredits2->m_dwCryptRndChallengeFor = challenge;

//	I you uncomment one of the following lines the check has to fail
	//abyPublicKey[5] = 34;
	//m_abyMyPublicKey[5] = 22;
	//pachSignature[5] = 232;

	newcredits2->SetSecureIdent(abyPublicKey,PublicKeyLen);

//	Now verify this signature - if it's true everything is fine
	bool bResult = VerifyIdent(newcredits2,pachSignature,sigsize,0,0);

	delete newcredits;
	delete newcredits2;

	return bResult;
}
#endif
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CClientCredits::GetSecureWaitStartTime(uint32 dwForIP)
{
//	Check the waiting time just in case it was not initialized
	if (m_dwUnSecureWaitTime == 0)
		return (::GetTickCount() - 100);

//	This hash is protected by SecureHash
	if (m_Credits.nKeySize != 0)
	{
	//	The client was already identified -> good boy
		if (GetCurrentIdentState(dwForIP) == IS_IDENTIFIED)
		{
			return m_dwSecureWaitTime;
		}
	//	The client is not (yet) identified
		else
		{
		//	Case 1: Client entered into the waiting queue or was unbanned before he was able to identify himself.
		//			Let's use unsecure time for a while
		//	Case 2: Client disable SUI -> use unsecure time
			if (dwForIP == m_dwWaitTimeIP)
			{
				return m_dwUnSecureWaitTime;
			}
		//	The client is in waiting queue(WQ), but not identified, so let's update unsecure time and simulate an enter in to WQ,
		//	this way a bad client will restart from the end of the queue as normal one & proper client will get his
		//	time as soon as he'll be identified
			else
			{
				m_dwUnSecureWaitTime = (::GetTickCount() - 100);
				m_dwWaitTimeIP = dwForIP;
				return m_dwUnSecureWaitTime;
			}
		}
	}
	else
	{
	//	Not a SecureHash Client - handle it like before for now (no security checks)
		return m_dwUnSecureWaitTime;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CClientCredits::SetSecWaitStartTime(uint32 dwForIP)
{
	m_dwSecureWaitTime = m_dwUnSecureWaitTime = ::GetTickCount();
	m_dwWaitTimeIP = dwForIP;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
