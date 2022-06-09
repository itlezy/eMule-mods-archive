// Time.h: interface for the CPreciseTime class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <time.h>

class CPreciseTime
{
public:
	CPreciseTime()
		:m_nRegularTime(0)
		,m_uPrecision(0)
	{ }
	CPreciseTime(time_t nRegular, USHORT uPrecision)
		:m_nRegularTime(nRegular)
		,m_uPrecision(uPrecision)
	{ }
	static CPreciseTime GetCurrentTime()
	{
		SYSTEMTIME stSystemTime;
		GetSystemTime(&stSystemTime);
		return CPreciseTime(time(NULL), stSystemTime.wMilliseconds);
	}
	long operator -(CPreciseTime& tm)
	{
		return ((long)(m_nRegularTime - tm.m_nRegularTime) * 1000 + m_uPrecision - tm.m_uPrecision);
	}

	__declspec(property(get=_GetRegular))	time_t	Time;
	__declspec(property(get=_GetPrecision))	USHORT	Precision;
	__declspec(property(get=_GetIsNull))	bool	IsNull;
	__declspec(property(get=_GetFullTime))	LPCTSTR	FullTime;

	time_t	_GetRegular() const { return m_nRegularTime; }
	USHORT	_GetPrecision() const { return m_uPrecision; }
	bool	_GetIsNull() const { return (m_nRegularTime == 0 && m_uPrecision == 0); }
	LPCTSTR _GetFullTime()
	{
		struct tm* tmInfo = localtime(&m_nRegularTime);
		_stprintf(m_szFullTime, _T("%d/%02d/%04d %d:%02d:%02d")
			,tmInfo->tm_mday
			,tmInfo->tm_mon
			,tmInfo->tm_year + 1900
			,tmInfo->tm_hour
			,tmInfo->tm_min
			,tmInfo->tm_sec
			);
		return m_szFullTime;
	}
protected:
	time_t	m_nRegularTime;
	USHORT	m_uPrecision;
	TCHAR	m_szFullTime[100];
};
