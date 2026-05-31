CC=clang++
FLAGS=-std=c++23 -Wall -Wextra -Wpedantic -ggdb
SRC=main.cpp
OUT=out.exe

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(FLAGS) -I./include

all: $(OUT)

prepr: 
	$(CC) -o main.txt -E $(FLAGS) main.cpp


build_test:
	$(CC) -o ./tests/out/tok_test.exe $(FLAGS) -I./include ./tests/tok_test.cpp
