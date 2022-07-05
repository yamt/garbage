WASI_SDK_DIR = /tmp/wasi-sdk-12.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror

OBJS = mod.o
BIN = test.wasm

$(BIN): $(OBJS)
	$(CC) \
	-Wl,--allow-undefined \
	-Wl,--export,call_indirect \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@
	$(CC) -S $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
