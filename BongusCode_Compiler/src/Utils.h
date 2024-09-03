#pragma once
#include <stdlib.h>
#include <stdio.h>

#ifdef _DEBUG
#define DoIfDebug(x) x
#else
#define DoIfDebug(x) __noop
#endif


// exit calls the destructors of thread-local objects and callbacks registered by atexit and _onexit.
// https://learn.microsoft.com/en-us/previous-versions/6wdz5232(v=vs.140)
// Because MSVC doesn't understand that we're exiting, any call to CompilerHardExit should be coalesced with a stub return for MSVC to catch on.
//#define HardExit(exitCode, exitMsg) exitMsg == nullptr ? CompilerHardExit(exitCode) : CompilerHardExit(exitCode, exitMsg); return
#define CompilerHardExit(exitCode) \
  wprintf("Compilation aborted, exit code: " L#exitCode L"\n"); \
  exit(exitCode); \
  return


#define GetArraySize(arr) (sizeof(arr) / sizeof(arr[0]))

#define ZeroMem(dest, size) memset(dest, 0, size)

namespace Utils
{
	void PrintCurrentWorkingDirectory(void);
}