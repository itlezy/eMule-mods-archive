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

#include "BarShader.h" // Spread bars [Slugfiller/MorphXT] - Stulle

// ==> Removed Spreadbars (old version) [SlugFiller] - Stulle
/*
//Xman PowerRelease
struct Spread_Struct{
	uint64 start;
	uint64 end;
	uint32 count;
};
//Xman end
*/
// <== Removed Spreadbars (old version) [SlugFiller] - Stulle

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
		// ==> Spread bars [Slugfiller/MorphXT] - Stulle
		InChangedSpreadSortValue = false;
		InChangedFullSpreadCount = false;
		InChangedSpreadBar = false;
		lastSpreadSortValue = 0;;
		lastFullSpreadCount = 0;
		// <== Spread bars [Slugfiller/MorphXT] - Stulle
		m_uFileupdatetime = 0; //Xman Code Improvement -> don't update to often

		//Xman advanced upload-priority
		m_unotcountedtransferred = 0;
		m_tlastdataupdate = 0;
		//Xman end
	}

	void	MergeFileStats( CStatisticFile* toMerge );
	void	AddRequest();
	void	AddAccepted();
	//Xman PowerRelease
	/*
	void	AddTransferred(uint64 bytes);
	*/
	// ==> Removed Spreadbars (old version) [SlugFiller] - Stulle
	/*
	~CStatisticFile();	
	void	AddTransferred(uint64 start, uint32 bytes);
	void	AddBlockTransferred(uint64 start, uint64 end, uint32 count);
	*/
	// <== Removed Spreadbars (old version) [SlugFiller] - Stulle
	//Xman end

	UINT	GetRequests() const				{return requested;}
	UINT	GetAccepts() const				{return accepted;}
	uint64	GetTransferred() const			{return transferred;}
	UINT	GetAllTimeRequests() const		{return alltimerequested;}
	UINT	GetAllTimeAccepts() const		{return alltimeaccepted;}
	uint64	GetAllTimeTransferred() const	{return alltimetransferred;}
	void	SetAllTimeRequests(uint32 nVal);
	void	SetAllTimeAccepts(uint32 nVal);
	void	SetAllTimeTransferred(uint64 nVal);

	//Xman advanced upload-priority
	uint64	GetCountedTransferred() const	{return alltimetransferred - m_unotcountedtransferred;}
	void  UpdateCountedTransferred();
	uint64 m_unotcountedtransferred;
	uint32 m_tlastdataupdate;
	//Xman end

	CKnownFile* fileParent;
	// ==> Removed Spreadbars (old version) [SlugFiller] - Stulle
	/*
	CTypedPtrList<CPtrList, Spread_Struct*> spreadlist; //Xman PowerRelease
	*/
	// <== Removed Spreadbars (old version) [SlugFiller] - Stulle

private:
	uint32 requested;
	uint32 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;
	uint32 m_uFileupdatetime; //Xman Code Improvement -> don't update to often

	// ==> Spread bars [Slugfiller/MorphXT] - Stulle
	static CBarShader s_SpreadBar;
	bool	InChangedSpreadSortValue;
	bool	InChangedFullSpreadCount;
	bool	InChangedSpreadBar;
	CBitmap m_bitmapSpreadBar;
	int		lastSize;
	bool	lastbFlat;
	float	lastSpreadSortValue;
	float	lastFullSpreadCount;

public:
	CRBMap<uint64, uint64> spreadlist;
	~CStatisticFile()
	{
		m_bitmapSpreadBar.DeleteObject();
	}
	void	AddTransferred(uint64 start, uint64 bytes);
	void	AddBlockTransferred(uint64 start, uint64 end, uint64 count);
	void	DrawSpreadBar(CDC* dc, RECT* rect, bool bFlat) /*const*/;
	float	GetSpreadSortValue() /*const*/;
	float	GetFullSpreadCount() /*const*/;
	void	ResetSpreadBar(); //MORPH	- Added by AndCycle, SLUGFILLER: Spreadbars - per file
	// <== Spread bars [Slugfiller/MorphXT] - Stulle

	bool	GetFairPlay() const; // Fair Play [AndCycle/Stulle] - Stulle
};
