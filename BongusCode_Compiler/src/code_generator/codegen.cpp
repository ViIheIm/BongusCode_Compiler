#include "codegen.h"
#include "../AST/ASTNode.h"
#include "../AST/ASTAPI.h"
#include "../symbol_table/symtable.h"
#include "../Exit.h"
#include "../Utils.h"
#include <cassert>
#include <iostream>



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
	PrimitiveType type;
};


static i32 g_tempsNamingCounter = 0;

inline static void ResetTempsNaming(void)
{
	g_tempsNamingCounter = 0;
}

// The recordAllocs-parameter is where we store the stack space information
// (CurrentFunctionMetaData::temporariesStackSectionSize for instance).
inline static TempVar AllocStackSpace(i32* recordAllocs, const i32 size, const PrimitiveType type)
{
	std::string retStr("_t" + std::to_string(g_tempsNamingCounter++));
	const i32 stackAdress = *recordAllocs;
	*recordAllocs += size;

	return { retStr, stackAdress, type };
}

// This is where we commit the last allocation made with AllocStackSpace().
// We may allocate before we know the size, so the top-of-stack will be returned by AllocStackSpace(), meanwhile,
// when we finally know the size of our previous allocation, we call this function to commit the allocation by incrementing
// the stack variable.
//inline static void CommitLastStackAlloc(i32* recordAllocs, i32 size)
//{
//	*recordAllocs += size;
//}

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
	// string2 -> REG variant
	// string3 -> size variant (WORD PTR / DWORD PTR / QWORD PTR)
	std::tuple<std::string, std::string, std::string> GetFetchInstructionsForType(const RG reg, const PrimitiveType type)
	{
		std::string movVariant("mov");
		std::string regVariant = GetReg(reg, type);
		std::string sizeVariant = GetWordKindFromType(type);

		if (type == PrimitiveType::i16 || type == PrimitiveType::ui16)
		{
			movVariant = "movzx";
			regVariant = GetReg(reg, PrimitiveType::ui64); // Yields 64-bit version (e.g. RAX).
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
		std::string result = "mov " + GetWordKindFromType(destType) + " " + std::to_string(destAdress) + "[rsp]" + ", " + GetReg(reg, destType);

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
					¤(pointer + 1) = 200
				and not typically
					¤(pointer + **another pointer**) = 200

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
			wprintf(L"ERROR: couldn't find pointee type in " __FUNCTION__ "\n");
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


	static TempVar GenOpNodeCode(std::string& code, AST::Node* node)
	{
		visitedNodes.push_back(node);

	
		switch (node->GetNodeKind())
		{
		case Node_k::OpNode:
		{
			AST::OpNode* asOpNode = (AST::OpNode*)node;
			std::string opString = asOpNode->GetOpAsString();
	

			TempVar t0 = GenOpNodeCode(code, asOpNode->GetLHS());
			TempVar t1 = GenOpNodeCode(code, asOpNode->GetRHS());
	
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);
			const PrimitiveType t0Type = t0.type;

			const i32 t1ActualAdress = GetAdressOfTemporary(t1);
			const PrimitiveType t1Type = t1.type;

			if (opString == "+")
			{
				std::string output = "\n; " + t0.name + " += " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, t0Type) + "\n" +
														 OperateOnReg(RG::RAX, "add", t1ActualAdress, t1Type) + "\n" +
														 PushRegIntoMem(RG::RAX, t0ActualAdress, t0Type);
				code += output;
			}
			if (opString == "-")
			{
				std::string output = "\n; " + t0.name + " -= " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, t0Type) + "\n" +
														 OperateOnReg(RG::RAX, "sub", t1ActualAdress, t1Type) + "\n" +
														 PushRegIntoMem(RG::RAX, t0ActualAdress, t0Type);

				code += output;
			}
			else if (opString == "*")
			{
				std::string output = "\n; " + t0.name + " *= " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, t0Type) + "\n" +
														 OperateOnReg(RG::RAX, "imul", t1ActualAdress, t1Type) + "\n" +
														 PushRegIntoMem(RG::RAX, t0ActualAdress, t0Type);

				code += output;
			}
			else if (opString == "/")
			{
				// TODO: Division is anal. Here's a guide:
				// https://www.youtube.com/watch?v=vwTYM0oSwjg
				// TLDR: For 64 bit division, the result goes in rax, the remainder in rdx
				// The divisor goes in rbx.
				//const std::string& RBX = GetReg(RG::RBX, exprType);
				//const std::string& RDX = GetReg(RG::RDX, exprType);

				std::string output = "\n; " + t0.name + " /= " + t1.name + "\n" +
														 FetchIntoReg(RG::RAX, t0ActualAdress, t0Type) + "\n" +									// Store _tfirst in eax
														 FetchIntoReg(RG::RBX, t1ActualAdress, t1Type) + "\n" +									// Store divisor in rbx
														 "xor RDX, RDX\n" +																											// You have to make sure to 0 out rdx first, or else you get an integer underflow :P.
														 "div RBX\n" +																													// Perform operation in ebx
														 FetchImmediateIntoReg(RG::RBX, "3405691582 ; 0xCAFEBABE") + "\n" +			// Store sentinel value CAFEBABE in rbx in case of bugs.
														 FetchImmediateIntoReg(RG::RDX, "4276993775 ; 0xFEEDBEEF") + "\n" +			// Do the same for rdx with FEEDBEEF since it was also used.
														 PushRegIntoMem(RG::RAX, t0ActualAdress, t0Type);												// Store result in _tfirst on stack
				code += output;
			}

			return t0;
		}
		case Node_k::IntNode:
		{
			AST::IntNode* asIntNode = (AST::IntNode*)node;

			const PrimitiveType t0Type = AST::IntNode::s_defaultIntLiteralType;
			TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, GetSizeFromType(t0Type), t0Type);

			std::string intValueAsString = std::to_string(asIntNode->Get());
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);
			

			std::string output = "\n; " + t0.name + " = " + intValueAsString + "\n" +
													 FetchImmediateIntoMem(t0ActualAdress, t0Type, intValueAsString) + "\n" +
													 FetchIntoReg(RG::RAX, t0ActualAdress, t0Type);

			code += output;
	
			return t0;
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

			TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, GetSizeFromType(entry->asVar.type), entry->asVar.type);
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);

			std::string output = "\n; " + t0.name + " = (local var at stackLoc " + std::to_string(entry->asVar.adress) + ")\n" +
													 FetchIntoReg(RG::RAX, entry->asVar.adress, t0.type) + "\n" +
													 PushRegIntoMem(RG::RAX, t0ActualAdress, t0.type);

			code += output;

			return t0;
		}
		case Node_k::FunctionCallNode:
		{
			AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)node;

			// We've already made sure in the harvest pass that this is indeed a function, and in the semantics pass that this function can be called.
			std::string output;

			PushArgsIntoRegs(output, asFunctionCallNode);

			//std::string mangledFunctionName = std::string(MangleFunctionName(asFunctionCallNode->GetName().c_str()));
			std::string mangledFunctionName = asFunctionCallNode->GetSymTabEntry()->functionName;

			const PrimitiveType funcRetType = asFunctionCallNode->GetSymTabEntry()->asFunction.retType;
			TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, GetSizeFromType(funcRetType), funcRetType);
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);

			// Make sure to also store the result out into _t0.
			output += "\n; " + t0.name + " = result of function " + mangledFunctionName + "\n" +
								"call " + mangledFunctionName + "\n" +
								PushRegIntoMem(RG::RAX, t0ActualAdress, funcRetType);

			code += output;

			return t0;
		}
		case Node_k::AddrOfNode:
		{
			AST::AddrOfNode* asAddrOfNode = (AST::AddrOfNode*)node;
			SymTabEntry* entry = asAddrOfNode->GetSymTabEntry();
			const PrimitiveType addrOfNodeExprType = PrimitiveType::pointer;

			TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, GetSizeFromType(addrOfNodeExprType), addrOfNodeExprType);

			const i32 t0ActualAdress = GetAdressOfTemporary(t0);
			std::string output = "\n; " + t0.name + " = (addr of local var at stackLoc " + std::to_string(entry->asVar.adress) + ")\n" +
													 OperateOnReg(RG::RAX, "lea", entry->asVar.adress, addrOfNodeExprType) + "\n" +
													 PushRegIntoMem(RG::RAX, t0ActualAdress, addrOfNodeExprType);


			code += output;

			return t0;
		}
		case Node_k::DerefNode:
		{
			AST::DerefNode* asDerefNode = (AST::DerefNode*)node;
			const PrimitiveType pointerType = PrimitiveType::pointer;
			
			TempVar t0 = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, GetSizeFromType(pointerType), pointerType);

			const PrimitiveType pointeeType = GetPointeeTypeFromDerefNode(asDerefNode);

			TempVar t1 = GenOpNodeCode(code, asDerefNode->GetExpr());

			std::string output = GenDerefCode(pointeeType);
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);
			

			output += "\n; Move out to " + t0.name + "\n" +
								PushRegIntoMem(RG::RAX, t0ActualAdress, t0.type);

			code += output;

			return t0;
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
							"\nmov " + assignmentString + ", " + toFromReg;

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

			std::string output = "\n" + movToRaxOp + readReg + ", " + RefLocalVar(entry->asVar.adress, symType) +
													 "\nmov " + assignmentString + ", " + writeReg;

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

			const std::string& toReg = GetReg(RG::RAX, iterVarType);

			std::string output = "\nmov " + toReg + ", " + std::to_string(asIntNode->Get());
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

			std::string output = "\n" + movToRaxOp + readReg + ", " + bringInSymString + "\n";
			code += output;

			break;
		}
		}
		
		// Now it's time to compare with the iter variable and jump if greater than or equal to.
		const std::string iterVarString = GetWordKindFromType(iterVarType) + " " + std::to_string(iterVarAddress) + "[rsp]";
		code += "\ncmp " + iterVarString + ", " + GetReg(RG::RAX, iterVarType) +
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
		const i32 actualAddress = GetAdressOfTemporary(iterVar);
		GenAssignmentToStackMem(code, node->GetLowerBound(), actualAddress, iterVarType);


		// Now we must generate the jump instruction.
		code += "\njmp SHORT " + bodyLabel + "\n";

		// And then for the actual head, where we increment the iter variable.

		const std::string iterVarAssignmentString = GetWordKindFromType(iterVarType) + " " + std::to_string(actualAddress) + "[rsp]";

		const std::string& toFromReg = GetReg(RG::RAX, iterVarType);

		code += "\n" + headLabel + ":\n" +
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
		const PrimitiveType iterVarType = PrimitiveType::ui64;
		TempVar iterVar = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, GetSizeFromType(iterVarType), iterVarType);
		
		
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
			const PrimitiveType argType = GetTypeFromNode(arg);
			const i32 exprSize = GetSizeFromType(argType);

			// We need to generate the code for the values we're pushing before we push them.
			TempVar t0 = GenOpNodeCode(code, arg);

			const std::string& reg = GetReg(callingConvention[nextSlot], argType);
			const i32 t0ActualAdress = GetAdressOfTemporary(t0);

			std::string output = "\n; Push " + t0.name + " into " + reg + "\n" +
													 FetchIntoReg(callingConvention[nextSlot], t0ActualAdress, argType);

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
				ResetTempsNaming();
				TempVar t0 = GenOpNodeCode(code, node);
				
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

				// Generate operation code. Remember that the temporaries naming scheme needs to be reset!
				ResetTempsNaming();
				TempVar t0 = GenOpNodeCode(code, asAssNode->GetExpr());

				std::string output;

				switch (assNodeVarNodeKind)
				{
				case Node_k::SymNode:
				{
					//output = "\n; (local var at stackLoc " + std::to_string(stackLocation) + ") = Result of expr(rax)" +
					//	"\nmov " + RefLocalVar(stackLocation, exprType) + ", " + GetReg(RG::RAX, exprType) + "\n";
					output = "\n; (local var at stackLoc " + std::to_string(stackLocation) + ") = Result of expr(rax)\n" +
									 PushRegIntoMem(RG::RAX, stackLocation, exprType) + "\n";

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
					
					// Result held in RAX, hence not using t1.
					TempVar t1 = GenOpNodeCode(code, asDerefNode->GetExpr());

					const i32 t0ActualAdress = GetAdressOfTemporary(t0);


#pragma region REFACTORINO
					// Get the correct version of RCX for pushing RCX -> pointee.
					// TODO: Refactor.
					std::string RCXVariant("invalid register");
					switch (pointeeType)
					{
					case PrimitiveType::ui16:
					case PrimitiveType::i16:
						RCXVariant = "CX";
						break;

					case PrimitiveType::ui32:
					case PrimitiveType::i32:
						RCXVariant = "ECX";
						break;

					case PrimitiveType::ui64:
					case PrimitiveType::i64:
						RCXVariant = "RCX";
						break;

					default:
						break;
					}
					if (RCXVariant == "invalid register")
					{
						wprintf(L"ERROR: Couldn't find register in " __FUNCTION__ "\n");
						Exit(ErrCodes::internal_compiler_error);
					}
#pragma endregion


					output = "\n; Copy " + t0.name + " to rcx, as a middle-man\n" +
									 FetchIntoReg(RG::RCX, t0ActualAdress, pointerType) + "\n"
									 "mov [RAX], " + RCXVariant + "\n";


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

				// Generate operation code. Remember that the temporaries naming scheme needs to be reset!
				ResetTempsNaming();
				TempVar t0 = GenOpNodeCode(code, asReturnNode->GetRetExpr());

				// Check to see if the allocation done by the expression evaluation of GenOpNodeCode() requires more memory than the last evaluation.
				gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);

				code += "\n\n";

				break;
			}
			case Node_k::FunctionCallNode:
			{
				AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)node;
				
				SymTabEntry* entry = asFunctionCallNode->GetSymTabEntry();

				PrimitiveType funcRetType = entry->asFunction.retType;
				
				PushArgsIntoRegs(code, asFunctionCallNode);

				//code += "\ncall " + std::string(std::string(MangleFunctionName(asFunctionCallNode->GetName().c_str()))) + "\n";
				code += "\ncall " + entry->functionName + "\n";

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
	inline static std::string GetExternFunctionsList(AST::Node* nodeHead)
	{
		std::string result("; External C functions list\n");

		for (AST::Node* childNode : nodeHead->GetChildren())
		{
			if (childNode->GetNodeKind() == Node_k::ExternFwdDeclNode)
			{
				AST::ExternFwdDeclNode* asExternFwdDeclNode = (AST::ExternFwdDeclNode*)childNode;
				AST::FwdDeclNode* fwdDeclNode = (AST::FwdDeclNode*)asExternFwdDeclNode->GetFwdDeclNode();

				// The name is mangled in the harvest pass.
				result += "EXTERN " + fwdDeclNode->GetSymTabEntry()->functionName + " : PROC\n";
			}
		}

		return result;
	}

	// TODO: Generation of the data and code sections should probably be handled differently, and maybe move these out from this namespace.
	inline static void GenerateDataSection(std::string& code)
	{
		code += ".data\n\n\n";
	}

	inline static void GenerateCodeSection(std::string& code)
	{
		code += ".code\n";
	}

	inline static void GenerateHeader(AST::Node* nodeHead, std::string& code)
	{
		code += "OPTION DOTNAME   ; Allows the use of dot notation(MASM64 requires this for 64 - bit assembly)\n";

		code += "\n\n";

		code += GetExternFunctionsList(nodeHead) + "\n";

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

	Boilerplate::GenerateHeader(nodeHead, boilerplateHeader);
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
			//char* nameCStr = MangleFunctionName(asFunctionNode->GetName().c_str());
			//mangledFuncName = std::string(nameCStr);
			//free(nameCStr);
			mangledFuncName = asFunctionNode->GetSymTabEntry()->functionName;
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
