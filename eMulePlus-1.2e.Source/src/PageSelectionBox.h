#pragma once


// CPageSelectionBox dialog

class CPageSelectionBox : public CDialog
{
	DECLARE_DYNAMIC(CPageSelectionBox)

public:
	CPageSelectionBox(CWnd* pParent = NULL);   // standard constructor
	virtual ~CPageSelectionBox();

// Dialog Data
	enum { IDD = IDD_EMULE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
