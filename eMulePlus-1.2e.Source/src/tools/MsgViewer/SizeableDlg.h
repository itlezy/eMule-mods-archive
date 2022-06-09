// SizeableDlg.h: interface for the CSizeableDlg class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

class CSizeableDlg : public CDialog
{
protected:
	void OnSizeControl(CWnd& stWnd, int cx, int cy, UINT nLocks);
	void OnSizeControl(UINT nDlgCtrlID, int cx, int cy, UINT nLocks);

	CSizeableDlg(UINT nID, CWnd* pParent) : CDialog(nID, pParent) {}
};
