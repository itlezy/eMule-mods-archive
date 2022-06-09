// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TrayIconImpl.h"
#include "TrayMenuDlg.h"
#include "MeterIcon.h"
#include "../Engine/Sockets/TasksSockets.h"
#include "../Engine/XML/XML.h"

class CConnectDlg :
	 public CDialogImpl<CConnectDlg>
	,public CWinDataExchange<CConnectDlg>
{
public:
	enum { IDD = IDD_CONNECTDLG };
	CString	m_sAddr;
	int		m_nPort;

public:
	BEGIN_MSG_MAP(CConnectDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
	END_MSG_MAP()

	BEGIN_DDX_MAP(CConnectDlg)
		DDX_TEXT(IDC_ADDR, m_sAddr)
		DDX_INT(IDC_PORT, m_nPort)
	END_DDX_MAP()

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		m_sAddr = DEF_ADDR;
		m_nPort = DEF_PORT;

		CenterWindow();

		// First DDX call, hooks up variables to controls.
		DoDataExchange(false);

		return TRUE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		EndDialog(wID);
		return 0;
	}
	LRESULT OnBnClickedOk(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/);
};


class CMainDlg :
	public CDialogImpl<CMainDlg>,
	public CUpdateUI<CMainDlg>,
	public CMessageFilter,
	public CIdleHandler,
	public CTrayIconImpl<CMainDlg>
{
public:
	enum { IDD = IDD_MAINDLG };

protected:
	UINT	m_nTimerID;
	bool	m_bConnected;

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg)
	{
		return IsDialogMessage(pMsg);
	}

	virtual BOOL OnIdle()
	{
		return FALSE;
	}

	BEGIN_UPDATE_UI_MAP(CMainDlg)
	END_UPDATE_UI_MAP()

	BEGIN_MSG_MAP(CMainDlg)
		MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_EXIT, OnCancel)
		COMMAND_ID_HANDLER(IDC_CONNECT, OnConnect)
		MESSAGE_HANDLER(WM_UI_STATE, OnUIState)
		MSG_WM_TIMER(OnTimer)	// for testing purpose only
		CHAIN_MSG_MAP(CTrayIconImpl<CMainDlg>)
	END_MSG_MAP()

// Handler prototypes (uncomment arguments if needed):
//	LRESULT MessageHandler(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
//	LRESULT CommandHandler(WORD /*wNotifyCode*/, WORD /*wID*/, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
//	LRESULT NotifyHandler(int /*idCtrl*/, LPNMHDR /*pnmh*/, BOOL& /*bHandled*/)

	LRESULT OnInitDialog(UINT /*uMsg*/, WPARAM /*wParam*/, LPARAM /*lParam*/, BOOL& /*bHandled*/)
	{
		EMULE_TRY

		if(!g_stEngine.Init(m_hWnd))
			return FALSE;

		g_stEngine.m_hMainWnd = m_hWnd;

		// center the dialog on the screen
		CenterWindow();

		// set icons
		HICON hIcon = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
										 IMAGE_ICON, ::GetSystemMetrics(SM_CXICON), ::GetSystemMetrics(SM_CYICON),
										 LR_DEFAULTCOLOR);
		SetIcon(hIcon, TRUE);

		HICON hIconSmall = (HICON)::LoadImage(_Module.GetResourceInstance(), MAKEINTRESOURCE(IDR_MAINFRAME),
												IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON),
												::GetSystemMetrics(SM_CYSMICON), LR_DEFAULTCOLOR);

		SetIcon(hIconSmall, FALSE);

		// register object for message filtering and idle updates
		CMessageLoop		*pLoop = _Module.GetMessageLoop();

		ATLASSERT(pLoop != NULL);
		pLoop->AddMessageFilter(this);
		pLoop->AddIdleHandler(this);

		UIAddChildWindowContainer(m_hWnd);

		// Install the tray icon
		HICON hIconTray = CreateMeterIcon(IDI_TRAYICON_DISCONNECTED, 16, 16, 10, 0, 0, 0);

		InstallTrayIcon(_T("eMule Plus"), hIconTray, NULL);

		m_bConnected = false;

		return TRUE;

		EMULE_CATCH
		return FALSE;
	}

	LRESULT OnOK(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CString		strXml;

		GetDlgItem(IDC_DEBUG_XML).GetWindowText(strXml.GetBuffer(1000), 1000);
		strXml.ReleaseBuffer();

		CXmlTask* pTask = CXmlTask::ParseXml(strXml);
		if(pTask)
		{
			pTask->SetReceived(false);
			pTask->SetClient(g_stEngine.m_pXmlClient);
			g_stEngine.Sockets.Push(pTask);		
		}
	//	Hide the dialog
		ShowWindow(SW_HIDE);
		return 0;
	}

	LRESULT OnCancel(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CloseDialog(wID);
		return 0;
	}

	LRESULT OnConnect(WORD /*wNotifyCode*/, WORD wID, HWND /*hWndCtl*/, BOOL& /*bHandled*/)
	{
		CConnectDlg dlg;
		if(dlg.DoModal() == IDOK)
		{
//			g_stEngine.AllocTcpConnect(inet_addr(dlg.m_sAddr), dlg.m_nPort, T_CLIENT_XML, NULL);
			CXml_hello *pCompletionTask = new CXml_hello;
			pCompletionTask->m_sClient = _T("tray");
			CTask_Connect *pTask = new CTask_Connect(dlg.m_sAddr, dlg.m_nPort, T_CLIENT_XML, pCompletionTask);
			g_stEngine.Sockets.Push(pTask);
		}

		return 0;
	}

	LRESULT OnUIState(UINT /*uMsg*/, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
	{
		bHandled = TRUE;
		switch(wParam)
		{
		case XML_CONNECTED:
			m_bConnected = true;
			// for testing purpose only
			m_nTimerID = SetTimer(1, 20);
			break;
		case XML_DISCONNECTED:
			m_bConnected = false;
			break;
		}

		return 0;
	}

	void CloseDialog(int nVal)
	{
		EMULE_TRY

		g_stEngine.Uninit();

		DestroyWindow();
		::PostQuitMessage(nVal);

		EMULE_CATCH
	}

	LRESULT TrayIconFct(UINT nMsg, UINT nHWP)
	{
		EMULE_TRY

		if (nMsg == WM_RBUTTONUP)
		{
			static bool bSingleInstance = true;
			if(bSingleInstance)
			{
				bSingleInstance = false;
				CPoint pos;
				GetCursorPos(&pos);
				if(m_bConnected)
				{
					CTrayMenuDlg dlg(pos, 16, 96, 8, 48);
					switch(dlg.DoModal(NULL))
					{
						case IDC_SPEED:			OnTraySpeed();		 break;
						case IDC_TOMAX:			OnTrayToMax();		 break;
						case IDC_TOMIN:			OnTrayToMin();		 break;
						case IDC_CONNECT:		OnTrayConnect();	 break;
						case IDC_DISCONNECT:	OnTrayDisconnect();	 break;
						case IDC_PREFERENCES:	OnTrayPreferences(); break;
						case IDC_ABOUT:			OnTrayAbout();		 break;
						case IDC_EXIT:			OnTrayExit();		 break;
						default:									 break;
					}
				}
				else
				{
					// Load the menu
					CMenu oMenu;
					if (!oMenu.LoadMenu(IDR_POPUP))
						return 0;
					// Get the sub-menu
					CMenuHandle oPopup(oMenu.GetSubMenu(0));
					// Prepare
					PrepareMenu(oPopup);
					// Get the menu position
					CPoint pos;
					GetCursorPos(&pos);
					// Make app the foreground
					SetForegroundWindow(m_hWnd);
					// Set the default menu item
/*					if (m_nDefault == 0)
						oPopup.SetMenuDefaultItem(0, TRUE);
					else
						oPopup.SetMenuDefaultItem(m_nDefault);*/
					// Track
					oPopup.TrackPopupMenu(TPM_LEFTALIGN, pos.x, pos.y, m_hWnd);
					// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
					PostMessage(WM_NULL);
					// Done
					oMenu.DestroyMenu();
				}
				bSingleInstance = true;
			}
		}
		else if (nMsg == WM_LBUTTONDBLCLK)
		{
			OnTrayAbout();
		}
		return 0;

		EMULE_CATCH
		return -1;
	}

	void OnTraySpeed()
	{
		::MessageBox(NULL, _T("Speed"), _T(""), MB_OK);
	}
	void OnTrayToMax()
	{
		::MessageBox(NULL, _T("ToMax"), _T(""), MB_OK);
	}
	void OnTrayToMin()
	{
		::MessageBox(NULL, _T("ToMin"), _T(""), MB_OK);
	}
	void OnTrayConnect()
	{
		CXml_connect* pTask = new CXml_connect;
		if(pTask)
		{
			pTask->SetClient(g_stEngine.m_pXmlClient);
			pTask->m_sAddr = _T("62.241.53.15");
			pTask->m_dwPort = 4242;
			g_stEngine.Sockets.Push(pTask);
		}
	}
	void OnTrayDisconnect()
	{
		CXml_disconnect* pTask = new CXml_disconnect;
		if(pTask)
		{
			pTask->SetClient(g_stEngine.m_pXmlClient);
			g_stEngine.Sockets.Push(pTask);
		}
	}
	void OnTrayPreferences()
	{
		::MessageBox(NULL, _T("Preferences"), _T(""), MB_OK);
	}
	void OnTrayAbout()
	{
		ShowWindow(SW_SHOW);
	}
	void OnTrayExit()
	{
		CXml_quit* pTask = new CXml_quit;
		pTask->SetClient(g_stEngine.m_pXmlClient);
		g_stEngine.Sockets.Push(pTask);
		m_bConnected = false;
//		CloseDialog(IDOK);
	}

	// for testing purpose only
	void OnTimer(UINT idEvent, TIMERPROC /*TimerProc*/)
	{
		if(idEvent == m_nTimerID)
		{
			if(!m_bConnected)
			{
				KillTimer(idEvent);
				HICON hIconTray = CreateMeterIcon(IDI_TRAYICON_DISCONNECTED, 16, 16, 10, 0, 0, 0);
				ChangeTrayIcon(hIconTray);
				return;
			}
			static int maxval = 32;
			static int val = 0;
			static int dir = 1;

			HICON hIcon = CreateMeterIcon(IDI_TRAYICON_STD, 16, 16,
										  maxval, val,
										  RGB(0,255,0), RGB(39,95,39));
			val += dir;
			if(val <= 0 || val >= maxval)
				dir = -dir;

			ChangeTrayIcon(hIcon);
			CString strTooltip;
			strTooltip.Format(_T("Up: %d.%02d (%d.%02d) | Down: %d.%02d (%d.%02d)"),
								rand()%33, rand()%100, rand()%2, rand()%100,
								val, rand()%100, rand()%2, rand()%100);
			SetTooltipText(strTooltip);
		}
		return;
	}
};
