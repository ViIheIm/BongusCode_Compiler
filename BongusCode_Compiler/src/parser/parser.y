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

%token KWD_NIHIL
%token SYM_PTR
%token KWD_UI8
%token KWD_I8
%token KWD_UI16
%token KWD_I16
%token KWD_UI32
%token KWD_I32
%token KWD_UI64
%token KWD_I64

%token KWD_RETURN
%token KWD_FOR

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
%token RANGE_SYMBOL
%token COMMA

%token ADDR_OF_OP

%type<ASTNode> globalEntries
%type<ASTNode> globalEntry
%type<ASTNode> functions
%type<ASTNode> function
%type<ASTNode> functionHead
%type<ASTNode> paramList
%type<ASTNode> param
%type<ASTNode> fwdDecl

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

%type<ASTNode> functionCall
%type<ASTNode> argsList
%type<ASTNode> arg

%type<ASTNode> addrOfOp
%type<ASTNode> derefOp

%type<primtype> type

%type<ASTNode> forLoop
%type<ASTNode> forLoopHead

%type<ASTNode> value
%type<ASTNode> lvalue
%type<ASTNode> rvalue

%%
// AST construction with semantic actions on page 259.

// Functions & Fwd Decl-----------------------------------------------------------------------
program: globalEntries				{ g_nodeHead = AST::MakeNullNode(); g_nodeHead->AdoptChildren($1); }
			 ;

globalEntries: globalEntries globalEntry	{ $1->MakeSiblings($2); $$ = $1; }
			 | globalEntry
			 ;


globalEntry: function
		   | fwdDecl
		   ;

function: functionHead scope {
			$$ = $1;
			$1->AdoptChildren($2);

			// Add children to $2 through $1->GetArgsList()->rSibling ...
			AST::FunctionNode* asFuncNode = (AST::FunctionNode*)$1;
			AST::ArgNode* arg = (AST::ArgNode*)asFuncNode->GetArgsList();

			if (arg != nullptr)
			{
				std::wstring* str = new std::wstring(arg->GetName());
				AST::Node* declNode = AST::MakeDeclNode(str, arg->GetType(), arg->GetPointeeType());

				for (const AST::Node* n = arg->GetRightSibling(); n != nullptr; n = n->GetRightSibling())
				{
					AST::ArgNode* asArgNode = (AST::ArgNode*)n;

					// More whack string handovers.
					std::wstring* str = new std::wstring(asArgNode->GetName());

					// Create a new declnode and append it to the list by going through the head declNode.
					declNode->MakeSiblings(AST::MakeDeclNode(str, asArgNode->GetType(), asArgNode->GetPointeeType()));
				}


				// Now we need to swap the already generated nodes in the body and the newly added declNodes, because otherwise the new declNodes will
				// end up at the end of the function, and will thusly fall after the return statement on returning functions and raise an unreachable-code error.
				AST::Node* oldHead = $2->GetChildren()[0];
				declNode->MakeSiblings(oldHead);
				$2->UnbindChildren();
				$2->AdoptChildren(declNode);
			}
		}
		;

functionHead: type ID LPAREN paramList RPAREN		{ $$ = AST::MakeFunctionNode($1, $2, $4); }
						;

paramList: paramList COMMA param	{ $1->MakeSiblings($3); $$ = $1; }
		 | param
		 | KWD_NIHIL				{ $$ = nullptr; }
		 ;

param: type ID						{ $$ = AST::MakeArgNode($2, $1); }
		 | type SYM_PTR ID		{ $$ = AST::MakeArgNode($3, PrimitiveType::pointer, $1); }
		 ;


fwdDecl: type ID LPAREN paramList RPAREN SEMI		{ $$ = AST::MakeFwdDeclNode($1, $2, $4); }
			 ;
//!Functions & Fwd Decl-----------------------------------------------------------------------


scopes: scopes scope				{ $1->MakeSiblings($2); $$ = $1; }
	  | scope
	  ;

scope: LCURLY stmts RCURLY			{ $$ = AST::MakeScopeNode(); $$->AdoptChildren($2); }
		 | LCURLY RCURLY						{ $$ = AST::MakeScopeNode(); $$->AdoptChildren(AST::MakeNullNode()); }
		 ;

stmts: stmts stmt SEMI				{ $$ = $1->MakeSiblings($2); }
	 | stmt SEMI					{ $$ = $1; }
	 ;

stmt: expr							{ $$ = $1; }
		| varDecl						{ $$ = $1; }
		| varAss						{ $$ = $1; }
		| returnOp					{ $$ = $1; }
		| forLoop						{ $$ = $1; }
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

factor: NUM_LIT								{ $$ = AST::MakeIntNode($1); }
			| ID										{ $$ = AST::MakeSymNode($1); }
			| LPAREN expr RPAREN		{ $$ = $2; }
			| functionCall					{ $$ = $1; }
			| addrOfOp							{ $$ = $1; }
			| derefOp								{ $$ = $1; }
			;
//!Mathematical expression --------------------------------------------------------------------


// Variable declaration -----------------------------------------------------------------------
varDecl: type ID					{ $$ = AST::MakeDeclNode($2, $1); }
			 | type SYM_PTR ID	{ $$ = AST::MakeDeclNode($3, PrimitiveType::pointer, $1); }
			 ;

type: KWD_UI16						{ $$ = PrimitiveType::ui16; }
		| KWD_I16							{ $$ = PrimitiveType::i16;	}

		| KWD_UI32						{ $$ = PrimitiveType::ui32;	}
		| KWD_I32							{ $$ = PrimitiveType::i32;	}

		| KWD_UI64						{ $$ = PrimitiveType::ui64; }
		| KWD_I64							{ $$ = PrimitiveType::i64;	}

		| KWD_NIHIL						{ $$ = PrimitiveType::nihil; }
		;
//!Variable declaration -----------------------------------------------------------------------


// Variable assignment ------------------------------------------------------------------------
varAss: lvalue EQ_OP expr				{ $$ = AST::MakeAssNode($1, $3); }
			;
//!Variable assignment ------------------------------------------------------------------------


// Return operation ---------------------------------------------------------------------------
returnOp: KWD_RETURN expr			{ $$ = AST::MakeReturnNode($2); }
		;
//!Return operation ---------------------------------------------------------------------------


// For loop -----------------------------------------------------------------------------------
forLoop:	forLoopHead scope		{ $$ = AST::MakeForLoopNode($1, $2); }
			 ;

forLoopHead:	KWD_FOR LPAREN value RANGE_SYMBOL value RPAREN { $$ = AST::MakeForLoopHeadNode($5, $3); }
					 ;
//!For loop -----------------------------------------------------------------------------------

// Function call ------------------------------------------------------------------------------
functionCall: ID LPAREN argsList RPAREN	{ $$ = AST::MakeFunctionCallNode($1, $3); }
						;

argsList: argsList COMMA arg			{ $1->MakeSiblings($3); $$ = $1; }
		 | arg							{ $$ = $1; }
		 | %empty						{ $$ = nullptr; }
		 ;

arg: expr								{ $$ = $1; }
	 ;
//!Function call ------------------------------------------------------------------------------


// Address of ---------------------------------------------------------------------------------
addrOfOp:	ADDR_OF_OP ID { $$ = AST::MakeAddrOfNode($2); }
				;
//!Address of ---------------------------------------------------------------------------------


// Dereference --------------------------------------------------------------------------------
derefOp: SYM_PTR expr		{ $$ = AST::MakeDerefNode($2); }
			 ;
//!Dereference --------------------------------------------------------------------------------


// Value categories ---------------------------------------------------------------------------
value: lvalue
		 | rvalue
		 ;

lvalue: ID			{ $$ = AST::MakeSymNode($1); }
			| derefOp
			;

rvalue: NUM_LIT	{ $$ = AST::MakeIntNode($1); }
			;
//!Value categories ---------------------------------------------------------------------------

%%


void yy::parser::error(const location_type& loc, const std::string& msg)
{
	std::cerr << "ERROR: " << msg << " at " << loc << std::endl;
	Exit(ErrCodes::syntax_error);
}