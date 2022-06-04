#include "StdAfx.h"
#include "QueuedThread.h"
#include "OtherFunctions.h"
#include "Runnable.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

QueuedThread::QueuedThread() : _runnable(NULL), _stop(false)
{
}

void QueuedThread::stop(){
	_stop = true;
	evNotEmpty.set();
	join();
}

void QueuedThread::push_back(Runnable*runable)
{
	mtQueue.Lock();
	_queue.push_back(runable);
	mtQueue.Unlock();
	evNotEmpty.set();
}

void QueuedThread::run()
{
	DbgSetThreadName("QueuedThread");
	
	size_t failtick;
	while(true)
	{
		while(!_queue.empty())
		{
			mtQueue.Lock();
			Runnable* runnable = _queue.front();
			_runnable = runnable;
			_queue.pop_front();
			mtQueue.Unlock();
			if (!runnable->isRemoved())
			{
			setPriority(runnable->getPriority());
			if(!runnable->run())
			{
					_runnable = NULL;
					mtQueue.Lock();
					_failedqueue.push_back(runnable);
					mtQueue.Unlock();
				failtick = GetTickCount();
					continue;
			}
		}
			_runnable = NULL;
			delete runnable;
		}
		if(_stop)
			break;
		if(_failedqueue.empty())
		{
			evNotEmpty.wait();
			continue;
		}
		size_t current_tick = GetTickCount();
		if(current_tick - failtick <= 500)
		{
			evNotEmpty.wait(100);
			continue;
		}
		mtQueue.Lock();
		Runnable* runnable = _failedqueue.front();
		_runnable = runnable;
		_failedqueue.pop_front();
		mtQueue.Unlock();
		if (!runnable->isRemoved())
		{
			if (_failedqueue.size() == 0)
				failtick = current_tick;
			else
				failtick = current_tick - 200;
			setPriority(runnable->getPriority());
			if(!runnable->run())
			{
				_runnable = NULL;
				mtQueue.Lock();
				_failedqueue.push_back(runnable);
				mtQueue.Unlock();
			continue;
		}
	}
		_runnable = NULL;
		delete runnable;
	}
	for (std::deque<Runnable*>::const_iterator it = _queue.begin(); it != _queue.end(); ++it)
		delete *it;
	for (std::deque<Runnable*>::const_iterator it = _failedqueue.begin(); it != _failedqueue.end(); ++it)
		delete *it;
}

void QueuedThread::remove(void*key) const
{
	mtQueue.Lock();
	for (std::deque<Runnable*>::const_iterator it = _queue.begin(); it != _queue.end(); ++it)
		(*it)->setRemoved(key);
	for (std::deque<Runnable*>::const_iterator it = _failedqueue.begin(); it != _failedqueue.end(); ++it)
		(*it)->setRemoved(key);
	while (_runnable != NULL && _runnable->isRemoved())
		sleep(10);
	mtQueue.Unlock();
}
