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

//Xman PowerRelease
struct Spread_Struct{
	uint64 start;
	uint64 end;
	uint32 count;
};
//Xman end

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
	~CStatisticFile();	
	void	AddTransferred(uint64 start, uint32 bytes);
	void	AddBlockTransferred(uint64 start, uint64 end, uint32 count);
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
	CTypedPtrList<CPtrList, Spread_Struct*> spreadlist; //Xman PowerRelease

private:
	uint32 requested;
	uint32 accepted;
	uint64 transferred;
	uint32 alltimerequested;
	uint64 alltimetransferred;
	uint32 alltimeaccepted;
	uint32 m_uFileupdatetime; //Xman Code Improvement -> don't update to often
};
