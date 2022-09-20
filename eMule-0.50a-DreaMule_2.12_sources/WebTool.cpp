// WebTool.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "WebTool.h"
#include ".\webtool.h"


// CWebTool dialog

IMPLEMENT_DYNAMIC(CWebTool, CDialog)
CWebTool::CWebTool(CWnd* pParent /*=NULL*/)
	: CDialog(CWebTool::IDD, pParent)
{
}

CWebTool::~CWebTool()
{
}

void CWebTool::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, IDC_BACK , m_ButtonBack);
	DDX_Control(pDX, IDC_NEXT , m_ButtonNext);
	DDX_Control(pDX, IDC_STOP , m_ButtonStop);
	DDX_Control(pDX, IDC_REFRESH , m_ButtonRefresh);
	DDX_Control(pDX, IDC_GOHOME , m_ButtonGoHome);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CWebTool, CDialog)
	ON_BN_CLICKED(IDC_BACK, OnBnClickedBack)
	ON_BN_CLICKED(IDC_NEXT, OnBnClickedNext)
	ON_BN_CLICKED(IDC_STOP, OnBnClickedStop)
	ON_BN_CLICKED(IDC_REFRESH, OnBnClickedRefresh)
	ON_BN_CLICKED(IDC_GOHOME, OnBnClickedGoHome)
	ON_WM_SIZE()
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CWebTool message handlers

void CWebTool::OnBnClickedBack()
{
	// TODO: Add your control notification handler code here
	((CemuleDlg*)(AfxGetMainWnd()))->browser->GoBack();
}

void CWebTool::OnBnClickedNext()
{
	// TODO: Add your control notification handler code here
	((CemuleDlg*)(AfxGetMainWnd()))->browser->GoForward();
}

void CWebTool::OnBnClickedStop()
{
	// TODO: Add your control notification handler code here
	((CemuleDlg*)(AfxGetMainWnd()))->browser->Stop();
}

void CWebTool::OnBnClickedRefresh()
{
	// TODO: Add your control notification handler code here
	((CemuleDlg*)(AfxGetMainWnd()))->browser->Refresh();
}

void CWebTool::OnBnClickedGoHome()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	((CemuleDlg*)(AfxGetMainWnd()))->browser->GoHome();
	//CRect a(0,0,0,0);
	//((CemuleDlg*)(AfxGetMainWnd()))->browser->Create(NULL , NULL , WS_VISIBLE | WS_CHILD , a, this , AFX_IDW_PANE_FIRST , NULL );
	//((CemuleDlg*)(AfxGetMainWnd()))->browser->Navigate2(_T("www.donkeyhote2.com") , NULL , NULL );
	((CemuleDlg*)(AfxGetMainWnd()))->browser->Navigate2(_T("http://pootzmod.sourceforge.net/212user.html") , NULL , NULL );
	
	
}
void CWebTool::OnSize(UINT nType, int cx, int cy)
{
	CRect rcClient;
	CDialog::OnSize(nType, cx, cy);
	::AfxGetMainWnd()->GetClientRect(&rcClient);
	SetWindowPos(NULL, rcClient.left+9, rcClient.top+63, rcClient.Width()-18, 40 , SWP_NOZORDER);
	// TODO: Add your message handler code here
	//각 버튼위치 지정
	m_ButtonBack.SetWindowPos(NULL , 0 , 0 , 0 , 0 , SWP_NOSIZE ); //뒤로 버튼
	m_ButtonNext.SetWindowPos(NULL , 40 , 0 , 0 , 0 , SWP_NOSIZE ); //앞으로 버튼
	m_ButtonStop.SetWindowPos(NULL , 80 , 0 , 0 , 0 , SWP_NOSIZE ); //멈춤 버튼
	m_ButtonRefresh.SetWindowPos(NULL , 120 , 0 , 0 , 0 , SWP_NOSIZE ); //새로고침 버튼
	m_ButtonGoHome.SetWindowPos(NULL , 160 , 0 , 0 , 0 , SWP_NOSIZE ); //홈 버튼

}

void CWebTool::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: Add your message handler code here
	// Do not call CDialog::OnPaint() for painting messages
	CRect rc;
	GetClientRect(&rc);

	CDC rDC;
	rDC.CreateCompatibleDC(&dc);
	rDC.SelectObject( ((CemuleDlg*)(AfxGetMainWnd()))->m_Image);

	dc.StretchBlt( 0 , 0 , rc.Width() , rc.Height() , &rDC , 0 , 165 , 7  , 40 , SRCCOPY );  
}

BOOL CWebTool::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  Add extra initialization here
	m_ButtonBack.SetSkin( IDB_BACK1 , IDB_BACK3 , IDB_BACK2 , IDB_BACK1 , 0 , 0 , 0 , 0 , 0 );
	m_ButtonNext.SetSkin( IDB_NEXT1 , IDB_NEXT3 , IDB_NEXT2 , IDB_NEXT1 , 0 , 0 , 0 , 0 , 0 );
	m_ButtonRefresh.SetSkin( IDB_REFRESH1 , IDB_REFRESH3 , IDB_REFRESH2 , IDB_REFRESH1 , 0 , 0 , 0 , 0 , 0 );
	m_ButtonGoHome.SetSkin( IDB_GOHOME1 , IDB_GOHOME3 , IDB_GOHOME2 , IDB_GOHOME1 , 0 , 0 , 0 , 0 , 0 );
	m_ButtonStop.SetSkin( IDB_STOP1 , IDB_STOP3 , IDB_STOP2 , IDB_STOP1 , 0 , 0 , 0 , 0 , 0 );


	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
