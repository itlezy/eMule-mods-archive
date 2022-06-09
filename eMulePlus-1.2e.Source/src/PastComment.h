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

enum _EnumPartFileRating
{
	PF_RATING_NONE = 0,
	PF_RATING_FAKE,
	PF_RATING_POOR,
	PF_RATING_GOOD,			//Fair and Good are reversed for some reason
	PF_RATING_FAIR,
	PF_RATING_EXCELLENT
};
typedef EnumDomain<_EnumPartFileRating>		EnumPartFileRating;

class CPastComment
{
public:
	CPastComment();
	CPastComment(const uchar client[16]);
	CPastComment( const uchar client[16], const CString &strClientName, const CString &strFileName,
					const CString &strComment, EnumPartFileRating eRating );
	~CPastComment() {}

	const CString& GetClientName() const			{ return m_strClientName; }
	const CString& GetFileName() const				{ return m_strFileName; }
	const CString& GetComment() const				{ return m_strComment; }
	EnumPartFileRating GetRating() const			{ return m_eRating; }
	const uchar* GetClientHash() const				{ return m_ClientHash; }

	bool operator==(const CPastComment &pc) const;

protected:
	uchar				m_ClientHash[16];
	CString				m_strClientName;
	CString				m_strFileName;
	CString				m_strComment;
	EnumPartFileRating	m_eRating;
};

#ifndef NEW_SOCKETS_TRAY
class CPastCommentList : public CList<CPastComment, CPastComment&>
{
public:
	CPastCommentList() {}
	~CPastCommentList() {}

	void Add(const uchar client[16], const CString &strClientName, const CString &strFileName, const CString &strComment, EnumPartFileRating eRating);
	void Remove(const uchar client[16]);
	POSITION FindComment(const uchar client[16]) const;

	bool GetCommentRating(const uchar client[16], CString *pstrComment, EnumPartFileRating *peRating) const;
	const CString& GetFileName(const uchar client[16]);
	const CString& GetComment(const uchar client[16]);
	EnumPartFileRating GetRate(const uchar client[16]);
};
#endif //NEW_SOCKETS_TRAY
