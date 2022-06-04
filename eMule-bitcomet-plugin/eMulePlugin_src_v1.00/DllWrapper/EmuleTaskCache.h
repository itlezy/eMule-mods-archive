#pragma once

#include <list>
#include <string>

struct range_data_t
{
	range_data_t(uint64 range_start, uint64 range_end)
	{
		start	= range_start;
		end		= range_end;
	}

	range_data_t()
	{
		start	= 0;
		end		= 0;
	}

	uint64 	start;
	uint64	end;
};

class CEmuleTaskCache
{
public:
	CEmuleTaskCache(uint64 range_start, uint64 range_end);
	~CEmuleTaskCache();

	bool			insert(uint64 start, uint64 end, const BYTE* data);
	bool			erase(uint64 start, uint64 end);
	bool			is_complete(uint64 start, uint64 end);
	bool			is_empty() const { return m_buffer_data == NULL; }
	void			clear();

	void			get_range(uint64& start, uint64& end) const;
	const uchar*	get_buffer() const { return m_buffer_data; }

private:
	bool init_buffer_size(size_t bufsize);
	void release_buffer();

private:
	// should be in sorted order
	std::list<range_data_t>			m_uncompleted_range_list;
	uchar*							m_buffer_data;

	uint64							m_total_range_start;
	uint64							m_total_range_end;

public:
	std::string dump();
};
