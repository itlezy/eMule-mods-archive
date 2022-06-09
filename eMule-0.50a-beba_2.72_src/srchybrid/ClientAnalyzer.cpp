// Tux: Feature: Client Analyzer [start]
//////////////////////////////////////////////////////////////////////////
//
//this file is part of eMule
//Copyright (C)2002-2010 WiZaRd (thewizardofdos@gmail.com)
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


//////////////////////////////////////////////////////////////////////////
//About ClientAnalyzer:
//---------------------
//The purpose of this class is to provide an interface that is controlling all 
//kinds of clients actions in eMule and to provide a score determined by the actions of each client
//That way, Modders won't need to update an "antileech" database file regularly with
//strings/opcodes or other things that are loose guesses about a client but just implement
//that class so leechers will be detected by their ACTIONS
//
//
//A few examples:
//---------------
//
//A releaser uses a mod with kick/ban/PS on partfiles and so on - would/should you ban him?
//Yes you WOULD with default methods but no, you SHOULD not ban that client!
//He is RELEASING, he is supporting the network and increasing its health...
//But as you can't distinguish between "good" clients using that mod (aka releases) 
//and "bad" clients using the same mod (aka leechers) you would ban him/punish him on sight
//(though the latter should be forbidden iMHO - still waiting for a statement by the devs)
//
//A client uses a leecher mod by a modder with a brain of > half the size of a walnut,
//which means he won't send a string/opcode/something to identify himself - what to do?
//Default methods fail miserably in such cases... and the default CS is too exploitable
//
//A client uses a leecher mod which spams the network - what to do?
//Using default methods you are required to release a new filter file over and over again...
//The class will use the user editable blocklist and remember good and bad messages automatically
//
//
//So, a final conclusion:
//-----------------------
//The class will NOT (NEVER!) need any updates in its final state!
//The class will AUTOMATICALLY collect and use data from each client!
//The class will REMEMBER that data and store it in a antileech.met file!
//The class will work ONLY by analyzing the BEHAVIOUR of a client to us! 
//That means it is ABSOLUTELY following the rules and maybe even worth to be used officially!
//
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// CHANGELOG
//////////////////////////////////////////////////////////////////////////
//
//v1.0:		first version as discussed on emfuture and other boards
//
//v1.1:		2 changes: 
//-			UL/DL can only lower the score to 1/2 of the base score [SEPPL]
//-			No boost for higher reask times [CICCIOBASTARDO]
//
//v1.2:		2 BuGFiXes:
//-			ReaskTime was not evaluated and assigned correctly
//-			ReaskTime punisher was not calculated by MINUTES but by SECONDS!
//
//v1.3:		8 changes / 2 BuGFiXes:
//-			XS evaluation now uses the official system though slightly changed
//-			XS asks will only be counted for not too rare files
//-			We will also answer to XS exploiters to prevent entering a deadlock situation
//			when both clients will think the other one is an exploiter and noone will send anymore
//-			EVERY bad action now reduces the score - this will punish VERY bad clients harder
//-			Slow XS asks won't stack up credits [CICCIOBASTARDO]
//-			BuGFiX: fix in default initialization
//-			BuGFiX: the first connect/reask was not counted (reaskcounter = 0) but avgtime was set
//-			Added a "bad action counter" - for Devs and interested... not evaluated, yet
//-			Major change: rare file UL/DL is counted separately and weighted differently
//-			Major change: the "first contact" is saved and a client gets a bonus for every week 
//						  he keeps his hash to encourage users to keep it and to punish hashchangers
//v1.4:		16 changes / 1 BuGFiX
//-			Credits are merged if no data is present
//-			Force one score calculation on startup
//-			New function "IsBadGuy()" - can be used to draw an icon or something...
//-			Made some functions non-static using the parent pointer
//-			Only use the UL/DL treshold as a punisher for clients that have something for us
//-         Fast Reask treshold will only be used after collecting 3 reasks
//-         XS spam/exploit check changed due to buggy implementation (***/WiZ)
//-			XS exploiters will only be checked after 10 reasks
//-			Punishment for Nick- and/or ModThieves reduced to 3*AT_MIDPUNISH
//-			Combined punishment for UDP-FNF and/or FileFakers
//-			Changed logging to an uniform standard
//-			Added missing initialisation for the badaction counter
//-			tweaked some functions to not call ReCheckScore() if it wasn't needed
//-			replaced the MEMSET command by md4clr for the hashkey
//-			Major change: met header changed because 64 bit size data is now written to a single tag
//-			changed some types to more appropriate ones
//-			BuGFiX: fixed the incorrect badaction counter (crash on exit)
//v1.5:		5 changes / 1 BuGFiX
//-			several small changes and optimizations
//-			added a new functionality! clients that are banned for sending corrupt parts by eMule are now 
//			stored for 60 days and banned on sight
//-			added another modthief check
//-			added a timer so a client cannot change his "bad" state for at least 2 hours to prevent ppl 
//			from avoiding punishment by resending different data after they detected a CA client
//-			"Modfaker" detection/punishment added (zz_fly), these clients fall into the same category as modthieves and nickthieves
//-			AT_FULLFILEWEIGHT and AT_PARTFILEWEIGHT were both raised to 1.5 (1.0 had NO effect so far as I was just testing)
//-			added additional modthief detection schemes (some taken from Xtreme DLP v38)
//
//////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "emule.h"
#include "ClientAnalyzer.h"
#include "Preferences.h"
#include "SafeFile.h"
#include "Opcodes.h"
#include "Packets.h"	
#include "emuledlg.h"
#include "Log.h"
#include "updownclient.h"
#include "Version.h"
#include "ClientCredits.h"
#include "PartFile.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define chunksDL uint64( (double)GetDownloadedTotal()/PARTSIZE + AT_ROUNDOFFSET )
#define chunksUL uint64( (double)GetUploadedTotal()/PARTSIZE + AT_ROUNDOFFSET )
#define dwWeeks (time(NULL) - GetFirstMet()) / 604800

UINT	GetPrimeNumber(const UINT min)
{
	UINT foundPrime = min > 0 ? min-1 : 0;
	bool prime = false;
	while(!prime)
	{
		++foundPrime;
		prime = true;
		for(int cur_i = 2, end = (int)sqrt((double)foundPrime); cur_i <= end; ++cur_i)
		{
			if(foundPrime % cur_i == 0)
			{
				prime = false;
				break;
			}
		}
	}
	return foundPrime;
}

void AppendWithBlank(CString& str, const CString& strApp)
{
	str.Append(str.IsEmpty() ? strApp : (L" " + strApp));
}

CString GetReasonString(const UINT i)
{
	CString reason = L"";

	if(i & AT_NICKTHIEF)
		AppendWithBlank(reason, L"NickThief");
	if(i & AT_MODTHIEF)
		AppendWithBlank(reason, L"ModThief");
	if(i & AT_FILEFAKER)
		AppendWithBlank(reason, L"FileFaker");
	if(i & AT_UDPFNFFAKER)
		AppendWithBlank(reason, L"UDPFNFFaker");
	if(i & AT_MODFAKER)
		AppendWithBlank(reason, L"ModFaker");

	return reason;
}

CAntiLeechData::CAntiLeechData(CFileDataIO* file)
{
	Init();
    
	file->ReadHash16(m_pData->abyKey);
	UINT tagcount = file->ReadUInt32();
	for (UINT j = 0; j < tagcount; ++j)
	{
		CTag* newtag = new CTag(file, false);
		bool bDel = true;
		switch(newtag->GetNameID())
		{
//>>> RARE files
			case AT_UPLOAD_RARE:
			{
				if (newtag->IsInt64(false))
					m_pData->uSentBytesRare = newtag->GetInt64();
				else if (newtag->IsInt())
					((UINT*)&m_pData->uSentBytesRare)[0] = newtag->GetInt();
				break;
			}
			case AT_UPLOAD_RARE_HI:
			{
				if (newtag->IsInt())
					((UINT*)&m_pData->uSentBytesRare)[1] = newtag->GetInt();
				break;
			}
			case AT_DOWNLOAD_RARE:
			{
				if (newtag->IsInt64(false))
					m_pData->uGotBytesRare = newtag->GetInt64();
				else if (newtag->IsInt())
					((UINT*)&m_pData->uGotBytesRare)[0] = newtag->GetInt();
				break;
			}
			case AT_DOWNLOAD_RARE_HI:
			{
				if (newtag->IsInt())
					((UINT*)&m_pData->uGotBytesRare)[1] = newtag->GetInt();
				break;
			}
//<<< RARE files
			case AT_UPLOAD_PART:
			{
				if (newtag->IsInt64(false))
					m_pData->uSentBytesPartial = newtag->GetInt64();
				else if (newtag->IsInt())
					((UINT*)&m_pData->uSentBytesPartial)[0] = newtag->GetInt();
				break;
			}
			case AT_UPLOAD_PART_HI:
			{
				if (newtag->IsInt())
					((UINT*)&m_pData->uSentBytesPartial)[1] = newtag->GetInt();
				break;
			}
			case AT_UPLOAD:
			{
				if (newtag->IsInt64(false))
					m_pData->uSentBytes = newtag->GetInt64();
				else if (newtag->IsInt())
					((UINT*)&m_pData->uSentBytes)[0] = newtag->GetInt();
				break;
			}
			case AT_UPLOAD_HI:
			{
				if (newtag->IsInt())
					((UINT*)&m_pData->uSentBytes)[1] = newtag->GetInt();
				break;
			}


			case AT_DOWNLOAD_PART:
			{
				if (newtag->IsInt64(false))
					m_pData->uGotBytesPartial = newtag->GetInt64();
				else if (newtag->IsInt())
					((UINT*)&m_pData->uGotBytesPartial)[0] = newtag->GetInt();
				break;
			}
			case AT_DOWNLOAD_PART_HI:
			{
				if (newtag->IsInt())
					((UINT*)&m_pData->uGotBytesPartial)[1] = newtag->GetInt();
				break;
			}
			case AT_DOWNLOAD:
			{
				if (newtag->IsInt64(false))
					m_pData->uGotBytes = newtag->GetInt64();
				else if (newtag->IsInt())
					((UINT*)&m_pData->uGotBytes)[0] = newtag->GetInt();
				break;
			}
			case AT_DOWNLOAD_HI:
			{
				if (newtag->IsInt())
					((UINT*)&m_pData->uGotBytes)[1] = newtag->GetInt();
				break;
			}

			
			case AT_LASTSEEN:
			{
				if (newtag->IsInt())
					m_pData->dwLastSeen = newtag->GetInt();
				break;
			}
			case AT_FIRSTMET:
			{
				if (newtag->IsInt())
					m_pData->dwFirstMet = newtag->GetInt();
				break;
			}
//keep bad status for some time
			case AT_BADTIMER:
			{
				if(newtag->IsInt())
					m_pData->dwBadTimer = newtag->GetInt();
				break;
			}
			case AT_BADSTATUS:
			{
				if(newtag->IsInt())
					m_uiBadForThisSession = newtag->GetInt();
				break;
			}
//keep bad status for some time

			case AT_REASKS:
			{
				if(newtag->IsInt())
					m_pData->uReasks = newtag->GetInt();
				break;
			}
			case AT_AVGREASK:
			{
				if(newtag->IsInt())
					m_pData->uAvgReaskTime = newtag->GetInt();
				break;
			}
			case AT_SPAMS:
			{
				if(newtag->IsInt())
					m_pData->iBlockedMessages = newtag->GetInt();
				break;
			}
			case AT_XS_ASKS:
			{
				if(newtag->IsInt())
					m_pData->uXSAsks = newtag->GetInt();
				break;
			}
			case AT_XS_ANSW:
			{
				if(newtag->IsInt())
					m_pData->uXSAnsw = newtag->GetInt();
				break;
			}
			case AT_FASTXS_ASKS:
			{
				if(newtag->IsInt())
					m_pData->iFastXSAsks = newtag->GetInt();
				break;
			}
			
			case AT_BADULSESSIONS:
			{
				if(newtag->IsInt())
					m_pData->uBadULSessions = (uint8)newtag->GetInt();
				break;
			}
			case AT_BADDLSESSIONS:
			{
				if(newtag->IsInt())
					m_pData->uBadDLSessions = (uint8)newtag->GetInt();
				break;
			}

			default:
			{
				bDel = false;
				break;
			}
		}
		if(bDel)
			delete newtag;
		else
			taglist.Add(newtag); //hmmm - strange tags? might be new versions
	}

	//Failsafe:
	if(m_pData->uAvgReaskTime == 0)
		m_pData->uReasks = 0;

	ReCheckScore(); //perform the first check
}

CAntiLeechData::CAntiLeechData(const uchar* key)
{
	Init();
	md4cpy(m_pData->abyKey, key);
}

//>>> CTempCAList
CAntiLeechData::CAntiLeechData(const CAntiLeechData* toCopy)
{
	m_pParent = toCopy->GetParent();
	m_uiReaskTime = toCopy->m_uiReaskTime;
	m_uiLastXS = toCopy->m_uiLastXS;
	m_bBadWithoutULDL = toCopy->m_bBadWithoutULDL;
	m_fLastScore = toCopy->m_fLastScore;
	m_bCheckScore = toCopy->m_bCheckScore;
	m_uiBadForThisSession = toCopy->m_uiBadForThisSession;
	m_pData = new CAntiLeechStruct;	
	memcpy(m_pData, toCopy->m_pData, sizeof(CAntiLeechStruct));
	for(int i = 0; i < toCopy->taglist.GetCount(); ++i)
		taglist.Add(toCopy->taglist[i]);
}

void CAntiLeechData::Merge(const CAntiLeechData* toMerge)
{
// 	//this should *ONLY* be called ONCE and for the original entry, i.e. the m_pParent should always be NULL here
// 	ASSERT(m_pParent == NULL);
 	m_pParent = toMerge->GetParent();
	m_uiReaskTime = toMerge->m_uiReaskTime;
	m_uiLastXS = toMerge->m_uiLastXS;
	m_bBadWithoutULDL = toMerge->m_bBadWithoutULDL;
	m_fLastScore = toMerge->m_fLastScore;
	m_bCheckScore = toMerge->m_bCheckScore;
	m_uiBadForThisSession = toMerge->m_uiBadForThisSession;
	memcpy(m_pData, toMerge->m_pData, sizeof(CAntiLeechStruct));

	for(int i = 0; i < taglist.GetCount(); ++i)
		delete taglist[i];
	taglist.RemoveAll();
	for(int i = 0; i < toMerge->taglist.GetCount(); ++i)
		taglist.Add(toMerge->taglist[i]);
}
//<<< CTempCAList

CAntiLeechData::~CAntiLeechData()
{
	for(int i = 0; i < taglist.GetCount(); ++i)
		delete taglist[i];
	taglist.RemoveAll();

	delete m_pData;
}

void CAntiLeechData::Init()
{
	//create a new datastruct and be sure to reset all values
	m_pData = new CAntiLeechStruct;	
	memset(m_pData, 0, sizeof(CAntiLeechStruct));
	md4clr(m_pData->abyKey);
	m_uiBadForThisSession = 0;
	m_fLastScore = 1.0f;
	SetLastSeen();
	SetFirstMet();
	m_pParent = NULL;
	m_uiReaskTime = FILEREASKTIME;
	m_uiLastXS = 0;
	m_bBadWithoutULDL = false;
	ReCheckScore();	
}

bool CAntiLeechData::IsEmpty() const
{
	//even if we have no data, we save up to 150 days!
	return	
		GetFirstMet() < (DWORD)(time(NULL) - 12960000) // today - 150 days
		&& !IsBlockedByBadTimer() //keep bad status for some time
//v1.6: obviously we save too much data... so we should have different parameter to check if we want to keep a client
//1. save bad guys in any case!
		&& !IsBadGuy()
		&& !IsFastAskClient()
		&& GetSpams() == 0
		&& !IsXSExploiter()
		&& !IsXSSpammer()
		&& !ShouldBanForBadDownloads()
		&& !ShouldntUploadForBadSessions()
//2. save clients that we had some transfer with
		&& m_pData->uSentBytes < PARTSIZE 
		&& m_pData->uGotBytes < PARTSIZE;
// 		&& m_pData->uReasks == 0
// 		&& m_pData->iBlockedMessages < 0
// 		&& m_pData->uXSAsks == 0
// 		&& m_pData->uXSAnsw == 0
// 		&& m_pData->iFastXSAsks < 0
// 		&& m_pData->uBadULSessions == 0
// 		&& m_pData->uBadDLSessions == 0
// 		&& m_pData->uSentBytes == 0
// 		&& m_pData->uGotBytes == 0);
}

void CAntiLeechData::WriteToFile(CFileDataIO* file) 
{
	file->WriteHash16(m_pData->abyKey);

	UINT uTagCount = 0;
	ULONG uTagCountFilePos = (ULONG)file->GetPosition();
	file->WriteUInt32(uTagCount);

	// standard tags
	if (m_pData->uSentBytesRare) 
	{
		if(CTag(AT_UPLOAD_RARE, m_pData->uSentBytesRare, true).WriteTagToFile(file))
			++uTagCount;
	}
	if (m_pData->uSentBytesPartial) 
	{
		if(CTag(AT_UPLOAD_PART, m_pData->uSentBytesPartial, true).WriteTagToFile(file))
			++uTagCount;
	}
	if(m_pData->uSentBytes)
	{
		if(CTag(AT_UPLOAD, m_pData->uSentBytes, true).WriteTagToFile(file))
			++uTagCount;
	}

	if (m_pData->uGotBytesRare) 
	{
		if(CTag(AT_DOWNLOAD_RARE, m_pData->uGotBytesRare, true).WriteTagToFile(file))
			++uTagCount;
	}
	if (m_pData->uGotBytesPartial) 
	{
		if(CTag(AT_DOWNLOAD_PART, m_pData->uGotBytesPartial, true).WriteTagToFile(file))
			++uTagCount;
	}
	if (m_pData->uGotBytes) 
	{
		if(CTag(AT_DOWNLOAD, m_pData->uGotBytes, true).WriteTagToFile(file))
			++uTagCount;
	}


	if(CTag(AT_LASTSEEN, m_pData->dwLastSeen).WriteTagToFile(file))
		++uTagCount;
	if(CTag(AT_FIRSTMET, m_pData->dwFirstMet).WriteTagToFile(file))
		++uTagCount;
//keep bad status for some time
	if(m_pData->dwBadTimer != 0)
	{
		if(CTag(AT_BADTIMER, m_pData->dwBadTimer).WriteTagToFile(file))
			++uTagCount;
	}
	if(CTag(AT_BADSTATUS, m_uiBadForThisSession).WriteTagToFile(file))
		++uTagCount;
//keep bad status for some time

	if(m_pData->uReasks) //don't save "unneeded" tags
	{
		if(CTag(AT_REASKS, m_pData->uReasks).WriteTagToFile(file))
			++uTagCount;

		if(CTag(AT_AVGREASK, m_pData->uAvgReaskTime).WriteTagToFile(file))
			++uTagCount;
	}
	if(m_pData->iBlockedMessages > 0) //don't save "unneeded" tags
	{
		if(CTag(AT_SPAMS, m_pData->iBlockedMessages).WriteTagToFile(file))
			++uTagCount;
	}
	if(m_pData->uXSAsks > m_pData->uXSAnsw) //don't save "unneeded" tags
	{
		if(CTag(AT_XS_ASKS, m_pData->uXSAsks).WriteTagToFile(file))
			++uTagCount;

		if(CTag(AT_XS_ANSW, m_pData->uXSAnsw).WriteTagToFile(file))
			++uTagCount;
	}
	if(m_pData->iFastXSAsks > 0) //don't save "unneeded" tags
	{
		if(CTag(AT_FASTXS_ASKS, m_pData->iFastXSAsks).WriteTagToFile(file))
			++uTagCount;
	}

	if(m_pData->uBadULSessions) //don't save "unneeded" tags
	{
		if(CTag(AT_BADULSESSIONS, m_pData->uBadULSessions).WriteTagToFile(file))
			++uTagCount;
	}

	if(m_pData->uBadDLSessions) //don't save "unneeded" tags
	{
		if(CTag(AT_BADDLSESSIONS, m_pData->uBadDLSessions).WriteTagToFile(file))
			++uTagCount;
	}

	// unidentified tags (for future versions)
	for (int j = 0; j < taglist.GetCount(); ++j) 
	{
		if(taglist[j]->WriteTagToFile(file))
			++uTagCount;
	}

	file->Seek(uTagCountFilePos, CFile::begin);
	file->WriteUInt32(uTagCount);
	file->Seek(0, CFile::end);
}


//////////////////////////////////////////////////////////////////////////
// TRANSFER STATS - UPLOAD
void CAntiLeechData::AddUploaded(const uint64& bytes, const bool bPartial, const UINT srccount) 
{
	if(bPartial)
		m_pData->uSentBytesPartial += bytes;
	if(srccount < AT_RAREFILE)
		m_pData->uSentBytesRare += bytes;
	m_pData->uSentBytes += bytes;
	ReCheckScore();
}

uint64  CAntiLeechData::GetUploadedRare() const
{
	return m_pData->uSentBytesRare;
}

uint64	CAntiLeechData::GetUploadedPartial() const
{
	return m_pData->uSentBytesPartial;
}

uint64	CAntiLeechData::GetUploadedFull() const
{
	return m_pData->uSentBytes - m_pData->uSentBytesPartial;
}

//THIS is the data WE SENT! 
//Rare data is weighted differently as is partial data as we can expect something for that in return!
uint64	CAntiLeechData::GetUploadedTotal() const
{
	return GetUploadedFull() 
		+ uint64(AT_PARTFILEWEIGHT * m_pData->uSentBytesPartial)
//As mentioned by James R. Bath we shouldn't "punish" downloaders of rare files... basically they'd help spreading those files
//may be changed later again, though...
		/*+ uint64(AT_RARESHAREWEIGHT * m_pData->uSentBytesRare)*/;
}


//////////////////////////////////////////////////////////////////////////
// TRANSFER STATS - DOWNLOAD
void CAntiLeechData::AddDownloaded(const uint64& bytes, const bool bPartial, const UINT srccount) 
{
	if(bPartial)
		m_pData->uGotBytesPartial += bytes;
	if(srccount < AT_RAREFILE)
		m_pData->uGotBytesRare += bytes;
	m_pData->uGotBytes += bytes;
	ReCheckScore();
}

uint64	CAntiLeechData::GetDownloadedRare() const
{
	return m_pData->uGotBytesRare;
}

uint64	CAntiLeechData::GetDownloadedPartial() const
{
	return m_pData->uGotBytesPartial;
}

uint64	CAntiLeechData::GetDownloadedFull() const
{
	return m_pData->uGotBytes - m_pData->uGotBytesPartial;
}

//THIS is the data WE GOT! 
//Rare data is weighted differently TWICE as is FULL data as we should pay back such nice behavior!
uint64	CAntiLeechData::GetDownloadedTotal() const
{
	//TODO: rare stuff!
	return uint64(AT_FULLFILEWEIGHT * GetDownloadedFull()) 
		+ m_pData->uGotBytesPartial
		+ uint64(AT_RARESHAREWEIGHT * m_pData->uGotBytesRare);
}

//////////////////////////////////////////////////////////////////////////
// BAD TRANSFER SESSIONS
//tracks the SUCCESSIVE failed UL/DL attempts
void	CAntiLeechData::AddULSession(const bool bBad)
{
	if(bBad)
	{
		if(m_pData->uBadULSessions < 255)
		++m_pData->uBadULSessions;
	}
	else
		m_pData->uBadULSessions = 0;
}

void	CAntiLeechData::AddDLSession(const bool bBad)
{
	if(bBad)
	{
		if(m_pData->uBadDLSessions < 255)
		++m_pData->uBadDLSessions;
	}
	else
		m_pData->uBadDLSessions = 0;
}

bool	CAntiLeechData::ShouldntUploadForBadSessions() const
{
	return m_pData->uBadULSessions > AT_BADUPLOADSTRES;
}

bool	CAntiLeechData::ShouldBanForBadDownloads() const
{
	return m_pData->uBadDLSessions > AT_BADDOWNLOADSTRES;
}

//////////////////////////////////////////////////////////////////////////
// ANTI SPAM
void	CAntiLeechData::AddSpam()		
{
	++m_pData->iBlockedMessages;
	if(m_pParent && thePrefs.GetLogAnalyzerEvents())
		theApp.QueueDebugLogLineEx(LOG_WARNING, L"Analyzer: Spamcounter for (%s) increased: %i", m_pParent->DbgGetClientInfo(), m_pData->iBlockedMessages);
	theApp.antileechlist->IncBadActionCounter(BAD_ACTION_SPAMS);
	ReCheckScore();
}

void	CAntiLeechData::DecSpam()		
{
	//we do not have to recheck the score if this guys isn't punished, yet
	//counter-1 = 0 -> no spam :)
	//counter-1 > 0 -> spammer, but reduced - check needed
	const bool bCheckNeeded = m_pData->iBlockedMessages > 0; 
	--m_pData->iBlockedMessages;
	//if(m_pParent && thePrefs.GetLogAnalyzerEvents())
	//	theApp.QueueDebugLogLineEx(LOG_INFO, L"Analyzer: Spamcounter for (%s) decreased: %i", m_pParent->DbgGetClientInfo(), m_pData->iBlockedMessages);
	if(bCheckNeeded)
		ReCheckScore();
}

int		CAntiLeechData::GetSpams() const
{
	return m_pData->iBlockedMessages;
}

//////////////////////////////////////////////////////////////////////////
// ANTI FAST REASK
bool	CAntiLeechData::IsFastAskClient() const
{
	return GetReaskCount() > 2 
		&& GetAvgReaskTime()
		&& (GetAvgReaskTime() + AT_REASKBUFFER) < m_uiReaskTime;
}

void	CAntiLeechData::AddReask(const UINT time)	
{
	const bool bBadTimeDiff1 = (GetAvgReaskTime() + AT_REASKBUFFER) < m_uiReaskTime;
	m_pData->uAvgReaskTime = ((m_pData->uAvgReaskTime*m_pData->uReasks)+time)/(++m_pData->uReasks);
	const bool bBadTimeDiff2 = (GetAvgReaskTime() + AT_REASKBUFFER) < m_uiReaskTime;
	if(m_pParent && thePrefs.GetLogAnalyzerEvents() && bBadTimeDiff1 != bBadTimeDiff2) //log only if needed
		theApp.QueueDebugLogLineEx(LOG_WARNING, L"Analyzer: Reask for (%s) updated: %i reasks @ %s avg", m_pParent->DbgGetClientInfo(), m_pData->uReasks, CastSecondsToHM(m_pData->uAvgReaskTime/1000));	
	if(time + AT_REASKBUFFER < m_uiReaskTime) //FiX too many "fast reask" entries in stat, only count the really bad ones :)
		theApp.antileechlist->IncBadActionCounter(BAD_ACTION_FASTREASK);
	ReCheckScore();
}

UINT	CAntiLeechData::GetAvgReaskTime() const	
{
	return m_pData->uAvgReaskTime;
}

UINT	CAntiLeechData::GetReaskCount() const	
{
	return m_pData->uReasks;
}

//////////////////////////////////////////////////////////////////////////
// ANTI XS-EXPLOIT/SPAM
bool	CAntiLeechData::IsXSSpammer() const
{
	return m_pData->iFastXSAsks > AT_XSSPAM_LEVEL;
}

void	CAntiLeechData::IncFastXSCounter()
{
	++m_pData->iFastXSAsks;
	if(m_pParent && thePrefs.GetLogAnalyzerEvents())
		theApp.QueueDebugLogLineEx(LOG_WARNING, L"Analyzer: FastXS for (%s) increased: %i", m_pParent->DbgGetClientInfo(), m_pData->iFastXSAsks);
	theApp.antileechlist->IncBadActionCounter(BAD_ACTION_FASTXS);
	ReCheckScore();
	EvalXS();
}

void	CAntiLeechData::DecFastXSCounter()
{
	//we do not have to recheck the score if this guy isn't punished, yet
	//counter-1 = 0 -> no fast XS anymore :)
	//counter-1 > 0 -> fast XS :(- check needed
	//[CICCIOBASTARDO] - Don't stack up credits for slow XS asks
	if(m_pData->iFastXSAsks > 0)
	{
		--m_pData->iFastXSAsks;
		//if(m_pParent && thePrefs.GetLogAnalyzerEvents())
		//	theApp.QueueDebugLogLineEx(LOG_INFO, L"Analyzer: FastXS for (%s) decreased: %i", m_pParent->DbgGetClientInfo(), m_pData->iFastXSAsks);
		ReCheckScore();
	}
	EvalXS();
}

void CAntiLeechData::EvalXS()
{
	if(m_pParent == NULL)
		return;

	const DWORD dwNow = ::GetTickCount();
	if(m_uiLastXS)
		theApp.QueueDebugLogLineEx(LOG_INFO, L"Analyzer: Received XS request after %s from %s - fastXS count: %u", CastSecondsToHM((dwNow - m_uiLastXS)/1000), m_pParent->DbgGetClientInfo(), m_pData->iFastXSAsks);
	else
		theApp.QueueDebugLogLineEx(LOG_INFO, L"Analyzer: Received first XS in this session from %s", m_pParent->DbgGetClientInfo());
	m_uiLastXS = dwNow;
}

void	CAntiLeechData::IncXSAsks()	
{
	const bool b = IsXSExploiter();
	++m_pData->uXSAsks;	
	//if(m_pParent && thePrefs.GetLogAnalyzerEvents())
	//	theApp.QueueDebugLogLineEx(LOG_INFO, L"XS Asks for (%s) increased: %i", m_pParent->DbgGetClientInfo(), m_pData->uXSAsks);
	if(b != IsXSExploiter())
	{
		if(m_pParent && thePrefs.GetLogAnalyzerEvents())
			theApp.QueueDebugLogLineEx(LOG_ERROR, L"Analyzer: (%s) reached XS exploiter level: %u : %u", m_pParent->DbgGetClientInfo(), m_pData->uXSAsks, m_pData->uXSAnsw);
		ReCheckScore();
	}
}

void	CAntiLeechData::IncXSAnsw()	
{
	const bool b = IsXSExploiter();
	++m_pData->uXSAnsw;
	//if(m_pParent && thePrefs.GetLogAnalyzerEvents())
	//	theApp.QueueDebugLogLineEx(LOG_INFO, L"Analyzer: XS Answs for (%s) increased: %i", m_pParent->DbgGetClientInfo(), m_pData->uXSAnsw);
	if(b != IsXSExploiter())
	{
		if(m_pParent && thePrefs.GetLogAnalyzerEvents())
			theApp.QueueDebugLogLineEx(LOG_SUCCESS, L"Analyzer: (%s) left XS exploiter level: %u : %u", m_pParent->DbgGetClientInfo(), m_pData->uXSAsks, m_pData->uXSAnsw);
		ReCheckScore();
	}
}

bool	CAntiLeechData::IsXSExploiter()	const
{
	return (m_pData->uXSAsks > AT_XSEXPLOIT_LEVEL && m_pData->uXSAsks > AT_XSEXPLOIT_PERCENT*m_pData->uXSAnsw);
}

//////////////////////////////////////////////////////////////////////////
// SCORE CALCULATION
bool	CAntiLeechData::IsBadGuy() const	
{
	if(/*thePrefs.IsExcludeULDLBadGuys() &&*/ !m_bBadWithoutULDL)
		return false;
	return m_fLastScore < 1.0f;
//	return m_fLastScore < 1.0f 
//		|| m_uiBadForThisSession != 0;
}

float CAntiLeechData::GetScore()
{
	if(!m_bCheckScore)
		return m_fLastScore;
	m_bCheckScore = false;

	if(m_pParent == NULL)
		return 0; //???

	float cur_score = AT_BASESCORE;

	if(GetSpams() > 0)
		cur_score -= GetSpams() * AT_LOWPUNISH;
	if(IsXSSpammer())
		cur_score -= m_pData->iFastXSAsks * AT_LOWPUNISH;
	if(IsXSExploiter())
		cur_score -= AT_HIPUNISH;

	if(GetReaskCount() > 2 && GetAvgReaskTime())
	{
		//BuGFiX: e.g. 22min avg. + 5 = 27 - 29 = -2 --> -2*2.5f punishment
//		const float fTimeDiff = (GetAvgReaskTime() + AT_REASKBUFFER - m_uiReaskTime)/60000.0f; //BuGFiX: how could I miss that!? :confused:
		const UINT fTimeDiff = GetAvgReaskTime() + AT_REASKBUFFER;
		//[CICCIOBASTARDO] - Don't reward a higher reask
		if(fTimeDiff < m_uiReaskTime)
		//idea, just written down... punish clients who asked in such short times OFTEN harder than others!
		//	cur_score += fTimeDiff * AT_LOWPUNISH;
		//ok let's try that idea :)
			cur_score -= GetReaskCount() * AT_LOWPUNISH; //FiX v1.5 [gidi]
	}

	//really bad!
	if(m_uiBadForThisSession != 0)
	{
		//NickThieves and ModThieves are commonly used... and useless
		//but pretty often used both in one mod, so don't punish them too hard
		if(CheckBadFlag(AT_NICKTHIEF | AT_MODTHIEF | AT_MODFAKER))
			cur_score -= 3*AT_MIDPUNISH; //= 3 chunks needed to balance
		//this is also a pretty common combination... but it is much worse so punish them harder
		if(CheckBadFlag(AT_FILEFAKER | AT_UDPFNFFAKER))
			cur_score -= AT_HIPUNISH; //= 5 chunks needed to balance
	}

	//Spike2: decrease score after 3 failed DL attempts
	//this is for clients (e.g. AJ) that grant us an upload slot but do not send any data
	if(m_pData->uBadDLSessions > 2)
		cur_score -= m_pData->uBadDLSessions*AT_MIDPUNISH; //= 3..5 chunks needed to balance

	//[SEPPL] - Don't let the score fall too low because of UL/DL
	m_bBadWithoutULDL = cur_score < AT_BASESCORE;
	if(!m_pParent->GetRequestFile() || m_pParent->GetDownloadState() > DS_ONQUEUE)
	{
		//Keep an even score with clients who do not have something for us
		// 		if(cur_score < AT_BASESCORE)
		// 			cur_score = AT_BASESCORE;
		; //nothing to do ;)
	}
	else
	{
		//VOODOO 2 DO :)
		//number of chunks we downloaded	
		cur_score += chunksDL * AT_MIDPUNISH;
		//number of chunks we upped
		cur_score -= chunksUL * AT_MIDPUNISH;

		//number of weeks we know this client
		cur_score += dwWeeks * AT_LOWPUNISH;

		//[WiZaRd] - but only for clients who can't "pay back"
		// 		if(cur_score < AT_BASESCORE / 2)
		// 			cur_score = AT_BASESCORE / 2;
	}

	//Cap the score
	if(cur_score < AT_MINSCORE)
		cur_score = AT_MINSCORE;
	else if(cur_score > AT_MAXSCORE)
		cur_score = AT_MAXSCORE;

	m_fLastScore = cur_score;
	return m_fLastScore;
}

void CAntiLeechData::SetBadForThisSession(const UINT i, const CString& strDetail)
{	
	SetBadTimer(); //keep bad status for some time
	if((m_uiBadForThisSession&i) != 0)
		return; //we already logged that event so there is no need to proceed

	switch(i)
	{
		case AT_NICKTHIEF:
			theApp.antileechlist->IncBadActionCounter(BAD_ACTION_NICKTHIEF);
			break;

		case AT_MODTHIEF:
			theApp.antileechlist->IncBadActionCounter(BAD_ACTION_MODTHIEF);
			break;
	
		case AT_FILEFAKER:
			theApp.antileechlist->IncBadActionCounter(BAD_ACTION_FILEFAKE);
			break;

		case AT_UDPFNFFAKER:
			theApp.antileechlist->IncBadActionCounter(BAD_ACTION_UDPFNF);
			break;
		
		case AT_MODFAKER:
			theApp.antileechlist->IncBadActionCounter(BAD_ACTION_MODFAKER);
			break;
		
		default:
			ASSERT(0); //this should not happen...
			return;
	}
	
	m_uiBadForThisSession |= i;
	if(m_pParent)
	{
#ifdef _DEBUG
		; //no condition here
#else
		if(thePrefs.GetLogAnalyzerEvents())
#endif
		{
			CString reason = GetReasonString(i);
			if(!strDetail.IsEmpty())
				reason.AppendFormat(L" - %s", strDetail);
			theApp.QueueDebugLogLineEx(LOG_ERROR, L"Analyzer: Marked as %s: (%s)", reason, m_pParent->DbgGetClientInfo());
		}

		if(AdditionalModThiefCheck(i))
			return; //detected and updated the flag&timer - no need to recheck the score twice
	}

	ReCheckScore();
}

void CAntiLeechData::ClearBadForThisSession(const UINT i)
{
	if((m_uiBadForThisSession & i) == 0)
		return; //flag not  set, quick return

	CString reason = GetReasonString(i);

	//keep bad status for some time
	if(IsBlockedByBadTimer())
	{
		if(thePrefs.GetLogAnalyzerEvents())
			theApp.QueueDebugLogLineEx(LOG_ERROR, L"Analyzer: client %s was NOT cleared from %s state (blocked by timer)", m_pParent ? m_pParent->DbgGetClientInfo() : L"NULL", reason);
		return;
	}

	if(m_pParent)
	{
#ifdef _DEBUG
		if(m_pParent)
#else
		if(m_pParent && thePrefs.GetLogAnalyzerEvents())
#endif
			theApp.QueueDebugLogLineEx(LOG_SUCCESS, L"Analyzer: Cleared from %s state: (%s)", reason, m_pParent->DbgGetClientInfo());
	}
	m_uiBadForThisSession &= ~i;
	ReCheckScore();
}

bool CAntiLeechData::GetBadForThisSession(const UINT i) const
{
	return (m_uiBadForThisSession & i) != 0;
}

//////////////////////////////////////////////////////////////////////////
// ANTINICKTHIEF
//
//Remark!
//Other than other modders (nonleecher of course) believe it is NOT recommended
//to create a tag containing other chars than letters only... just look up a copy of
//the nickthief code to see why...

#define MIN_LENGTH	4						//min chars to use
#define MAX_LENGTH	8						//max chars to use
#if (MIN_LENGTH > MAX_LENGTH)
#error MAX_LENGTH MUST BE GREATER THAN OR EQUAL TO MIN_LENGTH
#endif
#define MAX_ADD		(MAX_LENGTH-MIN_LENGTH) //max chars to add
CString	CAntiLeechData::m_sAntiNickThiefTag = L"";
CString	CAntiLeechData::m_sAntiNickThiefTag2 = L"";
void CAntiLeechData::CreateAntiNickThiefTag()
{
	uint8 maxchar = uint8(MIN_LENGTH+rand()%MAX_ADD); 

	m_sAntiNickThiefTag = L"";
	for(uint8 i = 0; i < maxchar; ++i)
		m_sAntiNickThiefTag.AppendChar( (rand()%2 ? L'A' : L'a') + (_TINT)rand()%25 );

	maxchar = uint8(MIN_LENGTH+rand()%MAX_ADD); 
	m_sAntiNickThiefTag2 = L"";
	for(uint8 i = 0; i < maxchar; ++i)
		m_sAntiNickThiefTag2.AppendChar( (rand()%2 ? L'A' : L'a') + (_TINT)rand()%25 );
}

CString	CAntiLeechData::GetAntiNickThiefNick()
{	
	CString tag1 = L"";
	CString tag2 = L"";
	if(rand()%2 == 0)
	{
		if(rand()%2 == 0)
			tag1.Format(L"(%s)", m_sAntiNickThiefTag);
		else
			tag1.Format(L"{%s}", m_sAntiNickThiefTag);
		tag2.Format(L"[%s]", m_sAntiNickThiefTag2);
	}
	else
	{
		if(rand()%2 == 0)
			tag1.Format(L"(%s)", m_sAntiNickThiefTag2);
		else
			tag1.Format(L"{%s}", m_sAntiNickThiefTag2);
		tag2.Format(L"[%s]", m_sAntiNickThiefTag);
	}
	CString ret = L"";
	ret.Format(L"%s %s %s", tag1, thePrefs.GetUserNick(), tag2);
	return ret;
}

void CAntiLeechData::Check4NickThief()
{
	if(m_pParent == NULL || m_pParent->GetUserName() == NULL)
	{
		ASSERT(0);
		return;
	}

	//you can add a switch here...
	CString strNick = m_pParent->GetUserName();
	if(strNick.IsEmpty())
	{
		ClearBadForThisSession(AT_NICKTHIEF);
		return; //just a sanity check
	}

	const bool bHit1 = strNick.Find(m_sAntiNickThiefTag) != -1;
	const bool bHit2 = strNick.Find(m_sAntiNickThiefTag2) != -1;
	//only changed the logs for now... we *could* give it different weight, though...
	if(bHit1 && bHit2)
		SetBadForThisSession(AT_NICKTHIEF, L"double tag");
	else if(bHit1 || bHit2)
		SetBadForThisSession(AT_NICKTHIEF, L"simple tag");
	else
	{
		//there are some wise guys out there I think... 
		//(Torni, is that you? *lol*)		
		strNick.MakeLower();
		CString tofind = m_sAntiNickThiefTag;
		CString tofind2 = m_sAntiNickThiefTag2;
		tofind.MakeLower(); 
		tofind2.MakeLower();
		//just add a logline for now... 
		//no punish them - but only if both cases are true (very unlikely...)
		const bool bHit1 = strNick.Find(tofind) != -1;
		const bool bHit2 = strNick.Find(tofind2) != -1;
		if(bHit1 && bHit2)
			SetBadForThisSession(AT_NICKTHIEF, L"wrong case double tag");
		else if(bHit1 || bHit2)
			SetBadForThisSession(AT_NICKTHIEF, L"wrong case single tag");
		else 
			ClearBadForThisSession(AT_NICKTHIEF);
	}
}

//////////////////////////////////////////////////////////////////////////
// ANTIMODTHIEF
CString	CAntiLeechData::m_sMyVersion;
void CAntiLeechData::Check4ModThief()
{	
	if(m_pParent == NULL)
	{
		ASSERT(0);
		return;
	}

	//you can add a switch here...
	const CString strMod = m_pParent->GetClientModVer();
	const CString OurMod = MOD_VERSION; //cache it
	if(/*!strMod.IsEmpty() && */StrStrI(strMod, OurMod)) //uses our string
	{
		CString strDetail = L"";
		//but not the correct length
		if(strMod.GetLength() != OurMod.GetLength()) 
			strDetail.Format(L"invalid length (%i != %i)", strMod.GetLength(), OurMod.GetLength());
		//but uses a wrong eMule version
		else if(_tcscmp(m_pParent->GetClientSoftVer(), m_sMyVersion) != 0) 
			strDetail.Format(L"invalid version (%s != %s)", m_pParent->GetClientSoftVer(), m_sMyVersion);
		//but not in correct case
		else if(_tcscmp(strMod, OurMod) != 0) 
			strDetail.Format(L"invalid case (%s != %s)", strMod, OurMod);
		//but uses a wrong eMule version (detailed)
		else if(m_pParent->GetRealVersion() != ((CemuleApp::m_nVersionMjr<<17)|(CemuleApp::m_nVersionMin<<10)|(CemuleApp::m_nVersionUpd<<7)|(UINT(DBG_VERSION_BUILD))))
		{
			const UINT Maj = (m_pParent->GetRealVersion() >> 17) & 0x7F;
			const UINT Min = (m_pParent->GetRealVersion() >> 10) & 0x7F;
			const UINT Up  = (m_pParent->GetRealVersion() >>  7) & 0x07;
			const UINT Dbg = (m_pParent->GetRealVersion()) & 0x7F;
			strDetail.Format(L"invalid version+ (%u.%u.%u.%u != %u.%u.%u.%u)", Maj, Min, Up, Dbg, VERSION_MJR, VERSION_MIN, VERSION_UPDATE, DBG_VERSION_BUILD);
		}

		if(!strDetail.IsEmpty())
		{
			SetBadForThisSession(AT_MODTHIEF, strDetail);
			return;
		}
	}
	ClearBadForThisSession(AT_MODTHIEF);
}

//////////////////////////////////////////////////////////////////////////
// ANTIMODFAKE
void CAntiLeechData::Check4ModFaker(const bool bIsBadShareaza)
{	
	if(m_pParent == NULL)
	{
		ASSERT(0);
		return;
	}

	CString strModFaker = L"";

//>>> zz_fly::Bad Shareaza detection
	//note: Shareaza like client send UDPPort tag AFTER Misc Options tag
	if (bIsBadShareaza					// was UDP sent after Misc Options tag
		&& m_pParent->GetClientSoft() == SO_EMULE)	// but pretends to be a mule?
		strModFaker = L"Fake Shareaza";
//<<< zz_fly::Bad Shareaza detection
	else
	{
//zz_fly :: anti some modstring thief :: start
		//This code checks for supposedly bad clients (modfakers) which use a known mods' string
		//(mods which use the anti-modthief code from Xman) by checking if they actually use the necessary pattern
		const CString strMod = m_pParent->GetClientModVer();
		const CString strNick = m_pParent->GetUserName();
		if(!strMod.IsEmpty() && !strNick.IsEmpty())
		{			
			struct sDaS //DoubleAndString :)
			{
				sDaS(const CString& str, const double& d)
				{
					strMod = str;
					dVer = d;
				}
				~sDaS()	{}
				CString strMod;
				double	dVer;
			};

			static const sDaS testModString[] = 
			{
				sDaS(L"Xtreme", 4.4),
				sDaS(L"ScarAngel", 2.5),
				sDaS(L"Mephisto", 1.5),
				sDaS(L"MorphXT", 10.0),
				sDaS(L"EastShare", 13.0),
				sDaS(L"StulleMule", 6.0),
//				sDaS(L"MagicAngel", 3.0),
				sDaS(L"DreaMule", 3.0),
				sDaS(L"X-Mod", 0.0),
				sDaS(L"RaJiL", 2.2),
				sDaS(L"AcKroNiC", 6.0)
			};

			for(int i = 0; i < ARRSIZE(testModString); ++i)
			{
                const int tag1 = strNick.Find(L'«' + testModString[i].strMod);
				if(strMod.Find(testModString[i].strMod) != -1) 
				{
					const int iPos = strMod.ReverseFind(L' ');
					if(iPos != -1)
					{
						const float version = (float)_tstof(strMod.Mid(iPos+1));
						const bool bSupportVer = testModString[i].dVer == 0.0f || version == 9.7f || version >= testModString[i].dVer;
						if(bSupportVer)
						{
							if(tag1 == -1) //not found in nick?
								strModFaker = L"Xtreme Fake1";
//>>> WiZaRd - Enhancement
							else
							{
                                const int tag2 = strNick.Find(L'»', tag1+1);
								if(tag2 == -1) //found in nick... but no enclosing brackets!?
									strModFaker = L"Xtreme Fake4";
							}
//<<< WiZaRd - Enhancement
						}
					}
					else //no version!?
						strModFaker = L"Xtreme Fake2";
				}
				//Xtreme based clients MIGHT send the addon in the nick but not a modstring! check out the code @SendHelloPacket in Xtreme
				else if(!strMod.IsEmpty() && tag1 != -1) //found in nick but not in modstring!?
					strModFaker = L"Xtreme Fake3";
			}
		}
	}
//zz_fly :: end

	if(strModFaker.IsEmpty())
		ClearBadForThisSession(AT_MODFAKER);
	else
		SetBadForThisSession(AT_MODFAKER, strModFaker);
}

//////////////////////////////////////////////////////////////////////////
// MISC CHECK FUNCTIONS
//AdditionalModThiefCheck is called whenever a client achieves a VERYBAD state.
//In this case we check if we know about that client (i.e. he's using OUR mod or another CA enabled mod) and if so, 
//we can be pretty sure that he's also a modfaker because WE are the "good guys" :)
bool CAntiLeechData::AdditionalModThiefCheck(const UINT i)
{
	if(m_pParent /*&& i != AT_MODTHIEF*/)
	{
		const CString strMod = m_pParent->GetClientModVer();
		if(strMod.IsEmpty())
			return false;

		struct sDaS //DoubleAndString :)
		{
			sDaS(const CString& str, const double& d)
			{
				strMod = str;
				dVer = d;
			}
			~sDaS()	{}
			CString strMod;
			double	dVer;
		};

		//uses a CA enabled client... thus it shouldn't be a bad client :)
		static const sDaS strModArr[] = 
		{
//these are *NOT* necessary for the CA to do its work... but they are a nice addon :)
			sDaS(L"Tombstone v", 0),
			sDaS(L"Tombstone Xtended v", 0),
			sDaS(L"eMuleFuture v", 0),
			sDaS(L"AcKroNiC v", 5.0),
			sDaS(L"AcKroNiC-IL v", 5.0), // some of those are still around
			sDaS(L"SharkX v", 0),
			sDaS(L"zBOOM v", 0.1),
			sDaS(L"beba v", 1.0),
			sDaS(L"Spike2 v", 0.8),
			sDaS(L"AnalyZZUL ", 1.0),
//<<<
		};

		CString strBad = L"";
		CString strModThis = L"";
		for(int j = 0; j < _countof(strModArr); ++j)
		{			
			int iPos = strMod.ReverseFind(L'v');
			if(iPos == -1)
				iPos = strMod.ReverseFind(L' ');
			strModThis = strModArr[j].strMod;
			if(StrStrI(strMod, strModThis)) //modstring found
			{	
				const int iPos = strMod.Find(strModThis);
				if(iPos != -1)
				{
					const double version = _tstof(strMod.Mid(iPos+strModThis.GetLength()));
					if(version >= strModArr[j].dVer)
						strBad = L"bad CA mod";
				}
				else //modstring found but not case-sensitive... probably a faker
					strBad = L"CA mod faker";
			}
			//if the modstring is equal up to the separator ONLY... probably a faker
			else if(iPos != -1 && _tcsnicmp(strMod, strModThis, iPos) == 0)
				strBad = L"CA mod faker2";
			if(!strBad.IsEmpty())
			{
				CString msg = L"";
				msg.Format(L"%s version of a CA mod (%s)!", GetReasonString(i), strBad);
//				if(j >= _countof(strModArr) - 1)
//					SetBadForThisSession(AT_MODTHIEF, msg);
				SetBadForThisSession(AT_MODFAKER, msg);
				return true;
			}
		}
	}

	return false;
}

void CAntiLeechData::Check4FileFaker()
{
	if(m_pParent == NULL)
		return;

	if(m_pParent->GetUploadFileID() != NULL && m_pParent->GetRequestFile() && m_pParent->IsCompleteSource())
	{
//		CKnownFile* upreqfile = m_pParent->GetUploadReqFile();
//		if(upreqfile && reqfile == upreqfile)
		//we speak about the same file
		if (md4cmp(m_pParent->GetUploadFileID(), m_pParent->GetRequestFile()->GetFileHash()) == 0) 
		{
			SetBadForThisSession(AT_FILEFAKER);
			return;
		}
	}
	ClearBadForThisSession(AT_FILEFAKER);
}

CString	CAntiLeechData::GetAntiLeechDataString() const
{
	CString reason = L"";
	if(m_uiBadForThisSession != 0)
		reason.Format(L"Very bad: %s\n", GetReasonString(m_uiBadForThisSession));

		//[SEPPL] - Don't let the score fall too low because of UL/DL 
		if(m_pParent 
			&& (!m_pParent->GetRequestFile() || m_pParent->GetDownloadState() > DS_ONQUEUE))
			; //idle :P
		else if(chunksDL < chunksUL)
			reason.AppendFormat(L"Bad UL/DL ratio (%I64u/%I64u)\n", chunksUL, chunksDL);

	if(IsFastAskClient())
		reason.AppendFormat(L"Reask: %u @ %s avg\n", GetReaskCount(), CastSecondsToHM(GetAvgReaskTime()/1000));
	if(GetSpams() > 0)
		reason.AppendFormat(L"Spams: %i\n", GetSpams());
	if(IsXSExploiter())
		reason.AppendFormat(L"XS: %u | %u\n", m_pData->uXSAsks, m_pData->uXSAnsw);
	if(IsXSSpammer())
		reason.AppendFormat(L"FastXS: %i\n", m_pData->iFastXSAsks);
	if(m_pData->uBadULSessions || m_pData->uBadDLSessions)
		reason.AppendFormat(L"Failed UL/DL: %u/%u\n", m_pData->uBadULSessions, m_pData->uBadDLSessions);		
	
	//NOTE: we *could* create a multiline entry for the display, too
	reason.Replace(L"\n", L" "); //try to get a single line with spacings

	return reason;
}

//////////////////////////////////////////////////////////////////////////
// LIST 
CAntiLeechDataList::CAntiLeechDataList()
{
	memset(m_uiBadActions, 0, sizeof(m_uiBadActions));
	m_dwLastSaved = ::GetTickCount();
	m_mapClients.InitHashTable(GetPrimeNumber(5000)); 
	m_mapBadClients.InitHashTable(GetPrimeNumber(100)); //>>> Store corrupt part senders
	CAntiLeechData::CreateAntiNickThiefTag();
	CAntiLeechData::m_sMyVersion.Format(L"eMule v%u.%u%c", VERSION_MJR, VERSION_MIN, L'a' + VERSION_UPDATE);
	LoadList();
}

CAntiLeechDataList::~CAntiLeechDataList()
{
	SaveList();
	CAntiLeechData* cur_credit = NULL;
	CCKey tmpkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos)
	{
		m_mapClients.GetNextAssoc(pos, tmpkey, cur_credit);
		delete cur_credit;
	}
	m_mapClients.RemoveAll(); //needed? doesn't hurt either...
//>>> Store corrupt part senders
	corruptPartSenderInfo* info = NULL;
	pos = m_mapBadClients.GetStartPosition();
	while (pos)
	{
		m_mapBadClients.GetNextAssoc(pos, tmpkey, info);
		delete info;
	}
	m_mapBadClients.RemoveAll(); //needed? doesn't hurt either...
//<<< Store corrupt part senders
}

bool CAntiLeechDataList::LoadList(const CString& path) 
{
	bool bTriedWithBackup = false;
	CString strFileName = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + ANTILEECH_MET_FILENAME;
	const bool bCustomLoad = !path.IsEmpty();
	if(bCustomLoad)
		strFileName = path;
	const UINT iOpenFlags = CFile::modeRead|CFile::osSequentialScan|CFile::typeBinary|CFile::shareDenyWrite;
	CSafeBufferedFile file;
	CFileException fexp;

loadstart:
	if (!file.Open(strFileName, iOpenFlags, &fexp))
	{
		if (fexp.m_cause != CFileException::fileNotFound)
		{
			if(bTriedWithBackup)
			{
				CString strError(L"Failed to load AntiLeechFile, a new file will be created");
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, _countof(szError)))
					strError.AppendFormat(L" - %s", szError);
				LogError(LOG_STATUSBAR, strError);
				return false;
			}
			else
			{
				bTriedWithBackup = true;
				strFileName.Format(L"%s%s.bak", thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), ANTILEECH_MET_FILENAME);
				goto loadstart; //I hate using gotos but it works...
			}
		}
		return false;
	}
	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	try
	{
		uint8 version = file.ReadUInt8();
		if (version != AT_MET_HEADER
			&& version != AT_MET_HEADER_ACCEPTED)
		{
			if(bTriedWithBackup)
			{
				LogWarning(GetResString(IDS_ERR_CREDITFILEOLD));
				file.Close();
				return false;
			}
			else
			{
				bTriedWithBackup = true;
				strFileName.Format(L"%s%s.bak", thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), ANTILEECH_MET_FILENAME);
				goto loadstart; //I hate using gotos but it works...
			}
		}

		// everything is ok, lets see if the backup exists...
		CString strBakFileName;
		strBakFileName.Format(L"%s%s.bak", thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), ANTILEECH_MET_FILENAME);

		DWORD dwBakFileSize = 0;
		BOOL bCreateBackup = TRUE;

		HANDLE hBakFile = ::CreateFile(strBakFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
			OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hBakFile != INVALID_HANDLE_VALUE)
		{
			// OK, the backup exists, get the size
			dwBakFileSize = ::GetFileSize(hBakFile, NULL); //debug
			if (dwBakFileSize > (DWORD)file.GetLength())
			{
				// the size of the backup was larger then the original file, something is wrong here, don't overwrite old backup...
				bCreateBackup = FALSE;
			}
			//else: backup is smaller or the same size as original file, proceed with copying of file
			::CloseHandle(hBakFile);
		}
		//else: the backup doesn't exist, create it

		if (bCreateBackup)
		{
			file.Close(); // close the file before copying

			if (!::CopyFile(strFileName, strBakFileName, FALSE))
				LogError(L"Failed to make backup of AntiLeechFile");
			else
				bTriedWithBackup = true; //reset - no use to try loading the backup now - no going back :)

			// reopen file
			CFileException fexp;
			if (!file.Open(strFileName, iOpenFlags, &fexp))
			{
				CString strError(L"Failed to load AntiLeechFile, a new file will be created");
				TCHAR szError[MAX_CFEXP_ERRORMSG];
				if (fexp.GetErrorMessage(szError, _countof(szError)))
					strError.AppendFormat(L" - %s", szError);
				LogError(LOG_STATUSBAR, strError);
				return false;
			}
			setvbuf(file.m_pStream, NULL, _IOFBF, 16384);
			file.Seek(1, CFile::begin); //set filepointer behind file version byte
		}

		UINT count = file.ReadUInt32();
		UINT cDeleted = 0;

		const DWORD dwExpired = (DWORD)time(NULL) - 12960000; // today - 150 days
		for (UINT i = 0; i < count; ++i)
		{
			CAntiLeechData* newcredits = new CAntiLeechData(&file);

			//Too old? Delete 'em!
			if (newcredits->GetLastSeen() < dwExpired || newcredits->IsEmpty())
			{
				++cDeleted;
				delete newcredits;
				continue;
			}

//>>> .met-Merger			
			if(bCustomLoad)
			{
				CAntiLeechData* tmpcredits = NULL;
				if(m_mapClients.Lookup(CCKey(newcredits->GetKey()), tmpcredits))
				{
					theApp.QueueDebugLogLineEx(LOG_WARNING, L"DBG: WARNING - Found duplicate credits for %s", md4str(newcredits->GetKey()));
					//choose the "better one" - we default to the older one :)
					if(newcredits->GetLastSeen() > tmpcredits->GetLastSeen())
					{
						++cDeleted;
						delete newcredits;
						continue;
					}
				}
			}
//<<< .met-Merger

			m_mapClients.SetAt(CCKey(newcredits->GetKey()), newcredits);
		}		

//>>> Store corrupt part senders
		UINT countBad = 0;
		if(file.GetPosition() != file.GetLength()) //avoid "corrupt antileech.met" message
			countBad = file.ReadUInt32();
		UINT cDeletedBad = 0;
		const DWORD dwExpiredBad = (DWORD)time(NULL) - 5184000; // today - 60 days
		
		uchar hash[16];
		DWORD dwAdded = 0;
		for (UINT i = 0; i < countBad; ++i)
		{
			file.ReadHash16(hash);
			dwAdded = file.ReadUInt32();
			CCKey key(hash);
			corruptPartSenderInfo* info = NULL;
			if(!m_mapBadClients.Lookup(key, info))
				info = new corruptPartSenderInfo(hash, dwAdded);
			else
				ASSERT(0);

			if(info->dwAdded >= dwExpiredBad) 
				m_mapBadClients.SetAt(CCKey(hash), info);
			else
			{
				delete info;
				++cDeletedBad;
			}
		}
//<<< Store corrupt part senders

		file.Close();

		CString logLine = L"AntiLeechFile loaded";
		if(count)
			logLine.AppendFormat(L", %u clients are known", count-cDeleted);
		if(cDeleted > 0)
			logLine.AppendFormat(L", %u clients deleted (not seen within 150 days or empty)", cDeleted);
		if(countBad)
			logLine.AppendFormat(L", %u corrupt part senders loaded", countBad-cDeletedBad);
		if(cDeletedBad > 0)
			logLine.AppendFormat(L", %u entries deleted (not seen within 60 days)", countBad-cDeletedBad);
		AddLogLine(false, logLine);
		AddDebugLogLine(false, L"Using %s of data for CA clients!", 
			CastItoXBytes((count-cDeleted)*sizeof(CAntiLeechStruct) + (countBad-cDeletedBad)*sizeof(corruptPartSenderInfo)) );
		
		return true;
	}
	catch(CFileException* error)
	{
		if(bTriedWithBackup)
		{
			if (error->m_cause == CFileException::endOfFile)
				LogError(LOG_STATUSBAR, L"Error: AntiLeechFile is corrupted and will be replaced!");
			else
			{
				TCHAR buffer[MAX_CFEXP_ERRORMSG];
				error->GetErrorMessage(buffer, _countof(buffer));
				LogError(LOG_STATUSBAR, L"Unexpected file error while reading AntiLeechFile: %s", buffer);
			}
			error->Delete();
			file.Close();
			return false;
		}
		else
		{
			error->Delete();
			file.Close();
			bTriedWithBackup = true;
			strFileName.Format(L"%s%s.bak", thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), ANTILEECH_MET_FILENAME);
			goto loadstart; //I hate using gotos but it works...
		}
	}
	//return false; //should never be reached
}

void CAntiLeechDataList::SaveList()
{
	if (thePrefs.GetLogFileSaving())
		AddDebugLogLine(false, L"Saving AntiLeechFile \"%s\"", ANTILEECH_MET_FILENAME);
	m_dwLastSaved = ::GetTickCount();

	const CString name = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + ANTILEECH_MET_FILENAME;
	CSafeBufferedFile file;
	CFileException fexp;
	if (!file.Open(name, CFile::modeWrite|CFile::modeCreate|CFile::typeBinary|CFile::shareDenyWrite, &fexp))
	{
		CString strError(L"Failed to save AntiLeechFile");
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (fexp.GetErrorMessage(szError, _countof(szError)))
			strError.AppendFormat(L" - %s", szError);
		LogError(LOG_STATUSBAR, strError);
		return;
	}

	setvbuf(file.m_pStream, NULL, _IOFBF, 16384);

	UINT count = 0;
	CAntiLeechData* cur_credit;	
	CCKey tempkey(0);
	POSITION pos = m_mapClients.GetStartPosition();
	while (pos)
	{
		m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
		if (!cur_credit->IsEmpty())
			++count; 
	}

	try
	{
		file.WriteUInt8(AT_MET_HEADER);
		file.WriteUInt32(count);
		pos = m_mapClients.GetStartPosition();
		while (pos)
		{
			m_mapClients.GetNextAssoc(pos, tempkey, cur_credit);
			if (!cur_credit->IsEmpty())
				cur_credit->WriteToFile(&file);
		}

//>>> Store corrupt part senders
		count = 0;
		pos = m_mapBadClients.GetStartPosition();
		corruptPartSenderInfo* info = NULL;
		while (pos)
		{
			m_mapBadClients.GetNextAssoc(pos, tempkey, info);
			if(info->dwAdded < (DWORD)(time(NULL) - 5184000)) // today - 60 days
				m_mapBadClients.RemoveKey(tempkey);
			else
				++count; 
		}

		file.WriteUInt32(count);
		pos = m_mapBadClients.GetStartPosition();
		while (pos)
		{
			m_mapBadClients.GetNextAssoc(pos, tempkey, info);
			file.WriteHash16(info->hash);
			file.WriteUInt32(info->dwAdded);
		}
//<<< Store corrupt part senders

		if (thePrefs.GetCommitFiles() >= 2 
			|| (thePrefs.GetCommitFiles() >= 1 && !theApp.emuledlg->IsRunning()))
			file.Flush();		
	}
	catch(CFileException* error)
	{
		CString strError(L"Failed to save AntiLeechFile");
		TCHAR szError[MAX_CFEXP_ERRORMSG];
		if (error->GetErrorMessage(szError, _countof(szError)))
			strError.AppendFormat(L" - %s", szError);
		LogError(LOG_STATUSBAR, strError);
		error->Delete();
	}
	file.Close();
}

CAntiLeechData* CAntiLeechDataList::GetData(const uchar* key)
{
	CAntiLeechData* result;
	CCKey tkey(key);
	if (!m_mapClients.Lookup(tkey, result))
	{
		result = new CAntiLeechData(key);
		m_mapClients.SetAt(CCKey(result->GetKey()), result);
	}
	result->SetLastSeen();
	return result;
}

//>>> Store corrupt part senders
void	CAntiLeechDataList::AddCorruptPartSender(const uchar* pSender)
{
	if(isnulmd4(pSender))
		return;

	if(IsCorruptPartSender(pSender)) //no dupcheck needed - simply update it!
		return;

	m_mapBadClients.SetAt(CCKey(pSender), new corruptPartSenderInfo(pSender, (DWORD)time(NULL)));
}

bool	CAntiLeechDataList::IsCorruptPartSender(const uchar* pClient)
{
	corruptPartSenderInfo* info = NULL;
	CCKey key(pClient);
	if (m_mapBadClients.Lookup(key, info))
	{
		if(info->dwAdded >= (DWORD)(time(NULL) - 5184000)) // today - 60 days
			return true;

		delete info;
		m_mapBadClients.RemoveKey(key);
	}
	return false;
}
//<<< Store corrupt part senders

void CAntiLeechDataList::Process()
{
	if (::GetTickCount() - m_dwLastSaved > MIN2MS(18))
		SaveList();
}

void CAntiLeechData::SetParent(CUpDownClient* client, const bool bSetOnly)
{
	if(bSetOnly)
	{
		m_pParent = client;
		return;
	}

	if(m_pParent == NULL || m_pParent != client)
	m_pParent = client;
	else
	{
		//TODO! can it happen that 2 clients have the same data!?
		//I think this could happen for hashthieves, etc...
		//the question is: do we have to do something about that issue?
//		ASSERT(0);
//		return;
		m_pParent = client;
	}

	if(m_pParent == NULL)
		return;

	switch(m_pParent->GetClientSoft())
	{
		case SO_MLDONKEY:
			m_uiReaskTime = MIN_REQUESTTIME;
			break;

		case SO_SHAREAZA:
			m_uiReaskTime = MIN_REQUESTTIME*2;
			break;
            
		//21 mins - reask time by old version of the client
		//taken from eMule+ source
		case SO_EDONKEY:
		case SO_OLDEMULE:
			m_uiReaskTime = MIN2MS(21);
			break;

		//eMule+ uses 1 minute less, though this shouldn't hurt... 
		case SO_EMULEPLUS:
			m_uiReaskTime = MIN2MS(28);
			break;

		case SO_EMULE:
			m_uiReaskTime = FILEREASKTIME;
			break;

		//any information about those 3 clients? I couldn't find their default reasktime...
//		case SO_TRUSTYFILES:
//		case SO_HYDRANODE:
//		case SO_EASYMULE2:
		default:
			m_uiReaskTime = MIN_REQUESTTIME; //we don't know their limits, default to lowest
			break;
	}

	//if the client has credits...	
	if(m_pParent->Credits() != NULL
		//... but we have no stats about him...
		&& GetUploadedTotal() == 0
		&& GetDownloadedTotal() == 0)
	{
		//... then we add the credits' values to the antileechdata 
		//take it as "normal" data, not partial, not rare...
		AddUploaded(m_pParent->Credits()->GetUploadedTotal(), false, AT_RAREFILE+1);
		AddDownloaded(m_pParent->Credits()->GetDownloadedTotal(), false, AT_RAREFILE+1);
	}
}

//>>> CTempCAList
void CAntiLeechDataList::SetParent(CUpDownClient* client)
{
	m_TempList.SetParent(client);
}

void CAntiLeechDataList::ResetParent(CUpDownClient* client)
{
	m_TempList.ResetParent(client);
}

void CAntiLeechDataList::Verify(CUpDownClient* client)
{
	m_TempList.Verify(client);
}

//////////////////////////////////////////////////////////////////////////
CTempCAList::CTempCAList()
{
}

CTempCAList::~CTempCAList()
{
	//cleanup
	CList<CAntiLeechData*>* cur = NULL;
	CCKey tmpkey(0);
	POSITION pos = m_DataList.GetStartPosition();
	while (pos)
	{
		m_DataList.GetNextAssoc(pos, tmpkey, cur);
		ASSERT(cur->GetCount() <= 1); //there should only be (max) 1 entry left (the original one)
		delete cur;
	}
	m_DataList.RemoveAll(); //needed? doesn't hurt either...
}

void CTempCAList::SetParent(CUpDownClient* client)
{	
	//if a client already HAS valid data attached, there is no need to proceed
	if(client->GetAntiLeechData() != NULL)
	{
		ASSERT(client->GetAntiLeechData()->GetParent() == client); //check the cross-pointers
		return;
	}

	//There are 2 possibilities:
	//1) we don't have an entry yet
	//-> create a new list and proceed with 2)
	//2) we already have at least one entry in our list
	//-> append a new entry to the list and link it to the client
	CAntiLeechData* data = theApp.antileechlist->GetData(client->GetUserHash());;
	CCKey key(data->GetKey());	
	CList<CAntiLeechData*>* pList = NULL;
	if(!m_DataList.Lookup(key, pList))
	{
		//if we already assigned a client then this one must be a bad guy
		if(data->GetParent() != NULL)
		{			
			//TODO: should we ban all other clients with the same hash?
//			client->Ban(NULL, BA_BANHASHTHIEF);
			ASSERT(0); //this should never happen here... I mean: the list is EMPTY, so how can the data be attached to someone?!
		}
		pList = new CList<CAntiLeechData*>;			
		
		//add the original entry at the top! this entry will have a NULL m_pParent member until we verified the correct one! 
		pList->AddTail(data);
		m_DataList.SetAt(key, pList);
	}

	//create a copy of the original entry and attach it to the client
	data = new CAntiLeechData(pList->GetHead());
	pList->AddTail(data);
	data->SetParent(client);
	client->SetAntiLeechData(data);

	//added this so "IS_UNAVAILABLE" clients get verified ASAP... but might be buggy?
	if(client->HasPassedSecureIdent(true))
		Verify(client);
}

void CTempCAList::ResetParent(CUpDownClient* client)
{
	CAntiLeechData* data = client->GetAntiLeechData();
	data->SetParent(NULL); //reset the m_pParent pointer of the antileech data so we can re-assign it if needed

	//this is called on client deletion... we have to cleanup in that case :)	
	CCKey key(data->GetKey());
	CList<CAntiLeechData*>* pList = NULL;
	if(!m_DataList.Lookup(key, pList))
	{
		ASSERT(0); //can that happen!?
		delete data;
		return;
	}

	//if we FIND the clients' data, we remove it, otherwise...?
	POSITION pos = pList->Find(data);	
	if(pos)
	{
		if(pos != pList->GetHeadPosition())
	{
		//i.e. not only the original entry - we NEVER delete the original entry as we want to save it, later
			delete data;
		pList->RemoveAt(pos);
	}	
}
	else
	{
		ASSERT(0);
		delete data;
	}
}

bool LeftIsBetter(const CUpDownClient* pLeft, const CUpDownClient* pRight)
{
	bool bLeft = pLeft->HasPassedSecureIdent(true);
	bool bRight = pRight->HasPassedSecureIdent(true);

	if(bLeft && bRight)
	{
		//both passed the first check... but one might be "IS_UNAVAILABLE"...
		bLeft = pLeft->HasPassedSecureIdent(false);
		bRight = pRight->HasPassedSecureIdent(false);

		if(bLeft != bRight)
			return bLeft;
//		return true; //default to "true" (i.e. "keep the old client")
		return false; //default to "false" because I think this will happen often after ip-changes
	}
	//at least one did not pass the check... bLeft will return the proper one
	return bLeft;
}

void CTempCAList::Verify(CUpDownClient* client)
{
	//if a client does not have valid data attached, there is no need to proceed
	CAntiLeechData* data = client->GetAntiLeechData();
	if(data == NULL)
	{
		ASSERT(0); //can that happen!?
		return;
	}

	CCKey key(data->GetKey());
	CList<CAntiLeechData*>* pList = NULL;
	if(!m_DataList.Lookup(key, pList))
	{
		ASSERT(0); //can that happen!?
		return;
	}

	CAntiLeechData* head = pList->GetHead();
	//already linked to the proper entry? exit!
	if(data == head)
		return;

	CUpDownClient* pLeft = head->GetParent();
	if(pLeft == client
		//same client connected multiple times... might happen on startup
		//this is solved in "AttachToAlreadyKnown"
		|| (pLeft && pLeft->GetConnectIP() == client->GetConnectIP())) 
		return;

	//Merge the original entry with the one of the correctly identified client	
	//if we already assigned a client then this one must be a bad guy - or not?
	if(pLeft!= NULL)
	{		
		//Hmm... well... I think this can only happen in ONE case:
		//we connected to a client without SecIdent and meet the proper (SUI-enabled) client later on
		//No, actually it can also happen if someone reconnects with a different IP... but in that case we shouldn't have 2 instances
		//of the client as "AttachToAlreadyKnown"?
		if(LeftIsBetter(pLeft, client))
		{
			theApp.QueueLogLineEx(LOG_WARNING, L"%s: verification of CA data for %s canceled (%s is \"better\")", md4str(client->GetUserHash()), client->DbgGetClientInfo(), pLeft->DbgGetClientInfo());
			return; //nothing to do if the old one is the better one
		}
		
		//If the new one is better... well, in this case, we simply "swap" the corresponding data
		//1) change the data so we have the "original" at the top again
		POSITION pos = pList->Find(data);
		if(pos)
		{
			//remove it ASAP - dunno whether the Add commands might change the list
			pList->RemoveAt(pos);

//			ASSERT(pos != pList->GetHeadPosition());
			theApp.QueueLogLineEx(LOG_WARNING, L"%s: swapping CA data for %s and %s during verification", md4str(client->GetUserHash()), client->DbgGetClientInfo(), pLeft->DbgGetClientInfo());

			//copy the "bad" clients' data to the bottom and attach it			
			pList->AddTail(new CAntiLeechData(head));
			pList->GetTail()->SetParent(pLeft, true);
			pLeft->SetAntiLeechData(pList->GetTail());

			//merge the "good" clients' data into the original entry and attach it
			head->Merge(data);
			head->SetParent(client, true);
			client->SetAntiLeechData(head);

			//cleanup :)
			delete data;

			//TODO: should we ban all other clients with the same hash?
//			pList->GetTail()->GetParent()->Ban(NULL, BA_BANHASHTHIEF);		
		}
		else
		{
			ASSERT(0);
			delete data;
		}
		return;
	}

	//merge the "good" clients' data into the original entry and attach it
	head->Merge(data);
	head->SetParent(client, true);
	client->SetAntiLeechData(head);	

	//delete the old (temporary) entry from our list
	POSITION pos = pList->Find(data);
	delete data;
	if(pos)
		pList->RemoveAt(pos);
	else
		ASSERT(0);
}
//<<< CTempCAList
// Tux: Feature: Client Analyzer [end]