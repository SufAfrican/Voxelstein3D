#include "StdAfx.h"

#include <windows.h>
static LARGE_INTEGER _iTimeStart = {0};
static LARGE_INTEGER _iFrequency = {0};

double Time::GetTime(void)
{
  // first run
  if(_iFrequency.QuadPart == 0)
  {
    if(!QueryPerformanceFrequency(&_iFrequency))
    {
      printf("QueryPerformanceFrequency failed!");
      return 0;
    }
    if(!QueryPerformanceCounter(&_iTimeStart))
    {
      printf("QueryPerformanceCounter failed!");
      return 0;
    }
  }

  LARGE_INTEGER iTimeNow;
  if(!QueryPerformanceCounter(&iTimeNow))
    return 0;

  return (iTimeNow.QuadPart - _iTimeStart.QuadPart) / (double)(_iFrequency.QuadPart);
}
