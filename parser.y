%code requires {
  #include "Ast.hpp"
  #include "llvm/MC/TargetRegistry.h"
  #include "llvm/Support/FileSystem.h"
  #include "llvm/TargetParser/Host.h"
  #include "llvm/Support/TargetSelect.h"
  #include "llvm/Target/TargetMachine.h"
  #include "llvm/Target/TargetOptions.h"
  #include "llvm/IR/LegacyPassManager.h"
}

%{
#include <iostream>
#include <memory>
#include <string>
#include <llvm/Support/CommandLine.h>
#include "IRGenerator.hpp"

namespace cl = llvm::cl;

cl::opt<bool> EmitExecutable(
    "emit-executable",
    cl::desc("Generate executable file directly")
);

cl::opt<std::string> OutputFile(
    "o",
    cl::desc("Output executable file name"),
    cl::value_desc("filename")
);

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
    bool bl;
    char* str;
    Stmt* stmt;
    Expr* expr;
    Root* root;
}

%token MAIN DECLARE INT IF ELSE PRINT BOOL
%token <str> ID
%token <bl> TRUE FALSE
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
        TypeChecker checker;
        checker.TypeCheck(*$5);
        llvm::LLVMContext context;
        llvm::Module module("main", context);
        IRGenerator generator(context, &module);
        generator.Generate(*$5);

        if (EmitIR) {
            module.print(llvm::outs(), nullptr);
        } else if (EmitExecutable) {
            std::string Error;
            llvm::Triple triple(llvm::sys::getDefaultTargetTriple());
            module.setTargetTriple(triple);

            const llvm::Target* TheTarget = llvm::TargetRegistry::lookupTarget(triple, Error);
            if (!TheTarget) {
                std::cerr << "Error initializing target: " << Error << std::endl;
                exit(1);
            }

            llvm::TargetOptions opt;
            std::optional<llvm::Reloc::Model> RM = llvm::Reloc::Static;

            llvm::TargetMachine* TargetMachine = TheTarget->createTargetMachine(
                triple, "generic", "", opt, RM
            );
            module.setDataLayout(TargetMachine->createDataLayout());

            // Генерация объектного файла
            std::string ObjectFile = OutputFile.empty() ? "temp.o" : OutputFile + ".o";
            std::error_code EC;
            llvm::raw_fd_ostream dest(ObjectFile, EC, llvm::sys::fs::OF_None);
            if (EC) {
                std::cerr << "Could not open file: " << EC.message() << std::endl;
                exit(1);
            }

            llvm::legacy::PassManager pass;
            if (TargetMachine->addPassesToEmitFile(
                pass, dest, nullptr, llvm::CodeGenFileType::ObjectFile
            )) {
                std::cerr << "Failed to emit object file" << std::endl;
                exit(1);
            }

            pass.run(module);
            dest.flush();

            // Линковка
            std::string ExeFile = OutputFile.empty() ? std::string("temp_program") : OutputFile;
            std::string linkCmd = "gcc -no-pie " + ObjectFile + " -o " + ExeFile;
            if (system(linkCmd.c_str()) != 0) {
                std::cerr << "Linking failed" << std::endl;
                exit(1);
            }

            if (OutputFile.empty()) {
                std::remove(ObjectFile.c_str());
            }
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
        $$ = new Declare(std::string($2), "int");
        free($2);
    }
    | DECLARE ID COLON BOOL {
        $$ = new Declare(std::string($2), "bool");
        free($2);
    }
    ;

assignment:
    ID ASSIGN expr {
        $$ = new Assignment(std::string($1), std::unique_ptr<Expr>($3));
        free($1);
    }
    ;if_stmt:
    IF LPAREN expr EQ expr RPAREN LBRACE statements RBRACE {
        Expr* cond = new BinOp("==", std::unique_ptr<Expr>($3), std::unique_ptr<Expr>($5));
        $$ = new Condition(std::unique_ptr<Expr>(cond), std::unique_ptr<Stmt>($8), nullptr);
    }
    | IF LPAREN expr EQ expr RPAREN LBRACE statements RBRACE ELSE LBRACE statements RBRACE {
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
    | TRUE            { $$ = new Boolean($1); }
    | FALSE            { $$ = new Boolean($1); }
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

    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    yyparse();
    return 0;
}

void yyerror(const char *s) {
    std::cerr << "Error: " << s << std::endl;
}
