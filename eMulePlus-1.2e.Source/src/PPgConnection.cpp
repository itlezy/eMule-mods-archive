//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "PPgConnection.h"
#include "FirewallOpener.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgConnection, CPropertyPage)
CPPgConnection::CPPgConnection()
	: CPropertyPage(CPPgConnection::IDD)
	, m_bShowOverhead(FALSE)
	, m_bLancastEnable(FALSE)
	, m_bOpenPorts(TRUE)
	, m_bLimitlessDownload(FALSE)
	, m_bUdpDisable(FALSE)
{
}

CPPgConnection::~CPPgConnection()
{
}

void CPPgConnection::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CONTYPE, m_conprof);
	DDX_Check(pDX, IDC_SHOWOVERHEAD, m_bShowOverhead);
	DDX_Check(pDX, IDC_LANCASTENABLE, m_bLancastEnable);
	DDX_Check(pDX, IDC_LIMITLESS, m_bLimitlessDownload);
	DDX_Check(pDX, IDC_OPENPORTS, m_bOpenPorts);
	DDX_Text(pDX, IDC_QUEUESIZE, m_strQueueSize);
	DDX_Text(pDX, IDC_MAXCON, m_strMaxConn);
	DDX_Text(pDX, IDC_MAXCON5, m_strMaxCon5);
	DDX_Text(pDX, IDC_UPLOAD_CAP, m_strUploadCap);
	DDX_Text(pDX, IDC_MAXUP, m_strUploadMax);
	DDX_Text(pDX, IDC_DOWNLOAD_CAP, m_strDownloadCap);
	DDX_Text(pDX, IDC_MAXDOWN, m_strDownloadMax);
	DDX_Text(pDX, IDC_PORT, m_strPort);
	DDX_Text(pDX, IDC_UDPPORT, m_strUdpPort);
	DDX_Check(pDX, IDC_UDPDISABLE, m_bUdpDisable);
}

BEGIN_MESSAGE_MAP(CPPgConnection, CPropertyPage)
	ON_CBN_SELCHANGE(IDC_CONTYPE, OnCbnSelchangeConType)
	ON_BN_CLICKED(IDC_UDPDISABLE, OnEnChangeUDPDisable)
	ON_EN_CHANGE(IDC_UDPPORT, OnSettingsChange)
	ON_EN_KILLFOCUS(IDC_DOWNLOAD_CAP, OnSpeedChange)
	ON_EN_KILLFOCUS(IDC_UPLOAD_CAP, OnSpeedChange)
	ON_EN_KILLFOCUS(IDC_MAXDOWN, OnSpeedChange)
	ON_EN_KILLFOCUS(IDC_MAXUP, OnSpeedChange)
	ON_EN_CHANGE(IDC_DOWNLOAD_CAP, OnSettingsChange)
	ON_EN_CHANGE(IDC_UPLOAD_CAP, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXDOWN, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXUP, OnSettingsChange)
	ON_EN_CHANGE(IDC_PORT, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXCON, OnSettingsChange)
	ON_BN_CLICKED(IDC_LANCASTENABLE, OnSettingsChange)
	ON_BN_CLICKED(IDC_OPENPORTS, OnSettingsChange)
	ON_EN_CHANGE(IDC_MAXCON5, OnSettingsChange)
	ON_BN_CLICKED(IDC_LIMITLESS, OnBnClickedLimitless)
	ON_EN_CHANGE(IDC_QUEUESIZE, OnSettingsChange)
	ON_EN_KILLFOCUS(IDC_QUEUESIZE, OnQueueChange)
	ON_BN_CLICKED(IDC_SHOWOVERHEAD, OnSettingsChange)
END_MESSAGE_MAP()

BOOL CPPgConnection::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	m_conprof.AddString(_T(""));
	m_conprof.AddString(_T("ADSL 256/128"));
	m_conprof.AddString(_T("ADSL 512/128"));
	m_conprof.AddString(_T("ADSL 512/256"));
	m_conprof.AddString(_T("ADSL 768/128"));
	m_conprof.AddString(_T("ADSL 1024/256"));
	m_conprof.AddString(_T("ADSL 1024/512"));
	m_conprof.AddString(_T("Cable"));
	m_conprof.AddString(_T("T1"));
	m_conprof.AddString(_T("Modem 56k"));
	m_conprof.AddString(_T("ISDN 64k"));
	m_conprof.AddString(_T("ISDN 128k"));

	int	iProfile = m_pPrefs->GetProfile();

	iProfile = (iProfile < m_conprof.GetCount()) ? iProfile : 0;
	m_conprof.SetCurSel(iProfile);

	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_PORT)))->SetLimitText(5);
	(reinterpret_cast<CEdit*>(GetDlgItem(IDC_UDPPORT)))->SetLimitText(5);

	LoadSettings();
	EnableEditBoxes((iProfile == 0) ? TRUE : FALSE);
	GetDlgItem(IDC_OPENPORTS)->EnableWindow(g_App.m_pPrefs->GetWindowsVersion() == _WINVER_XP_ || g_App.m_pPrefs->GetWindowsVersion() == _WINVER_SE_);
	Localize();

	return TRUE;
}

void CPPgConnection::LoadSettings(void)
{
	if (::IsWindow(m_hWnd))
	{
		if (m_pPrefs->GetMaxUpload() != 0)
		{
			m_pPrefs->SetMaxDownload(TieUploadDownload(m_pPrefs->GetMaxUpload(), m_pPrefs->GetMaxDownloadValue()));
		}

		m_strPort.Format(_T("%u"), m_pPrefs->GetPort());

		unsigned uiVal = m_pPrefs->GetUDPPort();

		m_bUdpDisable = (uiVal == 0);
		GetDlgItem(IDC_UDPPORT)->EnableWindow(uiVal != 0);
		uiVal = (uiVal != 0) ? uiVal : (m_pPrefs->GetPort() + (m_pPrefs->GetPort() & 0x1E) + 1);
		m_strUdpPort.Format(_T("%u"), (uiVal > 0xFFFF) ? (uiVal - 40) : uiVal);

		m_strDownloadCap.Format(_T("%u"), m_pPrefs->GetMaxGraphDownloadRate() / 10);
		m_strUploadCap.Format(_T("%u"), m_pPrefs->GetMaxGraphUploadRate() / 10);

		uiVal = m_pPrefs->GetMaxDownload();
		if (uiVal == UNLIMITED)
			m_strDownloadMax = _T("0");
		else
			FractionalRate2String(&m_strDownloadMax, m_pPrefs->GetMaxDownloadValue());

		FractionalRate2String(&m_strUploadMax, m_pPrefs->GetMaxUpload());

		m_strMaxConn.Format(_T("%u"), m_pPrefs->GetMaxConnections());
		m_bLancastEnable = m_pPrefs->GetLancastEnabled();
		m_bOpenPorts = m_pPrefs->GetOpenPorts();
		m_strQueueSize.Format(_T("%u"), m_pPrefs->GetQueueSize());
		m_strMaxCon5.Format(_T("%u"), m_pPrefs->GetMaxConPerFive());
		m_bShowOverhead = m_pPrefs->ShowOverhead();
		m_bLimitlessDownload = m_pPrefs->LimitlessDownload();

		UpdateData(FALSE);

		SetModified(FALSE);
	}
}

BOOL CPPgConnection::OnApply()
{
	if (m_bModified)
	{
		int iTmp;

		UpdateData(TRUE);

		uint32	dwLastMaxGU = m_pPrefs->GetMaxGraphUploadRate();
		uint32	dwLastMaxGD = m_pPrefs->GetMaxGraphDownloadRate();
		bool	bLastOpenPorts = m_pPrefs->GetOpenPorts();

		OnSpeedChange();

		m_pPrefs->SetProfile(m_conprof.GetCurSel());

		m_pPrefs->SetMaxGraphDownloadRate((iTmp = _tstoi(m_strDownloadCap)) * 10);
		m_strDownloadCap.Format(_T("%u"), iTmp);
		m_pPrefs->SetMaxGraphUploadRate((iTmp = _tstoi(m_strUploadCap)) * 10);
		m_strUploadCap.Format(_T("%u"), iTmp);

		m_pPrefs->SetMaxUpload(String2FranctionalRate(m_strUploadMax));
	//	Here zero can be only when limitless is enabled and upload limit >= 10
		m_pPrefs->SetMaxDownload(((iTmp = String2FranctionalRate(m_strDownloadMax)) != 0) ? iTmp : m_pPrefs->GetMaxGraphDownloadRate());

	//	Valid values range is 1..65535
		if ((unsigned)((iTmp = _tstoi(m_strPort)) - 1) <= 0xFFFE)
			m_pPrefs->SetPort(static_cast<uint16>(iTmp));
		else
			iTmp = m_pPrefs->GetPort();
		m_strPort.Format(_T("%u"), iTmp);

		iTmp = ((((iTmp = _tstoi(m_strUdpPort)) != 0) && !m_bUdpDisable) ? iTmp : 0);
		if (iTmp > 0xFFFF)
		{
		//	Set UDP port based on the TCP port as value isn't in valid range
			iTmp = ((iTmp = m_pPrefs->GetPort() + (m_pPrefs->GetPort() & 0x1E) + 1) > 0xFFFF) ? (iTmp - 40) : iTmp;
		}
		m_pPrefs->SetUDPPort(static_cast<uint16>(iTmp));
		if (iTmp != 0)	//avoid display 0 what means disabled
			m_strUdpPort.Format(_T("%u"), iTmp);
		m_bUdpDisable = (iTmp == 0);
		GetDlgItem(IDC_UDPPORT)->EnableWindow(iTmp != 0);

		m_pPrefs->SetShowOverhead(B2b(m_bShowOverhead));
	//	Now force an update of statusbar
		g_App.m_pMDlg->ResizeStatusBar();
		g_App.m_pMDlg->ShowTransferRate();

		uint32	mu = m_pPrefs->GetMaxGraphUploadRate();

		if (dwLastMaxGU != mu)
			g_App.m_pMDlg->m_dlgStatistics.SetARange(false, mu / 10);

		uint32 md = m_pPrefs->GetMaxGraphDownloadRate();

		if (dwLastMaxGD != md)
			g_App.m_pMDlg->m_dlgStatistics.SetARange(true, md / 10);

		m_pPrefs->SetLancastEnabled(B2b(m_bLancastEnable));

		m_pPrefs->SetOpenPorts(B2b(m_bOpenPorts));
		if (bLastOpenPorts != m_pPrefs->GetOpenPorts())
		{
			g_App.m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_UDP);
			g_App.m_pFirewallOpener->RemoveRule(EMULE_DEFAULTRULENAME_TCP);

			bool	bAlreadyExisted = (bool)(g_App.m_pFirewallOpener->DoesRuleExist(m_pPrefs->GetPort(), NAT_PROTOCOL_TCP) || g_App.m_pFirewallOpener->DoesRuleExist(m_pPrefs->GetUDPPort(), NAT_PROTOCOL_UDP));
			bool	bResult = g_App.m_pFirewallOpener->OpenPort(m_pPrefs->GetPort(), NAT_PROTOCOL_TCP, EMULE_DEFAULTRULENAME_TCP, false);

			if (m_pPrefs->GetUDPPort() != 0)
				bResult = bResult && g_App.m_pFirewallOpener->OpenPort(m_pPrefs->GetUDPPort(), NAT_PROTOCOL_UDP, EMULE_DEFAULTRULENAME_UDP, false);
			if (bResult)
			{
				if (!bAlreadyExisted)
					AfxMessageBox(GetResString(IDS_FO_PREF_SUCCCEEDED), MB_ICONINFORMATION | MB_OK);
				else
					// TODO: actually we could offer the user to remove existing rules
					AfxMessageBox(GetResString(IDS_FO_PREF_EXISTED), MB_ICONINFORMATION | MB_OK);
			}
			else
				AfxMessageBox(GetResString(IDS_FO_PREF_FAILED), MB_ICONSTOP | MB_OK);
		}

	//	Set range of the toolbar-speed-meter
		uint32	dwMaxRange = m_pPrefs->GetMaxGraphDownloadRate();

		if (m_pPrefs->GetMaxGraphUploadRate() > dwMaxRange)
			dwMaxRange = m_pPrefs->GetMaxGraphUploadRate();
		UINT nLastMaxRange, nLastMinRange;
		g_App.m_pMDlg->m_ctlToolBar.GetSpeedMeterRange(nLastMaxRange, nLastMinRange);
		dwMaxRange /= 10;
		if (nLastMaxRange != dwMaxRange)
			g_App.m_pMDlg->m_ctlToolBar.SetSpeedMeterRange(dwMaxRange, nLastMinRange);

		unsigned uiOSMaxCon = ::GetMaxConnections();

		if ((iTmp = _tstoi(m_strMaxConn)) == 0)
			iTmp = CPreferences::GetRecommendedMaxConnections();
		if (static_cast<unsigned>(iTmp) > uiOSMaxCon)
		{
			CString strMessage;
			strMessage.Format(GetResString(IDS_PW_WARNING), GetResString(IDS_PW_MAXC), uiOSMaxCon);
			int iResult = MessageBox(strMessage, GetResString(IDS_PW_MAXC), MB_ICONWARNING | MB_YESNO);
			if (iResult != IDYES)
				iTmp = CPreferences::GetRecommendedMaxConnections();
		}
		m_strMaxConn.Format(_T("%u"), iTmp);
		m_pPrefs->SetMaxConnections(static_cast<uint16>(iTmp));

		int qs = _tstoi(m_strQueueSize);

		if (qs > MAX_QUEUE)
			qs = MAX_QUEUE;
		if (qs < MIN_QUEUE)
			qs = MIN_QUEUE;

		m_strQueueSize.Format(_T("%u"), qs);
		m_pPrefs->SetQueueSize(qs);

		int mc5 = _tstoi(m_strMaxCon5);
		int nMax = (LOBYTE(g_App.m_pPrefs->GetWindowsVersion()) == 0x04) ? 100 : 200;

		if (mc5 > nMax)
			mc5 = nMax;
		else if (mc5 < 5)
			mc5 = 5;

		m_strMaxCon5.Format(_T("%u"), mc5);
		m_pPrefs->SetMaxDownloadConperFive(mc5);

		m_pPrefs->SetLimitlessDownload(B2b(m_bLimitlessDownload));

		SetModified(FALSE);
		UpdateData(FALSE);
	}

	return CPropertyPage::OnApply();
}

void CPPgConnection::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_CONPROF, IDS_CONPROF },
		{ IDC_TR_FRM, IDS_TR_FRM },
		{ IDC_CAPACITIES, IDS_PW_CON_CAP },
		{ IDC_LIMITS, IDS_PW_CON_LIMIT },
		{ IDC_DCAP_LBL, IDS_DOWNLOAD_NOUN },
		{ IDC_UCAP_LBL, IDS_UPLOAD_NOUN },
		{ IDC_KBS1, IDS_KBYTESEC },
		{ IDC_KBS2, IDS_KBYTESEC },
		{ IDC_QUEUE_FRM, IDS_QUEUE_FRM },
		{ IDC_QUEUESIZE_TEXT, IDS_QUEUESIZE_TEXT },
		{ IDC_MAXCONN_FRM, IDS_PW_MAXC },
		{ IDC_CLIENTPORT_FRM, IDS_PW_CLIENTPORT },
		{ IDC_MAXCON5_TEXT, IDS_MAXCON5_TEXT },
		{ IDC_UDPDISABLE, IDS_DISABLED },
		{ IDC_LIMITLESS, IDS_LIMITLESS },
		{ IDC_SHOWOVERHEAD, IDS_SHOWOVERHEAD },
		{ IDC_LANCASTENABLE, IDS_LANCASTENABLE },
		{ IDC_OPENPORTS, IDS_OPENPORTSENABLE }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;
		int		iCS;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
		
		iCS = m_conprof.GetCurSel();
		m_conprof.DeleteString(0);
		GetResString(&strRes, IDS_CUSTOMPROFIL);
		m_conprof.InsertString(0, strRes);
		if (iCS != CB_ERR)
			m_conprof.SetCurSel(iCS);
	}
}

void CPPgConnection::OnCbnSelchangeConType(void)
{
	static const TCHAR *pcCaps[][5] = {
		{ _T("32"), _T("16"), _T("32"), _T("14"), _T("256") },
		{ _T("64"), _T("16"), _T("64"), _T("14"), _T("512") },
		{ _T("64"), _T("32"), _T("64"), _T("30"), _T("512") },
		{ _T("96"), _T("16"), _T("96"), _T("14"), _T("512") },
		{ _T("128"), _T("32"), _T("128"), _T("30"), _T("768") },
		{ _T("128"), _T("64"), _T("128"), _T("60"), _T("1024") },
		{ _T("128"), _T("48"), _T("128"), _T("44"), _T("1024") },
		{ _T("180"), _T("180"), _T("180"), _T("180"), _T("2048") },
		{ _T("5"), _T("2"), _T("5"), _T("2"), _T("80") },
		{ _T("8"), _T("8"), _T("8"), _T("6"), _T("80") },
		{ _T("16"), _T("16"), _T("16"), _T("14"), _T("100") }
	};
	int cs = m_conprof.GetCurSel();

	if (--cs < 0)
		EnableEditBoxes(TRUE);
	else
	{
		EnableEditBoxes(FALSE);
		if (static_cast<unsigned>(cs) < ARRSIZE(pcCaps))
		{
			m_strDownloadCap = pcCaps[cs][0];
			m_strUploadCap = pcCaps[cs][1];
			m_strDownloadMax = pcCaps[cs][2];
			m_strUploadMax = pcCaps[cs][3];
			m_strMaxConn = pcCaps[cs][4];
		}
	}
	SetModified();
	UpdateData(FALSE);
}

void CPPgConnection::EnableEditBoxes(BOOL bEnable)
{
	UpdateData(TRUE);

	GetDlgItem(IDC_DOWNLOAD_CAP)->EnableWindow(bEnable);
	GetDlgItem(IDC_UPLOAD_CAP)->EnableWindow(bEnable);
	GetDlgItem(IDC_MAXDOWN)->EnableWindow(bEnable && !m_bLimitlessDownload);
	GetDlgItem(IDC_MAXUP)->EnableWindow(bEnable);
	GetDlgItem(IDC_MAXCON)->EnableWindow(bEnable);
	OnSpeedChange();	// update download speed in case of limitless
}

void CPPgConnection::OnQueueChange()
{
	UpdateData(TRUE);

	uint32 qs = _tstoi(m_strQueueSize);

	if (qs > MAX_QUEUE)
		qs = MAX_QUEUE;
	if (qs < MIN_QUEUE)
		qs = MIN_QUEUE;

	UpdateData(FALSE);
}

void CPPgConnection::OnSpeedChange()
{
	UpdateData(TRUE);
	
	uint32 uc;
	if (!m_strUploadCap.IsEmpty())
		uc = ValidateUpCapability(10 * _tstoi(m_strUploadCap));
	else
		uc = m_pPrefs->GetMaxGraphUploadRate();
	m_strUploadCap.Format(_T("%u"), uc / 10);

	uint32 mu;
	if (!m_strUploadMax.IsEmpty())
	{
		if ((mu = String2FranctionalRate(m_strUploadMax)) < 10)
			mu = (mu == 0) ? ~0 : 10;
	}
	else
		mu = m_pPrefs->GetMaxUpload();
	mu = (mu > uc) ? uc : mu;

	uint32 dc;
	if (!m_strDownloadCap.IsEmpty())
		dc = ValidateDownCapability(10 * _tstoi(m_strDownloadCap));
	else
		dc = m_pPrefs->GetMaxGraphDownloadRate();
	m_strDownloadCap.Format(_T("%u"), dc / 10);

	uint32 md;

	if (m_bLimitlessDownload)
		md = (mu >= 100) ? UNLIMITED : dc;
	else
	{
		if (!m_strDownloadMax.IsEmpty())
		{
			if ((md = String2FranctionalRate(m_strDownloadMax)) < 10)
				md = (md == 0) ? ~0 : 10;
		}
		else
			md = m_pPrefs->GetMaxDownloadValue();
		md = (md > dc) ? dc : md;
	}

	FractionalRate2String(&m_strUploadMax, mu);
	md = TieUploadDownload(mu, md);
	if (md == UNLIMITED)
		m_strDownloadMax = _T("0");
	else
		FractionalRate2String(&m_strDownloadMax, md);

	UpdateData(FALSE);
}

void CPPgConnection::OnEnChangeUDPDisable()
{
	UpdateData(TRUE);
	GetDlgItem(IDC_UDPPORT)->EnableWindow(!m_bUdpDisable);

	if (!m_bUdpDisable && (m_pPrefs->GetUDPPort() == 0))
	{
		unsigned p = _tstoi(m_strPort);

	//	Valid values range is 1..65535
		p = (((p - 1) <= 0xFFFE) ? p : m_pPrefs->GetPort());
		p += (p & 0x1E) + 1;
		m_strUdpPort.Format(_T("%u"), (p > 0xFFFF) ? (p - 40) : p);
	}

	SetModified();

	UpdateData(FALSE);
}

void CPPgConnection::OnBnClickedLimitless()
{
	UpdateData(TRUE);

	GetDlgItem(IDC_MAXDOWN)->EnableWindow((m_conprof.GetCurSel() == 0) && !m_bLimitlessDownload);
	OnSpeedChange();

	SetModified();
}
