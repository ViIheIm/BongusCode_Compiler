#pragma once


/*
	Page 280:
	We walk(make a pass over) the AST for two purposes:
	● To process symbol declarations and
	● to connect each symbol reference with its declaration.
*/

namespace AST
{
	class Node;

	void BuildSymbolTable(AST::Node* nodeHead);
}