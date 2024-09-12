#include "AST_Harvest_Pass.h"
#include "ASTNode.h"
#include "../symbol_table/symtable.h"
#include "../Exit.h"
#include <typeinfo>
#include <cassert>

static void ProcessNode(AST::Node* n);

void AST::BuildSymbolTable(Node* nodeHead)
{
    return ProcessNode(nodeHead);
}

static void ProcessNode(AST::Node* n)
{
    #define symtab g_symTable
    Node_k nodeKind = n->GetNodeKind();
    switch (nodeKind)
    {
        case Node_k::ScopeNode:
        {
            symtab.OpenScope();
            break;
        }

        case Node_k::DeclNode:
        {
            AST::DeclNode* asDeclNode = (AST::DeclNode*)n;
            asDeclNode->SetScopeDepth(symtab.GetScopeDepth());
            symtab.EnterSymbol(asDeclNode->GetName(), asDeclNode->GetType(), asDeclNode->GetSize(), false);

            break;
        }

        case Node_k::SymNode:
        {
            AST::SymNode* asSymNode = (AST::SymNode*)n;
            std::wstring composedKey = symtab.ComposeKey(asSymNode->GetName(), symtab.GetScopeDepth());
            SymTabEntry* sym = symtab.RetrieveSymbol(composedKey);
            if (sym == nullptr)
            {
                wprintf(L"ERROR: Undeclared symbol: %s\n", asSymNode->GetName().c_str());
                Exit(ErrCodes::undeclared_symbol);
            }
            break;
        }

        case Node_k::FunctionNode:
        {
            AST::FunctionNode* asFunctionNode = (AST::FunctionNode*)n;
            symtab.EnterSymbol(asFunctionNode->GetName(), asFunctionNode->GetRetType(), 0, true);

            break;
        }

        case Node_k::FunctionCallNode:
        {
            AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)n;
                                                                                                // All functions must exist in the global namespace, i.e. depth 0.
            SymTabEntry* entry = g_symTable.RetrieveSymbol(g_symTable.ComposeKey(asFunctionCallNode->GetName(), SymTable::s_globalNamespace));

            if (entry == nullptr)
            {
                wprintf(L"ERROR: Undeclared symbol: %s\nThere is no function with this name.\n", asFunctionCallNode->GetName().c_str());
                Exit(ErrCodes::undeclared_symbol);
            }

            break;
        }
    }
    for (AST::Node* childnode : n->GetChildren())
    {
        ProcessNode(childnode);
    }

    if (nodeKind == Node_k::ScopeNode)
    {
        symtab.CloseScope();
    }

    #undef symtab
}
