#pragma once
//this file is part of eMule morphXT
//Copyright (C)2006 leuk_he ( strEmail.Format("%s@%s", "leukhe", "gmail.com") / http://emulemorph.sf.net )
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

class CPPgNTService : public CPPgtooltipped  
{
	DECLARE_DYNAMIC(CPPgNTService )

public:
	CPPgNTService ();
	virtual ~CPPgNTService ();

	enum { IDD = IDD_PPG_NTSERVER };

	void Localize(void);

	BOOL m_bIsInit;
private:
	enum eTab	{NONE, WEBSERVER,MULTIWEBSERVER,NTSERVICE};
	CTabCtrl	m_tabCtr;
	eTab		m_currentTab;
	CImageList	m_imageList;
	void		SetTab(eTab tab);
public:
	afx_msg void OnTcnSelchangeTab(NMHDR * /* pNMHDR*/, LRESULT *pResult);
	void	InitTab(bool firstinit, int Page = 0);
protected:
	BOOL m_bModified;

	void LoadSettings(void);

	int  FillStatus();

	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	void SetModified(BOOL bChanged = TRUE){
		m_bModified = bChanged;
		CPropertyPage::SetModified(bChanged);
	}

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	afx_msg void OnBnClickedInstall();	
	afx_msg void OnBnClickedUnInstall();	
	afx_msg void OnBnStartSystem();	
	afx_msg void OnBnManualStart();	
	afx_msg void OnBnAllSettings();	
	afx_msg void OnBnRunBRowser();	
	afx_msg void OnBnReplaceStart();
	afx_msg void OnCbnSelChangeOptLvl()		{SetModified();}
	CComboBox	m_cbOptLvl;

	void InitOptLvlCbn(bool bFirstInit = false);
};
