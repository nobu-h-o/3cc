%{
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "ast.h"
#include "parser.tab.h"
%}

%%
"return"    { return RETURN; }
"while"     { return WHILE; }
"for"       { return FOR; }
"if"        { return IF; }
"else"      { return ELSE; }
"print"     { return PRINT; }
[0-9]+      { yylval.number = atoi(yytext); return NUMBER; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.string = strdup(yytext); return IDENTIFIER; }
"="         { return ASSIGN; }
";"         { return SEMICOLON; }
","         { return COMMA; }
"+"         { return ADD; }
"-"         { return SUB; }
"*"         { return MUL; }
"/"         { return DIV; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }
"<"         { return LT; }
">"         { return GT; }
"<="        { return LE; }
">="        { return GE; }
"=="        { return EQ; }
"!="        { return NE; }
[ \t\n]     { /* ignore whitespace */ }
.           { fprintf(stderr, "Unknown character: %s\n", yytext); exit(1); }
%%

int yywrap(void) {
    return 1;
}
