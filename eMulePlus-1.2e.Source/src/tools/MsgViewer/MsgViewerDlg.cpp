// MsgViewerDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MsgViewer.h"
#include "MsgViewerDlg.h"
#include "ProcessFileDlg.h"
#include "MsgViewDlg.h"
#include "FilterDlg.h"
#include "msgviewerdlg.h"
#include "PreviewDlg.h"
#include "msgviewerdlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgViewerDlg dialog

const TCHAR CMsgViewerDlg::s_szSection[] = _T("AppShareViewer");

CMsgViewerDlg::CMsgViewerDlg(CWnd* pParent /*=NULL*/)
	: CSizeableDlg(CMsgViewerDlg::IDD, pParent),
	m_stRecentList(0, s_szSection, _T("File%d"), 0x100)
{
	//{{AFX_DATA_INIT(CMsgViewerDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMsgViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgViewerDlg)
	DDX_Control(pDX, IDC_SPIN, m_wndSpin);
	DDX_Control(pDX, IDC_FILTERSTATS, m_wndFilterStats);
	DDX_Control(pDX, IDC_FILTERPROGRESS, m_wndFilterProgress);
	DDX_Control(pDX, IDC_FILENAMECOMBO, m_wndFileNameCombo);
	DDX_Control(pDX, IDC_SLIDER, m_wndSlider);
	DDX_Control(pDX, IDC_LIST, m_wndList);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CMsgViewerDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgViewerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_LOAD, OnLoad)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_NOTIFY(NM_DBLCLK, IDC_LIST, OnDblclkList)
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_NOTIFY(NM_RELEASEDCAPTURE, IDC_SLIDER, OnReleasedcaptureSlider)
	ON_CBN_SELENDOK(IDC_FILENAMECOMBO, OnSelendokFilenamecombo)
	ON_BN_CLICKED(IDC_FILTER, OnFilter)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN, OnDeltaposSpin)
	ON_WM_DROPFILES()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgViewerDlg message handlers

BOOL CMsgViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// modify list control's style
	ListView_SetExtendedListViewStyleEx(m_wndList, LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT, LVS_EX_HEADERDRAGDROP | LVS_EX_FULLROWSELECT);

	VERIFY(-1 != m_wndList.InsertColumn(0, _T("No")));
	VERIFY(-1 != m_wndList.InsertColumn(1, _T("Time")));
	VERIFY(-1 != m_wndList.InsertColumn(2, _T("Size")));
	VERIFY(-1 != m_wndList.InsertColumn(3, _T("MsgID")));
	VERIFY(-1 != m_wndList.InsertColumn(4, _T("Protocol")));
	VERIFY(-1 != m_wndList.InsertColumn(5, _T("ClientID")));
	VERIFY(-1 != m_wndList.InsertColumn(6, _T("Client Type")));
	VERIFY(-1 != m_wndList.InsertColumn(7, _T("Msg description")));
	
	// stretch columns
	for (int nIndex = 0; nIndex < 8; nIndex++)
	{
		CString strEntry;
		strEntry.Format(_T("Col%d"), nIndex);

		UINT nSize = AfxGetApp()->GetProfileInt(s_szSection, strEntry, 50);
		m_wndList.SetColumnWidth(nIndex, nSize);
	}

	CRect stRect;
	GetClientRect(&stRect);
	OnSize(0xFFFFFFFF, stRect.right, stRect.bottom); // updates things

	// apply the stored size
	UINT nSize = 0;
	PBYTE pData = NULL;

	// insert to the combo the MRU list
	m_stRecentList.ReadList();
	for (nIndex = 0; nIndex < m_stRecentList.GetSize(); nIndex++)
	{
		CString strItem = m_stRecentList[nIndex];
		if (!strItem.IsEmpty())
			VERIFY(m_wndFileNameCombo.AddString(m_stRecentList[nIndex]) != LB_ERR);
	}

	g_stFilter.Open();

	if (AfxGetApp()->GetProfileBinary(s_szSection, _T("Dlg"), &pData, &nSize) && (sizeof(WINDOWPLACEMENT) == nSize))
	{
		WINDOWPLACEMENT* pPlacement = (WINDOWPLACEMENT*) pData;
		ASSERT(pPlacement);

		MoveWindow(&pPlacement->rcNormalPosition, FALSE);
		if (SW_SHOWMAXIMIZED == pPlacement->showCmd)
			ShowWindow(SW_SHOWMAXIMIZED);

		delete[] pData;
	}

	// process command line parameter
	CString strFilePath = AfxGetApp()->m_lpCmdLine;
	if (!strFilePath.IsEmpty())
	{
		// check if this string has is enclosed within \" characters.
		if (_T('"') == strFilePath[0])
		{
			int nLen = strFilePath.GetLength();
			if ((nLen >= 2) && (_T('"') == strFilePath[nLen - 1]))
				strFilePath = strFilePath.Mid(1, nLen - 2);
		}
	}

	TryOpen(strFilePath);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CMsgViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CMsgViewerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMsgViewerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CMsgViewerDlg::OpenFile(PCTSTR szFileName)
{
	m_wndList.DeleteAllItems();

	m_wndSlider.ShowWindow(SW_HIDE);
	m_wndSlider.SetPos(0);
	m_wndFilterStats.SetWindowText(_T(""));
	m_wndFilterProgress.SetRange32(0, 0);
	m_wndFilterProgress.SetPos(0);
	m_wndSpin.ShowWindow(SW_HIDE);
	m_wndSlider.ShowWindow(SW_HIDE);

	if (!szFileName || !*szFileName)
		return TRUE; // no really file parsing required

	CString strFileName = szFileName;
	CProcessFileDlg Dlg(m_stFileInfo, strFileName, this);
	if (IDOK != Dlg.DoModal())
		return FALSE;

	DWORD dwItems = m_stFileInfo.m_arrMsgData.GetSize();
	if (!dwItems && m_stFileInfo.m_64nSizeFile && !m_stFileInfo.m_dwFilteredOut)
	{
		// nothing could be read although the file is not empty
		m_stFileInfo.Close();

		AfxMessageBox(_T("Invalid file format"));
		return FALSE;
	}

	// now - update the slider /spin range / offset / etc.
	DWORD dwPages = dwItems / MAX_VIEWABLE + (0 != (dwItems  % MAX_VIEWABLE));
	if (dwPages > 1)
	{
		m_wndSlider.SetRange(0, dwPages - 1);
		m_wndSpin.SetRange32(0, dwPages - 1);
		m_wndSlider.ShowWindow(SW_SHOW);
		m_wndSpin.ShowWindow(SW_SHOW);
	}
	
	// update the filter statistics
	m_wndFilterProgress.SetRange32(0, dwItems + m_stFileInfo.m_dwFilteredOut);
	m_wndFilterProgress.SetPos(dwItems);

	CString strText;
	strText.Format(_T("%u / %u"), dwItems, m_stFileInfo.m_dwFilteredOut + dwItems);
	m_wndFilterStats.SetWindowText(strText);

	// show first portion
	OnReleasedcaptureSlider(NULL, NULL);

	return TRUE;
}


void CMsgViewerDlg::OnLoad() 
{
	TCHAR szFileName[MAX_PATH] = _T("");

	OPENFILENAME stOFN;
	ZeroMemory(&stOFN, sizeof(stOFN));
	stOFN.lStructSize = sizeof(stOFN);
	stOFN.hwndOwner = m_hWnd;
	stOFN.hInstance = AfxGetInstanceHandle();
	stOFN.lpstrFile = szFileName;
	stOFN.nMaxFile = MAX_PATH;
	stOFN.lpstrTitle = _T("Open  AppShare File");
	stOFN.Flags = OFN_ENABLESIZING | OFN_FILEMUSTEXIST;
	stOFN.lpstrFilter = _T("eMule(*.em)\0*.em\0All Files (*.*)\0*.*\0");
	if (GetOpenFileName(&stOFN))
		TryOpen(szFileName);
}

void CMsgViewerDlg::TryOpen(PCTSTR szFileName)
{
	if (OpenFile(szFileName) && *szFileName)
	{
		int nItem = m_wndFileNameCombo.FindString(-1, szFileName);
		if (CB_ERR == nItem)
		{
			nItem = m_wndFileNameCombo.InsertString(0, szFileName);
			ASSERT(CB_ERR != nItem);
			m_stRecentList.Add(szFileName);
		}
		VERIFY(m_wndFileNameCombo.SetCurSel(nItem) != CB_ERR);
	}
}

void CMsgViewerDlg::OnDestroy() 
{
	OpenFile(NULL); // close all stuff

	CDialog::OnDestroy();
	
	// save the last opened file
	m_stRecentList.WriteList(); // this dummy thing erases the whole profile!

	// save the filter state
	g_stFilter.Save();

	// save the columns info
	for (int nIndex = 0; nIndex < 8; nIndex++)
	{
		CString strEntry;
		strEntry.Format(_T("Col%d"), nIndex);

		int nWidth = m_wndList.GetColumnWidth(nIndex);
		VERIFY(AfxGetApp()->WriteProfileInt(s_szSection, strEntry, nWidth));
	}

	// save the whole dialog size
	WINDOWPLACEMENT stWP;
	stWP.length = sizeof(stWP);
	VERIFY(GetWindowPlacement(&stWP));

	VERIFY(AfxGetApp()->WriteProfileBinary(s_szSection, _T("Dlg"), (PBYTE) &stWP, sizeof(stWP)));
}

void CMsgViewerDlg::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult) 
{
	OnOK();	
	*pResult = 0;
}

void CMsgViewerDlg::OnRefresh() 
{
	if (INVALID_HANDLE_VALUE != m_stFileInfo.m_hFile)
	{
		CString strFileName;
		m_wndFileNameCombo.GetWindowText(strFileName);
		if (!strFileName.IsEmpty())
			OpenFile(strFileName);
	}
}

void CMsgViewerDlg::OnReleasedcaptureSlider(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_wndList.DeleteAllItems();

	// get current slider position
	int nPos = m_wndSlider.GetPos();
	if (pNMHDR)
		m_wndSpin.SetPos(nPos); // same as ours

	VERIFY(m_wndList.LockWindowUpdate());
	// and now - initialize list view items
	m_stFileInfo.FillListCtrl(m_wndList, nPos * MAX_VIEWABLE, MAX_VIEWABLE);
	m_wndList.UnlockWindowUpdate();

	if (pResult)
		*pResult = 0;
}

void CMsgViewerDlg::OnSelendokFilenamecombo() 
{
	CString strItem;
	m_wndFileNameCombo.GetWindowText(strItem);

	if (!OpenFile(strItem))
	{
		// remove this dummy item
		int nItem = m_wndFileNameCombo.GetCurSel();
		ASSERT(CB_ERR != nItem);

		VERIFY(m_wndFileNameCombo.DeleteString(nItem) != CB_ERR);

		// remove the item with the same index from the MRU list
		m_stRecentList.Remove(nItem);

		OpenFile(NULL); // simply close all stuff
	}
}

void CMsgViewerDlg::OnFilter() 
{
	CFilterDlg Dlg;
	if (Dlg.DoModal() == IDOK)
		OnRefresh();
	else
		g_stFilter.Open(); // restore the previous state
}

void CMsgViewerDlg::OnSize(UINT nType, int cx, int cy) 
{
	if (0xFFFFFFFF != nType)
		CDialog::OnSize(nType, cx, cy);

	OnSizeControl(m_wndList, cx, cy, 10);
	OnSizeControl(m_wndSlider, cx, cy, 2);
	OnSizeControl(IDC_LOAD, cx, cy, 3);
	OnSizeControl(IDC_REFRESH, cx, cy, 3);
	OnSizeControl(m_wndFileNameCombo, cx, cy, 2);
	OnSizeControl(IDC_FILTER, cx, cy, 15);
	OnSizeControl(m_wndFilterProgress, cx, cy, 14);
	OnSizeControl(IDC_FILTERSTATS, cx, cy, 12);
	OnSizeControl(IDC_STATSCAPTION, cx, cy, 12);
	OnSizeControl(m_wndSpin, cx, cy, 3);

}

void CMsgViewerDlg::OnDeltaposSpin(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NM_UPDOWN* pNMUpDown = (NM_UPDOWN*)pNMHDR;
	
	m_wndSlider.SetPos(pNMUpDown->iPos + pNMUpDown->iDelta);
	OnReleasedcaptureSlider(NULL, NULL);

	*pResult = 0;
}

void CMsgViewerDlg::OnDropFiles(HDROP hDropInfo) 
{
	TCHAR szPath[MAX_PATH];
	UINT nRes = DragQueryFile(hDropInfo, 0, szPath, MAX_PATH);
	DragFinish(hDropInfo);

	if (nRes)
		TryOpen(szPath);

	CDialog::OnDropFiles(hDropInfo);
}

void CMsgViewerDlg::OnOK()
{
	// check at which position the doubble-click was
	CPoint stPoint;
	VERIFY(GetCursorPos(&stPoint));
	m_wndList.ScreenToClient(&stPoint);
	stPoint.x = 2;

	int nItem = m_wndList.HitTest(stPoint);
	if (-1 != nItem)
	{
		ASSERT(INVALID_HANDLE_VALUE != m_stFileInfo.m_hFile);

		// retreive the item data
		DWORD dwMsgIndex = m_wndList.GetItemData(nItem);
		const MSG_FILE_INFO::MSG_DATA& stData = m_stFileInfo.m_arrMsgData.GetAt(dwMsgIndex);

		DWORD dwSize = 0;
		PVOID pBuf = m_stFileInfo.ReadMsg(dwMsgIndex, dwSize);

		if (pBuf)
		{
			CMsgViewDlg Dlg(pBuf, dwSize, stData.m_n64Pos, stData.m_nIdProtoType, this);
			Dlg.DoModal();
			delete[] pBuf;
		}
	}
}

