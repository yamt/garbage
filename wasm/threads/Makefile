# Note: llvm >= 15 is necessary for tls
# https://reviews.llvm.org/D126107
WASI_SDK_DIR = /opt/wasi-sdk-20.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += -fno-builtin-printf
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror
CFLAGS += -g

# make -j4 CC=/opt/wasi-sdk-20.0/bin/clang THREAD_MODEL=posix
# SYSROOT = ${HOME}/git/wasi-libc/sysroot
CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -pthread
CFLAGS += -ftls-model=local-exec
#CFLAGS += -mbulk-memory
#CFLAGS += -matomics
CLINKFLAGS += -pthread
CLINKFLAGS += --sysroot $(SYSROOT)
CLINKFLAGS += -Wl,--import-memory
CLINKFLAGS += -Wl,--export-memory
# Note: --import-memory implicitly disables the "memory" export.
# Revisit when https://reviews.llvm.org/D135898 is available.

#TARGET = wasm32-wasi-pthread
TARGET = wasm32-wasi-threads
CFLAGS += --target=$(TARGET)
CLINKFLAGS += --target=$(TARGET)

#CLINKFLAGS += -Wl,--allow-undefined

CLINKFLAGS += -Wl,--gc-sections
#CLINKFLAGS += -Wl,--relocatable

CLINKFLAGS += -z stack-size=$(STACK_SIZE)
CLINKFLAGS += -Wl,--initial-memory=$(INITIAL_MEMORY)
CLINKFLAGS += -Wl,--max-memory=$(MAX_MEMORY)
#CLINKFLAGS += -Wl,--export=__data_end -Wl,--export=__heap_base
#CLINKFLAGS += -Wl,--export=malloc -Wl,--export=free

NAME = threads
OBJS = $(NAME).o
BIN = $(NAME).wasm

STACK_SIZE = 4096
INITIAL_MEMORY = 65536
MAX_MEMORY = 65536000

BINS = $(BIN)

all: $(BINS)

$(BIN): $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-v \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
