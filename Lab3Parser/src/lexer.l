/* src/lexer.l */
%option noyywrap
%option nounput noinput
%option never-interactive

%{
#define YY_NO_UNISTD_H
#include <cstdlib>
#include <cstring>
#include <string>
#include "../generated/parser.hpp"  // токени + yylval
%}

DIGIT      [0-9]
ID         [a-zA-Z_][a-zA-Z0-9_]*
WS         [ \t\r\n]+

%%
"//".*                         { /* коментар до к≥нц€ р€дка */ }
"/*"([^*]|\*+[^*/])*\*+"/"    { /* (просте) багатор€дкове, опц. */ }

"int"                          { return KW_INT; }
"double"                       { return KW_DOUBLE; }
"if"                           { return KW_IF; }
"else"                         { return KW_ELSE; }
"while"                        { return KW_WHILE; }
"print"                        { return KW_PRINT; }

"=="                           { return EQ; }
"!="                           { return NE; }
"<="                           { return LE; }
">="                           { return GE; }
"&&"                           { return AND; }
"||"                           { return OR; }

{ID}                           { yylval.str = new std::string(yytext); return IDENT; }

{DIGIT}+("."{DIGIT}+)?         { yylval.num = std::strtod(yytext, nullptr); return NUMBER; }
"."{DIGIT}+                    { yylval.num = std::strtod(yytext, nullptr); return NUMBER; }

{WS}                           { /* пропускаЇмо */ }

.                              { return yytext[0]; }
%%
