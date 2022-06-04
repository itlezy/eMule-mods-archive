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
//MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
//GNU General Public License for more details.
//
//You should have received a copy of the GNU General Public License
//along with this program; if not, write to the Free Software
//Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once
class CKnownFile;
#include "BarShader.h" //Spreadbars

class CStatisticFile
{
public:
	CStatisticFile()
	{
		requested = 0;
		transferred = 0;
		accepted = 0;
		alltimerequested = 0;
		alltimetransferred = 0;
		alltimeaccepted = 0;
		//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
		InChangedSpreadSortValue = false;
		InChangedFullSpreadCount = false;
		InChangedSpreadBar = false;
		lastSpreadSortValue = 0;;
		lastFullSpreadCount = 0;
		//MORPH END   - Added by SiRoB, Reduce SpreadBar CPU consumption
		//Morph Start - Added by AndCycle, Equal Chance For Each File
		m_bInChangedEqualChanceValue = false;
		lastCheckEqualChanceSemiValue = time(NULL);
		m_dLastEqualChanceBiasValue = 1;
		m_dLastEqualChanceSemiValue = 0;
		m_dwSessionShareTime = time(NULL);
		//Morph End - Added by AndCycle, Equal Chance For Each File
	}
	//MORPH START - Added by SiRoB, Reduce SpreadBar CPU consumption
	~CStatisticFile()
	{
		m_bitmapSpreadBar.DeleteObject();
	}
	//MORPH END   - Added by SiRoB, Reduce SpreadBar CPU consumption

	void	MergeFileStats( CStatisticFile* toMerge );
	void	AddRequest();
	void	AddAccepted();
	//MORPH START - Added by IceCream SLUGFILLER: Spreadbars
	/*
	void	AddTransferred(uint64 bytes);
	*/
	void	AddTransferred(uint64 start, uint64 bytes);
	void	AddBlockTransferred(uint64 start, uint64 end, uint64 count);
	void	DrawSpreadBar(CDC* dc, RECT* rect, bool bFlat) /*const*/;
	float	GetSpreadSortValue() /*const*/;
	float	GetFullSpreadCount() /*const*/;
	void	ResetSpreadBar(); //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	//MORPH END - Added by IceCream SLUGFILLER: Spreadbars

	UINT	GetRequests() const				{return requested;}
	UINT	GetAccepts() const				{return accepted;}
	uint64	GetTransferred() const			{return transferred;}
	UINT	GetAllTimeRequests() const		{return alltimerequested;}
	UINT	GetAllTimeAccepts() const		{return alltimeaccepted;}
	uint64	GetAllTimeTransferred() const	{return alltimetransferred;}
	void	SetAllTimeRequests(uint32 nVal);
	void	SetAllTimeAccepts(uint32 nVal);
	void	SetAllTimeTransferred(uint64 nVal);
	
	CKnownFile* fileParent;
	//Morph Start - Added by AndCycle, Equal Chance For Each File
	double	GetEqualChanceValue();
	CString	GetEqualChanceValueString(bool detail = true) const;
	time_t	GetSessionShareTime() const	{ return time(NULL) - m_dwSessionShareTime; }
	void	SetSessionShareTime()		{ m_dwSessionShareTime = time(NULL); }
	//Morph End - Added by AndCycle, Equal Chance For Each File
	//EastShare	Start - FairPlay by AndCycle
	bool	GetFairPlay() const;
	//EastShare	End   - FairPlay by AndCycle
	//MORPH START - Added by IceCream SLUGFILLER: Spreadbars
	CRBMap<uint64, uint64> spreadlist;
private:
	static CBarShader s_SpreadBar;
	//MORPH - Added by SiRoB, Reduce SpreadBar CPU consumption
	bool	InChangedSpreadSortValue;
	bool	InChangedFullSpreadCount;
	bool	InChangedSpreadBar;
	CBitmap m_bitmapSpreadBar;
	int		lastSize;
	bool	lastbFlat;
	float	lastSpreadSortValue;
	float	lastFullSpreadCount;
	//MORPH - Added by SiRoB, Reduce SpreadBar CPU consumption
	//MORPH END   - Added by IceCream, SLUGFILLER: Spreadbars

private:
	uint32 requested;
	uint32 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;
	//Morph Start - Added by AndCycle, Equal Chance For Each File
	bool	m_bInChangedEqualChanceValue;
	time_t	lastCheckEqualChanceSemiValue; //vs2005
	double	m_dLastEqualChanceBiasValue;
	double	m_dLastEqualChanceSemiValue;
	time_t  m_dwSessionShareTime; //vs2005
	//Morph End - Added by AndCycle, Equal Chance For Each File
};
