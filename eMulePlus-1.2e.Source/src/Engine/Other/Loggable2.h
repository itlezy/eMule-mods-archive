// Loggable2.h: interface for the CLoggable2 class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "../../Loggable.h"

//   1 -  99	Notice
// 100 - 149	Warning
// 150 - 199	Debug
// 200 - 255	Error
//         0	Reserved for extension
typedef enum
{
	LOG_START		= 0,
	LOG_NOTICE		= 1,
	LOG_WARNING		= 100,
	LOG_NORMAL_END	= 148,
	LOG_DEBUG_XML	= 149,
	LOG_DEBUG		= 150,
	LOG_ERROR		= 200,
	LOG_END			= 255
} EnumLogType;

class CLoggable2 : public CLoggable
{
public:
	static void AddLog(EnumLogType eType, LPCTSTR szLine, ...);
};
