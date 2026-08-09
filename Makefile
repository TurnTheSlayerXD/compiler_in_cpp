CC=clang++
FLAGS=-std=c++20 -Wall -Wextra -Wpedantic
OUT=ast_builder.exe
OUT_GDB=gdb_out.exe

SRC=\
./src/main.cpp\
./include/tokenizer.h\
./include/node.h\
./include/parser.h\
./include/help.h\
./include/parsing_expr.h\
./include/tree_preprocessing.h

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(FLAGS) -I./include -O2 ./src/main.cpp

all: $(OUT)

$(OUT_GDB): $(SRC)
	$(CC) -o $(OUT_GDB) $(FLAGS) -ggdb -I./include ./src/main.cpp

gdb: $(OUT_GDB)


build_tok_test:
	$(CC) -o ./tests/out/tok_test.exe $(FLAGS) -I./include ./tests/tok_test.cpp

build_parser_test:
	$(CC) -o ./tests/out/parser_test.exe $(FLAGS) -I./include ./tests/parser_test.cpp