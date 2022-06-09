#pragma once 


// CCommentDialog dialog 

class CCommentDialog : public CDialog 
{ 
   DECLARE_DYNAMIC(CCommentDialog) 

public: 
   CCommentDialog(CKnownFile* file);   // standard constructor 
   virtual ~CCommentDialog(); 
   void Localize(); 
   virtual BOOL OnInitDialog(); 

    

// Dialog Data 
   enum { IDD = IDD_COMMENT }; 
protected: 
   virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support 
   DECLARE_MESSAGE_MAP() 
public: 
   afx_msg void OnBnClickedApply(); 
   afx_msg void OnBnClickedErase();
   afx_msg void OnBnClickedCancel(); 
private: 
	CComboBox   ratebox;//For rate 
	CKnownFile* m_file; 
}; 