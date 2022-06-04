#pragma once

template<class T>
class ThreadLocal
{
public:
	ThreadLocal(): _slot(TlsAlloc())
	{
		ATLASSERT(_slot != TLS_OUT_OF_INDEXES);
	}
	~ThreadLocal()
	{
		TlsFree(_slot);
	}
	void init(T* t)
	{
		TlsSetValue(_slot, t);
	}
	inline T* get()
	{
		T* pT = reinterpret_cast<T*>(TlsGetValue(_slot));
		ATLASSERT(pT != NULL);
		return pT;
	}
	T& operator*()
	{
		return *get();
	}
	T* operator->()
	{
		return get();
	}
	void release()
	{
		delete get();
	}
private:
	DWORD _slot;
};

template<class T>
class ThreadLocalPtr
{
public:
	ThreadLocalPtr(ThreadLocal<T>* tl, T* t = new T()) : _tl(tl){ _tl->init(t); }
	~ThreadLocalPtr(){ _tl->release(); }
private:
	ThreadLocal<T>*_tl;
};
#define DEF_THREAD_LOCAL(name, T, args) \
	ThreadLocal<T> name;\
	static ThreadLocalPtr<T> p##name(&name, new T args);
