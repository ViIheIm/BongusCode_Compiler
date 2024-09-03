#pragma once
#include "Definitions.h"

enum class ErrCodes : i32
{
	success, // Typically unused
	malformed_cmd_line,
	undeclared_symbol,
	syntax_error,
	unknown_type,
	unreachable_code
};

inline const wchar_t* ErrorsToString[] = {
	L"Success",
	L"Malformed command line arguments list",
	L"Undeclared symbol",
	L"Syntax error",
	L"Unknown type",
	L"Unreachable code"
};

[[noreturn]] void Exit(ErrCodes errCode);