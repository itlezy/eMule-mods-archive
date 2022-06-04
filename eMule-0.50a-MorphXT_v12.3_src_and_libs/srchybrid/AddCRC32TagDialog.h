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

// Original file: Written by Mighty Knife, EMule Morph Team

#pragma once

#include "FileProcessing.h"

class AddCRC32InputBox : public CDialog
{
	DECLARE_DYNAMIC(AddCRC32InputBox)

public:
	AddCRC32InputBox(CWnd* pParent = NULL);   // standard constructor
	virtual ~AddCRC32InputBox();
	virtual BOOL OnInitDialog();
	afx_msg void OnOK();
	afx_msg void OnCancel();
	enum { IDD = IDD_ADDCRC32TAG };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	CString m_CRC32Prefix;
	CString m_CRC32Suffix;
	bool    m_DontAddCRC32;
	bool    m_CRC32ForceUppercase;
	bool    m_CRC32ForceAdding;
public:
	CString GetCRC32Prefix () { return m_CRC32Prefix; }
	CString GetCRC32Suffix () { return m_CRC32Suffix; }
	bool	GetDontAddCRC32 () { return m_DontAddCRC32; }
	bool	GetCRC32ForceUppercase () { return m_CRC32ForceUppercase; }
	bool	GetCRC32ForceAdding () { return m_CRC32ForceAdding; }
private:
};

class CCRC32RenameWorker : public CFileProcessingWorker {
	DECLARE_DYNCREATE(CCRC32RenameWorker)
public:
	CString m_FilenamePrefix;
	CString m_FilenameSuffix;
	bool m_DontAddCRCAndSuffix;
	bool m_CRC32ForceUppercase;
	bool m_CRC32ForceAdding;

	CCRC32RenameWorker() { m_DontAddCRCAndSuffix=false; m_CRC32ForceUppercase = false; }
	virtual void SetDontAddCRCAndSuffix (bool _b) { m_DontAddCRCAndSuffix = _b; }
	virtual void SetCRC32ForceUppercase (bool _b) { m_CRC32ForceUppercase = _b; }
	virtual void SetCRC32ForceAdding (bool _b) { m_CRC32ForceAdding = _b; }
	virtual void SetFilenamePrefix (CString _s) { m_FilenamePrefix = _s; }
	virtual void SetFilenameSuffix (CString _s) { m_FilenameSuffix = _s; }
	virtual void Run ();
private:
};

class CCRC32CalcWorker : public CFileProcessingWorker {
	DECLARE_DYNCREATE(CCRC32CalcWorker)
public:
	CCRC32CalcWorker() { }
	virtual void Run ();
private:
};
