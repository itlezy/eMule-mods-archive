#pragma once

class Runnable
{
public:
	Runnable(Poco::Thread::Priority prio = Poco::Thread::PRIO_NORMAL) :_prio(prio), removed(false){}
	virtual ~Runnable(){}
	Poco::Thread::Priority getPriority() const{return _prio;}
	virtual bool run() = 0;
	virtual void setRemoved(void*key) = 0;
	bool isRemoved() const{ return removed; }
protected:
	Poco::Thread::Priority     _prio;
	bool removed;
};
