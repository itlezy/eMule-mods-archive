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
#pragma once

#include "types.h"
#include "mapkey.h"

#pragma warning(push, 3)	// preserve current state, then set warning level 3
#pragma warning(disable:4702) // unreachable code
#include "crypto51\rsa.h"
#pragma warning(pop)

#define	 MAXPUBKEYSIZE		80

#define CRYPT_CIP_REMOTECLIENT	10
#define CRYPT_CIP_LOCALCLIENT	20
#define CRYPT_CIP_NONECLIENT	30

#pragma pack(1)
struct CreditStruct{
	uchar		abyKey[16];
	uint32		nUploadedLo;	// uploaded TO him
	uint32		nDownloadedLo;	// downloaded from him
	uint32		nLastSeen;
	uint32		nUploadedHi;	// upload high 32
	uint32		nDownloadedHi;	// download high 32
	uint16		nReserved3;
	byte		nKeySize;
	uchar		abySecureIdent[MAXPUBKEYSIZE];
};
#pragma pack()

enum EIdentState{
	IS_NOTAVAILABLE,
	IS_IDNEEDED,
	IS_IDENTIFIED,
	IS_IDFAILED,
	IS_IDBADGUY
};

class CClientCredits
{
	friend class CClientCreditsList;
public:
	CClientCredits(const CreditStruct *pInCredits);
	CClientCredits(const uchar* key);
	~CClientCredits();

	const uchar* GetKey() const				{return m_Credits.abyKey;}
	uchar*	GetSecureIdent()				{return m_abyPublicKey;}
	byte	GetSecIDKeyLen() const			{return m_nPublicKeyLen;}
	const CreditStruct* GetDataStruct() const	{return &m_Credits;}
	void	AddDownloaded(uint32 bytes, uint32 dwForIP);
	void	AddUploaded(uint32 bytes, uint32 dwForIP);
	uint64	GetUploadedTotal() const;
	uint64	GetDownloadedTotal() const;
	double	GetScoreRatio(uint32 dwForIP, bool inverted = false) const;
	bool	HasHigherScoreRatio(uint32 dwForIP) const;
	void	SetLastSeen()					{m_Credits.nLastSeen = time(NULL);}
	bool	SetSecureIdent(const uchar *pachIdent, byte nIdentLen); // Public key cannot change, use only if there is not public key yet
	EIdentState	GetCurrentIdentState(uint32 dwForIP) const; // can be != IdentState
	uint32	GetSecureWaitStartTime(uint32 dwForIP);
	void	SetSecWaitStartTime(uint32 dwForIP);

	uint32	m_dwCryptRndChallengeFor;
	uint32	m_dwCryptRndChallengeFrom;
private:
	void			InitalizeIdent();

	CreditStruct	m_Credits;
	byte			m_nPublicKeyLen;
	uint32			m_dwIdentIP;
	uint32			m_dwSecureWaitTime;
	uint32			m_dwUnSecureWaitTime;
	uint32			m_dwWaitTimeIP;			// client IP assigned to the waittime
	byte			m_abyPublicKey[MAXPUBKEYSIZE];			// even keys which are not verified will be stored here, and - if verified - copied into the struct
protected:
	void	Verified(uint32 dwForIP);
	EIdentState IdentState;
};

class CClientCreditsList : public CLoggable{
public:
	CClientCreditsList();
	~CClientCreditsList();
	
			// return signature size, 0 = Failed | use sigkey param for debug only
	byte	CreateSignature(CClientCredits* pTarget, uchar* pachOutput, byte nMaxSize, uint32 ChallengeIP, byte byChaIPKind, CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey = NULL);
	bool	VerifyIdent(CClientCredits* pTarget, const uchar *pachSignature, byte nInputSize, uint32 dwForIP, byte byChaIPKind);	

	CClientCredits* GetCredit(const uchar* key);
	void	Process();
	byte	GetPubKeyLen() const			{return m_nMyPublicKeyLen;}
	byte*	GetPublicKey()					{return m_abyMyPublicKey;}
	bool	CryptoAvailable();
protected:
	void	LoadList();
	void	SaveList();
	void	InitalizeCrypting();
	bool	CreateKeyPair();
#ifdef _DEBUG
	bool	Debug_CheckCrypting();
#endif
private:
	CMap<CCKey, const CCKey&, CClientCredits*, CClientCredits*> m_mapClients;
	uint32			m_nLastSaved;
	CryptoPP::RSASSA_PKCS1v15_SHA_Signer*		m_pSignkey;
	byte			m_abyMyPublicKey[MAXPUBKEYSIZE];
	byte			m_nMyPublicKeyLen;
};
