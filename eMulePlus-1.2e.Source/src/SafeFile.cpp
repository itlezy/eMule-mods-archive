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

#include "StdAfx.h"
#include "safefile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


CSafeFile::CSafeFile(LPCTSTR lpszFileName, UINT nOpenFlags)
	:CFile(lpszFileName, nOpenFlags)
{}

CSafeFile::CSafeFile() : CFile() {}

	
UINT CSafeFile::Read(void* lpBuf,UINT nCount){
	if (GetPosition()+nCount > GetLength())
		AfxThrowFileException(CFileException::endOfFile,0,GetFileName());
	return CFile::Read(lpBuf,nCount);
}


CSafeMemFile::CSafeMemFile(BYTE* lpBuffer,UINT nBufferSize,UINT nGrowBytes)
	:CMemFile(lpBuffer,nBufferSize,nGrowBytes)
{}

CSafeMemFile::CSafeMemFile(UINT nGrowBytes) : CMemFile(nGrowBytes) {}

	
UINT CSafeMemFile::Read(void* lpBuf,UINT nCount){
	if (GetPosition()+nCount > this->GetLength())
		AfxThrowFileException(CFileException::endOfFile,0,GetFileName());
	return CMemFile::Read(lpBuf,nCount);
}

CSafeBufferedFile::CSafeBufferedFile(LPCTSTR lpszFileName, UINT nOpenFlags)
	:CStdioFile(lpszFileName, nOpenFlags)
{}

CSafeBufferedFile::CSafeBufferedFile() : CStdioFile() {}

	
UINT CSafeBufferedFile::Read(void* lpBuf,UINT nCount){
	// that's terrible slow
//	if (GetPosition()+nCount > this->GetLength())
//		AfxThrowFileException(CFileException::endOfFile,0,GetFileName());
	UINT uRead = CStdioFile::Read(lpBuf,nCount);
	if (uRead != nCount)
		AfxThrowFileException(CFileException::endOfFile,0,GetFileName());
	return uRead;
}