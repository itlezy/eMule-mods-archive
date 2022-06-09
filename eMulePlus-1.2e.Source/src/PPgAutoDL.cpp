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
#include "emule.h"
#include "PPgAutoDL.h"
#include "AutoDL.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


IMPLEMENT_DYNAMIC(CPPgAutoDL, CPropertyPage)

CPPgAutoDL::CPPgAutoDL()
	: CPropertyPage(CPPgAutoDL::IDD)
	, m_lInterval(0)
	, m_bAutoDLEnabled(FALSE)
{
}

CPPgAutoDL::~CPPgAutoDL()
{
}

void CPPgAutoDL::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Check(pDX, IDC_ADL_USEIT, m_bAutoDLEnabled);
	DDX_Control(pDX, IDC_ADL_LIST, m_ctrlList);
	DDX_Text(pDX, IDC_EDITINTERVAL, m_lInterval);
}

BEGIN_MESSAGE_MAP(CPPgAutoDL, CPropertyPage)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_ADL_LIST, OnLvnItemchangedList)
	ON_NOTIFY(LVN_ENDLABELEDIT, IDC_ADL_LIST, OnLvnEndlabeleditList)
	ON_BN_CLICKED(IDC_ADL_USEIT, OnBnClickedAutoDL)
	ON_BN_CLICKED(IDC_ADL_ISET, OnBnClickedSet)
	ON_BN_CLICKED(IDC_ADL_NEW, OnBnClickedNew)
	ON_BN_CLICKED(IDC_ADL_DELETE, OnBnClickedDelete)
END_MESSAGE_MAP()

BOOL CPPgAutoDL::OnInitDialog()
{
	CPropertyPage::OnInitDialog();

	CRect rect;
	m_ctrlList.GetClientRect(&rect);
	m_ctrlList.InsertColumn(0, _T(""), LVCFMT_LEFT, rect.Width() - 70);
	m_ctrlList.InsertColumn(1, _T(""), LVCFMT_RIGHT, 60);

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgAutoDL::LoadSettings(void)
{
	if(::IsWindow(m_hWnd))
	{
		CString s;

		m_bAutoDLEnabled = g_App.m_pAutoDL->UseIt;
		m_ctrlList.DeleteAllItems();
		for(int i = 0; i < g_App.m_pAutoDL->UrlCount; i++)
		{
			CAutoDLData data = g_App.m_pAutoDL->UrlItem[i];
			int nIndex = m_ctrlList.GetItemCount();
			nIndex = m_ctrlList.InsertItem(nIndex, data.acUrl);
			s.Format(_T("%ld"), data.ulInterval);
			m_ctrlList.SetItemText(nIndex, 1, s);
			m_ctrlList.SetItemData(nIndex, data.ulInterval);
		}

		UpdateData(FALSE);

		OnBnClickedAutoDL();

		SetModified(FALSE);
	}
}

BOOL CPPgAutoDL::OnApply()
{
	if(m_bModified)
	{
		UpdateData(TRUE);

		g_App.m_pAutoDL->UseIt = B2b(m_bAutoDLEnabled);
		g_App.m_pAutoDL->ClearUrlList();
		for(int i = 0; i < m_ctrlList.GetItemCount(); i++)
		{
			CAutoDLData data;
			_tcsncpy(data.acUrl, m_ctrlList.GetItemText(i, 0), DLURLMAX);
			data.acUrl[DLURLMAX - 1] = _T('\0');
			data.ulInterval = m_ctrlList.GetItemData(i);
			data.lLastCheck = 0;
			g_App.m_pAutoDL->AddUrlItem(data);
		}
		g_App.m_pAutoDL->SavePrefs();
		// Restart feature
		g_App.m_pAutoDL->Restart();

		SetModified(FALSE);
	}
	return CPropertyPage::OnApply();
}

void CPPgAutoDL::Localize(void)
{	
	static const uint16 s_auResTbl[] =
	{
		IDS_SV_URL,			//URL
		IDS_INTERVAL		//Interval
	};
	static const uint16 s_auResTbl2[][2] =
	{
		{ IDC_ADL_USEIT, IDS_ENABLED },
		{ IDC_ADL_ISET, IDS_PW_APPLY },
		{ IDC_ADL_URLS, IDS_URL_CONFIG },
		{ IDC_ADL_ITITLE, IDS_ADL_INTERVALBL },
		{ IDC_ADL_NEW, IDS_NEW_URL },
		{ IDC_ADL_DELETE, IDS_ERASE }
	};

	if(::IsWindow(m_hWnd))
	{
		CHeaderCtrl	*pHeaderCtrl = m_ctrlList.GetHeaderCtrl();
		CString		strRes;
		HDITEM		hdi;

		hdi.mask = HDI_TEXT;

		for (unsigned ui = 0; ui < ARRSIZE(s_auResTbl); ui++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl[ui]));
			hdi.pszText = const_cast<LPTSTR>(strRes.GetString());
			pHeaderCtrl->SetItem(static_cast<int>(ui), &hdi);
		}
		for (uint32 i = 0; i < ARRSIZE(s_auResTbl2); i++)
		{
			::GetResString(&strRes, static_cast<UINT>(s_auResTbl2[i][1]));
			SetDlgItemText(s_auResTbl2[i][0], strRes);
		}
	}
}

void CPPgAutoDL::OnBnClickedAutoDL()
{
	UpdateData(TRUE);

	m_ctrlList.EnableWindow(m_bAutoDLEnabled);
	GetDlgItem(IDC_EDITINTERVAL)->EnableWindow(m_bAutoDLEnabled);
	GetDlgItem(IDC_ADL_ISET)->EnableWindow(m_bAutoDLEnabled);
	GetDlgItem(IDC_ADL_NEW)->EnableWindow(m_bAutoDLEnabled);
	GetDlgItem(IDC_ADL_DELETE)->EnableWindow(m_bAutoDLEnabled);

	SetModified();
}

void CPPgAutoDL::OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	if(pNMLV->uOldState & LVIS_SELECTED)
	{
		UpdateData();
		CString s; 
		if(m_lInterval > 0)
			s.Format(_T("%ld"), m_lInterval);
		else
			s = _T("---");
		m_ctrlList.SetItemText(pNMLV->iItem, 1, s);
		m_ctrlList.SetItemData(pNMLV->iItem, m_lInterval);
		SetModified();
	}
	else if(pNMLV->uNewState & LVIS_SELECTED)
	{
		UpdateData(TRUE);
		m_lInterval = m_ctrlList.GetItemData(pNMLV->iItem);
		UpdateData(FALSE);
	}
	*pResult = 0;
}

void CPPgAutoDL::OnLvnEndlabeleditList(NMHDR *pNMHDR, LRESULT *pResult)
{
	NMLVDISPINFO *pDispInfo = reinterpret_cast<NMLVDISPINFO*>(pNMHDR);
	
	if(pDispInfo && pDispInfo->item.pszText != NULL)
		m_ctrlList.SetItemText(pDispInfo->item.iItem, 0, pDispInfo->item.pszText);

	*pResult = 0;
}

void CPPgAutoDL::OnBnClickedSet()
{
	POSITION pos = m_ctrlList.GetFirstSelectedItemPosition();
	if(pos)
	{
		UpdateData();
		int nItem = m_ctrlList.GetNextSelectedItem(pos);
		CString s; 
		if(m_lInterval > 0)
			s.Format(_T("%ld"), m_lInterval);
		else
			s = _T("---");
		m_ctrlList.SetItemText(nItem, 1, s);
		m_ctrlList.SetItemData(nItem, m_lInterval);
		SetModified();
	}
}

void CPPgAutoDL::OnBnClickedNew()
{
	int nPos = m_ctrlList.GetItemCount();
	nPos = m_ctrlList.InsertItem(nPos, _T("http://enter_your_url"));
	m_ctrlList.SetItemText(nPos, 1, _T("60"));
	m_ctrlList.SetItemData(nPos, 60);
}

void CPPgAutoDL::OnBnClickedDelete()
{
	POSITION pos = m_ctrlList.GetFirstSelectedItemPosition();
	if(pos)
	{
		int nItem = m_ctrlList.GetNextSelectedItem(pos);
		m_ctrlList.DeleteItem(nItem);
	}
}
