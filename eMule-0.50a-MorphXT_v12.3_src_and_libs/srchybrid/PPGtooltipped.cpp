//this file is part of eMule Morph
//Copyright (C)2006 leuk_he (leukhe  at gmail dot com)
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

#include <stdafx.h>
#include "preferences.h"
#include "ToolTips\PPToolTip.h"// [TPT] - Tooltips in preferences
#include "PPGtooltipped.h"
#include "OtherFunctions.h"
#include "emule.h"

//IMPLEMENT_DYNAMIC(CPPgtooltipped , CPropertyPage)

/* This a subclass of CpropertyPage to encapsulate the TOoltip behaviour of the preferecnes window */

CPPgtooltipped ::CPPgtooltipped (UINT nIDTemplate) :CPropertyPage(nIDTemplate)
{
   pm_tree=NULL;
}


BOOL
CPPgtooltipped::PreTranslateMessage(MSG* pMsg)// [TPT] - Tooltips in preferences
{
	m_Tip.RelayEvent(pMsg);

	return CPropertyPage::PreTranslateMessage(pMsg);
}

	
CPPgtooltipped::~CPPgtooltipped(){};

void 
CPPgtooltipped::InitTooltips(CTreeOptionsCtrlEx * tree /* = NULL*/)
{   pm_tree=tree;
    if (tree)
       m_Tip.Create(tree);
	else 
		  m_Tip.Create(this);
    COLORREF grad1=RGB(255,255,25); // YELLOW
    COLORREF grad2=RGB(155,155,120); // NOT USED IN HGRADIENT....
	COLORREF grad3=RGB(160,160,200); // LIGHT BLUE
    m_Tip.SetEffectBk(CPPToolTip::PPTOOLTIP_EFFECT_HGRADIENT);
    theApp.LoadSkinColor(_T("Tooltipgrad1"), grad1);
    theApp.LoadSkinColor(_T("Tooltipgrad2"), grad2);
    theApp.LoadSkinColor(_T("Tooltipgrad3"), grad3);
   	m_Tip.SetGradientColors(grad1,grad2, grad3);
	m_Tip.SetDelayTime(TTDT_INITIAL,thePrefs.GetToolTipDelay()*1000); /* show after 1 second default */
	m_Tip.SetDelayTime(TTDT_AUTOPOP, 15000); /* show for 15 seconds */


	//m_Tip.SetGradientColors(RGB(255,255,225),RGB(0,0,0), RGB(255,198,167));
	
}
//virtual void 
//CPPgtooltipped::LocalizeTooltips();
void CPPgtooltipped::SetTool(int ControlID, int RCStringID)
{  
	if (thePrefs.GetToolTipDelay() > 0) //disable if 0
	   m_Tip.AddTool(GetDlgItem(ControlID), GetResString(RCStringID));
};


void CPPgtooltipped::SetTool(HTREEITEM TreeItem, int RCStringID)
{  
	if (thePrefs.GetToolTipDelay() > 0) { 
	  int toolid;
	  toolid=m_Tip.AddTool(pm_tree, RCStringID);
	  if (pm_tree) 
	    {pm_tree->SetUserItemData(TreeItem,toolid);
	    }
	}
};


CPPgtooltippedDialog ::CPPgtooltippedDialog (UINT nIDTemplate) :CDialog(nIDTemplate)
{
   pm_tree=NULL;
}


BOOL
CPPgtooltippedDialog ::PreTranslateMessage(MSG* pMsg)// [TPT] - Tooltips in preferences
{
	m_Tip.RelayEvent(pMsg);

	return CDialog::PreTranslateMessage(pMsg);
}

	
CPPgtooltippedDialog::~CPPgtooltippedDialog(){};

void 
CPPgtooltippedDialog::InitTooltips(){
    m_Tip.Create(this);
    COLORREF grad1=RGB(255,255,25); // YELLOW
    COLORREF grad2=RGB(155,155,120); // NOT USED IN HGRADIENT....
	COLORREF grad3=RGB(160,160,200); // LIGHT BLUE
    m_Tip.SetEffectBk(CPPToolTip::PPTOOLTIP_EFFECT_HGRADIENT);
    theApp.LoadSkinColor(_T("Tooltipgrad1"), grad1);
    theApp.LoadSkinColor(_T("Tooltipgrad2"), grad2);
    theApp.LoadSkinColor(_T("Tooltipgrad3"), grad3);
   	m_Tip.SetGradientColors(grad1,grad2, grad3);
	m_Tip.SetDelayTime(TTDT_INITIAL,thePrefs.GetToolTipDelay()*1000); /* show after 1 second default */
	m_Tip.SetDelayTime(TTDT_AUTOPOP, 15000); /* show for 15 seconds */
}

void CPPgtooltippedDialog::SetTool(int ControlID, int RCStringID)
{  
	if (thePrefs.GetToolTipDelay() > 0) //disable if 0
	   m_Tip.AddTool(GetDlgItem(ControlID), GetResString(RCStringID));
};

