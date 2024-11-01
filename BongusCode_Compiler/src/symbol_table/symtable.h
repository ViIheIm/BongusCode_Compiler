#pragma once
#include <unordered_map>
#include <string>
#include "../BongusTable.h"

struct SymTabEntry
{
	std::wstring name;

	bool isFunction;
	union
	{
		struct
		{
			PrimitiveType type;
			ui32 size;
			// Filled in codegen.cpp
			ui32 adress;
		} asVar;

		struct
		{
			PrimitiveType retType;
		} asFunction;
	};
};

/*
	Key composition.

	nihil Foo(ui64 bar)
	{
		i8 baz.
	}

	would yield the keys ".Foo" & ".Foo.bar" & ".Foo.baz" respectively.
*/

class SymTable
{
public:

	SymTable();
	~SymTable() = default;

	// These 2 are deprecated.
	void OpenScope(void);
	void CloseScope(void);

	// Use these instead.
	void OpenFunction(const std::wstring& name);
	void CloseFunction(void);

	std::wstring ComposeKey(const std::wstring& name);
	std::wstring ComposeGlobalKey(const std::wstring& name);

	SymTabEntry* EnterSymbol(const std::wstring& name, PrimitiveType type, ui32 size, const bool isFunction);

	// The key here should be composed with ComposeKey already.
	SymTabEntry* RetrieveSymbol(const std::wstring& composedKey);


private:

	std::unordered_map<std::wstring, SymTabEntry> table;
	
	// Deprecated
	i16 depth = 0;
	static const std::wstring s_globalNamespace;

	// For prepending unto variables.
	std::wstring currentFunction;
};

// Global symbol table instance.
inline SymTable g_symTable;