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
    node->entry = nullptr;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeOpNode(const Op_k op, Node* lhs, Node* rhs)
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

AST::Node* AST::MakeDeclNode(std::wstring* s, const PrimitiveType type, const PrimitiveType pointeeType)
{
    DeclNode* node = new DeclNode();
    assert(node && "Failed to allocate decl node");
    node->c = *s;
    node->t = type;
    node->pointeeType = pointeeType;
    node->kind = Node_k::DeclNode;
    node->entry = nullptr;
    
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
    case PrimitiveType::pointer:
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

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l, and in the function nonterminal in the parser.
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

    node->entry = nullptr;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeArgNode(std::wstring* s, const PrimitiveType type, const PrimitiveType pointeeType)
{
    ArgNode* node = new ArgNode();
    assert(node && "Failed to allocate arg node");
    node->c = *s;
    node->kind = Node_k::ArgNode;
    node->pointeeType = pointeeType;
    node->type = type;
    node->entry = nullptr;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeFunctionCallNode(std::wstring* s, Node* args)
{
    FunctionCallNode* node = new FunctionCallNode();
    assert(node && "Failed to allocate function call node");
    node->c = *s;
    node->kind = Node_k::FunctionCallNode;
    node->args = args;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeFwdDeclNode(PrimitiveType retType, std::wstring* s, Node* argsListNode)
{
    FwdDeclNode* node = new FwdDeclNode();
    assert(node && "Failed to allocate fwd decl node");
    node->kind = Node_k::FwdDeclNode;
    node->name = *s;
    node->retType = retType;

    // In the case of a Nihil arg (e.g. i32 main(Nihil)), argsListNode will be nullptr, so that is perfectly valid behaviour.
    node->argsList = argsListNode;

    node->entry = nullptr;

    // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
    delete s;

    return node;
}

AST::Node* AST::MakeExternFwdDeclNode(Node* fwdDeclNode)
{
  ExternFwdDeclNode* node = new ExternFwdDeclNode();
  assert(node && "Failed to allocate extern fwd decl node");
  node->kind = Node_k::ExternFwdDeclNode;
  node->fwdDeclNode = fwdDeclNode;

  return node;
}

AST::Node* AST::MakeAddrOfNode(std::wstring* name)
{
  AddrOfNode* node = new AddrOfNode();
  assert(node && "Failed to allocate addr of node");
  node->kind = Node_k::AddrOfNode;
  node->name = *name;
  node->entry = nullptr;

  // Accommodate the whack handover of the string. The allocation is found in {ID} in lexer.l.
  delete name;

  return node;
}

AST::Node* AST::MakeDerefNode(Node* expression)
{
  DerefNode* node = new DerefNode();
  assert(node && "Failed to allocate deref node");
  node->kind = Node_k::DerefNode;
  node->expr = expression;


  return node;
}

AST::Node* AST::MakeForLoopNode(Node* head, Node* body)
{
  ForLoopNode* node = new ForLoopNode();
  assert(node && "Failed to allocate for loop node");
  node->kind = Node_k::ForLoopNode;
  node->head = head;
  node->body = body;

  return node;
}

AST::Node* AST::MakeForLoopHeadNode(Node* upperBound, Node* lowerBound)
{
  ForLoopHeadNode* node = new ForLoopHeadNode();
  assert(node && "Failed to allocate for loop head node");
  node->kind = Node_k::ForLoopHeadNode;
  node->upperBound = upperBound;
  node->lowerBound = lowerBound;

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
    for (Node* n : parents[i]->GetChildren())
    {
      parents.push_back(n);
      callback(n, args);
    }
  }
}

std::vector<AST::Node*> AST::GetAllChildrenRecursively(Node* parent)
{
  // Frail recursion simulation again.
  std::vector<Node*> allChildren{ parent };

  for (i32 i = 0; i < allChildren.size(); i++)
  {
    for (Node* n : allChildren[i]->GetChildren())
    {
      allChildren.push_back(n);
    }
  }

  return allChildren;
}

std::vector<AST::Node*> AST::GetAllChildNodesOfType(Node* parent, const Node_k kind)
{
  std::vector<Node*> allChildren = GetAllChildrenRecursively(parent);

  std::vector<Node*> childrenOfKind;

  for (Node* n : allChildren)
  {
    if (n->kind == kind)
    {
      childrenOfKind.push_back(n);
    }
  }

  return childrenOfKind;
}
