// DlgAdsOption.cpp : 
//

#include "stdafx.h"
#include "emule.h"
#include "DlgAdsOption.h"


IMPLEMENT_DYNCREATE(CDlgAdsOption, CDHtmlDialog)

CDlgAdsOption::CDlgAdsOption(CWnd* pParent /*=NULL*/)
	: CDHtmlDialog(CDlgAdsOption::IDD, CDlgAdsOption::IDH, pParent)
{
}

CDlgAdsOption::~CDlgAdsOption()
{
}

void CDlgAdsOption::DoDataExchange(CDataExchange* pDX)
{
	CDHtmlDialog::DoDataExchange(pDX);
}

BOOL CDlgAdsOption::OnInitDialog()
{
	CDHtmlDialog::OnInitDialog();
	return TRUE; 
}

BEGIN_MESSAGE_MAP(CDlgAdsOption, CDHtmlDialog)
END_MESSAGE_MAP()

BEGIN_DHTML_EVENT_MAP(CDlgAdsOption)
	DHTML_EVENT_ONCLICK(_T("ButtonOK"), OnButtonOK)
	DHTML_EVENT_ONCLICK(_T("ButtonCancel"), OnButtonCancel)
END_DHTML_EVENT_MAP()


STDMETHODIMP CDlgAdsOption::ShowContextMenu(DWORD /*dwID*/, POINT* /*ppt*/, IUnknown* /*pcmdtReserved*/, IDispatch* /*pdispReserved*/)
{
	return S_OK;	
}

// CDlgAdsOption Message processing.

HRESULT CDlgAdsOption::OnButtonOK(IHTMLElement* /*pElement*/)
{
	OnOK();
	return S_OK; 
}

HRESULT CDlgAdsOption::OnButtonCancel(IHTMLElement* /*pElement*/)
{
	OnCancel();
	return S_OK; 
}
