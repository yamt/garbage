WASI_SDK_DIR = /opt/wasi-sdk-24.0
SYSROOT = $(WASI_SDK_DIR)/share/wasi-sysroot/

CC = $(WASI_SDK_DIR)/bin/clang
CXX = $(WASI_SDK_DIR)/bin/clang++
LD = $(WASI_SDK_DIR)/bin/wasm-ld
NM = $(WASI_SDK_DIR)/bin/nm

CFLAGS = -Os
#CFLAGS += -fno-builtin-printf
#CFLAGS += --sysroot $(SYSROOT)
CFLAGS += -I../embed
CFLAGS += -Wall -Werror
CFLAGS += -g
CLINKFLAGS += -g

#SYSROOT = ${HOME}/git/wasi-libc/sysroot
#CFLAGS += --sysroot $(SYSROOT)
#CLINKFLAGS += --sysroot $(SYSROOT)

CXXFLAG = "${CFLAGS} -fno-exceptions -fno-rtti"

CLINKFLAGS += -z stack-size=$(STACK_SIZE)
CLINKFLAGS += -Wl,--initial-memory=$(INITIAL_MEMORY)
CLINKFLAGS += -Wl,--max-memory=$(MAX_MEMORY)

STACK_SIZE = 8192
INITIAL_MEMORY = 6553600
MAX_MEMORY = 6553600

OBJS = cxxhello.o
BIN = cxxhello.wasm

BINS = $(BIN)

.SUFFIXES: .cxx

all: $(BINS)

$(BIN): $(OBJS)
	$(CXX) \
	$(CLINKFLAGS) \
	-v \
	-o $@ $<

.c.o:
	$(CC) -c $(CFLAGS) -o $@ $<

.cxx.o:
	$(CXX) -c $(CXXFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -f $(BINS)
	rm -f $(OBJS)
