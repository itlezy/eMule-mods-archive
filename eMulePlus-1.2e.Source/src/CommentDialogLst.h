#pragma once 
#include "ResizableLib\ResizableDialog.h"
#include "MuleListCtrl.h"
// CCommentDialogLst dialog 

class CCommentDialogLst : public CResizableDialog 
{ 
   DECLARE_DYNAMIC(CCommentDialogLst) 

public: 
   CCommentDialogLst(CPartFile* file); 
   virtual ~CCommentDialogLst(); 
   void Localize(); 
   virtual BOOL OnInitDialog(); 

// Dialog Data 
   enum { IDD = IDD_COMMENTLST }; 
protected: 
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support 
   CMuleListCtrl pmyListCtrl;
   DECLARE_MESSAGE_MAP()
   virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
   afx_msg void OnNMRclickList(NMHDR *pNMHDR, LRESULT *pResult); 
public: 
   afx_msg void OnBnClickedApply(); 
   afx_msg void OnBnClickedRefresh(); 
   afx_msg void OnDestroy();
private: 
   void CompleteList(); 
   CPartFile* m_file; 
};
