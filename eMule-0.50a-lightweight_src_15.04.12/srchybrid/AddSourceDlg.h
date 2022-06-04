//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "ResizableLib/ResizableDialog.h"
#include "Neo\ModeLess.h" // NEO: MLD - [ModelesDialogs] <-- Xanatos --

class CPartFile;

struct SUnresolvedHostname
{
	SUnresolvedHostname()
	{
		nPort = 0;
	}
	CStringA strHostname;
	uint16 nPort;
};

// CAddSourceDlg dialog

class CAddSourceDlg : public CModResizableDialog // NEO: MLD - [ModelesDialogs] <-- Xanatos --
{
	DECLARE_DYNAMIC(CAddSourceDlg)

public:
	CAddSourceDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAddSourceDlg();

	void SetFile( CPartFile* pFile );

// Dialog Data
	enum { IDD = IDD_ADDSOURCE };

protected:
	CPartFile* m_pFile;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedOk();
};
