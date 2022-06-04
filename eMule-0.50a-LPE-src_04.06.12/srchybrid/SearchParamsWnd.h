#pragma once

#include "EditX.h"
#include "ComboBoxEx2.h"
#include "ListCtrlEditable.h"

typedef enum EOptsRows
{
	orMinSize,
	orMaxSize,
	orAvailability,
	orCompleteSources,
	orExtension,
	orCodec,
	orBitrate,
	orLength,
	orTitle,
	orAlbum,
	orArtist
};

class CSearchParamsWnd : public CDialogBar
{
	DECLARE_DYNAMIC(CSearchParamsWnd);

// Construction
public:
	CSearchParamsWnd();
	virtual ~CSearchParamsWnd();

// Dialog Data
	enum { IDD = IDD_SEARCH_PARAMS };

	CEditX m_ctlName;
	CButton m_ctlStart;
	CButton m_ctlCancel;
	CButton m_ctlMore;
	CSearchResultsWnd* m_searchdlg;

	void LocalizeAll();// X: [RUL] - [Remove Useless Localize]
	void Localize();

	SSearchParams* GetParameters();
	void SetParameters(const SSearchParams* pParams);
	void ProcessEd2kSearchLinkRequest(CString strSearchTerm);

	virtual CSize CalcDynamicLayout(int, DWORD nMode);
	virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	HCURSOR m_hcurMove;
	CComboBoxEx2 m_ctlMethod;
	CComboBoxEx2 m_ctlFileType;
	CEditableListCtrl m_ctlOpts;
	CRect m_rcNameLbl;
	CRect m_rcName;
	CRect m_rcFileTypeLbl;
	CRect m_rcFileType;
	CRect m_rcReset;
	CRect m_rcMethodLbl;
	CRect m_rcMethod;
	CRect m_rcOpts;
	CRect m_rcStart;
	CRect m_rcMore;
	CRect m_rcCancel;
	CSize m_szMRU;
	CSize m_szFloat;

	void UpdateControls();
	void InitMethodsCtrl();
	void InitFileTypesCtrl();
	uint64 GetSearchAttrSize(const CString& rstrExpr);
	uint_ptr GetSearchAttrNumber(const CString& rstrExpr);
	uint_ptr GetSearchAttrLength(const CString& rstrExpr);

	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnInitDialog(WPARAM, LPARAM);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedStart();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedMore();
	afx_msg void OnCbnSelChangeMethod();
	afx_msg void OnCbnSelEndOkMethod();
	afx_msg void OnDDClicked();
	afx_msg void OnBnClickedSearchReset();
	afx_msg void OnEnChangeName();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
};
