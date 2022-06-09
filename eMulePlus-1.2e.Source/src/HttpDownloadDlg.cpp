/*
Module : HTTPDOWNLOADDLG.CPP
Purpose: Defines the implementation for an MFC dialog which performs HTTP downloads
         similiar to the Internet Explorer download dialog
Created: PJN / 14-11-1999
History: PJN / 25-01-2000 1. Fixed a problem where server authentication was not being detected correctly,
                          while proxy authentication was being handled.
                          2. Updated the way and periodicity certain UI controls are updated during the
                          HTTP download

Copyright (c) 1999 - 2000 by PJ Naughter.
All rights reserved.


*/

/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "resource.h"
#include "HttpDownloadDlg.h"
#include "emule.h"

///////////////////////////////// Defines /////////////////////////////////////
#define HAS_ZLIB
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

const UINT WM_HTTPDOWNLOAD_THREAD_FINISHED = WM_APP + 1;


////////////////////////////////////// gzip ///////////////////////////////////
//in the spirit of zlib, lets do something horrible with defines ;)
#ifdef HAS_ZLIB

#include "zlib/zlib.h"

static int const gz_magic[2] = {0x1f, 0x8b}; /* gzip magic header */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ascii text */
#define HEAD_CRC     0x02 /* bit 1 set: header CRC present */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define RESERVED     0xE0 /* bits 5..7: reserved */

static int get_byte(HINTERNET m_hHttpFile) {
	unsigned char c;
	DWORD dwBytesRead;
	BOOL b = ::InternetReadFile(m_hHttpFile, &c, 1, &dwBytesRead);
	if(!b)
		return EOF;
	else
		return c;
}

static int check_header(z_stream *stream, HINTERNET m_hHttpFile) {
	int method; /* method byte */
	int flags;  /* flags byte */
	uInt len;
	int c;

	/* Check the gzip magic header */
	for(len = 0; len < 2; len++) {
		c = get_byte(m_hHttpFile);
		if(c != gz_magic[len]) {
			if(len != 0) stream->avail_in++, stream->next_in--;
			if(c != EOF) {
				stream->avail_in++, stream->next_in--;
				//do not support transparent streams
				return stream->avail_in != 0 ? Z_DATA_ERROR : Z_STREAM_END;
			}
			return stream->avail_in != 0 ? Z_OK : Z_STREAM_END;
		}
	}
	method = get_byte(m_hHttpFile);
	flags = get_byte(m_hHttpFile);
	if(method != Z_DEFLATED || (flags & RESERVED) != 0)
		return Z_DATA_ERROR;

	/* Discard time, xflags and OS code: */
	for(len = 0; len < 6; len++) (void)get_byte(m_hHttpFile);

	if((flags & EXTRA_FIELD) != 0) { /* skip the extra field */
		len  =  (uInt)get_byte(m_hHttpFile);
		len += ((uInt)get_byte(m_hHttpFile))<<8;
		/* len is garbage if EOF but the loop below will quit anyway */
		while(len-- != 0 && get_byte(m_hHttpFile) != EOF) ;
	}
	if((flags & ORIG_NAME) != 0) { /* skip the original file name */
		while((c = get_byte(m_hHttpFile)) != 0 && c != EOF) ;
	}
	if((flags & COMMENT) != 0) {   /* skip the .gz file comment */
		while((c = get_byte(m_hHttpFile)) != 0 && c != EOF) ;
	}
	if((flags & HEAD_CRC) != 0) {  /* skip the header crc */
		for(len = 0; len < 2; len++) (void)get_byte(m_hHttpFile);
	}
	//return Z_DATA_ERROR if we hit EOF?
	return Z_OK;
}

#define ACCEPT_ENCODING_HEADER _T("Accept-Encoding: gzip, x-gzip, identity, *;q=0\r\n")
#define ENCODING_CLEAN_UP      if(bEncodedWithGZIP) inflateEnd(&zs)

#define ENCODING_INIT          BOOL bEncodedWithGZIP = FALSE;               \
                               z_stream zs;                                 \
                               unsigned char cBufferGZIP[1024 * 8]

#define PREPARE_DECODER                                                     \
  if(bEncodedWithGZIP) {                                                    \
    zs.next_out = cBufferGZIP;                                              \
    zs.zalloc = (alloc_func)0;                                              \
    zs.zfree = (free_func)0;                                                \
    zs.opaque = (voidpf)0;                                                  \
    zs.next_in = (unsigned char*)szReadBuf;                                 \
    zs.next_out = Z_NULL;                                                   \
    zs.avail_in = 0;                                                        \
	zs.avail_out = sizeof(szReadBuf);                                       \
                                                                            \
    VERIFY(inflateInit2(&zs, -MAX_WBITS) == Z_OK);                          \
    int result = check_header(&zs, m_hHttpFile);                            \
    if(result != Z_OK) {                                                    \
      TRACE(_T("An exception occured while decoding the download file\n")); \
      HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_ERROR_READFILE);      \
      inflateEnd(&zs);									                    \
    }                                                                       \
  }

#define DECODE_DATA(CFILE, DATA, LEN)                                       \
  if(bEncodedWithGZIP) {                                                    \
    zs.next_in = (unsigned char*)DATA;                                      \
    zs.avail_in = LEN;                                                      \
    int iResult;                                                            \
    do {                                                                    \
      zs.total_out = 0;                                                     \
      zs.next_out = cBufferGZIP;                                            \
      zs.avail_out = 1024;                                                  \
      iResult = inflate(&zs, Z_SYNC_FLUSH);                                 \
      CFILE.Write(cBufferGZIP, zs.total_out);                               \
      if(iResult == Z_STREAM_ERROR || iResult == Z_DATA_ERROR) {            \
        TRACE(_T("An exception occured while decoding the download file\n"));\
        HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_ERROR_READFILE);    \
        ENCODING_CLEAN_UP;                                                  \
        return;                                                             \
      }                                                                     \
    } while(iResult == Z_OK && zs.avail_out == 0);                          \
  } else                                                                    \
    CFILE.Write(DATA, LEN)

#else

#define ACCEPT_ENCODING_HEADER _T("Accept-Encoding: identity, *;q=0\r\n")

#define ENCODING_CLEAN_UP ((void)0)

#define ENCODING_INIT ((void)0)

#define PREPARE_DECODER ((void)0)

#define DECODE_DATA(CFILE, DATA, LEN) CFILE.Write(DATA, LEN)

#endif


///////////////////////////////// Implementation //////////////////////////////
IMPLEMENT_DYNAMIC(CHttpDownloadDlg, CDialog);



CHttpDownloadDlg::CHttpDownloadDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CHttpDownloadDlg::IDD, pParent)
{
	m_hInternetSession = NULL;
	m_hHttpConnection = NULL;
	m_hHttpFile = NULL;
	m_bAbort = FALSE;
	m_bSafeToClose = FALSE;
	m_pThread = NULL;
	m_strInitializingTitle = _T("");
	m_nIDPercentage = IDS_HTTPDOWNLOAD_OF;
}

void CHttpDownloadDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATUS, m_ctrlStatus);
	DDX_Control(pDX, IDC_TRANSFER_RATE, m_ctrlTransferRate);
	DDX_Control(pDX, IDC_TIMELEFT, m_ctrlTimeLeft);
	DDX_Control(pDX, IDC_PROGRESS1, m_ctrlProgress);
	DDX_Control(pDX, IDC_FILESTATUS, m_ctrlFileStatus);
	DDX_Control(pDX, IDC_ANIMATE1, m_ctrlAnimate);
}

BEGIN_MESSAGE_MAP(CHttpDownloadDlg, CDialog)
	ON_WM_DESTROY()
	ON_WM_CLOSE()
	ON_MESSAGE(WM_HTTPDOWNLOAD_THREAD_FINISHED, OnThreadFinished)
END_MESSAGE_MAP()


LRESULT CHttpDownloadDlg::OnThreadFinished(WPARAM wParam, LPARAM /*lParam*/)
{
	//It's now safe to close since the thread has signaled us
	m_bSafeToClose = TRUE;

	//Stop the animation
	m_ctrlAnimate.Stop();
	Sleep(1000);
	//If an error occured display the message box
	if (m_bAbort)
		EndDialog(IDCANCEL);
	else if (wParam)
	{
		//AfxMessageBox(m_sError);
		EndDialog(IDCANCEL);
	}
	else
		EndDialog(IDOK);

	return 0L;
}

BOOL CHttpDownloadDlg::OnInitDialog()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDCANCEL, IDS_CANCEL },
		{ IDC_STATIC_ESTTIME, IDS_DLCOL_REMAININGTIME },
		{ IDC_STATIC_TRATE, IDS_INFLST_USER_AVERAGEDOWNRATE }
	};
	CString	strTmp;

	for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl); ui++)
		SetDlgItemText(s_auResTbl[ui][0], GetResString(s_auResTbl[ui][1]));
	SetWindowText(m_strInitializingTitle);

	//Let the parent class do its thing
	CDialog::OnInitDialog();

	//Setup the animation control
	m_ctrlAnimate.Open(IDR_HTTPDOWNLOAD_ANIMATION);

	//Validate the URL
	ASSERT(m_sURLToDownload.GetLength()); //Did you forget to specify the file to download
	if (!AfxParseURL(m_sURLToDownload, m_dwServiceType, m_sServer, m_sObject, m_uPort))
	{
		//Try sticking "http://" before it
		m_sURLToDownload = _T("http://") + m_sURLToDownload;
		if (!AfxParseURL(m_sURLToDownload, m_dwServiceType, m_sServer, m_sObject, m_uPort))
		{
			TRACE(_T("Failed to parse the URL: %s\n"), m_sURLToDownload);
			EndDialog(IDCANCEL);
			return TRUE;
		}
	}

	//Check to see if the file we are downloading to exists and if
	//it does, then ask the user if they were it overwritten
	CFileStatus fs;
	ASSERT(m_sFileToDownloadInto.GetLength());
	if (CFile::GetStatus(m_sFileToDownloadInto, fs))
	{
		strTmp.Format(GetResString(IDS_HTTPDOWNLOAD_OK_TO_OVERWRITE), m_sFileToDownloadInto);
		if (AfxMessageBox(strTmp, MB_YESNO) != IDYES)
		{
			TRACE(_T("Failed to confirm file overwrite, download aborted\n"));
			EndDialog(IDCANCEL);
			return TRUE;
		}
	}

	//Try and open the file we will download into
	if (!m_FileToWrite.Open(m_sFileToDownloadInto, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyWrite))
	{
		TRACE(_T("Failed to open the file to download into, Error:%d\n"), GetLastError());
		strTmp.Format(GetResString(IDS_HTTPDOWNLOAD_FAIL_FILE_OPEN), ::GetLastError());
		AfxMessageBox(strTmp);
		EndDialog(IDCANCEL);
		return TRUE;
	}

	//Pull out just the filename component
	int nSlash = m_sObject.ReverseFind(_T('/'));
	if (nSlash == -1)
		nSlash = m_sObject.ReverseFind(_T('\\'));
	if (nSlash != -1 && m_sObject.GetLength() > 1)
		m_sFilename = m_sObject.Right(m_sObject.GetLength() - nSlash - 1);
	else
		m_sFilename = m_sObject;

	//Set the file status text
	strTmp.Format(GetResString(IDS_HTTPDOWNLOAD_OF), m_sFilename, m_sServer);
	m_ctrlFileStatus.SetWindowText(strTmp);

	//Spin off the background thread which will do the actual downloading
	m_pThread = AfxBeginThread(_DownloadThread, this, THREAD_PRIORITY_NORMAL+ g_App.m_pPrefs->GetMainProcessPriority(), 0, CREATE_SUSPENDED);
	if (m_pThread == NULL)
	{
		TRACE(_T("Failed to create download thread, dialog is aborting\n"));
		EndDialog(IDCANCEL);
		return TRUE;
	}
	m_pThread->m_bAutoDelete = FALSE;
	m_pThread->ResumeThread();

	return TRUE;
}

UINT AFX_CDECL CHttpDownloadDlg::_DownloadThread(LPVOID pParam)
{
	//Convert from the SDK world to the C++ world
	CHttpDownloadDlg* pDlg = (CHttpDownloadDlg*) pParam;
	ASSERT(pDlg);
	ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CHttpDownloadDlg)));

	g_App.m_pPrefs->InitThreadLocale();

	pDlg->DownloadThread();
	return 0;
}

void CHttpDownloadDlg::SetPercentage(int nPercentage)
{
	//Change the progress control
	m_ctrlProgress.SetPos(nPercentage);

	//Change the caption text
	CString sPercentage;
	sPercentage.Format(_T("%d"), nPercentage);
	CString	strCaption;
	strCaption.Format(GetResString(m_nIDPercentage), sPercentage, m_sFilename);
	SetWindowText(strCaption);
}

void CHttpDownloadDlg::SetProgressRange(DWORD dwFileSize)
{
	m_ctrlProgress.SetRange(0, (short)((dwFileSize+512)/1024));
}

void CHttpDownloadDlg::SetProgress(DWORD dwBytesRead)
{
	m_ctrlProgress.SetPos(dwBytesRead/1024);
}

void CHttpDownloadDlg::SetTimeLeft(DWORD dwSecondsLeft, DWORD dwBytesRead, DWORD dwFileSize)
{
	CString	strTimeLeft;

	strTimeLeft.Format( GetResString(IDS_HTTPDOWNLOAD_TIMELEFT),
		CastSecondsToHM(dwSecondsLeft), CastItoXBytes(dwBytesRead), CastItoXBytes(dwFileSize) );
	m_ctrlTimeLeft.SetWindowText(strTimeLeft);
}

void CHttpDownloadDlg::SetStatus(const CString& sCaption)
{
	m_ctrlStatus.SetWindowText(sCaption);
}

void CHttpDownloadDlg::SetStatus(UINT nID)
{
	SetStatus(GetResString(nID));
}

void CHttpDownloadDlg::SetStatus(UINT nID, LPCTSTR lpsz1)
{
	CString strStatus;
	
	strStatus.Format(GetResString(nID), lpsz1);
	SetStatus(strStatus);
}

void CHttpDownloadDlg::SetTransferRate(double KbPerSecond)
{
	CString sRate;

	sRate.Format(_T("%.2f %s"), KbPerSecond, GetResString(IDS_KBYTESEC));
	m_ctrlTransferRate.SetWindowText(sRate);
}

void CHttpDownloadDlg::PlayAnimation()
{
	m_ctrlAnimate.Play(0, (UINT)-1, (UINT)-1);
}

void CHttpDownloadDlg::HandleThreadErrorWithLastError(UINT nIDError, DWORD dwLastError)
{
	//Form the error string to report
	m_sError.Format(GetResString(nIDError), (dwLastError != 0) ? dwLastError : ::GetLastError());

	//Delete the file being downloaded to if it is present
	m_FileToWrite.Close();
	::DeleteFile(m_sFileToDownloadInto);

	PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED, 1);
}

void CHttpDownloadDlg::HandleThreadError(UINT nIDError)
{
	GetResString(&m_sError, nIDError);
	PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED, 1);
}

void CHttpDownloadDlg::DownloadThread()
{
	ENCODING_INIT;
	//Create the Internet session handle
	ASSERT(m_hInternetSession == NULL);
	m_hInternetSession = ::InternetOpen(HTTP_USERAGENT, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (m_hInternetSession == NULL)
	{
		TRACE(_T("Failed in call to InternetOpen, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_GENERIC_ERROR);
		return;
	}

	//Should we exit the thread
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}

	//Setup the status callback function
	if (::InternetSetStatusCallback(m_hInternetSession, _OnStatusCallBack) == INTERNET_INVALID_STATUS_CALLBACK)
	{
		TRACE(_T("Failed in call to InternetSetStatusCallback, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_GENERIC_ERROR);
		return;
	}

	//Should we exit the thread
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}

	//Make the connection to the HTTP server
	ASSERT(m_hHttpConnection == NULL);
	if (m_sUserName.GetLength())
		m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_sServer, m_uPort, m_sUserName,
                                          m_sPassword, m_dwServiceType, 0, (DWORD) this);
	else
		m_hHttpConnection = ::InternetConnect(m_hInternetSession, m_sServer, m_uPort, NULL,
                                          NULL, m_dwServiceType, 0, (DWORD) this);
	if (m_hHttpConnection == NULL)
	{
		TRACE(_T("Failed in call to InternetConnect, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER);
		return;
	}

	//Should we exit the thread
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}

	//Start the animation to signify that the download is taking place
	PlayAnimation();

	//Issue the request to read the file
	LPCTSTR ppszAcceptTypes[2];
	ppszAcceptTypes[0] = _T("*/*");  //We support accepting any mime file type since this is a simple download of a file
	ppszAcceptTypes[1] = NULL;
	ASSERT(m_hHttpFile == NULL);
	m_hHttpFile = HttpOpenRequest(m_hHttpConnection, NULL, m_sObject, NULL, NULL, ppszAcceptTypes, INTERNET_FLAG_RELOAD |
								  INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_KEEP_CONNECTION, (DWORD)this);
	if (m_hHttpFile == NULL)
	{
		TRACE(_T("Failed in call to HttpOpenRequest, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER);
		return;
	}

	//Should we exit the thread
	if (m_bAbort)
	{
		PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
		return;
	}

	//fill in what encoding we support
	HttpAddRequestHeaders(m_hHttpFile, ACCEPT_ENCODING_HEADER, CSTRLEN(ACCEPT_ENCODING_HEADER), HTTP_ADDREQ_FLAG_ADD);

//label used to jump to if we need to resend the request
resend:

	//Issue the request
	BOOL bSend = ::HttpSendRequest(m_hHttpFile, NULL, 0, NULL, 0);
	if (!bSend)
	{
		TRACE(_T("Failed in call to HttpSendRequest, Error:%d\n"), ::GetLastError());
		HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_FAIL_CONNECT_SERVER);
		return;
	}

	//	Check the HTTP status code
	DWORD	dwStatusCode = 0, dwSz = sizeof(dwStatusCode);

	if (!HttpQueryInfo(m_hHttpFile, HTTP_QUERY_STATUS_CODE | HTTP_QUERY_FLAG_NUMBER, &dwStatusCode, &dwSz, NULL))
	{
		TRACE(_T("Failed in call to HttpQueryInfo for HTTP query status code, Error:%d\n"), ::GetLastError());
		HandleThreadError(IDS_HTTPDOWNLOAD_INVALID_SERVER_RESPONSE);
		return;
	}
	else
	{
		//Handle any authentication errors
		if (dwStatusCode == HTTP_STATUS_PROXY_AUTH_REQ || dwStatusCode == HTTP_STATUS_DENIED)
		{
			// We have to read all outstanding data on the Internet handle
			// before we can resubmit request. Just discard the data.
			char szData[51];
			DWORD dwSize;
			do
			{
				::InternetReadFile(m_hHttpFile, (LPVOID)szData, 50, &dwSize);
			}
			while (dwSize != 0);

			//Bring up the standard authentication dialog
			if (::InternetErrorDlg(GetSafeHwnd(), m_hHttpFile, ERROR_INTERNET_INCORRECT_PASSWORD, FLAGS_ERROR_UI_FILTER_FOR_ERRORS |
                             FLAGS_ERROR_UI_FLAGS_GENERATE_DATA | FLAGS_ERROR_UI_FLAGS_CHANGE_OPTIONS, NULL) == ERROR_INTERNET_FORCE_RETRY)
				goto resend;
		}
		else if (dwStatusCode != HTTP_STATUS_OK)
		{
			TRACE(_T("Failed to retrieve a HTTP 200 status, Status Code:%d\n"), dwStatusCode);
			HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_INVALID_HTTP_RESPONSE, dwStatusCode);
			return;
		}
	}

	//Check to see if any encodings are supported
	TCHAR szContentEncoding[32];
	DWORD dwEncodeStringSize = sizeof(szContentEncoding);
	if(::HttpQueryInfo(m_hHttpFile, HTTP_QUERY_CONTENT_ENCODING, szContentEncoding, &dwEncodeStringSize, NULL))
	{
		if(!_tcsicmp(szContentEncoding, _T("gzip")) || !_tcsicmp(szContentEncoding, _T("x-gzip")))
			bEncodedWithGZIP = TRUE;
	}

	//Update the status control to reflect that we are getting the file information
	SetStatus(IDS_HTTPDOWNLOAD_GETTING_FILE_INFORMATION);

	// Get the length of the file
	DWORD dwFileSize = 0;
	BOOL bGotFileSize = FALSE;

	dwSz = sizeof(dwFileSize);
	if ( ::HttpQueryInfo( m_hHttpFile, HTTP_QUERY_CONTENT_LENGTH | HTTP_QUERY_FLAG_NUMBER,
		&dwFileSize, &dwSz, NULL ) && (dwFileSize != 0) )
	{
		//	Set the progress control range
		bGotFileSize = TRUE;
		SetProgressRange(dwFileSize);
	}

	//Update the status to say that we are now downloading the file
	SetStatus(IDS_HTTPDOWNLOAD_RETREIVEING_FILE);

	//Now do the actual read of the file
	DWORD dwStartTicks = ::GetTickCount();
	DWORD dwCurrentTicks = dwStartTicks;
	DWORD dwBytesRead = 0;
	char szReadBuf[1024];
	DWORD dwBytesToRead = 1024;
	DWORD dwTotalBytesRead = 0;
	DWORD dwLastTotalBytes = 0;
	DWORD dwLastPercentage = 0;

	PREPARE_DECODER;
	do
	{
		if (!::InternetReadFile(m_hHttpFile, szReadBuf, dwBytesToRead, &dwBytesRead))
		{
			TRACE(_T("Failed in call to InternetReadFile, Error:%d\n"), ::GetLastError());
			HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_ERROR_READFILE);
			ENCODING_CLEAN_UP;
			return;
		}
		else if (dwBytesRead && !m_bAbort)
		{
			//Write the data to file
			try
			{
				DECODE_DATA(m_FileToWrite, szReadBuf, dwBytesRead);
			}
			catch(CFileException *error)
			{
				TRACE(_T("An exception occured while writing to the download file\n"));
				HandleThreadErrorWithLastError(IDS_HTTPDOWNLOAD_ERROR_READFILE, error->m_lOsError);
				error->Delete();
				//clean up any encoding data before we return
				ENCODING_CLEAN_UP;
				return;
			}

			//Increment the total number of bytes read
			dwTotalBytesRead += dwBytesRead;

			UpdateControlsDuringTransfer(dwStartTicks, dwCurrentTicks, dwTotalBytesRead, dwLastTotalBytes,
                                     dwLastPercentage, bGotFileSize, dwFileSize);
		}
	}
	while (dwBytesRead && !m_bAbort);

	//clean up any encoding data before we return
	ENCODING_CLEAN_UP;

	//Delete the file being downloaded to if it is present and the download was aborted
	m_FileToWrite.Close();
	if (m_bAbort)
		::DeleteFile(m_sFileToDownloadInto);

	//We're finished
	PostMessage(WM_HTTPDOWNLOAD_THREAD_FINISHED);
}

void CHttpDownloadDlg::UpdateControlsDuringTransfer(DWORD dwStartTicks, DWORD& dwCurrentTicks, DWORD dwTotalBytesRead, DWORD& dwLastTotalBytes,
                                                    DWORD& dwLastPercentage, BOOL bGotFileSize, DWORD dwFileSize)
{
	if (bGotFileSize)
	{
		//Update the percentage downloaded in the caption
		DWORD dwPercentage = (DWORD) (dwTotalBytesRead * 100.0 / dwFileSize);
		if (dwPercentage != dwLastPercentage)
		{
			SetPercentage(dwPercentage);
			dwLastPercentage = dwPercentage;

			//Update the progress control bar
			SetProgress(dwTotalBytesRead);
		}
	}

	//Update the transfer rate amd estimated time left every second
	DWORD dwNowTicks = GetTickCount();
	DWORD dwTimeTaken = dwNowTicks - dwCurrentTicks;
	if (dwTimeTaken > 1000)
	{
		double KbPerSecond = ((double)(dwTotalBytesRead) - (double)(dwLastTotalBytes)) / ((double)(dwTimeTaken));
		SetTransferRate(KbPerSecond);

		//Setup for the next time around the loop
		dwCurrentTicks = dwNowTicks;
		dwLastTotalBytes = dwTotalBytesRead;

		if (bGotFileSize)
		{
			//Update the estimated time left
			if (dwTotalBytesRead)
			{
				DWORD dwSecondsLeft = (DWORD) (((double)dwNowTicks - dwStartTicks) / dwTotalBytesRead *
					(dwFileSize - dwTotalBytesRead) / 1000);
				SetTimeLeft(dwSecondsLeft, dwTotalBytesRead, dwFileSize);
			}
		}
	}
}

void CALLBACK CHttpDownloadDlg::_OnStatusCallBack(HINTERNET hInternet, DWORD dwContext, DWORD dwInternetStatus,
                                                  LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	CHttpDownloadDlg* pDlg = (CHttpDownloadDlg*) dwContext;
	ASSERT(pDlg);
	ASSERT(pDlg->IsKindOf(RUNTIME_CLASS(CHttpDownloadDlg)));
	pDlg->OnStatusCallBack(hInternet, dwInternetStatus, lpvStatusInformation, dwStatusInformationLength);
}

void CHttpDownloadDlg::OnStatusCallBack(HINTERNET /*hInternet*/, DWORD dwInternetStatus,
                                         LPVOID lpvStatusInformation, DWORD dwStatusInformationLength)
{
	UINT	dwResStrId = 0;

	switch (dwInternetStatus)
	{
		case INTERNET_STATUS_RESOLVING_NAME:
			dwResStrId = IDS_HTTPDOWNLOAD_RESOLVING_NAME;
			break;
		case INTERNET_STATUS_NAME_RESOLVED:
			dwResStrId = IDS_HTTPDOWNLOAD_RESOLVED_NAME;
			break;
		case INTERNET_STATUS_CONNECTING_TO_SERVER:
			dwResStrId = IDS_HTTPDOWNLOAD_CONNECTING;
			break;
		case INTERNET_STATUS_CONNECTED_TO_SERVER:
			dwResStrId = IDS_HTTPDOWNLOAD_CONNECTED;
			break;
		case INTERNET_STATUS_REDIRECT:
			dwResStrId = IDS_HTTPDOWNLOAD_REDIRECTING;
			break;
	}
	if (dwResStrId != 0)
	{
		const TCHAR	*pcStr = reinterpret_cast<LPCTSTR>(lpvStatusInformation);	// IE6
		INT			iFlags = IS_TEXT_UNICODE_UNICODE_MASK;
		CString		strBuf;

	// Try to figure out if it is ANSI or Unicode. IE is playing a strange game with that data...
	// In some cases the strings are even encoded as Unicode *with* a trailing NUL-byte, which
	// means that the nr. of bytes in the Unicode string is odd! Thus, the Windows API function
	// 'IsTextUnicode' must not be invoked with 'IS_TEXT_UNICODE_ODD_LENGTH', otherwise it will
	// again give false results.
	// For IE7+:
	// INTERNET_STATUS_RESOLVING_NAME		Unicode: server name
	// INTERNET_STATUS_NAME_RESOLVED		ANSI: IP address
	// INTERNET_STATUS_CONNECTING_TO_SERVER	ANSI: IP address
	// INTERNET_STATUS_CONNECTED_TO_SERVER	ANSI: IP address
		if (IsTextUnicode(lpvStatusInformation, dwStatusInformationLength, &iFlags) == 0)
		{
			strBuf = reinterpret_cast<LPCSTR>(lpvStatusInformation);
			pcStr = strBuf.GetString();
		}
		SetStatus(dwResStrId, pcStr);
	}
}

void CHttpDownloadDlg::OnDestroy()
{
	//Wait for the worker thread to exit
	if (m_pThread)
	{
		WaitForSingleObject(m_pThread->m_hThread, INFINITE);
		delete m_pThread;
		m_pThread = NULL;
	}

	//Free up the internet handles we may be using
	if (m_hHttpFile)
	{
		::InternetCloseHandle(m_hHttpFile);
		m_hHttpFile = NULL;
	}
	if (m_hHttpConnection)
	{
		::InternetCloseHandle(m_hHttpConnection);
		m_hHttpConnection = NULL;
	}
	if (m_hInternetSession)
	{
		::InternetCloseHandle(m_hInternetSession);
		m_hInternetSession = NULL;
	}

	//Let the parent class do its thing
	CDialog::OnDestroy();
}

void CHttpDownloadDlg::OnCancel()
{
	// Asynchronously free up the internet handles we may be using.
	// Otherwise we may get some kind of deadlock situation, because 'InternetConnect'
	// may not return for a very long time...
	if (m_hHttpFile)
	{
		::InternetCloseHandle(m_hHttpFile);
		m_hHttpFile = NULL;
	}
	if (m_hHttpConnection)
	{
		::InternetCloseHandle(m_hHttpConnection);
		m_hHttpConnection = NULL;
	}
	if (m_hInternetSession)
	{
		::InternetCloseHandle(m_hInternetSession);
		m_hInternetSession = NULL;
	}

	//Just set the abort flag to TRUE and
	//disable the cancel button
	m_bAbort = TRUE;
	GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
	SetStatus(IDS_HTTPDOWNLOAD_ABORTING_TRANSFER);
}

void CHttpDownloadDlg::OnClose()
{
	if (m_bSafeToClose)
		CDialog::OnClose();
	else
	{
		//Just set the abort flag to TRUE and
		//disable the cancel button
		m_bAbort = TRUE;
		GetDlgItem(IDCANCEL)->EnableWindow(FALSE);
		SetStatus(IDS_HTTPDOWNLOAD_ABORTING_TRANSFER);
	}
}