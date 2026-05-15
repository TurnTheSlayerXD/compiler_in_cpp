CC=clang

FLAGS=-Wall -Wextra -Wpedantic -ggdb -std=c17 -I./includes
OUT=./a.exe


SRC_LIST=main.c tokenizer.c token_types.h ./includes/macros.h ./includes/strdup.h ./includes/string_view.h

all: $(OUT)

$(OUT): $(SRC_LIST) 
	$(CC) -o $(OUT) $(FLAGS) main.c 


prepr: 
	$(CC) -o main.txt -E $(FLAGS) main.c

TOKEN_TEST=tests/test_tokenizer.c
OUT_TOKEN_TEST=tests/out/test_tokenizer

run_test: $(OUT_TOKEN_TEST) 

test_valgrind: 
	valgrind ./tests/out/test_tokenizer -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment --leak-check=full -s

$(OUT_TOKEN_TEST): $(TOKEN_TEST) $(SRC_LIST)
	$(CC) -o $(OUT_TOKEN_TEST) $(FLAGS) $(TOKEN_TEST) && ./$(OUT_TOKEN_TEST)