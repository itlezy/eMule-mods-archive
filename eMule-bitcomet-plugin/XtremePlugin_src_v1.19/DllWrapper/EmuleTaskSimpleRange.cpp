#include "StdAfx.h"

#include "EmuleTaskSimpleRange.h"
#include "EmuleTaskCache.h"
#include "srchybrid/opcodes.h"
#include "srchybrid/updownclient.h"
#include <sstream>
#include <algorithm>
#include <vector>
#include <map>
using namespace std;

typedef uint64 uint64_t;
#include "blocklist.h"

request_block_t::~request_block_t()
{
	if(m_cache)
	{
		delete m_cache;
		m_cache = NULL;
	}
}

void request_block_t::add_range(CEmuleTaskSimpleRange* range)
{


	vector<CEmuleTaskSimpleRange*>::iterator find = std::find(m_range_list.begin(), m_range_list.end(), range);
	ASSERT(find == m_range_list.end());
	if(find == m_range_list.end())
	{
		m_range_list.push_back(range);
	}
}

void request_block_t::delete_range(CEmuleTaskSimpleRange* range)
{


	vector<CEmuleTaskSimpleRange*>::iterator find = std::find(m_range_list.begin(), m_range_list.end(), range);
	ASSERT(find != m_range_list.end());
	if(find != m_range_list.end())
	{
		m_range_list.erase(find);
	}
}

void request_block_t::add_download_client(CUpDownClient* sender)
{
	m_UpDownClient_list.push_back(sender);
}

void request_block_t::remove_download_client(const CUpDownClient* sender)
{


	vector<CUpDownClient*>::iterator find = std::find(m_UpDownClient_list.begin(), m_UpDownClient_list.end(), sender);
	if(find != m_UpDownClient_list.end())
	{
		m_UpDownClient_list.erase(find);
	}
}

void request_block_t::on_received_data(const BYTE *data, uint64 start, uint64 end, const CUpDownClient* sender)
{


	if(data == NULL)
		return;

	if(m_cache == NULL)
	{
		try
		{
			m_cache = new CEmuleTaskCache(StartOffset, EndOffset);
		}
		catch(...)
		{
			m_cache = NULL;
			return;
		}
	}
	m_cache->insert(start, end+1, data);

	bool block_finish = m_cache->is_complete(StartOffset, EndOffset+1);

	if(block_finish)
	{
		remove_download_client(sender);
		finished = true;
	}
}

void request_block_t::get_finished_range_list(std::vector<CEmuleTaskSimpleRange*>& range_list)
{


	for(vector<CEmuleTaskSimpleRange*>::iterator iter = m_range_list.begin(); iter != m_range_list.end(); iter++)
	{
		CEmuleTaskSimpleRange* range = *iter;
		if(range == NULL)
			continue;

		bool finished = true;
		for(vector<request_block_t*>::iterator iter1 = range->m_block_list.begin(); iter1 != range->m_block_list.end(); iter1++)
		{
			request_block_t* block = *iter1;
			if(block == NULL)
				continue;
			if(!block->finished)
			{
				finished = false;
				break;
			}
		}

		if(finished)
		{
			range_list.push_back(range);
		}

	}
}

const uchar* request_block_t::get_buffer()
{
	if(m_cache == NULL)
		return NULL;
	
	return m_cache->get_buffer();
}

void request_block_t::release_cache_auto()
{


	bool all_read = true;
	for(vector<CEmuleTaskSimpleRange*>::iterator iter = m_range_list.begin(); iter != m_range_list.end(); iter++)
	{
		CEmuleTaskSimpleRange* range = *iter;
		if(range == NULL)
			continue;

		if(!range->m_read)
		{
			all_read = false;
			break;
		}
	}

	if(all_read)
	{
		if(m_cache)
		{
			delete m_cache;
			m_cache = NULL;
		}
	}
}

void request_block_t::get_suitable_parts(const std::list<request_block_t*>& block_list, const string& available_parts, 
				const vector<request_block_t*>& requested_blocks, int& suitable_part, std::vector<request_block_t*>& suitable_blocks )
{
	suitable_part = -1;
	suitable_blocks.clear();

	map<int, vector<request_block_t*>> blocks_in_part;
	for(list<request_block_t*>::const_iterator iter = block_list.begin(); iter != block_list.end(); iter++)
	{
		request_block_t* block = *iter;
		if(block && (uint32)block->PartNum < available_parts.size() && available_parts[block->PartNum] == '1')
		{
			vector<request_block_t*>::const_iterator find = std::find(requested_blocks.begin(), requested_blocks.end(), block);
			if(find != requested_blocks.end())
			{
				continue;
			}

			map<int, vector<request_block_t*>>::iterator find_part = blocks_in_part.find(block->PartNum);
			if(find_part == blocks_in_part.end())
			{
				vector<request_block_t*> block_list;
				block_list.push_back(block);
				blocks_in_part[block->PartNum] = block_list;
			}
			else
			{
				vector<request_block_t*>& block_list = find_part->second;
				block_list.push_back(block);
			}
		}
	}

	suitable_part = -1;
	size_t max_blocks_in_part = 0;
	for(map<int, vector<request_block_t*>>::const_iterator iter = blocks_in_part.begin(); iter != blocks_in_part.end(); iter++)
	{
		if(iter->second.size() > max_blocks_in_part)
		{
			suitable_part = iter->first;
		}
	}

	if(suitable_part != -1)
	{
		suitable_blocks = blocks_in_part[suitable_part];
	}
	else
	{
		suitable_blocks.clear();
	}
}
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

CEmuleTaskSimpleRange::CEmuleTaskSimpleRange(uint64 start, uint64 end)
	: m_start(start)
	, m_end(end)
	, m_read(false)
{
	ASSERT( start <= end );
}

CEmuleTaskSimpleRange::~CEmuleTaskSimpleRange(void)
{
}


bool CEmuleTaskSimpleRange::read_data(char* pBuffer)
{
	ASSERT(pBuffer);
	if(pBuffer == NULL)
		return false;

	blocklist block_list;

	bool error = false;
	for(vector<request_block_t*>::iterator iter = m_block_list.begin(); iter != m_block_list.end(); iter++)
	{
		request_block_t* block = *iter;
		if(block == NULL)
			continue;

		if(block->get_buffer() == NULL)
		{
			error = true;
			break;
		}

		ASSERT(block->StartOffset >= m_start && block->EndOffset <= m_end);
		if(block->StartOffset >= m_start && block->EndOffset <= m_end)
		{
			memcpy(pBuffer + (block->StartOffset - m_start), block->get_buffer(), block->length());
			block_list.insert(block->StartOffset, block->EndOffset+1);
		}
		else
		{
			error = true;
			break;
		}

		if(!block->finished)
		{
			error = true;
			break;
		}
	}

	// 检查是否完整
	bool has_gap = true;
	vector<blocklist::block_t> blockvector;
	block_list.subset(m_start, m_end, blockvector);
	if(blockvector.size() == 1)
	{
		if(blockvector[0] == blocklist::block_t(m_start, m_end))
		{
			has_gap = false;
		}
	}

	ASSERT(!has_gap);
	if(has_gap)
	{
		error = true;
	}

	if(!error)
	{
		m_read = true;
	}

	return !error;

}

void CEmuleTaskSimpleRange::release_cache_auto()
{


	for(vector<request_block_t*>::iterator iter = m_block_list.begin(); iter != m_block_list.end(); iter++)
	{
		request_block_t* block = *iter;
		if(block == NULL)
			continue;

		block->release_cache_auto();
	}
}

void CEmuleTaskSimpleRange::get_range(uint64& start, uint64& end) const
{
	start 	= m_start;
	end		= m_end;
}

void request_block_t::range_to_blocks(uint64 start, uint64 end, std::list<request_block_t*>& requested_block_list)
{
	requested_block_list.clear();

	int part_begin 	= (int)(start / PARTSIZE);
	int part_end	= (int)(end / PARTSIZE);

	uint64 offset = start;
	blocklist block_list;

	for (int i = part_begin; i <= part_end; i++)
	{
		// part loop
		uint64 current_part_begin 	= i * PARTSIZE; 
		uint64 current_part_end		= (i+1) * PARTSIZE - 1;

		ASSERT(current_part_begin <= offset && offset <= current_part_end+1);

		// block loop
		while (true)
		{
			request_block_t*	pblock = new request_block_t();
			pblock->PartNum		= i;
			pblock->StartOffset	= offset;
			// pblock->end is the least value among offset + EMBLOCKSIZE -1, end, current_part_end
			pblock->EndOffset		= (offset + EMBLOCKSIZE - 1) < current_part_end ?  
				((offset + EMBLOCKSIZE - 1) < end ? (offset + EMBLOCKSIZE -1 ) : end) : 
				((current_part_end < end) ? current_part_end : end);

			offset = pblock->EndOffset + 1;
			requested_block_list.push_back(pblock);
			block_list.insert(pblock->StartOffset, pblock->EndOffset);

			if (pblock->EndOffset == current_part_end) 
			{
				break;
			}

			if (pblock->EndOffset == end)
			{
				return;
			}
		}
	}

	// 检查是否完整
	bool has_gap = true;
	vector<blocklist::block_t> blockvector;
	block_list.subset(start, end, blockvector);
	if(blockvector.size() == 1)
	{
		if(blockvector[0] == blocklist::block_t(start, end))
		{
			has_gap = false;
		}
	}

	ASSERT(!has_gap);
}

void request_block_t::bind_block_to_range(CEmuleTaskSimpleRange* range, request_block_t* block)
{
	ASSERT(range != NULL && block != NULL);
	if(range == NULL || block == NULL)
		return;

	range->add_block(block);
	block->add_range(range);
}

void request_block_t::unbind_block_from_range(CEmuleTaskSimpleRange* range, request_block_t* block)
{
	ASSERT(range != NULL && block != NULL);
	if(range == NULL || block == NULL)
		return;

	range->delete_block(block);
	block->delete_range(range);
}

bool request_block_t::is_range_list_empty()
{
	return m_range_list.empty();
}

bool request_block_t::is_requested()
{
	return !m_UpDownClient_list.empty();
}

bool request_block_t::has_send_request_to(const CUpDownClient* sender)
{
	vector<CUpDownClient*>::iterator find = std::find(m_UpDownClient_list.begin(), m_UpDownClient_list.end(), sender);
	return (find != m_UpDownClient_list.end());
}

//CEmuleTaskCache* CEmuleTaskSimpleRange::transfer_cache_owner()
//{
//	CEmuleTaskCache* p = m_cache_ptr;
//	m_cache_ptr	= NULL;
//
//	typedef std::list<request_block_t*>::iterator iterator_t;
//
//	iterator_t iter = m_unrequested_block_list.begin();
//	for ( ; iter != m_unrequested_block_list.end(); ++iter ) {
//		delete *iter;
//	}
//
//	iter = m_requested_block_list.begin();
//	for ( ; iter != m_requested_block_list.end(); ++iter ) {
//		delete *iter;
//	}
//
//	m_unrequested_block_list.clear();
//	m_requested_block_list.clear();
//
//	return p;
//}

//std::list<request_block_t*>::iterator CEmuleTaskSimpleRange::get_requested_block_iterator(uint64 start, uint64 end)
//{
//	for ( std::list<request_block_t*>::iterator iter = m_requested_block_list.begin(); iter != m_requested_block_list.end(); ++iter )
//	{
//		if ( start == (*iter)->StartOffset && end == (*iter)->EndOffset )
//			return iter;
//	}
//	return m_requested_block_list.end();
//}

void CEmuleTaskSimpleRange::add_block(request_block_t* block)
{


	vector<request_block_t*>::iterator find = std::find(m_block_list.begin(), m_block_list.end(), block);
	ASSERT(find == m_block_list.end());
	if(find == m_block_list.end())
	{
		m_block_list.push_back(block);
	}
}

void CEmuleTaskSimpleRange::delete_block(request_block_t* block)
{


	vector<request_block_t*>::iterator find = std::find(m_block_list.begin(), m_block_list.end(), block);
	ASSERT(find != m_block_list.end());
	if(find != m_block_list.end())
	{
		m_block_list.erase(find);
	}
}

void CEmuleTaskSimpleRange::unbind_blocks()
{
	// make a copy first
	vector<request_block_t*> block_list = m_block_list;

	for(vector<request_block_t*>::iterator iter = block_list.begin(); iter != block_list.end(); iter++)
	{
		request_block_t* block = *iter;
		if(block == NULL)
			continue;
		
		request_block_t::unbind_block_from_range(this, block);
	}
}

std::string CEmuleTaskSimpleRange::dump_unrequested_block()
{
	// "[0,5],[10,20]"

	std::stringstream ss;

	typedef std::list<request_block_t*>::iterator iterator_t;
	//for (iterator_t iter = m_unrequested_block_list.begin(); iter != m_unrequested_block_list.end(); ++iter)
	//{
	//	if(!ss.str().empty())
	//		ss << ",";

	//	ss << "[" << (*iter)->StartOffset << ',' << (*iter)->EndOffset << ']';
	//}

	std::string output = ss.str();
	return output;
}

std::string CEmuleTaskSimpleRange::dump_requested_block()
{
	// "[0,5],[10,20]"

	std::stringstream ss;

	typedef std::list<request_block_t*>::iterator iterator_t;
	//for (iterator_t iter = m_requested_block_list.begin(); iter != m_requested_block_list.end(); ++iter)
	//{
	//	if(!ss.str().empty())
	//		ss << ",";

	//	request_block_t* request_block = (*iter);

	//	if(request_block && request_block->pUpDownClient)
	//	{
	//		ss << ',' << request_block->pUpDownClient->GetUserName();
	//	}

	//	ss << "[" << (*iter)->StartOffset << ',' << (*iter)->EndOffset;
	//	if(request_block && request_block->pUpDownClient)
	//	{
	//		USES_CONVERSION;
	//		LPSTR x = W2A(request_block->pUpDownClient->GetUserName());
	//		ss << ',' << x;
	//	}
	//	ss << ']';
	//}

	std::string output = ss.str();
	return output;
}
