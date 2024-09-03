#pragma once
#include "../Definitions.h"
#include "../BongusTable.h"
#include "ASTAPI.h"
#include <vector>

class NodeVisitor;

// Node kind.
enum class Node_k : ui16
{
	Node,
	IntNode,
	SymNode,
	OpNode,
	AssNode,
	ScopeNode,
	DeclNode,
	ReturnNode
};

namespace AST
{
	// A disaster handle which all nodes register themselves at.
	// If we encounter compromising memory leaks or maybe want to print all nodes easily
	// we can go through this list and be sure that we find every single node in the AST.
	inline std::vector<Node*> g_disasterHandle;

	// Page 251 illustrates how to design ASTs.
	class Node
	{
	public:
	
		Node(Node* rSibling = nullptr, Node* lmostChild = nullptr, Node* parent = nullptr);
		virtual ~Node();
	
		// Page 253.
		Node* MakeSiblings(Node* y);
		// Page 253.
		void AdoptChildren(Node* y);

		// Because some nodes, like OpNodes for instance, have more children than the 1 mandated
		// by this base class(OpNodes for instance again has 2 additional children, being lhs and rhs)
		// we need a GetChildren to be a virtual method.
		virtual std::vector<Node*> GetChildren(void);

		inline const Node_k GetNodeKind(void) const { return kind; }

		friend Node* MakeNullNode();
		friend void DoForAllChildren(Node*, void(*)(Node*, void*), void*);

	protected:
	
		// Each node points to its next (right) sibling forming a singly-linked list of siblings.
		Node* rSibling;
	
		// To get access to a list head in constant time, each node also points to its leftmost sibling.
		Node* lmostSibling;
	
		// Each node n points to its leftmost child, which forms the beginning of the list of n's children.
		Node* lmostChild;
	
		// Each node points to its parent.
		Node* parent;


		// Get clean RTTI with a kind enum.
		Node_k kind;
	};


	// Represents raw literals.
	class IntNode : public Node
	{
	public:

		IntNode() = default;
		virtual ~IntNode() override = default;
		inline const ui64 Get(void) const { return n; }
		constexpr ui16 GetSize(void) const { return 8; }	// TODO: This size might need to be readjusted later.
		friend Node* MakeIntNode(i32);

	private:

		ui64 n;
	};


	// Represents IDs like variables.
	class SymNode : public Node
	{
	public:

		SymNode() = default;
		virtual ~SymNode() override = default;
		inline const std::wstring& GetName(void) const { return c; }
		friend Node* MakeSymNode(std::wstring*);

	private:

		std::wstring c;
	};


	// Represents an operation like addition or subtraction.
	class OpNode : public Node
	{
	public:

		OpNode() = default;
		// OpNode needs a destructor for the lhs and rhs.
		virtual ~OpNode() override;
		virtual std::vector<Node*> GetChildren(void) override;
		inline Node* GetLHS(void) const { return lhs; }
		inline Node* GetRHS(void) const { return rhs; }
		inline const wchar_t GetOp(void) const { return op; }
		
		// Overload that returns a narrow string version.
		inline std::string GetOpAsString(void) const
		{
			char buf[2] = { 0, 0 };
			buf[0] = (char)op;

			return std::string(buf);
		}

		friend Node* MakeOpNode(wchar_t, Node*, Node*);

	private:

		Node* lhs;
		Node* rhs;
		wchar_t op;
	};


	// Represents assignments to variables.
	class AssNode : public Node
	{
	public:

		AssNode() = default;
		virtual ~AssNode() override;
		virtual std::vector<Node*> GetChildren(void) override;
		inline Node* GetVar(void) const { return var; }
		inline Node* GetExpr(void) const { return expr; }
		friend Node* MakeAssNode(Node*, Node*);

	private:

		Node* var;
		Node* expr;
	};


	// Represents a scope, like an anonymous scope or a function scope created with curly braces in C.
	// Equivalent to a null node for now.
	class ScopeNode : public Node
	{
	public:

		ScopeNode() = default;
		virtual ~ScopeNode() = default;
		friend Node* MakeScopeNode();
	};


	// Represents only the declaration of a variable.
	class DeclNode : public Node
	{
	public:

		DeclNode() = default;
		virtual ~DeclNode() override = default;
		inline const std::wstring& GetName(void) const { return c; }
		inline const PrimitiveType GetType(void) const { return t; }
		inline const i16 GetSize(void) const { return size; }
		inline const i16 GetScopeDepth(void) const { return scopeDepth; }
		inline void SetScopeDepth(const i16 depth) { scopeDepth = depth; }
		friend Node* MakeDeclNode(std::wstring*, PrimitiveType);

	private:

		std::wstring c;
		PrimitiveType t;
		i16 size;
		i16 scopeDepth;
	};

	// Represents a return operation.
	class ReturnNode : public Node
	{
	public:

		ReturnNode() = default;
		virtual ~ReturnNode() override;
		virtual std::vector<Node*> GetChildren(void) override;
		inline Node* GetRetExpr(void) const { return retExpr; }
		friend Node* MakeReturnNode(Node*);

	private:

		// Should always be an opnode, but this is enforced(warned about if it is not adhered to) in the makenode-function.
		Node* retExpr;

	};
}