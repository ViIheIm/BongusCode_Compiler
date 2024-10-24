#pragma once
#include "Definitions.h"

/*
  String library tailor made to Clarsveila//Bonguscode
*/

// Copies a string literal into a CStr-array.
__forceinline void WriteCStr(const char* str, ui16 strLen, char* outStr)
{
	for (ui16 i = 0; i < strLen; i++) { outStr[i] = str[i]; }
}

// Narrows a wide string to a narrow string.
__forceinline void NarrowWideString(const wchar_t* wideStr, ui16 strLen, char* outStr)
{
	for (ui16 i = 0; i < strLen; i++) { outStr[i] = wideStr[i];	}
}

// Widens a narrow string to a wide string.
__forceinline void WidenNarrowString(const char* narrowStr, ui16 strLen, wchar_t* outStr)
{
	for (ui16 i = 0; i < strLen; i++) { outStr[i] = narrowStr[i]; }
}

// We can see if a wide string contains non-ascii characters by checking if the total sum of a wchar is greater than 127(max ascii value).
__forceinline bool IsStringAscii(const wchar_t* wideStr, ui16 strLen)
{
	for (ui16 i = 0; i < strLen; i++)
	{
		if (wideStr[i] > 127u)
		{
			return false;
		}
	}

	return true;
}

// Same procedure as above, only that this is for one char.
__forceinline bool IsUnicode(wchar_t chr)
{
	return chr > 127;
}