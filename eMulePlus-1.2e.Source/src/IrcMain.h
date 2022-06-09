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

#include "Loggable.h"

class CIrcSocket;
class CIrcWnd;

class CIrcMain : public CLoggable
{
public:
	CIrcMain(void);
	~CIrcMain(void);
	void ParseMessage(CString strRawMessage);
	void PreParseMessage(const char *pcBuf, int iLen);
	void SendLogin();
	void Connect();
	void Disconnect(bool bIsShuttingDown = false);
	void SetConnectStatus(bool bConnected);
	void SetIRCWnd(CIrcWnd* pWndIRC)	{m_pWndIRC = pWndIRC;}
	int SendString(const CString &strSend);
	void ParsePerform();
	CString GetNick() const				{ return m_strNick; }

private:
	CIrcSocket	*m_pIrcSocket;
	CIrcWnd		*m_pWndIRC;
	CStringA	m_strPreParseBufA;
	CString		m_strUser;
	CString		m_strNick;
};
