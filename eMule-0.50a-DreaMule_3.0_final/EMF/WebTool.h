#pragma once



// CHtmlCtrl Html 视图

class CHtmlCtrl : public CHtmlView
{
	DECLARE_DYNCREATE(CHtmlCtrl)

public:
	CHtmlCtrl(){ };           // 动态创建所使用的受保护的构造函数
	virtual ~CHtmlCtrl(){ };

	BOOL CreateFromStatic(UINT nID, CWnd* pParent);

public:
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnBeforeNavigate2(LPCTSTR lpszURL, DWORD nFlags, LPCTSTR lpszTargetFrameName, CByteArray& baPostedData, LPCTSTR lpszHeaders, BOOL* pbCancel);
	afx_msg void OnDestroy();
	virtual void OnDocumentComplete(LPCTSTR lpszURL);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	virtual void OnNavigateComplete2(LPCTSTR strURL);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	virtual void OnStatusTextChange(LPCTSTR lpszText);
protected:
	virtual void PostNcDestroy();
};