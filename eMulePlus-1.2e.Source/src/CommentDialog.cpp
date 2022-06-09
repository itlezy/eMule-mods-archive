//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

#include "stdafx.h" 
#include "emule.h" 
#include "CommentDialog.h" 

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// CommentDialog dialog 

IMPLEMENT_DYNAMIC(CCommentDialog, CDialog) 
CCommentDialog::CCommentDialog(CKnownFile* file) 
   : CDialog(CCommentDialog::IDD, 0) 
{ 
   m_file = file; 
} 

CCommentDialog::~CCommentDialog() 
{ 
} 

void CCommentDialog::DoDataExchange(CDataExchange* pDX) 
{ 
   CDialog::DoDataExchange(pDX); 
   DDX_Control(pDX, IDC_RATELIST, ratebox);//for rate 
} 

BEGIN_MESSAGE_MAP(CCommentDialog, CDialog) 
   ON_BN_CLICKED(IDC_CMT_OK, OnBnClickedApply) 
   ON_BN_CLICKED(IDC_CMT_ERASE, OnBnClickedErase)
   ON_BN_CLICKED(IDC_CMT_CANCEL, OnBnClickedCancel) 
END_MESSAGE_MAP() 

void CCommentDialog::OnBnClickedApply() 
{ 
	EMULE_TRY

	CString strValue;

	GetDlgItemText(IDC_CMT_TEXT, strValue);
// No empty comments
	if(!strValue.IsEmpty() || ((byte)ratebox.GetCurSel() != 0))
	{ 
		int ratio = ratebox.GetCurSel();
		if (ratio == 4)
			ratio = PF_RATING_GOOD;
		else if (ratio == 3)
			ratio = PF_RATING_FAIR;

		m_file->SetFileComment(strValue);
		m_file->SetFileRating(static_cast<_EnumPartFileRating>(ratio));//for Rate
		CDialog::OnOK();
	}
	else
	{
		AfxMessageBox(GetResString(IDS_CMT_NOCOMMENT));
	}

	EMULE_CATCH
} 

void CCommentDialog::OnBnClickedErase() 
{
	EMULE_TRY

	if(AfxMessageBox(GetResString(IDS_ERASE_SURE), MB_YESNO) == IDYES)
	{
		m_file->RemoveFileCommentAndRating();
		CDialog::OnOK();
	}

	EMULE_CATCH
}

void CCommentDialog::OnBnClickedCancel() 
{ 
   CDialog::OnCancel(); 
} 

BOOL CCommentDialog::OnInitDialog()
{ 
   CDialog::OnInitDialog(); 
   Localize(); 

   SetDlgItemText(IDC_CMT_TEXT, m_file->GetFileComment()); 
   ((CEdit*)GetDlgItem(IDC_CMT_TEXT))->SetLimitText(MAXFILECOMMENTLEN);

   return TRUE; 
} 

void CCommentDialog::Localize(void)
{ 
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_CMT_OK, IDS_PW_APPLY },
		{ IDC_CMT_CANCEL, IDS_CANCEL },
		{ IDC_CMT_ERASE, IDS_ERASE },
		{ IDC_CMT_LQUEST, IDS_CMT_QUEST },
		{ IDC_CMT_LAIDE, IDS_CMT_AIDE },
		{ IDC_RATEQUEST, IDS_CMT_RATEQUEST },
		{ IDC_RATEHELP, IDS_CMT_RATEHELP }
	};
	static const uint16 s_auResCombo[] = {
		IDS_CMT_NOTRATED, IDS_CMT_FAKE, IDS_CMT_POOR,
		IDS_CMT_FAIR, IDS_CMT_GOOD, IDS_CMT_EXCELLENT
	};

	EMULE_TRY

	for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		SetDlgItemText(s_auResTbl[i][0], GetResString(static_cast<UINT>(s_auResTbl[i][1])));

	while (ratebox.GetCount()>0)
		ratebox.DeleteString(0); 
	
	for (uint32 i = 0; i < ARRSIZE(s_auResCombo); i++)
		ratebox.AddString(GetResString(s_auResCombo[i]));

	int ratio;
	if(m_file->GetFileRating() == PF_RATING_FAIR)
		ratio = 3;
	else if (m_file->GetFileRating() == PF_RATING_GOOD)
		ratio = 4;
	else
		ratio = static_cast<int>(m_file->GetFileRating());

	if(ratebox.SetCurSel(ratio)==CB_ERR)
		ratebox.SetCurSel(0); 
	 
	CString strTitle; 
	
	strTitle.Format(GetResString(IDS_CMT_TITLE), m_file->GetFileName());
	SetWindowText(strTitle); 

	EMULE_CATCH
} 
