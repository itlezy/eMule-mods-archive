#ifndef __ATLWFILE_H__
#define __ATLWFILE_H__

#pragma once

/////////////////////////////////////////////////////////////////////////////
// Windows File API wrappers
//
// Written by Bjarke Viksoe (bjarke@viksoe.dk)
// Copyright (c) 2001 Bjarke Viksoe.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name is included. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability if it causes any damage to you or your
// computer whatsoever. It's free, so don't hassle me about it.
//
// Beware of bugs.
//


#ifndef __cplusplus
   #error ATL requires C++ compilation (use a .cpp suffix)
#endif

#ifndef INVALID_SET_FILE_POINTER
   #define INVALID_SET_FILE_POINTER ((DWORD)-1)
#endif // INVALID_SET_FILE_POINTER

#ifndef _ATL_DLL_IMPL
namespace ATL
{
#endif


/////////////////////////////////////////////////////////////////
// Standard file wrapper

// Win32 File wrapper class
// Important: Don't make the destructor "virtual" because we need
//            the class v-table to look like the HANDLE type!
// NOTE: Win 95/98 only supports < 4Gb files; see Q250301

template< bool t_bManaged >
class CFileT
{
public:
   HANDLE m_hFile;

public:
   CFileT(HANDLE hFile = INVALID_HANDLE_VALUE) 
   {
      m_hFile = hFile;
   }
   CFileT(const CFileT<t_bManaged>& file) 
   {
      m_hFile = INVALID_HANDLE_VALUE;
      DuplicateHandle(file.m_hFile);
   }
   ~CFileT()
   { 
      if( t_bManaged ) Close(); 
   }
   operator HFILE() const 
   { 
      return (HFILE) m_hFile; 
   }
   operator HANDLE() const 
   { 
      return m_hFile; 
   }
   const CFileT<t_bManaged>& operator=(const CFileT<t_bManaged>& file)
   {
      DuplicateHandle(file.m_hFile);
      return *this;
   }
   BOOL Open(LPCTSTR pstrFileName, 
             DWORD dwAccess = GENERIC_READ, 
             DWORD dwShareMode = FILE_SHARE_READ, 
             DWORD dwFlags = OPEN_EXISTING,
             DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL)
   {
      _ASSERTE(!::IsBadStringPtr(pstrFileName,-1));
      Close();
      // Attempt file creation
      HANDLE hFile = ::CreateFile(pstrFileName, 
         dwAccess, 
         dwShareMode, 
         NULL,
         dwFlags, 
         dwAttributes, 
         NULL);
      if( hFile == INVALID_HANDLE_VALUE ) return FALSE;
      m_hFile = hFile;
      return TRUE;
   }
   BOOL Create(LPCTSTR pstrFileName,
               DWORD dwAccess = GENERIC_WRITE, 
               DWORD dwShareMode = 0 /*DENY ALL*/, 
               DWORD dwFlags = CREATE_ALWAYS,
               DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL)
   {
      return Open(pstrFileName, dwAccess, dwShareMode, dwFlags, dwAttributes);
   }
   void Close()
   {
      if( m_hFile == INVALID_HANDLE_VALUE ) return;
      ::CloseHandle(m_hFile);
      m_hFile = INVALID_HANDLE_VALUE;
   }
   BOOL IsOpen() const
   {
      return m_hFile != INVALID_HANDLE_VALUE;
   }
   void Attach(HANDLE hHandle)
   {
      Close();
      m_hFile = hHandle;
   }   
   HANDLE Detach()
   {
      HANDLE h = m_hFile;
      m_hFile = INVALID_HANDLE_VALUE;
      return h;
   }
   BOOL Read(LPVOID lpBuf, DWORD nCount)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf!=NULL);
      _ASSERTE(!::IsBadWritePtr(lpBuf, nCount));
      if( nCount==0 ) return TRUE;   // avoid Win32 "null-read"
      DWORD dwRead;
      if( !::ReadFile(m_hFile, lpBuf, nCount, &dwRead, NULL) ) return FALSE;
      // Win32s will not return an error all the time (usually DISK_FULL)
      return nCount == dwRead;
   }
   BOOL Read(LPVOID lpBuf, DWORD nCount, LPDWORD pdwRead)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf);
      _ASSERTE(!::IsBadWritePtr(lpBuf, nCount));
      _ASSERTE(pdwRead);
      *pdwRead = 0;
      if( nCount==0 ) return TRUE;   // avoid Win32 "null-read"
      if( !::ReadFile(m_hFile, lpBuf, nCount, pdwRead, NULL) ) return FALSE;
      // Win32s will not return an error all the time (usually DISK_FULL)
      return nCount == *pdwRead;
   }
   BOOL Write(LPCVOID lpBuf, DWORD nCount)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf!=NULL);
      _ASSERTE(!::IsBadReadPtr(lpBuf, nCount));   
      if( nCount == 0 ) return TRUE; // avoid Win32 "null-write" option
      DWORD dwWritten;
      if( !::WriteFile(m_hFile, lpBuf, nCount, &dwWritten, NULL) ) return FALSE;
      // Win32s will not return an error all the time (usually DISK_FULL)
      return nCount == dwWritten;
   }
   BOOL Write(LPCVOID lpBuf, DWORD nCount, LPDWORD pdwWritten)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      _ASSERTE(lpBuf);
      _ASSERTE(!::IsBadReadPtr(lpBuf, nCount));
      _ASSERTE(pdwWritten);    
      *pdwWritten = 0;
      if( nCount == 0 ) return TRUE; // avoid Win32 "null-write" option
      if( !::WriteFile(m_hFile, lpBuf, nCount, pdwWritten, NULL) ) return FALSE;
      // Win32s will not return an error all the time (usually DISK_FULL)
      return nCount == *pdwWritten;
   }
   DWORD Seek(LONG lOff, UINT nFrom)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      DWORD dwNew = ::SetFilePointer(m_hFile, lOff, NULL, (DWORD) nFrom);
      if( dwNew == INVALID_SET_FILE_POINTER ) return (DWORD) -1;
      return dwNew;
   }
   DWORD GetPosition() const
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      DWORD dwPos = ::SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
      if( dwPos == INVALID_SET_FILE_POINTER ) return (DWORD) -1;
      return dwPos;
   }
   BOOL Lock(DWORD dwOffset, DWORD dwSize)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::LockFile(m_hFile, dwOffset, 0, dwSize, 0);
   }
   BOOL Unlock(DWORD dwOffset, DWORD dwSize)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::UnlockFile(m_hFile, dwOffset, 0, dwSize, 0);
   }
   BOOL SetEOF()
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::SetEndOfFile(m_hFile);
   }
   BOOL Flush()
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::FlushFileBuffers(m_hFile);
   }
   DWORD GetSize() const
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::GetFileSize(m_hFile, NULL);
   }
   DWORD GetType() const
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::GetFileType(m_hFile);
   }
   BOOL GetFileTime(FILETIME* ftCreate, FILETIME* ftAccess, FILETIME* ftModified)
   {
      _ASSERTE(m_hFile!=INVALID_HANDLE_VALUE);
      return ::GetFileTime(m_hFile, ftCreate, ftAccess, ftModified);
   }
   BOOL DuplicateHandle(HANDLE hOther)
   {
      ATLASSERT(m_hFile==INVALID_HANDLE_VALUE);
      ATLASSERT(hOther!=INVALID_HANDLE_VALUE);
      HANDLE process = ::GetCurrentProcess();
      BOOL res = ::DuplicateHandle(process, hOther, process, &m_hFile, NULL, FALSE, DUPLICATE_SAME_ACCESS);
      _ASSERTE(res);
      return res;
   }
   static BOOL FileExists(LPCTSTR pstrFileName)
   {
      _ASSERTE(!::IsBadStringPtr(pstrFileName, MAX_PATH));
      return ::GetFileAttributes(pstrFileName) != 0xFFFFFFFF;
   }
   static BOOL Delete(LPCTSTR pstrFileName)
   {
      _ASSERTE(!::IsBadStringPtr(pstrFileName, MAX_PATH));
      return ::DeleteFile(pstrFileName);
   }
   static BOOL Rename(LPCTSTR pstrSourceFileName, LPCTSTR pstrTargetFileName)
   {
      _ASSERTE(!::IsBadStringPtr(pstrSourceFileName, MAX_PATH));
      _ASSERTE(!::IsBadStringPtr(pstrTargetFileName, MAX_PATH));
      return ::MoveFile(pstrSourceFileName, pstrTargetFileName);
   }
};

typedef CFileT<true> CFile;
typedef CFileT<false> CFileHandle;


/////////////////////////////////////////////////////////////////
// Temporary file (temp filename and auto delete)

class CTemporaryFile : public CFileT<true>
{
public:
   TCHAR m_szFileName[MAX_PATH];
public:
   ~CTemporaryFile()
   { 
      Close();
      Delete(m_szFileName);
   };
   BOOL Create(LPTSTR pstrFileName, 
               UINT cchFilename,
               DWORD dwAccess = GENERIC_WRITE, 
               DWORD dwShareMode = 0 /*DENY ALL*/, 
               DWORD dwFlags = CREATE_ALWAYS,
               DWORD dwAttributes = FILE_ATTRIBUTE_NORMAL)
   {
      _ASSERTE(!::IsBadStringPtr(pstrFileName,cchFilename));
      // If a valid filename buffer is supplied we'll create
      // and return a new temporary filename.
      if( cchFilename > 0 ) {
         ::GetTempPath(cchFilename, pstrFileName);
         ::GetTempFileName(pstrFileName, _T("BV"), 0, pstrFileName);
      }
      ::lstrcpy(m_szFileName, pstrFileName);
      return Open(pstrFileName, dwAccess, dwShareMode, dwFlags, dwAttributes);
   }
};


#ifndef _ATL_DLL_IMPL
}; //namespace ATL
#endif


#endif // __ATLWFILE_H___
