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

#include "stdafx.h"
#include "resource.h"
#include "OtherFunctions.h"
#include "SimpleCleanup.h"
#include ".\simplecleanup.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// CSimpleCleanupDialog dialog

IMPLEMENT_DYNAMIC(CSimpleCleanupDialog, CDialog)

BEGIN_MESSAGE_MAP(CSimpleCleanupDialog, CDialog)
	ON_BN_CLICKED(IDC_NEWCHARACTER, OnBnClickedNewcharacter)
	ON_BN_CLICKED(IDC_DELETECHARACTER, OnBnClickedDeletecharacter)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_REPLACECHARSLIST, OnLvnItemchangedReplacecharslist)
	ON_BN_CLICKED(IDC_BUTTONEDIT, OnBnClickedButtonedit)
	ON_WM_CLOSE()
END_MESSAGE_MAP()

CSimpleCleanupDialog::CSimpleCleanupDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CSimpleCleanupDialog::IDD, pParent)
{
}

CSimpleCleanupDialog::~CSimpleCleanupDialog()
{
}

void CSimpleCleanupDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CSimpleCleanupDialog::Localize () {
	SetWindowText (GetResString (IDS_SC_HEADLINE));
	GetDlgItem (IDC_SC_STATIC1)->SetWindowText (GetResString (IDS_SC_SEARCHFOR));
	GetDlgItem (IDC_SC_STATIC2)->SetWindowText (GetResString (IDS_SC_REPLACEBY));
	GetDlgItem (IDC_SC_STATIC3)->SetWindowText (GetResString (IDS_REPLACECHAR));
	GetDlgItem (IDC_SC_STATIC4)->SetWindowText (GetResString (IDS_REPLACECHARBY));
	GetDlgItem (IDC_CHECKDOTTOSPACE)->SetWindowText (GetResString (IDS_CHECKDOTTOSPACE));
	GetDlgItem (IDC_CHECKUNDERLINETOSPACE)->SetWindowText (GetResString (IDS_CHECKUNDERLINETOSPACE));
	GetDlgItem (IDC_CHECKCONVERTAPOSTROPHES)->SetWindowText (GetResString (IDS_CHECKCONVERTAPOSTROPHES));
	GetDlgItem (IDC_CHECKCONVERTHEX)->SetWindowText (GetResString (IDS_CHECKCONVERTHEX));
	GetDlgItem (IDC_CHECKSEARCHANDREPLACE)->SetWindowText (GetResString (IDS_CHECKSEARCHANDREPLACE));
	GetDlgItem (IDC_CHECKCASESENSITIVE)->SetWindowText (GetResString (IDS_CHECKCASESENSITIVE));
	GetDlgItem (IDC_CHECKCHARACTERREPLACE)->SetWindowText (GetResString (IDS_CHECKCHARACTERREPLACE));
	GetDlgItem (IDC_NEWCHARACTER)->SetWindowText (GetResString (IDS_SC_ADD));
	GetDlgItem (IDC_BUTTONCHANGE)->SetWindowText (GetResString (IDS_SC_EDIT));
	GetDlgItem (IDC_DELETECHARACTER)->SetWindowText (GetResString (IDS_SC_DELETE));
	GetDlgItem (IDCANCEL)->SetWindowText (GetResString (IDS_CANCEL));
}

BOOL CSimpleCleanupDialog::OnInitDialog() {
	CDialog::OnInitDialog();

	Localize ();

	InitWindowStyles(this);

	m_ReplaceListBox = (CListCtrl*) GetDlgItem (IDC_REPLACECHARSLIST);

	CRect R;
	m_ReplaceListBox->GetClientRect (R);
		
	m_ReplaceListBox->InsertColumn (0,GetResString (IDS_SEARCHFORCHAR),0,R.Width ()/2);
	m_ReplaceListBox->InsertColumn (1,GetResString (IDS_REPLACEBYCHAR),0,R.Width ()/2);

	WriteConfig ();

	return TRUE;
}

// Convert the string _str to a CStringList by extracting all the elements.
// _str is assumed to be a string formed by:
//      "str";"str";"str";...;"str"
// Because a filename can't contain a >"< character we are sure that no "str" element
// contains a >"<.
void StringToStringList (CString _str, CStringList& _slist) {
	// Empty the list
	_slist.RemoveAll ();
	// Convert to a list
	int start=0, end=-1;
	do {
		// Find the start of the string, denoted by ";".
		// We start the search after the end of the previous string - any
		// characters inbetween (like ';') will be ignored.
		start = _str.Find ('\"',end+1);
		if (start != -1) {
			// ok, now the end
			end = _str.Find ('\"',start+1);
			if (end == -1) {
				// The last string in the list. Set "end" to the last character,
				// this is a >"< character
				end = _str.GetLength ()-1;
			}
			// Extract the string and store it; Remove the surrounding "".
			CString s = _str.Mid (start+1,end-start-1);
			_slist.AddTail (s);
		}
	} while (start != -1);
}


// Convert a CString list to a string in the form
//      "str";"str";"str";...;"str"
CString StringListToString (CStringList& _slist) {
	CString res;
	POSITION pos = _slist.GetHeadPosition ();
	// Add the first string of the list. Remove any '"' characters
	if (!_slist.IsEmpty ()) {
	  res = res + '\"' + _slist.GetNext (pos).SpanExcluding (_T("\"")) + '\"';
	}
	// Now add all the rest
	while (pos != NULL) {
	  res = res + _T(";\"") + _slist.GetNext (pos).SpanExcluding (_T("\"")) + _T('\"');
	}
    return res;
}

// Now the main replacement routine. This routine replaces all _source characters/strings
// by _dest characters/strings. 
void ReplaceChars (CString& _str, CString _source, CString _dest, bool _casesensitive) {
	// Duplicate the strings
	CString src2 = _source;
	CString str2 = _str;
	// Convert to uppercase if we have to work case-insensitive
	if (!_casesensitive) {
		str2.MakeUpper ();
		src2.MakeUpper ();
	} 

	// Make sure we don't have an empty source
	if ((str2 == "") || (src2=="")) return;

	// Iterate through the string
	int p=0;
	do {
		// Search for the source string
		p = str2.Find (src2,p);
		if (p != -1) {
			// got it - replace it in the original string and in the copy
			_str.Delete (p,src2.GetLength());
			str2.Delete (p,src2.GetLength());
			_str.Insert (p,_dest);
			str2.Insert (p,_dest);
			// Set the pointer after the inserted string to avoid recursive replacements
			p += _dest.GetLength ();
		}
	} while (p != -1);
}

// The replacement routine for hexadecimal numbers
void ReconstructCharacters (CString& _str) {
	int p=0;
	const CString hex = _T("0123456789ABCDEF");
	while (p < _str.GetLength ()) {
		// look for "%xx"-numbers
		if (_str [p]=='%') {
			CString newchar = _str.Mid (p+1,2).MakeUpper ();
			if (newchar.GetLength ()==2) {
				int c1 = hex.Find (newchar [0]);
				int c2 = hex.Find (newchar [1]);
				if ((c1 != -1) && (c2 != -1)) {
					// ok, we got a character from this hex number
					int c = (c1<<4) | c2;
					// basic check
					if (c > 31) {
						// Replace the "%" by the char, delete the two hexadecimal numbers
						_str.SetAt (p,c);
						_str.Delete (p+1,2);
					} 
				}
			}
		} 
		p++;
	}
}

// The next routine is the replacement wrapper. The routine iterates through all possible
// and (with the _options field) configured replacement to cleanup the given filename. 
// The return value is the new filename.
CString SimpleCleanupFilename (CString _filename, int _options, 
							   CString _searchfor, CString _replaceby,
							   CString _searchforchars, CString _replacebychars) {
  if (_filename==_T("")) return _T(""); // nothing to do
    
  // Separate the filename from its extension to ensure separate treatment!
  int lastdot = _filename.ReverseFind ('.');
  
  CString name, ext;
  if (lastdot==-1) {
	  name = _filename;
  } else {
	  name = _filename.Left (lastdot);
	  ext = _filename.Right (_filename.GetLength ()-lastdot-1);
  }

  // Remove dots
  if ((_options & SCO_DOTTOSPACE) != 0) {
	  ReplaceChars (name,_T("."),_T(" "),false);
	  ReplaceChars (ext,_T("."),_T(" "),false);
  }
  
  // Remove underlines
  if ((_options & SCO_UNDERLINETOSPACE) != 0) {
	  ReplaceChars (name,_T("_"),_T(" "),false);
	  ReplaceChars (ext,_T("_"),_T(" "),false);
  }

  // Correct apostrophes
  if ((_options & SCO_REPLACEAPOSTROPHE) != 0) {
	  ReplaceChars (name,_T("\xB4"),_T("'"),false);
	  ReplaceChars (ext,_T("\xB4"),_T("'"),false);
	  ReplaceChars (name,_T("`"),_T("'"),false);
	  ReplaceChars (ext,_T("`"),_T("'"),false);
  }

  // Rebuild characters from hexadecimal numbers
  if ((_options & SCO_REPLACEHEX) != 0) {
	  ReconstructCharacters (name);
	  ReconstructCharacters (ext);
  }

  // Standard search & replace
  if ((_options & SCO_SEARCHANDREPLACE) != 0) {
	  ReplaceChars (name,_searchfor,_replaceby,(_options & SCO_SEARCHCASESENSITIVE) != 0);
	  ReplaceChars (ext,_searchfor,_replaceby,(_options & SCO_SEARCHCASESENSITIVE) != 0);
  }

  // Search & replace single characters
  if ((_options & SCO_REPLACECHARS) != 0) {
	  CStringList searchforchars,replacebychars;
	  StringToStringList (_searchforchars,searchforchars);
	  StringToStringList (_replacebychars,replacebychars);
	  if ((searchforchars.GetCount () == replacebychars.GetCount()) &&
		  (searchforchars.GetCount () > 0)) {
		POSITION dpos=replacebychars.GetHeadPosition();
		for (POSITION pos=searchforchars.GetHeadPosition(); pos != NULL;) {
		CString src=searchforchars.GetNext(pos);
		CString dst=replacebychars.GetNext(dpos);
		ReplaceChars (name,src,dst,true);
		ReplaceChars (ext,src,dst,true);
		}
	  }
  }

  // Reconstruct the filename and return it to the caller
  if (lastdot==-1) {
	  return name;
  } else {
	  return name+_T(".")+ext;
  }
}


void CSimpleCleanupDialog::OnBnClickedNewcharacter()
{
	CString c1,c2;
	GetDlgItem (IDC_CHARSOURCE)->GetWindowText (c1);
	GetDlgItem (IDC_CHARDEST)->GetWindowText (c2);

	if (c1 != "") {
		// Add the element at the end of the list. The list is sorted anyway
		// so the element will be placed to a "good" position.
		int i=m_ReplaceListBox->GetItemCount ();
		i = m_ReplaceListBox->InsertItem (i,c1);
		m_ReplaceListBox->SetItemText (i,1,c2);

		GetDlgItem (IDC_CHARSOURCE)->SetWindowText (_T(""));
		GetDlgItem (IDC_CHARDEST)->SetWindowText (_T(""));

		GetDlgItem (IDC_CHARSOURCE)->SetFocus ();
	}
}

void CSimpleCleanupDialog::OnBnClickedDeletecharacter()
{
	POSITION pos;
	do {
		pos = m_ReplaceListBox->GetFirstSelectedItemPosition ();
		if (pos != NULL) {
			m_ReplaceListBox->DeleteItem (m_ReplaceListBox->GetNextSelectedItem (pos));
		}
	} while (pos != NULL);
}

void CSimpleCleanupDialog::OnLvnItemchangedReplacecharslist(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	//LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);

	POSITION pos = m_ReplaceListBox->GetFirstSelectedItemPosition ();
	if (pos != NULL) {
		int i=m_ReplaceListBox->GetNextSelectedItem (pos);
		GetDlgItem (IDC_CHARSOURCE)->SetWindowText (m_ReplaceListBox->GetItemText (i,0));
		GetDlgItem (IDC_CHARDEST)->SetWindowText (m_ReplaceListBox->GetItemText (i,1));
	}

	*pResult = 0;
}

void CSimpleCleanupDialog::OnBnClickedButtonedit()
{
	CString c1,c2;
	GetDlgItem (IDC_CHARSOURCE)->GetWindowText (c1);
	GetDlgItem (IDC_CHARDEST)->GetWindowText (c2);

	POSITION pos = m_ReplaceListBox->GetFirstSelectedItemPosition ();
 
	if ((c1 != "") && (pos != NULL)) {
		int i=m_ReplaceListBox->GetNextSelectedItem (pos);
		m_ReplaceListBox->SetItemText (i,0,c1);
		m_ReplaceListBox->SetItemText (i,1,c2);
	}
}
// Retrieve the config from the dialog in the form that SimpleCleanupFilename understands
void CSimpleCleanupDialog::ReadConfig () {
  GetDlgItem (IDC_EDITSEARCH)->GetWindowText (m_searchfor);
  GetDlgItem (IDC_EDITREPLACEBY)->GetWindowText (m_replaceby);
  m_options = (IsDlgButtonChecked (IDC_CHECKDOTTOSPACE)         * SCO_DOTTOSPACE         ) |
			  (IsDlgButtonChecked (IDC_CHECKUNDERLINETOSPACE)   * SCO_UNDERLINETOSPACE   ) |
			  (IsDlgButtonChecked (IDC_CHECKCONVERTAPOSTROPHES) * SCO_REPLACEAPOSTROPHE  ) |
			  (IsDlgButtonChecked (IDC_CHECKCONVERTHEX)         * SCO_REPLACEHEX         ) |
			  (IsDlgButtonChecked (IDC_CHECKSEARCHANDREPLACE)   * SCO_SEARCHANDREPLACE   ) |
			  (IsDlgButtonChecked (IDC_CHECKCASESENSITIVE)      * SCO_SEARCHCASESENSITIVE) |
			  (IsDlgButtonChecked (IDC_CHECKCHARACTERREPLACE)   * SCO_REPLACECHARS       );
  // Collect the character replacements in lists
  CStringList src;
  CStringList dst;
  for (int i=0; i < m_ReplaceListBox->GetItemCount (); i++) {
	  // don't allow '"' in the strings!
	  CString c1 = m_ReplaceListBox->GetItemText (i,0).SpanExcluding (_T("\""));
	  CString c2 = m_ReplaceListBox->GetItemText (i,1).SpanExcluding (_T("\""));
	  if (c1 != "") {
		src.AddTail (c1);
		dst.AddTail (c2);
	  }
  }
  // Convert the lists to strings and return them
  m_searchforchars = StringListToString (src);
  m_replacebychars = StringListToString (dst);
}

void CSimpleCleanupDialog::OnOK()
{
	// TODO: Fügen Sie hier Ihren spezialisierten Code ein, und/oder rufen Sie die Basisklasse auf.
	ReadConfig ();
	CDialog::OnOK();
}

// Set a config
void CSimpleCleanupDialog::WriteConfig () {
  GetDlgItem (IDC_EDITSEARCH)->SetWindowText (m_searchfor);
  GetDlgItem (IDC_EDITREPLACEBY)->SetWindowText (m_replaceby);
  CheckDlgButton (IDC_CHECKDOTTOSPACE,         (m_options & SCO_DOTTOSPACE         )!=0 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton (IDC_CHECKUNDERLINETOSPACE,   (m_options & SCO_UNDERLINETOSPACE   )!=0 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton (IDC_CHECKCONVERTAPOSTROPHES, (m_options & SCO_REPLACEAPOSTROPHE  )!=0 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton (IDC_CHECKCONVERTHEX,         (m_options & SCO_REPLACEHEX         )!=0 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton (IDC_CHECKSEARCHANDREPLACE,   (m_options & SCO_SEARCHANDREPLACE   )!=0 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton (IDC_CHECKCASESENSITIVE,      (m_options & SCO_SEARCHCASESENSITIVE)!=0 ? BST_CHECKED : BST_UNCHECKED);
  CheckDlgButton (IDC_CHECKCHARACTERREPLACE,   (m_options & SCO_REPLACECHARS       )!=0 ? BST_CHECKED : BST_UNCHECKED);
  // Add the strings to the ListView
  CStringList src;
  CStringList dst;
  StringToStringList (m_searchforchars,src);
  StringToStringList (m_replacebychars,dst);
  POSITION dpos = dst.GetHeadPosition ();
  for (POSITION pos = src.GetHeadPosition (); (pos != NULL) && (dpos != NULL);) {
	CString c1 = src.GetNext (pos);
	CString c2 = dst.GetNext (dpos);
	int i = m_ReplaceListBox->InsertItem (m_ReplaceListBox->GetItemCount(),c1);
	m_ReplaceListBox->SetItemText (i,1,c2);
  }
}

void CSimpleCleanupDialog::GetConfig (int& _options, CString& _searchfor, CString& _replaceby,
									  CString& _searchforchars, CString& _replacebychars) {
  _options = m_options;
  _searchfor = m_searchfor;
  _replaceby = m_replaceby;
  _searchforchars = m_searchforchars;
  _replacebychars = m_replacebychars;
} 

void CSimpleCleanupDialog::SetConfig (int _options, CString _searchfor, CString _replaceby,
									  CString _searchforchars, CString _replacebychars) {
  m_options = _options;
  m_searchfor = _searchfor;
  m_replaceby = _replaceby;
  m_searchforchars = _searchforchars;
  m_replacebychars = _replacebychars;
}

