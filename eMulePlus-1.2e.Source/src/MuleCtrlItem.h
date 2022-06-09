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

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <map>
#include <vector>
#pragma warning(pop)

class CPartFile;
class CUpDownClient;

class CMuleCtrlItem
{
private:
	DWORD				m_dwUpdateTimer;
	CBitmap				m_bmpStatus;
	bool				m_bIsVisible;

public:
//	Constructor
						CMuleCtrlItem()
							: m_dwUpdateTimer(0), m_bIsVisible(false) {}
	virtual			   ~CMuleCtrlItem()
						{ m_bmpStatus.DeleteObject(); }

//	Accessors
	DWORD				GetUpdateTimer() const { return m_dwUpdateTimer; }
	CBitmap			   &GetBitmap() { return m_bmpStatus; }
	virtual CPartFile  *GetFile() const = 0;
	bool				IsVisible() const { return m_bIsVisible; }
	virtual void		SetVisibility(bool bIsVisible) { m_bIsVisible = bIsVisible; }

//	General methods
	virtual void		SetUpdateTimer(DWORD dwUpdate) { m_dwUpdateTimer = dwUpdate; }
	virtual void		ResetUpdateTimer() { m_dwUpdateTimer = 0; }
};

class CPartFileDLItem;

class CSourceDLItem : public CMuleCtrlItem
{
public:
	CPartFileDLItem	   *m_pParentFileItem;

private:
	CUpDownClient	   *m_pSource;

	bool				m_bIsAvailable;
	CPartFile		   *m_pParentFile;

	CPartFile		   *GetFile() const { return m_pParentFile; }


public:
	CSourceDLItem(CUpDownClient	*pSource,CPartFile *pParentFile,
		CPartFileDLItem *pParentFileItem, bool bIsAvailable = true )
		: m_pSource(pSource),m_pParentFile(pParentFile),m_pParentFileItem(pParentFileItem),
		m_bIsAvailable(bIsAvailable) {}
	~CSourceDLItem();

	CUpDownClient*		GetSource() const { return m_pSource; }

	CPartFile*			GetParentFile() const { return m_pParentFile; }
	CPartFileDLItem*	GetParentFileItem() const { return m_pParentFileItem; }
	void				SetAvailability(bool bIsAvailable) { m_bIsAvailable = bIsAvailable; }
	bool				IsAvailable() const { return m_bIsAvailable; }
	bool				IsAskedForAnotherFile() const;
};

class CPartFileDLItem : public CMuleCtrlItem
{
public:
	typedef std::vector<CSourceDLItem*>				SourceItemVector;

	struct
	{
		bool			m_bSrcsAreVisible;
		bool			m_bShowUploadingSources : 1;
		bool			m_bShowOnQueueSources : 1;
		bool			m_bShowFullQueueSources : 1;
		bool			m_bShowConnectedSources : 1;
		bool			m_bShowConnectingSources : 1;
		bool			m_bShowNNPSources : 1;
		bool			m_bShowWaitForFileReqSources : 1;
		bool			m_bShowLowToLowIDSources : 1;
		bool			m_bShowLowIDOnOtherSrvSources : 1;
		bool			m_bShowBannedSources : 1;
		bool			m_bShowErrorSources : 1;
		bool			m_bShowA4AFSources : 1;
		bool			m_bShowUnknownSources : 1;
	};

private:
	typedef map<CUpDownClient*, CSourceDLItem*>	DLSourceMap;
	typedef DLSourceMap::iterator				SourceIter;
	typedef pair<SourceIter,SourceIter>			SourceRange;

	CPartFile		   *m_pFile;
	DLSourceMap			m_mapSources;

	void				RemoveAllSources(void);

public:
						CPartFileDLItem(CPartFile *pPartFile);
	virtual			   ~CPartFileDLItem();

	CPartFile		   *GetFile() const { return m_pFile; }

	CSourceDLItem*		CreateSourceItem(CUpDownClient *pSource,bool bIsAvailable);
	bool				DeleteSourceItem(CSourceDLItem *pSourceItem);
	SourceIter			FindSourceItem(CUpDownClient *pSource) { return(m_mapSources.find(pSource)); }
	void				FilterNoSources();
	void				FilterAllSources();
	SourceItemVector   *GetSources();
};
