CC=clang++
FLAGS=-O2 -std=c++17 -Wall -Wextra
SRC=main.cpp
OUT=out.exe

$(OUT): $(SRC)
	$(CC) -o $(OUT) $(SRC) $(FLAGS)

all: $(OUT)

prepr: 
	$(CC) -o main.txt -E $(FLAGS) main.cpp
