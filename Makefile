OUT=./compiler.exe

SRC=./src/common.go \
./src/main.go \
./src/tokenizer.go


all: $(OUT)

$(OUT): $(SRC)
	go build -o $(OUT) ./src