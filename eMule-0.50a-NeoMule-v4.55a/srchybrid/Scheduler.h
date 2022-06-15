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
#define ACTION_CONHALF		8 // NEO: QS - [QuickStart] <-- Xanatos --
#ifdef NEO_BC // NEO: NBCs - [NeoBandwidthControlSheduler] -- Xanatos -->
#define ACTION_ULMODE		9
#define ACTION_ULMAX		10
#define ACTION_ULMIN		11

#define ACTION_UPBASEPING	12
#define ACTION_UPTOLERANCE	13
#define ACTION_UPPROZENT	14

#define ACTION_UPSTREAM		15

#define ACTION_DLMODE		16
#define ACTION_DLMAX		17
#define ACTION_DLMIN		18

#define ACTION_DOWNBASEPING	19
#define ACTION_DOWNTOLERANCE 20
#define ACTION_DOWNPROZENT	21

#define ACTION_DOWNSTREAM	22

	
#define ACTION_MANCON		23
#define ACTION_MANSET		24

#define ACTION_INCLUDEOH	25
#endif // NEO_BC // NEO: NBCs END <-- Xanatos --

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
   uint32			time;
   uint32			time2;
#ifdef NEO_BC // NEO: NBCs - [NeoBandwidthControlSheduler] -- Xanatos -->
   CString			values[25];
   int				actions[25];
   void ResetActions()	{for (uint8 index=0;index<25;index++) {actions[index]=0;values[index]="";}}
#else
   // NEO: QS - [QuickStart] <-- Xanatos -->
   CString			values[17]; 
   int				actions[17];
   void ResetActions()	{for (uint8 index=0;index<16;index++) {actions[index]=0;values[index]=_T("");}}
   // NEO: QS END <-- Xanatos --
#endif // NEO_BC // NEO: NBCs END <-- Xanatos --
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
	
	uint16	original_upload;
	uint16	original_download;
	UINT	original_connections;
	UINT	original_cons5s;
	UINT	original_sources;
	UINT	original_conshalf; // NEO: QS - [QuickStart] <-- Xanatos --

#ifdef NEO_BC // NEO: NBCs - [NeoBandwidthControlSheduler] -- Xanatos -->
	int		original_UlMode;
	float	original_UlMax;
	float	original_UlMin;

	int		original_UpBasePing;
	int		original_UpTolerance;
	int		original_UpProzent;

	float	original_UpStream;

	int		original_DlMode;
	float	original_DlMax;
	float	original_DlMin;

	int		original_DownBasePing;
	int		original_DownTolerance;
	int		original_DownProzent;

	float	original_DownStream;
	
	bool	original_ManageCons;
	float	original_ManageFactor;

	bool	original_IncludeOh;
#endif // NEO_BC // NEO: NBCs END <-- Xanatos --

private:
	CArray<Schedule_Struct*,Schedule_Struct*> schedulelist;
	int		m_iLastCheckedMinute;
};
