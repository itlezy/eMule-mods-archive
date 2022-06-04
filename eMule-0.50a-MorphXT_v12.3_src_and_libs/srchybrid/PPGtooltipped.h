#pragma once

#include "preferences.h"
#include "TreeOptionsCtrlEx.h"
#include "ToolTips\PPToolTip.h"// [TPT] - Tooltips in preferences


class CPPgtooltipped :public CPropertyPage
{
	////////////////////////////////////////////////////////////////////////////
// CPropertyPage -- one page of a tabbed dialog

	//DECLARE_DYNAMIC(CPPgtooltipped )

// Construction
public:
	// simple construction
	 CPPgtooltipped (UINT nIDTemplate);

  	virtual BOOL PreTranslateMessage(MSG* pMsg);// [TPT] - Tooltips in preferences
	virtual void InitTooltips(CTreeOptionsCtrlEx * tree = NULL);
	void SetTool(int ControlID, int RCStringID);
	void SetTool(HTREEITEM TreeItem, int RCString);
// Overridables
public:
  
// Implementation
public:
	virtual ~CPPgtooltipped();

protected:
	// private implementation data
	CPPToolTip m_Tip;
	CTreeOptionsCtrlEx *pm_tree;
	//DECLARE_MESSAGE_MAP()

};


class CPPgtooltippedDialog :public CDialog
{
public:
	// simple construction
	 CPPgtooltippedDialog (UINT nIDTemplate);

  	virtual BOOL PreTranslateMessage(MSG* pMsg);// [TPT] - Tooltips in preferences
	virtual void InitTooltips();
	void SetTool(int ControlID, int RCStringID);
// Implementation
public:
	virtual ~CPPgtooltippedDialog ();
protected:
	// private implementation data
	CPPToolTip m_Tip;
    CTreeOptionsCtrlEx *pm_tree; // not really  needed But makes it simpler

};



