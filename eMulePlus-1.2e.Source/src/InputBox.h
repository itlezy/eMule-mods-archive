//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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

class InputBox : public CDialog
{
	DECLARE_DYNAMIC(InputBox)

public:
	InputBox(const CString &strTitle, const CString &strDefText, bool bEditFilenameMode = false);
	virtual ~InputBox();
// Dialog Data
	enum { IDD = IDD_INPUTBOX };

	const CString& GetInput() const	{ return m_strReturn; }
	bool	WasCancelled() const	{ return m_bCancel; }

protected:
	virtual BOOL OnInitDialog();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	afx_msg void OnCleanFilename();
	afx_msg void OnOK();
	DECLARE_MESSAGE_MAP()

private:
	CString	m_strTitle;
	CString	m_strDefault;
	CString	m_strReturn;
	bool	m_bCancel;
	bool	m_bFilenameMode;
};
