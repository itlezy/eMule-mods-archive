// MsgViewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MsgViewer.h"
#include "MsgViewDlg.h"

#include "Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CMsgViewDlg dialog

CString UnicodeToCString(USHORT* pArr, DWORD dwCount)
{
#ifdef _UNICODE
	return CString(pArr, dwCount);
#endif // _UNICODE

	char szBufStat[0x100];
	PSTR szBuf = (dwCount > _countof(szBufStat)) ? new char[dwCount] : szBufStat;

	ASSERT(szBuf);
	if (dwCount)
		VERIFY(WideCharToMultiByte(CP_ACP, 0, (PWSTR) pArr, dwCount, szBuf, dwCount, NULL, NULL));

	CString strRet(szBuf, dwCount);

	if (szBuf != szBufStat)
		delete[] szBuf;

	return strRet;
}

CMsgViewDlg::CMsgViewDlg(LPCVOID pData, DWORD dwDataSize, __int64 nPos, ULONG nIdProtoType, CWnd* pParent /*=NULL*/)
	: CSizeableDlg(CMsgViewDlg::IDD, pParent), m_pData(pData), m_dwDataSize(dwDataSize)
{
	//{{AFX_DATA_INIT(CMsgViewDlg)
	m_strMsgData = _T("");
	//}}AFX_DATA_INIT

	ASSERT(pData);

	m_strMsgData.Format(_T("\tFile offset %08X:%08X, "), (DWORD) (nPos >> 0x20), (DWORD) nPos);
/*
	CStream_Mem stStream;
	stStream.m_pPtr = (PBYTE) m_pData;
	stStream.m_dwSize = m_dwDataSize;


	COpCode* pMsg = COpCode::Read(stStream, (UCHAR) nIdProtoType, (UCHAR) (nIdProtoType >> 8), (T_CLIENT_TYPE) (nIdProtoType >> 16));
	if (pMsg)
	{
		CString strDelta;

#define BEGIN_API_MSG(name, id) case id: { m_strMsgData += "\r\n\t"; m_strMsgData += #name; m_strMsgData += "\r\n\r\n"; CApiMsg_##name* pFmtMsg = (CApiMsg_##name*) pMsg;
#define END_API_MSG } break;
#define PARAM_SIMPLE(name, type) strDelta.Format(_T("\t%s = %d (%08X)\r\n"), #name, pFmtMsg->_##name, pFmtMsg->_##name); m_strMsgData += strDelta;
#define PARAM_ARR(name, type) strDelta.Format(_T("\t%s = [%d]\r\n"), #name, pFmtMsg->_##name##_Count); m_strMsgData += strDelta;
#undef PARAM_RECT
#define PARAM_RECT(name) strDelta.Format(_T("\t%s = { %d, %d, %d, %d }\r\n"), #name, pFmtMsg->_##name.left, pFmtMsg->_##name.top, pFmtMsg->_##name.right, pFmtMsg->_##name.bottom); m_strMsgData += strDelta;
#undef PARAM_STR
#define PARAM_STR(name)	strDelta.Format(_T("\t%s = %s\r\n"), #name, UnicodeToCString(pFmtMsg->_##name##_Ptr, pFmtMsg->_##name##_Count)); m_strMsgData += strDelta;
#undef PARAM_POINT
#define PARAM_POINT(name) strDelta.Format(_T("\t%s = %d.%d\r\n"), #name, pFmtMsg->_##name.x, pFmtMsg->_##name.y); m_strMsgData += strDelta;

		switch (pMsg->GetID())
		{

#include "../Shared/ApiMsgDefs.h"

#undef BEGIN_API_MSG
#undef END_API_MSG
#undef PARAM_SIMPLE
#undef PARAM_ARR
#undef PARAM_RECT
		}

		delete pMsg;

	} else
		m_strMsgData += _T("\r\n\t\tThe message is either invalid or unknown\r\n");
*/
	// now - write the message's binary data
	m_strMsgData += _T("\r\n\t\tBinary representation\r\n");

	for (DWORD dwPos = 0; dwPos < dwDataSize; dwPos += 0x10)
	{
		if (dwPos >= 8192)
		{
			AfxMessageBox(_T("Message too long. Only first 8192 bytes diplayed"));
			break;
		}

		// Write offset
		CString strLineBin, strLineAsc;
		strLineBin.Format(_T("%08X  "), dwPos);

		for (DWORD dwLinePos = 0; dwLinePos < 0x10; dwLinePos++)
		{
			CString strDelta;
			if (dwPos + dwLinePos < dwDataSize)
			{
				char chThis = ((PCSTR) pData)[dwPos + dwLinePos];
				strDelta.Format(_T("%02X "), (DWORD) (BYTE) chThis);

				TCHAR wchThis = _T('.');
				if ((chThis > 9) && !OemToCharBuff(&chThis, &wchThis, 1))
					wchThis = _T('?');
				strLineAsc += wchThis;

			} else
			{
				strDelta = _T("   ");
				strLineAsc += _T(' ');
			}

			if (7 == (dwLinePos % 8))
				strDelta += _T(' ');

			strLineBin += strDelta;
		}

		strLineBin += _T("  ");
		strLineBin += strLineAsc;
		strLineBin += _T("\r\n");

		m_strMsgData += strLineBin;
	}
}


void CMsgViewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMsgViewDlg)
	DDX_Text(pDX, IDC_MSGDATA, m_strMsgData);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMsgViewDlg, CDialog)
	//{{AFX_MSG_MAP(CMsgViewDlg)
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_EXPORT, OnExport)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CMsgViewDlg message handlers

void CMsgViewDlg::OnSize(UINT nType, int cx, int cy) 
{
	if (0xFFFFFFFF != nType)
		CDialog::OnSize(nType, cx, cy);

	OnSizeControl(IDC_MSGDATA, cx, cy, 10);
	OnSizeControl(IDOK, cx, cy, 15);
	OnSizeControl(IDC_EXPORT, cx, cy, 15);
}


void CMsgViewDlg::OnDestroy() 
{
	CDialog::OnDestroy();

	// Save our size
	CRect stRect;
	GetWindowRect(&stRect);
	VERIFY(AfxGetApp()->WriteProfileBinary(_T("MsgView"), _T("MsgView"), (PBYTE) &stRect, sizeof(stRect)));
	
}

BOOL CMsgViewDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	CRect stRect;
	GetClientRect(&stRect);
	OnSize(0xFFFFFFFF, stRect.right, stRect.bottom);

	// apply the stored size
	UINT nSize = 0;
	PBYTE pData = NULL;

	if (AfxGetApp()->GetProfileBinary(_T("MsgView"), _T("MsgView"), &pData, &nSize))
	{
		if (sizeof(CRect) == nSize)
			MoveWindow((RECT*) pData);

		delete[] pData;
	}

	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CMsgViewDlg::OnExport() 
{
	TCHAR szFileName[MAX_PATH] = _T("");

	OPENFILENAME stOFN;
	ZeroMemory(&stOFN, sizeof(stOFN));
	stOFN.lStructSize = sizeof(stOFN);
	stOFN.hwndOwner = m_hWnd;
	stOFN.hInstance = AfxGetInstanceHandle();
	stOFN.lpstrFile = szFileName;
	stOFN.nMaxFile = MAX_PATH;
	stOFN.lpstrTitle = _T("Export eMule message");
	stOFN.Flags = OFN_ENABLESIZING | OFN_OVERWRITEPROMPT;
	stOFN.lpstrFilter = _T("eMule single message (*.bin)\0*.bin\0All Files (*.*)\0*.*\0");
	if (GetSaveFileName(&stOFN))
	{
		HANDLE hFile = CreateFile(szFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (INVALID_HANDLE_VALUE != hFile)
		{
			DWORD dwWritten = 0;
			BOOL bSuccess = 
				WriteFile(hFile, m_pData, m_dwDataSize, &dwWritten, NULL) &&
				(dwWritten == m_dwDataSize);

			VERIFY(CloseHandle(hFile));

			if (!bSuccess)
				AfxMessageBox(_T("There was an error while saving the message"), MB_ICONERROR);

		} else
			AfxMessageBox(_T("Failed to open the file for writing"), MB_ICONERROR);
	}
}
