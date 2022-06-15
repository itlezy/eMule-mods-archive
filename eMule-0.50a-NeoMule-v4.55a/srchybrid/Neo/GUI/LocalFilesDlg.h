//this file is part of eMule
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

// NEO: XSF - [ExtendedSharedFiles] -- Xanatos -->

#pragma once
#include "ModeLess.h" // NEO: MLD - [ModelesDialogs] 
#include "SharedFilesPage.h"

//////////////////////////////////////////////////////////////////////////////
// CLocalFilesDialog

class CLocalFilesDialog : public CModResizableSheet // NEO: MLD - [ModelesDialogs] 
{
	DECLARE_DYNAMIC(CLocalFilesDialog)

public:
	CLocalFilesDialog();
	virtual ~CLocalFilesDialog();

	void	UpdateAll(); // NEO: VSF - [VirtualSharedFiles]

protected:
	CSharedFilesPage m_wndSharedFiles;

	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
	afx_msg void OnDestroy();
};

// NEO: XSF END <-- Xanatos --
