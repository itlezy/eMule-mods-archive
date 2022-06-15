//this file is part of NeoMule
//Copyright (C)2006-2008 David Xanatos ( XanatosDavid@googlemail.com / http://NeoMule.sf.net )
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


#pragma once
#include "Neo\GUI\CP\TreeOptionsCtrl.h" // NEO - [TreeControl] <-- Xanatos --
#include "Neo/GUI/CP/TreeFunctions.h"

class CKnownPreferences;
class CPartPreferences;
struct Category_Struct;

#ifdef LANCAST // NEO: NLC - [NeoLanCast] -- Xanatos -->

class CPPgLancast : public CPropertyPage
{
	DECLARE_DYNAMIC(CPPgLancast)

public:
	CPPgLancast();
	virtual ~CPPgLancast();

	// NEO: FCFG - [FileConfiguration]
	void SetFiles(const CSimpleArray<CObject*>* paFiles) { m_paFiles = paFiles; m_bDataChanged = true;}
	void SetCategory(Category_Struct* Category) {m_Category = Category; m_bDataChanged = true;}
	// NEO: FCFG END

	// NEO: FCFG - [FileConfiguration]
	void	GetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs, bool OperateData = false);
	void	SetFilePreferences(CKnownPreferences* KnownPrefs, CPartPreferences* PartPrefs, bool OperateData = false);
	// NEO: FCFG END

// Dialog Data
	enum { IDD = IDD_PPG_MOD };

protected:
	// NEO: FCFG - [FileConfiguration]
	//CString m_strCaption;
	const CSimpleArray<CObject*>* m_paFiles;
	Category_Struct* m_Category;
	bool m_bDataChanged;
	uint32 m_timer;
	static LPCTSTR sm_pszNotAvail;
	// NEO: FCFG END

	CTreeOptionsCtrl m_ctrlTreeOptions; // NEO - [TreeControl] <-- Xanatos --
	bool m_bInitializedTreeOpts;

	HTREEITEM m_htiLancastUpload;
		HTREEITEM m_htiMaxLanUpload;
		HTREEITEM m_htiLanUploadSlots;
			HTREEITEM m_htiMaxLanUploadSlots;
		HTREEITEM m_htiLanUploadBufferSize;
	HTREEITEM m_htiLancastDownload;
		HTREEITEM m_htiMaxLanDownload;
		HTREEITEM m_htiLanDownloadBufferSize;
	HTREEITEM m_htiCustomLanCastAdapter;
		HTREEITEM m_htiCustomLanCastMask;
	HTREEITEM m_htiLancast;
		HTREEITEM m_htiLancastDisable;
		HTREEITEM m_htiLancastEnable;
		HTREEITEM m_htiLancastDefault;
		HTREEITEM m_htiLancastGlobal;
		HTREEITEM m_htiLancastEnabled;
		HTREEITEM m_htiCustomizedLanCast;
			HTREEITEM m_htiLanCastPort;
		HTREEITEM m_htiLANIntervals;
		HTREEITEM m_htiLanCastReask;
			HTREEITEM m_htiLanReaskIntervals;
			HTREEITEM m_htiNnPLanReaskIntervals;
		HTREEITEM m_htiAutoBroadcastLanFiles;
		HTREEITEM m_htiUseLanMultiTransfer;
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	HTREEITEM m_htiVoodoo;
		HTREEITEM m_htiVoodooTransferDisable;
		HTREEITEM m_htiVoodooTransferEnable;
		HTREEITEM m_htiVoodooTransferDefault;
		HTREEITEM m_htiVoodooTransferGlobal;
		HTREEITEM m_htiUseVoodooTransfer;
		HTREEITEM m_htiSlaveAllowed;
		HTREEITEM m_htiSlaveHosting;
		HTREEITEM m_htiAutoConnectVoodoo;
		HTREEITEM m_htiUseVirtualVoodooFiles;
		HTREEITEM m_htiVoodooSourceExchange;
			HTREEITEM m_htiVoodooSourceExchangeDisable;
			HTREEITEM m_htiVoodooSourceExchangeEnable1;
			HTREEITEM m_htiVoodooSourceExchangeEnable2;
			HTREEITEM m_htiVoodooSourceExchangeDefault;
			HTREEITEM m_htiVoodooSourceExchangeGlobal;
		HTREEITEM m_htiVoodooSpell;
		HTREEITEM m_htiVoodooPort;
		HTREEITEM m_htiVoodooCastEnabled;
			HTREEITEM m_htiSearchForSlaves;
			HTREEITEM m_htiSearchForMaster;
		HTREEITEM m_htiHideVoodooFiles; // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

	bool	m_bLancastEnabled;

	int		m_iMaxLanDownload;
	bool	m_bSetLanDownloadBuffer;
	int		m_iLanDownloadBufferSize;

	int 	m_iMaxLanUpload;
	bool	m_bSetLanUploadBuffer;
	int		m_iLanUploadBufferSize;

	int		m_iMaxLanUploadSlots;

	bool	m_bCustomizedLanCast;
	CString	m_sLanCastGroup;
	int		m_uLanCastPort;

	bool	m_bCustomLanCastAdapter;
	DWORD	m_uLanCastAdapterIPAdress;
	DWORD	m_uLanCastAdapterSubNet;

	SrbtC	m_EnableLanCast;
	SintD	m_LcIntervals;

	SintD	m_LanSourceReaskTime;
	SintD	m_LanNNPSourceReaskTime;

	bool	m_bAutoBroadcastLanFiles;
	SintD	m_iAutoBroadcastLanFiles;

	bool	m_bUseLanMultiTransfer;

#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	bool	m_bUseVoodooTransfer;
	bool	m_bSlaveAllowed;
	bool	m_bSlaveHosting;
	SrbtC	m_EnableVoodoo;

	CString	m_sVoodooSpell;
	int		m_nVoodooPort;

	UINT	m_uAutoConnectVoodoo;
	SintD	m_iVoodooReconectTime;

	UINT	m_uUseVirtualVoodooFiles;

	SrbtC	m_VoodooXS;

	UINT	m_uVoodooCastEnabled;

	UINT	m_uSearchForSlaves;
	UINT	m_uSearchForMaster;
	SintD	m_iVoodooSearchIntervals;

	bool	m_bHideVoodooFiles; // NEO: PP - [PasswordProtection]
#endif // VOODOO // NEO: VOODOO END

	void ClearAllMembers();

	//void Localize();
	void LoadSettings();
	void RefreshData(); // NEO: FCFG - [FileConfiguration]

	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	virtual BOOL OnKillActive();
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
	afx_msg LRESULT OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT DrawTreeItemHelp(WPARAM wParam, LPARAM lParam);
	afx_msg void OnHelp();
	afx_msg BOOL OnHelpInfo(HELPINFO* pHelpInfo);

	void CheckEnable();

private:
	void SetLimits();

	void SetTreeRadioEx(HTREEITEM &htiDisable, HTREEITEM &htiEnable, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
#ifdef VOODOO // NEO: VOODOO - [UniversalPartfileInterface]
	void CPPgLancast::SetTreeRadioXs(HTREEITEM &htiDisable, HTREEITEM &htiEnable1, HTREEITEM &htiEnable2, HTREEITEM &htiDefault, HTREEITEM &htiGlobal, HTREEITEM &htiParent, SrbtC Value);
#endif // VOODOO // NEO: VOODOO END
};

#endif //LANCAST // NEO: NLC END <-- Xanatos --