//////////////////////////////////////////////////////////////////////////
//
//this file is part of eMule
//Copyright (C)2002-2010 WiZaRd (thewizardofdos@gmail.com)
//
//For more infos see the .cpp file!
//
//////////////////////////////////////////////////////////////////////////

#pragma once
#include "MapKey.h"

#ifdef  CLIENTANALYZER

class CTag;
class CFileDataIO;

//////////////////////////////////////////////////////////////////////////
// IMPORTANT DEFINES
#define ANTILEECH_MET_FILENAME		_T("antileech.met")

#define AT_MET_HEADER_ACCEPTED		0x01 //first version
#define AT_MET_HEADER				0x02 //current - 64bit tags!

#define AT_UPLOAD_PART				0x01
#define AT_UPLOAD_PART_HI			0x02
#define AT_UPLOAD					0x03
#define AT_UPLOAD_HI				0x04

#define AT_DOWNLOAD_PART			0x05
#define AT_DOWNLOAD_PART_HI			0x06
#define AT_DOWNLOAD					0x07
#define AT_DOWNLOAD_HI				0x08

#define	AT_LASTSEEN					0x09

#define AT_REASKS					0x0A
#define AT_AVGREASK					0x0B
#define AT_SPAMS					0x0C
#define AT_XS_ASKS					0x0D
#define AT_XS_ANSW					0x0E
#define AT_FASTXS_ASKS				0x0F

#define AT_UPLOAD_RARE				0x10
#define AT_UPLOAD_RARE_HI			0x11
#define AT_DOWNLOAD_RARE			0x12
#define AT_DOWNLOAD_RARE_HI			0x13

#define AT_FIRSTMET					0x14
#define AT_BADTIMER					0x15 //keep bad status for some time
#define AT_BADSTATUS				0x16 //keep bad status for some time

#define AT_BADULSESSIONS			0x17
#define AT_BADDLSESSIONS			0x18

//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
// ADJUSTABLE VALUES (for now)
#define AT_RAREFILE				(25)		//number of srcs below which a file is considered rare
#define AT_XSSPAM_LEVEL			(3)			//too fast xs requests before being marked as spammer
#define AT_XSEXPLOIT_LEVEL		(10)		//messages that must be received before checking
#define AT_XSEXPLOIT_PERCENT	(3)			//3 = 1/3 of all requests answered? 

//score defines
#define AT_BASESCORE			(1.0f)	//every client starts with AT_BASESCORE points
#define AT_HIPUNISH				(0.5f)	//for severe cases, subtract by AT_HIPUNISH
#define AT_MIDPUNISH			(0.1f)	//for less severe cases, subtract by AT_MIDPUNISH
#define AT_LOWPUNISH			(0.025f)		//for really weak cases, subtract by AT_LOWPUNISH
#define AT_MINSCORE				(0.01f)		//the score might not fall below AT_MINSCORE
#define AT_MAXSCORE				(10.0f)	//the score might not raise above AT_MAXSCORE
#define AT_ROUNDOFFSET			(0.2f)		//rounding offset - 0.2 means that 80% of a chunk will count as 1 chunk
#define	AT_REASKBUFFER			(MIN2MS(5))	//a buffer for not punishing great parts of clients

//ul/dl weighting defines
#define AT_FULLFILEWEIGHT		(1.5f)		//full file upload is counted different - this is to reward real sharer
#define AT_PARTFILEWEIGHT		(1.5f)		//partial file download is counted different because that means that the client should be able to share something with us
//NOTE: 1.0 means that it counts TWICE as it's already counted in the main counter now!
#define AT_RARESHAREWEIGHT		(1.0f)		//boost uploaders of rare files /*and punish downloaders of this stuff, they should be grateful :)*/

//thresholds
#define AT_BADUPLOADSTRES		(3)			//Try to increase successful ul sessions
#define	AT_BADDOWNLOADSTRES		(5)			//Spike2: ban/filter after 6 failed DL attempts

#define AT_NICKTHIEF			0x01
#define AT_MODTHIEF				0x02
#define AT_UDPFNFFAKER			0x04
#define AT_FILEFAKER			0x08
#define AT_MODFAKER				0x10
//
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//Bad action stat:
#define BAD_ACTION_NICKTHIEF	0
#define BAD_ACTION_MODTHIEF		1
#define BAD_ACTION_FILEFAKE		2
#define BAD_ACTION_UDPFNF		3
#define BAD_ACTION_FASTREASK	4
#define BAD_ACTION_SPAMS		5
#define BAD_ACTION_FASTXS		6
#define BAD_ACTION_MODFAKER		7
#define BAD_ACTION_COUNTER		8 //d'oh!
//
//////////////////////////////////////////////////////////////////////////

#pragma pack(1)
//size is 16+4+4+4+4+4+4+4+4+4+1+1+8+8+8+8+8+8
struct CAntiLeechStruct
{
	uchar		abyKey[16];			//like for the credits, this data is hash-dependant
	DWORD		dwLastSeen;			//like for the credits, save the last known time
	DWORD		dwFirstMet;			//keep track about how long we know this guy
	DWORD		dwBadTimer;			//keep bad status for some time
	UINT		uReasks;			//anti fast reask
	UINT		uAvgReaskTime;		//anti fast reask //in secs!
	int			iBlockedMessages;	//anti spam
	UINT		uXSAsks;			//anti XS exploit
	UINT		uXSAnsw;			//anti XS exploit
	int			iFastXSAsks;		//anti XS spam
	uint8		uBadULSessions;		//successive failed UL sessions
	uint8		uBadDLSessions;		//successive failed DL sessions
	uint64		uSentBytesRare;		//uploaded TO this client from a rare file
	uint64		uSentBytesPartial;	//uploaded TO this client from a partial file
	uint64		uSentBytes;			//uploaded TO this client from a FULL file (real sharing)
	uint64		uGotBytesRare;		//downloaded FROM this client from a rare file
	uint64		uGotBytesPartial;	//downloaded FROM this client from a partial file
	uint64		uGotBytes;			//downloaded FROM this client from a FULL file (real sharing)
};
#pragma pack()

class CAntiLeechData
{
public:
	CAntiLeechData(CFileDataIO* file);
	CAntiLeechData(const uchar* key);

	CAntiLeechData(const CAntiLeechData* toCopy);
	void	Merge(const CAntiLeechData* toMerge);

	~CAntiLeechData();

	void	Init();

//////////////////////////////////////////////////////////////////////////
// ANTINICKTHIEF	
	static	CString	GetAntiNickThiefNick();		//this creates a new string or returns the current one
	void	Check4NickThief();					//performs the nickthief checks	
	static	void	CreateAntiNickThiefTag();	//this will be automatically called
	static	CString m_sAntiNickThiefTag;
	static	CString m_sAntiNickThiefTag2;

//////////////////////////////////////////////////////////////////////////
// ANTIMODTHIEF
	void	Check4ModThief();					//performs the modthief checks	
	static	CString m_sMyVersion;

//////////////////////////////////////////////////////////////////////////
// ANTIMODFAKE
	void	Check4ModFaker(const bool bIsBadShareaza = false);	//performs the modfaker checks	

//////////////////////////////////////////////////////////////////////////
// MISC CHECK FUNCTIONS
	bool	AdditionalModThiefCheck(const UINT i);			//performs an additional modthief check (bad clients with our modstring are probably modthieves)
	void	Check4FileFaker();					//performs the filefaker check

//////////////////////////////////////////////////////////////////////////
// COMMON ANTITHIEF/FAKE
	void	SetBadForThisSession(const UINT i, const CString& strDetail = L"");	//marks a client as bad for a certain deed
	bool	GetBadForThisSession(const UINT i) const;
	void	ClearBadForThisSession(const UINT i);	//remove the mark of a certain deed

//////////////////////////////////////////////////////////////////////////
// TRANSFER STATS - UPLOAD
	//functions to set/retrieve the uploaded data
	void	AddUploaded(const uint64& bytes, const bool bPartial, const UINT srccount);
	uint64	GetUploadedRare() const;
	uint64	GetUploadedPartial() const;
	uint64	GetUploadedFull() const;
	uint64	GetUploadedTotal() const;

//////////////////////////////////////////////////////////////////////////
// TRANSFER STATS - DOWNLOAD
	//functions to set/retrieve the downloaded data
	void	AddDownloaded(const uint64& bytes, const bool bPartial, const UINT srccount);
	uint64	GetDownloadedRare() const;
	uint64	GetDownloadedPartial() const;
	uint64	GetDownloadedFull() const;
	uint64	GetDownloadedTotal() const;

//////////////////////////////////////////////////////////////////////////
// BAD TRANSFER SESSIONS
	void	AddULSession(const bool bBad);
	void	AddDLSession(const bool bBad);
	bool	ShouldBanForBadDownloads() const;
	bool	ShouldntUploadForBadSessions() const;

//////////////////////////////////////////////////////////////////////////
// ANTI SPAM
	void	AddSpam();			//increases the spamcounter
	void	DecSpam();			//lowers the spamcounter
	int		GetSpams() const;	//returns the number of spams

//////////////////////////////////////////////////////////////////////////
// ANTI FAST REASK
	bool	IsFastAskClient() const;	//returns whether this client asks too fast
	void	AddReask(const UINT time);	//adds a reask and updates the avg time
	UINT	GetAvgReaskTime() const;		//returns the avg time
	UINT	GetReaskCount() const;			//returns the reask count

//////////////////////////////////////////////////////////////////////////
// ANTI XS-EXPLOIT/SPAM
	bool	IsXSSpammer() const;	//returns wether a client is considered a xs spammer
	void	IncFastXSCounter();		//increases the fast xs counter
	void	DecFastXSCounter();		//lowers the fast xs counter
	void	IncXSAsks();			//increases the counter of our xs requests
	void	IncXSAnsw();			//increases the number of answers that we received
	bool	IsXSExploiter() const;	//returns wether a client is considered a xs exploiter

//////////////////////////////////////////////////////////////////////////
// SCORE CALCULATION
	float	GetScore();		//returns the score of that client

//////////////////////////////////////////////////////////////////////////
// KNOW YOUR ENEMY
	bool	IsBadGuy() const;
	DWORD	GetLastSeen() const	{return m_pData->dwLastSeen;}	
	void	SetLastSeen()		{m_pData->dwLastSeen = time(NULL);}

	DWORD	GetFirstMet() const	{return m_pData->dwFirstMet;}	
	void	SetFirstMet()		{m_pData->dwFirstMet = time(NULL);}

//keep bad status for some time
	bool	IsBlockedByBadTimer() const	{return time(NULL) - m_pData->dwBadTimer < 7200;} // = 2h
	void	SetBadTimer()		{m_pData->dwBadTimer = time(NULL);}

	void	SetParent(CUpDownClient* client, const bool bSetOnly = false);
	bool	IsEmpty() const;
	void	WriteToFile(CFileDataIO* file);

	const uchar* GetKey() const					{return m_pData->abyKey;}
	void		ReCheckScore()					{m_bCheckScore = true;}

//this tells us the reasons for the score reduce :)
	CString	GetAntiLeechDataString() const;
	UINT	GetReaskTime() const				{return m_uiReaskTime;}
 //>>> CTempCAList - for the copy constructor
	CUpDownClient* GetParent() const			{return m_pParent;}
	float	GetLastScore() const				{return m_fLastScore;}
	bool	GetCheckScore() const				{return m_bCheckScore;}
	UINT	GetBadForThisSession() const		{return m_uiBadForThisSession;}
	bool	CheckBadFlag(const UINT i) const	{return (m_uiBadForThisSession & i) != 0;}
	CAntiLeechStruct* GetData() const			{return m_pData;}
	const CArray<CTag*>*	GetTagList() const	{return &taglist;}
 //<<< CTempCAList

private: 
	CUpDownClient* m_pParent;
	UINT	m_uiReaskTime;
	UINT	m_uiLastXS;
	void	EvalXS();
	bool	m_bBadWithoutULDL;
	float	m_fLastScore;
	bool	m_bCheckScore;
	UINT	m_uiBadForThisSession;
	CAntiLeechStruct*	m_pData;
	CArray<CTag*>	taglist;
};

class CTempCAList
{
public:
	CTempCAList();
	~CTempCAList();	
	
	void	SetParent(CUpDownClient* client);
	void	ResetParent(CUpDownClient* client);
	void	Verify(CUpDownClient* client);

private:
	CMap<CCKey, const CCKey&, CList<CAntiLeechData*>*, CList<CAntiLeechData*>* > m_DataList;
}; 

//////////////////////////////////////////////////////////////////////////
//LIST

class CAntiLeechDataList
{
public:
	CAntiLeechDataList();
	~CAntiLeechDataList();

	CAntiLeechData* GetData(const uchar* key);
	void	Process();

	UINT	GetBadActionCounter(const uint8 i) const		{return m_uiBadActions[i];}
	void	IncBadActionCounter(const uint8 i)				{++m_uiBadActions[i];}

//>>> CTempCAList
	void	SetParent(CUpDownClient* client);
	void	ResetParent(CUpDownClient* client);
	void	Verify(CUpDownClient* client);
//<<< CTempCAList

//>>> Store corrupt part senders
	void	AddCorruptPartSender(const uchar* pSender);
	bool	IsCorruptPartSender(const uchar* pClient);
	struct corruptPartSenderInfo
	{
		corruptPartSenderInfo()
		{
			md4clr(hash);
			dwAdded = time(NULL);
		}
		corruptPartSenderInfo(const uchar* thash, DWORD tdwAdded)
		{
			md4cpy(hash, thash);
			dwAdded = tdwAdded;
		}
		uchar	hash[16];
		UINT	dwAdded;
	};
//<<< Store corrupt part senders
protected:
	bool	LoadList(const CString& path = L"");
	void	SaveList();

private:
	CTempCAList		m_TempList; //>>> CTempCAList
	CMap<CCKey, const CCKey&, corruptPartSenderInfo*, corruptPartSenderInfo*>	m_mapBadClients; //>>> Store corrupt part senders
	CMap<CCKey, const CCKey&, CAntiLeechData*, CAntiLeechData*> m_mapClients;
	DWORD			m_dwLastSaved;
	UINT			m_uiBadActions[BAD_ACTION_COUNTER]; 
};
#endif