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
#pragma once

#include "COPTIONTREE\OptionTree.h"

class CUpDownClient;

enum EnumInfoListType
{
	INFOLISTTYPE_NONE = 0,
	INFOLISTTYPE_SOURCE = 1,
	INFOLISTTYPE_FILE = 2
};

class COptionTreeItemComboBox;
class COptionTreeItemStatic;

class CInfoListCtrl : public COptionTree
{
public:
	CInfoListCtrl();
	virtual ~CInfoListCtrl();
	void Localize();

protected:
	void ClearList();
	void CreateList();

	void FillSourcenameList();
	void UpdateData(bool bExpand = false);
	void SaveState(EnumInfoListType eListType)
	{
		if (eListType == INFOLISTTYPE_SOURCE && m_pClient != NULL)
		{
			if (UserData.General)
				m_bExpandedUser[0] = UserData.General->IsExpanded();
			if (UserData.Transfer)
				m_bExpandedUser[1] = UserData.Transfer->IsExpanded();
			if (UserData.Scores)
				m_bExpandedUser[2] = UserData.Scores->IsExpanded();
			if (UserData.RemoteScores)
				m_bExpandedUser[3] = UserData.RemoteScores->IsExpanded();
		}
		else if (eListType == INFOLISTTYPE_FILE && m_pPartFile != NULL)
		{
			if (FileData.General)
				m_bExpandedFile[0] = FileData.General->IsExpanded();
			if (FileData.Transfer)
				m_bExpandedFile[1] = FileData.Transfer->IsExpanded();
		}
	}

protected:
	CPartFile* 	m_pPartFile;
	CUpDownClient* m_pClient;
	EnumInfoListType m_eListType;
	
	UINT m_nUpdateTimer;
	COLORREF m_crEntryTextColor;

	bool m_bShown;
	BOOL m_bExpandedUser[4];
	BOOL m_bExpandedFile[2];
	
	struct _TAG_USERDATA_
	{
		COptionTreeItem			*General;
		COptionTreeItemStatic	*UserName;
		COptionTreeItemStatic	*UserHash;
		COptionTreeItemStatic	*ClientSoftware;
		COptionTreeItemStatic	*IPAddress;
		COptionTreeItemStatic	*ID;
		COptionTreeItemStatic	*ServerIP;
		COptionTreeItemStatic	*ServerName;

		COptionTreeItem			*Transfer;
		COptionTreeItemStatic	*CurDownloading;
		COptionTreeItemStatic	*DownloadedSession;
		COptionTreeItemStatic	*UploadedSession;
		COptionTreeItemStatic	*AverageDownloadrate;
		COptionTreeItemStatic	*AverageUploadrate;
		COptionTreeItemStatic	*DownloadedTotal;
		COptionTreeItemStatic	*UploadedTotal;

		COptionTreeItem			*Scores;
		COptionTreeItemStatic	*DlUpModifier;
		COptionTreeItemStatic	*CommunityUser;
		COptionTreeItemStatic	*Rating;
		COptionTreeItemStatic	*UploadQueueScore;

		COptionTreeItemStatic	*SFRatio;
		COptionTreeItemStatic	*RFRatio;

		COptionTreeItem			*RemoteScores;
		COptionTreeItemStatic	*RemoteDlUpModifier;
		COptionTreeItemStatic	*RemoteRating;
		COptionTreeItemStatic	*RemoteQueueRank;
	} UserData;
	
	struct _TAG_FILEDATA_
	{
		COptionTreeItem			*General;
		COptionTreeItemStatic	*FullName;
		COptionTreeItemStatic	*MetFile;
		COptionTreeItemStatic	*Hash;
		COptionTreeItemStatic	*FileSize;
		COptionTreeItemStatic	*RealSize;
		COptionTreeItemStatic	*PartFileStatus;
		COptionTreeItemComboBox	*SourceNames;
		bool					bSourceNameCombo;
		int						iSourceNameUpdateDelay;
		
		COptionTreeItem			*Transfer;
		COptionTreeItemStatic	*FoundSources;
		COptionTreeItemStatic	*CompleteSources;
		COptionTreeItemStatic	*FilepartCount;
		COptionTreeItemStatic	*PartAvailable;
		COptionTreeItemStatic	*Transferred;
		COptionTreeItemStatic	*DataRate;
		COptionTreeItemStatic	*Transferring;
		COptionTreeItemStatic	*CompletedSize;
		COptionTreeItemStatic	*LastSeenComplete;
		COptionTreeItemStatic	*LastProgress;
	} FileData;

public:
	BOOL Create(DWORD dwStyle, RECT rcRect, CWnd* pParentWnd, UINT nID);

	void SetList(EnumInfoListType enumItemType, void * pObject = NULL);

	EnumInfoListType GetType() { return m_eListType; }
	void SetType(EnumInfoListType in_ListType) { m_eListType = in_ListType; }

	CString GetName() const;
	CPartFile* GetFile() { return m_pPartFile; }
	CUpDownClient* GetClient() { return m_pClient; }

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
