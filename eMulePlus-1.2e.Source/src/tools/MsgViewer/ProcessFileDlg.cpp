// ProcessFileDlg.cpp : implementation file
//

#include "stdafx.h"
#include "msgviewer.h"
#include "ProcessFileDlg.h"
#include "FilterDlg.h"
#include <process.h>
#include <shlwapi.h>
#pragma comment (lib, "shlwapi.lib")
#include "Common.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

void MSG_FILE_INFO::ParseFile(PVOID pPtr)
{
	MSG_FILE_INFO* pThis = (MSG_FILE_INFO*) pPtr;
	ASSERT(pThis);

	for (pThis->m_dwFilteredOut = 0; !pThis->m_bInterrupt && pThis->m_64nSizeLeft; )
		try
		{
			pThis->AddNext(NULL);
		}
		catch (CFileOver)
		{
			break;
		}

	if (IsWindow(pThis->m_hNotifyWnd))
		PostMessage(pThis->m_hNotifyWnd, WM_COMMAND, IDC_FILE_PROCESSED, 0);
}

void MSG_FILE_INFO::Read(PVOID pBuf, DWORD dwBufSize)
{
	ASSERT(INVALID_HANDLE_VALUE != m_hFile);
	CFileOver stFileOver;

	if (dwBufSize > m_64nSizeLeft)
		throw stFileOver; // no more data left

	if (pBuf)
	{
		DWORD dwRead = 0;
		if (!ReadFile(m_hFile, pBuf, dwBufSize, &dwRead, NULL) || (dwRead != dwBufSize))
			throw stFileOver;
	} else
		// simply move the file pointer
		if ((SetFilePointer(m_hFile, dwBufSize, NULL, FILE_CURRENT) == 0xFFFFFFFF) &&
			(GetLastError() != NO_ERROR))
			throw stFileOver;
	m_64nSizeLeft -= dwBufSize;
}

void MSG_FILE_INFO::AddNext(const MSG_DATA* pOwner)
{
	ASSERT(m_64nSizeLeft);

	MSG_DATA stData;
	ZeroMemory(&stData, sizeof(stData));
	stData.m_n64Pos = m_64nSizeFile - m_64nSizeLeft;

	if (!pOwner)
	{
		// look for the 'safe value'
		const DWORD dwSafeVal = 0xF00FC55C;
		DWORD dwValue = 0;
		BYTE nByte = 0;
		__int64 n64Pos = m_64nSizeLeft;

		while (true)
		{
			Read(&nByte, sizeof(nByte));
			dwValue >>= 8;
			dwValue |= (nByte << 24);
			if (dwValue == dwSafeVal)
				break;
		}
		n64Pos -= m_64nSizeLeft;

		// NOTE: If the damage size is greater than 65535 bytes - we divide it.
		if (dwValue == dwSafeVal)
		{
			ASSERT(n64Pos >= sizeof(DWORD));
			n64Pos -= sizeof(DWORD);
		}

		while (n64Pos)
		{
			stData.m_nFlags = 2; // damage
			stData.m_dwSize = (USHORT) min((__int64) 0xFFFF, n64Pos);
			m_arrMsgData.Add(stData); // add damaged packet
			stData.m_n64Pos += stData.m_dwSize;
			n64Pos -= stData.m_dwSize;
		}

		if (!m_64nSizeLeft)
			return; // enough

		if (dwValue == dwSafeVal)
			stData.m_n64Pos += sizeof(DWORD);

		ASSERT(stData.m_n64Pos == m_64nSizeFile - m_64nSizeLeft);

		// each message record must consist of at least time (FILETIME)
		Read(&stData.m_stTime, sizeof(stData.m_stTime));

		stData.m_n64Pos += sizeof(FILETIME);

	} else
	{
		CopyMemory(&stData.m_stTime, &pOwner->m_stTime, sizeof(stData.m_stTime));
		stData.m_nFlags = 1; // nested
	}

	// read the header
	Read(&stData.m_nClientID, sizeof(stData.m_nClientID));
	Read(&stData.m_nIdProtoType, sizeof(stData.m_nIdProtoType));
	Read(&stData.m_bOut, 1);
	Read(&stData.m_dwSize, sizeof(stData.m_dwSize));
	stData.m_n64Pos += 13; // what we've just read.

	// bypass the message.
	Read(NULL, stData.m_dwSize);
	if (g_stFilter.AllowMsg(stData))
		m_arrMsgData.Add(stData); // ok
	else
		m_dwFilteredOut++;
}

BOOL MSG_FILE_INFO::Open(LPCTSTR szFileName, HWND hNotifyWnd)
{
	Close(); // if opened

	m_hNotifyWnd = hNotifyWnd;

	// If so - it is better to make a copy of this file, before opening it
	PCTSTR szExt = PathFindExtension(szFileName);
	if (_T('.') == *szExt)
		szExt--;

	if (((_T('a') == szExt[0]) || (_T('b') == szExt[0])) && (_T('_') == szExt[-1]))
	{
		// first of all copy this file to another (temporary) location
		TCHAR szTempFolder[MAX_PATH];
		VERIFY(GetTempPath(MAX_PATH, szTempFolder));
		TCHAR szTempFileName[MAX_PATH];
		VERIFY(GetTempFileName(szTempFolder, _T(""), 0, szTempFileName));

		m_strTmpFile = szTempFileName;
		if (!CopyFile(szFileName, m_strTmpFile, FALSE))
		{
			AfxMessageBox(_T("Could not copy this file to a temporary location"));
			return FALSE; // Damn!
		}
		szFileName = m_strTmpFile;
	}

	m_hFile = CreateFile(szFileName, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (INVALID_HANDLE_VALUE == m_hFile)
	{
		AfxMessageBox(_T("Could not open the file for reading"));
		return FALSE; // Damn!
	}

	// Get the file size
	BY_HANDLE_FILE_INFORMATION stBHI;
	VERIFY(GetFileInformationByHandle(m_hFile, &stBHI));

	m_64nSizeFile = stBHI.nFileSizeLow + 0x100000000 * stBHI.nFileSizeHigh;
	m_64nSizeLeft = m_64nSizeFile;

	// now - process the whole file
	m_bInterrupt = FALSE;
	m_arrMsgData.SetSize(0, 10000); // suppose that is optimal

	VERIFY(-1 != _beginthread(ParseFile, 0, this));
	return TRUE;
}

void MSG_FILE_INFO::Close()
{
	if (INVALID_HANDLE_VALUE != m_hFile)
	{
		VERIFY(CloseHandle(m_hFile));
		m_hFile = INVALID_HANDLE_VALUE;
	}

	m_arrMsgData.RemoveAll();

	// delete also previously created temp file
	if (!m_strTmpFile.IsEmpty() && (GetFileAttributes(m_strTmpFile) != 0xFFFFFFFF))
		VERIFY(DeleteFile(m_strTmpFile));
	m_strTmpFile.Empty();
}

void MSG_FILE_INFO::FillListCtrl(CListCtrl& wndList, UINT nFirstMsg, UINT nMaxMsgs)
{
	wndList.DeleteAllItems();

	UINT nStopMsg = min((unsigned) m_arrMsgData.GetSize(), nFirstMsg + nMaxMsgs);

	for (UINT nMsg = nFirstMsg; nMsg < nStopMsg; nMsg++)
	{
		MSG_DATA& stData = m_arrMsgData.GetAt(nMsg);
		// add this item to our list control
		int nListItem = nMsg - nFirstMsg;

		// format the item No
		CString strText;
		strText.Format(_T("%u"), nMsg + 1);
		VERIFY(wndList.InsertItem(nListItem, strText) != -1);

		// set the message index
		VERIFY(wndList.SetItemData(nListItem, nMsg));

		// Time
		SYSTEMTIME stTime;
		if (!FileTimeToSystemTime(&stData.m_stTime, &stTime))
			ZeroMemory(&stTime, sizeof(stTime));

		strText.Format(_T("%02d/%02d/%04d, %02d:%02d-%02d.%03d"), stTime.wDay, stTime.wMonth, stTime.wYear,
			stTime.wHour, stTime.wMinute, stTime.wSecond, stTime.wMilliseconds);
		VERIFY(wndList.SetItemText(nListItem, 1, strText));

		// Size
		PCTSTR szFormat = /*(0xFFFFFFFF == stData.m_dwMsgPosHigh) ? _T("< %u >") : */_T("%u");
		strText.Format(szFormat, stData.m_dwSize);
		VERIFY(wndList.SetItemText(nListItem, 2, strText));

		if (stData.m_nFlags != 2)
		{
			// MsgID
			strText.Format(_T("%u"), stData.m_nID);
			VERIFY(wndList.SetItemText(nListItem, 3, strText));

			// Protocol
			strText.Format(_T("%u"), stData.m_nProto);
			VERIFY(wndList.SetItemText(nListItem, 4, strText));

			// ClientID
			strText.Format(_T("%u"), stData.m_nClientID);
			VERIFY(wndList.SetItemText(nListItem, 5, strText));

			// ClientType
			LPCTSTR szType;
			switch ((T_CLIENT_TYPE) stData.m_nType)
			{
			case T_CLIENT_PEER:
				szType = _T("Peer");
				break;
			case T_CLIENT_SERVER:
				szType = _T("Server");
				break;
			default:
				strText.Format(_T("<%u>"), stData.m_nType);
				szType = strText;
			}
			VERIFY(wndList.SetItemText(nListItem, 6, szType));

			// Description
			VERIFY(wndList.SetItemText(nListItem, 7, stData.m_szName));

		} else
			VERIFY(wndList.SetItemText(nListItem, 7, _T("<Damage>")));
	}
}

PVOID MSG_FILE_INFO::ReadMsg(UINT nMsg, DWORD& dwSize)
{
	ASSERT(nMsg < (unsigned) m_arrMsgData.GetSize());

	MSG_DATA& stData = m_arrMsgData.GetAt(nMsg);
/*
	if (0xFFFFFFFF == stData.m_dwMsgPosHigh)
	{
		// find the preceding message that is not nested
		for (UINT nBack = 1; nBack <= nMsg; nBack++)
		{
			MSG_DATA& stParent = m_arrMsgData.GetAt(nMsg - nBack);
			if (0xFFFFFFFF == stParent.m_dwMsgPosHigh)
				continue; // also nested

			DWORD dwParentSize = 0;
			PVOID pParent = ReadMsg(nMsg - nBack, dwParentSize);
			if (pParent)
			{
				if (Stretch(pParent, dwParentSize, dwSize, nBack - 1))
					return pParent;

				delete[] pParent;
				return NULL;
			}
		}

		return NULL;
	}

	// here we must also check if the position this message is nested. Skip it currently
	__int64 nPos64 = m_64nSizeFile;
	nPos64 -= stData.m_dwMsgPosLow;
	nPos64 -= stData.m_dwMsgPosHigh * 0x100000000;
	LONG nPosHigh = (LONG) (nPos64 >> 0x20);

	if ((0xFFFFFFFF == SetFilePointer(m_hFile, (DWORD) nPos64, &nPosHigh, FILE_BEGIN)) && (NO_ERROR != GetLastError()))
	{
		AfxMessageBox(_T("Could not jump to needed position"));
		return NULL; // could not read
	}
*/

	long nPosHigh = (long) (stData.m_n64Pos >> 0x20);

	if ((0xFFFFFFFF == SetFilePointer(m_hFile, (long) stData.m_n64Pos, &nPosHigh, FILE_BEGIN)) && (NO_ERROR != GetLastError()))
	{
		AfxMessageBox(_T("Could not jump to needed position"));
		return NULL; // could not read
	}

	PBYTE pBuf = new BYTE[stData.m_dwSize];
	if (pBuf)
	{
		if (ReadFile(m_hFile, pBuf, stData.m_dwSize, &dwSize, NULL) && (dwSize == stData.m_dwSize))
			return pBuf;

		delete[] pBuf;
		AfxMessageBox(_T("Could not read from the file"));
	} else
		AfxMessageBox(_T("Not enough memory to show the message! (probably the message is invalid)"));

	return NULL;
}

BOOL MSG_FILE_INFO::Stretch(PVOID pBuf, DWORD dwBufSize, DWORD& dwSize, UINT nMsgIndex)
{
	ASSERT(FALSE);
/*	CIdlMsg_Unknown stMsg;
	if (stMsg.Init(pBuf, dwBufSize) || (stMsg._MsgSize < 6 * sizeof(DWORD)))
		return FALSE;

	DWORD dwInternalSize = stMsg._MsgSize - 6 * sizeof(DWORD);
	PDWORD pData = ((PDWORD) pBuf) + 5;

	if (*pData++ != dwInternalSize)
		return FALSE;

	// now - bypass the needed count of messages
	do
	{
		if ((*pData > dwInternalSize) || (*pData < 2 * sizeof(DWORD)))
			return FALSE;

		if (!nMsgIndex--)
			break;

		dwInternalSize -= *pData;
		pData = (PDWORD) (((PBYTE) pData) + *pData);

	} while (true);

	((PDWORD) pBuf)[0] = dwSize = *pData + 3 * sizeof(DWORD);
	((PDWORD) pBuf)[1] = pData[1];
	MoveMemory(((PDWORD) pBuf) + 5, pData + 2, *pData - 2 * sizeof(DWORD));*/
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CProcessFileDlg dialog


CProcessFileDlg::CProcessFileDlg(MSG_FILE_INFO& stData, const CString& strPath, CWnd* pParent /*=NULL*/)
	: CDialog(CProcessFileDlg::IDD, pParent), m_stData(stData), m_strFileName(strPath)
{
	//{{AFX_DATA_INIT(CProcessFileDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
}


void CProcessFileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CProcessFileDlg)
	DDX_Control(pDX, IDC_PROGRESS, m_wndProgress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CProcessFileDlg, CDialog)
	//{{AFX_MSG_MAP(CProcessFileDlg)
	ON_BN_CLICKED(IDC_FILE_PROCESSED, OnFileProcessed)
	ON_BN_CLICKED(IDC_INTERRUPT, OnInterrupt)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CProcessFileDlg message handlers

BOOL CProcessFileDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	if (m_stData.Open(m_strFileName, m_hWnd))
	{
		__int64 nFileSize64 = m_stData.m_64nSizeFile;
		if (nFileSize64 >= 0x100000000)
			nFileSize64 >>= 0x20;

		m_wndProgress.SetRange32(0, (DWORD) nFileSize64);
		SetTimer(1, 300, NULL);

	} else
		EndDialog(IDCANCEL);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CProcessFileDlg::OnFileProcessed() 
{
	EndDialog(IDOK);	
}

void CProcessFileDlg::OnInterrupt() 
{
	m_stData.m_bInterrupt = TRUE;	
}

void CProcessFileDlg::OnTimer(UINT nIDEvent) 
{
	__int64 nSizeParsed64 = m_stData.m_64nSizeFile - m_stData.m_64nSizeLeft;
	if (m_stData.m_64nSizeFile >= 0x100000000)
		nSizeParsed64 >>= 0x20;

	m_wndProgress.SetPos((DWORD) nSizeParsed64);
	
	CDialog::OnTimer(nIDEvent);
}
