// MuleSystrayDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MuleSystrayDlg.h"
#include "emule.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//	Thresholds to disable franctional speeds for slider controls
#define UP_FRAC_THRESHOLD	100
#define DN_FRAC_THRESHOLD	120

//Cax2 - new class without context menu
BEGIN_MESSAGE_MAP(CInputBox, CEdit)
    ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

void CInputBox::OnContextMenu(CWnd* pWnd, CPoint point)
{
	NOPRM(pWnd); NOPRM(point);
}

/////////////////////////////////////////////////////////////////////////////
// CMuleSystrayDlg dialog
CMuleSystrayDlg::CMuleSystrayDlg(CWnd *pParent, CPoint pt) : CDialog(CMuleSystrayDlg::IDD, pParent)
{
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
	DDX_Control(pDX, IDC_DOWNTXT, m_DownSpeedInput);
	DDX_Control(pDX, IDC_UPTXT, m_UpSpeedInput);
	DDX_Text(pDX, IDC_DOWNTXT, m_strDownSpeedTxt);
	DDX_Text(pDX, IDC_UPTXT, m_strUpSpeedTxt);
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
			SetCapture();
		else
			ReleaseCapture();
	}
	else
		SetCapture();

	CDialog::OnMouseMove(nFlags, point);
}

BOOL CMuleSystrayDlg::OnInitDialog()
{
	static const int s_aiResTbl[][2] =
	{
		{ IDC_DOWNLBL, IDS_DOWNLOAD_NOUN },
		{ IDC_UPLBL, IDS_UPLOAD_NOUN },
		{ IDC_DOWNKB, IDS_KBYTESEC },
		{ IDC_UPKB, IDS_KBYTESEC }
	};

	CDialog::OnInitDialog();
	m_bClosingDown = false;

	CRect		r;
	CWnd		*p;
	HINSTANCE	hInst = AfxGetInstanceHandle();

	m_hUpArrow = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_UP), IMAGE_ICON, 16, 16, 0);
	m_hDownArrow = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_DOWN), IMAGE_ICON, 16, 16, 0);
	m_ctrlUpArrow.SetIcon(m_hUpArrow);
	m_ctrlDownArrow.SetIcon(m_hDownArrow);

	LOGFONT		lfStaticFont = {0};
	CFont		*pDefGuiFont = CFont::FromHandle(static_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT)));

	pDefGuiFont->GetLogFont(&lfStaticFont);

	p = GetDlgItem(IDC_SPEED);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlSpeed.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_SPEED);
		m_ctrlSpeed.m_nBtnID = IDC_SPEED;
		GetResString(&m_ctrlSpeed.m_strText, IDS_TRAYDLG_SPEED);

		m_ctrlSpeed.m_bUseIcon = true;
		m_ctrlSpeed.m_sIcon.cx = 16;
		m_ctrlSpeed.m_sIcon.cy = 16;
		m_ctrlSpeed.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_SPEED), IMAGE_ICON, m_ctrlSpeed.m_sIcon.cx, m_ctrlSpeed.m_sIcon.cy, 0);
		m_ctrlSpeed.m_bParentCapture = true;

		LOGFONT		lfFont = lfStaticFont;

		lfFont.lfWeight += 200;			// make it bold
		m_ctrlSpeed.m_cfFont.CreateFontIndirect(&lfFont);

		m_ctrlSpeed.m_bNoHover = true;
	}

	p = GetDlgItem(IDC_TOMAX);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlAllToMax.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_TOMAX);
		m_ctrlAllToMax.m_nBtnID = IDC_TOMAX;
		GetResString(&m_ctrlAllToMax.m_strText, IDS_PW_UA);

		m_ctrlAllToMax.m_bUseIcon = true;
		m_ctrlAllToMax.m_sIcon.cx = 16;
		m_ctrlAllToMax.m_sIcon.cy = 16;
		m_ctrlAllToMax.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_TOMAX), IMAGE_ICON, m_ctrlAllToMax.m_sIcon.cx, m_ctrlAllToMax.m_sIcon.cy, 0);
		m_ctrlAllToMax.m_bParentCapture = true;
		m_ctrlAllToMax.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_TOMIN);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlAllToMin.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_TOMIN);
		m_ctrlAllToMin.m_nBtnID = IDC_TOMIN;
		GetResString(&m_ctrlAllToMin.m_strText, IDS_PW_PA);

		m_ctrlAllToMin.m_bUseIcon = true;
		m_ctrlAllToMin.m_sIcon.cx = 16;
		m_ctrlAllToMin.m_sIcon.cy = 16;
		m_ctrlAllToMin.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_TOMIN), IMAGE_ICON, m_ctrlAllToMin.m_sIcon.cx, m_ctrlAllToMin.m_sIcon.cy, 0);
		m_ctrlAllToMin.m_bParentCapture = true;
		m_ctrlAllToMin.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_RESTORE);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlRestore.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_RESTORE);
		m_ctrlRestore.m_nBtnID = IDC_RESTORE;
		GetResString(&m_ctrlRestore.m_strText, IDS_MAIN_POPUP_RESTORE);

		m_ctrlRestore.m_bUseIcon = true;
		m_ctrlRestore.m_sIcon.cx = 16;
		m_ctrlRestore.m_sIcon.cy = 16;
		m_ctrlRestore.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_RESTORE), IMAGE_ICON, m_ctrlRestore.m_sIcon.cx, m_ctrlRestore.m_sIcon.cy, 0);
		m_ctrlRestore.m_bParentCapture = true;

		LOGFONT		lfFont = lfStaticFont;

		lfFont.lfWeight += 200;			// make it bold
		m_ctrlRestore.m_cfFont.CreateFontIndirect(&lfFont);
	}

	p = GetDlgItem(IDC_CONNECT);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlConnect.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_CONNECT);
		m_ctrlConnect.m_nBtnID = IDC_CONNECT;
		GetResString(&m_ctrlConnect.m_strText, IDS_CONNECTTOANYSERVER);

		m_ctrlConnect.m_bUseIcon = true;
		m_ctrlConnect.m_sIcon.cx = 16;
		m_ctrlConnect.m_sIcon.cy = 16;
		m_ctrlConnect.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_CONNECT), IMAGE_ICON, m_ctrlConnect.m_sIcon.cx, m_ctrlConnect.m_sIcon.cy, 0);
		m_ctrlConnect.m_bParentCapture = true;
		m_ctrlConnect.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_DISCONNECT);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlDisconnect.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_DISCONNECT);
		m_ctrlDisconnect.m_nBtnID = IDC_DISCONNECT;
		GetResString(&m_ctrlDisconnect.m_strText, IDS_MAIN_BTN_DISCONNECT);

		m_ctrlDisconnect.m_bUseIcon = true;
		m_ctrlDisconnect.m_sIcon.cx = 16;
		m_ctrlDisconnect.m_sIcon.cy = 16;
		m_ctrlDisconnect.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_DISCONNECT), IMAGE_ICON, m_ctrlDisconnect.m_sIcon.cx, m_ctrlDisconnect.m_sIcon.cy, 0);
		m_ctrlDisconnect.m_bParentCapture = true;
		m_ctrlDisconnect.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_PREFERENCES);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlPreferences.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_PREFERENCES);
		m_ctrlPreferences.m_nBtnID = IDC_PREFERENCES;
		GetResString(&m_ctrlPreferences.m_strText, IDS_PREFERENCES);

		m_ctrlPreferences.m_bUseIcon = true;
		m_ctrlPreferences.m_sIcon.cx = 16;
		m_ctrlPreferences.m_sIcon.cy = 16;
		m_ctrlPreferences.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_PREF_GENERAL), IMAGE_ICON, m_ctrlPreferences.m_sIcon.cx, m_ctrlPreferences.m_sIcon.cy, 0);
		m_ctrlPreferences.m_bParentCapture = true;
		m_ctrlPreferences.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	p = GetDlgItem(IDC_EXIT);
	if(p)
	{
		p->GetWindowRect(r);
		ScreenToClient(r);
		m_ctrlExit.Create(NULL, NULL, WS_CHILD|WS_VISIBLE, r, this, IDC_EXIT);
		m_ctrlExit.m_nBtnID = IDC_EXIT;
		GetResString(&m_ctrlExit.m_strText, IDS_EXIT);

		m_ctrlExit.m_bUseIcon = true;
		m_ctrlExit.m_sIcon.cx = 16;
		m_ctrlExit.m_sIcon.cy = 16;
		m_ctrlExit.m_hIcon = (HICON)::LoadImage(hInst, MAKEINTRESOURCE(IDI_TRAY_EXIT), IMAGE_ICON, m_ctrlExit.m_sIcon.cx, m_ctrlExit.m_sIcon.cy, 0);
		m_ctrlExit.m_bParentCapture = true;
		m_ctrlExit.m_cfFont.CreateFontIndirect(&lfStaticFont);
	}

	CString strBuffer;

	for (uint32 i = 0; i < ARRSIZE(s_aiResTbl); i++)
	{
		GetResString(&strBuffer, static_cast<UINT>(s_aiResTbl[i][1]));
		SetDlgItemText(s_aiResTbl[i][0], strBuffer);
	}

	uint32	dwMaxUp = g_App.m_pPrefs->GetMaxGraphUploadRate();
	uint32	dwMaxDown = g_App.m_pPrefs->GetMaxGraphDownloadRate();
	uint32	dwCurUp = g_App.m_pPrefs->GetMaxUpload();
	uint32	dwCurDown = g_App.m_pPrefs->GetMaxDownload();

	if (dwCurDown == UNLIMITED)
		dwCurDown = 0;

	dwCurDown = (dwMaxDown < dwCurDown) ? dwMaxDown : dwCurDown;
	dwCurUp = (dwMaxUp < dwCurUp) ? dwMaxUp : dwCurUp;

	FractionalRate2String(&m_strDownSpeedTxt, dwCurDown);
	FractionalRate2String(&m_strUpSpeedTxt, dwCurUp);

// Disable when Limitless Download
	bool	bLimited = !g_App.m_pPrefs->LimitlessDownload();

	m_DownSpeedInput.EnableWindow(bLimited);
	m_ctrlDownSpeedSld.EnableWindow(bLimited);

	m_ctrlDownSpeedSld.SetRange(10, dwMaxDown);
	if (dwMaxDown >= DN_FRAC_THRESHOLD)
	{
		m_ctrlDownSpeedSld.SetPageSize((dwMaxDown > 200) ? 100 : 50);
		m_ctrlDownSpeedSld.SetLineSize(10);
	}
	m_ctrlDownSpeedSld.SetPos(dwCurDown);

	m_ctrlUpSpeedSld.SetRange(10, dwMaxUp);
	if (dwMaxUp >= UP_FRAC_THRESHOLD)
	{
		m_ctrlUpSpeedSld.SetPageSize(50);
		m_ctrlUpSpeedSld.SetLineSize(10);
	}
	m_ctrlUpSpeedSld.SetPos(dwCurUp);

	UpdateData(FALSE);

	CFont	Font;

	Font.CreateFont(-14, 0, 900, 0, 700, 0, 0, 0, 0,
		OUT_STROKE_PRECIS, CLIP_STROKE_PRECIS, DRAFT_QUALITY, VARIABLE_PITCH | FF_SWISS, _T("Tahoma"));

	UINT	winver = g_App.m_pPrefs->GetWindowsVersion();

	if(winver == _WINVER_95_ || winver == _WINVER_NT4_)
	{
		m_ctrlSidebar.SetColors(GetSysColor(COLOR_CAPTIONTEXT),
									GetSysColor(COLOR_ACTIVECAPTION),
										GetSysColor(COLOR_ACTIVECAPTION));
	}
	else
	{
		m_ctrlSidebar.SetColors(GetSysColor(COLOR_CAPTIONTEXT),
									GetSysColor(COLOR_ACTIVECAPTION),
										GetSysColor(27));	//COLOR_GRADIENTACTIVECAPTION
	}

	m_ctrlSidebar.SetHorizontal(false);
	m_ctrlSidebar.SetFont(&Font);
	m_ctrlSidebar.SetWindowText(CLIENT_NAME_WITH_VER);

	CRect	rDesktop;
	CWnd	*pDesktopWnd = GetDesktopWindow();

	pDesktopWnd->GetClientRect(rDesktop);

	CPoint	pt = m_ptInitialPosition;

	pDesktopWnd->ScreenToClient(&pt);

	int		xpos, ypos;

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

	TCHAR	cLast = (m_strDownSpeedTxt.IsEmpty()) ? _T('0') : m_strDownSpeedTxt.GetAt(m_strDownSpeedTxt.GetLength() - 1);

//	Don't process when a user entered decimal period
	if ((cLast >= _T('0')) && (cLast <= _T('9')))
	{
		uint32	dwSpeed = TieUploadDownload(g_App.m_pPrefs->GetMaxUpload(), String2FranctionalRate(m_strDownSpeedTxt));
		
		dwSpeed = g_App.m_pPrefs->SetMaxDownloadWithCheck(dwSpeed);
		FractionalRate2String(&m_strDownSpeedTxt, dwSpeed);
		m_ctrlDownSpeedSld.SetPos(dwSpeed);

		UpdateData(FALSE);
	}
}

void CMuleSystrayDlg::OnChangeUptxt()
{
	UpdateData();

	TCHAR	cLast = (m_strUpSpeedTxt.IsEmpty()) ? _T('0') : m_strUpSpeedTxt.GetAt(m_strUpSpeedTxt.GetLength() - 1);

//	Don't process when a user entered decimal period
	if ((cLast >= _T('0')) && (cLast <= _T('9')))
	{
		uint32	dwSpeed = g_App.m_pPrefs->SetMaxUploadWithCheck(String2FranctionalRate(m_strUpSpeedTxt));

		FractionalRate2String(&m_strUpSpeedTxt, dwSpeed);
		m_ctrlUpSpeedSld.SetPos(dwSpeed);

		dwSpeed = g_App.m_pPrefs->GetMaxDownload();

		if (dwSpeed == UNLIMITED)
			dwSpeed = 0;
		else
			g_App.m_pPrefs->SetMaxDownload(dwSpeed);
		FractionalRate2String(&m_strDownSpeedTxt, dwSpeed);
		m_ctrlDownSpeedSld.SetPos(dwSpeed);

		UpdateData(FALSE);
	}
}

void CMuleSystrayDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	uint32	dwSpeed;

	if(pScrollBar == (CScrollBar*)&m_ctrlDownSpeedSld)
	{
		if ((nSBCode == TB_THUMBTRACK) || (nSBCode == TB_THUMBPOSITION))
		{
			dwSpeed = nPos;
			if (g_App.m_pPrefs->GetMaxGraphDownloadRate() >= DN_FRAC_THRESHOLD)
			{
			//	Round to nearest integer speed
				dwSpeed %= 10;
				dwSpeed = nPos - ((dwSpeed < 5) ? dwSpeed : (dwSpeed - 10));
			}
		}
		else
			dwSpeed = m_ctrlDownSpeedSld.GetPos();

		dwSpeed = TieUploadDownload(g_App.m_pPrefs->GetMaxUpload(), dwSpeed);
		m_ctrlDownSpeedSld.SetPos(dwSpeed);
		FractionalRate2String(&m_strDownSpeedTxt, dwSpeed);
		UpdateData(FALSE);
		g_App.m_pPrefs->SetMaxDownload(dwSpeed);
	}
	else if(pScrollBar == (CScrollBar*)&m_ctrlUpSpeedSld)
	{
		if ((nSBCode == TB_THUMBTRACK) || (nSBCode == TB_THUMBPOSITION))
		{
			dwSpeed = nPos;
			if (g_App.m_pPrefs->GetMaxGraphUploadRate() >= UP_FRAC_THRESHOLD)
			{
			//	Round to nearest integer speed
				dwSpeed %= 10;
				dwSpeed = nPos - ((dwSpeed < 5) ? dwSpeed : (dwSpeed - 10));
			}
		}
		else
			dwSpeed = m_ctrlUpSpeedSld.GetPos();

		FractionalRate2String(&m_strUpSpeedTxt, dwSpeed);
		m_ctrlUpSpeedSld.SetPos(dwSpeed);
		g_App.m_pPrefs->SetMaxUpload(dwSpeed);

		dwSpeed = g_App.m_pPrefs->GetMaxDownload();

		if (dwSpeed == UNLIMITED)
			dwSpeed = 0;
		else
			g_App.m_pPrefs->SetMaxDownload(dwSpeed);
		FractionalRate2String(&m_strDownSpeedTxt, dwSpeed);
		m_ctrlDownSpeedSld.SetPos(dwSpeed);

		UpdateData(FALSE);
	}

	CDialog::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CMuleSystrayDlg::OnLButtonUp(UINT nFlags, CPoint point)
{
	ReleaseCapture();
	EndDialog(m_nExitCode);
	m_bClosingDown = true;

	CDialog::OnLButtonUp(nFlags, point);
}

void CMuleSystrayDlg::OnRButtonDown(UINT nFlags, CPoint point)
{
	CRect	systrayRect;

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
//	Apply last settings, as due to waiting for decimal periods, something couldn't be applied
	g_App.m_pPrefs->SetMaxUploadWithCheck(String2FranctionalRate(m_strUpSpeedTxt));

	if (!g_App.m_pPrefs->LimitlessDownload())
	{
		uint32	dwSpeed = String2FranctionalRate(m_strDownSpeedTxt);

		dwSpeed = TieUploadDownload(g_App.m_pPrefs->GetMaxUpload(), dwSpeed);
		g_App.m_pPrefs->SetMaxDownloadWithCheck(dwSpeed);
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
