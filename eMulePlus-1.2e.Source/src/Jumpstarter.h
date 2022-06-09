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
// Written by obaldin
#pragma once

class CUpDownClient;
class CKnownFile;
class DbEnv;
class Db;

class CJumpstarter
{
public:
	CJumpstarter(CKnownFile *file);
	~CJumpstarter(void);

	static void OpenDatabases(DbEnv *pDbEnvironment);
	static void CloseDatabases(void);

	static bool ShouldBeEnabledForFile(CKnownFile *file);
	static bool IsJsCompleteForFile(CKnownFile *file);
	static void EnableForFile(CKnownFile *file);
	void Disable();

	void AddSentBlock(CUpDownClient *client, const uint64 &qwStartOffset, uint32 togo);
	void WriteJumpstartPartStatus(CUpDownClient *client, CMemFile *file);
	bool AllowChunkForClient(uint32 partNo, CUpDownClient *client);

private:
	bool IsChunkComplete(uint32 partNo);
	uint32 NumVisibleInChunksMap(uchar *chunks_map);
	static bool ReadJsEnabled(CKnownFile *file);
	static void WriteJsEnabled(CKnownFile *file, bool enabled);

private:
	CKnownFile		*m_KFile;
	uint32			m_dwParts;

	static DbEnv	*pDbEnv;
	static Db		*pDbJumpstart;
	static Db		*pDbUserBlocks;
	static Db		*pDbUserChunks;
	static Db		*pDbJSOptions;
};
