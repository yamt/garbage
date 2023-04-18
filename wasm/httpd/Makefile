WASI_SDK_DIR = /opt/wasi-sdk-20.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
CFLAGS = -std=c11
#CFLAGS += -fno-builtin-printf
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror
CFLAGS += -g

ifeq ($(THREADS),1)
# make -j4 CC=/opt/wasi-sdk-20.0/bin/clang THREAD_MODEL=posix
#SYSROOT = ${HOME}/git/wasi-libc/sysroot
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -pthread
CFLAGS += -ftls-model=local-exec
CFLAGS += -DTHREADS
#CFLAGS += -mbulk-memory
#CFLAGS += -matomics
CLINKFLAGS += -pthread
#CLINKFLAGS += --sysroot $(SYSROOT)
CLINKFLAGS += -Wl,--import-memory

CFLAGS += --target=wasm32-wasi-threads
CLINKFLAGS += --target=wasm32-wasi-threads
endif

#CLINKFLAGS += -Wl,--allow-undefined

CLINKFLAGS += -Wl,--gc-sections
#CLINKFLAGS += -Wl,--relocatable

NAME = httpd$(SUFFIX)
OBJS = main.o
BIN = $(NAME).wasm

STACK_SIZE = 8192
INITIAL_MEMORY = 65536
MAX_MEMORY = 65536

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
	-o $@ $<

#	-Wl,--export=malloc -Wl,--export=free \

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<
	$(NM) $@

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)

.PHONY: style
style:
	clang-format-12 -i *.[ch]
