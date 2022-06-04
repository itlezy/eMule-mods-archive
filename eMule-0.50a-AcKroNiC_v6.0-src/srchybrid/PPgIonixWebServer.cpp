// PPgIonixWebserver.cpp : implementation file
#include "stdafx.h"
#include "emule.h"
#include "emuledlg.h"
#include "otherfunctions.h"
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "WebServer.h"
#include "PPgIonixWebServer.h"
#include "PreferencesDlg.h"
#include "MD5Sum.h" //>>> [ionix] - iONiX::Advanced WebInterface Account Management
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#define HIDDEN_PASSWORD _T("*****")

IMPLEMENT_DYNAMIC(CPPgIonixWebServer, CPropertyPage)
CPPgIonixWebServer::CPPgIonixWebServer()
	:  CPPgtooltipped(CPPgIonixWebServer::IDD)
{
	m_bIsInit = false;
	// MORPH start tabbed option [leuk_he]
	m_imageList.DeleteImageList();
	m_imageList.Create(16, 16, theApp.m_iDfltImageListColorFlags | ILC_MASK, 14+1, 0);
	m_imageList.Add(CTempIconLoader(_T("WEB")));
	// MORPH end tabbed option [leuk_he]
}

CPPgIonixWebServer::~CPPgIonixWebServer()
{
}


void CPPgIonixWebServer::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);

	//>>> [ionix] - iONiX::Advanced WebInterface Account Management
	DDX_Control(pDX, IDC_ACCOUNTSELECT, m_cbAccountSelector); 
	DDX_Control(pDX, IDC_ADVADMIN_USERLEVEL, m_cbUserlevel); 
	//<<< [ionix] - iONiX::Advanced WebInterface Account Management
	  // MORPH start tabbed options [leuk_he]
    DDX_Control(pDX, IDC_TAB_WEBSERVER2 , m_tabCtr);
  // MORPH end tabbed options [leuk_he]

}

BEGIN_MESSAGE_MAP(CPPgIonixWebServer, CPropertyPage)
//>>> [ionix] - iONiX::Advanced WebInterface Account Management
	ON_CBN_SELCHANGE(IDC_ACCOUNTSELECT, UpdateSelection)
	ON_BN_CLICKED(IDC_ADVADMINENABLED,   OnEnableChange)
	ON_BN_CLICKED(IDC_ADVADMIN_KAD, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_TRANSFER, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SEARCH, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SERVER, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_SHARED, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_STATS, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_PREFS, OnSettingsChange)
	ON_BN_CLICKED(IDC_ADVADMIN_DFILES, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_ADVADMIN_USERLEVEL, OnSettingsChange)	
	ON_BN_CLICKED(IDC_ADVADMIN_DELETE, OnBnClickedDel)
	ON_BN_CLICKED(IDC_ADVADMIN_NEW, OnBnClickedNew)	
	ON_EN_CHANGE(IDC_ADVADMIN_PASS, OnMultiPWChange)
	ON_EN_CHANGE(IDC_ADVADMIN_CATS, OnMultiCatsChange)
//	ON_EN_CHANGE(IDC_ADVADMIN_USER, OnSettingsChange)
//<<< [ionix] - iONiX::Advanced WebInterface Account Management
	// MORPH start tabbed option [leuk_he]
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_WEBSERVER2, OnTcnSelchangeTab)
	// MORPH end tabbed option [leuk_he]
	ON_WM_HELPINFO()
END_MESSAGE_MAP()


BOOL CPPgIonixWebServer::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
 
	//>>> [ionix] - iONiX::Advanced WebInterface Account Management
	FillComboBox();
	FillUserlevelBox();
	m_cbAccountSelector.SetCurSel(0); // new account
	//<<< [ionix] - iONiX::Advanced WebInterface Account Management

	LoadSettings();
    InitTooltips(); //MORPH leuk_he tolltipped

	Localize();

	return TRUE;
}

void CPPgIonixWebServer::LoadSettings(void)
{
	if(m_hWnd)
	{
		CheckDlgButton(IDC_ADVADMINENABLED, thePrefs.UseIonixWebsrv());
		m_bIsInit = true;
	}
}

void CPPgIonixWebServer::OnEnableChange() {
	thePrefs.m_bIonixWebsrv= (IsDlgButtonChecked(IDC_ADVADMINENABLED)!=0);

    SetBoxes();
	if (!thePrefs.m_bIonixWebsrv) {
	       		FillComboBox();
	            FillUserlevelBox();
	}
    SetModified();
}



BOOL CPPgIonixWebServer::OnApply()
{	

	theApp.webserver->SaveWebServConf(); //>>> [ionix] - iONiX::Advanced WebInterface Account Management

	SetModified(FALSE);

	LoadSettings();
	return CPropertyPage::OnApply();
}

void CPPgIonixWebServer::Localize(void)
{
	if(m_hWnd){
		SetWindowText(GetResString(IDS_TAB_MULTI_USER) + _T(" ") + GetResString(IDS_PW_WS));

		// MORPH start tabbed options [leuk_he]
		InitTab(true,1);
		m_tabCtr.SetCurSel(theApp.emuledlg->preferenceswnd->StartPageWebServer);
		// MORPH end tabbed options [leuk_he]

		GetDlgItem(IDC_ADVADMINENABLED)->SetWindowText(GetResString(IDS_ADVADMINENABLED));
		GetDlgItem(IDC_ADVADMIN_NOTE)->SetWindowText(GetResString(IDS_ADVADMIN_NOTE));
		//>>> [ionix] - iONiX::Advanced WebInterface Account Management	
		GetDlgItem(IDC_STATIC_ADVADMIN)->SetWindowText(GetResString(IDS_ADVADMIN_GROUP));
		GetDlgItem(IDC_STATIC_ADVADMIN_ACC)->SetWindowText(GetResString(IDS_ADVADMIN_ACC));
		GetDlgItem(IDC_ADVADMIN_DELETE)->SetWindowText(GetResString(IDS_ADVADMIN_DELETE));
		GetDlgItem(IDC_ADVADMIN_NEW)->SetWindowText(GetResString(IDS_ADVADMIN_NEW));
		GetDlgItem(IDC_ADVADMIN_KAD)->SetWindowText(GetResString(IDS_ADVADMIN_KAD));
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->SetWindowText(GetResString(IDS_ADVADMIN_TRANSFER));
		GetDlgItem(IDC_ADVADMIN_SEARCH)->SetWindowText(GetResString(IDS_ADVADMIN_SEARCH));
		GetDlgItem(IDC_ADVADMIN_SERVER)->SetWindowText(GetResString(IDS_ADVADMIN_SERVER));
		GetDlgItem(IDC_ADVADMIN_SHARED)->SetWindowText(GetResString(IDS_ADVADMIN_SHARED));
		GetDlgItem(IDC_ADVADMIN_STATS)->SetWindowText(GetResString(IDS_ADVADMIN_STATS));
		GetDlgItem(IDC_ADVADMIN_PREFS)->SetWindowText(GetResString(IDS_ADVADMIN_PREFS));
		GetDlgItem(IDC_ADVADMIN_DFILES)->SetWindowText(GetResString(IDS_ADVADMIN_DFILES));
		GetDlgItem(IDC_STATIC_ADVADMIN_USERLEVEL)->SetWindowText(GetResString(IDS_ADVADMIN_USERLEVEL));
		GetDlgItem(IDC_STATIC_ADVADMIN_PASS)->SetWindowText(GetResString(IDS_ADVADMIN_PASS));
		GetDlgItem(IDC_STATIC_ADVADMIN_USER)->SetWindowText(GetResString(IDS_ADVADMIN_USER));
		GetDlgItem(IDC_STATIC_ADVADMIN_CATS)->SetWindowText(GetResString(IDS_ADVADMIN_CAT));
		//<<< [ionix] - iONiX::Advanced WebInterface Account Management
		SetTool(IDC_ADVADMINENABLED,IDS_ADVADMINENABLED_TIP);
		SetTool(IDC_ACCOUNTSELECT,IDS_ACCOUNTSELECT_TIP);
		SetTool(IDC_ADVADMIN_USER,IDS_ADVADMIN_USER_TIP);
		SetTool(IDC_ADVADMIN_PASS,IDS_ADVADMIN_PASS_TIP);
		SetTool(IDC_ADVADMIN_KAD,IDS_ADVADMIN_KAD_TIP);
		SetTool(IDC_ADVADMIN_TRANSFER,IDS_ADVADMIN_TRANSFER_TIP);
		SetTool(IDC_ADVADMIN_SEARCH,IDS_ADVADMIN_SEARCH_TIP);
		SetTool(IDC_ADVADMIN_SERVER,IDS_ADVADMIN_SERVER_TIP);
		SetTool(IDC_ADVADMIN_SHARED,IDS_ADVADMIN_SHARED_TIP);
		SetTool(IDC_ADVADMIN_STATS,IDS_ADVADMIN_STATS_TIP);
		SetTool(IDC_ADVADMIN_PREFS,IDS_ADVADMIN_PREFS_TIP);
		SetTool(IDC_ADVADMIN_DFILES,IDS_ADVADMIN_DFILES_TIP);
		SetTool(IDC_ADVADMIN_USERLEVEL,IDS_ADVADMIN_USERLEVEL_TIP);
		SetTool(IDC_ADVADMIN_CATS,IDS_ADVADMIN_CATS_TIP);
		SetTool(IDC_ADVADMIN_DELETE,IDS_ADVADMIN_DELETE_TIP);
		SetTool(IDC_STATIC_ADVADMIN_CATS,IDS_STATIC_ADVADMIN_CATS_TIP);
		SetTool(IDC_STATIC_ADVADMIN_USER,IDS_STATIC_ADVADMIN_USER_TIP);
		SetTool(IDC_STATIC_ADVADMIN_PASS,IDS_STATIC_ADVADMIN_PASS_TIP);
		SetTool(IDC_STATIC_ADVADMIN_USERLEVEL,IDS_STATIC_ADVADMIN_USERLEVEL_TIP);
		SetTool(IDC_ADVADMIN_NOTE,IDS_ADVADMIN_NOTE_TIP);
	}
}

//>>> [ionix] - ionixguide
void CPPgIonixWebServer::OnHelp()
{
//heApp.ShowHelp();
}

BOOL CPPgIonixWebServer::OnCommand(WPARAM wParam, LPARAM lParam)
{
	if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}
	return __super::OnCommand(wParam, lParam);
}

BOOL CPPgIonixWebServer::OnHelpInfo(HELPINFO* /*pHelpInfo */)
{
	OnHelp();
	return TRUE;
}
//<<< [ionix] - ionixguide

BOOL CPPgIonixWebServer::OnSetActive()
{ 
	if (IsWindow(m_hWnd))
	     SetBoxes();
	return TRUE;
}

afx_msg void CPPgIonixWebServer::SetBoxes()
{
	bool bWSEnalbed = (
		( (theApp.emuledlg->preferenceswnd->m_wndWebServer.GetSafeHwnd()  == NULL) ||
		    theApp.emuledlg->preferenceswnd->m_wndWebServer.GetDlgItem(IDC_WSENABLED) == NULL) 
		   && thePrefs.GetWSIsEnabled()) && theApp.webserver->iMultiUserversion>0 ||
		((theApp.emuledlg->preferenceswnd->m_wndWebServer.GetSafeHwnd()!=  NULL) &&
		theApp.emuledlg->preferenceswnd->m_wndWebServer.GetDlgItem(IDC_WSENABLED) != NULL 
		&& theApp.emuledlg->preferenceswnd->m_wndWebServer.IsDlgButtonChecked(IDC_WSENABLED) &&
		theApp.webserver->iMultiUserversion>0 )
		;

	if(bWSEnalbed && IsDlgButtonChecked(IDC_ADVADMINENABLED)!=0)
	{
		GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
		m_cbAccountSelector.EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_DFILES)->EnableWindow(TRUE);
		m_cbUserlevel.EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(TRUE);
		UpdateSelection();
	}
	else
	{	
		if(bWSEnalbed && theApp.webserver->iMultiUserversion)
			GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(TRUE);
		else
			GetDlgItem(IDC_ADVADMINENABLED)->EnableWindow(FALSE);
		m_cbAccountSelector.EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_KAD)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_TRANSFER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SEARCH)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SERVER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_SHARED)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_STATS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_PREFS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_DFILES)->EnableWindow(FALSE);
		m_cbUserlevel.EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_PASS)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_USER)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_CATS)->EnableWindow(FALSE);
	}
}

//>>> [ionix] - iONiX::Advanced WebInterface Account Management
void CPPgIonixWebServer::UpdateSelection()
{
	int accountsel = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
	{
		//reset all if no selection possible
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(FALSE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(TRUE);
		CheckDlgButton(IDC_ADVADMIN_KAD, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_TRANSFER, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SEARCH, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SERVER, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_SHARED, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_STATS, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_PREFS, BST_UNCHECKED);
		CheckDlgButton(IDC_ADVADMIN_DFILES, BST_UNCHECKED);
		m_cbUserlevel.SetCurSel(0);
		GetDlgItem(IDC_ADVADMIN_PASS)->SetWindowText(_T(""));
		GetDlgItem(IDC_ADVADMIN_USER)->SetWindowText(_T(""));
		GetDlgItem(IDC_ADVADMIN_CATS)->SetWindowText(_T(""));
	}
	else
	{
		//set all data to our selectors
		GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
		GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
		CheckDlgButton(IDC_ADVADMIN_KAD, tmp.RightsToKad);
		CheckDlgButton(IDC_ADVADMIN_TRANSFER, tmp.RightsToTransfered);
		CheckDlgButton(IDC_ADVADMIN_SEARCH, tmp.RightsToSearch);
		CheckDlgButton(IDC_ADVADMIN_SERVER, tmp.RightsToServers);
		CheckDlgButton(IDC_ADVADMIN_SHARED, tmp.RightsToSharedList);
		CheckDlgButton(IDC_ADVADMIN_STATS, tmp.RightsToStats);
		CheckDlgButton(IDC_ADVADMIN_PREFS, tmp.RightsToPrefs);
		CheckDlgButton(IDC_ADVADMIN_DFILES, tmp.RightsToDownloadFiles);
		m_cbUserlevel.SetCurSel(tmp.RightsToAddRemove);
		GetDlgItem(IDC_ADVADMIN_PASS)->SetWindowText(HIDDEN_PASSWORD);
		GetDlgItem(IDC_ADVADMIN_USER)->SetWindowText(tmp.User);
		GetDlgItem(IDC_ADVADMIN_CATS)->SetWindowText(tmp.RightsToCategories);
	}
}

void CPPgIonixWebServer::FillComboBox()
{
	//clear old values first
	m_cbAccountSelector.ResetContent();
	m_cbAccountSelector.InsertString(0, GetResString(IDS_ADVADMIN_NEW));
	for(POSITION pos = theApp.webserver->AdvLogins.GetHeadPosition(); pos; theApp.webserver->AdvLogins.GetNext(pos))
		m_cbAccountSelector.InsertString(theApp.webserver->AdvLogins.GetKeyAt(pos), theApp.webserver->AdvLogins.GetValueAt(pos).User);	
}

void CPPgIonixWebServer::FillUserlevelBox()
{
	//clear old values first
	m_cbUserlevel.ResetContent();
	m_cbUserlevel.InsertString(0, GetResString(IDS_ADVADMIN_GUEST));
	m_cbUserlevel.InsertString(1, GetResString(IDS_ADVADMIN_OPERATOR));
	m_cbUserlevel.InsertString(2, GetResString(IDS_ADVADMIN_ADMIN));
	m_cbUserlevel.InsertString(3, GetResString(IDS_ADVADMIN_HIADMIN));
}


afx_msg void CPPgIonixWebServer::OnMultiPWChange()
{
	int accountsel  = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel  == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
		return;

	CString buffer;
	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD && MD5Sum(buffer).GetHash() != tmp.Pass)
		OnSettingsChange();
}

afx_msg void CPPgIonixWebServer::OnMultiCatsChange()
{
	int accountsel  = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel  == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
		return;

	CString buffer;

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD && buffer != tmp.RightsToCategories)
		OnSettingsChange();
}

#define SET_TCHAR_TO_STRING(t, s) {_stprintf(t, _T("%s"), s);}

void CPPgIonixWebServer::OnSettingsChange()
{
	SetModified();

	int accountsel  = m_cbAccountSelector.GetCurSel();
	WebServDef tmp;
	if(accountsel  == -1 || !theApp.webserver->AdvLogins.Lookup(accountsel , tmp))
		return;

	tmp.RightsToKad = IsDlgButtonChecked(IDC_ADVADMIN_KAD)!=0;
	tmp.RightsToTransfered = IsDlgButtonChecked(IDC_ADVADMIN_TRANSFER)!=0;
	tmp.RightsToSearch = IsDlgButtonChecked(IDC_ADVADMIN_SEARCH)!=0;
	tmp.RightsToServers = IsDlgButtonChecked(IDC_ADVADMIN_SERVER)!=0;
	tmp.RightsToSharedList = IsDlgButtonChecked(IDC_ADVADMIN_SHARED)!=0;
	tmp.RightsToStats = IsDlgButtonChecked(IDC_ADVADMIN_STATS)!=0;
	tmp.RightsToPrefs = IsDlgButtonChecked(IDC_ADVADMIN_PREFS)!=0;
	tmp.RightsToDownloadFiles = IsDlgButtonChecked(IDC_ADVADMIN_DFILES)!=0;
	//tmp.RightsToAddRemove = IsDlgButtonChecked(IDC_ADVADMIN_ADMIN)!=0;
	int j = m_cbUserlevel.GetCurSel();
	ASSERT(j <= 3); //only 0,1,2,3 allowed
	tmp.RightsToAddRemove = (uint8) j;
	
	CString buffer;
	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD)
		SET_TCHAR_TO_STRING(tmp.Pass, MD5Sum(buffer).GetHash());
	
	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.RightsToCategories, buffer);

	theApp.webserver->AdvLogins.SetAt(accountsel , tmp);

	FillComboBox();
	m_cbAccountSelector.SetCurSel(accountsel );
}

void CPPgIonixWebServer::OnBnClickedNew()
{

    WebServDef tmp;
	CString buffer;
	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);
    if (_tcslen(tmp.User)==0) // username not filled?
       return; 

	SetModified();

	int i = theApp.webserver->AdvLogins.IsEmpty() ? 1 : theApp.webserver->AdvLogins.GetCount()+1;

	tmp.RightsToKad = IsDlgButtonChecked(IDC_ADVADMIN_KAD)!=0;
	tmp.RightsToTransfered = IsDlgButtonChecked(IDC_ADVADMIN_TRANSFER)!=0;
	tmp.RightsToSearch = IsDlgButtonChecked(IDC_ADVADMIN_SEARCH)!=0;
	tmp.RightsToServers = IsDlgButtonChecked(IDC_ADVADMIN_SERVER)!=0;
	tmp.RightsToSharedList = IsDlgButtonChecked(IDC_ADVADMIN_SHARED)!=0;
	tmp.RightsToStats = IsDlgButtonChecked(IDC_ADVADMIN_STATS)!=0;
	tmp.RightsToPrefs = IsDlgButtonChecked(IDC_ADVADMIN_PREFS)!=0;
	tmp.RightsToDownloadFiles = IsDlgButtonChecked(IDC_ADVADMIN_DFILES)!=0;
	//tmp.RightsToAddRemove = IsDlgButtonChecked(IDC_ADVADMIN_ADMIN)!=0;
	int j = m_cbUserlevel.GetCurSel();
	ASSERT(j <= 3); //only 0,1,2,3 allowed
	tmp.RightsToAddRemove = (uint8) j;

	GetDlgItem(IDC_ADVADMIN_PASS)->GetWindowText(buffer);
	if(buffer != HIDDEN_PASSWORD)
		SET_TCHAR_TO_STRING(tmp.Pass, MD5Sum(buffer).GetHash());

	GetDlgItem(IDC_ADVADMIN_USER)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.User, buffer);

	GetDlgItem(IDC_ADVADMIN_CATS)->GetWindowText(buffer);
	SET_TCHAR_TO_STRING(tmp.RightsToCategories, buffer);

	theApp.webserver->AdvLogins.SetAt(i, tmp);
	theApp.webserver->SaveWebServConf(); //DEBUG should only be done at apply?
	FillComboBox();
	m_cbAccountSelector.SetCurSel(i);
    UpdateSelection();


	GetDlgItem(IDC_ADVADMIN_DELETE)->EnableWindow(TRUE);
	GetDlgItem(IDC_ADVADMIN_NEW)->EnableWindow(FALSE);
}
//<<< [ionix] - iONiX::Advanced WebInterface Account Management

void CPPgIonixWebServer::OnBnClickedDel()
{
	SetModified();

	const int i = m_cbAccountSelector.GetCurSel();  
    WebServDef tmp;  
    if(i == -1 || !theApp.webserver->AdvLogins.Lookup(i, tmp))   
         return;  
  
	CRBMap<uint32, WebServDef> tmpmap;  
  
    //retrieve all "wrong" entries  
    for(POSITION pos = theApp.webserver->AdvLogins.GetHeadPosition(); pos;)  
    {  
         POSITION pos2 = pos;  
         theApp.webserver->AdvLogins.GetNext(pos);  
  
         const int j = theApp.webserver->AdvLogins.GetKeyAt(pos2);  
         if(j == i)  
              theApp.webserver->AdvLogins.RemoveAt(pos2);  
         else if(j > i)  
         {  
              tmpmap.SetAt(j-1, theApp.webserver->AdvLogins.GetValueAt(pos2));  
              theApp.webserver->AdvLogins.RemoveAt(pos2);  
         }  
    }
	
	//reinsert all "wrong" entries correctly
	for(POSITION pos = tmpmap.GetHeadPosition(); pos; tmpmap.GetNext(pos))
		theApp.webserver->AdvLogins.SetAt(tmpmap.GetKeyAt(pos), tmpmap.GetValueAt(pos));
	
	FillComboBox();
	m_cbAccountSelector.SetCurSel(0); //set to the empty field
	UpdateSelection();

	
}
//<<< [ionix] - iONiX::Advanced WebInterface Account Management

// MORPH start tabbed option [leuk_he]
void CPPgIonixWebServer::InitTab(bool firstinit, int Page)
{
	if (m_tabCtr.GetSafeHwnd() != NULL  && firstinit ) {
		m_tabCtr.DeleteAllItems();
		m_tabCtr.SetImageList(&m_imageList);
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, WEBSERVER, GetResString(IDS_TAB_WEB_SERVER), 0, (LPARAM)WEBSERVER); 
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, MULTIWEBSERVER, GetResString(IDS_TAB_MULTI_USER), 0, (LPARAM)MULTIWEBSERVER);
		m_tabCtr.InsertItem(TCIF_TEXT | TCIF_IMAGE | TCIF_PARAM, NTSERVICE, GetResString(IDS_TAB_NT_SERVICE), 0, (LPARAM)MULTIWEBSERVER);
	}
    if (m_tabCtr.GetSafeHwnd() != NULL     )
	   m_tabCtr.SetCurSel(Page);
}
void CPPgIonixWebServer::OnTcnSelchangeTab(NMHDR * /*pNMHDR */, LRESULT *pResult)
{
	int cur_sel = m_tabCtr.GetCurSel();
	theApp.emuledlg->preferenceswnd->SwitchTab(cur_sel);
	*pResult = 0;
}
// MORPH end tabbed option [leuk_he]

