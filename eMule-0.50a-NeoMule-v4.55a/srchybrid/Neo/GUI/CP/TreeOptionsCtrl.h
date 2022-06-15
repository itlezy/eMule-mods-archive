/* Xanatos:
* Recoded for obtimal use in Neo Mod
*/

/*
Module : TreeOptionsCtrl.h
Purpose: Defines the interface for an MFC class to implement a tree options control 
         similiar to the advanced tab as seen on the "Internet options" dialog in 
         Internet Explorer 4 and later
Created: PJN / 31-03-1999


Copyright (c) 1999 - 2003 by PJ Naughter.  (Web: www.naughter.com, Email: pjna@naughter.com)

All rights reserved.

Copyright / Usage Details:

You are allowed to include the source code in any product (commercial, shareware, freeware or otherwise) 
when your product is released in binary form. You are allowed to modify the source code in any way you want 
except you cannot modify the copyright details at the top of each module. If you want to distribute source 
code with your application, then you are only allowed to distribute versions released by the author. This is 
to maintain a single distribution point for the source code. 

*/


/////////////////////////////// Defines ///////////////////////////////////////
#ifndef __TREEOPTIONSCTRL_H__
#define __TREEOPTIONSCTRL_H__



/////////////////////////////// Includes //////////////////////////////////////

#ifndef __AFXDTCTL_H__
#pragma message("To avoid this message please put afxdtctl.h in your PCH (normally stdafx.h)")
#include <afxdtctl.h>
#endif



/////////////////////////////// Classes ///////////////////////////////////////


//forward declaration
class CTreeOptionsCtrl;
class CTreeOptionsBrowseButton;


//Class which represents a combo box used by the tree options class
class CTreeOptionsCombo : public CComboBox
{
public:
	//Constructors / Destructors
	CTreeOptionsCombo();
	virtual ~CTreeOptionsCombo();

protected:
	//Misc methods
	void SetButtonBuddy(CTreeOptionsBrowseButton* pButton) { m_pButtonCtrl = pButton; };
	void SetTreeBuddy(CTreeOptionsCtrl* pTreeCtrl) { m_pTreeCtrl = pTreeCtrl; };
	void SetTreeItem(HTREEITEM hItem) { m_hTreeCtrlItem = hItem; };
	virtual DWORD GetWindowStyle();
	virtual int GetDropDownHeight();
	BOOL IsRelatedWnd(CWnd* pChild);

	//{{AFX_VIRTUAL(CTreeOptionsCombo)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsCombo)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode ();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsCombo)

	//Member variables
	CTreeOptionsCtrl* m_pTreeCtrl;
	CTreeOptionsBrowseButton* m_pButtonCtrl;
	HTREEITEM m_hTreeCtrlItem;
	friend class CTreeOptionsCtrl;
};



//Class which represents a combo box which allows a Font Name to be specified
class CTreeOptionsFontNameCombo : public CTreeOptionsCombo
{
public:
	//Constructors / Destructors
	CTreeOptionsFontNameCombo();
	virtual ~CTreeOptionsFontNameCombo();

protected:
	//{{AFX_VIRTUAL(CTreeOptionsFontNameCombo)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsFontNameCombo)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsFontNameCombo)

	//Misc Methods
	virtual DWORD GetWindowStyle();
	int EnumFontProc(CONST LOGFONT* lplf, CONST TEXTMETRIC* lptm, DWORD dwType);
	static int CALLBACK _EnumFontProc(CONST LOGFONT* lplf, CONST TEXTMETRIC* lptm, DWORD dwType, LPARAM lpData);
};



//Class which represents a combo box which allows a True / False value to be specified
class CTreeOptionsBooleanCombo : public CTreeOptionsCombo
{
public:
	//Constructors / Destructors
	CTreeOptionsBooleanCombo();
	virtual ~CTreeOptionsBooleanCombo();

protected:
	//{{AFX_VIRTUAL(CTreeOptionsBooleanCombo)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsBooleanCombo)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsBooleanCombo)
};



//forward declaration
class CTreeOptionsBrowseButton;



//Class which represents an edit box used by the tree options class
class CTreeOptionsEdit : public CEdit
{
public:
	//Constructors / Destructors
	CTreeOptionsEdit();
	virtual ~CTreeOptionsEdit();

protected:
	//Misc methods
	void SetTreeBuddy(CTreeOptionsCtrl* pTreeCtrl) { m_pTreeCtrl = pTreeCtrl; };
	void SetButtonBuddy(CTreeOptionsBrowseButton* pButtonCtrl) { m_pButtonCtrl = pButtonCtrl; };
	void SetTreeItem(HTREEITEM hItem) { m_hTreeCtrlItem = hItem; };
	virtual DWORD GetWindowStyle();
	virtual int GetHeight(int nItemHeight);
	virtual void BrowseForFolder(const CString& sInitialFolder);
	virtual void BrowseForFile(const CString& sInitialFile);
	virtual CString GetBrowseForFolderCaption();
	virtual CString GetBrowseForFileCaption();
	virtual CString GetFileExtensionFilter();

	//{{AFX_VIRTUAL(CTreeOptionsEdit)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsEdit)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode ();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	static int CALLBACK SHBrowseSetSelProc(HWND hWnd, UINT uMsg, LPARAM /*lParam*/, LPARAM lpData);

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsEdit)

	//Member variables
	CTreeOptionsCtrl* m_pTreeCtrl;
	CTreeOptionsBrowseButton* m_pButtonCtrl;
	HTREEITEM m_hTreeCtrlItem;
	BOOL m_bDoNotDestroyUponLoseFocus;

	friend class CTreeOptionsCtrl;
	friend class CTreeOptionsBrowseButton;
};


// Note: Item Date Setting: ((((DWORD)dMin) << 16) + (DWORD)dMax)
//Class which represents the spin control which can be used in association with an edit box by the tree options class
class CTreeOptionsSpinCtrl : public CSpinButtonCtrl
{
public:
	//Constructors / Destructors
	CTreeOptionsSpinCtrl();
	virtual ~CTreeOptionsSpinCtrl();

protected:
	//Misc methods
	void SetTreeBuddy(CTreeOptionsCtrl* pTreeCtrl);
	void SetEditBuddy(CTreeOptionsEdit* pEdit);
	void SetTreeItem(HTREEITEM hItem) { m_hTreeCtrlItem = hItem; };
	virtual DWORD GetWindowStyle();
	virtual void GetDefaultRange(int &lower, int& upper);

	//{{AFX_VIRTUAL(CTreeOptionsSpinCtrl)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsSpinCtrl)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsSpinCtrl)

	//Member variables
	CTreeOptionsCtrl* m_pTreeCtrl;
	HTREEITEM m_hTreeCtrlItem;
	CTreeOptionsEdit* m_pEdit;
	friend class CTreeOptionsCtrl;
};



//Class which represents the browse button which can be used in association with an edit box by the tree options class
class CTreeOptionsBrowseButton : public CButton
{
public:
	//Constructors / Destructors
	CTreeOptionsBrowseButton();
	virtual ~CTreeOptionsBrowseButton();

protected:
	//Misc methods
	void            SetTreeBuddy(CTreeOptionsCtrl* pTreeCtrl);
	void            SetTreeItem(HTREEITEM hItem) { m_hTreeCtrlItem = hItem; };
	void            SetEditBuddy(CTreeOptionsEdit* pEdit);
	void            SetComboBuddy(CTreeOptionsCombo* pCombo);
	virtual DWORD   GetWindowStyle();
	virtual int     GetWidth();
	virtual CString GetCaption();
	COLORREF        GetColor() const { return m_Color; };
	void            SetColor(COLORREF color);
	void            GetFontItem(LOGFONT* pLogFont);
	void            SetFontItem(const LOGFONT* pLogFont);
	virtual void    BrowseForColor();
	virtual void    BrowseForFont();
	virtual void    BrowseForOpaque();

	//{{AFX_VIRTUAL(CTreeOptionsBrowseButton)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsBrowseButton)
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnClicked();
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsBrowseButton)

	//Member variables
	COLORREF m_Color;
	LOGFONT m_Font;
	CTreeOptionsCtrl* m_pTreeCtrl;
	CTreeOptionsEdit* m_pEdit;
	CTreeOptionsCombo* m_pCombo;
	HTREEITEM m_hTreeCtrlItem;
	friend class CTreeOptionsCtrl;
};



//Class which is used for browsing for filenames
class CTreeOptionsFileDialog : public CFileDialog
{
public:
	//Constructors / Destructors
	CTreeOptionsFileDialog(BOOL bOpenFileDialog, LPCTSTR lpszDefExt = NULL, LPCTSTR lpszFileName = NULL,
		DWORD dwFlags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT,	LPCTSTR lpszFilter = NULL, CWnd* pParentWnd = NULL);

protected:
	DECLARE_DYNAMIC(CTreeOptionsFileDialog)

	virtual void OnInitDone();

	//{{AFX_MSG(CTreeOptionsFileDialog)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};



//Class which represents a date / time control used by the list options class
class CTreeOptionsDateCtrl : public CDateTimeCtrl
{
public:
	//Constructors / Destructors
	CTreeOptionsDateCtrl();
	virtual ~CTreeOptionsDateCtrl();

	//Methods
	virtual CString GetDisplayText(const SYSTEMTIME& st);

protected:
	//Misc methods
	void SetTreeBuddy(CTreeOptionsCtrl* pTreeCtrl) { m_pTreeCtrl = pTreeCtrl; };
	void SetTreeItem(HTREEITEM hItem) { m_hTreeCtrlItem = hItem; };
	virtual DWORD GetWindowStyle();
	virtual BOOL IsRelatedWnd(CWnd* pChild);
	void GetDateTime(SYSTEMTIME& st) const { CopyMemory(&st, &m_SystemTime, sizeof(SYSTEMTIME)); };
	void SetDateTime(const SYSTEMTIME& st) { CopyMemory(&m_SystemTime, &st, sizeof(SYSTEMTIME)); };

	//{{AFX_VIRTUAL(CTreeOptionsDateCtrl)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsDateCtrl)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode ();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsDateCtrl)

	//Member variables
	CTreeOptionsCtrl* m_pTreeCtrl;
	HTREEITEM m_hTreeCtrlItem;
	BOOL m_bDoNotDestroyUponLoseFocus;
	friend class CTreeOptionsCtrl;
	SYSTEMTIME m_SystemTime;
};



//Class which represents a time control used by the list options class
class CTreeOptionsTimeCtrl : public CTreeOptionsDateCtrl
{
public:
	//Constructors / Destructors
	CTreeOptionsTimeCtrl();
	virtual ~CTreeOptionsTimeCtrl();

	//methods
	virtual CString GetDisplayText(const SYSTEMTIME& st);

protected:
	virtual DWORD GetWindowStyle();

	//{{AFX_VIRTUAL(CTreeOptionsTimeCtrl)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsTimeCtrl)
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

	DECLARE_DYNCREATE(CTreeOptionsTimeCtrl)
};



//Class which represents IP Address control used by the list options class
class CTreeOptionsIPAddressCtrl : public CIPAddressCtrl
{
public:
	//Constructors / Destructors
	CTreeOptionsIPAddressCtrl();
	virtual ~CTreeOptionsIPAddressCtrl();

	//methods
	virtual CString GetDisplayText(DWORD dwAddress);

protected:
	//Misc methods
	void SetTreeBuddy(CTreeOptionsCtrl* pTreeCtrl) { m_pTreeCtrl = pTreeCtrl; };
	void SetTreeItem(HTREEITEM hItem) { m_hTreeCtrlItem = hItem; };
	virtual DWORD GetWindowStyle();
	DWORD GetIPAddress() const { return m_dwAddress; };
	void SetIPAddress(DWORD dwAddress) { m_dwAddress = dwAddress; };
	virtual BOOL IsRelatedWnd(CWnd* pChild);

	//{{AFX_VIRTUAL(CTreeOptionsIPAddressCtrl)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsIPAddressCtrl)
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg UINT OnGetDlgCode ();
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsIPAddressCtrl)

	//Member variables
	CTreeOptionsCtrl* m_pTreeCtrl;
	HTREEITEM m_hTreeCtrlItem;
	BOOL m_bDoNotDestroyUponLoseFocus;
	friend class CTreeOptionsCtrl;
	DWORD m_dwAddress;
};

//Class which represents Numerical EditBox
class CNumTreeOptionsEdit : public CTreeOptionsEdit
{
public:
	CNumTreeOptionsEdit(){m_uLastChar = 0;} // NEO: FCFG - [FileConfiguration] <-- Xanatos --
	virtual ~CNumTreeOptionsEdit(){}

	virtual DWORD GetWindowStyle() { return CTreeOptionsEdit::GetWindowStyle() /* | ES_NUMBER*/; }

protected:
	bool m_bSelf;
	UINT m_uLastChar; // NEO: FCFG - [FileConfiguration] <-- Xanatos --

	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//afx_msg void OnEnChange(); 
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CNumTreeOptionsEdit)
};

//Class which represents normal Push Button
class CTreeOptionsButton : public CTreeOptionsBrowseButton
{
public:
	CTreeOptionsButton(){}
	virtual ~CTreeOptionsButton(){}

protected:
	virtual void BrowseForOpaque();

	DECLARE_MESSAGE_MAP()
	DECLARE_DYNCREATE(CTreeOptionsButton)
};



//Class which is stored in the tree options item data
class CTreeOptionsItemData
{
public:
	//Enums
	enum ControlType
	{
		Unknown,
		Normal,
		Spin,
		FileBrowser,
		FolderBrowser,
		ColorBrowser,
		FontBrowser,
		CheckBox,
		SemiCheckBox, 
		CheckBoxGroup,
		RadioButton,
		RadioButtonEX,
		ComboBox,
		EditBox,
		DateTimeCtrl,
		IPAddressCtrl,
		OpaqueBrowser,
	};

	//Data
	CRuntimeClass* m_pRuntimeClass1;
	CRuntimeClass* m_pRuntimeClass2;
	ControlType    m_Type;
	ControlType    m_Type2;
	DWORD          m_dwItemData;
	DWORD          m_dwItemData2;
	COLORREF       m_Color;
	LOGFONT        m_Font;
	SYSTEMTIME     m_DateTime;
	DWORD          m_dwIPAddress;
	BOOL           m_bDrawColorForIcon;

	BOOL		   m_bEnabled;
	BOOL		   m_bEnabled2;
	int			   m_nImageEnabled;
	int			   m_nImageDisabled;

	int			   m_nItemLines;
	int			   m_nItemColumns;
	int			   m_nItemLength;

	CString        m_sInfo;

	//Methods
	CTreeOptionsItemData() 
	{
		m_Type = Unknown;
		m_Type2 = Unknown;
		m_pRuntimeClass1 = NULL;
		m_pRuntimeClass2 = NULL;
		m_dwItemData = NULL;
	    m_dwItemData2 = NULL;
		m_Color = RGB(255, 0, 0);
		ZeroMemory(&m_Font, sizeof(LOGFONT));
		ZeroMemory(&m_DateTime, sizeof(SYSTEMTIME));
		m_dwIPAddress = 0;
		m_bDrawColorForIcon = TRUE;

		m_bEnabled = TRUE;
		m_bEnabled2 = TRUE;
		m_nImageEnabled = 15; // Empty
		m_nImageDisabled = 15; // Empty

		m_nItemLines = 1;
		m_nItemColumns = 0;
		m_nItemLength = 0;

		m_sInfo.Empty();
	}
};

// pre defined treeview image list indices
#define	TREEOPTSCTRLIMG_EDIT	11
#define TREEOPTSCTRLIMG_EMPTY	15 

#define	WM_TREEOPTSCTRL_NOTIFY		(WM_USER + 0x101 + 1)
#define WM_TREEITEM_HELP			(WM_USER + 0x102 + 1)

typedef struct
{
	NMHDR nmhdr;
	HTREEITEM hItem;
} TREEOPTSCTRLNOTIFY;

//The actual tree options control class
class CTreeOptionsCtrl : public CTreeCtrl
{
public:
	//Constructors / Destructors
	CTreeOptionsCtrl(UINT uImageListColorFlags = ILC_COLOR);
	virtual ~CTreeOptionsCtrl();
	void	SetImageListColorFlags(UINT uImageListColorFlags){m_uImageListColorFlags = uImageListColorFlags; }

	//Misc
	void    SetAutoSelect(BOOL bAutoSelect) { m_bAutoSelect = bAutoSelect; };
	BOOL    GetAutoSelect() const { return m_bAutoSelect; };
	void    SetImageListResourceIDToUse(UINT nResourceID) { m_nilID = nResourceID; };
	UINT    GetImageListResourceIDToUse() const { return m_nilID; };
	void    SetToggleOverIconOnly(BOOL bToggle) { m_bToggleOverIconOnly = bToggle; };
	BOOL    GetToggleOverIconOnly() const { return m_bToggleOverIconOnly; };
	DWORD   GetUserItemData(HTREEITEM hItem) const;
	BOOL    SetUserItemData(HTREEITEM hItem, DWORD dwData);
	void    SetTextSeparator(const CString& sSeparator) { m_sSeparator = sSeparator; };
	CString GetTextSeparator() const { return m_sSeparator; };
	void    SetEditLabel(HTREEITEM hItem, const CString& rstrLabel);
	void    SetNeoStyle(BOOL NeoStyle = TRUE) { m_bNeoStyle = NeoStyle; }
	int		GetHeight(HTREEITEM hItem, int nItemHeight) const;
	int		GetEditLines(HTREEITEM hItem) const;
	int		GetEditColumns(HTREEITEM hItem) const;
	int		GetEditLimitText(HTREEITEM hItem) const;
	void	SetItemSize(HTREEITEM hItem, int columns = 0, int lines = 0, int length = 0);
	void    SetItemInfo(HTREEITEM hItem, LPCTSTR sInfo);
	void    Clear();
	virtual BOOL DeleteAllItems();

	//Inserting items into the control
	HTREEITEM InsertGroup(LPCTSTR lpszItem, int nImage, HTREEITEM hParent = TVI_ROOT, HTREEITEM hAfter = TVI_LAST, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);
	HTREEITEM InsertCheckBox(LPCTSTR lpszItem, HTREEITEM hParent, BOOL bCheck = TRUE, UINT cType = 0, HTREEITEM hAfter = TVI_LAST, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL); 
	HTREEITEM InsertRadioButton(LPCTSTR lpszItem, HTREEITEM hParent, BOOL bCheck = TRUE, HTREEITEM hAfter = TVI_LAST, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL, BOOL UnSetable = FALSE);
	HTREEITEM InsertButton(LPCTSTR lpszItem, HTREEITEM hParent, void(*fx)()/*, LPCSTR sText = NULL*/, HTREEITEM hAfter = TVI_LAST); 
	BOOL AddButton(HTREEITEM hItem, CRuntimeClass* pRuntimeClassButton, void(*fx)()/*, LPCSTR sText*/); 

	//Validation methods
	BOOL IsGroup(HTREEITEM hItem) const;
	BOOL IsCheckBox(HTREEITEM hItem) const;
	//BOOL IsNormalCheckBox(HTREEITEM hItem) const;
	BOOL IsSemiCheckBox(HTREEITEM hItem) const;
	BOOL IsCheckBoxGroup(HTREEITEM hItem) const;
	BOOL IsRadioButton(HTREEITEM hItem) const;
	BOOL IsRadioButtonEx(HTREEITEM hItem) const;
	BOOL IsEditBox(HTREEITEM hItem) const;
	BOOL IsFileItem(HTREEITEM hItem) const;
	BOOL IsFolderItem(HTREEITEM hItem) const;
	BOOL IsColorItem(HTREEITEM hItem) const;
	BOOL IsFontItem(HTREEITEM hItem) const;
	BOOL IsDateTimeItem(HTREEITEM hItem) const;
	BOOL IsIPAddressItem(HTREEITEM hItem) const;
	BOOL IsOpaqueItem(HTREEITEM hItem) const;

	//Setting / Getting combo states
	BOOL SetCheckBox(HTREEITEM hItem, UINT bCheck);
	BOOL GetCheckBox(HTREEITEM hItem, UINT& bCheck) const;

	//Setting / Getting radio states
	virtual BOOL SetRadioButton(HTREEITEM hParent, int nIndex);
	virtual BOOL SetRadioButton(HTREEITEM hItem);
	BOOL GetRadioButton(HTREEITEM hParent, int& nIndex, HTREEITEM& hCheckItem) const;
	BOOL GetRadioButton(HTREEITEM hItem, BOOL& bCheck) const;

	//Enable / Disbale items
	BOOL SetGroupEnable(HTREEITEM hItem, BOOL bEnable);
	BOOL SetCheckBoxEnable(HTREEITEM hItem, BOOL bEnable) { SetItemEnable(hItem, bEnable); return TRUE; }
	BOOL SetRadioButtonEnable(HTREEITEM hItem, BOOL bEnable) { SetItemEnable(hItem, bEnable); return TRUE; }
	BOOL GetRadioButtonEnable(HTREEITEM hItem, BOOL& bEnable) const { bEnable = IsItemEnabled(hItem); return TRUE; }
	BOOL GetCheckBoxEnable(HTREEITEM hItem, BOOL& bEnable) const { bEnable = IsItemEnabled(hItem); return TRUE; }

	BOOL SetItemIcons(HTREEITEM hItem, int nImageEnabled, int nImageDisabled);
	BOOL IsItemEnabled(HTREEITEM hItem, BOOL bSecund = FALSE) const;
	BOOL SetItemEnable(HTREEITEM hItem, BOOL bEnable, BOOL bChilds = TRUE, BOOL bSecund = FALSE, BOOL bImgIgnore = TRUE);


	//Adding a combo box to an item
	BOOL    AddComboBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClass, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);
	CString GetComboText(HTREEITEM hItem) const;
	void    SetComboText(HTREEITEM hItem, const CString& sComboText);

	//Adding an edit box (and a spin control) to an item
	BOOL    AddEditBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClassEditCtrl, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);
	BOOL    AddEditBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClassEditCtrl, CRuntimeClass* pRuntimeClassSpinCtrl, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);
	CString GetEditText(HTREEITEM hItem) const;
	void    SetEditText(HTREEITEM hItem, const CString& sEditText);

	//Adding a file / Folder edit box (and a browse button) to an item
	BOOL    AddFileEditBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClassEditCtrl, CRuntimeClass* pRuntimeClassButton, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);
	CString GetFileEditText(HTREEITEM hItem) const;
	void    SetFileEditText(HTREEITEM hItem, const CString& sEditText);
	BOOL    AddFolderEditBox(HTREEITEM hItem, CRuntimeClass* pRuntimeClassEditCtrl, CRuntimeClass* pRuntimeClassButton, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);
	CString GetFolderEditText(HTREEITEM hItem) const;
	void    SetFolderEditText(HTREEITEM hItem, const CString& sEditText);

	//Adding a Color selector to an item
	BOOL     AddColorSelector(HTREEITEM hItem, CRuntimeClass* pRuntimeClassButton, DWORD dwItemData = NULL, BOOL bDrawColorForIcon = TRUE, DWORD dwItemData2 = NULL);  
	COLORREF GetColor(HTREEITEM hItem) const;
	void     SetColor(HTREEITEM hItem, COLORREF color);

	//Adding a font name selector to an item
	BOOL     AddFontSelector(HTREEITEM hItem, CRuntimeClass* pRuntimeClassButton, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);  
	void     GetFontItem(HTREEITEM hItem, LOGFONT* pLogFont) const;
	void     SetFontItem(HTREEITEM hItem, const LOGFONT* pLogFont);

	//Adding a Date Time  selector to an item
	BOOL     AddDateTime(HTREEITEM hItem, CRuntimeClass* pRuntimeClassDateTime, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);  
	void     GetDateTime(HTREEITEM hItem, SYSTEMTIME& st) const;
	void     SetDateTime(HTREEITEM hItem, const SYSTEMTIME& st);

	//Adding an IP Address selector to an item
	BOOL     AddIPAddress(HTREEITEM hItem, CRuntimeClass* pRuntimeClassIPAddress, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);  
	DWORD    GetIPAddress(HTREEITEM hItem) const;
	void     SetIPAddress(HTREEITEM hItem, DWORD dwAddress);

	//Adding a Opaque selector to an item
	BOOL     AddOpaque(HTREEITEM hItem, CRuntimeClass* pRuntimeClass1, CRuntimeClass* pRuntimeClass2, DWORD dwItemData = NULL, DWORD dwItemData2 = NULL);  
	DWORD    GetOpaque(HTREEITEM hItem, BOOL bSecund = FALSE) const;
	void     SetOpaque(HTREEITEM hItem, DWORD dwItemData, DWORD dwItemData2 = NULL);

	//Virtual methods    
	virtual void OnCreateImageList();
	virtual HTREEITEM CopyItem(HTREEITEM hItem, HTREEITEM htiNewParent, HTREEITEM htiAfter = TVI_LAST);
	virtual HTREEITEM CopyBranch(HTREEITEM htiBranch, HTREEITEM htiNewParent, HTREEITEM htiAfter = TVI_LAST);

	virtual void HandleChildControlLosingFocus();
	void UpdateCheckBoxGroup(HTREEITEM hItem);

protected:
	//Variables
	CImageList                 m_ilTree;
	UINT                       m_nilID;
	CTreeOptionsCombo*         m_pCombo;
	CTreeOptionsEdit*          m_pEdit;
	CTreeOptionsSpinCtrl*      m_pSpin;
	CTreeOptionsBrowseButton*  m_pButton;
	CTreeOptionsDateCtrl*      m_pDateTime;
	CTreeOptionsIPAddressCtrl* m_pIPAddress;
	HTREEITEM                  m_hControlItem;
	HTREEITEM                  m_hLastControlItem;
	BOOL                       m_bToggleOverIconOnly;
	BOOL                       m_bAutoSelect;
	CFont                      m_Font;
	CString                    m_sSeparator;
	BOOL                       m_bBeingCleared;
	UINT                       m_uImageListColorFlags;
	BOOL                       m_bNeoStyle; 

	//Methods
	virtual void NotifyParent(UINT uCode, HTREEITEM hItem); 
	virtual void SendHelpMessage(HTREEITEM hItem);
	virtual void DestroyOldChildControl();
	virtual void RemoveChildControlText(HTREEITEM hItem);
	virtual void CreateNewChildControl(HTREEITEM hItem);
	virtual void CreateSpinCtrl(CRuntimeClass* pRuntimeClassSpinCtrl, CRect rItem, CRect rText, CRect rPrimaryControl);
	virtual void CreateBrowseButton(CRuntimeClass* pRuntimeClassBrowseButton, CRect rItem, CRect rText);
	virtual void UpdateTreeControlValueFromChildControl(HTREEITEM hItem);
	virtual void HandleCheckBox(HTREEITEM hItem); 
	virtual void HandleSecundItem(HTREEITEM hItem); 
	//virtual BOOL SetEnabledSemiCheckBox(HTREEITEM hItem, BOOL bSemi);
	//virtual BOOL GetSemiCheckBox(HTREEITEM hItem, BOOL& bSemi) const;
	virtual int  GetIndentPostion(HTREEITEM hItem) const;
	virtual void MemDeleteAllItems(HTREEITEM hParent);
	void		 SetLastSelectedItem(HTREEITEM item) {m_hLastControlItem = item;};
	HTREEITEM    GetLastSelectedItem() {return m_hLastControlItem;}

	//{{AFX_VIRTUAL(CTreeOptionsCtrl)
	virtual void PreSubclassWindow();
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CTreeOptionsCtrl)
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnDestroy();
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnKeyDown( UINT nChar, UINT nRepCnt, UINT nFlags );
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnKeyUp(UINT nChar, UINT nRepCnt, UINT nFlags);
	//}}AFX_MSG
	afx_msg void OnEditChange(); 
	afx_msg void OnComboChange(); 
	afx_msg BOOL OnClick(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnSelchanged(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnDeleteItem(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg LRESULT OnSetFocusToChild(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRepositionChild(WPARAM wParam, LPARAM lParam);
	afx_msg BOOL OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg BOOL OnItemExpanding(NMHDR* pNMHDR, LRESULT* pResult);

	DECLARE_DYNAMIC(CTreeOptionsCtrl)

	DECLARE_MESSAGE_MAP()

	friend class CTreeOptionsEdit;
	friend class CTreeOptionsStatic;
	friend class CTreeOptionsCombo;
	friend class CTreeOptionsSpinCtrl;
	friend class CTreeOptionsBrowseButton;
	friend class CTreeOptionsDateCtrl;
	friend class CTreeOptionsIPAddressCtrl;
};

//Dialog Data exchange support
void DDX_TreeCheck(CDataExchange* pDX, int nIDC, HTREEITEM hItem, bool& bCheck);
void DDX_TreeCheck(CDataExchange* pDX, int nIDC, HTREEITEM hItem, UINT& bCheck); 
void DDX_TreeRadio(CDataExchange* pDX, int nIDC, HTREEITEM hParent, int& nIndex);
void DDX_TreeEdit(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);
void DDX_TreeEdit(CDataExchange* pDX, int nIDC, HTREEITEM hItem, int& nValue);
void DDX_TreeEdit(CDataExchange* pDX, int nIDC, HTREEITEM hItem, float& nValue);
void DDX_TreeCombo(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);
void DDX_TreeFileEdit(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);
void DDX_TreeFolderEdit(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);
void DDX_TreeColor(CDataExchange* pDX, int nIDC, HTREEITEM hItem, COLORREF& color);
void DDX_TreeFont(CDataExchange* pDX, int nIDC, HTREEITEM hItem, LOGFONT* pLogFont);
void DDX_TreeBoolean(CDataExchange* pDX, int nIDC, HTREEITEM hItem, BOOL& bValue);
void DDX_TreeDateTime(CDataExchange* pDX, int nIDC, HTREEITEM hItem, SYSTEMTIME& st);
void DDX_TreeIPAddress(CDataExchange* pDX, int nIDC, HTREEITEM hItem, DWORD& dwAddress);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, CString& sText);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, int& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, UINT& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, long& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, DWORD& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, float& value);
void DDX_Text(CDataExchange* pDX, int nIDC, HTREEITEM hItem, double& value);

#define BLANK_INFO _T(" ") // Blank Help Text

#endif //__TREEOPTIONSCTRL_H__



