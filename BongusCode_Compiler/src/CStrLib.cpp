#include "CStrLib.h"
#include "Utils.h"

/*
	Name mangling in bonguscode is like how most C-compilers mangle their names.
	Firstly, we prepend an underscore.
	Secondly, and this is the more involved operation;
	In the case of unicode characters, we expand out the unicode character
	into a string of it's hex value.
*/
constexpr ui16 hexExpansionAmount = 4;

void Mangle(wchar_t* outStr, const ui16 newStrLen)
{
	const ui16 newStrByteLen = sizeof(wchar_t) * newStrLen;

	wchar_t* tempMem = (wchar_t*)malloc(newStrByteLen);
	ZeroMem(tempMem, newStrByteLen);

	// Start by prepending an underscore.
	wcscpy(tempMem, outStr);
	*outStr = L'_';

	// Copy everything back.
	wcscpy(outStr + 1, tempMem);

	// Clear tempMem.
	ZeroMem(tempMem, newStrByteLen);


	for (wchar_t* c = outStr; *c != 0; c++)
	{
		// Only mangle unicode characters.
		if (IsUnicode(*c))
		{
			// Copy over everything after c to tempMem.
			wcscpy(tempMem, c + 1);

			swprintf(c, L"%.*X", hexExpansionAmount, *c);

			// Copy everything after c back.
			wcscpy(c + hexExpansionAmount, tempMem);

			// Clear tempMem.
			ZeroMem(tempMem, newStrByteLen);

			// We're not iterating c here because if it's at the end of the string, and there's a unicode character there, it will skip past said character.
			// Besides, no character we just wrote will be a unicode character, so it's safe to reread characters we just wrote.
		}
	}
}

std::string MangleFunctionName(const wchar_t* wstr)
{
	// Figure out new size, taking unicode character expansion into account.
	// It's defaulted to 1 to take leading underscore into account.
	ui16 newStrLen = 1;
	for (const wchar_t* c = wstr; *c != 0; c++)
	{
		if (IsUnicode(*c))
		{
			newStrLen += hexExpansionAmount;
		}
		else
		{
			newStrLen += 1;
		}
	}
	// Add size for null terminator.
	newStrLen += 1;

	wchar_t* expandedWideStr = (wchar_t*)malloc(sizeof(wchar_t) * newStrLen);
	ZeroMem(expandedWideStr, sizeof(wchar_t) * newStrLen);
	wcscpy(expandedWideStr, wstr);
	Mangle(expandedWideStr, newStrLen);
	char* narrowedStr = (char*)malloc(sizeof(char) * newStrLen);
	NarrowWideString(expandedWideStr, newStrLen, narrowedStr);
	free(expandedWideStr);

	std::string result(narrowedStr);

	free(narrowedStr);

	return result;
}

std::string GetNarrowedString(const wchar_t* wideStr)
{
	std::string result;

	for (const wchar_t* C = wideStr; *C != 0; C++)
	{
		result += *C;
	}

	return result;
}