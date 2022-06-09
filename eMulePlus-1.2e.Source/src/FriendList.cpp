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
#include "FriendList.h"
#include "updownclient.h"
#include "Friend.h"
#include "emule.h"
#include "SafeFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

#define EMFRIENDS_MET_FILENAME	_T("emfriends.met")

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFriendList::CFriendList(void)
{
	LoadList();
	m_nLastSaved = ::GetTickCount();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFriendList::~CFriendList(void)
{
	SaveList();
	for (POSITION pos = m_listFriends.GetHeadPosition(); pos != NULL; )
		delete m_listFriends.GetNext(pos);
	m_listFriends.RemoveAll();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//	LoadList() loads the users friends list from the "emfriends.met" file.
bool CFriendList::LoadList()
{
	CFriend				*pFriend = NULL;
	CSafeBufferedFile	file;

	try
	{
		CString	strFileName = g_App.m_pPrefs->GetConfigDir() + EMFRIENDS_MET_FILENAME;

		if (!file.Open(strFileName, CFile::modeRead | CFile::typeBinary | CFile::shareDenyWrite))
			return false;

		byte	header;

		file.Read(&header, 1);
		if (header != MET_HEADER)
		{
			file.Close();
			return false;
		}
		uint32	nFriendsNumber;

		file.Read(&nFriendsNumber, 4);
		for (uint32 i = 0; i < nFriendsNumber; i++)
		{
			pFriend = new CFriend();
			pFriend->LoadFromFile(file);
			m_listFriends.AddTail(pFriend);
		}
		file.Close();
		return true;
	}
	catch (CFileException * error)
	{
		OUTPUT_DEBUG_TRACE();
		if (error->m_cause == CFileException::endOfFile)
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_EMFRIENDSINVALID);
		else
		{
			AddLogLine(LOG_FL_SBAR | LOG_RGB_ERROR, IDS_ERR_READEMFRIENDS, GetErrorMessage(error));
		}
		error->Delete();
		g_App.m_pMDlg->DisableAutoBackup();

		return false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendList::SaveList()
{
	m_nLastSaved = ::GetTickCount();
	CStdioFile file;
	CString strFileName = g_App.m_pPrefs->GetConfigDir() + EMFRIENDS_MET_FILENAME;

	if (!file.Open(strFileName, CFile::modeCreate | CFile::modeWrite | CFile::typeBinary | CFile::shareDenyWrite))
		return;
	byte header = MET_HEADER;
	file.Write(&header, 1);
	uint32 nRecordsNumber = m_listFriends.GetCount();
	file.Write(&nRecordsNumber, 4);
	for (POSITION pos = m_listFriends.GetHeadPosition(); pos != NULL;)
	{
		m_listFriends.GetNext(pos)->WriteToFile(file);
	}
	file.Close();
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CFriend* CFriendList::SearchFriend(const uchar *abyUserHash, uint32 m_dwIP, uint16 m_uPort) const {

	POSITION pos = m_listFriends.GetHeadPosition();

	while (pos != NULL)
	{
		CFriend *pFriend = m_listFriends.GetNext(pos);
	//	To avoid that an unwanted clients becomes a friend, we have to distinguish between friends with
	//	a userhash and friends which are identified by IP + port only.
		if (pFriend->m_dwHasHash)
		{
		//	Check for a friend which has the same userhash as the specified one
			if (!md4cmp(pFriend->GetUserHash(), abyUserHash))
				return pFriend;
		}
		else
		{
			if (pFriend->m_dwLastUsedIP == m_dwIP && pFriend->m_nLastUsedPort == m_uPort)
				return pFriend;
		}
	}
	return NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendList::RefreshFriend(CFriend *pFriend) const
{
	if (m_pctlFriendList != NULL && ::IsWindow(m_pctlFriendList->m_hWnd))
		m_pctlFriendList->RefreshFriend(pFriend);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CFriendList::ShowFriends() const
{
	if (!m_pctlFriendList)
	{
		ASSERT (false);
		return;
	}
	m_pctlFriendList->DeleteAllItems();
	for (POSITION pos = m_listFriends.GetHeadPosition(); pos != NULL; )
	{
		CFriend * cur_friend = m_listFriends.GetNext(pos);
		m_pctlFriendList->AddFriend(cur_friend);
	}
}

//Added this to work with the IRC.. Probably a better way to do it.. But wanted this in the release..
bool CFriendList::AddFriend(uchar t_m_abyUserhash[16], uint32 tm_dwLastSeen, uint32 tm_dwLastUsedIP, uint32 tm_nLastUsedPort, uint32 tm_dwLastChatted, CString tm_strName, uint32 tm_dwHasHash)
{
	// client must have an IP (HighID) or a hash
	if (tm_dwLastUsedIP<16777216 && tm_dwHasHash==0)
		return false;
	if ( tm_dwLastUsedIP && IsAlreadyFriend(tm_dwLastUsedIP, tm_nLastUsedPort))
		return false;
	CFriend* Record = new CFriend(t_m_abyUserhash, tm_dwLastSeen, tm_dwLastUsedIP, tm_nLastUsedPort, tm_dwLastChatted, tm_strName, tm_dwHasHash);
	m_listFriends.AddTail(Record);
	ShowFriends();
	return true;
}

//	Added for the friends function in the IRC..
bool CFriendList::IsAlreadyFriend(uint32 tm_dwLastUsedIP, uint32 tm_nLastUsedPort)
{
	for (POSITION pos = m_listFriends.GetHeadPosition(); pos != NULL; )
	{
		CFriend * cur_friend = m_listFriends.GetNext(pos);
		if (cur_friend->m_dwLastUsedIP == tm_dwLastUsedIP && cur_friend->m_nLastUsedPort == tm_nLastUsedPort)
		{
			return true;
		}
	}
	return false;
}

bool CFriendList::AddFriend(CUpDownClient *pClient)
{
	if (pClient->IsFriend())
		return false;
	// client must have an IP (HighID) or a hash
	if (pClient->HasLowID() && !pClient->HasValidHash())
		return false;
	CFriend* NewFriend = new CFriend(pClient);
	pClient->m_pFriend = NewFriend;
	m_listFriends.AddTail(NewFriend);
	if (m_pctlFriendList)
		m_pctlFriendList->AddFriend(NewFriend);

	g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.UpdateClient(pClient);
	g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.UpdateClient(pClient);
	g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.UpdateClient(pClient);
	g_App.m_pDownloadList->UpdateSource(pClient);
	return true;
}

void CFriendList::RemoveFriend(CFriend *pToDel)
{
	POSITION pos = m_listFriends.Find(pToDel);
	if (!pos)
	{
		ASSERT (false);
		return;
	}

	pToDel->SetLinkedClient(NULL);

	if (m_pctlFriendList)
		m_pctlFriendList->RemoveFriend(pToDel);
	m_listFriends.RemoveAt(pos);
	delete pToDel;
}

void CFriendList::RemoveFriend(CUpDownClient *pClient)
{
	for (POSITION pos = m_listFriends.GetHeadPosition(); pos != NULL;)
	{
		CFriend	*pFriend = m_listFriends.GetNext(pos);
		if (pFriend->m_dwLastUsedIP == pClient->GetIP() && pFriend->m_nLastUsedPort == pClient->GetUserPort())
		{
			RemoveFriend(pFriend);
			g_App.m_pMDlg->m_wndTransfer.m_ctlQueueList.UpdateClient(pClient);
			g_App.m_pMDlg->m_wndTransfer.m_ctlUploadList.UpdateClient(pClient);
			g_App.m_pMDlg->m_wndTransfer.m_ctlClientList.UpdateClient(pClient);
			g_App.m_pDownloadList->UpdateSource(pClient);
			break;
		}
	}
}

void CFriendList::Process()
{
	if (::GetTickCount() - m_nLastSaved > MIN2MS(25))	// save every 25 minutes
		SaveList();
}