#include "codegen.h"
#include "../AST/ASTNode.h"
#include "../AST/ASTAPI.h"
#include "../symbol_table/symtable.h"
#include "../Exit.h"
#include "../CStrLib.h"
#include "../Utils.h"
#include <cassert>

/*
	Name mangling in bonguscode is like how most C-compilers mangle their names.
	Firstly, we prepend an underscore.
	Secondly, and this is the more involved operation;
	In the case of unicode characters, we expand out the unicode character
	into a string of it's hex value.
*/
constexpr ui16 hexExpansionAmount = 4;

inline static void Mangle(wchar_t* outStr, const ui16 newStrLen)
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

inline static char* MangleFunctionName(const wchar_t* wstr)
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

	return narrowedStr;
}

namespace Registers
{
	enum class eRegisters : ui16
	{
		RAX,
		RBX,
		RCX,
		RDX,
		R8,
		R9,
		size
	};

	const std::string Regs[(ui64)eRegisters::size][4] = {
		{ "RAX", "EAX", "AX", "AL" },
		{ "RBX", "EBX", "BX", "BL" },
		{ "RCX", "ECX", "CX", "CL" },
		{ "RDX", "EDX", "DX", "DL" },
		{ "R8", "R8D", "R8W", "R8B" },
		{ "R9", "R9D", "R9W", "R9B" },
	};

	inline static const ui16 GetSubscriptFromType(const PrimitiveType type)
	{
		switch (type)
		{
		case PrimitiveType::ui64:
		case PrimitiveType::i64:
			return 0;

		case PrimitiveType::ui32:
		case PrimitiveType::i32:
			return 1;

		case PrimitiveType::ui16:
		case PrimitiveType::i16:
			return 2;

		case PrimitiveType::ui8:
		case PrimitiveType::i8:
			return 3;

		default:
			wprintf(L"ERROR: Type %hu supplied to " __FUNCSIG__ " does not correspond with any register size.\n", type);
			Exit(ErrCodes::internal_compiler_error);
			return -1;
		}
	}
}

using RG = Registers::eRegisters;
inline static const std::string& GetReg(RG reg, const PrimitiveType type)
{
	using namespace Registers;

	return Regs[(ui64)reg][GetSubscriptFromType(type)];
}

// Processes local variables by incrementing the total allocation size, aswell as entering
// stack location for this variable into the symbol table.
void ProcessLocalsCallback(AST::Node* n, void* args)
{
	i32* allocSize = (i32*)args;

	if (n->GetNodeKind() == Node_k::DeclNode)
	{
		AST::DeclNode* asDeclNode = (AST::DeclNode*)n;

		std::wstring key = g_symTable.ComposeKey(asDeclNode->GetName(), asDeclNode->GetScopeDepth());
		SymTabEntry* entry = g_symTable.RetrieveSymbol(key);
		entry->asVar.adress = *allocSize;

		*allocSize += asDeclNode->GetSize();
	}
}

namespace CurrentFunctionMetaData
{
	// How far into the stack local variables occupy.
	// After this point, the stack space left is used for temporaries
	// during expression evaluation.
	i32 varsStackSectionSize = 0;

	// This is the amount our temporaries consume.
	// varsStackSectionSize + temporariesStackSectionSize = total stack size
	i32 temporariesStackSectionSize = 0;

	static std::string funcName("NO_NAME_ASSIGNED");

	PrimitiveType retType;
}

// This uses pointers instead of modifying the above namespace's globals directly, so we can easily swap out where the metadata
// will be stored in the future.
inline static void ResetFunctionMetaData(i32* varsStackSectionSize, i32* temporariesStackSectionSize, std::string* funcName)
{
	*varsStackSectionSize = 0;
	*temporariesStackSectionSize = 0;
	*funcName = std::string("NO_NAME_ASSIGNED");
}

ui32 AllocLocals(AST::Node* funcHeadNode)
{
	// Gather up sizes.
	i32 allocSize = 0;
	AST::DoForAllChildren(funcHeadNode, &ProcessLocalsCallback, &allocSize);

	return allocSize;
}

// Temporary variables used by the compiler, like _t0, _t1.
struct TempVar
{
	std::string name;
	i32 adress;

	static const PrimitiveType s_tempVarDefaultType = AST::IntNode::s_defaultIntLiteralType;
};

// Allocation is done in two steps. First we get the allocated space with AllocStackSpace(), then we call CommitLastStackAlloc().
// The recordAllocs-parameter is where we store the stack space information
// (CurrentFunctionMetaData::temporariesStackSectionSize for instance).
inline static TempVar AllocStackSpace(i32* recordAllocs, bool resetHitCount = false)
{
	static i32 hitCount = 0; // <-- Yield _t0, _t1, _t2 ...

	if (resetHitCount)
	{
		hitCount = 0;
	}

	std::string retStr("_t" + std::to_string(hitCount++));

	return { retStr, *recordAllocs };
}

// This is where we commit the last allocation made with AllocStackSpace().
// We may allocate before we know the size, so the top-of-stack will be returned by AllocStackSpace(), meanwhile,
// when we finally know the size of our previous allocation, we call this function to commit the allocation by incrementing
// the stack variable.
inline static void CommitLastStackAlloc(i32* recordAllocs, i32 size)
{
	*recordAllocs += size;
}

inline static const ui16 GetSizeFromType(const PrimitiveType type)
{
	switch (type)
	{
	case PrimitiveType::ui64:
	case PrimitiveType::i64:
		return 8;
	case PrimitiveType::ui32:
	case PrimitiveType::i32:
		return 4;
	case PrimitiveType::ui16:
	case PrimitiveType::i16:
		return 2;
	default:
		wprintf(L"ERROR: Invalid type: %s.\n", PrimitiveTypeReflectionWide[(ui16)type]);
		Exit(ErrCodes::internal_compiler_error);
	}
};



namespace Prologue
{
	inline static void WriteFunctionNameProc(std::string& code, const std::string& functionName)
	{
		//code += "; TODO: Hard coded name, bad!\n" +
		//		CurrentFunctionMetaData::funcName + " PROC\n";

		code += functionName + " PROC\n";
	}
	
	inline static void SetupStackFrame(std::string& code)
	{
		code += "push rbp\n"\
			"mov rbp, rsp\n";
	}
	
	
	
	inline static void GenerateStackAllocation(std::string& code, const i32 allocSize)
	{
		code += "sub rsp, " + std::to_string(allocSize) + "\n";
	}
	
	inline static void GenerateFunctionPrologue(std::string& code, const i32 stackAllocSizeForLocals, const i32 stackAllocSizeForTemporaries, const std::string& functionName)
	{
		WriteFunctionNameProc(code, functionName);
	
		SetupStackFrame(code);
	
		code += "; Alloc section for local variables.\n";
		GenerateStackAllocation(code, stackAllocSizeForLocals);
		code += "; Alloc section for temporary variables.\n";
		GenerateStackAllocation(code, stackAllocSizeForTemporaries);
	}
}

namespace Epilogue
{
	inline static void WriteFunctionNameEndp(std::string& code, const std::string& functionName)
	{
		//code += "; TODO: Hard coded name, bad!\n" +
		//		CurrentFunctionMetaData::funcName + " ENDP\n";
		code += functionName + " ENDP\n";
	}

	inline static void RestoreStackFrame(std::string& code)
	{
		code += "mov rsp, rbp\n"\
				"pop rbp\n";
	}

	inline static void GenerateStackDeallocation(std::string& code, const i32 allocSize)
	{
		code += "add rsp, " + std::to_string(allocSize) + "\n";
	}

	inline static void GenerateFunctionEpilogue(std::string& code, const i32 stackAllocSizeForLocals, const i32 stackAllocSizeForTemporaries, const std::string& functionName)
	{
		code += "; Dealloc section for local variables.\n";
		GenerateStackDeallocation(code, stackAllocSizeForLocals);
		code += "; Dealloc section for temporary variables.\n";
		GenerateStackDeallocation(code, stackAllocSizeForTemporaries);

		RestoreStackFrame(code);

		code += "ret\n";

		WriteFunctionNameEndp(code, functionName);
	}
}

// Ugly hackaround for my architectural ineptitude. Because of how things are, a node like an opnode will get visited twice.
// Thus, we add nodes to a visited vector, and make sure to not visit them again.
// Maybe swap out for a hash-map in the future, if you decide to keep this monstrosity.
static std::vector<AST::Node*> visitedNodes;
bool FindNodeInVisitedNodes(AST::Node* node)
{
	for (ui32 i = 0; i < visitedNodes.size(); i++)
	{
		if (visitedNodes[i] == node)
		{
			return true;
		}
	}

	return false;
}

// TODO: Only necessary for debug printing, remove later.
#include <iostream>
namespace Body
{
	inline static std::string RefTempVar(const i32 offset, const PrimitiveType type)
	{
		// EXAMPLE:
		// mov eax, DWORD PTR 4[rsp]					// Go past the locals and into the temporaries section of the stack.

		std::string wordKind;

		switch (type)
		{
			case PrimitiveType::ui64:
			case PrimitiveType::i64:
			{
				wordKind = "QWORD PTR";
				break;
			}

			case PrimitiveType::ui32:
			case PrimitiveType::i32:
			{
				wordKind = "DWORD PTR";
				break;
			}

			case PrimitiveType::ui16:
			case PrimitiveType::i16:
			{
				wordKind = "WORD PTR";
				break;
			}

			default:
			{
				wprintf(L"ERROR: Type %hu supplied to " __FUNCSIG__ " is invalid.\n", type);
				Exit(ErrCodes::internal_compiler_error);
				break;
			}
		}

		return std::string(wordKind + " " + std::to_string(CurrentFunctionMetaData::varsStackSectionSize + offset) + "[rsp]");
	}
	inline static std::string RefLocalVar(const i32 offset, const PrimitiveType type)
	{
		std::string wordKind;

		switch (type)
		{
		case PrimitiveType::ui64:
		case PrimitiveType::i64:
		{
			wordKind = "QWORD PTR";
			break;
		}

		case PrimitiveType::ui32:
		case PrimitiveType::i32:
		{
			wordKind = "DWORD PTR";
			break;
		}

		case PrimitiveType::ui16:
		case PrimitiveType::i16:
		{
			wordKind = "WORD PTR";
			break;
		}

		default:
		{
			wprintf(L"ERROR: Type %hu supplied to " __FUNCSIG__ " is invalid.\n", type);
			Exit(ErrCodes::internal_compiler_error);
			break;
		}
		}

		return std::string(wordKind + " " + std::to_string(offset) + "[rsp]");
	}

	inline static void PushArgsIntoRegs(std::string& code, AST::FunctionCallNode* node);

	static void GenOpNodeCode(std::string& code, AST::Node* node, const TempVar& t0, const PrimitiveType exprType)
	{
		visitedNodes.push_back(node);

		// Get size from expression type.
		const ui16 exprSize = GetSizeFromType(exprType);
	
		switch (node->GetNodeKind())
		{
		case Node_k::OpNode:
		{
			AST::OpNode* asOpNode = (AST::OpNode*)node;
			std::string opString = asOpNode->GetOpAsString();
	

			GenOpNodeCode(code, asOpNode->GetLHS(), t0, exprType);

			TempVar t1 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize);
			GenOpNodeCode(code, asOpNode->GetRHS(), t1, exprType);
	
			const std::string& RAX = GetReg(RG::RAX, exprType);

			if (opString == "+")
			{
				std::string output = "\n; " + t0.name + " += " + t1.name +
									 "\nmov " + RAX + ", " + RefTempVar(t0.adress, exprType) +		// Store _tfirst in eax
									 "\nadd " + RAX + ", " + RefTempVar(t1.adress, exprType) +		// Perform operation in eax
									 "\nmov " + RefTempVar(t0.adress, exprType) + ", " + RAX + "\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
			if (opString == "-")
			{
				std::string output = "\n; " + t0.name + " -= " + t1.name +
									 "\nmov " + RAX + ", " + RefTempVar(t0.adress, exprType) +		// Store _tfirst in eax
									 "\nsub " + RAX + ", " + RefTempVar(t1.adress, exprType) +		// Perform operation in eax
									 "\nmov " + RefTempVar(t0.adress, exprType) + ", " + RAX + "\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
			else if (opString == "*")
			{
				std::string output = "\n; " + t0.name + " *= " + t1.name +
									 "\nmov " + RAX + ", " + RefTempVar(t0.adress, exprType) +		// Store _tfirst in eax
									 "\nimul " + RAX + ", " + RefTempVar(t1.adress, exprType) +		// Perform operation in eax
									 "\nmov " + RefTempVar(t0.adress, exprType) + ", " + RAX + "\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
			else if (opString == "/")
			{
				// TODO: Division is anal. Here's a guide:
				// https://www.youtube.com/watch?v=vwTYM0oSwjg
				// TLDR: For 64 bit division, the result goes in rax, the remainder in rdx
				// The divisor goes in rbx.
				const std::string& RBX = GetReg(RG::RBX, exprType);
				const std::string& RDX = GetReg(RG::RDX, exprType);

				std::string output = "\n; " + t0.name + " /= " + t1.name +
									 "\nmov " + RAX + ", " + RefTempVar(t0.adress, exprType) +		// Store _tfirst in eax
									 "\nmov " + RBX + ", " + RefTempVar(t1.adress, exprType) +		// Store divisor in rbx
									 "\nxor " + RDX + ", " + RDX +									// You have to make sure to 0 out rdx first, or else you get an integer underflow :P.
									 "\ndiv " + RBX +												// Perform operation in ebx
									 "\nmov " + RBX + ", 3405691582 ; 0xCAFEBABE" +					// Store sentinel value CAFEBABE in rbx in case of bugs.
									 "\nmov " + RDX + ", 4276993775 ; 0xFEEDBEEF" +					// Do the same for rdx with FEEDBEEF since it was also used.
									 "\nmov " + RefTempVar(t0.adress, exprType) + ", " + RAX + "\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
	
			break;
		}
		case Node_k::IntNode:
		{
			AST::IntNode* asIntNode = (AST::IntNode*)node;
	
			// Get size we need to allocate.
			//const ui16 sizeToAlloc = GetSizeFromType(exprType);
			//CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, sizeToAlloc);

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			const std::string& RAX = GetReg(RG::RAX, exprType);

			std::string output = "\n; " + t0.name + " = " + std::to_string(asIntNode->Get()) +
								 "\nmov " + RefTempVar(t0.adress, exprType) + ", " + std::to_string(asIntNode->Get()) +
								 "\nmov " + RAX + ", " + RefTempVar(t0.adress, exprType) + "\n"; // Also store result in rax.
			code += output;
			// std::cerr << output;
	
			break;
		}
		case Node_k::SymNode:
		{
			AST::SymNode* asSymNode = (AST::SymNode*)node;

			std::wstring key = g_symTable.ComposeKey(asSymNode->GetName(), 1); // TODO: Restructure symbol table.
			SymTabEntry* entry = g_symTable.RetrieveSymbol(key);

			if (entry == nullptr)
			{
				wprintf(L"ERROR: Couldn't find symtable entry for %s.\n", asSymNode->GetName().c_str());
				Exit(ErrCodes::undeclared_symbol);
			}

			// The temporary inherits it's size from the exprType-param.
			// If the local variable we're copying from is larger, truncation will happen here, and a warning will be issued.
			// If the local variable we're copying from is smaller, we'll include values outside of the variable which happen to be there on the stack, and this will be even worse.
			// TODO: The types might be the same size though, in which case this isn't a truncation. Split this out into two checks - One for signed-unsigned mismatch, the other for truncation.
			if (exprType != entry->asVar.type)
			{
				wprintf(L"WARNING: Truncation or signed-unsigned mismatch when trying to copy %s into temporary.\n"\
						L"Assignee has type %s, local var has type %s.\n\n",
						entry->name.c_str(), PrimitiveTypeReflectionWide[(ui16)exprType], PrimitiveTypeReflectionWide[(ui16)entry->asVar.type]
				);

				code += "\n; WARNING - Truncation or signed-unsigned mismatch";
			}

			// We must figure out whichever one is smaller. The temporary will still have exprType as it's type, but when we get the value
			// into rax we need to use the appropriate size (QWORD PTR/DWORD PTR ... and RAX/EAX/AX ...)
			const PrimitiveType smallestType = exprSize < entry->asVar.size ? exprType : entry->asVar.type;

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			// One version of the register is for when we get the local variable into rax,
			// and the other is for when we export the value from rax into the temporary.
			std::string readReg(GetReg(RG::RAX, smallestType));
			const std::string& writeReg = GetReg(RG::RAX, exprType);


			// If the local variable we're moving into rax is 2 bytes in size(ui16/i16), we need to zero extend it.
			// If this is the case, readReg must also be changed from AX to RAX, so this ugly hackaround does that to.
			// TODO: Refactor this.
			std::string movToRaxOp("mov ");
			if (entry->asVar.type == PrimitiveType::ui16 || entry->asVar.type == PrimitiveType::i16)
			{
				movToRaxOp = "movzx ";
				readReg = Registers::Regs[(ui16)RG::RAX][0]; // Yields RAX.
			}

			std::string output = "\n; " + t0.name + " = (local var at stackLoc " + std::to_string(entry->asVar.adress) + ")\n" +
								 movToRaxOp + readReg + ", " + RefLocalVar(entry->asVar.adress, smallestType) +
								 "\nmov " + RefTempVar(t0.adress, exprType) + ", " + writeReg + "\n";

			code += output;
			// std::cerr << output;

			break;
		}
		case Node_k::FunctionCallNode:
		{
			AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)node;

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			// We've already made sure in the harvest pass that this is indeed a function, and in the semantics pass that this function can be called.
			std::string output;

			PrimitiveType funcRetType = g_symTable.RetrieveSymbol(g_symTable.ComposeKey(asFunctionCallNode->GetName(), SymTable::s_globalNamespace))->asFunction.retType;
			PushArgsIntoRegs(output, asFunctionCallNode);

			// Get the mangled function name!
			std::string mangledFunctionName = std::string(MangleFunctionName(asFunctionCallNode->GetName().c_str()));

			// Make sure to also store the result out into _t0.
			output += "; " + t0.name + " = result of function " + mangledFunctionName + "\n" +
					  "call " + mangledFunctionName + "\n" +
					  "mov " + RefTempVar(t0.adress, exprType) + ", " + GetReg(RG::RAX, exprType) + "\n";

			code += output;

			break;
		}
		}
	}

	inline static void PushArgsIntoRegs(std::string& code, AST::FunctionCallNode* node)
	{
		// Returns the default int type for int literal nodes, the var type of symnodes' symtable entries and
		// function return type of function call nodes. That should be it for stuff that can appear in expressions.
		const auto GetTypeFromNode = [](AST::Node* n) -> const PrimitiveType {
			switch (n->GetNodeKind())
			{
			case Node_k::IntNode:
			{
				return AST::IntNode::s_defaultIntLiteralType;
				break;
			}
			case Node_k::SymNode:
			{
				AST::SymNode* asSymNode = (AST::SymNode*)n;
				SymTabEntry* entry = g_symTable.RetrieveSymbol(g_symTable.ComposeKey(asSymNode->GetName(), 1));
				return entry->asVar.type;
				break;
			}
			case Node_k::FunctionCallNode:
			{
				AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)n;
				SymTabEntry* entry = g_symTable.RetrieveSymbol(g_symTable.ComposeKey(asFunctionCallNode->GetName(), 1));
				return entry->asFunction.retType;
				break;
			}
			default:
			{
				wprintf(L"ERROR: No type deducible from node n in " __FUNCSIG__ "\n");
				Exit(ErrCodes::unknown_type);
				break;
			}
			}
		};

		static const RG callingConvention[] = {
			RG::RCX,
			RG::RDX,
			RG::R8,
			RG::R9
		};
		// Keeps track of how far we've gotten into the calling-convention registers/stack.
		relptr_t nextSlot = 0;

		for (AST::Node* arg = node->GetArgs(); arg != nullptr; arg = arg->GetRightSibling())
		{
			const PrimitiveType exprType = GetTypeFromNode(arg);

			// We need to generate the code for the values we're pushing before we push them.
			TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize);
			GenOpNodeCode(code, arg, t0, exprType);

			const std::string& reg = GetReg(callingConvention[nextSlot], exprType);
			code += "; Push " + t0.name + " into " + reg + "\n"
					"mov " + reg + ", " + RefTempVar(t0.adress, exprType) + "\n\n";


			// TODO: In the future we might want to support more than 4 arguments.
			if (!(nextSlot < GetArraySize(callingConvention)))
			{
				wprintf(L"WARNING: Ran out of registers while trying to call function %s.\n", node->GetName().c_str());
				break;
			}
			nextSlot++;
		}
	}

	// Retrieves args(if any) by pushing the registers according to the calling convention out to the arg variables.
	inline static void RetrieveArgs(std::string& code, AST::FunctionNode* functionNode)
	{
		static const RG callingConvention[] = {
			RG::RCX,
			RG::RDX,
			RG::R8,
			RG::R9
		};

		relptr_t nextSlot = 0;

		for (AST::Node* node = functionNode->GetArgsList(); node != nullptr; node = node->GetRightSibling())
		{
			AST::ArgNode* asArgNode = (AST::ArgNode*)node;

			std::wstring composedKey = g_symTable.ComposeKey(asArgNode->GetName(), 1);
			SymTabEntry* entry = g_symTable.RetrieveSymbol(composedKey);

			std::string readReg(GetReg(callingConvention[nextSlot], entry->asVar.type));
			std::string movToRaxOp("mov ");

			code += movToRaxOp + RefLocalVar(entry->asVar.adress, entry->asVar.type) + ", " + readReg + "\n";


			// TODO: In the future we might want to support more than 4 arguments.
			if (!(nextSlot < GetArraySize(callingConvention)))
			{
				wprintf(L"WARNING: Ran out of registers while trying to retrieve args for function %s.\n", functionNode->GetName().c_str());
				break;
			}
			nextSlot++;
		}
	}
	
	void GenerateFunctionBody(std::string& code, AST::Node* node, i32* const largestTempAllocation)
	{
		auto gatherLargestAllocation = [](i32* const out, i32 newAllocSize) -> void {
			if (newAllocSize > *out)
			{
				*out = newAllocSize;
			}
		};

		visitedNodes.push_back(node);
	
		switch (node->GetNodeKind())
		{
			case Node_k::OpNode:
			{
				// This is just a lone op node without assignment, but we'll perform the evaluation.
				// Because of this, we're supplying the default type.
				TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, true);
				GenOpNodeCode(code, node, t0, TempVar::s_tempVarDefaultType);
				
				// Check to see if the allocation done by the expression evaluation of GenOpNodeCode() requires more memory than the last evaluation.
				gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);
				// Enforce allocation policy.
				CurrentFunctionMetaData::temporariesStackSectionSize = 0;




				break;
			}
			case Node_k::AssNode:
			{
				AST::AssNode* asAssNode = (AST::AssNode*)node;

				// Get target of assignment
				i32 stackLocation = -1;
				PrimitiveType exprType = PrimitiveType::invalid;

				if (asAssNode->GetVar()->GetNodeKind() == Node_k::SymNode)
				{
					AST::SymNode* var = (AST::SymNode*)asAssNode->GetVar();

					std::wstring key = g_symTable.ComposeKey(var->GetName(), 1); // TODO: Restructure symbol table.
					SymTabEntry* entry = g_symTable.RetrieveSymbol(key);

					if (entry == nullptr)
					{
						wprintf(L"ERROR: Couldn't find symtable entry for %s.\n", var->GetName().c_str());
						Exit(ErrCodes::undeclared_symbol);
					}


					stackLocation = entry->asVar.adress;
					exprType = entry->asVar.type;
				}

				// Generate operation code. Remember that the temporaries stack section needs to be reset!
				TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, true);
				// The type and size all temporaries involved in this expression will inherit depends on the type we're assigning to, effectively
				// handling truncation immediately.
				GenOpNodeCode(code, asAssNode->GetExpr(), t0, exprType);

				// Check to see if the allocation done by the expression evaluation of GenOpNodeCode() requires more memory than the last evaluation.
				gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);
				// Enforce allocation policy(doing the aforementioned resetting).
				CurrentFunctionMetaData::temporariesStackSectionSize = 0;




				// Store result (held in rax and on the temporaries-stack) in var.
				// TODO: Maybe integrate function name mangling for variable names aswell, so we can write out the variable name instead of stack location.
				std::string output = "\n; (local var at stackLoc " + std::to_string(stackLocation) + ") = Result of expr(rax)" +
									 "\nmov " + RefLocalVar(stackLocation, exprType) + ", " + GetReg(RG::RAX, exprType) + "\n";

				code += output;
				// std::cerr << output;

				break;
			}
			case Node_k::ReturnNode:
			{
				AST::ReturnNode* asReturnNode = (AST::ReturnNode*)node;

				// The result of the expression we're about to evaluate is stored in rax by default,
				// so we don't need to do anything more than ensure that the expression's code is generated.

				const PrimitiveType retType = CurrentFunctionMetaData::retType;
				code += "\n; Return expression(ret_t: " + std::string(PrimitiveTypeReflectionNarrow[(ui16)retType]) + "):\n";

				// Generate operation code. Remember that the temporaries stack section needs to be reset!
				TempVar t0(AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, true));
				// TODO: The type supplied here from outside(s_defaultType/i32) will be too small to hold values gathered from ui64s/i64s.
				// In the future, fix this by properly implementing functions, so we may stick it's return type here instead.
				GenOpNodeCode(code, asReturnNode->GetRetExpr(), t0, retType);

				// Check to see if the allocation done by the expression evaluation of GenOpNodeCode() requires more memory than the last evaluation.
				gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);

				// Add some padding.
				code += "\n\n";

				// TODO: We're no longer generating the epilogue when we return. Instead, if we find a return statement(in an if-statement), stick the result in RAX and GOTO(!)
				// the epilogue of the function, skipping all the other code in between.
				// On the other hand, if we find a lone return statement that obscures code after it, we just raise an error.
				// CurrentFunctionMetaData::temporariesStackSectionSize = *largestTempAllocation;
				// Epilogue::GenerateFunctionEpilogue(code, CurrentFunctionMetaData::varsStackSectionSize, CurrentFunctionMetaData::temporariesStackSectionSize);

				break;
			}
			case Node_k::FunctionCallNode:
			{
				AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)node;
				PrimitiveType funcRetType = g_symTable.RetrieveSymbol(g_symTable.ComposeKey(asFunctionCallNode->GetName(), SymTable::s_globalNamespace))->asFunction.retType;
				PushArgsIntoRegs(code, asFunctionCallNode);
				code += "call " + std::string(std::string(MangleFunctionName(asFunctionCallNode->GetName().c_str()))) + "\n";
			}
		}
	
		for (AST::Node* child : node->GetChildren())
		{
			// If the child node has been visited already, skip it.
			if (FindNodeInVisitedNodes(child))
			{
				continue;
			}
			GenerateFunctionBody(code, child, largestTempAllocation);
		}
	}
}

namespace Boilerplate
{
	// TODO: Generation of the data and code sections should probably be handled differently, and maybe move these out from this namespace.
	inline static void GenerateDataSection(std::string& code)
	{
		code += ".data\n\n\n";
	}

	inline static void GenerateCodeSection(std::string& code)
	{
		code += ".code\n";
	}

	inline static void GenerateHeader(std::string& code)
	{
		//code += ".686P        ; Enable x86 - 64 (64 - bit mode)\n" \
				".model flat  ; Flat memory model\n";
		code += "OPTION DOTNAME   ; Allows the use of dot notation(MASM64 requires this for 64 - bit assembly)\n";

		GenerateDataSection(code);
		GenerateCodeSection(code);
	}


	inline static void GenerateFooter(std::string& code)
	{
		// "END directive required at end of file" - MASM.
		code += "END";
	}
}

void GenerateCode(AST::Node* nodeHead, std::string& outCode)
{
	// Start by generating the boilerplate skeleton once, then generate function code for each function node, then finish by generating the boilerplate
	// at the bottom of the ASM file.
	// Note narrowing to narrow string from wide string.
	std::string boilerplateHeader, boilerplateFooter;

	Boilerplate::GenerateHeader(boilerplateHeader);
	outCode = boilerplateHeader;
	//NOTE /\ is assignment, not += !!!!!

	for (AST::Node* childNode : nodeHead->GetChildren())
	{
		if (childNode->GetNodeKind() != Node_k::FunctionNode)
		{
			// childNode is a forward declaration node, skip it.
			continue;
		}
		AST::FunctionNode* asFunctionNode = (AST::FunctionNode*)childNode;

		std::string prologue, body, epilogue;
		
		// Because we use the stack for temporaries, we need to figure out how much stack space to reserve in the body,
		// and cannot go on amount of declnodes alone in the prologue function.
		// Therefore, we defer the composing of the complete code until we know the total amount of stack space to reserve.

		// Firstly, figure out the amount of stack space required by local variables, and allocate them.
		// Results for variables is stored in the symbol table.
		CurrentFunctionMetaData::varsStackSectionSize = AllocLocals(childNode);

		// Special case for the main function, because you can't define the entrypoint to be whatever with the Microsoft linker.
		std::string mangledFuncName;
		if (asFunctionNode->GetName() == std::wstring(WideMainFunctionName))
		{
			// Stick the main function name (in BCL) in there as a comment.
			mangledFuncName = "; main (In BCL: " + std::string(NarrowMainFunctionName) + ")\nmain";
		}
		else
		{
			char* nameCStr = MangleFunctionName(asFunctionNode->GetName().c_str());
			mangledFuncName = std::string(nameCStr);
			free(nameCStr);
		}
		CurrentFunctionMetaData::funcName = mangledFuncName;
		CurrentFunctionMetaData::retType = asFunctionNode->GetRetType();

		// We need to get the biggest size the stack will ever grow to so we can enforce our allocation policy.
		// Without this we'd allocate more and more stack size for each expression evaluation, even though temporaries should start back at 0 when evaluating a new expression.
		i32 largestTemporariesAlloc = 0;

		body = "\n\n\n; Body\n";
		Body::RetrieveArgs(body, asFunctionNode);
		Body::GenerateFunctionBody(body, childNode, &largestTemporariesAlloc);

		CurrentFunctionMetaData::temporariesStackSectionSize = largestTemporariesAlloc;

		prologue = "\n\n\n; Prologue\n";
		Prologue::GenerateFunctionPrologue(
			prologue,
			CurrentFunctionMetaData::varsStackSectionSize,
			CurrentFunctionMetaData::temporariesStackSectionSize,
			CurrentFunctionMetaData::funcName
		);

		epilogue = "\n\n\n; Epilogue\n";
		Epilogue::GenerateFunctionEpilogue(
			epilogue,
			CurrentFunctionMetaData::varsStackSectionSize,
			CurrentFunctionMetaData::temporariesStackSectionSize,
			CurrentFunctionMetaData::funcName
		);
	

		outCode += prologue + body + epilogue;

		ResetFunctionMetaData(
			&CurrentFunctionMetaData::varsStackSectionSize,
			&CurrentFunctionMetaData::temporariesStackSectionSize,
			&CurrentFunctionMetaData::funcName
		);
	}

	Boilerplate::GenerateFooter(boilerplateFooter);
	outCode += boilerplateFooter;
}
