#include "StdAfx.h"
#include "Engine/Minidump.h"
#include <windows.h>
#include <DbgHelp.h>
#include <ctime>

static bool CreateDump(PEXCEPTION_POINTERS pExceptionInfo)
{
  // create filename
  char pFilename[MAX_PATH];
  DWORD iFilenameLength = GetModuleFileName(NULL, pFilename, MAX_PATH);
  sprintf_s(pFilename + iFilenameLength, MAX_PATH - iFilenameLength, ".%lu.dmp", ((unsigned int)time(NULL)) - 1199163600);

  // load DLL
  HMODULE hDebugHelp = LoadLibraryA("DbgHelp.dll");
  if (hDebugHelp == NULL)
  {
    return false;
  }

  // get function from DLL
  //
  typedef BOOL (WINAPI *MiniDumpWriteDumpFn_t)(
    HANDLE hProcess,
    DWORD dwPid,
    HANDLE hFile,
    MINIDUMP_TYPE DumpType,
    CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
    CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
    CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam);

  MiniDumpWriteDumpFn_t pMiniDumpWriteDump = (MiniDumpWriteDumpFn_t)::GetProcAddress(hDebugHelp, "MiniDumpWriteDump");

  // couldnt find function
  if (pMiniDumpWriteDump == NULL)
  {
    return false;
  }

  // construct exception info
  _MINIDUMP_EXCEPTION_INFORMATION mdei;
  mdei.ThreadId = GetCurrentThreadId();
  mdei.ExceptionPointers = pExceptionInfo;
  mdei.ClientPointers = 0;


  HANDLE hDumpFile;

  // try to open a new dump file
  hDumpFile = CreateFile(pFilename,
                         GENERIC_WRITE,
                         FILE_SHARE_READ,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL,
                         NULL);
  if (hDumpFile == INVALID_HANDLE_VALUE)
  {
    return false;
  }

  // write dump
  if (pMiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), hDumpFile, MiniDumpNormal, &mdei, NULL, NULL))
  {
    // succeeded
    CloseHandle(hDumpFile);
    return true;
  }

  CloseHandle(hDumpFile);
  return false;
}
static LPTOP_LEVEL_EXCEPTION_FILTER _prevExceptionFilter = NULL;

// entry point where control comes on an unhandled exception
static LONG WINAPI TopLevelExceptionFilter(PEXCEPTION_POINTERS pExceptionInfo)
{
  static bool bBeenHere = false;

  if (!bBeenHere)
  {
    bBeenHere = true;
    CreateDump(pExceptionInfo);
  }

  if (_prevExceptionFilter) return _prevExceptionFilter(pExceptionInfo);
  else return EXCEPTION_CONTINUE_SEARCH;
}

void Minidump::Activate(void)
{
  _prevExceptionFilter = SetUnhandledExceptionFilter(TopLevelExceptionFilter);
}

void Minidump::Deactivate(void)
{
  SetUnhandledExceptionFilter(_prevExceptionFilter);
}
