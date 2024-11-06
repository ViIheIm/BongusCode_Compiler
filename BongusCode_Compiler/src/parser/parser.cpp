// A Bison parser, made by GNU Bison 3.7.4.

// Skeleton implementation for Bison LALR(1) parsers in C++

// Copyright (C) 2002-2015, 2018-2020 Free Software Foundation, Inc.

// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

// As a special exception, you may create a larger work that contains
// part or all of the Bison parser skeleton and distribute that work
// under terms of your choice, so long as that work isn't itself a
// parser generator using the skeleton or a modified version thereof
// as a parser skeleton.  Alternatively, if you modify or redistribute
// the parser skeleton itself, you may (at your option) remove this
// special exception, which will cause the skeleton and the resulting
// Bison output files to be licensed under the GNU General Public
// License without this special exception.

// This special exception was added by the Free Software Foundation in
// version 2.2 of Bison.

// DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
// especially those whose name start with YY_ or yy_.  They are
// private implementation details that can be changed or removed.





#include "parser.hpp"


// Unqualified %code blocks.
#line 29 "parser.y"

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

#line 61 "parser.cpp"


#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> // FIXME: INFRINGES ON USER NAME SPACE.
#   define YY_(msgid) dgettext ("bison-runtime", msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(msgid) msgid
# endif
#endif


// Whether we are compiled with exception support.
#ifndef YY_EXCEPTIONS
# if defined __GNUC__ && !defined __EXCEPTIONS
#  define YY_EXCEPTIONS 0
# else
#  define YY_EXCEPTIONS 1
# endif
#endif

#define YYRHSLOC(Rhs, K) ((Rhs)[K].location)
/* YYLLOC_DEFAULT -- Set CURRENT to span from RHS[1] to RHS[N].
   If N is 0, then set CURRENT to the empty location which ends
   the previous symbol: RHS[0] (always defined).  */

# ifndef YYLLOC_DEFAULT
#  define YYLLOC_DEFAULT(Current, Rhs, N)                               \
    do                                                                  \
      if (N)                                                            \
        {                                                               \
          (Current).begin  = YYRHSLOC (Rhs, 1).begin;                   \
          (Current).end    = YYRHSLOC (Rhs, N).end;                     \
        }                                                               \
      else                                                              \
        {                                                               \
          (Current).begin = (Current).end = YYRHSLOC (Rhs, 0).end;      \
        }                                                               \
    while (false)
# endif


// Enable debugging if requested.
#if YYDEBUG

// A pseudo ostream that takes yydebug_ into account.
# define YYCDEBUG if (yydebug_) (*yycdebug_)

# define YY_SYMBOL_PRINT(Title, Symbol)         \
  do {                                          \
    if (yydebug_)                               \
    {                                           \
      *yycdebug_ << Title << ' ';               \
      yy_print_ (*yycdebug_, Symbol);           \
      *yycdebug_ << '\n';                       \
    }                                           \
  } while (false)

# define YY_REDUCE_PRINT(Rule)          \
  do {                                  \
    if (yydebug_)                       \
      yy_reduce_print_ (Rule);          \
  } while (false)

# define YY_STACK_PRINT()               \
  do {                                  \
    if (yydebug_)                       \
      yy_stack_print_ ();                \
  } while (false)

#else // !YYDEBUG

# define YYCDEBUG if (false) std::cerr
# define YY_SYMBOL_PRINT(Title, Symbol)  YYUSE (Symbol)
# define YY_REDUCE_PRINT(Rule)           static_cast<void> (0)
# define YY_STACK_PRINT()                static_cast<void> (0)

#endif // !YYDEBUG

#define yyerrok         (yyerrstatus_ = 0)
#define yyclearin       (yyla.clear ())

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab
#define YYRECOVERING()  (!!yyerrstatus_)

namespace yy {
#line 153 "parser.cpp"

  /// Build a parser object.
  parser::parser (yy::Lexer& lexer_yyarg)
#if YYDEBUG
    : yydebug_ (false),
      yycdebug_ (&std::cerr),
#else
    :
#endif
      lexer (lexer_yyarg)
  {}

  parser::~parser ()
  {}

  parser::syntax_error::~syntax_error () YY_NOEXCEPT YY_NOTHROW
  {}

  /*---------------.
  | symbol kinds.  |
  `---------------*/

  // basic_symbol.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (const basic_symbol& that)
    : Base (that)
    , value (that.value)
    , location (that.location)
  {}


  /// Constructor for valueless symbols.
  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_MOVE_REF (location_type) l)
    : Base (t)
    , value ()
    , location (l)
  {}

  template <typename Base>
  parser::basic_symbol<Base>::basic_symbol (typename Base::kind_type t, YY_RVREF (semantic_type) v, YY_RVREF (location_type) l)
    : Base (t)
    , value (YY_MOVE (v))
    , location (YY_MOVE (l))
  {}

  template <typename Base>
  parser::symbol_kind_type
  parser::basic_symbol<Base>::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }

  template <typename Base>
  bool
  parser::basic_symbol<Base>::empty () const YY_NOEXCEPT
  {
    return this->kind () == symbol_kind::S_YYEMPTY;
  }

  template <typename Base>
  void
  parser::basic_symbol<Base>::move (basic_symbol& s)
  {
    super_type::move (s);
    value = YY_MOVE (s.value);
    location = YY_MOVE (s.location);
  }

  // by_kind.
  parser::by_kind::by_kind ()
    : kind_ (symbol_kind::S_YYEMPTY)
  {}

#if 201103L <= YY_CPLUSPLUS
  parser::by_kind::by_kind (by_kind&& that)
    : kind_ (that.kind_)
  {
    that.clear ();
  }
#endif

  parser::by_kind::by_kind (const by_kind& that)
    : kind_ (that.kind_)
  {}

  parser::by_kind::by_kind (token_kind_type t)
    : kind_ (yytranslate_ (t))
  {}

  void
  parser::by_kind::clear ()
  {
    kind_ = symbol_kind::S_YYEMPTY;
  }

  void
  parser::by_kind::move (by_kind& that)
  {
    kind_ = that.kind_;
    that.clear ();
  }

  parser::symbol_kind_type
  parser::by_kind::kind () const YY_NOEXCEPT
  {
    return kind_;
  }

  parser::symbol_kind_type
  parser::by_kind::type_get () const YY_NOEXCEPT
  {
    return this->kind ();
  }


  // by_state.
  parser::by_state::by_state () YY_NOEXCEPT
    : state (empty_state)
  {}

  parser::by_state::by_state (const by_state& that) YY_NOEXCEPT
    : state (that.state)
  {}

  void
  parser::by_state::clear () YY_NOEXCEPT
  {
    state = empty_state;
  }

  void
  parser::by_state::move (by_state& that)
  {
    state = that.state;
    that.clear ();
  }

  parser::by_state::by_state (state_type s) YY_NOEXCEPT
    : state (s)
  {}

  parser::symbol_kind_type
  parser::by_state::kind () const YY_NOEXCEPT
  {
    if (state == empty_state)
      return symbol_kind::S_YYEMPTY;
    else
      return YY_CAST (symbol_kind_type, yystos_[+state]);
  }

  parser::stack_symbol_type::stack_symbol_type ()
  {}

  parser::stack_symbol_type::stack_symbol_type (YY_RVREF (stack_symbol_type) that)
    : super_type (YY_MOVE (that.state), YY_MOVE (that.value), YY_MOVE (that.location))
  {
#if 201103L <= YY_CPLUSPLUS
    // that is emptied.
    that.state = empty_state;
#endif
  }

  parser::stack_symbol_type::stack_symbol_type (state_type s, YY_MOVE_REF (symbol_type) that)
    : super_type (s, YY_MOVE (that.value), YY_MOVE (that.location))
  {
    // that is emptied.
    that.kind_ = symbol_kind::S_YYEMPTY;
  }

#if YY_CPLUSPLUS < 201103L
  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (const stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    return *this;
  }

  parser::stack_symbol_type&
  parser::stack_symbol_type::operator= (stack_symbol_type& that)
  {
    state = that.state;
    value = that.value;
    location = that.location;
    // that is emptied.
    that.state = empty_state;
    return *this;
  }
#endif

  template <typename Base>
  void
  parser::yy_destroy_ (const char* yymsg, basic_symbol<Base>& yysym) const
  {
    if (yymsg)
      YY_SYMBOL_PRINT (yymsg, yysym);

    // User destructor.
    YYUSE (yysym.kind ());
  }

#if YYDEBUG
  template <typename Base>
  void
  parser::yy_print_ (std::ostream& yyo, const basic_symbol<Base>& yysym) const
  {
    std::ostream& yyoutput = yyo;
    YYUSE (yyoutput);
    if (yysym.empty ())
      yyo << "empty symbol";
    else
      {
        symbol_kind_type yykind = yysym.kind ();
        yyo << (yykind < YYNTOKENS ? "token" : "nterm")
            << ' ' << yysym.name () << " ("
            << yysym.location << ": ";
        YYUSE (yykind);
        yyo << ')';
      }
  }
#endif

  void
  parser::yypush_ (const char* m, YY_MOVE_REF (stack_symbol_type) sym)
  {
    if (m)
      YY_SYMBOL_PRINT (m, sym);
    yystack_.push (YY_MOVE (sym));
  }

  void
  parser::yypush_ (const char* m, state_type s, YY_MOVE_REF (symbol_type) sym)
  {
#if 201103L <= YY_CPLUSPLUS
    yypush_ (m, stack_symbol_type (s, std::move (sym)));
#else
    stack_symbol_type ss (s, sym);
    yypush_ (m, ss);
#endif
  }

  void
  parser::yypop_ (int n)
  {
    yystack_.pop (n);
  }

#if YYDEBUG
  std::ostream&
  parser::debug_stream () const
  {
    return *yycdebug_;
  }

  void
  parser::set_debug_stream (std::ostream& o)
  {
    yycdebug_ = &o;
  }


  parser::debug_level_type
  parser::debug_level () const
  {
    return yydebug_;
  }

  void
  parser::set_debug_level (debug_level_type l)
  {
    yydebug_ = l;
  }
#endif // YYDEBUG

  parser::state_type
  parser::yy_lr_goto_state_ (state_type yystate, int yysym)
  {
    int yyr = yypgoto_[yysym - YYNTOKENS] + yystate;
    if (0 <= yyr && yyr <= yylast_ && yycheck_[yyr] == yystate)
      return yytable_[yyr];
    else
      return yydefgoto_[yysym - YYNTOKENS];
  }

  bool
  parser::yy_pact_value_is_default_ (int yyvalue)
  {
    return yyvalue == yypact_ninf_;
  }

  bool
  parser::yy_table_value_is_error_ (int yyvalue)
  {
    return yyvalue == yytable_ninf_;
  }

  int
  parser::operator() ()
  {
    return parse ();
  }

  int
  parser::parse ()
  {
    int yyn;
    /// Length of the RHS of the rule being reduced.
    int yylen = 0;

    // Error handling.
    int yynerrs_ = 0;
    int yyerrstatus_ = 0;

    /// The lookahead symbol.
    symbol_type yyla;

    /// The locations where the error started and ended.
    stack_symbol_type yyerror_range[3];

    /// The return value of parse ().
    int yyresult;

#if YY_EXCEPTIONS
    try
#endif // YY_EXCEPTIONS
      {
    YYCDEBUG << "Starting parse\n";


    /* Initialize the stack.  The initial state will be set in
       yynewstate, since the latter expects the semantical and the
       location values to have been already stored, initialize these
       stacks with a primary value.  */
    yystack_.clear ();
    yypush_ (YY_NULLPTR, 0, YY_MOVE (yyla));

  /*-----------------------------------------------.
  | yynewstate -- push a new symbol on the stack.  |
  `-----------------------------------------------*/
  yynewstate:
    YYCDEBUG << "Entering state " << int (yystack_[0].state) << '\n';
    YY_STACK_PRINT ();

    // Accept?
    if (yystack_[0].state == yyfinal_)
      YYACCEPT;

    goto yybackup;


  /*-----------.
  | yybackup.  |
  `-----------*/
  yybackup:
    // Try to take a decision without lookahead.
    yyn = yypact_[+yystack_[0].state];
    if (yy_pact_value_is_default_ (yyn))
      goto yydefault;

    // Read a lookahead token.
    if (yyla.empty ())
      {
        YYCDEBUG << "Reading a token\n";
#if YY_EXCEPTIONS
        try
#endif // YY_EXCEPTIONS
          {
            yyla.kind_ = yytranslate_ (yylex (&yyla.value, &yyla.location));
          }
#if YY_EXCEPTIONS
        catch (const syntax_error& yyexc)
          {
            YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
            error (yyexc);
            goto yyerrlab1;
          }
#endif // YY_EXCEPTIONS
      }
    YY_SYMBOL_PRINT ("Next token is", yyla);

    if (yyla.kind () == symbol_kind::S_YYerror)
    {
      // The scanner already issued an error message, process directly
      // to error recovery.  But do not keep the error token as
      // lookahead, it is too special and may lead us to an endless
      // loop in error recovery. */
      yyla.kind_ = symbol_kind::S_YYUNDEF;
      goto yyerrlab1;
    }

    /* If the proper action on seeing token YYLA.TYPE is to reduce or
       to detect an error, take that action.  */
    yyn += yyla.kind ();
    if (yyn < 0 || yylast_ < yyn || yycheck_[yyn] != yyla.kind ())
      {
        goto yydefault;
      }

    // Reduce or error.
    yyn = yytable_[yyn];
    if (yyn <= 0)
      {
        if (yy_table_value_is_error_ (yyn))
          goto yyerrlab;
        yyn = -yyn;
        goto yyreduce;
      }

    // Count tokens shifted since error; after three, turn off error status.
    if (yyerrstatus_)
      --yyerrstatus_;

    // Shift the lookahead token.
    yypush_ ("Shifting", state_type (yyn), YY_MOVE (yyla));
    goto yynewstate;


  /*-----------------------------------------------------------.
  | yydefault -- do the default action for the current state.  |
  `-----------------------------------------------------------*/
  yydefault:
    yyn = yydefact_[+yystack_[0].state];
    if (yyn == 0)
      goto yyerrlab;
    goto yyreduce;


  /*-----------------------------.
  | yyreduce -- do a reduction.  |
  `-----------------------------*/
  yyreduce:
    yylen = yyr2_[yyn];
    {
      stack_symbol_type yylhs;
      yylhs.state = yy_lr_goto_state_ (yystack_[yylen].state, yyr1_[yyn]);
      /* If YYLEN is nonzero, implement the default value of the
         action: '$$ = $1'.  Otherwise, use the top of the stack.

         Otherwise, the following line sets YYLHS.VALUE to garbage.
         This behavior is undocumented and Bison users should not rely
         upon it.  */
      if (yylen)
        yylhs.value = yystack_[yylen - 1].value;
      else
        yylhs.value = yystack_[0].value;

      // Default location.
      {
        stack_type::slice range (yystack_, yylen);
        YYLLOC_DEFAULT (yylhs.location, range, yylen);
        yyerror_range[1].location = yylhs.location;
      }

      // Perform the reduction.
      YY_REDUCE_PRINT (yyn);
#if YY_EXCEPTIONS
      try
#endif // YY_EXCEPTIONS
        {
          switch (yyn)
            {
  case 2: // program: globalEntries
#line 122 "parser.y"
                                                { g_nodeHead = AST::MakeNullNode(); g_nodeHead->AdoptChildren((yystack_[0].value.ASTNode)); }
#line 620 "parser.cpp"
    break;

  case 3: // globalEntries: globalEntries globalEntry
#line 125 "parser.y"
                                                { (yystack_[1].value.ASTNode)->MakeSiblings((yystack_[0].value.ASTNode)); (yylhs.value.ASTNode) = (yystack_[1].value.ASTNode); }
#line 626 "parser.cpp"
    break;

  case 4: // globalEntries: globalEntry
#line 126 "parser.y"
                           { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 632 "parser.cpp"
    break;

  case 5: // globalEntry: function
#line 130 "parser.y"
             { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 638 "parser.cpp"
    break;

  case 6: // globalEntry: fwdDecl
#line 131 "parser.y"
                     { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 644 "parser.cpp"
    break;

  case 7: // function: functionHead scope
#line 134 "parser.y"
                             {
			(yylhs.value.ASTNode) = (yystack_[1].value.ASTNode);
			(yystack_[1].value.ASTNode)->AdoptChildren((yystack_[0].value.ASTNode));

			// Add children to $2 through $1->GetArgsList()->rSibling ...
			AST::FunctionNode* asFuncNode = (AST::FunctionNode*)(yystack_[1].value.ASTNode);
			AST::ArgNode* arg = (AST::ArgNode*)asFuncNode->GetArgsList();

			if (arg != nullptr)
			{
				std::wstring* str = new std::wstring(arg->GetName());
				AST::Node* declNode = AST::MakeDeclNode(str, arg->GetType());

				for (const AST::Node* n = arg->GetRightSibling(); n != nullptr; n = n->GetRightSibling())
				{
					AST::ArgNode* asArgNode = (AST::ArgNode*)n;

					// More whack string handovers.
					std::wstring* str = new std::wstring(asArgNode->GetName());

					// Create a new declnode and append it to the list by going through the head declNode.
					declNode->MakeSiblings(AST::MakeDeclNode(str, asArgNode->GetType()));
				}


				// Now we need to swap the already generated nodes in the body and the newly added declNodes, because otherwise the new declNodes will
				// end up at the end of the function, and will thusly fall after the return statement on returning functions and raise an unreachable-code error.
				AST::Node* oldHead = (yystack_[0].value.ASTNode)->GetChildren()[0];
				declNode->MakeSiblings(oldHead);
				(yystack_[0].value.ASTNode)->UnbindChildren();
				(yystack_[0].value.ASTNode)->AdoptChildren(declNode);
			}
		}
#line 682 "parser.cpp"
    break;

  case 8: // functionHead: type ID LPAREN paramList RPAREN
#line 169 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeFunctionNode((yystack_[4].value.primtype), (yystack_[3].value.str), (yystack_[1].value.ASTNode)); }
#line 688 "parser.cpp"
    break;

  case 9: // paramList: paramList COMMA param
#line 172 "parser.y"
                                        { (yystack_[2].value.ASTNode)->MakeSiblings((yystack_[0].value.ASTNode)); (yylhs.value.ASTNode) = (yystack_[2].value.ASTNode); }
#line 694 "parser.cpp"
    break;

  case 10: // paramList: param
#line 173 "parser.y"
                   { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 700 "parser.cpp"
    break;

  case 11: // paramList: KWD_NIHIL
#line 174 "parser.y"
                                                        { (yylhs.value.ASTNode) = nullptr; }
#line 706 "parser.cpp"
    break;

  case 12: // param: type ID
#line 177 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeArgNode((yystack_[0].value.str), (yystack_[1].value.primtype)); }
#line 712 "parser.cpp"
    break;

  case 13: // param: type SYM_PTR ID
#line 178 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeArgNode((yystack_[0].value.str), PrimitiveType::pointer, (yystack_[2].value.primtype)); }
#line 718 "parser.cpp"
    break;

  case 14: // fwdDecl: type ID LPAREN paramList RPAREN SEMI
#line 182 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeFwdDeclNode((yystack_[5].value.primtype), (yystack_[4].value.str), (yystack_[2].value.ASTNode)); }
#line 724 "parser.cpp"
    break;

  case 15: // scope: LCURLY stmts RCURLY
#line 191 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeScopeNode(); (yylhs.value.ASTNode)->AdoptChildren((yystack_[1].value.ASTNode)); }
#line 730 "parser.cpp"
    break;

  case 16: // stmts: stmts stmt SEMI
#line 194 "parser.y"
                                                { (yylhs.value.ASTNode) = (yystack_[2].value.ASTNode)->MakeSiblings((yystack_[1].value.ASTNode)); }
#line 736 "parser.cpp"
    break;

  case 17: // stmts: stmt SEMI
#line 195 "parser.y"
                                                        { (yylhs.value.ASTNode) = (yystack_[1].value.ASTNode); }
#line 742 "parser.cpp"
    break;

  case 18: // stmt: expr
#line 198 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 748 "parser.cpp"
    break;

  case 19: // stmt: varDecl
#line 199 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 754 "parser.cpp"
    break;

  case 20: // stmt: varAss
#line 200 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 760 "parser.cpp"
    break;

  case 21: // stmt: returnOp
#line 201 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 766 "parser.cpp"
    break;

  case 22: // expr: addExpr
#line 206 "parser.y"
                                                        { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 772 "parser.cpp"
    break;

  case 23: // addExpr: addExpr PLUS_OP mulExpr
#line 209 "parser.y"
                                        { (yylhs.value.ASTNode) = AST::MakeOpNode(L'+', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 778 "parser.cpp"
    break;

  case 24: // addExpr: addExpr MINUS_OP mulExpr
#line 210 "parser.y"
                                        { (yylhs.value.ASTNode) = AST::MakeOpNode(L'-', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 784 "parser.cpp"
    break;

  case 25: // addExpr: mulExpr
#line 211 "parser.y"
             { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 790 "parser.cpp"
    break;

  case 26: // mulExpr: mulExpr MUL_OP factor
#line 214 "parser.y"
                                        { (yylhs.value.ASTNode) = AST::MakeOpNode(L'*', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 796 "parser.cpp"
    break;

  case 27: // mulExpr: mulExpr DIV_OP factor
#line 215 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeOpNode(L'/', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 802 "parser.cpp"
    break;

  case 28: // mulExpr: factor
#line 216 "parser.y"
             { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 808 "parser.cpp"
    break;

  case 29: // factor: NUM_LIT
#line 219 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeIntNode((yystack_[0].value.num)); }
#line 814 "parser.cpp"
    break;

  case 30: // factor: ID
#line 220 "parser.y"
                                                                { (yylhs.value.ASTNode) = AST::MakeSymNode((yystack_[0].value.str)); }
#line 820 "parser.cpp"
    break;

  case 31: // factor: LPAREN expr RPAREN
#line 221 "parser.y"
                                                { (yylhs.value.ASTNode) = (yystack_[1].value.ASTNode); }
#line 826 "parser.cpp"
    break;

  case 32: // factor: functionCall
#line 222 "parser.y"
                                                        { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 832 "parser.cpp"
    break;

  case 33: // factor: addrOfOp
#line 223 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 838 "parser.cpp"
    break;

  case 34: // factor: derefOp
#line 224 "parser.y"
                                                                        { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 844 "parser.cpp"
    break;

  case 35: // varDecl: type ID
#line 230 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeDeclNode((yystack_[0].value.str), (yystack_[1].value.primtype)); }
#line 850 "parser.cpp"
    break;

  case 36: // varDecl: type SYM_PTR ID
#line 231 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeDeclNode((yystack_[0].value.str), PrimitiveType::pointer, (yystack_[2].value.primtype)); }
#line 856 "parser.cpp"
    break;

  case 37: // type: KWD_UI16
#line 234 "parser.y"
                                                        { (yylhs.value.primtype) = PrimitiveType::ui16; }
#line 862 "parser.cpp"
    break;

  case 38: // type: KWD_I16
#line 235 "parser.y"
                                                                                { (yylhs.value.primtype) = PrimitiveType::i16;	}
#line 868 "parser.cpp"
    break;

  case 39: // type: KWD_UI32
#line 237 "parser.y"
                                                                        { (yylhs.value.primtype) = PrimitiveType::ui32;	}
#line 874 "parser.cpp"
    break;

  case 40: // type: KWD_I32
#line 238 "parser.y"
                                                                                { (yylhs.value.primtype) = PrimitiveType::i32;	}
#line 880 "parser.cpp"
    break;

  case 41: // type: KWD_UI64
#line 240 "parser.y"
                                                                        { (yylhs.value.primtype) = PrimitiveType::ui64; }
#line 886 "parser.cpp"
    break;

  case 42: // type: KWD_I64
#line 241 "parser.y"
                                                                                { (yylhs.value.primtype) = PrimitiveType::i64;	}
#line 892 "parser.cpp"
    break;

  case 43: // type: KWD_NIHIL
#line 243 "parser.y"
                                                                        { (yylhs.value.primtype) = PrimitiveType::nihil; }
#line 898 "parser.cpp"
    break;

  case 44: // varAss: ID EQ_OP expr
#line 249 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeAssNode(AST::MakeSymNode((yystack_[2].value.str)) /* <--- Hurr durr */, (yystack_[0].value.ASTNode)); }
#line 904 "parser.cpp"
    break;

  case 45: // returnOp: KWD_RETURN expr
#line 255 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeReturnNode((yystack_[0].value.ASTNode)); }
#line 910 "parser.cpp"
    break;

  case 46: // functionCall: ID LPAREN argsList RPAREN
#line 261 "parser.y"
                                        { (yylhs.value.ASTNode) = AST::MakeFunctionCallNode((yystack_[3].value.str), (yystack_[1].value.ASTNode)); }
#line 916 "parser.cpp"
    break;

  case 47: // argsList: argsList COMMA arg
#line 264 "parser.y"
                                                { (yystack_[2].value.ASTNode)->MakeSiblings((yystack_[0].value.ASTNode)); (yylhs.value.ASTNode) = (yystack_[2].value.ASTNode); }
#line 922 "parser.cpp"
    break;

  case 48: // argsList: arg
#line 265 "parser.y"
                                                                        { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 928 "parser.cpp"
    break;

  case 49: // argsList: %empty
#line 266 "parser.y"
                                                                        { (yylhs.value.ASTNode) = nullptr; }
#line 934 "parser.cpp"
    break;

  case 50: // arg: expr
#line 269 "parser.y"
                                                                        { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 940 "parser.cpp"
    break;

  case 51: // addrOfOp: ADDR_OF_OP ID
#line 275 "parser.y"
                              { (yylhs.value.ASTNode) = AST::MakeAddrOfNode((yystack_[0].value.str)); }
#line 946 "parser.cpp"
    break;

  case 52: // derefOp: SYM_PTR expr
#line 281 "parser.y"
                                { (yylhs.value.ASTNode) = AST::MakeDerefNode((yystack_[0].value.ASTNode)); }
#line 952 "parser.cpp"
    break;


#line 956 "parser.cpp"

            default:
              break;
            }
        }
#if YY_EXCEPTIONS
      catch (const syntax_error& yyexc)
        {
          YYCDEBUG << "Caught exception: " << yyexc.what() << '\n';
          error (yyexc);
          YYERROR;
        }
#endif // YY_EXCEPTIONS
      YY_SYMBOL_PRINT ("-> $$ =", yylhs);
      yypop_ (yylen);
      yylen = 0;

      // Shift the result of the reduction.
      yypush_ (YY_NULLPTR, YY_MOVE (yylhs));
    }
    goto yynewstate;


  /*--------------------------------------.
  | yyerrlab -- here on detecting error.  |
  `--------------------------------------*/
  yyerrlab:
    // If not already recovering from an error, report this error.
    if (!yyerrstatus_)
      {
        ++yynerrs_;
        std::string msg = YY_("syntax error");
        error (yyla.location, YY_MOVE (msg));
      }


    yyerror_range[1].location = yyla.location;
    if (yyerrstatus_ == 3)
      {
        /* If just tried and failed to reuse lookahead token after an
           error, discard it.  */

        // Return failure if at end of input.
        if (yyla.kind () == symbol_kind::S_YYEOF)
          YYABORT;
        else if (!yyla.empty ())
          {
            yy_destroy_ ("Error: discarding", yyla);
            yyla.clear ();
          }
      }

    // Else will try to reuse lookahead token after shifting the error token.
    goto yyerrlab1;


  /*---------------------------------------------------.
  | yyerrorlab -- error raised explicitly by YYERROR.  |
  `---------------------------------------------------*/
  yyerrorlab:
    /* Pacify compilers when the user code never invokes YYERROR and
       the label yyerrorlab therefore never appears in user code.  */
    if (false)
      YYERROR;

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYERROR.  */
    yypop_ (yylen);
    yylen = 0;
    YY_STACK_PRINT ();
    goto yyerrlab1;


  /*-------------------------------------------------------------.
  | yyerrlab1 -- common code for both syntax error and YYERROR.  |
  `-------------------------------------------------------------*/
  yyerrlab1:
    yyerrstatus_ = 3;   // Each real token shifted decrements this.
    // Pop stack until we find a state that shifts the error token.
    for (;;)
      {
        yyn = yypact_[+yystack_[0].state];
        if (!yy_pact_value_is_default_ (yyn))
          {
            yyn += symbol_kind::S_YYerror;
            if (0 <= yyn && yyn <= yylast_
                && yycheck_[yyn] == symbol_kind::S_YYerror)
              {
                yyn = yytable_[yyn];
                if (0 < yyn)
                  break;
              }
          }

        // Pop the current state because it cannot handle the error token.
        if (yystack_.size () == 1)
          YYABORT;

        yyerror_range[1].location = yystack_[0].location;
        yy_destroy_ ("Error: popping", yystack_[0]);
        yypop_ ();
        YY_STACK_PRINT ();
      }
    {
      stack_symbol_type error_token;

      yyerror_range[2].location = yyla.location;
      YYLLOC_DEFAULT (error_token.location, yyerror_range, 2);

      // Shift the error token.
      error_token.state = state_type (yyn);
      yypush_ ("Shifting", YY_MOVE (error_token));
    }
    goto yynewstate;


  /*-------------------------------------.
  | yyacceptlab -- YYACCEPT comes here.  |
  `-------------------------------------*/
  yyacceptlab:
    yyresult = 0;
    goto yyreturn;


  /*-----------------------------------.
  | yyabortlab -- YYABORT comes here.  |
  `-----------------------------------*/
  yyabortlab:
    yyresult = 1;
    goto yyreturn;


  /*-----------------------------------------------------.
  | yyreturn -- parsing is finished, return the result.  |
  `-----------------------------------------------------*/
  yyreturn:
    if (!yyla.empty ())
      yy_destroy_ ("Cleanup: discarding lookahead", yyla);

    /* Do not reclaim the symbols of the rule whose action triggered
       this YYABORT or YYACCEPT.  */
    yypop_ (yylen);
    YY_STACK_PRINT ();
    while (1 < yystack_.size ())
      {
        yy_destroy_ ("Cleanup: popping", yystack_[0]);
        yypop_ ();
      }

    return yyresult;
  }
#if YY_EXCEPTIONS
    catch (...)
      {
        YYCDEBUG << "Exception caught: cleaning lookahead and stack\n";
        // Do not try to display the values of the reclaimed symbols,
        // as their printers might throw an exception.
        if (!yyla.empty ())
          yy_destroy_ (YY_NULLPTR, yyla);

        while (1 < yystack_.size ())
          {
            yy_destroy_ (YY_NULLPTR, yystack_[0]);
            yypop_ ();
          }
        throw;
      }
#endif // YY_EXCEPTIONS
  }

  void
  parser::error (const syntax_error& yyexc)
  {
    error (yyexc.location, yyexc.what ());
  }

#if YYDEBUG || 0
  const char *
  parser::symbol_name (symbol_kind_type yysymbol)
  {
    return yytname_[yysymbol];
  }
#endif // #if YYDEBUG || 0





  const signed char parser::yypact_ninf_ = -21;

  const signed char parser::yytable_ninf_ = -44;

  const signed char
  parser::yypact_[] =
  {
      59,   -21,   -21,   -21,   -21,   -21,   -21,   -21,     1,    59,
     -21,   -21,   -13,   -21,    15,   -21,   -21,    27,   -21,     6,
       3,   -21,    40,    40,    40,    49,     2,    50,   -21,    17,
      39,   -21,   -21,    22,   -21,   -21,   -21,   -21,   -21,    69,
      40,    40,    55,   -21,   -21,    62,   -21,   -21,    52,   -21,
      40,    40,    40,    40,   -21,    82,    47,    23,   -21,    54,
     -21,   -21,    25,   -21,   -21,   -21,    39,    39,   -21,   -21,
     -21,    61,    59,   -21,    84,   -21,    40,   -21,   -21,   -21,
     -21
  };

  const signed char
  parser::yydefact_[] =
  {
       0,    43,    37,    38,    39,    40,    41,    42,     0,     2,
       4,     5,     0,     6,     0,     1,     3,     0,     7,     0,
      30,    29,     0,     0,     0,     0,     0,     0,    18,    22,
      25,    28,    19,     0,    20,    21,    32,    33,    34,     0,
       0,    49,    30,    52,    45,     0,    51,    15,     0,    17,
       0,     0,     0,     0,    35,     0,    11,     0,    10,     0,
      44,    50,     0,    48,    31,    16,    23,    24,    26,    27,
      36,     8,     0,    12,     0,    46,     0,    14,     9,    13,
      47
  };

  const signed char
  parser::yypgoto_[] =
  {
     -21,   -21,   -21,    79,   -21,   -21,   -21,    18,   -21,   -21,
     -21,    63,   -20,   -21,    12,    13,   -21,   -17,   -21,   -21,
     -21,   -21,    16,   -21,   -21
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     8,     9,    10,    11,    12,    57,    58,    13,    18,
      26,    27,    28,    29,    30,    31,    32,    14,    34,    35,
      36,    62,    63,    37,    38
  };

  const signed char
  parser::yytable_[] =
  {
      33,    15,    43,    44,    45,    20,    21,     1,    22,    33,
      17,     2,     3,     4,     5,     6,     7,    23,    19,    40,
      60,    61,    59,    24,    41,    54,    47,    39,    55,    25,
      20,    21,     1,    22,    50,    51,     2,     3,     4,     5,
       6,     7,    23,    42,    21,    71,    22,    75,    24,    72,
     -43,    76,    46,   -43,    25,    59,    61,    73,    52,    53,
      74,    24,    66,    67,     1,    68,    69,    25,     2,     3,
       4,     5,     6,     7,    56,    49,    41,    65,     2,     3,
       4,     5,     6,     7,    64,    70,    77,    79,    16,    48,
      78,     0,    80
  };

  const signed char
  parser::yycheck_[] =
  {
      17,     0,    22,    23,    24,     3,     4,     5,     6,    26,
      23,     9,    10,    11,    12,    13,    14,    15,     3,    16,
      40,    41,    39,    21,    21,     3,    24,    21,     6,    27,
       3,     4,     5,     6,    17,    18,     9,    10,    11,    12,
      13,    14,    15,     3,     4,    22,     6,    22,    21,    26,
       3,    26,     3,     6,    27,    72,    76,     3,    19,    20,
       6,    21,    50,    51,     5,    52,    53,    27,     9,    10,
      11,    12,    13,    14,     5,    25,    21,    25,     9,    10,
      11,    12,    13,    14,    22,     3,    25,     3,     9,    26,
      72,    -1,    76
  };

  const signed char
  parser::yystos_[] =
  {
       0,     5,     9,    10,    11,    12,    13,    14,    29,    30,
      31,    32,    33,    36,    45,     0,    31,    23,    37,     3,
       3,     4,     6,    15,    21,    27,    38,    39,    40,    41,
      42,    43,    44,    45,    46,    47,    48,    51,    52,    21,
      16,    21,     3,    40,    40,    40,     3,    24,    39,    25,
      17,    18,    19,    20,     3,     6,     5,    34,    35,    45,
      40,    40,    49,    50,    22,    25,    42,    42,    43,    43,
       3,    22,    26,     3,     6,    22,    26,    25,    35,     3,
      50
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    28,    29,    30,    30,    31,    31,    32,    33,    34,
      34,    34,    35,    35,    36,    37,    38,    38,    39,    39,
      39,    39,    40,    41,    41,    41,    42,    42,    42,    43,
      43,    43,    43,    43,    43,    44,    44,    45,    45,    45,
      45,    45,    45,    45,    46,    47,    48,    49,    49,    49,
      50,    51,    52
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     2,     1,     1,     1,     2,     5,     3,
       1,     1,     2,     3,     6,     3,     3,     2,     1,     1,
       1,     1,     1,     3,     3,     1,     3,     3,     1,     1,
       1,     3,     1,     1,     1,     2,     3,     1,     1,     1,
       1,     1,     1,     1,     3,     2,     4,     3,     1,     0,
       1,     2,     2
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "ID", "NUM_LIT",
  "KWD_NIHIL", "SYM_PTR", "KWD_UI8", "KWD_I8", "KWD_UI16", "KWD_I16",
  "KWD_UI32", "KWD_I32", "KWD_UI64", "KWD_I64", "KWD_RETURN", "EQ_OP",
  "PLUS_OP", "MINUS_OP", "MUL_OP", "DIV_OP", "LPAREN", "RPAREN", "LCURLY",
  "RCURLY", "SEMI", "COMMA", "ADDR_OF_OP", "$accept", "program",
  "globalEntries", "globalEntry", "function", "functionHead", "paramList",
  "param", "fwdDecl", "scope", "stmts", "stmt", "expr", "addExpr",
  "mulExpr", "factor", "varDecl", "type", "varAss", "returnOp",
  "functionCall", "argsList", "arg", "addrOfOp", "derefOp", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const short
  parser::yyrline_[] =
  {
       0,   122,   122,   125,   126,   130,   131,   134,   169,   172,
     173,   174,   177,   178,   182,   191,   194,   195,   198,   199,
     200,   201,   206,   209,   210,   211,   214,   215,   216,   219,
     220,   221,   222,   223,   224,   230,   231,   234,   235,   237,
     238,   240,   241,   243,   249,   255,   261,   264,   265,   266,
     269,   275,   281
  };

  void
  parser::yy_stack_print_ () const
  {
    *yycdebug_ << "Stack now";
    for (stack_type::const_iterator
           i = yystack_.begin (),
           i_end = yystack_.end ();
         i != i_end; ++i)
      *yycdebug_ << ' ' << int (i->state);
    *yycdebug_ << '\n';
  }

  void
  parser::yy_reduce_print_ (int yyrule) const
  {
    int yylno = yyrline_[yyrule];
    int yynrhs = yyr2_[yyrule];
    // Print the symbols being reduced, and their result.
    *yycdebug_ << "Reducing stack by rule " << yyrule - 1
               << " (line " << yylno << "):\n";
    // The symbols being reduced.
    for (int yyi = 0; yyi < yynrhs; yyi++)
      YY_SYMBOL_PRINT ("   $" << yyi + 1 << " =",
                       yystack_[(yynrhs) - (yyi + 1)]);
  }
#endif // YYDEBUG

  parser::symbol_kind_type
  parser::yytranslate_ (int t)
  {
    // YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to
    // TOKEN-NUM as returned by yylex.
    static
    const signed char
    translate_table[] =
    {
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      25,    26,    27
    };
    // Last valid token kind.
    const int code_max = 282;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return YY_CAST (symbol_kind_type, translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

} // yy
#line 1369 "parser.cpp"

#line 284 "parser.y"



void yy::parser::error(const location_type& loc, const std::string& msg)
{
	std::cerr << "ERROR: " << msg << " at " << loc << std::endl;
	Exit(ErrCodes::syntax_error);
}
