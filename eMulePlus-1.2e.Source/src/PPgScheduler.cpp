// PPgScheduler.cpp : implementation file

#include "stdafx.h"
#include "emule.h"
#include "PPgScheduler.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CPPgScheduler, CPropertyPage)
CPPgScheduler::CPPgScheduler()
	: CPropertyPage(CPPgScheduler::IDD)
	, m_bSchedulerEnabled(FALSE)
	, m_bExceptMon(FALSE)
	, m_bExceptTue(FALSE)
	, m_bExceptWed(FALSE)
	, m_bExceptThu(FALSE)
	, m_bExceptFri(FALSE)
	, m_bExceptSat(FALSE)
	, m_bExceptSun(FALSE)
{
}

CPPgScheduler::~CPPgScheduler()
{
}

void CPPgScheduler::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SCH_SHIFT1_TIME, m_Shift1TimeCombo);
	DDX_Control(pDX, IDC_SCH_SHIFT2_TIME, m_Shift2TimeCombo);
	DDX_Check(pDX, IDC_SCH_ENABLED, m_bSchedulerEnabled);
	DDX_Check(pDX, IDC_SCH_EXCEPT_MON, m_bExceptMon);
	DDX_Check(pDX, IDC_SCH_EXCEPT_TUE, m_bExceptTue);
	DDX_Check(pDX, IDC_SCH_EXCEPT_WED, m_bExceptWed);
	DDX_Check(pDX, IDC_SCH_EXCEPT_THU, m_bExceptThu);
	DDX_Check(pDX, IDC_SCH_EXCEPT_FRI, m_bExceptFri);
	DDX_Check(pDX, IDC_SCH_EXCEPT_SAT, m_bExceptSat);
	DDX_Check(pDX, IDC_SCH_EXCEPT_SUN, m_bExceptSun);
	DDX_Text(pDX, IDC_SCH_SHIFT1_DOWN, m_strShift1Down);
	DDX_Text(pDX, IDC_SCH_SHIFT1_UP, m_strShift1Up);
	DDX_Text(pDX, IDC_SCH_SHIFT1_CONN, m_strShift1Conn);
	DDX_Text(pDX, IDC_SCH_SHIFT1_5SEC, m_strShift1Conn5Sec);
	DDX_Text(pDX, IDC_SCH_SHIFT2_UP, m_strShift2Up);
	DDX_Text(pDX, IDC_SCH_SHIFT2_DOWN, m_strShift2Down);
	DDX_Text(pDX, IDC_SCH_SHIFT2_CONN, m_strShift2Conn);
	DDX_Text(pDX, IDC_SCH_SHIFT2_5SEC, m_strShift2Conn5Sec);
}

BEGIN_MESSAGE_MAP(CPPgScheduler, CPropertyPage)
	ON_BN_CLICKED(IDC_SCH_ENABLED, OnBnClickedSCHEnabled)
	ON_CBN_SELCHANGE(IDC_SCH_SHIFT1_TIME, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_SCH_SHIFT2_TIME, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT1_UP, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT1_DOWN, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT1_CONN, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT1_5SEC, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT2_UP, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT2_DOWN, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT2_CONN, OnSettingsChange)
	ON_EN_CHANGE(IDC_SCH_SHIFT2_5SEC, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCH_EXCEPT_MON, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCH_EXCEPT_TUE, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCH_EXCEPT_WED, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCH_EXCEPT_THU, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCH_EXCEPT_FRI, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCH_EXCEPT_SAT, OnSettingsChange)
	ON_BN_CLICKED(IDC_SCH_EXCEPT_SUN, OnSettingsChange)
END_MESSAGE_MAP()

BOOL CPPgScheduler::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CString	strTime;
	int		iRc;

	for (uint32 dwInterval = 0; dwInterval < (24 * 3600); dwInterval += 1800)
	{
		strTime.Format(_T("%u:%02u"), dwInterval / 3600, dwInterval % 3600 / 60);
		m_Shift1TimeCombo.SetItemData(iRc = m_Shift1TimeCombo.AddString(strTime), dwInterval);
		if (dwInterval == m_pPrefs->GetSCHShift1())
			m_Shift1TimeCombo.SetCurSel(iRc);
		m_Shift2TimeCombo.SetItemData(iRc = m_Shift2TimeCombo.AddString(strTime), dwInterval);
		if (dwInterval == m_pPrefs->GetSCHShift2())
			m_Shift2TimeCombo.SetCurSel(iRc);
	}

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgScheduler::LoadSettings(void)
{
	if (::IsWindow(m_hWnd))
	{
		if(m_pPrefs->GetSCHShift1Upload() == UNLIMITED)
			m_strShift1Up = _T("0");
		else
			FractionalRate2String(&m_strShift1Up, m_pPrefs->GetSCHShift1Upload());

		if(m_pPrefs->GetSCHShift1Download() == UNLIMITED)
			m_strShift1Down = _T("0");
		else
			FractionalRate2String(&m_strShift1Down, m_pPrefs->GetSCHShift1Download());
			
		m_strShift1Conn.Format(_T("%u"), m_pPrefs->GetSCHShift1conn());
		m_strShift1Conn5Sec.Format(_T("%u"), m_pPrefs->GetSCHShift15sec());
		
		if(m_pPrefs->GetSCHShift2Upload() == UNLIMITED)
			m_strShift2Up = _T("0");
		else
			FractionalRate2String(&m_strShift2Up, m_pPrefs->GetSCHShift2Upload());

		if(m_pPrefs->GetSCHShift2Download() == UNLIMITED)
			m_strShift2Down = _T("0");
		else
			FractionalRate2String(&m_strShift2Down, m_pPrefs->GetSCHShift2Download());

		m_strShift2Conn.Format(_T("%u"), m_pPrefs->GetSCHShift2conn());
		m_strShift2Conn5Sec.Format(_T("%u"), m_pPrefs->GetSCHShift25sec());

		m_bExceptMon = m_pPrefs->IsSCHExceptMon();
		m_bExceptTue = m_pPrefs->IsSCHExceptTue();
		m_bExceptWed = m_pPrefs->IsSCHExceptWed();
		m_bExceptThu = m_pPrefs->IsSCHExceptThu();
		m_bExceptFri = m_pPrefs->IsSCHExceptFri();
		m_bExceptSat = m_pPrefs->IsSCHExceptSat();
		m_bExceptSun = m_pPrefs->IsSCHExceptSun();

		m_bSchedulerEnabled = m_pPrefs->IsSCHEnabled();

		UpdateData(FALSE);

		OnBnClickedSCHEnabled();

		SetModified(FALSE);
	}
}

BOOL CPPgScheduler::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		uint32	dwVal;

		if (!m_strShift1Up.IsEmpty())
		{
			dwVal = String2FranctionalRate(m_strShift1Up);
			m_pPrefs->SetSCHShift1Upload((uint16)((dwVal < 10) ? ((dwVal != 0) ? 10 : UNLIMITED) : dwVal));
		}
		if (!m_strShift1Down.IsEmpty())
		{
			dwVal = String2FranctionalRate(m_strShift1Down);
			m_pPrefs->SetSCHShift1Download((dwVal < 10) ? ((dwVal != 0) ? 10 : UNLIMITED) : dwVal);
		}
		g_App.m_pUploadQueue->SCHShift1UploadCheck();

		if (!m_strShift1Conn.IsEmpty())
		{
			int i = _tstoi(m_strShift1Conn);
			m_pPrefs->SetSCHShift1conn((uint16)((i != 0) ? i : 0xFFFF));
		}
		if (!m_strShift1Conn5Sec.IsEmpty())
		{
			int i = _tstoi(m_strShift1Conn5Sec);
			m_pPrefs->SetSCHShift15sec((uint16)((i != 0) ? i : 0xFFFF));
		}

		if (!m_strShift2Up.IsEmpty())
		{
			dwVal = String2FranctionalRate(m_strShift2Up);
			m_pPrefs->SetSCHShift2Upload((uint16)((dwVal < 10) ? ((dwVal != 0) ? 10 : UNLIMITED) : dwVal));
		}
		if (!m_strShift2Down.IsEmpty())
		{
			dwVal = String2FranctionalRate(m_strShift2Down);
			m_pPrefs->SetSCHShift2Download((dwVal < 10) ? ((dwVal != 0) ? 10 : UNLIMITED) : dwVal);
		}
		g_App.m_pUploadQueue->SCHShift2UploadCheck();

		if (!m_strShift2Conn.IsEmpty())
		{
			int i = _tstoi(m_strShift2Conn);
			m_pPrefs->SetSCHShift2conn((uint16)((i != 0) ? i : 0xFFFF));
		}
		if (!m_strShift2Conn5Sec.IsEmpty())
		{
			int i = _tstoi(m_strShift2Conn5Sec);
			m_pPrefs->SetSCHShift25sec((uint16)((i != 0) ? i : 0xFFFF));
		}

		if (m_Shift1TimeCombo.GetCurSel() != CB_ERR)
			m_pPrefs->SetSCHShift1(m_Shift1TimeCombo.GetItemData(m_Shift1TimeCombo.GetCurSel()));

		if (m_Shift2TimeCombo.GetCurSel() != CB_ERR)
			m_pPrefs->SetSCHShift2(m_Shift2TimeCombo.GetItemData(m_Shift2TimeCombo.GetCurSel()));

		m_pPrefs->SetSCHExceptMon(B2b(m_bExceptMon));
		m_pPrefs->SetSCHExceptTue(B2b(m_bExceptTue));
		m_pPrefs->SetSCHExceptWed(B2b(m_bExceptWed));
		m_pPrefs->SetSCHExceptThu(B2b(m_bExceptThu));
		m_pPrefs->SetSCHExceptFri(B2b(m_bExceptFri));
		m_pPrefs->SetSCHExceptSat(B2b(m_bExceptSat));
		m_pPrefs->SetSCHExceptSun(B2b(m_bExceptSun));

		m_pPrefs->SetSCHEnabled(B2b(m_bSchedulerEnabled));

		SetModified(FALSE);
	}
	return CPropertyPage::OnApply();
}

void CPPgScheduler::Localize(void)
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_SCH_SHIFT1, IDS_SCH_SHIFT1 },
		{ IDC_SCH_SHIFT2, IDS_SCH_SHIFT2 },
		{ IDC_SCH_KB1, IDS_KBYTESEC },
		{ IDC_SCH_KB2, IDS_KBYTESEC },
		{ IDC_SCH_TIME, IDS_TIME },
		{ IDC_SCH_DOWN, IDS_DOWNLOAD_NOUN },
		{ IDC_SCH_UP, IDS_UPLOAD_NOUN },
		{ IDC_SCH_CONN, IDS_PW_MAXC },
		{ IDC_SCH_5SEC, IDS_MAXCON5_TEXT },
		{ IDC_SCH_FRM, IDS_SCH_EXCEPT },
		{ IDC_SCH_EXCEPT_MON, IDS_SCH_EXCEPT_MON },
		{ IDC_SCH_EXCEPT_TUE, IDS_SCH_EXCEPT_TUE },
		{ IDC_SCH_EXCEPT_WED, IDS_SCH_EXCEPT_WED },
		{ IDC_SCH_EXCEPT_THU, IDS_SCH_EXCEPT_THU },
		{ IDC_SCH_EXCEPT_FRI, IDS_SCH_EXCEPT_FRI },
		{ IDC_SCH_EXCEPT_SAT, IDS_SCH_EXCEPT_SAT },
		{ IDC_SCH_EXCEPT_SUN, IDS_SCH_EXCEPT_SUN },
		{ IDC_SCH_ENABLED, IDS_ENABLED }
	};

	if (::IsWindow(m_hWnd))
	{
		CString	strRes;

		for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[i][1]));
			SetDlgItemText(s_auResTbl[i][0], strRes);
		}
	}
}

void CPPgScheduler::OnBnClickedSCHEnabled()
{
	static const uint16 s_auResTbl[] =
	{
		IDC_SCH_SHIFT1_TIME, IDC_SCH_SHIFT1_UP,
		IDC_SCH_SHIFT1_DOWN, IDC_SCH_SHIFT1_CONN,
		IDC_SCH_SHIFT1_5SEC, IDC_SCH_SHIFT2_TIME,
		IDC_SCH_SHIFT2_UP,   IDC_SCH_SHIFT2_DOWN,
		IDC_SCH_SHIFT2_CONN, IDC_SCH_SHIFT2_5SEC,
		IDC_SCH_EXCEPT_MON,  IDC_SCH_EXCEPT_TUE,
		IDC_SCH_EXCEPT_WED,  IDC_SCH_EXCEPT_THU,
		IDC_SCH_EXCEPT_FRI,  IDC_SCH_EXCEPT_SAT,
		IDC_SCH_EXCEPT_SUN
	};

	UpdateData(TRUE);

	for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		GetDlgItem(s_auResTbl[i])->EnableWindow(m_bSchedulerEnabled);

	SetModified();
}
