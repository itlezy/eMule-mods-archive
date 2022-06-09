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
#include "afx.h"

#define	MAX_CFEXP_ERRORMSG	(MAX_PATH + 256)

class CSafeFile : public CFile{
public:
	CSafeFile();
	CSafeFile::CSafeFile(LPCTSTR lpszFileName, UINT nOpenFlags);
	virtual UINT Read(void* lpBuf,UINT nCount);
};

class CSafeMemFile : public CMemFile{
public:
	CSafeMemFile(UINT nGrowBytes = 0);
	CSafeMemFile::CSafeMemFile(BYTE* lpBuffer,UINT nBufferSize,UINT nGrowBytes = 0);
	virtual UINT Read(void* lpBuf,UINT nCount);
};

class CSafeBufferedFile : public CStdioFile{
public:
	CSafeBufferedFile();
	CSafeBufferedFile::CSafeBufferedFile(LPCTSTR lpszFileName, UINT nOpenFlags);
	virtual UINT Read(void* lpBuf,UINT nCount);
};