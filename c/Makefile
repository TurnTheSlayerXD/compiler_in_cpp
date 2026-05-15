
FLAGS=-Wall -Wextra -Wpedantic -ggdb -O2 -std=c17 -I./includes
OUT=./a.out


SRC_LIST=main.c tokenizer.c token_types.h



all: $(OUT)

$(OUT): $(SRC_LIST) 
	gcc -o $(OUT) $(FLAGS) main.c 




TOKEN_TEST=tests/test_tokenizer.c
OUT_TOKEN_TEST=tests/out/test_tokenizer

run_test: $(OUT_TOKEN_TEST) 

test_valgrind: 
	valgrind ./tests/out/test_tokenizer -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=all -fsanitize=float-divide-by-zero -fsanitize=float-cast-overflow -fno-sanitize=null -fno-sanitize=alignment --leak-check=full -s

$(OUT_TOKEN_TEST): $(TOKEN_TEST) $(SRC_LIST)
	gcc -o $(OUT_TOKEN_TEST) $(FLAGS) $(TOKEN_TEST) && ./$(OUT_TOKEN_TEST)