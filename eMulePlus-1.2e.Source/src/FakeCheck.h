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

//	General idea based on eMule Morph's CFakecheck

#pragma once
#include "types.h"
#include "ListenSocket.h"
#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <map>
#pragma warning(pop)

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
//	From unrar.h (http://www.rarlab.com/rar/UnRARDLL.exe)
//	for fakes.rar support

#define ERAR_END_ARCHIVE     10
#define ERAR_NO_MEMORY       11
#define ERAR_BAD_DATA        12
#define ERAR_BAD_ARCHIVE     13
#define ERAR_UNKNOWN_FORMAT  14
#define ERAR_EOPEN           15
#define ERAR_ECREATE         16
#define ERAR_ECLOSE          17
#define ERAR_EREAD           18
#define ERAR_EWRITE          19
#define ERAR_SMALL_BUF       20
#define ERAR_UNKNOWN         21

#define RAR_OM_LIST           0
#define RAR_OM_EXTRACT        1

#define RAR_SKIP              0
#define RAR_TEST              1
#define RAR_EXTRACT           2

#define RAR_VOL_ASK           0
#define RAR_VOL_NOTIFY        1

#define RAR_DLL_VERSION       4

#pragma pack(1)
struct RARHeaderData
{
	char         ArcName[260];
	char         FileName[260];
	unsigned int Flags;
	unsigned int PackSize;
	unsigned int UnpSize;
	unsigned int HostOS;
	unsigned int FileCRC;
	unsigned int FileTime;
	unsigned int UnpVer;
	unsigned int Method;
	unsigned int FileAttr;
	char         *CmtBuf;
	unsigned int CmtBufSize;
	unsigned int CmtSize;
	unsigned int CmtState;
};

struct RAROpenArchiveDataEx
{
	const char   *ArcName;
	const wchar_t *ArcNameW;
	unsigned int OpenMode;
	unsigned int OpenResult;
	char         *CmtBuf;
	unsigned int CmtBufSize;
	unsigned int CmtSize;
	unsigned int CmtState;
	unsigned int Flags;
	unsigned int Reserved[32];
};
#pragma pack()

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////

typedef std::map<CString, CString>	_mapFakeMap;

class CFakeCheck
{
public:
	CFakeCheck();
	~CFakeCheck();

	void	Init(void);
	INT		LoadFromDatFile();
	void	GetFakeComment(const CString &strFileHash, const uint64 qwFileLenght, CString *pstrComment);
	BOOL	UpdateFakeList();
	BOOL	ExtractRARArchive(const CString &strArchivePath, CString strDestFolder);

private:
	bool	AddFake(const CString &strHash, const CString &strLenght, const CString &strRealTitle);
	void	RemoveAllFakes();
	BOOL	RetrieveFakesDotTxt(OUT uint32 &dwVersion, OUT CString &strED2KLink);

	void	OutOpenArchiveError(int iError, const CString& strArchivePath);
	void	OutProcessFileError(int iError);

	_mapFakeMap	m_mapFakeMap;

//	RAR support
	HANDLE	(WINAPI *pfnRAROpenArchiveEx)(RAROpenArchiveDataEx *pArchiveData);
	int		(WINAPI *pfnRARCloseArchive)(HANDLE hArcData);
	int		(WINAPI *pfnRARReadHeader)(HANDLE hArcData, RARHeaderData *pHeaderData);
	int		(WINAPI *pfnRARProcessFile)(HANDLE hArcData, int iOperation, LPTSTR strDestFolder, char* strDestName);
};

/////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////
