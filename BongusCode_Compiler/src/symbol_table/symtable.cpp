#include "symtable.h"

const std::wstring SymTable::s_globalNamespace = std::wstring(L"");

SymTable::SymTable() : currentFunction(s_globalNamespace)
{

}

void SymTable::OpenScope(void)
{
	wprintf(L"WARNING: " __FUNCSIG__ " is deprecated.");
	depth++;
}

void SymTable::CloseScope(void)
{
	wprintf(L"WARNING: " __FUNCSIG__ " is deprecated.");
	depth--;
}

void SymTable::OpenFunction(const std::wstring& name)
{
	currentFunction += name;
}

void SymTable::CloseFunction(void)
{
	currentFunction = s_globalNamespace;
}

std::wstring SymTable::ComposeKey(const std::wstring& name)
{
	return currentFunction + L"." + name;
}

std::wstring SymTable::ComposeGlobalKey(const std::wstring& name)
{
	return s_globalNamespace + L"." + name;
}

SymTabEntry* SymTable::EnterSymbol(const std::wstring& name, const PrimitiveType type, const PrimitiveType pointeeType, ui32 size, const bool isFunction)
{
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
		entry.asVar.adress = -1;
		entry.asVar.pointeeType = pointeeType;
	}

	std::wstring composedKey = ComposeKey(name);

	table.insert(std::pair{ composedKey, entry });

	return RetrieveSymbol(composedKey);
}

SymTabEntry* SymTable::RetrieveSymbol(const std::wstring& composedKey)
{
	return table.contains(composedKey) ? &table.at(composedKey) : nullptr;
}
