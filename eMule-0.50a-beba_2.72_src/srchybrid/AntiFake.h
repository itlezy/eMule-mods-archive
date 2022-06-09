// Tux: Feature: additional fake checks [start]
//Copyright (C)2012 WiZaRd ( strEmail.Format("%s@%s", "thewizardofdos", "gmail.com") / http://www.emulefuture.de )
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

class CSearchFile;
class CMuleMemDC;
struct SSearchParams;

#define FA_GOOD		0
#define FA_OK		1
#define FA_UNKNOWN	2
#define FA_SUSPECT	3
#define FA_FAKE		4
int		GetFakeAlyzerRating(const CSearchFile* content, CString* ret = NULL);
CString	GetFakeComment(const CSearchFile* content, const bool bSimple, int* i = NULL);
bool	ColorSearchFile(const CSearchFile* content, CMuleMemDC* odc);
bool	IsBadResult(const CSearchFile* file, const SSearchParams* params, CString& reason);
// Tux: Feature: additional fake checks [end]