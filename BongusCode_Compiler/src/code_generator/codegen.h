#pragma once
#include <string>

namespace AST
{
	class Node;
}


void GenerateCode(AST::Node* nodeHead, std::string& outCode);