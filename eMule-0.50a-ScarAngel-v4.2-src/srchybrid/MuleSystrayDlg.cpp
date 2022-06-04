// MuleSystrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MuleSystrayDlg.h"
#include "emule.h"
#include "preferences.h"
#include "opcodes.h"
#include "otherfunctions.h"
#include "Scheduler.h" // Don't reset Connection Settings for Webserver/CML/MM [Stulle] - Stulle

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

//Xman
/*
CMuleSystrayDlg::CMuleSystrayDlg(CWnd* pParent, CPoint pt, int iMaxUp, int iMaxDown, int iCurUp, int iCurDown)
*/
CMuleSystrayDlg::CMuleSystrayDlg(CWnd* pParent, CPoint pt, float iMaxUp, float iMaxDown, float iCurUp, float iCurDown)
//Xman end
	: CDialog(CMuleSystrayDlg::IDD, pParent)
{
	if(iCurDown == UNLIMITED)
		iCurDown = 0;
	//Xman
	/*
	if(iCurUp == UNLIMITED)
		iCurUp = 0;
	*/
	//Xman end

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
	//Xman
	/*
	DDX_Control(pDX, IDC_DOWNTXT, m_DownSpeedInput);
	DDX_Control(pDX, IDC_UPTXT, m_UpSpeedInput);
	DDX_Text(pDX, IDC_DOWNTXT, m_nDownSpeedTxt);
	DDX_Text(pDX, IDC_UPTXT, m_nUpSpeedTxt);
	*/
	DDX_Control(pDX, IDC_DOWNTXT, m_DownSpeedInput);
	DDX_Control(pDX, IDC_UPTXT, m_UpSpeedInput);
	//Xman end
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

	//zz_fly :: make font not bold for chinese :: start
	if(p)
	{
		p->GetFont()->GetLogFont(&lfStaticFont);
		bValidFont = true;
	}

	if(bValidFont && (thePrefs.m_wLanguageID != 2052) && (thePrefs.m_wLanguageID != 1028))
		lfStaticFont.lfWeight += 200;			// make it bold
	//zz_fly :: make font not bold for chinese :: end

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
			//zz_fly :: make font not bold for chinese :: start 
			/*
			LOGFONT lfFont = lfStaticFont;
			lfFont.lfWeight += 200;			// make it bold
			m_ctrlRestore.m_cfFont.CreateFontIndirect(&lfFont);
			*/
			m_ctrlRestore.m_cfFont.CreateFontIndirect(&lfStaticFont);
			//zz_fly :: make font not bold for chinese :: end 
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

	//Xman
	/*
	m_ctrlDownSpeedSld.SetRange(0,m_iMaxDown);
	m_ctrlDownSpeedSld.SetPos(m_nDownSpeedTxt);

	m_ctrlUpSpeedSld.SetRange(0,m_iMaxUp);
	m_ctrlUpSpeedSld.SetPos(m_nUpSpeedTxt);

	m_DownSpeedInput.EnableWindow(m_nDownSpeedTxt >0);
	m_UpSpeedInput.EnableWindow(m_nUpSpeedTxt >0);
	*/
	CString strBuffer;
	strBuffer.Format(_T("%.1f"), m_nDownSpeedTxt);
	GetDlgItem(IDC_DOWNTXT)->SetWindowText(strBuffer);
	strBuffer.Format(_T("%.1f"), m_nUpSpeedTxt);
	GetDlgItem(IDC_UPTXT)->SetWindowText(strBuffer);

	m_ctrlDownSpeedSld.SetRange(0,int(m_iMaxDown*10.0f));
	m_ctrlDownSpeedSld.SetPos(int(m_nDownSpeedTxt*10.0f));

	m_ctrlUpSpeedSld.SetRange(1,int(m_iMaxUp*10.0f));
	m_ctrlUpSpeedSld.SetPos(int(m_nUpSpeedTxt*10.0f));

	m_DownSpeedInput.EnableWindow(m_nDownSpeedTxt > 0.0f);
	//Xman end

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
	// ==> Show ModID in systray dialog sidebar [Stulle] - Stulle
	/*
	m_ctrlSidebar.SetWindowText(_T("eMule ") + theApp.m_strCurVersionLong);
	*/
	m_ctrlSidebar.SetWindowText(_T("eMule ") + theApp.m_strCurVersionLong + _T(" [") + theApp.m_strModLongVersion + _T("]"));
	// <== Show ModID in systray dialog sidebar [Stulle] - Stulle
	
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

void CMuleSystrayDlg::OnChangeDowntxt() 
{
	UpdateData();
	//Xman
	CString strBuffer;
	if(GetDlgItem(IDC_DOWNTXT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_DOWNTXT)->GetWindowText(strBuffer);
		m_nDownSpeedTxt = (float)_tstof(strBuffer);
		// fix non-numeric input
		/*
		if(strBuffer.GetAt(0) < '0' || strBuffer.GetAt(0) > '9')
		{
			strBuffer.Format(_T("%.1f"),0.0f);
			GetDlgItem(IDC_DOWNTXT)->SetWindowText(strBuffer);
		}
		*/
		// auto complete to float (removed)
		/*
		else if(strBuffer.FindOneOf(_T(".,")) == -1)
		{
			strBuffer.Format(_T("%.1f"),m_nDownSpeedTxt);
			GetDlgItem(IDC_DOWNTXT)->SetWindowText(strBuffer);
		}
		*/
	}
	float m_nDownSpeedTxtOld = m_nDownSpeedTxt;
	//Xman end

	if(thePrefs.GetMaxGraphDownloadRate() == UNLIMITED)	//Cax2 - shouldn't be anymore...
	{
		if(m_nDownSpeedTxt > 64)		//Cax2 - why 64 ???
			m_nDownSpeedTxt = 64;
	} else {
		if(m_nDownSpeedTxt > thePrefs.GetMaxGraphDownloadRate())
			m_nDownSpeedTxt = thePrefs.GetMaxGraphDownloadRate();
	}

	//Xman
	/*
	m_ctrlDownSpeedSld.SetPos(m_nDownSpeedTxt);
	
	if(m_nDownSpeedTxt < 1){
		m_nDownSpeedTxt = 0;
		m_DownSpeedInput.EnableWindow(false);
	}
	*/
	m_ctrlDownSpeedSld.SetPos(int(m_nDownSpeedTxt*10.0f));
	
	if(m_nDownSpeedTxt < 0.1f){
		m_nDownSpeedTxt = 0.0f;
		m_DownSpeedInput.EnableWindow(false);
	}
	if(m_nDownSpeedTxt != m_nDownSpeedTxtOld)
	{
		strBuffer.Format(_T("%.1f"),m_nDownSpeedTxt);
		GetDlgItem(IDC_DOWNTXT)->SetWindowText(strBuffer);
	}
	//Xman end

	thePrefs.SetMaxDownload((m_nDownSpeedTxt == 0) ? UNLIMITED : m_nDownSpeedTxt);
	theApp.scheduler->SaveOriginals(); // Don't reset Connection Settings for Webserver/CML/MM [Stulle] - Stulle

	UpdateData(FALSE);
}

void CMuleSystrayDlg::OnChangeUptxt() 
{
	UpdateData();
	//Xman
	/*
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
	*/
	if(GetDlgItem(IDC_UPTXT)->GetWindowTextLength())
	{
		CString strBuffer;
		GetDlgItem(IDC_UPTXT)->GetWindowText(strBuffer);
		m_nUpSpeedTxt = (float)_tstof(strBuffer);
		// auto complete to float (removed)
		/*
		if(strBuffer.FindOneOf(_T(".,")) == -1)
		{
			strBuffer.Format(_T("%.1f"),m_nUpSpeedTxt);
			GetDlgItem(IDC_DOWNTXT)->SetWindowText(strBuffer);
		}
		*/
	}

	if(m_nUpSpeedTxt > thePrefs.GetMaxGraphUploadRate())
	{
		m_nUpSpeedTxt = thePrefs.GetMaxGraphUploadRate();
		CString strBuffer;
		strBuffer.Format(_T("%.1f"), m_nUpSpeedTxt);
		GetDlgItem(IDC_UPTXT)->SetWindowText(strBuffer);
	}
	if(m_nUpSpeedTxt < 0.1f)
	{
		m_nUpSpeedTxt = 0.1f;
		CString strBuffer;
		strBuffer.Format(_T("%.1f"), m_nUpSpeedTxt);
		GetDlgItem(IDC_UPTXT)->SetWindowText(strBuffer);
	}

	m_ctrlUpSpeedSld.SetPos(int(m_nUpSpeedTxt*10.0f));

	thePrefs.SetMaxUpload(m_nUpSpeedTxt);
	theApp.scheduler->SaveOriginals(); // Don't reset Connection Settings for Webserver/CML/MM [Stulle] - Stulle
	//Xman end

	UpdateData(FALSE);
}

void CMuleSystrayDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	if(pScrollBar == (CScrollBar*)&m_ctrlDownSpeedSld)
	{
		//Xman
		/*
		m_nDownSpeedTxt = m_ctrlDownSpeedSld.GetPos();
		if(m_nDownSpeedTxt < 1){
			m_nDownSpeedTxt = 0;
		*/
		m_nDownSpeedTxt = m_ctrlDownSpeedSld.GetPos()/10.0f;
		if(m_nDownSpeedTxt < 0.1f){
			m_nDownSpeedTxt = 0.0f;
		//Xman end
			m_DownSpeedInput.EnableWindow(false);
		}
		else{
			m_DownSpeedInput.EnableWindow(true);
		}
		//Xman
		CString strBuffer;
		strBuffer.Format(_T("%.1f"), m_nDownSpeedTxt);
		GetDlgItem(IDC_DOWNTXT)->SetWindowText(strBuffer);
		//Xman end
		UpdateData(FALSE);
		thePrefs.SetMaxDownload((m_nDownSpeedTxt == 0) ? UNLIMITED : m_nDownSpeedTxt);
	}
	else if(pScrollBar == (CScrollBar*)&m_ctrlUpSpeedSld)
	{
		//Xman
		/*
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
		*/
		m_nUpSpeedTxt = m_ctrlUpSpeedSld.GetPos()/10.0f;
		if(m_nUpSpeedTxt < 0.1f)
			m_nUpSpeedTxt = 0.1f;
		CString strBuffer;
		strBuffer.Format(_T("%.1f"), m_nUpSpeedTxt);
		GetDlgItem(IDC_UPTXT)->SetWindowText(strBuffer);
		UpdateData(FALSE);
		thePrefs.SetMaxUpload(m_nUpSpeedTxt);
		//Xman end
	}
	theApp.scheduler->SaveOriginals(); // Don't reset Connection Settings for Webserver/CML/MM [Stulle] - Stulle
	
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
	if(HIWORD(wParam) == BN_CLICKED)
	{	
		ReleaseCapture();
		m_nExitCode = LOWORD(wParam);
		EndDialog(m_nExitCode);
		m_bClosingDown = true;
	}

	return CDialog::OnCommand(wParam, lParam);
}
