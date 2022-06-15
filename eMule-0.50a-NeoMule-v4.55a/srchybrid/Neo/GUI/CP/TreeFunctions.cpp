//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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
#include "OtherFunctions.h"
#include "Neo\GUI\CP\TreeOptionsCtrl.h" // NEO - [TreeControl] <-- Xanatos --
#include "TreeFunctions.h"
#include "Neo/FilePreferences.h"
#include "Neo/Functions.h"
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl] -- Xanatos -->
#include "Neo/BC/BandwidthControl.h"
#endif // NEO_BC // NEO: NBC END <-- Xanatos --


/******************************************************
* Lets summarise what this smart functions can do:
* 
*	First for the Radio buttons, thay know whitch values are valid and whitch are to be interpreted as "use default" and "use global"
* 
*	And now the main thing, the Smart Edit Boxes what thay can do:
*		Thay can interpret Sec, Min, Hr, Day and also Kilo, Mega and thay use them in the output for a easyer use
*		Thay can interpret def, glb as value to "use default" or "use global", 
*		Thay can apply Min/Max limits on the fly 
*		Thay can interpret def, std, glb input as enter default/standard/global value
*		Thay don't change the value if the edit box is empty
*		Thay also can handle multiple subsets of min/std/max values when the content interpretation depands on som other setting
*		Thay deny input of propobly wrong data, for example if a setting is specyfyed without having first specyfy the setting for proper interpretation
*		And so on... thay are realy smart you know ;) 
*
*		Note: Some featues are only implemented in the int version of the edit boxes
* 
*/


///////////////////////////////////////////////////////
// Insert objects into the prefs tree
//

void SetTreeGroup(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, int Icon, HTREEITEM Parent, LPCTSTR Info){
	hItem = hCtrl.InsertGroup(Label,Icon, Parent); 
	hCtrl.SetItemInfo(hItem, Info ? Info : BLANK_INFO);
}

void SetTreeEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info){
	hItem = hCtrl.InsertItem(Label, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, Parent);
	hCtrl.AddEditBox(hItem,RUNTIME_CLASS(CTreeOptionsEdit));
	hCtrl.SetItemInfo(hItem,Info);
}

void SetTreeNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info){
	hItem = hCtrl.InsertItem(Label, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, Parent);
	hCtrl.AddEditBox(hItem,RUNTIME_CLASS(CNumTreeOptionsEdit));
	hCtrl.SetItemInfo(hItem,Info);
}

void SetTreeCheckEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info, BOOL TreeState, BOOL Value){
	hItem = hCtrl.InsertCheckBox(Label,Parent,Value,TreeState?1:0);
	hCtrl.AddEditBox(hItem,RUNTIME_CLASS(CTreeOptionsEdit));
	hCtrl.SetItemInfo(hItem,Info);
}

void SetTreeCheckNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info, BOOL TreeState, BOOL Value){
	hItem = hCtrl.InsertCheckBox(Label,Parent,Value,TreeState?1:0);
	hCtrl.AddEditBox(hItem,RUNTIME_CLASS(CNumTreeOptionsEdit));
	hCtrl.SetItemInfo(hItem,Info);
}

void SetTreeRadioNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info, BOOL UnSetable, BOOL Value){
	hItem = hCtrl.InsertRadioButton(Label, Parent, Value, TVI_LAST, NULL, NULL, UnSetable);
	hCtrl.AddEditBox(hItem,RUNTIME_CLASS(CNumTreeOptionsEdit));
	hCtrl.SetItemInfo(hItem,Info);
}


void SetTreeCheck(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info, BOOL TreeState, BOOL Value){
	hItem = hCtrl.InsertCheckBox(Label,Parent,Value,TreeState?1:0);
	hCtrl.SetItemInfo(hItem,Info);
}

void SetTreeRadio(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent,  LPCTSTR Info, BOOL UnSetable, BOOL Value){
	hItem = hCtrl.InsertRadioButton(Label, Parent, Value, TVI_LAST, NULL, NULL, UnSetable);
	hCtrl.SetItemInfo(hItem,Info);
}

/*void SetTreeCheckSpinNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info, int Min, int Max, BOOL TreeState, BOOL Value){
	hItem = hCtrl.InsertCheckBox(Label,Parent,Value,TreeState?1:0);
	hCtrl.AddEditBox(hItem,RUNTIME_CLASS(CNumTreeOptionsEdit),RUNTIME_CLASS(CTreeOptionsSpinCtrl),((((DWORD)Min) << 16) + (DWORD)Max));
	hCtrl.SetItemInfo(hItem,Info);
}*/

CString MkRdBtnLbl(CString label, SrbtC& Def, int nIndex)
{
	if(nIndex == Def.Def){
		if(Def.Cat == -1) // this is a normal default as no cat setting is present
			label.Append( _T(" (def)"));
		else // this is a global setting as we have a cat one too
			label.Append( _T(" (glb)"));
	}if(nIndex == Def.Cat) // notmal cat setting present
		label.Append( _T(" (cat)"));
	return label;
}

////////////////////////////////////////////////////////////////////7
// Get Set tree item values
//

void DDX_TreeRadioEx(CDataExchange* pDX, int nIDC, HTREEITEM hParent, SrbtC& nIndex)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	CTreeOptionsCtrl* pCtrlTreeOptions = (CTreeOptionsCtrl*) CWnd::FromHandlePermanent(hWndCtrl);
	ASSERT(pCtrlTreeOptions);
	ASSERT(pCtrlTreeOptions->IsKindOf(RUNTIME_CLASS(CTreeOptionsCtrl)));

	int Index;
	if (pDX->m_bSaveAndValidate)
	{
		HTREEITEM hCheckItem;
		VERIFY( pCtrlTreeOptions->GetRadioButton(hParent, Index, hCheckItem) );

		if(Index == -1)
			nIndex.Val = FCFG_UNK;
		else if(Index == nIndex.Cut + 1)
			nIndex.Val = FCFG_DEF;
		else if(Index == nIndex.Cut + 2)
			nIndex.Val = FCFG_GLB;
		else
			nIndex.Val = Index;
	}
	else
	{
		if(nIndex.Val == FCFG_UNK)
			Index = -1;
		else if(nIndex.Val == FCFG_DEF)
			Index = nIndex.Cut + 1;
		else if(nIndex.Val == FCFG_GLB)
			Index = nIndex.Cut + 2;
		else
			Index = nIndex.Val;

		VERIFY( pCtrlTreeOptions->SetRadioButton(hParent, Index) );
	}
}

////////////////////////////////////
// Helpers for the smart edit box
//

int GetRightDef(SintD& nValue)
{
	switch(nValue.Def){
		case FCFG_DEF:
		case FCFG_GLB:
			return nValue.Glb;
		case FCFG_AUT:
			return nValue.Aut[nValue.SubSet()];
		case FCFG_STD:
			return nValue.Std[nValue.SubSet()];
		case FCFG_UNK:
		default:
			return nValue.Def;
	}
}

void ParseFormatedNumber(SintD& nValue, CString& sText)
{
	sText.Replace(_T(" (std)"), _T("")); // just in case

	switch(nValue.Format[nValue.SubSet()]){
		case 't':{
			int multi = 0;
			int index;
			index = sText.Find(_T("m"));
			if(index > 0){
				multi = 60;
			}else{
				index = sText.Find(_T("s"));
				if(index > 0){
					multi = 1;
				}else{
					index = sText.Find(_T("h"));
					if(index > 0){
						multi = 60*60;
					}else{
						index = sText.Find(_T("d"));
						if(index > 0){
							multi = 60*60*24;
						}else{
							index = sText.GetLength()+1;
							// choice default multiplyed
							//int def = GetRightDef(nValue);
							int def = nValue.Val;
							if(def == FCFG_AUT || def == FCFG_UNK )
								multi = 1;
							else if(def > 60*60*24)
								multi = 60*60*24;
							else if(def > 60*60)
								multi = 60*60;
							else if(def > 60)
								multi = 60;
							else
								multi = 1;
						}
					}
				}
			}
			int value = _ttoi(sText.Left(index));
			if(value == 0 && sText != _T("0"))
				break;
			nValue.Val = value*multi;
			break;
		}
		case 'n':{
			int multi = 0;
			int index;
			index = sText.Find(_T("."));
			if(index > 0){
				multi = 1;
			}else{
				index = sText.Find(_T("k"));
				if(index > 0){
					multi = 1000;
				}else{
					index = sText.Find(_T("m"));
					if(index > 0){
						multi = 1000*1000;
					}else{
						index = sText.GetLength()+1;
						// choice default multiplyed
						//int def = GetRightDef(nValue);
						int def = nValue.Val;
						if(def == FCFG_AUT || def == FCFG_UNK )
							multi = 1;
						else if(def > 1000*1000)
							multi = 1000;
						else if(def > 1000)
							multi = 1000;
						else
							multi = 1;
					}
				}
			}
			int value = _ttoi(sText.Left(index));
			if(value == 0 && sText != _T("0"))
				break;
			nValue.Val = value*multi;
			break;
		}
		case 'f':{
			nValue.Val = (int)(_wtof(sText)*100.F);
			break;
		}
		default:
			nValue.Val = _ttoi(sText);
	}
}

CString FormatNumber(SintD& nValue, int value, CString Addon = _T(""))
{
	if(value == 0)
		return _T("0");
	// Note: FCFG_AUT is the only one special seting that is also valid for the base there for it have a different handling
	if(value == FCFG_AUT)
	{
		CString sText;
		if(nValue.Aut[nValue.SubSet()] == FCFG_UNK || !nValue.Aut[nValue.SubSet()])
			sText.Format(_T("??? (aut)")); 
		else
			sText = FormatNumber(nValue,nValue.Aut[nValue.SubSet()],_T(" (aut)"));

		sText.Append(Addon);
		return sText;	
	}

	CString sText;
	switch(nValue.Format[nValue.SubSet()]){
		case 't':{
			int divider;
			if(value % (60*60*24) == 0)
				divider = 60*60*24;
			else if(value % (60*60) == 0)
				divider = 60*60;
			else if(value % (60) == 0)
				divider = 60;
			else
				divider = 1;

			sText.Format(_T("%i"),value/divider); 

			if(divider == 60*60*24)
				sText.Append(_T(" Day"));
			else if(divider == 60*60)
				sText.Append(_T(" Hr"));
			else if(divider == 60)
				sText.Append(_T(" Min"));
			else
				sText.Append(_T(" Sec"));
			break;
		}
		case 'n':{
			int divider;
			if(value % (1000*1000) == 0)
				divider = 1000*1000;
			else if(value % (1000) == 0)
				divider = 1000;
			else
				divider = 1;

			sText.Format(_T("%i"),value/divider); 

			if(divider == 1000*1000)
				sText.Append(_T("M"));
			else if(divider == 1000)
				sText.Append(_T("k"));
			break;
		}
		case 'f':{
			sText.Format(_T("%.2f"),(float)value/100.F); 
			break;
		}
		default:
			sText.Format(_T("%i"),value); 
	}
	sText.Append(Addon);
	return sText;
}

CString GetPreSetStr(SintD& nValue)
{
	CString sText;
	if(nValue.PreSet() == 1) // default
	{
		if(nValue.Def == FCFG_DEF) // we dont have category specyfic tweaks (its global)
			sText = FormatNumber(nValue,nValue.Glb,_T(" (glb def)"));
		else if(nValue.Def == FCFG_UNK) // we have files from different categories
			sText.Format(_T("??? (def)"));
		else
			sText = FormatNumber(nValue,GetRightDef(nValue),_T(" (def)"));
	}
	else if(nValue.PreSet() == 2) // global
		sText = FormatNumber(nValue,nValue.Glb,_T(" (glb)"));
	return sText;
}

////////////////////////////////////
// get set smart editbox values
//

// Note: When we are in this function we assume that the entered datas are corrected by CheckTreeEditLimit
//			in case thay arn't the prefs.Check function will corrent them anyway
void DDX_TreeEditEx(CDataExchange* pDX, int nIDC, HTREEITEM hItem, SintD& nValue)
{
	HWND hWndCtrl = pDX->PrepareCtrl(nIDC);
	CTreeOptionsCtrl* pCtrlTreeOptions = (CTreeOptionsCtrl*) CWnd::FromHandlePermanent(hWndCtrl);
	ASSERT(pCtrlTreeOptions);
	ASSERT(pCtrlTreeOptions->IsKindOf(RUNTIME_CLASS(CTreeOptionsCtrl)));

	CString sText;
	if (pDX->m_bSaveAndValidate)
	{
		sText = pCtrlTreeOptions->GetEditText(hItem);
		sText.MakeLower();
		if(!nValue.Base)
		{
			if(sText.Find(_T("def")) != -1)
				nValue.Val = FCFG_DEF;
			else if(sText.Find(_T("glb")) != -1)
				nValue.Val = FCFG_GLB;
			else if(sText.Find(_T("aut")) != -1 && nValue.Aut[nValue.SubSet()]) // Note: the check order is nessesery becouse we ma have "xxx (aut) (def)"
				nValue.Val = FCFG_AUT;
			else if(sText.IsEmpty())
				nValue.Val = FCFG_UNK;
			else
				ParseFormatedNumber(nValue,sText);
		}
		else
		{
			if(sText.Find(_T("aut")) != -1 && nValue.Aut[nValue.SubSet()])
				nValue.Val = FCFG_AUT;
			else if(!sText.IsEmpty())
				ParseFormatedNumber(nValue,sText);
		}
	}
	else
	{
		if(!nValue.Base)
		{
			if(nValue.SubSet(true) == 0xFF)
			{
				pCtrlTreeOptions->SetItemEnable(hItem,FALSE,FALSE);
				sText = GetPreSetStr(nValue);
			}
			else
			{
				pCtrlTreeOptions->SetItemEnable(hItem,TRUE,FALSE);

				switch(nValue.Val){
				case FCFG_DEF:	
					ASSERT(nValue.Def != FCFG_STD);
					if(nValue.Def == FCFG_DEF) // we dont have category specyfic tweaks (its global)
						sText = FormatNumber(nValue,nValue.Glb,_T(" (glb def)"));
					else if(nValue.Def == FCFG_UNK) // we have files from different categories
						sText.Format(_T("??? (def)")); 
					else
						sText = FormatNumber(nValue,GetRightDef(nValue),_T(" (def)"));
					break;
				case FCFG_GLB: 
					sText = FormatNumber(nValue,nValue.Glb,_T(" (glb)"));
					break;
				case FCFG_UNK:	
					sText.Empty(); 
					break;
				default:	
					sText = FormatNumber(nValue,nValue.Val);
				}
			}
		}
		else
		{
			sText = FormatNumber(nValue,nValue.Val);
		}
		pCtrlTreeOptions->SetEditText(hItem, sText);
	}
}

//////////////////////////////////////////////
// smart editbox values validator helper
//

void PrepAuxSet(CTreeOptionsCtrl &hCtrl, HTREEITEM htiEdit, int MaxRadio, SintD &Edit,bool isGlobal, HTREEITEM htiRadio)
{
	int Index;
	HTREEITEM hCheckItem;
	hCtrl.GetRadioButton(htiRadio ? htiRadio : htiEdit, Index, hCheckItem);

	if(Index == -1 || Index > MaxRadio)
	{
		if(Index == MaxRadio + 1) // default
			Edit.aS(0xFF | 1 << 8);
		else if(Index == MaxRadio + 2) // global
			Edit.aS(0xFF | 2 << 8);
		else
			Edit.aS(0xFF);
	}
	else
		Edit.aS(Index | isGlobal << 8);

	if(Edit.SubSet(true) == 0xFF)
		hCtrl.SetEditText(htiEdit,GetPreSetStr(Edit));
	else
		hCtrl.SetEditText(htiEdit,FormatNumber(Edit,Edit.Std[Edit.SubSet()],Edit.Base ? _T("") : _T(" (std)")));

	hCtrl.SetItemEnable(htiEdit,Edit.SubSet(true) != 0xFF,FALSE);
}

UINT PrepAuxSet(int Index,bool isGlobal)
{
	if(Index == FCFG_UNK || Index > FCFG_BASE)
	{
		if(Index == FCFG_DEF) // default
			return 0xFF | 1 << 8;
		if(Index == FCFG_GLB) // global
			return 0xFF | 2 << 8;
		return 0xFF;
	}

	return Index | isGlobal << 8;
}

//////////////////////////////////////////////
// validate on the fly smart editbox values
//

BOOL CheckTreeEditLimit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, SintD Def)
{
	if(Def.SubSet(true) == 0xFF) // editbox dissabled due to undefined set
		return FALSE;

	CString strBuffer = hCtrl.GetEditText(hItem);
	if(!strBuffer.IsEmpty())
	{
		strBuffer.MakeLower();
		if(strBuffer.Find(_T("std")) != -1)
		{
			strBuffer = FormatNumber(Def,Def.Std[Def.SubSet()]);
			hCtrl.SetEditText(hItem,strBuffer);
			return TRUE;	
		}
		else if(strBuffer.Find(_T("def")) != -1)
		{
			if(Def.PreSet()) // we have a mode set so we can't use prefs from other levels
			{
				hCtrl.SetEditText(hItem,FormatNumber(Def,Def.Std[Def.SubSet()],Def.Base ? _T("") : _T(" (std)")));
				return FALSE;
			}
			strBuffer = FormatNumber(Def,GetRightDef(Def),Def.Base ? _T("") : _T(" (def)"));
			hCtrl.SetEditText(hItem,strBuffer);
			return TRUE;
		}
		else if(strBuffer.Find(_T("glb")) != -1)
		{
			if(Def.PreSet()) // we have a mode set so we can't use prefs from other levels
			{
				hCtrl.SetEditText(hItem,FormatNumber(Def,Def.Std[Def.SubSet()],Def.Base ? _T("") : _T(" (std)")));
				return FALSE;
			}
			strBuffer = FormatNumber(Def,Def.Glb,Def.Base ? _T("") : _T(" (glb)"));
			hCtrl.SetEditText(hItem,strBuffer);
			return TRUE;	
		}
		else if(strBuffer.Find(_T("aut")) != -1 && Def.Aut[Def.SubSet()]) // Note: auto is habdled in the same way as any normal numerical value
		{
			strBuffer = FormatNumber(Def,FCFG_AUT);
			hCtrl.SetEditText(hItem,strBuffer);
			return TRUE;	
		}
		else
		{
			ParseFormatedNumber(Def,strBuffer);
			int iBuffer = Def.Val;
			int iOldBuffer = iBuffer;
			MinMax(&iBuffer, Def.Min[Def.SubSet()], Def.Max[Def.SubSet()]);
			strBuffer = FormatNumber(Def,iBuffer);
			hCtrl.SetEditText(hItem,strBuffer);
			if(iBuffer != iOldBuffer)
				return TRUE;
		}
	}
	return FALSE;
}

BOOL CheckTreeEditLimit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, int Min, int Def, int Max){
	CString strBuffer = hCtrl.GetEditText(hItem);
	if(!strBuffer.IsEmpty()){
		int iOldBuffer = _tstol(strBuffer);
		int iBuffer = Str2Int(strBuffer,Min,Def,Max);
		if(iBuffer != iOldBuffer){
			hCtrl.SetEditText(hItem,Int2Str(iBuffer));
			return TRUE;
		}
	}
	return FALSE;
}

BOOL CheckTreeEditLimit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, float Min, float Def, float Max){
	CString strBuffer = hCtrl.GetEditText(hItem);
	if(!strBuffer.IsEmpty()){
		float fOldBuffer = (float)_tstof(strBuffer);
		float fBuffer = Str2FloatNum(strBuffer,Min,Def,Max);
		if(fBuffer != fOldBuffer){
			hCtrl.SetEditText(hItem,FloatNum2Str(fBuffer));
			return TRUE;
		}
	}
	return FALSE;
}

#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
void SetTreeAdapterIndex(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info, BOOL Mask, BOOL WithCheck, BOOL TreeState, BOOL Value){
	if(WithCheck)
		hItem = hCtrl.InsertCheckBox(Label,Parent,Value,TreeState?1:0);
	else
		hItem = hCtrl.InsertItem(Label, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, Parent);
	hCtrl.AddComboBox(hItem,RUNTIME_CLASS(CAdapterIndex),0,Mask);
	//hCtrl.SetItemSize(hItem,15);
	hCtrl.SetItemInfo(hItem,Info);
}

void DDX_AdapterIndex(CTreeOptionsCtrl* hCtrl, CDataExchange* pDX, int /*nIDC*/, HTREEITEM hItem, DWORD& nValue)
{
	if (pDX->m_bSaveAndValidate){
		BOOL Mask = (BOOL)hCtrl->GetOpaque(hItem,TRUE);
		USES_CONVERSION;
		DWORD ip = inet_addr(T2CA(hCtrl->GetComboText(hItem)));
		if (ip && (ip != INADDR_NONE || Mask))
			nValue = ip;
	}else{
		hCtrl->SetComboText(hItem, ipstr(nValue));
	}
}


IMPLEMENT_DYNCREATE(CAdapterIndex, CTreeOptionsCombo)

CAdapterIndex::CAdapterIndex()
{
}

CAdapterIndex::~CAdapterIndex()
{
}

BEGIN_MESSAGE_MAP(CAdapterIndex, CTreeOptionsCombo)
	//{{AFX_MSG_MAP(CAdapterIndex)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CAdapterIndex::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  //Add strings to the combo
	BOOL Mask = (BOOL)m_pTreeCtrl->GetOpaque(m_hTreeCtrlItem,TRUE);
	if(Mask){
		//AddString(_T("0.0.0.0")); // ok this one is useless
		AddString(_T("255.0.0.0"));
		AddString(_T("255.255.0.0"));
		AddString(_T("255.255.255.0"));
		AddString(_T("255.255.255.255"));
	}else{
		CStringList* list = theApp.bandwidthControl->GetAdapterList();
		if(list == NULL)
			AddString(GetResString(IDS_ERROR));
		else
			while(!list->IsEmpty())
				AddString(list->RemoveHead());
		delete list;
	}
	
	return 0;
}

DWORD CAdapterIndex::GetWindowStyle()
{
	return WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN;
}
#endif // NEO_BC // NEO: NBC END


void SetTreePriority(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info){
	hItem = hCtrl.InsertItem(Label, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, Parent);
	hCtrl.AddComboBox(hItem,RUNTIME_CLASS(CPriority));
	//hCtrl.SetItemSize(hItem,15);
	hCtrl.SetItemInfo(hItem,Info);
}

void DDX_PriorityT(CTreeOptionsCtrl* hCtrl, CDataExchange* pDX, int /*nIDC*/, HTREEITEM hItem, int& nValue)
{
	CString sText;
	if (pDX->m_bSaveAndValidate){
		sText = hCtrl->GetComboText(hItem);
		if(sText == GetResString(IDS_X_PRIO_HIGH))
			nValue = THREAD_PRIORITY_HIGHEST;
		else if(sText == GetResString(IDS_X_PRIO_ABOVE))
			nValue = THREAD_PRIORITY_ABOVE_NORMAL;
		else if(sText == GetResString(IDS_X_PRIO_BELOW))
			nValue = THREAD_PRIORITY_BELOW_NORMAL;
		else if(sText == GetResString(IDS_X_PRIO_LOW))
			nValue = THREAD_PRIORITY_LOWEST;
		else if(sText == GetResString(IDS_X_PRIO_NORMAL))
			nValue = THREAD_PRIORITY_NORMAL;
	}else{
		switch(nValue){
			case THREAD_PRIORITY_HIGHEST:
				sText = GetResString(IDS_X_PRIO_HIGH); break;
			case THREAD_PRIORITY_ABOVE_NORMAL:
				sText = GetResString(IDS_X_PRIO_ABOVE); break;
			case THREAD_PRIORITY_BELOW_NORMAL:
				sText = GetResString(IDS_X_PRIO_BELOW); break;
			case THREAD_PRIORITY_LOWEST:
				sText = GetResString(IDS_X_PRIO_LOW); break;
			case THREAD_PRIORITY_NORMAL:
			default:
				sText = GetResString(IDS_X_PRIO_NORMAL);
		}
		hCtrl->SetComboText(hItem, sText);
	}
}

void DDX_PriorityC(CTreeOptionsCtrl* hCtrl, CDataExchange* pDX, int /*nIDC*/, HTREEITEM hItem, DWORD& nValue)
{
	CString sText;
	if (pDX->m_bSaveAndValidate){
		sText = hCtrl->GetComboText(hItem);
		if(sText == GetResString(IDS_X_PRIO_HIGH))
			nValue = HIGH_PRIORITY_CLASS;
		else if(sText == GetResString(IDS_X_PRIO_ABOVE))
			nValue = ABOVE_NORMAL_PRIORITY_CLASS;
		else if(sText == GetResString(IDS_X_PRIO_BELOW))
			nValue = BELOW_NORMAL_PRIORITY_CLASS;
		else if(sText == GetResString(IDS_X_PRIO_LOW))
			nValue = IDLE_PRIORITY_CLASS;
		else if(sText == GetResString(IDS_X_PRIO_NORMAL))
			nValue = NORMAL_PRIORITY_CLASS;
	}else{
		switch(nValue){
			case HIGH_PRIORITY_CLASS:
				sText = GetResString(IDS_X_PRIO_HIGH); break;
			case ABOVE_NORMAL_PRIORITY_CLASS:
				sText = GetResString(IDS_X_PRIO_ABOVE); break;
			case BELOW_NORMAL_PRIORITY_CLASS:
				sText = GetResString(IDS_X_PRIO_BELOW); break;
			case IDLE_PRIORITY_CLASS:
				sText = GetResString(IDS_X_PRIO_LOW); break;
			case NORMAL_PRIORITY_CLASS:
			default:
				sText = GetResString(IDS_X_PRIO_NORMAL);
		}
		hCtrl->SetComboText(hItem, sText);
	}
}

IMPLEMENT_DYNCREATE(CPriority, CTreeOptionsCombo)

CPriority::CPriority()
{
}

CPriority::~CPriority()
{
}

BEGIN_MESSAGE_MAP(CPriority, CTreeOptionsCombo)
	//{{AFX_MSG_MAP(CPriority)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


int CPriority::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	
  //Add strings to the combo
	AddString(GetResString(IDS_X_PRIO_HIGH));
	AddString(GetResString(IDS_X_PRIO_ABOVE));
	AddString(GetResString(IDS_X_PRIO_NORMAL));
	AddString(GetResString(IDS_X_PRIO_BELOW));
	AddString(GetResString(IDS_X_PRIO_LOW));

	return 0;
}

DWORD CPriority::GetWindowStyle()
{
	return WS_CHILD | WS_VISIBLE | WS_VSCROLL | CBS_DROPDOWN;
}
