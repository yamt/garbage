WAMR_DIR = $(HOME)/git/wasm-micro-runtime
# https://github.com/WebAssembly/wasi-sdk/releases/download/wasi-sdk-12/wasi-sdk-12.0-macos.tar.gz
WASI_SDK_DIR = /tmp/wasi-sdk-12.0

SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/
SYSROOT_PTHREAD = /tmp/wasi-sysroot-pthread

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm
AR = $(WASI_SDK_DIR)/bin/llvm-ar

CFLAGS = -Os
CFLAGS += --sysroot $(SYSROOT_PTHREAD)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror

OBJS = mod.o
BIN = test.wasm

$(BIN): $(OBJS)
	$(CC) \
	-Wl,--allow-undefined \
	-Wl,--export,call_indirect \
	-Wl,--export=__heap_base,--export=__data_end \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@
	$(CC) -S $(CFLAGS) $<

.PHONY: clean
clean:
	rm -f $(BIN)
	rm -f $(OBJS)

# https://github.com/bytecodealliance/wasm-micro-runtime/blob/main/doc/pthread_library.md
.PHONY: sysroot_pthread
sysroot_pthread:
	mkdir -p $(SYSROOT_PTHREAD)
	cp -R $(SYSROOT) $(SYSROOT_PTHREAD)
	$(AR) -d $(SYSROOT_PTHREAD)/lib/wasm32-wasi/libc.a dlmalloc.o
	cp $(WAMR_DIR)/wamr-sdk/app/libc-builtin-sysroot/include/pthread.h \
		$(SYSROOT_PTHREAD)/include
