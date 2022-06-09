#pragma once

class CEditDelayed;
class CFilterDlg;
class CAbstractFile;
class CFilterItem{// X: [FI] - [FilterItem]
public:
	CFilterItem():m_nFilterColumn(0),IgnoredColums(0),filter(0),dlg(NULL){};

	BOOL SetFilter(size_t newfilter);
	virtual void	ReloadFileList()=0;
	CFilterDlg*		dlg;
	CAtlArray<CString>	m_astrFilter;
	CHeaderCtrl		m_ctlListHeader;
	CString			m_strEvaluatedContent;
	size_t			m_nFilterColumn;
	size_t			IgnoredColums;
	size_t			filter;
protected:
	bool IsFilteredItem(const CAbstractFile* pKnownFile) const;
	virtual void GetItemDisplayText(const CAbstractFile* file, int iSubItem, LPTSTR pszText, int cchTextMax) const=0;
};