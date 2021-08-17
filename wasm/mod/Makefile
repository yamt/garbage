WASI_SDK_DIR = /tmp/wasi-sdk/wasi-sdk-12.0

CC = $(WASI_SDK_DIR)/bin/clang

OBJS = mod.o
BIN = test.wasm

$(BIN): $(OBJS)
	$(CC) -o $@ $<

.c.o:
	$(CC) -c -Os -o $@ $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
