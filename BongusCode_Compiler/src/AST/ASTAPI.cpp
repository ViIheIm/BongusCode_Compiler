#include "ASTAPI.h"
#include "ASTNode.h"
#include "../Exit.h"
#include <cassert>

AST::Node* AST::MakeIntNode(i32 n)
{
    IntNode* node = new IntNode();
    assert(node && "Failed to allocate int node");
    node->n = n;
    node->kind = Node_k::IntNode;
    return node;
}

AST::Node* AST::MakeSymNode(std::wstring* s)
{
    SymNode* node = new SymNode();
    assert(node && "Failed to allocate sym node");
    node->c = *s;
    node->kind = Node_k::SymNode;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeOpNode(wchar_t op, Node* lhs, Node* rhs)
{
    OpNode* node = new OpNode();
    assert(node && "Failed to allocate op node");
    node->lhs = lhs;
    node->rhs = rhs;
    node->op = op;
    node->kind = Node_k::OpNode;
    return node;
}

AST::Node* AST::MakeAssNode(Node* var, Node* expr)
{
    AssNode* node = new AssNode();
    assert(node && "Failed to allocate assignment node");
    node->var = var;
    node->expr = expr;
    node->kind = Node_k::AssNode;
    return node;
}

AST::Node* AST::MakeScopeNode()
{
    ScopeNode* node = new ScopeNode();
    assert(node && "Failed to allocate scope node");
    node->kind = Node_k::ScopeNode;
    return node;
}

AST::Node* AST::MakeDeclNode(std::wstring* s, PrimitiveType type)
{
    DeclNode* node = new DeclNode();
    assert(node && "Failed to allocate decl node");
    node->c = *s;
    node->t = type;
    node->kind = Node_k::DeclNode;
    
    // Figure out size.
    switch (type)
    {
    case PrimitiveType::invalid:
    {
        node->size = -1;
        wprintf(L"ERROR: Invalid type encountered in: " __FUNCSIG__ L"\n");
        Exit(ErrCodes::unknown_type);
        break;
    }
    case PrimitiveType::ui8:
    case PrimitiveType::i8:
    {
        node->size = 1;
        break;
    }
    case PrimitiveType::ui16:
    case PrimitiveType::i16:
    {
        node->size = 2;
        break;
    }
    case PrimitiveType::ui32:
    case PrimitiveType::i32:
    {
        node->size = 4;
        break;
    }
    case PrimitiveType::ui64:
    case PrimitiveType::i64:
    {
        node->size = 8;
        break;
    }
    default:
    {
        node->size = -1;
        wprintf(L"ERROR: Unknown type encountered in: " __FUNCSIG__ L"\n");
        Exit(ErrCodes::unknown_type);
        break;
    }
    }

    // Default scope depth to 0. This is properly handled later in the harvest pass over the AST, and in the function nonterminal in the parser.
    node->scopeDepth = 0;


    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeReturnNode(Node* retExpr)
{
    ReturnNode* node = new ReturnNode();
    assert(node && "Failed to allocate return node");
    node->retExpr = retExpr;
    node->kind = Node_k::ReturnNode;
    return node;
}

AST::Node* AST::MakeFunctionNode(PrimitiveType retType, std::wstring* s, Node* argsListNode)
{
    FunctionNode* node = new FunctionNode();
    assert(node && "Failed to allocate function node");
    node->kind = Node_k::FunctionNode;
    node->name = *s;
    node->retType = retType;

    // In the case of a Nihil arg (e.g. i32 main(Nihil)), argsListNode will be nullptr, so that is perfectly valid behaviour.
    node->argsList = argsListNode;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeArgNode(PrimitiveType type, std::wstring* s)
{
    ArgNode* node = new ArgNode();
    assert(node && "Failed to allocate arg node");
    node->c = *s;
    node->kind = Node_k::ArgNode;
    node->type = type;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeNullNode()
{
    Node* node = new Node();
    assert(node && "Failed to allocate null node");
    node->kind = Node_k::Node;
    return node;
}

void AST::DoForAllChildren(Node* parent, void(*callback)(Node*, void*), void* args)
{
    // Simulate recursion by simulating the stack growing as parent would change.
    // I suspect this code is very fickle and frail, so keep you eye on it, because it will break at some point.
    std::vector<Node*> parents{ parent };

    for (i32 i = 0; i < parents.size(); i++)
    {
        for (Node* n = parents[i]->lmostChild; n != nullptr; n = n->rSibling)
        {
            parents.push_back(n);
            callback(n, args);
        }
    }
}
