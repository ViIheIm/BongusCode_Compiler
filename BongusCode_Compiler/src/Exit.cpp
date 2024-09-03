#include "Exit.h"
#include <stdlib.h>
#include <stdio.h>

void Exit(ErrCodes errCode)
{
	wprintf(L"Compilation aborted with exit code %i (%s)\n", errCode, ErrorsToString[(i32)errCode]);
	exit((i32)errCode);
}
