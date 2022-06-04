// khaos::categorymod+
// SelCategoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "SelCategoryDlg.h"
#include "OtherFunctions.h"
#include "emuleDlg.h"
#include "TransferDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// CSelCategoryDlg dialog

IMPLEMENT_DYNAMIC(CSelCategoryDlg, CDialog)

CSelCategoryDlg::CSelCategoryDlg(CWnd* /*pWnd*/)
    :CPropertyPage(CSelCategoryDlg::IDD)
{
	// If they have selected to use the active category as the default
	// when adding links, then set m_Return to it.  Otherwise, use 'All' (0).
	if (thePrefs.UseActiveCatForLinks())
		m_Return =	theApp.emuledlg->transferwnd->GetActiveCategory();
	else
		m_Return = 0;

	m_bCreatedNew = false;
	m_cancel = true; //MORPH - Added by SiRoB
}



CSelCategoryDlg::~CSelCategoryDlg()
{
}

void CSelCategoryDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CSelCategoryDlg, CDialog)
END_MESSAGE_MAP()

BOOL CSelCategoryDlg::OnInitDialog()
{
   CDialog::OnInitDialog();

//   InitTooltips();

	// Load the language strings.
	GetDlgItem(IDC_STATIC_INS)->SetWindowText(GetResString(IDS_CAT_SELDLGTXT));
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
	SetWindowText(GetResString(IDS_CAT_SELDLGCAP));
     // localize & tooltip added by leuk_he
	GetDlgItem(IDC_DONTASKMEAGAINCB)->SetWindowText(GetResString(IDS_DONOTASKAGAIN));
//	SetTool(IDC_DONTASKMEAGAINCB,IDC_DONTASKMEAGAINCBSEL_TIP);
//	SetTool(IDC_STATIC_INS,IDS_CAT_SELDLGTXT_TIP);
//	SetTool(IDC_CATCOMBO,IDS_CAT_SELDLGTXT_TIP);
//	SetTool(IDCANCEL,IDS_OKCANCELSEL_TIP);
//	SetTool(IDOK,IDS_OKCANCELSEL_TIP);
	

	// 'All' is always an option.
	((CComboBox*)GetDlgItem(IDC_CATCOMBO))->AddString(GetResString(IDS_ALL) + _T("/") + GetResString(IDS_CAT_UNASSIGN));

	// If there are more categories, add them to the list.
	if (thePrefs.GetCatCount() > 1)
		for (int i=1; i < thePrefs.GetCatCount(); i++)
					((CComboBox*)GetDlgItem(IDC_CATCOMBO))->AddString(thePrefs.GetCategory(i)->strTitle);

	// Select the category that is currently visible in the transfer dialog as default, or 0 if they are
	// not using "New Downloads Default To Active Category"
	((CComboBox*)GetDlgItem(IDC_CATCOMBO))->SetCurSel(thePrefs.UseActiveCatForLinks()?theApp.emuledlg->transferwnd->GetActiveCategory():0);
   if(thePrefs.SelectCatForNewDL())
		CheckDlgButton(IDC_DONTASKMEAGAINCB,0);
	else
		CheckDlgButton(IDC_DONTASKMEAGAINCB,1);

	return TRUE;
}

void CSelCategoryDlg::OnOK()
{
	thePrefs.m_bSelCatOnAdd= (IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0); // leuk_he add don't ask me again
	m_cancel = false; //MORPH - Added by SiRoB
	int	comboIndex = ((CComboBox*)GetDlgItem(IDC_CATCOMBO))->GetCurSel();

	if(comboIndex >= 0)
	{
		CString* catTitle = new CString(thePrefs.GetCategory(comboIndex)->strTitle);
		catTitle->Trim();

		CString	comboText;
		((CComboBox*)GetDlgItem(IDC_CATCOMBO))->GetWindowText(comboText);
		comboText.Trim();

		if (catTitle->CompareNoCase(comboText) == 0 || (comboIndex == 0 && comboText.Compare(GetResString(IDS_ALL) + _T("/") + GetResString(IDS_CAT_UNASSIGN)) == 0))
			m_Return = comboIndex;
		else {
			m_bCreatedNew = true;
			m_Return = theApp.emuledlg->transferwnd->AddCategory(comboText, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), _T(""),_T(""));
		}

		delete catTitle;
		catTitle = NULL;
	}
	else
	{
		CString	comboText;
		((CComboBox*)GetDlgItem(IDC_CATCOMBO))->GetWindowText(comboText);
		comboText.Trim();

		m_bCreatedNew = true;
		m_Return = theApp.emuledlg->transferwnd->AddCategory(comboText, thePrefs.GetMuleDirectory(EMULE_INCOMINGDIR), _T(""),_T(""));
	}

	CDialog::OnOK();
}

void CSelCategoryDlg::OnCancel()
{
	  thePrefs.m_bSelCatOnAdd= (IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);
	// m_Return will still be default, so we don't have to do a darn thing here.
	CDialog::OnCancel();
}