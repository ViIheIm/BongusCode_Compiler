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

enum class PrimitiveType : ui16
{
	invalid,

	ui8,
	i8,

	ui16,
	i16,

	ui32,
	i32,

	ui64,
	i64
};