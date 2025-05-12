CC = g++
BISON = bison
FLEX = flex
NameCompiler = compiler

LLVM_BUILD_DIR = ./llvm-project/build
LLVM_CONFIG = $(LLVM_BUILD_DIR)/bin/llvm-config

LLVM_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
LLVM_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags --system-libs --libs all)

CXXFLAGS = -std=c++17 -fno-rtti -fno-pie $(LLVM_CXXFLAGS)
LDFLAGS = $(LLVM_LDFLAGS) -lfl -no-pie -fPIE

OBJ = parser.tab.o lex.yy.o Ast.o IRGenerator.o

all: $(NameCompiler) test clean

parser.tab.c: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l
	$(FLEX) lexer.l

Ast.o: Ast.cpp Ast.hpp
	$(CC) $(CXXFLAGS) -c Ast.cpp

IRGenerator.o: IRGenerator.cpp IRGenerator.hpp
	$(CC) $(CXXFLAGS) -c IRGenerator.cpp

parser.tab.o: parser.tab.c
	$(CC) $(CXXFLAGS) -c parser.tab.c

lex.yy.o: lex.yy.c
	$(CC) $(CXXFLAGS) -c lex.yy.c

$(NameCompiler): $(OBJ)
	$(CC) -o $@ $(OBJ) $(LDFLAGS)

test: $(NameCompiler)
	python3 test_compiler.py

clean:
	rm -f parser.tab.* lex.yy.* $(OBJ) $(NameCompiler) *.ll temp_program.o
