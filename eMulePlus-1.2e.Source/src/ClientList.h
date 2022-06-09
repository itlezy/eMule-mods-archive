//	this file is part of eMule
//	Copyright (C)2002 Merkur ( merkur-@users.sourceforge.net / http://www.emule-project.net )
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

#ifdef OLD_SOCKETS_ENABLED
#include "ListenSocket.h"
#endif //OLD_SOCKETS_ENABLED
#include "loggable.h"

typedef CMap<uint32, uint32, uint32, uint32>	ClientInfoList;

class CUpDownClient;

class ClientsData
{
public:
	ClientInfoList* m_pClients;

	ClientsData()
	{
	//	+1 so on SO_LAST position we hold all clients data
		m_pClients	= new ClientInfoList[SO_LAST + 1];
	};
	virtual ~ClientsData()
	{
		if (m_pClients)
		{
			for (int i = 0; i < SO_LAST + 1; i++)
			{
				m_pClients[i].RemoveAll();
			}
			delete []m_pClients;
		}
	}
};

class CClientList : public CLoggable
{
	friend class CClientListCtrl;

public:
	CClientList();
	~CClientList();

	void	SetClientListCtrl(CClientListCtrl *pctlClientList)
	{
		m_pctlClientList = pctlClientList;
	}

	bool	AddClient(CUpDownClient* toadd, bool bSkipDupTest = false);
#ifdef OLD_SOCKETS_ENABLED

	bool	AttachToAlreadyKnown(CUpDownClient** client, CClientReqSocket* sender);
#endif //OLD_SOCKETS_ENABLED

	void	RemoveClient(CUpDownClient *pClient);
	void	UpdateClient(CUpDownClient *pClient);
	void	DeleteAll();
//	Statistical functions
	void	GetStatistics(uint32 &totalclient, uint32 stats[], ClientsData *pData, uint32 &totalMODs, uint32 *pdwTotalPlusMODs, CMap<POSITION, POSITION, uint32, uint32> *MODs, CMap<POSITION, POSITION, uint32, uint32> *PlusMODs, CMap<int, int, uint32, uint32> *pCountries);
	uint32	GetA4AFSourcesCount();
	int	GetClientCount()
	{
		return m_clientList.GetCount();
	}

	void GetMODType(POSITION pos_in, CString *pstrOut)
	{
		if (pos_in != 0)
			*pstrOut = liMODsTypes.GetAt(pos_in);
		else
			*pstrOut = _T("");
	}

	bool	IsValidClient(CUpDownClient* tocheck);
	CUpDownClient* FindClientByIP(uint32 clientip, uint16 port);
	CUpDownClient* FindClientByUserHash(uchar* clienthash);
	CUpDownClient* FindClient(uint32 dwUserIDHyb, uint16 uUserPort, uint32 dwSrvIP, uint16 uSrvPort, uchar *pbyteUserHash);
	
	void	GetClientListByFileID(CTypedPtrList<CPtrList, CUpDownClient*> *pClientList, const uchar *fileid);
#ifdef OLD_SOCKETS_ENABLED

	void	Debug_SocketDeleted(CClientReqSocket* deleted);
#endif //OLD_SOCKETS_ENABLED
	void	ResetIP2Country();

	void	UpdateBanCounters();

private:
	typedef CTypedPtrList<CPtrList, CUpDownClient*>	ClientList;

	CClientListCtrl		   *m_pctlClientList;
	ClientList				m_clientList;

protected:
	CList<CString, CString&> liMODsTypes;
};
