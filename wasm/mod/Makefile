WASI_SDK_DIR = /tmp/wasi-sdk/wasi-sdk-12.0

CC = $(WASI_SDK_DIR)/bin/clang
CFLAGS = -Os
CFLAGS += -I../embed

LD = $(WASI_SDK_DIR)/bin/wasm-ld

OBJS = mod.o
BIN = test.wasm

$(BIN): $(OBJS)
	$(LD) --allow-undefined --no-entry --export entry -o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
