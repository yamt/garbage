WASI_SDK_DIR = /tmp/wasi-sdk/wasi-sdk-12.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed

OBJS = mod.o
BIN = test.wasm

$(BIN): $(OBJS)
	#$(LD) --allow-undefined --no-entry --export entry -o $@ $<
	$(CC) \
	-Wl,--allow-undefined \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)
