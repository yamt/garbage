WAMR_DIR = ~/git/wasm-micro-runtime/product-mini/platforms/darwin/b
WASI_SDK_DIR = /tmp/wasi-sdk/wasi-sdk-12.0

IWASM = $(WAMR_DIR)/iwasm
CC = $(WASI_SDK_DIR)/bin/clang

OBJS = wasm_hello.o
BIN = test.wasm

.PHONY: run
run: $(BIN)
	$(IWASM) $(BIN)

.c.o:
	$(CC) -c -Os -o $@ $<

$(BIN): $(OBJS)
	$(CC) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
