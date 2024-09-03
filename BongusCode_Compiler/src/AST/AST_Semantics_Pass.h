#pragma once

namespace AST
{
	class Node;

	// We perform a semantics pass to enforce semantics like the return operation not obscuring more code, making it unreachable.
	void SemanticsPass(AST::Node* nodeHead);
}