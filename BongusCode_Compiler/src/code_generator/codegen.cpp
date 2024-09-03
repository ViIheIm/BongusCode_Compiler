#include "codegen.h"
#include "../AST/ASTNode.h"
#include "../AST/ASTAPI.h"
#include "../symbol_table/symtable.h"
#include "../Exit.h"

/*
	TODO: Integrate function name mangler from sandbox!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

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
		entry->stackLocation = *allocSize;

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
}

ui32 AllocLocals(AST::Node* funcHeadNode)
{
	// Gather up sizes.
	i32 allocSize = 0;
	AST::DoForAllChildren(funcHeadNode, &ProcessLocalsCallback, &allocSize);

	return allocSize;
}

// Allocation is done in two steps. First we get the allocated space with AllocStackSpace(), then we call CommitLastStackAlloc().
// The recordAllocs-parameter is where we store the stack space information
// (CurrentFunctionMetaData::temporariesStackSectionSize for instance).
inline static std::tuple<std::string, i32> AllocStackSpace(i32* recordAllocs, bool resetHitCount = false)
{
	static i32 hitCount = 0; // <-- Yield _t0, _t1, _t2 ...

	if (resetHitCount)
	{
		hitCount = 0;
	}

	std::string retStr("_t" + std::to_string(hitCount++));

	return std::tuple<std::string, i32>(retStr, *recordAllocs);
}

// This is where we commit the last allocation made with AllocStackSpace().
// We may allocate before we know the size, so the top-of-stack will be returned by AllocStackSpace(), meanwhile,
// when we finally know the size of our previous allocation, we call this function to commit the allocation by incrementing
// the stack variable.
inline static void CommitLastStackAlloc(i32* recordAllocs, i32 size)
{
	*recordAllocs += size;
}





namespace Prologue
{
	inline static void WriteFunctionNameProc(std::string& code)
	{
		code += "; TODO: Hard coded name, bad!\n"\
				"main PROC\n";
	}
	
	inline static void SetupStackFrame(std::string& code)
	{
		code += "push rbp\n"\
			"mov rbp, rsp\n";
	}
	
	
	
	inline static void GenerateStackAllocation(std::string& code, i32 allocSize)
	{
		code += "sub rsp, " + std::to_string(allocSize) + "\n";
	}
	
	inline static void GenerateFunctionPrologue(std::string& code, AST::Node* node, i32 stackAllocSizeForLocals, i32 stackAllocSizeForTemporaries)
	{
		WriteFunctionNameProc(code);
	
		SetupStackFrame(code);
	
		code += "; Alloc section for local variables.\n";
		GenerateStackAllocation(code, stackAllocSizeForLocals);
		code += "; Alloc section for temporary variables.\n";
		GenerateStackAllocation(code, stackAllocSizeForTemporaries);
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
	constexpr const i32 temporariesTOS = 0;
	inline static std::string RefTempVar(const i32 offset)
	{
		// EXAMPLE:
		// mov eax, DWORD PTR 4[rsp]					// Go past the locals and into the temporaries section of the stack.
		return std::string("QWORD PTR " + std::to_string(CurrentFunctionMetaData::varsStackSectionSize + offset) + "[rsp]");
	}
	inline static std::string RefLocalVar(const i32 offset)
	{
		return std::string("QWORD PTR " + std::to_string(offset) + "[rsp]");
	}

	//										TODO: Stick name and place in a custom struct and send a const reference to that, to reduce
	//											  memory usage for this recursive function.
	static void GenOpNodeCode(std::string& code, AST::Node* node, const std::string& name, const i32 place)
	{
		/*
			; (c + d)
			mov eax, DWORD PTR d$[rsp]
			mov ecx, DWORD PTR c$[rsp]
			add ecx, eax				; (ecx = c + d)
			mov eax, ecx				; (eax = c + d) CACHE RES IN EAX
		*/
		visitedNodes.push_back(node);
	
		switch (node->GetNodeKind())
		{
		case Node_k::OpNode:
		{
			AST::OpNode* asOpNode = (AST::OpNode*)node;
			std::string opString = asOpNode->GetOpAsString();
	

			GenOpNodeCode(code, asOpNode->GetLHS(), name, place);
	
			auto [t1, t1place] = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize);
			GenOpNodeCode(code, asOpNode->GetRHS(), t1, t1place);
	
			if (opString == "+")
			{
				std::string output = "\n; " + name + " += " + t1 +
									 "\nmov rax, " + RefTempVar(place) +		// Store _tfirst in eax
									 "\nadd rax, " + RefTempVar(t1place) +		// Perform operation in eax
									 "\nmov " + RefTempVar(place) + ", rax\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
			if (opString == "-")
			{
				std::string output = "\n; " + name + " -= " + t1 +
									 "\nmov rax, " + RefTempVar(place) +		// Store _tfirst in eax
									 "\nsub rax, " + RefTempVar(t1place) +		// Perform operation in eax
									 "\nmov " + RefTempVar(place) + ", rax\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
			else if (opString == "*")
			{
				std::string output = "\n; " + name + " *= " + t1 +
									 "\nmov rax, " + RefTempVar(place) +		// Store _tfirst in eax
									 "\nimul rax, " + RefTempVar(t1place) +		// Perform operation in eax
									 "\nmov " + RefTempVar(place) + ", rax\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
			else if (opString == "/")
			{
				// TODO: Division is anal. Here's a guide:
				// https://www.youtube.com/watch?v=vwTYM0oSwjg
				// TLDR: For 64 bit division, the result goes in rax, the remainder in rdx
				// The divisor goes in rbx.
				std::string output = "\n; " + name + " /= " + t1 +
									 "\nmov rax, " + RefTempVar(place) +		// Store _tfirst in eax
									 "\nmov rbx, " + RefTempVar(t1place) +		// Store divisor in rbx
									 "\nxor rdx, rdx" +							// You have to make sure to 0 out rdx first, or else you get an integer underflow :P.
									 "\ndiv rbx" +								// Perform operation in ebx
									 "\nmov rbx, 3405691582 ; 0xCAFEBABE" +		// Store sentinel value CAFEBABE in rbx in case of bugs.
									 "\nmov rdx, 4276993775 ; 0xFEEDBEEF" +		// Do the same for rdx with FEEDBEEF since it was also used.
									 "\nmov " + RefTempVar(place) + ", rax\n";	// Store result in _tfirst on stack
				code += output;
				// std::cerr << output;
			}
	
			break;
		}
		case Node_k::IntNode:
		{
			AST::IntNode* asIntNode = (AST::IntNode*)node;
	
			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, asIntNode->GetSize());

			std::string output = "\n; " + name + " = " + std::to_string(asIntNode->Get()) +
								 "\nmov " + RefTempVar(place) + ", " + std::to_string(asIntNode->Get()) +
								 "\nmov rax, " + RefTempVar(place) + "\n"; // Also store result in rax.
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

			CommitLastStackAlloc(&CurrentFunctionMetaData::temporariesStackSectionSize, entry->size);

			std::string output = "\n; " + name + " = (local var at stackLoc " + std::to_string(entry->stackLocation) + ")" +
								 "\nmov rax, " + RefLocalVar(entry->stackLocation) +
								 "\nmov " + RefTempVar(place) + ", rax" + "\n";

			code += output;
			// std::cerr << output;
		}
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
			auto[t1, t1place] = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, true);
			GenOpNodeCode(code, node, t1, t1place);
			
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


				stackLocation = entry->stackLocation;
			}

			// Generate operation code. Remember that the temporaries stack section needs to be reset!
			auto [t1, t1place] = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, true);
			GenOpNodeCode(code, asAssNode->GetExpr(), t1, t1place);

			// Check to see if the allocation done by the expression evaluation of GenOpNodeCode() requires more memory than the last evaluation.
			gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);
			// Enforce allocation policy(doing the aforementioned resetting).
			CurrentFunctionMetaData::temporariesStackSectionSize = 0;




			// Store result (held in rax and on the temporaries-stack) in var.
			std::string output = "\n; (local var at stackLoc " + std::to_string(stackLocation) + ") = Result of expr(rax)" +
								 "\nmov " + RefLocalVar(stackLocation) + ", rax\n";

			code += output;
			// std::cerr << output;

			break;
		}
		case Node_k::ReturnNode:
		{
			AST::ReturnNode* asReturnNode = (AST::ReturnNode*)node;

			// The result of the expression we're about to evaluate is stored in rax by default,
			// so we don't need to do anything more than ensure that the expression's code is generated.

			code += "\n; Return expression:";

			// Generate operation code. Remember that the temporaries stack section needs to be reset!
			auto [t1, t1place] = AllocStackSpace(&CurrentFunctionMetaData::temporariesStackSectionSize, true);
			GenOpNodeCode(code, asReturnNode->GetRetExpr(), t1, t1place);

			// Check to see if the allocation done by the expression evaluation of GenOpNodeCode() requires more memory than the last evaluation.
			gatherLargestAllocation(largestTempAllocation, CurrentFunctionMetaData::temporariesStackSectionSize);
			// Enforce allocation policy(doing the aforementioned resetting).
			CurrentFunctionMetaData::temporariesStackSectionSize = 0;

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

namespace Epilogue
{
	inline static void WriteFunctionNameEndp(std::string& code)
	{
		code += "; TODO: Hard coded name, bad!\n"\
				"main ENDP\n";
	}

	inline static void RestoreStackFrame(std::string& code)
	{
		code += "mov rsp, rbp\n"\
				"pop rbp\n";
	}

	inline static void GenerateStackDeallocation(std::string& code, i32 allocSize)
	{
		code += "add rsp, " + std::to_string(allocSize) + "\n";
	}

	inline static void GenerateFunctionEpilogue(std::string& code, AST::Node* node, i32 stackAllocSizeForLocals, i32 stackAllocSizeForTemporaries)
	{
		code += "; Dealloc section for local variables.\n";
		GenerateStackDeallocation(code, stackAllocSizeForLocals);
		code += "; Dealloc section for temporary variables.\n";
		GenerateStackDeallocation(code, stackAllocSizeForTemporaries);

		RestoreStackFrame(code);

		code += "ret\n";

		WriteFunctionNameEndp(code);
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

	// Function body, add loop later
	{
		std::string prologue, body, epilogue;
		
		// Because we use the stack for temporaries, we need to figure out how much stack space to reserve in the body,
		// and cannot go on amount of declnodes alone in the prologue function.
		// Therefore, we defer the composing of the complete code until we know the total amount of stack space to reserve.

		// Firstly, figure out the amount of stack space required by local variables, and allocate them.
		// Results for variables is stored in the symbol table.
		CurrentFunctionMetaData::varsStackSectionSize = AllocLocals(nodeHead);

		// We need to get the biggest size the stack will ever grow to so we can enforce our allocation policy.
		// Without this we'd allocate more and more stack size for each expression evaluation, even though temporaries should start back at 0 when evaluating a new expression.
		i32 largestTemporariesAlloc = 0;

		body += "\n\n\n; Body\n";
		Body::GenerateFunctionBody(body, nodeHead, &largestTemporariesAlloc);

		CurrentFunctionMetaData::temporariesStackSectionSize = largestTemporariesAlloc;

		prologue += "\n\n\n; Prologue\n";
		Prologue::GenerateFunctionPrologue(prologue, nodeHead, CurrentFunctionMetaData::varsStackSectionSize, CurrentFunctionMetaData::temporariesStackSectionSize);

		epilogue += "\n\n\n; Epilogue\n";
		Epilogue::GenerateFunctionEpilogue(epilogue, nodeHead, CurrentFunctionMetaData::varsStackSectionSize, CurrentFunctionMetaData::temporariesStackSectionSize);
	
		outCode += prologue + body + epilogue;
	}

	Boilerplate::GenerateFooter(boilerplateFooter);
	outCode += boilerplateFooter;

	// std::cerr << outCode << std::endl;
}
