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
#include "Friend.h"
#include "updownclient.h"
#include "packets.h"
#include "emule.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFriend::CFriend(void)
{
	m_dwLastSeen = 0;
	m_dwLastUsedIP = 0;
	m_nLastUsedPort = 0;
	m_dwLastChatted = 0;
	m_pLinkedClient = NULL;
	md4clr(m_abyUserhash);
	m_dwHasHash = 0;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Added this to work with the IRC.. Probably a better way to do it.. But wanted this in the release..
CFriend::CFriend(uchar tm_abyUserhash[16], uint32 tm_dwLastSeen, uint32 tm_dwLastUsedIP,
				 uint32 tm_nLastUsedPort, uint32 tm_dwLastChatted, CString tm_strName, uint32 tm_dwHasHash)
{
	m_dwLastSeen = tm_dwLastSeen;
	m_dwLastUsedIP = tm_dwLastUsedIP;
	m_nLastUsedPort = tm_nLastUsedPort;
	m_dwLastChatted = tm_dwLastChatted;
	if (tm_dwHasHash && tm_abyUserhash)
	{
		md4cpy(m_abyUserhash,tm_abyUserhash);
		m_dwHasHash = md4cmp0(m_abyUserhash) ? 1 : 0;
	}
	else
	{
		md4clr(m_abyUserhash);
		m_dwHasHash = 0;
	}
	m_strName = tm_strName;
	m_pLinkedClient = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFriend::CFriend(CUpDownClient* client)
{
	ASSERT ( client );

	m_dwLastSeen = time(NULL);
	m_dwLastUsedIP = client->GetIP();
	m_nLastUsedPort = client->GetUserPort();
	m_dwLastChatted = 0;
	m_pLinkedClient = NULL;
	SetLinkedClient(client);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFriend::~CFriend(void)
{
	if (m_pLinkedClient != NULL)
	{
		m_pLinkedClient->m_pFriend = NULL;
		m_pLinkedClient = NULL;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriend::LoadFromFile(CFile &file)
{
//	Read the header
	file.Read(m_abyUserhash,16);
	m_dwHasHash = md4cmp0(m_abyUserhash) ? 1 : 0;
	file.Read(&m_dwLastUsedIP,4);
	file.Read(&m_nLastUsedPort,2);
	file.Read(&m_dwLastSeen,4);
	file.Read(&m_dwLastChatted,4);
	m_dwHasHash = 1;

//	Read the friend name ignoring any other tags
	uint32		dwTagCnt;

	file.Read(&dwTagCnt, 4);
	for (uint32 j = 0; j < dwTagCnt; j++)
	{
		CTag	*newtag = new CTag();

		newtag->FillFromStream(file);
		switch (newtag->GetTagID())
		{
			case FF_NAME:
				if (newtag->IsStr())
				{
#ifdef _UNICODE
					if (m_strName.IsEmpty())
#endif
						newtag->GetStringValue(&m_strName);
				}
				break;
		}
		delete newtag;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriend::WriteToFile(CFile &file)
{
	byte	abyteBuf[34];
	CWrTag	tagWr;
	uint32	dwTagCnt = 0;
	bool	bUniName = false;

	if (!m_strName.IsEmpty())
	{
		dwTagCnt++;
		if (IsUTF8Required(m_strName))
		{
			dwTagCnt++;
			bUniName = true;
		}
	}

	if (!m_dwHasHash)
		md4clr(m_abyUserhash);
	md4cpy(&abyteBuf[0], m_abyUserhash);
	POKE_DWORD(&abyteBuf[16], m_dwLastUsedIP);
	POKE_WORD(&abyteBuf[20], static_cast<uint16>(m_nLastUsedPort));
	POKE_DWORD(&abyteBuf[22], m_dwLastSeen);
	POKE_DWORD(&abyteBuf[26], m_dwLastChatted);
	POKE_DWORD(&abyteBuf[30], dwTagCnt);

	file.Write(abyteBuf, sizeof(abyteBuf));

	if (dwTagCnt != 0)
	{
		if (bUniName)
			tagWr.WriteToFile(FF_NAME, m_strName, file, cfUTF8withBOM);
		tagWr.WriteToFile(FF_NAME, m_strName, file);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriend::SetLinkedClient(CUpDownClient *pLinkedClient)
{
	if (pLinkedClient != m_pLinkedClient)
	{
		if (pLinkedClient != NULL)
		{
			m_dwLastSeen = time(NULL);
			m_dwLastUsedIP = pLinkedClient->GetIP();
			m_nLastUsedPort = pLinkedClient->GetUserPort();
			m_strName = pLinkedClient->GetUserName();
			md4cpy(m_abyUserhash, pLinkedClient->GetUserHash());
			m_dwHasHash = md4cmp0(m_abyUserhash) ? 1 : 0;
		}
		if (m_pLinkedClient != NULL)	//	old client is no longer friend, since it is no longer the linked client
			m_pLinkedClient->m_pFriend = NULL;
		m_pLinkedClient = pLinkedClient;
	}
	g_App.m_pFriendList->RefreshFriend(this);
}
