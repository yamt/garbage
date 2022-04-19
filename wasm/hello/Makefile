WASI_SDK_DIR = /tmp/wasi-sdk-14.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror

OBJS = hello.o
BIN = test.wasm

BINS = $(BIN)
BINS += $(BIN).aot-default
BINS += $(BIN).aot-xip

all: $(BINS)

$(BIN): $(OBJS)
	$(CC) \
	-Wl,--allow-undefined \
	-Wl,--export,call_indirect \
	-o $@ $<

$(BIN).aot-default: $(BIN)
	wamrc \
	-o $@ $<

$(BIN).aot-xip: $(BIN)
	wamrc \
	--enable-indirect-mode \
	--disable-llvm-intrinsics \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@
	# $(CC) -S $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
