#pragma once

//#define CATCH_MEMORY_LEAKS 1
#ifdef CATCH_MEMORY_LEAKS
  #define DEBUGNEW new( _CLIENT_BLOCK, __FILE__, __LINE__)
  #define new DEBUGNEW
  #define malloc(a) _malloc_dbg(a,_CLIENT_BLOCK, __FILE__, __LINE__)
  #define free(a) _free_dbg(a,_CLIENT_BLOCK)
#endif

/// Monitors allocations and reports leaks (CATCH_MEMORY_LEAKS must be defined)
class MemoryLeakFinder
{
public:
  /// Start monitoring memory allocations
  static void Activate(void);
  /// Writes memory leak info to debug output
  static void DumpStatistics(void);
};
