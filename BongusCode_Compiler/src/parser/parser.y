%language "c++"
%require "3.0"
%skeleton "lalr1.cc"
%output "parser.cpp"

%code requires
{
	namespace yy
	{
		class Lexer;
	}

	namespace AST
	{
		class Node;
	}

	enum class PrimitiveType : unsigned short;
}

%defines
%locations

%parse-param { yy::Lexer& lexer } // Construct parser object with lexer.

%define parse.trace

%code
{
	#include <stdio.h>
	#include "../lexer/lexer.h"
	#include "../AST/ASTNode.h"
	#include "../AST/ASTAPI.h"
	#include "../BongusTable.h"
	#include "../Exit.h"

	#undef yylex
	#define yylex lexer.lex  // Within bison's parse() we should invoke lexer.lex(), not the global yylex()

	// Jank-ass temp global to store the head. TODO: Please fix.
	extern AST::Node* g_nodeHead;
}


%union {
	unsigned long long int num;
	// const wchar_t* str;
	std::wstring* str;
	AST::Node* ASTNode;
	PrimitiveType primtype;
}
%token <str> ID
%token <num> NUM_LIT

%token KWD_UI8
%token KWD_I8
%token KWD_UI16
%token KWD_I16
%token KWD_UI32
%token KWD_I32
%token KWD_UI64
%token KWD_I64

%token KWD_RETURN

%token EQ_OP
%token PLUS_OP
%token MINUS_OP
%token MUL_OP
%token DIV_OP

%token LPAREN
%token RPAREN

%token LCURLY
%token RCURLY


%token SEMI

%type<ASTNode> functions
%type<ASTNode> function
%type<ASTNode> functionHead
%type<ASTNode> scopes
%type<ASTNode> scope

%type<ASTNode> stmts
%type<ASTNode> stmt

%type<ASTNode> expr
%type<ASTNode> varDecl
%type<ASTNode> varAss
%type<ASTNode> returnOp

%type<ASTNode> addExpr
%type<ASTNode> mulExpr
%type<ASTNode> factor

%type<primtype> type

%%
// AST construction with semantic actions on page 259.

program: functions					{ g_nodeHead = AST::MakeNullNode(); g_nodeHead->AdoptChildren($1); }
	   ;

functions: functions function		{ $$ = $1->MakeSiblings($2); }
		 | function
		 ;

function: functionHead scope		{ $$ = $1->AdoptChildren($2); }
		;

functionHead

scopes: scopes scope				{ $$ = $1->MakeSiblings($2); }
	  | scope
	  ;

scope: LCURLY stmts RCURLY			{ $$ = AST::MakeScopeNode(); $$->AdoptChildren($2); }
	 ;

stmts: stmts stmt SEMI				{ $$ = $1->MakeSiblings($2); }
	 | stmt SEMI					{ $$ = $1; }
	 ;

stmt: expr							{ $$ = $1; }
	| varDecl						{ $$ = $1; }
	| varAss						{ $$ = $1; }
	| returnOp						{ $$ = $1; }
	;


// Mathematical expression --------------------------------------------------------------------				
expr: addExpr						{ $$ = $1; }
	;

addExpr: addExpr PLUS_OP mulExpr	{ $$ = AST::MakeOpNode(L'+', $1, $3); }
	   | addExpr MINUS_OP mulExpr	{ $$ = AST::MakeOpNode(L'-', $1, $3); }
	   | mulExpr
	   ;

mulExpr: mulExpr MUL_OP factor		{ $$ = AST::MakeOpNode(L'*', $1, $3); }
	   | mulExpr DIV_OP factor		{ $$ = AST::MakeOpNode(L'/', $1, $3); }
	   | factor
	   ;

factor: NUM_LIT						{ $$ = AST::MakeIntNode($1); }
	  | ID							{ $$ = AST::MakeSymNode($1); }
	  | LPAREN expr RPAREN			{ $$ = $2; }
	  ;
//!Mathematical expression --------------------------------------------------------------------


// Variable declaration -----------------------------------------------------------------------
varDecl: type ID					{ $$ = AST::MakeDeclNode($2, $1); }
	   ;

type: KWD_UI16						{ $$ = PrimitiveType::ui16; }
	| KWD_I16						{ $$ = PrimitiveType::i16;	}

	| KWD_UI32						{ $$ = PrimitiveType::ui32;	}
	| KWD_I32						{ $$ = PrimitiveType::i32;	}

	| KWD_UI64						{ $$ = PrimitiveType::ui64; }
	| KWD_I64						{ $$ = PrimitiveType::i64;	}
	;
//!Variable declaration -----------------------------------------------------------------------


// Variable assignment ------------------------------------------------------------------------
varAss: ID EQ_OP expr				{ $$ = AST::MakeAssNode(AST::MakeSymNode($1) /* <--- Hurr durr */, $3); }
	  ;
//!Variable assignment ------------------------------------------------------------------------


// Return operation ---------------------------------------------------------------------------
returnOp: KWD_RETURN expr			{ $$ = AST::MakeReturnNode($2); }
		;
//!Return operation ---------------------------------------------------------------------------

%%


void yy::parser::error(const location_type& loc, const std::string& msg)
{
	std::cerr << "ERROR: " << msg << " at " << loc << std::endl;
	Exit(ErrCodes::syntax_error);
}