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

#pragma once

#define FCFG_ASC 3

struct SintD{ // struct int default
	void MSMG(int MIN, int STD, int MAX, int GLB, char format = 0)
	{
		Min[0] = MIN;
		Std[0] = STD;
		Max[0] = MAX;
		Glb = GLB;
		Format[0] = format;
		Aut[0] = 0;
		Set = 0;
	}
	void MSMa(int MIN, int STD, int MAX, int Set, char format = 0)
	{
		ASSERT(Set < FCFG_ASC);
		Min[Set] = MIN;
		Std[Set] = STD;
		Max[Set] = MAX;
		Format[Set] = format;
		Aut[Set] = 0;
	}
	void DV(int DEF, int VAL, bool base = false)
	{
		Def = DEF;
		Val = VAL;
		Base = base;
	}
	void A(int aut, int Set = 0)
	{
		ASSERT(Set < FCFG_ASC);
		Aut[Set] = aut;
	}
	void aS(UINT set)
	{
		Set = set;
	}
	int SubSet(bool real = false)
	{
		if(real || ((Set & 0xFF) > 0 && (Set & 0xFF) < FCFG_ASC))
			return (Set & 0xFF) ;
		return 0;
	}
	int PreSet()
	{
		return ((Set >> 8) & 0xFF);
	}
	int	Val; // value to storr in preferences			// May Be everything
	int Min[FCFG_ASC]; // minimal allowed value
	int Def; // relativ default (global or cat)			// may be eberything
	int Std[FCFG_ASC]; // standard hardcoded setting
	int Glb; // current global setting					// may be aut
	int Max[FCFG_ASC]; // maximal allowed value
	int Aut[FCFG_ASC]; // automatic value				// may be unk
	UINT Set;
	bool Base; // base prefs layer have a different handling
	char Format[FCFG_ASC]; // 't' time format, 'n' large number
};
struct SrbtC{ // struct RadioButton Cut
	void CVDC(int CUT, int VAL, int DEF = -1, int CAT = -1)
	{
		Cut = CUT;
		Val = VAL;
		Def = DEF;
		Cat = CAT;
	}
	int Val;
	int Cut;
	int Def;
	int Cat;
};

template<class TYPE_PTR>
void CheckAndClearPrefs(TYPE_PTR* Prefs)
{
	if(Prefs->IsEmpty()){
		delete Prefs;
		Prefs = NULL;
	}else
		Prefs->CheckTweaks();
}

void SetTreeGroup(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, int Icon, HTREEITEM Parent = TVI_ROOT, LPCTSTR Info = NULL);
void SetTreeEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info = NULL);
void SetTreeNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info = NULL);
void SetTreeCheckEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info = NULL, BOOL TreeState = FALSE, BOOL Value = FALSE);
void SetTreeRadioNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info = NULL, BOOL UnSetable = FALSE, BOOL Value = FALSE);
void SetTreeCheckNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info = NULL, BOOL TreeState = FALSE, BOOL Value = FALSE);
void SetTreeCheck(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent,  LPCTSTR Info = NULL, BOOL TreeState = FALSE, BOOL Value = FALSE);
void SetTreeRadio(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent,  LPCTSTR Info = NULL, BOOL UnSetable = FALSE, BOOL Value = FALSE);
//void SetTreeCheckSpinNumEdit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info = NULL, int Min = 0, int Max = INT_MAX, BOOL TreeState = FALSE, BOOL Value = FALSE);

CString MkRdBtnLbl(CString label, SrbtC& Def, int nIndex);

void DDX_TreeRadioEx(CDataExchange* pDX, int nIDC, HTREEITEM hParent, SrbtC& nIndex);
void DDX_TreeEditEx(CDataExchange* pDX, int nIDC, HTREEITEM hItem, SintD& nValue);

void PrepAuxSet(CTreeOptionsCtrl &hCtrl, HTREEITEM htiEdit, int MaxRadio, SintD &Edit,bool isGlobal, HTREEITEM htiRadio = NULL);
UINT PrepAuxSet(int Index, bool isGlobal);

BOOL CheckTreeEditLimit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, SintD Def);

BOOL CheckTreeEditLimit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, int Min, int Def, int Max);
BOOL CheckTreeEditLimit(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, float Min, float Def, float Max);

int GetRightDef(SintD& nValue);
#ifdef NEO_BC // NEO: NBC - [NeoBandwidthControl]
void SetTreeAdapterIndex(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info, BOOL Mask = FALSE, BOOL WithCheck = FALSE, BOOL TreeState = FALSE, BOOL Value = FALSE);
void DDX_AdapterIndex(CTreeOptionsCtrl* hCtrl, CDataExchange* pDX, int nIDC, HTREEITEM hItem, DWORD& nValue);

class CAdapterIndex : public CTreeOptionsCombo
{
public:
	CAdapterIndex();
	virtual ~CAdapterIndex();

protected:
	DWORD GetWindowStyle();
	//{{AFX_VIRTUAL(CAdapterIndex)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CAdapterIndex)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CAdapterIndex)
};
#endif // NEO_BC // NEO: NBC END

void SetTreePriority(CTreeOptionsCtrl &hCtrl, HTREEITEM &hItem, LPCTSTR Label, HTREEITEM Parent, LPCTSTR Info);
void DDX_PriorityT(CTreeOptionsCtrl* hCtrl, CDataExchange* pDX, int nIDC, HTREEITEM hItem, int& nValue);
void DDX_PriorityC(CTreeOptionsCtrl* hCtrl, CDataExchange* pDX, int nIDC, HTREEITEM hItem, DWORD& nValue);

class CPriority : public CTreeOptionsCombo
{
public:
	CPriority();
	virtual ~CPriority();

protected:
	DWORD GetWindowStyle();
	//{{AFX_VIRTUAL(CPriority)
	//}}AFX_VIRTUAL

	//{{AFX_MSG(CPriority)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()

  DECLARE_DYNCREATE(CPriority)
};
