#include "StdAfx.h"

#include <vector>
#include <algorithm>
#include <sstream>
using namespace std;

typedef uint64 uint64_t;

#include "blocklist.h"

#define bcassert ASSERT

struct range_begin_less
{
	inline bool operator()( const blocklist::block_t& a, const blocklist::block_t& b) const
	{
		return a.first < b.first;
	}
	inline bool operator()( const blocklist::block_t& a, const uint64_t& b) const
	{
		return a.first < b;
	}
	inline bool operator()( const uint64_t& a, const blocklist::block_t& b) const
	{
		return a < b.first;
	}
};

void blocklist::insert(const uint64_t begin, const uint64_t end)
{
	bcassert( begin <= end );
	if(!(begin<end))
	{
		return;
	}
	// find the position of the block just before "begin"
	blocks_t::iterator iter_head = m_blocklist.end();
	blocks_t::iterator iter_insert = std::lower_bound( m_blocklist.begin(), m_blocklist.end(), begin, range_begin_less() );
	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_insert != m_blocklist.begin() )
		iter_head = iter_insert - 1;
	else
		iter_head = iter_insert;

	// if the head is overlapped, extend the overlapped existed block
	if ( iter_head != m_blocklist.end() && iter_head->first <= begin && begin <= iter_head->second )
	{
		iter_head->second = max( iter_head->second, end );
		iter_insert = iter_head;
	}
	else
	{
		iter_insert = m_blocklist.insert( iter_insert, block_t(begin,end) );
	}

	// find the position of the block just before "end"
	blocks_t::iterator iter_tail = m_blocklist.end();
	blocks_t::iterator iter_last = std::lower_bound( iter_insert, m_blocklist.end(), end, range_begin_less() );
	if ( iter_last != m_blocklist.end() && iter_last->first == end )
		iter_last ++;

	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_last != m_blocklist.begin() )
		iter_tail = iter_last - 1;
	else
		iter_tail = iter_last;

	// if the tail is overlapped, extend the inserted new block
	if ( iter_tail != m_blocklist.end() && iter_tail->first <= end && end <= iter_tail->second )
	{
		iter_insert->second = max( iter_insert->second, iter_tail->second );
	}

	// remove the overlapped blocks after the insert point
	if ( iter_insert != iter_last )
		m_blocklist.erase(iter_insert + 1, iter_last);
}

void blocklist::erase(const uint64_t begin, const uint64_t end)
{
	bcassert( begin <= end );
	if(!(begin<end))
	{
		return;
	}
	// find the position of the block just before "begin"
	blocks_t::iterator iter_head = m_blocklist.end();
	blocks_t::iterator iter_insert = std::lower_bound( m_blocklist.begin(), m_blocklist.end(), begin, range_begin_less() );
	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_insert != m_blocklist.begin() )
		iter_head = iter_insert - 1;
	else
		iter_head = iter_insert;

	// if the head is overlapped, split the overlapped existed block
	if ( iter_head != m_blocklist.end() && iter_head->first < begin && begin <= iter_head->second )
	{
		block_t block = *iter_head;
		iter_head = m_blocklist.insert( iter_head, block );
		iter_head->second = begin;
		(iter_head + 1)->first = begin;
		iter_insert = iter_head + 1;
	}

	// find the position of the block just before "end"
	blocks_t::iterator iter_tail = m_blocklist.end();
	blocks_t::iterator iter_last = std::lower_bound( iter_insert, m_blocklist.end(), end, range_begin_less() );
	//if ( iter_last != m_blocklist.end() && iter_last->first == end )
	//	iter_last ++;
	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_last != m_blocklist.begin() )
		iter_tail = iter_last - 1;
	else
		iter_tail = iter_last;

	// if the tail is overlapped, split the inserted new block
	if ( iter_tail != m_blocklist.end() && iter_tail->first <= end && end < iter_tail->second )
	{
		iter_tail->first = end;
		iter_last = iter_tail;
	}

	// remove the overlapped blocks after the insert point
	m_blocklist.erase(iter_insert, iter_last);
}

void blocklist::subset(const uint64_t begin, const uint64_t end, blocks_t& blockvector) const
{
	bcassert( begin <= end );
	if(!(begin<end))
	{
		return;
	}
	blockvector.clear();

	// find the position of the block just before "begin"
	blocks_t::const_iterator iter_head = m_blocklist.end();
	blocks_t::const_iterator iter_insert = std::lower_bound( m_blocklist.begin(), m_blocklist.end(), begin, range_begin_less() );
	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_insert != m_blocklist.begin() )
		iter_head = iter_insert - 1;
	else
		iter_head = iter_insert;

	// if the head is overlapped, insert the overlapped existed block
	if ( iter_head != m_blocklist.end() && iter_head->first <= begin && begin < iter_head->second )
	{
		blockvector.insert( blockvector.end(), block_t(begin, min( iter_head->second, end ) ) );
		iter_insert = iter_head + 1;
	}

	// find the position of the block just before "end"
	blocks_t::const_iterator iter_tail = m_blocklist.end();
	blocks_t::const_iterator iter_last = std::lower_bound( iter_insert, m_blocklist.end(), end, range_begin_less() );
	// iter_insert is the block just after "begin", so iter_head is the one before that
	if ( iter_last != m_blocklist.begin() )
		iter_tail = iter_last - 1;
	else
		iter_tail = iter_last;

	// insert the overlapped blocks after the insert point
	if ( iter_insert != iter_last )
		blockvector.insert( blockvector.end(), iter_insert, iter_last );

	// if the tail is overlapped, correct the end point
	if ( iter_tail != m_blocklist.end() && iter_tail->first < end && end <= iter_tail->second )
	{
		bcassert( !blockvector.empty() );
		blockvector[blockvector.size()-1].second = end;
	}
}

void blocklist::subset(const uint64_t begin, const uint64_t end, blocklist& blocklist) const
{
	subset(begin, end, blocklist.m_blocklist);
}

void blocklist::insert(const blocks_t& blockvector)
{
	for(blocks_t::const_iterator iter = blockvector.begin(); iter != blockvector.end(); iter++ )
		insert( iter->first, iter->second );
}

void blocklist::insert(const blocklist& blocklist)
{
	insert( blocklist.m_blocklist );
}

void blocklist::erase(const blocks_t& blockvector)
{
	for(blocks_t::const_iterator iter = blockvector.begin(); iter != blockvector.end(); iter++ )
		erase( iter->first, iter->second );
}

void blocklist::erase(const blocklist& blocklist)
{
	erase( blocklist.m_blocklist );
}

uint64_t blocklist::size(void) const
{
	uint64_t size = 0;
	for(blocks_t::const_iterator iter = m_blocklist.begin(); iter != m_blocklist.end(); iter++ )
	{
		size += iter->second - iter->first;
	}
	return size;
}

void blocklist::clear(void)
{
	m_blocklist.clear();
}

bool blocklist::empty(void) const
{
	return m_blocklist.empty();
}

bool blocklist::in(const uint64_t pos,block_t& range)
{
	blocks_t::const_iterator itr
		= std::upper_bound( m_blocklist.begin(), m_blocklist.end(), pos, range_begin_less() );

	if(itr==m_blocklist.begin())
	{
		return false;
	}

	--itr;

	if(pos>=itr->first&&pos<itr->second)
	{
		range=*itr;
		return true;
	}
	else
	{
		return false;
	}
}

bool blocklist::intersection(const block_t& range)
{
	if(m_blocklist.empty())
	{
		return false;
	}

	blocks_t::const_iterator itr
		= std::upper_bound( m_blocklist.begin(), m_blocklist.end(), range, range_begin_less() );

	if(itr==m_blocklist.end())
	{
		--itr;
		if(range.first<itr->second)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	if(range.second<itr->first)
	{
		if(itr==m_blocklist.begin())
		{
			return false;
		}

		--itr;
		if(range.first >= itr->second)
		{
			return false;
		}
	}
	return true;
}

void blocklist::dump(string& dump) const
{
	dump.clear();

	std::stringstream ss;
	for(blocks_t::const_iterator iter = m_blocklist.begin(); iter != m_blocklist.end(); iter++ )
	{
		const block_t& range = *iter;
		ss << "[";
		ss << range.first;
		ss << ",";
		ss << range.second;
		ss << ")";
	}

	dump = ss.str();
}

