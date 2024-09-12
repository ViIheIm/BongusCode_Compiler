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

void SymTable::EnterSymbol(const std::wstring& name, PrimitiveType type, ui32 size, const bool isFunction)
{
	// SymTabEntry entry{ .name = name, .type = type, .size = size, .adress = 0, .isFunction = isFunction };
	SymTabEntry entry;
	entry.name = name;
	entry.isFunction = isFunction;
	if (isFunction)
	{
		entry.asFunction.retType = type;
	}
	else // It's a variable
	{
		entry.asVar.type = type;
		entry.asVar.size = size;
		entry.asVar.adress = 0;
	}

	std::wstring composedKey = ComposeKey(name, depth);

	table.insert(std::pair{ composedKey, entry });
}

SymTabEntry* SymTable::RetrieveSymbol(const std::wstring& composedKey)
{
	return table.contains(composedKey) ? &table.at(composedKey) : nullptr;
}
