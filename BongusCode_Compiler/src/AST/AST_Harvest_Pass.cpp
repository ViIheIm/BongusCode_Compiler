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
        case Node_k::DeclNode:
        {
            AST::DeclNode* asDeclNode = (AST::DeclNode*)n;

            const std::wstring key = symtab.ComposeKey(asDeclNode->GetName());
            if (symtab.RetrieveSymbol(key))
            {
              wprintf(L"ERROR: More than 1 symbol with the same name: %s\n", asDeclNode->GetName().c_str());
              Exit(ErrCodes::duplicate_symbols);
            }

            //asDeclNode->SetScopeDepth(symtab.GetScopeDepth());
            SymTabEntry* newEntry = symtab.EnterSymbol(asDeclNode->GetName(), asDeclNode->GetType(), asDeclNode->GetPointeeType(), asDeclNode->GetSize(), false);

            // Connect declaration with the newly entered symbol table entry.
            asDeclNode->SetSymTabEntry(newEntry);

            if (asDeclNode->GetType() == PrimitiveType::nihil)
            {
                wprintf(L"ERROR: A variable can not be of type nihil.\n");
                Exit(ErrCodes::unknown_type);
            }

            break;
        }

        case Node_k::SymNode:
        {
            AST::SymNode* asSymNode = (AST::SymNode*)n;
            std::wstring composedKey = symtab.ComposeKey(asSymNode->GetName());
            SymTabEntry* sym = symtab.RetrieveSymbol(composedKey);
            if (sym == nullptr)
            {
                wprintf(L"ERROR: Undeclared symbol: %s\n", asSymNode->GetName().c_str());
                Exit(ErrCodes::undeclared_symbol);
            }

            asSymNode->SetSymTabEntry(sym);

            break;
        }

        case Node_k::FwdDeclNode:
        {
            AST::FwdDeclNode* asFwdDeclNode = (AST::FwdDeclNode*)n;

            SymTabEntry* entryCandidate = symtab.RetrieveSymbol(symtab.ComposeKey(asFwdDeclNode->GetName()));
            if (entryCandidate == nullptr)
            {
              entryCandidate = symtab.EnterSymbol(asFwdDeclNode->GetName(), asFwdDeclNode->GetRetType(), PrimitiveType::invalid, 0, true);
            }
            
            asFwdDeclNode->SetSymTabEntry(entryCandidate);

            break;
        }

        case Node_k::FunctionNode:
        {
            AST::FunctionNode* asFunctionNode = (AST::FunctionNode*)n;

            // If the entry already exists within the symbol table, then this function has been forward declared.
            std::wstring key = symtab.ComposeKey(asFunctionNode->GetName());
            SymTabEntry* entryCandidate = symtab.RetrieveSymbol(key);
            if (entryCandidate == nullptr)
            {
              entryCandidate = symtab.EnterSymbol(asFunctionNode->GetName(), asFunctionNode->GetRetType(), PrimitiveType::invalid, 0, true);
            }

            asFunctionNode->SetSymTabEntry(entryCandidate);

            // Now, set this function as the current function so that all enclosed variables' keys will be prepended with this function name.
            symtab.OpenFunction(key);

            break;
        }

        case Node_k::FunctionCallNode:
        {
            AST::FunctionCallNode* asFunctionCallNode = (AST::FunctionCallNode*)n;

            // We need a global key for our function, as the function we're trying to call lies in the global namespace, not in the current function.
            const std::wstring key = g_symTable.ComposeGlobalKey(asFunctionCallNode->GetName());
            SymTabEntry* entry = g_symTable.RetrieveSymbol(key);

            if (entry == nullptr)
            {
                wprintf(L"ERROR: Undeclared symbol \"%s\"\nThere is no function with this name.\n", asFunctionCallNode->GetName().c_str());
                Exit(ErrCodes::undeclared_symbol);
            }

            asFunctionCallNode->SetSymTabEntry(entry);

            break;
        }

        case Node_k::ArgNode:
        {
          AST::ArgNode* asArgNode = (AST::ArgNode*)n;
          SymTabEntry* entry = g_symTable.RetrieveSymbol(g_symTable.ComposeKey(asArgNode->GetName()));

          asArgNode->SetSymTabEntry(entry);
          
          break;
        }

        case Node_k::AddrOfNode:
        {
          AST::AddrOfNode* asAddrOfNode = (AST::AddrOfNode*)n;
          SymTabEntry* entry = symtab.RetrieveSymbol(symtab.ComposeKey(asAddrOfNode->GetName()));

          if (entry == nullptr)
          {
            wprintf(L"ERROR: Undeclared symbol \"%s\"\nThere is no variable with this name, you cannot get it's address.\n", asAddrOfNode->GetName().c_str());
            Exit(ErrCodes::undeclared_symbol);
          }

          asAddrOfNode->SetSymTabEntry(entry);

          break;
        }

        //case Node_k::DerefNode:
        //{
        //  AST::DerefNode* asDerefNode = (AST::DerefNode*)n;
        //
        //  SymTabEntry* entry = symtab.RetrieveSymbol(symtab.ComposeKey(asDerefNode->GetName()));
        //
        //  if (entry == nullptr)
        //  {
        //    wprintf(L"ERROR: Undeclared symbol \"%s\"\nYou cannot dereference this variable -- it doesn't exist.\n", asDerefNode->GetName().c_str());
        //    Exit(ErrCodes::undeclared_symbol);
        //  }
        //
        //  asDerefNode->SetSymTabEntry(entry);
        //
        //  break;
        //}
    }
    for (AST::Node* childnode : n->GetChildren())
    {
        ProcessNode(childnode);
    }

    if (nodeKind == Node_k::FunctionNode)
    {
      symtab.CloseFunction();
    }

    #undef symtab
}
