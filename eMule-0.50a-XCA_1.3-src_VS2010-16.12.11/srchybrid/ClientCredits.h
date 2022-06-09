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
#pragma once
#include "MapKey.h"
#pragma warning(disable:4516) // access-declarations are deprecated; member using-declarations provide a better alternative
#pragma warning(disable:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(disable:4100) // unreferenced formal parameter
#pragma warning(disable:4702) // unreachable code
#include <crypto/rsa.h> //Xman
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative

#define	 MAXPUBKEYSIZE		80

#define CRYPT_CIP_REMOTECLIENT	10
#define CRYPT_CIP_LOCALCLIENT	20
#define CRYPT_CIP_NONECLIENT	30

#pragma pack(1)
//struct CreditStruct_29a{
//	uchar		abyKey[16];
//	uint32		nUploadedLo;	// uploaded TO him
//	uint32		nDownloadedLo;	// downloaded from him
//	uint32		nLastSeen;
//	uint32		nUploadedHi;	// upload high 32
//	uint32		nDownloadedHi;	// download high 32
//	uint16		nReserved3;
//};
struct CreditStruct{
	uchar		abyKey[16];
	uint32		nUploadedLo;	// uploaded TO him
	uint32		nDownloadedLo;	// downloaded from him
	uint64		nLastSeen;		// X: [64T] - [64BitTime]
	uint32		nUploadedHi;	// upload high 32
	uint32		nDownloadedHi;	// download high 32
	uint16		nReserved3;
	uint8		nKeySize;
	uchar		abySecureIdent[MAXPUBKEYSIZE];
};
#pragma pack()


enum EIdentState{
	IS_NOTAVAILABLE,
	IS_IDNEEDED,
	IS_IDENTIFIED,
	IS_IDFAILED,
	IS_IDBADGUY,
};

class CClientCredits
{
	friend class CClientCreditsList;
public:
	CClientCredits(CreditStruct* in_credits);
	CClientCredits(const uchar* key, CreditStruct* in_credits);
	~CClientCredits() {}

	const uchar* GetKey() const					{return m_pCredits->abyKey;}
	/*
	uchar*	GetSecureIdent()					{return m_abyPublicKey;}
	*/	//Enig123::ACAT optimization
	const uchar* GetSecureIdent() const				{return m_pCredits->abySecureIdent;}

	uint8	GetSecIDKeyLen() const				{return m_nPublicKeyLen;}
	//CreditStruct* GetDataStruct() const			{return m_pCredits->credit;}
	void	ClearWaitStartTime();
	void	AddDownloaded(uint32 bytes, uint32 dwForIP);
	void	AddUploaded(uint32 bytes, uint32 dwForIP);
	uint64	GetUploadedTotal() const;
	uint64	GetDownloadedTotal() const;
	float	GetScoreRatio(uint32 dwForIP) const;
	const float	GetMyScoreRatio(uint32 dwForIP) const; // See own credits

	void	SetLastSeen()					{m_pCredits->nLastSeen = time(NULL);}
	bool	SetSecureIdent(const uchar* pachIdent, uint8 nIdentLen); // Public key cannot change, use only if there is not public key yet
	uint32	m_dwCryptRndChallengeFor;
	uint32	m_dwCryptRndChallengeFrom;
	EIdentState	GetCurrentIdentState(uint32 dwForIP) const; // can be != IdentState
	uint32	GetSecureWaitStartTime(uint32 dwForIP);
	void	SetSecWaitStartTime(uint32 dwForIP);

	void	SetWaitStartTimeBonus(uint32 dwForIP, uint32 timestamp); //Xman Xtreme Full CHunk

	//Xman Extened credit- table-arragement
	void	MarkToDelete() {m_bmarktodelete=true;} 
	void	UnMarkToDelete() {m_bmarktodelete=false;}
	bool	GetMarkToDelete() const {return m_bmarktodelete;} 
	uint64	GetLastSeen() const {return m_pCredits->nLastSeen;}// X: [64T] - [64BitTime]
	//Xman end

protected:
	void	Verified(uint32 dwForIP);
	EIdentState IdentState;
private:
	void			InitalizeIdent();
	CreditStruct*	m_pCredits;
	/*
	byte			m_abyPublicKey[80];			// even keys which are not verified will be stored here, and - if verified - copied into the struct
	*/	//Enig123::ACAT optimization
	uint32			m_dwIdentIP;
	uint32			m_dwSecureWaitTime;
	uint32			m_dwUnSecureWaitTime;
	uint32			m_dwWaitTimeIP;			   // client IP assigned to the waittime
	uint8			m_nPublicKeyLen;
	bool			m_bmarktodelete;			//Xman Extened credit- table-arragement
	// ==> Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
public:
	void	InitPayBackFirstStatus();
	bool	GetPayBackFirstStatus()			{return m_bPayBackFirst;}
	//bool	GetPayBackFirstStatus2()		{return m_bPayBackFirst2;}
private:
	void	TestPayBackFirstStatus();
	bool	m_bPayBackFirst;
	//bool	m_bPayBackFirst2;
	// <== Pay Back First [AndCycle/SiRoB/Stulle] - Stulle
};

struct ClientCreditContainer{
	CreditStruct		credit;
	CClientCredits*		clientCredit;
};
#ifdef REPLACE_MFCMAP
typedef unordered_map<const uchar*, ClientCreditContainer*, hash_unordered, hash_unordered> CClientCreditMap;
#else
typedef CMap<CCKey,const CCKey&,ClientCreditContainer*,ClientCreditContainer*> CClientCreditMap;
#endif

class CClientCreditsList
{
public:
	CClientCreditsList();
	~CClientCreditsList();
	
			// return signature size, 0 = Failed | use sigkey param for debug only
	uint8	CreateSignature(CClientCredits* pTarget, uchar* pachOutput, uint8 nMaxSize, uint32 ChallengeIP, uint8 byChaIPKind, CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey = NULL);
	bool	VerifyIdent(CClientCredits* pTarget, const uchar* pachSignature, uint8 nInputSize, uint32 dwForIP, uint8 byChaIPKind);	

	CClientCredits* GetCredit(const uchar* key) ;
	void	Process();
	uint8	GetPubKeyLen() const			{return m_nMyPublicKeyLen;}
	byte*	GetPublicKey()					{return m_abyMyPublicKey;}
	bool	CryptoAvailable();

#ifdef _DEBUG
	void	PrintStatistic();
#endif

protected:
	void	LoadList();
	void	SaveList();
	void	InitalizeCrypting();
	bool	CreateKeyPair();
#ifdef _DEBUG
	bool	Debug_CheckCrypting();
#endif
private:
	CClientCreditMap m_mapClients;
	uint32			m_nLastSaved;
	CryptoPP::RSASSA_PKCS1v15_SHA_Signer*		m_pSignkey;
	byte			m_abyMyPublicKey[80];
	uint8			m_nMyPublicKeyLen;
	UINT	GetPrime(UINT calc) const; //zz_fly :: prime table
};
