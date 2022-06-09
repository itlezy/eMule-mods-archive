// PageSelectionBox.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PageSelectionBox.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// CPageSelectionBox dialog

IMPLEMENT_DYNAMIC(CPageSelectionBox, CDialog)
CPageSelectionBox::CPageSelectionBox(CWnd* pParent /*=NULL*/)
	: CDialog(CPageSelectionBox::IDD, pParent)
{
}

CPageSelectionBox::~CPageSelectionBox()
{
}

void CPageSelectionBox::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPageSelectionBox, CDialog)
END_MESSAGE_MAP()


// CPageSelectionBox message handlers
