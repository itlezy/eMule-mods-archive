//this file is part of eMule
//Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

class CPartFile;

class CPreviewThread : public CWinThread
{
	DECLARE_DYNCREATE(CPreviewThread)

protected:
	CPreviewThread();           // protected constructor used by dynamic creation
	virtual ~CPreviewThread();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL	InitInstance();
	virtual int		Run();
	void	SetValues(CPartFile *pPartFile, const CString &strCmd, const CString &strCmdArgs);

private:
	CPartFile	*m_pPartFile;
	CString		m_strPlayerCmd;
	CString		m_strPlayerArgs;
};
