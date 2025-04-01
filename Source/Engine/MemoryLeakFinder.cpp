#include "StdAfx.h"
#include <crtdbg.h>

#ifdef CATCH_MEMORY_LEAKS
static _CrtMemState Checkpoint;
#endif

void MemoryLeakFinder::Activate(void)
{
#ifdef CATCH_MEMORY_LEAKS
  _CrtMemCheckpoint(&Checkpoint);
#endif
}

void MemoryLeakFinder::DumpStatistics(void)
{
#ifdef CATCH_MEMORY_LEAKS
  _CrtMemDumpAllObjectsSince(&Checkpoint);
  _CrtMemDumpStatistics(&Checkpoint);
  _CrtCheckMemory();
#endif
}