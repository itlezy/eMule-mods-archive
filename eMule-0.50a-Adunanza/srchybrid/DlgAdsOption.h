#pragma once

class CDlgAdsOption : public CDHtmlDialog
{
	DECLARE_DYNCREATE(CDlgAdsOption)

public:
	CDlgAdsOption(CWnd* pParent = NULL);
	virtual ~CDlgAdsOption();
	HRESULT OnButtonOK(IHTMLElement *pElement);
	HRESULT OnButtonCancel(IHTMLElement *pElement);
	enum { IDD = IDD_ADSDISABLE_OPTION, IDH = IDR_HTML_DLGADSOPTION };

protected:
   	STDMETHOD(ShowContextMenu)(DWORD dwID, POINT* ppt, IUnknown* pcmdtReserved, IDispatch* pdispReserved);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	DECLARE_DHTML_EVENT_MAP()
};
