// Tux: Feature: additional fake checks [start]
//Copyright (C)2012 WiZaRd ( strEmail.Format("%s@%s", "thewizardofdos", "gmail.com") / http://www.emulefuture.de )
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
#include "stdafx.h"
#include "AntiFake.h"
#include "SearchFile.h"
#include "opcodes.h"
#include "MemDC.h"
#include "otherfunctions.h"
#include "SearchParams.h"

int		GetFakeAlyzerRating(const CSearchFile* content, CString* ret)
{	
	UINT nPositives = 0;
	UINT nNegatives = 0;

	// Check for matches in file name against given tags (if given)
	bool isNameMismatch = false;
	CString strFileName = content->GetFileName();	
	CString strArtist = content->GetStrTagValue(FT_MEDIA_ARTIST);
	CString strAlbum = content->GetStrTagValue(FT_MEDIA_ALBUM);
	CString strTitle = content->GetStrTagValue(FT_MEDIA_TITLE);

	// this filters ".avi.exe" files and similar that show up as "video" files
	const CString strGivenType = content->GetFileType();
	const CString strType = GetFileTypeByName(strFileName);
	if (strType != strGivenType)
	{
		if (ret)
			ret->Format(L"%s-Type mismatch!", ret->IsEmpty() ? L"" : L"\n\t");
		++nNegatives;
	}

	if (!strArtist.IsEmpty() || !strAlbum.IsEmpty() || !strTitle.IsEmpty())
	{
		if (content->GetFileType() == ED2KFTSTR_AUDIO || content->GetFileType() == ED2KFTSTR_VIDEO)
		{
			strFileName.MakeLower();
			if (!strArtist.IsEmpty() && strFileName.Find(strArtist.MakeLower()) == -1)
				isNameMismatch = true;
			else if (!strAlbum.IsEmpty() && strFileName.Find(strAlbum.MakeLower()) == -1)
				isNameMismatch = true;
			else if (!strTitle.IsEmpty() && strFileName.Find(strTitle.MakeLower()) == -1)
				isNameMismatch = true;
			if (!isNameMismatch)
			{
				if(ret)
					ret->Format(L"%s+No name mismatch detected!", ret->IsEmpty() ? L"" : L"\n\t");
				//				++nPositives; //don't count that as "positive"
			}
			else
			{
				if(ret)
					ret->Format(L"%s-Name mismatch!", ret->IsEmpty() ? L"" : L"\n\t");
				++nNegatives;
			}
		}
		else if(!content->GetFileType().IsEmpty()) 
		{
			if(ret)		
				ret->Format(L"%s-No audio/video file with audio/video tags!", ret->IsEmpty() ? L"" : L"\n\t");
			++nNegatives;
		}
	}

	const UINT nBitrate = content->GetIntTagValue(FT_MEDIA_BITRATE);
	UINT nAvgBitrate = nBitrate;
	if (content->GetIntTagValue(FT_MEDIA_LENGTH) > 0)
		nAvgBitrate = (UINT)(uint64) (content->GetFileSize() / 128ULL) / content->GetIntTagValue(FT_MEDIA_LENGTH);
	const CString strCodec = content->GetStrTagValue(FT_MEDIA_CODEC);
	if (nBitrate || nAvgBitrate || !strCodec.IsEmpty())
	{
		if (content->GetFileType() == ED2KFTSTR_AUDIO || content->GetFileType() == ED2KFTSTR_VIDEO)
		{
			if (content->GetFileType() == ED2KFTSTR_AUDIO)
			{
				const bool isMP3 = content->GetFileName().Right(4).MakeLower() == L".mp3";
				// No serious files have lower rate than this
				if (isMP3 && nBitrate >= 128 && nBitrate % 8 == 0 && nBitrate < 1500 && nBitrate <= 2 * nAvgBitrate)
				{
					if(ret)
						ret->AppendFormat(L"%s+Good bitrate for .mp3 file!", ret->IsEmpty() ? L"" : L"\n\t");
					++nPositives;
				}				
				// Illegal!?
				else if ((isMP3 && nBitrate < 56) || (nBitrate > 4 * nAvgBitrate) || (nBitrate % 2 != 0))
				{
					if(ret)
						ret->AppendFormat(L"%s-Illegal/bad bitrate for .mp3 file!", ret->IsEmpty() ? L"" : L"\n\t");
					++nNegatives;
				}				
			}
			else //if (content->GetFileType() == ED2KFTSTR_VIDEO)
			{
				// The good ones are usually within these bounds
				if (nAvgBitrate >= 900 && nAvgBitrate < 30000 && nBitrate <= 2 * nAvgBitrate) 
				{
					if(ret)
						ret->AppendFormat(L"%s+Good bitrate for video file!", ret->IsEmpty() ? L"" : L"\n\t");
					++nPositives;
				}
				// Should not be possible!
				else if (nBitrate > 4 * nAvgBitrate)
				{
					if(ret)
						ret->AppendFormat(L"%s-Illegal/bad bitrate for video file!", ret->IsEmpty() ? L"" : L"\n\t");
					++nNegatives;
				}				
			}
		}
		// Only audio and video files have bitrate, so it must be fake!
		else if(!content->GetFileType().IsEmpty()) 
		{	
			if(ret)		
				ret->AppendFormat(L"%s-No audio/video file with bitrate!", ret->IsEmpty() ? L"" : L"\n\t");
			++nNegatives;
		}

		// 		// common fake file identifier
		//		// (not used very much anymore and will give a false positive on approx. 1 file out of 137)
		// 		if (((uint64) content->GetFileSize() % 137ULL) == 0)
		// 		{
		// 			if(ret)
		// 				ret->AppendFormat(L"%ssize %% 137 == 0", ret->IsEmpty() ? L"" : L"\n\t");
		// 			++nNegatives;
		// 		}
	}

	// added a quick check for common bad codecs
	CString strExtension = GetExtension(strFileName);
	strExtension.MakeLower();
	if(strExtension == L".wma" || strExtension == L".wmv")
	{
		if(ret)		
			ret->AppendFormat(L"%s-Known bad (DRM protected) filetype (%s)", ret->IsEmpty() ? L"" : L"\n\t", strExtension);
		++nNegatives;
	}
	if(StrStrI(strCodec, L"wmaudio2") || StrStrI(strCodec, L"wmv3"))
	{
		if(ret)		
			ret->AppendFormat(L"%s-Known bad (DRM protected) codec (%s)", ret->IsEmpty() ? L"" : L"\n\t", strCodec);
		++nNegatives;
	}

	// reasonable availability (KAD only!)
	if (content->IsKademlia())
	{
		const UINT nAvailability = content->GetSourceCount();
		const UINT nKnownPublisher = (content->GetKadPublishInfo() & 0x00FF0000) >> 16;
		if (nKnownPublisher > 0 && nKnownPublisher < 100 && (10 * nKnownPublisher) < nAvailability)
		{
			if(ret)
				ret->AppendFormat(L"%s-Way too many publishers!", ret->IsEmpty() ? L"" : L"\n\t");
			++nNegatives;
		}
		else if (2 * nKnownPublisher > nAvailability || nKnownPublisher >= 100)
		{
			if(ret)
				ret->AppendFormat(L"%s+Good publisher quota!", ret->IsEmpty() ? L"" : L"\n\t");
			++nPositives;
		}
	}

	// merge positives and negatives into a score 
	int nScore = nPositives;
	if(nNegatives)
		nScore -= nNegatives;

	if (nScore < -1)
		return FA_FAKE;
	if (nScore < 0)
		return FA_SUSPECT;
	if (nScore > 0)
		return FA_OK;
	if (nScore > 1)
		return FA_GOOD;
	return FA_UNKNOWN;
}

bool	ColorSearchFile(const CSearchFile* content, CMuleMemDC* dc)
{
	if(content == NULL)
		return false;

	const COLORREF cr = dc->GetBkColor();
	switch(GetFakeAlyzerRating(content, NULL))
	{
		case FA_FAKE:
			dc->FillBackground(RGB(GetRValue(cr), GetGValue(cr)*0.85, GetBValue(cr)*0.85));
			return true;

		case FA_SUSPECT:
			dc->FillBackground(RGB(GetRValue(cr), GetGValue(cr), GetBValue(cr)*0.85));
			return true;

		case FA_GOOD:
			dc->FillBackground(RGB(GetRValue(cr)*0.85, GetGValue(cr), GetBValue(cr)*0.85));
			return true;

		case FA_OK:
			dc->FillBackground(RGB(GetRValue(cr)*0.85, GetGValue(cr)*0.85, GetBValue(cr)));
			return true;

		case FA_UNKNOWN:
		default:
			return false;
	}
}

CString	GetFakeComment(const CSearchFile* content, const bool bSimple, int* i)
{
	CString ret = L"";
	int r = GetFakeAlyzerRating(content, &ret);
	if(i != NULL)
		*i = r;
	if(bSimple)
	{
		switch(r)
		{
			default:
			case FA_UNKNOWN:
				return L"NEUTRAL";
			case FA_OK:
				return L"OK";
			case FA_GOOD:
				return L"GOOD";
			case FA_SUSPECT:
				return L"SUSPECT";
			case FA_FAKE:		
				return L"FAKE";
		}
	}
	if(ret.IsEmpty())
		ret = L"-";
	return ret;
}

bool	IsKnownSpamResult(const CString& strFilename)
{
	//Share IOS16-64-v257 and any other file from your computer with your friends without downloading it -- using SeeToo.zip
	//drop if the download starts with this string
	static const CString combo1[] = 
	{
		L"Secured Downloading of",
	};
	//drop if found somewhere in the string
	static const CString combo2[] = 
	{
		L"New Secured",
		L"BitTorrent downloader",
		L"multimedia toolbar",
		L"Web Hottest Videos Personal Player",
		L"best ultimate player",
		L"Share Accelerator",
		L"genuine advantage validation",
	};
	
	CString strName = L"";
	const int pos = strFilename.ReverseFind(L'.');
	if (pos != -1)
		strName = strFilename.Left(pos);
	else
		strName = strFilename;
	strName.Replace(L".", L" ");
	strName.Replace(L"_", L" ");

	for(int i = 0; i < ARRSIZE(combo1); ++i)
	{
		if(_tcsnicmp(strName, combo1[i], combo1[i].GetLength()) == 0)
			return true;
	}
	for(int i = 0; i < ARRSIZE(combo2); ++i)
	{
		if(StrStrI(strName, combo2[i]))
			return true;
	}

// 	//TODO: hash filter
// 	//virus file: size: 3913342 - hash: C5E7E2B92F48C7875E5E68DCB52AC6C8
// 	if(uHash != NULL)
// 	{		
// 		static const CString refhash[] =
// 		{	
// 			L"C5E7E2B92F48C7875E5E68DCB52AC6C8"	//known virus file
// 		};
// 
// 		const CString strHash = md4str(uHash);
// 		for(int i = 0; i < ARRSIZE(refhash); ++i)
// 			if(_tcsicmp(strHash, refhash[i]) == 0)
// 				return true;
// 	}
	return false;
}

bool	IsBadResult(const CSearchFile* file, const SSearchParams* params, CString& reason)
{
	if(IsKnownSpamResult(file->GetFileName()))
	{
		reason.Format(L"detected via spam list");
		return true;
	}

	if(params == NULL)
		return false;

//Note: the single checks are intentional! 
//		I know that I *could* make 1 big "||" concatenated check but that way 
//		I can add logs or some tracing later...

	//size checks
	if(params->ullMinSize != 0 && file->GetFileSize() < params->ullMinSize)
	{
		reason.Format(L"Result too small: %s < %s", CastItoXBytes(file->GetFileSize()), CastItoXBytes(params->ullMinSize));
		return true;
	}
    if(params->ullMaxSize != 0 && file->GetFileSize() > params->ullMaxSize)
	{
		reason.Format(L"Result too big: %s > %s", CastItoXBytes(file->GetFileSize()), CastItoXBytes(params->ullMaxSize));
		return true;
	}

	//filetype check
	const CString strFileType = file->GetFileType();
	if(!params->strFileType.IsEmpty())
	{
		if(_tcsicmp(strFileType, (CString)params->strFileType) != 0)
		{
			reason.Format(L"Wrong filetype: %s != %s", strFileType, params->strFileType);
			return true;
		}
	}

	const UINT bitrate = file->GetIntTagValue(FT_MEDIA_BITRATE);

	//there can't be audio/video files without bitrates... or can there!?
	//in any case, there may not be files other than audio/video that have a bitrate... those are fakes for sure!
// #define	ED2KFTSTR_AUDIO			"Audio"	// value for eD2K tag FT_FILETYPE
// #define	ED2KFTSTR_VIDEO			"Video"	// value for eD2K tag FT_FILETYPE
// #define	ED2KFTSTR_IMAGE			"Image"	// value for eD2K tag FT_FILETYPE
// #define	ED2KFTSTR_DOCUMENT		"Doc"	// value for eD2K tag FT_FILETYPE
// #define	ED2KFTSTR_PROGRAM		"Pro"	// value for eD2K tag FT_FILETYPE
// #define	ED2KFTSTR_ARCHIVE		"Arc"	// eMule internal use only
// #define	ED2KFTSTR_CDIMAGE		"Iso"	// eMule internal use only
// #define ED2KFTSTR_EMULECOLLECTION	"EmuleCollection" // Value for eD2K tag FT_FILETYPE
// #define	ED2KFTSTR_HASH			"Hash"
// #ifdef BITTORRENT
// #define ED2KFTSTR_TORRENT		"BTMeta"
// #endif
	bool bAudioVideo = false;
	if(strFileType == ED2KFTSTR_AUDIO)
	{
		bAudioVideo = true;
		if(bitrate == 0)
		{
			reason.Format(L"Audio file without bitrate!?");
			return true;
		}
	}
	if(strFileType == ED2KFTSTR_VIDEO)
	{
		bAudioVideo = true;
		//this *CAN* happen (.mpg files for example)
//		if(bitrate == 0)
//		{
//			reason.Format(L"Video file without bitrate!?");
//			return true;
//		}
	}
	//no audio/video!?
	if(!bAudioVideo && bitrate != 0)
	{
		reason.Format(L"Non audio/video file with bitrate!?");
		return true;
	}

	//extension check
//	if (_tcscmp(rstrFileType, _T(ED2KFTSTR_PROGRAM))==0)
//		CString strDetailFileType = GetFileTypeByName(GetFileName());
//	if(!params->strExtension.IsEmpty() && _tcsicmp(file->GetFileType(), params->strExpression) != 0)
//		return true;
	CString ext = GetExtension(file->GetFileName());
	ext = ext.Mid(1); // skip the starting '.'
	if(!params->strExtension.IsEmpty() && _tcsicmp(ext, params->strExtension) != 0)
	{
		reason.Format(L"Wrong extension: %s != %s", ext, params->strExtension);
		return true;
	}

	//availability
	if(params->uAvailability != 0 && file->GetSourceCount() < params->uAvailability)
	{
		reason.Format(L"Availability too low: %u < %u", file->GetSourceCount(), params->uAvailability);
		return true;
	}

	//complete sources
	if(params->uComplete != 0 && file->GetIntTagValue(FT_COMPLETE_SOURCES) < params->uComplete)
	{
		reason.Format(L"Complete source count too low: %u < %u", file->GetIntTagValue(FT_COMPLETE_SOURCES), params->uComplete);
		return true;
	}

	//bitrate
    if(params->ulMinBitrate != 0 && bitrate < params->ulMinBitrate)
	{
		reason.Format(L"Bitrate too low: %u < %u", bitrate, params->ulMinBitrate);
		return true;
	}

	//min length
	if(params->ulMinLength != 0 && file->GetIntTagValue(FT_MEDIA_LENGTH) < params->ulMinLength)
	{
		reason.Format(L"Length too low: %u < %u", file->GetIntTagValue(FT_MEDIA_LENGTH), params->ulMinLength);
		return true;
	}

	//codec
	if(!params->strCodec.IsEmpty() && _tcsicmp(file->GetStrTagValue(FT_MEDIA_CODEC), params->strCodec) != 0)
	{
		reason.Format(L"Codec mismatch: %s != %s", file->GetStrTagValue(FT_MEDIA_CODEC), params->strCodec);
		return true;
	}

	//title
	if(!params->strTitle.IsEmpty() && _tcsicmp(file->GetStrTagValue(FT_MEDIA_TITLE), params->strTitle) != 0)
	{
		reason.Format(L"Title mismatch: %s != %s", file->GetStrTagValue(FT_MEDIA_TITLE), params->strTitle);
		return true;
	}

	//artist
	if(!params->strArtist.IsEmpty() && _tcsicmp(file->GetStrTagValue(FT_MEDIA_ARTIST), params->strArtist) != 0)
	{
		reason.Format(L"Artist mismatch: %s != %s", file->GetStrTagValue(FT_MEDIA_ARTIST), params->strArtist);
		return true;
	}

	//album
	if(!params->strAlbum.IsEmpty() && _tcsicmp(file->GetStrTagValue(FT_MEDIA_ALBUM), params->strAlbum) != 0)
	{
		reason.Format(L"Album mismatch: %s != %s", file->GetStrTagValue(FT_MEDIA_ALBUM), params->strAlbum);
		return true;
	}

//TODO: more checks...
//		we *should* check and parse more complex search terms, e.g. "((NOT XYZ AND NOT ABC) OR CDE)"
	return false;
}
// Tux: Feature: additional fake checks [end]