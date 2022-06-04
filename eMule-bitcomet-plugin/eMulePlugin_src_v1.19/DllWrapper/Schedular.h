#pragma once

#include <vector>

struct simple_task_with_same_hash_t
{	
	uchar					hash[16];
	int						cur_index;
	std::vector<uint32>		task_id_vector;
};

class CSchedular
{
public:
	CSchedular(void);
	~CSchedular(void);

	bool	add_simple_id(const uchar hash[16], uint32 client_id);
	bool	remove_simple_id(const uchar hash[16], uint32 client_id);

	uint32  get_next_id_with_same_hash(const uchar hash[16]);
	void	clear();

private:
	std::vector<simple_task_with_same_hash_t>	simple_task_vec;
};
