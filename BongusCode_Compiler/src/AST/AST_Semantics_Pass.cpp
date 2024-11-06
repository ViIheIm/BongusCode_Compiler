#include "AST_Semantics_Pass.h"
#include "ASTNode.h"
#include "../Exit.h"
#include "../symbol_table/symtable.h"

static void ProcessNode(AST::Node* n);

void AST::SemanticsPass(Node* nodeHead)
{
	ProcessNode(nodeHead);
	wprintf(L"SEMANTICS PASS: Semantically legal program recognized.\n");
}

static void ProcessNode(AST::Node* n)
{
	Node_k nodeKind = n->GetNodeKind();
	switch (nodeKind)
	{
		/*
			SEMANTIC RULE : Unreachable code is illegal.
			We will not attempt to recover from such an error by deleting
			right siblings(the unreachable code), we will simply raise an error and abort compilation.
		*/
		case Node_k::ReturnNode:
		{
			if (n->HasRightSiblings())
			{
				wprintf(L"ERROR: Unreachable code.\n");
				Exit(ErrCodes::unreachable_code);
			}

			break;
		}

		// We check to make sure that any attempted function call is done on an actual function
		case Node_k::FunctionCallNode:
		{
			AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)n;

			// We make sure that this entry exists in the harvest pass.
			SymTabEntry* entry = asFunctionCallNode->GetSymTabEntry();

			if (!entry->isFunction)
			{
				wprintf(L"ERROR: You cannot call %s -- it is not a function.\n", asFunctionCallNode->GetName().c_str());
				Exit(ErrCodes::attempted_to_call_a_non_function);
			}

			break;
		}

		/*
			Here we check for dereference operations involving more than 1 pointer.
			Adding several pointers together is illegal, so a subexpression of a dereference operation
			may only contain an offset from a single pointer.
			E.g.
				¤(ptr + 1) = 200.			<-- Legal
				¤(ptr + äggPtr) = 200  <-- illegal
		*/
		case Node_k::DerefNode:
		{
			AST::DerefNode* asDerefNode = (AST::DerefNode*)n;

			ui16 numPointersFound = 0;
			for (AST::Node* subExprSymNode : AST::GetAllChildNodesOfType(asDerefNode, Node_k::SymNode))
			{
				AST::SymNode* asSymNode = (AST::SymNode*)subExprSymNode;

				if (asSymNode->GetSymTabEntry()->asVar.type == PrimitiveType::pointer)
				{
					numPointersFound++;
				}
			}

			if (numPointersFound > 1)
			{
				wprintf(L"ERROR: You may not add several pointers together in a dereference expression.\n");
				Exit(ErrCodes::attempted_to_dereference_pointer_offset_involving_several_pointers);
			}

			break;
		}
	}
	for (AST::Node* childNode : n->GetChildren())
	{
		ProcessNode(childNode);
	}
}