%code requires {
    #include "Ast.hpp"
}

%{
#include <iostream>
#include <memory>
#include <string>

int print_ast = 0;
char *ast_filename = NULL;

extern int yylex();
void yyerror(const char *s);
%}

%union {
    int num;
    char* str;
    Stmt* stmt;
    Expr* expr;
    Root* root;
}

%token MAIN DECLARE INT IF ELSE PRINT
%token <str> ID
%token <num> NUMBER
%token ASSIGN EQ PLUS MINUS MULT DIV
%token LPAREN RPAREN LBRACE RBRACE SEMICOLON COLON

%type <stmt> statement declaration assignment if_stmt print_stmt
%type <root> program statements
%type <expr> expr

%left PLUS MINUS
%left MULT DIV

%%

program:
    MAIN LPAREN RPAREN LBRACE statements RBRACE {
        $$ = $5;
        $$->InterpretStmt(globalScope);

        if (print_ast) {
          std::ofstream outFile(ast_filename);
          if (outFile.is_open()) {
              $$->PrintAst(outFile);
              outFile.close();
          }
        }
    }
    ;

statements:
    statement { $$ = new Root($1, nullptr); }
    | statement statements {  $$ = new Root($1, $2); }
    | { $$ = nullptr; }
    ;

statement:
    declaration SEMICOLON { $$ = $1; }
    | assignment SEMICOLON { $$ = $1; }
    | if_stmt { $$ = $1; }
    | print_stmt SEMICOLON { $$ = $1; }
    ;

declaration:
    DECLARE ID COLON INT {
        $$ = new Declare(std::string($2));
        free($2);
    }
    ;

assignment:
    ID ASSIGN expr {
        $$ = new Assignment(std::string($1), std::unique_ptr<Expr>($3));
        free($1);
    }
    ;if_stmt:
    IF LPAREN expr EQ expr RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE {
        Expr* cond = new BinOp("==", std::unique_ptr<Expr>($3), std::unique_ptr<Expr>($5));
        $$ = new Condition(std::unique_ptr<Expr>(cond), std::unique_ptr<Stmt>($8), std::unique_ptr<Stmt>($12));
    }
    ;

print_stmt:
    PRINT LPAREN expr RPAREN {
        $$ = new Print(std::unique_ptr<Expr>($3));
    }
    ;

expr:
    NUMBER           { $$ = new Number($1); }
    | ID             { $$ = new Variable(std::string($1)); free($1); }
    | expr PLUS expr { $$ = new BinOp("+", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
    | expr MINUS expr { $$ = new BinOp("-", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
    | expr MULT expr { $$ = new BinOp("*", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
    | expr DIV expr  { $$ = new BinOp("/", std::unique_ptr<Expr>($1), std::unique_ptr<Expr>($3)); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

%%

int main(int argc, char *argv[]) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "print_ast=true") == 0) {
            print_ast = 1;
        } else if (strncmp(argv[i], "ast_file=", 9) == 0) {
            ast_filename = argv[i] + 9;
        }
    }

    yyparse();
    return 0;
}

void yyerror(const char *s) {
    std::cerr << "Error: " << s << std::endl;
}
