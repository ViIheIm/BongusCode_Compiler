@echo off
echo Bat-file working directory: %cd%
reflex.exe "lexer.l" --bison-cc --bison_locations --namespace=yy --lexer=Lexer --noyywrap --header-file="lexer.h" --outfile="lexer.cpp"