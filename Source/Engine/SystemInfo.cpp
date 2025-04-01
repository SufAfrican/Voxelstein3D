#include "StdAfx.h"
#include "Engine/SystemInfo.h"
#include <windows.h>

#define GETCPUID(func) \
  __asm mov	eax, func; \
	__asm cpuid; \
	__asm mov	a, eax; \
  __asm mov	d, edx; 

SystemInfo *GetSystemInfo(void)
{
  static SystemInfo _info;
  return &_info;
}

SystemInfo::SystemInfo()
{
  m_bSSE = false;
  m_bSSE2 = false;
  m_bMMX = false;
  m_b3DNow = false;

  // get max function index
  unsigned int iMaxFunc = 0;
  __asm mov	eax, 0;
	__asm cpuid;
  __asm mov	iMaxFunc, eax;

  // get features
  if(iMaxFunc >= 1)
  {
    unsigned int iFeatureFlags = 0;
    __asm mov	eax, 1;
	  __asm cpuid;
    __asm mov	iFeatureFlags, edx;

    if(iFeatureFlags & (1<<23))
      m_bMMX = true;

    if(iFeatureFlags & (1<<25))
      m_bSSE = true;

    if(iFeatureFlags & (1<<26))
      m_bSSE2 = true;
  }

  // get extended features
  if(iMaxFunc >= 0x80000001)
  {
    unsigned int iFeatureFlags = 0;
    __asm mov	eax, 0x80000001;
	  __asm cpuid;
    __asm mov	iFeatureFlags, edx;

    if(iFeatureFlags & (1<<31))
      m_b3DNow = true;
  }
}

void SystemInfo::GetMemory(unsigned int &iTotal, unsigned int &iFree)
{
  MEMORYSTATUS ms;
  ZeroMemory(&ms, sizeof(MEMORYSTATUS));
  GlobalMemoryStatus(&ms);
  iTotal = ms.dwTotalPhys/(1024*1024);
  iFree = ms.dwAvailPhys/(1024*1024);
}

unsigned int SystemInfo::GetNumCPUs(void)
{
  unsigned int iCPUs = 0;
  DWORD dwMaskProcess, dwMaskSystem;
  GetProcessAffinityMask(GetCurrentProcess(), &dwMaskProcess, &dwMaskSystem);
  for (int i = 0; i < 32; i++)
  {
    if ((dwMaskProcess >> i) & 1) iCPUs++;
  }
  return iCPUs;
}

unsigned int SystemInfo::GetCPUSpeed(int iCPU)
{
  DWORD dwBufSize = sizeof(DWORD);
  DWORD dwMHz = 0;
  HKEY hKey;

  char strRegKey[256];
  sprintf(strRegKey, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\%i", iCPU);

  // open the key where the proc speed is hidden
  int iError = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                             strRegKey,
                             0,
                             KEY_READ,
                             &hKey);
  if(iError != ERROR_SUCCESS)
  {
    return 0;
  }

  // get mhz key
  if(RegQueryValueEx(hKey, "~MHz", NULL, NULL, (LPBYTE) &dwMHz, &dwBufSize) != ERROR_SUCCESS)
    return 0;
 
  return dwMHz;
}