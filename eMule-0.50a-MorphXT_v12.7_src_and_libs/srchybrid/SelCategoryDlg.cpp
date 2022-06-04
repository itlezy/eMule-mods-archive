// khaos::categorymod+
// SelCategoryDlg.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "SelCategoryDlg.h"
#include "OtherFunctions.h"
#include "emuleDlg.h"
#include "TransferDlg.h"
#include "DownloadQueue.h"
#include "ED2KLink.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

// CSelCategoryDlg dialog

IMPLEMENT_DYNAMIC(CSelCategoryDlg, CDialog)

CSelCategoryDlg::CSelCategoryDlg(CWnd* /*pWnd*/,bool bFromClipboard)
    :CPPgtooltippedDialog(CSelCategoryDlg::IDD)
{
	// If they have selected to use the active category as the default
	// when adding links, then set m_Return to it.  Otherwise, use 'All' (0).
	if (!bFromClipboard && thePrefs.UseActiveCatForLinks())
		m_Return =	theApp.emuledlg->transferwnd->GetActiveCategory();
	else
		m_Return = 0;
   m_bFromClipboard=   bFromClipboard; // Morph added by leuk_he (mixing of clipboard and non clipboard can occur, clipboard overrides)
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

   InitTooltips();

	// Load the language strings.
	GetDlgItem(IDC_STATIC_INS)->SetWindowText(GetResString(IDS_CAT_SELDLGTXT));
	GetDlgItem(IDCANCEL)->SetWindowText(GetResString(IDS_CANCEL));
	SetWindowText(GetResString(IDS_CAT_SELDLGCAP));
     // localize & tooltip added by leuk_he

	CString ListFilesNames;

	if (theApp.downloadqueue->m_ED2KLinkQueue.GetCount())
	{   // show the list how it is when the dialog is constructed. 
		for (POSITION pos = theApp.downloadqueue->m_ED2KLinkQueue.GetHeadPosition(); pos != 0; theApp.downloadqueue->m_ED2KLinkQueue.GetNext(pos))
		{
			ListFilesNames +=  theApp.downloadqueue->m_ED2KLinkQueue.GetAt(pos)->GetName();
			ListFilesNames +=   _T("\r\n");
		}
		GetDlgItem(IDC_SELFILES)->SetWindowText(ListFilesNames.Left(ListFilesNames.GetLength()-2));
		SetTool(IDC_SELFILES,IDS_SELFILES_TIP);
	}
	else // no need to display empty box
		GetDlgItem(IDC_SELFILES)->ShowWindow(SW_HIDE);

	if (m_bFromClipboard) {
		GetDlgItem(IDC_DONTASKMEAGAINCB)->SetWindowText(GetResString(IDS_DONOTWATCHCLIP    ));
        SetTool(IDC_DONTASKMEAGAINCB,IDS_DONOTWATCHCLIP_TIP );
	}
	else
	{
    	GetDlgItem(IDC_DONTASKMEAGAINCB)->SetWindowText(GetResString(IDS_DONOTASKAGAIN));
        SetTool(IDC_DONTASKMEAGAINCB,IDC_DONTASKMEAGAINCBSEL_TIP);
	}

	SetTool(IDC_STATIC_INS,IDS_CAT_SELDLGTXT_TIP);
	SetTool(IDC_CATCOMBO,IDS_CAT_SELDLGTXT_TIP);
	SetTool(IDCANCEL,IDS_OKCANCELSEL_TIP);
	SetTool(IDOK,IDS_OKCANCELSEL_TIP);
	

	// 'All' is always an option.
	//khaos::categorymod+
	/*
	((CComboBox*)GetDlgItem(IDC_CATCOMBO))->AddString(GetResString(IDS_ALL) + _T("/") + GetResString(IDS_CAT_UNASSIGN));

	// If there are more categories, add them to the list.
	if (thePrefs.GetCatCount() > 1)
		for (int i=1; i < thePrefs.GetCatCount(); i++)
	*/
		for (int i=0; i < thePrefs.GetCatCount(); i++)
	//khaos::categorymod-
			((CComboBox*)GetDlgItem(IDC_CATCOMBO))->AddString(thePrefs.GetCategory(i)->strTitle);

	// Select the category that is currently visible in the transfer dialog as default, or 0 if they are
	// not using "New Downloads Default To Active Category"
	((CComboBox*)GetDlgItem(IDC_CATCOMBO))->SetCurSel(thePrefs.UseActiveCatForLinks()?theApp.emuledlg->transferwnd->GetActiveCategory():0);

   // default setting for do not ask again is watch ed2k clipboard or select cat for new dl.
   if(m_bFromClipboard?thePrefs.watchclipboard:thePrefs.SelectCatForNewDL())
		CheckDlgButton(IDC_DONTASKMEAGAINCB,0);
	else
		CheckDlgButton(IDC_DONTASKMEAGAINCB,1);

	return TRUE;
}

void CSelCategoryDlg::OnOK()
{

  if (m_bFromClipboard) 
	  thePrefs.watchclipboard =(IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);
  else
  	  thePrefs.m_bSelCatOnAdd= (IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);

  m_cancel = false; //MORPH - Added by SiRoB
  int	comboIndex = ((CComboBox*)GetDlgItem(IDC_CATCOMBO))->GetCurSel();

	if(comboIndex >= 0)
	{
		CString* catTitle = new CString(thePrefs.GetCategory(comboIndex)->strTitle);
		catTitle->Trim();

		CString	comboText;
		((CComboBox*)GetDlgItem(IDC_CATCOMBO))->GetWindowText(comboText);
		comboText.Trim();

		//khaos::categorymod+
		/*
		if (catTitle->CompareNoCase(comboText) == 0 || (comboIndex == 0 && comboText.Compare(GetResString(IDS_ALL) + _T("/") + GetResString(IDS_CAT_UNASSIGN)) == 0))
		*/
		if (catTitle->CompareNoCase(comboText) == 0 || (comboIndex == 0 && comboText.Compare(thePrefs.GetCategory(0)->strTitle) == 0))
		//khaos::categorymod-
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

  if (m_bFromClipboard) 
	  thePrefs.watchclipboard =(IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);
  else
  	  thePrefs.m_bSelCatOnAdd= (IsDlgButtonChecked(IDC_DONTASKMEAGAINCB)==0);

  // m_Return will still be default, so we don't have to process the link. 
	CDialog::OnCancel();
}