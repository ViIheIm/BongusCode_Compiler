#include "symtable.h"

void SymTable::OpenScope(void)
{
	depth++;
}

void SymTable::CloseScope(void)
{
	depth--;
}

std::wstring SymTable::ComposeKey(const std::wstring& name, i16 scopeDepth)
{
	return std::wstring(name + std::wstring(L".") + std::to_wstring(scopeDepth));
}

void SymTable::EnterSymbol(const std::wstring& name, PrimitiveType type, ui32 size)
{
	SymTabEntry entry{ .name = name, .type = type, .size = size, .stackLocation = 0 };
	std::wstring composedKey = ComposeKey(name, depth);

	table.insert(std::pair{ composedKey, entry });
}

SymTabEntry* SymTable::RetrieveSymbol(const std::wstring& composedKey)
{
	return table.contains(composedKey) ? &table.at(composedKey) : nullptr;
}
