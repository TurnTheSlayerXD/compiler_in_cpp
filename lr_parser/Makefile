CC=clang++
FLAGS=-Wall -Wextra -Wpedantic -O2 -std=c++23
GDB_FLAGS=-Wall -Wextra -Wpedantic -std=c++23 -ggdb
SRC=main.cpp structs.h


all: lr_parser
lr_parser: $(SRC)
	$(CC) -o lr_parser $(FLAGS) ./main.cpp

gdb_lr_parser: $(SRC)
	$(CC) -o gdb_lr_parser $(GDB_FLAGS) ./main.cpp
