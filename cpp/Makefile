CC=clang++
FLAGS=-std=c++23 -Wall -Wextra -Wpedantic -ggdb
SRC=main.cpp
OUT=out.exe

INCLUDE=\
./include/tokenizer.h

$(OUT): $(SRC) $(INCLUDE)
	$(CC) -o $(OUT) $(FLAGS) -I./include $(SRC)

all: $(OUT)

prepr: 
	$(CC) -o main.txt -E $(FLAGS) -I./include $(SRC)


build_test:
	$(CC) -o ./tests/out/tok_test.exe $(FLAGS) -I./include ./tests/tok_test.cpp
