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

	// NOTE:������ָ���ĵ������򷵻�NULL
	CEmuleTaskSimple* get_task_by_hash(const uchar hash[16]);
	CEmuleTaskSimple* get_task_by_id(const uint32 id);

	const uchar* clientid_to_hash(uint32 client_id);

	//	hasn��client_idת������
	//	������  hash - Ҫת��Ϊclient_id��hashֵ
	//	����ֵ�����hashֵ���ڣ��������Ӧ��client_id�����򣬷��� -1��
	uint32   hash_to_client_id(const uchar hash[16]);

	//	����hostӵ�������״̬
	//	�ú����������Եĵ��ã��Լ�ʱ��������״̬
	void	update_pieces_status_from_host();	

private:
	std::list<CEmuleTaskSimple*> 	m_task_list;

public:
	const std::list<CEmuleTaskSimple*>& get_task_list() const {return m_task_list;};
};
