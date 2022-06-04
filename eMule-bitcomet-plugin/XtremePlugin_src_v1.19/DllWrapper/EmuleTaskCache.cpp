#include "StdAfx.h"
#include "EmuleTaskCache.h"
#include <algorithm>
#include <sstream>

struct range_begin_less
{
	inline bool operator()( const range_data_t& a, const range_data_t& b) const
	{
		return a.start < b.start;
	}
	inline bool operator()( const range_data_t& a, const uint64& b) const
	{
		return a.start < b;
	}
	inline bool operator()( const uint64& a, const range_data_t& b) const
	{
		return a < b.start;
	}
};

CEmuleTaskCache::CEmuleTaskCache(uint64 range_start, uint64 range_end)
{
	ASSERT(range_end >= range_start);

	m_uncompleted_range_list.clear();
	m_buffer_data	= NULL;
	m_total_range_start = range_start;
	m_total_range_end	= range_end;
	
	uint64 length = range_end - range_start + 1;
	init_buffer_size((size_t)length);
}

bool CEmuleTaskCache::init_buffer_size(size_t bufsize)
{
	ASSERT(bufsize != 0);
	if (bufsize == 0)
	{
		return false;
	}

	if (m_buffer_data != NULL)
	{
		delete m_buffer_data;
		m_buffer_data = NULL;
	}

	try
	{
		m_buffer_data = new uchar[bufsize];
		memset(m_buffer_data, 0, bufsize);
	}
	catch (std::bad_alloc)
	{
		m_buffer_data = NULL;
		return false;
	}

	return true;
}

CEmuleTaskCache::~CEmuleTaskCache(void)
{
	clear();
	release_buffer();
}

// NOTE: data length = end - start
// [start, end)
bool CEmuleTaskCache::insert(uint64 start, uint64 end, const BYTE* data)
{
	// TODO: merge

	ASSERT(start <= end && data != NULL);
	if (start > end || data == NULL)
		return false;

	if (m_buffer_data == NULL)
		return false;

	if (is_complete(start, end))
		return true;

	std::list<range_data_t>::iterator iter_head;
	std::list<range_data_t>::iterator iter_insert;
	std::list<range_data_t>::iterator iter_insert_prev;


	iter_insert = std::lower_bound(m_uncompleted_range_list.begin(), m_uncompleted_range_list.end(), start, range_begin_less() );
	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_insert != m_uncompleted_range_list.begin() )
	{
		iter_head = iter_insert;
		--iter_head;
	}
	else
		iter_head = iter_insert;

	// if the head is overlapped, extend the overlapped existed block
	if ( iter_head != m_uncompleted_range_list.end() && iter_head->start <= start && start <= iter_head->end )
	{
		iter_head->end = max( iter_head->end, end );
		iter_insert = iter_head;
	}
	else
	{
		iter_insert = m_uncompleted_range_list.insert( iter_insert, range_data_t(start, end) );
	}

	// find the position of the block just before "end"
	std::list<range_data_t>::iterator iter_tail = m_uncompleted_range_list.end();
	std::list<range_data_t>::iterator iter_last = std::lower_bound( iter_insert, m_uncompleted_range_list.end(), end, range_begin_less() );
	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_last != m_uncompleted_range_list.begin() )
	{
		iter_tail = iter_last;
		--iter_tail;
	}
	else
		iter_tail = iter_last;

	// if the tail is overlapped, extend the inserted new block
	if ( iter_tail != m_uncompleted_range_list.end() && iter_tail->start <= end && end <= iter_tail->end )
	{
		iter_insert->end = max( iter_insert->end, iter_tail->end );
	}

	// remove the overlapped blocks after the insert point
	if ( iter_insert != iter_last )
	{
		std::list<range_data_t>::iterator	iter_tmp = iter_insert;
		++iter_tmp;
		m_uncompleted_range_list.erase(iter_tmp, iter_last);
	}

	std::list<range_data_t>::iterator		iter_insert_next = iter_insert;

	if ( iter_insert_next != m_uncompleted_range_list.end() )
		++iter_insert_next;

	if ( iter_insert_next != m_uncompleted_range_list.end() && iter_insert_next != iter_insert && iter_insert_next->start == iter_insert->end )
	{
		iter_insert->end = iter_insert_next->end;
		m_uncompleted_range_list.erase(iter_insert_next);
	}

	uint64 offset = start - m_total_range_start;
	int length = (int)(end - start);
	memcpy(m_buffer_data + (size_t)offset, data, length);

	return true;
}

// NOTE : 区间为前闭后开区间　[start, end)
bool CEmuleTaskCache::is_complete(uint64 start, uint64 end)
{
	ASSERT(start <= end);
	if (start > end)
	{
		return false;
	}

	for ( std::list<range_data_t> ::const_iterator iter = m_uncompleted_range_list.begin(); 
			iter != m_uncompleted_range_list.end();
			++iter
			)
	{
		if (iter->start <= start && iter->end >= end)
		{
			return true;
		}
	}
	return false;
}

void CEmuleTaskCache::clear()
{
	m_uncompleted_range_list.clear();
}

void CEmuleTaskCache::release_buffer()
{
	if (m_buffer_data != NULL)
	{
		delete[] m_buffer_data;
		m_buffer_data	= NULL;
	}
}

void CEmuleTaskCache::get_range(uint64& start, uint64& end) const
{
	start 	= m_total_range_start;
	end		= m_total_range_end;	
}

std::string CEmuleTaskCache::dump()
{
	// "[0,5),[10,20)"

	std::stringstream ss;

	typedef std::list<range_data_t>::iterator iterator_t;
	for (iterator_t iter = m_uncompleted_range_list.begin(); iter != m_uncompleted_range_list.end(); ++iter)
	{
		if(!ss.str().empty())
			ss << ",";

		ss << "[" << iter->start << ',' << iter->end << ')';
	}

	std::string output = ss.str();
	return output;
}
