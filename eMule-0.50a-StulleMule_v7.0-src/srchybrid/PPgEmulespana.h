#pragma once

#include "TreeOptionsCtrlEx.h"
/*Commented by SiRoB
#include "SkinsListCtrl.h" 	// Added by by MoNKi [MoNKi: -Skin Selector-]
*/
// Cuadro de diálogo de CPPgEmulespana

// MORPH START leuk_he tooltipped
/*
class CPPgEmulespana : public CPropertyPage
*/
class CPPgEmulespana : public CPPgtooltipped  
// MORPH END leuk_he tooltipped
{
	DECLARE_DYNAMIC(CPPgEmulespana)

public:
	CPPgEmulespana();
	virtual ~CPPgEmulespana();

// Datos del cuadro de diálogo
	enum { IDD = IDD_PPG_EMULESPANA1 };

	void	Localize(void);

protected:
	bool	m_bInitializedTreeOpts;
	CTreeOptionsCtrlEx	m_ctrlTreeOptions;
	
/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -invisible mode-]
	HTREEITEM	m_htiInvisibleModeRoot;
	HTREEITEM	m_htiInvisibleMode;
	HTREEITEM	m_htiInvisibleModeMod;
	HTREEITEM	m_htiInvisibleModeKey;
	BOOL		m_bInvisibleMode;
	CString		m_sInvisibleModeMod;
	CString		m_sInvisibleModeKey;
	UINT		m_iInvisibleModeActualKeyModifier;
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -UPnPNAT Support-]
	HTREEITEM	m_htiUPnPGroup;
	HTREEITEM	m_htiUPnP;
//	HTREEITEM	m_htiUPnPWeb;
	//HTREEITEM	m_htiUPnPTryRandom;
	//bool		m_bUPnP;
//	bool		m_bUPnPWeb;
	
	int m_iUPnPPort;
	bool m_bUPnPClearOnClose;
    bool m_bUPnPLimitToFirstConnection;
	int m_iDetectuPnP; // allowed values: 
    /*#define UPNP_DO_AUTODETECT 2
    #define UPNP_DETECTED 0
    #define UPNP_NOT_DETECTED -1 
	#define UPNP_NO_DETECTEDTION -2 
    #define UPNP_NOT_NEEDED -10
	*/
	HTREEITEM	m_htiUPnPPort;
	HTREEITEM	m_htiUPnPClearOnClose;
    HTREEITEM	m_htiUPnPLimitToFirstConnection;
	HTREEITEM	m_htiDetectuPnP; // allowed values: 

    bool m_bUPnPForceUpdate;
	HTREEITEM	m_htiUPnPForceUpdate;


	//BOOL		m_bUPnPTryRandom;
	// End MoNKi

/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Support for High Contrast Mode-]
	HTREEITEM	m_htiHighContrastRoot;
	HTREEITEM	m_htiHighContrast;
	HTREEITEM	m_htiHighContrastDisableSkins;
	BOOL		m_bHighContrast;
	BOOL		m_bHighContrastDisableSkins;
	// End MoNKi
*/
	// Added by by MoNKi [MoNKi: -Improved ICS-Firewall support-]
	HTREEITEM	m_htiICFSupportRoot;
	HTREEITEM	m_htiICFSupport;
	HTREEITEM	m_htiICFSupportClearAtEnd;
	HTREEITEM	m_htiICFSupportServerUDP;
	bool		m_bICFSupport;
	bool		m_bICFSupportClearAtEnd;
	bool		m_bICFSupportServerUDP;
	// End MoNKi
	
/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -Skin Selector-]
	CTabCtrl m_tabPrefs;
	CString  m_sTBSkinsDir;
	CString	 m_sSkinsDir;
	CSkinsListCtrl m_listTBSkins;
	CSkinsListCtrl m_listSkins;
	CEdit m_editSkinsDir;
	// End MoNKi
*/
	// Added by MoNKi [ SlugFiller: -lowIdRetry- ]
	HTREEITEM	m_htiLowIdRetry;
	int			m_iLowIdRetry;
	// End SlugFiller

	// Added by MoNKi [ MoNKi: -Wap Server- ]
	HTREEITEM	m_htiWapRoot;
	HTREEITEM	m_htiWapEnable;
	HTREEITEM	m_htiWapPort;
	HTREEITEM	m_htiWapTemplate;
	HTREEITEM	m_htiWapPass;
	HTREEITEM	m_htiWapLowEnable;
	HTREEITEM	m_htiWapLowPass;
	bool		m_bWapEnable;
	int			m_iWapPort;
	CString		m_sWapTemplate;
	CString		m_sWapPass;
	bool		m_bWapLowEnable;
	CString		m_sWapLowPass;
	// End MoNKi
	// MORPH START leuk_he upnp bindaddr
    DWORD m_dwUpnpBindAddr; 
    HTREEITEM m_htiUpnpBinaddr ;
	//HTREEITEM m_htipnpBindAddrIsDhcp ;
	//MORPH END leuk_he upnp bindaddr
/*Commented by SiRoB
	// Added by MoNKi [MoNKi: -USS initial TTL-]
	HTREEITEM	m_htiUSSRoot;
	HTREEITEM	m_htiUSSTTL;
	int			m_iUSSTTL;
	// End MoNKi

	// Added by MoNKi [MoNKi: -Custom incoming folder icon-]
	HTREEITEM	m_htiCustomIncomingIcon;
	BOOL		m_bCustomIncomingIcon;
	// End MoNKi
*/
	// Added by MoNKi [MoNKi: -Random Ports-]
	HTREEITEM	m_htiRandomPortsResetTime;
	int			m_iRandomPortsResetTime;
	// End MoNKi

	void ChangeTab(int nitem);

	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	virtual void DoDataExchange(CDataExchange* pDX);    // Compatibilidad con DDX o DDV

	DECLARE_MESSAGE_MAP()
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
public:
/*Commented by SiRoB
	afx_msg void OnPaint();
*/
	afx_msg void OnDestroy();
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();

/*Commented by SiRoB
	// Added by by MoNKi [MoNKi: -Skin Selector-]
	afx_msg void OnTcnSelchangeTabEmulespanaPrefs(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnSkinsDir();
	afx_msg void OnBnClickedBtnSkinsReload();
	afx_msg void OnEnChangeEditSkinsDir();
	afx_msg void OnSysColorChange();
	afx_msg void OnLvnItemchangedListSkins(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnLvnItemchangedListTbSkins(NMHDR *pNMHDR, LRESULT *pResult);
	// End MoNKi
*/
};
