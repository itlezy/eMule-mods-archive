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
#include "BarShader.h" // ZZUL-TRA :: Spreadbars

class CKnownFile;

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
	// ZZUL-TRA :: Spreadbars :: Start Reduce CPU consumption
		InChangedSpreadSortValue = false;
		InChangedFullSpreadCount = false;
		InChangedSpreadBar = false;
		lastSpreadSortValue = 0;;
		lastFullSpreadCount = 0;
		// ZZUL-TRA :: Spreadbars :: End
	}
	// ZZUL-TRA :: Spreadbars :: Start Reduce CPU consumption
	~CStatisticFile()
	{
		m_bitmapSpreadBar.DeleteObject();
	}
	// ZZUL-TRA :: Spreadbars :: End

	void	MergeFileStats( CStatisticFile* toMerge );
	void	AddRequest();
	void	AddAccepted();
	// ZZUL-TRA :: Spreadbars :: Start
	/*
	void	AddTransferred(uint64 bytes);
	*/
	void	AddTransferred(uint64 start, uint64 bytes);
	void	AddBlockTransferred(uint64 start, uint64 end, uint64 count);
	void	DrawSpreadBar(CDC* dc, RECT* rect, bool bFlat) /*const*/;
	float	GetSpreadSortValue() /*const*/;
	float	GetFullSpreadCount() /*const*/;
	void	ResetSpreadBar(); 
	// ZZUL-TRA :: Spreadbars :: End

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

	// ZZUL-TRA :: Spreadbars :: Start
	CRBMap<uint64, uint64> spreadlist;
	static CBarShader s_SpreadBar;
	// ZZUL-TRA :: Spreadbars :: Start Reduce SpreadBar CPU consumption
	bool	InChangedSpreadSortValue;
	bool	InChangedFullSpreadCount;
	bool	InChangedSpreadBar;
	CBitmap m_bitmapSpreadBar;
	int		lastSize;
	bool	lastbFlat;
	float	lastSpreadSortValue;
	float	lastFullSpreadCount;
	// ZZUL-TRA :: Spreadbars :: End

private:
	uint32 requested;
	uint32 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;
};
