//this file is part of eMule
//Copyright (C)2002-2006 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "emule.h"
#include "emuleDlg.h"
#include "otherfunctions.h"
#include "Preferences.h"
#include <atlsmtpconnection.h>

#if (defined(_UNICODE) && defined(_MSC_FULL_VER) && (_MSC_FULL_VER == 13103077))
#pragma message("Warning: compiled application might not run on Win2K or WinXP SP1 (at least VS 2003 SP1 is required)")
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////////////////
// CSMTPConnectionEx

#define ATLSMTP_CHALLENGE	"334"
#define ATLSMTP_AUTH_SUCC	"235"

class CSMTPConnectionEx : public CSMTPConnection
{
public:
	BOOL AuthLogin(LPCTSTR pcUserName, LPCTSTR pcPassword)
	{
		if (!Connected())
			return FALSE;

		char	pcBuf[ATLSMTP_MAX_LINE_LENGTH];
		int		iLen1;

		if ( !AtlSmtpSendAndCheck( reinterpret_cast<HANDLE>(m_hSocket), "AUTH LOGIN\r\n", sizeof("AUTH LOGIN\r\n") - 1,
			pcBuf, &iLen1, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_CHALLENGE, &m_Overlapped ) )
		{
			return FALSE;
		}

		int	iLen2 = ATLSMTP_MAX_LINE_LENGTH;

		if (!Base64Decode(pcBuf + 4, iLen1 - 4, reinterpret_cast<BYTE *>(pcBuf), &iLen2) || memcmp(pcBuf, "Username:", iLen2) != 0)
			return FALSE;

		CT2CAEX<128>	pcUserNameA(pcUserName);

		iLen2 = strlen(pcUserNameA);
		iLen1 = ATLSMTP_MAX_LINE_LENGTH - 2;

		if (!Base64Encode(reinterpret_cast<const BYTE *>(pcUserNameA.m_psz), iLen2, pcBuf, &iLen1))
			return FALSE;

		pcBuf[iLen1++] = '\r'; pcBuf[iLen1++] = '\n';

		if ( !AtlSmtpSendAndCheck( reinterpret_cast<HANDLE>(m_hSocket), pcBuf, iLen1,
			pcBuf, &iLen1, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_CHALLENGE, &m_Overlapped) )
		{
			return FALSE;
		}

		iLen2 = ATLSMTP_MAX_LINE_LENGTH;

		if (!Base64Decode(pcBuf + 4, iLen1 - 4, reinterpret_cast<BYTE *>(pcBuf), &iLen2) || memcmp(pcBuf, "Password:", iLen2) != 0)
			return FALSE;

		CT2CAEX<128>	pcPasswordA(pcPassword);

		iLen2 = strlen(pcPasswordA);
		iLen1 = ATLSMTP_MAX_LINE_LENGTH - 2;

		if (!Base64Encode(reinterpret_cast<const BYTE *>(pcPasswordA.m_psz), iLen2, pcBuf, &iLen1))
			return FALSE;

		pcBuf[iLen1++] = '\r'; pcBuf[iLen1++] = '\n';

		if ( !AtlSmtpSendAndCheck( reinterpret_cast<HANDLE>(m_hSocket), pcBuf, iLen1,
			pcBuf, &iLen1, ATLSMTP_MAX_LINE_LENGTH, ATLSMTP_AUTH_SUCC, &m_Overlapped) )
		{
			return FALSE;
		}

		return TRUE;
	}
};

///////////////////////////////////////////////////////////////////////////////
// CNotifierMailThread

class CNotifierMailThread : public CWinThread
{
	DECLARE_DYNCREATE(CNotifierMailThread)

protected:
	CNotifierMailThread();           // protected constructor used by dynamic creation
	virtual ~CNotifierMailThread();
	static CCriticalSection m_csOLE;

public:
	CString	m_strBody;

	virtual	BOOL InitInstance();
};

CCriticalSection CNotifierMailThread::m_csOLE;

IMPLEMENT_DYNCREATE(CNotifierMailThread, CWinThread)

CNotifierMailThread::CNotifierMailThread()
{
}

CNotifierMailThread::~CNotifierMailThread()
{
}

BOOL CNotifierMailThread::InitInstance()
{
	if (g_App.m_pMDlg != NULL && g_App.m_pMDlg->IsRunning())
	{
		m_csOLE.Lock();
		CoInitialize(NULL);

		EMULE_TRY

		g_App.m_pPrefs->InitThreadLocale();

		UINT	uiCodePage = CP_UTF8;

		CSMTPConnectionEx	smtp;

		if (smtp.Connect(g_App.m_pPrefs->GetSMTPServer()))
		{
			if (g_App.m_pMDlg != NULL && g_App.m_pMDlg->IsRunning())
			{
				if ( !g_App.m_pPrefs->IsSMTPAuthenticated()
					|| smtp.AuthLogin(g_App.m_pPrefs->GetSMTPUserName(), g_App.m_pPrefs->GetSMTPPassword()) )
				{
					CMimeMessage	msg;

					msg.SetSenderName(g_App.m_pPrefs->GetSMTPName(), uiCodePage);
					msg.SetSender(g_App.m_pPrefs->GetSMTPFrom());
					msg.AddRecipient(g_App.m_pPrefs->GetSMTPTo());
					if (g_App.m_pPrefs->IsSMTPMsgInSubjEnabled())
						msg.SetSubject(m_strBody, uiCodePage);
					else
					{
						msg.SetSubject(GetResString(IDS_SMTPMSG_SUBJECT), uiCodePage);
						msg.AddText(m_strBody, -1, 1, uiCodePage);
					}

					if (smtp.SendMessage(msg))
						g_App.m_pMDlg->AddLogLine(0, IDS_SMTPMSG_SENT, g_App.m_pPrefs->GetSMTPTo());
					else
						g_App.m_pMDlg->AddLogLine(LOG_RGB_ERROR, IDS_SMTPMSG_ERROR, _T("SendMessage"));
				}
				else
					g_App.m_pMDlg->AddLogLine(LOG_RGB_ERROR, IDS_SMTPMSG_ERROR, _T("Authorization"));
			}
		}
		else
			g_App.m_pMDlg->AddLogLine(LOG_RGB_ERROR, IDS_SMTPMSG_ERROR, _T("Connect"));

		EMULE_CATCH

		CoUninitialize();
		m_csOLE.Unlock();
	}
	return FALSE;
}

void CEmuleDlg::SendMail(LPCTSTR pcText, bool bMsgEnabled, bool bSendEnabled)
{
	if (bMsgEnabled && bSendEnabled)
	{
		CNotifierMailThread	*pThread = reinterpret_cast<CNotifierMailThread *>(
			AfxBeginThread(RUNTIME_CLASS(CNotifierMailThread), THREAD_PRIORITY_LOWEST, 0, CREATE_SUSPENDED) );

		if (pThread != NULL)
		{
			pThread->m_strBody = pcText;
			pThread->ResumeThread();
		}
	}
}
