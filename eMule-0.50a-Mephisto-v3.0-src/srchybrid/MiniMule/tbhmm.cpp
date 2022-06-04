// TBHMM.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "TBHMM.h"
#include "preferences.h"
#include "downloadqueue.h"
#include "uploadqueue.h"
#include "server.h"
#include "sockets.h"
#include "emuledlg.h"
#include "opcodes.h"
#include "WebServices.h"
#include "otherfunctions.h"
#include "Version.h"
#include "Statistics.h"
#include "TransferDlg.h" // added - Stulle
#include "Log.h"
#include "kademlia/kademlia/kademlia.h"
#include "kademlia/kademlia/prefs.h"
#include "BandWidthControl.h" // Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-

// TBHMM dialog
//IMPLEMENT_DYNAMIC(CTBHMM, CSnapDialog)
CTBHMM::CTBHMM(CWnd* pParent /*=NULL*/)
: CSnapDialog(CTBHMM::IDD, pParent) 
{
	for (int i = 0; i < ARRSIZE(m_hConState); i++)
		m_hConState[i] = NULL;
	smmin = thePrefs.GetSpeedMeterMin();
	smmax = thePrefs.GetSpeedMeterMax();
	running = false;
	m_nLastUpdate = ::GetTickCount(); //Fafner: fixed uninitialized memory read (BC) - 1213
}

CTBHMM::~CTBHMM()
{
	running = false;
	for (int i = 0; i < ARRSIZE(m_hConState); i++){
		if (m_hConState[i]) VERIFY( ::DestroyIcon(m_hConState[i]) );
	}
	m_ctrlSpeedMeter.DestroyWindow();
}

void CTBHMM::DoDataExchange(CDataExchange* pDX)
{
	CSnapDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MM_CONNSTATE, m_ctrlMMConnState);
	// ==> added - Stulle
	DDX_Control(pDX, IDC_MM_INCOMING, m_btnIncoming);
	DDX_Control(pDX, IDC_MM_RESTORE, m_btnRestore);
	// <== added - Stulle
}

BEGIN_MESSAGE_MAP(CTBHMM, CSnapDialog)
	ON_BN_CLICKED(IDC_MM_MENU, OnMenuButtonClicked)
	// ==> added - Stulle
	ON_BN_CLICKED(IDC_MM_INCOMING, OnIncomingClicked)
	ON_BN_CLICKED(IDC_MM_RESTORE, OnRestoreClicked)
	// <== added - Stulle
END_MESSAGE_MAP()

// TBHMM message handlers
BOOL CTBHMM::OnInitDialog()
{
	CSnapDialog::OnInitDialog();
	InitWindowStyles(this);
	sysinfo.Init();
	CString buffer = theApp.m_strModLongVersion;	
	CString buffer2;
	buffer += _T(" ");
	buffer += _T("Mini-Mule");
	buffer += _T("     ||     ");	
	// ==> changed - Stulle
	buffer2.Format(_T("CPU: %3d%% Mem: %s"),theApp.sysinfo->GetCpuUsage(), CastItoXBytes(sysinfo.GetProcessMemoryUsageInt()));
	// <== changed - Stulle
	buffer += buffer2;
	SetWindowText(buffer);

	m_hConState[0] = theApp.LoadIcon(_T("MM_DISDIS"), 32, 32);
	m_hConState[1] = theApp.LoadIcon(_T("MM_DISLOW"), 32, 32);
	m_hConState[2] = theApp.LoadIcon(_T("MM_DISCON"), 32, 32);
	m_hConState[3] = theApp.LoadIcon(_T("MM_LOWDIS"), 32, 32);
	m_hConState[4] = theApp.LoadIcon(_T("MM_LOWLOW"), 32, 32);
	m_hConState[5] = theApp.LoadIcon(_T("MM_LOWCON"), 32, 32);
	m_hConState[6] = theApp.LoadIcon(_T("MM_CONDIS"), 32, 32);
	m_hConState[7] = theApp.LoadIcon(_T("MM_CONLOW"), 32, 32);
	m_hConState[8] = theApp.LoadIcon(_T("MM_CONCON"), 32, 32);

	// ==> added - Stulle
	m_btnIncoming.SetIcon(_T("OPENFOLDER"));	
	m_btnIncoming.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnIncoming.SetLeftAlign(true); 
	m_btnRestore.SetIcon(_T("RESTOREWINDOW"));
	m_btnRestore.SetAlign(CButtonST::ST_ALIGN_HORIZ);
	m_btnRestore.SetLeftAlign(true);
	// <== added - Stulle

	RECT rect;
	UINT nPlaceholderID = IDC_SM_PLACEHOLDER;
	this->GetDlgItem(nPlaceholderID)->GetWindowRect(&rect);
	this->ScreenToClient(&rect);
	m_ctrlSpeedMeter.CreateEx(WS_EX_STATICEDGE, NULL, NULL, WS_CHILD | WS_VISIBLE, rect, this, 123, 0);
	m_ctrlSpeedMeter.SetRange(smmin,smmax);
	m_ctrlSpeedMeter.EnableWindow(true);
	RunMiniMule();
	return true;
}

// [TPT] - Improved minimule
void CTBHMM::RunMiniMule(bool resetMiniMule)
{
	reset = resetMiniMule;
	if (!running)
	{
		running = true;
		AfxBeginThread(run, this, THREAD_PRIORITY_LOWEST);	
	}
}

UINT CTBHMM::run(LPVOID p)
{
	CTBHMM * minimule = (CTBHMM *)p;
	minimule->run();
	return 0;
}

void CTBHMM::run()
{
	MMUpdate();
    running = FALSE;
}

void CTBHMM::MMUpdate()
{
 //   CSingleLock sLock2(&theApp.minimulemutex); // wait for the main.
 //   sLock2.Lock(); // mmupdate acces thing that should not be deleted. 

	if (theApp.m_app_state == APP_STATE_SHUTTINGDOWN ||
		!theApp.emuledlg->IsRunning()) 
		return;
	try
	{

		uint32 mmUpdateIt = thePrefs.GetMiniMuleUpdate();
		if (::GetTickCount() - m_nLastUpdate < mmUpdateIt*1000)
			return;
		m_nLastUpdate = ::GetTickCount();
		if (reset)
		{
			smmin = thePrefs.GetSpeedMeterMin();
			smmax = thePrefs.GetSpeedMeterMax();
			m_ctrlSpeedMeter.SetRange(smmin,smmax);
			m_ctrlSpeedMeter.SoftReset();
		}
		TCHAR buffer3[50];
		CString buffer;
		CString buffer2;

		SetTransparent(thePrefs.GetMiniMuleTransparency());
	
		buffer = theApp.m_strModLongVersion;	
		buffer += _T(" ");
		buffer += _T("Mini-Mule");
		buffer += _T("     ||     ");
		// ==> changed - Stulle
		buffer2.Format(_T("CPU: %3d%% Mem: %s"),theApp.sysinfo->GetCpuUsage(), CastItoXBytes(sysinfo.GetProcessMemoryUsageInt()));
		// <== changed - Stulle
		buffer += buffer2;
		SetWindowText(buffer);

/*
		int lastuprateoverheadkb = theStats.GetUpDatarateOverhead();
		int lastdownrateoverheadkb = theStats.GetDownDatarateOverhead();
		uint32 upratekb = theApp.uploadqueue->GetDatarate();
		uint32 downratekb = theApp.downloadqueue->GetDatarate();
*/
		CDownloadQueue::SDownloadStats myStats;
		theApp.downloadqueue->GetDownloadSourcesStats(myStats);

		UINT uIconIdx = theApp.emuledlg->GetConnectionStateIconIndex();
		if (uIconIdx >= ARRSIZE(m_hConState)){
			ASSERT(0);
			uIconIdx = 0;
		}
		m_ctrlMMConnState.SetIcon(m_hConState[uIconIdx]);

		CString ConBuffer = _T("");
		if(theApp.serverconnect->IsConnected())
		{
			ConBuffer.AppendFormat(_T("%s"), theApp.serverconnect->GetCurrentServer()->GetListName());
			buffer.Format(_T("%u"),theApp.serverconnect->GetClientID());
			if (theApp.serverconnect->IsLowID())
				buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_IDLOW));
			else 
				buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_IDHIGH));
		}
		else if(Kademlia::CKademlia::IsConnected())
		{
			buffer.Format(_T("%u"),Kademlia::CKademlia::GetPrefs()->GetIPAddress());
			if (Kademlia::CKademlia::IsFirewalled())
				buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_FIREWALLED));
			else 
				buffer.AppendFormat(_T(" [%s]"),GetResString(IDS_KADOPEN));
		}
		if(Kademlia::CKademlia::IsConnected() && theApp.serverconnect->IsConnected())
			ConBuffer.AppendFormat(_T(" + "));
		else if (!Kademlia::CKademlia::IsConnected() && !theApp.serverconnect->IsConnected())
		{
			ConBuffer.AppendFormat(_T("Not Connected"));
			buffer.Format(_T("Not Connected"));
		}
		if(Kademlia::CKademlia::IsConnected())
			ConBuffer.AppendFormat(_T("KAD"));

		GetDlgItem(IDC_MM_SERVER)->SetWindowText(ConBuffer);
		GetDlgItem(IDC_MM_ID)->SetWindowText(buffer);
/*
		if (theApp.serverconnect != NULL)
		{
			if (theApp.serverconnect->IsConnected())
				m_ctrlMMConnState.SetIcon(m_hCSDconn);
			else if (theApp.serverconnect->IsConnecting())
				m_ctrlMMConnState.SetIcon(m_hCSCing);
			else
				m_ctrlMMConnState.SetIcon(m_hCSConn);
			if (theApp.serverconnect->IsConnected())
			{ 
				buffer.Format(_T("%u"),theApp.serverconnect->GetClientID());
				if (theApp.serverconnect->IsLowID()) buffer2 = GetResString(IDS_IDLOW);
				else buffer2 = GetResString(IDS_IDHIGH);
				GetDlgItem(IDC_MM_SERVER)->SetWindowText(theApp.serverconnect->GetCurrentServer()->GetListName());
				GetDlgItem(IDC_MM_ID)->SetWindowText(buffer + _T(" [") + buffer2 + _T("]"));
			} else {
				GetDlgItem(IDC_MM_SERVER)->SetWindowText(GetResString(IDS_NOTCONNECTED));
				GetDlgItem(IDC_MM_ID)->SetWindowText(GetResString(IDS_NOTCONNECTED));
			}
		}
*/
		// ==> added - Stulle
		int iTotal;
		//Xman
		// Maella -Accurate measure of bandwidth: eDonkey data + control, network adapter-
		// Retrieve the current datarates
		uint32 downratekb;	uint32 eMuleInOverall;
		uint32 upratekb; uint32 eMuleOutOverall;
		uint32 notUsed;
		theApp.pBandWidthControl->GetDatarates(thePrefs.GetDatarateSamples(),
			downratekb, eMuleInOverall,
			upratekb, eMuleOutOverall,
			notUsed, notUsed);

		uint32 lastuprateoverheadkb=eMuleOutOverall-upratekb;
		uint32 lastdownrateoverheadkb=eMuleInOverall-downratekb;
		uint64 bEmuleIn= theApp.pBandWidthControl->GeteMuleIn();
		uint64 bEmuleOut= theApp.pBandWidthControl->GeteMuleOut();
		//Xman end

		if ( bEmuleIn>0 && bEmuleOut>0 ) 
		{
			// Session
			if (bEmuleIn<bEmuleOut) 
			{
				buffer.Format(_T("%s %.2f : 1"),GetResString(IDS_MM_RATIO),(float)bEmuleOut/bEmuleIn);
			} 
			else 
			{
				buffer.Format(_T("%s 1 : %.2f"),GetResString(IDS_MM_RATIO),(float)bEmuleIn/bEmuleOut);
			}
		}
		else 
		{
			buffer.Format(_T("%s %s"), GetResString(IDS_MM_RATIO), GetResString(IDS_FSTAT_WAITING)); // Localize
		}
		GetDlgItem(IDC_MM_RATIO)->SetWindowText(buffer);

		if (thePrefs.GetMMCompl()) {
			buffer.Format(_T("%s"), GetResString(IDS_DL_TRANSFCOMPL));
			buffer2.Format(_T(": %i"),thePrefs.GetDownSessionCompletedFiles());
		}
		else {
			buffer.Format(_T("%s"), GetResString(IDS_DL_TRANSFCOMPL));
			buffer += (_T(": "));
			buffer2.Format(_T("%i"), theApp.emuledlg->transferwnd->GetDownloadList()->GetCompleteDownloads(0, iTotal));
		}
		GetDlgItem(IDC_MM_COMPLCOUNT)->SetWindowText(buffer + buffer2);
		// <== added - Stulle

		buffer.Format( GetResString( IDS_MM_ACTDL ) , myStats.a[1] );
		GetDlgItem(IDC_MM_DLCOUNT)->SetWindowText(buffer);
		buffer.Format(GetResString(IDS_STATS_ACTUL),theApp.uploadqueue->GetUploadQueueLength());
		GetDlgItem(IDC_MM_ULCOUNT)->SetWindowText(buffer);
		buffer.Format(GetResString(IDS_MM_DATA),CastItoXBytes(bEmuleOut),CastItoXBytes( bEmuleIn ));

		GetDlgItem(IDC_MM_ULDLTRANS)->SetWindowText(buffer);
		// ==> changed - Stulle
		/*
		if( thePrefs.ShowOverhead() )
			_stprintf(buffer3,GetResString(IDS_UPDOWN), (float)upratekb/1024, (float)lastuprateoverheadkb/1024, (float)downratekb/1024, (float)lastdownrateoverheadkb/1024);
		else
			_stprintf(buffer3,GetResString(IDS_UPDOWNSMALL),(float)upratekb/1024, (float)downratekb/1024);
		*/
		if( thePrefs.ShowOverhead() )
			_stprintf(buffer3,GetResString(IDS_MM_UPDOWN), CastItoXBytes(upratekb, false, true), (float)lastuprateoverheadkb/1024, CastItoXBytes(downratekb, false, true), (float)lastdownrateoverheadkb/1024);
		else
			_stprintf(buffer3,GetResString(IDS_MM_DATA),CastItoXBytes(upratekb, false, true), CastItoXBytes(downratekb, false, true));
		// <== changed - Stulle
		GetDlgItem(IDC_MM_ULDL)->SetWindowText(buffer3);
		SetSpeedMeterValues((int)upratekb/1024, (int)downratekb/1024);

		// ==> added - Stulle
		m_btnIncoming.SetIcon(_T("OPENFOLDER"));	
		m_btnIncoming.SetAlign(CButtonST::ST_ALIGN_HORIZ);
		m_btnIncoming.SetLeftAlign(true); 
		m_btnRestore.SetIcon(_T("RESTOREWINDOW"));
		m_btnRestore.SetAlign(CButtonST::ST_ALIGN_HORIZ);
		m_btnRestore.SetLeftAlign(true);
		// <== added - Stulle
	}
	catch(...)
	{
		ASSERT(0);
		return;
	}
}

void CTBHMM::OnMenuButtonClicked()
{
	CRect rectBn;
	CPoint thePoint;
	GetDlgItem(IDC_MM_MENU)->GetWindowRect(&rectBn);
	thePoint = rectBn.BottomRight();
	DoMenu(thePoint);
}

void CTBHMM::DoMenu(CPoint doWhere)
{
	theApp.emuledlg->OnTrayRButtonUp(doWhere);
}

// ==> added - Stulle
void CTBHMM::OnIncomingClicked(){ 
	ShellExecute(NULL, _T("open"), thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR),NULL, NULL, SW_SHOW); 
}

void CTBHMM::OnRestoreClicked(){
	theApp.emuledlg->RestoreWindow();
}
// <==  added - Stulle