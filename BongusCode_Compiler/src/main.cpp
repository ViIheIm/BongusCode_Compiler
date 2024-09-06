#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <string.h>
#include <vector>

#include "Definitions.h"
#include "Exit.h"
#include "Utils.h"
#include "parser/parser.hpp"
#include "lexer/lexer.h"
#include "AST/AST_Harvest_Pass.h"
#include "AST/AST_Semantics_Pass.h"
#include "symbol_table/symtable.h"
#include "code_generator/codegen.h"

/*
wchar_t ProgramSrc[] = L"\n"
"i32 main(void)\n"
"{\n"
"    i32 a = 10;"
"    Claudere a;\n"
"}\n";

wchar_t ProgramSrc[] = LR"(

{
	ui64 Ξ.
	ui64 n.
	ui64 n2.
	ui64 n3.	

	Ξ = 1.
	n = 2.
	n2 = 3.
	n3 = 4.
    Ξ = Ξ + n2 + n3.
	
}

)";
*/


#include "AST/ASTNode.h"
AST::Node* g_nodeHead = nullptr;

inline static void PrintUsage(void)
{
	wprintf(L"USAGE: BongusCodeCompiler.exe \"sourceFilePath\" \"outFilePath\"\n");
}

// Tries to assemble, link and run the program, aswell as to print out the error level.
inline static void TryRunProgram(void);

i32 main(i32 argc, char** argv)
{
	// Print out all command line args for debugging.
#ifdef _DEBUG
	for (i32 i = 0; i < argc; i++)
	{
		printf("arg %i: %s\n", i, argv[i]);
	}
#endif

	(void)_setmode(_fileno(stdout), _O_U16TEXT);


	if (argc > 3)
	{
		wprintf(L"ERROR: Malformed command arguments.\n");
		PrintUsage();
		Exit(ErrCodes::malformed_cmd_line);
	}

	if (argv[1] == nullptr)
	{
		wprintf(L"ERROR: No input source file supplied.\n");
		Exit(ErrCodes::malformed_cmd_line);
	}

	if (argv[2] == nullptr)
	{
		wprintf(L"ERROR: No output filepath supplied.\n");
		Exit(ErrCodes::malformed_cmd_line);
	}

	const char* fSourceFilePath = argv[1];
	const char* fOutputFilePath = argv[2];
	
	FILE* translationUnit = fopen(fSourceFilePath, "r");
	
	if (translationUnit == nullptr)
	{
		wprintf(L"ERROR: Unable to open source file.\n");
		Exit(ErrCodes::malformed_cmd_line);
	}

	yy::Lexer lexer(translationUnit);
	
	yy::parser parser(lexer);

#if PARSER_DEBUG_TRACE == 1
	parser.set_debug_level(1);
#endif

	if (parser.parse() == 0) { wprintf(L"PARSER: Syntactically legal program recognized.\n"); }

	// Now we're done with reading the translation unit, so we can close it down.
	fclose(translationUnit);


	// First pass over AST: we harvest the symbol declarations and resolve symbol references. Page 280.
	AST::BuildSymbolTable(g_nodeHead);
	
	// Second pass over the AST: we check to make sure no semantic rules are violated.
	AST::SemanticsPass(g_nodeHead);

	// Now it's finally time to generate some code.
	std::string code;
	GenerateCode(g_nodeHead, code);

	delete g_nodeHead;

	// At last, we can write out our assembly to a file.
	FILE* outFile = fopen(fOutputFilePath, "w");
	fwrite(code.c_str(), sizeof(code[0]), code.length(), outFile);
	fclose(outFile);
	


	//TryRunProgram();

	return 0;
}


inline static void TryRunProgram(void)
{
	wprintf(L"Trying to assemble, link and run the compiled program, aswell as print it's exit code.\n");
	system("masm -c ../../Test_Output/out.asm");
	system(R"(link.exe ../../Test_Output/out.obj -ENTRY:main -OUT:"../../Test_Output/out.exe")");
	system("call ../../Test_Output/out.exe");
	system("echo %errorlevel%");
}