#pragma once
#include "ResizableLib\ResizableDialog.h"
#include "IconStatic.h"
#include "kademlia/routing/contact.h"

class CKadContactListCtrl;
class CKadContactHistogramCtrl;
class CKadSearchListCtrl;
class CCustomAutoComplete;

class CKademliaWnd : public CResizableDialog
{
	DECLARE_DYNAMIC(CKademliaWnd)

public:
	CKademliaWnd(CWnd* pParent = NULL);   // standard constructor
	virtual ~CKademliaWnd();

	// Dialog Data
	enum { IDD = IDD_KADEMLIAWND };

	// Contacts
	UINT GetContactCount() const;
	void UpdateKadContactCount();
	void ShowContacts();
	void HideContacts();
	bool ContactAdd(const Kademlia::CContact* contact);
	void ContactRem(const Kademlia::CContact* contact);
	void ContactRef(const Kademlia::CContact* contact);

	// Searches
	CKadSearchListCtrl* searchList;

	void Localize();
	void UpdateControlsState();
	BOOL SaveAllSettings();

protected:
	CStatic kadContactLab;
	CStatic kadSearchLab;
	CIconStatic m_ctrlBootstrap;
	CKadContactListCtrl* m_contactListCtrl;
	CKadContactHistogramCtrl* m_contactHistogramCtrl;
	CCustomAutoComplete* m_pacONBSIPs;
	HICON icon_kadcont;
	HICON icon_kadsea;

	void SetAllIcons();

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnBnClickedBootstrapbutton();
	afx_msg void OnBnConnect();
	afx_msg void OnBnClickedFirewallcheckbutton();
	afx_msg void OnSysColorChange();
	afx_msg void OnEnSetfocusBootstrapip();
};
