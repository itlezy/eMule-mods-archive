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
#include "emule.h"
#include "ServerWnd.h"
#include "HttpDownloadDlg.h"
#include "HTRichEditCtrl.h"
#include "ED2KLink.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/prefs.h"
#include "kademlia/utils/MiscUtils.h"
#include "kademlia/kademlia/indexed.h"
#include "kademlia/kademlia/UDPFirewallTester.h"
#include "Kademlia/routing/RoutingZone.h"
#include "OtherFunctions.h"
#include "emuledlg.h"
#include "Server.h"
#include "ServerList.h"
#include "Sockets.h"
#include "MuleStatusBarCtrl.h"
#include "Log.h"
#include "UserMsgs.h"
#include "opcodes.h" //Xman ModID
#include "clientlist.h"
//#include "KadContactHistogramCtrl.h"
#include "UPnPImpl.h" //zz_fly :: show UPnP status
#include "UPnPImplWrapper.h" //zz_fly :: show UPnP status

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define	SVWND_SPLITTER_YOFF		6
#define	SVWND_SPLITTER_HEIGHT	4

#define SZ_DEBUG_LOG_TITLE			_T("Verbose")

void CreateNetworkInfo(CRichEditCtrlX& rCtrl, CHARFORMAT& rcfDef, CHARFORMAT& rcfBold, bool bFullInfo)
{
	CString buffer;
//	if (bFullInfo)
//	{
	///////////////////////////////////////////////////////////////////////////
	// Ports Info
	///////////////////////////////////////////////////////////////////////////
	rCtrl.SetSelectionCharFormat(rcfBold);
	rCtrl << GetResString(IDS_CLIENT) << _T("\r\n");
	rCtrl.SetSelectionCharFormat(rcfDef);

	//rCtrl << GetResString(IDS_PW_NICK) << _T(":\t") << thePrefs.GetUserNick() << _T("\r\n");
	rCtrl << GetResString(IDS_CD_UHASH) << _T("\t") << md4str((uchar*)thePrefs.GetUserHash()) << _T("\r\n");
	rCtrl << _T("TCP-") << GetResString(IDS_PORT) << _T(":\t") << thePrefs.GetPort() << _T("\r\n");
	rCtrl << _T("UDP-") << GetResString(IDS_PORT) << _T(":\t") << thePrefs.GetUDPPort() << _T("\r\n");
	rCtrl << _T("\r\n");
//	}

	///////////////////////////////////////////////////////////////////////////
	// Kademlia
	///////////////////////////////////////////////////////////////////////////
	rCtrl.SetSelectionCharFormat(rcfBold);
	rCtrl << GetResString(IDS_KADEMLIA) << _T(" ") << GetResString(IDS_NETWORK) << _T("\r\n");
	rCtrl.SetSelectionCharFormat(rcfDef);
	
	rCtrl << GetResString(IDS_STATUS) << _T(":\t");
	if(Kademlia::CKademlia::IsConnected()){
		rCtrl << GetResString((Kademlia::CKademlia::IsFirewalled())?IDS_FIREWALLED:IDS_KADOPEN);
		if (Kademlia::CKademlia::IsRunningInLANMode())
			rCtrl << _T(" (") << GetResString(IDS_LANMODE) << _T(")");
		rCtrl << _T("\r\n");
		rCtrl << _T("UDP ") + GetResString(IDS_STATUS) << _T(":\t");
		if(Kademlia::CUDPFirewallTester::IsFirewalledUDP(true))
			rCtrl << GetResString(IDS_FIREWALLED);
		else if (Kademlia::CUDPFirewallTester::IsVerified())
			rCtrl << GetResString(IDS_KADOPEN);
		else{
			CString strTmp = GetResString(IDS_UNVERIFIED);
			strTmp.MakeLower();
			rCtrl << GetResString(IDS_KADOPEN) + _T(" (") + strTmp + _T(")");
		}
		rCtrl << _T("\r\n");

		CString IP;
		IP = ipstr(ntohl(Kademlia::CKademlia::GetPrefs()->GetIPAddress()));
		buffer.Format(_T("%s:%i"), IP, thePrefs.GetUDPPort());
		rCtrl << GetResString(IDS_IP) << _T(":") << GetResString(IDS_PORT) << _T(":\t") << buffer << _T("\r\n");

		//buffer.Format(_T("%u"),Kademlia::CKademlia::GetPrefs()->GetIPAddress());
		buffer.Format(_T("%u"),ntohl(Kademlia::CKademlia::GetPrefs()->GetIPAddress())); //Xman Bugfix
		rCtrl << GetResString(IDS_ID) << _T(":\t") << buffer << _T("\r\n");
		if (Kademlia::CKademlia::GetPrefs()->GetUseExternKadPort() && Kademlia::CKademlia::GetPrefs()->GetExternalKadPort() != 0
			&& Kademlia::CKademlia::GetPrefs()->GetInternKadPort() != Kademlia::CKademlia::GetPrefs()->GetExternalKadPort())
		{
			buffer.Format(_T("%u"), Kademlia::CKademlia::GetPrefs()->GetExternalKadPort());
			rCtrl << GetResString(IDS_EXTERNUDPPORT) << _T(":\t") << buffer << _T("\r\n");
		}
		
		if (Kademlia::CUDPFirewallTester::IsFirewalledUDP(true)) {
			rCtrl << GetResString(IDS_BUDDY) << _T(":\t");
			switch ( theApp.clientlist->GetBuddyStatus() )
			{
				case Disconnected:
					rCtrl << GetResString(IDS_BUDDYNONE);
					break;
				case Connecting:
					rCtrl << GetResString(IDS_CONNECTING);
					break;
				case Connected:
					rCtrl << GetResString(IDS_CONNECTED);
					break;
			}
			rCtrl << _T("\r\n");
		}

		CString sKadID;
		Kademlia::CKademlia::GetPrefs()->GetKadID(&sKadID);
		rCtrl << GetResString(IDS_CD_UHASH) << _T("\t") << sKadID << _T("\r\n");

		//if (bFullInfo)
		//{

			rCtrl << GetResString(IDS_UUSERS) << _T(":\t") << GetFormatedUInt_PTR(Kademlia::CKademlia::GetKademliaUsers()) << _T("\r\n");/*_T(" (Experimental: ") <<  GetFormatedUInt_PTR(Kademlia::CKademlia::GetKademliaUsers(true)) << _T(")\r\n");*/
			rCtrl << GetResString(IDS_PW_FILES) << _T(":\t") << GetFormatedUInt_PTR(Kademlia::CKademlia::GetKademliaFiles()) << _T("\r\n");
			/*rCtrl <<  GetResString(IDS_INDEXED) << _T(":\r\n");
			buffer.Format(GetResString(IDS_KADINFO_SRC) , Kademlia::CKademlia::GetIndexed()->m_uTotalIndexSource);
			rCtrl << buffer;
			buffer.Format(GetResString(IDS_KADINFO_KEYW), Kademlia::CKademlia::GetIndexed()->m_uTotalIndexKeyword);
			rCtrl << buffer;
			//buffer.Format(_T("\t%s: %u\r\n"), GetResString(IDS_NOTES), Kademlia::CKademlia::GetIndexed()->m_uTotalIndexNotes);
			//rCtrl << buffer;
			buffer.Format(_T("\t%s: %u\r\n"), GetResString(IDS_THELOAD), Kademlia::CKademlia::GetIndexed()->m_uTotalIndexLoad);
			rCtrl << buffer;*/
		//}
	}
	else if (Kademlia::CKademlia::IsRunning())
		rCtrl << GetResString(IDS_CONNECTING) << _T("\r\n");
	else
		rCtrl << GetResString(IDS_DISCONNECTED) << _T("\r\n");
	rCtrl << _T("\r\n");

	///////////////////////////////////////////////////////////////////////////
	// ED2K
	///////////////////////////////////////////////////////////////////////////
	rCtrl.SetSelectionCharFormat(rcfBold);
	rCtrl << _T("eD2K ") << GetResString(IDS_NETWORK) << _T("\r\n");
	rCtrl.SetSelectionCharFormat(rcfDef);

	rCtrl << GetResString(IDS_STATUS) << _T(":\t");
	if (theApp.serverconnect->IsConnected())
		rCtrl << GetResString(IDS_CONNECTED);
	else if(theApp.serverconnect->IsConnecting())
		rCtrl << GetResString(IDS_CONNECTING);
	else 
		rCtrl << GetResString(IDS_DISCONNECTED);
	rCtrl << _T("\r\n");
/*
	//I only show this in full display as the normal display is not
	//updated at regular intervals.
	if (bFullInfo && theApp.serverconnect->IsConnected())
	{
		uint32 uTotalUser = 0;
		uint32 uTotalFile = 0;

		theApp.serverlist->GetUserFileStatus(uTotalUser, uTotalFile);
		rCtrl << GetResString(IDS_UUSERS) << _T(":\t") << GetFormatedUInt(uTotalUser) << _T("\r\n");
		rCtrl << GetResString(IDS_PW_FILES) << _T(":\t") << GetFormatedUInt(uTotalFile) << _T("\r\n");
	}
*/
	if (theApp.serverconnect->IsConnected()){
		rCtrl << GetResString(IDS_IP) << _T(":") << GetResString(IDS_PORT) << _T(":") ;
		if (theApp.serverconnect->IsLowID() && theApp.GetPublicIP(true) == 0)
			buffer = GetResString(IDS_UNKNOWN);
		else
			buffer.Format(_T("%s:%u"), ipstr(theApp.GetPublicIP(true)), thePrefs.GetPort());
		rCtrl << _T("\t") << buffer << _T("\r\n");

		rCtrl << GetResString(IDS_ID) << _T(":\t");
		if (theApp.serverconnect->IsConnected()){
			buffer.Format(_T("%u"),theApp.serverconnect->GetClientID());
			rCtrl << buffer;
		}
		rCtrl << _T("\r\n");

		rCtrl << _T("\t");
		rCtrl << GetResString((theApp.serverconnect->IsLowID())?IDS_IDLOW:IDS_IDHIGH);
/*
		CServer* cur_server = theApp.serverconnect->GetCurrentServer();
		CServer* srv = cur_server ? theApp.serverlist->GetServerByAddress(cur_server->GetAddress(), cur_server->GetPort()) : NULL;
		if (srv){
			rCtrl << _T("\r\n");
			rCtrl.SetSelectionCharFormat(rcfBold);
			rCtrl << _T("eD2K ") << GetResString(IDS_SERVER) << _T("\r\n");
			rCtrl.SetSelectionCharFormat(rcfDef);

			rCtrl << GetResString(IDS_SW_NAME) << _T(":\t") << srv->GetListName() << _T("\r\n");
			rCtrl << GetResString(IDS_DESCRIPTION) << _T(":\t") << srv->GetDescription() << _T("\r\n");
			rCtrl << GetResString(IDS_IP) << _T(":") << GetResString(IDS_PORT) << _T(":\t") << srv->GetAddress() << _T(":") << (theApp.serverconnect->IsConnectedObfuscated()?srv->GetObfuscationPortTCP():srv->GetPort()) << _T("\r\n");  //>>> shadow2004::show correct serverport [WiZaRd] 
			//Morph Start - aux Ports, by lugdunummaster
			rCtrl << L"Aux Ports" << _T(":\t") << srv->GetConnPort() << _T("\r\n"); 
			//Morph End - aux Ports, by lugdunummaster
			rCtrl << GetResString(IDS_VERSION) << _T(":\t") << srv->GetVersion() << _T("\r\n");
			rCtrl << GetResString(IDS_UUSERS) << _T(":\t") << GetFormatedUInt(srv->GetUsers()) << _T("\r\n");
			rCtrl << GetResString(IDS_PW_FILES) << _T(":\t") << GetFormatedUInt(srv->GetFiles()) << _T("\r\n");
			rCtrl << GetResString(IDS_FSTAT_CONNECTION) << _T(":\t");
			rCtrl << GetResString((theApp.serverconnect->IsConnectedObfuscated())?IDS_OBFUSCATED:IDS_PRIONORMAL);
			rCtrl << _T("\r\n");


			if (bFullInfo)
			{
				rCtrl << GetResString(IDS_IDLOW) << _T(":\t") << GetFormatedUInt(srv->GetLowIDUsers()) << _T("\r\n");
				rCtrl << GetResString(IDS_PING) << _T(":\t") << srv->GetPing() << _T(" ms\r\n");

				rCtrl << _T("\r\n");
				rCtrl.SetSelectionCharFormat(rcfBold);
				rCtrl << _T("eD2K ") << GetResString(IDS_SERVER) << _T(" ") << GetResString(IDS_FEATURES) << _T("\r\n");
				rCtrl.SetSelectionCharFormat(rcfDef);

				rCtrl << GetResString(IDS_SERVER_LIMITS) << _T(": ") << GetFormatedUInt(srv->GetSoftFiles()) << _T("/") << GetFormatedUInt(srv->GetHardFiles()) << _T("\r\n");

				if (thePrefs.IsExtControlsEnabled()){
					rCtrl << GetResString(IDS_SRV_TCPCOMPR) << _T(": ");
					rCtrl << GetResString((srv->GetTCPFlags() & SRV_TCPFLG_COMPRESSION)?IDS_YES:IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_SHORTTAGS) << _T(": ");
					rCtrl << GetResString(((srv->GetTCPFlags() & SRV_TCPFLG_NEWTAGS) || (srv->GetUDPFlags() & SRV_UDPFLG_NEWTAGS))?IDS_YES:IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << _T("Unicode") << _T(": ");
					rCtrl << GetResString(((srv->GetTCPFlags() & SRV_TCPFLG_UNICODE) || (srv->GetUDPFlags() & SRV_UDPFLG_UNICODE))?IDS_YES:IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_SERVERFEATURE_INTTYPETAGS) << _T(": ");
					rCtrl << GetResString((srv->GetTCPFlags() & SRV_TCPFLG_TYPETAGINTEGER)?IDS_YES:IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_SRV_UDPSR) << _T(": ");
					rCtrl << GetResString((srv->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES)?IDS_YES:IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_SRV_UDPSR) << _T(" #2: ");
					rCtrl << GetResString((srv->GetUDPFlags() & SRV_UDPFLG_EXT_GETSOURCES2)?IDS_YES:IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_SRV_UDPFR) << _T(": ");
					if (srv->GetUDPFlags() & SRV_UDPFLG_EXT_GETFILES)
						rCtrl << GetResString(IDS_YES);
					else
						rCtrl << GetResString(IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_SRV_LARGEFILES) << _T(": ");
					if (srv->SupportsLargeFilesTCP() || srv->SupportsLargeFilesUDP())
						rCtrl << GetResString(IDS_YES);
					else
						rCtrl << GetResString(IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_PROTOCOLOBFUSCATION) << _T(" (UDP)") << _T(": ");
					if (srv->SupportsObfuscationUDP())
						rCtrl << GetResString(IDS_YES);
					else
						rCtrl << GetResString(IDS_NO);
					rCtrl << _T("\r\n");

					rCtrl << GetResString(IDS_PROTOCOLOBFUSCATION) << _T(" (TCP)") << _T(": ");
					if (srv->SupportsObfuscationTCP())
						rCtrl << GetResString(IDS_YES);
					else
						rCtrl << GetResString(IDS_NO);
					rCtrl << _T("\r\n");
				}
			}
		}
*/
			rCtrl << _T("\r\n");
		}

	//zz_fly :: show UPnP status :: start
	//if(thePrefs.GetUPnPNat())
	if(thePrefs.IsUPnPEnabled() && theApp.m_pUPnPFinder)
	{
		rCtrl << _T("\r\n");
		rCtrl.SetSelectionCharFormat(rcfBold);
		rCtrl << GetResString(IDS_UPNPSTATUS) << _T("\r\n");
		rCtrl.SetSelectionCharFormat(rcfDef);
		//CString upnpinfo = theApp.m_UPnPNat.GetLastError(); //ACAT UPnP
		CString upnpinfo = theApp.m_pUPnPFinder->GetImplementation() ? theApp.m_pUPnPFinder->GetImplementation()->GetStatusString() : _T(""); //Official UPNP
		rCtrl << (upnpinfo.IsEmpty() ? _T("Unknown") : upnpinfo) << _T("\r\n");
	}
	//zz_fly :: show UPnP status :: end

	//morph4u mod +
	rCtrl << _T("\r\n");
    rCtrl.SetSelectionCharFormat(rcfBold);
	rCtrl << _T("About") << _T("\r\n"); 
    rCtrl.SetSelectionCharFormat(rcfDef);
    rCtrl << _T("Version") << _T(":\t") << _T("eMule v") + theApp.m_strCurVersionLong + _T(" -LPE-") << _T("\r\n"); 
    rCtrl << _T("Build") << _T(":\t") << __DATE__ << _T("\r\n"); 
	CString str = _T("https://www.facebook.com/morph4u");
	str.Replace(_T("https://"), _T(""));
	rCtrl << _T("Support") << _T(":\t") << str << _T("\r\n");
	//morph4u mod -
}

// CServerWnd dialog

IMPLEMENT_DYNAMIC(CServerWnd, CDialog)

BEGIN_MESSAGE_MAP(CServerWnd, CResizableDialog)
	//ON_BN_CLICKED(IDC_ADDSERVER, OnBnClickedAddserver)
	//ON_BN_CLICKED(IDC_UPDATESERVERMETFROMURL, OnBnClickedUpdateServerMetFromUrl)
	//ON_BN_CLICKED(IDC_UPDATENODESDATFROMURL, OnBnClickedUpdateNodesDatFromUrl)
	//ON_BN_CLICKED(IDC_BOOTSTRAPBUTTON, OnBnClickedBootstrapbutton)
	//ON_BN_CLICKED(IDC_FIREWALLCHECKBUTTON, OnBnClickedFirewallcheckbutton)
	//ON_BN_CLICKED(IDC_KADCONNECT, OnBnKADConnect)
	//ON_EN_SETFOCUS(IDC_BOOTSTRAPIP, OnEnSetfocusBootstrapip)
	//ON_EN_CHANGE(IDC_BOOTSTRAPIP, UpdateControlsState)
	//ON_EN_CHANGE(IDC_BOOTSTRAPPORT, UpdateControlsState)
	//ON_BN_CLICKED(IDC_RADCLIENTS, UpdateControlsState)
	//ON_BN_CLICKED(IDC_RADIP, UpdateControlsState)
	ON_BN_CLICKED(IDC_LOGRESET, OnBnClickedResetLog)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB3, OnTcnSelchangeTab3)
	ON_NOTIFY(EN_LINK, IDC_LOGBOX, OnEnLinkLogBox)
	//ON_WM_SYSCOLORCHANGE()
	ON_WM_CTLCOLOR()
	ON_STN_DBLCLK(IDC_SERVLST_ICO, OnStnDblclickServlstIco)
	ON_NOTIFY(UM_SPN_SIZED, IDC_SPLITTER_SERVER, OnSplitterMoved)
END_MESSAGE_MAP()

CServerWnd::CServerWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CServerWnd::IDD, pParent)
{
	//m_contactHistogramCtrl = new CKadContactHistogramCtrl;
	logbox = new CHTRichEditCtrl;
	debuglog = new CHTRichEditCtrl;
	icon_srvlist = NULL;
	memset(&m_cfDef, 0, sizeof m_cfDef);
	memset(&m_cfBold, 0, sizeof m_cfBold);
	StatusSelector.m_bCloseable = false;
}

CServerWnd::~CServerWnd()
{
	if (icon_srvlist)
		VERIFY( DestroyIcon(icon_srvlist) );
	//delete m_contactHistogramCtrl;
	delete debuglog;
	delete logbox;
}

BOOL CServerWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();
	if (theApp.m_fontLog.m_hObject == NULL)
	{
		CFont* pFont = GetDlgItem(IDC_MYINFO)->GetFont();
		LOGFONT lf;
		pFont->GetObject(sizeof lf, &lf);
		theApp.m_fontLog.CreateFontIndirect(&lf);
	}


	// using ES_NOHIDESEL is actually not needed, but it helps to get around a tricky window update problem!
	// If that style is not specified there are troubles with right clicking into the control for the very first time!?
#define	LOG_PANE_RICHEDIT_STYLES WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY | ES_NOHIDESEL
	RECT rect;

	GetDlgItem(IDC_LOGBOX)->GetWindowRect(&rect);
	GetDlgItem(IDC_LOGBOX)->DestroyWindow();
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
	if (logbox->Create(LOG_PANE_RICHEDIT_STYLES, rect, this, IDC_LOGBOX)){
		logbox->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
		logbox->SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
		logbox->SetEventMask(logbox->GetEventMask() | ENM_LINK);
		if (theApp.m_fontLog.m_hObject)
			logbox->SetFont(&theApp.m_fontLog);
		logbox->SetTitle(GetResString(IDS_SV_LOG));
		logbox->SetAutoURLDetect(FALSE);
		logbox->AppendText(_T("eMule v") + theApp.m_strCurVersionLong + _T(" -LPE-")); 
		logbox->AppendText(_T("\n\n")); 
	}

	GetDlgItem(IDC_DEBUG_LOG)->GetWindowRect(&rect);
	GetDlgItem(IDC_DEBUG_LOG)->DestroyWindow();
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&rect, 2);
	if (debuglog->Create(LOG_PANE_RICHEDIT_STYLES, rect, this, IDC_DEBUG_LOG)){
		debuglog->ModifyStyleEx(0, WS_EX_STATICEDGE, SWP_FRAMECHANGED);
		debuglog->SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
		if (theApp.m_fontLog.m_hObject)
			debuglog->SetFont(&theApp.m_fontLog);
		debuglog->SetTitle(SZ_DEBUG_LOG_TITLE);
		debuglog->SetAutoURLDetect(FALSE);
	}

	SetAllIcons();
	serverlistctrl.Init();

	TCITEM newitem;
	/*CString name;// X: [RUL] - [Remove Useless Localize]
	name = GetResString(IDS_SV_LOG);
	name.Replace(_T("&"), _T("&&"));*/
	newitem.mask = TCIF_TEXT | TCIF_IMAGE;
	newitem.pszText = _T("")/*const_cast<LPTSTR>((LPCTSTR)name)*/;
	newitem.iImage = 0;
	VERIFY( StatusSelector.InsertItem(StatusSelector.GetItemCount(), &newitem) == PaneLog );

	/*name = SZ_DEBUG_LOG_TITLE;
	name.Replace(_T("&"), _T("&&"));
	newitem.mask = TCIF_TEXT | TCIF_IMAGE;*/
	newitem.pszText = _T("")/*const_cast<LPTSTR>((LPCTSTR)name)*/;
	newitem.iImage = 0;
	VERIFY( StatusSelector.InsertItem(StatusSelector.GetItemCount(), &newitem) == PaneVerboseLog );

	Localize();

	//AddAnchor(*m_contactHistogramCtrl,TOP_RIGHT);
	//AddAnchor(IDC_FIREWALLCHECKBUTTON, TOP_RIGHT);
	//AddAnchor(IDC_KADCONNECT, TOP_RIGHT);
	//AddAnchor(IDC_BSSTATIC, TOP_RIGHT);
	//AddAnchor(IDC_ADDSERVER, TOP_RIGHT);
	//AddAnchor(IDC_BOOTSTRAPBUTTON, TOP_RIGHT);
	//AddAnchor(IDC_BOOTSTRAPPORT, TOP_RIGHT);
	//AddAnchor(IDC_BOOTSTRAPIP, TOP_RIGHT);
	//AddAnchor(IDC_SSTATIC4, TOP_RIGHT);
	//AddAnchor(IDC_SSTATIC7, TOP_RIGHT);
	//AddAnchor(IDC_RADCLIENTS, TOP_RIGHT);
	//AddAnchor(IDC_RADIP, TOP_RIGHT);
	//AddAnchor(IDC_KADFRAME, TOP_RIGHT); //morph4u

	AddAnchor(IDC_SERVLST_ICO, TOP_LEFT);
	AddAnchor(IDC_SERVLIST_TEXT, TOP_LEFT);
	AddAnchor(serverlistctrl, TOP_LEFT, MIDDLE_RIGHT);
	AddAnchor(IDC_MYINFO, TOP_RIGHT, BOTTOM_RIGHT);
	AddAnchor(m_MyInfo, TOP_RIGHT, BOTTOM_RIGHT);
	//AddAnchor(IDC_UPDATESERVERMETFROMURL, TOP_RIGHT);
	//AddAnchor(IDC_UPDATENODESDATFROMURL, TOP_RIGHT);
	AddAnchor(StatusSelector, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(IDC_LOGRESET, MIDDLE_RIGHT); // avoid resizing GUI glitches with the tab control by adding this control as the last one (Z-order)
	// The resizing of those log controls (rich edit controls) works 'better' when added as last anchors (?)
	AddAnchor(*logbox, MIDDLE_LEFT, BOTTOM_RIGHT);
	AddAnchor(*debuglog, MIDDLE_LEFT, BOTTOM_RIGHT);

	// Set the tab control to the bottom of the z-order. This solves a lot of strange repainting problems with
	// the rich edit controls (the log panes).
	::SetWindowPos(StatusSelector, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE | SWP_NOCOPYBITS | SWP_NOMOVE | SWP_NOSIZE);

	//CheckDlgButton(IDC_RADCLIENTS,1);
	debug = true;
	ToggleDebugWindow();

	debuglog->ShowWindow(SW_HIDE);
	logbox->ShowWindow(SW_SHOW);

	// optional: restore last used log pane
	if (thePrefs.GetRestoreLastLogPane())
	{
		if (thePrefs.GetLastLogPaneID() >= 0 && thePrefs.GetLastLogPaneID() < StatusSelector.GetItemCount())
		{
			int iCurSel = StatusSelector.GetCurSel();
			StatusSelector.SetCurSel(thePrefs.GetLastLogPaneID());
			if (thePrefs.GetLastLogPaneID() == StatusSelector.GetCurSel())
				UpdateLogTabSelection();
			else
				StatusSelector.SetCurSel(iCurSel);
		}
	}

	m_MyInfo.SendMessage(EM_SETMARGINS, EC_LEFTMARGIN | EC_RIGHTMARGIN, MAKELONG(3, 3));
	m_MyInfo.SetAutoURLDetect();
	m_MyInfo.SetEventMask(m_MyInfo.GetEventMask() | ENM_LINK);

	PARAFORMAT pf = {0};
	pf.cbSize = sizeof pf;
	if (m_MyInfo.GetParaFormat(pf)){
		pf.dwMask |= PFM_TABSTOPS;
		pf.cTabCount = 4;
		pf.rgxTabs[0] = 900;
		pf.rgxTabs[1] = 1000;
		pf.rgxTabs[2] = 1100;
		pf.rgxTabs[3] = 1200;
		m_MyInfo.SetParaFormat(pf);
	}

	m_cfDef.cbSize = sizeof m_cfDef;
	if (m_MyInfo.GetSelectionCharFormat(m_cfDef)){
		m_cfBold = m_cfDef;
		m_cfBold.dwMask |= CFM_BOLD;
		m_cfBold.dwEffects |= CFE_BOLD;
	}

	InitWindowStyles(this);

	// splitter
	RECT rcSpl;
	rcSpl.left = 55;
	rcSpl.right = 300;
	rcSpl.top = 55;
	rcSpl.bottom = rcSpl.top + SVWND_SPLITTER_HEIGHT;
	m_wndSplitter.Create(WS_CHILD | WS_VISIBLE, rcSpl, this, IDC_SPLITTER_SERVER);
	m_wndSplitter.SetDrawBorder(true);
	InitSplitter();

	return true;
}

void CServerWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SERVLIST, serverlistctrl);
	DDX_Control(pDX, IDC_TAB3, StatusSelector);
	DDX_Control(pDX, IDC_MYINFOLIST, m_MyInfo);
	//DDX_Control(pDX, IDC_KAD_HISTOGRAM, *m_contactHistogramCtrl);
}
/*
void CServerWnd::OnEnSetfocusBootstrapip()
{
	CheckRadioButton(IDC_RADIP, IDC_RADCLIENTS, IDC_RADIP);
}
*/
/*void CServerWnd::OnBnClickedBootstrapbutton()
{
	CString strIP;
	uint16 nPort = 0;

	if (!IsDlgButtonChecked(IDC_RADCLIENTS))
	{
		GetDlgItemText(IDC_BOOTSTRAPIP,strIP);
		strIP.Trim();

		// auto-handle ip:port
		int iPos;
		if ((iPos = strIP.Find(_T(':'))) != -1)
		{
			SetDlgItemText(IDC_BOOTSTRAPPORT,strIP.Mid(iPos+1));
			strIP = strIP.Left(iPos);
			SetDlgItemText(IDC_BOOTSTRAPIP,strIP);
		}

		CString strPort;
		GetDlgItemText(IDC_BOOTSTRAPPORT,strPort);
		strPort.Trim();
		nPort = (uint16)_ttoi(strPort);

		// invalid IP/Port
		if (strIP.GetLength()<7 || nPort==0)
			return;
	}

	if( !Kademlia::CKademlia::IsRunning() )
	{
		Kademlia::CKademlia::Start();
		theApp.emuledlg->ShowConnectionState();
	}
	if (!strIP.IsEmpty() && nPort)
	{
		// JOHNTODO - Switch between Kad1 and Kad2
		Kademlia::CKademlia::Bootstrap(strIP, nPort);
       }
}*/


/*void CServerWnd::OnBnClickedFirewallcheckbutton()
{
	Kademlia::CKademlia::RecheckFirewalled();
}*/

/*void CServerWnd::OnBnKADConnect()
{
	if (Kademlia::CKademlia::IsConnected())
		Kademlia::CKademlia::Stop();
	else if (Kademlia::CKademlia::IsRunning())
		Kademlia::CKademlia::Stop();
	else
		Kademlia::CKademlia::Start();
	theApp.emuledlg->ShowConnectionState();
}*/

/*void CServerWnd::ShowContacts()
{
	m_contactHistogramCtrl->ShowWindow(SW_SHOW);
}

void CServerWnd::HideContacts()
{
	m_contactHistogramCtrl->ShowWindow(SW_HIDE);
}

void CServerWnd::ContactAdd(const Kademlia::CContact* contact)
{
	m_contactHistogramCtrl->ContactAdd(contact);
}

void CServerWnd::ContactRem(const Kademlia::CContact* contact)
{
	m_contactHistogramCtrl->ContactRem(contact);
}*/
/*
void CServerWnd::OnSysColorChange()
{
	CResizableDialog::OnSysColorChange();
	SetAllIcons();
}
*/
void CServerWnd::SetAllIcons()
{
	CImageList iml;
	iml.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 0, 1);
	iml.Add(CTempIconLoader(_T("FILEINFO"))); //0
	StatusSelector.SetImageList(&iml);
	m_imlLogPanes.DeleteImageList();
	m_imlLogPanes.Attach(iml.Detach());

	if (icon_srvlist)
		VERIFY( DestroyIcon(icon_srvlist) );
	icon_srvlist = theApp.LoadIcon(_T("KADSERVER"), 16, 16);
	((CStatic*)GetDlgItem(IDC_SERVLST_ICO))->SetIcon(icon_srvlist);
}

void CServerWnd::Localize()// X: [RUL] - [Remove Useless Localize]
{
	//SetDlgItemText(IDC_BSSTATIC,GetResString(IDS_BOOTSTRAP));
	//SetDlgItemText(IDC_ADDSERVER,GetResString(IDS_ADD));
	//SetDlgItemText(IDC_BOOTSTRAPBUTTON,GetResString(IDS_BOOTSTRAP));
	//SetDlgItemText(IDC_SSTATIC4,GetResString(IDS_SV_ADDRESS) + _T(':'));
	//SetDlgItemText(IDC_SSTATIC7,GetResString(IDS_SV_PORT) + _T(':'));
	//SetDlgItemText(IDC_FIREWALLCHECKBUTTON,GetResString(IDS_KAD_RECHECKFW));
	
	//SetDlgItemText(IDC_RADCLIENTS,GetResString(IDS_RADCLIENTS));
	//SetDlgItemText(IDC_UPDATESERVERMETFROMURL, _T("eD2K ") + GetResString(IDS_NETWORK) +  _T(" ") + GetResString(IDS_SV_SERVERLIST));
	//SetDlgItemText(IDC_UPDATENODESDATFROMURL,GetResString(IDS_KADCONTACTLAB));

	//SetDlgItemText(IDC_SERVLIST_TEXT,GetResString(IDS_SV_SERVERLIST));
	SetDlgItemText(IDC_LOGRESET,GetResString(IDS_PW_RESET));
	SetDlgItemText(IDC_MYINFO,GetResString(IDS_MYINFO));
	//SetDlgItemText(IDC_KADFRAME,GetResString(IDS_SV_UPDATE));

	TCITEM item;
	CString name = GetResString(IDS_SV_LOG);
	name.Replace(_T("&"), _T("&&"));
	item.mask = TCIF_TEXT;
	item.pszText = const_cast<LPTSTR>((LPCTSTR)name);
	StatusSelector.SetItem(PaneLog, &item);

	name = SZ_DEBUG_LOG_TITLE;
	name.Replace(_T("&"), _T("&&"));
	//item.mask = TCIF_TEXT;
	item.pszText = const_cast<LPTSTR>((LPCTSTR)name);
	StatusSelector.SetItem(PaneVerboseLog, &item);

	UpdateLogTabSelection();
	//UpdateControlsState();
	//m_contactHistogramCtrl->Localize();
}

/*void CServerWnd::OnBnClickedAddserver()
{
	CString serveraddr;
	GetDlgItemText(IDC_BOOTSTRAPIP,serveraddr);
	serveraddr.Trim();
	if (serveraddr.IsEmpty()) {
		AfxMessageBox(GetResString(IDS_SRV_ADDR));
		return;
	}

	uint16 uPort = 0;
	if (_tcsnicmp(serveraddr, _T("ed2k://"), 7) == 0){
		CED2KLink* pLink = NULL;
		try{
			pLink = CED2KLink::CreateLinkFromUrl(serveraddr);
			serveraddr.Empty();
			if (pLink && pLink->GetKind() == CED2KLink::kServer){
				//CED2KServerLink* pServerLink = pLink->GetServerLink();
				//if (pServerLink){
				CED2KServerLink* pServerLink = (CED2KServerLink*)pLink;
				serveraddr = pServerLink->GetAddress();
				uPort = pServerLink->GetPort();
				SetDlgItemText(IDC_BOOTSTRAPIP, serveraddr);
				SetDlgItemInt(IDC_BOOTSTRAPPORT, uPort, FALSE);
				//}
			}
		}
		catch(CString strError){
			AfxMessageBox(strError);
			serveraddr.Empty();
		}
		delete pLink;
	}
	else{
		// if the port is specified with the IP, ignore any possible specified port in the port control
		int iColon = serveraddr.Find(_T(':'));
		if (iColon != -1) {
			uPort = (uint16)_tstoi(serveraddr.Mid(iColon + 1));
			serveraddr = serveraddr.Left(iColon);
		}
		else {
			if (!GetDlgItem(IDC_BOOTSTRAPPORT)->GetWindowTextLength()){
				AfxMessageBox(GetResString(IDS_SRV_PORT));
				return;
			}

			BOOL bTranslated = FALSE;
			uPort = (uint16)GetDlgItemInt(IDC_BOOTSTRAPPORT, &bTranslated, FALSE);
			if (!bTranslated){
				AfxMessageBox(GetResString(IDS_SRV_PORT));
				return;
			}
		}
	}

	if (serveraddr.IsEmpty() || uPort == 0){
		AfxMessageBox(GetResString(IDS_SRV_ADDR));
		return;
	}

	AddServer(uPort, serveraddr);
}*/

void CServerWnd::PasteServerFromClipboard()
{
	CString strServer = theApp.CopyTextFromClipboard();
	strServer.Trim();
	if (strServer.IsEmpty())
		return;

	int nPos = 0;
	CString strTok = strServer.Tokenize(_T(" \t\r\n"), nPos);
	while (!strTok.IsEmpty())
	{
		CString strAddress;
		uint16 nPort = 0;
		CED2KLink* pLink = NULL;
		try{
			pLink = CED2KLink::CreateLinkFromUrl(strTok);
			if (pLink && pLink->GetKind() == CED2KLink::kServer){
				//CED2KServerLink* pServerLink = pLink->GetServerLink();
				//if (pServerLink){
				CED2KServerLink* pServerLink = (CED2KServerLink*)pLink;
				strAddress = pServerLink->GetAddress();
				nPort = pServerLink->GetPort();
				//}
			}
		}
		catch(CString strError){
			AfxMessageBox(strError);
		}
		delete pLink;

		if (strAddress.IsEmpty() || nPort == 0)
			break;

		(void)AddServer(nPort, strAddress, _T(""), false);
		strTok = strServer.Tokenize(_T(" \t\r\n"), nPos);
	}
}

bool CServerWnd::AddServer(uint16 nPort, CString strAddress, CString strName, bool bShowErrorMB)
{
	CServer* toadd = new CServer(nPort, strAddress);

	// Barry - Default all manually added servers to high priority
	//if (thePrefs.GetManualAddedServersHighPriority())
		toadd->SetPreference(SRV_PR_HIGH);

	if (strName.IsEmpty())
		strName = strAddress;
	toadd->SetListName(strName);

	if (!serverlistctrl.AddServer(toadd, true))
	{
		CServer* pFoundServer = theApp.serverlist->GetServerByAddress(toadd->GetAddress(), toadd->GetPort());
		if (pFoundServer == NULL && toadd->GetIP() != 0)
			pFoundServer = theApp.serverlist->GetServerByIPTCP(toadd->GetIP(), toadd->GetPort());
		if (pFoundServer)
		{
			static const TCHAR _aszServerPrefix[] = _T("Server");
			if (_tcsnicmp(toadd->GetListName(), _aszServerPrefix, ARRSIZE(_aszServerPrefix)-1) != 0)
			{
				pFoundServer->SetListName(toadd->GetListName());
				serverlistctrl.RefreshServer(pFoundServer);
			}
		}
		else
		{
			if (bShowErrorMB)
				AfxMessageBox(GetResString(IDS_SRV_NOTADDED));
		}
		delete toadd;
		return false;
	}
	else
	{
		AddLogLine(true, GetResString(IDS_SERVERADDED), toadd->GetListName());
		return true;
	}
}

/*void CServerWnd::OnBnClickedUpdateServerMetFromUrl()
{
    CString strURL;
           strURL = thePrefs.GetServerMetUpdateURL();
		UpdateServerMetFromURL(strURL);
}*/

/*void CServerWnd::OnBnClickedUpdateNodesDatFromUrl()
{
    CString strURL;
		    strURL = thePrefs.GetNodesDatUpdateURL();
		UpdateNodesDatFromURL(strURL);
}*/

void CServerWnd::OnBnClickedResetLog()
{
	int cur_sel = StatusSelector.GetCurSel();
	if (cur_sel == -1)
		return;
	if (cur_sel == PaneVerboseLog)
	{
		theApp.emuledlg->ResetDebugLog();
		theApp.emuledlg->statusbar->SetText(_T(""), SBarLog, 0);
	}
	if (cur_sel == PaneLog)
	{
		theApp.emuledlg->ResetLog();
		theApp.emuledlg->statusbar->SetText(_T(""), SBarLog, 0);
	}
}

void CServerWnd::OnTcnSelchangeTab3(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	UpdateLogTabSelection();
	*pResult = 0;
}

void CServerWnd::UpdateLogTabSelection()
{
	int cur_sel = StatusSelector.GetCurSel();
	if (cur_sel == -1)
		return;
	if (cur_sel == PaneVerboseLog)
	{
		logbox->ShowWindow(SW_HIDE);
		debuglog->ShowWindow(SW_SHOW);
		if (debuglog->IsAutoScroll() && (StatusSelector.GetItemState(cur_sel, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED))
			debuglog->ScrollToLastLine(true);
		debuglog->Invalidate();
		StatusSelector.HighlightItem(cur_sel, FALSE);
	}
        if (cur_sel == PaneLog)
	{
		debuglog->ShowWindow(SW_HIDE);
		logbox->ShowWindow(SW_SHOW);
		if (logbox->IsAutoScroll() && (StatusSelector.GetItemState(cur_sel, TCIS_HIGHLIGHTED) & TCIS_HIGHLIGHTED))
			logbox->ScrollToLastLine(true);
		logbox->Invalidate();
		StatusSelector.HighlightItem(cur_sel, FALSE);
	}
}

void CServerWnd::ToggleDebugWindow()
{
	int cur_sel = StatusSelector.GetCurSel();
	if (thePrefs.GetVerbose() && !debug)
	{
		TCITEM newitem;
		CString name;
		name = SZ_DEBUG_LOG_TITLE;
		name.Replace(_T("&"), _T("&&"));
		newitem.mask = TCIF_TEXT | TCIF_IMAGE;
		newitem.pszText = const_cast<LPTSTR>((LPCTSTR)name);
		newitem.iImage = 0;
		StatusSelector.InsertItem(StatusSelector.GetItemCount(),&newitem);
		debug = true;
	}
	else if (!thePrefs.GetVerbose() && debug)
	{
		if (cur_sel == PaneVerboseLog)
		{
			StatusSelector.SetCurSel(PaneLog);
			StatusSelector.SetFocus();
		}
		debuglog->ShowWindow(SW_HIDE);
		logbox->ShowWindow(SW_SHOW);
		StatusSelector.DeleteItem(PaneVerboseLog);
		debug = false;
	}
}

void CServerWnd::UpdateMyInfo()
{
	m_MyInfo.SetRedraw(FALSE);
	m_MyInfo.SetWindowText(_T(""));
	CreateNetworkInfo(m_MyInfo, m_cfDef, m_cfBold, thePrefs.GetVerbose());
	m_MyInfo.SetRedraw(TRUE);
	m_MyInfo.Invalidate();
}

CString CServerWnd::GetMyInfoString() {
	CString buffer;
	m_MyInfo.GetWindowText(buffer);

	return buffer;
}

BOOL CServerWnd::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN)
	{
		// Don't handle Ctrl+Tab in this window. It will be handled by main window.
		if (pMsg->wParam == VK_TAB && GetAsyncKeyState(VK_CONTROL) < 0)
			return FALSE;
		if (pMsg->wParam == VK_ESCAPE)
			return FALSE;

	}

	return CResizableDialog::PreTranslateMessage(pMsg);
}

void CServerWnd::OnEnLinkLogBox(NMHDR *pNMHDR, LRESULT *pResult)
{
	*pResult = 0;
	ENLINK* pEnLink = reinterpret_cast<ENLINK *>(pNMHDR);
	if (pEnLink && pEnLink->msg == WM_LBUTTONDOWN)
	{
		CString strUrl;
		ShellExecute(NULL, NULL, strUrl, NULL, NULL, SW_SHOWDEFAULT);
		*pResult = 1;
	}
}

/*void CServerWnd::UpdateControlsState()
{
	CString strLabel;
	UINT StrID;
	if (Kademlia::CKademlia::IsConnected())
		StrID = IDS_MAIN_BTN_DISCONNECT;
	else if (Kademlia::CKademlia::IsRunning())
		StrID = IDS_MAIN_BTN_CANCEL;
	else
		StrID = IDS_MAIN_BTN_CONNECT;
	strLabel = GetResString(IDS_KADEMLIA) + _T(' ') + GetResString(IDS_NETWORK) + _T(' ') + GetResString(StrID);
	strLabel.Remove(_T('&'));
	SetDlgItemText(IDC_KADCONNECT,strLabel);

	CString strBootstrapIP;
	GetDlgItemText(IDC_BOOTSTRAPIP, strBootstrapIP);
	CString strBootstrapPort;
	GetDlgItemText(IDC_BOOTSTRAPPORT, strBootstrapPort);
	bool isipempty = IsDlgButtonChecked(IDC_RADIP)>0 && !strBootstrapIP.IsEmpty();
	GetDlgItem(IDC_ADDSERVER)->EnableWindow(isipempty);
	GetDlgItem(IDC_BOOTSTRAPBUTTON)->EnableWindow(
		!Kademlia::CKademlia::IsConnected()
		&& (  (   isipempty && (strBootstrapIP.Find(_T(':')) != -1 || !strBootstrapPort.IsEmpty())
			  )
		    || IsDlgButtonChecked(IDC_RADCLIENTS)>0));
}*/

void CServerWnd::SaveAllSettings()
{
	thePrefs.SetLastLogPaneID(StatusSelector.GetCurSel());
}

void CServerWnd::OnStnDblclickServlstIco()
{
	theApp.emuledlg->ShowPreferences(IDD_PPG_PROXY);//morph4u IDD_PPG_SERVER -> IDD_PPG_PROXY
}

void CServerWnd::DoResize(int delta)
{
	//if(!delta)
		//return;
	CSplitterControl::ChangeHeight(&serverlistctrl, delta, CW_TOPALIGN);
	CSplitterControl::ChangeHeight(&StatusSelector, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(logbox, -delta, CW_BOTTOMALIGN);
	CSplitterControl::ChangeHeight(debuglog, -delta, CW_BOTTOMALIGN);
	UpdateSplitterRange();
}

void CServerWnd::InitSplitter()
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	m_wndSplitter.SetRange(rcWnd.top+100,rcWnd.bottom-50);
	LONG splitpos = 5+(thePrefs.GetSplitterbarPositionServer() * rcWnd.Height()) / 100;

	CRect rcDlgItem;

	serverlistctrl.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.bottom=splitpos-10;
	serverlistctrl.MoveWindow(rcDlgItem);

	GetDlgItem(IDC_LOGRESET)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = splitpos + 9;
	rcDlgItem.bottom = splitpos + 30;
	GetDlgItem(IDC_LOGRESET)->MoveWindow(rcDlgItem);

	StatusSelector.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = splitpos + 10;
	rcDlgItem.bottom = rcWnd.bottom-3;
	StatusSelector.MoveWindow(rcDlgItem);

	logbox->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top=splitpos+35;
	rcDlgItem.bottom = rcWnd.bottom-9;
	logbox->MoveWindow(rcDlgItem);

	//debuglog->GetWindowRect(rcDlgItem);
	//ScreenToClient(rcDlgItem);
	//rcDlgItem.top=splitpos+35;
	//rcDlgItem.bottom = rcWnd.bottom-9;
	debuglog->MoveWindow(rcDlgItem);

	long right=rcDlgItem.right;
	GetDlgItem(IDC_SPLITTER_SERVER)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.right=right;
	GetDlgItem(IDC_SPLITTER_SERVER)->MoveWindow(rcDlgItem);

	ReattachAnchors();
}

void CServerWnd::ReattachAnchors()
{
	RemoveAnchor(serverlistctrl);
	RemoveAnchor(StatusSelector);
	RemoveAnchor(IDC_LOGRESET);
	RemoveAnchor(*logbox);
	RemoveAnchor(*debuglog);

	AddAnchor(serverlistctrl, TOP_LEFT, CSize(100, thePrefs.GetSplitterbarPositionServer()));
	AddAnchor(StatusSelector, CSize(0, thePrefs.GetSplitterbarPositionServer()), BOTTOM_RIGHT);
	AddAnchor(IDC_LOGRESET, MIDDLE_RIGHT);
	AddAnchor(*logbox, CSize(0, thePrefs.GetSplitterbarPositionServer()), BOTTOM_RIGHT);
	AddAnchor(*debuglog, CSize(0, thePrefs.GetSplitterbarPositionServer()), BOTTOM_RIGHT);

	GetDlgItem(IDC_LOGRESET)->Invalidate();

	if (logbox->IsWindowVisible())
		logbox->Invalidate();
	if (debuglog->IsWindowVisible())
		debuglog->Invalidate();
}

void CServerWnd::UpdateSplitterRange()
{
	CRect rcWnd;
	GetWindowRect(rcWnd);
	ScreenToClient(rcWnd);

	CRect rcDlgItem;
	serverlistctrl.GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);

	m_wndSplitter.SetRange(rcWnd.top + 100, rcWnd.bottom - 50);

	LONG splitpos = rcDlgItem.bottom + SVWND_SPLITTER_YOFF;
	thePrefs.SetSplitterbarPositionServer((splitpos  * 100) / rcWnd.Height());

	GetDlgItem(IDC_LOGRESET)->GetWindowRect(rcDlgItem);
	ScreenToClient(rcDlgItem);
	rcDlgItem.top = splitpos + 9;
	rcDlgItem.bottom = splitpos + 30;
	GetDlgItem(IDC_LOGRESET)->MoveWindow(rcDlgItem);

	ReattachAnchors();
}

LRESULT CServerWnd::DefWindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{
	switch (message)
	{
		// arrange transferwindow layout
		case WM_PAINT:
			if (m_wndSplitter)
			{
				CRect rcWnd;
				GetWindowRect(rcWnd);
				if (rcWnd.Height() > 0)
				{
					RECT rcDown;
					serverlistctrl.GetWindowRect(&rcDown);
					ScreenToClient(&rcDown);

					// splitter paint update
					RECT rcSpl;
					rcSpl.left = 3;
					rcSpl.right = rcDown.right;
					rcSpl.top = rcDown.bottom + SVWND_SPLITTER_YOFF;
					rcSpl.bottom = rcSpl.top + SVWND_SPLITTER_HEIGHT;
					m_wndSplitter.MoveWindow(&rcSpl, TRUE);
					UpdateSplitterRange();
				}
			}
			break;

		case WM_WINDOWPOSCHANGED:
			if (m_wndSplitter)
				m_wndSplitter.Invalidate();
			break;
	}

	return CResizableDialog::DefWindowProc(message, wParam, lParam);
}

void CServerWnd::OnSplitterMoved(NMHDR* pNMHDR, LRESULT* /*pResult*/)
{
	SPC_NMHDR* pHdr = (SPC_NMHDR*)pNMHDR;
	DoResize(pHdr->delta);
}

HBRUSH CServerWnd::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{
	HBRUSH hbr = theApp.emuledlg->GetCtlColor(pDC, pWnd, nCtlColor);
	if (hbr)
		return hbr;
	return __super::OnCtlColor(pDC, pWnd, nCtlColor);
}
