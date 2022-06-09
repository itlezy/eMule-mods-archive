// PPgXtreme.cpp : implementation file
//

#include "stdafx.h"
#include "emule.h"
#include "PPgXtreme.h"
#include "emuleDlg.h"
#include "Preferences.h"
#include "OtherFunctions.h"
#include "opcodes.h"
#include "UploadQueue.h"
#include "BandWidthControl.h"
#include "SharedFileList.h" //Xman advanced upload-priority

// CPPgXtreme dialog
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

IMPLEMENT_DYNAMIC(CPPgXtreme, CPropertyPage)
CPPgXtreme::CPPgXtreme()
	: CPropertyPage(CPPgXtreme::IDD)
{
}

CPPgXtreme::~CPPgXtreme()
{
}

/*void CPPgXtreme::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}
*/
BEGIN_MESSAGE_MAP(CPPgXtreme, CPropertyPage)
	ON_BN_CLICKED(IDC_SHOWBLOCKINGRATIO, OnSettingsChange) //Xman count block/success send
	ON_BN_CLICKED(IDC_DROPBLOCKINGSOCKETS, OnSettingsChange) //Xman count block/success send
	ON_BN_CLICKED(IDC_NAFCFULLCONTROL, OnSettingsChange) // X: [CI] - [Code Improvement]
	ON_BN_CLICKED(IDC_RETRYCONNECTIONATTEMPTS, OnSettingsChange)	//Xman 
	ON_EN_CHANGE(IDC_MTU_EDIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_USEDOUBLESENDSIZE, OnSettingsChange)
	ON_BN_CLICKED(IDC_RETRIEVEMTUFROMSOCKET, OnSettingsChange) // netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	ON_BN_CLICKED(IDC_MULTIQUEUE, OnSettingsChange) // Maella -One-queue-per-file- (idea bloodymad)
	ON_BN_CLICKED(IDC_OPENMORESLOTS, OnOpenMoreSlots)
	ON_WM_HSCROLL()
	//ON_BN_CLICKED(IDC_SENDBUFFER1, OnSettingsChange)// X: [DSRB] - [Dynamic Send and Receive Buffer]
	//ON_BN_CLICKED(IDC_SENDBUFFER2, OnSettingsChange)
	//ON_BN_CLICKED(IDC_SENDBUFFER3, OnSettingsChange)
	//ON_BN_CLICKED(IDC_SENDBUFFER4, OnSettingsChange) //zz_fly :: support 24k send buffer
	//Xman chunk chooser
	ON_BN_CLICKED(IDC_CC_MAELLA, OnSettingsChange)
	ON_BN_CLICKED(IDC_CC_ZZ, OnSettingsChange)
	// ==> Superior Client Handling [Stulle] - Stulle
	/*
	ON_BN_CLICKED(IDC_ONERELEASESLOT, OnSettingsChange)
	*/
	// <== Superior Client Handling [Stulle] - Stulle
	ON_BN_CLICKED(IDC_A_UPPRIO, OnSettingsChange) //Xman advanced upload-priority
    ON_BN_CLICKED(IDC_MAINWINDOW_CHECK, OnSettingsChange)
    ON_BN_CLICKED(IDC_ACTIVEDOWNLOADSBOLD, OnSettingsChange) //Xman Show active downloads bold
	ON_BN_CLICKED(IDC_FUNNYNICK, OnSettingsChange) //Xman Funny-Nick (Stulle/Morph)
	ON_BN_CLICKED(IDC_PRF_GUI_MISC_SDL_CHK, OnSettingsChange) // X-Ray :: SessionDownload
	ON_BN_CLICKED(IDC_PRF_MINQR, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PRF_COLOR, OnSettingsChange) 
	ON_BN_CLICKED(IDC_PRF_STATUS, OnSettingsChange) 
END_MESSAGE_MAP()

// CPPgXtreme message handlers

BOOL CPPgXtreme::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	LoadSettings();
	Localize();
	m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgXtreme::LoadSettings(void)
{
	CString buffer;

		// ==> Superior Client Handling [Stulle] - Stulle
		/*
	//Xman always one release-slot
	CheckDlgButton(IDC_ONERELEASESLOT, thePrefs.UseReleasseSlot());
	//Xman end
	*/
		// <== Superior Client Handling [Stulle] - Stulle
		CheckDlgButton(IDC_MAINWINDOW_CHECK, thePrefs.GetRestoreLastMainWndDlg());

	//Xman advanced upload-priority
	CheckDlgButton(IDC_A_UPPRIO, thePrefs.UseAdvancedAutoPtio());
	//Xman end

	CheckDlgButton((thePrefs.m_chunkchooser==1)?IDC_CC_MAELLA:IDC_CC_ZZ, TRUE);

	CheckDlgButton(IDC_OPENMORESLOTS, thePrefs.m_openmoreslots);

    // X-Ray :: SessionDownload :: Start
    CheckDlgButton(IDC_PRF_GUI_MISC_SDL_CHK, thePrefs.m_bShowSessionDownload);
    // X-Ray :: SessionDownload :: End
    CheckDlgButton(IDC_PRF_MINQR, thePrefs.m_bShowMinQR);
	CheckDlgButton(IDC_PRF_COLOR, thePrefs.m_bShowDownloadColor);
	CheckDlgButton(IDC_PRF_STATUS, thePrefs.m_bShowFileStatusIcons);

	//Xman count block/success send
	CheckDlgButton(IDC_SHOWBLOCKINGRATIO, thePrefs.ShowBlockRatio());
	if(!IsDlgButtonChecked(IDC_OPENMORESLOTS))
	{
		CheckDlgButton(IDC_DROPBLOCKINGSOCKETS, FALSE);
		GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(FALSE);
	}
	else
	{
		GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(TRUE);
		CheckDlgButton(IDC_DROPBLOCKINGSOCKETS, thePrefs.DropBlockingSockets());
	}
	//Xman end

	CheckDlgButton(IDC_FUNNYNICK, thePrefs.DisplayFunnyNick()); //Xman Funny-Nick (Stulle/Morph)

	CheckDlgButton(IDC_ACTIVEDOWNLOADSBOLD, thePrefs.GetShowActiveDownloadsBold()); //Xman Show active downloads bold
	CheckDlgButton(IDC_NAFCFULLCONTROL, thePrefs.GetNAFCFullControl());

	CheckDlgButton(IDC_RETRYCONNECTIONATTEMPTS, thePrefs.retryconnectionattempts); //Xman 
// X: [DSRB] - [Dynamic Send and Receive Buffer]
/*
	switch(thePrefs.GetSendbuffersize())
	{
		case 6000:
			CheckDlgButton(IDC_SENDBUFFER1, TRUE);
			break;
		case 12000:
			CheckDlgButton(IDC_SENDBUFFER3, TRUE);
			break;
		//zz_fly :: support 24k send buffer :: start
		case 24000:
			CheckDlgButton(IDC_SENDBUFFER4, TRUE);				
			break;
		//zz_fly :: support 24k send buffer :: end
		default:
			CheckDlgButton(IDC_SENDBUFFER2, TRUE);
			break;
	}
*/
	// Maella -One-queue-per-file- (idea bloodymad)
	CheckDlgButton(IDC_MULTIQUEUE, thePrefs.GetEnableMultiQueue());

	buffer.Format(_T("%u"), thePrefs.GetMTU());
	SetDlgItemText(IDC_MTU_EDIT,buffer);

	CheckDlgButton(IDC_USEDOUBLESENDSIZE, thePrefs.usedoublesendsize);
	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	CheckDlgButton(IDC_RETRIEVEMTUFROMSOCKET, thePrefs.retrieveMTUFromSocket);
	GetDlgItem(IDC_RETRIEVEMTUFROMSOCKET)->EnableWindow(thePrefs.GetWindowsVersion() >= _WINVER_VISTA_);
	// netfinity: end
}

BOOL CPPgXtreme::OnApply()
{
	if(m_bModified) // X: [CI] - [Code Improvement] Apply if modified
	{
		//UpdateData(false);

// X: [DSRB] - [Dynamic Send and Receive Buffer]
/*
		if(IsDlgButtonChecked(IDC_SENDBUFFER1))
			thePrefs.SetSendbuffersize(6000);
		else if(IsDlgButtonChecked(IDC_SENDBUFFER2))
			thePrefs.SetSendbuffersize(8192);
		else if(IsDlgButtonChecked(IDC_SENDBUFFER3))
			thePrefs.SetSendbuffersize(12000);
		//zz_fly :: support 24k send buffer :: start
		else if(IsDlgButtonChecked(IDC_SENDBUFFER4))
			thePrefs.SetSendbuffersize(24000); 
		//zz_fly :: support 24k send buffer :: end
		theApp.uploadqueue->ChangeSendBufferSize(thePrefs.GetSendbuffersize());
*/

	// ==> Superior Client Handling [Stulle] - Stulle
	/*
		//Xman always one release-slot
		thePrefs.SetUseReleaseSlot(IsDlgButtonChecked(IDC_ONERELEASESLOT)!=0);
		//Xman end
*/
	// <== Superior Client Handling [Stulle] - Stulle
        	thePrefs.SetRestoreLastMainWndDlg(IsDlgButtonChecked(IDC_MAINWINDOW_CHECK)!=0);

		//Xman advanced upload-priority
		bool tempupprio=IsDlgButtonChecked(IDC_A_UPPRIO)!=0;
		tempupprio=(tempupprio!=thePrefs.UseAdvancedAutoPtio());
		thePrefs.SetAdvancedAutoPrio(IsDlgButtonChecked(IDC_A_UPPRIO)!=0);
		if(tempupprio)
		{
			if(thePrefs.UseAdvancedAutoPtio())
				theApp.sharedfiles->CalculateUploadPriority(true);
			else
				theApp.sharedfiles->CalculateUploadPriority_Standard();
		}
		//Xman end


		//Xman chunk chooser
		if(IsDlgButtonChecked(IDC_CC_MAELLA)!=0)
			thePrefs.m_chunkchooser=1;
		else
			thePrefs.m_chunkchooser=2;
		//Xman end

		//Xman count block/success send
		thePrefs.SetShowBlockRatio(IsDlgButtonChecked(IDC_SHOWBLOCKINGRATIO)!=0);
		thePrefs.SetDropBlockingSockets(IsDlgButtonChecked(IDC_DROPBLOCKINGSOCKETS)!=0);

		thePrefs.retryconnectionattempts=IsDlgButtonChecked(IDC_RETRYCONNECTIONATTEMPTS)!=0; //Xman 

		// Maella -One-queue-per-file- (idea bloodymad)
		thePrefs.SetEnableMultiQueue(IsDlgButtonChecked(IDC_MULTIQUEUE)!=0);

		thePrefs.m_openmoreslots=IsDlgButtonChecked(IDC_OPENMORESLOTS)!=0;

		thePrefs.SetNAFCFullControl(IsDlgButtonChecked(IDC_NAFCFULLCONTROL)!=0);
		theApp.pBandWidthControl->SetWasNAFCLastActive(thePrefs.GetNAFCFullControl()); //Xman x4.1

		{
			uint16 MTU = (uint16)GetDlgItemInt(IDC_MTU_EDIT,NULL,FALSE);
			if(MTU > 1500) MTU = 1500;
			if(MTU < 500) MTU = 500;
			thePrefs.SetMTU(MTU);
		}
		
		thePrefs.usedoublesendsize=IsDlgButtonChecked(IDC_USEDOUBLESENDSIZE)!=0;
		// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
		thePrefs.retrieveMTUFromSocket = IsDlgButtonChecked(IDC_RETRIEVEMTUFROMSOCKET)!=0;

        thePrefs.m_bShowActiveDownloadsBold=(IsDlgButtonChecked(IDC_ACTIVEDOWNLOADSBOLD)!=0); //Xman Show active downloads bold

		//Xman Funny-Nick (Stulle/Morph)
		thePrefs.SetDisplayFunnyNick(IsDlgButtonChecked(IDC_FUNNYNICK)!=0);
		//Xman end

        thePrefs.m_bShowSessionDownload = IsDlgButtonChecked(IDC_PRF_GUI_MISC_SDL_CHK)!=0;// X-Ray :: SessionDownload
        thePrefs.m_bShowMinQR = IsDlgButtonChecked(IDC_PRF_MINQR)!=0;
		thePrefs.m_bShowDownloadColor = IsDlgButtonChecked(IDC_PRF_COLOR)!=0;
		thePrefs.m_bShowFileStatusIcons = IsDlgButtonChecked(IDC_PRF_STATUS)!=0;

		LoadSettings();
		SetModified(FALSE);
		m_bModified = false; // X: [CI] - [Code Improvement] Apply if modified
	}
	return CPropertyPage::OnApply();
}

void CPPgXtreme::Localize(void)
{	
	if(m_hWnd)
	{
		CString buffer;

		SetDlgItemText(IDC_UPLOADMANAGEMENT,GetResString(IDS_UPLOADMANAGEMENT_FRAME));
		SetDlgItemText(IDC_MISC_FRM,GetResString(IDS_PW_MISC));
   
	    // ==> Superior Client Handling [Stulle] - Stulle
		/*
		//Xman always one release-slot
		SetDlgItemText(IDC_ONERELEASESLOT,GetResString(IDS_ONERELEASESLOT));
	    */
		// <== Superior Client Handling [Stulle] - Stulle
		//Xman advanced upload-priority
		SetDlgItemText(IDC_A_UPPRIO,GetResString(IDS_A_UPPRIO));

		//Xman chunk chooser
		SetDlgItemText(IDC_CHUNKCHOOSER,GetResString(IDS_CHUNKCHOOSER));

		//Xman count block/success send
		SetDlgItemText(IDC_SHOWBLOCKINGRATIO,GetResString(IDS_SHOWBLOCKINGRATIO));
		SetDlgItemText(IDC_DROPBLOCKINGSOCKETS,GetResString(IDS_DROPBLOCKINGSOCKETS));

		SetDlgItemText(IDC_RETRYCONNECTIONATTEMPTS,GetResString(IDS_RETRYCONNECTIONATTEMPTS)); //Xman 

		// Maella -One-queue-per-file- (idea bloodymad)
		SetDlgItemText(IDC_MULTIQUEUE,GetResString(IDS_MULTIQUEUE));

		SetDlgItemText(IDC_USEDOUBLESENDSIZE,GetResString(IDS_USEDOUBLESENDSIZE));
		SetDlgItemText(IDC_RETRIEVEMTUFROMSOCKET,GetResString(IDS_RETRIEVEMTUFROMSOCKET)); // netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
		SetDlgItemText(IDC_OPENMORESLOTS,GetResString(IDS_OPENMORESLOTS));
		SetDlgItemText(IDC_NAFCFULLCONTROL,GetResString(IDS_PPG_MAELLA_NAFC_CHECK02));
		
		//SetDlgItemText(IDC_SENDBUFFER_STATIC,GetResString(IDS_SENDBUFFER));// X: [DSRB] - [Dynamic Send and Receive Buffer]

        SetDlgItemText(IDC_ACTIVEDOWNLOADSBOLD,GetResString(IDS_ACTIVEDOWNLOADSBOLD));

		//Xman Funny-Nick (Stulle/Morph)
		SetDlgItemText(IDC_FUNNYNICK,GetResString(IDS_FUNNYNICK));
		//Xman end

	}
}

void CPPgXtreme::OnOpenMoreSlots()
{
	if(!IsDlgButtonChecked(IDC_OPENMORESLOTS))
	{
		CheckDlgButton(IDC_DROPBLOCKINGSOCKETS, FALSE);
		GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(FALSE);
	}
	else
		GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(TRUE);

	OnSettingsChange();
}
