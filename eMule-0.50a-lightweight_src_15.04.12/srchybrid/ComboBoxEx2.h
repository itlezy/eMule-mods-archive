#pragma once

class CComboBoxEx2 : public CComboBoxEx
{
	DECLARE_DYNAMIC(CComboBoxEx2)
public:
	CComboBoxEx2();
	virtual ~CComboBoxEx2();

	int AddItem(LPCTSTR pszText);
	BOOL SelectString(LPCTSTR pszText);
	BOOL SelectItemDataStringA(LPCSTR pszText);

protected:
	DECLARE_MESSAGE_MAP()
};

void UpdateHorzExtent(CComboBox &rctlComboBox, int iIconWidth);
