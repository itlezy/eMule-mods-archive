#include "StdAfx.h"
#include "SimpleClientTaskManager.h"
#include "EmuleTaskSimple.h"

#include "srchybrid/emule.h"
#include "srchybrid/DownloadQueue.h"

CSimpleClientTaskManager::CSimpleClientTaskManager(void)
{
}

CSimpleClientTaskManager::~CSimpleClientTaskManager(void)
{
	ASSERT( m_task_list.empty() );
}

/*
 * 	Purpose: add a task with hash and client_id to the task_manager
 *
 * 	Precondition: hash != NULL 
 *
 *	Return Value: if all preconditions meet, create a task with the hash and 
 *	id given by the params and return true. Otherwise it returns false.
 */
bool CSimpleClientTaskManager::add_task(const uchar hash[16], uint32 client_id)
{
	if ( hash == NULL )
		return false;

	CEmuleTaskSimple* task = get_task_by_hash(hash);
	if( task != NULL )
	{
		task->set_client_id(client_id);
		return true;
	}

	CEmuleTaskSimple* ptask = new CEmuleTaskSimple(hash, client_id);
	m_task_list.push_back(ptask);

	return true;
}

bool CSimpleClientTaskManager::delete_task(const uchar hash[16])
{
	if (hash == NULL) return false;

	typedef std::list<CEmuleTaskSimple*>::iterator iterator_t;

	iterator_t iter = m_task_list.begin();
	for ( ; iter != m_task_list.end(); ++iter ) {
		if ( memcmp(hash, (*iter)->get_hash(), 16) == 0 ) {
			delete (*iter);
			m_task_list.erase(iter);
			return true;
		}
	}

	return false;
}

bool CSimpleClientTaskManager::is_in_task_list(const uchar hash[16])
{
	if (hash == NULL)  return false;

	typedef std::list<CEmuleTaskSimple*>::iterator iterator_t;
	iterator_t iter = m_task_list.begin();

	for ( ; iter != m_task_list.end(); ++iter ) {
		if (memcmp(hash, (*iter)->get_hash(), 16) == 0)
			break;
	}

	return ( iter != m_task_list.end() );
}

int  CSimpleClientTaskManager::task_num() const
{
	return (int)m_task_list.size();
}

CEmuleTaskSimple* CSimpleClientTaskManager::get_task_by_hash(const uchar hash[16])
{
	if (hash == NULL) 		
		return NULL;

	typedef std::list<CEmuleTaskSimple*>::iterator iterator_t;
	iterator_t iter = m_task_list.begin();

	for ( ; iter != m_task_list.end(); ++iter ) {
		if (memcmp(hash, (*iter)->get_hash(), 16) == 0)
			break;
	}

	return iter == m_task_list.end() ? NULL : (*iter);
}

CEmuleTaskSimple* CSimpleClientTaskManager::get_task_by_id(const uint32 id)
{

	typedef std::list<CEmuleTaskSimple*>::iterator iterator_t;
	iterator_t iter = m_task_list.begin();

	for ( ; iter != m_task_list.end(); ++iter ) {
		if ((*iter)->get_client_id() == id)
			break;
	}
	return iter == m_task_list.end() ? NULL : (*iter);
}


void CSimpleClientTaskManager::clear()
{
	typedef std::list<CEmuleTaskSimple*>::iterator task_iterator_t;

	task_iterator_t task_iter = m_task_list.begin();

	for ( ; task_iter != m_task_list.end(); ++task_iter ) {
		delete *task_iter;
	}	

	m_task_list.clear();
}

const uchar* CSimpleClientTaskManager::clientid_to_hash(uint32 client_id)
{
	typedef std::list<CEmuleTaskSimple*>::iterator iterator_t;
	iterator_t iter = m_task_list.begin();

	for ( ; iter != m_task_list.end(); ++iter ) {
		if ( (*iter)->get_client_id() == client_id )
			break;
	}	

	return ( iter == m_task_list.end() ?  NULL : (*iter)->get_hash() );
}

uint32  CSimpleClientTaskManager::hash_to_client_id(const uchar hash[16])
{
	CEmuleTaskSimple* ptask = get_task_by_hash(hash);
	return ptask ? ptask->get_client_id() : 0xFFFFFFFF;
}

void CSimpleClientTaskManager::update_pieces_status_from_host()
{
	typedef std::list<CEmuleTaskSimple*>::iterator iterator_t;

	for (iterator_t iter = m_task_list.begin(); iter != m_task_list.end(); ++iter)
	{
		(*iter)->update_pieces_status_from_host();
	}
}
