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
#pragma once

#define ACTION_SETUPL		1
#define ACTION_SETDOWNL		2
#define ACTION_SOURCESL		3
#define ACTION_CON5SEC		4
#define ACTION_CONS			5
#define ACTION_CATSTOP		6
#define ACTION_CATRESUME	7
//EastShare START - Added by Pretender, add USS settings in scheduler tab
#define ACTION_USSMAXPING	8
#define ACTION_USSGOUP		9
#define ACTION_USSGODOWN	10
#define ACTION_USSMINUP		11
//EastShare END - Added by Pretender, add USS settings in scheduler tab

// Mighty Knife: additional scheduling events
#define ACTION_BACKUP  	    12
#define ACTION_UPDIPCONF    13
#define ACTION_UPDFAKES	    14
#define ACTION_UPDCOUNTRY   15
#define ACTION_RELOAD	    16
// [end] Mighty Knife

#define DAY_DAYLY		0
#define DAY_MO			1
#define DAY_DI			2
#define DAY_MI			3
#define DAY_DO			4
#define DAY_FR			5
#define DAY_SA			6
#define DAY_SO			7
#define DAY_MO_FR		8
#define DAY_MO_SA		9 
#define DAY_SA_SO		10

struct Schedule_Struct{
   CString			title;
   bool				enabled;
   UINT				day;
   time_t			time; //vs2005
   time_t			time2; //vs2005
   CString			values[16];
   int				actions[16];
   void ResetActions()	{for (uint8 index=0;index<16;index++) {actions[index]=0;values[index]=_T("");}}
   ~Schedule_Struct() {  }
};

class CScheduler
{
public:
	CScheduler();
	~CScheduler();

	int		AddSchedule(Schedule_Struct* schedule);
	void	UpdateSchedule(int index, Schedule_Struct* schedule) { if (index<schedulelist.GetCount())schedulelist.SetAt(index,schedule);}
	Schedule_Struct* GetSchedule(int index) {if (index<schedulelist.GetCount()) return schedulelist.GetAt(index); else return NULL; }
	void	RemoveSchedule(int index);
	void	RemoveAll();
	int		LoadFromFile();
	void	SaveToFile();
	int		Check(bool forcecheck=false);
	UINT	GetCount()		{ return schedulelist.GetCount();}
	void	SaveOriginals();
	void	RestoreOriginals();
	void	ActivateSchedule(int index,bool makedefault=false);
	bool	HasWeekly(int action); // MORPH leuk_he : automatic weekly ipfilter/fakefilter update. 
	void 	SetWeekly(int action,bool activate); // MORPH leuk_he : automatic weekly ipfilter/fakefilter update. 
	
	UINT	original_upload; //MORPH uint16 is not enough
	UINT    original_download; //MORPH uint16 is not enough
	UINT	original_connections;
	UINT	original_cons5s;
	UINT	original_sources;
	//EastShare START - Added by Pretender, add USS settings in scheduler tab
	int	original_ussmaxping;
	int	original_ussgoup;
	int	original_ussgodown;
	UINT original_ussminup; //MORPH uint16 is not enough
	//EastShare END - Added by Pretender, add USS settings in scheduler tab

private:
	CArray<Schedule_Struct*,Schedule_Struct*> schedulelist;
	int		m_iLastCheckedMinute;
};
