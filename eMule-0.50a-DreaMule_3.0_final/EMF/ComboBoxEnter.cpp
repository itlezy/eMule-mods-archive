// ComboBoxEnter.cpp : 实现文件
//

#include "stdafx.h"
#include "emule.h"
#include "ComboBoxEnter.h"
#include ".\comboboxenter.h"


// CComboBoxEnter

IMPLEMENT_DYNAMIC(CComboBoxEnter, CComboBox)
CComboBoxEnter::CComboBoxEnter()
{
}

CComboBoxEnter::~CComboBoxEnter()
{
}


BEGIN_MESSAGE_MAP(CComboBoxEnter, CComboBox)
END_MESSAGE_MAP()



// CComboBoxEnter 消息处理程序


BOOL CComboBoxEnter::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	ASSERT( pMsg->message != WM_CHARTOITEM );

	if (pMsg->message == WM_KEYDOWN)
	{
		GetParent()->SendMessage(pMsg->message, pMsg->wParam,  pMsg->lParam);
	}
	return CComboBox::PreTranslateMessage(pMsg);
}
