//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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
#include "AddFriend.h"
#include "Friend.h"
#include "otherfunctions.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CAddFriend, CDialog)
CAddFriend::CAddFriend()
	: CDialog(CAddFriend::IDD, 0)
{
}

CAddFriend::~CAddFriend()
{
}

void CAddFriend::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BOOL CAddFriend::OnInitDialog()
{
	CDialog::OnInitDialog();
	Localize();

//	Limit maximal number of input characters
	reinterpret_cast<CEdit*>(GetDlgItem(IDC_USERNAME))->SetLimitText(MAX_NICK_LENGTH);
	reinterpret_cast<CEdit*>(GetDlgItem(IDC_PORT))->SetLimitText(5);

	return true;
}

BEGIN_MESSAGE_MAP(CAddFriend, CDialog)
	ON_BN_CLICKED(IDC_ADD, OnAddBtn)
END_MESSAGE_MAP()


// CAddFriend message handlers
void CAddFriend::Localize()
{
	static const uint16 s_auResTbl[][2] =
	{
		{ IDC_INFO1, IDS_PAF_REQINFO },
		{ IDC_INFO2, IDS_PAF_MOREINFO },
		{ IDC_ADD, IDS_ADD },
		{ IDCANCEL, IDS_CANCEL },
		{ IDC_STATIC31, IDS_CD_UNAME },
		{ IDC_STATIC34, IDS_CD_UIP }
	};

	SetWindowText(GetResString(IDS_ADDAFRIEND));

	for (uint32 i = 0; i < ARRSIZE(s_auResTbl); i++)
		SetDlgItemText(s_auResTbl[i][0], GetResString(static_cast<UINT>(s_auResTbl[i][1])));

	SetDlgItemText(IDC_STATIC35, GetResString(IDS_PORT) + _T(':'));
}

void CAddFriend::OnAddBtn()
{
	uint32		dwIP, dwByte1, dwByte2, dwByte3, dwByte4;
	bool		bInvalidInput;
	CString		strTemp;
	int			iPort;

	GetDlgItemText(IDC_PORT, strTemp);
	bInvalidInput = (strTemp.IsEmpty() || ((iPort = _tstoi(strTemp)) > 0xffff));

	GetDlgItemText(IDC_IP, strTemp);
	if ( bInvalidInput ||
		(_stscanf(strTemp, _T("%u.%u.%u.%u"), &dwByte1, &dwByte2, &dwByte3, &dwByte4) != 4) ||
		(dwByte1 > 255) || (dwByte2 > 255) || (dwByte3 > 255) || (dwByte4 > 255) )
	{
		MessageBox(GetResString(IDS_ERR_NOVALIDFRIENDINFO));
	}
	else
	{
		dwIP = dwByte1 | (dwByte2 << 8) | (dwByte3 << 16) | (dwByte4 << 24);

		GetDlgItemText(IDC_USERNAME, strTemp);

		g_App.m_pFriendList->AddFriend(NULL, 0, dwIP, static_cast<uint16>(iPort), 0, strTemp, 0);
	
		OnCancel();
	}
}
