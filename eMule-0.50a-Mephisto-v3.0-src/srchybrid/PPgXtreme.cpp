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

void CPPgXtreme::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CPPgXtreme, CPropertyPage)
	ON_BN_CLICKED(IDC_13RATIO, OnSettingsChange)	//Xman 1:3 Ratio
	ON_BN_CLICKED(IDC_SHOWBLOCKINGRATIO, OnSettingsChange) //Xman count block/success send
	ON_BN_CLICKED(IDC_DROPBLOCKINGSOCKETS, OnSettingsChange) //Xman count block/success send
	ON_BN_CLICKED(IDC_NAFCFULLCONTROL, OnBnClickedNafcfullcontrol)
	ON_BN_CLICKED(IDC_RETRYCONNECTIONATTEMPTS, OnSettingsChange)	//Xman 
	ON_EN_CHANGE(IDC_MTU_EDIT, OnSettingsChange)
	ON_BN_CLICKED(IDC_USEDOUBLESENDSIZE, OnSettingsChange)
	ON_BN_CLICKED(IDC_RETRIEVEMTUFROMSOCKET, OnSettingsChange) // netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	ON_BN_CLICKED(IDC_MULTIQUEUE, OnSettingsChange) // Maella -One-queue-per-file- (idea bloodymad)
	ON_BN_CLICKED(IDC_OPENMORESLOTS, OnOpenMoreSlots)
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDC_HIGHPRIO_RADIO, OnBnClickedrioRadio)
	ON_BN_CLICKED(IDC_ABOVENORMALPRIO_RADIO, OnBnClickedrioRadio)
	ON_BN_CLICKED(IDC_NORMALPRIO_RADIO, OnBnClickedrioRadio)
	ON_BN_CLICKED(IDC_SENDBUFFER1, OnSettingsChange)
	ON_BN_CLICKED(IDC_SENDBUFFER2, OnSettingsChange)
	ON_BN_CLICKED(IDC_SENDBUFFER3, OnSettingsChange)
	ON_BN_CLICKED(IDC_SENDBUFFER4, OnSettingsChange) //zz_fly :: support 24k send buffer
	//Xman chunk chooser
	ON_BN_CLICKED(IDC_CC_MAELLA, OnSettingsChange)
	ON_BN_CLICKED(IDC_CC_ZZ, OnSettingsChange)
	ON_BN_CLICKED(IDC_AUTOUPDATEIPFILTER, OnSettingsChange) //Xman auto update IPFilter
	// ==> Superior Client Handling [Stulle] - Stulle
	/*
	ON_BN_CLICKED(IDC_ONERELEASESLOT, OnSettingsChange)
	*/
	// <== Superior Client Handling [Stulle] - Stulle
	ON_BN_CLICKED(IDC_A_UPPRIO, OnSettingsChange) //Xman advanced upload-priority
END_MESSAGE_MAP()

// CPPgXtreme message handlers

BOOL CPPgXtreme::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	((CSliderCtrl*)GetDlgItem(IDC_SAMPLERATESLIDER))->SetRange(1,20,true);

	LoadSettings();
	Localize();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgXtreme::LoadSettings(void)
{
		
		CString buffer;
		//Xman 1:3 Ratio
		CheckDlgButton(IDC_13RATIO, thePrefs.Is13Ratio());
		//Xman end

		// ==> Superior Client Handling [Stulle] - Stulle
		/*
		//Xman always one release-slot
		CheckDlgButton(IDC_ONERELEASESLOT, thePrefs.UseReleasseSlot());
		//Xman end
		*/
		// <== Superior Client Handling [Stulle] - Stulle

		//Xman advanced upload-priority
		CheckDlgButton(IDC_A_UPPRIO, thePrefs.UseAdvancedAutoPtio());
		//Xman end

		//Xman chunk chooser
		if(thePrefs.GetChunkChooseMethod()==1)
			CheckDlgButton(IDC_CC_MAELLA,TRUE);
		else
			CheckDlgButton(IDC_CC_ZZ,TRUE);
		//Xman end

		//Xman auto update IPFilter
		// ==> Advanced Updates [MorphXT/Stulle] - Stulle
		/*
		CheckDlgButton(IDC_AUTOUPDATEIPFILTER, thePrefs.AutoUpdateIPFilter());
		*/
		// <== Advanced Updates [MorphXT/Stulle] - Stulle
		//Xman end

		CheckDlgButton(IDC_OPENMORESLOTS, thePrefs.m_openmoreslots);

		//Xman count block/success send
		CheckDlgButton(IDC_SHOWBLOCKINGRATIO, thePrefs.ShowBlockRatio());
		// ==> Mephisto Upload - Mephisto
		/*
		if(!IsDlgButtonChecked(IDC_OPENMORESLOTS))
		{
			CheckDlgButton(IDC_DROPBLOCKINGSOCKETS, FALSE);
			GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(FALSE);
		}
		else
		*/
		// <== Mephisto Upload - Mephisto
		{
			GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(TRUE);
			CheckDlgButton(IDC_DROPBLOCKINGSOCKETS, thePrefs.DropBlockingSockets());
		}
		//Xman end

		CheckDlgButton(IDC_NAFCFULLCONTROL, thePrefs.GetNAFCFullControl());

		CheckDlgButton(IDC_RETRYCONNECTIONATTEMPTS, thePrefs.retryconnectionattempts); //Xman 

		GetDlgItem(IDC_SENDBUFFER3)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SENDBUFFER4)->ShowWindow(SW_HIDE);

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
			GetDlgItem(IDC_SENDBUFFER4)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_SENDBUFFER3)->ShowWindow(SW_HIDE);
			CheckDlgButton(IDC_SENDBUFFER4, TRUE);				
			break;
		//zz_fly :: support 24k send buffer :: end
		default:
			CheckDlgButton(IDC_SENDBUFFER2, TRUE);
		}

		// Maella -One-queue-per-file- (idea bloodymad)
		CheckDlgButton(IDC_MULTIQUEUE, thePrefs.GetEnableMultiQueue());

		//Xman process prio
		processprio=thePrefs.GetMainProcessPriority();
		switch (thePrefs.GetWindowsVersion())
		{
		case _WINVER_98_:
		case _WINVER_95_:	
		case _WINVER_ME_:
			GetDlgItem(IDC_ABOVENORMALPRIO_RADIO)->EnableWindow(false);
			break;
		default: 
			GetDlgItem(IDC_ABOVENORMALPRIO_RADIO)->EnableWindow(true);
		}
		switch(processprio)
		{
		case NORMAL_PRIORITY_CLASS:
			CheckDlgButton(IDC_NORMALPRIO_RADIO,true);
			break;
		case ABOVE_NORMAL_PRIORITY_CLASS:
			CheckDlgButton(IDC_ABOVENORMALPRIO_RADIO,true);
			break;
		case HIGH_PRIORITY_CLASS:
			CheckDlgButton(IDC_HIGHPRIO_RADIO,true);
			break;
		}
		//Xman end


		buffer.Format(_T("%u"), thePrefs.GetMTU());
		GetDlgItem(IDC_MTU_EDIT)->SetWindowText(buffer);

		CheckDlgButton(IDC_USEDOUBLESENDSIZE, thePrefs.usedoublesendsize);
		// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
		CheckDlgButton(IDC_RETRIEVEMTUFROMSOCKET, thePrefs.retrieveMTUFromSocket);
		if(thePrefs.GetWindowsVersion() >= _WINVER_VISTA_)
			GetDlgItem(IDC_RETRIEVEMTUFROMSOCKET)->EnableWindow(true);
		else
			GetDlgItem(IDC_RETRIEVEMTUFROMSOCKET)->EnableWindow(false);
		// netfinity: end
		buffer.Format(GetResString(IDS_AVG_DATARATE_TIME),thePrefs.GetDatarateSamples());
		GetDlgItem(IDC_AVG_DATARATE_TIME)->SetWindowText(buffer);

		((CSliderCtrl*)GetDlgItem(IDC_SAMPLERATESLIDER))->SetPos(thePrefs.GetDatarateSamples());
}

BOOL CPPgXtreme::OnApply()
{
	
	//UpdateData(false);

	//Xman process prio
	thePrefs.SetMainProcessPriority(processprio);
	SetPriorityClass(GetCurrentProcess(), thePrefs.GetMainProcessPriority());
	//Xman end

	//zz_fly :: support 24k send buffer :: start
	if(GetDlgItem(IDC_SENDBUFFER4)->IsWindowVisible() && !(IsDlgButtonChecked(IDC_SENDBUFFER4)))
	{
		GetDlgItem(IDC_SENDBUFFER3)->ShowWindow(SW_SHOW);
		GetDlgItem(IDC_SENDBUFFER4)->ShowWindow(SW_HIDE);
	}
	//zz_fly :: support 24k send buffer :: end

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


	//Xman 1:3 Ratio
	thePrefs.Set13Ratio(IsDlgButtonChecked(IDC_13RATIO)!=0);
	//Xman end

	// ==> Superior Client Handling [Stulle] - Stulle
	/*
	//Xman always one release-slot
	thePrefs.SetUseReleaseSlot(IsDlgButtonChecked(IDC_ONERELEASESLOT)!=0);
	//Xman end
	*/
	// <== Superior Client Handling [Stulle] - Stulle

	//Xman advanced upload-priority
	bool tempupprio=IsDlgButtonChecked(IDC_A_UPPRIO)!=0;
	if(tempupprio!=thePrefs.UseAdvancedAutoPtio())
		tempupprio=true;
	else
		tempupprio=false;
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

	//Xman auto update IPFilter
	// ==> Advanced Updates [MorphXT/Stulle] - Stulle
	/*
	thePrefs.SetAutoUpdateIPFilter(IsDlgButtonChecked(IDC_AUTOUPDATEIPFILTER)!=0);
	*/
	// <== Advanced Updates [MorphXT/Stulle] - Stulle
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

	thePrefs.SetDatarateSamples((uint8)((CSliderCtrl*)GetDlgItem(IDC_SAMPLERATESLIDER))->GetPos());

	{
		CString buffer;
		GetDlgItem(IDC_MTU_EDIT)->GetWindowText(buffer);
		uint16 MTU = (uint16)_tstoi(buffer);
		if(MTU > 1500) MTU = 1500;
		if(MTU < 500) MTU = 500;
		thePrefs.SetMTU(MTU);
	}
	
	thePrefs.usedoublesendsize=IsDlgButtonChecked(IDC_USEDOUBLESENDSIZE)!=0;
	// netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
	thePrefs.retrieveMTUFromSocket = IsDlgButtonChecked(IDC_RETRIEVEMTUFROMSOCKET)!=0;

	LoadSettings();
	SetModified(FALSE);


	return CPropertyPage::OnApply();
}

void CPPgXtreme::Localize(void)
{	
	if(m_hWnd)
	{
		CString buffer;

		GetDlgItem(IDC_UPLOADMANAGEMENT)->SetWindowText(GetResString(IDS_UPLOADMANAGEMENT_FRAME));

		//Xman 1:3 Ratio
		GetDlgItem(IDC_13RATIO)->SetWindowText(GetResString(IDS_13RATIO));

		// ==> Superior Client Handling [Stulle] - Stulle
		/*
		//Xman always one release-slot
		GetDlgItem(IDC_ONERELEASESLOT)->SetWindowText(GetResString(IDS_ONERELEASESLOT));
		*/
		// <== Superior Client Handling [Stulle] - Stulle

		//Xman advanced upload-priority
		GetDlgItem(IDC_A_UPPRIO)->SetWindowText(GetResString(IDS_A_UPPRIO));

		//Xman chunk chooser
		GetDlgItem(IDC_CHUNKCHOOSER)->SetWindowText(GetResString(IDS_CHUNKCHOOSER));

		//Xman count block/success send
		GetDlgItem(IDC_SHOWBLOCKINGRATIO)->SetWindowText(GetResString(IDS_SHOWBLOCKINGRATIO));
		GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->SetWindowText(GetResString(IDS_DROPBLOCKINGSOCKETS));

		//Xman auto update IPFilter
		// ==> Advanced Updates [MorphXT/Stulle] - Stulle
		/*
		GetDlgItem(IDC_AUTOUPDATEIPFILTER)->SetWindowText(GetResString(IDS_AUTOUPDATEIPFILTER));
		*/
		GetDlgItem(IDC_AUTOUPDATEIPFILTER)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_AUTOUPDATEIPFILTER)->EnableWindow(FALSE);
		// <== Advanced Updates [MorphXT/Stulle] - Stulle


		GetDlgItem(IDC_RETRYCONNECTIONATTEMPTS)->SetWindowText(GetResString(IDS_RETRYCONNECTIONATTEMPTS)); //Xman 

		//Xman process prio
		GetDlgItem(IDC_HIGHPRIO_RADIO)->SetWindowText(GetResString(IDS_HIGHPRIOPROCESS));
		GetDlgItem(IDC_ABOVENORMALPRIO_RADIO)->SetWindowText(GetResString(IDS_ABOVENORMALPROCESS));
		GetDlgItem(IDC_NORMALPRIO_RADIO)->SetWindowText(GetResString(IDS_NORMALPROCESS));
		GetDlgItem(IDC_PROCESSPRIO_STATIC)->SetWindowText(GetResString(IDS_PROCESSGROUP));
		//Xman end

		// Maella -One-queue-per-file- (idea bloodymad)
		GetDlgItem(IDC_MULTIQUEUE)->SetWindowText(GetResString(IDS_MULTIQUEUE));

		GetDlgItem(IDC_USEDOUBLESENDSIZE)->SetWindowText(GetResString(IDS_USEDOUBLESENDSIZE));
		// ==> Mephisto Upload - Mephisto
		GetDlgItem(IDC_USEDOUBLESENDSIZE)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_USEDOUBLESENDSIZE)->EnableWindow(FALSE);
		// <== Mephisto Upload - Mephisto
		GetDlgItem(IDC_RETRIEVEMTUFROMSOCKET)->SetWindowText(GetResString(IDS_RETRIEVEMTUFROMSOCKET)); // netfinity: Maximum Segment Size (MSS - Vista only) //added by zz_fly
		GetDlgItem(IDC_STATIC_MTU)->SetWindowText(GetResString(IDS_PPG_MAELLA_MTU_STATIC));
		GetDlgItem(IDC_OPENMORESLOTS)->SetWindowText(GetResString(IDS_OPENMORESLOTS));
		// ==> Mephisto Upload - Mephisto
		GetDlgItem(IDC_OPENMORESLOTS)->ShowWindow(SW_HIDE);
		GetDlgItem(IDC_OPENMORESLOTS)->EnableWindow(FALSE);
		// <== Mephisto Upload - Mephisto
		GetDlgItem(IDC_NAFCFULLCONTROL)->SetWindowText(GetResString(IDS_PPG_MAELLA_NAFC_CHECK02));
		buffer.Format(GetResString(IDS_AVG_DATARATE_TIME),thePrefs.GetDatarateSamples());
		GetDlgItem(IDC_AVG_DATARATE_TIME)->SetWindowText(buffer);

		GetDlgItem(IDC_SENDBUFFER_STATIC)->SetWindowText(GetResString(IDS_SENDBUFFER));
	}
}


void CPPgXtreme::OnBnClickedNafcfullcontrol()
{
	OnSettingsChange();
}

void CPPgXtreme::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar) 
{
	SetModified(TRUE);

	int pos=((CSliderCtrl*)GetDlgItem(IDC_SAMPLERATESLIDER))->GetPos();
	CString buffer;
	buffer.Format(GetResString(IDS_AVG_DATARATE_TIME),pos);
	GetDlgItem(IDC_AVG_DATARATE_TIME)->SetWindowText(buffer);

	CPropertyPage::OnHScroll(nSBCode, nPos, pScrollBar);
}

//Xman process prio
void CPPgXtreme::OnBnClickedrioRadio()
{
	SetModified(true);
	if(IsDlgButtonChecked(IDC_NORMALPRIO_RADIO))
		processprio=NORMAL_PRIORITY_CLASS;
	else if(IsDlgButtonChecked(IDC_ABOVENORMALPRIO_RADIO))
		processprio=ABOVE_NORMAL_PRIORITY_CLASS;
	else if(IsDlgButtonChecked(IDC_HIGHPRIO_RADIO))
		processprio=HIGH_PRIORITY_CLASS;
	
}
//Xman end


void CPPgXtreme::OnOpenMoreSlots()
{
	// ==> Mephisto Upload - Mephisto
	/*
	if(!IsDlgButtonChecked(IDC_OPENMORESLOTS))
	{
		CheckDlgButton(IDC_DROPBLOCKINGSOCKETS, FALSE);
		GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(FALSE);
	}
	else
	*/
	// <== Mephisto Upload - Mephisto
		GetDlgItem(IDC_DROPBLOCKINGSOCKETS)->EnableWindow(TRUE);

	OnSettingsChange();
}
