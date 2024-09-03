#include "AST_Semantics_Pass.h"
#include "ASTNode.h"
#include "../Exit.h"

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
		// SEMANTIC RULE: Unreachable code is illegal.
		// We will not attempt to recover from such an error by deleting
		// right siblings(the unreachable code), we will simply raise an error and abort compilation.
		case Node_k::ReturnNode:
		{
			if (n->HasRightSiblings())
			{
				wprintf(L"ERROR: Unreachable code.\n");
				Exit(ErrCodes::unreachable_code);
			}
		}
	}
	for (AST::Node* childNode : n->GetChildren())
	{
		ProcessNode(childNode);
	}
}