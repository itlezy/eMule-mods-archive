// TrayMenuDlg.h														11.12.03
//------------------------------------------------------------------------------
#ifndef __TRAYMENUDLG_H__4618ADE8_3C45_46bd_ADF6_5080FF7D2326__INCLUDED_
#define __TRAYMENUDLG_H__4618ADE8_3C45_46bd_ADF6_5080FF7D2326__INCLUDED_

#include "TrayMenuBtn.h"
#include "GStatic.h"
#include "..\..\opcodes.h"

// classview doesn't know "BEGIN_MSG_MAP_EX", so we have to fool it
#ifdef BEGIN_MAP_EX
#undef BEGIN_MAP_EX
#endif
#ifdef MOO
#define BEGIN_MAP_EX(x) BEGIN_MSG_MAP(x)
#else
#define BEGIN_MAP_EX(x) BEGIN_MSG_MAP_EX(x)
#endif

class CInputBox : public CWindowImpl<CInputBox, CEdit>
{
    BEGIN_MAP_EX(CInputBox)
        MSG_WM_CONTEXTMENU(OnContextMenu)
    END_MSG_MAP()
 
    void OnContextMenu(HWND hwndCtrl, CPoint ptClick)
    {
        // do nothing
    }
};

class CTrayMenuDlg : public CDialogImpl<CTrayMenuDlg>,
					 public CWinDataExchange<CTrayMenuDlg>
{
public:
    enum { IDD = IDD_SYSTRAYDLG };
 
    BEGIN_MAP_EX(CTrayMenuDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        MESSAGE_HANDLER(WM_CLOSE, OnClose)
        MSG_WM_HSCROLL(OnHScroll)
		MSG_WM_MOUSEMOVE(OnMouseMove)
		MSG_WM_LBUTTONUP(OnLButtonUp)
		MSG_WM_RBUTTONDOWN(OnRButtonDown)
		MSG_WM_KILLFOCUS(OnKillFocus)
		MSG_WM_SHOWWINDOW(OnShowWindow)
		MSG_WM_CAPTURECHANGED(OnCaptureChanged)
		MSG_WM_COMMAND(OnCommand)
		COMMAND_HANDLER_EX(IDC_DOWNTXT, EN_CHANGE, OnChangeDownTxt)
		COMMAND_HANDLER_EX(IDC_UPTXT, EN_CHANGE, OnChangeUpTxt)
    END_MSG_MAP()

	BEGIN_DDX_MAP(CTrayMenuDlg)
        DDX_CONTROL(IDC_DOWNTXT, m_ctrlDownSpeedTxt)
		DDX_CONTROL(IDC_UPTXT, m_ctrlUpSpeedTxt)
		DDX_INT(IDC_DOWNTXT, m_nDownSpeed)
		DDX_INT(IDC_UPTXT, m_nUpSpeed)
    END_DDX_MAP()

protected:
	UINT m_nExitCode;
	bool m_bClosingDown;

	CPoint m_ptInitialPosition;

	UINT m_nMaxDown;
	UINT m_nMaxUp;
	
	UINT m_nDownSpeed;
	UINT m_nUpSpeed;

	CTrackBarCtrl m_ctrlDownSpeedSld;
	CTrackBarCtrl m_ctrlUpSpeedSld;

	CInputBox m_ctrlDownSpeedTxt;
	CInputBox m_ctrlUpSpeedTxt;

	CStatic m_ctrlDownArrow;
	CStatic m_ctrlUpArrow;

	CGradientStatic m_ctrlSidebar;

	HICON m_hDownArrow;
	HICON m_hUpArrow;

	CTrayMenuBtn m_ctrlSpeed;
	CTrayMenuBtn m_ctrlAllToMax;
	CTrayMenuBtn m_ctrlAllToMin;
	CTrayMenuBtn m_ctrlConnect;
	CTrayMenuBtn m_ctrlDisconnect;
	CTrayMenuBtn m_ctrlPreferences;
	CTrayMenuBtn m_ctrlAbout;
	CTrayMenuBtn m_ctrlExit;
	

	HWND m_hLastWnd;
public:

	CTrayMenuDlg(CPoint pt, UINT nMaxUp, UINT nMaxDown, UINT nCurUp, UINT nCurDown)
	{
		m_ptInitialPosition = pt;
		
		if (nCurDown == UNLIMITED)
			nCurDown = 0;
		if (nCurUp == UNLIMITED)
			nCurUp = 0;

		m_nMaxUp = nMaxUp;
		m_nMaxDown = nMaxDown;
		m_nUpSpeed = nCurUp;
		m_nDownSpeed = nCurDown;

		m_hUpArrow = NULL;
		m_hDownArrow = NULL;

		m_bClosingDown = false;
		m_nExitCode = 0;

		m_hLastWnd = NULL;
	}

	~CTrayMenuDlg()
	{
		if (m_hUpArrow)
			DestroyIcon(m_hUpArrow);
		if (m_hDownArrow)
			DestroyIcon(m_hDownArrow);
	}

protected:
	void CreateTrayBtn(CTrayMenuBtn* pBtn, UINT nID, UINT nStringID, UINT nIconID, bool bBold = false, bool bHover = true)
	{
		EMULE_TRY

		HWND hTmp = GetDlgItem(nID);
		if (hTmp)
		{
			CRect rcWindow;
			::GetWindowRect(hTmp,rcWindow);
			ScreenToClient(&rcWindow);
			pBtn->Create(m_hWnd, rcWindow, NULL, WS_CHILD|WS_VISIBLE, 
								NULL, nID, NULL);
			pBtn->m_nBtnID = nID;
			TCHAR szBuffer[256];
			AtlLoadString(nStringID, szBuffer, _countof(szBuffer));
			pBtn->m_strText.Format(_T("%s"), szBuffer);
			pBtn->m_bUseIcon = true;
			pBtn->m_stIcon.cx = 16;
			pBtn->m_stIcon.cy = 16;
			pBtn->m_hIcon = (HICON)::LoadImage(_Module.GetModuleInstance(),
												MAKEINTRESOURCE(nIconID),
												IMAGE_ICON,
												pBtn->m_stIcon.cx,
												pBtn->m_stIcon.cy, 0);
			pBtn->m_bParentCapture = true;
			pBtn->m_cfFont = AtlGetStockFont(ANSI_VAR_FONT);//(DEFAULT_GUI_FONT);
			if(bBold)
			{	
				LOGFONT lfFont;
				pBtn->m_cfFont.GetLogFont(&lfFont);
				pBtn->m_cfFont.DeleteObject();
				lfFont.lfWeight += 200;
				pBtn->m_cfFont.CreateFontIndirect(&lfFont);
			}
			if(!bHover)
			{	
				pBtn->m_bNoHover = true;
			}
		}

		EMULE_CATCH
	}

	LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
		EMULE_TRY

		m_bClosingDown = false;

		m_hDownArrow = (HICON)::LoadImage(_Module.GetModuleInstance(), 
										MAKEINTRESOURCE(IDI_TRAY_DOWN), 
										IMAGE_ICON, 16, 16, 0);
		
		m_hUpArrow = (HICON)::LoadImage(_Module.GetModuleInstance(), 
										MAKEINTRESOURCE(IDI_TRAY_UP), 
										IMAGE_ICON, 16, 16, 0);
	
		m_ctrlDownArrow.Attach(GetDlgItem(IDC_TRAYDOWN));
		m_ctrlDownArrow.SetIcon(m_hDownArrow);

		m_ctrlUpArrow.Attach(GetDlgItem(IDC_TRAYUP));
		m_ctrlUpArrow.SetIcon(m_hUpArrow);

		m_ctrlDownSpeedSld.Attach(GetDlgItem(IDC_DOWNSLD));
		m_ctrlDownSpeedSld.SetRange(1, m_nMaxDown);
		m_ctrlDownSpeedSld.SetPos(m_nDownSpeed);
		
		m_ctrlUpSpeedSld.Attach(GetDlgItem(IDC_UPSLD));
        m_ctrlUpSpeedSld.SetRange(1, m_nMaxUp);		
		m_ctrlUpSpeedSld.SetPos(m_nUpSpeed);	

		m_ctrlSidebar.SubclassWindow(GetDlgItem(IDC_SIDEBAR));
		DoDataExchange(false);

		CreateTrayBtn(&m_ctrlSpeed, IDC_SPEED, IDS_TRAY_SPEED, IDI_TRAY_SPEED, true, false);
		CreateTrayBtn(&m_ctrlAllToMax, IDC_TOMAX, IDS_TRAY_TOMAX, IDI_TRAY_TOMAX);
		CreateTrayBtn(&m_ctrlAllToMin, IDC_TOMIN, IDS_TRAY_TOMIN, IDI_TRAY_TOMIN);
		CreateTrayBtn(&m_ctrlConnect, IDC_CONNECT, IDS_TRAY_CONNECT, IDI_TRAY_CONNECT);
		CreateTrayBtn(&m_ctrlDisconnect, IDC_DISCONNECT, IDS_TRAY_DISCONNECT, IDI_TRAY_DISCONNECT);
		CreateTrayBtn(&m_ctrlPreferences, IDC_PREFERENCES, IDS_TRAY_PREFERENCES, IDI_TRAY_PREFERENCES);
		CreateTrayBtn(&m_ctrlAbout, IDC_ABOUT, IDS_TRAY_ABOUT, IDI_TRAY_ABOUT, true);
		CreateTrayBtn(&m_ctrlExit, IDC_EXIT, IDS_TRAY_EXIT, IDI_TRAY_EXIT);		
	
		CFont Font;
		Font.CreateFont(-16,0,900,0,700,0,0,0,0,3,2,1,34,_T("Tahoma"));

// ToDo
	/*	UINT winver = GetWinVersion();
		if(winver == _WINVER_95_ || winver == _WINVER_NT4_)
		{
	*/		m_ctrlSidebar.SetColors(GetSysColor(COLOR_CAPTIONTEXT), 
										GetSysColor(COLOR_ACTIVECAPTION), 
											GetSysColor(COLOR_ACTIVECAPTION));
	/*	}
		else
		{
			m_ctrlSidebar.SetColors(GetSysColor(COLOR_CAPTIONTEXT), 
										GetSysColor(COLOR_ACTIVECAPTION), 
											GetSysColor(27));	//COLOR_GRADIENTACTIVECAPTION
		}
	*/

		m_ctrlSidebar.SetHorizontal(false);
		m_ctrlSidebar.SetFont(&Font);
		TCHAR szBuffer[256];
		AtlLoadString(IDR_MAINFRAME, szBuffer, _countof(szBuffer));
		m_ctrlSidebar.SetWindowText(szBuffer);

		CRect rcDesktop;
		HWND hDesktopWnd = ::GetDesktopWindow();
		::GetClientRect(hDesktopWnd, rcDesktop);

		CPoint pt = m_ptInitialPosition;
		::ScreenToClient(hDesktopWnd, &pt);
		int xpos, ypos;

		CRect rcWnd;
		GetWindowRect(rcWnd);
		if(m_ptInitialPosition.x + rcWnd.Width() < rcDesktop.right)
			xpos = pt.x;
		else
			xpos = pt.x - rcWnd.Width();
		if(m_ptInitialPosition.y - rcWnd.Height() < rcDesktop.top)
			ypos = pt.y;
		else
			ypos = pt.y - rcWnd.Height();

		MoveWindow(xpos, ypos, rcWnd.Width(), rcWnd.Height());

        return TRUE;

		EMULE_CATCH
		return FALSE;
    }
 
    LRESULT OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
    {
        EndDialog(m_nExitCode);
		m_bClosingDown = true;
        return 0;
    }
	
	void OnMouseMove(UINT nFlags, CPoint point)
	{	
		EMULE_TRY

		HWND hWnd = ChildWindowFromPointEx(point, CWP_SKIPINVISIBLE|CWP_SKIPDISABLED);
		if (hWnd)
		{
			if (hWnd == m_hWnd || hWnd == GetDlgItem(IDC_SIDEBAR))
			{
				SetCapture();
			}
			else
			{	
				ReleaseCapture();
			}
		}
		else
		{
			SetCapture();
		}
	
		SetMsgHandled(false);

		EMULE_CATCH
	}

	void OnLButtonUp(UINT nFlags, CPoint point)
	{
		ReleaseCapture();
		EndDialog(m_nExitCode);
		m_bClosingDown = true;
		SetMsgHandled(false);
	}

	void OnRButtonDown(UINT nFlags, CPoint point)
	{
		CRect rcClient;
		GetClientRect(rcClient);

		if (point.x <= rcClient.left || point.x >= rcClient.right || 
			point.y <= rcClient.top  || point.y >= rcClient.bottom)
		{
			ReleaseCapture();
			EndDialog(m_nExitCode);
			m_bClosingDown = true;
		}

		SetMsgHandled(false);
	}

	void OnKillFocus(HWND hWnd)
	{
		if (!m_bClosingDown)
		{
			ReleaseCapture();
			EndDialog(m_nExitCode);
			m_bClosingDown = true;
		}

		SetMsgHandled(false);
	}

	void OnShowWindow(BOOL bShow, UINT nStatus)
	{
		if (!bShow && !m_bClosingDown)
		{
			ReleaseCapture();
			EndDialog(m_nExitCode);
			m_bClosingDown = true;
		}
		else if(bShow)
		{
			SetCapture();
		}

		SetMsgHandled(false);
	}

	void OnCaptureChanged(HWND hWnd)
	{
		if (hWnd && hWnd != m_hWnd && !::IsChild(m_hWnd, hWnd))
		{	
			EndDialog(m_nExitCode);
			m_bClosingDown = true;
		}	
		
		SetMsgHandled(false);
	}

	void OnCommand(UINT nCmd, UINT nID, HWND hWnd)
	{
		if (nCmd == BN_CLICKED)
		{
			ReleaseCapture();
			m_nExitCode = nID;
			EndDialog(m_nExitCode);
			m_bClosingDown = true;
		}

		SetMsgHandled(false);
	}

	void OnHScroll(UINT nSBCode, short nPos, HWND hWnd)
	{
		EMULE_TRY

		if (hWnd == m_ctrlDownSpeedSld.m_hWnd)
		{
			m_nDownSpeed = m_ctrlDownSpeedSld.GetPos();
			DoDataExchange(false, IDC_DOWNTXT);
			SetMsgHandled(true);
		}
		else if (hWnd == m_ctrlUpSpeedSld.m_hWnd)
		{
			m_nUpSpeed = m_ctrlUpSpeedSld.GetPos();
			DoDataExchange(false, IDC_UPTXT);
			SetMsgHandled(true);
		}
		else
		{
			SetMsgHandled(false);
		}

		EMULE_CATCH
	}

	void OnChangeDownTxt(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
		EMULE_TRY

		static bool bCDTRecursionfix = false;
		if(bCDTRecursionfix)
			return;
		bCDTRecursionfix = true;				
		//----------------------------------------------------------------------

		DoDataExchange(true, IDC_DOWNTXT);
		
		if (m_nDownSpeed < 1)
			m_nDownSpeed = 1;
		if (m_nDownSpeed > m_nMaxDown)
			m_nDownSpeed = m_nMaxDown;

		// ToDo
		/*
		if(g_eMuleApp.m_pGlobPrefs->GetMaxGraphDownloadRate() == UNLIMITED)	//Cax2 - shouldn't be anymore...
		{
			if(m_nDownSpeedTxt > 64)		//Cax2 - why 64 ???
				m_nDownSpeedTxt = 64;
		} 
		else 
		{
			if(m_nDownSpeedTxt > g_eMuleApp.m_pGlobPrefs->GetMaxGraphDownloadRate())
				m_nDownSpeedTxt = g_eMuleApp.m_pGlobPrefs->GetMaxGraphDownloadRate();
		}
		*/

		m_ctrlDownSpeedSld.SetPos(m_nDownSpeed);

		DoDataExchange(false, IDC_DOWNTXT);

		m_ctrlDownSpeedTxt.SetSel(-1);

		//----------------------------------------------------------------------
		bCDTRecursionfix = false;

		EMULE_CATCH
	}

	void OnChangeUpTxt(UINT uCode, int nCtrlID, HWND hwndCtrl)
	{
		EMULE_TRY

		static bool bCUTRecursionfix = false;
		if(bCUTRecursionfix)
			return;
		bCUTRecursionfix = true;
		//----------------------------------------------------------------------

		DoDataExchange(true, IDC_UPTXT);
			
		if (m_nUpSpeed < 1)
			m_nUpSpeed = 1;
		if (m_nUpSpeed > m_nMaxUp)
			m_nUpSpeed = m_nMaxUp;

		// ToDo
		if (m_nUpSpeed >= 10 /*&& g_eMuleApp.m_pGlobPrefs->LimitlessDownload()*/)
		{	
			HWND hFocus = GetFocus();
			m_ctrlDownSpeedTxt.EnableWindow(true);
			m_ctrlDownSpeedSld.EnableWindow(true);
			if(hFocus != GetFocus())
				::SetFocus(hFocus);
		} 
		else 
		{
			HWND hFocus = GetFocus();
			m_ctrlDownSpeedTxt.EnableWindow(false);
			m_ctrlDownSpeedSld.EnableWindow(false);
			if(hFocus != GetFocus())
				::SetFocus(hFocus);
		}
			
		// ToDo
		/*
		if(g_eMuleApp.m_pGlobPrefs->GetMaxGraphUploadRate() == UNLIMITED)
		{
			if(m_nUpSpeedTxt > 16)
				m_nUpSpeedTxt = 16;
		} 
		else 
		{
			if(m_nUpSpeedTxt > g_eMuleApp.m_pGlobPrefs->GetMaxGraphUploadRate())
				m_nUpSpeedTxt = g_eMuleApp.m_pGlobPrefs->GetMaxGraphUploadRate();
		}
		*/

		m_ctrlUpSpeedSld.SetPos(m_nUpSpeed);
		DoDataExchange(false, IDC_UPTXT);

		m_ctrlUpSpeedTxt.SetSel(-1);

		//----------------------------------------------------------------------
		bCUTRecursionfix = false;	

		EMULE_CATCH
	}
};

#endif	// #ifndef __TRAYMENUDLG_H__4618ADE8_3C45_46bd_ADF6_5080FF7D2326__INCLUDED_
