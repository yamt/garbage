WASI_SDK_DIR = /opt/wasi-sdk-16.0
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

# make -j4 CC=/opt/wasi-sdk-16.0/bin/clang THREAD_MODEL=posix
SYSROOT = /Users/yamamoto/git/wasi-libc/sysroot
CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -pthread
#CFLAGS += -mbulk-memory
#CFLAGS += -matomics
CLINKFLAGS += -pthread
CLINKFLAGS += --sysroot $(SYSROOT)

#CLINKFLAGS += -Wl,--allow-undefined

CLINKFLAGS += -Wl,--gc-sections
#CLINKFLAGS += -Wl,--relocatable

NAME = threads
OBJS = $(NAME).o
BIN = $(NAME).wasm

STACK_SIZE = 8192
INITIAL_MEMORY = 65536
MAX_MEMORY = 6553600

BINS = $(BIN)

all: $(BINS)

$(BIN): $(OBJS)
	$(CC) \
	$(CLINKFLAGS) \
	-v \
	-z stack-size=$(STACK_SIZE) \
	-Wl,--initial-memory=$(INITIAL_MEMORY) \
	-Wl,--max-memory=$(MAX_MEMORY) \
	-Wl,--export=__data_end -Wl,--export=__heap_base \
	-Wl,--export=malloc -Wl,--export=free \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
