#pragma once

/////////////////////////////////////////////////////////////////////////////
// CProcessFileDlg dialog

#include <afxtempl.h>

struct MSG_FILE_INFO {
	HANDLE m_hFile;

	__int64 m_64nSizeFile;
	__int64 m_64nSizeLeft;

	CString m_strTmpFile;

	struct MSG_DATA {
		FILETIME	m_stTime;
		DWORD		m_dwSize;
		BYTE		m_nID;
		BYTE		m_nProto;
		ULONG		m_nType;
		ULONG		m_nClientID;
		ULONG		m_nIdProtoType;
		BOOL		m_bOut;
		LPCTSTR		m_szName;
		BYTE		m_nFlags; // 1-nested, 2-damage.
		__int64		m_n64Pos;

		MSG_DATA() {}
		MSG_DATA(const MSG_DATA& stOther) { operator = (stOther); }
		void operator = (const MSG_DATA& stOther) { CopyMemory(this, &stOther, sizeof(MSG_DATA)); }
	};

	CArray <MSG_DATA, MSG_DATA&> m_arrMsgData;
	DWORD m_dwFilteredOut;

	BOOL Open(LPCTSTR szFileName, HWND hNotifyWnd);
	void Close();
	volatile BOOL m_bInterrupt;

	void FillListCtrl(CListCtrl&, UINT nFirstMsg, UINT nMaxMsgs);
	
	PVOID ReadMsg(UINT nMsg, DWORD& dwSize);
	BOOL Stretch(PVOID, DWORD dwBufSize, DWORD& dwSize, UINT nMsgIndex);

	MSG_FILE_INFO() : m_hFile(INVALID_HANDLE_VALUE) {}
	~MSG_FILE_INFO() { Close(); }
private:

	void AddNext(const MSG_DATA* pOwner);
	void Read(PVOID pBuf, DWORD dwBufSize);

	class CFileOver {}; // exception

	static void ParseFile(PVOID);

	HWND m_hNotifyWnd;
};

class CProcessFileDlg : public CDialog
{
// Construction

	MSG_FILE_INFO& m_stData;
	const CString& m_strFileName;

public:
	CProcessFileDlg(MSG_FILE_INFO&, const CString&, CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CProcessFileDlg)
	enum { IDD = IDD_PROCESSFILE };
	CProgressCtrl	m_wndProgress;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProcessFileDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CProcessFileDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnFileProcessed();
	afx_msg void OnInterrupt();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
