#pragma once


// CComboBoxEnter

class CComboBoxEnter : public CComboBox
{
	DECLARE_DYNAMIC(CComboBoxEnter)

public:
	CComboBoxEnter();
	virtual ~CComboBoxEnter();

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


