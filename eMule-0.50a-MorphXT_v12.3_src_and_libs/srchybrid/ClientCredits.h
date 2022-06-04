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
#include <cryptopp/rsa.h>
#pragma warning(default:4702) // unreachable code
#pragma warning(default:4100) // unreferenced formal parameter
#pragma warning(default:4244) // conversion from 'type1' to 'type2', possible loss of data
#pragma warning(default:4516) // access-declarations are deprecated; member using-declarations provide a better alternative

#define	 MAXPUBKEYSIZE		80

#define CRYPT_CIP_REMOTECLIENT	10
#define CRYPT_CIP_LOCALCLIENT	20
#define CRYPT_CIP_NONECLIENT	30

#pragma pack(1)
struct CreditStruct_29a{
	uchar		abyKey[16];
	uint32		nUploadedLo;	// uploaded TO him
	uint32		nDownloadedLo;	// downloaded from him
	uint32		nLastSeen;
	uint32		nUploadedHi;	// upload high 32
	uint32		nDownloadedHi;	// download high 32
	uint16		nReserved3;
};
//Morph Start - modified by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)

struct CreditStruct_30c{
	uchar		abyKey[16];
	uint32		nUploadedLo;	// uploaded TO him
	uint32		nDownloadedLo;	// downloaded from him
	uint32		nLastSeen;
	uint32		nUploadedHi;	// upload high 32
	uint32		nDownloadedHi;	// download high 32
	uint16		nReserved3;
	uint8		nKeySize;
	uchar		abySecureIdent[MAXPUBKEYSIZE];
};

// Moonlight: SUQWT: Add the wait time data to the structure.
#pragma pack(1)
struct CreditStruct_30c_SUQWTv1{
	uchar		abyKey[16];
	uint32		nUploadedLo;	// uploaded TO him
	uint32		nDownloadedLo;	// downloaded from him
	uint32		nLastSeen;
	uint32		nUploadedHi;	// upload high 32
	uint32		nDownloadedHi;	// download high 32
	uint16		nReserved3;
	uint8		nKeySize;
	uchar		abySecureIdent[MAXPUBKEYSIZE];
	uint32		nSecuredWaitTime;	// Moonlight: SUQWT
	uint32		nUnSecuredWaitTime;	// Moonlight: SUQWT
};

struct CreditStruct_30c_SUQWTv2{
	uint32		nSecuredWaitTime;	// Moonlight: SUQWT
	uint32		nUnSecuredWaitTime;	// Moonlight: SUQWT
	uchar		abyKey[16];
	uint32		nUploadedLo;	// uploaded TO him
	uint32		nDownloadedLo;	// downloaded from him
	time_t		nLastSeen;  // vs2005
	uint32		nUploadedHi;	// upload high 32
	uint32		nDownloadedHi;	// download high 32
	uint16		nReserved3;
	uint8		nKeySize;
	uchar		abySecureIdent[MAXPUBKEYSIZE];
};

// Moonlight: SUQWT
struct CreditStructSUQWT {
	uint32		nSecuredWaitTime;
	uint32		nUnSecuredWaitTime;
};

//original commented out
/*
struct CreditStruct{
	uchar		abyKey[16];
	uint32		nUploadedLo;	// uploaded TO him
	uint32		nDownloadedLo;	// downloaded from him
	uint32		nLastSeen;
	uint32		nUploadedHi;	// upload high 32
	uint32		nDownloadedHi;	// download high 32
	uint16		nReserved3;
	uint8		nKeySize;
	uchar		abySecureIdent[MAXPUBKEYSIZE];
};
*/

//Morph End - modified by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)

#pragma pack()
//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
typedef CreditStruct_30c_SUQWTv2	CreditStruct;	// Moonlight: Standard name for the credit structure.
//Morph End - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
enum EIdentState{
	IS_NOTAVAILABLE,
	IS_IDNEEDED,
	IS_IDENTIFIED,
	IS_IDFAILED,
	IS_IDBADGUY,
};
//EastShare Start - added by AndCycle, creditsystem integration
enum CreditSystemSelection {
	//becareful the sort order for the damn radio button in PPgEastShare.cpp and the check on creditSystemMode in preferences.cpp
	CS_OFFICIAL = 0,	
	CS_LOVELACE,
//	CS_RATIO,
	CS_PAWCIO,
	CS_EASTSHARE
};
//EastShare End - added by AndCycle, creditsystem integration
class CClientCredits
{
	friend class CClientCreditsList;
public:
	CClientCredits(CreditStruct* in_credits);
	CClientCredits(const uchar* key);
	~CClientCredits();

	const uchar* GetKey() const					{return m_pCredits->abyKey;}
	uchar*	GetSecureIdent()				{return m_abyPublicKey;}
	uint8	GetSecIDKeyLen() const				{return m_nPublicKeyLen;}
	CreditStruct* GetDataStruct() const		{return m_pCredits;}
	void	ClearWaitStartTime();
	//MORPH START - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	void    SaveUploadQueueWaitTime(int iKeepPct = 100);		// Moonlight: SUQWT
	void	ClearUploadQueueWaitTime();							// Moonlight: SUQWT
	//MORPH END - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	void	AddDownloaded(uint32 bytes, uint32 dwForIP);
	void	AddUploaded(uint32 bytes, uint32 dwForIP);
	uint64	GetUploadedTotal() const;
	uint64	GetDownloadedTotal() const;
	float	GetScoreRatio(uint32 dwForIP) /*const*/;
	float	GetMyScoreRatio(uint32 dwForIP) const; //MORPH - Added by IceCream, VQB: ownCredits
	void	SetLastSeen()					{m_pCredits->nLastSeen = time(NULL);}
	bool	SetSecureIdent(const uchar* pachIdent, uint8 nIdentLen); // Public key cannot change, use only if there is not public key yet
	//Morph Start - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	bool	IsActive(time_t dwExpire);	// Moonlight: SUQWT, new function to determine if the record has expired.
	//Morph End - added by AndCycle, Moonlight's Save Upload Queue Wait Time (MSUQWT)
	uint32	m_dwCryptRndChallengeFor;
	uint32	m_dwCryptRndChallengeFrom;
	EIdentState	GetCurrentIdentState(uint32 dwForIP) const; // can be != IdentState
	//EastShare START - Modified by TAHO, modified SUQWT
	/*
	uint32	GetSecureWaitStartTime(uint32 dwForIP);
	*/
	sint64	GetSecureWaitStartTime(uint32 dwForIP);
	//EastShare END - Modified by TAHO, modified SUQWT
	void	SetSecWaitStartTime(uint32 dwForIP);
	void	SetSecWaitStartTime(); //EastShare - Added by TAHO, modified SUQWT

	//EastShare Start - added by AndCycle, Pay Back First
	bool	GetPayBackFirstStatus()			{return m_bPayBackFirst;}
	void	InitPayBackFirstStatus();
	//EastShare End - added by AndCycle, Pay Back First
	//MORPH START - Added by SiRoB, reduce a little CPU usage for ratio count
	void	ResetCheckScoreRatio() {m_bCheckScoreRatio = true;}
	//MORPH END   - Added by SiRoB, reduce a little CPU usage for ratio count

	//MORPH START - Added by Stulle, fix score display
	bool			GetHasScore(uint32 dwForIP);
	//MORPH END - Added by Stulle, fix score display

protected:
	void	Verified(uint32 dwForIP);
	EIdentState IdentState;
private:
	void			InitalizeIdent();
	CreditStruct*	m_pCredits;
	byte			m_abyPublicKey[80];			// even keys which are not verified will be stored here, and - if verified - copied into the struct
	uint8			m_nPublicKeyLen;
	uint32			m_dwIdentIP;
    //Commander - Changed: SUQWT - Start
	//EastShare START - Modified by TAHO, modified SUQWT
	/*
	uint32			m_dwSecureWaitTime;
	uint32			m_dwUnSecureWaitTime;
	*/
	sint64			m_dwSecureWaitTime;
	sint64			m_dwUnSecureWaitTime;
        // EastShare - added by TAHO, modified SUQWT
        //Commander - Changed: SUQWT - End
        uint32			m_dwWaitTimeIP;	
	//Morph Start - Added by AndCycle, reduce a little CPU usage for ratio count
	bool			m_bCheckScoreRatio;
	float			m_fLastScoreRatio;
	//Morph End - Added by AndCycle, reduce a little CPU usage for ratio count

	//EastShare Start - added by AndCycle, Pay Back First
	bool			m_bPayBackFirst;
	void			TestPayBackFirstStatus();
	//EastShare End - added by AndCycle, Pay Back First

};

class CClientCreditsList
{
public:
	CClientCreditsList();
	~CClientCreditsList();
	
			// return signature size, 0 = Failed | use sigkey param for debug only
	uint8	CreateSignature(CClientCredits* pTarget, uchar* pachOutput, uint8 nMaxSize, uint32 ChallengeIP, uint8 byChaIPKind, CryptoPP::RSASSA_PKCS1v15_SHA_Signer* sigkey = NULL);
	bool	VerifyIdent(CClientCredits* pTarget, const uchar* pachSignature, uint8 nInputSize, uint32 dwForIP, uint8 byChaIPKind);

	CClientCredits* GetCredit(const uchar* key);
	void	Process();
	uint8	GetPubKeyLen() const			{return m_nMyPublicKeyLen;}
	byte*	GetPublicKey()					{return m_abyMyPublicKey;}
	bool	CryptoAvailable();
	bool	IsSaveUploadQueueWaitTime() { return m_bSaveUploadQueueWaitTime;}//MORPH - Added by AndCycle, Save Upload Queue Wait Time (SUQWT)
	void	ResetCheckScoreRatio(); //MORPH START - Added by SiRoB, reduce a little CPU usage for ratio count
protected:
	void	LoadList();
	void	SaveList();
	void	InitalizeCrypting();
	bool	CreateKeyPair();
	bool	m_bSaveUploadQueueWaitTime;//MORPH - Added by SiRoB, Save Upload Queue Wait Time (SUQWT)
#ifdef _DEBUG
	bool	Debug_CheckCrypting();
#endif
private:
	CMap<CCKey, const CCKey&, CClientCredits*, CClientCredits*> m_mapClients;
	uint32			m_nLastSaved;
	CryptoPP::RSASSA_PKCS1v15_SHA_Signer*		m_pSignkey;
	byte			m_abyMyPublicKey[80];
	uint8			m_nMyPublicKeyLen;
};
