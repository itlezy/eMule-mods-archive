//this file is part of NeoMule
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
#include "ListViewWalkerPreferenceSheet.h"
#include "UserMsgs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// CListViewWalkerPreferenceSheet

IMPLEMENT_DYNAMIC(CListViewWalkerPreferenceSheet, CPropertySheet)

BEGIN_MESSAGE_MAP(CListViewWalkerPreferenceSheet, CPropertySheet)
	ON_BN_CLICKED(IDC_NEXT, OnNext)
	ON_BN_CLICKED(IDC_PREV, OnPrev)
END_MESSAGE_MAP()

CListViewWalkerPreferenceSheet::CListViewWalkerPreferenceSheet(UINT nIDCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(nIDCaption, pParentWnd, iSelectPage)
{
}

CListViewWalkerPreferenceSheet::CListViewWalkerPreferenceSheet(LPCTSTR pszCaption, CWnd* pParentWnd, UINT iSelectPage)
	:CPropertySheet(pszCaption, pParentWnd, iSelectPage)
{
}

CListViewWalkerPreferenceSheet::~CListViewWalkerPreferenceSheet()
{
}

BOOL CListViewWalkerPreferenceSheet::OnInitDialog()
{
	BOOL bResult = CPropertySheet::OnInitDialog();

	//////////////////////////////////////////////////////////////////////////
	// Add additional controls
	//
	//if (m_pListCtrl != NULL) // NEO: FCFG - [FileConfiguration]
	{
		// switching from multi-selection to single selection is currently not supported -> disable Up/Down controls
		DWORD dwCtrlStyle = (m_aItems.GetSize()>1) ? WS_DISABLED : 0;

		const struct
		{
			CButton	   *pCtlBtn;
			UINT		uCtlId;
			LPCTSTR		pszLabel;
			LPCTSTR		pszSymbol;
			DWORD		dwStyle;
		} aCtrls[] =
		{
			{ &m_ctlPrev,  IDC_PREV, _T("&Prev"),  _T("5"), WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP },
			{ &m_ctlNext,  IDC_NEXT, _T("&Next"),  _T("6"), WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP },
			// NEO: FCFG - [FileConfiguration]
			{ &m_ctlCopy,  IDC_CPY,  _T("&Copy"),  _T("") , WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP },
			{ &m_ctlPaste, IDC_PST,  _T("&Paste"), _T("") , WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP },
			{ &m_ctlReset, IDC_RST,  _T("&Reset"), _T("") , WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP }
			// NEO: FCFG END
		};

		int iLeftMostButtonId = IDOK;
		int iMax = 32767;
		static const int _aiPropSheetButtons[] = { IDOK, IDCANCEL, ID_APPLY_NOW, IDHELP };
		for (int i = 0; i < ARRSIZE(_aiPropSheetButtons); i++)
		{
			CWnd* pBtn = GetDlgItem(_aiPropSheetButtons[i]);
			if (pBtn /*&& pBtn->IsWindowVisible()*/)
			{
				CRect rcBtn;
				pBtn->GetWindowRect(&rcBtn);
				ScreenToClient(&rcBtn);
				if (rcBtn.left < iMax)
				{
					iMax = rcBtn.left;
					iLeftMostButtonId = _aiPropSheetButtons[i];
				}
			}
		}

		CWnd* pctlOk = GetDlgItem(iLeftMostButtonId);
		CRect rcOk;
		pctlOk->GetWindowRect(&rcOk);
		ScreenToClient(&rcOk);
		CFont* pDefCtrlFont = pctlOk->GetFont();

		for (int i = m_pListCtrl ? 0 : 2; i < ARRSIZE(aCtrls); i++) // NEO: FCFG - [FileConfiguration]
		{
			const int iNaviBtnWidth = rcOk.Width()/2;
			CRect rc;
			rc.left = rcOk.left - (8 + iNaviBtnWidth) * (ARRSIZE(aCtrls) - i);
			rc.top = rcOk.top;
			rc.right = rc.left + iNaviBtnWidth;
			rc.bottom = rc.top + rcOk.Height();
			VERIFY( aCtrls[i].pCtlBtn->Create(aCtrls[i].pszLabel, (i < 3 ? dwCtrlStyle : 0) | aCtrls[i].dwStyle, rc, this, aCtrls[i].uCtlId) ); // NEO: FCFG - [FileConfiguration]

			if (theApp.m_fontSymbol.m_hObject && i < 2) // NEO: FCFG - [FileConfiguration]
			{
				aCtrls[i].pCtlBtn->SetFont(&theApp.m_fontSymbol);
				aCtrls[i].pCtlBtn->SetWindowText(aCtrls[i].pszSymbol); // show down-arrow
			}
			else
				aCtrls[i].pCtlBtn->SetFont(pDefCtrlFont);
			//AddAnchor(*aCtrls[i].pCtlBtn, BOTTOM_RIGHT);
		}
	}

	return bResult;
}

void CListViewWalkerPreferenceSheet::ChangeData(CObject* pObj)
{
	m_aItems.RemoveAll();
	m_aItems.Add(pObj);
	SendMessage(UM_DATA_CHANGED);

	for (int iPage = 0; iPage < GetPageCount(); iPage++)
	{
		CPropertyPage* pPage = GetPage(iPage);
		if (pPage && pPage->m_hWnd)
		{
			pPage->SendMessage(UM_DATA_CHANGED);
			pPage->SetModified(FALSE);
		}
	}
	GetActivePage()->OnSetActive();
}

void CListViewWalkerPreferenceSheet::OnPrev()
{
	ASSERT( m_pListCtrl != NULL );
	if (m_pListCtrl == NULL)
		return;

	CObject* pObj = m_pListCtrl->GetPrevSelectableItem();
	if (pObj)
		ChangeData(pObj);
	else
		MessageBeep(MB_OK);
}

void CListViewWalkerPreferenceSheet::OnNext()
{
	ASSERT( m_pListCtrl != NULL );
	if (m_pListCtrl == NULL)
		return;

	CObject* pObj = m_pListCtrl->GetNextSelectableItem(); 
	if (pObj)
		ChangeData(pObj);
	else
		MessageBeep(MB_OK);
}