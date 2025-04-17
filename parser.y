%code requires {
  #include "Ast.hpp"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include <llvm/Support/CommandLine.h>
#include "IRGenerator.hpp"

namespace cl = llvm::cl;

cl::opt<bool> EmitIR(
    "emit-ir",
    cl::desc("Generate LLVM IR instead of executing")
);

cl::opt<std::string> ASTOutput(
    "ast-output",
    cl::desc("Output file for AST dump"),
    cl::value_desc("filename")
);

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
        if (EmitIR) {
            llvm::LLVMContext context;
            llvm::Module module("main", context);
            IRGenerator generator(context, &module);
            generator.Generate(*$5);
            module.print(llvm::outs(), nullptr);
        } else {
            InterpreterBase visitor;
            $5->InterpretStmt(globalScope, visitor);
        }

        if (!ASTOutput.empty()) {
            std::ofstream out(ASTOutput.c_str());
            $5->PrintAst(out);
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
    cl::ParseCommandLineOptions(argc, argv, "My LLVM Tool\n");

    /* if (EmitIR) {
        std::cout << "Generating LLVM IR.\n";
    }

    if (!ASTOutput.empty()) {
        std::cout << "Outputting AST to file: " << ASTOutput << "\n";
    } */

    yyparse();
    return 0;
}

void yyerror(const char *s) {
    std::cerr << "Error: " << s << std::endl;
}
