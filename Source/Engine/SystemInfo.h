#pragma once

class SystemInfo
{
public:
  SystemInfo();

  /// Returns true if SSE is supported
  inline bool HasSSE(void) { return m_bSSE; }
  /// Returns true if MMX is supported
  inline bool HasMMX(void) { return m_bMMX; }
  /// Returns true if SSE2 is supported
  inline bool HasSSE2(void) { return m_bSSE2; }
  /// Returns true if 3DNow! is supported
  inline bool Has3DNow(void) { return m_b3DNow; }

  /// Returns amount of memory in megabytes
  void GetMemory(unsigned int &iTotal, unsigned int &iFree);

  /// Returns number of CPUs
  unsigned int GetNumCPUs(void);
  /// Returns CPU info
  unsigned int GetCPUSpeed(int iCPU = 0);

private:
  bool m_bSSE;
  bool m_bSSE2;
  bool m_bMMX;
  bool m_b3DNow;
};

extern SystemInfo *GetSystemInfo(void);