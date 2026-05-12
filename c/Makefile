
FLAGS=-Wall -Wextra -Wpedantic -ggdb -O2 -std=c17 -I./includes
OUT=./a.out


SRC_LIST=main.c



all: $(OUT)

$(OUT): $(SRC_LIST) 
	gcc -o $(OUT) $(FLAGS) main.c 




TOKEN_TEST=tests/test_tokenizer.c
OUT_TOKEN_TEST=tests/out/test_tokenizer

run_test: $(TOKEN_TEST)
	gcc -o $(OUT_TOKEN_TEST) $(FLAGS) $(TOKEN_TEST) && ./$(OUT_TOKEN_TEST)