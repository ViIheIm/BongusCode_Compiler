#include "ASTNode.h"
#include <cassert>

AST::Node::Node(Node* c_rSibling, Node* c_lmostChild, Node* c_parent)
    : rSibling(c_rSibling)
    , lmostSibling(this) /* lmostSibling cannot be nullptr for MakeSiblings() to work. */
    , lmostChild(c_lmostChild)
    , parent(c_parent)
{
    // Register ourselves on the disaster list.
    g_disasterHandle.push_back(this);
}

AST::Node::~Node()
{
    delete rSibling;
    delete lmostChild;
}

AST::Node* AST::Node::MakeSiblings(AST::Node* y)
{
    assert(y && "y may not be null");

    // Find the rightmost node in this list.
    AST::Node* xsibs = this;
    while (xsibs->rSibling != nullptr)
    {
        xsibs = xsibs->rSibling;
    }

    // Join the lists.
    AST::Node* ysibs = y->lmostSibling;
    xsibs->rSibling = ysibs;

    // Set pointers for the new siblings.
    ysibs->lmostSibling = xsibs->lmostSibling;
    ysibs->parent = xsibs->parent;
    while (ysibs->rSibling != nullptr)
    {
        ysibs = ysibs->rSibling;
        ysibs->lmostSibling = xsibs->lmostSibling;
        ysibs->parent = xsibs->parent;
    }

    return ysibs;
}

void AST::Node::AdoptChildren(AST::Node* y)
{
    assert(y && "y may not be null");

    if (lmostChild != nullptr)
    {
        lmostChild->MakeSiblings(y);
    }
    else
    {
        AST::Node* ysibs = y->lmostSibling;
        lmostChild = ysibs;

        while (ysibs != nullptr)
        {
            ysibs->parent = this;
            ysibs = ysibs->rSibling;
        }
    }
}

std::vector<AST::Node*> AST::Node::GetChildren(void)
{
    // Easy first hit.
    if (lmostChild == nullptr)
    {
        return std::vector<Node*>();
    }
    
    std::vector<Node*> res;

    // Simply drop down to our child and keep adding on rightward siblings.
    Node* n = lmostChild;
    while (n != nullptr)
    {
        res.push_back(n);
        n = n->rSibling;
    }

    return res;
}


AST::OpNode::~OpNode()
{
    delete lhs;
    delete rhs;
}

std::vector<AST::Node*> AST::OpNode::GetChildren(void)
{
    // Run base implementation first.
    std::vector<Node*> res = Node::GetChildren();

    if (lhs != nullptr)
    {
        res.push_back(lhs);
    }
    if (rhs != nullptr)
    {
        res.push_back(rhs);
    }

    return res;
}





AST::AssNode::~AssNode()
{
    delete var;
    delete expr;
}

std::vector<AST::Node*> AST::AssNode::GetChildren(void)
{
    // Run base implementation first.
    std::vector<Node*> res = Node::GetChildren();

    if (var != nullptr)
    {
        res.push_back(var);
    }
    if (expr != nullptr)
    {
        res.push_back(expr);
    }

    return res;
}

AST::ReturnNode::~ReturnNode()
{
    delete retExpr;
}

std::vector<AST::Node*> AST::ReturnNode::GetChildren(void)
{
    // Run base implementation first.
    std::vector<Node*> res = Node::GetChildren();

    if (retExpr != nullptr)
    {
        res.push_back(retExpr);
    }

    return res;
}

std::vector<AST::Node*> AST::FunctionNode::GetChildren(void)
{
    // Run base implementation first.
    std::vector<Node*> res = Node::GetChildren();

    if (argsList != nullptr)
    {
        res.push_back(argsList);

        for (Node* nextNode = argsList->GetRightSibling(); nextNode != nullptr; nextNode = nextNode->GetRightSibling())
        {
          res.push_back(nextNode);
        }
    }

    return res;
}

std::vector<AST::Node*> AST::FunctionCallNode::GetChildren(void)
{
  // Run base implementation first.
  std::vector<Node*> res = Node::GetChildren();

  if (args != nullptr)
  {
    res.push_back(args);

    for (Node* nextNode = args->GetRightSibling(); nextNode != nullptr; nextNode = nextNode->GetRightSibling())
    {
      res.push_back(nextNode);
    }
  }

  return res;
}

std::vector<AST::Node*> AST::ExternFwdDeclNode::GetChildren(void)
{
  // Run base implementation first.
  std::vector<Node*> res = Node::GetChildren();

  res.push_back(fwdDeclNode);

  return res;
}

std::vector<AST::Node*> AST::DerefNode::GetChildren(void)
{
  // Run base implementation first.
  std::vector<Node*> res = Node::GetChildren();

  if (expr != nullptr)
  {
    res.push_back(expr);

    for (Node* nextNode = expr->GetRightSibling(); nextNode != nullptr; nextNode = nextNode->GetRightSibling())
    {
      res.push_back(nextNode);
    }
  }

  return res;
}

std::vector<AST::Node*> AST::ForLoopNode::GetChildren(void)
{
  // Run base implementation first.
  std::vector<Node*> res = Node::GetChildren();

  res.push_back(head);
  res.push_back(body);

  return res;
}

std::vector<AST::Node*> AST::ForLoopHeadNode::GetChildren(void)
{
  // Run base implementation first.
  std::vector<Node*> res = Node::GetChildren();

  res.push_back(upperBound);
  res.push_back(lowerBound);

  return res;
}


