#pragma once

#include <list>
#include <vector>

class CEmuleTaskSimpleRange;
class CEmuleTaskCache;
class CUpDownClient;

class request_block_t;

class CEmuleTaskSimple
{
public:
	CEmuleTaskSimple(const uchar hash[16], uint32 client_id);
	~CEmuleTaskSimple();

	const uchar*			get_hash() const; 
	uint32					get_client_id() const;
	void					set_client_id(uint32 id);

	bool					add_range(uint64 start, uint64 end, std::list<request_block_t*>& requested_block_list);
	bool					delete_completed_range(CEmuleTaskSimpleRange* range);
	bool					has_finished_range();
	CEmuleTaskSimpleRange*	get_first_completed_range();
	CEmuleTaskSimpleRange*	find_finished_range(uint64 start, uint64 end);


	void					get_unfinished_block(std::list<request_block_t*>& block_list, bool need_unrequested) const;
	void					get_requested_block(std::vector<request_block_t*>& block_list, const CUpDownClient* sender) const;
	void					on_block_request_cancel(uint64 start, uint64 end, const CUpDownClient* client);
	void					on_block_request_received(const BYTE *data, uint64 start, uint64 end, uint64 block_start, uint64 block_end, const CUpDownClient* client);

	void 					close();
	void					update_pieces_status_from_host();
	void					prepare_export_exist_file_to_host();
	bool					get_export_block_range(uint64& offset, uint32& len);

protected:
	request_block_t*		find_block(uint64 start, uint64 end);

private:
	std::list<CEmuleTaskSimpleRange*>			m_uncompleted_range_list;
	std::list<CEmuleTaskSimpleRange*>			m_completed_range_list;

	std::list<request_block_t*>					m_block_list;

	uchar	m_hash[16];
	uint32	m_client_id;
	
public:
	bool	m_file_exist_already;
protected:
	struct export_progress_t
	{
		std::vector<bool> host_needed_pieces;
		uint32 piece_len;
		uint32 first_piece_len;
		uint32 export_pos;
		uint64 file_size;

		export_progress_t() : piece_len(0), first_piece_len(0), export_pos(0), file_size(0) {}
	};
	export_progress_t	m_export_progress;


public:
	std::string dump_uncompleted_range();
	std::string dump_completed_range();
	std::string dump_block_list();
};
