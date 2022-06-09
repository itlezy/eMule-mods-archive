// TrayMenuBtn.h														11.12.03
//------------------------------------------------------------------------------
#ifndef __TRAYMENUBTN_H__62BD5C8C_CE48_4973_ACFF_451D8350D95C__INCLUDED_
#define __TRAYMENUBTN_H__62BD5C8C_CE48_4973_ACFF_451D8350D95C__INCLUDED_

#ifdef BEGIN_MAP_EX
#undef BEGIN_MAP_EX
#endif
#ifdef MOO
#define BEGIN_MAP_EX(x) BEGIN_MSG_MAP(x)
#else
#define BEGIN_MAP_EX(x) BEGIN_MSG_MAP_EX(x)
#endif

class CTrayMenuBtn : public CWindowImpl<CTrayMenuBtn>
{
public:
	BEGIN_MAP_EX(CTrayMenuBtn)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_ERASEBKGND(OnEraseBkgnd)
	END_MSG_MAP()

public:
	bool	m_bBold;
	bool	m_bMouseOver;
	bool	m_bNoHover;
	bool	m_bUseIcon;
	bool	m_bParentCapture;
	UINT	m_nBtnID;
	SIZE	m_stIcon;
	HICON	m_hIcon;
	CString m_strText;
	CFont	m_cfFont;

	HWND	m_hLastWnd;

public:
	CTrayMenuBtn()
	{
		m_bBold = false;
		m_bMouseOver = false;
		m_bNoHover = false;
		m_bUseIcon = false;
		m_bParentCapture = false;
		m_hIcon = NULL;
		m_nBtnID = rand();
		m_stIcon.cx = 0;
		m_stIcon.cy = 0;
		m_strText = "";

		m_hLastWnd = NULL;
	}

	~CTrayMenuBtn()
	{
		if (m_hIcon)
			DestroyIcon(m_hIcon);
	}

	void OnMouseMove(UINT nFlags, CPoint point)
	{
		CRect rClient;
		GetClientRect(rClient);

		if (rClient.PtInRect(point))
		{
		//	if(m_hLastWnd == NULL)
				m_hLastWnd = SetCapture();

			if (!m_bNoHover)
			{	
				if(!m_bMouseOver)
				{
					m_bMouseOver = true;
					RedrawWindow();
				}
			}
		}
		else
		{
			if(m_bParentCapture)
			{
				HWND hParentWnd = GetParent();
				if (hParentWnd)
					::SetCapture(hParentWnd);
				else
					ReleaseCapture();
			}
			else
				ReleaseCapture();
		
		/*	if(m_hLastWnd)
				::SetCapture(m_hLastWnd);
			else
				::ReleaseCapture();
			m_hLastWnd = NULL;
		*/	
			if(m_bMouseOver)
			{
				m_bMouseOver = false;
				RedrawWindow();
			}
		}

		SetMsgHandled(false);
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		CRect rClient;
		GetClientRect(rClient);

		if (rClient.PtInRect(point))
		{
			HWND hParentWnd = GetParent();
			if (hParentWnd)
				::PostMessage(hParentWnd, WM_COMMAND, MAKEWPARAM(m_nBtnID,BN_CLICKED), (long)m_hWnd);
		}
		else
		{
			if (m_hLastWnd)
				::SetCapture(m_hLastWnd);
			else
				::ReleaseCapture();
			m_hLastWnd = NULL;

			m_bMouseOver = false;
			Invalidate();
		}		
		
		SetMsgHandled(false);
	}

	LRESULT OnEraseBkgnd(HDC hDC)
	{	
		EMULE_TRY

		if (hDC == NULL)
		{	
			hDC = GetDC();
		}

		CRect rcClient;
		GetClientRect(rcClient);

		HDC hMemDC = ::CreateCompatibleDC(hDC);
		HBITMAP hMemBMP = ::CreateCompatibleBitmap(hDC, rcClient.Width(), rcClient.Height());
		HBITMAP hOldBMP = (HBITMAP)::SelectObject(hMemDC, hMemBMP);
				
		HFONT hOldFONT = NULL;
		if (m_cfFont.operator HFONT())
		{
			hOldFONT = (HFONT)::SelectObject(hMemDC, m_cfFont.operator HFONT());
		}

		BOOL bEnabled = IsWindowEnabled();
		
		if (m_bMouseOver && bEnabled)
		{	
			::FillRect(hMemDC, rcClient, GetSysColorBrush(COLOR_HIGHLIGHT));
			::SetTextColor(hMemDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
		}
		else
		{
			::FillRect(hMemDC, rcClient, GetSysColorBrush(COLOR_BTNFACE));
			::SetTextColor(hMemDC, GetSysColor(COLOR_BTNTEXT));
		}

		int iLeftOffset = 0;
		if (m_bUseIcon)
		{		
			::DrawState(hMemDC, NULL, NULL, (LPARAM)m_hIcon, NULL, 2, 
						rcClient.Height()/2-m_stIcon.cy/2, 
						16, 16, DST_ICON|DSS_NORMAL);
			iLeftOffset = m_stIcon.cx + 4;
		}

		::SetBkMode(hMemDC, TRANSPARENT);
		CRect rcText(0,0,0,0);
		::DrawText(hMemDC, m_strText.operator LPCTSTR(), m_strText.GetLength(),
					rcText, DT_CALCRECT|DT_SINGLELINE|DT_LEFT);
		//CPoint pt((rcClient.Width()>>1)-(rcText.Width()>>1),(rcClient.Height()>>1)-(rcText.Height()>>1));
		CPoint pt(rcClient.left+2+iLeftOffset, rcClient.Height()/2-rcText.Height()/2);
		CPoint sz(rcText.Width(),rcText.Height());
		::DrawState(hMemDC, NULL, NULL, (LPARAM)m_strText.operator LPCTSTR(), m_strText.GetLength(), 
					rcClient.left+2+iLeftOffset, 
					rcClient.Height()/2-rcText.Height()/2,
					rcText.Width(), rcText.Height(), 	
					DST_TEXT | (bEnabled ? DSS_NORMAL : DSS_DISABLED));


		::BitBlt(hDC, 0, 0, rcClient.Width(), rcClient.Height(), hMemDC, 0, 0, SRCCOPY);
		::SelectObject(hMemDC, hOldBMP);
		if (hOldFONT)
		{	
			::SelectObject(hMemDC, hOldFONT);
		}
		DeleteDC(hMemDC);
		DeleteObject(hMemBMP);

		SetMsgHandled(true);
		return FALSE;

		EMULE_CATCH
		return FALSE;
	}
};

#endif	// #ifndef __TRAYMENUBTN_H__62BD5C8C_CE48_4973_ACFF_451D8350D95C__INCLUDED_