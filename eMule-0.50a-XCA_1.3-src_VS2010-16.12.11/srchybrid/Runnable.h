#pragma once

class Runnable
{
public:
	Runnable(Poco::Thread::Priority prio = Poco::Thread::PRIO_NORMAL) :_prio(prio){}
	virtual ~Runnable(){}
	Poco::Thread::Priority getPriority() const{return _prio;}
	virtual bool run() = 0;
protected:
	Poco::Thread::Priority     _prio;
};
