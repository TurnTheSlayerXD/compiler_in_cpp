CC=clang++
FLAGS=-std=c++20 -Wall -Wextra -Wpedantic -O2
SRC=main.cpp
OUT=out.exe
OUT_GDB=gdb_out.exe

INCLUDE=\
./include/tokenizer.h\
./include/node.h\
./include/parser.h\
./include/help.h\
./include/parsing_expr.h\
./include/sort_op_nodes.h


$(OUT): $(SRC) $(INCLUDE)
	$(CC) -o $(OUT) $(FLAGS) -I./include $(SRC)

all: $(OUT)

$(OUT_GDB): $(SRC) $(INCLUDE)
	$(CC) -o $(OUT_GDB) $(FLAGS) -ggdb -I./include $(SRC)

gdb: $(OUT_GDB)

ASM=asm.txt
$(ASM): $(SRC) $(INCLUDE)
	$(CC) -o $(ASM) -S -std=c++23 -O2 -I./include $(SRC)

asm: $(ASM)

prepr: 
	$(CC) -o main.txt -E $(FLAGS) -I./include $(SRC)


build_tok_test:
	$(CC) -o ./tests/out/tok_test.exe $(FLAGS) -I./include ./tests/tok_test.cpp

build_parser_test:
	$(CC) -o ./tests/out/parser_test.exe $(FLAGS) -I./include ./tests/parser_test.cpp