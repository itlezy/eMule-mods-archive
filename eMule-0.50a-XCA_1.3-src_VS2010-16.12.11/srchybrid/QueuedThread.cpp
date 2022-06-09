#include "StdAfx.h"
#include "QueuedThread.h"
#include "OtherFunctions.h"
#include "Runnable.h"
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

QueuedThread::QueuedThread() : _stop(false)
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
	
	std::deque<Runnable*> failed;
	size_t failtick;
	while(true)
	{
		while(!_queue.empty())
		{
			mtQueue.Lock();
			Runnable* runnable = _queue.front();
			_queue.pop_front();
			mtQueue.Unlock();
			setPriority(runnable->getPriority());
			if(!runnable->run())
			{
				failed.push_back(runnable);
				failtick = GetTickCount();
			}
		}
		if(_stop)
			break;
		if(failed.empty())
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
		if (failed.size() == 1)
		{
			Runnable* runnable = failed.front();
			setPriority(runnable->getPriority());
			if(!runnable->run())
				failtick = current_tick;
			else
				failed.pop_front();
			continue;
		}
		Runnable* runnable = failed.front();
		failed.pop_front();
		setPriority(runnable->getPriority());
		if(!runnable->run())
			failed.push_back(runnable);
		failtick = current_tick - 200;
	}
	for (size_t i = 0 ; i < failed.size(); ++i)
		delete failed[i];
}

bool QueuedThread::inQueue(Runnable*runable) const
{
	std::deque<Runnable*>::const_iterator it = std::find(_queue.begin(), _queue.end(), runable);
	return it != _queue.end();
}
