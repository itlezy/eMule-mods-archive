// ComboBoxEnter.cpp : ʵ���ļ�
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



// CComboBoxEnter ��Ϣ�������


BOOL CComboBoxEnter::PreTranslateMessage(MSG* pMsg)
{
	// TODO: �ڴ����ר�ô����/����û���
	ASSERT( pMsg->message != WM_CHARTOITEM );

	if (pMsg->message == WM_KEYDOWN)
	{
		GetParent()->SendMessage(pMsg->message, pMsg->wParam,  pMsg->lParam);
	}
	return CComboBox::PreTranslateMessage(pMsg);
}
