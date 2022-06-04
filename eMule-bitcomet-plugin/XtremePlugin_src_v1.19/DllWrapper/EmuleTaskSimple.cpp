#include "StdAfx.h"
#include "EmuleTaskSimple.h"
#include "EmuleTaskSimpleRange.h"
#include "EmuleTaskCache.h"

#include "srchybrid/PartFile.h"
#include "srchybrid/DownloadQueue.h"
#include "srchybrid/emule.h"
#include "srchybrid/updownclient.h"
#include "srchybrid/emuleDlg.h"
#include "srchybrid/SharedFileList.h"

#include <algorithm>
#include <sstream>
using namespace std;

#include "DllWrapper/common.h"
extern host_callback_t g_host_callback_func;


bool operator == (const request_block_t* block1, const request_block_t &block2)
{ 
	return block2 == *block1; 
}

bool operator == (const CEmuleTaskSimpleRange* range1, const CEmuleTaskSimpleRange &range2)
{ 
	return range2 == *range1; 
}

CEmuleTaskSimple::CEmuleTaskSimple(const uchar hash[16], uint32 client_id)
	: m_file_exist_already(false)
{
	ASSERT(hash != NULL);

	m_client_id			= client_id;
	memcpy(m_hash, hash, 16);
}


CEmuleTaskSimple::~CEmuleTaskSimple()
{
	close();
}

const uchar* CEmuleTaskSimple::get_hash() const
{
	return m_hash;
}

uint32 CEmuleTaskSimple::get_client_id() const
{
	return m_client_id;
}

void CEmuleTaskSimple::set_client_id(uint32 id)
{
	m_client_id = id;
}

void CEmuleTaskSimple::close()
{
	CPartFile* part_file_ptr = NULL;
	part_file_ptr = theAppPtr->downloadqueue->GetFileByID(m_hash);

	if ( part_file_ptr != NULL && part_file_ptr->CanStopFile() )
		part_file_ptr->StopFile();

	typedef std::list<CEmuleTaskSimpleRange*>::iterator	range_iterator_t;

	for (range_iterator_t iter = m_uncompleted_range_list.begin(); iter != m_uncompleted_range_list.end(); ++iter ) 
	{
		delete *iter;
	}

	for (range_iterator_t iter = m_completed_range_list.begin(); iter != m_completed_range_list.end(); ++iter ) 
	{
		delete *iter;
	}

	for (list<request_block_t*>::iterator iter = m_block_list.begin(); iter != m_block_list.end(); ++iter ) 
	{
		delete *iter;
	}

	m_uncompleted_range_list.clear();
	m_completed_range_list.clear();
	m_block_list.clear();
}

/*
 * 	Purpose: 		add a range to a CEmuleTaskSimple object  
 *
 * 	Precondition:	start <= end, and range [start, end] does 
 * 	not overlap within other ranges in m_uncompleted_range_list
 *
 * 	Return Value:	true when all preconditions are meet, false
 * 	otherwise
 */
bool CEmuleTaskSimple::add_range(uint64 start, uint64 end, list<request_block_t*>& requested_block_list)
{
	ASSERT( start <= end );
	if ( start > end )
		return false;

	requested_block_list.clear();

	CEmuleTaskSimpleRange* new_range = new CEmuleTaskSimpleRange(start, end);
	m_uncompleted_range_list.push_back(new_range);

	list<request_block_t*>	block_list;
	request_block_t::range_to_blocks(start, end, block_list);

	for(list<request_block_t*>::iterator iter = block_list.begin(); iter != block_list.end(); iter++)
	{
		request_block_t* new_block = *iter;

		list<request_block_t*>::iterator find = std::find(m_block_list.begin(), m_block_list.end(), *new_block);
		if(find != m_block_list.end())
		{
			delete new_block;
			new_block = *find;
		}
		else
		{
			m_block_list.push_back(new_block);
		}
		request_block_t::bind_block_to_range(new_range, new_block);
		requested_block_list.push_back(new_block);
	}

	return true;
}

/*
 * 	Purpose: 		delete a range in a CEmuleTaskSimple object  
 *
 * 	Precondition:	start <= end, and range [start, end] is actually
 * 	in m_uncompleted_range_list 
 *
 * 	Return Value:	true when all preconditions are meet, false
 * 	otherwise
 */
bool CEmuleTaskSimple::delete_completed_range(CEmuleTaskSimpleRange* range)
{
	ASSERT( range != NULL);
	if ( range == NULL ) 
		return false;

	list<CEmuleTaskSimpleRange*>::iterator find = std::find(m_completed_range_list.begin(), m_completed_range_list.end(), range);
	ASSERT(find != m_completed_range_list.end());
	if(find == m_completed_range_list.end())
	{
		return false;
	}
	
	range->unbind_blocks();
	delete (range);
	m_completed_range_list.erase(find);

	// 删除没有和任何 range 关联的 block
	for(list<request_block_t*>::iterator iter = m_block_list.begin(); iter != m_block_list.end(); )
	{
		request_block_t* block = *iter;
		if(block != NULL)
		{
			if(block->is_range_list_empty())
			{
				delete block;
				iter = m_block_list.erase(iter);
				continue;
			}
		}		
		iter++;
	}
	return true;
}

/*
 * 	Purpose: 		find a range that contains [start, end] 
 *
 * 	Precondition:	start <= end 
 *
 * 	Return Value:	when all preconditions are meet, and
 *	[start, end] falls in a range in m_uncompleted_range_list, it returns
 *	the pointer to the CEmuleTaskSimpleRange object [start, end]
 *	falls in.  Else	it returns NULL.
 */
request_block_t* CEmuleTaskSimple::find_block(uint64 start, uint64 end)
{
	request_block_t new_block(start, end);

	list<request_block_t*>::iterator find = std::find(m_block_list.begin(), m_block_list.end(), new_block);
	if(find != m_block_list.end())
	{
		return *find;
	}
	else
	{
		return NULL;
	}
}

CEmuleTaskSimpleRange* CEmuleTaskSimple::get_first_completed_range()
{
	CEmuleTaskSimpleRange* p = m_completed_range_list.empty() ? NULL : m_completed_range_list.front();

	return p;
}

CEmuleTaskSimpleRange* CEmuleTaskSimple::find_finished_range(uint64 start, uint64 end)
{
	CEmuleTaskSimpleRange range(start, end);

	list<CEmuleTaskSimpleRange*>::iterator find = std::find(m_completed_range_list.begin(), m_completed_range_list.end(), range);
	ASSERT(find != m_completed_range_list.end());
	if(find != m_completed_range_list.end())
	{
		CEmuleTaskSimpleRange* finished_range = *find;
		return finished_range;
	}

	return NULL;
}

void CEmuleTaskSimple::get_unfinished_block(list<request_block_t*>& block_list, bool need_unrequested) const
{
	block_list.clear();

	for(list<request_block_t*>::const_iterator iter = m_block_list.begin(); iter != m_block_list.end(); iter++)
	{
		request_block_t* block = *iter;
		if(block && !block->finished)
		{
			if(need_unrequested && !block->is_requested() )
			{
				block_list.push_back(block);
			}
			else
			{
				block_list.push_back(block);
			}
		}
	}

	return;
}

void CEmuleTaskSimple::get_requested_block(std::vector<request_block_t*>& block_list, const CUpDownClient* sender) const
{
	block_list.clear();

	for(list<request_block_t*>::const_iterator iter = m_block_list.begin(); iter != m_block_list.end(); iter++)
	{
		request_block_t* block = *iter;
		if(block)
		{
			if(block->has_send_request_to(sender) )
			{
				block_list.push_back(block);
			}
		}
	}

	return;
}


void CEmuleTaskSimple::on_block_request_cancel(uint64 start, uint64 end, const CUpDownClient* client)
{

	request_block_t* block = find_block(start, end);
	if(block == NULL)
		return;

	block->remove_download_client(client);
}

void CEmuleTaskSimple::on_block_request_received(const BYTE *data, uint64 start, uint64 end, uint64 block_start, uint64 block_end, const CUpDownClient* client)
{

	request_block_t* block = find_block(block_start, block_end);
	if(block == NULL)
		return;

	block->on_received_data(data, start, end, client);

	if(block->finished)
	{
		vector<CEmuleTaskSimpleRange*> range_list;
		block->get_finished_range_list(range_list);

		for(vector<CEmuleTaskSimpleRange*>::iterator iter = range_list.begin(); iter != range_list.end(); iter++)
		{
			CEmuleTaskSimpleRange* range = *iter;
			list<CEmuleTaskSimpleRange*>::iterator find = std::find(m_uncompleted_range_list.begin(), m_uncompleted_range_list.end(), range);
			{
				if(find != m_uncompleted_range_list.end())
				{
					m_uncompleted_range_list.erase(find);
					m_completed_range_list.push_back(range);
				}
			}
		}
	}
}

bool CEmuleTaskSimple::has_finished_range()
{
	return !m_completed_range_list.empty();
}

void CEmuleTaskSimple::update_pieces_status_from_host()
{
	// 在已下载数据导入host之前不能更新host已下载piece状态，否则会将host已下载而plugin未下载的错误数据传给host
	if(m_file_exist_already)
		return;

	CPartFile* part_file = NULL;
	part_file = theAppPtr->downloadqueue->GetFileByID(m_hash);

	if ( part_file == NULL )
	{
		return;
	}

	uint32 part_count = part_file->GetPartCount();
	uint64 nFileSize  = part_file->GetFileSize();

	bool has_any_completed_part = false;
	if ( g_host_callback_func.HOST_GetHostPieceStatus )
	{
		const char* pFinishedPieces = NULL;
		uint32 piece_num = 0;
		uint32 piece_len = 0;
		uint32 first_piece_len = 0;

		if ( g_host_callback_func.HOST_GetHostPieceStatus(m_client_id, pFinishedPieces, piece_num, piece_len, first_piece_len) )
		{
			string finished_pieces;
			finished_pieces.assign(pFinishedPieces, piece_num);

			ASSERT( piece_len > 0 && first_piece_len > 0 );
			if( piece_len == 0 || first_piece_len == 0)
				return;

			uint64 start = 0;
			uint64 end = first_piece_len - 1;

			for(size_t i = 0; i < finished_pieces.size(); i++)
			{
				// update gaplist status
				end = min(end, (uint64)nFileSize - 1);
				if(finished_pieces[i] == '1')
				{
					part_file->FillGap(start, end);
				}
				else
				{
					// 不调用AddGap()，否则会将只下载了一部分的part标记为未下载
					//part_file->AddGap(start, end);
				}

				start = end + 1;
				end = start + piece_len - 1;
			}
		}
	}

	for (UINT uPart = 0; uPart < part_count; uPart++)
	{
		if (part_file->IsComplete((uint64)uPart*PARTSIZE, (uint64)(uPart + 1)*PARTSIZE - 1, true))
		{
			has_any_completed_part = true;
			break;
		}
	}

	if(has_any_completed_part)
	{
		if (part_file->GetStatus() == PS_EMPTY)
		{
			if (theAppPtr->emuledlg->IsRunning()) // may be called during shutdown!
			{
				if (part_file->GetHashCount() == part_file->GetED2KPartCount() && !part_file->hashsetneeded)
				{
					// Successfully completed part, make it available for sharing
					part_file->SetStatus(PS_READY);
					theAppPtr->sharedfiles->SafeAddKFile(part_file);
				}
			}
		}
	}

}

void CEmuleTaskSimple::prepare_export_exist_file_to_host()
{
	if(!m_file_exist_already)
		return;

	const char* pFinishedPieces = NULL;
	uint32 piece_num = 0;
	uint32 piece_len = 0;
	uint32 first_piece_len = 0;

	if ( g_host_callback_func.HOST_GetHostPieceStatus(m_client_id, pFinishedPieces, piece_num, piece_len, first_piece_len) )
	{
		CPartFile* part_file_ptr = theAppPtr->downloadqueue->GetFileByID(m_hash);
		ASSERT(part_file_ptr);
		if(!part_file_ptr)
			return;
		m_export_progress.file_size = part_file_ptr->GetFileSize();

		ASSERT(piece_num > 0);
		if(piece_num == 0)
			return;

		string finished_pieces;
		finished_pieces.assign(pFinishedPieces, piece_num);

		ASSERT( piece_len > 0 && first_piece_len > 0 && piece_len >= first_piece_len);
		if( piece_len == 0 || first_piece_len == 0 || piece_len < first_piece_len)
			return;

		m_export_progress.host_needed_pieces.resize(finished_pieces.size(), false);
		for(size_t i = 0; i < finished_pieces.size(); i++)
		{
			// 忽略头部不完整的piece
			if(i == 0 && first_piece_len != piece_len)
				continue;

			uint64 start = (i == 0 ? 0 : first_piece_len + (i-1) * piece_len );

			// 忽略尾部不完整的piece
			if( i == finished_pieces.size()-1 && start + piece_len >= m_export_progress.file_size)
				continue;

			if(finished_pieces[i] != '1')
			{
				if(part_file_ptr->IsComplete(start, start + piece_len - 1, true))
					m_export_progress.host_needed_pieces[i] = true;
			}
		}
		m_export_progress.piece_len = piece_len;
		m_export_progress.first_piece_len = first_piece_len;
		m_export_progress.export_pos = 0;
	}
}

bool CEmuleTaskSimple::get_export_block_range(uint64& offset, uint32& len)
{
	offset = 0;
	len = 0;

	if(!m_file_exist_already)
		return false;

	ASSERT(!m_export_progress.host_needed_pieces.empty());
	if(m_export_progress.host_needed_pieces.empty())
	{
		return false;
	}

	ASSERT(m_export_progress.piece_len != 0 && m_export_progress.first_piece_len != 0 && m_export_progress.piece_len >= m_export_progress.first_piece_len);
	if(m_export_progress.piece_len == 0 || m_export_progress.first_piece_len == 0 || m_export_progress.piece_len < m_export_progress.first_piece_len)
	{
		return false;
	}

	if(m_export_progress.export_pos >= m_export_progress.host_needed_pieces.size())
	{
		// 全部导出完毕
		m_file_exist_already = false;
		return false;
	}

	while(m_export_progress.export_pos < m_export_progress.host_needed_pieces.size())
	{
		if(m_export_progress.host_needed_pieces[m_export_progress.export_pos])
		{
			// 不应出现在头部的不完整的piece
			if( m_export_progress.export_pos == 0 )
			{
				ASSERT(m_export_progress.first_piece_len == m_export_progress.piece_len);
			}

			// 找到一个可以导出的区块
			offset = m_export_progress.export_pos == 0 ? 0
					: m_export_progress.first_piece_len + (m_export_progress.export_pos-1) * m_export_progress.piece_len;
			len = m_export_progress.piece_len;

			// 不应出现在尾部的不完整的piece
			if( m_export_progress.export_pos == m_export_progress.host_needed_pieces.size()-1 )
			{
				ASSERT(offset + len < m_export_progress.file_size);
			}

		}
		m_export_progress.export_pos ++;

		if(len != 0)
			return true;
	}

	if(m_export_progress.export_pos >= m_export_progress.host_needed_pieces.size())
		m_file_exist_already = false;

	return false;
}

std::string CEmuleTaskSimple::dump_uncompleted_range()
{
	// "[0,5],[10,20]"

	std::stringstream ss;

	typedef std::list<CEmuleTaskSimpleRange*>::iterator iterator_t;
	for (iterator_t iter = m_uncompleted_range_list.begin(); iter != m_uncompleted_range_list.end(); ++iter)
	{
		if(!ss.str().empty())
			ss << ",";

		uint64 start = 0;
		uint64 end = 0;
		(*iter)->get_range(start, end);
		uint32 length = (*iter)->length();
		ss << "[" << start/1024/1024 << "." << start/1024 << "M+" << length/1024 << "k]";
	}

	std::string output = ss.str();
	return output;
}

std::string CEmuleTaskSimple::dump_completed_range()
{
	// "[0,5],[10,20]"

	std::stringstream ss;

	typedef std::list<CEmuleTaskSimpleRange*>::iterator iterator_t;
	for (iterator_t iter = m_completed_range_list.begin(); iter != m_completed_range_list.end(); ++iter)
	{
		if(!ss.str().empty())
			ss << ",";

		uint64 start = 0;
		uint64 end = 0;
		(*iter)->get_range(start, end);
		uint32 length = (*iter)->length();
		ss << "[" << start/1024/1024 << "." << start/1024 << "M+" << length/1024 << "k]";
	}

	std::string output = ss.str();
	return output;
}

std::string CEmuleTaskSimple::dump_block_list()
{
	// "[0,5],[10,20]"

	std::stringstream ss;

	typedef std::list<request_block_t*>::iterator iterator_t;
	for (iterator_t iter = m_block_list.begin(); iter != m_block_list.end(); ++iter)
	{
		if(!ss.str().empty())
			ss << ",";

		uint64 start = (*iter)->StartOffset;
		//uint64 end = (*iter)->EndOffset;
		uint32 length = (*iter)->length();
		ss << "[" << start/1024/1024 << "." << start/1024 << "M+" << length/1024 << "k]";
	}

	std::string output = ss.str();
	return output;
}
