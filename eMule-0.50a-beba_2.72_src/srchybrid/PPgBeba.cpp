// Tux: others: beba prefs [start]
//this file is part of eMule beba
//Copyright (C)2005-2010 Tuxman ( der_tuxman@arcor.de / http://tuxproject.de)
//
//This program is free software; you can redistribute it and/or
//modify it under the terms of the GNU General Public License
//as published by the Free Software Foundation; either
//version 2 of the License, or (at your option) any later version.
//
//This program is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#include "stdafx.h"
#include "emule.h"
#include "PPgBeba.h"
#include "UserMsgs.h"
// Tux: Feature: Auto Hard Limit [start]
#include "PartFile.h"
#include "DownloadQueue.h"
// Tux: Feature: Auto Hard Limit [end]
// Tux: Improvement: repaint prefs wnd if needed :-) [start]
#include "eMuleDlg.h"
#include "PreferencesDlg.h"
// Tux: Improvement: repaint prefs wnd if needed :-) [end]

IMPLEMENT_DYNAMIC(CPPgBeba, CPropertyPage)
CPPgBeba::CPPgBeba()

	: CPropertyPage(CPPgBeba::IDD)
	, m_ctrlTreeOptions(theApp.m_iDfltImageListColorFlags)
{
	m_bInitializedTreeOpts = false;
	m_htiEmuSettings = NULL;	// Tux: Feature: Emulate others
	m_htiCreditSystem = NULL;	// Tux: Feature: Analyzer CS
	m_htiAutoHL = NULL;			// Tux: Feature: Auto Hard Limit 
	m_htiUploadTweaks = NULL;
	m_htiQueue = NULL;
	m_htiDownloadTweaks = NULL;
	m_htiReaskTweaks = NULL;
	m_htiMiscTweaks = NULL;

	// Tux: Feature: Emulate others [start]
	m_htiEmuMLDonkey = NULL;
	m_bEmuMLDonkey = false;
	m_htiEmueDonkey = NULL;
	m_bEmueDonkey = false;
	m_htiEmueDonkeyHybrid = NULL;
	m_bEmueDonkeyHybrid = false;
	m_htiEmuShareaza = NULL;
	m_bEmuShareaza = false;
	m_htiEmuLphant = NULL;
	m_bEmuLphant = false;
	// Tux: Feature: Emulate others [end]

	// Tux: Feature: Analyzer CS [start]
	m_htiCSOfficial = NULL;
	m_htiCSAnalyzer = NULL;
	m_bCreditSystem = 0;
	// Tux: Feature: Analyzer CS [end]
	// Tux: Feature: Payback First [start]
	m_htiPaybackFirstRadio = NULL;
	m_htiPaybackDisabled = NULL;
	m_htiPaybackConst = NULL;
	m_htiPaybackVar = NULL;
	m_htiPaybackAuto = NULL;
	m_iPaybackFirst = 0;
	// Tux: Feature: Payback First [end]

	// Tux: Feature: Auto Hard Limit [start]
	m_htiUseAutoHLRadio = NULL;
	m_htiUseAutoHLPerFile = NULL;
	m_htiUseAutoHLOn = NULL;
	m_htiUseAutoHLOff = NULL;
	m_iUseAutoHL = 0;
	m_htiMaxSources = NULL;
	m_iMaxSources = 0;
	m_htiMaxPerFileSources = NULL;
	m_iMaxPerFileSources = 0;
	m_htiMinPerFileSources = NULL;
	m_iMinPerFileSources = 0;
	m_htiAHLUpdate = NULL;
	m_iAHLUpdate = 0;
	// Tux: Feature: Auto Hard Limit [end]

	// Tux: Feature: Snarl notifications [start]
	m_htiSnarlDisabled = NULL;
	m_bSnarlDisabled = false;
	// Tux: Feature: Snarl notifications [end]
	// Tux: Feature: Probabilistic Queue [start]
	m_htiRandQueue = NULL;
	m_bRandQueue = false;
	// Tux: Feature: Probabilistic Queue [end]
	// Tux: Feature: Infinite Queue [start]
	m_htiInfiniteQueue = NULL;
	m_bInfiniteQueue = false;
	// Tux: Feature: Infinite Queue [end]
	
	// Tux: Feature: Release Bonus [start]
	m_htiReleaseBonus = NULL;
	m_htiReleaseBonusEnabled = NULL;
	m_bReleaseBonusEnabled = false;
	m_htiReleaseBonusFactor = NULL;
	m_iReleaseBonusFactor = 0;
	// Tux: Feature: Release Bonus [end]

	// Tux: Feature: SLS [start]
	m_htiSLS = NULL;
	m_htiUseSLS = NULL;
	m_bUseSaveLoadSources = false;
	m_htiSLSlimit = NULL;
	m_iSLSlimit = 0;
	m_htiSLSnumber = NULL;
	m_iSLSnumber = 0;
	// Tux: Feature: SLS [end]
	// Tux: Feature: Reask sources after IP change [start]
	m_htiReaskSrcAfterIPChange = NULL;
	m_bReaskSrcAfterIPChange = false;
	m_htiReaskFileSrc = NULL;
	m_iReaskFileSrc = 0;
	// Tux: Feature: Reask sources after IP change [end]
	// Tux: Feature: Relative Priority [start]
	m_htiRelativePriority = NULL;
	m_htiRelativePrioAutoSet = NULL;
	m_bRelativePrioAutoSet = false;
	m_htiRelativePrioAutoTime = NULL;
	m_iRelativePrioAutoTime = 0;
	// Tux: Feature: Relative Priority [end]
	// Tux: Feature: Filename disparity check [start]
	m_htiFDC = NULL;
	m_htiNamecheckenabled = NULL;
	m_bNamecheckenabled = false;
	m_htiFDCMode = NULL;
	m_htiNamecheckIcon = NULL;
	m_htiNamecheckColored = NULL;
	m_iNamecheckmode = 0;
	// Tux: Feature: Filename disparity check [end]
	
	// Tux: Feature: IntelliFlush [start]
	m_htiSystemGroup = NULL;
	m_htiFileBufferGroup = NULL;
	m_htiFileBufferSize = NULL;
	m_htiFileBufferTime = NULL;
	m_htiIntelliFlushCheck = NULL;
	m_iFileBufferSize = 0;
	m_iFileBufferTime = 0;
	m_bIntelliFlush = false;
	// Tux: Feature: IntelliFlush [end]	
	// Tux: Feature: Automatic shared files updater [start]
	m_htiDirWatcher = NULL;
	m_bDirWatcher = false;
	// Tux: Feature: Automatic shared files updater [end]
	
	bReopenPrefs = false;	// Tux: Improvement: repaint prefs wnd if needed :-)
}

void CPPgBeba::DoDataExchange(CDataExchange* pDX){
	CPropertyPage::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_BEBASETTINGS, m_ctrlTreeOptions);
	CString buffer;	// Tux: Feature: IntelliFlush

	if (!m_bInitializedTreeOpts) {
		int iImgEmuSettings = 8;	// Tux: Feature: Emulate others
		int iImgCreditSystems = 8;
		int iImgPaybackFirst = 8;	// Tux: Feature: Payback First
		int iImgHardLimit = 8;		// Tux: Feature: Auto Hard Limit
		int iImgUploadTweaks = 8;
		int iImgQueue = 8;
		int iImgReleaseBonus = 8;	// Tux: Feature: Release Bonus
		int iImgDownloadTweaks = 8;
		int iImgMiscTweaks = 8;
		int iImgSLS = 8;		// Tux: Feature: SLS
		int iImgReaskTweaks = 8;
		int iImgFDC = 8;	// Tux: Feature: Filename Disparity Check
		int iImgFDCMode = 8;	// Tux: Feature: Filename Disparity Check
		int iImgRelativePriority = 8;	// Tux: Feature: Relative Priority
		int iImgFileBuffer = 8;	// Tux: Feature: IntelliFlush
		CImageList* piml = m_ctrlTreeOptions.GetImageList(TVSIL_NORMAL);
		if (piml) {
			iImgEmuSettings = piml->Add(CTempIconLoader(_T("EMULATE")));		// Tux: Feature: Emulate others
			iImgCreditSystems = piml->Add(CTempIconLoader(_T("CLIENTSKNOWN")));
			iImgPaybackFirst = piml->Add(CTempIconLoader(_T("UP1DOWN0")));		// Tux: Feature: Payback First
			iImgHardLimit = piml->Add(CTempIconLoader(_T("HARDLIMIT")));		// Tux: Feature: Auto Hard Limit
			iImgUploadTweaks = piml->Add(CTempIconLoader(_T("UPLOAD")));
			iImgQueue = piml->Add(CTempIconLoader(_T("LISTADD")));
			iImgReleaseBonus = piml->Add(CTempIconLoader(_T("RELEASEBONUS")));	// Tux: Feature: Release Bonus
			iImgDownloadTweaks = piml->Add(CTempIconLoader(_T("DOWNLOAD")));
			iImgSLS = piml->Add(CTempIconLoader(_T("SLS")));					// Tux: Feature: SLS
			iImgReaskTweaks = piml->Add(CTempIconLoader(_T("REASK")));
			iImgFDC = piml->Add(CTempIconLoader(_T("RATING_FAKE")));			// Tux: Feature: Filename Disparity Check
			iImgFDCMode = piml->Add(CTempIconLoader(_T("DISSIMILAR_NAME")));	// Tux: Feature: Filename Disparity Check
			iImgRelativePriority = piml->Add(CTempIconLoader(_T("PRIORITY")));	// Tux: Feature: Relative Priority
			iImgMiscTweaks = piml->Add(CTempIconLoader(_T("MISCTWEAKS")));
			iImgFileBuffer = piml->Add(CTempIconLoader(_T("HARDDISK")));		// Tux: Feature: IntelliFlush
		}

		// Tux: Feature: Emulate others [start]
		m_htiEmuSettings = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_EMULATE_SETTINGS), iImgEmuSettings, TVI_ROOT);
		m_htiEmuMLDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_MLDONKEY), m_htiEmuSettings, m_bEmuMLDonkey);
		m_htiEmueDonkey = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_EDONKEY), m_htiEmuSettings, m_bEmueDonkey);
		m_htiEmueDonkeyHybrid = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_EDONKEYHYBRID), m_htiEmuSettings, m_bEmueDonkeyHybrid);
		m_htiEmuShareaza = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_SHAREAZA), m_htiEmuSettings, m_bEmuShareaza);
		m_htiEmuLphant = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_EMULATE_LPHANT), m_htiEmuSettings, m_bEmuLphant);
		// Tux: Feature: Emulate others [end]

		m_htiCreditSystem = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_CREDIT_SYSTEM), iImgCreditSystems, TVI_ROOT);
		// Tux: Feature: Analyzer CS [start]
		m_htiCSOfficial = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_OFFICIAL_CREDIT), m_htiCreditSystem, m_bCreditSystem == 1);
		m_htiCSAnalyzer = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ANALYZER_CREDIT), m_htiCreditSystem, m_bCreditSystem == 2);
		// Tux: Feature: Analyzer CS [end]
		// Tux: Feature: Payback First [start]
		m_htiPaybackFirstRadio = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_PAYBACK_FIRST), iImgPaybackFirst, m_htiCreditSystem);
		m_htiPaybackDisabled = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_DISABLED), m_htiPaybackFirstRadio, m_iPaybackFirst == 0);
		m_htiPaybackConst = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PAYBACK_FIRST_CONST), m_htiPaybackFirstRadio, m_iPaybackFirst == 1);
		m_htiPaybackVar = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PAYBACK_FIRST_VAR), m_htiPaybackFirstRadio, m_iPaybackFirst == 2);
		m_htiPaybackAuto = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PAYBACK_FIRST_AUTO), m_htiPaybackFirstRadio, m_iPaybackFirst == 3);
		// Tux: Feature: Payback First [end]

		// Tux: Feature: Auto Hard Limit [start]
		m_htiAutoHL = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_AUTOHL), iImgHardLimit, TVI_ROOT);
		m_htiUseAutoHLRadio = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_USE_AUTOHL), iImgHardLimit, m_htiAutoHL);
		m_htiUseAutoHLPerFile = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_PER_FILE), m_htiUseAutoHLRadio, m_iUseAutoHL == 0);
		m_htiUseAutoHLOn = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_OFF), m_htiUseAutoHLRadio, m_iUseAutoHL == 1);
		m_htiUseAutoHLOff = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ON), m_htiUseAutoHLRadio, m_iUseAutoHL == 2);
		m_htiMaxSources = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAX_SOURCES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoHL);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxSources, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMaxPerFileSources = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MAX_PERFILESOURCES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoHL);
		m_ctrlTreeOptions.AddEditBox(m_htiMaxPerFileSources, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiMinPerFileSources = m_ctrlTreeOptions.InsertItem(GetResString(IDS_MIN_PERFILESOURCES), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoHL);
		m_ctrlTreeOptions.AddEditBox(m_htiMinPerFileSources, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiAHLUpdate = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AHL_UPDATETIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiAutoHL);
		m_ctrlTreeOptions.AddEditBox(m_htiAHLUpdate, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// Tux: Feature: Auto Hard Limit [end]

		m_htiUploadTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_UPLOAD_TWEAKS), iImgUploadTweaks, TVI_ROOT);
		m_htiQueue = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_QUEUE_TWEAKS), iImgQueue, m_htiUploadTweaks);
		m_htiRandQueue = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RANDQUEUE), m_htiQueue, m_bRandQueue);		// Tux: Feature: Probabilistic Queue
		m_htiInfiniteQueue = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_INFINITEQUEUE), m_htiQueue, m_bInfiniteQueue);	// Tux: Feature: Infinite Queue
		// Tux: Feature: Release Bonus [start]
		m_htiReleaseBonus = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RELEASE_BONUS), iImgReleaseBonus, m_htiUploadTweaks);
		m_htiReleaseBonusEnabled = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_RELEASE_BONUS_ENB), m_htiReleaseBonus, m_bReleaseBonusEnabled);
		m_htiReleaseBonusFactor = m_ctrlTreeOptions.InsertItem(GetResString(IDS_RELEASE_BONUS_FAC), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiReleaseBonus);
		m_ctrlTreeOptions.AddEditBox(m_htiReleaseBonusFactor, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// Tux: Feature: Release Bonus [end]

		m_htiDownloadTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_DOWNLOAD_TWEAKS), iImgDownloadTweaks, TVI_ROOT);
		// Tux: Feature: SLS [start]
		m_htiSLS = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_SLS_PREFS), iImgSLS, m_htiDownloadTweaks);
		m_htiUseSLS = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_USE_SLS), m_htiSLS, m_bUseSaveLoadSources);
		m_htiSLSlimit = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SLS_LIMIT), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSLS);
		m_ctrlTreeOptions.AddEditBox(m_htiSLSlimit, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiSLSnumber = m_ctrlTreeOptions.InsertItem(GetResString(IDS_SLS_NUMBER), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiSLS);
		m_ctrlTreeOptions.AddEditBox(m_htiSLSnumber, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// Tux: Feature: SLS [end]
		// Tux: Feature: Reask sources after IP change [start]
		m_htiReaskTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_REASK_TWEAKS), iImgReaskTweaks, m_htiDownloadTweaks);
		m_htiReaskSrcAfterIPChange = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_REASK_SRC_AFTER_IP_CHANGE), m_htiReaskTweaks, m_bReaskSrcAfterIPChange);
		m_htiReaskFileSrc = m_ctrlTreeOptions.InsertItem(GetResString(IDS_REASK_FILE_SRC), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiReaskTweaks);
		m_ctrlTreeOptions.AddEditBox(m_htiReaskFileSrc, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// Tux: Feature: Reask sources after IP change [end]
		// Tux: Feature: Relative Priority [start]
		m_htiRelativePriority = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_RELATIVE_PRIORITY), iImgRelativePriority, m_htiDownloadTweaks);
		m_htiRelativePrioAutoSet = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_AUTORELATIVEPRIO), m_htiRelativePriority, m_bRelativePrioAutoSet);
		m_htiRelativePrioAutoTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_AUTORELATIVEPRIOPREF), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiRelativePriority);
		m_ctrlTreeOptions.AddEditBox(m_htiRelativePrioAutoTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		// Tux: Feature: Relative Priority [end]
		// Tux: Feature: Filename disparity check [start]
		m_htiFDC = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FDC), iImgFDC, m_htiDownloadTweaks);
		m_htiNamecheckenabled = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_NAMECHECKENABLED), m_htiFDC, m_bNamecheckenabled);
		m_htiFDCMode = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FDC_MODE), iImgFDCMode, m_htiFDC);
		m_htiNamecheckIcon = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_ICONMODE), m_htiFDCMode, m_iNamecheckmode == 0);
		m_htiNamecheckColored = m_ctrlTreeOptions.InsertRadioButton(GetResString(IDS_COLORMODE), m_htiFDCMode, m_iNamecheckmode == 1);
		// Tux: Feature: Filename disparity check [end]
		
		m_htiMiscTweaks = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_MISC_TWEAKS), iImgMiscTweaks, TVI_ROOT);
		// Tux: Feature: IntelliFlush [start]
		m_htiFileBufferGroup = m_ctrlTreeOptions.InsertGroup(GetResString(IDS_FILEBUFFER), iImgFileBuffer, m_htiMiscTweaks);
		buffer.Format(_T("%s [%s]"), GetResString(IDS_FILEBUFFERSIZE), GetResString(IDS_KBYTES));
		m_htiFileBufferSize = (thePrefs.m_bExtControls ? m_ctrlTreeOptions.InsertItem(buffer, TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFileBufferGroup) : NULL);	// Tux: Improvement: hide more extended controls
		if (thePrefs.m_bExtControls)	// Tux: Improvement: hide more extended controls
			m_ctrlTreeOptions.AddEditBox(m_htiFileBufferSize, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiFileBufferTime = m_ctrlTreeOptions.InsertItem(GetResString(IDS_FILEBUFFER_TIME), TREEOPTSCTRLIMG_EDIT, TREEOPTSCTRLIMG_EDIT, m_htiFileBufferGroup);
		m_ctrlTreeOptions.AddEditBox(m_htiFileBufferTime, RUNTIME_CLASS(CNumTreeOptionsEdit));
		m_htiIntelliFlushCheck = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_FILEBUFFER_INTELLI), m_htiFileBufferGroup, m_bIntelliFlush || m_iFileBufferTime < 1 || m_iFileBufferTime > 5);
		// Tux: Feature: IntelliFlush [end]
		m_htiDirWatcher = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DIR_WATCHER), m_htiMiscTweaks, m_bDirWatcher);	// Tux: Feature: Automatic shared files updater
		m_htiSnarlDisabled = m_ctrlTreeOptions.InsertCheckBox(GetResString(IDS_DONT_USE_SNARL), m_htiMiscTweaks, m_bSnarlDisabled);	// Tux: Feature: Snarl notifications
		
		// Tux: Improvement: Auto-expanding [start]
		m_ctrlTreeOptions.Expand(m_htiUseAutoHLRadio, TVE_EXPAND);	// Tux: Feature: Auto Hard Limit
		m_ctrlTreeOptions.Expand(m_htiFDCMode, TVE_EXPAND);	// Tux: Feature: Filename disparity check
		if (!thePrefs.m_bExtControls)	// Tux: Improvement: hide more extended controls
			m_ctrlTreeOptions.Expand(m_htiFileBufferGroup, TVE_EXPAND);	// Tux: Feature: IntelliFlush
		// Tux: Improvement: Auto-expanding [end]
		
		m_ctrlTreeOptions.SendMessage(WM_VSCROLL, SB_TOP);
		m_bInitializedTreeOpts = true;

		// Tux: Improvement: Bold options [start]
		m_ctrlTreeOptions.SetItemState(m_htiEmuSettings, TVIS_BOLD, TVIS_BOLD);		// Tux: Feature: Emulate others
		m_ctrlTreeOptions.SetItemState(m_htiCreditSystem, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiPaybackFirstRadio, TVIS_BOLD, TVIS_BOLD); // Tux: Feature: Payback First
		m_ctrlTreeOptions.SetItemState(m_htiAutoHL, TVIS_BOLD, TVIS_BOLD);			// Tux: Feature: Auto Hard Limit
		m_ctrlTreeOptions.SetItemState(m_htiUseAutoHLRadio, TVIS_BOLD, TVIS_BOLD);	// Tux: Feature: Auto Hard Limit
		m_ctrlTreeOptions.SetItemState(m_htiUploadTweaks, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiQueue, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiReleaseBonus, TVIS_BOLD, TVIS_BOLD);	// Tux: Feature: Release Bonus
		m_ctrlTreeOptions.SetItemState(m_htiDownloadTweaks, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiSLS, TVIS_BOLD, TVIS_BOLD);				// Tux: Feature: SLS
		m_ctrlTreeOptions.SetItemState(m_htiReaskTweaks, TVIS_BOLD, TVIS_BOLD);		// Tux: Feature: Reask sources after IP change
		m_ctrlTreeOptions.SetItemState(m_htiRelativePriority, TVIS_BOLD, TVIS_BOLD);// Tux: Feature: Relative Priority
		m_ctrlTreeOptions.SetItemState(m_htiFDC, TVIS_BOLD, TVIS_BOLD);// Tux: Feature: Filename disparity check
		m_ctrlTreeOptions.SetItemState(m_htiFDCMode, TVIS_BOLD, TVIS_BOLD);// Tux: Feature: Filename disparity check
		m_ctrlTreeOptions.SetItemState(m_htiMiscTweaks, TVIS_BOLD, TVIS_BOLD);
		m_ctrlTreeOptions.SetItemState(m_htiFileBufferGroup, TVIS_BOLD, TVIS_BOLD);	// Tux: Feature: IntelliFlush
		// Tux: Improvement: Bold options [end]
	}

	// Tux: Feature: Emulate others [start]
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiEmuMLDonkey, m_bEmuMLDonkey);
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiEmueDonkey, m_bEmueDonkey);
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiEmueDonkeyHybrid, m_bEmueDonkeyHybrid);
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiEmuShareaza, m_bEmuShareaza);
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiEmuLphant, m_bEmuLphant);
	// Tux: Feature: Emulate others [end]

	DDX_TreeRadio(pDX, IDC_BEBASETTINGS, m_htiCreditSystem, m_bCreditSystem);	// Tux: Feature: Analyzer CS
	DDX_TreeRadio(pDX, IDC_BEBASETTINGS, m_htiPaybackFirstRadio, m_iPaybackFirst);	// Tux: Feature: Payback First

	// Tux: Feature: Auto Hard Limit [start]
	DDX_TreeRadio(pDX, IDC_BEBASETTINGS, m_htiUseAutoHLRadio, m_iUseAutoHL); 
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiMaxSources, m_iMaxSources);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiMaxPerFileSources, m_iMaxPerFileSources);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiMinPerFileSources, m_iMinPerFileSources);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiAHLUpdate, m_iAHLUpdate);
	DDV_MinMaxInt(pDX, m_iMaxSources, 1500, _UI16_MAX);
	DDV_MinMaxInt(pDX, m_iMaxPerFileSources, 150, _UI16_MAX);
	DDV_MinMaxInt(pDX, m_iMinPerFileSources, 25, _UI16_MAX);
	DDV_MinMaxInt(pDX, m_iAHLUpdate, 10, 600);
	// Tux: Feature: Auto Hard Limit [end]

	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiRandQueue, m_bRandQueue);		// Tux: Feature: Probabilistic Queue
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiInfiniteQueue, m_bInfiniteQueue);	// Tux: Feature: Infinite Queue
	// Tux: Feature: Release Bonus [start]
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiReleaseBonusEnabled, m_bReleaseBonusEnabled);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiReleaseBonusFactor, m_iReleaseBonusFactor);
	DDV_MinMaxInt(pDX, m_iReleaseBonusFactor, 1, 32);
	// Tux: Feature: Release Bonus [end]

	// Tux: Feature: SLS [start]
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiUseSLS, m_bUseSaveLoadSources);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiSLSlimit, m_iSLSlimit);
	DDV_MinMaxInt(pDX, m_iSLSlimit, 1, 200);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiSLSnumber, m_iSLSnumber);
	DDV_MinMaxInt(pDX, m_iSLSnumber, 1, 100);
	// Tux: Feature: SLS [end]
	// Tux: Feature: Reask sources after IP change [start]
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiReaskSrcAfterIPChange, m_bReaskSrcAfterIPChange);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiReaskFileSrc, m_iReaskFileSrc);
	DDV_MinMaxInt(pDX, m_iReaskFileSrc, 29, 55);
	// Tux: Feature: Reask sources after IP change [end]
	// Tux: Feature: Relative Priority [start]
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiRelativePrioAutoSet, m_bRelativePrioAutoSet);
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiRelativePrioAutoTime, m_iRelativePrioAutoTime);
	DDV_MinMaxInt(pDX, m_iRelativePrioAutoTime, 10, _UI16_MAX);
	// Tux: Feature: Relative Priority [end]
	// Tux: Feature: Filename disparity check [start]
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiNamecheckenabled, m_bNamecheckenabled);
	DDX_TreeRadio(pDX, IDC_BEBASETTINGS, m_htiFDCMode, m_iNamecheckmode);
	// Tux: Feature: Filename disparity check [end]
	
	// Tux: Feature: IntelliFlush [start]
	if (thePrefs.m_bExtControls) {	// Tux: Improvement: hide more extended controls
		DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiFileBufferSize, m_iFileBufferSize);
		DDV_MinMaxInt(pDX, m_iFileBufferSize, 256, 30*1024); //kB!
	}	// Tux: Improvement: hide more extended controls
	DDX_TreeEdit(pDX, IDC_BEBASETTINGS, m_htiFileBufferTime, m_iFileBufferTime);
	DDV_MinMaxInt(pDX, m_iFileBufferTime, 0, 60); //minutes!
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiIntelliFlushCheck, m_bIntelliFlush);	
	if (m_iFileBufferTime < 1 || m_iFileBufferTime > 5) {
		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiIntelliFlushCheck, FALSE);
		m_ctrlTreeOptions.SetCheckBox(m_htiIntelliFlushCheck, TRUE);
	}
	else {
		m_ctrlTreeOptions.SetCheckBoxEnable(m_htiIntelliFlushCheck, TRUE);
		m_ctrlTreeOptions.SetCheckBox(m_htiIntelliFlushCheck, m_bIntelliFlush);
	}
	// Tux: Feature: IntelliFlush [end]
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiDirWatcher, m_bDirWatcher);	// Tux: Feature: Automatic shared files updater
	DDX_TreeCheck(pDX, IDC_BEBASETTINGS, m_htiSnarlDisabled, m_bSnarlDisabled);		// Tux: Feature: Snarl notifications
}

BEGIN_MESSAGE_MAP(CPPgBeba, CPropertyPage)
	ON_WM_HSCROLL()
	ON_WM_DESTROY()

	ON_MESSAGE(UM_TREEOPTSCTRL_NOTIFY, OnTreeOptsCtrlNotify)
	ON_NOTIFY(NM_CUSTOMDRAW, IDC_SLIDERFDC, OnNMCustomdrawSliderfdc)	// Tux: Feature: Filename disparity check
	ON_BN_CLICKED(IDC_PRF_UP_SLOTCTRL_OMSIN, OnSettingsChange)	// Tux: Feature: Slot Control
END_MESSAGE_MAP()

BOOL CPPgBeba::OnInitDialog()
{
	CString strBuffer;

	// Tux: Feature: Emulate others [start]
	m_bEmuMLDonkey = thePrefs.IsEmuMLDonkey();
	m_bEmueDonkey = thePrefs.IsEmueDonkey();
	m_bEmueDonkeyHybrid = thePrefs.IsEmueDonkeyHybrid();
	m_bEmuShareaza = thePrefs.IsEmuShareaza();
	m_bEmuLphant = thePrefs.IsEmuLphant();
	// Tux: Feature: Emulate others [end]

	m_bCreditSystem = thePrefs.UseCreditSystem();
	m_iPaybackFirst = thePrefs.GetPaybackFirst();	// Tux: Feature: Payback First

	// Tux: Feature: Auto Hard Limit [start]
	m_iUseAutoHL = thePrefs.IsUseAutoHL()+1;
	m_iMaxSources = thePrefs.GetMaxSourcesHL();
	m_iMaxPerFileSources = thePrefs.GetMaxAutoHL();
	m_iMinPerFileSources = thePrefs.GetMinAutoHL();
	m_iAHLUpdate = thePrefs.GetAutoHLUpdateTimer();
	// Tux: Feature: Auto Hard Limit [end]

	m_bRandQueue = thePrefs.GetRandQueue();		// Tux: Feature: Probabilistic Queue
	m_bInfiniteQueue = thePrefs.infiniteQueue;	// Tux: Feature: Infinite Queue
	// Tux: Feature: Release Bonus [start]
	m_bReleaseBonusEnabled = thePrefs.IsReleaseBonus();
	m_iReleaseBonusFactor = thePrefs.GetReleaseBonus();
	// Tux: Feature: Release Bonus [end]

	// Tux: Feature: SLS [start]
	m_bUseSaveLoadSources = thePrefs.UseSaveLoadSources();
	m_iSLSlimit = thePrefs.GetActivationLimitSLS();
	m_iSLSnumber = thePrefs.GetSourcesToSaveSLS();
	// Tux: Feature: SLS [end]
	// Tux: Feature: Reask sources after IP change [start]
	m_bReaskSrcAfterIPChange = thePrefs.GetReaskSrcAfterIPChange();
	m_iReaskFileSrc = (thePrefs.GetReaskTimeDif() + FILEREASKTIME)/60000;
	// Tux: Feature: Reask sources after IP change [end]
	// Tux: Feature: Relative Priority [start]
	m_bRelativePrioAutoSet = thePrefs.GetRelativePrioAutoSet();
	m_iRelativePrioAutoTime = thePrefs.GetRelativePrioAutoTimeP();
	// Tux: Feature: Relative Priority [end]
	// Tux: Feature: Filename disparity check [start]
	m_bNamecheckenabled = thePrefs.GetNamecheckenabled();
	m_iNamecheckmode = thePrefs.GetPaintFDCRed();
	// Tux: Feature: Filename disparity check [end]
	
	// Tux: Feature: IntelliFlush [start]
	m_iFileBufferSize = thePrefs.GetFileBufferSize() / 1024; //kB!
	m_iFileBufferTime = thePrefs.GetFileBufferFlushTime() / 60 / 1000; //minutes!
	m_bIntelliFlush = m_iFileBufferTime < 1 || m_iFileBufferTime > 5 || thePrefs.IsUseIntelliFlush();
	// Tux: Feature: IntelliFlush [end]
	m_bDirWatcher = thePrefs.GetDirectoryWatcher();	// Tux: Feature: Automatic shared files updater
	m_bSnarlDisabled = thePrefs.IsSnarlDisabled();	// Tux: Feature: Snarl notifications

	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);
	m_ctrlTreeOptions.SetImageListColorFlags(theApp.m_iDfltImageListColorFlags);

	LoadSettings();
	Localize();

	return TRUE;
}

void CPPgBeba::LoadSettings(void)
{
	if (m_hWnd)
	{
		// Tux: Feature: Slot Control [start]
		uint32 MaxUpSpeed = thePrefs.maxupload;
		uint32 MinSlots = (uint32)(sqrtl(MaxUpSpeed)-1);
		if (MaxUpSpeed != UNLIMITED)
			((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->SetRange(MinSlots < 1 ? 1 : (MinSlots > 4 ? 4 : MinSlots), MaxUpSpeed/3, TRUE);
		else
			((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->SetRange(1, 255, TRUE);
		((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->SetPos(thePrefs.GetUpSlots()-1);
		ShowUpSlotValues();

		if (thePrefs.maxupload != UNLIMITED) {
			GetDlgItem(IDC_PRF_UP_SLOTCTRL_OMSIN)->EnableWindow(true);
			CheckDlgButton(IDC_PRF_UP_SLOTCTRL_OMSIN, thePrefs.GetOpenMoreSlots());
		}
		else {
			GetDlgItem(IDC_PRF_UP_SLOTCTRL_OMSIN)->EnableWindow(false);
			CheckDlgButton(IDC_PRF_UP_SLOTCTRL_OMSIN, false);
		}
		// Tux: Feature: Slot Control [end]

		// Tux: Feature: Filename disparity check [start]
		CString strBuffer;
		fdcspos = 100 - thePrefs.FDCSensitivity; //convert the max percentage mismatch to percent match so the slider goes the right way
		CSliderCtrl* fdcslider = (CSliderCtrl*)GetDlgItem(IDC_SLIDERFDC);
		fdcslider->SetRange(6,30,false);
		fdcslider->SetPos(fdcspos);
		
		OnFDCChange();
		// Tux: Feature: Filename disparity check [end]
	}
}

BOOL CPPgBeba::OnApply(){
	CString sBuffer;

	m_ctrlTreeOptions.HandleChildControlLosingFocus();

	// Tux: Feature: Emulate others [start]
	thePrefs.m_bEmuMLDonkey = m_bEmuMLDonkey;
	thePrefs.m_bEmueDonkey = m_bEmueDonkey;
	thePrefs.m_bEmueDonkeyHybrid = m_bEmueDonkeyHybrid;
	thePrefs.m_bEmuShareaza = m_bEmuShareaza;
	thePrefs.m_bEmuLphant = m_bEmuLphant;
	// Tux: Feature: Emulate others [end]

	thePrefs.SetCreditSystem((uint8)m_bCreditSystem);	// Tux: Feature: Analyzer CS
	thePrefs.SetPaybackFirst((uint8)m_iPaybackFirst);	// Tux: Feature: Payback First

	// Tux: Feature: Auto Hard Limit [start]
	if (thePrefs.IsUseAutoHL() != m_iUseAutoHL - 1)
	{
		thePrefs.SetUseAutoHL(m_iUseAutoHL - 1);
		if (thePrefs.IsUseAutoHL() == 0) // globally disabled
		{
			for (UINT i = 0; i < (UINT)theApp.downloadqueue->GetFileCount(); i++)
			{
				CPartFile* file = theApp.downloadqueue->GetFileByIndex(i);
				file->SetPrivateMaxSources(thePrefs.GetMaxSourcePerFileDefault());
			}
		}
	}
	thePrefs.SetMaxSourcesHL((uint16)m_iMaxSources);
	thePrefs.SetMaxAutoHL((uint16)m_iMaxPerFileSources);
	thePrefs.SetMinAutoHL((uint16)m_iMinPerFileSources);
	thePrefs.SetAutoHLUpdateTimer((uint16)m_iAHLUpdate);
	// Tux: Feature: Auto Hard Limit [end]

	// Tux: Feature: Slot Control [start]
	thePrefs.SetUpSlots((uint32)((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->GetPos()+1);
	thePrefs.SetOpenMoreSlots(IsDlgButtonChecked(IDC_PRF_UP_SLOTCTRL_OMSIN) ? true : false);
	// Tux: Feature: Slot Control [end]
	thePrefs.m_bRandQueue = m_bRandQueue;		// Tux: Feature: Probabilistic Queue
	// Tux: Feature: Infinite Queue [start]
	if (thePrefs.infiniteQueue != m_bInfiniteQueue)
		bReopenPrefs = true;	// Tux: Improvement: repaint prefs wnd if needed :-)
	thePrefs.infiniteQueue = m_bInfiniteQueue;
	// Tux: Feature: Infinite Queue [end]

	// Tux: Feature: Release Bonus [start]
	thePrefs.m_bReleaseBonusEnabled = m_bReleaseBonusEnabled;
	thePrefs.m_iReleaseBonusFactor = m_iReleaseBonusFactor;
	// Tux: Feature: Release Bonus [end]

	// Tux: Feature: SLS [start]
	if (m_bUseSaveLoadSources) {
		thePrefs.SetSaveLoadSource(true);
		thePrefs.SetActivationLimitSLS((uint8)m_iSLSlimit);
		thePrefs.SetSourcesToSaveSLS((uint8)m_iSLSnumber);
	}
	else
		thePrefs.SetSaveLoadSource(false);
	// Tux: Feature: SLS [end]
	// Tux: Feature: Reask sources after IP change [start]
	thePrefs.m_bReaskSrcAfterIPChange = m_bReaskSrcAfterIPChange;
	thePrefs.m_uReaskTimeDif = (m_iReaskFileSrc-29)*60000;
	// Tux: Feature: Reask sources after IP change [end]
	// Tux: Feature: Relative Priority [start]
	thePrefs.m_bRelativePrioAutoSet = m_bRelativePrioAutoSet;
	thePrefs.m_iRelativePrioAutoTime = (uint16)m_iRelativePrioAutoTime;
	// Tux: Feature: Relative Priority [end]
	// Tux: Feature: Filename disparity check [start]
	thePrefs.m_bNamecheckenabled = m_bNamecheckenabled;
	thePrefs.m_bPaintFDCRed = (m_iNamecheckmode == 1 ? true : false);
	thePrefs.FDCSensitivity = 100 - fdcspos;
	OnFDCChange();
	SetModified(TRUE);
	// Tux: Feature: Filename disparity check [end]
	
	// Tux: Feature: IntelliFlush [start]
	thePrefs.SetFileBufferSize(m_iFileBufferSize*1024);
	thePrefs.SetFileBufferFlushTime(m_iFileBufferTime*60*1000);
	thePrefs.SetUseIntelliFlush(m_bIntelliFlush || m_iFileBufferTime < 1 || m_iFileBufferTime > 5);
	// Tux: Feature: IntelliFlush [end]
	// Tux: Feature: Automatic shared files updater [start]
	if (thePrefs.m_bDirectoryWatcher != m_bDirWatcher) {
		thePrefs.m_bDirectoryWatcher = m_bDirWatcher;
		theApp.ResetDirectoryWatcher();
	}
	// Tux: Feature: Automatic shared files updater [end]
	thePrefs.SetSnarlDisabled(m_bSnarlDisabled);		// Tux: Feature: Snarl notifications
	
	// Tux: Improvement: repaint prefs wnd if needed :-) [start]
	if (bReopenPrefs) {
		bReopenPrefs = false;
		theApp.emuledlg->preferenceswnd->CloseDialog();
		theApp.emuledlg->preferenceswnd->OpenDialog();
	}
	// Tux: Improvement: repaint prefs wnd if needed :-) [end]

	SetModified();
	return CPropertyPage::OnApply();
}

BOOL CPPgBeba::OnKillActive()
{
	m_ctrlTreeOptions.HandleChildControlLosingFocus();
	return CPropertyPage::OnKillActive();
}

void CPPgBeba::Localize(void)
{
	if (m_hWnd)
	{
		CString buffer;	// Tux: Feature: IntelliFlush
		SetWindowText(GetResString(IDS_PW_BEBA));
		
		// Tux: Feature: Emulate others [start]
		if (m_htiEmuSettings)      m_ctrlTreeOptions.SetItemText(m_htiEmuSettings, GetResString(IDS_EMULATE_SETTINGS));
		if (m_htiEmuMLDonkey)      m_ctrlTreeOptions.SetItemText(m_htiEmuMLDonkey, GetResString(IDS_EMULATE_MLDONKEY));
		if (m_htiEmueDonkey)       m_ctrlTreeOptions.SetItemText(m_htiEmueDonkey, GetResString(IDS_EMULATE_EDONKEY));
		if (m_htiEmueDonkeyHybrid) m_ctrlTreeOptions.SetItemText(m_htiEmueDonkeyHybrid, GetResString(IDS_EMULATE_EDONKEYHYBRID));
		if (m_htiEmuShareaza)      m_ctrlTreeOptions.SetItemText(m_htiEmuShareaza, GetResString(IDS_EMULATE_SHAREAZA));
		if (m_htiEmuLphant)        m_ctrlTreeOptions.SetItemText(m_htiEmuLphant, GetResString(IDS_EMULATE_LPHANT));
		// Tux: Feature: Emulate others [end]

		// Tux: Feature: Auto Hard Limit [start]
		if (m_htiAutoHL)            m_ctrlTreeOptions.SetItemText(m_htiAutoHL, GetResString(IDS_AUTOHL));
		if (m_htiUseAutoHLRadio)    m_ctrlTreeOptions.SetItemText(m_htiUseAutoHLRadio, GetResString(IDS_USE_AUTOHL));
		if (m_htiUseAutoHLPerFile)  m_ctrlTreeOptions.SetItemText(m_htiUseAutoHLPerFile, GetResString(IDS_PER_FILE));
		if (m_htiUseAutoHLOn)       m_ctrlTreeOptions.SetItemText(m_htiUseAutoHLOn, GetResString(IDS_OFF));
		if (m_htiUseAutoHLOff)      m_ctrlTreeOptions.SetItemText(m_htiUseAutoHLOff, GetResString(IDS_ON));
		if (m_htiMaxSources)        m_ctrlTreeOptions.SetEditLabel(m_htiMaxSources, GetResString(IDS_MAX_SOURCES));
		if (m_htiMaxPerFileSources) m_ctrlTreeOptions.SetEditLabel(m_htiMaxPerFileSources, GetResString(IDS_MAX_PERFILESOURCES));
		if (m_htiMinPerFileSources) m_ctrlTreeOptions.SetEditLabel(m_htiMinPerFileSources, GetResString(IDS_MIN_PERFILESOURCES));
		if (m_htiAHLUpdate)         m_ctrlTreeOptions.SetEditLabel(m_htiAHLUpdate, GetResString(IDS_AHL_UPDATETIME));
		// Tux: Feature: Auto Hard Limit [end]
		
		// Tux: Feature: Payback First [start]
		if (m_htiPaybackFirstRadio) m_ctrlTreeOptions.SetItemText(m_htiPaybackFirstRadio, GetResString(IDS_PAYBACK_FIRST));
		if (m_htiPaybackDisabled)   m_ctrlTreeOptions.SetItemText(m_htiPaybackDisabled, GetResString(IDS_DISABLED));
		if (m_htiPaybackConst)      m_ctrlTreeOptions.SetItemText(m_htiPaybackConst, GetResString(IDS_PAYBACK_FIRST_CONST));
		if (m_htiPaybackVar)        m_ctrlTreeOptions.SetItemText(m_htiPaybackVar, GetResString(IDS_PAYBACK_FIRST_VAR));
		if (m_htiPaybackAuto)       m_ctrlTreeOptions.SetItemText(m_htiPaybackAuto, GetResString(IDS_PAYBACK_FIRST_AUTO));
		// Tux: Feature: Payback First

		// Tux: Feature: Slot Control [start]
		GetDlgItem(IDC_PRF_UP_SLOTCTRL_LBL)->SetWindowText(GetResString(IDS_PRF_UP_SLOTCTRL_LBL));
		GetDlgItem(IDC_PRF_UP_SLOTCTRL_DRPC_LBL)->SetWindowText(GetResString(IDS_PRF_UP_SLOTCTRL_DRPC_LBL));
		GetDlgItem(IDC_PRF_UP_SLOTCTRL_OMSIN)->SetWindowText(GetResString(IDS_PRF_UP_SLOTCTRL_OMSIN));
		// Tux: Feature: Slot Control [end]
		
		// Tux: Feature: Filename disparity check [start]
		GetDlgItem(IDC_SLIDER_LABEL)->SetWindowText(GetResString(IDS_FDC_SENSITIVITY));
		GetDlgItem(IDC_STATIC_LOW)->SetWindowText(GetResString(IDS_PRIOLOW));
		GetDlgItem(IDC_STATIC_HIGH)->SetWindowText(GetResString(IDS_PRIOHIGH));
		// Tux: Feature: Filename disparity check [end]
		
		// Tux: Feature: IntelliFlush [start]
		if (m_htiFileBufferGroup)   m_ctrlTreeOptions.SetItemText(m_htiFileBufferGroup, GetResString(IDS_FILEBUFFER));
		if (m_htiFileBufferSize) {
			buffer.Format(L"%s [%s]", GetResString(IDS_FILEBUFFERSIZE), GetResString(IDS_KBYTES));
			m_ctrlTreeOptions.SetEditLabel(m_htiFileBufferSize, buffer);
		}
		if (m_htiFileBufferTime)    m_ctrlTreeOptions.SetEditLabel(m_htiFileBufferTime, GetResString(IDS_FILEBUFFER_TIME));
		if (m_htiIntelliFlushCheck) m_ctrlTreeOptions.SetItemText(m_htiIntelliFlushCheck, GetResString(IDS_FILEBUFFER_INTELLI));
		// Tux: Feature: IntelliFlush [end]
		
		GetDlgItem(IDC_WARNING_BEBA)->SetWindowText(GetResString(IDS_WARNING_BEBA));	// Tux: others: information label
	}
}

CPPgBeba::~CPPgBeba(void)
{
}

LRESULT CPPgBeba::OnTreeOptsCtrlNotify(WPARAM wParam, LPARAM /*lParam*/)
{
	if (wParam == IDC_BEBASETTINGS)
		SetModified();
	return 0;
}

void CPPgBeba::OnHScroll(UINT /*nSBCode*/, UINT /*nPos*/, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);
	CString temp;

	// Tux: Feature: Slot Control [start]
	if (pScrollBar == (CScrollBar*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))
		ShowUpSlotValues();
	// Tux: Feature: Slot Control [end]
}

void CPPgBeba::OnDestroy()
{
	m_ctrlTreeOptions.DeleteAllItems();
	m_ctrlTreeOptions.DestroyWindow();
	m_bInitializedTreeOpts = false;

	m_htiEmuSettings = NULL;
	m_htiCreditSystem = NULL;
	m_htiAutoHL = NULL;
	m_htiUploadTweaks = NULL;
	m_htiQueue = NULL;
	m_htiDownloadTweaks = NULL;
	m_htiReaskTweaks = NULL;
	m_htiMiscTweaks = NULL;

	// Tux: Feature: Emulate others [start]
	m_htiEmuMLDonkey = NULL;
	m_bEmuMLDonkey = false;
	m_htiEmueDonkey = NULL;
	m_bEmueDonkey = false;
	m_htiEmueDonkeyHybrid = NULL;
	m_bEmueDonkeyHybrid = false;
	m_htiEmuShareaza = NULL;
	m_bEmuShareaza = false;
	m_htiEmuLphant = NULL;
	m_bEmuLphant = false;
	// Tux: Feature: Emulate others [end]

	// Tux: Feature: Analyzer CS [start]
	m_htiCSOfficial = NULL;
	m_htiCSAnalyzer = NULL;
	m_bCreditSystem = 0;
	// Tux: Feature: Analyzer CS [end]
	// Tux: Feature: Payback First [start]
	m_htiPaybackFirstRadio = NULL;
	m_htiPaybackDisabled = NULL;
	m_htiPaybackConst = NULL;
	m_htiPaybackVar = NULL;
	m_htiPaybackAuto = NULL;
	m_iPaybackFirst = 0;
	// Tux: Feature: Payback First [end]

	// Tux: Feature: Auto Hard Limit [start]
	m_htiUseAutoHLRadio = NULL;
	m_htiUseAutoHLPerFile = NULL;
	m_htiUseAutoHLOn = NULL;
	m_htiUseAutoHLOff = NULL;
	m_iUseAutoHL = 0;
	m_htiMaxSources = NULL;
	m_iMaxSources = 0;
	m_htiMaxPerFileSources = NULL;
	m_iMaxPerFileSources = 0;
	m_htiMinPerFileSources = NULL;
	m_iMinPerFileSources = 0;
	m_htiAHLUpdate = NULL;
	m_iAHLUpdate = 0;
	// Tux: Feature: Auto Hard Limit [end]

	// Tux: Feature: Probabilistic Queue [start]
	m_htiRandQueue = NULL;
	m_bRandQueue = false;
	// Tux: Feature: Probabilistic Queue [end]
	// Tux: Feature: Infinite Queue [start]
	m_htiInfiniteQueue = NULL;
	m_bInfiniteQueue = false;
	// Tux: Feature: Infinite Queue [end]
	// Tux: Feature: Release Bonus [start]
	m_htiReleaseBonusEnabled = NULL;
	m_bReleaseBonusEnabled = false;
	m_htiReleaseBonusFactor = NULL;
	m_iReleaseBonusFactor = 0;
	// Tux: Feature: Release Bonus [end]

	// Tux: Feature: SLS [start]
	m_htiSLS = NULL;
	m_htiUseSLS = NULL;
	m_bUseSaveLoadSources = false;
	m_htiSLSlimit = NULL;
	m_iSLSlimit = 0;
	m_htiSLSnumber = NULL;
	m_iSLSnumber = 0;
	// Tux: Feature: SLS [end]
	// Tux: Feature: Reask sources after IP change [start]
	m_htiReaskSrcAfterIPChange = NULL;
	m_bReaskSrcAfterIPChange = false;
	m_htiReaskFileSrc = NULL;
	m_iReaskFileSrc = 0;
	// Tux: Feature: Reask sources after IP change [end]
	// Tux: Feature: Relative Priority [start]
	m_htiRelativePriority = NULL;
	m_htiRelativePrioAutoSet = NULL;
	m_bRelativePrioAutoSet = false;
	m_htiRelativePrioAutoTime = NULL;
	m_iRelativePrioAutoTime = 0;
	// Tux: Feature: Relative Priority [end]
	// Tux: Feature: Filename disparity check [start]
	m_htiFDC = NULL;
	m_htiNamecheckenabled = NULL;
	m_bNamecheckenabled = false;
	m_htiFDCMode = NULL;
	m_htiNamecheckIcon = NULL;
	m_htiNamecheckColored = NULL;
	m_iNamecheckmode = 0;
	// Tux: Feature: Filename disparity check [end]
	
	// Tux: Feature: IntelliFlush [start]
	m_htiSystemGroup = NULL;
	m_htiFileBufferGroup = NULL;
	m_htiFileBufferSize = NULL;
	m_htiFileBufferTime = NULL;
	m_htiIntelliFlushCheck = NULL;
	m_iFileBufferSize = 0;
	m_iFileBufferTime = 0;
	m_bIntelliFlush = false;
	// Tux: Feature: IntelliFlush [end]
	// Tux: Feature: Automatic shared files updater [start]
	m_htiDirWatcher = NULL;
	m_bDirWatcher = false;
	// Tux: Feature: Automatic shared files updater [end]
	// Tux: Feature: Snarl notifications [start]
	m_htiSnarlDisabled = NULL;
	m_bSnarlDisabled = false;
	// Tux: Feature: Snarl notifications [end]
	
	bReopenPrefs = false;	// Tux: Improvement: repaint prefs wnd if needed :-)

	CPropertyPage::OnDestroy();
}

// Tux: Feature: Filename disparity check [start]
void CPPgBeba::OnFDCChange()
{
	if (m_bNamecheckenabled) {
		//GetDlgItem(IDC_SLIDERFDC)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SLIDERFDC)->EnableWindow(TRUE);
		GetDlgItem(IDC_SLIDER_LABEL)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_LOW)->EnableWindow(TRUE);
		GetDlgItem(IDC_STATIC_HIGH)->EnableWindow(TRUE);
	}
	else {
		//GetDlgItem(IDC_SLIDERFDC)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_SLIDERFDC)->EnableWindow(FALSE);
		GetDlgItem(IDC_SLIDER_LABEL)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_LOW)->EnableWindow(FALSE);
		GetDlgItem(IDC_STATIC_HIGH)->EnableWindow(FALSE);
	}
}

void CPPgBeba::OnNMCustomdrawSliderfdc(NMHDR* /*pNMHDR*/, LRESULT *pResult)
{
	int newpos = ((CSliderCtrl*)GetDlgItem(IDC_SLIDERFDC))->GetPos();
	if (newpos != fdcspos) {//store new position and set modified to enable apply
		fdcspos = newpos;
		SetModified(TRUE);
	}
	*pResult = 0;
}
// Tux: Feature: Filename disparity check [end]

// Tux: Feature: Slot Control [start]
void CPPgBeba::ShowUpSlotValues()
{
	CString strBuffer;
	strBuffer.Format(_T("%i+1 LowID"), ((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->GetPos());
	GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLOTS)->SetWindowText(strBuffer);
	if (thePrefs.GetMaxUpload() == UNLIMITED) {
		if (thePrefs.maxGraphUploadRate == UNLIMITED)
			strBuffer = _T("UNLIMITED");
		else
			strBuffer.Format(_T("%s - %s"),CastItoXBytes((UINT)((thePrefs.maxGraphUploadRate*1024)/(((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->GetPos()+1))),CastItoXBytes((UINT)((thePrefs.maxGraphUploadRate*1024)/((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->GetPos())));
	}
	else
		strBuffer.Format(_T("%s - %s"),CastItoXBytes((UINT)((thePrefs.GetMaxUpload()*1024)/(((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->GetPos()+1))),CastItoXBytes((UINT)((thePrefs.GetMaxUpload()*1024)/((CSliderCtrl*)GetDlgItem(IDC_PRF_UP_SLOTCTRL_SLIDER))->GetPos())));
	GetDlgItem(IDC_PRF_UP_SLOTCTRL_DRPC)->SetWindowText(strBuffer);
}
// Tux: Feature: Slot Control [end]
// Tux: others: beba prefs [end]