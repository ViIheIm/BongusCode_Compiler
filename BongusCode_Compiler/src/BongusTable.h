#pragma once
#include "Definitions.h"

// Deprecated in favour of enum in the parser.
enum Tok : i32
{
	End,
	Invalid,
	Preproc,
	Identifier,

	// Literal tokens.
	OpenParen,
	CloseParen,
	OpenCurly,
	CloseCurly,
	Semicolon,
	Comma,

	// Type keywords.
	Kwd_i8,
	Kwd_ui8,
	Kwd_i16,
	Kwd_ui16,
	Kwd_i32,
	Kwd_ui32,
	Kwd_i64,
	Kwd_ui64,
	StringLiteral,
	IntegerLiteral,
	FloatLiteral,

	// Operators
	AssignmentOperator,
	MinusOperator,
	PlusOperator,
	MultiplyOperator,
	DereferenceOperator,
	GreaterThanOrEqualToOperator
};

/*
enum class PrimitiveType : ui16
{
	invalid,

	nihil,

	ui8,
	i8,

	ui16,
	i16,

	ui32,
	i32,

	ui64,
	i64
};

inline const char* PrimitiveTypeReflectionNarrow[] = {
	"invalid",

	"nihil",

	"ui8",
	"i8",

	"ui16",
	"i16",

	"ui32",
	"i32",

	"ui64",
	"i64"
};

inline const wchar_t* PrimitiveTypeReflectionWide[] = {
	L"invalid",
	
	L"nihil",

	L"ui8",
	L"i8",
	
	L"ui16",
	L"i16",
	
	L"ui32",
	L"i32",
	
	L"ui64",
	L"i64"
};
*/


#pragma region PrimitiveTypesList
#define LIST(X) X(invalid) X(nihil) X(pointer) X(ui8) X(i8) X(ui16) X(i16) X(ui32) X(i32) X(ui64) X(i64)
#pragma endregion

#define X(val) val,
	
enum class PrimitiveType : ui16
{
	LIST(X)
};

#undef X
#define X(val) #val,

inline const char* PrimitiveTypeReflectionNarrow[] = {
	LIST(X)
};

#undef X
#define X(val) L#val,

inline const wchar_t* PrimitiveTypeReflectionWide[] = {
	LIST(X)
};


// The default name of the main function. When generating this function, it must be swapped out for the unmangled "main" for the linker to catch on.
inline const char* NarrowMainFunctionName = "Viviscere";
inline const wchar_t* WideMainFunctionName = L"Viviscere";

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
	ReturnNode,
	FunctionNode,
	ArgNode,
	FunctionCallNode,
	FwdDeclNode,
	AddrOfNode,
	DerefNode,
	ForLoopNode,
	ForLoopHeadNode,
};
