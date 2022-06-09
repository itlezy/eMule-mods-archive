//	This file is part of eMule Plus
//
//	This program is free software; you can redistribute it and/or
//	modify it under the terms of the GNU General Public License
//	as published by the Free Software Foundation; either
//	version 2 of the License, or (at your option) any later version.
//
//	This program is distributed in the hope that it will be useful,
//	but WITHOUT ANY WARRANTY; without even the implied warranty of
//	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//	GNU General Public License for more details.
//
//	You should have received a copy of the GNU General Public License
//	along with this program; if not, write to the Free Software
//	Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
#pragma once

#include "types.h"

class CPartFile;
class CSourceData;

class CSourceSaver
{
public:
	CSourceSaver();

	bool	Process(CPartFile *pPartFile, int iMaxSourcesToSave = 10, bool bIgnoreTimer = false);
	void	LoadSources(CPartFile *pPartFile);

private:
	typedef	CTypedPtrList<CPtrList, CSourceData*> SourceList;

	void	LoadSourcesFromFile(CPartFile *pPartFile, SourceList *plistSources);
	void	SaveSources(CPartFile *pPartFile, int iMaxSourcesToSave);

	CString	CalcExpiration(uint32 dwDays);
	bool	IsExpired(const CString &strExpirationDate);

	uint32	m_dwLastTimeSaved;
};
