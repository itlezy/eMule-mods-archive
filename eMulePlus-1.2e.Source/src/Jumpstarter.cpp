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
//
#include "stdafx.h"
#include "types.h"
#include "Jumpstarter.h"
#include "KnownFile.h"
#include "updownclient.h"
#include "otherfunctions.h"
#include "BerkeleyDb/build_win32/db_cxx.h"

#define DATABASE_FILE_NAME "Jumpstart.db"

#define NUM_CHUNKS_TO_SHOW 2

#define JS_INVISIBLE 0
#define JS_VISIBLE 1
#define JS_COMPLETED 0xFF
#define JS_MAX_USERS 0xFE

// static members
DbEnv*	CJumpstarter::pDbEnv = NULL;
Db*		CJumpstarter::pDbJumpstart = NULL;
Db*		CJumpstarter::pDbUserBlocks = NULL;
Db*		CJumpstarter::pDbUserChunks = NULL;
Db*		CJumpstarter::pDbJSOptions = NULL;

struct StartAndSize
{
	uint32 m_lStart;
	uint32 m_lSize;
};

struct FileUser
{
	uchar m_fileHash[16];
	uchar m_userHash[16];
};

struct FileUserChunk
{
	uchar m_fileHash[16];
	uchar m_userHash[16];
	uint32 chunk_no;
};

struct JSOptions
{
	uint32 mode; // 0 - disabled, >0 enabled
	uint32 reserved[10];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// sorting function for StartAndSize records
static int compare_start_size(DB *pDb, const DBT *pDbt1, const DBT *pDbt2)
{
	StartAndSize	*ss1 = reinterpret_cast<StartAndSize*>(pDbt1->data);
	StartAndSize	*ss2 = reinterpret_cast<StartAndSize*>(pDbt2->data);
	NOPRM(pDb);

	if (ss1->m_lStart < ss2->m_lStart)
		return -1;

	if (ss1->m_lStart > ss2->m_lStart)
		return 1;

	// m_lStart is equal
	if (ss1->m_lSize < ss2->m_lSize)
		return -1;

	if (ss1->m_lSize > ss2->m_lSize)
		return 1;

	// equal
	return 0;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CJumpstarter::CJumpstarter(CKnownFile* file)
{
	m_KFile = file;
	m_dwParts = m_KFile->GetPartCount();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CJumpstarter::~CJumpstarter(void)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJumpstarter::OpenDatabases(DbEnv *pDbEnvironment)
{
	pDbEnv = pDbEnvironment;
	pDbJumpstart = new Db(pDbEnv, 0);
	pDbJumpstart->set_pagesize(16 * 1024); // 16Kb page m_lSize
	pDbJumpstart->open( NULL, DATABASE_FILE_NAME, "File-Chunks", DB_HASH,
	                    DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0 );

	pDbUserBlocks = new Db(pDbEnv, 0);
	pDbUserBlocks->set_flags(DB_DUPSORT);
	pDbUserBlocks->set_dup_compare(&compare_start_size);
	pDbUserBlocks->set_pagesize(16 * 1024); // 16Kb page size
	pDbUserBlocks->open( NULL, DATABASE_FILE_NAME, "FileUserChunk-Blocks64",
	                     DB_HASH, DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0 );

	pDbUserChunks = new Db(pDbEnv, 0);
	pDbUserChunks->set_pagesize(16 * 1024); // 16Kb page size
	pDbUserChunks->open( NULL, DATABASE_FILE_NAME, "FileUser-Chunk", DB_HASH,
	                     DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0 );

	pDbJSOptions = new Db(pDbEnv, 0);
	pDbJSOptions->set_pagesize(16 * 1024); // 16Kb page size
	pDbJSOptions->open( NULL, DATABASE_FILE_NAME, "File-JSOptions", DB_HASH,
	                    DB_CREATE | DB_THREAD | DB_AUTO_COMMIT, 0 );
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJumpstarter::CloseDatabases(void)
{
	if (pDbEnv == NULL)
		return;

	pDbUserChunks->close(0);
	safe_delete(pDbUserChunks);

	pDbUserBlocks->close(0);
	safe_delete(pDbUserBlocks);

	pDbJumpstart->close(0);
	safe_delete(pDbJumpstart);

	pDbJSOptions->close(0);
	safe_delete(pDbJSOptions);

	pDbEnv = NULL;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CJumpstarter::ShouldBeEnabledForFile(CKnownFile *file)
{
	return ReadJsEnabled(file);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CJumpstarter::IsJsCompleteForFile(CKnownFile* file)
{
	try
	{
		const uchar * m_fileHash = file->GetFileHash();
		Dbt key((void *) m_fileHash, 16);
		uint32 nChunks = file->GetPartCount();
		uchar* chunks_info = new uchar[nChunks];
		Dbt data(chunks_info, sizeof(uchar) * nChunks);
		data.set_ulen(sizeof(uchar) * nChunks);
		data.set_flags(DB_DBT_USERMEM);

		int res = pDbJumpstart->get(NULL, &key, &data, 0);

		bool all_complete = true;

		if (res != DB_NOTFOUND)
		{
			for (uint32 i = 0; i < nChunks; i++)
			{
				if (chunks_info[i] != JS_COMPLETED)
				{
					all_complete = false;
					break;
				}
			}
		}

		delete[] chunks_info;

		if (res == DB_NOTFOUND)
		{
			return false;
		}
		else
		{
			return all_complete;
		}
	}
	catch (DbException & dbe)
	{
		AfxMessageBox(CString(dbe.what()));
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CJumpstarter::ReadJsEnabled(CKnownFile* file)
{
	const uchar * m_fileHash = file->GetFileHash();
	Dbt key((void *) m_fileHash, 16);
	JSOptions js_options;
	Dbt js_options_data(&js_options, sizeof(js_options));
	js_options_data.set_ulen(sizeof(js_options));
	js_options_data.set_flags(DB_DBT_USERMEM);

	int res = pDbJSOptions->get(NULL, &key, &js_options_data, 0);

	if (res == DB_NOTFOUND)
		return false;

	return (js_options.mode > 0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJumpstarter::WriteJsEnabled(CKnownFile* file, bool enabled)
{
	// later we might m_lStart using transactions here
	const uchar * m_fileHash = file->GetFileHash();
	Dbt key((void *) m_fileHash, 16);
	JSOptions js_options;
	Dbt js_options_data(&js_options, sizeof(js_options));
	js_options_data.set_ulen(sizeof(js_options));
	js_options_data.set_flags(DB_DBT_USERMEM);

	int res = pDbJSOptions->get(NULL, &key, &js_options_data, 0);

	if (res == DB_NOTFOUND)
	{
		memzero(&js_options, sizeof(js_options));
	}

	js_options.mode = enabled ? 1 : 0;
	pDbJSOptions->put(NULL, &key, &js_options_data, DB_AUTO_COMMIT);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJumpstarter::Disable()
{
	WriteJsEnabled(m_KFile, false);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJumpstarter::EnableForFile(CKnownFile* file)
{
	if (ReadJsEnabled(file))
		return;

	if (IsJsCompleteForFile(file))
		return;

	const uchar* m_fileHash = file->GetFileHash();

	uint32 nChunks = file->GetPartCount();

	uchar* chunks_info = new uchar[nChunks];

	Dbt key((void *) m_fileHash, 16);

	Dbt data(chunks_info, sizeof(uchar) * nChunks);

	data.set_ulen(sizeof(uchar) * nChunks);

	data.set_flags(DB_DBT_USERMEM);

	// FIXME: Wrap this in a transaction
	int res = pDbJumpstart->get(NULL, &key, &data, 0);

	if (res == DB_NOTFOUND)
	{
		for (uint32 i = 0; i < nChunks; i++)
			chunks_info[i] = 0;

		pDbJumpstart->put(NULL, &key, &data, DB_AUTO_COMMIT);
	}

	WriteJsEnabled(file, true);

	delete[] chunks_info;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJumpstarter::AddSentBlock(CUpDownClient *client, const uint64 &qwStartOffset, uint32 togo)
{
	if (togo == 0)
		return;

#if 0	// we upload only within one chunk!
	if (start_offset / PARTSIZE != (start_offset + togo - 1) / PARTSIZE)
	{
		// split on part border and do two recursive calls to ourselves
		uint32 next_part_start = ((start_offset / PARTSIZE) + 1) * PARTSIZE;
		AddSentBlock(client, start_offset, next_part_start - start_offset);
		AddSentBlock(client, next_part_start, togo - (next_part_start - start_offset));
		return;
	}
#endif

	int dbresult;
	int dbresult2;

	// the block belongs to one part here
	uint32 partno = static_cast<uint32>(qwStartOffset / PARTSIZE);

	DbTxn *tid;
	pDbEnv->txn_begin(NULL, &tid, 0); // m_lStart the transaction

	// add the new record (database is sorted)
	FileUserChunk fuchunk;
	md4cpy(&fuchunk.m_fileHash, m_KFile->GetFileHash());
	md4cpy(&fuchunk.m_userHash, client->GetUserHash());
	fuchunk.chunk_no = partno;
	Dbt key_fuchunk(&fuchunk, sizeof(fuchunk));

	StartAndSize start_and_size;
	Dbt data_start_and_size(&start_and_size, sizeof(start_and_size));
	Dbc	*dbcp, *dbcp2;

	start_and_size.m_lStart = static_cast<uint32>(qwStartOffset - static_cast<uint64>(partno) * PARTSIZE);
	start_and_size.m_lSize = togo;

	pDbUserBlocks->put(tid, &key_fuchunk, &data_start_and_size, 0);

	// now, do a pass removing overlaps
	int blocks_count;
	bool need_to_merge;

	do
	{
		pDbUserBlocks->cursor(tid, &dbcp, 0);
		pDbUserBlocks->cursor(tid, &dbcp2, 0);

		data_start_and_size.set_flags(DB_DBT_USERMEM);
		data_start_and_size.set_ulen(sizeof(start_and_size));

		StartAndSize start_and_size2;
		Dbt data_start_and_size2(&start_and_size2, sizeof(start_and_size2));
		data_start_and_size2.set_flags(DB_DBT_USERMEM);
		data_start_and_size2.set_ulen(sizeof(start_and_size2));

		dbresult = dbcp->get(&key_fuchunk, &data_start_and_size, DB_SET);

		ASSERT(dbresult == 0);

		blocks_count = 1;

		dbresult2 = dbcp2->get(&key_fuchunk, &data_start_and_size2, DB_SET);

		ASSERT(dbresult2 == 0);

		dbresult2 = dbcp2->get(&key_fuchunk, &data_start_and_size2, DB_NEXT_DUP);

		need_to_merge = false;
		while (dbresult2 == 0)
		{
			// check for overlap
			if (start_and_size2.m_lStart <= start_and_size.m_lStart + start_and_size.m_lSize)
			{
				uint32 dwNewStart, dwNewSize;

				need_to_merge = true;
				dwNewStart = start_and_size.m_lStart;
				dwNewSize = max( start_and_size.m_lStart + start_and_size.m_lSize,
				                start_and_size2.m_lStart + start_and_size2.m_lSize ) - dwNewStart;

				dbresult = dbcp->del(0);
				dbresult2 = dbcp2->del(0);
				start_and_size.m_lStart = dwNewStart;
				start_and_size.m_lSize = dwNewSize;
				pDbUserBlocks->put(tid, &key_fuchunk, &data_start_and_size, 0);
				break;
			}
			dbresult = dbcp->get(&key_fuchunk, &data_start_and_size, DB_NEXT_DUP);

			ASSERT(dbresult == 0);

			blocks_count++;
			dbresult2 = dbcp2->get(&key_fuchunk, &data_start_and_size2, DB_NEXT_DUP);
		}
		dbcp->close();
		dbcp2->close();
	} while(need_to_merge || (dbresult2 != DB_NOTFOUND));

	if (blocks_count == 1)
	{
		uint32 partsize = m_KFile->GetPartSize(partno);

		if ((start_and_size.m_lStart == 0) && (start_and_size.m_lSize == partsize))
		{
			// Yes, this is full part
			CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart: User %s completed chunk %d of file %s"), client->GetUserName(), partno, m_KFile->GetFileName());

			FileUser fu;
			md4cpy(&fu.m_fileHash, m_KFile->GetFileHash());
			md4cpy(&fu.m_userHash, client->GetUserHash());
			Dbt key_fu(&fu, sizeof(fu));

			uchar* chunks_info = new uchar[m_dwParts];
			Dbt data_chunks_info(chunks_info, sizeof(uchar) * m_dwParts);
			data_chunks_info.set_ulen(sizeof(uchar) * m_dwParts);
			data_chunks_info.set_flags(DB_DBT_USERMEM);

			int res = pDbUserChunks->get(tid, &key_fu, &data_chunks_info, 0);

			if (res == DB_NOTFOUND)
			{
				// the chunk data for this user does not exist,
				// probably he got on queue before we've enabled
				// the Jumpstart on this file. easiest way is
				// to ignore him :)
				CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart: User %s (file %s) has no chunks info"), client->GetUserName(), m_KFile->GetFileName());
			}
			else
			{
				chunks_info[partno] = JS_COMPLETED;
				pDbUserChunks->put(tid, &key_fu, &data_chunks_info, 0);
			}

			Dbt key_filehash(&fu.m_fileHash, sizeof(fu.m_fileHash));

			res = pDbJumpstart->get(tid, &key_filehash, &data_chunks_info, 0);

			if (res == DB_NOTFOUND)
			{
				// the chunk data for this file does not exist
				// this is a serious error
				CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart: File %s has no chunks info, disabled !"), m_KFile->GetFileName());
				m_KFile->SetJumpstartEnabled(false);
			}
			else
			{
				chunks_info[partno] = JS_COMPLETED;
				pDbJumpstart->put(tid, &key_filehash, &data_chunks_info, 0);
			//	Check if the file is complete
				bool has_more_chunks = false;

				for (uint32 i = 0; i < m_dwParts; i++)
				{
					if (chunks_info[i] != JS_COMPLETED)
					{
						has_more_chunks = true;
						break;
					}
				}

				if (!has_more_chunks)
				{
					// hooray! the file's done
					CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart: File %s has all chunks completed, switching to normal!"), m_KFile->GetFileName());
					m_KFile->SetJumpstartEnabled(false);
				}
			}

			delete[] chunks_info;
		} // end if got full part
	} // end if(blocks_count==1) - one block in this part after merging

	tid->commit(0);
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CJumpstarter::WriteJumpstartPartStatus(CUpDownClient* client, CMemFile* file)
{
	DbTxn *tid;
	pDbEnv->txn_begin(NULL, &tid, 0);

	// load chunk data for this user
	FileUser fu;
	md4cpy(&fu.m_fileHash, m_KFile->GetFileHash());
	md4cpy(&fu.m_userHash, client->GetUserHash());
	Dbt key_fu(&fu, sizeof(fu));

	uchar* chunks_info = new uchar[m_dwParts];
	Dbt data_chunks_info(chunks_info, sizeof(uchar) * m_dwParts);
	data_chunks_info.set_ulen(sizeof(uchar) * m_dwParts);
	data_chunks_info.set_flags(DB_DBT_USERMEM);

	int res = pDbUserChunks->get(tid, &key_fu, &data_chunks_info, 0);

	if (res == DB_NOTFOUND)
	{
		// the chunk data for this user does not exist
		//CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart: New user %s for file %s"),client->GetUserName(),m_KFile->GetFileName());
		for (uint32 i = 0; i < m_dwParts; i++)
			chunks_info[i] = JS_INVISIBLE;
	}

	// load file chunks
	Dbt key_filehash(&fu.m_fileHash, sizeof(fu.m_fileHash));

	uchar* file_chunks_info = new uchar[m_dwParts];

	Dbt data_file_chunks_info(file_chunks_info, sizeof(uchar) * m_dwParts);

	data_file_chunks_info.set_ulen(sizeof(uchar) * m_dwParts);
	data_file_chunks_info.set_flags(DB_DBT_USERMEM);

	res = pDbJumpstart->get(tid, &key_filehash, &data_file_chunks_info, 0);

	if (res == DB_NOTFOUND)
	{
		// the chunk data for this file does not exist
		// this is a serious error
		CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart: File %s has no chunks info in WriteJumpstartPartStatus, disabled!"), m_KFile->GetFileName());
		m_KFile->SetJumpstartEnabled(false);
	}

	// find chunks to show
	uint32	dwVisibleCnt = NumVisibleInChunksMap(chunks_info);

	while (dwVisibleCnt < NUM_CHUNKS_TO_SHOW)
	{
		uint32 best_part = 0xFFFF;
		uint32 dwBestPartValue = JS_COMPLETED;

		for (uint32 i = 0; i < m_dwParts; i++)
		{
			if ((chunks_info[i] == JS_INVISIBLE) && (file_chunks_info[i] != JS_COMPLETED))
			{
			//	Select a chunk with less downloaders
				if (file_chunks_info[i] < dwBestPartValue)
				{
					// new one is better
					best_part = i;
					dwBestPartValue = file_chunks_info[i];
				}
			}
		} // end for(uint32 i=0; i<m_dwParts ...

		if (best_part == 0xFFFF)
		{
		//	Nothing more to add => report the parts which are already visible
			break;
		}
		else
		{
			CLoggable::AddLogLine( LOG_FL_DBG, _T("Jumpstart: Exposing chunk %d of file %s to user %s"),
			                            best_part, m_KFile->GetFileName(), client->GetUserName() );
			chunks_info[best_part] = JS_VISIBLE;

			if (file_chunks_info[best_part] < JS_MAX_USERS)
				file_chunks_info[best_part]++;
			dwVisibleCnt++;
		}
	} // end while

	// save new data for user and file
	pDbUserChunks->put(tid, &key_fu, &data_chunks_info, 0);
	pDbJumpstart->put(tid, &key_filehash, &data_file_chunks_info, 0);

	tid->commit(0);

	// now actualy write the part status
	uint32	dwDone = 0, dwParts = m_KFile->GetED2KPartCount();

	file->Write(&dwParts, 2);
	while (dwDone < dwParts)
	{
		byte towrite = 0;

		for (uint32 i = 0; i < 8; i++)
		{
		//	Report that we have "void" part for files with size % PARTSIZE = 0
			if (((dwDone == (dwParts - 1)) && (dwParts != m_dwParts)) || (chunks_info[dwDone] != JS_INVISIBLE))
				towrite |= (1 << i);

			if (++dwDone >= dwParts)
				break;
		}

		file->Write(&towrite, 1);
	}

	delete[] chunks_info;
	delete[] file_chunks_info;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint32 CJumpstarter::NumVisibleInChunksMap(uchar* chunks_map)
{
	uint32 count_visible = 0;

	for (uint32 i = 0; i < m_dwParts; i++)
	{
		if (chunks_map[i] == JS_VISIBLE)
			count_visible++;
	}

	return count_visible;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CJumpstarter::IsChunkComplete(uint32 partNo)
{
	uchar m_fileHash[16];
	md4cpy(&m_fileHash, m_KFile->GetFileHash());
	Dbt key(m_fileHash, 16);
	uint32 nChunks = m_KFile->GetPartCount();
	uchar* chunks_info = new uchar[nChunks];
	Dbt data(chunks_info, sizeof(uchar) * nChunks);
	data.set_ulen(sizeof(uchar) * nChunks);
	data.set_flags(DB_DBT_USERMEM);

	int res = pDbJumpstart->get(NULL, &key, &data, 0);
	bool	bResult = false;

	if (res != DB_NOTFOUND)
		bResult = (chunks_info[partNo] == JS_COMPLETED);

	delete[] chunks_info;
	return bResult;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool CJumpstarter::AllowChunkForClient(uint32 partNo, CUpDownClient* client)
{
	// load chunk data for this user
	//CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart DEBUG: AllowChunkForClient: user: %s chunk: %d"), client->GetUserName(), partNo);
	FileUser fu;
	md4cpy(&fu.m_fileHash, m_KFile->GetFileHash());
	md4cpy(&fu.m_userHash, client->GetUserHash());
	Dbt key_fu(&fu, sizeof(fu));

	uchar* chunks_info = new uchar[m_dwParts];
	Dbt data_chunks_info(chunks_info, sizeof(uchar) * m_dwParts);
	data_chunks_info.set_ulen(sizeof(uchar) * m_dwParts);
	data_chunks_info.set_flags(DB_DBT_USERMEM);

	int res = pDbUserChunks->get(NULL, &key_fu, &data_chunks_info, 0);

	uchar this_chunk_info = chunks_info[partNo];

	delete[] chunks_info;

	if (res == DB_NOTFOUND)
	{
		// the chunk data for this user does not exist
		CLoggable::AddLogLine(LOG_FL_DBG | LOG_RGB_WARNING, _T("Jumpstart WARNING: User %s asked for file %s, no chunk data, denied"), client->GetClientNameWithSoftware(), m_KFile->GetFileName());
		return false;
	}

	if (this_chunk_info == JS_COMPLETED)
	{
#ifdef _DEBUG
		CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart DEBUG: User %s asked for file %s, completed by this user, allowed"), client->GetClientNameWithSoftware(), m_KFile->GetFileName());
#endif
		return true;
	}

	if (this_chunk_info == JS_INVISIBLE)
	{
#ifdef _DEBUG
		CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart DEBUG: User %s asked for file %s, chunk invisible for user, denied"), client->GetClientNameWithSoftware(), m_KFile->GetFileName());
#endif
		return false;
	}

	// here we are when the chunk is visible for this user
	if (IsChunkComplete(partNo))
	{
#ifdef _DEBUG
		CLoggable::AddLogLine(LOG_FL_DBG, _T("Jumpstart DEBUG: User %s asked for file %s, chunk completed, denied"), client->GetClientNameWithSoftware(), m_KFile->GetFileName());
#endif
		return false;
	}

	return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
