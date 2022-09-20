// WebBrowser.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "WebBrowser.h"
#include "otherfunctions.h"
#include "Preferences.h"
#include ".\webbrowser.h"
#include "langids.h" //>>> WiZaRd::Lang Reduction

#define VERYCD_HOMEPAGE _T("http://www.dreamule.org/versao/30beta6.php")
//#define VERYCD_HOMEPAGE _T("about:blank")
//#define POST_FORM_HEADER _T("Content-Type: application/x-www-form-urlencoded\r\n")

// CWebBrowserWnd 对话框

IMPLEMENT_DYNAMIC(CWebBrowserWnd, CDialog)
CWebBrowserWnd::CWebBrowserWnd(CWnd* pParent /*=NULL*/)
	: CResizableDialog(CWebBrowserWnd::IDD, pParent)
{
	m_pExplorer = NULL;
	//m_pExplorer = new CHtmlCtrl(); // VC-SearchDream[2006-12-25]: Move to OnInitDialog 
}

CWebBrowserWnd::~CWebBrowserWnd()
{
	 if (m_pExplorer)
	 {
		 m_pExplorer = NULL, delete m_pExplorer;
	 }
}

void CWebBrowserWnd::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CUSTOM_BAR, m_toolbar);
	DDX_Control(pDX, IDC_COMBO_ADDRESS, m_addressbox);
	DDX_Control(pDX, IDC_STATIC_WEBINFO, m_status);
}


BEGIN_MESSAGE_MAP(CWebBrowserWnd, CResizableDialog)
	ON_CBN_SELENDOK(IDC_COMBO_ADDRESS,OnNewAddress)
	ON_WM_KEYDOWN()

	ON_COMMAND( TB_BACK, OnBack )
	ON_COMMAND( TB_FORWARD, OnForward )
	ON_COMMAND( TB_STOP, OnStop )
	ON_COMMAND( TB_HOME, OnHomePage )
	ON_COMMAND( TB_REFRESH, OnRefresh )
	ON_WM_SIZE()
END_MESSAGE_MAP()


// CWebBrowserWnd 消息处理程序

BOOL CWebBrowserWnd::OnInitDialog()
{
	CResizableDialog::OnInitDialog();

	// VC-SearchDream[2006-12-25]: Move the following code from the tail of m_animate.Open(IDR_VERYEARTH)
	// Add Browser
	CRect toolRect,frameRect,addressRect,staticRect, statusRect, loaderRect;
	GetDlgItem(IDC_BROWSERFRAME)->GetWindowRect(&frameRect);
	GetDlgItem(IDC_BROWSERFRAME)->DestroyWindow();	
	::MapWindowPoints(NULL, m_hWnd, (LPPOINT)&frameRect, 2);

	

	// VC-SearchDream[2006-12-25]: Add try-catch
	try
	{
		m_pExplorer = new CHtmlCtrl(); // VC-SearchDream[2006-12-25]: Move from CWebBrowserWnd
		m_pExplorer->Create(NULL, NULL ,WS_CHILD | WS_VISIBLE &~WS_BORDER,frameRect,this, IDC_BROWSERFRAME,NULL);
		m_pExplorer->SetSilent(true);
	}
	catch(...)
	{
		if (m_pExplorer)
		{
			delete m_pExplorer;
			m_pExplorer = NULL;
		}

		//CString strError;
		//strError.Format(GetResString(IDS_CREATE_WEBBROWSER_FAIL), VERYCD_HOMEPAGE);
		DisableBrowser(IDS_CREATE_WEBBROWSER_FAIL);
		//ShellExecute(this->m_hWnd, _T("open"), CString(VERYCD_HOMEPAGE), _T(""), _T(""), SW_SHOW);

		return TRUE; // Directly Return
	}
	// VC-SearchDream[2006-12-25]: Move the up code from the tail of m_animate.Open(IDR_VERYEARTH)
	
	// Add Toolbar
	m_toolbar.Init();

	// Add Title
//	GetDlgItem(IDC_STATIC_BROWSER)->SetWindowText(GetResString(IDS_WEBBROWSER));  
	GetDlgItem(IDC_STATIC_ADDRESS)->SetWindowText(GetResString(IDS_ADDRESS));

	//m_browsericon = theApp.LoadIcon(_T("WEBBROWSER"));
	//((CStatic *)GetDlgItem(IDC_BROWSER_ICO))->SetIcon(m_browsericon);

	
	//工具条位置
	CSize size;
	m_toolbar.GetMaxSize(&size);
	m_toolbar.GetWindowRect(&toolRect);
	ScreenToClient(&toolRect);
	m_toolbar.MoveWindow(toolRect.left,toolRect.top,size.cx,toolRect.Height());
	
	// Set Static_Address Postion
	m_toolbar.GetWindowRect(&toolRect);
	GetDlgItem(IDC_STATIC_ADDRESS)->GetWindowRect(&staticRect);
	staticRect.left = toolRect.right;
	staticRect.right = staticRect.left + 65 ;//checar
	ScreenToClient(&staticRect);
	GetDlgItem(IDC_STATIC_ADDRESS)->MoveWindow(&staticRect);

	// Set Address Box Position
	m_addressbox.GetWindowRect(&addressRect);
	addressRect.left = toolRect.right + 65;
	ScreenToClient(&addressRect);
	m_addressbox.MoveWindow(&addressRect);
	m_addressbox.SetItemHeight(-1,160);

	// Set Window Style
	UINT uStyle = ::GetWindowLong(m_pExplorer->m_hWnd,GWL_EXSTYLE);
	uStyle |= WS_EX_STATICEDGE;
	::SetWindowLong(m_pExplorer->m_hWnd,GWL_EXSTYLE,uStyle);

	AddAnchor(GetDlgItem(IDC_STATIC_ADDRESS)->m_hWnd,TOP_LEFT,NOANCHOR);
	AddAnchor(m_addressbox.m_hWnd,TOP_LEFT,BOTTOM_RIGHT);
	AddAnchor(m_pExplorer->m_hWnd,TOP_LEFT,BOTTOM_RIGHT);
//	AddAnchor(GetDlgItem(IDC_VERYEARTH)->m_hWnd,TOP_RIGHT,NOANCHOR);
	AddAnchor(m_status.m_hWnd,BOTTOM_LEFT, BOTTOM_RIGHT);

	// Navigate Homepage
	m_pExplorer->SetSilent(true);// VC-linhai[2007-08-02]:
								 // 修改网页浏览器执行脚本时弹出调试窗口的Bug
								 // 修改后只会弹出一个提示窗口
								 // 如果完全不弹出执行Bug消息，需要在IE浏览器-〉Internet选项-〉高级，选中禁用脚本调试(其他)

	OnHomePage();

	return TRUE;
}

void CWebBrowserWnd::Localize(void)
{
	GetDlgItem(IDC_STATIC_BROWSER)->SetWindowText(GetResString(IDS_WEBBROWSER));  
	GetDlgItem(IDC_STATIC_ADDRESS)->SetWindowText(GetResString(IDS_ADDRESS));
	m_toolbar.Localize();

	if (::IsWindow(m_staticError.GetSafeHwnd()))
		m_staticError.SetWindowText(GetResString(m_uStridDisableReason));
}

void CWebBrowserWnd::SetAddress(LPCTSTR lpszURL)
{
	m_addressbox.SetWindowText(lpszURL);
}

void CWebBrowserWnd::StartAnimation()
{

}

void CWebBrowserWnd::StopAnimation()
{

}

void CWebBrowserWnd::OnNewAddress()
{
	CString str;
	m_addressbox.GetLBText(m_addressbox.GetCurSel(), str);
	m_pExplorer->Navigate2(str, 0, NULL);
}
void CWebBrowserWnd::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if(nChar == '\r')
	{
		OnNewAddressEnter();
	}

	CResizableDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

//Chocobo Start
//浏览器访问指定页面，added by Chocobo on 2006.08.07
//方便帮助链接在内置浏览器中显示
void CWebBrowserWnd::Navigate(LPCTSTR lpszURL)
{
	if (NULL == m_pExplorer)	return;
	m_pExplorer->Navigate2(lpszURL,0,0,0,0,0);
}
//Chocobo End

void CWebBrowserWnd::OnNewAddressEnter()
{

	CString str;
	m_addressbox.GetWindowText(str);
	
	m_pExplorer->Navigate2(str, 0, NULL);
	//if(!m_addressbox.FindString(0,str))
	//{
		
        m_addressbox.InsertString(0,str);
	//}
}

CString CWebBrowserWnd::ResourceToURL(LPCTSTR lpszURL)
{
	// This functions shows how to convert an URL to point
	// within the application
	// I only use it once to get the startup page

	CString m_strURL;
	HINSTANCE hInstance = AfxGetResourceHandle();
	ASSERT(hInstance != NULL);
	
	LPTSTR lpszModule = new TCHAR[_MAX_PATH];
	
	if (GetModuleFileName(hInstance, lpszModule, _MAX_PATH))
	{
		m_strURL.Format(_T("res://%s/%s"), lpszModule, lpszURL);
	}
	
	delete []lpszModule;

	return m_strURL;
}

void CWebBrowserWnd::OnBack()
{
	m_pExplorer->GoBack();
}

void CWebBrowserWnd::OnForward()
{
	m_pExplorer->GoForward();
}

void CWebBrowserWnd::OnStop()
{
	m_pExplorer->Stop();
}

void CWebBrowserWnd::OnHomePage()
{
//>>> WiZaRd::Lang Reduction
	CString strURL = VERYCD_HOMEPAGE;
	switch(thePrefs.GetLanguageID())
	{
		//TODO: add multiple urls
		case LANGID_DE_DE:
			strURL = _T("http://www.dreamule.org/versao/3.0_DE_DE.php");
			break;
		case LANGID_EN_US:
			strURL = _T("http://www.dreamule.org/versao/3.0_EN_US.php");
			break;
		case LANGID_ES_ES_T:
			strURL = _T("http://www.dreamule.org/versao/3.0_ES_ES_T.php");
			break;
		case LANGID_IT_IT:
			strURL = _T("http://www.dreamule.org/versao/3.0_IT_IT.php");
			break;
		case LANGID_PT_BR:
			strURL = _T("http://www.dreamule.org/versao/3.0_PT_BR.php");
			break;
		case LANGID_FR_FR:
			strURL = _T("http://www.dreamule.org/versao/3.0_FR_FR.php");
			break;
	}
//<<< WiZaRd::Lang Reduction
	m_pExplorer->Navigate2(strURL, NULL,NULL,NULL,NULL);
}

void CWebBrowserWnd::OnRefresh()
{
	StartAnimation();
    m_pExplorer->Refresh();
}

void CWebBrowserWnd::DisableBrowser(UINT uStridReason)
{
	GetDlgItem(IDC_STATIC_BROWSER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CUSTOM_BAR)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ADDRESS)->ShowWindow(SW_HIDE);	
	GetDlgItem(IDC_COMBO_ADDRESS)->ShowWindow(SW_HIDE);

	m_uStridDisableReason = uStridReason;

	CRect rect;
	GetClientRect(&rect);
	m_staticError.Create(NULL, WS_CHILD|WS_VISIBLE, rect, this, IDC_RUNTIMEERROR);
	m_staticError.SetWindowText(GetResString(m_uStridDisableReason));
	m_staticError.SetFont(&(theApp.m_fontLog));
}

void CWebBrowserWnd::OnSize(UINT nType, int cx, int cy)
{
	CResizableDialog::OnSize(nType, cx, cy);

	// TODO: 在此处添加消息处理程序代码
	if (::IsWindow(m_staticError.GetSafeHwnd()))
		m_staticError.CenterWindow();
}
