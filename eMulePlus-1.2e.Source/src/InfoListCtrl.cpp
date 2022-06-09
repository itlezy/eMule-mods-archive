//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
// (p) 2002 by FoRcHa (a.k.a. NO)  [seppforcher38@hotmail.com]

#include "stdafx.h"
#include "emule.h"
#include "server.h"
#include "ServerList.h"
#include "InfoListCtrl.h"
#include "updownclient.h"
#include "SharedFileList.h"
#include "otherfunctions.h"
#include "COptionTree\OptionTreeItemComboBox.h"
#include "COptionTree\OptionTreeItemStatic.h"
#include "COptionTree\OptionTreeDef.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CInfoListCtrl::CInfoListCtrl()
{
	m_eListType = INFOLISTTYPE_NONE;
	m_pPartFile = NULL;
	m_pClient = NULL;
	m_bShown = false;

	for(int i = 0; i < 4; i++)
		m_bExpandedUser[i] = true;
	for(int i = 0; i < 2; i++)
		m_bExpandedFile[i] = true;

	m_crEntryTextColor = RGB(0,45,126);

//	Initialize a file structure
	UserData.General = NULL;
	UserData.UserName = NULL;
	UserData.UserHash = NULL;
	UserData.ClientSoftware = NULL;
	UserData.IPAddress = NULL;
	UserData.ID = NULL;
	UserData.ServerIP = NULL;
	UserData.ServerName = NULL;

	UserData.Transfer = NULL;
	UserData.CurDownloading = NULL;
	UserData.DownloadedSession = NULL;
	UserData.UploadedSession = NULL;
	UserData.AverageDownloadrate = NULL;
	UserData.AverageUploadrate = NULL;
	UserData.DownloadedTotal = NULL;
	UserData.UploadedTotal = NULL;

	UserData.Scores = NULL;
	UserData.DlUpModifier = NULL;
	UserData.CommunityUser = NULL;
	UserData.Rating = NULL;
	UserData.UploadQueueScore = NULL;
	UserData.RFRatio = NULL;
	UserData.SFRatio = NULL;

	UserData.RemoteScores = NULL;
	UserData.RemoteDlUpModifier = NULL;
	UserData.RemoteRating = NULL;
	UserData.RemoteQueueRank = NULL;

//	Initialize a file structure
	FileData.General = NULL;
	FileData.FullName = NULL;
	FileData.MetFile = NULL;
	FileData.Hash = NULL;
	FileData.FileSize = NULL;
	FileData.RealSize = NULL;
	FileData.PartFileStatus = NULL;
	FileData.SourceNames = NULL;

	FileData.Transfer = NULL;
	FileData.FoundSources = NULL;
	FileData.CompleteSources = NULL;
	FileData.Transferring = NULL;
	FileData.FilepartCount = NULL;
	FileData.PartAvailable = NULL;
	FileData.LastSeenComplete = NULL;
	FileData.LastProgress = NULL;
	FileData.Transferred = NULL;
	FileData.CompletedSize = NULL;
	FileData.DataRate = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CInfoListCtrl::~CInfoListCtrl()
{
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BEGIN_MESSAGE_MAP(CInfoListCtrl, COptionTree)
	ON_WM_TIMER()
	ON_WM_SHOWWINDOW()
END_MESSAGE_MAP()
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BOOL CInfoListCtrl::Create(DWORD dwStyle, RECT rcRect, CWnd *pParentWnd, UINT nID)
{
	dwStyle |= WS_CLIPCHILDREN;
	return COptionTree::Create(dwStyle, rcRect, pParentWnd, OT_OPTIONS_SHADEEXPANDCOLUMN | OT_OPTIONS_SHADEROOTITEMS, nID);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoListCtrl::Localize()
{
	if(GetSafeHwnd())
	{
		CreateList();
		UpdateData(false);
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoListCtrl::ClearList()
{
	SetRedraw(FALSE);
//	Now we can safely remove items from the tree (delete we call inside)
	DeleteAllItems();
	SetRedraw(TRUE);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// an intern fuction that create a OptionTree
// eklmn: please remember that SetRedraw() prevents COptionTree class from redraw during an objects creation
void CInfoListCtrl::CreateList()
{
	SetRedraw(FALSE);

	if (m_eListType != INFOLISTTYPE_NONE)
	{
	//	Now we can safely remove items from the tree (delete we call inside)
		DeleteAllItems();
	}

	SetRedraw(TRUE);

	try
	{
		if (m_eListType == INFOLISTTYPE_SOURCE)
		{
			UserData.General = InsertItem(new COptionTreeItem);
			UserData.General->SetLabelText(GetResString(IDS_PW_GENERAL));
			UserData.General->Expand(m_bExpandedUser[0]);
			UserData.UserName = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.General);
			UserData.UserName->SetLabelText(GetResString(IDS_INFLST_USER_USERNAME));
			if(UserData.UserName->CreateStaticItem(0))
				UserData.UserName->SetTextColor(m_crEntryTextColor);
			UserData.UserHash = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.General);
			UserData.UserHash->SetLabelText(GetResString(IDS_INFLST_USER_USERHASH));
			if(UserData.UserHash->CreateStaticItem(0))
				UserData.UserHash->SetTextColor(m_crEntryTextColor);
			UserData.ClientSoftware = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.General);
			UserData.ClientSoftware->SetLabelText(GetResString(IDS_INFLST_USER_CLIENTSOFTWARE));
			if(UserData.ClientSoftware->CreateStaticItem(0))
				UserData.ClientSoftware->SetTextColor(m_crEntryTextColor);
			UserData.IPAddress = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.General);
			UserData.IPAddress->SetLabelText(GetResString(IDS_INFLST_USER_IPADDRESS));
			if(UserData.IPAddress->CreateStaticItem(0))
				UserData.IPAddress->SetTextColor(m_crEntryTextColor);
			UserData.ID = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.General);
			UserData.ID->SetLabelText(GetResString(IDS_ID));
			if(UserData.ID->CreateStaticItem(0))
				UserData.ID->SetTextColor(m_crEntryTextColor);
			UserData.ServerIP = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.General);
			UserData.ServerIP->SetLabelText(GetResString(IDS_SERVERIP));
			if(UserData.ServerIP->CreateStaticItem(0))
				UserData.ServerIP->SetTextColor(m_crEntryTextColor);
			UserData.ServerName = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.General);
			UserData.ServerName->SetLabelText(GetResString(IDS_INFLST_USER_SERVERNAME));
			if(UserData.ServerName->CreateStaticItem(0))
				UserData.ServerName->SetTextColor(m_crEntryTextColor);

			UserData.Transfer = InsertItem(new COptionTreeItem);
			UserData.Transfer->SetLabelText(GetResString(IDS_TRANSFER_NOUN));
			UserData.Transfer->Expand(m_bExpandedUser[1]);
			UserData.CurDownloading = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Transfer);
			UserData.CurDownloading->SetLabelText(GetResString(IDS_INFLST_USER_CURDOWNLOAD));
			if(UserData.CurDownloading->CreateStaticItem(0))
				UserData.CurDownloading->SetTextColor(m_crEntryTextColor);
			UserData.DownloadedSession = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Transfer);
			UserData.DownloadedSession->SetLabelText(GetResString(IDS_INFLST_USER_SESSIONDOWNLOAD));
			if(UserData.DownloadedSession->CreateStaticItem(0))
				UserData.DownloadedSession->SetTextColor(m_crEntryTextColor);
			UserData.UploadedSession = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Transfer);
			UserData.UploadedSession->SetLabelText(GetResString(IDS_INFLST_USER_SESSIONUPLOAD));
			if(UserData.UploadedSession->CreateStaticItem(0))
				UserData.UploadedSession->SetTextColor(m_crEntryTextColor);
			UserData.AverageDownloadrate = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Transfer);
			UserData.AverageDownloadrate->SetLabelText(GetResString(IDS_INFLST_USER_AVERAGEDOWNRATE));
			if(UserData.AverageDownloadrate->CreateStaticItem(0))
				UserData.AverageDownloadrate->SetTextColor(m_crEntryTextColor);
			UserData.AverageUploadrate = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Transfer);
			UserData.AverageUploadrate->SetLabelText(GetResString(IDS_INFLST_USER_AVERAGEUPRATE));
			if(UserData.AverageUploadrate->CreateStaticItem(0))
				UserData.AverageUploadrate->SetTextColor(m_crEntryTextColor);
			UserData.DownloadedTotal = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Transfer);
			UserData.DownloadedTotal->SetLabelText(GetResString(IDS_INFLST_USER_TOTALDOWNLOAD));
			if(UserData.DownloadedTotal->CreateStaticItem(0))
				UserData.DownloadedTotal->SetTextColor(m_crEntryTextColor);
			UserData.UploadedTotal = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Transfer);
			UserData.UploadedTotal->SetLabelText(GetResString(IDS_INFLST_USER_TOTALUPLOAD));
			if(UserData.UploadedTotal->CreateStaticItem(0))
				UserData.UploadedTotal->SetTextColor(m_crEntryTextColor);

			UserData.Scores = InsertItem(new COptionTreeItem);
			UserData.Scores->SetLabelText(GetResString(IDS_INFLST_USER_SCORES));
			UserData.Scores->Expand(m_bExpandedUser[2]);
			UserData.DlUpModifier = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Scores);
			UserData.DlUpModifier->SetLabelText(GetResString(IDS_INFLST_USER_DLUPMODIFIER));
			if(UserData.DlUpModifier->CreateStaticItem(0))
				UserData.DlUpModifier->SetTextColor(m_crEntryTextColor);
			UserData.CommunityUser = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Scores);
			UserData.CommunityUser->SetLabelText(GetResString(IDS_COMMUNITY));
			if(UserData.CommunityUser->CreateStaticItem(0))
				UserData.CommunityUser->SetTextColor(m_crEntryTextColor);
			UserData.Rating = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Scores);
			UserData.Rating->SetLabelText(GetResString(IDS_RATING));
			if(UserData.Rating->CreateStaticItem(0))
				UserData.Rating->SetTextColor(m_crEntryTextColor);
			UserData.UploadQueueScore = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Scores);
			UserData.UploadQueueScore->SetLabelText(GetResString(IDS_INFLST_USER_UPLOADQUEUESCORE));
			if(UserData.UploadQueueScore->CreateStaticItem(0))
				UserData.UploadQueueScore->SetTextColor(m_crEntryTextColor);
			UserData.RFRatio = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Scores);
			UserData.RFRatio->SetLabelText(GetResString(IDS_INFLST_USER_RFRATIO));
			if(UserData.RFRatio->CreateStaticItem(0))
				UserData.RFRatio->SetTextColor(m_crEntryTextColor);
			UserData.SFRatio = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.Scores);
			UserData.SFRatio->SetLabelText(GetResString(IDS_INFLST_USER_SFRATIO));
			if(UserData.SFRatio->CreateStaticItem(0))
				UserData.SFRatio->SetTextColor(m_crEntryTextColor);

			UserData.RemoteScores = InsertItem(new COptionTreeItem);
			UserData.RemoteScores->SetLabelText(GetResString(IDS_INFLST_REMOTE_SCORES));
			UserData.RemoteScores->Expand(m_bExpandedUser[3]);
			UserData.RemoteDlUpModifier = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.RemoteScores);
			UserData.RemoteDlUpModifier->SetLabelText(GetResString(IDS_INFLST_USER_DLUPMODIFIER));
			if(UserData.RemoteDlUpModifier->CreateStaticItem(0))
				UserData.RemoteDlUpModifier->SetTextColor(m_crEntryTextColor);
			UserData.RemoteRating = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.RemoteScores);
			UserData.RemoteRating->SetLabelText(GetResString(IDS_RATING));
			if(UserData.RemoteRating->CreateStaticItem(0))
				UserData.RemoteRating->SetTextColor(m_crEntryTextColor);
			UserData.RemoteQueueRank = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, UserData.RemoteScores);
			UserData.RemoteQueueRank->SetLabelText(GetResString(IDS_INFLST_REMOTE_RANKING));
			if(UserData.RemoteQueueRank->CreateStaticItem(0))
				UserData.RemoteQueueRank->SetTextColor(m_crEntryTextColor);
		}
		else
		{
			FileData.General = InsertItem(new COptionTreeItem);
			FileData.General->SetLabelText(GetResString(IDS_PW_GENERAL));
			FileData.General->Expand(m_bExpandedFile[0]);
			FileData.FullName = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.General);
			FileData.FullName->SetLabelText(GetResString(IDS_INFLST_FILE_FULLNAME));
			if(FileData.FullName->CreateStaticItem(0))
				FileData.FullName->SetTextColor(m_crEntryTextColor);
			FileData.MetFile = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.General);
			FileData.MetFile->SetLabelText(GetResString(IDS_DL_FILENAME));
			if(FileData.MetFile->CreateStaticItem(0))
				FileData.MetFile->SetTextColor(m_crEntryTextColor);
			FileData.Hash = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.General);
			FileData.Hash->SetLabelText(GetResString(IDS_HASH));
			if(FileData.Hash->CreateStaticItem(0))
				FileData.Hash->SetTextColor(m_crEntryTextColor);
			FileData.FileSize = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.General);
			FileData.FileSize->SetLabelText(GetResString(IDS_INFLST_FILE_FILESIZE));
			if(FileData.FileSize->CreateStaticItem(0))
				FileData.FileSize->SetTextColor(m_crEntryTextColor);
			FileData.RealSize = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.General);
			FileData.RealSize->SetLabelText(GetResString(IDS_SIZE_ON_DISK));
			if(FileData.RealSize->CreateStaticItem(0))
				FileData.RealSize->SetTextColor(m_crEntryTextColor);
			FileData.PartFileStatus = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.General);
			FileData.PartFileStatus->SetLabelText(GetResString(IDS_INFLST_FILE_PARTFILESTATUS));
			if(FileData.PartFileStatus->CreateStaticItem(0))
				FileData.PartFileStatus->SetTextColor(m_crEntryTextColor);
			FileData.SourceNames = (COptionTreeItemComboBox*)InsertItem(new COptionTreeItemComboBox(), FileData.General);
			FileData.SourceNames->SetLabelText(GetResString(IDS_INFLST_FILE_SOURCENAMES));
			if(FileData.SourceNames->CreateComboItem(NULL) == TRUE)
			{
				FileData.SourceNames->SetTextColor(m_crEntryTextColor);
				FileData.SourceNames->SetCurSel(0);
				FileData.bSourceNameCombo = true;
			}
			else
				FileData.bSourceNameCombo = false;
			FileData.iSourceNameUpdateDelay = 0;

			FileData.Transfer = InsertItem(new COptionTreeItem);
			FileData.Transfer->SetLabelText(GetResString(IDS_TRANSFER_NOUN));
			FileData.Transfer->Expand(m_bExpandedFile[1]);
			FileData.FoundSources = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.FoundSources->SetLabelText(GetResString(IDS_FD_SOURCES));
			if(FileData.FoundSources->CreateStaticItem(0))
				FileData.FoundSources->SetTextColor(m_crEntryTextColor);
			FileData.CompleteSources = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.CompleteSources->SetLabelText(GetResString(IDS_SF_COMPLETESRC));
			if(FileData.CompleteSources->CreateStaticItem(0))
				FileData.CompleteSources->SetTextColor(m_crEntryTextColor);
			FileData.Transferring = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.Transferring->SetLabelText(GetResString(IDS_INFLST_FILE_TRANSFERRINGSOURCES));
			if(FileData.Transferring->CreateStaticItem(0))
				FileData.Transferring->SetTextColor(m_crEntryTextColor);

			FileData.FilepartCount = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.FilepartCount->SetLabelText(GetResString(IDS_INFLST_FILE_FILEPARTCOUNT));
			if(FileData.FilepartCount->CreateStaticItem(0))
				FileData.FilepartCount->SetTextColor(m_crEntryTextColor);
			FileData.PartAvailable = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.PartAvailable->SetLabelText(GetResString(IDS_INFLST_FILE_PARTAVAILABLE));
			if(FileData.PartAvailable->CreateStaticItem(0))
				FileData.PartAvailable->SetTextColor(m_crEntryTextColor);
			FileData.LastSeenComplete = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.LastSeenComplete->SetLabelText(GetResString(IDS_LASTSEENCOMPLETE));
			if(FileData.LastSeenComplete->CreateStaticItem(0))
				FileData.LastSeenComplete->SetTextColor(m_crEntryTextColor);
			FileData.LastProgress = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.LastProgress->SetLabelText(GetResString(IDS_LASTRECEPTION));
			if(FileData.LastProgress->CreateStaticItem(0))
				FileData.LastProgress->SetTextColor(m_crEntryTextColor);
			FileData.Transferred = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.Transferred->SetLabelText(GetResString(IDS_INFLST_FILE_TRANSFERRED));
			if(FileData.Transferred->CreateStaticItem(0))
				FileData.Transferred->SetTextColor(m_crEntryTextColor);
			FileData.CompletedSize = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.CompletedSize->SetLabelText(GetResString(IDS_INFLST_FILE_COMPLETEDSIZE));
			if(FileData.CompletedSize->CreateStaticItem(0))
				FileData.CompletedSize->SetTextColor(m_crEntryTextColor);
			FileData.DataRate = (COptionTreeItemStatic*)InsertItem(new COptionTreeItemStatic, FileData.Transfer);
			FileData.DataRate->SetLabelText(GetResString(IDS_INFLST_FILE_DATARATE));
			if(FileData.DataRate->CreateStaticItem(0))
				FileData.DataRate->SetTextColor(m_crEntryTextColor);
		}
	}
	catch (CException* error )
	{
		OUTPUT_DEBUG_TRACE();
		error->Delete();
		ClearList();
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// this sfunction create & fill an InfoList if it visible
void CInfoListCtrl::UpdateData(bool bExpandAllItems)
{
	if(!IsWindowVisible())
		return;

	EMULE_TRY

	CString strBuffer;

	if (m_eListType == INFOLISTTYPE_SOURCE)
	{
		if (m_pClient != NULL)
		{
			if(UserData.General->IsExpanded())
			{
				if (!m_pClient->IsUserNameEmpty())
					UserData.UserName->SetStaticText(m_pClient->GetUserName());
				else
					UserData.UserName->SetStaticText(_T("?"));

				if (m_pClient->HasValidHash())
					UserData.UserHash->SetStaticText(HashToString(m_pClient->GetUserHash()));
				else
					UserData.UserHash->SetStaticText(_T("?"));

				UserData.ClientSoftware->SetStaticText(m_pClient->GetFullSoftVersionString());
				
				strBuffer.Format(_T("%u (%s)"), m_pClient->GetUserIDHybrid(),
					::GetResString((m_pClient->HasLowID()) ? IDS_PRIOLOW : IDS_PRIOHIGH));
				UserData.ID->SetStaticText(strBuffer);

				strBuffer.Format(_T("%s:%u"),m_pClient->GetFullIP(), m_pClient->GetUserPort());
				UserData.IPAddress->SetStaticText(strBuffer);

				if(m_pClient->GetServerIP())
				{
					strBuffer.Format(_T("%s:%u"), ipstr(m_pClient->GetServerIP()), m_pClient->GetServerPort());
					UserData.ServerIP->SetStaticText(strBuffer);

					CServer* pServer = g_App.m_pServerList->GetServerByIP(m_pClient->GetServerIP());
					if(pServer)
						UserData.ServerName->SetStaticText(pServer->GetListName());
					else
						UserData.ServerName->SetStaticText(_T("?"));
				}
				else
				{
					UserData.ServerIP->SetStaticText(_T("?"));
					UserData.ServerName->SetStaticText(_T("?"));
				}
			}

			if(UserData.Transfer->IsExpanded())
			{
				CKnownFile* pSharedFile = g_App.m_pSharedFilesList->GetFileByID(m_pClient->m_reqFileHash);
				if(pSharedFile)
					UserData.CurDownloading->SetStaticText(pSharedFile->GetFileName());
				else
					UserData.CurDownloading->SetStaticText(_T("-"));

				UserData.UploadedSession->SetStaticText(CastItoXBytes(m_pClient->GetTransferredDown()));

				UserData.DownloadedSession->SetStaticText(CastItoXBytes(m_pClient->GetTransferredUp()));

				strBuffer.Format(_T("%.1f %s"),
					static_cast<double>(m_pClient->GetDownloadDataRate())/1024.0, GetResString(IDS_KBYTESEC));
				UserData.AverageUploadrate->SetStaticText(strBuffer);

				strBuffer.Format(_T("%.1f %s"),
					static_cast<double>(m_pClient->GetUpDataRate()) / 1024.0, GetResString(IDS_KBYTESEC));
				UserData.AverageDownloadrate->SetStaticText(strBuffer);
			}

			if(m_pClient->Credits())
			{

				UserData.UploadedTotal->SetStaticText(CastItoXBytes(m_pClient->Credits()->GetDownloadedTotal()));
				UserData.DownloadedTotal->SetStaticText(CastItoXBytes(m_pClient->Credits()->GetUploadedTotal()));

				strBuffer.Format(_T("%.1f"),m_pClient->Credits()->GetScoreRatio(m_pClient->GetIP()));
				UserData.DlUpModifier->SetStaticText(strBuffer);
			}
			else
			{
				UserData.UploadedTotal->SetStaticText(_T("?"));
				UserData.DownloadedTotal->SetStaticText(_T("?"));
				UserData.DlUpModifier->SetStaticText(_T("?"));
			}

			if(UserData.Scores->IsExpanded())
			{
				if(g_App.m_pPrefs->CommunityEnabled())
					UserData.CommunityUser->SetStaticText(YesNoStr(m_pClient->IsCommunity()));
				else
					UserData.CommunityUser->SetStaticText(GetResString(IDS_DISABLED));

				bool	bLoadSourceName = (_tcsstr(m_pClient->GetUserName(), ::GetResString(IDS_SAVED_SOURCE)) != NULL) ||
					(_tcsstr(m_pClient->GetUserName(), ::GetResString(IDS_EXCHANGEDSOURCE)) != NULL);

				if (!bLoadSourceName)
				{
					if (m_pClient->IsDownloading())
						strBuffer = _T("-");
					else
						strBuffer.Format(_T("%u"), m_pClient->GetScore(true));
					UserData.Rating->SetStaticText(strBuffer);
				}
				else
					UserData.Rating->SetStaticText(_T("?"));

				if(m_pClient->GetUploadState() != US_NONE && !m_pClient->IsDownloading())
				{
					strBuffer.Format(_T("%u"), m_pClient->GetScore(false));
					UserData.UploadQueueScore->SetStaticText(strBuffer);
				}
				else
					UserData.UploadQueueScore->SetStaticText(_T("-"));

				CKnownFile* pSharedFile = g_App.m_pSharedFilesList->GetFileByID(m_pClient->m_reqFileHash);

				if (pSharedFile)
				{
					strBuffer.Format(_T("%.2f"), pSharedFile->GetPopularityRatio());
					UserData.RFRatio->SetStaticText(strBuffer);
				}
				else
					UserData.RFRatio->SetStaticText(_T("-"));

				if (pSharedFile)
				{
					strBuffer.Format(_T("%.2f"), pSharedFile->GetSizeRatio());
					UserData.SFRatio->SetStaticText(strBuffer);
				}
				else
					UserData.SFRatio->SetStaticText(_T("-"));
			}

			if(UserData.RemoteScores->IsExpanded())
			{
				if (m_pClient->GetRemoteQueueRank())
				{
					strBuffer.Format(_T("%u"),m_pClient->GetRemoteQueueRank());
					UserData.RemoteQueueRank->SetStaticText(strBuffer);
				}
				else
					UserData.RemoteQueueRank->SetStaticText(_T("?"));

				strBuffer.Format(_T("%.1f"),m_pClient->GetRemoteBaseModifier());
				UserData.RemoteDlUpModifier->SetStaticText(strBuffer);

				strBuffer.Format(_T("%u"),m_pClient->GetRemoteRatio());
				UserData.RemoteRating->SetStaticText(strBuffer);
			}
		}
	}
	else if (m_eListType == INFOLISTTYPE_FILE)
	{
		if (m_pPartFile != NULL)
		{
			if(FileData.General->IsExpanded())
			{
				FileData.FullName->SetStaticText(m_pPartFile->GetFileName());
				FileData.MetFile->SetStaticText(m_pPartFile->GetFilePath());
				FileData.Hash->SetStaticText(HashToString(m_pPartFile->GetFileHash()));
				FileData.FileSize->SetStaticText(CastItoXBytes(m_pPartFile->GetFileSize()));
				FileData.RealSize->SetStaticText(CastItoXBytes(m_pPartFile->GetRealFileSize()));
				FileData.PartFileStatus->SetStaticText(m_pPartFile->GetPartFileStatus());
				if(FileData.iSourceNameUpdateDelay % 10 == 0)
					FillSourcenameList();
				FileData.iSourceNameUpdateDelay++;
			}

			if(FileData.Transfer->IsExpanded())
			{
				strBuffer.Format(_T("%u"),m_pPartFile->GetPartCount());
				FileData.FilepartCount->SetStaticText(strBuffer);

				double percent	=	0.0;
				if (m_pPartFile->GetPartCount())
				{
					percent	=	static_cast<double>((m_pPartFile->GetAvailablePartCount()*100.0) / m_pPartFile->GetPartCount());
				}
				strBuffer.Format(_T("%u (%.1f%%)"), m_pPartFile->GetAvailablePartCount(), percent);
				FileData.PartAvailable->SetStaticText(strBuffer);

				if(m_pPartFile->lastseencomplete == NULL)
					FileData.LastSeenComplete->SetStaticText(GetResString(IDS_NEVER));
				else
					FileData.LastSeenComplete->SetStaticText(m_pPartFile->LocalizeLastSeenComplete());

				if (m_pPartFile->GetTransferred()==0)
					FileData.LastProgress->SetStaticText(GetResString(IDS_NEVER));
				else
					FileData.LastProgress->SetStaticText(m_pPartFile->LocalizeLastDownTransfer());

				FileData.Transferred->SetStaticText(CastItoXBytes(m_pPartFile->GetTransferred()));


				strBuffer.Format(_T("%s (%.2f%%)"), CastItoXBytes(m_pPartFile->GetCompletedSize()), m_pPartFile->GetPercentCompleted2());

				EnumPartFileStatuses	eFileStatus = m_pPartFile->GetStatus();
				uint64	qwFileSz = ((eFileStatus == PS_COMPLETING) || (eFileStatus == PS_COMPLETE)) ? m_pPartFile->GetFileSize() : m_pPartFile->GetCompletedSize();

				if (qwFileSz != 0)
				{
					double	dTmp = 100 / static_cast<double>(qwFileSz);

					strBuffer.AppendFormat( _T("  %s %.2f%%  %s %.2f%%"),
						GetResString(IDS_FD_COMPRESSION), dTmp * static_cast<double>(m_pPartFile->GetGainDueToCompression()),
						GetResString(IDS_FD_CORRUPTION), dTmp * static_cast<double>(m_pPartFile->GetLostDueToCorruption()) );
				}
				FileData.CompletedSize->SetStaticText(strBuffer);

				strBuffer.Format(_T("%.2f %s"),
					static_cast<double>(m_pPartFile->GetDataRate())/1024.0, GetResString(IDS_KBYTESEC));
				FileData.DataRate->SetStaticText(strBuffer);

				strBuffer.Format(_T("%u"), m_pPartFile->GetSourceCount());
				FileData.FoundSources->SetStaticText(strBuffer);

				strBuffer.Format(_T("%u"), m_pPartFile->GetCompleteSourcesCount());
				FileData.CompleteSources->SetStaticText(strBuffer);

				strBuffer.Format(_T("%u"), m_pPartFile->GetTransferringSrcCount());
				FileData.Transferring->SetStaticText(strBuffer);
			}
		}
	}

	if(bExpandAllItems)
		ExpandAllItems();

	UpdatedItems();

	EMULE_CATCH
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoListCtrl::OnTimer(UINT nIDEvent)
{
	if(nIDEvent == m_nUpdateTimer)
		UpdateData();
	COptionTree::OnTimer(nIDEvent);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoListCtrl::OnShowWindow(BOOL bShow, UINT nStatus)
{
	COptionTree::OnShowWindow(bShow, nStatus);

	if(bShow)
	{
		//create a list by the open if client was specified
		if (m_pClient != NULL || m_pPartFile != NULL)
			CreateList();
		//activate a timer to fill a list
		m_nUpdateTimer = SetTimer(1, 500, NULL);
		m_bShown = true;
	}
	else
	{
		KillTimer(m_nUpdateTimer);
		//save actuall state before list will be cleared
		SaveState(m_eListType);
		ClearList();
		m_bShown = false;
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoListCtrl::SetList(EnumInfoListType enumItemType, void * pObject)
{
	BOOL	bIsWindowVisible = IsWindowVisible();

	// change an item
	switch(enumItemType)
	{
		case INFOLISTTYPE_SOURCE:
			// update pointers
			m_pClient = reinterpret_cast<CUpDownClient*>(pObject);
			m_pPartFile = NULL;
			//create a list if windows is visible, infolist is open  & type was changed
			if (m_bShown && bIsWindowVisible && m_eListType != INFOLISTTYPE_SOURCE)
			{
				//save actuall state
				SaveState(m_eListType);
				// update a object type & create a list
				m_eListType = INFOLISTTYPE_SOURCE;
				CreateList();
			}
			else
				// just update a object type
				m_eListType = INFOLISTTYPE_SOURCE;
			// update a header in Transfer window
			g_App.m_pMDlg->m_wndTransfer.UpdateInfoHeader();
			break;
		case INFOLISTTYPE_FILE:
			// update pointers
			m_pClient = NULL;
			m_pPartFile = reinterpret_cast<CPartFile*>(pObject);
			if (m_bShown && bIsWindowVisible && m_eListType != INFOLISTTYPE_FILE)
			{
				//save actuall state
				SaveState(m_eListType);
				// update a object type, infolist is open & create a list
				m_eListType = INFOLISTTYPE_FILE;
				CreateList();
			}
			else
				// just update a object type
				m_eListType = INFOLISTTYPE_FILE;
			// update a header in Transfer window
			g_App.m_pMDlg->m_wndTransfer.UpdateInfoHeader();
			break;
		case INFOLISTTYPE_NONE:
		default:
			m_pClient = NULL;
			m_pPartFile = NULL;
		//	Update the header in the Transfer window
			g_App.m_pMDlg->m_wndTransfer.UpdateInfoHeader();
			if (m_bShown && bIsWindowVisible && m_eListType != INFOLISTTYPE_NONE)
			{
				ClearList();
			}
			m_eListType = INFOLISTTYPE_NONE;
	}

	// Update ( (re)create & fill the InfoList) will be done every 500ms over timer
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct FSL_TempStruct
{
	CString strFileName;
	int iCount;
};
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CInfoListCtrl::FillSourcenameList()
{
	if (FileData.bSourceNameCombo)
	{
		//FileData.SourceNames->Clear();
		while(FileData.SourceNames->GetCount())
			FileData.SourceNames->DeleteString(0);

		if (m_pPartFile)
		{
			CList<FSL_TempStruct,FSL_TempStruct&> lstSourceNames;
			ClientList		clientListCopy;
			POSITION		pos2, pos3, posHighest;
			CUpDownClient	*pClient;
			CString			strFileName;

			m_pPartFile->GetCopySourceLists(SLM_ALL, &clientListCopy);
			for (ClientList::const_iterator cIt = clientListCopy.begin(); cIt != clientListCopy.end(); cIt++)
			{
				pClient = *cIt;
				if (!pClient->IsClientFilenameEmpty())
				{
					strFileName = pClient->GetClientFilename();

					bool bFound = false;
					pos2 = lstSourceNames.GetHeadPosition();
					for(int i = 0; i < lstSourceNames.GetCount(); i++)
					{
						pos3 = pos2;
						FSL_TempStruct src = lstSourceNames.GetNext(pos2);
						if (strFileName == src.strFileName)
						{
							src.iCount++;
							lstSourceNames.SetAt(pos3, src);
							bFound = true;
							break;
						}
					}
					if(!bFound)
					{
						FSL_TempStruct src;
						src.strFileName = strFileName;
						src.iCount = 1;
						lstSourceNames.AddTail(src);
					}
				}
			}

			if (lstSourceNames.GetCount() > 0)
			{
				for (int i = 0; i < lstSourceNames.GetCount(); i++)
				{
					pos2 = lstSourceNames.GetHeadPosition();
					posHighest = pos2;

					int			iHighest = 0;

					for (int j = 0; j < lstSourceNames.GetCount(); j++)
					{
						pos3 = pos2;

						FSL_TempStruct srctmp = lstSourceNames.GetNext(pos2);

						if (srctmp.iCount > iHighest)
						{
							iHighest = srctmp.iCount;
							posHighest = pos3;
						}
					}

					FSL_TempStruct src = lstSourceNames.GetAt(posHighest);
					strFileName.Format(_T("%s   (%u)"), src.strFileName, src.iCount);
					FileData.SourceNames->AddString(strFileName);
					src.iCount = 0;
					lstSourceNames.SetAt(posHighest, src);
				}
			}
			else
			{
				FileData.SourceNames->AddString(GetResString(IDS_INFLST_FILE_NOSOURCES));
			}

			FileData.SourceNames->SetCurSel(0);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CString CInfoListCtrl::GetName() const
{
	if (m_eListType == INFOLISTTYPE_SOURCE)
		return m_pClient == NULL ? _T("") : m_pClient->GetUserName();
	else
		return m_pPartFile == NULL ? _T("") : m_pPartFile->GetFileName();
}
