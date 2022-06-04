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
#include "SpeedGraphWnd.h" // X: [SGW] - [SpeedGraphWnd]

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgConnection, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgConnection, CPropertyPage)
	ON_BN_CLICKED(IDC_STARTTEST, OnStartPortTest)
	ON_EN_CHANGE(IDC_DOWNLOAD_CAP, OnSettingsChange)
	//ON_BN_CLICKED(IDC_UDPDISABLE, OnEnChangeUDPDisable)
	ON_EN_CHANGE(IDC_UDPPORT, OnEnChangeUDP)
	ON_EN_CHANGE(IDC_UPLOAD_CAP, OnSettingsChange)
	ON_EN_CHANGE(IDC_PORT, OnEnChangeTCP)
	ON_EN_CHANGE(IDC_MAXCON, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXSOURCEPERFILE, OnSettingsChange)
	//ON_BN_CLICKED(IDC_AUTOCONNECT, OnSettingsChange)
	//ON_BN_CLICKED(IDC_REFUSEUPLOAD, OnRUChange)// X: [RU] - [RefuseUpload]
	ON_BN_CLICKED(IDC_RANDOMPORTONSTARTUP, OnSettingsChange) // X: [RPOS] - [RandomPortOnStartup]
	ON_BN_CLICKED(IDC_RECONN, OnSettingsChange)
	ON_BN_CLICKED(IDC_NETWORK_ED2K, OnSettingsChange)
	//Xman Xtreme Upload
	//ON_BN_CLICKED(IDC_ULIMIT_LBL, OnLimiterChange)
	//ON_BN_CLICKED(IDC_DLIMIT_LBL, OnLimiterChange)
	ON_EN_CHANGE(IDC_MAXDOWN, OnSettingsChange)
	//ON_EN_CHANGE(IDC_RUMAXDOWN, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXUP, OnSettingsChange)
	ON_EN_KILLFOCUS(IDC_MAXUP, OnEnKillfocusMaxup)
	//Xman end
	ON_WM_HSCROLL()
	//ON_BN_CLICKED(IDC_NETWORK_KADEMLIA, OnSettingsChange)
	ON_BN_CLICKED(IDC_OPENPORTS, OnBnClickedOpenports)
	//ON_BN_CLICKED(IDC_PREF_UPNPONSTART, OnSettingsChange) //Xman official UPNP removed
	ON_BN_CLICKED(IDC_NAFCFULLCONTROL, OnSettingsChange) // X: [CI] - [Code Improvement]
	ON_BN_CLICKED(IDC_PBF_CHECK, OnSettingsChange) 
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
	//DDX_Control(pDX, IDC_RUMAXDOWN, m_ruMax);
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
	uint16 tcp = (uint16)GetDlgItemInt(IDC_PORT,NULL,FALSE);
	uint16 udp = (uint16)GetDlgItemInt(IDC_UDPPORT,NULL,FALSE);

	GetDlgItem(IDC_STARTTEST)->EnableWindow( 
		tcp == theApp.listensocket->GetConnectedPort() && 
		udp == theApp.clientudp->GetConnectedPort() 
	);

	if (istcpport == 0)
		OnEnChangeUDPDisable();
	else if (istcpport == 1)
		OnSettingsChange();
}
/*
void CPPgConnection::OnRUChange()// X: [RU] - [RefuseUpload]
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
	m_ruMax.EnableWindow(IsDlgButtonChecked(IDC_REFUSEUPLOAD));
}
*/
void CPPgConnection::OnEnChangeUDPDisable()
{
	if (guardian)
		return;

	guardian = true;
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified

	//GetDlgItem(IDC_UDPPORT)->EnableWindow(!IsDlgButtonChecked(IDC_UDPDISABLE));

	//if (IsDlgButtonChecked(IDC_UDPDISABLE))
	//	SetDlgItemInt(IDC_UDPPORT,0,FALSE);
	/*else*/ if(GetDlgItem(IDC_UDPPORT)->GetWindowTextLength()>0 && GetDlgItemInt(IDC_UDPPORT,NULL,FALSE)==0)
		SetDlgItemInt(IDC_UDPPORT,thePrefs.port+10,FALSE);

	// don't use GetNetworkKademlia here
	//CheckDlgButton(IDC_NETWORK_KADEMLIA, (thePrefs.networkkademlia && !IsDlgButtonChecked(IDC_UDPDISABLE) != 0));
	//GetDlgItem(IDC_NETWORK_KADEMLIA)->EnableWindow(IsDlgButtonChecked(IDC_UDPDISABLE) == 0);

	guardian = false;
}

BOOL CPPgConnection::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();

	OnEnChangePorts(2);
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
				  // EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgConnection::LoadSettings(void)
{
	if (m_hWnd)
	{
		if (thePrefs.maxupload != 0)
			thePrefs.maxdownload = thePrefs.GetMaxDownload();

		//SetDlgItemInt(IDC_UDPPORT,thePrefs.udpport,FALSE);
		//CheckDlgButton(IDC_UDPDISABLE, (thePrefs.udpport == 0));

		//GetDlgItem(IDC_UDPPORT)->EnableWindow(thePrefs.udpport > 0);
		CString strBuffer;
	
		strBuffer.Format(_T("%.1f"),(float) thePrefs.maxGraphDownloadRate);		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		m_maxDownloadCapacity.SetWindowText(strBuffer);


		strBuffer.Format(_T("%.1f"), (float)thePrefs.maxGraphUploadRate);		// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		m_maxUploadCapacity.SetWindowText(strBuffer);


		// Maybe a string "unlimited" would be better
		strBuffer.Format(_T("%.1f"), 
			(thePrefs.GetMaxDownload() >= UNLIMITED)?
				0.0f
			:
				(float)thePrefs.GetMaxDownload()); // To have ',' or '.' (see french)
		m_maxDownload.SetWindowText(strBuffer);
		
		strBuffer.Format(_T("%.1f"), 
			(thePrefs.GetMaxUpload() >= UNLIMITED)?
				0.0f
			:
				(float)thePrefs.maxupload); // To have ',' or '.' (see french)
		m_maxUpload.SetWindowText(strBuffer);

		//Xman end		

		//trBuffer.Format(_T("%.1f"), (float)thePrefs.rumax/1024);// X: [RU] - [RefuseUpload]
		//m_ruMax.SetWindowText(strBuffer);

		SetDlgItemInt(IDC_UDPPORT,thePrefs.udpport,FALSE);
		SetDlgItemInt(IDC_PORT,thePrefs.port,FALSE);
		SetDlgItemInt(IDC_MAXCON,thePrefs.maxconnections,FALSE);
		SetDlgItemInt(IDC_MAXSOURCEPERFILE,
			(thePrefs.maxsourceperfile == 0xFFFF)?
				0
			:
				thePrefs.maxsourceperfile,FALSE);

		CheckDlgButton(IDC_RECONN, thePrefs.reconnect);
		//CheckDlgButton(IDC_AUTOCONNECT, thePrefs.autoconnect);
		//CheckDlgButton(IDC_REFUSEUPLOAD, thePrefs.refuseupload);// X: [RU] - [RefuseUpload]
		CheckDlgButton(IDC_RANDOMPORTONSTARTUP, thePrefs.randomPortOnStartup); // X: [RPOS] - [RandomPortOnStartup]
		//CheckDlgButton(IDC_NETWORK_KADEMLIA, thePrefs.GetNetworkKademlia());
		CheckDlgButton(IDC_NETWORK_ED2K, thePrefs.networked2k);
		//GetDlgItem(IDC_NETWORK_KADEMLIA)->EnableWindow(thePrefs.GetUDPPort() > 0);
		//m_ruMax.EnableWindow(IsDlgButtonChecked(IDC_REFUSEUPLOAD));// X: [RU] - [RefuseUpload]
		CheckDlgButton(IDC_NAFCFULLCONTROL, thePrefs.GetNAFCFullControl());
		CheckDlgButton(IDC_PBF_CHECK, thePrefs.m_bPayBackFirst);

		// don't try on XP SP2 or higher, not needed there anymore
		GetDlgItem(IDC_OPENPORTS)->ShowWindow(
			(thePrefs.GetWindowsVersion() == _WINVER_XP_ && IsRunningXPSP2() == 0 && theApp.m_pFirewallOpener->DoesFWConnectionExist())?
				SW_SHOW
			:
				SW_HIDE
			);
		
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
	}
}

BOOL CPPgConnection::OnApply()
{
	if(m_bModified){ // X: [CI] - [Code Improvement] Apply if modified
	TCHAR buffer[21];
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
	if(m_maxUploadCapacity.GetWindowTextLength() > 0)
	{ 
		m_maxUploadCapacity.GetWindowText(buffer, 20);
		float upload = (float)_tstof(buffer);
		//Xman after changing capacity the sysmenu must be updated
		if(upload != lastMaxGraphUploadRate)
			caphaschanged = true;
		//Xman end
		thePrefs.SetMaxGraphUploadRate(
			(upload<= 0.0f || upload >= UNLIMITED)?
				16.0f
				:(upload<5.0f)?
					5.0f
			:
				upload);
	}
	// Download rate max, Download rate graph
	if(m_maxDownloadCapacity.GetWindowTextLength() > 0)
	{
		m_maxDownloadCapacity.GetWindowText(buffer, 20);
		float download = (float)_tstof(buffer);
		//Xman after changing capacity the sysmenu must be updated
		if(download != lastMaxGraphDownloadRate)
			caphaschanged = true;
		//Xman end
		thePrefs.SetMaxGraphDownloadRate((download <= 0) ? 96.0f : download);
	}

	//Xman after changing capacity the sysmenu must be updated
	if(caphaschanged==true)
		theApp.emuledlg->CreateTrayMenues(); //dirty hack which updated the sysmenu
	//Xman end

	// Upload rate
	if(m_maxUpload.GetWindowTextLength()>0)
	{
		m_maxUpload.GetWindowText(buffer,20);
		float upload = (float)_tstof(buffer);
		
			if(upload<= 0.0f || upload >= UNLIMITED)
				upload = 11.0f;
			else if(upload<3.0f)
				upload = 3.0f;
			if (thePrefs.GetMaxGraphUploadRate() < upload)
				upload = thePrefs.GetMaxGraphUploadRate() * 0.8f;

			thePrefs.SetMaxUpload(upload);
	}
	// Download rate
	if(m_maxDownload.GetWindowTextLength()>0)
	{
		m_maxDownload.GetWindowText(buffer, 20);
			float download = (float)_tstof(buffer);
			thePrefs.SetMaxDownload((download <= 0.0f || download >= UNLIMITED) ? UNLIMITED : download);
	}

	if (thePrefs.GetMaxGraphDownloadRate() < thePrefs.GetMaxDownload() && thePrefs.GetMaxDownload() != UNLIMITED)
		thePrefs.SetMaxDownload(thePrefs.GetMaxGraphDownloadRate() * 0.8f);
	/*
	if(m_ruMax.GetWindowTextLength())// X: [RU] - [RefuseUpload]
	{
		m_ruMax.GetWindowText(buffer, 20);
		double download = _tstof(buffer);
		if(download > 0.0f)
			thePrefs.rumax =(uint32)(download*1024);
	}
	*/
	if (GetDlgItem(IDC_PORT)->GetWindowTextLength())
	{
		uint16 nNewPort = GetDlgItemInt(IDC_PORT, NULL, FALSE);
		if (nNewPort!=0 && nNewPort != thePrefs.port){
			thePrefs.port = nNewPort;
			if (theApp.IsPortchangeAllowed())
				theApp.listensocket->Rebind();
			else
				bRestartApp = true;
		}
	}
	
	if (GetDlgItem(IDC_MAXSOURCEPERFILE)->GetWindowTextLength())
	{
		uint32 nMaxsourceperfile=GetDlgItemInt(IDC_MAXSOURCEPERFILE, NULL, FALSE);
		if(nMaxsourceperfile)
			thePrefs.maxsourceperfile = nMaxsourceperfile;
	}

	if (GetDlgItem(IDC_UDPPORT)->GetWindowTextLength())
	{
		uint16 nNewPort = GetDlgItemInt(IDC_UDPPORT, NULL, FALSE);
		if (nNewPort!=0 /*&& !IsDlgButtonChecked(IDC_UDPDISABLE)*/ && nNewPort != thePrefs.udpport){
			thePrefs.udpport = nNewPort;
			if (theApp.IsPortchangeAllowed())
				theApp.clientudp->Rebind();
			else 
				bRestartApp = true;
		}
	}

	//thePrefs.SetNetworkKademlia(IsDlgButtonChecked(IDC_NETWORK_KADEMLIA)!=0);
	thePrefs.SetNetworkED2K(IsDlgButtonChecked(IDC_NETWORK_ED2K)!=0);

	//	if(IsDlgButtonChecked(IDC_UDPDISABLE)) thePrefs.udpport=0;
	//GetDlgItem(IDC_UDPPORT)->EnableWindow(!IsDlgButtonChecked(IDC_UDPDISABLE));

	//thePrefs.autoconnect = IsDlgButtonChecked(IDC_AUTOCONNECT)!=0;
	thePrefs.reconnect = IsDlgButtonChecked(IDC_RECONN)!=0;
	//thePrefs.refuseupload = IsDlgButtonChecked(IDC_REFUSEUPLOAD)!=0;// X: [RU] - [RefuseUpload]
	thePrefs.randomPortOnStartup = IsDlgButtonChecked(IDC_RANDOMPORTONSTARTUP)!=0;// X: [RPOS] - [RandomPortOnStartup]
	thePrefs.SetNAFCFullControl(IsDlgButtonChecked(IDC_NAFCFULLCONTROL)!=0);
	thePrefs.m_bPayBackFirst = IsDlgButtonChecked(IDC_PBF_CHECK)!=0;

	// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
		if(lastMaxGraphUploadRate != thePrefs.GetMaxGraphUploadRate()){
		theApp.emuledlg->statisticswnd->SetARange(false, (int)thePrefs.GetMaxGraphUploadRate());
			if(theApp.m_pSpeedGraphWnd)
				theApp.m_pSpeedGraphWnd->SetUSpeedMeterRange((uint32)thePrefs.GetMaxGraphUploadRate()); // X: [SGW] - [SpeedGraphWnd]
		}
		if(lastMaxGraphDownloadRate != thePrefs.GetMaxGraphDownloadRate()){
		theApp.emuledlg->statisticswnd->SetARange(true, (int)thePrefs.GetMaxGraphDownloadRate());
			if(theApp.m_pSpeedGraphWnd)
				theApp.m_pSpeedGraphWnd->SetDSpeedMeterRange((uint32)thePrefs.GetMaxGraphDownloadRate()); // X: [SGW] - [SpeedGraphWnd]
		}
	// Maella end


	//Xman Xtreme Upload
	CalculateMaxUpSlotSpeed();
	thePrefs.m_slotspeed=(float)m_ctlMaxUp.GetPos()/10.0f;
       
	if (GetDlgItem(IDC_MAXCON)->GetWindowTextLength()){
		UINT tempcon = GetDlgItemInt(IDC_MAXCON, NULL, FALSE);
		if(tempcon==0)
			tempcon=CPreferences::GetRecommendedMaxConnections();
		thePrefs.maxconnections = tempcon;
	}


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

	SetModified(FALSE);
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	LoadSettings();

	theApp.emuledlg->ShowConnectionState();
    
	// X-Ray :: AutoRestartIfNecessary :: Start
	/*
	if (bRestartApp)
		AfxMessageBox(GetResString(IDS_NOPORTCHANGEPOSSIBLE));
	*/
	if (bRestartApp && AfxMessageBox(GetResString(IDS_RESTARTEMULEONCHANGE), MB_YESNO | MB_ICONEXCLAMATION, 0) == IDYES)
		theApp.emuledlg->RestartMuleApp();
	// X-Ray :: AutoRestartIfNecessary :: End
	
	OnEnChangePorts(2);
	}

	return CPropertyPage::OnApply();
}

void CPPgConnection::Localize(void)
{	
	if (m_hWnd)
	{
		SetWindowText(GetResString(IDS_PW_CONNECTION));
		SetDlgItemText(IDC_CAPACITIES_FRM,GetResString(IDS_PW_CON_CAPFRM));
		SetDlgItemText(IDC_DCAP_LBL,GetResString(IDS_PW_CON_DOWNLBL));
		SetDlgItemText(IDC_UCAP_LBL,GetResString(IDS_PW_CON_UPLBL));
		SetDlgItemText(IDC_LIMITS_FRM,GetResString(IDS_PW_CON_LIMITFRM));
		//Xman Xtreme upload
		//SetDlgItemText(IDC_DLIMIT_LBL,GetResString(IDS_PW_DOWNL)); 
		//SetDlgItemText(IDC_ULIMIT_LBL,GetResString(IDS_PW_UPL));
		SetDlgItemText(IDC_DCAP_LBL2,GetResString(IDS_PW_CON_DOWNLBL));
		SetDlgItemText(IDC_UCAP_LBL2,GetResString(IDS_PW_CON_UPLBL));
		SetDlgItemText(IDC_UCAP_LBL3,GetResString(IDS_UPLOADSLOTSPEED_LABEL));
		SetDlgItemText(IDC_STATIC_DOWNINFO,GetResString(IDS_STATIC_DOWNINFO));
		//Xman end		
		//SetDlgItemText(IDC_CONNECTION_NETWORK,GetResString(IDS_NETWORK));
		ShowLimitValues();
		SetDlgItemText(IDC_MAXCONN_FRM,GetResString(IDS_PW_CONLIMITS));
		SetDlgItemText(IDC_MAXCONLABEL,GetResString(IDS_PW_MAXC));
		SetDlgItemText(IDC_CLIENTPORT_FRM,GetResString(IDS_PW_CLIENTPORT));
		SetDlgItemText(IDC_MAXSRC_FRM,GetResString(IDS_PW_MAXSOURCES));
		//SetDlgItemText(IDC_AUTOCONNECT,GetResString(IDS_PW_AUTOCON));
		//SetDlgItemText(IDC_REFUSEUPLOAD,GetResString(IDS_PW_REFUSEUPLOAD));// X: [RU] - [RefuseUpload]
		SetDlgItemText(IDC_RANDOMPORTONSTARTUP,GetResString(IDS_RANDOMPORTONSTARTUP));// X: [RPOS] - [RandomPortOnStartup]
		SetDlgItemText(IDC_RECONN,GetResString(IDS_PW_RECON));
		SetDlgItemText(IDC_MAXSRCHARD_LBL,GetResString(IDS_HARDLIMIT));
		//SetDlgItemText(IDC_UDPDISABLE,GetResString(IDS_UDPDISABLED));
		SetDlgItemText(IDC_OPENPORTS,GetResString(IDS_FO_PREFBUTTON));
		SetDlgItemText(IDC_STARTTEST,GetResString(IDS_STARTTEST));
		//Xman official UPNP removed
		//SetDlgItemText(IDC_PREF_UPNPONSTART,GetResString(IDS_UPNPSTART));
		SetDlgItemText(IDC_NAFCFULLCONTROL,GetResString(IDS_PPG_MAELLA_NAFC_CHECK02));
		SetDlgItemText(IDC_PBF_CHECK,GetResString(IDS_PBFDETAIL));
		SetDlgItemText(IDC_NETWORK_ED2K,_T("eD2K ") + GetResString(IDS_NETWORK) + _T(" (") +  GetResString(IDS_PW_AUTOCON) + _T(")"));
	}
}

void CPPgConnection::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
//Xman
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

	buffer.Format(_T("%.1f KB/s"),(float) m_ctlMaxUp.GetPos()/10);
	SetDlgItemText(IDC_SLOTSPEED_LBL,buffer);
}
//Xman end

//Xman
/*
void CPPgConnection::OnLimiterChange()
{
	m_ctlMaxDown.ShowWindow(IsDlgButtonChecked(IDC_DLIMIT_LBL) ? SW_SHOW : SW_HIDE);
	m_ctlMaxUp.ShowWindow(IsDlgButtonChecked(IDC_ULIMIT_LBL) ? SW_SHOW : SW_HIDE);

	ShowLimitValues();
	OnSettingsChange(); // X: [CI] - [Code Improvement] Apply if modified
}
*/
//Xman end

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
	TriggerPortTest(
		GetDlgItemInt(IDC_PORT,NULL,FALSE),
		GetDlgItemInt(IDC_UDPPORT,NULL,FALSE)
	);
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
	CString strbuffer;
	float maxUp=10;
	if(GetDlgItemText(IDC_MAXUP,strbuffer)>0)
		maxUp=(float)_tstof(strbuffer);
	int newMax=(int)ceil(GetMaxSlotSpeed(maxUp)*10.0f);
	int oldMax = m_ctlMaxUp.GetRangeMax();
	if(newMax != oldMax)
	{
		int newPos = (newMax * m_ctlMaxUp.GetPos() + oldMax / 2) / oldMax;
		if(newPos < 15)
			newPos = 15;
		m_ctlMaxUp.SetRange(15, newMax,true);
		m_ctlMaxUp.SetPos(newPos);
     }
}

void CPPgConnection::OnEnKillfocusMaxup()
{
		CalculateMaxUpSlotSpeed();
		ShowLimitValues();
}
//Xman end