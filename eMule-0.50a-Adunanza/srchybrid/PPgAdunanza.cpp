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
#include "stdafx.h"
#include "emule.h"
#include "SearchDlg.h"
#include "PreferencesDlg.h"
#include "HttpDownloadDlg.h"
#include "version.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "StatisticsDlg.h"
#include "ServerWnd.h"
#include "TransferWnd.h"
#include "ChatWnd.h"
#include "SharedFilesWnd.h"
#include "KademliaWnd.h"
#include "IrcWnd.h"
#include "WebServices.h"
#include "PPgAdunanzA.h"
#include "AdunanzA.h"
#include "RemoteSettings.h"
#include "DAMessageBox.h"
// Anis Hireche REVISION -> OK! (:
IMPLEMENT_DYNAMIC(CPPgAdunanzA, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgAdunanzA, CPropertyPage)
	ON_EN_CHANGE(IDC_ADU_EDIT, OnEnChangeAduEdit)
	ON_BN_CLICKED(IDC_ADU_ANONYM_STATS, OnBnClickedAduAnonymStats)
	ON_BN_CLICKED(IDC_ADU_NO_OBF, OnBnClickedAduNoObf)
	ON_BN_CLICKED(IDC_NOADUTIPS_CHECK, OnBnClickedAduNoTips)
	ON_BN_CLICKED(IDC_BANDA_EXT_CHECK, OnBnClickedBandaExtCheck)
	ON_EN_CHANGE(IDC_VALOREBANDAUPESX, OnEnChangeValorebandaupesx)								
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER1, StreamingChange)
END_MESSAGE_MAP()

void CPPgAdunanzA::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_SLIDER1, m_ctlStreaming);
}

CPPgAdunanzA::CPPgAdunanzA() : CPropertyPage(CPPgAdunanzA::IDD)
{
}

CPPgAdunanzA::~CPPgAdunanzA(){

}

BOOL CPPgAdunanzA::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();

	return TRUE;
}

void CPPgAdunanzA::LoadSettings(void)
{
  if (!m_hWnd)
    return;

	CString strBuffer;
	// Sistemo i controlli
	// Setto il valore del max upload slots
	strBuffer.Format( _T("%u"), thePrefs.m_AduMaxUpSlots);
	GetDlgItem(IDC_ADU_EDIT)->SetWindowText(strBuffer);
	
	CheckDlgButton(IDC_BANDA_EXT_CHECK, thePrefs.m_AduRipBanda);

	strBuffer.Format( _T("%d"), thePrefs.m_BufferStreaming);
	GetDlgItem(IDC_LABEL_STREAMING)->SetWindowText(strBuffer);


	if(thePrefs.m_AduRipBanda)
	{
		GetDlgItem(IDC_MES_ERR_UP_EXT)->ShowWindow(false);
		GetDlgItem(IDC_VALOREBANDAUPESX)->ShowWindow(false);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT7)->ShowWindow(false);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT2b)->ShowWindow(false);
	}
	else
	{	strBuffer.Format( _T("%u"),thePrefs.m_AduValRipBanda);  
		GetDlgItem(IDC_VALOREBANDAUPESX)->SetWindowText(strBuffer);
		GetDlgItem(IDC_MES_ERR_UP_EXT)->ShowWindow(true);
		GetDlgItem(IDC_VALOREBANDAUPESX)->ShowWindow(true);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT7)->ShowWindow(true);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT2b)->ShowWindow(true);
	}

	CheckDlgButton(IDC_ADU_ANONYM_STATS, thePrefs.m_AduSendStats);
	CheckDlgButton(IDC_ADU_NO_OBF, thePrefs.m_AduNoObf);
	CheckDlgButton(IDC_NOADUTIPS_CHECK, thePrefs.m_AduNoTips);
	m_ctlStreaming.SetRange(1,10);
	m_ctlStreaming.SetPos(thePrefs.m_BufferStreaming);
}

BOOL CPPgAdunanzA::OnApply()
{	
	uint8 numBuffer = m_ctlStreaming.GetPos();
	thePrefs.m_BufferStreaming = numBuffer;
	bool savedata = true;
	CString strBuffer;
	GetDlgItem(IDC_ADU_EDIT)->GetWindowText( strBuffer );
	
	uint32 _maxUpSlots = _tstoi( strBuffer );
	if (_maxUpSlots <4) _maxUpSlots = 4;
	if (_maxUpSlots > 100) _maxUpSlots = 100;

	strBuffer.Format( _T("%u"), _maxUpSlots);

	GetDlgItem(IDC_ADU_EDIT)->SetWindowText( strBuffer );
	thePrefs.m_AduMaxUpSlots = _maxUpSlots;
	

	if (IsDlgButtonChecked(IDC_BANDA_EXT_CHECK))
		thePrefs.m_AduRipBanda = true;
	else 
		thePrefs.m_AduRipBanda = false;
	
	if (thePrefs.m_AduRipBanda)
		CalcolaRatio(false);
	else 
	{
		CString strBuffer;
		GetDlgItem(IDC_VALOREBANDAUPESX)->GetWindowText(strBuffer);
		
		uint32 partial =_tstoi( strBuffer );
		
		if (strBuffer == "")
		{ 
			savedata = false;
			thePrefs.m_AduRipBanda = true;
			CalcolaRatio(false);
		}

		if (partial < ADUNANZA_MIN_BW_TROLLER)
		{
			partial = ADUNANZA_MIN_BW_TROLLER;
		}
		
		if ((theApp.emuledlg->preferenceswnd->m_wndConnection.IsDlgButtonChecked(IDC_ULIMIT_LBL)))
		//ho un limite di banda
		{ 
			if(partial > (thePrefs.maxupload - ADUNANZA_MIN_BW_TROLLER))
			{
				CString msg;
				msg.Format(_T("\n\nATTENZIONE!\n\n")
				_T("Valore di upload esterno troppo alto. Corregilo.\n\n"));
				CDAMessageBox mblu(NULL, msg, false);
				mblu.DoModal();
				thePrefs.m_AduRipBanda = true;
				CalcolaRatio(false);
				savedata = false;
			}
		}
		else
		{ //sono su illimitato
			if(partial > (thePrefs.maxGraphUploadRate - ADUNANZA_MIN_BW_TROLLER))
			{
				CString msg;
				msg.Format(_T("\n\nATTENZIONE!\n\n")
				_T("Valore di upload esterno troppo alto. Corregilo.\n\n"));
				CDAMessageBox mblu(NULL, msg, false);
				mblu.DoModal();
				thePrefs.m_AduRipBanda = true;
				CalcolaRatio(false);
				savedata = false;
			}	
		}

		if (savedata)
		{
			thePrefs.m_AduValRipBanda = partial;
			CalcolaRatio(true);	
		}
	}

	thePrefs.m_AduSendStats =	IsDlgButtonChecked(IDC_ADU_ANONYM_STATS) != 0;
	thePrefs.m_AduNoObf =		IsDlgButtonChecked(IDC_ADU_NO_OBF);
	thePrefs.m_AduNoTips =		IsDlgButtonChecked(IDC_NOADUTIPS_CHECK) != 0;
	SetModified(FALSE);
	return TRUE;
}

void CPPgAdunanzA::OnEnChangeAduEdit()
{
	SetModified(TRUE);
}

void CPPgAdunanzA::OnBnClickedAduNoObf()
{
	SetModified(TRUE);
}

void CPPgAdunanzA::OnBnClickedAduAnonymStats()
{
	SetModified(TRUE);
}
void CPPgAdunanzA::OnBnClickedAduNoTips()
{
	SetModified(TRUE);
}

void CPPgAdunanzA::OnBnClickedBandaExtCheck()
{
	if (IsDlgButtonChecked(IDC_BANDA_EXT_CHECK))
	{
		GetDlgItem(IDC_MES_ERR_UP_EXT)->ShowWindow(false);
		GetDlgItem(IDC_VALOREBANDAUPESX)->ShowWindow(false);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT7)->ShowWindow(false);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT2b)->ShowWindow(false);
	}
	else
	{	CString strBuffer;
		strBuffer.Format( _T(""));
		GetDlgItem(IDC_VALOREBANDAUPESX)->SetWindowText(strBuffer);
		GetDlgItem(IDC_MES_ERR_UP_EXT)->ShowWindow(true);
		GetDlgItem(IDC_VALOREBANDAUPESX)->ShowWindow(true);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT7)->ShowWindow(true);
		GetDlgItem(IDC_STATIC_MEDIAUPEXT2b)->ShowWindow(true);
	}
	SetModified(TRUE);
}


void CPPgAdunanzA::OnEnChangeValorebandaupesx()
{
	SetModified(true);
}

//Anis -> occupiamoci del dialog di uscita
IMPLEMENT_DYNAMIC(CAskExit, CDialog)

CAskExit::CAskExit(CWnd* pParent /*=NULL*/) : CDialog(CAskExit::IDD, pParent)
{

}

CAskExit::~CAskExit()
{
}

void CAskExit::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAskExit, CDialog)
	ON_BN_CLICKED(IDYES, OnBnClickedYes)
	ON_BN_CLICKED(IDNO, OnBnClickedNo)
	ON_BN_CLICKED(IDNOMINIMIZE, OnBnClickedNominimize)
END_MESSAGE_MAP()


// gestori di messaggi CAskExit -> ANIS

void CAskExit::OnBnClickedYes()
{
	thePrefs.confirmExit= (IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);
    EndDialog(IDYES);
}

void CAskExit::OnBnClickedNo()
{
	thePrefs.confirmExit= IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0;
	EndDialog(IDNO);
}

void CAskExit::OnBnClickedNominimize()
{
	if (thePrefs.GetMinToTray())
		theApp.emuledlg->PostMessage(WM_SYSCOMMAND , SC_MINIMIZE, 0);
	else
	    theApp.emuledlg->PostMessage(WM_SYSCOMMAND, 0x170, 0);
	 EndDialog(IDNO);
}
void CPPgAdunanzA::StreamingChange(NMHDR *pNMHDR, LRESULT *pResult) //Anis Streaming
{
	CString buffer;
	buffer.Format(_T("%u"),m_ctlStreaming.GetPos());
	GetDlgItem(IDC_LABEL_STREAMING)->SetWindowText(buffer);
	SetModified(true);
	*pResult = 0;
}
