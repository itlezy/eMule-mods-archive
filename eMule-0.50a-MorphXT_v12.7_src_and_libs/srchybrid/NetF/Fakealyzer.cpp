/**
 *
  */
#include "stdafx.h"
#include "Fakealyzer.h"

bool
CFakealyzer::IsFakeKadNode(const uchar* kad_id, uint32 /*ip*/, uint16 /*udp_port*/)
{
	// Check for MediaDefender fake contact ID's (This is quite useless, but is kept for reference)
	if (kad_id[5] == 14 && kad_id[14] == 111 && kad_id[6] < 240 && kad_id[7] == (kad_id[6] + 7) && kad_id[8] == (kad_id[6] + 13))
	{
		return true;
	}
	
	return false;
}

bool
CFakealyzer::IsFakeClient(const uchar* client_hash, uint32 /*ip*/, uint16 /*tcp_port*/)
{
	// Check for MediaDefender fake contact ID's (This is quite useless, but is kept for reference)
	if (client_hash[5] == 14 && client_hash[14] == 111 && client_hash[6] < 240 && client_hash[7] == (client_hash[6] + 7) && client_hash[8] == (client_hash[6] + 13))
	{
		return true;
	}
	
	return false;
}

int
CFakealyzer::CheckSearchResult(CSearchFile* content)
{
	uint32 nBitrate = content->GetIntTagValue(FT_MEDIA_BITRATE);
	uint32 nAvgBitrate = nBitrate;
	int	   nPositives = 0;
	int	   nNegatives = 0;

	if (content->GetIntTagValue(FT_MEDIA_LENGTH) > 0)
		nAvgBitrate = (uint32) (uint64) (content->GetFileSize() / 128ULL) / content->GetIntTagValue(FT_MEDIA_LENGTH);
	if (nBitrate || nAvgBitrate)
	{
		// Check for matches in file name against given tags (if given)
		bool isNameMismatch = false;
		CString strFileName = content->GetFileName();
		CString strArtist = content->GetStrTagValue(FT_MEDIA_ARTIST);
		CString strAlbum = content->GetStrTagValue(FT_MEDIA_ALBUM);
		CString strTitle = content->GetStrTagValue(FT_MEDIA_TITLE);
		if (!(strArtist.IsEmpty() && strAlbum.IsEmpty() && strTitle.IsEmpty()))
		{
			isNameMismatch = true;
			strFileName.MakeLower();
			if (!strArtist.IsEmpty() && strFileName.Find(strArtist.MakeLower()) != -1)
				isNameMismatch = false;
			if (!strAlbum.IsEmpty() && strFileName.Find(strAlbum.MakeLower()) != -1)
				isNameMismatch = false;
			if (!strTitle.IsEmpty() && strFileName.Find(strTitle.MakeLower()) != -1)
				isNameMismatch = false;
			if (!isNameMismatch)
				++nPositives;
		}

		if (content->GetFileType() == ED2KFTSTR_AUDIO)
		{
			bool isMP3 = false;
			if (content->GetFileName().Right(4).MakeLower() == _T(".mp3"))
				isMP3 = true; 
			if (!isNameMismatch && (nBitrate >= 128 && (nBitrate % 8 == 0) && isMP3) && (nBitrate < 1500) && (nBitrate <= 2 * nAvgBitrate)) // No serious files have lower rate than this
				++nPositives;
			else if ((nBitrate < 56 && isMP3) || (nBitrate > 4 * nAvgBitrate)) // Illegal!?
				++nNegatives;
		}
		else if (content->GetFileType() == ED2KFTSTR_VIDEO)
		{
			if (!isNameMismatch && (nAvgBitrate >= 900) && (nAvgBitrate < 30000) && (nBitrate <= 2 * nAvgBitrate)) // The good ones are usually within these bounds
				++nPositives;
			else if (nBitrate > 4 * nAvgBitrate) // Should not be possible!
				++nNegatives;
		}
		else if (content->GetFileType() != _T("")) // Only audio and video files have bitrate, so it must be fake!
		{
			++nNegatives;
		}
	}

	// common fake file identifier (not used very much anymore and will give a false positive on approx. 1 file out of 137)
	//if (((uint64) content->GetFileSize() % 137ULL) == 0)
	//{
	//	++nNegatives;
	//}

	// reasonable availability (KAD only!)
	if (content->IsKademlia())
	{
		uint32 nAvailability = content->GetSourceCount();
		uint32 nKnownPublisher = (content->GetKadPublishInfo() & 0x00FF0000) >> 16;
		if ((10 * nKnownPublisher < nAvailability) && (nKnownPublisher > 0) && (nKnownPublisher < 100))
			++nNegatives;
		else if ((2 * nKnownPublisher > nAvailability) || (nKnownPublisher >= 100))
			++nPositives;
	}

	// merge positives and negatives into a score 
	int nScore;
	if (nNegatives > 0)
	{
		nScore = (nPositives > 0 ? (nPositives - 1) : 0) - nNegatives;
	}
	else
	{
		nScore = nPositives;
	}

	if (nScore <= -2)
		return FAKE;
	else if (nScore <= -1)
		return SUSPECT;
	else if (nScore >= 2)
		return GOOD;
	else if (nScore >= 1)
		return OK;
	else
		return UNKNOWN;
}
