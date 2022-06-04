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
#pragma once
#include "shahashset.h"
class CSafeMemFile;

struct SUnresolvedHostname
{
	SUnresolvedHostname()
	{
		nPort = 0;
	}
	CStringA strHostname;
	uint16 nPort;
	CString strURL;
};


class CED2KLink
{
public:
	static CED2KLink* CreateLinkFromUrl(const TCHAR* url);
	virtual ~CED2KLink();

	// MORPH START - Modified by Commander, Friendlinks [emulEspaa] - added by zz_fly
	/*
	typedef enum { kServerList, kServer , kFile , kNodesList, kSearch, kInvalid } LinkType;
	*/
	typedef enum { kServerList, kServer , kFile , kNodesList, kSearch, kFriend, kFriendList, kInvalid } LinkType;
	// MORPH END - Modified by Commander, Friendlinks [emulEspaa]

	virtual LinkType GetKind() const = 0;
	virtual void GetLink(CString& lnk) const = 0;
	virtual class CED2KServerListLink* GetServerListLink() = 0;
	virtual class CED2KServerLink* GetServerLink() = 0;
	virtual class CED2KFileLink* GetFileLink() = 0;
	virtual class CED2KNodesListLink* GetNodesListLink() = 0;
	virtual class CED2KSearchLink* GetSearchLink() = 0;
};


class CED2KServerLink : public CED2KLink
{
public:
	CED2KServerLink(const TCHAR* ip,const TCHAR* port);
	virtual ~CED2KServerLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();
	virtual CED2KNodesListLink* GetNodesListLink()		{ return NULL; }
	virtual CED2KSearchLink* GetSearchLink()			{ return NULL; }

	const CString& GetAddress() const { return m_strAddress; }
	uint16 GetPort() const { return m_port;}
	void GetDefaultName(CString& defName) const { defName = m_defaultName; }

private:
	CED2KServerLink();
	CED2KServerLink(const CED2KServerLink&);
	CED2KServerLink& operator=(const CED2KServerLink&);

	CString m_strAddress;
	uint16 m_port;
	CString m_defaultName;
};


class CED2KFileLink : public CED2KLink
{
public:
	CED2KFileLink(const TCHAR* pszName, const TCHAR* pszSize, const TCHAR* pszHash, const CStringArray& param, const TCHAR* pszSources);
	virtual ~CED2KFileLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();
	virtual CED2KNodesListLink* GetNodesListLink() 		{ return NULL; }
	virtual CED2KSearchLink* GetSearchLink()			{ return NULL; }
	
	const TCHAR* GetName() const			{ return m_name; }
	const uchar* GetHashKey() const			{ return m_hash;}
	const CAICHHash& GetAICHHash() const	{ return m_AICHHash;}
	EMFileSize GetSize() const				{ return (uint64)_tstoi64(m_size); }	
	bool HasValidSources() const			{ return (SourcesList != NULL); }
	bool HasHostnameSources() const			{ return (!m_HostnameSourcesList.IsEmpty()); }
	bool HasValidAICHHash() const			{ return m_bAICHHashValid; }

	CSafeMemFile* SourcesList;
	CSafeMemFile* m_hashset;
	CTypedPtrList<CPtrList, SUnresolvedHostname*> m_HostnameSourcesList;

private:
	CED2KFileLink();
	CED2KFileLink(const CED2KFileLink&);
	CED2KFileLink& operator=(const CED2KFileLink&);

	CString m_name;
	CString m_size;
	uchar	m_hash[16];
	bool	m_bAICHHashValid;
	CAICHHash m_AICHHash;
};


class CED2KServerListLink : public CED2KLink
{
public:
	CED2KServerListLink(const TCHAR* pszAddress);
	virtual ~CED2KServerListLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();
	virtual CED2KNodesListLink* GetNodesListLink()		{ return NULL; }
	virtual CED2KSearchLink* GetSearchLink()			{ return NULL; }

	const TCHAR* GetAddress() const { return m_address; }

private:
	CString m_address;
};


class CED2KNodesListLink : public CED2KLink
{
public:
	CED2KNodesListLink(const TCHAR* pszAddress);
	virtual ~CED2KNodesListLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();
	virtual CED2KNodesListLink* GetNodesListLink();
	virtual CED2KSearchLink* GetSearchLink()			{ return NULL; }

	const TCHAR* GetAddress() const { return m_address; }

private:
	CString m_address;
};

class CED2KSearchLink : public CED2KLink
{
public:
	CED2KSearchLink(const TCHAR* pszSearchTerm);
	virtual ~CED2KSearchLink();

	virtual LinkType GetKind() const;
	virtual void GetLink(CString& lnk) const;
	virtual CED2KServerListLink* GetServerListLink();
	virtual CED2KServerLink* GetServerLink();
	virtual CED2KFileLink* GetFileLink();
	virtual CED2KNodesListLink* GetNodesListLink();
	virtual CED2KSearchLink* GetSearchLink();

	CString GetSearchTerm() const { return m_strSearchTerm; }

private:
	CString m_strSearchTerm;
};

// MORPH START - Added by Commander, Friendlinks [emulEspaa] - added by zz_fly
class CED2KFriendLink : public CED2KLink
{
public:
	CED2KFriendLink(LPCTSTR userName, LPCTSTR userHash);
	CED2KFriendLink(LPCTSTR userName, uchar userHash[]);
	virtual ~CED2KFriendLink()	{ }

	virtual LinkType	GetKind() const;
	virtual void	GetLink(CString& lnk) const;
	virtual CED2KServerListLink*	GetServerListLink();
	virtual CED2KServerLink*		GetServerLink();
	virtual CED2KFileLink*			GetFileLink();
	virtual CED2KNodesListLink* GetNodesListLink()		{ return NULL; }
	virtual CED2KSearchLink* GetSearchLink()			{ return NULL; }

	CString	GetUserName() const						{ return m_sUserName; }
	void	GetUserHash(uchar userHash[]) const		{ memcpy(userHash, m_hash, 16*sizeof(uchar)); }

private:
	CString	m_sUserName;
	uchar	m_hash[16];
};

class CED2KFriendListLink : public CED2KLink
{
public:
	CED2KFriendListLink(LPCTSTR address);
	virtual ~CED2KFriendListLink()	{ }

	virtual LinkType GetKind() const;
	virtual void	GetLink(CString& lnk) const;
	virtual CED2KServerListLink*	GetServerListLink();
	virtual CED2KServerLink*		GetServerLink();
	virtual CED2KFileLink*			GetFileLink();
	virtual CED2KNodesListLink* GetNodesListLink()		{ return NULL; }
	virtual CED2KSearchLink* GetSearchLink()			{ return NULL; }

	CString	GetAddress() const		{ return m_address; }

private:
	CString	m_address;
};
// MORPH END - Added by Commander, Friendlinks [emulEspaa]