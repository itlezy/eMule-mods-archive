#pragma once

class CCriticalSection_INL2 : public CRITICAL_SECTION
{
	DWORD m_dwSafeValue;
	static const DWORD s_dwSafeValue;
public:
	inline CCriticalSection_INL2() { InitializeCriticalSection(this); m_dwSafeValue = s_dwSafeValue; }
	inline ~CCriticalSection_INL2() { m_dwSafeValue = 0; DeleteCriticalSection(this); }
	inline void Enter() { if (m_dwSafeValue == s_dwSafeValue) EnterCriticalSection(this); }
	inline void Leave() { if (m_dwSafeValue == s_dwSafeValue) LeaveCriticalSection(this); }

	inline BOOL TryEnter(BOOL (WINAPI *pfnTryEnterCriticalSection)(CRITICAL_SECTION*))
		{ ASSERT(pfnTryEnterCriticalSection); return (m_dwSafeValue == s_dwSafeValue) && pfnTryEnterCriticalSection(this); }
};

class CRWLockLite
{
	CCriticalSection_INL2 m_csLock;
	LONG m_nReaders;
public:
	CRWLockLite() : m_nReaders(0) {}

	void ReadLock() { m_csLock.Enter(); InterlockedIncrement(&m_nReaders); m_csLock.Leave(); }
	void ReadUnlock() { InterlockedDecrement(&m_nReaders); }
	void WriteLock() { m_csLock.Enter(); while (m_nReaders) Sleep(0); }
	void WriteUnlock() { m_csLock.Leave(); }
};



enum SecuredVarErrors
{
	VariableNotInitialized,
	GetTooLongWaiting,
	PutTooLongWaiting
};


// Secured variable and array
template <class T> class CSecuredVar
{
	friend class T;
public:
	CSecuredVar()
	{
		m_bInitialized = false;
	}
	CSecuredVar(T NewVal)
	{
		Put(NewVal);
	}

	~CSecuredVar()
	{
	}

	T Get() const
	{
		#ifdef _DEBUG
		DWORD dwStart = ::GetTickCount();
		#endif // _DEBUG

//		CSingleLock AccessLock(&m_Access, TRUE);
		m_Access.ReadLock();

		if(!m_bInitialized)
			throw VariableNotInitialized;
		T TempVal = m_Val;

		#ifdef _DEBUG
		DWORD dwElapsed = ::GetTickCount() - dwStart;
		if(dwElapsed > 1000)
		{
			TRACE1("Waited %ld milliseconds to get value", dwElapsed);
			throw GetTooLongWaiting;
		}
		#endif // _DEBUG

		m_Access.ReadUnlock();

		return TempVal;
	}

	const T* Get(T* pOutVal) const
	{
		#ifdef _DEBUG
		DWORD dwStart = ::GetTickCount();
		#endif // _DEBUG

//		CSingleLock AccessLock(&m_Access, TRUE);
		m_Access.ReadLock();

		if(!m_bInitialized)
			throw VariableNotInitialized;
		*pOutVal = m_Val;

		#ifdef _DEBUG
		DWORD dwElapsed = ::GetTickCount() - dwStart;
		if(dwElapsed > 1000)
		{
			TRACE1("Waited %ld milliseconds to get value", dwElapsed);
			throw GetTooLongWaiting;
		}
		#endif // _DEBUG

		m_Access.ReadUnlock();

		return pOutVal;
	}

//	Direct value access when protection is not required (e.g. preferences loading)
	T* GetRaw()
	{
		m_bInitialized = true;	// assume it's initialized right after return
		return &m_Val;
	}

	void Put(const T &NewVal)
	{
#ifdef _DEBUG
		DWORD dwStart = ::GetTickCount();
#endif // _DEBUG

//		CSingleLock AccessLock(&m_Access, TRUE);
		m_Access.WriteLock();

		m_bInitialized = true;
		m_Val = NewVal;

#ifdef _DEBUG
		DWORD dwElapsed = ::GetTickCount() - dwStart;
		if(dwElapsed > 1000)
		{
			TRACE1("Waited %ld milliseconds to put value", dwElapsed);
			throw PutTooLongWaiting;
		}
#endif // _DEBUG

		m_Access.WriteUnlock();
	}
	void operator=(T NewVal)
	{
		Put(NewVal);
	}
	operator T() const
	{
		return Get();
	}
	T& operator=(CSecuredVar<T> &NewVal)
	{
		Put(NewVal.Get());
		return *this;
	}
	operator CSecuredVar<T>&()
	{
		return *this;
	}

private:
	T m_Val;

//	mutable CMutex	m_Access;
	mutable CRWLockLite	m_Access;
	bool	m_bInitialized;
};
