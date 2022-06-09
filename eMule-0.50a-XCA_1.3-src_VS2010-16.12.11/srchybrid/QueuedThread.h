#pragma once
#include <deque>
class Runnable;

class QueuedThread : public Poco::Thread
{
	std::deque<Runnable*> _queue;
	Poco::Event evNotEmpty;
	Poco::FastMutex mtQueue;
	bool _stop;
public:
	QueuedThread();
	void stop();
	void push_back(Runnable*runable);
	virtual void run();
	bool inQueue(Runnable*runable) const;
};
