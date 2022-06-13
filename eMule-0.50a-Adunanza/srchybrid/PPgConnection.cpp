//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include <math.h>
#include "emule.h"
#include "PPgConnection.h"
#include "Scheduler.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "Opcodes.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "HelpIDs.h"
#include "Statistics.h"
#include "Firewallopener.h"
#include "ListenSocket.h"
#include "ClientUDPSocket.h"
#include "LastCommonRouteFinder.h"
#include "AdunanzA.h"
#include ".\ppgconnection.h"
#include "PreferencesDlg.h"
#include "RemoteSettings.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgConnection, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgConnection, CPropertyPage)
	ON_BN_CLICKED(IDC_STARTTEST, OnStartPortTest)
	ON_EN_CHANGE(IDC_DOWNLOAD_CAP, OnSettingsChange)
	ON_EN_CHANGE(IDC_UDPPORT, OnEnChangeUDP)
	ON_EN_CHANGE(IDC_UPLOAD_CAP, OnSettingsChange)
	ON_EN_CHANGE(IDC_PORT, OnEnChangeTCP)
	ON_EN_CHANGE(IDC_MAXCON, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXSOURCEPERFILE, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOCONNECT, OnSettingsChange)
	ON_BN_CLICKED(IDC_RECONN, OnSettingsChange)
	ON_BN_CLICKED(IDC_NETWORK_ED2K, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWOVERHEAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_ULIMIT_LBL, OnLimiterChange)
	ON_BN_CLICKED(IDC_DLIMIT_LBL, OnLimiterChange)
	ON_WM_HSCROLL()
	//ON_BN_CLICKED(IDC_NETWORK_KADEMLIA, OnSettingsChange) //Anis -> KAdu SEMPRE Attiva!
	ON_WM_HELPINFO()
	ON_BN_CLICKED(IDC_PREF_UPNPONSTART, OnSettingsChange)
	ON_BN_CLICKED(IDC_RANDOMPORTS, OnRandomPortsChange)
	ON_BN_CLICKED(IDC_BUTTON1, &CPPgConnection::OnBnClickedButton1)
	ON_EN_CHANGE(IDC_MINPORT, &CPPgConnection::PortaRndMin)
	ON_EN_CHANGE(IDC_MAXPORT, &CPPgConnection::PortaRndMax)
END_MESSAGE_MAP()

CPPgConnection::CPPgConnection()
	: CPropertyPage(CPPgConnection::IDD)
{
	guardian = false;
}

CPPgConnection::~CPPgConnection()
{
}

void CPPgConnection::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MAXDOWN_SLIDER, m_ctlMaxDown);
	DDX_Control(pDX, IDC_MAXUP_SLIDER, m_ctlMaxUp);
	CString text;
	DDX_Control(pDX, IDC_MINPORT, m_minRndPort);
	m_minRndPort.GetWindowText(text);
	DDV_MinMaxInt(pDX,_ttoi(text),1,0xFFFF);
	DDX_Control(pDX, IDC_MAXPORT, m_maxRndPort);
	m_maxRndPort.GetWindowText(text);
	DDX_Control(pDX, IDC_MINPORT, m_minRndPort);
}

void CPPgConnection::OnEnChangeTCP()
{
	OnEnChangePorts(true);
}

void CPPgConnection::OnEnChangeUDP()
{
	OnEnChangePorts(false);
}

void CPPgConnection::OnEnChangePorts(uint8 istcpport)
{
	// ports unchanged?
	CString buffer;
	GetDlgItem(IDC_PORT)->GetWindowText(buffer);
	uint16 tcp = (uint16)_tstoi(buffer);
	GetDlgItem(IDC_UDPPORT)->GetWindowText(buffer);
	uint16 udp = (uint16)_tstoi(buffer);

	GetDlgItem(IDC_STARTTEST)->EnableWindow( 
		tcp == theApp.listensocket->GetConnectedPort() && 
		udp == theApp.clientudp->GetConnectedPort() 
	);

	OnSettingsChange();
}

BOOL CPPgConnection::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	m_minRndPortSpin.SetBuddy(&m_minRndPort);
	m_minRndPortSpin.SetRange32(1,0xffff);
	m_maxRndPortSpin.SetBuddy(&m_maxRndPort);
	m_maxRndPortSpin.SetRange32(1,0xffff);
	LoadSettings();
	Localize();

	OnEnChangePorts(2);

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgConnection::LoadSettings(void)
{
	if (m_hWnd)
	{
		if (thePrefs.maxupload != 0)
			thePrefs.maxdownload = thePrefs.GetMaxDownload();

		CString strBuffer;
		
		strBuffer.Format(_T("%d"), thePrefs.udpport);
		GetDlgItem(IDC_UDPPORT)->SetWindowText(strBuffer);
	
		strBuffer.Format(_T("%d"), thePrefs.maxGraphDownloadRate);
		GetDlgItem(IDC_DOWNLOAD_CAP)->SetWindowText(strBuffer);

		m_ctlMaxDown.SetRange(10, thePrefs.maxGraphDownloadRate);
		SetRateSliderTicks(m_ctlMaxDown);

		if (thePrefs.maxGraphUploadRate != UNLIMITED)
			strBuffer.Format(_T("%d"), thePrefs.maxGraphUploadRate);
		else
			strBuffer = _T("0");
		GetDlgItem(IDC_UPLOAD_CAP)->SetWindowText(strBuffer);

		m_ctlMaxUp.SetRange(10, thePrefs.GetMaxGraphUploadRate(true));
		SetRateSliderTicks(m_ctlMaxUp);

		CheckDlgButton( IDC_DLIMIT_LBL, (thePrefs.maxdownload != UNLIMITED));
		CheckDlgButton( IDC_ULIMIT_LBL, (thePrefs.maxupload != UNLIMITED));

		m_ctlMaxDown.SetPos((thePrefs.maxdownload != UNLIMITED) ? thePrefs.maxdownload : thePrefs.maxGraphDownloadRate);
		m_ctlMaxUp.SetPos((thePrefs.maxupload != UNLIMITED) ? thePrefs.maxupload : thePrefs.GetMaxGraphUploadRate(true));

		strBuffer.Format(_T("%d"), thePrefs.port);
		GetDlgItem(IDC_PORT)->SetWindowText(strBuffer);

		strBuffer.Format(_T("%d"), thePrefs.maxconnections);
		GetDlgItem(IDC_MAXCON)->SetWindowText(strBuffer);

		if (thePrefs.maxsourceperfile == 0xFFFF)
			GetDlgItem(IDC_MAXSOURCEPERFILE)->SetWindowText(_T("0"));
		else{
			strBuffer.Format(_T("%d"), thePrefs.maxsourceperfile);
			GetDlgItem(IDC_MAXSOURCEPERFILE)->SetWindowText(strBuffer);
		}

		if (thePrefs.reconnect)
			CheckDlgButton(IDC_RECONN, 1);
		else
			CheckDlgButton(IDC_RECONN, 0);
		
		if (thePrefs.m_bshowoverhead)
			CheckDlgButton(IDC_SHOWOVERHEAD, 1);
		else
			CheckDlgButton(IDC_SHOWOVERHEAD, 0);

		if (thePrefs.autoconnect)
			CheckDlgButton(IDC_AUTOCONNECT, 1);
		else
			CheckDlgButton(IDC_AUTOCONNECT, 0);

		CheckDlgButton(IDC_NETWORK_KADEMLIA, 1);

		//GetDlgItem(IDC_NETWORK_KADEMLIA)->EnableWindow(thePrefs.GetUDPPort() > 0);

		if (thePrefs.networked2k)
			CheckDlgButton(IDC_NETWORK_ED2K, 1);
		else
			CheckDlgButton(IDC_NETWORK_ED2K, 0);

		CString rndText;

		CheckDlgButton(IDC_RANDOMPORTS,thePrefs.GetUseRandomPorts());
		rndText.Format(_T("%u"), thePrefs.GetMinRandomPort());
		m_minRndPort.SetWindowText(rndText);
		rndText.Format(_T("%u"), thePrefs.GetMaxRandomPort());
		m_maxRndPort.SetWindowText(rndText);

		GetDlgItem(IDC_PORT)->EnableWindow(!thePrefs.GetUseRandomPorts());
		GetDlgItem(IDC_UDPPORT)->EnableWindow(!thePrefs.GetUseRandomPorts() && thePrefs.udpport>0);
		
		m_minRndPort.EnableWindow(thePrefs.GetUseRandomPorts());
		m_maxRndPort.EnableWindow(thePrefs.GetUseRandomPorts());
		m_minRndPortSpin.EnableWindow(thePrefs.GetUseRandomPorts());
		m_maxRndPortSpin.EnableWindow(thePrefs.GetUseRandomPorts());

		if (thePrefs.IsUPnPNat())
			CheckDlgButton(IDC_PREF_UPNPONSTART, 1);
		else
			CheckDlgButton(IDC_PREF_UPNPONSTART, 0);

		ShowLimitValues();
		OnLimiterChange();
	}
}

BOOL CPPgConnection::OnApply()
{
	TCHAR buffer[510];
	int lastmaxgu = thePrefs.maxGraphUploadRate;
	int lastmaxgd = thePrefs.maxGraphDownloadRate;
	bool bRestartApp = false;

	if (GetDlgItem(IDC_DOWNLOAD_CAP)->GetWindowTextLength())
	{ 
		GetDlgItem(IDC_DOWNLOAD_CAP)->GetWindowText(buffer, 20);
		thePrefs.SetMaxGraphDownloadRate(_tstoi(buffer));
	}

	m_ctlMaxDown.SetRange(10, thePrefs.GetMaxGraphDownloadRate(), TRUE);
	SetRateSliderTicks(m_ctlMaxDown);

	if (GetDlgItem(IDC_UPLOAD_CAP)->GetWindowTextLength())
	{
		GetDlgItem(IDC_UPLOAD_CAP)->GetWindowText(buffer, 20);
		thePrefs.SetMaxGraphUploadRate(_tstoi(buffer));
	}

	m_ctlMaxUp.SetRange(10, thePrefs.GetMaxGraphUploadRate(true), TRUE);
	SetRateSliderTicks(m_ctlMaxUp);

    uint16 ulSpeed;

	if (!IsDlgButtonChecked(IDC_ULIMIT_LBL))
		ulSpeed = UNLIMITED;
	else
		ulSpeed = (uint16)m_ctlMaxUp.GetPos();

	if (thePrefs.GetMaxGraphUploadRate(true) < ulSpeed && ulSpeed != UNLIMITED)
		ulSpeed = (uint16)(thePrefs.GetMaxGraphUploadRate(true) * 0.8);

    if(ulSpeed > thePrefs.GetMaxUpload()) 
	{
        // make USS go up to higher ul limit faster
        theApp.lastCommonRouteFinder->InitiateFastReactionPeriod();
    }

    thePrefs.SetMaxUpload(ulSpeed);

	if (thePrefs.GetMaxUpload() != UNLIMITED)
		m_ctlMaxUp.SetPos(thePrefs.GetMaxUpload());
	
	if (!IsDlgButtonChecked(IDC_DLIMIT_LBL))
		thePrefs.SetMaxDownload(UNLIMITED);
	else
		thePrefs.SetMaxDownload(m_ctlMaxDown.GetPos());

	if (thePrefs.GetMaxGraphDownloadRate() < thePrefs.GetMaxDownload() && thePrefs.GetMaxDownload() != UNLIMITED)
		thePrefs.SetMaxDownload((uint16)(thePrefs.GetMaxGraphDownloadRate() * 0.8));

	if (thePrefs.GetMaxDownload() != UNLIMITED)
		m_ctlMaxDown.SetPos(thePrefs.GetMaxDownload());

	if (GetDlgItem(IDC_PORT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_PORT)->GetWindowText(buffer, 20);
		uint16 nNewPort = ((uint16)_tstoi(buffer)) ? (uint16)_tstoi(buffer) : (uint16)thePrefs.port;
		if (nNewPort != thePrefs.port){
			thePrefs.port = nNewPort;
			if (theApp.IsPortchangeAllowed())
				theApp.listensocket->Rebind();
			else
				bRestartApp = true;
		}
	}
	
	if (GetDlgItem(IDC_MAXSOURCEPERFILE)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MAXSOURCEPERFILE)->GetWindowText(buffer, 20);
		thePrefs.maxsourceperfile = (_tstoi(buffer)) ? _tstoi(buffer) : 1;
	}

	if (GetDlgItem(IDC_UDPPORT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_UDPPORT)->GetWindowText(buffer, 20);
		uint16 nNewPort = ((uint16)_tstoi(buffer) ? (uint16)_tstoi(buffer) : (uint16)thePrefs.udpport);
		if (nNewPort != thePrefs.udpport){
			thePrefs.udpport = nNewPort;
			if (theApp.IsPortchangeAllowed())
				theApp.clientudp->Rebind();
			else 
				bRestartApp = true;
		}
	}

	if (IsDlgButtonChecked(IDC_SHOWOVERHEAD)){
		if (!thePrefs.m_bshowoverhead){
			// reset overhead data counters before starting to meassure!
			theStats.ResetDownDatarateOverhead();
			theStats.ResetUpDatarateOverhead();
		}
		thePrefs.m_bshowoverhead = true;
	}
	else{
		if (thePrefs.m_bshowoverhead){
			// free memory used by overhead computations
			theStats.ResetDownDatarateOverhead();
			theStats.ResetUpDatarateOverhead();
		}
		thePrefs.m_bshowoverhead = false;
	}


	thePrefs.SetNetworkKademlia(true);


	if (IsDlgButtonChecked(IDC_NETWORK_ED2K))
		thePrefs.SetNetworkED2K(true);
	else
		thePrefs.SetNetworkED2K(false);

	thePrefs.autoconnect = IsDlgButtonChecked(IDC_AUTOCONNECT)!=0;
	thePrefs.reconnect = IsDlgButtonChecked(IDC_RECONN)!=0;
		
	if (lastmaxgu != thePrefs.maxGraphUploadRate) 
		theApp.emuledlg->statisticswnd->SetARange(false, thePrefs.GetMaxGraphUploadRate(true));
	if (lastmaxgd!=thePrefs.maxGraphDownloadRate)
		theApp.emuledlg->statisticswnd->SetARange(true, thePrefs.maxGraphDownloadRate);

	UINT tempcon = thePrefs.maxconnections;
	if (GetDlgItem(IDC_MAXCON)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MAXCON)->GetWindowText(buffer, 20);
		tempcon = (_tstoi(buffer)) ? _tstoi(buffer) : CPreferences::GetRecommendedMaxConnections();
	}

	if (tempcon > (unsigned)::GetMaxWindowsTCPConnections())
	{
		CString strMessage;
		strMessage.Format(GetResString(IDS_PW_WARNING), GetResString(IDS_PW_MAXC), ::GetMaxWindowsTCPConnections());
		int iResult = AfxMessageBox(strMessage, MB_ICONWARNING | MB_YESNO);
		if (iResult != IDYES)
		{
			//TODO: set focus to max connection?
			strMessage.Format(_T("%d"), thePrefs.maxconnections);
			GetDlgItem(IDC_MAXCON)->SetWindowText(strMessage);
			tempcon = ::GetMaxWindowsTCPConnections();
		}
	}
	thePrefs.maxconnections = tempcon;

	if((BOOL)thePrefs.IsUPnPNat() != (IsDlgButtonChecked(IDC_PREF_UPNPONSTART) != 0) )
	{
  		theApp.m_UPnP_IGDControlPoint->SetUPnPNat((IsDlgButtonChecked(IDC_PREF_UPNPONSTART) != 0)); // and start/stop nat. 
	}


	bool oldUseRandom = thePrefs.GetUseRandomPorts();
	thePrefs.SetUseRandomPorts(IsDlgButtonChecked(IDC_RANDOMPORTS)!=0);

	unsigned long rndValue1, rndValue2, oldRndMin, oldRndMax;
	CString rndText;
	m_minRndPort.GetWindowText(rndText);
	rndValue1 = _tstoi(rndText);
	m_maxRndPort.GetWindowText(rndText);
	rndValue2 = _tstoi(rndText);

	if (rndValue1 > 0xFFFF) rndValue1 = 0xFFFF;
	if (rndValue1 < 1) rndValue1 = 1;
	if (rndValue2 > 0xFFFF) rndValue2 = 0xFFFF;
	if (rndValue2 < 1) rndValue2 = 1;
	if (rndValue1 > rndValue2){
		int tmp = rndValue2;
		rndValue2 = rndValue1;
		rndValue1 = tmp;
	}

	oldRndMin = thePrefs.GetMinRandomPort();
	oldRndMax = thePrefs.GetMaxRandomPort();

	if(rndValue1 != oldRndMin || rndValue2 != oldRndMax){
		thePrefs.SetMinRandomPort((uint16)rndValue1);
		thePrefs.SetMaxRandomPort((uint16)rndValue2);
	}

	if((IsDlgButtonChecked(IDC_RANDOMPORTS)!=0) != oldUseRandom){
		if (theApp.IsPortchangeAllowed()){
			theApp.listensocket->Rebind();
			theApp.clientudp->Rebind();
		}
		else{
			bRestartApp = true;
			thePrefs.SetRandomPortsResetOnRestart(true);
		}
	}
	else if(oldUseRandom){
		if(rndValue1 != oldRndMin || rndValue2 != oldRndMax){
			if (theApp.IsPortchangeAllowed()){
				theApp.listensocket->Rebind();
				theApp.clientudp->Rebind();
			}
			else {
				bRestartApp = true;
				thePrefs.SetRandomPortsResetOnRestart(true);
			}
		}
	}

	if (thePrefs.maxGraphUploadRate > 50000) { //Anis fix max value. se l'intero è maggiore di 65000 circa va in overflow.
		thePrefs.maxupload = 50000;
		thePrefs.SetMaxGraphUploadRate(50000);
		m_ctlMaxUp.SetRange(10, 50000,1);
	}

	if (thePrefs.maxGraphDownloadRate > 50000) {
		thePrefs.maxdownload = 50000;
		thePrefs.SetMaxGraphDownloadRate(50000);
		m_ctlMaxDown.SetRange(10, 50000,1);
	}

	if (thePrefs.maxconnections > 50000) {
		thePrefs.maxconnections = 50000;
	}

	if (thePrefs.maxsourceperfile > 50000) {
		thePrefs.maxsourceperfile = 50000;
	}


	theApp.scheduler->SaveOriginals();

	SetModified(FALSE);
	LoadSettings();

	theApp.emuledlg->ShowConnectionState();

	if (thePrefs.m_AduValRipBanda > thePrefs.GetMaxUpload()) CalcolaRatio(true);
	else CalcolaRatio(false);

	if (bRestartApp)
		AfxMessageBox(GetResString(IDS_NOPORTCHANGEPOSSIBLE));

	OnEnChangePorts(2);

	return CPropertyPage::OnApply();
}

void CPPgConnection::Localize(void)
{	
	if (m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_CONNECTION));
		GetDlgItem(IDC_CAPACITIES_FRM)->SetWindowText(GetResString(IDS_PW_CON_CAPFRM));
		GetDlgItem(IDC_DCAP_LBL)->SetWindowText(GetResString(IDS_PW_CON_DOWNLBL));
		GetDlgItem(IDC_UCAP_LBL)->SetWindowText(GetResString(IDS_PW_CON_UPLBL));
		GetDlgItem(IDC_LIMITS_FRM)->SetWindowText(GetResString(IDS_PW_CON_LIMITFRM));
		GetDlgItem(IDC_DLIMIT_LBL)->SetWindowText(GetResString(IDS_PW_DOWNL));
		GetDlgItem(IDC_ULIMIT_LBL)->SetWindowText(GetResString(IDS_PW_UPL));
		GetDlgItem(IDC_CONNECTION_NETWORK)->SetWindowText(GetResString(IDS_NETWORK));
		GetDlgItem(IDC_KBS2)->SetWindowText(GetResString(IDS_KBYTESPERSEC));
		GetDlgItem(IDC_KBS3)->SetWindowText(GetResString(IDS_KBYTESPERSEC));
		ShowLimitValues();
		GetDlgItem(IDC_MAXCONN_FRM)->SetWindowText(GetResString(IDS_PW_CONLIMITS));
		GetDlgItem(IDC_MAXCONLABEL)->SetWindowText(GetResString(IDS_PW_MAXC));
		GetDlgItem(IDC_SHOWOVERHEAD)->SetWindowText(GetResString(IDS_SHOWOVERHEAD));
		GetDlgItem(IDC_CLIENTPORT_FRM)->SetWindowText(GetResString(IDS_PW_CLIENTPORT));
		GetDlgItem(IDC_MAXSRC_FRM)->SetWindowText(GetResString(IDS_PW_MAXSOURCES));
		GetDlgItem(IDC_AUTOCONNECT)->SetWindowText(GetResString(IDS_PW_AUTOCON));
		GetDlgItem(IDC_RECONN)->SetWindowText(GetResString(IDS_PW_RECON));
		GetDlgItem(IDC_MAXSRCHARD_LBL)->SetWindowText(GetResString(IDS_HARDLIMIT));
		SetDlgItemText(IDC_STARTTEST, GetResString(IDS_STARTTEST) );
		GetDlgItem(IDC_PREF_UPNPONSTART)->SetWindowText(GetResString(IDS_UPNPSTART));
		GetDlgItem(IDC_RANDOMPORTS)->SetWindowText(GetResString(IDS_RANDOMPORTS));
		GetDlgItem(IDC_LBL_MIN)->SetWindowText(GetResString(IDS_MINPORT));
		GetDlgItem(IDC_LBL_MAX)->SetWindowText(GetResString(IDS_MAXPORT));
	}
}

void CPPgConnection::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	if (pScrollBar->GetSafeHwnd() == m_ctlMaxUp.m_hWnd)
	{
		uint32 maxup = m_ctlMaxUp.GetPos();
		uint32 maxdown = m_ctlMaxDown.GetPos();
		if (maxup < 4 && maxup*3 < maxdown)
		{
			m_ctlMaxDown.SetPos(maxup*3);
		}
		if (maxup < 10 && maxup*4 < maxdown)
		{
			m_ctlMaxDown.SetPos(maxup*4);
		}
	}
	else if (pScrollBar->GetSafeHwnd() == m_ctlMaxDown.m_hWnd)
	{
		uint32 maxup = m_ctlMaxUp.GetPos();
		uint32 maxdown = m_ctlMaxDown.GetPos();
		if (maxdown < 13 && maxup*3 < maxdown)
		{
			m_ctlMaxUp.SetPos((int)ceil((double)maxdown/3));
		}
		if (maxdown < 41 && maxup*4 < maxdown)
		{
			m_ctlMaxUp.SetPos((int)ceil((double)maxdown/4));
		}
	}

	ShowLimitValues();

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CPPgConnection::ShowLimitValues()
{
	CString buffer;

	if (!IsDlgButtonChecked(IDC_ULIMIT_LBL))
		buffer = _T("");
	else
		buffer.Format(_T("%u %s"), m_ctlMaxUp.GetPos(), GetResString(IDS_KBYTESPERSEC));
	GetDlgItem(IDC_KBS4)->SetWindowText(buffer);
	
	if (!IsDlgButtonChecked(IDC_DLIMIT_LBL))
		buffer = _T("");
	else
		buffer.Format(_T("%u %s"), m_ctlMaxDown.GetPos(), GetResString(IDS_KBYTESPERSEC));
	GetDlgItem(IDC_KBS1)->SetWindowText(buffer);
}

void CPPgConnection::OnLimiterChange()
{
	m_ctlMaxDown.ShowWindow(IsDlgButtonChecked(IDC_DLIMIT_LBL) ? SW_SHOW : SW_HIDE);
	m_ctlMaxUp.ShowWindow(IsDlgButtonChecked(IDC_ULIMIT_LBL) ? SW_SHOW : SW_HIDE);

	ShowLimitValues();
	SetModified(TRUE);	
}

void CPPgConnection::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_Connection);
}

BOOL CPPgConnection::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgConnection::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}


void CPPgConnection::OnStartPortTest()
{
	CString buffer;

	GetDlgItem(IDC_PORT)->GetWindowText(buffer);
	uint16 tcp = (uint16)_tstoi(buffer);

	GetDlgItem(IDC_UDPPORT)->GetWindowText(buffer);
	uint16 udp = (uint16)_tstoi(buffer);

	TriggerPortTest(tcp, udp);
}

void CPPgConnection::SetRateSliderTicks(CSliderCtrl& rRate)
{
	rRate.ClearTics();
	int iMin = 0, iMax = 0;
	rRate.GetRange(iMin, iMax);
	int iDiff = iMax - iMin;
	if (iDiff > 0)
	{
		CRect rc;
		rRate.GetWindowRect(&rc);
		if (rc.Width() > 0)
		{
			int iTic;
			int iPixels = rc.Width() / iDiff;
			if (iPixels >= 6)
				iTic = 1;
			else
			{
				iTic = 10;
				while (rc.Width() / (iDiff / iTic) < 8)
					iTic *= 10;
			}
			if (iTic)
			{
				for (int i = ((iMin+(iTic-1))/iTic)*iTic; i < iMax; /**/)
				{
					rRate.SetTic(i);
					i += iTic;
				}
			}
			rRate.SetPageSize(iTic);
		}
	}
}

void CPPgConnection::OnRandomPortsChange() {
	CString udpStr;
	unsigned int udp;

	GetDlgItem(IDC_UDPPORT)->GetWindowText(udpStr);
	udp = _tstoi(udpStr);

	GetDlgItem(IDC_UDPPORT)->EnableWindow(!IsDlgButtonChecked(IDC_RANDOMPORTS) && udp>0);
	GetDlgItem(IDC_PORT)->EnableWindow(!IsDlgButtonChecked(IDC_RANDOMPORTS));
	m_minRndPort.EnableWindow(IsDlgButtonChecked(IDC_RANDOMPORTS));
	m_maxRndPort.EnableWindow(IsDlgButtonChecked(IDC_RANDOMPORTS));
	m_minRndPortSpin.EnableWindow(IsDlgButtonChecked(IDC_RANDOMPORTS));
	m_maxRndPortSpin.EnableWindow(IsDlgButtonChecked(IDC_RANDOMPORTS));

	SetModified(TRUE);	
}

void CPPgConnection::OnBnClickedButton1()
{		
	extern bool AduStream;
	extern bool nonchiudereopzioni;
	if(!AduStream) {
		AfxMessageBox(_T("Disabilitare lo streaming prima di effettuare questa operazione."));
		return;
	}
		StartAdunanzaTest_Settings();
		if(!nonchiudereopzioni)
			EndDialog(IDD_PPG_CONNECTION);
		SetModified(0);
}

void CPPgConnection::PortaRndMin()
{
	SetModified(true);
}

void CPPgConnection::PortaRndMax()
{
	SetModified(true);
}
