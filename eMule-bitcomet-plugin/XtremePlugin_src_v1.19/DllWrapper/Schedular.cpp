#include "StdAfx.h"
#include "Schedular.h"

#include <algorithm>

CSchedular::CSchedular(void)
{
}

CSchedular::~CSchedular(void)
{
}

bool CSchedular::add_simple_id(const uchar hash[16], uint32 client_id)
{
	ASSERT( hash != NULL );
	if ( hash == NULL )		return false;
	
	typedef std::vector<simple_task_with_same_hash_t>::iterator task_iterator_t;
	task_iterator_t	iter;

	for (iter = simple_task_vec.begin(); iter != simple_task_vec.end(); ++iter)
	{
		if ( memcmp(iter->hash, hash, 16) == 0 )
			break;
	}

	// 该hash值在表中原本不存在
	if ( iter == simple_task_vec.end() )
	{
		simple_task_with_same_hash_t	task_with_same_hash;

		memcpy(task_with_same_hash.hash, hash, 16);
		task_with_same_hash.cur_index					= 0;
		task_with_same_hash.task_id_vector.push_back(client_id);

		simple_task_vec.push_back(task_with_same_hash);
	}
	else
	// 该hash值在表中原本已经存在
	{
		const std::vector<uint32>& id_vec = iter->task_id_vector;
		if ( std::find(id_vec.begin(), id_vec.end(), client_id) != id_vec.end() )
		{
			// 重复添加相同的
			return false;
		}
		iter->task_id_vector.push_back(client_id);
	}
	return true;
}

bool CSchedular::remove_simple_id(const uchar hash[16], uint32 client_id)
{
	ASSERT( hash != NULL );
	if ( hash == NULL )		return false;
	
	typedef std::vector<simple_task_with_same_hash_t>::iterator task_iterator_t;
	typedef std::vector<uint32>::iterator						id_iterator_t;

	task_iterator_t	task_iter;
	id_iterator_t			id_iter;

	for (task_iter = simple_task_vec.begin(); task_iter != simple_task_vec.end(); ++task_iter)
	{
		if ( memcmp(task_iter->hash, hash, 16) == 0 )
			break;
	}

	if ( task_iter == simple_task_vec.end() )
	{
		return false;
	}

	int index = -1;
	std::vector<uint32>& id_vec = task_iter->task_id_vector;

	for (id_iter = id_vec.begin(); id_iter != id_vec.end(); ++id_iter)
	{
		index++;
		if ( client_id == *id_iter )
			break;
	}

	if ( id_iter == id_vec.end() )
	{
		return false;
	}

	

	if ( index < task_iter->cur_index )
	{
		task_iter->cur_index--;
		if (task_iter->cur_index == -1)
			task_iter->cur_index = (task_iter->task_id_vector.size() - 1);
	}

	if  ( index == task_iter->cur_index && index == (int)(id_vec.size() - 1))
	{
		task_iter->cur_index = id_vec.empty() ? -1 : 0;
	}

	task_iter->task_id_vector.erase(id_iter);

	if ( id_vec.empty() )
	{
		simple_task_vec.erase(task_iter);
	}

	return true;
}


uint32  CSchedular::get_next_id_with_same_hash(const uchar hash[16])
{
	ASSERT( hash != NULL );
	if ( hash == NULL )		return false;
	
	typedef std::vector<simple_task_with_same_hash_t>::iterator task_iterator_t;
	typedef std::vector<uint32>::iterator						id_iterator_t;

	task_iterator_t	task_iter;
	id_iterator_t			id_iter;

	for (task_iter = simple_task_vec.begin(); task_iter != simple_task_vec.end(); ++task_iter)
	{
		if ( memcmp(task_iter->hash, hash, 16) == 0 )
			break;
	}

	if ( task_iter == simple_task_vec.end() )
	{
		return 0xffffffff;
	}
	
	std::vector<uint32>& id_vec = task_iter->task_id_vector;
	ASSERT ( !id_vec.empty() );
	if ( id_vec.empty() )
	{
		return 0xffffffff;
	}

	ASSERT ( (task_iter->cur_index) < (int)id_vec.size() );
	if ( (task_iter->cur_index) >= (int)id_vec.size())
	{
		return 0xffffffff;
	}

	uint32 result = id_vec[task_iter->cur_index];
	task_iter->cur_index = (task_iter->cur_index + 1) % id_vec.size();

	return result;
}

void CSchedular::clear()
{
	simple_task_vec.clear();
}
