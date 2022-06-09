// PreviewDlg.cpp : implementation file
//

#include "stdafx.h"
#include "MsgViewer.h"
#include "PreviewDlg.h"
#include ".\previewdlg.h"


// CPreviewDlg dialog

IMPLEMENT_DYNAMIC(CPreviewDlg, CDialog)
CPreviewDlg::CPreviewDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CPreviewDlg::IDD, pParent)
{
}

CPreviewDlg::~CPreviewDlg()
{
}

void CPreviewDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CPreviewDlg, CDialog)
	ON_WM_PAINT()
END_MESSAGE_MAP()


// CPreviewDlg message handlers

void CPreviewDlg::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CRect stRect;
	GetClientRect(stRect);
	VERIFY(BitBlt(dc.m_hDC, 0, 0, stRect.right, stRect.bottom, m_hImageDC, 0, 0, SRCCOPY));
}


BOOL CPreviewDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	ShowWindow(SW_MAXIMIZE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
