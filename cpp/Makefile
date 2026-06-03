CC=clang++
FLAGS=-std=c++23 -Wall -Wextra -Wpedantic -ggdb -O2
SRC=main.cpp
OUT=out.exe

INCLUDE=\
./include/tokenizer.h\
./include/node.h\
./include/parser.h\
./include/help.h


$(OUT): $(SRC) $(INCLUDE)
	$(CC) -o $(OUT) $(FLAGS) -I./include $(SRC)


all: $(OUT)

ASM=asm.txt
$(ASM): $(SRC) $(INCLUDE)
	$(CC) -o $(ASM) -S -std=c++23 -O2 -I./include $(SRC)

asm: $(ASM)

prepr: 
	$(CC) -o main.txt -E $(FLAGS) -I./include $(SRC)


build_test:
	$(CC) -o ./tests/out/tok_test.exe $(FLAGS) -I./include ./tests/tok_test.cpp
