//AdunanzA Source Code File 2011-2012
//Modulo creato da Anis Hireche
#include "stdafx.h"
#include <afxinet.h>
#include "emule.h"
#include "enbitmap.h"
#include "OtherFunctions.h"
#include "Preferences.h"
#include "emuledlg.h"
#include "Statistics.h"
#include "ListenSocket.h"
#include "ClientUDPSocket.h"
#include "HttpDownloadDlg.h"
#include "AdunanzA.h" //Anis
#include "AduWizard.h"//Anis
#include "DAMessageBox.h"
#include "RemoteSettings.h"//Anis
#include "Tlhelp32.h" //Anis -> ProcessList

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//Anis -> Riscritto praticamente da zero

bool X = false;
bool isAutomatic = false;  //Anis -> Settarlo a true quando sarà operativo
bool nonchiudereopzioni = false;
void ConfiguraAdunanza(); //Anis
extern bool FileExist(LPCTSTR filename);
extern LONG GetStringRegKey(HKEY hKey, const std::wstring &strValueName, std::wstring &strValue, const std::wstring &strDefaultValue); //Funzione di Anis
extern inline void AfxEnableDlgItem(CWnd *pDlg, int nIDDlgItem, BOOL bEnabled = TRUE); //Anis function Afx.

//determino automaticamente la versione del sistema operativo.
 
bool isNT() {
 
	OSVERSIONINFO OSversion;
	OSversion.dwOSVersionInfoSize=sizeof(OSVERSIONINFO);
	::GetVersionEx(&OSversion);
 
	switch(OSversion.dwPlatformId) {
 
		case VER_PLATFORM_WIN32_WINDOWS:
			return 0;
			break;
 
		case VER_PLATFORM_WIN32_NT:
			return 1;
			break;

		default: //Anis -> caso teoricamente impossibile
			return 1;
}
}
//Anis -> determino la qualità del sistema operativo.
BOOL is64() {
	#if defined(_WIN64)
		return TRUE;  // 64-bit programs run only on Win64
	#elif defined(_WIN32)
		BOOL f64 = FALSE;
		return IsWow64Process(GetCurrentProcess(), &f64) && f64;
 
	#else
		return FALSE; // Win64 does not support Win16
	#endif
}
//Anis -> ci divertiamo un pò con l'assembly?
// 18/08/2012 -> Questa funzione la disabilito per portare alla possibilità futura di compilare AdunanzA a 64bit. Visual C Compiler infatti non supporta l'assembly inline in tale architettura.
/*DWORD GetMegaHeartz() {
 
	LARGE_INTEGER ulFreq, ulTicks, ulValue, ulStartCounter, ulEAX_EDX;
 
	if (QueryPerformanceFrequency(&ulFreq)) {
		QueryPerformanceCounter(&ulTicks);
		ulValue.QuadPart = ulTicks.QuadPart + ulFreq.QuadPart;
		__asm RDTSC
		__asm mov ulEAX_EDX.LowPart, EAX
		__asm mov ulEAX_EDX.HighPart, EDX
		ulStartCounter.QuadPart = ulEAX_EDX.QuadPart;
		do {
			QueryPerformanceCounter(&ulTicks);
		} while (ulTicks.QuadPart <= ulValue.QuadPart);
		__asm RDTSC
		__asm mov ulEAX_EDX.LowPart, EAX
		__asm mov ulEAX_EDX.HighPart, EDX
		// Anis -> calcoliamo il numero di cicli nell'intervallo -> 1 milione di heartz = 1 mhz
		return (DWORD) ((ulEAX_EDX.QuadPart - ulStartCounter.QuadPart) / 1000000);
	} 

	else
		return 0;
//Anis -> this is high quality code! 
}*/

class Wizard : public CPropertyPageEx
{
	DECLARE_DYNCREATE(Wizard)

public:
	Wizard();

	Wizard(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL) : CPropertyPageEx(nIDTemplate) {
		if (pszCaption)
		{
			m_strCaption = pszCaption; // "convenience storage"
			m_psp.pszTitle = m_strCaption;
			m_psp.dwFlags |= PSP_USETITLE;
		}
		if (pszHeaderTitle && pszHeaderTitle[0] != _T('\0'))
		{
			m_strHeaderTitle = pszHeaderTitle;
			m_psp.dwSize = sizeof(m_psp);
		}
		if (pszHeaderSubTitle && pszHeaderSubTitle[0] != _T('\0'))
		{
			m_strHeaderSubTitle = pszHeaderSubTitle;
			m_psp.dwSize = sizeof(m_psp);
		}
	}

protected:
	CString m_strCaption;
	virtual BOOL OnSetActive();
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNCREATE(Wizard, CPropertyPageEx)

BEGIN_MESSAGE_MAP(Wizard, CPropertyPageEx)
END_MESSAGE_MAP()

Wizard::Wizard() : CPropertyPageEx()
{

}

void Wizard::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPageEx::DoDataExchange(pDX);
}

BOOL Wizard::OnSetActive() 
{
	CPropertySheetEx* pSheet = (CPropertySheetEx*)GetParent();
	if (pSheet->IsWizard())
	{
		int iPages = pSheet->GetPageCount();
		int iActPage = pSheet->GetActiveIndex();
		DWORD dwButtons = 0;
		if (iActPage > 0)
			dwButtons |= PSWIZB_BACK;
		if (iActPage < iPages)
			dwButtons |= PSWIZB_NEXT;
		if (iActPage == iPages-1)
		{
			if (pSheet->m_psh.dwFlags & PSH_WIZARDHASFINISH)
				dwButtons &= ~PSWIZB_NEXT;
			dwButtons |= PSWIZB_FINISH;
		}
		pSheet->SetWizardButtons(dwButtons);
	}
	return CPropertyPageEx::OnSetActive();
}

///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1Welcome dialog

class CPPgWiz1Welcome : public Wizard
{
	DECLARE_DYNAMIC(CPPgWiz1Welcome)

public:
	CPPgWiz1Welcome();
	CPPgWiz1Welcome(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL)
		: Wizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
	}
	virtual ~CPPgWiz1Welcome();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WIZ1_WELCOME };

protected:
	CFont m_FontTitle;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPPgWiz1Welcome, Wizard)

BEGIN_MESSAGE_MAP(CPPgWiz1Welcome, Wizard)
END_MESSAGE_MAP()

CPPgWiz1Welcome::CPPgWiz1Welcome()
	: Wizard(CPPgWiz1Welcome::IDD)
{
}

CPPgWiz1Welcome::~CPPgWiz1Welcome()
{
}

void CPPgWiz1Welcome::DoDataExchange(CDataExchange* pDX)
{
	Wizard::DoDataExchange(pDX);
}

BOOL CPPgWiz1Welcome::OnInitDialog()
{
	X = false;
	CFont fontVerdanaBold;
	CreatePointFont(fontVerdanaBold, 12*10, _T("Verdana Bold"));
	LOGFONT lf;
	fontVerdanaBold.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_FontTitle.CreateFontIndirect(&lf);
	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_WIZ1_TITLE);
	pStatic->SetFont(&m_FontTitle);
	Wizard::OnInitDialog();
	InitWindowStyles(this);
	GetDlgItem(IDC_WIZ1_TITLE)->SetWindowText(GetResString(IDS_WIZ1_WELCOME_TITLE));
	GetDlgItem(IDC_WIZ1_ACTIONS)->SetWindowText(GetResString(IDS_WIZ1_WELCOME_ACTIONS));
	GetDlgItem(IDC_WIZ1_BTN_HINT)->SetWindowText(GetResString(IDS_WIZ1_WELCOME_BTN_HINT));
	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1General dialog

class CPPgWiz1General : public Wizard
{
	DECLARE_DYNAMIC(CPPgWiz1General)

public:
	CPPgWiz1General();
	CPPgWiz1General(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL) : Wizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{
		m_iAutoConnectAtStart = 1;
		m_iAutoStart = 1;
	}
	virtual ~CPPgWiz1General();
	virtual BOOL OnInitDialog();

// Dialog Data
	enum { IDD = IDD_WIZ1_GENERAL };

	CString m_strNick;
	int m_iAutoConnectAtStart;
	int m_iAutoStart;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTimer( UINT nIDEvent );
	afx_msg void StartTimer();
private:
	void RilevaVLC();
};

IMPLEMENT_DYNAMIC(CPPgWiz1General, Wizard)

BEGIN_MESSAGE_MAP(CPPgWiz1General, Wizard)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON1, &CPPgWiz1General::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CPPgWiz1General::OnBnClickedButton2)
	ON_NOTIFY(NM_CLICK, IDC_SYSLINK1, &CPPgWiz1General::OnNMClickSyslink1)
END_MESSAGE_MAP()

CPPgWiz1General::CPPgWiz1General() : Wizard(CPPgWiz1General::IDD)
{
	m_iAutoConnectAtStart = 1;
	m_iAutoStart = 1;
}

CPPgWiz1General::~CPPgWiz1General()
{
	extern bool updating;
	updating = false;
}

void CPPgWiz1General::DoDataExchange(CDataExchange* pDX)
{
	Wizard::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_NICK, m_strNick);
	DDX_Check(pDX, IDC_AUTOCONNECT, m_iAutoConnectAtStart);
	DDX_Check(pDX, IDC_AUTOSTART, m_iAutoStart);
}

void CPPgWiz1General::RilevaVLC() {
	///////////////ANIS ADU MOD -> RILEVAZIONE AUTOMATICA VLC///////////////
	HKEY hKey; 
	std::wstring VLCdir;
	if(ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, REG_VLC_DIR, 0, KEY_READ, &hKey)) {
		GetStringRegKey(hKey, REG_VLC_VAR, VLCdir, L""); 
		thePrefs.m_strVideoPlayer = VLCdir.c_str();
		if (thePrefs.m_strVideoPlayer == _T("")) {
			SetDlgItemText(TESTO_INFO,_T("Non è stato rilevato VLC. AduTeam consiglia di scaricare il player VLC, scaricabile cliccando il tasto sottostante: 'Scarica e installa VLC'. Una volta installato, premere il tasto"));
			return;
		}
		thePrefs.m_strVideoPlayer += VLC_EXE_NAME;
		SetDlgItemText(TESTO_INFO,_T("VLC è stato rilevato correttamente. Premere 'Avanti' per continuare la configurazione guidata passo-passo e salvare le impostazioni."));
		GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
		SetDlgItemTextW(TESTO_INFO2,_T(""));
		return;
	}
	///////////////ANIS ADU MOD -> RILEVAZIONE AUTOMATICA VLC///////////////	
}

BOOL CPPgWiz1General::OnInitDialog()
{
	Wizard::OnInitDialog();
	InitWindowStyles(this);
	((CEdit*)GetDlgItem(IDC_NICK))->SetLimitText(thePrefs.GetMaxUserNickLength());
	GetDlgItem(IDC_NICK_FRM)->SetWindowText(GetResString(IDS_ENTERUSERNAME));
	GetDlgItem(IDC_AUTOCONNECT)->SetWindowText(GetResString(IDS_FIRSTAUTOCON));
	GetDlgItem(IDC_AUTOSTART)->SetWindowText(GetResString(IDS_WIZ_STARTWITHWINDOWS));
	if(FileExist(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe")))
		DeleteFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe"));
	HKEY hKey; 
	std::wstring VLCdir;
	if (thePrefs.m_strVideoPlayer == _T(""))
		RilevaVLC();
	else if(ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, REG_VLC_DIR, 0, KEY_READ, &hKey)) {
		GetStringRegKey(hKey, REG_VLC_VAR, VLCdir, L"");
		if (thePrefs.m_strVideoPlayer == _T(""))
			RilevaVLC();
		else if (VLCdir.append(VLC_EXE_NAME).c_str() == thePrefs.m_strVideoPlayer) {
			SetDlgItemText(TESTO_INFO,_T("VLC è stato rilevato correttamente. Premere 'Avanti' per continuare la configurazione guidata passo-passo e salvare le impostazioni."));
			SetDlgItemTextW(TESTO_INFO2,_T(""));
			GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
			GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
		}
		else {
			SetDlgItemText(TESTO_INFO,_T("Un player personalizzato è stato rilevato. Premere 'Avanti' per continuare la configurazione guidata passo-passo e salvare le impostazioni."));
			SetDlgItemTextW(TESTO_INFO2,_T(""));
			GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
			GetDlgItem(IDC_BUTTON2)->EnableWindow(true);
		}
	}
	else {
		SetDlgItemText(TESTO_INFO,_T("Un player personalizzato è stato rilevato. Premere 'Avanti' per continuare la configurazione guidata passo-passo e salvare le impostazioni."));
		SetDlgItemTextW(TESTO_INFO2,_T(""));
		GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(true);
	}
	return TRUE;
}

///////////////////////////////////////////////////////////////////////////////
// CPPgWiz1End dialog

class CPPgWiz1End : public Wizard
{
	DECLARE_DYNAMIC(CPPgWiz1End)

public:
	CPPgWiz1End();
	CPPgWiz1End(UINT nIDTemplate, LPCTSTR pszCaption = NULL, LPCTSTR pszHeaderTitle = NULL, LPCTSTR pszHeaderSubTitle = NULL) : Wizard(nIDTemplate, pszCaption, pszHeaderTitle, pszHeaderSubTitle)
	{}
	virtual ~CPPgWiz1End();
	virtual BOOL OnInitDialog();

	enum { IDD = IDD_WIZ1_END };

protected:
	CFont m_FontTitle;
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedRadio2();
	afx_msg void OnBnClickedRadio3();
};

IMPLEMENT_DYNAMIC(CPPgWiz1End, Wizard)

BEGIN_MESSAGE_MAP(CPPgWiz1End, Wizard)
	ON_BN_CLICKED(IDC_RADIO2, &CPPgWiz1End::OnBnClickedRadio2)
	ON_BN_CLICKED(IDC_RADIO3, &CPPgWiz1End::OnBnClickedRadio3)
END_MESSAGE_MAP()

CPPgWiz1End::CPPgWiz1End() : Wizard(CPPgWiz1End::IDD)
{}

CPPgWiz1End::~CPPgWiz1End()
{}

//Anis -> Questa funzione la userò per determinare se fare o meno lo speedtest. In poche parole se il ping è troppo alto significa che la connessione è in uso eccessivamente e non si può proseguire con lo speed test (darebbe risultati falsi.)
CString ExecCmd(CString pCmdArg) {   //Anis 
	CString strResult; // Contains result of cmdArg.    
	HANDLE hChildStdoutRd; // Read-side, used in calls to ReadFile() to get child's stdout output.   
	HANDLE hChildStdoutWr; // Write-side, given to child process using si struct.    
	BOOL fSuccess;    // Create security attributes to create pipe.  
	SECURITY_ATTRIBUTES saAttr = {sizeof(SECURITY_ATTRIBUTES)} ;   
	saAttr.bInheritHandle       = TRUE; // Set the bInheritHandle flag so pipe handles are inherited by child process. Required.  
	saAttr.lpSecurityDescriptor = NULL;    // Create a pipe to get results from child's stdout.   

	if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0))   {return strResult;}    
	STARTUPINFO si = {sizeof(STARTUPINFO)}; // specifies startup parameters for child process.    
	si.dwFlags     = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; // STARTF_USESTDHANDLES is Required.   
	si.hStdOutput  = hChildStdoutWr; // Requires STARTF_USESTDHANDLES in dwFlags.  
	si.hStdError   = hChildStdoutWr; // Requires STARTF_USESTDHANDLES in dwFlags.   // si.hStdInput remains null.   
	si.wShowWindow = SW_HIDE; // Prevents cmd window from flashing. Requires STARTF_USESHOWWINDOW in dwFlags.   
	PROCESS_INFORMATION pi  = {0}; // Create the child process.  
	fSuccess = CreateProcess(NULL, pCmdArg.GetBuffer(), NULL, NULL, TRUE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);       

	if (!fSuccess)   
		return strResult;
	WaitForSingleObject(pi.hProcess, 2000);   
	TerminateProcess(pi.hProcess, 0); // Kill process if it is still running. Tested using cmd "ping blah -n 99"   
	if (!CloseHandle(hChildStdoutWr))   {return strResult;}    // Read output from the child process.  
	for (;;)   {    
		DWORD dwRead;    
		CHAR chBuf[4096];       // Read from pipe that is the standard output for child process.      
		bool done = !ReadFile( hChildStdoutRd, chBuf, 4096, &dwRead, NULL) || dwRead == 0;      
		if( done )      {break;}       // Append result to string.      
		strResult += CString( chBuf, dwRead) ;  
	} 
	CloseHandle( hChildStdoutRd );    // CreateProcess docs specify that these must be closed.    
	CloseHandle( pi.hProcess );   
	CloseHandle( pi.hThread );   
	return strResult;
}


void StartConfigAdunanzA() { //SpeedTest engine -> by Anis
	if(X) {
		if (isAutomatic) {
			// Anis -> Controllo ping. Se minore della tolleranza impostata allora da avviso di rete in uso da programmi di terze parti.
retry:
			isAutomatic = true;
			CString str;
			str = ExecCmd(_T("ping debian.fastweb.it -n 1"));
			str.Replace(_T("\x0d\x0d\x0a"), _T("\x0d\x0a"));
			int num = str.GetLength()-8;
			str.Delete(0,num);
			wchar_t *src, *dst; 
			for (src = str.GetBuffer(), dst = str.GetBuffer(); *src; src++)
				if ('0' <= *src && *src <= '9') 
					*dst++ = *src;
				*dst = '\0'; 

			if(str.IsEmpty()) {
				AfxMessageBox(_T("Il server risulta essere offline. Verrà lanciata la configurazione manuale."));
				isAutomatic = false;
				goto personalized;
			}

			if(_tstoi(str) > theApp.rm->TolleranzaTest) {
				isAutomatic = false;
				if(AfxMessageBox(_T("La rete risulta occupata da altri software. Chiuderli e riprovare."),MB_RETRYCANCEL | MB_ICONHAND) == IDRETRY)
					goto retry;
				else	
					goto personalized;
			}

			CHttpDownloadDlg dlgDownload;
			dlgDownload.X = true;
			dlgDownload.m_sURLToDownload = theApp.rm->SpeedTestUrl;
			dlgDownload.m_sFileToDownloadInto = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("speed_test.adu");
			dlgDownload.DoModal();
	}
	else {
personalized:
			AduWizard WizardPersonalizzato;
			WizardPersonalizzato.DoModal();
		}
	}
}

void ConfiguraAdunanza() { //SpeedTest Engine - Core by Anis Hireche
	switch (isNT()) { //Anis

		case true:
			if(is64()) {
				switch (thePrefs.maxGraphDownloadRate) {
					case ADSL1:
					case ADSL2:
					case ADSL4:			
					case ADSL6:
					case ADSL8:	
					{
						thePrefs.maxconnections = 800;
						thePrefs.maxsourceperfile = 600;
						break;
							  }
					case ADSL12:
					case ADSL16:
					case ADSL20:
					case FIBRA10:
					case FIBRA100: {
						thePrefs.maxconnections = 1600;
						thePrefs.maxsourceperfile = 800;
						break;
							   }
				}
			}
			else if(is64()) {
				switch (thePrefs.maxGraphDownloadRate) {
					case ADSL1:
					case ADSL2:
					case ADSL4:			
					case ADSL6:
					case ADSL8:
					{
						thePrefs.maxconnections = 650;
						thePrefs.maxsourceperfile = 400;
						break;
							  }
					case ADSL12:
					case ADSL16:
					case ADSL20:
					case FIBRA10:
					case FIBRA100: {
						thePrefs.maxconnections = 1200;
						thePrefs.maxsourceperfile = 600;
						break;
							   }
				}
			}

			else if(!is64()) {
				switch (thePrefs.maxGraphDownloadRate) {
					case ADSL1:
					case ADSL2:
					case ADSL4:			
					case ADSL6:
					case ADSL8:
					{
						thePrefs.maxconnections = 650;
						thePrefs.maxsourceperfile = 400;
						break;
							  }
					case ADSL12:
					case ADSL16:
					case ADSL20:
					case FIBRA10:
					case FIBRA100: {
						thePrefs.maxconnections = 1200;
						thePrefs.maxsourceperfile = 600;
						break;
							   }
				}
			}
			else if(!is64()) {
				switch (thePrefs.maxGraphDownloadRate) {
					case ADSL1:
					case ADSL2:
					case ADSL4:			
					case ADSL6:
					case ADSL8:
						{
						thePrefs.maxconnections = 500;
						thePrefs.maxsourceperfile = 400;
						break;
							  }
					case ADSL12:
					case ADSL16:
					case ADSL20:
					case FIBRA10:
					case FIBRA100: {
						thePrefs.maxconnections = 800;
						thePrefs.maxsourceperfile = 600;
						break;
							   }
				}
			}

			break;

		case false: //Anis -> Sistema operativo vecchio = half-open limitatissime indipendenti dal tipo di connessione. limite imposto dal software.
			thePrefs.maxconnections = 500;
			thePrefs.maxsourceperfile = 400;
			break;
	}

}

void CPPgWiz1End::DoDataExchange(CDataExchange* pDX)
{
	Wizard::DoDataExchange(pDX);
}

BOOL CPPgWiz1End::OnInitDialog()
{
	isAutomatic = false;	 // Anis -> settarlo a true quando sarà pronto.
	CheckRadioButton(IDC_RADIO2,IDC_RADIO3,IDC_RADIO3); //Anis -> valore automatico di default
	CFont fontVerdanaBold;
	CreatePointFont(fontVerdanaBold, 12*10, _T("Verdana Bold"));
	LOGFONT lf;
	fontVerdanaBold.GetLogFont(&lf);
	lf.lfWeight = FW_BOLD;
	m_FontTitle.CreateFontIndirect(&lf);

	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_WIZ1_TITLE);
	pStatic->SetFont(&m_FontTitle);

	Wizard::OnInitDialog();
	InitWindowStyles(this);
	GetDlgItem(IDC_WIZ1_TITLE)->SetWindowText(GetResString(IDS_WIZ1_END_TITLE));
	GetDlgItem(IDC_WIZ1_BTN_HINT)->SetWindowText(GetResString(IDS_WIZ1_END_BTN_HINT));

	return TRUE;
}


///////////////////////////////////////////////////////////////////////////////
// CPShtWiz1

class CPShtWiz1 : public CPropertySheetEx
{
	DECLARE_DYNAMIC(CPShtWiz1)

public:
	CPShtWiz1(UINT nIDCaption, CWnd* pParentWnd = NULL, UINT iSelectPage = 0);
	virtual ~CPShtWiz1();

protected:
	DECLARE_MESSAGE_MAP()
};

IMPLEMENT_DYNAMIC(CPShtWiz1, CPropertySheetEx)

BEGIN_MESSAGE_MAP(CPShtWiz1, CPropertySheetEx)
END_MESSAGE_MAP()

CPShtWiz1::CPShtWiz1(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage) :CPropertySheetEx(nIDCaption, pParentWnd, iSelectPage)
{
}

CPShtWiz1::~CPShtWiz1()
{
}

bool WizardAdunanzA() { //Anis -> Questa è la funzione che organizza i vari frammenti dell'AduWizard.
	extern bool WizardNotOpen;
	WizardNotOpen = false;
	nonchiudereopzioni = false;
	CEnBitmap bmWatermark;
	VERIFY( bmWatermark.LoadImage(IDR_WIZ1_WATERMARK, _T("GIF"), NULL, GetSysColor(COLOR_WINDOW)) );
	CEnBitmap bmHeader;
	VERIFY( bmHeader.LoadImage(IDR_WIZ1_HEADER, _T("GIF"), NULL, GetSysColor(COLOR_WINDOW)) );
	CPropertySheetEx sheet(GetResString(IDS_WIZ1), NULL, 0, bmWatermark, NULL, bmHeader);
	sheet.m_psh.dwFlags |= PSH_WIZARD;
	sheet.m_psh.dwFlags |= PSH_WIZARD97;

	CPPgWiz1Welcome	page1(IDD_WIZ1_WELCOME, GetResString(IDS_WIZ1));
	page1.m_psp.dwFlags |= PSP_HIDEHEADER;
	sheet.AddPage(&page1);
	CPPgWiz1General page2(IDD_WIZ1_GENERAL, GetResString(IDS_WIZ1), GetResString(IDS_PW_GENERAL), _T("Configurazione"));

	if (isNT()) //Anis -> Configurazione automatica supportata solo dai moderni OS. Almeno per ora... Causa proposta VLC che è incompatibile con sistemi < NT
		sheet.AddPage(&page2);

	//Anis -> rendo migliore la configurazione personalizzata di adunanza scritta da Emanem
	
	CPPgWiz1End page7(IDD_WIZ1_END, GetResString(IDS_WIZ1));
	page7.m_psp.dwFlags |= PSP_HIDEHEADER;
	sheet.AddPage(&page7);

	int iResult = sheet.DoModal();

	if (iResult == IDCANCEL) {
		X=false; //Anis -> le prugne.
		nonchiudereopzioni = true;
	}
	else
		X=true;

	page2.m_strNick = thePrefs.GetUserNick();
	if (page2.m_strNick.IsEmpty())
		page2.m_strNick = DEFAULT_NICK;
	page2.m_strNick.Trim();
	if (page2.m_strNick.IsEmpty())
		page2.m_strNick = DEFAULT_NICK;
	thePrefs.SetUserNick(page2.m_strNick);	
	thePrefs.SetAutoStart(page2.m_iAutoStart!=0);

	if(thePrefs.GetAutoStart())
		AddAutoStart();
	else
		RemAutoStart();

	ASSERT( thePrefs.port != 0 && thePrefs.udpport != 0);
	
	if (thePrefs.port == 0)
		thePrefs.port = 4662;

	if (thePrefs.udpport == 0)
		thePrefs.udpport = 4672;

	if ((thePrefs.port!=theApp.listensocket->GetConnectedPort()) || (thePrefs.udpport!=theApp.clientudp->GetConnectedPort())) {
		if (!theApp.IsPortchangeAllowed())
			AfxMessageBox(GetResString(IDS_NOPORTCHANGEPOSSIBLE));
		else {
			theApp.listensocket->Rebind();
			theApp.clientudp->Rebind();
		}
	}

	// mod Adu
	// Emanem
	// Anis -> Revisione Totale
	// ho messo i valori prestabiliti
	thePrefs.m_bUPnPNat = true; //Anis -> uPnP attivato di default.
	thePrefs.m_bShowDwlPercentage = true; //Anis -> metto la percentuale dei download di default 
	thePrefs.m_bCryptLayerSupported = true; //Anis -> Supportato per default - sarebbe l'offuscamento su ed2k.
	thePrefs.notifierOnDownloadFinished = true; //Anis -> notifica download completi.
	thePrefs.notifierOnImportantError = true; //Anis -> Mi serve per lo streaming.
	thePrefs.SetNewAutoDown(true);
	thePrefs.SetNewAutoUp(true);
	thePrefs.SetTransferFullChunks(true);
	thePrefs.SetSafeServerConnectEnabled(false);
	thePrefs.SetAutoConnect(true);
	thePrefs.SetNetworkKademlia(true);
	::StartConfigAdunanzA();
	return TRUE;
}

AduWizard::ConnectionEntry AduWizard::m_connections[] = 
{	//Anis
	//nome, download, upload
	{ _T("Personalizzato"),									0,    0},
	{ _T("DSL 1 MegaBit"),                           ADSL1,  UPLOAD_ADSL_LOW},
	{ _T("DSL 2 MegaBit"),                           ADSL2,  UPLOAD_ADSL_LOW},
	{ _T("DSL 4 MegaBit"),                           ADSL4,  UPLOAD_ADSL_LOW},
	{ _T("DSL 6 MegaBit"),							 ADSL6,  UPLOAD_ADSL},
	{ _T("DSL 8 MegaBit"),							 ADSL8,  UPLOAD_ADSL},
	{ _T("DSL 12 MegaBit"),							ADSL12,  UPLOAD_ADSL},
	{ _T("DSL 16 MegaBit"),							ADSL16,  UPLOAD_ADSL},
	{ _T("DSL 20 MegaBit"),                         ADSL20,  UPLOAD_ADSL},
	{ _T("Fibra Ottica 10 MegaBit"),               FIBRA10, UPLOAD_FIBRA},
	{ _T("Fibra Ottica 100 MegaBit"),			  FIBRA100, UPLOAD_FIBRA}
};


// finestra di dialogo AduWizard

IMPLEMENT_DYNAMIC(AduWizard, CDialog)


AduWizard::AduWizard(CWnd* pParent /*=NULL*/): CDialog(AduWizard::IDD, pParent)
{
	CDAMessageBox mb(NULL,
		_T("AVVISO!\n\n")
		_T("Ricordiamo che non impostare il tipo di connessione può comportare problemi alla navigazione e all'uso del telefono.\n\n")
		_T("Impostarli sbagliati peggiorerà le prestazioni.\n\n")
		_T("Impostare valori bassi per l'upload ridurrà notevolmente le vostre possibilità di scaricare.\n\n")
		_T("Premere il tasto 'OK' per proseguire con la configurazione manuale."),false,false);
	mb.DoModal();
	m_icnWnd = NULL;
	last_selected=-1;
}

AduWizard::~AduWizard()
{
}

BEGIN_MESSAGE_MAP(AduWizard, CDialog)
	ON_BN_CLICKED(IDC_WIZ_APPLY_BUTTON, OnBnClickedWizApplyButton)
	ON_NOTIFY(NM_CLICK, IDC_PROVIDERS, OnNMClickProviders)
END_MESSAGE_MAP()

BOOL AduWizard::OnInitDialog()
{
	CDialog::OnInitDialog();
	InitWindowStyles(this);
	SetIcon(m_icnWnd = theApp.LoadIcon(_T("Wizard")), FALSE);
	CheckRadioButton(IDC_KBITS, IDC_KBYTES, IDC_KBYTES);
	SetDlgItemInt(IDC_WIZ_TRUEDOWNLOAD_BOX, 0, FALSE);
	SetDlgItemInt(IDC_WIZ_TRUEUPLOAD_BOX, 0, FALSE);
	m_provider.InsertColumn(0, GetResString(IDS_PW_CONNECTION), LVCFMT_LEFT, 150);
	m_provider.InsertColumn(1, GetResString(IDS_WIZ_DOWN), LVCFMT_LEFT, 85);
	m_provider.InsertColumn(2, GetResString(IDS_WIZ_UP), LVCFMT_LEFT, 85);
	m_provider.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
 	
 	for(int i = 0; i < sizeof(m_connections)/sizeof(m_connections[0]); i++)
 	{
 		ConnectionEntry & c = m_connections[ i ];
 
 		m_provider.InsertItem( i , c.name );
 		CString temp;
 
 		temp.Format( _T("%u"), c.down );
 		m_provider.SetItemText(i  , 1, temp );
 
 		temp.Format( _T("%u"), c.up );
		m_provider.SetItemText(i , 2, temp );
 	}
 
 	CheckDlgButton(IDC_KBITS,0);
 	CheckDlgButton(IDC_KBYTES,1);
	m_provider.SetSelectionMark(0);
	m_provider.SetItemState(0, LVIS_FOCUSED | LVIS_SELECTED, LVIS_FOCUSED | LVIS_SELECTED);
	SetCustomItemsActivation();
	return TRUE;
}

BOOL AduWizard::PreTranslateMessage(MSG* pMsg) { //Anis -> blocco i tasti f4 e escape per evitare che un furbetto chiuda la finestra modale inaspettatamente.
	if(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_F4)
		return TRUE;
	return CDialog::PreTranslateMessage(pMsg);
}

void AduWizard::OnBnClickedWizApplyButton()
{
	TCHAR buffer[510];
	int upload, download;
	if (GetDlgItem(IDC_WIZ_TRUEDOWNLOAD_BOX)->GetWindowTextLength())
	{ 
		GetDlgItem(IDC_WIZ_TRUEDOWNLOAD_BOX)->GetWindowText(buffer, 20);
		download = _tstoi(buffer);
	}
	else
		download = 0;

	if (GetDlgItem(IDC_WIZ_TRUEUPLOAD_BOX)->GetWindowTextLength())
	{ 
		GetDlgItem(IDC_WIZ_TRUEUPLOAD_BOX)->GetWindowText(buffer, 20);
		upload = _tstoi(buffer);
	}
	else
		upload = 0;

	if(IsDlgButtonChecked(IDC_KBITS)==1) {upload/=8;download/=8;}
	
	if(download > 50000) download = 50000;
	if(upload > 50000) upload = 50000;
	if(download < 10) download = 10;
	if(upload < 10) upload = 10;

	thePrefs.maxGraphDownloadRate = download;
	thePrefs.maxGraphUploadRate = upload;
	thePrefs.maxdownload = download; //Anis -> rimosso casting non-sense
	thePrefs.maxupload = upload;

	//Anis -> fix max value. se l'intero è maggiore di 65000 circa va in overflow.
	//Anis -> Minimo 10. (meno non avrebbe senso per qualunque cosa).
	
	CDialog::OnOK();
}

void AduWizard::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROVIDERS, m_provider);
}

void AduWizard::OnNMClickProviders(NMHDR *pNMHDR, LRESULT *pResult)
{
	SetCustomItemsActivation();
	UINT up, down;
	int selected = m_provider.GetSelectionMark();
    ConnectionEntry & c = m_connections[ selected ];
    down = c.down;
    up = c.up;
	*pResult = 0;
	SetDlgItemInt(IDC_WIZ_TRUEDOWNLOAD_BOX, down, FALSE);
	SetDlgItemInt(IDC_WIZ_TRUEUPLOAD_BOX, up, FALSE);
	CheckRadioButton(IDC_KBITS, IDC_KBYTES, IDC_KBYTES);
}

void AduWizard::SetCustomItemsActivation()
{
	BOOL bActive = m_provider.GetSelectionMark() == 0; // Mod Adu: lupz -> cambio la posizione del personalizzato
	GetDlgItem(IDC_WIZ_TRUEUPLOAD_BOX)->EnableWindow(bActive);
	GetDlgItem(IDC_WIZ_TRUEDOWNLOAD_BOX)->EnableWindow(bActive);
	GetDlgItem(IDC_KBITS)->EnableWindow(bActive);
	GetDlgItem(IDC_KBYTES)->EnableWindow(bActive);
}
void CPPgWiz1End::OnBnClickedRadio2()
{
	isAutomatic = true;
}

void CPPgWiz1End::OnBnClickedRadio3()
{
	isAutomatic = false;
}

void CPPgWiz1General::OnBnClickedButton1()
{
	///////////////ANIS ADU MOD -> RILEVAZIONE AUTOMATICA VLC///////////////
	HKEY hKey; 
	std::wstring VLCdir;
	if(ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE, REG_VLC_DIR, 0, KEY_READ, &hKey)) {
		GetStringRegKey(hKey, REG_VLC_VAR, VLCdir, L""); 
		thePrefs.m_strVideoPlayer = VLCdir.c_str();
		if (thePrefs.m_strVideoPlayer == _T("")) {
			SetDlgItemText(TESTO_INFO,_T("Non è stato rilevato VLC. AduTeam consiglia di scaricare il player VLC, scaricabile cliccando il tasto sottostante: 'Scarica e installa VLC'. Una volta installato, premere il tasto"));
			AfxMessageBox(_T("VLC non è stato rilevato. Assicurarsi di averlo installato correttamente."));
			return;
		}
		thePrefs.m_strVideoPlayer += VLC_EXE_NAME;
		SetDlgItemText(TESTO_INFO,_T("VLC è stato rilevato correttamente. Premere 'Avanti' per continuare la configurazione guidata passo-passo e salvare le impostazioni."));
		GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
		SetDlgItemTextW(TESTO_INFO2,_T(""));
		return;
	}
	AfxMessageBox(_T("VLC non è stato rilevato. Assicurarsi di averlo installato correttamente."));
	///////////////ANIS ADU MOD -> RILEVAZIONE AUTOMATICA VLC///////////////	
}

void CPPgWiz1General::OnBnClickedButton2()
{
	extern bool updating;
	updating = true; //Anis -> serve per fare in modo che il downloaddialog sia centrato :3
	try {
		GetDlgItem(IDC_BUTTON2)->EnableWindow(false);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(false);
		CHttpDownloadDlg dlgDownload;
		dlgDownload.m_sURLToDownload = _T("http://downloads.sourceforge.net/project/vlc/2.0.4/win32/vlc-2.0.4-win32.exe?r=http%3A%2F%2Fsourceforge.net%2Fprojects%2Fvlc%2Ffiles%2F2.0.4%2Fwin32%2Fvlc-2.0.4-win32.exe%2Fdownload%3Faccel_key%3D60%253A1352342087%253Ahttp%25253A%2F%2Fwww.videolan.org%2Fvlc%2F%253Ac74790bd%2524fbdb249f6acefd77a9a93980cfaf560e0e504107%26click_id%3Ddcadf9d6-294c-11e2-a2c9-0200ac1d1d8a%26source%3Daccel&ts=1352342088&use_mirror=switch");
		dlgDownload.m_sFileToDownloadInto = thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe");
		GetDlgItem(TESTO_INFO)->SetWindowText(_T("Download di VLC in corso. Attendere..."));
		SetDlgItemTextW(TESTO_INFO2,_T(""));
		int result = dlgDownload.DoModal();
		if(result) {
			if(FileExist(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe"))) {
				CPropertySheet* psheet = (CPropertySheet*) GetParent();   
				psheet->SetWizardButtons(PSWIZB_NEXT & PSWIZB_BACK);
				ShellExecute(NULL, NULL, _T("vlc_installer.exe"), _T("/S"), thePrefs.GetMuleDirectory(EMULE_CONFIGDIR), SW_SHOWDEFAULT);
				GetDlgItem(TESTO_INFO)->SetWindowText(_T("Installazione di VLC in corso. Attendere..."));
				StartTimer();
			}
			else {
				GetDlgItem(TESTO_INFO)->SetWindowText(_T("Il server risulta essere offline. Riprovare."));
				GetDlgItem(IDC_BUTTON2)->EnableWindow(true);
				GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
			}
		}
		else {
			GetDlgItem(TESTO_INFO)->SetWindowText(_T("Il server risulta essere offline. Riprovare."));
			GetDlgItem(IDC_BUTTON2)->EnableWindow(true);
			GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
		}
		updating = false;
	}
	catch(...) { //Anis -> Se sei sfigato, finisci in quel blocco.
		GetDlgItem(IDC_BUTTON2)->EnableWindow(true);
		GetDlgItem(IDC_BUTTON1)->EnableWindow(true);
		GetDlgItem(TESTO_INFO)->SetWindowText(_T("Errore critico durante l'operazione. Riprovare."));
		SetDlgItemTextW(TESTO_INFO2,_T(""));
		updating = false;
		if(FileExist(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe")))
			DeleteFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe"));
	}
}

void CPPgWiz1General::OnNMClickSyslink1(NMHDR *pNMHDR, LRESULT *pResult)
{
	ShellExecute(this->m_hWnd,_T("open"),_T("http://aduteca.adunanza.net/Emule_AdunanzA/Guida/AduStreaming"),NULL,NULL, SW_SHOW);
	*pResult = 0;
}

void CPPgWiz1General::StartTimer()
{
    SetTimer(0x1000, 1000, 0); //Anis -> Ogni secondo faccio il controllo dell'installer vlc per sapere se è in esecuzione o meno
}

DWORD FindProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if ( processesSnapshot == INVALID_HANDLE_VALUE )
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if ( !processName.compare(processInfo.szExeFile) )
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while ( Process32Next(processesSnapshot, &processInfo) )
	{
		if ( !processName.compare(processInfo.szExeFile) )
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}
	
	CloseHandle(processesSnapshot);
	return 0;
}

bool IsVLCInstallerRunning() {
	if (FindProcessId(_T("vlc_installer.exe")) == 0)
		return 0;
	else					// Process is running
		return 1;
	return -1;
}

void CPPgWiz1General::OnTimer( UINT nIDEvent )
{
    // Per minute timer ticked.
    if(nIDEvent == 0x1000)
    {	
		if (!IsVLCInstallerRunning()) {
			if(FileExist(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe"))) {
				DeleteFile(thePrefs.GetMuleDirectory(EMULE_CONFIGDIR) + _T("vlc_installer.exe"));
				RilevaVLC();
				CPropertySheet* psheet = (CPropertySheet*) GetParent();   
				psheet->SetWizardButtons(PSWIZB_BACK | PSWIZB_NEXT);
				KillTimer(0x1000);
			}
		}
    }
}
