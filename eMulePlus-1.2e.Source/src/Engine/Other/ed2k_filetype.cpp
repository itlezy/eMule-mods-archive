#include "stdafx.h"
#include "ed2k_filetype.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
static SED2KFileType _aED2KFileTypes[] =
{
	{ _T(".669"),   ED2KFT_AUDIO },
	{ _T(".aac"),	ED2KFT_AUDIO },
	{ _T(".aif"),	ED2KFT_AUDIO },
	{ _T(".aiff"),	ED2KFT_AUDIO },
	{ _T(".amf"),   ED2KFT_AUDIO },
	{ _T(".ams"),   ED2KFT_AUDIO },
	{ _T(".ape"),	ED2KFT_AUDIO },
	{ _T(".au"),	ED2KFT_AUDIO },
	{ _T(".dbm"),   ED2KFT_AUDIO },
	{ _T(".dmf"),   ED2KFT_AUDIO },
	{ _T(".dsm"),   ED2KFT_AUDIO },
	{ _T(".far"),   ED2KFT_AUDIO },
	{ _T(".flac"),	ED2KFT_AUDIO },
	{ _T(".it"),    ED2KFT_AUDIO },
	{ _T(".mdl"),   ED2KFT_AUDIO },
	{ _T(".med"),   ED2KFT_AUDIO },
	{ _T(".mid"),	ED2KFT_AUDIO },
	{ _T(".midi"),	ED2KFT_AUDIO },
	{ _T(".mod"),   ED2KFT_AUDIO },
	{ _T(".mol"),   ED2KFT_AUDIO },
	{ _T(".mp1"),	ED2KFT_AUDIO },
	{ _T(".mp2"),	ED2KFT_AUDIO },
	{ _T(".mp3"),	ED2KFT_AUDIO },
	{ _T(".mp4"),	ED2KFT_AUDIO },
	{ _T(".mpa"),	ED2KFT_AUDIO },
	{ _T(".mpc"),	ED2KFT_AUDIO },
	{ _T(".mpp"),	ED2KFT_AUDIO },
	{ _T(".mtm"),   ED2KFT_AUDIO },
	{ _T(".nst"),   ED2KFT_AUDIO },
	{ _T(".ogg"),	ED2KFT_AUDIO },
	{ _T(".okt"),   ED2KFT_AUDIO },
	{ _T(".psm"),   ED2KFT_AUDIO },
	{ _T(".ptm"),   ED2KFT_AUDIO },
	{ _T(".ra"),	ED2KFT_AUDIO },
	{ _T(".rmi"),	ED2KFT_AUDIO },
	{ _T(".s3m"),   ED2KFT_AUDIO },
	{ _T(".stm"),   ED2KFT_AUDIO },
	{ _T(".ult"),   ED2KFT_AUDIO },
	{ _T(".umx"),   ED2KFT_AUDIO },
	{ _T(".wav"),	ED2KFT_AUDIO },
	{ _T(".wma"),	ED2KFT_AUDIO },
	{ _T(".wow"),   ED2KFT_AUDIO },
	{ _T(".xm"),    ED2KFT_AUDIO },

	{ _T(".asf"),	ED2KFT_VIDEO },
	{ _T(".avi"),	ED2KFT_VIDEO },
	{ _T(".divx"),	ED2KFT_VIDEO },
	{ _T(".m1v"),	ED2KFT_VIDEO },
	{ _T(".m2v"),	ED2KFT_VIDEO },
	{ _T(".mkv"),	ED2KFT_VIDEO },
	{ _T(".mov"),	ED2KFT_VIDEO },
	{ _T(".mp1v"),	ED2KFT_VIDEO },
	{ _T(".mp2v"),	ED2KFT_VIDEO },
	{ _T(".mpe"),	ED2KFT_VIDEO },
	{ _T(".mpeg"),	ED2KFT_VIDEO },
	{ _T(".mpg"),	ED2KFT_VIDEO },
	{ _T(".mps"),	ED2KFT_VIDEO },
	{ _T(".mpv"),	ED2KFT_VIDEO },
	{ _T(".mpv1"),	ED2KFT_VIDEO },
	{ _T(".mpv2"),	ED2KFT_VIDEO },
	{ _T(".ogm"),	ED2KFT_VIDEO },
	{ _T(".qt"),	ED2KFT_VIDEO },
	{ _T(".ram"),	ED2KFT_VIDEO },
	{ _T(".rm"),	ED2KFT_VIDEO },
	{ _T(".rv"),	ED2KFT_VIDEO },
	{ _T(".rv9"),	ED2KFT_VIDEO },
	{ _T(".ts"),	ED2KFT_VIDEO },
	{ _T(".vivo"),	ED2KFT_VIDEO },
	{ _T(".vob"),	ED2KFT_VIDEO },
	{ _T(".wmv"),	ED2KFT_VIDEO },
	{ _T(".xvid"),	ED2KFT_VIDEO },

	{ _T(".bmp"),	ED2KFT_IMAGE },
	{ _T(".dcx"),	ED2KFT_IMAGE },
	{ _T(".emf"),	ED2KFT_IMAGE },
	{ _T(".gif"),	ED2KFT_IMAGE },
	{ _T(".ico"),	ED2KFT_IMAGE },
	{ _T(".jpeg"),	ED2KFT_IMAGE },
	{ _T(".jpg"),	ED2KFT_IMAGE },
	{ _T(".pct"),	ED2KFT_IMAGE },
	{ _T(".pcx"),	ED2KFT_IMAGE },
	{ _T(".pic"),	ED2KFT_IMAGE },
	{ _T(".pict"),	ED2KFT_IMAGE },
	{ _T(".png"),	ED2KFT_IMAGE },
	{ _T(".psd"),	ED2KFT_IMAGE },
	{ _T(".psp"),	ED2KFT_IMAGE },
	{ _T(".tga"),	ED2KFT_IMAGE },
	{ _T(".tif"),	ED2KFT_IMAGE },
	{ _T(".tiff"),	ED2KFT_IMAGE },
	{ _T(".wmf"),	ED2KFT_IMAGE },
	{ _T(".xif"),	ED2KFT_IMAGE },

	{ _T(".ace"),	ED2KFT_ARCHIVE },
	{ _T(".arj"),	ED2KFT_ARCHIVE },
	{ _T(".gz"),	ED2KFT_ARCHIVE },
	{ _T(".hqx"),	ED2KFT_ARCHIVE },
	{ _T(".lha"),	ED2KFT_ARCHIVE },
	{ _T(".rar"),	ED2KFT_ARCHIVE },
	{ _T(".sea"),	ED2KFT_ARCHIVE },
	{ _T(".sit"),	ED2KFT_ARCHIVE },
	{ _T(".tar"),	ED2KFT_ARCHIVE },
	{ _T(".tgz"),	ED2KFT_ARCHIVE },
	{ _T(".uc2"),	ED2KFT_ARCHIVE },
	{ _T(".zip"),	ED2KFT_ARCHIVE },

	{ _T(".bat"),	ED2KFT_PROGRAM },
	{ _T(".cmd"),	ED2KFT_PROGRAM },
	{ _T(".com"),	ED2KFT_PROGRAM },
	{ _T(".exe"),	ED2KFT_PROGRAM },

	{ _T(".bin"),	ED2KFT_CDIMAGE },
	{ _T(".bwa"),	ED2KFT_CDIMAGE },
	{ _T(".bwi"),	ED2KFT_CDIMAGE },
	{ _T(".bws"),	ED2KFT_CDIMAGE },
	{ _T(".bwt"),	ED2KFT_CDIMAGE },
	{ _T(".ccd"),	ED2KFT_CDIMAGE },
	{ _T(".cue"),	ED2KFT_CDIMAGE },
	{ _T(".dmg"),	ED2KFT_CDIMAGE },
	{ _T(".dmz"),	ED2KFT_CDIMAGE },
	{ _T(".img"),	ED2KFT_CDIMAGE },
	{ _T(".iso"),	ED2KFT_CDIMAGE },
	{ _T(".mdf"),	ED2KFT_CDIMAGE },
	{ _T(".mds"),	ED2KFT_CDIMAGE },
	{ _T(".nrg"),	ED2KFT_CDIMAGE },
	{ _T(".sub"),	ED2KFT_CDIMAGE },
	{ _T(".toast"), ED2KFT_CDIMAGE },

	// To be uncommented after we use the 'Doc' ed2k filetype for search expressions
	{ _T(".txt"),   ED2KFT_DOCUMENT },
	{ _T(".nfo"),   ED2KFT_DOCUMENT },
	{ _T(".diz"),   ED2KFT_DOCUMENT },
	{ _T(".doc"),   ED2KFT_DOCUMENT },
	{ _T(".rtf"),   ED2KFT_DOCUMENT },
	{ _T(".pdf"),   ED2KFT_DOCUMENT },	// double check this!
	{ _T(".xls"),   ED2KFT_DOCUMENT },	// double check this!
	{ _T(".html"),  ED2KFT_DOCUMENT },
	{ _T(".htm"),   ED2KFT_DOCUMENT }
};
///////////////////////////////////////////////////////////////////////////////////////////////////////
bool InitFileTypeArray(void)
{
	qsort(_aED2KFileTypes, _countof(_aED2KFileTypes), sizeof _aED2KFileTypes[0], CompareE2DKFileType);
#ifdef _DEBUG
	// check for duplicate entries
	LPCTSTR pszLast = _aED2KFileTypes[0].pszExt;
	for (int i = 1; i < _countof(_aED2KFileTypes); i++)
	{
		ASSERT( _tcscmp(pszLast, _aED2KFileTypes[i].pszExt) != 0 );
		pszLast = _aED2KFileTypes[i].pszExt;
	}
#endif
	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
int __cdecl CompareE2DKFileType(const void* p1, const void* p2)
{
	return _tcscmp( ((const SED2KFileType*)p1)->pszExt, ((const SED2KFileType*)p2)->pszExt );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////
EED2KFileType GetED2KFileTypeID(LPCTSTR pszFileName)
{
	LPCTSTR pszExt = _tcsrchr(pszFileName, _T('.'));

	if (pszExt == NULL)
		return ED2KFT_ANY;

	CString strExt(pszExt);

	strExt.MakeLower();

	SED2KFileType ft;

	ft.pszExt = strExt;
	ft.iFileType = ED2KFT_ANY;
	
	const SED2KFileType	*pFound = (SED2KFileType*)bsearch(&ft, _aED2KFileTypes, _countof(_aED2KFileTypes), sizeof _aED2KFileTypes[0], CompareE2DKFileType);
	
	if (pFound != NULL)
		return pFound->iFileType;

	return ED2KFT_ANY;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////