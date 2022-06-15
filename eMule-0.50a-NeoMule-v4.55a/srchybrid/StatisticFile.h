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

class CFileDataIO; // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
#include "BarShader.h"

class CStatisticFile
{
	friend class CKnownFile;
	friend class CPartFile;
public:
	CStatisticFile()
	{
		requested = 0;
		transferred = 0;
		accepted = 0;
		alltimerequested = 0;
		alltimetransferred = 0;
		alltimeaccepted = 0;
		guifileupdatetime = 0; // NEO: CI#10 - [CodeImprovement] <-- Xanatos --
		completereleases = 0.0F; // NEO: NPT - [NeoPartTraffic] <-- Xanatos --
	}

	void	MergeFileStats( CStatisticFile* toMerge );
	void	AddRequest();
	void	AddAccepted();
	//void	AddTransferred(uint64 bytes);
	void	AddTransferred(uint64 start, uint32 bytes); // NEO: NPT - [NeoPartTraffic] <-- Xanatos --

	UINT	GetRequests() const				{return requested;}
	UINT	GetAccepts() const				{return accepted;}
	uint64	GetTransferred() const			{return transferred;}
	UINT	GetAllTimeRequests() const		{return alltimerequested;}
	UINT	GetAllTimeAccepts() const		{return alltimeaccepted;}
	uint64	GetAllTimeTransferred() const	{return alltimetransferred;}
	
	// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
	float	GetPartRelease(uint16 part);
	uint32	GetPartTraffic(uint16 part);
	uint32	GetPartTrafficSession(uint16 part);

	float	GetCompleteReleases() const		{return completereleases;}

	void	DrawTrafficStatusBar(CDC* dc, LPCRECT rect, bool onlygreyrect, bool bFlat, COLORREF(*GetTrafficColor)(float), uint16 part = 0xffff) const;

	void	ResetStats(bool all=false);

	bool	HasTraffic() {return (alltimespreadlist.IsEmpty() == FALSE);}

	bool	LoadTraffic(CFileDataIO* file);
	void	SaveTraffic(CFileDataIO* file);
	bool	LoadOldTraffic(CFileDataIO* file);
	// NEO: NPT END <-- Xanatos --

	CKnownFile* fileParent;

private:
	uint32 requested;
	uint32 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;

	uint32 guifileupdatetime; // NEO: CI#10 - [CodeImprovement] <-- Xanatos --

	// NEO: NPT - [NeoPartTraffic] -- Xanatos -->
	CRBMap<uint64, uint32> sessionspreadlist;
	CRBMap<uint64, uint32> alltimespreadlist;
	float completereleases;

	static CBarShader s_TrafficPartStatusBar;
	static CBarShader s_TrafficBlockStatusBar;
	static CBarShader s_SessionTrafficBlockStatusBar;

	void	AddBlockTransferred(uint64 start, uint64 end, uint32 count, CRBMap<uint64, uint32>& spreadlist);
	void	MergeSpreadList(CRBMap<uint64, uint32>& targetspreadlist,CRBMap<uint64, uint32>& sourcespreadlist);
	void	RecalcCompleteReleases();
	float	CalcReleases(uint64 start, uint64 end, CRBMap<uint64, uint32>& spreadlist) const;
	uint32	CalcTraffic(uint64 start, uint64 end, CRBMap<uint64, uint32>& spreadlist) const;
	// NEO: NPT END <-- Xanatos --
};
