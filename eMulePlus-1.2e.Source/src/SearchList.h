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

typedef struct
{
	CString	strFileName;
	LPCTSTR	pcFileType;
	CString	strFakeCheck;
	CString	strFileHash;
	CString	strIndex;
	uint64	qwFileSize;
	uint32	dwSourceCount;
	uint32	dwCompleteSourceCount;
	uint32	dwSrvRating;
} SearchFileStruct;

class SearchList;

class CSearchFile : public CAbstractFile
{
	friend class CPartFile;
public:
	CSearchFile(CFile& in_data, ECodingFormat eCF, uint32 nSearchID, CUpDownClient* Sender = NULL, LPCTSTR pszDirectory = NULL);
	virtual ~CSearchFile();
	uint32	GetIntTagValue(byte tagname);
	bool	GetIntTagValue(byte tagname, uint32 *pdwOut);
	uint64	GetInt64TagValue(byte tagname);
	void	SetIntTagValue(byte tagname, uint32 dwVal);
	CString	GetStrTagValue(byte tagname);
	void	AddExistentIntTag(byte tagname, uint32 dwInc);
	void	AddCompleteSources(uint32 in_dwCount);
	void	UpdateLastSeenComplete(uint32 dwSeconds);
	void	UpdateSrvRating(CSearchFile *pAdd);
	uint32	GetSourceCount()				{ return GetIntTagValue(FT_SOURCES); }
	uint32	GetCompleteSourceCount()		{ return GetIntTagValue(FT_COMPLETE_SOURCES); }
	uint32	GetLastSeenCompleteValue()		{ uint32 dwVal = 0x7FFFFFFF; GetIntTagValue(FT_LASTSEENCOMPLETE, &dwVal); return dwVal; }
	uint32	GetSearchID()					{ return m_nSearchID; }
	uint32	GetClientHybridID() const		{ return m_dwClientIDHybrid; }
	uint32	GetClientID() const				{ return m_dwClientID; }
	uint16	GetClientPort()					{ return m_uClientPort; }
	uint32	GetClientServerIP()				{ return m_nClientServerIP; }
	uint16	GetClientServerPort()			{ return m_nClientServerPort; }
	EnumSearchFileTypes GetType()			{ return m_eType; }
	CString	GetSearchFileDir()				{ return m_strSearchFileDirectory; }
	uint32	GetMediaLength()				{ return GetIntTagValue(FT_MEDIA_LENGTH); }
	uint32	GetMediaBitrate()				{ return GetIntTagValue(FT_MEDIA_BITRATE); }
	CString	GetMediaCodec()					{ return GetStrTagValue(FT_MEDIA_CODEC); }
	int		GetSrvFileRating() const		{ return m_iSrvRating; }
	void	GetSrvFileRatingEx(double *pdRating, uint32 *pdwUsers) const	{ *pdRating = m_dSrvRating; *pdwUsers = m_dwVoters; }

private:
	uint32	m_dwClientIDHybrid;
	uint32	m_dwClientID;
	uint32	m_nSearchID;
	uint16	m_uClientPort;

	uint16	m_nClientServerPort;
	uint32	m_nClientServerIP;
	CString	m_strSearchFileDirectory;
	int		m_iSrvRating;	// integer server rating to draw an image
	double	m_dSrvRating;	// float server rating to estimate and show overall rating
	uint32	m_dwVoters;		// estimated value of voted users
	EnumSearchFileTypes m_eType;
};

class CSearchList
{
friend class CSearchListCtrl;
public:
	CSearchList();
	~CSearchList();
	void	Clear();
	void	NewSearch(CSearchListCtrl *in_wnd, const CString &strTypes, uint32 dwSearchID, bool MobilMuleSearch = false);
	uint16	ProcessSearchAnswer(char* packet, uint32 size, ECodingFormat eCF, bool* pbIsMoreResultsAvailable = NULL);
	void	ProcessSharedFileListAnswer(byte *pbytePacket, uint32 size, CUpDownClient* Sender, LPCTSTR pszDirectory = NULL, bool bFirstDir = false);
	bool	AllowUDPSearchAnswer();
	uint16	ProcessUDPSearchAnswer(CMemFile &pckStream, ECodingFormat eCF);
	void	SetOutputWnd(CSearchListCtrl* in_wnd)		{ m_pctlSearchList = in_wnd; }
	void	RemoveResults(uint32 nSearchID);
	void	RemoveResults(CSearchFile* todel);
	void	ShowResults(uint32 nSearchID);
	CString	GetWebList(const CString &strLinePattern, int iSortBy, bool bAscending, bool bShowColumn1, bool bShowColumn2, bool bShowColumn3, bool bShowColumn4, bool bShowColumn5) const;
	void	AddFileToDownloadByHash(uchar* hash);
	void	AddFileToDownloadByHash(uchar* hash, EnumCategories eCatID);

	uint16	GetFoundFiles(uint32 searchID) {
		uint16 returnVal;
		foundFilesCount.Lookup(searchID,returnVal);
		return returnVal;
	}
	// mobilemule
	CSearchFile*	DetachNextFile(uint32 nSearchID);

private:
	bool	AddToList(CSearchFile *pAddedFile, bool bClientResponse);

	CTypedPtrList<CPtrList, CSearchFile*> list;
	CMap<uint32, uint32, uint16, uint16> foundFilesCount;

	CSearchListCtrl		*m_pctlSearchList;
	CString				m_strResultType;
	uint32				m_dwCurrentSearchCount;
	bool				m_bMobilMuleSearch;
};
