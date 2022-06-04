#pragma once
// CPartStatusCtrl
class CDownloadClientsCtrl;
class CPartFile;
class CPartStatusCtrl : public CStatic
{
	DECLARE_DYNAMIC(CPartStatusCtrl)

public:
	CPartStatusCtrl(CDownloadClientsCtrl*pDcl):pdcl(pDcl),dwUpdated(0){}
	virtual ~CPartStatusCtrl();
	void Refresh(bool force=true);
	void Refresh(CPartFile*file);
protected:
	CDownloadClientsCtrl* pdcl;
	CBitmap          status;
	DWORD            dwUpdated;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
};