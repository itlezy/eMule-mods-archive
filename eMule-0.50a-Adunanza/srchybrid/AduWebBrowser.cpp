// AduWebBrowser.cpp : file di implementazione
// eMule AdunanzA Source Code File.

#include "stdafx.h"
#include "AduWebBrowser.h"
#include "RemoteSettings.h"
#include "emule.h"
#include "DAMessageBox.h"

bool haicliccato = false;
bool a[4] = {false, false, false, false};
//Anis Hireche module - AdunanzA 03/10/11

// finestra di dialogo AduWebBrowser

IMPLEMENT_DYNAMIC(AduWebBrowser, CDialog)

BEGIN_MESSAGE_MAP(AduWebBrowser, CResizableDialog)
	ON_WM_SHOWWINDOW()
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_EN_SETFOCUS(IDC_EDIT2, AduWebBrowser::OnEnSetfocusEdit2)
	ON_EN_KILLFOCUS(IDC_EDIT2, AduWebBrowser::OnEnKillfocusEdit2)
	ON_BN_CLICKED(IDC_BUTTON5, AduWebBrowser::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON4, AduWebBrowser::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON1, AduWebBrowser::OnBnClickedButton1)
END_MESSAGE_MAP()

AduWebBrowser::AduWebBrowser(CWnd* pParent /*=NULL*/) : CResizableDialog(AduWebBrowser::IDD, pParent)
{

}

AduWebBrowser::~AduWebBrowser()
{
}

void AduWebBrowser::StartTimer()
{
    SetTimer( 0x1000, 1000, 0 ); //Anis -> Ogni secondo faccio il controllo dello stato della pagina web.
}


void AduWebBrowser::OnSize(UINT nType, int cx, int cy) 
{
	//Anis -> che bordello...
	CDialog::OnSize(nType, cx, cy);
	
	CRect rect;
	GetClientRect(&rect);
	if(m_browser)
		m_browser.SetWindowPos(NULL, rect.left, rect.top + 27, rect.Width(), rect.Height() - 27, NULL);
	
	if(GetDlgItem(IDC_EDIT2)->GetSafeHwnd())
		GetDlgItem(IDC_EDIT2)->SetWindowPos(NULL,162, 2, rect.Width() - 164, 23, NULL);

}

void AduWebBrowser::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
}

BOOL AduWebBrowser::OnInitDialog() { 
	CResizableDialog::OnInitDialog();
	StartTimer();
	return true;
}

void AduWebBrowser::DoDataExchange(CDataExchange* pDX)
{
	DDX_Control(pDX, ADU_BROWSER, m_browser);
	CDialog::DoDataExchange(pDX);
}

BOOL AduWebBrowser::PreTranslateMessage(MSG* pMsg) 
{
		if(pMsg->wParam == 0x41)
			a[0] = true;
		else if(pMsg->wParam == 0x4E) 
			a[1] = true;
		else if(pMsg->wParam == 0x49)
			a[2] = true;
		else if(pMsg->wParam == 0x53)
			a[3] = true;
		else {
			a[0] = false;
			a[1] = false;
			a[2] = false;
			a[3] = false;
		}

		if(a[0] == true && a[1] == true && a[2] == true && a[3] == true) {
			for(int i = 0; i<4; i++)
				a[i] == false;
			CString msg;
			msg.Format(_T("EASTER EGG!\n\nSalve,\nSono Anis Hireche, programmatore della versione 3.18 di eMule AdunanzA.\nTi ringrazio per aver utilizzato software AduTeam e spero che tu ne sia rimasto soddisfatto, soprattutto in termini di qualità ed efficienza. Continua a supportarci mantenendo attivi i nostri sponsor: permetterai così di migliorare ulteriormente\nil software che stai usando. Sicuri della tua scelta, tutti noi ti ringraziamo\nanticipatamente per il tuo contributo, augurandoti un buon download.\n\n - Anis Hireche - Programmer at AduTeam"));
			CDAMessageBox easter_egg(NULL, msg, false, false);
			easter_egg.DoModal();
		}

		if (haicliccato) {
			if(pMsg->wParam == VK_RETURN) {
				COleVariant vEmpty;
				CString vURL;
				CEdit *edtFirstName;
				edtFirstName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT2));
				edtFirstName->GetWindowTextW(vURL);

				if(m_browser)
					m_browser.Navigate(vURL, vEmpty, vEmpty, vEmpty, vEmpty);	

				edtFirstName->SetWindowText((CString)vURL);
				m_browser.SetFocus();
				haicliccato = false;
				return true;
			}
	}
	return CDialog::PreTranslateMessage(pMsg);
}


BEGIN_EVENTSINK_MAP(AduWebBrowser, CResizableDialog)
	ON_EVENT(AduWebBrowser, ADU_BROWSER, 113, AduWebBrowser::TitleChangeBrowser, VTS_BSTR)
END_EVENTSINK_MAP()

void AduWebBrowser::TitleChangeBrowser(LPCTSTR Text)
{
	haicliccato = false;
	CEdit *edtFirstName;
	edtFirstName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT2));
	CString urlAttuale;
	urlAttuale = m_browser.GetLocationURL();
	edtFirstName->SetWindowText((CString)urlAttuale);
	m_browser.SetFocus();

}

void AduWebBrowser::OnEnSetfocusEdit2()
{
	haicliccato = true;
}

void AduWebBrowser::OnEnKillfocusEdit2()
{
	haicliccato = false;
}

void AduWebBrowser::OnBnClickedButton5()
{
	m_browser.Refresh();
	COleVariant vEmpty;
	CString vURL;
	CEdit *edtFirstName;
	edtFirstName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT2));
	edtFirstName->GetWindowTextW(vURL);
	edtFirstName->SetWindowText((CString)vURL);
	m_browser.SetFocus();
}

void AduWebBrowser::OnBnClickedButton4()
{
		COleVariant vEmpty;
		CString vURL;
		CEdit *edtFirstName;
		edtFirstName = reinterpret_cast<CEdit *>(GetDlgItem(IDC_EDIT2));
		edtFirstName->GetWindowTextW(vURL);

		if(m_browser)
			m_browser.Navigate(vURL, vEmpty, vEmpty, vEmpty, vEmpty);	

		edtFirstName->SetWindowText((CString)vURL);
		m_browser.SetFocus();
}

void AduWebBrowser::OnBnClickedButton1()
{
	try { //Anis -> fottuto in pieno.
		m_browser.GoBack();
	}
	catch(...) {
		COleVariant vEmpty;
		m_browser.Navigate(theApp.rm->HomeAduBrowser , vEmpty, vEmpty, vEmpty, vEmpty);
		AfxEnableDlgItem(this,IDC_BUTTON1,false);
	}
}

void AduWebBrowser::OnTimer( UINT nIDEvent )
{
    // Per minute timer ticked.
    if( nIDEvent == 0x1000 )
    {
		if(!m_browser.GetBusy()) {
			GetDlgItem(IDC_BUTTON5)->SetWindowText(_T("Refresh"));
			AfxEnableDlgItem(this,IDC_EDIT2,true);
			AfxEnableDlgItem(this,IDC_BUTTON1,true);
			AfxEnableDlgItem(this,IDC_BUTTON5,true);
		}
		else {
			GetDlgItem(IDC_BUTTON5)->SetWindowText(_T("Loading..."));
			AfxEnableDlgItem(this,IDC_BUTTON5,false);
		}
    }
}