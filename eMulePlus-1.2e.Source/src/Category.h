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

#pragma warning(push)
#pragma warning(disable:4702) // unreachable code
#include <map>
#pragma warning(pop)

//	Predefined category IDs
enum _EnumCategories
{
	CAT_NONE = 0,
	CAT_FIRSTUSERCAT = 1,
	CAT_LASTUSERCAT = 99,

	CAT_PREDEFINED = 100,
	CAT_ALL = CAT_PREDEFINED,
	CAT_UNCATEGORIZED,
	CAT_INCOMPLETE,
	CAT_COMPLETED,
	CAT_WAITING,
	CAT_DOWNLOADING,
	CAT_ERRONEOUS,
	CAT_PAUSED,
	CAT_STOPPED,
	CAT_STALLED,
	CAT_ACTIVE,
	CAT_INACTIVE,

	CAT_LASTSTATUSCAT = CAT_INACTIVE,	// Update this if you add a new status category

	CAT_VIDEO,
	CAT_AUDIO,
	CAT_ARCHIVES,
	CAT_CDIMAGES,

	CAT_LASTPREDEFINEDCAT,
	CAT_TOTALPREDEFINEDCATS = CAT_LASTPREDEFINEDCAT-CAT_PREDEFINED, // NOTE: _total_ to distinguish it
																//		from _numPredefinedCats_ which would be
																//		the number visible

	CAT_MAXPREDEFINEDCATS = 155
};
typedef EnumDomain<_EnumCategories>		EnumCategories;

class CPreferences;
class CCatDialog;
class CPartFile;

class CCat
{
//	Making these two classes friends means we don't have to put public
//		setters on this class which would make our data members writable
//		by classes which shouldn't be mucking with them.
friend class	CPreferences;
friend class	CCatDialog;

public:
				CCat();
				CCat( LPCTSTR strTitle,
					  LPCTSTR strSavePath=NULL,
					  LPCTSTR strTempPath=NULL,
					  LPCTSTR strComment=NULL,
					  LPCTSTR strAutoCatExt=NULL );
				CCat(EnumCategories ePredefinedCatID);

private:
	CString				m_strSavePath;
	CString				m_strTempPath;
	CString				m_strTitle;
	CString				m_strComment;
	CString				m_strAutoCatExt;
	COLORREF			m_crColor;
	byte				m_iPriority;
	bool				m_bIsPredefined;
	EnumCategories		m_eCatID;

	typedef CArray<CCat*,CCat*>				CCatArray;
	typedef std::map<EnumCategories,byte>	CCatIDMap;

	static EnumCategories	g_eAllCatType;
	static CCatArray		g_arrCat;
	static CCatIDMap		g_mapCatID;
	static byte				g_iNumPredefinedCats;

public:
//	Accessors
	const CString	&GetPath() const		{ return m_strSavePath; }
	const CString	&GetTempPath() const	{ return m_strTempPath; }
	const CString	&GetTitle() const		{ return m_strTitle; }
	void SetTitle(const CString &strVal)	{ m_strTitle = strVal; }
	const CString	&GetComment() const		{ return m_strComment; }
	COLORREF		GetColor() const		{ return m_crColor; }
	byte			GetPriority() const		{ return m_iPriority; }
	const CString	&GetAutoCatExt() const	{ return m_strAutoCatExt; }
	bool			IsPredefined() const	{ return m_bIsPredefined; }
	EnumCategories	GetID() const			{ return m_eCatID; }

//	NOTE: Category indices are:					0 .. GetNumCats()-1
//		  Indices for user categories are:		1 .. GetNumUserCats()
//		  IDs for user categories are:			1 .. CAT_PREDEFINED-1
//		  IDs for predefined categories are:	CAT_PREDEFINED .. CAT_PREDEFINED + GetNumPredefinedCats()-1
	static void				Finalize();
	static int				AddCat(CCat* pCat, bool bAssignNewID = true);
	static int				AddPredefinedCat(CCat *pCat);
	static bool				MoveCat(unsigned uiFromIdx, unsigned uiToIdx);
	static void				RemoveCatByIndex(int iIndex);
	static int				GetNumCats()						{ return g_arrCat.GetCount(); }
	static int				GetNumUserCats()					{ return g_arrCat.GetCount() - g_iNumPredefinedCats; }
	static const TCHAR*		GetCatPathByIndex(int index)		{ return g_arrCat[index]->m_strSavePath; }
	static int				CatIndexToUserCatIndex(int index)	{ return index - g_iNumPredefinedCats + 1; }
	static int				UserCatIndexToCatIndex(int iUserIdx) { return iUserIdx - 1 + g_iNumPredefinedCats; }
	static COLORREF			GetCatColorByIndex(unsigned uiIdx)	{ return (uiIdx >= static_cast<unsigned>(g_arrCat.GetCount())) ? 0 : g_arrCat[uiIdx]->m_crColor; }
	static COLORREF			GetCatColorByID(const EnumCategories &eCatID)	{ CCat* pCat = GetCatByID(eCatID); return pCat == NULL ? 0 : pCat->GetColor(); }
	static EnumCategories	GetAllCatType()						{ return g_eAllCatType; }
	static void				SetAllCatType(const EnumCategories &e)	{ g_eAllCatType = e; }
	static bool				CatIndexIsValid(int iIndex)			{ return (iIndex >= 0 && iIndex < g_arrCat.GetCount()); }
	static byte				GetNumPredefinedCats()				{ return g_iNumPredefinedCats; }
	static EnumCategories	GetCatIDByIndex(int index)			{ return GetCatByIndex(index)!=NULL ? GetCatByIndex(index)->m_eCatID : CAT_NONE; }
	static EnumCategories	GetCatIDByUserIndex(int iUserIdx)	{ return GetCatIDByIndex(UserCatIndexToCatIndex(iUserIdx)); }
	static int				GetCatIndexByID(const EnumCategories &eCatID);
	static int				GetUserCatIndexByID(const EnumCategories &eCatID) { return CatIndexToUserCatIndex(GetCatIndexByID(eCatID)); }
	static CCat				*GetCatByIndex(int index)			{ if (CatIndexIsValid(index)) return g_arrCat[index]; else return NULL; }
	static CCat				*GetCatByUserIndex(int iUserIdx)	{ return GetCatByIndex(UserCatIndexToCatIndex(iUserIdx)); }
	static CCat				*GetCatByID(const EnumCategories &eCatID) { return GetCatByIndex(GetCatIndexByID(eCatID)); }
	static EnumCategories	GetNewCatID();
	static bool				FileBelongsToGivenCat(CPartFile *file,EnumCategories eCatID, bool bIgnoreViewFilter = false);
	static CString			GetPredefinedCatTitle(EnumCategories eCatID, int langID = 0);
};
