// Refer to cdefs.l and ctokens.l in REflex's examples folder.

%top{
#include "../parser/parser.hpp"
using BTok = yy::parser::token::token_kind_type;

#include "../BuildSettings.h"
#if LEXER_LOGGING == 1
#define LEXLOG(s, ...) wprintf(s, __VA_ARGS__)
#else
#define LEXLOG(s, ...)
#endif
}

%option unicode


%option matcher=reflex
%option nodefault
%option fast


COMMENT			"||"[^\n]*
WHITESPACE		[[:space:]]+

KWD_NIHIL		"nihil"
SYM_PTR			¤
KWD_UI8			"ui8"
KWD_I8			"i8"
KWD_UI16		"ui16"
KWD_I16			"i16"
KWD_UI32		"ui32"
KWD_I32			"i32"
KWD_UI64		"ui64"
KWD_I64			"i64"

KWD_RETURN		"Claudere"

ID				[[:alpha:]_][[:alnum:]_]*
NUM_LIT			0|[1-9][0-9]*

EQOP			=
PLUSOP			\+
MINUSOP			\-
MULOP			\*
DIVOP			\/

LPAREN			\(
RPAREN			\)

LCURLY			\{
RCURLY			\}

SEMI			\.
COMMA			\,

%%

{COMMENT}
{WHITESPACE}

{KWD_NIHIL}
	LEXLOG(L"Found KWD_NIHIL: %s\n", wstr().c_str());
	return BTok::KWD_NIHIL;

{SYM_PTR}
	LEXLOG(L"Found SYM_PTR: %s\n", wstr().c_str());
	return BTok::SYM_PTR;

{KWD_UI8}
	LEXLOG(L"Found KWD_UI8: %s\n", wstr().c_str());
	return BTok::KWD_UI8;

{KWD_I8}
	LEXLOG(L"Found KWD_I8: %s\n", wstr().c_str());
	return BTok::KWD_I8;

{KWD_UI16}
	LEXLOG(L"Found KWD_UI16: %s\n", wstr().c_str());
	return BTok::KWD_UI16;

{KWD_I16}
	LEXLOG(L"Found KWD_I16: %s\n", wstr().c_str());
	return BTok::KWD_I16;

{KWD_UI32}
	LEXLOG(L"Found KWD_UI32: %s\n", wstr().c_str());
	return BTok::KWD_UI32;

{KWD_I32}
	LEXLOG(L"Found KWD_I32: %s\n", wstr().c_str());
	return BTok::KWD_I32;

{KWD_UI64}
	LEXLOG(L"Found KWD_UI64: %s\n", wstr().c_str());
	return BTok::KWD_UI64;

{KWD_I64}
	LEXLOG(L"Found KWD_I64: %s\n", wstr().c_str());
	return BTok::KWD_I64;

{KWD_RETURN}
	LEXLOG(L"Found KWD_RETURN: %s\n", wstr().c_str());
	return BTok::KWD_RETURN;

{ID}
	LEXLOG(L"Found ID: %s\n", wstr().c_str());

	// TODO: Ugly and jank heap allocation to ensure that the string lives long enough for the parser to get it. Please fix man.
	// Memory leak is handled in MakeNode(), make sure to modify that aswell when you find something more elegant than this.
	yylval.str = new std::wstring(wstr());

	return BTok::ID;

{NUM_LIT}
	LEXLOG(L"Found NUM_LIT: %s\n", wstr().c_str());

	// Set yylval to integer value wrought from the input string.
	yylval.num = std::stoll(wstr());

	return BTok::NUM_LIT;


{EQOP}
	LEXLOG(L"Found EQ_OP: %s\n", wstr().c_str());
	return BTok::EQ_OP;

{PLUSOP}
	LEXLOG(L"Found PLUS_OP: %s\n", wstr().c_str());
	return BTok::PLUS_OP;

{MINUSOP}
	LEXLOG(L"Found MINUS_OP: %s\n", wstr().c_str());
	return BTok::MINUS_OP;

{MULOP}
	LEXLOG(L"Found MUL_OP: %s\n", wstr().c_str());
	return BTok::MUL_OP;

{DIVOP}
	LEXLOG(L"Found DIV_OP: %s\n", wstr().c_str());
	return BTok::DIV_OP;

{LPAREN}
	LEXLOG(L"Found LPAREN: %s\n", wstr().c_str());
	return BTok::LPAREN;

{RPAREN}
	LEXLOG(L"Found RPAREN: %s\n", wstr().c_str());
	return BTok::RPAREN;

{LCURLY}
	LEXLOG(L"Found LCURLY: %s\n", wstr().c_str());
	return BTok::LCURLY;

{RCURLY}
	LEXLOG(L"Found RCURLY: %s\n", wstr().c_str());
	return BTok::RCURLY;

{SEMI}
	LEXLOG(L"Found SEMI: %s\n", wstr().c_str());
	return BTok::SEMI;

{COMMA}
	LEXLOG(L"Found COMMA: %s\n", wstr().c_str());
	return BTok::COMMA;

%%