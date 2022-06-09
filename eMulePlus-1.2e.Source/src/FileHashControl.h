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

class CProcessingCmdThread;

class CFileHashControl
{
public:
	CFileHashControl(void);
	virtual ~CFileHashControl(void);

	HRESULT Init(void);
	HRESULT Destroy(void);

//	Adds file to the hash thread queue to be hashed
	HRESULT AddToHash(const CString &strInFolder, const CString &strFileName);
	HRESULT KillThread(void);

	void SetThreadPriority(int iPriority);

protected:
	HRESULT	CreateProcessingThread(void);

	CProcessingCmdThread	*m_pProcessThread;
	int		m_iThreadPriority;
	bool	m_bInitialized;
};
