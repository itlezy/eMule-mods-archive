#pragma once

// Control flags
#define HTC_ISAUTOSCROLL	0x0000001
#define HTC_ISWORDWRAP		0x0000002
#define HTC_ISLIMITED		0x0000004
#define HTC_ISDEFAULTLINKS	0x0000008
#define HTC_ISARROWCURSOR	0x0000010

// Text style flags
#define HTC_BOLD			0x0000001
#define HTC_ITALIC			0x0000002
#define HTC_UNDERLINE		0x0000004
#define HTC_STRIKEOUT		0x0000008
#define HTC_HAVENOLINK		0x0000010
#define HTC_LINK			0x0000020

typedef struct
{
	LPCTSTR		pszScheme;
	unsigned	uiLen;
} LinkDetect_Table;

#define LINKDETECT_TABLESZ	12
extern LinkDetect_Table		_apszSchemes[LINKDETECT_TABLESZ];

class CHTRichEditCtrl : public CRichEditCtrl
{
	DECLARE_DYNAMIC(CHTRichEditCtrl)

public:
	CHTRichEditCtrl();
	virtual ~CHTRichEditCtrl();

	void SetTitle(LPCTSTR pszTitle);
	void Reset();
	void GetLastLogEntry(CString *pstrOut);

	void AppendText(LPCTSTR pszMsg, int iMsgLen, COLORREF crTextColor = CLR_DEFAULT, COLORREF crBackColor = CLR_DEFAULT, DWORD dwFlags = 0);
	void AppendText(const CString &strMsg, COLORREF crTextColor = CLR_DEFAULT, COLORREF crBackColor = CLR_DEFAULT, DWORD dwFlags = 0);

	CString GetHtml();
	CString GetToolTip();

	void SetFont(CFont* pFont, bool bRedraw = true);
	void ScrollToLastLine();

	DWORD				m_dwFlags;
	COLORREF			m_crDefaultForeground;
	COLORREF			m_crDefaultBackground;
	CString				m_strURL;

protected:
	CString				m_strTitle;
	long				m_lMaxBufSize;
	HCURSOR				m_hArrowCursor;

	void SaveLogToDisk();
	void SaveRtfToDisk();

	DECLARE_MESSAGE_MAP()
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg BOOL OnEnLink(NMHDR *pNMHDR, LRESULT *pResult);

private:
	static DWORD __stdcall MEditStreamOutCallbackA(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
	{
		CStringA		*pstrBufferA = reinterpret_cast<CStringA*>(dwCookie);

		pstrBufferA->Append(reinterpret_cast<char*>(pbBuff), cb);
		*pcb = cb;

		return 0;
	}
	static DWORD __stdcall MEditStreamOutCallbackW(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
	{
		CStringW		*pstrBufferW = reinterpret_cast<CStringW*>(dwCookie);

		pstrBufferW->Append(reinterpret_cast<WCHAR*>(pbBuff), cb/2);;
		*pcb = cb;

		return 0;
	}
};
