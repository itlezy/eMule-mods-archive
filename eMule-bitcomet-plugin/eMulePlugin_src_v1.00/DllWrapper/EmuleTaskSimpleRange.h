#pragma once

#include <list>
#include <vector>
#include <string>

class CEmuleTaskSimpleRange;
class CUpDownClient;
class CEmuleTaskCache;

class request_block_t
{
public:
	request_block_t()
		: StartOffset(0)
		, EndOffset(0)
		, PartNum(-1)
		, finished(false)
		, m_cache(NULL)
	{
	}

	request_block_t(uint64 start, uint64 end)
		: StartOffset(start)
		, EndOffset(end)
		, PartNum(-1)
		, finished(false)
		, m_cache(NULL)
	{
	}

	~request_block_t();

	bool operator == (const request_block_t& block1) const
	{ 
		return StartOffset == block1.StartOffset && EndOffset == block1.EndOffset; 
	}
	
	const uint32 length() {return uint32(EndOffset - StartOffset + 1);}
	void add_download_client(CUpDownClient* sender);
	void remove_download_client(const CUpDownClient* sender);
	void on_received_data(const BYTE *data, uint64 start, uint64 end, const CUpDownClient* sender);
	void get_finished_range_list(std::vector<CEmuleTaskSimpleRange*>& range_list);
	void release_cache_auto();
	const uchar* get_buffer();
	bool is_range_list_empty();
	bool is_requested();
	bool has_send_request_to(const CUpDownClient* sender);

public:
	static void range_to_blocks(uint64 start, uint64 end, std::list<request_block_t*>& requested_block_list);
	static void bind_block_to_range(CEmuleTaskSimpleRange* range, request_block_t* block);
	static void unbind_block_from_range(CEmuleTaskSimpleRange* range, request_block_t* block);
	static void get_suitable_parts(const std::list<request_block_t*>& block_list, const std::string& available_parts, 
		const std::vector<request_block_t*>& requested_blocks, int& suitable_part, std::vector<request_block_t*>& suitable_blocks);

public:
	uint64	StartOffset;
	uint64	EndOffset;
	int     PartNum;
	bool	finished;

protected:
	void add_range(CEmuleTaskSimpleRange* range);
	void delete_range(CEmuleTaskSimpleRange* range);

protected:
	CEmuleTaskCache* m_cache;
	std::vector<CUpDownClient*>	m_UpDownClient_list;
	std::vector<CEmuleTaskSimpleRange*>	m_range_list;
};

class CEmuleTaskSimpleRange
{
public:
	CEmuleTaskSimpleRange(uint64 start, uint64 end);
	~CEmuleTaskSimpleRange(void);

	void get_range(uint64& start, uint64& end) const;
	bool read_data(char* pBuffer);
	void release_cache_auto();
	void unbind_blocks();
	const uint32 length() {return uint32(m_end - m_start + 1);}

	bool operator == (const CEmuleTaskSimpleRange& range1) const
	{ 
		return m_start == range1.m_start && m_end == range1.m_end; 
	}

protected:
	friend class request_block_t;
	void add_block(request_block_t* block);
	void delete_block(request_block_t* block);

private:
	uint64		m_start;
	uint64		m_end;
	bool		m_read;

	std::vector<request_block_t*> m_block_list;

public:
	std::string dump_unrequested_block();
	std::string dump_requested_block();
};
