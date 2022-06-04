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

// Extended Prompt on Exit dialog [leuk_he] - Stulle

class CAskExitDlg : public CDialog
{
	DECLARE_DYNAMIC(CAskExitDlg)

public:
	CAskExitDlg();   // standard constructor
	virtual ~CAskExitDlg();
	virtual BOOL  OnInitDialog() ;
// Dialog Data
	enum { IDD = IDD_ASKEXIT };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedYes();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedYesservice();
	afx_msg void OnBnClickedNominimize();
};