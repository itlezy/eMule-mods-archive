//this file is part of eMule
// added by quekky
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

#include "ColorButton.h"

// CCatDialog dialog

class CCatDialog : public CDialog
{
	DECLARE_DYNAMIC(CCatDialog)

public:
	CCatDialog(int catindex);   // standard constructor
	virtual ~CCatDialog();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_CAT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
//	afx_msg void OnCbnSelchangeColorselector();
	afx_msg LONG OnSelChange(UINT lParam, LONG wParam);
	DECLARE_MESSAGE_MAP()
	CColorButton m_ctlColor;
private:
	CCat	*m_pCat;
	DWORD	m_dwNewColor;

	void UpdateData();
	void Localize();

	CComboBox m_cmbPriority;
public:
	afx_msg void OnBnClickedBrowse();
	afx_msg void OnBnClickedBrowseTemp();
	afx_msg void OnBnClickedOk();
};
