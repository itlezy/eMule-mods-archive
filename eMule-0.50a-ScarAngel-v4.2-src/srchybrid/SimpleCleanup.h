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

class CSimpleCleanupDialog : public CDialog
{
	DECLARE_DYNAMIC(CSimpleCleanupDialog)

public:
	CSimpleCleanupDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CSimpleCleanupDialog();
	virtual BOOL OnInitDialog();
// Dialog Data
	enum { IDD = IDD_SIMPLECLEANUP };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
	CListCtrl* m_ReplaceListBox;
	void ReadConfig ();
	void WriteConfig ();
	int m_options;
	CString m_searchfor;
	CString m_replaceby;
	CString m_searchforchars;
	CString m_replacebychars;

	void Localize ();
public:

	void GetConfig (int& _options, CString& _searchfor, CString& _replaceby,
					CString& _searchforchars, CString& _replacebychars);
	void SetConfig (int _options, CString _searchfor, CString _replaceby,
					CString _searchforchars, CString _replacebychars);

public:
	afx_msg void OnBnClickedNewcharacter();
	afx_msg void OnBnClickedDeletecharacter();
	afx_msg void OnLvnItemchangedReplacecharslist(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedButtonedit();
protected:
	virtual void OnOK();
};

// Constants to configure simple cleanup; can be combined by || for the 'options' field
// of SimpleCleanupFilename.
#define SCO_DOTTOSPACE           (1)
#define SCO_UNDERLINETOSPACE     (2)
#define SCO_REPLACEAPOSTROPHE    (4)
#define SCO_REPLACEHEX		     (8)
#define SCO_SEARCHANDREPLACE    (16)
#define SCO_SEARCHCASESENSITIVE (32)
#define SCO_REPLACECHARS        (64)

CString SimpleCleanupFilename (CString _filename, int _options, 
							   CString _searchfor, CString _replaceby,
							   CString _searchforchars, CString _replacebychars );
