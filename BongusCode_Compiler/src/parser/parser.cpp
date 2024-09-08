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
  case 2: // program: functions
#line 109 "parser.y"
                                                        { g_nodeHead = AST::MakeNullNode(); g_nodeHead->AdoptChildren((yystack_[0].value.ASTNode)); }
#line 620 "parser.cpp"
    break;

  case 3: // functions: functions function
#line 112 "parser.y"
                                        { (yylhs.value.ASTNode) = (yystack_[1].value.ASTNode)->MakeSiblings((yystack_[0].value.ASTNode)); }
#line 626 "parser.cpp"
    break;

  case 4: // functions: function
#line 113 "parser.y"
                   { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 632 "parser.cpp"
    break;

  case 5: // function: functionHead scope
#line 116 "parser.y"
                                        { (yylhs.value.ASTNode) = (yystack_[1].value.ASTNode); (yystack_[1].value.ASTNode)->AdoptChildren((yystack_[0].value.ASTNode)); }
#line 638 "parser.cpp"
    break;

  case 6: // functionHead: type ID LPAREN paramList RPAREN
#line 119 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeFunctionNode((yystack_[4].value.primtype), (yystack_[3].value.str), (yystack_[1].value.ASTNode)); }
#line 644 "parser.cpp"
    break;

  case 7: // paramList: paramList COMMA param
#line 122 "parser.y"
                                        { (yystack_[2].value.ASTNode)->MakeSiblings((yystack_[0].value.ASTNode)); (yylhs.value.ASTNode) = (yystack_[2].value.ASTNode); }
#line 650 "parser.cpp"
    break;

  case 8: // paramList: param
#line 123 "parser.y"
                   { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 656 "parser.cpp"
    break;

  case 9: // paramList: KWD_NIHIL
#line 124 "parser.y"
                                                        { (yylhs.value.ASTNode) = nullptr; }
#line 662 "parser.cpp"
    break;

  case 10: // param: type ID
#line 127 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeArgNode((yystack_[1].value.primtype), (yystack_[0].value.str)); }
#line 668 "parser.cpp"
    break;

  case 11: // scope: LCURLY stmts RCURLY
#line 136 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeScopeNode(); (yylhs.value.ASTNode)->AdoptChildren((yystack_[1].value.ASTNode)); }
#line 674 "parser.cpp"
    break;

  case 12: // stmts: stmts stmt SEMI
#line 139 "parser.y"
                                                { (yylhs.value.ASTNode) = (yystack_[2].value.ASTNode)->MakeSiblings((yystack_[1].value.ASTNode)); }
#line 680 "parser.cpp"
    break;

  case 13: // stmts: stmt SEMI
#line 140 "parser.y"
                                                        { (yylhs.value.ASTNode) = (yystack_[1].value.ASTNode); }
#line 686 "parser.cpp"
    break;

  case 14: // stmt: expr
#line 143 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 692 "parser.cpp"
    break;

  case 15: // stmt: varDecl
#line 144 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 698 "parser.cpp"
    break;

  case 16: // stmt: varAss
#line 145 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 704 "parser.cpp"
    break;

  case 17: // stmt: returnOp
#line 146 "parser.y"
                                                                { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 710 "parser.cpp"
    break;

  case 18: // expr: addExpr
#line 151 "parser.y"
                                                        { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 716 "parser.cpp"
    break;

  case 19: // addExpr: addExpr PLUS_OP mulExpr
#line 154 "parser.y"
                                        { (yylhs.value.ASTNode) = AST::MakeOpNode(L'+', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 722 "parser.cpp"
    break;

  case 20: // addExpr: addExpr MINUS_OP mulExpr
#line 155 "parser.y"
                                        { (yylhs.value.ASTNode) = AST::MakeOpNode(L'-', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 728 "parser.cpp"
    break;

  case 21: // addExpr: mulExpr
#line 156 "parser.y"
             { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 734 "parser.cpp"
    break;

  case 22: // mulExpr: mulExpr MUL_OP factor
#line 159 "parser.y"
                                        { (yylhs.value.ASTNode) = AST::MakeOpNode(L'*', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 740 "parser.cpp"
    break;

  case 23: // mulExpr: mulExpr DIV_OP factor
#line 160 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeOpNode(L'/', (yystack_[2].value.ASTNode), (yystack_[0].value.ASTNode)); }
#line 746 "parser.cpp"
    break;

  case 24: // mulExpr: factor
#line 161 "parser.y"
             { (yylhs.value.ASTNode) = (yystack_[0].value.ASTNode); }
#line 752 "parser.cpp"
    break;

  case 25: // factor: NUM_LIT
#line 164 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeIntNode((yystack_[0].value.num)); }
#line 758 "parser.cpp"
    break;

  case 26: // factor: ID
#line 165 "parser.y"
                                                                { (yylhs.value.ASTNode) = AST::MakeSymNode((yystack_[0].value.str)); }
#line 764 "parser.cpp"
    break;

  case 27: // factor: LPAREN expr RPAREN
#line 166 "parser.y"
                                                { (yylhs.value.ASTNode) = (yystack_[1].value.ASTNode); }
#line 770 "parser.cpp"
    break;

  case 28: // varDecl: type ID
#line 172 "parser.y"
                                                        { (yylhs.value.ASTNode) = AST::MakeDeclNode((yystack_[0].value.str), (yystack_[1].value.primtype)); }
#line 776 "parser.cpp"
    break;

  case 29: // type: KWD_UI16
#line 175 "parser.y"
                                                        { (yylhs.value.primtype) = PrimitiveType::ui16; }
#line 782 "parser.cpp"
    break;

  case 30: // type: KWD_I16
#line 176 "parser.y"
                                                                { (yylhs.value.primtype) = PrimitiveType::i16;	}
#line 788 "parser.cpp"
    break;

  case 31: // type: KWD_UI32
#line 178 "parser.y"
                                                                { (yylhs.value.primtype) = PrimitiveType::ui32;	}
#line 794 "parser.cpp"
    break;

  case 32: // type: KWD_I32
#line 179 "parser.y"
                                                                { (yylhs.value.primtype) = PrimitiveType::i32;	}
#line 800 "parser.cpp"
    break;

  case 33: // type: KWD_UI64
#line 181 "parser.y"
                                                                { (yylhs.value.primtype) = PrimitiveType::ui64; }
#line 806 "parser.cpp"
    break;

  case 34: // type: KWD_I64
#line 182 "parser.y"
                                                                { (yylhs.value.primtype) = PrimitiveType::i64;	}
#line 812 "parser.cpp"
    break;

  case 35: // type: KWD_NIHIL
#line 184 "parser.y"
                                                                { wprintf(L"ERROR: A variable may not be of type nihil.\n"); Exit(ErrCodes::unknown_type); }
#line 818 "parser.cpp"
    break;

  case 36: // varAss: ID EQ_OP expr
#line 190 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeAssNode(AST::MakeSymNode((yystack_[2].value.str)) /* <--- Hurr durr */, (yystack_[0].value.ASTNode)); }
#line 824 "parser.cpp"
    break;

  case 37: // returnOp: KWD_RETURN expr
#line 196 "parser.y"
                                                { (yylhs.value.ASTNode) = AST::MakeReturnNode((yystack_[0].value.ASTNode)); }
#line 830 "parser.cpp"
    break;


#line 834 "parser.cpp"

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





  const signed char parser::yypact_ninf_ = -16;

  const signed char parser::yytable_ninf_ = -36;

  const signed char
  parser::yypact_[] =
  {
      35,   -16,   -16,   -16,   -16,   -16,   -16,   -16,     6,    35,
     -16,    -3,    24,   -16,   -16,    21,   -16,    39,    45,   -16,
      -2,    -2,     0,    40,   -16,     5,    18,   -16,   -16,    58,
     -16,   -16,    44,    -2,   -16,   -16,    42,   -16,    41,   -16,
      -2,    -2,    -2,    -2,   -16,    63,    37,   -16,    64,   -16,
     -16,   -16,    18,    18,   -16,   -16,   -16,    35,   -16,   -16
  };

  const signed char
  parser::yydefact_[] =
  {
       0,    35,    29,    30,    31,    32,    33,    34,     0,     2,
       4,     0,     0,     1,     3,     0,     5,     0,    26,    25,
       0,     0,     0,     0,    14,    18,    21,    24,    15,     0,
      16,    17,     0,     0,    26,    37,     0,    11,     0,    13,
       0,     0,     0,     0,    28,     9,     0,     8,     0,    36,
      27,    12,    19,    20,    22,    23,     6,     0,    10,     7
  };

  const signed char
  parser::yypgoto_[] =
  {
     -16,   -16,   -16,    59,   -16,   -16,    12,   -16,   -16,    48,
      -5,   -16,    10,    -4,   -16,   -15,   -16,   -16
  };

  const signed char
  parser::yydefgoto_[] =
  {
      -1,     8,     9,    10,    11,    46,    47,    16,    22,    23,
      24,    25,    26,    27,    28,    12,    30,    31
  };

  const signed char
  parser::yytable_[] =
  {
      29,    34,    19,    18,    19,     1,    13,    29,     2,     3,
       4,     5,     6,     7,    20,    35,    36,    48,    21,    15,
      21,    40,    41,    37,    18,    19,     1,    17,    49,     2,
       3,     4,     5,     6,     7,    20,    42,    43,    54,    55,
       1,    21,    48,     2,     3,     4,     5,     6,     7,    45,
      52,    53,     2,     3,     4,     5,     6,     7,    56,    32,
      33,    44,    57,    50,    39,    51,   -35,    58,    14,    59,
      38
  };

  const signed char
  parser::yycheck_[] =
  {
      15,     3,     4,     3,     4,     5,     0,    22,     8,     9,
      10,    11,    12,    13,    14,    20,    21,    32,    20,    22,
      20,    16,    17,    23,     3,     4,     5,     3,    33,     8,
       9,    10,    11,    12,    13,    14,    18,    19,    42,    43,
       5,    20,    57,     8,     9,    10,    11,    12,    13,     5,
      40,    41,     8,     9,    10,    11,    12,    13,    21,    20,
      15,     3,    25,    21,    24,    24,     3,     3,     9,    57,
      22
  };

  const signed char
  parser::yystos_[] =
  {
       0,     5,     8,     9,    10,    11,    12,    13,    27,    28,
      29,    30,    41,     0,    29,    22,    33,     3,     3,     4,
      14,    20,    34,    35,    36,    37,    38,    39,    40,    41,
      42,    43,    20,    15,     3,    36,    36,    23,    35,    24,
      16,    17,    18,    19,     3,     5,    31,    32,    41,    36,
      21,    24,    38,    38,    39,    39,    21,    25,     3,    32
  };

  const signed char
  parser::yyr1_[] =
  {
       0,    26,    27,    28,    28,    29,    30,    31,    31,    31,
      32,    33,    34,    34,    35,    35,    35,    35,    36,    37,
      37,    37,    38,    38,    38,    39,    39,    39,    40,    41,
      41,    41,    41,    41,    41,    41,    42,    43
  };

  const signed char
  parser::yyr2_[] =
  {
       0,     2,     1,     2,     1,     2,     5,     3,     1,     1,
       2,     3,     3,     2,     1,     1,     1,     1,     1,     3,
       3,     1,     3,     3,     1,     1,     1,     3,     2,     1,
       1,     1,     1,     1,     1,     1,     3,     2
  };


#if YYDEBUG
  // YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
  // First, the terminals, then, starting at \a YYNTOKENS, nonterminals.
  const char*
  const parser::yytname_[] =
  {
  "\"end of file\"", "error", "\"invalid token\"", "ID", "NUM_LIT",
  "KWD_NIHIL", "KWD_UI8", "KWD_I8", "KWD_UI16", "KWD_I16", "KWD_UI32",
  "KWD_I32", "KWD_UI64", "KWD_I64", "KWD_RETURN", "EQ_OP", "PLUS_OP",
  "MINUS_OP", "MUL_OP", "DIV_OP", "LPAREN", "RPAREN", "LCURLY", "RCURLY",
  "SEMI", "COMMA", "$accept", "program", "functions", "function",
  "functionHead", "paramList", "param", "scope", "stmts", "stmt", "expr",
  "addExpr", "mulExpr", "factor", "varDecl", "type", "varAss", "returnOp", YY_NULLPTR
  };
#endif


#if YYDEBUG
  const unsigned char
  parser::yyrline_[] =
  {
       0,   109,   109,   112,   113,   116,   119,   122,   123,   124,
     127,   136,   139,   140,   143,   144,   145,   146,   151,   154,
     155,   156,   159,   160,   161,   164,   165,   166,   172,   175,
     176,   178,   179,   181,   182,   184,   190,   196
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
      25
    };
    // Last valid token kind.
    const int code_max = 280;

    if (t <= 0)
      return symbol_kind::S_YYEOF;
    else if (t <= code_max)
      return YY_CAST (symbol_kind_type, translate_table[t]);
    else
      return symbol_kind::S_YYUNDEF;
  }

} // yy
#line 1224 "parser.cpp"

#line 200 "parser.y"



void yy::parser::error(const location_type& loc, const std::string& msg)
{
	std::cerr << "ERROR: " << msg << " at " << loc << std::endl;
	Exit(ErrCodes::syntax_error);
}
