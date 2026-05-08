OUT=./compiler.exe

SRC=./src/common.go \
./src/main.go \
./src/tokenizer.go


all: $(OUT)

$(OUT): $(SRC)
	go build -o $(OUT) ./src



test: ./test/test_token.go
	go run ./test/test_token.go ./src/tokenizer.go