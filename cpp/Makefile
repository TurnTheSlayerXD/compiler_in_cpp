CC=clang++
FLAGS=-std=c++20 -Wall -Wextra -Wpedantic
SRC=main.cpp
OUT=out.exe
OUT_GDB=gdb_out.exe

INCLUDE=\
./include/tokenizer.h\
./include/node.h\
./include/parser.h\
./include/help.h\
./include/parsing_expr.h\
./include/tree_preprocessing.h


$(OUT): $(SRC) $(INCLUDE)
	$(CC) -o $(OUT) $(FLAGS) -I./include $(SRC)

all: $(OUT)

$(OUT_GDB): $(SRC) $(INCLUDE)
	$(CC) -o $(OUT_GDB) $(FLAGS) -ggdb -I./include $(SRC)

gdb: $(OUT_GDB)

test.asm: ./test.c
	clang -o test.asm -S test.c

to_asm: test.asm

exe_from_asm:
	clang -o test.exe ./test.asm

prepr: 
	$(CC) -o main.txt -E $(FLAGS) -I./include $(SRC)


build_tok_test:
	$(CC) -o ./tests/out/tok_test.exe $(FLAGS) -I./include ./tests/tok_test.cpp

build_parser_test:
	$(CC) -o ./tests/out/parser_test.exe $(FLAGS) -I./include ./tests/parser_test.cpp