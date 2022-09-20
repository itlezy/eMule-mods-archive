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
#include "stdafx.h"
#include <math.h>
#include "emule.h"
#include "PPgConnection.h"
#include "wizard.h"
#include "Scheduler.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "Opcodes.h"
#include "StatisticsDlg.h"
#include "Kademlia/Kademlia/Kademlia.h"
#include "Statistics.h"
#include "Firewallopener.h"
#include "ListenSocket.h"
#include "ClientUDPSocket.h"
#include "Log.h"
//#include "UPnPFinder.h" //Xman official UPNP removed

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgConnection, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgConnection, CPropertyPage)
	ON_BN_CLICKED(IDC_STARTTEST, OnStartPortTest)
	ON_EN_CHANGE(IDC_DOWNLOAD_CAP, OnSettingsChange)
	ON_BN_CLICKED(IDC_UDPDISABLE, OnEnChangeUDPDisable)
	ON_EN_CHANGE(IDC_UDPPORT, OnEnChangeUDP)
	ON_EN_CHANGE(IDC_UPLOAD_CAP, OnSettingsChange)
	ON_EN_CHANGE(IDC_PORT, OnEnChangeTCP)
	ON_EN_CHANGE(IDC_MAXCON, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXSOURCEPERFILE, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOCONNECT, OnSettingsChange)
	ON_BN_CLICKED(IDC_RECONN, OnSettingsChange)
	ON_BN_CLICKED(IDC_WIZARD, OnBnClickedWizard)
	ON_BN_CLICKED(IDC_NETWORK_ED2K, OnSettingsChange)
	ON_BN_CLICKED(IDC_SHOWOVERHEAD, OnSettingsChange)
	//Xman Xtreme Upload
	//ON_BN_CLICKED(IDC_ULIMIT_LBL, OnLimiterChange)
	//ON_BN_CLICKED(IDC_DLIMIT_LBL, OnLimiterChange)
	ON_EN_CHANGE(IDC_MAXDOWN, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXUP, OnSettingsChange)
	ON_EN_KILLFOCUS(IDC_MAXUP, OnEnKillfocusMaxup)
	//Xman end
	//Xman GlobalMaxHarlimit for fairness
	ON_BN_CLICKED(IDC_ACCEPTRATIO, OnSettingsChange)
	ON_BN_CLICKED(IDC_ACCEPTSOURCES, OnSettingsChange)
	//Xman end
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_NETWORK_KADEMLIA, OnSettingsChange)
	ON_BN_CLICKED(IDC_OPENPORTS, OnBnClickedOpenports)
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
	//Xman
	//DDX_Control(pDX, IDC_MAXDOWN_SLIDER, m_ctlMaxDown);
	DDX_Control(pDX, IDC_MAXUP_SLIDER, m_ctlMaxUp);

	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	DDX_Control(pDX, IDC_MAXUP, m_maxUpload);
	DDX_Control(pDX, IDC_UPLOAD_CAP, m_maxUploadCapacity);
	DDX_Control(pDX, IDC_MAXDOWN, m_maxDownload);
	DDX_Control(pDX, IDC_DOWNLOAD_CAP, m_maxDownloadCapacity);
	// Maella end

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

	if (istcpport == 0)
		OnEnChangeUDPDisable();
	else if (istcpport == 1)
		OnSettingsChange();
}

void CPPgConnection::OnEnChangeUDPDisable()
{
	if (guardian)
		return;

	uint16 tempVal = 0;
	CString strBuffer;
	TCHAR buffer[510];
	
	guardian = true;
	SetModified();

	GetDlgItem(IDC_UDPPORT)->EnableWindow(!IsDlgButtonChecked(IDC_UDPDISABLE));

	if (GetDlgItem(IDC_UDPPORT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_UDPPORT)->GetWindowText(buffer, 20);
		tempVal = (uint16)_tstoi(buffer);
	}
	else
		buffer[0] = _T('\0');
	
	if (IsDlgButtonChecked(IDC_UDPDISABLE) || (!IsDlgButtonChecked(IDC_UDPDISABLE) && tempVal == 0))
	{
		tempVal = (uint16)_tstoi(buffer) ? (uint16)(_tstoi(buffer)+10) : (uint16)(thePrefs.port+10);
		if (IsDlgButtonChecked(IDC_UDPDISABLE))
			tempVal = 0;
		strBuffer.Format(_T("%d"), tempVal);
		GetDlgItem(IDC_UDPPORT)->SetWindowText(strBuffer);
	}

	if (thePrefs.networkkademlia && !IsDlgButtonChecked(IDC_UDPDISABLE) != 0) // don't use GetNetworkKademlia here
		CheckDlgButton(IDC_NETWORK_KADEMLIA, 1);
	else
		CheckDlgButton(IDC_NETWORK_KADEMLIA, 0);
	GetDlgItem(IDC_NETWORK_KADEMLIA)->EnableWindow(IsDlgButtonChecked(IDC_UDPDISABLE) == 0);

	guardian = false;
}

BOOL CPPgConnection::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

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
		CheckDlgButton(IDC_UDPDISABLE, (thePrefs.udpport == 0));

		GetDlgItem(IDC_UDPPORT)->EnableWindow(thePrefs.udpport > 0);
	
		strBuffer.Format(_T("%.1f"),(float) thePrefs.maxGraphDownloadRate);		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		GetDlgItem(IDC_DOWNLOAD_CAP)->SetWindowText(strBuffer);

		//Xman
		//m_ctlMaxDown.SetRange(1, thePrefs.maxGraphDownloadRate);
		//SetRateSliderTicks(m_ctlMaxDown);

		strBuffer.Format(_T("%.1f"), (float)thePrefs.maxGraphUploadRate);		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		GetDlgItem(IDC_UPLOAD_CAP)->SetWindowText(strBuffer);

		//Xman
		//m_ctlMaxUp.SetRange(1, thePrefs.maxGraphUploadRate);
		//SetRateSliderTicks(m_ctlMaxUp);

		//CheckDlgButton( IDC_DLIMIT_LBL, (thePrefs.maxdownload != UNLIMITED));
		//CheckDlgButton( IDC_ULIMIT_LBL, (thePrefs.maxupload != UNLIMITED));

		//m_ctlMaxDown.SetPos((thePrefs.maxdownload != UNLIMITED) ? thePrefs.maxdownload : thePrefs.maxGraphDownloadRate);
		//m_ctlMaxUp.SetPos((thePrefs.maxupload != UNLIMITED) ? thePrefs.maxupload : thePrefs.maxGraphUploadRate);

		if(thePrefs.GetMaxDownload() >= UNLIMITED){
			// Maybe a string "unlimited" would be better
			strBuffer.Format(_T("%.1f"), 0.0f); // To have ',' or '.' (see french)
			GetDlgItem(IDC_MAXDOWN)->SetWindowText(strBuffer); 
		}
		else {
			strBuffer.Format(_T("%.1f"), (float)thePrefs.GetMaxDownload());
			GetDlgItem(IDC_MAXDOWN)->SetWindowText(strBuffer);
		}
		
		if(thePrefs.GetMaxUpload() >= UNLIMITED){
			// Maybe a string "unlimited" would be better
			strBuffer.Format(_T("%.1f"), 0.0f); // To have ',' or '.' (see french)
			GetDlgItem(IDC_MAXUP)->SetWindowText(strBuffer); 
		}
		else {
			strBuffer.Format(_T("%.1f"), (float)thePrefs.maxupload);
			GetDlgItem(IDC_MAXUP)->SetWindowText(strBuffer);
		}
		//Xman end		

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
			CheckDlgButton(IDC_AUTOCONNECT, 1);

		if (thePrefs.GetNetworkKademlia())
			CheckDlgButton(IDC_NETWORK_KADEMLIA, 1);
		else
			CheckDlgButton(IDC_NETWORK_KADEMLIA, 1);
		GetDlgItem(IDC_NETWORK_KADEMLIA)->EnableWindow(thePrefs.GetUDPPort() > 0);

		if (thePrefs.networked2k)
			CheckDlgButton(IDC_NETWORK_ED2K, 1);
		else
			CheckDlgButton(IDC_NETWORK_ED2K, 0);

		// don't try on XP SP2 or higher, not needed there anymore
		if (thePrefs.GetWindowsVersion() == _WINVER_XP_ && IsRunningXPSP2() == 0 && theApp.m_pFirewallOpener->DoesFWConnectionExist())
			GetDlgItem(IDC_OPENPORTS)->ShowWindow(SW_SHOW);
		else
			GetDlgItem(IDC_OPENPORTS)->ShowWindow(SW_HIDE);
		
		//Xman official UPNP removed
		/*
		if (thePrefs.GetWindowsVersion() != _WINVER_95_ && thePrefs.GetWindowsVersion() != _WINVER_98_ && thePrefs.GetWindowsVersion() != _WINVER_NT4_)
			GetDlgItem(IDC_PREF_UPNPONSTART)->EnableWindow(true);
		else
			GetDlgItem(IDC_PREF_UPNPONSTART)->EnableWindow(false);

		if (thePrefs.IsUPnPEnabled())
			CheckDlgButton(IDC_PREF_UPNPONSTART, 1);
		else
			CheckDlgButton(IDC_PREF_UPNPONSTART, 0);
		*/
		
		//Xman Xtreme Upload
		CalculateMaxUpSlotSpeed();
		m_ctlMaxUp.SetPos((int)(thePrefs.m_slotspeed*10.0f +0.5f));		
		ShowLimitValues();
		//Xman
		
		//Xman GlobalMaxHarlimit for fairness
		strBuffer.Format(_T("%u"),thePrefs.m_uMaxGlobalSources);
		GetDlgItem(IDC_MAXGLOBALSOURCES)->SetWindowText(strBuffer);
		if(thePrefs.m_bAcceptsourcelimit==true)
			CheckDlgButton(IDC_ACCEPTSOURCES,TRUE);
		else
			CheckDlgButton(IDC_ACCEPTRATIO,TRUE);
		//Xman end
	}
}

BOOL CPPgConnection::OnApply()
{
	TCHAR buffer[510];
	//int lastmaxgu = thePrefs.maxGraphUploadRate;
	//int lastmaxgd = thePrefs.maxGraphDownloadRate;
	bool bRestartApp = false;

	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	float lastMaxGraphUploadRate = thePrefs.GetMaxGraphUploadRate();
	float lastMaxGraphDownloadRate = thePrefs.GetMaxGraphDownloadRate();

	//Xman after changing capacity the sysmenu must be updated
	bool caphaschanged = false;
	//Xman end

	// Upload rate max, Upload rate graph
	if(GetDlgItem(IDC_UPLOAD_CAP)->GetWindowTextLength() > 0)
	{ 
		GetDlgItem(IDC_UPLOAD_CAP)->GetWindowText(buffer, 20);
		float upload = (float)_tstof(buffer);
		//Xman after changing capacity the sysmenu must be updated
		if(upload != lastMaxGraphUploadRate)
			caphaschanged = true;
		//Xman end
		if(upload<= 0.0f || upload >= UNLIMITED)
			thePrefs.SetMaxGraphUploadRate(16.0f);
		else if(upload<5.0f)
			thePrefs.SetMaxGraphUploadRate(5.0f);
		else
			thePrefs.SetMaxGraphUploadRate(upload);
	}
	// Download rate max, Download rate graph
	if(GetDlgItem(IDC_DOWNLOAD_CAP)->GetWindowTextLength() > 0)
	{
		GetDlgItem(IDC_DOWNLOAD_CAP)->GetWindowText(buffer, 20);
		float download = (float)_tstof(buffer);
		//Xman after changing capacity the sysmenu must be updated
		if(download != lastMaxGraphDownloadRate)
			caphaschanged = true;
		//Xman end
		thePrefs.SetMaxGraphDownloadRate((download <= 0) ? 96.0f : download);
	}

	//Xman after changing capacity the sysmenu must be updated
	if(caphaschanged==true)
		theApp.emuledlg->Localize(); //dirty hack which updated the sysmenu
	//Xman end

	// Upload rate
	if(GetDlgItem(IDC_MAXUP)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MAXUP)->GetWindowText(buffer,20);
		float upload = (float)_tstof(buffer);
		if(upload<= 0.0f || upload >= UNLIMITED)
			thePrefs.SetMaxUpload(11.0f);
		else if(upload<3.0f)
			thePrefs.SetMaxUpload(3.0f);
		else
			thePrefs.SetMaxUpload(upload);
		
	}
	// Download rate
	if(GetDlgItem(IDC_MAXDOWN)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MAXDOWN)->GetWindowText(buffer,20);
		double download = (float)_tstof(buffer);
		thePrefs.SetMaxDownload((download <= 0.0f || download >= UNLIMITED) ? UNLIMITED : (float)download);
	}

	if (thePrefs.GetMaxGraphUploadRate() < thePrefs.GetMaxUpload() && thePrefs.GetMaxUpload() != UNLIMITED)
		thePrefs.SetMaxUpload(thePrefs.GetMaxGraphUploadRate() * 0.8f);

	if (thePrefs.GetMaxGraphDownloadRate() < thePrefs.GetMaxDownload() && thePrefs.GetMaxDownload() != UNLIMITED)
		thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate() * 0.8f);



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
		uint16 nNewPort = ((uint16)_tstoi(buffer) && !IsDlgButtonChecked(IDC_UDPDISABLE)) ? (uint16)_tstoi(buffer) : (uint16)0;
		if (nNewPort != thePrefs.udpport){
			thePrefs.udpport = nNewPort;
			if (theApp.IsPortchangeAllowed())
				theApp.clientudp->Rebind();
			else 
				bRestartApp = true;
		}
	}

	//Xman
	if (IsDlgButtonChecked(IDC_SHOWOVERHEAD)){
		thePrefs.m_bshowoverhead = true;
	}
	else{
		thePrefs.m_bshowoverhead = false;
	}
	//Xman end

	if (IsDlgButtonChecked(IDC_NETWORK_KADEMLIA))
		thePrefs.SetNetworkKademlia(true);
	else
		thePrefs.SetNetworkKademlia(false);

	if (IsDlgButtonChecked(IDC_NETWORK_ED2K))
		thePrefs.SetNetworkED2K(true);
	else
		thePrefs.SetNetworkED2K(false);

	//	if(IsDlgButtonChecked(IDC_UDPDISABLE)) thePrefs.udpport=0;
	GetDlgItem(IDC_UDPPORT)->EnableWindow(!IsDlgButtonChecked(IDC_UDPDISABLE));

	thePrefs.autoconnect = IsDlgButtonChecked(IDC_AUTOCONNECT)!=0;
	thePrefs.reconnect = IsDlgButtonChecked(IDC_RECONN)!=0;
		
	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
	if(lastMaxGraphUploadRate != thePrefs.GetMaxGraphUploadRate()) 
		theApp.emuledlg->statisticswnd->SetARange(false, (int)thePrefs.GetMaxGraphUploadRate());
	if(lastMaxGraphDownloadRate != thePrefs.GetMaxGraphDownloadRate())
		theApp.emuledlg->statisticswnd->SetARange(true, (int)thePrefs.GetMaxGraphDownloadRate());
	// Maella end


	//Xman Xtreme Upload
	CalculateMaxUpSlotSpeed();
	thePrefs.m_slotspeed=(float)m_ctlMaxUp.GetPos()/10.0f;

	//Xman GlobalMaxHarlimit for fairness
	GetDlgItem(IDC_MAXGLOBALSOURCES)->GetWindowText(buffer,20);
	thePrefs.m_uMaxGlobalSources=(_tstoi(buffer)) ? _tstoi(buffer) : 1000;
	thePrefs.m_bAcceptsourcelimit=IsDlgButtonChecked(IDC_ACCEPTSOURCES)!=0;
	//Xman end

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

	//Xman official UPNP removed
	/*
	if (IsDlgButtonChecked(IDC_PREF_UPNPONSTART) != 0){
		if (!thePrefs.IsUPnPEnabled()){
			thePrefs.m_bEnableUPnP = true;
			if (theApp.m_pUPnPFinder != NULL && thePrefs.IsUPnPEnabled()){
				try
				{
					if (theApp.m_pUPnPFinder->AreServicesHealthy())
						theApp.m_pUPnPFinder->StartDiscovery(thePrefs.GetPort(), thePrefs.GetUDPPort());
				}
				catch ( CUPnPFinder::UPnPError& ) {}
				catch ( CException* e ) { e->Delete(); }
			}
		}
	}
	else
		thePrefs.m_bEnableUPnP = false;
	*/

	theApp.scheduler->SaveOriginals();

	SetModified(FALSE);
	LoadSettings();

	theApp.emuledlg->ShowConnectionState();

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
		//Xman Xtreme upload
		//GetDlgItem(IDC_DLIMIT_LBL)->SetWindowText(GetResString(IDS_PW_DOWNL)); 
		//GetDlgItem(IDC_ULIMIT_LBL)->SetWindowText(GetResString(IDS_PW_UPL));
		GetDlgItem(IDC_DCAP_LBL2)->SetWindowText(GetResString(IDS_PW_CON_DOWNLBL));
		GetDlgItem(IDC_UCAP_LBL2)->SetWindowText(GetResString(IDS_PW_CON_UPLBL));
		GetDlgItem(IDC_UCAP_LBL3)->SetWindowText(GetResString(IDS_UPLOADSLOTSPEED_LABEL));
		GetDlgItem(IDC_STATIC_DOWNINFO)->SetWindowText(GetResString(IDS_STATIC_DOWNINFO));
		//Xman end		
		GetDlgItem(IDC_CONNECTION_NETWORK)->SetWindowText(GetResString(IDS_NETWORK));
		GetDlgItem(IDC_KBS2)->SetWindowText(GetResString(IDS_KBYTESPERSEC));
		GetDlgItem(IDC_KBS3)->SetWindowText(GetResString(IDS_KBYTESPERSEC));
		ShowLimitValues();
		//Xman removed:
		//GetDlgItem(IDC_MAXCONN_FRM)->SetWindowText(GetResString(IDS_PW_CONLIMITS));
		GetDlgItem(IDC_MAXCONLABEL)->SetWindowText(GetResString(IDS_PW_MAXC));
		GetDlgItem(IDC_SHOWOVERHEAD)->SetWindowText(GetResString(IDS_SHOWOVERHEAD));
		GetDlgItem(IDC_CLIENTPORT_FRM)->SetWindowText(GetResString(IDS_PW_CLIENTPORT));
		GetDlgItem(IDC_MAXSRC_FRM)->SetWindowText(GetResString(IDS_PW_MAXSOURCES));
		GetDlgItem(IDC_AUTOCONNECT)->SetWindowText(GetResString(IDS_PW_AUTOCON));
		GetDlgItem(IDC_RECONN)->SetWindowText(GetResString(IDS_PW_RECON));
		GetDlgItem(IDC_MAXSRCHARD_LBL)->SetWindowText(GetResString(IDS_HARDLIMIT));
		GetDlgItem(IDC_WIZARD)->SetWindowText(GetResString(IDS_WIZARD));
		GetDlgItem(IDC_UDPDISABLE)->SetWindowText(GetResString(IDS_UDPDISABLED));
		GetDlgItem(IDC_OPENPORTS)->SetWindowText(GetResString(IDS_FO_PREFBUTTON));
		SetDlgItemText(IDC_STARTTEST, GetResString(IDS_STARTTEST) );
		//Xman official UPNP removed
		//GetDlgItem(IDC_PREF_UPNPONSTART)->SetWindowText(GetResString(IDS_UPNPSTART));
		
		//Xman GlobalMaxHarlimit for fairness
		GetDlgItem(IDC_STATIC_MAXGLOBALSOURCES)->SetWindowText(GetResString(IDS_MAXGLOBALSOURCES));
		GetDlgItem(IDC_ACCEPTSOURCES)->SetWindowText(GetResString(IDS_ACCEPTSOURCES));
		GetDlgItem(IDC_ACCEPTRATIO)->SetWindowText(GetResString(IDS_ACCEPTRATIO));
		//Xman end
	}
}

void CPPgConnection::OnBnClickedWizard()
{
	CConnectionWizardDlg conWizard;
	conWizard.DoModal();
}

void CPPgConnection::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);
/*
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
			m_ctlMaxUp.SetPos(ceil((double)maxdown/3));
		}
		if (maxdown < 41 && maxup*4 < maxdown)
		{
			m_ctlMaxUp.SetPos(ceil((double)maxdown/4));
		}
	}
*/
	ShowLimitValues();

	UpdateData(false); 
	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}
//Xman Xtreme Upload
void CPPgConnection::ShowLimitValues()
{
	CString buffer;

	buffer.Format(_T("%.1f %s"),(float) m_ctlMaxUp.GetPos()/10, GetResString(IDS_KBYTESPERSEC));
	GetDlgItem(IDC_SLOTSPEED_LBL)->SetWindowText(buffer);
}
//Xman end

/* //Xman
void CPPgConnection::OnLimiterChange()
{
	m_ctlMaxDown.ShowWindow(IsDlgButtonChecked(IDC_DLIMIT_LBL) ? SW_SHOW : SW_HIDE);
	m_ctlMaxUp.ShowWindow(IsDlgButtonChecked(IDC_ULIMIT_LBL) ? SW_SHOW : SW_HIDE);

	ShowLimitValues();
	SetModified(TRUE);	
}*/



void CPPgConnection::OnBnClickedOpenports()
{
	OnApply();
	theApp.m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_UDP);
	theApp.m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_TCP);
	bool bAlreadyExisted = false;
	if (theApp.m_pFirewallOpener->DoesRuleExist(thePrefs.GetPort(), NAT_PROTOCOL_TCP) || theApp.m_pFirewallOpener->DoesRuleExist(thePrefs.GetUDPPort(), NAT_PROTOCOL_UDP)){
		bAlreadyExisted = true;
	}
	bool bResult = theApp.m_pFirewallOpener->OpenPort(thePrefs.GetPort(), NAT_PROTOCOL_TCP, EMULE_DEFAULTRULENAME_TCP, false);
	if (thePrefs.GetUDPPort() != 0)
		bResult = bResult && theApp.m_pFirewallOpener->OpenPort(thePrefs.GetUDPPort(), NAT_PROTOCOL_UDP, EMULE_DEFAULTRULENAME_UDP, false);
	if (bResult){
		if (!bAlreadyExisted)
			AfxMessageBox(GetResString(IDS_FO_PREF_SUCCCEEDED), MB_ICONINFORMATION | MB_OK);
		else
			// TODO: actually we could offer the user to remove existing rules
			AfxMessageBox(GetResString(IDS_FO_PREF_EXISTED), MB_ICONINFORMATION | MB_OK);
	}
	else
		AfxMessageBox(GetResString(IDS_FO_PREF_FAILED), MB_ICONSTOP | MB_OK);
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
/* //Xman
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
				for (int i = ((iMin+(iTic-1))/iTic)*iTic; i < iMax; )
				{
					rRate.SetTic(i);
					i += iTic;
				}
			}
			rRate.SetPageSize(iTic);
		}
	}
}
*/
//Xman Xtreme Upload
void CPPgConnection::CalculateMaxUpSlotSpeed()
{
	TCHAR buffer[21];
	float maxUp=10;
	if(GetDlgItem(IDC_MAXUP)->GetWindowText(buffer, 20)>0)
		maxUp=(float)_tstof(buffer);
	float maxSlotSpeed=3.0f;
	if (maxUp<6) maxSlotSpeed=2.0f;
	if (maxUp>=10)
		maxSlotSpeed=maxUp/(3+(maxUp-10)/20.0f);
	if (maxSlotSpeed>XTREME_MAX_SLOTSPEED)
		maxSlotSpeed=XTREME_MAX_SLOTSPEED;
	float a=ceil(maxSlotSpeed*10.0f);
	int b=(int)a;
	if((float)m_ctlMaxUp.GetPos()>a)
	{
		m_ctlMaxUp.SetPos(b);
	}
	m_ctlMaxUp.SetRange(15,b,true);

	//Xman GlobalMaxHarlimit for fairness
	CString strbuffer;
	strbuffer.Format(_T("%u"),(uint32)(maxUp*400 - (maxUp-10.0f)*100));
	GetDlgItem(IDC_MAXGLOBALSOURCES)->SetWindowText(strbuffer);

}




void CPPgConnection::OnEnKillfocusMaxup()
{
		CalculateMaxUpSlotSpeed();
		ShowLimitValues();
}
//Xman end