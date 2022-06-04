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

#include "types.h"
#include "Preferences.h"

class CSivkaFileSettings : public CDialog
{
	DECLARE_DYNAMIC(CSivkaFileSettings)

public:
	CSivkaFileSettings(); // standard constructor
	virtual ~CSivkaFileSettings();
	void SetPrefs(CPreferences* in_prefs) {	app_prefs = in_prefs;}
	enum { IDD = IDD_SIVKAFILESETTINGS }; // Dialog Data

protected:
	CPreferences *app_prefs;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog(void);

	afx_msg	void OnBnClickedMaxSourcesPerFileTakeOver();
	afx_msg	void OnBnClickedEnableAutoNNS();
	afx_msg	void OnBnClickedEnableAutoFQS();
	afx_msg	void OnBnClickedEnableAutoQRS();
	afx_msg	void OnBnClickedTakeOver();
	afx_msg	void OnBnClickedSwitch();
	afx_msg	void OnBnClickedEnableAutoDropNNSTakeOver();
	afx_msg	void OnBnClickedAutoNNS_TimerTakeOver();
	afx_msg	void OnBnClickedMaxRemoveNNSLimitTakeOver();
	afx_msg	void OnBnClickedEnableAutoDropFQSTakeOver();
	afx_msg	void OnBnClickedAutoFQS_TimerTakeOver();
	afx_msg	void OnBnClickedMaxRemoveFQSLimitTakeOver();
	afx_msg	void OnBnClickedEnableAutoDropQRSTakeOver();
	afx_msg	void OnBnClickedAutoHQRS_TimerTakeOver();
	afx_msg	void OnBnClickedMaxRemoveQRSTakeOver();
	afx_msg	void OnBnClickedMaxRemoveQRSLimitTakeOver();
	afx_msg void OnBnClickedHQRXmanTakeOver();
	afx_msg void OnBnClickedHQRXman();
	afx_msg	void OnBnClickedGlobalHlTakeOver(); // Global Source Limit (customize for files) - Stulle

private:
	void LoadSettings(void);
	void Localize();
	void SetWithDefaultValues(void);

	bool m_RestoreDefault;
};