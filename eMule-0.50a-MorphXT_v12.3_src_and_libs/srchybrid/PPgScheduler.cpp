//this file is part of eMule
//Copyright (C)2002-2008 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "stdafx.h"
#include "emule.h"
#include "PPGtooltipped.h" //MORPH leuk_he addded tooltipped
#include "PreferencesDlg.h"
#include "InputBox.h"
#include "emuledlg.h"
#include "Preferences.h"
#include "Scheduler.h"
#include "MenuCmds.h"
#include "HelpIDs.h"

// Mighty Knife: additional scheduling events
#include "XMessageBox.h"
// [end] Mighty Knife

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif


IMPLEMENT_DYNAMIC(CPPgScheduler, CPropertyPage)

BEGIN_MESSAGE_MAP(CPPgScheduler, CPropertyPage)
/* MORPH START  leuk_he: Remove 2nd apply in scheduler
	ON_BN_CLICKED(IDC_APPLY, OnBnClickedApply)
	 MORPH END leuk_he: Remove 2nd apply in scheduler */
	ON_BN_CLICKED(IDC_CHECKNOENDTIME, OnDisableTime2)
	ON_BN_CLICKED(IDC_ENABLE, OnEnableChange)
	ON_BN_CLICKED(IDC_NEW, OnBnClickedAdd)
	ON_BN_CLICKED(IDC_REMOVE, OnBnClickedRemove)
	ON_NOTIFY(NM_CLICK, IDC_SCHEDLIST, OnNmClickList)
	ON_NOTIFY(NM_DBLCLK, IDC_SCHEDACTION, OnNmDblClkActionlist)
	ON_NOTIFY(NM_RCLICK, IDC_SCHEDACTION, OnNmRClickActionlist)
// MORPH START  leuk_he: Remove 2nd apply in scheduler
	ON_NOTIFY(LVN_ITEMCHANGING, IDC_SCHEDLIST, OnListItemChanging)
	ON_EN_CHANGE(IDC_ENABLE, OnSettingsChange)
	ON_EN_CHANGE(IDC_ENABLE, OnSettingsChange)
	ON_EN_CHANGE(IDC_S_ENABLE, OnSettingsChange)
	ON_EN_CHANGE(IDC_S_TITLE, OnSettingsChange)
	ON_EN_CHANGE(IDC_ENABLE, OnSettingsChange)
	ON_CBN_SELCHANGE(IDC_TIMESEL, OnSettingsChange)
	ON_NOTIFY(DTN_DATETIMECHANGE,IDC_DATETIMEPICKER1, OnSettingsChangeTime)
	ON_NOTIFY(DTN_DATETIMECHANGE,IDC_DATETIMEPICKER2, OnSettingsChangeTime)
	ON_EN_CHANGE(IDC_CHECKNOENDTIME, OnSettingsChange)
// MORPH END leuk_he: Remove 2nd apply in scheduler
	ON_WM_HELPINFO()
END_MESSAGE_MAP()

CPPgScheduler::CPPgScheduler()
        : CPPgtooltipped (CPPgScheduler::IDD) 
 /*leuk_he  tooltipped 
	: CPropertyPage(CPPgScheduler::IDD)
 */ //leuk_he  tooltipped 
{
// MORPH START  leuk_he: Remove 2nd apply in scheduler
	modified=0;
	bSuppressModifications=0;
	miActiveSelection=-1;
	
// MORPH END leuk_he: Remove 2nd apply in scheduler
}

CPPgScheduler::~CPPgScheduler()
{
}

void CPPgScheduler::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TIMESEL, m_timesel);
	DDX_Control(pDX, IDC_SCHEDACTION, m_actions);
	DDX_Control(pDX, IDC_DATETIMEPICKER1, m_time);
	DDX_Control(pDX, IDC_DATETIMEPICKER2, m_timeTo);
	DDX_Control(pDX, IDC_SCHEDLIST, m_list);
}

BOOL CPPgScheduler::OnInitDialog()
{
	CPropertyPage::OnInitDialog();
	InitWindowStyles(this);

	m_list.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	ASSERT( (m_list.GetStyle() & LVS_SINGLESEL) == 0);
	m_list.InsertColumn(0, GetResString(IDS_TITLE),		LVCFMT_LEFT, 150);
	m_list.InsertColumn(1, GetResString(IDS_S_DAYS),	LVCFMT_LEFT,  80);
	m_list.InsertColumn(2, GetResString(IDS_STARTTIME), LVCFMT_LEFT,  80);
	m_time.SetFormat(_T("H:mm"));
	m_timeTo.SetFormat(_T("H:mm"));

	m_actions.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_INFOTIP);
	ASSERT( (m_actions.GetStyle() & LVS_SINGLESEL) == 0 );
	m_actions.InsertColumn(0, GetResString(IDS_ACTION), LVCFMT_LEFT, 150);
	m_actions.InsertColumn(1, GetResString(IDS_VALUE),  LVCFMT_LEFT,  80);

	InitTooltips(); //leuk_he tooltipped
	Localize();
	CheckDlgButton(IDC_ENABLE,thePrefs.IsSchedulerEnabled());
	FillScheduleList();

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CPPgScheduler::Localize(void)
{
	if(m_hWnd)
	{
		SetWindowText(GetResString(IDS_SCHEDULER));

		GetDlgItem(IDC_ENABLE)->SetWindowText(GetResString(IDS_ENABLED));
		GetDlgItem(IDC_S_ENABLE)->SetWindowText(GetResString(IDS_ENABLED));
		GetDlgItem(IDC_STATIC_S_TITLE)->SetWindowText(GetResString(IDS_TITLE));
		GetDlgItem(IDC_STATIC_DETAILS)->SetWindowText(GetResString(IDS_DETAILS));
		GetDlgItem(IDC_STATIC_S_TIME)->SetWindowText(GetResString(IDS_TIME));
		
		GetDlgItem(IDC_STATIC_S_ACTION)->SetWindowText(GetResString(IDS_ACTION));
     /* MORPH START  leuk_he: Remove 2nd apply in scheduler 
		GetDlgItem(IDC_APPLY)->SetWindowText(GetResString(IDS_PW_APPLY));
    /* MORPH START  leuk_he: Remove 2nd apply in scheduler */
		GetDlgItem(IDC_REMOVE)->SetWindowText(GetResString(IDS_REMOVE));
		GetDlgItem(IDC_NEW)->SetWindowText(GetResString(IDS_NEW));
		GetDlgItem(IDC_CHECKNOENDTIME)->SetWindowText(GetResString(IDS_CHECKNOENDTIME));
		
		while (m_timesel.GetCount()>0) m_timesel.DeleteString(0);
		for (int i=0;i<11;i++) 
			m_timesel.AddString(GetDayLabel(i));
		m_timesel.SetCurSel(0);
		if (m_list.GetSelectionMark()!=-1) m_timesel.SetCurSel(theApp.scheduler->GetSchedule(m_timesel.GetCurSel())->day);

		// leuk_he tooltipped
		SetTool(IDC_ENABLE,IDS_ENABLE_SCHED_TIP);
		SetTool(IDC_REMOVE,IDS_REMOVE_SCHED_TIP);
		SetTool(IDC_NEW,IDS_NEW_SCHED_TIP);
		SetTool(IDC_SCHEDLIST,IDS_SCHEDLIST_TIP);
		SetTool(IDC_S_ENABLE,IDS_S_ENABLE_TIP);
		SetTool(IDC_S_TITLE,IDS_S_TITLE_TIP);
		SetTool(IDC_TIMESEL,IDS_TIMESEL_SCHED_TIP);
		SetTool(IDC_DATETIMEPICKER1,IDS_DATETIMEPICKER1_TIP);
		SetTool(IDC_DATETIMEPICKER2,IDS_DATETIMEPICKER2_TIP);
		SetTool(IDC_CHECKNOENDTIME,IDS_CHECKNOENDTIME_SCHED_TIP);
		SetTool(IDC_SCHEDACTION,IDS_SCHEDACTION_TIP);
		// leuk_he tooltips end
	}
}

// MORPH START  leuk_he: Remove 2nd apply in scheduler
/*  old code:
void CPPgScheduler::OnNmClickList(NMHDR*  pNMHDR , LRESULT*  pResult )
{
	if (m_list.GetSelectionMark()>-1) LoadSchedule(m_list.GetSelectionMark());
}
 new code: */

// Handle Changes of the focus on the list.
void CPPgScheduler::OnListItemChanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	if(((NM_LISTVIEW*)pNMHDR)->uNewState & LVIS_SELECTED && !(((NM_LISTVIEW*)pNMHDR)->uOldState & LVIS_SELECTED))
		OnNmClickList(pNMHDR,pResult)	;
}

// MORPH START  leuk_he: Remove 2nd apply in scheduler continue:
void CPPgScheduler::OnNmClickList(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{  
	// If there are modifications you first need to appy before 
	// an other schedule can be select. 
	if (m_list.GetSelectionMark()>-1) 
		if (!modified) {   miActiveSelection=m_list.GetSelectionMark() ;
			               LoadSchedule(miActiveSelection);
		}
		else // reset to previous selection and tell the user once he should apply
		{   static bool WarnOnce=false;
			//m_list.SetSelectionMark(prevsel);
 	        // m_list.SetItemState(miActiveSelection,LVIS_SELECTED, LVIS_SELECTED);
			if (!WarnOnce) {
				MessageBox(GetResString(IDS_SCHED_WARN_APPLY), //Press Ok or Apply before selecting an other schedule"
					   GetResString(IDS_SCHED_WARN_APPLY_TITLE ));
				WarnOnce=true;
			}
			*pResult=TRUE;
		}
}
// MORPH END leuk_he: Remove 2nd apply in scheduler 

void CPPgScheduler::LoadSchedule(int index) {
// MORPH START  leuk_he: Remove 2nd apply in scheduler
  bSuppressModifications=true;	
  GetDlgItem(IDC_S_TITLE)->SetWindowText(_T("")); // clear
// MORPH END leuk_he: Remove 2nd apply in scheduler
	Schedule_Struct* schedule=theApp.scheduler->GetSchedule(index);
	GetDlgItem(IDC_S_TITLE)->SetWindowText(schedule->title);

	//time
	CTime time=time.GetCurrentTime();
	if (schedule->time>0) time=schedule->time;
	m_time.SetTime(&time);
	
	CTime time2=time2.GetCurrentTime();
	if (schedule->time2>0) time2=schedule->time2;
	m_timeTo.SetTime(&time2);

	//time kindof (days)
	m_timesel.SetCurSel(schedule->day);

	CheckDlgButton(IDC_S_ENABLE,(schedule->enabled));
	CheckDlgButton(IDC_CHECKNOENDTIME, schedule->time2==0);

	OnDisableTime2();

	m_actions.DeleteAllItems();
	for (int i=0;i<16;i++) {
		if (schedule->actions[i]==0) break;
		m_actions.InsertItem(i,GetActionLabel(schedule->actions[i]));
		//MORPH START - Changed by Stulle, Show cat name for scheduler cat actions
		if(schedule->actions[i]==ACTION_CATSTOP || schedule->actions[i]==ACTION_CATRESUME)
		{
			int iCat = _tstoi(schedule->values[i]);
			CString strCatTitle;
			if(iCat == -1)
				strCatTitle = GetResString(IDS_ALL);
			else
			{
				Category_Struct* thisCat = thePrefs.GetCategory(iCat);
				if(thisCat)
					strCatTitle = thisCat->strTitle;
				else
					strCatTitle = GetResString(IDS_UNKNOWN);
			}
			m_actions.SetItemText(i,1,schedule->values[i]+L" ("+strCatTitle+L")");
		}
		else
		//MORPH END   - Changed by Stulle, Show cat name for scheduler cat actions
			m_actions.SetItemText(i,1,schedule->values[i]);
		m_actions.SetItemData(i,schedule->actions[i]);
	}
// MORPH START  leuk_he: Remove 2nd apply in scheduler
	bSuppressModifications=false;	
// MORPH END leuk_he: Remove 2nd apply in scheduler
}

void CPPgScheduler::FillScheduleList() {

	m_list.DeleteAllItems();
	
// MORPH START  leuk_he: Remove 2nd apply in scheduler
    bSuppressModifications=true;
	m_actions.DeleteAllItems(); // clear
//MORPH END leuk_he: Remove 2nd apply in scheduler	
	for (uint8 index=0;index<theApp.scheduler->GetCount();index++) {
		m_list.InsertItem(index , theApp.scheduler->GetSchedule(index)->title );
		CTime time(theApp.scheduler->GetSchedule(index)->time);
		CString timeS;
		m_list.SetItemText(index, 1, GetDayLabel(theApp.scheduler->GetSchedule(index)->day));
		timeS.Format(_T("%s"),time.Format(_T("%H:%M")));
		m_list.SetItemText(index, 2, timeS);
	}
	if (m_list.GetItemCount()>0) {
		m_list.SetSelectionMark(0);
		m_list.SetItemState(0, LVIS_SELECTED, LVIS_SELECTED);
		LoadSchedule(0);
	}
// MORPH START  leuk_he: Remove 2nd apply in scheduler
	bSuppressModifications=false;
//MORPH END leuk_he: Remove 2nd apply in scheduler
}

void CPPgScheduler::OnBnClickedAdd()
{
	int index;
// MORPH START  leuk_he: Remove 2nd apply in scheduler
   if (modified) 
		{   MessageBox(GetResString(IDS_SCHED_WARN_APPLY), //Press Ok or Apply before selecting an other schedule"
					   GetResString(IDS_SCHED_WARN_APPLY_TITLE ));
            return;
		}
	m_actions.DeleteAllItems(); // clear
	
// MORPH END leuk_he: Remove 2nd apply in scheduler
	Schedule_Struct* newschedule=new Schedule_Struct();
	newschedule->day=0;
	newschedule->enabled=false;
	newschedule->time=time(NULL);
	newschedule->time2=time(NULL);
	newschedule->title=_T("?");
	newschedule->ResetActions();

	index=theApp.scheduler->AddSchedule(newschedule);
	m_list.InsertItem(index , newschedule->title );
// MORPH START  leuk_he: Remove 2nd apply in scheduler
	miActiveSelection  =index;
	GetDlgItem(IDC_S_TITLE)->SetWindowText(newschedule->title);
  miActiveSelection  =index;
	SetModified();
// MORPH END leuk_he: Remove 2nd apply in scheduler
 	m_list.SetSelectionMark(index);

	RecheckSchedules();
}

// MORPH START  leuk_he: Remove 2nd apply in scheduler
void CPPgScheduler::SetModified(bool bChanged){
	if (!bSuppressModifications)
	{   CPropertyPage::SetModified(bChanged);
		modified=bChanged;
	}
}
// MORPH END leuk_he: Remove 2nd apply in scheduler

/* MORPH START  leuk_he: Remove 2nd apply in scheduler
void CPPgScheduler::OnBnClickedApply()
{
	int index=m_list.GetSelectionMark();
*/
BOOL CPPgScheduler::OnApply(){
	int index=miActiveSelection	;
// MORPH END leuk_he: Remove 2nd apply in scheduler

	if (index>-1) {
		Schedule_Struct* schedule=theApp.scheduler->GetSchedule(index);

		//title
		GetDlgItem(IDC_S_TITLE)->GetWindowText(schedule->title);

		//time
		CTime myTime;
		DWORD result=m_time.GetTime(myTime);
		if (result == GDT_VALID){
			struct tm tmTemp;
			schedule->time=safe_mktime(myTime.GetLocalTm(&tmTemp));
		}
		CTime myTime2;
		DWORD result2=m_timeTo.GetTime(myTime2);
		if (result2 == GDT_VALID){
			struct tm tmTemp;
			schedule->time2=safe_mktime(myTime2.GetLocalTm(&tmTemp));
		}
		if (IsDlgButtonChecked(IDC_CHECKNOENDTIME)) schedule->time2=0;

		//time kindof (days)
		schedule->day=m_timesel.GetCurSel();
		schedule->enabled=IsDlgButtonChecked(IDC_S_ENABLE)!=0;

		schedule->ResetActions();
		for (uint8 i=0;i<m_actions.GetItemCount();i++) {
			schedule->actions[i]=m_actions.GetItemData(i);
			//MORPH START - Changed by Stulle, Show cat name for scheduler cat actions
			if(schedule->actions[i]==ACTION_CATSTOP || schedule->actions[i]==ACTION_CATRESUME)
			{
				CString strCat = m_actions.GetItemText(i,1);
				schedule->values[i]=strCat.Left(strCat.Find(L"(")-1);
			}
			else
			//MORPH END   - Changed by Stulle, Show cat name for scheduler cat actions
				schedule->values[i]=m_actions.GetItemText(i,1);
		}
		
		m_list.SetItemText(index, 0, schedule->title);
		m_list.SetItemText(index, 1, GetDayLabel(schedule->day));
		CTime time(theApp.scheduler->GetSchedule(index)->time);
		CString timeS;
		timeS.Format(_T("%s"),time.Format(_T("%H:%M")));
		m_list.SetItemText(index, 2, timeS);
	}
	RecheckSchedules();
  // MORPH START  leuk_he: Remove 2nd apply in scheduler
  SetModified(false);
	return CPropertyPage::OnApply();
  // MORPH END leuk_he: Remove 2nd apply in scheduler

}

void CPPgScheduler::OnBnClickedRemove()
{
  /*MORPH START  leuk_he: Remove 2nd apply in scheduler
	int index=m_list.GetSelectionMark();
 */
	int index=miActiveSelection;
 // MORPH END leuk_he: Remove 2nd apply in scheduler
  

	if (index!=-1) theApp.scheduler->RemoveSchedule(index);
	FillScheduleList();
	theApp.scheduler->RestoreOriginals();

	RecheckSchedules();
  // MORPH START  leuk_he: Remove 2nd apply in scheduler
  miActiveSelection  =m_list.GetSelectionMark() ;
  SetModified();
  // MORPH END leuk_he: Remove 2nd apply in scheduler
	

}

/* MORPH START  leuk_he: Remove 2nd apply in scheduler
BOOL CPPgScheduler::OnApply(){
	SetModified(FALSE);
	return CPropertyPage::OnApply();
}
   MORPH END leuk_he: Remove 2nd apply in scheduler*/

CString CPPgScheduler::GetActionLabel(int index) {
	switch (index) {
		case ACTION_SETUPL		: return GetResString(IDS_PW_UPL);
		case ACTION_SETDOWNL	: return GetResString(IDS_PW_DOWNL);
		case ACTION_SOURCESL	: return GetResString(IDS_LIMITSOURCES);
		case ACTION_CON5SEC		: return GetResString(IDS_LIMITCONS5SEC);
		case ACTION_CATSTOP		: return GetResString(IDS_SCHED_CATSTOP);
		case ACTION_CATRESUME	: return GetResString(IDS_SCHED_CATRESUME);
		case ACTION_CONS		: return GetResString(IDS_PW_MAXC);
		//EastShare START - Added by Pretender, add USS settings in scheduler tab
		case ACTION_USSMAXPING	: return GetResString(IDS_USS_MAXPING);
		case ACTION_USSGOUP		: return GetResString(IDS_USS_GOINGUPDIVIDER);
		case ACTION_USSGODOWN	: return GetResString(IDS_USS_GOINGDOWNDIVIDER);
		case ACTION_USSMINUP	: return GetResString(IDS_MINUPLOAD);
		//EastShare END - Added by Pretender, add USS settings in scheduler tab

		// Mighty Knife: additional scheduling events
		case ACTION_BACKUP  	: return GetResString(IDS_SCHED_BACKUP);
		case ACTION_UPDIPCONF	: return GetResString(IDS_SCHED_UPDATE_IPCONFIG);
		case ACTION_UPDFAKES	: return GetResString(IDS_SCHED_UPDATE_FAKES);
		case ACTION_UPDCOUNTRY	: return GetResString(IDS_SCHED_UPDATE_COUNTRY);
		case ACTION_RELOAD		: return GetResString(IDS_SF_RELOAD); // MORPH add teload on schedule
		// [end] MIghty Knife
	}
	//MORPH - Modified by IceCream, return a CString
	/*
	return NULL;
	*/
	return _T("");
	//MORPH - Modified by IceCream, return a CString
}

CString CPPgScheduler::GetDayLabel(int index) {
	switch (index) {
		case DAY_DAYLY : return GetResString(IDS_DAYLY);
		case DAY_MO		: return GetResString(IDS_MO);
		case DAY_DI		: return GetResString(IDS_DI);
		case DAY_MI		: return GetResString(IDS_MI);
		case DAY_DO		: return GetResString(IDS_DO);
		case DAY_FR		: return GetResString(IDS_FR);
		case DAY_SA		: return GetResString(IDS_SA);
		case DAY_SO		: return GetResString(IDS_SO);
		case DAY_MO_FR	: return GetResString(IDS_DAY_MO_FR);
		case DAY_MO_SA	: return GetResString(IDS_DAY_MO_SA);
		case DAY_SA_SO	: return GetResString(IDS_DAY_SA_SO);
	}
	//MORPH - Modified by IceCream, return a CString
	/*
	return NULL;
	*/
	return _T("");
	//MORPH - Modified by IceCream, return a CString
}

void CPPgScheduler::OnNmDblClkActionlist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	if (m_actions.GetSelectionMark()!=-1) {
		int ac=m_actions.GetItemData(m_actions.GetSelectionMark());
		// Mighty Knife: actions without parameters
		if (ac < ACTION_BACKUP || ac > ACTION_RELOAD) {
			if (ac!=6 && ac!=7) OnCommand(MP_CAT_EDIT,0);
		} 
		// [end] Mighty Knife
	}

	*pResult = 0;
}

void CPPgScheduler::OnNmRClickActionlist(NMHDR* /*pNMHDR*/, LRESULT* pResult)
{
	POINT point;
	::GetCursorPos(&point);

	CTitleMenu m_ActionMenu;
	CMenu m_ActionSel;
	CMenu m_CatActionSel;

	bool isCatAction=false;
	if (m_actions.GetSelectionMark()!=-1) {
		int ac=m_actions.GetItemData(m_actions.GetSelectionMark());
		if (ac==6 || ac==7) isCatAction=true;
	}

	// Mighty Knife: actions without parameters
	bool isParameterless = false;
	if (m_actions.GetSelectionMark()!=-1) {
		int ac=m_actions.GetItemData(m_actions.GetSelectionMark());
		if (ac>=ACTION_BACKUP && ac<=IDS_SF_RELOAD) isParameterless=true;
	}
	// [end] Mighty Knife

	m_ActionMenu.CreatePopupMenu();
	m_ActionSel.CreatePopupMenu();
	m_CatActionSel.CreatePopupMenu();

	UINT nFlag=MF_STRING;
	if (m_actions.GetSelectionMark()==-1) nFlag=MF_STRING | MF_GRAYED;

	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_SETUPL,GetResString(IDS_PW_UPL));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_SETDOWNL,GetResString(IDS_PW_DOWNL));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_SOURCESL,GetResString(IDS_LIMITSOURCES));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_CON5SEC,GetResString(IDS_LIMITCONS5SEC));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_CONS,GetResString(IDS_PW_MAXC));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_CATSTOP,GetResString(IDS_SCHED_CATSTOP));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_CATRESUME,GetResString(IDS_SCHED_CATRESUME));
	//EastShare START - Added by Pretender, add USS settings in scheduler tab
	CString Buffer;
	Buffer.Format(GetResString(IDS_USS_MAXPING),500);
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_USSMAXPING,Buffer);
	Buffer.Format(GetResString(IDS_USS_GOINGUPDIVIDER),1000);
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_USSGOUP,Buffer);
	Buffer.Format(GetResString(IDS_USS_GOINGDOWNDIVIDER),1000);
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_USSGODOWN,Buffer);
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_USSMINUP,GetResString(IDS_MINUPLOAD));
	//EastShare END - Added by Pretender, add USS settings in scheduler tab

	// Mighty Knife: additional scheduling events
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_BACKUP,GetResString(IDS_SCHED_BACKUP));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_UPDFAKES,GetResString(IDS_SCHED_UPDATE_FAKES));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_UPDIPCONF,GetResString(IDS_SCHED_UPDATE_IPCONFIG));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_UPDCOUNTRY,GetResString(IDS_SCHED_UPDATE_COUNTRY));
	m_ActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+ACTION_RELOAD,GetResString(IDS_SF_RELOAD));
	// [end] MIghty Knife

	m_ActionMenu.AddMenuTitle(GetResString(IDS_ACTION));
	m_ActionMenu.AppendMenu(MF_POPUP,(UINT_PTR)m_ActionSel.m_hMenu,	GetResString(IDS_ADD));

	// Mighty Knife: actions without parameters
	if (!isParameterless) {
		if (isCatAction) {
			//khaos::categorymod+
			/*
			if (thePrefs.GetCatCount()>1)
				m_CatActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+20,GetResString(IDS_ALLUNASSIGNED));
			m_CatActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+21,GetResString(IDS_ALL));
			for (int i=1;i<thePrefs.GetCatCount();i++)
			*/
			m_CatActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+21,GetResString(IDS_ALL));
			m_CatActionSel.AppendMenu(MF_SEPARATOR);
			for (int i=0;i<thePrefs.GetCatCount();i++)
			//khaos::categorymod-
				m_CatActionSel.AppendMenu(MF_STRING,MP_SCHACTIONS+22+i,thePrefs.GetCategory(i)->strTitle);
			m_ActionMenu.AppendMenu(MF_POPUP,(UINT_PTR)m_CatActionSel.m_hMenu,	GetResString(IDS_SELECTCAT));
		} else
			m_ActionMenu.AppendMenu(nFlag,MP_CAT_EDIT,	GetResString(IDS_EDIT));
	}
	// [end] Mighty Knife


	m_ActionMenu.AppendMenu(nFlag,MP_CAT_REMOVE,GetResString(IDS_REMOVE));

	m_ActionMenu.TrackPopupMenu(TPM_LEFTALIGN |TPM_RIGHTBUTTON, point.x, point.y, this);
	VERIFY( m_ActionSel.DestroyMenu() );
	VERIFY( m_CatActionSel.DestroyMenu() );
	VERIFY( m_ActionMenu.DestroyMenu() );

	*pResult = 0;
}

BOOL CPPgScheduler::OnCommand(WPARAM wParam, LPARAM lParam)
{ 
   int item= m_actions.GetSelectionMark(); 
	// add
	if (wParam>=MP_SCHACTIONS && wParam<MP_SCHACTIONS+20 && m_actions.GetItemCount()<16)
	{
		int action=wParam-MP_SCHACTIONS;
		int i=m_actions.GetItemCount();
		m_actions.InsertItem(i,GetActionLabel(action));
    // MORPH START  leuk_he: Remove 2nd apply in scheduler
		SetModified();
    // MORPH END leuk_he: Remove 2nd apply in scheduler
		m_actions.SetItemData(i,action);
		m_actions.SetSelectionMark(i);
		if (action<6)
			OnCommand(MP_CAT_EDIT,0);
		// Mighty Knife START: parameterless schedule events
		else if (action>=ACTION_BACKUP && action<=ACTION_RELOAD) {
			// Small warning message
			if (action == ACTION_UPDIPCONF || action == ACTION_UPDFAKES || action == ACTION_UPDCOUNTRY) {
				XMessageBox (NULL,GetResString (IDS_SCHED_UPDATE_WARNING),
							 GetResString (IDS_WARNING),MB_OK | MB_ICONINFORMATION,NULL);
				m_actions.SetItemText(i,1,L"update");
			}
			else
				m_actions.SetItemText(i,1,L"-");
			CTime myTime1 ;m_time.GetTime(myTime1);  // MORPH add check for one time events
			CTime myTime2 ;m_timeTo.GetTime(myTime2);
			if ( myTime1!= myTime2) // leuk_he: warn because will be executeed every minute! 
				if(XMessageBox (NULL,GetResString(IDS_SCHED_WARNENDTIME),
				   GetResString (IDS_WARNING),MB_OKCANCEL| MB_ICONINFORMATION,NULL)== IDOK)
					 m_timeTo.SetTime(&myTime1); // On ok reset end time. 
		}
		//  Mighty Knife END
		//MORPH START - Changed by Stulle, Show cat name for scheduler cat actions
		else
			m_actions.SetItemText(i,1,L"-1 ("+GetResString(IDS_ALL)+L")");
		//MORPH END   - Changed by Stulle, Show cat name for scheduler cat actions
	}
	else if (wParam>=MP_SCHACTIONS+20 && wParam<=MP_SCHACTIONS+80)
	{
		CString newval;
		//MORPH START - Changed by Stulle, Show cat name for scheduler cat actions
		/*
		newval.Format(_T("%i"),wParam-MP_SCHACTIONS-22);
		*/
		int iCat = wParam-MP_SCHACTIONS-22;
		CString strCatTitle;
		if(iCat == -1)
			strCatTitle = GetResString(IDS_ALL);
		else
		{
			Category_Struct* thisCat = thePrefs.GetCategory(iCat);
			if(thisCat)
				strCatTitle = thisCat->strTitle;
			else // should not happen
				strCatTitle = GetResString(IDS_UNKNOWN);
		}
		newval.Format(L"%i (%s)",iCat,strCatTitle);
		//MORPH END   - Changed by Stulle, Show cat name for scheduler cat actions
		m_actions.SetItemText(item,1,newval);
		// MORPH START  leuk_he: Remove 2nd apply in scheduler
		SetModified();
		// lhane
   }
	else if (wParam == ID_HELP)
	{
		OnHelp();
		return TRUE;
	}

   switch (wParam){ 
		case MP_CAT_EDIT: 
        { 
			if (item!=-1) {
				InputBox inputbox;
				// todo: differen prompts
				CString prompt;
				switch (m_actions.GetItemData(item)) {
					case 1:
					case 2:
						prompt=GetResString(IDS_SCHED_ENTERDATARATELIMIT)+_T(" (")+GetResString(IDS_KBYTESPERSEC)+_T(")");
						break;
					default: prompt=GetResString(IDS_SCHED_ENTERVAL);
				}
				inputbox.SetLabels(GetResString(IDS_SCHED_ACTCONFIG),prompt,m_actions.GetItemText(item,1));
				inputbox.DoModal();
				CString res=inputbox.GetInput();
				if (!inputbox.WasCancelled()) m_actions.SetItemText(item,1,res);
         // MORPH START  leuk_he: Remove 2nd apply in scheduler
				SetModified();
        //MORPH END leuk_he: Remove 2nd apply in schedulerd
			}
			break; 
        }
		case MP_CAT_REMOVE:
		{
			// remove
			if (item!=-1) {
				int ix=m_actions.GetSelectionMark();
				if (ix!=-1) {
					m_actions.DeleteItem(ix);
				}
        // MORPH START  leuk_he: Remove 2nd apply in scheduler
				SetModified();
        // MORPH END leuk_he: Remove 2nd apply in scheduler
			}
			break;
		}
   } 
   return CPropertyPage::OnCommand(wParam, lParam);
}

void CPPgScheduler::RecheckSchedules() {
	theApp.scheduler->Check(true);
}

void CPPgScheduler::OnEnableChange() {
	thePrefs.scheduler=IsDlgButtonChecked(IDC_ENABLE)!=0;
	if (!thePrefs.scheduler) theApp.scheduler->RestoreOriginals();
	
	RecheckSchedules();
	theApp.emuledlg->preferenceswnd->m_wndConnection.LoadSettings();	
  /* MORPH START  leuk_he: Remove 2nd apply in scheduler
	SetModified();
   MORPH END leuk_he: Remove 2nd apply in scheduler   */ 
}

void CPPgScheduler::OnDisableTime2() {
	GetDlgItem(IDC_DATETIMEPICKER2)->EnableWindow( IsDlgButtonChecked(IDC_CHECKNOENDTIME)==0 );
}

void CPPgScheduler::OnHelp()
{
	theApp.ShowHelp(eMule_FAQ_Preferences_Scheduler);
}

BOOL CPPgScheduler::OnHelpInfo(HELPINFO* /*pHelpInfo*/)
{
	OnHelp();
	return TRUE;
}
