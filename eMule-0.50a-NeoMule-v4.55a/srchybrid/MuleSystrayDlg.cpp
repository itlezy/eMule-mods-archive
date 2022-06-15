// MuleSystrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MuleSystrayDlg.h"
#include "emule.h"
#include "preferences.h"
#include "opcodes.h"
#include "otherfunctions.h"
#include "Neo/NeoPreferences.h" // NEO: NCFG - [NeoConfiguration] <-- Xanatos --

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


//Cax2 - new class without context menu
BEGIN_MESSAGE_MAP(CInputBox, CEdit)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CInputBox::OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/)
{
	//Cax2 - nothing to see here!
}

/////////////////////////////////////////////////////////////////////////////
// CMuleSystrayDlg dialog

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
bool CMuleSystrayDlg::USSTab;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

CMuleSystrayDlg::CMuleSystrayDlg(CWnd* pParent, CPoint pt, int iMaxUp, int iMaxDown, int iCurUp, int iCurDown)
	: CDialog(CMuleSystrayDlg::IDD, pParent)
{
	if(iCurDown == UNLIMITED)
		iCurDown = 0;
	if(iCurUp == UNLIMITED)
		iCurUp = 0;

	//{{AFX_DATA_INIT(CMuleSystrayDlg)
	m_nDownSpeedTxt = iMaxDown < iCurDown ? iMaxDown : iCurDown;
	m_nUpSpeedTxt = iMaxUp < iCurUp ? iMaxUp : iCurUp;
	//}}AFX_DATA_INIT

	m_iMaxUp = iMaxUp;
	m_iMaxDown = iMaxDown;
	m_ptInitialPosition = pt;

	m_hUpArrow = NULL;
	m_hDownArrow = NULL;

	m_nExitCode = 0;
	m_bClosingDown = false;
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	USSTab = true;
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
}

CMuleSystrayDlg::~CMuleSystrayDlg()
{
	if(m_hUpArrow)
		DestroyIcon(m_hUpArrow);
	if(m_hDownArrow)
		DestroyIcon(m_hDownArrow);
}

void CMuleSystrayDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMuleSystrayDlg)
	DDX_Control(pDX, IDC_TRAYUP, m_ctrlUpArrow);
	DDX_Control(pDX, IDC_TRAYDOWN, m_ctrlDownArrow);
	DDX_Control(pDX, IDC_SIDEBAR, m_ctrlSidebar);
	DDX_Control(pDX, IDC_UPSLD, m_ctrlUpSpeedSld);
	DDX_Control(pDX, IDC_DOWNSLD, m_ctrlDownSpeedSld);
	DDX_Control(pDX, IDC_DOWNTXT, m_DownSpeedInput);
	DDX_Control(pDX, IDC_UPTXT, m_UpSpeedInput);
	DDX_Text(pDX, IDC_DOWNTXT, m_nDownSpeedTxt);
	DDX_Text(pDX, IDC_UPTXT, m_nUpSpeedTxt);
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	DDX_Control(pDX, IDC_MIN_SLD, m_MinSlider);
	DDX_Control(pDX, IDC_MAX_SLD, m_MaxSlider);
	DDX_Control(pDX, IDC_MAX_PING_SLD, m_MaxPingSlider);
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMuleSystrayDlg, CDialog)
	//{{AFX_MSG_MAP(CMuleSystrayDlg)
	ON_WM_MOUSEMOVE()
	ON_EN_CHANGE(IDC_DOWNTXT, OnChangeDowntxt)
	ON_EN_CHANGE(IDC_UPTXT, OnChangeUptxt)
	ON_WM_HSCROLL()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_KILLFOCUS()
	ON_WM_SHOWWINDOW()
	ON_WM_CAPTURECHANGED()
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	ON_EN_CHANGE(IDC_MIN_TXT, OnChangeMintxt)
	ON_EN_CHANGE(IDC_MAX_TXT, OnChangeMaxtxt)
	ON_EN_CHANGE(IDC_MAX_PING_TXT, OnChangeMaxPingtxt)
	ON_BN_CLICKED(IDC_SS_ON, OnChangeSSOn)
	ON_BN_CLICKED(IDC_NAFC_ON, OnChangeNAFCOn)
	ON_BN_CLICKED(IDC_CHANGE_SS, OnBnClickedChangeSs)
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CMuleSystrayDlg message handlers

void CMuleSystrayDlg::OnMouseMove(UINT nFlags, CPoint point)
{	
	CWnd *pWnd = ChildWindowFromPoint(point, CWP_SKIPINVISIBLE|CWP_SKIPDISABLED);
	if(pWnd)
	{
		if(pWnd == this || pWnd == &m_ctrlSidebar)			
			SetCapture();			// me, myself and i
		else						 
			ReleaseCapture();		// sweet child of mine
	}
	else
		SetCapture();				// i'm on the outside, i'm looking in ...

	CDialog::OnMouseMove(nFlags, point);
}

BOOL CMuleSystrayDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	m_bClosingDown = false;

	CRect r;
	CWnd *p;

	m_hUpArrow = theApp.LoadIcon(_T("UPLOAD"));
	m_hDownArrow = theApp.LoadIcon(_T("DOWNLOAD"));
	m_ctrlUpArrow.SetIcon(m_hUpArrow); 
	m_ctrlDownArrow.SetIcon(m_hDownArrow); 
    		
	bool	bValidFont = false;
	LOGFONT lfStaticFont = {0};

	p = GetDlgItem(IDC_SPEED);
	if(p)
	{
		p->GetFont()->GetLogFont(&lfStaticFont);
		bValidFont = true;
	}

	p = GetDlgItem(IDC_SPEED);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlSpeed.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_SPEED);
		m_ctrlSpeed.m_nBtnID = IDC_SPEED;
		//p->GetWindowText(m_ctrlSpeed.m_strText);
		m_ctrlSpeed.m_strText = GetResString(IDS_TRAYDLG_SPEED);
		m_ctrlSpeed.m_strText.Remove(_T('&'));

		m_ctrlSpeed.m_bUseIcon = true;
		m_ctrlSpeed.m_sIcon.cx = 16;
		m_ctrlSpeed.m_sIcon.cy = 16;
		m_ctrlSpeed.m_hIcon = theApp.LoadIcon(_T("SPEED"), m_ctrlSpeed.m_sIcon.cx, m_ctrlSpeed.m_sIcon.cy);
		m_ctrlSpeed.m_bParentCapture = true;
		if(bValidFont)
		{	
			LOGFONT lfFont = lfStaticFont;
			lfFont.lfWeight += 200;			// make it bold
			m_ctrlSpeed.m_cfFont.CreateFontIndirect(&lfFont);
		}
		
		m_ctrlSpeed.m_bNoHover = true;
	}

	p = GetDlgItem(IDC_TOMAX);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlAllToMax.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_TOMAX);
		m_ctrlAllToMax.m_nBtnID = IDC_TOMAX;
		//p->GetWindowText(m_ctrlAllToMax.m_strText);
		m_ctrlAllToMax.m_strText = GetResString(IDS_PW_UA);
		m_ctrlAllToMax.m_strText.Remove(_T('&'));

		m_ctrlAllToMax.m_bUseIcon = true;
		m_ctrlAllToMax.m_sIcon.cx = 16;
		m_ctrlAllToMax.m_sIcon.cy = 16;
		m_ctrlAllToMax.m_hIcon = theApp.LoadIcon(_T("SPEEDMAX"), m_ctrlAllToMax.m_sIcon.cx, m_ctrlAllToMax.m_sIcon.cy);
		m_ctrlAllToMax.m_bParentCapture = true;
		if(bValidFont)
			m_ctrlAllToMax.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_TOMIN);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlAllToMin.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_TOMIN);
		m_ctrlAllToMin.m_nBtnID = IDC_TOMIN;
		//p->GetWindowText(m_ctrlAllToMin.m_strText);
		m_ctrlAllToMin.m_strText = GetResString(IDS_PW_PA);
		m_ctrlAllToMin.m_strText.Remove(_T('&'));

		m_ctrlAllToMin.m_bUseIcon = true;
		m_ctrlAllToMin.m_sIcon.cx = 16;
		m_ctrlAllToMin.m_sIcon.cy = 16;
		m_ctrlAllToMin.m_hIcon = theApp.LoadIcon(_T("SPEEDMIN"), m_ctrlAllToMin.m_sIcon.cx, m_ctrlAllToMin.m_sIcon.cy);
		m_ctrlAllToMin.m_bParentCapture = true;
		if(bValidFont)
			m_ctrlAllToMin.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	p = GetDlgItem(IDC_SS_SPEED);
	if(p)
	{
		p->GetFont()->GetLogFont(&lfStaticFont);
		bValidFont = true;
	}

	p = GetDlgItem(IDC_SS_SPEED);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlSSSpeed.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_SS_SPEED);
		m_ctrlSSSpeed.m_nBtnID = IDC_SS_SPEED;
		m_ctrlSSSpeed.m_strText = (USSTab ? GetResString(IDS_X_TRAYDLG_USS_SPEED) : GetResString(IDS_X_TRAYDLG_DSS_SPEED));
		m_ctrlSSSpeed.m_bUseIcon = true;
		m_ctrlSSSpeed.m_sIcon.cx = 16;
		m_ctrlSSSpeed.m_sIcon.cy = 16;
		m_ctrlSSSpeed.m_hIcon = theApp.LoadIcon(_T("BANDWIDTHCONTROL"), m_ctrlSSSpeed.m_sIcon.cx, m_ctrlSSSpeed.m_sIcon.cy);
		m_ctrlSSSpeed.m_bParentCapture = true;
		if(bValidFont)
		{	
			LOGFONT lfFont = lfStaticFont;
			lfFont.lfWeight += 200;			// make it bold
			m_ctrlSSSpeed.m_cfFont.CreateFontIndirect(&lfFont);
		}
		
		m_ctrlSSSpeed.m_bNoHover = true;
	}
	if((p = GetDlgItem(IDC_CHANGE_SS)) != NULL)
		p->SetWindowText(USSTab ? _T("--->") : _T("<---") );
	InitSSDialog();
#endif // NEO_BC // NEO: NBC END <-- Xanatos --

	p = GetDlgItem(IDC_RESTORE);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlRestore.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_RESTORE);
		m_ctrlRestore.m_nBtnID = IDC_RESTORE;
		//p->GetWindowText(m_ctrlRestore.m_strText);
		m_ctrlRestore.m_strText = GetResString(IDS_MAIN_POPUP_RESTORE);
		m_ctrlRestore.m_strText.Remove(_T('&'));

		m_ctrlRestore.m_bUseIcon = true;
		m_ctrlRestore.m_sIcon.cx = 16;
		m_ctrlRestore.m_sIcon.cy = 16;
		m_ctrlRestore.m_hIcon = theApp.LoadIcon(_T("RESTOREWINDOW"), m_ctrlRestore.m_sIcon.cx, m_ctrlRestore.m_sIcon.cy);
		m_ctrlRestore.m_bParentCapture = true;
		if(bValidFont)
		{	
			LOGFONT lfFont = lfStaticFont;
			lfFont.lfWeight += 200;			// make it bold
			m_ctrlRestore.m_cfFont.CreateFontIndirect(&lfFont);
		}	
	}
	
	p = GetDlgItem(IDC_CONNECT);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlConnect.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_CONNECT);
		m_ctrlConnect.m_nBtnID = IDC_CONNECT;
		//p->GetWindowText(m_ctrlConnect.m_strText);
		m_ctrlConnect.m_strText = GetResString(IDS_MAIN_BTN_CONNECT);
		m_ctrlConnect.m_strText.Remove(_T('&'));

		m_ctrlConnect.m_bUseIcon = true;
		m_ctrlConnect.m_sIcon.cx = 16;
		m_ctrlConnect.m_sIcon.cy = 16;
		m_ctrlConnect.m_hIcon = theApp.LoadIcon(_T("CONNECT"), m_ctrlConnect.m_sIcon.cx, m_ctrlConnect.m_sIcon.cy);
		m_ctrlConnect.m_bParentCapture = true;
		if(bValidFont)
			m_ctrlConnect.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_DISCONNECT);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlDisconnect.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_DISCONNECT);
		m_ctrlDisconnect.m_nBtnID = IDC_DISCONNECT;
		//p->GetWindowText(m_ctrlDisconnect.m_strText);
		m_ctrlDisconnect.m_strText = GetResString(IDS_MAIN_BTN_DISCONNECT);
		m_ctrlDisconnect.m_strText.Remove(_T('&'));

		m_ctrlDisconnect.m_bUseIcon = true;
		m_ctrlDisconnect.m_sIcon.cx = 16;
		m_ctrlDisconnect.m_sIcon.cy = 16;
		m_ctrlDisconnect.m_hIcon = theApp.LoadIcon(_T("DISCONNECT"), m_ctrlDisconnect.m_sIcon.cx, m_ctrlDisconnect.m_sIcon.cy);
		m_ctrlDisconnect.m_bParentCapture = true;
		if(bValidFont)
			m_ctrlDisconnect.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_PREFERENCES);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlPreferences.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_PREFERENCES);
		m_ctrlPreferences.m_nBtnID = IDC_PREFERENCES;
		//p->GetWindowText(m_ctrlPreferences.m_strText);
		m_ctrlPreferences.m_strText = GetResString(IDS_EM_PREFS);
		m_ctrlPreferences.m_strText.Remove(_T('&'));

		m_ctrlPreferences.m_bUseIcon = true;
		m_ctrlPreferences.m_sIcon.cx = 16;
		m_ctrlPreferences.m_sIcon.cy = 16;
		m_ctrlPreferences.m_hIcon = theApp.LoadIcon(_T("Preferences"), m_ctrlPreferences.m_sIcon.cx, m_ctrlPreferences.m_sIcon.cy);
		m_ctrlPreferences.m_bParentCapture = true;
		if(bValidFont)
			m_ctrlPreferences.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_TRAY_EXIT);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlExit.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_EXIT);
		m_ctrlExit.m_nBtnID = IDC_EXIT;
		//p->GetWindowText(m_ctrlExit.m_strText);
		m_ctrlExit.m_strText = GetResString(IDS_EXIT);
		m_ctrlExit.m_strText.Remove(_T('&'));

		m_ctrlExit.m_bUseIcon = true;
		m_ctrlExit.m_sIcon.cx = 16;
		m_ctrlExit.m_sIcon.cy = 16;
		m_ctrlExit.m_hIcon = theApp.LoadIcon(_T("EXIT"), m_ctrlExit.m_sIcon.cx, m_ctrlExit.m_sIcon.cy);
		m_ctrlExit.m_bParentCapture = true;
		if(bValidFont)
			m_ctrlExit.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	if((p = GetDlgItem(IDC_DOWNLBL)) != NULL)
		p->SetWindowText(GetResString(IDS_PW_CON_DOWNLBL));
	if((p = GetDlgItem(IDC_UPLBL)) != NULL)
		p->SetWindowText(GetResString(IDS_PW_CON_UPLBL));
	if((p = GetDlgItem(IDC_DOWNKB)) != NULL)
		p->SetWindowText(GetResString(IDS_KBYTESPERSEC));
	if((p = GetDlgItem(IDC_UPKB)) != NULL)
		p->SetWindowText(GetResString(IDS_KBYTESPERSEC));

	m_ctrlDownSpeedSld.SetRange(0,m_iMaxDown);
	m_ctrlDownSpeedSld.SetPos(m_nDownSpeedTxt);

	m_ctrlUpSpeedSld.SetRange(0,m_iMaxUp);
	m_ctrlUpSpeedSld.SetPos(m_nUpSpeedTxt);

	m_DownSpeedInput.EnableWindow(m_nDownSpeedTxt >0);
	m_UpSpeedInput.EnableWindow(m_nUpSpeedTxt >0);

	CFont Font;
	Font.CreateFont(-16,0,900,0,700,0,0,0,0,3,2,1,34,_T("Tahoma"));

	UINT winver = thePrefs.GetWindowsVersion();
	if (winver == _WINVER_95_ || winver == _WINVER_NT4_ || g_bLowColorDesktop)
	{
		m_ctrlSidebar.SetColors(GetSysColor(COLOR_CAPTIONTEXT), 
								GetSysColor(COLOR_ACTIVECAPTION), 
								GetSysColor(COLOR_ACTIVECAPTION));
	}
	else
	{
		m_ctrlSidebar.SetColors(GetSysColor(COLOR_CAPTIONTEXT), 
								GetSysColor(COLOR_ACTIVECAPTION), 
								GetSysColor(COLOR_GRADIENTACTIVECAPTION));
	}

	m_ctrlSidebar.SetHorizontal(false);
	m_ctrlSidebar.SetFont(&Font);
	m_ctrlSidebar.SetWindowText(theApp.GetAppTitle()); // NEO: NV - [NeoVersion] <-- Xanatos --
	
	CRect rDesktop;
	CWnd *pDesktopWnd = GetDesktopWindow();
	pDesktopWnd->GetClientRect(rDesktop);
	
	CPoint pt = m_ptInitialPosition;
	pDesktopWnd->ScreenToClient(&pt);
	int xpos, ypos;

	GetWindowRect(r);
	if(m_ptInitialPosition.x + r.Width() < rDesktop.right)
		xpos = pt.x;
	else
		xpos = pt.x - r.Width();
	if(m_ptInitialPosition.y - r.Height() < rDesktop.top)
		ypos = pt.y;
	else
		ypos = pt.y - r.Height();
	
	MoveWindow(xpos, ypos, r.Width(), r.Height());
	SetCapture();
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
void CMuleSystrayDlg::InitSSDialog(bool change){
	CWnd *p;
	if (USSTab){
		m_MinTxt = NeoPrefs.GetMinBCUpload();
		m_MaxTxt = NeoPrefs.GetMaxBCUpload();
		if (NeoPrefs.IsNAFCUpload())
			m_MaxPingTxt = (int)NeoPrefs.GetMaxUpStream();
		else if (NeoPrefs.GetUpMaxPingMethod() == 0)
			m_MaxPingTxt = NeoPrefs.GetBasePingUp();
		else if (NeoPrefs.GetUpMaxPingMethod() == 1)
			m_MaxPingTxt = NeoPrefs.GetPingUpTolerance();
		else if (NeoPrefs.GetUpMaxPingMethod() == 2)
			m_MaxPingTxt = NeoPrefs.GetPingUpProzent();
		CheckDlgButton(IDC_SS_ON,NeoPrefs.IsUSSEnabled());
		CheckDlgButton(IDC_NAFC_ON,NeoPrefs.IsNAFCUpload());
		CString strBuffer;
		p = GetDlgItem(IDC_MIN_TXT);
		if (p){
			strBuffer.Format(_T("%.1f"), NeoPrefs.GetMinBCUpload());
			p->SetWindowText(strBuffer);
		}
		
		p = GetDlgItem(IDC_MAX_TXT);
		if (p){
			if(NeoPrefs.IsAutoMaxUpload()){
				p->SetWindowText(_T("Auto")); 
			}
			else {
				strBuffer.Format(_T("%.1f"), NeoPrefs.GetMaxBCUpload());
				GetDlgItem(IDC_MAX_TXT)->SetWindowText(strBuffer);
			}
		}

		m_MinSlider.SetRange((m_iMaxUp <= 10 ? 10 : 1),(m_iMaxUp <= 10 ? m_iMaxUp*10 : m_iMaxUp),true);
		m_MinSlider.SetPos((int)(m_iMaxUp <= 10 ? m_MinTxt*10 : m_MinTxt));

		m_MaxSlider.SetRange(0,(m_iMaxUp <= 10 ? m_iMaxUp*10 : m_iMaxUp),true);
		m_MaxSlider.SetPos((int)(m_iMaxUp <= 10 ? m_MaxTxt*10 : m_MaxTxt));

		if (NeoPrefs.IsNAFCUpload()){
			m_MaxPingSlider.SetRange((m_iMaxUp <= 10 ? 10 : 1),(m_iMaxUp <= 10 ? m_iMaxUp*10 : m_iMaxUp),true);
			m_MaxPingSlider.SetPos((m_iMaxUp <= 10 ? m_MaxPingTxt*10 : m_MaxPingTxt));
		}else if (NeoPrefs.GetUpMaxPingMethod() == 0){
			m_MaxPingSlider.SetRange(1,250,true);
			m_MaxPingSlider.SetPos(m_MaxPingTxt/10);
		}else if (NeoPrefs.GetUpMaxPingMethod() == 1){
			m_MaxPingSlider.SetRange(1,24,true);
			m_MaxPingSlider.SetPos(m_MaxPingTxt);
		}else if (NeoPrefs.GetUpMaxPingMethod() == 2){
			m_MaxPingSlider.SetRange(1,100,true);
			m_MaxPingSlider.SetPos(m_MaxPingTxt/10);
		}

		if((p = GetDlgItem(IDC_MIN_S)) != NULL)
			p->SetWindowText(GetResString(IDS_X_TRAYDLG_MINUP));
		if((p = GetDlgItem(IDC_MAX_S)) != NULL)
			p->SetWindowText(GetResString(IDS_X_TRAYDLG_MAXUP));

		if((p = GetDlgItem(IDC_MAX_PING_S)) != NULL){
			if (NeoPrefs.IsNAFCUpload()){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_STREAM));
				strBuffer.Format(_T("%d"),m_MaxPingTxt);
				GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T("kb/s"));
			}else if (NeoPrefs.GetUpMaxPingMethod() == 0){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_PING));
				strBuffer.Format(_T("%d"),m_MaxPingTxt);
				GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T("ms"));
			}else if (NeoPrefs.GetUpMaxPingMethod() == 1){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF));
				if (m_MaxPingTxt <= 8)
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_F));
				else if (m_MaxPingTxt <= 16)
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_N));
				else
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_S));
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T(""));
			}else if (NeoPrefs.GetUpMaxPingMethod() == 2){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_PING));
				strBuffer.Format(_T("%d"),m_MaxPingTxt);
				GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T("%%"));
			}
		}
		m_ctrlSSSpeed.m_strText=GetResString(IDS_X_TRAYDLG_USS_SPEED);
		m_ctrlSSSpeed.Invalidate();
	}
	else {
		m_MinTxt = NeoPrefs.GetMinBCDownload();
		m_MaxTxt = NeoPrefs.GetMaxBCDownload();
		if (NeoPrefs.IsNAFCDownload())
			m_MaxPingTxt = (int)NeoPrefs.GetMaxDownStream();
		else if (NeoPrefs.GetDownMaxPingMethod() == 0)
			m_MaxPingTxt = NeoPrefs.GetBasePingDown();
		else if (NeoPrefs.GetDownMaxPingMethod() == 1)
			m_MaxPingTxt = NeoPrefs.GetPingDownTolerance();
		else if (NeoPrefs.GetDownMaxPingMethod() == 2)
			m_MaxPingTxt = NeoPrefs.GetPingDownProzent();
		CheckDlgButton(IDC_SS_ON,NeoPrefs.IsDSSEnabled());
		CheckDlgButton(IDC_NAFC_ON,NeoPrefs.IsNAFCDownload());
		CString strBuffer;

		p = GetDlgItem(IDC_MIN_TXT);
		if (p){
			strBuffer.Format(_T("%.1f"), NeoPrefs.GetMinBCDownload());
			p->SetWindowText(strBuffer);
		}
		
		p = GetDlgItem(IDC_MAX_TXT);
		if (p){
			if(NeoPrefs.IsAutoMaxDownload()){
				p->SetWindowText(_T("Auto")); 
			}
			else {
				strBuffer.Format(_T("%.1f"), NeoPrefs.GetMaxBCDownload());
				GetDlgItem(IDC_MAX_TXT)->SetWindowText(strBuffer);
			}
		}

		m_MinSlider.SetRange((m_iMaxDown <= 10 ? 10 : 1),(m_iMaxDown <= 10 ? m_iMaxDown*10 : m_iMaxDown),true);
		m_MinSlider.SetPos((int)(m_iMaxDown <= 10 ? m_MinTxt*10 : m_MinTxt));

		m_MaxSlider.SetRange(0,(m_iMaxDown <= 10 ? m_iMaxDown*10 : m_iMaxDown),true);
		m_MaxSlider.SetPos((int)(m_iMaxDown <= 10 ? m_MaxTxt*10 : m_MaxTxt));

		if (NeoPrefs.IsNAFCDownload()){
			m_MaxPingSlider.SetRange((m_iMaxDown <= 10 ? 10 : 1),(m_iMaxDown <= 10 ? m_iMaxDown*10 : m_iMaxDown),true);
			m_MaxPingSlider.SetPos((m_iMaxDown <= 10 ? m_MaxPingTxt*10 : m_MaxPingTxt));
		}else if (NeoPrefs.GetDownMaxPingMethod() == 0){
			m_MaxPingSlider.SetRange(1,350,true);
			m_MaxPingSlider.SetPos(m_MaxPingTxt/10);
		}else if (NeoPrefs.GetDownMaxPingMethod() == 1){
			m_MaxPingSlider.SetRange(1,24,true);
			m_MaxPingSlider.SetPos(m_MaxPingTxt);
		}else if (NeoPrefs.GetDownMaxPingMethod() == 2){
			m_MaxPingSlider.SetRange(1,100,true);
			m_MaxPingSlider.SetPos(m_MaxPingTxt/10);
		}

		if((p = GetDlgItem(IDC_MIN_S)) != NULL)
			p->SetWindowText(GetResString(IDS_X_TRAYDLG_MINDOWN));
		if((p = GetDlgItem(IDC_MAX_S)) != NULL)
			p->SetWindowText(GetResString(IDS_X_TRAYDLG_MAXDOWN));

		if((p = GetDlgItem(IDC_MAX_PING_S)) != NULL){
			if (NeoPrefs.IsNAFCDownload()){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_STREAM));
				strBuffer.Format(_T("%d"),m_MaxPingTxt);
				GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T("kb/s"));
			}else if (NeoPrefs.GetDownMaxPingMethod() == 0){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_PING));
				strBuffer.Format(_T("%d"),m_MaxPingTxt);
				GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T("ms"));
			}else if (NeoPrefs.GetDownMaxPingMethod() == 1){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF));
				if (m_MaxPingTxt <= 8)
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_F));
				else if (m_MaxPingTxt <= 16)
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_N));
				else
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_S));
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T(""));
			}else if (NeoPrefs.GetDownMaxPingMethod() == 2){
				p->SetWindowText(GetResString(IDS_X_TRAYDLG_PING));
				strBuffer.Format(_T("%d"),m_MaxPingTxt);
				GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
				GetDlgItem(IDC_MAX_PING_MS)->SetWindowText(_T("%%"));
			}
		}
		m_ctrlSSSpeed.m_strText=GetResString(IDS_X_TRAYDLG_DSS_SPEED);
		m_ctrlSSSpeed.Invalidate();
	}

	if(change)
		return;

	ChangeOn(0);

	if ((p = GetDlgItem(IDC_MIN_KB)) != NULL)
		p->SetWindowText(GetResString(IDS_X_KBYTESEC));
	if ((p = GetDlgItem(IDC_MAX_KB)) != NULL)
		p->SetWindowText(GetResString(IDS_X_KBYTESEC));
}

void CMuleSystrayDlg::ChangeOn(int ctrl) 
{
	BOOL val = FALSE;

	if (USSTab){
		if(ctrl == 0){
			val = NeoPrefs.IsNAFCUpload() || NeoPrefs.IsUSSEnabled();
		}else if(ctrl == 1){
			val = IsDlgButtonChecked(IDC_NAFC_ON);
			if(val)
				CheckDlgButton(IDC_SS_ON,FALSE);
			NeoPrefs.SetUploadControl(val ? 1 : 0);
		}else if(ctrl == 2){
			val = IsDlgButtonChecked(IDC_SS_ON);
			if(val)
				CheckDlgButton(IDC_NAFC_ON,FALSE);
			NeoPrefs.SetUploadControl(val ? 2 : 0);
		}
	}else{
		if(ctrl == 0){
			val = NeoPrefs.IsNAFCDownload() || NeoPrefs.IsDSSEnabled();
		}else if(ctrl == 1){
			val = IsDlgButtonChecked(IDC_NAFC_ON);
			if(val)
				CheckDlgButton(IDC_SS_ON,FALSE);
			NeoPrefs.SetDownloadControl(val ? 1 : 0);
		}else if(ctrl == 2){
			val = IsDlgButtonChecked(IDC_SS_ON);
			if(val)
				CheckDlgButton(IDC_NAFC_ON,FALSE);
			NeoPrefs.SetDownloadControl(val ? 2 : 0);
		}
	}

	if(val){
		InitSSDialog(true);

		GetDlgItem(IDC_MIN_S)->EnableWindow(TRUE);
		GetDlgItem(IDC_MIN_SLD)->EnableWindow(TRUE);
		GetDlgItem(IDC_MIN_TXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_MIN_KB)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_S)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_SLD)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_TXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_KB)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_PING_S)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_PING_SLD)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_PING_TXT)->EnableWindow(TRUE);
		GetDlgItem(IDC_MAX_PING_MS)->EnableWindow(TRUE);
	}
	else{
		GetDlgItem(IDC_MIN_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_MIN_SLD)->EnableWindow(FALSE);
		GetDlgItem(IDC_MIN_TXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_MIN_KB)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_SLD)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_TXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_KB)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_PING_S)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_PING_SLD)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_PING_TXT)->EnableWindow(FALSE);
		GetDlgItem(IDC_MAX_PING_MS)->EnableWindow(FALSE);
	}
}

void CMuleSystrayDlg::OnChangeSSOn()
{
	ChangeOn(2);
}

void CMuleSystrayDlg::OnChangeNAFCOn()
{
	ChangeOn(1);
}

void CMuleSystrayDlg::OnChangeMintxt(){
	CString buffer;
	if(GetDlgItem(IDC_MIN_TXT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MIN_TXT)->GetWindowText(buffer);
		m_MinTxt = (float)_tstof(buffer);
	}

	if (USSTab){
		if(m_MinTxt < 1.0f)
			m_MinTxt = 1.0f;
		m_MinSlider.SetPos((m_iMaxUp <= 10 ? (uint32)(m_MinTxt*10.0f) : (uint32)m_MinTxt));
		NeoPrefs.SetMinBCUpload(m_MinTxt);
	}
	else{
		if(m_MinTxt < 1.0f)
			m_MinTxt = 1.0f;
		m_MinSlider.SetPos((m_iMaxDown <= 10 ? (uint32)(m_MinTxt*10.0f) : (uint32)m_MinTxt));
		NeoPrefs.SetMinBCDownload(m_MinTxt);
	}
}

void CMuleSystrayDlg::OnChangeMaxtxt(){
	CString buffer;
	if(GetDlgItem(IDC_MAX_TXT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_MAX_TXT)->GetWindowText(buffer);
		m_MaxTxt = (float)_tstof(buffer);
	}

	if (USSTab){
		if(m_MaxTxt < 1.0f)
			m_MaxTxt = 0.0f;
		m_MaxSlider.SetPos((int)(m_iMaxUp <= 10 ? m_MaxTxt*10 : m_MaxTxt));
		NeoPrefs.SetMaxBCUpload(m_MaxTxt);
	}
	else {
		if(m_MaxTxt < 1.0f)
			m_MaxTxt = 0.0f;
		m_MaxSlider.SetPos((int)(m_iMaxDown <= 10 ? m_MaxTxt*10 : m_MaxTxt));
		NeoPrefs.SetMaxBCDownload(m_MaxTxt);
	}
}

void CMuleSystrayDlg::OnChangeMaxPingtxt(){
	if (USSTab){
		if (NeoPrefs.IsNAFCUpload()){
			UpdateData();

			if(m_MaxPingTxt < 1)
				m_MaxPingTxt = 1;
			m_MaxPingSlider.SetPos((m_iMaxUp <= 10 ? m_MaxPingTxt*10 : m_MaxPingTxt));
			NeoPrefs.SetMaxUpStream((float)m_MaxPingTxt);
		}else if (NeoPrefs.GetUpMaxPingMethod() == 0){
			UpdateData();

			if(m_MaxPingTxt < 1)
				m_MaxPingTxt = 1;
			if(m_MaxPingTxt > 2500)
				m_MaxPingTxt = 2500;
			m_MaxPingSlider.SetPos((m_MaxPingTxt < 10 ? 1 : m_MaxPingTxt/10));
			NeoPrefs.SetBasePingUp((m_MaxPingTxt < 10 ? 10 : m_MaxPingTxt));

			UpdateData(FALSE);
		}else if (NeoPrefs.GetUpMaxPingMethod() == 2){
			UpdateData();

			if(m_MaxPingTxt < 1)
				m_MaxPingTxt = 1;
			if(m_MaxPingTxt > 1000)
				m_MaxPingTxt = 1000;
			m_MaxPingSlider.SetPos((m_MaxPingTxt < 10 ? 1 : m_MaxPingTxt/10));
			NeoPrefs.SetBasePingUp((m_MaxPingTxt < 10 ? 10 : m_MaxPingTxt));

			UpdateData(FALSE);
		}
	}
	else{
		if (NeoPrefs.IsNAFCDownload()){
			UpdateData();

			if(m_MaxPingTxt < 1)
				m_MaxPingTxt = 1;
			m_MaxPingSlider.SetPos((m_iMaxDown <= 10 ? m_MaxPingTxt*10 : m_MaxPingTxt));
			NeoPrefs.SetMaxDownStream((float)m_MaxPingTxt);
		}else if (NeoPrefs.GetDownMaxPingMethod() == 0){
			UpdateData();

			if(m_MaxPingTxt < 1)
				m_MaxPingTxt = 1;
			if(m_MaxPingTxt > 3500)
				m_MaxPingTxt = 3500;
			m_MaxPingSlider.SetPos((m_MaxPingTxt < 10 ? 1 : m_MaxPingTxt/10));
			NeoPrefs.SetBasePingDown((m_MaxPingTxt < 10 ? 10 : m_MaxPingTxt));

			UpdateData(FALSE);
		}else if (NeoPrefs.GetDownMaxPingMethod() == 2){
			UpdateData();

			if(m_MaxPingTxt < 1)
				m_MaxPingTxt = 1;
			if(m_MaxPingTxt > 1000)
				m_MaxPingTxt = 1000;
			m_MaxPingSlider.SetPos((m_MaxPingTxt < 10 ? 1 : m_MaxPingTxt/10));
			NeoPrefs.SetBasePingDown((m_MaxPingTxt < 10 ? 10 : m_MaxPingTxt));

			UpdateData(FALSE);
		}
	}
}

void CMuleSystrayDlg::OnBnClickedChangeSs()
{
	USSTab=!USSTab;

	CWnd *p;
	if((p = GetDlgItem(IDC_CHANGE_SS)) != NULL)
		p->SetWindowText(USSTab ? _T("--->") : _T("<---") );

	InitSSDialog();
}

#endif // NEO_BC // NEO: NBC END <-- Xanatos --

void CMuleSystrayDlg::OnChangeDowntxt() 
{
	UpdateData();

	if(thePrefs.GetMaxGraphDownloadRate() == UNLIMITED)	//Cax2 - shouldn't be anymore...
	{
		if(m_nDownSpeedTxt > 64)		//Cax2 - why 64 ???
			m_nDownSpeedTxt = 64;
	} else {
		if(m_nDownSpeedTxt > thePrefs.GetMaxGraphDownloadRate())
			m_nDownSpeedTxt = thePrefs.GetMaxGraphDownloadRate();
	}

	m_ctrlDownSpeedSld.SetPos(m_nDownSpeedTxt);
	
	if(m_nDownSpeedTxt < 1){
		m_nDownSpeedTxt = 0;
		m_DownSpeedInput.EnableWindow(false);
	}

	thePrefs.SetMaxDownload((m_nDownSpeedTxt == 0) ? UNLIMITED : m_nDownSpeedTxt);

	UpdateData(FALSE);
}

void CMuleSystrayDlg::OnChangeUptxt() 
{
	UpdateData();
	if(thePrefs.GetMaxGraphUploadRate(true) == UNLIMITED)
	{
		if(m_nUpSpeedTxt > 16)
			m_nUpSpeedTxt = 16;
	} else {
		if(m_nUpSpeedTxt > thePrefs.GetMaxGraphUploadRate(true))
			m_nUpSpeedTxt = thePrefs.GetMaxGraphUploadRate(true);
	}
	m_ctrlUpSpeedSld.SetPos(m_nUpSpeedTxt);
	
	if(m_nUpSpeedTxt < 1){
		m_nUpSpeedTxt = 0;
		m_UpSpeedInput.EnableWindow(false);
	}
	thePrefs.SetMaxUpload((m_nUpSpeedTxt == 0) ? UNLIMITED : m_nUpSpeedTxt);

	UpdateData(FALSE);
}

void CMuleSystrayDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == (CScrollBar*)&m_ctrlDownSpeedSld)
	{
		m_nDownSpeedTxt = m_ctrlDownSpeedSld.GetPos();
		if(m_nDownSpeedTxt < 1){
			m_nDownSpeedTxt = 0;
			m_DownSpeedInput.EnableWindow(false);
		}
		else{
			m_DownSpeedInput.EnableWindow(true);
		}
		UpdateData(FALSE);
		thePrefs.SetMaxDownload((m_nDownSpeedTxt == 0) ? UNLIMITED : m_nDownSpeedTxt);
	}
	else if(pScrollBar == (CScrollBar*)&m_ctrlUpSpeedSld)
	{
		m_nUpSpeedTxt = m_ctrlUpSpeedSld.GetPos();
		if(m_nUpSpeedTxt < 1){
			m_nUpSpeedTxt = 0;
			m_UpSpeedInput.EnableWindow(false);
		}
		else{
			m_UpSpeedInput.EnableWindow(true);
		}
		UpdateData(FALSE);
		thePrefs.SetMaxUpload((m_nUpSpeedTxt == 0) ? UNLIMITED : m_nUpSpeedTxt);
	}
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	else {
		if (USSTab){
			if(pScrollBar == (CScrollBar*)&m_MinSlider){
				m_MinTxt = (float)m_MinSlider.GetPos();
				if (m_iMaxUp <= 10)
					m_MinTxt/=10.0f;
				if (m_MinTxt > m_MaxTxt && m_MaxTxt != 0.0f){
					m_MaxTxt=m_MinTxt;
					m_MaxSlider.SetPos((int)(m_iMaxUp<=10 ? m_MaxTxt*10 : m_MaxTxt));
					NeoPrefs.SetMaxBCUpload(m_MaxTxt);
				}

				CString strBuffer;
				strBuffer.Format(_T("%.1f"), m_MinTxt);
				GetDlgItem(IDC_MIN_TXT)->SetWindowText(strBuffer);
				if (m_MaxTxt != 0.0f)
					strBuffer.Format(_T("%.1f"), m_MaxTxt);
				else
					strBuffer = _T("Auto");
				GetDlgItem(IDC_MAX_TXT)->SetWindowText(strBuffer);
				NeoPrefs.SetMinBCUpload(m_MinTxt);
			}
			else if(pScrollBar == (CScrollBar*)&m_MaxSlider){
				m_MaxTxt = (float)m_MaxSlider.GetPos();
				if (m_iMaxUp <= 10)
					m_MaxTxt/=10.0f;
				if (m_MaxTxt < 1.0f && m_MaxTxt > 0.0f)
					m_MaxTxt = 1.0f;

				CString strBuffer;
				if (m_MaxTxt != 0.0f)
					strBuffer.Format(_T("%.1f"), m_MaxTxt);
				else
					strBuffer = _T("Auto");
				GetDlgItem(IDC_MAX_TXT)->SetWindowText(strBuffer);

				NeoPrefs.SetMaxBCUpload(m_MaxTxt);
			}
			else if(pScrollBar == (CScrollBar*)&m_MaxPingSlider){
				if (NeoPrefs.IsNAFCUpload()){
					m_MaxPingTxt = m_MaxPingSlider.GetPos();
					if (m_iMaxUp <= 10)
						m_MaxPingTxt/=10;
					if (m_MaxPingTxt < 1)
						m_MaxPingTxt = 1;
					CString strBuffer;
					strBuffer.Format(_T("%d"),m_MaxPingTxt);
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
					NeoPrefs.SetMaxUpStream((float)m_MaxPingTxt);
				}else if (NeoPrefs.GetUpMaxPingMethod() == 0){
					m_MaxPingTxt = m_MaxPingSlider.GetPos()*10;
					CString strBuffer;
					strBuffer.Format(_T("%d"),m_MaxPingTxt);
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
					NeoPrefs.SetBasePingUp(m_MaxPingTxt);
				}else if (NeoPrefs.GetUpMaxPingMethod() == 1){
					m_MaxPingTxt = m_MaxPingSlider.GetPos();
					NeoPrefs.SetPingUpTolerance(m_MaxPingTxt);
					if (m_MaxPingTxt <= 8)
						GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_F));
					else if (m_MaxPingTxt <= 16)
						GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_N));
					else
						GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_S));
				}else if (NeoPrefs.GetUpMaxPingMethod() == 2){
					m_MaxPingTxt = m_MaxPingSlider.GetPos()*10;
					CString strBuffer;
					strBuffer.Format(_T("%d"),m_MaxPingTxt);
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
					NeoPrefs.SetPingUpProzent(m_MaxPingTxt);
				}
			}
		}
		else {
			if(pScrollBar == (CScrollBar*)&m_MinSlider){
				m_MinTxt = (float)m_MinSlider.GetPos();
				if (m_iMaxDown <= 10)
					m_MinTxt/=10.0f;
				if (m_MinTxt > m_MaxTxt && m_MaxTxt != 0.0f){
					m_MaxTxt=m_MinTxt;
					m_MaxSlider.SetPos((int)(m_iMaxDown<=10 ? m_MaxTxt*10 : m_MaxTxt));
					NeoPrefs.SetMaxBCDownload(m_MaxTxt);
				}
				CString strBuffer;
				strBuffer.Format(_T("%.1f"), m_MinTxt);
				GetDlgItem(IDC_MIN_TXT)->SetWindowText(strBuffer);
				if (m_MaxTxt != 0.0f)
					strBuffer.Format(_T("%.1f"), m_MaxTxt);
				else
					strBuffer = _T("Auto");
				GetDlgItem(IDC_MAX_TXT)->SetWindowText(strBuffer);
				NeoPrefs.SetMinBCDownload(m_MinTxt);
			}
			else if(pScrollBar == (CScrollBar*)&m_MaxSlider){
				m_MaxTxt = (float)m_MaxSlider.GetPos();
				if (m_iMaxDown <= 10)
					m_MaxTxt/=10.0f;
				if (m_MaxTxt < 1.0f && m_MaxTxt > 0.0f)
					m_MaxTxt = 1.0f;

				CString strBuffer;
				if (m_MaxTxt != 0.0f)
					strBuffer.Format(_T("%.1f"), m_MaxTxt);
				else
					strBuffer = _T("Auto");
				GetDlgItem(IDC_MAX_TXT)->SetWindowText(strBuffer);

				NeoPrefs.SetMaxBCDownload(m_MaxTxt);
			}
			else if(pScrollBar == (CScrollBar*)&m_MaxPingSlider){
				if (NeoPrefs.IsNAFCDownload()){
					m_MaxPingTxt = m_MaxPingSlider.GetPos();
					if (m_iMaxDown <= 10)
						m_MaxPingTxt/=10;
					if (m_MaxPingTxt < 1)
						m_MaxPingTxt = 1;
					CString strBuffer;
					strBuffer.Format(_T("%d"),m_MaxPingTxt);
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
					NeoPrefs.SetMaxDownStream((float)m_MaxPingTxt);
				}else if (NeoPrefs.GetDownMaxPingMethod() == 0){
					m_MaxPingTxt = m_MaxPingSlider.GetPos()*10;
					CString strBuffer;
					strBuffer.Format(_T("%d"),m_MaxPingTxt);
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
					NeoPrefs.SetBasePingDown(m_MaxPingTxt);
				}else if (NeoPrefs.GetDownMaxPingMethod() == 1){
					m_MaxPingTxt = m_MaxPingSlider.GetPos();
					NeoPrefs.SetPingDownTolerance(m_MaxPingTxt);
					if (m_MaxPingTxt <= 8)
						GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_F));
					else if (m_MaxPingTxt <= 16)
						GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_N));
					else
						GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(GetResString(IDS_X_TRAYDLG_SURF_S));
				}else if (NeoPrefs.GetDownMaxPingMethod() == 2){
					m_MaxPingTxt = m_MaxPingSlider.GetPos()*10;
					CString strBuffer;
					strBuffer.Format(_T("%d"),m_MaxPingTxt);
					GetDlgItem(IDC_MAX_PING_TXT)->SetWindowText(strBuffer);
					NeoPrefs.SetPingDownProzent(m_MaxPingTxt);
				}
			}
		}
	}
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	
	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMuleSystrayDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
	ReleaseCapture();
	EndDialog(m_nExitCode);
	m_bClosingDown = true;

	CDialog::OnLButtonUp(nFlags, point);
}

//bond006: systray menu gets stuck (bugfix)
void CMuleSystrayDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	CRect systrayRect;
	GetClientRect(&systrayRect);

	if(point.x<=systrayRect.left || point.x>=systrayRect.right || point.y<=systrayRect.top || point.y>=systrayRect.bottom)
	{
		ReleaseCapture();
		EndDialog(m_nExitCode);
		m_bClosingDown = true;
	}

	CDialog::OnRButtonDown(nFlags,point);
}

void CMuleSystrayDlg::OnKillFocus(CWnd* pNewWnd) 
{
	CDialog::OnKillFocus(pNewWnd);
	
	if(!m_bClosingDown)
	{
		ReleaseCapture();
		EndDialog(m_nExitCode);
		m_bClosingDown = true;
	}
}

void CMuleSystrayDlg::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	if(!bShow && !m_bClosingDown)
	{
		ReleaseCapture();
		EndDialog(m_nExitCode);
		m_bClosingDown = true;
	}

	CDialog::OnShowWindow(bShow, nStatus);
}

void CMuleSystrayDlg::OnCaptureChanged(CWnd *pWnd) 
{
	if(pWnd && pWnd != this && !IsChild(pWnd))
	{
		EndDialog(m_nExitCode);
		m_bClosingDown = true;
	}
	CDialog::OnCaptureChanged(pWnd);
}

BOOL CMuleSystrayDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
	if(HIWORD(wParam) == BN_CLICKED && LOWORD(wParam) != IDC_SS_ON && LOWORD(wParam) != IDC_NAFC_ON && LOWORD(wParam)!= IDC_CHANGE_SS)
#else
	if(HIWORD(wParam) == BN_CLICKED)
#endif // NEO_BC // NEO: NBC END <-- Xanatos --
	{	
		ReleaseCapture();
		m_nExitCode = LOWORD(wParam);
		EndDialog(m_nExitCode);
		m_bClosingDown = true;
	}

	return CDialog::OnCommand(wParam, lParam);
}
