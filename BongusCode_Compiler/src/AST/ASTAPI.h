#pragma once
#include "../Definitions.h"
#include "../BongusTable.h"
#include <string>

/*
	API is implemented from specification on page 252 an onward.
*/

namespace AST
{

	class Node;
	class OpNode;
	
	// makeIntNode(int n) instantiates a node that represents the constant integer n and that offers
	// an accessor method that returns n.
	Node* MakeIntNode(i32 n);

	// makeSymNode(Symbol s) instantiates a node for a symbol s.
	// Methods must	be included to set and get the symbol table entry for s,
	// from which its type, protection, and scope information can be retrieved
	Node* MakeSymNode(std::wstring* s);

	// makeOpNode(Operator o) instantiates a node for an operation, such as
	// addition or subtraction.
	// Details of the operation must be provided by accessor methods.
	Node* MakeOpNode(wchar_t op, Node* lhs, Node* rhs);

	// makeAssNode(Node var, Node expr) instantiates a node for an assignment to node var.
	Node* MakeAssNode(Node* var, Node* expr);

	// makeBlockNode() instantiates a block node. The short is only there for overload resolution.
	Node* MakeScopeNode();

	// makeNode(Symbol s) instantiates a node for a variable decl with the name s.
	Node* MakeDeclNode(std::wstring* s, PrimitiveType type);

	// makeNode(OpNode retExpr) instantiates a node for a return operation.
	Node* MakeReturnNode(Node* retExpr);

	// makeNode(ret_t, name, argsList) instantiates a function head node.
	Node* MakeFunctionNode(PrimitiveType retType, std::wstring* s, Node* argsListNode);

	// makeNode(Symbol name, Type type) instantiates a node for an argument list.
	Node* MakeArgNode(PrimitiveType type, std::wstring* s);

	// makeNode(Symbol name) instantiates a function call node.
	Node* MakeFunctionCallNode(std::wstring* s, Node* args);

	// makeFwdDeclNode(ret_t, name, argsList) instantiates a forward decl node in a similar manner as to makeFunctionNode.
	Node* MakeFwdDeclNode(PrimitiveType retType, std::wstring* s, Node* argsListNode);

	// makeNullNode() instantiates a null node that explicitly represents the
	// absence of structure.For consistency in processing an AST, it is better to
	// have a null node than to have gaps in the AST or null pointers.
	Node* MakeNullNode();


	// Runs the callback on every single child node recursively, so from all children to all childrens children etc.
	void DoForAllChildren(Node* parent, void(*callback)(Node*, void*), void* args);
}