CC = g++
BISON = bison
FLEX = flex
NameCompiler = compiler

all: $(NameCompiler) clean test

parser.tab.c: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l
	$(FLEX) lexer.l

$(NameCompiler): parser.tab.c lex.yy.c
	$(CC) -o $(NameCompiler) parser.tab.c lex.yy.c -lfl

test: $(NameCompiler) clean
	python3 test_compiler.py

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c