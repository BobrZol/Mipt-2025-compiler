CC = g++
BISON = bison
FLEX = flex
NameCompiler = compiler

# Объявляем объектные файлы
OBJ = parser.tab.o lex.yy.o Ast.o

all: $(NameCompiler) clean test

parser.tab.c: parser.y
	$(BISON) -d parser.y

lex.yy.c: lexer.l
	$(FLEX) lexer.l

Ast.o: Ast.cpp Ast.hpp
	$(CC) -c Ast.cpp

parser.tab.o: parser.tab.c Ast.hpp
	$(CC) -c parser.tab.c

lex.yy.o: lex.yy.c Ast.hpp
	$(CC) -c lex.yy.c

# Линковка всех объектных файлов в исполняемый файл
$(NameCompiler): $(OBJ)
	$(CC) -o $(NameCompiler) $(OBJ) -lfl

test: $(NameCompiler)
 # valgrind --leak-check=full --show-leak-kinds=all ./$(NameCompiler)
	python3 test_compiler.py

clean:
	rm -f parser.tab.c parser.tab.h lex.yy.c $(OBJ)
