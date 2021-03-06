//this file is part of eMule
//Copyright (C)2002-2007 Merkur ( strEmail.Format("%s@%s", "devteam", "emule-project.net") / http://www.emule-project.net )
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
#include "Scheduler.h"
#include "OtherFunctions.h"
#include "ini2.h"
#include "Preferences.h"
#include "DownloadQueue.h"
#include "emuledlg.h"
#include "MenuCmds.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


CScheduler::CScheduler(){
	LoadFromFile();
	SaveOriginals();
	m_iLastCheckedMinute=60;
}

CScheduler::~CScheduler(){
	SaveToFile();
	RemoveAll();
}

void CScheduler::LoadFromFile(){

	CString strName;
	CString temp;

	strName.Format(_T("%spreferences.ini"),thePrefs.GetMuleDirectory(EMULE_CONFIGDIR));
	CIni ini(strName, _T("Scheduler"));
	
	size_t max = ini.GetInt(_T("Count"),0);
	size_t count = 0;

	while (count<max) {
		strName.Format(_T("Schedule#%i"),count);
		temp=ini.GetString(_T("Title"), NULL, strName);
		if (!temp.IsEmpty()) {
			Schedule_Struct* news= new Schedule_Struct();
			news->title=temp;
			news->day=ini.GetInt(_T("Day"),0);
			news->enabled=ini.GetBool(_T("Enabled"));
			news->time=ini.GetUInt64(_T("StartTime"));// X: [64T] - [64BitTime]
			news->time2=ini.GetUInt64(_T("EndTime"));
			ini.SerGet(true, news->actions,
				ARRSIZE(news->actions), _T("Actions"));
			ini.SerGet(true, news->values,
				ARRSIZE(news->values), _T("Values"));

			AddSchedule(news);
			count++;
		} else break;
	}
}

void CScheduler::SaveToFile(){

	CString temp;
	Schedule_Struct* schedule;

	CIni ini(thePrefs.GetConfigFile(), _T("Scheduler"));
	ini.WriteInt(_T("Count"), (int)GetCount());
	
	for (size_t i=0; i<GetCount();i++) {
		schedule=theApp.scheduler->GetSchedule(i);

		temp.Format(_T("Schedule#%i"),i);
		ini.WriteString(_T("Title"),schedule->title,temp);
		ini.WriteInt(_T("Day"),schedule->day);
		ini.WriteUInt64(_T("StartTime"),schedule->time);// X: [64T] - [64BitTime]
		ini.WriteUInt64(_T("EndTime"),schedule->time2);
		ini.WriteBool(_T("Enabled"),schedule->enabled);

		ini.SerGet(false, schedule->actions,
			ARRSIZE(schedule->actions), _T("Actions"));
		ini.SerGet(false, schedule->values,
			ARRSIZE(schedule->values), _T("Values"));
	}
}

void CScheduler::RemoveSchedule(size_t index){
	
	if (index>=schedulelist.GetCount()) return;

	Schedule_Struct* todel;
	todel=schedulelist.GetAt(index);
	delete todel;
	schedulelist.RemoveAt(index);
}

void CScheduler::RemoveAll(){
	while( schedulelist.GetCount()>0 )
		RemoveSchedule(0);
}

size_t CScheduler::AddSchedule(Schedule_Struct* schedule) {
	schedulelist.Add(schedule);
	return schedulelist.GetCount()-1;
}

INT_PTR CScheduler::Check(bool forcecheck){
	if (!thePrefs.IsSchedulerEnabled()
		|| theApp.scheduler->GetCount()==0
		|| !CemuleDlg::IsRunning()) return -1;

	Schedule_Struct* schedule;
	struct tm tmTemp;
	CTime tNow = CTime(safe_mktime(CTime::GetCurrentTime().GetLocalTm(&tmTemp)));
	
	if (!forcecheck && tNow.GetMinute()==m_iLastCheckedMinute) return -1;

	m_iLastCheckedMinute=tNow.GetMinute();
	theApp.scheduler->RestoreOriginals();

	for (size_t si=0;si<theApp.scheduler->GetCount();si++) {
		schedule=theApp.scheduler->GetSchedule(si);
		if (schedule->actions[0]==0 || !schedule->enabled) continue;

		// check day of week
		if (schedule->day!=DAY_DAYLY) {
			int dow=tNow.GetDayOfWeek();
			switch (schedule->day) {
				case DAY_MO : if (dow!=2) continue;
					break;
				case DAY_DI : if (dow!=3) continue;
					break;
				case DAY_MI : if (dow!=4) continue;
					break;
				case DAY_DO : if (dow!=5) continue;
					break;
				case DAY_FR : if (dow!=6) continue;
					break;
				case DAY_SA : if (dow!=7) continue;
					break;
				case DAY_SO : if (dow!=1) continue;
					break;
				case DAY_MO_FR : if (dow==7 || dow==1 ) continue;
					break;
				case DAY_MO_SA : if (dow==1) continue;
					break;
				case DAY_SA_SO : if (dow>=2 && dow<=6) continue;
			}
		}

		//check time
		UINT h1,h2,m1,m2;
		CTime t1=CTime(schedule->time);
		CTime t2=CTime(schedule->time2);
		h1=t1.GetHour();	h2=t2.GetHour();
		m1=t1.GetMinute();	m2=t2.GetMinute();
		uint_ptr it1,it2, itn;
		it1=h1*60 + m1;
		it2=h2*60 + m2;
		itn=tNow.GetHour()*60 + tNow.GetMinute();
		if (it1<=it2) { // normal timespan
			if ( !(itn>=it1 && itn<it2) ) continue;
		} else {		   // reversed timespan (23:30 to 5:10)  now 10
			if ( !(itn>=it1 || itn<it2)) continue;
		}

		// ok, lets do the actions of this schedule
		ActivateSchedule(si,schedule->time2==0);
	}

	return -1;
}

void CScheduler::SaveOriginals() {
	original_upload=thePrefs.GetMaxUpload();
	original_download=thePrefs.GetMaxDownload();
	original_connections=thePrefs.GetMaxConnections();
	original_cons5s=thePrefs.GetMaxConperFive();
	original_sources=thePrefs.GetMaxSourcePerFileDefault();
}

void CScheduler::RestoreOriginals() {
	thePrefs.SetMaxUpload(original_upload);
	thePrefs.SetMaxDownload(original_download);
	thePrefs.SetMaxConnections(original_connections);
	thePrefs.SetMaxConsPerFive(original_cons5s);
	thePrefs.SetMaxSourcesPerFile(original_sources);
}

void CScheduler::ActivateSchedule(size_t index,bool makedefault) {
	Schedule_Struct* schedule=GetSchedule(index);

	for (size_t ai=0;ai<16;ai++) {
		if (schedule->actions[ai]==0) break;
		if (schedule->values[ai].IsEmpty() /* maybe ignore in some future cases...*/ ) continue;
		if(schedule->actions[ai]<3){
			//Xman
			// Maella [FAF] -Allow Bandwidth Settings in <1KB Incremements-
			float fTemp=(float)_tstof(schedule->values[ai]);
			if(schedule->actions[ai]==1){
				thePrefs.SetMaxUpload(fTemp);
				//thePrefs.CheckSlotSpeed(); //Xman Xtreme Upload
				if (makedefault)
					original_upload= original_upload=fTemp; 
			}
			else if(schedule->actions[ai]==2){
				thePrefs.SetMaxDownload(fTemp);
				if (makedefault)
					original_download=fTemp;
			}
			//Xman end
		}
		else{
			UINT iTemp=_tstoi(schedule->values[ai]);
			switch (schedule->actions[ai]) {
				case 3 :
					thePrefs.SetMaxSourcesPerFile(iTemp);
					if (makedefault)
						original_sources=iTemp;
					break;
				case 4 :
					thePrefs.SetMaxConsPerFive(iTemp);
					if (makedefault)
						original_cons5s=iTemp;
					break;
				case 5 :
					thePrefs.SetMaxConnections(iTemp);
					if (makedefault)
						original_connections=iTemp;
					break;
				case 6 :
					theApp.downloadqueue->SetCatStatus(iTemp,MP_STOP);
					break;
				case 7 :
					theApp.downloadqueue->SetCatStatus(iTemp,MP_RESUME);
					break;
			}
		}
	}
}