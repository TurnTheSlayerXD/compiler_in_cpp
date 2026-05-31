CC=clang++
FLAGS=-std=c++23 -Wall -Wextra -Wpedantic -ggdb
SRC=main.cpp
OUT=out.exe

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(FLAGS)

all: $(OUT)

prepr: 
	$(CC) -o main.txt -E $(FLAGS) main.cpp
