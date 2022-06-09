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
#include "Category.h"
#include "SourceSaver.h"
#include "Loggable.h"

class CServer;
class CPartFile;
class CKnownFile;
class CSearchFile;
class CUpDownClient;
class CClientSource;
class CHostnameSourceWnd;
class CSharedFileList;

class CDownloadQueue : public CLoggable
{
	friend class CWebServer;
public:
			CDownloadQueue(CSharedFileList* in_sharedfilelist);
		   ~CDownloadQueue();
	void	Process();
	void	Init();
	void	AddSearchToDownload(CSearchFile *pNewSearchFile, EnumCategories eCatID = CAT_NONE, bool bPaused = false);
	void	AddFileLinkToDownload(class CED2KFileLink *pLink, EnumCategories eCatID = CAT_NONE);
	bool	FileExists(const uchar* fileid);
	bool	IsInDLQueue(CKnownFile *pFileToTest);
	CPartFile*	GetFileByID(const uchar* filehash);
	CPartFile*	GetFileByIndex(int index);
	CUpDownClient* CheckAndAddSource(CPartFile *pSenderFile, uint32 dwUserIDHyb, uint16 uUserPort, uint32 dwSrvIP, uint16 uSrvPort, uchar *pbyteUserHash);
	bool	CheckAndAddKnownSource(CPartFile* pPartFile, CUpDownClient* pKnownSource);
	void	RemoveSource(CUpDownClient *toremove, bool updatewindow = true);
	void	DeleteAll();
	void	ResumeFiles();
	void	RemoveFile(CPartFile* toremove);
	uint32	GetDataRate() const							{return m_dwAverDataRate;}
	void	SortByPriority();

	CServer* pCurUDPServer;
	void	StopUDPRequests();
	
	void	GetDownloadStats(uint32 adwSrc[], uint64 aqwData[]);
	void	AddPartFilesToShare();
	void	AddDownload(CPartFile* newfile, bool bPaused = false);
	CUpDownClient* GetDownloadClientByIP_UDP(uint32 dwIP, uint16 nUDPPort);
	void	AddDownDataOverheadSourceExchange(uint32 data)	{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadSourceExchange += data;
															  m_nDownDataOverheadSourceExchangePackets++;}
	void	AddDownDataOverheadFileRequest(uint32 data)		{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadFileRequest += data;
															  m_nDownDataOverheadFileRequestPackets++;}
	void	AddDownDataOverheadServer(uint32 data)			{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadServer += data;
															  m_nDownDataOverheadServerPackets++;}
	void	AddDownDataOverheadOther(uint32 data)			{ m_nDownDataRateMSOverhead += data;
															  m_nDownDataOverheadOther += data;
															  m_nDownDataOverheadOtherPackets++;}
	uint32	GetDownDataRateOverhead()					{return m_nDownDataRateOverhead;}
	uint64	GetDownDataOverheadSourceExchange()			{return m_nDownDataOverheadSourceExchange;}
	uint64	GetDownDataOverheadFileRequest()			{return m_nDownDataOverheadFileRequest;}
	uint64	GetDownDataOverheadServer()					{return m_nDownDataOverheadServer;}
	uint64	GetDownDataOverheadOther()					{return m_nDownDataOverheadOther;}
	uint64	GetDownDataOverheadSourceExchangePackets()	{return m_nDownDataOverheadSourceExchangePackets;}
	uint64	GetDownDataOverheadFileRequestPackets()		{return m_nDownDataOverheadFileRequestPackets;}
	uint64	GetDownDataOverheadServerPackets()			{return m_nDownDataOverheadServerPackets;}
	uint64	GetDownDataOverheadOtherPackets()			{return m_nDownDataOverheadOtherPackets;}
	void	CompDownDataRateOverhead();

	int		GetFileCount()								{return m_partFileList.GetCount();}
	int		GetTransferringFiles() const;
	int		GetPausedFileCount() const;
	int		GetStoppedFileCount() const;
	int		GetActiveFileCount() const;
	void	StartNextFile(EnumCategories eCatID);

	CPartFile* 	GetA4AFAutoFile()						{return m_A4AF_auto_file;}
	void	SetA4AFAutoFile(CPartFile* file);

	void	AddClientHostnameToResolve(CTypedPtrList<CPtrList, CClientSource*>* pLink);
	void	ResolveNextSourceHostname();
	void	SourceHostnameResolved(WPARAM wp, LPARAM lp);
	void	GetUDPSearchStatus(CString *pstrOut);

	void	ResetCatParts(int cat);
	void	SetCatPrio(int cat, byte newprio);
	void	SetCatStatus(int cat, uint16 newstatus);
	void	SetAutoCat(CPartFile* newfile);

	void	SetRequiredSourcesRefresh(CPartFile* pFile)		{if (!m_LocalServerSourcesReqQueue.Find(pFile)) m_LocalServerSourcesReqQueue.AddTail(pFile);}
	void	ResetLastTCPRequestTime()						{m_dwLastTCPSourcesRequestTime = 0;}
	void	SetLastTCPSrcReqServer(CServer* pServer)		{if (pServer) m_pLastTCPSrcReqServer = pServer;}
	CServer*	GetLastTCPSrcReqServer()					{return m_pLastTCPSrcReqServer;}
	void	SetAutoSourcesPerFile();
	void	SaveAllSLSFiles();

	void	SetBanCount(uint32 dwNewValue)					{ m_dwBannedCounter = dwNewValue; }
	void	UpdateSourceStatesAfterServerChange();

	CSourceSaver m_sourcesaver;
protected:
	bool	SendNextUDPPacket();

private:
	void				SaveAllPartFileStats();

private:
	std::deque<uint64>		m_averageDataRateList;
	std::deque<DWORD>		m_averageTickList;
	CTypedPtrList<CPtrList, CPartFile*> m_partFileList;
	CSharedFileList	   *m_pSharedFileList;
	uint32				m_dwDataRate;
	unsigned			m_uiUDCounter;

	uint32				m_dwLastUDPSearchTime;
	uint32				m_dwLastUDPStatTime;
	CPartFile			*m_pLastUDPSearchedFile;

	uint32				m_nDownDataRateOverhead;
	uint32				m_nDownDataRateMSOverhead;
	uint64				m_nDownDataOverheadSourceExchange;
	uint64				m_nDownDataOverheadSourceExchangePackets;
	uint64				m_nDownDataOverheadFileRequest;
	uint64				m_nDownDataOverheadFileRequestPackets;
	uint64				m_nDownDataOverheadServer;
	uint64				m_nDownDataOverheadServerPackets;
	uint64				m_nDownDataOverheadOther;
	uint64				m_nDownDataOverheadOtherPackets;

	std::deque<uint32>	m_DownDataOverheadDeque;	//Down Data Rate Overhead
	uint32				m_dwSumDownDataOverheadInDeque;

	uint32				m_dwAverDataRate;
	uint32				m_lastPartFileStatsSave;

	uint32				m_dwBannedCounter;

//	Hostname sources in ED2K superLink
	CTypedPtrList<CPtrList, CClientSource*>		hostnameResolveQueue;
	CHostnameSourceWnd	*m_wndHSCallback;
	char				hostentBuffer[MAXGETHOSTSTRUCT];
	byte				sourceHostnameResolveRetry;
	bool				m_bIsResolving;

	bool				m_bIsInitialized;
	CPartFile		   *m_A4AF_auto_file;

	CTypedPtrList<CPtrList, CPartFile*>	m_LocalServerSourcesReqQueue;
	DWORD				m_dwLastTCPSourcesRequestTime;
	CServer			*m_pLastTCPSrcReqServer;
};

//	Hostname sources in ED2K superLink
class CHostnameSourceWnd : public CWnd
{
public:
					CHostnameSourceWnd();

CDownloadQueue	   *m_pOwner;

protected:
DECLARE_MESSAGE_MAP()

afx_msg LRESULT		OnSourceHostnameResolved(WPARAM wParam, LPARAM lParam);
};
