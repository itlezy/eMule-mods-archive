#pragma once
#include<map>
//#define HAVE_BOOST //morph4u outcomment because have no boost

#if defined(HAVE_BOOST) || defined(_STLP_WIN32) || _MSC_FULL_VER >= 150030304
#ifdef HAVE_BOOST
#include<boost/unordered_map.hpp>
#include<boost/unordered_set.hpp>
using namespace boost;
//#define REPLACE_ATLMAP
//#define REPLACE_MFCMAP
#elif defined(HAVE_3RD_PART_TR1)
#include<tr1/unordered_map>
#include<tr1/unordered_set>
using namespace std::tr1;
#else
#include<unordered_map>
#include<unordered_set>
using namespace std::tr1;
#endif
#define HAVE_UNORDERED
#define SET unordered_set
#define MAP unordered_map
#define MULTIMAP unordered_multimap

// Pred
#define LPSTR_Pred lpstr_unordered, lpstr_unordered
#define LPTSTR_Pred lptstr_unordered, lptstr_unordered

struct lpstr_unordered{
	size_t operator()(LPCSTR s) const{
		size_t hash = 0;
		while (*s!=0)
			hash = _rotl_ptr(hash, 11) + *s++;
		return hash;
	}
	bool operator()(LPCSTR s1, LPCSTR s2) const{
		return strcmp(s1, s2) == 0;
	}
};

struct lptstr_unordered{
	size_t operator()(LPCTSTR s) const{
		size_t hash = 0;
		while (*s!=0)
			hash = _rotl_ptr(hash, 11) + *s++;
		return hash;
	}
	bool operator()(LPCTSTR s1, LPCTSTR s2) const{
		return _tcscmp(s1, s2) == 0;
	}
};

struct cstring_unordered{
	size_t operator()(const CString&s) const{
		size_t hash = 1;
		for (int i = 0;i < s.GetLength();i++)
			hash = _rotl_ptr(hash, 11) + s.GetAt(i);
		return hash;
	}
	bool operator()(const CString& s1, const CString& s2) const{
		return s1.Compare(s2) == 0;
	}
};
#ifdef REPLACE_ATLMAP
typedef unordered_map<CString, CString, cstring_unordered, cstring_unordered> CStringToStringMap;
typedef unordered_map<CString, HBITMAP, cstring_unordered, cstring_unordered> CStringToHBITMAPMap;
#ifdef USE_METAFILE
typedef unordered_map<CString, HENHMETAFILE, cstring_unordered, cstring_unordered> CStringToHMETAFILEMap;
#endif
typedef unordered_map<CString, int, cstring_unordered, cstring_unordered> CStringToIntMap;
typedef unordered_map<CString, uint_ptr, cstring_unordered, cstring_unordered> CStringToUIntPTRMap;
#else
typedef CAtlMap<CString, CString, CStringElementTraitsI<CString>, CStringElementTraitsI<CString>> CStringToStringMap;
typedef CAtlMap<CString, HBITMAP, CStringElementTraitsI<CString>> CStringToHBITMAPMap;
#ifdef USE_METAFILE
typedef CAtlMap<CString, HENHMETAFILE, CStringElementTraitsI<CString>> CStringToHMETAFILEMap;
#endif
typedef CAtlMap<CString, int, CStringElementTraitsI<CString>> CStringToIntMap;
typedef CAtlMap<CString, uint_ptr, CStringElementTraitsI<CString>> CStringToUIntPTRMap;
#endif
#else
#define SET std::set
#define MAP std::map
#define MULTIMAP std::multimap

// Pred
#define LPSTR_Pred lpstr_map
#define LPTSTR_Pred lptstr_map

struct lpstr_map{
	bool operator()(LPCSTR s1, LPCSTR s2) const{
		return strcmp(s1, s2) < 0;
	}
};
struct lptstr_map{
	bool operator()(LPCTSTR s1, LPCTSTR s2) const{
		return _tcscmp(s1, s2) < 0;
	}
};
#endif