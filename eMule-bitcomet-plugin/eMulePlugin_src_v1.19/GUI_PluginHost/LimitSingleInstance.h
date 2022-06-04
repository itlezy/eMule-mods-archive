//Microsoft Knowledge Base Article - 243953

#ifndef LimitSingleton_H
#define LimitSingleton_H

#include <windows.h> 


// 窗口识别消息 
#define WMU_WHERE_ARE_YOU_MSG _T("WMU_WHERE_ARE_YOU-{F17F89AB-1825-464F-82C3-4161E40E2A48}")
const UINT WMU_WHERE_ARE_YOU = ::RegisterWindowMessage( WMU_WHERE_ARE_YOU_MSG );

//this code is from Q243953 in case you lose the article and wonder
//where this code came from...
class CLimitSingleton
{
protected:
  DWORD  m_dwLastError;
  HANDLE m_hMutex;

public:
  CLimitSingleton(TCHAR *strMutexName)
  {
    //be sure to use a name that is unique for this application otherwise
    //two apps may think they are the same if they are using same name for
    //3rd parm to CreateMutex
    m_hMutex = CreateMutex(NULL, FALSE, strMutexName); //do early
    m_dwLastError = GetLastError(); //save for use later...
  }
   
  ~CLimitSingleton() 
  {
    if (m_hMutex)  //don't forget to close handles...
    {
       CloseHandle(m_hMutex); //do as late as possible
       m_hMutex = NULL; //good habit to be in
    }
  }

  BOOL IsAnotherInstanceRunning() 
  {
    return (ERROR_ALREADY_EXISTS == m_dwLastError);
  }
};
#endif
