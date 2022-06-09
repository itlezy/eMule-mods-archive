#include "stdafx.h"
#include "FilterItem.h"
#include "KnownFile.h"
#include "EditDelayed.h"
#include "PartFile.h"
#include "Preferences.h"
#include "FilterDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

bool CFilterItem::IsFilteredItem(const CAbstractFile* pKnownFile) const
{
	if (m_astrFilter.GetCount() == 0)
		return false;

	// filtering is done by text only for all colums to keep it consistent and simple for the user even if that
	// doesn't allows complex filters
	TCHAR szFilterTarget[256];
	GetItemDisplayText(pKnownFile, m_nFilterColumn,
					   szFilterTarget, _countof(szFilterTarget));

	bool bItemFiltered = false;
	for (size_t i = 0; i < m_astrFilter.GetCount(); i++)
	{
		const CString& rstrExpr = m_astrFilter.GetAt(i);
		bool bAnd = true;
		LPCTSTR pszText = (LPCTSTR)rstrExpr;
		if (pszText[0] == _T('-')) {
			pszText += 1;
			bAnd = false;
		}

		bool bFound = (wcsistr(szFilterTarget, pszText) != NULL);
		if ((bAnd && !bFound) || (!bAnd && bFound)) {
			bItemFiltered = true;
			break;
		}
	}
	return bItemFiltered;
}

BOOL CFilterItem::SetFilter(size_t newfilter)
{
	// category filter menuitems
	if(filter != newfilter){
		dlg->filter = filter = newfilter;
		ReloadFileList();
		dlg->UpdateFilterLabel();
	}
	return TRUE;
}

