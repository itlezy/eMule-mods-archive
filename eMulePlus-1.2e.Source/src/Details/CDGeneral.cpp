//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h"
#include "..\resource.h"
#include "CDGeneral.h"
#include "..\updownclient.h"
#include "..\ServerList.h"
#include "..\server.h"
#include "..\emule.h"
#include "..\IP2Country.h"

IMPLEMENT_DYNCREATE(CCDGeneral, CPropertyPage)

CCDGeneral::CCDGeneral() : CPropertyPage(CCDGeneral::IDD)
{
	m_hClientIcon = NULL;
	m_eCurrClientSoft = SO_LAST;
}

CCDGeneral::~CCDGeneral()
{
	if(m_hClientIcon != NULL)
		::DestroyIcon(m_hClientIcon);
}

void CCDGeneral::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_USERNAME_VAL, m_ctrlUserName);
	DDX_Control(pDX, IDC_SERVERNAME_VAL, m_ctrlServerName);
	DDX_Control(pDX, IDC_CLIENTSOFTWARE_VAL, m_ctrlClientSoftware);
	DDX_Control(pDX, IDC_CLIENTICON, m_ctrlClientIcon);
}


BEGIN_MESSAGE_MAP(CCDGeneral, CPropertyPage)
END_MESSAGE_MAP()

BOOL CCDGeneral::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	Localize();
	Update();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCDGeneral::Update()
{
	static const uint16 s_auClientIconResID[] =
	{
		IDI_COMPROT32,			//SO_PLUS
		IDI_COMPROT32,			//SO_EMULE
		IDI_AMULE32,			//SO_AMULE
		IDI_EDONKEYHYBRID32,	//SO_EDONKEYHYBRID
		IDI_NORMAL32,			//SO_EDONKEY
		IDI_MLDONKEY32,			//SO_MLDONKEY
		IDI_SECUREHASH32,		//SO_OLDEMULE + non-SUI
		IDI_SHAREAZA32,			//SO_SHAREAZA
		IDI_XMULE32,			//SO_XMULE
		IDI_LPHANT32,			//SO_LPHANT
		IDI_UNKNOWN32			//SO_UNKNOWN
	};

	EMULE_TRY

	if ((m_pClient == NULL) || !::IsWindow(GetSafeHwnd()))
		return;

//	Reload icon only when client version is changed (e.g. Unknown->another)
	if (m_eCurrClientSoft != m_pClient->GetClientSoft())
	{
		m_eCurrClientSoft = m_pClient->GetClientSoft();
		if(m_hClientIcon != NULL)
			::DestroyIcon(m_hClientIcon);

		HINSTANCE	hInst = AfxGetInstanceHandle();
		uint32		dwIdx = m_eCurrClientSoft;

		if ((dwIdx == SO_EMULE) || (dwIdx == SO_PLUS))
		{
			if (m_pClient->m_pCredits->GetCurrentIdentState(m_pClient->GetIP()) != IS_IDENTIFIED)
				dwIdx = SO_OLDEMULE;
		}
		else if (dwIdx > SO_UNKNOWN)
			dwIdx = SO_UNKNOWN;
		m_hClientIcon = reinterpret_cast<HICON>(::LoadImage(hInst, MAKEINTRESOURCE(s_auClientIconResID[dwIdx]), IMAGE_ICON, 0, 0, 0));
		m_ctrlClientIcon.SetIcon(m_hClientIcon);
	}

	CString	strBuffer;
	UINT	dwResStrId;

	if (!m_pClient->IsUserNameEmpty())
		m_ctrlUserName.SetWindowText(m_pClient->GetUserName());
	else
		m_ctrlUserName.SetWindowText(_T("?"));
	
	if (m_pClient->HasValidHash())
	{
		TCHAR acHashStr[MAX_HASHSTR_SIZE];

		SetDlgItemText(IDC_USERHASH_VAL, md4str(m_pClient->GetUserHash(), acHashStr));
	}
	else
		SetDlgItemText(IDC_USERHASH_VAL, _T("?"));

	m_ctrlClientSoftware.SetWindowText(m_pClient->GetFullSoftVersionString());

	if (m_pClient->SupportsCryptLayer())
	{
		if ( g_App.m_pPrefs->IsClientCryptLayerSupported() &&
			(m_pClient->RequestsCryptLayer() || g_App.m_pPrefs->IsClientCryptLayerRequested()) &&
			(m_pClient->IsObfuscatedConnectionEstablished() || !((m_pClient->m_pRequestSocket != NULL) && m_pClient->m_pRequestSocket->IsConnected())) )
		{
			dwResStrId = IDS_ENABLED;
		}
		else
			dwResStrId = IDS_SUPPORTED;
	}
	else
		dwResStrId = IDS_IDENTNOSUPPORT;
	::GetResString(&strBuffer, dwResStrId);
	SetDlgItemText(IDC_OBFUSCATION_VAL, strBuffer);

	SetDlgItemText(IDC_BANNED_VAL, m_pClient->GetBanString());

	strBuffer.Format(_T("%u (%s)"), m_pClient->GetUserIDHybrid(),
						::GetResString((m_pClient->HasLowID()) ? IDS_PRIOLOW : IDS_PRIOHIGH));
	SetDlgItemText(IDC_ID_VAL, strBuffer);

	strBuffer.Format(_T("%s:%u"), m_pClient->GetFullIP(), m_pClient->GetUserPort());
	if (g_App.m_pIP2Country->IsIP2Country())
		strBuffer.AppendFormat(_T(" (%s)"), m_pClient->GetCountryName());
	SetDlgItemText(IDC_IPADDRESS_VAL, strBuffer);
	
	if (m_pClient->GetServerIP())
	{
		strBuffer.Format(_T("%s:%u"), ipstr(m_pClient->GetServerIP()), m_pClient->GetServerPort());

		CServer	*pServer = g_App.m_pServerList->GetServerByIP(m_pClient->GetServerIP());

		if (pServer != NULL)
		{
			if (g_App.m_pIP2Country->IsIP2Country())
				strBuffer.AppendFormat(_T(" (%s)"), pServer->GetCountryName());
			m_ctrlServerName.SetWindowText(pServer->GetListName());
		}
		else
			m_ctrlServerName.SetWindowText(_T("?"));
		SetDlgItemText(IDC_SERVERIP_VAL, strBuffer);
	}
	else
	{
		SetDlgItemText(IDC_SERVERIP_VAL, _T("?"));
		m_ctrlServerName.SetWindowText(_T("?"));
	}

	if (m_pClient->Credits())
	{
		dwResStrId = IDS_IDENTNOSUPPORT;
		if (g_App.m_pClientCreditList->CryptoAvailable())
		{
			switch (m_pClient->Credits()->GetCurrentIdentState(m_pClient->GetIP()))
			{
				case IS_IDFAILED:
				case IS_IDNEEDED:
				case IS_IDBADGUY:
					dwResStrId = IDS_IDENTFAILED;
					break;
				case IS_IDENTIFIED:
					dwResStrId = IDS_IDENTOK;
				case IS_NOTAVAILABLE:
					break;
			}
		}
		::GetResString(&strBuffer, dwResStrId);
		SetDlgItemText(IDC_IDENTIFICATION_VAL, strBuffer);
	}
	else
		SetDlgItemText(IDC_IDENTIFICATION_VAL, _T("?"));

	EMULE_CATCH
}

void CCDGeneral::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_USERHASH_LBL, IDS_CD_UHASH },
		{ IDC_IDENTIFICATION_LBL, IDS_CD_IDENT },
		{ IDC_CLIENTSOFTWARE_LBL, IDS_CD_CSOFT },
		{ IDC_CDG_IPADDRESS_LBL, IDS_CD_UIP },
		{ IDC_SERVERNAME_LBL, IDS_CD_SNAME },
	};
	static const uint16 s_auResTbl2[][2] =
	{
		{ IDC_ID_LBL, IDS_ID },
		{ IDC_SERVERIP_LBL, IDS_SERVERIP },
		{ IDC_BANNED_LBL, IDS_BANNED },
		{ IDC_OBFUSCATION_LBL, IDS_OBFUSCATION }
	};

	if (GetSafeHwnd())
	{
		CString strBuffer;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strBuffer);
		}
		for (uint32 i = 0; i < ARRSIZE(s_auResTbl2); i++)
		{
			GetResString(&strBuffer, static_cast<UINT>(s_auResTbl2[i][1]));
			strBuffer += _T(":");
			SetDlgItemText(s_auResTbl2[i][0], strBuffer);
		}
	}
}
