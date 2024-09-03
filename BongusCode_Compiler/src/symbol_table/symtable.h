#pragma once
#include <unordered_map>
#include <string>
#include "../BongusTable.h"

struct SymTabEntry
{
	std::wstring name;
	PrimitiveType type;

	ui32 size;

	// Filled in codegen.cpp
	ui32 stackLocation;
};

/*
	A symbol table key is composed of its name concatenated with a dot and its depth, e.g.
	the source

	{
		ui8 foo.

		{
			i8 bar.
		}
	}

	would yield the keys "foo.1" and "bar.2" respectively.
*/

class SymTable
{
public:

	void OpenScope(void);
	void CloseScope(void);
	inline const i16 GetScopeDepth(void) const { return depth; }

	std::wstring ComposeKey(const std::wstring& name, i16 scopeDepth);

	void EnterSymbol(const std::wstring& name, PrimitiveType type, ui32 size);

	// The key here should be composed with ComposeKey already.
	SymTabEntry* RetrieveSymbol(const std::wstring& composedKey);

	// bool DeclaredLocally(const std::wstring& key);

private:

	std::unordered_map<std::wstring, SymTabEntry> table;
	i16 depth = 0;

};

// Global symbol table instance.
inline SymTable g_symTable;