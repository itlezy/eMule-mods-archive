#pragma once 
#include <list>

#include "SingletonEx.h"

class CEmuleTaskSimple;
class CEmuleTaskSimpleRange;


class CSimpleClientTaskManager : public SingletonEx<CSimpleClientTaskManager>
{
public:
	CSimpleClientTaskManager(void);
	~CSimpleClientTaskManager(void);

	bool add_task(const	uchar hash[16], uint32 	client_id);
	bool delete_task(const uchar hash[16]);

	bool is_in_task_list(const uchar hash[16]);
	int  task_num() const;

	void clear();

	// NOTE:不存在指定的的任务则返回NULL
	CEmuleTaskSimple* get_task_by_hash(const uchar hash[16]);
	CEmuleTaskSimple* get_task_by_id(const uint32 id);

	const uchar* clientid_to_hash(uint32 client_id);

	//	hasn到client_id转换函数
	//	参数：  hash - 要转换为client_id的hash值
	//	返回值：如果hash值存在，返回其对应的client_id；否则，返回 -1。
	uint32   hash_to_client_id(const uchar hash[16]);

	//	更新host拥有区块的状态
	//	该函数被周期性的调用，以及时更新区块状态
	void	update_pieces_status_from_host();	

private:
	std::list<CEmuleTaskSimple*> 	m_task_list;

public:
	const std::list<CEmuleTaskSimple*>& get_task_list() const {return m_task_list;};
};
