#pragma once
#include <deque>
class Runnable;

class QueuedThread : public Poco::Thread
{
	std::deque<Runnable*> _queue;
	std::deque<Runnable*> _failedqueue;
	Poco::Event evNotEmpty;
	mutable Poco::FastMutex mtQueue;
	Runnable* volatile _runnable;
	volatile bool _stop;
public:
	QueuedThread();
	void stop();
	void push_back(Runnable*runable);
	virtual void run();
	void remove(void*key) const;
};
