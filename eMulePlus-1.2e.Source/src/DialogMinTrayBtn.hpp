// ------------------------------------------------------------
//  CDialogMinTrayBtn template class
//  MFC CDialog with minimize to systemtray button (0.04a)
//  Supports WinXP styles (thanks to David Yuheng Zhao for CVisualStylesXP - yuheng_zhao@yahoo.com)
// ------------------------------------------------------------
//  DialogMinTrayBtn.hpp
//  zegzav - 2002,2003 - eMule project (http://www.emule-project.net)
// ------------------------------------------------------------

#include "AfxBeginMsgMapTemplate.h"
#include "emule.h"

#define CAPTION_BUTTONSPACE		(2)

#define TIMERMINTRAYBTN_ID		0x76617a67
#define TIMERMINTRAYBTN_PERIOD	200	// ms

BEGIN_TM_PART_STATES(TRAYBUTTON)
	TM_STATE(1, TRAYBS, NORMAL)
	TM_STATE(2, TRAYBS, HOT)
	TM_STATE(3, TRAYBS, PUSHED)
	TM_STATE(4, TRAYBS, DISABLED)
	// Inactive
	TM_STATE(5, TRAYBS, INORMAL)
	TM_STATE(6, TRAYBS, IHOT)
	TM_STATE(7, TRAYBS, IPUSHED)
	TM_STATE(8, TRAYBS, IDISABLED)
END_TM_PART_STATES()

#define BMP_TRAYBTN_WIDTH		(21)
#define BMP_TRAYBTN_HEIGHT		(21)
#define BMP_TRAYBTN_BLUE		_T("IDB_LUNA_BLUE")
#define BMP_TRAYBTN_METALLIC	_T("IDB_LUNA_METALLIC")
#define BMP_TRAYBTN_HOMESTEAD	_T("IDB_LUNA_HOMESTEAD")
#define BMP_TRAYBTN_TRANSCOLOR	(RGB(255,0,255))

#define VISUALSTYLESXP_DEFAULTFILE		L"LUNA.MSSTYLES"
#define VISUALSTYLESXP_NAMEBLUE			L"NORMALCOLOR"
#define VISUALSTYLESXP_NAMEMETALLIC		L"METALLIC"
#define VISUALSTYLESXP_NAMEHOMESTEAD	L"HOMESTEAD"

// _WIN32_WINNT >= 0x0501 (XP only)
#define _WM_THEMECHANGED	0x031A
#define _ON_WM_THEMECHANGED()														\
	{	_WM_THEMECHANGED, 0, 0, 0, AfxSig_l,										\
		(AFX_PMSG)(AFX_PMSGW)														\
		(static_cast< LRESULT (AFX_MSG_CALL CWnd::*)(void) > (_OnThemeChanged))		\
	},

template <class BASE> CThemeHelperST CDialogMinTrayBtn<BASE>::m_themeHelper = CThemeHelperST();

template <class BASE> CDialogMinTrayBtn<BASE>::CDialogMinTrayBtn() :
	m_MinTrayBtnPos(0,0), m_MinTrayBtnSize(0,0), m_bMinTrayBtnEnabled(TRUE), m_bMinTrayBtnVisible(TRUE),
	m_bMinTrayBtnUp(TRUE), m_bMinTrayBtnCapture(FALSE), m_bMinTrayBtnActive(FALSE), m_bMinTrayBtnHitTest(FALSE),
	m_pfnTransparentBlt(NULL), m_hMsImg32(NULL)
{
	MinTrayBtnInit();
}

template <class BASE> CDialogMinTrayBtn<BASE>::CDialogMinTrayBtn(LPCTSTR lpszTemplateName, CWnd *pParentWnd) : BASE(lpszTemplateName, pParentWnd),
	m_MinTrayBtnPos(0,0), m_MinTrayBtnSize(0,0), m_bMinTrayBtnEnabled(TRUE), m_bMinTrayBtnVisible(TRUE),
	m_bMinTrayBtnUp(TRUE), m_bMinTrayBtnCapture(FALSE), m_bMinTrayBtnActive(FALSE), m_bMinTrayBtnHitTest(FALSE),
	m_pfnTransparentBlt(NULL), m_hMsImg32(NULL)
{
	MinTrayBtnInit();
}

template <class BASE> CDialogMinTrayBtn<BASE>::CDialogMinTrayBtn(UINT nIDTemplate, CWnd* pParentWnd) : BASE(nIDTemplate, pParentWnd),
	m_MinTrayBtnPos(0,0), m_MinTrayBtnSize(0,0), m_bMinTrayBtnEnabled(TRUE), m_bMinTrayBtnVisible(TRUE),
	m_bMinTrayBtnUp(TRUE), m_bMinTrayBtnCapture(FALSE), m_bMinTrayBtnActive(FALSE), m_bMinTrayBtnHitTest(FALSE),
	m_pfnTransparentBlt(NULL), m_hMsImg32(NULL)
{
	MinTrayBtnInit();
}

template <class BASE> CDialogMinTrayBtn<BASE>::~CDialogMinTrayBtn()
{
	if (m_hMsImg32 != NULL)
		FreeLibrary(m_hMsImg32);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnInit()
{
	m_nMinTrayBtnTimerId = 0;

	// - Never use the 'TransparentBlt' function under Win9x (read SDK)
	// - Load the 'MSIMG32.DLL' only, if it's really needed.
	MinTrayBtnInitBitmap();
	if (g_App.m_pPrefs->GetWindowsVersion() != _WINVER_95_ && m_pfnTransparentBlt == NULL)
	{
		m_hMsImg32 = LoadLibrary(_T("MSIMG32.DLL"));

		if (m_hMsImg32 != NULL)
		{
			(FARPROC &)m_pfnTransparentBlt = GetProcAddress(m_hMsImg32, "TransparentBlt");
			if (m_pfnTransparentBlt == NULL)
			{
				FreeLibrary(m_hMsImg32);
				m_hMsImg32 = NULL;
			}
		}
	}
}

BEGIN_MESSAGE_MAP_TEMPLATE(template <class BASE>, CDialogMinTrayBtn<BASE>, CDialogMinTrayBtn, BASE)
	ON_WM_NCPAINT()
	ON_WM_NCACTIVATE()
	ON_WM_NCHITTEST()
	ON_WM_NCLBUTTONDOWN()
	ON_WM_NCRBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_TIMER()
	_ON_WM_THEMECHANGED()
END_MESSAGE_MAP()

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::OnInitDialog()
{
	BOOL	bReturn = BASE::OnInitDialog();

	m_nMinTrayBtnTimerId = SetTimer(TIMERMINTRAYBTN_ID, TIMERMINTRAYBTN_PERIOD, NULL);
	return bReturn;
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnNcPaint() 
{
	BASE::OnNcPaint();
	MinTrayBtnUpdatePosAndSize();
	MinTrayBtnDraw();
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::OnNcActivate(BOOL bActive)
{
	MinTrayBtnUpdatePosAndSize();

	BOOL	bResult = BASE::OnNcActivate(bActive);

	m_bMinTrayBtnActive = bActive;
	MinTrayBtnDraw();
	return bResult;
}

#if _MFC_VER>=0x0800
template <class BASE> LRESULT CDialogMinTrayBtn<BASE>::OnNcHitTest(CPoint point)
#else
template <class BASE> UINT CDialogMinTrayBtn<BASE>::OnNcHitTest(CPoint point)
#endif
{
	BOOL	bPreviousHitTest = m_bMinTrayBtnHitTest;

	m_bMinTrayBtnHitTest = MinTrayBtnHitTest(point);
	if (!IsWindowsClassicStyle() && m_bMinTrayBtnHitTest != bPreviousHitTest)
		MinTrayBtnDraw();	// Windows XP Style (hot button)
	if (m_bMinTrayBtnHitTest)
		return HTMINTRAYBUTTON;
	return BASE::OnNcHitTest(point);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnNcLButtonDown(UINT nHitTest, CPoint point) 
{
	if ((GetStyle() & WS_DISABLED) != 0 || (!MinTrayBtnIsEnabled()) || (!MinTrayBtnIsVisible()) || (!MinTrayBtnHitTest(point)))
	{
		BASE::OnNcLButtonDown(nHitTest, point);
		return;
	}

	SetCapture();
	m_bMinTrayBtnCapture = TRUE;
	MinTrayBtnSetDown();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnNcRButtonDown(UINT nHitTest, CPoint point) 
{
	if ((GetStyle() & WS_DISABLED) != 0 || (!MinTrayBtnIsVisible()) || (!MinTrayBtnHitTest(point)))
		BASE::OnNcRButtonDown(nHitTest, point);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnMouseMove(UINT nFlags, CPoint point) 
{
	if ((GetStyle() & WS_DISABLED) != 0 || (!m_bMinTrayBtnCapture))
	{ 
		BASE::OnMouseMove(nFlags, point);
		return;
	}

	ClientToScreen(&point);
	m_bMinTrayBtnHitTest = MinTrayBtnHitTest(point);
	if (m_bMinTrayBtnHitTest)
	{
		if (m_bMinTrayBtnUp)
			MinTrayBtnSetDown();
	}
	else if (!m_bMinTrayBtnUp)
		MinTrayBtnSetUp();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnLButtonUp(UINT nFlags, CPoint point) 
{
	if ((GetStyle() & WS_DISABLED) || (!m_bMinTrayBtnCapture))
	{
		BASE::OnLButtonUp(nFlags, point);
		return;
	}

	ReleaseCapture();
	m_bMinTrayBtnCapture = FALSE;
	MinTrayBtnSetUp();

	ClientToScreen(&point);
	if (MinTrayBtnHitTest(point))
		SendMessage(WM_SYSCOMMAND, SC_MINIMIZETRAY, MAKEWPARAM(point.x, point.y)); 
}

template <class BASE> void CDialogMinTrayBtn<BASE>::OnTimer(UINT_PTR nIDEvent)
{
	if (!IsWindowsClassicStyle() && (nIDEvent == m_nMinTrayBtnTimerId))
	{
	//	Visual XP Style (hot button)
		CPoint	point;

		GetCursorPos(&point);

		BOOL	bHitTest = MinTrayBtnHitTest(point);

		if (m_bMinTrayBtnHitTest != bHitTest)
		{
			m_bMinTrayBtnHitTest = bHitTest;
			MinTrayBtnDraw();
		}
	}
}

template <class BASE> LRESULT CDialogMinTrayBtn<BASE>::_OnThemeChanged()
{
	MinTrayBtnInitBitmap();
	return 0;
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnUpdatePosAndSize()
{
	bool	bNrmWnd = (GetExStyle() & WS_EX_TOOLWINDOW) == 0;
	int		iXSz;

	if (IsWindowsClassicStyle())
		iXSz = GetSystemMetrics(bNrmWnd ? SM_CXSIZE : SM_CXSMSIZE) - 2;
	else
	//	SM_SX*SIZE seems wrong when theming is on, as buttons are squares, we use Y size.
		iXSz = GetSystemMetrics(bNrmWnd ? SM_CYSIZE : SM_CYSMSIZE) - 4;

	m_MinTrayBtnSize = CSize(iXSz, GetSystemMetrics(bNrmWnd ? SM_CYSIZE : SM_CYSMSIZE) - 4);

	DWORD	dwStyle = GetStyle();
	bool	bSzFrm = (dwStyle & WS_THICKFRAME) != 0;
	CRect	rcWnd;

	GetWindowRect(&rcWnd);
	iXSz += CAPTION_BUTTONSPACE;

	m_MinTrayBtnPos = CSize( rcWnd.Width() - ((iXSz + CAPTION_BUTTONSPACE) << 1) + GetSystemMetrics(bSzFrm ? SM_CXSIZEFRAME : SM_CXFIXEDFRAME),
		CAPTION_BUTTONSPACE + GetSystemMetrics(bSzFrm ? SM_CYSIZEFRAME : SM_CYFIXEDFRAME) );

//	If it isn't a toolbox and minimize/maximize buttons are present, include their size
	if (bNrmWnd && (dwStyle & (WS_MINIMIZEBOX | WS_MAXIMIZEBOX)) != 0)
		m_MinTrayBtnPos.x -= IsWindowsClassicStyle() ? ((iXSz << 1) + CAPTION_BUTTONSPACE) : ((iXSz + CAPTION_BUTTONSPACE) << 1);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnShow()
{
	if (MinTrayBtnIsVisible())
		return;

	m_bMinTrayBtnVisible = TRUE;
	if (IsWindowVisible())
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnHide()
{
	if (!MinTrayBtnIsVisible())
		return;

	m_bMinTrayBtnVisible = FALSE;
	if (IsWindowVisible())
		RedrawWindow(NULL, NULL, RDW_FRAME | RDW_INVALIDATE | RDW_UPDATENOW);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnEnable()
{
	if (MinTrayBtnIsEnabled())
		return;

	m_bMinTrayBtnEnabled = TRUE;
	MinTrayBtnSetUp();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnDisable()
{
	if (!MinTrayBtnIsEnabled())
		return;

	m_bMinTrayBtnEnabled = FALSE;
	if (m_bMinTrayBtnCapture)
	{
		ReleaseCapture();
		m_bMinTrayBtnCapture = FALSE;
	}
	MinTrayBtnSetUp();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnDraw()
{
	if (!MinTrayBtnIsVisible())
		return;

	CDC	*pDC = GetWindowDC();

	if (pDC == NULL)
		return;

	CRect	rcBtn = MinTrayBtnGetRect();
	UINT	uiState;

	if (IsWindowsClassicStyle())
	{
	//	Button
		uiState = DFCS_BUTTONPUSH;

		if (!m_bMinTrayBtnUp)
			uiState |= DFCS_PUSHED;

		pDC->DrawFrameControl(rcBtn, DFC_BUTTON, uiState);

	//	Dot
		rcBtn.DeflateRect(2,2);

		UINT	iCptWdth = MinTrayBtnGetSize().cy + (CAPTION_BUTTONSPACE << 1);
		UINT	iPixRatio1 = iCptWdth >= 20 ? 2 + ((iCptWdth - 20) >> 3) : (iCptWdth >= 14 ? 2 : 1);
		UINT	iPixRatio2 = iCptWdth >= 12 ? 1 + ((iCptWdth - 12) >> 3) : 0;
		CRect	rcDot(CPoint(0, 0), CPoint((1 + iPixRatio1 * 3) >> 1, iPixRatio1));
		CSize	szSpc((1 + iPixRatio2 * 3) >> 1, iPixRatio2);

		rcDot += rcBtn.BottomRight() - rcDot.Size() - szSpc;

		if (!m_bMinTrayBtnUp)
			rcDot += CPoint(1, 1);

		int	iColor = COLOR_BTNTEXT;

		if (!m_bMinTrayBtnEnabled)
		{
			iColor = COLOR_GRAYTEXT;
			pDC->FillSolidRect(rcDot + CPoint(1, 1), GetSysColor(COLOR_BTNHILIGHT));
		}
		pDC->FillSolidRect(rcDot, GetSysColor(iColor));
	}
	else
	{
	//	VisualStylesXP
		uiState = MINBS_NORMAL;

		if (!m_bMinTrayBtnEnabled)
			uiState = TRAYBS_DISABLED;
		else if ((GetStyle() & WS_DISABLED) == 0 && m_bMinTrayBtnHitTest)
			uiState = (m_bMinTrayBtnCapture) ? MINBS_PUSHED : MINBS_HOT;

	//	Inactive
		if (!m_bMinTrayBtnActive)
			uiState += 4;	// Inactive state TRAYBS_Ixxx

		if (m_bmMinTrayBtnBitmap.m_hObject != NULL && m_pfnTransparentBlt != NULL)
		{
		//	Known theme (bitmap)
			CBitmap	*pBmpOld;
			CDC		dcMem;

			if (dcMem.CreateCompatibleDC(pDC) && (pBmpOld = dcMem.SelectObject(&m_bmMinTrayBtnBitmap)) != NULL)
			{
				m_pfnTransparentBlt( pDC->m_hDC, rcBtn.left, rcBtn.top, rcBtn.Width(), rcBtn.Height(), dcMem.m_hDC, 0,
					BMP_TRAYBTN_HEIGHT * (uiState - 1), BMP_TRAYBTN_WIDTH, BMP_TRAYBTN_HEIGHT, BMP_TRAYBTN_TRANSCOLOR );
				dcMem.SelectObject(pBmpOld);
			}
		}
		else
		{
			HTHEME	hTheme = m_themeHelper.OpenThemeData(m_hWnd, L"Window");

			if (hTheme != NULL)
			{
				m_themeHelper.DrawThemeBackground(hTheme, NULL, pDC->m_hDC, WP_MINBUTTON, uiState, &rcBtn, NULL);
				m_themeHelper.CloseThemeData(hTheme);
			}
		}
	}

	ReleaseDC(pDC);
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::MinTrayBtnHitTest(CPoint point) const
{
	CRect	r;

	GetWindowRect(&r);
	point.Offset(-r.TopLeft());
	r = MinTrayBtnGetRect();
	r.InflateRect(0, CAPTION_BUTTONSPACE);
	return r.PtInRect(point);
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnSetUp()
{
	m_bMinTrayBtnUp = TRUE;
	MinTrayBtnDraw();
}

template <class BASE> void CDialogMinTrayBtn<BASE>::MinTrayBtnSetDown()
{
	m_bMinTrayBtnUp = FALSE;
	MinTrayBtnDraw();
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::IsWindowsClassicStyle() const
{
	return m_bMinTrayBtnWindowsClassicStyle;
}

template <class BASE> void CDialogMinTrayBtn<BASE>::SetWindowText(LPCTSTR lpszString)
{
	BASE::SetWindowText(lpszString);
	MinTrayBtnDraw();
}

template <class BASE> TCHAR* CDialogMinTrayBtn<BASE>::GetVisualStylesXPColor()
{
	if (IsWindowsClassicStyle())
		return NULL;

	WCHAR	szwThemeFile[MAX_PATH];
	WCHAR	szwThemeColor[256];

	if (m_themeHelper.GetCurrentThemeName(szwThemeFile, ARRSIZE(szwThemeFile), szwThemeColor, ARRSIZE(szwThemeColor), NULL, 0) != S_OK)
		return NULL;

	WCHAR	*p;

	if ((p = wcsrchr(szwThemeFile, L'\\')) == NULL)
		return NULL;

	if (_wcsicmp(++p, VISUALSTYLESXP_DEFAULTFILE) == 0)
	{
		if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEBLUE) == 0)
			return BMP_TRAYBTN_BLUE;
		else if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEMETALLIC) == 0)
			return BMP_TRAYBTN_METALLIC;
		else if (_wcsicmp(szwThemeColor, VISUALSTYLESXP_NAMEHOMESTEAD) == 0)
			return BMP_TRAYBTN_HOMESTEAD;
	}

	return NULL;
}

template <class BASE> BOOL CDialogMinTrayBtn<BASE>::MinTrayBtnInitBitmap()
{
	m_bMinTrayBtnWindowsClassicStyle = !m_themeHelper.IsThemeActive() || !m_themeHelper.IsAppThemed();

	m_bmMinTrayBtnBitmap.DeleteObject();

	TCHAR	*pcBmp;

	if ((pcBmp = GetVisualStylesXPColor()) == NULL)
		return FALSE;

	return m_bmMinTrayBtnBitmap.LoadBitmap(pcBmp);
}
