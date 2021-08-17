WAMR_DIR = $(HOME)/git/wasm-micro-runtime
WAMR_LIB_DIR = $(WAMR_DIR)/product-mini/platforms/darwin/b
WASI_SDK_DIR = /tmp/wasi-sdk/wasi-sdk-12.0

CC = clang
CFLAGS = -Os
CFLAGS += -I$(WAMR_DIR)/core/iwasm/include

CLINKFLAGS = -L$(WAMR_LIB_DIR) -lvmlib 

OBJS = main.o
BIN = test
MOD = ../mod/test.wasm

.PHONY: run
run: $(BIN) $(MOD)
	./$(BIN) $(MOD)

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

$(BIN): $(OBJS)
	$(CC) $(CLINKFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
