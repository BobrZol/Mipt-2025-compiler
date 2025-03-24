%{
#include <iostream>
#include "parser.tab.h"
%}

%option noyywrap

%%

"main"          { return MAIN; }
"declare"       { return DECLARE; }
"int"           { return INT; }
"if"            { return IF; }
"else"          { return ELSE; }
"print"         { return PRINT; }
[a-zA-Z][a-zA-Z0-9]* { yylval.str = strdup(yytext); return ID; }
[0-9]+          { yylval.num = atoi(yytext); return NUMBER; }
"="             { return ASSIGN; }
"=="            { return EQ; }
"+"             { return PLUS; }
"-"             { return MINUS; }
"*"             { return MULT; }
"/"             { return DIV; }
"("             { return LPAREN; }
")"             { return RPAREN; }
"{"             { return LBRACE; }
"}"             { return RBRACE; }
";"             { return SEMICOLON; }
":"             { return COLON; }
[ \t\n]         ;
.               { std::cerr << "Unknown character: " << yytext << std::endl; }

%%

// int yyerror(const char *s) {
//     std::cerr << "Error: " << s << std::endl;
//     return 0;
// }
