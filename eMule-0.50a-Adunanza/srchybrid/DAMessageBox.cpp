// DAMessageBox.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "otherfunctions.h"
#include "DAMessageBox.h"
#include ".\damessagebox.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CDAMessageBox, CDialog)

BEGIN_MESSAGE_MAP(CDAMessageBox, CDialog)
	ON_WM_PAINT()
	ON_BN_CLICKED(IDC_DA_OK, OnBnClickedFatto)
	ON_BN_CLICKED(IDC_BUTTON1, OnBnClickedButton1)
END_MESSAGE_MAP()

CDAMessageBox::CDAMessageBox(CWnd* pParent, LPCTSTR cap, BOOL da, BOOL link)
	: CDialog(CDAMessageBox::IDD, pParent)
{

	caption = cap;
	enableda = da;
	enablelink = link;

}

CDAMessageBox::~CDAMessageBox()
{
	if (m_icnWnd)
		VERIFY( DestroyIcon(m_icnWnd) );
}

void CDAMessageBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CDAMessageBox::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);

	SetIcon(m_icnWnd = theApp.LoadIcon(_T("Info")), FALSE);

	CheckDlgButton(IDC_DA_CHECK, false);

	VERIFY( m_iconIn.Attach(theApp.LoadImage(_T("Info"), _T("JPG"))) );

	GetDlgItem(IDC_DA_TESTO)->SetWindowText(caption);

	extern bool adu_controllo;
	if(adu_controllo)
		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("Per saperne di più"));
	else
		GetDlgItem(IDC_BUTTON1)->SetWindowText(_T("Dettagli e Informazioni"));

	if (!enableda)
		GetDlgItem(IDC_DA_CHECK)->ShowWindow(SW_HIDE);

	if (enablelink)
		GetDlgItem(IDC_BUTTON1)->ShowWindow(true);

	Localize();

	SetActiveWindow();
	return true;
}

void CDAMessageBox::OnBnClickedFatto()
{
	if (IsDlgButtonChecked(IDC_DA_CHECK))
		CDialog::OnOK();
	else
		CDialog::OnCancel();
}

void CDAMessageBox::Localize(void){ }

void CDAMessageBox::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	if (m_iconIn.GetSafeHandle())
	{
		CDC dcMem;
		if (dcMem.CreateCompatibleDC(&dc))
		{
			CBitmap* pOldBM = dcMem.SelectObject(&m_iconIn);
			BITMAP BM;
			m_iconIn.GetBitmap(&BM);
			dc.BitBlt(16, 75, BM.bmWidth, BM.bmHeight, &dcMem, 0, 0, SRCCOPY);
			if (pOldBM)
				dcMem.SelectObject(pOldBM);
		}
	}
}

void CDAMessageBox::OnBnClickedButton1()
{
	//tigerjact aggiungo link
	//Anis aggiungo secondo link
	extern bool adu_controllo;
	if(adu_controllo)
		ShellExecute(NULL, _T("open"), _T("http://forum.adunanza.net/forum.php"),NULL, NULL, SW_SHOW);
	else
		ShellExecute(NULL, _T("open"), _T("http://aduteca.adunanza.net/Emule_AdunanzA/FAQ_Domande_e_Risposte_Frequenti/AduTips"),NULL, NULL, SW_SHOW);
	OnBnClickedFatto();
}


