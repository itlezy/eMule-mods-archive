#include "stdafx.h"
#include "TreeOptionsInvisibleModCombo.h"
#include "emule.h"
#include "OtherFunctions.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNCREATE(CTreeOptionsInvisibleModCombo, CTreeOptionsCombo)

CTreeOptionsInvisibleModCombo::CTreeOptionsInvisibleModCombo()
{
}

CTreeOptionsInvisibleModCombo::~CTreeOptionsInvisibleModCombo()
{
}

BEGIN_MESSAGE_MAP(CTreeOptionsInvisibleModCombo, CTreeOptionsCombo)
	ON_WM_CREATE()
	ON_CONTROL_REFLECT(CBN_SELCHANGE, OnCbnSelchange)
END_MESSAGE_MAP()


int CTreeOptionsInvisibleModCombo::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	m_bSelf = true;
	if (CTreeOptionsCombo::OnCreate(lpCreateStruct) == -1)
		return -1;
	m_bSelf = false;

	for(size_t i=0; i < _countof(ModKey);i++)
		AddString(ModKey[i]);
	return 0;
}

void CTreeOptionsInvisibleModCombo::OnCbnSelchange()
{
	if (!m_bSelf)
		((CTreeOptionsCtrlEx*)m_pTreeCtrl)->NotifyParent((UINT_PTR)this, m_hTreeCtrlItem);
}
