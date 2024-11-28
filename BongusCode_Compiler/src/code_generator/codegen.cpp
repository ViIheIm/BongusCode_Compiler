#include "codegen.h"
#include "../AST/ASTNode.h"
#include "../AST/ASTAPI.h"
#include "../symbol_table/symtable.h"
#include "../Exit.h"
#include "../CStrLib.h"
#include "../Utils.h"
#include <cassert>
#include <iostream>

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
		case PrimitiveType::pointer:
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

		//std::wstring key = g_symTable.ComposeKey(asDeclNode->GetName(), asDeclNode->GetScopeDepth());
		//SymTabEntry* entry = g_symTable.RetrieveSymbol(key);
		SymTabEntry* entry = asDeclNode->GetSymTabEntry();
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

	AST::FunctionNode* currentFunction = nullptr;
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
	case PrimitiveType::pointer:
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

namespace Tools
{
	inline static std::string GetWordKindFromType(const PrimitiveType type)
	{
		switch (type)
		{
		case PrimitiveType::ui64:
		case PrimitiveType::i64:
		case PrimitiveType::pointer:
		{
			return std::string("QWORD PTR");
			break;
		}

		case PrimitiveType::ui32:
		case PrimitiveType::i32:
		{
			return std::string("DWORD PTR");
		}

		case PrimitiveType::ui16:
		case PrimitiveType::i16:
		{
			return std::string("WORD PTR");
		}

		default:
		{
			wprintf(L"ERROR: Type %hu supplied to " __FUNCSIG__ " is invalid.\n", type);
			Exit(ErrCodes::internal_compiler_error);
		}
		}
	}

	inline static i32 GetAdressOfTemporary(const TempVar& t)
	{
		return CurrentFunctionMetaData::varsStackSectionSize + t.adress;
	}

	inline static std::string RefTempVar(const i32 offset, const PrimitiveType type)
	{
		// EXAMPLE:
		// mov eax, DWORD PTR 4[rsp]					// Go past the locals and into the temporaries section of the stack.

		std::string wordKind = GetWordKindFromType(type);
		return std::string(wordKind + " " + std::to_string(CurrentFunctionMetaData::varsStackSectionSize + offset) + "[rsp]");
	}
	inline static std::string RefLocalVar(const i32 offset, const PrimitiveType type)
	{
		std::string wordKind = GetWordKindFromType(type);
		return std::string(wordKind + " " + std::to_string(offset) + "[rsp]");
	}

	// string1 -> mov variant
	// string2 -> RAX variant
	// string3 -> size variant (WORD PTR / DWORD PTR / QWORD PTR)
	std::tuple<std::string, std::string, std::string> GetFetchInstructionsForType(const RG reg, const PrimitiveType type)
	{
		std::string movVariant("mov");
		std::string regVariant = GetReg(reg, type);
		std::string sizeVariant = GetWordKindFromType(type);

		if (type == PrimitiveType::i16 || type == PrimitiveType::ui16)
		{
			movVariant = "movzx ";
			regVariant = Registers::Regs[(ui16)RG::RAX][0]; // Yields RAX.
		}

		return std::tuple(movVariant, regVariant, sizeVariant);
	}

	std::string FetchIntoReg(const RG reg, const i32 sourceAdress, const PrimitiveType sourceType)
	{
		const auto [movVariant, regVariant, sizeVariant] = GetFetchInstructionsForType(reg, sourceType);

		std::string result = movVariant + " " + regVariant + ", " + sizeVariant + " " + std::to_string(sourceAdress) + "[rsp]";

		return result;
	}

	std::string FetchImmediateIntoReg(const RG reg, const std::string& immediate)
	{
		const auto [movVariant, regVariant, sizeVariant] = GetFetchInstructionsForType(reg, AST::IntNode::s_defaultIntLiteralType);

		std::string result = movVariant + " " + regVariant + ", " + immediate;

		return result;
	}

	std::string FetchImmediateIntoMem(const i32 destAdress, const PrimitiveType destType, const std::string& immediate)
	{
		const auto [movVariant, regVariant, sizeVariant] = GetFetchInstructionsForType(RG::RAX, destType);

		std::string result = movVariant + " " + sizeVariant + " " + std::to_string(destAdress) + "[rsp], " + immediate;

		return result;
	}

	std::string OperateOnReg(const RG reg, const std::string& op, const i32 operandAdress, const PrimitiveType operandType)
	{
		const auto [movVariant, regVariant, sizeVariant] = GetFetchInstructionsForType(reg, operandType);

		std::string result = op + " " + regVariant + ", " + sizeVariant + " " + std::to_string(operandAdress) + "[rsp]";

		return result;
	}

	std::string PushRegIntoMem(const RG reg, const i32 destAdress, const PrimitiveType destType)
	{
		const auto [movVariant, regVariant, sizeVariant] = GetFetchInstructionsForType(reg, destType);

		std::string result = movVariant + " " + sizeVariant + " " + std::to_string(destAdress) + "[rsp]" + ", " + regVariant;

		return result;
	}
}
using namespace Tools;

namespace Body
{
	inline static void PushArgsIntoRegs(std::string& code, AST::FunctionCallNode* node);
	
	// Returns a tuple consisting of <read register, write register, mov type>
	// Could be e.g. <EAX, EAX, movzx>.
	inline static std::tuple<std::string, std::string, std::string> GetTypeDependentInstructions(
		const RG readReg,
		const RG writeReg,
		const PrimitiveType readRegType,
		const PrimitiveType writeRegType,
		const PrimitiveType localVarType
	)
	{
		std::string readRegStr(GetReg(readReg, readRegType));
		std::string writeRegStr(GetReg(writeReg, writeRegType));
		std::string movToRaxOpStr("mov ");

		// If the local var we're moving into rax's type is 2 bytes in size then we need to zero extend it.
		if (localVarType == PrimitiveType::ui16 || localVarType == PrimitiveType::i16)
		{
			movToRaxOpStr = "movzx ";
			readRegStr = Registers::Regs[(ui16)RG::RAX][0]; // Yields RAX.
		}
		
		return std::tuple(readRegStr, writeRegStr, movToRaxOpStr);
	}

	inline static const PrimitiveType GetPointeeTypeFromDerefNode(AST::DerefNode* derefNode)
	{
		/*
				Hunt through the subexpr until we find a pointer node.
				Most sane dereference operations evolve from a single pointer, e.g.
					�(pointer + 1) = 200
				and not typically
					�(pointer + **another pointer**) = 200

				in fact(just checked this in compiler explorer), adding a pointer to another pointer in C++ raises an error,
				so we can safely look for only 1 pointer in the subexpression, and take it's pointeeType.

				There's a safety check in the semantics pass which checks for more than 1 pointer in a subexpression,
				which raises an error if several are found, so we can happily pick the first pointee type here and call it a day.
			*/

		PrimitiveType pointeeType = PrimitiveType::invalid;

		for (AST::Node* symNode : AST::GetAllChildNodesOfType(derefNode, Node_k::SymNode))
		{
			AST::SymNode* asSymNode = (AST::SymNode*)symNode;
			SymTabEntry* entry = asSymNode->GetSymTabEntry();

			if (entry->asVar.type == PrimitiveType::pointer)
			{
				// Find first pointer, get it's pointee-type and break.
				pointeeType = entry->asVar.pointeeType;
				break;
			}
		}

		if (pointeeType == PrimitiveType::invalid)
		{
			wprintf(L"ERROR: couldn't find pointee type in " __FUNCDNAME__ "\n");
			Exit(ErrCodes::internal_compiler_error);
		}

		return pointeeType;
	}

	inline static const std::string GenDerefCode(const PrimitiveType pointeeType)
	{
			std::string readReg(GetReg(RG::RAX, pointeeType));
			std::string movToRaxOp("mov ");
			std::string wordKind = GetWordKindFromType(pointeeType);
			if (pointeeType == PrimitiveType::ui16 || pointeeType == PrimitiveType::i16)
			{
				movToRaxOp = "movzx ";
				readReg = Registers::Regs[(ui16)RG::RAX][0]; // Yields RAX.
			}
			
			// By this point, the entire expression should be generated and held in rax.
			// Dereference rax and store it out in _t0.
			std::string output = "\n" + movToRaxOp + readReg + ", " + wordKind + "[RAX]\n";

			return output;
	}


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

			const i32 t0ActualAdress = GetAdressOfTemporary(t0);
			const i32 t1ActualAdress = GetAdressOfTemporary(t1);

			if (opString == "+")
			{
				std::string output = "\n; " + t0.name + " += " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, exprType) + "\n" +
														 OperateOnReg(RG::RAX, "add", t1ActualAdress, exprType) + "\n" +
														 PushRegIntoMem(RG::RAX, t0ActualAdress, exprType);
				code += output;
			}
			if (opString == "-")
			{
				std::string output = "\n; " + t0.name + " -= " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, exprType) + "\n" +
														 OperateOnReg(RG::RAX, "sub", t1ActualAdress, exprType) + "\n" +
														 PushRegIntoMem(RG::RAX, t0ActualAdress, exprType);

				code += output;
			}
			else if (opString == "*")
			{
				std::string output = "\n; " + t0.name + " *= " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, exprType) + "\n" +
														 OperateOnReg(RG::RAX, "imul", t1ActualAdress, exprType) + "\n" +
														 PushRegIntoMem(RG::RAX, t0ActualAdress, exprType);

				code += output;
			}
			else if (opString == "/")
			{
				// TODO: Division is anal. Here's a guide:
				// https://www.youtube.com/watch?v=vwTYM0oSwjg
				// TLDR: For 64 bit division, the result goes in rax, the remainder in rdx
				// The divisor goes in rbx.
				const std::string& RBX = GetReg(RG::RBX, exprType);
				const std::string& RDX = GetReg(RG::RDX, exprType);

				std::string output = "\n; " + t0.name + " /= " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, exprType) + "\n" +								// Store _tfirst in eax
														 FetchIntoReg(RG::RBX, t1ActualAdress, exprType) + "\n" +								// Store divisor in rbx
														 "xor " + RDX + ", " + RDX + "\n" +																			// You have to make sure to 0 out rdx first, or else you get an integer underflow :P.
														 "div " + RBX + "\n" +																									// Perform operation in ebx
														 FetchImmediateIntoReg(RG::RBX, "3405691582 ; 0xCAFEBABE") + "\n" +			// Store sentinel value CAFEBABE in rbx in case of bugs.
														 FetchImmediateIntoReg(RG::RDX, "4276993775 ; 0xFEEDBEEF") + "\n" +			// Do the same for rdx with FEEDBEEF since it was also used.
														 PushRegIntoMem(RG::RAX, t0ActualAdress, exprType);											// Store result in _tfirst on stack
				code += output;
			}
	
			break;
		}
		case Node_k::IntNode:
		{
			AST::IntNode* asIntNode = (AST::IntNode*)node;

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			std::string intValueAsString = std::to_string(asIntNode->Get());
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);

			std::string output = "\n; " + t0.name + " = " + intValueAsString + "\n" +
													 FetchImmediateIntoMem(t0ActualAdress, exprType, intValueAsString) + "\n" +
													 FetchIntoReg(RG::RAX, t0ActualAdress, exprType);

			code += output;
	
			break;
		}
		case Node_k::SymNode:
		{
			AST::SymNode* asSymNode = (AST::SymNode*)node;

			SymTabEntry* entry = asSymNode->GetSymTabEntry();

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

			const PrimitiveType smallestType = exprSize < entry->asVar.size ? exprType : entry->asVar.type;

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			const i32 t0ActualAdress = GetAdressOfTemporary(t0);

			std::string output = "\n; " + t0.name + " = (local var at stackLoc " + std::to_string(entry->asVar.adress) + ")\n" +
													 FetchIntoReg(RG::RAX, entry->asVar.adress, smallestType) + "\n" +
													 PushRegIntoMem(RG::RAX, t0ActualAdress, exprType);

			code += output;

			break;
		}
		case Node_k::FunctionCallNode:
		{
			AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)node;

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			// We've already made sure in the harvest pass that this is indeed a function, and in the semantics pass that this function can be called.
			std::string output;

			//PrimitiveType funcRetType = g_symTable.RetrieveSymbol(g_symTable.ComposeKey(asFunctionCallNode->GetName(), SymTable::s_globalNamespace))->asFunction.retType;
			PrimitiveType funcRetType = asFunctionCallNode->GetSymTabEntry()->asFunction.retType;
			PushArgsIntoRegs(output, asFunctionCallNode);

			// Get the mangled function name!
			std::string mangledFunctionName = std::string(MangleFunctionName(asFunctionCallNode->GetName().c_str()));
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);

			// Make sure to also store the result out into _t0.
			output += "\n; " + t0.name + " = result of function " + mangledFunctionName + "\n" +
								"call " + mangledFunctionName + "\n" +
								PushRegIntoMem(RG::RAX, t0ActualAdress, exprType);

			code += output;

			break;
		}
		case Node_k::AddrOfNode:
		{
			AST::AddrOfNode* asAddrOfNode = (AST::AddrOfNode*)node;
			SymTabEntry* entry = asAddrOfNode->GetSymTabEntry();
			const PrimitiveType addrOfNodeExprType = PrimitiveType::pointer;

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			std::string readReg(GetReg(RG::RAX, addrOfNodeExprType));
			const std::string& writeReg = GetReg(RG::RAX, exprType);

			std::string output = "\n; " + t0.name + " = (addr of local var at stackLoc " + std::to_string(entry->asVar.adress) + ")\n" +
				"lea " + readReg + ", " + RefLocalVar(entry->asVar.adress, addrOfNodeExprType) +
				"\nmov " + RefTempVar(t0.adress, addrOfNodeExprType) + ", " + writeReg + "\n";

			code += output;

			break;
		}
		case Node_k::DerefNode:
		{
			AST::DerefNode* asDerefNode = (AST::DerefNode*)node;
			const PrimitiveType pointerType = PrimitiveType::pointer;
			
			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, exprSize);

			PrimitiveType pointeeType = GetPointeeTypeFromDerefNode(asDerefNode);

			TempVar t1 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize);
			GenOpNodeCode(code, asDerefNode->GetExpr(), t1, pointerType);

			/*
				mov     rax, QWORD PTR b$[rsp]
				movzx   eax, WORD PTR[rax]
			*/

			//std::string readReg(GetReg(RG::RAX, pointeeType));
			//std::string movToRaxOp("mov ");
			//std::string wordKind = GetWordKindFromType(pointeeType);
			//if (pointeeType == PrimitiveType::ui16 || pointeeType == PrimitiveType::i16)
			//{
			//	movToRaxOp = "movzx ";
			//	readReg = Registers::Regs[(ui16)RG::RAX][0]; // Yields RAX.
			//}
			//
			//// By this point, the entire expression should be generated and held in rax.
			//// Dereference rax and store it out in _t0.
			//std::string output = "\n; _t0 = Deref operation of previous expr(rax).\n" +
			//	movToRaxOp + readReg + ", " + wordKind + "[RAX]\n";
			std::string output = GenDerefCode(pointeeType);

			output += "; Move out to " + t1.name +
				"\nmov " + RefTempVar(t1.adress, exprType) + ", " + GetReg(RG::RAX, exprType) + "\n";

			code += output;

			break;
		}
		}
	}

	// This function generates code to store a value into a memory address either through reading a variable
	// or by supplying an immediate value, depending on if the node given is a symNode or an intNode.
	inline static void GenAssignmentToStackMem(std::string& code, AST::Node* valueNode, const ui32 stackAddress, const PrimitiveType assigneeType)
	{
		std::string assignmentString = GetWordKindFromType(assigneeType) + " " + std::to_string(stackAddress) + "[rsp]";

		switch (valueNode->GetNodeKind())
		{
		case Node_k::IntNode:
		{
			AST::IntNode* asIntNode = (AST::IntNode*)valueNode;

			const std::string& toFromReg = GetReg(RG::RAX, assigneeType);

			std::string output = "\nmov " + toFromReg + ", " + std::to_string(asIntNode->Get()) +
							"\nmov " + assignmentString + ", " + toFromReg + "\n";

			code += output;

			break;
		}
		case Node_k::SymNode:
		{
			AST::SymNode* asSymNode = (AST::SymNode*)valueNode;
			SymTabEntry* entry = asSymNode->GetSymTabEntry();
			const PrimitiveType symType = entry->asVar.type;

			const auto [readReg, writeReg, movToRaxOp] = GetTypeDependentInstructions(
				RG::RAX,
				RG::RAX,
				symType,
				assigneeType,
				symType
			);

			std::string output = movToRaxOp + readReg + ", " + RefLocalVar(entry->asVar.adress, symType) +
													 "\nmov " + assignmentString + ", " + writeReg + "\n";

			code += output;

			break;
		}
		}
	}

	inline static void GenForLoopHeadComparison(std::string& code, AST::Node* upperBound, const std::string& labelToJumpTo, const ui32 iterVarAddress, const PrimitiveType iterVarType)
	{
		switch (upperBound->GetNodeKind())
		{
		case Node_k::IntNode:
		{
			// mov eax, 5
			AST::IntNode* asIntNode = (AST::IntNode*)upperBound;

			const std::string& toReg = GetReg(RG::RAX, AST::IntNode::s_defaultIntLiteralType);

			std::string output = "\nmov " + toReg + ", " + std::to_string(asIntNode->Get()) + "\n";
			code += output;

			break;
		}
		case Node_k::SymNode:
		{
			// mov eax, DWORD PTR upperBound[rsp]
			AST::SymNode* asSymNode = (AST::SymNode*)upperBound;
			SymTabEntry* entry = asSymNode->GetSymTabEntry();
			const PrimitiveType symType = entry->asVar.type;

			const std::string bringInSymString = GetWordKindFromType(entry->asVar.type) + " " + std::to_string(entry->asVar.adress) + "[rsp]";
			const auto [readReg, writeReg, movToRaxOp] = GetTypeDependentInstructions(
				RG::RAX,
				RG::RAX,
				symType,
				symType,
				symType
			);

			std::string output = movToRaxOp + readReg + ", " + bringInSymString + "\n";
			code += output;

			break;
		}
		}
		
		// Now it's time to compare with the iter variable and jump if greater than or equal to.
		const std::string iterVarString = GetWordKindFromType(iterVarType) + " " + std::to_string(iterVarAddress) + "[rsp]";
		code += "cmp " + iterVarString + ", " + GetReg(RG::RAX, iterVarType) +
						"\njge SHORT " + labelToJumpTo + "\n";
	}

	inline static void GenForLoopHeadCode(
		std::string& code,
		AST::ForLoopHeadNode* node,
		TempVar& iterVar,
		const PrimitiveType iterVarType,
		const std::string& headLabel,
		const std::string& bodyLabel,
		const std::string& exitLabel
	)
	{
		const ui32 actualAddress = CurrentFunctionMetaData::varsStackSectionSize + iterVar.adress;
		GenAssignmentToStackMem(code, node->GetLowerBound(), actualAddress, iterVarType);
	
		// Now we must generate the jump instruction.
		code += "jmp SHORT " + bodyLabel + "\n";

		// And then for the actual head, where we increment the iter variable.

		const std::string iterVarAssignmentString = GetWordKindFromType(iterVarType) + " " + std::to_string(actualAddress) + "[rsp]";

		const std::string& toFromReg = GetReg(RG::RAX, iterVarType);

		code += headLabel + ":\n" +
						"\nmov " + toFromReg + ", " + iterVarAssignmentString +
						"\ninc " + toFromReg +
						"\nmov " + iterVarAssignmentString + ", " + toFromReg + "\n";


		// Now we can generate code for the comparison between iterVar and the upper bound.
		GenForLoopHeadComparison(code, node->GetUpperBound(), exitLabel, actualAddress, iterVarType);
	}

	void GenerateFunctionBody(std::string& code, AST::Node* node, i32* const largestTempAllocation);

	inline static void GenForLoopCode(std::string& code, AST::ForLoopNode* node, i32* const largestTempAllocation)
	{
		// The iter var(typically i in C/C++ for loops) will be maintained as a temporary variable.
		TempVar iterVar = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize);
		const PrimitiveType iterVarType = PrimitiveType::ui64;
		//CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, GetSizeFromType(iterVarType));
		
		// This variable will not take functions into account, so if it encounters 2 loops in function Foo,
		// and then a loop in main, the main loop will not start over numbered as 0.
		static ui32 s_forLoopsEncountered = 0;
		std::string forLoopNumStr = std::to_string(s_forLoopsEncountered);
		std::string headLabel = "LH" + forLoopNumStr + "@" + CurrentFunctionMetaData::funcName;
		std::string bodyLabel = "LB" + forLoopNumStr + "@" + CurrentFunctionMetaData::funcName;
		std::string exitLabel = "LE" + forLoopNumStr + "@" + CurrentFunctionMetaData::funcName;
		s_forLoopsEncountered++;

		// Fix the head (iter var init + comparison)
		GenForLoopHeadCode(code, (AST::ForLoopHeadNode*)node->GetHead(), iterVar, iterVarType, headLabel, bodyLabel, exitLabel);

		// Generate body
		code += bodyLabel + ":\n";
		GenerateFunctionBody(code, node->GetBody(), largestTempAllocation);

		// Jump back to head after executing an iteration.
		code += "jmp SHORT " + headLabel + "\n";

		// Place the exit label.
		code += exitLabel + ":\n";
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
				SymTabEntry* entry = asSymNode->GetSymTabEntry();
				return entry->asVar.type;
				break;
			}
			case Node_k::FunctionCallNode:
			{
				AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)n;
				SymTabEntry* entry = asFunctionCallNode->GetSymTabEntry();
				return entry->asFunction.retType;
				break;
			}
			case Node_k::AddrOfNode:
			{
				return PrimitiveType::pointer;
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
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);

			std::string output = "\n; Push " + t0.name + " into " + reg + "\n" +
													 FetchIntoReg(callingConvention[nextSlot], t0ActualAdress, exprType);

			code += output;

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

			//std::wstring composedKey = g_symTable.ComposeKey(asArgNode->GetName(), 1);
			//SymTabEntry* entry = g_symTable.RetrieveSymbol(composedKey);
			SymTabEntry* entry = asArgNode->GetSymTabEntry();

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

				AST::Node* assNodeVar = asAssNode->GetVar();
				const Node_k assNodeVarNodeKind = assNodeVar->GetNodeKind();

				switch (assNodeVarNodeKind)
				{
				case Node_k::SymNode:
				{
					AST::SymNode* var = (AST::SymNode*)assNodeVar;

					SymTabEntry* entry = var->GetSymTabEntry();

					if (entry == nullptr)
					{
						wprintf(L"ERROR: Couldn't find symtable entry for %s.\n", var->GetName().c_str());
						Exit(ErrCodes::undeclared_symbol);
					}


					stackLocation = entry->asVar.adress;
					exprType = entry->asVar.type;

					break;
				}
				case Node_k::DerefNode:
				{
					AST::DerefNode* derefOp = (AST::DerefNode*)assNodeVar;

					exprType = GetPointeeTypeFromDerefNode(derefOp);

					break;
				}
				}

				// Generate operation code. Remember that the temporaries stack section needs to be reset!
				TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, true);
				GenOpNodeCode(code, asAssNode->GetExpr(), t0, exprType);

				std::string output;

				switch (assNodeVarNodeKind)
				{
				case Node_k::SymNode:
				{
					output = "\n; (local var at stackLoc " + std::to_string(stackLocation) + ") = Result of expr(rax)" +
						"\nmov " + RefLocalVar(stackLocation, exprType) + ", " + GetReg(RG::RAX, exprType) + "\n";

					break;
				}

				case Node_k::DerefNode:
				{
					const PrimitiveType pointeeType = exprType;

					/*
						mov RAX, ptr
						mov [RAX], t0(holds expr)
					*/

					AST::DerefNode* asDerefNode = (AST::DerefNode*)assNodeVar;
					const PrimitiveType pointerType = PrimitiveType::pointer;
					
					TempVar t1 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize);
					GenOpNodeCode(code, asDerefNode->GetExpr(), t1, pointerType);

					output = "; Copy result to rcx, as a middle-man\n" \
						"mov rcx, " + RefTempVar(t0.adress, pointerType) +
						"\nmov [RAX], RCX\n";


					break;
				}
				}

				code += output;

				gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);
				CurrentFunctionMetaData::temporariesStackSectionSize = 0;
				
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
				GenOpNodeCode(code, asReturnNode->GetRetExpr(), t0, retType);

				// Check to see if the allocation done by the expression evaluation of GenOpNodeCode() requires more memory than the last evaluation.
				gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);

				code += "\n\n";

				break;
			}
			case Node_k::FunctionCallNode:
			{
				AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)node;
				
				PrimitiveType funcRetType = asFunctionCallNode->GetSymTabEntry()->asFunction.retType;
				
				PushArgsIntoRegs(code, asFunctionCallNode);

				code += "\ncall " + std::string(std::string(MangleFunctionName(asFunctionCallNode->GetName().c_str()))) + "\n";

				break;
			}
			case Node_k::ForLoopNode:
			{
				AST::ForLoopNode* asForLoopNode = (AST::ForLoopNode*)node;

				GenForLoopCode(code, asForLoopNode, largestTempAllocation);

				break;
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
			//mangledFuncName = "; main (In BCL: " + std::string(NarrowMainFunctionName) + ")\nmain";
			mangledFuncName = "main";
		}
		else
		{
			char* nameCStr = MangleFunctionName(asFunctionNode->GetName().c_str());
			mangledFuncName = std::string(nameCStr);
			free(nameCStr);
		}
		CurrentFunctionMetaData::funcName = mangledFuncName;
		CurrentFunctionMetaData::retType = asFunctionNode->GetRetType();
		CurrentFunctionMetaData::currentFunction = asFunctionNode;

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
