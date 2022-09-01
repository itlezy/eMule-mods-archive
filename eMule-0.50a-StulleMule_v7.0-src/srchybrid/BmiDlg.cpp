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
#include "BmiDlg.h"
#include "OtherFunctions.h" // for GetResString()

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CBmiDlg, CDialog)

BEGIN_MESSAGE_MAP(CBmiDlg, CDialog)
	ON_BN_CLICKED(IDC_BMI_ENGLISH, OnBnClickedEnglish)
	ON_BN_CLICKED(IDC_BMI_METRIC, OnBnClickedMetric)
END_MESSAGE_MAP()

CBmiDlg::CBmiDlg()
	: CDialog(CBmiDlg::IDD, 0)
{
}

CBmiDlg::~CBmiDlg()
{
}

BOOL CBmiDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	Localize();

	return TRUE;
}

void CBmiDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

void CBmiDlg::Localize(void)
{	
	if(m_hWnd)
	{
		CString strBuffer;

		SetWindowText(GetResString(IDS_BMI_WND));
		GetDlgItem(IDC_BMI_HEAD)->SetWindowText(GetResString(IDS_BMI_HEAD));
		GetDlgItem(IDC_BMI_WEIGHT_LABEL)->SetWindowText(GetResString(IDS_BMI_WEIGHT_LABEL));
		GetDlgItem(IDC_BMI_HEIGHT_LABEL)->SetWindowText(GetResString(IDS_BMI_HEIGHT_LABEL));
		GetDlgItem(IDC_BMI_ENGLISH)->SetWindowText(GetResString(IDS_BMI_ENGLISH));
		GetDlgItem(IDC_BMI_METRIC)->SetWindowText(GetResString(IDS_BMI_METRIC));
		GetDlgItem(IDC_BMI_RESULT_LABEL)->SetWindowText(GetResString(IDS_BMI_RESULT_LABEL));
		GetDlgItem(IDC_BMI_RESULT)->SetWindowText(GetResString(IDS_BMI_RESULT));
		GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT));
		GetDlgItem(IDOK)->SetWindowText(GetResString(IDS_OK));

		strBuffer.Format(_T("%.2f"), 0.0f);
		GetDlgItem(IDC_BMI_WEIGHT)->SetWindowText(strBuffer);
		GetDlgItem(IDC_BMI_HEIGHT)->SetWindowText(strBuffer);
	}
}

void CBmiDlg::OnBnClickedEnglish()
{
	TCHAR buffer[510];
	CString strBuffer;

	if(GetDlgItem(IDC_BMI_WEIGHT)->GetWindowTextLength() && GetDlgItem(IDC_BMI_HEIGHT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_BMI_WEIGHT)->GetWindowText(buffer,20);
		float Weight = (float)_tstof(buffer);

		GetDlgItem(IDC_BMI_HEIGHT)->GetWindowText(buffer,20);
		float Height = (float)_tstof(buffer);

		float result = (Weight/(Height*Height))*703;

		strBuffer.Format(_T("%.1f"), result);
		GetDlgItem(IDC_BMI_RESULT)->SetWindowText(strBuffer);

		if (result < 18.5f)
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT1));
		else if (result >= 18.5f && result < 24.9f)
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT2));
		else if (result >= 25.0f && result < 29.9f)
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT3));
		else
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT4));
	}
}

void CBmiDlg::OnBnClickedMetric()
{
	TCHAR buffer[510];
	CString strBuffer;

	if(GetDlgItem(IDC_BMI_WEIGHT)->GetWindowTextLength() && GetDlgItem(IDC_BMI_HEIGHT)->GetWindowTextLength())
	{
		GetDlgItem(IDC_BMI_WEIGHT)->GetWindowText(buffer,20);
		float Weight = (float)_tstof(buffer);

		GetDlgItem(IDC_BMI_HEIGHT)->GetWindowText(buffer,20);
		float Height = (float)_tstof(buffer);

		float result = Weight/(Height*Height);

		strBuffer.Format(_T("%.1f"), result);
		GetDlgItem(IDC_BMI_RESULT)->SetWindowText(strBuffer);

		if (result < 18.5f)
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT1));
		else if (result >= 18.5f && result < 24.9f)
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT2));
		else if (result >= 25.0f && result < 29.9f)
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT3));
		else
			GetDlgItem(IDC_BMI_RESULTSTR)->SetWindowText(GetResString(IDS_BMI_RESULT4));
	}
}